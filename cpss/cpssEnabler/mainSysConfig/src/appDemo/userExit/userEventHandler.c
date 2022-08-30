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
* @file userEventHandler.c
*
* @brief This module defines the Application Demo requested event (polling) mode
* process creation and event handling.
*
* @version   50
********************************************************************************
*/

#if (defined CHX_FAMILY)
    #define DXCH_CODE
#endif /* (defined CHX_FAMILY) */
#if (defined PX_FAMILY)
    #define PX_CODE
#endif /* (defined PX_FAMILY) */

#include <gtOs/gtOsExc.h>
#include <gtOs/gtOsMsgQ.h>

#include <appDemo/os/appOs.h>
#include <appDemo/userExit/userEventHandler.h>
#include <appDemo/sysHwConfig/appDemoDb.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/events/private/prvCpssGenEvReq.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/common/port/private/prvCpssPortPcsCfg.h>

#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxExMxInterrupts.h>

#include <gtExtDrv/drivers/gtIntDrv.h>
#include <extUtils/rxEventHandler/rxEventHandler.h>

#include <cmdShell/cmdDb/cmdBase.h>

#ifdef DXCH_CODE
#include <extUtils/auEventHandler/auEventHandler.h>
#include <cpss/dxCh/dxChxGen/port/macCtrl/prvCpssDxChGEMacCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortAp.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/cpssDxChBrgFdbManager.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManager_utils.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManager_db.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqQfc.h>
#endif /*DXCH_CODE*/

#ifdef PX_CODE
#include <cpss/px/diag/cpssPxDiagDataIntegrity.h>
#include <appDemo/userExit/px/appDemoPxDiagDataIntegrityAux.h>
#include <cpss/px/port/cpssPxPortManager.h>
#endif

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#ifdef SHARED_MEMORY
extern GT_BOOL multiProcessAppDemo;
#endif

#define NEW_PORT_INIT_SEQ

#define MAX_GRACEFUL_EXIT_RETRY     10


/* debug flag to open trace of events */
extern GT_U32 wrapCpssTraceEvents;
/* indication the we do 'system reset' */
GT_U32   eventRequestDrvnModeReset = 0;
#define FREE_TASK_CNS   0xFFFFFFFF
GT_UINTPTR rxEventHanderHnd = 0;
GT_UINTPTR auEventHandlerHnd = 0;

#ifdef DXCH_CODE
/* Set of two FEC counters allocated per virtual lane */
FEC_COUNTERS_STC *uniFecEventCounters[MAX_NUM_DEVICES];

#endif

/* global variables used in single Rx task mode */
/* is single Rx task mode enabled */
static GT_BOOL singleTaskRxEnable = GT_FALSE;
/* is random distribution of packets to msgQs enabled */
static GT_BOOL singleTaskRxRandomEnable = GT_FALSE;
/* number of treat Rx tasks */
static GT_U32  treatTasksNum = 2;
static CPSS_OS_MSGQ_ID singleRxMsgQIds[8];
CPSS_OS_MUTEX rxMutex;
#ifdef PTP_PLUGIN_SUPPORT
static APP_DEMO_LINK_CHNG_FUNC linkChngFuncPtr = NULL;
#endif
/* treat RX task id array for reset - size should be equal to "treatTasksNum" */
GT_TASK treatRxEventHandlerTid[2];

/* task id array for reset */
GT_TASK eventHandlerTidArr[APP_DEMO_PROCESS_NUMBER];

#if defined(CHX_FAMILY) && defined(IMPL_GALTIS)
#include <galtisAgent/wrapCpss/dxCh/diag/wrapCpssDxChDiagDataIntegrity.h>
#endif /* defined(CHX_FAMILY) && defined(IMPL_GALTIS) */

/* The next define can be uncommented for debug use only !! */
/* #define APPDEMO_REQ_MODE_DEBUG */

#ifdef APPDEMO_REQ_MODE_DEBUG
  static GT_CHAR * uniEvName[CPSS_UNI_EVENT_COUNT_E] = {UNI_EV_NAME};
  #ifdef ASIC_SIMULATION
    #define DBG_LOG(x)  osPrintf x
  #else
    extern int  logMsg (char *fmt, int arg1, int arg2, int arg3,
                        int arg4, int arg5, int arg6);
    #define DBG_LOG(x)  osPrintf x
  #endif
  #define DBG_INFO(x)   osPrintf x
#else
  #define DBG_INFO(x)
  #define DBG_LOG(x)
#endif

/* Notification callback for catching events */
EVENT_NOTIFY_FUNC *notifyEventArrivedFunc = NULL;
/* indication that task created */
static GT_U32   taskCreated = 0;
/* Local routines */
static unsigned __TASKCONV appDemoEvHndlr(GT_VOID * param);
static unsigned __TASKCONV appDemoSingleRxEvHndlr(GT_VOID * param);
static unsigned __TASKCONV appDemoSingleRxTreatTask(GT_VOID * param);
#ifdef GM_USED
static GT_TASK gm_appDemoEmulateLinkChangeTask_Tid = (GT_TASK)0;
static unsigned __TASKCONV gm_appDemoEmulateLinkChangeTask(GT_VOID * param);
#endif /*GM_USED*/

GT_U32  disableAutoPortMgr;
GT_BOOL portMgr;

/* Local Defines */
#define EV_HANDLER_MAX_PRIO 200

#define MAX_UBURST_EVENTS_CHUNK 64


#ifdef DXCH_CODE
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/vnt/cpssDxChVnt.h>
#include <appDemo/userExit/dxCh/appDemoDxChEventHandle.h>

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagDataIntegrity.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>

#endif /*DXCH_CODE*/

#ifdef PX_CODE
#include <appDemo/userExit/px/appDemoPxEventHandle.h>
extern GT_STATUS   pipe_linkChange(
    IN GT_U8 devNum,
    IN GT_U32 phyPortNum,
    IN GT_BOOL printLinkChangeEnabled
);

#include <cpss/px/port/cpssPxPortMapping.h>
#include <cpss/px/port/cpssPxPortCtrl.h>
#include <cpss/px/port/cpssPxPortAp.h>

#endif /*PX_CODE*/

/* Roundup Number */
#define ROUNDUP_DIV_MAC(_number , _divider)             \
        (((_number) + ((_divider)-1)) / (_divider))

extern GT_STATUS AN_WA_Task_SyncChangeEventBuildAndSend
(
    GT_U8    devNum,
    GT_U32   portMacNum
);

extern GT_STATUS AN_WA_Task_AnCompleteEventBuildAndSend
(
    GT_U8    devNum,
    GT_U32   portMacNum
);


static GT_STATUS appDemoEnPpEvTreat
(
    GT_U8                   devNum,
    GT_U32                  uniEv,
    GT_U32                  evExtData
);
extern GT_STATUS appDemoGenNetRxPktHandle(IN GT_U8 devNum,
                                          IN GT_U8 queueIdx);
#ifdef CHX_FAMILY
extern GT_STATUS cpssEnGenAuMsgHandle(IN GT_UINTPTR auEvHandler,
                                      IN GT_U8  devNum,
                                      IN GT_U32 evExtData);
#else /*CHX_FAMILY*/
/* STUBS */
static GT_STATUS cpssEnGenAuMsgHandle(IN GT_UINTPTR auEvHandler,
                                      IN GT_U8  devNum,
                                      IN GT_U32 evExtData)
{
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(auEvHandler);
    GT_UNUSED_PARAM(evExtData);
    return GT_OK;
}
#endif /*CHX_FAMILY*/

GT_STATUS appDemoPortInitSeqLinkStatusChangedStage
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum
);

GT_STATUS appDemoPortInitSeqPortStatusChangeSignal
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    GT_U32 uniEvent
);

/**
* @struct EV_HNDLR_PARAM
*/
typedef struct{

    /** The process unified event handle (got from appDemoCpssEventBind). */
    GT_UINTPTR evHndl;

    /** The process number */
    GT_U32 hndlrIndex;

} EV_HNDLR_PARAM;


APP_DEMO_CALL_BACK_ON_REMOTE_PORTS_FUNC appDemoCallBackOnRemotePortsFunc = NULL;

static GT_STATUS prvUniEvMaskAllSet
(
    IN  CPSS_UNI_EV_CAUSE_ENT       cpssUniEventArr[],
    IN  GT_U32                      arrLength,
    IN  CPSS_UNI_EV_CAUSE_ENT       uniEvSkipArr[],
    IN  GT_U32                      skipArrLength,
    IN  CPSS_EVENT_MASK_SET_ENT     operation
);

static GT_STATUS prvUniEvMaskAllDeviceSet
(
    IN  GT_U8                       devIdx,
    IN  CPSS_UNI_EV_CAUSE_ENT       cpssUniEventArr[],
    IN  GT_U32                      arrLength,
    IN  CPSS_UNI_EV_CAUSE_ENT       uniEvSkipArr[],
    IN  GT_U32                      skipArrLength,
    IN  CPSS_EVENT_MASK_SET_ENT     operation
);

static GT_STATUS cpssEnRxPacketGet
(
    IN RX_EV_HANDLER_EV_TYPE_ENT evType,
    IN GT_U8                devNum,
    IN GT_U8                queue
);
static GT_STATUS appDemoGetMaxPackets
(
    IN GT_U8                devNum,
    IN GT_U8                queue,
    IN GT_U32               maxPacket
);


/* Event Counter increment function */
EVENT_COUNTER_INCREMENT_FUNC *eventIncrementFunc = NULL;


#if defined(DXCH_CODE)
#ifdef IMPL_GALTIS
/* Event Counter increment function. Implemented in Galtis*/
DXCH_DATA_INTEGRITY_EVENT_COUNTER_INCREMENT_FUNC *dxChDataIntegrityEventIncrementFunc = NULL;
#else /* STUB */
GT_VOIDFUNCPTR dxChDataIntegrityEventIncrementFunc = NULL;
#endif
#endif /* defined(DXCH_CODE) && defined(IMPL_GALTIS) */

#if defined(PX_CODE)
/* Event Counter increment function. */
PX_DATA_INTEGRITY_EVENT_COUNTER_INCREMENT_FUNC *pxDataIntegrityEventIncrementFunc = NULL;
#endif /* defined(PX_CODE) */

#ifdef DXCH_CODE
extern GT_U32   trainingTrace;/* option to disable the print in runtime*/
#define TRAINING_DBG_PRINT_MAC(x) if(trainingTrace) cpssOsPrintSync x
#ifndef ASIC_SIMULATION
CPSS_OS_SIG_SEM tuneSmid = 0; /* semaphore to manage serdes tuning selector task */
CPSS_PORTS_BMP_STC todoTuneBmp; /* bitmap of ports to run tuning on */
extern unsigned __TASKCONV appDemoDxChLion2SerdesAutoTuningStartTask
(
    GT_VOID * param
);
extern unsigned __TASKCONV lion2WaTask
(
    GT_VOID * param
);
extern CPSS_OS_SIG_SEM waTSmid;
extern CPSS_PORTS_BMP_STC todoWaBmp; /* bitmap of ports to run WA's on */
extern CPSS_PORTS_BMP_STC todoRxTrainingCompleteBmp; /* bitmap of ports to check is Rx training complete */

extern GT_U32 locksTimeoutArray[CPSS_MAX_PORTS_NUM_CNS];
#endif /* ndef ASIC_SIMULATION */
#endif /* DXCH_CODE */


/* flag to enable the printings of 'link change' --
    by default not enabled because it may cause RDE tests to fail due to the
    printings , tests that cause link change.
*/
#ifdef ASIC_SIMULATION
static GT_U32   printLinkChangeEnabled = 1;
#else  /*ASIC_SIMULATION*/
static GT_U32   printLinkChangeEnabled = 0;
#endif /*ASIC_SIMULATION*/

static GT_U32   printEeeInterruptInfo = 0;/* WM not supports this interrupt */


/* flag that state the tasks may process events that relate to AUQ messages.
    this flag allow us to stop processing those messages , by that the AUQ may
    be full, or check AU storm prevention, and other.

    use function appDemoAllowProcessingOfAuqMessages(...) to set this flag
*/
static GT_BOOL  allowProcessingOfAuqMessages = GT_TRUE;

/* fatal error handling type */
static CPSS_ENABLER_FATAL_ERROR_TYPE prvAppDemoFatalErrorType = CPSS_ENABLER_FATAL_ERROR_EXECUTE_TYPE_E;

/* flag meaning appDemoEnPpEvTreat will catch <Signal Detect triggered> interrupt.
Warning: <Signal Detect triggered> is new interrupt in XCAT2 B1, if enabled using
    cpssDxChPortGePrbsIntReplaceEnableSet it overrides old but not obsolete
    <QSGMII PRBS error> interrupt, so in appDemoEnPpEvTreat it's treated as
    CPSS_PP_PORT_PRBS_ERROR_QSGMII_E case */
GT_BOOL catchSignalDetectInterrupt = GT_FALSE;

/* flag to print info about Data Integrity event */
static GT_BOOL prvAppDemoDataIntegrityEventDump = GT_FALSE;

/* flag to print info about MACSec events */
static GT_BOOL prvAppDemoMacSecEventsDump = GT_FALSE;

static GT_BOOL prvAppDemoUniEventEmulateLegacyCounters = GT_TRUE;

/**
* @internal appDemoPrintLinkChangeFlagSet function
* @endinternal
*
* @brief   function to allow set the flag of : printLinkChangeEnabled
*
* @param[in] enable                   - enable/disable the printings of 'link change'
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoPrintLinkChangeFlagSet
(
    IN GT_U32   enable
)
{
    printLinkChangeEnabled = enable;
    return GT_OK;
}

/**
* @internal appDemoGenEventLegacyCounterEnable function
* @endinternal
*
* @brief   Emulates old counters functionality. In this mode there will be no dynamic allocation per event/extended data, and
*          the counters will be incremented per event type.
*
* @param[in] enable   - enable/disable 'old' counting mode
*                       GT_TRUE     - emulate legacy mode
*                       GT_FALSE    - the new count mode is active
*
*/
GT_VOID appDemoGenEventLegacyCounterEnable
(
    IN GT_BOOL   enable
)
{
    prvAppDemoUniEventEmulateLegacyCounters = enable;
}

/**
* @internal appDemoPrintLinkChangeFlagGet function
* @endinternal
*
* @brief   function to get the flag of : printLinkChangeEnabled
*
* @retval printLinkChangeEnabled - do we print link status change info
*/
GT_BOOL appDemoPrintLinkChangeFlagGet(void)
{
    return printLinkChangeEnabled;
}

/**
* @internal appDemoPrintPortEeeInterruptInfoSet function
* @endinternal
*
* @brief   function to allow set the flag of : printEeeInterruptInfo
*
* @param[in] enable                   - enable/disable the printings of ports 'EEE interrupts info'
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoPrintPortEeeInterruptInfoSet
(
    IN GT_U32   enable
)
{
    printEeeInterruptInfo = enable;
    return GT_OK;
}

#ifdef DXCH_CODE

static GT_U32   debug_print_fdb_manager_learning = 0;
GT_STATUS debug_print_fdb_manager_learning_enable_set(IN GT_U32    enable)
{
    debug_print_fdb_manager_learning = enable;
    return GT_OK;
}

static GT_U32 appDemoFdbManagerLearningTaskNeedAuqMessagesFromCpss = 0; /* set to 1 to work like SAI+Sonic */
GT_STATUS appDemoFdbManagerLearningTaskInfo
(
    IN GT_BOOL  needAuqMessagesFromCpss
)
{
    appDemoFdbManagerLearningTaskNeedAuqMessagesFromCpss = needAuqMessagesFromCpss;
    return GT_OK;
}

/**
* @internal cpssEnFdbManagerAuMsgHandle function
* @endinternal
*
* @brief   This routine gets and handles the Address Update messages in FDB Manager Mode.
*
* @param[in] fdbManagerId          - the FDB Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] scanParamPtr          - the learning scan attributes.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssEnFdbManagerAuMsgHandle
(
    GT_U32                                              fdbManagerId,
    CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_SCAN_PARAMS_STC  *scanParamPtr
)
{
    static CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC *entriesLearningPtr = NULL;
    static GT_U32                                            entriesLearningAllocSize = 0;
    GT_U32                                                  entriesLearningNum;
    GT_U32                                                  auIndex;
    GT_STATUS                   rc = GT_OK;

    FDB_MANAGER_ID_CHECK(fdbManagerId);

    if(appDemoFdbManagerLearningTaskNeedAuqMessagesFromCpss == GT_TRUE)
    {
        GT_U32 numEntries;

        numEntries = PRV_SHARED_FDB_MANAGER_DB_VAR_GET(prvCpssDxChFdbManagerDbArr)[fdbManagerId]->capacityInfo.maxEntriesPerLearningScan;

        if(entriesLearningPtr == NULL)
        {
            /*need first allocation*/
            entriesLearningAllocSize = numEntries > 256 ? numEntries : 256;
            entriesLearningPtr = cpssOsMalloc(sizeof(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC) * entriesLearningAllocSize);
        }
        else
        if(entriesLearningAllocSize < numEntries)
        {
            /*need larger allocation*/

            entriesLearningAllocSize = numEntries;
            entriesLearningPtr = cpssOsRealloc(entriesLearningPtr,sizeof(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC) * entriesLearningAllocSize);
        }
        else
        {
            /* we already have entriesLearningPtr with proper size */
        }

        if(entriesLearningPtr == NULL)
        {
            return GT_OUT_OF_CPU_MEM;
        }
    }


    while(rc != GT_NO_MORE)
    {
        if(eventRequestDrvnModeReset)
        {
            break;
        }

        if(appDemoFdbManagerLearningTaskNeedAuqMessagesFromCpss == GT_FALSE)
        {
            /* support mode that the application not need any info about the learned AUQ messages */
            rc = cpssDxChBrgFdbManagerLearningScan(fdbManagerId, scanParamPtr, NULL, NULL);
        }
        else
        {
            /* support mode that the application MUST know about the learned AUQ messages */
            /* like in SAI that send those messages to the Sonic                          */
            rc = cpssDxChBrgFdbManagerLearningScan(fdbManagerId, scanParamPtr, entriesLearningPtr, &entriesLearningNum);
            if(debug_print_fdb_manager_learning && entriesLearningNum)
            {
                cpssOsPrintf("learning of [%d] entries \n",entriesLearningNum);
                if(rc == GT_OK || rc == GT_NO_MORE)
                {
                    for (auIndex = 0; auIndex < entriesLearningNum; auIndex++)
                    {
                        /* dummy print ... just to allow access to the array */
                        cpssOsPrintf("learning type [%d] \n",entriesLearningPtr[auIndex].updateType);
                    }
                }
            }
        }
    }
    return rc;
}
#endif

/**
* @internal appDemoAllowProcessingOfAuqMessages function
* @endinternal
*
* @brief   function to allow set the flag of : allowProcessingOfAuqMessages
*
* @param[in] enable                   - enable/disable the processing of the AUQ messages
*
* @retval GT_OK                    - on success
*
* @note flag that state the tasks may process events that relate to AUQ messages.
*       this flag allow us to stop processing those messages , by that the AUQ may
*       be full, or check AU storm prevention, and other.
*
*/
GT_STATUS   appDemoAllowProcessingOfAuqMessages
(
    GT_BOOL     enable
)
{
    GT_U8   dev;

    if(enable == GT_TRUE && allowProcessingOfAuqMessages == GT_FALSE)
    {
        /* move from no learn to learn */
        allowProcessingOfAuqMessages = GT_TRUE;/*allow appDemoEnPpEvTreat to learn !!! */
        /* start processing the messages stuck in the queue */

        for(dev = SYSTEM_DEV_NUM_MAC(0); dev < SYSTEM_DEV_NUM_MAC(appDemoPpConfigDevAmount); dev++)
        {
            if(appDemoPpConfigList[dev].valid == GT_FALSE)
            {
                continue;
            }

            /* call appDemoEnPpEvTreat instead of cpssEnGenAuMsgHandle ,
               to support FDB-manager , so it will also can support 'burst' and 'no treatment' */
            appDemoEnPpEvTreat(dev,CPSS_PP_EB_AUQ_PENDING_E,0);/*unused param*/
        }
    }

    allowProcessingOfAuqMessages = enable;

    return GT_OK;
}

static CPSS_UNI_EV_CAUSE_ENT evHndlr0CauseDefaultArr[] = APP_DEMO_P0_UNI_EV_DEFAULT;
static CPSS_UNI_EV_CAUSE_ENT evHndlr1CauseDefaultArr[] = APP_DEMO_P1_UNI_EV_DEFAULT;
static CPSS_UNI_EV_CAUSE_ENT evHndlr2CauseDefaultArr[] = APP_DEMO_P2_UNI_EV_DEFAULT;
static CPSS_UNI_EV_CAUSE_ENT evHndlr3CauseDefaultArr[] = APP_DEMO_P3_UNI_EV_DEFAULT;
static CPSS_UNI_EV_CAUSE_ENT evHndlr4CauseDefaultArr[] = APP_DEMO_P4_UNI_EV_DEFAULT;
static CPSS_UNI_EV_CAUSE_ENT evHndlr5CauseDefaultArr[] = APP_DEMO_P5_UNI_EV_DEFAULT;
static CPSS_UNI_EV_CAUSE_ENT evHndlr6CauseDefaultArr[] = APP_DEMO_P6_UNI_EV_DEFAULT;
static CPSS_UNI_EV_CAUSE_ENT evHndlr7CauseDefaultArr[] = APP_DEMO_P7_UNI_EV_DEFAULT;
static CPSS_UNI_EV_CAUSE_ENT evHndlr8CauseDefaultArr[] = APP_DEMO_P8_UNI_EV_DEFAULT;
static CPSS_UNI_EV_CAUSE_ENT evHndlr9CauseDefaultArr[] = APP_DEMO_P9_UNI_EV_DEFAULT;

static GT_U32 evHndlr0CauseDefaultArrSize = sizeof(evHndlr0CauseDefaultArr)/sizeof(evHndlr0CauseDefaultArr[0]);
static GT_U32 evHndlr1CauseDefaultArrSize = sizeof(evHndlr1CauseDefaultArr)/sizeof(evHndlr1CauseDefaultArr[0]);
static GT_U32 evHndlr2CauseDefaultArrSize = sizeof(evHndlr2CauseDefaultArr)/sizeof(evHndlr2CauseDefaultArr[0]);
static GT_U32 evHndlr3CauseDefaultArrSize = sizeof(evHndlr3CauseDefaultArr)/sizeof(evHndlr3CauseDefaultArr[0]);
static GT_U32 evHndlr4CauseDefaultArrSize = sizeof(evHndlr4CauseDefaultArr)/sizeof(evHndlr4CauseDefaultArr[0]);
static GT_U32 evHndlr5CauseDefaultArrSize = sizeof(evHndlr5CauseDefaultArr)/sizeof(evHndlr5CauseDefaultArr[0]);
static GT_U32 evHndlr6CauseDefaultArrSize = sizeof(evHndlr6CauseDefaultArr)/sizeof(evHndlr6CauseDefaultArr[0]);
static GT_U32 evHndlr7CauseDefaultArrSize = sizeof(evHndlr7CauseDefaultArr)/sizeof(evHndlr7CauseDefaultArr[0]);
static GT_U32 evHndlr8CauseDefaultArrSize = sizeof(evHndlr8CauseDefaultArr)/sizeof(evHndlr8CauseDefaultArr[0]);
static GT_U32 evHndlr9CauseDefaultArrSize = sizeof(evHndlr9CauseDefaultArr)/sizeof(evHndlr9CauseDefaultArr[0]);


static CPSS_UNI_EV_CAUSE_ENT evHndlr6CauseSingleTaskRxArr[] = APP_DEMO_P6_UNI_EV_SINGLE_RX_TASK_MODE;
static GT_U32 evHndlr6CauseSingleTaskRxArrSize = sizeof(evHndlr6CauseSingleTaskRxArr)/sizeof(evHndlr6CauseSingleTaskRxArr[0]);

static CPSS_UNI_EV_CAUSE_ENT evHndlr7CauseNoAuqPendingArr[] = APP_DEMO_P7_UNI_EV_NO_AUQ_PENDING;
static GT_U32 evHndlr7CauseNoAuqPendingArrSize = sizeof(evHndlr7CauseNoAuqPendingArr)/sizeof(evHndlr7CauseNoAuqPendingArr[0]);

static CPSS_UNI_EV_CAUSE_ENT *evHndlrCauseAllArr[APP_DEMO_PROCESS_NUMBER];
static GT_U32 evHndlrCauseAllArrSize[APP_DEMO_PROCESS_NUMBER];

/* Unified events array of events that should not be unmask when calling unmask-all API */
static CPSS_UNI_EV_CAUSE_ENT uniEvSkipArr[] = APP_DEMO_SKIP_ARR;
static GT_U32  uniEvSkipArrLength = sizeof(uniEvSkipArr)/sizeof(uniEvSkipArr[0]);

/* Note - should be only static (global), cannot resides in stack! */
static EV_HNDLR_PARAM taskParamArr[APP_DEMO_PROCESS_NUMBER];

#ifdef DXCH_CODE

#define APPDEMO_LINK_TRACE_DB_SIZE 100000

typedef struct
{
    GT_BOOL valid;
    GT_U32 portNum;
    GT_U32 portMacNum;
    GT_U32 timeStampSec;
    GT_U32 timeStampNanoSec;
    GT_U32 eventType;
    GT_U32 eventValue;

}APPDEMO_LINK_TRACE_DB_STC;

static APPDEMO_LINK_TRACE_DB_STC linkTraceDB[APPDEMO_LINK_TRACE_DB_SIZE] = {{GT_FALSE, 0, 0, 0, 0, 0, 0}};
static GT_U32 currentEventIndex = 0;
static GT_BOOL linkTraceFeatureEnabled = GT_FALSE;
static GT_U32 secondsStart = 0,nanoSecondsStart = 0;

GT_STATUS dbgPrintLinkTraceEnable(GT_BOOL enable)
{
    linkTraceFeatureEnabled = enable;
    cpssOsTimeRT(&secondsStart,&nanoSecondsStart);
    return GT_OK;
}

GT_STATUS dbgClearLinkTraceDb()
{
    cpssOsMemSet(linkTraceDB, 0, sizeof(APPDEMO_LINK_TRACE_DB_STC)*APPDEMO_LINK_TRACE_DB_SIZE);
    secondsStart = 0;
    nanoSecondsStart = 0;
    currentEventIndex = 0;
    return GT_OK;
}

GT_STATUS dbgPrintLinkTraceDb()
{
    GT_U32 i;

    for(i = 0; i < APPDEMO_LINK_TRACE_DB_SIZE; i++)
    {
        if(linkTraceDB[i].valid)
        {
            cpssOsPrintf("(%d) port = %d, mac = %d, timeSec = %d, timeNanoSec = %09d, ",
                         i,
                         linkTraceDB[i].portNum,
                         linkTraceDB[i].portMacNum,
                         linkTraceDB[i].timeStampSec,
                         linkTraceDB[i].timeStampNanoSec);
            cpssOsPrintf("eventType = %s, eventValue = %s\n",
                         (linkTraceDB[i].eventType == 44) ? "PORT_LINK_STATUS_CHANGED" : "PORT_PCS_ALIGN_LOCK_LOST",
                         (linkTraceDB[i].eventValue == 0) ? "DOWN" : "UP");
        }
    }

    return GT_OK;
}

/* Table name for print purpouse */
static GT_U8 strNameBuffer[120] = "";
static GT_U8 strNameBuffer1[120] = "";

#ifdef CPSS_LOG_ENABLE
extern PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_CPSS_DXCH_TABLE_ENT[];
extern GT_U32 prvCpssLogEnum_size_CPSS_DXCH_TABLE_ENT;
extern PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT[];
extern GT_U32 prvCpssLogEnum_size_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT;

#define PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(tableType, tableEnum, strBuffer) \
    PRV_CPSS_LOG_ENUM_STRING_GET_MAC(tableType, tableEnum, strBuffer)

#else

#define PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(tableType, tableEnum, strBuffer)
#endif


/*******************************************************************************
* appDemoDxChLion2DataIntegrityScan
*
* DESCRIPTION:
*       This routine treats Data Integrity events.
*
* INPUTS:
*       devNum - device number
*       evExtData - event external data
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
#define APP_DEMO_DXCH_LION2_DATA_INTEGRITY_EVENTS_NUM_CNS 40

GT_STATUS appDemoDxChLion2DataIntegrityScan
(
    GT_U8   devNum,
    GT_U32  evExtData
)
{
    GT_STATUS   rc;                 /* return code */
    GT_U32      eventsNum = APP_DEMO_DXCH_LION2_DATA_INTEGRITY_EVENTS_NUM_CNS;     /* initial number of ECC/parity events */
    CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[APP_DEMO_DXCH_LION2_DATA_INTEGRITY_EVENTS_NUM_CNS];
    GT_BOOL     isScanFinished = GT_FALSE;     /* events scan finish status */

