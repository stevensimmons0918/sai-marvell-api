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

#if SHELL_ENABLE


#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <cpssHalShell.h>
//#include "cpssHalDevice.h"
#include <cpssHalExt.h>
/* Initialize lock/unlock mutex */
#include <cpss/common/private/prvCpssGlobalMutex.h>
#include <cmdShell/os/cmdOs.h>
#include <cmdShell/common/cmdExtServices.h>
#include <cmdShell/os/cmdStreamImpl.h>
#include <cmdShell/shell/userInitCmdDb.h>
#include <cmdShell/FS/cmdFS.h>
#include <gtUtil/gtBmPool.h>
#include <gtOs/gtOsTask.h>

#include "cpss/generic/events/cpssGenEventUnifyTypes.h"
#include "cpss/generic/events/cpssGenEventRequests.h"
#include "cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h"
#include "cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h"
#include "extUtils/rxEventHandler/rxEventHandler.h"
#include "cpssCommon/cpssPresteraDefs.h"
#include "extUtils/IOStream/IOStream.h"

#define XPS_CPSS_INVALID_TTY 7

extern CMD_STARTUP_SHELL_TYPE_ENT cmdDefaultShellType;
extern GT_STATUS cmdInit(GT_U32  devNum);
extern GT_STATUS luaCLI_tcpInstance(IOStreamPTR stream);
extern GT_STATUS luaCLI_LoadConfigurationFile(IOStreamPTR IOStream,
                                              GT_VOID_PTR *luaInstancePtrPtr);

static GT_U32  enhUtUseCaptureToCpu = 0;
GT_U32 *enhHalUtUseCaptureToCpuPtr = &enhUtUseCaptureToCpu;
extern GT_U32 *tgfCmdCpssNetEnhUtUseCaptureToCpuPtr;
GT_UINTPTR  rxEventHanderHnd; /* For LUA callbacks */
GT_U32   cpssHalInitializeShellTaskId;

GT_STATUS osTimerWkAfter
(
    GT_U32 mils
);
#define MAX_NUM_DEVICES  128
GT_U32   *uniEventCounters[MAX_NUM_DEVICES];
typedef struct
{
    GT_U32      accessParamsBmp;
    GT_U32      addr;
    GT_U32      data;
} APP_DEMO_CPSS_HW_ACCESS_DB_STC;
static unsigned int cpssHalInitializeCmdEvent_count[4]= {0, 0, 0, 0};
void cpssHalInitializeCmdEvent(int i)
{
    cpssHalInitializeCmdEvent_count[i]++;
    return;
}

