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
* mvHwsPortCtrlIpc.c
*
* DESCRIPTION:
*       FreeRTOS only;
*       Port Control AP IPC API
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#include <mvHwsPortCtrlInc.h>
#include <mvHwsPortCtrlIpc.h>
#include <mvPortModeElements.h>
#include "common.h"
#include <mvShmIpc_wrapper.h>
#include <ic.h>
#include <mvHwsPortCfgIf.h>

MV_HWS_IPC_CTRL_MSG_STRUCT ipcMsg;

#if defined(BC2_DEV_SUPPORT)|| defined(AC5_DEV_SUPPORT)
#ifndef AC5_DEV_SUPPORT
extern GT_U32 portBC2SerdesNum[];
#endif
extern GT_U32 portAC3SerdesNum[];
#else /*  not BC2_DEV_SUPPORT*/
#if !defined(BC3_DEV_SUPPORT) && !defined(ALDRIN2_DEV_SUPPORT) && !defined(PIPE_DEV_SUPPORT) && !defined(RAVEN_DEV_SUPPORT) && !defined(FALCON_DEV_SUPPORT)
extern GT_U32 portSerdesNum[];
#endif
#endif

#ifdef FALCON_DEV_SUPPORT
extern MV_BOOL getSkipResEnabled(GT_U32 portMacNum);
#endif

#define AP_1G_SINGLE_CAPABILITY(mode)      (AP_CTRL_1000Base_KX_GET(mode)  && (0==AP_CTRL_10GBase_KR_GET(mode)) && (0==AP_CTRL_ADV_25G_ALL_GET(mode)))
#define AP_10G_SINGLE_CAPABILITY(mode)     (AP_CTRL_10GBase_KR_GET(mode) && (0==AP_CTRL_ADV_25G_ALL_GET(mode)))
#define AP_25G_SINGLE_CAPABILITY(mode)     (AP_CTRL_ADV_25G_ALL_GET(mode) && (0==AP_CTRL_10GBase_KR_GET(mode)))
#define AP_10G_25G_CAPABILITY(mode)        (AP_CTRL_10GBase_KR_GET(mode) && AP_CTRL_ADV_25G_ALL_GET(mode))

#ifndef CONFIG_MV_IPC_FREERTOS_DRIVER
/**
* @internal host2servicecpu_isr function
* @endinternal
*
* @brief   This interrupt handling only works when servicecpu is not working in ipc mode
*         It's trigged when host sends a doorbell irq to servicecpu
*         This interrupt handling only works for MSYS family boards
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static long host2servicecpu_isr(void *data)
{
    MV_U32 reg;

    mv_os_printf("Interrupt from host CPU\n");

    MV_REG_BIT_RESET(CPU_DOORBELL_IN_MASK_REG, 1 << IPC_BASE_DOORBELL);
    MV_REG_WRITE(CPU_DOORBELL_IN_REG, ~(1 << IPC_BASE_DOORBELL));

    /* Generate a doorbell irq to host CPU */
    mv_os_printf("Generate a doorbell irq to host CPU back\n");
    MV_REG_WRITE(CPU_DOORBELL_OUT_REG, 1 << IPC_BASE_DOORBELL);

    MV_REG_BIT_SET(CPU_DOORBELL_IN_MASK_REG, (1));

    return 0;
}
#endif /* CONFIG_MV_IPC_FREERTOS_DRIVER */

#if 0
/**
* @internal ipc_request_irq function
* @endinternal
*
* @brief   This API register polling task handler
*         It is implemented via Port Control Supervisour task
*         The API is for compatibility with IPC link structure
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static int ipc_request_irq(unsigned int link_id, void *dev_id)
{
    /* This API register polling task handler */
    /* It is implemented via Port Control Supervisour task */
    return 0;
}
#endif

#if 0 /*Not used*/
/**
* @internal mvIpcSendTriggerPolling function
* @endinternal
*
*/
MV_VOID mvIpcSendTriggerPolling(MV_U32 linkId, MV_U32 chnId)
{
    /* Do nothing for polling */
    return;
}
#endif

