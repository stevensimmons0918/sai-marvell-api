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
* @file cpssAppReferenceInit.c
*
* @brief This file includes functions to be called on cpss app platform refernce
* initialization,
*
* @version   47
********************************************************************************
*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
    #include <gtStack/gtOsSocket.h>
    #include <gtOs/gtOsTask.h>
    #include <gtOs/gtOsTimer.h>
    #include <gtUtil/gtBmPool.h>
    #include <gtOs/gtOsGen.h>
    #include <gtOs/gtOsMem.h>
    #include <gtOs/gtOsIo.h>
#ifdef __cplusplus
}
#endif /* __cplusplus */

#include <cmdShell/common/cmdCommon.h>
#include <cmdShell/shell/cmdMain.h>

#include <cpssAppPlatformBoardConfig.h>

#ifndef CPSS_APP_REF_OSMEM_DEFAULT_SIZE_CNS
    /* Default memory size */
    #define CPSS_APP_REF_OSMEM_DEFAULT_SIZE_CNS (2048*1024)
#endif

GT_STATUS cpssEnablerIsAppPlat
(
)
{
#ifdef CPSS_APP_PLATFORM
    return GT_OK;
#else
    return GT_FAIL;
#endif
}

/**
* @internal cpssAppRefOsFuncBind function
* @endinternal
*
* @brief   This function call CPSS to do initial initialization.
*
* @retval GT_OK      - on success.
* @retval GT_FAIL    - on failure.
*
* @note This function must be called before phase 1 init.
*
*/
GT_STATUS cpssAppRefOsFuncBind
(
    GT_VOID
)
{
    GT_STATUS             rc = GT_OK;
    CPSS_OS_FUNC_BIND_STC osFuncBind;

#if defined(SHARED_MEMORY)
    rc = cpssAppPlatformOsCbGet(CPSS_APP_PLATFORM_OS_CALL_TYPE_SHARED_E, &osFuncBind);
#else
    rc = cpssAppPlatformOsCbGet(CPSS_APP_PLATFORM_OS_CALL_TYPE_STATIC_E, &osFuncBind);
#endif
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = cpssExtServicesBind(NULL, &osFuncBind, NULL);
    if(rc != GT_OK)
    {
        return rc;
    }

    return rc;
} /* cpssAppRefOsFuncBind */

/**
* @internal userAppInit function
* @endinternal
*
* @brief   This routine is the starting point of the Driver.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS userAppInit(GT_VOID)
{
    GT_STATUS rc;

#ifdef CPSS_APP_PLATFORM_REFERENCE
    extern GT_STATUS cpssPpInit(void);
#endif

    /* must be called before any Os function */
    rc = osWrapperOpen(NULL);
    if(rc != GT_OK)
    {
        osPrintf("osWrapper initialization failure!\n");
        return rc;
    }

    /* Initialize memory pool. It must be done before any memory allocations */
    /* must be before osWrapperOpen(...) that calls osStaticMalloc(...) */
    rc = osMemInit(CPSS_APP_REF_OSMEM_DEFAULT_SIZE_CNS, GT_TRUE);
    if (rc != GT_OK)
    {
        osPrintf("osMemInit() failed, rc=%d\n", rc);
        return rc;
    }

    rc = cpssAppRefOsFuncBind();
    if (rc != GT_OK)
    {
        osPrintf("cpssAppRefOsFuncBind() failed, rc=%d\n", rc);
        return rc;
    }

#ifdef CPSS_APP_PLATFORM_REFERENCE

    rc = cpssPpInit();

/*
 *  It will fail. Ignore.
    if (rc != GT_OK)
    {
        osPrintf("cpssPpInit() failed, rc=%d\n", rc);
    }
 */

    rc = cmdInit(0);
    if (rc != GT_OK)
    {
        osPrintf("cmdInit() failed, rc=%d\n", rc);
        return rc;
    }
#endif

    return rc;

} /* userAppInitialize */

