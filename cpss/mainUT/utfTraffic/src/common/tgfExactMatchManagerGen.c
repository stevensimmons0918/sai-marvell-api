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
* @file tgfExactMatchManagerGen.c
*
* @brief Generic API implementation for Exact Match
*
* @version   1
********************************************************************************
*/

/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <../../cpssEnabler/mainCmd/h/cmdShell/common/cmdExtServices.h>

#include <utf/private/prvUtfHelpers.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>

#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <common/tgfExactMatchManagerGen.h>
#include <cpss/dxCh/dxChxGen/exactMatchManager/private/prvCpssDxChExactMatchManager_db.h>
#endif /*CHX_FAMILY*/
/******************************************************************************\
 *                              Macro definitions                             *
\******************************************************************************/

#ifdef CHX_FAMILY

extern GT_STATUS prvTgfConvertGenericAction2ToDxChTtiAction2
(
    IN  PRV_TGF_TTI_ACTION_2_STC     *actionPtr,
    OUT CPSS_DXCH_TTI_ACTION_STC   *dxChTtiActionPtr
);

/* convert ExactMatch client into device specific format */
#define PRV_TGF_S2D_CLIENT_CONVERT_MAC(dstClient, srClient)                    \
    do                                                                         \
    {                                                                          \
        switch (srClient)                                                      \
        {                                                                      \
            case PRV_TGF_EXACT_MATCH_CLIENT_TTI_E:                             \
                dstClient = CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E;                \
                break;                                                         \
                                                                               \
            case PRV_TGF_EXACT_MATCH_CLIENT_IPCL_0_E:                          \
                dstClient = CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_0_E;             \
                break;                                                         \
                                                                               \
            case PRV_TGF_EXACT_MATCH_CLIENT_IPCL_1_E:                          \
                dstClient = CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_1_E;             \
                break;                                                         \
                                                                               \
            case PRV_TGF_EXACT_MATCH_CLIENT_IPCL_2_E:                          \
                dstClient = CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_2_E;             \
                break;                                                         \
                                                                               \
            case PRV_TGF_EXACT_MATCH_CLIENT_EPCL_E:                            \
                dstClient = CPSS_DXCH_EXACT_MATCH_CLIENT_EPCL_E;               \
                break;                                                         \
                                                                               \
            default:                                                           \
                return GT_BAD_PARAM;                                           \
        }                                                                      \
    } while (0)


/* no initialized parameter value */
#define NOT_INIT_CNS    0xFFFFFFFF

#endif /* CHX_FAMILY */



/******************************************************************************\
 *                     Private function's implementation                      *
\******************************************************************************/

#ifdef CHX_FAMILY