#if (defined BC2_DEV_SUPPORT) || (defined AC3_DEV_SUPPORT)
/**
* @internal mvIpcPCIDrblInit function
* @endinternal
*
*/
static MV_VOID mvIpcPCIDrblInit(void)
{
    /* Forward Outbound doorbell to PCIe Host */
    /* now all ipc channels share one doorbell - IPC_BASE_DOORBELL*/
    MV_REG_WRITE(CPU_DOORBELL_OUT_MASK_REG, 1 << IPC_BASE_DOORBELL);

    MV_REG_BIT_SET(CPU_DOORBELL_IN_MASK_REG, 1 << IPC_BASE_DOORBELL);

    MV_REG_WRITE(CPU_INT_SRC_CTRL(IRQ_OUT_DOORBELL_SUM),
                 (1 << ICBIT_SRC_CTRL_EP_MASK)  |
                 (1 << ICBIT_SRC_CTRL_INT_EN)   |
                 (1 << ICBIT_SRC_CTRL_PRIORITY) |
                 (1 << ICBIT_SRC_CTRL_EP_SEL));

#ifndef CONFIG_MV_IPC_FREERTOS_DRIVER
    /* Register Doorbell interrupt handler */
    iICRegisterHandler(IRQ_IN_DOORBELL_LOW, host2servicecpu_isr, (void *)0,
                       IRQ_ENABLE, IRQ_PRIO_DEFAULT);
    iICRegisterHandler(IRQ_IN_DOORBELL_HIGH, host2servicecpu_isr, (void *)1,
                       IRQ_ENABLE, IRQ_PRIO_DEFAULT);
#endif /* CONFIG_MV_IPC_FREERTOS_DRIVER */

    return;
}
#endif /* #if (defined BC2_DEV_SUPPORT) || (defined AC3_DEV_SUPPORT)*/

/**
* @internal mvPortCtrlIpcInit function
* @endinternal
*
* @brief   Initialize Port control IPC infrastructure
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvPortCtrlIpcInit(void)
{
#if defined BC2_DEV_SUPPORT
    mvIpcPCIDrblInit();
#endif
    wr_Ipc_Init((void *)SHMEM_BASE, (void*)SHMEM_REGS, SHMEM_SIZE);

    return GT_OK;
}

/**
* @internal mvPortCtrlIpcActivate function
* @endinternal
*
* @brief   Initialize Port control IPC infrastructure
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvPortCtrlIpcActivate(void)
{
    return GT_OK;
}

/**
* @internal mvHwsIpcRequestGet function
* @endinternal
*
* @brief   The API is called from Supervisour process to retrive message
*         from IPC driver
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsIpcRequestGet(GT_U32 priority, GT_U32 msgSize, char *msg)
{
#ifndef FALCON_DEV_SUPPORT
    int channel;
#endif

#ifdef MICRO_INIT
    if (wr_Ipc_boot_channel_ready())
    {
        if(wr_Ipc_boot_channel_receive())
        {
            return GT_FAIL;
        }
    }
#endif

    if (priority != MV_HWS_IPC_HIGH_PRI_QUEUE && priority != MV_HWS_IPC_LOW_PRI_QUEUE)
        return GT_FAIL;
#ifndef FALCON_DEV_SUPPORT
    channel = (priority == MV_HWS_IPC_HIGH_PRI_QUEUE)
            ? MV_HWS_IPC_HI_PRI_RX_CH_ID : MV_HWS_IPC_LO_PRI_RX_CH_ID;
    if (wr_Ipc_rx_msg(channel, msg) == 1)
    {
        return GT_OK;
    }
#endif
    return GT_FAIL;
}

/**
* @internal mvHwsIpcReplyMsg function
* @endinternal
*
* @brief   Buils and Send reply from HW Services to the Host
*
* @param[in] queueId                  - queue ID
* @param[in] txReplyMsg               - pointer to message
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsIpcReplyMsg
(
    GT_U32                      queueId,
    MV_HWS_IPC_REPLY_MSG_STRUCT *txReplyMsg
)
{
    int status=-1;

    if (queueId == MV_PORT_CTRL_NO_MSG_REPLY)
    {
        return GT_OK;
    }

    mvPortCtrlIpcLock();
    status = wr_Ipc_tx_queue_send(queueId, txReplyMsg, sizeof(MV_HWS_IPC_REPLY_MSG_STRUCT));
    mvPortCtrlIpcUnlock();
    if (status == -1)
    {
        mv_os_printf("IPC: Failed to sent reply message via channel %d\n", queueId);
    }

    return GT_OK;
}

/**
* @internal mvHwsIpcRequestSet function
* @endinternal
*
* @brief   IPC command simulation
*
* @param  portOpt - bitmask
*       Bits 0-15 - placeholder for polarity bits
*       Bits 16-31 - placeholder for FEC config data
*
* @note param 'portOpt' is used by Micro-init only
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsIpcRequestSet(GT_U32 type, GT_U32 port, GT_U32 mode, GT_U32 portOpt)
{
    GT_STATUS rc = GT_OK;
#if defined (AN_PORT_SM) && !defined (FALCON_DEV_SUPPORT)
    MV_PORT_CTRL_MODULE module = M7_AN_PORT_SM;
#endif
    ipcMsg.devNum = 0;
    ipcMsg.msgData.portInit.portGroup = 0;
    ipcMsg.msgData.portInit.phyPortNum = port;

    /* Code not relevant for BobK CM3 due to space limit */