#ifdef IMPL_GALTIS
    GT_U32 i; /*loop iterator*/
#endif /* IMPL_GALTIS */


    while(isScanFinished != GT_TRUE)
    {
        rc = cpssDxChDiagDataIntegrityEventsGet(devNum, evExtData, &eventsNum, eventsArr, &isScanFinished);
        if(GT_OK != rc)
        {
            return rc;
        }

        if (prvAppDemoDataIntegrityEventDump)
        {
            GT_U32 ii;
            GT_CHAR *eventTypeName[] = {"parity", "single ECC", "multiple ECC", "single and multiple ECC"};
            GT_CHAR *memUsageTypeName[] = {"unknow", "configuration", "data", "state"};
            GT_CHAR *correctionTypeName[] = {"unknow", "none", "drop or resend", "HW correction", "SW correction", "reboot", "scrub"};

            cpssOsPrintf("\n Data Integrity Event, device %d evExtData 0X%X \n", devNum, evExtData);

            for (ii = 0; ii < eventsNum; ii++)
            {
                cpssOsPrintf("== Event Type: ");
                if (eventsArr[ii].eventsType >= sizeof(eventTypeName)/sizeof(eventTypeName[0]))
                {
                    cpssOsPrintf("%d", eventsArr[ii].eventsType);
                }
                else
                {
                    cpssOsPrintf("%s", eventTypeName[eventsArr[ii].eventsType]);
                }

                cpssOsPrintf(" Usage: ");

                if (eventsArr[ii].memoryUseType >= sizeof(memUsageTypeName)/sizeof(memUsageTypeName[0]))
                {
                    cpssOsPrintf("%d", eventsArr[ii].memoryUseType);
                }
                else
                {
                    cpssOsPrintf("%s", memUsageTypeName[eventsArr[ii].memoryUseType]);
                }

                cpssOsPrintf(" Correction Method: ");

                if (eventsArr[ii].correctionMethod >= sizeof(correctionTypeName)/sizeof(correctionTypeName[0]))
                {
                    cpssOsPrintf("%d\n", eventsArr[ii].correctionMethod);
                }
                else
                {
                    cpssOsPrintf("%s\n", correctionTypeName[eventsArr[ii].correctionMethod]);
                }

                if (eventsArr[ii].location.isTcamInfoValid)
                {
                    /* TCAM event */
                    cpssOsPrintf("  TCAM entry %s Port Group BMP 0X%X rule index %d\n",
                                 (eventsArr[ii].location.tcamMemLocation.arrayType == CPSS_DIAG_DATA_INTEGRITY_TCAM_ARRAY_TYPE_X_E) ? "X": "Y",
                                  eventsArr[ii].location.portGroupsBmp,
                                  eventsArr[ii].location.tcamMemLocation.ruleIndex);
                }
                else
                {
                    /* RAM event */
                    PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(eventsArr[ii].location.hwEntryInfo.hwTableType,
                                                                CPSS_DXCH_TABLE_ENT, strNameBuffer);
                    PRV_CPSS_DATA_INTEGRITY_ENUM_STRING_GET_MAC(eventsArr[ii].location.ramEntryInfo.memType,
                                                                CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT, strNameBuffer1);
                    cpssOsPrintf("  RAM HW entry index:  %d RAM Row: %d Port Group BMP: 0X%X\n",   eventsArr[ii].location.hwEntryInfo.hwTableEntryIndex,
                                                                              eventsArr[ii].location.ramEntryInfo.ramRow,
                                                                              eventsArr[ii].location.portGroupsBmp);
                    cpssOsPrintf("   HW table type     :  %s\n",   strNameBuffer);
                    cpssOsPrintf("   Memory table type :  %s\n",   strNameBuffer1);
                    cpssOsPrintf("     pipe %d, client %d, mem %d, instance type %d, index %d\n",
                                     eventsArr[ii].location.ramEntryInfo.memLocation.dfxPipeId,
                                     eventsArr[ii].location.ramEntryInfo.memLocation.dfxClientId,
                                     eventsArr[ii].location.ramEntryInfo.memLocation.dfxMemoryId,
                                     eventsArr[ii].location.ramEntryInfo.memLocation.dfxInstance.dfxInstanceType,
                                     eventsArr[ii].location.ramEntryInfo.memLocation.dfxInstance.dfxInstanceIndex);

                }
            }
        }
#ifdef IMPL_GALTIS
        if(dxChDataIntegrityEventIncrementFunc != NULL)
        {
            for(i = 0; i < eventsNum; i++)
            {
                /* counting the event */
                rc = dxChDataIntegrityEventIncrementFunc(devNum, &eventsArr[i]);
                if(GT_OK != rc)
                {
                    cpssOsPrintf("appDemoDxChLion2DataIntegrityScan: failed to increment event counter for memType %d\r\n", eventsArr[i].location.ramEntryInfo.memType);
                    return rc;
                }
            }
        }
#endif /* IMPL_GALTIS */
    }

    return GT_OK;
}
#endif /* DXCH_CODE */

#ifdef PX_CODE
/*******************************************************************************
* appDemoPxDataIntegrityEventsScan
*
* DESCRIPTION:
*       This routine treats Data Integrity events.
*
* INPUTS:
*       devNum - device number
*       evExtData - event external data
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
#define APP_DEMO_PX_DATA_INTEGRITY_EVENTS_NUM_CNS 20

GT_STATUS appDemoPxDataIntegrityEventsScan
(
    GT_U8   devNum,
    GT_U32  evExtData
)
{
    GT_STATUS   rc;                 /* return code */
    GT_U32      eventsNum = APP_DEMO_PX_DATA_INTEGRITY_EVENTS_NUM_CNS;     /* initial number of ECC/parity events */
    CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC     eventsArr[APP_DEMO_PX_DATA_INTEGRITY_EVENTS_NUM_CNS];
    GT_BOOL     isScanFinished = GT_FALSE;     /* events scan finish status */
    GT_U32 i; /*loop iterator*/

    while(isScanFinished != GT_TRUE)
    {
        eventsNum = APP_DEMO_PX_DATA_INTEGRITY_EVENTS_NUM_CNS;
        rc = cpssPxDiagDataIntegrityEventsGet(devNum, evExtData, &eventsNum, eventsArr, &isScanFinished);
        if(GT_OK != rc)
        {
            return rc;
        }


        if(pxDataIntegrityEventIncrementFunc != NULL)
        {
            for(i = 0; i < eventsNum; i++)
            {
                /* counting the event */
                rc = pxDataIntegrityEventIncrementFunc(devNum, &eventsArr[i]);
                if(GT_OK != rc)
                {
                    cpssOsPrintf("appDemoPxDataIntegrityScan: failed to increment event counter for memType %d\r\n", eventsArr[i].location.ramEntryInfo.memType);
                    return rc;
                }
            }
        }
    }

    return GT_OK;
}
#endif /* PX_CODE */



/**
* @internal appDemoDeviceEventHandlerPreInit function
* @endinternal
*
* @brief   Pre init for the event handler tasks
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoDeviceEventHandlerPreInit
(
    IN GT_U8 dev
)
{
    GT_STATUS rc;

    /* Reset Event Counter DB */
    appUtilsEventCounters[dev] = NULL;

    rc = cpssAppUtilsEventHandlerPreInit(dev);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Get event increment. */
    eventIncrementFunc = cpssAppUtilsGenEventCounterIncrement;

    /* just trigger to allocate the needed memory for the device (during initialization)
       and not to wait till the first event.

       because the 'Enhanced UT' will check for memory leakage , and if first
       event will come only after the test started , then the test will fail .
    */
        /* don't care about the return value */
    /* counting the event */
    (GT_VOID)eventIncrementFunc(dev, 0xFFFFFFFF, 0);

    return GT_OK;
}

/**
* @internal appDemoEventHandlerPreInit function
* @endinternal
*
* @brief   Pre init for the event handler tasks
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoEventHandlerPreInit
(
    IN GT_VOID
)
{
    GT_STATUS rc;
    GT_U8   dev;

    if(appDemoSysConfig.appDemoActiveDeviceBmp &&
       eventIncrementFunc == cpssAppUtilsGenEventCounterIncrement)
    {
        /* already done on 'first part' , so skip global settings ,
           do only per device
        */
    }
    else
    {
        /* Get event increment. */
        eventIncrementFunc = cpssAppUtilsGenEventCounterIncrement;

        /* Reset Event Counter DB */
        cmdOsMemSet(appUtilsEventCounters, 0, sizeof(appUtilsEventCounters));
#ifdef DXCH_CODE
        /* Reset Fec Event Counter DB */
        cmdOsMemSet(uniFecEventCounters, 0, sizeof(uniFecEventCounters));
#endif
    }
    /* just trigger to allocate the needed memory for the device (during initialization)
       and not to wait till the first event.

       because the 'Enhanced UT' will check for memory leakage , and if first
       event will come only after the test started , then the test will fail .
    */

    for(dev = SYSTEM_DEV_NUM_MAC(0); dev < SYSTEM_DEV_NUM_MAC(appDemoPpConfigDevAmount); dev++)
    {
        if(appDemoPpConfigList[dev].valid == GT_FALSE)
        {
            continue;
        }

        SYSTEM_SKIP_NON_ACTIVE_DEV(dev);

        rc = cpssAppUtilsEventHandlerPreInit(appDemoPpConfigList[dev].devNum);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* don't care about the return value */
        (GT_VOID)eventIncrementFunc(appDemoPpConfigList[dev].devNum, 0xFFFFFFFF, 0);
    }

    return GT_OK;
}

 /**
* @internal appDemoDeviceEventRequestDrvnModeInit function
* @endinternal
*
* @brief   This routine spawns the App Demo event handlers.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoDeviceEventRequestDrvnModeInit
(
    IN GT_U8 devIdx
)
{
    GT_STATUS rc;              /* The returned code            */
    GT_U32    i;               /* Iterator                     */
    GT_CHAR   name[30];        /* The task/msgQ name           */
    GT_U32    value;
    unsigned (__TASKCONV *start_addr)(GT_VOID*);

    if (appDemoDbEntryGet("singleRxTask",&value) == GT_OK)
    {
        singleTaskRxEnable = (value == 1) ? GT_TRUE : GT_FALSE;
    }
    if (appDemoDbEntryGet("singleRxTaskRandom",&value) == GT_OK)
    {
        singleTaskRxRandomEnable = (value == 1) ? GT_TRUE : GT_FALSE;
    }

    if (appDemoDbEntryGet("disableAutoPortMgr", &disableAutoPortMgr) != GT_OK)
    {
        disableAutoPortMgr = 0;
    }

    if (appDemoDbEntryGet("portMgr", &value) != GT_OK)
    {
        portMgr = GT_FALSE;
    }
    else
    {
        portMgr = (value == 1) ? GT_TRUE : GT_FALSE;
    }

    /* set default arrays and sizes */
    evHndlrCauseAllArr[0] = evHndlr0CauseDefaultArr;
    evHndlrCauseAllArr[1] = evHndlr1CauseDefaultArr;
    evHndlrCauseAllArr[2] = evHndlr2CauseDefaultArr;
    evHndlrCauseAllArr[3] = evHndlr3CauseDefaultArr;
    evHndlrCauseAllArr[4] = evHndlr4CauseDefaultArr;
    evHndlrCauseAllArr[5] = evHndlr5CauseDefaultArr;
    evHndlrCauseAllArr[6] = evHndlr6CauseDefaultArr;
    evHndlrCauseAllArr[7] = evHndlr7CauseDefaultArr;
    evHndlrCauseAllArr[8] = evHndlr8CauseDefaultArr;
    evHndlrCauseAllArr[9] = evHndlr9CauseDefaultArr;

    evHndlrCauseAllArrSize[0] = evHndlr0CauseDefaultArrSize;
    evHndlrCauseAllArrSize[1] = evHndlr1CauseDefaultArrSize;
    evHndlrCauseAllArrSize[2] = evHndlr2CauseDefaultArrSize;
    evHndlrCauseAllArrSize[3] = evHndlr3CauseDefaultArrSize;
    evHndlrCauseAllArrSize[4] = evHndlr4CauseDefaultArrSize;
    evHndlrCauseAllArrSize[5] = evHndlr5CauseDefaultArrSize;
    evHndlrCauseAllArrSize[6] = evHndlr6CauseDefaultArrSize;
    evHndlrCauseAllArrSize[7] = evHndlr7CauseDefaultArrSize;
    evHndlrCauseAllArrSize[8] = evHndlr8CauseDefaultArrSize;
    evHndlrCauseAllArrSize[9] = evHndlr9CauseDefaultArrSize;

    /* override defaults */
    /*rxEventHandlerLibInit();*/
#ifdef DXCH_CODE
    /*auEventHandlerLibInit();*/
#endif /*DXCH_CODE*/
#ifdef SHARED_MEMORY
    if(multiProcessAppDemo == GT_TRUE)
    {
        evHndlrCauseAllArrSize[1] = 0;
        evHndlrCauseAllArrSize[2] = 0;
        evHndlrCauseAllArrSize[3] = 0;
        evHndlrCauseAllArrSize[4] = 0;
        evHndlrCauseAllArrSize[5] = 0;
        evHndlrCauseAllArrSize[6] = 0;
        evHndlrCauseAllArrSize[7] = 0;
        evHndlrCauseAllArrSize[8] = 0;
    }
    else
#endif
    {
        if(appDemoPpConfigDevAmount == 0)
        {
            rxEventHandlerInitHandler(
                                      RX_EV_HANDLER_DEVNUM_ALL,
                                      RX_EV_HANDLER_QUEUE_ALL,
                                      RX_EV_HANDLER_TYPE_ANY_E,
                                      &rxEventHanderHnd);

            if (singleTaskRxEnable == GT_TRUE)
            {
                evHndlrCauseAllArrSize[1] = evHndlrCauseAllArrSize[2] = evHndlrCauseAllArrSize[3] =
                                evHndlrCauseAllArrSize[4] = evHndlrCauseAllArrSize[5] = 0;

                evHndlrCauseAllArr[APP_DEMO_SINGLE_RX_TASK_NUMBER] = evHndlr6CauseSingleTaskRxArr;
                evHndlrCauseAllArrSize[APP_DEMO_SINGLE_RX_TASK_NUMBER] = evHndlr6CauseSingleTaskRxArrSize;
            }
#ifdef DXCH_CODE
            auEventHandlerInitHandler(
                                      AU_EV_HANDLER_DEVNUM_ALL,
                                      AU_EV_HANDLER_TYPE_ANY_E,
                                      &auEventHandlerHnd);
#endif /*DXCH_CODE*/
        }
    }
    if(appDemoDbEntryGet("NoBindAuqPendingEvent",&value) == GT_OK)
    {
        evHndlrCauseAllArr[7] = evHndlr7CauseNoAuqPendingArr;
        evHndlrCauseAllArrSize[7] = evHndlr7CauseNoAuqPendingArrSize;
    }

    {
        /* silent the next events that are part of the ' controlled learning' .
           this event cause when we add/delete mac from the FDB and the FDB notify
           us , that the action done ,
           we get those from 4 port groups and it slows the operations */
        if (appDemoPpConfigList[devIdx].devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
        {
            for(i = 0 ; i < evHndlrCauseAllArrSize[0];i++)
            {
                if(evHndlr0CauseDefaultArr[i] == CPSS_PP_MAC_NA_LEARNED_E)
                {
                    evHndlr0CauseDefaultArr[i] = evHndlr0CauseDefaultArr[evHndlrCauseAllArrSize[0] - 1];
                    break;
                }
            }

            /* we removed CPSS_PP_MAC_NA_LEARNED_E */
            /* so need to decrement the number of elements */
            evHndlrCauseAllArrSize[0] = evHndlr0CauseDefaultArrSize - 1;
        }
    }

#if defined(DXCH_CODE) && defined(IMPL_GALTIS)

    if(CPSS_IS_DXCH_FAMILY_MAC(appDemoPpConfigList[devIdx].devFamily))
    {
        /* Get event increment CB routine. */
        wrCpssDxChDiagDataIntegrityCountersCbGet(&dxChDataIntegrityEventIncrementFunc);

        if(dxChDataIntegrityEventIncrementFunc != NULL)
        {
            /* trigger to allocate the needed memory for the device (during initialization)*/
            if(appDemoPpConfigList[devIdx].valid == GT_FALSE)
            {
                return GT_FAIL;
            }
            /* don't care about the return value */
            (GT_VOID)dxChDataIntegrityEventIncrementFunc(appDemoPpConfigList[devIdx].devNum, NULL);
        }
    }
#endif /* defined(DXCH_CODE) && defined(IMPL_GALTIS) */

#if defined(PX_CODE) && defined(IMPL_GALTIS)

    if(CPSS_IS_PX_FAMILY_MAC(appDemoPpConfigList[devIdx].devFamily))
    {
        /* Get event increment CB routine. */
        appDemoPxDiagDataIntegrityCountersCallBackGet(&pxDataIntegrityEventIncrementFunc);
        if(pxDataIntegrityEventIncrementFunc != NULL)
        {
                (GT_VOID)pxDataIntegrityEventIncrementFunc(appDemoPpConfigList[devIdx].devNum, NULL);
        }
    }
#endif

    /* bind the events for all the App-Demo event handlers */

    /* NOTE : MUST lock interrupts from first bind and unmask till end
             of bind and unmask , so we will not loose any event.

       explanation on option to loose interrupt if not locking:
       I will describe loose interrupt of 'link change' for a port that when
       'power up to board' is in state of 'up'.

       assume that the loop in not under 'lock interrupts'

       the event of 'link change' CPSS_PP_PORT_LINK_STATUS_CHANGED_E is in taskParamArr[9]
       and the event of CPSS_PP_PORT_RX_FIFO_OVERRUN_E is in taskParamArr[0]

       BUT those two events are actually in the same register of 'per port' ,
       so if the HW initialized also the 'rx fifo overrun' interrupts , then
       after the 'bind and unmask' of this event the ISR will read (and clear)
       the register of the 'per port' and will not care about the 'link change'
       event that is now LOST !
    */
    /* lock section to disable interruption of ISR while unmasking events */
#if 1
    if (appDemoPpConfigDevAmount==0)
    {
        for (i = 0; i < APP_DEMO_PROCESS_NUMBER; i++)
        {
            if(evHndlrCauseAllArrSize[i] == 0 || evHndlrCauseAllArr[i][0] == CPSS_UNI_RSRVD_EVENT_E)
                continue;
            taskParamArr[i].hndlrIndex = i;

            DBG_LOG(("appDemoEventRequestDrvnModeInit: subscribed group #%d for %d events\n", i, evHndlrCauseAllArrSize[i], 3, 4, 5, 6));
            /* call CPSS to bind the events under single handler */
            rc = cpssEventBind(evHndlrCauseAllArr[i],
                               evHndlrCauseAllArrSize[i],
                               &taskParamArr[i].evHndl);
            if (GT_OK != rc)
            {
                /*return rc;*/
            }
        }
    }


    for (i = 0; i < APP_DEMO_PROCESS_NUMBER; i++)
    {
         if(evHndlrCauseAllArrSize[i] == 0 || evHndlrCauseAllArr[i][0] == CPSS_UNI_RSRVD_EVENT_E)
            continue;

        /* call the CPSS to enable those interrupts in the HW of the device */
        rc = prvUniEvMaskAllDeviceSet(devIdx,evHndlrCauseAllArr[i],
                                evHndlrCauseAllArrSize[i],
                                &uniEvSkipArr[0],
                                uniEvSkipArrLength,
                                CPSS_EVENT_UNMASK_E);
        if (GT_OK != rc)
        {

            return GT_FAIL;
        }
    }
#endif
#ifdef DXCH_CODE

    if(CPSS_IS_DXCH_FAMILY_MAC(appDemoPpConfigList[devIdx].devFamily))
    {
        switch (appDemoPpConfigList[devIdx].deviceId)
        {/* in existing Hooper RD boards there is mistake with default state of GPP interrupt,
            mask it to prevent not needed interrupts storming */
            case CPSS_LION2_HOOPER_PORT_GROUPS_0123_DEVICES_CASES_MAC:
                (GT_VOID)cpssEventDeviceMaskSet(appDemoPpConfigList[devIdx].devNum,
                                                CPSS_PP_GPP_E,
                                                CPSS_EVENT_MASK_E);
                break;
            case CPSS_BOBK_ALL_DEVICES_CASES_MAC:
                /* BobK device has GPP#5 muxed with LED interface.
                   Need to disable event to avoid storming from LED related data. */
                (GT_VOID)cpssEventDeviceMaskWithEvExtDataSet(appDemoPpConfigList[devIdx].devNum,
                                                CPSS_PP_GPP_E, 5, CPSS_EVENT_MASK_E);
                break;
            default:
                break;
        }

        if((appDemoPpConfigList[devIdx].devFamily != CPSS_PP_FAMILY_DXCH_LION2_E) &&
            (!PRV_CPSS_SIP_5_10_CHECK_MAC(appDemoPpConfigList[devIdx].devNum)))  /* Bobcat2_B0, Bobk, Aldrin, AC3X, Bobcat3 */
        {
            (GT_VOID)cpssEventDeviceMaskSet(appDemoPpConfigList[devIdx].devNum,
                                        CPSS_PP_PORT_LANE_SYNC_STATUS_CHANGED_E,
                                            CPSS_EVENT_MASK_E);
        }
        else
        {
            GT_PHYSICAL_PORT_NUM            portNum;
            CPSS_PORT_INTERFACE_MODE_ENT    ifMode;

            /* loop over all ports */
            for (portNum = 0; portNum < (appDemoPpConfigList[devIdx].maxPortNumber); portNum++)
            {
                CPSS_ENABLER_PORT_SKIP_CHECK(appDemoPpConfigList[devIdx].devNum, portNum);

                rc = cpssDxChPortInterfaceModeGet(appDemoPpConfigList[devIdx].devNum, portNum, &ifMode);
                if (rc != GT_OK)
                {

                    return rc;
                }

                if(   (CPSS_PORT_INTERFACE_MODE_KR_E == ifMode)
                   || (CPSS_PORT_INTERFACE_MODE_KR2_E == ifMode)
                   || (CPSS_PORT_INTERFACE_MODE_KR4_E == ifMode)
                   || (CPSS_PORT_INTERFACE_MODE_CR_E == ifMode)
                   || (CPSS_PORT_INTERFACE_MODE_CR2_E == ifMode)
                   || (CPSS_PORT_INTERFACE_MODE_CR4_E == ifMode)
                   || (CPSS_PORT_INTERFACE_MODE_SR_LR_E == ifMode)
                   || (CPSS_PORT_INTERFACE_MODE_SR_LR2_E == ifMode)
                   || (CPSS_PORT_INTERFACE_MODE_XHGS_E == ifMode)
                   || (CPSS_PORT_INTERFACE_MODE_XHGS_SR_E == ifMode))
                {
                    if(!PRV_CPSS_SIP_5_15_CHECK_MAC(appDemoPpConfigList[devIdx].devNum)) /* Bobk, Aldrin, AC3X, Bobcat3 */
                    {

                         (GT_VOID)cpssEventDeviceMaskWithEvExtDataSet(appDemoPpConfigList[devIdx].devNum,
                                 CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E,
                                 portNum, CPSS_EVENT_UNMASK_E);
                    }
                }
                else if(CPSS_PORT_INTERFACE_MODE_RXAUI_E == ifMode)
                {
                    (GT_VOID)cpssEventDeviceMaskWithEvExtDataSet(appDemoPpConfigList[devIdx].devNum,
                        CPSS_PP_PORT_LANE_SYNC_STATUS_CHANGED_E,
                        ((portNum<<8)|0x1),
                        CPSS_EVENT_UNMASK_E);
                    (GT_VOID)cpssEventDeviceMaskWithEvExtDataSet(appDemoPpConfigList[devIdx].devNum,
                        CPSS_PP_PORT_LANE_SYNC_STATUS_CHANGED_E,
                        ((portNum<<8)|0x2),
                        CPSS_EVENT_UNMASK_E);
                }
            }
        }

        if(appDemoPpConfigList[devIdx].devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E)
        {
            (GT_VOID)cpssEventDeviceMaskSet(appDemoPpConfigList[devIdx].devNum,
                                            CPSS_PP_PORT_PRBS_ERROR_QSGMII_E,
                                            CPSS_EVENT_MASK_E);
        }


    }
#endif
   if (appDemoPpConfigDevAmount==0)
   {
       if (singleTaskRxEnable == GT_TRUE)
       {
           for (i = 0; i < treatTasksNum; i++)
           {
               /* create msgQ */
               osSprintf(name, "msgQ_%d", i);
               rc = cpssOsMsgQCreate(name,APP_DEMO_MSGQ_SIZE,
                                     sizeof(APP_DEMO_RX_PACKET_PARAMS),
                                     &(singleRxMsgQIds[i]));
               if (rc != GT_OK)
               {
                   return rc;
               }

               if(i == 0)/* create mutex only once */
               {
                   rc = cpssOsMutexCreate("rxMutex",&rxMutex);
                   if (rc != GT_OK)
                   {
                       return rc;
                   }
               }

               /* spawn rx treat task */
               osSprintf(name, "treatRx_%d", i);
               rc = osTaskCreate(name,
                                 EV_HANDLER_MAX_PRIO - APP_DEMO_SINGLE_RX_TASK_NUMBER - 1,
                                 _8KB,
                                 appDemoSingleRxTreatTask,
                                 &(singleRxMsgQIds[i]),
                                 &treatRxEventHandlerTid[i]);
               if (rc != GT_OK)
               {
                   return rc;
               }
           }
       }


       /* spawn all the event handler processes */
       for (i = 0; i < APP_DEMO_PROCESS_NUMBER; i++)
       {
           if(evHndlrCauseAllArrSize[i] == 0 || evHndlrCauseAllArr[i][0] == CPSS_UNI_RSRVD_EVENT_E)
               continue;
           osSprintf(name, "evHndl_%d", i);

           if (singleTaskRxEnable == GT_TRUE && i == APP_DEMO_SINGLE_RX_TASK_NUMBER)
           {
               start_addr = appDemoSingleRxEvHndlr;
           }
           else
           {
               start_addr = appDemoEvHndlr;
           }

           taskCreated = 0;
           rc = osTaskCreate(name,
                             EV_HANDLER_MAX_PRIO - i,
                             _32KB,
                             start_addr,
                             &taskParamArr[i],
                             &eventHandlerTidArr[i]);
           if (rc != GT_OK)
           {
               return GT_FAIL;
           }


           while(taskCreated == 0)
           {
               /* wait for indication that task created */
               osTimerWkAfter(1);

               if(0 == cpssOsStrCmp("fake",name))
               {
                    /* will never happen as name is with other name */
                    /* but we add fake condition for klockwork
                       that not aware that taskCreated is set to 0 by other task
                    */
                    break;
               }
           }
       }
   }

    return GT_OK;
}

/**
* @internal appDemoEventRequestDrvnModeInit function
* @endinternal
*
* @brief   This routine spawns the App Demo event handlers.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoEventRequestDrvnModeInit
(
    IN GT_VOID
)
{
    GT_STATUS rc;              /* The returned code            */
    GT_U32    i;               /* Iterator                     */
    GT_CHAR   name[30];        /* The task/msgQ name           */
    GT_U32    value;
    unsigned (__TASKCONV *start_addr)(GT_VOID*);
#if defined(DXCH_CODE)
    GT_U8   dev;
