
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
* @file mvHwsPortCtrlAn.c
*
* @brief Port Control AP Detection State Machine for Raven
*
* @version   1
********************************************************************************
*/
#include <mvHwsPortCtrlInc.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <mvHwsPortCtrlLog.h>
#include <mvHwsPortCtrlAn.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mv_hws_avago_if.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mvAvagoIf.h>
#include <mvHwsPortCtrlCommonEng.h>

GT_U32 mvHwsAnDetectExecutionEnabled = 0;
GT_U32 mvHws40GAnResolutionBitShift = AP_INT_REG_802_3_NEXT_PG_TX_CODE_16_31_40G_SHIFT;
GT_U8 mvHwsMiMode = 1;

GT_U8               refClockSourceDb[MV_PORT_CTRL_MAX_AP_PORT_NUM];
GT_U32              mvPortCtrlAnStopDebugFlag = 0;
MV_HWS_DFE_MODE     mvPortCtrlLinkUpDfeMode = DFE_PCAL;

extern GT_BOOL                         cfgHalEnable;
extern MV_HWS_AVAGO_SERDES_HAL_TYPES   cfgHalType;
extern GT_U32                          cfgHalRegSelect;
extern GT_32                           cfgHalRegisterValue;
extern GT_U8 stage11d[16] ;
extern GT_U8 calDeltaCntr[16] ;

/**
* @internal mvHwsApDetectStateSet function
* @endinternal
*
* @brief   AP Detection process enable / disable set
* @param[in] state             - AN state machine
*
* */
void mvHwsAnDetectStateSet(GT_U32 state)
{
    mvHwsAnDetectExecutionEnabled = state;
}
#if 0
/**
* @internal mvHwsAnPortEnableCtrlSet function
* @endinternal
*
* @brief   Set the entity responsible to preform port enable
*          after training Host or Service CPU (default value
*         is service CPU)
*
* @param[in] portEnableCtrl        - true: Enable control by
*                                    Service CPU
*                                    false: disable control by
*                                    Service CPU, only from
*                                    host
* */
void mvHwsAnPortEnableCtrlSet(GT_BOOL portEnableCtrl)
{
    mvHwsAnPortEnableCtrl = portEnableCtrl;
}


/**
* @internal mvHwsAnPortEnableCtrlGet function
* @endinternal
*
* @brief   Get the entity responsible to preform port enable
*          after training Host or Service CPU (default value is
*          service CPU
*
* @param[out] portEnableCtrl        - true: Enable control by
*                                    Service CPU
*                                    false: disable control by
*                                    Service CPU, only from
*                                    host
**/
void mvHwsAnPortEnableCtrlGet(GT_BOOL *portEnableCtrl)
{
    *portEnableCtrl = mvHwsAnPortEnableCtrl;
}
#endif
/**
* @internal mvHwsAnResolutionBitSet function
* @endinternal
*
* @brief   Set AN Resolution bit
*
* @param[in] anResBit         - resolution bit shift
**/
void mvHwsAnResolutionBitSet(MV_HWS_PORT_STANDARD portMode, GT_U32 anResBit)
{
    if(portMode ==_40GBase_KR2)
        mvHws40GAnResolutionBitShift = anResBit;
}

