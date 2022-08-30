/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
* mvHwsIpcApi.c
*
* DESCRIPTION:
*           This file contains APIs for HWS IPC
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
******************************************************************************/
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIpcDefs.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIpcApis.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>

/**************************** Globals ****************************************************/
#ifndef MV_HWS_REDUCED_BUILD

/* the structure to gather HWS IPC statistics on Host*/
/*MV_HWS_IPC_STATISTICS_STRUCT mvHwsIpcStatistics[HWS_MAX_DEVICE_NUM];*/

/**************************** Pre-Declaration ********************************************/
GT_STATUS mvHwsSetIpcInfo(GT_U8 devNum, GT_U32 msgType, MV_HWS_IPC_CTRL_MSG_STRUCT *msgDataPtr,
                          GT_U32 msgLength);
/*GT_STATUS mvHwsIpcCtrlMsgTx(MV_HWS_IPC_CTRL_MSG_STRUCT *txCtrlMsg);*/
/*GT_STATUS mvHwsIpcReplyMsgRx(GT_U8 devNum, MV_HWS_IPC_CTRL_MSG_DEF_TYPE msgId, GT_U32 queueId,
                             MV_HWS_IPC_REPLY_MSG_STRUCT *rxReplyData);*/

#if defined(CPU_BE)
#include <cpssCommon/cpssPresteraDefs.h>
static void prvMvHwsIpcToLE(MV_HWS_IPC_CTRL_MSG_STRUCT *requestMsg);
static void prvMvHwsIpcFromLE(MV_HWS_IPC_REPLY_MSG_STRUCT *replyMsg);
#endif

/**
* @internal mvHwsIpcDbInit function
* @endinternal
*
* @brief   HW Services Ipc database initialization
*
* @param[in] devNum                   - device number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_VOID mvHwsIpcDbInit
(
    GT_U8 devNum
)
{
    hwsOsMemSetFuncPtr(PRV_SHARED_LAB_SERVICES_DIR_PORT_SRC_GLOBAL_VAR(hwsIpcQueueIdPool)[devNum],0,MV_HWS_MAX_HOST2HWS_REQ_MSG_NUM*sizeof(GT_U32));
    /*hwsOsMemSetFuncPtr(&(mvHwsIpcStatistics[devNum]),0,sizeof(mvHwsIpcStatistics[devNum]));*/
}

/**
* @internal mvHwsGetQueueId function
* @endinternal
*
* @brief   finds free queue for Host 2 HWS connection
*
* @param[in] devNum                   - device number
* @param[in] queueId                  - pointer to queue ID
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_U32 mvHwsGetQueueId
(
    GT_U8 devNum,
    GT_U32 * queueId
)
{
    GT_U32   i;

    for (i = MV_HWS_IPC_TX_0_CH_ID; i < MV_HWS_MAX_HOST2HWS_REQ_MSG_NUM; i++)
    {
        if (PRV_SHARED_LAB_SERVICES_DIR_PORT_SRC_GLOBAL_VAR(hwsIpcQueueIdPool)[devNum][i] == MV_HWS_IPC_FREE_QUEUE)
        {
            *queueId = i;
            PRV_SHARED_LAB_SERVICES_DIR_PORT_SRC_GLOBAL_VAR(hwsIpcQueueIdPool)[devNum][i] = MV_HWS_IPC_QUEUE_BUSY;
            return GT_OK;
        }
    }

    if (i == MV_HWS_MAX_HOST2HWS_REQ_MSG_NUM)
    {
        /*mvHwsIpcStatistics[devNum].mvHwsIpcGenFailureCount++;*/
        hwsOsPrintf("No free Host2Hws TX message\n");
        return GT_NO_RESOURCE;
    }

    return GT_OK;
}

/**
* @internal mvHwsReturnQueueId function
* @endinternal
*
* @brief   returns queue Id to the pool
*
* @param[in] devNum                   - device number
* @param[in] queueId                  - queue Id
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsReturnQueueId
(
    GT_U8 devNum,
    GT_U32 queueId
)
{
    if (queueId >= MV_HWS_MAX_HOST2HWS_REQ_MSG_NUM)
    {
        /*mvHwsIpcStatistics[devNum].mvHwsIpcGenFailureCount++;*/
        hwsOsPrintf("mvHwsReturnQueueId queue ID %d doesn't exist \n", queueId);
        return GT_BAD_PARAM;
    }

    PRV_SHARED_LAB_SERVICES_DIR_PORT_SRC_GLOBAL_VAR(hwsIpcQueueIdPool)[devNum][queueId] = MV_HWS_IPC_FREE_QUEUE;

    return GT_OK;

}
/**
* @internal mvHwsIpcCtrlMsgTxCpuId function
* @endinternal
*
* @brief   Send IPC message from Host to HW Services
*
* @param[in] txCtrlMsg                - pointer to the message
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsIpcCtrlMsgTxCpuId
(
    MV_HWS_IPC_CTRL_MSG_STRUCT *txCtrlMsg,
    GT_U8                       cpuId
)
{
#ifdef ASIC_SIMULATION
    return GT_OK;
#endif
   /* printf("mvHwsIpcCtrlMsgTxCpuId cpuIdx %d type %d\n",cpuId, txCtrlMsg->ctrlMsgType);*/
    /* Check that the message is valid: msg type and queueId are in the defined range */
    if ((txCtrlMsg->ctrlMsgType >= MV_HWS_IPC_LAST_CTRL_MSG_TYPE ) ||
         (txCtrlMsg->msgQueueId >= MV_HWS_MAX_HWS2HOST_REPLY_QUEUE_NUM))
    {
        /*mvHwsIpcStatistics[txCtrlMsg->devNum].mvHwsPortIpcFailureCount[txCtrlMsg->msgData.portGeneral.phyPortNum]++;*/
        hwsOsPrintf ("mvHwsIpcCtrlMsgTx wrong parameter msg type %d queue ID %d",txCtrlMsg->ctrlMsgType,txCtrlMsg->msgQueueId);
        return GT_BAD_PARAM;
    }

    /* Update counter */
    /*mvHwsIpcStatistics[txCtrlMsg->devNum].mvHwsHostTxMsgCount[txCtrlMsg->ctrlMsgType]++;*/

#if defined(CPU_BE)
    /* convert request to LE */
    prvMvHwsIpcToLE(txCtrlMsg);
#endif
    CHECK_STATUS(prvCpssGenericSrvCpuIpcMessageSendCpuId(
            txCtrlMsg->devNum, cpuId,
            ((txCtrlMsg->ctrlMsgType == MV_HWS_IPC_PORT_AP_SYS_CFG_VALID_MSG) ||
             (txCtrlMsg->ctrlMsgType == MV_HWS_IPC_PORT_AP_ENABLE_WAIT_ACK_MSG))
                ? MV_HWS_IPC_HIGH_PRI_QUEUE
                : MV_HWS_IPC_LOW_PRI_QUEUE,
            (GT_U8*)txCtrlMsg,
            sizeof(MV_HWS_IPC_CTRL_MSG_STRUCT)
    ));

    return GT_OK;
}

/**
* @internal mvHwsIpcCtrlMsgTx function
* @endinternal
*
* @brief   Send IPC message from Host to HW Services
*
* @param[in] txCtrlMsg                - pointer to the message
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
/*GT_STATUS mvHwsIpcCtrlMsgTx
(
    MV_HWS_IPC_CTRL_MSG_STRUCT *txCtrlMsg
)
{
    return mvHwsIpcCtrlMsgTxCpuId(txCtrlMsg, txCtrlMsg->msgData.portGeneral.phyPortNum/16);
}*/

