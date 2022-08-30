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
* @file mvHwsPortCtrlSupervisor.c
*
* @brief Port Control Supervisor State Machine
*
* @version   1
********************************************************************************
*/
#include <mvHwsPortCtrlInc.h>
#include <mvHwsPortCtrlDoorbellEvents.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mv_hws_avago_if.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mvAvagoIf.h>
#include <NetPort.h>
#if defined(AC5_DEV_SUPPORT)
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/mvComphyIf.h>
#endif

static MV_HWS_PORT_CTRL_COMMON *infoPtr;
static GT_U32 spvHighMsgCount = 0;
static GT_U32 spvLowMsgCount  = 0;

#if defined(MV_HWS_FREE_RTOS) && !defined(CM3)
/* This is needed only for service CPU on MSYS */
#include <hw/common.h>
#define IPC_CACHE_INVALIDATE(_ptr,_len) armv7_dcache_wbinv_range((unsigned long)(_ptr), (_len))
#else
#define IPC_CACHE_INVALIDATE(_ptr,_len)
#endif /* MV_HWS_FREE_RTOS */

#ifdef MV_PORT_MTC_WO_TASK
static GT_BOOL mvPortCtrlMtcReady = GT_FALSE;
extern GT_U32 mvPortCtrlAnStopDebugFlag;
#include "mvHwsPortCtrlMtc.h"
#endif
/*#define DBG_IPC_E*/
/**
* @internal mvPortCtrlSpvRoutine function
* @endinternal
*
* @brief   Supervisor process execution sequence
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void mvPortCtrlSpvRoutine(void* pvParameters)
{
#ifndef FALCON_DEV_SUPPORT
    mvPortCtrlSyncLock();
    mvPortCtrlSyncUnlock();
#endif

    infoPtr = &(mvPortCtrlSpv.info);
    for( ;; )
    {
        /*
        ** State Machine Transitions Table
        ** +================+=============+=============+=============+
        ** + Event \ State  + High Msg O1 + Low Msg O2  + Delay O3    +
        ** +================+=============+=============+=============+
        ** + High Msg       +     O1      +     O2      +    ==> O1   +
        ** +================+=============+=============+=============+
        ** + Low Msg        +   ==> O2    +   Invalid   +   Invalid   +
        ** +================+=============+=============+=============+
        ** + Delay          +   Invalid   +   ==> O3    +   Invalid   +
        ** +================+=============+=============+=============+
        */
        if (mvPortCtrlSpv.funcTbl[infoPtr->event][infoPtr->state] != NULL)
        {
            mvPortCtrlSpv.funcTbl[infoPtr->event][infoPtr->state]();
        }
        else
        {
            mvPcPrintf("Error, Port Ctrl, Supervisor Func table: state[%d] event[%d] is NULL\n",
                       infoPtr->state, infoPtr->event);
        }

    }
}

/**
* @internal mvPortCtrlSpvHighMsgExecute function
* @endinternal
*
* @brief   Supervisor High priority message process state processing
*         - Handle "Pizza Arbbiter" Configuration
*         Read message from high priority queue
*         Message received
*         Update database
*         Add Log entry
*/
void mvPortCtrlSpvHighMsgExecute(MV_HWS_IPC_CTRL_MSG_STRUCT *recvMsg)
{
    MV_HWS_IPC_PORT_INFO_STRUCT *msgParams;
#if !defined (RAVEN_DEV_SUPPORT) && !( defined (FALCON_DEV_SUPPORT) && defined (MICRO_INIT) )
    MV_HWS_PORT_CTRL_AP_PORT_TIMER *portTimer;
#else
    MV_HWS_AN_SM_TIMER *portTimer;
#if !( defined (FALCON_DEV_SUPPORT) && defined (MICRO_INIT) )
    MV_HWS_AN_SM_INFO *anSm;
#endif
#endif
    GT_U8 portNumBase = 0;
    GT_U8 portIndex;

#ifdef BC2_DEV_SUPPORT
    if(BobcatA0 == hwsDeviceSpecInfo[0].devType)
    {
        portNumBase = MV_PORT_CTRL_BC2_AP_PORT_NUM_BASE;
    }
    else if(Alleycat3A0 == hwsDeviceSpecInfo[0].devType)
    {
        portNumBase = MV_PORT_CTRL_AC3_AP_PORT_NUM_BASE;
    }
    else
    {
        portNumBase = 0;
    }
#else
    portNumBase = MV_PORT_CTRL_AP_PORT_NUM_BASE;
#endif

    if (recvMsg->ctrlMsgType == MV_HWS_IPC_PORT_AP_SYS_CFG_VALID_MSG)
    {
        /* Extract target port from IPC message and update system config valid status */
        /* System configuration use the format of Port Init message */

        msgParams = &(recvMsg->msgData.apSysConfig);
        portIndex = MV_HWS_PORT_CTRL_AP_LOGICAL_PORT_GET(msgParams->phyPortNum, portNumBase);
#if !defined (RAVEN_DEV_SUPPORT) && !( defined (FALCON_DEV_SUPPORT) && defined (MICRO_INIT) )
        portTimer = &(mvHwsPortCtrlApPortMng.apPortTimer[portIndex]);
#else
        portTimer = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].timer);
