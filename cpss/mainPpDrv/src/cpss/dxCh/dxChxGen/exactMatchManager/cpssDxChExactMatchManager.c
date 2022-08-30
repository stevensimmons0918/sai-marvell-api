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
* @file cpssDxChExactMatchManager.c
*
* @brief Exact Match manager support.
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/exactMatchManager/cpssDxChExactMatchManager.h>
#include <cpss/dxCh/dxChxGen/exactMatch/cpssDxChExactMatch.h>
#include <cpss/dxCh/dxChxGen/private/lpm/hw/prvCpssDxChLpmHw.h>
#include <cpss/generic/private/prvCpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/pcl/private/prvCpssDxChPcl.h>
#include <cpss/dxCh/dxChxGen/mirror/private/prvCpssDxChMirror.h>
#include <cpss/dxCh/dxChxGen/cuckoo/prvCpssDxChCuckoo.h>

#include <cpss/dxCh/dxChxGen/exactMatchManager/private/prvCpssDxChExactMatchManagerLog.h>
#include <cpss/dxCh/dxChxGen/exactMatchManager/private/prvCpssDxChExactMatchManager_utils.h>
#include <cpss/dxCh/dxChxGen/exactMatchManager/private/prvCpssDxChExactMatchManager_db.h>
#include <cpss/dxCh/dxChxGen/exactMatchManager/private/prvCpssDxChExactMatchManager_hw.h>
#include <cpss/dxCh/dxChxGen/exactMatchManager/private/prvCpssDxChExactMatchManager_debug.h>
#include <cpss/common/systemRecovery/private/prvCpssCommonSystemRecoveryParallel.h>

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* global variables macros */

#define PRV_SHARED_EMM_DIR_TTI_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.dxchEmMgrDir.emMgrSrc._var)

#define EM_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(_mask, _entryValue, _inputFilterValue)   \
    if((_mask != 0) && (((_entryValue) & (_mask)) != _inputFilterValue))                    \
    {                                                                                       \
        return GT_OK;                                                                       \
    }
#define EM_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(_entryValue, _inputFilterValue)         \
    if(_entryValue != _inputFilterValue)                                                    \
    {                                                                                       \
        return GT_OK;                                                                       \
    }

static GT_STATUS mainLogicEntryDelete
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC               *exactMatchManagerPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                              *entryPtr,
    IN GT_BOOL                                                              deleteSpecificBank,
    IN GT_U32                                                               bankId,
    OUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_DELETE_API_STATISTICS_ENT   *deleteApiStatisticsPtr
);

/**
* @internal internal_cpssDxChExactMatchManagerCreate function
* @endinternal
*
* @brief The function creates the Exact Match Manager and its
*        databases according to input capacity structure.
*        Internally all Exact Match global parameters / modes
*        initialized to their defaults (No HW access - just SW
*        defaults that will be later applied to registered devices).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
static GT_STATUS internal_cpssDxChExactMatchManagerCreate
(
    IN GT_U32                                                exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC            *capacityPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC              *lookupPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC    *entryAttrPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC               *agingPtr
)
{
    GT_STATUS rc=GT_OK;
    GT_BOOL managerAllocated=GT_FALSE;

    rc = prvCpssDxChExactMatchManagerDbCreate(exactMatchManagerId,capacityPtr,lookupPtr,entryAttrPtr,agingPtr,&managerAllocated);

    if(rc != GT_OK && managerAllocated == GT_TRUE)
    {
        /* incase of 'error' : if the manager was allocated
            we need to 'undo' the partial create. */

        /* delete the manager from the DB */
        (void)prvCpssDxChExactMatchManagerDbDelete(exactMatchManagerId);

        return rc;
    }

    if(rc != GT_OK)
    {
        return rc;
    }

    /* initialize cuckoo DB */
    rc = prvCpssDxChCuckooDbInit(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId);
    if (rc != GT_OK)
    {
        /* delete cuckoo DB */
        (void)prvCpssDxChCuckooDbDelete(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId);
    }
    return rc;
}

/**
* @internal cpssDxChExactMatchManagerCreate function
* @endinternal
*
* @brief The function creates the Exact Match Manager and its
*        databases according to input capacity structure.
*        Internally all Exact Match global parameters / modes
*        initialized to their defaults (No HW access - just SW
*        defaults that will be later applied to registered devices).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
GT_STATUS cpssDxChExactMatchManagerCreate
(
    IN GT_U32                                                exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC            *capacityPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC              *lookupPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC    *entryAttrPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC               *agingPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchManagerCreate);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, exactMatchManagerId, capacityPtr, lookupPtr, entryAttrPtr, agingPtr));

    rc = internal_cpssDxChExactMatchManagerCreate(exactMatchManagerId, capacityPtr, lookupPtr, entryAttrPtr, agingPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, exactMatchManagerId, capacityPtr, lookupPtr, entryAttrPtr, agingPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChExactMatchManagerDelete function
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
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChExactMatchManagerDelete
(
    IN GT_U32 exactMatchManagerId
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;
    GT_BOOL                       managerHwWriteBlock;

    EM_MANAGER_ID_CHECK(exactMatchManagerId);
    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    /*  in HA we should not delete from HW - since the delete read the entry first from the HW
        then we should skip this call, else we will get a fail on the read values that are not
        correct since we are in catchup stage */
    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_EXACT_MATCH_MANAGER_E);
    if (!((tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E) &&
         (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)) && managerHwWriteBlock == GT_FALSE)
    {
        /* Flush all entries from the HW of all registered devices */
        rc = prvCpssDxChExactMatchManagerHwFlushAll(exactMatchManagerPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* delete the manager from the DB */
    rc = prvCpssDxChExactMatchManagerDbDelete(exactMatchManagerId);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* delete cuckoo DB */
    rc = prvCpssDxChCuckooDbDelete(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId);
    return rc;
}
/**
* @internal cpssDxChExactMatchManagerDelete function
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
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChExactMatchManagerDelete
(
    IN GT_U32 exactMatchManagerId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchManagerDelete);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, exactMatchManagerId));

    rc = internal_cpssDxChExactMatchManagerDelete(exactMatchManagerId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, exactMatchManagerId));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);

    return rc;
}

/**
* @internal duplicationsInDevListCheck function
* @endinternal
*
* @brief  function to check that no pair is more than once in the 'pair' list
*
* @param[in] pairListArr[]         - the array of pairs (device ids + portGroupBmp)
*                                    to add to the Exact Match Manager.
* @param[in] numOfPairs            - the number of pairs in the array.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman,Hauk.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - duplication detected
*/
static GT_STATUS    duplicationsInDevListCheck
(
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC    pairListArr[],/*arrSizeVarName=numOfPairs*/
    IN GT_U32                                                   numOfPairs
)
{
    GT_U32              ii,jj;

    GT_U8               devNum_ii;
    GT_PORT_GROUPS_BMP  portGroupsBmpArr_ii;

    GT_U8               devNum_jj;
    GT_PORT_GROUPS_BMP  portGroupsBmpArr_jj;

    for(ii = 0 ; ii < numOfPairs ; ii++)
    {
        devNum_ii = pairListArr[ii].devNum;
        portGroupsBmpArr_ii = pairListArr[ii].portGroupsBmp;

        for(jj = ii+1 ; jj < numOfPairs ; jj++)
        {
            devNum_jj = pairListArr[jj].devNum;
            portGroupsBmpArr_jj = pairListArr[jj].portGroupsBmp;

            if((devNum_ii == devNum_jj)&&
               (portGroupsBmpArr_ii == portGroupsBmpArr_jj))
            {
                /* duplication */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                    "devNum [%d] and portGroupsBmpArr[%d] is duplicated in the 'pair list' (index[%d] and index[%d])",
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
* @internal internal_cpssDxChExactMatchManagerDevListAdd function
* @endinternal
*
* @brief   This function adding (registering) device(s) to specific Exact Match Manager Instance.
*          It can be in initialization time or in run-time in the context of Hot-Insertion or PP reset.
*          Note: the assumption is that in all use cases (Init / Reset / Hot-Insertion) PP Exact Match
*          is flushed and don't have any old other entries. The only exception is the HA use-case
*          (which will be handled by dedicated procedure).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
static GT_STATUS internal_cpssDxChExactMatchManagerDevListAdd
(
    IN GT_U32                                                   exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC    pairListArr[],/*arrSizeVarName=numOfPairs*/
    IN GT_U32                                                   numOfPairs
)
{
    GT_STATUS           rc,rc1;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    GT_U32              ii;
    GT_U8               devNum;
    GT_PORT_GROUPS_BMP  portGroupsBmp;
    GT_PORT_GROUPS_BMP  tempPortGroupsBmp;
    GT_U32              isDeviceValid;

    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_STC         dbChecks;
    CPSS_SYSTEM_RECOVERY_INFO_STC                       tempSystemRecovery_Info;
    GT_BOOL                                             managerHwWriteBlock;

    EM_MANAGER_ID_CHECK(exactMatchManagerId);
    CPSS_NULL_PTR_CHECK_MAC(pairListArr);

    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    /*******************************************/
    /* check no device+portGroups validity */
    /*******************************************/
    if(numOfPairs == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "numOfPairs is ZERO ... no meaning calling the function");
    }
    for(ii = 0 ; ii < numOfPairs ; ii++)
    {
        devNum = pairListArr[ii].devNum;
        portGroupsBmp = pairListArr[ii].portGroupsBmp;

        PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
        PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
                                               | CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E | CPSS_ALDRIN2_E);

        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
        /* tempPortGroupsBmp can be changed after the call to the MACRO */
        tempPortGroupsBmp = portGroupsBmp;
        PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,tempPortGroupsBmp,PRV_CPSS_DXCH_UNIT_EM_E);
        tempPortGroupsBmp = portGroupsBmp;
        PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,tempPortGroupsBmp,PRV_CPSS_DXCH_UNIT_TCAM_E);
        tempPortGroupsBmp = portGroupsBmp;
        PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,tempPortGroupsBmp,PRV_CPSS_DXCH_UNIT_TTI_E);
        tempPortGroupsBmp = portGroupsBmp;
        PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,tempPortGroupsBmp,PRV_CPSS_DXCH_UNIT_PCL_E);
    }

    /*******************************************/
    /* check no device+portGroups duplications */
    /*******************************************/
    rc = duplicationsInDevListCheck(pairListArr,numOfPairs);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*******************************************/
    /* add devices to DB (+ DB validity check) */
    /*******************************************/
    rc = prvCpssDxChExactMatchManagerDbDevListAdd(exactMatchManagerId,pairListArr,numOfPairs);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* for error injection tests  ,simulate crash before hw write after sw DB write */
    rc = prvCpssSystemRecoveryErrorInjectionCheck (PRV_CPSS_SYSYEM_RECOVERY_ERROR_INJECTION_INDEX_EM_1_E);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* start HW initialization on the new devices */
    for(ii = 0 ; ii < numOfPairs ; ii++)
    {
        devNum = pairListArr[ii].devNum;
        portGroupsBmp = pairListArr[ii].portGroupsBmp;

        /* !!! start protecting the DB of the device !!! */
        EM_LOCK_DEV_NUM_SET_IS_VALID(devNum,isDeviceValid);
        if(!isDeviceValid)
        {
            /* unlock the device ... because the CPSS removed it ! */
            /* we can't use it !!! ... go to the next registered device */
            EM_UNLOCK_DEV_NUM(devNum);

            /* the device was removed during operation ?! */
            continue;
        }

        /* !!! done protecting the DB of the device !!! */
        EM_UNLOCK_DEV_NUM(devNum);
        rc = prvCpssDxChExactMatchManagerHwInitNewDevice(exactMatchManagerPtr,devNum,portGroupsBmp);
        if(rc != GT_OK)
        {
            break;
        }
    }

    if (rc != GT_OK)
    {
        /************************************************/
        /* remove devices from DB (+ DB validity check) */
        /************************************************/
        rc1 = prvCpssDxChExactMatchManagerDbDevListRemove(exactMatchManagerId,pairListArr,numOfPairs);
        if(rc1 != GT_OK)
        {
            return rc1;
        }
        else
        {
            return rc;
        }
    }
    /* in HA process we need to validate SW and HW are equal after syncronization. */
    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_EXACT_MATCH_MANAGER_E);
    if (((tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E) &&
        (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)) || managerHwWriteBlock == GT_TRUE )
    {
        cpssOsMemSet(&dbChecks,0,sizeof(dbChecks));
        dbChecks.globalCheckEnable = GT_TRUE;
        rc = prvCpssDxChExactMatchManagerHwDbValidityCheck(exactMatchManagerId,&dbChecks);
        if (rc != GT_OK)
        {
            /************************************************/
            /* remove devices from DB (+ DB validity check) */
            /************************************************/
            rc1 = prvCpssDxChExactMatchManagerDbDevListRemove(exactMatchManagerId,pairListArr,numOfPairs);
            if(rc1 != GT_OK)
            {
                return rc1;
            }
            else
            {
                return rc;
            }
        }

        /* if all is ok then we need to create a temporary database for the manager and
           read all HW memory to be used later when replay of entries is done */
        rc = prvCpssDxChExactMatchManagerHwHaTempDbCreate(exactMatchManagerId);
        if (rc != GT_OK)
        {
            /************************************************/
            /* remove devices from DB (+ DB validity check) */
            /************************************************/
            rc1 = prvCpssDxChExactMatchManagerDbDevListRemove(exactMatchManagerId,pairListArr,numOfPairs);
            if(rc1 != GT_OK)
            {
                return rc1;
            }
            else
            {
                return rc;
            }
        }
    }

    /* for error injection tests  ,simulate crash before hw write after sw DB write and Hw initialization on new device*/
    rc = prvCpssSystemRecoveryErrorInjectionCheck (PRV_CPSS_SYSYEM_RECOVERY_ERROR_INJECTION_INDEX_EM_2_E);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* start to write exact match entries from manager to HW of the new devices */
    rc = prvCpssDxChExactMatchManagerHwWriteEntriesFromDbToHwOfNewDevices(exactMatchManagerPtr,pairListArr,numOfPairs);
    if(rc != GT_OK)
    {
        /************************************************/
        /* remove devices from DB (+ DB validity check) */
        /************************************************/
        rc1 = prvCpssDxChExactMatchManagerDbDevListRemove(exactMatchManagerId,pairListArr,numOfPairs);
        if(rc1 != GT_OK)
        {
            return rc1;
        }
        else
        {
            return rc;
        }
    }

    return GT_OK;
}
/**
* @internal cpssDxChExactMatchManagerDevListAdd function
* @endinternal
*
* @brief   This function adding (registering) device(s) to specific Exact Match Manager Instance.
*          It can be in initialization time or in run-time in the context of Hot-Insertion or PP reset.
*          Note: the assumption is that in all use cases (Init / Reset / Hot-Insertion) PP Exact Match
*          is flushed and don't have any old other entries. The only exception is the HA use-case
*          (which will be handled by dedicated procedure).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
GT_STATUS cpssDxChExactMatchManagerDevListAdd
(
    IN GT_U32                                                   exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC    pairListArr[],/*arrSizeVarName=numOfPairs*/
    IN GT_U32                                                   numOfPairs
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchManagerDevListAdd);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, exactMatchManagerId,pairListArr,numOfPairs));

    rc = internal_cpssDxChExactMatchManagerDevListAdd(exactMatchManagerId,pairListArr,numOfPairs);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, exactMatchManagerId,pairListArr,numOfPairs));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChExactMatchManagerDevListRemove function
* @endinternal
*
* @brief   This function removing (unregistering) device(s) from
*          specific Exact Match Manager Instance (in Hot Removal
*          and reset etc..). NOTE: the Exact Match manager will
*          stop accessing to those devices (and will not access
*          to it during current function)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
static GT_STATUS internal_cpssDxChExactMatchManagerDevListRemove
(
    IN GT_U32                                                   exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC    pairListArr[],/*arrSizeVarName=numOfPairs*/
    IN GT_U32                                                   numOfPairs
)
{
    GT_STATUS           rc;
    GT_U8               devNum;
    GT_PORT_GROUPS_BMP  portGroupsBmp,tempPortGroupsBmp;
    GT_U32              ii;

    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;
    GT_BOOL                       managerHwWriteBlock;

    EM_MANAGER_ID_CHECK(exactMatchManagerId);
    CPSS_NULL_PTR_CHECK_MAC(pairListArr);

    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    /*******************************************/
    /* check no device+portGroups validity */
    /*******************************************/
    if(numOfPairs == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "numOfPairs is ZERO ... no meaning calling the function");
    }
    for(ii = 0 ; ii < numOfPairs ; ii++)
    {
        devNum = pairListArr[ii].devNum;
        portGroupsBmp = pairListArr[ii].portGroupsBmp;

        PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
        PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E
                                               | CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_AC3X_E | CPSS_ALDRIN2_E);

        PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
        /* tempPortGroupsBmp can be changed after the call to the MACRO */
        tempPortGroupsBmp = portGroupsBmp;
        PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,tempPortGroupsBmp,PRV_CPSS_DXCH_UNIT_EM_E);
        tempPortGroupsBmp = portGroupsBmp;
        PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,tempPortGroupsBmp,PRV_CPSS_DXCH_UNIT_TCAM_E);
        tempPortGroupsBmp = portGroupsBmp;
        PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,tempPortGroupsBmp,PRV_CPSS_DXCH_UNIT_TTI_E);
        tempPortGroupsBmp = portGroupsBmp;
        PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,tempPortGroupsBmp,PRV_CPSS_DXCH_UNIT_PCL_E);
    }

    /********************************/
    /* check no device duplications */
    /********************************/
    rc = duplicationsInDevListCheck(pairListArr,numOfPairs);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* in HA we should not delete from HW - since the delete read the entry first from the HW
       then we should skip this call, else we will get a fail on the read values that are not
       correct since we are in catchup stage */
    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_EXACT_MATCH_MANAGER_E);
    if (!((tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E) &&
         (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)) && managerHwWriteBlock == GT_FALSE)
    {
        /* Flush all entries from the HW of all devices in the list */
        for(ii = 0 ; ii < numOfPairs ; ii++)
        {
            devNum = pairListArr[ii].devNum;
            portGroupsBmp = pairListArr[ii].portGroupsBmp;
            rc = prvCpssDxChExactMatchManagerHwFlushAllSpecificDevice(exactMatchManagerPtr,devNum,portGroupsBmp);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    /**********************************************/
    /* remove devices from DB (+ DB validity check) */
    /**********************************************/
    rc = prvCpssDxChExactMatchManagerDbDevListRemove(exactMatchManagerId,pairListArr,numOfPairs);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}
/**
* @internal cpssDxChExactMatchManagerDevListRemove function
* @endinternal
*
* @brief   This function removing (unregistering) device(s) from
*          specific Exact Match Manager Instance (in Hot Removal
*          and reset etc..). NOTE: the Exact Match manager will
*          stop accessing to those devices (and will not access
*          to it during current function)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
GT_STATUS cpssDxChExactMatchManagerDevListRemove
(
    IN GT_U32                                                   exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC    pairListArr[],/*arrSizeVarName=numOfPairs*/
    IN GT_U32                                                   numOfPairs
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchManagerDevListRemove);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, exactMatchManagerId,pairListArr,numOfPairs));

    rc = internal_cpssDxChExactMatchManagerDevListRemove(exactMatchManagerId,pairListArr,numOfPairs);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, exactMatchManagerId,pairListArr,numOfPairs));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);

    return rc;
}

