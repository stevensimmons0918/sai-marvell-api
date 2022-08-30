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
* @file snetFalconTxQ.c
*
* @brief Falcon TxQ module processing
*
* @version   1
********************************************************************************
*/

#include <asicSimulation/SKernel/suserframes/snetFalconTxQ.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahEgress.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahEq.h>
#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SLog/simLogInfoTypePacket.h>


/**
* @internal snetHawkTxqPdxPreemptionCheck function
* @endinternal
*
* @brief   Hawk : PDX : check if need to convert TXQ info for preemption ports
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
*
*  COMMENTS :
*/
static GT_VOID snetHawkTxqPdxPreemptionCheck
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    GT_U32  regAddr,regValue;
    GT_U32  regOffset,bitOffset;

    DECLARE_FUNC_NAME(snetHawkTxqPdxPreemptionCheck);

    __LOG_PARAM(descrPtr->egressPhysicalPortInfo.sip6_queue_group_offset);
    __LOG_PARAM(descrPtr->egressPhysicalPortInfo.sip6_queue_group_index);

    /*check if preemption is enabled*/
    regOffset = (descrPtr->egressPhysicalPortInfo.sip6_queue_group_index) >> 5;
    bitOffset = (descrPtr->egressPhysicalPortInfo.sip6_queue_group_index) & 0x1F;
    regAddr = SMEM_SIP6_TXQ_SDQ_PREEMPTION_ENABLE_REG(devObjPtr,regOffset);

    smemRegGet(devObjPtr,regAddr,&regValue);

    if(0 == (regValue & (1<<bitOffset)))
    {
        __LOG(("Preemption is NOT enabled for queue_group_index [%d] \n",
            descrPtr->egressPhysicalPortInfo.sip6_queue_group_index));

        return;/* no more to do */
    }

    if(descrPtr->egressPhysicalPortInfo.sip6_queue_group_offset & (1<<3))
    {
        __LOG(("Preemption is enabled for queue_group_index [%d] and considered 'preemptive traffic' !!! \n",
            descrPtr->egressPhysicalPortInfo.sip6_queue_group_index));

        __LOG(("Preemption changes sip6_queue_group_index from [%d] to [%d] ,sip6_queue_group_offset from [%d] to [%d] \n",
        descrPtr->egressPhysicalPortInfo.sip6_queue_group_index,        /*before*/
        descrPtr->egressPhysicalPortInfo.sip6_queue_group_index | 0x1,  /*after*/

        descrPtr->egressPhysicalPortInfo.sip6_queue_group_offset,      /*before*/
        descrPtr->egressPhysicalPortInfo.sip6_queue_group_offset & 0x7 /*after*/
        ));

        descrPtr->egressPhysicalPortInfo.sip6_queue_group_index |= 0x1;

        /* Remove bit 3 from sip6_queue_group_offset */
        descrPtr->egressPhysicalPortInfo.sip6_queue_group_offset &= 0x7;
    }
    else
    {
        __LOG(("Preemption is enabled for queue_group_index [%d] and considered 'express traffic' \n",
            descrPtr->egressPhysicalPortInfo.sip6_queue_group_index));
    }

    return;
}

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
)
{
    DECLARE_FUNC_NAME(snetFalconTxqPdxQueueGroupMap);
    GT_U32  regAddr,regValue;
    GT_U32  tableInstance = 0;/* The HW choose randomly/stress considerations , that not care the simulation.
                                 because the HW also duplicate the table 0 to the other tables. */
    GT_U32  ingressPipe;

    ingressPipe = smemGetCurrentPipeId(devObjPtr);

    __LOG(("The accessing to the PDX index[%d] 'queue group mapping' is done at 'ingress pipe[%d] (at ingress tile[%d])' ! \n",
        descrPtr->egressPhysicalPortInfo.sip6_queue_group_index,
        ingressPipe,
        ingressPipe/2));

    regAddr = SMEM_SIP6_TXQ_PDX_QUEUE_GROUP_MAP_TBL_MEM(devObjPtr,
        descrPtr->egressPhysicalPortInfo.sip6_queue_group_index,
        tableInstance);

    smemRegGet(devObjPtr,regAddr,&regValue);

    if(SMEM_CHT_IS_SIP6_15_GET(devObjPtr) &&
        devObjPtr->deviceFamily != SKERNEL_HARRIER_FAMILY)/*sip6.20*/
    {
        queueMapInfoPtr->queue_base      = SMEM_U32_GET_FIELD(regValue,0,9);
        queueMapInfoPtr->dp_core_local_trg_port = SMEM_U32_GET_FIELD(regValue,9,6);
        queueMapInfoPtr->queue_pds_index = SMEM_U32_GET_FIELD(regValue,15,3);
        queueMapInfoPtr->queue_pdx_index = SMEM_U32_GET_FIELD(regValue,18,2);
    }
    else
    if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
    {
        queueMapInfoPtr->queue_base      = SMEM_U32_GET_FIELD(regValue,0,8);
        queueMapInfoPtr->dp_core_local_trg_port = SMEM_U32_GET_FIELD(regValue,8,5);
        queueMapInfoPtr->queue_pds_index = SMEM_U32_GET_FIELD(regValue,13,3);
        queueMapInfoPtr->queue_pdx_index = SMEM_U32_GET_FIELD(regValue,16,2);
    }
    else
    {
        queueMapInfoPtr->queue_base      = SMEM_U32_GET_FIELD(regValue,0,9);
        queueMapInfoPtr->dp_core_local_trg_port = SMEM_U32_GET_FIELD(regValue,9,4);
        queueMapInfoPtr->queue_pds_index = SMEM_U32_GET_FIELD(regValue,13,3);
        queueMapInfoPtr->queue_pdx_index = SMEM_U32_GET_FIELD(regValue,16,2);
    }

    __LOG_PARAM(queueMapInfoPtr->queue_base);
    __LOG_PARAM(queueMapInfoPtr->dp_core_local_trg_port);
    __LOG_PARAM(queueMapInfoPtr->queue_pds_index);
    __LOG_PARAM(queueMapInfoPtr->queue_pdx_index);

    return;
}