/**
* @internal mvHwsSetIpcInfo function
* @endinternal
*
* @brief   writes IPC data to message structure
*
* @param[in] devNum                   - system device number
* @param[in] msgDataPtr               - pointer to message data
* @param[in] msgLength                - message length
* @param[in] msgType                  - message type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSetIpcInfo
(
    GT_U8                        devNum,
    MV_HWS_IPC_CTRL_MSG_DEF_TYPE msgType,
    MV_HWS_IPC_CTRL_MSG_STRUCT   *msgDataPtr,
    GT_U32                       msgLength
)
{
    GT_U32 queueId;

    msgDataPtr->devNum      = devNum;
    msgDataPtr->msgLength   = (GT_U8)msgLength;
    msgDataPtr->ctrlMsgType = (GT_U8)msgType;
    CHECK_STATUS(mvHwsGetQueueId(devNum, &queueId));
    msgDataPtr->msgQueueId  = (GT_U8)queueId;

    return GT_OK;
}

/**
* @internal mvHwsIpcReplyMsgRx function
* @endinternal
*
* @brief   Gets reply from HW Services to the Host
*
* @param[in] devNum                   - system device number
* @param[in] queueId                  - queue ID
* @param[in] msgId                    - message ID
* @param[in] rxReplyData              - pointer to message
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_TIMEOUT               - timeout to receive reply message
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_INITIALIZED       - srvCpu IPC not initialized for this device
*/
#if 0
GT_STATUS mvHwsIpcReplyMsgRx
(
    GT_U8                        devNum,
    MV_HWS_IPC_CTRL_MSG_DEF_TYPE msgId,
    GT_U32                       queueId,
    MV_HWS_IPC_REPLY_MSG_STRUCT  *rxReplyData
)
{
    GT_STATUS rcode;
    GT_U32 replyTimeout = 200;

    /* Wait for message from Service CPU */
    do
    {
        rcode = prvCpssGenericSrvCpuIpcMessageRecv(PRV_SHARED_LAB_SERVICES_DIR_PORT_SRC_GLOBAL_VAR(hwsIpcChannelHandlers)[devNum], queueId, (GT_U8*)rxReplyData, NULL);
        if (rcode != GT_NO_MORE)
        {
            break;
        }
        else
        {
            hwsOsExactDelayPtr(devNum, 0, 1);
            replyTimeout--;
        }
    } while (replyTimeout > 0);

#if defined(CPU_BE)
    /* convert reply from LE to BE */
    prvMvHwsIpcFromLE(rxReplyData);
#endif
    /* Free the queue */
    CHECK_STATUS(mvHwsReturnQueueId(devNum, queueId));

    if (rcode != GT_OK)
    {
        hwsOsPrintf("Failed to read from channel %d\n", queueId);
        return (rcode == GT_NO_MORE) ? GT_TIMEOUT : rcode;
    }

    if (rxReplyData->replyTo != (GT_U32)msgId)
    {
        /*mvHwsIpcStatistics[devNum].mvHwsIpcGenFailureCount++;*/
        hwsOsPrintf ("mvHwsIpceplyMsgRx wrong msg ID %d Expetced %d queue ID %d\n",rxReplyData->replyTo, msgId,queueId);
        return GT_BAD_PARAM;
    }

    /* Updte statistics*/
    /*mvHwsIpcStatistics[devNum].mvHwsHostRxMsgCount[msgId]++;*/

    return GT_OK;
}
#endif

/**
* @internal mvHwsIpcReplyMsgRxCpuId function
* @endinternal
*
* @brief   Gets reply from HW Services to the Host
*
* @param[in] devNum                   - system device number
* @param[in] queueId                  - queue ID
* @param[in] msgId                    - message ID
* @param[in] rxReplyData              - pointer to message
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_TIMEOUT               - timeout to receive reply message
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_INITIALIZED       - srvCpu IPC not initialized for this device
*/
GT_STATUS mvHwsIpcReplyMsgRxCpuId
(
    GT_U8                        devNum,
    GT_U8                        cpuId,
    MV_HWS_IPC_CTRL_MSG_DEF_TYPE msgId,
    GT_U32                       queueId,
    MV_HWS_IPC_REPLY_MSG_STRUCT  *rxReplyData
)
{
    GT_STATUS rcode;
    GT_U32 replyTimeout = 10000;
#ifdef ASIC_SIMULATION
    /* Free the queue */
    CHECK_STATUS(mvHwsReturnQueueId(devNum, queueId));

    return GT_OK;
#endif
    /* Wait for message from Service CPU */
    do
    {
        rcode = prvCpssGenericSrvCpuIpcMessageRecvCpuId(devNum, cpuId, queueId, (GT_U8*)rxReplyData, NULL);
        if (rcode != GT_NO_MORE)
        {
            break;
        }
        else
        {
            hwsOsExactDelayPtr(devNum, 0, 1);
            replyTimeout--;
        }
    } while (replyTimeout > 0);

#if defined(CPU_BE)
    /* convert reply from LE to BE */
    prvMvHwsIpcFromLE(rxReplyData);
#endif
    /* Free the queue */
    CHECK_STATUS(mvHwsReturnQueueId(devNum, queueId));

    if (rcode != GT_OK)
    {
        hwsOsPrintf("Failed to read from channel %d msg ID %d cpu ID %d\n", queueId, msgId,cpuId);
        return (rcode == GT_NO_MORE) ? GT_TIMEOUT : rcode;
    }

    if (rxReplyData->replyTo != (GT_U32)msgId)
    {
        /*mvHwsIpcStatistics[devNum].mvHwsIpcGenFailureCount++;*/
        hwsOsPrintf ("mvHwsIpceplyMsgRx wrong msg ID %d Expetced %d queue ID %d cpu ID %d\n",rxReplyData->replyTo, msgId,queueId, cpuId);
        return GT_BAD_PARAM;
    }

    /* Updte statistics*/
    /*mvHwsIpcStatistics[devNum].mvHwsHostRxMsgCount[msgId]++;*/

    return GT_OK;
}

/**
* @internal mvHwsGetCm3DevPort function
* @endinternal
*
* @brief   Convert from dev port to cm3 parameters according to the msg id
*
* @param[in] devNum               - device number
* @param[in] requestMsg           - pointer to request message
* @param[in] msgId                - message ID
* @param[out] cm3Dev              - cm3 dev number(0..15 per raven)
* @param[out] cm3Port             - cm3 port number (0..15)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_TIMEOUT               - timeout to receive reply message
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_INITIALIZED       - srvCpu IPC not initialized for this device
*/
 GT_STATUS mvHwsGetCm3DevPort