/*
* @internal prvExactMatchManagerTtiActionValidity function
* @endinternal
*
* @brief  Check that the Exact Match TTI Action correspond with
*         the TTI Action defined in the Profile Expander Table
*
* @param[in] logicFormatPtr -points to Exact Match Entry - tti action in logic format
* @param[in] dbLogicFormatPtr-points to Expanded Entry - tti action in logic format (keept in DB)
* @param[in] ttiExpandedActionOriginPtr-points to Expanded flags keept in DB
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvExactMatchManagerTtiActionValidity
(
    IN  CPSS_DXCH_TTI_ACTION_STC                             *logicFormatPtr,
    IN  CPSS_DXCH_TTI_ACTION_STC                             *dbLogicFormatPtr,
    IN  CPSS_DXCH_EXACT_MATCH_EXPANDED_TTI_ACTION_ORIGIN_STC *ttiExpandedActionOriginPtr
)
{
    GT_STATUS   rc=GT_OK;

    CPSS_NULL_PTR_CHECK_MAC(logicFormatPtr);
    CPSS_NULL_PTR_CHECK_MAC(dbLogicFormatPtr);
    CPSS_NULL_PTR_CHECK_MAC(ttiExpandedActionOriginPtr);

    if (ttiExpandedActionOriginPtr->overwriteExpandedActionCommand==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->command != dbLogicFormatPtr->command)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in command field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionUserDefinedCpuCode==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->userDefinedCpuCode != dbLogicFormatPtr->userDefinedCpuCode)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in userDefinedCpuCode field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionMirrorToIngressAnalyzerIndex==GT_FALSE)
    {
        /* check that the fields are equal */
        if ((logicFormatPtr->mirrorToIngressAnalyzerEnable != dbLogicFormatPtr->mirrorToIngressAnalyzerEnable)||
            (logicFormatPtr->mirrorToIngressAnalyzerIndex != dbLogicFormatPtr->mirrorToIngressAnalyzerIndex))
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in mirrorToIngressAnalyzerEnable/mirrorToIngressAnalyzerIndex field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionRedirectCommand==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->redirectCommand != dbLogicFormatPtr->redirectCommand)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirectCommand field \n");
        }
    }
    if(logicFormatPtr->redirectCommand==CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E)
    {
        if (ttiExpandedActionOriginPtr->overwriteExpandedActionEgressInterface==GT_FALSE)
        {
            /* check that the fields are equal */
            if  (logicFormatPtr->egressInterface.type != dbLogicFormatPtr->egressInterface.type)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in egressInterface.type field \n");
            }
            switch(logicFormatPtr->egressInterface.type)
            {
            case CPSS_INTERFACE_PORT_E:
                /* check that the fields are equal */
                if(logicFormatPtr->egressInterface.devPort.portNum != dbLogicFormatPtr->egressInterface.devPort.portNum)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in egressInterface.devPort.portNum  field \n");
                }
                /* check that the fields are equal */
                if(logicFormatPtr->egressInterface.devPort.hwDevNum != dbLogicFormatPtr->egressInterface.devPort.hwDevNum)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in egressInterface.devPort.hwDevNum  field \n");
                }
                break;
            case CPSS_INTERFACE_TRUNK_E:
                /* check that the fields are equal */
                if(logicFormatPtr->egressInterface.trunkId != dbLogicFormatPtr->egressInterface.trunkId)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in egressInterface.trunkId field \n");
                }
                break;
            case CPSS_INTERFACE_VIDX_E:
                /* check that the fields are equal */
                if (logicFormatPtr->egressInterface.vidx != dbLogicFormatPtr->egressInterface.vidx)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in egressInterface.vidx  field \n");
                }
                break;
            case CPSS_INTERFACE_VID_E:
                /* check that the fields are equal */
                if(logicFormatPtr->egressInterface.vlanId != dbLogicFormatPtr->egressInterface.vlanId)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in egressInterface.vlanId field \n");
                }
                break;
            case CPSS_INTERFACE_DEVICE_E:
                /* check that the fields are equal */
                if(logicFormatPtr->egressInterface.hwDevNum != dbLogicFormatPtr->egressInterface.hwDevNum)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in egressInterface.hwDevNum field \n");
                }
                break;
            case CPSS_INTERFACE_FABRIC_VIDX_E:
                 /* check that the fields are equal */
                if (logicFormatPtr->egressInterface.fabricVidx != dbLogicFormatPtr->egressInterface.fabricVidx)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in egressInterface.fabricVidx field \n");
                }
                break;
            case CPSS_INTERFACE_INDEX_E:
                /* check that the fields are equal */
                if(logicFormatPtr->egressInterface.index != dbLogicFormatPtr->egressInterface.index)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in egressInterface.index field \n");
                }
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in egressInterface \n");
                break;
            }
        }

        if (ttiExpandedActionOriginPtr->overwriteExpandedActionVntl2Echo==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->vntl2Echo != dbLogicFormatPtr->vntl2Echo)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in vntl2Echo field \n");
            }
        }
        if (ttiExpandedActionOriginPtr->overwriteExpandedActionTunnelStart==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->tunnelStart != dbLogicFormatPtr->tunnelStart)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in tunnelStart field \n");
            }
        }
        if (logicFormatPtr->tunnelStart==GT_FALSE)
        {
            if (ttiExpandedActionOriginPtr->overwriteExpandedActionArpPtr == GT_FALSE)
            {
                /* check that the fields are equal */
                if (logicFormatPtr->arpPtr != dbLogicFormatPtr->arpPtr)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in arpPtr field \n");
                }
            }
        }
        else
        {
            if (ttiExpandedActionOriginPtr->overwriteExpandedActionTunnelIndex==GT_FALSE)
            {
                /* check that the fields are equal */
                if (logicFormatPtr->tunnelStartPtr != dbLogicFormatPtr->tunnelStartPtr)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in tunnelStartPtr field \n");
                }
            }
            if (ttiExpandedActionOriginPtr->overwriteExpandedActionTsPassengerPacketType==GT_FALSE)
            {
                /* check that the fields are equal */
                if (logicFormatPtr->tsPassengerPacketType != dbLogicFormatPtr->tsPassengerPacketType)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in tsPassengerPacketType field \n");
                }
            }
        }
        if (ttiExpandedActionOriginPtr->overwriteExpandedActionModifyMacDa==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->modifyMacDa != dbLogicFormatPtr->modifyMacDa)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in modifyMacDa field \n");
            }
        }
        if (ttiExpandedActionOriginPtr->overwriteExpandedActionModifyMacSa==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->modifyMacSa != dbLogicFormatPtr->modifyMacSa)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in modifyMacSa field \n");
            }
        }
    }
    if(logicFormatPtr->redirectCommand==CPSS_DXCH_TTI_VRF_ID_ASSIGN_E)
    {
        if (ttiExpandedActionOriginPtr->overwriteExpandedActionVrfId==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->vrfId != dbLogicFormatPtr->vrfId)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in vrfId field \n");
            }
        }
    }
    if(logicFormatPtr->redirectCommand==CPSS_DXCH_TTI_NO_REDIRECT_E)
    {
        if (ttiExpandedActionOriginPtr->overwriteExpandedActionFlowId==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->flowId != dbLogicFormatPtr->flowId)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in flowId field \n");
            }
        }
        if (ttiExpandedActionOriginPtr->overwriteExpandedActionIpclUdbConfigTable==GT_FALSE)
        {
            /* check that the fields are equal */
            if ((logicFormatPtr->iPclUdbConfigTableEnable != dbLogicFormatPtr->iPclUdbConfigTableEnable)||
                (logicFormatPtr->iPclUdbConfigTableIndex != dbLogicFormatPtr->iPclUdbConfigTableIndex))
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in iPclUdbConfigTableEnable/iPclUdbConfigTableIndex field \n");
            }
        }
    }
    if(logicFormatPtr->redirectCommand==CPSS_DXCH_TTI_REDIRECT_TO_ROUTER_LOOKUP_E)
    {
        if (ttiExpandedActionOriginPtr->overwriteExpandedActionRouterLttPtr==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->routerLttPtr!= dbLogicFormatPtr->routerLttPtr)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in routerLttPtr field \n");
            }
        }
    }
    if(logicFormatPtr->redirectCommand!=CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E)
    {
        if (ttiExpandedActionOriginPtr->overwriteExpandedActionPcl2OverrideConfigIndex == GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->pcl1OverrideConfigIndex != dbLogicFormatPtr->pcl1OverrideConfigIndex)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in pcl1OverrideConfigIndex field \n");
            }
        }
        if (ttiExpandedActionOriginPtr->overwriteExpandedActionPcl1OverrideConfigIndex==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->pcl0_1OverrideConfigIndex != dbLogicFormatPtr->pcl0_1OverrideConfigIndex)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in pcl0_1OverrideConfigIndex field \n");
            }
        }
        if (ttiExpandedActionOriginPtr->overwriteExpandedActionPcl0OverrideConfigIndex==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->pcl0OverrideConfigIndex != dbLogicFormatPtr->pcl0OverrideConfigIndex)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in pcl0OverrideConfigIndex field \n");
            }
        }
        if (ttiExpandedActionOriginPtr->overwriteExpandedActionIpclConfigIndex==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->iPclConfigIndex!= dbLogicFormatPtr->iPclConfigIndex)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in iPclConfigIndex field \n");
            }
        }

        if(logicFormatPtr->copyReservedAssignmentEnable == GT_TRUE)
        {
            if (ttiExpandedActionOriginPtr->overwriteExpandedActionCopyReservedAssignmentEnable==GT_FALSE)
            {
                /* check that the fields are equal */
                if (logicFormatPtr->copyReserved != dbLogicFormatPtr->copyReserved )
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in copyReserved field \n");
                }
            }
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionBindToCentralCounter==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->bindToCentralCounter != dbLogicFormatPtr->bindToCentralCounter)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in bindToCentralCounter field \n");
        }
    }

    if (ttiExpandedActionOriginPtr->overwriteExpandedActionCentralCounterIndex == GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->centralCounterIndex != dbLogicFormatPtr->centralCounterIndex)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in centralCounterIndex field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionBindToPolicerMeter==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->bindToPolicerMeter != dbLogicFormatPtr->bindToPolicerMeter)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in bindToPolicerMeter field \n");
        }
    }

    if (ttiExpandedActionOriginPtr->overwriteExpandedActionBindToPolicer == GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->bindToPolicer != dbLogicFormatPtr->bindToPolicer)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in bindToPolicer field \n");
        }
    }

    if (ttiExpandedActionOriginPtr->overwriteExpandedActionPolicerIndex==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->policerIndex != dbLogicFormatPtr->policerIndex)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in policerIndex field \n");
        }
    }

    if (ttiExpandedActionOriginPtr->overwriteExpandedActionSourceIdSetEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->sourceIdSetEnable != dbLogicFormatPtr->sourceIdSetEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in sourceIdSetEnable field \n");
        }
    }

    if (ttiExpandedActionOriginPtr->overwriteExpandedActionSourceId == GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->sourceId != dbLogicFormatPtr->sourceId)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in sourceId field \n");
        }
    }

    if (ttiExpandedActionOriginPtr->overwriteExpandedActionActionStop==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->actionStop != dbLogicFormatPtr->actionStop)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in actionStop field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionBridgeBypass==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->bridgeBypass != dbLogicFormatPtr->bridgeBypass)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in bridgeBypass field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionIngressPipeBypass==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->ingressPipeBypass != dbLogicFormatPtr->ingressPipeBypass)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in ingressPipeBypass field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTag0VlanPrecedence==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->tag0VlanPrecedence!= dbLogicFormatPtr->tag0VlanPrecedence)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in tag0VlanPrecedence field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionNestedVlanEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->nestedVlanEnable != dbLogicFormatPtr->nestedVlanEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in nestedVlanEnable field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTag0VlanCmd==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->tag0VlanCmd != dbLogicFormatPtr->tag0VlanCmd)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in tag0VlanCmd field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTag0VlanId==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->tag0VlanId != dbLogicFormatPtr->tag0VlanId)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in tag0VlanId field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTag1VlanCmd==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->tag1VlanCmd != dbLogicFormatPtr->tag1VlanCmd)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in tag1VlanCmd field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTag1VlanId==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->tag1VlanId != dbLogicFormatPtr->tag1VlanId)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in tag1VlanId field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionQosPrecedence==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->qosPrecedence != dbLogicFormatPtr->qosPrecedence)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in qosPrecedence field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionQosProfile==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->qosProfile != dbLogicFormatPtr->qosProfile)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in qosProfile field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionModifyDscp==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->modifyDscp != dbLogicFormatPtr->modifyDscp)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in modifyDscp field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionModifyTag0==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->modifyTag0Up != dbLogicFormatPtr->modifyTag0Up)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in modifyTag0Up field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionKeepPreviousQoS==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->keepPreviousQoS != dbLogicFormatPtr->keepPreviousQoS)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in keepPreviousQoS field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTrustUp==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->trustUp != dbLogicFormatPtr->trustUp)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in trustUp field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTrustDscp==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->trustDscp != dbLogicFormatPtr->trustDscp)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in trustDscp field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTrustExp==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->trustExp != dbLogicFormatPtr->trustExp)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in trustExp field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionRemapDscp==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->remapDSCP != dbLogicFormatPtr->remapDSCP)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in remapDSCP field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTag0Up==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->tag0Up != dbLogicFormatPtr->tag0Up)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in tag0Up field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTag1UpCommand==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->tag1UpCommand != dbLogicFormatPtr->tag1UpCommand)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in tag1UpCommand field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTag1Up==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->tag1Up != dbLogicFormatPtr->tag1Up)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in tag1Up field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTtPassengerPacketType==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->ttPassengerPacketType != dbLogicFormatPtr->ttPassengerPacketType)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in ttPassengerPacketType field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionCopyTtlExpFromTunnelHeader==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->copyTtlExpFromTunnelHeader != dbLogicFormatPtr->copyTtlExpFromTunnelHeader)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in copyTtlExpFromTunnelHeader field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTunnelTerminate==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->tunnelTerminate != dbLogicFormatPtr->tunnelTerminate)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in tunnelTerminate field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionMplsCommand==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->mplsCommand != dbLogicFormatPtr->mplsCommand)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in mplsCommand field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionHashMaskIndex==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->hashMaskIndex!= dbLogicFormatPtr->hashMaskIndex)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in hashMaskIndex field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionQosMappingTableIndex==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->qosMappingTableIndex != dbLogicFormatPtr->qosMappingTableIndex)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in qosMappingTableIndex field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionMplsLLspQoSProfileEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->mplsLLspQoSProfileEnable != dbLogicFormatPtr->mplsLLspQoSProfileEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in mplsLLspQoSProfileEnable field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionMplsTtl==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->mplsTtl != dbLogicFormatPtr->mplsTtl)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in mplsTtl field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionEnableDecrementTtl==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->enableDecrementTtl != dbLogicFormatPtr->enableDecrementTtl)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in enableDecrementTtl field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionSourceEPortAssignmentEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->sourceEPortAssignmentEnable != dbLogicFormatPtr->sourceEPortAssignmentEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in sourceEPortAssignmentEnable field \n");
        }
    }

    if (ttiExpandedActionOriginPtr->overwriteExpandedActionSourceEPort==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->sourceEPort != dbLogicFormatPtr->sourceEPort)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in sourceEPort field \n");
        }
    }

    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTtHeaderLength==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->ttHeaderLength != dbLogicFormatPtr->ttHeaderLength)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in ttHeaderLength field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionIsPtpPacket==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->isPtpPacket != dbLogicFormatPtr->isPtpPacket)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in isPtpPacket field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionOamTimeStampEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->oamTimeStampEnable != dbLogicFormatPtr->oamTimeStampEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in oamTimeStampEnable field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionPtpTriggerType==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->ptpTriggerType != dbLogicFormatPtr->ptpTriggerType)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in ptpTriggerType field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionPtpOffset==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->ptpOffset != dbLogicFormatPtr->ptpOffset)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in ptpOffset field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionOamOffsetIndex==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->oamOffsetIndex != dbLogicFormatPtr->oamOffsetIndex)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in oamOffsetIndex field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionOamProcessWhenGalOrOalExistsEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->oamProcessWhenGalOrOalExistsEnable != dbLogicFormatPtr->oamProcessWhenGalOrOalExistsEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in oamProcessWhenGalOrOalExistsEnable field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionOamProcessEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->oamProcessEnable != dbLogicFormatPtr->oamProcessEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in oamProcessEnable field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionCwBasedPw==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->cwBasedPw != dbLogicFormatPtr->cwBasedPw)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in cwBasedPw field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTtlExpiryVccvEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->ttlExpiryVccvEnable != dbLogicFormatPtr->ttlExpiryVccvEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in ttlExpiryVccvEnable field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionPwe3FlowLabelExist==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->pwe3FlowLabelExist != dbLogicFormatPtr->pwe3FlowLabelExist)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in pwe3FlowLabelExist field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionPwCwBasedETreeEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->pwCwBasedETreeEnable != dbLogicFormatPtr->pwCwBasedETreeEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in pwCwBasedETreeEnable field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionQosUseUpAsIndexEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->qosUseUpAsIndexEnable != dbLogicFormatPtr->qosUseUpAsIndexEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in qosUseUpAsIndexEnable field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionRxIsProtectionPath==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->rxIsProtectionPath != dbLogicFormatPtr->rxIsProtectionPath)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in rxIsProtectionPath field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionRxProtectionSwitchEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->rxProtectionSwitchEnable != dbLogicFormatPtr->rxProtectionSwitchEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in rxProtectionSwitchEnable field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionSetMacToMe==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->setMacToMe != dbLogicFormatPtr->setMacToMe)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in setMacToMe field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionOamProfile==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->oamProfile != dbLogicFormatPtr->oamProfile)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in oamProfile field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionApplyNonDataCwCommand==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->applyNonDataCwCommand != dbLogicFormatPtr->applyNonDataCwCommand)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in applyNonDataCwCommand field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionContinueToNextTtiLookup==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->continueToNextTtiLookup != dbLogicFormatPtr->continueToNextTtiLookup)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in continueToNextTtiLookup field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->oamChannelTypeToOpcodeMappingEnable != dbLogicFormatPtr->oamChannelTypeToOpcodeMappingEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in oamChannelTypeToOpcodeMappingEnable field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionPassengerParsingOfTransitMplsTunnelMode==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->passengerParsingOfTransitMplsTunnelMode != dbLogicFormatPtr->passengerParsingOfTransitMplsTunnelMode)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in passengerParsingOfTransitMplsTunnelMode field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionPassengerParsingOfTransitNonMplsTransitTunnelEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr-> passengerParsingOfTransitNonMplsTransitTunnelEnable != dbLogicFormatPtr->passengerParsingOfTransitNonMplsTransitTunnelEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in passengerParsingOfTransitNonMplsTransitTunnelEnable field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionSkipFdbSaLookupEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->skipFdbSaLookupEnable != dbLogicFormatPtr->skipFdbSaLookupEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in skipFdbSaLookupEnable field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionIpv6SegmentRoutingEndNodeEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->ipv6SegmentRoutingEndNodeEnable != dbLogicFormatPtr->ipv6SegmentRoutingEndNodeEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in ipv6SegmentRoutingEndNodeEnable field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTunnelHeaderStartL4Enable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->tunnelHeaderLengthAnchorType != dbLogicFormatPtr->tunnelHeaderLengthAnchorType)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in tunnelHeaderLengthAnchorType field \n");
        }
    }
    if (ttiExpandedActionOriginPtr->overwriteExpandedActionTriggerHashCncClient==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->triggerHashCncClient != dbLogicFormatPtr->triggerHashCncClient)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in triggerHashCncClient field \n");
        }
    }
    return rc;
}
/*
* @internal validateEntryFormatParams_exactMatchEntryTtiFormat function
* @endinternal
*
* @brief  function to validate the 'API LEVEL' Exact Match entry TTI format parameters.
*         based on prvCpssDxChExactMatchTtiActionValidity
*         NOTE: the 'key' was already checked
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match Manager.
* @param[in] entryPtr              - (pointer to) the entry (application format)
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   validateEntryFormatParams_exactMatchEntryTtiFormat
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                  *entryPtr
)
{
    GT_U32                                  rc = GT_OK;
    GT_U32                                  expandedActionIndex;

    expandedActionIndex = entryPtr->expandedActionIndex;

    /* validate the Exact Match entry parameter before we save it to the DB */
    rc = prvCpssDxChExactMatchManagerDbValidateTtiAction(&(entryPtr->exactMatchAction.ttiAction));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"Error in validation of TTI Action expandedActionParamsPtr->expandedAction, expandedActionIndex[%d] ",
                                      expandedActionIndex);
    }

    /* Check that the Exact Match TTI Action correspond with
       the TTI Action defined in the Profile Expander Table */
    rc = prvExactMatchManagerTtiActionValidity(&(entryPtr->exactMatchAction.ttiAction),
                                                &exactMatchManagerPtr->exactMatchExpanderArray[expandedActionIndex].exactMatchExpandedEntry.expandedAction.ttiAction,
                                                &exactMatchManagerPtr->exactMatchExpanderArray[expandedActionIndex].exactMatchExpandedEntry.expandedActionOrigin.ttiExpandedActionOrigin);

    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}
/*
* @internal prvExactMatchManagerPclActionValidity function
* @endinternal
*
* @brief  Check that the Exact Match PCL Action correspond with
*         the PCL Action defined in the Profile Expander Table
*
* @param[in] devNum                     - device number
* @param[in] logicFormatPtr             - points to Exact Match Entry - PCL action in logic format
* @param[in] dbLogicFormatPtr           - points to Expanded Entry - PCL action in logic format (keept in DB)
* @param[in] pclExpandedActionOriginPtr - points to Expanded flags keept in DB
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvExactMatchManagerPclActionValidity
(
    IN  CPSS_DXCH_PCL_ACTION_STC                                *logicFormatPtr,
    IN  CPSS_DXCH_PCL_ACTION_STC                                *dbLogicFormatPtr,
    IN  CPSS_DXCH_EXACT_MATCH_EXPANDED_PCL_ACTION_ORIGIN_STC    *pclExpandedActionOriginPtr
)
{
    GT_STATUS   rc=GT_OK;

    CPSS_NULL_PTR_CHECK_MAC(logicFormatPtr);
    CPSS_NULL_PTR_CHECK_MAC(dbLogicFormatPtr);
    CPSS_NULL_PTR_CHECK_MAC(pclExpandedActionOriginPtr);

    if (pclExpandedActionOriginPtr->overwriteExpandedActionUserDefinedCpuCode==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->mirror.cpuCode != dbLogicFormatPtr->mirror.cpuCode)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in mirror.cpuCode field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionPktCmd==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->pktCmd != dbLogicFormatPtr->pktCmd)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in pktCmd field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionRedirectCommand==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->redirect.redirectCmd != dbLogicFormatPtr->redirect.redirectCmd)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.redirectCmd field \n");
        }
    }

    if(logicFormatPtr->redirect.redirectCmd==CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E)
    {
        if (pclExpandedActionOriginPtr->overwriteExpandedActionEgressInterface==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->redirect.data.outIf.outInterface.type != dbLogicFormatPtr->redirect.data.outIf.outInterface.type )
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.outIf.outInterface.type  field \n");
            }
            switch(logicFormatPtr->redirect.data.outIf.outInterface.type)
            {
            case CPSS_INTERFACE_PORT_E:
                /* check that the fields are equal */
                if (logicFormatPtr->redirect.data.outIf.outInterface.devPort.portNum != dbLogicFormatPtr->redirect.data.outIf.outInterface.devPort.portNum )
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.outIf.outInterface.devPort.portNum  field \n");
                }
                /* check that the fields are equal */
                if (logicFormatPtr->redirect.data.outIf.outInterface.devPort.hwDevNum  != dbLogicFormatPtr->redirect.data.outIf.outInterface.devPort.hwDevNum)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.outIf.outInterface.devPort.hwDevNum field \n");
                }
                break;
            case CPSS_INTERFACE_TRUNK_E:
                /* check that the fields are equal */
                if (logicFormatPtr->redirect.data.outIf.outInterface.trunkId  != dbLogicFormatPtr->redirect.data.outIf.outInterface.trunkId)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.outIf.outInterface.trunkId field \n");
                }
                break;
            case CPSS_INTERFACE_VIDX_E:
                /* check that the fields are equal */
                if (logicFormatPtr->redirect.data.outIf.outInterface.vidx != dbLogicFormatPtr->redirect.data.outIf.outInterface.vidx )
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.outIf.outInterface.vidx  field \n");
                }
                break;
            case CPSS_INTERFACE_VID_E:
                /* check that the fields are equal */
                if (logicFormatPtr->redirect.data.outIf.outInterface.vlanId != dbLogicFormatPtr->redirect.data.outIf.outInterface.vlanId )
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.outIf.outInterface.vlanId  field \n");
                }
                break;
            case CPSS_INTERFACE_DEVICE_E:
                /* check that the fields are equal */
                if (logicFormatPtr->redirect.data.outIf.outInterface.hwDevNum != dbLogicFormatPtr->redirect.data.outIf.outInterface.hwDevNum)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.outIf.outInterface.hwDevNum field \n");
                }
                break;
            case CPSS_INTERFACE_FABRIC_VIDX_E:
                 /* check that the fields are equal */
                if (logicFormatPtr->redirect.data.outIf.outInterface.fabricVidx != dbLogicFormatPtr->redirect.data.outIf.outInterface.fabricVidx)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.outIf.outInterface.fabricVidx field \n");
                }
                break;
            case CPSS_INTERFACE_INDEX_E:
                /* check that the fields are equal */
                if (logicFormatPtr->redirect.data.outIf.outInterface.index != dbLogicFormatPtr->redirect.data.outIf.outInterface.index)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.outIf.outInterface.index field \n");
                }
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in logicFormatPtr->redirect.data.outIf.outInterface.typefield \n");
                break;
            }
        }
        if (pclExpandedActionOriginPtr->overwriteExpandedActionVntl2Echo==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->redirect.data.outIf.vntL2Echo != dbLogicFormatPtr->redirect.data.outIf.vntL2Echo)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.outIf.vntL2Echo field \n");
            }
        }
        if (pclExpandedActionOriginPtr->overwriteExpandedActionModifyMacDA==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->redirect.data.outIf.modifyMacDa != dbLogicFormatPtr->redirect.data.outIf.modifyMacDa)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.outIf.modifyMacDa field \n");
            }
        }
        if (pclExpandedActionOriginPtr->overwriteExpandedActionModifyMacSA==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->redirect.data.outIf.modifyMacSa != dbLogicFormatPtr->redirect.data.outIf.modifyMacSa)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.outIf.vntL2Echo field \n");
            }
        }
        if (pclExpandedActionOriginPtr->overwriteExpandedActionTunnelStart==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->redirect.data.outIf.tunnelStart != dbLogicFormatPtr->redirect.data.outIf.tunnelStart)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.outIf.tunnelStart field \n");
            }
        }
        if (logicFormatPtr->redirect.data.outIf.tunnelStart==GT_TRUE)
        {
            if (pclExpandedActionOriginPtr->overwriteExpandedActionTunnelIndex == GT_FALSE)
            {
                /* check that the fields are equal */
                if (logicFormatPtr->redirect.data.outIf.tunnelPtr != dbLogicFormatPtr->redirect.data.outIf.tunnelPtr)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in logicFormatPtr->redirect.data.outIf.tunnelPtr field \n");
                }
            }
            if (pclExpandedActionOriginPtr->overwriteExpandedActionTsPassengerPacketType==GT_FALSE)
            {
               /* check that the fields are equal */
                if (logicFormatPtr->redirect.data.outIf.tunnelType != dbLogicFormatPtr->redirect.data.outIf.tunnelType)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.outIf.tunnelType field \n");
                }
            }
        }
        else
        {
            if (pclExpandedActionOriginPtr->overwriteExpandedActionArpPtr==GT_FALSE)
            {
                /* check that the fields are equal */
                if (logicFormatPtr->redirect.data.outIf.arpPtr != dbLogicFormatPtr->redirect.data.outIf.arpPtr)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.outIf.arpPtr field \n");
                }
            }
        }
    }
    if(logicFormatPtr->redirect.redirectCmd==CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E)
    {
        if(pclExpandedActionOriginPtr->overwriteExpandedActionMacSa==GT_FALSE)
        {
            if(cpssOsMemCmp(logicFormatPtr->redirect.data.modifyMacSa.macSa.arEther,
                         dbLogicFormatPtr->redirect.data.modifyMacSa.macSa.arEther,
                         sizeof(dbLogicFormatPtr->redirect.data.modifyMacSa.macSa.arEther))!=0)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.modifyMacSa.macSa field \n");
            }
            /* check that the fields are equal */
            if (logicFormatPtr->redirect.data.modifyMacSa.arpPtr != dbLogicFormatPtr->redirect.data.modifyMacSa.arpPtr)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.modifyMacSa.arpPtr field \n");
            }
        }
    }

    if((logicFormatPtr->redirect.redirectCmd == CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_E) ||
       (logicFormatPtr->redirect.redirectCmd == CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ECMP_E))
    {
        if (pclExpandedActionOriginPtr->overwriteExpandedActionRouterLttPtr==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->redirect.data.routerLttIndex != dbLogicFormatPtr->redirect.data.routerLttIndex)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.routerLttIndex field \n");
            }
        }
    }
    if (logicFormatPtr->redirect.redirectCmd==CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E)

    {
        if (pclExpandedActionOriginPtr->overwriteExpandedActionVrfId==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->redirect.data.vrfId != dbLogicFormatPtr->redirect.data.vrfId )
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.vrfId field \n");
            }
        }
    }

    if((logicFormatPtr->redirect.redirectCmd == CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_AND_ASSIGN_VRF_ID_E) ||
       (logicFormatPtr->redirect.redirectCmd == CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ECMP_AND_ASSIGN_VRF_ID_E))
    {
        if (pclExpandedActionOriginPtr->overwriteExpandedActionRouterLttPtr==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->redirect.data.routeAndVrfId.routerLttIndex != dbLogicFormatPtr->redirect.data.routeAndVrfId.routerLttIndex)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.routerLttIndex field \n");
            }
        }
        if (pclExpandedActionOriginPtr->overwriteExpandedActionVrfId==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->redirect.data.routeAndVrfId.vrfId != dbLogicFormatPtr->redirect.data.routeAndVrfId.vrfId)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in redirect.data.vrfId field \n");
            }
        }
    }

    if (pclExpandedActionOriginPtr->overwriteExpandedActionActionStop==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->actionStop != dbLogicFormatPtr->actionStop)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in actionStop field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionSetMacToMe==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->setMacToMe != dbLogicFormatPtr->setMacToMe)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in setMacToMe field \n");
        }
    }
    /* PCL1-pcl0_1OverrideConfigIndex and PCL2-pcl1OverrideConfigIndex */
    if (pclExpandedActionOriginPtr->overwriteExpandedActionPCL2OverrideConfigIndex==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->lookupConfig.pcl1OverrideConfigIndex != dbLogicFormatPtr->lookupConfig.pcl1OverrideConfigIndex)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in lookupConfig.pcl1OverrideConfigIndex field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionPCL1OverrideConfigIndex==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->lookupConfig.pcl0_1OverrideConfigIndex != dbLogicFormatPtr->lookupConfig.pcl0_1OverrideConfigIndex)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in lookupConfig.pcl0_1OverrideConfigIndex field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionIPCLConfigurationIndex==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->lookupConfig.ipclConfigIndex != dbLogicFormatPtr->lookupConfig.ipclConfigIndex)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in lookupConfig.ipclConfigIndex field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionPolicerEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->policer.policerEnable != dbLogicFormatPtr->policer.policerEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in policer.policerEnable field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionPolicerIndex==GT_FALSE)
    {
       /* check that the fields are equal */
        if (logicFormatPtr->policer.policerId != dbLogicFormatPtr->policer.policerId)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in policer.policerId field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionMirrorTcpRstAndFinPacketsToCpu==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->mirror.mirrorTcpRstAndFinPacketsToCpu != dbLogicFormatPtr->mirror.mirrorTcpRstAndFinPacketsToCpu)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in mirror.mirrorTcpRstAndFinPacketsToCpu field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionMirrorMode==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->mirror.egressMirrorToAnalyzerMode != dbLogicFormatPtr->mirror.egressMirrorToAnalyzerMode)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in mirror.egressMirrorToAnalyzerMode field \n");
        }
    }

    if (pclExpandedActionOriginPtr->overwriteExpandedActionMirror==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->mirror.mirrorToRxAnalyzerPort != dbLogicFormatPtr->mirror.mirrorToRxAnalyzerPort)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in mirror.mirrorToRxAnalyzerPort field \n");
        }
        /* check that the fields are equal */
        if (logicFormatPtr->mirror.egressMirrorToAnalyzerIndex != dbLogicFormatPtr->mirror.egressMirrorToAnalyzerIndex)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in mirror.egressMirrorToAnalyzerIndex field \n");
        }

        /* check that the fields are equal */
        if (logicFormatPtr->mirror.mirrorToTxAnalyzerPortEn != dbLogicFormatPtr->mirror.mirrorToTxAnalyzerPortEn)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in mirror.mirrorToTxAnalyzerPortEn field \n");
        }
        /* check that the fields are equal */
        if (logicFormatPtr->mirror.ingressMirrorToAnalyzerIndex != dbLogicFormatPtr->mirror.ingressMirrorToAnalyzerIndex)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in mirror.ingressMirrorToAnalyzerIndex field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionBypassBridge==GT_FALSE)
    {
       /* check that the fields are equal */
        if (logicFormatPtr->bypassBridge != dbLogicFormatPtr->bypassBridge)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in bypassBridge field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionBypassIngressPipe==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->bypassIngressPipe != dbLogicFormatPtr->bypassIngressPipe)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in bypassIngressPipe field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionNestedVlanEnable==GT_FALSE)
    {
        if(logicFormatPtr->egressPolicy == GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->vlan.ingress.nestedVlan != dbLogicFormatPtr->vlan.ingress.nestedVlan)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in vlan.ingress.nestedVlan field \n");
            }
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionVlanPrecedence==GT_FALSE)
    {
        if(logicFormatPtr->egressPolicy == GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->vlan.ingress.precedence != dbLogicFormatPtr->vlan.ingress.precedence)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in vlan.ingress.precedence field \n");
            }
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionVlan0Command==GT_FALSE)
    {
        if(logicFormatPtr->egressPolicy == GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->vlan.ingress.modifyVlan != dbLogicFormatPtr->vlan.ingress.modifyVlan)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in vlan.ingress.modifyVlan field \n");
            }
        }
        else
        {
            /* check that the fields are equal */
            if (logicFormatPtr->vlan.egress.vlanCmd != dbLogicFormatPtr->vlan.egress.vlanCmd)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in vlan.egress.vlanCmd field \n");
            }
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionVlan0==GT_FALSE)
    {
        if(logicFormatPtr->egressPolicy == GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->vlan.ingress.vlanId != dbLogicFormatPtr->vlan.ingress.vlanId)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in vlan.ingress.vlanId field \n");
            }
        }
        else
        {
            /* check that the fields are equal */
            if (logicFormatPtr->vlan.egress.vlanId != dbLogicFormatPtr->vlan.egress.vlanId)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in vlan.egress.vlanId field \n");
            }
        }
    }

    if (pclExpandedActionOriginPtr->overwriteExpandedActionVlan1Command==GT_FALSE)
    {
        if(logicFormatPtr->egressPolicy == GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->vlan.ingress.vlanId1Cmd != dbLogicFormatPtr->vlan.ingress.vlanId1Cmd)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in vlan.ingress.vlanId1Cmd field \n");
            }
        }
        else
        {
            /* check that the fields are equal */
            if (logicFormatPtr->vlan.egress.vlanId1ModifyEnable != dbLogicFormatPtr->vlan.egress.vlanId1ModifyEnable)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in vlan.egress.vlanId1ModifyEnable field \n");
            }
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionVlan1==GT_FALSE)
    {
        if(logicFormatPtr->egressPolicy == GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->vlan.ingress.vlanId1 != dbLogicFormatPtr->vlan.ingress.vlanId1)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in vlan.ingress.vlanId1 field \n");
            }
        }
        else
        {
            /* check that the fields are equal */
            if (logicFormatPtr->vlan.egress.vlanId1 != dbLogicFormatPtr->vlan.egress.vlanId1)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in vlan.egress.vlanId1 field \n");
            }
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionSourceIdEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->sourceId.assignSourceId != dbLogicFormatPtr->sourceId.assignSourceId)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in sourceId.assignSourceId field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionSourceId==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->sourceId.sourceIdValue != dbLogicFormatPtr->sourceId.sourceIdValue)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in sourceId.sourceIdValue field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionMatchCounterEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->matchCounter.enableMatchCount != dbLogicFormatPtr->matchCounter.enableMatchCount)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in matchCounter,enableMatchCount field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionMatchCounterIndex==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->matchCounter.matchCounterIndex != dbLogicFormatPtr->matchCounter.matchCounterIndex)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in matchCounter.matchCounterIndex field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionQosProfileMakingEnable==GT_FALSE)
    {
        if(logicFormatPtr->egressPolicy == GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->qos.ingress.profileAssignIndex != dbLogicFormatPtr->qos.ingress.profileAssignIndex)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in qos.ingress.profileAssignIndex field \n");
            }
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionQosPrecedence==GT_FALSE)
    {
        if(logicFormatPtr->egressPolicy == GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->qos.ingress.profilePrecedence != dbLogicFormatPtr->qos.ingress.profilePrecedence)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in qos.ingress.profilePrecedence field \n");
            }
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionQoSProfile==GT_FALSE)
    {
        if(logicFormatPtr->egressPolicy == GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->qos.ingress.profileIndex != dbLogicFormatPtr->qos.ingress.profileIndex)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in qos.ingress.profileIndex field \n");
            }
        }
    }

    if (pclExpandedActionOriginPtr->overwriteExpandedActionQoSModifyDSCP==GT_FALSE)
    {
        if(logicFormatPtr->egressPolicy == GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->qos.ingress.modifyDscp != dbLogicFormatPtr->qos.ingress.modifyDscp)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in qos.ingress.modifyDscp field \n");
            }
        }
        else
        {
            /* check that the fields are equal */
            if (logicFormatPtr->qos.egress.modifyDscp != dbLogicFormatPtr->qos.egress.modifyDscp)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in qos.egress.modifyDscp field \n");
            }
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionUp0==GT_FALSE)
    {
        if(logicFormatPtr->egressPolicy == GT_TRUE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->qos.egress.up != dbLogicFormatPtr->qos.egress.up)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in qos.egress.up field \n");
            }
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionModifyUp0==GT_FALSE)
    {
        if(logicFormatPtr->egressPolicy == GT_TRUE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->qos.egress.modifyUp != dbLogicFormatPtr->qos.egress.modifyUp)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in qos.egress.modifyUp field \n");
            }
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionModifyUp1==GT_FALSE)
    {
        if(logicFormatPtr->egressPolicy == GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->qos.ingress.modifyUp != dbLogicFormatPtr->qos.ingress.modifyUp)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in qos.ingress.modifyUp field \n");
            }
        }
        else
        {
            /* check that the fields are equal */
            if (logicFormatPtr->qos.egress.up1ModifyEnable != dbLogicFormatPtr->qos.egress.up1ModifyEnable)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in qos.egress.up1ModifyEnable field \n");
            }
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionUp1Command==GT_FALSE)
    {
        if(logicFormatPtr->egressPolicy == GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->qos.ingress.up1Cmd != dbLogicFormatPtr->qos.ingress.up1Cmd)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in qos.ingress.up1Cmd field \n");
            }
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionUp1==GT_FALSE)
    {
        if(logicFormatPtr->egressPolicy == GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->qos.ingress.up1 != dbLogicFormatPtr->qos.ingress.up1)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in qos.ingress.up1 field \n");
            }
        }
        else
        {
            /* check that the fields are equal */
            if (logicFormatPtr->qos.egress.up1 != dbLogicFormatPtr->qos.egress.up1)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in qos.egress.up1 field \n");
            }
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionOamTimestampEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->oam.timeStampEnable != dbLogicFormatPtr->oam.timeStampEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in oam.timeStampEnable field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionOamOffsetIndex==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->oam.offsetIndex != dbLogicFormatPtr->oam.offsetIndex)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in oam.offsetIndex field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionOamProcessingEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->oam.oamProcessEnable != dbLogicFormatPtr->oam.oamProcessEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in oam.oamProcessEnable field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionOamProfile==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->oam.oamProfile != dbLogicFormatPtr->oam.oamProfile)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in oam.oamProfile field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionFlowId==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->flowId != dbLogicFormatPtr->flowId)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in flowId field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionSourcePortEnable==GT_FALSE)
    {
       /* check that the fields are equal */
        if (logicFormatPtr->sourcePort.assignSourcePortEnable != dbLogicFormatPtr->sourcePort.assignSourcePortEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in sourcePort.assignSourcePortEnable field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionSourcePort==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->sourcePort.sourcePortValue != dbLogicFormatPtr->sourcePort.sourcePortValue)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in sourcePort.sourcePortValue field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionLatencyMonitor==GT_FALSE)
    {
       /* check that the fields are equal */
        if (logicFormatPtr->latencyMonitor.latencyProfile != dbLogicFormatPtr->latencyMonitor.latencyProfile)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in latencyMonitor.latencyProfile field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionLatencyMonitorEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->latencyMonitor.monitoringEnable != dbLogicFormatPtr->latencyMonitor.monitoringEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in latencyMonitor.monitoringEnable field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionSkipFdbSaLookup==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->skipFdbSaLookup != dbLogicFormatPtr->skipFdbSaLookup)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in skipFdbSaLookup field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionTriggerInterrupt==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->triggerInterrupt != dbLogicFormatPtr->triggerInterrupt)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in triggerInterrupt field \n");
        }
    }

    if (pclExpandedActionOriginPtr->overwriteExpandedActionPhaMetadataAssignEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->epclPhaInfo.phaThreadType != dbLogicFormatPtr->epclPhaInfo.phaThreadType)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in epclPhaInfo.phaThreadType field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionPhaMetadata==GT_FALSE)
    {
        switch (logicFormatPtr->epclPhaInfo.phaThreadType)
        {
        case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_TYPE_NOT_USED_E:
            /* check that the fields are equal */
            if (logicFormatPtr->epclPhaInfo.phaThreadUnion.notNeeded != dbLogicFormatPtr->epclPhaInfo.phaThreadUnion.notNeeded)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in epclPhaInfo.phaThreadUnion.notNeeded field \n");
            }
            break;
        case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_IOAM_INGRESS_SWITCH_E:
            /* check that the fields are equal */
            if (logicFormatPtr->epclPhaInfo.phaThreadUnion.epclIoamIngressSwitch.ioamIncludesBothE2EOptionAndTraceOption != dbLogicFormatPtr->epclPhaInfo.phaThreadUnion.epclIoamIngressSwitch.ioamIncludesBothE2EOptionAndTraceOption)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in epclPhaInfo.phaThreadUnion.epclIoamIngressSwitch.ioamIncludesBothE2EOptionAndTraceOption field \n");
            }
            break;
        case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INT_IOAM_MIRROR_E:
            /* check that the fields are equal */
            if (logicFormatPtr->epclPhaInfo.phaThreadUnion.epclIntIoamMirror.erspanIII_FT != dbLogicFormatPtr->epclPhaInfo.phaThreadUnion.epclIntIoamMirror.erspanIII_FT)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in epclPhaInfo.phaThreadUnion.epclIntIoamMirror.erspanIII_FT field \n");
            }
            /* check that the fields are equal */
            if (logicFormatPtr->epclPhaInfo.phaThreadUnion.epclIntIoamMirror.erspanIII_P != dbLogicFormatPtr->epclPhaInfo.phaThreadUnion.epclIntIoamMirror.erspanIII_P)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in epclPhaInfo.phaThreadUnion.epclIntIoamMirror.erspanIII_P field \n");
            }
            break;
         case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_CLASSIFIER_NSH_OVER_VXLAN_GPE_E:
            /* check that the fields are equal */
            if (logicFormatPtr->epclPhaInfo.phaThreadUnion.epclClassifierNshOverVxlanGpe.classifierNshOverVxlanGpe_SourceClass != dbLogicFormatPtr->epclPhaInfo.phaThreadUnion.epclClassifierNshOverVxlanGpe.classifierNshOverVxlanGpe_SourceClass)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in epclPhaInfo.phaThreadUnion.epclClassifierNshOverVxlanGpe.classifierNshOverVxlanGpe_SourceClass field \n");
            }
            /* check that the fields are equal */
            if (logicFormatPtr->epclPhaInfo.phaThreadUnion.epclClassifierNshOverVxlanGpe.classifierNshOverVxlanGpe_TenantID != dbLogicFormatPtr->epclPhaInfo.phaThreadUnion.epclClassifierNshOverVxlanGpe.classifierNshOverVxlanGpe_TenantID)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in epclPhaInfo.phaThreadUnion.epclClassifierNshOverVxlanGpe.classifierNshOverVxlanGpe_TenantID field \n");
            }
            break;
        case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ERSPAN_TYPE_II_MIRROR_E:
            /* check that the fields are equal */
            if (logicFormatPtr->epclPhaInfo.phaThreadUnion.epclErspanTypeIImirror.erspanAnalyzerIndex != dbLogicFormatPtr->epclPhaInfo.phaThreadUnion.epclErspanTypeIImirror.erspanAnalyzerIndex)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in epclPhaInfo.phaThreadUnion.epclErspanTypeIImirror.erspanAnalyzerIndex field \n");
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "bad value for in epclPhaInfo.phaThreadType field \n");
        }
    }

    if (pclExpandedActionOriginPtr->overwriteExpandedActionPhaThreadNumberAssignEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->epclPhaInfo.phaThreadIdAssignmentMode != dbLogicFormatPtr->epclPhaInfo.phaThreadIdAssignmentMode)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in epclPhaInfo.phaThreadIdAssignmentMode field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionPhaThreadNumber==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->epclPhaInfo.phaThreadId != dbLogicFormatPtr->epclPhaInfo.phaThreadId)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in epclPhaInfo.phaThreadId field \n");
        }
    }

    if (pclExpandedActionOriginPtr->overwriteExpandedActionCutThroughTerminateId==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->terminateCutThroughMode != dbLogicFormatPtr->terminateCutThroughMode)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in terminateCutThroughMode field \n");
        }
    }

    if(logicFormatPtr->copyReserved.assignEnable==GT_TRUE)
    {
        if (pclExpandedActionOriginPtr->overwriteExpendedActionCopyReserved==GT_FALSE)
        {
            /* check that the fields are equal */
            if (logicFormatPtr->copyReserved.copyReserved != dbLogicFormatPtr->copyReserved.copyReserved )
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in copyReserved.copyReserved field \n");
            }
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionIpfixEnable==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->ipfixEnable != dbLogicFormatPtr->ipfixEnable)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in ipfixEnablefield \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionTriggerHashCncClient==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->triggerHashCncClient != dbLogicFormatPtr->triggerHashCncClient)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in triggerHashCncClient field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionEgressCncIndexMode==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->egressCncIndexMode != dbLogicFormatPtr->egressCncIndexMode)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in egressCncIndexMode field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionEnableEgressMaxSduSizeCheck==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->enableEgressMaxSduSizeCheck != dbLogicFormatPtr->enableEgressMaxSduSizeCheck)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in enableEgressMaxSduSizeCheck field \n");
        }
    }
    if (pclExpandedActionOriginPtr->overwriteExpandedActionEgressMaxSduSizeProfile==GT_FALSE)
    {
        /* check that the fields are equal */
        if (logicFormatPtr->egressMaxSduSizeProfile != dbLogicFormatPtr->egressMaxSduSizeProfile)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mismatch in egressMaxSduSizeProfile field \n");
        }
    }

    return rc;
}

