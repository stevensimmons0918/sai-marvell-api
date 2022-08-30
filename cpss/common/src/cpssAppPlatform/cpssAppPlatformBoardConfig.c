/*******************************************************************************
*              (c), Copyright 2018, Marvell International Ltd.                 *
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
* @file cpssAppPlatformBoardConfig.c
*
* @brief CPSS Application platform Board initialize
*
* @version   1
********************************************************************************
*/
#include <cpss/common/init/cpssInit.h>
#include <cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>
#include <profiles/cpssAppPlatformProfile.h>
#include <cpssAppPlatformSysConfig.h>
#include <cpssAppPlatformBoardConfig.h>
#include <gtExtDrv/drivers/gtUartDrv.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
    #include <gtStack/gtOsSocket.h>
    #include <gtOs/gtOsInet.h>
    #include <gtOs/gtOsMsgQ.h>
    #include <gtOs/gtOsStdLib.h>
    #include <gtOs/gtOsExc.h>
#ifdef __cplusplus
}
#endif /* __cplusplus */

/** to get extern definition of external Drivers **/
#include <gtExtDrv/drivers/gtPciDrv.h>
#include <gtExtDrv/drivers/gtSmiHwCtrl.h>
#include <gtExtDrv/drivers/gtTwsiHwCtrl.h>
#include <gtExtDrv/drivers/gtIntDrv.h>
#include <gtExtDrv/drivers/gtDmaDrv.h>
#include <gtExtDrv/drivers/gtCacheMng.h>
#include <gtExtDrv/drivers/gtEthPortCtrl.h>
#include <gtExtDrv/drivers/gtHsuDrv.h>

#define FORCE_FUNC_CAST (GT_VOID_PTR)

#ifdef IMPL_GALTIS
extern GT_U32 *enhUtUseCaptureToCpuPtr;
#ifdef SHARED_MEMORY
    enhUtUseCaptureToCpuPtr = (GT_U32*)osMalloc(sizeof(GT_U32)); /* allocated in shared area */
#endif
#ifdef IMPL_TGF
extern GT_U32 *tgfCmdCpssNetEnhUtUseCaptureToCpuPtr;

GT_VOID prvCpssAppPlatformNetEnhUtUseCaptureToCpuSet()
{
    tgfCmdCpssNetEnhUtUseCaptureToCpuPtr = enhUtUseCaptureToCpuPtr;
}
#endif /* IMPL_TGF */
#endif /* IMPL_GALTIS */

/** Note below 2 functions are from gtOs **/
GT_STATUS shrMemGetDefaultExtDrvFuncs
(
    OUT CPSS_EXT_DRV_FUNC_BIND_STC  *extDrvFuncBindInfoPtr
);

GT_STATUS shrMemGetDefaultOsBindFuncsThruDynamicLoader
(
    OUT CPSS_OS_FUNC_BIND_STC *osFuncBindPtr
);

/**
* @internal cpssAppOsLog function
* @endinternal
*
* @brief   Function for printing the logs of cpss log
*
* @param[in] lib                      - the function will print the log of the functions in "lib".
* @param[in] type                     - the function will print the logs from "type".
* @param[in] format                   - usual printf  string.
*                                      ... - additional parameters.
*                                       None.
* @Comments: Oringally from appDemoOsLog
*/
GT_VOID cpssAppOsLog
(
    IN    CPSS_LOG_LIB_ENT      lib,
    IN    CPSS_LOG_TYPE_ENT     type,
    IN    const char*           format,
    ...
)
{
    /*** This Logging mechanism can be improved further ***/
    va_list args;
    char buffer[2048];
    (void)lib;
    (void)type;
    va_start(args, format);
    cpssOsVsprintf(buffer, format, args);
    va_end(args);
    /** default print on screen ***/
    CPSS_APP_PLATFORM_LOG_PRINT_MAC(buffer);
}