(
    GT_U8                                   devNum,
    MV_HWS_IPC_CTRL_MSG_STRUCT              *requestMsg,
    MV_HWS_IPC_CTRL_MSG_DEF_TYPE            msgId,
    GT_U8                                   *cm3Dev,
    GT_U16                                  *cm3Port,
    GT_U16                                  *origPort
)
{
    GT_STATUS rc = GT_OK;
    GT_U16      port = 0, i;
    MV_HWS_IPC_PORT_AP_DATA_STRUCT      *apCfgPtr;
    MV_HWS_IPC_PORT_IF_GET_DATA_STRUCT  *apCmnStcPtr;
    MV_HWS_IPC_PORT_PARMAS_DATA_STRUCT  *portPrmPtr;
    MV_HWS_IPC_PORT_SERDES_MANUAL_RX_CONFIG_DATA_STRUCT *serdesRxPtr;
    MV_HWS_IPC_PORT_SERDES_MANUAL_TX_CONFIG_DATA_STRUCT *serdesTxPtr;
    MV_HWS_IPC_PORT_SERDES_TX_OFFSETS_STC *serdesTxOffsetPtr;
    MV_HWS_IPC_PORT_SERDES_TX_CONFIG_STC *serdesTxConfigPtr;
    GT_BOOL                             isCpu = GT_FALSE;
    port = port;
    *cm3Dev = 0; /*devNum;*/
    *cm3Port = 0;
    if(HWS_DEV_SILICON_TYPE(devNum) ==  Falcon)
    {
        /* these struct is common for all msg*/
        apCmnStcPtr  = &requestMsg->msgData.portInterfaceGet;
        port = apCmnStcPtr->phyPortNum;
        *origPort = apCmnStcPtr->phyPortNum;
        if (mvHwsMtipIsReducedPort(devNum, port))
        {
            apCmnStcPtr->phyPortNum = 16;
            *cm3Dev = (GT_U8)(port - (hwsDeviceSpecInfo[devNum].portsNum- hwsDeviceSpecInfo[devNum].numOfTiles*4));
            *cm3Port = 16;
            isCpu = GT_TRUE;
        }
        else
        {
            apCmnStcPtr->phyPortNum = apCmnStcPtr->phyPortNum % 16;
            *cm3Dev = (GT_U8)(port/16);
            *cm3Port = port % 16;
        }
        switch (msgId) {
            case MV_HWS_IPC_PORT_AP_ENABLE_MSG:
            case MV_HWS_IPC_PORT_AP_DISABLE_MSG:
                apCfgPtr = &requestMsg->msgData.apConfig;
                if ( isCpu ) {
                    apCfgPtr->laneNum = apCfgPtr->pcsNum = apCfgPtr->macNum = 16;
                }
                else
                {
                    apCfgPtr->laneNum = apCfgPtr->laneNum % 16;
                    apCfgPtr->pcsNum = apCfgPtr->pcsNum % 16;
                    apCfgPtr->macNum = apCfgPtr->macNum % 16;
                }
                break;

            case MV_HWS_IPC_PORT_PARAMS_MSG:
                portPrmPtr = &requestMsg->msgData.portParams;
                if ( isCpu ) {
                    portPrmPtr->portMacNumber = portPrmPtr->portPcsNumber = portPrmPtr->firstLaneNum = 16;
                    portPrmPtr->activeLanesList[0] = 16;
                }
                else
                {
                    portPrmPtr->portMacNumber = portPrmPtr->portMacNumber % 16;
                    portPrmPtr->portPcsNumber = portPrmPtr->portPcsNumber % 16;
                    portPrmPtr->firstLaneNum  = portPrmPtr->firstLaneNum % 16;
                    for(i = 0; i< portPrmPtr->numOfActLanes; i++)
                    {
                        portPrmPtr->activeLanesList[i] = (GT_U8)portPrmPtr->activeLanesList[i] % 16;
                    }
                }
                break;

            case MV_HWS_IPC_PORT_SERDES_RX_PARAMETERS_OFFSET_CONFIG_MSG:
                serdesTxOffsetPtr  = &requestMsg->msgData.portSerdesTxParametersOffsetConfig;
                if ( isCpu ) {
                    serdesTxOffsetPtr->serdesLane = 16;
                }
                else
                {
                    serdesTxOffsetPtr->serdesLane = serdesTxOffsetPtr->serdesLane %16;
                }
                break;

            case MV_HWS_IPC_PORT_SERDES_RX_CONFIG_MSG:
            case MV_HWS_IPC_PORT_SERDES_TX_CONFIG_MSG:
                /* the struct is almost the same, we use only serdesLane*/
                serdesTxConfigPtr  = &requestMsg->msgData.serdesTxConfig;
                if ( isCpu ) {
                    serdesTxConfigPtr->serdesLane = 16;
                }
                else
                {
                    serdesTxConfigPtr->serdesLane = serdesTxConfigPtr->serdesLane %16;
                }
                break;

            case MV_HWS_IPC_PORT_AP_SYS_CFG_VALID_MSG:
            case MV_HWS_IPC_PORT_AP_ENABLE_WAIT_ACK_MSG:
            case MV_HWS_IPC_PORT_AP_CFG_GET_MSG:
            case MV_HWS_IPC_PORT_AP_STATUS_MSG:
            case MV_HWS_IPC_PORT_AP_STATS_MSG:
            case MV_HWS_IPC_PORT_AP_STATS_RESET_MSG:
            case MV_HWS_IPC_PORT_AP_INTROP_GET_MSG:
            case MV_HWS_IPC_PORT_AP_INTROP_SET_MSG:
            case MV_HWS_IPC_PORT_AP_DEBUG_GET_MSG:
            case MV_HWS_IPC_PORT_AVAGO_SERDES_INIT_MSG:
            case MV_HWS_IPC_PORT_AVAGO_SERDES_RESET_DATA_MSG:
            case MV_HWS_IPC_VOS_OVERRIDE_PARAMS_DATA_MSG:
            case MV_HWS_IPC_VOS_OVERRIDE_MODE_SET_MSG:
            case MV_HWS_IPC_PORT_INIT_MSG:
            case MV_HWS_IPC_PORT_RESET_MSG:
            case MV_HWS_IPC_HWS_LOG_GET_MSG:
            case MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_MSG:
            case MV_HWS_IPC_PORT_AUTO_TUNE_STOP_MSG:
            case MV_HWS_IPC_PORT_AUTO_TUNE_STATE_CHK_MSG:
            case MV_HWS_IPC_PORT_LINK_STATUS_GET_MSG:
            case MV_HWS_IPC_PORT_FEC_CONFIG_GET_MSG:
            case MV_HWS_IPC_PORT_TX_ENABLE_GET_MSG:
            case MV_HWS_IPC_PORT_POLARITY_SET_MSG:
            case MV_HWS_IPC_PORT_FEC_CONFIG_MSG:
            case MV_HWS_IPC_PORT_TX_ENABLE_MSG:
            case MV_HWS_IPC_PORT_SIGNAL_DETECT_GET_MSG:
            case MV_HWS_IPC_PORT_CDR_LOCK_STATUS_GET_MSG:
            case MV_HWS_IPC_PORT_LOOPBACK_SET_MSG:
            case MV_HWS_IPC_PORT_LOOPBACK_STATUS_GET_MSG:
            case MV_HWS_IPC_PORT_PPM_SET_MSG:
            case MV_HWS_IPC_PORT_PCS_ACTIVE_STATUS_GET_CONFIG_MSG:
            case MV_HWS_IPC_PORT_FC_STATE_SET_MSG:
            case MV_HWS_IPC_PORT_SERDES_RX_CONFIG_OVERRIDE_MSG:
            case MV_HWS_IPC_PORT_OPERATION_MSG:
            case MV_HWS_IPC_AN_RES_BIT_SET_MSG:
                break;

            case MV_HWS_IPC_PORT_SERDES_MANUAL_RX_CONFIG_MSG:
                serdesRxPtr = &requestMsg->msgData.serdesManualRxConfig;
                if ( isCpu ) {
                    serdesRxPtr->serdesNum = 16;
                }
                else
                {
                    serdesRxPtr->serdesNum = serdesRxPtr->serdesNum % 16;
                }
                return GT_OK;

            case MV_HWS_IPC_PORT_SERDES_MANUAL_TX_CONFIG_MSG:
                serdesTxPtr = &requestMsg->msgData.serdesManualTxConfig;
                if ( isCpu ) {
                    serdesTxPtr->serdesNum = 16;
                }
                else
                {
                    serdesTxPtr->serdesNum = serdesTxPtr->serdesNum % 16;
                }
                return GT_OK;

            case MV_HWS_IPC_NOOP_MSG:
                port = requestMsg->msgData.noop.data;
                requestMsg->msgData.noop.data = port%16;
                *cm3Dev = (GT_U8)(port/16);
                *cm3Port = port % 16;
                break;

            case MV_HWS_IPC_PORT_AVAGO_GUI_SET_MSG:
            default:
                hwsOsPrintf ("mvHwsGetCm3DevPort  msgID %d orig dev:%d port:%d  No need to change\n", msgId, devNum, port);
                return GT_OK;
        }

        /*hwsOsPrintf ("ipc msg %d  port:%d -->cm3:%d port:%d \n", msgId, port, *cm3Dev, *cm3Port);*/

    }
    return rc;
}
/**
* @internal mvHwsUpdateCm3DevPortReply function
* @endinternal
*
* @brief   Convert from dev port to cm3 parameters acourding to
*          the msg id
*
* @param[in] devNum               - device number
* @param[in] cm3Dev               - cm3 device number
* @param[in] msgId                - message ID
* @param[out] replyData           - reply data
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_TIMEOUT               - timeout to receive reply message
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_INITIALIZED       - srvCpu IPC not initialized for this device
*/
GT_STATUS mvHwsUpdateCm3DevPortReply
(
    GT_U8                                   devNum,
    GT_U8                                   cm3Dev,
    MV_HWS_IPC_CTRL_MSG_DEF_TYPE            msgId,
    MV_HWS_IPC_REPLY_MSG_STRUCT             *replyData,
    GT_U16                                  origPort
)
{
    GT_STATUS rc = GT_OK;
    MV_HWS_IPC_PORT_REPLY_AP_STATUS_GET *apStatusIpc;
    MV_HWS_IPC_PORT_REPLY_AP_CFG_GET    *apCfgIpc;
    GT_BOOL     isCpu = GT_FALSE;

    GT_UNUSED_PARAM(origPort);
    if(HWS_DEV_SILICON_TYPE(devNum) ==  Falcon)
    {
        /* these struct is common for all msg*/
        /*apCmnStcPtr  = &requestMsg->msgData.portInterfaceGet;*/
        if (mvHwsMtipIsReducedPort(devNum, origPort))
        {
            isCpu = GT_TRUE;
        }

        switch (msgId) {
            case MV_HWS_IPC_PORT_AP_STATUS_MSG:
                apStatusIpc = &(replyData->readData.portApStatusGet);
                if ( isCpu )
                {
                    apStatusIpc->laneNum = origPort;
                }
                else
                {
                    apStatusIpc->laneNum += cm3Dev * 16;
                }
                break;
            case MV_HWS_IPC_PORT_AP_CFG_GET_MSG:
                apCfgIpc = &(replyData->readData.portApCfgGet);
                if ( isCpu )
                {
                    apCfgIpc->ifNum = origPort;
                }
                else
                {
                    apCfgIpc->ifNum += cm3Dev * 16;
                }
                break;
            default:
                return GT_OK;
        }

    }
    return rc;

}
/**
* @internal mvHwsIpcSendRequestAndGetReply function
* @endinternal
*
* @brief   Gets reply from HW Services to the Host
*
* @param[in] requestMsg               - pointer to request message
* @param[in] replyData                - pointer to reply message
* @param[in] msgId                    - message ID
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_TIMEOUT               - timeout to receive reply message
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_INITIALIZED       - srvCpu IPC not initialized for this device
*/
GT_STATUS mvHwsIpcSendRequestAndGetReply
(
    GT_U8                                   devNum,
    MV_HWS_IPC_CTRL_MSG_STRUCT              *requestMsg,
    MV_HWS_IPC_REPLY_MSG_STRUCT             *replyData,
    GT_U32                                  msgLength,
    MV_HWS_IPC_CTRL_MSG_DEF_TYPE            msgId
)
{
/*#ifdef ASIC_SIMULATION
    (GT_VOID)devNum;
    (GT_VOID)*requestMsg;
    (GT_VOID)*replyData;
    (GT_VOID)msgLength;
    (GT_VOID)msgId;
#else*/
    GT_STATUS rc;
    GT_U8 cm3Dev;
    GT_U16 cm3Port, origPort;
    CPSS_SYSTEM_RECOVERY_INFO_STC  oldSystemRecoveryInfo,newSystemRecoveryInfo;
    if (HWS_DEV_SIP_6_10_CHECK_MAC(devNum))
    {
        hwsOsPrintf("mvHwsIpcSendRequestAndGetReply: msg %d was used in sip 6_10 \n",msgId);
        return GT_NOT_SUPPORTED;
    }

    CHECK_STATUS(mvHwsGetCm3DevPort(devNum, requestMsg, msgId, &cm3Dev, &cm3Port, &origPort));
    /* Set IPC info */
    CHECK_STATUS(mvHwsSetIpcInfo (devNum,msgId,requestMsg, msgLength));
    /* save recovery state*/
    rc = cpssSystemRecoveryStateGet(&oldSystemRecoveryInfo);
    if (rc != GT_OK)
    {
        return rc;
    }
    newSystemRecoveryInfo = oldSystemRecoveryInfo;


    newSystemRecoveryInfo.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
    prvCpssSystemRecoveryStateUpdate(&newSystemRecoveryInfo);

    /* Send IPC message */
    /*CHECK_STATUS(mvHwsIpcCtrlMsgTx (requestMsg));*/
    rc = mvHwsIpcCtrlMsgTxCpuId(requestMsg, cm3Dev);
    if (rc != GT_OK)
    {
        prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
        /* Free the queue */
        CHECK_STATUS(mvHwsReturnQueueId(devNum, requestMsg->msgQueueId));
        return rc;
    }

    /* Wait for reply */
    /*CHECK_STATUS(mvHwsIpcReplyMsgRx (requestMsg->devNum,msgId,requestMsg->msgQueueId,replyData));*/
    rc = mvHwsIpcReplyMsgRxCpuId (requestMsg->devNum, cm3Dev, msgId, requestMsg->msgQueueId, replyData);
    if (rc != GT_OK)
    {
        prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    rc = mvHwsUpdateCm3DevPortReply(devNum, cm3Dev, msgId, replyData, origPort);
    if (rc != GT_OK)
    {
        prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
/*#endif*/
    return GT_OK;
}

/**
* @internal mvHwsIpcSendMsgNoReply function
* @endinternal
*
* @brief   Send a message to HW Services
*         No reply expected
* @param[in] requestMsg               - pointer to request message
* @param[in] msgId                    - message ID
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsIpcSendMsgNoReply
(
    GT_U8                                   devNum,
    MV_HWS_IPC_CTRL_MSG_STRUCT              *requestMsg,
    GT_U32                                  msgLength,
    MV_HWS_IPC_CTRL_MSG_DEF_TYPE            msgId
)
{
/*#ifdef ASIC_SIMULATION
    (GT_VOID)devNum;
    (GT_VOID)*requestMsg;
    (GT_VOID)msgLength;
    (GT_VOID)msgId;
#else*/
    GT_U8 cm3Dev;
    GT_U16 cm3Port, origPort;
    /* Set IPC info */
    if (HWS_DEV_SIP_6_10_CHECK_MAC(devNum))
    {
        hwsOsPrintf("mvHwsIpcSendRequestAndGetReply: msg %d was used in sip 6_10 \n",msgId);
        return GT_NOT_SUPPORTED;
    }

    requestMsg->devNum      = devNum;
    requestMsg->msgLength   = (GT_U8)msgLength;
    requestMsg->ctrlMsgType = (GT_U8)msgId;
    requestMsg->msgQueueId  = 0;

    CHECK_STATUS(mvHwsGetCm3DevPort(devNum, requestMsg, msgId, &cm3Dev, &cm3Port, &origPort));

    /* Send IPC message */
    /*CHECK_STATUS(mvHwsIpcCtrlMsgTx (requestMsg));*/
    CHECK_STATUS(mvHwsIpcCtrlMsgTxCpuId(requestMsg, cm3Dev));

/*#endif*/
    return GT_OK;
}

#if defined(CPU_BE)

#define PRV_TO_LE_FUNC(_type) \
    static void PRV_TO_LE_ ## _type(_type *r)
PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_AP_DATA_STRUCT)
{
    r->portGroup  = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->advMode    = CPSS_32BIT_LE(r->advMode);
    r->options    = CPSS_32BIT_LE(r->options);
    r->polarityVector = CPSS_16BIT_LE(r->polarityVector);
}
PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_INFO_STRUCT)
{
    r->portGroup  = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->portMode   = CPSS_32BIT_LE(r->portMode);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_AP_INTROP_STRUCT)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->portMode = CPSS_32BIT_LE(r->portMode);
    r->attrBitMask = CPSS_16BIT_LE(r->attrBitMask);
    r->txDisDuration = CPSS_16BIT_LE(r->txDisDuration);
    r->abilityDuration = CPSS_16BIT_LE(r->abilityDuration);
    r->abilityMaxInterval = CPSS_16BIT_LE(r->abilityMaxInterval);
    r->abilityFailMaxInterval = CPSS_16BIT_LE(r->abilityFailMaxInterval);
    r->apLinkDuration = CPSS_16BIT_LE(r->apLinkDuration);
    r->apLinkMaxInterval = CPSS_16BIT_LE(r->apLinkMaxInterval);
    r->pdLinkDuration = CPSS_16BIT_LE(r->pdLinkDuration);
    r->pdLinkMaxInterval = CPSS_16BIT_LE(r->pdLinkMaxInterval);
