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
* @file snetPipe.h
*
* @brief
*
* @version   1
********************************************************************************
*/
#ifndef __snetPipeh
#define __snetPipeh

#include <asicSimulation/SKernel/smain/smain.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal snetChtPerformFromCpuDma_forPipeDevice function
* @endinternal
*
* @brief   needed by PIPE device : Copy frame data from CPU memory to bufferId
*/
GT_VOID snetChtPerformFromCpuDma_forPipeDevice
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 txQue,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    OUT GT_BOOL * queueEmptyPtr,
    OUT GT_BOOL * isLastPacketPtr
);

/**
* @internal snetChtInterruptTxSdmaEnd_forPipeDevice function
* @endinternal
*
* @brief   needed by PIPE device : generate TX SDMA END interrupt on specific queue
*/
GT_VOID snetChtInterruptTxSdmaEnd_forPipeDevice
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 txQue
);


/**
* @internal snetChtTxMacPortGet_forPipeDevice function
* @endinternal
*
* @brief   needed by PIPE device : Get TX MAC number for 'MAC_PORT_REASON_TYPE_TXQ_DQ_TO_TXDMA_E'
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - the descriptor. relevant when != NULL and needed for:
*                                      reason == MAC_PORT_REASON_TYPE_TXQ_DQ_TO_TXDMA_E
* @param[in] egressPort               - egress physical port number from the EGF to get the MAC number
* @param[in] needGlobalToLocalConvert - for SIP5 - indication that need global to local port conversion
*
* @param[out] txDmaDevObjPtrPtr        - (pointer to) pointer to the 'TX DMA port' egress device object
*                                      can be NULL
* @param[out] txDmaLocalPortPtr        - (pointer to) local port number of the 'TX DMA port'
*                                      TX MAC number:
*                                      Extended mode:  port 9 - MAC 12, port 11 - MAC 14
*                                      Normal mode: MAC number = port number
*                                      RETURN:
*                                      indication that port exists -
*                                      GT_TRUE - port exists
*                                      GT_FALSE - port not exists (this case can happen on NULL port ...)
*/
GT_BOOL snetChtTxMacPortGet_forPipeDevice
(
    IN SKERNEL_DEVICE_OBJECT   *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32                  egressPort,
    IN GT_BOOL                 needGlobalToLocalConvert,
    OUT SKERNEL_DEVICE_OBJECT  **txDmaDevObjPtrPtr,
    OUT GT_U32                 *txDmaLocalPortPtr
);

typedef enum{
    TXQ_COUNTE_MODE_ALL_E,
    TXQ_COUNTE_MODE_INGRESS_DEVICE_ONLY_E,/*ingress device*/
    TXQ_COUNTE_MODE_EGRESS_DEVICE_ONLY_E/*egress device*/
}TXQ_COUNTE_MODE_ENT;

typedef struct{
    GT_CHAR*    fieldName;
    GT_U32      numBytes;
}TCAM_KEY_METADATA_STC;

/**
* @internal snetChtTxQCounterSets_forPipeDevice function
* @endinternal
*
* @brief   Update egress counters
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] destPorts[]              - destination port vector.
*                                      destVlanTagged  - destination tagged ports.
* @param[in] packetType               - type of packet.
* @param[in] forwardRestrictionBmpPtr - pointer to forwarding restrictions ports bitmap
* @param[in] egressPort               - egress port - relevant only when destPorts == NULL
*                                       None
*/
void snetChtTxQCounterSets_forPipeDevice
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 destPorts[],
    IN SKERNEL_PORTS_BMP_STC *pktFilteredPortsBmpPtr,
    IN SKERNEL_EGR_PACKET_TYPE_ENT packetType,
    IN SKERNEL_PORTS_BMP_STC *forwardRestrictionBmpPtr,
    IN TXQ_COUNTE_MODE_ENT counteMode,
    IN GT_U32   egressPort
);

/**
* @internal txqToPortQueueDisableCheck_forPipeDevice function
* @endinternal
*
* @brief   needed by PIPE device : Analyse egressPort for disabled TC
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - egress port
*
* @retval is queue disabled        - GT_TRUE - queue disabled
* @retval GT_FALSE                 - queue enabled
*
* @note Egress ports with (disabled TC) == (packet TC for egress port)
*       need to be removed from destPorts[portInx].
*       All other ports where disabled TC != packet TC
*       need to be handled in the snetChtEgressDev.
*
*/
GT_BOOL txqToPortQueueDisableCheck_forPipeDevice
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN    SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN    GT_U32                           egressPort
);

/**
* @internal egressTxFifoAndMac_forPipeDevice function
* @endinternal
*
* @brief   needed by PIPE device : let the TxFifo and MAC to egress the packet
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
* @param[in] txFifoPort               - egress Port (tx fifo port)
* @param[in] frameDataPtr             - pointer to frame data
* @param[in] frameDataSize            - frame data size
*                                       COMMENTS :
*/
void egressTxFifoAndMac_forPipeDevice
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32               txFifoPort,
    IN GT_U8                *frameDataPtr,
    IN GT_U32               frameDataSize
);

/**
* @internal snetPipeFrameProcess function
* @endinternal
*
* @brief   Process frame from 'MAC' port (not SDMA)
*/
GT_VOID snetPipeFrameProcess
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SBUF_BUF_ID bufferId,
    IN GT_U32 srcPort
);

/**
* @internal snetPipeFromCpuDmaProcess function
* @endinternal
*
* @brief   Process frame from 'MG TXQ-SDMA' port (not MAC)
*/
GT_VOID snetPipeFromCpuDmaProcess
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SBUF_BUF_ID bufferId
);

/**
* @internal snetPipeEgressPacketProc function
* @endinternal
*
* @brief   Egress pipe Processing (for PIPE device)
*/
GT_VOID snetPipeEgressPacketProc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_PIPE_DESCR_STC * pipe_descrPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __snetPipeh */