#endif
        portTimer->sysCfStatus = PORT_SM_SERDES_SYSTEM_VALID;
        portTimer->sysCfMode = msgParams->portMode;
        mvPortCtrlLogAdd(SPV_LOG(SPV_HIGH_MSG_PROCESS_STATE, recvMsg->ctrlMsgType,
                                 msgParams->phyPortNum, AP_PORT));
        /* TODO - Add handling of system state DISABLE / ENABLE */

        spvHighMsgCount++;
        if (spvHighMsgCount >= mvPortCtrlSpv.highMsgThreshold)
        {
            spvHighMsgCount = 0;
            infoPtr->event = SPV_LOW_MSG_EVENT;
        }
    }
#ifdef RAVEN_DEV_SUPPORT
    else if (recvMsg->ctrlMsgType == MV_HWS_IPC_PORT_AP_ENABLE_WAIT_ACK_MSG)
    {
        /* Extract target port from IPC message and update system config valid status */
        /* System configuration use the format of Port Init message */

        msgParams = &(recvMsg->msgData.apEnableWaitAck);
        portIndex = MV_HWS_PORT_CTRL_AP_LOGICAL_PORT_GET(msgParams->phyPortNum, portNumBase);

        anSm    = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].info);
        if (anSm->status == AN_PORT_ENABLE_WAIT)
        {
            anSm->status = AN_PORT_DELETE_IN_PROGRESS;
        }

        mvPortCtrlLogAdd(SPV_LOG(SPV_HIGH_MSG_PROCESS_STATE, recvMsg->ctrlMsgType,
                                 msgParams->phyPortNum, AP_PORT));
        spvHighMsgCount++;
        if (spvHighMsgCount >= mvPortCtrlSpv.highMsgThreshold)
        {
            spvHighMsgCount = 0;
            infoPtr->event = SPV_LOW_MSG_EVENT;
        }
    }
#endif
}

/**
* @internal mvPortCtrlSpvHighMsg function
* @endinternal
*
* @brief   Supervisor High priority message process state execution
*/
void mvPortCtrlSpvHighMsg(void)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT recvMsg;

    infoPtr->state = SPV_HIGH_MSG_PROCESS_STATE;
    osMemSet(&recvMsg, 0, sizeof(recvMsg));
    /* Process messages from High priority IPC message queue */
    if (mvHwsIpcRequestGet(MV_HWS_IPC_HIGH_PRI_QUEUE,
                           sizeof(MV_HWS_IPC_CTRL_MSG_STRUCT),
                           (char*)&recvMsg) != GT_OK)
    {
        spvHighMsgCount = 0;
        infoPtr->event = SPV_LOW_MSG_EVENT;
    }
    else
    {
        /*printf("H%d ",recvMsg.ctrlMsgType);*/
        #ifdef DBG_IPC_E
        {
             GT_U8 i;
             for (i=0; i<(recvMsg.msgLength+4);i++)
             {
                 printf("%02x ",recvMsg.msgData.ctrlData[i]);
                 if (((i+1) % 16) == 0)
                     printf("\n");
             }
             printf("\n");
        }
        #endif
         mvPortCtrlSpvHighMsgExecute(&recvMsg);
    }
}

