/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file appDemoBoardConfig.c
*
* @brief Includes board specific initialization definitions and data-structures.
*
* @version   131
********************************************************************************
*/



#include <cpss/common/cpssTypes.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>


#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#ifdef CHX_FAMILY
#include <appDemo/utils/hsu/appDemoDxChHsuUtils.h>
#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>
#endif
#include <appDemo/userExit/userEventHandler.h>


#include <appDemo/sysHwConfig/appDemoDb.h>
#include <cpss/common/systemRecovery/private/prvCpssCommonSystemRecoveryParallel.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


#ifdef CPSS_APP_PLATFORM

#include <cpssAppPlatformSysConfig.h>
#include <appReference/cpssAppRefUtils.h>

extern GT_CHAR_PTR cpssAppPlatformLastUsedProfileGet
(
    GT_VOID
);

extern GT_STATUS cpssAppPlatformSysInit
(
   IN GT_CHAR  *profileName
);

#endif

#if (defined ASIC_SIMULATION_ENV_FORBIDDEN && defined ASIC_SIMULATION)
    /* 'Forbid' the code to run as ASIC_SIMULATION ... we need 'like' HW compilation ! */
    #undef ASIC_SIMULATION
#endif

#ifdef ASIC_SIMULATION

    #include <os/simTypesBind.h>

    extern GT_U32 skernelStatusGet
    (
        void
    );

#endif /*ASIC_SIMULATION*/

/**
* @internal cpssDmaFreeAll  function
* @endinternal
*
* @brief   Free DMA allocation
* @param[in] devNum         - cpss Logical Device number.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssDmaFreeAll
(
   GT_VOID
)
{
    GT_STATUS rc =GT_OK;

    /* destroy DMA only after other registered function detached from the DMA */
    osPrintSync("appDemoDmaFreeAll : destroy All DMA allocations \n");
    osCacheDmaFreeAll();

    if(GT_OK != rc)
    {
       return rc;
    }

    /********************************/
    /* wait until the reset is done */
    /********************************/
    #ifdef ASIC_SIMULATION
    #ifndef _WIN32 /* it seems that linux miss something */
        SIM_OS_MAC(simOsSleep)(100);
    #endif /*_WIN32*/
    while(skernelStatusGet())
    {
        osTimerWkAfter(10);
        osPrintf("7");
    }
    osPrintf("\n");
    #else /* not ASIC_SIMULATION */
    osTimerWkAfter(10);
    #endif /* not ASIC_SIMULATION */

    return rc;
}

GT_STATUS appDemoAskModeEnableGet
(
   OUT  GT_BOOL * askModeEnablePtr
)
{
    if(NULL==askModeEnablePtr)
    {
        return GT_BAD_PTR;
    }
#ifdef CPSS_APP_PLATFORM
    *askModeEnablePtr = GT_TRUE;
#else
    *askModeEnablePtr = GT_FALSE;
#endif
    return GT_OK;
}