#if (!defined MV_HWS_REDUCED_BUILD) || defined(MICRO_INIT)
#ifdef REG_PORT_TASK
    if (type == MV_HWS_IPC_PORT_INIT_MSG)
    {
        ipcMsg.ctrlMsgType = MV_HWS_IPC_PORT_INIT_MSG;
        ipcMsg.msgQueueId = MV_PORT_CTRL_NO_MSG_REPLY;
        ipcMsg.msgLength = sizeof(MV_HWS_IPC_PORT_INIT_DATA_STRUCT);
        ipcMsg.msgData.portInit.portMode = mode;
        ipcMsg.msgData.portInit.lbPort = GT_FALSE;
        ipcMsg.msgData.portInit.refClock = MHz_156;
        ipcMsg.msgData.portInit.refClockSource = PRIMARY_LINE_SRC;
        return mvPortCtrlProcessMsgSend(M2_PORT_MNG, &ipcMsg);
    }
#endif
#ifdef AP_GENERAL_TASK
    else
    if (type == MV_HWS_IPC_PORT_LINK_STATUS_GET_MSG)
    {
        ipcMsg.ctrlMsgType = MV_HWS_IPC_PORT_LINK_STATUS_GET_MSG;
        ipcMsg.msgQueueId = MV_PORT_CTRL_NO_MSG_REPLY;
        ipcMsg.msgLength = sizeof(MV_HWS_IPC_PORT_INFO_STRUCT);
        ipcMsg.msgData.portLinkStatus.portMode = mode;
        return mvPortCtrlProcessMsgSend(M6_GENERAL, &ipcMsg);
    }
    else if (type == MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_MSG)
    {
        ipcMsg.ctrlMsgType = MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_MSG;
        ipcMsg.msgQueueId = MV_PORT_CTRL_NO_MSG_REPLY;
        ipcMsg.msgLength = sizeof(MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_DATA_STRUCT);
        ipcMsg.msgData.portAutoTuneSetExt.portMode = (mode & 0xFFFF);
        ipcMsg.msgData.portAutoTuneSetExt.portTuningMode = ((mode >> 16) & 0xFFFF);
        return mvPortCtrlProcessMsgSend(M6_GENERAL, &ipcMsg);
    }
#endif
#ifdef REG_PORT_TASK
    else
