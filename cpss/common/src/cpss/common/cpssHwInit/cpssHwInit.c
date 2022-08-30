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
* @file cpssHwInit.c
*
* @brief Includes CPSS level basic HW initialization functions.
*
* @version   43
********************************************************************************
*/
#ifdef CHX_FAMILY
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/generic/trunk/private/prvCpssTrunkTypes.h>
#endif /*CHX_FAMILY*/
#include <cpss/common/cpssHwInit/private/prvCpssCommonCpssHwInitLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/cpssHwInit/cpssHwInit.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <cpss/driver/interrupts/cpssDrvComIntSvcRtn.h>
#include <cpss/common/cpssHwInit/private/prvCpssHwInit.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxExMxInterrupts.h>
#include <cpss/common/systemRecovery/private/prvCpssCommonSystemRecovery.h>

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*global variables macros*/

#define HWINIT_GLOVAR(_var) \
    PRV_SHARED_GLOBAL_VAR_GET(commonMod.genericHwInitDir._var)

/* macro to free global pointer (given by the 'path') */
#define FREE_GLOBAL_PTR_MAC(path) FREE_PTR_MAC(PRV_SHARED_GLOBAL_VAR_GET(path))

#define PRV_CPSS_GEN_NETIF_MII_DEV_NOT_SET 0xFFFFFFFF

#ifdef CPSS_API_LOCK_PROTECTION
#define PRV_CPSS_API_MUTEX_LOCK_BY_DEV_MAX_NAME_SIZE 60

#endif

#define BASE_ADDR_NOT_USED_CNS 0xffffffff

/**
* @internal prvCpssHwIfSet function
* @endinternal
*
* @brief   This function sets cpss HW interface parameters.
*         called during "phase1" initialization .
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] mngInterfaceType         - Management interface type
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - on bad device number or interface type
*/
GT_STATUS prvCpssHwIfSet(
    IN GT_U8                            devNum,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT    mngInterfaceType
)
{
    if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((GT_U32)mngInterfaceType > 31)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_PP_MAC(devNum)->hwIfSupportedBmp = PRV_CPSS_HW_IF_BMP_MAC(mngInterfaceType);

    if(CPSS_CHANNEL_IS_PEX_MAC(mngInterfaceType))
    {
        /* in most cases we will treat the "PCI" and "PEX" the same */
        PRV_CPSS_PP_MAC(devNum)->hwIfSupportedBmp |= PRV_CPSS_HW_IF_BMP_MAC(CPSS_CHANNEL_PCI_E);
    }

    if(CPSS_CHANNEL_IS_PEX_MBUS_MAC(mngInterfaceType))
    {
        /* in most cases we will treat the "PCI" and "PEX" and "PEX_MBUS" the same */
        PRV_CPSS_PP_MAC(devNum)->hwIfSupportedBmp |= PRV_CPSS_HW_IF_BMP_MAC(CPSS_CHANNEL_PCI_E);
        PRV_CPSS_PP_MAC(devNum)->hwIfSupportedBmp |= PRV_CPSS_HW_IF_BMP_MAC(CPSS_CHANNEL_PEX_E);
    }

    return GT_OK;

}

/**
* @internal prvCpssNonMultiPortGroupsDeviceSet function
* @endinternal
*
* @brief   This function sets cpss DB to be 'non-multi-port-groups' device.
*         It is called during "phase1" initialization.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - on bad device number
*/
GT_STATUS prvCpssNonMultiPortGroupsDeviceSet(
    IN GT_U8                            devNum
)
{
    GT_U32              firstActivePortGroupId;/* port group Id for the first active port group */
    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr;/* pointer to common device info */
    GT_U32              ii,jj;   /* iterator */

    if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    devPtr = PRV_CPSS_PP_MAC(devNum);

    /* fill the 'multi port groups' info for the simplicity of the macros that
       needed to support loops on port group !!! */
    devPtr->portGroupsInfo.isMultiPortGroupDevice = GT_FALSE;
    devPtr->portGroupsInfo.numOfPortGroups = 1;
    firstActivePortGroupId = CPSS_NON_MULTI_PORT_GROUP_DEVICE_INDEX_CNS;
    devPtr->portGroupsInfo.activePortGroupsBmp        = 1 << firstActivePortGroupId;
    devPtr->portGroupsInfo.firstActivePortGroup       = firstActivePortGroupId;
    devPtr->portGroupsInfo.lastActivePortGroup        = firstActivePortGroupId;
    devPtr->portGroupsInfo.cpuPortPortGroupId         = firstActivePortGroupId;
    devPtr->portGroupsInfo.auqPortGroupIdLastServed   = firstActivePortGroupId;
    devPtr->portGroupsInfo.fuqPortGroupIdLastServed   = firstActivePortGroupId;
    devPtr->portGroupsInfo.secureBreachNextPortGroupToServe = firstActivePortGroupId;
    devPtr->portGroupsInfo.ptpNextPortGroupToServe    = firstActivePortGroupId;
    devPtr->portGroupsInfo.phaViolationCapturedNextPortGroupToServe = firstActivePortGroupId;
    for (ii = 0; ii < PRV_CPSS_MAX_DIRECTIONS_CNS ; ii ++)
    {
        for (jj = 0; jj < PRV_CPSS_MAX_PTP_TIMESTAMP_QUEUES_CNS; jj++)
        {
              devPtr->portGroupsInfo.ptpIngressEgressNextPortGroupToServe[ii][jj] = firstActivePortGroupId;
        }
    }

    /* from this point on we can start use the macros of:
       PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC,
       PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC
    */

    return GT_OK;
}



/**
* @internal initDb function
* @endinternal
*
* @brief   This function set the DB of cpss of 'System level' - to 'init' stage .
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success,
*/
static GT_STATUS initDb
(
    void
)
{
    GT_U32  ii;
    GT_STATUS rc;
    CPSS_SYSTEM_RECOVERY_ERROR_INJECTION_STC systemRecoveryErrorInjection;

    /* initializing the HA per PP DB */
    cpssOsMemSet(
        &(HWINIT_GLOVAR(sysGenGlobalInfo.cpssPpHaDevData)),
        0, sizeof(HWINIT_GLOVAR(sysGenGlobalInfo.cpssPpHaDevData)));
    cpssOsMemSet(
        PRV_CPSS_PP_CONFIG_ARR_MAC,
        0, sizeof(PRV_CPSS_PP_CONFIG_ARR_MAC));
    /* initializing the per FAMILY DB */
#ifdef CHX_FAMILY
    cpssOsMemSet(
        &(HWINIT_GLOVAR(prvCpssFamilyInfoArray)),
        0, sizeof(HWINIT_GLOVAR(prvCpssFamilyInfoArray)));
    HWINIT_GLOVAR(sysGenGlobalInfo.prvCpssGenNetIfMiiCtrl.rxValid) = GT_FALSE;
    HWINIT_GLOVAR(sysGenGlobalInfo.prvCpssGenNetIfMiiCtrl.txValid) = GT_FALSE;
#endif /*CHX_FAMILY*/

    HWINIT_GLOVAR(sysGenGlobalInfo.prvMiiDevNum) = PRV_CPSS_GEN_NETIF_MII_DEV_NOT_SET;

    if(HWINIT_GLOVAR(sysGenGlobalInfo.prvCpssAfterDbRelease) == GT_FALSE)
    {
        /* initialize only once */
        for (ii = 0 ; ii < PRV_CPSS_MAX_PP_DEVICES_CNS; ii++)
        {
            HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[ii].busBaseAddr) = BASE_ADDR_NOT_USED_CNS;
            HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[ii].deviceDidHwReset) = GT_TRUE;
            HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[ii].deviceNeedNetIfInit) = GT_TRUE;
            HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[ii].dmaInfoValid) = GT_FALSE;
        }
    }

    HWINIT_GLOVAR(sysGenGlobalInfo.supportDualDeviceId) = GT_FALSE;

    /* initializing the systemRecoveryErrorInjection */
    cpssOsMemSet(&systemRecoveryErrorInjection,0,sizeof(systemRecoveryErrorInjection));
    rc = prvCpssSystemRecoveryErrorInjectionSet(systemRecoveryErrorInjection);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssSysConfigPhase1 function
* @endinternal
*
* @brief   This function sets cpss system level system configuration parameters
*         before any of the device's phase1 initialization .
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success,
*/
GT_STATUS prvCpssSysConfigPhase1
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
#ifdef CPSS_API_LOCK_PROTECTION
    GT_U8  i,j;
    char   prvCpssApiLockByDevNumMtxName[PRV_CPSS_API_MUTEX_LOCK_BY_DEV_MAX_NAME_SIZE];
    char   suffix[20];
#endif

#ifdef CPSS_API_LOCK_PROTECTION

    rc = cpssOsMutexCreate(
        "mtxFdbMng",&(PRV_SHARED_GLOBAL_VAR_GET(
            commonMod.genericHwInitDir.prvCpssApiLockDevicelessMtx[PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS])));
    if(rc != GT_OK)
    {
        return rc;
    }
#if defined  CPSS_USE_MUTEX_PROFILER

    rc = cpssOsMutexSetGlAttributes(
        PRV_SHARED_GLOBAL_VAR_GET(
            commonMod.genericHwInitDir.prvCpssApiLockDevicelessMtx[PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS]),
        GT_FALSE,GT_FALSE,GT_FALSE,0,
        GT_FALSE,GT_TRUE,PRV_CPSS_FUNCTIONALITY_FDB_MANAGER_CNS);
    if(rc != GT_OK)
    {
        return rc;
    }
#endif

    rc = cpssOsMutexCreate(
        "mtxEmMng",&(PRV_SHARED_GLOBAL_VAR_GET(
            commonMod.genericHwInitDir.prvCpssApiLockDevicelessMtx[PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS])));
    if(rc != GT_OK)
    {
        return rc;
    }
#if defined  CPSS_USE_MUTEX_PROFILER

    rc = cpssOsMutexSetGlAttributes(
        PRV_SHARED_GLOBAL_VAR_GET(
            commonMod.genericHwInitDir.prvCpssApiLockDevicelessMtx[PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS]),
        GT_FALSE,GT_FALSE,GT_FALSE,0,
        GT_FALSE,GT_TRUE,PRV_CPSS_FUNCTIONALITY_EXACT_MATCH_MANAGER_CNS);
    if(rc != GT_OK)
    {
        return rc;
    }
#endif

    rc = cpssOsMutexCreate(
        "mtxLpmMng",
        &(PRV_SHARED_GLOBAL_VAR_GET(
            commonMod.genericHwInitDir.prvCpssApiLockDevicelessMtx[PRV_CPSS_FUNCTIONALITY_LPM_MANAGER_CNS])));
    if(rc != GT_OK)
    {
        return rc;
    }
#if defined  CPSS_USE_MUTEX_PROFILER

    rc = cpssOsMutexSetGlAttributes(
        PRV_SHARED_GLOBAL_VAR_GET(
            commonMod.genericHwInitDir.prvCpssApiLockDevicelessMtx[PRV_CPSS_FUNCTIONALITY_LPM_MANAGER_CNS]),
        GT_FALSE,GT_FALSE,GT_FALSE,0,
        GT_FALSE,GT_TRUE,PRV_CPSS_FUNCTIONALITY_LPM_MANAGER_CNS);
    if(rc != GT_OK)
    {
        return rc;
    }
#endif

    rc = cpssOsMutexCreate(
        "mtxTcamMng",
        &PRV_SHARED_GLOBAL_VAR_GET(
            commonMod.genericHwInitDir.prvCpssApiLockDevicelessMtx[PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS]));
    if(rc != GT_OK)
    {
        return rc;
    }