/**
* @internal prvTgfConvertGenericToDxChExactMatchManagerCapacityStc
*           function
* @endinternal
*
* @brief   Convert generic Exact Match Manager capacity stc into
*          device specific Exact Match Manager capacity stc
*
* @param[in] capacityPtr       - (pointer to) Exact Match Manager capacity structure
*
* @param[out] dxChCapacityPtr  - (pointer to) DxCh Exact Match Manager capacity structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChExactMatchManagerCapacityStc
(
    IN  PRV_TGF_EXACT_MATCH_MANAGER_CAPACITY_STC      *capacityPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC    *dxChCapacityPtr
)
{
    /* reset variables */
    cpssOsMemSet((GT_VOID*) dxChCapacityPtr, 0, sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC));

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChCapacityPtr->hwCapacity), &(capacityPtr->hwCapacity), numOfHwIndexes);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChCapacityPtr->hwCapacity), &(capacityPtr->hwCapacity), numOfHashes);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChCapacityPtr, capacityPtr, maxTotalEntries);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChCapacityPtr, capacityPtr, maxEntriesPerAgingScan);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChCapacityPtr, capacityPtr, maxEntriesPerDeleteScan);

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChExactMatchManagerLookupStc
*           function
* @endinternal
*
* @brief   Convert generic Exact Match Manager lookup stc into device
*          specific Exact Match Manager lookup stc
*
* @param[in] lookupPtr       - (pointer to) Exact Match Manager lookup structure
*
* @param[out] dxChLookupPtr  - (pointer to) DxCh Exact Manager Match lookup structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChExactMatchManagerLookupStc
(
    IN  PRV_TGF_EXACT_MATCH_MANAGER_LOOKUP_STC      *lookupPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC    *dxChLookupPtr
)
{
    GT_U32 i,j;
    GT_STATUS rc;

    /* reset variables */
    cpssOsMemSet((GT_VOID*) dxChLookupPtr, 0, sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC));

    for (i=0; i<PRV_TGF_EXACT_MATCH_LOOKUP_LAST_E; i++)
    {
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChLookupPtr->lookupsArray[i]), &(lookupPtr->lookupsArray[i]), lookupEnable);
        PRV_TGF_S2D_CLIENT_CONVERT_MAC(dxChLookupPtr->lookupsArray[i].lookupClient,lookupPtr->lookupsArray[i].lookupClient);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChLookupPtr->lookupsArray[i]), &(lookupPtr->lookupsArray[i]), lookupClientMappingsNum);
        switch (lookupPtr->lookupsArray[i].lookupClient)
        {
        case PRV_TGF_EXACT_MATCH_CLIENT_TTI_E:
            for (j=0; j<PRV_TGF_EXACT_MATCH_MANAGER_MAX_NUM_PROFILE_ID_MAPPING_ENTRIES_CNS; j++)
            {
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChLookupPtr->lookupsArray[i].lookupClientMappingsArray[j].ttiMappingElem),
                                               &(lookupPtr->lookupsArray[i].lookupClientMappingsArray[j].ttiMappingElem), enableExactMatchLookup);
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChLookupPtr->lookupsArray[i].lookupClientMappingsArray[j].ttiMappingElem),
                                               &(lookupPtr->lookupsArray[i].lookupClientMappingsArray[j].ttiMappingElem), keyType);
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChLookupPtr->lookupsArray[i].lookupClientMappingsArray[j].ttiMappingElem),
                                               &(lookupPtr->lookupsArray[i].lookupClientMappingsArray[j].ttiMappingElem), profileId);
            }
            break;
        case PRV_TGF_EXACT_MATCH_CLIENT_IPCL_0_E:
        case PRV_TGF_EXACT_MATCH_CLIENT_IPCL_1_E:
        case PRV_TGF_EXACT_MATCH_CLIENT_IPCL_2_E:
        case PRV_TGF_EXACT_MATCH_CLIENT_EPCL_E:
            for (j=0; j<PRV_TGF_EXACT_MATCH_MANAGER_MAX_NUM_PROFILE_ID_MAPPING_ENTRIES_CNS; j++)
            {
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChLookupPtr->lookupsArray[i].lookupClientMappingsArray[j].pclMappingElem),
                                               &(lookupPtr->lookupsArray[i].lookupClientMappingsArray[j].pclMappingElem), enableExactMatchLookup);
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChLookupPtr->lookupsArray[i].lookupClientMappingsArray[j].pclMappingElem),
                                               &(lookupPtr->lookupsArray[i].lookupClientMappingsArray[j].pclMappingElem), packetType);
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChLookupPtr->lookupsArray[i].lookupClientMappingsArray[j].pclMappingElem),
                                               &(lookupPtr->lookupsArray[i].lookupClientMappingsArray[j].pclMappingElem), profileId);
                PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChLookupPtr->lookupsArray[i].lookupClientMappingsArray[j].pclMappingElem),
                                               &(lookupPtr->lookupsArray[i].lookupClientMappingsArray[j].pclMappingElem), subProfileId);
            }
            break;
        default:
            return GT_BAD_PARAM;
        }
    }

    for (i=0; i<PRV_TGF_EXACT_MATCH_MANAGER_MAX_PROFILES_NUM_CNS; i++)
    {
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChLookupPtr->profileEntryParamsArray[i]), &(lookupPtr->profileEntryParamsArray[i]), isValidProfileId);
        prvTgfConvertGenericToDxChExactMatchKeyParams(&(lookupPtr->profileEntryParamsArray[i].keyParams),&(dxChLookupPtr->profileEntryParamsArray[i].keyParams));
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChLookupPtr->profileEntryParamsArray[i]), &(lookupPtr->profileEntryParamsArray[i]), defaultActionType);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChLookupPtr->profileEntryParamsArray[i]), &(lookupPtr->profileEntryParamsArray[i]), defaultActionEn);
        if (lookupPtr->profileEntryParamsArray[i].defaultActionEn==GT_TRUE)
        {
            switch (lookupPtr->profileEntryParamsArray[i].defaultActionType)
            {
            case PRV_TGF_EXACT_MATCH_ACTION_TTI_E:
                /* convert tti action type2 into device specific format */
                rc = prvTgfConvertGenericAction2ToDxChTtiAction2(&(lookupPtr->profileEntryParamsArray[i].defaultAction.ttiAction), &(dxChLookupPtr->profileEntryParamsArray[i].defaultAction.ttiAction));
                if (GT_OK != rc)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericAction2ToDxChTtiAction2 FAILED, rc = [%d]", rc);

                    return rc;
                }
                break;
            case PRV_TGF_EXACT_MATCH_ACTION_PCL_E:
            case PRV_TGF_EXACT_MATCH_ACTION_EPCL_E:
                rc = prvTgfConvertGenericToDxChRuleAction(&(lookupPtr->profileEntryParamsArray[i].defaultAction.pclAction), &(dxChLookupPtr->profileEntryParamsArray[i].defaultAction.pclAction));
                if (GT_OK != rc)
                {
                    PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChRuleAction FAILED, rc = [%d]", rc);

                    return rc;
                }
                break;
            default:
                return GT_BAD_PARAM;
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChExactMatchManagerExpendedActionStc
*           function
* @endinternal
*
* @brief   Convert generic Exact Match Manager Expended Action stc into
*          device specific Exact Match Manager Expended Action stc
*
* @param[in] lookupPtr       - (pointer to) Exact Match Expended Manager Action structure
*
* @param[out] dxChLookupPtr  - (pointer to) DxCh Exact Match Expended Manager Action structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChExactMatchManagerExpendedActionStc
(
    IN  PRV_TGF_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC      *expendedActionPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC    *dxChExpendedActionPtr
)
{
    GT_STATUS rc;

    /* reset variables */
    cpssOsMemSet((GT_VOID*) dxChExpendedActionPtr, 0, sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC));

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(dxChExpendedActionPtr,expendedActionPtr, exactMatchExpandedEntryValid);
    PRV_TGF_S2D_ACTION_TYPE_CONVERT_MAC(dxChExpendedActionPtr->expandedActionType, expendedActionPtr->expandedActionType);

    if (expendedActionPtr->exactMatchExpandedEntryValid==GT_TRUE)
    {
        switch (expendedActionPtr->expandedActionType)
        {
        case PRV_TGF_EXACT_MATCH_ACTION_TTI_E:
            /* convert tti action type2 into device specific format */
            rc = prvTgfConvertGenericAction2ToDxChTtiAction2(&(expendedActionPtr->expandedAction.ttiAction), &(dxChExpendedActionPtr->expandedAction.ttiAction));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericAction2ToDxChTtiAction2 FAILED, rc = [%d]", rc);

                return rc;
            }
            break;
        case PRV_TGF_EXACT_MATCH_ACTION_PCL_E:
        case PRV_TGF_EXACT_MATCH_ACTION_EPCL_E:
            rc = prvTgfConvertGenericToDxChRuleAction(&(expendedActionPtr->expandedAction.pclAction), &(dxChExpendedActionPtr->expandedAction.pclAction));
            if (GT_OK != rc)
            {
                PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChRuleAction FAILED, rc = [%d]", rc);

                return rc;
            }
            break;
        default:
            return GT_BAD_PARAM;
        }

        rc = prvTgfConvertGenericUseExpandedToDxChUseExpanded(expendedActionPtr->expandedActionType,&(expendedActionPtr->expandedActionOrigin),&(dxChExpendedActionPtr->expandedActionOrigin));
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericUseExpandedToDxChUseExpanded FAILED, rc = [%d]", rc);
        }
    }

    return GT_OK;
}