/*
* @internal validateEntryFormatParams_exactMatchEntryPclFormat function
* @endinternal
*
* @brief  function to validate the 'API LEVEL' Exact Match entry PCL format parameters.
*         based on prvCpssDxChExactMatchPclActionValidity
*         NOTE: the 'key' was already checked
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match Manager.
* @param[in] entryPtr              - (pointer to) the entry (application format)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   validateEntryFormatParams_exactMatchEntryPclFormat
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                  *entryPtr
)
{
    GT_U32                                  rc = GT_OK;
    GT_U32                                  expandedActionIndex;

    expandedActionIndex = entryPtr->expandedActionIndex;
    rc = prvCpssDxChExactMatchManagerDbValidatePclAction(&(entryPtr->exactMatchAction.pclAction));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"Error in validation of PCL Action expandedActionParamsPtr->expandedAction, expandedActionIndex[%d] ",
                                      expandedActionIndex);
    }

    /* Check that the Exact Match PCL Action correspond with
       the PCL Action defined in the Profile Expander Table */
    rc = prvExactMatchManagerPclActionValidity(&(entryPtr->exactMatchAction.pclAction),
                                                &(exactMatchManagerPtr->exactMatchExpanderArray[expandedActionIndex].exactMatchExpandedEntry.expandedAction.pclAction),
                                                &(exactMatchManagerPtr->exactMatchExpanderArray[expandedActionIndex].exactMatchExpandedEntry.expandedActionOrigin.pclExpandedActionOrigin));

    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/*
* @internal prvCpssDxChExactMatchManagerValidateEntryFormatParams function
* @endinternal
*
* @brief  function to validate the 'API LEVEL' Exact Match parameters.
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match Manager.
* @param[in] entryPtr              - (pointer to) the entry (application format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   prvCpssDxChExactMatchManagerValidateEntryFormatParams
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                  *entryPtr
)
{
    /* check validity of expandedActionIndex */
    if (entryPtr->expandedActionIndex>=CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_ENTRIES_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                                       "expandedActionIndex[%d] is bigger the max legal value[%d]",
                                       entryPtr->expandedActionIndex);
    }
    /* first check that the Expanded Action pointed to by expandedActionIndex is valid */
    if(exactMatchManagerPtr->exactMatchExpanderArray[entryPtr->expandedActionIndex].exactMatchExpandedEntryValid==GT_FALSE)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                                       "expandedActionIndex[%d] is not a valid entry in the expanded table",
                                       entryPtr->expandedActionIndex);
    }
    else
    {
        /* if the expander entry is valid, we need to make sure it has the same actionType
           as the one we want to configure in the exact match entry rule */
        if (exactMatchManagerPtr->exactMatchExpanderArray[entryPtr->expandedActionIndex].exactMatchExpandedEntry.expandedActionType!=
            entryPtr->exactMatchActionType)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                                          "mismatch in exactMatchActionType:"
                                          "in the Exact Match Rule exactMatchActionType[%d] != "
                                          "exactMatchActionType[%d] of the Expanded entry in expandedActionIndex[%d]",
                                          entryPtr->exactMatchActionType,
                                          exactMatchManagerPtr->exactMatchExpanderArray[entryPtr->expandedActionIndex].exactMatchExpandedEntry.expandedActionType,
                                          entryPtr->expandedActionIndex);
        }
    }

    if (entryPtr->exactMatchUserDefined>=BIT_16)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                                       "exactMatchUserDefined[%d] is bigger than SUPPORTED VALUES : 0..65535 (16 bits).",
                                       entryPtr->exactMatchUserDefined);
    }
    /* validate the 'KEY' (hash related) parameters */
    switch(entryPtr->exactMatchEntry.key.keySize)
    {
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E:
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E:
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E:
            break;
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E:
            /* TTI keys in Falcon are 10B, 20B, 30B - 47B is not relevant */
            if (entryPtr->exactMatchActionType==CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                                       "entryPtr->exactMatchEntry.key.keySize[%d] is bigger than SUPPORTED:TTI keys in Falcon are 10B, 20B, 30B - 47B is not relevant.",
                                       entryPtr->exactMatchEntry.key.keySize);
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(entryPtr->exactMatchEntry.key.keySize);
    }

    switch(entryPtr->exactMatchEntry.lookupNum)
    {
        case CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E:
        case CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(entryPtr->exactMatchEntry.lookupNum);
    }

    /* validate the rest of the parameters */
    switch(entryPtr->exactMatchActionType)
    {
        case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
            return validateEntryFormatParams_exactMatchEntryTtiFormat(exactMatchManagerPtr, entryPtr);
        case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
        case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
            return validateEntryFormatParams_exactMatchEntryPclFormat(exactMatchManagerPtr, entryPtr);
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(entryPtr->exactMatchActionType);
    }
}

/*
* @internal prvCpssDxChExactMatchManagerValidateEntryKeyFormatParams function
* @endinternal
*
* @brief  function to validate the 'API LEVEL' Exact Match key parameters.
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match Manager.
* @param[in] entryPtr              - (pointer to) the entry (application format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   prvCpssDxChExactMatchManagerValidateEntryKeyFormatParams
(
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                  *entryPtr
)
{
     /* validate the 'KEY' (hash related) parameters */
    switch(entryPtr->exactMatchEntry.key.keySize)
    {
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E:
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E:
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E:
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(entryPtr->exactMatchEntry.key.keySize);
    }

    switch(entryPtr->exactMatchEntry.lookupNum)
    {
        case CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E:
        case CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(entryPtr->exactMatchEntry.lookupNum);
    }
    return GT_OK;
}

/*
* @internal prvCpssDxChExactMatchManagerCounterPacketTypeGet function
* @endinternal
*
* @brief  function to get counter type for the current packet type
*
* @param[in] exactMatchManagerPtr   - (pointer to) the Exact Match Manager
* @param[in] entryPtr               - (pointer to) the entry (application format) if NULL ignored
* @param[in] dbEntryPtr             - (pointer to) the manager entry format (manager format) if NULL ignored
* @param[out] counterActionTypePtr  - (pointer to) the counter action type for the current packet type
* @param[out] counterKeySizePtr     - (pointer to) the counter key size for the current packet type
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
*
*/
static GT_STATUS   prvCpssDxChExactMatchManagerCounterPacketTypeGet
(
    IN  PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                  *entryPtr,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC    *dbEntryPtr,
    OUT CPSS_DXCH_EXACT_MATCH_CLIENT_ENT                        *counterClientTypePtr,
    OUT CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT                      *counterKeySizePtr
)
{
    if(entryPtr) /* called from 'add' */
    {
        if (entryPtr->exactMatchEntry.lookupNum>CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "TTI illegal entryPtr->exactMatchEntry.lookupNum[%d] \n",entryPtr->exactMatchEntry.lookupNum);
        }
        *counterClientTypePtr = exactMatchManagerPtr->lookupInfo.lookupsArray[entryPtr->exactMatchEntry.lookupNum].lookupClient;
        *counterKeySizePtr = entryPtr->exactMatchEntry.key.keySize;
        /* sanity check */
        switch (entryPtr->exactMatchActionType)
        {
        case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
            if (*counterClientTypePtr!=CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "TTI mismatch actionType[%d] and clientType[%d] \n",entryPtr->exactMatchActionType,*counterClientTypePtr);
            }
            break;
        case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
            if ((*counterClientTypePtr!=CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_0_E)&&
                (*counterClientTypePtr!=CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_1_E)&&
                (*counterClientTypePtr!=CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_2_E))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "IPCL mismatch actionType[%d] and clientType[%d] \n",entryPtr->exactMatchActionType,*counterClientTypePtr);
            }
            break;
        case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
            if (*counterClientTypePtr!=CPSS_DXCH_EXACT_MATCH_CLIENT_EPCL_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "EPCL mismatch actionType[%d] and clientType[%d] \n",entryPtr->exactMatchActionType,*counterClientTypePtr);
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(entryPtr->exactMatchActionType);
            break;
        }
    }
    else         /* called from 'delete' */
    {
        *counterClientTypePtr = exactMatchManagerPtr->lookupInfo.lookupsArray[dbEntryPtr->hwExactMatchLookupNum].lookupClient;
        /* sanity check */
        switch (dbEntryPtr->hwExactMatchEntryType)
        {
        case 0:/* CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E */
            if (*counterClientTypePtr!=CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "TTI mismatch actionType[%d] and clientType[%d] \n",dbEntryPtr->hwExactMatchEntryType,*counterClientTypePtr);
            }
            break;
        case 1:/* CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E */
            if ((*counterClientTypePtr!=CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_0_E)&&
                (*counterClientTypePtr!=CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_1_E)&&
                (*counterClientTypePtr!=CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_2_E))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "IPCL mismatch actionType[%d] and clientType[%d] \n",dbEntryPtr->hwExactMatchEntryType,*counterClientTypePtr);
            }
            break;
        case 2:/* CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E */
            if (*counterClientTypePtr!=CPSS_DXCH_EXACT_MATCH_CLIENT_EPCL_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "EPCL mismatch actionType[%d] and clientType[%d] \n",dbEntryPtr->hwExactMatchEntryType,*counterClientTypePtr);
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(dbEntryPtr->hwExactMatchEntryType);
            break;
        }

        switch(dbEntryPtr->hwExactMatchKeySize)
        {
            case 0:
                *counterKeySizePtr = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
                break;
            case 1:
                *counterKeySizePtr = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;
                break;
            case 2:
                *counterKeySizePtr = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;
                break;
            case 3:
                *counterKeySizePtr = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(dbEntryPtr->hwExactMatchKeySize);
        }
    }

    return GT_OK;
}

/*
* @internal saveEntryFormatParamsToDb function
* @endinternal
*
* @brief  function to save values of the Exact Match entry into DB
*         copy values from application format to manager format
*
* @param[in] entryPtr               - (pointer to) the entry (application format)
* @param[out] dbEntryPtr            - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS   saveEntryFormatParamsToDb
(
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                  *entryPtr,
    OUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC   *dbEntryPtr
)
{
    GT_STATUS rc = GT_OK;

    /* set new entry to be as refreshed - meaning it wont be deleted in coming aging scan */
    dbEntryPtr->age = 1;

    switch(entryPtr->exactMatchEntry.key.keySize)
    {
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E:
            dbEntryPtr->hwExactMatchKeySize = 0;
            break;
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E:
            dbEntryPtr->hwExactMatchKeySize = 1;
            break;
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E:
            dbEntryPtr->hwExactMatchKeySize = 2;
            break;
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E:
            dbEntryPtr->hwExactMatchKeySize = 3;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(entryPtr->exactMatchEntry.key.keySize);
    }

    switch (entryPtr->exactMatchEntry.lookupNum)
    {
        case CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E:
            dbEntryPtr->hwExactMatchLookupNum = 0;
            break;
        case CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E:
            dbEntryPtr->hwExactMatchLookupNum = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(entryPtr->exactMatchEntry.lookupNum);
    }

    switch (entryPtr->exactMatchActionType)
    {
        case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
            dbEntryPtr->hwExactMatchEntryType = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_HW_ENTRY_TYPE_TTI_E;
            rc = saveEntryFormatParamsToDb_exactMatchEntryTtiFormat(&entryPtr->exactMatchAction.ttiAction,
                                                               &dbEntryPtr->specificFormat.prvTtiEntryFormat);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
            dbEntryPtr->hwExactMatchEntryType = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_HW_ENTRY_TYPE_PCL_E;
            rc = saveEntryFormatParamsToDb_exactMatchEntryPclFormat(&entryPtr->exactMatchAction.pclAction,
                                                               &dbEntryPtr->specificFormat.prvPclEntryFormat);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
            dbEntryPtr->hwExactMatchEntryType = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_HW_ENTRY_TYPE_EPCL_E;
            rc = saveEntryFormatParamsToDb_exactMatchEntryEpclFormat(&entryPtr->exactMatchAction.pclAction,
                                                               &dbEntryPtr->specificFormat.prvEpclEntryFormat);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(entryPtr->exactMatchActionType);
    }

    dbEntryPtr->exactMatchUserDefined = (GT_U16)entryPtr->exactMatchUserDefined;
    dbEntryPtr->hwExactMatchExpandedActionIndex = entryPtr->expandedActionIndex;
    cpssOsMemCpy(dbEntryPtr->pattern,entryPtr->exactMatchEntry.key.pattern,sizeof(dbEntryPtr->pattern));

    return GT_OK;
}