/**
* @internal mvHwsAnDetectRoutine function
* @endinternal
*
* @brief execute the Routine (MSG -> ACTIVE -> IDLE->)
*
*/
void mvHwsAnDetectRoutine(void* pvParameters)
{
    unsigned short anTaskNum;
    MV_HWS_PORT_CTRL_COMMON *infoPtr;

    mvPortCtrlSyncLock();
    mvPortCtrlSyncUnlock();
    anTaskNum = *( unsigned short * )pvParameters;

    infoPtr = &(mvHwsPortCtrlAnPortDetect.info[anTaskNum]);
    mvPortCtrlLogAddStatus(MV_PORT_CTRL_LOG_EXTRA_STATUS_SIZE-1, MV_PORT_CTRL_AP_VERSION);

    for( ;; )
    {
        /*
        ** State Machine Transitions Table
        ** +================+=============+=============+=============+
        ** + Event \ State  +  Active O1  +   Msg O2    +  Delay O3   +
        ** +================+=============+=============+=============+
        ** + Active         +     O1      +   Invalid   +    ==> O1   +
        ** +================+=============+=============+=============+
        ** + Msg            +   ==> O2    +     O2      +   Invalid   +
        ** +================+=============+=============+=============+
        ** + Delay          +   Invalid   +   ==> O3    +   Invalid   +
        ** +================+=============+=============+=============+
        */
        if (mvHwsPortCtrlAnPortDetect.funcTbl[infoPtr->event][infoPtr->state] != NULL)
        {
            mvHwsPortCtrlAnPortDetect.funcTbl[infoPtr->event][infoPtr->state](anTaskNum);
        }
        else
        {
            mvPcPrintf("Error, Port Detect, Func table: state[%d] event[%d] is NULL\n",
                       infoPtr->state, infoPtr->event);
        }
    }
}

/**
* @internal mvPortCtrlAnPortDetectionActiveExec function
* @endinternal
*
* @brief   AN Detect Active state execution
*         - Exract port state
*         - Execute state functionality
*
* @param[in] portIndex             - number of physical port
*/
void mvPortCtrlAnPortDetectionActiveExec(GT_U8 portIndex)
{
    MV_HWS_AN_SM_INFO *anSm = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].info);
    /*mvPcPrintf("mvPortCtrlAnPortDetectionActiveExec, PortIndex-%d state %d!!!\n", portIndex, anSm->state);*/

#ifdef DEBUG_AN
    {
        static GT_U8 stateDbg[MV_PORT_CTRL_MAX_AP_PORT_NUM] = {0xff}, statusDbg[MV_PORT_CTRL_MAX_AP_PORT_NUM] = {0xff}, serdesStatusDbg[MV_PORT_CTRL_MAX_AP_PORT_NUM] = {0xff};
        GT_U32                      serdesStatus;
        MV_HWS_PORT_INIT_PARAMS     curPortParams;

        if (GT_OK == hwsPortModeParamsGetToBuffer(0, 0, (GT_U32)anSm->portNum, anSm->portMode, &curPortParams))
        {
            if ((mvAnSerdesStatus(curPortParams.activeLanesList[0],AN_SERDES_AN_ARB_STATE,&serdesStatus)) == GT_OK)
            {
                if ((anSm->state != stateDbg[portIndex]) ||
                    (anSm->status != statusDbg[portIndex]) ||
                    (serdesStatus != serdesStatusDbg[portIndex]))
                {
                    printf("ap task, Port-%d state %d status %d serdesStatus %d\n", portIndex, anSm->state,anSm->status, serdesStatus);
                    stateDbg[portIndex] = anSm->state;
                    statusDbg[portIndex] = anSm->status;
                    serdesStatusDbg[portIndex] = serdesStatus;
                    AP_PRINT_MAC(("mvAnPortAutoNeg : port %d, AN ARB state: 0x%x\n", portIndex, serdesStatus));
                }
            }
        }
    }
#endif
    /* check if port disabled or allPortInChip disabled*/
    if(mvPortCtrlAnStopDebugFlag & ((1 << anSm->portNum)|(0x80000)))
    {
        return;
    }
    switch (anSm->state)
    {
    case AN_PORT_SM_IDLE_STATE:
        break;

   case AN_PORT_SM_AN_STATE:
        mvAnPortAutoNeg(portIndex);
        break;

    case AN_PORT_SM_SERDES_CONFIG_STATE:
        mvAnPortSerdesConfig(portIndex);
        break;

    case AN_PORT_SM_LINK_STATUS_STATE:
        mvAnPortLinkUp(portIndex);
        break;

    case AN_PORT_SM_DELETE_STATE:
    case AN_PORT_SM_DISABLE_STATE:
        mvAnPortDeleteValidate(portIndex);
        break;

    default:
        mvPcPrintf("Error, AN Detect, PortIndex-%d Invalid state %d!!!\n", portIndex, anSm->state);
        break;
    }
}