/**
* @internal prvTgfConvertGenericToDxChExactMatchManagerEntryAttrStc
*           function
* @endinternal
*
* @brief   Convert generic Exact Match Manager Entry attributes stc into
*          device specific Exact Match Manager Entry attributes stc
*
* @param[in] entryAttrPtr       - (pointer to) Exact Match Manager Entry attributes structure
*
* @param[out] dxChEntryAttrPtr  - (pointer to) DxCh Exact Match Manager Entry attributes structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*/
static GT_STATUS prvTgfConvertGenericToDxChExactMatchManagerEntryAttrStc
(
    IN  PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC      *entryAttrPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC    *dxChEntryAttrPtr
)
{
    GT_U32 i;
    GT_STATUS rc;

    /* reset variables */
    cpssOsMemSet((GT_VOID*) dxChEntryAttrPtr, 0, sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC));

    for (i=0; i<PRV_TGF_EXACT_MATCH_MANAGER_EXPANDED_ACTION_ENTRIES_NUM_CNS; i++)
    {
        rc = prvTgfConvertGenericToDxChExactMatchManagerExpendedActionStc(&(entryAttrPtr->expandedArray[i]),&(dxChEntryAttrPtr->expandedArray[i]));
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChExactMatchManagerExpendedActionStc FAILED, rc = [%d]", rc);
        }
    }
    return GT_OK;
}

#endif

/**
* @internal prvTgfExactMatchManagerCreate function
* @endinternal
*
* @brief The function creates the Exact Match Manager and its
*        databases according to input capacity structure.
*        Internally all Exact Match global parameters / modes
*        initialized to their defaults (No HW access - just SW
*        defaults that will be later applied to registered devices).
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id to associate with
*                                    the newly created manager.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] capacityPtr           - (pointer to) parameters of the HW structure and
*                                    capacity of its managed PPs Exact Matchs like number of
*                                    indexes, the amount of hashes etc. As Exact Match
*                                    Manager is created and entries may be added before
*                                    PP registered we must know in advance the relevant
*                                    capacity. In Falcon for example we must know the
*                                    shared memory allocation mode.
* @param[in] lookupPtr             - (pointer to) parameters that affect lookup of entries:
*                                    clients and their profiles
* @param[in] entryAttrPtr          - (pointer to) parameters that affect exact match entries:
*                                    expanded configuration
* @param[in] agingPtr              - (pointer to) parameters that affect refresh and aging
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_OUT_OF_CPU_MEM        - on fail to do Cpu memory allocation.
* @retval GT_ALREADY_EXIST         - if the Exact Match Manager id already exists.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfExactMatchManagerCreate
(
    IN GT_U32                                               exactMatchManagerId,
    IN PRV_TGF_EXACT_MATCH_MANAGER_CAPACITY_STC             *capacityPtr,
    IN PRV_TGF_EXACT_MATCH_MANAGER_LOOKUP_STC               *lookupPtr,
    IN PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC     *entryAttrPtr,
    IN PRV_TGF_EXACT_MATCH_MANAGER_AGING_STC                *agingPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC             dxChCapacity;
    CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC               dxChLookup;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC     dxChEntryAttr;
    CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC                dxChAging;

    /* reset variables */
    cpssOsMemSet((GT_VOID*) &dxChCapacity, 0, sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC));
    cpssOsMemSet((GT_VOID*) &dxChLookup, 0, sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC));
    cpssOsMemSet((GT_VOID*) &dxChEntryAttr, 0, sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC));
    cpssOsMemSet((GT_VOID*) &dxChAging, 0, sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC));

    rc = prvTgfConvertGenericToDxChExactMatchManagerCapacityStc(capacityPtr,&dxChCapacity);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChExactMatchManagerCapacityStc FAILED, rc = [%d]", rc);
    }

    rc = prvTgfConvertGenericToDxChExactMatchManagerLookupStc(lookupPtr,&dxChLookup);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChExactMatchManagerLookupStc FAILED, rc = [%d]", rc);
    }

    rc = prvTgfConvertGenericToDxChExactMatchManagerEntryAttrStc(entryAttrPtr,&dxChEntryAttr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChExactMatchManagerEntryAttrStc FAILED, rc = [%d]", rc);
    }

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChAging,agingPtr, agingRefreshEnable);

    rc = cpssDxChExactMatchManagerCreate(exactMatchManagerId,&dxChCapacity,&dxChLookup,&dxChEntryAttr,&dxChAging);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChExactMatchManagerCreate FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(exactMatchManagerId);
    TGF_PARAM_NOT_USED(capacityPtr);
    TGF_PARAM_NOT_USED(lookupPtr);
    TGF_PARAM_NOT_USED(entryAttrPtr);
    TGF_PARAM_NOT_USED(agingPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfExactMatchManagerDelete function
* @endinternal
*
* @brief  The function de-allocates specific Exact Match
*         Manager Instance. including all setting, entries, Data
*         bases from all attached devices - and return to
*         initial state. NOTE: the Exact Match manager will
*         remove all HW entries from all the registered devices.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                   (APPLICABLE RANGES :0..31)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvTgfExactMatchManagerDelete
(
    IN GT_U32 exactMatchManagerId
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;

    rc = cpssDxChExactMatchManagerDelete(exactMatchManagerId);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChExactMatchManagerDelete FAILED, rc = [%d]", rc);
    }
    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(exactMatchManagerId);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfExactMatchManagerDevListAdd function
* @endinternal
*
* @brief   This function adding (registering) device(s) to specific Exact Match Manager Instance.
*          It can be in initialization time or in run-time in the context of Hot-Insertion or PP reset.
*          Note: the assumption is that in all use cases (Init / Reset / Hot-Insertion) PP Exact Match
*          is flushed and don't have any old other entries. The only exception is the HA use-case
*          (which will be handled by dedicated procedure).
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] pairListArr[]         - the array of pairs (device ids + portGroupBmp)
*                                    to add to the Exact Match Manager.
* @param[in] numOfPairs            - the number of pairs in the array.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters , duplicated devices.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_ALREADY_EXIST         - if device already exist in DB
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in pairListArr
*
* @note
*   NONE
*
*/
GT_STATUS prvTgfExactMatchManagerDevListAdd
(
    IN GT_U32                                                   exactMatchManagerId,
    IN PRV_TGF_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC      pairListArr[],/*arrSizeVarName=numOfPairs*/
    IN GT_U32                                                   numOfPairs
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    GT_U32 i;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC  *dxChPairListArr;

    dxChPairListArr = cpssOsMalloc(numOfPairs*sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC));

    /* reset variables */
    cpssOsMemSet((GT_VOID*) dxChPairListArr, 0, numOfPairs*sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC));

    for (i=0; i<numOfPairs; i++)
    {
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChPairListArr[i]),&(pairListArr[i]), devNum);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChPairListArr[i]),&(pairListArr[i]), portGroupsBmp);
    }

    rc = cpssDxChExactMatchManagerDevListAdd(exactMatchManagerId,dxChPairListArr,numOfPairs);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChExactMatchManagerDevListAdd FAILED, rc = [%d]", rc);
    }

    /* free allocated memory */
    cpssOsFree(dxChPairListArr);
    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(exactMatchManagerId);
    TGF_PARAM_NOT_USED(pairListArr);
    TGF_PARAM_NOT_USED(numOfPairs);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfExactMatchManagerDevListRemove function
* @endinternal
*
* @brief   This function removing (unregistering) device(s) from
*          specific Exact Match Manager Instance (in Hot Removal
*          and reset etc..). NOTE: the Exact Match manager will
*          stop accessing to those devices (and will not access
*          to it during current function)
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] pairListArr[]         - the array of pairs (device ids + portGroupBmp)
*                                    to remove from the Exact Match Manager.
* @param[in] numOfPairs            - the number of pairs in the array.
*
* @retval GT_OK                    - on success
* @retval GT_NO_SUCH               - if one of the devices not in DB.
* @retval GT_BAD_PARAM             - on wrong parameters , duplicated devices.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*
*/
GT_STATUS prvTgfExactMatchManagerDevListRemove
(
    IN GT_U32                                                   exactMatchManagerId,
    IN PRV_TGF_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC      pairListArr[],/*arrSizeVarName=numOfPairs*/
    IN GT_U32                                                   numOfPairs
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    GT_U32 i;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC  *dxChPairListArr;

    dxChPairListArr = cpssOsMalloc(numOfPairs*sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC));

    /* reset variables */
    cpssOsMemSet((GT_VOID*) dxChPairListArr, 0, numOfPairs*sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC));

    for (i=0; i<numOfPairs; i++)
    {
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChPairListArr[i]),&(pairListArr[i]), devNum);
        PRV_TGF_STC_S2D_FIELD_COPY_MAC(&(dxChPairListArr[i]),&(pairListArr[i]), portGroupsBmp);
    }

    rc = cpssDxChExactMatchManagerDevListRemove(exactMatchManagerId,dxChPairListArr,numOfPairs);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChExactMatchManagerDevListRemove FAILED, rc = [%d]", rc);
    }

    /* free allocated memory */
    cpssOsFree(dxChPairListArr);
    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(exactMatchManagerId);
    TGF_PARAM_NOT_USED(pairListArr);
    TGF_PARAM_NOT_USED(numOfPairs);
    return GT_BAD_STATE;
#endif

}