/**
* @internal snetFalconTxqToTxDmaCheck function
* @endinternal
*
* @brief   Falcon : check if the TxDma allow to get descriptors from the TXQ.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
* @param[in] localDpInPipe            - local DP in the pipe
* @param[in] globalDpInTile           - DP in the tile
*
*   return:
*   GT_TRUE  - the TXDMA     ask to get descriptors from the TXQ
*   GT_FALSE - the TXDMA NOT ask to get descriptors from the TXQ (descriptor should be dropped)
*/
static GT_BOOL snetFalconTxqToTxDmaCheck
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32                 localDpInPipe,
    IN GT_U32                 globalDp
)
{
    DECLARE_FUNC_NAME(snetFalconTxqToTxDmaCheck);

    GT_U32  regAddr;
    GT_U32  enable_desc_request;

    regAddr = SMEM_SIP6_TXDMA_CUT_TXDMA_GLOBAL_CONFIG_REG_MAC(devObjPtr,localDpInPipe);
    smemRegFldGet(devObjPtr, regAddr, 0, 1, &enable_desc_request);
    if(enable_desc_request == 0)
    {
        __LOG(("CONFIGURATION ERROR : The TxDMA[%d] (global DP)  is not allow to get descriptors from the TXQ (field enable_desc_request = 0)\n",
            globalDp));
        descrPtr->haAction.drop = SKERNEL_EXT_PKT_CMD_HARD_DROP_E;

        return GT_FALSE;
    }

    __LOG(("The TxDMA[%d] (global DP) allow to get descriptors from the TXQ (field enable_desc_request = 1)\n",
        globalDp));

    return GT_TRUE;
}