#if defined  CPSS_USE_MUTEX_PROFILER

    rc = cpssOsMutexSetGlAttributes(
        PRV_SHARED_GLOBAL_VAR_GET(
            commonMod.genericHwInitDir.prvCpssApiLockDevicelessMtx[PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS]),
        GT_FALSE,GT_FALSE,GT_FALSE,0,
        GT_FALSE,GT_TRUE,PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);
    if(rc != GT_OK)
    {
        return rc;
    }
#endif

    rc = cpssOsMutexCreate(
        "mtxSysRecovery",
        &PRV_SHARED_GLOBAL_VAR_GET(
            commonMod.genericHwInitDir.prvCpssApiLockDevicelessMtx[PRV_CPSS_FUNCTIONALITY_SYSTEM_RECOVERY_CNS]));
    if(rc != GT_OK)
    {
        return rc;
    }
#if defined  CPSS_USE_MUTEX_PROFILER

    rc = cpssOsMutexSetGlAttributes(
        PRV_SHARED_GLOBAL_VAR_GET(
            commonMod.genericHwInitDir.prvCpssApiLockDevicelessMtx[PRV_CPSS_FUNCTIONALITY_SYSTEM_RECOVERY_CNS]),
        GT_FALSE,GT_FALSE,GT_FALSE,0,
        GT_TRUE,GT_FALSE,0);
    if(rc != GT_OK)
    {
        return rc;
    }
#endif

    rc = cpssOsMutexCreate(
        "mtxDevLessConf",
        &PRV_SHARED_GLOBAL_VAR_GET(
            commonMod.genericHwInitDir.prvCpssApiLockDevicelessMtx[PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS]));
    if(rc != GT_OK)
    {
        return rc;
    }
#if defined  CPSS_USE_MUTEX_PROFILER

    rc = cpssOsMutexSetGlAttributes(
        PRV_SHARED_GLOBAL_VAR_GET(
            commonMod.genericHwInitDir.prvCpssApiLockDevicelessMtx[PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS]),
        GT_FALSE,GT_FALSE,GT_FALSE,0,
        GT_FALSE,GT_FALSE,0);
    if(rc != GT_OK)
    {
        return rc;
    }
#endif

    rc = cpssOsMutexCreate(
        "mtxDevLessRxTx",
        &PRV_SHARED_GLOBAL_VAR_GET(
            commonMod.genericHwInitDir.prvCpssApiLockDevicelessMtx[PRV_CPSS_FUNCTIONALITY_RXTX_CNS]));
    if(rc != GT_OK)
    {
        return rc;
    }
#if defined  CPSS_USE_MUTEX_PROFILER

    rc = cpssOsMutexSetGlAttributes(
        PRV_SHARED_GLOBAL_VAR_GET(
            commonMod.genericHwInitDir.prvCpssApiLockDevicelessMtx[PRV_CPSS_FUNCTIONALITY_RXTX_CNS]),
        GT_FALSE,GT_FALSE,GT_TRUE,0,
        GT_FALSE,GT_FALSE,0);
    if(rc != GT_OK)
    {
        return rc;
    }
#endif

    cpssOsMemSet(prvCpssApiLockByDevNumMtxName,'\0',PRV_CPSS_API_MUTEX_LOCK_BY_DEV_MAX_NAME_SIZE);
    for(i = 0;
         i < (sizeof(PRV_SHARED_GLOBAL_VAR_GET(
                    commonMod.genericHwInitDir.prvCpssApiLockByDevNumAndFunctionalityMtx))
            / sizeof(PRV_SHARED_GLOBAL_VAR_GET(
                    commonMod.genericHwInitDir.prvCpssApiLockByDevNumAndFunctionalityMtx[0])));
         i++)
    {
        for(j=0; j<PRV_CPSS_MAX_DEV_FUNCTIONALITY_RESOLUTION_CNS; j++)
        {
            cpssOsStrCpy(prvCpssApiLockByDevNumMtxName, "mtx");
            switch (j)
            {
            case PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS:
                cpssOsSprintf(suffix, "%s%d","ConfDev",i);
                break;
            case PRV_CPSS_FUNCTIONALITY_RXTX_CNS:
                cpssOsSprintf(suffix, "%s%d","RxTxDev",i);
                break;
            case PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRUNK_MANAGER_CNS:
                cpssOsSprintf(suffix, "%s%d","TrunkMDev",i);
                break;
            case PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS:
                cpssOsSprintf(suffix, "%s%d","IpfixMDev",i);
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
            cpssOsStrCat(prvCpssApiLockByDevNumMtxName,suffix);
            rc = cpssOsMutexCreate(
                prvCpssApiLockByDevNumMtxName,
                &PRV_SHARED_GLOBAL_VAR_GET(
                    commonMod.genericHwInitDir.prvCpssApiLockByDevNumAndFunctionalityMtx[i][j]));
            if(rc != GT_OK)
            {
                return rc;
            }

#if defined CPSS_USE_MUTEX_PROFILER
            if (j==PRV_CPSS_FUNCTIONALITY_RXTX_CNS)
            {
                rc = cpssOsMutexSetGlAttributes(
                    PRV_SHARED_GLOBAL_VAR_GET(
                    commonMod.genericHwInitDir.prvCpssApiLockByDevNumAndFunctionalityMtx[i][j]),
                    GT_FALSE, GT_TRUE, GT_TRUE, i, GT_FALSE,GT_FALSE,0);
            }
            else
            {
                if (j==PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRUNK_MANAGER_CNS)
                {
                    rc = cpssOsMutexSetGlAttributes(
                        PRV_SHARED_GLOBAL_VAR_GET(
                            commonMod.genericHwInitDir.prvCpssApiLockByDevNumAndFunctionalityMtx[i][j]),
                        GT_FALSE, GT_TRUE, GT_FALSE, i,
                        GT_FALSE,GT_TRUE,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRUNK_MANAGER_CNS);
                }
                else if (j==PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS)
                {
                    rc = cpssOsMutexSetGlAttributes(
                        PRV_SHARED_GLOBAL_VAR_GET(
                            commonMod.genericHwInitDir.prvCpssApiLockByDevNumAndFunctionalityMtx[i][j]),
                        GT_FALSE, GT_TRUE, GT_FALSE, i,
                        GT_FALSE,GT_TRUE,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);
                }
                else
                {
                    rc = cpssOsMutexSetGlAttributes(
                        PRV_SHARED_GLOBAL_VAR_GET(
                            commonMod.genericHwInitDir.prvCpssApiLockByDevNumAndFunctionalityMtx[i][j]),
                        GT_FALSE, GT_TRUE, GT_FALSE, i, GT_FALSE,GT_FALSE,0);
                }
            }
            if(rc != GT_OK)
            {
                return rc;
            }
#endif
        }
    }

    rc = cpssOsMutexCreate(
        "mtxZeroLevel",
        &PRV_SHARED_GLOBAL_VAR_GET(
            commonMod.genericHwInitDir.prvCpssApiLockDevicelessMtxZeroLevel));
    if(rc != GT_OK)
    {
       return rc;
    }
#if defined  CPSS_USE_MUTEX_PROFILER
    rc = cpssOsMutexSetGlAttributes(
        PRV_SHARED_GLOBAL_VAR_GET(
            commonMod.genericHwInitDir.prvCpssApiLockDevicelessMtxZeroLevel),
        GT_TRUE,GT_FALSE,GT_FALSE,0, GT_FALSE,GT_FALSE,0);
    if(rc != GT_OK)
    {
        return rc;
    }

#endif

#endif
    /* set DB to init stage */
    rc = initDb();
    if(rc != GT_OK)
    {
        return rc;
    }

    if (! PRV_SHARED_GLOBAL_VAR_GET(commonMod.genericHwInitDir.prvCpssLockSem))
    {
        /* initialize CPSS mutex for task lock */
        rc = cpssOsMutexCreate(
            "prvCpssLockSem",
            &PRV_SHARED_GLOBAL_VAR_GET(commonMod.genericHwInitDir.prvCpssLockSem));
    }

    PRV_SHARED_GLOBAL_VAR_SET(sysConfigPhase1Done,GT_TRUE);


    return rc;
}