#ifdef AN_PORT_SM
    r->anPam4LinkMaxInterval = CPSS_16BIT_LE(r->anPam4LinkMaxInterval);
#endif
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_AVAGO_GUI_STRUCT)
{
    r->state = CPSS_32BIT_LE(r->state);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_VOS_OVERRIDE_PARAMS)
{
    GT_U32 i;
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->portMode = CPSS_32BIT_LE(r->portMode);
    for (i = 0; i < 8; i++)
        r->vosOverrideParamsBufferPtr[i] = CPSS_32BIT_LE(r->vosOverrideParamsBufferPtr[i]);
    r->vosOverrideParamsBufferIdx = CPSS_32BIT_LE(r->vosOverrideParamsBufferIdx);
    r->vosOverrideParamsBufferLength = CPSS_32BIT_LE(r->vosOverrideParamsBufferLength);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_VOS_OVERRIDE_MODE_STRUCT)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->portMode = CPSS_32BIT_LE(r->portMode);
    r->vosOverrideMode = CPSS_32BIT_LE(r->vosOverrideMode);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_NOOP_DATA_STRUCT)
{
    r->data = CPSS_32BIT_LE(r->data);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_INIT_DATA_STRUCT)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->portMode = CPSS_32BIT_LE(r->portMode);
    r->lbPort = CPSS_32BIT_LE(r->lbPort);
    r->refClock = CPSS_32BIT_LE(r->refClock);
    r->refClockSource = CPSS_32BIT_LE(r->refClockSource);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_RESET_DATA_STRUCT)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->portMode = CPSS_32BIT_LE(r->portMode);
    r->action = CPSS_32BIT_LE(r->action);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_DATA_STRUCT)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->portMode = CPSS_32BIT_LE(r->portMode);
    r->portTuningMode = CPSS_32BIT_LE(r->portTuningMode);
    r->optAlgoMask = CPSS_32BIT_LE(r->optAlgoMask);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_AUTO_TUNE_STOP_DATA_STRUCT)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->portMode = CPSS_32BIT_LE(r->portMode);
    r->stopRx = CPSS_32BIT_LE(r->stopRx);
    r->stopTx = CPSS_32BIT_LE(r->stopTx);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_POLARITY_SET_DATA_STRUCT)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->portMode = CPSS_32BIT_LE(r->portMode);
    r->txInvMask = (GT_32)(CPSS_32BIT_LE(r->txInvMask));
    r->rxInvMask = (GT_32)(CPSS_32BIT_LE(r->rxInvMask));
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_FEC_CONFIG_DATA_STRUCT)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->portMode = CPSS_32BIT_LE(r->portMode);
    r->portFecType = CPSS_32BIT_LE(r->portFecType);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_TX_ENABLE_DATA_STRUCT)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->portMode = CPSS_32BIT_LE(r->portMode);
    r->enable = CPSS_32BIT_LE(r->enable);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_LOOPBACK_SET_DATA_STRUCT)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->portMode = CPSS_32BIT_LE(r->portMode);
    r->lpPlace = CPSS_32BIT_LE(r->lpPlace);
    r->lbType = CPSS_32BIT_LE(r->lbType);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_LOOPBACK_GET_DATA_STRUCT)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->portMode = CPSS_32BIT_LE(r->portMode);
    r->lpPlace = CPSS_32BIT_LE(r->lpPlace);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_PPM_SET_DATA_STRUCT)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->portMode = CPSS_32BIT_LE(r->portMode);
    r->portPPM = CPSS_32BIT_LE(r->portPPM);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_IF_GET_DATA_STRUCT)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_SERDES_MANUAL_RX_CONFIG_DATA_STRUCT)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->serdesNum = CPSS_32BIT_LE(r->serdesNum);
    r->serdesType = CPSS_32BIT_LE(r->serdesType);
    r->portTuningMode = CPSS_32BIT_LE(r->portTuningMode);
    r->sqlch = CPSS_32BIT_LE(r->sqlch);
    r->ffeRes = CPSS_32BIT_LE(r->ffeRes);
    r->ffeCap = CPSS_32BIT_LE(r->ffeCap);
    r->dfeEn = CPSS_32BIT_LE(r->dfeEn);
    r->alig = CPSS_32BIT_LE(r->alig);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_SERDES_MANUAL_TX_CONFIG_DATA_STRUCT)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->serdesNum = CPSS_32BIT_LE(r->serdesNum);
    r->serdesType = CPSS_32BIT_LE(r->serdesType);
    r->manTxTuneStc.txComphyH.txAmp      = CPSS_32BIT_LE(r->manTxTuneStc.txComphyH.txAmp);
    r->manTxTuneStc.txComphyH.txAmpAdj   = CPSS_32BIT_LE(r->manTxTuneStc.txComphyH.txAmpAdj);
    r->manTxTuneStc.txComphyH.txEmph0    = CPSS_32BIT_LE(r->manTxTuneStc.txComphyH.txEmph0);
    r->manTxTuneStc.txComphyH.txEmph1    = CPSS_32BIT_LE(r->manTxTuneStc.txComphyH.txEmph1);
    r->manTxTuneStc.txComphyH.txAmpShft  = CPSS_32BIT_LE(r->manTxTuneStc.txComphyH.txAmpShft);
    r->manTxTuneStc.txComphyH.txEmphEn   = CPSS_32BIT_LE(r->manTxTuneStc.txComphyH.txEmphEn);
    r->manTxTuneStc.txComphyH.slewCtrlEn   = CPSS_32BIT_LE(r->manTxTuneStc.txComphyH.slewCtrlEn);
    r->manTxTuneStc.txComphyH.slewRate   = CPSS_32BIT_LE(r->manTxTuneStc.txComphyH.slewRate);

    r->manTxTuneStc.txAvago.atten        = (r->manTxTuneStc.txAvago.atten);
    r->manTxTuneStc.txAvago.post         = (r->manTxTuneStc.txAvago.post);
    r->manTxTuneStc.txAvago.pre          = (r->manTxTuneStc.txAvago.pre);
    r->manTxTuneStc.txAvago.pre2         = (r->manTxTuneStc.txAvago.pre2);
    r->manTxTuneStc.txAvago.pre3         = (r->manTxTuneStc.txAvago.pre3);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_PCS_ACTIVE_STATUS_GET_DATA_STRUCT)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->portMode = CPSS_32BIT_LE(r->portMode);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_FLOW_CONTROL_SET_DATA_STRUCT)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->portMode = CPSS_32BIT_LE(r->portMode);
    r->fcState = CPSS_32BIT_LE(r->fcState);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_ENABLE_CTRL_DATA_STRUCT)
{
    r->portEnableCtrl = CPSS_32BIT_LE(r->portEnableCtrl);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_SERDES_TX_OFFSETS_STC)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->serdesSpeed = CPSS_32BIT_LE(r->serdesSpeed);
    r->serdesLane = r->serdesLane /*one byte length */;
    r->offsets = CPSS_16BIT_LE(r->offsets);
}
PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_ADAPTIVE_CTLE_ENABLE_STC)
{
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->enable = r->enable /*one byte length */;
}