/**
* @internal mvPortCtrlSpvLowMsgExecute function
* @endinternal
*
* @brief   Supervisor Low priority message process state processing
*         - Validate system process can receive messgae
*         In case Valid
*         Handle Port Init / Reset IPC Message
*         Add log entry
*         Send message to Port process
*         Handle AP Port IPC Message
*         Add log entry
*         Send message to AP Detection process
*         Handle General IPC Message
*         Add log entry
*         Send message to General process
*         Message not received
*         Trigger state change
*         - Not Valid
*         Trigger state change
*/
void mvPortCtrlSpvLowMsgExecute(MV_HWS_IPC_CTRL_MSG_STRUCT *recvMsg)
{
    MV_HWS_IPC_PORT_INIT_DATA_STRUCT *msgParams;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyMsg;
    GT_UINTPTR avagoAaplAddr;
    unsigned int avagoAaplSize;
    GT_U32 avagoGuiState;
#ifdef AN_PORT_SM
    MV_PORT_CTRL_MODULE module = M7_AN_PORT_SM;
#endif

#ifdef AC5_DEV_SUPPORT
    MV_HWS_SERDES_RX_CONFIG_DATA_UNT    rxSerdesTune;
    MV_HWS_SERDES_TX_CONFIG_DATA_UNT    txSerdesTune;
#endif

    msgParams = (MV_HWS_IPC_PORT_INIT_DATA_STRUCT*)&(recvMsg->msgData);
    switch (recvMsg->ctrlMsgType)
    {
    case MV_HWS_IPC_HWS_LOG_GET_MSG:
        mvHwsLogInfoGet(&replyMsg.readData.logGet.fwBaseAddr,
                &replyMsg.readData.logGet.logBaseAddr,
                &replyMsg.readData.logGet.logPointerAddr,
                &replyMsg.readData.logGet.logCountAddr,
                &replyMsg.readData.logGet.logResetAddr);

        replyMsg.returnCode = GT_OK;
        replyMsg.replyTo = recvMsg->ctrlMsgType;
        mvHwsIpcReplyMsg(recvMsg->msgQueueId, &replyMsg);
        break;

    case MV_HWS_IPC_VOS_OVERRIDE_PARAMS_DATA_MSG:
#ifndef MV_HWS_AVAGO_NO_VOS_WA
        if ( !(recvMsg->msgData.vosOverrideParams.vosOverrideParamsBufferIdx == 0xFFFFFFFF &&
            recvMsg->msgData.vosOverrideParams.vosOverrideParamsBufferLength == 2) )
        {
            replyMsg.returnCode = mvHwsAvagoSerdesManualVosParamsSet(0,
                                &(recvMsg->msgData.vosOverrideParams.vosOverrideParamsBufferPtr[0]),
                                recvMsg->msgData.vosOverrideParams.vosOverrideParamsBufferIdx,
                                recvMsg->msgData.vosOverrideParams.vosOverrideParamsBufferLength);
        }
#ifndef ALDRIN_DEV_SUPPORT
        else if (recvMsg->msgData.vosOverrideParams.vosOverrideParamsBufferIdx == 0xFFFFFFFF &&
            recvMsg->msgData.vosOverrideParams.vosOverrideParamsBufferLength == 2)
        {
            replyMsg.returnCode = mvHwsAvagoSerdesManualVosMinAndBaseForBc3ParamsSet(0,
                                recvMsg->msgData.vosOverrideParams.vosOverrideParamsBufferPtr[0],
                                recvMsg->msgData.vosOverrideParams.vosOverrideParamsBufferPtr[1]);
        }
#endif
#else
        replyMsg.returnCode = GT_OK;
#endif
        replyMsg.replyTo = recvMsg->ctrlMsgType;
        mvHwsIpcReplyMsg(recvMsg->msgQueueId, &replyMsg);
        break;
    case MV_HWS_IPC_VOS_OVERRIDE_MODE_SET_MSG:
#ifndef MV_HWS_AVAGO_NO_VOS_WA
        replyMsg.returnCode = mvHwsAvagoSerdesVosOverrideModeSet(0,
                                recvMsg->msgData.vosOverrideModeSet.vosOverrideMode);
#else
        replyMsg.returnCode = GT_OK;
#endif
        replyMsg.replyTo = recvMsg->ctrlMsgType;
        mvHwsIpcReplyMsg(recvMsg->msgQueueId, &replyMsg);
        break;
    case MV_HWS_IPC_PORT_AVAGO_SERDES_INIT_MSG:

#ifdef MV_SERDES_AVAGO_IP
        mvHwsAvagoAaplAddrGet(0, &avagoAaplAddr, &avagoAaplSize);
        /*printf("aapl0x%08x %d ", avagoAaplAddr, avagoAaplSize);*/
#else
        avagoAaplAddr = 0;
        avagoAaplSize = 0;
        /*printf("Aapl0x%08x %d ",avagoAaplAddr, avagoAaplSize);*/
#endif /* MV_SERDES_AVAGO_IP */
        replyMsg.replyTo    = recvMsg->ctrlMsgType;
        replyMsg.returnCode = GT_OK;
        replyMsg.readData.portAvagoAaplGet.addr = (GT_U32)avagoAaplAddr;
        replyMsg.readData.portAvagoAaplGet.size = avagoAaplSize;
        /* Host is about to deliver the full content of AAPL structure.
         * Need to invalidate this area to be able to see content.*/
        IPC_CACHE_INVALIDATE(avagoAaplAddr, avagoAaplSize);
        mvHwsIpcReplyMsg(recvMsg->msgQueueId, &replyMsg);
        break;
    case MV_HWS_IPC_PORT_AVAGO_SERDES_RESET_DATA_MSG:
#ifdef MV_SERDES_AVAGO_IP
        mvHwsAvagoAaplInit(0,0);
#ifdef MV_PORT_MTC_TASK
        mvPortCtrlMtcUnlock();
#elif MV_PORT_MTC_WO_TASK
        mvPortCtrlMtcReady = GT_TRUE;
#endif
#endif
        replyMsg.replyTo    = recvMsg->ctrlMsgType;
        replyMsg.returnCode = GT_OK;
        mvHwsIpcReplyMsg(recvMsg->msgQueueId, &replyMsg);
        break;
    case MV_HWS_IPC_PORT_AVAGO_GUI_SET_MSG:
        avagoGuiState = recvMsg->msgData.avagoGuiSet.state;
        avagoGuiState &= MV_PORT_CTRL_AVAGO_GUI_MASK;
#ifdef AP_PORT_SM
        mvHwsApDetectStateSet(avagoGuiState);
#endif

#ifdef AN_PORT_SM
        mvHwsAnDetectStateSet(avagoGuiState);
#endif
        replyMsg.replyTo    = recvMsg->ctrlMsgType;
        replyMsg.returnCode = GT_OK;
        mvHwsIpcReplyMsg(recvMsg->msgQueueId, &replyMsg);
        break;

    /* Send messages to Port Mng process message queue */
#ifdef REG_PORT_TASK
    case MV_HWS_IPC_PORT_INIT_MSG:
    case MV_HWS_IPC_PORT_RESET_MSG:
    case MV_HWS_IPC_PORT_RESET_EXT_MSG:
        mvPortCtrlPortSm[msgParams->phyPortNum].type = REG_PORT;
        mvPortCtrlLogAdd(SPV_LOG(SPV_LOW_MSG_PROCESS_STATE, recvMsg->ctrlMsgType,
                                 msgParams->phyPortNum, REG_PORT));
        mvPortCtrlProcessMsgSend(M2_PORT_MNG, recvMsg);
        break;
#endif
    /* Send messages to AP Detection process message queue */
    case MV_HWS_IPC_PORT_AP_ENABLE_MSG:
    case MV_HWS_IPC_PORT_AP_DISABLE_MSG:
    case MV_HWS_IPC_PORT_AP_CFG_GET_MSG:
    case MV_HWS_IPC_PORT_AP_STATUS_MSG:
    case MV_HWS_IPC_PORT_AP_STATS_MSG:
    case MV_HWS_IPC_PORT_AP_STATS_RESET_MSG:
    case MV_HWS_IPC_PORT_AP_INTROP_GET_MSG:
    case MV_HWS_IPC_PORT_AP_INTROP_SET_MSG:
    case MV_HWS_IPC_PORT_AP_DEBUG_GET_MSG:
    case MV_HWS_IPC_PORT_ENABLE_CTRL_SET_MSG:
    case MV_HWS_IPC_PORT_ENABLE_CTRL_GET_MSG:
    case MV_HWS_IPC_PORT_ADAPTIVE_CTLE_ENABLE_MSG:
    case MV_HWS_IPC_NON_AP_PORT_IS_LINK_UP_MSG:
    case MV_HWS_IPC_PORT_LINK_BINDING_SET_MSG:
    case MV_HWS_IPC_PORT_LINK_BINDING_REGISTER_AP_MSG:
    case MV_HWS_IPC_PORT_OPERATION_MSG:
#ifdef RAVEN_DEV_SUPPORT
    case MV_HWS_IPC_AN_RES_BIT_SET_MSG:
#endif

#if (defined (BOBK_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT))
    case MV_HWS_IPC_PORT_SERDES_RX_CONFIG_OVERRIDE_MSG:
#endif /* (defined (BOBK_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)) */
#ifdef REG_PORT_TASK
        mvPortCtrlPortSm[msgParams->phyPortNum].type = AP_PORT;
#endif
        if ((recvMsg->ctrlMsgType == MV_HWS_IPC_PORT_AP_ENABLE_MSG) ||
            (recvMsg->ctrlMsgType == MV_HWS_IPC_PORT_AP_DISABLE_MSG))
        {
            mvPortCtrlLogAdd(SPV_LOG(SPV_LOW_MSG_PROCESS_STATE, recvMsg->ctrlMsgType,
                                     msgParams->phyPortNum, AP_PORT));
        }
#ifdef AP_PORT_SM
        mvPortCtrlProcessMsgSend(M5_AP_PORT_DET, recvMsg);
        break;
#endif
#ifdef AN_PORT_SM
        if (msgParams->phyPortNum >= MV_HWS_PORT_CTRL_AN_NUM_PORTS_PER_TASK_CNS)
        {
            module = M9_AN_PORT_SM2;
        }
        mvPortCtrlProcessMsgSend(module, recvMsg);
        break;
#endif

    case MV_HWS_IPC_NOOP_MSG:
        {
#ifdef MICRO_INIT
            /**
             * This will stop polling task upon cpssInitSystem fastBoot.
             */
            pollingTaskDisableEnable(0);
#endif
#if 1
/* AP to CPSS Doorbell events test */
            if (recvMsg->msgData.noop.data == 8)
                recvMsg->msgData.noop.data = doorbellIntrAddEvent(AP_DOORBELL_EVENT_IPC_E);
            if (recvMsg->msgData.noop.data == 10)
                recvMsg->msgData.noop.data = doorbellIntrAddEvent(AP_DOORBELL_EVENT_PORT_STATUS_CHANGE(30));

#endif
            replyMsg.readData.noopReply.data = recvMsg->msgData.noop.data+2;
            replyMsg.returnCode = GT_OK;
            replyMsg.replyTo = recvMsg->ctrlMsgType;
            mvHwsIpcReplyMsg(recvMsg->msgQueueId, &replyMsg);
            break;
        }

#if (defined (BOBK_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)|| defined(RAVEN_DEV_SUPPORT) || defined (PIPE_DEV_SUPPORT))
    case MV_HWS_IPC_PORT_SERDES_RX_PARAMETERS_OFFSET_CONFIG_MSG:
        mvPortCtrlLogAdd(SPV_LOG(SPV_LOW_MSG_PROCESS_STATE, recvMsg->ctrlMsgType,
                                 msgParams->phyPortNum, AP_PORT));
        replyMsg.returnCode = mvHwsAvagoSerdesTxParametersOffsetSet(recvMsg->devNum,
                  recvMsg->msgData.portSerdesTxParametersOffsetConfig.portGroup,
                  recvMsg->msgData.portSerdesTxParametersOffsetConfig.phyPortNum,
                  recvMsg->msgData.portSerdesTxParametersOffsetConfig.serdesSpeed,
                  recvMsg->msgData.portSerdesTxParametersOffsetConfig.serdesLane,
                  recvMsg->msgData.portSerdesTxParametersOffsetConfig.offsets,
                 &replyMsg.readData.portSerdesTxParametersOffsetInfoCfg.txOffsetBaseAddr,
                 &replyMsg.readData.portSerdesTxParametersOffsetInfoCfg.txOffsetSize);
        replyMsg.replyTo = recvMsg->ctrlMsgType;
        mvHwsIpcReplyMsg(recvMsg->msgQueueId, &replyMsg);
        break;
#endif /* (defined (BOBK_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)) */

#if defined (ALDRIN_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(RAVEN_DEV_SUPPORT) || defined(AC5_DEV_SUPPORT)
    case MV_HWS_IPC_PORT_SERDES_RX_CONFIG_MSG:
        mvPortCtrlLogAdd(SPV_LOG(SPV_LOW_MSG_PROCESS_STATE, recvMsg->ctrlMsgType,
                                 msgParams->phyPortNum, AP_PORT));
#if defined(AC5_DEV_SUPPORT)
        hwsOsMemCopyFuncPtr(&rxSerdesTune.rxComphyC12GP41P2V,
                            &recvMsg->msgData.serdesRxConfig.rxCfg.rxComphyC12GP41P2V,
                            sizeof(MV_HWS_MAN_TUNE_COMPHY_C12GP41P2V_RX_CONFIG_DATA));
        replyMsg.returnCode = mvHwsComphySerdesManualRxDBSet(recvMsg->devNum,
                              recvMsg->msgData.serdesRxConfig.portGroup,
                              recvMsg->msgData.serdesRxConfig.phyPortNum,
                              recvMsg->msgData.serdesRxConfig.serdesSpeed,
                              recvMsg->msgData.serdesRxConfig.serdesLane,
                              &rxSerdesTune);
#else
        replyMsg.returnCode = mvHwsAvagoSerdesManualRxDBSet(recvMsg->devNum,
                              recvMsg->msgData.serdesRxConfig.portGroup,
                              recvMsg->msgData.serdesRxConfig.phyPortNum,
                              recvMsg->msgData.serdesRxConfig.serdesSpeed,
                              recvMsg->msgData.serdesRxConfig.serdesLane,
                              &recvMsg->msgData.serdesRxConfig.rxCfg.rxAvago);
#endif
        replyMsg.replyTo = recvMsg->ctrlMsgType;
        mvHwsIpcReplyMsg(recvMsg->msgQueueId, &replyMsg);
        break;

    case MV_HWS_IPC_PORT_SERDES_TX_CONFIG_MSG:
        mvPortCtrlLogAdd(SPV_LOG(SPV_LOW_MSG_PROCESS_STATE, recvMsg->ctrlMsgType,
                                 msgParams->phyPortNum, AP_PORT));
#if defined(AC5_DEV_SUPPORT)
        hwsOsMemCopyFuncPtr(&txSerdesTune.txComphyC12GP41P2V,
                            &recvMsg->msgData.serdesTxConfig.txCfg.txComphyC12GP41P2V,
                            sizeof(MV_HWS_MAN_TUNE_COMPHY_C12GP41P2V_TX_CONFIG_DATA));
        replyMsg.returnCode = mvHwsComphySerdesManualTxDBSet(recvMsg->devNum,
                              recvMsg->msgData.serdesTxConfig.portGroup,
                              recvMsg->msgData.serdesTxConfig.phyPortNum,
                              recvMsg->msgData.serdesTxConfig.serdesSpeed,
                              recvMsg->msgData.serdesTxConfig.serdesLane,
                              &txSerdesTune);
#else
        replyMsg.returnCode = mvHwsAvagoSerdesManualTxDBSet(recvMsg->devNum,
                              recvMsg->msgData.serdesTxConfig.portGroup,
                              recvMsg->msgData.serdesTxConfig.phyPortNum,
                              recvMsg->msgData.serdesTxConfig.serdesSpeed,
                              recvMsg->msgData.serdesTxConfig.serdesLane,
                              &recvMsg->msgData.serdesTxConfig.txCfg.txAvago);
#endif
        replyMsg.replyTo = recvMsg->ctrlMsgType;
        mvHwsIpcReplyMsg(recvMsg->msgQueueId, &replyMsg);
        break;
#endif

    default:
        replyMsg.returnCode = GT_NOT_SUPPORTED;
        replyMsg.replyTo = recvMsg->ctrlMsgType;
        mvHwsIpcReplyMsg(recvMsg->msgQueueId, &replyMsg);
        break;
    }
    spvLowMsgCount++;
}