/*
* @internal prvCpssDxChExactMatchManagerAddEntry function
* @endinternal
*
* @brief   This function adds entry to CPSS Exact Match Manager's database and HW.
*          NOTE: the Exact Match manager will set the info to all the registered devices.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match Manager
* @param[in] entryPtr              - (pointer to) Exact Match entry format to be added
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
static GT_STATUS prvCpssDxChExactMatchManagerAddEntry
(
    IN  PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC           *exactMatchManagerPtr,
    IN  CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                          *entryPtr,
    IN  PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC         *calcInfoPtr,
    OUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_ENT   *addApiStatisticsPtr
)
{
    GT_STATUS rc,rc1;

    /********************************/
    /* get the counter packet type  */
    /********************************/
    rc = prvCpssDxChExactMatchManagerCounterPacketTypeGet(exactMatchManagerPtr,entryPtr, NULL, &calcInfoPtr->counterClientType,&calcInfoPtr->counterKeyType);
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
    rc = prvCpssDxChExactMatchManagerDbAddNewEntry(exactMatchManagerPtr, calcInfoPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /**************************************/
    /* save entry format values to the DB */
    /**************************************/
    rc = saveEntryFormatParamsToDb(entryPtr, calcInfoPtr->dbEntryPtr);
    if(rc != GT_OK)
    {
        /* need to revert what was done in prvCpssDxChExactMatchManagerDbAddNewEntry */
        rc1 = prvCpssDxChExactMatchManagerDbDeleteOldEntry(exactMatchManagerPtr, calcInfoPtr);
        if (rc1!=GT_OK)
        {
            return rc1;
        }
        return rc;
    }

    /*for error injection tests  ,simulate crash before hw write after sw DB write */
    rc = prvCpssSystemRecoveryErrorInjectionCheck (PRV_CPSS_SYSYEM_RECOVERY_ERROR_INJECTION_INDEX_EM_4_E);
    if(rc != GT_OK)
    {
        return rc;
    }

    /***************************************************************************/
    /* write the entry to HW of all registered devices                         */
    /***************************************************************************/
    rc = prvCpssDxChExactMatchManagerHwWriteByHwIndex(exactMatchManagerPtr, calcInfoPtr);
    if(rc != GT_OK)
    {
        /* need to revert what was done in prvCpssDxChExactMatchManagerDbAddNewEntry */
        rc1 = prvCpssDxChExactMatchManagerDbDeleteOldEntry(exactMatchManagerPtr, calcInfoPtr);
        if (rc1!=GT_OK)
        {
            return rc1;
        }

        *addApiStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_HW_UPDATE_E;
        return rc;
    }

    /* not failed ... so we can update the 'OK' part */
    *addApiStatisticsPtr =
        (calcInfoPtr->rehashStageId >= 5) ?
            PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_OK_REHASHING_STAGE_5_AND_ABOVE_E :
            PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_OK_REHASHING_STAGE_0_E + calcInfoPtr->rehashStageId;

    /*for error injection tests  ,simulate crash after hw entry write before  sw DB update  */
    rc = prvCpssSystemRecoveryErrorInjectionCheck (PRV_CPSS_SYSYEM_RECOVERY_ERROR_INJECTION_INDEX_EM_5_E);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* update DB with the correct number of exact match bounded to Expander index */
    if (exactMatchManagerPtr->exactMatchExpanderArray[calcInfoPtr->dbEntryPtr->hwExactMatchExpandedActionIndex].exactMatchExpandedEntryValid==GT_TRUE)
    {
        exactMatchManagerPtr->exactMatchExpanderArray[calcInfoPtr->dbEntryPtr->hwExactMatchExpandedActionIndex].exactMatchEntriesCounter++;
    }
    else
    {
        /* should never happen since this is check before we start adding the exact match
           entry to HW in prvCpssDxChExactMatchManagerValidateEntryFormatParams */
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "ERROR: exactMatchExpanderArray[%d] is not valid.\n",calcInfoPtr->dbEntryPtr->hwExactMatchExpandedActionIndex);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChExactMatchManagerUpdateEntry function
* @endinternal
*
* @brief   This function updates entry to CPSS Exact Match Manager's database and HW.
*          NOTE: the Exact Match manager will set the info to all the registered devices.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match Manager
* @param[in] entryPtr              - (pointer to) Exact Match entry format to be updated
* @param[out] calcInfoPtr          - (pointer to) the calculated hash and lookup results for updated entry
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
static GT_STATUS prvCpssDxChExactMatchManagerUpdateEntry
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC               *exactMatchManagerPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                              *entryPtr,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC             *calcInfoPtr,
    OUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_API_STATISTICS_ENT   *updateApiStatisticsPtr
)
{
    GT_STATUS rc;
    GT_U32    oldExpandedActionIndex;
    GT_U32    newExpandedActionIndex;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC oldDbEntry;/* used for reconstruct incase of fail */

    /* save old Expander index */
    oldExpandedActionIndex = calcInfoPtr->dbEntryPtr->hwExactMatchExpandedActionIndex;
    /* get new Expander index */
    newExpandedActionIndex = entryPtr->expandedActionIndex;

    /**************************************
      Update entry format values to the DB
    **************************************/
    cpssOsMemCpy(&oldDbEntry,calcInfoPtr->dbEntryPtr,sizeof(PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC));
    rc = saveEntryFormatParamsToDb(entryPtr, calcInfoPtr->dbEntryPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /****************************************************
      Write the entry to HW of all registered devices
    ****************************************************/
    rc = prvCpssDxChExactMatchManagerHwWriteByHwIndex(exactMatchManagerPtr, calcInfoPtr);
    if(rc != GT_OK)
    {
        /* need to reconstruct the DB before the change done in saveEntryFormatParamsToDb */
        cpssOsMemCpy(calcInfoPtr->dbEntryPtr,&oldDbEntry, sizeof(PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC));

        *updateApiStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_HW_UPDATE_E;
        return rc;
    }

    /* not failed ... so we can update the 'OK' part */
    *updateApiStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_API_STATISTICS_OK_E;

    /* update Expander index counters */
    if (oldExpandedActionIndex!=newExpandedActionIndex)
    {
        if ((exactMatchManagerPtr->exactMatchExpanderArray[oldExpandedActionIndex].exactMatchExpandedEntryValid==GT_TRUE)&&
            (exactMatchManagerPtr->exactMatchExpanderArray[oldExpandedActionIndex].exactMatchEntriesCounter!=0))
        {
             /* decrement old counter */
            exactMatchManagerPtr->exactMatchExpanderArray[oldExpandedActionIndex].exactMatchEntriesCounter--;
        }
        else
        {
            /* should never happen */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "exactMatchManagerPtr->exactMatchExpanderArray[%d] is not valid or can not be decremented \n",oldExpandedActionIndex);
        }

        if(exactMatchManagerPtr->exactMatchExpanderArray[newExpandedActionIndex].exactMatchExpandedEntryValid==GT_TRUE)
        {
            /* increment new counter */
            exactMatchManagerPtr->exactMatchExpanderArray[newExpandedActionIndex].exactMatchEntriesCounter++;
        }
        else
        {
            /* should never happen */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "exactMatchManagerPtr->exactMatchExpanderArray[%d] is not valid and can not be incremented\n",newExpandedActionIndex);
        }
    }

    return GT_OK;
}

void prvCpssDxChExactMatchManagerDebugPrintEnableSet(GT_U32 enable)
{
    PRV_SHARED_EMM_DIR_TTI_SRC_GLOBAL_VAR_GET(debugPrint) = (enable) ? GT_TRUE : GT_FALSE;
}

/**
* @internal prvCpssDxChExactMatchManagerEntryDeleteFromBank
*           function
* @endinternal
*
* @brief   This function delete entry from the CPSS Exact Match Manager's database and HW
*          from specific bank. All types of entries can be deleted using this API.
*          NOTE: the Exact Match manager will remove the info from all the registered devices.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
* @param[in] bankId                - bank id to delete.
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
static GT_STATUS prvCpssDxChExactMatchManagerEntryDeleteFromBank
(
    IN GT_U32                                           exactMatchManagerId,
    IN GT_U32                                           bankId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC          *entryPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_DELETE_API_STATISTICS_ENT deleteApiStatistics;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC            *exactMatchManagerPtr;

    EM_MANAGER_ID_CHECK(exactMatchManagerId); /* must be done here because used in 'API' statistics */

    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    /* call the main logic */
    rc = mainLogicEntryDelete(exactMatchManagerPtr, entryPtr, GT_TRUE, bankId,&deleteApiStatistics);

    return rc;
}

/*
* @internal prvCpssDxChExactMatchManagerDbRehashFullEntry function
* @endinternal
*
* @brief   This function re-hash index for the new entry that can't be added to HW
*          when all banks for the current hash index are occupied.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerPtr    (pointer to) the Exact Match manager.
* @param[in] entryPtr              - (pointer to) new Exact Match entry
* @param[in] calcInfoPtr           - (pointer to) the calculated hash and lookup results
* @param[out] addApiStatisticsPtr  - (pointer to) the type of API ok/error statistics
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on operation fail
*
*/
static GT_STATUS  prvCpssDxChExactMatchManagerDbRehashFullEntry
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC           *exactMatchManagerPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                          *entryPtr,
    INOUT   PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC    *calcInfoPtr,
    OUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_ENT  *addApiStatisticsPtr
)
{
    GT_STATUS  rc;
    PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_INFO_STC entryInfo;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC  calcInfo;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC currentEntry;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC *currentEntryPtr = &currentEntry;
    GT_U32 depth,i,numOfOperations,ii;
    GT_U32 sizeInBytes,bankStep,hwIndex,leftOperations;
    PRV_CPSS_DXCH_CUCKOO_DB_LOG_ARRAY_STC *cuckooDbLogPtr;
    GT_U32 exactMatchManagerId;

    cpssOsMemSet(&entryInfo,0,sizeof(PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_INFO_STC));
    cpssOsMemSet(&calcInfo,0,sizeof(PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC));

    /* get Excat Match manager id */
    EM_MANAGER_ID_GET_MAC(exactMatchManagerPtr,exactMatchManagerId);

    /* convert enum key size ro size in bytes */
    EM_CONV_KEY_ENUM_TO_SIZE_IN_BYTES_MAC(entryPtr->exactMatchEntry.key.keySize,sizeInBytes);

    /* get cuckoo max depth definition according to key size in bytes */
    rc = prvCpssDxChCuckooDbMaxDepthGet(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId,sizeInBytes,&depth);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* prepare entryInfo */
    entryInfo.bank = PRV_CPSS_DXCH_CUCKOO_INVALID_BANK_CNS;
    entryInfo.sizeInBytes = sizeInBytes;
    cpssOsMemCpy(entryInfo.data,entryPtr->exactMatchEntry.key.pattern,sizeof(GT_U8)*sizeInBytes);
    entryInfo.depth = depth;
    entryInfo.line = 0;

    /* Rehash Exact Match entry */
    rc = prvCpssDxChCuckooDbRehashEntry(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId,&entryInfo);

    /* Free place found in DB */
    if (rc == GT_OK)
    {
        /* update stages in cuckoo operation log */
        rc = prvCpssDxChCuckooDbLogUpdateStages(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId);
        if (rc != GT_OK)
        {
            goto exit_cleanly_lbl;

        }

        /* get pointer to cuckoo log operations */
        rc = prvCpssDxChCuckooDbLogPtrGet(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId,&numOfOperations,&cuckooDbLogPtr);
        if (rc != GT_OK)
        {
            goto exit_cleanly_lbl;

        }

        /* update HW */
        /* the operations in the log array are ordered from last stage untill new entry in stage 0.
           All operations except stage 0 are in couples: (add entry + clean entry) in the same stage.
           At stage 0 only single operation add new entry.
           Each add/clean operation is duplicated according to key sizeInBytes for each [bank][line].
           example for lines in operation log:                                                                                             .
           add operation for key sizeInBytes == 33:
           ----------------------------------------
           index 0: {data = "\016\000\003\000\016\016\000\000\016\003\000\000\003\016\000\000\016\003\000\016\000\000\003\016\016\003\000\000\016\000\016\003", '\000' <repeats 14 times>,
                    sizeInBytes = 33, stage = 3, line = 820, bank = 3, isFree = 0, isLock = 0, isFirst = 1}
           index 1: {data = "\016\000\003\000\016\016\000\000\016\003\000\000\003\016\000\000\016\003\000\016\000\000\003\016\016\003\000\000\016\000\016\003", '\000' <repeats 14 times>,
                    sizeInBytes = 33, stage = 3, line = 820, bank = 4, isFree = 0, isLock = 0, isFirst = 0}
           index 2: {data = "\016\000\003\000\016\016\000\000\016\003\000\000\003\016\000\000\016\003\000\016\000\000\003\016\016\003\000\000\016\000\016\003", '\000' <repeats 14 times>,
                    sizeInBytes = 33, stage = 3, line = 820, bank = 5, isFree = 0, isLock = 0, isFirst = 0}
           clean opeartion for key sizeInBytes == 33:
           ------------------------------------------
           index 3: {data = '\000' <repeats 46 times>, sizeInBytes = 33, stage = 3, line = 730, bank = 0, isFree = 1, isLock = 0, isFirst = 0}
           index 4: {data = '\000' <repeats 46 times>, sizeInBytes = 33, stage = 3, line = 730, bank = 1, isFree = 1, isLock = 0, isFirst = 0}
           index 5: {data = '\000' <repeats 46 times>, sizeInBytes = 33, stage = 3, line = 730, bank = 2, isFree = 1, isLock = 0, isFirst = 0}
        */
        for (i=0,leftOperations=numOfOperations; i<numOfOperations; i+=bankStep*2,leftOperations-=bankStep*2)
        {
            /* convert key size in bytes to bank step */
            EM_CONV_KEY_SIZE_IN_BYTES_TO_BANK_STEP_MAC(cuckooDbLogPtr[i].sizeInBytes,bankStep);

            /* last entry is new entry to add */
            if (leftOperations == bankStep)
            {
                /* choose selected bank according to cuckoo operation log;
                   calcInfoPtr->crcMultiHashArr is already calculated */
                calcInfoPtr->selectedBankId = cuckooDbLogPtr[i].bank;
                /* update stageId according to cuckoo operation log
                   for the statistics we need to return the highest
                   number of movements during the rehash*/
                calcInfoPtr->rehashStageId = cuckooDbLogPtr[0].stage;

                if (PRV_SHARED_EMM_DIR_TTI_SRC_GLOBAL_VAR_GET(debugPrint))
                {
                    switch(bankStep)
                    {
                    case 1:
                        EM_PRINT_CURRENT_5BYTES_BANK_STAGE(entryPtr, cuckooDbLogPtr[i].bank, cuckooDbLogPtr[i].line, cuckooDbLogPtr[i].stage);
                        break;
                    case 2:
                        EM_PRINT_CURRENT_19BYTES_BANK_STAGE(entryPtr, cuckooDbLogPtr[i].bank, cuckooDbLogPtr[i].line, cuckooDbLogPtr[i].stage);
                        break;
                    case 3:
                        EM_PRINT_CURRENT_33BYTES_BANK_STAGE(entryPtr, cuckooDbLogPtr[i].bank, cuckooDbLogPtr[i].line, cuckooDbLogPtr[i].stage);
                        break;
                    case 4:
                        EM_PRINT_CURRENT_47BYTES_BANK_STAGE(entryPtr, cuckooDbLogPtr[i].bank, cuckooDbLogPtr[i].line, cuckooDbLogPtr[i].stage);
                        break;
                    }
                }

                /* Add the last entry to free place */
                rc = prvCpssDxChExactMatchManagerAddEntry(exactMatchManagerPtr, entryPtr, calcInfoPtr, addApiStatisticsPtr);
                if (rc != GT_OK)
                {
                    goto exit_cleanly_lbl;
                }

                /* Add entry to cuckoo DB */
                rc = prvCpssDxChCuckooDbEntryAdd(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId,cuckooDbLogPtr[i].bank,cuckooDbLogPtr[i].line,
                                                 cuckooDbLogPtr[i].sizeInBytes,cuckooDbLogPtr[i].data,GT_FALSE);
                if (rc != GT_OK)
                {
                    goto exit_cleanly_lbl;
                }

                /* delete cuckoo operation log */
                rc = prvCpssDxChCuckooDbLogArrayDelete(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId);

                return rc;

            }
            /* need to move entries*/
            else
            {
                /* calculate hw Index from info received from cuckoo operation log */
                EM_HW_INDEX_GET_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes,
                                    cuckooDbLogPtr[i+bankStep].bank,cuckooDbLogPtr[i+bankStep].line,hwIndex);

                /* read Exact Match Entry from DB */
                rc = prvCpssDxChExactMatchManagerDbEntryReadByHwIndex(exactMatchManagerPtr,hwIndex,currentEntryPtr);
                if (rc != GT_OK)
                {
                    goto exit_cleanly_lbl;
                }

                /* calculate hash for Exact Match entry */
                rc = prvCpssDxChExactMatchManagerDbHashCalculate(exactMatchManagerPtr, currentEntryPtr, &calcInfo);
                if(rc != GT_OK)
                {
                    goto exit_cleanly_lbl;
                }

                /* choose selected bank according to cuckoo operation log */
                calcInfo.selectedBankId = cuckooDbLogPtr[i].bank;
                /* update stageId according to cuckoo operation log */
                calcInfo.rehashStageId = cuckooDbLogPtr[i].stage;

                if (PRV_SHARED_EMM_DIR_TTI_SRC_GLOBAL_VAR_GET(debugPrint))
                {
                    switch(bankStep)
                    {
                    case 1:
                        EM_PRINT_CURRENT_5BYTES_BANK_STAGE(currentEntryPtr, cuckooDbLogPtr[i].bank, cuckooDbLogPtr[i].line, cuckooDbLogPtr[i].stage);
                        break;
                    case 2:
                        EM_PRINT_CURRENT_19BYTES_BANK_STAGE(currentEntryPtr, cuckooDbLogPtr[i].bank, cuckooDbLogPtr[i].line, cuckooDbLogPtr[i].stage);
                        break;
                    case 3:
                        EM_PRINT_CURRENT_33BYTES_BANK_STAGE(currentEntryPtr, cuckooDbLogPtr[i].bank, cuckooDbLogPtr[i].line, cuckooDbLogPtr[i].stage);
                        break;
                    case 4:
                        EM_PRINT_CURRENT_47BYTES_BANK_STAGE(currentEntryPtr, cuckooDbLogPtr[i].bank, cuckooDbLogPtr[i].line, cuckooDbLogPtr[i].stage);
                        break;
                    }
                }

                /* Move entry to free place */
                rc = prvCpssDxChExactMatchManagerAddEntry(exactMatchManagerPtr, currentEntryPtr, &calcInfo, addApiStatisticsPtr);
                if (rc != GT_OK)
                {
                    goto exit_cleanly_lbl;
                }

                /* Add entry to cuckoo DB */
                rc = prvCpssDxChCuckooDbEntryAdd(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId,cuckooDbLogPtr[i].bank,cuckooDbLogPtr[i].line,
                                                 cuckooDbLogPtr[i].sizeInBytes,cuckooDbLogPtr[i].data,GT_FALSE);
                if (rc != GT_OK)
                {
                    goto exit_cleanly_lbl;
                }

                /* delete previous place; API will also delete entry from cuckoo DB */
                rc = prvCpssDxChExactMatchManagerEntryDeleteFromBank(exactMatchManagerId,cuckooDbLogPtr[i+bankStep].bank,currentEntryPtr);
                if (rc != GT_OK)
                {
                    goto exit_cleanly_lbl;
                }
            }
        }

    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL,
        "the entry not exists and not able to add it (trying 'Cuckoo') ");

exit_cleanly_lbl:

    /* delete cuckoo operation log */
    prvCpssDxChCuckooDbLogArrayDelete(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId);

    /* update cuckoo statistics counters according to exact match DB */
    prvCpssDxChCuckooDbSyncStatistics(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId);

    return rc;
}

/*
* @internal mainLogicEntryAdd function
* @endinternal
*
* @brief   This function adding entry to CPSS Exact Match Manager's database and HW.
*          All types of entries can be added using this API.
*          NOTE: the Exact Match manager will set the info to all the registered devices.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match Manager.
* @param[in] entryPtr              - (pointer to) Exact Match entry format to be added.
* @param[in] paramsPtr             - (pointer to) extra info related to 'add entry' operation.
* @param[out] addApiStatisticsPtr  - (pointer to) the type of API ok/error statistics
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
static GT_STATUS mainLogicEntryAdd
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC           *exactMatchManagerPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                          *entryPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC               *paramsPtr,
    OUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_ENT  *addApiStatisticsPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC calcInfo;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC *calcInfoPtr = &calcInfo;
    GT_U32 line,sizeInBytes,exactMatchManagerId,ii;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;
    GT_BOOL                       managerHwWriteBlock;

    cpssOsMemSet(&calcInfo, 0, sizeof(PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC));

    *addApiStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_INPUT_INVALID_E;
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);
    CPSS_NULL_PTR_CHECK_MAC(paramsPtr);

    *addApiStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS___LAST___E;

    /* get Excat Match manager id */
    EM_MANAGER_ID_GET_MAC(exactMatchManagerPtr,exactMatchManagerId);

    if(!exactMatchManagerPtr->headOfFreeList.isValid_entryPointer)
    {
        rc = prvCpssDxChExactMatchManagerDbTableFullDiagnostic(exactMatchManagerPtr);

        if(rc == GT_FULL)
        {
            *addApiStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_TABLE_FULL_E;
            return rc;
        }
        else
        {
            /* unexpected error : bad state */
            *addApiStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_BAD_STATE_E;
            return rc;
        }
    }

    /********************************/
    /* validity checks of SW format */
    /********************************/
    rc = prvCpssDxChExactMatchManagerValidateEntryFormatParams(exactMatchManagerPtr,entryPtr);
    if(rc != GT_OK)
    {
        *addApiStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_INPUT_INVALID_E;
        return rc;
    }

    /*************************************/
    /* calculate where to save in the DB */
    /*************************************/
    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_EXACT_MATCH_MANAGER_E);
    rc = prvCpssDxChExactMatchManagerDbCalcHashAndLookupResult(exactMatchManagerPtr,entryPtr,paramsPtr,&calcInfo);
    if(rc != GT_OK)
    {
        if((rc==GT_NOT_FOUND)&&
           (((tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E)&&
            (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E))||(managerHwWriteBlock == GT_TRUE)) &&
           (paramsPtr!=NULL))
        {
            *addApiStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_REPLAY_ENTRY_NOT_FOUND_E;
            return rc;
        }
        else
        {
            /* should not fail ! so if failed this is bad state */
            *addApiStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_BAD_STATE_E;
            return rc;
        }
    }

    if (((tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E) &&
        (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)) || managerHwWriteBlock == GT_TRUE )
    {
        if (calcInfo.calcEntryType != PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_FREE_E)
        {
            if (calcInfo.calcEntryType == PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_EXISTS_E)
            {
                /*  if we are in HA process and we get here it means that the entry was already
                    replayed to the DB.
                    nothing more to do - all DB and HW are updated correctly
                    return GT_ALREADY_EXIST */
                *addApiStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_ENTRY_EXIST_E;
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST,
                "the entry already exists : at hwIndex[%d] (for 'update' entry use other API!) ",
                calcInfo.dbEntryPtr->hwIndex);
            }
            else
            {
                /* in HA we can not get here, it means some kind of fail since the entry should be in the HW
                    and we should have found the proper index for it, and the SW place is free */
                *addApiStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_BAD_STATE_E;
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }
    }
    else
    {
        if (calcInfo.calcEntryType == PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CALC_ENTRY_TYPE_NOT_FOUND_AND_NO_FREE_E)
        {
            if (paramsPtr->rehashEnable)
            {
                if (PRV_SHARED_EMM_DIR_TTI_SRC_GLOBAL_VAR_GET(debugPrint))
                {
                    cpssOsPrintf("-------------------------Original Start---------------------------------\n");

                    /* calc line from hwIndex */
                    EM_LINE_FROM_HW_INDEX_GET_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes,
                                                  calcInfoPtr->crcMultiHashArr[calcInfoPtr->rehashStageBank],line);

                    switch(calcInfoPtr->bankStep)
                    {
                    case 1:
                        EM_PRINT_CURRENT_5BYTES_BANK_STAGE(entryPtr, calcInfoPtr->rehashStageBank, line, 0);
                        break;
                    case 2:
                        EM_PRINT_CURRENT_19BYTES_BANK_STAGE(entryPtr, calcInfoPtr->rehashStageBank, line, 0);
                        break;
                    case 3:
                        EM_PRINT_CURRENT_33BYTES_BANK_STAGE(entryPtr, calcInfoPtr->rehashStageBank, line, 0);
                        break;
                    case 4:
                        EM_PRINT_CURRENT_47BYTES_BANK_STAGE(entryPtr, calcInfoPtr->rehashStageBank, line, 0);
                        break;
                    }

                    EM_PRINT_BANKS_ENTRY_PARAMS(exactMatchManagerPtr,calcInfoPtr);
                    cpssOsPrintf("-------------------------Original End-----------------------------------\n");
                }

                rc = prvCpssDxChExactMatchManagerDbRehashFullEntry(exactMatchManagerPtr, entryPtr, &calcInfo, addApiStatisticsPtr);
                if(rc != GT_OK)
                {
                    if (rc == GT_FULL)
                    {
                        *addApiStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_TABLE_FULL_E;
                    }
                    else
                    {
                        /* should not fail ! so if failed this is bad state */
                        *addApiStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_BAD_STATE_E;

                        /* delete cuckoo operation log */
                        rc = prvCpssDxChCuckooDbLogArrayDelete(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId);
                        if(rc != GT_OK)
                        {
                            return rc;
                        }
                    }
                    return rc;
                }
                return GT_OK;
            }
        }

        if (calcInfo.calcEntryType == PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_EXISTS_E)
        {
            *addApiStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_ENTRY_EXIST_E;
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST,
                "the entry already exists : at hwIndex[%d] (for 'update' entry use other API!) ",
                calcInfo.dbEntryPtr->hwIndex);
        }
        else
        if(calcInfo.calcEntryType == PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CALC_ENTRY_TYPE_NOT_FOUND_AND_NO_FREE_E)
        {
            *addApiStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_TABLE_FULL_E;
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL,
                "the entry not exists and not able to add it (before trying 'Cuckoo') ");
        }
    }
    /* from this point ... should not fail ! so if failed this is bad state */
    *addApiStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_BAD_STATE_E;

    /*********************************************/
    /* choose the best bank to use for the entry */
    /*********************************************/

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_EXACT_MATCH_MANAGER_E);
    if (!((tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E) &&
         (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)) && managerHwWriteBlock == GT_FALSE)
    {
        /* For HA the bank we have in calcInfo is already the correct one we need to set.
           no need to find the most populated */

        /* use free entry at the most populated bank (or Cuckoo if none). */
        rc = prvCpssDxChExactMatchManagerDbGetMostPopulatedBankId(exactMatchManagerPtr,&calcInfo);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /******************************************************************************/
    /* adding new entry to DB and HW                                              */
    /******************************************************************************/
    rc = prvCpssDxChExactMatchManagerAddEntry(exactMatchManagerPtr, entryPtr, &calcInfo, addApiStatisticsPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /******************************************************************************/
    /* adding new entry to cuckoo DB                                              */
    /******************************************************************************/

    /* convert enum key size to size in bytes */
    EM_CONV_KEY_ENUM_TO_SIZE_IN_BYTES_MAC(entryPtr->exactMatchEntry.key.keySize,sizeInBytes);

    /* calc line from hwIndex */
    EM_LINE_FROM_HW_INDEX_GET_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes,
                                  calcInfo.crcMultiHashArr[calcInfo.selectedBankId],line);

    rc = prvCpssDxChCuckooDbEntryAdd(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E, exactMatchManagerId, calcInfo.selectedBankId,line,sizeInBytes,entryPtr->exactMatchEntry.key.pattern,GT_TRUE);
    if (rc!=GT_OK)
    {
        return rc;
    }

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_EXACT_MATCH_MANAGER_E);
    if (((tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E) &&
        (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)) || managerHwWriteBlock == GT_TRUE )
    {
        /* Sync of HW to SW pass ok - need to set the index in the tempDb as taken */
        rc = prvCpssDxChExactMatchManagerDbSetIndexInHwTempDbAsTaken(exactMatchManagerPtr,&calcInfo);
        if (rc!=GT_OK)
        {
            return rc;
        }
    }
    return rc;
}

