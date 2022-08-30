/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/********************************************************************************
* cpssHalInitServices.c
*
* DESCRIPTION:
*       Services initialization
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <cpssHalExt.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

extern GT_STATUS appDemoWrap_osTaskCreate
(
    IN  const GT_CHAR *name,
    IN  GT_U32  prio,
    IN  GT_U32  stack,
    IN  unsigned(__TASKCONV *start_addr)(void*),
    IN  void    *arglist,
    OUT GT_TASK *tid
);

extern GT_STATUS appDemoWrap_osTaskDelete
(
    IN GT_TASK tid
);

#ifdef ASIC_SIMULATION
#include <cpssHalSimPrv.h>
#include <cpssHalSim.h>
#endif

#include <gtExtDrv/drivers/gtIntDrv.h>
#include <gtExtDrv/drivers/gtPciDrv.h>
#include <gtExtDrv/drivers/gtCacheMng.h>
#include <gtExtDrv/drivers/gtDmaDrv.h>
#include <gtExtDrv/drivers/gtDragoniteDrv.h>
#include <gtExtDrv/drivers/gtEthPortCtrl.h>
#include <gtExtDrv/drivers/gtHsuDrv.h>
#include <gtExtDrv/drivers/gtSmiDrvCtrl.h>
#include <gtExtDrv/drivers/gtSmiHwCtrl.h>

#include <gtStack/gtOsSocket.h>

#include <gtOs/gtOsGen.h>
#include <gtOs/gtOsExc.h>
#include <gtOs/gtOsInet.h>
#include <gtOs/gtOsStdLib.h>
#include <gtOs/gtOsMsgQ.h>
#include <gtOs/gtOsMem.h>

#include <gtUtil/gtBmPool.h>

#include <cmdShell/common/cmdExtServices.h>
#include <cpss/generic/events/cpssGenEventUnifyTypes.h>
#include <cpssCommon/cpssPresteraDefs.h>


char **cmdArgv;
char **cmdEnv;

extern void mrvlExtOsLogFunc
(
    CPSS_LOG_LIB_ENT      lib,
    CPSS_LOG_TYPE_ENT     type,
    const char*           format,
    ...
);

GT_STATUS cpssHalTraceHwAccessWriteFunc
(
    IN GT_U8    cpssDevNum,
    IN GT_U32   portGroupId,
    IN GT_BOOL  isrContext,
    IN CPSS_TRACE_HW_ACCESS_ADDR_SPACE_ENT  addrSpace,
    IN GT_U32   addr,
    IN GT_U32   length,
    IN GT_U32   *dataPtr,
    IN GT_U32   mask
);
GT_STATUS cpssHalTraceHwAccessReadFunc
(
    IN GT_U8    cpssDevNum,
    IN GT_U32   portGroupId,
    IN GT_BOOL  isrContext,
    IN CPSS_TRACE_HW_ACCESS_ADDR_SPACE_ENT  addrSpace,
    IN GT_U32   addr,
    IN GT_U32   length,
    IN GT_U32   *dataPtr
);
#define FORCE_FUNC_CAST (void*)

/*******************************************************************************
* cpssHalInitServicesGetDefaultExtDrvFuncs
*
* DESCRIPTION:
*       Receives default cpss bind from extDrv
* INPUTS:
*       None.
* OUTPUTS:
*       extDrvFuncBindInfoPtr - (pointer to) set of call back functions
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       Function should reside into CPSS library to resolve correct
*       pointers to functions.
*
*******************************************************************************/
GT_STATUS cpssHalInitServicesGetDefaultExtDrvFuncs
(
    CPSS_EXT_DRV_FUNC_BIND_STC  *extDrvFuncBindInfoPtr
)
{
    osMemSet(extDrvFuncBindInfoPtr, 0, sizeof(*extDrvFuncBindInfoPtr));

    /* bind the external drivers functions to the CPSS */
    extDrvFuncBindInfoPtr->extDrvMgmtCacheBindInfo.extDrvMgmtCacheFlush      =
        FORCE_FUNC_CAST extDrvMgmtCacheFlush;
    extDrvFuncBindInfoPtr->extDrvMgmtCacheBindInfo.extDrvMgmtCacheInvalidate =
        FORCE_FUNC_CAST extDrvMgmtCacheInvalidate;

    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiInitDriverFunc      =
        hwIfSmiInitDriver;
    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiWriteRegFunc        =
        hwIfSmiWriteReg;
    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiReadRegFunc         =
        hwIfSmiReadReg;
    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiTaskRegRamReadFunc  =
        hwIfSmiTskRegRamRead;
    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiTaskRegRamWriteFunc =
        hwIfSmiTskRegRamWrite;
    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiTaskRegVecReadFunc  =
        hwIfSmiTskRegVecRead;
    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiTaskRegVecWriteFunc =
        hwIfSmiTskRegVecWrite;
    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiTaskWriteRegFunc    =
        hwIfSmiTaskWriteReg;
    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiTaskReadRegFunc     =
        hwIfSmiTaskReadReg;
    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiIntReadRegFunc      =
        hwIfSmiInterruptReadReg;
    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiIntWriteRegFunc     =
        hwIfSmiInterruptWriteReg;
    extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiDevVendorIdGetFunc  =
        extDrvSmiDevVendorIdGet;
    /*  used only in linux -- will need to be under some kind of COMPILATION FLAG
        extDrvFuncBindInfoPtr->extDrvHwIfSmiBindInfo.extDrvHwIfSmiTaskWriteFieldFunc  = hwIfSmiTaskWriteRegField;
    */

#ifdef GT_I2C
    extDrvFuncBindInfoPtr->extDrvHwIfTwsiBindInfo.extDrvHwIfTwsiInitDriverFunc =
        hwIfTwsiInitDriver;
    extDrvFuncBindInfoPtr->extDrvHwIfTwsiBindInfo.extDrvHwIfTwsiWriteRegFunc   =
        hwIfTwsiWriteReg;
    extDrvFuncBindInfoPtr->extDrvHwIfTwsiBindInfo.extDrvHwIfTwsiReadRegFunc    =
        hwIfTwsiReadReg;
#endif /* GT_I2C */


    /*  XBAR related services */
#if defined(IMPL_FA) || defined(IMPL_XBAR)
    extDrvFuncBindInfoPtr->extDrvMgmtHwIfBindInfo.extDrvI2cMgmtMasterInitFunc    =
        gtI2cMgmtMasterInit;
    extDrvFuncBindInfoPtr->extDrvMgmtHwIfBindInfo.extDrvMgmtReadRegisterFunc     =
        FORCE_FUNC_CAST extDrvMgmtReadRegister;
    extDrvFuncBindInfoPtr->extDrvMgmtHwIfBindInfo.extDrvMgmtWriteRegisterFunc    =
        FORCE_FUNC_CAST extDrvMgmtWriteRegister;
    extDrvFuncBindInfoPtr->extDrvMgmtHwIfBindInfo.extDrvMgmtIsrReadRegisterFunc  =
        FORCE_FUNC_CAST extDrvMgmtIsrReadRegister;
    extDrvFuncBindInfoPtr->extDrvMgmtHwIfBindInfo.extDrvMgmtIsrWriteRegisterFunc =
        FORCE_FUNC_CAST extDrvMgmtIsrWriteRegister;
#endif

    extDrvFuncBindInfoPtr->extDrvDmaBindInfo.extDrvDmaWriteDriverFunc =
        extDrvDmaWrite;
    extDrvFuncBindInfoPtr->extDrvDmaBindInfo.extDrvDmaReadFunc        =
        extDrvDmaRead;

    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortRxInitFunc
        = extDrvEthPortRxInit;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortTxInitFunc
        = extDrvEthPortTxInit;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortEnableFunc
        = extDrvEthPortEnable;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortDisableFunc
        = extDrvEthPortDisable;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortTxFunc
        = extDrvEthPortTx;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortInputHookAddFunc
        = extDrvEthInputHookAdd;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortTxCompleteHookAddFunc
        = extDrvEthTxCompleteHookAdd;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortRxPacketFreeFunc
        = extDrvEthRxPacketFree;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPortTxModeSetFunc
        = FORCE_FUNC_CAST extDrvEthPortTxModeSet;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthRawSocketModeSetFunc  =
        extDrvEthRawSocketModeSet;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthRawSocketModeGetFunc  =
        extDrvEthRawSocketModeGet;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvLinuxModeSetFunc  =
        extDrvLinuxModeSet;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvLinuxModeGetFunc  =
        extDrvLinuxModeGet;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthRawSocketRxHookAddFunc =
        extDrvEthRawSocketRxHookAdd;

    extDrvFuncBindInfoPtr->extDrvHsuDrvBindInfo.extDrvHsuMemBaseAddrGetFunc =
        extDrvHsuMemBaseAddrGet;
    extDrvFuncBindInfoPtr->extDrvHsuDrvBindInfo.extDrvHsuWarmRestartFunc =
        extDrvHsuWarmRestart;
    extDrvFuncBindInfoPtr->extDrvHsuDrvBindInfo.extDrvHsuInboundSdmaEnableFunc =
        extDrvHsuInboundSdmaEnable;
    extDrvFuncBindInfoPtr->extDrvHsuDrvBindInfo.extDrvHsuInboundSdmaDisableFunc =
        extDrvHsuInboundSdmaDisable;
    extDrvFuncBindInfoPtr->extDrvHsuDrvBindInfo.extDrvHsuInboundSdmaStateGetFunc =
        extDrvHsuInboundSdmaStateGet;

#if defined (XCAT_DRV)
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthCpuCodeToQueueFunc
        = extDrvEthCpuCodeToQueue;
    extDrvFuncBindInfoPtr->extDrvEthPortBindInfo.extDrvEthPrePendTwoBytesHeaderSetFunc
        = extDrvEthPrePendTwoBytesHeaderSet;
#endif

    extDrvFuncBindInfoPtr->extDrvIntBindInfo.extDrvIntConnectFunc     =
        extDrvIntConnect;
    extDrvFuncBindInfoPtr->extDrvIntBindInfo.extDrvIntDisconnectFunc  =
        extDrvIntDisconnect;
    extDrvFuncBindInfoPtr->extDrvIntBindInfo.extDrvIntEnableFunc      =
        extDrvIntEnable;
    extDrvFuncBindInfoPtr->extDrvIntBindInfo.extDrvIntDisableFunc     =
        extDrvIntDisable;
    extDrvFuncBindInfoPtr->extDrvIntBindInfo.extDrvIntLockModeSetFunc =
        FORCE_FUNC_CAST extDrvSetIntLockUnlock;

    extDrvFuncBindInfoPtr->extDrvPciInfo.extDrvPciConfigWriteRegFunc        =
        extDrvPciConfigWriteReg;
    extDrvFuncBindInfoPtr->extDrvPciInfo.extDrvPciConfigReadRegFunc         =
        extDrvPciConfigReadReg;
    extDrvFuncBindInfoPtr->extDrvPciInfo.extDrvPciDevFindFunc               =
        extDrvPciFindDev;
    extDrvFuncBindInfoPtr->extDrvPciInfo.extDrvPciIntVecFunc                =
        FORCE_FUNC_CAST extDrvGetPciIntVec;
    extDrvFuncBindInfoPtr->extDrvPciInfo.extDrvPciIntMaskFunc               =
        FORCE_FUNC_CAST extDrvGetIntMask;
    extDrvFuncBindInfoPtr->extDrvPciInfo.extDrvPciCombinedAccessEnableFunc  =
        extDrvEnableCombinedPciAccess;
    extDrvFuncBindInfoPtr->extDrvPciInfo.extDrvPciDoubleWriteFunc           =
        extDrvPciDoubleWrite;
    extDrvFuncBindInfoPtr->extDrvPciInfo.extDrvPciDoubleReadFunc            =
        extDrvPciDoubleRead;

#ifdef DRAGONITE_TYPE_A1
    extDrvFuncBindInfoPtr->extDrvDragoniteInfo.extDrvDragoniteShMemBaseAddrGetFunc =
        extDrvDragoniteShMemBaseAddrGet;
#endif

    return GT_OK;
}