#endif
    GT_U32  taskPriority;      /* Priority of the created task*/

    if (appDemoDbEntryGet("singleRxTask",&value) == GT_OK)
    {
        singleTaskRxEnable = (value == 1) ? GT_TRUE : GT_FALSE;
    }
    if (appDemoDbEntryGet("singleRxTaskRandom",&value) == GT_OK)
    {
        singleTaskRxRandomEnable = (value == 1) ? GT_TRUE : GT_FALSE;
    }

    if (appDemoDbEntryGet("disableAutoPortMgr", &disableAutoPortMgr) != GT_OK)
    {
        disableAutoPortMgr = 0;
    }

    if (appDemoDbEntryGet("portMgr", &value) != GT_OK)
    {
        portMgr = GT_FALSE;
    }
    else
    {
        portMgr = (value == 1) ? GT_TRUE : GT_FALSE;
    }

    /* set default arrays and sizes */
    evHndlrCauseAllArr[0] = evHndlr0CauseDefaultArr;
    evHndlrCauseAllArr[1] = evHndlr1CauseDefaultArr;
    evHndlrCauseAllArr[2] = evHndlr2CauseDefaultArr;
    evHndlrCauseAllArr[3] = evHndlr3CauseDefaultArr;
    evHndlrCauseAllArr[4] = evHndlr4CauseDefaultArr;
    evHndlrCauseAllArr[5] = evHndlr5CauseDefaultArr;
    evHndlrCauseAllArr[6] = evHndlr6CauseDefaultArr;
    evHndlrCauseAllArr[7] = evHndlr7CauseDefaultArr;
    evHndlrCauseAllArr[8] = evHndlr8CauseDefaultArr;
    evHndlrCauseAllArr[9] = evHndlr9CauseDefaultArr;

    evHndlrCauseAllArrSize[0] = evHndlr0CauseDefaultArrSize;
    evHndlrCauseAllArrSize[1] = evHndlr1CauseDefaultArrSize;
    evHndlrCauseAllArrSize[2] = evHndlr2CauseDefaultArrSize;
    evHndlrCauseAllArrSize[3] = evHndlr3CauseDefaultArrSize;
    evHndlrCauseAllArrSize[4] = evHndlr4CauseDefaultArrSize;
    evHndlrCauseAllArrSize[5] = evHndlr5CauseDefaultArrSize;
    evHndlrCauseAllArrSize[6] = evHndlr6CauseDefaultArrSize;
    evHndlrCauseAllArrSize[7] = evHndlr7CauseDefaultArrSize;
    evHndlrCauseAllArrSize[8] = evHndlr8CauseDefaultArrSize;
    evHndlrCauseAllArrSize[9] = evHndlr9CauseDefaultArrSize;

    /* override defaults */
    rxEventHandlerLibInit();

#ifdef DXCH_CODE
    auEventHandlerLibInit();

#endif /*DXCH_CODE*/
#ifdef SHARED_MEMORY

    if(multiProcessAppDemo == GT_TRUE)
    {
        evHndlrCauseAllArrSize[1] = 0;
        evHndlrCauseAllArrSize[2] = 0;
        evHndlrCauseAllArrSize[3] = 0;
        evHndlrCauseAllArrSize[4] = 0;
        evHndlrCauseAllArrSize[5] = 0;
        evHndlrCauseAllArrSize[6] = 0;
        evHndlrCauseAllArrSize[7] = 0;
        evHndlrCauseAllArrSize[8] = 0;
    }
    else
#endif
    {
        rxEventHandlerInitHandler(
                RX_EV_HANDLER_DEVNUM_ALL,
                RX_EV_HANDLER_QUEUE_ALL,
                RX_EV_HANDLER_TYPE_ANY_E,
                &rxEventHanderHnd);

        if (singleTaskRxEnable == GT_TRUE)
        {
            evHndlrCauseAllArrSize[1] = evHndlrCauseAllArrSize[2] = evHndlrCauseAllArrSize[3] =
            evHndlrCauseAllArrSize[4] = evHndlrCauseAllArrSize[5] = 0;

            evHndlrCauseAllArr[APP_DEMO_SINGLE_RX_TASK_NUMBER] = evHndlr6CauseSingleTaskRxArr;
            evHndlrCauseAllArrSize[APP_DEMO_SINGLE_RX_TASK_NUMBER] = evHndlr6CauseSingleTaskRxArrSize;
        }
#ifdef DXCH_CODE

        auEventHandlerInitHandler(
                AU_EV_HANDLER_DEVNUM_ALL,
                AU_EV_HANDLER_TYPE_ANY_E,
                &auEventHandlerHnd);

#endif /*DXCH_CODE*/
    }
    if(appDemoDbEntryGet("NoBindAuqPendingEvent",&value) == GT_OK)
    {
        evHndlrCauseAllArr[7] = evHndlr7CauseNoAuqPendingArr;
        evHndlrCauseAllArrSize[7] = evHndlr7CauseNoAuqPendingArrSize;
    }


    {
        /* silent the next events that are part of the ' controlled learning' .
           this event cause when we add/delete mac from the FDB and the FDB notify
           us , that the action done ,
           we get those from 4 port groups and it slows the operations */
        if (appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].devFamily ==
            CPSS_PP_FAMILY_DXCH_LION2_E)
        {
            for(i = 0 ; i < evHndlrCauseAllArrSize[0];i++)
            {
                if(evHndlr0CauseDefaultArr[i] == CPSS_PP_MAC_NA_LEARNED_E)
                {
                    evHndlr0CauseDefaultArr[i] = evHndlr0CauseDefaultArr[evHndlrCauseAllArrSize[0] - 1];
                    break;
                }
            }

            /* we removed CPSS_PP_MAC_NA_LEARNED_E */
            /* so need to decrement the number of elements */
            evHndlrCauseAllArrSize[0] = evHndlr0CauseDefaultArrSize - 1;
        }
    }

#if defined(DXCH_CODE) && defined(IMPL_GALTIS)

    /* Get event increment CB routine. */
    wrCpssDxChDiagDataIntegrityCountersCbGet(&dxChDataIntegrityEventIncrementFunc);

    if(dxChDataIntegrityEventIncrementFunc != NULL)
    {
        /* trigger to allocate the needed memory for the device (during initialization)*/
        for(dev = SYSTEM_DEV_NUM_MAC(0); dev < SYSTEM_DEV_NUM_MAC(appDemoPpConfigDevAmount); dev++)
        {
            if(appDemoPpConfigList[dev].valid == GT_FALSE)
            {
                continue;
            }
            /* don't care about the return value */
            (GT_VOID)dxChDataIntegrityEventIncrementFunc(appDemoPpConfigList[dev].devNum, NULL);
        }
    }

#endif /* defined(DXCH_CODE) && defined(IMPL_GALTIS) */
#ifdef PX_CODE

    /* Get event increment CB routine. */
    appDemoPxDiagDataIntegrityCountersCallBackGet(&pxDataIntegrityEventIncrementFunc);
    if(pxDataIntegrityEventIncrementFunc != NULL)
    {
            (GT_VOID)pxDataIntegrityEventIncrementFunc(0, NULL);
    }
#endif


    /* bind the events for all the App-Demo event handlers */

    /* NOTE : MUST lock interrupts from first bind and unmask till end
             of bind and unmask , so we will not loose any event.

       explanation on option to loose interrupt if not locking:
       I will describe loose interrupt of 'link change' for a port that when
       'power up to board' is in state of 'up'.

       assume that the loop in not under 'lock interrupts'

       the event of 'link change' CPSS_PP_PORT_LINK_STATUS_CHANGED_E is in taskParamArr[9]
       and the event of CPSS_PP_PORT_RX_FIFO_OVERRUN_E is in taskParamArr[0]

       BUT those two events are actually in the same register of 'per port' ,
       so if the HW initialized also the 'rx fifo overrun' interrupts , then
       after the 'bind and unmask' of this event the ISR will read (and clear)
       the register of the 'per port' and will not care about the 'link change'
       event that is now LOST !
    */
    /* lock section to disable interruption of ISR while unmasking events */

    for (i = 0; i < APP_DEMO_PROCESS_NUMBER; i++)
    {
        if(evHndlrCauseAllArrSize[i] == 0 || evHndlrCauseAllArr[i][0] == CPSS_UNI_RSRVD_EVENT_E)
            continue;
        taskParamArr[i].hndlrIndex = i;

        DBG_LOG(("appDemoEventRequestDrvnModeInit: subscribed group #%d for %d events\n", i, evHndlrCauseAllArrSize[i], 3, 4, 5, 6));
        /* call CPSS to bind the events under single handler */
        rc = cpssEventBind(evHndlrCauseAllArr[i],
                           evHndlrCauseAllArrSize[i],
                           &taskParamArr[i].evHndl);
        if (GT_OK != rc)
        {
            return GT_FAIL;
        }
     }


    for (i = 0; i < APP_DEMO_PROCESS_NUMBER; i++)
    {
         if(evHndlrCauseAllArrSize[i] == 0 || evHndlrCauseAllArr[i][0] == CPSS_UNI_RSRVD_EVENT_E)
            continue;

        /* call the CPSS to enable those interrupts in the HW of the device */
        rc = prvUniEvMaskAllSet(evHndlrCauseAllArr[i],
                                evHndlrCauseAllArrSize[i],
                                &uniEvSkipArr[0],
                                uniEvSkipArrLength,
                                CPSS_EVENT_UNMASK_E);
        if (GT_OK != rc)
        {

            return GT_FAIL;
        }
    }

#ifdef DXCH_CODE

    for(dev = SYSTEM_DEV_NUM_MAC(0); dev < SYSTEM_DEV_NUM_MAC(appDemoPpConfigDevAmount); dev++)
    {
        if(GT_FALSE == appDemoPpConfigList[dev].valid)
        {
            continue;
        }


        switch (appDemoPpConfigList[dev].deviceId)
        {/* in existing Hooper RD boards there is mistake with default state of GPP interrupt,
            mask it to prevent not needed interrupts storming */
            case CPSS_LION2_HOOPER_PORT_GROUPS_0123_DEVICES_CASES_MAC:
                (GT_VOID)cpssEventDeviceMaskSet(appDemoPpConfigList[dev].devNum,
                                                CPSS_PP_GPP_E,
                                                CPSS_EVENT_MASK_E);
                break;
            case CPSS_BOBK_ALL_DEVICES_CASES_MAC:
                /* BobK device has GPP#5 muxed with LED interface.
                   Need to disable event to avoid storming from LED related data. */
                (GT_VOID)cpssEventDeviceMaskWithEvExtDataSet(appDemoPpConfigList[dev].devNum,
                                                CPSS_PP_GPP_E, 5, CPSS_EVENT_MASK_E);
                break;
            default:
                break;
        }

        if((appDemoPpConfigList[dev].devFamily != CPSS_PP_FAMILY_DXCH_LION2_E) &&
            (!PRV_CPSS_SIP_5_10_CHECK_MAC(appDemoPpConfigList[dev].devNum)))  /* Bobcat2_B0, Bobk, Aldrin, AC3X, Bobcat3 */
        {

            (GT_VOID)cpssEventDeviceMaskSet(appDemoPpConfigList[dev].devNum,
                                        CPSS_PP_PORT_LANE_SYNC_STATUS_CHANGED_E,
                                            CPSS_EVENT_MASK_E);
        }
        else
        {
            GT_PHYSICAL_PORT_NUM            portNum;
            CPSS_PORT_INTERFACE_MODE_ENT    ifMode;

            /* loop over all ports */
            for (portNum = 0; portNum < (appDemoPpConfigList[dev].maxPortNumber); portNum++)
            {
                CPSS_ENABLER_PORT_SKIP_CHECK(appDemoPpConfigList[dev].devNum, portNum);
                if(!appDemoDxChIsLocalPort_or_RemotePort_with_MAC_PHY_OBJ(dev, portNum))
                {
                    continue;
                }

                rc = cpssDxChPortInterfaceModeGet(appDemoPpConfigList[dev].devNum, portNum, &ifMode);
                if (rc != GT_OK)
                {

                    return rc;
                }

                if(   (CPSS_PORT_INTERFACE_MODE_KR_E == ifMode)
                   || (CPSS_PORT_INTERFACE_MODE_KR2_E == ifMode)
                   || (CPSS_PORT_INTERFACE_MODE_KR4_E == ifMode)
                   || (CPSS_PORT_INTERFACE_MODE_CR_E == ifMode)
                   || (CPSS_PORT_INTERFACE_MODE_CR2_E == ifMode)
                   || (CPSS_PORT_INTERFACE_MODE_CR4_E == ifMode)
                   || (CPSS_PORT_INTERFACE_MODE_SR_LR_E == ifMode)
                   || (CPSS_PORT_INTERFACE_MODE_SR_LR4_E == ifMode)
                   || (CPSS_PORT_INTERFACE_MODE_SR_LR2_E == ifMode)
                   || (CPSS_PORT_INTERFACE_MODE_XHGS_E == ifMode)
                   || (CPSS_PORT_INTERFACE_MODE_XHGS_SR_E == ifMode))
                {
                    if(!PRV_CPSS_SIP_5_15_CHECK_MAC(appDemoPpConfigList[dev].devNum)) /* Bobk, Aldrin, AC3X, Bobcat3 */
                    {

                         (GT_VOID)cpssEventDeviceMaskWithEvExtDataSet(appDemoPpConfigList[dev].devNum,
                                 CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E,
                                 portNum, CPSS_EVENT_UNMASK_E);
                    }
                }
                else if(CPSS_PORT_INTERFACE_MODE_RXAUI_E == ifMode)
                {
                    (GT_VOID)cpssEventDeviceMaskWithEvExtDataSet(appDemoPpConfigList[dev].devNum,
                        CPSS_PP_PORT_LANE_SYNC_STATUS_CHANGED_E,
                        ((portNum<<8)|0x1),
                        CPSS_EVENT_UNMASK_E);
                    (GT_VOID)cpssEventDeviceMaskWithEvExtDataSet(appDemoPpConfigList[dev].devNum,
                        CPSS_PP_PORT_LANE_SYNC_STATUS_CHANGED_E,
                        ((portNum<<8)|0x2),
                        CPSS_EVENT_UNMASK_E);
                }
            }
        }


        if(appDemoPpConfigList[dev].devFamily != CPSS_PP_FAMILY_DXCH_XCAT2_E)
        {
            (GT_VOID)cpssEventDeviceMaskSet(appDemoPpConfigList[dev].devNum,
                                            CPSS_PP_PORT_PRBS_ERROR_QSGMII_E,
                                            CPSS_EVENT_MASK_E);
        }


    }
#endif





    if (singleTaskRxEnable == GT_TRUE)
    {
        for (i = 0; i < treatTasksNum; i++)
        {
            /* create msgQ */
            osSprintf(name, "msgQ_%d", i);
            rc = cpssOsMsgQCreate(name,APP_DEMO_MSGQ_SIZE,
                                  sizeof(APP_DEMO_RX_PACKET_PARAMS),
                                  &(singleRxMsgQIds[i]));
            if (rc != GT_OK)
            {
                return rc;
            }

            if(i == 0)/* create mutex only once */
            {
                    rc = cpssOsMutexCreate("rxMutex",&rxMutex);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
            }

            /* spawn rx treat task */
            osSprintf(name, "treatRx_%d", i);
            rc = osTaskCreate(name,
                          EV_HANDLER_MAX_PRIO - APP_DEMO_SINGLE_RX_TASK_NUMBER - 1,
                          _8KB,
                          appDemoSingleRxTreatTask,
                          &(singleRxMsgQIds[i]),
                          &treatRxEventHandlerTid[i]);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }


    /* spawn all the event handler processes */
    for (i = 0; i < APP_DEMO_PROCESS_NUMBER; i++)
    {
        if(evHndlrCauseAllArrSize[i] == 0 || evHndlrCauseAllArr[i][0] == CPSS_UNI_RSRVD_EVENT_E)
            continue;
        osSprintf(name, "evHndl_%d", i);

        if (singleTaskRxEnable == GT_TRUE && i == APP_DEMO_SINGLE_RX_TASK_NUMBER)
        {
            start_addr = appDemoSingleRxEvHndlr;
        }
        else
        {
            start_addr = appDemoEvHndlr;
        }

        taskCreated = 0;
        taskPriority = EV_HANDLER_MAX_PRIO - i;
#ifdef DXCH_CODE
        if(evHndlrCauseAllArr[i][0] == CPSS_PP_EB_AUQ_PENDING_E)
        {
            /* For FDB manager performance improvement - set learning thread higher priority, aging lower */
            taskPriority = EV_HANDLER_MAX_PRIO;
        }
#endif
        rc = osTaskCreate(name,
                          taskPriority,
                          _32KB,
                          start_addr,
                          &taskParamArr[i],
                          &eventHandlerTidArr[i]);
        if (rc != GT_OK)
        {
            return GT_FAIL;
        }


        while(taskCreated == 0)
        {
            /* wait for indication that task created */
            osTimerWkAfter(1);
           if(0 == cpssOsStrCmp("fake",name))
           {
                /* will never happen as name is with other name */
                /* but we add fake condition for klockwork
                   that not aware that taskCreated is set to 0 by other task
                */
                break;
           }
        }
    }

#ifdef GM_USED

    /* the GM not hold MAC and therefor not generate next event :
        CPSS_PP_PORT_LINK_STATUS_CHANGED_E

        still we have WA using simulation on top of the GM.
    */
    {
        osSprintf(name, "GM_link_status");
        start_addr = gm_appDemoEmulateLinkChangeTask;

        taskCreated = 0;
        rc = osTaskCreate(name,
                          EV_HANDLER_MAX_PRIO - i,
                          _32KB,
                          start_addr,
                          NULL,
                          &gm_appDemoEmulateLinkChangeTask_Tid);
        if (rc != GT_OK)
        {
            return GT_FAIL;
        }


        while(taskCreated == 0)
        {
            /* wait for indication that task created */
            osTimerWkAfter(1);
           if(0 == cpssOsStrCmp("fake",name))
           {
                /* will never happen as name is with other name */
                /* but we add fake condition for klockwork
                   that not aware that taskCreated is set to 0 by other task
                */
                break;
           }
        }
    }
#endif /*GM_USED*/


    return GT_OK;
}

extern GT_STATUS prvCpssDrvEventSupportCheck
(
    IN  GT_U8                    devNum,
    IN  CPSS_UNI_EV_CAUSE_ENT    uniEvent,
    IN  GT_U32                   evExtData
);
/**
* @internal prvUniEvMaskAllDeviceSet function
* @endinternal
*
* @brief   This routine unmasks all the events according to the unified event list.
* @brief   It takes device also as parameter
* @param[in] cpssUniEventArr[]        - The CPSS unified event list.
* @param[in] arrLength                - The unified event list length.
* @param[in] operation                - type of  mask/unmask to do on the events
*                                       GT_OK on success, or
*                                       GT_FAIL if failed.
*/
GT_STATUS prvUniEvMaskAllDeviceSet
(
    IN  GT_U8                       devIdx,
    IN  CPSS_UNI_EV_CAUSE_ENT       cpssUniEventArr[],
    IN  GT_U32                      arrLength,
    IN  CPSS_UNI_EV_CAUSE_ENT       uniEvSkipArr[],
    IN  GT_U32                      skipArrLength,
    IN  CPSS_EVENT_MASK_SET_ENT     operation
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    i, j;           /* Iterators                  */
    GT_U8     devNum;         /* Device number              */
    GT_U32    time=0;
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery; /* holds system recovery information */

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(GT_FALSE == appDemoPpConfigList[devIdx].valid)
    {
        return GT_FAIL;
    }
    devNum = appDemoPpConfigList[devIdx].devNum;

    if ( PRV_CPSS_SIP_5_CHECK_MAC(devNum) &&
         (system_recovery.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E) &&
         (system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E ) )
        time = 200;

    /* unmask the interrupt */
    for (i = 0; i < arrLength; i++)
    {
        osTimerWkAfter(time);

        if (cpssUniEventArr[i] > CPSS_UNI_EVENT_COUNT_E) /* last event */
        {
            return GT_FAIL;
        }

        /* unmask the interrupt for all PPs/XBARs/FAs */

        /* assume PRV_CPSS_MAX_PP_DEVICES_CNS >= PRV_CPSS_MAX_XBAR_DEVICES_CNS */
#ifdef DXCH_CODE
        if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
        {
            if((cpssUniEventArr[i] == CPSS_PP_GPP_E)&&
                (GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,PRV_CPSS_DXCH_BOBCAT2_GPP_INTERRUPT_WA_E)))
            {
                continue;
            }
        }
#endif /*DXCH_CODE*/
        CPSS_TBD_BOOKMARK /* must define PEX window range to prevent this interrupt on all
                new devices */

        if(cpssUniEventArr[i] == CPSS_PP_PEX_HIT_DEFAULT_WIN_ERR_E)
        {
            continue;
        }

        if(PRV_CPSS_PP_MAC(devNum) &&
            CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
        {
            rc = prvCpssDrvEventSupportCheck(devNum,cpssUniEventArr[i],
                PRV_CPSS_DRV_EV_REQ_UNI_EV_EXTRA_DATA_ANY_CNS);
            if(rc != GT_OK)
            {
                /* the device not supports this event */
                /* do not call CPSS to avoid 'ERROR LOG' indications about those that are not supported */
                rc= GT_OK;
                continue;
            }
        }

        /* Don't skip events when CPSS disables (masks) them */
        if(operation != CPSS_EVENT_MASK_E)
        {
            if (uniEvSkipArr!=NULL)
            {
                for (j = 0; j < skipArrLength; j++)
                {
                    if (cpssUniEventArr[i] == uniEvSkipArr[j])
                    {
                        break;
                    }
                }
                /* Skip event found */
                if(j < skipArrLength)
                {
                    continue;
                }
            }
        }

        rc = cpssEventDeviceMaskSet(appDemoPpConfigList[devIdx].devNum, cpssUniEventArr[i], operation);
        switch(rc)
        {
            case GT_NOT_INITIALIZED:
                /* assume there are no FA/XBAR devices in the system ,
                    because there was initialization of the 'phase 1' for any FA/XBAR */

                /* fall through */
            case GT_BAD_PARAM:
                /* assume that this PP/FA/XBAR device not exists , so no DB of PP/FA/XBAR devices needed .... */

                /* fall through */
            case GT_NOT_FOUND:
                /* this event not relevant to this device */
                rc = GT_OK;
                break;

            case GT_OK:
                break;

            default:
                /* other real error */
                break;
        }
    }

    return rc;
}


/**
* @internal prvUniEvMaskAllSet function
* @endinternal
*
* @param[in] cpssUniEventArr[]        - The CPSS unified event list.
* @param[in] arrLength                - The unified event list length.
* @param[in] operation                - type of  mask/unmask to do on the events
*                                       GT_OK on success, or
*                                       GT_FAIL if failed.
*/
static GT_STATUS prvUniEvMaskAllSet
(
    IN  CPSS_UNI_EV_CAUSE_ENT       cpssUniEventArr[],
    IN  GT_U32                      arrLength,
    IN  CPSS_UNI_EV_CAUSE_ENT       uniEvSkipArr[],
    IN  GT_U32                      skipArrLength,
    IN  CPSS_EVENT_MASK_SET_ENT     operation
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    i, j;           /* Iterators                     */
    GT_U8     dev;            /* Device iterator              */
    GT_U8     devNum;         /* Device number              */
    GT_U32    time=0;
        CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery; /* holds system recovery information */

        rc = cpssSystemRecoveryStateGet(&system_recovery);
        if (rc != GT_OK)
        {
          return rc;
        }

    if ( (appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].valid) &&
         (PRV_CPSS_SIP_5_CHECK_MAC((appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].devNum))) &&
         (system_recovery.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E) &&
         (system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E ) )
        time = 200;

    /* unmask the interrupt */
    for (i = 0; i < arrLength; i++)
    {
        osTimerWkAfter(time);

        if (cpssUniEventArr[i] > CPSS_UNI_EVENT_COUNT_E) /* last event */
        {
            return GT_FAIL;
        }

        /* Don't skip events when CPSS disables (masks) them */
        if(operation != CPSS_EVENT_MASK_E)
        {
            if (uniEvSkipArr!=NULL)
            {
                for (j = 0; j < skipArrLength; j++)
                {
                    if (cpssUniEventArr[i] == uniEvSkipArr[j])
                    {
                        break;
                    }
                }
                /* Skip event found */
                if(j < skipArrLength)
                {
                    continue;
                }
            }
        }

        /* unmask the interrupt for all PPs/XBARs/FAs */

        /* assume PRV_CPSS_MAX_PP_DEVICES_CNS >= PRV_CPSS_MAX_XBAR_DEVICES_CNS */
        for (dev = 0; dev < PRV_CPSS_MAX_PP_DEVICES_CNS; dev++)
        {
            if(GT_FALSE == appDemoPpConfigList[dev].valid)
            {
                continue;
            }
            devNum = appDemoPpConfigList[dev].devNum;

#ifdef DXCH_CODE
            if (CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
            {
                if((cpssUniEventArr[i] == CPSS_PP_GPP_E)&&
                    (GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,PRV_CPSS_DXCH_BOBCAT2_GPP_INTERRUPT_WA_E)))
                {
                    continue;
                }
            }
#endif /*DXCH_CODE*/
            CPSS_TBD_BOOKMARK /* must define PEX window range to prevent this interrupt on all
                    new devices */

            if(cpssUniEventArr[i] == CPSS_PP_PEX_HIT_DEFAULT_WIN_ERR_E)
            {
                continue;
            }



            if(PRV_CPSS_PP_MAC(devNum) &&
               CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
            {
                rc = prvCpssDrvEventSupportCheck(devNum,cpssUniEventArr[i],
                    PRV_CPSS_DRV_EV_REQ_UNI_EV_EXTRA_DATA_ANY_CNS);
                if(rc != GT_OK)
                {
                    /* the device not supports this event */
                    /* do not call CPSS to avoid 'ERROR LOG' indications about those that are not supported */
                    rc = GT_OK;
                    continue;
                }
            }

#ifdef ASIC_SIMULATION
            if(CPSS_PP_LMU_LATENCY_EXCEED_THRESHOLD_E == cpssUniEventArr[i] &&
               operation == CPSS_EVENT_UNMASK_E)
            {
                /* it generates too many interrupts that cause simulation to waist buffers :
                    sbufAlloc: no buffers
                    smemChtActiveWriteInterruptsMaskReg:device[falcon_single_tile] no buffers to update interrupt mask
                */
                continue;
            }
#endif /*ASIC_SIMULATION*/

            if(CPSS_PP_HSR_PRP_PNT_NUM_OF_READY_ENTRIES_E == cpssUniEventArr[i] &&
               operation == CPSS_EVENT_UNMASK_E)
            {
                /* it generates interrupts for every packet as the PNT hold no 'ready' entries.
                   the interrupts should be explicitly unmassked only by a 'test/application'
                   after setting properly the PNT table with 'ready' entries.
                */
                continue;
            }


            rc = cpssEventDeviceMaskSet(appDemoPpConfigList[dev].devNum, cpssUniEventArr[i], operation);
            switch(rc)
            {
                case GT_NOT_INITIALIZED:
                    /* assume there are no FA/XBAR devices in the system ,
                       because there was initialization of the 'phase 1' for any FA/XBAR */

                    /* fall through */
                case GT_BAD_PARAM:
                    /* assume that this PP/FA/XBAR device not exists , so no DB of PP/FA/XBAR devices needed .... */

                    /* fall through */
                case GT_NOT_FOUND:
                    /* this event not relevant to this device */
                    rc = GT_OK;
                    break;

                case GT_OK:
                    break;

                default:
                    /* other real error */
                    break;
            }
        }
    }

    return rc;
} /* prvUniEvMaskAllSet */

static  GT_TASK appDemoTestsEventHandlerTid = 0;     /* task ID for test's event handler */

/**
* @internal appDemoEventsToTestsHandlerBind function
* @endinternal
*
* @brief   This routine creates new event handler for the unified event list.
*
* @param[in] cpssUniEventArr[]        - The CPSS unified event list.
* @param[in] arrLength                - The unified event list length.
* @param[in] operation                - type of  mask/unmask to do on the events
*                                       GT_OK on success, or
*                                       GT_FAIL if failed.
*/
GT_STATUS appDemoEventsToTestsHandlerBind
(
    IN  CPSS_UNI_EV_CAUSE_ENT       cpssUniEventArr[],
    IN  GT_U32                      arrLength,
    IN  CPSS_EVENT_MASK_SET_ENT     operation
)
{
    EV_HNDLR_PARAM  taskParam;     /* parameters for task */
    GT_STATUS       rc;            /* return code */
    char    name[30] = "eventGenerationTask"; /* task name */

    if ((operation == CPSS_EVENT_UNMASK_E) && (appDemoTestsEventHandlerTid == 0))
    {
        /* call CPSS to bind the events under single handler */
        rc = cpssEventBind(cpssUniEventArr, arrLength, &taskParam.evHndl);
        if (GT_OK != rc)
        {
            osPrintf("appDemoEventsToTestsHandlerBind: bind fail %d", rc);
            return rc;
        }

        /* Create event handler task */
        taskParam.hndlrIndex = 25;
        taskCreated = 0;
        rc = osTaskCreate(name, EV_HANDLER_MAX_PRIO,  _32KB, appDemoEvHndlr,
                          &taskParam,  &appDemoTestsEventHandlerTid);
        if (rc != GT_OK)
        {
            osPrintf("appDemoEventsToTestsHandlerBind: task create fail %d", rc);
            return rc;
        }

        while(taskCreated == 0)
        {
            /* wait for indication that task created */
            osTimerWkAfter(1);
           if(0 == cpssOsStrCmp("fake",name))
           {
                /* will never happen as name is with other name */
                /* will never happen as devIdx is 0..128 */
                /* but we add fake condition for klockwork
                   that not aware that taskCreated is set to 0 by other task
                */
                break;
           }
        }
    }

    /* call the CPSS to enable those interrupts in the HW of the device */
    rc = prvUniEvMaskAllSet(cpssUniEventArr, arrLength, NULL, 0, operation);
    if (GT_OK != rc)
    {
        osPrintf("appDemoEventsToTestsHandlerBind: mask/unmask fail %d", rc);
        return rc;
    }
    return GT_OK;
}

