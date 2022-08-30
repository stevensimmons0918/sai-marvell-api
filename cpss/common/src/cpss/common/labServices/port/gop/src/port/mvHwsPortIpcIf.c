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
* mvHwsHost2HwsIfWraper.c
*
* DESCRIPTION: Port external interface
*
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 55 $
******************************************************************************/

#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortApInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCtrlApInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIpcDefs.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortApInitIfPrv.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIpcDefs.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCtrlApDefs.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#ifdef _VISUALC
#pragma warning( disable : 4204)
#endif

#if defined (FALCON_DEV_SUPPORT) && defined (MICRO_INIT)
#include <cpss/common/labServices/port/gop/port/mvHwsIpcApis.h>
#endif

#ifndef MICRO_INIT

/**************************** Globals ****************************************************/



/**************************** Pre-Declaration ********************************************/
extern GT_U32 txParamesOffsetsFwBaseAddr;
extern GT_U32 txParamesOffsetsFwSize;

/**
* @internal mvHwsIpcSendPortParams function
* @endinternal
*
* @brief   Sends to HWS request to set port parameters.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsIpcSendPortParams
(
    GT_U8                    devNum,
    GT_U32                   portGroup,
    GT_U32                   phyPortNum,
    MV_HWS_PORT_STANDARD     portMode
)
{
    GT_U8 i;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_IPC_CTRL_MSG_STRUCT requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_PARMAS_DATA_STRUCT portParams;

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* get port params */
    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        hwsOsPrintf("mvHwsIpcSendPortParams: portMode %d for port %d is not supported\n", portMode, phyPortNum);
        return GT_NOT_SUPPORTED;
    }

    /* fill IPC message data */
    portParams.portGroup = (GT_U16)portGroup;
    portParams.phyPortNum = (GT_U16)phyPortNum;
    portParams.portStandard = portMode;
    portParams.portMacType = curPortParams.portMacType;
    portParams.portMacNumber = curPortParams.portMacNumber;
    portParams.portPcsType = curPortParams.portPcsType;
    portParams.portPcsNumber = curPortParams.portPcsNumber;
    portParams.portFecMode = curPortParams.portFecMode;
    portParams.serdesSpeed = curPortParams.serdesSpeed;
    portParams.firstLaneNum = (GT_U8)curPortParams.firstLaneNum;
    portParams.numOfActLanes = (GT_U8)curPortParams.numOfActLanes;
    /* set active lane list*/
    for(i = 0; i< curPortParams.numOfActLanes; i++)
    {
        portParams.activeLanesList[i] = (GT_U8)curPortParams.activeLanesList[i];
    }
    portParams.serdesMediaType = curPortParams.serdesMediaType;
    portParams.serdes10BitStatus = curPortParams.serdes10BitStatus;

    /* Construct the msg */
    requestMsg.msgData.portParams = portParams;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_PARMAS_DATA_STRUCT),
                                                MV_HWS_IPC_PORT_PARAMS_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortInitIpc function
* @endinternal
*
* @brief   Sends to HWS request to init physical port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] lbPort                   - if true, init port without serdes activity
* @param[in] refClock                 - Reference clock frequency
* @param[in] refClock                 - Reference clock source line
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortInitIpc
(
    GT_U8                    devNum,
    GT_U32                   portGroup,
    GT_U32                   phyPortNum,
    MV_HWS_PORT_STANDARD     portMode,
    GT_BOOL                  lbPort,
    MV_HWS_REF_CLOCK_SUP_VAL refClock,
    MV_HWS_REF_CLOCK_SOURCE  refClockSource)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_INIT_DATA_STRUCT portInit =
        {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode, lbPort, refClock, refClockSource};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portInit = portInit;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_INIT_DATA_STRUCT),
                                                MV_HWS_IPC_PORT_INIT_MSG));

    return (replyData.returnCode);
}
/**
* @internal mvHwsPortResetIpc function
* @endinternal
*
* @brief   Sends to HWS request to power down or reset physical port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*                                      lbPort     - if true, init port without serdes activity
* @param[in] action                   - Power down or reset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortResetIpc
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               phyPortNum,
    MV_HWS_PORT_STANDARD portMode,
    MV_HWS_PORT_ACTION   action
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_RESET_DATA_STRUCT portReset = {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode, action};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portReset = portReset;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_RESET_DATA_STRUCT),
                                                MV_HWS_IPC_PORT_RESET_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortAutoTuneStateCheckIpc function
* @endinternal
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*                                      portTuningMode - port TX related tuning mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAutoTuneStateCheckIpc
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_AUTO_TUNE_STATUS *rxTune,
    MV_HWS_AUTO_TUNE_STATUS *txTune
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_INFO_STRUCT portAutoTuneStateChk = {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    requestMsg.msgData.portAutoTuneStateChk = portAutoTuneStateChk;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_PORT_AUTO_TUNE_STATE_CHK_MSG));

    /* Update reply parameters */
    *rxTune = replyData.readData.portAutoTuneStateChk.rxTune;
    *txTune = replyData.readData.portAutoTuneStateChk.txTune;

    return (replyData.returnCode);
}
/**
* @internal mvHwsPortLinkStatusGetIpc function
* @endinternal
*
* @brief   Returns port link status.
*         Can run at any time.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortLinkStatusGetIpc
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               phyPortNum,
    MV_HWS_PORT_STANDARD portMode,
    GT_BOOL              *linkStatus
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_INFO_STRUCT portLinkStatus = {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portLinkStatus = portLinkStatus;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_PORT_LINK_STATUS_GET_MSG));

    /* Update reply parameters */
    *linkStatus = replyData.readData.portStatusGet.status;

   return (replyData.returnCode);
}
/**
* @internal mvHwsPortAutoTuneSetExtIpc function
* @endinternal
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portTuningMode           - port TX related tuning mode
* @param[in] optAlgoMask              - bit mask for optimization algorithms
* @param[in] portTuningMode           - port tuning mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAutoTuneSetExtIpc
(
    GT_U8                      devNum,
    GT_U32                     portGroup,
    GT_U32                     phyPortNum,
    MV_HWS_PORT_STANDARD       portMode,
    MV_HWS_PORT_AUTO_TUNE_MODE portTuningMode,
    GT_U32                     optAlgoMask,
    void                       *results
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_DATA_STRUCT portAutoTuneSetExt =
        {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode, portTuningMode, optAlgoMask};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    results = results; /* to avoid warning */

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portAutoTuneSetExt = portAutoTuneSetExt;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_DATA_STRUCT),
                                                MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortAutoTuneStopIpc function
* @endinternal
*
* @brief   Send IPC message to stop Tx and Rx training
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] stopRx                   - stop RX
* @param[in] stopTx                   - stop Tx
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAutoTuneStopIpc
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               phyPortNum,
    MV_HWS_PORT_STANDARD portMode,
    GT_BOOL              stopRx,
    GT_BOOL              stopTx
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_AUTO_TUNE_STOP_DATA_STRUCT portAutoTuneStop =
        {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode, stopRx, stopTx};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portAutoTuneStop = portAutoTuneStop;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_AUTO_TUNE_STOP_DATA_STRUCT),
                                                MV_HWS_IPC_PORT_AUTO_TUNE_STOP_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsSerdesManualRxConfigIpc function
* @endinternal
*
* @brief   Send IPC message to configure SERDES Rx parameters for all SERDES lanes.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*                                      phyPortNum - physical port number
*                                      portMode   - port standard metric
*                                      config params: sqlch,ffeRes,ffeCap,dfeEn,alig
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesManualRxConfigIpc
(
    GT_U8                                devNum,
    GT_U32                               portGroup,
    GT_U32                               serdesNum,
    MV_HWS_SERDES_TYPE                   serdesType,
    IN MV_HWS_SERDES_RX_CONFIG_DATA_UNT  *rxConfigPtr
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;

    MV_HWS_IPC_PORT_SERDES_MANUAL_RX_CONFIG_DATA_STRUCT serdesManualRxConfig =
    {(GT_U16)portGroup, serdesNum, serdesType,
         rxConfigPtr->rxComphyH.portTuningMode,
         rxConfigPtr->rxComphyH.sqlch,
         rxConfigPtr->rxComphyH.ffeRes,
         rxConfigPtr->rxComphyH.ffeCap,
         rxConfigPtr->rxComphyH.dfeEn,
         rxConfigPtr->rxComphyH.alig};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.serdesManualRxConfig = serdesManualRxConfig;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_SERDES_MANUAL_RX_CONFIG_DATA_STRUCT),
                                                MV_HWS_IPC_PORT_SERDES_MANUAL_RX_CONFIG_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsSerdesManualTxConfigIpc function
* @endinternal
*
* @brief   Send IPC message to configure SERDES Tx parameters for all SERDES lanes.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*                                      phyPortNum - physical port number
*                                      portMode   - port standard metric
*                                      config params: txAmp,txAmpAdj,emph0,emph1,txAmpShft
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSerdesManualTxConfigIpc
(
    GT_U8                             devNum,
    GT_U32                            portGroup,
    GT_U32                            serdesNum,
    MV_HWS_SERDES_TYPE                serdesType,
    MV_HWS_SERDES_TX_CONFIG_DATA_UNT  *manTxTuneStcPtr
)
{
#ifdef ASIC_SIMULATION
   devNum = devNum;
   portGroup = portGroup;
   serdesNum = serdesNum;
   serdesType = serdesType;
   manTxTuneStcPtr = manTxTuneStcPtr;
#else
   MV_HWS_IPC_PORT_SERDES_MANUAL_TX_CONFIG_DATA_STRUCT serdesManualTxConfig;
   MV_HWS_IPC_CTRL_MSG_STRUCT      requestMsg;
   MV_HWS_IPC_REPLY_MSG_STRUCT     replyData;

    if (manTxTuneStcPtr == NULL)
    {
        return GT_BAD_PTR;
    }
    hwsOsMemSetFuncPtr(&serdesManualTxConfig, 0, sizeof(MV_HWS_IPC_PORT_SERDES_MANUAL_TX_CONFIG_DATA_STRUCT));
    serdesManualTxConfig.portGroup = (GT_U16)portGroup;
    serdesManualTxConfig.serdesNum = serdesNum;
    serdesManualTxConfig.serdesType = serdesType;
    hwsOsMemCopyFuncPtr(&serdesManualTxConfig.manTxTuneStc, manTxTuneStcPtr, sizeof(MV_HWS_SERDES_TX_CONFIG_DATA_UNT));
    /* Construct the msg */
    requestMsg.msgData.serdesManualTxConfig = serdesManualTxConfig;

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                               sizeof(MV_HWS_IPC_PORT_SERDES_MANUAL_TX_CONFIG_DATA_STRUCT),
                                               MV_HWS_IPC_PORT_SERDES_MANUAL_TX_CONFIG_MSG));

    return (replyData.returnCode);