/*******************************************************************************
* prvcpssAppPlatGetDefaultOsBindFuncs
*
* DESCRIPTION:
*       Receives default cpss bind from OS
* INPUTS:
*       None.
* OUTPUTS:
*       osFuncBindPtr - (pointer to) set of call back functions
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       Function should reside into CPSS library to resolve correct
*       pointers to functions. This is originaly taken from cpssGetDefaultExtDrvFuncs
*
*******************************************************************************/
static GT_STATUS prvCpssAppPlatGetDefaultOsBindFuncs
(
    OUT CPSS_OS_FUNC_BIND_STC *osFuncBindPtr
)
{
    osMemSet(osFuncBindPtr,0,sizeof(*osFuncBindPtr));

    /* bind the OS functions to the CPSS */
    osFuncBindPtr->osMemBindInfo.osMemBzeroFunc =             osBzero;
    osFuncBindPtr->osMemBindInfo.osMemSetFunc   =             osMemSet;
    osFuncBindPtr->osMemBindInfo.osMemCpyFunc   =             osMemCpy;
    osFuncBindPtr->osMemBindInfo.osMemMoveFunc  =             osMemMove;
    osFuncBindPtr->osMemBindInfo.osMemCmpFunc   =             osMemCmp;
    osFuncBindPtr->osMemBindInfo.osMemStaticMallocFunc =      osStaticMalloc;
#ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG
    osFuncBindPtr->osMemBindInfo.osMemMallocFunc =            osDbgMalloc_MemoryLeakageDbg;
    osFuncBindPtr->osMemBindInfo.osMemReallocFunc =           osRealloc_MemoryLeakageDbg;
    osFuncBindPtr->osMemBindInfo.osMemFreeFunc   =            osFree_MemoryLeakageDbg;
#else
    osFuncBindPtr->osMemBindInfo.osMemMallocFunc =            osDbgMalloc;
    osFuncBindPtr->osMemBindInfo.osMemReallocFunc =           osRealloc;
    osFuncBindPtr->osMemBindInfo.osMemFreeFunc   =            osFree;
#endif
    osFuncBindPtr->osMemBindInfo.osMemCacheDmaMallocFunc =    osCacheDmaMalloc;
    osFuncBindPtr->osMemBindInfo.osMemCacheDmaFreeFunc =      osCacheDmaFree;
    osFuncBindPtr->osMemBindInfo.osMemPhyToVirtFunc =         osPhy2Virt;
    osFuncBindPtr->osMemBindInfo.osMemVirtToPhyFunc =         osVirt2Phy;

    osFuncBindPtr->osStrBindInfo.osStrlenFunc       = osStrlen;
    osFuncBindPtr->osStrBindInfo.osStrCpyFunc       = osStrCpy;
    osFuncBindPtr->osStrBindInfo.osStrNCpyFunc      = osStrNCpy;
    osFuncBindPtr->osStrBindInfo.osStrChrFunc       = osStrChr;
    osFuncBindPtr->osStrBindInfo.osStrStrFunc       = osStrStr;
    osFuncBindPtr->osStrBindInfo.osStrRevChrFunc    = osStrrChr;
    osFuncBindPtr->osStrBindInfo.osStrCmpFunc       = osStrCmp;
    osFuncBindPtr->osStrBindInfo.osStrNCmpFunc      = osStrNCmp;
    osFuncBindPtr->osStrBindInfo.osStrCatFunc       = osStrCat;
    osFuncBindPtr->osStrBindInfo.osStrStrNCatFunc   = osStrNCat;
    osFuncBindPtr->osStrBindInfo.osStrChrToUpperFunc= osToUpper;
    osFuncBindPtr->osStrBindInfo.osStrTo32Func      = osStrTo32;
    osFuncBindPtr->osStrBindInfo.osStrToU32Func     = osStrToU32;
    osFuncBindPtr->osStrBindInfo.osStrTolFunc       = osStrTol;

    osFuncBindPtr->osSemBindInfo.osMutexCreateFunc     = osMutexCreate;
    osFuncBindPtr->osSemBindInfo.osMutexDeleteFunc     = osMutexDelete;
 #if defined  CPSS_USE_MUTEX_PROFILER
    osFuncBindPtr->osSemBindInfo.osMutexSetGlAttributesFunc = osMutexSetGlAttributes;
    osFuncBindPtr->osSemBindInfo.osCheckUnprotectedPerDeviceDbAccess = osCheckUnprotectedPerDeviceDbAccess;
 #endif

    osFuncBindPtr->osSemBindInfo.osMutexLockFunc       = osMutexLock;
    osFuncBindPtr->osSemBindInfo.osMutexUnlockFunc     = osMutexUnlock;

    osFuncBindPtr->osSemBindInfo.osSigSemBinCreateFunc = FORCE_FUNC_CAST osSemBinCreate;
#ifdef  LINUX
    osFuncBindPtr->osSemBindInfo.osSigSemMCreateFunc   = osSemMCreate;
    osFuncBindPtr->osSemBindInfo.osSigSemCCreateFunc   = osSemCCreate;
#endif
    osFuncBindPtr->osSemBindInfo.osSigSemDeleteFunc    = osSemDelete;
    osFuncBindPtr->osSemBindInfo.osSigSemWaitFunc      = osSemWait;
    osFuncBindPtr->osSemBindInfo.osSigSemSignalFunc    = osSemSignal;

    osFuncBindPtr->osIoBindInfo.osIoBindStdOutFunc  = osBindStdOut;
    osFuncBindPtr->osIoBindInfo.osIoPrintfFunc      = osPrintf;
#ifdef  LINUX
    osFuncBindPtr->osIoBindInfo.osIoVprintfFunc     = osVprintf;
#endif
    osFuncBindPtr->osIoBindInfo.osIoSprintfFunc     = osSprintf;
    osFuncBindPtr->osIoBindInfo.osIoSnprintfFunc    = osSnprintf;
#if defined(LINUX) || defined(VXWORKS) || defined(WIN32)
    osFuncBindPtr->osIoBindInfo.osIoVsprintfFunc    = osVsprintf;
    osFuncBindPtr->osIoBindInfo.osIoVsnprintfFunc   = osVsnprintf;
#endif
    osFuncBindPtr->osIoBindInfo.osIoPrintSynchFunc  = osPrintSync;
    osFuncBindPtr->osIoBindInfo.osIoGetsFunc        = osGets;

#if defined(LINUX) || defined(FREEBSD) || defined(WIN32)
    osFuncBindPtr->osIoBindInfo.osIoFlastErrorStrFunc = osFlastErrorStr;
    osFuncBindPtr->osIoBindInfo.osIoFopenFunc       = osFopen;
    osFuncBindPtr->osIoBindInfo.osIoFcloseFunc      = osFclose;
    osFuncBindPtr->osIoBindInfo.osIoRewindFunc      = osRewind;
    osFuncBindPtr->osIoBindInfo.osIoFprintfFunc     = osFprintf;
    osFuncBindPtr->osIoBindInfo.osIoFgets           = osFgets;
    osFuncBindPtr->osIoBindInfo.osIoFwriteFunc      = osFwrite;
    osFuncBindPtr->osIoBindInfo.osIoFreadFunc       = osFread;
    osFuncBindPtr->osIoBindInfo.osIoFgetLengthFunc  = osFgetLength;
    osFuncBindPtr->osIoBindInfo.osIoFatalErrorFunc  = (CPSS_OS_FATAL_ERROR_FUNC)osFatalError;
#endif

    osFuncBindPtr->osInetBindInfo.osInetNtohlFunc = osNtohl;
    osFuncBindPtr->osInetBindInfo.osInetHtonlFunc = osHtonl;
    osFuncBindPtr->osInetBindInfo.osInetNtohsFunc = osNtohs;
    osFuncBindPtr->osInetBindInfo.osInetHtonsFunc = osHtons;
    osFuncBindPtr->osInetBindInfo.osInetNtoaFunc  = osInetNtoa;

    osFuncBindPtr->osTimeBindInfo.osTimeWkAfterFunc = osTimerWkAfter;
    osFuncBindPtr->osTimeBindInfo.osTimeTickGetFunc = osTickGet;
    osFuncBindPtr->osTimeBindInfo.osTimeGetFunc     = osTime;
    osFuncBindPtr->osTimeBindInfo.osTimeRTFunc      = osTimeRT;
    osFuncBindPtr->osTimeBindInfo.osStrftimeFunc    = osStrftime;
#if defined(LINUX) || defined(VXWORKS) || defined(WIN32)
    osFuncBindPtr->osTimeBindInfo.osGetSysClockRateFunc = osGetSysClockRate;
#endif

#ifdef  LINUX
    osFuncBindPtr->osTimeBindInfo.osDelayFunc       = osDelay;
#endif

#if !defined(UNIX) || defined(ASIC_SIMULATION)
    osFuncBindPtr->osIntBindInfo.osIntEnableFunc   = osIntEnable;
    osFuncBindPtr->osIntBindInfo.osIntDisableFunc  = osIntDisable;
    osFuncBindPtr->osIntBindInfo.osIntConnectFunc  = osInterruptConnect;
    osFuncBindPtr->osIntBindInfo.osIntDisconnectFunc  = osInterruptDisconnect;
#endif
#if (!defined(FREEBSD) && !defined(UCLINUX)) || defined(ASIC_SIMULATION)
    /* this function required for sand_os_mainOs_interface.c
     * Now it is implemented for:
     *   all os with ASIC simulation
     *   vxWorks
     *   Win32
     *   Linux (stub which does nothing)
     */
    osFuncBindPtr->osIntBindInfo.osIntModeSetFunc  = FORCE_FUNC_CAST osSetIntLockUnlock;
#endif
    osFuncBindPtr->osRandBindInfo.osRandFunc  = osRand;
    osFuncBindPtr->osRandBindInfo.osSrandFunc = osSrand;

    osFuncBindPtr->osTaskBindInfo.osTaskCreateFunc = osTaskCreate;
    osFuncBindPtr->osTaskBindInfo.osTaskDeleteFunc = osTaskDelete;
    osFuncBindPtr->osTaskBindInfo.osTaskGetSelfFunc= osTaskGetSelf;
    osFuncBindPtr->osTaskBindInfo.osTaskLockFunc   = osTaskLock;
    osFuncBindPtr->osTaskBindInfo.osTaskUnLockFunc = osTaskUnLock;

#if defined(LINUX) || defined(VXWORKS) || defined(ASIC_SIMULATION)
    osFuncBindPtr->osStdLibBindInfo.osQsortFunc    = osQsort;
    osFuncBindPtr->osStdLibBindInfo.osBsearchFunc  = osBsearch;
#endif

    osFuncBindPtr->osMsgQBindInfo.osMsgQCreateFunc  = osMsgQCreate;
    osFuncBindPtr->osMsgQBindInfo.osMsgQDeleteFunc  = osMsgQDelete;
    osFuncBindPtr->osMsgQBindInfo.osMsgQSendFunc    = osMsgQSend;
    osFuncBindPtr->osMsgQBindInfo.osMsgQRecvFunc    = osMsgQRecv;
    osFuncBindPtr->osMsgQBindInfo.osMsgQNumMsgsFunc = osMsgQNumMsgs;
    osFuncBindPtr->osMsgQBindInfo.osMsgQNumMsgsFunc = osMsgQNumMsgs;
    osFuncBindPtr->osLogBindInfo.osLogFunc          = cpssAppOsLog;

    osFuncBindPtr->osSocketInfo.osSocketLastErrorFunc = osSocketLastError;
    osFuncBindPtr->osSocketInfo.osSocketLastErrorStrFunc = osSocketLastErrorStr;
    osFuncBindPtr->osSocketInfo.osSocketTcpCreateFunc = osSocketTcpCreate;
    osFuncBindPtr->osSocketInfo.osSocketUdpCreateFunc = osSocketUdpCreate;
    osFuncBindPtr->osSocketInfo.osSocketTcpDestroyFunc = osSocketTcpDestroy;
    osFuncBindPtr->osSocketInfo.osSocketUdpDestroyFunc = osSocketUdpDestroy;
    osFuncBindPtr->osSocketInfo.osSocketCreateAddrFunc = osSocketCreateAddr;
    osFuncBindPtr->osSocketInfo.osSocketDestroyAddrFunc = osSocketDestroyAddr;
    osFuncBindPtr->osSocketInfo.osSocketBindFunc = osSocketBind;
    osFuncBindPtr->osSocketInfo.osSocketListenFunc = osSocketListen;
    osFuncBindPtr->osSocketInfo.osSocketAcceptFunc = osSocketAccept;
    osFuncBindPtr->osSocketInfo.osSocketConnectFunc = osSocketConnect;
    osFuncBindPtr->osSocketInfo.osSocketSetNonBlockFunc = osSocketSetNonBlock;
    osFuncBindPtr->osSocketInfo.osSocketSetBlockFunc = osSocketSetBlock;
    osFuncBindPtr->osSocketInfo.osSocketSendFunc = osSocketSend;
    osFuncBindPtr->osSocketInfo.osSocketSendToFunc = osSocketSendTo;
    osFuncBindPtr->osSocketInfo.osSocketRecvFunc = osSocketRecv;
    osFuncBindPtr->osSocketInfo.osSocketRecvFromFunc = osSocketRecvFrom;
    osFuncBindPtr->osSocketInfo.osSocketSetSocketNoLingerFunc = osSocketSetSocketNoLinger;
    osFuncBindPtr->osSocketInfo.osSocketExtractIpAddrFromSocketAddrFunc = osSocketExtractIpAddrFromSocketAddr;
    osFuncBindPtr->osSocketInfo.osSocketGetSocketAddrSizeFunc = osSocketGetSocketAddrSize;
    osFuncBindPtr->osSocketInfo.osSocketShutDownFunc = (CPSS_SOCKET_SHUTDOWN_FUNC)osSocketShutDown;

    osFuncBindPtr->osSocketSelectInfo.osSelectCreateSetFunc = osSelectCreateSet;
    osFuncBindPtr->osSocketSelectInfo.osSelectEraseSetFunc = osSelectEraseSet;
    osFuncBindPtr->osSocketSelectInfo.osSelectZeroSetFunc = osSelectZeroSet;
    osFuncBindPtr->osSocketSelectInfo.osSelectAddFdToSetFunc = osSelectAddFdToSet;
    osFuncBindPtr->osSocketSelectInfo.osSelectClearFdFromSetFunc = osSelectClearFdFromSet;
    osFuncBindPtr->osSocketSelectInfo.osSelectIsFdSetFunc = osSelectIsFdSet;
    osFuncBindPtr->osSocketSelectInfo.osSelectCopySetFunc = osSelectCopySet;
    osFuncBindPtr->osSocketSelectInfo.osSelectFunc = osSelect;
    osFuncBindPtr->osSocketSelectInfo.osSocketGetSocketFdSetSizeFunc = osSocketGetSocketFdSetSize;

    /* Now we should be sure that ALL pointers are set.
     * Occasionally you can add new field into the structure and forget
     * to initialize it!
     *
     * Warning! Here we suppose that compiler doesn't realign this structure!!!
     * Thus we can scan all words inside structure as pointers, there are no
     * alignment bytes there! */

    return GT_OK;
}

