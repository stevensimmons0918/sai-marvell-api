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
* @file snetFalconTxQ.h
*
* @brief Falcon TxQ module processing
*
* @version   1
********************************************************************************
*/
#ifndef __snetFalconTxQh
#define __snetFalconTxQh

#include <asicSimulation/SKernel/smain/smain.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
queue_pdx_index - PDX ID destination  -- (tile number 0..3)
queue_pds_index - PDS ID destination  -- (local in tile) DP index (0..7)
                  NOTE: in tiles 1,3 it is 'mirrored DP[]' to the one in tile 0
dp_core_local_trg_port - The port number used by the PDX burst fifo, PDS and TX -- (local in DP) DMA number (0..8) - used for PFC only
queue_base - The queue base used by the SDQ and PDQ  -- added with 'queue_group_offset' to form EGF
*/

typedef struct
{
    GT_U32  queue_base;
    GT_U32  dp_core_local_trg_port;
    GT_U32  queue_pds_index;
    GT_U32  queue_pdx_index;
}QUEUE_GROUP_MAP_ENTRY_STC;


/**
* @internal snetFalconTxqProcess function
* @endinternal
*
* @brief   Falcon : do TXQ processing , to be ready to send to TxFifo , TxDma , MAC.
*         The 'Global DMA' is saved into : descrPtr->egressPhysicalPortInfo.txDmaMacPort.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
*/
GT_VOID snetFalconTxqProcess
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

/**
* @internal snetFalconTxqPdxQueueGroupMap function
* @endinternal
*
* @brief   Falcon : PDX : map the <sip6_queue_group_index> to :
*         1. Queue PDX index - PDX ID destination -- (tile number 0..3)
*         2. Queue PDS index - PDS ID destination -- (local in tile) DP index (0..7)
*         3. DP Core Local Trg Port - The port number used by the PDX burst fifo, PDS and TX -- (local in DP) DMA number (0..8) - used for PFC only
*         4. Queue base - The queue base used by the SDQ and PDQ -- added with 'queue_group_offset' to form EGF
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
*
* @param[out] queueMapInfoPtr          - the info read from QueueGroupMap.
*                                      COMMENTS :
*/
GT_VOID snetFalconTxqPdxQueueGroupMap
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    OUT QUEUE_GROUP_MAP_ENTRY_STC   *queueMapInfoPtr
);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __snetFalconTxQh */