#endif
    return GT_OK;
}

/**
* @internal mvHwsPortPolaritySetIpc function
* @endinternal
*
* @brief   Send message to set the port polarity of the Serdes lanes (Tx/Rx).
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] txInvMask                - bitmap of 32 bit, each bit represent Serdes
* @param[in] rxInvMask                - bitmap of 32 bit, each bit represent Serdes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortPolaritySetIpc
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               phyPortNum,
    MV_HWS_PORT_STANDARD portMode,
    GT_32                txInvMask,
    GT_32                rxInvMask
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_POLARITY_SET_DATA_STRUCT portPolaritySet =
        {(GT_U16)portGroup,(GT_U16)phyPortNum,portMode,txInvMask,rxInvMask};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portPolaritySet = portPolaritySet;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_POLARITY_SET_DATA_STRUCT),
                                                MV_HWS_IPC_PORT_POLARITY_SET_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortFecConfigIpc function
* @endinternal
*
* @brief   Send message to configure FEC disable/enable on port.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portFecType              - GT_TRUE for FEC enable, GT_FALSE otherwise
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortFecConfigIpc
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               phyPortNum,
    MV_HWS_PORT_STANDARD portMode,
    MV_HWS_PORT_FEC_MODE portFecType
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_FEC_CONFIG_DATA_STRUCT portFecConfig = {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode, portFecType};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portFecConfig = portFecConfig;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_FEC_CONFIG_DATA_STRUCT),
                                                MV_HWS_IPC_PORT_FEC_CONFIG_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortFecConfigGetIpc function
* @endinternal
*
* @brief   Send message to get FEC status.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] fecMode                  - (pointer to) FEC mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortFecConfigGetIpc
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               phyPortNum,
    MV_HWS_PORT_STANDARD portMode,
    MV_HWS_PORT_FEC_MODE *fecMode
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_INFO_STRUCT portFecConfigGet = {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portFecConfigGet = portFecConfigGet;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_PORT_FEC_CONFIG_GET_MSG));

    /* Update reply parameters */
    *fecMode = replyData.readData.portStatusGet.status;

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortTxEnableIpc function
* @endinternal
*
* @brief   Turn of the port Tx according to selection.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] enable                   - enable/disable port Tx
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortTxEnableIpc
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               phyPortNum,
    MV_HWS_PORT_STANDARD portMode,
    GT_BOOL              enable
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT            requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT           replyData;
    MV_HWS_IPC_PORT_TX_ENABLE_DATA_STRUCT portTxEnableData = {(GT_U16)portGroup,(GT_U16)phyPortNum,portMode,enable};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portTxEnableData = portTxEnableData;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_TX_ENABLE_DATA_STRUCT),
                                                MV_HWS_IPC_PORT_TX_ENABLE_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortTxEnableGetIpc function
* @endinternal
*
* @brief   Retrieve the status of all port serdeses.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortTxEnableGetIpc
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               phyPortNum,
    MV_HWS_PORT_STANDARD portMode,
    GT_BOOL              serdesTxStatus[]
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT  requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_INFO_STRUCT portTxEnableGet = {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode};
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 i;

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portTxEnableGet = portTxEnableGet;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_PORT_TX_ENABLE_GET_MSG));

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }
    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    /* on each related serdes */
    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        serdesTxStatus[i] = replyData.readData.portSerdesTxEnableGet.status[i];
    }

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortSignalDetectGetIpc function
* @endinternal
*
* @brief   Retrieve the status of all port serdeses.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortSignalDetectGetIpc
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               phyPortNum,
    MV_HWS_PORT_STANDARD portMode,
    GT_BOOL              signalDetect[]
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT  requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_INFO_STRUCT portSignalDetectGet = {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode};
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 i;

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portSignalDetectGet = portSignalDetectGet;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_PORT_SIGNAL_DETECT_GET_MSG));

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }

    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    /* on each related serdes */
    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        signalDetect[i] = replyData.readData.portSerdesSignalDetectGet.status[i];
    }

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortCdrLockStatusGetIpc function
* @endinternal
*
* @brief   Send message to get the CDR lock status of all port serdeses.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortCdrLockStatusGetIpc
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               phyPortNum,
    MV_HWS_PORT_STANDARD portMode,
    GT_BOOL              cdrLockStatus[]
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT  requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_INFO_STRUCT portCdrLockStatus = {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode};
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    GT_U32 i;

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portCdrLockStatus = portCdrLockStatus;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_PORT_CDR_LOCK_STATUS_GET_MSG));

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }
    /* rebuild active lanes list according to current configuration (redundancy) */
    CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, portGroup, phyPortNum, portMode, curLanesList));

    /* on each related serdes */
    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        cdrLockStatus[i] = replyData.readData.portSerdesCdrLockStatusGet.status[i];
    }

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortLoopbackSetIpc function
* @endinternal
*
* @brief   Send message to activates the port loopback modes.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] lpPlace                  - unit for loopback configuration
*                                      lpType     - loopback type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortLoopbackSetIpc
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               phyPortNum,
    MV_HWS_PORT_STANDARD portMode,
    MV_HWS_UNIT          lpPlace,
    MV_HWS_PORT_LB_TYPE  lbType
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT               requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT              replyData;
    MV_HWS_IPC_PORT_LOOPBACK_SET_DATA_STRUCT portLoopbackSet =
        {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode, lpPlace, lbType};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portLoopbackSet = portLoopbackSet;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_LOOPBACK_SET_DATA_STRUCT),
                                                MV_HWS_IPC_PORT_LOOPBACK_SET_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortFlowControlStateSetIpc function
* @endinternal
*
* @brief   Activates the port loopback modes.
*         Can be run only after create port not under traffic.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] fcState                  - flow control state
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortFlowControlStateSetIpc
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          phyPortNum,
    MV_HWS_PORT_STANDARD            portMode,
    MV_HWS_PORT_FLOW_CONTROL_ENT    fcState
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT               requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT              replyData;
    MV_HWS_IPC_PORT_FLOW_CONTROL_SET_DATA_STRUCT portFcStateSet =
        {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode, fcState};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portFcStateSet = portFcStateSet;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_FLOW_CONTROL_SET_DATA_STRUCT),
                                                MV_HWS_IPC_PORT_FC_STATE_SET_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortLoopbackStatusGetIpc function
* @endinternal
*
* @brief   Send IPC message to retrive MAC loopback status.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] lpPlace                  - unit for loopback configuration
*
* @param[out] lbType                   - supported loopback type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortLoopbackStatusGetIpc
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_UNIT             lpPlace,
    MV_HWS_PORT_LB_TYPE     *lbType
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT               requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT              replyData;
    MV_HWS_IPC_PORT_LOOPBACK_GET_DATA_STRUCT portLoopbackGet =
        {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode, lpPlace};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portLoopbackGet = portLoopbackGet;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_LOOPBACK_GET_DATA_STRUCT),
                                                MV_HWS_IPC_PORT_LOOPBACK_STATUS_GET_MSG));

    *lbType = replyData.readData.portLoopbackStatusGet.lbType;

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortPPMSetIpc function
* @endinternal
*
* @brief   Send IPC message to increase/decrease Tx clock on port (added/sub ppm).
*         Can be run only after create port not under traffic.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] portPPM                  - limited to +/- 3 taps
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortPPMSetIpc
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               phyPortNum,
    MV_HWS_PORT_STANDARD portMode,
    MV_HWS_PPM_VALUE     portPPM
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT          requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT         replyData;
    MV_HWS_IPC_PORT_PPM_SET_DATA_STRUCT portPPMSet = {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode, portPPM};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portPPMSet = portPPMSet;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_PPM_SET_DATA_STRUCT),
                                                MV_HWS_IPC_PORT_PPM_SET_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortPPMGetIpc function
* @endinternal
*
* @brief   Send message to check the entire line configuration
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] portPPM                  - current PPM
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortPPMGetIpc
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               phyPortNum,
    MV_HWS_PORT_STANDARD portMode,
    MV_HWS_PPM_VALUE     *portPPM
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT  requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_INFO_STRUCT portPPMGet = {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portPPMGet = portPPMGet;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_PORT_PPM_GET_MSG));

    *portPPM = replyData.readData.portPpmGet.portPpm;

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortInterfaceGetIpc function
* @endinternal
*
* @brief   Send message to gets Interface mode and speed of a specified port.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - core number
* @param[in] phyPortNum               - physical port number (or CPU port)
*
* @param[out] portModePtr              - interface mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortInterfaceGetIpc
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               phyPortNum,
    MV_HWS_PORT_STANDARD *portModePtr
)
{
#if 0
    MV_HWS_IPC_CTRL_MSG_STRUCT         requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT        replyData;
    MV_HWS_IPC_PORT_IF_GET_DATA_STRUCT portInterfaceGet = {(GT_U16)portGroup, (GT_U16)phyPortNum};

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /* Construct the msg */
    requestMsg.msgData.portInterfaceGet = portInterfaceGet;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_IF_GET_DATA_STRUCT),
                                                MV_HWS_IPC_PORT_IF_GET_MSG));

    *portModePtr = replyData.readData.portIfGet.portIf;

    return (replyData.returnCode);
#else
    /* avoid warnings */
    devNum      = devNum;
    portGroup   = portGroup;
    phyPortNum  = phyPortNum;
    portModePtr = portModePtr;

    /* this functionality cannot be ran in internal cpu since it requires port
        elements info for all port modes which doesn't exist in internal CPU */
    return GT_NOT_SUPPORTED;
#endif
}