/**
* @internal prvTgfExactMatchManagerEntryAdd function
* @endinternal
*
* @brief   This function adding entry to CPSS Exact Match Manager's database and HW.
*          All types of entries can be added using this API.
*          NOTE: the Exact Match manager will set the info to all the registered devices.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entryPtr              - (pointer to) Exact Match entry format to be added.
* @param[in] paramsPtr             - (pointer to) extra info related to 'add entry' operation.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_ALREADY_EXIST         - the entry already exists.
* @retval GT_FULL                  - the table is FULL.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*
*/
GT_STATUS prvTgfExactMatchManagerEntryAdd
(
    IN GT_U32                                               exactMatchManagerId,
    IN PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_STC                *entryPtr,
    IN PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC     *paramsPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                dxChEntry;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC     dxChParams;

    /* reset variables */
    cpssOsMemSet((GT_VOID*) &dxChEntry, 0, sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC));
    cpssOsMemSet((GT_VOID*) &dxChParams, 0, sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC));

    rc = prvTgfConvertGenericExactMatchEntryToDxChExactMatchEntry(&(entryPtr->exactMatchEntry),&dxChEntry.exactMatchEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericExactMatchEntryToDxChExactMatchEntry FAILED, rc = [%d]", rc);
    }

    PRV_TGF_S2D_ACTION_TYPE_CONVERT_MAC(dxChEntry.exactMatchActionType, entryPtr->exactMatchActionType);

    switch(entryPtr->exactMatchActionType)
    {
    case PRV_TGF_EXACT_MATCH_ACTION_TTI_E:
        /* convert tti action type2 into device specific format */
        rc = prvTgfConvertGenericAction2ToDxChTtiAction2(&(entryPtr->exactMatchAction.ttiAction), &dxChEntry.exactMatchAction.ttiAction);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericAction2ToDxChTtiAction2 FAILED, rc = [%d]", rc);

            return rc;
        }
        break;
    case PRV_TGF_EXACT_MATCH_ACTION_PCL_E:
    case PRV_TGF_EXACT_MATCH_ACTION_EPCL_E:
        rc = prvTgfConvertGenericToDxChRuleAction(&(entryPtr->exactMatchAction.pclAction), &dxChEntry.exactMatchAction.pclAction);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChRuleAction FAILED, rc = [%d]", rc);

            return rc;
        }
        break;
    default:
        return GT_BAD_PARAM;
    }

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChEntry ,entryPtr, expandedActionIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChEntry ,entryPtr, exactMatchUserDefined);

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChParams ,paramsPtr, rehashEnable);

    rc = cpssDxChExactMatchManagerEntryAdd(exactMatchManagerId,&dxChEntry,&dxChParams);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChExactMatchManagerEntryAdd FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(exactMatchManagerId);
    TGF_PARAM_NOT_USED(entryPtr);
    TGF_PARAM_NOT_USED(paramsPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfExactMatchManagerEntryUpdate function
* @endinternal
*
* @brief   The function updates existing entry in CPSS Exact Match Manager's database & HW.
*          All types of entries can be updated using this API.
*          The fuction only update the action of the entry.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entryPtr              - (pointer to) Exact Match entry format with full info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager is not initialized
* @retval GT_NOT_ALLOWED           - if trying to overwrite static entry with dynamic one
*
* @note
*   NONE
*
*/
GT_STATUS prvTgfExactMatchManagerEntryUpdate
(
    IN GT_U32                                                exactMatchManagerId,
    IN PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_STC                 *entryPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                dxChEntry;

    /* reset variables */
    cpssOsMemSet((GT_VOID*) &dxChEntry, 0, sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC));

    rc = prvTgfConvertGenericExactMatchEntryToDxChExactMatchEntry(&(entryPtr->exactMatchEntry),&dxChEntry.exactMatchEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericExactMatchEntryToDxChExactMatchEntry FAILED, rc = [%d]", rc);
    }

    PRV_TGF_S2D_ACTION_TYPE_CONVERT_MAC(dxChEntry.exactMatchActionType, entryPtr->exactMatchActionType);

    switch(entryPtr->exactMatchActionType)
    {
    case PRV_TGF_EXACT_MATCH_ACTION_TTI_E:
        /* convert tti action type2 into device specific format */
        rc = prvTgfConvertGenericAction2ToDxChTtiAction2(&(entryPtr->exactMatchAction.ttiAction), &dxChEntry.exactMatchAction.ttiAction);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericAction2ToDxChTtiAction2 FAILED, rc = [%d]", rc);

            return rc;
        }
        break;
    case PRV_TGF_EXACT_MATCH_ACTION_PCL_E:
    case PRV_TGF_EXACT_MATCH_ACTION_EPCL_E:
        rc = prvTgfConvertGenericToDxChRuleAction(&(entryPtr->exactMatchAction.pclAction), &dxChEntry.exactMatchAction.pclAction);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChRuleAction FAILED, rc = [%d]", rc);

            return rc;
        }
        break;
    default:
        return GT_BAD_PARAM;
    }

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChEntry ,entryPtr, expandedActionIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChEntry ,entryPtr, exactMatchUserDefined);

    rc = cpssDxChExactMatchManagerEntryUpdate(exactMatchManagerId,&dxChEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChExactMatchManagerEntryUpdate FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(exactMatchManagerId);
    TGF_PARAM_NOT_USED(entryPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfExactMatchManagerEntryDelete function
* @endinternal
*
* @brief   This function delete entry from the CPSS Exact Match Manager's database and HW.
*          All types of entries can be deleted using this API.
*          NOTE: the Exact Match manager will remove the info from all the registered devices.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entryPtr              - (pointer to) Exact Match entry format to be deleted.
*                                     NOTE: only the 'exactMatchEntryKey' part is used by the API.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_FOUND             - the entry was not found.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*
*/
GT_STATUS prvTgfExactMatchManagerEntryDelete
(
    IN GT_U32                                         exactMatchManagerId,
    IN PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_STC          *entryPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                dxChEntry;

    /* reset variables */
    cpssOsMemSet((GT_VOID*) &dxChEntry, 0, sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC));

    rc = prvTgfConvertGenericExactMatchEntryToDxChExactMatchEntry(&(entryPtr->exactMatchEntry),&dxChEntry.exactMatchEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericExactMatchEntryToDxChExactMatchEntry FAILED, rc = [%d]", rc);
    }

    PRV_TGF_S2D_ACTION_TYPE_CONVERT_MAC(dxChEntry.exactMatchActionType, entryPtr->exactMatchActionType);

    switch(entryPtr->exactMatchActionType)
    {
    case PRV_TGF_EXACT_MATCH_ACTION_TTI_E:
        /* convert tti action type2 into device specific format */
        rc = prvTgfConvertGenericAction2ToDxChTtiAction2(&(entryPtr->exactMatchAction.ttiAction), &dxChEntry.exactMatchAction.ttiAction);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericAction2ToDxChTtiAction2 FAILED, rc = [%d]", rc);

            return rc;
        }
        break;
    case PRV_TGF_EXACT_MATCH_ACTION_PCL_E:
    case PRV_TGF_EXACT_MATCH_ACTION_EPCL_E:
        rc = prvTgfConvertGenericToDxChRuleAction(&(entryPtr->exactMatchAction.pclAction), &dxChEntry.exactMatchAction.pclAction);
        if (GT_OK != rc)
        {
            PRV_UTF_LOG1_MAC("[TGF]: prvTgfConvertGenericToDxChRuleAction FAILED, rc = [%d]", rc);

            return rc;
        }
        break;
    default:
        return GT_BAD_PARAM;
    }

    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChEntry ,entryPtr, expandedActionIndex);
    PRV_TGF_STC_S2D_FIELD_COPY_MAC(&dxChEntry ,entryPtr, exactMatchUserDefined);

    rc = cpssDxChExactMatchManagerEntryDelete(exactMatchManagerId,&dxChEntry);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChExactMatchManagerEntryDelete FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(exactMatchManagerId);
    TGF_PARAM_NOT_USED(entryPtr);
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfExactMatchManagerCounterVerify function
* @endinternal
*
* @brief   This function compare the counters from the CPSS Exact Match Manager's database
*          and an expected counter list.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] expCounters           - (pointer to) expected counters
* @param[in] checkBankCounters     - GT_TRUE: check the bank counters
*                                    GT_FALSE: do not check ban counters.
*                                    this is used in case the additions
*                                    of the entries are random, and bankCounters
*                                    are not updated in the test
*
*
* @retval
*   NONE
*
* @note
*   NONE
*
*/
GT_VOID prvTgfExactMatchManagerCounterVerify
(
    IN GT_U32                                       exactMatchManagerId,
    IN PRV_TGF_EXACT_MATCH_MANAGER_COUNTERS_STC    *expCounters,
    IN GT_BOOL                                      checkBankCounters
)
{
#ifdef CHX_FAMILY
    GT_U32                                      i;
    CPSS_DXCH_EXACT_MATCH_MANAGER_COUNTERS_STC  getCounters;
    GT_STATUS                                   rc;
    rc = cpssDxChExactMatchManagerCountersGet(exactMatchManagerId, &getCounters);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchManagerCountersGet: %d", prvTgfDevNum);

    /* Verify Counter */
    UTF_VERIFY_EQUAL0_STRING_MAC(expCounters->usedEntriesIndexes,   getCounters.usedEntriesIndexes,    "usedEntriesIndexes: counter unexpected");
    UTF_VERIFY_EQUAL0_STRING_MAC(expCounters->freeEntriesIndexes,   getCounters.freeEntriesIndexes,    "usedEntriesIndexes: counter unexpected");
    UTF_VERIFY_EQUAL0_STRING_MAC(expCounters->ttiClientKeySizeEntriesArray[0],   getCounters.ttiClientKeySizeEntriesArray[0], "ttiClient with KeySize=5B: counter unexpected");
    UTF_VERIFY_EQUAL0_STRING_MAC(expCounters->ttiClientKeySizeEntriesArray[1],   getCounters.ttiClientKeySizeEntriesArray[1], "ttiClient with KeySize=19B: counter unexpected");
    UTF_VERIFY_EQUAL0_STRING_MAC(expCounters->ttiClientKeySizeEntriesArray[2],   getCounters.ttiClientKeySizeEntriesArray[2], "ttiClient with KeySize=33B: counter unexpected");
    UTF_VERIFY_EQUAL0_STRING_MAC(expCounters->ttiClientKeySizeEntriesArray[3],   getCounters.ttiClientKeySizeEntriesArray[3], "ttiClient with KeySize=47B: counter unexpected");
    UTF_VERIFY_EQUAL0_STRING_MAC(expCounters->ipcl0ClientKeySizeEntriesArray[0],   getCounters.ipcl0ClientKeySizeEntriesArray[0], "ipcl0Client with KeySize=5B: counter unexpected");
    UTF_VERIFY_EQUAL0_STRING_MAC(expCounters->ipcl0ClientKeySizeEntriesArray[1],   getCounters.ipcl0ClientKeySizeEntriesArray[1], "ipcl0Client with KeySize=19B: counter unexpected");
    UTF_VERIFY_EQUAL0_STRING_MAC(expCounters->ipcl0ClientKeySizeEntriesArray[2],   getCounters.ipcl0ClientKeySizeEntriesArray[2], "ipcl0Client with KeySize=33B: counter unexpected");
    UTF_VERIFY_EQUAL0_STRING_MAC(expCounters->ipcl0ClientKeySizeEntriesArray[3],   getCounters.ipcl0ClientKeySizeEntriesArray[3], "ipcl0Client with KeySize=47B: counter unexpected");
    UTF_VERIFY_EQUAL0_STRING_MAC(expCounters->ipcl1ClientKeySizeEntriesArray[0],   getCounters.ipcl1ClientKeySizeEntriesArray[0], "ipcl1Client with KeySize=5B: counter unexpected");
    UTF_VERIFY_EQUAL0_STRING_MAC(expCounters->ipcl1ClientKeySizeEntriesArray[1],   getCounters.ipcl1ClientKeySizeEntriesArray[1], "ipcl1Client with KeySize=19B: counter unexpected");
    UTF_VERIFY_EQUAL0_STRING_MAC(expCounters->ipcl1ClientKeySizeEntriesArray[2],   getCounters.ipcl1ClientKeySizeEntriesArray[2], "ipcl1Client with KeySize=33B: counter unexpected");
    UTF_VERIFY_EQUAL0_STRING_MAC(expCounters->ipcl1ClientKeySizeEntriesArray[3],   getCounters.ipcl1ClientKeySizeEntriesArray[3], "ipcl1Client with KeySize=47B: counter unexpected");
    UTF_VERIFY_EQUAL0_STRING_MAC(expCounters->ipcl2ClientKeySizeEntriesArray[0],   getCounters.ipcl2ClientKeySizeEntriesArray[0], "ipcl2Client with KeySize=5B: counter unexpected");
    UTF_VERIFY_EQUAL0_STRING_MAC(expCounters->ipcl2ClientKeySizeEntriesArray[1],   getCounters.ipcl2ClientKeySizeEntriesArray[1], "ipcl2Client with KeySize=19B: counter unexpected");
    UTF_VERIFY_EQUAL0_STRING_MAC(expCounters->ipcl2ClientKeySizeEntriesArray[2],   getCounters.ipcl2ClientKeySizeEntriesArray[2], "ipcl2Client with KeySize=33B: counter unexpected");
    UTF_VERIFY_EQUAL0_STRING_MAC(expCounters->ipcl2ClientKeySizeEntriesArray[3],   getCounters.ipcl2ClientKeySizeEntriesArray[3], "ipcl2Client with KeySize=47B: counter unexpected");
    UTF_VERIFY_EQUAL0_STRING_MAC(expCounters->epclClientKeySizeEntriesArray[0],   getCounters.epclClientKeySizeEntriesArray[0], "epclClient with KeySize=5B: counter unexpected");
    UTF_VERIFY_EQUAL0_STRING_MAC(expCounters->epclClientKeySizeEntriesArray[1],   getCounters.epclClientKeySizeEntriesArray[1], "epclClient with KeySize=19B: counter unexpected");
    UTF_VERIFY_EQUAL0_STRING_MAC(expCounters->epclClientKeySizeEntriesArray[2],   getCounters.epclClientKeySizeEntriesArray[2], "epclClient with KeySize=33B: counter unexpected");
    UTF_VERIFY_EQUAL0_STRING_MAC(expCounters->epclClientKeySizeEntriesArray[3],   getCounters.epclClientKeySizeEntriesArray[3], "epclClient with KeySize=47B: counter unexpected");

    if (checkBankCounters==GT_TRUE)
    {
        for (i=0;i<CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_NUM_BANKS_CNS;i++)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(expCounters->bankCounters[i],   getCounters.bankCounters[i], "bankCounters[%d]: counter unexpected", i);
        }
    }
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(exactMatchManagerId);
    TGF_PARAM_NOT_USED(expCounters);
    return GT_BAD_STATE;