#endif
#endif
    if (type == MV_HWS_IPC_PORT_AP_ENABLE_MSG)
    {
        return mvHwsIpcApRequestSet(type, port, mode, ((portOpt >> 16) & 0xFF)/* FEC ability mask */, ((portOpt >> 24) & 0xFF)/* FEC request mask */, (GT_U8)(portOpt & 0xFF)/* polarity vector mask */);
    }
    else if (type == MV_HWS_IPC_PORT_AP_DISABLE_MSG)
    {
        ipcMsg.ctrlMsgType = MV_HWS_IPC_PORT_AP_DISABLE_MSG;
        ipcMsg.msgQueueId = MV_PORT_CTRL_NO_MSG_REPLY;
        ipcMsg.msgLength = sizeof(MV_HWS_IPC_PORT_AP_DATA_STRUCT);
#if defined AP_PORT_SM && !defined (FALCON_DEV_SUPPORT)
        rc = mvPortCtrlProcessMsgSend(M5_AP_PORT_DET, &ipcMsg);
#endif
#ifdef AN_PORT_SM
#if !defined (FALCON_DEV_SUPPORT)
        if (port >= MV_HWS_PORT_CTRL_AN_NUM_PORTS_PER_TASK_CNS)
        {
            module = M9_AN_PORT_SM2;
        }
        rc = mvPortCtrlProcessMsgSend(module, &ipcMsg);
#else
        ipcMsg.msgData.apConfig.portGroup = 0;
        ipcMsg.msgData.apConfig.phyPortNum = port%16;
        rc = (MV_STATUS)wr_Ipc_tx_queue_send_raven(port/16, 1,&ipcMsg,sizeof(ipcMsg));
#endif
#endif
        return rc;
    }
#if (!defined MV_HWS_REDUCED_BUILD) || defined(MICRO_INIT)
#ifdef REG_PORT_TASK
    else if (MV_HWS_IPC_PORT_RESET_MSG == type)
    {
        ipcMsg.ctrlMsgType = MV_HWS_IPC_PORT_RESET_MSG;
        ipcMsg.msgQueueId = MV_PORT_CTRL_NO_MSG_REPLY;
        ipcMsg.msgLength = sizeof(MV_HWS_IPC_PORT_RESET_DATA_STRUCT);
        ipcMsg.msgData.portReset.action = 0 /* PORT_POWER_DOWN */;
        ipcMsg.msgData.portReset.portMode = mode;
        return mvPortCtrlProcessMsgSend(M2_PORT_MNG, &ipcMsg);
    }
#endif
    else if ((MV_HWS_IPC_PORT_AP_SYS_CFG_VALID_MSG == type)||(MV_HWS_IPC_PORT_AP_ENABLE_WAIT_ACK_MSG == type))
    {
        ipcMsg.ctrlMsgType = type;
        ipcMsg.msgQueueId = MV_PORT_CTRL_NO_MSG_REPLY;
        ipcMsg.msgLength = sizeof(MV_HWS_IPC_PORT_INFO_STRUCT);
        ipcMsg.msgData.portReset.portMode = mode;
        ipcMsg.msgData.portFecConfig.portGroup = 0;
        ipcMsg.msgData.portFecConfig.portMode = mode;
        ipcMsg.msgData.portFecConfig.phyPortNum = port;
        ipcMsg.msgData.portFecConfig.portFecType = ((portOpt >> 16) & 0xFF);/* FEC type offset */
#ifndef FALCON_DEV_SUPPORT
        mvPortCtrlSpvHighMsgExecute(&ipcMsg);
#endif
        return GT_OK;
    }
#endif
    return GT_NOT_IMPLEMENTED;
}

/**
 * mvHwsIpcApRequestSet
 *
 * @brief   IPC command simulation which can be used by AP FW on
 *          a Service CPU (CM3) for debugging purpose or by
 *          Micro-init to create an AP port
 *
 * @param type
 * @param portMacNum
 * @param mode
 * @param fecAbil
 * @param fecReq
 * @param polarity
 *
 * @return GT_STATUS
 */