/**
* @internal internal_cpssDxChExactMatchManagerEntryAdd function
* @endinternal
*
* @brief   This function adding entry to CPSS Exact Match Manager's database and HW.
*          All types of entries can be added using this API.
*          NOTE: the Exact Match manager will set the info to all the registered devices.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
static GT_STATUS internal_cpssDxChExactMatchManagerEntryAdd
(
    IN GT_U32                                               exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC              *entryPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC   *paramsPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_ENT  addApiStatistics;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC          *exactMatchManagerPtr;

    EM_MANAGER_ID_CHECK(exactMatchManagerId); /* must be done here because used in 'API' statistics */

    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    /* call the main logic */
    rc = mainLogicEntryAdd(exactMatchManagerPtr,entryPtr,paramsPtr,&addApiStatistics);

    if(addApiStatistics <
        PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS___LAST___E)
    {
        /* increment the 'API statistics' */
        exactMatchManagerPtr->apiEntryAddStatisticsArr[addApiStatistics]++;
    }

    return rc;
}

/**
* @internal cpssDxChExactMatchManagerEntryAdd function
* @endinternal
*
* @brief   This function adding entry to CPSS Exact Match Manager's database and HW.
*          All types of entries can be added using this API.
*          NOTE: the Exact Match manager will set the info to all the registered devices.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
GT_STATUS cpssDxChExactMatchManagerEntryAdd
(
    IN GT_U32                                               exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC              *entryPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC   *paramsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchManagerEntryAdd);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, exactMatchManagerId,entryPtr,paramsPtr));

    rc = internal_cpssDxChExactMatchManagerEntryAdd(exactMatchManagerId,entryPtr,paramsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, exactMatchManagerId,entryPtr,paramsPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);

    return rc;
}

/*
* @internal restoreEntryFormatParamsFromDb_exactMatchEntryTtiFormat_redirectConfig function
* @endinternal
*
* @brief  function to restore retirectPtr values of the Exact Match entry from DB into format
*         PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_TTI_FORMAT_STC
*         copy values from manager format to application format
*
* @param[in] dbEntryPtr            - (pointer to) the manager entry format (manager format)
* @param[out] entryPtr             - (pointer to) the entry (application format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval none
*/
static GT_STATUS restoreEntryFormatParamsFromDb_exactMatchEntryTtiFormat_redirectConfig
(
    IN  PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_TTI_FORMAT_STC   *dbEntryPtr,
    OUT CPSS_DXCH_TTI_ACTION_STC                                    *entryPtr
)
{
    /*treat redirectPtr field as:
        routerLttPtr when Redirect_Command="IPNextHop"
        vrfId when Redirect_Command="Assign_VRF_ID"
        flowId when Redirect_Command="NoRedirect"
        TS_ptr when Redirect_Command="Egress_Interface" and TunnelStart="True"
        ARP_ptr when Redirect_Command="Egress_Interface" and TunnelStart="false" */

    entryPtr->redirectCommand = dbEntryPtr->redirectCommand;

    /* save to application format from DB format */
    switch(entryPtr->redirectCommand)
    {
        case CPSS_DXCH_TTI_NO_REDIRECT_E:
            entryPtr->flowId=dbEntryPtr->redirectPtr;
            break;
        case CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E:
            entryPtr->egressInterface.type = dbEntryPtr->egressInterface_type;
            switch(entryPtr->egressInterface.type)
            {
                case CPSS_INTERFACE_PORT_E:
                    entryPtr->egressInterface.devPort.hwDevNum=dbEntryPtr->egressInterface.devPort.hwDevNum;
                    entryPtr->egressInterface.devPort.portNum=dbEntryPtr->egressInterface.devPort.portNum;
                    PRV_CPSS_DXCH_DUAL_HW_DEVICE_CONVERT_INTERFACE_MAC(&(entryPtr->egressInterface));
                    break;
                case CPSS_INTERFACE_TRUNK_E:
                    entryPtr->egressInterface.trunkId=dbEntryPtr->egressInterface.trunkId;
                    break;
                case CPSS_INTERFACE_VIDX_E:
                    entryPtr->egressInterface.vidx=dbEntryPtr->egressInterface.vidx;
                    break;
                default:
                    break;
            }

            entryPtr->tunnelStart=dbEntryPtr->tunnelStart;
            if (entryPtr->tunnelStart==GT_TRUE)
            {
                 entryPtr->tunnelStartPtr=dbEntryPtr->redirectPtr;
            }
            else
            {
                 entryPtr->arpPtr=dbEntryPtr->redirectPtr;
            }
            break;
        case CPSS_DXCH_TTI_REDIRECT_TO_ROUTER_LOOKUP_E:
            entryPtr->routerLttPtr=dbEntryPtr->redirectPtr;
            break;
        case CPSS_DXCH_TTI_VRF_ID_ASSIGN_E:
            entryPtr->vrfId=dbEntryPtr->redirectPtr;
            break;
        default:
            break;
    }
    return GT_OK;
}

/*
* @internal restoreEntryFormatParamsFromDb_exactMatchEntryPclFormat_redirectConfig function
* @endinternal
*
* @brief  function to restore retirectPtr values of the Exact Match entry from DB into format
*         PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_PCL_FORMAT_STC
*         copy values from manager format to application format
*
* @param[in] dbEntryPtr            - (pointer to) the manager entry format (manager format)
* @param[out] entryPtr             - (pointer to) the entry (application format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval none
*/
static GT_STATUS restoreEntryFormatParamsFromDb_exactMatchEntryPclFormat_redirectConfig
(
    IN  PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_PCL_FORMAT_STC   *dbEntryPtr,
    OUT CPSS_DXCH_PCL_ACTION_STC                                    *entryPtr
)
{
    /*treat redirectPtr field as:
        routerLttPtr when Redirect_Command="IPNextHop"
        vrfId when Redirect_Command="Assign_VRF_ID"
        TS_ptr when Redirect_Command="Egress_Interface" and TunnelStart="True"
        ARP_ptr when Redirect_Command="Egress_Interface" and TunnelStart="false" */

    /* save to DB format from application format  */
    entryPtr->redirect.redirectCmd = dbEntryPtr->redirectCmd;
    switch(entryPtr->redirect.redirectCmd)
    {
        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E:
            break;
        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E:
            entryPtr->redirect.data.outIf.outInterface.type=dbEntryPtr->outInterface_type;
            switch(entryPtr->redirect.data.outIf.outInterface.type)
            {
                case CPSS_INTERFACE_PORT_E:
                    entryPtr->redirect.data.outIf.outInterface.devPort.hwDevNum=dbEntryPtr->outInterface.devPort.hwDevNum;
                    entryPtr->redirect.data.outIf.outInterface.devPort.portNum=dbEntryPtr->outInterface.devPort.portNum;
                    break;
                case CPSS_INTERFACE_TRUNK_E:
                    entryPtr->redirect.data.outIf.outInterface.trunkId=dbEntryPtr->outInterface.trunkId;
                    break;
                case CPSS_INTERFACE_VIDX_E:
                    entryPtr->redirect.data.outIf.outInterface.vidx=dbEntryPtr->outInterface.vidx;
                    break;
                default:
                    break;
            }

            entryPtr->redirect.data.outIf.tunnelStart=dbEntryPtr->tunnelStart;
            if (entryPtr->redirect.data.outIf.tunnelStart==GT_TRUE)
            {
                entryPtr->redirect.data.outIf.tunnelPtr= dbEntryPtr->redirectPtr;

            }
            else
            {
                 entryPtr->redirect.data.outIf.arpPtr=dbEntryPtr->redirectPtr;

            }

            entryPtr->redirect.data.outIf.vntL2Echo=dbEntryPtr->vntL2Echo;
            entryPtr->redirect.data.outIf.tunnelType=dbEntryPtr->tunnelType;
            entryPtr->redirect.data.outIf.modifyMacDa=dbEntryPtr->modifyMacDa;
            entryPtr->redirect.data.outIf.modifyMacSa=dbEntryPtr->modifyMacSa;

            break;
        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_E:
            entryPtr->redirect.data.routerLttIndex = PRV_CPSS_DXCH_SIP6_CONVERT_LEAF_HW_INDEX_TO_LEAF_SW_INDEX_MAC(dbEntryPtr->redirectPtr);
            break;
        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E:
            entryPtr->redirect.data.vrfId = dbEntryPtr->redirectPtr;
            break;
        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E:
            entryPtr->redirect.data.modifyMacSa.arpPtr=dbEntryPtr->redirectPtr;

            entryPtr->redirect.data.modifyMacSa.macSa.arEther[0] = (GT_U8)(dbEntryPtr->macSa_high_16 >>  8);
            entryPtr->redirect.data.modifyMacSa.macSa.arEther[1] = (GT_U8)(dbEntryPtr->macSa_high_16 >>  0);
            entryPtr->redirect.data.modifyMacSa.macSa.arEther[2] = (GT_U8)(dbEntryPtr->macSa_low_32  >> 24);
            entryPtr->redirect.data.modifyMacSa.macSa.arEther[3] = (GT_U8)(dbEntryPtr->macSa_low_32  >> 16);
            entryPtr->redirect.data.modifyMacSa.macSa.arEther[4] = (GT_U8)(dbEntryPtr->macSa_low_32  >>  8);
            entryPtr->redirect.data.modifyMacSa.macSa.arEther[5] = (GT_U8)(dbEntryPtr->macSa_low_32  >>  0);
            break;
        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_AND_ASSIGN_VRF_ID_E:
            entryPtr->redirect.data.routeAndVrfId.routerLttIndex = PRV_CPSS_DXCH_SIP6_CONVERT_LEAF_HW_INDEX_TO_LEAF_SW_INDEX_MAC(dbEntryPtr->redirectPtr);
            entryPtr->redirect.data.routeAndVrfId.vrfId = dbEntryPtr->vrfId;
            break;
        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ECMP_E:
            entryPtr->redirect.data.routerLttIndex = dbEntryPtr->redirectPtr; /* ecmp table Index */
            break;
        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ECMP_AND_ASSIGN_VRF_ID_E:
            entryPtr->redirect.data.routeAndVrfId.routerLttIndex = dbEntryPtr->redirectPtr; /* ecmp table Index */
            entryPtr->redirect.data.routeAndVrfId.vrfId = dbEntryPtr->vrfId;
            break;
        default:
            break;
    }
    return GT_OK;
}

/*
* @internal restoreEntryFormatParamsFromDb_exactMatchEntryTtiFormat function
* @endinternal
*
* @brief  function to restore values of the Exact Match entry from DB in format
*         CPSS_DXCH_TTI_ACTION_STC
*         copy values from manager format to application format
*
* @param[in]  dbEntryPtr            - (pointer to) the manager entry format (manager format)
* @param[out] entryPtr               - (pointer to) the entry (application format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval none
*/
static GT_STATUS restoreEntryFormatParamsFromDb_exactMatchEntryTtiFormat
(
    IN  PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_TTI_FORMAT_STC   *dbEntryPtr,
    OUT CPSS_DXCH_TTI_ACTION_STC                                    *entryPtr
)
{
    GT_STATUS rc = GT_OK;
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT    dsaCpuCode;

    /* save to application format from DB format */
    entryPtr->tunnelTerminate = dbEntryPtr->tunnelTerminate;
    entryPtr->tsPassengerPacketType = dbEntryPtr->tsPassengerPacketType;
    entryPtr->ttHeaderLength = dbEntryPtr->ttHeaderLength;
    entryPtr->tunnelHeaderLengthAnchorType = dbEntryPtr->tunnelHeaderLengthAnchorType;
    entryPtr->continueToNextTtiLookup = dbEntryPtr->continueToNextTtiLookup;
    entryPtr->copyTtlExpFromTunnelHeader = dbEntryPtr->copyTtlExpFromTunnelHeader;
    entryPtr->mplsCommand = dbEntryPtr->mplsCommand;
    entryPtr->mplsTtl = dbEntryPtr->mplsTtl;
    entryPtr->enableDecrementTtl = dbEntryPtr->enableDecrementTtl;
    entryPtr->passengerParsingOfTransitMplsTunnelMode = dbEntryPtr->passengerParsingOfTransitMplsTunnelMode;
    entryPtr->passengerParsingOfTransitNonMplsTransitTunnelEnable = dbEntryPtr->passengerParsingOfTransitNonMplsTransitTunnelEnable;

    entryPtr->redirectCommand = dbEntryPtr->redirectCommand;
    rc = restoreEntryFormatParamsFromDb_exactMatchEntryTtiFormat_redirectConfig(dbEntryPtr,entryPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    entryPtr->vntl2Echo = dbEntryPtr->vntl2Echo;
    entryPtr->bridgeBypass = dbEntryPtr->bridgeBypass;
    entryPtr->tag0VlanCmd = dbEntryPtr->tag0VlanCmd;
    entryPtr->tag0VlanId = (GT_U16)dbEntryPtr->tag0VlanId;
    switch (dbEntryPtr->tag1VlanCmd)
    {
    case 0:
        entryPtr->tag1VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
        break;
    case 1:
        entryPtr->tag1VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    entryPtr->tag1VlanId = (GT_U16)dbEntryPtr->tag1VlanId;
    entryPtr->tag0VlanPrecedence = dbEntryPtr->tag0VlanPrecedence;
    entryPtr->nestedVlanEnable = dbEntryPtr->nestedVlanEnable;
    entryPtr->actionStop = dbEntryPtr->actionStop;
    entryPtr->bindToPolicerMeter  = dbEntryPtr->bindToPolicerMeter;
    entryPtr->bindToPolicer  = dbEntryPtr->bindToPolicer;
    entryPtr->policerIndex = dbEntryPtr->policerIndex;
    entryPtr->qosPrecedence = dbEntryPtr->qosPrecedence;
    entryPtr->keepPreviousQoS = dbEntryPtr->keepPreviousQoS;
    entryPtr->trustUp = dbEntryPtr->trustUp;
    entryPtr->trustDscp = dbEntryPtr->trustDscp;
    entryPtr->trustExp = dbEntryPtr->trustExp;
    entryPtr->qosProfile = dbEntryPtr->qosProfile;
    entryPtr->modifyTag0Up = dbEntryPtr->modifyTag0Up;
    entryPtr->tag1UpCommand = dbEntryPtr->tag1UpCommand;
    entryPtr->modifyDscp = dbEntryPtr->modifyDscp;
    entryPtr->tag0Up = dbEntryPtr->tag0Up;
    entryPtr->tag1Up = dbEntryPtr->tag1Up;
    entryPtr->remapDSCP = dbEntryPtr->remapDSCP;
    entryPtr->qosUseUpAsIndexEnable = dbEntryPtr->qosUseUpAsIndexEnable;
    entryPtr->qosMappingTableIndex = dbEntryPtr->qosMappingTableIndex;
    entryPtr->mplsLLspQoSProfileEnable = dbEntryPtr->mplsLLspQoSProfileEnable;
    entryPtr->pcl0OverrideConfigIndex = dbEntryPtr->pcl0OverrideConfigIndex;
    entryPtr->pcl0_1OverrideConfigIndex = dbEntryPtr->pcl0_1OverrideConfigIndex;
    entryPtr->pcl1OverrideConfigIndex  = dbEntryPtr->pcl1OverrideConfigIndex;
    entryPtr->iPclConfigIndex = dbEntryPtr->iPclConfigIndex;

    /* deal with NON- 1:1 field values */

    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(entryPtr->command, dbEntryPtr->command);

    if (entryPtr->command!=CPSS_PACKET_CMD_FORWARD_E)
    {
        dsaCpuCode = dbEntryPtr->userDefinedCpuCode;
        rc = prvCpssDxChNetIfDsaToCpuCode(dsaCpuCode,
                                          &entryPtr->userDefinedCpuCode);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        entryPtr->userDefinedCpuCode = 0;/* CPSS_NET_UNDEFINED_CPU_CODE_E */
    }

    switch (dbEntryPtr->ttPassengerPacketType)
    {
    case 0:
        entryPtr->ttPassengerPacketType = CPSS_DXCH_TTI_PASSENGER_IPV4V6_E;
        break;
    case 1:
        entryPtr->ttPassengerPacketType = CPSS_DXCH_TTI_PASSENGER_MPLS_E;
        break;
    case 2:
        /* not supported for sip6 */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "CPSS_DXCH_TTI_PASSENGER_ETHERNET_CRC_E not supported for sip6. "
                                                    "fail in saveEntryFormatParamsToDb_exactMatchEntryTtiFormat");
        /*entryPtr->ttPassengerPacketType = CPSS_DXCH_TTI_PASSENGER_ETHERNET_CRC_E;*/
        break;
    case 3:
        entryPtr->ttPassengerPacketType = CPSS_DXCH_TTI_PASSENGER_ETHERNET_NO_CRC_E;
        break;
    default:
        /* if not TT then TT Passenger Type is not relevant */
        if(dbEntryPtr->tunnelTerminate == 0)/* GT_FALSE */
        {
            entryPtr->ttPassengerPacketType = CPSS_DXCH_TTI_PASSENGER_IPV4_E;
            break;
        }
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ttPassengerPacketType not supported "
                                                    "fail in restoreEntryFormatParamsFromDb_exactMatchEntryTtiFormat");
    }

    if (dbEntryPtr->iPclUdbConfigTableIndex == 0)
    {
        entryPtr->iPclUdbConfigTableEnable = GT_FALSE;
        entryPtr->iPclUdbConfigTableIndex = 0;
    }
    else
    {
        entryPtr->iPclUdbConfigTableEnable = GT_TRUE;
        switch (dbEntryPtr->iPclUdbConfigTableIndex)
        {
            case 1:
                entryPtr->iPclUdbConfigTableIndex = CPSS_DXCH_PCL_PACKET_TYPE_UDE_E;
                break;
            case 2:
                entryPtr->iPclUdbConfigTableIndex = CPSS_DXCH_PCL_PACKET_TYPE_UDE1_E;
                break;
            case 3:
                entryPtr->iPclUdbConfigTableIndex = CPSS_DXCH_PCL_PACKET_TYPE_UDE2_E;
                break;
            case 4:
                entryPtr->iPclUdbConfigTableIndex = CPSS_DXCH_PCL_PACKET_TYPE_UDE3_E;
                break;
            case 5:
                entryPtr->iPclUdbConfigTableIndex = CPSS_DXCH_PCL_PACKET_TYPE_UDE4_E;
                break;
            case 6:
                entryPtr->iPclUdbConfigTableIndex = CPSS_DXCH_PCL_PACKET_TYPE_UDE5_E;
                break;
            case 7:
                entryPtr->iPclUdbConfigTableIndex = CPSS_DXCH_PCL_PACKET_TYPE_UDE6_E;
                break;
            default:
                /* iPclUdbConfigTableIndex is 3 bits --> max value is 7*/
                break;
        }
    }
    /* end to deal with NON- 1:1 field values */

    entryPtr->sourceEPortAssignmentEnable = dbEntryPtr->sourceEPortAssignmentEnable;
    entryPtr->sourceEPort = dbEntryPtr->sourceEPort;
    entryPtr->bindToCentralCounter = dbEntryPtr->bindToCentralCounter;
    entryPtr->centralCounterIndex = dbEntryPtr->centralCounterIndex;
    entryPtr->modifyMacSa = dbEntryPtr->modifyMacSa;
    entryPtr->modifyMacDa = dbEntryPtr->modifyMacDa;
    entryPtr->hashMaskIndex = dbEntryPtr->hashMaskIndex;
    entryPtr->setMacToMe = dbEntryPtr->setMacToMe;
    entryPtr->rxProtectionSwitchEnable = dbEntryPtr->rxProtectionSwitchEnable;
    entryPtr->rxIsProtectionPath = dbEntryPtr->rxIsProtectionPath;
    entryPtr->pwTagMode = dbEntryPtr->pwTagMode;
    entryPtr->oamTimeStampEnable = dbEntryPtr->oamTimeStampEnable;
    entryPtr->oamOffsetIndex = dbEntryPtr->oamOffsetIndex;
    entryPtr->oamProcessEnable = dbEntryPtr->oamProcessEnable;
    entryPtr->oamProcessWhenGalOrOalExistsEnable = dbEntryPtr->oamProcessWhenGalOrOalExistsEnable;
    entryPtr->oamProfile = dbEntryPtr->oamProfile;
    entryPtr->oamChannelTypeToOpcodeMappingEnable = dbEntryPtr->oamChannelTypeToOpcodeMappingEnable;
    entryPtr->isPtpPacket = dbEntryPtr->isPtpPacket;
    entryPtr->ptpTriggerType = dbEntryPtr->ptpTriggerType;
    entryPtr->ptpOffset = dbEntryPtr->ptpOffset;
    entryPtr->ipv6SegmentRoutingEndNodeEnable = dbEntryPtr->ipv6SegmentRoutingEndNodeEnable;
    entryPtr->sourceIdSetEnable = dbEntryPtr->sourceIdSetEnable;
    entryPtr->sourceId = dbEntryPtr->sourceId;
    entryPtr->ingressPipeBypass = dbEntryPtr->ingressPipeBypass;
    entryPtr->mirrorToIngressAnalyzerIndex = dbEntryPtr->mirrorToIngressAnalyzerIndex;
    entryPtr->cwBasedPw = dbEntryPtr->cwBasedPw;
    entryPtr->ttlExpiryVccvEnable = dbEntryPtr->ttlExpiryVccvEnable;
    entryPtr->pwe3FlowLabelExist = dbEntryPtr->pwe3FlowLabelExist;
    entryPtr->pwCwBasedETreeEnable = dbEntryPtr->pwCwBasedETreeEnable;
    entryPtr->applyNonDataCwCommand = dbEntryPtr->applyNonDataCwCommand;
    entryPtr->skipFdbSaLookupEnable = dbEntryPtr->skipFdbSaLookupEnable;
    entryPtr->exactMatchOverTtiEn = dbEntryPtr->exactMatchOverTtiEn;
    entryPtr->copyReservedAssignmentEnable = dbEntryPtr->copyReservedAssignmentEnable;
    entryPtr->copyReserved = dbEntryPtr->copyReserved;
    entryPtr->triggerHashCncClient = dbEntryPtr->triggerHashCncClient;

    return GT_OK;
}