#ifdef CHX_FAMILY
static void free_PRV_CPSS_DXCH_TABLES_INFO_PER_FAMILY_STC(
    IN PRV_CPSS_DXCH_TABLES_INFO_PER_FAMILY_STC*    perFamPtr
)
{
    FREE_PTR_MAC(perFamPtr->tablesInfoArr);
    FREE_PTR_MAC(perFamPtr->directAccessTableInfoArr);
}
#endif /*CHX_FAMILY*/
/**
* @internal prvCpssSysConfigDestroy function
* @endinternal
*
* @brief   This function destroy cpss system level system configuration parameters
*         after all device removed .
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success,
*/
GT_STATUS prvCpssSysConfigDestroy
(
    GT_VOID
)
{
    GT_STATUS rc;
    PRV_CPSS_DRV_DEVICE_INTERRUPTS_INFO_STC * devInterruptInfoPtr;
    PRV_CPSS_DRV_FAMILY_ENT intFamily;

#ifdef CPSS_API_LOCK_PROTECTION
    GT_U32  i,j;
#endif

    /* Multiple calls of cpssPpDestroy end up calling this function with NULL ptr
     * which is a wrong scenario hence return GT_NOT_INITIALIZED
     */
    if (cpssSharedGlobalVarsPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if (PRV_SHARED_GLOBAL_VAR_GET(commonMod.genericHwInitDir.prvCpssLockSem) != 0)
    {
        /* destroy CPSS mutex for task lock */
        (void)cpssOsMutexDelete(PRV_SHARED_GLOBAL_VAR_GET(commonMod.genericHwInitDir.prvCpssLockSem));
        PRV_SHARED_GLOBAL_VAR_GET(commonMod.genericHwInitDir.prvCpssLockSem) = 0;
    }

#ifdef CHX_FAMILY
    /* free dynamically allocated global DBs */
    FREE_GLOBAL_PTR_MAC(mainPpDrvMod.dxChCpssHwInit.tablesSrc.dxChXcat3TablesInfoPtr);
    FREE_GLOBAL_PTR_MAC(mainPpDrvMod.dxChCpssHwInit.tablesSrc.lion2TablesInfoPtr);
    FREE_GLOBAL_PTR_MAC(mainPpDrvMod.dxChCpssHwInit.tablesSrc.bobcat2TablesInfoPtr);
    FREE_GLOBAL_PTR_MAC(mainPpDrvMod.dxChCpssHwInit.tablesSrc.bobkTablesInfoPtr);
    FREE_GLOBAL_PTR_MAC(mainPpDrvMod.dxChCpssHwInit.tablesSrc.directAccessBobcat2TableInfoPtr);
    FREE_GLOBAL_PTR_MAC(mainPpDrvMod.dxChCpssHwInit.tablesSrc.bobcat2B0TablesInfo_overrideA0Ptr);
    FREE_GLOBAL_PTR_MAC(mainPpDrvMod.dxChCpssHwInit.tablesSrc.directAccessBobkTableInfoPtr);
    FREE_GLOBAL_PTR_MAC(mainPpDrvMod.dxChConfigDir.hawkNumberOfClientsArr);
    FREE_GLOBAL_PTR_MAC(mainPpDrvMod.dxChConfigDir.hawkPriorityArr);
    FREE_GLOBAL_PTR_MAC(mainPpDrvMod.dxChConfigDir.config88e1690.MacPhy88E1690_IntrScanArr);
    FREE_GLOBAL_PTR_MAC(mainPpDrvMod.dxChConfigDir.config88e1690.MacPhy88E1690_MaskRegDefaultSummaryArr);
    FREE_GLOBAL_PTR_MAC(mainPpDrvMod.dxChConfigDir.config88e1690.MacPhy88E1690_MaskRegMapArr);
    FREE_GLOBAL_PTR_MAC(mainPpDrvMod.dxChConfigDir.config88e1690.macPhy88e1690ObjPtr);
    FREE_GLOBAL_PTR_MAC(mainPpDrvMod.dxChConfigDir.config88e1690.saveStatsCounterSet);
    FREE_GLOBAL_PTR_MAC(mainPpDrvMod.dxChPortDir.portPaSrc.prv_dxChBcat2B0Ptr);
    FREE_GLOBAL_PTR_MAC(mainPpDrvMod.dxChPortDir.portPaSrc.prv_dxChBcat2B0_FldInitStcArr);
    FREE_GLOBAL_PTR_MAC(mainPpDrvMod.dxChPortDir.portPaSrc.prv_bc2_B0_resConfigPtr);

    {
        GT_U32  ii;

        PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.dxChCpssHwInit.tablesSrc.bobcat3TablesInfoPtr,NULL);
        PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.dxChCpssHwInit.tablesSrc.aldrin2TablesInfoPtr,NULL);
        PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.dxChCpssHwInit.tablesSrc.falconTablesInfoPtr,NULL);
        PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.dxChCpssHwInit.tablesSrc.hawkTablesInfoPtr,NULL);
        PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.dxChCpssHwInit.tablesSrc.phoenixTablesInfoPtr,NULL);
        PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.dxChCpssHwInit.tablesSrc.harrierTablesInfoPtr,NULL);
        PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.dxChCpssHwInit.tablesSrc.ironman_L_TablesInfoPtr,NULL);
        PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.dxChCpssHwInit.tablesSrc.directAccessBobcat3TableInfoPtr,NULL);
        PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.dxChCpssHwInit.tablesSrc.aldrin2directAccessTableInfoPtr,NULL);
        PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.dxChCpssHwInit.tablesSrc.falcon_directAccessTableInfoPtr,NULL);
        PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.dxChCpssHwInit.tablesSrc.hawk_directAccessTableInfoPtr,NULL);
        PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.dxChCpssHwInit.tablesSrc.phoenix_directAccessTableInfoPtr,NULL);
        PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.dxChCpssHwInit.tablesSrc.harrier_directAccessTableInfoPtr,NULL);
        PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.dxChCpssHwInit.tablesSrc.ironman_L_directAccessTableInfoPtr,NULL);

        for(ii = CPSS_PP_FAMILY_DXCH_BOBCAT3_E /*first device that support it*/ ;
            ii < CPSS_PP_FAMILY_END_DXCH_E ;
            ii++)
        {
            free_PRV_CPSS_DXCH_TABLES_INFO_PER_FAMILY_STC(&PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.dxChCpssHwInit.tablesSrc.tempTablesPerFamilyArr[ii].bobcat3Tables));
            free_PRV_CPSS_DXCH_TABLES_INFO_PER_FAMILY_STC(&PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.dxChCpssHwInit.tablesSrc.tempTablesPerFamilyArr[ii].aldrin2Tables));
            free_PRV_CPSS_DXCH_TABLES_INFO_PER_FAMILY_STC(&PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.dxChCpssHwInit.tablesSrc.tempTablesPerFamilyArr[ii].falconTables));
            free_PRV_CPSS_DXCH_TABLES_INFO_PER_FAMILY_STC(&PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.dxChCpssHwInit.tablesSrc.tempTablesPerFamilyArr[ii].hawkTables));
            free_PRV_CPSS_DXCH_TABLES_INFO_PER_FAMILY_STC(&PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.dxChCpssHwInit.tablesSrc.tempTablesPerFamilyArr[ii].phoenixTables));
            free_PRV_CPSS_DXCH_TABLES_INFO_PER_FAMILY_STC(&PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.dxChCpssHwInit.tablesSrc.tempTablesPerFamilyArr[ii].harrierTables));
            free_PRV_CPSS_DXCH_TABLES_INFO_PER_FAMILY_STC(&PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.dxChCpssHwInit.tablesSrc.tempTablesPerFamilyArr[ii].ironmanTables));
        }

        for(ii = 0; ii < (CPSS_PP_FAMILY_LAST_E+31)/32; ii ++)
        {
            PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.dxChCpssHwInit.tablesSrc.tablesDbInitPreFamily[ii]) = 0;
        }
    }
#endif
    for(intFamily = 0 ; intFamily < PRV_CPSS_DRV_FAMILY_LAST_E; intFamily++)
    {
        if(intFamily == PRV_CPSS_DRV_FAMILY_PX_PIPE_E)
        {
            /* Shared memory not supported (TBD) */
            continue;
        }

        devInterruptInfoPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[intFamily]);
        FREE_PTR_MAC(devInterruptInfoPtr->interruptsScanArray);
        FREE_PTR_MAC(devInterruptInfoPtr->maskRegistersDefaultValuesArray);
        FREE_PTR_MAC(devInterruptInfoPtr->maskRegistersAddressesArray);
    }

#ifdef CPSS_API_LOCK_PROTECTION

    for(i=0; i<PRV_CPSS_MAX_FUNCTIONALITY_RESOLUTION_CNS; i++)
    {
        if (PRV_SHARED_GLOBAL_VAR_GET(
            commonMod.genericHwInitDir.prvCpssApiLockDevicelessMtx[i]) != 0)
        {
            /* if the mutex is in use, wait until it's becoming unlocked.
               This will prevent deleting the mutex while it's in use. */
            CPSS_API_LOCK_DEVICELESS_MAC(i);

            /* destroy CPSS deviceless mutexes */
            (void)cpssOsMutexDelete(
                PRV_SHARED_GLOBAL_VAR_GET(
                    commonMod.genericHwInitDir.prvCpssApiLockDevicelessMtx[i]));
            PRV_SHARED_GLOBAL_VAR_GET(
                commonMod.genericHwInitDir.prvCpssApiLockDevicelessMtx[i]) = 0;
        }
    }

    for(i = 0;
         i < (sizeof(PRV_SHARED_GLOBAL_VAR_GET(
                    commonMod.genericHwInitDir.prvCpssApiLockByDevNumAndFunctionalityMtx))
            / sizeof(PRV_SHARED_GLOBAL_VAR_GET(
                    commonMod.genericHwInitDir.prvCpssApiLockByDevNumAndFunctionalityMtx[0])));
         i++)
    {
        for(j=0; j<PRV_CPSS_MAX_DEV_FUNCTIONALITY_RESOLUTION_CNS; j++)
        {
            if(PRV_SHARED_GLOBAL_VAR_GET(
                    commonMod.genericHwInitDir.prvCpssApiLockByDevNumAndFunctionalityMtx[i][j]) != 0)
            {
                /* if the mutex is in use, wait until it's becoming unlocked.
                   This will prevent deleting the mutex while it's in use. */
                CPSS_API_LOCK_MAC(i,j);

                /* destroy CPSS per device mutexes */
                (void)cpssOsMutexDelete(
                    PRV_SHARED_GLOBAL_VAR_GET(
                    commonMod.genericHwInitDir.prvCpssApiLockByDevNumAndFunctionalityMtx[i][j]));
                PRV_SHARED_GLOBAL_VAR_GET(
                    commonMod.genericHwInitDir.prvCpssApiLockByDevNumAndFunctionalityMtx[i][j]) = 0;
            }
        }
    }

    if (PRV_SHARED_GLOBAL_VAR_GET(
       commonMod.genericHwInitDir.prvCpssApiLockDevicelessMtxZeroLevel != 0))
    {
        /* if the mutex is in use, wait until it's becoming unlocked.
           This will prevent deleting the mutex while it's in use. */
        CPSS_ZERO_LEVEL_API_LOCK_MAC;

        /* destroy CPSS zero level mutex */
        (void)cpssOsMutexDelete(
            PRV_SHARED_GLOBAL_VAR_GET(
                commonMod.genericHwInitDir.prvCpssApiLockDevicelessMtxZeroLevel));
        PRV_SHARED_GLOBAL_VAR_GET(
           commonMod.genericHwInitDir.prvCpssApiLockDevicelessMtxZeroLevel) = 0;
    }
#endif

    FREE_GLOBAL_PTR_MAC(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsDeviceSpecInfoGlobal);

    /* should be called to allow clear of the DB */
    HWINIT_GLOVAR(sysGenGlobalInfo.prvCpssAfterDbRelease) = GT_FALSE;
    PRV_SHARED_GLOBAL_VAR_SET(sysConfigPhase1Done,GT_FALSE);


    /* restore DB to init stage */
    rc = initDb();

    return rc;
}

/**
* @internal internal_cpssPpInterruptsDisable function
* @endinternal
*
* @brief   This API disable PP interrupts
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS internal_cpssPpInterruptsDisable
(
    IN GT_U8     devNum
)
{
    GT_STATUS rc;
    GT_U32 portGroupId = 0;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    /* lock interrupt scan */
    PRV_CPSS_INT_SCAN_LOCK();
    PRV_CPSS_DRV_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
        {
            /* mask all interrupts */
            rc = prvCpssDrvExMxDxHwPpInterruptsMaskToDefault(devNum,portGroupId);
            if (rc != GT_OK)
            {
                PRV_CPSS_INT_SCAN_UNLOCK();
                return rc;
            }
        }
    PRV_CPSS_DRV_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    /* unlock interrupt scan */
    PRV_CPSS_INT_SCAN_UNLOCK();

    return GT_OK;
}

/**
* @internal cpssPpInterruptsDisable function
* @endinternal
*
* @brief   This API disable PP interrupts
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssPpInterruptsDisable
(
    IN GT_U8     devNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPpInterruptsDisable);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssPpInterruptsDisable(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}




/**
* @internal internal_cpssPpConfigDevDataImport function
* @endinternal
*
* @brief   Imports data for device specific information. Used in catch up process
*         during initialization as the first step in the catch up process for
*         configuring the standby CPU.
*         NOTE: for a device that will be initialized in "pp phase 1" with
*         High availability mode of "standby" , this function MUST be called prior
*         to the "pp phase 1"
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
* @param[in] configDevDataBufferPtr   - pointer to a pre allocated buffer for holding
*                                      information on devices in the system
* @param[in] configDevDataBufferSize  - size of the pre allocated buffer
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number or configDevDataBufferSize
*/
static GT_STATUS internal_cpssPpConfigDevDataImport
(
    IN GT_U8     devNum,
    IN void      *configDevDataBufferPtr,
    IN GT_U32    configDevDataBufferSize
)
{
    /* don't check if the device exists !!! */
    /* since it may be before "phase 1 init" */

    /* check only the range */
    if((devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS) ||
       (configDevDataBufferSize < sizeof(PRV_CPSS_PP_HA_CONFIG_INFO_STC)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    HWINIT_GLOVAR(sysGenGlobalInfo.cpssPpHaDevData[devNum]) = configDevDataBufferPtr;
    return GT_OK;
}

/**
* @internal cpssPpConfigDevDataImport function
* @endinternal
*
* @brief   Imports data for device specific information. Used in catch up process
*         during initialization as the first step in the catch up process for
*         configuring the standby CPU.
*         NOTE: for a device that will be initialized in "pp phase 1" with
*         High availability mode of "standby" , this function MUST be called prior
*         to the "pp phase 1"
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
* @param[in] configDevDataBufferPtr   - pointer to a pre allocated buffer for holding
*                                      information on devices in the system
* @param[in] configDevDataBufferSize  - size of the pre allocated buffer
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number or configDevDataBufferSize
*/
GT_STATUS cpssPpConfigDevDataImport
(
    IN GT_U8     devNum,
    IN void      *configDevDataBufferPtr,
    IN GT_U32    configDevDataBufferSize
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPpConfigDevDataImport);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, configDevDataBufferPtr, configDevDataBufferSize));

    rc = internal_cpssPpConfigDevDataImport(devNum, configDevDataBufferPtr, configDevDataBufferSize);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, configDevDataBufferPtr, configDevDataBufferSize));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPpConfigDevDataExport function