GT_STATUS mvHwsIpcApRequestSet(GT_U32 type,
                               GT_U32 portMacNum,
                               GT_U32 mode,
                               GT_U32 fecAbil,
                               GT_U32 fecReq,
                               GT_U16 polarity
                               )
{
#ifndef RAVEN_DEV_SUPPORT
    if (type == MV_HWS_IPC_PORT_AP_ENABLE_MSG)
        return mvHwsIpcApEnable(portMacNum, 0, mode, fecAbil, fecReq, 0, 0, polarity);
    else
#endif
        return GT_NOT_IMPLEMENTED;
}

/**
 * @note AP port debugging is not currently supported for RAVEN
 */
#ifndef RAVEN_DEV_SUPPORT

/**
* @internal mvHwsIpcApEnable function
* @endinternal
*
* @brief   IPC command simulation which can be
*           used by AP FW on a Service CPU (CM3) for debugging
*           purpose or by Micro-init to create an AP port
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsIpcApEnable(GT_U32 portMacNum,
                           GT_U32 relLaneNum,
                           GT_U32 portAdvMode,
                           GT_U32 fecAbil,
                           GT_U32 fecReq,
                           GT_U32 fcPause,
                           GT_U32 fcAsmDir,
                           GT_U16 polarity
                           )
{
    GT_BOOL consortium = GT_FALSE;
    GT_U32 fec = 0;
    GT_U32 reqLanes;
    MV_HWS_PORT_STANDARD portMode;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_STATUS rc;
#if defined (AN_PORT_SM) && !defined (FALCON_DEV_SUPPORT)
    MV_PORT_CTRL_MODULE module = M7_AN_PORT_SM;
#endif
#ifdef FALCON_DEV_SUPPORT
    int ret;
#endif

    ipcMsg.devNum = 0;
    ipcMsg.ctrlMsgType = MV_HWS_IPC_PORT_AP_ENABLE_MSG;
    ipcMsg.msgQueueId = MV_PORT_CTRL_NO_MSG_REPLY;
    ipcMsg.msgLength = sizeof(MV_HWS_IPC_PORT_AP_DATA_STRUCT);
    ipcMsg.msgData.apConfig.portGroup = 0;
#ifdef FALCON_DEV_SUPPORT
    ipcMsg.msgData.apConfig.phyPortNum = portMacNum %16;
#else
    ipcMsg.msgData.apConfig.phyPortNum = portMacNum;
#endif

    if (GT_OK != mvHwsPortApGetMaxLanesPortModeGet(portAdvMode, &portMode))
    {
        return GT_BAD_PARAM;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(0, 0, portMacNum, portMode, &curPortParams))
    {
        return GT_BAD_PARAM;
    } else {
        reqLanes = (curPortParams.serdesMediaType == RXAUI_MEDIA) ? 2 * curPortParams.numOfActLanes : curPortParams.numOfActLanes;
        if (relLaneNum >= reqLanes ) {
             osPrintf("(failed): laneNum %d reqLanes %d\n", relLaneNum, reqLanes);
            return GT_BAD_PARAM;
        }
    }

#if defined(MICRO_INIT)
#ifdef ALDRIN_DEV_SUPPORT
        /* Build params for the refClockSourceDb with refClock=MHz_78 (bits 4-7), refClockSource=SECONDARY_LINE_SRC (bits 1-3) and isValid=1 (bit 0) */
        ipcMsg.msgData.apConfig.refClockSrcParams = 0x13;
#elif defined(FALCON_DEV_SUPPORT)
        ipcMsg.msgData.apConfig.refClockSrcParams = 1;
#endif
        ipcMsg.msgData.apConfig.polarityVector = polarity;
#endif

#if defined (BC2_DEV_SUPPORT)
        ipcMsg.msgData.apConfig.laneNum = portBC2SerdesNum[portMacNum - MV_PORT_CTRL_BC2_AP_PORT_NUM_BASE];