/**
* @internal prvCpssAppPlatGetDefaultTraceFuncs function
* @endinternal
*
* @brief   Receives default cpss bind from trace
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note Function should reside into CPSS library to resolve correct
*       pointers to functions. Copied from cpssGetDefaultTraceFuncs.
*
*/
static GT_STATUS prvCpssAppPlatGetDefaultTraceFuncs
(
    OUT CPSS_TRACE_FUNC_BIND_STC  *traceFuncBindInfoPtr
)
{
    osMemSet(traceFuncBindInfoPtr,0,sizeof(*traceFuncBindInfoPtr));

    /* bind the external drivers functions to the CPSS */
    traceFuncBindInfoPtr->traceHwBindInfo.traceHwAccessReadFunc  = NULL;
    traceFuncBindInfoPtr->traceHwBindInfo.traceHwAccessWriteFunc = NULL;
    traceFuncBindInfoPtr->traceHwBindInfo.traceHwAccessDelayFunc = NULL;

    return GT_OK;
}

/**
* @internal prvCpssAppPlatGetDefaultExtDrvFuncs function
* @endinternal
*
* @brief   Receives default cpss bind from extDrv
*
* @param[out] extDrvFuncBindInfoPtr    - (pointer to) set of call back functions
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note Function should reside into CPSS library to resolve correct
*       pointers to functions. Copied from cpssGetDefaultExtDrvFuncs
*
*/
static GT_STATUS prvCpssAppPlatGetDefaultExtDrvFuncs
(
    OUT CPSS_EXT_DRV_FUNC_BIND_STC  *extDrvFuncBindInfoPtr
)
{
    osMemSet(extDrvFuncBindInfoPtr,0,sizeof(*extDrvFuncBindInfoPtr));

    /* bind the external drivers functions to the CPSS */
    extDrvFuncBindInfoPtr->extDrvMgmtCacheBindInfo.extDrvMgmtCacheFlush      = FORCE_FUNC_CAST extDrvMgmtCacheFlush;
    extDrvFuncBindInfoPtr->extDrvMgmtCacheBindInfo.extDrvMgmtCacheInvalidate = FORCE_FUNC_CAST extDrvMgmtCacheInvalidate;

    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiInitDriverFunc      = hwIfSmiInitDriver;
    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiWriteRegFunc        = hwIfSmiWriteReg;
    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiReadRegFunc         = hwIfSmiReadReg;
    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiTaskRegRamReadFunc  = hwIfSmiTskRegRamRead;
    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiTaskRegRamWriteFunc = hwIfSmiTskRegRamWrite;
    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiTaskRegVecReadFunc  = hwIfSmiTskRegVecRead;
    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiTaskRegVecWriteFunc = hwIfSmiTskRegVecWrite;
    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiTaskWriteRegFunc    = hwIfSmiTaskWriteReg;
    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiTaskReadRegFunc     = hwIfSmiTaskReadReg;
    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiIntReadRegFunc      = hwIfSmiInterruptReadReg;
    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiIntWriteRegFunc     = hwIfSmiInterruptWriteReg;
    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiDevVendorIdGetFunc  = extDrvSmiDevVendorIdGet;
/*  used only in linux -- will need to be under some kind of COMPILATION FLAG
    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiTaskWriteFieldFunc  = hwIfSmiTaskWriteRegField;
*/

#ifdef GT_I2C
    extDrvFuncBindInfoPtr->extDrvHwIfTwsiBindInfo.extDrvHwIfTwsiInitDriverFunc = hwIfTwsiInitDriver;
    extDrvFuncBindInfoPtr->extDrvHwIfTwsiBindInfo.extDrvHwIfTwsiWriteRegFunc   = hwIfTwsiWriteReg;
    extDrvFuncBindInfoPtr->extDrvHwIfTwsiBindInfo.extDrvHwIfTwsiReadRegFunc    = hwIfTwsiReadReg;
#endif /* GT_I2C */

    extDrvFuncBindInfoPtr->extDrvDmaBindInfo.extDrvDmaWriteDriverFunc = extDrvDmaWrite;
    extDrvFuncBindInfoPtr->extDrvDmaBindInfo.extDrvDmaReadFunc        = extDrvDmaRead;

    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortRxInitFunc            = extDrvEthPortRxInit;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortTxInitFunc            = extDrvEthPortTxInit;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortEnableFunc            = extDrvEthPortEnable;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortDisableFunc           = extDrvEthPortDisable;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortTxFunc                = extDrvEthPortTx;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortInputHookAddFunc      = extDrvEthInputHookAdd;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortTxCompleteHookAddFunc = extDrvEthTxCompleteHookAdd;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortRxPacketFreeFunc      = extDrvEthRxPacketFree;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortTxModeSetFunc         = FORCE_FUNC_CAST extDrvEthPortTxModeSet;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthRawSocketModeSetFunc  = extDrvEthRawSocketModeSet;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthRawSocketModeGetFunc  = extDrvEthRawSocketModeGet;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvLinuxModeSetFunc  = extDrvLinuxModeSet;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvLinuxModeGetFunc  = extDrvLinuxModeGet;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthRawSocketRxHookAddFunc = extDrvEthRawSocketRxHookAdd;

    extDrvFuncBindInfoPtr->extDrvHsuDrvBindInfo.extDrvHsuMemBaseAddrGetFunc = extDrvHsuMemBaseAddrGet;
    extDrvFuncBindInfoPtr->extDrvHsuDrvBindInfo.extDrvHsuWarmRestartFunc = extDrvHsuWarmRestart;
    extDrvFuncBindInfoPtr->extDrvHsuDrvBindInfo.extDrvHsuInboundSdmaEnableFunc = extDrvHsuInboundSdmaEnable;
    extDrvFuncBindInfoPtr->extDrvHsuDrvBindInfo.extDrvHsuInboundSdmaDisableFunc = extDrvHsuInboundSdmaDisable;
    extDrvFuncBindInfoPtr->extDrvHsuDrvBindInfo.extDrvHsuInboundSdmaStateGetFunc = extDrvHsuInboundSdmaStateGet;

#if defined (XCAT_DRV)
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthCpuCodeToQueueFunc        = extDrvEthCpuCodeToQueue;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPrePendTwoBytesHeaderSetFunc = extDrvEthPrePendTwoBytesHeaderSet;
#endif

    extDrvFuncBindInfoPtr->extDrvIntBindInfo.extDrvIntConnectFunc     = extDrvIntConnect;
    extDrvFuncBindInfoPtr->extDrvIntBindInfo.extDrvIntDisconnectFunc =  extDrvIntDisconnect;
    extDrvFuncBindInfoPtr->extDrvIntBindInfo.extDrvIntEnableFunc      = extDrvIntEnable;
    extDrvFuncBindInfoPtr->extDrvIntBindInfo.extDrvIntDisableFunc     = extDrvIntDisable;
    extDrvFuncBindInfoPtr->extDrvIntBindInfo.extDrvIntLockModeSetFunc = FORCE_FUNC_CAST extDrvSetIntLockUnlock;

    extDrvFuncBindInfoPtr->extDrvPciInfo.extDrvPciConfigWriteRegFunc        = extDrvPciConfigWriteReg;
    extDrvFuncBindInfoPtr->extDrvPciInfo.extDrvPciConfigReadRegFunc         = extDrvPciConfigReadReg;
    extDrvFuncBindInfoPtr->extDrvPciInfo.extDrvPciDevFindFunc               = extDrvPciFindDev;
    extDrvFuncBindInfoPtr->extDrvPciInfo.extDrvPciIntVecFunc                = FORCE_FUNC_CAST extDrvGetPciIntVec;
    extDrvFuncBindInfoPtr->extDrvPciInfo.extDrvPciIntMaskFunc               = FORCE_FUNC_CAST extDrvGetIntMask;
    extDrvFuncBindInfoPtr->extDrvPciInfo.extDrvPciCombinedAccessEnableFunc  = extDrvEnableCombinedPciAccess;
    extDrvFuncBindInfoPtr->extDrvPciInfo.extDrvPciDoubleWriteFunc           = extDrvPciDoubleWrite;
    extDrvFuncBindInfoPtr->extDrvPciInfo.extDrvPciDoubleReadFunc            = extDrvPciDoubleRead;
    extDrvFuncBindInfoPtr->extDrvPciInfo.extDrvPciSetDevIdFunc              = extDrvPciSetDevId;

#ifdef DRAGONITE_TYPE_A1
    extDrvFuncBindInfoPtr->extDrvDragoniteInfo.extDrvDragoniteShMemBaseAddrGetFunc = extDrvDragoniteShMemBaseAddrGet;
#endif

    return GT_OK;
}