/*******************************************************************************
* cpssHalInitializeShellTask
*
* DESCRIPTION:
*
* INPUTS:
*       None.
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
*
*******************************************************************************/
unsigned __TASKCONV cpssHalInitializeShellTask
(
    GT_VOID * param
)
{
    GT_STATUS                   status;
    GT_U8                       devNum;
    GT_U8                       queue;
    GT_U32                      numOfBuff=1;
    GT_U8                       *packetBuffs[1];
    GT_U32                      buffLen[1];
    GT_U32_PTR                  wait_ptr = NULL;
    GT_U32                      evExtData;
    CPSS_DXCH_NET_RX_PARAMS_STC rxParams;
    GT_U32      evBitmapArr[CPSS_UNI_EV_BITMAP_SIZE_CNS] = {0}; /* event bitmap array */

    GT_UINTPTR  pktReceiveEventsHndlr = 0;

    CPSS_UNI_EV_CAUSE_ENT pktReceiveEventsArr[] =
    {
        CPSS_PP_RX_BUFFER_QUEUE0_E,
        CPSS_PP_RX_BUFFER_QUEUE1_E,
        CPSS_PP_RX_BUFFER_QUEUE2_E,
        CPSS_PP_RX_BUFFER_QUEUE3_E,
        CPSS_PP_RX_BUFFER_QUEUE4_E,
        CPSS_PP_RX_BUFFER_QUEUE5_E,
        CPSS_PP_RX_BUFFER_QUEUE6_E,
        CPSS_PP_RX_BUFFER_QUEUE7_E
    };

    status = rxEventHandlerLibInit();
    if (status != GT_OK)
    {
        return -1;
    }

    status = rxEventHandlerInitHandler(RX_EV_HANDLER_DEVNUM_ALL,
                                       RX_EV_HANDLER_QUEUE_ALL,
                                       RX_EV_HANDLER_TYPE_RX_E, &rxEventHanderHnd);
    if (status != GT_OK)
    {
        return -1;
    }

    status = cpssEventBind(pktReceiveEventsArr,
                           (sizeof(pktReceiveEventsArr)/sizeof(CPSS_UNI_EV_CAUSE_ENT)),
                           &pktReceiveEventsHndlr);
    if (status != GT_OK)
    {
        return -1;
    }

    for (queue = 0; queue < CPSS_TC_RANGE_CNS; queue++)
    {
        status = cpssEventDeviceMaskSet(0, pktReceiveEventsArr[queue],
                                        CPSS_EVENT_UNMASK_E);
    }

    while (1)
    {
        status = cpssEventSelect(pktReceiveEventsHndlr,
                                 wait_ptr,
                                 evBitmapArr,
                                 CPSS_UNI_EV_BITMAP_SIZE_CNS);
        cpssHalInitializeCmdEvent(0);
        for (queue = 0; queue < CPSS_TC_RANGE_CNS; queue++)
        {
            status = cpssEventRecv(pktReceiveEventsHndlr,
                                   CPSS_PP_RX_BUFFER_QUEUE0_E+queue,
                                   &evExtData,
                                   &devNum);
            cpssHalInitializeCmdEvent(1);

            while (status ==  GT_OK)
            {
                numOfBuff=1;
                status = cpssDxChNetIfSdmaRxPacketGet(devNum,
                                                      queue,
                                                      &numOfBuff,
                                                      packetBuffs,
                                                      buffLen,
                                                      &rxParams);
                cpssHalInitializeCmdEvent(2);
                if (status == GT_OK)
                {
                    /* send the buffer to the application - appCallback */
                    rxEventHandlerDoCallbacks(rxEventHanderHnd,
                                              RX_EV_HANDLER_TYPE_RX_E,
                                              devNum,
                                              queue,
                                              numOfBuff,
                                              packetBuffs,
                                              buffLen,
                                              &rxParams);
                    cpssHalInitializeCmdEvent(3);
                    cpssDxChNetIfRxBufFree(devNum, queue, packetBuffs, numOfBuff);
                }
            }
        }
    }
    return 0;
}