#elif defined (AC3_DEV_SUPPORT)
        ipcMsg.msgData.apConfig.laneNum = portAC3SerdesNum[portMacNum - MV_PORT_CTRL_AC3_AP_PORT_NUM_BASE];
#elif defined(AC5_DEV_SUPPORT)
        ipcMsg.msgData.apConfig.laneNum = portAC3SerdesNum[portMacNum - MV_PORT_CTRL_AP_PORT_NUM_BASE];
#elif defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)|| defined(RAVEN_DEV_SUPPORT)
        ipcMsg.msgData.apConfig.laneNum = portMacNum;
#elif defined(FALCON_DEV_SUPPORT)
        ipcMsg.msgData.apConfig.laneNum = curPortParams.activeLanesList[0]%16;
#else /* if bobk */
        ipcMsg.msgData.apConfig.laneNum = portSerdesNum[portMacNum - MV_PORT_CTRL_AP_PORT_NUM_BASE];
#endif
        ipcMsg.msgData.apConfig.laneNum += relLaneNum;
#if defined (BC2_DEV_SUPPORT) || defined (AC5_DEV_SUPPORT) || defined(FALCON_DEV_SUPPORT)
        ipcMsg.msgData.apConfig.pcsNum = ipcMsg.msgData.apConfig.phyPortNum;
#else
        ipcMsg.msgData.apConfig.pcsNum = ipcMsg.msgData.apConfig.laneNum;
#endif
        ipcMsg.msgData.apConfig.macNum = ipcMsg.msgData.apConfig.phyPortNum;
        ipcMsg.msgData.apConfig.advMode = portAdvMode;
#ifndef ALDRIN_DEV_SUPPORT
        if (AP_CTRL_25GBase_KR1_GET(portAdvMode)) {
            AP_CTRL_25GBase_KR1_SET(ipcMsg.msgData.apConfig.advMode, 1);
            AP_CTRL_25GBase_KR1S_SET(ipcMsg.msgData.apConfig.advMode, 1);
        }
        if (AP_CTRL_25GBase_CR1_GET(portAdvMode)) {
            AP_CTRL_25GBase_CR1_SET(ipcMsg.msgData.apConfig.advMode, 1);
            AP_CTRL_25GBase_CR1S_SET(ipcMsg.msgData.apConfig.advMode, 1);
        }
#ifdef FALCON_DEV_SUPPORT
        if (AP_CTRL_50GBase_KR1_GET(portAdvMode)) {
                AN_CTRL_SKIP_RES_SET(ipcMsg.msgData.apConfig.options, getSkipResEnabled(portMacNum));
        }
#endif
#endif
        //ipcMsg.msgData.apConfig.options = 0x200; /* [09:09] loopback Enable */

        osPrintf("MacNum %d LaneNum %d portAdvMode %d fecAbil %d fecReq %d polarity 0x%x\n",
         portMacNum, ipcMsg.msgData.apConfig.laneNum, portAdvMode,fecAbil, fecReq, polarity);
        /*
        ** Options
        ** [00:00] Flow Control Pause Enable
        ** [01:01] Flow Control Asm Direction
        ** [02:02] Remote Flow Control Pause Enable
        ** [03:03] Remote Flow Control Asm Direction
        ** [04:04] FEC Suppress En
        ** [05:05] FEC Request
        ** [06:07] Reserved  = RS-FEC requested, BASE-R FEC requested (for 25G IEEE)
        ** [09:09] loopback Enable
        ** [10:10]consortium (RS-FEC) ability (F1)
        ** [11:11]consortium (BASE-R FEC) ability (F2)
        ** [12:12]consortium (RS-FEC) request (F3)
        ** [13:13]consortium (BASE-R FEC) request (F4)
        ** [14:15] Reserved
        */

        if ( AP_10G_SINGLE_CAPABILITY(portAdvMode)) {
            if (consortium) {
                fec = 2;
            } else {
                fec = 1;/*Always FC-FEC*/
            }
        }