/**
* @internal mvPortCtrlAnDetectActive function
* @endinternal
*
* @brief   AP Detect Active state execution
*         - Scan all ports and call port execution function
*/
void mvPortCtrlAnDetectActive(GT_U8 anTaskNum)
{
    GT_U8 portIndex;
    GT_U8 maxAnPortNum = MV_PORT_CTRL_MAX_AP_PORT_NUM;
    GT_U8 startAnPort = 0;

    /* first task is responsible for ports 0-7*/
    if (anTaskNum == 0)
    {
        maxAnPortNum = MV_HWS_PORT_CTRL_AN_NUM_PORTS_PER_TASK_CNS;
    }
    else /* second task is responsible for ports 8-15*/
    {
        startAnPort = MV_HWS_PORT_CTRL_AN_NUM_PORTS_PER_TASK_CNS;
    }

    mvHwsPortCtrlAnPortDetect.info[anTaskNum].state = AN_PORT_DETECT_ACTIVE_PORT_STATE;
    for (portIndex = startAnPort; portIndex < maxAnPortNum; portIndex++)
    {
        mvPortCtrlAnPortDetectionActiveExec(portIndex);
    }

    mvHwsPortCtrlAnPortDetect.info[anTaskNum].event = AN_PORT_DETECT_DELAY_EVENT;
    mvPortCtrlLogAddStatus(MV_PORT_CTRL_LOG_EXTRA_STATUS_SIZE-2, mvPortCtrlCurrentTs());
}

/**
* @internal mvPortCtrlAnPortMsgReply function
* @endinternal
*
* @brief   AP Port mng info message reply
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvPortCtrlAnPortMsgReply(MV_HWS_IPC_REPLY_MSG_STRUCT *replyMsg, GT_U8 queueId)
{
    /* Build and Send IPC reply */
    CHECK_STATUS(mvHwsIpcReplyMsg(queueId, replyMsg));

    return GT_OK;
}