GT_STATUS xpsCpssCmdInit(void)
{
    GT_STATUS retVal;

    /* initialize external services (can't call directly to mainOs functions)*/
    if ((retVal = cmdInitExtServices()) != GT_OK)
    {
        return retVal;
    }

    if ((retVal = cmdStreamSocketInit()) != GT_OK)
    {
        /* os wrapper failed to initialize */
        cmdOsPrintf("cmdStreamSocketInit() initialization failure! %d\n", retVal);
        return retVal;
    }
    /* add test app level commands to commander database */
    if ((retVal = cmdUserInitDatabase()) != GT_OK)
    {
        cmdOsPrintf("failed to initialize command database! %d\n", retVal);
        return retVal;
    }

    cmdFSinit();

    /*TODO revisit*/
    //luaCLI_LoadConfigurationFile( NULL , &(defaultStdio->customPtr));
    //luaCLI_LoadConfigurationFile( NULL , NULL);

    return GT_OK;
}
void cmdShellEnable(GT_BOOL en)
{
    GT_STATUS retVal;
    static GT_U32 taskId = -1;
    CPSS_DXCH_CFG_DEV_INFO_STC      devInfo;
    GT_STATUS st = GT_OK;
    GT_BOOL isM0 = GT_FALSE;

    st = cpssDxChCfgDevInfoGet(0, &devInfo);
    if (st != GT_OK)
    {
        return;
    }
    if ((devInfo.genDevInfo.devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E) ||
        (devInfo.genDevInfo.devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E))
    {
        isM0 = GT_TRUE;
    }

    /* initialize LUA CLI */
    if (en)
    {
        if (taskId != -1)
        {
            return;
        }
        cmdStreamSocketServiceCreate(
            "luaCLI",
            ((isM0)?"127.0.0.1":NULL), 12345/*port*/,
            //NULL, 12345/*port*/,
            cmdSocketInstanceHandler,
            GT_TRUE/*multipleInstances*/,
            GT_TRUE/*isConsole*/);
        /* Create one single task to handle all services */
        if ((retVal = cmdStreamSocketServiceStart("luaCLI")) != GT_OK)
        {
            cmdOsPrintf("commander: failed to create LUA CLI service %d\n", retVal);
        }
        if (cmdOsTaskCreate(
                "CPSSGenServer",
                STREAM_THREAD_PRIO+1,   /* thread priority          */
                65536,                  /* use default stack size   */
                (unsigned(__TASKCONV *)(void*))
                cmdStreamSocketServiceListenerTask,
                NULL,
                &(taskId)) != GT_OK)
        {
            cmdOsPrintf("commander: failed to create CPSSGenServer task thread\n");
        }
        else
        {
            cmdOsPrintf("commander: thread running:%s\n\n", "LUA_CLI");
        }
    }
    else
    {
        if (taskId == -1)
        {
            return;
        }
        cmdOsTaskDelete(taskId);
        taskId = -1;
        if ((retVal = cmdStreamSocketServiceCloseAll()) != GT_OK)
        {
            cmdOsPrintf("commander: failed to stop LUA CLI service %d\n", retVal);
        }
    }
}
/*******************************************************************************
* cpssHalInitializeCmdInitApi
*
* DESCRIPTION:
*
* INPUTS:
*       None.
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
*
*******************************************************************************/
GT_STATUS cpssHalInitializeCmdInitApi(void)
{

    tgfCmdCpssNetEnhUtUseCaptureToCpuPtr = enhHalUtUseCaptureToCpuPtr;
    cmdDefaultShellType = CMD_STARTUP_SHELL_LUACLI_E;
    osTimerWkAfter(500);

    xpsCpssCmdInit();
    return GT_OK;
}
/*******************************************************************************
* applicationExtServicesBind
*
* DESCRIPTION:
*
* INPUTS:
*       None.
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
*
*******************************************************************************/
#ifdef NO_MULTI_DEFINITION
static GT_BOOL cpssHalInitializeCmdCpuEtherPortUsed(void)
{
    printf("STUB - %s\n", __func__);
    return GT_FALSE;
}
GT_STATUS cpssHalInitializeCmdDbDump(GT_VOID)
{
    printf("STUB - %s\n", __func__);
    return GT_OK;
}
GT_STATUS cpssResetSystem(GT_BOOL doHwReset)
{
    printf("STUB - %s\n", __func__);
    return GT_OK;
}
GT_STATUS cpssHalInitializeCmdShowBoardsList(GT_VOID)
{
    printf("STUB - %s\n", __func__);
    return GT_OK;
}
static GT_BOOL cpssHalInitializeCmdIsSystemInitialized(void)
{
    printf("STUB - %s\n", __func__);
    return GT_OK;
}
static GT_STATUS cpssHalInitializeCmdPpConfigPrint(GT_U8  devNum)
{
    printf("STUB - %s\n", __func__);
    return GT_OK;
}
GT_STATUS   cpssHalInitializeCmdAllowProcessingOfAuqMessages(GT_BOOL enable)
{
    printf("STUB - %s\n", __func__);
    return GT_OK;
}
static GT_STATUS prvcpssHalInitializeCmdTraceHwAccessOutputModeSet(
    GT_U32   mode)
{
    printf("STUB - %s\n", __func__);
    return GT_OK;
}
static GT_STATUS prvcpssHalInitializeCmdEventFatalErrorEnable(
    GT_32 fatalErrorType)
{
    printf("STUB - %s\n", __func__);
    return GT_OK;
}
GT_STATUS cpssHalInitializeCmdDbEntryAdd(
    GT_CHAR *namePtr,
    GT_U32  value
)
{
    printf("STUB - %s\n", __func__);
    return GT_OK;
}
GT_STATUS cpssHalInitializeCmdDbEntryGet(
    GT_CHAR *namePtr,
    GT_U32  *valuePtr
)
{
    printf("STUB - %s\n", __func__);
    return GT_NO_SUCH;
}