/**
* @internal prvCpssAppPlatformBoardInit function
* @endinternal
*
* @brief   Bind OS and external driver calls.
*
* @param [in] *profileListPtr     - Board profile,
* @param [in] *systemRecovery     - system recovery mode
*
* @retval GT_OK                   - on success,
* @retval GT_FAIl                 - otherwise.
*/
extern GT_VOID libhelper_constructor();

static GT_STATUS prvCpssAppPlatformBoardInit
(
    IN CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_STC *boardProfilePtr,
    IN CPSS_SYSTEM_RECOVERY_INFO_STC      *systemRecovery
)
{
    GT_STATUS rc = GT_OK;
    CPSS_OS_FUNC_BIND_STC       osFuncBindInfo;
    CPSS_EXT_DRV_FUNC_BIND_STC  extDrvFuncBindInfo;
    CPSS_TRACE_FUNC_BIND_STC    traceFuncBindInfo;
    (void)systemRecovery;

    CPSS_APP_PLATFORM_LOG_DBG_MAC("Profile: %s, OsCall:%d , drvCall:%d\n", boardProfilePtr->boardName,
                      boardProfilePtr->osCallType, boardProfilePtr->extDrvCallType);

    if(   boardProfilePtr->osCallType == CPSS_APP_PLATFORM_OS_CALL_TYPE_SHARED_E
        || boardProfilePtr->extDrvCallType == CPSS_APP_PLATFORM_EXT_DRV_CALL_TYPE_SHARED_E
      )
    {
        libhelper_constructor();
    }

    CPSS_APP_PLATFORM_LOG_PRINT_MAC("Board Name :%s\n", boardProfilePtr->boardName);

    rc = cpssAppPlatformOsCbGet(boardProfilePtr->osCallType, &osFuncBindInfo);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformBcOsGet);

    rc = cpssAppPlatformExtDrvCbGet(boardProfilePtr->extDrvCallType, &extDrvFuncBindInfo);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformBcExtDrvGet);

    rc = prvCpssAppPlatGetDefaultTraceFuncs(&traceFuncBindInfo);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvCpssAppPlatGetDefaultTraceFuncs);

    rc = cpssExtServicesBind(&extDrvFuncBindInfo, &osFuncBindInfo, &traceFuncBindInfo);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssExtServicesBind);

    rc = cpssPpInit();
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssPpInit);