* @endinternal
*
* @brief   Imports data for device specific information. Used in catch up process
*         during initialization as the first step in the catch up process for
*         configuring the standby CPU.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
* @param[in,out] configDevDataBufferPtr   - pointer to a pre allocated buffer for
*                                      holding information on devices in the system
* @param[in,out] configDevDataBufferSizePtr - size of the pre allocated buffer
* @param[in,out] configDevDataBufferPtr   - pointer to a pre allocated buffer for
*                                      holding information on devices in the system
*                                      filled by the function.
* @param[in,out] configDevDataBufferSizePtr - size of the filled data in the pre
*                                      allocated buffer, in case of failure
*                                      (GT_BAD_SIZE), needed size of the block
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_SIZE              - block is not big enough to hold all the data
* @retval GT_BAD_PARAM             - wrong device Number
*/
static GT_STATUS internal_cpssPpConfigDevDataExport
(
    IN    GT_U8     devNum,
    INOUT void      *configDevDataBufferPtr,
    INOUT GT_U32    *configDevDataBufferSizePtr
)
{
    PRV_CPSS_PP_HA_CONFIG_INFO_STC *devData;

    if(!PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(*configDevDataBufferSizePtr < sizeof(PRV_CPSS_PP_HA_CONFIG_INFO_STC))
    {
        *configDevDataBufferSizePtr = sizeof(PRV_CPSS_PP_HA_CONFIG_INFO_STC);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE, LOG_ERROR_NO_MSG);
    }

    devData = configDevDataBufferPtr;

    /********************/
    /* export next info */
    /********************/
    devData->deviceId   = PRV_CPSS_PP_MAC(devNum)->devType;
    devData->revision   = PRV_CPSS_PP_MAC(devNum)->revision;

    *configDevDataBufferSizePtr = sizeof(PRV_CPSS_PP_HA_CONFIG_INFO_STC);

    return GT_OK;
}

/**
* @internal cpssPpConfigDevDataExport function
* @endinternal
*
* @brief   Imports data for device specific information. Used in catch up process
*         during initialization as the first step in the catch up process for
*         configuring the standby CPU.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
* @param[in,out] configDevDataBufferPtr   - pointer to a pre allocated buffer for
*                                      holding information on devices in the system
* @param[in,out] configDevDataBufferSizePtr - size of the pre allocated buffer
* @param[in,out] configDevDataBufferPtr   - pointer to a pre allocated buffer for
*                                      holding information on devices in the system
*                                      filled by the function.
* @param[in,out] configDevDataBufferSizePtr - size of the filled data in the pre
*                                      allocated buffer, in case of failure
*                                      (GT_BAD_SIZE), needed size of the block
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_SIZE              - block is not big enough to hold all the data
* @retval GT_BAD_PARAM             - wrong device Number
*/
GT_STATUS cpssPpConfigDevDataExport
(
    IN    GT_U8     devNum,
    INOUT void      *configDevDataBufferPtr,
    INOUT GT_U32    *configDevDataBufferSizePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPpConfigDevDataExport);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, configDevDataBufferPtr, configDevDataBufferSizePtr));

    rc = internal_cpssPpConfigDevDataExport(devNum, configDevDataBufferPtr, configDevDataBufferSizePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, configDevDataBufferPtr, configDevDataBufferSizePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPpConfigDevDbRelease function
* @endinternal
*
* @brief   private (internal) function to release all the DB of the device.
*         NOTE: function 'free' the allocated memory ,buffers, semaphores
*         and restore DB to 'pre-init' state
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
*/
GT_STATUS prvCpssPpConfigDevDbRelease
(
    IN    GT_U8     devNum
)
{
    GT_STATUS rc;
    GT_U32    ii;
    PRV_CPSS_GEN_PP_CONFIG_STC* devPtr;
    GT_U32  queue;/*rx/tx queue*/
    GT_U32  portGroupId;/*iterator for port group Id*/

    if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        /* device not found */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    devPtr = PRV_CPSS_PP_MAC(devNum);

#ifdef CHX_FAMILY
    /* save the state of the AUQ,FUQ,CNC HW positions */
    rc = prvCpssPpConfigDevDbPrvInfoSet(devNum,
        &(PRV_CPSS_PP_MAC(devNum)->intCtrl.auDescCtrl[0]),
        &(PRV_CPSS_PP_MAC(devNum)->intCtrl.fuDescCtrl[0]),
        &(PRV_CPSS_PP_MAC(devNum)->intCtrl.secondaryAuDescCtrl[0]),
        &(PRV_CPSS_PP_MAC(devNum)->intCtrl.rxDescList[0]),
        &(PRV_CPSS_PP_MAC(devNum)->intCtrl.au1DescCtrl[0]),
        &(PRV_CPSS_PP_MAC(devNum)->intCtrl.secondaryAu1DescCtrl[0]),
        &(PRV_CPSS_PP_MAC(devNum)->intCtrl.cnc23_fuDescCtrl[0]),
        &(PRV_CPSS_PP_MAC(devNum)->intCtrl.auqDeadLockWa[0]),
        &(PRV_CPSS_PP_MAC(devNum)->intCtrl.activeAuqIndex[0])
        );

    if(rc != GT_OK)
    {
        return rc;
    }
#endif /*CHX_FAMILY*/


    /* UN-bind the old device from the DSMA Tx completed callback to the driver */
    rc = prvCpssDrvInterruptPpTxEndedCbBind(devNum,NULL);
    if(rc != GT_OK)
    {
        return rc;
    }

#ifdef CHX_FAMILY
    /* check if we can detach from the DB of 'per family' */
    if (HWINIT_GLOVAR(prvCpssFamilyInfoArray[devPtr->devFamily])
       && (HWINIT_GLOVAR(prvCpssFamilyInfoArray[devPtr->devFamily]->numReferences)))
    {
        HWINIT_GLOVAR(prvCpssFamilyInfoArray[devPtr->devFamily]->numReferences)--;
        if(HWINIT_GLOVAR(prvCpssFamilyInfoArray[devPtr->devFamily]->numReferences) == 0)
        {
            /* no more references to this DB , so release it */
            FREE_PTR_MAC(HWINIT_GLOVAR(prvCpssFamilyInfoArray[devPtr->devFamily]));
        }
    }
#endif /*CHX_FAMILY*/

    for(queue = 0; queue < NUM_OF_SDMA_QUEUES; queue++)
    {
        FREE_PTR_MAC(devPtr->intCtrl.txDescList[queue].swTxDescBlock);
        FREE_PTR_MAC(devPtr->intCtrl.rxDescList[queue].swRxDescBlock);
        FREE_PTR_MAC(devPtr->intCtrl.txDescList[queue].revPacketIdDb);

        if(devPtr->intCtrl.txDescList[queue].poolId)
        {
            rc = cpssBmPoolDelete(devPtr->intCtrl.txDescList[queue].poolId);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    for(ii = 0 ; ii < CPSS_MAX_PORT_GROUPS_CNS ; ii++)
    {
        FREE_PTR_MAC(devPtr->intCtrl.sharedCncDescCtrl[ii].isMyCncUploadTriggeredBmpArr);
    }


    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        for (ii = 0 ; ii < CPSS_PHY_SMI_INTERFACE_MAX_E; ii++)
        {
            if(devPtr->smiMasters[portGroupId][ii] != NULL)
            {
                cpssHwDriverDestroy(devPtr->smiMasters[portGroupId][ii]);
            }
        }

        for (ii = 0 ; ii < CPSS_PHY_XSMI_INTERFACE_MAX_E; ii++)
        {
            if(devPtr->xsmiMasters[portGroupId][ii] != NULL)
            {
                cpssHwDriverDestroy(devPtr->xsmiMasters[portGroupId][ii]);
            }
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

    FREE_PTR_MAC(devPtr->intCtrl.txGeneratorPacketIdDb.cookie);
    FREE_PTR_MAC(devPtr->intCtrl.txGeneratorPacketIdDb.freeLinkedList);
    FREE_PTR_MAC(devPtr->phyPortInfoArray);
#ifdef CHX_FAMILY
    /* let the trunk remove all it's dynamic allocations */
    if(PRV_CPSS_DXCH_FAMILY_CHECK_MAC(devNum))
    {
        (void)prvCpssGenericTrunkDestroy(devNum);
    }
#endif /*CHX_FAMILY*/
    FREE_PTR_MAC(devPtr->tmInfo.pfcTmGluePfcDevInfoPtr);

    /* free Egf force status bitmap pointer */
    FREE_PTR_MAC(devPtr->portEgfForceStatusBitmapPtr);

    /* set the device info pointer as NULL --
       so we can do "phase 1" for a device with this devNum */
    devPtr = NULL;
    FREE_PTR_MAC(PRV_CPSS_PP_CONFIG_ARR_MAC[devNum]);
    PRV_CPSS_PP_CONFIG_ARR_MAC[devNum] = NULL;
    HWINIT_GLOVAR(sysGenGlobalInfo.cpssPpHaDevData[devNum]) = NULL;

    FREE_PTR_MAC(PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]));
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum])= NULL;


#ifdef CHX_FAMILY
    /* check if there are still existing families (quicker then loop on devices) */
    for(ii = 0 ; ii < CPSS_PP_FAMILY_LAST_E ; ii++)
    {
        if(HWINIT_GLOVAR(prvCpssFamilyInfoArray[ii]))
        {
            break;
        }
    }
#else
    ii = CPSS_PP_FAMILY_LAST_E;
#endif /*CHX_FAMILY*/

    if(ii == CPSS_PP_FAMILY_LAST_E)
    {
#ifdef CHX_FAMILY
        /* release VCT DB */
        rc = prvCpssVctDbRelease();
        if(rc != GT_OK)
        {
            return rc;
        }
#endif /*CHX_FAMILY*/

        /* no more existing devices */
        HWINIT_GLOVAR(sysGenGlobalInfo.prvCpssAfterDbRelease) = GT_TRUE;

#ifdef CHX_FAMILY
        for (ii = 0; ii < PRV_CPSS_GEN_NETIF_MAX_RX_QUEUES_NUM_CNS; ii++)
        {
            if (HWINIT_GLOVAR(sysGenGlobalInfo.prvCpssGenNetIfMiiRxQCtrl.intQueId[ii]))
            {
                (void)cpssOsMsgQDelete(HWINIT_GLOVAR(sysGenGlobalInfo.prvCpssGenNetIfMiiRxQCtrl.intQueId[ii]));
                HWINIT_GLOVAR(sysGenGlobalInfo.prvCpssGenNetIfMiiRxQCtrl.intQueId[ii]) = (CPSS_OS_MSGQ_ID)0;
            }
        }
#endif /*CHX_FAMILY*/
    }


    /*  let the cpssDriver do the same "remove" of the device. */
    rc = prvCpssDrvHwPpDevRemove(devNum);

    return rc;
}