/**
* @internal mvPortCtrlAnPortDetectionMsgExec function
* @endinternal
*
* @brief   AP Detect message state execution
*         - Extract Message type, Message params, and Port index
*         - Handle AP Enable message
*         AP State == IDLE
*         Extract and update message parameters
*         Execute port initial configuration
*         Trigger state change
*         AP State != IDLE
*         Send reply error message to calling applicaion
*         - Handle AP Disable message
*         AP State != IDLE
*         Execute port delete
*         AP State == IDLE
*         Print error message
*/
static void mvPortCtrlAnPortDetectionMsgExec(MV_HWS_IPC_CTRL_MSG_STRUCT *msg, GT_U8 anTaskNum)
{
    GT_U8 msgType;
    GT_U8 portIndex;
    MV_HWS_IPC_PORT_AP_DATA_STRUCT *msgParams;
    MV_HWS_AN_SM_INFO *anSm;
    MV_HWS_AN_SM_STATS *anStats;
    MV_HWS_AN_SM_TIMER *anTimer;
    MV_HWS_AP_DETECT_ITEROP *anIntrop;
    MV_HWS_IPC_PORT_AP_INTROP_STRUCT  *apIntropSetMsg;
#if 0
    MV_HWS_IPC_PORT_ENABLE_CTRL_DATA_STRUCT  *portEnableCtrlSetMsg;
#endif
    MV_HWS_IPC_PORT_OPERATION_STRUCT  *portOperationMsg;
    MV_HWS_IPC_AN_RES_BIT_INFO_STC  *anResBitMsg;
    GT_U8 portNumBase = 0;
    GT_U32 maxLanes, serdesIndex;

    MV_HWS_IPC_REPLY_MSG_STRUCT replyMsg;
    MV_HWS_PORT_INIT_PARAMS     curPortParams;

    msgType = msg->ctrlMsgType;
    msgParams = (MV_HWS_IPC_PORT_AP_DATA_STRUCT*)&(msg->msgData);

    replyMsg.replyTo    = msgType;
    replyMsg.returnCode = GT_OK;

    portNumBase = MV_PORT_CTRL_AP_PORT_NUM_BASE;

    /* Validate input port number */
    if ((msgParams->phyPortNum < portNumBase) &&
        (msgType != MV_HWS_IPC_PORT_AP_INTROP_GET_MSG) &&
        (msgType != MV_HWS_IPC_PORT_AP_INTROP_SET_MSG) &&
        (msgType != MV_HWS_IPC_PORT_AP_DEBUG_GET_MSG))
    {
        AP_PRINT_MAC(("mvPortCtrlAnPortDetectionMsgExec GT_BAD_PARAM msgParams->phyPortNum %d portNumBase %d\n",msgParams->phyPortNum, portNumBase));
        replyMsg.returnCode = GT_BAD_PARAM;
        mvPortCtrlAnPortMsgReply(&replyMsg, msg->msgQueueId);
        return;
    }

    portIndex = MV_HWS_PORT_CTRL_AP_LOGICAL_PORT_GET(msgParams->phyPortNum, portNumBase);
    anSm     = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].info);
    anStats  = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].stats);
    anTimer = &(mvHwsPortCtrlAnPortDetect.anPortSm[portIndex].timer);
    /*AN_PRINT_MAC(("DETECT port:%d advertise msgType:%d\n",msgParams->phyPortNum, msgType));*/

    switch (msgType)
    {
    case MV_HWS_IPC_PORT_AP_ENABLE_MSG:
        refClockSourceDb[msgParams->phyPortNum] = msgParams->refClockSrcParams;
        if (anSm->state == AN_PORT_SM_IDLE_STATE)
        {
            /*apSm->queueId = msg->msgQueueId;*/
            AP_CTRL_QUEUEID_SET(anSm->ifNum, msg->msgQueueId)
            anSm->portNum = msgParams->phyPortNum;
            AP_CTRL_LANE_SET(anSm->ifNum, msgParams->laneNum);

           /* AP_CTRL_MAC_SET(anSm->ifNum, msgParams->macNum);*/
            anSm->capability = 0;
            AP_CTRL_ADV_ALL_SET(anSm->capability, msgParams->advMode);
            anSm->options = 0;
            AP_CTRL_LB_EN_SET(anSm->options, AP_CTRL_LB_EN_GET(msgParams->options));
            AP_CTRL_FC_PAUSE_SET(anSm->options, AP_CTRL_FC_PAUSE_GET(msgParams->options));
            AP_CTRL_FC_ASM_SET(anSm->options, AP_CTRL_FC_ASM_GET(msgParams->options));
            AP_CTRL_FEC_ABIL_SET(anSm->options, AP_CTRL_FEC_ABIL_GET(msgParams->options));
            AP_CTRL_FEC_REQ_SET(anSm->options, AP_CTRL_FEC_REQ_GET(msgParams->options));
#ifdef CONSORTIUM_SUPPORT
            AP_CTRL_FEC_ABIL_CONSORTIUM_SET(anSm->options, AP_CTRL_FEC_ABIL_CONSORTIUM_GET(msgParams->options));
            AP_CTRL_FEC_REQ_CONSORTIUM_SET(anSm->options, AP_CTRL_FEC_REQ_CONSORTIUM_GET(msgParams->options));
#endif
            AP_CTRL_FEC_ADVANCED_REQ_SET(anSm->options, AP_CTRL_FEC_ADVANCED_REQ_GET(msgParams->options));
            AP_CTRL_CTLE_BIAS_VAL_SET(anSm->options, AP_CTRL_CTLE_BIAS_VAL_GET(msgParams->options));
            AN_CTRL_PRECODING_SET(anSm->options, AN_CTRL_PRECODING_GET(msgParams->options));
            AN_CTRL_SKIP_RES_SET(anSm->options, AN_CTRL_SKIP_RES_GET(msgParams->options));
            AN_CTRL_RX_TRAIN_SET(anSm->options, AN_CTRL_RX_TRAIN_GET(msgParams->options));
            AN_CTRL_IS_OPTICAL_MODE_SET(anSm->options, AN_CTRL_IS_OPTICAL_MODE_GET(msgParams->options));

            AP_PRINT_MAC(("detect port:%d idx:%d capability:0x%x advMode:0x%x options:0x%x\n",msgParams->phyPortNum, portIndex, anSm->capability, msgParams->advMode, anSm->options));
            anSm->polarityVector = msgParams->polarityVector;

            /* set secial speed flag */
            AP_CTRL_SPECIAL_SPEED_SET(anSm->options, AP_CTRL_SPECIAL_SPEED_GET(msgParams->options));

            /* Initial configuration */
            /* set random timer when deleting port, each port will start AP in a different time*/
            mvPortCtrlThresholdSet(((portIndex%8) * 100), &(anTimer->abilityThreshold));
            stage11d[anSm->portNum]=0;
            calDeltaCntr[anSm->portNum]=0xFF;
            /*anSm->status = AN_PORT_START_EXECUTE;*/
            AN_PORT_SM_SET_STATUS(anSm->portNum, anSm, AN_PORT_START_EXECUTE);

            /*anSm->state = AN_PORT_SM_AN_STATE;*/
            AN_PORT_SM_SET_STATE(anSm->portNum, anSm, AN_PORT_SM_AN_STATE);
            mvPortCtrlAnPortMsgReply(&replyMsg, msg->msgQueueId);
        }
        else
        {
            mvPcPrintf("Error, AP Detect, Port-%d is under execution[0x%x], AP Enable is not valid!!!\n", msgParams->phyPortNum, anSm->state);
            replyMsg.returnCode = GT_CREATE_ERROR;
            mvPortCtrlAnPortMsgReply(&replyMsg, msg->msgQueueId);
        }
        break;

    case MV_HWS_IPC_PORT_AP_DISABLE_MSG:


        if (anSm->state != AN_PORT_SM_IDLE_STATE)
        {
            /*delete serdes manual db*/
            maxLanes = mvPortCtrlApPortGetMaxLanes(anSm->capability);
            if (maxLanes > 0)
            {
                curPortParams.numOfActLanes = maxLanes;
                curPortParams.portStandard = _10GBase_KR; /* parameter is ignored */
                curPortParams.firstLaneNum = anSm->portNum;
                hwsPortParamsGetLanes(0, 0, anSm->portNum, &curPortParams);
                for (serdesIndex = 0; serdesIndex < maxLanes; serdesIndex++)
                {
                    mvHwsAvagoSerdesManualDBClear(0, anSm->portNum, curPortParams.activeLanesList[serdesIndex]);
                }
            }

            mvAnPortRemoteFaultEnable(portIndex,GT_FALSE);
            mvAnPortDeleteMsg(portIndex);
            mvPortCtrlAnPortMsgReply(&replyMsg, msg->msgQueueId);
        }
        else
        {
            mvPcPrintf("Error, AP Detect, Port-%d is NOT under execution, AP Disable is not valid!!!\n", msgParams->phyPortNum);
            replyMsg.returnCode = GT_FAIL;
            mvPortCtrlAnPortMsgReply(&replyMsg, msg->msgQueueId);
        }
        break;

    case MV_HWS_IPC_PORT_AP_CFG_GET_MSG:
        replyMsg.readData.portApCfgGet.ifNum      = anSm->ifNum;
        replyMsg.readData.portApCfgGet.capability = anSm->capability;
        replyMsg.readData.portApCfgGet.options    = anSm->options;
        AP_PRINT_MAC(("detect MV_HWS_IPC_PORT_AP_CFG_GET_MSG port:%d idx:%d capability:0x%x options:0x%x\n",msgParams->phyPortNum, portIndex, anSm->capability, anSm->options));
        mvPortCtrlAnPortMsgReply(&replyMsg, msg->msgQueueId);
        break;

    case MV_HWS_IPC_PORT_AP_STATUS_MSG:

          /* return AP disabled only after both state machines reached idle state and application asked to disable AP on port */
        replyMsg.readData.portApStatusGet.state       = anSm->state;
        replyMsg.readData.portApStatusGet.status      = anSm->status;
        replyMsg.readData.portApStatusGet.laneNum     = AP_CTRL_LANE_GET(anSm->ifNum);
        replyMsg.readData.portApStatusGet.ARMSmStatus = anSm->ARBSmStatus;
        replyMsg.readData.portApStatusGet.hcdStatus   = anSm->hcdStatus;
        AP_PRINT_MAC(("detect MV_HWS_IPC_PORT_AP_STATUS_MSG port:%d idx:%d capability:0x%x options:0x%x hcdStatus:0x%x\n",msgParams->phyPortNum, portIndex, anSm->capability, anSm->options,anSm->hcdStatus));
        mvPortCtrlAnPortMsgReply(&replyMsg, msg->msgQueueId);
        break;

    case MV_HWS_IPC_PORT_AP_STATS_MSG:
        replyMsg.readData.portApStatsGet.txDisCnt          = 0 /* using Avago AN */;
        replyMsg.readData.portApStatsGet.abilityCnt        = 0 /* using Avago AN */;
        replyMsg.readData.portApStatsGet.abilitySuccessCnt = 0 /* using Avago AN */;
        replyMsg.readData.portApStatsGet.linkFailCnt       = anStats->linkFailCnt;
        replyMsg.readData.portApStatsGet.linkSuccessCnt    = anStats->linkSuccessCnt;
        replyMsg.readData.portApStatsGet.hcdResoultionTime = anStats->hcdResoultionTime;
        replyMsg.readData.portApStatsGet.linkUpTime        = anStats->linkUpTime;
        mvPortCtrlAnPortMsgReply(&replyMsg, msg->msgQueueId);
        break;

    case MV_HWS_IPC_PORT_AP_STATS_RESET_MSG:
        mvPortCtrlDbgStatsReset(msgParams->phyPortNum);
        mvPortCtrlAnPortMsgReply(&replyMsg, msg->msgQueueId);
        break;

    case MV_HWS_IPC_PORT_AP_INTROP_GET_MSG:
        anIntrop = &(mvHwsPortCtrlAnPortDetect.introp);
        replyMsg.readData.portApIntropGet.txDisDuration          = 0 /* using Avago AN */;
        replyMsg.readData.portApIntropGet.abilityDuration        = anIntrop->abilityDuration;
        replyMsg.readData.portApIntropGet.abilityMaxInterval     = anIntrop->abilityMaxInterval;
        replyMsg.readData.portApIntropGet.abilityFailMaxInterval = anIntrop->abilityFailMaxInterval;
        replyMsg.readData.portApIntropGet.apLinkDuration         = anIntrop->apLinkDuration;
        replyMsg.readData.portApIntropGet.apLinkMaxInterval      = anIntrop->apLinkMaxInterval;
        replyMsg.readData.portApIntropGet.pdLinkDuration         = anIntrop->pdLinkDuration;
        replyMsg.readData.portApIntropGet.pdLinkMaxInterval      = anIntrop->pdLinkMaxInterval;
        replyMsg.readData.portApIntropGet.anPam4LinkMaxInterval  = anIntrop->anPam4LinkMaxInterval;
        mvPortCtrlAnPortMsgReply(&replyMsg, msg->msgQueueId);
        break;

    case MV_HWS_IPC_PORT_AP_INTROP_SET_MSG:
        apIntropSetMsg  = (MV_HWS_IPC_PORT_AP_INTROP_STRUCT*)&(msg->msgData);
        mvPortCtrlDbgIntropCfg(apIntropSetMsg);
        mvPortCtrlAnPortMsgReply(&replyMsg, msg->msgQueueId);
        break;

    case MV_HWS_IPC_PORT_AP_DEBUG_GET_MSG:
        mvPortCtrlLogInfoGet(&replyMsg.readData.logGet.fwBaseAddr,
                &replyMsg.readData.logGet.logBaseAddr,
                &replyMsg.readData.logGet.logPointerAddr,
                &replyMsg.readData.logGet.logCountAddr,
                &replyMsg.readData.logGet.logResetAddr);
        mvPortCtrlAnPortMsgReply(&replyMsg, msg->msgQueueId);
        break;
#if 0
    case MV_HWS_IPC_PORT_ENABLE_CTRL_SET_MSG:
        portEnableCtrlSetMsg = (MV_HWS_IPC_PORT_ENABLE_CTRL_DATA_STRUCT*)&(msg->msgData);
        mvHwsAnPortEnableCtrlSet((GT_BOOL)portEnableCtrlSetMsg->portEnableCtrl);
        mvPortCtrlAnPortMsgReply(&replyMsg, msg->msgQueueId);
        break;

    case MV_HWS_IPC_PORT_ENABLE_CTRL_GET_MSG:
        mvHwsAnPortEnableCtrlGet((GT_BOOL*)&replyMsg.readData.portEnableCtrlGet.portEnableCtrl);
        mvPortCtrlAnPortMsgReply(&replyMsg, msg->msgQueueId);
        break;
#endif
    case MV_HWS_IPC_AN_RES_BIT_SET_MSG:
        anResBitMsg = (MV_HWS_IPC_AN_RES_BIT_INFO_STC*)&(msg->msgData);
        mvHwsAnResolutionBitSet((MV_HWS_PORT_STANDARD)anResBitMsg->portMode,(GT_U32)anResBitMsg->anResBit);
        mvPortCtrlAnPortMsgReply(&replyMsg, msg->msgQueueId);
        break;

    case MV_HWS_IPC_PORT_OPERATION_MSG:
        portOperationMsg = (MV_HWS_IPC_PORT_OPERATION_STRUCT*)&(msg->msgData);
        mvPortCtrlLogAdd(AN_PORT_MNG_LOG_DEBUG_SERDES(msgParams->phyPortNum, LOG_DEBUG_SERDES_OPERATION, portOperationMsg->portOperation));

        /* mvPortCtrlLogAdd(AN_PORT_MNG_LOG_DEBUG(msgParams->phyPortNum, (portOperationMsg->portOperation&0x7), portOperationMsg->portData[0], portOperationMsg->portData[1]));*/
        /* mvPortCtrlLogAdd(AN_PORT_MNG_LOG_DEBUG(msgParams->phyPortNum, (portOperationMsg->portOperation&0x7), portOperationMsg->portData[2], portOperationMsg->portData[3]));*/
        switch (portOperationMsg->portOperation) {
#ifdef RAVEN_DEV_SUPPORT
            case MV_HWS_IPC_PORT_OPERATION_AN_DISABLE:
                if ( (GT_BOOL)portOperationMsg->portData[1] ) /* allPortsInChip*/
                {
                    if ( (GT_BOOL)portOperationMsg->portData[0] )
                    {
                        mvPortCtrlAnStopDebugFlag |= (0x80000 );
                    }
                    else
                    {
                        mvPortCtrlAnStopDebugFlag &= ~(0x80000 );
                    }

                }
                else
                {
                    if ( (GT_BOOL)portOperationMsg->portData[0] )
                    {
                        mvPortCtrlAnStopDebugFlag |= (1<<msgParams->phyPortNum );
                    }
                    else
                    {
                        mvPortCtrlAnStopDebugFlag &= ~(1<<msgParams->phyPortNum );
                    }
                }
                break;

            case MV_HWS_IPC_PORT_OPERATION_LINK_UP_DFE_MODE:
                mvPortCtrlLinkUpDfeMode = (MV_HWS_DFE_MODE)portOperationMsg->portData[0];
                break;
            case MV_HWS_IPC_PORT_OPERATION_CFG_HAL_SET:
                cfgHalEnable        = GT_TRUE;
                cfgHalType          = (MV_HWS_AVAGO_SERDES_HAL_TYPES)portOperationMsg->portData[0];
                cfgHalRegSelect     = (GT_U32)portOperationMsg->portData[1];
                cfgHalRegisterValue = (GT_32)portOperationMsg->portData[2];
                break;

            case MV_HWS_IPC_PORT_OPERATION_INTERCONNECT_PROFILE_SET:
                maxLanes = (GT_U8)portOperationMsg->portData[1];
                if (maxLanes > 0)
                {
                    curPortParams.numOfActLanes = maxLanes;
                    curPortParams.portStandard = _10GBase_KR; /* parameter is ignored */
                    curPortParams.firstLaneNum = msgParams->phyPortNum;
                    hwsPortParamsGetLanes(0, 0, msgParams->phyPortNum, &curPortParams);
                    for (serdesIndex = 0; serdesIndex < maxLanes; serdesIndex++)
                    {
                        mvHwsAvagoSerdesManualInterconnectDBSet(0, 0, msgParams->phyPortNum, curPortParams.activeLanesList[serdesIndex], (GT_U8)portOperationMsg->portData[0]);
                    }
                }
                break;

            case MV_HWS_IPC_PORT_OPERATION_GLOBAL_RX_TERMINATION_SET:
                mvHwsAvagoSerdesDefaultRxTerminationSet(0, 0, (GT_U8)portOperationMsg->portData[0]);
                break;

            case MV_HWS_IPC_PORT_OPERATION_TX_PRESET_SET:
                mvHwsAvagoSerdesManualTxPresetDBSet(0, msgParams->phyPortNum, /* serdes number */
                                                   (GT_U8)portOperationMsg->portData[0] /*spped*/,
                                                   (GT_U8)portOperationMsg->portData[1] /*pre*/,
                                                   (GT_U8)portOperationMsg->portData[2] /* atten */,
                                                   (GT_U8)portOperationMsg->portData[3] /*post*/);
                break;

            case MV_HWS_IPC_PORT_OPERATION_HOST_OR_MI_SET:
                mvHwsMiMode = (GT_U8)portOperationMsg->portData[0];
                break;
#endif
            default:
                break;
        }
        mvPortCtrlAnPortMsgReply(&replyMsg, msg->msgQueueId);
        break;
    }

}