/**
* @internal cpssEmulateSwCrash function
* @endinternal
*
* @brief   Simulate SW crash.Required for HA testing.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoHaEmulateSwCrash
(
    GT_VOID
)
{
    GT_STATUS rc;
    CPSS_SYSTEM_RECOVERY_INFO_STC   newSystemRecoveryInfo;
    GT_U32 i;
#ifndef CPSS_APP_PLATFORM
    APP_DEMO_PP_CONFIG *appDemoPpConfigListPtr =  NULL;

    if (appDemoCpssPciProvisonDone == GT_TRUE)
    {
        appDemoPpConfigListPtr = cpssOsMalloc(APP_DEMO_PP_CONFIG_SIZE_CNS * sizeof(APP_DEMO_PP_CONFIG));
        cpssOsMemSet(appDemoPpConfigListPtr,0,APP_DEMO_PP_CONFIG_SIZE_CNS * sizeof(APP_DEMO_PP_CONFIG));
    }
#endif

    /* state to the special DB that the device did HW reset */
    for (i = 0; i < PRV_CPSS_MAX_PP_DEVICES_CNS; i++)
    {
      if(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[i] == NULL)
      {
          continue;
      }

      prvCpssPpConfigDevDbHwResetSet(i,GT_TRUE);

#ifdef CPSS_APP_PLATFORM

        rc = cpssDevSupportSystemResetSet(i,GT_TRUE);
        if(GT_OK != rc)
        {
           return rc;
        }

#endif


#ifndef CPSS_APP_PLATFORM

        if (appDemoCpssPciProvisonDone == GT_TRUE)
        {
            cpssOsMemCpy(&appDemoPpConfigListPtr[i], &appDemoPpConfigList[i],sizeof(APP_DEMO_PP_CONFIG) );
        }
#endif
    }

    /*disable write to HW*/

    cpssOsMemSet(&newSystemRecoveryInfo.systemRecoveryMode,0,sizeof(CPSS_SYSTEM_RECOVERY_MODE_STC));
    newSystemRecoveryInfo.systemRecoveryState = CPSS_SYSTEM_RECOVERY_INIT_STATE_E;
    newSystemRecoveryInfo.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_HA_E;

    rc = cpssSystemRecoveryStateSet(&newSystemRecoveryInfo);
    if(GT_OK != rc)
    {
       return rc;
    }

    /*clear systemRecoveryManagers Db */
    cpssOsMemSet(&(PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_MANAGERS_DB),0,sizeof(PRV_CPSS_SYSTEM_RECOVERY_MANAGER_STC));

    /*reset system - HW is not really reseted since we disabled write to HW*/
    rc = cpssResetSystem(GT_FALSE);
    if(GT_OK != rc)
    {
       return rc;
    }
 #ifndef CPSS_APP_PLATFORM
    if (appDemoCpssPciProvisonDone == GT_TRUE)
    {
        GT_U32 j;
        for (i = 0; (i < PRV_CPSS_MAX_PP_DEVICES_CNS); i++)
        {
            if (appDemoPpConfigListPtr[i].valid == GT_TRUE)
            {
                appDemoPpConfigList[i].valid = GT_TRUE;
                appDemoPpConfigList[i].channel = appDemoPpConfigListPtr[i].channel;
                appDemoPpConfigList[i].pciInfo.pciDevVendorId.vendorId =  appDemoPpConfigListPtr[i].pciInfo.pciDevVendorId.vendorId;
                appDemoPpConfigList[i].pciInfo.pciDevVendorId.devId = appDemoPpConfigListPtr[i].pciInfo.pciDevVendorId.devId;
                appDemoPpConfigList[i].pciInfo.pciIdSel = appDemoPpConfigListPtr[i].pciInfo.pciIdSel;
                appDemoPpConfigList[i].pciInfo.pciBusNum = appDemoPpConfigListPtr[i].pciInfo.pciBusNum;
                appDemoPpConfigList[i].pciInfo.funcNo = appDemoPpConfigListPtr[i].pciInfo.funcNo;

                /* Get the Pci header info  */
                for(j = 0; j < 16; j ++)
                {
                    appDemoPpConfigList[i].pciInfo.pciHeaderInfo[j] = appDemoPpConfigListPtr[i].pciInfo.pciHeaderInfo[j];
                }
                appDemoPpConfigList[i].devNum = appDemoPpConfigListPtr[i].devNum;
                appDemoPpConfigList[i].hwDevNum = appDemoPpConfigListPtr[i].hwDevNum;
                appDemoPpConfigList[i].devFamily = appDemoPpConfigListPtr[i].devFamily;
            }
        }
        cpssOsFree(appDemoPpConfigListPtr);
    }
#endif

#ifdef CPSS_APP_PLATFORM
    /*in case ISR was not gracefuly terminated while locking the mutex*/
    if(prvCpssIntrScanMtx!=(CPSS_OS_MUTEX)0)
    {
        PRV_CPSS_INT_SCAN_UNLOCK();
    }
#endif

    return cpssDmaFreeAll();

}

#ifdef CPSS_APP_PLATFORM
GT_STATUS cpssReInitSystem
(
    GT_VOID
)
{

    return cpssAppPlatformSysInit(cpssAppPlatformLastUsedProfileGet());
}

GT_STATUS appDemoEventRequestDrvnModeInit
(
    IN GT_VOID
)
{
    GT_STATUS rc;
    CPSS_APP_PLATFORM_PROFILE_STC *profilePtr = NULL;

    rc = cpssAppPlatformProfileGet(cpssAppPlatformLastUsedProfileGet(), &profilePtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformProfileGet);

    rc = cpssAppPlatformUserEventsInit(profilePtr);
    if(rc != GT_OK)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssAppPlatformUserEventsInit ret=%d\r\n", rc);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }


    return rc ;
}


#endif