#endif

}

/**
* @internal prvTgfExactMatchManagerDatabaseCheck function
* @endinternal
*
* @brief Run -time API that can be triggered to test various aspects
*        of Exact Match Manager. Tests includes things such as
*        internal DB, sync with HW, counters correctness etc.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] checksPtr             - (Pointer to) DB check attributes
* @param[out] resultArray          - (pointer to) Test result array.
* @param[out] errorNumberPtr       - (pointer to) Number of error types filled in resultArray
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager is not initialized
*/
GT_VOID prvTgfExactMatchManagerDatabaseCheck
(
    IN  GT_U32                                                  exactMatchManagerId,
    IN  PRV_TGF_EXACT_MATCH_MANAGER_DB_CHECKS_STC               *checksPtr,
    OUT PRV_TGF_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT        resultArray[PRV_TGF_EXACT_MATCH_MANAGER_DB_CHECKS___LAST___E],
    OUT GT_U32                                                  *errorNumberPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS                                           rc;
    GT_U32                                              i;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_STC         checks;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT  dxChResultArray[CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS___LAST___E];

    checks.globalCheckEnable = checksPtr->globalCheckEnable;
    checks.countersCheckEnable = checksPtr->countersCheckEnable;
    checks.dbFreeListCheckEnable = checksPtr->dbFreeListCheckEnable;
    checks.dbUsedListCheckEnable = checksPtr->dbUsedListCheckEnable;
    checks.dbIndexPointerCheckEnable = checksPtr->dbIndexPointerCheckEnable;
    checks.dbAgingBinCheckEnable = checksPtr->dbAgingBinCheckEnable;
    checks.dbAgingBinUsageMatrixCheckEnable = checksPtr->dbAgingBinUsageMatrixCheckEnable;
    checks.hwUsedListCheckEnable = checksPtr->hwUsedListCheckEnable;
    checks.cuckooDbCheckEnable = checksPtr->cuckooDbCheckEnable;

    rc = cpssDxChExactMatchManagerDatabaseCheck(exactMatchManagerId,&checks,dxChResultArray,errorNumberPtr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchManagerDatabaseCheck: %d", prvTgfDevNum);

    /* set results */
    for (i=0;i<CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS___LAST___E;i++)
    {
        resultArray[i] = dxChResultArray[i];
    }
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(exactMatchManagerId);
    TGF_PARAM_NOT_USED(checksPtr);
    TGF_PARAM_NOT_USED(resultArray);
    TGF_PARAM_NOT_USED(errorNumberPtr);
   return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfExactMatchManagerCompletionForHa function
* @endinternal
*
* @brief  After all replay is done there is a need to clean
*         temporary database and to invaliudate HW entries that
*         are not needed anymore
*
* @retval GT_OK                    - if success, or
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device
* @retval GT_FAIL                  - otherwise.
*
*
*/
GT_STATUS prvTgfExactMatchManagerCompletionForHa
(
    GT_VOID
)
{
#ifdef CHX_FAMILY
    /* call device specific API */
    return prvCpssDxChExactMatchManagerCompletionForHa();
#else
    return GT_BAD_STATE;
#endif
}

/**
* @internal prvTgfExactMatchManagerDeleteScan function
* @endinternal
*
* @brief  The function scans the entire Exact Match (Till maximal per
*         scan allowed). Delete the filtered entries.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] exactMatchScanStart   - Indicates the starting point of the scan.
*                                    GT_TRUE  - Start from the beginning of the used list.
*                                    GT_FALSE - Start from the last scanned point.
* @param[in] paramsPtr             - the delete scan attributes.
* @param[out] entriesDeletedArray  - (pointer to) Update event array.
*                                    This is optional (can be NULL if application do not require it).
*                                    If used, should have memory for at least "max entries allowed per delete scan".
* @param[out] entriesDeletedNumberPtr - (pointer to) Number of Update events filled in array.
*                                    This is optional (can be NULL if application do not require it).
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - if no more entries to delete
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong exactMatchManagerId and paramsPtr data
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager is not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*/

GT_STATUS prvTgfExactMatchManagerDeleteScan
(
    IN  GT_U32                                                  exactMatchManagerId,
    IN  GT_BOOL                                                 exactMatchScanStart,
    IN  PRV_TGF_EXACT_MATCH_MANAGER_DELETE_SCAN_PARAMS_STC      *paramsPtr,
    OUT PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC      entriesDeletedArray[],    /*maxArraySize=1024*/
    INOUT GT_U32                                                *entriesDeletedNumberPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DELETE_SCAN_PARAMS_STC    scanParam;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC    *dxChEntriesDeletedArray;

    if (entriesDeletedNumberPtr!=NULL) /* the size of allocated entriesDeletedArray */
    {
        /* this was the first entry added */
        dxChEntriesDeletedArray = cpssOsMalloc((*entriesDeletedNumberPtr)*sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC));
        if(dxChEntriesDeletedArray==NULL)
        {
           PRV_UTF_LOG0_MAC("cpssOsMalloc: failed");
           return GT_BAD_PTR;
        }
        /* reset variables */
        cpssOsMemSet((GT_VOID*) dxChEntriesDeletedArray, 0,
                     (*entriesDeletedNumberPtr)*sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC));
    }
    else
    {
        dxChEntriesDeletedArray=NULL;/* null is a legal value for the function */
    }

    cpssOsMemCpy(&scanParam, paramsPtr, sizeof(scanParam));

    rc = cpssDxChExactMatchManagerDeleteScan(exactMatchManagerId, exactMatchScanStart, &scanParam, dxChEntriesDeletedArray, entriesDeletedNumberPtr);
    if ((GT_OK != rc)&&(GT_NO_MORE != rc))
    {
        if(dxChEntriesDeletedArray!=NULL)
            cpssOsFree(dxChEntriesDeletedArray);

        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChExactMatchManagerDeleteScan FAILED, rc = [%d]", rc);
    }

    if ((*entriesDeletedNumberPtr)>0 && (dxChEntriesDeletedArray!=NULL) && (entriesDeletedArray!=NULL))
    {
        cpssOsMemCpy(dxChEntriesDeletedArray, entriesDeletedArray, (*entriesDeletedNumberPtr) * sizeof(PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC));
    }

    if(dxChEntriesDeletedArray!=NULL)
        cpssOsFree(dxChEntriesDeletedArray);

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(exactMatchManagerId);
    TGF_PARAM_NOT_USED(exactMatchScanStart);
    TGF_PARAM_NOT_USED(paramsPtr);
    TGF_PARAM_NOT_USED(entriesDeletedArray);
    TGF_PARAM_NOT_USED(entriesDeletedNumberPtr);
   return GT_BAD_STATE;
#endif

}