/**
* @internal mvHwsPortPcsActiveStatusGetIpc function
* @endinternal
*
* @brief   Return number of PCS active lanes or 0 if PCS unit is under RESET.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @param[out] numOfLanes               - number of lanes agregated in PCS
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortPcsActiveStatusGetIpc
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  *numOfLanes
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT                          requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT                         replyData;
    MV_HWS_IPC_PORT_PCS_ACTIVE_STATUS_GET_DATA_STRUCT   portPcsActiveStatus = {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* send port params (to internal CPU) */
    CHECK_STATUS(mvHwsIpcSendPortParams(devNum, portGroup, phyPortNum, portMode));

    /*construct the msg*/
    requestMsg.msgData.portPcsActiveStatus = portPcsActiveStatus;

    /* send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply (devNum,&requestMsg,&replyData,
                                                 sizeof(MV_HWS_IPC_PORT_PCS_ACTIVE_STATUS_GET_DATA_STRUCT),
                                                 MV_HWS_IPC_PORT_PCS_ACTIVE_STATUS_GET_CONFIG_MSG));

    *numOfLanes = replyData.readData.portReplyGet.results;

    return (replyData.returnCode);
}

GT_STATUS mvHwsNoopIpc
(
    GT_U8                           devNum,
    GT_U32                          data
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT               requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT              replyData;

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.noop.data = data;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_NOOP_DATA_STRUCT),
                                                MV_HWS_IPC_NOOP_MSG));

    hwsOsPrintf("IPC NOOP rc=%d data=%d\n",replyData.returnCode,replyData.readData.noopReply.data);
    return (replyData.returnCode);
}



/**
* @internal mvHwsPortApStartIpc function
* @endinternal
*
* @brief   Send message to configure AP port parameters and start execution
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - core number
* @param[in] phyPortNum               - physical port number
* @param[in] apCfg                    - AP configuration parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApStartIpc
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum,
    GT_U32 *apCfg
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT     requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT    replyData;
    MV_HWS_IPC_PORT_AP_DATA_STRUCT apCfgIpc;
    MV_HWS_AP_CFG                  *localApCfg = (MV_HWS_AP_CFG*)apCfg;
    GT_U16 phyPcsNum = mvHwsPortLaneMuxingGetMacFromSerdes(devNum,(GT_U8)portGroup,phyPortNum,(GT_U16)localApCfg->apLaneNum);
    if (mvHwsMtipIsReducedPort(devNum, phyPortNum))
    {
        localApCfg->apLaneNum = 16;
    }

    apCfgIpc.portGroup          = (GT_U16)portGroup;
    apCfgIpc.phyPortNum         = (GT_U16)phyPortNum;
    apCfgIpc.advMode            = localApCfg->modesVector;
    apCfgIpc.options            = (GT_U32)localApCfg->fcPause       << AP_CTRL_FC_PAUSE_SHIFT      |
                                  (GT_U32)localApCfg->fcAsmDir      << AP_CTRL_FC_ASM_SHIFT        |
                                  (GT_U32)localApCfg->fecSup        << AP_CTRL_FEC_ABIL_SHIFT      |
                                  (GT_U32)localApCfg->fecReq        << AP_CTRL_FEC_REQ_SHIFT       |
                                  (GT_U32)localApCfg->nonceDis      << AP_CTRL_LB_EN_SHIFT         |
                                  (GT_U32)localApCfg->ctleBiasValue << AP_CTRL_CTLE_BIAS_VAL_SHIFT |
                                  (GT_U32)(localApCfg->specialSpeeds&0x3) << AP_CTRL_20G_R1_SHIFT        |
                                  (GT_U32)localApCfg->skipRes       << AN_CTRL_SKIP_RES_SHIFT      |
                                  (GT_U32)((localApCfg->extraOperation & MV_HWS_PORT_SERDES_OPERATION_PRECODING_E) << AN_CTRL_PRECODING_SHIFT) |
                                    /* rx train is bit 5 in extraOperation and will set bit 19 in apCfgIpc.options */
                                  (GT_U32)(((localApCfg->extraOperation & MV_HWS_PORT_SERDES_OPERATION_RX_TRAIN_E)>> 5) << AN_CTRL_RX_TRAIN_SHIFT)|
                                    /* is_fiber is bit 6 in extraOperation and will set bit 20 in apCfgIpc.options */
                                  (GT_U32)(((localApCfg->extraOperation & MV_HWS_PORT_SERDES_OPERATION_IS_FIBER_E)>> 6) << AN_CTRL_IS_OPTICAL_MODE_SHIFT);

    apCfgIpc.laneNum            = (GT_U8)localApCfg->apLaneNum;
    apCfgIpc.pcsNum             = (GT_U8)phyPcsNum;                                     /* pcs num OF THE NEGOTIATION lane*/
    apCfgIpc.macNum             = (GT_U8)phyPortNum;
    apCfgIpc.polarityVector     = (GT_U16)localApCfg->polarityVector;
    apCfgIpc.refClockSrcParams  = (GT_U8)localApCfg->refClockCfg.isValid |              /* valid serdes number */
                                  (GT_U8)localApCfg->refClockCfg.cpllOutFreq << 1 |     /* cpll out frequency*/
                                  (GT_U8)localApCfg->refClockCfg.refClockSource << 4 |  /* reference clock source*/
                                  (GT_U8)localApCfg->refClockCfg.refClockFreq << 5;     /* reference clock freq*/


    /** [10:10]consortium (RS-FEC) ability (F1)
    ** [11:11]consortium (BASE-R FEC) ability (F2)
    ** [12:12]consortium (RS-FEC) request (F3)
    ** [13:13]consortium (BASE-R FEC) request (F4)*/
    AP_CTRL_RS_FEC_REQ_CONSORTIUM_SET(apCfgIpc.options,
                                       ((localApCfg->fecAdvanceReq >> FEC_ADVANCE_CONSORTIUM_SHIFT) & AP_ST_HCD_FEC_RES_RS)? 1:0);
    AP_CTRL_FC_FEC_REQ_CONSORTIUM_SET(apCfgIpc.options,
                                       ((localApCfg->fecAdvanceReq >> FEC_ADVANCE_CONSORTIUM_SHIFT) & AP_ST_HCD_FEC_RES_FC)? 1:0);
    AP_CTRL_RS_FEC_ABIL_CONSORTIUM_SET(apCfgIpc.options,
                                       ((localApCfg->fecAdvanceAbil >> FEC_ADVANCE_CONSORTIUM_SHIFT) & AP_ST_HCD_FEC_RES_RS)? 1:0);
    AP_CTRL_FC_FEC_ABIL_CONSORTIUM_SET(apCfgIpc.options,
                                       ((localApCfg->fecAdvanceAbil >> FEC_ADVANCE_CONSORTIUM_SHIFT) & AP_ST_HCD_FEC_RES_FC)? 1:0);

    if(hwsDeviceSpecInfo[devNum].devType != Bobcat3 && hwsDeviceSpecInfo[devNum].devType != Aldrin &&
       hwsDeviceSpecInfo[devNum].devType != Aldrin2 && hwsDeviceSpecInfo[devNum].devType != Pipe &&
       hwsDeviceSpecInfo[devNum].devType != Falcon)
    {
        apCfgIpc.pcsNum = (GT_U8)phyPortNum;
    }

    if(hwsDeviceSpecInfo[devNum].devType == Bobcat3 || hwsDeviceSpecInfo[devNum].devType == Aldrin2)
    {
        if((AP_CTRL_ADV_50G_CONSORTIUM_GET(apCfgIpc.advMode)) &&
        (AP_CTRL_FC_FEC_ABIL_CONSORTIUM_GET(apCfgIpc.options)) &&
        ((phyPortNum % 4) != 0))
        {
            /* only slot 0 in MSPG can work with 50G and FC-FEC mode */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    AP_CTRL_FEC_ADVANCED_REQ_SET(apCfgIpc.options, (localApCfg->fecAdvanceReq>>FEC_ADVANCE_BASE_R_SHIFT));

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.apConfig = apCfgIpc;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_AP_DATA_STRUCT),
                                                MV_HWS_IPC_PORT_AP_ENABLE_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortApStopIpc function
* @endinternal
*
* @brief   Send message to stop AP port execution
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - core number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApStopIpc
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT     requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT    replyData;
    MV_HWS_IPC_PORT_AP_DATA_STRUCT apPortCfg = {(GT_U16)portGroup, (GT_U16)phyPortNum, 0, 0, 0, 0, 0, 0, 0};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.apConfig = apPortCfg;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_AP_DATA_STRUCT),
                                                MV_HWS_IPC_PORT_AP_DISABLE_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortApSysAckIpc function
* @endinternal
*
* @brief   Send message to notify AP state machine that port resource allocation
*         was executed by the Host and it can continue execution
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - core number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApSysAckIpc
(
    GT_U8                  devNum,
    GT_U32                 portGroup,
    GT_U32                 phyPortNum,
    MV_HWS_PORT_STANDARD   portMode
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT  requestMsg;
    MV_HWS_IPC_PORT_INFO_STRUCT apPortSysCfg = {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode};

    /* Construct the msg */
    requestMsg.msgData.apSysConfig = apPortSysCfg;

    /* Send message to HWS and don't wait for the reply */
    CHECK_STATUS(mvHwsIpcSendMsgNoReply(devNum,&requestMsg,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_PORT_AP_SYS_CFG_VALID_MSG));

    return GT_OK;
}

/**
* @internal mvHwsPortApEnableWaitAckIpc function
* @endinternal
*
* @brief   Send message to notify AP state machine that port was
*         disabled and ap machine can continue execution
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - core number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApEnableWaitAckIpc
(
    GT_U8                  devNum,
    GT_U32                 portGroup,
    GT_U32                 phyPortNum
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT  requestMsg;
    MV_HWS_IPC_PORT_INFO_STRUCT apPortEnableWaitAck = {(GT_U16)portGroup, (GT_U16)phyPortNum, NON_SUP_MODE};

    /* Construct the msg */
    requestMsg.msgData.apEnableWaitAck = apPortEnableWaitAck;

    /* Send message to HWS and don't wait for the reply */
    CHECK_STATUS(mvHwsIpcSendMsgNoReply(devNum,&requestMsg,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_PORT_AP_ENABLE_WAIT_ACK_MSG));

    return GT_OK;
}