/**
* @internal prvCpssPpConfigDevDbBusBaseAddrAttach function
* @endinternal
*
* @brief   private (internal) function to attach the base address to devNum
*         this needed for the 'initialization after the DB release processing'
*         when the initialization for a device during 'phase 1' will be we will
*         know what 'old devNum' it used and so accessing the special DB for this
*         device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
* @param[in] busBaseAddr              -  the address of the device on the interface bus (pci/pex/smi/twsi)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
*/
GT_STATUS prvCpssPpConfigDevDbBusBaseAddrAttach
(
    IN GT_U8        devNum,
    IN GT_UINTPTR   busBaseAddr
)
{
    GT_U32      ii;

    if(devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* look for this base address if already in the DB , because if already in DB
    we need to move info from that device to the 'new device' */
    for(ii = 0 ; ii < PRV_CPSS_MAX_PP_DEVICES_CNS; ii++)
    {
        if(HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[ii].busBaseAddr) == busBaseAddr)
        {
            break;
        }
    }

    if(ii == PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        /* this is a new base address */
        /* just set it into DB , for the device */
        HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[devNum].busBaseAddr) = busBaseAddr;
        HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[devNum].deviceDidHwReset) = GT_TRUE;
        HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[devNum].deviceNeedNetIfInit) = GT_TRUE;
        HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfoValid) = GT_FALSE;
    }
    else if(ii != devNum)/* found the bus base address on 'old' device number */
    {
        /* move the info from the 'old device' to the new device */
        cpssOsMemCpy(&(HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[devNum])),
                     &(HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[ii])),
                     sizeof(HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[0])));

        /*restore old device to init values */
        HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[ii].busBaseAddr) = BASE_ADDR_NOT_USED_CNS;
        HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[ii].deviceDidHwReset) = GT_TRUE;
        HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[ii].deviceNeedNetIfInit) = GT_TRUE;
        HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[ii].dmaInfoValid) = GT_FALSE;
    }

    return GT_OK;
}

/**
* @internal prvCpssPpConfigDevDbPrvInfoSet function
* @endinternal
*
* @brief   private (internal) function to :
*         set the AUQ,FUQ private cpss info.
*         this needed for the 'Initialization after the DB release processing'
*         when the initialization for a device during 'Phase 1' will be we will
*         know what 'Old devNum' it used and so accessing the special DB for this
*         device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
* @param[in] auqInternalInfoPtr       - (pointer to)AU queue private info , of all port groups
* @param[in] fuqInternalInfoPtr       - (pointer to)FU queue private info , of all port groups
* @param[in] secondaryAuqInternalInfoPtr - (pointer to)secondary AU queue private info , of all port groups
* @param[in] rxDescListPtr            - (pointer to)Rx descriptors list private info
* @param[in] auq1InternalInfoPtr      - (pointer to)AU1 queue private info , of all port groups
* @param[in] secondaryAuq1InternalInfoPtr - (pointer to)secondary AU1 queue private info , of all port groups
* @param[in] cnc23FuqInternalInfoPtr  - (pointer to)FU queue for CNC23 private info , of all port groups (can be NULL when queue not used)
* @param[in] auqDeadLockWaPtr         - (pointer to)AUQ deadlock private info (can be NULL when queue not used)
* @param[in] activeAuqIndexPtr        - (pointer to)AUQ 'active AUQ' private info (can be NULL when queue not used)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_NOT_FOUND             - device not found in the DB
*
* @note NOTE: not checking for NULL pointers to speed up performances
*
*/
GT_STATUS prvCpssPpConfigDevDbPrvInfoSet
(
    IN GT_U8                        devNum,
    IN PRV_CPSS_AU_DESC_CTRL_STC    *auqInternalInfoPtr,
    IN PRV_CPSS_AU_DESC_CTRL_STC    *fuqInternalInfoPtr,
    IN PRV_CPSS_AU_DESC_CTRL_STC    *secondaryAuqInternalInfoPtr,
    IN PRV_CPSS_RX_DESC_LIST_STC    *rxDescListPtr,
    IN PRV_CPSS_AU_DESC_CTRL_STC    *auq1InternalInfoPtr,
    IN PRV_CPSS_AU_DESC_CTRL_STC    *secondaryAuq1InternalInfoPtr,
    IN PRV_CPSS_AU_DESC_CTRL_STC    *cnc23FuqInternalInfoPtr,
    IN PRV_CPSS_AU_DESC_CTRL_FOR_DEAD_LOCK_STC *auqDeadLockWaPtr,
    IN GT_U32  *activeAuqIndexPtr
)
{
    /* Note:
       Due to compiler bug the variable i should be defined as volatile and a
       temporary variable, dmaInfo, is needed instead of making assignments
       directly to sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfo.XXX[i]
       */

    volatile GT_U32 i;                        /* volatile due to compiler bug */
    GT_U32   numOfRxSdmaQueues;               /* number of RX SDMA queues in device */
    PRV_CPSS_DMA_RE_INIT_INFO_STC   *dmaInfo;

    if(! PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    else if(HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[devNum].busBaseAddr) == BASE_ADDR_NOT_USED_CNS)
    {
        /* Soft Reset was done. The PP DMAs states were changed. No need to store it. */
        return GT_OK;
    }
    else if (HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfoValid) == GT_FALSE)
    {
        return GT_OK;
    }

    dmaInfo = &(HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfo));
    if(dmaInfo->fuqUseSeparate == GT_TRUE)
    {
        for( i = 0 ; i < CPSS_MAX_PORT_GROUPS_CNS ; i++ )
        {
            cpssOsMemCpy(&(dmaInfo->fuqInternalInfo[i]), &fuqInternalInfoPtr[i], sizeof(PRV_CPSS_AU_DESC_CTRL_STC));
        }

        if(cnc23FuqInternalInfoPtr)
        {
            for( i = 0 ; i < CPSS_MAX_PORT_GROUPS_CNS ; i++ )
            {
                cpssOsMemCpy(&(dmaInfo->cnc23FuqInternalInfo[i]), &cnc23FuqInternalInfoPtr[i], sizeof(PRV_CPSS_AU_DESC_CTRL_STC));
            }
        }

    }

    for( i = 0 ; i < CPSS_MAX_PORT_GROUPS_CNS ; i++ )
    {
        cpssOsMemCpy(&(dmaInfo->auqInternalInfo[i]), &auqInternalInfoPtr[i], sizeof(PRV_CPSS_AU_DESC_CTRL_STC));
        cpssOsMemCpy(&(dmaInfo->secondaryAuqInternalInfo[i]), &secondaryAuqInternalInfoPtr[i], sizeof(PRV_CPSS_AU_DESC_CTRL_STC));
        cpssOsMemCpy(&(dmaInfo->auq1InternalInfo[i]), &auq1InternalInfoPtr[i], sizeof(PRV_CPSS_AU_DESC_CTRL_STC));
        cpssOsMemCpy(&(dmaInfo->secondaryAuq1InternalInfo[i]), &secondaryAuq1InternalInfoPtr[i], sizeof(PRV_CPSS_AU_DESC_CTRL_STC));
        cpssOsMemCpy(&(dmaInfo->auqDeadLockWa[i]), &auqDeadLockWaPtr[i], sizeof(PRV_CPSS_AU_DESC_CTRL_FOR_DEAD_LOCK_STC));
        cpssOsMemCpy(&(dmaInfo->activeAuqIndex[i]), &activeAuqIndexPtr[i], sizeof(GT_U32));
    }

     /* BC3 and above have 32 RX SDMA queues. All previous devices have only 8 queues. */
    numOfRxSdmaQueues = (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) ? NUM_OF_SDMA_QUEUES : NUM_OF_RX_QUEUES;

    for( i = 0 ; i < numOfRxSdmaQueues ; i++ )
    {
        /* "swRxDescBlock" is start of array of Rx descriptors, while "next2Return" and "next2Receive"   */
        /* are pointers to descriptors in this array. After the SW restart operation the array is not    */
        /* valid any more and we have to reallocate it. The new next2Return and next2Receive pointers    */
        /* will have to point to descriptors in the new allocated array with the same offsets from       */
        /* array start as were in the original array, therefore offsets are calculated here since the    */
        /* pointers value themselves will have no meaning after the array pointed now by "swRxDescBlock" */
        /* is released.                                                                                  */
        dmaInfo->rxDescInfo[i].next2ReturnOffset =
            (GT_U32)(rxDescListPtr[i].next2Return - (PRV_CPSS_SW_RX_DESC_STC *)rxDescListPtr[i].swRxDescBlock);
        dmaInfo->rxDescInfo[i].next2ReceiveOffset =
            (GT_U32)(rxDescListPtr[i].next2Receive - (PRV_CPSS_SW_RX_DESC_STC *)rxDescListPtr[i].swRxDescBlock);
        dmaInfo->rxDescInfo[i].freeDescNum = rxDescListPtr[i].freeDescNum;
        dmaInfo->rxDescInfo[i].headerOffset = rxDescListPtr[i].headerOffset;
        dmaInfo->rxDescInfo[i].forbidQEn = rxDescListPtr[i].forbidQEn;
    }

    return GT_OK;
}