static GT_STATUS cpssHalInitializeCmdPpConfigGet(
    GT_U8    devIndex,
    CMD_APP_PP_CONFIG* ppConfigPtr
)
{
    printf("STUB - %s\n", __func__);
    return GT_OK;
}
GT_STATUS cpssInitSystem(
    GT_U32  boardIdx,
    GT_U32  boardRevId,
    GT_U32  reloadEeprom
)
{
    printf("STUB - %s\n", __func__);
    return GT_OK;
}
void cpssInitSystemGet(
    GT_U32  *boardIdxPtr,
    GT_U32  *boardRevIdPtr,
    GT_U32  *reloadEepromPtr
)
{
    printf("STUB - %s\n", __func__);
    *boardIdxPtr = 0;
    *boardRevIdPtr = 0;
    *reloadEepromPtr = 0;
}
GT_STATUS   confi2InitSystem(
    GT_U8        theConfiId,
    GT_BOOL      echoOn
)
{
    printf("STUB - %s\n", __func__);
    return GT_OK;
}
static GT_STATUS prvcpssHalInitializeCmdTraceHwAccessEnable(
    GT_U8                devNum,
    GT_U32               accessType,
    GT_BOOL              enable
)
{
    printf("STUB - %s\n", __func__);
    return GT_OK;
}
GT_STATUS cpssHalInitializeCmdEventsDataBaseGet(
    GT_U32 ***eventCounterBlockGet
)
{
    printf("STUB - %s\n", __func__);
    *eventCounterBlockGet = uniEventCounters;
    return GT_OK;
}
GT_STATUS appDemoGenEventCounterGet
(
    GT_U8                    devNum,
    CPSS_UNI_EV_CAUSE_ENT    uniEvent,
    GT_BOOL                  clearOnRead,
    GT_U32                 *counterPtr
)
{
    printf("STUB - %s\n", __func__);
    *counterPtr=0;
    return GT_OK;
}
GT_STATUS appDemoTraceHwAccessDbLine
(
    GT_U32 index,
    APP_DEMO_CPSS_HW_ACCESS_DB_STC * dbLine
)
{
    printf("STUB - %s\n", __func__);
    dbLine->accessParamsBmp=0;
    dbLine->addr=0;
    dbLine->data=0;
    return GT_OK;
}
GT_U32 appDemoDxChTcamTtiBaseIndexGet
(
    GT_U8                            devNum,
    GT_U32                           hitNum
)
{
    printf("STUB - %s\n", __func__);
    return 0;
}
GT_U32 appDemoDxChTcamTtiNumOfIndexsGet
(
    GT_U8                            devNum,
    GT_U32                           hitNum
)
{
    printf("STUB - %s\n", __func__);
    return 0;
}
GT_STATUS cmdCpssInitDatabase(void)
{
    printf("STUB - %s\n", __func__);
    return GT_OK;
}