/**
* @internal mvHwsPortApCfgGetIpc function
* @endinternal
*
* @brief   Send message to retrive AP port configuration parameters
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - core number
* @param[in] phyPortNum               - physical port number
* @param[in] apCfg                    - AP configuration parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApCfgGetIpc
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum,
    GT_U32 *apCfg
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT       requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT      replyData;
    MV_HWS_IPC_PORT_REPLY_AP_CFG_GET *apCfgIpc;
    MV_HWS_AP_CFG                    *apCfgReply = (MV_HWS_AP_CFG*)apCfg;
    MV_HWS_IPC_PORT_AP_DATA_STRUCT   apPortCfg = {(GT_U16)portGroup, (GT_U16)phyPortNum, 0, 0, 0, 0, 0, 0, 0};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.apConfig = apPortCfg;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_AP_DATA_STRUCT),
                                                MV_HWS_IPC_PORT_AP_CFG_GET_MSG));

    apCfgIpc = &(replyData.readData.portApCfgGet);
    apCfgReply->fecAdvanceReq = 0;
    apCfgReply->fecAdvanceAbil = 0;
    /* Update reply parameters */
    apCfgReply->apLaneNum   = AP_CTRL_LANE_GET(apCfgIpc->ifNum);
    apCfgReply->modesVector = apCfgIpc->capability;
    apCfgReply->fcPause     = AP_CTRL_FC_PAUSE_GET(apCfgIpc->options);
    apCfgReply->fcAsmDir    = AP_CTRL_FC_ASM_GET(apCfgIpc->options);
    apCfgReply->fecSup      = AP_CTRL_FEC_ABIL_GET(apCfgIpc->options);
    apCfgReply->fecReq      = AP_CTRL_FEC_REQ_GET(apCfgIpc->options);
    apCfgReply->fecAdvanceReq |= AP_CTRL_FEC_ADVANCED_REQ_GET(apCfgIpc->options) << FEC_ADVANCE_BASE_R_SHIFT;

    /*apCfgReply->fecAdvanceAbil = */
    apCfgReply->fecAdvanceReq  |= (AP_CTRL_RS_FEC_REQ_CONSORTIUM_GET(apCfgIpc->options))? (AP_ST_HCD_FEC_RES_RS << FEC_ADVANCE_CONSORTIUM_SHIFT):0;
    apCfgReply->fecAdvanceReq  |= (AP_CTRL_FC_FEC_REQ_CONSORTIUM_GET(apCfgIpc->options))? (AP_ST_HCD_FEC_RES_FC << FEC_ADVANCE_CONSORTIUM_SHIFT):0;
    apCfgReply->fecAdvanceAbil |= (AP_CTRL_RS_FEC_ABIL_CONSORTIUM_GET(apCfgIpc->options))? (AP_ST_HCD_FEC_RES_RS << FEC_ADVANCE_CONSORTIUM_SHIFT):0;
    apCfgReply->fecAdvanceAbil |= (AP_CTRL_FC_FEC_ABIL_CONSORTIUM_GET(apCfgIpc->options))? (AP_ST_HCD_FEC_RES_FC << FEC_ADVANCE_CONSORTIUM_SHIFT):0;
    apCfgReply->nonceDis    = AP_CTRL_LB_EN_GET(apCfgIpc->options);
    apCfgReply->refClockCfg.refClockFreq   = MHz_156;
    apCfgReply->refClockCfg.refClockSource = PRIMARY_LINE_SRC;

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortApStatusGetIpc function
* @endinternal
*
* @brief   Send message to retrive AP port status parameters
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - core number
* @param[in] phyPortNum               - physical port number
* @param[in] apStatus                 - AP Status parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApStatusGetIpc
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum,
    GT_U32 *apStatus
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT          requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT         replyData;
    MV_HWS_IPC_PORT_REPLY_AP_STATUS_GET *apStatusIpc;
    MV_HWS_AP_PORT_STATUS               *apStatusReply = (MV_HWS_AP_PORT_STATUS*)apStatus;
    MV_HWS_IPC_PORT_INFO_STRUCT         apPortStatus = {(GT_U16)portGroup, (GT_U16)phyPortNum, NON_SUP_MODE};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.apStatusGet = apPortStatus;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_PORT_AP_STATUS_MSG));

    apStatusIpc = &(replyData.readData.portApStatusGet);

    /* Update reply parameters */
    apStatusReply->apLaneNum                = apStatusIpc->laneNum;
    apStatusReply->smState                  = apStatusIpc->state;
    apStatusReply->smStatus                 = apStatusIpc->status;
    apStatusReply->arbStatus                = apStatusIpc->ARMSmStatus;
    apStatusReply->hcdResult.hcdFound       = AP_ST_HCD_FOUND_GET(apStatusIpc->hcdStatus);
    apStatusReply->hcdResult.hcdLinkStatus  = AP_ST_HCD_LINK_GET(apStatusIpc->hcdStatus);
    apStatusReply->hcdResult.hcdResult      = AP_ST_HCD_TYPE_GET(apStatusIpc->hcdStatus);
    apStatusReply->hcdResult.hcdFecEn       = (AP_ST_HCD_FEC_RES_GET(apStatusIpc->hcdStatus)
                                               != AP_ST_HCD_FEC_RES_NONE) ? 1 : 0;
    apStatusReply->hcdResult.hcdFecType     = AP_ST_HCD_FEC_RES_GET(apStatusIpc->hcdStatus);
    apStatusReply->hcdResult.hcdFcRxPauseEn = AP_ST_HCD_FC_RX_RES_GET(apStatusIpc->hcdStatus);
    apStatusReply->hcdResult.hcdFcTxPauseEn = AP_ST_HCD_FC_TX_RES_GET(apStatusIpc->hcdStatus);
    apStatusReply->hcdResult.hcdMisc        = AP_ST_HCD_IS_OPTICAL_MODE_GET(apStatusIpc->hcdStatus);

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortApStatsGetIpc function
* @endinternal
*
* @brief   Send message to retrive AP port stats parameters
*
* @param[in] devNum                   - physical device number
* @param[in] portGroup                - core number
* @param[in] phyPortNum               - physical port number
* @param[in] apStats                  - AP Stats parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApStatsGetIpc
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum,
    GT_U32 *apStats
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT         requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT        replyData;
    MV_HWS_IPC_PORT_REPLY_AP_STATS_GET *apStatsIpc;
    MV_HWS_AP_PORT_STATS               *apStatsReply = (MV_HWS_AP_PORT_STATS*)apStats;
    MV_HWS_IPC_PORT_INFO_STRUCT        apPortStats = {(GT_U16)portGroup, (GT_U16)phyPortNum, NON_SUP_MODE};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.apStatsGet = apPortStats;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_PORT_AP_STATS_MSG));

    apStatsIpc = &(replyData.readData.portApStatsGet);

    /* Update reply parameters */
    apStatsReply->txDisCnt          = apStatsIpc->txDisCnt;
    apStatsReply->abilityCnt        = apStatsIpc->abilityCnt;
    apStatsReply->abilitySuccessCnt = apStatsIpc->abilitySuccessCnt;
    apStatsReply->linkFailCnt       = apStatsIpc->linkFailCnt;
    apStatsReply->linkSuccessCnt    = apStatsIpc->linkSuccessCnt;
    apStatsReply->hcdResoultionTime = apStatsIpc->hcdResoultionTime;
    apStatsReply->linkUpTime        = apStatsIpc->linkUpTime;

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortApStatsResetIpc function
* @endinternal
*
* @brief   Reset AP port statistics information
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApStatsResetIpc
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT  requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_INFO_STRUCT apPortStats = {(GT_U16)portGroup, (GT_U16)phyPortNum, NON_SUP_MODE};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.apStatsReset = apPortStats;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_PORT_AP_STATS_RESET_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortApIntropSetIpc function
* @endinternal
*
* @brief   Set AP port introp information
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*                                      apintrop    - AP introp parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApIntropSetIpc
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum,
    GT_U32 *apIntrop
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT  requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_AP_PORT_INTROP       *localApIntropCfg = (MV_HWS_AP_PORT_INTROP*)apIntrop;

    MV_HWS_IPC_PORT_AP_INTROP_STRUCT apIntropIpc =
    {
        (GT_U16)portGroup,
        (GT_U16)phyPortNum,
        NON_SUP_MODE,
        localApIntropCfg->attrBitMask,
        localApIntropCfg->txDisDuration,
        localApIntropCfg->abilityDuration,
        localApIntropCfg->abilityMaxInterval,
        localApIntropCfg->abilityFailMaxInterval,
        localApIntropCfg->apLinkDuration,
        localApIntropCfg->apLinkMaxInterval,
        localApIntropCfg->pdLinkDuration,
        localApIntropCfg->pdLinkMaxInterval,
        localApIntropCfg->anPam4LinkMaxInterval,
    };
    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.apIntropSet = apIntropIpc;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_AP_INTROP_STRUCT),
                                                MV_HWS_IPC_PORT_AP_INTROP_SET_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortApIntropGetIpc function
* @endinternal
*
* @brief   Return AP port introp information
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*                                      apintrop    - AP introp parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApIntropGetIpc
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum,
    GT_U32 *apIntrop
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT          requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT         replyData;
    MV_HWS_IPC_PORT_REPLY_AP_INTROP_GET *apIntropIpc;
    MV_HWS_AP_PORT_INTROP               *apIntropReply = (MV_HWS_AP_PORT_INTROP*)apIntrop;
    MV_HWS_IPC_PORT_INFO_STRUCT         apPortIntrop = {(GT_U16)portGroup, (GT_U16)phyPortNum, NON_SUP_MODE};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.apIntropGet = apPortIntrop;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_PORT_AP_INTROP_GET_MSG));

    apIntropIpc = &(replyData.readData.portApIntropGet);

    /* Update reply parameters */
    apIntropReply->txDisDuration          = apIntropIpc->txDisDuration;
    apIntropReply->abilityDuration        = apIntropIpc->abilityDuration;
    apIntropReply->abilityMaxInterval     = apIntropIpc->abilityMaxInterval;
    apIntropReply->abilityFailMaxInterval = apIntropIpc->abilityFailMaxInterval;
    apIntropReply->apLinkDuration         = apIntropIpc->apLinkDuration;
    apIntropReply->apLinkMaxInterval      = apIntropIpc->apLinkMaxInterval;
    apIntropReply->pdLinkDuration         = apIntropIpc->pdLinkDuration;
    apIntropReply->pdLinkMaxInterval      = apIntropIpc->pdLinkMaxInterval;
    apIntropReply->anPam4LinkMaxInterval  = apIntropIpc->anPam4LinkMaxInterval;
    return (replyData.returnCode);
}

/**
* @internal mvHwsPortApDebugGetIpc function
* @endinternal
*
* @brief   Return AP debug information
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] apDebug                  - AP debug parameters
*
* @param[out] apDebug                  - AP debug parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApDebugGetIpc
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum,
    GT_U32 *apDebug
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT   requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT  replyData;
    MV_HWS_IPC_LOG_GET           *apDebugIpc;
    MV_HWS_FW_LOG                *apDebugReply = (MV_HWS_FW_LOG*)apDebug;
    MV_HWS_IPC_PORT_INFO_STRUCT  apPortDebug = {(GT_U16)portGroup, (GT_U16)phyPortNum, NON_SUP_MODE};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.logGet = apPortDebug;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_PORT_AP_DEBUG_GET_MSG));

    apDebugIpc = &(replyData.readData.logGet);

    /* Update reply parameters */
    apDebugReply->fwBaseAddr       = apDebugIpc->fwBaseAddr;
    apDebugReply->fwLogBaseAddr    = apDebugIpc->logBaseAddr;
    apDebugReply->fwLogCountAddr   = apDebugIpc->logCountAddr;
    apDebugReply->fwLogPointerAddr = apDebugIpc->logPointerAddr;
    apDebugReply->fwLogResetAddr   = apDebugIpc->logResetAddr;

    return (replyData.returnCode);
}