PRV_TO_LE_FUNC(MV_HWS_IPC_NON_AP_PORT_IS_LINK_UP_STC)
{
    GT_U8 i;
    r->portGroup = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum = CPSS_16BIT_LE(r->phyPortNum);
    r->isLinkUp = r->isLinkUp; /*one byte length */
    for (i = 0; i < MV_HWS_IPC_MAX_LANES_NUM; i++)
    {
       r->trainLf[i] = r->trainLf[i]; /*one byte length */
       r->enhTrainDelay[i] = r->enhTrainDelay[i]; /*one byte length */
       r->serdesList[i] = /*CPSS_16BIT_LE*/(r->serdesList[i]);
    }
    r->numOfSerdeses = r->numOfSerdeses; /*one byte length */
}


PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_SERDES_RX_CONFIG_OVERRIDE_STC)
{
    r->portGroup                      = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum                     = CPSS_16BIT_LE(r->phyPortNum);
    r->rxCfg.serdesSpeed              = CPSS_32BIT_LE(r->rxCfg.serdesSpeed);
    r->rxCfg.ctleParams.dcGain        = CPSS_16BIT_LE(r->rxCfg.ctleParams.dcGain);
    r->rxCfg.ctleParams.lowFrequency  = CPSS_16BIT_LE(r->rxCfg.ctleParams.lowFrequency);
    r->rxCfg.ctleParams.highFrequency = CPSS_16BIT_LE(r->rxCfg.ctleParams.highFrequency);
    r->rxCfg.ctleParams.bandWidth     = CPSS_16BIT_LE(r->rxCfg.ctleParams.bandWidth);
    r->rxCfg.ctleParams.squelch       = CPSS_16BIT_LE(r->rxCfg.ctleParams.squelch);
    r->rxCfg.etlParams.etlMinDelay  = r->rxCfg.etlParams.etlMinDelay;
    r->rxCfg.etlParams.etlMaxDelay  = r->rxCfg.etlParams.etlMaxDelay;
    r->rxCfg.etlParams.etlEnableOverride = r->rxCfg.etlParams.etlEnableOverride;
    r->rxCfg.fieldOverrideBmp         = CPSS_16BIT_LE(r->rxCfg.fieldOverrideBmp);
}


