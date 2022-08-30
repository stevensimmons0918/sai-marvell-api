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
* @file snetCheetahTxQ.c
*
* @brief TxQ module processing
*
* @version   10
********************************************************************************
*/

#include <asicSimulation/SKernel/suserframes/snetCheetahTxQ.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahEgress.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahEq.h>
#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SLog/simLogInfoTypePacket.h>

#define TXQ_BUFF_SIZE              30

/**
* @internal simTxqPrintDebugInfo function
* @endinternal
*
* @brief   Print info for given port and queue
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] port                     -  num
* @param[in] queue                    - queue
*/
GT_VOID simTxqPrintDebugInfo
(
    IN SKERNEL_DEVICE_OBJECT   *devObjPtr,
    IN GT_U32                   port,
    IN GT_U32                   queue
)
{
    GT_U32 counter = 0;
    SIM_TRANSMIT_QUEUE_STC *txqPtr;

    txqPtr = &devObjPtr->txqPortsArr[port].onHoldPacketsArr[queue];

    while(txqPtr)
    {
        if(!txqPtr->descrPtr)
        {
            simGeneralPrintf("Empty queue: counter %d, descrPtr %08x, nextDescrPtr %08x, port %d, queue %d , egressPort %d \n",
                    counter, txqPtr->descrPtr, txqPtr->nextDescrPtr, port, queue,txqPtr->egressPort);
        }
        else
        {
            simGeneralPrintf("counter: %d, descrPtr %08x, nextDescrPtr %08x, port %d, queue %d, egressPort %d , descrPtr->numberOfSubscribers %d\n",
                    counter, txqPtr->descrPtr, txqPtr->nextDescrPtr, port, queue,txqPtr->egressPort, txqPtr->descrPtr->numberOfSubscribers);
        }

        counter++;
        txqPtr = txqPtr->nextDescrPtr;
    }
}

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
)
{
    SIM_TRANSMIT_QUEUE_STC *txqPtr, *tempTxqPtr;
    SKERNEL_FRAME_CHEETAH_DESCR_STC *tempDecrPtr;

    if(txqPortNum >= SKERNEL_DEV_MAX_SUPPORTED_TXQ_PORTS_CNS)
    {
        skernelFatalError("simTxqEnqueuePacket: TxQ Port [%d] bigger than supported [%d]", txqPortNum, SKERNEL_DEV_MAX_SUPPORTED_TXQ_PORTS_CNS);
    }

    tempDecrPtr = snetChtEqDuplicateDescrForTxqEnqueue(devObjPtr, descrPtr);
    if (tempDecrPtr == NULL)
    {
        /* we need to return the buffer to the pool */
        descrPtr->frameBuf->freeState = SBUF_BUFFER_STATE_ALLOCATOR_CAN_FREE_E;
        return GT_FAIL;
    }
    
    tempDecrPtr->frameBuf->numberOfSubscribers++;
    tempDecrPtr->numberOfSubscribers++;
    /*indicate that this buffer is still used */
    tempDecrPtr->frameBuf->freeState = SBUF_BUFFER_STATE_OTHER_WILL_FREE_E;

    /* NOTE: use 'txqPortNum' even though this is not 'MAC port' to access devObjPtr->portsArr[port]
        just because we not need real 'MAC port' for this operation !!! */
    txqPtr = &devObjPtr->txqPortsArr[txqPortNum].onHoldPacketsArr[tempDecrPtr->queue_priority];
    if(!txqPtr->descrPtr)
    {
        /* this is first struct */
        if(txqPtr->nextDescrPtr)
        {
            skernelFatalError("simTxqEnqueuePacket: nextDescrPtr must be empty here");
        }
        txqPtr->descrPtr = tempDecrPtr;
        txqPtr->egressPort = egressPort;
        txqPtr->destVlanTagged = destVlanTagged;
    }
    else
    {
        /* search end of the list */
        while(txqPtr->nextDescrPtr)
        {
            txqPtr = txqPtr->nextDescrPtr;
        }
        /* create next struct */
        tempTxqPtr = (SIM_TRANSMIT_QUEUE_STC *)calloc(1, sizeof(SIM_TRANSMIT_QUEUE_STC));
        tempTxqPtr->descrPtr = tempDecrPtr;
        tempTxqPtr->egressPort = egressPort;
        tempTxqPtr->destVlanTagged = destVlanTagged;

        txqPtr->nextDescrPtr = tempTxqPtr;
    }

    /*simTxqPrintDebugInfo(devObjPtr, port, descrPtr->queue_priority);*/

    return GT_OK;
}


