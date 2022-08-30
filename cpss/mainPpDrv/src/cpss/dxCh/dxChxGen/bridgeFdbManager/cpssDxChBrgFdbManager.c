/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file cpssDxChBrgFdbManager.c
*
* @brief FDB manager support.
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/cpssDxChBrgFdbManager.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrg.h>
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrgFdbAu.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdbRouting.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpCtrl.h>

#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManagerLog.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManager_utils.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManager_db.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManager_hw.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManager_debug.h>

/* HA recovery */
#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/common/systemRecovery/private/prvCpssCommonSystemRecoveryParallel.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* #define FDB_MANAGER_REHASHING_DEBUG */

#define UNUSED_PARAMETER(x) x = x

/* NOTE: next 2 lines protect the implementation in this file from using wrong 'mutex' ! */
/* ALL FDB manager APIs should lock the 'FDB manager mutex' : PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS */
#undef PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS
#undef PRV_CPSS_FUNCTIONALITY_RXTX_CNS

/* error for mismatch the da/sa drop commands from 'entry' and the 'global mode' */
#define DROP_CMD_MISMATCH_ERROR(cmd,globalCmd)                              \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,                          \
            "%s - drop command [%s] must be drop command [%s] due to global [%s] ",  \
            #cmd,                                                            \
            (cmd == CPSS_PACKET_CMD_DROP_SOFT_E)       ? "SOFT" : "HARD" ,   \
            (globalCmd == CPSS_PACKET_CMD_DROP_SOFT_E) ? "SOFT" : "HARD" ,   \
            #globalCmd)

/** Macro to determine if the entry type is supported by the created shadow table */
#define FDB_MANAGER_SUPPORTED_ENTRY_CHECK(_fdbManagerPtr, _entryType)       \
    if((IS_FDB_MANAGER_FOR_SIP_4(_fdbManagerPtr)) &&                        \
            (_entryType > CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E))  \
    {                                                                       \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,                         \
                "Invalid entry type for this FDB manager instance");        \
    }

#define FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(_mask, _entryValue, _inputFilterValue)      \
    if((_mask != 0) && (((_entryValue) & (_mask)) != _inputFilterValue))                    \
    {                                                                                       \
        return GT_OK;                                                                       \
    }

#define FDB_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(_entryValue, _inputFilterValue)        \
    if((GT_U32)_entryValue != _inputFilterValue)                                            \
    {                                                                                       \
        return GT_OK;                                                                       \
    }

/* Vid1 filter is applicable only for MUXING_MODE_TAG1_VID */
#define FDB_SCAN_VALIDATE_FILTER_VID1_MAC_AND_RETURN_MAC(_MuxingMode, _mask, _entryValue, _inputFilterValue)      \
    if((_MuxingMode == CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_TAG1_VID_E) &&       \
       (_mask != 0) && (((_entryValue) & (_mask)) != _inputFilterValue))                    \
    {                                                                                       \
        return GT_OK;                                                                       \
    }

static GT_STATUS prvCpssDxChFdbManagerDbEntryDelete
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC   *dbEntryPtr
);

static GT_STATUS prvCpssDxChFdbManagerEntryDelete
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC               *fdbManagerPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC             *calcInfoPtr,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_DELETE_API_STATISTICS_ENT   *deleteApiStatisticsPtr
);

/**
* @internal internal_cpssDxChBrgFdbManagerCreate function
* @endinternal
*
* @brief  The function creates the FDB Manager and its databases according to
*        input capacity structure. Internally all FDB global parameters / modes
*        initialized to their defaults (No HW access - just SW defaults that will
*        be later applied to registered devices).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id to associate with the newly created manager.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] capacityPtr           - (pointer to) parameters of the HW structure and capacity of its managed PPs FDBs
*                                    like number of indexes, the amount of hashes etc.
*                                    As FDB Manager is created and entries may be added before PP registered
*                                    we must know in advance the relevant capacity.
*                                    In Falcon for example we must know the shared memory allocation mode.
* @param[in] entryAttrPtr          - (pointer to) parameters that affects how HW/SW entries are
*                                    structured and specific fields behavior including:
*                                    MUX'd fields in MAC / IPMC entries format, SA / DA drop commands mode &
*                                    IP NH packet command.
* @param[in] learningPtr           - (pointer to) parameters that affects which sort of entries to learn etc.
*                                    (Learn MAC of Routed packets and MAC no-space updates).
* @param[in] lookupPtr             - (pointer to) parameters that affect lookup of entries: MAC VLAN Lookup mode
*                                    (none, single or double tags), IVL single-tag MSB mode.
*                                    And two new Falcon setting: IPv4 UC and IPv6 UC masks
*                                    (allow using FDB for prefixes lookup and not just exact match)
* @param[in] agingPtr              - (pointer to) parameters that affect refresh and aging
*                                      (Refresh Destination UC, Refresh Destination MC &Refresh IP UC)
*
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_OUT_OF_CPU_MEM        - on fail to do Cpu memory allocation.
* @retval GT_ALREADY_EXIST         - if the FDB Manager id already exists.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgFdbManagerCreate
(
    IN GT_U32                                           fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC           *capacityPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC   *entryAttrPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC           *learningPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC             *lookupPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC              *agingPtr
)
{
    GT_STATUS rc;
    GT_BOOL managerAllocated;

    rc = prvCpssDxChFdbManagerDbCreate(fdbManagerId,capacityPtr,entryAttrPtr,learningPtr,lookupPtr,agingPtr,&managerAllocated);

    if(rc != GT_OK && managerAllocated == GT_TRUE)
    {
        /* incase of 'error' : if the manager was allocated
            we need to 'undo' the partial create. */

        /* delete the manager from the DB */
        (void)prvCpssDxChFdbManagerDbDelete(fdbManagerId);
    }

    return rc;
}

/**
* @internal cpssDxChBrgFdbManagerCreate function
* @endinternal
*
* @brief  The function creates the FDB Manager and its databases according to
*        input capacity structure. Internally all FDB global parameters / modes
*        initialized to their defaults (No HW access - just SW defaults that will
*        be later applied to registered devices).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id to associate with the newly created manager.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] capacityPtr           - (pointer to) parameters of the HW structure and capacity of its managed PPs FDBs
*                                    like number of indexes, the amount of hashes etc.
*                                    As FDB Manager is created and entries may be added before PP registered
*                                    we must know in advance the relevant capacity.
*                                    In Falcon for example we must know the shared memory allocation mode.
* @param[in] entryAttrPtr          - (pointer to) parameters that affects how HW/SW entries are
*                                    structured and specific fields behavior including:
*                                    MUX'd fields in MAC / IPMC entries format, SA / DA drop commands mode &
*                                    IP NH packet command.
* @param[in] learningPtr           - (pointer to) parameters that affects which sort of entries to learn etc.
*                                    (Learn MAC of Routed packets and MAC no-space updates).
* @param[in] lookupPtr             - (pointer to) parameters that affect lookup of entries: MAC VLAN Lookup mode
*                                    (none, single or double tags), IVL single-tag MSB mode.
*                                    And two new Falcon setting: IPv4 UC and IPv6 UC masks
*                                    (allow using FDB for prefixes lookup and not just exact match)
* @param[in] agingPtr              - (pointer to) parameters that affect refresh and aging
*                                      (Refresh Destination UC, Refresh Destination MC &Refresh IP UC)
*
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_OUT_OF_CPU_MEM        - on fail to do Cpu memory allocation.
* @retval GT_ALREADY_EXIST         - if the FDB Manager id already exists.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbManagerCreate
(
    IN GT_U32                                           fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC           *capacityPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC   *entryAttrPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC           *learningPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC             *lookupPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC              *agingPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbManagerCreate);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, fdbManagerId, capacityPtr, entryAttrPtr, learningPtr, lookupPtr, agingPtr));

    rc = internal_cpssDxChBrgFdbManagerCreate(fdbManagerId, capacityPtr, entryAttrPtr, learningPtr, lookupPtr, agingPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, fdbManagerId, capacityPtr, entryAttrPtr, learningPtr, lookupPtr, agingPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgFdbManagerDelete function
* @endinternal
*
* @brief  The function de-allocates specific FDB Manager Instance.
*         including all setting, entries, Data bases from all attached devices -
*         and return to initial state.
*         NOTE: the FDB manager will remove all HW entries from all the registered devices.
*
* @param[in] fdbManagerId            - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgFdbManagerDelete
(
    IN GT_U32 fdbManagerId
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr;

    FDB_MANAGER_ID_CHECK(fdbManagerId);
    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    /* Flush all entries from the HW of all registered devices */
    rc = prvCpssDxChFdbManagerHwFlushAll(fdbManagerPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* delete the manager from the DB */
    rc = prvCpssDxChFdbManagerDbDelete(fdbManagerId);

    return rc;
}

/**
* @internal cpssDxChBrgFdbManagerDelete function
* @endinternal
*
* @brief  The function de-allocates specific FDB Manager Instance.
*         including all setting, entries, Data bases from all attached devices -
*         and return to initial state.
*         NOTE: the FDB manager will remove all HW entries from all the registered devices.
*
* @param[in] fdbManagerId            - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbManagerDelete
(
    IN GT_U32 fdbManagerId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbManagerDelete);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, fdbManagerId));

    rc = internal_cpssDxChBrgFdbManagerDelete(fdbManagerId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, fdbManagerId));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}

/********************************/
/* validity checks of SW format */
/********************************/

/**
* @internal validateEntryParams_mux_macEntry function
* @endinternal
*
* @brief  function to validate the 'API LEVEL' FDB entry MAC format parameters , muxing part.
*         validate MUXED fields values of the FDB entry : fdbEntryMacAddrFormat
*         based on sip6FdbSpecialMuxedFieldsSet_mac
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[in] entryPtr              - (pointer to) the entry (application format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   validateEntryParams_mux_macEntry
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC      *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC *entryPtr
)
{
    GT_U32      maxSourceId=0;     /* upper bound for valid sourceId */
    GT_U32      maxUdb=0;          /* upper bound for valid udb      */
    GT_U32      maxVid1=0;         /* upper bound for valid vid1     */
    GT_U32      maxDaAccessLevel=0;/* upper bound for valid daAccessLevel */
    GT_U32      maxSaAccessLevel=BIT_1;/* upper bound for valid saAccessLevel */

    switch(fdbManagerPtr->entryAttrInfo.shadowType)
    {
        case CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E:
            maxUdb           = BIT_4;
            maxSourceId      = BIT_5;
            maxDaAccessLevel = BIT_3;
            maxSaAccessLevel = BIT_3;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E:
            maxSaAccessLevel = 0;
            switch (fdbManagerPtr->entryAttrInfo.entryMuxingMode)
            {
                default:
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_9_SRC_ID_8_UDB_TAG1_VID_DISABLE_E:
                    maxUdb           = BIT_8;
                    maxSourceId      = BIT_9;
                    maxDaAccessLevel = BIT_3;
                    maxSaAccessLevel = BIT_3;
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_12_SRC_ID_5_UDB_TAG1_VID_DISABLE_E:
                    maxUdb           = BIT_5;
                    maxSourceId      = BIT_12;
                    maxDaAccessLevel = BIT_3;
                    maxSaAccessLevel = BIT_3;
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_8_UDB_TAG1_VID_ENABLE_E:
                    maxVid1          = BIT_12;
                    maxUdb           = BIT_8;
                    maxSourceId      = BIT_6;
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_5_UDB_TAG1_VID_ENABLE_E:
                    maxVid1          = BIT_12;
                    maxUdb           = BIT_5;
                    maxSourceId      = BIT_6;
                    break;
            }
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E:
            /* Common parameters which can be supported both on the SIP6 and SIP5 devices
             * should be independent of muxing mode */
            maxVid1          = 0;
            maxUdb           = 0;
            maxSourceId      = BIT_12;
            maxDaAccessLevel = 0;
            maxSaAccessLevel = 0;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP6_E:
            switch (fdbManagerPtr->entryAttrInfo.macEntryMuxingMode)
            {
                case CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_TAG1_VID_E:
                    maxVid1          = BIT_12;
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_SRC_ID_E:
                    maxSourceId      = BIT_12;
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_UDB_10_SRC_ID_1_DA_ACCESS_LEVEL_E:
                    maxUdb           = BIT_10;
                    maxSourceId      = BIT_1;
                    maxDaAccessLevel = BIT_1;
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_UDB_8_SRC_ID_3_DA_ACCESS_LEVEL_E:
                    /* UNLIKLE the CPSS : the value is continues. (no 2 bits hole) */
                    maxUdb           = BIT_8;
                    maxSourceId      = BIT_3;
                    maxDaAccessLevel = BIT_1;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(fdbManagerPtr->entryAttrInfo.macEntryMuxingMode);
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(fdbManagerPtr->entryAttrInfo.shadowType);
    }

    if (entryPtr->vid1 >= maxVid1)
    {
        if(maxVid1 == 0)
        {
            /* we allow 0 only */
             if(entryPtr->vid1 != 0)
             {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "entryPtr->vid1 [%d] must be ZERO because not exists in the entry (due to muxing mode)",
                    entryPtr->vid1);
             }
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "entryPtr->vid1 [%d] must be less than [%d]",
                entryPtr->vid1,maxVid1);
        }
    }

    if (entryPtr->sourceID >= maxSourceId)
    {
        if(maxSourceId == 0)
        {
            /* we allow 0 only */
             if(entryPtr->sourceID != 0)
             {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "entryPtr->sourceID [%d] must be ZERO because not exists in the entry (due to muxing mode)",
                    entryPtr->sourceID);
             }
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "entryPtr->sourceID [%d] must be less than [%d]",
                entryPtr->sourceID,maxSourceId);
        }
    }

    if (entryPtr->userDefined >= maxUdb)
    {
        if(maxUdb == 0)
        {
            /* we allow 0 only */
             if(entryPtr->userDefined != 0)
             {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "entryPtr->userDefined [%d] must be ZERO because not exists in the entry (due to muxing mode)",
                    entryPtr->userDefined);
             }
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "entryPtr->userDefined [%d] must be less than [%d]",
                entryPtr->userDefined,maxUdb);
        }
    }

    if (entryPtr->daSecurityLevel >= maxDaAccessLevel)
    {
        if(maxDaAccessLevel == 0)
        {
            /* we allow 0 only */
             if(entryPtr->daSecurityLevel != 0)
             {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "entryPtr->daSecurityLevel [%d] must be ZERO because not exists in the entry (due to muxing mode)",
                    entryPtr->daSecurityLevel);
             }
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "entryPtr->daSecurityLevel [%d] must be less than [%d]",
                entryPtr->daSecurityLevel,maxDaAccessLevel);
        }
    }

    if (entryPtr->saSecurityLevel >= maxSaAccessLevel)
    {
        if(maxSaAccessLevel == 0)
        {
            /* we allow 0 only */
            if(entryPtr->saSecurityLevel != 0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "entryPtr->saSecurityLevel [%d] must be ZERO because not exists in the entry (due to muxing mode)",
                        entryPtr->saSecurityLevel);
            }
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "entryPtr->saSecurityLevel [%d] must be less than [%d]",
                    entryPtr->saSecurityLevel,maxSaAccessLevel);
        }
    }

    return GT_OK;
}

/**
* @internal validateEntryParams_mux_ipMcEntry function
* @endinternal
*
* @brief  function to validate the 'API LEVEL' FDB entry IP MC format parameters , muxing part.
*         validate MUXED fields values of the FDB entry : fdbEntryIpv4McFormat and fdbEntryIpv6McFormat
*         based on sip6FdbSpecialMuxedFieldsSet_ipmc
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[in] entryPtr              - (pointer to) the entry (application format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   validateEntryParams_mux_ipMcEntry
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC       *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_IP_MC_FORMAT_STC     *entryPtr
)
{
    GT_U32      maxSourceId=0;              /* upper bound for valid sourceId */
    GT_U32      maxUdb=0;                   /* upper bound for valid udb      */
    GT_U32      maxDaAccessLevel = BIT_1;   /* upper bound for valid daAccessLevel */
    GT_U32      maxVid1 = 0;

    switch(fdbManagerPtr->entryAttrInfo.shadowType)
    {
        case CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E:
            maxUdb           = 0;       /* AC5 Multicast do not support */
            maxSourceId      = 0;       /* AC5 Multicast do not support */
            maxDaAccessLevel = BIT_3;
            break;

        case CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E:
            maxSourceId      = 0;       /* SIP5 Multicast do not support */
            maxDaAccessLevel = 0;
            switch (fdbManagerPtr->entryAttrInfo.entryMuxingMode)
            {
                default:
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_9_SRC_ID_8_UDB_TAG1_VID_DISABLE_E:
                    maxUdb           = BIT_8;
                    maxDaAccessLevel = BIT_3;
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_12_SRC_ID_5_UDB_TAG1_VID_DISABLE_E:
                    maxUdb           = BIT_5;
                    maxDaAccessLevel = BIT_3;
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_8_UDB_TAG1_VID_ENABLE_E:
                    maxUdb           = BIT_8;
                    maxVid1          = BIT_12;
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_5_UDB_TAG1_VID_ENABLE_E:
                    maxUdb           = BIT_5;
                    maxVid1          = BIT_12;
                    break;
            }
            break;

        case CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E:
            /* Common parameters which can be supported both on the SIP6 and SIP5 devices */
            maxSourceId      = BIT_7;
            maxDaAccessLevel = 0;
            maxUdb           = 0;
            break;

        case CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP6_E:
            switch (fdbManagerPtr->entryAttrInfo.ipmcEntryMuxingMode)
            {
                case CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_SRC_ID_E:
                    maxSourceId = BIT_7;
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_UDB_4_SRC_ID_3_E:
                    maxSourceId = BIT_3;
                    maxUdb = BIT_4;
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_UDB_E:
                    maxUdb = BIT_7;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(fdbManagerPtr->entryAttrInfo.ipmcEntryMuxingMode);
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(fdbManagerPtr->entryAttrInfo.shadowType);
    }

    if (entryPtr->sourceID >= maxSourceId)
    {
        if(maxSourceId == 0)
        {
            /* we allow 0 only */
             if(entryPtr->sourceID != 0)
             {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "sourceID [%d] must be ZERO because not exists in the entry (due to muxing mode)",
                    entryPtr->sourceID);
             }
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "sourceID [%d] must be less than [%d]",
                entryPtr->sourceID, maxSourceId);
        }
    }

    if (entryPtr->userDefined >= maxUdb)
    {
        if(maxUdb == 0)
        {
            /* we allow 0 only */
             if(entryPtr->userDefined != 0)
             {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "udb [%d] must be ZERO because not exists in the entry (due to muxing mode)",
                    entryPtr->userDefined);
             }
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "udb [%d] must be less than [%d]",
                entryPtr->userDefined, maxUdb);
        }
    }

    if (entryPtr->vid1 >= maxVid1)
    {
        if(maxVid1 == 0)
        {
            /* we allow 0 only */
             if(entryPtr->vid1 != 0)
             {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "vid1 [%d] must be ZERO because not exists in the entry (due to muxing mode)",
                    entryPtr->vid1);
             }
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "vid1 [%d] must be less than [%d]",
                entryPtr->vid1, maxVid1);
        }
    }

    if (entryPtr->daSecurityLevel >= maxDaAccessLevel)
    {
        if(maxDaAccessLevel == 0)
        {
            /* we allow 0 only */
             if(entryPtr->daSecurityLevel != 0)
             {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "entryPtr->daSecurityLevel [%d] must be ZERO because not exists in the entry (due to muxing mode)",
                    entryPtr->daSecurityLevel);
             }
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "entryPtr->daSecurityLevel [%d] must be less than [%d]",
                entryPtr->daSecurityLevel,maxDaAccessLevel);
        }
    }

    return GT_OK;
}

/**
* @internal validateEntryFormatParams_dstInterface_macOrIpmc function
* @endinternal
*
* @brief  function to validate the 'API LEVEL' FDB entry MAC/IPMC format parameters , part 'dstInterfacePtr' .
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] dstInterfacePtr        - (pointer to) the destination interface (application format)
* @param[in] isMultiCast            - indication that entry is for multicast
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   validateEntryFormatParams_dstInterface_macOrIpmc
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC       *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_INTERFACE_INFO_STC   *dstInterfacePtr,
    IN  GT_U32                                              isMultiCast
)
{
    GT_U32      maxVal = 0;

    switch(dstInterfacePtr->type)
    {
        case CPSS_INTERFACE_PORT_E:
            maxVal = (IS_FDB_MANAGER_FOR_SIP_4(fdbManagerPtr))?BIT_6:BIT_13;
            CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(
                dstInterfacePtr->interfaceInfo.devPort.portNum,
                maxVal);
            maxVal = (IS_FDB_MANAGER_FOR_SIP_4(fdbManagerPtr))?BIT_5:BIT_10;
            CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(
                dstInterfacePtr->interfaceInfo.devPort.hwDevNum,
                maxVal);

            if(isMultiCast)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                    "'Port interface' is not supported for For multicast MAC or IPM entries");
            }

            break;
        case CPSS_INTERFACE_TRUNK_E:
            maxVal = (IS_FDB_MANAGER_FOR_SIP_4(fdbManagerPtr))?BIT_7:BIT_12;
            CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(
                dstInterfacePtr->interfaceInfo.trunkId,
                maxVal);

            if(isMultiCast)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                    "'Trunk interface' is not supported for For multicast MAC or IPM entries");
            }

            break;
        case CPSS_INTERFACE_VIDX_E:
            switch(fdbManagerPtr->entryAttrInfo.shadowType)
            {
                case CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP6_E:
                case CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E:
                    maxVal = BIT_14;
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E:
                    maxVal = BIT_16;
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E:
                    maxVal = BIT_12;
                    break;
                default:
                    break;
            }
            CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(
                dstInterfacePtr->interfaceInfo.vidx,
                maxVal);
            break;
        case CPSS_INTERFACE_VID_E:
            /* no param needed !!! */
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(dstInterfacePtr->type);
    }

    return GT_OK;
}

/**
* @internal validateEntryFormatParams_fdbEntryMacAddrFormat function
* @endinternal
*
* @brief  function to validate the 'API LEVEL' FDB entry MAC format parameters.
*         based on prvDxChBrgFdbBuildMacEntryHwFormat
*         NOTE: the 'key' was already checked
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] entryPtr              - (pointer to) the entry (application format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   validateEntryFormatParams_fdbEntryMacAddrFormat
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC      *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC *entryPtr
)
{
    GT_STATUS   rc;
    GT_BIT      isMacMc;

    isMacMc  = entryPtr->macAddr.arEther[0] & 1; /* check bit 40 of the mac address */

/*  next are checked with explicit 'mux mode' inside :  validateEntryParams_mux_macEntry(...)
    CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(entryPtr->daSecurityLevel  ,BIT_1 );
    CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(entryPtr->saSecurityLevel  ,BIT_1 );
    CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(entryPtr->sourceID         ,BIT_12);
    CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(entryPtr->userDefined      ,BIT_12);
*/
    /* check muxed fields that depends on global configurations */
    rc = validateEntryParams_mux_macEntry(fdbManagerPtr,entryPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    switch(entryPtr->daCommand)
    {
        case CPSS_PACKET_CMD_FORWARD_E      :
        case CPSS_PACKET_CMD_MIRROR_TO_CPU_E:
        case CPSS_PACKET_CMD_TRAP_TO_CPU_E  :
        case CPSS_PACKET_CMD_DROP_HARD_E    :
        case CPSS_PACKET_CMD_DROP_SOFT_E    :
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(entryPtr->daCommand);
    }

    switch(entryPtr->saCommand)
    {
        case CPSS_PACKET_CMD_FORWARD_E      :
            break;
        case CPSS_PACKET_CMD_DROP_HARD_E    :
        case CPSS_PACKET_CMD_DROP_SOFT_E    :
            break;
        case CPSS_PACKET_CMD_MIRROR_TO_CPU_E:
        case CPSS_PACKET_CMD_TRAP_TO_CPU_E  :
            if((fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP6_E) ||
                (fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E))
            {
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(entryPtr->saCommand);
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(entryPtr->saCommand);
    }

    rc = validateEntryFormatParams_dstInterface_macOrIpmc(fdbManagerPtr,
            &entryPtr->dstInterface,
            isMacMc);
    if(rc != GT_OK)
    {
        return rc;
    }

    if((fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E) ||
            (fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E))
    {
        if(entryPtr->daQoSParameterSetIndex >= BIT_3)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "daQoSParameterSetIndex [%d] must be less than [%d]",
                    entryPtr->daQoSParameterSetIndex, BIT_3);
        }
        if(entryPtr->saQoSParameterSetIndex >= BIT_3)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "saQoSParameterSetIndex [%d] must be less than [%d]",
                    entryPtr->saQoSParameterSetIndex, BIT_3);
        }

        switch(entryPtr->mirrorToAnalyzerPort)
        {
            case CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_DISABLED_E:
            case CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_SA_OR_DA_E:
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_DA_E:
            case CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_SA_E:
                if(fdbManagerPtr->entryAttrInfo.shadowType != CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E)
                {
                    CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(entryPtr->mirrorToAnalyzerPort);
                }
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(entryPtr->mirrorToAnalyzerPort);
        }
    }
    else
    {
        if(entryPtr->mirrorToAnalyzerPort != CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_DISABLED_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Mirror to analyzer is not support for this device type\n");
        }
        if((entryPtr->daQoSParameterSetIndex != 0) || (entryPtr->saQoSParameterSetIndex != 0))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "QOS parameter is not support for this device type\n");
        }
    }
    return GT_OK;
}

/**
* @internal validateEntryFormatParams_fdbEntryIpMcAddrFormat function
* @endinternal
*
* @brief  function to validate the 'API LEVEL' FDB entry IP MC format parameters.
*         based on prvDxChBrgFdbBuildMacEntryHwFormat
*         NOTE: the 'key' was already checked
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] entryPtr              - (pointer to) the entry (application format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   validateEntryFormatParams_fdbEntryIpMcAddrFormat
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC       *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_IP_MC_FORMAT_STC     *entryPtr
)
{
    GT_STATUS   rc;

    /* check muxed fields that depends on global configurations */
    rc = validateEntryParams_mux_ipMcEntry(fdbManagerPtr, entryPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    switch(entryPtr->daCommand)
    {
        case CPSS_PACKET_CMD_FORWARD_E      :
        case CPSS_PACKET_CMD_MIRROR_TO_CPU_E:
        case CPSS_PACKET_CMD_TRAP_TO_CPU_E  :
        case CPSS_PACKET_CMD_DROP_HARD_E    :
        case CPSS_PACKET_CMD_DROP_SOFT_E    :
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(entryPtr->daCommand);
    }

    if((fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E) ||
            (fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E))
    {
        if(entryPtr->daQoSParameterSetIndex >= BIT_3)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "daQoSParameterSetIndex [%d] must be less than [%d]",
                    entryPtr->daQoSParameterSetIndex, BIT_3);
        }
        if(entryPtr->saQoSParameterSetIndex >= BIT_3)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "saQoSParameterSetIndex [%d] must be less than [%d]",
                    entryPtr->saQoSParameterSetIndex, BIT_3);
        }
    }
    else
    {
        if(entryPtr->mirrorToAnalyzerPort != CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_DISABLED_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Mirror to analyzer is not support for this device type\n");
        }
        if((entryPtr->daQoSParameterSetIndex != 0) || (entryPtr->saQoSParameterSetIndex != 0))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "QOS parameter is not support for this device type\n");
        }
    }

    return validateEntryFormatParams_dstInterface_macOrIpmc(fdbManagerPtr, &entryPtr->dstInterface, GT_TRUE);
}

/**
* @internal validateEntryFormatParams_dstInterface_IpvxUc function
* @endinternal
*
* @brief  function to validate the 'API LEVEL' FDB entry IPvx UC format parameters , part 'dstInterfacePtr' .
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] dstInterfacePtr        - (pointer to) the destination interface (application format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; AC5.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   validateEntryFormatParams_dstInterface_IpvxUc
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC     *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_INTERFACE_INFO_STC *dstInterfacePtr

)
{
    GT_U32          maxVal;
    fdbManagerPtr = fdbManagerPtr;

    switch(dstInterfacePtr->type)
    {
        case CPSS_INTERFACE_PORT_E:
            if((fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E) ||
                    (fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E))
            {
                maxVal = BIT_13;
            }
            else
            {
                maxVal = BIT_14;
            }
            CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(
                dstInterfacePtr->interfaceInfo.devPort.portNum,
                maxVal);
            CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(
                dstInterfacePtr->interfaceInfo.devPort.hwDevNum,
                BIT_10);
            break;
        case CPSS_INTERFACE_TRUNK_E:
            CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(
                dstInterfacePtr->interfaceInfo.trunkId,
                BIT_12);
            break;
        case CPSS_INTERFACE_VIDX_E:
            maxVal = (fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E)?BIT_16:BIT_14;
            CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(
                    dstInterfacePtr->interfaceInfo.vidx,
                    maxVal);
            break;
        case CPSS_INTERFACE_VID_E:
            /* no param needed !!! */
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(dstInterfacePtr->type);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChFdbManagerNextHopCommonInfoValidate function
* @endinternal
*
* @brief  function validates values of the FDB entry for next hop common format
*         copy values from application format to manager format
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] entryPtr               - (pointer to) the entry next hop full info (application format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; AC5.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   prvCpssDxChFdbManagerNextHopCommonInfoValidate
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC                           *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_STC  *entryPtr
)
{
    GT_U32          maxValue;
    fdbManagerPtr = fdbManagerPtr;

    switch (entryPtr->countSet)
    {
        case CPSS_IP_CNT_SET0_E:
        case CPSS_IP_CNT_SET1_E:
        case CPSS_IP_CNT_SET2_E:
        case CPSS_IP_CNT_SET3_E:
        case CPSS_IP_CNT_NO_SET_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                "Wrong 'Next Hop' counter value [%d]", entryPtr->countSet);
    }

    CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(
        entryPtr->nextHopVlanId,
        BIT_13);

    maxValue = (fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E)?BIT_3:BIT_1;
    CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(
        entryPtr->mtuProfileIndex,
        maxValue);

    return GT_OK;
}

/**
* @internal validateEntryFormatParams_common_info_IpvxUc_sip5 function
* @endinternal
*
* @brief  function to validate the 'API LEVEL' FDB entry IPvx UC format parameters , part 'ucCommonInfo' .
*
* @param[in] fdbManagerPtr        - (pointer to) the FDB Manager.
* @param[in] ucCommonInfo         - (pointer to) the 'common' routing info (application format)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; AC5.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   validateEntryFormatParams_common_info_IpvxUc_sip5
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC      *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_IP_UC_COMMON_STC          *ucCommonInfoPtr

)
{
    if(fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E)
    {
        if((ucCommonInfoPtr->qosProfileMarkingEnable != GT_FALSE) ||
           (ucCommonInfoPtr->qosProfilePrecedence    != CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E) ||
           (ucCommonInfoPtr->modifyUp                != CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E) ||
           (ucCommonInfoPtr->modifyDscp              != CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E) ||
           (ucCommonInfoPtr->arpBcTrapMirrorEnable   != GT_FALSE) ||
           (ucCommonInfoPtr->dipAccessLevel          != 0) ||
           (ucCommonInfoPtr->ingressMirrorToAnalyzerEnable != GT_FALSE))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(ucCommonInfoPtr->qosProfileIndex, BIT_7);
        CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(ucCommonInfoPtr->dipAccessLevel,  BIT_6);
        CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(ucCommonInfoPtr->ingressMirrorToAnalyzerIndex, (BIT_3-1));
    }
    return GT_OK;
}

/**
* @internal validateEntryFormatParams_next_hop_IpvxUc function
* @endinternal
*
* @brief  function to validate the 'API LEVEL' FDB entry IPvx UC format parameters , part 'ucRouteInfoPtr' .
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] ucRouteType            - UC routing type (application format)
* @param[in] ucRouteInfoPtr         - (pointer to) the 'next hop' routing info (application format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; AC5.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   validateEntryFormatParams_next_hop_IpvxUc
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC               *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_ENT                ucRouteType,
    IN CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_UNT  *ucRouteInfoPtr

)
{
    GT_STATUS   rc;
    GT_U32      maxPointer;

    fdbManagerPtr = fdbManagerPtr;

    switch(ucRouteType)
    {
        case CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E:
            rc = prvCpssDxChFdbManagerNextHopCommonInfoValidate(fdbManagerPtr, &ucRouteInfoPtr->fullFdbInfo);
            if (rc != GT_OK)
            {
                return rc;
            }
            maxPointer = ((fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E) ||
                    (fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E))?BIT_15:BIT_16;
            CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(
                ucRouteInfoPtr->fullFdbInfo.pointer.tunnelStartPointer,
                maxPointer);
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_NAT_PTR_E:
            if(fdbManagerPtr->entryAttrInfo.shadowType != CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP6_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            rc = prvCpssDxChFdbManagerNextHopCommonInfoValidate(fdbManagerPtr, &ucRouteInfoPtr->fullFdbInfo);
            if (rc != GT_OK)
            {
                return rc;
            }
            CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(
                ucRouteInfoPtr->fullFdbInfo.pointer.natPointer,
                BIT_16);
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E:
            rc = prvCpssDxChFdbManagerNextHopCommonInfoValidate(fdbManagerPtr, &ucRouteInfoPtr->fullFdbInfo);
            if (rc != GT_OK)
            {
                return rc;
            }
            maxPointer = ((fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E) ||
                    (fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E))?BIT_17:BIT_18;
            CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(
                ucRouteInfoPtr->fullFdbInfo.pointer.arpPointer,
                maxPointer);
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_POINTER_TO_ROUTER_E:
            if(fdbManagerPtr->entryAttrInfo.shadowType != CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP6_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(
                ucRouteInfoPtr->nextHopPointerToRouter,
                BIT_15);
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_ECMP_POINTER_TO_ROUTER_E:
            if(fdbManagerPtr->entryAttrInfo.shadowType != CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP6_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(
                ucRouteInfoPtr->ecmpPointerToRouter,
                BIT_15);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(ucRouteType);
    }

    return GT_OK;
}

/**
* @internal validateEntryFormatParams_fdbEntryIpv4UcFormat function
* @endinternal
*
* @brief  function to validate the 'API LEVEL' FDB entry IPv4 UC format parameters.
*         NOTE: the 'key' was already checked
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] entryPtr              - (pointer to) the entry (application format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; AC5.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   validateEntryFormatParams_fdbEntryIpv4UcFormat
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC     *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_IPV4_UC_ROUTING_INFO_STC *entryPtr
)
{
    GT_STATUS   rc;

    rc = validateEntryFormatParams_next_hop_IpvxUc(fdbManagerPtr, entryPtr->ucRouteType, &entryPtr->ucRouteInfo);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E)
    {
        rc = validateEntryFormatParams_common_info_IpvxUc_sip5(fdbManagerPtr, &entryPtr->ucCommonInfo);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if((entryPtr->ucRouteType == CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E) ||
            (entryPtr->ucRouteType == CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_NAT_PTR_E) ||
            (entryPtr->ucRouteType == CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E))
    {
        rc = validateEntryFormatParams_dstInterface_IpvxUc(fdbManagerPtr, &entryPtr->ucRouteInfo.fullFdbInfo.dstInterface);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal validateEntryFormatParams_fdbEntryIpv6UcFormat function
* @endinternal
*
* @brief  function to validate the 'API LEVEL' FDB entry IPv6 UC format parameters.
*         NOTE: the 'key' was already checked
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] entryPtr              - (pointer to) the entry (application format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; AC5.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   validateEntryFormatParams_fdbEntryIpv6UcFormat
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC     *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_IPV6_UC_ROUTING_INFO_STC *entryPtr
)
{
    GT_STATUS   rc;

    rc = validateEntryFormatParams_next_hop_IpvxUc(fdbManagerPtr, entryPtr->ucRouteType, &entryPtr->ucRouteInfo);
    if(rc != GT_OK)
    {
        return rc;
    }

    if((fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E) ||
        (fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E))
    {
        rc = validateEntryFormatParams_common_info_IpvxUc_sip5(fdbManagerPtr, &entryPtr->ucCommonInfo);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if((entryPtr->ucRouteType == CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E) ||
            (entryPtr->ucRouteType == CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_NAT_PTR_E) ||
            (entryPtr->ucRouteType == CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E))
    {
        rc = validateEntryFormatParams_dstInterface_IpvxUc(fdbManagerPtr,&entryPtr->ucRouteInfo.fullFdbInfo.dstInterface);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/************************************/
/* validity checks of SW KEY format */
/************************************/

/**
* @internal validateEntryKeyFormatParams_fdbEntryMacAddrFormat function
* @endinternal
*
* @brief  function to validate the 'API LEVEL' FDB MAC key parameters.
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] entryPtr              - (pointer to) the entry (application format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   validateEntryKeyFormatParams_fdbEntryMacAddrFormat
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC      *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC *entryPtr
)
{
    switch(fdbManagerPtr->entryAttrInfo.shadowType)
    {
        case CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E:
            CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(entryPtr->fid      ,BIT_12);
            if(entryPtr->vid1 != 0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                        "entryPtr->vid1[%d] must be ZERO - current device does not support",
                        entryPtr->vid1);
            }
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E:
        case CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP6_E:
        case CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E:
            switch(fdbManagerPtr->learningInfo.macVlanLookupMode)
            {
                case CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_ONLY_E:
                case CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_E:
                    CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(entryPtr->fid      ,BIT_13);
                    break;
                case CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_AND_VID1_E:
                    CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(entryPtr->fid      ,BIT_13);
                    CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(entryPtr->vid1     ,BIT_12);
                    break;
                default:/* we will not get here ... already checked during 'manager create' */
                    break;
            }
            break;
    }

    return GT_OK;
}

/**
* @internal validateEntryKeyFormatParams_fdbEntryIpMcAddrFormat function
* @endinternal
*
* @brief  function to validate the 'API LEVEL' FDB IP MC key parameters.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[in] entryPtr              - (pointer to) the entry (application format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   validateEntryKeyFormatParams_fdbEntryIpMcAddrFormat
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC       *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_IP_MC_FORMAT_STC     *entryPtr
)
{
    GT_U32 maxVal;

    maxVal = (fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E)?BIT_12:BIT_13;
    CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(entryPtr->fid      ,maxVal);

    return GT_OK;
}

/**
* @internal validateEntryKeyFormatParams_fdbEntryIpV4UcAddrFormat function
* @endinternal
*
* @brief  function to validate the 'API LEVEL' FDB IPv4 UC key parameters.
*
* @param[in] entryPtr              - (pointer to) the entry (application format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; AC5.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   validateEntryKeyFormatParams_fdbEntryIpV4UcAddrFormat
(
    IN CPSS_DXCH_BRG_FDB_MANAGER_IPV4_UC_ROUTING_INFO_STC   *entryPtr
)
{
    CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(entryPtr->vrfId      ,BIT_12);

    return GT_OK;
}

/**
* @internal validateEntryKeyFormatParams_fdbEntryIpV6UcAddrFormat function
* @endinternal
*
* @brief  function to validate the 'API LEVEL' FDB IP UC key parameters.
*
* @param[in] entryPtr              - (pointer to) the entry (application format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; AC5.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   validateEntryKeyFormatParams_fdbEntryIpV6UcAddrFormat
(
    IN CPSS_DXCH_BRG_FDB_MANAGER_IPV6_UC_ROUTING_INFO_STC   *entryPtr
)
{
    CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(entryPtr->vrfId      ,BIT_12);

    return GT_OK;
}

/**
* @internal prvCpssDxChFdbManagerValidateEntryKeyFormatParams function
* @endinternal
*
* @brief  function to validate the 'API LEVEL' FDB key parameters.
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] entryPtr              - (pointer to) the entry (application format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   prvCpssDxChFdbManagerValidateEntryKeyFormatParams
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC            *entryPtr
)
{
    switch(entryPtr->fdbEntryType)
    {
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E:
            return validateEntryKeyFormatParams_fdbEntryMacAddrFormat(fdbManagerPtr,
                                                                &entryPtr->format.fdbEntryMacAddrFormat);
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E:
            return validateEntryKeyFormatParams_fdbEntryIpV4UcAddrFormat(
                                                                &entryPtr->format.fdbEntryIpv4UcFormat);
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E:
            return validateEntryKeyFormatParams_fdbEntryIpV6UcAddrFormat(
                                                                &entryPtr->format.fdbEntryIpv6UcFormat);
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E:
            return validateEntryKeyFormatParams_fdbEntryIpMcAddrFormat(fdbManagerPtr,
                                                                &entryPtr->format.fdbEntryIpv4McFormat);
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E:
            return validateEntryKeyFormatParams_fdbEntryIpMcAddrFormat(fdbManagerPtr,
                                                                &entryPtr->format.fdbEntryIpv6McFormat);
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(entryPtr->fdbEntryType);
    }
}

/**
* @internal prvCpssDxChFdbManagerValidateEntryFormatParams function
* @endinternal
*
* @brief  function to validate the 'API LEVEL' FDB parameters.
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] entryPtr              - (pointer to) the entry (application format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   prvCpssDxChFdbManagerValidateEntryFormatParams
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC            *entryPtr
)
{
    GT_STATUS rc;

    /* validate the 'KEY' (hash related) parameters */
    rc = prvCpssDxChFdbManagerValidateEntryKeyFormatParams(fdbManagerPtr,entryPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* validate the rest of the parameters */
    switch(entryPtr->fdbEntryType)
    {
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E:
            return validateEntryFormatParams_fdbEntryMacAddrFormat (fdbManagerPtr, &entryPtr->format.fdbEntryMacAddrFormat);
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E:
            return validateEntryFormatParams_fdbEntryIpv4UcFormat  (fdbManagerPtr, &entryPtr->format.fdbEntryIpv4UcFormat);
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E:
            return validateEntryFormatParams_fdbEntryIpv6UcFormat  (fdbManagerPtr, &entryPtr->format.fdbEntryIpv6UcFormat);
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E:
            return validateEntryFormatParams_fdbEntryIpMcAddrFormat(fdbManagerPtr, &entryPtr->format.fdbEntryIpv4McFormat);
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E:
            return validateEntryFormatParams_fdbEntryIpMcAddrFormat(fdbManagerPtr, &entryPtr->format.fdbEntryIpv6McFormat);
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(entryPtr->fdbEntryType);
    }
}

/**
* @internal saveEntryFormatParamsToDb_fdbEntryMacAddrFormat_interfaceAndSourceId function
* @endinternal
*
* @brief  function to save interface and Source ID values of the FDB entry into DB in format
*         PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_MAC_ADDR_FORMAT_STC
*         copy values from application format to manager format
*
* @param[in] entryPtr               - (pointer to) the entry (application format)
* @param[out] dbEntryPtr            - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval none
*/
static GT_VOID saveEntryFormatParamsToDb_fdbEntryMacAddrFormat_interfaceAndSourceId
(
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC      *entryPtr,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_MAC_ADDR_FORMAT_STC  *dbEntryPtr
)
{
    /* save to DB format from application format  */
    dbEntryPtr->srcId              = entryPtr->sourceID         ;

    dbEntryPtr->dstInterface_type  = entryPtr->dstInterface.type;
    switch(entryPtr->dstInterface.type)
    {
        case CPSS_INTERFACE_PORT_E:
            dbEntryPtr->dstInterface.devPort.hwDevNum  = entryPtr->dstInterface.interfaceInfo.devPort.hwDevNum;
            dbEntryPtr->dstInterface.devPort.portNum   = entryPtr->dstInterface.interfaceInfo.devPort.portNum ;
            break;
        case CPSS_INTERFACE_TRUNK_E:
            dbEntryPtr->dstInterface.trunkId   = entryPtr->dstInterface.interfaceInfo.trunkId;
            break;
        case CPSS_INTERFACE_VIDX_E:
            dbEntryPtr->dstInterface.vidx      = entryPtr->dstInterface.interfaceInfo.vidx;
            break;
        case CPSS_INTERFACE_VID_E:
            dbEntryPtr->dstInterface.vidx      = FLOOD_VIDX_CNS;
            break;
        default:
            break;
    }

    return;
}

/**
* @internal saveEntryFormatParamsToDb_fdbEntryMacAddrFormat function
* @endinternal
*
* @brief  function to save values of the FDB entry into DB in format
*         PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_MAC_ADDR_FORMAT_STC
*         copy values from application format to manager format
*
* @param[in] entryPtr               - (pointer to) the entry (application format)
* @param[in] shadowType             - FDB manager SW shadow table type
* @param[out] dbEntryPtr            - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval none
*/
static GT_VOID saveEntryFormatParamsToDb_fdbEntryMacAddrFormat
(
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC      *entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_ENT                shadowType,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_MAC_ADDR_FORMAT_STC  *dbEntryPtr
)
{
    /* save to DB format from application format  */
    dbEntryPtr->fid                = entryPtr->fid              ;
    dbEntryPtr->daAccessLevel      = entryPtr->daSecurityLevel  ;
    dbEntryPtr->saAccessLevel      = entryPtr->saSecurityLevel  ;
    dbEntryPtr->userDefined        = entryPtr->userDefined      ;
    dbEntryPtr->isStatic           = entryPtr->isStatic         ;
    dbEntryPtr->age                = entryPtr->age              ;
    dbEntryPtr->daRoute            = entryPtr->daRoute          ;
    dbEntryPtr->appSpecificCpuCode = entryPtr->appSpecificCpuCode;
    dbEntryPtr->vid1               = entryPtr->vid1;

    saveEntryFormatParamsToDb_fdbEntryMacAddrFormat_interfaceAndSourceId(entryPtr,dbEntryPtr);

    if((shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E) ||
        (shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E))
    {
        dbEntryPtr->daQosIndex           = entryPtr->daQoSParameterSetIndex;
        dbEntryPtr->saQosIndex           = entryPtr->saQoSParameterSetIndex;
        dbEntryPtr->saCommand            = entryPtr->saCommand;
        dbEntryPtr->daCommand            = entryPtr->daCommand;
        switch(entryPtr->mirrorToAnalyzerPort)
        {
            case CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_DISABLED_E:
                dbEntryPtr->saLookupIngressMirrorToAnalyzerPort = 0;
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_SA_OR_DA_E:
                dbEntryPtr->saLookupIngressMirrorToAnalyzerPort = 1;
                dbEntryPtr->daLookupIngressMirrorToAnalyzerPort = 1;
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_DA_E:
                dbEntryPtr->daLookupIngressMirrorToAnalyzerPort = 1;
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_SA_E:
                dbEntryPtr->saLookupIngressMirrorToAnalyzerPort = 1;
                break;
            default:
                break;
        }
    }
    else
    {
        /* deal with NON- 1:1 field values */
        if(entryPtr->saCommand == CPSS_PACKET_CMD_DROP_HARD_E ||
                entryPtr->saCommand == CPSS_PACKET_CMD_DROP_SOFT_E )
        {
            dbEntryPtr->saCommand = 1;
        }
        else
        {
            dbEntryPtr->saCommand = 0;
        }

        if(entryPtr->daCommand == CPSS_PACKET_CMD_DROP_HARD_E ||
                entryPtr->daCommand == CPSS_PACKET_CMD_DROP_SOFT_E )
        {
            dbEntryPtr->daCommand = 3;/* drop */
        }
        else
        {
            dbEntryPtr->daCommand = entryPtr->daCommand;
        }
    }

    dbEntryPtr->macAddr_low_32 = entryPtr->macAddr.arEther[2] << 24 |
                                 entryPtr->macAddr.arEther[3] << 16 |
                                 entryPtr->macAddr.arEther[4] <<  8 |
                                 entryPtr->macAddr.arEther[5] <<  0 ;

    dbEntryPtr->macAddr_high_16 = entryPtr->macAddr.arEther[0] <<  8 |
                                 entryPtr->macAddr.arEther[1] <<  0 ;


    /* deal with implicit fields */
    dbEntryPtr->valid     = 1;
    dbEntryPtr->skip      = 0;
    dbEntryPtr->spUnknown = 0;

    return;
}

/**
* @internal saveEntryFormatParamsToDb_fdbEntryIpMcFormat_interfaceAndSourceId function
* @endinternal
*
* @brief  function to save interface and Source ID values of the FDB entry into DB in format
*         PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IP_MC_ADDR_FORMAT_STC
*         copy values from application format to manager format
*
* @param[in] entryPtr               - (pointer to) the entry (application format)
* @param[out] dbEntryPtr            - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval none
*/
static GT_VOID saveEntryFormatParamsToDb_fdbEntryIpMcFormat_interfaceAndSourceId
(
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_IP_MC_FORMAT_STC      *entryPtr,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IP_MC_ADDR_FORMAT_STC  *dbEntryPtr
)
{
    /* save to DB format from application format  */
    dbEntryPtr->srcId              = entryPtr->sourceID         ;

    dbEntryPtr->dstInterface_type  = entryPtr->dstInterface.type;
    switch(entryPtr->dstInterface.type)
    {
        case CPSS_INTERFACE_PORT_E:
            dbEntryPtr->dstInterface.devPort.hwDevNum  = entryPtr->dstInterface.interfaceInfo.devPort.hwDevNum;
            dbEntryPtr->dstInterface.devPort.portNum   = entryPtr->dstInterface.interfaceInfo.devPort.portNum ;
            break;
        case CPSS_INTERFACE_TRUNK_E:
            dbEntryPtr->dstInterface.trunkId   = entryPtr->dstInterface.interfaceInfo.trunkId;
            break;
        case CPSS_INTERFACE_VIDX_E:
            dbEntryPtr->dstInterface.vidx      = entryPtr->dstInterface.interfaceInfo.vidx;
            break;
        case CPSS_INTERFACE_VID_E:
            dbEntryPtr->dstInterface.vidx      = FLOOD_VIDX_CNS;
            break;
        default:
            break;
    }

    return;
}

/**
* @internal saveEntryFormatParamsToDb_fdbEntryIpMcFormat function
* @endinternal
*
* @brief  function to save values of the FDB entry into DB in format
*         PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IP_MC_ADDR_FORMAT_STC
*         copy values from application format to manager format
*
* @param[in] entryPtr               - (pointer to) the entry (application format)
* @param[in] shadowType             - FDB manager SW shadow table type
* @param[out] dbEntryPtr            - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval none.
*/
static GT_VOID   saveEntryFormatParamsToDb_fdbEntryIpMcFormat
(
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_IP_MC_FORMAT_STC           *entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_ENT                   shadowType,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IP_MC_ADDR_FORMAT_STC  *dbEntryPtr
)
{
    /* save to DB format from application format  */
    dbEntryPtr->fid                = entryPtr->fid              ;
    dbEntryPtr->daAccessLevel      = entryPtr->daSecurityLevel  ;
    dbEntryPtr->userDefined        = entryPtr->userDefined      ;
    dbEntryPtr->isStatic           = entryPtr->isStatic         ;
    dbEntryPtr->age                = entryPtr->age              ;
    dbEntryPtr->daRoute            = entryPtr->daRoute          ;
    dbEntryPtr->appSpecificCpuCode = entryPtr->appSpecificCpuCode;
    dbEntryPtr->vid1               = entryPtr->vid1;

    saveEntryFormatParamsToDb_fdbEntryIpMcFormat_interfaceAndSourceId(entryPtr,dbEntryPtr);

    if((shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E) ||
        (shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E))
    {
        dbEntryPtr->daQosIndex           = entryPtr->daQoSParameterSetIndex;
        dbEntryPtr->saQosIndex           = entryPtr->saQoSParameterSetIndex;
        dbEntryPtr->daCommand            = entryPtr->daCommand;
        switch(entryPtr->mirrorToAnalyzerPort)
        {
            default:
            case CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_DISABLED_E:
                dbEntryPtr->saLookupIngressMirrorToAnalyzerPort = 0;
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_SA_OR_DA_E:
                dbEntryPtr->saLookupIngressMirrorToAnalyzerPort = 1;
                dbEntryPtr->daLookupIngressMirrorToAnalyzerPort = 1;
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_DA_E:
                dbEntryPtr->daLookupIngressMirrorToAnalyzerPort = 1;
                break;
            case CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_SA_E:
                dbEntryPtr->saLookupIngressMirrorToAnalyzerPort = 1;
                break;
        }
    }
    else
    {
        /* deal with NON- 1:1 field values */
        if(entryPtr->daCommand == CPSS_PACKET_CMD_DROP_HARD_E ||
                entryPtr->daCommand == CPSS_PACKET_CMD_DROP_SOFT_E )
        {
            dbEntryPtr->daCommand = 3;/* drop */
        }
        else
        {
            dbEntryPtr->daCommand = entryPtr->daCommand;
        }
    }

    dbEntryPtr->sipAddr = entryPtr->sipAddr[0] << 24 |
                          entryPtr->sipAddr[1] << 16 |
                          entryPtr->sipAddr[2] <<  8 |
                          entryPtr->sipAddr[3];

    dbEntryPtr->dipAddr = entryPtr->dipAddr[0] << 24 |
                          entryPtr->dipAddr[1] << 16 |
                          entryPtr->dipAddr[2] <<  8 |
                          entryPtr->dipAddr[3];

    /* deal with implicit fields */
    dbEntryPtr->valid     = 1;
    dbEntryPtr->skip      = 0;
    dbEntryPtr->spUnknown = 0;

    return;
}

/**
* @internal prvCpssDxChFdbManager_ip_unicast_CommonInfoSet function
* @endinternal
*
* @brief  function to save values of the FDB entry into DB for next hop common format
*         copy values from application format to manager format
*
* @param[in] entryPtr               - (pointer to) the entry next hop full info (application format)
* @param[out] dbEntryPtr            - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; AC5; Falcon; AC5P; AC5X; Harrier; Ironman;
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   prvCpssDxChFdbManager_ip_unicast_CommonInfoSet
(
    IN CPSS_DXCH_BRG_FDB_MANAGER_IP_UC_COMMON_STC                   *entryPtr,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV_UC_COMMON_FORMAT_STC *dbEntryPtr
)
{
    dbEntryPtr->qosProfilePrecedence            = (entryPtr->qosProfilePrecedence == CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E)?0:1;
    dbEntryPtr->arpBcTrapMirrorEn               = BOOL2BIT_MAC(entryPtr->arpBcTrapMirrorEnable);
    dbEntryPtr->qosProfileMarkingEn             = BOOL2BIT_MAC(entryPtr->qosProfileMarkingEnable);
    dbEntryPtr->qosProfileIndex                 = entryPtr->qosProfileIndex;
    dbEntryPtr->dipAccessLevel                  = entryPtr->dipAccessLevel;
    dbEntryPtr->ingressMirrorToAnalyzerIndex    = (entryPtr->ingressMirrorToAnalyzerEnable)?(entryPtr->ingressMirrorToAnalyzerIndex+1):0;
    PRV_CPSS_DXCH_FDB_CONVERT_ATTRIBUTE_MODIFY_TO_HW_VAL_MAC(dbEntryPtr->modifyUp, entryPtr->modifyUp);
    PRV_CPSS_DXCH_FDB_CONVERT_ATTRIBUTE_MODIFY_TO_HW_VAL_MAC(dbEntryPtr->modifyDscp, entryPtr->modifyDscp);

    return GT_OK;
}

/**
* @internal prvCpssDxChFdbManagerNextHopCommonInfoSet function
* @endinternal
*
* @brief  function to save values of the FDB entry into DB for next hop common format
*         copy values from application format to manager format
*
* @param[in] entryPtr               - (pointer to) the entry next hop full info (application format)
* @param[out] dbEntryPtr            - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; AC5.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   prvCpssDxChFdbManagerNextHopCommonInfoSet
(
    IN CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_STC  *entryPtr,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV_UC_COMMON_FORMAT_STC             *dbEntryPtr
)
{
    /* save to DB format from application format  */
    dbEntryPtr->ttlHopLimitDecEnable            = entryPtr->ttlHopLimitDecEnable;
    dbEntryPtr->ttlHopLimDecOptionsExtChkByPass = entryPtr->ttlHopLimDecOptionsExtChkByPass;
    dbEntryPtr->countSet                        = entryPtr->countSet;
    dbEntryPtr->ICMPRedirectEnable              = entryPtr->ICMPRedirectEnable;
    dbEntryPtr->mtuProfileIndex                 = entryPtr->mtuProfileIndex;
    dbEntryPtr->nextHopVlanId                   = entryPtr->nextHopVlanId;

    return GT_OK;
}

/**
* @internal prvCpssDxChFdbManagerIpUcCommonInfoSet function
* @endinternal
*
* @brief  function to save values of the FDB entry into DB for common format:
*         IP MC
*         copy values from application format to manager format
*
* @param[in] entryPtr               - (pointer to) the next hop entry full info (application format)
* @param[out] dbEntryPtr            - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; AC5.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   prvCpssDxChFdbManagerIpUcCommonInfoSet
(
    IN CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_ENT                ucRouteType,
    IN CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_UNT  *ucRouteInfoPtr,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV_UC_COMMON_FORMAT_STC *ipUcCommonInfoPtr
)
{
    if((ucRouteType != CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_POINTER_TO_ROUTER_E) &&
            (ucRouteType != CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_ECMP_POINTER_TO_ROUTER_E))
    {
        ipUcCommonInfoPtr->dstInterface_type  = ucRouteInfoPtr->fullFdbInfo.dstInterface.type;
        switch(ucRouteInfoPtr->fullFdbInfo.dstInterface.type)
        {
            case CPSS_INTERFACE_PORT_E:
                ipUcCommonInfoPtr->dstInterface.devPort.hwDevNum    = ucRouteInfoPtr->fullFdbInfo.dstInterface.interfaceInfo.devPort.hwDevNum;
                ipUcCommonInfoPtr->dstInterface.devPort.portNum     = ucRouteInfoPtr->fullFdbInfo.dstInterface.interfaceInfo.devPort.portNum;
                break;
            case CPSS_INTERFACE_TRUNK_E:
                ipUcCommonInfoPtr->dstInterface.trunkId             = ucRouteInfoPtr->fullFdbInfo.dstInterface.interfaceInfo.trunkId;
                break;
            case CPSS_INTERFACE_VIDX_E:
                ipUcCommonInfoPtr->dstInterface.vidx                = ucRouteInfoPtr->fullFdbInfo.dstInterface.interfaceInfo.vidx;
                break;
            case CPSS_INTERFACE_VID_E:
                ipUcCommonInfoPtr->dstInterface.vidx                = FLOOD_VIDX_CNS;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                        "Wrong UC Router destination interface type [%d]", ucRouteInfoPtr->fullFdbInfo.dstInterface.type);
        }
    }

    /* save to DB format from application format  */
    switch (ucRouteType)
    {
        case CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E:
            prvCpssDxChFdbManagerNextHopCommonInfoSet(&ucRouteInfoPtr->fullFdbInfo, ipUcCommonInfoPtr);
            ipUcCommonInfoPtr->pointerInfo = ucRouteInfoPtr->fullFdbInfo.pointer.tunnelStartPointer;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_NAT_PTR_E:
            prvCpssDxChFdbManagerNextHopCommonInfoSet(&ucRouteInfoPtr->fullFdbInfo, ipUcCommonInfoPtr);
            ipUcCommonInfoPtr->pointerInfo = ucRouteInfoPtr->fullFdbInfo.pointer.natPointer;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E:
            prvCpssDxChFdbManagerNextHopCommonInfoSet(&ucRouteInfoPtr->fullFdbInfo, ipUcCommonInfoPtr);
            ipUcCommonInfoPtr->pointerInfo = ucRouteInfoPtr->fullFdbInfo.pointer.arpPointer;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_POINTER_TO_ROUTER_E:
            ipUcCommonInfoPtr->pointerInfo = ucRouteInfoPtr->nextHopPointerToRouter;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_ECMP_POINTER_TO_ROUTER_E:
            ipUcCommonInfoPtr->pointerInfo = ucRouteInfoPtr->ecmpPointerToRouter;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Wrong UC Router type [%d]", ucRouteType);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChFdbManagerNextHopCommonInfoGet function
* @endinternal
*
* @brief  function to retrieve values of DB entry into the FDB entry next hop for common format:
*         copy values from manager format to application format
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[out] entryPtr              - (pointer to) the entry next hop full info(application format)
* @param[in] dbEntryPtr             - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; AC5.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   prvCpssDxChFdbManagerNextHopCommonInfoGet
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC               *fdbManagerPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_STC  *entryPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV_UC_COMMON_FORMAT_STC  *dbEntryPtr
)
{
    fdbManagerPtr = fdbManagerPtr;

    /* retrieve to application format from DB format */
    entryPtr->ttlHopLimitDecEnable = dbEntryPtr->ttlHopLimitDecEnable;
    entryPtr->ttlHopLimDecOptionsExtChkByPass = dbEntryPtr->ttlHopLimDecOptionsExtChkByPass;
    entryPtr->countSet = dbEntryPtr->countSet;
    entryPtr->ICMPRedirectEnable = dbEntryPtr->ICMPRedirectEnable;
    entryPtr->mtuProfileIndex = dbEntryPtr->mtuProfileIndex;
    entryPtr->nextHopVlanId = dbEntryPtr->nextHopVlanId;

    return GT_OK;
}

/**
* @internal prvCpssDxChFdbManagerIpUcCommonInfoGet function
* @endinternal
*
* @brief  function to retrieve values from the DB to FDB entry for common format:
*         IP MC
*         copy values from manager format to application format
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[out] entryPtr              - (pointer to) the entry (application format)
* @param[in] dbEntryPtr             - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; AC5.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   prvCpssDxChFdbManagerIpUcCommonInfoGet
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC               *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_ENT                ucRouteType,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV_UC_COMMON_FORMAT_STC  *ipUcCommonInfoPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_UNT *ucRouteInfoPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_IP_UC_COMMON_STC                  *ucCommonInfoPtr
)
{
    fdbManagerPtr = fdbManagerPtr;

    cpssOsMemSet(ucRouteInfoPtr, 0, sizeof(CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_UNT));
    if(ucRouteType <= CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E)
    {
        ucRouteInfoPtr->fullFdbInfo.dstInterface.type  = ipUcCommonInfoPtr->dstInterface_type;
        switch(ucRouteInfoPtr->fullFdbInfo.dstInterface.type)
        {
            case CPSS_INTERFACE_PORT_E:
                ucRouteInfoPtr->fullFdbInfo.dstInterface.interfaceInfo.devPort.hwDevNum  = ipUcCommonInfoPtr->dstInterface.devPort.hwDevNum;
                ucRouteInfoPtr->fullFdbInfo.dstInterface.interfaceInfo.devPort.portNum   = ipUcCommonInfoPtr->dstInterface.devPort.portNum;
                break;
            case CPSS_INTERFACE_TRUNK_E:
                ucRouteInfoPtr->fullFdbInfo.dstInterface.interfaceInfo.trunkId   = ipUcCommonInfoPtr->dstInterface.trunkId;
                break;
            case CPSS_INTERFACE_VIDX_E:
                ucRouteInfoPtr->fullFdbInfo.dstInterface.interfaceInfo.vidx      = ipUcCommonInfoPtr->dstInterface.vidx;
                break;
            case CPSS_INTERFACE_VID_E:
                ucRouteInfoPtr->fullFdbInfo.dstInterface.interfaceInfo.vidx      = FLOOD_VIDX_CNS;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Wrong destination interface type [%d]", ucRouteInfoPtr->fullFdbInfo.dstInterface.type);
        }
    }

    /* retrieve from application format to DB format */
    switch (ucRouteType)
    {
        case CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E:
            prvCpssDxChFdbManagerNextHopCommonInfoGet(fdbManagerPtr,
                                                      &ucRouteInfoPtr->fullFdbInfo,
                                                      ipUcCommonInfoPtr);
            ucRouteInfoPtr->fullFdbInfo.pointer.tunnelStartPointer = ipUcCommonInfoPtr->pointerInfo;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_NAT_PTR_E:
            prvCpssDxChFdbManagerNextHopCommonInfoGet(fdbManagerPtr,
                                                      &ucRouteInfoPtr->fullFdbInfo,
                                                      ipUcCommonInfoPtr);
            ucRouteInfoPtr->fullFdbInfo.pointer.natPointer = ipUcCommonInfoPtr->pointerInfo;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E:
            prvCpssDxChFdbManagerNextHopCommonInfoGet(fdbManagerPtr,
                                                      &ucRouteInfoPtr->fullFdbInfo,
                                                      ipUcCommonInfoPtr);
            ucRouteInfoPtr->fullFdbInfo.pointer.arpPointer = ipUcCommonInfoPtr->pointerInfo;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_POINTER_TO_ROUTER_E:
            ucRouteInfoPtr->nextHopPointerToRouter = ipUcCommonInfoPtr->pointerInfo;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_ECMP_POINTER_TO_ROUTER_E:
            ucRouteInfoPtr->ecmpPointerToRouter = ipUcCommonInfoPtr->pointerInfo;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Wrong UC Router type [%d]", ucRouteType);
    }

    if(fdbManagerPtr->entryAttrInfo.shadowType != CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E)
    {
        return GT_OK;
    }

    /* Parameters related to SIP5 only */
    ucCommonInfoPtr->qosProfileMarkingEnable = BIT2BOOL_MAC(ipUcCommonInfoPtr->qosProfileMarkingEn);
    ucCommonInfoPtr->qosProfileIndex         = ipUcCommonInfoPtr->qosProfileIndex;
    ucCommonInfoPtr->qosProfilePrecedence    = (ipUcCommonInfoPtr->qosProfilePrecedence == 0)?CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E:CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
    PRV_CPSS_DXCH_FDB_CONVERT_HW_VAL_TO_ATTRIBUTE_MODIFY_MAC(ucCommonInfoPtr->modifyUp, ipUcCommonInfoPtr->modifyUp);
    PRV_CPSS_DXCH_FDB_CONVERT_HW_VAL_TO_ATTRIBUTE_MODIFY_MAC(ucCommonInfoPtr->modifyDscp, ipUcCommonInfoPtr->modifyDscp);
    ucCommonInfoPtr->arpBcTrapMirrorEnable   = BIT2BOOL_MAC(ipUcCommonInfoPtr->qosProfileMarkingEn);
    ucCommonInfoPtr->dipAccessLevel          = ipUcCommonInfoPtr->dipAccessLevel;
    if (ipUcCommonInfoPtr->ingressMirrorToAnalyzerIndex)
    {
        ucCommonInfoPtr->ingressMirrorToAnalyzerEnable=GT_TRUE;
        ucCommonInfoPtr->ingressMirrorToAnalyzerIndex = ipUcCommonInfoPtr->ingressMirrorToAnalyzerIndex-1;
    }
    else
    {
        ucCommonInfoPtr->ingressMirrorToAnalyzerEnable=GT_FALSE;
        ucCommonInfoPtr->ingressMirrorToAnalyzerIndex = 0;
    }

    return GT_OK;
}

/**
* @internal saveEntryFormatParamsToDb_fdbEntryIpv4UcFormat function
* @endinternal
*
* @brief  function to save values of the FDB entry into DB in format
*         PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV4_UC_FORMAT_STC
*         copy values from application format to manager format
*
* @param[in] entryPtr               - (pointer to) the entry (application format)
* @param[in] shadowType             - FDB manager SW shadow table type
* @param[out] dbEntryPtr            - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; AC5.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   saveEntryFormatParamsToDb_fdbEntryIpv4UcFormat
(
    IN CPSS_DXCH_BRG_FDB_MANAGER_IPV4_UC_ROUTING_INFO_STC       *entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_ENT                 shadowType,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV4_UC_FORMAT_STC   *dbEntryPtr
)
{
    dbEntryPtr->ipUcCommonInfo.vrfId              = entryPtr->vrfId;
    dbEntryPtr->ipUcCommonInfo.ucRouteExtType     = entryPtr->ucRouteType;

    /* save to DB format from application format  */
    prvCpssDxChFdbManagerIpUcCommonInfoSet(entryPtr->ucRouteType,
                                           &entryPtr->ucRouteInfo,
                                           &dbEntryPtr->ipUcCommonInfo);
    if(shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E)
    {
        prvCpssDxChFdbManager_ip_unicast_CommonInfoSet(&entryPtr->ucCommonInfo,
                &dbEntryPtr->ipUcCommonInfo);
    }

    dbEntryPtr->ipUcCommonInfo.age       = entryPtr->age;

    /* deal with implicit fields */
    dbEntryPtr->ipUcCommonInfo.valid     = 1;
    dbEntryPtr->ipUcCommonInfo.skip      = 0;

    /* deal with implicit fields */
    dbEntryPtr->ipAddr             = entryPtr->ipv4Addr.arIP[0] << 24 |
                                     entryPtr->ipv4Addr.arIP[1] << 16 |
                                     entryPtr->ipv4Addr.arIP[2] <<  8 |
                                     entryPtr->ipv4Addr.arIP[3] <<  0;

    return GT_OK;
}

/**
* @internal saveEntryFormatParamsToDb_fdbEntryIpv6UcFormat function
* @endinternal
*
* @brief  function to save values of the FDB entry into DB in format
*         PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV6_UC_DATA_FORMAT_STC,
*         PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV6_UC_KEY_FORMAT_STC
*         copy values from application format to manager format
*
* @param[in] entryPtr               - (pointer to) the entry (application format)
* @param[in] shadowType             - FDB manager SW shadow table type
* @param[out] dbEntryDataPtr        - (pointer to) the manager data entry format (manager format)
* @param[out] dbEntryDataPtr        - (pointer to) the manager key entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; AC5.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   saveEntryFormatParamsToDb_fdbEntryIpv6UcFormat
(
    IN CPSS_DXCH_BRG_FDB_MANAGER_IPV6_UC_ROUTING_INFO_STC           *entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_ENT                     shadowType,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV6_UC_DATA_FORMAT_STC  *dbEntryDataPtr,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV6_UC_KEY_FORMAT_STC   *dbEntryKeyPtr
)
{
    dbEntryDataPtr->ipUcCommonInfo.vrfId            = entryPtr->vrfId;
    dbEntryDataPtr->ipUcCommonInfo.ucRouteExtType   = entryPtr->ucRouteType;

    /* save to DB format from application format  */
    prvCpssDxChFdbManagerIpUcCommonInfoSet(entryPtr->ucRouteType,
                                           &entryPtr->ucRouteInfo,
                                           &dbEntryDataPtr->ipUcCommonInfo);
    if(shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E)
    {
        prvCpssDxChFdbManager_ip_unicast_CommonInfoSet(&entryPtr->ucCommonInfo,
                &dbEntryDataPtr->ipUcCommonInfo);
    }


    /* deal with implicit fields */
    dbEntryDataPtr->ipAddr_127_106    =  entryPtr->ipv6Addr.arIP[ 0] << 14 |
                                         entryPtr->ipv6Addr.arIP[ 1] << 6  |
                                         entryPtr->ipv6Addr.arIP[ 2] >> 2;           /* total 22 bits */

    dbEntryKeyPtr->ipAddr_105_96      = (entryPtr->ipv6Addr.arIP[ 2] & 0x3) << 8 |
                                         entryPtr->ipv6Addr.arIP[ 3];                /* total 10 bits */

    dbEntryKeyPtr->ipAddr_95_64       =  entryPtr->ipv6Addr.arIP[ 4] << 24 |
                                         entryPtr->ipv6Addr.arIP[ 5] << 16 |
                                         entryPtr->ipv6Addr.arIP[ 6] <<  8 |
                                         entryPtr->ipv6Addr.arIP[ 7];                /* total 32 bits */

    dbEntryKeyPtr->ipAddr_63_32       =  entryPtr->ipv6Addr.arIP[ 8] << 24 |
                                         entryPtr->ipv6Addr.arIP[ 9] << 16 |
                                         entryPtr->ipv6Addr.arIP[10] <<  8 |
                                         entryPtr->ipv6Addr.arIP[11];                /* total 32 bits */

    dbEntryKeyPtr->ipAddr_31_0       =   entryPtr->ipv6Addr.arIP[12] << 24 |
                                         entryPtr->ipv6Addr.arIP[13] << 16 |
                                         entryPtr->ipv6Addr.arIP[14] <<  8 |
                                         entryPtr->ipv6Addr.arIP[15];                /* total 32 bits */

    dbEntryKeyPtr->age                      = entryPtr->age;
    dbEntryDataPtr->ipUcCommonInfo.age      = entryPtr->age;
    dbEntryDataPtr->ipUcCommonInfo.valid    = 1;
    dbEntryDataPtr->ipUcCommonInfo.skip     = 0;

    switch(entryPtr->ucRouteInfo.fullFdbInfo.ipv6ExtInfo.siteId)
    {
        case CPSS_IP_SITE_ID_INTERNAL_E:
            dbEntryDataPtr->ipv6DestSiteId = 0;
            break;
        case CPSS_IP_SITE_ID_EXTERNAL_E:
            dbEntryDataPtr->ipv6DestSiteId = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Wrong Site ID [%d]",
                                          entryPtr->ucRouteInfo.fullFdbInfo.ipv6ExtInfo.siteId);
    }

    dbEntryDataPtr->ipv6ScopeCheck = entryPtr->ucRouteInfo.fullFdbInfo.ipv6ExtInfo.scopeCheckingEnable;

    return GT_OK;
}

/**
* @internal updateEntryFormatParamsToDb_fdbEntryMacAddrFormat function
* @endinternal
*
* @brief  function to update values of the FDB entry into DB in format
*         PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_MAC_ADDR_FORMAT_STC
*         copy values from application format to manager format
*
* @param[in] entryPtr               - (pointer to) the entry (application format)
* @param[in] shadowType             - FDB manager SW shadow table type
* @param[in] paramsPtr              - (pointer to) update manager entry parameters.
* @param[inout] dbEntryPtr            - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS   updateEntryFormatParamsToDb_fdbEntryMacAddrFormat
(
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC          *entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_ENT                    shadowType,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC            *paramsPtr,
    INOUT PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_MAC_ADDR_FORMAT_STC    *dbEntryPtr
)
{
    GT_BOOL updateSourceInterface =
        (paramsPtr && paramsPtr->updateOnlySrcInterface && dbEntryPtr->isStatic == GT_FALSE) ? GT_TRUE : GT_FALSE;

    if (updateSourceInterface)
    {
        saveEntryFormatParamsToDb_fdbEntryMacAddrFormat_interfaceAndSourceId(entryPtr,dbEntryPtr);
        dbEntryPtr->age = 1;    /* Refresh the entry, on entry update */
    }
    else
    {
        saveEntryFormatParamsToDb_fdbEntryMacAddrFormat(entryPtr, shadowType, dbEntryPtr);
    }

    return GT_OK;
}

/**
* @internal updateEntryFormatParamsToDb_fdbEntryIpMcRoutingFormat function
* @endinternal
*
* @brief  function to update values of the FDB entry into DB in format
*         PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IP_MC_ADDR_FORMAT_STC
*         copy values from application format to manager format
*
* @param[in] entryPtr               - (pointer to) the entry (application format)
* @param[in] shadowType             - FDB manager SW shadow table type
* @param[in] paramsPtr              - (pointer to) update manager entry parameters.
* @param[inout] dbEntryPtr            - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS   updateEntryFormatParamsToDb_fdbEntryIpMcRoutingFormat
(
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_IP_MC_FORMAT_STC             *entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_ENT                    shadowType,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC            *paramsPtr,
    INOUT PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IP_MC_ADDR_FORMAT_STC  *dbEntryPtr
)
{
    GT_BOOL updateSourceInterface =
        (paramsPtr && paramsPtr->updateOnlySrcInterface && dbEntryPtr->isStatic == GT_FALSE) ? GT_TRUE : GT_FALSE;

    if (updateSourceInterface)
    {
        saveEntryFormatParamsToDb_fdbEntryIpMcFormat_interfaceAndSourceId(entryPtr,dbEntryPtr);
        dbEntryPtr->age = 1;    /* Refresh the entry, on entry update */
    }
    else
    {
        saveEntryFormatParamsToDb_fdbEntryIpMcFormat(entryPtr, shadowType, dbEntryPtr);
    }

    return GT_OK;
}

/**
* @internal updateEntryFormatParamsToDb_fdbEntryIpV4UcRoutingFormat function
* @endinternal
*
* @brief  function to update values of the FDB entry into DB in format
*         PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV4_UC_FORMAT_STC
*         copy values from application format to manager format
*
* @param[in] entryPtr               - (pointer to) the entry (application format)
* @param[in] shadowType             - FDB manager SW shadow table type
* @param[inout] dbEntryPtr            - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   updateEntryFormatParamsToDb_fdbEntryIpV4UcRoutingFormat
(
    IN CPSS_DXCH_BRG_FDB_MANAGER_IPV4_UC_ROUTING_INFO_STC       *entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_ENT                 shadowType,
    INOUT PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV4_UC_FORMAT_STC *dbEntryPtr
)
{
    return saveEntryFormatParamsToDb_fdbEntryIpv4UcFormat(entryPtr, shadowType, dbEntryPtr);
}

/**
* @internal saveEntryFormatParamsToDb function
* @endinternal
*
* @brief  function to save values of the FDB entry into DB
*         copy values from application format to manager format
*
* @param[in] entryPtr               - (pointer to) the entry (application format)
* @param[in] shadowType             - FDB manager SW shadow table type
* @param[out] dbEntryPtr            - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   saveEntryFormatParamsToDb
(
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_ENT        shadowType,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC   *dbEntryPtr
)
{
    switch(entryPtr->fdbEntryType)
    {
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E:
            dbEntryPtr->hwFdbEntryType = PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E;
            saveEntryFormatParamsToDb_fdbEntryMacAddrFormat(
                &entryPtr->format.fdbEntryMacAddrFormat,
                shadowType,
                &dbEntryPtr->specificFormat.prvMacEntryFormat);
            return GT_OK;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E:
            dbEntryPtr->hwFdbEntryType = PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_UC_E;
            return saveEntryFormatParamsToDb_fdbEntryIpv4UcFormat(
               &entryPtr->format.fdbEntryIpv4UcFormat,
               shadowType,
               &dbEntryPtr->specificFormat.prvIpv4UcEntryFormat);
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E:
            dbEntryPtr->hwFdbEntryType = PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_MCAST_E;
            saveEntryFormatParamsToDb_fdbEntryIpMcFormat(
                &entryPtr->format.fdbEntryIpv4McFormat,
                shadowType,
                &dbEntryPtr->specificFormat.prvIpv4McEntryFormat);
            return GT_OK;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E:
            dbEntryPtr->hwFdbEntryType = PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_MCAST_E;
            saveEntryFormatParamsToDb_fdbEntryIpMcFormat(
                &entryPtr->format.fdbEntryIpv6McFormat,
                shadowType,
                &dbEntryPtr->specificFormat.prvIpv6McEntryFormat);
            return GT_OK;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(entryPtr->fdbEntryType);
    }
}

/**
* @internal saveEntryIpV6UcFormatParamsToDb function
* @endinternal
*
* @brief  function to save values of the FDB entry of type IPv6 UC into DB
*         copy values from application format to manager format
*
* @param[in] entryPtr               - (pointer to) the entry (application format)
* @param[in] shadowType             - FDB manager SW shadow table type
* @param[out] dbEntryPtr            - (pointer to) the manager entry format 'key'  (manager format)
* @param[out] dbEntryPtr            - (pointer to) the manager entry format 'Data' (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; AC5.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   saveEntryIpV6UcFormatParamsToDb
(
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_ENT         shadowType,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC   *dbEntryPtr,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC   *dbEntryExtPtr
)
{
    dbEntryPtr->hwFdbEntryType = PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E;
    dbEntryExtPtr->hwFdbEntryType = PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E;

    return saveEntryFormatParamsToDb_fdbEntryIpv6UcFormat(&entryPtr->format.fdbEntryIpv6UcFormat,
                                                           shadowType,
                                                          &dbEntryExtPtr->specificFormat.prvIpv6UcDataEntryFormat,
                                                          &dbEntryPtr->specificFormat.prvIpv6UcKeyEntryFormat);
}

/**
* @internal updateEntryIpV6UcFormatParamsToDb function
* @endinternal
*
* @brief  function to update values of the FDB entry of type IPv6 UC into DB
*         copy values from application format to manager format
*
* @param[in] entryPtr               - (pointer to) the entry (application format)
* @param[in] shadowType             - FDB manager SW shadow table type
* @param[out] dbEntryPtr            - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; AC5.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   updateEntryIpV6UcFormatParamsToDb
(
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_ENT         shadowType,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC   *dbEntryPtr,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC   *dbEntryExtPtr
)
{
    dbEntryPtr->hwFdbEntryType = PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E;
    dbEntryExtPtr->hwFdbEntryType = PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E;

    return saveEntryFormatParamsToDb_fdbEntryIpv6UcFormat(&entryPtr->format.fdbEntryIpv6UcFormat,
                                                           shadowType,
                                                          &dbEntryExtPtr->specificFormat.prvIpv6UcDataEntryFormat,
                                                          &dbEntryPtr->specificFormat.prvIpv6UcKeyEntryFormat);
}

/**
* @internal updateEntryFormatParamsToDb function
* @endinternal
*
* @brief  function to update values of the FDB entry into DB
*         copy values from application format to manager format
*
* @param[in] entryPtr               - (pointer to) the entry (application format)
* @param[in] shadowType             - FDB manager SW shadow table type
* @param[in] paramsPtr              - (pointer to) update manager entry parameters
* @param[inout] dbEntryPtr          - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS   updateEntryFormatParamsToDb
(
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                  *entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_ENT             shadowType,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC    *paramsPtr,
    INOUT PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC     *dbEntryPtr
)
{
    switch (entryPtr->fdbEntryType)
    {
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E:
            dbEntryPtr->hwFdbEntryType = PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E;
            return updateEntryFormatParamsToDb_fdbEntryMacAddrFormat(
                &entryPtr->format.fdbEntryMacAddrFormat,
                shadowType,
                paramsPtr,
                &dbEntryPtr->specificFormat.prvMacEntryFormat);
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E:
            dbEntryPtr->hwFdbEntryType = PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_MCAST_E;
            return updateEntryFormatParamsToDb_fdbEntryIpMcRoutingFormat(
                &entryPtr->format.fdbEntryIpv4McFormat,
                shadowType,
                paramsPtr,
                &dbEntryPtr->specificFormat.prvIpv4McEntryFormat);
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E:
            dbEntryPtr->hwFdbEntryType = PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_MCAST_E;
            return updateEntryFormatParamsToDb_fdbEntryIpMcRoutingFormat(
                &entryPtr->format.fdbEntryIpv6McFormat,
                shadowType,
                paramsPtr,
                &dbEntryPtr->specificFormat.prvIpv6McEntryFormat);
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E:
            dbEntryPtr->hwFdbEntryType = PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_UC_E;
            return updateEntryFormatParamsToDb_fdbEntryIpV4UcRoutingFormat(
                &entryPtr->format.fdbEntryIpv4UcFormat,
                shadowType,
                &dbEntryPtr->specificFormat.prvIpv4UcEntryFormat);
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(entryPtr->fdbEntryType);
    }
}

/**
* @internal prvCpssDxChFdbManagerDbIsEntryKeyMatchDbEntryKey function
* @endinternal
*
* @brief   The function check if the key of the entry from application (entryPtr)
*          match                 the key of the entry from the manager DB (dbEntryPtr).
*          the result retrieved in (*isMatchPtr)
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager.
* @param[in] entryPtr              - (pointer to) the entry (application format)
*                                     the function actually uses only the 'key' of the entry.
* @param[in] hwFdbEntryType        - the format of the manager entry
* @param[in] dbEntryPtr            - (pointer to) the entry (manager format)
*                                     the function actually uses only the 'key' of the entry.
* @param[out] isMatchPtr           - (pointer to) is the key match ?
*                                    GT_TRUE  - the key     match
*                                    GT_FALSE - the key not match
*
* @retval GT_OK                    - on success
* @retval other                    - unexpected error
*
* @note
*   NONE
*
*/
GT_STATUS prvCpssDxChFdbManagerDbIsEntryKeyMatchDbEntryKey
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC     *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                *entryPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC      *dbEntryPtr,
    OUT GT_BOOL                                           *isMatchPtr
)
{
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_ENT  fdbEntryType;

    *isMatchPtr = GT_FALSE;

    switch(dbEntryPtr->hwFdbEntryType)
    {
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E:
            fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E;
            break;
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_MCAST_E:
            fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E;
            break;
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_MCAST_E:
            fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E;
            break;
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_UC_E:
            fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E;
            break;
        default:
            return GT_OK;
    }

    if(fdbEntryType != entryPtr->fdbEntryType)
    {
        /* the entry type in FDB manager is not the same ... so check next index */
        return GT_OK;
    }

    switch(entryPtr->fdbEntryType)
    {
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E:
            {
                GT_U32 macAddr_low_32,macAddr_high_16;

                macAddr_low_32 =
                    entryPtr->format.fdbEntryMacAddrFormat.macAddr.arEther[2] << 24 |
                    entryPtr->format.fdbEntryMacAddrFormat.macAddr.arEther[3] << 16 |
                    entryPtr->format.fdbEntryMacAddrFormat.macAddr.arEther[4] <<  8 |
                    entryPtr->format.fdbEntryMacAddrFormat.macAddr.arEther[5] <<  0 ;

                if(macAddr_low_32 !=
                   dbEntryPtr->specificFormat.prvMacEntryFormat.macAddr_low_32 )
                {
                    return GT_OK;
                }

                macAddr_high_16 =
                    entryPtr->format.fdbEntryMacAddrFormat.macAddr.arEther[0] <<  8 |
                    entryPtr->format.fdbEntryMacAddrFormat.macAddr.arEther[1] <<  0 ;

                if(macAddr_high_16 !=
                   dbEntryPtr->specificFormat.prvMacEntryFormat.macAddr_high_16 )
                {
                    return GT_OK;
                }

                if(fdbManagerPtr->learningInfo.macVlanLookupMode !=
                        CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_ONLY_E)
                {
                    if(dbEntryPtr->specificFormat.prvMacEntryFormat.fid !=
                            entryPtr->format.fdbEntryMacAddrFormat.fid)
                    {
                        return GT_OK;
                    }
                }

                if(fdbManagerPtr->learningInfo.macVlanLookupMode ==
                   CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_AND_VID1_E)
                {
                    if(dbEntryPtr->specificFormat.prvMacEntryFormat.vid1 !=
                       entryPtr->format.fdbEntryMacAddrFormat.vid1)
                    {
                        return GT_OK;
                    }
                }
            }
            break;

        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E:
            {
                GT_U32 ipAddr =
                    entryPtr->format.fdbEntryIpv4McFormat.sipAddr[0] << 24 |
                    entryPtr->format.fdbEntryIpv4McFormat.sipAddr[1] << 16 |
                    entryPtr->format.fdbEntryIpv4McFormat.sipAddr[2] <<  8 |
                    entryPtr->format.fdbEntryIpv4McFormat.sipAddr[3];

                if (dbEntryPtr->specificFormat.prvIpv4McEntryFormat.sipAddr != ipAddr)
                {
                    return GT_OK;
                }

                ipAddr =
                    entryPtr->format.fdbEntryIpv4McFormat.dipAddr[0] << 24 |
                    entryPtr->format.fdbEntryIpv4McFormat.dipAddr[1] << 16 |
                    entryPtr->format.fdbEntryIpv4McFormat.dipAddr[2] <<  8 |
                    entryPtr->format.fdbEntryIpv4McFormat.dipAddr[3];

                if (dbEntryPtr->specificFormat.prvIpv4McEntryFormat.dipAddr != ipAddr)
                {
                    return GT_OK;
                }

                if(dbEntryPtr->specificFormat.prvIpv4McEntryFormat.fid !=
                   entryPtr->format.fdbEntryIpv4McFormat.fid)
                {
                    return GT_OK;
                }
            }
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E:
            {
                GT_U32 ipAddr =
                    entryPtr->format.fdbEntryIpv6McFormat.sipAddr[0] << 24 |
                    entryPtr->format.fdbEntryIpv6McFormat.sipAddr[1] << 16 |
                    entryPtr->format.fdbEntryIpv6McFormat.sipAddr[2] <<  8 |
                    entryPtr->format.fdbEntryIpv6McFormat.sipAddr[3];
                if (dbEntryPtr->specificFormat.prvIpv6McEntryFormat.sipAddr != ipAddr)
                {
                    return GT_OK;
                }
                ipAddr =
                    entryPtr->format.fdbEntryIpv6McFormat.dipAddr[0] << 24 |
                    entryPtr->format.fdbEntryIpv6McFormat.dipAddr[1] << 16 |
                    entryPtr->format.fdbEntryIpv6McFormat.dipAddr[2] <<  8 |
                    entryPtr->format.fdbEntryIpv6McFormat.dipAddr[3];
                if (dbEntryPtr->specificFormat.prvIpv6McEntryFormat.dipAddr != ipAddr)
                {
                    return GT_OK;
                }
                if(dbEntryPtr->specificFormat.prvIpv6McEntryFormat.fid !=
                   entryPtr->format.fdbEntryIpv6McFormat.fid)
                {
                    return GT_OK;
                }
                break;
            }
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E:
            {
                GT_U32 ipAddr =
                    entryPtr->format.fdbEntryIpv4UcFormat.ipv4Addr.arIP[0] << 24 |
                    entryPtr->format.fdbEntryIpv4UcFormat.ipv4Addr.arIP[1] << 16 |
                    entryPtr->format.fdbEntryIpv4UcFormat.ipv4Addr.arIP[2] <<  8 |
                    entryPtr->format.fdbEntryIpv4UcFormat.ipv4Addr.arIP[3];

                if (dbEntryPtr->specificFormat.prvIpv4UcEntryFormat.ipAddr != ipAddr)
                {
                    return GT_OK;
                }
                if(dbEntryPtr->specificFormat.prvIpv4UcEntryFormat.ipUcCommonInfo.vrfId !=
                   entryPtr->format.fdbEntryIpv4UcFormat.vrfId)
                {
                    return GT_OK;
                }
                break;
            }
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(entryPtr->fdbEntryType);
    }

    *isMatchPtr = GT_TRUE;
    return GT_OK;

}

/**
* @internal prvCpssDxChFdbManagerDbIsIpv6UcEntryKeyMatchDbEntryKey function
* @endinternal
*
* @brief   The function checks is the key of the IPv6 UC entry from application (entryPtr)
*          match                 the key of the IPv6 UC entry from the manager DB (dbEntryPtr).
*          the result retrieved in (*isMatchPtr)
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager.
* @param[in] dbEntryPtr            - (pointer to) the entry (manager format)
*                                     the function actually uses only the 'key' of the entry.
* @param[in] dbEntryExtPtr         - (pointer to) the extended entry (manager format)
*                                     valid for Ipv6 UC entriy or NULL otherwise
* @param[out] isMatchPtr           - (pointer to) is the key match ?
*                                    GT_TRUE  - the key     match
*                                    GT_FALSE - the key not match
*
* @retval GT_OK                    - on success
* @retval other                    - unexpected error
*
* @note
*   NONE
*
*/
GT_STATUS prvCpssDxChFdbManagerDbIsIpv6UcEntryKeyMatchDbEntryKey
(
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                *entryPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC      *dbEntryPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC      *dbEntryExtPtr,
    OUT GT_BOOL                                           *isMatchPtr
)
{
    GT_U32 ipAddr_127_106 =
        entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[ 0] << 14 |
        entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[ 1] << 6  |
        entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[ 2] >> 2;

    GT_U32 ipAddr_105_96 =
        (entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[ 2] & 0x3) << 8 |
         entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[ 3];
    GT_U32 ipAddr_95_64  =
        entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[ 4] << 24 |
        entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[ 5] << 16 |
        entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[ 6] <<  8 |
        entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[ 7];
    GT_U32 ipAddr_63_32  =
        entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[ 8] << 24 |
        entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[ 9] << 16 |
        entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[10] <<  8 |
        entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[11];

    GT_U32 ipAddr_31_0   =
        entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[12] << 24 |
        entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[13] << 16 |
        entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[14] <<  8 |
        entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[15];

    *isMatchPtr = GT_FALSE;

    if (dbEntryPtr->specificFormat.prvIpv6UcKeyEntryFormat.ipAddr_105_96 !=
        ipAddr_105_96)
    {
        return GT_OK;
    }
    if (dbEntryPtr->specificFormat.prvIpv6UcKeyEntryFormat.ipAddr_95_64 !=
        ipAddr_95_64)
    {
        return GT_OK;
    }
    if (dbEntryPtr->specificFormat.prvIpv6UcKeyEntryFormat.ipAddr_63_32 !=
        ipAddr_63_32)
    {
        return GT_OK;
    }
    if (dbEntryPtr->specificFormat.prvIpv6UcKeyEntryFormat.ipAddr_31_0 !=
        ipAddr_31_0)
    {
        return GT_OK;
    }

    if (dbEntryExtPtr->specificFormat.prvIpv6UcDataEntryFormat.ipAddr_127_106 !=
        ipAddr_127_106)
    {
        return GT_OK;
    }

    if(dbEntryExtPtr->specificFormat.prvIpv6UcDataEntryFormat.ipUcCommonInfo.vrfId !=
       entryPtr->format.fdbEntryIpv6UcFormat.vrfId)
    {
        return GT_OK;
    }

    *isMatchPtr = GT_TRUE;

    return GT_OK;

}

/**
* @internal prvCpssDxChFdbManagerCounterPacketTypeGet function
* @endinternal
*
* @brief  function to get counter type for the current packet type
*
* @param[in] entryPtr               - (pointer to) the entry (application format)
*                                       if NULL ignored
* @param[in] dbEntryPtr             - (pointer to) the manager entry format (manager format)
*                                       if NULL ignored
* @param[out] counterTypePtr        - (pointer to) the counter type for the current packet type
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
*
*/
static GT_STATUS   prvCpssDxChFdbManagerCounterPacketTypeGet
(
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC            *entryPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC  *dbEntryPtr,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_ENT   *counterTypePtr
)
{
    GT_BIT      isMacMc;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_ENT fdbEntryType;

    if(entryPtr) /* called from 'add' */
    {
        fdbEntryType = entryPtr->fdbEntryType;
    }
    else         /* called from 'delete' */
    {
        switch(dbEntryPtr->hwFdbEntryType)
        {
            case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E:
                fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_MCAST_E:
                fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_MCAST_E:
                fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_UC_E:
                fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E;
                break;
            case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E:
            case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E:
                fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(dbEntryPtr->hwFdbEntryType);
        }
    }

    switch(fdbEntryType)
    {
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E:
            if(entryPtr)
            {
                isMacMc  = entryPtr->format.fdbEntryMacAddrFormat.macAddr.arEther[0] & 1; /* check bit 40 of the mac address */
                if(isMacMc)
                {
                    *counterTypePtr = entryPtr->format.fdbEntryMacAddrFormat.isStatic == GT_TRUE ?
                        PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_MULTICAST_STATIC_E :
                        PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_MULTICAST_DYNAMIC_E;
                }
                else
                {
                    *counterTypePtr = entryPtr->format.fdbEntryMacAddrFormat.isStatic == GT_TRUE ?
                        PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_UNICAST_STATIC_E :
                        PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_UNICAST_DYNAMIC_E;
                }
            }
            else
            {
                isMacMc  = dbEntryPtr->specificFormat.prvMacEntryFormat.macAddr_high_16 & 0x0100; /* check bit 40 of the mac address */
                if(isMacMc)
                {
                    *counterTypePtr = dbEntryPtr->specificFormat.prvMacEntryFormat.isStatic == GT_TRUE ?
                        PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_MULTICAST_STATIC_E :
                        PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_MULTICAST_DYNAMIC_E;
                }
                else
                {
                    *counterTypePtr = dbEntryPtr->specificFormat.prvMacEntryFormat.isStatic == GT_TRUE ?
                        PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_UNICAST_STATIC_E :
                        PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_UNICAST_DYNAMIC_E;
                }
            }
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E:
            *counterTypePtr = PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_IPV4_UNICAST_ROUTE_E;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E:
            *counterTypePtr = PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_IPV6_UNICAST_ROUTE_KEY_E;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E:
            *counterTypePtr = PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_IPV4_MULTICAST_E;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E:
            *counterTypePtr = PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_IPV6_MULTICAST_E;
            break;
        default:
            break;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChFdbManagerAddEntry function
* @endinternal
*
* @brief   This function adds entry to CPSS FDB Manager's database and HW.
*          NOTE: the FDB manager will set the info to all the registered devices.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager
* @param[in] entryPtr              - (pointer to) FDB entry format to be added
* @param[out] calcInfoPtr          - (pointer to) the calculated hash and lookup results for new entry
* @param[out] addApiStatisticsPtr  - (pointer to) the type of API ok/error statistics
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FULL                  - the table is FULL
* @retval GT_HW_ERROR              - on hardware error
*
* @note
*   NONE
*
*/
static GT_STATUS prvCpssDxChFdbManagerAddEntry
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC           *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                      *entryPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC         *calcInfoPtr,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ENT  *addApiStatisticsPtr
)
{
    GT_STATUS rc;

    /********************************/
    /* get the counter packet type  */
    /********************************/
    rc = prvCpssDxChFdbManagerCounterPacketTypeGet(entryPtr, NULL, &calcInfoPtr->counterType);
    if(rc != GT_OK)
    {
        return rc;
    }

    /******************************************************************************/
    /* adding new entry :                                                         */
    /* update metadata DB :indexArr[] , bankInfoArr[] , typeCountersArr[] */
    /* update headOfFreeList , headOfUsedList , tailOfUsedList                    */
    /* update calcInfoPtr->dbEntryPtr as valid pointer                            */
    /******************************************************************************/
    rc = prvCpssDxChFdbManagerDbAddNewEntry(fdbManagerPtr, GT_TRUE, calcInfoPtr);
    if(rc != GT_OK)
    {
        return rc;
    }
    /* add one new entry for IPv6 UC type */
    if (calcInfoPtr->bankStep > 1)
    {
        calcInfoPtr->counterType = PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_IPV6_UNICAST_ROUTE_DATA_E;
        rc = prvCpssDxChFdbManagerDbAddNewEntry(fdbManagerPtr, GT_FALSE, calcInfoPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /**************************************/
    /* save entry format values to the DB */
    /**************************************/
    if (calcInfoPtr->bankStep > 1)
    {
        rc = saveEntryIpV6UcFormatParamsToDb(entryPtr,
                fdbManagerPtr->entryAttrInfo.shadowType,
                calcInfoPtr->dbEntryPtr,
                calcInfoPtr->dbEntryExtPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        rc = saveEntryFormatParamsToDb(entryPtr,
                fdbManagerPtr->entryAttrInfo.shadowType,
                calcInfoPtr->dbEntryPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /***************************************************************************/
    /* write the entry to HW of all registered devices                         */
    /***************************************************************************/
    rc = prvCpssDxChFdbManagerHwWriteByHwIndex(fdbManagerPtr, calcInfoPtr);
    if(rc != GT_OK)
    {
        *addApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_HW_UPDATE_E;
        return rc;
    }

    /* not failed ... so we can update the 'OK' part */
    *addApiStatisticsPtr =
        (calcInfoPtr->rehashStageId >= 5) ?
            PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_OK_REHASHING_STAGE_5_AND_ABOVE_E :
            PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_OK_REHASHING_STAGE_0_E + calcInfoPtr->rehashStageId;

    return GT_OK;
}

/**
* @internal prvCpssDxChFdbManagerUpdateEntry function
* @endinternal
*
* @brief   This function updates entry to CPSS FDB Manager's database and HW.
*          NOTE: the FDB manager will set the info to all the registered devices.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager
* @param[in] entryPtr              - (pointer to) FDB entry format to be added
* @param[out] calcInfoPtr          - (pointer to) the calculated hash and lookup results for new entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*
* @note
*   NONE
*
*/
static GT_STATUS prvCpssDxChFdbManagerUpdateEntry
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC               *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                          *entryPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC             *calcInfoPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC            *paramsPtr,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ENT   *updateApiStatisticsPtr
)
{
    GT_STATUS rc;
    GT_U32    bankId = calcInfoPtr->selectedBankId;

    /**************************************
      Update entry format values to the DB
    **************************************/
    if (calcInfoPtr->bankStepArr[bankId] > 1)
    {
        rc = updateEntryIpV6UcFormatParamsToDb(entryPtr,
                fdbManagerPtr->entryAttrInfo.shadowType,
                calcInfoPtr->dbEntryPtr,
                calcInfoPtr->dbEntryExtPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        rc = updateEntryFormatParamsToDb(entryPtr,
                fdbManagerPtr->entryAttrInfo.shadowType,
                paramsPtr,
                calcInfoPtr->dbEntryPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /****************************************************
      Write the entry to HW of all registered devices
    ****************************************************/
    rc = prvCpssDxChFdbManagerHwWriteByHwIndex(fdbManagerPtr, calcInfoPtr);
    if(rc != GT_OK)
    {
        *updateApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_HW_UPDATE_E;
        return rc;
    }

    /* not failed ... so we can update the 'OK' part */
    *updateApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_OK_E;

    return GT_OK;
}

/**
* @internal mainLogicEntryAdd_overLimitCheck function
* @endinternal
*
* @brief   The function check if adding new address will cause 'over-the-limit'
*           violation in the 'dynamic-UC-mac' entries.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[in] dbEntryPtr            - (pointer to) FDB Manager entry format to be added.
* @param[in] isAddFromLearningscan - Indicates if add entry called from learning scan.
*                                    GT_TRUE   - Add entry called from learning scan.
*                                    GT_FALSE  - Add entry not called from learning scan.
* @param[out] addApiStatisticsPtr  - (pointer to) the type of API ok/error statistics
*
* @retval GT_OK                    - on success
* @retval GT_LEARN_LIMIT_PORT_ERROR         - if learn limit on port reached.
* @retval GT_LEARN_LIMIT_TRUNK_ERROR        - if learn limit on trunk reached.
* @retval GT_LEARN_LIMIT_GLOBAL_EPORT_ERROR - if learn limit on global eport reached.
* @retval GT_LEARN_LIMIT_FID_ERROR          - if learn limit on fid reached.
* @retval GT_LEARN_LIMIT_GLOBAL_ERROR       - if learn limit globally reached.
*
* @note
*   NONE
*
*/
static GT_STATUS mainLogicEntryAdd_overLimitCheck
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC           *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                      *entryPtr,
    IN GT_BOOL                                                  isAddFromLearningScan,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ENT  *addApiStatisticsPtr
)
{
    fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo.rcError          = GT_OK;
    fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo.limitCountNeeded = GT_FALSE;

    if(entryPtr->fdbEntryType != CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E)
    {
        return GT_OK;
    }

    if(GT_FALSE == prvCpssDxChFdbManagerDbIsOverLimitCheck_applicationFormat(fdbManagerPtr,LIMIT_OPER_ADD_NEW_E,
        &entryPtr->format.fdbEntryMacAddrFormat))
    {
        return GT_OK;
    }

    switch(fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo.rcError)
    {
        case GT_LEARN_LIMIT_PORT_ERROR:
            *addApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_PORT_LIMIT_E;
            break;
        case GT_LEARN_LIMIT_TRUNK_ERROR:
            *addApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_TRUNK_LIMIT_E;
            break;
        case GT_LEARN_LIMIT_GLOBAL_EPORT_ERROR:
            *addApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_GLOBAL_EPORT_LIMIT_E;
            break;
        case GT_LEARN_LIMIT_FID_ERROR:
            *addApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_FID_LIMIT_E;
            break;
        case GT_LEARN_LIMIT_GLOBAL_ERROR:
            *addApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_GLOBAL_LIMIT_E;
            break;
        default:
            /* should not get here */
            *addApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_BAD_STATE_E;
            break;
    }


    /* the dynamic UC mac entry is limited by one of the limits : global/port/global-eport/trunk/fid
       we are not allowed to add it into the table.
    */
    if(isAddFromLearningScan)
    {
        return /* not error for the LOG */ fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo.rcError;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo.rcError,
                "the new entry exceed the dynamic Uc Mac Limit");
    }
}

/**
* @internal mainLogicEntryUpdate_overLimitCheck function
* @endinternal
*
* @brief   The function check if updating an address will cause 'over-the-limit'
*           violation in the 'dynamic-UC-mac' entries.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[in] dbEntryPtr            - (pointer to) FDB Manager entry format of the old entry info (shadow format).
* @param[in] newEntrySwPtr         - (pointer to) FDB entry format of the new entry info (application format) .
* @param[in] isUpdateFromLearningScan - Indicates if add entry called from learning scan.
*                                    GT_TRUE   - update entry called from learning scan.
*                                    GT_FALSE  - update entry not called from learning scan.
* @param[out] updateApiStatisticsPtr  - (pointer to) the type of API ok/error statistics
*
* @retval GT_OK                    - on success
* @retval GT_LEARN_LIMIT_PORT_ERROR         - if learn limit on port reached.
* @retval GT_LEARN_LIMIT_TRUNK_ERROR        - if learn limit on trunk reached.
* @retval GT_LEARN_LIMIT_GLOBAL_EPORT_ERROR - if learn limit on global eport reached.
* @retval GT_LEARN_LIMIT_FID_ERROR          - if learn limit on fid reached.
* @retval GT_LEARN_LIMIT_GLOBAL_ERROR       - if learn limit globally reached.
*
* @note
*   NONE
*
*/
static GT_STATUS mainLogicEntryUpdate_overLimitCheck
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC    *dbEntryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *newEntrySwPtr,
    IN GT_BOOL                                                  isUpdateFromLearningScan,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ENT  *updateApiStatisticsPtr
)
{

    if(dbEntryPtr->hwFdbEntryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E)
    {
        /* get to fill info about the old entry , into:
            fdbManagerPtr->dynamicUcMacLimitPtr->oldInterfaceLimitInfo */
        (void)prvCpssDxChFdbManagerDbIsOverLimitCheck(fdbManagerPtr,
            LIMIT_OPER_INTERFACE_UPDATE_OLD_PART_INFO_GET_E,
            &dbEntryPtr->specificFormat.prvMacEntryFormat);
    }
    else
    {
        fdbManagerPtr->dynamicUcMacLimitPtr->oldInterfaceLimitInfo.rcError          = GT_OK;
        fdbManagerPtr->dynamicUcMacLimitPtr->oldInterfaceLimitInfo.limitCountNeeded = GT_FALSE;
    }

    if(newEntrySwPtr->fdbEntryType != CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E)
    {
        fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo.rcError          = GT_OK;
        fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo.limitCountNeeded = GT_FALSE;
        return GT_OK;
    }

    /* get to fill info about the new entry ,into:
        fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo */
    if(GT_FALSE == prvCpssDxChFdbManagerDbIsOverLimitCheck_applicationFormat(fdbManagerPtr,
        LIMIT_OPER_INTERFACE_UPDATE_NEW_PART_INFO_GET_E,
        &newEntrySwPtr->format.fdbEntryMacAddrFormat))
    {
        return GT_OK;
    }

    switch(fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo.rcError)
    {
        case GT_LEARN_LIMIT_PORT_ERROR:
            *updateApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_PORT_LIMIT_E;
            break;
        case GT_LEARN_LIMIT_TRUNK_ERROR:
            *updateApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_TRUNK_LIMIT_E;
            break;
        case GT_LEARN_LIMIT_GLOBAL_EPORT_ERROR:
            *updateApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_GLOBAL_EPORT_LIMIT_E;
            break;
        case GT_LEARN_LIMIT_FID_ERROR:
            *updateApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_FID_LIMIT_E;
            break;
        case GT_LEARN_LIMIT_GLOBAL_ERROR:
        default:
            /* should not get here */
            *updateApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_BAD_STATE_E;
            break;
    }


    /* the dynamic UC mac entry is limited by one of the limits : global/port/global-eport/trunk/fid
       we are not allowed to update it in the table.
    */
    if(isUpdateFromLearningScan)
    {
        return /* not error for the LOG */ fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo.rcError;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo.rcError,
                "the new entry exceed the dynamic Uc Mac Limit");
    }
}

/**
* @internal prvCpssDxChFdbManagerUpdateEntryAndCounter function
* @endinternal
*
* @brief  function to update DB and counter if required
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[in] calcInfoPtr           - (pointer to) information about entry to delete
* @param[in] entryPtr              - (pointer to) the entry (application format)
* @param[in] paramsPtr             - (pointer to) extra info related to 'update entry' operation
* @param[out] updateApiStatisticsPtr
*                                  - (pointer to) the type of the API ok/error statistics
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_LEARN_LIMIT_PORT_ERROR         - if learn limit on port reached.
* @retval GT_LEARN_LIMIT_TRUNK_ERROR        - if learn limit on trunk reached.
* @retval GT_LEARN_LIMIT_GLOBAL_EPORT_ERROR - if learn limit on global eport reached.
* @retval GT_LEARN_LIMIT_FID_ERROR          - if learn limit on fid reached.
* @retval GT_LEARN_LIMIT_GLOBAL_ERROR       - if learn limit globally reached.
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChFdbManagerUpdateEntryAndCounter
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC               *fdbManagerPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC             *calcInfoPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                          *entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC            *paramsPtr,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ENT   *updateApiStatisticsPtr,
    IN GT_BOOL                                                      isUpdateFromLearningscan
)
{
    PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_ENT lastCounterType;
    GT_STATUS                                   rc,rc1;

    if(NULL != fdbManagerPtr->dynamicUcMacLimitPtr)
    {
        /* check if the updated entry not violate any 'limit'
           and get info about the limits that relate to the old entry and those of
           the new entry.
        */
        rc = mainLogicEntryUpdate_overLimitCheck(fdbManagerPtr,
            calcInfoPtr->dbEntryPtr,/* the 'old entry' */
            entryPtr,/* the new entry */
            isUpdateFromLearningscan,
            updateApiStatisticsPtr);
        if(rc != GT_OK)
        {
            /* delete the old entry from HW and SW */
            /* it will also decrement 'limit counters' on the old interface (if applicable) */
            rc1 = prvCpssDxChFdbManagerDbEntryDelete(fdbManagerPtr,calcInfoPtr->dbEntryPtr);
            if(rc1 != GT_OK)
            {
                return rc1;
            }

            /* already got dedicated 'statistics' value ! */
            return rc;
        }
    }

    /* store last counter type */
    rc = prvCpssDxChFdbManagerCounterPacketTypeGet(NULL,calcInfoPtr->dbEntryPtr,&lastCounterType);
    if(rc != GT_OK)
    {
        return rc;
    }

    /**************************************
      Update entry format values to the DB and HW
    **************************************/
    rc = prvCpssDxChFdbManagerUpdateEntry(fdbManagerPtr, entryPtr, calcInfoPtr, paramsPtr, updateApiStatisticsPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /**************************************************************************************************
      Counters update.
      The only case the API should update the counters is when static entry overwrites dynamic entry -
      in all other cases counters should not be updated at all.
    **************************************************************************************************/
    rc = prvCpssDxChFdbManagerCounterPacketTypeGet(entryPtr, NULL, &calcInfoPtr->counterType);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* The update entry type is MAC static unicast */
    if (calcInfoPtr->counterType == PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_UNICAST_STATIC_E)
    {

        /* The DB entry type was MAC dynamic unicast */
        if (lastCounterType == PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_UNICAST_DYNAMIC_E)
        {
            /********************************
              Update the typeCountersArr
            ********************************/
            fdbManagerPtr->typeCountersArr[PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_UNICAST_STATIC_E].entryTypePopulation++;
            /* coherency check -- can't be ZERO because we are going to decrement it */
            CHECK_X_NOT_ZERO_MAC(fdbManagerPtr->typeCountersArr[PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_UNICAST_DYNAMIC_E].entryTypePopulation);
            fdbManagerPtr->typeCountersArr[PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_MAC_UNICAST_DYNAMIC_E].entryTypePopulation--;
        }
    }

    /* 1. add    the new entry to   its relevant limits
       2. remove the old entry from its relevant limits
    */
    rc = prvCpssDxChFdbManagerDbLimitOper(fdbManagerPtr,LIMIT_OPER_INTERFACE_UPDATE_E);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

void prvCpssDxChFdbManagerDebugPrintEnableSet(GT_U32 enable)
{
    PRV_SHARED_FDB_MANAGER_DB_VAR_GET(reHashDebugPrint) = (enable) ? GT_TRUE : GT_FALSE;
}

/**
* @internal prvCpssDxChFdbManagerUpdateFdbEntry function
* @endinternal
*
* @brief    This function updates relocated regular and mixed entries.
*
*   If source entry X and destination entry Y[i] have different formats, the entry Y[i] will be deleted,
*   then the source entry X will be added to index Y[j].
*   X --> Y[j](Y[i]) --> Deleted
*   Example 1:  IPv6 -> MAC - MAC entry ocupies one bank but has one free left or right neighbor bank.
*               If the rightmost bank is free, the IPv6 key will occupy the bank of the deleted MAC and the IPv6 data the next free bank,
*               else the IPv6 key will occupy the free bank and the IPv6 data the bank of the deleted MAC.
*  
*   Example 2:  MAC -> IPv6 - IPv6 entry occipies two banks, so  MAC entry will occupy the first bank of the deleted IPv6, and the second bank will remain free. 
*  
*   If source and destination entry have the same format, the destination entry will be updated by the source.
*   X --> Y[i]
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB manager
* @param[in] currentEntryPtr        - (pointer to) current FDB entry
* @param[in] currentCalcInfoPtr     - (pointer to) the calculated hash and lookup results for current FDB entry
* @param[out] addApiStatisticsPtr   - (pointer to) the type of API ok/error statistics
*
* @retval GT_OK                     - on success
* @retval GT_FAIL                   - on fail
*
*
* @note The function updates mixed entries in DB and HW 
*       To prevent traffic loss, the FDB entry should be moved to other bank before update/delete.
*/
static GT_STATUS prvCpssDxChFdbManagerUpdateRelocatedFdbEntry
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                     *currentEntryPtr,
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC        *currentCalcInfoPtr,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ENT  *addApiStatisticsPtr
)
{
    GT_STATUS                                                   rc;
    GT_U32                                                      entryBankStep;
    PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ENT   updateApiStatistics;
    PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_DELETE_API_STATISTICS_ENT   deleteApiStatistics;
    GT_U32                                                      bankId = currentCalcInfoPtr->selectedBankId;
    PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_ENT                 oldCounterType;
    PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_ENT                 newCounterType;

    if (currentEntryPtr->fdbEntryType == CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E)
    {
        /* Two banks original */
        entryBankStep = 2;
    }
    else
    {
        /* One bank original */
        entryBankStep = 1;
    }

    currentCalcInfoPtr->bankStep = currentCalcInfoPtr->bankStepArr[bankId];

    /* Mixed entries formats - first delete the relocated entry, after add the new one */
    if(entryBankStep != currentCalcInfoPtr->bankStep)
    {
        rc = prvCpssDxChFdbManagerEntryDelete(fdbManagerPtr, currentCalcInfoPtr, &deleteApiStatistics);
        if(deleteApiStatistics <
            PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_DELETE_API_STATISTICS___LAST___E)
        {
            /* increment the 'API statistics' */
            fdbManagerPtr->apiEntryDeleteStatisticsArr[deleteApiStatistics]++;
        }
        if (rc != GT_OK)
        {
            return rc;
        }
        /* Assign new format values */
        if(entryBankStep > 1)
        {
            currentCalcInfoPtr->bankStep = entryBankStep;
            /* For IPv6 entry align bank to start from the even index */
            currentCalcInfoPtr->selectedBankId = (currentCalcInfoPtr->selectedBankId % 2) ? currentCalcInfoPtr->selectedBankId - 1 : currentCalcInfoPtr->selectedBankId; 
        }

        rc = prvCpssDxChFdbManagerAddEntry(fdbManagerPtr, currentEntryPtr, currentCalcInfoPtr, addApiStatisticsPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        rc = prvCpssDxChFdbManagerCounterPacketTypeGet(NULL, currentCalcInfoPtr->dbEntryPtr, &oldCounterType);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDxChFdbManagerCounterPacketTypeGet(currentEntryPtr, NULL, &newCounterType);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Override current entry with the previous one */
        rc = prvCpssDxChFdbManagerUpdateEntry(fdbManagerPtr, currentEntryPtr, currentCalcInfoPtr, NULL, &updateApiStatistics);
        if(updateApiStatistics <
            PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS___LAST___E)
        {
            /* increment the 'API statistics' */
            fdbManagerPtr->apiEntryUpdateStatisticsArr[updateApiStatistics]++;
        }
        if (rc != GT_OK)
        {
            return rc;
        }
        /* Update counter - in case of replaced entry type and new entry type do not match */
        if (newCounterType != oldCounterType)
        {
            fdbManagerPtr->typeCountersArr[newCounterType].entryTypePopulation++;
            /* coherency check -- can't be ZERO because we are going to decrement it */
            CHECK_X_NOT_ZERO_MAC(fdbManagerPtr->typeCountersArr[oldCounterType].entryTypePopulation);
            fdbManagerPtr->typeCountersArr[oldCounterType].entryTypePopulation--;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChFdbManagerRelocateEntries function
* @endinternal
*
* @brief    This function relocates entries and free space to be used for new added entry.
*
*   Entry at index Y will be copied to index Z, then new entry X will be copied to index Y
*   so traffic of existing entries wouldn’t be loss and new entry could be added by using index X.
*
*   New entry X --> Y[i] -->Z[j]
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB manager
* @param[in] totalStages            - number of rehashing stages
* @param[in] entryPtr               - (pointer to) new FDB entry
* @param[in] calcInfoPtr            - (pointer to) the calculated hash and lookup results for new entry
* @param[out] addApiStatisticsPtr   - (pointer to) the type of API ok/error statistics
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*
*
* @note The function relocates entries in DB and HW
*/
static GT_STATUS   prvCpssDxChFdbManagerRelocateEntries
(
    IN   PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN   GT_U32                                                  totalStages,
    IN   CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                     *entryPtr,
    IN   PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC        *calcInfoPtr,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ENT   *addApiStatisticsPtr
)
{
    GT_STATUS                                                   rc;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                         * currentEntryPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC            * currentCalcInfoPtr;
    GT_U32                                                      ii;

    /* Traverse rehashing array starting from the last element */
    for (ii = 0; ii < totalStages; ii++)
    {
        /* Get the entry to be relocated */
        currentEntryPtr     = &fdbManagerPtr->rehashingTraceArray[totalStages - 1 - ii].entry;
        /* Get relocation info relevant to the entry */
        currentCalcInfoPtr  = &fdbManagerPtr->rehashingTraceArray[totalStages - 1 - ii].calcInfo;

#ifdef FDB_MANAGER_REHASHING_DEBUG
        if (PRV_SHARED_FDB_MANAGER_DB_VAR_GET(reHashDebugPrint)) 
        {
            cpssOsPrintf("-------------------------Start Relocation---------------------------------\n");
            cpssOsPrintf("-------------------------Before---------------------------------\n");
            rc = prvCpssDxChBrgFdbManagerRehashInfoPrint(fdbManagerPtr, currentEntryPtr, currentCalcInfoPtr, totalStages - ii, currentCalcInfoPtr->rehashStageBank);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
#endif
        /* Start from the last entry */
        if (ii == 0)
        {
            /* Add the last entry to the free place */
            rc = prvCpssDxChFdbManagerAddEntry(fdbManagerPtr, currentEntryPtr, currentCalcInfoPtr, addApiStatisticsPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        else
        {
            rc = prvCpssDxChFdbManagerUpdateRelocatedFdbEntry(fdbManagerPtr, currentEntryPtr, currentCalcInfoPtr, addApiStatisticsPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

#ifdef FDB_MANAGER_REHASHING_DEBUG
        if (PRV_SHARED_FDB_MANAGER_DB_VAR_GET(reHashDebugPrint)) 
        {
            cpssOsPrintf("-------------------------After---------------------------------\n");
            rc = prvCpssDxChBrgFdbManagerRehashInfoPrint(fdbManagerPtr, currentEntryPtr, currentCalcInfoPtr, totalStages - ii, currentCalcInfoPtr->selectedBankId);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
#endif
    }

#ifdef FDB_MANAGER_REHASHING_DEBUG
    if (PRV_SHARED_FDB_MANAGER_DB_VAR_GET(reHashDebugPrint)) 
    {
        cpssOsPrintf("-------------------------Before---------------------------------\n");
        rc = prvCpssDxChBrgFdbManagerRehashInfoPrint(fdbManagerPtr, entryPtr, calcInfoPtr, totalStages - ii, calcInfoPtr->rehashStageBank);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
#endif
    rc = prvCpssDxChFdbManagerUpdateRelocatedFdbEntry(fdbManagerPtr, entryPtr, calcInfoPtr, addApiStatisticsPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
#ifdef FDB_MANAGER_REHASHING_DEBUG
    if (PRV_SHARED_FDB_MANAGER_DB_VAR_GET(reHashDebugPrint))
    {
        cpssOsPrintf("-------------------------After---------------------------------\n");

        rc = prvCpssDxChBrgFdbManagerRehashInfoPrint(fdbManagerPtr, entryPtr, calcInfoPtr, totalStages - ii, calcInfoPtr->selectedBankId);
        if (rc != GT_OK)
        {
            return rc;
        }
        cpssOsPrintf("-------------------------End Relocation---------------------------------\n\n\n\n\n");
    }
#endif
    return rc;
}

/**
* @internal prvCpssDxChFdbManagerDbRehashMacEntry function
* @endinternal
*
* @brief   This function re-hash index for the new entry that can't be added to HW
*          when all banks for the current hash index are occupied.
*
*          NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB manager.
* @param[in] entryPtr              - (pointer to) new FDB entry
* @param[in] calcInfoPtr           - (pointer to) the calculated hash and lookup results
* @param[out] addApiStatisticsPtr  - (pointer to) the type of API ok/error statistics
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on operation fail
*
*/
static GT_STATUS   prvCpssDxChFdbManagerDbRehashMacEntry
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr,
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC *calcInfoPtr,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ENT  *addApiStatisticsPtr
)
{
    GT_STATUS  rc;

    if (PRV_SHARED_FDB_MANAGER_DB_VAR_GET(reHashDebugPrint))
    {
        rc = prvCpssDxChBrgFdbManagerRehashInfoPrint(fdbManagerPtr, entryPtr, calcInfoPtr, 0, -1);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* Rehash FDB entry in the current bank starting from the first stage */
    rc = prvCpssDxChFdbManagerDbRehashEntry(fdbManagerPtr, 1, calcInfoPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Free place found in DB */
    if (calcInfoPtr->rehashStageId)
    {
        /* Relocate existing FDB entries and add the new one */
        return prvCpssDxChFdbManagerRelocateEntries(fdbManagerPtr, calcInfoPtr->rehashStageId, entryPtr, calcInfoPtr,  addApiStatisticsPtr);
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL,
        "the entry not exists and not able to add it (trying 'Cuckoo') ");
}

/**
* @internal mainLogicEntryAdd function
* @endinternal
*
* @brief   This function adding entry to CPSS FDB Manager's database and HW.
*          All types of entries can be added using this API.
*          NOTE: the FDB manager will set the info to all the registered devices.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[in] entryPtr              - (pointer to) FDB entry format to be added.
* @param[in] paramsPtr             - (pointer to) extra info related to 'add entry' operation.
* @param[in] inputHwIndex          - Indicates exact hwindex.
* @param[in] isInputHwIndexValid   - Indicates if inputHwIndex is valid or not.
*                                    GT_TRUE  - inputHwIndex is valid.
*                                    GT_FALSE - inputHwIndex is not valid.
* @param[in] isAddFromLearningscan - Indicates if add entry called from learning scan.
*                                    GT_TRUE   - Add entry called from learning scan.
*                                    GT_FALSE  - Add entry not called from learning scan.
* @param[out] addApiStatisticsPtr  - (pointer to) the type of API ok/error statistics
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_ALREADY_EXIST         - the entry already exists.
* @retval GT_FULL                  - the table is FULL.
* @retval GT_LEARN_LIMIT_PORT_ERROR         - if learn limit on port reached.
* @retval GT_LEARN_LIMIT_TRUNK_ERROR        - if learn limit on trunk reached.
* @retval GT_LEARN_LIMIT_GLOBAL_EPORT_ERROR - if learn limit on global eport reached.
* @retval GT_LEARN_LIMIT_FID_ERROR          - if learn limit on fid reached.
* @retval GT_LEARN_LIMIT_GLOBAL_ERROR       - if learn limit globally reached.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*
*/
static GT_STATUS mainLogicEntryAdd
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC           *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                      *entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC           *paramsPtr,
    IN GT_U32                                                   inputHwIndex,
    IN GT_BOOL                                                  isInputHwIndexValid,
    IN GT_BOOL                                                  isAddFromLearningscan,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ENT  *addApiStatisticsPtr
)
{
    GT_STATUS                                           rc;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC    calcInfo;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC    *calcInfoPtr = &calcInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC           updateParams;
    PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ENT   updateApiStatistics;

    *addApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_INPUT_INVALID_E;
    /* check if the entry is supported by the current device */
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);
    CPSS_NULL_PTR_CHECK_MAC(paramsPtr);
    FDB_MANAGER_SUPPORTED_ENTRY_CHECK(fdbManagerPtr, entryPtr->fdbEntryType);
    if(IS_FDB_MANAGER_FOR_SIP_4(fdbManagerPtr) &&
            (paramsPtr->rehashEnable == GT_TRUE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Rehash is not supported");
    }

    cpssOsMemSet(&calcInfo, 0, sizeof(PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC));
    if(paramsPtr->tempEntryExist == GT_TRUE)
    {
        if (paramsPtr->tempEntryOffset >= fdbManagerPtr->numOfBanks)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                "The bank ID[%d] is out of the acceptable range of hashes[%d]",
                                          paramsPtr->tempEntryOffset,
                                          fdbManagerPtr->numOfBanks);
        }
        if(entryPtr->fdbEntryType != CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                    "'SP unknown' entriy is relevant for MAC entry type");
        }
    }

    *addApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS___LAST___E;

    if(!fdbManagerPtr->headOfFreeList.isValid_entryPointer)
    {
        rc = prvCpssDxChFdbManagerDbTableFullDiagnostic(fdbManagerPtr);

        if(rc == GT_FULL)
        {
            *addApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_TABLE_FULL_E;
            /* HW creates tempEntry, if SP bit set, even if FDB manager reached the max limit (SW Limit)
             * In this case:
             *      tempEntryExist == TRUE  --- tempEntry need to be deleted (Done in the down logic, after lookUp)
             *      tempEntryExist == FALSE --- Ignore and return
             */
            if(paramsPtr->tempEntryExist == GT_FALSE)
            {
                return rc;
            }
        }
        else
        {
            /* unexpected error : bad state */
            *addApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_BAD_STATE_E;
            return rc;
        }
    }

    /********************************/
    /* validity checks of SW format */
    /********************************/
    rc = prvCpssDxChFdbManagerValidateEntryFormatParams(fdbManagerPtr,entryPtr);
    if(rc != GT_OK)
    {
        *addApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_INPUT_INVALID_E;
        return rc;
    }

    /*************************************/
    /* calculate where to save in the DB */
    /*************************************/
    /* Do not calculate the hwIndex for all banks if bankId and hwIndex is already provided
     * Specific case where caller already aware of the hwindex (e.g FDB Manager HA)
     */
    if(isInputHwIndexValid)
    {
        calcInfoPtr->calcEntryType     = PRV_CPSS_DXCH_FDB_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_FREE_E;
        calcInfoPtr->selectedBankId    = paramsPtr->tempEntryOffset;
        calcInfoPtr->crcMultiHashArr[calcInfo.selectedBankId]   = inputHwIndex;
        if(entryPtr->fdbEntryType == CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E)
        {
            calcInfo.bankStep = 2;
            calcInfoPtr->crcMultiHashArr[calcInfo.selectedBankId+1] = inputHwIndex+1;
        }
    }
    else
    {
        rc = prvCpssDxChFdbManagerDbCalcHashAndLookupResult(fdbManagerPtr,entryPtr,paramsPtr,&calcInfo);
        if(rc != GT_OK)
        {
            /* should not fail ! so if failed this is bad state */
            *addApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_BAD_STATE_E;
            return rc;
        }
    }

    if (calcInfo.calcEntryType == PRV_CPSS_DXCH_FDB_MANAGER_DB_CALC_ENTRY_TYPE_NOT_FOUND_AND_NO_FREE_E)
    {
        if (paramsPtr->rehashEnable)
        {
            if(NULL != fdbManagerPtr->dynamicUcMacLimitPtr)
            {
                /* check if the new entry not violate any 'limit' */
                rc = mainLogicEntryAdd_overLimitCheck(fdbManagerPtr,entryPtr,
                    isAddFromLearningscan,addApiStatisticsPtr);
                if(rc != GT_OK)
                {
                    /* already got dedicated 'statistics' value ! */
                    return rc;
                }
            }

            /* reset rehashing info */
            cpssOsMemSet(fdbManagerPtr->rehashingTraceArray, 0, sizeof(fdbManagerPtr->rehashingTraceArray));

            rc = prvCpssDxChFdbManagerDbRehashMacEntry(fdbManagerPtr, entryPtr, &calcInfo, addApiStatisticsPtr);
            if(rc != GT_OK)
            {
                if (rc == GT_FULL)
                {
                    *addApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_TABLE_FULL_E;
                }
                else
                {
                    /* should not fail ! so if failed this is bad state */
                    *addApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_BAD_STATE_E;
                }
                return rc;
            }
            return GT_OK;
        }
    }

    if (calcInfo.calcEntryType == PRV_CPSS_DXCH_FDB_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_EXISTS_E)
    {
        /** relevant for MAC address entries only */
        if(paramsPtr->tempEntryExist == GT_TRUE)
        {
            if(paramsPtr->tempEntryOffset == calcInfo.selectedBankId)
            {
                /* This can happen if for example we get stream of NA of same MAC -
                   as it takes HW time to install SP entry. */
            }
            else
            {
                if(!calcInfo.usedBanksArr[paramsPtr->tempEntryOffset])
                {
                    /* Free entry in tempEntryOffset */
                    /* Typical case when processing 1st NA of MAC notified by HW It's
                       possible that for example till we processed that NA,
                       CPU added same entry (possibly static) but in different bank.
                    */

                    /* found: Flush tempEntryOffset and return error already-exist */
                    rc = prvCpssDxChFdbManagerHwFlushByHwIndex(fdbManagerPtr,
                        calcInfo.crcMultiHashArr[paramsPtr->tempEntryOffset]/*hwIndex*/,
                        paramsPtr->tempEntryOffset,/* bankId */
                        GT_FALSE);/* we not update the bank counters in HW */
                    if(rc != GT_OK)
                    {
                        *addApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_HW_UPDATE_E;
                        return rc;
                    }

                }
                else
                {
                    /* Other entry in tempEntryOffset */
                    /* This can happen if till we processed that NA, CPU or other
                       tile used that index for other entry */
                }
            }
        }

        if(isAddFromLearningscan && IS_FDB_MANAGER_FOR_SIP_4(fdbManagerPtr))
        {
            /**************************************
              Update entry format values to the DB and HW
             **************************************/
            updateParams.updateOnlySrcInterface = GT_TRUE;
            rc = prvCpssDxChFdbManagerUpdateEntryAndCounter(fdbManagerPtr, &calcInfo, entryPtr, &updateParams, &updateApiStatistics,
                isAddFromLearningscan);
            if(rc != GT_OK)
            {
                return rc;
            }

            if(updateApiStatistics < PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS___LAST___E)
            {
                /* increment the 'API statistics' */
                fdbManagerPtr->apiEntryUpdateStatisticsArr[updateApiStatistics]++;
            }
            return GT_OK;
        }
        else
        {
            *addApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_ENTRY_EXIST_E;
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST,
                    "the entry already exists : at hwIndex[%d] (for 'update' entry use other API!) ",
                    calcInfo.dbEntryPtr->hwIndex);
        }
    }
    else
    if(calcInfo.calcEntryType == PRV_CPSS_DXCH_FDB_MANAGER_DB_CALC_ENTRY_TYPE_NOT_FOUND_AND_NO_FREE_E)
    {
        *addApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_TABLE_FULL_E;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL,
            "the entry not exists and not able to add it (before trying 'Cuckoo') ");
    }
    else
    {
        /* SW limit exceed, Delete the temporary created by HW     - Delete temp entry */
        if(*addApiStatisticsPtr == PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_TABLE_FULL_E)
        {
            rc = prvCpssDxChFdbManagerHwFlushByHwIndex(fdbManagerPtr,
                    calcInfo.crcMultiHashArr[paramsPtr->tempEntryOffset]/*hwIndex*/,
                    paramsPtr->tempEntryOffset,/* bankId */
                    GT_FALSE);/* we not update the bank counters in HW */
            if(rc != GT_OK)
            {
                *addApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_HW_UPDATE_E;
                return rc;
            }
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL, LOG_ERROR_NO_MSG);
        }
    }

    if(NULL != fdbManagerPtr->dynamicUcMacLimitPtr)
    {
        /* check if the new entry not violate any 'limit' */
        rc = mainLogicEntryAdd_overLimitCheck(fdbManagerPtr,entryPtr,
            isAddFromLearningscan,addApiStatisticsPtr);
        if(rc != GT_OK)
        {
            GT_STATUS rc1;
            /* If API included indication that tempEntryExist
               (meaning there is SP entry at tempEntryOffset and its indeed
               free in our database – we'll have to flush it from HW –
               so it would not stall that entry index */
            if(paramsPtr->tempEntryExist == GT_TRUE &&
               !calcInfo.usedBanksArr[paramsPtr->tempEntryOffset])
            {
                /* found: Flush tempEntryOffset and return error already-exist */
                rc1 = prvCpssDxChFdbManagerHwFlushByHwIndex(fdbManagerPtr,
                    calcInfo.crcMultiHashArr[paramsPtr->tempEntryOffset]/*hwIndex*/,
                    paramsPtr->tempEntryOffset,/* bankId */
                    GT_FALSE);/* we not update the bank counters in HW */
                if(rc1 != GT_OK)
                {
                    *addApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_HW_UPDATE_E;
                    return rc1;
                }
            }

            /* already got dedicated 'statistics' value ! */
            return rc;
        }
    }

    /* from this point ... should not fail ! so if failed this is bad state */
    *addApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_BAD_STATE_E;

    /*********************************************/
    /* choose the best bank to use for the entry */
    /*********************************************/
    if(paramsPtr->tempEntryExist == GT_TRUE &&
      calcInfo.usedBanksArr[paramsPtr->tempEntryOffset] == 0/* not already used index ! */)
    {
        /* the caller specify specific bankId to be used (got it from the 'NA' message) */
        /* If not found use index at tempEntryOffset (overriding temp entry). */
        calcInfo.selectedBankId = paramsPtr->tempEntryOffset;
    }
    else if(!isInputHwIndexValid)
    {
        /* If not found use free entry at the most populated bank (or Cuckoo if none). */
        rc = prvCpssDxChFdbManagerDbGetMostPopulatedBankId(fdbManagerPtr,&calcInfo);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /******************************************************************************/
    /* adding new entry to DB and HW                                              */
    /******************************************************************************/
    return prvCpssDxChFdbManagerAddEntry(fdbManagerPtr, entryPtr, &calcInfo, addApiStatisticsPtr);
}

static GT_STATUS prvCpssDxChFdbManagerThresholdSet
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr
)
{
    GT_STATUS                                           rc;
    GT_U8                                               devNum;
    PRV_CPSS_DXCH_BRG_FDB_MANAGER_THRESHOLD_TYPE_ENT    threshold;

    /* Threshold is applicable only if macNoSpace is enabled */
    if(!fdbManagerPtr->learningInfo.macNoSpaceUpdatesEnable)
    {
        return GT_OK;
    }

    /* Threshold calculation */
    if ((fdbManagerPtr->totalPopulation <= fdbManagerPtr->thresholdB) &&
            (fdbManagerPtr->currentThresholdType != PRV_CPSS_DXCH_BRG_FDB_MANAGER_THRESHOLD_TYPE_A_E))
    {
        threshold = PRV_CPSS_DXCH_BRG_FDB_MANAGER_THRESHOLD_TYPE_A_E;
    }
    else if ((fdbManagerPtr->totalPopulation <= fdbManagerPtr->thresholdC) &&
             (fdbManagerPtr->totalPopulation > fdbManagerPtr->thresholdB) &&
             (fdbManagerPtr->currentThresholdType != PRV_CPSS_DXCH_BRG_FDB_MANAGER_THRESHOLD_TYPE_B_E))
    {
        threshold = PRV_CPSS_DXCH_BRG_FDB_MANAGER_THRESHOLD_TYPE_B_E;
    }
    else if((fdbManagerPtr->totalPopulation > fdbManagerPtr->thresholdC) &&
            (fdbManagerPtr->currentThresholdType != PRV_CPSS_DXCH_BRG_FDB_MANAGER_THRESHOLD_TYPE_C_E))
    {
        threshold = PRV_CPSS_DXCH_BRG_FDB_MANAGER_THRESHOLD_TYPE_C_E;
    }
    else
    {
        return GT_OK;
    }

    rc = prvCpssDxChFdbManagerDbDeviceIteratorGetNext(fdbManagerPtr, GT_TRUE, &devNum);
    while(rc == GT_OK)
    {
        prvCpssDxChFdbManagerLearningThrottlingSet(devNum, threshold);
        rc = prvCpssDxChFdbManagerDbDeviceIteratorGetNext(fdbManagerPtr, GT_FALSE, &devNum);
    }

    /* Update the threshold, once the fine tune parameters configured in HW */
    fdbManagerPtr->currentThresholdType = threshold;
    return GT_OK;
}

/**
* @internal internal_cpssDxChBrgFdbManagerEntryAdd function
* @endinternal
*
* @brief   This function adding entry to CPSS FDB Manager's database and HW.
*          All types of entries can be added using this API.
*          NOTE: the FDB manager will set the info to all the registered devices.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entryPtr              - (pointer to) FDB entry format to be added.
* @param[in] paramsPtr             - (pointer to) extra info related to 'add entry' operation.
* @param[in] isAddFromLearningscan - Indicates if add entry called from learning scan.
*                                    GT_TRUE   - Add entry called from learning scan.
*                                    GT_FALSE  - Add entry not called from learning scan.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_ALREADY_EXIST         - the entry already exists.
* @retval GT_FULL                  - the table is FULL.
* @retval GT_LEARN_LIMIT_PORT_ERROR         - if learn limit on port reached.
* @retval GT_LEARN_LIMIT_TRUNK_ERROR        - if learn limit on trunk reached.
* @retval GT_LEARN_LIMIT_GLOBAL_EPORT_ERROR - if learn limit on global eport reached.
* @retval GT_LEARN_LIMIT_FID_ERROR          - if learn limit on fid reached.
* @retval GT_LEARN_LIMIT_GLOBAL_ERROR       - if learn limit globally reached.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*
*/
static GT_STATUS internal_cpssDxChBrgFdbManagerEntryAdd
(
    IN GT_U32                                           fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   *paramsPtr,
    IN GT_BOOL                                           isAddFromLearningscan
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ENT    addApiStatistics;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC            *fdbManagerPtr;

    FDB_MANAGER_ID_CHECK(fdbManagerId); /* must be done here because used in 'API' statistics */

    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    /* call the main logic */
    rc = mainLogicEntryAdd(fdbManagerPtr,entryPtr,paramsPtr,0,GT_FALSE,isAddFromLearningscan,&addApiStatistics);

    if(addApiStatistics <
        PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS___LAST___E)
    {
        /* increment the 'API statistics' */
        fdbManagerPtr->apiEntryAddStatisticsArr[addApiStatistics]++;
    }
    if(rc != GT_OK)
    {
        return rc;
    }

    return prvCpssDxChFdbManagerThresholdSet(fdbManagerPtr);
}


/**
* @internal cpssDxChBrgFdbManagerEntryAdd function
* @endinternal
*
* @brief   This function adding entry to CPSS FDB Manager's database and HW.
*          All types of entries can be added using this API.
*          NOTE: the FDB manager will set the info to all the registered devices.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entryPtr              - (pointer to) FDB entry format to be added.
* @param[in] paramsPtr             - (pointer to) extra info related to 'add entry' operation.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_ALREADY_EXIST         - the entry already exists.
* @retval GT_FULL                  - the table is FULL.
* @retval GT_LEARN_LIMIT_PORT_ERROR         - if learn limit on port reached.
* @retval GT_LEARN_LIMIT_TRUNK_ERROR        - if learn limit on trunk reached.
* @retval GT_LEARN_LIMIT_GLOBAL_EPORT_ERROR - if learn limit on global eport reached.
* @retval GT_LEARN_LIMIT_FID_ERROR          - if learn limit on fid reached.
* @retval GT_LEARN_LIMIT_GLOBAL_ERROR       - if learn limit globally reached.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*
*/
GT_STATUS cpssDxChBrgFdbManagerEntryAdd
(
    IN GT_U32                                           fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   *paramsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbManagerEntryAdd);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, fdbManagerId, entryPtr, paramsPtr));

    rc = internal_cpssDxChBrgFdbManagerEntryAdd(fdbManagerId, entryPtr, paramsPtr, GT_FALSE);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, fdbManagerId, entryPtr, paramsPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}

/**
* @internal prvCpssDxChFdbManagerEntryDelete function
* @endinternal
*
* @brief  function to deletes the entry in all devices(HW & DB)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerPtr           - (pointer to) the FDB Manager.
* @param[in] calcInfoPtr             - (pointer to) information about entry to delete
* @param[out] deleteApiStatisticsPtr - (pointer to) the type of API ok/error statistics
*
* @retval GT_OK                      - on success
* @retval GT_BAD_PARAM               - on wrong parameters
* @retval GT_BAD_PTR                 - on NULL pointer.
* @retval GT_NOT_FOUND               - the entry was not found.
* @retval GT_HW_ERROR                - on hardware error
* @retval GT_NOT_INITIALIZED         - if the FDB Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE   - on not applicable device
*/
static GT_STATUS prvCpssDxChFdbManagerEntryDelete
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC               *fdbManagerPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC             *calcInfoPtr,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_DELETE_API_STATISTICS_ENT   *deleteApiStatisticsPtr
)
{
    GT_STATUS                                        rc;

    /********************************/
    /* get the counter packet type  */
    /********************************/
    /* use the entry that we found in the DB ... not the one given by application , because the application only gave 'key' fields */
    rc = prvCpssDxChFdbManagerCounterPacketTypeGet(NULL,calcInfoPtr->dbEntryPtr,&calcInfoPtr->counterType);
    if(rc != GT_OK)
    {
        return rc;
    }

    /******************************************************************************/
    /* delete old entry :                                                         */
    /* update metadata DB :indexArr[] , bankInfoArr[] , typeCountersArr[] */
    /* update headOfFreeList , headOfUsedList , tailOfUsedList                    */
    /* update calcInfoPtr->dbEntryPtr about valid pointers                        */
    /* lastGetNextInfo , lastTranslplantInfo , lastDeleteInfo                     */
    /******************************************************************************/
    rc = prvCpssDxChFdbManagerDbDeleteOldEntry(fdbManagerPtr, GT_TRUE, calcInfoPtr);
    if(rc != GT_OK)
    {
        return rc;
    }
    /* delete one more entry for IPv6 UC type */
    if (calcInfoPtr->bankStep > 1)
    {
        calcInfoPtr->counterType = PRV_CPSS_DXCH_FDB_MANAGER_COUNTERS_TYPE_IPV6_UNICAST_ROUTE_DATA_E;
        rc = prvCpssDxChFdbManagerDbDeleteOldEntry(fdbManagerPtr, GT_FALSE, calcInfoPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /***************************************************************************/
    /* flush the entry from HW of all registered devices                       */
    /***************************************************************************/
    rc = prvCpssDxChFdbManagerHwFlushByHwIndex(fdbManagerPtr,
        calcInfoPtr->crcMultiHashArr[calcInfoPtr->selectedBankId]/*hwIndex*/,
        calcInfoPtr->selectedBankId,/* bankId */
        GT_TRUE);/* we update the bank counters in HW */
    if(rc != GT_OK)
    {
        *deleteApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_DELETE_API_STATISTICS_ERROR_HW_UPDATE_E;
        return rc;
    }
    if (calcInfoPtr->bankStep > 1)
    {
        rc = prvCpssDxChFdbManagerHwFlushByHwIndex(fdbManagerPtr,
            calcInfoPtr->crcMultiHashArr[calcInfoPtr->selectedBankId+1]/*hwIndex*/,
            calcInfoPtr->selectedBankId+1,/* bankId */
            GT_TRUE);/* we update the bank counters in HW */
        if(rc != GT_OK)
        {
            *deleteApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_DELETE_API_STATISTICS_ERROR_HW_UPDATE_E;
            return rc;
        }
    }

    *deleteApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_DELETE_API_STATISTICS_OK_E;

    /* Flush the FDB manager no space cache at the event of delete */
    if(fdbManagerPtr->learningInfo.macNoSpaceUpdatesEnable)
    {
        /* No Need to reset memory, usedEntriesCnt=0, is sufficient to indicate cache is not used */
        fdbManagerPtr->noSpaceCacheEntry.usedEntriesCnt = 0;
    }
    return GT_OK;
}

/**
* @internal mainLogicEntryDelete function
* @endinternal
*
* @brief   This function delete entry from the CPSS FDB Manager's database and HW.
*          All types of entries can be deleted using this API.
*          NOTE: the FDB manager will remove the info from all the registered devices.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerPtr           - (pointer to) the FDB Manager.
* @param[in] entryPtr                - (pointer to) FDB entry format to be deleted.
*                                      NOTE: only the 'fdbEntryKey' part is used by the API.
* @param[out] deleteApiStatisticsPtr - (pointer to) the type of API ok/error statistics
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_FOUND             - the entry was not found.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*
*/
static GT_STATUS mainLogicEntryDelete
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC               *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                          *entryPtr,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_DELETE_API_STATISTICS_ENT   *deleteApiStatisticsPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC calcInfo;

    *deleteApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_DELETE_API_STATISTICS_ERROR_INPUT_INVALID_E;
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    *deleteApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_DELETE_API_STATISTICS___LAST___E;

    /********************************/
    /* validity checks of SW format */
    /********************************/
    rc = prvCpssDxChFdbManagerValidateEntryKeyFormatParams(fdbManagerPtr,entryPtr);
    if(rc != GT_OK)
    {
        *deleteApiStatisticsPtr =
            PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_DELETE_API_STATISTICS_ERROR_INPUT_INVALID_E;

        return rc;
    }

    /* from this point ... should not fail ! so if failed this is bad state */
    *deleteApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_DELETE_API_STATISTICS_ERROR_BAD_STATE_E;

    /*******************************************/
    /* calculate where 'key' is expected in DB */
    /*******************************************/
    rc = prvCpssDxChFdbManagerDbCalcHashAndLookupResult(fdbManagerPtr,entryPtr,NULL,&calcInfo);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(calcInfo.calcEntryType != PRV_CPSS_DXCH_FDB_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_EXISTS_E)
    {
        *deleteApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_DELETE_API_STATISTICS_ERROR_ENTRY_NOT_FOUND_E;

        /* the 'key' was not found in the DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND,LOG_ERROR_NO_MSG);
    }
    return prvCpssDxChFdbManagerEntryDelete(fdbManagerPtr, &calcInfo, deleteApiStatisticsPtr);
}

/**
* @internal internal_cpssDxChBrgFdbManagerEntryDelete function
* @endinternal
*
* @brief   This function delete entry from the CPSS FDB Manager's database and HW.
*          All types of entries can be deleted using this API.
*          NOTE: the FDB manager will remove the info from all the registered devices.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entryPtr              - (pointer to) FDB entry format to be deleted.
*                                     NOTE: only the 'fdbEntryKey' part is used by the API.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_FOUND             - the entry was not found.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*
*/
static GT_STATUS internal_cpssDxChBrgFdbManagerEntryDelete
(
    IN GT_U32                                           fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_DELETE_API_STATISTICS_ENT deleteApiStatistics;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC *fdbManagerPtr;

    FDB_MANAGER_ID_CHECK(fdbManagerId); /* must be done here because used in 'API' statistics */

    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    /* call the main logic */
    rc = mainLogicEntryDelete(fdbManagerPtr, entryPtr, &deleteApiStatistics);

    if(deleteApiStatistics < PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_DELETE_API_STATISTICS___LAST___E)
    {
        /* increment the 'API statistics' */
        fdbManagerPtr->apiEntryDeleteStatisticsArr[deleteApiStatistics]++;
    }
    if(rc != GT_OK)
    {
        return rc;
    }

    return prvCpssDxChFdbManagerThresholdSet(fdbManagerPtr);
}

/**
* @internal cpssDxChBrgFdbManagerEntryDelete function
* @endinternal
*
* @brief   This function delete entry from the CPSS FDB Manager's database and HW.
*          All types of entries can be deleted using this API.
*          NOTE: the FDB manager will remove the info from all the registered devices.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entryPtr              - (pointer to) FDB entry format to be deleted.
*                                     NOTE: only the 'fdbEntryKey' part is used by the API.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_FOUND             - the entry was not found.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*
*/
GT_STATUS cpssDxChBrgFdbManagerEntryDelete
(
    IN GT_U32                                           fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbManagerEntryDelete);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, fdbManagerId, entryPtr));

    rc = internal_cpssDxChBrgFdbManagerEntryDelete(fdbManagerId, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, fdbManagerId, entryPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}

/**
* @internal prvCpssDxChFdbManagerEntryTempDeleteKeyMatchCheck function
* @endinternal
*
* @brief   This function matches key parameters of the incoming entry with HW values.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[in] entryPtr              - (pointer to) FDB entry format to be deleted.
*                                        NOTE: only the 'fdbEntryKey' part is used by the API.
* @param[in] hwFormatType          - FDB Entry hw format type.
* @param[out] isMatchedPtr         - (pointer to) matching status
*                                       GT_TRUE  - is valid & matched, should be flushed
*                                       GT_FALSE - is not valid or not matched, should not be flushed
* @note
*   NONE
*
*/
static GT_STATUS prvCpssDxChFdbManagerEntryTempDeleteKeyMatchCheck
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC               *fdbManagerPtr,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC          *entryPtr,
    IN  PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_ENT              hwFormatType,
    OUT GT_BOOL                                                      *isMatchedPtr
)
{
    GT_U32          *hwDataArr = &fdbManagerPtr->tempInfo.sip6_hwFormatWords[0];
    GT_U32           hwValue;
    GT_BOOL          isSkip, isValid, isSp;
    GT_U32           vid1 = 0;
    GT_ETHERADDR     macAddr;

    *isMatchedPtr = GT_FALSE;
    /* Check Entry should be Valid,
     * Skip bit should be Disabled,
     * SP bit should be True */
    switch(hwFormatType)
    {
        case PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP4_E:
            isValid = U32_GET_FIELD_MAC(hwDataArr[0],0,1);
            isSkip  = U32_GET_FIELD_MAC(hwDataArr[0],1,1);
            isSp    = U32_GET_FIELD_MAC(hwDataArr[3],2,1);
            if(!isValid || isSkip || !isSp)
            {
                return GT_OK;
            }
            macAddr.arEther[5] = (GT_U8)U32_GET_FIELD_MAC(hwDataArr[0], 17, 8);
            macAddr.arEther[4] = (GT_U8)((U32_GET_FIELD_MAC(hwDataArr[0], 25, 7)) |
                    (U32_GET_FIELD_MAC(hwDataArr[1], 0, 1) << 7));
            macAddr.arEther[3] = (GT_U8)U32_GET_FIELD_MAC(hwDataArr[1], 1,  8);
            macAddr.arEther[2] = (GT_U8)U32_GET_FIELD_MAC(hwDataArr[1], 9,  8);
            macAddr.arEther[1] = (GT_U8)U32_GET_FIELD_MAC(hwDataArr[1], 17, 8);
            macAddr.arEther[0] = (GT_U8)((U32_GET_FIELD_MAC(hwDataArr[1], 25, 7)) |
                    (U32_GET_FIELD_MAC(hwDataArr[2], 0, 1) << 7));
            if(cpssOsMemCmp(&entryPtr->macAddr, &macAddr, sizeof(macAddr)) != 0)
            {
                return GT_OK;
            }

            if(fdbManagerPtr->learningInfo.macVlanLookupMode !=
                    CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_ONLY_E)
            {
                hwValue = U32_GET_FIELD_MAC(hwDataArr[0], 5, 12);
                FDB_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(entryPtr->fid, hwValue);
            }
            break;

        case PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP6_E:
            SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_SKIP_E, hwValue);
            isSkip = BIT2BOOL_MAC(hwValue);
            SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_VALID_E, hwValue);
            isValid = BIT2BOOL_MAC(hwValue);
            SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_SP_UNKNOWN_E, hwValue);
            isSp = BIT2BOOL_MAC(hwValue);
            if(!isValid || isSkip || !isSp)
            {
                return GT_OK;
            }
            /* MAC address */
            SIP6_FDB_HW_ENTRY_FIELD_MAC_ADDR_GET_MAC(hwDataArr, macAddr.arEther);
            if(cpssOsMemCmp(&entryPtr->macAddr, &macAddr, sizeof(macAddr)) != 0)
            {
                return GT_OK;
            }
            /* FID */
            if(fdbManagerPtr->learningInfo.macVlanLookupMode !=
                    CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_ONLY_E)
            {
                SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_FID_E, hwValue);
                FDB_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(entryPtr->fid, hwValue);
            }
            /* VID1 */
            if(fdbManagerPtr->learningInfo.macVlanLookupMode ==
                    CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_AND_VID1_E)
            {
                SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_ORIG_VID1_0_E, hwValue);
                U32_SET_FIELD_MAC(vid1, 0, 1, hwValue);

                SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_ORIG_VID1_6_1_E, hwValue);
                U32_SET_FIELD_MAC(vid1, 1, 6, hwValue);

                SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_ORIG_VID1_11_7_E, hwValue);
                U32_SET_FIELD_MAC(vid1, 7, 5, hwValue);

                FDB_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(entryPtr->vid1, vid1);
            }
            break;

        case PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP5_E:
        case PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP5_20_E:
            SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP5_FDB_FDB_TABLE_FIELDS_SKIP_E, hwValue);
            isSkip = BIT2BOOL_MAC(hwValue);
            SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP5_FDB_FDB_TABLE_FIELDS_VALID_E, hwValue);
            isValid = BIT2BOOL_MAC(hwValue);
            SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP5_FDB_FDB_TABLE_FIELDS_SP_UNKNOWN_E, hwValue);
            isSp = BIT2BOOL_MAC(hwValue);
            if(!isValid || isSkip || !isSp)
            {
                return GT_OK;
            }
            /* MAC address */
            SIP5_FDB_HW_ENTRY_FIELD_MAC_ADDR_GET_MAC(hwDataArr, macAddr.arEther);
            if(cpssOsMemCmp(&entryPtr->macAddr, &macAddr, sizeof(macAddr)) != 0)
            {
                return GT_OK;
            }
            /* FID */
            if(fdbManagerPtr->learningInfo.macVlanLookupMode !=
                    CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_ONLY_E)
            {
                SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP5_FDB_FDB_TABLE_FIELDS_FID_E, hwValue);
                FDB_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(entryPtr->fid, hwValue);
            }
            /* VID1 */
            if(fdbManagerPtr->learningInfo.macVlanLookupMode ==
                    CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_AND_VID1_E)
            {
                SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP5_FDB_FDB_TABLE_FIELDS_ORIG_VID1_E, hwValue);
                FDB_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(entryPtr->vid1, hwValue);
            }
            break;
        default:
            break;
    }

   *isMatchedPtr = GT_TRUE;
    return GT_OK;
}

/**
* @internal mainLogicEntryTempDelete function
* @endinternal
*
* @brief   This function delete temporary entry from the HW.
*          NOTE: the FDB manager will remove only if:
*           - The entry is temp entry
*           - Only present in HW (FDB Manager same index should be free)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerPtr              - (pointer to) the FDB Manager.
* @param[in] entryPtr                   - (pointer to) FDB entry format to be deleted.
*                                             NOTE: only the 'fdbEntryKey' part is used by the API.
* @param[in] tempEntryOffset            - Temporary entry offset in Hardware for the SP entry.
* @param[in] tempDeleteApiStatisticsPtr - (pointer to) the type of API ok/error statistics.
*
* @retval GT_OK                         - on success
* @retval GT_BAD_PARAM                  - on wrong parameters
* @retval GT_BAD_PTR                    - on NULL pointer.
* @retval GT_NOT_FOUND                  - the entry was not found.
* @retval GT_HW_ERROR                   - on hardware error
* @retval GT_NOT_INITIALIZED            - if the FDB Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE      - on not applicable device
*
* @note
*   NONE
*
*/
static GT_STATUS mainLogicEntryTempDelete
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC                    *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                               *entryPtr,
    IN GT_U32                                                             tempEntryOffset,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_TEMP_DELETE_API_STATISTICS_ENT   *tempDeleteApiStatisticsPtr
)
{
    GT_STATUS                                           rc;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC    calcInfo;
    GT_U32                                              *hwDataArr = &fdbManagerPtr->tempInfo.sip6_hwFormatWords[0];
    GT_BOOL                                             isMatched;
    GT_U8                                               devNum;
    PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_ENT    hwFormatType;

    *tempDeleteApiStatisticsPtr =
        PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_TEMP_DELETE_API_STATISTICS_ERROR_INPUT_INVALID_E;
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    /********************************/
    /* validity checks of SW format */
    /********************************/
    rc = prvCpssDxChFdbManagerValidateEntryKeyFormatParams(fdbManagerPtr, entryPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Bank ID - Validity check
     * Only MAC entry is expected - Other types will not have temporary entry */
    if(tempEntryOffset > fdbManagerPtr->numOfBanks ||
            entryPtr->fdbEntryType != CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    *tempDeleteApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_TEMP_DELETE_API_STATISTICS___LAST___E;

    /*******************************************/
    /* calculate where 'key' is expected in DB */
    /*******************************************/
    /* calculate hash for FDB entry */
    rc = prvCpssDxChFdbManagerDbHashCalculate(fdbManagerPtr, entryPtr, &calcInfo);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChFdbManagerDbDeviceIteratorGetNext(fdbManagerPtr, GT_TRUE, &devNum);
    if(rc != GT_OK)
    {
        /* In case of GT_NO_MORE also return failure to indicate HA synch expecting at least one device */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChFdbManagerDeviceHwFormatGet(fdbManagerPtr, devNum, &hwFormatType);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Get from HW entry using: tempEntryOffset, calcInfo
     * Read entries from HW (1st PP, 1st Tile) */
    LOCK_DEV_NUM(devNum);
    rc = prvCpssDxChPortGroupReadTableEntry(devNum,
            0,                                              /*portGroupId - 0, indicates 1st PP's 1st tile*/
            CPSS_DXCH_TABLE_FDB_E,
            calcInfo.crcMultiHashArr[tempEntryOffset],  /* hwIndex */
            hwDataArr);
    UNLOCK_DEV_NUM(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }
    rc = prvCpssDxChFdbManagerEntryTempDeleteKeyMatchCheck(fdbManagerPtr,
            &entryPtr->format.fdbEntryMacAddrFormat,
            hwFormatType,
            &isMatched);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* flush the index: if SP entry found */
    if(isMatched == GT_TRUE)
    {
        /***************************************************************************/
        /* flush the entry from HW of all registered devices                       */
        /***************************************************************************/
        rc = prvCpssDxChFdbManagerHwFlushByHwIndex(fdbManagerPtr,
                calcInfo.crcMultiHashArr[tempEntryOffset]   /* hwIndex */,
                tempEntryOffset,                            /* bankId */
                GT_FALSE);                                  /* SP Entry - no need to update counter */
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    *tempDeleteApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_TEMP_DELETE_API_STATISTICS_OK_E;
    return rc;
}

/**
* @internal internal_cpssDxChBrgFdbManagerEntryTempDelete function
* @endinternal
*
* @brief   This function deletes temporary entry from the HW.
*          NOTE: the FDB manager will remove if:
*           - The entry is temp entry
*           - Only present in HW (FDB Manager same index should be free)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entryPtr              - (pointer to) FDB entry format to be deleted.
*                                     NOTE: only the 'fdbEntryKey' part is used by the API.
* @param[in] tempEntryOffset       - Temporary entry offset in Hardware for the SP entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_FOUND             - the entry was not found.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*
*/
static GT_STATUS internal_cpssDxChBrgFdbManagerEntryTempDelete
(
    IN GT_U32                                           fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr,
    IN GT_U32                                           tempEntryOffset
)
{
    GT_STATUS                                                       rc;
    PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_TEMP_DELETE_API_STATISTICS_ENT  tempDeleteApiStatistics;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC                  *fdbManagerPtr;

    FDB_MANAGER_ID_CHECK(fdbManagerId); /* must be done here because used in 'API' statistics */

    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    /* call the main logic */
    rc = mainLogicEntryTempDelete(fdbManagerPtr, entryPtr, tempEntryOffset, &tempDeleteApiStatistics);

    if(tempDeleteApiStatistics < PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_TEMP_DELETE_API_STATISTICS___LAST___E)
    {
        /* increment the 'API statistics' */
        fdbManagerPtr->apiEntryTempDeleteStatisticsArr[tempDeleteApiStatistics]++;
    }
    return rc;
}

/**
* @internal cpssDxChBrgFdbManagerEntryTempDelete function
* @endinternal
*
* @brief   This function deletes temporary entry from the HW.
*          NOTE: the FDB manager will remove if:
*           - The entry is temp entry
*           - Only present in HW (FDB Manager same index should be free)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entryPtr              - (pointer to) FDB entry format to be deleted.
*                                     NOTE: only the 'fdbEntryKey' part is used by the API.
* @param[in] tempEntryOffset       - Temporary entry offset in Hardware for the SP entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_FOUND             - the entry was not found.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*
*/
GT_STATUS cpssDxChBrgFdbManagerEntryTempDelete
(
    IN GT_U32                                           fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr,
    IN GT_U32                                           tempEntryOffset
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbManagerEntryTempDelete);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, fdbManagerId, entryPtr, tempEntryOffset));

    rc = internal_cpssDxChBrgFdbManagerEntryTempDelete(fdbManagerId, entryPtr, tempEntryOffset);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, fdbManagerId, entryPtr, tempEntryOffset));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}


/**
* @internal duplicationsInDevListCheck function
* @endinternal
*
* @brief  function to check that no device is more than once in the 'dev' list
*
* @param[in] devListArr[]          - the array of device ids to add to the FDB Manager.
* @param[in] numOfDevs             - the number of device ids in the array.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - duplication detected
*/
static GT_STATUS    duplicationsInDevListCheck
(
    IN GT_U8  devListArr[], /*arrSizeVarName=numOfDevs*/
    IN GT_U32 numOfDevs
)
{
    GT_U32  ii,jj;
    GT_U8   devNum_ii,devNum_jj;

    for(ii = 0 ; ii < numOfDevs ; ii++)
    {
        devNum_ii = devListArr[ii];

        for(jj = ii+1 ; jj < numOfDevs ; jj++)
        {
            devNum_jj = devListArr[jj];

            if(devNum_ii == devNum_jj)
            {
                /* duplication */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                    "devNum [%d] is duplicated in the 'dev list' (index[%d] and index[%d])",
                    devNum_ii,
                    ii,
                    jj);
            }
        }
    }

    /* no duplications */
    return GT_OK;
}


/**
* @internal internal_cpssDxChBrgFdbManagerDevListAdd function
* @endinternal
*
* @brief   This function adding (registering) device(s) to specific FDB Manager Instance.
*          It can be in initialization time or in run-time in the context of Hot-Insertion or PP reset.
*          Note: the assumption is that in all use cases (Init / Reset / Hot-Insertion)
*          PP FDB is flushed and don't have any old MAC or other entries.
*          The only exception is the HA use-case (which will be handled by dedicated procedure).
*          Note: every time new device is added application may be required to call
*                in addition to this API, the per-ports APIs:
*                cpssDxChBrgFdbManagerPortLearningSet(...) and
*                cpssDxChBrgFdbManagerPortRoutingSet(...) with proper values
*                for each new device' port
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] devListArr[]          - the array of device ids to add to the FDB Manager.
* @param[in] numOfDevs             - the number of device ids in the array.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters , duplicated devices.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_ALREADY_EXIST         - if device already exist in DB
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*
* @note
*   NONE
*
*/
static GT_STATUS internal_cpssDxChBrgFdbManagerDevListAdd
(
    IN GT_U32 fdbManagerId,
    IN GT_U8  devListArr[], /*arrSizeVarName=numOfDevs*/
    IN GT_U32 numOfDevs
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr;
    GT_U32  ii;
    GT_U8   devNum;
    GT_U32  numOfEports;
    GT_U32  isDeviceValid;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;
    GT_BOOL                       managerHwWriteBlock,fdbUnderHaSave;
    PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_ENT    hwFormatType;

    FDB_MANAGER_ID_CHECK(fdbManagerId);
    CPSS_NULL_PTR_CHECK_MAC(devListArr);

    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    /********************************/
    /* check no device duplications */
    /********************************/
    rc = duplicationsInDevListCheck(devListArr,numOfDevs);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_FDB_MANAGER_E);
    fdbUnderHaSave = PRV_CPSS_DXCH_PP_MAC(devListArr[0])->bridge.fdbUnderHa;

    /* set fdb to skip all hw write*/
    if(((tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E)&&
        (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)) ||
        (managerHwWriteBlock == GT_TRUE))
    {
        devNum = devListArr[0];
        LOCK_DEV_NUM(devNum);
        PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbUnderHa = GT_TRUE;
        UNLOCK_DEV_NUM(devNum);
    }

    /*******************************************/
    /* add devices to DB (+ DB validity check) */
    /*******************************************/
    rc = prvCpssDxChFdbManagerDbDevListAdd(fdbManagerId,devListArr,numOfDevs);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* start HW initialization on the new devices */
    for(ii = 0 ; ii < numOfDevs ; ii++)
    {
        devNum = devListArr[ii];
        /* !!! start protecting the DB of the device !!! */
        LOCK_DEV_NUM_SET_IS_VALID(devNum,isDeviceValid);
        if(!isDeviceValid)
        {
            /* unlock the device ... because the CPSS removed it ! */
            /* we can't use it !!! ... go to the next registered device */
            UNLOCK_DEV_NUM(devNum);

            /* the device was removed during operation ?! */
            continue;
        }

        if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
        {
            numOfEports = PRV_CPSS_DXCH_MAX_PORT_NUMBER_MAC(devNum);
        }
        else
        {
            numOfEports = PRV_CPSS_PP_MAC(devNum)->numOfPorts;
        }

        /* !!! done protecting the DB of the device !!! */
        UNLOCK_DEV_NUM(devNum);

        /* Update the device types incase of shadow type is hybrid */
        if((fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E) ||
            (fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E))
        {
            rc = prvCpssDxChFdbManagerDeviceHwFormatGet(fdbManagerPtr, devNum, &hwFormatType);
            if(rc != GT_OK)
            {
                return rc;
            }
            fdbManagerPtr->hwDeviceTypesBitmap |= hwFormatType;
        }

        rc = prvCpssDxChFdbManagerHwInitNewDevice(fdbManagerPtr,devNum,numOfEports);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* start to write entries from manager to HW of the new devices */
    rc = prvCpssDxChFdbManagerHwWriteEntriesFromDbToHwOfNewDevices(fdbManagerPtr,
        devListArr,
        numOfDevs);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* restore fdb hw write*/
    if(((tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E)&&
        (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)) ||
        (managerHwWriteBlock == GT_TRUE))
    {
        devNum = devListArr[0];
        LOCK_DEV_NUM(devNum);
        PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbUnderHa = fdbUnderHaSave;
        UNLOCK_DEV_NUM(devNum);
    }

    return GT_OK;
}

/**
* @internal cpssDxChBrgFdbManagerDevListAdd function
* @endinternal
*
* @brief   This function adding (registering) device(s) to specific FDB Manager Instance.
*          It can be in initialization time or in run-time in the context of Hot-Insertion or PP reset.
*          Note: the assumption is that in all use cases (Init / Reset / Hot-Insertion)
*          PP FDB is flushed and don't have any old MAC or other entries.
*          The only exception is the HA use-case (which will be handled by dedicated procedure).
*          Note: every time new device is added application may be required to call
*                in addition to this API, the per-ports APIs:
*                cpssDxChBrgFdbManagerPortLearningSet(...) and
*                cpssDxChBrgFdbManagerPortRoutingSet(...) with proper values
*                for each new device' port
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] devListArr[]          - the array of device ids to add to the FDB Manager.
* @param[in] numOfDevs             - the number of device ids in the array.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters , duplicated devices.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_ALREADY_EXIST         - if device already exist in DB
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*
* @note
*   NONE
*
*/
GT_STATUS cpssDxChBrgFdbManagerDevListAdd
(
    IN GT_U32 fdbManagerId,
    IN GT_U8  devListArr[], /*arrSizeVarName=numOfDevs*/
    IN GT_U32 numOfDevs
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbManagerDevListAdd);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, fdbManagerId, devListArr, numOfDevs));

    rc = internal_cpssDxChBrgFdbManagerDevListAdd(fdbManagerId, devListArr, numOfDevs);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, fdbManagerId, devListArr, numOfDevs));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChBrgFdbManagerDevListRemove function
* @endinternal
*
* @brief   This function removing (unregistering) device(s) from specific FDB Manager Instance
*          (in Hot Removal and reset etc..).
*          NOTE: the FDB manager will stop accessing to those devices (and will
*          not access to it during current function)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] devListArr[]          - the array of device ids to remove from the FDB Manager.
* @param[in] numOfDevs             - the number of device ids in the array.
*
* @retval GT_OK                    - on success
* @retval GT_NO_SUCH               - if one of the devices not in DB.
* @retval GT_BAD_PARAM             - on wrong parameters , duplicated devices.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*
*/
static GT_STATUS internal_cpssDxChBrgFdbManagerDevListRemove
(
    IN GT_U32 fdbManagerId,
    IN GT_U8  devListArr[], /*arrSizeVarName=numOfDevs*/
    IN GT_U32 numOfDevs
)
{
    GT_STATUS   rc;

    FDB_MANAGER_ID_CHECK(fdbManagerId);
    CPSS_NULL_PTR_CHECK_MAC(devListArr);

    /********************************/
    /* check no device duplications */
    /********************************/
    rc = duplicationsInDevListCheck(devListArr,numOfDevs);
    if(rc != GT_OK)
    {
        return rc;
    }

    /**********************************************/
    /* remove devices to DB (+ DB validity check) */
    /**********************************************/
    rc = prvCpssDxChFdbManagerDbDevListRemove(fdbManagerId,devListArr,numOfDevs);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChBrgFdbManagerDevListRemove function
* @endinternal
*
* @brief   This function removing (unregistering) device(s) from specific FDB Manager Instance
*          (in Hot Removal and reset etc..).
*          NOTE: the FDB manager will stop accessing to those devices (and will
*          not access to it during current function)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] devListArr[]          - the array of device ids to remove from the FDB Manager.
* @param[in] numOfDevs             - the number of device ids in the array.
*
* @retval GT_OK                    - on success
* @retval GT_NO_SUCH               - if one of the devices not in DB.
* @retval GT_BAD_PARAM             - on wrong parameters , duplicated devices.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*
*/
GT_STATUS cpssDxChBrgFdbManagerDevListRemove
(
    IN GT_U32 fdbManagerId,
    IN GT_U8  devListArr[], /*arrSizeVarName=numOfDevs*/
    IN GT_U32 numOfDevs
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbManagerDevListRemove);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, fdbManagerId, devListArr, numOfDevs));

    rc = internal_cpssDxChBrgFdbManagerDevListRemove(fdbManagerId, devListArr, numOfDevs);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, fdbManagerId, devListArr, numOfDevs));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}

/**
* @internal restoreEntryFormatParamsFromDb_fdbEntryMacAddrFormat function
* @endinternal
*
* @brief  function to restore values of the FDB entry from DB in format
*         PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_MAC_ADDR_FORMAT_STC
*         copy values from manager format to application format
*         the 'opposite' logic of saveEntryFormatParamsToDb_fdbEntryMacAddrFormat(...)
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] dbEntryPtr             - (pointer to) the manager entry format (manager format)
* @param[out] entryPtr              - (pointer to) the entry (application format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   restoreEntryFormatParamsFromDb_fdbEntryMacAddrFormat
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC         *fdbManagerPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_MAC_ADDR_FORMAT_STC *dbEntryPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC   *entryPtr
)
{
    GT_U16      *u16Ptr = (void*)entryPtr;
    fdbManagerPtr = fdbManagerPtr;

    /* there is alignment gap between 'macAddr[6]' and 'GT_U32 fid' */
    /* gap of 16 bits   */
    /* lets fill it with ZEROs to the caller can compare 'sizeof(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC)'*/
    u16Ptr[3] = 0;

    /* restore from DB format to application format  */
    entryPtr->fid                = dbEntryPtr->fid            ;
    entryPtr->vid1               = dbEntryPtr->vid1           ;
    entryPtr->daSecurityLevel    = dbEntryPtr->daAccessLevel  ;
    entryPtr->saSecurityLevel    = dbEntryPtr->saAccessLevel  ;
    entryPtr->sourceID           = dbEntryPtr->srcId          ;
    entryPtr->userDefined        = dbEntryPtr->userDefined    ;
    entryPtr->isStatic           = dbEntryPtr->isStatic       ;
    entryPtr->age                = dbEntryPtr->age            ;
    entryPtr->daRoute            = dbEntryPtr->daRoute        ;
    entryPtr->appSpecificCpuCode = dbEntryPtr->appSpecificCpuCode;

    entryPtr->dstInterface.type  = dbEntryPtr->dstInterface_type;
    switch(entryPtr->dstInterface.type)
    {
        case CPSS_INTERFACE_PORT_E:
            entryPtr->dstInterface.interfaceInfo.devPort.hwDevNum = dbEntryPtr->dstInterface.devPort.hwDevNum  ;
            entryPtr->dstInterface.interfaceInfo.devPort.portNum  = dbEntryPtr->dstInterface.devPort.portNum   ;
            break;
        case CPSS_INTERFACE_TRUNK_E:
            entryPtr->dstInterface.interfaceInfo.trunkId = dbEntryPtr->dstInterface.trunkId;
            break;
        case CPSS_INTERFACE_VIDX_E:
            entryPtr->dstInterface.interfaceInfo.vidx    = dbEntryPtr->dstInterface.vidx;
            break;
        case CPSS_INTERFACE_VID_E:
            entryPtr->dstInterface.interfaceInfo.vidx      = FLOOD_VIDX_CNS;
            break;
        default:
            break;
    }

    if((fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E) ||
        (fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E))
    {
        entryPtr->saCommand              = dbEntryPtr->saCommand;
        entryPtr->daCommand              = dbEntryPtr->daCommand;
        entryPtr->daQoSParameterSetIndex = dbEntryPtr->daQosIndex;
        entryPtr->saQoSParameterSetIndex = dbEntryPtr->saQosIndex;
        switch((dbEntryPtr->saLookupIngressMirrorToAnalyzerPort << 1) | dbEntryPtr->daLookupIngressMirrorToAnalyzerPort)
        {
            case 0:
                entryPtr->mirrorToAnalyzerPort = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_DISABLED_E;
                break;
            case 1:
                entryPtr->mirrorToAnalyzerPort = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_DA_E;
                break;
            case 2:
                entryPtr->mirrorToAnalyzerPort = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_SA_E;
                break;
            case 3:
                entryPtr->mirrorToAnalyzerPort = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_SA_OR_DA_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(dbEntryPtr->saLookupIngressMirrorToAnalyzerPort);
        }
    }
    else
    {
        /* deal with NON- 1:1 field values */
        if(dbEntryPtr->saCommand == 1)
        {
            entryPtr->saCommand = fdbManagerPtr->entryAttrInfo.saDropCommand;
        }
        else
        {
            entryPtr->saCommand = CPSS_PACKET_CMD_FORWARD_E;
        }

        if(dbEntryPtr->daCommand == 3)
        {
            entryPtr->daCommand = fdbManagerPtr->entryAttrInfo.daDropCommand;
        }
        else
        {
            entryPtr->daCommand = dbEntryPtr->daCommand;
        }
        entryPtr->daQoSParameterSetIndex = 0;
        entryPtr->saQoSParameterSetIndex = 0;
        entryPtr->mirrorToAnalyzerPort = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_DISABLED_E;
    }

    entryPtr->macAddr.arEther[0] = (GT_U8)(dbEntryPtr->macAddr_high_16 >>  8);
    entryPtr->macAddr.arEther[1] = (GT_U8)(dbEntryPtr->macAddr_high_16 >>  0);
    entryPtr->macAddr.arEther[2] = (GT_U8)(dbEntryPtr->macAddr_low_32  >> 24);
    entryPtr->macAddr.arEther[3] = (GT_U8)(dbEntryPtr->macAddr_low_32  >> 16);
    entryPtr->macAddr.arEther[4] = (GT_U8)(dbEntryPtr->macAddr_low_32  >>  8);
    entryPtr->macAddr.arEther[5] = (GT_U8)(dbEntryPtr->macAddr_low_32  >>  0);

    return GT_OK;
}

/**
* @internal restoreEntryFormatParamsFromDb_fdbEntryIpMcAddrFormat function
* @endinternal
*
* @brief  function to restore values of the FDB entry from DB in format
*         PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IP_MC_ADDR_FORMAT_STC
*         copy values from manager format to application format
*         the 'opposite' logic of saveEntryFormatParamsToDb_fdbEntryIpMcFormat(...)
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] dbEntryPtr             - (pointer to) the manager entry format (manager format)
* @param[out] entryPtr              - (pointer to) the entry (application format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   restoreEntryFormatParamsFromDb_fdbEntryIpMcAddrFormat
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC           *fdbManagerPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IP_MC_ADDR_FORMAT_STC *dbEntryPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_IP_MC_FORMAT_STC        *entryPtr
)
{
    fdbManagerPtr = fdbManagerPtr;

    /* restore from DB format to application format  */
    entryPtr->fid                = dbEntryPtr->fid            ;
    entryPtr->daSecurityLevel    = dbEntryPtr->daAccessLevel  ;
    entryPtr->sourceID           = dbEntryPtr->srcId          ;
    entryPtr->userDefined        = dbEntryPtr->userDefined    ;
    entryPtr->isStatic           = dbEntryPtr->isStatic       ;
    entryPtr->age                = dbEntryPtr->age            ;
    entryPtr->daRoute            = dbEntryPtr->daRoute        ;
    entryPtr->appSpecificCpuCode = dbEntryPtr->appSpecificCpuCode;

    entryPtr->dstInterface.type  = dbEntryPtr->dstInterface_type;
    switch(entryPtr->dstInterface.type)
    {
        case CPSS_INTERFACE_PORT_E:
            entryPtr->dstInterface.interfaceInfo.devPort.hwDevNum = dbEntryPtr->dstInterface.devPort.hwDevNum  ;
            entryPtr->dstInterface.interfaceInfo.devPort.portNum  = dbEntryPtr->dstInterface.devPort.portNum   ;
            break;
        case CPSS_INTERFACE_TRUNK_E:
            entryPtr->dstInterface.interfaceInfo.trunkId = dbEntryPtr->dstInterface.trunkId;
            break;
        case CPSS_INTERFACE_VIDX_E:
            entryPtr->dstInterface.interfaceInfo.vidx    = dbEntryPtr->dstInterface.vidx;
            break;
        case CPSS_INTERFACE_VID_E:
            entryPtr->dstInterface.interfaceInfo.vidx      = FLOOD_VIDX_CNS;
            break;
        default:
            break;
    }

    if((fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E) ||
            (fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E))
    {
        switch((dbEntryPtr->saLookupIngressMirrorToAnalyzerPort << 1) | dbEntryPtr->daLookupIngressMirrorToAnalyzerPort)
        {
            case 0:
                entryPtr->mirrorToAnalyzerPort = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_DISABLED_E;
                break;
            case 1:
                entryPtr->mirrorToAnalyzerPort = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_DA_E;
                break;
            case 2:
                entryPtr->mirrorToAnalyzerPort = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_SA_E;
                break;
            case 3:
                entryPtr->mirrorToAnalyzerPort = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_SA_OR_DA_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(dbEntryPtr->saLookupIngressMirrorToAnalyzerPort);
        }
    }

    if(dbEntryPtr->daCommand == 3)
    {
        entryPtr->daCommand = fdbManagerPtr->entryAttrInfo.daDropCommand;
    }
    else
    {
        entryPtr->daCommand = dbEntryPtr->daCommand;
    }

    entryPtr->sipAddr[0] = (GT_U8)(dbEntryPtr->sipAddr  >> 24);
    entryPtr->sipAddr[1] = (GT_U8)(dbEntryPtr->sipAddr  >> 16);
    entryPtr->sipAddr[2] = (GT_U8)(dbEntryPtr->sipAddr  >>  8);
    entryPtr->sipAddr[3] = (GT_U8)(dbEntryPtr->sipAddr  >>  0);

    entryPtr->dipAddr[0] = (GT_U8)(dbEntryPtr->dipAddr  >> 24);
    entryPtr->dipAddr[1] = (GT_U8)(dbEntryPtr->dipAddr  >> 16);
    entryPtr->dipAddr[2] = (GT_U8)(dbEntryPtr->dipAddr  >>  8);
    entryPtr->dipAddr[3] = (GT_U8)(dbEntryPtr->dipAddr  >>  0);

    return GT_OK;
}

/**
* @internal restoreEntryFormatParamsFromDb_fdbEntryIpv4UcAddrFormat function
* @endinternal
*
* @brief  function to restore values of the FDB entry from DB in format
*         PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV4_UC_FORMAT_STC
*         copy values from manager format to application format
*         the 'opposite' logic of saveEntryFormatParamsToDb_fdbEntryIpv4UcFormat (...)
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] dbEntryPtr             - (pointer to) the manager entry format (manager format)
* @param[out] entryPtr              - (pointer to) the entry (application format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; AC5.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   restoreEntryFormatParamsFromDb_fdbEntryIpv4UcAddrFormat
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC           *fdbManagerPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV4_UC_FORMAT_STC    *dbEntryPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_IPV4_UC_ROUTING_INFO_STC      *entryPtr
)
{
    fdbManagerPtr = fdbManagerPtr;

    entryPtr->vrfId                = dbEntryPtr->ipUcCommonInfo.vrfId;
    entryPtr->ucRouteType          = dbEntryPtr->ipUcCommonInfo.ucRouteExtType;

    prvCpssDxChFdbManagerIpUcCommonInfoGet(fdbManagerPtr,
                                           entryPtr->ucRouteType,
                                           &dbEntryPtr->ipUcCommonInfo,
                                           &entryPtr->ucRouteInfo,
                                           &entryPtr->ucCommonInfo);

    entryPtr->age                   = dbEntryPtr->ipUcCommonInfo.age;
    entryPtr->ipv4Addr.arIP[0]      = (dbEntryPtr->ipAddr >> 24) & 0xff;
    entryPtr->ipv4Addr.arIP[1]      = (dbEntryPtr->ipAddr >> 16) & 0xff;
    entryPtr->ipv4Addr.arIP[2]      = (dbEntryPtr->ipAddr >>  8) & 0xff;
    entryPtr->ipv4Addr.arIP[3]      = (dbEntryPtr->ipAddr >>  0) & 0xff;

    return GT_OK;
}

/**
* @internal restoreEntryFormatParamsFromDb_fdbEntryIpv6UcAddrFormat function
* @endinternal
*
* @brief  function to restore values of the FDB entry from DB in format
*         PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV6_UC_FORMAT_STC
*         copy values from manager format to application format
*         the 'opposite' logic of saveEntryFormatParamsToDb_fdbEntryIpv6UcFormat (...)
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] dbEntryPtr             - (pointer to) the manager entry format (manager format)
* @param[out] entryPtr              - (pointer to) the entry (application format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; AC5.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   restoreEntryFormatParamsFromDb_fdbEntryIpv6UcAddrFormat
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC               *fdbManagerPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV6_UC_DATA_FORMAT_STC   *dbEntryDataPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV6_UC_KEY_FORMAT_STC    *dbEntryKeyPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_IPV6_UC_ROUTING_INFO_STC          *entryPtr
)
{
    fdbManagerPtr = fdbManagerPtr;

    entryPtr->vrfId                 = dbEntryDataPtr->ipUcCommonInfo.vrfId;
    entryPtr->ucRouteType           = dbEntryDataPtr->ipUcCommonInfo.ucRouteExtType;
    entryPtr->age                   = dbEntryDataPtr->ipUcCommonInfo.age;

    prvCpssDxChFdbManagerIpUcCommonInfoGet(fdbManagerPtr,
                                           entryPtr->ucRouteType,
                                           &dbEntryDataPtr->ipUcCommonInfo,
                                           &entryPtr->ucRouteInfo,
                                           &entryPtr->ucCommonInfo);

    entryPtr->ipv6Addr.arIP[ 0]     = (dbEntryDataPtr->ipAddr_127_106   >> 14) & 0xff;
    entryPtr->ipv6Addr.arIP[ 1]     = (dbEntryDataPtr->ipAddr_127_106   >>  6) & 0xff;
    entryPtr->ipv6Addr.arIP[ 2]     = ((dbEntryDataPtr->ipAddr_127_106  << 2) | 
                                        ((dbEntryKeyPtr->ipAddr_105_96  >> 8) & 0x3)) & 0xff;
    entryPtr->ipv6Addr.arIP[ 3]     =  dbEntryKeyPtr->ipAddr_105_96            & 0xFF;

    entryPtr->ipv6Addr.arIP[ 4]     = (dbEntryKeyPtr->ipAddr_95_64      >> 24) & 0xff;
    entryPtr->ipv6Addr.arIP[ 5]     = (dbEntryKeyPtr->ipAddr_95_64      >> 16) & 0xff;
    entryPtr->ipv6Addr.arIP[ 6]     = (dbEntryKeyPtr->ipAddr_95_64      >>  8) & 0xff;
    entryPtr->ipv6Addr.arIP[ 7]     = (dbEntryKeyPtr->ipAddr_95_64      >>  0) & 0xff;

    entryPtr->ipv6Addr.arIP[ 8]     = (dbEntryKeyPtr->ipAddr_63_32      >> 24) & 0xff;
    entryPtr->ipv6Addr.arIP[ 9]     = (dbEntryKeyPtr->ipAddr_63_32      >> 16) & 0xff;
    entryPtr->ipv6Addr.arIP[10]     = (dbEntryKeyPtr->ipAddr_63_32      >>  8) & 0xff;
    entryPtr->ipv6Addr.arIP[11]     = (dbEntryKeyPtr->ipAddr_63_32      >>  0) & 0xff;

    entryPtr->ipv6Addr.arIP[12]     = (dbEntryKeyPtr->ipAddr_31_0       >> 24) & 0xff;
    entryPtr->ipv6Addr.arIP[13]     = (dbEntryKeyPtr->ipAddr_31_0       >> 16) & 0xff;
    entryPtr->ipv6Addr.arIP[14]     = (dbEntryKeyPtr->ipAddr_31_0       >>  8) & 0xff;
    entryPtr->ipv6Addr.arIP[15]     = (dbEntryKeyPtr->ipAddr_31_0       >>  0) & 0xff;

    if(dbEntryDataPtr->ipv6DestSiteId == 0)
    {
        entryPtr->ucRouteInfo.fullFdbInfo.ipv6ExtInfo.siteId = CPSS_IP_SITE_ID_INTERNAL_E;
    }
    else
    {
        entryPtr->ucRouteInfo.fullFdbInfo.ipv6ExtInfo.siteId = CPSS_IP_SITE_ID_EXTERNAL_E;
    }

    entryPtr->ucRouteInfo.fullFdbInfo.ipv6ExtInfo.scopeCheckingEnable = dbEntryDataPtr->ipv6ScopeCheck;

    return GT_OK;
}

/**
* @internal prvCpssDxChBrgFdbManagerRestoreEntryFormatParamsFromDb function
* @endinternal
*
* @brief  function to restore values of the FDB entry from DB
*         copy values from manager format to application format
*         the 'opposite' logic of saveEntryFormatParamsToDb(...)
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] dbEntryPtr             - (pointer to) the manager entry format (manager format)
* @param[out] entryPtr              - (pointer to) the entry (application format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS   prvCpssDxChBrgFdbManagerRestoreEntryFormatParamsFromDb
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC *fdbManagerPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC  *dbEntryPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC           *entryPtr
)
{
    cpssOsMemSet(entryPtr, 0, sizeof(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC));

    switch(dbEntryPtr->hwFdbEntryType)
    {
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E:
            entryPtr->fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E;
            return restoreEntryFormatParamsFromDb_fdbEntryMacAddrFormat(fdbManagerPtr,
                &dbEntryPtr->specificFormat.prvMacEntryFormat,
                &entryPtr->format.fdbEntryMacAddrFormat);
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_UC_E:
            entryPtr->fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E;
            return restoreEntryFormatParamsFromDb_fdbEntryIpv4UcAddrFormat(fdbManagerPtr,
                &dbEntryPtr->specificFormat.prvIpv4UcEntryFormat,
                &entryPtr->format.fdbEntryIpv4UcFormat);
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_MCAST_E:
            entryPtr->fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E;
            return restoreEntryFormatParamsFromDb_fdbEntryIpMcAddrFormat(fdbManagerPtr,
                &dbEntryPtr->specificFormat.prvIpv4McEntryFormat,
                &entryPtr->format.fdbEntryIpv4McFormat);
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_MCAST_E:
            entryPtr->fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E;
            return restoreEntryFormatParamsFromDb_fdbEntryIpMcAddrFormat(fdbManagerPtr,
                &dbEntryPtr->specificFormat.prvIpv6McEntryFormat,
                &entryPtr->format.fdbEntryIpv6McFormat);
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(dbEntryPtr->hwFdbEntryType);
    }
}

/**
* @internal prvCpssDxChBrgFdbManagerRestoreEntryIpV6UcFormatParamsFromDb function
* @endinternal
*
* @brief  function to restore values of the FDB entry for IpV6 UC from DB
*         copy values from manager format to application format
*         the 'opposite' logic of saveEntryFormatParamsToDb(...)
*
* @param[in] fdbManagerPtr          - (pointer to) the FDB Manager.
* @param[in] dbEntryPtr             - (pointer to) the manager entry format (manager format)
* @param[in] dbEntryExtPtr          - (pointer to) the manager extended entry format (manager format)
* @param[out] entryPtr              - (pointer to) the entry (application format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS   prvCpssDxChBrgFdbManagerRestoreEntryIpV6UcFormatParamsFromDb
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC *fdbManagerPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC  *dbEntryPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC  *dbEntryExtPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC           *entryPtr
)
{
    cpssOsMemSet(entryPtr, 0, sizeof(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC));

    entryPtr->fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E;

    return restoreEntryFormatParamsFromDb_fdbEntryIpv6UcAddrFormat(fdbManagerPtr,
                                                                   &dbEntryExtPtr->specificFormat.prvIpv6UcDataEntryFormat,
                                                                   &dbEntryPtr->specificFormat.prvIpv6UcKeyEntryFormat,
                                                                   &entryPtr->format.fdbEntryIpv6UcFormat);
}

/**
* @internal internal_cpssDxChBrgFdbManagerEntryGet function
* @endinternal
*
* @brief   The function return entry info from the manager by 'key'
*          NOTE: no HW accessing.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entryPtr              - (pointer to) FDB entry format with the 'fdbEntryKey'.
*                                     NOTE: only the 'fdbEntryKey' part is used by the API.(as [in] parameter)
* @param[out] entryPtr             - (pointer to) FDB entry format with full info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_FOUND             - if entry not found for that 'key'.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
*
* @note
*   NONE
*
*/
static GT_STATUS internal_cpssDxChBrgFdbManagerEntryGet
(
    IN GT_U32                                           fdbManagerId,
    INOUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC           *entryPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC calcInfo;

    FDB_MANAGER_ID_CHECK(fdbManagerId);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    /********************************/
    /* validity checks of SW format */
    /********************************/
    rc = prvCpssDxChFdbManagerValidateEntryKeyFormatParams(fdbManagerPtr,entryPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*******************************************/
    /* calculate where 'key' is expected in DB */
    /*******************************************/
    rc = prvCpssDxChFdbManagerDbCalcHashAndLookupResult(fdbManagerPtr,entryPtr,NULL,&calcInfo);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(calcInfo.calcEntryType != PRV_CPSS_DXCH_FDB_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_EXISTS_E)
    {
        /* the 'key' was not found in the DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND,LOG_ERROR_NO_MSG);
    }

    if (calcInfo.bankStep > 1)
    {
        /* convert DB format to application format - IPv6 UC */
        rc = prvCpssDxChBrgFdbManagerRestoreEntryIpV6UcFormatParamsFromDb(fdbManagerPtr, calcInfo.dbEntryPtr, calcInfo.dbEntryExtPtr, entryPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        /* convert DB format to application format */
        rc = prvCpssDxChBrgFdbManagerRestoreEntryFormatParamsFromDb(fdbManagerPtr, calcInfo.dbEntryPtr, entryPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChBrgFdbManagerEntryGet function
* @endinternal
*
* @brief   The function return entry info from the manager by 'key'
*          NOTE: no HW accessing.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entryPtr              - (pointer to) FDB entry format with the 'fdbEntryKey'.
*                                     NOTE: only the 'fdbEntryKey' part is used by the API.(as [in] parameter)
* @param[out] entryPtr             - (pointer to) FDB entry format with full info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_FOUND             - if entry not found for that 'key'.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
*
* @note
*   NONE
*
*/
GT_STATUS cpssDxChBrgFdbManagerEntryGet
(
    IN GT_U32                                           fdbManagerId,
    INOUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC           *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbManagerEntryGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, fdbManagerId, entryPtr));

    rc = internal_cpssDxChBrgFdbManagerEntryGet(fdbManagerId, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, fdbManagerId, entryPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgFdbManagerEntryGetNext function
* @endinternal
*
* @brief   The function return first entry or next entry from previous one.
*          NOTE: 1. the 'list' is not sorted by key or hwIndex.
*                2. no HW accessing.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] entryPtr             - (pointer to) FDB entry format with full info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NO_MORE               - if no more entries to retrieve
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
*
* @note
*   NONE
*
*/
static GT_STATUS internal_cpssDxChBrgFdbManagerEntryGetNext
(
    IN GT_U32                                           fdbManagerId,
    IN GT_BOOL                                          getFirst,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC             *entryPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC    *dbEntryPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC    *dbEntryDataPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC    *dbEntryKeyPtr;

    FDB_MANAGER_ID_CHECK(fdbManagerId);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    rc = prvCpssDxChFdbManagerDbApplicationIteratorEntryGetNext(fdbManagerPtr,getFirst,&fdbManagerPtr->lastGetNextInfo,&dbEntryPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    if (dbEntryPtr->hwFdbEntryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E)
    {
        dbEntryKeyPtr = dbEntryPtr;
        /** IpV6 UC entry comprises of two types of entries - key and data and resides subsequently in DB and HW,
         *  so the order of writing/reading to/from DB/HW should be subsequent as well.
         */
        rc = prvCpssDxChFdbManagerDbApplicationIteratorEntryGetNext(fdbManagerPtr,GT_FALSE,&fdbManagerPtr->lastGetNextInfo,&dbEntryPtr);
        if(rc != GT_OK)
        {
            return rc;
        }

        dbEntryDataPtr = dbEntryPtr;
        /* convert DB format to application format - IPv6 UC */
        rc = prvCpssDxChBrgFdbManagerRestoreEntryIpV6UcFormatParamsFromDb(fdbManagerPtr, dbEntryKeyPtr, dbEntryDataPtr, entryPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        /* convert DB format to application format */
        rc = prvCpssDxChBrgFdbManagerRestoreEntryFormatParamsFromDb(fdbManagerPtr, dbEntryPtr, entryPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChBrgFdbManagerEntryGetNext function
* @endinternal
*
* @brief   The function return first entry or next entry from previous one.
*          NOTE: 1. the 'list' is not sorted by key or hwIndex.
*                2. no HW accessing.
*           API for fetching next entry data either first one or next relative
*           to last entry retrieved.
*           FDB Manager hold internal pointer to the last entry retrieved and
*           it use that to locate next one (see FDB Manager Data Structure section for details).
*           Note that if that entry is deleted, the Delete API is moving this
*           pointer to previous entry (which can become NULL if that was the
*           first entry).
*           Once scan completed and API fetched all entries in the list it
*           returns GT_NO_MORE, Application can still call this API to see if
*           any additional entries has been added. It will keep getting
*           GT_NO_MORE until no additional entries or until restarted the scan
*           from beginning.
*           To start over from head of the list Application should pass True on
*           getFirst (its valid to restart the scan anytime - Regardless the
*           position of the scan pointer).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] entryPtr             - (pointer to) FDB entry format with full info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NO_MORE               - if no more entries to retrieve
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
*
* @note
*   NONE
*
*/
GT_STATUS cpssDxChBrgFdbManagerEntryGetNext
(
    IN GT_U32                                           fdbManagerId,
    IN GT_BOOL                                          getFirst,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC             *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbManagerEntryGetNext);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, fdbManagerId, getFirst, entryPtr));

    rc = internal_cpssDxChBrgFdbManagerEntryGetNext(fdbManagerId, getFirst, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, fdbManagerId, getFirst, entryPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}

/**
* @internal mainLogicEntryUpdate function
* @endinternal
*
* @brief   This function updates entry to CPSS FDB Manager's database and HW.
*          All types of entries can be updated using this API.
*          NOTE:
*          - the FDB manager will set the info to all the registered devices
*          - if entry was found, need to check that there is no trying to overwrite static entry with dynamic one
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager
* @param[in] entryPtr              - (pointer to) FDB entry format to be updated
* @param[in] paramsPtr             - (pointer to) extra info related to 'update entry' operation
* @param[out] updateApiStatisticsPtr
*                                  - (pointer to) the type of the API ok/error statistics
* @param[out] bankIndexPtr         - (pointer to) bank index, the updated entry belongs to.
* @param[out] dbEntryPtrPtr        - (pointer to) dbEntryPtr , that should exists for the 'updated entry'
*                                       needed for moved entries by 'AUQ message' that not hold all the
*                                       info in HW and need info from SW too.
* @param[in] isUpdateFromLearningScan - Indicates if add entry called from learning scan.
*                                    GT_TRUE   - update entry called from learning scan.
*                                    GT_FALSE  - update entry not called from learning scan.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - the entry not found
* @retval GT_LEARN_LIMIT_PORT_ERROR         - if learn limit on port reached.
* @retval GT_LEARN_LIMIT_TRUNK_ERROR        - if learn limit on trunk reached.
* @retval GT_LEARN_LIMIT_GLOBAL_EPORT_ERROR - if learn limit on global eport reached.
* @retval GT_LEARN_LIMIT_FID_ERROR          - if learn limit on fid reached.
* @retval GT_LEARN_LIMIT_GLOBAL_ERROR       - if learn limit globally reached.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the FDB Manager is not initialized
* @retval GT_NOT_ALLOWED           - if trying to overwrite static entry with dynamic one
*
* @note
*   NONE
*
*/
static GT_STATUS mainLogicEntryUpdate
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC               *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                          *entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC            *paramsPtr,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ENT   *updateApiStatisticsPtr,
    OUT GT_U32                                                      *bankIndexPtr,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC               **dbEntryPtrPtr,
    IN GT_BOOL                                                      isUpdateFromLearningscan
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC calcInfo;

    *updateApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_INPUT_INVALID_E;

    CPSS_NULL_PTR_CHECK_MAC(entryPtr);
    CPSS_NULL_PTR_CHECK_MAC(paramsPtr);

    *updateApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS___LAST___E;

    /********************************
      Validity checks of SW format
     ********************************/
    rc = prvCpssDxChFdbManagerValidateEntryFormatParams(fdbManagerPtr, entryPtr);
    if(rc != GT_OK)
    {
        *updateApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_INPUT_INVALID_E;
        return rc;
    }

    /*************************************
      Lookup for existing entry in the DB
     *************************************/
    rc = prvCpssDxChFdbManagerDbCalcHashAndLookupResult(fdbManagerPtr, entryPtr, NULL,&calcInfo);
    if(rc != GT_OK)
    {
        /* Should not fail, so if failed this is bad state */
        *updateApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_BAD_STATE_E;
        return rc;
    }

    if(calcInfo.calcEntryType != PRV_CPSS_DXCH_FDB_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_EXISTS_E)
    {
        *updateApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_NOT_FOUND_E;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "the entry doesn't exists so can not update it");
    }

    if(bankIndexPtr != NULL)
    {
        *bankIndexPtr = calcInfo.selectedBankId;
    }

    if (calcInfo.dbEntryPtr->hwFdbEntryType == CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E)
    {
        if (calcInfo.dbEntryPtr->specificFormat.prvMacEntryFormat.isStatic == GT_TRUE)
        {
            if (entryPtr->format.fdbEntryMacAddrFormat.isStatic == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_ALLOWED,
                    "the static entry could not be overwritten by the dynamic one: at hwIndex[%d]",
                    calcInfo.dbEntryPtr->hwIndex);
            }
        }
    }

    if(dbEntryPtrPtr)
    {
        *dbEntryPtrPtr = calcInfo.dbEntryPtr;
    }

    /* from this point ... should not fail ! so if failed this is bad state */
    *updateApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_BAD_STATE_E;

    return prvCpssDxChFdbManagerUpdateEntryAndCounter(fdbManagerPtr,
            &calcInfo,
            entryPtr,
            paramsPtr,
            updateApiStatisticsPtr,
            isUpdateFromLearningscan);
}

/**
* @internal cpssDxChBrgFdbManagerEntryUpdate function
* @endinternal
*
* @brief   The function updates existing entry in CPSS FDB Manager's database & HW.
*          All types of entries can be updated using this API.
*          Notes:
*          - Application can use this API to update entire entry data or just the source interface,
*          which is the recommended method when updating entry as a result of Entry Moved update event.
*          In this case when HW informs SW that dynamic MAC has changed, not all entry fields are being passed,
*          so to avoid overwriting previous values with defaults, application suggests to choose to overwrite only the source interface.
*          - API checks and not allows overwriting existing static entry with dynamic entry (with the same key) while the opposite is allowed.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entryPtr              - (pointer to) FDB entry format with full info.
* @param[in] paramsPtr             - (pointer to) Update FDB entry parameters
* @param[out] bankIndexPtr         - (pointer to) bank index, the updated entry belongs to.
* @param[out] dbEntryPtrPtr        - (pointer to) dbEntryPtr , that should exists for the 'updated entry'
*                                       needed for moved entries by 'AUQ message' that not hold all the
*                                       info in HW and need info from SW too.
* @param[in] isUpdateFromLearningScan - Indicates if add entry called from learning scan.
*                                    GT_TRUE   - update entry called from learning scan.
*                                    GT_FALSE  - update entry not called from learning scan.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_INITIALIZED       - if the FDB Manager is not initialized
* @retval GT_NOT_ALLOWED           - if trying to overwrite static entry with dynamic one
* @retval GT_LEARN_LIMIT_PORT_ERROR         - if learn limit on port reached.
* @retval GT_LEARN_LIMIT_TRUNK_ERROR        - if learn limit on trunk reached.
* @retval GT_LEARN_LIMIT_GLOBAL_EPORT_ERROR - if learn limit on global eport reached.
* @retval GT_LEARN_LIMIT_FID_ERROR          - if learn limit on fid reached.
* @retval GT_LEARN_LIMIT_GLOBAL_ERROR       - if learn limit globally reached.
*
* @note
*   NONE
*
*/
static GT_STATUS internal_cpssDxChBrgFdbManagerEntryUpdate
(
    IN GT_U32                                             fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC               *entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC *paramsPtr,
    OUT GT_U32                                           *bankIndexPtr,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC    **dbEntryPtrPtr,
    IN GT_BOOL                                            isUpdateFromLearningscan
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ENT    updateApiStatistics;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr;

    FDB_MANAGER_ID_CHECK(fdbManagerId); /* must be done here because used in 'API' statistics */

    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    /* call the main logic */
    rc = mainLogicEntryUpdate(fdbManagerPtr, entryPtr, paramsPtr, &updateApiStatistics, bankIndexPtr,dbEntryPtrPtr,isUpdateFromLearningscan);

    if(updateApiStatistics < PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS___LAST___E)
    {
        /* increment the 'API statistics' */
        fdbManagerPtr->apiEntryUpdateStatisticsArr[updateApiStatistics]++;
    }

    return rc;
}

/**
* @internal cpssDxChBrgFdbManagerEntryUpdate function
* @endinternal
*
* @brief   The function updates existing entry in CPSS FDB Manager's database & HW.
*          All types of entries can be updated using this API.
*          Notes:
*          - Application can use this API to update entire entry data or just the source interface,
*          which is the recommended method when updating entry as a result of Entry Moved update event.
*          In this case when HW informs SW that dynamic MAC has changed, not all entry fields are being passed,
*          so to avoid overwriting previous values with defaults, application suggests to choose to overwrite only the source interface.
*          - API checks and not allows overwriting existing static entry with dynamic entry (with the same key) while the opposite is allowed.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entryPtr              - (pointer to) FDB entry format with full info.
* @param[in] paramsPtr             - (pointer to) Update FDB entry parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_INITIALIZED       - if the FDB Manager is not initialized
* @retval GT_NOT_ALLOWED           - if trying to overwrite static entry with dynamic one
* @retval GT_LEARN_LIMIT_PORT_ERROR         - if learn limit on port reached.
* @retval GT_LEARN_LIMIT_TRUNK_ERROR        - if learn limit on trunk reached.
* @retval GT_LEARN_LIMIT_GLOBAL_EPORT_ERROR - if learn limit on global eport reached.
* @retval GT_LEARN_LIMIT_FID_ERROR          - if learn limit on fid reached.
* @retval GT_LEARN_LIMIT_GLOBAL_ERROR       - if learn limit globally reached.
*
* @note
*   NONE
*
*/
GT_STATUS cpssDxChBrgFdbManagerEntryUpdate
(
    IN GT_U32                                           fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              *entryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC *paramsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbManagerEntryUpdate);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, fdbManagerId, entryPtr, paramsPtr));

    rc = internal_cpssDxChBrgFdbManagerEntryUpdate(fdbManagerId, entryPtr, paramsPtr, NULL,NULL,GT_FALSE);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, fdbManagerId, entryPtr, paramsPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}

/**
* @internal cpssDxChBrgFdbManagerConfigGet function
* @endinternal
*
* @brief API for fetching current FDB Manager setting as they were defined in FDB Manager Create.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] capacityPtr          - (pointer to) parameters of the HW structure and capacity of its managed PPs FDBs:
*                                    like number of indexes, the amount of hashes etc.
*                                    As FDB Manager is created and entries may be added before PP registered,
*                                    we must know in advance the relevant capacity.
*                                    In Falcon, for example, we must know the shared memory allocation mode.
* @param[out] entryAttrPtr         - (pointer to) parameters that affects how HW/SW entries are
*                                    structured and specific fields behavior including:
*                                    MUX'd fields in MAC / IPMC entries format, SA / DA drop commands mode and
*                                    IP NH packet command.
* @param[out] learningPtr          - (pointer to) parameters that affects which sort of entries to learn etc.
*                                    (Learn MAC of Routed packets and MAC no-space updates).
* @param[out] lookupPtr            - (pointer to) parameters that affect lookup of entries: MAC VLAN Lookup mode
*                                    (none, single or double tags), IVL single-tag MSB mode.
*                                    And two new Falcon setting: IPv4 UC and IPv6 UC masks
*                                    (allow using FDB for prefixes lookup and not just exact match)
* @param[out] agingPtr             - (pointer to) parameters that affect refresh and aging
*                                      (Refresh Destination UC, Refresh Destination MC &Refresh IP UC)
*
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS internal_cpssDxChBrgFdbManagerConfigGet
(
    IN  GT_U32                                          fdbManagerId,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC          *capacityPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC  *entryAttrPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC          *learningPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC            *lookupPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC             *agingPtr
)
{
    FDB_MANAGER_ID_CHECK(fdbManagerId);
    CPSS_NULL_PTR_CHECK_MAC(capacityPtr);
    CPSS_NULL_PTR_CHECK_MAC(entryAttrPtr);
    CPSS_NULL_PTR_CHECK_MAC(learningPtr);
    CPSS_NULL_PTR_CHECK_MAC(lookupPtr);
    CPSS_NULL_PTR_CHECK_MAC(agingPtr);

    prvCpssDxChBrgFdbManagerDbConfigGet(fdbManagerId, capacityPtr, entryAttrPtr, learningPtr, lookupPtr, agingPtr);

    return GT_OK;
}

/**
* @internal cpssDxChBrgFdbManagerConfigGet function
* @endinternal
*
* @brief API for fetching current FDB Manager setting as they were defined in FDB Manager Create.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] capacityPtr          - (pointer to) parameters of the HW structure and capacity of its managed PPs FDBs:
*                                    like number of indexes, the amount of hashes etc.
*                                    As FDB Manager is created and entries may be added before PP registered,
*                                    we must know in advance the relevant capacity.
*                                    In Falcon, for example, we must know the shared memory allocation mode.
* @param[out] entryAttrPtr         - (pointer to) parameters that affects how HW/SW entries are
*                                    structured and specific fields behavior including:
*                                    MUX'd fields in MAC / IPMC entries format, SA / DA drop commands mode and
*                                    IP NH packet command.
* @param[out] learningPtr          - (pointer to) parameters that affects which sort of entries to learn etc.
*                                    (Learn MAC of Routed packets and MAC no-space updates).
* @param[out] lookupPtr            - (pointer to) parameters that affect lookup of entries: MAC VLAN Lookup mode
*                                    (none, single or double tags), IVL single-tag MSB mode.
*                                    And two new Falcon setting: IPv4 UC and IPv6 UC masks
*                                    (allow using FDB for prefixes lookup and not just exact match)
* @param[out] agingPtr             - (pointer to) parameters that affect refresh and aging
*                                      (Refresh Destination UC, Refresh Destination MC &Refresh IP UC)
*
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS cpssDxChBrgFdbManagerConfigGet
(
    IN  GT_U32                                          fdbManagerId,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC          *capacityPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC  *entryAttrPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC          *learningPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC            *lookupPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC             *agingPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbManagerConfigGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, fdbManagerId, capacityPtr, entryAttrPtr, learningPtr, lookupPtr, agingPtr));

    rc = internal_cpssDxChBrgFdbManagerConfigGet(fdbManagerId, capacityPtr, entryAttrPtr, learningPtr, lookupPtr, agingPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, fdbManagerId, capacityPtr, entryAttrPtr, learningPtr, lookupPtr, agingPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}

/**
* @internal cpssDxChBrgFdbManagerDevListGet function
* @endinternal
*
* @brief API for fetching current FDB Manager registered devices (added by 'add device' API).
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[inout] numOfDevicesPtr    - (pointer to) the number of devices in the array deviceListArray as 'input parameter'
*                                    and actual number of devices as 'output parameter'.
* @param[out] deviceListArray[]    - array of registered devices (no more than list length).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS internal_cpssDxChBrgFdbManagerDevListGet
(
    IN GT_U32           fdbManagerId,
    INOUT GT_U32        *numOfDevicesPtr,
    OUT GT_U8           deviceListArray[] /*arrSizeVarName=*numOfDevicesPtr*/
)
{
    FDB_MANAGER_ID_CHECK(fdbManagerId);
    CPSS_NULL_PTR_CHECK_MAC(numOfDevicesPtr);
    CPSS_NULL_PTR_CHECK_MAC(deviceListArray);

    return prvCpssDxChBrgFdbManagerDbDevListGet(fdbManagerId, numOfDevicesPtr, deviceListArray);
}

/**
* @internal cpssDxChBrgFdbManagerDevListGet function
* @endinternal
*
* @brief API for fetching current FDB Manager registered devices (added by 'add device' API).
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[inout] numOfDevicesPtr    - (pointer to) the number of devices in the array deviceListArray as 'input parameter'
*                                    and actual number of devices as 'output parameter'.
* @param[out] deviceListArray[]    - array of registered devices (no more than list length).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS cpssDxChBrgFdbManagerDevListGet
(
    IN GT_U32           fdbManagerId,
    INOUT GT_U32        *numOfDevicesPtr,
    OUT GT_U8           deviceListArray[] /*arrSizeVarName=*numOfDevicesPtr*/
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbManagerDevListGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, fdbManagerId, numOfDevicesPtr, deviceListArray));

    rc = internal_cpssDxChBrgFdbManagerDevListGet(fdbManagerId, numOfDevicesPtr, deviceListArray);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, fdbManagerId, numOfDevicesPtr, deviceListArray));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgFdbManagerPortLearningSet function
* @endinternal
*
* @brief   The function configures ports learning related attributes
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] learningPtr           - (pointer to)learning attributes specific for port to set
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgFdbManagerPortLearningSet
(
    IN GT_U8                                        devNum,
    IN GT_PORT_NUM                                  portNum,
    IN CPSS_DXCH_BRG_FDB_MANAGER_PORT_LEARNING_STC *learningPtr
)
{
    GT_STATUS       rc = GT_OK;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(learningPtr);

    /* set the <NA_MSG_TO_CPU_EN>*/
    rc = cpssDxChBrgFdbNaToCpuPerPortSet(devNum, portNum, learningPtr->naMsgToCpuEnable);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Auto Learning is Disabled for FDB Manager
     * Set unkSrcAddrCmd */
    rc = cpssDxChBrgFdbPortLearnStatusSet(devNum, portNum, GT_FALSE, learningPtr->unkSrcAddrCmd);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* AC5 is not supported for these below params */
    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        /* Set Learning priority */
        rc = cpssDxChBrgFdbLearnPrioritySet(devNum, portNum, learningPtr->learnPriority);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* Set user group */
        rc = cpssDxChBrgFdbUserGroupSet(devNum, portNum, learningPtr->userGroup);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* moved mac SA command (for dynamic entries) */
        rc = cpssDxChBrgFdbPortMovedMacSaCommandSet(devNum, portNum, learningPtr->movedMacSaCmd);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        /* AC5 - Storm prevent should be aligned with NA Msg to CPU.
         *       NA to CPU disabled -> SP entry creation should be disabled
         *       NA to CPU enabled  -> SP entry creation should be enabled
         */
        rc = cpssDxChBrgFdbNaStormPreventSet(devNum, portNum, learningPtr->naMsgToCpuEnable);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChBrgFdbManagerPortLearningSet function
* @endinternal
*
* @brief   The function configures ports learning related attributes
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] learningPtr           - (pointer to)learning attributes specific for port to set
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbManagerPortLearningSet
(
    IN GT_U8                                        devNum,
    IN GT_PORT_NUM                                  portNum,
    IN CPSS_DXCH_BRG_FDB_MANAGER_PORT_LEARNING_STC *learningPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbManagerPortLearningSet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, learningPtr));

    rc = internal_cpssDxChBrgFdbManagerPortLearningSet(devNum, portNum, learningPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, learningPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}

/**
* @internal cpssDxChBrgFdbManagerPortLearningGet function
* @endinternal
*
* @brief   The function gets ports learning related attributes
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[out] learningPtr          - (pointer to)learning attributes specific for port to get
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgFdbManagerPortLearningGet
(
    IN GT_U8                                        devNum,
    IN GT_PORT_NUM                                  portNum,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_PORT_LEARNING_STC *learningPtr
)
{
    GT_BOOL dummy;
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(learningPtr);

    /* Get the <NA_MSG_TO_CPU_EN>*/
    rc = cpssDxChBrgFdbNaToCpuPerPortGet(devNum, portNum, &(learningPtr->naMsgToCpuEnable));
    if(rc != GT_OK)
    {
        return rc;
    }

     /* Get unkSrcAddrCmd */
    rc = cpssDxChBrgFdbPortLearnStatusGet(devNum, portNum, &dummy, &(learningPtr->unkSrcAddrCmd));
    if (rc != GT_OK)
    {
        return rc;
    }

    /* AC5 is not supported for these below params */
    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        /* Get Learning priority */
        rc = cpssDxChBrgFdbLearnPriorityGet(devNum, portNum, &(learningPtr->learnPriority));
        if (rc != GT_OK)
        {
            return rc;
        }
        /* Get user group */
        rc = cpssDxChBrgFdbUserGroupGet(devNum, portNum, &(learningPtr->userGroup));
        if (rc != GT_OK)
        {
            return rc;
        }

        /* moved mac SA command (for dynamic entries) */
        rc = cpssDxChBrgFdbPortMovedMacSaCommandGet(devNum, portNum, &learningPtr->movedMacSaCmd);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChBrgFdbManagerPortLearningGet function
* @endinternal
*
* @brief   The function gets ports learning related attributes
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[out] learningPtr          - (pointer to)learning attributes specific for port to get
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbManagerPortLearningGet
(
    IN GT_U8                                        devNum,
    IN GT_PORT_NUM                                  portNum,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_PORT_LEARNING_STC *learningPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbManagerPortLearningGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, learningPtr));

    rc = internal_cpssDxChBrgFdbManagerPortLearningGet(devNum, portNum, learningPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, learningPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgFdbManagerPortRoutingSet function
* @endinternal
*
* @brief   The function configures ports "FDB based" routing status
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] routingPtr            - (pointer to)routing status of the port to set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgFdbManagerPortRoutingSet
(
    IN GT_U8                                        devNum,
    IN GT_PORT_NUM                                  portNum,
    IN CPSS_DXCH_BRG_FDB_MANAGER_PORT_ROUTING_STC  *routingPtr
)
{
    GT_STATUS                   rc = GT_OK;
    CPSS_IP_PROTOCOL_STACK_ENT  protocol;
    GT_BOOL                     enable;


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(routingPtr);

    if (routingPtr->unicastIpv4RoutingEn == routingPtr->unicastIpv6RoutingEn)
    {
        protocol = CPSS_IP_PROTOCOL_IPV4V6_E;
        enable = routingPtr->unicastIpv4RoutingEn;
        rc = cpssDxChBrgFdbRoutingPortIpUcEnableSet(devNum, portNum, protocol, enable);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        rc = cpssDxChBrgFdbRoutingPortIpUcEnableSet(devNum,
                                                    portNum,
                                                    CPSS_IP_PROTOCOL_IPV4_E,
                                                    routingPtr->unicastIpv4RoutingEn);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = cpssDxChBrgFdbRoutingPortIpUcEnableSet(devNum,
                                                    portNum,
                                                    CPSS_IP_PROTOCOL_IPV6_E,
                                                    routingPtr->unicastIpv6RoutingEn);
    }
    return rc;
}

/**
* @internal cpssDxChBrgFdbManagerPortRoutingSet function
* @endinternal
*
* @brief   The function configures ports "FDB based" routing status
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] routingPtr            - (pointer to)routing status of the port to set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbManagerPortRoutingSet
(
    IN GT_U8                                        devNum,
    IN GT_PORT_NUM                                  portNum,
    IN CPSS_DXCH_BRG_FDB_MANAGER_PORT_ROUTING_STC  *routingPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbManagerPortRoutingSet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, routingPtr));

    rc = internal_cpssDxChBrgFdbManagerPortRoutingSet(devNum, portNum, routingPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, routingPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgFdbManagerPortRoutingGet function
* @endinternal
*
* @brief   The function gets ports "FDB based" routing status
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[out] routingPtr           - (pointer to)routing status of the port to get.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgFdbManagerPortRoutingGet
(
    IN  GT_U8                                        devNum,
    IN  GT_PORT_NUM                                  portNum,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_PORT_ROUTING_STC  *routingPtr
)
{
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(routingPtr);

    rc = cpssDxChBrgFdbRoutingPortIpUcEnableGet(devNum,
                                                portNum,
                                                CPSS_IP_PROTOCOL_IPV6_E,
                                                &(routingPtr->unicastIpv6RoutingEn));
    if (rc != GT_OK)
    {
        return rc;
    }
    return cpssDxChBrgFdbRoutingPortIpUcEnableGet(devNum,
                                                  portNum,
                                                  CPSS_IP_PROTOCOL_IPV4_E,
                                                  &(routingPtr->unicastIpv4RoutingEn));
}

/**
* @internal cpssDxChBrgFdbManagerPortRoutingGet function
* @endinternal
*
* @brief   The function gets ports "FDB based" routing status
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; AC5.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[out] routingPtr           - (pointer to)routing status of the port to get.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbManagerPortRoutingGet
(
    IN  GT_U8                                        devNum,
    IN  GT_PORT_NUM                                  portNum,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_PORT_ROUTING_STC  *routingPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbManagerPortRoutingGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, routingPtr));

    rc = internal_cpssDxChBrgFdbManagerPortRoutingGet(devNum, portNum, routingPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, routingPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}


/**
* @internal prvAu2EventMsgIpUnicast function
* @endinternal
*
* @brief  This function is called for IPv4/6 Unicast type of AU message
*         It translates the AU message from HW format into below according to the type,
*           - CPSS_DXCH_BRG_FDB_MANAGER_IPV4_UC_ROUTING_INFO_STC or
*           - CPSS_DXCH_BRG_FDB_MANAGER_IPV6_UC_ROUTING_INFO_STC
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; AC5.
*
* @param[in]  devNum                  - number of device whose descriptor queue is processed
* @param[in]  auMsgArrayOfWordsPtr    - (pointer to) the AU message in HW format filled by PP.
* @param[out] ipv4UnicastPtr          - (pointer to) FDB Manager IPv4 unicast routing format.
* @param[out] ipv6UnicastPtr          - (pointer to) FDB Manager IPv6 unicast routing format.
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
static GT_STATUS prvAu2EventMsgIpUnicast
(
    IN  GT_U8                                               devNum,
    IN  GT_U32                                             *auMsgArrayOfWordsPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_IPV4_UC_ROUTING_INFO_STC *ipv4UnicastPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_IPV6_UC_ROUTING_INFO_STC *ipv6UnicastPtr
)
{
    GT_U32                                                      wordNum;
    GT_U32                                                      hwValue;
    GT_HW_DEV_NUM                                               associatedHwDevNum;
    GT_U32                                                      associatedVlanId;
    GT_U32                                                      vrfId;
    GT_BOOL                                                     age;
    GT_BOOL                                                     isSip6 = PRV_CPSS_SIP_6_CHECK_MAC(devNum)?GT_TRUE:GT_FALSE;
    CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_ENT              *ucRouteTypePtr;

    CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_UNT *ucRouteInfoPtr;
    CPSS_DXCH_BRG_FDB_MANAGER_IP_UC_COMMON_STC                  *ucCommonInfoPtr;

    SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                                  SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TARGET_DEVICE_E,
                                  hwValue);
    associatedHwDevNum = hwValue;

    /* Get Vlan the associated with this entry */
    SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                                  SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_NEXT_HOP_EVLAN_E,
                                  associatedVlanId);
    SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                                  SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_VRF_ID_E,
                                  vrfId);
    SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                                  SIP5_FDB_AU_MSG_TABLE_FIELDS_AGE_E,
                                  hwValue);
    age = BIT2BOOL_MAC(hwValue);

    /* IP Addr */
    if (ipv4UnicastPtr != NULL) /* ipv4Adddr */
    {
        SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                                      SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV4_DIP_E,
                                      hwValue);
        ipv4UnicastPtr->ipv4Addr.arIP[3] = (GT_U8)(hwValue >> 0);
        ipv4UnicastPtr->ipv4Addr.arIP[2] = (GT_U8)(hwValue >> 8);
        ipv4UnicastPtr->ipv4Addr.arIP[1] = (GT_U8)(hwValue >> 16);
        ipv4UnicastPtr->ipv4Addr.arIP[0] = (GT_U8)(hwValue >> 24);
        ucRouteInfoPtr = &ipv4UnicastPtr->ucRouteInfo;
        ucRouteTypePtr = &ipv4UnicastPtr->ucRouteType;
        ucCommonInfoPtr= &ipv4UnicastPtr->ucCommonInfo;
        ipv4UnicastPtr->vrfId = vrfId;
        ipv4UnicastPtr->age = age;
    }
    else
    {/* ipv6Addr */
        for(wordNum = 0; wordNum < 4; wordNum++)
        {
            if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                   SIP6_FDB_AU_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_31_0_E + wordNum,
                   hwValue);
            }
            else
            {
                SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                   SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_0_E + wordNum,
                   hwValue);
            }
            ipv6UnicastPtr->ipv6Addr.arIP[((3 - wordNum) * 4)]     = (GT_U8)(hwValue >> 24);
            ipv6UnicastPtr->ipv6Addr.arIP[((3 - wordNum) * 4) + 1] = (GT_U8)(hwValue >> 16);
            ipv6UnicastPtr->ipv6Addr.arIP[((3 - wordNum) * 4) + 2] = (GT_U8)(hwValue >> 8);
            ipv6UnicastPtr->ipv6Addr.arIP[((3 - wordNum) * 4) + 3] = (GT_U8)(hwValue);
        }
        ucRouteInfoPtr = &ipv6UnicastPtr->ucRouteInfo;
        ucRouteTypePtr = &ipv6UnicastPtr->ucRouteType;
        ucCommonInfoPtr= &ipv6UnicastPtr->ucCommonInfo;
        ipv6UnicastPtr->vrfId = vrfId;
        ipv6UnicastPtr->age = age;
    }

    /* dstInterface */
    SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                                  SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_USE_VIDX_E,
                                  hwValue);
    if(hwValue == 1)
    {
        ucRouteInfoPtr->fullFdbInfo.dstInterface.type = CPSS_INTERFACE_VIDX_E;

        /* get vidx value */
        SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                                      SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_EVIDX_E,
                                      hwValue);
        ucRouteInfoPtr->fullFdbInfo.dstInterface.interfaceInfo.vidx = (GT_U16)hwValue;

        if(ucRouteInfoPtr->fullFdbInfo.dstInterface.interfaceInfo.vidx==FLOOD_VIDX_CNS)
        {
            ucRouteInfoPtr->fullFdbInfo.dstInterface.type   = CPSS_INTERFACE_VID_E;
        }
    }
    else /* not a vidx */
    {
         SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                                       SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IS_TRUNK_E,
                                       hwValue);
         if(hwValue==1)/* trunk interface */
         {
             SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                                           SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TRUNK_NUM_E,
                                           hwValue);
             ucRouteInfoPtr->fullFdbInfo.dstInterface.interfaceInfo.trunkId          = (GT_TRUNK_ID)hwValue;
             ucRouteInfoPtr->fullFdbInfo.dstInterface.interfaceInfo.devPort.hwDevNum = associatedHwDevNum;
             ucRouteInfoPtr->fullFdbInfo.dstInterface.type                           = CPSS_INTERFACE_TRUNK_E;
         }
         else/* port interface */
         {
             SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                                           SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_EPORT_NUM_E,
                                           hwValue);
            ucRouteInfoPtr->fullFdbInfo.dstInterface.interfaceInfo.devPort.portNum  = hwValue;
            ucRouteInfoPtr->fullFdbInfo.dstInterface.interfaceInfo.devPort.hwDevNum = associatedHwDevNum;
            ucRouteInfoPtr->fullFdbInfo.dstInterface.type                           = CPSS_INTERFACE_PORT_E;
         }
    }

    /* nextHopVlanId */
    ucRouteInfoPtr->fullFdbInfo.nextHopVlanId = associatedVlanId;

    /* ttl/hop decrement enable */
    SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                                  SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_DEC_TTL_OR_HOP_COUNT_E,
                                  hwValue);
    ucRouteInfoPtr->fullFdbInfo.ttlHopLimitDecEnable =  BIT2BOOL_MAC(hwValue);

    /* Bypass TTL Options Or Hop Extension */
    SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                                  SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_BYPASS_TTL_OPTIONS_OR_HOP_EXTENSION_E,
                                  hwValue);
    ucRouteInfoPtr->fullFdbInfo.ttlHopLimDecOptionsExtChkByPass =  BIT2BOOL_MAC(hwValue);

    /* countSet */
    SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                                  SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_COUNTER_SET_INDEX_E,
                                  hwValue);
    switch(hwValue)
    {
        case 0:
            ucRouteInfoPtr->fullFdbInfo.countSet = CPSS_IP_CNT_SET0_E;
            break;
        case 1:
            ucRouteInfoPtr->fullFdbInfo.countSet = CPSS_IP_CNT_SET1_E;
            break;
        case 2:
            ucRouteInfoPtr->fullFdbInfo.countSet = CPSS_IP_CNT_SET2_E;
            break;
        case 3:
            ucRouteInfoPtr->fullFdbInfo.countSet = CPSS_IP_CNT_NO_SET_E;
            break;
        case 7:
            ucRouteInfoPtr->fullFdbInfo.countSet = CPSS_IP_CNT_NO_SET_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(hwValue);
    }

    /* ICMPRedirectEnable */
    SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                                  SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_ICMP_REDIRECT_EXCEP_MIRROR_EN_E,
                                  hwValue);
    ucRouteInfoPtr->fullFdbInfo.ICMPRedirectEnable = BIT2BOOL_MAC(hwValue);

    /* MTU profile index */
    SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
            SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_MTU_INDEX_E,
            hwValue);
    ucRouteInfoPtr->fullFdbInfo.mtuProfileIndex = hwValue;

    if(isSip6)
    {
        /* Route Type */
        SIP6_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_ROUTE_TYPE_E,
                hwValue);
        if(hwValue)
        {
            /* Pointer */
            SIP6_FDB_AU_MSG_FIELD_GET_MAC(devNum, auMsgArrayOfWordsPtr,
                    SIP6_FDB_AU_MSG_TABLE_FIELDS_ROUTE_POINTER_TYPE_E,
                    hwValue);
            if(hwValue)
            {
                /* NHE */
                SIP6_FDB_AU_MSG_FIELD_GET_MAC(devNum, auMsgArrayOfWordsPtr,
                        SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ECMP_OR_NHE_POINTER_E,
                        hwValue);
                ucRouteInfoPtr->nextHopPointerToRouter = hwValue;
                *ucRouteTypePtr = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_POINTER_TO_ROUTER_E;
            }
            else
            {
                /* ECMP */
                SIP6_FDB_AU_MSG_FIELD_GET_MAC(devNum, auMsgArrayOfWordsPtr,
                        SIP6_FDB_FDB_TABLE_FIELDS_UC_ROUTE_ECMP_OR_NHE_POINTER_E,
                        hwValue);
                ucRouteInfoPtr->ecmpPointerToRouter = hwValue;
                *ucRouteTypePtr = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_ECMP_POINTER_TO_ROUTER_E;
            }
        }
        else
        {
            /* NHE entry */
            SIP6_FDB_AU_MSG_FIELD_GET_MAC(devNum, auMsgArrayOfWordsPtr,
                    SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_START_OF_TUNNEL_E,
                    hwValue);
            if(hwValue)
            {
                SIP6_FDB_AU_MSG_FIELD_GET_MAC(devNum, auMsgArrayOfWordsPtr,
                        SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TS_IS_NAT_E,
                        hwValue);
                if(hwValue)
                {
                    SIP6_FDB_AU_MSG_FIELD_GET_MAC(devNum, auMsgArrayOfWordsPtr,
                            SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TUNNEL_PTR_E,
                            hwValue);
                    ucRouteInfoPtr->fullFdbInfo.pointer.natPointer = hwValue;
                    *ucRouteTypePtr = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_NAT_PTR_E;
                }
                else
                {
                    SIP6_FDB_AU_MSG_FIELD_GET_MAC(devNum, auMsgArrayOfWordsPtr,
                            SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TUNNEL_PTR_E,
                            hwValue);
                    ucRouteInfoPtr->fullFdbInfo.pointer.tunnelStartPointer = hwValue;
                    *ucRouteTypePtr = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E;
                }
            }
            else
            {
                SIP6_FDB_AU_MSG_FIELD_GET_MAC(devNum, auMsgArrayOfWordsPtr,
                        SIP6_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_ARP_PTR_E,
                        hwValue);
                ucRouteInfoPtr->fullFdbInfo.pointer.arpPointer = hwValue;
                *ucRouteTypePtr = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E;
            }
        }
    }
    else
    {
        SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_INGRESS_MIRROR_TO_ANALYZER_INDEX_E,
                hwValue);

        if (hwValue==0)
        {
            ucCommonInfoPtr->ingressMirrorToAnalyzerEnable=GT_FALSE;
            ucCommonInfoPtr->ingressMirrorToAnalyzerIndex = 0;
        }
        else
        {
            ucCommonInfoPtr->ingressMirrorToAnalyzerEnable=GT_TRUE;
            ucCommonInfoPtr->ingressMirrorToAnalyzerIndex = hwValue-1;

        }

        SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
            SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_QOS_PROFILE_MARKING_EN_E,
            hwValue);
        ucCommonInfoPtr->qosProfileMarkingEnable = BIT2BOOL_MAC(hwValue);

        SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
            SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_QOS_PROFILE_INDEX_E,
            hwValue);
        ucCommonInfoPtr->qosProfileIndex = hwValue;

        SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
            SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_QOS_PROFILE_PRECEDENCE_E,
            hwValue);
        ucCommonInfoPtr->qosProfilePrecedence = (hwValue == 0)?CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E:CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;

        SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_MODIFY_UP_E,
                hwValue);
        PRV_CPSS_DXCH_FDB_CONVERT_HW_VAL_TO_ATTRIBUTE_MODIFY_MAC(ucCommonInfoPtr->modifyUp, hwValue);

        SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
            SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_MODIFY_DSCP_E,
            hwValue);
        PRV_CPSS_DXCH_FDB_CONVERT_HW_VAL_TO_ATTRIBUTE_MODIFY_MAC(ucCommonInfoPtr->modifyDscp, hwValue);

        SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TUNNEL_START_E,
                hwValue);
        if(hwValue)
        {
            SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                    SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TUNNEL_PTR_E,
                    hwValue);
            ucRouteInfoPtr->fullFdbInfo.pointer.tunnelStartPointer = hwValue;
            *ucRouteTypePtr = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E;
        }
        else
        {
            SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                    SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_ARP_PTR_E,
                    hwValue);
            ucRouteInfoPtr->fullFdbInfo.pointer.arpPointer = hwValue;
            *ucRouteTypePtr = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E;
        }

        SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_ARP_BC_TRAP_MIRROR_EN_E,
                hwValue);
        ucCommonInfoPtr->arpBcTrapMirrorEnable = BIT2BOOL_MAC(hwValue);

        /* dip access level */
        SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_DIP_ACCESS_LEVEL_E,
                hwValue);
        ucCommonInfoPtr->dipAccessLevel = hwValue;
    }

    /* TODO - is this value need to be used SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TUNNEL_TYPE_E
     * PASSENGER_OTHER / PASSENGER_ETHERNET_E */

    /* siteId */
    SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                                  SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_DST_SITE_ID_E,
                                  hwValue);
    if(hwValue == 0)
    {
        ucRouteInfoPtr->fullFdbInfo.ipv6ExtInfo.siteId = CPSS_IP_SITE_ID_INTERNAL_E;
    }
    else
    {
        ucRouteInfoPtr->fullFdbInfo.ipv6ExtInfo.siteId = CPSS_IP_SITE_ID_EXTERNAL_E;
    }

    /* scopeCheckingEnable */
    SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                                  SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_SCOPE_CHECK_E,
                                  hwValue);
    ucRouteInfoPtr->fullFdbInfo.ipv6ExtInfo.scopeCheckingEnable = BIT2BOOL_MAC(hwValue);

    return GT_OK;
}

/**
* @internal prvAu2EventMsgMac_sip4 function
* @endinternal
*
* @brief  This function is called for MAC type of AU message
*         It translates the AU message from HW format into below according to the type,
*           - CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC
*
* @note   APPLICABLE DEVICES:      AC5.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]  devNum                  - number of device whose descriptor queue is processed
* @param[in]  auMsgArrayOfWordsPtr    - (pointer to) the AU message in HW format filled by PP.
* @param[out] fdbEntryMacPtr          - (pointer to) FDB Manager MAC address format.
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
static GT_STATUS prvAu2EventMsgMac_sip4
(
    IN  GT_U32                                               *auMsgArrayOfWordsPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC  *fdbEntryMacPtr
)
{
    GT_U32                                               hwValue;
    GT_U8                                                useVidx = 0;

    /* MacAddr */
    fdbEntryMacPtr->macAddr.arEther[5] = (GT_U8)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[0], 16, 8);
    fdbEntryMacPtr->macAddr.arEther[4] = (GT_U8)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[0], 24, 8);
    fdbEntryMacPtr->macAddr.arEther[3] = (GT_U8)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[1], 0,  8);
    fdbEntryMacPtr->macAddr.arEther[2] = (GT_U8)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[1], 8,  8);
    fdbEntryMacPtr->macAddr.arEther[1] = (GT_U8)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[1], 16, 8);
    fdbEntryMacPtr->macAddr.arEther[0] = (GT_U8)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[1], 24, 8);

    /* Source ID */
    fdbEntryMacPtr->sourceID        = (GT_U8)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[3], 2, 5);
    if(fdbEntryMacPtr->macAddr.arEther[0] & 1)
    {
        useVidx = 1;
    }

    /* FID */
    hwValue = (GT_U16)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[2], 0, 12);
    fdbEntryMacPtr->fid = hwValue;
    fdbEntryMacPtr->vid1= 0;

    /* multiple */
    hwValue = U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[2], 15, 1);
    if(hwValue || useVidx)
    {
        /* Vidx */
        fdbEntryMacPtr->dstInterface.interfaceInfo.vidx =
            (GT_U16)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[2], 17, 12);

        /* multiple or multicast */
        fdbEntryMacPtr->dstInterface.type =
            (fdbEntryMacPtr->dstInterface.interfaceInfo.vidx == 0xFFF)?CPSS_INTERFACE_VID_E:CPSS_INTERFACE_VIDX_E;
    }
    else
    {
        /* isTrunk */
        hwValue = (GT_U8)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[2], 17, 1);
        if(hwValue == 1)
        {
            /* Interface is trunk */
            hwValue = (GT_U8)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[2], 18, 7);
            fdbEntryMacPtr->dstInterface.interfaceInfo.trunkId          = (GT_TRUNK_ID)hwValue;
            fdbEntryMacPtr->dstInterface.type                           = CPSS_INTERFACE_TRUNK_E;
        }
        else
        {
            /* Interface is Dev/Port */
            fdbEntryMacPtr->dstInterface.interfaceInfo.devPort.portNum =
                (GT_U8)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[2], 18, 6);
            fdbEntryMacPtr->dstInterface.interfaceInfo.devPort.hwDevNum =
                U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[3], 7, 5);
            fdbEntryMacPtr->dstInterface.type = CPSS_INTERFACE_PORT_E;
        }
        fdbEntryMacPtr->userDefined     = U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[2], 25, 4);
    }

    fdbEntryMacPtr->daSecurityLevel        = (GT_U8)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[0], 1, 3);
    fdbEntryMacPtr->saSecurityLevel        = (GT_U8)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[0], 12, 3);
    fdbEntryMacPtr->daRoute                = BIT2BOOL_MAC(U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[2], 30, 1));
    fdbEntryMacPtr->isStatic               = BIT2BOOL_MAC(U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[3], 18, 1));
    fdbEntryMacPtr->daQoSParameterSetIndex = (GT_U8)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[3], 15, 3);
    fdbEntryMacPtr->saQoSParameterSetIndex = (GT_U8)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[3], 12, 3);
    /* mirror To Rx Analyzer Port */
    hwValue                                = U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[3], 31, 1);
    switch(hwValue)
    {
        case 0:
            fdbEntryMacPtr->mirrorToAnalyzerPort = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_DISABLED_E;
            break;
        case 1:
            fdbEntryMacPtr->mirrorToAnalyzerPort = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_SA_OR_DA_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(hwValue);
    }

    HW_FORMAT_2_CMD_MAC((GT_U8)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[3], 21, 3),
            fdbEntryMacPtr->daCommand);
    HW_FORMAT_2_CMD_MAC((GT_U8)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[3], 24, 3),
            fdbEntryMacPtr->saCommand);

    /* Age */
    fdbEntryMacPtr->age  = (BIT2BOOL_MAC(U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[2], 13, 1)));

    /* appSpecificCpuCode */
    fdbEntryMacPtr->appSpecificCpuCode = BIT2BOOL_MAC(U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[2], 29, 1));
    return GT_OK;
}

/**
* @internal prvAu2EventMsgIpMulticast_sip4 function
* @endinternal
*
* @brief  This function is called for Multicast type of AU message
*         It translates the AU message from HW format into below according to the type,
*           - CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_IP_MC_FORMAT_STC
*
* @note   APPLICABLE DEVICES:      AC5.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]  devNum                  - number of device whose descriptor queue is processed
* @param[in]  auMsgArrayOfWordsPtr    - (pointer to) the AU message in HW format filled by PP.
* @param[out] fdbEntryMacPtr          - (pointer to) FDB Manager IP multicast format.
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
static GT_STATUS prvAu2EventMsgIpMulticast_sip4
(
    IN  GT_U32                                               *auMsgArrayOfWordsPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_IP_MC_FORMAT_STC     *ipMulticastPtr
)
{
    GT_U32                                               hwValue;
    GT_U8                                                useVidx = 0;

    ipMulticastPtr->dipAddr[3] = (GT_U8)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[0], 16, 8);
    ipMulticastPtr->dipAddr[2] = (GT_U8)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[0], 24, 8);
    ipMulticastPtr->dipAddr[1] = (GT_U8)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[1], 0,  8);
    ipMulticastPtr->dipAddr[0] = (GT_U8)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[1], 8,  8);
    ipMulticastPtr->sipAddr[3] = (GT_U8)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[1], 16, 8);
    ipMulticastPtr->sipAddr[2] = (GT_U8)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[1], 24, 8);
    ipMulticastPtr->sipAddr[1] = (GT_U8)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[3], 0,  8);
    ipMulticastPtr->sipAddr[0] = (GT_U8)(U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[3], 8, 4) |
                                        (U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[3], 27, 4) << 4));
    /*ipmc-ipv4/6*/
    useVidx = 1;

    /* FID */
    hwValue = (GT_U16)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[2], 0, 12);
    ipMulticastPtr->fid = hwValue;
    ipMulticastPtr->vid1= 0;

    /* multiple */
    hwValue = U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[2], 15, 1);
    if(hwValue || useVidx)
    {
        /* Vidx */
        ipMulticastPtr->dstInterface.interfaceInfo.vidx =
            (GT_U16)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[2], 17, 12);

        /* multiple or multicast */
        ipMulticastPtr->dstInterface.type =
            (ipMulticastPtr->dstInterface.interfaceInfo.vidx == 0xFFF)?CPSS_INTERFACE_VID_E:CPSS_INTERFACE_VIDX_E;
    }
    else
    {
        /* isTrunk */
        hwValue = (GT_U8)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[2], 17, 1);
        if(hwValue == 1)
        {
            /* Interface is trunk */
            hwValue = (GT_U8)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[2], 18, 7);
            ipMulticastPtr->dstInterface.interfaceInfo.trunkId          = (GT_TRUNK_ID)hwValue;
            ipMulticastPtr->dstInterface.type                           = CPSS_INTERFACE_TRUNK_E;
        }
        else
        {
            /* Interface is Dev/Port */
            ipMulticastPtr->dstInterface.interfaceInfo.devPort.portNum =
                (GT_U8)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[2], 18, 6);
            ipMulticastPtr->dstInterface.interfaceInfo.devPort.hwDevNum =
                U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[3], 7, 5);
            ipMulticastPtr->dstInterface.type = CPSS_INTERFACE_PORT_E;
        }
        ipMulticastPtr->userDefined     = U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[2], 25, 4);
    }

    ipMulticastPtr->daSecurityLevel        = (GT_U8)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[0], 1, 3);
    ipMulticastPtr->daRoute                = BIT2BOOL_MAC(U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[2], 30, 1));
    ipMulticastPtr->isStatic               = BIT2BOOL_MAC(U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[3], 18, 1));
    ipMulticastPtr->daQoSParameterSetIndex = (GT_U8)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[3], 15, 3);
    ipMulticastPtr->saQoSParameterSetIndex = (GT_U8)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[3], 12, 3);
    /* mirror To Rx Analyzer Port */
    hwValue                                = U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[3], 31, 1);
    switch(hwValue)
    {
        case 0:
            ipMulticastPtr->mirrorToAnalyzerPort = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_DISABLED_E;
            break;
        case 1:
            ipMulticastPtr->mirrorToAnalyzerPort = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_SA_OR_DA_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(hwValue);
    }

    HW_FORMAT_2_CMD_MAC((GT_U8)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[3], 21, 3),
            ipMulticastPtr->daCommand);

    /* Age */
    ipMulticastPtr->age  = (BIT2BOOL_MAC(U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[2], 13, 1)));

    /* appSpecificCpuCode */
    ipMulticastPtr->appSpecificCpuCode = BIT2BOOL_MAC(U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[2], 29, 1));
    return GT_OK;
}

/**
* @internal prvAu2EventMsgMac function
* @endinternal
*
* @brief  This function is called for MAC type of AU message
*         It translates the AU message from HW format into below according to the type,
*           - CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; AC5.
*
* @param[in]  devNum                  - number of device whose descriptor queue is processed
* @param[in]  auMsgArrayOfWordsPtr    - (pointer to) the AU message in HW format filled by PP.
* @param[in]  isMacMove               - Indicates the entry type
*                                           GT_TRUE  - MAC move message, skip the invalid fields for this type.
*                                           GT_FALSE - Not a MAC move message, do not skip.
* @param[out] fdbEntryMacPtr          - (pointer to) FDB Manager MAC address format.
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
static GT_STATUS prvAu2EventMsgMac
(
    IN  GT_U8                                                 devNum,
    IN  GT_U32                                               *auMsgArrayOfWordsPtr,
    IN  GT_BOOL                                               isMacMove,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC  *fdbEntryMacPtr
)
{
    PRV_CPSS_DXCH_FDB_SPECIAL_MUXED_FIELDS_STC           specialFields;             /*special Muxed fields in the AU msg */
    GT_U32                                               hwValue;
    GT_BOOL                                              saMirrorToRxAnalyzerPortEn, daMirrorToRxAnalyzerPortEn;
    GT_BOOL                                              isTrunk;
    GT_BOOL                                              isSip6 = GT_FALSE;
    GT_HW_DEV_NUM                                        associatedHwDevNum;
    HW_FDB_ENTRY_EXT_TYPE_ENT                            macEntryType;

    if(!PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        return prvAu2EventMsgMac_sip4(auMsgArrayOfWordsPtr, fdbEntryMacPtr);
    }
    else if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        isSip6 = GT_TRUE;
    }

    SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                                  SIP5_FDB_AU_MSG_TABLE_FIELDS_DEV_ID_E,
                                  hwValue);
    associatedHwDevNum = hwValue;

    /* MacAddr */
    SIP5_FDB_AU_MSG_FIELD_MAC_ADDR_GET_MAC(devNum, auMsgArrayOfWordsPtr,
            &fdbEntryMacPtr->macAddr.arEther[0]);
    macEntryType = HW_FDB_ENTRY_TYPE_MAC_ADDR_E;

    /* FID */
    SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                                  SIP5_FDB_AU_MSG_TABLE_FIELDS_FID_E,
                                  hwValue);
    fdbEntryMacPtr->fid = hwValue;

    /* special muxed fields */
    prvCpssDxChBrgFdbAuMsgSpecialMuxedFieldsGet(devNum, auMsgArrayOfWordsPtr, macEntryType, 1 /* NA Message */,&specialFields);
    fdbEntryMacPtr->sourceID        = (specialFields.srcId == PRV_CPSS_DXCH_FDB_FIELD_NOT_USED_CNS)?0:specialFields.srcId;
    fdbEntryMacPtr->vid1            = (specialFields.origVid1 == PRV_CPSS_DXCH_FDB_FIELD_NOT_USED_CNS)?0:(GT_U16)specialFields.origVid1;
    fdbEntryMacPtr->userDefined     = specialFields.udb;

    /* dstInterface */
    /* is trunk */
    if(isSip6)
    {
        SIP6_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_IS_TRUNK_E,
                hwValue);
        isTrunk = BIT2BOOL_MAC(hwValue);
        if(isTrunk)
        {
            SIP6_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                    SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_TRUNK_NUM_E,
                    hwValue);
            fdbEntryMacPtr->dstInterface.interfaceInfo.trunkId          = (GT_TRUNK_ID)hwValue;
            fdbEntryMacPtr->dstInterface.type                           = CPSS_INTERFACE_TRUNK_E;
        }
        else
        {
            SIP6_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                    SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_EPORT_NUM_E,
                    hwValue);
            fdbEntryMacPtr->dstInterface.interfaceInfo.devPort.portNum  = hwValue;
            fdbEntryMacPtr->dstInterface.interfaceInfo.devPort.hwDevNum = associatedHwDevNum;
            fdbEntryMacPtr->dstInterface.type                           = CPSS_INTERFACE_PORT_E;
            PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_INTERFACE_MAC(&(fdbEntryMacPtr->dstInterface.interfaceInfo));
        }
    }
    else
    {
        SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                SIP5_FDB_AU_MSG_TABLE_FIELDS_IS_TRUNK_E,
                hwValue);
        isTrunk = BIT2BOOL_MAC(hwValue);
        if(isTrunk)
        {
            SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                    SIP5_FDB_AU_MSG_TABLE_FIELDS_TRUNK_NUM_E,
                    hwValue);
            fdbEntryMacPtr->dstInterface.interfaceInfo.trunkId          = (GT_TRUNK_ID)hwValue;
            fdbEntryMacPtr->dstInterface.type                           = CPSS_INTERFACE_TRUNK_E;
        }
        else
        {
            SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                    SIP5_FDB_AU_MSG_TABLE_FIELDS_EPORT_NUM_E,
                    hwValue);
            fdbEntryMacPtr->dstInterface.interfaceInfo.devPort.portNum  = hwValue;
            fdbEntryMacPtr->dstInterface.interfaceInfo.devPort.hwDevNum = associatedHwDevNum;
            fdbEntryMacPtr->dstInterface.type                           = CPSS_INTERFACE_PORT_E;
            PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_INTERFACE_MAC(&(fdbEntryMacPtr->dstInterface.interfaceInfo));
        }
    }

    /* MAC UPDATE message does not have these Values */
    if(isMacMove)
    {
        fdbEntryMacPtr->daSecurityLevel = 0;
        fdbEntryMacPtr->mirrorToAnalyzerPort = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_DISABLED_E;
        fdbEntryMacPtr->daQoSParameterSetIndex = 0;
        fdbEntryMacPtr->saQoSParameterSetIndex = 0;
        fdbEntryMacPtr->saSecurityLevel = 0;
        fdbEntryMacPtr->saCommand       = CPSS_MAC_TABLE_FRWRD_E;
        fdbEntryMacPtr->daCommand       = CPSS_MAC_TABLE_FRWRD_E;
        fdbEntryMacPtr->isStatic        = GT_FALSE; /* moved entry can't be static */
        fdbEntryMacPtr->daRoute         = GT_FALSE; /* need to get it from the fdb manager if exists */
    }
    else
    {
        fdbEntryMacPtr->daSecurityLevel = (specialFields.daAccessLevel == PRV_CPSS_DXCH_FDB_FIELD_NOT_USED_CNS)?0:specialFields.daAccessLevel;
        fdbEntryMacPtr->saSecurityLevel = (specialFields.saAccessLevel == PRV_CPSS_DXCH_FDB_FIELD_NOT_USED_CNS)?0:specialFields.saAccessLevel;

        if(isSip6)
        {
            /* daCommand */
            SIP6_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                    SIP6_FDB_AU_MSG_TABLE_FIELDS_DA_CMD_E,
                    hwValue);
            SIP6_CONVERT_HW_DA_CMD_TO_SW_VAL_MAC(devNum, fdbEntryMacPtr->daCommand, hwValue);

            /* saCommand */
            SIP6_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                    SIP6_FDB_AU_MSG_TABLE_FIELDS_SA_CMD_E,
                    hwValue);
            SIP6_CONVERT_HW_SA_CMD_TO_SW_VAL_MAC(devNum, fdbEntryMacPtr->saCommand, hwValue);

            /* avoid trash values for not supported fields */
            fdbEntryMacPtr->daQoSParameterSetIndex =
            fdbEntryMacPtr->saQoSParameterSetIndex = 0;
            fdbEntryMacPtr->mirrorToAnalyzerPort = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_DISABLED_E;
        }
        else
        {
            /* daCommand */
            SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                    SIP5_FDB_FDB_TABLE_FIELDS_DA_CMD_E,
                    hwValue);
            PRV_CPSS_DXCH_FDB_CONVERT_HW_VAL_TO_SW_CMD_MAC(fdbEntryMacPtr->daCommand,
                    hwValue);
            /* saCommand */
            SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                    SIP5_FDB_FDB_TABLE_FIELDS_SA_CMD_E,
                    hwValue);
            PRV_CPSS_DXCH_FDB_CONVERT_HW_VAL_TO_SW_CMD_MAC(fdbEntryMacPtr->saCommand,
                    hwValue);

            SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                    SIP5_FDB_AU_MSG_TABLE_FIELDS_DA_QOS_PARAM_SET_IDX_E,
                    hwValue);
            fdbEntryMacPtr->daQoSParameterSetIndex = hwValue;

            SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                    SIP5_FDB_AU_MSG_TABLE_FIELDS_SA_QOS_PARAM_SET_IDX_E,
                    hwValue);
            fdbEntryMacPtr->saQoSParameterSetIndex = hwValue;

            SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                    SIP5_FDB_AU_MSG_TABLE_FIELDS_SA_LOOKUP_INGRESS_MIRROR_TO_ANALYZER_E,
                    hwValue);
            saMirrorToRxAnalyzerPortEn = BIT2BOOL_MAC(hwValue);

            SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                    SIP5_FDB_AU_MSG_TABLE_FIELDS_DA_LOOKUP_INGRESS_MIRROR_TO_ANALYZER_E,
                    hwValue);
            daMirrorToRxAnalyzerPortEn = BIT2BOOL_MAC(hwValue);

            if((saMirrorToRxAnalyzerPortEn == GT_TRUE) && (daMirrorToRxAnalyzerPortEn == GT_TRUE))
            {
                fdbEntryMacPtr->mirrorToAnalyzerPort = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_SA_OR_DA_E;
            }
            else if(saMirrorToRxAnalyzerPortEn == GT_TRUE)
            {
                fdbEntryMacPtr->mirrorToAnalyzerPort = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_SA_E;
            }
            else if(daMirrorToRxAnalyzerPortEn == GT_TRUE)
            {
                fdbEntryMacPtr->mirrorToAnalyzerPort = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_DA_E;
            }
            else
            {
                fdbEntryMacPtr->mirrorToAnalyzerPort = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_DISABLED_E;
            }
        }

        /* isStatic */
        SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                                      SIP5_FDB_AU_MSG_TABLE_FIELDS_IS_STATIC_E,
                                      hwValue);
        fdbEntryMacPtr->isStatic = BIT2BOOL_MAC(hwValue);

        /* daRoute */
        SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                SIP5_FDB_AU_MSG_TABLE_FIELDS_DA_ROUTE_E,
                hwValue);
        fdbEntryMacPtr->daRoute = BIT2BOOL_MAC(hwValue);
    }

    /* Age */
    SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                                  SIP5_FDB_AU_MSG_TABLE_FIELDS_AGE_E,
                                  hwValue);
    fdbEntryMacPtr->age = BIT2BOOL_MAC(hwValue);

    /* appSpecificCpuCode */
    SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                                  SIP5_FDB_AU_MSG_TABLE_FIELDS_APP_SPECIFIC_CPU_CODE_E,
                                  hwValue);
    fdbEntryMacPtr->appSpecificCpuCode = BIT2BOOL_MAC(hwValue);
    return GT_OK;
}

/**
* @internal prvAu2EventMsgIpMulticast function
* @endinternal
*
* @brief  This function is called for Multicast type of AU message
*         It translates the AU message from HW format into below according to the type,
*           - CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_IP_MC_FORMAT_STC
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; AC5.
*
* @param[in]  devNum                  - number of device whose descriptor queue is processed
* @param[in]  auMsgArrayOfWordsPtr    - (pointer to) the AU message in HW format filled by PP.
* @param[out] fdbEntryMacPtr          - (pointer to) FDB Manager IP multicast format.
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
static GT_STATUS prvAu2EventMsgIpMulticast
(
    IN  GT_U8                                                 devNum,
    IN  GT_U32                                               *auMsgArrayOfWordsPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_IP_MC_FORMAT_STC     *ipMulticastPtr
)
{
    PRV_CPSS_DXCH_FDB_SPECIAL_MUXED_FIELDS_STC           specialFields;             /*special Muxed fields in the AU msg */
    GT_U32                                               hwValue;
    GT_BOOL                                              isTrunk;
    GT_BOOL                                              isSip6 = GT_FALSE;
    GT_HW_DEV_NUM                                        associatedHwDevNum;
    HW_FDB_ENTRY_EXT_TYPE_ENT                            macEntryType;
    GT_BOOL                                              saMirrorToRxAnalyzerPortEn, daMirrorToRxAnalyzerPortEn;

    if(!PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        return prvAu2EventMsgIpMulticast_sip4(auMsgArrayOfWordsPtr, ipMulticastPtr);
    }
    else if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        isSip6 = GT_TRUE;
    }

    SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                                  SIP5_FDB_AU_MSG_TABLE_FIELDS_DEV_ID_E,
                                  hwValue);
    associatedHwDevNum = hwValue;

    /* SIP */
    SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
            SIP5_FDB_AU_MSG_TABLE_FIELDS_SIP_E,
            hwValue);
    ipMulticastPtr->sipAddr[3] = (GT_U8)(hwValue >> 0);
    ipMulticastPtr->sipAddr[2] = (GT_U8)(hwValue >> 8);
    ipMulticastPtr->sipAddr[1] = (GT_U8)(hwValue >> 16);
    ipMulticastPtr->sipAddr[0] = (GT_U8)(hwValue >> 24);

    /* DIP */
    SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
            SIP5_FDB_AU_MSG_TABLE_FIELDS_DIP_E,
            hwValue);
    ipMulticastPtr->dipAddr[3] = (GT_U8)(hwValue >> 0);
    ipMulticastPtr->dipAddr[2] = (GT_U8)(hwValue >> 8);
    ipMulticastPtr->dipAddr[1] = (GT_U8)(hwValue >> 16);
    ipMulticastPtr->dipAddr[0] = (GT_U8)(hwValue >> 24);
    macEntryType = HW_FDB_ENTRY_TYPE_IPV4_MCAST_E;

    /* FID */
    SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                                  SIP5_FDB_AU_MSG_TABLE_FIELDS_FID_E,
                                  hwValue);
    ipMulticastPtr->fid = hwValue;

    /* special muxed fields */
    prvCpssDxChBrgFdbAuMsgSpecialMuxedFieldsGet(devNum, auMsgArrayOfWordsPtr, macEntryType, 1 /* NA Message */,&specialFields);
    ipMulticastPtr->sourceID        = (specialFields.srcId == PRV_CPSS_DXCH_FDB_FIELD_NOT_USED_CNS)?0:specialFields.srcId;
    ipMulticastPtr->vid1            = (specialFields.origVid1 == PRV_CPSS_DXCH_FDB_FIELD_NOT_USED_CNS)?0:(GT_U16)specialFields.origVid1;
    ipMulticastPtr->userDefined     = specialFields.udb;

    /* dstInterface */
    /* is trunk */
    if(isSip6)
    {
        SIP6_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_IS_TRUNK_E,
                hwValue);
        isTrunk = BIT2BOOL_MAC(hwValue);
        if(isTrunk)
        {
            SIP6_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                    SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_TRUNK_NUM_E,
                    hwValue);
            ipMulticastPtr->dstInterface.interfaceInfo.trunkId          = (GT_TRUNK_ID)hwValue;
            ipMulticastPtr->dstInterface.type                           = CPSS_INTERFACE_TRUNK_E;
        }
        else
        {
            SIP6_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                    SIP6_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_EPORT_NUM_E,
                    hwValue);
            ipMulticastPtr->dstInterface.interfaceInfo.devPort.portNum  = hwValue;
            ipMulticastPtr->dstInterface.interfaceInfo.devPort.hwDevNum = associatedHwDevNum;
            ipMulticastPtr->dstInterface.type                           = CPSS_INTERFACE_PORT_E;
            PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_INTERFACE_MAC(&(ipMulticastPtr->dstInterface.interfaceInfo));
        }
    }
    else
    {
        SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                SIP5_FDB_AU_MSG_TABLE_FIELDS_IS_TRUNK_E,
                hwValue);
        isTrunk = BIT2BOOL_MAC(hwValue);
        if(isTrunk)
        {
            SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                    SIP5_FDB_AU_MSG_TABLE_FIELDS_TRUNK_NUM_E,
                    hwValue);
            ipMulticastPtr->dstInterface.interfaceInfo.trunkId          = (GT_TRUNK_ID)hwValue;
            ipMulticastPtr->dstInterface.type                           = CPSS_INTERFACE_TRUNK_E;
        }
        else
        {
            SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                    SIP5_FDB_AU_MSG_TABLE_FIELDS_EPORT_NUM_E,
                    hwValue);
            ipMulticastPtr->dstInterface.interfaceInfo.devPort.portNum  = hwValue;
            ipMulticastPtr->dstInterface.interfaceInfo.devPort.hwDevNum = associatedHwDevNum;
            ipMulticastPtr->dstInterface.type                           = CPSS_INTERFACE_PORT_E;
            PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_INTERFACE_MAC(&(ipMulticastPtr->dstInterface.interfaceInfo));
        }

        SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                SIP5_FDB_AU_MSG_TABLE_FIELDS_DA_QOS_PARAM_SET_IDX_E,
                hwValue);
        ipMulticastPtr->daQoSParameterSetIndex = hwValue;

        SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                SIP5_FDB_AU_MSG_TABLE_FIELDS_SA_QOS_PARAM_SET_IDX_E,
                hwValue);
        ipMulticastPtr->saQoSParameterSetIndex = hwValue;

        SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                SIP5_FDB_AU_MSG_TABLE_FIELDS_SA_LOOKUP_INGRESS_MIRROR_TO_ANALYZER_E,
                hwValue);
        saMirrorToRxAnalyzerPortEn = BIT2BOOL_MAC(hwValue);

        SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                SIP5_FDB_AU_MSG_TABLE_FIELDS_DA_LOOKUP_INGRESS_MIRROR_TO_ANALYZER_E,
                hwValue);
        daMirrorToRxAnalyzerPortEn = BIT2BOOL_MAC(hwValue);

        if((saMirrorToRxAnalyzerPortEn == GT_TRUE) && (daMirrorToRxAnalyzerPortEn == GT_TRUE))
        {
            ipMulticastPtr->mirrorToAnalyzerPort = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_SA_OR_DA_E;
        }
        else if(saMirrorToRxAnalyzerPortEn == GT_TRUE)
        {
            ipMulticastPtr->mirrorToAnalyzerPort = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_SA_E;
        }
        else if(daMirrorToRxAnalyzerPortEn == GT_TRUE)
        {
            ipMulticastPtr->mirrorToAnalyzerPort = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_DA_E;
        }
        else
        {
            ipMulticastPtr->mirrorToAnalyzerPort = CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_DISABLED_E;
        }
    }

    ipMulticastPtr->daSecurityLevel = (specialFields.daAccessLevel == PRV_CPSS_DXCH_FDB_FIELD_NOT_USED_CNS)?0:specialFields.daAccessLevel;

    if(isSip6)
    {
        /* daCommand */
        SIP6_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                SIP6_FDB_AU_MSG_TABLE_FIELDS_DA_CMD_E,
                hwValue);
        SIP6_CONVERT_HW_DA_CMD_TO_SW_VAL_MAC(devNum, ipMulticastPtr->daCommand, hwValue);
    }
    else
    {
        /* daCommand */
        SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                SIP5_FDB_FDB_TABLE_FIELDS_DA_CMD_E,
                hwValue);
        PRV_CPSS_DXCH_FDB_CONVERT_HW_VAL_TO_SW_CMD_MAC(ipMulticastPtr->daCommand,
                hwValue);
    }

    /* isStatic */
    SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
            SIP5_FDB_AU_MSG_TABLE_FIELDS_IS_STATIC_E,
            hwValue);
    ipMulticastPtr->isStatic = BIT2BOOL_MAC(hwValue);

    /* daRoute */
    SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
            SIP5_FDB_AU_MSG_TABLE_FIELDS_DA_ROUTE_E,
            hwValue);
    ipMulticastPtr->daRoute = BIT2BOOL_MAC(hwValue);

    /* Age */
    SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                                  SIP5_FDB_AU_MSG_TABLE_FIELDS_AGE_E,
                                  hwValue);
    ipMulticastPtr->age = BIT2BOOL_MAC(hwValue);

    /* appSpecificCpuCode */
    SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                                  SIP5_FDB_AU_MSG_TABLE_FIELDS_APP_SPECIFIC_CPU_CODE_E,
                                  hwValue);
    ipMulticastPtr->appSpecificCpuCode = BIT2BOOL_MAC(hwValue);
    return GT_OK;
}

/**
* @internal prvAu2EventMsgUpdateEventInfo_sip4 function
* @endinternal
*
* @brief  This function is called for all type of AU message
*         It translates the AU message from HW format into event format
*           - CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC or
*           - CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_IP_MC_FORMAT_STC
*
* @note   APPLICABLE DEVICES:      AC5.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
*
* @param[in]  devNum                  - number of device whose descriptor queue is processed
* @param[in]  auMsgArrayOfWordsPtr    - (pointer to) the AU message in HW format filled by PP.
* @param[out] eventPtr                - (pointer to) FDB Manager event format.
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
static GT_STATUS prvAu2EventMsgUpdateEventInfo_sip4
(
    IN    GT_U32                                          *auMsgArrayOfWordsPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC  *eventPtr
)
{
    GT_U32     hwValue;

    /* get Message Type */
    hwValue = (CPSS_UPD_MSG_TYPE_ENT)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[0], 4, 3);
    switch(hwValue)
    {
        case CPSS_QA_E:
        case CPSS_QR_E:
        case CPSS_AA_E:
        case CPSS_TA_E:
        case CPSS_SA_E:
        case CPSS_QI_E:
        case CPSS_FU_E:
        case CPSS_HR_E:
            return GT_OK;
        case CPSS_NA_E: /* Valid Case */
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(hwValue);
    }

    /* mac no space */
    hwValue = U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[2], 31, 1);
    if(hwValue == 1)
    {
        eventPtr->updateType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_NO_SPACE_E;
        return GT_OK;
    }

    /* tempEntryOffset */
    eventPtr->updateType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_NEW_E;
    eventPtr->updateInfo.newEntryFormat.tempEntryOffset = U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[0], 7, 5);
    return GT_OK;
}


/**
* @internal prvAu2EventMsgUpdateEventInfo function
* @endinternal
*
* @brief  This function is called for all type of AU message
*         It translates the AU message from HW format into event format
*           - CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC or
*           - CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_IP_MC_FORMAT_STC
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in]  devNum                  - number of device whose descriptor queue is processed
* @param[in]  auMsgArrayOfWordsPtr    - (pointer to) the AU message in HW format filled by PP.
* @param[out] eventPtr                - (pointer to) FDB Manager event format.
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
static GT_STATUS prvAu2EventMsgUpdateEventInfo
(
    IN    GT_U8                                            devNum,
    IN    GT_U32                                          *auMsgArrayOfWordsPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC  *eventPtr
)
{
    GT_U32     hwValue;

    /* get Message Type */
    SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum, auMsgArrayOfWordsPtr,
            SIP5_FDB_AU_MSG_TABLE_FIELDS_MSG_TYPE_E,
            hwValue);
    switch(hwValue)
    {
        case CPSS_QA_E:
        case CPSS_QR_E:
        case CPSS_AA_E:
        case CPSS_TA_E:
        case CPSS_SA_E:
        case CPSS_QI_E:
        case CPSS_FU_E:
        case CPSS_HR_E:
            return GT_OK;
        case CPSS_NA_E: /* Valid Case */
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(hwValue);
    }

    /* mac no space */
    SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                                  SIP5_FDB_AU_MSG_TABLE_FIELDS_CHAIN_TOO_LONG_E,
                                  hwValue);
    if(hwValue == 1)
    {
        eventPtr->updateType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_NO_SPACE_E;
        return GT_OK;
    }

    /* IS NA Moved */
    SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                                  SIP5_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_IS_MOVED_E,
                                  hwValue);
    if(hwValue == 0)
    {/* NEW */
        /* tempEntryOffset */
        eventPtr->updateType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_NEW_E;
        SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                SIP5_FDB_AU_MSG_TABLE_FIELDS_MAC_ADDR_OFFSET_E,
                hwValue);
        eventPtr->updateInfo.newEntryFormat.tempEntryOffset = hwValue;
        return GT_OK;
    }

    /* Update */
    eventPtr->updateType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_MOVED_E;

    SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
            SIP5_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_OLD_DEVICE_E,
            hwValue);
    eventPtr->updateInfo.movedEntryFormat.oldInterface.interfaceInfo.devPort.hwDevNum = hwValue;

    SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
            SIP5_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_OLD_IS_TRUNK_E,
            hwValue);
    if(hwValue == 1)
    {
        SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                SIP5_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_OLD_TRUNK_NUM_E,
                hwValue);
        eventPtr->updateInfo.movedEntryFormat.oldInterface.interfaceInfo.trunkId          = (GT_TRUNK_ID)hwValue;
        eventPtr->updateInfo.movedEntryFormat.oldInterface.type                           = CPSS_INTERFACE_TRUNK_E;
    }
    else
    {

        SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
                SIP5_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_OLD_EPORT_E,
                hwValue);
        eventPtr->updateInfo.movedEntryFormat.oldInterface.interfaceInfo.devPort.portNum  = hwValue;
        eventPtr->updateInfo.movedEntryFormat.oldInterface.type                           = CPSS_INTERFACE_PORT_E;
    }

    SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum,auMsgArrayOfWordsPtr,
            SIP5_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_OLD_SRC_ID_E,
            hwValue);
    eventPtr->updateInfo.movedEntryFormat.oldsrcId = hwValue;
    return GT_OK;
}

/**
* @internal au2EventMsg function
* @endinternal
*
* @brief  This function is called whenever an address update message is received.
*         It translates the AU messages from HW format into CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC format
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in]  devNum                  - number of device whose descriptor queue is processed
* @param[in]  hwAuMessagesPtr         - (pointer to) the AU descriptor in HW format filled by PP.
* @param[out] eventPtr                - (pointer to) FDB Manager update event.
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
static GT_STATUS au2EventMsg
(
    IN  GT_U8                                             devNum,
    IN  PRV_CPSS_AU_DESC_EXT_8_STC                       *hwAuMessagesPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC *eventPtr
)
{
    GT_U32                              *auMsgArrayOfWordsPtr;      /*consider the AU message as array of words */
    GT_U32                              hwValue;
    GT_STATUS                           rc;
    GT_BOOL                             isMacMove;

    /* consider the AU message as array of words .
       by this we can treat it as 'fields format in table entry' */
    auMsgArrayOfWordsPtr = (GT_U32*)(&hwAuMessagesPtr->elem0);

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        rc = prvAu2EventMsgUpdateEventInfo(devNum, auMsgArrayOfWordsPtr, eventPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
        /* get entry type */
        SIP5_FDB_AU_MSG_FIELD_GET_MAC(devNum, auMsgArrayOfWordsPtr,
                SIP5_FDB_AU_MSG_TABLE_FIELDS_FDB_ENTRY_TYPE_E,
                hwValue);
    }
    else
    {
        rc = prvAu2EventMsgUpdateEventInfo_sip4(auMsgArrayOfWordsPtr, eventPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
        /* get entry type */
        hwValue = (GT_U8)U32_GET_FIELD_MAC(auMsgArrayOfWordsPtr[3], 19, 2);
    }

    switch(hwValue)
    {
        case CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E:
        case CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_FID_VID1_E:
            eventPtr->entry.fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E;
            isMacMove = (eventPtr->updateType == CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_MOVED_E)?GT_TRUE:GT_FALSE;
            prvAu2EventMsgMac(devNum, auMsgArrayOfWordsPtr, isMacMove, &eventPtr->entry.format.fdbEntryMacAddrFormat);
            break;
        case CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E:
        case CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_FID_VID1_E:
            eventPtr->entry.fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E;
            prvAu2EventMsgIpMulticast(devNum, auMsgArrayOfWordsPtr, &eventPtr->entry.format.fdbEntryIpv4McFormat);
            break;
        case CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_E:
        case CPSS_MAC_ENTRY_EXT_TYPE_IPV6_MCAST_FID_VID1_E:
            eventPtr->entry.fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E;
            prvAu2EventMsgIpMulticast(devNum, auMsgArrayOfWordsPtr, &eventPtr->entry.format.fdbEntryIpv6McFormat);
            break;
        case CPSS_MAC_ENTRY_EXT_TYPE_IPV4_UC_E:
            eventPtr->entry.fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E;
            prvAu2EventMsgIpUnicast(devNum, auMsgArrayOfWordsPtr, &eventPtr->entry.format.fdbEntryIpv4UcFormat, NULL);
            break;
        case CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_ADDR_ENTRY_E:
        case CPSS_MAC_ENTRY_EXT_TYPE_IPV6_UC_DATA_ENTRY_E:
            eventPtr->entry.fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E;
            prvAu2EventMsgIpUnicast(devNum, auMsgArrayOfWordsPtr, NULL, &eventPtr->entry.format.fdbEntryIpv6UcFormat);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(hwValue);
    }
    return GT_OK;
}

/**
* @internal prvDxChBrgFdbManagerMacNoSpaceCacheUpdate function
* @endinternal
*
* @brief  This function is called for MAC no space updates.
*         If Its exist in cache return, if not replace with the oldest entry
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in]  fdbManagerId            - the FDB Manager id.
*                                       (APPLICABLE RANGES : 0..31)
* @param[in]  hwAuMessagesPtr         - (pointer to) the AU descriptor in HW format filled by PP.
* @param[out] eventPtr                - (pointer to) FDB Manager update event.
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
static GT_STATUS prvDxChBrgFdbManagerMacNoSpaceCacheUpdate
(
    IN  GT_U32                         fdbManagerId,
    IN  PRV_CPSS_AU_DESC_EXT_8_STC    *hwAuMessagePtr,
    OUT GT_BOOL                       *macNoSpaceTryAdding
)
{
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr;
    PRV_CPSS_DXCH_BRG_FDB_MANAGER_NO_SPACE_CACHE_STC *cache;
    GT_U32                                            i;
    GT_U8                                             key;
    GT_U32                                            macAddrOffsetMask;

    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);
    cache = &fdbManagerPtr->noSpaceCacheEntry;

    /* mask not used for "No Space" NA's and random value of MACAddrOffset: bits 11-15
     * FOR AC5 - 7-11 */
    macAddrOffsetMask = IS_FDB_MANAGER_FOR_SIP_4(fdbManagerPtr)?FIELD_MASK_NOT_MAC(7,5):FIELD_MASK_NOT_MAC(11,5);
    hwAuMessagePtr->elem0.word0 &= macAddrOffsetMask;

    /* Check if entry exist */
    for(i=0; i<cache->usedEntriesCnt; i++)
    {
        if(cpssOsMemCmp(&cache->noSpaceKeyCache[i], hwAuMessagePtr, PRV_CPSS_DXCH_FDB_AU_NA_TO_CPU_MSG_DATA_BYTES_SIZE_CNS) == 0)
        {
            /* Entry exist - Add indication might have sent to CPU, so ignore */
            *macNoSpaceTryAdding = GT_FALSE;
            return GT_OK;
        }
    }

    /* Entry not exist, Need to write */
    if(cache->usedEntriesCnt < 4)
    {
        key = cache->usedEntriesCnt;
        cache->usedEntriesCnt += 1;
    }
    else
    {
        key = (cache->lastUpdatedEntry+1)%4;
    }

    /* 6 words are used for AU MSG */
    cpssOsMemCpy(&cache->noSpaceKeyCache[key], &(hwAuMessagePtr->elem0), PRV_CPSS_DXCH_FDB_AU_NA_TO_CPU_MSG_DATA_BYTES_SIZE_CNS);

    /* Update the last updated entry */
    cache->lastUpdatedEntry = key;
    *macNoSpaceTryAdding = GT_TRUE;
    return GT_OK;
}

/**
* @internal prvDxChBrgFdbManager_modifyEntryToValid function
* @endinternal
*
* @brief  This function is called to modify params, to pass validation in case of hybrid model
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Ironman; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Harrier; AC5.
*
* @param[inout] eventPtr          - (pointer to) FDB Manager update event.
*                                    GT_OK if successful, or GT_FAIL otherwise.
*/
static GT_STATUS prvDxChBrgFdbManager_modifyEntryToValid
(
    INOUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC *eventPtr
)
{
    switch(eventPtr->entry.fdbEntryType)
    {
        default:
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E:
            eventPtr->entry.format.fdbEntryMacAddrFormat.userDefined     = 0;
            eventPtr->entry.format.fdbEntryMacAddrFormat.vid1            = 0;
            eventPtr->entry.format.fdbEntryMacAddrFormat.daSecurityLevel = 0;
            eventPtr->entry.format.fdbEntryMacAddrFormat.saSecurityLevel = 0;
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E:
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E:
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E:
        case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E:
            break;
    }
    return GT_OK;
}

/**
* @internal prvDxChBrgFdbManagerAuMsgHandle function
* @endinternal
*
* @brief  This function is called for processing of the AU update messages.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in]  devNum                  - number of device whose descriptor queue is processed
* @param[in]  fdbManagerId            - the FDB Manager id.
*                                       (APPLICABLE RANGES : 0..31)
* @param[in]  hwAuMessagesPtr         - (pointer to) the AU descriptor in HW format filled by PP.
* @param[in]  portGroupIdPtr          - (pointer to) the portGroupIds the Au messages belongs to.
* @param[in]  numOfAu                 - num of AU messages to be processed.
* @param[in]  scanParamsPtr           - (pointer to) the learning scan attributes.
* @param[out] entriesLearningArray - (pointer to) Update event array.
* @param[out] entriesLearningNumPtr- (pointer to) Number of Update events filled in array.
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise.
*/
static GT_STATUS prvDxChBrgFdbManagerAuMsgHandle
(
    IN  GT_U8                                               devNum,
    IN  GT_U32                                              fdbManagerId,
    IN  PRV_CPSS_AU_DESC_EXT_8_STC                         *hwAuMessagesPtr,
    IN  GT_U32                                             *portGroupIdPtr,
    IN  GT_U32                                              numOfAu,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_SCAN_PARAMS_STC *scanParamsPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC   *entriesLearningArray,
    INOUT GT_U32                                           *entriesLearningNumPtr
)
{
    GT_U32                                            eventArrIndex;/* Inedx to the OUT event Array - only updates successful entries */
    GT_U32                                            auIndex;      /* Index to the hwAuMessages */
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC    addParams;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC updateParams;
    GT_STATUS                                         rc = GT_OK;
    GT_STATUS                                         addUpdateStatus = GT_OK;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC *eventPtr;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC  tempUpdateEvent;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC    *dbEntryPtr;   /* pointer to the entry in the DB , in case of 'moved entry' */
    GT_U32                                            bankIndex;
    GT_BOOL                                           macNoSpaceTryAdding; /* GT_FALSE - exist in cache
                                                                              GT_TRUE  - Added to cache */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC  calcInfo;

#ifndef PRV_CPSS_DXCH_BRG_FDB_MANAGER_ALLOW_HW_BANK_COUNTER_UPDATE_CNS
    GT_UNUSED_PARAM(portGroupIdPtr);
#endif
    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);
    eventArrIndex = (entriesLearningNumPtr == NULL)?0:*entriesLearningNumPtr;

    for (auIndex = fdbManagerPtr->tempInfo.lastHwAuMessageIndex; auIndex < numOfAu; auIndex++)
    {
        /** In case of AU Message is not valid, Add/Update returns error,
          * To indicate to application which entry returned error.
          *      Learning scan return error and 'OUT event Array' last entry will be the problematic entry.
          */
        if(addUpdateStatus == GT_BAD_PARAM)
        {
            break;
        }

        /* entriesLearningArray is optional, so use temp to process the message */
        eventPtr = (entriesLearningArray == NULL)?&tempUpdateEvent:&entriesLearningArray[eventArrIndex];

        /* parse the AU message from HW format to event format */
        rc = au2EventMsg(devNum, &(hwAuMessagesPtr[auIndex]), eventPtr);
        if (rc != GT_OK)
        {
            continue;
        }
        if(fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E)
        {
            rc = prvDxChBrgFdbManager_modifyEntryToValid(eventPtr);
            if(rc != GT_OK)
            {
                continue;
            }
        }

        if(eventPtr->entry.fdbEntryType == CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E)
        {
            CPSS_LOG_INFORMATION_MAC("NA MSG : [%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x]fid[%d] inFaceType[%d] param0[%d] , param1[%d]\n",
                eventPtr->entry.format.fdbEntryMacAddrFormat.macAddr.arEther[0],
                eventPtr->entry.format.fdbEntryMacAddrFormat.macAddr.arEther[1],
                eventPtr->entry.format.fdbEntryMacAddrFormat.macAddr.arEther[2],
                eventPtr->entry.format.fdbEntryMacAddrFormat.macAddr.arEther[3],
                eventPtr->entry.format.fdbEntryMacAddrFormat.macAddr.arEther[4],
                eventPtr->entry.format.fdbEntryMacAddrFormat.macAddr.arEther[5],
                eventPtr->entry.format.fdbEntryMacAddrFormat.fid,
                eventPtr->entry.format.fdbEntryMacAddrFormat.dstInterface.type,
                eventPtr->entry.format.fdbEntryMacAddrFormat.dstInterface.interfaceInfo.devPort.hwDevNum,
                eventPtr->entry.format.fdbEntryMacAddrFormat.dstInterface.interfaceInfo.devPort.portNum);
        }

        switch (eventPtr->updateType)
        {
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_NO_SPACE_E:
                rc = prvDxChBrgFdbManagerMacNoSpaceCacheUpdate(fdbManagerId,
                                                               &(hwAuMessagesPtr[auIndex]),
                                                               &macNoSpaceTryAdding);
                if (rc != GT_OK)
                {
                    continue;
                }
                if(!macNoSpaceTryAdding)
                {
                    /* Statistics - NO space filted cache */
                    fdbManagerPtr->apiLearningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_NO_SPACE_FILTERED_MSG_E]++;
                    /* increment the number of events */
                    eventArrIndex++;
                    break;
                }
                /* else - follow Add Entry logic */
                /* Statistics - NO space message */
                fdbManagerPtr->apiLearningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_NO_SPACE_MSG_E]++;
                GT_ATTR_FALLTHROUGH;
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_NEW_E:
                if(eventPtr->updateType == CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_NEW_E)
                {
                    fdbManagerPtr->apiLearningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_NA_MSG_E]++;
                }
                if (scanParamsPtr->addNewMacUcEntries)
                {
                    addParams.rehashEnable = scanParamsPtr->addWithRehashEnable;
                    /* Add entry */
                    if(eventPtr->updateType == CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_NEW_E)
                    {
                        addParams.tempEntryExist = GT_TRUE;
                        addParams.tempEntryOffset = eventPtr->updateInfo.newEntryFormat.tempEntryOffset;
                    }
                    else
                    {
                        addParams.tempEntryExist = GT_FALSE;
                    }

                    addUpdateStatus = internal_cpssDxChBrgFdbManagerEntryAdd(fdbManagerId,
                                                       &(eventPtr->entry),
                                                       &addParams,
                                                       GT_TRUE);
                    switch(addUpdateStatus)
                    {
                        case GT_OK:
                            break;
                        case GT_LEARN_LIMIT_PORT_ERROR           :
                            /* modify the entry type ! */
                            eventPtr->updateType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_NEW_REACH_LIMIT_E;
                            eventPtr->updateInfo.movedReachLimitEntryFormat.limitInfo.limitType  = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E;
                            eventPtr->updateInfo.movedReachLimitEntryFormat.limitInfo.limitIndex.devPort.hwDevNum = fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo.devPort.hwDevNum;
                            eventPtr->updateInfo.movedReachLimitEntryFormat.limitInfo.limitIndex.devPort.portNum  = fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo.devPort.portNum;
                            break;
                        case GT_LEARN_LIMIT_TRUNK_ERROR          :
                            /* modify the entry type ! */
                            eventPtr->updateType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_NEW_REACH_LIMIT_E;
                            eventPtr->updateInfo.movedReachLimitEntryFormat.limitInfo.limitType  = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_TRUNK_E;
                            eventPtr->updateInfo.movedReachLimitEntryFormat.limitInfo.limitIndex.trunkId = (GT_TRUNK_ID)fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo.trunkId;
                            break;
                        case GT_LEARN_LIMIT_GLOBAL_EPORT_ERROR   :
                            /* modify the entry type ! */
                            eventPtr->updateType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_NEW_REACH_LIMIT_E;
                            eventPtr->updateInfo.movedReachLimitEntryFormat.limitInfo.limitType  = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_GLOBAL_EPORT_E;
                            eventPtr->updateInfo.movedReachLimitEntryFormat.limitInfo.limitIndex.globalEport = fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo.globalEport;
                            break;
                        case GT_LEARN_LIMIT_FID_ERROR            :
                            /* modify the entry type ! */
                            eventPtr->updateType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_NEW_REACH_LIMIT_E;
                            eventPtr->updateInfo.movedReachLimitEntryFormat.limitInfo.limitType  = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_FID_E;
                            eventPtr->updateInfo.movedReachLimitEntryFormat.limitInfo.limitIndex.fid = fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo.fid;
                            break;
                        case GT_LEARN_LIMIT_GLOBAL_ERROR         :
                            /* modify the entry type ! */
                            eventPtr->updateType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_NEW_REACH_LIMIT_E;
                            eventPtr->updateInfo.movedReachLimitEntryFormat.limitInfo.limitType  = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_GLOBAL_E;
                            break;
                        default: /* other errors */
                            fdbManagerPtr->apiLearningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_ERROR_FAILED_TABLE_UPDATE_E]++;
                            continue;
                    }
                }
                /* increment the number of events */
                eventArrIndex++;
                break;

            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_MOVED_E:
                fdbManagerPtr->apiLearningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_TOTAL_MOVED_MSG_E]++;
                if(scanParamsPtr->updateMovedMacUcEntries)
                {
                    updateParams.updateOnlySrcInterface = GT_TRUE;
                    dbEntryPtr = NULL;
                    addUpdateStatus = internal_cpssDxChBrgFdbManagerEntryUpdate(fdbManagerId,
                                                    &(eventPtr->entry),
                                                    &updateParams,
                                                    &bankIndex,
                                                    &dbEntryPtr,
                                                    GT_TRUE);
                    switch(addUpdateStatus)
                    {
                        case GT_OK:
                            break;
                        case GT_LEARN_LIMIT_PORT_ERROR           :
                            /* modify the entry type ! */
                            eventPtr->updateType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_MOVED_REACH_LIMIT_AND_DELETED_E;
                            eventPtr->updateInfo.movedReachLimitEntryFormat.limitInfo.limitType  = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E;
                            eventPtr->updateInfo.movedReachLimitEntryFormat.limitInfo.limitIndex.devPort.hwDevNum = fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo.devPort.hwDevNum;
                            eventPtr->updateInfo.movedReachLimitEntryFormat.limitInfo.limitIndex.devPort.portNum  = fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo.devPort.portNum;
                            break;
                        case GT_LEARN_LIMIT_TRUNK_ERROR          :
                            /* modify the entry type ! */
                            eventPtr->updateType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_MOVED_REACH_LIMIT_AND_DELETED_E;
                            eventPtr->updateInfo.movedReachLimitEntryFormat.limitInfo.limitType  = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_TRUNK_E;
                            eventPtr->updateInfo.movedReachLimitEntryFormat.limitInfo.limitIndex.trunkId = (GT_TRUNK_ID)fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo.trunkId;
                            break;
                        case GT_LEARN_LIMIT_GLOBAL_EPORT_ERROR   :
                            /* modify the entry type ! */
                            eventPtr->updateType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_MOVED_REACH_LIMIT_AND_DELETED_E;
                            eventPtr->updateInfo.movedReachLimitEntryFormat.limitInfo.limitType  = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_GLOBAL_EPORT_E;
                            eventPtr->updateInfo.movedReachLimitEntryFormat.limitInfo.limitIndex.globalEport = fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo.globalEport;
                            break;
                        case GT_LEARN_LIMIT_FID_ERROR            :
                            /* modify the entry type ! */
                            eventPtr->updateType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_MOVED_REACH_LIMIT_AND_DELETED_E;
                            eventPtr->updateInfo.movedReachLimitEntryFormat.limitInfo.limitType  = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_FID_E;
                            eventPtr->updateInfo.movedReachLimitEntryFormat.limitInfo.limitIndex.fid = fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo.fid;
                            break;
                        case GT_LEARN_LIMIT_GLOBAL_ERROR         :
                            /* modify the entry type ! */
                            eventPtr->updateType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_MOVED_REACH_LIMIT_AND_DELETED_E;
                            eventPtr->updateInfo.movedReachLimitEntryFormat.limitInfo.limitType  = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_GLOBAL_E;
                            break;
                        default: /* other errors */
                            fdbManagerPtr->apiLearningScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_ERROR_FAILED_TABLE_UPDATE_E]++;
                            continue;
                    }

                    if(entriesLearningArray && dbEntryPtr)
                    {
                        /* update in the message to the application the fields that are not in the AU message */
                        eventPtr->entry.format.fdbEntryMacAddrFormat.daRoute        =  dbEntryPtr->specificFormat.prvMacEntryFormat.daRoute;
                        eventPtr->entry.format.fdbEntryMacAddrFormat.daSecurityLevel  =  dbEntryPtr->specificFormat.prvMacEntryFormat.daAccessLevel;
                        eventPtr->entry.format.fdbEntryMacAddrFormat.saSecurityLevel  =  dbEntryPtr->specificFormat.prvMacEntryFormat.saAccessLevel;
                        eventPtr->entry.format.fdbEntryMacAddrFormat.daCommand      =  dbEntryPtr->specificFormat.prvMacEntryFormat.daCommand == 3 ?
                                                                    fdbManagerPtr->entryAttrInfo.daDropCommand :
                                                                    dbEntryPtr->specificFormat.prvMacEntryFormat.daCommand;
                        eventPtr->entry.format.fdbEntryMacAddrFormat.saCommand      =  dbEntryPtr->specificFormat.prvMacEntryFormat.saCommand ?
                                                                    fdbManagerPtr->entryAttrInfo.saDropCommand :
                                                                    dbEntryPtr->specificFormat.prvMacEntryFormat.saCommand;
                    }
#ifdef PRV_CPSS_DXCH_BRG_FDB_MANAGER_ALLOW_HW_BANK_COUNTER_UPDATE_CNS
                    /* Increment the FDB counter (In case of dynamic entry moved, HW decrease the counter of that particular tile) */
                    rc = prvCpssDxChBrgFdbManagerPortGroupBankCounterUpdate(devNum, portGroupIdPtr[auIndex], bankIndex, GT_TRUE);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
#endif
                }
                else /* GT_FALSE == scanParamsPtr->updateMovedMacUcEntries */
                {
                    /*************************************
                      Lookup for existing entry in the DB
                     *************************************/
                    rc = prvCpssDxChFdbManagerDbCalcHashAndLookupResult(fdbManagerPtr,
                        &eventPtr->entry , NULL,&calcInfo);
                    if(rc != GT_OK)
                    {
                        continue;
                    }

                    if(calcInfo.calcEntryType != PRV_CPSS_DXCH_FDB_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_EXISTS_E)
                    {
                        continue;
                    }
#ifdef PRV_CPSS_DXCH_BRG_FDB_MANAGER_ALLOW_HW_BANK_COUNTER_UPDATE_CNS
                    bankIndex = calcInfo.selectedBankId;

                    /* Increment the FDB counter (In case of dynamic entry moved, HW decrease the counter of that particular tile) */
                    rc = prvCpssDxChBrgFdbManagerPortGroupBankCounterUpdate(devNum, portGroupIdPtr[auIndex], bankIndex, GT_TRUE);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
#endif
                }

                /* increment the number of events */
                eventArrIndex++;
                break;

            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_AGED_OUT_E:
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_AGED_DELETED_E:
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_DELETED_E:
            case CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_TRANSPLANTED_E:

            default:
                continue;
        }
    }

    if(addUpdateStatus == GT_BAD_PARAM)
    {
        /* Update the Learning array Index */
        if(entriesLearningNumPtr != NULL)
        {
            *entriesLearningNumPtr = eventArrIndex+1;
        }
        /* Keep the AU index, which are not processed by FDB manager but already read from AUQ
         * Next learning scan will start from there */
        fdbManagerPtr->tempInfo.lastHwAuMessageIndex  = auIndex;
        fdbManagerPtr->tempInfo.lastScanTotalNumOfAUs = numOfAu;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    else
    {
        /* Update the Learning array Index */
        if(entriesLearningNumPtr != NULL)
        {
            *entriesLearningNumPtr = eventArrIndex;
        }

        /* Reset the hw AU message counter, whenAuMsgHandle return success */
        fdbManagerPtr->tempInfo.lastHwAuMessageIndex  = 0;
        fdbManagerPtr->tempInfo.lastScanTotalNumOfAUs = 0;
    }
    return GT_OK;
}

/**
* @internal mainLogicLearningScan function
* @endinternal
*
* @brief   The function gets the AUQ messages according to the input attributes.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] paramsPtr             - the learning scan attributes.
* @param[out] entriesLearningArray - (pointer to) Update event array.
* @param[out] entriesLearningNumPtr- (pointer to) Number of Update events filled in array.
* @param[out] entriesLearningNumPtr- (pointer to) Number of Update events filled in array.
* @param[out] learningScanStatisticsPtr- (pointer to) the type of API ok/error statistics
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - if no more entries to retrieve
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS mainLogicLearningScan
(
    IN  GT_U32                                                      fdbManagerId,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_SCAN_PARAMS_STC          *paramsPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC            entriesLearningArray[],
    OUT GT_U32                                                      *entriesLearningNumPtr,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_ENT  *learningScanStatisticsPtr
)
{
    GT_STATUS                                                   rc = GT_OK;
    GT_STATUS                                                   nextValidDeviceRc;
    GT_STATUS                                                   rc1;
    GT_U32                                                      numOfAu;           /* number of AU messages to retrieve */
    GT_U32                                                      currEntries;       /* current number of AU entries handled by scan */
    GT_U32                                                      restEntriesToScan; /* rest of entries to reach scan operation done */
    GT_U8                                                       devNum;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC              *fdbManagerPtr;

    /* Indicates if need to loop again devices starting from devNum_withValidEntries, if MAX entries not reached */
    GT_U8                                                       devNum_withValidEntries = 0;
    GT_BOOL                                                     isValidEntryExist = GT_FALSE;

    *learningScanStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_ERROR_INPUT_INVALID_E;
    /* Check for valid arguments */
    CPSS_NULL_PTR_CHECK_MAC(paramsPtr);
    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    if((IS_FDB_MANAGER_FOR_SIP_4(fdbManagerPtr)) &&
            (paramsPtr->updateMovedMacUcEntries || paramsPtr->addWithRehashEnable))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    *learningScanStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS___LAST___E;

    if(entriesLearningNumPtr != NULL)
    {
        *entriesLearningNumPtr = 0;
    }

    currEntries = 0;
    nextValidDeviceRc = prvCpssDxChFdbManagerDbDeviceIteratorGetNext(fdbManagerPtr, GT_TRUE, &devNum);
    while(nextValidDeviceRc == GT_OK)
    {
        restEntriesToScan = fdbManagerPtr->capacityInfo.maxEntriesPerLearningScan - currEntries;

        /*************************************/
        /* get the AU message buffer from HW */
        /*************************************/
        numOfAu = (restEntriesToScan > AU_MAX_NUM_CNS) ? AU_MAX_NUM_CNS : restEntriesToScan;

        /* Finish the already read HW queue before reading the new messages */
        if(fdbManagerPtr->tempInfo.lastHwAuMessageIndex != 0)
        {
            numOfAu = fdbManagerPtr->tempInfo.lastScanTotalNumOfAUs;
        }
        else
        {
            LOCK_DEV_NUM(devNum);
            rc = prvDxChBrgFdbAuFuMsgBlockGet(devNum, MESSAGE_QUEUE_PRIMARY_AUQ_E,
                    &numOfAu, NULL, fdbManagerPtr->tempInfo.hwAuMessages, fdbManagerPtr->tempInfo.portGroupIds);
            UNLOCK_DEV_NUM(devNum);
        }

        if((rc != GT_NO_MORE) && (rc != GT_OK))
        {
            return rc;
        }

        if((rc == GT_OK) && (isValidEntryExist == GT_FALSE))
        {
            devNum_withValidEntries = devNum;
            isValidEntryExist = GT_TRUE;
        }

        /*********************************/
        /* handle all of the AU messages */
        /*********************************/
        if(numOfAu > 0)
        {
            rc1 = prvDxChBrgFdbManagerAuMsgHandle(devNum,
                    fdbManagerId,
                    fdbManagerPtr->tempInfo.hwAuMessages,
                    fdbManagerPtr->tempInfo.portGroupIds,
                    numOfAu,
                    paramsPtr,
                    entriesLearningArray,
                    entriesLearningNumPtr);
            if(rc1 != GT_OK)
            {
                return rc1;
            }
            currEntries +=numOfAu;
        }
        if(currEntries == fdbManagerPtr->capacityInfo.maxEntriesPerLearningScan)
        {
            *learningScanStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_E;
            break;
        }
        else if(currEntries > fdbManagerPtr->capacityInfo.maxEntriesPerLearningScan)
        {
            /* This will never happen */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        nextValidDeviceRc = prvCpssDxChFdbManagerDbDeviceIteratorGetNext(fdbManagerPtr, GT_FALSE, &devNum);

        if(nextValidDeviceRc == GT_NO_MORE)
        {
            /* Learning scan reads 4 messages from each AUQ, till the end device.
             * if maximum entries allowed per scan is not reached, start from the valid device again
             * devNum_withValidEntries = holds the first device with valid entries */
            if(isValidEntryExist == GT_TRUE)
            {
                devNum              = devNum_withValidEntries;  /* Start from the last valid device */
                nextValidDeviceRc   = GT_OK;                    /* Indicate this devNum is valid */
                isValidEntryExist   = GT_FALSE;                 /* New Loop - No valid device found for next device loop */
            }
            else /* No more valid entries but max allowed entries not reached yet */
            {
                *learningScanStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_OK_NO_MORE_E;
                rc = GT_NO_MORE;
                break;
            }
        }
    }
    return rc;
}

/**
* @internal internal_cpssDxChBrgFdbManagerLearningScan function
* @endinternal
*
* @brief   The function gets the AUQ messages according to the input attributes.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] paramsPtr             - the learning scan attributes.
* @param[out] entriesLearningArray - (pointer to) Update event array.
* @param[out] entriesLearningNumPtr- (pointer to) Number of Update events filled in array.
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - if no more entries to retrieve
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgFdbManagerLearningScan
(
    IN  GT_U32                                             fdbManagerId,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_SCAN_PARAMS_STC *paramsPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC   entriesLearningArray[],
    OUT GT_U32                                             *entriesLearningNumPtr
)
{
    GT_STATUS                                                   rc;
    PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS_ENT  learningScanStatistics;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC              *fdbManagerPtr;

    FDB_MANAGER_ID_CHECK(fdbManagerId);

    /* main logic of learning scan */
    rc = mainLogicLearningScan(fdbManagerId, paramsPtr, entriesLearningArray, entriesLearningNumPtr, &learningScanStatistics);

    if(learningScanStatistics < PRV_CPSS_DXCH_FDB_MANAGER_SCAN_LEARNING_API_STATISTICS___LAST___E)
    {
        fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

        /* increment the 'API statistics' */
        fdbManagerPtr->apiLearningScanStatisticsArr[learningScanStatistics]++;
    }

    return rc;
}

/**
* @internal cpssDxChBrgFdbManagerLearningScan function
* @endinternal
*
* @brief   The function gets the AUQ messages according to the input attributes.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] paramsPtr             - the learning scan attributes.
* @param[out] entriesLearningArray - (pointer to) Update event array.
* @param[out] entriesLearningNumPtr- (pointer to) Number of Update events filled in array.
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - if no more entries to retrieve
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbManagerLearningScan
(
    IN  GT_U32                                             fdbManagerId,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_SCAN_PARAMS_STC *paramsPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC   entriesLearningArray[],
    OUT GT_U32                                             *entriesLearningNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbManagerLearningScan);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, fdbManagerId, paramsPtr, entriesLearningArray, entriesLearningNumPtr));

    rc = internal_cpssDxChBrgFdbManagerLearningScan(fdbManagerId, paramsPtr, entriesLearningArray, entriesLearningNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, fdbManagerId, paramsPtr, entriesLearningArray, entriesLearningNumPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgFdbManagerLearningThrottlingGet function
* @endinternal
*
* @brief   The function gets the learning throttling attributes
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] dataPtr              - (pointer to) Learning throttling related attributes.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong fdbManagerId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgFdbManagerLearningThrottlingGet
(
    IN  GT_U32                                              fdbManagerId,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_THROTTLING_STC   *dataPtr
)
{
    GT_STATUS                                           rc;         /* return status */
    GT_U8                                               devNum;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC      *fdbManagerPtr;
    PRV_CPSS_AU_DESC_EXT_8_STC                          hwAuMessage;
    GT_U32                                              index;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC    updateEvent;

    /* Check for valid arguments */
    CPSS_NULL_PTR_CHECK_MAC(dataPtr);
    FDB_MANAGER_ID_CHECK(fdbManagerId);

    /* Fill throttling Data from FDB DB instance */
    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);
    dataPtr->macNoSpaceUpdatesEnable = fdbManagerPtr->learningInfo.macNoSpaceUpdatesEnable;
    dataPtr->thresholdB              = fdbManagerPtr->thresholdB;
    dataPtr->thresholdC              = fdbManagerPtr->thresholdC;
    dataPtr->currentUsedEntries      = fdbManagerPtr->totalPopulation;

    rc = prvCpssDxChFdbManagerDbDeviceIteratorGetNext(fdbManagerPtr, GT_TRUE, &devNum);
    if(rc != GT_OK)
    {
        /* no devices to work with */
        return GT_OK;
    }

    for(index = 0; index<fdbManagerPtr->noSpaceCacheEntry.usedEntriesCnt; index++)
    {
        cpssOsMemSet(&updateEvent, 0, sizeof(updateEvent));

        /* Get the AU message in HW format from FDB Manager instance */
        cpssOsMemCpy(&hwAuMessage.elem0, &(fdbManagerPtr->noSpaceCacheEntry.noSpaceKeyCache[index]), sizeof(hwAuMessage.elem0));
        cpssOsMemCpy(&hwAuMessage.elem1, &(fdbManagerPtr->noSpaceCacheEntry.noSpaceKeyCache[index][4]), sizeof(hwAuMessage.elem1));

        /* Parse the HW format to DB format */
        rc = au2EventMsg(devNum, &hwAuMessage, &updateEvent);
        if (rc != GT_OK)
        {
            return rc;
        }
        cpssOsMemCpy(&(dataPtr->noSpaceCache.noSpaceEntryCache[index]),
                     &updateEvent.entry,
                     sizeof(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC));
    }

    if(fdbManagerPtr->noSpaceCacheEntry.usedEntriesCnt < 4)
    {
        /* support the case that API called before there are such 4 entries */
        for(/* index continue*/; index < 4 /* the side of dataPtr->noSpaceCache.noSpaceEntryCache*/; index++)
        {
            dataPtr->noSpaceCache.noSpaceEntryCache[index].fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE__LAST__E;
        }
    }


    /* Fill throttling Data from HW */
    rc = cpssDxChBrgFdbNaMsgOnChainTooLongGet(devNum, &(dataPtr->hashTooLongEnabled));
    if(rc != GT_OK)
    {
        return rc;
    }

    return cpssDxChBrgFdbAuMsgRateLimitGet(devNum, &(dataPtr->auMsgRate), &dataPtr->auMsgRateLimitEn);
}

/**
* @internal cpssDxChBrgFdbManagerLearningThrottlingGet function
* @endinternal
*
* @brief   The function gets the learning throttling attributes
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] dataPtr              - (pointer to) Learning throttling related attributes.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong fdbManagerId.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbManagerLearningThrottlingGet
(
    IN  GT_U32                                              fdbManagerId,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_THROTTLING_STC   *dataPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbManagerLearningThrottlingGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, fdbManagerId, dataPtr));

    rc = internal_cpssDxChBrgFdbManagerLearningThrottlingGet(fdbManagerId, dataPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, fdbManagerId, dataPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgFdbManagerCountersGet function
* @endinternal
*
* @brief API for fetching FDB Manager counters.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] countersPtr          - (pointer to) FDB Manager Counters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
*/
static GT_STATUS internal_cpssDxChBrgFdbManagerCountersGet
(
    IN GT_U32                                   fdbManagerId,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC *countersPtr
)
{
    FDB_MANAGER_ID_CHECK(fdbManagerId);
    CPSS_NULL_PTR_CHECK_MAC(countersPtr);

    prvCpssDxChBrgFdbManagerDbCountersGet(fdbManagerId, countersPtr);

    return GT_OK;
}

/**
* @internal cpssDxChBrgFdbManagerCountersGet function
* @endinternal
*
* @brief API for fetching FDB Manager counters.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] countersPtr          - (pointer to) FDB Manager Counters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS cpssDxChBrgFdbManagerCountersGet
(
    IN GT_U32                                   fdbManagerId,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC *countersPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbManagerCountersGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, fdbManagerId, countersPtr));

    rc = internal_cpssDxChBrgFdbManagerCountersGet(fdbManagerId, countersPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, fdbManagerId, countersPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgFdbManagerStatisticsGet function
* @endinternal
*
* @brief API for fetching FDB Manager statistics.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] statisticsPtr        - (pointer to) FDB Manager Statistics.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS internal_cpssDxChBrgFdbManagerStatisticsGet
(
    IN GT_U32                                      fdbManagerId,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC  *statisticsPtr
)
{
    FDB_MANAGER_ID_CHECK(fdbManagerId);
    CPSS_NULL_PTR_CHECK_MAC(statisticsPtr);

    prvCpssDxChBrgFdbManagerDbStatisticsGet(fdbManagerId, statisticsPtr);

    return GT_OK;
}
/**
* @internal cpssDxChBrgFdbManagerStatisticsGet function
* @endinternal
*
* @brief API for fetching FDB Manager statistics.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] statisticsPtr        - (pointer to) FDB Manager Statistics.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
*/
GT_STATUS cpssDxChBrgFdbManagerStatisticsGet
(
    IN GT_U32                                      fdbManagerId,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC  *statisticsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbManagerStatisticsGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, fdbManagerId, statisticsPtr));

    rc = internal_cpssDxChBrgFdbManagerStatisticsGet(fdbManagerId, statisticsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, fdbManagerId, statisticsPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgFdbManagerStatisticsClear function
* @endinternal
*
* @brief API for clearing FDB Manager statistics.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS internal_cpssDxChBrgFdbManagerStatisticsClear
(
    IN GT_U32 fdbManagerId
)
{
    FDB_MANAGER_ID_CHECK(fdbManagerId);

    prvCpssDxChBrgFdbManagerDbStatisticsClear(fdbManagerId);

    return GT_OK;
}

/**
* @internal cpssDxChBrgFdbManagerStatisticsClear function
* @endinternal
*
* @brief API for clearing FDB Manager statistics.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS cpssDxChBrgFdbManagerStatisticsClear
(
    IN GT_U32 fdbManagerId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbManagerStatisticsClear);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, fdbManagerId));

    rc = internal_cpssDxChBrgFdbManagerStatisticsClear(fdbManagerId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, fdbManagerId));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}

/**
* @internal prvCpssDxChFdbManagerDbEntryDelete function
* @endinternal
*
* @brief API deleting entry in all devices.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entryPtr              - (pointer to) FDB entry format with full info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_FOUND             - the entry was not found.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChFdbManagerDbEntryDelete
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC   *dbEntryPtr
)
{
    GT_STATUS                                                   rc;
    PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_DELETE_API_STATISTICS_ENT   deleteApiStatistics;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC            calcInfo;
    GT_U32                                                      bankId, hwIndexExt;

    /* Fill the entry related data into calcInfo */
    cpssOsMemSet(&calcInfo, 0, sizeof(calcInfo));
    bankId                                      = dbEntryPtr->hwIndex % fdbManagerPtr->numOfBanks;
    calcInfo.dbEntryPtr                         = dbEntryPtr;
    calcInfo.selectedBankId                     = bankId;
    calcInfo.crcMultiHashArr[bankId]            = dbEntryPtr->hwIndex;
    calcInfo.calcEntryType                      = PRV_CPSS_DXCH_FDB_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_EXISTS_E;

    if(dbEntryPtr->hwFdbEntryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E)
    {
        /* IPv6 Data entry */
        calcInfo.bankStep                       = 2;
        hwIndexExt                              = dbEntryPtr->hwIndex + 1;
        calcInfo.crcMultiHashArr[bankId + 1]    = hwIndexExt;
        calcInfo.dbEntryExtPtr                  = &fdbManagerPtr->entryPoolPtr[fdbManagerPtr->indexArr[hwIndexExt].entryPointer];
    }
    else
    {
        calcInfo.bankStep                       = 1;
    }

    /* Delete the entry in all the devices */
    rc = prvCpssDxChFdbManagerEntryDelete(fdbManagerPtr, &calcInfo, &deleteApiStatistics);
    if(deleteApiStatistics < PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_DELETE_API_STATISTICS___LAST___E)
    {
        /* increment the 'API statistics' */
        fdbManagerPtr->apiEntryDeleteStatisticsArr[deleteApiStatistics]++;
    }
    return rc;
}

/**
* @internal validateAgingParamsEntryTypeAndFilters_ip_multicast function
* @endinternal
*
* @brief  function to validate the Aging scan param for IP multicast entry type.
*
* @param[in] paramsPtr             - (pointer to) the aging scan attributes.
* @param[in] dbEntryIpMcPtr        - (pointer to) the IP multicast entry format (DB entry)
* @param[out] isValidPtr           - (pointer to) the valid status.
*                                    GT_TRUE - Included in the aging scan.
*                                    GT_TRUE - Exclude from the aging scan.
* @param[out] deleteEnPtr          - (pointer to) the delete status.
*                                    GT_TRUE - Delete required if aged-out.
*                                    GT_TRUE - Do not delete even if aged-out.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS validateAgingParamsEntryTypeAndFilters_ip_multicast
(
    IN CPSS_DXCH_BRG_FDB_MANAGER_AGING_SCAN_PARAMS_STC           *paramsPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IP_MC_ADDR_FORMAT_STC  *dbEntryIpMcPtr,
    OUT GT_BOOL                                                  *isValidPtr,
    OUT GT_BOOL                                                  *deleteEnPtr
)
{
    /* invalidate the entries where not applicable filters are enabled */
    if((paramsPtr->checkAgeIpMcEntries != GT_TRUE) ||
        paramsPtr->hwDevNumMask_ePort || paramsPtr->hwDevNumMask_trunk || paramsPtr->ePortTrunkNumMask || paramsPtr->isTrunkMask)
    {
        return GT_OK;
    }

    switch(dbEntryIpMcPtr->dstInterface_type)
    {
        case CPSS_INTERFACE_VIDX_E:
        case CPSS_INTERFACE_VID_E:
            break;
        default:
            /* IP multicast entry with non-Multicast interface is not valid
             * CPSS add should not allow such entries */
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(dbEntryIpMcPtr->dstInterface_type);
    }

    if(dbEntryIpMcPtr->isStatic)
    {
        /* Static entries should never be part of aging scan */
        return GT_OK;
    }

    FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->fidMask, dbEntryIpMcPtr->fid, paramsPtr->fid);
    FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->userDefinedMask, dbEntryIpMcPtr->userDefined, paramsPtr->userDefined);
    *deleteEnPtr = (paramsPtr->deleteAgeoutIpMcEntries == GT_TRUE)?GT_TRUE:GT_FALSE;
    *isValidPtr  = GT_TRUE;
    return GT_OK;
}

/**
* @internal validateAgingParamsEntryTypeAndFilters_ip_unicast function
* @endinternal
*
* @brief  function to validate the Aging scan param for IP unicast entry type.
*
* @param[in] paramsPtr             - (pointer to) the aging scan attributes.
* @param[in] dbEntryIpUcPtr        - (pointer to) the IP unicast format (DB entry)
* @param[out] isValidPtr           - (pointer to) the valid status.
*                                    GT_TRUE - Included in the aging scan.
*                                    GT_TRUE - Exclude from the aging scan.
* @param[out] deleteEnPtr          - (pointer to) the delete status.
*                                    GT_TRUE - Delete required if aged-out.
*                                    GT_TRUE - Do not delete even if aged-out.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS validateAgingParamsEntryTypeAndFilters_ip_unicast
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_AGING_SCAN_PARAMS_STC              *paramsPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV_UC_COMMON_FORMAT_STC  *dbEntryIpUcPtr,
    OUT GT_BOOL                                                     *isValidPtr,
    OUT GT_BOOL                                                     *deleteEnPtr
)
{
    /* invalidate the entries where not applicable filters are enabled */
    if((paramsPtr->checkAgeIpUcEntries != GT_TRUE) ||
        paramsPtr->fidMask || paramsPtr->vid1Mask || paramsPtr->userDefinedMask)
    {
        return GT_OK;
    }

    switch(dbEntryIpUcPtr->dstInterface_type)
    {
        case CPSS_INTERFACE_VIDX_E:
        case CPSS_INTERFACE_VID_E:
            /* unicast entries treated as multicast. */
            break;
        case CPSS_INTERFACE_PORT_E:
            FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->isTrunkMask, GT_FALSE, paramsPtr->isTrunk);  /* isTrunk == GT_FALSE */
            FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->ePortTrunkNumMask, dbEntryIpUcPtr->dstInterface.devPort.portNum, paramsPtr->ePortTrunkNum);
            if(GT_FALSE == prvCpssDxChFdbManagerDbIsGlobalEport(fdbManagerPtr,
                dbEntryIpUcPtr->dstInterface.devPort.portNum))
            {
                /* check hwDevNum only if the entry is not associated with the global eport */
                FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->hwDevNumMask_ePort, dbEntryIpUcPtr->dstInterface.devPort.hwDevNum, paramsPtr->hwDevNum);
            }
            break;
        case CPSS_INTERFACE_TRUNK_E:
            FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->isTrunkMask, GT_TRUE, paramsPtr->isTrunk);  /* isTrunk == GT_TRUE */
            FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->hwDevNumMask_trunk, dbEntryIpUcPtr->dstInterface.devPort.hwDevNum, paramsPtr->hwDevNum);
            FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->ePortTrunkNumMask, dbEntryIpUcPtr->dstInterface.trunkId, paramsPtr->ePortTrunkNum);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(dbEntryIpUcPtr->dstInterface_type);
    }
    *deleteEnPtr = (paramsPtr->deleteAgeoutIpUcEntries == GT_TRUE)?GT_TRUE:GT_FALSE;
    *isValidPtr  = GT_TRUE;
    return GT_OK;
}

/**
* @internal validateAgingParamsEntryTypeAndFilters_mac function
* @endinternal
*
* @brief  function to validate the Aging scan param for MAC entry type.
*
* @param[in] macEntryMuxingMode    - The MAC entry muxing mode.
* @param[in] paramsPtr             - (pointer to) the aging scan attributes.
* @param[in] dbEntryIpUcPtr        - (pointer to) the MAC format (DB entry)
* @param[out] isValidPtr           - (pointer to) the valid status.
*                                    GT_TRUE - Included in the aging scan.
*                                    GT_TRUE - Exclude from the aging scan.
* @param[out] deleteEnPtr          - (pointer to) the delete status.
*                                    GT_TRUE - Delete required if aged-out.
*                                    GT_TRUE - Do not delete even if aged-out.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS validateAgingParamsEntryTypeAndFilters_mac
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_ENT      macEntryMuxingMode,
    IN CPSS_DXCH_BRG_FDB_MANAGER_AGING_SCAN_PARAMS_STC         *paramsPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_MAC_ADDR_FORMAT_STC  *dbEntryMacPtr,
    OUT GT_BOOL                                                *isValidPtr,
    OUT GT_BOOL                                                *deleteEnPtr
)
{
    if(dbEntryMacPtr->isStatic)
    {
        /* Static entries should never be part of aging scan */
        return GT_OK;
    }

    switch(dbEntryMacPtr->dstInterface_type)
    {
        /* MAC Multicast - Not applicable filters - Device, ePortTrunk, isTrunk */
        case CPSS_INTERFACE_VIDX_E:
        case CPSS_INTERFACE_VID_E:
            FDB_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->checkAgeMacMcEntries, GT_TRUE);
            /* invalidate the entries where not applicable filters are enabled */
            if(paramsPtr->hwDevNumMask_ePort || paramsPtr->hwDevNumMask_trunk || paramsPtr->ePortTrunkNumMask || paramsPtr->isTrunkMask)
            {
                return GT_OK;
            }
            FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->fidMask, dbEntryMacPtr->fid, paramsPtr->fid);
            FDB_SCAN_VALIDATE_FILTER_VID1_MAC_AND_RETURN_MAC(macEntryMuxingMode, paramsPtr->vid1Mask, dbEntryMacPtr->vid1, paramsPtr->vid1);
            FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->userDefinedMask, dbEntryMacPtr->userDefined, paramsPtr->userDefined);
            *deleteEnPtr = (paramsPtr->deleteAgeoutMacMcEntries == GT_TRUE)?GT_TRUE:GT_FALSE;
            break;
        case CPSS_INTERFACE_PORT_E:
            FDB_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->checkAgeMacUcEntries, GT_TRUE);
            FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->isTrunkMask, GT_FALSE, paramsPtr->isTrunk);  /* isTrunk == GT_FALSE */
            FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->fidMask, dbEntryMacPtr->fid, paramsPtr->fid);
            FDB_SCAN_VALIDATE_FILTER_VID1_MAC_AND_RETURN_MAC(macEntryMuxingMode, paramsPtr->vid1Mask, dbEntryMacPtr->vid1, paramsPtr->vid1);
            FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->ePortTrunkNumMask, dbEntryMacPtr->dstInterface.devPort.portNum, paramsPtr->ePortTrunkNum);
            FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->userDefinedMask, dbEntryMacPtr->userDefined, paramsPtr->userDefined);
            if(GT_FALSE == prvCpssDxChFdbManagerDbIsGlobalEport(fdbManagerPtr,
                dbEntryMacPtr->dstInterface.devPort.portNum))
            {
                /* check hwDevNum only if the entry is not associated with the global eport */
                FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->hwDevNumMask_ePort, dbEntryMacPtr->dstInterface.devPort.hwDevNum, paramsPtr->hwDevNum);
            }
            *deleteEnPtr = (paramsPtr->deleteAgeoutMacUcEportEntries == GT_TRUE)?GT_TRUE:GT_FALSE;
            break;
        case CPSS_INTERFACE_TRUNK_E:
            FDB_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->checkAgeMacUcEntries, GT_TRUE);
            FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->isTrunkMask, GT_TRUE, paramsPtr->isTrunk);   /* isTrunk == GT_TRUE */
            FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->fidMask, dbEntryMacPtr->fid, paramsPtr->fid);
            FDB_SCAN_VALIDATE_FILTER_VID1_MAC_AND_RETURN_MAC(macEntryMuxingMode, paramsPtr->vid1Mask, dbEntryMacPtr->vid1, paramsPtr->vid1);
            FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->ePortTrunkNumMask, dbEntryMacPtr->dstInterface.trunkId, paramsPtr->ePortTrunkNum);
            FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->userDefinedMask, dbEntryMacPtr->userDefined, paramsPtr->userDefined);
            FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->hwDevNumMask_trunk, dbEntryMacPtr->dstInterface.devPort.hwDevNum, paramsPtr->hwDevNum);
            *deleteEnPtr = (paramsPtr->deleteAgeoutMacUcTrunkEntries == GT_TRUE)?GT_TRUE:GT_FALSE;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(dbEntryMacPtr->dstInterface_type);
    }

    *isValidPtr  = GT_TRUE;

    return GT_OK;
}

/**
* @internal validateAgingParamsEntryTypeAndFilters function
* @endinternal
*
* @brief  function to validate the Aging scan param.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[in] paramsPtr             - (pointer to) the aging scan attributes.
* @param[in] dbEntryPtr            - (pointer to) the DB entry format
* @param[out] isValidPtr           - (pointer to) the valid status.
*                                    GT_TRUE - Included in the aging scan.
*                                    GT_TRUE - Exclude from the aging scan.
* @param[out] deleteEnPtr          - (pointer to) the delete status.
*                                    GT_TRUE - Delete required if aged-out.
*                                    GT_TRUE - Do not delete even if aged-out.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS validateAgingParamsEntryTypeAndFilters
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_AGING_SCAN_PARAMS_STC         *paramsPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC           *dbEntryPtr,
    OUT GT_BOOL                                                *isValidPtr,
    OUT GT_BOOL                                                *deleteEnPtr
)
{
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC       *dbEntryExtPtr;

    /* Entry Types & Not applicable filters Check
     * check if entry is included in processing                       - isValid
     * only if isValid is true, check if delete enabled for this type - deleteEn
     *     Note - Disable deleteEn for static entries.
     */
    *isValidPtr  = GT_FALSE;
    *deleteEnPtr = GT_FALSE;
    switch(dbEntryPtr->hwFdbEntryType)
    {
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E:
            /* MAC Unicast - All Filter are Applicable */
            return validateAgingParamsEntryTypeAndFilters_mac(fdbManagerPtr,fdbManagerPtr->entryAttrInfo.macEntryMuxingMode,
                                                              paramsPtr,
                                                             &dbEntryPtr->specificFormat.prvMacEntryFormat,
                                                              isValidPtr,
                                                              deleteEnPtr);

        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_UC_E:
            /* IP Unicast - Not applicable filters - fid, vid1, userDefined */
            return validateAgingParamsEntryTypeAndFilters_ip_unicast(fdbManagerPtr,paramsPtr,
                                                             &dbEntryPtr->specificFormat.prvIpv4UcEntryFormat.ipUcCommonInfo,
                                                              isValidPtr,
                                                              deleteEnPtr);
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E:
            /* Get the Data Part for the corresponding Key
             * Key part does not have any filter */
            dbEntryExtPtr   = &fdbManagerPtr->entryPoolPtr[fdbManagerPtr->indexArr[dbEntryPtr->hwIndex+1].entryPointer];
            return validateAgingParamsEntryTypeAndFilters_ip_unicast(fdbManagerPtr,paramsPtr,
                                                             &dbEntryExtPtr->specificFormat.prvIpv6UcDataEntryFormat.ipUcCommonInfo,
                                                              isValidPtr,
                                                              deleteEnPtr);
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E:
            return GT_OK;   /* Invalid Entry - Data part is processed while dealing with key */

        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_MCAST_E:
            /* IP Multicast - Not applicable filters - Device, ePortTrunk, isTrunk */
            return validateAgingParamsEntryTypeAndFilters_ip_multicast(paramsPtr,
                                                             &dbEntryPtr->specificFormat.prvIpv4McEntryFormat,
                                                              isValidPtr,
                                                              deleteEnPtr);
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_MCAST_E:
            return validateAgingParamsEntryTypeAndFilters_ip_multicast(paramsPtr,
                                                             &dbEntryPtr->specificFormat.prvIpv6McEntryFormat,
                                                              isValidPtr,
                                                              deleteEnPtr);
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(dbEntryPtr->hwFdbEntryType);
    }
}

/**
* @internal prvCpssDxChFdbManagerAgeBitUpdate function
* @endinternal
*
* @brief   The function Updates the age-bit in the DB entry.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[in] ageBit                - the age bit value to be updated.
* @param[inout] dbEntryPtr         - (pointer to) the DB entry format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong DB type
*/
static GT_STATUS prvCpssDxChFdbManagerAgeBitUpdate
(
    IN    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN    GT_U8                                             ageBit,
    INOUT PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC    *dbEntryPtr
)
{
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC  *dbEntryExtPtr;

    switch(dbEntryPtr->hwFdbEntryType)
    {
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E:
            dbEntryPtr->specificFormat.prvMacEntryFormat.age = ageBit;
            break;
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_MCAST_E:
            dbEntryPtr->specificFormat.prvIpv4McEntryFormat.age = ageBit;
            break;
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_MCAST_E:
            dbEntryPtr->specificFormat.prvIpv6McEntryFormat.age = ageBit;
            break;
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_UC_E:
            dbEntryPtr->specificFormat.prvIpv4UcEntryFormat.ipUcCommonInfo.age = ageBit;
            break;
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E:
            dbEntryExtPtr = &fdbManagerPtr->entryPoolPtr[fdbManagerPtr->indexArr[dbEntryPtr->hwIndex + 1].entryPointer];
            dbEntryPtr->specificFormat.prvIpv6UcKeyEntryFormat.age                     = ageBit;
            dbEntryExtPtr->specificFormat.prvIpv6UcDataEntryFormat.ipUcCommonInfo.age  = ageBit;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(dbEntryPtr->hwFdbEntryType);
    }
    return GT_OK;
}

/**
* @internal mainLogicAgingScan function
* @endinternal
*
* @brief   The function scans the entire FDB and process age-out for the filtered entries.
*          (Applicable entry types - MAC Unicast(dynamic), MAC Multicast, IPv4/6 Unicast, IPv4/6 Multicast)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] paramsPtr             - the aging scan attributes.
* @param[out] entriesAgedoutArray  - (pointer to) Update event array.
*                                    This is optional (can be NULL if application do not require it).
*                                    If used, should have memory for at least "max entries allowed per aging scan".
* @param[out] entriesAgedoutNumPtr - (pointer to) Number of Update events filled in array.
*                                    This is optional (can be NULL if application do not require it).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong fdbManagerId and paramsPtr data
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*/
static GT_STATUS mainLogicAgingScan
(
    IN  GT_U32                                                   fdbManagerId,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_AGING_SCAN_PARAMS_STC         *paramsPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC         entriesAgedoutArray[],
    OUT GT_U32                                                  *entriesAgedoutNumPtr,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_ENT   *agingScanStatisticsPtr
)
{
    GT_STATUS                                                    rc = GT_OK;
    GT_STATUS                                                    rc1;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC              *fdbManagerPtr;
    GT_U32                                                       curAgeBinId;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC               *dbEntryPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC               *dbEntryExtPtr;
    GT_U32                                                       eventArrIndex;
    GT_BOOL                                                      outEventUpdateEn;
    GT_BOOL                                                      isValid;
    GT_BOOL                                                      deleteEn;
    GT_BOOL                                                      agedOut;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_NODE_PTR_STC                    lastGetNextInfo;

    *agingScanStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_ERROR_INPUT_INVALID_E;

    /* Check for valid arguments */
    CPSS_NULL_PTR_CHECK_MAC(paramsPtr);

    FDB_MANAGER_ID_CHECK(fdbManagerId);
    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    *agingScanStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_ERROR_FAILED_TABLE_UPDATE_E;

    /* Get entry from the age-bin */
    curAgeBinId = fdbManagerPtr->agingBinInfo.currentAgingScanBinID;
    FDB_MANAGER_AGE_BIN_ID_CHECK(curAgeBinId);

    outEventUpdateEn = (entriesAgedoutArray != NULL)?GT_TRUE:GT_FALSE;

    eventArrIndex = 0;
    rc1 = prvCpssDxChFdbManagerDbAgeBinEntryGetNext(fdbManagerPtr,
            GT_TRUE,
            curAgeBinId,
            &lastGetNextInfo,
            &dbEntryPtr);
    while(rc1 != GT_NO_MORE)
    {
        /* Check if entry is valid for aging process - Filters and Entry Types
         * Set deleteEn, if delete flag is enabled in param filter */
        if(dbEntryPtr->hwFdbEntryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E)
        {
            /* IPv6 data entries are taken care as part of IPv6 key entry */
            rc1 = prvCpssDxChFdbManagerDbAgeBinEntryGetNext(fdbManagerPtr,
                    GT_FALSE,
                    curAgeBinId,
                    &lastGetNextInfo,
                    &dbEntryPtr);
            continue;
        }

        rc = validateAgingParamsEntryTypeAndFilters(fdbManagerPtr, paramsPtr, dbEntryPtr, &isValid, &deleteEn);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(isValid)
        {
            /* Check if entry is aged-out in all device */
            rc = prvCpssDxChFdbManagerHwAgedOutVerify(fdbManagerPtr, dbEntryPtr, &agedOut);
            if(rc != GT_OK)
            {
                return rc;
            }
            if(agedOut)
            {
                /* Only update the entry to OUT event - Before delete */
                if(outEventUpdateEn)
                {
                    /* Get IPv6 Data part for, IPv6 Unicast Key type */
                    if(dbEntryPtr->hwFdbEntryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E)
                    {
                        dbEntryExtPtr = &fdbManagerPtr->entryPoolPtr[fdbManagerPtr->indexArr[dbEntryPtr->hwIndex+1].entryPointer];
                        rc = prvCpssDxChBrgFdbManagerRestoreEntryIpV6UcFormatParamsFromDb(fdbManagerPtr,
                            dbEntryPtr,
                            dbEntryExtPtr,
                            &entriesAgedoutArray[eventArrIndex].entry);
                    }
                    else
                    {
                        rc = prvCpssDxChBrgFdbManagerRestoreEntryFormatParamsFromDb(fdbManagerPtr,
                            dbEntryPtr,
                            &entriesAgedoutArray[eventArrIndex].entry);
                    }
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                    if(deleteEn)
                    {
                        entriesAgedoutArray[eventArrIndex].updateType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_AGED_DELETED_E;
                    }
                    else
                    {
                        entriesAgedoutArray[eventArrIndex].updateType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_AGED_OUT_E;
                    }
                    eventArrIndex+=1;
                }
                if(deleteEn)
                {
                    rc = prvCpssDxChFdbManagerDbEntryDelete(fdbManagerPtr, dbEntryPtr);
                    if(rc != GT_OK)
                    {
                        /* should not happen - DB probably corrupted */
                        return rc;
                    }
                    fdbManagerPtr->apiAgingScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_TOTAL_AGEDOUT_DELETED_E]++;
                }
                /* Update DB that entry - aged-out (If entry if not deleted)*/
                else
                {
                    rc = prvCpssDxChFdbManagerAgeBitUpdate(fdbManagerPtr, 0, dbEntryPtr);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                    fdbManagerPtr->apiAgingScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_TOTAL_AGEDOUT_E]++;
                }
            }
            /* Update DB that entry - Refreshed */
            else
            {
                rc = prvCpssDxChFdbManagerAgeBitUpdate(fdbManagerPtr, 1, dbEntryPtr);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
        rc1 = prvCpssDxChFdbManagerDbAgeBinEntryGetNext(fdbManagerPtr,
                GT_FALSE,
                curAgeBinId,
                &lastGetNextInfo,
                &dbEntryPtr);
    }

    fdbManagerPtr->agingBinInfo.currentAgingScanBinID = (curAgeBinId + 1) % fdbManagerPtr->agingBinInfo.totalAgeBinAllocated;
    if(entriesAgedoutNumPtr != NULL)
    {
        *entriesAgedoutNumPtr = eventArrIndex;
    }
    *agingScanStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_OK_E;
    return rc;
}

/**
* @internal internal_cpssDxChBrgFdbManagerAgingScan function
* @endinternal
*
* @brief   The function scans the entire FDB and process age-out for the filtered entries.
*          (Applicable entry types - MAC Unicast(dynamic), MAC Multicast, IPv4/6 Unicast, IPv4/6 Multicast)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] paramsPtr             - the aging scan attributes.
* @param[out] entriesAgedoutArray  - (pointer to) Update event array.
*                                    This is optional (can be NULL if application do not require it).
*                                    If used, should have memory for at least "max entries allowed per aging scan".
* @param[out] entriesAgedoutNumPtr - (pointer to) Number of Update events filled in array.
*                                    This is optional (can be NULL if application do not require it).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong fdbManagerId and paramsPtr data
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*          +-------------------------+-------+---------+-------------+--------+---------+------+
*          |                         |FID    |Device   |ePort/Trunk  |isTrunk |Vid1     |User  |
*          +-------------------------+------------+------------------+--------+---------+------+
*          |MAC Unicast              |Valid  |Valid    |Valid        |Valid   |Valid    |Valid |
*          |MAC Multicast            |Valid  |NA       |NA           |NA      |Valid    |Valid |
*          |IPv4/6 Multicast         |Valid  |NA       |NA           |NA      |NA       |Valid |
*          |IPv4/6 Unicast           |NA     |Valid    |Valid        |Valid   |NA       |NA    |
*          +-------------------------+-------+---------+-------------+--------+---------+------+
*/
GT_STATUS internal_cpssDxChBrgFdbManagerAgingScan
(
    IN  GT_U32                                           fdbManagerId,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_AGING_SCAN_PARAMS_STC *paramsPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC entriesAgedoutArray[],
    OUT GT_U32                                          *entriesAgedoutNumPtr
)
{
    GT_STATUS                                                   rc, rc1;
    PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS_ENT       agingScanStatistics;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC              *fdbManagerPtr;

    FDB_MANAGER_ID_CHECK(fdbManagerId);
    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    /* main logic of aging scan */
    rc = mainLogicAgingScan(fdbManagerId, paramsPtr, entriesAgedoutArray, entriesAgedoutNumPtr, &agingScanStatistics);

    if(agingScanStatistics < PRV_CPSS_DXCH_FDB_MANAGER_SCAN_AGE_API_STATISTICS___LAST___E)
    {
        /* increment the 'API statistics' */
        fdbManagerPtr->apiAgingScanStatisticsArr[agingScanStatistics]++;
    }
    rc1 = prvCpssDxChFdbManagerThresholdSet(fdbManagerPtr);

    if(rc != GT_OK)
    {
        return rc;
    }
    return rc1;
}

/**
* @internal cpssDxChBrgFdbManagerAgingScan function
* @endinternal
*
* @brief   The function scans the entire FDB and process age-out for the filtered entries.
*          (Applicable entry types - MAC Unicast(dynamic), MAC Multicast, IPv4/6 Unicast, IPv4/6 Multicast)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] paramsPtr             - the aging scan attributes.
* @param[out] entriesAgedoutArray  - (pointer to) Update event array.
*                                    This is optional (can be NULL if application do not require it).
*                                    If used, should have memory for at least "max entries allowed per aging scan".
* @param[out] entriesAgedoutNumPtr - (pointer to) Number of Update events filled in array.
*                                    This is optional (can be NULL if application do not require it).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong fdbManagerId and paramsPtr data
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*          Invalid Filter table for entry type
*          +-------------------------+-------+---------+-------------+--------+---------+------+
*          |                         |FID    |Device   |ePort/Trunk  |isTrunk |Vid1     |User  |
*          +-------------------------+------------+------------------+--------+---------+------+
*          |MAC Unicast              |Valid  |Valid    |Valid        |Valid   |Valid    |Valid |
*          |MAC Multicast            |Valid  |NA       |NA           |NA      |Valid    |Valid |
*          |IPv4/6 Multicast         |Valid  |NA       |NA           |NA      |NA       |Valid |
*          |IPv4/6 Unicast           |NA     |Valid    |Valid        |Valid   |NA       |NA    |
*          +-------------------------+-------+---------+-------------+--------+---------+------+
*/
GT_STATUS cpssDxChBrgFdbManagerAgingScan
(
    IN  GT_U32                                           fdbManagerId,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_AGING_SCAN_PARAMS_STC *paramsPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC entriesAgedoutArray[],
    OUT GT_U32                                          *entriesAgedoutNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbManagerAgingScan);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, fdbManagerId, paramsPtr, entriesAgedoutArray, entriesAgedoutNumPtr));

    rc = internal_cpssDxChBrgFdbManagerAgingScan(fdbManagerId, paramsPtr, entriesAgedoutArray, entriesAgedoutNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, fdbManagerId, paramsPtr, entriesAgedoutArray, entriesAgedoutNumPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}

/**
* @internal prvCpssDxChFdbManagerDbEntryUpdate function
* @endinternal
*
* @brief API Updating entry in all devices.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entryPtr              - (pointer to) FDB entry format with full info.
* @param[in] newInterfacePtr       - (pointer to) The new interface info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_FOUND             - the entry was not found.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the FDB Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChFdbManagerDbEntryUpdate
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC       *fdbManagerPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC        *dbEntryPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_INTERFACE_INFO_STC   *newInterfacePtr
)
{
    GT_STATUS                                                   rc;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC            calcInfo;
    GT_U32                                                      bankId, hwIndexExt;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INTERFACE_INFO_UNT            *oldInterfacePtr;

    /* Fill the entry related data into calcInfo */
    cpssOsMemSet(&calcInfo, 0, sizeof(calcInfo));
    bankId                           = dbEntryPtr->hwIndex % fdbManagerPtr->numOfBanks;
    calcInfo.dbEntryPtr              = dbEntryPtr;
    calcInfo.selectedBankId          = bankId;
    calcInfo.crcMultiHashArr[bankId] = dbEntryPtr->hwIndex;
    calcInfo.calcEntryType           = PRV_CPSS_DXCH_FDB_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_EXISTS_E;

    switch(dbEntryPtr->hwFdbEntryType)
    {
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E:
            oldInterfacePtr                       = &dbEntryPtr->specificFormat.prvMacEntryFormat.dstInterface;
            dbEntryPtr->specificFormat.prvMacEntryFormat.dstInterface_type = newInterfacePtr->type;
            calcInfo.bankStep                     = 1;
            break;

        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_UC_E:
            oldInterfacePtr                       = &dbEntryPtr->specificFormat.prvIpv4UcEntryFormat.ipUcCommonInfo.dstInterface;
            dbEntryPtr->specificFormat.prvIpv4UcEntryFormat.ipUcCommonInfo.dstInterface_type = newInterfacePtr->type;
            calcInfo.bankStep                     = 1;
            break;

        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E:
            /* IPv6 Data entry */
            calcInfo.bankStep                     = 2;
            hwIndexExt                            = dbEntryPtr->hwIndex + 1;
            calcInfo.crcMultiHashArr[bankId + 1]  = hwIndexExt;
            calcInfo.dbEntryExtPtr                = &fdbManagerPtr->entryPoolPtr[fdbManagerPtr->indexArr[hwIndexExt].entryPointer];

            /* Get the dst Info pointer from data part of IPv6 entry */
            oldInterfacePtr                       = &calcInfo.dbEntryExtPtr->specificFormat.prvIpv6UcDataEntryFormat.ipUcCommonInfo.dstInterface;
            calcInfo.dbEntryExtPtr->specificFormat.prvIpv6UcDataEntryFormat.ipUcCommonInfo.dstInterface_type = newInterfacePtr->type;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(dbEntryPtr->hwFdbEntryType);
    }

    /*Update DB entry with the new interface details */
    switch(newInterfacePtr->type)
    {
        case CPSS_INTERFACE_PORT_E:
            oldInterfacePtr->devPort.hwDevNum    = newInterfacePtr->interfaceInfo.devPort.hwDevNum;
            oldInterfacePtr->devPort.portNum     = newInterfacePtr->interfaceInfo.devPort.portNum;
            break;
        case CPSS_INTERFACE_TRUNK_E:
            oldInterfacePtr->trunkId             = newInterfacePtr->interfaceInfo.trunkId;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(newInterfacePtr->type);
    }

    /****************************************************
      Write the entry to HW of all registered devices
     ****************************************************/
    rc = prvCpssDxChFdbManagerHwWriteByHwIndex(fdbManagerPtr, &calcInfo);
    if(rc != GT_OK)
    {
        fdbManagerPtr->apiEntryUpdateStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_HW_UPDATE_E]++;
        return rc;
    }

    if(NULL != fdbManagerPtr->dynamicUcMacLimitPtr && /* the 'limit' DB is enabled */
       dbEntryPtr->hwFdbEntryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E )
    {
        /*
           3. so now we need to:
                a. decrement 'limit counters' on the old interface (if applicable)
                b. increment 'limit counters' on the new interface (if applicable)
        */
        rc = prvCpssDxChFdbManagerDbLimitOper(fdbManagerPtr,
            LIMIT_OPER_INTERFACE_UPDATE_E);
        if(rc != GT_OK)
        {
            fdbManagerPtr->apiEntryUpdateStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_BAD_STATE_E]++;
            return rc;
        }
    }

    fdbManagerPtr->apiEntryUpdateStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_UPDATE_API_STATISTICS_OK_E]++;
    return rc;
}

/**
* @internal validateTransplantParamsEntryTypeAndFilters function
* @endinternal
*
* @brief  function to validate the transplant scan param.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[in] paramsPtr             - (pointer to) the transplant scan attributes.
* @param[in] dbEntryPtr            - (pointer to) the DB entry format
* @param[out] isValidPtr           - (pointer to) the valid status.
*                                    GT_TRUE - Included in the transplant scan.
*                                    GT_TRUE - Exclude from the transplant scan.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS validateTransplantParamsEntryTypeAndFilters
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_TRANSPLANT_SCAN_PARAMS_STC    *paramsPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC           *dbEntryPtr,
    OUT GT_BOOL                                                *isValidPtr
)
{
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INTERFACE_INFO_UNT     *dbInterfacePtr;
    GT_U32                                               dbIndexExtPtr;

    *isValidPtr  = GT_FALSE;
    switch(dbEntryPtr->hwFdbEntryType)
    {
        /* invalidate the entries where not applicable filters are enabled
         *   - Entry type not enabled (For static both 'static' and 'MacUc' both flag should be enabled)
         *   - MAC Multicast is not applicable
         *   - old interface type is not matching
         */
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E:
            if (dbEntryPtr->specificFormat.prvMacEntryFormat.isStatic == GT_TRUE)
            {
                FDB_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->transplantStaticEntries, GT_TRUE);
            }
            FDB_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->transplantMacUcEntries, GT_TRUE);
            if(dbEntryPtr->specificFormat.prvMacEntryFormat.dstInterface_type == CPSS_INTERFACE_VIDX_E ||
               dbEntryPtr->specificFormat.prvMacEntryFormat.dstInterface_type == CPSS_INTERFACE_VID_E )
            {
                return GT_OK;
            }
            FDB_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->oldInterface.type, dbEntryPtr->specificFormat.prvMacEntryFormat.dstInterface_type);
            FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->fidMask, dbEntryPtr->specificFormat.prvMacEntryFormat.fid, paramsPtr->fid);
            dbInterfacePtr = &dbEntryPtr->specificFormat.prvMacEntryFormat.dstInterface;
            break;

        /* invalidate the entries where not applicable filters are enabled
         *   - Entry type not enabled
         *   - fidMask is not applicable
         *   - old interface type is not matching
         */
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_UC_E:
            FDB_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->transplantIpUcEntries, GT_TRUE);
            FDB_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->fidMask, 0);
            FDB_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->oldInterface.type, dbEntryPtr->specificFormat.prvIpv4UcEntryFormat.ipUcCommonInfo.dstInterface_type);
            dbInterfacePtr = &dbEntryPtr->specificFormat.prvIpv4UcEntryFormat.ipUcCommonInfo.dstInterface;
            break;

        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E:
            FDB_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->transplantIpUcEntries, GT_TRUE);
            FDB_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->fidMask, 0);
            FDB_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->oldInterface.type, dbEntryPtr->specificFormat.prvIpv6UcDataEntryFormat.ipUcCommonInfo.dstInterface_type);
            /* Get destination interface info from IPv6 Data entry(For corresponding Key entry */
            dbIndexExtPtr  = fdbManagerPtr->indexArr[dbEntryPtr->hwIndex + 1].entryPointer;
            dbInterfacePtr = &fdbManagerPtr->entryPoolPtr[dbIndexExtPtr].specificFormat.prvIpv6UcDataEntryFormat.ipUcCommonInfo.dstInterface;
            break;

        default:
            return GT_OK;
    }

    /* Verify oldInterface with the DB Entry*/
    switch(paramsPtr->oldInterface.type)
    {
        case CPSS_INTERFACE_PORT_E:
            FDB_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->oldInterface.interfaceInfo.devPort.portNum, dbInterfacePtr->devPort.portNum);
            if(GT_FALSE == prvCpssDxChFdbManagerDbIsGlobalEport(fdbManagerPtr,
                paramsPtr->oldInterface.interfaceInfo.devPort.portNum))
            {
                /* check hwDevNum only if the entry is not associated with the global eport */
                FDB_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->oldInterface.interfaceInfo.devPort.hwDevNum, dbInterfacePtr->devPort.hwDevNum);
            }
            break;
        case CPSS_INTERFACE_TRUNK_E:
            FDB_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->oldInterface.interfaceInfo.trunkId, dbInterfacePtr->trunkId);
            break;
        default:
            /* Other interfaces are not valid for transplant */
            return GT_OK;
    }

    *isValidPtr = GT_TRUE;
    return GT_OK;
}

/**
* @internal mainLogicTransplantScan function
* @endinternal
*
* @brief   The function Transplant the FDB entries according to the input attributes.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in]  fdbManagerId               - the FDB Manager id.
*                                          (APPLICABLE RANGES : 0..31)
* @param[in]  scanStart                  - Indication to get Next API:
*                                          GT_TRUE  - Start the scan from start(even if lastTranslplantInfo is valid)
*                                          GT_FALSE - Start from lastDeleteInfo(in case of lastTranslplantInfo is invalid, get the first entry)
* @param[in]  paramsPtr                  - the Transplant scan attributes.
* @param[out] entriesTransplantedArray   - (pointer to) Update event array.
* @param[out] entriesTransplantedNumPtr  - (pointer to) Number of Update events filled in array.
* @param[out] transplantScanStatisticsPtr- (pointer to) the type of API ok/error statistics
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - if no more entries to retrieve
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS mainLogicTransplantScan
(
    IN  GT_U32                                                           fdbManagerId,
    IN  GT_BOOL                                                          scanStart,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_TRANSPLANT_SCAN_PARAMS_STC            *paramsPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC                 entriesTransplantedArray[],
    OUT GT_U32                                                          *entriesTransplantedNumPtr,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_SCAN_TRANSPLANT_API_STATISTICS_ENT    *transplantScanStatisticsPtr
)
{
    GT_STATUS                                                    rc = GT_OK;
    GT_STATUS                                                    rc1;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC              *fdbManagerPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC               *dbEntryPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC               *dbEntryExtPtr;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC            *eventPtr;
    GT_U32                                                       eventArrIndex;
    GT_BOOL                                                      isValid;
    GT_U32                                                       currEntries;

    *transplantScanStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_SCAN_TRANSPLANT_API_STATISTICS_ERROR_INPUT_INVALID_E;

    /* Check for valid arguments */
    CPSS_NULL_PTR_CHECK_MAC(paramsPtr);

    /* make here INPUT parameters check , to not wait for
       prvCpssDxChFdbManagerDbEntryUpdate(...) to fail */
    switch(paramsPtr->newInterface.type)
    {
        case CPSS_INTERFACE_PORT_E:
        case CPSS_INTERFACE_TRUNK_E:
            break;
        default:
            /* Other interfaces are not valid for transplant */
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(paramsPtr->newInterface.type);
    }
    switch(paramsPtr->oldInterface.type)
    {
        case CPSS_INTERFACE_PORT_E:
        case CPSS_INTERFACE_TRUNK_E:
            break;
        default:
            /* Other interfaces are not valid for transplant */
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(paramsPtr->oldInterface.type);
    }

    /* Skip fdbManagerId Validation, already verified by the caller */
    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    *transplantScanStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_SCAN_TRANSPLANT_API_STATISTICS_ERROR_FAILED_TABLE_UPDATE_E;

    eventArrIndex = 0;
    currEntries = 0;
    rc1 = prvCpssDxChFdbManagerDbApplicationIteratorEntryGetNext(fdbManagerPtr,
                                                        scanStart,
                                                        &fdbManagerPtr->lastTranslplantInfo,
                                                        &dbEntryPtr);
    while(rc1 != GT_NO_MORE)
    {
        /* Check if entry is valid for transplant process - Filters and Entry Types */
        rc = validateTransplantParamsEntryTypeAndFilters(fdbManagerPtr, paramsPtr, dbEntryPtr, &isValid);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(isValid && /* the 'filters' allow to transplant */
           NULL != fdbManagerPtr->dynamicUcMacLimitPtr && /* the 'limit' DB is enabled */
           dbEntryPtr->hwFdbEntryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E )
        {
            if(GT_TRUE == prvCpssDxChFdbManagerDbIsOverLimitCheck_newSpecificInterface(
                fdbManagerPtr,
                &dbEntryPtr->specificFormat.prvMacEntryFormat,/* the entry with old interface */
                &paramsPtr->newInterface))/* the new interface*/
            {
                if(entriesTransplantedArray != NULL)
                {
                    eventPtr = &entriesTransplantedArray[eventArrIndex];
                    eventPtr->updateType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_MOVED_REACH_LIMIT_AND_DELETED_E;

                    switch(fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo.rcError)
                    {
                        case GT_LEARN_LIMIT_PORT_ERROR           :
                            eventPtr->updateInfo.movedReachLimitEntryFormat.limitInfo.limitType  = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E;
                            eventPtr->updateInfo.movedReachLimitEntryFormat.limitInfo.limitIndex.devPort.hwDevNum = fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo.devPort.hwDevNum;
                            eventPtr->updateInfo.movedReachLimitEntryFormat.limitInfo.limitIndex.devPort.portNum  = fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo.devPort.portNum;
                            break;
                        case GT_LEARN_LIMIT_TRUNK_ERROR          :
                            eventPtr->updateInfo.movedReachLimitEntryFormat.limitInfo.limitType  = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_TRUNK_E;
                            eventPtr->updateInfo.movedReachLimitEntryFormat.limitInfo.limitIndex.trunkId = (GT_TRUNK_ID)fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo.trunkId;
                            break;
                        case GT_LEARN_LIMIT_GLOBAL_EPORT_ERROR   :
                            eventPtr->updateInfo.movedReachLimitEntryFormat.limitInfo.limitType  = CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_GLOBAL_EPORT_E;
                            eventPtr->updateInfo.movedReachLimitEntryFormat.limitInfo.limitIndex.globalEport = fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo.globalEport;
                            break;
                        default: /* GT_OK,other errors : SHOULD NOT GET HERE*/
                            break;
                    }
                }/*(entriesTransplantedArray != NULL)*/

                /* the movement of the entry to new interface cause to limit violation */
                /* we need to delete this entry !                                      */
                /* need to allow application to see the deleted entry */
                rc = prvCpssDxChBrgFdbManagerRestoreEntryFormatParamsFromDb(fdbManagerPtr,
                        dbEntryPtr,
                        &entriesTransplantedArray[eventArrIndex].entry);
                if(rc != GT_OK)
                {
                    return rc;
                }

                /* delete the entry from HW and SW */
                /* it will also decrement 'limit counters' on the old interface (if applicable) */
                rc = prvCpssDxChFdbManagerDbEntryDelete(fdbManagerPtr,dbEntryPtr);
                if(rc != GT_OK)
                {
                    return rc;
                }

                /* we have dedicated statistics for those transplant that deleted */
                fdbManagerPtr->apiTransplantScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_TRANSPLANT_API_STATISTICS_TOTAL_TRANSPLANTED_REACH_LIMIT_DELETED_E]++;

                eventArrIndex+=1;
                isValid = GT_FALSE;  /* indicate the next code to not update this entry */
            }
            else
            {
                /* 1. get to fill info about the old entry , into:
                    fdbManagerPtr->dynamicUcMacLimitPtr->oldInterfaceLimitInfo */
                (void)prvCpssDxChFdbManagerDbIsOverLimitCheck(fdbManagerPtr,
                    LIMIT_OPER_INTERFACE_UPDATE_OLD_PART_INFO_GET_E,
                    &dbEntryPtr->specificFormat.prvMacEntryFormat);

                /* 2. we already have info in fdbManagerPtr->dynamicUcMacLimitPtr->lastLimitInfo
                   that hold info about the new interface
                   was done by prvCpssDxChFdbManagerDbIsOverLimitCheck_newSpecificInterface
                */

                /*
                   3. the actual update of the limit counters , will be in
                        prvCpssDxChFdbManagerDbEntryUpdate(...) just before HW update:
                        a. decrement 'limit counters' on the old interface (if applicable)
                        b. increment 'limit counters' on the new interface (if applicable)
                */
            }
        }

        if(isValid)
        {
            /* Only update the entry to OUT event - Before update */
            if(entriesTransplantedArray != NULL)
            {
                /* Get IPv6 Data part for, IPv6 Unicast Key type */
                if(dbEntryPtr->hwFdbEntryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E)
                {
                    dbEntryExtPtr = &fdbManagerPtr->entryPoolPtr[fdbManagerPtr->indexArr[dbEntryPtr->hwIndex+1].entryPointer];
                    rc = prvCpssDxChBrgFdbManagerRestoreEntryIpV6UcFormatParamsFromDb(fdbManagerPtr,
                            dbEntryPtr,
                            dbEntryExtPtr,
                            &entriesTransplantedArray[eventArrIndex].entry);
                }
                else
                {
                    rc = prvCpssDxChBrgFdbManagerRestoreEntryFormatParamsFromDb(fdbManagerPtr,
                            dbEntryPtr,
                            &entriesTransplantedArray[eventArrIndex].entry);
                }
                if(rc != GT_OK)
                {
                    return rc;
                }
                entriesTransplantedArray[eventArrIndex].updateType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_TRANSPLANTED_E;
            }
            eventArrIndex+=1;
            rc = prvCpssDxChFdbManagerDbEntryUpdate(fdbManagerPtr, dbEntryPtr, &paramsPtr->newInterface);
            if(rc != GT_OK)
            {
                return rc;
            }
            fdbManagerPtr->apiTransplantScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_TRANSPLANT_API_STATISTICS_TOTAL_TRANSPLANTED_E]++;
        }
        currEntries+=1;
        if(currEntries >= fdbManagerPtr->capacityInfo.maxEntriesPerTransplantScan)
        {
            break;
        }
        rc1 = prvCpssDxChFdbManagerDbApplicationIteratorEntryGetNext(fdbManagerPtr,
                                                        GT_FALSE,
                                                        &fdbManagerPtr->lastTranslplantInfo,
                                                        &dbEntryPtr);
    }

    if(rc1 == GT_NO_MORE)
    {
        *transplantScanStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_SCAN_TRANSPLANT_API_STATISTICS_OK_NO_MORE_E;
        rc = GT_NO_MORE;
    }
    else
    {
        *transplantScanStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_SCAN_TRANSPLANT_API_STATISTICS_OK_E;
    }

    if(entriesTransplantedNumPtr != NULL)
    {
        *entriesTransplantedNumPtr = eventArrIndex;
    }
    return rc;
}

/**
* @internal internal_cpssDxChBrgFdbManagerTransplantScan function
* @endinternal
*
* @brief   The function scans the entire FDB (Till maximal per scan allowed).
*          Transplant(Change the source info) the filtered entries.
*          (Applicable entry types - MAC Unicast(static/dynamic), IPv4/6 Unicast)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId               - the FDB Manager id.
*                                         (APPLICABLE RANGES : 0..31)
* @param[in] scanStart                  - Indicates the starting point of the scan.
*                                         GT_TRUE  - Start from the beginning of the used list.
*                                         GT_FALSE - Start from the last scanned point.
* @param[in] paramsPtr                  - the transplant scan attributes.
* @param[out] entriesTransplantedArray  - (pointer to) Update event array.
*                                         This is optional (can be NULL if application do not require it).
*                                         If used, should have memory for at least "max entries allowed per transplant scan".
* @param[out] entriesTransplantedNumPtr - (pointer to) Number of Update events filled in array.
*                                         This is optional (can be NULL if application do not require it).
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - if no more entries to transplant
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong fdbManagerId and paramsPtr data
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*          Invalid Filter table for entry type
*          +-------------------------+------------+-------------+
*          |                         |FID         |Interface    |
*          +-------------------------+------------+-------------+
*          |MAC Unicast              |Valid       |Valid        |
*          |IPv4/6 Unicast           |NA          |Valid        |
*          +-------------------------+------------+-------------+
*/
GT_STATUS internal_cpssDxChBrgFdbManagerTransplantScan
(
    IN  GT_U32                                                fdbManagerId,
    IN  GT_BOOL                                               scanStart,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_TRANSPLANT_SCAN_PARAMS_STC *paramsPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC      entriesTransplantedArray[],
    OUT GT_U32                                               *entriesTransplantedNumPtr
)
{
    GT_STATUS                                                     rc;
    PRV_CPSS_DXCH_FDB_MANAGER_SCAN_TRANSPLANT_API_STATISTICS_ENT  transplantScanStatistics;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC                *fdbManagerPtr;

    FDB_MANAGER_ID_CHECK(fdbManagerId);
    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    /* main logic of Transplant scan */
    rc = mainLogicTransplantScan(fdbManagerId, scanStart, paramsPtr, entriesTransplantedArray, entriesTransplantedNumPtr, &transplantScanStatistics);

    if(transplantScanStatistics < PRV_CPSS_DXCH_FDB_MANAGER_SCAN_TRANSPLANT_API_STATISTICS___LAST___E)
    {
        /* increment the 'API statistics' */
        fdbManagerPtr->apiTransplantScanStatisticsArr[transplantScanStatistics]++;
    }
    return rc;
}


/**
* @internal cpssDxChBrgFdbManagerTransplantScan function
* @endinternal
*
* @brief   The function scans the entire FDB (Till maximal per scan allowed).
*          Transplant(Change the source info) the filtered entries.
*          (Applicable entry types - MAC Unicast(static/dynamic), IPv4/6 Unicast)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId               - the FDB Manager id.
*                                         (APPLICABLE RANGES : 0..31)
* @param[in] scanStart                  - Indicates the starting point of the scan.
*                                         GT_TRUE  - Start from the beginning of the used list.
*                                         GT_FALSE - Start from the last scanned point.
* @param[in] paramsPtr                  - the transplant scan attributes.
* @param[out] entriesTransplantedArray  - (pointer to) Update event array.
*                                         This is optional (can be NULL if application do not require it).
*                                         If used, should have memory for at least "max entries allowed per transplant scan".
* @param[out] entriesTransplantedNumPtr - (pointer to) Number of Update events filled in array.
*                                         This is optional (can be NULL if application do not require it).
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - if no more entries to transplant
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong fdbManagerId and paramsPtr data
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*          Invalid Filter table for entry type
*          +-------------------------+------------+-------------+
*          |                         |FID         |Interface    |
*          +-------------------------+------------+-------------+
*          |MAC Unicast              |Valid       |Valid        |
*          |IPv4/6 Unicast           |NA          |Valid        |
*          +-------------------------+------------+-------------+
*/
GT_STATUS cpssDxChBrgFdbManagerTransplantScan
(
    IN  GT_U32                                                fdbManagerId,
    IN  GT_BOOL                                               scanStart,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_TRANSPLANT_SCAN_PARAMS_STC *paramsPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC      entriesTransplantedArray[],
    OUT GT_U32                                               *entriesTransplantedNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbManagerTransplantScan);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, fdbManagerId, scanStart, paramsPtr, entriesTransplantedArray, entriesTransplantedNumPtr));

    rc = internal_cpssDxChBrgFdbManagerTransplantScan(fdbManagerId, scanStart, paramsPtr, entriesTransplantedArray, entriesTransplantedNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, fdbManagerId, scanStart, paramsPtr, entriesTransplantedArray, entriesTransplantedNumPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}

/**
* @internal validateDeleteParamsEntryTypeAndFilters_ip_multicast function
* @endinternal
*
* @brief  function to validate the Delete scan param for IP multicast entry type.
*
* @param[in] paramsPtr             - (pointer to) the delete scan attributes.
* @param[in] dbEntryIpMcPtr        - (pointer to) the IP multicast entry format (DB entry)
* @param[out] isValidPtr           - (pointer to) the valid status.
*                                    GT_TRUE - Included in the delete scan.
*                                    GT_FALSE - Exclude from the delete scan.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS validateDeleteParamsEntryTypeAndFilters_ip_multicast
(
    IN CPSS_DXCH_BRG_FDB_MANAGER_DELETE_SCAN_PARAMS_STC          *paramsPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IP_MC_ADDR_FORMAT_STC  *dbEntryIpMcPtr,
    OUT GT_BOOL                                                  *isValidPtr
)
{
    /* invalidate the entries where not applicable filters are enabled */
    if((paramsPtr->deleteIpMcEntries != GT_TRUE) ||
        paramsPtr->hwDevNumMask || paramsPtr->ePortTrunkNumMask || paramsPtr->isTrunkMask)
    {
        return GT_OK;
    }

    switch(dbEntryIpMcPtr->dstInterface_type)
    {
        case CPSS_INTERFACE_VIDX_E:
        case CPSS_INTERFACE_VID_E:
            break;
        default:
            /* IP multicast entry with non-Multicast interface is not valid
             * CPSS add should not allow such entries */
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(dbEntryIpMcPtr->dstInterface_type);
    }

    FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->fidMask, dbEntryIpMcPtr->fid, paramsPtr->fid);
    FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->userDefinedMask, dbEntryIpMcPtr->userDefined, paramsPtr->userDefined);
    *isValidPtr  = GT_TRUE;
    return GT_OK;
}

/**
* @internal validateDeleteParamsEntryTypeAndFilters_ip_unicast function
* @endinternal
*
* @brief  function to validate the Delete scan param for IP unicast entry type.
*
* @param[in] paramsPtr             - (pointer to) the delete scan attributes.
* @param[in] dbEntryIpUcPtr        - (pointer to) the IP unicast format (DB entry)
* @param[out] isValidPtr           - (pointer to) the valid status.
*                                    GT_TRUE - Included in the delete scan.
*                                    GT_TRUE - Exclude from the delete scan.
* @param[out] deleteEnPtr          - (pointer to) the delete status.
*                                    GT_TRUE - Delete required if aged-out.
*                                    GT_FALSE - Do not delete even if aged-out.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS validateDeleteParamsEntryTypeAndFilters_ip_unicast
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_DELETE_SCAN_PARAMS_STC         *paramsPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_IPV_UC_COMMON_FORMAT_STC  *dbEntryIpUcPtr,
    OUT GT_BOOL                                                     *isValidPtr
)
{
    /* invalidate the entries where not applicable filters are enabled */
    if((paramsPtr->deleteIpUcEntries != GT_TRUE) ||
        paramsPtr->fidMask || paramsPtr->vid1Mask || paramsPtr->userDefinedMask)
    {
        return GT_OK;
    }

    switch(dbEntryIpUcPtr->dstInterface_type)
    {
        case CPSS_INTERFACE_VIDX_E:
        case CPSS_INTERFACE_VID_E:
            /* unicast entries treated as multicast. */
            break;
        case CPSS_INTERFACE_PORT_E:
            FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->isTrunkMask, GT_FALSE, paramsPtr->isTrunk);  /* isTrunk == GT_FALSE */
            FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->ePortTrunkNumMask, dbEntryIpUcPtr->dstInterface.devPort.portNum, paramsPtr->ePortTrunkNum);
            if(GT_FALSE == prvCpssDxChFdbManagerDbIsGlobalEport(fdbManagerPtr,
                dbEntryIpUcPtr->dstInterface.devPort.portNum))
            {
                /* check hwDevNum only if the entry is not associated with the global eport */
                FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->hwDevNumMask, dbEntryIpUcPtr->dstInterface.devPort.hwDevNum, paramsPtr->hwDevNum);
            }
            break;
        case CPSS_INTERFACE_TRUNK_E:
            FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->isTrunkMask, GT_TRUE, paramsPtr->isTrunk);  /* isTrunk == GT_TRUE */
            FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->hwDevNumMask, dbEntryIpUcPtr->dstInterface.devPort.hwDevNum, paramsPtr->hwDevNum);
            FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->ePortTrunkNumMask, dbEntryIpUcPtr->dstInterface.trunkId, paramsPtr->ePortTrunkNum);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(dbEntryIpUcPtr->dstInterface_type);
    }
    *isValidPtr  = GT_TRUE;
    return GT_OK;
}


/**
* @internal validateDeleteParamsEntryTypeAndFilters_mac function
* @endinternal
*
* @brief  function to validate the scan param for MAC entry type.
*
* @param[in] macEntryMuxingMode    - The MAC entry muxing mode.
* @param[in] paramsPtr             - (pointer to) the scan attributes.
* @param[in] dbEntryIpUcPtr        - (pointer to) the MAC format (DB entry)
* @param[out] isValidPtr           - (pointer to) the valid status.
*                                    GT_TRUE - Included in the delete scan.
*                                    GT_FALSE - Exclude from the delete scan.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS validateDeleteParamsEntryTypeAndFilters_mac
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_ENT      macEntryMuxingMode,
    IN CPSS_DXCH_BRG_FDB_MANAGER_DELETE_SCAN_PARAMS_STC        *paramsPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_MAC_ADDR_FORMAT_STC  *dbEntryMacPtr,
    OUT GT_BOOL                                                *isValidPtr
)
{
    if (dbEntryMacPtr->isStatic == GT_TRUE)
    {
        FDB_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->deleteStaticEntries, GT_TRUE);
    }
    switch(dbEntryMacPtr->dstInterface_type)
    {
        /* MAC Multicast - Not applicable filters - Device, ePortTrunk, isTrunk */
        case CPSS_INTERFACE_VIDX_E:
        case CPSS_INTERFACE_VID_E:
            FDB_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->deleteMacMcEntries, GT_TRUE);
            /* invalidate the entries where not applicable filters are enabled */
            if(paramsPtr->hwDevNumMask || paramsPtr->ePortTrunkNumMask || paramsPtr->isTrunkMask)
            {
                return GT_OK;
            }
            FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->fidMask, dbEntryMacPtr->fid, paramsPtr->fid);
            FDB_SCAN_VALIDATE_FILTER_VID1_MAC_AND_RETURN_MAC(macEntryMuxingMode, paramsPtr->vid1Mask, dbEntryMacPtr->vid1, paramsPtr->vid1);
            FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->userDefinedMask, dbEntryMacPtr->userDefined, paramsPtr->userDefined);
            break;
        case CPSS_INTERFACE_PORT_E:
            FDB_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->deleteMacUcEntries, GT_TRUE);
            FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->isTrunkMask, GT_FALSE, paramsPtr->isTrunk);  /* isTrunk == GT_FALSE */
            FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->fidMask, dbEntryMacPtr->fid, paramsPtr->fid);
            FDB_SCAN_VALIDATE_FILTER_VID1_MAC_AND_RETURN_MAC(macEntryMuxingMode, paramsPtr->vid1Mask, dbEntryMacPtr->vid1, paramsPtr->vid1);
            FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->ePortTrunkNumMask, dbEntryMacPtr->dstInterface.devPort.portNum, paramsPtr->ePortTrunkNum);
            FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->userDefinedMask, dbEntryMacPtr->userDefined, paramsPtr->userDefined);
            if(GT_FALSE == prvCpssDxChFdbManagerDbIsGlobalEport(fdbManagerPtr,
                dbEntryMacPtr->dstInterface.devPort.portNum))
            {
                /* check hwDevNum only if the entry is not associated with the global eport */
                FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->hwDevNumMask, dbEntryMacPtr->dstInterface.devPort.hwDevNum, paramsPtr->hwDevNum);
            }
            break;
        case CPSS_INTERFACE_TRUNK_E:
            FDB_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->deleteMacUcEntries, GT_TRUE);
            FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->isTrunkMask, GT_TRUE, paramsPtr->isTrunk);   /* isTrunk == GT_TRUE */
            FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->fidMask, dbEntryMacPtr->fid, paramsPtr->fid);
            FDB_SCAN_VALIDATE_FILTER_VID1_MAC_AND_RETURN_MAC(macEntryMuxingMode, paramsPtr->vid1Mask, dbEntryMacPtr->vid1, paramsPtr->vid1);
            FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->ePortTrunkNumMask, dbEntryMacPtr->dstInterface.trunkId, paramsPtr->ePortTrunkNum);
            FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->userDefinedMask, dbEntryMacPtr->userDefined, paramsPtr->userDefined);
            FDB_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->hwDevNumMask, dbEntryMacPtr->dstInterface.devPort.hwDevNum, paramsPtr->hwDevNum);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(dbEntryMacPtr->dstInterface_type);
    }
    *isValidPtr  = GT_TRUE;
    return GT_OK;
}


/**
* @internal validateDeleteParamsEntryTypeAndFilters function
* @endinternal
*
* @brief  function to validate the delete scan param.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[in] paramsPtr             - (pointer to) the delete scan attributes.
* @param[in] dbEntryPtr            - (pointer to) the DB entry format
* @param[out] isValidPtr           - (pointer to) the valid status.
*                                    GT_TRUE - Included in the delete scan.
*                                    GT_FALSE - Exclude from the delete scan.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS validateDeleteParamsEntryTypeAndFilters
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_DELETE_SCAN_PARAMS_STC        *paramsPtr,
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC           *dbEntryPtr,
    OUT GT_BOOL                                                *isValidPtr
)
{
    *isValidPtr  = GT_FALSE;

    if(paramsPtr->onlyAboveLearnLimit == GT_TRUE)
    {
        if(dbEntryPtr->hwFdbEntryType != PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E)
        {
            /* this mode relevant only to 'TYPE_MAC_ADDR' */
            return GT_OK;
        }

        if(GT_FALSE == prvCpssDxChFdbManagerDbIsOverLimitCheck(fdbManagerPtr,LIMIT_OPER_CHECK_CURRENT_E,
            &dbEntryPtr->specificFormat.prvMacEntryFormat))
        {
            /* the entry not violate any limit */
            return GT_OK;
        }

        /* if we got here ... the next code will call to function :
           validateDeleteParamsEntryTypeAndFilters_mac(...)
           to allow to narrow down the deleted entries.(according to masks)
        */
    }

    switch(dbEntryPtr->hwFdbEntryType)
    {
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E:
            /* MAC Unicast - All Filter are Applicable */
            return validateDeleteParamsEntryTypeAndFilters_mac(fdbManagerPtr,fdbManagerPtr->entryAttrInfo.macEntryMuxingMode,
                                                         paramsPtr,
                                                        &dbEntryPtr->specificFormat.prvMacEntryFormat,
                                                         isValidPtr);

        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_UC_E:
            /* IP Unicast - Not applicable filters - fid, vid1, userDefined */
            return validateDeleteParamsEntryTypeAndFilters_ip_unicast(fdbManagerPtr,paramsPtr,
                                                             &dbEntryPtr->specificFormat.prvIpv4UcEntryFormat.ipUcCommonInfo,
                                                              isValidPtr);
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E:
            return validateDeleteParamsEntryTypeAndFilters_ip_unicast(fdbManagerPtr,paramsPtr,
                                                             &dbEntryPtr->specificFormat.prvIpv6UcDataEntryFormat.ipUcCommonInfo,
                                                              isValidPtr);

        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_MCAST_E:
            /* IP Multicast - Not applicable filters - Device, ePortTrunk, isTrunk */
            return validateDeleteParamsEntryTypeAndFilters_ip_multicast(paramsPtr,
                                                             &dbEntryPtr->specificFormat.prvIpv4McEntryFormat,
                                                              isValidPtr);
        case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_MCAST_E:
            return validateDeleteParamsEntryTypeAndFilters_ip_multicast(paramsPtr,
                                                             &dbEntryPtr->specificFormat.prvIpv6McEntryFormat,
                                                              isValidPtr);
        default:
            break;
    }
    return GT_OK;
}

/**
* @internal mainLogicDeleteScan function
* @endinternal
*
* @brief   The function delete the FDB entries according to the input attributes.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in]  fdbManagerId          - the FDB Manager id.
*                                     (APPLICABLE RANGES : 0..31)
* @param[in]  scanStart             - Indication to get Next API:
*                                       GT_TRUE  - Start the scan from start(even if lastDeleteInfo is valid)
*                                       GT_FALSE - Start from lastDeleteInfo(in case of lastDeleteInfo is invalid, get the first entry)
* @param[in]  paramsPtr             - the delete scan attributes.
* @param[out] entriesDeletedArray   - (pointer to) Update event array.
* @param[out] entriesDeletedNumPtr  - (pointer to) Number of Update events filled in array.
* @param[out] deleteScanStatistics  - (pointer to) the type of API ok/error statistics
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - if no more entries to retrieve
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or portNum.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS mainLogicDeleteScan
(
    IN  GT_U32                                                      fdbManagerId,
    IN  GT_BOOL                                                     scanStart,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_DELETE_SCAN_PARAMS_STC            *paramsPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC            entriesDeletedArray[],
    OUT GT_U32                                                      *entriesDeletedNumPtr,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_SCAN_DELETE_API_STATISTICS_ENT    *deleteScanStatisticsPtr
)
{
    GT_STATUS                                                    rc = GT_OK;
    GT_STATUS                                                    rc1;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC              *fdbManagerPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC               *dbEntryPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_ENTRY_FORMAT_STC               *dbEntryExtPtr = NULL;
    GT_U32                                                       eventArrIndex;
    GT_BOOL                                                      isValid;
    GT_U32                                                       currEntries;

    *deleteScanStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_SCAN_DELETE_API_STATISTICS_ERROR_INPUT_INVALID_E;

    /* Check for valid arguments */
    CPSS_NULL_PTR_CHECK_MAC(paramsPtr);

    /* Skip fdbManagerId Validation, already verified by the caller */
    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    *deleteScanStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_SCAN_DELETE_API_STATISTICS_ERROR_FAILED_TABLE_UPDATE_E;

    eventArrIndex = 0;
    currEntries = 0;
    rc1 = prvCpssDxChFdbManagerDbApplicationIteratorEntryGetNext(fdbManagerPtr,
                                                        scanStart,
                                                        &fdbManagerPtr->lastDeleteInfo,
                                                        &dbEntryPtr);
    while(rc1 != GT_NO_MORE)
    {
        /* Check if entry is valid for delete process - Filters and Entry Types */
        if(dbEntryPtr->hwFdbEntryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E)
        {
            /* IPv6 Data entry is validated as a part of IPv6 Key entry */
            rc1 = prvCpssDxChFdbManagerDbApplicationIteratorEntryGetNext(fdbManagerPtr,
                    GT_FALSE,
                    &fdbManagerPtr->lastDeleteInfo,
                    &dbEntryPtr);
            continue;
        }
        if(dbEntryPtr->hwFdbEntryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E)
        {
            /* In case of entry type IPv6 data entry need to be validated */
            dbEntryExtPtr = &fdbManagerPtr->entryPoolPtr[fdbManagerPtr->indexArr[dbEntryPtr->hwIndex+1].entryPointer];
            rc = validateDeleteParamsEntryTypeAndFilters(fdbManagerPtr, paramsPtr, dbEntryExtPtr, &isValid);
        }
        else
        {
            rc = validateDeleteParamsEntryTypeAndFilters(fdbManagerPtr, paramsPtr, dbEntryPtr, &isValid);
        }
        if(rc != GT_OK)
        {
            return rc;
        }

        if(isValid)
        {
            /* Only update the entry to OUT event - Before delete */
            if(entriesDeletedArray != NULL)
            {
                /* Get IPv6 Data part for, IPv6 Unicast Key type */
                if(dbEntryPtr->hwFdbEntryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E)
                {
                    rc = prvCpssDxChBrgFdbManagerRestoreEntryIpV6UcFormatParamsFromDb(fdbManagerPtr,
                            dbEntryPtr,
                            dbEntryExtPtr,
                            &entriesDeletedArray[eventArrIndex].entry);
                }
                else
                {
                    rc = prvCpssDxChBrgFdbManagerRestoreEntryFormatParamsFromDb(fdbManagerPtr,
                            dbEntryPtr,
                            &entriesDeletedArray[eventArrIndex].entry);
                }
                if(rc != GT_OK)
                {
                    return rc;
                }
                entriesDeletedArray[eventArrIndex].updateType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_DELETED_E;
            }
            eventArrIndex+=1;
            rc = prvCpssDxChFdbManagerDbEntryDelete(fdbManagerPtr, dbEntryPtr);
            if(rc != GT_OK)
            {
                return rc;
            }
            fdbManagerPtr->apiDeleteScanStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_SCAN_DELETE_API_STATISTICS_TOTAL_DELETED_E]++;
        }
        currEntries+=1;
        if(currEntries >= fdbManagerPtr->capacityInfo.maxEntriesPerDeleteScan)
        {
            break;
        }
        rc1 = prvCpssDxChFdbManagerDbApplicationIteratorEntryGetNext(fdbManagerPtr,
                                                        GT_FALSE,
                                                        &fdbManagerPtr->lastDeleteInfo,
                                                        &dbEntryPtr);
    }

    if(rc1 == GT_NO_MORE)
    {
        *deleteScanStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_SCAN_DELETE_API_STATISTICS_OK_NO_MORE_E;
        rc = GT_NO_MORE;
    }
    else
    {
        *deleteScanStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_SCAN_DELETE_API_STATISTICS_OK_E;
    }

    if(entriesDeletedNumPtr != NULL)
    {
        *entriesDeletedNumPtr = eventArrIndex;
    }
    return rc;
}

/**
* @internal internal_cpssDxChBrgFdbManagerDeleteScan function
* @endinternal
*
* @brief   The function scans the entire FDB (Till maximal per scan allowed).
*          Delete the filtered entries.
*          (Applicable entry types - MAC Unicast(static/dynamic), MAC Multicast, IPv4/6 Unicast, IPv4/6 Multicast)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] scanStart             - Indicates the starting point of the scan.
*                                    GT_TRUE  - Start from the beginning of the used list.
*                                    GT_FALSE - Start from the last scanned point.
* @param[in] paramsPtr             - the delete scan attributes.
* @param[out] entriesDeletedArray  - (pointer to) Update event array.
*                                    This is optional (can be NULL if application do not require it).
*                                    If used, should have memory for at least "max entries allowed per delete scan".
* @param[out] entriesDeletedNumPtr - (pointer to) Number of Update events filled in array.
*                                    This is optional (can be NULL if application do not require it).
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - if no more entries to delete
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong fdbManagerId and paramsPtr data
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*          Invalid Filter table for entry type
*          +-------------------------+-------+---------+-------------+--------+---------+------+
*          |                         |FID    |Device   |ePort/Trunk  |isTrunk |Vid1     |User  |
*          +-------------------------+------------+------------------+--------+---------+------+
*          |MAC Unicast              |Valid  |Valid    |Valid        |Valid   |Valid    |Valid |
*          |MAC Multicast            |Valid  |NA       |NA           |NA      |Valid    |Valid |
*          |IPv4/6 Multicast         |Valid  |NA       |NA           |NA      |NA       |Valid |
*          |IPv4/6 Unicast           |NA     |Valid    |Valid        |Valid   |NA       |NA    |
*          +-------------------------+-------+---------+-------------+--------+---------+------+
*/
GT_STATUS internal_cpssDxChBrgFdbManagerDeleteScan
(
    IN  GT_U32                                            fdbManagerId,
    IN  GT_BOOL                                           scanStart,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_DELETE_SCAN_PARAMS_STC *paramsPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC  entriesDeletedArray[],
    OUT GT_U32                                           *entriesDeletedNumPtr
)
{
    GT_STATUS                                                   rc, rc1;
    PRV_CPSS_DXCH_FDB_MANAGER_SCAN_DELETE_API_STATISTICS_ENT    deleteScanStatistics;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC             *fdbManagerPtr;

    FDB_MANAGER_ID_CHECK(fdbManagerId);
    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    /* main logic of Delete scan */
    rc = mainLogicDeleteScan(fdbManagerId, scanStart, paramsPtr, entriesDeletedArray, entriesDeletedNumPtr, &deleteScanStatistics);

    if(deleteScanStatistics < PRV_CPSS_DXCH_FDB_MANAGER_SCAN_DELETE_API_STATISTICS___LAST___E)
    {
        /* increment the 'API statistics' */
        fdbManagerPtr->apiDeleteScanStatisticsArr[deleteScanStatistics]++;
    }
    rc1 = prvCpssDxChFdbManagerThresholdSet(fdbManagerPtr);

    if(rc != GT_OK)
    {
        return rc;
    }
    return rc1;
}
/**
* @internal cpssDxChBrgFdbManagerDeleteScan function
* @endinternal
*
* @brief   The function scans the entire FDB (Till maximal per scan allowed).
*          Delete the filtered entries.
*          (Applicable entry types - MAC Unicast(static/dynamic), MAC Multicast, IPv4/6 Unicast, IPv4/6 Multicast)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] scanStart             - Indicates the starting point of the scan.
*                                    GT_TRUE  - Start from the beginning of the used list.
*                                    GT_FALSE - Start from the last scanned point.
* @param[in] paramsPtr             - the delete scan attributes.
* @param[out] entriesDeletedArray  - (pointer to) Update event array.
*                                    This is optional (can be NULL if application do not require it).
*                                    If used, should have memory for at least "max entries allowed per delete scan".
* @param[out] entriesDeletedNumPtr - (pointer to) Number of Update events filled in array.
*                                    This is optional (can be NULL if application do not require it).
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - if no more entries to delete
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong fdbManagerId and paramsPtr data
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*          Invalid Filter table for entry type
*          +-------------------------+-------+---------+-------------+--------+---------+------+
*          |                         |FID    |Device   |ePort/Trunk  |isTrunk |Vid1     |User  |
*          +-------------------------+------------+------------------+--------+---------+------+
*          |MAC Unicast              |Valid  |Valid    |Valid        |Valid   |Valid    |Valid |
*          |MAC Multicast            |Valid  |NA       |NA           |NA      |Valid    |Valid |
*          |IPv4/6 Multicast         |Valid  |NA       |NA           |NA      |NA       |Valid |
*          |IPv4/6 Unicast           |NA     |Valid    |Valid        |Valid   |NA       |NA    |
*          +-------------------------+-------+---------+-------------+--------+---------+------+
*/
GT_STATUS cpssDxChBrgFdbManagerDeleteScan
(
    IN  GT_U32                                            fdbManagerId,
    IN  GT_BOOL                                           scanStart,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_DELETE_SCAN_PARAMS_STC *paramsPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC  entriesDeletedArray[],
    OUT GT_U32                                           *entriesDeletedNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbManagerDeleteScan);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, fdbManagerId, scanStart, paramsPtr, entriesDeletedArray, entriesDeletedNumPtr));

    rc = internal_cpssDxChBrgFdbManagerDeleteScan(fdbManagerId, scanStart, paramsPtr, entriesDeletedArray, entriesDeletedNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, fdbManagerId, scanStart, paramsPtr, entriesDeletedArray, entriesDeletedNumPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgFdbManagerDatabaseCheck function
* @endinternal
*
* @brief   The function verifies the FDB manager DB/HW according to the input test types.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - The FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] checksPtr             - (Pointer to) DB check attributes
* @param[out] resultArray          - (pointer to) Test result array.
* @param[out] errorNumPtr          - (pointer to) Number of error types filled in resultArray
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong fdbManagerId and checksPtr data
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChBrgFdbManagerDatabaseCheck
(
    IN  GT_U32                                           fdbManagerId,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_STC          *checksPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT    resultArray[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E],
    OUT GT_U32                                           *errorNumPtr
)
{
    FDB_MANAGER_ID_CHECK(fdbManagerId);

    /* Check for valid arguments */
    CPSS_NULL_PTR_CHECK_MAC(resultArray);
    CPSS_NULL_PTR_CHECK_MAC(errorNumPtr);
    CPSS_NULL_PTR_CHECK_MAC(checksPtr);

    return prvCpssDxChBrgFdbManagerDbCheck(fdbManagerId, checksPtr, resultArray, errorNumPtr);
}

/**
* @internal cpssDxChBrgFdbManagerDatabaseCheck function
* @endinternal
*
* @brief   The function verifies the FDB manager DB/HW according to the input test types.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - The FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] checksPtr             - (Pointer to) DB check attributes
* @param[out] resultArray          - (pointer to) Test result array.
* @param[out] errorNumPtr          - (pointer to) Number of error types filled in resultArray
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong fdbManagerId and checksPtr data
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChBrgFdbManagerDatabaseCheck
(
    IN  GT_U32                                           fdbManagerId,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_STC          *checksPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT    resultArray[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E],
    OUT GT_U32                                           *errorNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbManagerDatabaseCheck);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, fdbManagerId, checksPtr, resultArray, errorNumPtr));

    rc = internal_cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, checksPtr, resultArray, errorNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, fdbManagerId, scanStart, checksPtr, resultArray, errorNumPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}

/**
* @internal prvCpssDxChFdbManagerHaReadEntry_sip5 function
* @endinternal
*
* @brief The function reads FDB entry from HW.
*
* @param[in] devNum               - device number.
* @param[in] hwIndex              - HW index to the FDB entry.
* @param[in] fdbManagerPtr        - (pointer to) the FDB Manager.
* @param[out] addParamsPtr        - (pointer to) the add param for FDB Manager entry add.
* @param[Out] tempEntryIsNonMac   - (Pointer to) the type indication of tempEntry,
*                                    GT_TRUE  - temp Entry offset in addParamsPtr is for non-mac type.
*                                    GT_FALSE - temp entry offset in addParamsPtr is for mac type.
* @param[out] isValidPtr          - Indicates is the entry is valid to add to SW/HW.
*
* @retval GT_OK                   - on success
* @retval GT_FAIL                 - on error
* @retval GT_BAD_PARAM            - on wrong FDB manager ID.
* @retval GT_NOT_INITIALIZED      - if the FDB Manager is not initialized.
*
*/
static GT_STATUS prvCpssDxChFdbManagerHaReadEntry_sip5
(
    IN  GT_U8                                             devNum,
    IN  GT_U32                                            hwIndex,
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   *addParamsPtr,
    OUT GT_BOOL                                          *isValidPtr
)
{
    GT_STATUS      rc;
    GT_U32         hwValue;
    GT_U32         entryType;
    GT_U32         *hwDataArr    = &fdbManagerPtr->tempInfo.sip6_hwFormatWords[0];
    GT_U32         *hwDataExtArr = &fdbManagerPtr->tempInfo.sip6_ipv6Ext_hwFormatWords[0];
    GT_U32         tempBankId    = hwIndex % fdbManagerPtr->numOfBanks;
    GT_BOOL        isIpv6        = GT_FALSE;
    GT_BOOL        isSkip        = GT_FALSE;

    *isValidPtr = GT_FALSE;

    /*** Read entries from HW (1st PP, 1st Tile) ***/
    rc = prvCpssDxChPortGroupReadTableEntry(devNum,
            0,                      /*portGroupId - 0, indicates 1st PP's 1st tile*/
            CPSS_DXCH_TABLE_FDB_E,
            hwIndex,
            hwDataArr);
    if(rc != GT_OK)
    {
        return rc;
    }

    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP5_FDB_FDB_TABLE_FIELDS_SKIP_E, hwValue);
    isSkip = BIT2BOOL_MAC(hwValue);
    SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP5_FDB_FDB_TABLE_FIELDS_VALID_E, hwValue);

    if(hwValue == 1 && (isSkip == GT_FALSE))
    {
        /* Get IPv6 data and key */
        SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                SIP5_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE_E,
                entryType);

        /* Get the IPv6 Data part in case of entry is valid and type is IPv6 */
        if(entryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E)
        {
            isIpv6 = GT_TRUE;
            rc = prvCpssDxChPortGroupReadTableEntry(devNum,
                    0,                      /*portGroupId - 0, indicates 1st PP's 1st tile*/
                    CPSS_DXCH_TABLE_FDB_E,
                    hwIndex+1,              /* IPv6 Data Part, next HwIndex */
                    hwDataExtArr);
            if(rc != GT_OK)
            {
                return rc;
            }

            SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataExtArr, SIP5_FDB_FDB_TABLE_FIELDS_VALID_E, hwValue);
            if(hwValue == 1)
            {
                *isValidPtr = GT_TRUE;
            }
            else
            {
                fdbManagerPtr->apiHASyncStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_HA_SYNC_API_STATISTICS_TOTAL_IPV6_DATA_INVALID_ENTRY_E]+=1;
            }
        }
        else if(entryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E)
        {
            /* Skip the FDB IPv6 data part, as its already processed while reading key part. */
            return GT_OK;
        }

        if(entryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E)
        {
            SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP5_FDB_FDB_TABLE_FIELDS_SP_UNKNOWN_E, hwValue);
            if(hwValue == 0)
            {
                *isValidPtr                     = GT_TRUE;
                addParamsPtr->tempEntryOffset   = tempBankId;
                addParamsPtr->tempEntryExist    = GT_TRUE;
            }
            else
            {
                fdbManagerPtr->apiHASyncStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_HA_SYNC_API_STATISTICS_TOTAL_SP_ENTRY_E]+=1;
            }
        }
        else
        {
            /* Non-MAC entry already HW index is decided */
            *isValidPtr                     = GT_TRUE;
            addParamsPtr->tempEntryOffset   = tempBankId;
            addParamsPtr->tempEntryExist    = GT_FALSE;
        }
    }

    if(*isValidPtr == GT_FALSE)
    {
        fdbManagerPtr->apiHASyncStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_HA_SYNC_API_STATISTICS_TOTAL_INVALID_E]+=1;
        /*** call flush entry for invalid/SP entries ***/
        rc = prvCpssDxChFdbManagerHwFlushByHwIndex(fdbManagerPtr,
                hwIndex,
                tempBankId  /* BankId */,
                GT_FALSE    /* Do not update counter - in case of invalid/SP entry */);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Flush the data part also in case of IPV6 */
        if(isIpv6)
        {
            rc = prvCpssDxChFdbManagerHwFlushByHwIndex(fdbManagerPtr,
                    hwIndex+1,
                    tempBankId  /* BankId */,
                    GT_FALSE    /* Do not update counter - in case of invalid/SP entry */);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChFdbManagerHwEntryRead function
* @endinternal
*
* @brief The function reads FDB entry from HW.
*
* @param[in] devNum               - device number.
* @param[in] hwIndex              - HW index to the FDB entry.
* @param[in] fdbManagerPtr        - (pointer to) the FDB Manager.
* @param[in] hwFormatType          - FDB Entry hw format type.
* @param[out] addParamsPtr        - (pointer to) the add param for FDB Manager entry add.
* @param[out] isValidPtr          - Indicates is the entry is valid to add to SW/HW.
*
* @retval GT_OK                   - on success
* @retval GT_FAIL                 - on error
* @retval GT_BAD_PARAM            - on wrong FDB manager ID.
* @retval GT_NOT_INITIALIZED      - if the FDB Manager is not initialized.
*
*/
GT_STATUS prvCpssDxChFdbManagerHwEntryRead
(
    IN  GT_U8                                             devNum,
    IN  GT_U32                                            hwIndex,
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN  PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_ENT  hwFormatType,
    OUT CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   *addParamsPtr,
    OUT GT_BOOL                                          *isValidPtr
)
{
    GT_STATUS      rc;
    GT_U32         hwValue;
    GT_U32         entryType;
    GT_U32         *hwDataArr    = &fdbManagerPtr->tempInfo.sip6_hwFormatWords[0];
    GT_U32         *hwDataExtArr = &fdbManagerPtr->tempInfo.sip6_ipv6Ext_hwFormatWords[0];
    GT_U32         tempBankId    = hwIndex % fdbManagerPtr->numOfBanks;
    GT_BOOL        isIpv6        = GT_FALSE;
    GT_BOOL        isSkip        = GT_FALSE;

    *isValidPtr = GT_FALSE;

    /*** Read entries from HW (1st PP, 1st Tile) ***/
    rc = prvCpssDxChPortGroupReadTableEntry(devNum,
            0,                      /*portGroupId - 0, indicates 1st PP's 1st tile*/
            CPSS_DXCH_TABLE_FDB_E,
            hwIndex,
            hwDataArr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Check Valid and Skip bit from hwDataArr */
    if(hwFormatType == PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP4_E)
    {
        isSkip  = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwDataArr[0],1,1));
        hwValue = U32_GET_FIELD_MAC(hwDataArr[0],0,1);
    }
    else
    {
        SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_SKIP_E, hwValue);
        isSkip = BIT2BOOL_MAC(hwValue);
        SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_VALID_E, hwValue);
    }
    if(hwValue == 1 && (isSkip == GT_FALSE))
    {
        /* Get IPv6 data and key */
        if(hwFormatType == PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP4_E)
        {
            entryType = U32_GET_FIELD_MAC(hwDataArr[0],3,2);
        }
        else
        {
            SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr,
                    SIP6_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE_E,
                    entryType);
        }

        /* Get the IPv6 Data part in case of entry is valid and type is IPv6 */
        if(entryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E)
        {
            isIpv6 = GT_TRUE;
            rc = prvCpssDxChPortGroupReadTableEntry(devNum,
                    0,                      /*portGroupId - 0, indicates 1st PP's 1st tile*/
                    CPSS_DXCH_TABLE_FDB_E,
                    hwIndex+1,              /* IPv6 Data Part, next HwIndex */
                    hwDataExtArr);
            if(rc != GT_OK)
            {
                return rc;
            }

            if(hwFormatType == PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP4_E)
            {
                hwValue = U32_GET_FIELD_MAC(hwDataArr[0],0,1);
            }
            else
            {
                SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataExtArr, SIP6_FDB_FDB_TABLE_FIELDS_VALID_E, hwValue);
            }
            if(hwValue == 1)
            {
                *isValidPtr = GT_TRUE;
            }
            else
            {
                fdbManagerPtr->apiHASyncStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_HA_SYNC_API_STATISTICS_TOTAL_IPV6_DATA_INVALID_ENTRY_E]+=1;
            }
        }
        else if(entryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E)
        {
            /* Skip the FDB IPv6 data part, as its already processed while reading key part. */
            return GT_OK;
        }

        if(entryType == PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E)
        {
            if(hwFormatType == PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP4_E)
            {
                hwValue = U32_GET_FIELD_MAC(hwDataArr[3],2,1);
            }
            else
            {
                SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(hwDataArr, SIP6_FDB_FDB_TABLE_FIELDS_SP_UNKNOWN_E, hwValue);
            }
            if(hwValue == 0)
            {
                *isValidPtr                     = GT_TRUE;
                addParamsPtr->tempEntryOffset   = tempBankId;
                addParamsPtr->tempEntryExist    = GT_TRUE;
            }
            else
            {
                fdbManagerPtr->apiHASyncStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_HA_SYNC_API_STATISTICS_TOTAL_SP_ENTRY_E]+=1;
            }
        }
        else
        {
            /* Non-MAC entry already HW index is decided */
            *isValidPtr                     = GT_TRUE;
            addParamsPtr->tempEntryOffset   = tempBankId;
            addParamsPtr->tempEntryExist    = GT_FALSE;
        }
    }

    if(*isValidPtr == GT_FALSE)
    {
        fdbManagerPtr->apiHASyncStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_HA_SYNC_API_STATISTICS_TOTAL_INVALID_E]+=1;
        /*** call flush entry for invalid/SP entries ***/
        rc = prvCpssDxChFdbManagerHwFlushByHwIndex(fdbManagerPtr,
                hwIndex,
                tempBankId  /* BankId */,
                GT_FALSE    /* Do not update counter - in case of invalid/SP entry */);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Flush the data part also in case of IPV6 */
        if(isIpv6)
        {
            rc = prvCpssDxChFdbManagerHwFlushByHwIndex(fdbManagerPtr,
                    hwIndex+1,
                    tempBankId  /* BankId */,
                    GT_FALSE    /* Do not update counter - in case of invalid/SP entry */);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChFdbManagerHaEntriesSync function
* @endinternal
*
* @brief The function populates DB according to the HW entries
*
* @param[in] fdbManagerPtr        - (pointer to) the FDB Manager.
*
* @retval GT_OK                   - on success
* @retval GT_FAIL                 - on error
* @retval GT_BAD_PARAM            - on wrong FDB manager ID.
* @retval GT_NOT_INITIALIZED      - if the FDB Manager is not initialized.
*
*/
static GT_STATUS prvCpssDxChFdbManagerHaEntriesSync
(
    IN PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr
)
{
    GT_STATUS                                               rc = GT_OK;
    GT_U32                                                  hwIndex;
    GT_U8                                                   devNum;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                     entry;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC          addParams;
    GT_BOOL                                                 isValid;                       /* Indicates Valid non-SP entry */
    PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS_ENT  addApiStatistics;
    PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_ENT        hwFormatType;

    rc = prvCpssDxChFdbManagerDbDeviceIteratorGetNext(fdbManagerPtr, GT_TRUE, &devNum);
    if(rc != GT_OK)
    {
        /* In case of GT_NO_MORE also return failure to indicate HA sync expecting atleast one device */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(&addParams, 0, sizeof(addParams));
    /* No need to enable cuckoo, in case of HA as all the entries location is already known */
    addParams.rehashEnable          = GT_FALSE;

    rc = prvCpssDxChFdbManagerDeviceHwFormatGet(fdbManagerPtr, devNum, &hwFormatType);
    if(rc != GT_OK)
    {
        return rc;
    }

    for(hwIndex = 0; hwIndex<fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes; hwIndex++)
    {
        /* Read FDB entry from */
        LOCK_DEV_NUM(devNum);
        switch(hwFormatType)
        {
            case PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP5_E:
            case PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP5_20_E:
                rc = prvCpssDxChFdbManagerHaReadEntry_sip5(devNum, hwIndex, fdbManagerPtr, &addParams, &isValid);
                break;
            case PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP6_E:
            case PRV_CPSS_DXCH_BRG_FDB_MANAGER_HW_FORMAT_TYPE_SIP4_E:
                rc = prvCpssDxChFdbManagerHwEntryRead(devNum, hwIndex, fdbManagerPtr, hwFormatType, &addParams, &isValid);
                break;
            default:
                UNLOCK_DEV_NUM(devNum);
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(hwFormatType);
        }
        UNLOCK_DEV_NUM(devNum);
        if(rc != GT_OK)
        {
            return rc;
        }

        /*** Call Add entry for each valid non-SP entry ***/
        if(isValid)
        {
            fdbManagerPtr->apiHASyncStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_HA_SYNC_API_STATISTICS_TOTAL_VALID_E]+=1;
            rc = prvCpssDxChFdbManagerDbBuildDbFormatFromHwEntry(fdbManagerPtr, hwFormatType, &entry);
            if(rc != GT_OK)
            {
                return rc;
            }

            /* call the main logic */
            rc = mainLogicEntryAdd(fdbManagerPtr, &entry, &addParams, hwIndex, GT_TRUE, GT_FALSE, &addApiStatistics);

            if(addApiStatistics <
                    PRV_CPSS_DXCH_FDB_MANAGER_ENTRY_ADD_API_STATISTICS___LAST___E)
            {
                /* increment the 'API statistics' */
                fdbManagerPtr->apiEntryAddStatisticsArr[addApiStatistics]++;
            }
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }
    rc = prvCpssDxChFdbManagerThresholdSet(fdbManagerPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(IS_FDB_MANAGER_FOR_SIP_4(fdbManagerPtr))
    {
        /* Bank counter not supported for AC5 */
        return rc;
    }
    else
    {
        return prvCpssDxChFdbManagerHwCounterSyncUpWithDb(fdbManagerPtr);
    }
}

/**
* @internal internal_cpssDxChBrgFdbManagerHighAvailabilityEntriesSync function
* @endinternal
*
* @brief   The function updates FDB Manager DB according to HW states.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - The FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong fdbManagerId data
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS internal_cpssDxChBrgFdbManagerHighAvailabilityEntriesSync
(
    IN  GT_U32 fdbManagerId
)
{
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr;
    CPSS_SYSTEM_RECOVERY_INFO_STC                           oldSystemRecoveryInfo,newSystemRecoveryInfo;
    GT_STATUS                                               rc;
    GT_BOOL                                                 saveManagerHwWriteBlock;

    FDB_MANAGER_ID_CHECK(fdbManagerId);
    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    /* save current recovery state & and make it complete - to enable HW write temporally */
    rc = cpssSystemRecoveryStateGet(&oldSystemRecoveryInfo);
    if (rc != GT_OK)
    {
        return rc;
    }
    newSystemRecoveryInfo = oldSystemRecoveryInfo;
    saveManagerHwWriteBlock = PRV_SYSTEM_RECOVERY_MANAGER_STATUS_MAC(CPSS_SYSTEM_RECOVERY_FDB_MANAGER_E).recoveryStarted;

    newSystemRecoveryInfo.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
    prvCpssSystemRecoveryStateUpdate(&newSystemRecoveryInfo);
    PRV_SYSTEM_RECOVERY_MANAGER_STATUS_MAC(CPSS_SYSTEM_RECOVERY_FDB_MANAGER_E).recoveryStarted = GT_FALSE;

    rc = prvCpssDxChFdbManagerHaEntriesSync(fdbManagerPtr);

    /* restore back the HA recovery state */
    prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
    PRV_SYSTEM_RECOVERY_MANAGER_STATUS_MAC(CPSS_SYSTEM_RECOVERY_FDB_MANAGER_E).recoveryStarted = saveManagerHwWriteBlock;

    return rc;
}
/**
* @internal cpssDxChBrgFdbManagerHighAvailabilityEntriesSync function
* @endinternal
*
* @brief   The function updates FDB Manager DB according to HW states.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - The FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong fdbManagerId data
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChBrgFdbManagerHighAvailabilityEntriesSync
(
    IN  GT_U32 fdbManagerId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbManagerHighAvailabilityEntriesSync);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, fdbManagerId));

    rc = internal_cpssDxChBrgFdbManagerHighAvailabilityEntriesSync(fdbManagerId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, fdbManagerId));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}

/**
* @internal mainLogicEntriesRewrite function
* @endinternal
*
* @brief   The function rewrites FDB Manager HW entries according to SW DB.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerPtr         - (pointer to) the FDB Manager.
* @param[in] entriesIndexesArray   - The array of HW index.
* @param[in] entriesIndexesNum     - number of HW indexes present in index array.
* @param[out] rewriteApiStatisticsPtr - (pointer to) the type of API ok/error statistics.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong fdbManagerId data
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS mainLogicEntriesRewrite
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC                  *fdbManagerPtr,
    IN  GT_U32                                                          entriesIndexesArray[],
    IN  GT_U32                                                          entriesIndexesNum,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_ENTRIES_REWRITE_API_STATISTICS_ENT    *rewriteApiStatisticsPtr
)
{
    GT_STATUS                                                rc;
    GT_U32                                                   hwIndex;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC         calcInfo;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INDEX_STC                  *dbIndexPtr;
    GT_U32                                                   indexNum;

    *rewriteApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRIES_REWRITE_API_STATISTICS_ERROR_INPUT_INVALID_E;
    CPSS_NULL_PTR_CHECK_MAC(entriesIndexesArray);

    /* For each incoming hwIndex:
     *    - Fill the calcInfo with
     *        bankstep, selectedBankId, entryPtr, entryExtPtr
     *    - Set ageBit = 1 (As refreshed)
     *    - Get key & data entry in case IPv6 entry type
     *    - Write entry to each tile of each device
     */
    for(indexNum=0; indexNum<entriesIndexesNum; indexNum++)
    {
        hwIndex = entriesIndexesArray[indexNum];
        FDB_MANAGER_HW_INDEX_CHECK(hwIndex, fdbManagerPtr);
        if(fdbManagerPtr->indexArr[hwIndex].isValid_entryPointer)
        {
            dbIndexPtr              = &fdbManagerPtr->indexArr[hwIndex];
            calcInfo.dbEntryPtr     = &fdbManagerPtr->entryPoolPtr[dbIndexPtr->entryPointer];
            switch(calcInfo.dbEntryPtr->hwFdbEntryType)
            {
                case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_MAC_ADDR_E:
                    calcInfo.dbEntryPtr->specificFormat.prvMacEntryFormat.age = 1;
                    calcInfo.bankStep       = 1;
                    break;
                case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_MCAST_E:
                    calcInfo.dbEntryPtr->specificFormat.prvIpv4McEntryFormat.age = 1;
                    calcInfo.bankStep       = 1;
                    break;
                case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_MCAST_E:
                    calcInfo.dbEntryPtr->specificFormat.prvIpv6McEntryFormat.age = 1;
                    calcInfo.bankStep       = 1;
                    break;
                case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV4_UC_E:
                    calcInfo.dbEntryPtr->specificFormat.prvIpv4UcEntryFormat.ipUcCommonInfo.age = 1;
                    calcInfo.bankStep       = 1;
                    break;

                case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_KEY_ENTRY_E:
                    /* In case of IPv6 key entry corrupted, Get the data entry from next hwIndex */
                    dbIndexPtr              = &fdbManagerPtr->indexArr[hwIndex + 1];
                    calcInfo.dbEntryExtPtr  = &fdbManagerPtr->entryPoolPtr[dbIndexPtr->entryPointer];
                    calcInfo.dbEntryPtr->specificFormat.prvIpv6UcKeyEntryFormat.age                     = 1;
                    calcInfo.dbEntryExtPtr->specificFormat.prvIpv6UcDataEntryFormat.ipUcCommonInfo.age  = 1;
                    calcInfo.bankStep       = 2;
                    break;
                case PRV_CPSS_DXCH_FDB_MANAGER_HW_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E:
                    /* In case of IPv6 Data entry corrupted, Get the key entry from previous hwIndex */
                    calcInfo.dbEntryExtPtr  = calcInfo.dbEntryPtr;
                    hwIndex                 -= 1;
                    dbIndexPtr              = &fdbManagerPtr->indexArr[hwIndex];
                    calcInfo.dbEntryPtr     = &fdbManagerPtr->entryPoolPtr[dbIndexPtr->entryPointer];
                    calcInfo.dbEntryPtr->specificFormat.prvIpv6UcKeyEntryFormat.age                     = 1;
                    calcInfo.dbEntryExtPtr->specificFormat.prvIpv6UcDataEntryFormat.ipUcCommonInfo.age  = 1;
                    calcInfo.bankStep       = 2;
                    break;

                default:
                    CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(calcInfo.dbEntryPtr->hwFdbEntryType);
            }

            /* Indication for not to update the counter */
            calcInfo.calcEntryType  = PRV_CPSS_DXCH_FDB_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_EXISTS_E;
            calcInfo.selectedBankId = hwIndex % fdbManagerPtr->numOfBanks;

            /***************************************************************************/
            /* write the entry to HW of all registered devices                         */
            /***************************************************************************/
            rc = prvCpssDxChFdbManagerHwWriteByHwIndex(fdbManagerPtr, &calcInfo);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
        else
        {
            /********************************************************************/
            /* flush the entry from HW of all registered devices                */
            /********************************************************************/
            rc = prvCpssDxChFdbManagerHwFlushByHwIndex(fdbManagerPtr,
                    hwIndex,
                    hwIndex % fdbManagerPtr->numOfBanks, /* bankId */
                    GT_FALSE);  /* do not update the bank counters in HW */
            if(rc != GT_OK)
            {
                return rc;
            }
        }
        fdbManagerPtr->apiEntriesRewriteStatisticsArr[PRV_CPSS_DXCH_FDB_MANAGER_ENTRIES_REWRITE_API_STATISTICS_TOTAL_REWRITE_E]+=1;
    }
    *rewriteApiStatisticsPtr = PRV_CPSS_DXCH_FDB_MANAGER_ENTRIES_REWRITE_API_STATISTICS_OK_E;
    return GT_OK;
}

/**
* @internal internal_cpssDxChBrgFdbManagerEntryRewrite function
* @endinternal
*
* @brief   The function rewrites FDB Manager HW entries according to SW DB.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - The FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entriesIndexesArray   - The array of HW index.
* @param[in] entriesIndexesNum     - number of HW indexes present in index array.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong fdbManagerId data
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChBrgFdbManagerEntryRewrite
(
    IN  GT_U32      fdbManagerId,
    IN  GT_U32      entriesIndexesArray[],
    IN  GT_U32      entriesIndexesNum
)
{
    GT_STATUS                                                       rc;
    PRV_CPSS_DXCH_FDB_MANAGER_ENTRIES_REWRITE_API_STATISTICS_ENT    rewriteApiStatistics;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC                  *fdbManagerPtr;

    FDB_MANAGER_ID_CHECK(fdbManagerId); /* must be done here because used in 'API' statistics */

    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    /* call the main logic */
    rc = mainLogicEntriesRewrite(fdbManagerPtr, entriesIndexesArray, entriesIndexesNum, &rewriteApiStatistics);

    if(rewriteApiStatistics <
        PRV_CPSS_DXCH_FDB_MANAGER_ENTRIES_REWRITE_API_STATISTICS___LAST___E)
    {
        /* increment the 'API statistics' */
        fdbManagerPtr->apiEntriesRewriteStatisticsArr[rewriteApiStatistics]++;
    }
    return rc;
}

/**
* @internal cpssDxChBrgFdbManagerEntryRewrite function
* @endinternal
*
* @brief   The function rewrites FDB Manager HW entries according to SW DB.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - The FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entriesIndexesArray   - The array of HW index.
* @param[in] entriesIndexesNum     - number of HW indexes present in index array.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong fdbManagerId data
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChBrgFdbManagerEntryRewrite
(
    IN  GT_U32 fdbManagerId,
    IN  GT_U32 entriesIndexesArray[], /* arrSizeVarName=entriesIndexesNum */
    IN  GT_U32 entriesIndexesNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbManagerEntryRewrite);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, fdbManagerId, entriesIndexesArray, entriesIndexesNum));

    rc = internal_cpssDxChBrgFdbManagerEntryRewrite(fdbManagerId, entriesIndexesArray, entriesIndexesNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, fdbManagerId, entriesIndexesArray, entriesIndexesNum));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChBrgFdbManagerLimitSet function
* @endinternal
*
* @brief   The function configures the maximal allowed dynamic UC MAC entries to
*           a specific limit info.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                - device number
* @param[in] limitInfoPtr          - (pointer to) limit info : global/fid/trunk/global-eport/port and the 'index'
* @param[out] limitCounterPtrPtr   - (pointer to) (pointer to) the DB that hold the {limitThreshold,actualCounter}
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or other param in limitInfoPtr.
* @retval GT_NOT_SUPPORTED         - the manager was created without support for 'learn limits'
*                                    or on global eport case : The manager for SHADOW_TYPE_SIP4 hold no 'global eport' support.
*                                    or on global eport case : The manager was created 'global eport' support.
* @retval GT_NO_SUCH               - hwDevNum not exists (no device that added by 'dev list add' hold this hwDevNum)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS limitCounterGet
(
    IN GT_U32  fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_STC       *limitInfoPtr,
    OUT PRV_CPSS_DXCH_FDB_MANAGER_DB_DYNAMIC_UC_MAC_LIMIT_STC   **limitCounterPtrPtr
)
{
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr;
    GT_U32      index;
    GT_HW_DEV_NUM   hwDevNum;

    *limitCounterPtrPtr = NULL;

    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);
    if(NULL == fdbManagerPtr->dynamicUcMacLimitPtr)
    {
        /* the pointer was not allocated because the manager was created with :
            capacityInfo.enableLearningLimits == GT_FALSE
        */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,
            "The manager was created without support for 'learning limits' (<enableLearningLimits>)");
    }

    switch(limitInfoPtr->limitType)
    {
        case CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_GLOBAL_E       :
            *limitCounterPtrPtr = &fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_global;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_FID_E          :
            index = limitInfoPtr->limitIndex.fid;
            *limitCounterPtrPtr = &fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_fid[index];
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_TRUNK_E        :
            index = limitInfoPtr->limitIndex.trunkId;
            *limitCounterPtrPtr = &fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_trunkId[index];
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_GLOBAL_EPORT_E :
            if(IS_FDB_MANAGER_FOR_SIP_4(fdbManagerPtr))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,
                    "The manager for SHADOW_TYPE_SIP4 hold no 'global eport' support ");
            }
            else
            if(NULL == fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_globalEportPtr)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,
                    "The manager was created 'global eport' support ");
            }

            index  = limitInfoPtr->limitIndex.globalEport;
            index -= fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_globalEport_base;
            *limitCounterPtrPtr = &fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_globalEportPtr[index];
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E         :
            index = limitInfoPtr->limitIndex.devPort.portNum;
            hwDevNum = limitInfoPtr->limitIndex.devPort.hwDevNum;

            if(hwDevNum >= PRV_CPSS_DXCH_FDB_MANAGER_DB_MAX_HW_DEV_NUM_CNS)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                    "hwDevNum [%d] >= [%d] (out of range)",
                    hwDevNum,
                    PRV_CPSS_DXCH_FDB_MANAGER_DB_MAX_HW_DEV_NUM_CNS);
            }
            else
            if(NULL == fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_perHwDev[hwDevNum])
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH,
                    "hwDevNum [%d] not exists (no device that added by 'dev list add' hold this hwDevNum)",
                    hwDevNum);
            }
            else
            if(index >= fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_perHwDev[hwDevNum]->numPorts)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                    "(hwDevNum [%d]) with portNum[%d] >= [%d] (out of range)",
                    hwDevNum,index,
                    fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_perHwDev[hwDevNum]->numPorts);
            }

            *limitCounterPtrPtr = &fdbManagerPtr->dynamicUcMacLimitPtr->dynamicUcMacLimit_perHwDev[hwDevNum]->perEportPtr[index];
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(limitInfoPtr->limitType);
    }

    return GT_OK;
}
/**
* @internal limitSet function
* @endinternal
*
* @brief   The function configures the maximal allowed dynamic UC MAC entries to
*           a specific limit info.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                - device number
* @param[in] limitInfoPtr          - (pointer to) limit info : global/fid/trunk/global-eport/port and the 'index'
* @param[in] limitValue            - the limit for this limit info
* @param[out] currentValuePtr      - (pointer to) the current number of dynamic UC mac entries in FDB for the specific limit info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or other param in limitInfoPtr.
* @retval GT_NOT_SUPPORTED         - the manager was created without support for 'learn limits'
*                                    or on global eport case : The manager for SHADOW_TYPE_SIP4 hold no 'global eport' support.
*                                    or on global eport case : The manager was created 'global eport' support.
* @retval GT_NO_SUCH               - hwDevNum not exists (no device that added by 'dev list add' hold this hwDevNum)
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS mainLimitSet
(
    IN GT_U32  fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_STC   *limitInfoPtr,
    IN GT_U32   limitValue,
    OUT GT_U32  *currentValuePtr,
    OUT GT_U32  *statisticsIndexPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_DYNAMIC_UC_MAC_LIMIT_STC   *limitCounterPtr;

    *statisticsIndexPtr = PRV_CPSS_DXCH_FDB_MANAGER_DB_LIMIT_SET_API_STATISTICS_ERROR_INPUT_E;

    CPSS_NULL_PTR_CHECK_MAC(limitInfoPtr);
    CPSS_NULL_PTR_CHECK_MAC(currentValuePtr);

    rc = limitCounterGet(fdbManagerId,limitInfoPtr,&limitCounterPtr);
    if(rc != GT_OK ||
        limitCounterPtr == NULL/* should not happen*/)
    {
        return rc;
    }

    /* increment the 'statistics' of this API */
    switch(limitInfoPtr->limitType)
    {
        case CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_GLOBAL_E       :
            *statisticsIndexPtr = PRV_CPSS_DXCH_FDB_MANAGER_DB_LIMIT_SET_API_STATISTICS_OK_GLOBAL_E;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_FID_E          :
            *statisticsIndexPtr = PRV_CPSS_DXCH_FDB_MANAGER_DB_LIMIT_SET_API_STATISTICS_OK_FID_E;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_TRUNK_E        :
            *statisticsIndexPtr = PRV_CPSS_DXCH_FDB_MANAGER_DB_LIMIT_SET_API_STATISTICS_OK_TRUNK_E;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_GLOBAL_EPORT_E :
            *statisticsIndexPtr = PRV_CPSS_DXCH_FDB_MANAGER_DB_LIMIT_SET_API_STATISTICS_OK_GLOBAL_EPORT_E;
            break;
        case CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E         :
            *statisticsIndexPtr = PRV_CPSS_DXCH_FDB_MANAGER_DB_LIMIT_SET_API_STATISTICS_OK_PORT_LIMIT_E;
            break;
        default:
            break;
    }

    limitCounterPtr->limitThreshold = limitValue;
    *currentValuePtr = limitCounterPtr->actualCounter;

    return GT_OK;
}

/**
* @internal internal_cpssDxChBrgFdbManagerLimitSet function
* @endinternal
*
* @brief   The function configures the maximal allowed dynamic UC MAC entries to
*           a specific limit info.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                - device number
* @param[in] limitInfoPtr          - (pointer to) limit info : global/fid/trunk/global-eport/port and the 'index'
* @param[in] limitValue            - the limit for this limit info
* @param[out] currentValuePtr      - (pointer to) the current number of dynamic UC mac entries in FDB for the specific limit info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or other param in limitInfoPtr.
* @retval GT_NOT_SUPPORTED         - the manager was created without support for 'learn limits'
*                                    or on global eport case : The manager for SHADOW_TYPE_SIP4 hold no 'global eport' support.
*                                    or on global eport case : The manager was created 'global eport' support.
* @retval GT_NO_SUCH               - hwDevNum not exists (no device that added by 'dev list add' hold this hwDevNum)
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgFdbManagerLimitSet
(
    IN GT_U32  fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_STC   *limitInfoPtr,
    IN GT_U32   limitValue,
    OUT GT_U32  *currentValuePtr
)
{
    GT_STATUS rc;
    GT_U32 statisticsIndex;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr;

    FDB_MANAGER_ID_CHECK(fdbManagerId);
    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    rc = mainLimitSet(fdbManagerId,limitInfoPtr,limitValue,currentValuePtr,&statisticsIndex);

    fdbManagerPtr->apiLimitSetArr[statisticsIndex]++;

    return rc;
}

/**
* @internal cpssDxChBrgFdbManagerLimitSet function
* @endinternal
*
* @brief   The function configures the maximal allowed dynamic UC MAC entries to
*           a specific limit info.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                - device number
* @param[in] limitInfoPtr          - (pointer to) limit info : global/fid/trunk/global-eport/port and the 'index'
* @param[in] limitValue            - the limit for this limit info
* @param[out] currentValuePtr      - (pointer to) the current number of dynamic UC mac entries in FDB for the specific limit info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or other param in limitInfoPtr.
* @retval GT_NOT_SUPPORTED         - the manager was created without support for 'learn limits'
*                                    or on global eport case : The manager for SHADOW_TYPE_SIP4 hold no 'global eport' support.
*                                    or on global eport case : The manager was created 'global eport' support.
* @retval GT_NO_SUCH               - hwDevNum not exists (no device that added by 'dev list add' hold this hwDevNum)
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbManagerLimitSet
(
    IN GT_U32  fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_STC   *limitInfoPtr,
    IN GT_U32   limitValue,
    OUT GT_U32  *currentValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbManagerLimitSet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, fdbManagerId, limitInfoPtr, limitValue, currentValuePtr));

    rc = internal_cpssDxChBrgFdbManagerLimitSet(fdbManagerId, limitInfoPtr, limitValue, currentValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, fdbManagerId, limitInfoPtr, limitValue, currentValuePtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgFdbManagerLimitGet function
* @endinternal
*
* @brief   The function get the maximal allowed dynamic UC MAC entries on
*           a specific limit info.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                - device number
* @param[in] limitInfoPtr          - (pointer to) limit info : global/fid/trunk/global-eport/port and the 'index'
* @param[out] limitValuePtr        - (pointer to) the limit for this index of limit info
* @param[out] currentValuePtr      - (pointer to) the current number of dynamic UC mac entries in FDB for the specific limit info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or other param in limitInfoPtr.
* @retval GT_NOT_SUPPORTED         - the manager was created without support for 'learn limits'
*                                    or on global eport case : The manager for SHADOW_TYPE_SIP4 hold no 'global eport' support.
*                                    or on global eport case : The manager was created 'global eport' support.
* @retval GT_NO_SUCH               - hwDevNum not exists (no device that added by 'dev list add' hold this hwDevNum)
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChBrgFdbManagerLimitGet
(
    IN GT_U32  fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_STC   *limitInfoPtr,
    OUT GT_U32  *limitValuePtr,
    OUT GT_U32  *currentValuePtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_DYNAMIC_UC_MAC_LIMIT_STC   *limitCounterPtr;

    FDB_MANAGER_ID_CHECK(fdbManagerId);
    CPSS_NULL_PTR_CHECK_MAC(limitInfoPtr);
    CPSS_NULL_PTR_CHECK_MAC(limitValuePtr);
    CPSS_NULL_PTR_CHECK_MAC(currentValuePtr);

    rc = limitCounterGet(fdbManagerId,limitInfoPtr,&limitCounterPtr);
    if(rc != GT_OK ||
        limitCounterPtr == NULL/* should not happen*/)
    {
        return rc;
    }

    *limitValuePtr   = limitCounterPtr->limitThreshold;
    *currentValuePtr = limitCounterPtr->actualCounter;

    return GT_OK;
}

/**
* @internal cpssDxChBrgFdbManagerLimitGet function
* @endinternal
*
* @brief   The function get the maximal allowed dynamic UC MAC entries on
*           a specific limit info.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                - device number
* @param[in] limitInfoPtr          - (pointer to) limit info : global/fid/trunk/global-eport/port and the 'index'
* @param[out] limitValuePtr        - (pointer to) the limit for this index of limit info
* @param[out] currentValuePtr      - (pointer to) the current number of dynamic UC mac entries in FDB for the specific limit info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or other param in limitInfoPtr.
* @retval GT_NOT_SUPPORTED         - the manager was created without support for 'learn limits'
*                                    or on global eport case : The manager for SHADOW_TYPE_SIP4 hold no 'global eport' support.
*                                    or on global eport case : The manager was created 'global eport' support.
* @retval GT_NO_SUCH               - hwDevNum not exists (no device that added by 'dev list add' hold this hwDevNum)
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgFdbManagerLimitGet
(
    IN GT_U32  fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_STC   *limitInfoPtr,
    OUT GT_U32  *limitValuePtr,
    OUT GT_U32  *currentValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbManagerLimitGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, fdbManagerId, limitInfoPtr, limitValuePtr, currentValuePtr));

    rc = internal_cpssDxChBrgFdbManagerLimitGet(fdbManagerId, limitInfoPtr, limitValuePtr, currentValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, fdbManagerId, limitInfoPtr, limitValuePtr, currentValuePtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChBrgFdbManagerConfigUpdate function
* @endinternal
*
* @brief API for updating global FDB Manager settings which were set in FDB Manager Create initialization phase.
*        The API includes only small subset of the Create API global parameters –
*        these are attributes which does not require special handling at the
*        FDB Manager, nor it contradicts with entries already installed into HW.
*        These settings can be changed in runtime, without the need to delete and
*        recreate the manager (like in case that changing hashing parameters for
*        example).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entryAttrPtr          - (pointer to) parameters to override entryAttrPtr
*                                       that was given during 'manager create' API.
*                                     NOTE: ignored if NULL
* @param[in] learningPtr           - (pointer to) parameters to override learningPtr
*                                       that was given during 'manager create' API.
*                                     NOTE: ignored if NULL
* @param[in] agingPtr              - (pointer to) parameters to override agingPtr
*                                       that was given during 'manager create' API.
*                                     NOTE: ignored if NULL
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - when entryAttrPtr and learningPtr and agingPtr are NULL pointers.
*/
static GT_STATUS internal_cpssDxChBrgFdbManagerConfigUpdate
(
    IN  GT_U32                                                        fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_ENTRY_ATTRIBUTES_STC  *entryAttrPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_LEARNING_STC          *learningPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_AGING_STC             *agingPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr;
    GT_U8       devNum;

    FDB_MANAGER_ID_CHECK(fdbManagerId);
    if(entryAttrPtr == NULL && learningPtr == NULL && agingPtr == NULL)
    {
        /* each pointer may be ignored , but ALL NULL pointer makes no sense */
        CPSS_NULL_PTR_CHECK_MAC(entryAttrPtr);
    }

    fdbManagerPtr = MANAGER_GET_MAC(fdbManagerId);

    rc = prvCpssDxChFdbManagerDbUpdateManagerParams(fdbManagerPtr,entryAttrPtr,learningPtr,agingPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* iterate on registered devices */
    rc = prvCpssDxChFdbManagerDbDeviceIteratorGetNext(fdbManagerPtr, GT_TRUE, &devNum);
    while(rc == GT_OK)
    {
        /* update the HW */
        rc = prvCpssDxChFdbManagerHwDeviceConfigUpdate(fdbManagerPtr , devNum , entryAttrPtr,learningPtr,agingPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDxChFdbManagerDbDeviceIteratorGetNext(fdbManagerPtr, GT_FALSE, &devNum);
    }

    return GT_OK;
}

/**
* @internal cpssDxChBrgFdbManagerConfigUpdate function
* @endinternal
*
* @brief API for updating global FDB Manager settings which were set in FDB Manager Create initialization phase.
*        The API includes only small subset of the Create API global parameters –
*        these are attributes which does not require special handling at the
*        FDB Manager, nor it contradicts with entries already installed into HW.
*        These settings can be changed in runtime, without the need to delete and
*        recreate the manager (like in case that changing hashing parameters for
*        example).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entryAttrPtr          - (pointer to) parameters to override entryAttrPtr
*                                       that was given during 'manager create' API.
*                                     NOTE: ignored if NULL
* @param[in] learningPtr           - (pointer to) parameters to override learningPtr
*                                       that was given during 'manager create' API.
*                                     NOTE: ignored if NULL
* @param[in] agingPtr              - (pointer to) parameters to override agingPtr
*                                       that was given during 'manager create' API.
*                                     NOTE: ignored if NULL
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - when entryAttrPtr and learningPtr and agingPtr are NULL pointers.
*/
GT_STATUS cpssDxChBrgFdbManagerConfigUpdate
(
    IN  GT_U32                                                        fdbManagerId,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_ENTRY_ATTRIBUTES_STC  *entryAttrPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_LEARNING_STC          *learningPtr,
    IN CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_AGING_STC             *agingPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgFdbManagerConfigUpdate);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, fdbManagerId, entryAttrPtr, learningPtr, agingPtr));

    rc = internal_cpssDxChBrgFdbManagerConfigUpdate(fdbManagerId, entryAttrPtr, learningPtr, agingPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, fdbManagerId, entryAttrPtr, learningPtr, agingPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);

    return rc;
}