/**
* @internal prvTgfExactMatchManagerAgingScan function
* @endinternal
*
* @brief   The function scans the entire Exact Match and process age-out for the filtered entries.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] paramsPtr             - the aging scan attributes.
* @param[out] entriesAgedOutArray  - (pointer to) Update event array.
*                                    This is optional (can be NULL if application do not require it).
*                                    If used, should have memory for at least "max entries allowed per aging scan".
* @param[out] entriesAgedOutNumPtr - (pointer to) Number of Update events filled in array.
*                                    This is optional (can be NULL if application do not require it).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong exactMatchManagerId and paramsPtr data
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager is not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*   NONE
*/
GT_STATUS prvTgfExactMatchManagerAgingScan
(
    IN  GT_U32                                                  exactMatchManagerId,
    IN  PRV_TGF_EXACT_MATCH_MANAGER_AGING_SCAN_PARAMS_STC       *paramsPtr,
    OUT PRV_TGF_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC      entriesAgedOutArray[],
    INOUT GT_U32                                                *entriesAgedOutNumPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    GT_U32    i;
    CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_SCAN_PARAMS_STC     scanParam;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC    *dxChEntriesdAgeoutArray;

    if (entriesAgedOutNumPtr!=NULL) /* the size of allocated entriesAgedOutArray */
    {
        /* this was the first entry added */
        dxChEntriesdAgeoutArray = cpssOsMalloc((*entriesAgedOutNumPtr)*sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC));
        if(dxChEntriesdAgeoutArray==NULL)
        {
           PRV_UTF_LOG0_MAC("cpssOsMalloc: failed");
           return GT_BAD_PTR;
        }
        /* reset variables */
        cpssOsMemSet((GT_VOID*) dxChEntriesdAgeoutArray, 0,
                     (*entriesAgedOutNumPtr)*sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC));
    }
    else
    {
        dxChEntriesdAgeoutArray=NULL; /* null is a legal value for the function */
    }

    cpssOsMemCpy(&scanParam, paramsPtr, sizeof(scanParam));

    rc = cpssDxChExactMatchManagerAgingScan(exactMatchManagerId, &scanParam, dxChEntriesdAgeoutArray, entriesAgedOutNumPtr);
    if ((GT_OK != rc)&&(GT_NO_MORE != rc))
    {
        if(dxChEntriesdAgeoutArray!=NULL)
            cpssOsFree(dxChEntriesdAgeoutArray);

        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChExactMatchManagerAgingScan FAILED, rc = [%d]", rc);
    }

    for (i=0;i<(*entriesAgedOutNumPtr);i++)
    {
       entriesAgedOutArray[i].updateType = dxChEntriesdAgeoutArray[i].updateType;
       entriesAgedOutArray[i].entry.expandedActionIndex = dxChEntriesdAgeoutArray[i].entry.expandedActionIndex;
       entriesAgedOutArray[i].entry.exactMatchUserDefined = dxChEntriesdAgeoutArray[i].entry.exactMatchUserDefined;
       cpssOsMemCpy(&entriesAgedOutArray[i].entry.exactMatchEntry,
                    &dxChEntriesdAgeoutArray[i].entry.exactMatchEntry,
                    sizeof(entriesAgedOutArray[i].entry.exactMatchEntry));
       switch(dxChEntriesdAgeoutArray[i].entry.exactMatchActionType)
       {
           case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
               entriesAgedOutArray[i].entry.exactMatchActionType=PRV_TGF_EXACT_MATCH_ACTION_TTI_E;
               cpssOsMemCpy(&entriesAgedOutArray[i].entry.exactMatchAction.ttiAction,
                            &dxChEntriesdAgeoutArray[i].entry.exactMatchAction.ttiAction,
                            sizeof(dxChEntriesdAgeoutArray[i].entry.exactMatchAction.ttiAction));
               break;
           case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
               entriesAgedOutArray[i].entry.exactMatchActionType=PRV_TGF_EXACT_MATCH_ACTION_PCL_E;
               cpssOsMemCpy(&entriesAgedOutArray[i].entry.exactMatchAction.pclAction,
                            &dxChEntriesdAgeoutArray[i].entry.exactMatchAction.pclAction,
                            sizeof(dxChEntriesdAgeoutArray[i].entry.exactMatchAction.pclAction));
               break;
           case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
               entriesAgedOutArray[i].entry.exactMatchActionType=PRV_TGF_EXACT_MATCH_ACTION_EPCL_E;
               cpssOsMemCpy(&entriesAgedOutArray[i].entry.exactMatchAction.pclAction,
                            &dxChEntriesdAgeoutArray[i].entry.exactMatchAction.pclAction,
                            sizeof(dxChEntriesdAgeoutArray[i].entry.exactMatchAction.pclAction));
               break;
            default:
               if(dxChEntriesdAgeoutArray!=NULL)
                    cpssOsFree(dxChEntriesdAgeoutArray);
               return GT_BAD_VALUE;
           }
    }
    if(dxChEntriesdAgeoutArray!=NULL)
        cpssOsFree(dxChEntriesdAgeoutArray);
    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(exactMatchManagerId);
    TGF_PARAM_NOT_USED(paramsPtr);
    TGF_PARAM_NOT_USED(entriesAgedOutArray);
    TGF_PARAM_NOT_USED(entriesAgedOutNumPtr);
   return GT_BAD_STATE;