/**
* @internal simChtTxqDequeue function
* @endinternal
*
* @brief   Process txq dequeue
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] dataPtr                  - pointer to txq info
* @param[in] dataLength               - length of the data
*/
static GT_VOID simChtTxqDequeue
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN GT_U8                 *dataPtr,
    IN GT_U32                 dataLength
)
{
    DECLARE_FUNC_NAME(simChtTxqDequeue);

    GT_U32                  txqPortNum;
    GT_U32                  queue;
    SIM_TRANSMIT_QUEUE_STC *txqPtr, *tempTxqPtr;
    SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr;

    /* parse the data from the buffer of the message */
    memcpy(&txqPortNum,dataPtr,sizeof(GT_U32));
    dataPtr+=sizeof(GT_U32);
    memcpy(&queue,dataPtr,sizeof(GT_U32));
    dataPtr+=sizeof(GT_U32);

    /* Allow to print to the LOG as if regular packet */
    SIM_OS_MAC(simOsTaskOwnTaskPurposeSet)(
        SIM_OS_TASK_PURPOSE_TYPE_PP_PIPE_PROCESSING_DAEMON_E,NULL);

    /*simGeneralPrintf("simChtTxqDequeue start: port %d, queue %d\n", port, queue);*/
    if(txqPortNum >= SKERNEL_DEV_MAX_SUPPORTED_TXQ_PORTS_CNS)
    {
        skernelFatalError("simChtTxqDequeue: TxQ Port [%d] bigger than supported [%d]", txqPortNum, SKERNEL_DEV_MAX_SUPPORTED_TXQ_PORTS_CNS);
    }

    txqPtr = &devObjPtr->txqPortsArr[txqPortNum].onHoldPacketsArr[queue];

    if(txqPtr->descrPtr)
    {
        __LOG(("########################################## SIM_LOG_CONTINUE_ENQUEUED_PACKET_STR -> on txqPortNum=%d,queue=%d \n",
            txqPortNum,
            queue));
    }


    while(txqPtr->descrPtr)
    {
        descrPtr = txqPtr->descrPtr;
        if(descrPtr->numberOfSubscribers == 0 ||
           descrPtr->frameBuf->numberOfSubscribers == 0)
        {
            skernelFatalError("simChtTxqDequeue: wrong numberOfSubscribers");
        }

        simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_TXQ_E);

        __LOG(("Start : TXQ port[%d] queue[%d] dequeue descriptor (egressPort[%d]) \n",
            txqPortNum,
            queue,
            txqPtr->egressPort));

        descrPtr->continueFromTxqDisabled = 1;
        __LOG_PARAM(descrPtr->continueFromTxqDisabled);

        descrPtr->numberOfSubscribers--;
        descrPtr->frameBuf->numberOfSubscribers--;

        /* allow the frame to finish it's processing from the same spot where is halted */
        snetChtEgressDev_afterEnqueFromTqxDq(devObjPtr, descrPtr,
            txqPtr->egressPort,
            txqPtr->destVlanTagged);

        __LOG(("Ended : TXQ port[%d] queue[%d] dequeue descriptor \n",
            txqPortNum,
            queue));

        if(descrPtr->frameBuf->numberOfSubscribers == 0)
        {
            /* this is the subscriber of this descriptor,
               we need to return the buffer to the pool */
            descrPtr->frameBuf->freeState = SBUF_BUFFER_STATE_ALLOCATOR_CAN_FREE_E;
            sbufFree(devObjPtr->bufPool, descrPtr->frameBuf);
        }

        /* delete node */
        tempTxqPtr = txqPtr->nextDescrPtr;
        if(tempTxqPtr)
        {
            txqPtr->descrPtr     = tempTxqPtr->descrPtr;
            txqPtr->nextDescrPtr = tempTxqPtr->nextDescrPtr;
            free(tempTxqPtr);
        }
        else
        {
            txqPtr->descrPtr = NULL;
            break;
        }
    }
}

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
)
{
    DECLARE_FUNC_NAME(smemTxqSendDequeueMessages);

    SBUF_BUF_ID      bufferId;       /* buffer */
    GT_U8           *dataPtr;        /* pointer to the data in the buffer */
    GT_U32           dataSize;       /* data size */
    GENERIC_MSG_FUNC genFunc = simChtTxqDequeue;/* generic function */

    GT_U32           queue = 0;
    GT_BOOL          isTcTxEnabled = GT_FALSE;

    for(queue = 0; queue < NUM_OF_TRAFFIC_CLASSES; queue++)
    {
        isTcTxEnabled = SMEM_U32_GET_FIELD(regValue, startBitToSum+queue, 1);
        if(isTcTxEnabled)
        {
            if(port >= SKERNEL_DEV_MAX_SUPPORTED_TXQ_PORTS_CNS)
            {
                skernelFatalError("smemTxqSendDequeueMessages: TxQ Port [%d] bigger than supported [%d]", port, SKERNEL_DEV_MAX_SUPPORTED_TXQ_PORTS_CNS);
            }

            /* check that the queue is not empty */
            if(!devObjPtr->txqPortsArr[port].onHoldPacketsArr[queue].descrPtr)
            {
                continue;
            }

            __LOG(("port[%d] queue[%d] is re-enabled for transmit send message for dequeu",port,queue));

            /* queue not empty and queue transmission enabled - so sendMessage */
            /* Get buffer */
            bufferId = sbufAlloc(devObjPtr->bufPool, TXQ_BUFF_SIZE);
            if (bufferId == NULL)
            {
                simWarningPrintf("smemTxqSendDequeueMessages: no buffers for TxQ\n");
                return;
            }

            /* Get actual data pointer */
            sbufDataGet(bufferId, (GT_U8 **)&dataPtr, &dataSize);

            /* put the name of the function into the message */
            memcpy(dataPtr,&genFunc,sizeof(GENERIC_MSG_FUNC));
            dataPtr+=sizeof(GENERIC_MSG_FUNC);

            /* save port */
            memcpy(dataPtr,&port,sizeof(GT_U32));
            dataPtr+=sizeof(GT_U32);

            /* save queue */
            memcpy(dataPtr,&queue,sizeof(GT_U32));
            dataPtr+=sizeof(GT_U32);

            /* set source type of buffer  */
            bufferId->srcType = SMAIN_SRC_TYPE_CPU_E;

            /* set message type of buffer */
            bufferId->dataType = SMAIN_MSG_TYPE_GENERIC_FUNCTION_E;

            /* put buffer to queue        */
            squeBufPut(devObjPtr->queueId, SIM_CAST_BUFF(bufferId));
        }
        else
        {
            /* queue transmission disabled - do nothing */
            /*simWarningPrintf("smemChtActiveWriteTxQConfigReg: port %d tc %d disabled\n", port, tc);*/
        }
    }
}