/**
* @internal mvHwsLogGetIpc function
* @endinternal
*
* @brief   Return FW Hws log information
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
* @param[in] hwsLogPtr                - hws log parameters pointer
*
* @param[out] hwsLogPtr                - hws log parameters pointer
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsLogGetIpc
(
    GT_U8  devNum,
    GT_U32 phyPortNum,
    GT_U32 *hwsLogPtr
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT    requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT   replyData;
    MV_HWS_IPC_LOG_GET            *ipcReplay;
    MV_HWS_FW_LOG                 *apDebugReply = (MV_HWS_FW_LOG*)hwsLogPtr;
    MV_HWS_IPC_PORT_INFO_STRUCT   hwsPortDebug = {0, (GT_U16)phyPortNum, NON_SUP_MODE};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.logGet = hwsPortDebug;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_HWS_LOG_GET_MSG));

    ipcReplay = &(replyData.readData.logGet);

    /* Update reply parameters */
    apDebugReply->fwBaseAddr       = ipcReplay->fwBaseAddr;
    apDebugReply->fwLogBaseAddr    = ipcReplay->logBaseAddr;
    apDebugReply->fwLogCountAddr   = ipcReplay->logCountAddr;
    apDebugReply->fwLogPointerAddr = ipcReplay->logPointerAddr;
    apDebugReply->fwLogResetAddr   = ipcReplay->logResetAddr;

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortVosOVerrideParamsSetIpc function
* @endinternal
*
* @brief   Set the VOS Override params for the service CPU in the DB. This API allows
*         the overall VOS params to be sent in small buffers in order to keep
*         low memory consumption in the IPC.
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
* @param[in] vosOverrideParamsBufPtr  - (pointer to) VOS Override params to set
* @param[in] vosOverrideParamsBufIdx  - relative index of the params buffer in the
*                                      overall VOS Override params DB.
* @param[in] vosOverrideParamsBufLength - length of the buffer array.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortVosOVerrideParamsSetIpc
(
    GT_U8  devNum,
    GT_U32 phyPortNum,
    GT_U32 *vosOverrideParamsBufPtr,
    GT_U32  vosOverrideParamsBufIdx,
    GT_U32  vosOverrideParamsBufLength
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT          requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT         replyData;
    MV_HWS_IPC_VOS_OVERRIDE_PARAMS      vosOverrideParams;

    /* Null pointer check */
    if (vosOverrideParamsBufPtr == NULL)
    {
        hwsOsPrintf("mvHwsPortVosOVerrideParamsSetIpc: vosOverrideParamsBufPtr is null for idx %d \n",
            vosOverrideParamsBufIdx);
        return GT_BAD_PARAM;
    }

    /* initialize replyData, requestMsg and vosOverrideParams */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));
    hwsOsMemSetFuncPtr(&requestMsg, 0, sizeof(MV_HWS_IPC_CTRL_MSG_STRUCT));
    hwsOsMemSetFuncPtr(&vosOverrideParams, 0, sizeof(MV_HWS_IPC_VOS_OVERRIDE_PARAMS));

    /* Setting the data */
    vosOverrideParams.phyPortNum = (GT_U16)phyPortNum;
    vosOverrideParams.portMode = NON_SUP_MODE;
    vosOverrideParams.vosOverrideParamsBufferIdx = vosOverrideParamsBufIdx;
    vosOverrideParams.vosOverrideParamsBufferLength = vosOverrideParamsBufLength;
    hwsOsMemCopyFuncPtr(&(vosOverrideParams.vosOverrideParamsBufferPtr[0]),
                       vosOverrideParamsBufPtr, (vosOverrideParamsBufLength*(sizeof(GT_U32))));
    requestMsg.msgData.vosOverrideParams = vosOverrideParams;

    /* Sending the data */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_VOS_OVERRIDE_PARAMS),
                                                MV_HWS_IPC_VOS_OVERRIDE_PARAMS_DATA_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortVosOverrideModeSetIpc function
* @endinternal
*
* @brief   Set the VOS Override mode for the service CPU.
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
* @param[in] vosOverride              - Vos override mode, GT_TRUE - override enable, else override disabled.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortVosOverrideModeSetIpc
(
    GT_U8  devNum,
    GT_U32 phyPortNum,
    GT_BOOL vosOverride
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT          requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT         replyData;
    MV_HWS_IPC_VOS_OVERRIDE_MODE_STRUCT    vosOverrideModeSet;

    /* initialize replyData, requestMsg and vosOverrideParams */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));
    hwsOsMemSetFuncPtr(&requestMsg, 0, sizeof(MV_HWS_IPC_CTRL_MSG_STRUCT));
    hwsOsMemSetFuncPtr(&vosOverrideModeSet, 0, sizeof(MV_HWS_IPC_VOS_OVERRIDE_MODE_STRUCT));

    /* Setting the data */
    vosOverrideModeSet.phyPortNum = (GT_U16)phyPortNum;
    vosOverrideModeSet.portMode = NON_SUP_MODE;
    vosOverrideModeSet.vosOverrideMode = vosOverride;
    requestMsg.msgData.vosOverrideModeSet = vosOverrideModeSet;

    /* Sending the data */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_VOS_OVERRIDE_MODE_STRUCT),
                                                MV_HWS_IPC_VOS_OVERRIDE_MODE_SET_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortApAvagoGuiSetIpc function
* @endinternal
*
* @brief   Set AP state machine state when Avago GUI is enabled
*         Avago GUI access Avago Firmware as SBUS command level
*         Therefore it is required to stop the periodic behiviour of AP state
*         machine when Avago GUI is enabled
* @param[in] devNum                   - system device number
* @param[in] cpuId                    - the cpu id
* @param[in] state                    - Avago GUI state
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApAvagoGuiSetIpc
(
    GT_U8 devNum,
    GT_U8 cpuId,
    GT_U8 state
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT  requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.avagoGuiSet.state = (GT_U32)state;
    requestMsg.msgData.avagoGuiSet.cpuId = (GT_U32)cpuId;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_AVAGO_GUI_STRUCT),
                                                MV_HWS_IPC_PORT_AVAGO_GUI_SET_MSG));

    return (replyData.returnCode);
}

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsPortApEnablePortCtrlSetIpc function
* @endinternal
*
* @brief   Set the entity responsible to preform port enable after training Host or Service CPU (default value service CPU)
*
* @param[in] devNum                   - system device number
* @param[in] srvCpuEnable             - if TRUE Service CPU responsible to preform port enable after training, if FALSE Host
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApEnablePortCtrlSetIpc
(
    GT_U8        devNum,
    GT_BOOL      srvCpuEnable
)
{

    MV_HWS_IPC_CTRL_MSG_STRUCT  requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.portEnableCtrlSet.portEnableCtrl = (GT_U32)srvCpuEnable;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_ENABLE_CTRL_DATA_STRUCT),
                                                MV_HWS_IPC_PORT_ENABLE_CTRL_SET_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortApEnablePortCtrlGetIpc function
* @endinternal
*
* @brief   Get the entity responsible to preform port enable after training Host or Service CPU (default value service CPU)
*
* @param[in] devNum                   - system device number
*
* @param[out] srvCpuEnablePtr          - (ptr to) port enable parmeter - if TRUE Service CPU responsible to preform port
*                                      enable after training, if FALSE Host
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApEnablePortCtrlGetIpc
(
    GT_U8        devNum,
    GT_BOOL      *srvCpuEnablePtr
)
{

    MV_HWS_IPC_CTRL_MSG_STRUCT  requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_INFO_STRUCT portInfo = {0, 0, NON_SUP_MODE};
    MV_HWS_IPC_PORT_ENABLE_CTRL_DATA_STRUCT *portEnableCtrlIpc;

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

     /* Construct the msg */
    requestMsg.msgData.avagoAddrGet = portInfo;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_PORT_ENABLE_CTRL_GET_MSG));

    portEnableCtrlIpc = &(replyData.readData.portEnableCtrlGet);

    *srvCpuEnablePtr = (GT_BOOL)portEnableCtrlIpc->portEnableCtrl;

    return (replyData.returnCode);
}
#endif
/**
* @internal mvHwsPortApSerdesTxParametersOffsetSetIpc function
* @endinternal
*
* @brief   Set serdes TX parameters offsets (negative of positive). Those offsets
*         will take place after resolution and prior to running TRX training.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] serdesNumber             - serdes number
* @param[in] offsets                  - serdes TX values offsets
* @param[in] serdesSpeed              port speed assicoated with the offsets
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApSerdesTxParametersOffsetSetIpc
(
    GT_U8         devNum,
    GT_U32        portGroup,
    GT_U32        phyPortNum,
    GT_U32        serdesNumber,
    GT_U16        offsets,
    MV_HWS_SERDES_SPEED serdesSpeed
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_SERDES_TX_OFFSETS_STC portSerdesTxParametersOffseConfig =
            {(GT_U16)portGroup, (GT_U16)phyPortNum, (GT_U32)serdesSpeed, (GT_U8)serdesNumber, offsets};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.portSerdesTxParametersOffsetConfig = portSerdesTxParametersOffseConfig;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_SERDES_TX_OFFSETS_STC),
                                                MV_HWS_IPC_PORT_SERDES_RX_PARAMETERS_OFFSET_CONFIG_MSG));

#ifndef ASIC_SIMULATION
    /* those parameters will abe host to read tx-offsets database directly from FW memory */
    txParamesOffsetsFwBaseAddr = replyData.readData.portSerdesTxParametersOffsetInfoCfg.txOffsetBaseAddr;
    txParamesOffsetsFwSize = replyData.readData.portSerdesTxParametersOffsetInfoCfg.txOffsetSize;
#endif

    return (replyData.returnCode);
}