/**
* @internal snetFalconTxqSdqEgressDmaGet function
* @endinternal
*
* @brief   Falcon : SDQ : get the local DMA number that serves this traffic.
*         (local DMA number 0..8 for the TxFifo,TxDMA to use)
*         NOTE: the 'DP[index]' is the same as SDQ[index].
*         the 'Global DMA' is saved into : descrPtr->egressPhysicalPortInfo.txDmaMacPort.
*/
static GT_VOID snetFalconTxqSdqEgressDmaGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN QUEUE_GROUP_MAP_ENTRY_STC   *queueMapInfoPtr
)
{
    DECLARE_FUNC_NAME(snetFalconTxqSdqEgressDmaGet);

    GT_U32  sdqIndex = queueMapInfoPtr->queue_pds_index,orig_sdqIndex;
    GT_U32  regAddr,regValue;
    GT_U32  minValue,maxValue;
    GT_U32  numOfPorts;
    GT_U32  localDma , globalDma;
    GT_U32  sdqLocalQueue;
    GT_U32  perPipe_sdqIndex;
    GT_U32  global_pipeId;
    GT_U32  egressTileId;
    GT_U32  mirroredSdqIndex;
    GT_U32 dmaEnable;
    GT_U32 globalDp;
    GT_BOOL isCpuSdma;

    /* save the value for later use */
    orig_sdqIndex = sdqIndex;

    /* calc the 'local queue index' of the traffic in this SDQ */
    sdqLocalQueue = queueMapInfoPtr->queue_base + descrPtr->egressPhysicalPortInfo.sip6_queue_group_offset;

    __LOG(("Start : The local sdqIndex[%d] will serve the descriptor (according to queueMapInfoPtr->queue_pds_index) in sdqLocalQueue[%d] \n",
        sdqIndex,sdqLocalQueue));
    __LOG(("sdqLocalQueue[%d] comes from : queueMapInfoPtr->queue_base[%d] + descrPtr->egressPhysicalPortInfo.sip6_queue_group_offset[%d] \n",
        sdqLocalQueue, queueMapInfoPtr->queue_base , descrPtr->egressPhysicalPortInfo.sip6_queue_group_offset));

    numOfPorts = 1/*for the 'cpu port'*/ + devObjPtr->multiDataPath.info[orig_sdqIndex].cpuPortDmaNum;

    egressTileId = queueMapInfoPtr->queue_pdx_index;
    if(egressTileId & 1)/* tile 1,3 are mirrored */
    {
        /* mirrored DP[] */
        mirroredSdqIndex = ((devObjPtr->multiDataPath.maxDp*devObjPtr->numOfPipesPerTile) - /* number of DPs per tile */
                    1) - sdqIndex;

        __LOG(("NOTE: The local sdqIndex[%d] in tile [%d] is considered global DP[%d] in the device (mirrored tile) \n",
            sdqIndex,egressTileId,
            mirroredSdqIndex + egressTileId*(devObjPtr->multiDataPath.maxDp*devObjPtr->numOfPipesPerTile)));

        /* use the mirrored DP[] for pipe calculations and global port calculations */
        sdqIndex = mirroredSdqIndex;
    }
    else
    {
        __LOG(("The local sdqIndex[%d] in tile [%d] is considered global DP[%d] in the device \n",
            sdqIndex,egressTileId,
            sdqIndex + egressTileId*(devObjPtr->multiDataPath.maxDp*devObjPtr->numOfPipesPerTile)));
    }

    global_pipeId = (egressTileId *                   /* Tile number */
                     devObjPtr->numOfPipesPerTile) +  /* number of pipes per Tile*/
                     (sdqIndex / devObjPtr->multiDataPath.maxDp);
    /* calculate the 'DP' index in the pipe */
    perPipe_sdqIndex = sdqIndex % devObjPtr->multiDataPath.maxDp;

    descrPtr->egressPhysicalPortInfo.egressPipeId = global_pipeId;

    /* set new PIPE id for the device */
    /* the 'restore' comes before calling snetChtErepProcess */
    smemSetCurrentPipeId(devObjPtr,descrPtr->egressPhysicalPortInfo.egressPipeId);

    globalDp = perPipe_sdqIndex+global_pipeId*devObjPtr->multiDataPath.maxDp;
    /* check if the TXDMA ready for the TXQ to get descriptors */
    if(GT_FALSE == snetFalconTxqToTxDmaCheck(devObjPtr,descrPtr,perPipe_sdqIndex,globalDp))
    {
        /* the TxDMA not allow to get descriptors for the TXQ*/
        return;
    }


    __LOG(("The accessing to the rest of TXQ is done at 'egress tile[%d]' , global pipe[%d] ! (from queue_pdx_index)\n",
        egressTileId,global_pipeId));

    for(localDma = 0 ; localDma < numOfPorts; localDma++)
    {

        regAddr = SMEM_SIP6_TXQ_SDQ_GLOBAL_PORT_ENABLE_REG(devObjPtr,orig_sdqIndex,localDma);
        smemRegFldGet(devObjPtr,regAddr,0,1,&dmaEnable);

        if(0 == dmaEnable)
        {
            __LOG(("localDma[%d]: is DISABLED \n",
                localDma));

            /* jump to check next DMA */
            continue;
        }

        regAddr = SMEM_SIP6_TXQ_SDQ_PORT_CONFIG_PORT_RANGE_LOW_REG (devObjPtr,orig_sdqIndex,localDma);
        smemRegGet(devObjPtr,regAddr,&regValue);
        minValue = SMEM_U32_GET_FIELD(regValue,0,9);
        regAddr = SMEM_SIP6_TXQ_SDQ_PORT_CONFIG_PORT_RANGE_HIGH_REG(devObjPtr,orig_sdqIndex,localDma);
        smemRegGet(devObjPtr,regAddr,&regValue);
        maxValue = SMEM_U32_GET_FIELD(regValue,0,9);

        if(sdqLocalQueue >= minValue && sdqLocalQueue <= maxValue)
        {
            __LOG(("localDma[%d]:The SDQ queue Index sdqLocalQueue[%d] part of range [%d to %d] , that refer to local DMA[%d] \n",
                localDma,sdqLocalQueue , minValue,maxValue, localDma));

            break;
        }

        __LOG(("localDma[%d]:The SDQ queue Index sdqLocalQueue[%d] not part of range [%d to %d] \n",
            localDma,sdqLocalQueue , minValue,maxValue));
    }

    if(localDma == numOfPorts)
    {
        /* not found proper range */
        __LOG(("Configuration ERROR : The SDQ queue Index sdqLocalQueue[%d] not part of ANY range --> DROP the packet \n",
            sdqLocalQueue));

        descrPtr->haAction.drop = SKERNEL_EXT_PKT_CMD_HARD_DROP_E;

        return;
    }

    if(localDma == devObjPtr->multiDataPath.info[orig_sdqIndex].cpuPortDmaNum)
    {
        isCpuSdma = smemMultiDpUnitIsTxFifoEnableSdmaCpuPortGet(devObjPtr,globalDp);
        if(isCpuSdma == GT_TRUE)
        {
            __LOG(("the 'CPU port' in global DP[%d] is SDMA to MG unit \n",
                globalDp));
        }
        else
        {
            __LOG(("the 'CPU port' in global DP[%d] is Network port to the Raven \n",
                globalDp));
        }
    }
    else
    {
        isCpuSdma = GT_FALSE;
    }



    /* for LOG info convert localDMA to globalDMA */
    smemConvertPipeIdAndLocalPortToGlobal_withPipeAndDpIndex(
        devObjPtr,
        global_pipeId,
        perPipe_sdqIndex,/* the SDQ index is also the DP index */
        localDma,
        isCpuSdma,
        &globalDma);

    __LOG(("End : In egress tile [%d] The local sdqIndex[%d] , sdqLocalQueue[%d] mapped to local DMA[%d] (global DMA[%d])\n",
        egressTileId,
        queueMapInfoPtr->queue_pds_index,/* the sdqIndex before 'mirror' issues
            for pipe calculations and global port calculations */
        sdqLocalQueue,
        localDma,
        globalDma));

    /* notify the counters that the packet is in the TXQ */
    smemFalconTxqQfcCounterTable1Update(devObjPtr,globalDma,GT_TRUE/* increment*/);
    descrPtr->egressPhysicalPortInfo.sip6_txq_couneter_was_incrmented = 1;

    descrPtr->egressPhysicalPortInfo.txDmaMacPort = globalDma;
    __LOG_PARAM(descrPtr->egressPhysicalPortInfo.txDmaMacPort);

    return;
}

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
)
{
    QUEUE_GROUP_MAP_ENTRY_STC queueMapInfo;

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_TXQ_E);


    if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
    {
        /* check if port is preemptive and change :
        descrPtr->egressPhysicalPortInfo.sip6_queue_group_index,
        descrPtr->egressPhysicalPortInfo.sip6_queue_group_offset
        */
        snetHawkTxqPdxPreemptionCheck(devObjPtr,descrPtr);
    }

    /* map <sip6_queue_group_index> to PDX info */
    snetFalconTxqPdxQueueGroupMap(devObjPtr,descrPtr,&queueMapInfo);

    /* get the DMA to egress the packet */
    snetFalconTxqSdqEgressDmaGet(devObjPtr,descrPtr,&queueMapInfo);

    return;
}