/**
* @internal mvPortCtrlAnDetectMsg function
* @endinternal
*
* @brief   AN Detect message state execution
*         - Read message from AN detection message queue
*         Message received
*         Execute message to completion
*         Message not received
*         Trigger state change
*/
void mvPortCtrlAnDetectMsg(GT_U8 anTaskNum)
{
    GT_STATUS rcode;

    MV_HWS_IPC_CTRL_MSG_STRUCT recvMsg;
    MV_PORT_CTRL_MODULE module = M7_AN_PORT_SM;

    osMemSet(&recvMsg, 0, sizeof(recvMsg));
    mvHwsPortCtrlAnPortDetect.info[anTaskNum].state = AN_PORT_DETECT_MSG_PROCESS_STATE;
    if (anTaskNum)
    {
        module = M9_AN_PORT_SM2;
    }

    do
    {
        rcode = mvPortCtrlProcessMsgRecv(module, MV_PROCESS_MSG_RX_DELAY, &recvMsg);
        if (rcode == GT_OK)
        {
            mvPortCtrlAnPortDetectionMsgExec(&recvMsg, anTaskNum);
        }
    } while (rcode == GT_OK);

    mvHwsPortCtrlAnPortDetect.info[anTaskNum].event = AN_PORT_DETECT_ACTIVE_PORT_EVENT;
}

/**
* @internal mvPortCtrlAnDetectDelay function
* @endinternal
*
* @brief   AN Delay state execution
*/
void mvPortCtrlAnDetectDelay(GT_U8 anTaskNum)
{
    mvHwsPortCtrlAnPortDetect.info[anTaskNum].state = AN_PORT_DETECT_DELAY_STATE;

    do
    {
        /* When Avago GUI is enabled, AN periodic execution is placed on hold
        ** AN process will not execute any activity
        */
        mvPortCtrlProcessDelay(mvHwsPortCtrlAnPortDetect.info[anTaskNum].delayDuration);
    } while (mvHwsAnDetectExecutionEnabled == MV_PORT_CTRL_AVAGO_GUI_ENABLED);

    mvHwsPortCtrlAnPortDetect.info[anTaskNum].event = AN_PORT_DETECT_MSG_PROCESS_EVENT;
}