#ifndef ALDRIN_DEV_SUPPORT
        else if ( AP_CTRL_ADV_25G_ALL_GET(portAdvMode)) {
            fec = 3;/*Always both*/
        }
#endif
        else {
            fec = 0;
        }

        if (consortium) {
            AP_CTRL_FEC_ABIL_CONSORTIUM_SET(ipcMsg.msgData.apConfig.options, fec);
        } else {
            AP_CTRL_FEC_ABIL_SET(ipcMsg.msgData.apConfig.options, fec);
        }

        if ( (fecReq & 0xF) != 0 && AP_10G_SINGLE_CAPABILITY(portAdvMode)) {
            if (consortium) {
                fec = 2;
                AP_CTRL_FEC_REQ_CONSORTIUM_SET(ipcMsg.msgData.apConfig.options, fec);
            } else {
                fec = 1;
                AP_CTRL_FEC_REQ_SET(ipcMsg.msgData.apConfig.options, fec);/*Enable – FC-FEC bit*/
            }
        }
#ifndef ALDRIN_DEV_SUPPORT
        else if ( (fecReq & 0xF) != 0 && AP_25G_SINGLE_CAPABILITY(portAdvMode)) {
            fec = 0;
            if (consortium) {
                AP_CTRL_FEC_REQ_CONSORTIUM_SET(ipcMsg.msgData.apConfig.options, 2);
            } else {
                AP_CTRL_FEC_REQ_SET(ipcMsg.msgData.apConfig.options, fec);/*Always 0*/
                AP_CTRL_FEC_ADVANCED_REQ_SET(ipcMsg.msgData.apConfig.options, 2);/*Enable – RS-FEC bit 7*/
            }
        }
#endif
        else if ( (fecReq & 0xF) != 0 && AP_10G_25G_CAPABILITY(portAdvMode)) {
            fec = 2;
            if (consortium) {
                AP_CTRL_FEC_REQ_CONSORTIUM_SET(ipcMsg.msgData.apConfig.options, fec);
            } else {
                AP_CTRL_FEC_REQ_SET(ipcMsg.msgData.apConfig.options, 1);/*Enable – FC-FEC bit*/
            }
            AP_CTRL_FEC_ADVANCED_REQ_SET(ipcMsg.msgData.apConfig.options, fec);/*Enable – RS-FEC bit*/
        }else {
            fec = 0;/*Disable - 0*/
            if (consortium) {
                AP_CTRL_FEC_REQ_CONSORTIUM_SET(ipcMsg.msgData.apConfig.options, fec);
            } else {
                AP_CTRL_FEC_REQ_SET(ipcMsg.msgData.apConfig.options, fec);
            }
            AP_CTRL_FEC_ADVANCED_REQ_SET(ipcMsg.msgData.apConfig.options, fec);
        }

#if !defined(FALCON_DEV_SUPPORT)
        AP_CTRL_FC_PAUSE_SET(ipcMsg.msgData.apConfig.options, fcPause);
        AP_CTRL_FC_ASM_SET(ipcMsg.msgData.apConfig.options, fcAsmDir);
#endif

#if defined(AP_PORT_SM) && !defined(FALCON_DEV_SUPPORT)
        rc = mvPortCtrlProcessMsgSend(M5_AP_PORT_DET, &ipcMsg);
#endif
#ifdef AN_PORT_SM
#if defined(FALCON_DEV_SUPPORT)
        ret = wr_Ipc_tx_queue_send_raven(portMacNum/16, 1, &ipcMsg, sizeof(ipcMsg));
        if (ret == 0)
            rc = GT_OK;
        else
            rc = GT_FAIL;
#else
        if (portMacNum >= MV_HWS_PORT_CTRL_AN_NUM_PORTS_PER_TASK_CNS)
        {
            module = M9_AN_PORT_SM2;
        }
        rc = mvPortCtrlProcessMsgSend(module, &ipcMsg);
#endif
#endif
        return rc;
}
#endif /*RAVEN_DEV_SUPPORT*/