/*******************************************************************************
* cpssHalInitServicesGetDefaultOsBindFuncs
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
*       pointers to functions.
*
*******************************************************************************/

GT_STATUS cpssHalInitServicesGetDefaultOsBindFuncs
(
    CPSS_OS_FUNC_BIND_STC *osFuncBindPtr
)
{
    osMemSet(osFuncBindPtr, 0, sizeof(*osFuncBindPtr));

    /* bind the OS functions to the CPSS */
    osFuncBindPtr->osMemBindInfo.osMemBzeroFunc =             osBzero;
    osFuncBindPtr->osMemBindInfo.osMemSetFunc   =             osMemSet;
    osFuncBindPtr->osMemBindInfo.osMemCpyFunc   =             osMemCpy;
    osFuncBindPtr->osMemBindInfo.osMemMoveFunc  =             osMemMove;
    osFuncBindPtr->osMemBindInfo.osMemCmpFunc   =             osMemCmp;
    osFuncBindPtr->osMemBindInfo.osMemStaticMallocFunc =      osStaticMalloc;
    osFuncBindPtr->osMemBindInfo.osMemMallocFunc =            osDbgMalloc;
    osFuncBindPtr->osMemBindInfo.osMemReallocFunc =           osRealloc;
    osFuncBindPtr->osMemBindInfo.osMemFreeFunc   =            osFree;
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

    osFuncBindPtr->osSemBindInfo.osMutexCreateFunc     = osMutexCreate;
    osFuncBindPtr->osSemBindInfo.osMutexDeleteFunc     = osMutexDelete;
    osFuncBindPtr->osSemBindInfo.osMutexLockFunc       = osMutexLock;
    osFuncBindPtr->osSemBindInfo.osMutexUnlockFunc     = osMutexUnlock;

    osFuncBindPtr->osSemBindInfo.osSigSemBinCreateFunc = FORCE_FUNC_CAST
                                                         osSemBinCreate;
    osFuncBindPtr->osSemBindInfo.osSigSemMCreateFunc   = osSemMCreate;
    osFuncBindPtr->osSemBindInfo.osSigSemCCreateFunc   = osSemCCreate;
    osFuncBindPtr->osSemBindInfo.osSigSemDeleteFunc    = osSemDelete;
    osFuncBindPtr->osSemBindInfo.osSigSemWaitFunc      = osSemWait;
    osFuncBindPtr->osSemBindInfo.osSigSemSignalFunc    = osSemSignal;

    osFuncBindPtr->osIoBindInfo.osIoBindStdOutFunc  = osBindStdOut;
    osFuncBindPtr->osIoBindInfo.osIoPrintfFunc      = osPrintf;
    osFuncBindPtr->osIoBindInfo.osIoVprintfFunc     = osVprintf;
    osFuncBindPtr->osIoBindInfo.osIoSprintfFunc     = osSprintf;
    osFuncBindPtr->osIoBindInfo.osIoSnprintfFunc    = osSnprintf;
    osFuncBindPtr->osIoBindInfo.osIoVsprintfFunc    = osVsprintf;
    osFuncBindPtr->osIoBindInfo.osIoVsnprintfFunc   = osVsnprintf;
    osFuncBindPtr->osIoBindInfo.osIoPrintSynchFunc  = osPrintSync;
    osFuncBindPtr->osIoBindInfo.osIoGetsFunc        = osGets;

    osFuncBindPtr->osIoBindInfo.osIoFlastErrorStrFunc = osFlastErrorStr;
    osFuncBindPtr->osIoBindInfo.osIoFopenFunc       = osFopen;
    osFuncBindPtr->osIoBindInfo.osIoFcloseFunc      = osFclose;
    osFuncBindPtr->osIoBindInfo.osIoRewindFunc      = osRewind;
    osFuncBindPtr->osIoBindInfo.osIoFprintfFunc     = osFprintf;
    osFuncBindPtr->osIoBindInfo.osIoFgets           = osFgets;
    osFuncBindPtr->osIoBindInfo.osIoFwriteFunc      = osFwrite;
    osFuncBindPtr->osIoBindInfo.osIoFreadFunc       = osFread;
    osFuncBindPtr->osIoBindInfo.osIoFgetLengthFunc  = osFgetLength;
    osFuncBindPtr->osIoBindInfo.osIoFatalErrorFunc  = (CPSS_OS_FATAL_ERROR_FUNC)
                                                      osFatalError;

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
    osFuncBindPtr->osTimeBindInfo.osGetSysClockRateFunc = osGetSysClockRate;

    osFuncBindPtr->osTimeBindInfo.osDelayFunc       = osDelay;

#if (!defined(FREEBSD) && !defined(UCLINUX)) || defined(ASIC_SIMULATION)
    /* this function required for sand_os_mainOs_interface.c
     * Now it is implemented for:
     *   all os with ASIC simulation
     *   vxWorks
     *   Win32
     *   Linux (stub which does nothing)
     */
    osFuncBindPtr->osIntBindInfo.osIntModeSetFunc  = FORCE_FUNC_CAST
                                                     osSetIntLockUnlock;
#endif

    osFuncBindPtr->osRandBindInfo.osRandFunc  = osRand;
    osFuncBindPtr->osRandBindInfo.osSrandFunc = osSrand;

    osFuncBindPtr->osTaskBindInfo.osTaskCreateFunc = appDemoWrap_osTaskCreate;
    osFuncBindPtr->osTaskBindInfo.osTaskDeleteFunc = appDemoWrap_osTaskDelete;
    osFuncBindPtr->osTaskBindInfo.osTaskGetSelfFunc= osTaskGetSelf;
    osFuncBindPtr->osTaskBindInfo.osTaskLockFunc   = osTaskLock;
    osFuncBindPtr->osTaskBindInfo.osTaskUnLockFunc = osTaskUnLock;

    osFuncBindPtr->osStdLibBindInfo.osQsortFunc    = osQsort;
    osFuncBindPtr->osStdLibBindInfo.osBsearchFunc  = osBsearch;
    /*
        osFuncBindPtr->osMsgQBindInfo.osMsgQCreateFunc  = osMsgQCreate;
        osFuncBindPtr->osMsgQBindInfo.osMsgQDeleteFunc  = osMsgQDelete;
        osFuncBindPtr->osMsgQBindInfo.osMsgQSendFunc    = osMsgQSend;
        osFuncBindPtr->osMsgQBindInfo.osMsgQRecvFunc    = osMsgQRecv;
        osFuncBindPtr->osMsgQBindInfo.osMsgQNumMsgsFunc = osMsgQNumMsgs;
        osFuncBindPtr->osMsgQBindInfo.osMsgQNumMsgsFunc = osMsgQNumMsgs;
    */
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
    osFuncBindPtr->osSocketInfo.osSocketSetSocketNoLingerFunc =
        osSocketSetSocketNoLinger;
    osFuncBindPtr->osSocketInfo.osSocketExtractIpAddrFromSocketAddrFunc =
        osSocketExtractIpAddrFromSocketAddr;
    osFuncBindPtr->osSocketInfo.osSocketGetSocketAddrSizeFunc =
        osSocketGetSocketAddrSize;
    osFuncBindPtr->osSocketInfo.osSocketShutDownFunc = (CPSS_SOCKET_SHUTDOWN_FUNC)
                                                       osSocketShutDown;

    osFuncBindPtr->osSocketSelectInfo.osSelectCreateSetFunc = osSelectCreateSet;
    osFuncBindPtr->osSocketSelectInfo.osSelectEraseSetFunc = osSelectEraseSet;
    osFuncBindPtr->osSocketSelectInfo.osSelectZeroSetFunc = osSelectZeroSet;
    osFuncBindPtr->osSocketSelectInfo.osSelectAddFdToSetFunc = osSelectAddFdToSet;
    osFuncBindPtr->osSocketSelectInfo.osSelectClearFdFromSetFunc =
        osSelectClearFdFromSet;
    osFuncBindPtr->osSocketSelectInfo.osSelectIsFdSetFunc = osSelectIsFdSet;
    osFuncBindPtr->osSocketSelectInfo.osSelectCopySetFunc = osSelectCopySet;
    osFuncBindPtr->osSocketSelectInfo.osSelectFunc = osSelect;
    osFuncBindPtr->osSocketSelectInfo.osSocketGetSocketFdSetSizeFunc =
        osSocketGetSocketFdSetSize;

    osFuncBindPtr->osLogBindInfo.osLogFunc = mrvlExtOsLogFunc;

    return GT_OK;
}