PRV_TO_LE_FUNC(MV_HWS_IPC_LINK_BINDING_STRUCT)
{
    r->portGroup    = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum   = CPSS_16BIT_LE(r->phyPortNum);
    r->phyPairNum   = CPSS_16BIT_LE(r->phyPairNum);
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_OPERATION_STRUCT)
{
    GT_U8 i;
    r->portGroup        = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum       = CPSS_16BIT_LE(r->phyPortNum);
    r->portOperation    = CPSS_32BIT_LE(r->portOperation);
    for (i = 0; i < MV_HWS_IPC_PORT_OPERATION_DATA_LEN_CNS; i++)
    {
        r->portData[i]         = CPSS_32BIT_LE(r->portData[i]);
    }
}

PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_SERDES_TX_CONFIG_STC)
{
    r->portGroup        = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum       = CPSS_16BIT_LE(r->phyPortNum);
    r->serdesSpeed      = CPSS_16BIT_LE(r->serdesSpeed);
    r->serdesLane       = CPSS_16BIT_LE(r->serdesLane);
    r->serdesType       = CPSS_32BIT_LE(r->serdesType);
    /* all the rest are U8 */
}
PRV_TO_LE_FUNC(MV_HWS_IPC_PORT_SERDES_RX_CONFIG_STC)
{
    r->portGroup        = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum       = CPSS_16BIT_LE(r->phyPortNum);
    r->serdesSpeed      = CPSS_16BIT_LE(r->serdesSpeed);
    r->serdesLane       = CPSS_16BIT_LE(r->serdesLane);
    r->serdesType       = CPSS_32BIT_LE(r->serdesType);
    if (r->serdesType == AVAGO_16NM)
    {
        r->rxCfg.rxAvago.squelch = CPSS_16BIT_LE(r->rxCfg.rxAvago.squelch);
        /* all the rest are U8 */
    }
}

PRV_TO_LE_FUNC(MV_HWS_IPC_AN_RES_BIT_INFO_STC)
{
    r->portGroup        = CPSS_16BIT_LE(r->portGroup);
    r->phyPortNum       = CPSS_16BIT_LE(r->phyPortNum);
    r->portMode         = CPSS_32BIT_LE(r->portMode);
    r->anResBit         = CPSS_32BIT_LE(r->anResBit);

}

/**
* @internal prvMvHwsIpcToLE function
* @endinternal
*
* @brief   Convert request message to Little Endian
*         Required when CPSS runs on CPU other than Little endian
* @param[in] requestMsg               - pointer to request message
*
* @param[out] requestMsg               - pointer to request message
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static void prvMvHwsIpcToLE
(
    MV_HWS_IPC_CTRL_MSG_STRUCT              *requestMsg
)
{
#define PRV_TO_LE(_type,_member) \
    PRV_TO_LE_ ## _type(&(requestMsg->msgData._member)); break

    switch ((MV_HWS_IPC_CTRL_MSG_DEF_TYPE)(requestMsg->ctrlMsgType))
    {
        case MV_HWS_IPC_PORT_AP_ENABLE_MSG:
        case MV_HWS_IPC_PORT_AP_DISABLE_MSG:
        case MV_HWS_IPC_PORT_AP_CFG_GET_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_AP_DATA_STRUCT, apConfig);
        case MV_HWS_IPC_PORT_AP_SYS_CFG_VALID_MSG:
        case MV_HWS_IPC_PORT_AP_ENABLE_WAIT_ACK_MSG:
        case MV_HWS_IPC_PORT_AP_STATUS_MSG:
        case MV_HWS_IPC_PORT_AP_STATS_MSG:
        case MV_HWS_IPC_PORT_AP_STATS_RESET_MSG:
        case MV_HWS_IPC_PORT_AP_INTROP_GET_MSG:
        case MV_HWS_IPC_PORT_AP_DEBUG_GET_MSG:
        case MV_HWS_IPC_PORT_PARAMS_MSG:
        case MV_HWS_IPC_PORT_AVAGO_SERDES_INIT_MSG:
        case MV_HWS_IPC_PORT_AVAGO_SERDES_RESET_DATA_MSG:
        case MV_HWS_IPC_HWS_LOG_GET_MSG:
        case MV_HWS_IPC_PORT_AUTO_TUNE_STATE_CHK_MSG:
        case MV_HWS_IPC_PORT_LINK_STATUS_GET_MSG:
        case MV_HWS_IPC_PORT_FEC_CONFIG_GET_MSG:
        case MV_HWS_IPC_PORT_TX_ENABLE_GET_MSG:
        case MV_HWS_IPC_PORT_SIGNAL_DETECT_GET_MSG:
        case MV_HWS_IPC_PORT_CDR_LOCK_STATUS_GET_MSG:
        case MV_HWS_IPC_PORT_PPM_GET_MSG:
        case MV_HWS_IPC_PORT_ENABLE_CTRL_GET_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_INFO_STRUCT, portLinkStatus);
        case MV_HWS_IPC_PORT_AP_INTROP_SET_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_AP_INTROP_STRUCT, apIntropSet);
        case MV_HWS_IPC_PORT_AVAGO_GUI_SET_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_AVAGO_GUI_STRUCT, avagoGuiSet);
        case MV_HWS_IPC_VOS_OVERRIDE_PARAMS_DATA_MSG:
            PRV_TO_LE(MV_HWS_IPC_VOS_OVERRIDE_PARAMS, vosOverrideParams);
        case MV_HWS_IPC_VOS_OVERRIDE_MODE_SET_MSG:
            PRV_TO_LE(MV_HWS_IPC_VOS_OVERRIDE_MODE_STRUCT, vosOverrideModeSet);
#if 0
        case MV_HWS_IPC_EXECUTE_CMDLINE_MSG:
            /*MV_HWS_IPC_PORT_INIT_DATA_STRUCT*/
            /* don't convert */
            break;