#endif
}

/**
* @internal prcTgfExactMatchManagerStatisticsGet function
* @endinternal
*
* @brief API for fetching Exact Match Manager statistics.
*
* @param[in]exctMatchManagerId  - the Exact Match Manager id.
*                                 (APPLICABLE RANGES : 0..31)
* @param[out]statisticsPtr      - (pointer to) Exact Match Manager statistics.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager is not initialized
*/
GT_STATUS prvTgfExactMatchManagerStatisticsGet
(
    IN GT_U32                                           exactMatchManagerId,
    OUT PRV_TGF_EXACT_MATCH_MANAGER_STATISTICS_STC      *statisticsPtr
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    CPSS_DXCH_EXACT_MATCH_MANAGER_STATISTICS_STC    dxChStatistics;

    rc = cpssDxChExactMatchManagerStatisticsGet(exactMatchManagerId, &dxChStatistics);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChExactMatchManagerStatisticsGet FAILED, rc = [%d]", rc);
    }

    cpssOsMemCpy(statisticsPtr, &dxChStatistics, sizeof(dxChStatistics));

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(exactMatchManagerId);
    TGF_PARAM_NOT_USED(statisticsPtr);
   return GT_BAD_STATE;
#endif

}

/**
* @internal prvTgfExactMatchManagerStatisticsClear function
* @endinternal
*
* @brief API for clearing Exact Match Manager statistics.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager is not initialized
*/
GT_STATUS prvTgfExactMatchManagerStatisticsClear
(
    IN GT_U32 exactMatchManagerId
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;

    rc = cpssDxChExactMatchManagerStatisticsClear(exactMatchManagerId);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC("[TGF]: cpssDxChExactMatchManagerStatisticsGet FAILED, rc = [%d]", rc);
    }

    return rc;
#endif /* CHX_FAMILY */

#if (!defined CHX_FAMILY)
    TGF_PARAM_NOT_USED(exactMatchManagerId);
    return GT_BAD_STATE;
#endif
}