/*******************************************************************************
* cpssHalInitServicesGetDefaultTraceFuncs
*
* DESCRIPTION:
*       Receives default cpss bind from trace
* INPUTS:
*       None.
* OUTPUTS:
*       extDrvFuncBindInfoPtr - (pointer to) set of call back functions
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       Function should reside into CPSS library to resolve correct
*       pointers to functions.
*
*******************************************************************************/
GT_STATUS cpssHalInitServicesGetDefaultTraceFuncs
(
    CPSS_TRACE_FUNC_BIND_STC  *traceFuncBindInfoPtr
)
{
    osMemSet(traceFuncBindInfoPtr, 0, sizeof(*traceFuncBindInfoPtr));
    traceFuncBindInfoPtr->traceHwBindInfo.traceHwAccessWriteFunc =
        cpssHalTraceHwAccessWriteFunc;
    traceFuncBindInfoPtr->traceHwBindInfo.traceHwAccessReadFunc =
        cpssHalTraceHwAccessReadFunc;
    return GT_OK;
}

/*******************************************************************************
* cpssHalInitServicesGetPciDev
*
* DESCRIPTION:
*       This routine search for Prestera Devices Over the PCI.
*
* INPUTS:
*       first - whether to bring the first device, if GT_FALSE return the next
*               device.
*       device - array with PCI device information.
*
* OUTPUTS:
*       pciInfo - the next device PCI info.
*
* RETURNS:
*       GT_OK      - on success.
*       GT_FAIL    - otherwise.
*       GT_NO_MORE - no more prestera devices.
*
* COMMENTS:
*       Last GT_PCI_DEV_VENDOR_ID structure in devices array must be filled with zeros.
*
*******************************************************************************/
GT_STATUS cpssHalInitServicesGetPciDev
(
    IN  GT_BOOL     first,
    IN GT_PCI_DEV_VENDOR_ID *device,
    GT_PCI_INFO *pciInfo
)
{
    GT_U32  busNo;                  /* pci bus number */
    GT_U32  deviceNo;               /* PCI device number */
    GT_U32  funcNo;                 /* PCI function number */
    static GT_U32 deviceIdx = 0;    /* device id index */
    static GT_U32 instance = 0;     /* device instance */
    static GT_U32 numOfDevices = 0;/* total number of devices in the system */
    GT_U8   i;
    GT_STATUS ret=0;                  /* function return value */

    /* check parameters */
    if (pciInfo == NULL)
    {
        return GT_BAD_PARAM;
    }
    /* check whether it is the first call */
    if (first == GT_TRUE)
    {
        deviceIdx = 0;
        instance = 0;
        numOfDevices = 0;
    }

#ifdef ASIC_SIMULATION
    if (GT_OK == cpssHalInitServicesSimulationFindPCIDev(device, &instance,
                                                         &deviceIdx, &busNo, &deviceNo, &funcNo))
    {
        goto foundInstanceOnPex_lbl;
    }
#endif

    /* scsimmon */
    /*call the BSP PCI facility to get all Prestera devices */
    for (; device[deviceIdx].vendorId != 0;)
    {
        //DBG_INFO/g(("search the PCI devices 0x%04x\n",device[deviceIdx].devId));



        ret = extDrvPciFindDev(device[deviceIdx].vendorId, device[deviceIdx].devId,
                               instance++, &busNo, &deviceNo, &funcNo);
        if (ret != GT_OK)
        {
            instance = 0; /* first instance for that device type */
            deviceIdx++;
            continue;
        }

        cpssOsPrintf("%s:%d returned %d: FOUND vend %x device %x B/D/F %x/%x/%x\n",
                     __func__,
                     __LINE__, ret,
                     device[deviceIdx].vendorId, device[deviceIdx].devId, busNo, deviceNo, funcNo);

#ifdef ASIC_SIMULATION
foundInstanceOnPex_lbl:
#endif

        numOfDevices++;
        pciInfo->pciDevVendorId = device[deviceIdx];
        pciInfo->pciIdSel   = deviceNo;
        pciInfo->pciBusNum  = busNo;
        pciInfo->funcNo     = funcNo;
        /* Get the Pci header info  */
        for (i = 0; i < 64; i += 4)
        {
            ret = extDrvPciConfigReadReg(busNo, deviceNo, funcNo, i,
                                         &(pciInfo->pciHeaderInfo[i / 4]));
            if (ret != GT_OK)
            {
                return GT_FAIL;
            }
        }

#ifdef ASIC_SIMULATION
        cpssHalInitServicesSimulationPCIInfoUpdate(pciInfo);
#endif

        return GT_OK;
    }
    /* scsimmon */
    return GT_NO_MORE;
}