#if defined(CPSS_LOG_ENABLE)
{
    char        buffer[80];
    const char *prvCpssLogErrorLogFileName;

#if defined  WIN32
    prvCpssLogErrorLogFileName = "c:\\temp\\cpss_log_entry";
#else
    prvCpssLogErrorLogFileName = "/tmp/cpss_log_entry";
#endif
    /* Create unique history file name from constant string and current time*/
    cpssOsSprintf(buffer, "%s_%d", prvCpssLogErrorLogFileName, osTime());
    /* Set log history file name */
    rc = cpssLogHistoryFileNameSet(buffer);
    if(rc != GT_OK)
    {
        return rc;
    }
}
#endif


    rc = extDrvUartInit();
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, extDrvUartInit);

    /* Call to fatal_error initialization, use default fatal error call_back */
    rc = osFatalErrorInit(NULL);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, osFatalErrorInit);

#ifdef IMPL_GALTIS
#ifdef IMPL_TGF
    prvCpssAppPlatformNetEnhUtUseCaptureToCpuSet();
#endif
#endif

    return rc;
}

/*
* @internal cpssAppPlatformBoardInit function
* @endinternal
*
* @brief   initialize the board OS and External Services.
*
* @param[in] inputProfileList    - Profile list containing board profile(s).
* @param[in] systemRecovery      - system recovery mode.
*
* @retval GT_OK                  - on success.
* @retval GT_BAD_PARAM           - if board profile is not present.
* @retval GT_FAIL                - otherwise.
*/
GT_STATUS cpssAppPlatformBoardInit
(
    IN CPSS_APP_PLATFORM_PROFILE_STC  *inputProfileList,
    IN CPSS_SYSTEM_RECOVERY_INFO_STC *systemRecovery
)
{
    GT_STATUS                           rc = GT_OK;
    GT_BOOL                             boardProfileFound = GT_FALSE;
    CPSS_APP_PLATFORM_PROFILE_STC       *profileListPtr = inputProfileList;
    CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_STC *boardProfilePtr = NULL;

    if(profileListPtr == NULL)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("Profile list is NULL\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_APP_START_LOOP_PROFILE_MAC(profileListPtr, CPSS_APP_PLATFORM_PROFILE_TYPE_BOARD_E)
    {
        if(profileListPtr->profileValue.boardInfoPtr == NULL)
        {
            /* board profile value is NULL. proceed to next profile */
            continue;
        }
        switch(profileListPtr->profileValue.boardInfoPtr->boardInfoType)
        {
            case CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_E:
                boardProfilePtr = profileListPtr->profileValue.boardInfoPtr->boardParam.boardPtr;
                if(boardProfilePtr == NULL)
                {
                    /* board profile value is NULL. proceed to next profile */
                    continue;
                }
                boardProfileFound = GT_TRUE;
                rc = prvCpssAppPlatformBoardInit(boardProfilePtr, systemRecovery);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvCpssAppPlatformBoardInit);

                break;
            case CPSS_APP_PLATFORM_BOARD_PARAM_LED_E:
                /* TBD */
                break;
#ifdef CPSS_APP_PLATFORM_PHASE_2
            case CPSS_APP_PLATFORM_BOARD_PARAM_CASCADE_E:
                /* TBD */
                break;
#endif
            default:
                /* do nothing */
                break;
        }
    }
    PRV_CPSS_APP_END_LOOP_PROFILE_MAC(profileListPtr, CPSS_APP_PLATFORM_PROFILE_TYPE_BOARD_E)

    if(boardProfileFound == GT_FALSE)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("Board profile not found\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/*
* @internal cpssAppPlatformOsCbGet function
* @endinternal
*
* @brief   Get OS callbacks.
*
* @param[in] osCallType     - OS call type static or shared.
*
* @param[OUT] osFuncBindPtr - OS callback functions.
*
* @retval GT_OK             - on success.
* @retval GT_FAIL           - otherwise.
*/
GT_STATUS cpssAppPlatformOsCbGet
(
    IN  CPSS_APP_PLATFORM_OS_CALL_TYPE_ENT  osCallType,
    OUT CPSS_OS_FUNC_BIND_STC             *osFuncBindPtr
)
{
    GT_STATUS rc = GT_OK;

    switch(osCallType)
    {
        case CPSS_APP_PLATFORM_OS_CALL_TYPE_STATIC_E:
            rc = prvCpssAppPlatGetDefaultOsBindFuncs(osFuncBindPtr);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvCpssAppPlatGetDefaultOsBindFuncs);
            break;
        case CPSS_APP_PLATFORM_OS_CALL_TYPE_SHARED_E:
            rc = shrMemGetDefaultOsBindFuncsThruDynamicLoader(osFuncBindPtr);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, shrMemGetDefaultOsBindFuncsThruDynamicLoader);
            break;
        case CPSS_APP_PLATFORM_OS_CALL_TYPE_EXTERNAL_E:
            break;
        case CPSS_APP_PLATFORM_OS_CALL_TYPE_LAST_E:
        default:
            rc = GT_BAD_PARAM;
            break;
    }

    return rc;
}