/*
* @internal restoreEntryFormatParamsFromDb_exactMatchEntryPclFormat function
* @endinternal
*
* @brief  function to restore values of the Exact Match entry from DB in format
*         CPSS_DXCH_PCL_ACTION_STC
*         copy values from manager format to application format
*
* @param[in]  dbEntryPtr            - (pointer to) the manager entry format (manager format)
* @param[out] entryPtr               - (pointer to) the entry (application format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval none
*/
static GT_STATUS restoreEntryFormatParamsFromDb_exactMatchEntryPclFormat
(
    IN  PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_PCL_FORMAT_STC   *dbEntryPtr,
    OUT CPSS_DXCH_PCL_ACTION_STC                                    *entryPtr
)
{
    GT_STATUS rc = GT_OK;
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT dsaCpuCode;

    entryPtr->egressPolicy = GT_FALSE;

    /* save to application format from DB format */
    entryPtr->exactMatchOverPclEn = dbEntryPtr->exactMatchOverPclEn;
    entryPtr->matchCounter.enableMatchCount = dbEntryPtr->bindToCncCounter;
    entryPtr->matchCounter.matchCounterIndex = dbEntryPtr->cncCounterIndex;

    entryPtr->oam.timeStampEnable= dbEntryPtr->oamTimeStampEnable;
    entryPtr->oam.offsetIndex = dbEntryPtr->oamOffsetIndex;
    entryPtr->oam.oamProcessEnable = dbEntryPtr->oamProcessEnable;
    entryPtr->oam.oamProfile = dbEntryPtr->oamProfile;
    entryPtr->flowId = dbEntryPtr->flowId;
    entryPtr->actionStop = dbEntryPtr->actionStop;
    entryPtr->bypassBridge = dbEntryPtr->bypassBridge;
    entryPtr->bypassIngressPipe = dbEntryPtr->bypassIngressPipe;
    entryPtr->lookupConfig.ipclConfigIndex = dbEntryPtr->ipclConfigIndex;
    entryPtr->lookupConfig.pcl0_1OverrideConfigIndex = dbEntryPtr->pcl0_1OverrideConfigIndex;
    entryPtr->lookupConfig.pcl1OverrideConfigIndex = dbEntryPtr->pcl1OverrideConfigIndex;
    entryPtr->copyReserved.copyReserved = dbEntryPtr->copyReserved;
    entryPtr->copyReserved.assignEnable = dbEntryPtr->copyReservedAssignEnable;
    entryPtr->triggerHashCncClient = dbEntryPtr->triggerHashCncClient;
    PRV_CPSS_PCL_CONVERT_HW_VAL_TO_ATTR_MODIFY_MAC(entryPtr->qos.ingress.modifyDscp, dbEntryPtr->modifyDscp);
    PRV_CPSS_PCL_CONVERT_HW_VAL_TO_ATTR_MODIFY_MAC(entryPtr->qos.ingress.modifyUp, dbEntryPtr->modifyUp);
    entryPtr->qos.ingress.profileIndex = dbEntryPtr->profileIndex;
    entryPtr->qos.ingress.profileAssignIndex = dbEntryPtr->profileAssignIndex;
    PRV_CPSS_CONVERT_HW_VAL_TO_PRECEDENCE_MAC(entryPtr->qos.ingress.profilePrecedence,dbEntryPtr->profilePrecedence);
    entryPtr->qos.ingress.up1Cmd = dbEntryPtr->up1Cmd;
    entryPtr->qos.ingress.up1 = dbEntryPtr->up1;
    entryPtr->setMacToMe = dbEntryPtr->setMacToMe;
    entryPtr->skipFdbSaLookup = dbEntryPtr->skipFdbSaLookup;
    entryPtr->triggerInterrupt = dbEntryPtr->triggerInterrupt;

    entryPtr->redirect.redirectCmd = dbEntryPtr->redirectCmd;
    rc = restoreEntryFormatParamsFromDb_exactMatchEntryPclFormat_redirectConfig(dbEntryPtr,entryPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    entryPtr->sourceId.sourceIdValue = dbEntryPtr->sourceIdValue;
    entryPtr->sourceId.assignSourceId = dbEntryPtr->assignSourceId;
    entryPtr->sourcePort.assignSourcePortEnable = dbEntryPtr->assignSourcePortEnable;
    entryPtr->sourcePort.sourcePortValue = dbEntryPtr->sourcePortValue;

    entryPtr->vlan.ingress.modifyVlan = dbEntryPtr->modifyVlan;
    entryPtr->vlan.ingress.nestedVlan = dbEntryPtr->nestedVlan;
    entryPtr->vlan.ingress.vlanId= dbEntryPtr->vlanId;
    PRV_CPSS_CONVERT_HW_VAL_TO_PRECEDENCE_MAC(entryPtr->vlan.ingress.precedence,dbEntryPtr->precedence);
    entryPtr->vlan.ingress.vlanId1Cmd = dbEntryPtr->vlanId1Cmd;
    entryPtr->vlan.ingress.vlanId1= dbEntryPtr->vlanId1;

    entryPtr->latencyMonitor.monitoringEnable = dbEntryPtr->monitoringEnable;
    entryPtr->latencyMonitor.latencyProfile = dbEntryPtr->latencyProfile;

    /* deal with NON- 1:1 field values */
    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(entryPtr->pktCmd, dbEntryPtr->pktCmd);
    if (entryPtr->pktCmd != CPSS_PACKET_CMD_FORWARD_E)
    {
        dsaCpuCode = dbEntryPtr->cpuCode;
        rc = prvCpssDxChNetIfDsaToCpuCode(dsaCpuCode,
                                           &entryPtr->mirror.cpuCode);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        entryPtr->mirror.cpuCode=0;
    }

    if ((dbEntryPtr->bindToPolicerMeter==0) && (dbEntryPtr->bindToPolicerCounter==0))
    {
        entryPtr->policer.policerEnable=CPSS_DXCH_PCL_POLICER_DISABLE_ALL_E;
    }
    else
    {
        if (dbEntryPtr->bindToPolicerMeter==1&&dbEntryPtr->bindToPolicerCounter==1)
        {
            entryPtr->policer.policerEnable=CPSS_DXCH_PCL_POLICER_ENABLE_METER_AND_COUNTER_E;
        }
        else
        {
            if (dbEntryPtr->bindToPolicerMeter==1&&dbEntryPtr->bindToPolicerCounter==0)
            {
                entryPtr->policer.policerEnable=CPSS_DXCH_PCL_POLICER_ENABLE_METER_ONLY_E;
            }
            else/* dbEntryPtr->bindToPolicerMeter==0&&dbEntryPtr->bindToPolicerCounter==1 */
            {
                entryPtr->policer.policerEnable=CPSS_DXCH_PCL_POLICER_ENABLE_COUNTER_ONLY_E;
            }
        }
    }

    entryPtr->policer.policerId = dbEntryPtr->policerIndex;

    entryPtr->mirror.mirrorTcpRstAndFinPacketsToCpu = dbEntryPtr->mirrorTcpRstAndFinPacketsToCpu;
    if (dbEntryPtr->ingressMirrorToAnalyzerIndex==0)
    {
        entryPtr->mirror.mirrorToRxAnalyzerPort=GT_FALSE;
        entryPtr->mirror.ingressMirrorToAnalyzerIndex = 0;
    }
    else
    {
        entryPtr->mirror.mirrorToRxAnalyzerPort=GT_TRUE;
        entryPtr->mirror.ingressMirrorToAnalyzerIndex = dbEntryPtr->ingressMirrorToAnalyzerIndex-1;
    }

    return GT_OK;
}

/*
* @internal restoreEntryFormatParamsFromDb_exactMatchEntryEpclFormat function
* @endinternal
*
* @brief  function to restore values of the Exact Match entry from DB in format
*         CPSS_DXCH_PCL_ACTION_STC
*         copy values from manager format to application format
*
* @param[in]  dbEntryPtr            - (pointer to) the manager entry format (manager format)
* @param[out] entryPtr              - (pointer to) the entry (application format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval none
*/
static GT_STATUS restoreEntryFormatParamsFromDb_exactMatchEntryEpclFormat
(
    IN  PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_EPCL_FORMAT_STC  *dbEntryPtr,
    OUT CPSS_DXCH_PCL_ACTION_STC                                   *entryPtr
)
{
    GT_STATUS rc = GT_OK;
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT dsaCpuCode;

    /* save to application format from DB format */
    entryPtr->exactMatchOverPclEn = dbEntryPtr->exactMatchOverPclEn;

    entryPtr->egressPolicy = GT_TRUE;

    entryPtr->matchCounter.enableMatchCount = dbEntryPtr->bindToCncCounter;
    entryPtr->matchCounter.matchCounterIndex = dbEntryPtr->cncCounterIndex;

    entryPtr->oam.timeStampEnable= dbEntryPtr->oamTimeStampEnable;
    entryPtr->oam.offsetIndex = dbEntryPtr->oamOffsetIndex;
    entryPtr->oam.oamProcessEnable = dbEntryPtr->oamProcessEnable;
    entryPtr->oam.oamProfile = dbEntryPtr->oamProfile;
    entryPtr->flowId = dbEntryPtr->flowId;

    entryPtr->copyReserved.copyReserved = dbEntryPtr->copyReserved;
    entryPtr->copyReserved.assignEnable = dbEntryPtr->copyReservedAssignEnable;

    entryPtr->qos.egress.modifyDscp = dbEntryPtr->modifyDscp;
    entryPtr->qos.egress.dscp = dbEntryPtr->dscp;
    entryPtr->qos.egress.modifyUp  = dbEntryPtr->modifyUp;
    entryPtr->qos.egress.up = dbEntryPtr->up;
    entryPtr->qos.egress.up1ModifyEnable = dbEntryPtr->up1ModifyEnable;
    entryPtr->qos.egress.up1 = dbEntryPtr->up1;

    entryPtr->vlan.egress.vlanCmd = dbEntryPtr->vlanCmd;
    entryPtr->vlan.egress.vlanId1ModifyEnable = dbEntryPtr->vlanId1ModifyEnable;
    entryPtr->vlan.egress.vlanId = dbEntryPtr->vlanId;
    entryPtr->vlan.egress.vlanId1 = dbEntryPtr->vlanId1;

    entryPtr->channelTypeToOpcodeMapEnable = dbEntryPtr->channelTypeToOpcodeMapEnable;
    entryPtr->terminateCutThroughMode =  dbEntryPtr->terminateCutThroughMode;

    entryPtr->latencyMonitor.monitoringEnable = dbEntryPtr->monitoringEnable;
    entryPtr->latencyMonitor.latencyProfile = dbEntryPtr->latencyProfile;

    entryPtr->egressCncIndexMode = dbEntryPtr->egressCncIndexMode;
    entryPtr->enableEgressMaxSduSizeCheck = dbEntryPtr->enableEgressMaxSduSizeCheck;
    entryPtr->egressMaxSduSizeProfile = dbEntryPtr->egressMaxSduSizeProfile;

    /* deal with NON- 1:1 field values */
    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(entryPtr->pktCmd, dbEntryPtr->pktCmd);
    if (entryPtr->pktCmd != CPSS_PACKET_CMD_FORWARD_E)
    {
        dsaCpuCode = dbEntryPtr->cpuCode;
        rc = prvCpssDxChNetIfDsaToCpuCode(dsaCpuCode,
                                           &entryPtr->mirror.cpuCode);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        entryPtr->mirror.cpuCode=0;
    }

    if (dbEntryPtr->bindToPolicerMeter==0&&dbEntryPtr->bindToPolicerCounter==0)
    {
        entryPtr->policer.policerEnable=CPSS_DXCH_PCL_POLICER_DISABLE_ALL_E;
    }
    else
    {
        if (dbEntryPtr->bindToPolicerMeter==1&&dbEntryPtr->bindToPolicerCounter==1)
        {
            entryPtr->policer.policerEnable=CPSS_DXCH_PCL_POLICER_ENABLE_METER_AND_COUNTER_E;
        }
        else
        {
            if (dbEntryPtr->bindToPolicerMeter==1&&dbEntryPtr->bindToPolicerCounter==0)
            {
                entryPtr->policer.policerEnable=CPSS_DXCH_PCL_POLICER_ENABLE_METER_ONLY_E;
            }
            else/* dbEntryPtr->bindToPolicerMeter==0&&dbEntryPtr->bindToPolicerCounter==1 */
            {
                entryPtr->policer.policerEnable=CPSS_DXCH_PCL_POLICER_ENABLE_COUNTER_ONLY_E;
            }
        }
    }

    entryPtr->policer.policerId = dbEntryPtr->policerIndex;

    if (dbEntryPtr->egressMirrorToAnalyzerIndex == 0)
    {
        entryPtr->mirror.mirrorToTxAnalyzerPortEn=GT_FALSE;
        entryPtr->mirror.egressMirrorToAnalyzerIndex=0;
    }
    else
    {
        entryPtr->mirror.mirrorToTxAnalyzerPortEn=GT_TRUE;
        entryPtr->mirror.egressMirrorToAnalyzerIndex = dbEntryPtr->egressMirrorToAnalyzerIndex-1;
    }

    PRV_CPSS_DXCH_MIRROR_CONVERT_EGRESS_MIRROR_MODE_TO_SW_VAL_MAC((dbEntryPtr->egressMirrorToAnalyzerMode),(entryPtr->mirror.egressMirrorToAnalyzerMode));

    if (dbEntryPtr->phaThreadNumberAssignmentEnable==0)
    {
        entryPtr->epclPhaInfo.phaThreadIdAssignmentMode=CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_DISABLED_E;
        entryPtr->epclPhaInfo.phaThreadId = 0;
    }
    else
    {
        if(dbEntryPtr->phaThreadNumber==0)
        {
            entryPtr->epclPhaInfo.phaThreadIdAssignmentMode=CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_BYPASS_PHA_E;
            entryPtr->epclPhaInfo.phaThreadId = 0;
        }
        else /* dbEntryPtr->phaThreadNumber!=0*/
        {
            entryPtr->epclPhaInfo.phaThreadIdAssignmentMode=CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENABLED_E;
            entryPtr->epclPhaInfo.phaThreadId = dbEntryPtr->phaThreadNumber;
        }
    }

    if (dbEntryPtr->phaMetadataAssignEnable==0)
    {
        entryPtr->epclPhaInfo.phaThreadType=CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_TYPE_NOT_USED_E;
        entryPtr->epclPhaInfo.phaThreadUnion.notNeeded = 0;
    }
    else
    {
        entryPtr->epclPhaInfo.phaThreadType=dbEntryPtr->phaThreadType;
        switch (entryPtr->epclPhaInfo.phaThreadType)
        {
            case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_IOAM_INGRESS_SWITCH_E:
                entryPtr->epclPhaInfo.phaThreadUnion.epclIoamIngressSwitch.ioamIncludesBothE2EOptionAndTraceOption =
                    BIT2BOOL_MAC((dbEntryPtr->phaMetadata >> 31) & 1);
                break;
            case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INT_IOAM_MIRROR_E:
                entryPtr->epclPhaInfo.phaThreadUnion.epclIntIoamMirror.erspanIII_P = BIT2BOOL_MAC((dbEntryPtr->phaMetadata >> 31) & 1);
                entryPtr->epclPhaInfo.phaThreadUnion.epclIntIoamMirror.erspanIII_FT = ((dbEntryPtr->phaMetadata >> 26) & 0x1F); /* frameType[30:26] */
                break;
            case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_CLASSIFIER_NSH_OVER_VXLAN_GPE_E:
                entryPtr->epclPhaInfo.phaThreadUnion.epclClassifierNshOverVxlanGpe.classifierNshOverVxlanGpe_TenantID = BIT2BOOL_MAC((dbEntryPtr->phaMetadata >> 16) & 0xFFFF);/* Desc<pha metadata>[31:16] */
                entryPtr->epclPhaInfo.phaThreadUnion.epclClassifierNshOverVxlanGpe.classifierNshOverVxlanGpe_SourceClass = ((dbEntryPtr->phaMetadata) & 0xFFFF); /*Desc<pha metadata>[15:0] */
                break;
            case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ERSPAN_TYPE_II_MIRROR_E:
                entryPtr->epclPhaInfo.phaThreadUnion.epclErspanTypeIImirror.erspanAnalyzerIndex = ((dbEntryPtr->phaMetadata)  & 0x7);
                break;
            default: /* unknown format */
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(entryPtr->epclPhaInfo.phaThreadType);
        }
    }
    /* end NON- 1:1 field values */

    return GT_OK;
}

/**
* @internal prvCpssDxChExactMatchManagerRestoreEntryFormatParamsFromDb function
* @endinternal
*
* @brief  function to restore values of the Exact Match entry
*         from DB copy values from manager format to application
*         format the 'opposite' logic of
*         saveEntryFormatParamsToDb(...)
*
* @param[in] dbEntryPtr             - (pointer to) the manager entry format (manager format)
* @param[out] entryPtr              - (pointer to) the entry (application format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS   prvCpssDxChExactMatchManagerRestoreEntryFormatParamsFromDb
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC    *dbEntryPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                 *entryPtr
)
{
    GT_STATUS rc=GT_OK;

    cpssOsMemSet(entryPtr, 0, sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC));

    switch(dbEntryPtr->hwExactMatchKeySize)
    {
        case 0:
            entryPtr->exactMatchEntry.key.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
            break;
        case 1:
            entryPtr->exactMatchEntry.key.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;
            break;
        case 2:
            entryPtr->exactMatchEntry.key.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;
            break;
        case 3:
            entryPtr->exactMatchEntry.key.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(dbEntryPtr->hwExactMatchKeySize);
    }

    switch (dbEntryPtr->hwExactMatchLookupNum)
    {
        case 0:
            entryPtr->exactMatchEntry.lookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
            break;
        case 1:
            entryPtr->exactMatchEntry.lookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(dbEntryPtr->hwExactMatchLookupNum);
    }

    switch(dbEntryPtr->hwExactMatchEntryType)
    {
        case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_HW_ENTRY_TYPE_TTI_E:
            entryPtr->exactMatchActionType = CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;
            rc = restoreEntryFormatParamsFromDb_exactMatchEntryTtiFormat(&dbEntryPtr->specificFormat.prvTtiEntryFormat,
                                                                    &entryPtr->exactMatchAction.ttiAction);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_HW_ENTRY_TYPE_PCL_E:
            entryPtr->exactMatchActionType = CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;
            restoreEntryFormatParamsFromDb_exactMatchEntryPclFormat(&dbEntryPtr->specificFormat.prvPclEntryFormat,
                                                                    &entryPtr->exactMatchAction.pclAction);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_HW_ENTRY_TYPE_EPCL_E:
            entryPtr->exactMatchActionType = CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E;
            rc = restoreEntryFormatParamsFromDb_exactMatchEntryEpclFormat(&dbEntryPtr->specificFormat.prvEpclEntryFormat,
                                                                          &entryPtr->exactMatchAction.pclAction);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(dbEntryPtr->hwExactMatchEntryType);
    }

    entryPtr->exactMatchUserDefined = dbEntryPtr->exactMatchUserDefined;
    entryPtr->expandedActionIndex = dbEntryPtr->hwExactMatchExpandedActionIndex;
    cpssOsMemCpy(entryPtr->exactMatchEntry.key.pattern,dbEntryPtr->pattern,sizeof(dbEntryPtr->pattern));

    return rc;
}

/*
* @internal mainLogicEntryUpdate function
* @endinternal
*
* @brief   This function updates entry to CPSS Exact Match Manager's database and HW.
*          All types of entries can be updated using this API.
*          NOTE:
*          - the Exact Match manager will set the info to all the registered devices
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerPtr   - (pointer to) the Exact Match Manager
* @param[in] entryPtr               - (pointer to) Exact Match entry format to be updated
* @param[out] updateApiStatisticsPtr- (pointer to) the type of the API ok/error statistics
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - the entry not found
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager is not initialized
*
* @note
*   NONE
*
*/
static GT_STATUS mainLogicEntryUpdate
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC               *exactMatchManagerPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                              *entryPtr,
    OUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_API_STATISTICS_ENT   *updateApiStatisticsPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC calcInfo;
    CPSS_DXCH_EXACT_MATCH_CLIENT_ENT                         lastCounterClientType;
    CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT                       lastCounterKeySize;

    *updateApiStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_INPUT_INVALID_E;

    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    *updateApiStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_API_STATISTICS___LAST___E;

    /********************************
      Validity checks of SW format
     ********************************/
    rc = prvCpssDxChExactMatchManagerValidateEntryFormatParams(exactMatchManagerPtr, entryPtr);
    if(rc != GT_OK)
    {
        *updateApiStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_INPUT_INVALID_E;
        return rc;
    }

    /*************************************
      Lookup for exixting entry in the DB
     *************************************/
    rc = prvCpssDxChExactMatchManagerDbCalcHashAndLookupResult(exactMatchManagerPtr, entryPtr, NULL, &calcInfo);
    if(rc != GT_OK)
    {
        /* Should not fail, so if failed this is bad state */
        *updateApiStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_BAD_STATE_E;
        return rc;
    }

    if(calcInfo.calcEntryType != PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_EXISTS_E)
    {
        *updateApiStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_NOT_FOUND_E;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "the entry doesn't exists so can not update it");
    }

    /* from this point ... should not fail ! so if failed this is bad state */
    *updateApiStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_BAD_STATE_E;

    /* store last counter type */
    rc = prvCpssDxChExactMatchManagerCounterPacketTypeGet(exactMatchManagerPtr,NULL,calcInfo.dbEntryPtr,&lastCounterClientType,&lastCounterKeySize);
    if(rc != GT_OK)
    {
        return rc;
    }

    /**************************************
      Update entry format values to the DB and HW
    **************************************/
    rc = prvCpssDxChExactMatchManagerUpdateEntry(exactMatchManagerPtr, entryPtr, &calcInfo, updateApiStatisticsPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /**************************************************************************************************
      Counters update.
      The only case the API should update the counters is when static entry overwrites dynamic entry -
      in all other cases counters should not be updated at all.
    **************************************************************************************************/
    rc = prvCpssDxChExactMatchManagerCounterPacketTypeGet(exactMatchManagerPtr,entryPtr, NULL, &calcInfo.counterClientType, &calcInfo.counterKeyType);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChExactMatchManagerEntryUpdate function
* @endinternal
*
* @brief   The function updates existing entry in CPSS Exact Match Manager's database & HW.
*          All types of entries can be updated using this API.
*          The function only updates the action of the entry.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
static GT_STATUS internal_cpssDxChExactMatchManagerEntryUpdate
(
    IN GT_U32                                                exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC               *entryPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_API_STATISTICS_ENT   updateApiStatistics;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC              *exactMatchManagerPtr;

    EM_MANAGER_ID_CHECK(exactMatchManagerId); /* must be done here because used in 'API' statistics */

    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    /* call the main logic */
    rc = mainLogicEntryUpdate(exactMatchManagerPtr, entryPtr, &updateApiStatistics);

    if(updateApiStatistics < PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_API_STATISTICS___LAST___E)
    {
        /* increment the 'API statistics' */
        exactMatchManagerPtr->apiEntryUpdateStatisticsArr[updateApiStatistics]++;
    }

    return rc;
}

/**
* @internal cpssDxChExactMatchManagerEntryUpdate function
* @endinternal
*
* @brief   The function updates existing entry in CPSS Exact Match Manager's database & HW.
*          All types of entries can be updated using this API.
*          The function only updates the action of the entry.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
GT_STATUS cpssDxChExactMatchManagerEntryUpdate
(
    IN GT_U32                                                exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC               *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchManagerEntryUpdate);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, exactMatchManagerId,entryPtr));

    rc = internal_cpssDxChExactMatchManagerEntryUpdate(exactMatchManagerId,entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, exactMatchManagerId,entryPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);

    return rc;
}

/**
* @internal prvCpssDxChExactMatchManagerEntryDelete function
* @endinternal
*
* @brief  function to deletes the entry in all devices(HW & DB)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerPtr    - (pointer to) the Exact Match Manager.
* @param[in] calcInfoPtr             - (pointer to) information about entry to delete
* @param[out] deleteApiStatisticsPtr - (pointer to) the type of API ok/error statistics
*
* @retval GT_OK                      - on success
* @retval GT_BAD_PARAM               - on wrong parameters
* @retval GT_BAD_PTR                 - on NULL pointer.
* @retval GT_NOT_FOUND               - the entry was not found.
* @retval GT_HW_ERROR                - on hardware error
* @retval GT_NOT_INITIALIZED         - if the Exact Match Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE   - on not applicable device
*/
static GT_STATUS prvCpssDxChExactMatchManagerEntryDelete
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC               *exactMatchManagerPtr,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC             *calcInfoPtr,
    OUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_DELETE_API_STATISTICS_ENT   *deleteApiStatisticsPtr
)
{
    GT_STATUS                                        rc;
    CPSS_SYSTEM_RECOVERY_INFO_STC                    tempSystemRecovery_Info;
    GT_BOOL                                          managerHwWriteBlock;
    /********************************/
    /* get the counter packet type  */
    /********************************/
    /* use the entry that we found in the DB ... not the one given by application , because the application only gave 'key' fields */
    rc = prvCpssDxChExactMatchManagerCounterPacketTypeGet(exactMatchManagerPtr,NULL,calcInfoPtr->dbEntryPtr,&calcInfoPtr->counterClientType,&calcInfoPtr->counterKeyType);
    if(rc != GT_OK)
    {
        return rc;
    }

    /******************************************************************************/
    /* delete old entry :                                                         */
    /* update metadata DB :indexArr[] , bankInfoArr[] , typeCountersArr[]         */
    /* update headOfFreeList , headOfUsedList , tailOfUsedList                    */
    /* update calcInfoPtr->dbEntryPtr about valid pointers                        */
    /* lastGetNextInfo , lastDeleteInfo                                           */
    /******************************************************************************/
    rc = prvCpssDxChExactMatchManagerDbDeleteOldEntry(exactMatchManagerPtr, calcInfoPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*for error injection tests  ,simulate crash after SW entry Delete before HW delete */
    rc = prvCpssSystemRecoveryErrorInjectionCheck (PRV_CPSS_SYSYEM_RECOVERY_ERROR_INJECTION_INDEX_EM_8_E);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* no need to Delete for HW - emulate HA process */
    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_EXACT_MATCH_MANAGER_E);
    if (!((tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E) &&
         (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)) && managerHwWriteBlock == GT_FALSE)
    {
        /***************************************************************************/
        /* flush the entry from HW of all registered devices                       */
        /***************************************************************************/
        rc = prvCpssDxChExactMatchManagerHwFlushByHwIndex(exactMatchManagerPtr,
                                                        calcInfoPtr->crcMultiHashArr[calcInfoPtr->selectedBankId]/*hwIndex*/);
        if(rc != GT_OK)
        {
            *deleteApiStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_DELETE_API_STATISTICS_ERROR_HW_UPDATE_E;
            return rc;
        }
    }

    *deleteApiStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_DELETE_API_STATISTICS_OK_E;

    /* update DB with the correct number of exact match bounded to Expander index */
    if(exactMatchManagerPtr->exactMatchExpanderArray[calcInfoPtr->dbEntryPtr->hwExactMatchExpandedActionIndex].exactMatchExpandedEntryValid==GT_TRUE)
    {
        if (exactMatchManagerPtr->exactMatchExpanderArray[calcInfoPtr->dbEntryPtr->hwExactMatchExpandedActionIndex].exactMatchEntriesCounter!=0)
        {
            exactMatchManagerPtr->exactMatchExpanderArray[calcInfoPtr->dbEntryPtr->hwExactMatchExpandedActionIndex].exactMatchEntriesCounter--;
        }
        else
        {
            /* should not happen */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "ERROR: exactMatchEntriesCounter[%d] can not be decremented.\n",
                                          exactMatchManagerPtr->exactMatchExpanderArray[calcInfoPtr->dbEntryPtr->hwExactMatchExpandedActionIndex].exactMatchEntriesCounter);
        }
        /* clear rest of fields from DB since the entry was completly deleted from hw & DB */
        calcInfoPtr->dbEntryPtr->exactMatchUserDefined = 0;
        calcInfoPtr->dbEntryPtr->hwExactMatchEntryType = 0;
        calcInfoPtr->dbEntryPtr->hwExactMatchExpandedActionIndex = 0;
        calcInfoPtr->dbEntryPtr->hwExactMatchKeySize = 0;
        calcInfoPtr->dbEntryPtr->hwExactMatchLookupNum = 0;
        cpssOsMemSet(calcInfoPtr->dbEntryPtr->pattern,0,sizeof(calcInfoPtr->dbEntryPtr->pattern));
        cpssOsMemSet(&calcInfoPtr->dbEntryPtr->specificFormat,0,sizeof(calcInfoPtr->dbEntryPtr->specificFormat));
        calcInfoPtr->dbEntryPtr->isFirst = GT_FALSE;

    }
    else
    {
        /* should never happen since if we delete an entry it must be pointed to a valid expander index */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "ERROR: exactMatchExpanderArray[%d] is not valid.\n",calcInfoPtr->dbEntryPtr->hwExactMatchExpandedActionIndex);
    }

    return GT_OK;
}

/**
* @internal mainLogicEntryDelete function
* @endinternal
*
* @brief   This function delete entry from the CPSS Exact Match Manager's database and HW.
*          All types of entries can be deleted using this API.
*          NOTE: the Exact Match manager will remove the info from all the registered devices.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerPtr    - (pointer to) the Exact Match Manager.
* @param[in] entryPtr                - (pointer to) Exact Match entry format to be deleted.
*                                      NOTE: only the 'exactMatchEntryKey' part is used by the API.
* @param[in] deleteSpecificBank      - whether to delete from specific bank id.
* @param[in] bankId                  - bank id to delete from. Valid if deleteSpecificBank == GT_TRUE.
* @param[out] deleteApiStatisticsPtr - (pointer to) the type of API ok/error statistics
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
static GT_STATUS mainLogicEntryDelete
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC               *exactMatchManagerPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                              *entryPtr,
    IN GT_BOOL                                                              deleteSpecificBank,
    IN GT_U32                                                               bankId,
    OUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_DELETE_API_STATISTICS_ENT   *deleteApiStatisticsPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC calcInfo;
    GT_U32 sizeInBytes,line,exactMatchManagerId,ii;
    GT_BOOL updateStatistics = GT_TRUE;

    *deleteApiStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_DELETE_API_STATISTICS_ERROR_INPUT_INVALID_E;
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    *deleteApiStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_DELETE_API_STATISTICS___LAST___E;

    /********************************/
    /* validity checks of SW format */
    /********************************/
    rc = prvCpssDxChExactMatchManagerValidateEntryKeyFormatParams(entryPtr);
    if(rc != GT_OK)
    {
        *deleteApiStatisticsPtr =
            PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_DELETE_API_STATISTICS_ERROR_INPUT_INVALID_E;

        return rc;
    }

    /* from this point ... should not fail ! so if failed this is bad state */
    *deleteApiStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_DELETE_API_STATISTICS_ERROR_BAD_STATE_E;

    /*******************************************/
    /* calculate where 'key' is expected in DB */
    /*******************************************/
    cpssOsMemSet(&calcInfo,0,sizeof(calcInfo));
    rc = prvCpssDxChExactMatchManagerDbCalcHashAndLookupResult(exactMatchManagerPtr,entryPtr,NULL,&calcInfo);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(calcInfo.calcEntryType != PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_EXISTS_E)
    {
        *deleteApiStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_DELETE_API_STATISTICS_ERROR_ENTRY_NOT_FOUND_E;

        /* the 'key' was not found in the DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND,LOG_ERROR_NO_MSG);
    }

    /* delete from specific bank; the entry is currently duplicated in 2 banks during entry movement in rehash function */
    if(deleteSpecificBank)
    {
        rc = prvCpssDxChExactMatchManagerBankCalcInfoGet(exactMatchManagerPtr, bankId, entryPtr, NULL, &calcInfo);
        if(rc != GT_OK)
        {
            return rc;
        }
        updateStatistics = GT_FALSE;
    }

    rc = prvCpssDxChExactMatchManagerEntryDelete(exactMatchManagerPtr, &calcInfo, deleteApiStatisticsPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /******************************************************************************/
    /* deleting entry from cuckoo DB                                              */
    /******************************************************************************/

    /*for error injection tests  ,simulate crash after SW & HW entry delete before cuckoo DB update  */
    rc = prvCpssSystemRecoveryErrorInjectionCheck (PRV_CPSS_SYSYEM_RECOVERY_ERROR_INJECTION_INDEX_EM_9_E);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* convert enum key size to size in bytes */
    EM_CONV_KEY_ENUM_TO_SIZE_IN_BYTES_MAC(entryPtr->exactMatchEntry.key.keySize,sizeInBytes);

    /* calc line from hwIndex */
    EM_LINE_FROM_HW_INDEX_GET_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes,
                                  calcInfo.crcMultiHashArr[calcInfo.selectedBankId],line);

    /* get Excat Match manager id */
    EM_MANAGER_ID_GET_MAC(exactMatchManagerPtr,exactMatchManagerId);

    rc = prvCpssDxChCuckooDbEntryDelete(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E, exactMatchManagerId, calcInfo.selectedBankId,line,sizeInBytes,updateStatistics);

    return rc;
}