GT_STATUS cpssHalTraceHwAccessWriteFunc
(
    IN GT_U8    cpssDevNum,
    IN GT_U32   portGroupId,
    IN GT_BOOL  isrContext,
    IN CPSS_TRACE_HW_ACCESS_ADDR_SPACE_ENT  addrSpace,
    IN GT_U32   addr,
    IN GT_U32   length,
    IN GT_U32   *dataPtr,
    IN GT_U32   mask
)
{
    int i, cnt;
    char data[1024]= {'\0'};
    cpssOsPrintf("WRITE cpssDevNum %u portGroupId 0x%x isrContext %d addrSpace %d addr 0x%08x length %u  mask %x\n",
                 cpssDevNum, portGroupId, isrContext, addrSpace, addr, length, mask);
    for (i=0, cnt =0; i<length; i++)
    {
        cnt += cpssOsSprintf(data+cnt, "%08x ", dataPtr[i]);
    }
    data[1023]='\0';
    cpssOsPrintf("data %s\n", data);

    return GT_OK;
}
GT_STATUS cpssHalTraceHwAccessReadFunc
(
    IN GT_U8    cpssDevNum,
    IN GT_U32   portGroupId,
    IN GT_BOOL  isrContext,
    IN CPSS_TRACE_HW_ACCESS_ADDR_SPACE_ENT  addrSpace,
    IN GT_U32   addr,
    IN GT_U32   length,
    IN GT_U32   *dataPtr
)
{
    int i, cnt;
    char data[1024]= {'\0'};
    cpssOsPrintf("READ cpssDevNum %u portGroupId 0x%x isrContext %d addrSpace %d addr 0x%08x length %u \n",
                 cpssDevNum, portGroupId, isrContext, addrSpace, addr, length);
    for (i=0, cnt =0; i<length; i++)
    {
        cnt += cpssOsSprintf(data+cnt, "%08x ", dataPtr[i]);
    }
    data[1023]='\0';
    cpssOsPrintf("data %s\n", data);

    return GT_OK;
}
