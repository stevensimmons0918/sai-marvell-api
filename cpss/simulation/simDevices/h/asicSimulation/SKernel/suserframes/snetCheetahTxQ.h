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
* @file snetCheetahTxQ.h
*
* @brief This is a header file for TxQ processing
*
* @version   10
********************************************************************************
*/
#ifndef __snetCheetahTxQh
#define __snetCheetahTxQh

#include <asicSimulation/SKernel/smain/smain.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal simTxqEnqueuePacket function
* @endinternal
*
* @brief   Enqueue the packet for given port and tc (descrPtr->queue_priority).
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - destination port <local dev target physical port>
* @param[in] txqPortNum               - the TXQ port that enqueue this packet
* @param[in] destVlanTagged           - the egress tag state
*
* @retval GT_STATUS                - GT_OK if succeed
*/
GT_STATUS simTxqEnqueuePacket
(
    IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN GT_U32                           egressPort,
    IN GT_U32                           txqPortNum,
    IN GT_U32                           destVlanTagged
);

/**
* @internal simTxqPrintDebugInfo function
* @endinternal
*
* @brief   Print info for given port and tc
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] port                     -  num
* @param[in] tc                       - traffic class
*/
GT_VOID simTxqPrintDebugInfo
(
    IN SKERNEL_DEVICE_OBJECT   *devObjPtr,
    IN GT_U32                   port,
    IN GT_U32                   tc
);


/**
* @internal smemTxqSendDequeueMessages function
* @endinternal
*
* @brief   Send dequeue messages
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] regValue                 - register value.
* @param[in] port                     -  num
* @param[in] startBitToSum            - start bit to read
*/
GT_VOID smemTxqSendDequeueMessages
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32  regValue,
    IN GT_U32  port,
    IN GT_U32  startBitToSum
);

/**
* @internal snetHaTablesFormatInit function
* @endinternal
*
* @brief   init the format of HA tables.
*
* @param[in] devObjPtr                - pointer to device object.
*/
void snetHaTablesFormatInit(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __snetCheetahTxQh */