/**
* @internal internal_cpssDxChExactMatchManagerEntryDelete function
* @endinternal
*
* @brief   This function delete entry from the CPSS Exact Match Manager's database and HW.
*          All types of entries can be deleted using this API.
*          NOTE: the Exact Match manager will remove the info from all the registered devices.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
static GT_STATUS internal_cpssDxChExactMatchManagerEntryDelete
(
    IN GT_U32                                           exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC          *entryPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_DELETE_API_STATISTICS_ENT deleteApiStatistics;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC            *exactMatchManagerPtr;

    EM_MANAGER_ID_CHECK(exactMatchManagerId); /* must be done here because used in 'API' statistics */

    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    /* call the main logic */
    rc = mainLogicEntryDelete(exactMatchManagerPtr, entryPtr, GT_FALSE,0,&deleteApiStatistics);

    if(deleteApiStatistics < PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_DELETE_API_STATISTICS___LAST___E)
    {
        /* increment the 'API statistics' */
        exactMatchManagerPtr->apiEntryDeleteStatisticsArr[deleteApiStatistics]++;
    }
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChExactMatchManagerEntryDelete function
* @endinternal
*
* @brief   This function delete entry from the CPSS Exact Match Manager's database and HW.
*          All types of entries can be deleted using this API.
*          NOTE: the Exact Match manager will remove the info from all the registered devices.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
GT_STATUS cpssDxChExactMatchManagerEntryDelete
(
    IN GT_U32                                           exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC          *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchManagerEntryDelete);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, exactMatchManagerId,entryPtr));

    rc = internal_cpssDxChExactMatchManagerEntryDelete(exactMatchManagerId,entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, exactMatchManagerId,dentryPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChExactMatchManagerEntryGet function
* @endinternal
*
* @brief   The function return entry info from the manager by 'key'
*          NOTE: no HW accessing.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entryPtr              - (pointer to) Exact Match entry format with the 'exactMatchEntryKey'.
*                                     NOTE: only the 'exactMatchEntryKey' part is used by the API.(as [in] parameter)
* @param[out] entryPtr             - (pointer to) Exact Match entry format with full info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_FOUND             - if entry not found for that 'key'.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager id is not initialized.
*
* @note
*   NONE
*
*/
static GT_STATUS internal_cpssDxChExactMatchManagerEntryGet
(
    IN      GT_U32                                        exactMatchManagerId,
    INOUT   CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC       *entryPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC calcInfo;

    EM_MANAGER_ID_CHECK(exactMatchManagerId);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    /********************************/
    /* validity checks of SW format */
    /********************************/
    rc = prvCpssDxChExactMatchManagerValidateEntryKeyFormatParams(entryPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*******************************************/
    /* calculate where 'key' is expected in DB */
    /*******************************************/
    rc = prvCpssDxChExactMatchManagerDbCalcHashAndLookupResult(exactMatchManagerPtr,entryPtr,NULL,&calcInfo);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(calcInfo.calcEntryType != PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_EXISTS_E)
    {
        /* the 'key' was not found in the DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND,LOG_ERROR_NO_MSG);
    }

    /* convert DB format to application format */
    rc = prvCpssDxChExactMatchManagerRestoreEntryFormatParamsFromDb(calcInfo.dbEntryPtr, entryPtr);
    if(rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

/**
* @internal cpssDxChExactMatchManagerEntryGet function
* @endinternal
*
* @brief   The function return entry info from the manager by 'key'
*          NOTE: no HW accessing.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entryPtr              - (pointer to) Exact Match entry format with the 'exactMatchEntryKey'.
*                                     NOTE: only the 'exactMatchEntryKey' part is used by the API.(as [in] parameter)
* @param[out] entryPtr             - (pointer to) Exact Match entry format with full info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_FOUND             - if entry not found for that 'key'.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager id is not initialized.
*
* @note
*   NONE
*
*/
GT_STATUS cpssDxChExactMatchManagerEntryGet
(
    IN      GT_U32                                        exactMatchManagerId,
    INOUT   CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC       *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchManagerEntryGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, exactMatchManagerId,entryPtr));

    rc = internal_cpssDxChExactMatchManagerEntryGet(exactMatchManagerId,entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, exactMatchManagerId,entryPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChExactMatchManagerEntryGetNext function
* @endinternal
*
* @brief   The function return first entry or next entry from previous one.
*          NOTE: 1. the 'list' is sorted per insertion order
*                2. no HW accessing.
*           API for fetching next entry data either first one or next relative
*           to last entry retrieved.
*           Once scan completed and API fetched all entries in
*           the list it returns GT_NO_MORE, Application can
*           still call this API to see if any additional entries
*           has been added. It will keep getting GT_NO_MORE
*           until no additional entries or until restarted the
*           scan from beginning. To start over from head of the
*           list Application should pass True on getFirst (its
*           valid to restart the scan anytime - Regardless the
*           position of the scan pointer).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] entryPtr             - (pointer to) Exact Match entry format with full info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NO_MORE               - if no more entries to retrieve
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager id is not initialized.
*
* @note
*   NONE
*
*/
static GT_STATUS internal_cpssDxChExactMatchManagerEntryGetNext
(
    IN  GT_U32                                          exactMatchManagerId,
    IN  GT_BOOL                                         getFirst,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC         *entryPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC    *dbEntryPtr;

    EM_MANAGER_ID_CHECK(exactMatchManagerId);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    rc = prvCpssDxChExactMatchManagerDbApplicationIteratorEntryGetNext(exactMatchManagerPtr,getFirst,&exactMatchManagerPtr->lastGetNextInfo,&dbEntryPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* convert DB format to application format */
    rc = prvCpssDxChExactMatchManagerRestoreEntryFormatParamsFromDb(dbEntryPtr, entryPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChExactMatchManagerEntryGetNext function
* @endinternal
*
* @brief   The function return first entry or next entry from previous one.
*          NOTE: 1. the 'list' is sorted per insertion order
*                2. no HW accessing.
*           API for fetching next entry data either first one or next relative
*           to last entry retrieved.
*           Once scan completed and API fetched all entries in
*           the list it returns GT_NO_MORE, Application can
*           still call this API to see if any additional entries
*           has been added. It will keep getting GT_NO_MORE
*           until no additional entries or until restarted the
*           scan from beginning. To start over from head of the
*           list Application should pass True on getFirst (its
*           valid to restart the scan anytime - Regardless the
*           position of the scan pointer).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] getFirst              - To start over from head of the list
*                                    should pass True on getFirst
* @param[out] entryPtr             - (pointer to) Exact Match entry format with full info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NO_MORE               - if no more entries to retrieve
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager id is not initialized.
*
* @note
*   NONE
*
*/
GT_STATUS cpssDxChExactMatchManagerEntryGetNext
(
    IN  GT_U32                                          exactMatchManagerId,
    IN  GT_BOOL                                         getFirst,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC         *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchManagerEntryGetNext);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, exactMatchManagerId,getFirst,entryPtr));

    rc = internal_cpssDxChExactMatchManagerEntryGetNext(exactMatchManagerId,getFirst,entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, exactMatchManagerId,getFirst,entryPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChExactMatchManagerExpandedActionUpdate function
* @endinternal
*
* @brief   The function set the Expanded Action table.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] expandedActionIndex   - Exact Match Expander table index
*                                    (APPLICABLE RANGES:0..15)
* @param[in] paramsPtr             - the expanded action attributes.
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
*  None
*/
static GT_STATUS internal_cpssDxChExactMatchManagerExpandedActionUpdate
(
    IN  GT_U32                                                          exactMatchManagerId,
    IN  GT_U32                                                          expandedActionIndex,
    IN  CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC    *paramsPtr
)
{
    GT_STATUS           rc;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;

    EM_MANAGER_ID_CHECK(exactMatchManagerId);
    CPSS_NULL_PTR_CHECK_MAC(paramsPtr);

    if (expandedActionIndex>=CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_ENTRIES_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"expandedActionIndex [%d] >= [%d] (out of range)",expandedActionIndex,
                                      CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_ENTRIES_NUM_CNS);
    }

    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    /* verify the entry we want to update already exist */
    if (exactMatchManagerPtr->exactMatchExpanderArray[expandedActionIndex].exactMatchExpandedEntryValid==GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"expandedActionIndex [%d] entry not valid - can not update",expandedActionIndex);
    }

    /* update the entry in Exact Match Manager DB*/
    rc = prvCpssDxChExactMatchManagerDbExpandedActionUpdate(exactMatchManagerId,expandedActionIndex,paramsPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* update the entry in HW*/
    rc = prvCpssDxChExactMatchManagerHwExpandedActionUpdate(exactMatchManagerPtr,expandedActionIndex,paramsPtr);
    if(rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}
/**
* @internal cpssDxChExactMatchManagerExpandedActionUpdate function
* @endinternal
*
* @brief   The function set the Expanded Action table.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] expandedActionIndex   - Exact Match Expander table index
*                                    (APPLICABLE RANGES:0..15)
* @param[in] paramsPtr             - the expanded action attributes.
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
*  None
*/
GT_STATUS cpssDxChExactMatchManagerExpandedActionUpdate
(
    IN  GT_U32                                                          exactMatchManagerId,
    IN  GT_U32                                                          expandedActionIndex,
    IN  CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC    *paramsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchManagerExpandedActionUpdate);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, exactMatchManagerId,expandedActionIndex,paramsPtr));

    rc = internal_cpssDxChExactMatchManagerExpandedActionUpdate(exactMatchManagerId,expandedActionIndex,paramsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, exactMatchManagerId,expandedActionIndex,paramsPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChExactMatchManagerExpandedActionGet function
* @endinternal
*
* @brief  The function return the Expanded Action table entry
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] expandedActionIndex   - Exact Match Expander table index
*                                    (APPLICABLE RANGES:0..15)
* @param[out] paramsPtr             - the expanded action attributes.
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
*  None
*/
static GT_STATUS internal_cpssDxChExactMatchManagerExpandedActionGet
(
    IN  GT_U32                                                          exactMatchManagerId,
    IN  GT_U32                                                          expandedActionIndex,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC    *paramsPtr
)
{
    GT_STATUS           rc;

    EM_MANAGER_ID_CHECK(exactMatchManagerId);
    CPSS_NULL_PTR_CHECK_MAC(paramsPtr);

    if (expandedActionIndex>=CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_ENTRIES_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"expandedActionIndex [%d] >= [%d] (out of range)",expandedActionIndex,
                                      CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_ENTRIES_NUM_CNS);
    }

    /* get the entry from Exact Match Manager DB*/
    rc = prvCpssDxChExactMatchManagerDbExpandedActionGet(exactMatchManagerId,expandedActionIndex,paramsPtr);
    if(rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}
/**
* @internal cpssDxChExactMatchManagerExpandedActionGet function
* @endinternal
*
* @brief The function return the Expanded Action table entry
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] expandedActionIndex   - Exact Match Expander table index
*                                    (APPLICABLE RANGES:0..15)
* @param[out] paramsPtr             - the expanded action attributes.
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
*  None
*/
GT_STATUS cpssDxChExactMatchManagerExpandedActionGet
(
    IN  GT_U32                                                          exactMatchManagerId,
    IN  GT_U32                                                          expandedActionIndex,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC    *paramsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchManagerExpandedActionGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, exactMatchManagerId,expandedActionIndex,paramsPtr));

    rc = internal_cpssDxChExactMatchManagerExpandedActionGet(exactMatchManagerId,expandedActionIndex,paramsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, exactMatchManagerId,expandedActionIndex,paramsPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChExactMatchManagerConfigGet function
* @endinternal
*
* @brief API for fetching current Exact Match Manager setting as they
*        were defined in Exact Match Manager Create.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] capacityPtr          - (pointer to) parameters of the HW structure and capacity of its managed
*                                    PPs EMs: like number of indexes, the amount of hashes etc. As Exact Match
*                                    Manager is created and entries may be added before PP registered,
*                                    we must know in advance the relevant capacity. In Falcon, for
*                                    example, we must know the shared memory allocation mode.
* @param[out] lookupPtr            - (pointer to) parameters that affect lookup of entries
* @param[out] entryAttrPtr         - (pointer to) parameters that affect exact match entries:
*                                    expanded configuration
* @param[out] agingPtr             - (pointer to) parameters that affect refresh and aging
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager is not initialized
*/
static GT_STATUS internal_cpssDxChExactMatchManagerConfigGet
(
    IN  GT_U32                                              exactMatchManagerId,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC          *capacityPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC            *lookupPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC  *entryAttrPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC             *agingPtr
)
{
    EM_MANAGER_ID_CHECK(exactMatchManagerId);
    CPSS_NULL_PTR_CHECK_MAC(capacityPtr);
    CPSS_NULL_PTR_CHECK_MAC(lookupPtr);
    CPSS_NULL_PTR_CHECK_MAC(entryAttrPtr);
    CPSS_NULL_PTR_CHECK_MAC(agingPtr);

    prvCpssDxChExactMatchManagerDbConfigGet(exactMatchManagerId, capacityPtr, lookupPtr, entryAttrPtr, agingPtr);

    return GT_OK;
}

/**
* @internal cpssDxChExactMatchManagerConfigGet function
* @endinternal
*
* @brief API for fetching current Exact Match Manager setting as they
*        were defined in Exact Match Manager Create.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] capacityPtr          - (pointer to) parameters of the HW structure and capacity of its managed
*                                    PPs EMs: like number of indexes, the amount of hashes etc. As Exact Match
*                                    Manager is created and entries may be added before PP registered,
*                                    we must know in advance the relevant capacity. In Falcon, for
*                                    example, we must know the shared memory allocation mode.
* @param[out] lookupPtr            - (pointer to) parameters that affect lookup of entries
* @param[out] entryAttrPtr         - (pointer to) parameters that affect exact match entries:
*                                    expanded configuration
* @param[out] agingPtr             - (pointer to) parameters that affect refresh and aging
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager is not initialized
*/
GT_STATUS cpssDxChExactMatchManagerConfigGet
(
    IN  GT_U32                                              exactMatchManagerId,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC          *capacityPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC            *lookupPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC  *entryAttrPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC             *agingPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchManagerConfigGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, exactMatchManagerId,capacityPtr,lookupPtr,entryAttrPtr,agingPtr));

    rc = internal_cpssDxChExactMatchManagerConfigGet(exactMatchManagerId,capacityPtr,lookupPtr,entryAttrPtr,agingPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, exactMatchManagerId,capacityPtr,lookupPtr,entryAttrPtr,agingPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChExactMatchManagerDevListGet function
* @endinternal
*
* @brief API for fetching current Exact Match Manager registered devices
*        (added by 'add device' API).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[inout] numOfPairsPtr      - (pointer to) the number of devices in the array pairListArr as 'input parameter'
*                                    and actual number of pairs as 'output parameter'.
* @param[out] pairListManagedArray[]- array of registered pairs (no more than list length).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager is not initialized
*/
static GT_STATUS internal_cpssDxChExactMatchManagerDevListGet
(
    IN    GT_U32                                                   exactMatchManagerId,
    INOUT GT_U32                                                   *numOfPairsPtr,
    OUT   CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC    pairListManagedArray[]/*maxArraySize=128*/
)
{
    EM_MANAGER_ID_CHECK(exactMatchManagerId);
    CPSS_NULL_PTR_CHECK_MAC(numOfPairsPtr);
    CPSS_NULL_PTR_CHECK_MAC(pairListManagedArray);

    return prvCpssDxChExactMatchManagerDbDevListGet(exactMatchManagerId, numOfPairsPtr, pairListManagedArray);
}

/**
* @internal cpssDxChExactMatchManagerDevListGet function
* @endinternal
*
* @brief API for fetching current Exact Match Manager registered devices
*        (added by 'add device' API).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[inout] numOfPairsPtr      - (pointer to) the number of devices in the array pairListArr as 'input parameter'
*                                    and actual number of pairs as 'output parameter'.
* @param[out] pairListManagedArray[]- array of registered pairs (no more than list length).

* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager is not initialized
*/
GT_STATUS cpssDxChExactMatchManagerDevListGet
(
    IN    GT_U32                                                   exactMatchManagerId,
    INOUT GT_U32                                                   *numOfPairsPtr,
    OUT   CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC    pairListManagedArray[]/*maxArraySize=128*/
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchManagerDevListGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, exactMatchManagerId,numOfPairsPtr,pairListManagedArray));

    rc = internal_cpssDxChExactMatchManagerDevListGet(exactMatchManagerId,numOfPairsPtr,pairListManagedArray);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, exactMatchManagerId,numOfPairsPtr,pairListManagedArray));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChExactMatchManagerCountersGet function
* @endinternal
*
* @brief API for fetching Exact Match Manager counters.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId    -the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] countersPtr           -(pointer to) Exact Match Manager Counters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager is not initialized
*/
static GT_STATUS internal_cpssDxChExactMatchManagerCountersGet
(
    IN  GT_U32                                      exactMatchManagerId,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_COUNTERS_STC  *countersPtr
)
{
    EM_MANAGER_ID_CHECK(exactMatchManagerId);
    CPSS_NULL_PTR_CHECK_MAC(countersPtr);

    prvCpssDxChExactMatchManagerDbCountersGet(exactMatchManagerId, countersPtr);

    return GT_OK;
}

/**
* @internal cpssDxChExactMatchManagerCountersGet function
* @endinternal
*
* @brief API for fetching Exact Match Manager counters.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId    -the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] countersPtr           -(pointer to) Exact Match Manager Counters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager is not initialized
*/
GT_STATUS cpssDxChExactMatchManagerCountersGet
(
    IN  GT_U32                                      exactMatchManagerId,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_COUNTERS_STC  *countersPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchManagerCountersGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, exactMatchManagerId,countersPtr));

    rc = internal_cpssDxChExactMatchManagerCountersGet(exactMatchManagerId,countersPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, exactMatchManagerId,countersPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChExactMatchManagerStatisticsGet function
* @endinternal
*
* @brief API for fetching Exact Match Manager statistics.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
static GT_STATUS internal_cpssDxChExactMatchManagerStatisticsGet
(
    IN GT_U32                                           exactMatchManagerId,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_STATISTICS_STC    *statisticsPtr
)
{
    EM_MANAGER_ID_CHECK(exactMatchManagerId);
    CPSS_NULL_PTR_CHECK_MAC(statisticsPtr);

    prvCpssDxChExactMatchManagerDbStatisticsGet(exactMatchManagerId, statisticsPtr);

    return GT_OK;
}
/**
* @internal cpssDxChExactMatchManagerStatisticsGet function
* @endinternal
*
* @brief API for fetching Exact Match Manager statistics.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
GT_STATUS cpssDxChExactMatchManagerStatisticsGet
(
    IN GT_U32                                           exactMatchManagerId,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_STATISTICS_STC    *statisticsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchManagerStatisticsGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, exactMatchManagerId,statisticsPtr));

    rc = internal_cpssDxChExactMatchManagerStatisticsGet(exactMatchManagerId,statisticsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, exactMatchManagerId,statisticsPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChExactMatchManagerStatisticsClear function
* @endinternal
*
* @brief API for clearing Exact Match Manager statistics.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager is not initialized
*/
static GT_STATUS internal_cpssDxChExactMatchManagerStatisticsClear
(
    IN GT_U32 exactMatchManagerId
)
{
    EM_MANAGER_ID_CHECK(exactMatchManagerId);

    prvCpssDxChExactMatchManagerDbStatisticsClear(exactMatchManagerId);

    return GT_OK;
}
/**
* @internal cpssDxChExactMatchManagerStatisticsClear function
* @endinternal
*
* @brief API for clearing Exact Match Manager statistics.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager is not initialized
*/
GT_STATUS cpssDxChExactMatchManagerStatisticsClear
(
    IN GT_U32 exactMatchManagerId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchManagerStatisticsClear);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, exactMatchManagerId));

    rc = internal_cpssDxChExactMatchManagerStatisticsClear(exactMatchManagerId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, exactMatchManagerId));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChExactMatchManagerDatabaseCheck function
* @endinternal
*
* @brief Run -time API that can be triggered to test various aspects
*        of Exact Match Manager. Tests includes things such as
*        internal DB, sync with HW, counters correctness etc.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
static GT_STATUS internal_cpssDxChExactMatchManagerDatabaseCheck
(
    IN  GT_U32                                                  exactMatchManagerId,
    IN  CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_STC             *checksPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT      resultArray[CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS___LAST___E],
    OUT GT_U32                                                  *errorNumberPtr
)
{
    EM_MANAGER_ID_CHECK(exactMatchManagerId);

    /* Check for valid arguments */
    CPSS_NULL_PTR_CHECK_MAC(resultArray);
    CPSS_NULL_PTR_CHECK_MAC(errorNumberPtr);
    CPSS_NULL_PTR_CHECK_MAC(checksPtr);

    return prvCpssDxChExactMatchManagerDbCheck(exactMatchManagerId, checksPtr, resultArray, errorNumberPtr);
}
/**
* @internal cpssDxChExactMatchManagerDatabaseCheck function
* @endinternal
*
* @brief Run -time API that can be triggered to test various aspects
*        of Exact Match Manager. Tests includes things such as
*        internal DB, sync with HW, counters correctness etc.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
GT_STATUS cpssDxChExactMatchManagerDatabaseCheck
(
    IN  GT_U32                                                  exactMatchManagerId,
    IN  CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_STC             *checksPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT      resultArray[CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS___LAST___E],
    OUT GT_U32                                                  *errorNumberPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchManagerDatabaseCheck);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, exactMatchManagerId,checksPtr,resultArray,errorNumberPtr));

    rc = internal_cpssDxChExactMatchManagerDatabaseCheck(exactMatchManagerId,checksPtr,resultArray,errorNumberPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, exactMatchManagerId,checksPtr,resultArray,errorNumberPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);

    return rc;
}


/**
* @internal validateDeleteParamsEntryTypeAndFilters function
* @endinternal
*
* @brief  function to validate the delete scan param.
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match Manager.
* @param[in] paramsPtr             - (pointer to) the delete scan attributes.
* @param[in] dbEntryPtr            - (pointer to) the DB entry format
* @param[out] isValidPtr           - (pointer to) the valid status.
*                                    GT_TRUE - Included in the delete scan.
*                                    GT_TRUE - Exclude from the delete scan.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS validateDeleteParamsEntryTypeAndFilters
(
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_DELETE_SCAN_PARAMS_STC    *paramsPtr,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC   *dbEntryPtr,
    OUT GT_BOOL                                                *isValidPtr
)
{
    GT_U32 sizeInBytes=0;
    GT_U32 i=0;

    *isValidPtr  = GT_FALSE;
    switch (dbEntryPtr->hwExactMatchEntryType)
    {
    case 0:
        EM_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->deleteClientEntries[CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E], GT_TRUE);
        break;
    case 1:
        EM_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->deleteClientEntries[CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E], GT_TRUE);
        break;
    case 2:
        EM_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->deleteClientEntries[CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E], GT_TRUE);
        break;
    default:
        break;
    }
    switch (dbEntryPtr->hwExactMatchLookupNum)
    {
        case 0:
            EM_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->deleteLookupEntries[CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E], GT_TRUE);
            break;
        case 1:
            EM_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->deleteLookupEntries[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E], GT_TRUE);
            break;
        default:
            break;
    }
    switch (dbEntryPtr->hwExactMatchKeySize)
    {
        case 0:
            EM_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->deleteKeySizeEntries[CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E], GT_TRUE);
            sizeInBytes = 5;
            break;
        case 1:
            EM_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->deleteKeySizeEntries[CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E], GT_TRUE);
            sizeInBytes = 19;
            break;
        case 2:
            EM_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->deleteKeySizeEntries[CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E], GT_TRUE);
            sizeInBytes = 33;
            break;
        case 3:
            EM_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->deleteKeySizeEntries[CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E], GT_TRUE);
            sizeInBytes = 47;
            break;
        default:
            break;
    }
    for (i=0;i<sizeInBytes;i++)
    {
        EM_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->mask[i], dbEntryPtr->pattern[i], paramsPtr->pattern[i]);
    }

    EM_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->userDefinedMask, dbEntryPtr->exactMatchUserDefined, paramsPtr->userDefinedPattern);

    *isValidPtr  = GT_TRUE;
    return GT_OK;
}

/**
* @internal prvCpssDxChExactMatchManagerDbEntryDelete function
* @endinternal
*
* @brief API deleting entry in all devices.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entryPtr              - (pointer to) Exact Match entry format with full info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_FOUND             - the entry was not found.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager id is not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChExactMatchManagerDbEntryDelete
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC   *dbEntryPtr
)
{
    GT_STATUS                                                           rc;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_DELETE_API_STATISTICS_ENT   deleteApiStatistics;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC            calcInfo;
    GT_U32                                                              bankId;
    GT_U32                                                              i=0;
    GT_U32                                                              sizeInBytes,line,exactMatchManagerId,ii;
    GT_BOOL                                                             updateStatistics = GT_TRUE;

    /* Fill the entry related data into calcInfo */
    cpssOsMemSet(&calcInfo, 0, sizeof(calcInfo));
    bankId = dbEntryPtr->hwIndex % exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes;
    calcInfo.dbEntryPtr                         = dbEntryPtr;
    calcInfo.selectedBankId                     = bankId;
    calcInfo.calcEntryType                      = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_EXISTS_E;
    calcInfo.bankStep                           = dbEntryPtr->hwExactMatchKeySize+1;

    for (i=0;i<calcInfo.bankStep;i++)
    {
        calcInfo.crcMultiHashArr[bankId + i] = dbEntryPtr->hwIndex + i;
    }

    /****************************************/
    /* prepare parameters for cuckoo delete */
    /****************************************/
    /* convert enum key size to size in bytes */
    EM_CONV_KEY_ENUM_TO_SIZE_IN_BYTES_MAC(dbEntryPtr->hwExactMatchKeySize,sizeInBytes);
    /* calc line from hwIndex */
    EM_LINE_FROM_HW_INDEX_GET_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes,
                                  calcInfo.crcMultiHashArr[calcInfo.selectedBankId],line);
    /* get Excat Match manager id */
    EM_MANAGER_ID_GET_MAC(exactMatchManagerPtr,exactMatchManagerId);


    /* Delete the entry in all the devices */
    rc = prvCpssDxChExactMatchManagerEntryDelete(exactMatchManagerPtr, &calcInfo, &deleteApiStatistics);
    if(deleteApiStatistics < PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_DELETE_API_STATISTICS___LAST___E)
    {
        /* increment the 'API statistics' */
        exactMatchManagerPtr->apiEntryDeleteStatisticsArr[deleteApiStatistics]++;
    }

    /******************************************************************************/
    /* deleting entry from cuckoo DB                                              */
    /******************************************************************************/
    rc = prvCpssDxChCuckooDbEntryDelete(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E, exactMatchManagerId, calcInfo.selectedBankId,line,sizeInBytes,updateStatistics);

    return rc;
}