/**
* @internal mvHwsPortApSerdesRxParametersManualSetIpc function
* @endinternal
*
* @brief   Set serdes RX parameters . Those offsets
*         will take place after TRX training.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] apPortNum                - physical port number
* @param[in] rxCfgPtr                 - serdes RX values per speeds 10G and 25G
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApSerdesRxParametersManualSetIpc
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          apPortNum,
    MV_HWS_MAN_TUNE_CTLE_CONFIG_OVERRIDE *rxCfgPtr
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_SERDES_RX_CONFIG_OVERRIDE_STC rxCfgOverride;

    if (rxCfgPtr == NULL)
    {
        return GT_FAIL;
    }
    rxCfgOverride.portGroup = (GT_U16)portGroup;
    rxCfgOverride.phyPortNum = (GT_U16)apPortNum;
    rxCfgOverride.rxCfg.ctleParams.bandWidth     = rxCfgPtr->ctleParams.bandWidth;
    rxCfgOverride.rxCfg.ctleParams.dcGain        = rxCfgPtr->ctleParams.dcGain;
    rxCfgOverride.rxCfg.ctleParams.highFrequency = rxCfgPtr->ctleParams.highFrequency;
    rxCfgOverride.rxCfg.ctleParams.lowFrequency  = rxCfgPtr->ctleParams.lowFrequency;
    rxCfgOverride.rxCfg.ctleParams.squelch       = rxCfgPtr->ctleParams.squelch;
    rxCfgOverride.rxCfg.serdesSpeed              = rxCfgPtr->serdesSpeed;
    rxCfgOverride.rxCfg.etlParams.etlMinDelay    = rxCfgPtr->etlParams.etlMinDelay;
    rxCfgOverride.rxCfg.etlParams.etlMaxDelay    = rxCfgPtr->etlParams.etlMaxDelay;
    rxCfgOverride.rxCfg.etlParams.etlEnableOverride = rxCfgPtr->etlParams.etlEnableOverride;
    rxCfgOverride.rxCfg.fieldOverrideBmp         = rxCfgPtr->fieldOverrideBmp;

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.portSerdesRxConfigOverride = rxCfgOverride;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_SERDES_RX_CONFIG_OVERRIDE_STC),
                                                MV_HWS_IPC_PORT_SERDES_RX_CONFIG_OVERRIDE_MSG));

    return (replyData.returnCode);
}


/**
* @internal mvHwsPortAdaptiveCtlePortEnableSetIpc function
* @endinternal
*
* @brief   set adaptive ctle p[ort enable/ disable
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] apPortNum                - physical port number
* @param[in] enable                   - enable or disable
*       adaptive ctle
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAdaptiveCtlePortEnableSetIpc
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U32                          apPortNum,
    GT_BOOL                         enable
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_ADAPTIVE_CTLE_ENABLE_STC adptCtlePortEnable = {(GT_U16)portGroup, (GT_U16)apPortNum, (GT_U8)enable};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.adaptCtlePortEnable = adptCtlePortEnable;
    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_ADAPTIVE_CTLE_ENABLE_STC),
                                                MV_HWS_IPC_PORT_ADAPTIVE_CTLE_ENABLE_MSG));
    return (replyData.returnCode);
}


/**
* @internal mvHwsPortIsLinkUpStatusSetIpc function
* @endinternal
*
* @brief   set port link up or link down status for non AP ports
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                  - physical port number
* @param[in] isLinkUp                 - port in link up or link
*                                       down
* @param[in] trainLfArr              - train LF from training
*                                       or enh training result.
* @param[in] enhTrainDelayArr        - enhtrain Delay from
*                                      enh training result.
* @param[in] serdesList              - port's serdeses arrray
* @param[in] numOfSerdes             - serdeses number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortIsLinkUpStatusSetIpc
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portNum,
    GT_BOOL                 isLinkUp,
    GT_U16                  *trainLfArr,
    GT_U16                  *enhTrainDelayArr,
    GT_U16                  *serdesList,
    GT_U8                   numOfSerdeses
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_NON_AP_PORT_IS_LINK_UP_STC  nonApPortIsLinkUp;
    GT_U8 ii;


    if (trainLfArr == NULL) {
        return GT_FAIL;
    }

    if (enhTrainDelayArr == NULL) {
        return GT_FAIL;
    }

    if (serdesList == NULL) {
        return GT_FAIL;
    }


    nonApPortIsLinkUp.portGroup          = (GT_U16)portGroup;
    nonApPortIsLinkUp.phyPortNum         = (GT_U16)portNum;
    nonApPortIsLinkUp.isLinkUp           = (GT_U8)isLinkUp;

    for (ii = 0 ; ii < numOfSerdeses; ii ++)
    {
        nonApPortIsLinkUp.trainLf[ii]         = (GT_U8)trainLfArr[ii];
        nonApPortIsLinkUp.enhTrainDelay[ii]   = (GT_U8)enhTrainDelayArr[ii];
        nonApPortIsLinkUp.serdesList[ii]      = (GT_U8)serdesList[ii];
    }

    nonApPortIsLinkUp.numOfSerdeses      = numOfSerdeses;

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.nonApPortIsLinkUp = nonApPortIsLinkUp;
    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_NON_AP_PORT_IS_LINK_UP_STC),
                                                MV_HWS_IPC_NON_AP_PORT_IS_LINK_UP_MSG));
    return (replyData.returnCode);
}

#if defined (PX_FAMILY)
/**
* @internal mvHwsPortLkbPortSetIpc function
* @endinternal
*
* @brief   send ipc message to hws - to enable link-binding on port
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                  - physical port number
* @param[in] portMode                 - HWS port mode
* @param[in] pairNum                  - physical pair port number.
*                                       when the link on portNum is down -
*                                       the remote fault enabled on the pair port.
* @param[in] add                      - 1-enable link binding on port, 0-disable
*
* @retval GT_OK                       - on success
* @retval GT_FAIL                     - on error
*/
GT_STATUS mvHwsPortLkbPortSetIpc
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U16                          portNum,
    MV_HWS_PORT_STANDARD            portMode,
    GT_U8                           pairNum,
    GT_U8                           add
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    portGroup = portGroup; /* Prevents 'unused param' compiler error */

    /* Construct the msg */
    requestMsg.msgData.portLinkBinding.phyPortNum = portNum;
    requestMsg.msgData.portLinkBinding.portMode   = portMode;
    requestMsg.msgData.portLinkBinding.phyPairNum = pairNum;
    requestMsg.msgData.portLinkBinding.enable = add;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum, &requestMsg, &replyData,
                                                sizeof(MV_HWS_IPC_LINK_BINDING_STRUCT),
                                                MV_HWS_IPC_PORT_LINK_BINDING_SET_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortLkbRegisterApPortIpc function
* @endinternal
*
* @brief   send ipc message to hws - to enable link-binding on AP port
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                  - physical port number
* @param[in] pairNum                  - physical pair port number.
*                                       when the link on portNum is down -
*                                       the remote fault enabled on the pair port.
*
* @retval GT_OK                       - on success
* @retval GT_FAIL                     - on error
*/
GT_STATUS mvHwsPortLkbRegisterApPortIpc
(
    GT_U8                           devNum,
    GT_U32                          portGroup,
    GT_U8                           apPortNum,
    GT_U8                           pairNum
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    portGroup = portGroup; /* Prevents 'unused param' compiler error */

    /* Construct the msg */
    requestMsg.msgData.portLinkBinding.phyPortNum = apPortNum;
    requestMsg.msgData.portLinkBinding.phyPairNum = pairNum;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum, &requestMsg, &replyData,
                                                sizeof(MV_HWS_IPC_LINK_BINDING_STRUCT),
                                                MV_HWS_IPC_PORT_LINK_BINDING_REGISTER_AP_MSG));

    return (replyData.returnCode);
}
#endif

#ifdef CHX_FAMILY
    extern GT_U32 hwsRavenDevBmp;
#endif

#endif /* MICRO_INIT */

/**
* @internal mvHwsPortAvagoCfgAddrGetIpc function
* @endinternal
*
* @brief   Return Avago Serdes Configuration structure address
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*
* @param[out] avagoCfgAddr             - Avago Serdes Configuration structure address
* @param[out] avagoCfgSize             - Avago Serdes Configuration structure size
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAvagoCfgAddrGetIpc
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroup,
    IN  GT_U32 phyPortNum,
    OUT GT_U32 *avagoCfgAddr,
    OUT GT_U32 *avagoCfgSize
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT   requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT  replyData;
    MV_HWS_IPC_PORT_REPLY_AVAGO_AAPL_GET *avagoAddrIpc;
    MV_HWS_IPC_PORT_INFO_STRUCT  avagoAddr = {(GT_U16)portGroup, (GT_U16)phyPortNum, NON_SUP_MODE};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    hwsOsMemSetFuncPtr(&requestMsg, 0, sizeof(MV_HWS_IPC_CTRL_MSG_STRUCT));
    requestMsg.msgData.avagoAddrGet = avagoAddr;

#if defined (FALCON_DEV_SUPPORT) && defined (MICRO_INIT)
    devNum = (GT_U8)phyPortNum/16;/* Use devNum to pass Raven number */
#endif
    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_PORT_AVAGO_SERDES_INIT_MSG));

    avagoAddrIpc = &(replyData.readData.portAvagoAaplGet);

    /* Update reply parameters */
    *avagoCfgAddr = (GT_U32)(avagoAddrIpc->addr);
    *avagoCfgSize = avagoAddrIpc->size;

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortAvagoAaplInitIpc function
* @endinternal
*
* @brief   Init Avago Serdes Configuration structure pointers parameters, on service CPU, with NULL values.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortAvagoAaplInitIpc
(
    GT_U8  devNum,
    GT_U32 portGroup,
    GT_U32 phyPortNum
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT  requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_PORT_INFO_STRUCT avagoAaplInitIpc = {(GT_U16)portGroup, (GT_U16)phyPortNum, NON_SUP_MODE};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.avagoAaplInit = avagoAaplInitIpc;

#if defined (FALCON_DEV_SUPPORT) && defined (MICRO_INIT)
    devNum = (GT_U8)phyPortNum/16;/* Use devNum to pass Raven number */
#endif
    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_INFO_STRUCT),
                                                MV_HWS_IPC_PORT_AVAGO_SERDES_RESET_DATA_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortOperationIpc function