/**
* @internal appDemoEventFatalErrorEnable function
* @endinternal
*
* @brief   Set fatal error handling type.
*
* @param[in] fatalErrorType           - fatal error handling type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong fatalErrorType
*/
GT_STATUS appDemoEventFatalErrorEnable
(
    CPSS_ENABLER_FATAL_ERROR_TYPE fatalErrorType
)
{
    switch(fatalErrorType)
    {
        case CPSS_ENABLER_FATAL_ERROR_NOTIFY_ONLY_TYPE_E:
        case CPSS_ENABLER_FATAL_ERROR_EXECUTE_TYPE_E:
        case CPSS_ENABLER_FATAL_ERROR_SILENT_TYPE_E:
            prvAppDemoFatalErrorType = fatalErrorType;
            break;
        default:
            return GT_BAD_PARAM;
    }
    return GT_OK;
}

/**
* @internal appDemoEventFatalErrorEnable function
* @endinternal
*
* @brief   Set fatal error handling type.
*
* @param[in] fatalErrorType           - fatal error handling type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong fatalErrorType
*/
GT_STATUS appDemoDataIntegrityEventDumpEnable
(
    GT_BOOL enable
)
{
    prvAppDemoDataIntegrityEventDump = enable;
    return GT_OK;
}

/**
* @internal appDemoMacSecEventsDumpEnable function
* @endinternal
*
* @brief   Set MACSec events dump.
*
* @param[in] enable        - enable/disable macsec events dump
*                            GT_TRUE - enable debugs
*                            GT_FALSE - disable debugs
*
* @retval    GT_OK         - on success
*/
GT_STATUS appDemoMacSecEventsDumpEnable
(
    GT_BOOL enable
)
{
    prvAppDemoMacSecEventsDump = enable;
    return GT_OK;
}

#if (defined DXCH_CODE) && (!defined ASIC_SIMULATION)

/* type of combo port:
   1 - XG;
   0 - GE
*/
static GT_U32 comboType = 1; /* be default XG */
GT_STATUS dbgSetComboType
(
    IN  GT_U32  cType
)
{
    GT_U32 oldType = comboType;

    comboType = cType;

    return oldType;
}

/**
* @internal prvAppDemoUserHandleDxChComboPort function
* @endinternal
*
* @brief   Set active MAC of combo port.
*
* @param[in] devNum                   - the device number.
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS prvAppDemoUserHandleDxChComboPort
(
    GT_U8                   devNum,
    GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_STATUS               rc;
    GT_U32                  i;
    GT_BOOL                 enable;
    CPSS_DXCH_PORT_COMBO_PARAMS_STC params;
    GT_BOOL                 signalState;
    GT_U32                  portGroupId;
    GT_U32                  localPort;
    GT_U32                  lanesArrayP9[2] = {13, 18};
    GT_U32                  lanesArrayP11[2] = {15, 20};
    GT_U32                  *portLanesArrayPtr;

    rc = cpssDxChPortComboModeEnableGet(devNum, portNum, &enable, &params);
    if(rc != GT_OK)
    {
        cpssOsPrintSync("cpssDxChPortComboModeEnableGet(portNum=%d):rc=%d\n",
                        portNum, rc);
    }

    if(!enable)
    {
        return GT_OK;
    }

    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portNum);
    portLanesArrayPtr = (9 == localPort) ? lanesArrayP9 : lanesArrayP11;
    for(i = 0; i < 2; i++)
    {
        /* check that signal up detected on port - on all its serdes lanes */
        rc = cpssDxChPortSerdesLaneSignalDetectGet(devNum, portGroupId,
                                                   portLanesArrayPtr[i],
                                                   &signalState);
        if (rc != GT_OK)
        {
            cpssOsPrintSync("cpssDxChPortSerdesLaneSignalDetectGet fail:portGroupId=%d,lane=%d,rc=%d\n",
                            portGroupId, portLanesArrayPtr[i], rc);
            return rc;
        }
        if (GT_TRUE == signalState)
        {
            break;
        }
    }

    if (i < 2) /* if signal changed to up */
    {
        rc = cpssDxChPortComboPortActiveMacSet(devNum, portNum,
                                               &(params.macArray[i]));
        if (rc != GT_OK)
        {
            cpssOsPrintSync("cpssDxChPortComboPortActiveMacSet fail:portNum=%d,macArrIdx=%d,rc=%d\n",
                            portNum, i, rc);
            return rc;
        }
    }

    return GT_OK;
}
#endif

#ifdef DXCH_CODE
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>

/**
* @internal sip5_20_linkChange function
* @endinternal
*
* @brief   function to handle link change in sip5_20 .
*
* @param[in] devNum                   - the device number.
* @param[in] phyPortNum               - the physical port number that generated the event of
*                                      CPSS_PP_PORT_LINK_STATUS_CHANGED_E
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS   sip5_20_linkChange(
    IN GT_U8 devNum,
    IN GT_U32 phyPortNum
)
{
    GT_STATUS rc;
    GT_BOOL linkUp;
    GT_BOOL isPortInUnidirectionalMode;/*is port of a 'link down' in mode of 'Unidirectional' */
    CPSS_PORT_MAC_TYPE_ENT   portMacType;
    GT_U32 regAddr;
    CPSS_DXCH_DETAILED_PORT_MAP_STC portMap;
    GT_U32      timeout;        /* resources free timeout counter */
    GT_U16      portTxqDescNum; /* number of not treated TXQ descriptors */
    GT_BOOL     portShaperEnable; /* current state of port shaper */
    GT_BOOL     portTcShaperEnable[CPSS_TC_RANGE_CNS]; /* current state of port TC shapers */
    GT_U8       tc; /* traffic class */
    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT portLinkStatusState;

    rc = cpssDxChPortPhysicalPortDetailedMapGet(devNum, phyPortNum, &portMap);
    if(rc != GT_OK)
    {
        DBG_LOG(("cpssDxChPortPhysicalPortDetailedMapGet: error, devNum=%d, port=%d, rc=%d\n",
                devNum, phyPortNum, rc, 4, 5, 6));
        return rc;
    }

    rc = cpssDxChPortMacTypeGet(devNum, phyPortNum, &portMacType);
    if (rc != GT_OK)
    {
        DBG_LOG(("cpssDxChPortMacTypeGet: error, devNum=%d, port=%d, rc=%d\n",
                devNum, phyPortNum, rc, 4, 5, 6));
        return rc;
    }

    /* 1. we got valid physical port for our MAC number */
    rc = cpssDxChPortLinkStatusGet(devNum, phyPortNum, &linkUp);
    if (rc != GT_OK)
    {
        DBG_LOG(("cpssDxChPortLinkStatusGet: error, devNum=%d, port=%d, rc=%d\n",
                devNum, phyPortNum, rc, 4, 5, 6));
        return rc;
    }

    if(printLinkChangeEnabled)
    {
        cpssOsPrintSync("LINK %s : devNum[%ld] (physical port[%2ld]) (MAC[%2d]) \n",
                            (linkUp == GT_FALSE) ? "DOWN" : "UP  ",
                                        (GT_U32)devNum, phyPortNum,
                                        portMap.portMap.macNum);
    }

    isPortInUnidirectionalMode = GT_FALSE;

    if(linkUp == GT_FALSE)
    {
        rc = cpssDxChVntOamPortUnidirectionalEnableGet(devNum, phyPortNum, &isPortInUnidirectionalMode);
#ifdef GM_USED
        isPortInUnidirectionalMode = GT_FALSE;
        rc = GT_OK;
#endif /*GM_USED*/

        if(rc != GT_OK)
        {
            DBG_LOG(("cpssDxChVntOamPortUnidirectionalEnableGet: error, devNum=%d, port=%d, rc=%d\n",
                    devNum, phyPortNum, rc, 4, 5, 6));

        }
        if(isPortInUnidirectionalMode == GT_TRUE)
        {
            if(printLinkChangeEnabled)
            {
                cpssOsPrintSync("MAC Bidirectional enabled : even though the devNum[%ld] (physical port[%ld]) is DOWN the port can egress packets \n",
                                    (GT_U32)devNum, phyPortNum);
            }

            /* !!! do not modify the filter !!! keep it as 'link up' */
            return GT_OK;
        }
    }



    /* 2. set CG Flush configuration */
    if((portMacType == CPSS_PORT_MAC_TYPE_CG_E) && (linkUp == GT_FALSE))
    {
        regAddr = PRV_DXCH_REG1_UNIT_CG_PORT_MAC_MAC(devNum, portMap.portMap.macNum).CGPORTMACCommandConfig;

        if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
             rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 22, 1, 1);
             if (rc != GT_OK)
             {
                DBG_LOG(("prvCpssHwPpSetRegField: error, devNum=%d, port=%d, rc=%d\n",
                        devNum, phyPortNum, rc, 4, 5, 6));
                return rc;
             }
        }
    }

    portLinkStatusState =  linkUp ?
            CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E :
            CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E;

    if(linkUp == GT_TRUE)
    {
        GT_BOOL     portEnabled;
        rc = cpssDxChPortEnableGet(devNum,phyPortNum,&portEnabled);
        /* if the caller set the MAC to be disabled ...
           we need to assume that the EGF filter should treat as 'link down' */
        if(rc == GT_OK && portEnabled == GT_FALSE)
        {
            portLinkStatusState = CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E;
        }
    }
    /* 3. need to set the EGF link status filter according to new state of the
       port */
    rc = cpssDxChBrgEgrFltPortLinkEnableSet(devNum, phyPortNum,
        portLinkStatusState);
    if (rc != GT_OK)
    {
        DBG_LOG(("cpssDxChBrgEgrFltPortLinkEnableSet: error, devNum=%d, port=%d, link state[%d] rc=%d\n",
                devNum, phyPortNum, portLinkStatusState, rc, 5, 6));
        return rc;
    }

    if(linkUp == GT_FALSE)
    {
        /****************************************/
        /* 4. Disable any shapers on given port */
        /****************************************/
        rc = cpssDxChPortTxShaperEnableGet(devNum, phyPortNum, &portShaperEnable);
        if (rc != GT_OK)
        {
            DBG_LOG(("cpssDxChPortTxShaperEnableGet: error, devNum=%d, port=%d\n", devNum, phyPortNum));
            return rc;
        }
        for(tc = 0; tc < CPSS_TC_RANGE_CNS; tc++)
        {
            rc = cpssDxChPortTxQShaperEnableGet(devNum, phyPortNum, tc, &portTcShaperEnable[tc]);
            if (rc != GT_OK)
            {
                DBG_LOG(("cpssDxChPortTxQShaperEnableGet: error, devNum=%d, port=%d tc =%d\n", devNum, phyPortNum, tc));
                return rc;
            }

        }
        rc = cpssDxChTxPortShapersDisable(devNum, phyPortNum);
        if (rc != GT_OK)
        {
            DBG_LOG(("cpssDxChTxPortShapersDisable: error, devNum=%d, port=%d\n", devNum, phyPortNum));
            return rc;
        }

        /*******************************************************/
        /* 5. Polling the TXQ port counter until it reach zero */
        /*******************************************************/
        for(timeout = 100; timeout > 0; timeout--)
        {
            rc = cpssDxChPortTxDescNumberGet(devNum, phyPortNum, &portTxqDescNum);
            if(rc != GT_OK)
            {
                return rc;
            }

            if(0 == portTxqDescNum)
                break;
            else
                cpssOsTimerWkAfter(10);
        }

        if(0 == timeout)
        {
            cpssOsPrintf("cpssDxChPortTxDescNumberGet: TXQ descriptor counter read timeout error for port=%d,portTxqDescNum=0x%x \n", phyPortNum, portTxqDescNum);
            return GT_TIMEOUT;
        }

        /*********************************************/
        /* 6. wait 1us for the TXQ-MAC path to drain */
        /*********************************************/
        cpssOsTimerWkAfter(1);

        /*************************************/
        /* 7. Re-enable all disabled shapers */
        /*************************************/
        rc = cpssDxChPortTxShaperEnableSet(devNum, phyPortNum, portShaperEnable);
        if (rc != GT_OK)
        {
            DBG_LOG(("cpssDxChPortTxShaperEnableSet: error, devNum=%d, port=%d\n", devNum, phyPortNum));
            return rc;
        }
        for(tc = 0; tc < CPSS_TC_RANGE_CNS; tc++)
        {
            rc = cpssDxChPortTxQShaperEnableSet(devNum, phyPortNum, tc, portTcShaperEnable[tc]);
            if (rc != GT_OK)
            {
                DBG_LOG(("cpssDxChPortTxQShaperEnableSet: error, devNum=%d, port=%d tc =%d\n", devNum, phyPortNum, tc));
                return rc;
            }

        }

        /***********************************/
        /* 8. unset CG Flush configuration */
        /***********************************/
        if(portMacType == CPSS_PORT_MAC_TYPE_CG_E)
        {
            regAddr = PRV_DXCH_REG1_UNIT_CG_PORT_MAC_MAC(devNum, portMap.portMap.macNum).CGPORTMACCommandConfig;

            if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                 rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 22, 1, 0);
                 if (rc != GT_OK)
                 {
                    DBG_LOG(("prvCpssHwPpSetRegField: error, devNum=%d, port=%d, rc=%d\n",
                            devNum, phyPortNum, rc, 4, 5, 6));
                    return rc;
                 }
            }
        }
    }

    return GT_OK;
}


/**
* @internal sip6_alignLockChange function
* @endinternal
*
* @brief   function to handle align lock change in sip6 .
*
* @param[in] devNum                   - the device number.
* @param[in] phyPortNum               - the physical port number that generated the event of
*                                      CPSS_PP_PORT_LINK_STATUS_CHANGED_E
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS   sip6_alignLockChange(
    IN GT_U8 devNum,
    IN GT_U32 phyPortNum
)
{
    GT_STATUS rc;
    GT_BOOL alignLockStatus;
    CPSS_DXCH_DETAILED_PORT_MAP_STC portMap;
    GT_U32 secondsCurrent, nanoSecondsCurrent;

    if(linkTraceFeatureEnabled == GT_FALSE)
    {
        return GT_OK;
    }

    rc = cpssDxChPortPhysicalPortDetailedMapGet(devNum, phyPortNum, &portMap);
    if(rc != GT_OK)
    {
        DBG_LOG(("cpssDxChPortPhysicalPortDetailedMapGet: error, devNum=%d, port=%d, rc=%d\n",
                devNum, phyPortNum, rc, 4, 5, 6));
        return rc;
    }

    rc = prvCpssPortPcsAlignLockStatusGet(devNum, phyPortNum, portMap.portMap.macNum, &alignLockStatus);
    if(rc != GT_OK)
    {
        DBG_LOG(("prvCpssPortPcsAlignLockStatusGet: error, devNum=%d, port=%d, rc=%d\n",
                devNum, phyPortNum, rc, 4, 5, 6));
        return rc;
    }

    cpssOsTimeRT(&secondsCurrent,&nanoSecondsCurrent);
    linkTraceDB[currentEventIndex].valid = GT_TRUE;
    linkTraceDB[currentEventIndex].portNum = phyPortNum;
    linkTraceDB[currentEventIndex].portMacNum = portMap.portMap.macNum;
    linkTraceDB[currentEventIndex].eventType = CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E;
    linkTraceDB[currentEventIndex].eventValue = alignLockStatus;

    linkTraceDB[currentEventIndex].timeStampSec = secondsCurrent-secondsStart;
    if(nanoSecondsCurrent >= nanoSecondsStart)
    {
        linkTraceDB[currentEventIndex].timeStampNanoSec = nanoSecondsCurrent-nanoSecondsStart;
    }
    else
    {
        linkTraceDB[currentEventIndex].timeStampNanoSec = (1000000000 - nanoSecondsStart) + nanoSecondsCurrent;
        linkTraceDB[currentEventIndex].timeStampSec--;
    }
    currentEventIndex++;
    if(currentEventIndex == APPDEMO_LINK_TRACE_DB_SIZE)
    {
        currentEventIndex = 0;
    }
    return GT_OK;
}
/**
* @internal linkChangeForCascadePort function
* @endinternal
*
* @brief   function to handle link change for cascade port.
*
* @param[in] devNum                   - the device number.
* @param[in] phyPortNum               - the physical port number that generated the event of
*                                      CPSS_PP_PORT_LINK_STATUS_CHANGED_E
* @param[in] portLinkStatusState - new link status
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/

static GT_STATUS   linkChangeForCascadePort(
    IN GT_U8 devNum,
    IN GT_U32 phyPortNum,
    IN CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT portLinkStatusState
)
{
    GT_U32 i;
    GT_PHYSICAL_PORT_NUM remotePhysicalPorts[PRV_CPSS_DXCH_FALCON_TXQ_MAX_REMOTE_PORT_MAC+1/*cascade*/];
    GT_U32 remotePhysicalPortsNum=0;
    GT_STATUS rc;
    CPSS_DXCH_DETAILED_PORT_MAP_STC portMap;

    rc = cpssDxChPortPhysicalPortDetailedMapGet(devNum, phyPortNum, &portMap);
    if(rc != GT_OK)
    {
        DBG_LOG(("cpssDxChPortPhysicalPortDetailedMapGet: error, devNum=%d, port=%d, rc=%d\n",
                devNum, phyPortNum, rc, 4, 5, 6));
        return rc;
    }

    rc = prvCpssDxChPortPhysicalPortMapReverseDmaMappingGet(devNum,portMap.portMap.txDmaNum,
            &remotePhysicalPortsNum,remotePhysicalPorts);
    if(rc!=GT_OK)
    {
        return rc;
    }

    for(i=0;i<remotePhysicalPortsNum;i++)
    {
        if((GT_U32)(remotePhysicalPorts[i])!=phyPortNum)
        {
             rc = cpssDxChBrgEgrFltPortLinkEnableSet(devNum, remotePhysicalPorts[i],
                    portLinkStatusState);
            if (rc != GT_OK)
            {
                DBG_LOG(("cpssDxChBrgEgrFltPortLinkEnableSet: error, devNum=%d, port=%d, link state[%d] rc=%d\n",
                        devNum, remotePhysicalPorts[i], portLinkStatusState, rc, 5, 6));
                return rc;
            }
        }
    }

    return GT_OK;
}


/**
* @internal sip6_linkChange function
* @endinternal
*
* @brief   function to handle link change in sip6 .
*
* @param[in] devNum                   - the device number.
* @param[in] phyPortNum               - the physical port number that generated the event of
*                                      CPSS_PP_PORT_LINK_STATUS_CHANGED_E
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS   sip6_linkChange(
    IN GT_U8 devNum,
    IN GT_U32 phyPortNum
)
{
    GT_STATUS rc;
    GT_BOOL linkUp;
    CPSS_DXCH_DETAILED_PORT_MAP_STC portMap;
    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT portLinkStatusState;
    GT_BOOL     portEnabled;
    GT_U32 secondsCurrent, nanoSecondsCurrent;


    rc = cpssDxChPortPhysicalPortDetailedMapGet(devNum, phyPortNum, &portMap);
    if(rc != GT_OK)
    {
        DBG_LOG(("cpssDxChPortPhysicalPortDetailedMapGet: error, devNum=%d, port=%d, rc=%d\n",
                devNum, phyPortNum, rc, 4, 5, 6));
        return rc;
    }

    /* 1. we got valid physical port for our MAC number */
    rc = cpssDxChPortLinkStatusGet(devNum, phyPortNum, &linkUp);
    if (rc != GT_OK)
    {
        DBG_LOG(("cpssDxChPortLinkStatusGet: error, devNum=%d, port=%d, rc=%d\n",
                devNum, phyPortNum, rc, 4, 5, 6));
        return rc;
    }

    if(printLinkChangeEnabled)
    {
        cpssOsPrintSync("LINK %s : devNum[%ld] (physical port[%2ld]) (MAC[%2d]) \n",
                            (linkUp == GT_FALSE) ? "DOWN" : "UP  ",
                                        (GT_U32)devNum, phyPortNum,
                                        portMap.portMap.macNum);
    }

    if(linkTraceFeatureEnabled)
    {
        cpssOsTimeRT(&secondsCurrent,&nanoSecondsCurrent);
        linkTraceDB[currentEventIndex].valid = GT_TRUE;
        linkTraceDB[currentEventIndex].portNum = phyPortNum;
        linkTraceDB[currentEventIndex].portMacNum = portMap.portMap.macNum;
        linkTraceDB[currentEventIndex].eventType = CPSS_PP_PORT_LINK_STATUS_CHANGED_E;
        linkTraceDB[currentEventIndex].eventValue = linkUp;

        linkTraceDB[currentEventIndex].timeStampSec = secondsCurrent-secondsStart;
        if(nanoSecondsCurrent >= nanoSecondsStart)
        {
            linkTraceDB[currentEventIndex].timeStampNanoSec = nanoSecondsCurrent-nanoSecondsStart;
        }
        else
        {
            linkTraceDB[currentEventIndex].timeStampNanoSec = (1000000000 - nanoSecondsStart) + nanoSecondsCurrent;
            linkTraceDB[currentEventIndex].timeStampSec--;
        }
        currentEventIndex++;
        if(currentEventIndex == APPDEMO_LINK_TRACE_DB_SIZE)
        {
            currentEventIndex = 0;
        }
    }

    portLinkStatusState =  linkUp ?
            CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E :
            CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E;

    rc = cpssDxChPortEnableGet(devNum,phyPortNum,&portEnabled);
    if (rc != GT_OK)
    {
          DBG_LOG(("cpssDxChPortEnableGet: error, devNum=%d, port=%d, rc=%d\n",
                    devNum, phyPortNum, rc));
        return rc;
    }

    /* 3. need to set the EGF link status filter according to new state of the
       port
        Note: (unlike sip 5.20)
            cpssDxChBrgEgrFltPortLinkEnableSet implementation checks port link enable status and allows
            FORCE_LINK_UP_E only if port TX enabled and there is TXQ resources allocated.
    */
    rc = cpssDxChBrgEgrFltPortLinkEnableSet(devNum, phyPortNum,
        portLinkStatusState);
    if (rc != GT_OK)
    {
        DBG_LOG(("cpssDxChBrgEgrFltPortLinkEnableSet: error, devNum=%d, port=%d, link state[%d] rc=%d\n",
                devNum, phyPortNum, portLinkStatusState, rc, 5, 6));
        return rc;
    }

    if(portMap.portMap.isExtendedCascadePort)
    {
        rc = linkChangeForCascadePort(devNum, phyPortNum,
        portLinkStatusState);

        if (rc != GT_OK)
        {
              DBG_LOG(("linkChangeForCascadePort: error, devNum=%d, port=%d, rc=%d\n",
                        devNum, phyPortNum, rc));
            return rc;
        }
    }
    if((linkUp == GT_FALSE))
    {
        /*Disable port,this will cause "open drain"*/
        rc = cpssDxChPortEnableSet(devNum, phyPortNum,GT_FALSE);
        if (rc != GT_OK)
        {
              DBG_LOG(("cpssDxChPortEnableSet: error, devNum=%d, port=%d, rc=%d\n",
                        devNum, phyPortNum, rc));
            return rc;
        }

        /*Restore previus port state*/
        if(portEnabled==GT_TRUE)
        {
            rc = cpssDxChPortEnableSet(devNum, phyPortNum,GT_TRUE);
            if (rc != GT_OK)
            {
                DBG_LOG(("cpssDxChPortEnableSet: error, devNum=%d, port=%d, rc=%d\n",
                        devNum, phyPortNum, rc));
                return rc;
            }
        }

    }

    return GT_OK;
}


/**
* @internal appDemoDxChApLinkChangeTreat function
* @endinternal
*
* @brief   Link change treat for DXCH AP running ports
*
* @param[in] devNum                   - the device number.
* @param[in] physicalPortNum          - physical port number.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS appDemoDxChApLinkChangeTreat
(
    IN GT_U8   devNum,
    IN GT_U32  portNum
)
{
    GT_STATUS   rc;
    CPSS_DXCH_DETAILED_PORT_MAP_STC portMapShadow;
    GT_U32  macNum;

    if((0 == PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) || (1 == PRV_CPSS_SIP_6_CHECK_MAC(devNum)))
    {/* for now it's just for Aldrin2 and BC3*/
        return GT_OK;
    }

    rc = cpssDxChPortPhysicalPortDetailedMapGet(devNum, portNum, &portMapShadow);
    if (rc != GT_OK)
    {
       DBG_LOG(("cpssDxChPortPhysicalPortDetailedMapGet:error!devNum=%d,port=%d,rc=%d\n",
                devNum, portNum, rc, 4, 5, 6));
       return rc;
    }

    macNum = portMapShadow.portMap.macNum;
    /* Yakov - TBD - code below uses prv functions - must replace by right CPSS API calls */
    if (PRV_CPSS_PORT_CG_E == PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType)
    {
        GT_U32  regAddr;
        GT_U32  data;

        /* AP state machine sends this interrupt on end of treat of port up/down,
            so I'm sure I get here right value */
        regAddr = PRV_DXCH_REG1_UNIT_CG_CONVERTERS_MAC(devNum, macNum).CGMAConvertersResets;
        if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            rc = cpssDrvPpHwRegBitMaskRead(devNum, 0, regAddr, 0xffffffff, &data);
             if (rc != GT_OK)
             {
                DBG_LOG(("cpssDrvPpHwRegBitMaskRead: error, devNum=%d, port=%d, rc=%d\n",
                        devNum, portNum, rc, 4, 5, 6));
                return rc;
             }
             if (((data>>26)&0x1) == 0)
             {
                 /* update port DB with the default port type value in order to prevent using CG MAC
                    when it is disabled  */
                 PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_XG_E;

                 /* init CG port register database */
                 rc = prvCpssDxChCgPortDbInvalidate(devNum, macNum, GT_TRUE);
                 if (rc != GT_OK)
                 {
                     DBG_LOG(("appDemoDxChApLinkChangeTreat:prvCpssDxChCgPortDbInvalidate:rc=%d,portNum=%d,GT_TRUE\n",rc, portNum,3,4,5,6));
                     return rc;
                 }
             }
        }

    }
    else
    {
        CPSS_PORT_SPEED_ENT speed;
        CPSS_PORT_INTERFACE_MODE_ENT ifMode;
        GT_BOOL             isCgUnitInUse;

        /* Check current mode */
        rc = cpssDxChPortSpeedGet(devNum, portNum, &speed);
        if(rc != GT_OK)
        {
            DBG_LOG(("appDemoDxChApLinkChangeTreat:cpssDxChPortSpeedGet:rc=%d,portNum=%d\n",rc, portNum,3,4,5,6));
            return rc;
        }
        rc = cpssDxChPortInterfaceModeGet(devNum, portNum, &ifMode);
        if(rc != GT_OK)
        {
            DBG_LOG(("appDemoDxChApLinkChangeTreat:cpssDxChPortInterfaceModeGet:rc=%d,portNum=%d\n",rc, portNum,3,4,5,6));
            return rc;
        }

        rc = prvCpssDxChIsCgUnitInUse(devNum, portNum, ifMode, speed, &isCgUnitInUse);
        if(rc != GT_OK)
        {
            DBG_LOG(("appDemoDxChApLinkChangeTreat:prvCpssDxChIsCgUnitInUse:rc=%d,portNum=%d,ifMode=%d,speed=%d\n", rc, portNum,ifMode,speed,5,6));
            return rc;
        }

        if(isCgUnitInUse)
        {/* must restore CG DB, because if we here it means link was restored after fail */
            PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_CG_E;

            /* init CG port register database */
            rc = prvCpssDxChCgPortDbInvalidate(devNum, macNum, GT_FALSE);
            if (rc != GT_OK)
            {
                DBG_LOG(("appDemoDxChApLinkChangeTreat:prvCpssDxChCgPortDbInvalidate:rc=%d,portNum=%d,GT_FALSE\n",rc, portNum,3,4,5,6));
                return rc;
            }
        }
    }

    return GT_OK;
}