/**
* @internal mainLogicDeleteScan function
* @endinternal
*
* @brief   The function delete the Exact Match entries according to the input attributes.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in]  exactMatchManagerId    - the Exact Match Manager id.
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
    IN  GT_U32                                                              exactMatchManagerId,
    IN  GT_BOOL                                                             scanStart,
    IN  CPSS_DXCH_EXACT_MATCH_MANAGER_DELETE_SCAN_PARAMS_STC                *paramsPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC                entriesDeletedArray[],
    OUT GT_U32                                                              *entriesDeletedNumPtr,
    OUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_DELETE_API_STATISTICS_ENT    *deleteScanStatisticsPtr
)
{
    GT_STATUS                                                    rc = GT_OK;
    GT_STATUS                                                    rc1;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC       *exactMatchManagerPtr;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC        *dbEntryPtr;
    GT_U32                                                       eventArrIndex;
    GT_BOOL                                                      isValid;
    GT_U32                                                       currEntries;

    *deleteScanStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_DELETE_API_STATISTICS_ERROR_INPUT_INVALID_E;

    /* Check for valid arguments */
    CPSS_NULL_PTR_CHECK_MAC(paramsPtr);

    /* Skip exactMatchManagerId Validation, already verified by the caller */
    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    *deleteScanStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_DELETE_API_STATISTICS___LAST___E;

    eventArrIndex = 0;
    currEntries = 0;
    rc1 = prvCpssDxChExactMatchManagerDbApplicationIteratorEntryGetNext(exactMatchManagerPtr,
                                                                 scanStart,
                                                                 &exactMatchManagerPtr->lastDeleteInfo,
                                                                 &dbEntryPtr);
    while(rc1 != GT_NO_MORE)
    {
        /* Check if entry is valid for scan delete process - Filters and Entry Types */
        rc = validateDeleteParamsEntryTypeAndFilters(paramsPtr, dbEntryPtr, &isValid);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(isValid)
        {
            /* Only update the entry to OUT event - Before delete */
            if(entriesDeletedArray != NULL)
            {
                rc = prvCpssDxChExactMatchManagerRestoreEntryFormatParamsFromDb(dbEntryPtr,
                                                                                &entriesDeletedArray[eventArrIndex].entry);

                if(rc != GT_OK)
                {
                    return rc;
                }
                entriesDeletedArray[eventArrIndex].updateType = CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_TYPE_DELETED_E;
            }
            eventArrIndex+=1;
            rc = prvCpssDxChExactMatchManagerDbEntryDelete(exactMatchManagerPtr, dbEntryPtr);
            if(rc != GT_OK)
            {
                return rc;
            }
            exactMatchManagerPtr->apiDeleteScanStatisticsArr[PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_DELETE_API_STATISTICS_TOTAL_DELETED_E]++;
        }
        currEntries+=1;
        if(currEntries >= exactMatchManagerPtr->capacityInfo.maxEntriesPerDeleteScan)
        {
            break;
        }
        rc1 = prvCpssDxChExactMatchManagerDbApplicationIteratorEntryGetNext(exactMatchManagerPtr,
                                                                            GT_FALSE,
                                                                            &exactMatchManagerPtr->lastDeleteInfo,
                                                                            &dbEntryPtr);
    }

    if(rc1 == GT_NO_MORE)
    {
        *deleteScanStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_DELETE_API_STATISTICS_OK_NO_MORE_E;
        rc = GT_NO_MORE;
    }
    else
    {
        *deleteScanStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_DELETE_API_STATISTICS_OK_E;
    }

    if(entriesDeletedNumPtr != NULL)
    {
        *entriesDeletedNumPtr = eventArrIndex;
    }
    return rc;
}

/**
* @internal internal_cpssDxChExactMatchManagerDeleteScan function
* @endinternal
*
* @brief  The function scans the entire Exact Match (Till maximal per
*         scan allowed). Delete the filtered entries.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
static GT_STATUS internal_cpssDxChExactMatchManagerDeleteScan
(
    IN  GT_U32                                                  exactMatchManagerId,
    IN  GT_BOOL                                                 exactMatchScanStart,
    IN  CPSS_DXCH_EXACT_MATCH_MANAGER_DELETE_SCAN_PARAMS_STC    *paramsPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC    entriesDeletedArray[],
    OUT GT_U32                                                  *entriesDeletedNumberPtr
)
{
    GT_STATUS                                                           rc;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_DELETE_API_STATISTICS_ENT    deleteScanStatistics;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC              *exactMatchManagerPtr;

    EM_MANAGER_ID_CHECK(exactMatchManagerId);
    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    /* main logic of Delete scan */
    rc = mainLogicDeleteScan(exactMatchManagerId, exactMatchScanStart, paramsPtr, entriesDeletedArray, entriesDeletedNumberPtr, &deleteScanStatistics);

    if(deleteScanStatistics < PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_DELETE_API_STATISTICS___LAST___E)
    {
        /* increment the 'API statistics' */
        exactMatchManagerPtr->apiDeleteScanStatisticsArr[deleteScanStatistics]++;
    }
    return rc;
}
/**
* @internal cpssDxChExactMatchManagerDeleteScan function
* @endinternal
*
* @brief  The function scans the entire Exact Match (Till maximal per
*         scan allowed). Delete the filtered entries.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
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
GT_STATUS cpssDxChExactMatchManagerDeleteScan
(
    IN  GT_U32                                                  exactMatchManagerId,
    IN  GT_BOOL                                                 exactMatchScanStart,
    IN  CPSS_DXCH_EXACT_MATCH_MANAGER_DELETE_SCAN_PARAMS_STC    *paramsPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC    entriesDeletedArray[],
    OUT GT_U32                                                  *entriesDeletedNumberPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchManagerDeleteScan);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, exactMatchManagerId,exactMatchScanStart,paramsPtr,entriesDeletedArray,entriesDeletedNumberPtr));

    rc = internal_cpssDxChExactMatchManagerDeleteScan(exactMatchManagerId,exactMatchScanStart,paramsPtr,entriesDeletedArray,entriesDeletedNumberPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, exactMatchManagerId,exactMatchScanStart,paramsPtr,entriesDeletedArray,entriesDeletedNumberPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);

    return rc;
}

/**
* @internal validateAgingParamsEntryTypeAndFilters function
* @endinternal
*
* @brief  function to validate the Aging scan param.
*
* @param[in] paramsPtr             - (pointer to) the aging scan attributes.
* @param[in] dbEntryPtr            - (pointer to) the DB entry format
* @param[out] isValidPtr           - (pointer to) the valid status.
*                                    GT_TRUE - Included in the aging scan.
*                                    GT_TRUE - Exclude from the aging scan.
* @param[out] deleteEnPtr          - (pointer to) the delete status.
*                                    GT_TRUE - Delete required if aged-out.
*                                    GT_TRUE - Do not delete even if aged-out.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
static GT_STATUS validateAgingParamsEntryTypeAndFilters
(
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_SCAN_PARAMS_STC     *paramsPtr,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC   *dbEntryPtr,
    OUT GT_BOOL                                                *isValidPtr,
    OUT GT_BOOL                                                *deleteEnPtr
)
{
    GT_U32 sizeInBytes=0;
    GT_U32 i=0;

    /* Entry Types & Not applicable filters Check
     * check if entry is included in processing                       - isValid
     * only if isValid is true, check if delete enabled for this type - deleteEn
     */
    *isValidPtr  = GT_FALSE;
    *deleteEnPtr = GT_FALSE;
    switch (dbEntryPtr->hwExactMatchEntryType)
    {
    case 0:
        EM_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->checkAgeClientEntries[CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E], GT_TRUE);
        break;
    case 1:
        EM_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->checkAgeClientEntries[CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E], GT_TRUE);
        break;
    case 2:
        EM_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->checkAgeClientEntries[CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E], GT_TRUE);
        break;
    default:
        break;
    }
    switch (dbEntryPtr->hwExactMatchLookupNum)
    {
        case 0:
            EM_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->checkAgeLookupEntries[CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E], GT_TRUE);
            break;
        case 1:
            EM_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->checkAgeLookupEntries[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E], GT_TRUE);
            break;
        default:
            break;
    }
    switch (dbEntryPtr->hwExactMatchKeySize)
    {
        case 0:
            EM_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->checkAgeKeySizeEntries[CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E], GT_TRUE);
            sizeInBytes = 5;
            break;
        case 1:
            EM_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->checkAgeKeySizeEntries[CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E], GT_TRUE);
            sizeInBytes = 19;
            break;
        case 2:
            EM_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->checkAgeKeySizeEntries[CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E], GT_TRUE);
            sizeInBytes = 33;
            break;
        case 3:
            EM_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->checkAgeKeySizeEntries[CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E], GT_TRUE);
            sizeInBytes = 47;
            break;
        default:
            break;
    }
    for (i=0;i<sizeInBytes;i++)
    {
        EM_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->mask[i], dbEntryPtr->pattern[i], paramsPtr->pattern[i]);
    }

    EM_SCAN_VALIDATE_FILTER_MAC_AND_RETURN_MAC(paramsPtr->userDefinedMask, dbEntryPtr->exactMatchUserDefined, paramsPtr->userDefinedPattern);

    /* the entry is valid for aging*/
    *isValidPtr  = GT_TRUE;

    /* now check if the entry should also be deleted */
    switch (dbEntryPtr->hwExactMatchEntryType)
    {
    case 0:
        EM_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->deleteAgeoutClientEntries[CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E], GT_TRUE);
        break;
    case 1:
        EM_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->deleteAgeoutClientEntries[CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E], GT_TRUE);
        break;
    case 2:
        EM_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->deleteAgeoutClientEntries[CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E], GT_TRUE);
        break;
    default:
        break;
    }
    switch (dbEntryPtr->hwExactMatchLookupNum)
    {
        case 0:
            EM_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->deleteAgeoutLookupEntries[CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E], GT_TRUE);
            break;
        case 1:
            EM_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->deleteAgeoutLookupEntries[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E], GT_TRUE);
            break;
        default:
            break;
    }
    switch (dbEntryPtr->hwExactMatchKeySize)
    {
        case 0:
            EM_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->deleteAgeoutKeySizeEntries[CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E], GT_TRUE);
            sizeInBytes = 5;
            break;
        case 1:
            EM_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->deleteAgeoutKeySizeEntries[CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E], GT_TRUE);
            sizeInBytes = 19;
            break;
        case 2:
            EM_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->deleteAgeoutKeySizeEntries[CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E], GT_TRUE);
            sizeInBytes = 33;
            break;
        case 3:
            EM_SCAN_VALIDATE_EXACT_MATCH_AND_RETURN_MAC(paramsPtr->deleteAgeoutKeySizeEntries[CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E], GT_TRUE);
            sizeInBytes = 47;
            break;
        default:
            break;
    }
    /* the entry is valid for age-delete */
    *deleteEnPtr = GT_TRUE;
    return GT_OK;
}

/**
* @internal prvCpssDxChExactMatchManagerAgeBitUpdate function
* @endinternal
*
* @brief   The function Updates the age-bit in the DB entry.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] ageBit                - the age bit value to be updated.
* @param[inout] dbEntryPtr         - (pointer to) the DB entry format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong DB type
*/
static GT_STATUS prvCpssDxChExactMatchManagerAgeBitUpdate
(
    IN    GT_U8                                                     ageBit,
    INOUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC     *dbEntryPtr
)
{
    dbEntryPtr->age = ageBit;
    return GT_OK;
}

/**
* @internal mainLogicAgingScan function
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
* @param[out] entriesAgedoutArray  - (pointer to) Update event array.
*                                    This is optional (can be NULL if application do not require it).
*                                    If used, should have memory for at least "max entries allowed per aging scan".
* @param[out] entriesAgedoutNumPtr - (pointer to) Number of Update events filled in array.
*                                    This is optional (can be NULL if application do not require it).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong exactMatchManagerId and paramsPtr data
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*/
static GT_STATUS mainLogicAgingScan
(
    IN  GT_U32                                                          exactMatchManagerId,
    IN  CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_SCAN_PARAMS_STC             *paramsPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC            entriesAgedoutArray[],
    OUT GT_U32                                                          *entriesAgedoutNumPtr,
    OUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_AGE_API_STATISTICS_ENT   *agingScanStatisticsPtr
)
{
    GT_STATUS                                                    rc = GT_OK;
    GT_STATUS                                                    rc1;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC       *exactMatchManagerPtr;
    GT_U32                                                       curAgeBinId;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC        *dbEntryPtr;
    GT_U32                                                       eventArrIndex;
    GT_BOOL                                                      outEventUpdateEn;
    GT_BOOL                                                      isValid;
    GT_BOOL                                                      deleteEn;
    GT_BOOL                                                      agedOut;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_NODE_PTR_STC            lastGetNextInfo;

    *agingScanStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_AGE_API_STATISTICS_ERROR_INPUT_INVALID_E;

    /* Check for valid arguments */
    CPSS_NULL_PTR_CHECK_MAC(paramsPtr);

    EM_MANAGER_ID_CHECK(exactMatchManagerId);
    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    *agingScanStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_AGE_API_STATISTICS___LAST___E;

    /* Get entry from the age-bin */
    curAgeBinId = exactMatchManagerPtr->agingBinInfo.currentAgingScanBinID;
    EM_MANAGER_AGE_BIN_ID_CHECK(curAgeBinId);

    outEventUpdateEn = (entriesAgedoutArray != NULL)?GT_TRUE:GT_FALSE;

    eventArrIndex = 0;
    rc1 = prvCpssDxChExactMatchManagerDbAgeBinEntryGetNext(exactMatchManagerPtr,
                                                           GT_TRUE,
                                                           curAgeBinId,
                                                           &lastGetNextInfo,
                                                           &dbEntryPtr);
    while(rc1 != GT_NO_MORE)
    {
        /* Check if entry is valid for aging process - Filters and Entry Types
         * Set deleteEn, if delete flag is enabled in param filter */
        rc = validateAgingParamsEntryTypeAndFilters(paramsPtr, dbEntryPtr, &isValid, &deleteEn);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(isValid)
        {
            /* Check if entry is aged-out in all device */
            rc = prvCpssDxChExactMatchManagerHwAgedOutVerify(exactMatchManagerPtr, dbEntryPtr, &agedOut);
            if(rc != GT_OK)
            {
                return rc;
            }
            if(agedOut)
            {
                /* Only update the entry to OUT event - Before delete */
                if(outEventUpdateEn)
                {
                    rc = prvCpssDxChExactMatchManagerRestoreEntryFormatParamsFromDb(dbEntryPtr,
                                                                                    &entriesAgedoutArray[eventArrIndex].entry);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                    if(deleteEn)
                    {
                        entriesAgedoutArray[eventArrIndex].updateType = CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_TYPE_AGED_DELETED_E;
                    }
                    else
                    {
                        entriesAgedoutArray[eventArrIndex].updateType = CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_TYPE_AGED_OUT_E;
                    }
                    eventArrIndex+=1;
                }
                if(deleteEn)
                {
                    rc = prvCpssDxChExactMatchManagerDbEntryDelete(exactMatchManagerPtr, dbEntryPtr);
                    if(rc != GT_OK)
                    {
                        /* should not happen - DB probably corrupted */
                        return rc;
                    }
                    exactMatchManagerPtr->apiAgingScanStatisticsArr[PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_AGE_API_STATISTICS_TOTAL_AGEDOUT_DELETED_E]++;
                }
                /* Update DB that entry - aged-out (If entry if not deleted)*/
                else
                {
                    rc = prvCpssDxChExactMatchManagerAgeBitUpdate(0, dbEntryPtr);/* ageBit=0 */
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                    exactMatchManagerPtr->apiAgingScanStatisticsArr[PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_AGE_API_STATISTICS_TOTAL_AGEDOUT_E]++;
                }
            }
            /* Update DB that entry - Refreshed */
            else
            {
                rc = prvCpssDxChExactMatchManagerAgeBitUpdate(1, dbEntryPtr);/* ageBit=1 */
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
        rc1 = prvCpssDxChExactMatchManagerDbAgeBinEntryGetNext(exactMatchManagerPtr,
                                                               GT_FALSE,
                                                               curAgeBinId,
                                                               &lastGetNextInfo,
                                                               &dbEntryPtr);
    }

    exactMatchManagerPtr->agingBinInfo.currentAgingScanBinID = (curAgeBinId + 1) % exactMatchManagerPtr->agingBinInfo.totalAgeBinAllocated;
    if(entriesAgedoutNumPtr != NULL)
    {
        *entriesAgedoutNumPtr = eventArrIndex;
    }
    *agingScanStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_AGE_API_STATISTICS_OK_E;
    return rc;
}

/**
* @internal internal_cpssDxChExactMatchManagerAgingScan function
* @endinternal
*
* @brief   The function scans the entire Exact Match and process age-out for the filtered entries.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
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
static GT_STATUS internal_cpssDxChExactMatchManagerAgingScan
(
    IN  GT_U32                                                  exactMatchManagerId,
    IN  CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_SCAN_PARAMS_STC     *paramsPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC    entriesAgedOutArray[],
    OUT GT_U32                                                  *entriesAgedOutNumPtr
)
{
    GT_STATUS                                                       rc;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_AGE_API_STATISTICS_ENT   agingScanStatistics;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC          *exactMatchManagerPtr;

    EM_MANAGER_ID_CHECK(exactMatchManagerId);
    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    /* main logic of aging scan */
    rc = mainLogicAgingScan(exactMatchManagerId, paramsPtr, entriesAgedOutArray, entriesAgedOutNumPtr, &agingScanStatistics);

    if(agingScanStatistics < PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_AGE_API_STATISTICS___LAST___E)
    {
        /* increment the 'API statistics' */
        exactMatchManagerPtr->apiAgingScanStatisticsArr[agingScanStatistics]++;
    }

    return rc;
}
/**
* @internal cpssDxChExactMatchManagerAgingScan function
* @endinternal
*
* @brief   The function scans the entire Exact Match and process age-out for the filtered entries.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
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
GT_STATUS cpssDxChExactMatchManagerAgingScan
(
    IN  GT_U32                                                  exactMatchManagerId,
    IN  CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_SCAN_PARAMS_STC     *paramsPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC    entriesAgedOutArray[],
    OUT GT_U32                                                  *entriesAgedOutNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchManagerAgingScan);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, exactMatchManagerId,paramsPtr,entriesAgedOutArray,entriesAgedOutNumPtr));

    rc = internal_cpssDxChExactMatchManagerAgingScan(exactMatchManagerId,paramsPtr,entriesAgedOutArray,entriesAgedOutNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, exactMatchManagerId,paramsPtr,entriesAgedOutArray,entriesAgedOutNumPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);

    return rc;
}


/**
* @internal mainLogicEntriesRewrite function
* @endinternal
*
* @brief   The function rewrites Exact Match Manager HW entries according to SW DB.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match Manager.
* @param[in] entriesIndexesArray   - The array of HW index.
* @param[in] entriesIndexesNum     - number of HW indexes present in index array.
* @param[out] rewriteApiStatisticsPtr - (pointer to) the type of API ok/error statistics.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong exactMatchManagerId data
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS mainLogicEntriesRewrite
(
    IN  PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC                  *exactMatchManagerPtr,
    IN  GT_U32                                                                  entriesIndexesArray[],
    IN  GT_U32                                                                  entriesIndexesNum,
    OUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRIES_REWRITE_API_STATISTICS_ENT    *rewriteApiStatisticsPtr
)
{
    GT_STATUS                                                       rc;
    GT_U32                                                          hwIndex;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC        calcInfo;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INDEX_STC                  *dbIndexPtr;
    GT_U32                                                          indexNum;

    *rewriteApiStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRIES_REWRITE_API_STATISTICS_ERROR_INPUT_INVALID_E;
    CPSS_NULL_PTR_CHECK_MAC(entriesIndexesArray);

    /* For each incoming hwIndex:
     *    - Fill the calcInfo with
     *        bankstep, selectedBankId, entryPtr
     *    - Set entries as refreshed - ageBit = 1
     *    - Get key & data entry
     *    - Write entry to each tile of each device
     */
    for(indexNum=0; indexNum<entriesIndexesNum; indexNum++)
    {
        hwIndex = entriesIndexesArray[indexNum];
        EM_MANAGER_HW_INDEX_CHECK(hwIndex, exactMatchManagerPtr);
        if(exactMatchManagerPtr->indexArr[hwIndex].isValid_entryPointer)
        {
            dbIndexPtr              = &exactMatchManagerPtr->indexArr[hwIndex];
            calcInfo.dbEntryPtr     = &exactMatchManagerPtr->entryPoolPtr[dbIndexPtr->entryPointer];

            /* if this is a first index indication then we can work with this
               if not we need to find the first index coresponding to the index we got */
            while(calcInfo.dbEntryPtr->isFirst==GT_FALSE)
            {
                /* go reverse untill first index is found */
                if (hwIndex!=0)
                {
                     hwIndex--;
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "FAIL: unexpected case,hwIndex is 0 and and can not be deducted \n");
                }
                 dbIndexPtr              = &exactMatchManagerPtr->indexArr[hwIndex];
                 calcInfo.dbEntryPtr     = &exactMatchManagerPtr->entryPoolPtr[dbIndexPtr->entryPointer];
            }

            switch(calcInfo.dbEntryPtr->hwExactMatchKeySize)
            {
            case 0: /* CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E */
                    calcInfo.bankStep        = 1;
                    break;
                case 1: /* CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E */
                    calcInfo.bankStep       = 2;
                    break;
                case 2: /* CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E */
                    calcInfo.bankStep       = 3;
                    break;
                case 3: /* CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E */
                    calcInfo.bankStep       = 4;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(calcInfo.dbEntryPtr->hwExactMatchKeySize);
            }

            /* set entry to be as refreshed - meaning it wont be deleted in coming pass */
            calcInfo.dbEntryPtr->age = 1;

            calcInfo.calcEntryType  = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_EXISTS_E;
            calcInfo.selectedBankId = hwIndex % exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes;

            /***************************************************************************/
            /* write the entry to HW of all registered devices                         */
            /***************************************************************************/
            rc = prvCpssDxChExactMatchManagerHwWriteByHwIndex(exactMatchManagerPtr, &calcInfo);
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
            rc = prvCpssDxChExactMatchManagerHwFlushByHwIndex(exactMatchManagerPtr,hwIndex);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
        exactMatchManagerPtr->apiEntriesRewriteStatisticsArr[PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRIES_REWRITE_API_STATISTICS_TOTAL_REWRITE_E]+=1;
    }
    *rewriteApiStatisticsPtr = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRIES_REWRITE_API_STATISTICS_OK_E;
    return GT_OK;
}

/**
* @internal internal_cpssDxChExactMatchManagerEntryRewrite function
* @endinternal
*
* @brief   The function rewrites Exact Match Manager HW entries according to SW DB.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - The Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entriesIndexesArray   - The array of HW index.
* @param[in] entriesIndexesNum     - number of HW indexes present in index array.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong exactMatchManagerId data
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChExactMatchManagerEntryRewrite
(
    IN  GT_U32      exactMatchManagerId,
    IN  GT_U32      entriesIndexesArray[],
    IN  GT_U32      entriesIndexesNum
)
{
    GT_STATUS                                                               rc;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRIES_REWRITE_API_STATISTICS_ENT    rewriteApiStatistics;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC                  *exactMatchManagerPtr;

    EM_MANAGER_ID_CHECK(exactMatchManagerId); /* must be done here because used in 'API' statistics */

    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    /* call the main logic */
    rc = mainLogicEntriesRewrite(exactMatchManagerPtr, entriesIndexesArray, entriesIndexesNum, &rewriteApiStatistics);

    if(rewriteApiStatistics <
        PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRIES_REWRITE_API_STATISTICS___LAST___E)
    {
        /* increment the 'API statistics' */
        exactMatchManagerPtr->apiEntriesRewriteStatisticsArr[rewriteApiStatistics]++;
    }
    return rc;
}

/*
* @internal prvCpssDxChExactMatchManagerEntryGetFromHw function
* @endinternal
*
* @brief   The function return entry info from the manager by 'key'
*          NOTE: the function do HW accessing.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entryPtr              - (pointer to) Exact Match entry format with the 'exactMatchEntryKey'.
*                                     NOTE: only the 'exactMatchEntryKey' part is used by the API.(as [in] parameter)
* @param[out] entryPtr             - (pointer to) Exact Match entry format with full info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_FOUND             - if entry not found for that 'key'.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager id is not initialized.
*
* @note
*   NONE
*
*/
GT_STATUS prvCpssDxChExactMatchManagerEntryGetFromHw
(
    IN      GT_U32                                        exactMatchManagerId,
    INOUT   CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC       *entryPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC calcInfo;

    EM_MANAGER_ID_CHECK(exactMatchManagerId);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    /********************************/
    /* validity checks of SW format */
    /********************************/
    rc = prvCpssDxChExactMatchManagerValidateEntryKeyFormatParams(entryPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*******************************************/
    /* calculate where 'key' is expected in DB */
    /*******************************************/
    rc = prvCpssDxChExactMatchManagerDbCalcHashAndLookupResult(exactMatchManagerPtr,entryPtr,NULL,&calcInfo);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(calcInfo.calcEntryType != PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_EXISTS_E)
    {
        /* the 'key' was not found in the DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND,LOG_ERROR_NO_MSG);
    }

    /* do HW read in order to invoke data integrity events -
       this function is used for SER and have no output */
    rc = prvCpssDxChExactMatchManagerHwReadByHwIndex(exactMatchManagerPtr,&calcInfo);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* convert DB format to application format */
    rc = prvCpssDxChExactMatchManagerRestoreEntryFormatParamsFromDb(calcInfo.dbEntryPtr, entryPtr);
    if(rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

/**
* @internal cpssDxChExactMatchManagerEntryRewrite function
* @endinternal
*
* @brief   The function rewrites Exact Match Manager HW entries according to SW DB.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - The Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] entriesIndexesArray   - The array of HW index.
* @param[in] entriesIndexesNum     - number of HW indexes present in index array.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong exactMatchManagerId data
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChExactMatchManagerEntryRewrite
(
    IN  GT_U32 exactMatchManagerId,
    IN  GT_U32 entriesIndexesArray[], /* arrSizeVarName=entriesIndexesNum */
    IN  GT_U32 entriesIndexesNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChExactMatchManagerEntryRewrite);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, exactMatchManagerId, entriesIndexesArray, entriesIndexesNum));

    rc = internal_cpssDxChExactMatchManagerEntryRewrite(exactMatchManagerId, entriesIndexesArray, entriesIndexesNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, exactMatchManagerId, entriesIndexesArray, entriesIndexesNum));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);

    return rc;
}