* @endinternal
*
* @brief   send ipc message to hws - to enable link-binding on AP port
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                  - physical port number
* @param[in] portOperation            - port operation
* @param[in] portData                 - port data
* @param[in] portData2                - additinal data
*
* @retval GT_OK                       - on success
* @retval GT_FAIL                     - on error
*
* portOperation = 1  MV_HWS_IPC_PORT_OPERATION_ENVELOPE_SET
* portOperation = 2  MV_HWS_IPC_PORT_OPERATION_AN_DISABLE
* portOperation = 3  MV_HWS_IPC_PORT_OPERATION_DUMP
* portOperation = 4  MV_HWS_IPC_PORT_OPERATION_LINK_UP_DFE_MODE
* portOperation = 5  MV_HWS_IPC_PORT_OPERATION_CFG_HAL_SET
* portOperation = 6  MV_HWS_IPC_PORT_OPERATION_INTERCONNECT_PROFILE_SET
* portOperation = 7  MV_HWS_IPC_PORT_OPERATION_GLOBAL_RX_TERMINATION_SET
* portOperation = 8  MV_HWS_IPC_PORT_OPERATION_TX_PRESET_SET
* portOperation = 9  MV_HWS_IPC_PORT_OPERATION_HOST_OR_MI_SET
*/
GT_STATUS mvHwsPortOperationIpc
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroup,
    IN  GT_U32 phyPortNum,
    IN  GT_U32 portOper,
    IN  GT_U32 *portDataPtr
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT        requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT       replyData;
    MV_HWS_IPC_PORT_OPERATION_STRUCT  portOperation;

    hwsOsMemSetFuncPtr(&portOperation, 0, sizeof(MV_HWS_IPC_PORT_OPERATION_STRUCT));
#if defined (FALCON_DEV_SUPPORT) && defined (MICRO_INIT)
    portOperation.phyPortNum = (GT_U16)phyPortNum%16;
#else
    portOperation.phyPortNum = (GT_U16)phyPortNum;
#endif
    portOperation.portGroup = (GT_U16)portGroup;
    portOperation.portOperation = portOper;
    if (portDataPtr != NULL)
    {
        hwsOsMemCopyFuncPtr(portOperation.portData, portDataPtr, sizeof(GT_U32) * MV_HWS_IPC_PORT_OPERATION_DATA_LEN_CNS);
    }


    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

#ifndef MICRO_INIT
#ifdef CHX_FAMILY
        /* Falcon 2T/4T serdes numbering alignment */
    if(hwsRavenDevBmp == 0xB5 || hwsRavenDevBmp == 0xA5)
    {
        if(MV_HWS_IPC_PORT_OPERATION_TX_PRESET_SET == portOper)
        {
            switch((phyPortNum / 16))
            {
                 case 1:
                    portOperation.phyPortNum += 16;
                    break;
                case 2:
                    portOperation.phyPortNum += 32;
                    break;
                case 3:
                    portOperation.phyPortNum += 32;
                    break;
                case 4:
                    portOperation.phyPortNum += 48;
                    break;
                case 0:
                default:
                    break;
            }
        }
    }
#endif
#endif

    /* Construct the msg */
    requestMsg.msgData.portOperation = portOperation;

#if defined (FALCON_DEV_SUPPORT) && defined (MICRO_INIT)
    devNum = (GT_U8)phyPortNum/16;/* Use devNum to pass Raven number */
#endif
    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_OPERATION_STRUCT),
                                                MV_HWS_IPC_PORT_OPERATION_MSG));

    /*hwsOsPrintf("IPC PORT OPERATION rc=%d portOperation=%d portData=0x%x portData2=0x%x\n", replyData.returnCode, portOperation.portOperation, portData, portData2);
    hwsOsPrintf("OPERATION  = 2 set cold envelope\n");
    hwsOsPrintf("OPERATION  = 3 set hot envelope\n");*/
    return (replyData.returnCode);
}

/**
* @internal mvHwsPortApSerdesTxConfigSetIpc function
* @endinternal
*
* @brief   Set serdes TX parameters .
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] serdesSpeed              port speed assicoated with the offsets
* @param[in] serdesNumber             - serdes number
* @param[in] txConfigPtr              - serdes TX config params
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApSerdesTxConfigSetIpc
(
    GT_U8                                     devNum,
    GT_U32                                    portGroup,
    GT_U32                                    phyPortNum,
    MV_HWS_SERDES_SPEED                       serdesSpeed,
    GT_U32                                    serdesNumber,
    MV_HWS_SERDES_TYPE                        serdesType,
    MV_HWS_SERDES_TX_OVERRIDE_CONFIG_DATA_UNT *txConfigPtr
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT           requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT          replyData;
    MV_HWS_IPC_PORT_SERDES_TX_CONFIG_STC portSerdesTxConfig;
    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));
    if (txConfigPtr == NULL)
    {
        return GT_FAIL;
    }
    hwsOsMemSetFuncPtr(&portSerdesTxConfig, 0, sizeof(MV_HWS_IPC_PORT_SERDES_TX_CONFIG_STC));
    portSerdesTxConfig.portGroup = (GT_U16)portGroup;
    portSerdesTxConfig.serdesSpeed = (GT_U16)serdesSpeed;
#if defined (FALCON_DEV_SUPPORT) && defined (MICRO_INIT)
    portSerdesTxConfig.phyPortNum = (GT_U16)phyPortNum%16;
    portSerdesTxConfig.serdesLane = (GT_U16)serdesNumber%16;
#else
    portSerdesTxConfig.phyPortNum = (GT_U16)phyPortNum;
    portSerdesTxConfig.serdesLane = (GT_U16)serdesNumber;
#endif
    portSerdesTxConfig.serdesType = (GT_U32)serdesType;
    hwsOsMemCopyFuncPtr(&portSerdesTxConfig.txCfg, txConfigPtr, sizeof(MV_HWS_SERDES_TX_CONFIG_DATA_UNT));

    /* Construct the msg */
    requestMsg.msgData.serdesTxConfig = portSerdesTxConfig;

#if defined (FALCON_DEV_SUPPORT) && defined (MICRO_INIT)
    devNum = (GT_U8)phyPortNum/16;/* Use devNum to pass Raven number */
#endif
    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_SERDES_TX_CONFIG_STC),
                                                MV_HWS_IPC_PORT_SERDES_TX_CONFIG_MSG));

    return (replyData.returnCode);
}

/**
* @internal mvHwsPortApSerdesRxConfigSetIpc function
* @endinternal
*
* @brief   Set serdes RX parameters
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] serdesSpeed              port speed assicoated with the offsets
* @param[in] serdesNumber             - serdes number
* @param[in] rxConfigPtr              - serdes TX config params
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApSerdesRxConfigSetIpc
(
    GT_U8                                     devNum,
    GT_U32                                    portGroup,
    GT_U32                                    phyPortNum,
    MV_HWS_SERDES_SPEED                       serdesSpeed,
    GT_U32                                    serdesNumber,
    MV_HWS_SERDES_TYPE                        serdesType,
    MV_HWS_SERDES_RX_OVERRIDE_CONFIG_DATA_UNT *rxConfigPtr
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT           requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT          replyData;
    MV_HWS_IPC_PORT_SERDES_RX_CONFIG_STC portSerdesRxConfig;
    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));
    if (rxConfigPtr == NULL)
    {
        return GT_FAIL;
    }
    hwsOsMemSetFuncPtr(&portSerdesRxConfig, 0, sizeof(MV_HWS_IPC_PORT_SERDES_RX_CONFIG_STC));
    portSerdesRxConfig.portGroup = (GT_U16)portGroup;
    portSerdesRxConfig.serdesSpeed = (GT_U16)serdesSpeed;
#if defined (FALCON_DEV_SUPPORT) && defined (MICRO_INIT)
    portSerdesRxConfig.phyPortNum = (GT_U16)phyPortNum%16;
    portSerdesRxConfig.serdesLane = (GT_U16)serdesNumber%16;
#else
    portSerdesRxConfig.phyPortNum = (GT_U16)phyPortNum;
    portSerdesRxConfig.serdesLane = (GT_U16)serdesNumber;
#endif
    portSerdesRxConfig.serdesType = (GT_U32)serdesType;
    hwsOsMemCopyFuncPtr(&portSerdesRxConfig.rxCfg, rxConfigPtr, sizeof(MV_HWS_SERDES_RX_CONFIG_DATA_UNT));

    /*portSerdesRxConfig.rxConfigStc.dcGain           =  (GT_U8)rxConfigPtr->dcGain;
    portSerdesRxConfig.rxConfigStc.lowFrequency     =  (GT_U8)rxConfigPtr->lowFrequency;
    portSerdesRxConfig.rxConfigStc.highFrequency    =  (GT_U8)rxConfigPtr->highFrequency;
    portSerdesRxConfig.rxConfigStc.bandWidth        =  (GT_U8)rxConfigPtr->bandWidth;
    portSerdesRxConfig.rxConfigStc.gainShape1       =  (GT_U8)rxConfigPtr->gainShape1;
    portSerdesRxConfig.rxConfigStc.gainShape2       =  (GT_U8)rxConfigPtr->gainShape2;
    portSerdesRxConfig.rxConfigStc.minLf            =  (GT_U8)rxConfigPtr->minLf;
    portSerdesRxConfig.rxConfigStc.maxLf            =  (GT_U8)rxConfigPtr->maxLf;
    portSerdesRxConfig.rxConfigStc.minHf            =  (GT_U8)rxConfigPtr->minHf;
    portSerdesRxConfig.rxConfigStc.maxHf            =  (GT_U8)rxConfigPtr->maxHf;
    portSerdesRxConfig.rxConfigStc.bfLf             =  (GT_8 )rxConfigPtr->bfLf;
    portSerdesRxConfig.rxConfigStc.bfHf             =  (GT_8 )rxConfigPtr->bfHf;
    portSerdesRxConfig.rxConfigStc.minPre1          =  (GT_8 )rxConfigPtr->minPre1;
    portSerdesRxConfig.rxConfigStc.maxPre1          =  (GT_8 )rxConfigPtr->maxPre1;
    portSerdesRxConfig.rxConfigStc.minPre2          =  (GT_8 )rxConfigPtr->minPre2;
    portSerdesRxConfig.rxConfigStc.maxPre2          =  (GT_8 )rxConfigPtr->maxPre2;
    portSerdesRxConfig.rxConfigStc.squelch          =  (GT_U8)rxConfigPtr->squelch;
    portSerdesRxConfig.rxConfigStc.shortChannelEn   =  (GT_U8)rxConfigPtr->shortChannelEn;*/

    /* Construct the msg */
    requestMsg.msgData.serdesRxConfig = portSerdesRxConfig;

#if defined (FALCON_DEV_SUPPORT) && defined (MICRO_INIT)
    devNum = (GT_U8)phyPortNum/16;/* Use devNum to pass Raven number */