/*
* @internal cpssAppPlatformExtDrvCbGet function
* @endinternal
*
* @brief   Get External services callbacks.
*
* @param[in] drvCallType            - external driver type static or shared.
*
* @param[OUT] extDrvFuncBindInfoPtr - external driver callback functions.
*
* @retval GT_OK                     - on success.
* @retval GT_FAIL                   - otherwise.
*/
GT_STATUS cpssAppPlatformExtDrvCbGet
(
    IN  CPSS_APP_PLATFORM_EXT_DRV_CALL_TYPE_ENT   drvCallType,
    OUT CPSS_EXT_DRV_FUNC_BIND_STC              *extDrvFuncBindInfoPtr
)
{
    GT_STATUS rc = GT_OK;

    switch(drvCallType)
    {
        case CPSS_APP_PLATFORM_EXT_DRV_CALL_TYPE_STATIC_E:
            prvCpssAppPlatGetDefaultExtDrvFuncs(extDrvFuncBindInfoPtr);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvCpssAppPlatGetDefaultExtDrvFuncs);
            break;
        case CPSS_APP_PLATFORM_EXT_DRV_CALL_TYPE_SHARED_E:
            shrMemGetDefaultExtDrvFuncs(extDrvFuncBindInfoPtr);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, hrMemGetDefaultExtDrvFuncs);
            break;
        case CPSS_APP_PLATFORM_EXT_DRV_CALL_TYPE_EXTERNAL_E:
            break;
        case CPSS_APP_PLATFORM_EXT_DRV_CALL_TYPE_LAST_E:
        default:
            rc = GT_BAD_PARAM;
            break;
    }

    return rc;
}