/**
* @internal mvPortCtrlSpvLowMsg function
* @endinternal
*
* @brief   Supervisor Low priority message process state execution
*/
void mvPortCtrlSpvLowMsg(void)
{
    GT_STATUS                  rcode;
#ifdef REG_PORT_TASK
    GT_U32                     portMsgNum;
#endif
    GT_U32                     apMsgNum;
#ifdef AP_GENERAL_TASK
    GT_U32                     genMsgNum;
#endif
    MV_HWS_IPC_CTRL_MSG_STRUCT recvMsg;
#ifdef AN_PORT_SM
    GT_U32                     apMsgNum2;
#endif

    infoPtr->state = SPV_LOW_MSG_PROCESS_STATE;

    /* Validate system process can receive messgaes
    ** Threshold is set to max of 6 message at each processing interval
    ** under the validation that the target queue has enough space
    */
#ifdef AP_PORT_SM
    rcode = mvPortCtrlProcessPendMsgNum(M5_AP_PORT_DET, &apMsgNum);
#endif
#ifdef AN_PORT_SM
    rcode = mvPortCtrlProcessPendMsgNum(M7_AN_PORT_SM, &apMsgNum);
    if (rcode == GT_OK)
    {
        rcode = mvPortCtrlProcessPendMsgNum(M9_AN_PORT_SM2, &apMsgNum2);
        apMsgNum += apMsgNum2;
    }

#endif
#ifdef REG_PORT_TASK
    rcode |= mvPortCtrlProcessPendMsgNum(M2_PORT_MNG, &portMsgNum);
#endif
#ifdef AP_GENERAL_TASK
    rcode |= mvPortCtrlProcessPendMsgNum(M6_GENERAL, &genMsgNum);
#endif

    if (rcode != GT_OK)
    {
        mvPcPrintf("Error, Supervisor, Failed to get number of pending messages\n");
        return;
    }
    if ((apMsgNum       < MV_PORT_CTRL_SPV_MSG_EXEC_THRESHOLD) &&
#ifdef AP_GENERAL_TASK
        (genMsgNum      < MV_PORT_CTRL_SPV_MSG_EXEC_THRESHOLD) &&
#endif
        (spvLowMsgCount < mvPortCtrlSpv.lowMsgThreshold))
    {
        /* Process messages from Low priority IPC message queue */
        if (mvHwsIpcRequestGet(MV_HWS_IPC_LOW_PRI_QUEUE,
                               sizeof(MV_HWS_IPC_CTRL_MSG_STRUCT),
                               (char*)&recvMsg) != GT_OK)
        {
#ifdef MV_PORT_MTC_WO_TASK
            if ((mvPortCtrlMtcReady) && (!(mvPortCtrlAnStopDebugFlag & 0x80000)))
            {
                mvPortCtrlPortMaintananceActive();
            }
#endif
            spvLowMsgCount = 0;
            infoPtr->event = SPV_DELAY_EVENT;
        }
        else
        {
            /*printf("L%d ",recvMsg.ctrlMsgType);*/
            #ifdef DBG_IPC_E
            {
                GT_U8 i;
                printf("L ");
                for (i=0; i<(recvMsg.msgLength+4);i++)
                {
                    printf("%02x ",recvMsg.msgData.ctrlData[i]);
                    if (((i+1) % 16) == 0)
                        printf("\n");
                }
                printf("\n");
            }
            #endif
           mvPortCtrlSpvLowMsgExecute(&recvMsg);
        }
    }
    else
    {
#ifdef MV_PORT_MTC_WO_TASK
        if ((mvPortCtrlMtcReady) && (!(mvPortCtrlAnStopDebugFlag & 0x80000)))
        {
            mvPortCtrlPortMaintananceActive();
        }
#endif
        spvLowMsgCount = 0;
        infoPtr->event = SPV_DELAY_EVENT;
    }
}

/**
* @internal mvPortCtrlSpvDelay function
* @endinternal
*
* @brief   Superviosr message delay state execution
*         - Execute delay
*         - Trigger state change
*/
void mvPortCtrlSpvDelay(void)
{
    infoPtr->state = SPV_DELAY_STATE;
    mvPortCtrlProcessDelay(infoPtr->delayDuration);
    infoPtr->event = SPV_HIGH_MSG_EVENT;
}