#endif
        case MV_HWS_IPC_NOOP_MSG:
            PRV_TO_LE(MV_HWS_IPC_NOOP_DATA_STRUCT, noop);
        case MV_HWS_IPC_PORT_INIT_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_INIT_DATA_STRUCT, portInit);
        case MV_HWS_IPC_PORT_RESET_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_RESET_DATA_STRUCT, portReset);
        case MV_HWS_IPC_PORT_RESET_EXT_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_INIT_DATA_STRUCT, portInit);
        case MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_DATA_STRUCT, portAutoTuneSetExt);
        case MV_HWS_IPC_PORT_AUTO_TUNE_STOP_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_AUTO_TUNE_STOP_DATA_STRUCT, portAutoTuneStop);
        case MV_HWS_IPC_PORT_SERDES_RESET_MSG:
            /*TODO Unused ?*/
            break;
        case MV_HWS_IPC_PORT_POLARITY_SET_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_POLARITY_SET_DATA_STRUCT, portPolaritySet);
        case MV_HWS_IPC_PORT_FEC_CONFIG_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_FEC_CONFIG_DATA_STRUCT, portFecConfig);
        case MV_HWS_IPC_PORT_TX_ENABLE_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_TX_ENABLE_DATA_STRUCT, portTxEnableData);
        case MV_HWS_IPC_PORT_LOOPBACK_SET_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_LOOPBACK_SET_DATA_STRUCT, portLoopbackSet);
        case MV_HWS_IPC_PORT_LOOPBACK_STATUS_GET_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_LOOPBACK_GET_DATA_STRUCT, portLoopbackGet);
        case MV_HWS_IPC_PORT_PPM_SET_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_PPM_SET_DATA_STRUCT, portPPMSet);
        case MV_HWS_IPC_PORT_IF_GET_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_IF_GET_DATA_STRUCT, portInterfaceGet);
        case MV_HWS_IPC_PORT_SERDES_MANUAL_RX_CONFIG_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_SERDES_MANUAL_RX_CONFIG_DATA_STRUCT, serdesManualRxConfig);
        case MV_HWS_IPC_PORT_SERDES_MANUAL_TX_CONFIG_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_SERDES_MANUAL_TX_CONFIG_DATA_STRUCT, serdesManualTxConfig);
        case MV_HWS_IPC_PORT_PCS_ACTIVE_STATUS_GET_CONFIG_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_PCS_ACTIVE_STATUS_GET_DATA_STRUCT, portPcsActiveStatus);
        case MV_HWS_IPC_PORT_FC_STATE_SET_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_FLOW_CONTROL_SET_DATA_STRUCT, portFcStateSet);
        case MV_HWS_IPC_PORT_ENABLE_CTRL_SET_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_ENABLE_CTRL_DATA_STRUCT, portEnableCtrlSet);
        case MV_HWS_IPC_PORT_SERDES_RX_PARAMETERS_OFFSET_CONFIG_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_SERDES_TX_OFFSETS_STC, portSerdesTxParametersOffsetConfig);
        case MV_HWS_IPC_PORT_SERDES_RX_CONFIG_OVERRIDE_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_SERDES_RX_CONFIG_OVERRIDE_STC, portSerdesRxConfigOverride);
        case MV_HWS_IPC_PORT_ADAPTIVE_CTLE_ENABLE_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_ADAPTIVE_CTLE_ENABLE_STC, adaptCtlePortEnable);
        case MV_HWS_IPC_NON_AP_PORT_IS_LINK_UP_MSG:
            PRV_TO_LE(MV_HWS_IPC_NON_AP_PORT_IS_LINK_UP_STC, nonApPortIsLinkUp);
        case MV_HWS_IPC_PORT_LINK_BINDING_SET_MSG:
        case MV_HWS_IPC_PORT_LINK_BINDING_REGISTER_AP_MSG:
            PRV_TO_LE(MV_HWS_IPC_LINK_BINDING_STRUCT, portLinkBinding);
        case MV_HWS_IPC_PORT_OPERATION_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_OPERATION_STRUCT, portOperation);
        case MV_HWS_IPC_PORT_SERDES_TX_CONFIG_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_SERDES_TX_CONFIG_STC, serdesTxConfig);
        case MV_HWS_IPC_PORT_SERDES_RX_CONFIG_MSG:
            PRV_TO_LE(MV_HWS_IPC_PORT_SERDES_RX_CONFIG_STC, serdesRxConfig);
        case MV_HWS_IPC_AN_RES_BIT_SET_MSG:
            PRV_TO_LE(MV_HWS_IPC_AN_RES_BIT_INFO_STC,anResBitSet);

        case MV_HWS_IPC_LAST_CTRL_MSG_TYPE:
            break;

        default:
            break;
    }
}


#define PRV_FROM_LE_FUNC(_type) \
    static void PRV_FROM_LE_ ## _type(_type *r)

PRV_FROM_LE_FUNC(MV_HWS_IPC_PORT_REPLY_AP_CFG_GET)
{
    r->ifNum = CPSS_32BIT_LE(r->ifNum);
    r->capability = CPSS_32BIT_LE(r->capability);
    r->options = CPSS_32BIT_LE(r->options);
}

PRV_FROM_LE_FUNC(MV_HWS_IPC_PORT_REPLY_AP_STATUS_GET)
{
    r->state = CPSS_16BIT_LE(r->state);
    r->status = CPSS_16BIT_LE(r->status);
    r->laneNum = CPSS_16BIT_LE(r->laneNum);
    r->ARMSmStatus = CPSS_16BIT_LE(r->ARMSmStatus);
    r->hcdStatus = CPSS_32BIT_LE(r->hcdStatus);
}

PRV_FROM_LE_FUNC(MV_HWS_IPC_PORT_REPLY_AP_STATS_GET)
{
    r->txDisCnt = CPSS_16BIT_LE(r->txDisCnt);
    r->abilityCnt = CPSS_16BIT_LE(r->abilityCnt);
    r->abilitySuccessCnt = CPSS_16BIT_LE(r->abilitySuccessCnt);
    r->linkFailCnt = CPSS_16BIT_LE(r->linkFailCnt);
    r->linkSuccessCnt = CPSS_16BIT_LE(r->linkSuccessCnt);
    r->hcdResoultionTime = CPSS_32BIT_LE(r->hcdResoultionTime);
    r->linkUpTime = CPSS_32BIT_LE(r->linkUpTime);
}

PRV_FROM_LE_FUNC(MV_HWS_IPC_PORT_REPLY_AP_INTROP_GET)
{
    r->txDisDuration = CPSS_16BIT_LE(r->txDisDuration);
    r->abilityDuration = CPSS_16BIT_LE(r->abilityDuration);
    r->abilityMaxInterval = CPSS_16BIT_LE(r->abilityMaxInterval);
    r->abilityFailMaxInterval = CPSS_16BIT_LE(r->abilityFailMaxInterval);
    r->apLinkDuration = CPSS_16BIT_LE(r->apLinkDuration);
    r->apLinkMaxInterval = CPSS_16BIT_LE(r->apLinkMaxInterval);
    r->pdLinkDuration = CPSS_16BIT_LE(r->pdLinkDuration);
    r->pdLinkMaxInterval = CPSS_16BIT_LE(r->pdLinkMaxInterval);

#ifdef RAVEN_DEV_SUPPORT
    r->anPam4LinkMaxInterval = CPSS_16BIT_LE(r->anPam4LinkMaxInterval);
#endif
}

PRV_FROM_LE_FUNC(MV_HWS_IPC_LOG_GET)
{
    r->fwBaseAddr = CPSS_32BIT_LE(r->fwBaseAddr);
    r->logBaseAddr = CPSS_32BIT_LE(r->logBaseAddr);
    r->logCountAddr = CPSS_32BIT_LE(r->logCountAddr);
    r->logPointerAddr = CPSS_32BIT_LE(r->logPointerAddr);
    r->logResetAddr = CPSS_32BIT_LE(r->logResetAddr);
}

PRV_FROM_LE_FUNC(MV_HWS_IPC_PORT_REPLY_AVAGO_AAPL_GET)
{
    r->addr = CPSS_32BIT_LE(r->addr);
    r->size = CPSS_32BIT_LE(r->size);
}

PRV_FROM_LE_FUNC(MV_HWS_IPC_NOOP_REPLY)
{
    r->data = CPSS_32BIT_LE(r->data);
}

PRV_FROM_LE_FUNC(MV_HWS_IPC_PORT_REPLY_AUTO_TUNE_STATE_CHK)
{
    r->rxTune = CPSS_32BIT_LE(r->rxTune);
    r->txTune = CPSS_32BIT_LE(r->txTune);
}

PRV_FROM_LE_FUNC(MV_HWS_IPC_PORT_REPLY_STATUS_GET)
{
    r->status = CPSS_32BIT_LE(r->status);
}

PRV_FROM_LE_FUNC(MV_HWS_IPC_PORT_REPLY_PER_SERDES_STATUS_GET)
{
    GT_U32 i;
    for (i = 0; i < MV_HWS_MAX_LANES_NUM_PER_PORT; i++)
        r->status[i] = CPSS_32BIT_LE(r->status[i]);
}

PRV_FROM_LE_FUNC(MV_HWS_IPC_PORT_REPLY_LOOPBACK_STATUS_GET)
{
    r->lbType = CPSS_32BIT_LE(r->lbType);
}

PRV_FROM_LE_FUNC(MV_HWS_IPC_PORT_REPLY_PPM_GET)
{
    r->portPpm = CPSS_32BIT_LE(r->portPpm);
}

PRV_FROM_LE_FUNC(MV_HWS_IPC_PORT_REPLY_IF_GET)
{
    r->portIf = CPSS_32BIT_LE(r->portIf);
}

PRV_FROM_LE_FUNC(MV_HWS_IPC_PORT_REPLY_RESULT)
{
    r->results = CPSS_32BIT_LE(r->results);
}