#endif
    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_PORT_SERDES_RX_CONFIG_STC),
                                                MV_HWS_IPC_PORT_SERDES_RX_CONFIG_MSG));

    return (replyData.returnCode);
}

#ifndef MICRO_INIT

/**
* @internal mvHwsPortApTxPresetSetIpc function
* @endinternal
*
* @brief   Set preset parameters
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNumber             - serdes number
* @param[in] serdesSpeed              - serdes speed
* @param[in] pre                      - preset pre value
* @param[in] atten                    - preset atten value
* @param[in] post                     - preset post value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApTxPresetSetIpc
(
    IN  GT_U8                                 devNum,
    IN  GT_U32                                portGroup,
    GT_U32                                    serdesNumber,
    IN  MV_HWS_SERDES_SPEED                   serdesSpeed,
    IN  GT_U8                                 pre,
    IN  GT_U8                                 atten,
    IN  GT_U8                                 post
)
{
    GT_U32 portData[MV_HWS_IPC_PORT_OPERATION_DATA_LEN_CNS];
    GT_STATUS rc;
    if (HWS_DEV_SERDES_TYPE(devNum, serdesNumber) == AVAGO_16NM)
    {
        portData[0]= serdesSpeed;
        portData[1]= pre;
        portData[2]= atten;
        portData[3]= post;
        /* instead of creating a new ipc, we will use OperationIpc */
        rc = mvHwsPortOperationIpc(devNum, portGroup,
                                   serdesNumber,
                                   MV_HWS_IPC_PORT_OPERATION_TX_PRESET_SET,
                                   portData);
        if(GT_OK != rc)
        {
            return rc;
        }
    }
    else
        return GT_NOT_SUPPORTED;
    return GT_OK;
}
/**
* @internal mvHwsPortApConfigStateHalSetIpc function
* @endinternal
*
* @brief   Set serdes RX parameters
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] ravenNum               - physical port number

* @param[in] disable                  - chip disable an
*
* @retval 0                        - on success
* @retval 1                        - on error
*/

GT_STATUS mvHwsPortApConfigStateHalSetIpc
(
    GT_U8                                 devNum,
    GT_U32                                portGroup,
    GT_U32                                chipIndex,
    GT_U32                                halType,
    GT_U32                                halRegSelect,
    GT_32                                 halRegisterValue
)
{
    GT_U32 portData[MV_HWS_IPC_PORT_OPERATION_DATA_LEN_CNS];
    portData[0] = (GT_U32)halType;
    portData[1] = (GT_U32)halRegSelect;
    portData[2] = (GT_U32)halRegisterValue;


    /* instead of creating a new ipc, we will use OperationIpc*/
    return mvHwsPortOperationIpc(devNum, portGroup, chipIndex*16,
                                 MV_HWS_IPC_PORT_OPERATION_CFG_HAL_SET,
                                 portData);
}

#endif /* MICRO_INIT */

/**
* @internal mvHwsPortApConfigInterconnectProfileIpc function
* @endinternal
*
* @brief   Set serdes Interconnect profile
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] numOfActLanes            - num of active lanes
* @param[in] interconnectProfile       - interconnect profile
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApConfigInterconnectProfileIpc
(
    GT_U8                                 devNum,
    GT_U32                                portGroup,
    GT_U32                                phyPortNum,
    GT_U8                                 numOfActLanes,
    GT_U32                                interconnectProfile
)
{
    GT_U32 portData[MV_HWS_IPC_PORT_OPERATION_DATA_LEN_CNS];
    hwsOsMemSetFuncPtr(&portData, 0, MV_HWS_IPC_PORT_OPERATION_DATA_LEN_CNS * sizeof(GT_U32));

    portData[0] = (GT_U32)interconnectProfile;
    portData[1] = (GT_U32)numOfActLanes;
    /* instead of creating a new ipc, we will use OperationIpc*/
    return mvHwsPortOperationIpc(devNum, portGroup, phyPortNum,
                                 MV_HWS_IPC_PORT_OPERATION_INTERCONNECT_PROFILE_SET,
                                 portData);
}

#ifndef MICRO_INIT

/**
* @internal mvHwsPortApSerdesAnDisableSetIpc function
* @endinternal
*
* @brief   Disable AN task on CM3
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum                - physical port number
* @param[in] disable                  - chip disable an
* @param[in] allPortsInChip           - disable all port in chip
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApSerdesAnDisableSetIpc
(
    IN  GT_U8                                 devNum,
    IN  GT_U32                                portGroup,
    IN  GT_U32                                phyPortNum,
    IN  GT_BOOL                               disable,
    IN  GT_BOOL                               allPortsInChip
)
{
    GT_U32 portData[MV_HWS_IPC_PORT_OPERATION_DATA_LEN_CNS];

    portData[0]= disable;
    portData[1]= allPortsInChip;

    /* instead of creating a new ipc, we will use OperationIpc*/
    return mvHwsPortOperationIpc(devNum, portGroup, phyPortNum,
                                 MV_HWS_IPC_PORT_OPERATION_AN_DISABLE,
                                 portData);
}
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
/**
* @internal mvHwsPortApSerdesLinkUpDfeModeIpc function
* @endinternal
*
* @brief   Set the DFE mode to use after port reach link-up
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] dfeMode                  - DFE mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApSerdesLinkUpDfeModeIpc
(
    IN  GT_U8                                 devNum,
    IN  GT_U32                                portGroup,
    IN  MV_HWS_DFE_MODE                       dfeMode
)
{
    GT_U32 portData[MV_HWS_IPC_PORT_OPERATION_DATA_LEN_CNS];
    GT_U32 portNum;
    portData[0]= (GT_U32)dfeMode;

    for(portNum = 0; portNum < hwsDeviceSpecInfo[devNum].portsNum ; portNum += 16)
    {
#ifdef CHX_FAMILY
        if(hwsRavenDevBmp & (1<<(portNum/16) ))
#endif
        {
            if(!mvHwsMtipIsReducedPort(devNum,portNum))
            {
                /* instead of creating a new ipc, we will use OperationIpc*/
                if(GT_OK != (mvHwsPortOperationIpc(devNum, portGroup, portNum, MV_HWS_IPC_PORT_OPERATION_LINK_UP_DFE_MODE, portData)) )
                {
                    return GT_FAIL;
                }
            }
        }
    }
    return GT_OK;
}

/**
* @internal mvHwsPortApSerdesGlobalRxTerminationModeIpc function
* @endinternal
*
* @brief   Set the GlobalRxTerminationMode mode to use in port
*          power-up
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] rxTermination            - rxTermination mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsPortApSerdesGlobalRxTerminationModeIpc
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroup,
    IN  GT_U32               rxTermination
)
{
    GT_U32 portData[MV_HWS_IPC_PORT_OPERATION_DATA_LEN_CNS];
    GT_U32 portNum;

    hwsOsMemSetFuncPtr(&portData, 0, MV_HWS_IPC_PORT_OPERATION_DATA_LEN_CNS * sizeof(GT_U32));
    portData[0]= rxTermination;
    for(portNum = 0; portNum < hwsDeviceSpecInfo[devNum].portsNum ; portNum += 16)
    {
#ifdef CHX_FAMILY
        if(hwsRavenDevBmp & (1<<(portNum/16) ))
#endif
        {
            if(!mvHwsMtipIsReducedPort(devNum,portNum))
            {
                /* instead of creating a new ipc, we will use OperationIpc*/
                if(GT_OK != (mvHwsPortOperationIpc(devNum, portGroup, portNum, MV_HWS_IPC_PORT_OPERATION_GLOBAL_RX_TERMINATION_SET, portData)) )
                {
                    return GT_FAIL;
                }
            }
        }
    }
    return GT_OK;
}

/**
* @internal mvHwsSetHostModeIpc function
* @endinternal
*
* @brief   Indicate to AP that it is controled by HOST CPU and
*          not MI (relevant for Falcon Only).
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] hostMode              - host mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsSetHostModeIpc
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroup,
    IN  GT_U32               hostMode
)
{
    GT_U32 portData[MV_HWS_IPC_PORT_OPERATION_DATA_LEN_CNS];
    GT_U32 portNum;

    hwsOsMemSetFuncPtr(&portData, 0, MV_HWS_IPC_PORT_OPERATION_DATA_LEN_CNS * sizeof(GT_U32));
    portData[0]= hostMode;
    for(portNum = 0; portNum < hwsDeviceSpecInfo[devNum].portsNum ; portNum += 16)
    {
#ifdef CHX_FAMILY
        if(hwsRavenDevBmp & (1<<(portNum/16) ))
#endif
        {
            if(!mvHwsMtipIsReducedPort(devNum,portNum))
            {
                /* instead of creating a new ipc, we will use OperationIpc*/
                if(GT_OK != (mvHwsPortOperationIpc(devNum, portGroup, portNum, MV_HWS_IPC_PORT_OPERATION_HOST_OR_MI_SET, portData)) )
                {
                    return GT_FAIL;
                }
            }
        }
    }
    return GT_OK;
}

/**
* @internal mvHwsApResolutionBitSetIpc function
* @endinternal
*
* @brief   Set AP AN resolution bit for 40G_KR2
*
* @param[in] devNum                 - system device number
* @param[in] portGroup              - port group (core) number
* @param[in] phyPortNum             - physical port number
* @param[in] portMode               - port mode
* @param[in] anResBit               - resolution bit
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsApResolutionBitSetIpc
(
    GT_U8                        devNum,
    GT_U32                       portGroup,
    GT_U32                       phyPortNum,
    MV_HWS_PORT_STANDARD         portMode,
    GT_U32                       anResBit
)
{
    MV_HWS_IPC_CTRL_MSG_STRUCT requestMsg;
    MV_HWS_IPC_REPLY_MSG_STRUCT replyData;
    MV_HWS_IPC_AN_RES_BIT_INFO_STC anResBitSet = {(GT_U16)portGroup, (GT_U16)phyPortNum, portMode, anResBit};

    /* initialize replyData */
    hwsOsMemSetFuncPtr(&replyData, 0, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));

    /* Construct the msg */
    requestMsg.msgData.anResBitSet = anResBitSet;

    /* Send request to HWS and wait for the reply */
    CHECK_STATUS(mvHwsIpcSendRequestAndGetReply(devNum,&requestMsg,&replyData,
                                                sizeof(MV_HWS_IPC_AN_RES_BIT_INFO_STC),
                                                MV_HWS_IPC_AN_RES_BIT_SET_MSG));

    return (replyData.returnCode);
}

#endif

#endif /*MICRO_INIT*/