/**
* @internal appDemoDxChLinkChangeTreat function
* @endinternal
*
* @brief   Common part of link change treat for DXCH AP and non-AP ports
*
* @param[in] devNum                   - the device number.
* @param[in] physicalPortNum          - physical port number
* @param[in] apPort                   - whether or not the port is an AP port
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS appDemoDxChLinkChangeTreat
(
    IN GT_U8                   devNum,
    IN GT_U32                  physicalPortNum,
    IN GT_BOOL                 apPort
)
{
    GT_STATUS   rc;
    CPSS_DXCH_IMPLEMENT_WA_ENT   waArr[1];
    GT_U32      additionalInfoBmpArr[1];
    GT_BOOL     isLinkUp;

    /* For SIP6 devices all port functionally is completely new, so no need to implement legacy WAs */
    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_TRUE)
    {
        rc = sip6_linkChange(devNum, physicalPortNum);
        if (rc != GT_OK)
        {
            DBG_LOG(("sip6_linkChange: error, devNum=%d, port=%d, rc=%d\n",
                    devNum, physicalPortNum, rc, 4, 5, 6));
            return rc;
        }

        return GT_OK;
    }

    /* work arround appearing wrong MIB counters after port link down */
    waArr[0] = CPSS_DXCH_IMPLEMENT_WA_WRONG_MIB_COUNTERS_LINK_DOWN_E;
    additionalInfoBmpArr[0] = physicalPortNum;
    rc = cpssDxChHwPpImplementWaInit(devNum,1,&waArr[0], &additionalInfoBmpArr[0]);
    if (GT_OK != rc)
    {
        DBG_LOG(("cpssDxChHwPpImplementWaInit: wrong MIB counters after port link down error, devNum=%d, port=%d, rc=%d\n",
                devNum, physicalPortNum, rc, 4, 5, 6));
        return rc;
    }

    /* WA for fixing CRC errors when Auto-Neg is disabled on 10M/100M port speed */
    waArr[0] = CPSS_DXCH_IMPLEMENT_WA_100BASEX_AN_DISABLE_E;
    additionalInfoBmpArr[0] = physicalPortNum;
    rc = cpssDxChHwPpImplementWaInit(devNum, 1, &waArr[0], &additionalInfoBmpArr[0]);
    if (rc != GT_OK)
    {
        DBG_LOG(("cpssDxChHwPpImplementWaInit: CRC errors on 10M/100M port speed, devNum=%d, port=%d, rc=%d\n",
                 devNum, physicalPortNum, rc, 4, 5, 6));
        return rc;
    }

    /* TODO - currently we exit and return here for the sake of performing above errata,
       and next code is not relevant for AP port. The CPSS_SRVCPU_PORT_LINK_STATUS_CHANGED_E
       event needs to be used fully as the handler for AP ports link status change. */
    rc = cpssDxChPortLinkStatusGet(devNum, physicalPortNum, &isLinkUp);
    if (rc != GT_OK)
    {
        DBG_LOG(("cpssDxChPortLinkStatusGet: error, devNum=%d, port=%d, rc=%d\n", devNum, physicalPortNum, rc, 4, 5, 6));
        return rc;
    }

    if (apPort)
    {
        if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            if (!portMgr)
            {
                CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT portLinkStatusState;
                portLinkStatusState =  isLinkUp ?
                        CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E :
                        CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E;

                rc = cpssDxChBrgEgrFltPortLinkEnableSet(devNum, physicalPortNum,
                    portLinkStatusState);
            }
        }
        return GT_OK;
    }

    if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        if (!portMgr)
        {
            rc = sip5_20_linkChange(devNum, physicalPortNum);
            if (rc != GT_OK)
            {
                DBG_LOG(("sip5_20_linkChange: error, devNum=%d, port=%d, rc=%d\n",
                        devNum, physicalPortNum, rc, 4, 5, 6));
                return rc;
            }
        } else
        {
            if(printLinkChangeEnabled)
            {
                cpssOsPrintSync("LINK %s : devNum[%ld] (physical port[%2ld]) \n",
                                    (isLinkUp == GT_FALSE) ? "DOWN" : "UP  ",
                                                (GT_U32)devNum, physicalPortNum);
            }

        }
    }

    /* check if need to do extra for remote physical ports */
    if ((appDemoCallBackOnRemotePortsFunc != NULL) &&
        (GT_TRUE == prvCpssDxChPortRemotePortCheck(devNum, physicalPortNum)))
    {
        rc = appDemoCallBackOnRemotePortsFunc(devNum, physicalPortNum, isLinkUp);
        if (rc != GT_OK)
        {
            DBG_LOG(("appDemoCallBackOnRemotePortsFunc: error, devNum=%d, port=%d, linkUp=%d rc=%d\n",
                     devNum, physicalPortNum, isLinkUp, rc, 5, 6));
            return rc;
        }
    }

    return GT_OK;
}

#endif /*DXCH_CODE*/

GT_U32 mmpcsPrint = 0;

extern GT_U32 mmpcsPrintSet(GT_U32 enable)
{
    GT_U32 oldState;

    oldState = mmpcsPrint;
    mmpcsPrint = enable;

    return oldState;
}

/**
* @internal isRxSdmaEvent function
* @endinternal
*
* @brief   This routine handles RxSDMA events.
*
* @param[in] devNum                   - the device number.
* @param[in] uniEv                    - Unified event number
* @param[in] evExtData                - Unified event additional information
*
* @param[out] isRxSdmaPerQueuePtr      - (pointer to) indication
*                                       that the event is one of
*                                       the RX SDMA events
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS isRxSdmaEvent
(
    IN GT_U8                   devNum,
    IN GT_U32                  uniEv,
    IN GT_U32                  evExtData,
    OUT GT_BOOL                *isRxSdmaPerQueuePtr
)
{
    GT_U8       queue;/* the global queueId (0..127) */
    GT_BOOL     isRxSdmaPerQueue;
    RX_EV_HANDLER_EV_TYPE_ENT rxOperType = RX_EV_HANDLER_TYPE_RX_E;

    queue = evExtData; /* the global queueId (0..127) */

        /* range 0..7 */
    if(uniEv >= CPSS_PP_RX_BUFFER_QUEUE0_E && uniEv <= CPSS_PP_RX_BUFFER_QUEUE7_E)
    {
        isRxSdmaPerQueue           = GT_TRUE;
    }
    else
    if(uniEv >= CPSS_PP_RX_ERR_QUEUE0_E && uniEv <= CPSS_PP_RX_ERR_QUEUE7_E)
    {
        /* rx_buffer event doesn't arrive on every rx buffer event,
         * so theoretically under heavy load, there could be scenario,
         * when just rx_error events will signal that there are
         * descriptors to treat in chain
         */
        isRxSdmaPerQueue           = GT_TRUE;
        rxOperType                 = RX_EV_HANDLER_TYPE_RX_ERR_E;
    }
    else   /* range 8..15 */
    if(uniEv >= CPSS_PP_RX_BUFFER_QUEUE8_E && uniEv <= CPSS_PP_RX_BUFFER_QUEUE15_E)
    {
        isRxSdmaPerQueue           = GT_TRUE;
    }
    else
    if(uniEv >= CPSS_PP_RX_ERR_QUEUE8_E && uniEv <= CPSS_PP_RX_ERR_QUEUE15_E)
    {
        isRxSdmaPerQueue           = GT_TRUE;
        rxOperType                 = RX_EV_HANDLER_TYPE_RX_ERR_E;
    }
    else   /* range 16..23 */
    if(uniEv >= CPSS_PP_RX_BUFFER_QUEUE16_E && uniEv <= CPSS_PP_RX_BUFFER_QUEUE23_E)
    {
        isRxSdmaPerQueue           = GT_TRUE;
    }
    else
    if(uniEv >= CPSS_PP_RX_ERR_QUEUE16_E && uniEv <= CPSS_PP_RX_ERR_QUEUE23_E)
    {
        isRxSdmaPerQueue           = GT_TRUE;
        rxOperType                 = RX_EV_HANDLER_TYPE_RX_ERR_E;
    }
    else   /* range 24..31 */
    if(uniEv >= CPSS_PP_RX_BUFFER_QUEUE24_E && uniEv <= CPSS_PP_RX_BUFFER_QUEUE31_E)
    {
        isRxSdmaPerQueue           = GT_TRUE;
    }
    else
    if(uniEv >= CPSS_PP_RX_ERR_QUEUE24_E && uniEv <= CPSS_PP_RX_ERR_QUEUE31_E)
    {
        isRxSdmaPerQueue           = GT_TRUE;
        rxOperType                 = RX_EV_HANDLER_TYPE_RX_ERR_E;
    }
    else   /* range 32..127 */
    if(uniEv >= CPSS_PP_RX_BUFFER_QUEUE32_E && uniEv <= CPSS_PP_RX_BUFFER_QUEUE127_E)
    {
        isRxSdmaPerQueue           = GT_TRUE;
    }
    else
    if(uniEv >= CPSS_PP_RX_ERR_QUEUE32_E && uniEv <= CPSS_PP_RX_ERR_QUEUE127_E)
    {
        isRxSdmaPerQueue           = GT_TRUE;
        rxOperType                 = RX_EV_HANDLER_TYPE_RX_ERR_E;
    }
    else
    {
        *isRxSdmaPerQueuePtr = GT_FALSE;
        return GT_OK;
    }

    *isRxSdmaPerQueuePtr = isRxSdmaPerQueue;

    /* handling the 'rx buffer' or 'rx error' on the needed queue */
    return cpssEnRxPacketGet(rxOperType, devNum, queue);
}

static APP_DEMO_AU_MESSAGE_HANDLER_METHORD_ENT auMsgHandlerMethord = APP_DEMO_AU_MESSAGE_HANDLER_LOW_LEVEL_E;

#ifdef DXCH_CODE
static GT_U32                                  appDemoFdbManagerId;
static GT_BOOL                                 appDemoAutoAgingEn  = GT_FALSE;
static GT_U32                                  appDemoAutoAgingInterval;          /* Debug purpose */
static GT_U32                                  appDemoAutoAgingIterationInterval; /* Dynamically calculated */
static GT_U32                                  fdbManagerAutoAgingTaskCreated = 0;
static GT_U32                                  appDemoAutoAgingStatisticsArr[APP_DEMO_PRV_CPSS_DXCH_FDB_MANAGER_AUTO_AGING_API_STATISTICS___LAST___E];

/**
* @internal appDemoFdbManagerAutoAging_cleanup function
* @endinternal
*
* @brief   This routine clean info for the soon to be killed task 'appDemoFdbManagerAutoAging'.
*
*/
static void appDemoFdbManagerAutoAging_cleanup
(
    IN void*    cookiePtr
)
{
    cookiePtr = cookiePtr;/*unused*/
    fdbManagerAutoAgingTaskCreated = 0;
}

static GT_U32   debug_print_fdb_manager_aging = 0;
GT_STATUS debug_print_fdb_manager_aging_enable_set(IN GT_U32    enable)
{
    debug_print_fdb_manager_aging = enable;
    return GT_OK;
}

static GT_U32 appDemoFdbManagerAgingTaskNeedMessagesFromCpss = 0; /* set to 1 to work like SAI+Sonic */
GT_STATUS appDemoFdbManagerAgingTaskInfo
(
    IN GT_BOOL  needMessagesFromCpss
)
{
    appDemoFdbManagerAgingTaskNeedMessagesFromCpss = needMessagesFromCpss;
    return GT_OK;
}

/*******************************************************************************
* appDemoFdbManagerAutoAging
*
* DESCRIPTION:
*       This routine handles the auto aging process.
*       calls the CPSS auto aging API in regular interval.(to meet the configured aging time)
*
* INPUTS:
*       param - The process data structure.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static unsigned __TASKCONV appDemoFdbManagerAutoAging
(
    GT_VOID * param
)
{
    GT_STATUS                                                           rc;
    CPSS_DXCH_BRG_FDB_MANAGER_AGING_SCAN_PARAMS_STC                     ageScanParam;
    APP_DEMO_PRV_CPSS_DXCH_FDB_MANAGER_AUTO_AGING_API_STATISTICS_ENT    statisticsType = APP_DEMO_PRV_CPSS_DXCH_FDB_MANAGER_AUTO_AGING_API_STATISTICS_ERROR_FAIL_E;
    static CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC entriesAgedoutArray[1024];
    GT_U32                                                  entriesAgedoutNum,entryIndex;

    GT_UNUSED_PARAM(param);

    /* state that the task supports 'Graceful exit' */
    appDemoTaskSupportGracefulExit(appDemoFdbManagerAutoAging_cleanup,NULL/*cookie*/);
    /* state that the task should not generate info to the LOG , because it
       is doing 'polling' */
    appDemoForbidCpssLogOnSelfSet(1);
    /* state that the task should not generate info to the 'Register trace' , because it
       is doing 'polling' */
    appDemoForbidCpssRegisterTraceOnSelfSet(1);

    /* indicate that task start running */
    fdbManagerAutoAgingTaskCreated = 1;
    osTimerWkAfter(1);

    osMemSet(&ageScanParam, 0, sizeof(ageScanParam));
    ageScanParam.checkAgeMacUcEntries           = GT_TRUE;
    ageScanParam.checkAgeMacMcEntries           = GT_TRUE;
    ageScanParam.checkAgeIpMcEntries            = GT_TRUE;
    ageScanParam.checkAgeIpUcEntries            = GT_TRUE;
    ageScanParam.deleteAgeoutMacUcEportEntries  = GT_TRUE;
    ageScanParam.deleteAgeoutMacUcTrunkEntries  = GT_TRUE;
    ageScanParam.deleteAgeoutMacMcEntries       = GT_TRUE;
    ageScanParam.deleteAgeoutIpUcEntries        = GT_TRUE;
    ageScanParam.deleteAgeoutIpMcEntries        = GT_TRUE;



    while (1)
    {
        /* check if task need termination */
        appDemoTaskCheckIfNeedTermination();
        /* appDemoAutoAgingIterationInterval - can change dynamically by appDemoFdbManagerControlSet
         * appDemoFdbManagerId               - can change dynamically by appDemoFdbManagerGlobalIDSet
         */
        if(appDemoAutoAgingEn)
        {
            osTimerWkAfter(appDemoAutoAgingIterationInterval);
            /* check if task need termination */
            appDemoTaskCheckIfNeedTermination();

            if(appDemoFdbManagerAgingTaskNeedMessagesFromCpss)
            {
                /* support mode that the application MUST know about the aged entries */
                /* like in SAI that send those entries to the Sonic                   */
                rc = cpssDxChBrgFdbManagerAgingScan(appDemoFdbManagerId, &ageScanParam, entriesAgedoutArray, &entriesAgedoutNum);
            }
            else
            {
                /* support mode that the application not need any info about the aged entries */
                rc = cpssDxChBrgFdbManagerAgingScan(appDemoFdbManagerId, &ageScanParam, NULL, NULL);

                entriesAgedoutNum = 0;
            }
            switch(rc)
            {
                case GT_OK:
                    statisticsType = APP_DEMO_PRV_CPSS_DXCH_FDB_MANAGER_AUTO_AGING_API_STATISTICS_OK_E;
                    break;
                case GT_FAIL:
                    statisticsType = APP_DEMO_PRV_CPSS_DXCH_FDB_MANAGER_AUTO_AGING_API_STATISTICS_ERROR_FAIL_E;
                    break;
                case GT_HW_ERROR:
                    statisticsType = APP_DEMO_PRV_CPSS_DXCH_FDB_MANAGER_AUTO_AGING_API_STATISTICS_ERROR_HW_ERROR_E;
                    break;
                case GT_BAD_PARAM:
                case GT_BAD_PTR:
                case GT_NOT_APPLICABLE_DEVICE:
                    statisticsType = APP_DEMO_PRV_CPSS_DXCH_FDB_MANAGER_AUTO_AGING_API_STATISTICS_ERROR_INPUT_INVALID_E;
                    break;
                default:
                    DBG_LOG(("appDemoFdbManagerAutoAging: cpssDxChBrgFdbManagerAgingScan failed", 1, 2, 3, 4, 5, 6));
                    break;
            }
            appDemoAutoAgingStatisticsArr[statisticsType]+=1;

            if(debug_print_fdb_manager_aging)
            {
                for(entryIndex = 0 ; entryIndex < entriesAgedoutNum ; entryIndex++ )
                {
                    cpssOsPrintf("Aged out : cpss Entry:  mac[%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x] fid[%d] \n",
                        entriesAgedoutArray[entryIndex].entry.format.fdbEntryMacAddrFormat.macAddr.arEther[0],
                        entriesAgedoutArray[entryIndex].entry.format.fdbEntryMacAddrFormat.macAddr.arEther[1],
                        entriesAgedoutArray[entryIndex].entry.format.fdbEntryMacAddrFormat.macAddr.arEther[2],
                        entriesAgedoutArray[entryIndex].entry.format.fdbEntryMacAddrFormat.macAddr.arEther[3],
                        entriesAgedoutArray[entryIndex].entry.format.fdbEntryMacAddrFormat.macAddr.arEther[4],
                        entriesAgedoutArray[entryIndex].entry.format.fdbEntryMacAddrFormat.macAddr.arEther[5],
                        entriesAgedoutArray[entryIndex].entry.format.fdbEntryMacAddrFormat.fid);
                }
            }

        }
        else
        {
            /* Time to avoid cpu consumption by aging task */
            osTimerWkAfter(1000);
        }
    }
    return 0;
}

static GT_STATUS prvAppDemoFdbManagerAutoAgingEnable
(
    IN GT_U32       autoAgingInterval
)
{
    GT_TASK                                              autoAgeTid;         /* Task Id */
    GT_STATUS                                            rc;
    CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC               capacityInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC       entryAttrInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC               learningInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC                 lookupInfo;
    CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC                  agingInfo;
    GT_U32                                               totalAgeBinAllocated;

    /* Validity check for auto aging interval */
    if(autoAgingInterval == 0)
    {
        DBG_LOG(("appDemoFdbManagerControlSet: autoAgingInterval invalid %d", autoAgingInterval, 2, 3, 4, 5, 6));
        return GT_BAD_PARAM;
    }

    /* Calculate interval between CPSS age call API */
    rc = cpssDxChBrgFdbManagerConfigGet(appDemoFdbManagerId, &capacityInfo, &entryAttrInfo, &learningInfo, &lookupInfo, &agingInfo);
    if(rc != GT_OK)
    {
        DBG_LOG(("appDemoFdbManagerAutoAging: cpssDxChBrgFdbManagerConfigGet failed", 1, 2, 3, 4, 5, 6));
        return rc;
    }

    /* appDemoAutoAgingIterationInterval in milisec
     * autoAgingInterval                 is in sec
     **/
    totalAgeBinAllocated                = ROUNDUP_DIV_MAC(capacityInfo.maxTotalEntries, capacityInfo.maxEntriesPerAgingScan);
    appDemoAutoAgingIterationInterval   = ROUNDUP_DIV_MAC(autoAgingInterval * 1000, totalAgeBinAllocated);
    appDemoAutoAgingInterval            = autoAgingInterval; /* kept in global data, Just for debug purpose */

    if(fdbManagerAutoAgingTaskCreated == 1)
    {
        return GT_OK;
    }

    /* used cpssOsTaskCreate instead of osTaskCreate , to use : appDemoWrap_osTaskCreate */
    /* For FDB manager performance improvement - set aging thread lower priority, learning higher */
    rc = cpssOsTaskCreate("FdbManagerAutoAging",
            EV_HANDLER_MAX_PRIO - 5,
            _64KB,
            appDemoFdbManagerAutoAging,
            NULL,
            &autoAgeTid);
    if (rc != GT_OK)
    {
        return GT_FAIL;
    }

    while(fdbManagerAutoAgingTaskCreated == 0)
    {
        /* wait for indication that task created */
        osTimerWkAfter(1);
    }
    return GT_OK;
}

GT_STATUS appDemoFdbManagerAutoAgingStatisticsClear()
{
    osMemSet(appDemoAutoAgingStatisticsArr, 0, sizeof(appDemoAutoAgingStatisticsArr));
    return GT_OK;
}

GT_STATUS appDemoFdbManagerAutoAgingStatisticsGet
(
    OUT APP_DEMO_CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC *statistics
)
{
    APP_DEMO_PRV_CPSS_DXCH_FDB_MANAGER_AUTO_AGING_API_STATISTICS_ENT    statisticType;
    GT_U32                                                              *statisticValuePtr;

    if(statistics == NULL)
    {
        return GT_BAD_PARAM;
    }

    osMemSet(statistics, 0, sizeof(APP_DEMO_CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC));

    /* Auto Aging appDemo task statistics */
    statisticType = APP_DEMO_PRV_CPSS_DXCH_FDB_MANAGER_AUTO_AGING_API_STATISTICS_OK_E;
    while(statisticType < APP_DEMO_PRV_CPSS_DXCH_FDB_MANAGER_AUTO_AGING_API_STATISTICS___LAST___E)
    {
        statisticValuePtr = NULL;
        switch(statisticType)
        {
            case APP_DEMO_PRV_CPSS_DXCH_FDB_MANAGER_AUTO_AGING_API_STATISTICS_OK_E:
                statisticValuePtr = &statistics->autoAgingOk;
                break;
            case APP_DEMO_PRV_CPSS_DXCH_FDB_MANAGER_AUTO_AGING_API_STATISTICS_ERROR_INPUT_INVALID_E:
                statisticValuePtr = &statistics->autoAgingErrorInputInvalid;
                break;
            case APP_DEMO_PRV_CPSS_DXCH_FDB_MANAGER_AUTO_AGING_API_STATISTICS_ERROR_FAIL_E:
                statisticValuePtr = &statistics->autoAgingErrorFail;
                break;
            case APP_DEMO_PRV_CPSS_DXCH_FDB_MANAGER_AUTO_AGING_API_STATISTICS_ERROR_HW_ERROR_E:
                statisticValuePtr = &statistics->autoAgingErrorHwError;
                break;
            default:
                break;
        }
        if(statisticValuePtr)
        {
            *statisticValuePtr = appDemoAutoAgingStatisticsArr[statisticType];
        }
        statisticType++;
    }
    return GT_OK;
}

GT_STATUS appDemoFdbManagerGlobalIDSet
(
    IN GT_U32 fdbManagerId
)
{
    if(fdbManagerId>=32)
    {
        return GT_BAD_PARAM;
    }
    appDemoFdbManagerId = fdbManagerId;
    return GT_OK;
}

GT_STATUS appDemoFdbManagerAuMsgHandlerMethod
(
    IN APP_DEMO_AU_MESSAGE_HANDLER_METHORD_ENT auMsgHandler
)
{
    GT_U32   dev;

    switch(auMsgHandler)
    {
        case APP_DEMO_AU_MESSAGE_HANDLER_LOW_LEVEL_E:
            break;
        case APP_DEMO_AU_MESSAGE_HANDLER_FDB_MANAGER_E:
        case APP_DEMO_AU_MESSAGE_HANDLER_FDB_MANAGER_WITH_CUCKOO_E:
            /* FDB Manager support AU queue only.
               Disable AU FIFO related interrupt to avoid redundant actions. */
            for(dev = SYSTEM_DEV_NUM_MAC(0); dev < SYSTEM_DEV_NUM_MAC(appDemoPpConfigDevAmount); dev++)
            {
                if(GT_FALSE == appDemoPpConfigList[dev].valid)
                {
                    continue;
                }

                (GT_VOID)cpssEventDeviceMaskSet(appDemoPpConfigList[dev].devNum,
                                                CPSS_PP_MAC_MESSAGE_TO_CPU_READY_E,
                                                CPSS_EVENT_MASK_E);
            }
            break;
        case APP_DEMO_AU_MESSAGE_HANDLER_DISABLED_E:
            break;
        default:
            DBG_LOG(("prvAppDemoFdbManagerAuMsgHandlerMethod - Invalid methord", 1, 2, 3, 4, 5, 6));
    }

    auMsgHandlerMethord = auMsgHandler;

    return GT_OK;
}

GT_STATUS appDemoFdbManagerControlSet
(
    IN APP_DEMO_AU_MESSAGE_HANDLER_METHORD_ENT  auMsgHandler,
    IN GT_BOOL                                  autoAgingEn,
    IN GT_U32                                   autoAgingInterval
)
{
    GT_STATUS   rc;

    switch(auMsgHandler)
    {
        case APP_DEMO_AU_MESSAGE_HANDLER_LOW_LEVEL_E:
            appDemoAutoAgingEn = GT_FALSE;
            break;
        case APP_DEMO_AU_MESSAGE_HANDLER_FDB_MANAGER_E:
        case APP_DEMO_AU_MESSAGE_HANDLER_FDB_MANAGER_WITH_CUCKOO_E:
            if(autoAgingEn == GT_TRUE)
            {
                rc = prvAppDemoFdbManagerAutoAgingEnable(autoAgingInterval);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
            appDemoAutoAgingEn = autoAgingEn;
            break;
        case APP_DEMO_AU_MESSAGE_HANDLER_DISABLED_E:
            appDemoAutoAgingEn = GT_FALSE;
            break;
        default:
            DBG_LOG(("prvAppDemoFdbManagerAuMsgHandlerMethod - Invalid methord", 1, 2, 3, 4, 5, 6));
            return GT_BAD_PARAM;
    }

    return appDemoFdbManagerAuMsgHandlerMethod(auMsgHandler);
}

/**
* @internal appDemoDxChMacSecEventsTreat function
* @endinternal
*
* @brief   This routing handles all MACsec events
*
* @param[in] devNum                   - the device number.
* @param[in] uniEv                    - Unified event number
* @param[in] evExtData                - Unified event additional information
*
* @retval GT_OK                    - on success,
*/
static GT_STATUS appDemoDxChMacSecEventsTreat
(
    IN GT_U8   devNum,
    IN GT_U32  uniEv,
    IN GT_U32  evExtData
)
{
    switch(uniEv)
    {
        case CPSS_PP_MACSEC_SA_EXPIRED_E:
        case CPSS_PP_MACSEC_SA_PN_FULL_E:
        case CPSS_PP_MACSEC_EGRESS_SEQ_NUM_ROLLOVER_E:
            if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) && prvAppDemoMacSecEventsDump)
            {
                static GT_CHAR * uniEvName[CPSS_UNI_EVENT_COUNT_E] = {UNI_EV_NAME};
                cpssOsPrintf("MACsec Unified event = %s\n", uniEvName[uniEv]);
                cpssOsPrintf("Interrupt info : devNum[%d], uniEv=[%d], direction=[%s], DP number=[%d]\n",
                             devNum, uniEv, (evExtData & 0x1) ? "Ingress" : "Egress", (evExtData >> 1) & 0x1);
            }
            break;
        case CPSS_PP_MACSEC_STATISTICS_SUMMARY_E:
            if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) && prvAppDemoDataIntegrityEventDump)
            {
                static GT_CHAR * uniEvName[CPSS_UNI_EVENT_COUNT_E] = {UNI_EV_NAME};
                GT_U8 source;
                source = (evExtData >> 8) & 0x7;
                cpssOsPrintf("MACsec Unified event = %s\n", uniEvName[uniEv]);
                cpssOsPrintf("Interrupt info : devNum[%d], uniEv=[%d], direction=[%s], DP number=[%d], source=[%s]\n",
                            devNum, uniEv, (evExtData & 0x1) ? "Ingress" : "Egress", (evExtData >> 1) & 0x1,
                            (source == 0) ? "SA expired statistics"     :
                            (source == 1) ? "SecY vPort statistics"     :
                            (source == 2) ? "IFC0 per vPort statistics" :
                            (source == 3) ? "IFC1 per vPort statistics" :
                            (source == 4) ? "RxCAM statistics"          :
                            (source == 5) ? "TCAM statistics"           :
                            (source == 6) ? "Port statistics"           :
                            "Source undefined");
            }
            break;
        case CPSS_PP_MACSEC_TRANSFORM_ENGINE_ERR_E:
            if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) && prvAppDemoDataIntegrityEventDump)
            {
                static GT_CHAR * uniEvName[CPSS_UNI_EVENT_COUNT_E] = {UNI_EV_NAME};
                cpssOsPrintf("MACsec Unified event = %s\n", uniEvName[uniEv]);
                cpssOsPrintf("Interrupt info : devNum[%d], uniEv=[%d], direction=[%s], DP number=[%d] error index=[%d]\n",
                            devNum, uniEv, (evExtData & 0x1) ? "Ingress" : "Egress", (evExtData >> 1) & 0x1, (evExtData >> 8) & 0xF);
            }
            break;
        default :
            break;
    }
    return GT_OK;
}