GT_STATUS   applicationExtServicesBind(
    CPSS_EXT_DRV_FUNC_BIND_STC   *extDrvFuncBindInfoPtr,
    CPSS_OS_FUNC_BIND_STC        *osFuncBindPtr,
    CMD_OS_FUNC_BIND_EXTRA_STC   *osExtraFuncBindPtr,
    CMD_FUNC_BIND_EXTRA_STC      *extraFuncBindPtr,
    CPSS_TRACE_FUNC_BIND_STC     *traceFuncBindPtr
)
{

    GT_STATUS rc;

    memset(extraFuncBindPtr, 0, sizeof(CMD_FUNC_BIND_EXTRA_STC));
    rc = cpssHalInitServicesGetDefaultExtDrvFuncs(extDrvFuncBindInfoPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssHalInitServicesGetDefaultOsBindFuncs(osFuncBindPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssHalInitServicesGetDefaultTraceFuncs(traceFuncBindPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    osExtraFuncBindPtr->osSocketsBindInfo.osSocketTcpCreate =
        osFuncBindPtr->osSocketInfo.osSocketTcpCreateFunc;
    osExtraFuncBindPtr->osSocketsBindInfo.osSocketUdpCreate =
        osFuncBindPtr->osSocketInfo.osSocketUdpCreateFunc;
    osExtraFuncBindPtr->osSocketsBindInfo.osSocketTcpDestroy =
        osFuncBindPtr->osSocketInfo.osSocketTcpDestroyFunc;
    osExtraFuncBindPtr->osSocketsBindInfo.osSocketUdpDestroy =
        osFuncBindPtr->osSocketInfo.osSocketUdpDestroyFunc;
    osExtraFuncBindPtr->osSocketsBindInfo.osSocketCreateAddr =
        osFuncBindPtr->osSocketInfo.osSocketCreateAddrFunc;
    osExtraFuncBindPtr->osSocketsBindInfo.osSocketDestroyAddr =
        osFuncBindPtr->osSocketInfo.osSocketDestroyAddrFunc;
    osExtraFuncBindPtr->osSocketsBindInfo.osSocketBind =
        osFuncBindPtr->osSocketInfo.osSocketBindFunc;
    osExtraFuncBindPtr->osSocketsBindInfo.osSocketListen =
        osFuncBindPtr->osSocketInfo.osSocketListenFunc;
    osExtraFuncBindPtr->osSocketsBindInfo.osSocketAccept =
        osFuncBindPtr->osSocketInfo.osSocketAcceptFunc;
    osExtraFuncBindPtr->osSocketsBindInfo.osSocketConnect =
        osFuncBindPtr->osSocketInfo.osSocketConnectFunc;
    osExtraFuncBindPtr->osSocketsBindInfo.osSocketSetNonBlock =
        osFuncBindPtr->osSocketInfo.osSocketSetNonBlockFunc;
    osExtraFuncBindPtr->osSocketsBindInfo.osSocketSetBlock =
        osFuncBindPtr->osSocketInfo.osSocketSetBlockFunc;
    osExtraFuncBindPtr->osSocketsBindInfo.osSocketSend =
        osFuncBindPtr->osSocketInfo.osSocketSendFunc;
    osExtraFuncBindPtr->osSocketsBindInfo.osSocketSendTo =
        osFuncBindPtr->osSocketInfo.osSocketSendToFunc;
    osExtraFuncBindPtr->osSocketsBindInfo.osSocketRecv =
        osFuncBindPtr->osSocketInfo.osSocketRecvFunc;
    osExtraFuncBindPtr->osSocketsBindInfo.osSocketRecvFrom =
        osFuncBindPtr->osSocketInfo.osSocketRecvFromFunc;
    osExtraFuncBindPtr->osSocketsBindInfo.osSocketSetSocketNoLinger =
        osFuncBindPtr->osSocketInfo.osSocketSetSocketNoLingerFunc;
    osExtraFuncBindPtr->osSocketsBindInfo.osSocketExtractIpAddrFromSocketAddr =
        osFuncBindPtr->osSocketInfo.osSocketExtractIpAddrFromSocketAddrFunc;
    osExtraFuncBindPtr->osSocketsBindInfo.osSocketGetSocketAddrSize =
        osFuncBindPtr->osSocketInfo.osSocketGetSocketAddrSizeFunc;
    osExtraFuncBindPtr->osSocketsBindInfo.osSocketShutDown =
        (CMD_OS_SOCKET_SHUT_DOWN_FUN)osFuncBindPtr->osSocketInfo.osSocketShutDownFunc;

    osExtraFuncBindPtr->osSocketsBindInfo.osSelectCreateSet =
        osFuncBindPtr->osSocketSelectInfo.osSelectCreateSetFunc;
    osExtraFuncBindPtr->osSocketsBindInfo.osSelectEraseSet =
        osFuncBindPtr->osSocketSelectInfo.osSelectEraseSetFunc;
    osExtraFuncBindPtr->osSocketsBindInfo.osSelectZeroSet =
        osFuncBindPtr->osSocketSelectInfo.osSelectZeroSetFunc;
    osExtraFuncBindPtr->osSocketsBindInfo.osSelectAddFdToSet =
        osFuncBindPtr->osSocketSelectInfo.osSelectAddFdToSetFunc;
    osExtraFuncBindPtr->osSocketsBindInfo.osSelectClearFdFromSet =
        osFuncBindPtr->osSocketSelectInfo.osSelectClearFdFromSetFunc;
    osExtraFuncBindPtr->osSocketsBindInfo.osSelectIsFdSet =
        osFuncBindPtr->osSocketSelectInfo.osSelectIsFdSetFunc;
    osExtraFuncBindPtr->osSocketsBindInfo.osSelectCopySet =
        osFuncBindPtr->osSocketSelectInfo.osSelectCopySetFunc;
    osExtraFuncBindPtr->osSocketsBindInfo.osSelect =
        osFuncBindPtr->osSocketSelectInfo.osSelectFunc;

    osExtraFuncBindPtr->osSocketsBindInfo.osSocketGetSocketFdSetSize =
        osFuncBindPtr->osSocketSelectInfo.osSocketGetSocketFdSetSizeFunc;

    osExtraFuncBindPtr->osTasksBindInfo.osTaskGetSelf = osTaskGetSelf;
    osExtraFuncBindPtr->osTasksBindInfo.osSetTaskPrior = osSetTaskPrior;

    osExtraFuncBindPtr->osPoolsBindInfo.gtPoolCreatePool = gtPoolCreatePool;
    osExtraFuncBindPtr->osPoolsBindInfo.gtPoolCreateDmaPool = gtPoolCreateDmaPool;
    osExtraFuncBindPtr->osPoolsBindInfo.gtPoolReCreatePool = gtPoolReCreatePool;
    osExtraFuncBindPtr->osPoolsBindInfo.gtPoolDeletePool = gtPoolDeletePool;
    osExtraFuncBindPtr->osPoolsBindInfo.gtPoolGetBuf = gtPoolGetBuf;
    osExtraFuncBindPtr->osPoolsBindInfo.gtPoolFreeBuf = gtPoolFreeBuf;
    osExtraFuncBindPtr->osPoolsBindInfo.gtPoolGetBufSize = gtPoolGetBufSize;
    osExtraFuncBindPtr->osPoolsBindInfo.gtPoolExpandPool = gtPoolExpandPool;
    osExtraFuncBindPtr->osPoolsBindInfo.gtPoolGetBufFreeCnt = gtPoolGetBufFreeCnt;
    osExtraFuncBindPtr->osPoolsBindInfo.gtPoolPrintStats = gtPoolPrintStats;


    extraFuncBindPtr->cpuEthernetPortBindInfo.cmdIsCpuEtherPortUsed =
        cpssHalInitializeCmdCpuEtherPortUsed;
    extraFuncBindPtr->eventsBindInfo.cmdCpssEventBind = cpssEventBind;
    extraFuncBindPtr->eventsBindInfo.cmdCpssEventSelect = cpssEventSelect;
    extraFuncBindPtr->eventsBindInfo.cmdCpssEventRecv = cpssEventRecv;
    extraFuncBindPtr->eventsBindInfo.cmdCpssEventDeviceMaskSet =
        cpssEventDeviceMaskSet;
    extraFuncBindPtr->appDbBindInfo.cmdAppDbEntryAdd =
        cpssHalInitializeCmdDbEntryAdd;
    extraFuncBindPtr->appDbBindInfo.cmdAppDbEntryGet =
        cpssHalInitializeCmdDbEntryGet;
    extraFuncBindPtr->appDbBindInfo.cmdAppDbDump = cpssHalInitializeCmdDbDump;
    extraFuncBindPtr->appDbBindInfo.cmdAppPpConfigGet=
        cpssHalInitializeCmdPpConfigGet;
    extraFuncBindPtr->appDbBindInfo.cmdInitSystem = cpssInitSystem;
    extraFuncBindPtr->appDbBindInfo.cmdInitSystemGet = cpssInitSystemGet;
    extraFuncBindPtr->appDbBindInfo.cmdResetSystem = cpssResetSystem;
    extraFuncBindPtr->appDbBindInfo.cmdInitConfi= confi2InitSystem;
    extraFuncBindPtr->appDbBindInfo.cmdAppShowBoardsList =
        cpssHalInitializeCmdShowBoardsList;
    extraFuncBindPtr->appDbBindInfo.cmdAppIsSystemInitialized =
        cpssHalInitializeCmdIsSystemInitialized;
    extraFuncBindPtr->appDbBindInfo.cmdAppPpConfigPrint=
        cpssHalInitializeCmdPpConfigPrint;
    extraFuncBindPtr->appDbBindInfo.cmdAppAllowProcessingOfAuqMessages =
        cpssHalInitializeCmdAllowProcessingOfAuqMessages;
    extraFuncBindPtr->appDbBindInfo.cmdAppTraceHwAccessEnable =
        prvcpssHalInitializeCmdTraceHwAccessEnable;
    extraFuncBindPtr->appDbBindInfo.cmdAppTraceHwAccessOutputModeSet =
        prvcpssHalInitializeCmdTraceHwAccessOutputModeSet;
    extraFuncBindPtr->appDbBindInfo.cmdAppStartHeapAllocCounter =
        osMemStartHeapAllocationCounter;
    extraFuncBindPtr->appDbBindInfo.cmdAppGetHeapAllocCounter =
        osMemGetHeapAllocationCounter;
    extraFuncBindPtr->appDbBindInfo.cmdAppEventFatalErrorEnable =
        prvcpssHalInitializeCmdEventFatalErrorEnable;
    extraFuncBindPtr->appDbBindInfo.cmdAppDemoEventsDataBaseGet =
        cpssHalInitializeCmdEventsDataBaseGet;
    return rc;
}

/*******************************************************************************
* cpssHalCmdShellEnable
*
* DESCRIPTION:
*
* INPUTS:
*       GT_BOOL enable
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*
*
*******************************************************************************/
#endif /*NO_MULTI_DEFINITION*/
GT_STATUS cpssHalCmdShellEnable(GT_BOOL enable)
{
    cmdShellEnable(enable);

    return GT_OK;
}
#endif