/**
* @internal prvCpssPpConfigDevDbPrvInfoGet function
* @endinternal
*
* @brief   private (internal) function to :
*         get the AUQ,FUQ private cpss info.
*         this needed for the 'Initialization after the DB release processing'
*         when the initialization for a device during 'Phase 1' will be we will
*         know what 'Old devNum' it used and so accessing the special DB for this
*         device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
* @param[in] portGroupId              - The port group Id , to support multi-port-groups device
*
* @param[out] auqInternalInfoPtr       - (pointer to)AU queue private info
* @param[out] fuqInternalInfoPtr       - (pointer to)FU queue private info (can be NULL when FUQ not used)
* @param[out] secondaryAuqInternalInfoPtr - (pointer to)secondary AU queue private info (can be NULL when queue not used)
* @param[out] auq1InternalInfoPtr      - (pointer to)AU1 queue private info
* @param[out] secondaryAuq1InternalInfoPtr - (pointer to)secondary AU1 queue private info (can be NULL when queue not used)
* @param[out] cnc23FuqInternalInfoPtr  - (pointer to)FU queue for CNC2,3  private info (can be NULL when FUQ not used)
* @param[out] auqDeadLockWaPtr         - (pointer to)AUQ deadlock private info (can be NULL when queue not used)
* @param[out] activeAuqIndexPtr        - (pointer to)AUQ 'active AUQ' private info (can be NULL when queue not used)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_NOT_FOUND             - device not found in the DB
* @retval GT_BAD_PTR               - one of the pointers is NULL
*/
GT_STATUS prvCpssPpConfigDevDbPrvInfoGet
(
    IN GT_U8                        devNum,
    IN GT_U32                       portGroupId,
    OUT PRV_CPSS_AU_DESC_CTRL_STC    *auqInternalInfoPtr,
    OUT PRV_CPSS_AU_DESC_CTRL_STC    *fuqInternalInfoPtr,
    OUT PRV_CPSS_AU_DESC_CTRL_STC    *secondaryAuqInternalInfoPtr,
    OUT PRV_CPSS_AU_DESC_CTRL_STC    *auq1InternalInfoPtr,
    OUT PRV_CPSS_AU_DESC_CTRL_STC    *secondaryAuq1InternalInfoPtr,
    OUT PRV_CPSS_AU_DESC_CTRL_STC    *cnc23FuqInternalInfoPtr,
    OUT PRV_CPSS_AU_DESC_CTRL_FOR_DEAD_LOCK_STC *auqDeadLockWaPtr,
    OUT GT_U32  *activeAuqIndexPtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(auqInternalInfoPtr);

    if(! PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    else if(HWINIT_GLOVAR(
        sysGenGlobalInfo.reInitializationDbPtr[devNum].busBaseAddr) == BASE_ADDR_NOT_USED_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    else if (HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfoValid) == GT_FALSE)
    {
        /* no meaning in non- DMA device */
        return GT_OK;
    }
    else if(portGroupId >= CPSS_MAX_PORT_GROUPS_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfo.fuqUseSeparate) == GT_TRUE)
    {
        CPSS_NULL_PTR_CHECK_MAC(fuqInternalInfoPtr);
        *fuqInternalInfoPtr = HWINIT_GLOVAR(
            sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfo.fuqInternalInfo[portGroupId]);

        if(cnc23FuqInternalInfoPtr)
        {
            *cnc23FuqInternalInfoPtr = HWINIT_GLOVAR(
                sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfo.cnc23FuqInternalInfo[portGroupId]);
        }
    }

    *auqInternalInfoPtr = HWINIT_GLOVAR(
        sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfo.auqInternalInfo[portGroupId]);

    if(secondaryAuqInternalInfoPtr)
    {
        *secondaryAuqInternalInfoPtr = HWINIT_GLOVAR(
            sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfo.secondaryAuqInternalInfo[portGroupId]);
    }

    if(auq1InternalInfoPtr)
    {
        *auq1InternalInfoPtr = HWINIT_GLOVAR(
            sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfo.auq1InternalInfo[portGroupId]);
    }

    if(secondaryAuq1InternalInfoPtr)
    {
        *secondaryAuq1InternalInfoPtr = HWINIT_GLOVAR(
            sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfo.secondaryAuq1InternalInfo[portGroupId]);
    }

    if(auqDeadLockWaPtr)
    {
        *auqDeadLockWaPtr = HWINIT_GLOVAR(
            sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfo.auqDeadLockWa[portGroupId]);
    }

    if(activeAuqIndexPtr)
    {
        *activeAuqIndexPtr = HWINIT_GLOVAR(
            sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfo.activeAuqIndex[portGroupId]);
    }

    return GT_OK;
}

/**
* @internal prvCpssPpConfigDevDbPrvNetInfInfoSet function
* @endinternal
*
* @brief   private (internal) function to :
*         set the Rx descriptors list private cpss info.
*         this needed for the 'Initialization after the DB release processing'
*         when the initialization for a device during 'Phase 1' will be we will
*         know what 'Old devNum' it used and so accessing the special DB for this
*         device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
* @param[in] rxDescPtr                - (pointer to) the start of Rx descriptors block.
* @param[in] rxDescListPtr            - (pointer to)Rx descriptors list private info.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_NOT_FOUND             - device not found in the DB
* @retval GT_BAD_PTR               - one of the pointers is NULL
*/
GT_STATUS prvCpssPpConfigDevDbPrvNetInfInfoSet
(
    IN GT_U8                        devNum,
    IN PRV_CPSS_RX_DESC_STC         *rxDescPtr,
    IN PRV_CPSS_RX_DESC_LIST_STC    *rxDescListPtr
)
{
    GT_U32 i;

    if(! PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    else if(HWINIT_GLOVAR(
        sysGenGlobalInfo.reInitializationDbPtr[devNum].busBaseAddr) == BASE_ADDR_NOT_USED_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    else if (HWINIT_GLOVAR(
        sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfoValid) == GT_FALSE)
    {
        /* no meaning in non- DMA device */
        return GT_OK;
    }

    for( i = 0 ; i < NUM_OF_RX_QUEUES ; i++ )
    {
        /* The Rx descriptors reside in memory as an array, the start of which is at "rxDescPtr". */
        /* For each queue we save for later use after SW restart a pointer to where the block of  */
        /* descriptors belonging to that queue start. Since they all reside sequentially in the   */
        /* same array, we simply have to increment the start pointer between loop cycle in the    */
        /* amount of descriptors of each queue.                                                   */
        HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfo.rxDescInfo[i].rxDescBlock) =
            rxDescPtr;
        rxDescPtr += rxDescListPtr[i].freeDescNum;
        /* At this stage the value of "freeDescNum" is equal to the total number of existing */
        /* descriptors, a value that is used later after SW restart to allocate the required */
        /* memory for the SW Rx descriptors.                                                 */
        HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfo.rxDescInfo[i].totalDescNum) =
            rxDescListPtr[i].freeDescNum;
    }

    return GT_OK;
}

/**
* @internal prvCpssPpConfigDevDbPrvNetInfInfoGet function
* @endinternal
*
* @brief   private (internal) function to :
*         get the Rx descriptors list private cpss info.
*         this needed for the 'Initialization after the DB release processing'
*         when the initialization for a device during 'Phase 1' will be we will
*         know what 'Old devNum' it used and so accessing the special DB for this
*         device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
* @param[in] firstRxQueue             - first RX SDMA queue to get info
* @param[in] numOfRxQueues            - number of RX SDMA queues to get info
*
* @param[out] rxDescListPtr            - (pointer to) Rx descriptors list private info
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_NOT_FOUND             - device not found in the DB
*/
GT_STATUS prvCpssPpConfigDevDbPrvNetInfInfoGet
(
    IN GT_U8                        devNum,
    IN GT_U32                       firstRxQueue,
    IN GT_U32                       numOfRxQueues,
    OUT PRV_CPSS_RX_DESC_LIST_STC   *rxDescListPtr
)
{
    PRV_CPSS_SW_RX_DESC_STC *firstSwRxDesc; /* Points to the first Sw Rx desc  */
    PRV_CPSS_SW_RX_DESC_STC *swRxDesc;      /* Points to the Sw Rx desc to init. */
    GT_U32 sizeOfDesc;          /* The amount of memory (in bytes) that*/
                                /* a single desc. will occupy, including*/
                                /* the alignment.                       */
    GT_U32 i, rxQueue;                       /* Loop counters */

    if(! PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    else if(HWINIT_GLOVAR(
        sysGenGlobalInfo.reInitializationDbPtr[devNum].busBaseAddr) == BASE_ADDR_NOT_USED_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    else if (HWINIT_GLOVAR(
        sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfoValid) == GT_FALSE)
    {
        /* no meaning in non- DMA device */
        return GT_OK;
    }

    if((firstRxQueue + numOfRxQueues) > NUM_OF_SDMA_QUEUES)
    {
        /* add check for klockwork */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    for( rxQueue = firstRxQueue ; rxQueue < (firstRxQueue + numOfRxQueues) ; rxQueue++ )
    {
        firstSwRxDesc = (PRV_CPSS_SW_RX_DESC_STC*)cpssOsMalloc(sizeof(PRV_CPSS_SW_RX_DESC_STC)*
            HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfo.rxDescInfo[rxQueue].totalDescNum));
        if(firstSwRxDesc == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        /* save pointer to allocated Mem block on per queue structure */
        rxDescListPtr[rxQueue].swRxDescBlock = firstSwRxDesc;

        rxDescListPtr[rxQueue].next2Return =
            firstSwRxDesc + HWINIT_GLOVAR(
                sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfo.rxDescInfo[rxQueue].next2ReturnOffset);
        rxDescListPtr[rxQueue].next2Receive =
            firstSwRxDesc + HWINIT_GLOVAR(
                sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfo.rxDescInfo[rxQueue].next2ReceiveOffset);
        rxDescListPtr[rxQueue].freeDescNum = HWINIT_GLOVAR(
            sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfo.rxDescInfo[rxQueue].freeDescNum);
        rxDescListPtr[rxQueue].maxDescNum = HWINIT_GLOVAR(
            sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfo.rxDescInfo[rxQueue].freeDescNum);
        rxDescListPtr[rxQueue].headerOffset = HWINIT_GLOVAR(
            sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfo.rxDescInfo[rxQueue].headerOffset);
        rxDescListPtr[rxQueue].forbidQEn = HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfo.rxDescInfo[rxQueue].forbidQEn);

        sizeOfDesc = sizeof(PRV_CPSS_RX_DESC_STC);
        if((sizeOfDesc % RX_DESC_ALIGN) != 0)
        {
            sizeOfDesc += (RX_DESC_ALIGN -(sizeof(PRV_CPSS_RX_DESC_STC) % RX_DESC_ALIGN));
        }

        swRxDesc = NULL;
        for(i = 0; (i < HWINIT_GLOVAR(
            sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfo.rxDescInfo[rxQueue].totalDescNum)); i++)
        {
            swRxDesc = firstSwRxDesc + i;

            swRxDesc->rxDesc = (PRV_CPSS_RX_DESC_STC*)
                ((GT_UINTPTR)HWINIT_GLOVAR(
                    sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfo.rxDescInfo[rxQueue].rxDescBlock) + i*sizeOfDesc) ;

            swRxDesc->swNextDesc  = (firstSwRxDesc + i + 1);
        }

        if( swRxDesc != NULL )
        {
            /* Close the cyclic desc. list. */
            swRxDesc->swNextDesc = firstSwRxDesc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssPpConfigDevDbHwResetSet function
* @endinternal
*
* @brief   private (internal) function to state that the device did HW reset or that
*         the device finished the 'pp logic init' function
*         this needed for the 'initialization after the DB release processing'
*         when the initialization for a device during 'phase 1' will be we will
*         know what 'old devNum' it used and so accessing the special DB for this
*         device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] didHwReset               - GT_TRUE - Did HW reset ,
*                                      GT_FALSE -  finished pp logical init.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_NOT_FOUND             - device not found in the DB
*/
GT_STATUS prvCpssPpConfigDevDbHwResetSet
(
    IN GT_U8     devNum,
    IN GT_BOOL   didHwReset
)
{
    if(! PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    else if(HWINIT_GLOVAR(
        sysGenGlobalInfo.reInitializationDbPtr[devNum].busBaseAddr) == BASE_ADDR_NOT_USED_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    if(didHwReset == GT_TRUE)
    {
        HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[devNum].busBaseAddr) = BASE_ADDR_NOT_USED_CNS;
        HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[devNum].deviceDidHwReset) = GT_TRUE;
        HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[devNum].deviceNeedNetIfInit) = GT_TRUE;
        HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfoValid) = GT_FALSE;

    }
    else
    {
        HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[devNum].deviceDidHwReset) = GT_FALSE;
    }

    return GT_OK;
}

/**
* @internal prvCpssPpConfigDevDbHwResetGet function
* @endinternal
*
* @brief   private (internal) function get the state that the device did HW reset
*         or that the device finished the 'pp logic init' function.
*         this needed for the 'initialization after the DB release processing'
*         when the initialization for a device during 'phase 1' will be we will
*         know what 'old devNum' it used and so accessing the special DB for this
*         device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
*
* @param[out] didHwResetPtr            - GT_TRUE - Did HW reset ,
*                                      GT_FALSE -  finished pp logical init.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_NOT_FOUND             - device not found in the DB
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssPpConfigDevDbHwResetGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *didHwResetPtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(didHwResetPtr);

    if(! PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    else if(HWINIT_GLOVAR(
        sysGenGlobalInfo.reInitializationDbPtr[devNum].busBaseAddr) == BASE_ADDR_NOT_USED_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    *didHwResetPtr = HWINIT_GLOVAR(
        sysGenGlobalInfo.reInitializationDbPtr[devNum].deviceDidHwReset);

    return GT_OK;
}

/**
* @internal prvCpssPpConfigDevDbNetIfInitSet function
* @endinternal
*
* @brief   Private (internal) function to set if network initialization is needed or
*         if configuration will be based on previously saved re-initialization parameters.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
* @param[in] needNetIfInit            - GT_TRUE  - Need network interface init
*                                      GT_FALSE - Network interface init using previously saved values.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_NOT_FOUND             - device not found in the DB
*/
GT_STATUS prvCpssPpConfigDevDbNetIfInitSet
(
    IN GT_U8     devNum,
    IN GT_BOOL   needNetIfInit
)
{
    if(! PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    else if(HWINIT_GLOVAR(
        sysGenGlobalInfo.reInitializationDbPtr[devNum].busBaseAddr) == BASE_ADDR_NOT_USED_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[devNum].deviceNeedNetIfInit) = needNetIfInit;

    return GT_OK;
}

/**
* @internal prvCpssPpConfigDevDbNetIfInitGet function
* @endinternal
*
* @brief   Private (internal) function to get whether network initialization is needed or
*         whether configuration will be based on previously saved re-initialization parameters.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number
*
* @param[out] needNetIfInitPtr         - (pointer to):
*                                      GT_TRUE  - Need network interface init
*                                      GT_FALSE - Network interface init using previously saved values.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_NOT_FOUND             - device not found in the DB
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssPpConfigDevDbNetIfInitGet
(
    IN  GT_U8     devNum,
    OUT GT_BOOL   *needNetIfInitPtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(needNetIfInitPtr);

    if(! PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    else if(HWINIT_GLOVAR(
        sysGenGlobalInfo.reInitializationDbPtr[devNum].busBaseAddr) == BASE_ADDR_NOT_USED_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    *needNetIfInitPtr = HWINIT_GLOVAR(
        sysGenGlobalInfo.reInitializationDbPtr[devNum].deviceNeedNetIfInit);

    return GT_OK;
}

/**
* @internal prvCpssPpConfigDevDbRenumber function
* @endinternal
*
* @brief   the function set parameters for the driver to renumber it's DB.
*         the function "renumber" the current device number of the device to a
*         new device number.
*         NOTE:
*         this function MUST be called under 'Interrupts are locked'
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] oldDevNum                - The PP's "old" device number .
* @param[in] newDevNum                - The PP's "new" device number swap the DB to.
*                                       GT_OK on success, or
*
* @retval GT_FAIL                  - on error
* @retval GT_ALREADY_EXIST         - the new device number is already used
*/
GT_STATUS prvCpssPpConfigDevDbRenumber
(
    IN GT_U8    oldDevNum,
    IN GT_U8    newDevNum
)
{
    /* swap the cpss DB */
    if (HWINIT_GLOVAR(
        sysGenGlobalInfo.reInitializationDbPtr[newDevNum].busBaseAddr) != BASE_ADDR_NOT_USED_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
    }

    cpssOsMemCpy(&(HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[newDevNum])),
                 &(HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[oldDevNum])),
                 sizeof(HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[0])));

    /* set old device with init values */
    HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[oldDevNum].busBaseAddr) = BASE_ADDR_NOT_USED_CNS;
    HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[oldDevNum].deviceDidHwReset) = GT_TRUE;
    HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[oldDevNum].deviceNeedNetIfInit) = GT_TRUE;
    HWINIT_GLOVAR(sysGenGlobalInfo.reInitializationDbPtr[oldDevNum].dmaInfoValid) = GT_FALSE;

    return GT_OK;
}