/**
* @internal appDemoDxChFecEventCounterIncrement function
* @endinternal
*
* @brief   Increment the FEC event counter per virtual lane according to its FEC type.
*
* @param[in] devNum                 - device number
* @param[in] evExtData              - unified event additional information
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - in case of bad parameters
* @retval GT_OUT_OF_RANGE          - exceeds array's bounds
*/
static GT_STATUS appDemoDxChFecEventCounterIncrement
(
    IN GT_U8                    devNum,
    IN GT_U32                   evExtData
)
{
    GT_U32 fecLane, fecLaneIndex;

    if(devNum >= MAX_NUM_DEVICES)
    {
        /* check device out of range */
        return GT_BAD_PARAM;
    }

    if(uniFecEventCounters[devNum] == NULL)
    {
        /* Allocate memory upon the first FEC event has received */
        uniFecEventCounters[devNum] =  (FEC_COUNTERS_STC *)osMalloc(sizeof(FEC_COUNTERS_STC));

        if(uniFecEventCounters[devNum] == NULL)
            return GT_OUT_OF_CPU_MEM;

        osMemSet(uniFecEventCounters[devNum], 0, sizeof(FEC_COUNTERS_STC));
    }

    fecLane = evExtData & 0xFFFF;

    if (fecLane % 2 != 0)
    {
        /* Supports only even virtual lanes */
        return GT_OK;
    }

    fecLaneIndex = fecLane / 2;
    if (fecLaneIndex >= FEC_COUNTERS_MAX_CNS)
    {
        return GT_OUT_OF_RANGE;
    }

    osMutexLock(appUtilsEventCounterUpdateLockMtx);

    switch (evExtData >> 16)
    {
        case 0:
            uniFecEventCounters[devNum]->fecCeEventCounters[fecLaneIndex]++;
            if (wrapCpssTraceEvents)
            {
                osPrintf(" FEC EC Events Count %d, FEC Lane[%d]\n", uniFecEventCounters[devNum]->fecCeEventCounters[fecLaneIndex], fecLane);
            }
            break;
        case 1:
            uniFecEventCounters[devNum]->fecNceEventCounters[fecLaneIndex]++;
            if (wrapCpssTraceEvents)
            {
                osPrintf(" FEC NEC Event Count %d, FEC Lane[%d]\n", uniFecEventCounters[devNum]->fecNceEventCounters[fecLaneIndex], fecLane);
            }
            break;
        default:
            osMutexUnlock(appUtilsEventCounterUpdateLockMtx);
            return GT_BAD_PARAM;
    }


    osMutexUnlock(appUtilsEventCounterUpdateLockMtx);

    return GT_OK;
}

#endif


/**
* @internal appDemoEnPpEvTreat function
* @endinternal
*
* @brief   This routine handles PP events.
*
* @param[in] devNum                   - the device number.
* @param[in] uniEv                    - Unified event number
* @param[in] evExtData                - Unified event additional information
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS appDemoEnPpEvTreat
(
    GT_U8                   devNum,
    GT_U32                  uniEv,
    GT_U32                  evExtData
)
{
    GT_STATUS   rc = GT_OK;
    GT_BOOL     isRxSdmaPerQueue;
    GT_U32      portNum;
    GT_U32      physicalPortNum = 0;
    GT_BOOL     hcdFound;
    CPSS_PORT_SPEED_ENT apSpeed;
    CPSS_PORT_INTERFACE_MODE_ENT apIfMode;
    CPSS_PP_FAMILY_TYPE_ENT      devFamily;

#ifdef DXCH_CODE
    CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_SCAN_PARAMS_STC data;
    GT_U32                       value;
    GT_BOOL     linkUp = GT_FALSE;
    CPSS_DXCH_PORT_AP_STATUS_STC         apStatusDx;
    CPSS_DXCH_UBURST_INFO_STC            uBurstData[MAX_UBURST_EVENTS_CHUNK];
    GT_U32                               uBurstEventsSize ;
#ifndef ASIC_SIMULATION
    CPSS_DXCH_IMPLEMENT_WA_ENT   waArr[1];
    GT_U32  additionalInfoBmpArr[1];
#endif
    GT_BOOL apEnable = GT_FALSE;
    CPSS_PM_PORT_PARAMS_STC portParams;
#endif
#ifdef PX_CODE
    CPSS_PX_PORT_AP_STATUS_STC  apStatusPx;
    GT_BOOL apEnablePx;
    IN  CPSS_PORT_MANAGER_STC   portManagerEvent;
#endif

    if(devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        return GT_BAD_PARAM;
    }


    /* NOTE: check the RX SDMA events that hold 256 events that we not in the
       'switch case' */
    rc = isRxSdmaEvent(devNum,uniEv,evExtData,&isRxSdmaPerQueue);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(isRxSdmaPerQueue == GT_TRUE)
    {
        /* already handled in isRxSdmaEvent(...) */
        return GT_OK;
    }

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
        /* device was removed */
        return GT_OK;
    }
    devFamily = PRV_CPSS_PP_MAC(devNum)->devFamily;
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);


    switch (uniEv)
    {
         case CPSS_PP_CM3_WD_E:
            cpssOsPrintf("\nCM3 Watchdog interrupt asserted from MG%d!\n", evExtData);
            break;
        case CPSS_PP_CM3_DOORBELL_E:
            cpssOsPrintf("\nCM3 Doorbell interrupt asserted from MG%d!\n", evExtData);
            break;
        case CPSS_PP_MAC_MESSAGE_TO_CPU_READY_E:
        case CPSS_PP_EB_AUQ_PENDING_E:

            if(allowProcessingOfAuqMessages == GT_TRUE)
            {
                switch(auMsgHandlerMethord)
                {
                    case APP_DEMO_AU_MESSAGE_HANDLER_LOW_LEVEL_E:
#ifdef DEBUG_LINUX_ISR_LOCK
                        /* make the system busy while test of UT(cpssDxChCfgReNumberDevNum)
                           may change the DB of CPSS */

                        rc = 0;
                        while(1)
                        {
                            GT_32 dummy;
                            /*lock*/
                            osSetIntLockUnlock(0,&dummy);
                            rc = cpssEnGenAuMsgHandle(auEventHandlerHnd,devNum, evExtData);
                            /*unlock*/
                            osSetIntLockUnlock(1,&dummy);
                        };
#endif /*DEBUG_LINUX_ISR_LOCK*/

                        rc = cpssEnGenAuMsgHandle(auEventHandlerHnd,devNum, evExtData);
                        if (GT_OK != rc)
                        {
                            DBG_LOG(("cpssEnAuMsgGet: error, devNum=%d, uniEv=%d, rc=%d\n", devNum, uniEv, rc, 4, 5, 6));
                        }
                        break;
#ifdef DXCH_CODE
                    case APP_DEMO_AU_MESSAGE_HANDLER_FDB_MANAGER_E:
                        data.addNewMacUcEntries         = GT_TRUE;
                        data.updateMovedMacUcEntries    = (PRV_CPSS_SIP_5_CHECK_MAC(devNum))?GT_TRUE:GT_FALSE;
                        data.addWithRehashEnable        = GT_FALSE;
                        rc = cpssEnFdbManagerAuMsgHandle(appDemoFdbManagerId, &data);
                        if (rc != GT_OK)
                        {
                            DBG_LOG(("cpssEnFdbManagerAuMsgHandle: error, devNum=%d, uniEv=%d, rc=%d\n", devNum, uniEv, rc, 4, 5, 6));
                        }
                        break;
                    case APP_DEMO_AU_MESSAGE_HANDLER_FDB_MANAGER_WITH_CUCKOO_E: /* TODO - Rehash is Yet to impliment */
                        data.addNewMacUcEntries         = GT_TRUE;
                        data.updateMovedMacUcEntries    = GT_TRUE;
                        data.addWithRehashEnable        = GT_TRUE;
                        rc = cpssEnFdbManagerAuMsgHandle(appDemoFdbManagerId, &data);
                        if (rc != GT_OK)
                        {
                            DBG_LOG(("cpssEnFdbManagerAuMsgHandle: error, devNum=%d, uniEv=%d, rc=%d\n", devNum, uniEv, rc, 4, 5, 6));
                        }
                        break;
#endif
                    case APP_DEMO_AU_MESSAGE_HANDLER_DISABLED_E:
                        break;
                    default:
                        DBG_LOG(("cpssEnAuMsgGet: error AU Message Handler %d", auMsgHandlerMethord, 2, 3, 4, 5, 6));
                }
            }

            break;
        case CPSS_PP_EB_FUQ_PENDING_E:
            /* FUQ messages are kept in the FUQ until explicitly called by the
                GaltisWrapper or other test case */
            /* the event handler is not processing those messages */
            break;

        /* Doorbell interrupts - start*/
        /*This interrupt is relevant only for App that decide to control port enable (and not let srvCpu to do it).*/
        case CPSS_SRVCPU_PORT_LINK_STATUS_CHANGED_E:

            portNum = (GT_U16)evExtData;
            /*osPrintf("CPSS_SRVCPU_PORT_LINK_STATUS_CHANGED_E - falcon doorbellsupport:rc=%d,portNum=%d\n",rc, portNum);*/
#if 0
            /*convert from MAC port number used by the SrvCPU to physical port number used by host cpu*/
#ifdef DXCH_CODE
            if (CPSS_IS_DXCH_FAMILY_MAC(devFamily))
            {
                rc = cpssDxChPortPhysicalPortMapReverseMappingGet(0,CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,portNum, &physicalPortNum);
                if(rc != GT_OK)
                {
                    osPrintf("CPSS_SRVCPU_PORT_LINK_STATUS_CHANGED_E - cpssDxChPortPhysicalPortMapReverseMappingGet:rc=%d,portNum=%d\n",rc, portNum);
                    return rc;
                }
            }
#endif
#ifdef PX_CODE
            if (CPSS_IS_PX_FAMILY_MAC(devFamily))
            {
                rc = cpssPxPortPhysicalPortMapReverseMappingGet(0, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E, portNum, &physicalPortNum);
                if(rc != GT_OK)
                {
                    osPrintf("CPSS_SRVCPU_PORT_LINK_STATUS_CHANGED_E - cpssPxPortPhysicalPortMapReverseMappingGet:rc=%d,portNum=%d\n",rc, portNum);
                    return rc;
                }
            }
#endif
            if ( appDemoPortInitSequenceIsPortApplicable(devNum, physicalPortNum) )
            {
                if ( !disableAutoPortMgr )
                {
                    appDemoPortInitSeqLinkStatusChangedStage(devNum, physicalPortNum);
                }
            }
#endif

 /* because now we miss sometimes this interrupt from unknown reason treat of AP link change done upon
        regular link change interrupt */

#if 0
#ifdef DXCH_CODE
            rc = appDemoDxChApLinkChangeTreat(devNum, physicalPortNum);
            if(rc != GT_OK)
            {
                DBG_LOG(("CPSS_SRVCPU_PORT_LINK_STATUS_CHANGED_E - appDemoDxChApLinkChangeTreat:rc=%d,portNum=%d\n",rc, portNum,3,4,5,6));
                return rc;
            }

            rc = appDemoDxChLinkChangeTreat(devNum, physicalPortNum, GT_TRUE /*in this case we are handling AP port events*/);
            if(rc != GT_OK)
            {
                DBG_LOG(("CPSS_SRVCPU_PORT_LINK_STATUS_CHANGED_E - appDemoDxChLinkChangeTreat:rc=%d,portNum=%d\n",rc, portNum,3,4,5,6));
                return rc;
            }
#endif
#endif /* if 0 */
            break;

        case CPSS_SRVCPU_PORT_802_3_AP_E:

            portNum = (GT_U16)evExtData;

            /*osPrintf("CPSS_SRVCPU_PORT_802_3_AP_E portNum=%d \n",portNum);*/
#ifdef DXCH_CODE
            if (CPSS_IS_DXCH_FAMILY_MAC(devFamily))
            {
                rc = cpssDxChPortPhysicalPortMapReverseMappingGet(devNum, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,portNum, &physicalPortNum);
                if(rc != GT_OK)
                {
                    osPrintf("CPSS_SRVCPU_PORT_LINK_STATUS_CHANGED_E - cpssDxChPortPhysicalPortMapReverseMappingGet:rc=%d,portNum=%d\n",rc, portNum);
                    return rc;
                }
            }
#endif
#ifdef PX_CODE
            if (CPSS_IS_PX_FAMILY_MAC(devFamily))
            {
                rc = cpssPxPortPhysicalPortMapReverseMappingGet(devNum, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E, portNum, &physicalPortNum);
                if(rc != GT_OK)
                {
                    osPrintf("CPSS_SRVCPU_PORT_LINK_STATUS_CHANGED_E - cpssPxPortPhysicalPortMapReverseMappingGet:rc=%d,portNum=%d\n",rc, portNum);
                    return rc;
                }
            }
#endif

            if (portMgr && !disableAutoPortMgr)
            {
                appDemoPortInitSeqPortStatusChangeSignal(devNum, physicalPortNum, CPSS_SRVCPU_PORT_802_3_AP_E);
                /* pizza allocation done inside port manager, so no need to continue */
                break;
            }

            /* init values to avoid warnings */
            hcdFound = GT_FALSE;
            apSpeed = CPSS_PORT_SPEED_NA_E;
            apIfMode = CPSS_PORT_INTERFACE_MODE_NA_E;

            /* query resolution results */
#ifdef DXCH_CODE
            if (CPSS_IS_DXCH_FAMILY_MAC(devFamily))
            {
                rc = cpssDxChPortApPortStatusGet(devNum,physicalPortNum,&apStatusDx);
                if(rc != GT_OK)
                {
                    osPrintf("CPSS_SRVCPU_PORT_802_3_AP_E - cpssDxChPortApPortStatusGet:rc=%d,portNum=%d\n",rc, physicalPortNum);
                    return rc;
                }
                hcdFound = apStatusDx.hcdFound;
                apSpeed = apStatusDx.portMode.speed;
                apIfMode = apStatusDx.portMode.ifMode;
            }
#endif
#ifdef PX_CODE
            if (CPSS_IS_PX_FAMILY_MAC(devFamily))
            {
                rc = cpssPxPortApPortStatusGet(devNum,physicalPortNum,&apStatusPx);
                if(rc != GT_OK)
                {
                    osPrintf("CPSS_SRVCPU_PORT_802_3_AP_E - cpssPxPortApPortStatusGet:rc=%d,portNum=%d\n",rc, physicalPortNum);
                    return rc;
                }
                hcdFound = apStatusPx.hcdFound;
                apSpeed = apStatusPx.portMode.speed;
                apIfMode = apStatusPx.portMode.ifMode;
            }
#endif
            /* resolution found - allocate pizza resources*/
            if(hcdFound)
            {
                CPSS_PORT_SPEED_ENT speed;
                CPSS_PORT_INTERFACE_MODE_ENT ifMode;
                CPSS_PORTS_BMP_STC portsBmp;
                CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
                CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp,physicalPortNum);

                /* Check current spped */
#ifdef DXCH_CODE
                if (CPSS_IS_DXCH_FAMILY_MAC(devFamily))
                {
                    rc = cpssDxChPortSpeedGet(devNum, physicalPortNum, &speed);
                    if(rc != GT_OK)
                    {
                        osPrintf("CPSS_SRVCPU_PORT_802_3_AP_E - cpssDxChPortSpeedGet:rc=%d,portNum=%d\n",rc, physicalPortNum);
                        return rc;
                    }
                    else
                    {
                        /*if speed <= CPSS_PORT_SPEED_1000_E we get the value from HW; at this stage speed is not always update correctly;
                          assume pizza allocation cant be less than 1G*/
                        if(speed <= CPSS_PORT_SPEED_1000_E)
                        {
                            speed = CPSS_PORT_SPEED_1000_E;
                        }
                    }
                }
#endif
#ifdef PX_CODE
                if (CPSS_IS_PX_FAMILY_MAC(devFamily))
                {
                    rc = cpssPxPortSpeedGet(devNum, physicalPortNum, &speed);
                    if(rc != GT_OK)
                    {
                        osPrintf("CPSS_SRVCPU_PORT_802_3_AP_E - cpssPxPortSpeedGet:rc=%d,portNum=%d\n",rc, physicalPortNum);
                        return rc;
                    }
                    else
                    {
                        /*if speed <= CPSS_PORT_SPEED_1000_E we get the value from HW; at this stage speed is not always update correctly;
                          assume pizza allocation cant be less than 1G*/
                        if(speed <= CPSS_PORT_SPEED_1000_E)
                        {
                            speed = CPSS_PORT_SPEED_1000_E;
                        }
                    }
                }
#endif
                /* check if port was already deleted */
                if (speed == CPSS_PORT_SPEED_NA_E)
                {
                    return GT_OK;
                }

                /*if pizza already configured, Release it if not the same speed */
                if((speed != CPSS_PORT_SPEED_NA_HCD_E) && ((speed != apSpeed) && (CPSS_PORT_SPEED_20000_E != speed)))
                {
                     /* Release pizza resources */
#ifdef DXCH_CODE
                    if (CPSS_IS_DXCH_FAMILY_MAC(devFamily))
                    {
                        rc = cpssDxChPortInterfaceModeGet(devNum, physicalPortNum, &ifMode);
                        if(rc != GT_OK)
                        {
                            osPrintf("CPSS_SRVCPU_PORT_802_3_AP_E - cpssDxChPortInterfaceModeGet:rc=%d,portNum=%d\n",rc, physicalPortNum);
                            return rc;
                        }

                        rc = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_FALSE, ifMode + CPSS_PORT_INTERFACE_MODE_NA_E, speed + CPSS_PORT_SPEED_NA_E);
                        if(rc != GT_OK)
                        {
                            osPrintf("CPSS_SRVCPU_PORT_802_3_AP_E - cpssDxChPortModeSpeedSet:rc=%d,portNum=%d\n", rc, physicalPortNum);
                            return rc;
                        }
                    }
#endif
#ifdef PX_CODE
                    if (CPSS_IS_PX_FAMILY_MAC(devFamily))
                    {
                        rc = cpssPxPortInterfaceModeGet(devNum, physicalPortNum, &ifMode);
                        if(rc != GT_OK)
                        {
                            osPrintf("CPSS_SRVCPU_PORT_802_3_AP_E - cpssPxPortInterfaceModeGet:rc=%d,portNum=%d\n",rc, physicalPortNum);
                            return rc;
                        }
                        rc = cpssPxPortModeSpeedSet(devNum, &portsBmp, GT_FALSE, ifMode + CPSS_PORT_INTERFACE_MODE_NA_E, speed + CPSS_PORT_SPEED_NA_E);
                        if(rc != GT_OK)
                        {
                            osPrintf("CPSS_SRVCPU_PORT_802_3_AP_E - cpssPxPortModeSpeedSet:rc=%d,portNum=%d\n", rc, physicalPortNum);
                            return rc;
                        }
                    }
#endif
                    speed = CPSS_PORT_SPEED_NA_HCD_E;
                }

                /* allocate pizza resources only if not configured earlier or speed changed */
                if(speed == CPSS_PORT_SPEED_NA_HCD_E)
                {
#ifdef DXCH_CODE
                    if (CPSS_IS_DXCH_FAMILY_MAC(devFamily))
                    {
                        rc = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_TRUE, apIfMode + CPSS_PORT_INTERFACE_MODE_NA_E, apSpeed + CPSS_PORT_SPEED_NA_E);
                        if(rc != GT_OK)
                        {
                            osPrintf("CPSS_SRVCPU_PORT_802_3_AP_E - cpssDxChPortModeSpeedSet:rc=%d,portNum=%d\n", rc, physicalPortNum);
                            return rc;
                        }
                    }
#endif
#ifdef PX_CODE
                    if (CPSS_IS_PX_FAMILY_MAC(devFamily))
                    {
                        rc = cpssPxPortModeSpeedSet(devNum, &portsBmp, GT_TRUE, apIfMode + CPSS_PORT_INTERFACE_MODE_NA_E, apSpeed + CPSS_PORT_SPEED_NA_E);
                        if(rc != GT_OK)
                        {
                            osPrintf("CPSS_SRVCPU_PORT_802_3_AP_E - cpssPxPortModeSpeedSet:rc=%d,portNum=%d\n", rc, physicalPortNum);
                            return rc;
                        }
                    }
#endif
                }
            }
            else
            {
                DBG_LOG(("CPSS_SRVCPU_PORT_802_3_AP_E - portNum=%d, no resolution\n",physicalPortNum,2,3,4,5,6));
                return GT_FAIL;
            }
            break;

#ifdef PX_CODE
    case CPSS_SRVCPU_PORT_REMOTE_FAULT_TX_CHANGE_E:
            if (portMgr)
            {
                portNum = (GT_U8)evExtData;
                portManagerEvent.portEvent = CPSS_PORT_MANAGER_EVENT_REMOTE_FAULT_TX_CHANGE_E;
                cpssPxPortManagerEventSet(devNum, portNum, &portManagerEvent);
            }
            break;
#endif
        case CPSS_SRVCPU_PORT_AP_DISABLE_E:

            portNum = (GT_U16)evExtData;

#ifdef DXCH_CODE
            if (CPSS_IS_DXCH_FAMILY_MAC(devFamily))
            {
                rc = cpssDxChPortPhysicalPortMapReverseMappingGet(devNum, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,portNum, &physicalPortNum);
                if(rc != GT_OK)
                {
                    osPrintf("CPSS_SRVCPU_PORT_LINK_STATUS_CHANGED_E - cpssDxChPortPhysicalPortMapReverseMappingGet:rc=%d,portNum=%d\n",rc, portNum);
                    return rc;
                }
            }
#endif
#ifdef PX_CODE
            if (CPSS_IS_PX_FAMILY_MAC(devFamily))
            {
                rc = cpssPxPortPhysicalPortMapReverseMappingGet(devNum, CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E, portNum, &physicalPortNum);
                if(rc != GT_OK)
                {
                    osPrintf("CPSS_SRVCPU_PORT_LINK_STATUS_CHANGED_E - cpssPxPortPhysicalPortMapReverseMappingGet:rc=%d,portNum=%d\n",rc, portNum);
                    return rc;
                }
            }
#endif

            /*osPrintf("CPSS_SRVCPU_PORT_AP_DISABLE mac %d portNum=%d \n",portNum, physicalPortNum);*/
            if (portMgr && !disableAutoPortMgr)
            {
                appDemoPortInitSeqPortStatusChangeSignal(devNum, physicalPortNum, CPSS_SRVCPU_PORT_AP_DISABLE_E);
                /* pizza allocation done inside port manager, so no need to continue */
                break;
            }
            /*TODO add support for non PM port*/
            break;
        /* Doorbell interrupts - end*/

        case CPSS_PP_PORT_LINK_STATUS_CHANGED_E:
            portNum = evExtData;

#ifdef PTP_PLUGIN_SUPPORT
        if (linkChngFuncPtr)
        {
              linkChngFuncPtr(devNum, evExtData);
        }
#endif

            if(portNum & BIT_31)/* see logic in internal_drvEventExtDataConvert(...) */
            {
                osPrintf("CPSS_PP_PORT_LINK_STATUS_CHANGED_E : got indication from MAC[%d] that is not mapped to physical port \n",
                    portNum & ~ BIT_31);
                /* The CPSS APIs not support this port ! it is unmapped MAC to physical port */
                break;
            }

            /* osPrintf("CPSS_PP_PORT_LINK_STATUS_CHANGED_E port %d",evExtData); */
            /* New port init sequence stage */

            if (portMgr && !disableAutoPortMgr)
            {
                appDemoPortInitSeqLinkStatusChangedStage(devNum, portNum);
            }
#ifdef DXCH_CODE
            if (CPSS_IS_DXCH_FAMILY_MAC(devFamily))
            {
                if(portMgr)
                {
                    rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParams);
                    if ((rc != GT_OK) && (rc != GT_NOT_INITIALIZED))
                    {
                        DBG_LOG(("CPSS_PP_PORT_LINK_STATUS_CHANGED_E - cpssDxChPortApPortConfigGet:rc=%d,portNum=%d\n",rc, portNum,3,4,5,6));
                        return rc;
                    }
                    apEnable = (portParams.portType == CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E);
                }
                else
                {
                    rc = cpssDxChPortApPortEnableGet(devNum, portNum, &apEnable);
                    if(rc != GT_OK)
                    {
                        DBG_LOG(("CPSS_PP_PORT_LINK_STATUS_CHANGED_E - cpssDxChPortApPortConfigGet:rc=%d,portNum=%d\n",rc, portNum,3,4,5,6));
                        return rc;
                    }
                }
                if (portMgr && !disableAutoPortMgr)
                {
                    break;
                }

                if ((apEnable != GT_FALSE) && PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
                {
                    /*If AP and XCAT3, doorbell interrupts is not supported, and enable/disable port is not done on SrvCpu level*/
                    rc = cpssDxChPortLinkStatusGet(devNum, portNum, &linkUp);
                    if (rc != GT_OK)
                    {
                        DBG_LOG(("cpssDxChPortLinkStatusGet:rc=%d,portNum=%d\n",rc, portNum,3,4,5,6));
                        return rc;
                    }

                    rc = cpssDxChPortEnableSet(devNum, portNum, linkUp);
                    if(rc != GT_OK)
                    {
                        DBG_LOG(("cpssDxChPortEnableSet:rc=%d,portNum=%d,linkUp=%d\n",rc, portNum,linkUp,4,5,6));
                        return rc;
                    }
                }
                else
                {
                    /* for XLG Unidirectional WA ports - enables/disables port */
                    /* for regular ports does nothing                          */
                    rc = cpssDxChPortEnableWaWithLinkStatusSet(
                        devNum, evExtData/*port*/);
                    if (rc != GT_OK)
                    {
                        DBG_LOG(
                            ("cpssDxChPortEnableWaWithLinkStatusSet: error, devNum=%d, port=%d, rc=%d\n",
                            devNum, evExtData, rc, 4, 5, 6));
                    }
                }

                if (apEnable)
                {
                    rc = appDemoDxChApLinkChangeTreat(devNum, portNum);
                    if(rc != GT_OK)
                    {
                        DBG_LOG(("CPSS_PP_PORT_LINK_STATUS_CHANGED_E - appDemoDxChApLinkChangeTreat:rc=%d,portNum=%d\n",rc, portNum,3,4,5,6));
                        return rc;
                    }
                }

                rc = appDemoDxChLinkChangeTreat(devNum, portNum, apEnable);
                if(rc != GT_OK)
                {
                    DBG_LOG(("CPSS_PP_PORT_LINK_STATUS_CHANGED_E - appDemoDxChLinkChangeTreat:rc=%d,portNum=%d\n",rc, portNum,3,4,5,6));
                    return rc;
                }

                if(!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                {
                    /* use function appDemoPrintLinkChangeFlagSet    to modify the : printLinkChangeEnabled */
                    if(printLinkChangeEnabled)
                    {
                        rc = cpssDxChPortLinkStatusGet(devNum, portNum, &linkUp);
                        if (rc != GT_OK)
                        {
                            return rc;
                        }
                        cpssOsPrintSync("LINK %s :devNum[%ld] %s port[%ld] \n",
                                            (linkUp == GT_FALSE) ? "DOWN" : "UP",
                                                        (GT_U32)devNum,
                                                        (prvCpssDxChPortRemotePortCheck(devNum,portNum) ? "(remote)" : ""),
                                                        portNum);
                    }
                }

#ifndef ASIC_SIMULATION
                if(PRV_CPSS_DXCH_BOBCAT2_A0_CHECK_MAC(devNum))
                {

                    /*****************************************************************************/
                    /* Important note: for BC2 evExtData -> MAC port number (NOT physical) */
                    /*****************************************************************************/

                    waArr[0] = CPSS_DXCH_IMPLEMENT_WA_TRI_SPEED_PORT_AN_FC_E;
                    additionalInfoBmpArr[0] = evExtData;
                    rc = cpssDxChHwPpImplementWaInit(devNum,1,&waArr[0],
                                                     &additionalInfoBmpArr[0]);
                    if (GT_OK != rc)
                    {
                        DBG_LOG(("cpssDxChHwPpImplementWaInit: error, devNum=%d, port=%d, rc=%d\n",
                                devNum, evExtData, rc, 4, 5, 6));
                    }
                }
#endif
            }
#endif/* DXCH_CODE */

#ifdef PX_CODE
                if (CPSS_IS_PX_FAMILY_MAC(devFamily))
                {
                    GT_U32 portNum = evExtData;

                    rc = cpssPxPortApPortEnableGet(devNum, portNum, &apEnablePx);
                    if(rc != GT_OK)
                    {
                        DBG_LOG(("CPSS_PP_PORT_LINK_STATUS_CHANGED_E - cpssPxPortApPortConfigGet:rc=%d,portNum=%d\n",rc, portNum, 3, 4, 5, 6));
                        return rc;
                    }

                    if (portMgr && !disableAutoPortMgr)
                    {
                        if (apEnablePx == GT_FALSE)
                        {
                            /* if non AP, break */

                            /* do callback for the 'DXPX' system ,that usually called from pipe_linkChange(...) */
                            /* part of fix to : CPSS-8941 BC3-Tx and Pipe-Rx counters are not good for 100G
                                [however end traffic generator counters are OK] */
                            if(appDemoCallBackOnLinkStatusChaneFunc)
                            {
                                GT_BOOL stopLogic,linkUp;
                                rc = cpssPxPortLinkStatusGet(devNum, portNum, &linkUp);
                                if (rc != GT_OK)
                                {
                                    return rc;
                                }

                                rc = appDemoCallBackOnLinkStatusChaneFunc(devNum,portNum,
                                    linkUp,
                                    APP_DEMO_CALL_BACK_ON_LINK_STATUS_CHANGE_AFTER_MAIN_LOGIC_E,
                                    &stopLogic/*not used here*/);
                                if (rc != GT_OK)
                                {
                                    cpssOsPrintSync("appDemoCallBackOnLinkStatusChaneFunc: (after logic) error, devNum=%d, port=%d, rc=%d\n",
                                            devNum, portNum, rc);
                                }

                            }

                            break;
                        }
                    }

                    rc = pipe_linkChange(devNum, portNum/* physical port number */,printLinkChangeEnabled);
                    if (rc != GT_OK)
                    {
                        DBG_LOG(("pipe_linkChange: error, devNum=%d, port=%d, rc=%d\n",
                                devNum, portNum, rc, 4, 5, 6));
                    }
                }
#endif/* PX_CODE */
            break;
        case CPSS_PP_BM_MISC_E: if( evExtData > 2 )
                                {
                                    break;
                                }
                                GT_ATTR_FALLTHROUGH;
        case CPSS_PP_BM_PORT_RX_BUFFERS_CNT_UNDERRUN_E:
        case CPSS_PP_BM_PORT_RX_BUFFERS_CNT_OVERRUN_E:
        case CPSS_PP_BM_WRONG_SRC_PORT_E:
        case CPSS_PP_CRITICAL_HW_ERROR_E:
        case CPSS_PP_PORT_PCS_PPM_FIFO_UNDERRUN_E:
        case CPSS_PP_PORT_PCS_PPM_FIFO_OVERRUN_E:

            if(prvAppDemoFatalErrorType != CPSS_ENABLER_FATAL_ERROR_SILENT_TYPE_E)
            {
                static GT_CHAR * uniEvName[CPSS_UNI_EVENT_COUNT_E] = {UNI_EV_NAME};
                osPrintf(" Critical HW Error : devNum[%d], uniEv=[%d], extraData[%d]\n", devNum, uniEv, evExtData);
                osPrintf(" Unified Event = %s\n", uniEvName[uniEv]);

#ifdef DXCH_CODE
                /* print more info about the interrupt */
                prvCpssDrvPpInterruptInfoPrint(devNum, evExtData);
#endif /*DXCH_CODE*/
            }

            if(prvAppDemoFatalErrorType == CPSS_ENABLER_FATAL_ERROR_EXECUTE_TYPE_E)
            {
                osFatalError(OS_FATAL_WARNING, "");
            }
            break;

#ifndef ASIC_SIMULATION
#ifdef DXCH_CODE
        case CPSS_PP_PORT_LANE_SYNC_STATUS_CHANGED_E:
            /*osPrintf(" CPSS_PP_PORT_LANE_SYNC_STATUS_CHANGED_E %d ",evExtData);*/
            if (devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
            {
                CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
                GT_PHYSICAL_PORT_NUM            portNum;

                portNum = evExtData>>8;
                TRAINING_DBG_PRINT_MAC(("get LANE_SYNC_STATUS_CHANGED:portNum=%d\n", portNum));
                rc = cpssDxChPortInterfaceModeGet(devNum, portNum, &ifMode);
                TRAINING_DBG_PRINT_MAC(("cpssDxChPortInterfaceModeGet:portNum=%d,ifMode=%d,rc=%d\n",
                        portNum, ifMode, rc));
                if((GT_OK == rc) && (CPSS_PORT_INTERFACE_MODE_RXAUI_E == ifMode))
                {
                    if(0 == waTSmid)
                    {
                        GT_U32   waTaskId;

                        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&todoWaBmp);
                        if (cpssOsSigSemBinCreate("waTSem", CPSS_OS_SEMB_EMPTY_E, &waTSmid) != GT_OK)
                        {
                            return GT_NO_RESOURCE;
                        }
                        TRAINING_DBG_PRINT_MAC(("create waTask\n"));
                        rc = cpssOsTaskCreate("waTask",    /* Task Name      */
                                              210,              /* Task Priority  */
                                              _32K,             /* Stack Size     */
                                              lion2WaTask,      /* Starting Point */
                                              (GT_VOID*)((GT_UINTPTR)devNum), /* Arguments list */
                                              &waTaskId);       /* task ID        */
                        if(rc != GT_OK)
                        {
                            cpssOsPrintSync("LANE_SYNC_STATUS_CHANGED(%d):failed to create links WA task:rc=%d\n",
                                                                    portNum, rc);
                        }
                    } /* if(0 == waTSmid) */

                    if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&todoWaBmp, portNum))
                    {
                        /* fix compilation warnings for some gcc compilers */
                        if (portNum >= CPSS_MAX_PORTS_NUM_CNS)
                        {
                            return GT_FAIL;
                        }
                        locksTimeoutArray[portNum] = 0;
                        CPSS_PORTS_BMP_PORT_SET_MAC(&todoWaBmp, portNum);
                        rc = cpssOsSigSemSignal(waTSmid);
                        if(rc != GT_OK)
                        {
                            cpssOsPrintSync("LANE_SYNC_STATUS_CHANGED:cpssOsSigSemSignal:rc=%d\n", rc);
                        }
                        TRAINING_DBG_PRINT_MAC(("set todoWaBmp portNum=%d\n", portNum));
                    }
                }
            }
            break;