/***
* @internal cpssAppPlatformWelcome function
* #endinternal
*
* @brief This function is assigned to cmdAppShowBoardsList
*
* @retval GT_OK                    - on success, Always sucess
**/
GT_STATUS cpssAppPlatRefWelcome(GT_VOID)
{
    osPrintf("\nCPSS Application Platform Reference:\n");
    return GT_OK;
}

#define BIND_APP_LEVEL_FUNC(infoPtr,level,funcName)     \
        infoPtr->level.funcName = funcName

/**
* @internal applicationExtServicesBind function
* @endinternal
*
* @brief  mainCmd calls this function, so the application (that implement
*         this function) will bind the mainCmd with OS functions
*
* @param[out] extDrvFuncBindInfoPtr - (pointer to) set of external driver call back functions - NOT USED
* @param[out] osFuncBindPtr         - (pointer to) set of OS call back functions
* @param[out] osExtraFuncBindPtr    - (pointer to) set of extra OS call back functions (that CPSS not use)
* @param[out] extraFuncBindPtr      - (pointer to) set of extra call back functions (that CPSS not use) (non OS functions)
* @param[out] traceFuncBindPtr      - (pointer to) set of Trace call back functions - NOT USED
*
* @retval GT_OK                     - on success
* @retval GT_FAIL                   - on error
*
* @note This function must be implemented by the Application !!!
*
*/
#ifdef CPSS_APP_PLATFORM
GT_STATUS applicationExtServicesBind(
    OUT CPSS_EXT_DRV_FUNC_BIND_STC *extDrvFuncBindInfoPtr,
    OUT CPSS_OS_FUNC_BIND_STC      *osFuncBindPtr,
    OUT CMD_OS_FUNC_BIND_EXTRA_STC *osExtraFuncBindPtr,
    OUT CMD_FUNC_BIND_EXTRA_STC    *extraFuncBindPtr,
    OUT CPSS_TRACE_FUNC_BIND_STC   *traceFuncBindPtr
)
{
    GT_STATUS rc = GT_OK;

    if(extDrvFuncBindInfoPtr == NULL || osFuncBindPtr    == NULL ||
       osExtraFuncBindPtr    == NULL || extraFuncBindPtr == NULL ||
       traceFuncBindPtr      == NULL)
    {
        return GT_BAD_PTR;
    }

#if defined(SHARED_MEMORY)
    rc = cpssAppPlatformOsCbGet(CPSS_APP_PLATFORM_OS_CALL_TYPE_SHARED_E, osFuncBindPtr);
#else
    rc = cpssAppPlatformOsCbGet(CPSS_APP_PLATFORM_OS_CALL_TYPE_STATIC_E, osFuncBindPtr);
#endif
    if(rc != GT_OK)
    {
        return rc;
    }

    osMemSet(extDrvFuncBindInfoPtr,0,sizeof(*extDrvFuncBindInfoPtr));
    osMemSet(traceFuncBindPtr,0,sizeof(*traceFuncBindPtr));

    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketTcpCreate                       );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketUdpCreate                       );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketTcpDestroy                      );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketUdpDestroy                      );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketCreateAddr                      );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketDestroyAddr                     );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketBind                            );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketListen                          );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketAccept                          );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketConnect                         );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketSetNonBlock                     );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketSetBlock                        );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketSend                            );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketSendTo                          );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketRecv                            );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketRecvFrom                        );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketSetSocketNoLinger               );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketExtractIpAddrFromSocketAddr     );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketGetSocketAddrSize               );
    osExtraFuncBindPtr->osSocketsBindInfo.osSocketShutDown = (CPSS_SOCKET_SHUTDOWN_FUNC)osSocketShutDown;
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSelectCreateSet                       );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSelectEraseSet                        );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSelectZeroSet                         );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSelectAddFdToSet                      );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSelectClearFdFromSet                  );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSelectIsFdSet                         );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSelectCopySet                         );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSelect                                );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketGetSocketFdSetSize              );

    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osTasksBindInfo,osTaskGetSelf                            );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osTasksBindInfo,osSetTaskPrior                           );

    extraFuncBindPtr->appDbBindInfo.cmdAppShowBoardsList = cpssAppPlatRefWelcome;

    return rc;
}
#endif