/**
* @internal prvCpssPrePhase1PpInit function
* @endinternal
*
* @brief   private (internal) function to make phase1 pre-init
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] prePhase1InitMode        - pre phase 1 init mode.
*
* @retval GT_OK                    - on success.
*/
GT_STATUS prvCpssPrePhase1PpInit
(
    IN    PRV_CPSS_PP_PRE_PHASE1_INIT_MODE_ENT     prePhase1InitMode
)
{
    PRV_SHARED_GLOBAL_VAR_SET(
        commonMod.genericHwInitDir.prvCpssPrePhase1PpInitMode, prePhase1InitMode);

    return GT_OK;
}

/**
* @internal prvCpssPrePhase1PpInitModeGet function
* @endinternal
*
* @brief   private (internal) function retrieving pre phase1 init mode
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[out] prePhase1InitModePtr     - pointer to pre phase 1 init mode .
*
* @retval GT_OK                    - on success.
*/
GT_STATUS prvCpssPrePhase1PpInitModeGet
(
    OUT PRV_CPSS_PP_PRE_PHASE1_INIT_MODE_ENT     *prePhase1InitModePtr
)
{
    *prePhase1InitModePtr =
        PRV_SHARED_GLOBAL_VAR_GET(commonMod.genericHwInitDir.prvCpssPrePhase1PpInitMode);
    return GT_OK;
}


/**
* @internal prvCpssPpConfigBitmapFirstActiveBitGet function
* @endinternal
*
* @brief   Get the first active bit from the given bitmap.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] bitmap                   - source bitmap.
*
* @param[out] firstBitPtr              - pointer to the index of the found bit
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or portGroupsBmp
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssPpConfigBitmapFirstActiveBitGet
(
    IN  GT_U32     bitmap,
    OUT GT_U32     *firstBitPtr
)
{
    GT_U32 bmp;       /* bitmap             */
    GT_U32 bmp4;      /* bitmap 4 LSBs      */
    GT_U32 i;         /* loop index         */

    CPSS_NULL_PTR_CHECK_MAC(firstBitPtr);

    /* the "first non-zero bit table" for values 0-15       */
    /* the value for 0 is not relevant                      */
    /* 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 */
    /* 0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1,  0,  2,  0,  1,  0  */
    /* packed in one 32-bit value                           */
    /* bits n,n+1 contain the "first non-zero bit" for n    */
    #define PRV_FIRST_NON_ZERO_BIT_CNS 0x12131210

    bmp = bitmap;

    /* search the first non-zero bit in bitmap */
    /* loop on 8 4-bit in 32-bit bitmap        */
    for (i = 0; (i < 8); i++, bmp >>= 4)
    {
        bmp4 = bmp & 0x0F;

        if (bmp4 == 0)
        {
            /* non-zero bit not found */
            /* search in next 4 bits  */
            continue;
        }

        /* non-zero bit found                                            */
        /* the expression below is the fast version of                   */
        /* (i * 4) + ((PRV_FIRST_NON_ZERO_BIT_CNS >> (bmp4 * 2)) & 0x03) */
        *firstBitPtr =
            (i << 2) + ((PRV_FIRST_NON_ZERO_BIT_CNS >> (bmp4 << 1)) & 0x03);
        return GT_OK;
    }

    /* occurs only if bmp == 0 */
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    /* this definition is local for this function */
    #undef PRV_FIRST_NON_ZERO_BIT_CNS
}