#endif /*DXCH_CODE*/

        case CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E:

                        /* New port init sequence stage */
            {
                GT_PHYSICAL_PORT_NUM           portNum;
                portNum = (GT_U16)evExtData;

                /*cpssOsPrintf("\n** CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E. Port- %d **\n", portNum);*/
                if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_TRUE)
                {
                    /* this event is not handled for SIP6 devices */
                    return GT_OK;
                }

                if (portMgr && !disableAutoPortMgr)
                {
                    appDemoPortInitSeqPortStatusChangeSignal(devNum, portNum, CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E);
                    /* training is done inside port manager, so no need to continue */
                    break;
                }

                /*if (portNum==56 || portNum == 58)
                {
                    break;
                }*/
            }
#ifdef DXCH_CODE
            if(!CPSS_IS_DXCH_FAMILY_MAC(devFamily))
            {
                /* do not access DX APIs */
            }
            else
            if ((devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
                (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)) ||
                (PRV_CPSS_SIP_5_10_CHECK_MAC(devNum)))  /* Bobcat2_B0, Bobk, Aldrin, AC3X, Bobcat3 */
           {
                GT_PHYSICAL_PORT_NUM           portNum;
                GT_U32                         serdesTraining = 0;
                CPSS_PORT_INTERFACE_MODE_ENT   ifMode;
                CPSS_PORTS_BMP_STC *todoBmp = &todoWaBmp;

                if((PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)) ||
                   (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))) /* Bobk, Aldrin, AC3X, Bobcat3 */
                {
                    todoBmp = &todoRxTrainingCompleteBmp;
                }

                portNum = (GT_U8)evExtData;

                rc = cpssDxChPortInterfaceModeGet(devNum, portNum, &ifMode);
                if (rc != GT_OK)
                {
                     return rc;
                }


                TRAINING_DBG_PRINT_MAC(("get MMPCS_SIGNAL_DETECT_CHANGE:portNum=%d\n", portNum));
                if (devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) {
                    rc = prvAppDemoUserHandleDxChComboPort(devNum, portNum);
                    if(rc != GT_OK)
                    {
                         cpssOsPrintSync("prvAppDemoUserHandleDxChComboPort(portNum=%d):rc=%d\n",
                                         portNum, rc);
                     }
                }

                if(appDemoDbEntryGet("serdesTraining", &serdesTraining) != GT_OK)
                {
                    if (devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
                    {
                        serdesTraining = 1;
                    }
                    else
                    {
                        serdesTraining = 0;
                    }
                }

                if(serdesTraining)
                {
                    if(0 == tuneSmid)
                    {
                        GT_U32   tuneStartTaskId;

                        if(cpssOsSigSemBinCreate("tuneSem", CPSS_OS_SEMB_EMPTY_E, &tuneSmid) != GT_OK)
                        {
                            cpssOsPrintSync("MMPCS_SIGNAL_DETECT(portNum=%d) tuneSem create:rc=%d\n",
                                            portNum, rc);
                        }
                        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&todoTuneBmp);
                        rc = cpssOsTaskCreate("tuneStrt",               /* Task Name      */
                                              210,                        /* Task Priority  */
                                              _64K,                     /* Stack Size     */
                                              appDemoDxChLion2SerdesAutoTuningStartTask,   /* Starting Point */
                                              (GT_VOID*)((GT_UINTPTR)devNum),         /* Arguments list */
                                              &tuneStartTaskId);               /* task ID        */
                        if (rc != GT_OK)
                        {
                            cpssOsPrintSync("MMPCS_SIGNAL_DETECT(portNum=%d) tuneStrt create:rc=%d\n",
                                            portNum, rc);
                        }
                    }

                    CPSS_PORTS_BMP_PORT_SET_MAC(&todoTuneBmp, portNum);
                    rc = cpssOsSigSemSignal(tuneSmid);

                    if(rc != GT_OK)
                    {
                        cpssOsPrintSync("MMPCS_SIGNAL_DETECT(portNum=%d) cpssOsSigSemSignal(tuneSmid):rc=%d\n",
                                        portNum, rc);
                    }
                }
                else
                {
                    if(0 == waTSmid)
                    {
                        GT_U32   waTaskId;

                        CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(todoBmp);
                        if (cpssOsSigSemBinCreate("waTSem", CPSS_OS_SEMB_EMPTY_E, &waTSmid) != GT_OK)
                        {
                            return GT_NO_RESOURCE;
                        }
                        rc = cpssOsTaskCreate("waTask",    /* Task Name      */
                                              210,              /* Task Priority  */
                                              _32K,             /* Stack Size     */
                                              lion2WaTask,      /* Starting Point */
                                              (GT_VOID*)((GT_UINTPTR)devNum), /* Arguments list */
                                              &waTaskId);       /* task ID        */
                        if(rc != GT_OK)
                        {
                            cpssOsPrintSync("failed to create links WA task:rc=%d\n", rc);
                        }
                    }

                    if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(todoBmp, portNum))
                    {
                        locksTimeoutArray[portNum] = 0;
                        CPSS_PORTS_BMP_PORT_SET_MAC(todoBmp, portNum);
                        rc = cpssOsSigSemSignal(waTSmid);
                        if(rc != GT_OK)
                        {
                            cpssOsPrintSync("MMPCS_SIGNAL_DETECT(portNum=%d) cpssOsSigSemSignal(waTSmid):rc=%d\n",
                                            portNum, rc);
                        }
                    }
                }
            }
            if(((PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)) ||
                (PRV_CPSS_SIP_5_CHECK_MAC(devNum))) && (1 == mmpcsPrint))
            {
                GT_PHYSICAL_PORT_NUM  portNum;

                portNum = evExtData&0xff;
                /* printf preferable, because printSync not implemented for every CPU */
                cpssOsPrintf("MMPCS_SIGNAL_DETECT:devNum=%d,portNum=%d\n",
                    devNum, portNum);
            }
#endif /*DXCH_CODE*/
#endif /* ASIC_SIMULATION */


            break;

        case CPSS_PP_DATA_INTEGRITY_ERROR_E:
#if defined(DXCH_CODE)
            if( PRV_CPSS_DXCH_LION2_FAMILY_CHECK_MAC(devNum) ||
                PRV_CPSS_DXCH_BOBCAT2_CHECK_MAC(devNum)      ||
                PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum)       ||
                PRV_CPSS_DXCH_BOBCAT3_CHECK_MAC(devNum)      ||
                PRV_CPSS_DXCH_CETUS_CHECK_MAC(devNum)        ||
                PRV_CPSS_DXCH_CAELUM_CHECK_MAC(devNum)       ||
                PRV_CPSS_DXCH_ALDRIN2_CHECK_MAC(devNum)      ||
                PRV_CPSS_DXCH_AC5_CHECK_MAC(devNum)          ||
                PRV_CPSS_DXCH_FALCON_CHECK_MAC(devNum)       ||
                PRV_CPSS_DXCH_AC5X_CHECK_MAC(devNum)         ||
                PRV_CPSS_DXCH_AC5P_CHECK_MAC(devNum)         ||
                PRV_CPSS_DXCH_HARRIER_CHECK_MAC(devNum)
              )


            {
                rc = appDemoDxChLion2DataIntegrityScan(devNum, evExtData);
            }
#endif
#ifdef PX_CODE
            if (CPSS_IS_PX_FAMILY_MAC(devFamily))
            {
                rc = appDemoPxDataIntegrityEventsScan(devNum, evExtData);
            }
#endif
            break;


        case CPSS_PP_PORT_EEE_E:
            if (printEeeInterruptInfo)
            {
                GT_PHYSICAL_PORT_NUM    portNum;
                GT_U32                  subEvent = (evExtData & 0xFF);

                portNum = evExtData>>8;
                osPrintf("EEE Interrupt: devNum[%ld] port[%ld] subEvent [%ld] \n",
                                                (GT_U32)devNum, portNum , subEvent);
            }
            break;
        case CPSS_PP_PORT_SYNC_STATUS_CHANGED_E:
            {
                GT_PHYSICAL_PORT_NUM           portNum;
                portNum = (GT_U16)evExtData;

                /*cpssOsPrintf("\n** CPSS_PP_PORT_SYNC_STATUS_CHANGED_E. Port- %d **\n", portNum);*/

                if (portMgr && !disableAutoPortMgr)
                {
                    appDemoPortInitSeqPortStatusChangeSignal(devNum, portNum, CPSS_PP_PORT_SYNC_STATUS_CHANGED_E);
                }
            }

#ifdef DXCH_CODE
            if (PRV_CPSS_DXCH_CETUS_CHECK_MAC(devNum) || PRV_CPSS_DXCH_CAELUM_CHECK_MAC(devNum))
            {
                rc = AN_WA_Task_SyncChangeEventBuildAndSend(devNum, evExtData);
            }
#endif /*DXCH_CODE*/
            break;

        case CPSS_PP_PORT_AN_HCD_FOUND_E:
#ifdef DXCH_CODE
            rc = cpssDxChPortPhysicalPortMapReverseMappingGet(devNum, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,evExtData, &physicalPortNum);
            if(rc != GT_OK)
            {
                osPrintf("CPSS_PP_PORT_AN_HCD_FOUND_E - cpssDxChPortPhysicalPortMapReverseMappingGet:rc=%d,portNum=%d\n",rc, evExtData);
                return rc;
            }

            if (portMgr && !disableAutoPortMgr)
            {
                appDemoPortInitSeqPortStatusChangeSignal(devNum, physicalPortNum, CPSS_PP_PORT_AN_HCD_FOUND_E);
                /* pizza allocation done inside port manager, so no need to continue */
                break;
            }
#endif /*DXCH_CODE*/
            break;

        case CPSS_PP_PORT_AN_RESTART_E:
#ifdef DXCH_CODE
            rc = cpssDxChPortPhysicalPortMapReverseMappingGet(devNum, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,evExtData, &physicalPortNum);
            if(rc != GT_OK)
            {
                osPrintf("CPSS_PP_PORT_AN_RESTART_E - cpssDxChPortPhysicalPortMapReverseMappingGet:rc=%d,portNum=%d\n",rc, evExtData);
                return rc;
            }

            if (portMgr && !disableAutoPortMgr)
            {
                appDemoPortInitSeqPortStatusChangeSignal(devNum, physicalPortNum, CPSS_PP_PORT_AN_RESTART_E);
                break;
            }
#endif /*DXCH_CODE*/
            break;

        case CPSS_PP_PORT_AN_PARALLEL_DETECT_E:
#ifdef DXCH_CODE
            rc = cpssDxChPortPhysicalPortMapReverseMappingGet(devNum, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,evExtData, &physicalPortNum);
            if(rc != GT_OK)
            {
                osPrintf("CPSS_PP_PORT_AN_PARALLEL_DETECT_E - cpssDxChPortPhysicalPortMapReverseMappingGet:rc=%d,portNum=%d\n",rc, evExtData);
                return rc;
            }

            if (portMgr && !disableAutoPortMgr)
            {
                appDemoPortInitSeqPortStatusChangeSignal(devNum, physicalPortNum, CPSS_PP_PORT_AN_PARALLEL_DETECT_E);
                break;
            }
#endif /*DXCH_CODE*/
            break;

        case CPSS_PP_PORT_AN_COMPLETED_E:
#ifdef DXCH_CODE
            if (PRV_CPSS_DXCH_CETUS_CHECK_MAC(devNum) || PRV_CPSS_DXCH_CAELUM_CHECK_MAC(devNum))
            {
                rc = AN_WA_Task_AnCompleteEventBuildAndSend(devNum, evExtData);
            }
#endif /*DXCH_CODE*/
            break;
        case CPSS_PP_PORT_PCS_GB_LOCK_SYNC_CHANGE_E:
            portNum = (GT_U16)evExtData;
            /*cpssOsPrintf("\n** CPSS_PP_PORT_PCS_GB_LOCK_SYNC_CHANGE_E. Port- %d **\n", portNum);*/


            NEW_PORT_INIT_SEQ
            /* New port init sequence stage */
            if (portMgr && !disableAutoPortMgr)
            {
                appDemoPortInitSeqPortStatusChangeSignal(devNum, portNum, CPSS_PP_PORT_PCS_GB_LOCK_SYNC_CHANGE_E);
            }

            break;
        case CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E:
            portNum = (GT_U16)evExtData;
            /*cpssOsPrintf("\n** CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E. Port- %d **\n", portNum);*/
#ifdef DXCH_CODE
            rc = sip6_alignLockChange(devNum, portNum);
            if(rc != GT_OK)
            {
                cpssOsPrintSync("CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E(portNum=%d) sip6_alignLockChange:rc=%d\n",
                                            portNum, rc);
            }
#endif /*DXCH_CODE*/
            NEW_PORT_INIT_SEQ
            /* New port init sequence stage */
            if (portMgr && !disableAutoPortMgr)
            {
                appDemoPortInitSeqPortStatusChangeSignal(devNum, portNum, CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E);
            }

            break;
#ifdef DXCH_CODE
        case CPSS_PP_TQ_PORT_MICRO_BURST_E:
            if((appDemoDbEntryGet("uBurstManualEventFetch", &value) == GT_NO_SUCH)||
                value==0)
            {
                do
                {
                    uBurstEventsSize = MAX_UBURST_EVENTS_CHUNK;
                    rc = cpssDxChPortTxUburstEventInfoGet(devNum,&uBurstEventsSize,uBurstData);
                    if(rc==GT_OK ||rc == GT_NO_MORE)
                    {
                        cpssOsPrintf("PORT_MICRO_BURST DETECT:Got  %d micro burst events\n",uBurstEventsSize);
                    }
                    else
                    {
                        cpssOsPrintSync("PORT_MICRO_BURST DETECT :cpssDxChPortTxUburstEventInfoGet failed:rc=%d\n",rc);
                    }
                }
                while(rc==GT_OK);
            }
            else
            {
                cpssOsPrintf("Got PORT_MICRO_BURST DETECT event.\n");
            }
            break;
       case CPSS_PP_TQ_PORT_HR_CROSSED_THRESHOLD_E:
            {

                GT_U32 tileNum,pipeNum,qfcNum,regNum,tc,port,phyPort;

                tc = (evExtData&0xFF);
                evExtData>>=8;
                port = (evExtData&0xFF);
                evExtData>>=8;
                regNum = (evExtData&0xFF);
                evExtData>>=8;
                qfcNum = (evExtData&0xF);
                evExtData>>=4;
                pipeNum = (evExtData&0x3);
                evExtData>>=2;
                tileNum = (evExtData&0x3);

                rc = prvCpssFalconTxqUtilsHeadroomTriggerPortGet(devNum,tileNum,pipeNum,qfcNum,regNum,port,&phyPort);
                if(rc != GT_OK)
                {
                     cpssOsPrintSync("prvCpssFalconTxqUtilsHeadroomTriggerPortGet rc=%d\n",rc);
                }

                cpssOsPrintf("HR_CROSSED_THRESHOLD crossed on tile %d , pipe %d , dp %d , reg %d , port %d ,tc %d.\n",
                    tileNum,pipeNum,qfcNum,regNum,port,tc);

                cpssOsPrintf("Detected HR event on physical port %d tc %d.\n", phyPort,tc);

            }
          break;
       case CPSS_PP_MACSEC_SA_EXPIRED_E:
       case CPSS_PP_MACSEC_SA_PN_FULL_E:
       case CPSS_PP_MACSEC_EGRESS_SEQ_NUM_ROLLOVER_E:
       case CPSS_PP_MACSEC_STATISTICS_SUMMARY_E:
       case CPSS_PP_MACSEC_TRANSFORM_ENGINE_ERR_E:
           appDemoDxChMacSecEventsTreat(devNum, uniEv, evExtData);
           break;
        case CPSS_PP_PORT_LANE_FEC_ERROR_E:
           rc = appDemoDxChFecEventCounterIncrement(devNum, evExtData);
           if(rc != GT_OK)
           {
               cpssOsPrintSync("CPSS_PP_PORT_LANE_FEC_ERROR_E (LANE=%d) rc=%d\n",
                                           evExtData & 0xFFFF, rc);
           }

           break;
#endif
        default:
            break;
    }

    return rc;
}


/* treat unified event of the device : PP/XBAR/FA */
GT_STATUS appDemoEniEvTreat
(
    GT_U8                   devNum,
    GT_U32                  uniEv,
    GT_U32                  evExtData
)
{
    GT_STATUS rc;

    if (/*(uniEv >= CPSS_PP_UNI_EV_MIN_E) &&*/
        (uniEv <= CPSS_SRVCPU_MAX_E))
    {
        rc = appDemoEnPpEvTreat(devNum, uniEv, evExtData);
    }
    else if ((uniEv >= CPSS_XBAR_UNI_EV_MIN_E) &&
             (uniEv <= CPSS_XBAR_UNI_EV_MAX_E))
    {
        rc = GT_NOT_SUPPORTED;
    }
    else if ((uniEv >= CPSS_FA_UNI_EV_MIN_E) &&
             (uniEv <= CPSS_FA_UNI_EV_MAX_E))
    {
        rc = GT_NOT_SUPPORTED;
    }
    else
    {
        rc = GT_NOT_SUPPORTED;
    }

    /* counting the event */
    if(eventIncrementFunc != NULL)
    {
        /* don't care about the return value */
        (GT_VOID)eventIncrementFunc(devNum, uniEv, evExtData);
    }

    return rc;
}


/*******************************************************************************
* appDemoEvHndlr
*
* DESCRIPTION:
*       This routine is the event handler for PSS Event-Request-Driven mode
*       (polling mode).
*
* INPUTS:
*       param - The process data structure.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static unsigned __TASKCONV appDemoEvHndlr
(
    GT_VOID * param
)
{
    GT_STATUS           rc;                                         /* return code         */
    GT_U32              i;                                          /* iterator            */
    GT_UINTPTR          evHndl;                                     /* event handler       */
    GT_U32              evBitmapArr[CPSS_UNI_EV_BITMAP_SIZE_CNS];   /* event bitmap array  */
    GT_U32              evBitmap;                                   /* event bitmap 32 bit */
    GT_U32              evExtData;                                  /* event extended data */
    GT_U8               devNum;                                     /* device number       */
    GT_U32              uniEv;                                      /* unified event cause */
    GT_U32              evCauseIdx;                                 /* event index         */
    EV_HNDLR_PARAM      *hndlrParamPtr;                             /* bind event array    */
#ifdef APPDEMO_REQ_MODE_DEBUG
    GT_U32 tid, prio;
#endif

    hndlrParamPtr = (EV_HNDLR_PARAM*)param;
    evHndl        = hndlrParamPtr->evHndl;


    /*osPrintf("appDemoEvHndlr[%d]: created \n",
        hndlrParamPtr->hndlrIndex);*/

    /* indicate that task start running */
    taskCreated = 1;
    osTimerWkAfter(1);

    while (1)
    {
        rc = cpssEventSelect(evHndl, NULL, evBitmapArr, (GT_U32)CPSS_UNI_EV_BITMAP_SIZE_CNS);
        if(eventRequestDrvnModeReset)
        {
            break;
        }

        if (GT_OK != rc)
        {
            DBG_LOG(("appDemoCpssEventSelect: err [%d]\n", rc, 2, 3, 4, 5, 6));
            continue;
        }

        for (evCauseIdx = 0; evCauseIdx < CPSS_UNI_EV_BITMAP_SIZE_CNS; evCauseIdx++)
        {
            evBitmap = evBitmapArr[evCauseIdx];

            for (i = 0; evBitmap; evBitmap >>= 1, i++)
            {
                if ((evBitmap & 1) == 0)
                {
                    continue;
                }

                uniEv = (evCauseIdx << 5) + i;

                if (cpssEventRecv(evHndl, uniEv, &evExtData, &devNum) == GT_OK)
                {
#ifdef APPDEMO_REQ_MODE_DEBUG
                    osTaskGetSelf(&tid);
                    osGetTaskPrior(tid, &prio);
                    DBG_LOG(("cpssEventRecv: %d <dev %d, %s, extData %d> tid 0x%x prio %d\n",
                             (GT_U32)hndlrParamPtr->hndlrIndex, devNum,
                             (GT_U32)uniEvName[uniEv], evExtData, tid, prio));
#endif
                    if (notifyEventArrivedFunc != NULL)
                    {
                        notifyEventArrivedFunc(devNum, uniEv, evExtData);
                    }

                    rc = appDemoEniEvTreat(devNum, uniEv, evExtData);
                    if (GT_OK != rc)
                    {
                        DBG_LOG(("appDemoEniEvTreat: error - 0x%x, uniEv [%d]\n", rc, uniEv, 3, 4, 5, 6));
                    }
                }
            }
        }
    }

    /* give indication that the thread exit */
    /*osPrintf("appDemoEvHndlr[%d]: thread exit during system reset \n",
        hndlrParamPtr->hndlrIndex);*/

    hndlrParamPtr->hndlrIndex = FREE_TASK_CNS;

    return 0;
}

static GT_U32 rx2cpu_queue_wrr[] = {APP_DEMO_SINGLE_RX_TASK_P0_WEIGHT,
                                    APP_DEMO_SINGLE_RX_TASK_P1_WEIGHT,
                                    APP_DEMO_SINGLE_RX_TASK_P2_WEIGHT,
                                    APP_DEMO_SINGLE_RX_TASK_P3_WEIGHT,
                                    APP_DEMO_SINGLE_RX_TASK_P4_WEIGHT,
                                    APP_DEMO_SINGLE_RX_TASK_P5_WEIGHT,
                                    APP_DEMO_SINGLE_RX_TASK_P6_WEIGHT,
                                    APP_DEMO_SINGLE_RX_TASK_P7_WEIGHT};