PRV_FROM_LE_FUNC(MV_HWS_IPC_PORT_ENABLE_CTRL_DATA_STRUCT)
{
    r->portEnableCtrl = CPSS_32BIT_LE(r->portEnableCtrl);
}

PRV_FROM_LE_FUNC(MV_HWS_IPC_PORT_TX_OFFSETS_INFO_STRUCT)
{
    r->txOffsetBaseAddr = CPSS_32BIT_LE(r->txOffsetBaseAddr);
    r->txOffsetSize = CPSS_32BIT_LE(r->txOffsetSize);
}

/**
* @internal prvMvHwsIpcFromLE function
* @endinternal
*
* @brief   Convert request message to Little Endian
*         Required when CPSS runs on CPU other than Little endian
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static void prvMvHwsIpcFromLE
(
    MV_HWS_IPC_REPLY_MSG_STRUCT              *replyMsg
)
{
    replyMsg->returnCode = CPSS_32BIT_LE(replyMsg->returnCode);
    replyMsg->replyTo = CPSS_32BIT_LE(replyMsg->replyTo);
#define PRV_FROM_LE(_type,_member) \
    PRV_FROM_LE_ ## _type(&(replyMsg->readData._member)); break
    switch ((MV_HWS_IPC_CTRL_MSG_DEF_TYPE)(replyMsg->replyTo))
    {
        case MV_HWS_IPC_PORT_AP_ENABLE_MSG:
        case MV_HWS_IPC_PORT_AP_DISABLE_MSG:
        case MV_HWS_IPC_PORT_AP_STATS_RESET_MSG:
        case MV_HWS_IPC_PORT_AP_INTROP_SET_MSG:
        case MV_HWS_IPC_PORT_PARAMS_MSG:
        case MV_HWS_IPC_PORT_AVAGO_GUI_SET_MSG:
        case MV_HWS_IPC_PORT_AVAGO_SERDES_RESET_DATA_MSG:
        case MV_HWS_IPC_VOS_OVERRIDE_PARAMS_DATA_MSG:
        case MV_HWS_IPC_VOS_OVERRIDE_MODE_SET_MSG:
#if 0
        case MV_HWS_IPC_EXECUTE_CMDLINE_MSG:
#endif
        case MV_HWS_IPC_PORT_INIT_MSG:
        case MV_HWS_IPC_PORT_RESET_MSG:
        case MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_MSG:
        case MV_HWS_IPC_PORT_AUTO_TUNE_STOP_MSG:
        case MV_HWS_IPC_PORT_POLARITY_SET_MSG:
        case MV_HWS_IPC_PORT_FEC_CONFIG_MSG:
        case MV_HWS_IPC_PORT_TX_ENABLE_MSG:
        case MV_HWS_IPC_PORT_LOOPBACK_SET_MSG:
        case MV_HWS_IPC_PORT_PPM_SET_MSG:
        case MV_HWS_IPC_PORT_SERDES_MANUAL_RX_CONFIG_MSG:
        case MV_HWS_IPC_PORT_SERDES_MANUAL_TX_CONFIG_MSG:
        case MV_HWS_IPC_PORT_FC_STATE_SET_MSG:
        case MV_HWS_IPC_PORT_ENABLE_CTRL_SET_MSG:
        case MV_HWS_IPC_PORT_RESET_EXT_MSG:
        case MV_HWS_IPC_PORT_SERDES_RESET_MSG:
        case MV_HWS_IPC_PORT_SERDES_RX_CONFIG_OVERRIDE_MSG:
        case MV_HWS_IPC_PORT_ADAPTIVE_CTLE_ENABLE_MSG:
        case MV_HWS_IPC_PORT_LINK_BINDING_SET_MSG:
        case MV_HWS_IPC_PORT_LINK_BINDING_REGISTER_AP_MSG:
        case MV_HWS_IPC_NON_AP_PORT_IS_LINK_UP_MSG:
        case MV_HWS_IPC_PORT_OPERATION_MSG:
            /* just returnCode */
            break;
        case MV_HWS_IPC_PORT_AP_SYS_CFG_VALID_MSG:
        case MV_HWS_IPC_PORT_AP_ENABLE_WAIT_ACK_MSG:
            /* No reply for this message */
            break;

        case MV_HWS_IPC_PORT_AP_CFG_GET_MSG:
            PRV_FROM_LE(MV_HWS_IPC_PORT_REPLY_AP_CFG_GET, portApCfgGet);
        case MV_HWS_IPC_PORT_AP_STATUS_MSG:
            PRV_FROM_LE(MV_HWS_IPC_PORT_REPLY_AP_STATUS_GET, portApStatusGet);
        case MV_HWS_IPC_PORT_AP_STATS_MSG:
            PRV_FROM_LE(MV_HWS_IPC_PORT_REPLY_AP_STATS_GET, portApStatsGet);
        case MV_HWS_IPC_PORT_AP_INTROP_GET_MSG:
            PRV_FROM_LE(MV_HWS_IPC_PORT_REPLY_AP_INTROP_GET, portApIntropGet);
        case MV_HWS_IPC_PORT_AP_DEBUG_GET_MSG:
        case MV_HWS_IPC_HWS_LOG_GET_MSG:
            PRV_FROM_LE(MV_HWS_IPC_LOG_GET, logGet);
        case MV_HWS_IPC_PORT_AVAGO_SERDES_INIT_MSG:
            PRV_FROM_LE(MV_HWS_IPC_PORT_REPLY_AVAGO_AAPL_GET, portAvagoAaplGet);
        case MV_HWS_IPC_NOOP_MSG:
            PRV_FROM_LE(MV_HWS_IPC_NOOP_REPLY, noopReply);
        case MV_HWS_IPC_PORT_AUTO_TUNE_STATE_CHK_MSG:
            PRV_FROM_LE(MV_HWS_IPC_PORT_REPLY_AUTO_TUNE_STATE_CHK, portAutoTuneStateChk);
        case MV_HWS_IPC_PORT_LINK_STATUS_GET_MSG:
        case MV_HWS_IPC_PORT_FEC_CONFIG_GET_MSG:
            PRV_FROM_LE(MV_HWS_IPC_PORT_REPLY_STATUS_GET, portStatusGet);
        case MV_HWS_IPC_PORT_TX_ENABLE_GET_MSG:
        case MV_HWS_IPC_PORT_SIGNAL_DETECT_GET_MSG:
        case MV_HWS_IPC_PORT_CDR_LOCK_STATUS_GET_MSG:
            PRV_FROM_LE(MV_HWS_IPC_PORT_REPLY_PER_SERDES_STATUS_GET, portSerdesTxEnableGet);
        case MV_HWS_IPC_PORT_LOOPBACK_STATUS_GET_MSG:
            PRV_FROM_LE(MV_HWS_IPC_PORT_REPLY_LOOPBACK_STATUS_GET, portLoopbackStatusGet);
        case MV_HWS_IPC_PORT_PPM_GET_MSG:
            PRV_FROM_LE(MV_HWS_IPC_PORT_REPLY_PPM_GET, portPpmGet);
        case MV_HWS_IPC_PORT_IF_GET_MSG:
            PRV_FROM_LE(MV_HWS_IPC_PORT_REPLY_IF_GET, portIfGet);
        case MV_HWS_IPC_PORT_PCS_ACTIVE_STATUS_GET_CONFIG_MSG:
            PRV_FROM_LE(MV_HWS_IPC_PORT_REPLY_RESULT, portReplyGet);
        case MV_HWS_IPC_PORT_ENABLE_CTRL_GET_MSG:
            PRV_FROM_LE(MV_HWS_IPC_PORT_ENABLE_CTRL_DATA_STRUCT, portEnableCtrlGet);
        case MV_HWS_IPC_PORT_SERDES_RX_PARAMETERS_OFFSET_CONFIG_MSG:
            PRV_FROM_LE(MV_HWS_IPC_PORT_TX_OFFSETS_INFO_STRUCT, portSerdesTxParametersOffsetInfoCfg);
        case MV_HWS_IPC_LAST_CTRL_MSG_TYPE:
            break;
        default:
            break;
    }
}
#endif /* defined(CPU_BE) */

#endif /* MV_HWS_REDUCED_BUILD */