/**
* @internal prvCpssPpConfigPortGroupFirstActiveGet function
* @endinternal
*
* @brief   Get the first active port group from the given bitmap.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number .
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* @param[out] portGroupIdPtr           - pointer to port Group Id.
*                                      for non multi-port groups device
*                                      gets the CPSS_PORT_GROUP_UNAWARE_MODE_CNS value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or portGroupsBmp
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssPpConfigPortGroupFirstActiveGet
(
    IN  GT_U8                  devNum,
    IN  GT_PORT_GROUPS_BMP     portGroupsBmp,
    OUT GT_U32                 *portGroupIdPtr
)
{
    GT_U32 bmp;       /* port group bitmap             */

    if (0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (! PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
    {
        CPSS_NULL_PTR_CHECK_MAC(portGroupIdPtr);
        *portGroupIdPtr = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        return GT_OK;
    }

    /* merge the device port group bitmap with the given bitmap */
    bmp = PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp;

    if (portGroupsBmp != CPSS_PORT_GROUP_UNAWARE_MODE_CNS)
    {
        bmp &= portGroupsBmp;
    }

    /* portGroupIdPtr NULL check done in called function */

    return prvCpssPpConfigBitmapFirstActiveBitGet(
        bmp, portGroupIdPtr);
}

/**
* @internal prvCpssPpConfigDevInfoGet function
* @endinternal
*
* @brief   Private (internal) function returns device static information
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @param[out] devInfoPtr               - (pointer to) device information
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
*/
GT_STATUS prvCpssPpConfigDevInfoGet
(
    IN  GT_U8                        devNum,
    OUT CPSS_GEN_CFG_DEV_INFO_STC   *devInfoPtr
)
{
    PRV_CPSS_GEN_PP_CONFIG_STC *ppConfigPtr; /* pointer to PP configuration DB */
    GT_U32 i;
    /* check devNum */
    if(! PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    ppConfigPtr = PRV_CPSS_PP_MAC(devNum);

    devInfoPtr->devType  = ppConfigPtr->devType;
    devInfoPtr->revision = ppConfigPtr->revision;
    devInfoPtr->devFamily = ppConfigPtr->devFamily;
    devInfoPtr->devSubFamily = ppConfigPtr->devSubFamily;
    devInfoPtr->maxPortNum = ppConfigPtr->numOfPorts - 1;
    devInfoPtr->numOfVirtPorts = ppConfigPtr->numOfVirtPorts;
    devInfoPtr->existingPorts = ppConfigPtr->existingPorts;
    /* set default mode */
    devInfoPtr->hwDevNumMode = CPSS_GEN_CFG_HW_DEV_NUM_MODE_SINGLE_E;
    devInfoPtr->cpuPortMode = ppConfigPtr->cpuPortMode;
    for (i = 0; i < sizeof(ppConfigPtr->supportedSipArr) / sizeof(ppConfigPtr->supportedSipArr[0]); i++)
    {
        devInfoPtr->supportedSipBmp[i] = ppConfigPtr->supportedSipArr[i];
    }

    return GT_OK;
}

/**
* @internal prvCpssPpConfigBitmapNumBitsGet function
* @endinternal
*
* @brief   Get the number of bits that set set to 1 in the given bitmap.
*
* @param[in] bitmap                   - source bitmap.
*                                       numbert of bits that are set to 1 in bitmap
*
*  @retval numOfBitsInBmp , numOfBitsInBitmap , numBitsInBmp , numBitsInBitmap
*/
GT_U32 prvCpssPpConfigBitmapNumBitsGet
(
    IN  GT_U32     bitmap
)
{
    GT_U32 i;         /* loop index         */
    GT_U32  numOfBits = 0;
    /* the "num of bits" for values 0-15       */
    /* the value for 0 is not relevant                      */
    static const GT_U8 numBits[16] = {
    /* binary           num of bits */
    /*  0000          */  0 ,
    /*  0001          */  1 ,
    /*  0010          */  1 ,
    /*  0011          */  2 ,
    /*  0100          */  1 ,
    /*  0101          */  2 ,
    /*  0110          */  2 ,
    /*  0111          */  3 ,
    /*  1000          */  1 ,
    /*  1001          */  2 ,
    /*  1010          */  2 ,
    /*  1011          */  3 ,
    /*  1100          */  2 ,
    /*  1101          */  3 ,
    /*  1110          */  3 ,
    /*  1111          */  4 };

    /* loop on (up to) 8 4-bit in 32-bit bitmap        */
    for (i = 0; (i < 8) && bitmap; i++, bitmap >>= 4)
    {
        numOfBits += numBits[bitmap & 0x0F];
    }

    return numOfBits;
}

/**
* @internal prvCpssPpConfigNumActivePortGroupsInBmpGet function
* @endinternal
*
* @brief   Get the number of active port groups in the given bitmap.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number .
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* @param[out] numActivePortGroupsPtr   - (pointer to)number of active port groups in the BMP.
*                                      for non multi-port groups device : value 1 is returned.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or portGroupsBmp
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssPpConfigNumActivePortGroupsInBmpGet
(
    IN  GT_U8                  devNum,
    IN  GT_PORT_GROUPS_BMP     portGroupsBmp,
    OUT GT_U32                 *numActivePortGroupsPtr
)
{
    GT_U32 bmp;       /* port group bitmap             */

    CPSS_NULL_PTR_CHECK_MAC(numActivePortGroupsPtr);

    if (0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (! PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
    {
        *numActivePortGroupsPtr = 1;
        return GT_OK;
    }

    /* merge the device port group bitmap with the given bitmap */
    bmp = PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp;
    if (portGroupsBmp != CPSS_PORT_GROUP_UNAWARE_MODE_CNS)
    {
        bmp &= portGroupsBmp;
    }

    /* portGroupIdPtr NULL check done in called function */
    *numActivePortGroupsPtr = prvCpssPpConfigBitmapNumBitsGet(bmp);

    return GT_OK;
}

GT_STATUS prvCpssDxChSharedCncDescCtrlDbManagerDump
(
    IN  GT_U8                       devNum
);


/**
* @internal prvCpssPpConfigQueuesMemoMapPrint function
* @endinternal
*
* @brief   Prints Queues Memories Adresses.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number .
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
*/
GT_STATUS prvCpssPpConfigQueuesMemoMapPrint
(
    IN  GT_U8                  devNum
)
{
    PRV_CPSS_INTERRUPT_CTRL_STC     *intCtrlPtr;
    GT_U32                          i;

    if (0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    intCtrlPtr = &(PRV_CPSS_PP_MAC(devNum)->intCtrl);

    /* AU Queues */
    for (i = 0; (i < CPSS_MAX_PORT_GROUPS_CNS); i++)
    {
        if (intCtrlPtr->auDescCtrl[i].blockSize == 0)
            continue;
        cpssOsPrintf(
            "AU Queue: %d blockAddr %08.8p blockSize: 0x%08.8X , currDescIdx :[%d] MG[%d] \n", i,
            intCtrlPtr->auDescCtrl[i].blockAddr,
            intCtrlPtr->auDescCtrl[i].blockSize,
            intCtrlPtr->auDescCtrl[i].currDescIdx,
            intCtrlPtr->auDescCtrl[i].mgUnitId
            );
    }

    /* FU Queues */
    for (i = 0; (i < CPSS_MAX_PORT_GROUPS_CNS); i++)
    {
        if (intCtrlPtr->fuDescCtrl[i].blockSize == 0)
            continue;
        cpssOsPrintf(
            "FU Queue: %d blockAddr %08.8p blockSize: 0x%08.8X , currDescIdx :[%d] ,unreadCncCounters[%d] MG[%d] \n", i,
            intCtrlPtr->fuDescCtrl[i].blockAddr,
            intCtrlPtr->fuDescCtrl[i].blockSize,
            intCtrlPtr->fuDescCtrl[i].currDescIdx,
            intCtrlPtr->fuDescCtrl[i].unreadCncCounters,
            intCtrlPtr->fuDescCtrl[i].mgUnitId
            );
    }

    /* FU Queues - cnc2,3 */
    for (i = 0; (i < CPSS_MAX_PORT_GROUPS_CNS); i++)
    {
        if (intCtrlPtr->cnc23_fuDescCtrl[i].blockSize == 0)
            continue;
        cpssOsPrintf(
            "FU Queue CNC2,3: %d blockAddr %08.8p blockSize: 0x%08.8X , currDescIdx :[%d] ,unreadCncCounters[%d] MG[%d] \n", i,
            intCtrlPtr->cnc23_fuDescCtrl[i].blockAddr,
            intCtrlPtr->cnc23_fuDescCtrl[i].blockSize,
            intCtrlPtr->cnc23_fuDescCtrl[i].currDescIdx,
            intCtrlPtr->cnc23_fuDescCtrl[i].unreadCncCounters,
            intCtrlPtr->cnc23_fuDescCtrl[i].mgUnitId
            );
    }

    /* Secondary AU Queues */
    for (i = 0; (i < CPSS_MAX_PORT_GROUPS_CNS); i++)
    {
        if (intCtrlPtr->secondaryAuDescCtrl[i].blockSize == 0)
            continue;
        cpssOsPrintf(
            "Secondary AU Queue: %d blockAddr %08.8p blockSize: 0x%08.8X , currDescIdx :[%d] MG[%d] \n", i,
            intCtrlPtr->secondaryAuDescCtrl[i].blockAddr,
            intCtrlPtr->secondaryAuDescCtrl[i].blockSize,
            intCtrlPtr->secondaryAuDescCtrl[i].currDescIdx,
            intCtrlPtr->secondaryAuDescCtrl[i].mgUnitId
            );
    }

    /* AU1 Queues */
    for (i = 0; (i < CPSS_MAX_PORT_GROUPS_CNS); i++)
    {
        if (intCtrlPtr->au1DescCtrl[i].blockSize == 0)
            continue;
        cpssOsPrintf(
            "AU1 Queue: %d blockAddr %08.8p blockSize: 0x%08.8X , currDescIdx :[%d]  MG[%d] \n", i,
            intCtrlPtr->au1DescCtrl[i].blockAddr,
            intCtrlPtr->au1DescCtrl[i].blockSize,
            intCtrlPtr->au1DescCtrl[i].currDescIdx,
            intCtrlPtr->au1DescCtrl[i].mgUnitId
            );
    }

    /* Secondary AU1 Queues */
    for (i = 0; (i < CPSS_MAX_PORT_GROUPS_CNS); i++)
    {
        if (intCtrlPtr->secondaryAu1DescCtrl[i].blockSize == 0)
            continue;
        cpssOsPrintf(
            "Secondary AU1 Queue: %d blockAddr %08.8p blockSize: 0x%08.8X , currDescIdx :[%d]  MG[%d] \n", i,
            intCtrlPtr->secondaryAu1DescCtrl[i].blockAddr,
            intCtrlPtr->secondaryAu1DescCtrl[i].blockSize,
            intCtrlPtr->secondaryAu1DescCtrl[i].currDescIdx,
            intCtrlPtr->secondaryAu1DescCtrl[i].mgUnitId
            );
    }


#ifdef CHX_FAMILY
    if(PRV_CPSS_PP_MAC(devNum)->intCtrl.use_sharedCncDescCtrl == GT_TRUE)
    {
        prvCpssDxChSharedCncDescCtrlDbManagerDump(devNum);
    }
#endif/*CHX_FAMILY*/
    return GT_OK;
}

/**
* @internal prvCpssPpConfigDevDbInfoGet function
* @endinternal
*
* @brief   function to get the init parameters given during initialization.
*         it is needed for application that need to do re-init without HW reset ,
*         and want to use same init parameters, that relate to DMAs .
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_NOT_FOUND             - device not found in the DB
* @retval GT_NOT_SUPPORTED         - device not hold DMA info
* @retval GT_NOT_INITIALIZED       - device was not yet initialized with DMA info
* @retval GT_BAD_PTR               - one of the pointers is NULL
*/
GT_STATUS prvCpssPpConfigDevDbInfoGet

(
    IN GT_U8                        devNum,
    OUT CPSS_NET_IF_CFG_STC*        *netIfCfgPtrPtr,
    OUT CPSS_AUQ_CFG_STC*           *auqCfgPtrPtr,
    OUT GT_BOOL*                    *fuqUseSeparatePtrPtr,
    OUT CPSS_AUQ_CFG_STC*           *fuqCfgPtrPtr,
    OUT GT_BOOL*                    *useMultiNetIfSdmaPtrPtr,
    OUT CPSS_MULTI_NET_IF_CFG_STC*  *multiNetIfCfgPtrPtr
)
{
    PRV_CPSS_DMA_RE_INIT_INFO_STC   *dmaInfo;

    CPSS_NULL_PTR_CHECK_MAC(netIfCfgPtrPtr);
    CPSS_NULL_PTR_CHECK_MAC(auqCfgPtrPtr);
    CPSS_NULL_PTR_CHECK_MAC(fuqUseSeparatePtrPtr);
    CPSS_NULL_PTR_CHECK_MAC(fuqCfgPtrPtr);
    CPSS_NULL_PTR_CHECK_MAC(useMultiNetIfSdmaPtrPtr);
    CPSS_NULL_PTR_CHECK_MAC(multiNetIfCfgPtrPtr);

    if(! PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    else if(HWINIT_GLOVAR(
        sysGenGlobalInfo.reInitializationDbPtr[devNum].busBaseAddr) == BASE_ADDR_NOT_USED_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    else if(! PRV_CPSS_HW_IF_PCI_COMPATIBLE_MAC(devNum))
    {
        /* all current info relate to DMA so currently not relevant to non
        PCI/PEX devices */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    else if(GT_FALSE == HWINIT_GLOVAR(
        sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfoValid))
    {
        /* the DMA was not bound yet (not initialized) */
        return /* not error for the LOG */ GT_NOT_INITIALIZED;
    }

    dmaInfo = &(HWINIT_GLOVAR(
        sysGenGlobalInfo.reInitializationDbPtr[devNum].dmaInfo));


    *netIfCfgPtrPtr         = &dmaInfo->netIfCfg;
    *auqCfgPtrPtr           = &dmaInfo->auqCfg;
    *fuqUseSeparatePtrPtr   = &dmaInfo->fuqUseSeparate;
    *fuqCfgPtrPtr           = &dmaInfo->fuqCfg;

    return GT_OK;
}


#if defined  CPSS_USE_MUTEX_PROFILER
void * prvCpssGetPrvCpssPpConfig
(
    IN GT_U32       devNum,
    IN const char *               functionName
)
{
    cpssOsCheckUnprotectedPerDeviceDbAccess(devNum,functionName);
    return  PRV_CPSS_PP_CONFIG_ARR_MAC[devNum];
}


void * prvCpssGetPrvCpssPpConfigInGenFormat
(
    IN GT_U32       devNum,
    IN const char *               functionName
)
{
    cpssOsCheckUnprotectedPerDeviceDbAccess(devNum,functionName);
    return  PRV_CPSS_PP_CONFIG_ARR_MAC[devNum];
}

#endif  /* CPSS_USE_MUTEX_PROFILER */

/**
* @internal prvCpssDefaultAddressUnusedSet function
* @endinternal
*
* @brief   This function set all the register addresses as 'unused'.
*
* @param[in,out] startPtr                 - pointer to start of register addresses
* @param[in] numOfRegs                - number of registers to initialize
* @param[in,out] startPtr                 - pointer to start of register addresses that where initialized
*/
void prvCpssDefaultAddressUnusedSet
(
    INOUT void   *startPtr,
    IN GT_U32   numOfRegs
)
{
    GT_U32        ii;
    GT_U32       *regsAddrPtr32;
    GT_U32        regsAddrPtr32Size;

    /*Fill the all words in the pointer with initial value*/
    regsAddrPtr32       = startPtr;
    regsAddrPtr32Size   = numOfRegs;

    for( ii = 0; ii < regsAddrPtr32Size; ii++ )
    {
        regsAddrPtr32[ii] = PRV_CPSS_SW_PTR_ENTRY_UNUSED;
    }

    return;
}