/*******************************************************************************
* appDemoSingleRxEvHndlr
*
* DESCRIPTION:
*       This routine is the event handler for all RX events for all queues.
*
* INPUTS:
*       param - The process data structure.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static unsigned __TASKCONV appDemoSingleRxEvHndlr
(
    GT_VOID * param
)
{
    GT_STATUS           rc;                                         /* return code         */
    GT_UINTPTR          evHndl;                                     /* event handler       */
    GT_U32              evBitmapArr[CPSS_UNI_EV_BITMAP_SIZE_CNS];   /* event bitmap array  */
    GT_U32              evExtData;                                  /* event extended data */
    GT_U8               devNum;                                     /* device number       */
    EV_HNDLR_PARAM      *hndlrParamPtr;                             /* bind event array    */

    GT_BOOL cpuRxNotEmpty[PRV_CPSS_MAX_PP_DEVICES_CNS][8] = {{0}};
    GT_U32 queue;
    GT_U32 maxPacket;
    GT_U8 dev;
    GT_U32 counter =0;
    GT_U32 fullQueues =0;
    hndlrParamPtr = (EV_HNDLR_PARAM*)param;
    evHndl        = hndlrParamPtr->evHndl;


    /*osPrintf("appDemoSingleRxEvHndlr[%d]: created \n",
        hndlrParamPtr->hndlrIndex);*/

    /* indicate that task start running */
    taskCreated = 1;

    while (1)
    {
        rc = cpssEventSelect(evHndl, NULL, evBitmapArr, (GT_U32)CPSS_UNI_EV_BITMAP_SIZE_CNS);
        if(eventRequestDrvnModeReset)
        {
            break;
        }
        if (GT_OK != rc)
        {
            DBG_LOG(("cpssEventSelect: err [%d]\n", rc, 2, 3, 4, 5, 6));
            continue;
        }
        do
        {
            for (queue = 0; queue < 8; queue++)
            {
                do
                {
                    /* get all the events for current queue*/
                    rc = cpssEventRecv(evHndl,CPSS_PP_RX_BUFFER_QUEUE0_E+queue,&evExtData,&devNum);
                    if (rc == GT_OK)
                    {
                        /* counting the event */
                        if(eventIncrementFunc != NULL)
                        {
                                (GT_VOID)eventIncrementFunc(devNum, CPSS_PP_RX_BUFFER_QUEUE0_E+queue, 0);
                        }
                        cpuRxNotEmpty[devNum][queue] = GT_TRUE;
                    }
                    rc = cpssEventRecv(evHndl,CPSS_PP_RX_ERR_QUEUE0_E+queue,&evExtData,&devNum);
                    if (rc == GT_OK)
                    {
                        /* counting the event */
                        if(eventIncrementFunc != NULL)
                        {
                                (GT_VOID)eventIncrementFunc(devNum, CPSS_PP_RX_ERR_QUEUE0_E+queue, 0);
                        }
                        cpuRxNotEmpty[devNum][queue] = GT_TRUE;
                    }
                }
                while (rc == GT_OK);

                maxPacket = rx2cpu_queue_wrr[queue];
                for(dev = SYSTEM_DEV_NUM_MAC(0); dev < SYSTEM_DEV_NUM_MAC(appDemoPpConfigDevAmount); dev++)
                {
                    if(appDemoPpConfigList[dev].valid == GT_FALSE || cpuRxNotEmpty[dev][queue] == GT_FALSE)
                    {
                        counter++;
                        continue;
                    }
                    /* get up to maxPacket */
                    rc = appDemoGetMaxPackets((GT_U8)appDemoPpConfigList[dev].devNum,(GT_U8)queue,maxPacket);
                    if (rc == GT_NO_MORE)
                    {
                        counter++;
                        cpssEventTreatedEventsClear(evHndl);
                        cpuRxNotEmpty[dev][queue] = GT_FALSE;
                    }
                }
                if (counter >= appDemoPpConfigDevAmount)
                {
                    U32_SET_FIELD_MAC(fullQueues,queue,1,0);
                }
                else
                {
                    U32_SET_FIELD_MAC(fullQueues,queue,1,1);
                }
                counter = 0;
            }
        } while ( fullQueues );
    }

    /* give indication that the thread exit */
    /*osPrintf("appDemoSingleRxEvHndlr[%d]: thread exit during system reset \n",
        hndlrParamPtr->hndlrIndex);*/
    hndlrParamPtr->hndlrIndex = FREE_TASK_CNS;

    return 0;
}


/*******************************************************************************
* appDemoSingleRxTreatTask
*
* DESCRIPTION:
*       This routine treats Rx packets in single Rx task mode.
*
* INPUTS:
*       param - The process data structure.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static unsigned __TASKCONV appDemoSingleRxTreatTask
(
    GT_VOID * param
)
{
    GT_STATUS           rc; /* return code         */
    GT_U32              size; /* size of buffer pointed by message */
    GT_U8               devNum;
    CPSS_OS_MSGQ_ID    *msgQIdPtr;
    APP_DEMO_RX_PACKET_PARAMS  rxParams;
    msgQIdPtr = (CPSS_OS_MSGQ_ID*)param;

    size = sizeof(APP_DEMO_RX_PACKET_PARAMS);

    while (1)
    {
        rc = cpssOsMsgQRecv(*msgQIdPtr,&rxParams,&size,CPSS_OS_MSGQ_WAIT_FOREVER);
        if (rc != GT_OK)
        {
            DBG_LOG(("cpssOsMsgQRecv: err [%d]\n", rc, 2, 3, 4, 5, 6));
            continue;
        }
        devNum = rxParams.devNum;
        rc = GT_NOT_IMPLEMENTED;
        if(IS_API_CH_DEV_MAC(devNum))/* is device support dxch API ? */
        {
#ifdef DXCH_CODE
            /* apply the App Demo treatment for the new packet */
            rc = appDemoDxChNetRxPktTreat(&rxParams);
#endif /*DXCH_CODE*/
        }
        else
        if(CPSS_IS_PX_FAMILY_MAC(PRV_CPSS_PP_MAC(devNum)->devFamily))
        {
#ifdef PX_CODE
            /* apply the App Demo treatment for the new packet */
            rc = appDemoPxNetRxPktTreat(&rxParams);
#endif /*PX_CODE*/
        }

        if (rc != GT_OK)
        {
            DBG_LOG(("XXX_NetRxPktTreat: err [%d]\n", rc, 2, 3, 4, 5, 6));
        }

        if(rc == GT_HW_ERROR_NEED_RESET)
        {
            /* fake check that will never happen , for klockwork warning of endless loop */
            break;
        }

    }

    /* will never happen */
    return 0;
}


/**
* @internal cpssEnRxPacketGet function
* @endinternal
*
* @brief   This routine handles the packet Rx event.
*
* @param[in] devNum                   - the device number.
* @param[in] queue                    - the  the rx event occurred upon
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS cpssEnRxPacketGet
(
    IN RX_EV_HANDLER_EV_TYPE_ENT evType,
    IN GT_U8                devNum,
    IN GT_U8                queue
)
{
    GT_STATUS       rc = GT_NOT_IMPLEMENTED;

    do
    {
        if(IS_API_CH_DEV_MAC(devNum))/* is device support dxch API ? */
        {
#ifdef DXCH_CODE
            /* apply the App Demo treatment for the new packet */
            rc = appDemoDxChNetRxPktHandle(rxEventHanderHnd, evType, devNum, queue);
#endif /*DXCH_CODE*/
        }
        else
        if(IS_API_PX_DEV_MAC(devNum))/* is device support PX API ? */
        {
#ifdef PX_CODE
            /* apply the App Demo treatment for the new packet */
            rc = appDemoPxNetRxPktHandle(rxEventHanderHnd, evType, devNum, queue);
#endif /*PX_CODE*/
        }

    }while(rc == GT_OK);

    return rc;
}

/**
* @internal appDemoGetMaxPackets function
* @endinternal
*
* @brief   This routine get the RX packets from given device and queue
*         up to maxPacket.
* @param[in] devNum                   - the device number.
* @param[in] queue                    - the  the rx event occurred upon
* @param[in] maxPacket                - maximum packets to get from the queue
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS appDemoGetMaxPackets
(
    IN GT_U8                devNum,
    IN GT_U8                queue,
    IN GT_U32               maxPacket
)
{
    GT_STATUS       rc = GT_NOT_IMPLEMENTED; /* The returned code            */
#ifdef DXCH_CODE
    GT_U32          counter = 0; /* counter */
    CPSS_OS_MSGQ_ID msgQId;
    GT_U32 number;

    do
    {
        /* choose msgQ */
        number = (singleTaskRxRandomEnable == GT_TRUE) ?
                    cpssOsRand() : queue;
        msgQId = singleRxMsgQIds[number % treatTasksNum];

        if(IS_API_CH_DEV_MAC(devNum))/* is device support dxch API ? */
        {
            /* apply the App Demo treatment for the new packet */
            rc = appDemoDxChNetRxPktGet(devNum, queue,msgQId);
        }
        else
        {
            rc = GT_NOT_IMPLEMENTED;
        }

        counter++;

    }while(rc == GT_OK && counter < maxPacket);

#elif defined PX_CODE
    GT_U32          counter = 0; /* counter */
    CPSS_OS_MSGQ_ID msgQId;
    GT_U32 number;

    do
    {
        /* choose msgQ */
        number = (singleTaskRxRandomEnable == GT_TRUE) ?
                    cpssOsRand() : queue;
        msgQId = singleRxMsgQIds[number % treatTasksNum];

        if(IS_API_PX_DEV_MAC(devNum))/* is device support Px API ? */
        {
            /* apply the App Demo treatment for the new packet */
            rc = appDemoPxNetRxPktGet(devNum, queue,msgQId);
        }
        else
        {
            rc = GT_NOT_IMPLEMENTED;
        }

        counter++;

    }while(rc == GT_OK && counter < maxPacket);
#endif /*PX_CODE*/

    return rc;
}

/**
* @internal appDemoEventsDataBaseGet function
* @endinternal
*
* @brief   this method is a getter to the events-table database
*
* @retval GT_OK                    - on success
*/

GT_STATUS appDemoEventsDataBaseGet
(
    APP_UTILS_UNI_EVENT_COUNTER_STC ***eventCounterBlockGet
)
{
    *eventCounterBlockGet = appUtilsEventCounters;
    return GT_OK;
}

#ifdef GM_USED
extern GT_VOID snetGmPortBmpLinkInfoGet
(
    IN  GT_U8    deviceNumber,
    IN  GT_U32   linkUpPortsBmpArr[4],
    IN  GT_U32   changedPortsBmpArr[4]
);
extern GT_STATUS   getSimDevIdFromSwDevNum
(
    IN GT_U8    swDevNum,
    IN  GT_U32  portGroupId,
    OUT GT_U32  *simDeviceIdPtr
);

static GT_U32 gm_appDemoEmulateLinkChangeTask_sleepTime = 250;
void gm_appDemoEmulateLinkChangeTask_sleepTimeSet(IN GT_U32 newTime)
{
    gm_appDemoEmulateLinkChangeTask_sleepTime = newTime + 1;/*to avoid 0*/
}

/*******************************************************************************
* gm_appDemoEmulateLinkChangeTask
*
* DESCRIPTION:
*       the GM not hold MAC and therefor not generate next event :
*        CPSS_PP_PORT_LINK_STATUS_CHANGED_E
*
*        still we have WA using simulation on top of the GM.
*
* INPUTS:
*       param - The process data structure.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static unsigned __TASKCONV gm_appDemoEmulateLinkChangeTask
(
    GT_VOID * param
)
{
    GT_STATUS rc;        /* The returned code   */
    GT_U8     devNum;    /* device number       */
    GT_U32   linkUpPortsBmpArr[4];
    GT_U32   changedPortsBmpArr[4];
    GT_U32   macPortNum;
    GT_U32   portGroupId = 0;
    GT_U32   sim_deviceId;
    GT_U32   portNum;/* physical port num that represent the MAC number */

    /* indicate that task start running */
    taskCreated = 1;
    osTimerWkAfter(1);

    while (1)
    {
        for(devNum = SYSTEM_DEV_NUM_MAC(0); devNum < SYSTEM_DEV_NUM_MAC(appDemoPpConfigDevAmount); devNum++)
        {
            if(appDemoPpConfigList[devNum].valid == GT_FALSE)
            {
                continue;
            }

            osTimerWkAfter(gm_appDemoEmulateLinkChangeTask_sleepTime);

            /* convert cpss devNum to simulation deviceId */
            rc = getSimDevIdFromSwDevNum(appDemoPpConfigList[devNum].devNum,portGroupId,&sim_deviceId);
            if(rc != GT_OK)
            {
                continue;
            }

            /* call the simulation to get the info */
            snetGmPortBmpLinkInfoGet((GT_U8)sim_deviceId,linkUpPortsBmpArr,changedPortsBmpArr);

            for(macPortNum = 0 ; macPortNum < 128 ; macPortNum++)
            {
                if(0 == (changedPortsBmpArr[macPortNum >> 5] & (1<<(macPortNum&0x1f))))
                {
                    continue;
                }

                portNum = macPortNum;

                #ifdef DXCH_CODE
                /* convert the MAC to physical port number */
                (void)prvCpssDxChPortEventPortMapConvert(
                    appDemoPpConfigList[devNum].devNum,
                    PRV_CPSS_EV_CONVERT_DIRECTION_HW_INTERRUPT_TO_UNI_EVENT_E,
                    macPortNum,
                    &portNum);
                #endif

                /* the MAC changed it's status from last time called */
                /* so emulate the event handling */

                (void)appDemoEnPpEvTreat(appDemoPpConfigList[devNum].devNum,
                    CPSS_PP_PORT_LINK_STATUS_CHANGED_E,
                    portNum);

                /* counting the event */
                if(eventIncrementFunc != NULL)
                {
                    /* don't care about the return value */
                    (GT_VOID)eventIncrementFunc(devNum, CPSS_PP_PORT_LINK_STATUS_CHANGED_E, portNum);
                }
            }
        }

        if(gm_appDemoEmulateLinkChangeTask_sleepTime == 0)
        {
            /* avoid warning of unreachable code on the 'return 0' at the end of function */
            break;
        }
    }

    /* avoid next warning : no return statement in function returning non-void [-Werror=return-type] */
    return 0;

}
#endif /*GM_USED*/
/**
* @internal appDemoDeviceEventMaskSet function
* @endinternal
*
* @brief   This routine masks/unmask events specific to device.
*
* @param[in] devNum                - device number
* @param[in] operation             - type of mask/unmask to do on the events
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoDeviceEventMaskSet
(
    IN GT_U8                       devNum,
    IN CPSS_EVENT_MASK_SET_ENT     operation
)
{
    GT_STATUS rc;              /* The returned code */
    GT_U32    i;               /* Iterator          */
    GT_U32    devIdx;

    if (GT_OK != appDemoDevIdxGet(devNum, &devIdx))
    {
        cpssOsPrintf("PP [%d] Not found in appDemoPpConfigList[].devNum \n",
        devNum);
        return GT_BAD_PARAM;
    }

    for (i = 0; i < APP_DEMO_PROCESS_NUMBER; i++)
    {
        if(evHndlrCauseAllArrSize[i] == 0 || evHndlrCauseAllArr[i][0] == CPSS_UNI_RSRVD_EVENT_E)
        {
            continue;
        }

        /* call the CPSS to disable those interrupts in the HW of the device */
        rc = prvUniEvMaskAllDeviceSet(devIdx,evHndlrCauseAllArr[i],
                                      evHndlrCauseAllArrSize[i],
                                      &uniEvSkipArr[0],
                                      uniEvSkipArrLength,
                                      operation);
        if (GT_OK != rc)
        {
            osPrintSync("prvUniEvMaskAllSet: failed on [i=%d] \n",i);
            return GT_FAIL;
        }
    }

    return GT_OK;
}

/**
* @internal appDemoDeviceSkipEventMaskSet function
* @endinternal
*
* @brief   This routine masks/unmask skip events specific to device.
*
* @param[in] devNum                - device number
* @param[in] operation             - type of mask/unmask to do on the events
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoDeviceSkipEventMaskSet
(
    IN GT_U8                       devNum,
    IN CPSS_EVENT_MASK_SET_ENT     operation
)
{
    GT_STATUS rc;              /* The returned code */
    GT_U32    devIdx;

    if (GT_OK != appDemoDevIdxGet(devNum, &devIdx))
    {
        cpssOsPrintf("PP [%d] Not found in appDemoPpConfigList[].devNum \n",
        devNum);
        return GT_BAD_PARAM;
    }

    /* call the CPSS to enable/disable those interrupts in the HW of the device */
    rc = prvUniEvMaskAllDeviceSet(devIdx,
                                  &uniEvSkipArr[0],
                                  uniEvSkipArrLength,
                                  NULL,
                                  0,
                                  operation);
    if (GT_OK != rc)
    {
        osPrintSync("prvUniEvMaskAllDeviceSet: failed [devNum=%d] \n", devNum);
    }

    return rc;
}

/**
* @internal appDemoDeviceEventRequestDrvnModeReset function
* @endinternal
*
* @brief   This routine deletes event DB
* counter.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoDeviceEventRequestDrvnModeReset
(
    IN GT_U8 devNum
)
{
    GT_STATUS rc;              /* The returned code            */
    GT_U32    i;               /* Iterator */

    if((appDemoPpConfigDevAmount-1) == 0)
    {
        eventRequestDrvnModeReset = 1;
#ifndef ASIC_SIMULATION
#ifdef DXCH_CODE
        if (waTSmid)
        {
            cpssOsSigSemSignal(waTSmid);

            /* provide time for WA task to exit */
            osTimerWkAfter(20);
            cpssOsSigSemDelete(waTSmid);

            osPrintSync("appDemoDeviceEventRequestDrvnModeReset: delete waTSmid \n");
        }
#endif
#endif
    }

        osPrintSync("appDemoDeviceEventRequestDrvnModeReset: destroy task handlers \n");
        /* changing the mask interrupts registers may invoke 'last minute' interrupts */
        /* let the task handle it before we continue */
    if((appDemoPpConfigDevAmount-1) == 0)
    {
        osTimerWkAfter(100);

        for (i = 0; i < APP_DEMO_PROCESS_NUMBER; i++)
        {
            if(evHndlrCauseAllArrSize[i] == 0 || evHndlrCauseAllArr[i][0] == CPSS_UNI_RSRVD_EVENT_E)
            {
                /*osPrintf("appDemoEvHndlr[%d]: not exists \n",
                  i);*/
                continue;
            }

            /* call CPSS to destroy the events */
            rc = cpssEventDestroy(taskParamArr[i].evHndl);
            if (GT_OK != rc)
            {
                osPrintSync("cpssEventDestroy: failed on [i=%d] \n",i);
                return GT_FAIL;
            }

            while(taskParamArr[i].hndlrIndex != FREE_TASK_CNS)
            {
                /* this is indication that the thread is exited properly ... no need to kill it */
                osTimerWkAfter(1);
                osPrintf("8");
            }
            /*osPrintf("\n");*/

            taskParamArr[i].hndlrIndex = 0;
            taskParamArr[i].evHndl     = 0;
            eventHandlerTidArr[i]      = 0;
        }

        if (singleTaskRxEnable == GT_TRUE)
        {
            osPrintSync("appDemoDeviceEventRequestDrvnModeReset: destroy RX task handlers \n");
            for (i = 0; i < treatTasksNum; i++)
            {
                /* delete rx treat task -- before deleting the mutex/semaphore */
                rc = osTaskDelete(treatRxEventHandlerTid[i]);
                if (rc != GT_OK)
                {
                    osPrintSync("singleTaskRxEnable : osTaskDelete: failed on [i=%d] \n",i);
                    return rc;
                }

                /* delete msgQ */
                rc = osMsgQDelete(singleRxMsgQIds[i]);
                if (rc != GT_OK)
                {
                    osPrintSync("osMsgQDelete : osTaskDelete: failed on [i=%d] \n",i);
                    return rc;
                }
                singleRxMsgQIds[i] = 0;
            }

            rc = osMutexDelete(rxMutex);
            if (rc != GT_OK)
            {
                osPrintSync("osMsgQDelete : osMutexDelete: failed \n");
                return rc;
            }

            rxMutex = 0;

        }


        osPrintSync("appDemoDeviceEventRequestDrvnModeReset: clean DB \n");
        /* clean main DB */
        evHndlrCauseAllArrSize[devNum] = 0;
        evHndlrCauseAllArr[devNum] = NULL;
        /* state that reset of appDemo events finished */
        eventRequestDrvnModeReset = 0;
    }
    /* free and Reset Event Counter DB (done during pre-init*/
    cpssAppUtilsUniEventsFree(devNum);

#ifdef CHX_FAMILY
    if(uniFecEventCounters[devNum] != NULL)
    {
        osFree(uniFecEventCounters[devNum]);
        uniFecEventCounters[devNum] = NULL;
    }
#endif

    return GT_OK;
}

/**
* @internal appDemoEventRequestDrvnModeReset function
* @endinternal
*
* @brief   This routine destroys event handlers tasks, delete message queues, clean
*         bindings for App Demo event handlers.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoEventRequestDrvnModeReset
(
    IN GT_VOID
)
{
    GT_STATUS rc;              /* The returned code            */
    GT_U32    i;               /* Iterator                     */
    GT_U32    retry;


    eventRequestDrvnModeReset = 1;
#ifndef ASIC_SIMULATION
#ifdef DXCH_CODE
    if (waTSmid)
    {
        cpssOsSigSemSignal(waTSmid);

        /* provide time for WA task to exit */
        osTimerWkAfter(20);
        cpssOsSigSemDelete(waTSmid);

        osPrintSync("appDemoEventRequestDrvnModeReset: delete waTSmid \n");
    }
#endif
#endif

    osPrintSync("appDemoEventRequestDrvnModeReset: mask HW events \n");
    for (i = 0; i < APP_DEMO_PROCESS_NUMBER; i++)
    {
        if(evHndlrCauseAllArrSize[i] == 0 || evHndlrCauseAllArr[i][0] == CPSS_UNI_RSRVD_EVENT_E)
        {
            /*osPrintf("appDemoEvHndlr[%d]: not exists \n",
                i);*/
            continue;
        }

        /* call the CPSS to disable those interrupts in the HW of the device */
        rc = prvUniEvMaskAllSet(evHndlrCauseAllArr[i],
                                evHndlrCauseAllArrSize[i],
                                &uniEvSkipArr[0],
                                uniEvSkipArrLength,
                                CPSS_EVENT_MASK_E);
        if (GT_OK != rc)
        {
            osPrintSync("prvUniEvMaskAllSet: failed on [i=%d] \n",i);
            return GT_FAIL;
        }
    }

    osPrintSync("appDemoEventRequestDrvnModeReset: destroy task handlers \n");
    /* changing the mask interrupts registers may invoke 'last minute' interrupts */
    /* let the task handle it before we continue */
    osTimerWkAfter(100);

    for (i = 0; i < APP_DEMO_PROCESS_NUMBER; i++)
    {
        if(evHndlrCauseAllArrSize[i] == 0 || evHndlrCauseAllArr[i][0] == CPSS_UNI_RSRVD_EVENT_E)
        {
            /*osPrintf("appDemoEvHndlr[%d]: not exists \n",
                i);*/
            continue;
        }

        /* call CPSS to destroy the events */
        rc = cpssEventDestroy(taskParamArr[i].evHndl);
        if (GT_OK != rc)
        {
            osPrintSync("cpssEventDestroy: failed on [i=%d] \n",i);
            return GT_FAIL;
        }

        retry = MAX_GRACEFUL_EXIT_RETRY;

        while(taskParamArr[i].hndlrIndex != FREE_TASK_CNS&&(0!=retry))
        {
            /* this is indication that the thread is exited properly ... no need to kill it */
            osTimerWkAfter(1);
            /*osPrintf("8");*/
            retry--;
        }
        /*osPrintf("\n");*/

        if(retry==0)
        {
            osPrintSync("appDemoEventRequestDrvnModeReset:Process %d termination started\n",eventHandlerTidArr[i]);
            /*we gave it a chance ,now kill it*/
            rc =   osTaskDelete(eventHandlerTidArr[i]);
            if (rc != GT_OK)
            {
                osPrintSync("osTaskDelete : osTaskDelete: failed on [i=%d] \n",i);
                return rc;
            }
            osPrintSync("appDemoEventRequestDrvnModeReset:Event handler number %d forcely killed.\n",i);
        }

        taskParamArr[i].hndlrIndex = 0;
        taskParamArr[i].evHndl     = 0;
        eventHandlerTidArr[i]      = 0;
    }


    if (singleTaskRxEnable == GT_TRUE)
    {
        osPrintSync("appDemoEventRequestDrvnModeReset: destroy RX task handlers \n");
        for (i = 0; i < treatTasksNum; i++)
        {
            /* delete rx treat task -- before deleting the mutex/semaphore */
            rc = osTaskDelete(treatRxEventHandlerTid[i]);
            if (rc != GT_OK)
            {
                osPrintSync("singleTaskRxEnable : osTaskDelete: failed on [i=%d] \n",i);
                return rc;
            }

            /* delete msgQ */
            rc = osMsgQDelete(singleRxMsgQIds[i]);
            if (rc != GT_OK)
            {
                osPrintSync("osMsgQDelete : osTaskDelete: failed on [i=%d] \n",i);
                return rc;
            }
            singleRxMsgQIds[i] = 0;
        }

        rc = osMutexDelete(rxMutex);
        if (rc != GT_OK)
        {
            osPrintSync("osMsgQDelete : osMutexDelete: failed \n");
            return rc;
        }

        rxMutex = 0;

    }

    osPrintSync("appDemoEventRequestDrvnModeReset: clean DB \n");
    /* clean main DB */
    for (i = 0; i < APP_DEMO_PROCESS_NUMBER; i++)
    {
        evHndlrCauseAllArrSize[i] = 0;
        evHndlrCauseAllArr[i] = NULL;
    }

#ifdef GM_USED
    osPrintSync("appDemoEventRequestDrvnModeReset: kill gm_appDemoEmulateLinkChangeTask \n");
    rc = osTaskDelete(gm_appDemoEmulateLinkChangeTask_Tid);
    if (rc != GT_OK)
    {
        osPrintSync("osTaskDelete : gm_appDemoEmulateLinkChangeTask_Tid: failed \n");
        return rc;
    }
    gm_appDemoEmulateLinkChangeTask_Tid = (GT_TASK)0;
#endif

    /* debug flag to open trace of events */
    wrapCpssTraceEvents = 0;
    singleTaskRxEnable = GT_FALSE;
    /* is random distribution of packets to msgQs enabled */
    singleTaskRxRandomEnable = GT_FALSE;
    /* number of treat Rx tasks */
    treatTasksNum = 2;

    /* flag meaning appDemoEnPpEvTreat will catch <Signal Detect triggered> interrupt.
    Warning: <Signal Detect triggered> is new interrupt in XCAT2 B1, if enabled using
        cpssDxChPortGePrbsIntReplaceEnableSet it overrides old but not obsolete
        <QSGMII PRBS error> interrupt, so in appDemoEnPpEvTreat it's treated as
        CPSS_PP_PORT_PRBS_ERROR_QSGMII_E case */
    catchSignalDetectInterrupt = GT_FALSE;

    /* flag that state the tasks may process events that relate to AUQ messages.
        this flag allow us to stop processing those messages , by that the AUQ may
        be full, or check AU storm prevention, and other.

        use function appDemoAllowProcessingOfAuqMessages(...) to set this flag
    */
    allowProcessingOfAuqMessages = GT_TRUE;

    /* fatal error handling type */
    prvAppDemoFatalErrorType = CPSS_ENABLER_FATAL_ERROR_EXECUTE_TYPE_E;

    mmpcsPrint = 0;

    osPrintSync("appDemoEventRequestDrvnModeReset: destroy the counters DB + related semaphore \n");
    {/* destroy the counters DB + related semaphore */
        if(appUtilsEventCounterUpdateLockMtx)
        {
            osMutexLock(appUtilsEventCounterUpdateLockMtx);
        }

        /* free and Reset Event Counter DB */
        for(i = 0 ; i < MAX_NUM_DEVICES; i ++)
        {
            cpssAppUtilsUniEventsFree(i);
        }
#ifdef CHX_FAMILY
        /* free and Reset FEC Event Counter DB */
        for(i = 0 ; i < MAX_NUM_DEVICES; i ++)
        {
            if(uniFecEventCounters[i] == NULL)
            {
                continue;
            }
            osFree(uniFecEventCounters[i]);
            uniFecEventCounters[i] = NULL;
        }
#endif
        if(appUtilsEventCounterUpdateLockMtx)
        {
            osMutexUnlock(appUtilsEventCounterUpdateLockMtx);

            osMutexDelete(appUtilsEventCounterUpdateLockMtx);
            appUtilsEventCounterUpdateLockMtx = 0;
        }
    }
    /* state that reset of appDemo events finished */
    eventRequestDrvnModeReset = 0;

    return GT_OK;
}


#ifdef PTP_PLUGIN_SUPPORT

void appDemoRegisterLinkChngFunc (APP_DEMO_LINK_CHNG_FUNC funcPtr)
{
    linkChngFuncPtr = funcPtr;
}

#endif

void appDemoLinkChngEventSimulate
(
    GT_U32 devNum,
    GT_U32 portNum
)
{
    (void)appDemoEnPpEvTreat(devNum,
    CPSS_PP_PORT_LINK_STATUS_CHANGED_E,
    portNum);
}

/**
* @internal appDemoEventsToTestsHandlerUnbind function
* @endinternal
*
* @brief   This routine unbind the unified event list and remove
*          the event handler made in appDemoEventsToTestsHandlerbind .
*
* @param[in] cpssUniEventArr[]        - The CPSS unified event list.
* @param[in] arrLength                - The unified event list length.
*/
GT_STATUS appDemoEventsToTestsHandlerUnbind
(
    IN  CPSS_UNI_EV_CAUSE_ENT       cpssUniEventArr[],
    IN  GT_U32                      arrLength
)
{
    GT_STATUS       rc;            /* return code */

    if (appDemoTestsEventHandlerTid != 0)
    {
        rc  = cpssEventUnBind(cpssUniEventArr, arrLength);
        if(rc != GT_OK)
        {
            return rc ;
        }

        rc = osTaskDelete(appDemoTestsEventHandlerTid);
        if (rc != GT_OK)
        {
            osPrintSync("osTaskDelete : osTaskDelete: failed on [i=%d] \n",appDemoTestsEventHandlerTid);
            return rc;
        }
        appDemoTestsEventHandlerTid = 0;
    }
    return GT_OK;
}
