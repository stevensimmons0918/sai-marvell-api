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
* @file snetFalconEgress.c
*
* @brief Falcon Egress processing
*
* @version   1
********************************************************************************
*/

#include <asicSimulation/SKernel/suserframes/snetFalconEgress.h>
#include <asicSimulation/SKernel/suserframes/snetFalconTxQ.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahEgress.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahEq.h>
#include <asicSimulation/SKernel/suserframes/snetXCat.h>
#include <asicSimulation/SKernel/suserframes/snetXCatPolicer.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahIngress.h>
#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SLog/simLogInfoTypePacket.h>

/**
* @internal snetChtErepProcessEgressMirror function
* @endinternal
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - target port.
*                                       COMMENTS:
*
* @note COMMENTS: base on 'snetChtTxQSniffAndStcDuplicate'
*
*/
static GT_VOID  snetChtErepProcessEgressMirror
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32               egressPort,
    IN GT_U32               allowDropsToMirror
)
{
    DECLARE_FUNC_NAME(snetChtErepProcessEgressMirror);

    SKERNEL_FRAME_CHEETAH_DESCR_STC * txSniffDescPtr;/* mirror descriptor */
    GT_U32  origPipeId;/* support multi-pipe device */

    if (descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_SOFT_DROP_E ||
        descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_HARD_DROP_E)
    {
        /* Packet expected to drop , but hold indication of 'mirror-to-analyzer' */
        if(allowDropsToMirror)
        {
            __LOG(("Packet expected to soft/hard drop , but hold indication of 'mirror-to-analyzer' : allowed to mirror \n"));
        }
        else
        {
            if(!SMEM_CHT_IS_SIP6_10_GET(devObjPtr)) /* only in sip6 (fixed in 6.10) */
            {
                /* EREP-21 : Dropped packets may be egress mirrored to CPU or Analyzer */
                __LOG(("WARNING : Due to Erratum : Packet expected to 'hard/soft drop' , but hold indication of 'mirror-to-analyzer' : allowed to mirror \n"));
            }
            else
            {
                __LOG(("Packet expected to soft/hard drop , but hold indication of 'mirror-to-analyzer' : NOT allowed to mirror \n"));
                return;
            }
        }
    }


    /* duplicate descriptor from the ingress core */
    txSniffDescPtr = snetChtEqDuplicateDescr(descrPtr->ingressDevObjPtr,descrPtr);

    /* set it as 'forward' ... we not want to keep it 'soft/hard drop' or other */
    txSniffDescPtr->packetCmd = SKERNEL_EXT_PKT_CMD_FORWARD_E;

    txSniffDescPtr->trgTagged = descrPtr->egressPhysicalPortInfo.egressVlanTagMode ;

    txSniffDescPtr->egressTrgPort = egressPort;/* this is global port */

    if(txSniffDescPtr->useVidx ||
       txSniffDescPtr->trgDev != descrPtr->ownDev)
    {
        /* save the target phy port of the orig packet used by HA for access tables 'phy port 1' ' eport 1' */
        txSniffDescPtr->eArchExtInfo.srcTrgEPort = egressPort;
    }
    else
    {
        /* save the target eport of the orig packet used by HA for access tables 'phy port 1' ' eport 1' */
        txSniffDescPtr->eArchExtInfo.srcTrgEPort = descrPtr->trgEPort;
    }

    txSniffDescPtr->txqToEq = 1;
    txSniffDescPtr->origDescrPtr = descrPtr;

    /* since TXQ send back to the EQ we need to set descriptor 'local/global src ports' accordingly */
    {
        GT_U32  localSrcPort = txSniffDescPtr->localPortGroupPortAsGlobalDevicePort;/*value of 'local port' (at this stage in TXQ) */
        GT_U32  globalSrcPort = txSniffDescPtr->localDevSrcPort;/* the global port num (at this stage in TXQ) */

        /* convert ingress local src port to src egress global port */
        txSniffDescPtr->localDevSrcPort   = localSrcPort;

        if(txSniffDescPtr->origIsTrunk == 0 && txSniffDescPtr->marvellTagged == 0)
        {
            txSniffDescPtr->origSrcEPortOrTrnk                = localSrcPort;
            txSniffDescPtr->localPortTrunkAsGlobalPortTrunk   =  globalSrcPort;
        }

        txSniffDescPtr->localPortGroupPortAsGlobalDevicePort = globalSrcPort;/* global port(not local) */
    }

    __LOG(("The EREP unit send the descriptor to the EQ \n"));

    /* support multi-pipe device */
    origPipeId = smemGetCurrentPipeId(devObjPtr);
    if(devObjPtr->numOfPipes)
    {
        __LOG(("The EREP unit send the 'Tx mirror' descriptor to the EQ of pipe[%d] according to INGRESS pipe \n",
            origPipeId));
    }

    /* l3StartOffsetPtr will be updated in chtHaUnit */
    txSniffDescPtr->haToEpclInfo.l3StartOffsetPtr = NULL;

    if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
    {
        /* Copy Original packet queue info to the mirrored descriptor */
        txSniffDescPtr->origQueueOffset       = descrPtr->egressPhysicalPortInfo.sip6_queue_group_offset;
        txSniffDescPtr->origQueueGroupIndex   = descrPtr->egressPhysicalPortInfo.sip6_queue_group_index;
        txSniffDescPtr->epclMetadataReady     = 0;  /* Need to rebuild the epcl metadata */
        txSniffDescPtr->localDevPacketSource  = SKERNEL_SRC_TX_REPLICATED_TYPE_E;
    }

    snetChtEqTxMirror (devObjPtr, txSniffDescPtr);

    /* restore pipeId ... as the EQ may send the packet to other egress pipe ! */
    smemSetCurrentPipeId(devObjPtr,origPipeId);

    /* Update TX mirrored flag in packet descriptor */
    __LOG(("Update TX mirrored flag in packet descriptor \n"));
    /*    descrPtr->txMirrorDone = txSniffDescPtr->txMirrorDone;*/
    descrPtr->bmpsEqUnitsGotTxqMirror = txSniffDescPtr->bmpsEqUnitsGotTxqMirror;

}

/**
* @internal snetFalconEgfQagEgressMirror function
* @endinternal
*
* @brief   egress mirroring
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - target port.
*
* @note base on 'snetChtTxQSniffAndStc'
*
*/
static GT_VOID snetFalconEgfQagEgressMirror
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 egressPort
)
{
    DECLARE_FUNC_NAME(snetFalconEgfQagEgressMirror);

    GT_U32  *memPtr;                /* pointer to memory */
    GT_U32  regAddr;                /* address of registers */
    GT_U32  fldVal;                 /* field value */
    GT_U32  value;

    regAddr = SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTE_TBL_MEM(devObjPtr,egressPort);
    memPtr = smemMemGet(devObjPtr, regAddr);

    value = SMEM_SIP5_20_EGF_QAG_PORT_TARGET_FIELD_GET(devObjPtr,memPtr,egressPort,
        SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_TARGET_PHYSICAL_PORT_EVLAN_MIRRORING_ENABLE_E);

    if(0 == value)
    {
        if(descrPtr->analyzerIndex)
        {
            __LOG(("EGF_QAG: the eVlan mirroring is not enabled on egress port[%d] although the vlan analyzer index is[%d] \n",
                egressPort,
                descrPtr->analyzerIndex));
        }

        /* the port is not allowed to send 'eVlan mirroring' */
        descrPtr->analyzerIndex = 0;
    }

    snetChtEgfEgressEPortEntryGet(devObjPtr,descrPtr,egressPort,GT_FALSE);/*global port*/

    if(descrPtr->eArchExtInfo.egfQagEgressEPortTablePtr != NULL)
    {
        /*Mirror To Analyzer Index*/
        fldVal = snetFieldValueGet(descrPtr->eArchExtInfo.egfQagEgressEPortTablePtr, 1 , 3);
        if(fldVal)
        {
            __LOG(("EGF_QAG:  Per EPort table - Mirror To Analyzer Index [%d] \n",
                fldVal));
        }

        snetXcatEgressMirrorAnalyzerIndexSelect(devObjPtr,descrPtr,fldVal);
    }

    if (descrPtr->analyzerIndex && descrPtr->disableErgMirrOnLb)
    {
        descrPtr->analyzerIndex = 0;
    __LOG(("EGF_QAG:  Mirroring is disabled because of 'Disable Egress Mirroring On Loopback'\n"));
    }

    return;
}


/**
* @internal snetFalconEftQagCutThrough function
* @endinternal
*
* @brief   Egress processing cut through support
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
* @param[in] egressPort               - <localDevTargetPort> the egress physical port.
*                                       indication that packet was send to the DMA of the port
*
*/
static GT_VOID snetFalconEftQagCutThrough
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32               egressPort
)
{
    DECLARE_FUNC_NAME(snetFalconEftQagCutThrough);
    GT_U32 ingressSpeed;
    GT_U32 egressSpeed;
    GT_U32 addr;
    GT_U32 *memPtr;
    GT_U32 regValue;
    GT_U32 defaultByteCount;
    GT_U32 cutThroughTerminateEn;

    if (descrPtr->cutThroughModeEnabled == 0)
    {
        /* not needed to terminated cut through mode */
        return;
    }

    /* default byte count caused cut through  terminate */
    addr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.distributor.cutThroughConfig;
    smemRegGet(devObjPtr, addr, &regValue);
    defaultByteCount      = (regValue & 0x3FFF);
    cutThroughTerminateEn = ((regValue >> 14) & 1);
    if ((descrPtr->byteCount == defaultByteCount) && cutThroughTerminateEn)
    {
        __LOG(
            ("EFT_QAG Cut Through - cutThroughTerminated  cutThroughTerminateEn %d defaultByteCount %d",
            cutThroughTerminateEn, defaultByteCount));
        descrPtr->cutThroughModeTerminated = 1;
    }

    /* slow to fast port cut through terminate */
    addr  = SMEM_SIP5_20_EGF_QAG_PORT_SOURCE_ATTRIBUTE_TBL_MEM(devObjPtr, descrPtr->localDevSrcPort);
    memPtr = smemMemGet(devObjPtr, addr);
    ingressSpeed = SMEM_SIP5_20_EGF_QAG_PORT_SOURCE_FIELD_GET(
        devObjPtr, memPtr, descrPtr->localDevSrcPort,
        SMEM_SIP6_EGF_QAG_PORT_SOURCE_ATTRIBUTES_TABLE_FIELDS_SOURCE_PHYSICAL_PORT_SPEED_E);

    addr  = SMEM_LION2_EGF_QAG_TARGET_PORT_MAPPER_TBL_MEM(devObjPtr, egressPort);
    memPtr = smemMemGet(devObjPtr, addr);
    egressSpeed = SMEM_SIP6_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_FIELD_GET(
        devObjPtr, memPtr, egressPort,
        SMEM_SIP6_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_PORT_SPEED_E);

    if ((egressSpeed > ingressSpeed) && descrPtr->cutThroughModeEnabled)
    {
        __LOG(
            ("EFT_QAG Cut Through - cutThroughTerminated  ingressSpeed %d  egressSpeed %d",
            ingressSpeed, egressSpeed));
        descrPtr->cutThroughModeTerminated = 1;
    }
}

/**
* @internal snetFalconEgfQag function
* @endinternal
*
* @brief   Falcon : TXQ-QAG processing.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
* @param[in] egressPort               - <localDevTargetPort> the egress physical port.
*/
static GT_VOID snetFalconEgfQag
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32               egressPort
)
{
    DECLARE_FUNC_NAME(snetFalconEgfQag);
    GT_U32  egrTagMode;

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_EGF_EFT_SHT_QAG_E);

    snetSip5_20EgfQagTcDpRemap(devObjPtr,descrPtr,egressPort);
    /* check for TXQ VID reassignment */
    snetChtEgfQagVidAssignment(devObjPtr, descrPtr, egressPort);

    /* the egress vlan tag mode is determined by different table then
       the 'vlan table' that used for 'vlan attributes and  members' !!*/
    egrTagMode =
        snetChtEgfQagVlanTagModeGet(devObjPtr, descrPtr,egressPort);

    __LOG(("egrTagMode [%s] (egress tag mode) \n",
        (egrTagMode == SKERNEL_FRAME_TR101_VLAN_EGR_TAG_UNTAGGED_E        )  ? "UNTAGGED        "  :
        (egrTagMode == SKERNEL_FRAME_TR101_VLAN_EGR_TAG_TAG0_E            )  ? "TAG0            "  :
        (egrTagMode == SKERNEL_FRAME_TR101_VLAN_EGR_TAG_TAG1_E            )  ? "TAG1            "  :
        (egrTagMode == SKERNEL_FRAME_TR101_VLAN_EGR_TAG_OUT_TAG0_IN_TAG1_E)  ? "OUT_TAG0_IN_TAG1"  :
        (egrTagMode == SKERNEL_FRAME_TR101_VLAN_EGR_TAG_OUT_TAG1_IN_TAG0_E)  ? "OUT_TAG1_IN_TAG0"  :
        (egrTagMode == SKERNEL_FRAME_TR101_VLAN_EGR_TAG_PUSH_TAG0_E       )  ? "PUSH_TAG0       "  :
        (egrTagMode == SKERNEL_FRAME_TR101_VLAN_EGR_TAG_POP_OUT_TAG_E     )  ? "POP_OUT_TAG     "  :
        /* --> this case already converted from 'no modify' to actual state of the SRC tagging */
        /* (egrTagMode == SKERNEL_FRAME_TR101_VLAN_EGR_TAG_DO_NOT_MODIFIED_E )  ? "DO_NOT_MODIFIED "  :*/
        "unknown"
    ));

    descrPtr->egressPhysicalPortInfo.egressVlanTagMode = egrTagMode;

    snetFalconEgfQagEgressMirror(devObjPtr,descrPtr,egressPort);

    /* cut through support */
    snetFalconEftQagCutThrough(devObjPtr, descrPtr, egressPort);
}

/**
* @internal snetFalconLmuPortTrigger function
* @endinternal
*
* @brief  Check LMU port-based triggering check.
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr             - pointer to frame data buffer Id
* @param[in] egressPort               - local egress port (not global) - (global since sip5).
*
*/
static GT_VOID snetFalconLmuPortTrigger
(
    IN SKERNEL_DEVICE_OBJECT                * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr,
    IN  GT_U32                                egressPort
)
{
    DECLARE_FUNC_NAME(snetFalconLmuPortTrigger);

    GT_U32                          localDevSrcPort;
    GT_U32                          localDevTrgPort;
    GT_U32                          srcIdx;
    GT_U32                          dstIdx;
    GT_U32                          srcProfile;
    GT_U32                          dstProfile;
    GT_U32                          memAddr;
    GT_U32                          memValue;
    GT_U32                          entryIndex;
    GT_U32                          portProfile = 0;
    GT_U32                          queueProfile = 0;
    GT_BIT                          profileMode = 0;

    if(descrPtr->lmuEn == GT_TRUE)
    {
        return;
    }

    localDevSrcPort = descrPtr->localDevSrcPort;
    localDevTrgPort = egressPort;

    memAddr = SMEM_SIP6_EPCL_SRC_PHYSICAL_PORT_MAP_TBL_MEM(devObjPtr,
        localDevSrcPort);
    smemRegGet(devObjPtr, memAddr, &memValue);
    srcIdx = SMEM_U32_GET_FIELD(memValue, 0, 8);
    srcProfile = SMEM_U32_GET_FIELD(memValue, 8, 9);

    memAddr = SMEM_SIP6_EPCL_TRG_PHYSICAL_PORT_MAP_TBL_MEM(devObjPtr,
        localDevTrgPort);
    smemRegGet(devObjPtr, memAddr, &memValue);
    dstIdx = SMEM_U32_GET_FIELD(memValue, 0, 8);
    dstProfile = SMEM_U32_GET_FIELD(memValue, 8, 9);

    if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
    {
        memAddr = SMEM_CHT3_EPCL_GLOBAL_REG(devObjPtr);
        smemRegFldGet(devObjPtr, memAddr, 17, 1, &profileMode);
        __LOG(("Latency monitoring profile mode %s\n",
              (profileMode == 0)?"PORT MODE":
              (profileMode == 1)?"QUEUE MODE":
                                 "UNKNOWN"));
        /* Queue Mode */
        if(profileMode == 1)
        {
            memAddr = SMEM_SIP6_10_EPCL_QUEUE_OFFSET_LATENCY_PROFILE_CONFIG_REG(devObjPtr, descrPtr->egressPhysicalPortInfo.sip6_queue_group_offset >> 1);
            smemRegFldGet(devObjPtr, memAddr, (descrPtr->egressPhysicalPortInfo.sip6_queue_group_offset % 2) *9 , 9, &queueProfile);
            __LOG(("Queue profile is %d\n", queueProfile));
            __LOG_PARAM(queueProfile);

            memAddr = SMEM_SIP6_10_EPCL_QUEUE_GROUP_LATENCY_PROFILE_CONFIG_TBL_MEM(devObjPtr,
                                      descrPtr->egressPhysicalPortInfo.sip6_queue_group_index);
            smemRegGet(devObjPtr, memAddr, &portProfile);
            __LOG(("Port profile is %d\n", portProfile));
            __LOG_PARAM(portProfile);
        }
    }

    entryIndex = (dstIdx * 256 + srcIdx);
    memAddr = SMEM_SIP6_EPCL_PORT_LATENCY_MONITORING_TBL_MEM(devObjPtr, entryIndex >> 5);

    smemRegGet(devObjPtr, memAddr, &memValue);
    __LOG(("Port Latency Monitoring En Table [%d][%d] = %d\n", srcIdx, dstIdx,
        SMEM_U32_GET_FIELD(memValue, entryIndex & 0x1F, 1)));
    if(SMEM_U32_GET_FIELD(memValue, entryIndex & 0x1F, 1) == 1)
    {
        descrPtr->lmuEn = GT_TRUE;
        if((SMEM_CHT_IS_SIP6_10_GET(devObjPtr) && (profileMode == 1)))
        {
            descrPtr->lmuProfile = (portProfile + queueProfile) & 0x1FF;
        }
        else
        {
            descrPtr->lmuProfile = (srcProfile + dstProfile) & 0x1FF;
        }
        __LOG(("descrPtr->lmuEn = %d\ndescrPtr->lmuProfile = %d\n",
            descrPtr->lmuEn, descrPtr->lmuProfile));
    }
}

/**
* @internal snetFalconLmuSampling function
* @endinternal
*
* @brief  Check LMU port-based triggering check.
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr             - pointer to frame data buffer Id
*
*/
static GT_VOID snetFalconLmuSampling
(
    IN SKERNEL_DEVICE_OBJECT                * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr
)
{
    DECLARE_FUNC_NAME(snetFalconLmuSampling);

    GT_U32                          samplingProfile;
    GT_U32                          samplingThresh;
    GT_U32                          samplingMode;
    GT_U32                          rndVal;
    GT_U32                          regAddr;
    GT_U32                          regVal;
    GT_U32                          packetNumber;

    if(GT_FALSE == descrPtr->lmuEn)
    {
        return;
    }

    regAddr = SMEM_SIP6_EPCL_LM_PROFILE2SAMPLING_PROFILE_REG(devObjPtr,
        (descrPtr->lmuProfile >> 3));
    smemRegGet(devObjPtr, regAddr, &regVal);
    samplingProfile = SMEM_U32_GET_FIELD(regVal, 3 * (descrPtr->lmuProfile & 0x7), 3);
    __LOG(("LMU Sampling profile is %d\n", samplingProfile));

    regAddr = SMEM_SIP6_EPCL_LM_SAMPLING_CFG_REG(devObjPtr, samplingProfile);
    smemRegGet(devObjPtr, regAddr, &samplingThresh);
    __LOG(("LMU Sampling threshold is %d\n", samplingThresh));
    __LOG_PARAM(samplingThresh);

    regAddr = SMEM_SIP6_EPCL_LM_CONTROL_REG(devObjPtr);
    smemRegFldGet(devObjPtr, regAddr, samplingProfile, 1, &samplingMode);
    __LOG(("LMU Sampling mode is %s\n",
        (samplingMode == 0)?"DETERMINISTIC":
        (samplingMode == 1)?"RANDOM":
                            "UNKNOWN"));

    if(samplingMode == 0)
    {
        regAddr = SMEM_SIP6_EPCL_LM_SAMPLING_COUNTER_REG(devObjPtr, samplingProfile);
        smemRegGet(devObjPtr, regAddr, &packetNumber);
        __LOG_PARAM(packetNumber);
        __LOG_PARAM((1 << samplingThresh));
        if(packetNumber % (1 << samplingThresh))
        {
            descrPtr->lmuEn = GT_FALSE;
            __LOG(("LMU sampling engine excluded the packet from latency measurement."
                "Only every %dth packet sampled\ndescrPtr->lmuEn = %d",
                (1 << samplingThresh), descrPtr->lmuEn));
        }
        else
        {
            __LOG(("continue with LMU sampling \n"));
        }
        smemRegSet(devObjPtr, regAddr, packetNumber + 1);
    }
    else if(samplingMode == 1)
    {
        srand(descrPtr->frameId ^ rand() ^ descrPtr->trgEPort ^ descrPtr->packetTimestamp);
        rndVal = (((GT_U32)rand() & 0x7FFF) /* 15 bits */ |
                   ((GT_U32)rand() << 15)) & 0x3FFFFFFF;/* using total 30 bits */

        __LOG_PARAM(rndVal);
        if(rndVal < samplingThresh)
        {
            descrPtr->lmuEn = GT_FALSE;
            __LOG(("LMU sampling engine excluded the packet from latency measurement."
                " %lu < %lu\n descrPtr->lmuEn = %d \n",
                rndVal, samplingThresh, descrPtr->lmuEn));
        }
        else
        {
            __LOG(("continue with LMU sampling \n"));
        }
    }
    else
    {
        skernelFatalError("Latency monitoring: impossible sampling mode");
    }
}

/**
* @internal snetFalconEgressControlPipe function
* @endinternal
*
* @brief   Falcon : do Control pipe Egress processing .
*         QAG --> HA --> EPCL --> PRE-Q --> EOAM --> EPLR --> ERMRK --> PHA --> EREP --> DP(RxDma).
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
* @param[in] egressPort               - <localDevTargetPort> the egress physical port.
*
* @retval is Packet dropped        - GT_TRUE : packet was   dropped.
*                                       GT_FALSE : packet was not dropped.
*                                       COMMENTS :
*/
static GT_BOOL snetFalconEgressControlPipe
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32               egressPort,
    OUT GT_U8               **frameDataPtrPtr,
    OUT GT_U32              *frameDataSizePtr
)
{
    DECLARE_FUNC_NAME(snetFalconEgressControlPipe);
    GT_U32 haFrameDataSize;
    GT_U32 fixedFrameDataSize;
    GT_U32 copiedFrameDataSize;
    GT_U32 trgPort = snetFalconPhaCalculateTargetPort(devObjPtr, descrPtr, egressPort);

    /* save the <localDevTargetPort> the egress physical port */
    descrPtr->egressPhysicalPortInfo.localPortIndex  = egressPort;
    descrPtr->egressPhysicalPortInfo.globalPortIndex = egressPort;

    SIM_LOG_PACKET_DESCR_SAVE
    snetFalconEgfQag(devObjPtr, descrPtr, egressPort);
    SIM_LOG_PACKET_DESCR_COMPARE("snetFalconEgfQag");

    /* Pure Cut Through or Reduced Latency Store and Forward mode */
    /* Cut through support                                                       */
    /* Cut through termination should be treated after all egress pipe subunits  */
    /* But HA bulds the updated packet and the Trapped or mirrored packet copy   */
    /* using descrPtr->payloadLength calculated based on descrPtr->origByteCount */
    /* But all cases of packet building in HA will cause fatal error or crash    */
    /* if descrPtr->origByteCount == 0x3FFF                                      */


    if (descrPtr->cutThroughModeEnabled)
    {
        if (descrPtr->cutThrough2StoreAndForward || descrPtr->cutThroughModeTerminated)
        {
            /* adjust descrPtr->payloadLength to real packet size */
            if ((descrPtr->payloadLength + descrPtr->frameBuf->actualDataSize)
                < descrPtr->origByteCount)
            {
                skernelFatalError(
                    "snetFalconEgressControlPipe : payloadLength 0x%X  origByteCount 0x%X cutThroughModeEnabled %d\n",
                    descrPtr->payloadLength, descrPtr->origByteCount, descrPtr->cutThroughModeEnabled);
            }
            descrPtr->payloadLength =
                descrPtr->payloadLength + descrPtr->frameBuf->actualDataSize
                - descrPtr->origByteCount;

            /* restore descrPtr->origByteCount */
            descrPtr->origByteCount  = descrPtr->frameBuf->actualDataSize;
            descrPtr->byteCount      = descrPtr->frameBuf->actualDataSize;
        }
    }

    /* Any case the packet building in HA will cause fatal error  */
    /* if descrPtr->origByteCount > 12K                           */
    if (descrPtr->origByteCount > SBUF_DATA_SIZE_CNS)
    {
        __LOG(("snetFalconEgressControlPipe - target port[%d] origByteCount 0x%X\n",
            egressPort, descrPtr->origByteCount));
        __LOG(("snetFalconEgressControlPipe - Not supported state\n"));
        __LOG(("snetFalconEgressControlPipe - DROPPING packet\n"));
        descrPtr->haAction.drop = SKERNEL_EXT_PKT_CMD_HARD_DROP_E;
        return GT_TRUE;
    }

    __LOG(("do HA unit - target port[%d]\n",
        egressPort));

    SIM_LOG_PACKET_DESCR_SAVE
    snetChtHaMain(devObjPtr, descrPtr, egressPort, (GT_U8)descrPtr->egressPhysicalPortInfo.egressVlanTagMode,
                       frameDataPtrPtr, frameDataSizePtr, &descrPtr->isCpuUseSdma);
    SIM_LOG_PACKET_DESCR_COMPARE("snetChtHaMain");
    if(descrPtr->haAction.drop == SKERNEL_EXT_PKT_CMD_HARD_DROP_E)
    {
        __LOG(("WARNING : packet is dropped by HA - target port[%d]\n",
            egressPort));

        return GT_TRUE;
    }

    __LOG(("do EPCL unit - target port[%d]\n",
        egressPort));

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_EPCL_E);
    SIM_LOG_PACKET_DESCR_SAVE
    snetXCatEPcl(devObjPtr, descrPtr, egressPort);

    snetFalconLmuPortTrigger(devObjPtr, descrPtr, egressPort);
    snetFalconLmuSampling(devObjPtr, descrPtr);

    SIM_LOG_PACKET_DESCR_COMPARE("snetXCatEPcl");

    if(descrPtr->epclAction.drop == SKERNEL_EXT_PKT_CMD_HARD_DROP_E)
    {
        __LOG(("WARNING : packet is dropped by EPCL - target port[%d]\n",
            egressPort));

        descrPtr->packetCmd = snetChtPktCmdResolution(descrPtr->packetCmd,
            SKERNEL_EXT_PKT_CMD_HARD_DROP_E);
        /*return GT_TRUE;*/
    }

    SIM_LOG_PACKET_DESCR_SAVE
    snetPreqProcess(devObjPtr, descrPtr, egressPort);
    SIM_LOG_PACKET_DESCR_COMPARE("snetPreqProcess");

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_EPLR_E);
    /* do EPLR after EPCL */
    SIM_LOG_PACKET_DESCR_SAVE
    snetXCatEgressPolicer(devObjPtr,descrPtr,egressPort);
    /* reset the field as it hold no meaning outside the POLICER and it makes 'diff' for the LOGGER */
    descrPtr->policerCycle = 0;
    SIM_LOG_PACKET_DESCR_COMPARE("snetChtEplrProcess");

    if(descrPtr->eplrAction.drop == SKERNEL_EXT_PKT_CMD_HARD_DROP_E)
    {
        __LOG(("WARNING : packet is dropped by EPLR - target port[%d]\n",
            egressPort));

        descrPtr->packetCmd = snetChtPktCmdResolution(descrPtr->packetCmd,
            SKERNEL_EXT_PKT_CMD_HARD_DROP_E);
        /*return GT_TRUE;*/
    }

    /* do egress final packet modifications (after EPCL and EPLR) */
    SIM_LOG_PACKET_DESCR_SAVE
    snetChtEgressFinalPacketAlteration(devObjPtr, descrPtr, egressPort);
    SIM_LOG_PACKET_DESCR_COMPARE("snetChtEgressFinalPacketAlteration");

    if(descrPtr->haAction.drop == SKERNEL_EXT_PKT_CMD_HARD_DROP_E)
    {
        __LOG(("WARNING : packet is dropped by due to PTP checks - target port[%d]\n",
            egressPort));

        descrPtr->packetCmd = snetChtPktCmdResolution(descrPtr->packetCmd,
            SKERNEL_EXT_PKT_CMD_HARD_DROP_E);
        /*return GT_TRUE;*/
    }


    if(SMAIN_NOT_VALID_CNS != UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_PHA))
    {
        /* do PHA after the ERMRK unit  */
        SIM_LOG_PACKET_DESCR_SAVE
        snetFalconPhaProcess(devObjPtr, descrPtr, trgPort);
        SIM_LOG_PACKET_DESCR_COMPARE("snetFalconPhaProcess");
    }
    else
    {
        __LOG(("NOTE : PHA unit not exists in the device \n"));
    }

    if(descrPtr->pha.pha_fw_bc_modification)
    {
        /* updated by the PHA */
        (*frameDataSizePtr) += descrPtr->pha.pha_fw_bc_modification;
        __LOG(("PHA thread changed the Packet length by [%d] bytes \n",
            descrPtr->pha.pha_fw_bc_modification));
    }

    if(descrPtr->haAction.drop == SKERNEL_EXT_PKT_CMD_HARD_DROP_E)
    {
        __LOG(("WARNING : packet is dropped by due to PHA unit checks - target port[%d]\n",
            egressPort));

        descrPtr->packetCmd = snetChtPktCmdResolution(descrPtr->packetCmd,
            SKERNEL_EXT_PKT_CMD_HARD_DROP_E);
        /*return GT_TRUE;*/
    }


    if(descrPtr->tables_read_error)
    {
        __LOG(("'tables_read_error' ERROR detected : packet is dropped - target port[%d]\n"
               "check for the 'violated' table previously in the LOG \n",
            egressPort));

        descrPtr->packetCmd = snetChtPktCmdResolution(descrPtr->packetCmd,
            SKERNEL_EXT_PKT_CMD_HARD_DROP_E);
        /*return GT_TRUE;*/
    }

    /* additional cutThroughModeTerminated treatment for case */
    /* when Cut Through terminated by EPCL                    */
    if (descrPtr->cutThroughModeEnabled
        && (descrPtr->cutThrough2StoreAndForward || descrPtr->cutThroughModeTerminated))
    {
        haFrameDataSize = *frameDataSizePtr;

        if ((haFrameDataSize + descrPtr->frameBuf->actualDataSize)
            < descrPtr->origByteCount)
        {
            skernelFatalError(
                "snetFalconEgressControlPipe : haFrameDataSize 0x%X  origByteCount actualDataSize %d\n",
                haFrameDataSize, descrPtr->origByteCount, descrPtr->frameBuf->actualDataSize);
        }
        fixedFrameDataSize =
            ((haFrameDataSize + descrPtr->frameBuf->actualDataSize)
            - descrPtr->origByteCount);
        *frameDataSizePtr = fixedFrameDataSize;
        if (fixedFrameDataSize > haFrameDataSize)
        {
            /* copy data not copied by HA */
            copiedFrameDataSize = (fixedFrameDataSize - haFrameDataSize);
            memcpy(
                (devObjPtr->egressBuffer + haFrameDataSize),
                (descrPtr->frameBuf->actualDataPtr
                    + (descrPtr->frameBuf->actualDataSize - copiedFrameDataSize)),
                copiedFrameDataSize);
        }

        /* restore descrPtr->origByteCount */
        descrPtr->origByteCount  = descrPtr->frameBuf->actualDataSize;
        descrPtr->byteCount      = descrPtr->frameBuf->actualDataSize;
    }

    /* update RX_DMA debug CP_TO_RC packet counter */
    snetSip6ChtRxPortDebugCpToRxCounterUpdate(devObjPtr, descrPtr);

    return GT_FALSE;
}



/**
* @internal sip6TxDmaPizzaIsPortConfig function
* @endinternal
*
* @brief   Falcon : Get port slices number in TxDma Pizza.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] txFifoPort                - global dma port number
*
* @retval is port config               - GT_TRUE : port have at
*                                       least one slices.
*                                       GT_FALSE : port don't
*                                       have slices
*/
static GT_BOOL sip6TxDmaPizzaIsPortConfig
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32  txFifoPort
)
{

    DECLARE_FUNC_NAME(sip6TxDmaPizzaIsPortConfig);
    GT_U32 regAddr;                     /* Register's address */
    GT_U32 fieldVal;
    GT_U32 channelSlicesNum = 0;
    GT_U32 sliceIndex;
    GT_U32 localPort;

    if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
    {
        __LOG(("NOTE: not implemented yet simulation check for sip6_10 TXD,TxFifo Pizza \n"));
        return GT_TRUE;
    }

    localPort = SMEM_DATA_PATH_RELATIVE_PORT_GET(devObjPtr,txFifoPort);

    if (GT_TRUE == smemMultiDpUnitIsCpuPortGet(devObjPtr,txFifoPort))
    {
        regAddr = TXDMA_PIZZA_SLICE_PER_DP_REG_MAC(devObjPtr,txFifoPort,32/*cpu slice index*/);
        smemRegFldGet(devObjPtr, regAddr, 31, 1, &fieldVal);
        if (fieldVal == 0)
        {
            __LOG(("WARNING : the port is cpu port, pizza on TxDMA cpu slice is not valid for target port[%d]\n",txFifoPort));
            /* the slice is not valid*/
            return GT_FALSE;
        }
        smemRegFldGet(devObjPtr, regAddr, 0, 4, &fieldVal);
        if (fieldVal == localPort)
        {
            __LOG(("the port is cpu port, with valid and good config for pizza on TxDMA for target port[%d]\n",txFifoPort));
           return GT_TRUE;
        }
        __LOG(("WARNING : the port is cpu port [%d], pizza on TxDMA slice 32 is valid ,but the slice value [%d] is not as expected[%d](according to local port number) \n",
           txFifoPort,fieldVal,localPort));
        return GT_FALSE;
    }
    /*txDMA*/
    for (sliceIndex = 0 ; sliceIndex < 32 ; sliceIndex++ )
    {
        regAddr = TXDMA_PIZZA_SLICE_PER_DP_REG_MAC(devObjPtr,txFifoPort,sliceIndex);
        smemRegFldGet(devObjPtr, regAddr, 31, 1, &fieldVal);
        if (fieldVal == 0)
        {
            /* the slice is not valid*/
            continue;
        }
        smemRegFldGet(devObjPtr, regAddr, 0, 4, &fieldVal);

        if (fieldVal == localPort )
        {
            channelSlicesNum++;
        }
    }

    if (channelSlicesNum > 0)
    {
        __LOG(("pizza on TxDMA have %d slices for target port[%d]\n",channelSlicesNum,txFifoPort));
        return GT_TRUE;
    }

    __LOG(("WARNING : pizza on TxDMA have NO slices for target port[%d]\n",txFifoPort));
    return GT_FALSE;
}

/**
* @internal sip6TxFifoPizzaIsPortConfig function
* @endinternal
*
* @brief   Falcon : Get port slices number in TxFifo Pizza.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] txFifoPort                - global dma port number
*
* @retval is port config               - GT_TRUE : port have at
*                                       least one slices.
*                                       GT_FALSE : port don't
*                                       have slices
*/
static GT_BOOL sip6TxFifoPizzaIsPortConfig
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32  txFifoPort
)
{

    DECLARE_FUNC_NAME(sip6TxFifoPizzaIsPortConfig);
    GT_U32 regAddr;                     /* Register's address */
    GT_U32 fieldVal;
    GT_U32 channelSlicesNum = 0;
    GT_U32 sliceIndex;
    GT_U32 localPort;

    if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
    {
        __LOG(("NOTE: not implemented yet simulation check for sip6_10 TXD,TxFifo Pizza \n"));
        return GT_TRUE;
    }

    localPort = SMEM_DATA_PATH_RELATIVE_PORT_GET(devObjPtr,txFifoPort);

    if (GT_TRUE == smemMultiDpUnitIsCpuPortGet(devObjPtr,txFifoPort))
    {
        regAddr = TXFIFO_PIZZA_SLICE_PER_DP_REG_MAC(devObjPtr,txFifoPort,32/*cpu slice index*/);
        smemRegFldGet(devObjPtr, regAddr, 31, 1, &fieldVal);
        if (fieldVal == 0)
        {
            __LOG(("WARNING : the port is cpu port, pizza on TxFifo cpu slice is not valid for target port[%d]\n",txFifoPort));
            /* the slice is not valid*/
            return GT_FALSE;
        }
        smemRegFldGet(devObjPtr, regAddr, 0, 4, &fieldVal);
        if (fieldVal == localPort)
        {
            __LOG(("the port is cpu port, with valid and good config for pizza on TxFifo for target port[%d]\n",txFifoPort));
           return GT_TRUE;
        }
        __LOG(("WARNING : the port is cpu port [%d], pizza on TxFifo slice 32 is valid ,but the slice value [%d] is not as expected[%d](according to local port number) \n",
           txFifoPort,fieldVal,localPort));
        return GT_FALSE;
    }
    /*dmaNum = SMEM_DUAL_UNIT_DMA_UNIT_INDEX_GET(devObjPtr,txFifoPort);*/
    /*printf("in sip6TxFifoPizzaIsPortConfig, txFifoPort %d, dma %d, port %d\n",txFifoPort, dmaNum,localPort);*/
    for (sliceIndex = 0 ; sliceIndex < 32 ; sliceIndex++ )
    {
        regAddr = TXFIFO_PIZZA_SLICE_PER_DP_REG_MAC(devObjPtr,txFifoPort,sliceIndex);
        smemRegFldGet(devObjPtr, regAddr, 31, 1, &fieldVal);
        if (fieldVal == 0)
        {
            /* the slice is not valid*/
            continue;
        }
        smemRegFldGet(devObjPtr, regAddr, 0, 4, &fieldVal);

        if (fieldVal == localPort )
        {
            channelSlicesNum++;
        }
    }

    if (channelSlicesNum > 0)
    {
        __LOG(("pizza on TxFifo have %d slices for target port[%d]\n",channelSlicesNum,txFifoPort));
        return GT_TRUE;
    }

    __LOG(("WARNING : pizza on TxFifo have NO slices for target port[%d]\n",txFifoPort));
    return GT_FALSE;
}


GT_VOID dummyPrintTxFifoBaseAddress
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32 ii, port,dp;
    GT_U32 regAddr;
    GT_U32  numOfPipes = devObjPtr->numOfPipes ? devObjPtr->numOfPipes : 1;
    printf("\n\n");
    for ( ii = 0 ; ii <(devObjPtr->multiDataPath.maxDp * numOfPipes); ii++ )
    {
        port = ((8*ii)+1);
        regAddr = TXFIFO_PIZZA_SLICE_PER_DP_REG_MAC(devObjPtr,port,0);
        dp =  SMEM_DUAL_UNIT_DMA_UNIT_INDEX_GET(devObjPtr,port);

        printf("port %d in want DP %d , have DP %d , first slice address is  [0x%8.8x]\n",port, ii,dp, regAddr );
    }
}

/**
* @internal sip6MacD2dPcsCalIsPortConfig function
* @endinternal
*
* @brief   Falcon : Get port slices number in D2D PCS calendar
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] txFifoPort                - global dma port number.
* @param[in] IsIngress                 - Egress or Ingress.
* @param[in] IsEagle                    -Eagle or Raven
*
* @retval is port config               - GT_TRUE : port have at
*                                       least one slices.
*                                       GT_FALSE : port don't
*                                       have slices
*/
static GT_BOOL sip6MacD2dPcsCalIsPortConfig
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32  txFifoPort,
    IN GT_BOOL isIngress,
    IN GT_BOOL isEagle
)
{

    DECLARE_FUNC_NAME(sip6MacD2dPcsCalIsPortConfig);

    GT_U32 registerIndex;
    GT_U32 pcsCalLength = 85;
    GT_U32 pcsCalRegNum ;
    GT_U32 pcsSliceOffset = 6;
    GT_U32 localPort;
    GT_U32 regAddr;
    GT_U32 regVal;
    GT_U32 sliceIndex;
    GT_U32 pcsSliceInRegNum = 4;
    GT_U32 sliceValue;
    GT_U32 pcsSliceMask = 0x3F;
    GT_U32 channelSlicesNum = 0;
    GT_BOOL isCpuPort;

    pcsCalRegNum = (pcsCalLength/pcsSliceInRegNum) + (pcsCalLength%pcsSliceInRegNum);

    localPort = SMEM_DATA_PATH_RELATIVE_PORT_GET(devObjPtr,txFifoPort);

    isCpuPort = smemMultiDpUnitIsCpuPortGet(devObjPtr,txFifoPort);

    if((isCpuPort == GT_TRUE) && (isEagle == GT_FALSE))
    {
        localPort = 0x10;
    }

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT___ALLOW_ALL_UNITS___E);/* wild card for LOG without the unit filter */
    for ( registerIndex = 0 ; registerIndex < pcsCalRegNum ; registerIndex++ )
    {
        regAddr = D2D_PCS_CAL_REG_INDEX_MAC(devObjPtr,txFifoPort,registerIndex, isIngress,isEagle);
        smemRegFldGet(devObjPtr, regAddr, 0, 24, &regVal);
        for (sliceIndex = 0 ; sliceIndex < pcsSliceInRegNum ; sliceIndex++)
        {
            sliceValue = (regVal & pcsSliceMask);
            if (sliceValue == localPort)
            {
                channelSlicesNum++;
            }

            if (((registerIndex*4) + sliceIndex) == (pcsCalLength -1) )
            {
                /* last slice*/
                break;
            }

            regVal = (regVal >> pcsSliceOffset);
         }
    }

    __LOG(("PCS calendar have %d slices for target port[%d]\n",channelSlicesNum,txFifoPort));
    if (channelSlicesNum > 0)
    {
        return GT_TRUE;
    }
    return GT_FALSE;

}

/**
* @internal sip6MacD2dMacCalIsPortConfig function
* @endinternal
*
* @brief   Falcon : Get port slices number in D2D MAC calendar
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] txFifoPort                - global dma port number.
* @param[in] isEagle                   -  Eagle Raven
*
* @retval is port config               - GT_TRUE : port have at
*                                       least one slices.
*                                       GT_FALSE : port don't
*                                       have slices
*/
static GT_BOOL sip6MacD2dMacCalIsPortConfig
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32  txFifoPort,
    IN GT_BOOL isEagle
)
{

    DECLARE_FUNC_NAME(sip6MacD2dEagleMacCalIsPortConfig);

    GT_U32 regAddr;        /* Register's address */
    GT_U32 regVal;      /* register value*/
    GT_U32 macSlicesInRegister = 4;
    GT_U32 macCalendarRegistersNum = 40; /*160/4*/
    GT_U32 registerIndex, sliceIndex;
    GT_U32 regMaskSliceValid, regMaskSliceValue;
    GT_U32 regSlicesValid, regSlicesValues;
    GT_U32 sliceValid, sliceValue;
    GT_BOOL isCpuPort;

    GT_U32 macCalSliceOffset = 8;
    GT_U32 localPort;
    GT_U32 channelSlicesNum = 0;

    GT_U32 macSliceValidMask = 0x80;
    GT_U32 macSliceValueMask = 0x3F;

    localPort = SMEM_DATA_PATH_RELATIVE_PORT_GET(devObjPtr,txFifoPort);

    isCpuPort = smemMultiDpUnitIsCpuPortGet(devObjPtr,txFifoPort);

    if((isCpuPort == GT_TRUE) && (isEagle == GT_FALSE))
    {
        localPort = 0x10;
    }

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT___ALLOW_ALL_UNITS___E);/* wild card for LOG without the unit filter */
    /* define mask for slices valid bits */
    regMaskSliceValid = (macSliceValidMask | (macSliceValidMask << macCalSliceOffset) | (macSliceValidMask << (2* macCalSliceOffset)) | (macSliceValidMask << (3* macCalSliceOffset)));
    /* define mask for slices values */
    regMaskSliceValue = (macSliceValueMask | (macSliceValueMask << macCalSliceOffset) | (macSliceValueMask << (2*macCalSliceOffset)) | (macSliceValueMask << (3*macCalSliceOffset)));

    /* go over MAC calendar slices. each register hold 4 slices. search for local port index*/
    for (registerIndex = 0 ; registerIndex < macCalendarRegistersNum ; registerIndex ++ )
    {
        /*regAddr = baseAddr + D2D_MAC_CFG_RX_TDM_SCHEDULE + (registerIndex * HWS_D2D_MAC_CHANNEL_STEP);*/
        regAddr = D2D_MAC_CAL_REG_INDEX_MAC(devObjPtr,txFifoPort,registerIndex,isEagle);
        smemRegFldGet(devObjPtr, regAddr, 0, 32, &regVal);

        regSlicesValid = (regMaskSliceValid & regVal); /* valid slices values*/
        regSlicesValues = (regMaskSliceValue & regVal); /* slices data values */

        for (sliceIndex = 0 ; sliceIndex < macSlicesInRegister ; sliceIndex ++)
        {
            sliceValid = (regSlicesValid & macSliceValidMask);
            sliceValue = (regSlicesValues & macSliceValueMask);
            if ((sliceValid == macSliceValidMask)&&(sliceValue == localPort))
            {
                channelSlicesNum++;
            }
            /* get next slice in the register*/
            regSlicesValid = (regSlicesValid >> macCalSliceOffset);
            regSlicesValues = (regSlicesValues >> macCalSliceOffset);
         }
     }

    __LOG(("MAC calendar have %d slices for target port[%d]\n",channelSlicesNum,txFifoPort));
    if (channelSlicesNum > 0)
    {
        return GT_TRUE;
    }
    return GT_FALSE;
}

/**
* @internal snetFalconUpdateTxDmaDebugCounters function
* @endinternal
*
* @brief   Falcon : do TxDma debug counting.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] txFifoPort                - global dma port number.
* @param[in] pizzaIsOk                - GT_TRUE : port have at least one slices.
*                                       GT_FALSE : port don't have any slices.
*/
static void snetFalconUpdateTxDmaDebugCounters(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  txFifoPort,
    IN GT_BOOL                 pizzaIsOk
)
{
    DECLARE_FUNC_NAME(snetFalconUpdateTxDmaDebugCounters);

    GT_U32 regAddr , value;
    GT_U32 allowedPort;
    GT_U32 txDmaUnit;
    GT_U32 localDma;

    pizzaIsOk = pizzaIsOk;/* I don't know how it may impact the debug counting */

    txDmaUnit = SMEM_DUAL_UNIT_DMA_UNIT_INDEX_GET(devObjPtr,txFifoPort);
    localDma  = SMEM_DATA_PATH_RELATIVE_PORT_GET (devObjPtr,txFifoPort);

    regAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[txDmaUnit].debug.enable_debug_Counters;
    smemRegGet(devObjPtr,regAddr, &value);
    if(0 == (value & 1))
    {
        __LOG(("The debug counters are disabled (not counting) at unit[%d] \n",
            txDmaUnit));
        return;
    }

    regAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[txDmaUnit].debug.txd_debug_config;
    smemRegGet(devObjPtr,regAddr, &value);

    if((value & 1) == 1)/* count all*/
    {
        __LOG(("The debug counters at unit[%d] count 'all ports' \n",
            txDmaUnit));
    }
    else
    {
        allowedPort = (value >> 4) & 0xF;
        __LOG(("The debug counters at unit[%d] count only specific port [%d] \n",
            txDmaUnit, allowedPort));

        if(allowedPort != localDma)
        {
            __LOG(("The debug counters at unit[%d] count only specific port [%d] that is not for current port [%d] \n",
                txDmaUnit, allowedPort));

            return;
        }
    }

     /* here is list of debug counters to be updated ... */

    regAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_txDMA[txDmaUnit].TXD_Status.global_status.events_counters.pds_desc_counter;

    /*
        the register holds 2 counters : bits 0..15 and 16..31 .
        each of them should be incremented
    */

    smemRegGet(devObjPtr,regAddr, &value);

    __LOG(("unit[%d] <PDS Real Descriptors Counter> and <PDS Total Descriptors Counter> incremented by 1 from [%d] \n",
        txDmaUnit , (value & 0xFFFF)));

    value += 0x00010001;
    smemRegSet(devObjPtr,regAddr, value);


    return;
}

/**
* @internal sip6TxDmaAndTxFifoPizzaIsChannelIdle function
* @endinternal
*
* @brief   Falcon : validate port configuration on eagle Tx DMA
*          and TxFIFO pizza.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] txFifoPort                - global dma port number.
*
* @retval is port config               - GT_TRUE : port have at
*                                       least one slices.
*                                       GT_FALSE : port don't
*                                       have slices
*/
static GT_BOOL sip6TxDmaAndTxFifoPizzaIsChannelIdle
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32               txFifoPort
)
{
    DECLARE_FUNC_NAME(sip6TxDmaAndTxFifoPizzaIsChannelIdle);

    GT_BOOL retVal = GT_TRUE;
    GT_BOOL isPortTxFifoConfig;/* indication that port config on TxFifo pizza */
    GT_BOOL isPortTxDmaConfig;/* indication that port config on TxDma pizza */

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT___ALLOW_ALL_UNITS___E);/* wild card for LOG without the unit filter */
     __LOG(("Start checking TX,TXFifo 'pizza' configurations for egress global DMA port[%d]\n",
            txFifoPort));
    isPortTxFifoConfig = sip6TxFifoPizzaIsPortConfig( devObjPtr,txFifoPort);
    if(isPortTxFifoConfig == GT_FALSE)
    {
        __LOG(("Tx FIFO pizza arbiter did not configured to support target port [%d]\n",txFifoPort));
        retVal = GT_FALSE;
    }
    isPortTxDmaConfig = sip6TxDmaPizzaIsPortConfig(devObjPtr, txFifoPort);
    if (isPortTxDmaConfig == GT_FALSE)
    {
        __LOG(("Tx DMA pizza arbiter did not configured to support target port [%d]\n",txFifoPort));
        retVal = GT_FALSE;
    }

    /* do counting in the TxDma */
    snetFalconUpdateTxDmaDebugCounters(devObjPtr,txFifoPort,isPortTxDmaConfig);


    __LOG(("Checking 'pizza' configurations for egress global DMA port[%d] - [%s] \n",
            txFifoPort,
            (retVal==GT_TRUE) ?
                "PASSED":
                "FAILED"));

    return retVal;
}


/**
* @internal sip6MacD2dRavenIsPortConfig function
* @endinternal
*
* @brief   Falcon : validate port configuration on raven D2D.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] txFifoPort                - global dma port number.
* @param[in] isIngress                - Egress or Ingress
*
* @retval is port config               - GT_TRUE : port have at
*                                       least one slices.
*                                       GT_FALSE : port don't
*                                       have slices
*/
GT_BOOL sip6MacD2dRavenIsPortConfig
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                txFifoPort,
    IN GT_BOOL               isIngress
)
{
    DECLARE_FUNC_NAME(sip6MacD2dRavenIsPortConfig);

    /*
    this function checks that the target port have gop calendars slices on Raven D2D.
    Ingress:
        Tx pcs calendar

    Egress:
        Rx mac calendr
        Rx pcs calendar
       */

    GT_BOOL retVal = GT_TRUE;
    GT_BOOL isPcsCalConfig = GT_FALSE; /*indication that the port have at least one slice on the PCS calendar */
    GT_BOOL isMacCalConfig = GT_FALSE;/* indication that the port have at least one slice on MAC calendar */

    __LOG(("check D2D calendars on Raven\n"));

    /* check PCS calendar*/
    isPcsCalConfig = sip6MacD2dPcsCalIsPortConfig( devObjPtr,txFifoPort,(!isIngress)/* raven direction is opsite to eagle*/, GT_FALSE /*isEagle*/);
    if(isPcsCalConfig == GT_FALSE)
    {
        retVal = GT_FALSE;
    }

    /*check Mac calendar*/
   if (isIngress == GT_FALSE )
   {
        isMacCalConfig = sip6MacD2dMacCalIsPortConfig(devObjPtr, txFifoPort,GT_FALSE /*isEagle*/);
        if(isMacCalConfig == GT_FALSE)
        {
            retVal = GT_FALSE;
        }
    }

    return retVal;
}

/**
* @internal sip6MacD2dEagleIsPortConfig function
* @endinternal
*
* @brief   Falcon : validate port configuration on eagle D2D.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] txFifoPort                - global dma port number.
* @param[in] isIngress                - Egress or Ingress
*
* @retval is port config               - GT_TRUE : port have at
*                                       least one slices.
*                                       GT_FALSE : port don't
*                                       have slices
*/
GT_BOOL sip6MacD2dEagleIsPortConfig
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                txFifoPort,
    IN GT_BOOL               isIngress
)
{
    DECLARE_FUNC_NAME(sip6MacD2dEagleIsPortConfig);

    /*
    this function checks that the target port have gop calendars slices on Eagle D2D.
    Ingress:
        Rx mac calendr
        Rx pcs calendar

    Egress:
        Tx pcs calendar
       */

    GT_BOOL retVal = GT_TRUE;
    GT_BOOL isPcsCalConfig = GT_FALSE; /*indication that the port have at least one slice on the PCS calendar */
    GT_BOOL isMacCalConfig = GT_FALSE;/* indication that the port have at least one slice on MAC calendar */

    __LOG(("check D2D calendars on Eagle\n"));

    /* check PCS calendar*/
    isPcsCalConfig = sip6MacD2dPcsCalIsPortConfig( devObjPtr,txFifoPort,isIngress,GT_TRUE /*isEagle*/);
    if(isPcsCalConfig == GT_FALSE)
    {
        retVal = GT_FALSE;
    }

    /*check Mac calendar*/
    if (GT_TRUE == isIngress)
    {
        isMacCalConfig = sip6MacD2dMacCalIsPortConfig(devObjPtr, txFifoPort,GT_TRUE /*isEagle*/);
        if(isMacCalConfig == GT_FALSE)
        {
            retVal = GT_FALSE;
        }
    }

    return retVal;
}

/**
* @internal sip6MACEgressIsChannelIdle function
* @endinternal
*
* @brief   Falcon : validate port configuration on eagle D2D and
*          Raven D2D.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] txFifoPort               - global dma port number.
* @param[out] isD2dTxInLoopbackModePtr   - (pointer to) indication that the D2D is
*                                        in loopback mode.
*
* @retval is port config               - GT_TRUE : port have at least one slices.
*                                       GT_FALSE : port don't have slices
*/
static GT_BOOL sip6MACEgressIsChannelIdle
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32               txFifoPort,
    OUT GT_BOOL             *isD2dTxInLoopbackModePtr
)
{
    DECLARE_FUNC_NAME(sip6MACEgressIsChannelIdle);

    GT_U32  regAddr,regVal;
    /*
    this function checks that the target port have gop calendars slices.
    the port connected to the gop through 2 D2D units.
    In Egress direction, the relevant calendars are :
    D2D Raven side:
        Rx mac calendr
        Rx pcs calendar

    D2D Eagle side
        Tx pcs calendar
       */

    GT_BOOL retVal = GT_TRUE;
    GT_BOOL isPortD2dEagleConfig;/* indication that port config on Eagle */
    GT_BOOL isPortD2dRavenConfig; /*indication that port config on Raven */

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT___ALLOW_ALL_UNITS___E);/* wild card for LOG without the unit filter */
    /* D2D on eagle side*/

    *isD2dTxInLoopbackModePtr = GT_FALSE;

    isPortD2dEagleConfig = sip6MacD2dEagleIsPortConfig( devObjPtr,txFifoPort,GT_FALSE/* EGRESS*/);
    if(isPortD2dEagleConfig == GT_FALSE)
    {
        __LOG(("miss port configurations on d2d Eagle side, egress - traget port[%d] \n",txFifoPort));
        retVal = GT_FALSE;
    }

    /* check eagle side for Tx to Rx loopback */
    regAddr = SMEM_SIP6_D2D_EAGLE_PCS_GLOBAL_CONTROL_REG(devObjPtr,txFifoPort);
    /* <PCS_LBK_ENA> - Enable near-end loopback (from Tx to RX loopback) */
    smemRegFldGet(devObjPtr, regAddr, 31, 1, &regVal);
    if(regVal)
    {
        __LOG(("WARNING : PCS_LBK_ENA = 1 : meaning the D2D in eagle side is in loopback mode (Tx to Rx) \n"));
        __LOG(("WARNING : so packet will not continue to the MAC layer , but will continue to ingress processing in the 'D2D' \n"));

        *isD2dTxInLoopbackModePtr = GT_TRUE;

        /* not continue to the D2D in Raven */
        return retVal;
    }

    /* D2D on raven side*/
    isPortD2dRavenConfig = sip6MacD2dRavenIsPortConfig( devObjPtr,txFifoPort,GT_FALSE/* EGRESS*/);
    if(isPortD2dRavenConfig == GT_FALSE)
    {
        __LOG(("miss port configurations on d2d Raven side, egress - traget port[%d] \n",txFifoPort));
        retVal = GT_FALSE;
    }

    /* check Raven side for Rx to Tx loopback */
    regAddr = SMEM_SIP6_D2D_RAVEN_PMA_OPERATION_CONTROL_INDEX_MAC(devObjPtr,txFifoPort);
    smemRegFldGet(devObjPtr, regAddr, 0, 4, &regVal);
    if((regVal & 0xd) == 0x9)/* code match mvHwsD2dPmaLaneTestLoopbackSet(...) */
    {
        __LOG(("WARNING : PMA_TX_SEL = 2 'Far-end loopback' : meaning the D2D in Raven side is in loopback mode (Rx to Tx) \n"));
        __LOG(("WARNING : so packet will not continue to the MAC layer , but will continue to ingress processing in the 'D2D' \n"));

        *isD2dTxInLoopbackModePtr = GT_TRUE;
    }


    return retVal;
}

/**
* @internal snetEgressD2dLoopbackTxToRx_ingressSide function
* @endinternal
*
* @brief  Do the ingress part (after the Tx to Rx loopback on D2D of Eagle)
*         do the actual ingress packet processing.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] dataPtr                  - (pointer to) start of info put by function snetEgressD2dLoopbackTxToRx_egressSide
* @param[in] dataLength               - length of info put by function snetEgressD2dLoopbackTxToRx_egressSide
* @param[in] frameDataPtr             - pointer to frame data
* @param[in] frameDataSize            - frame data size
*                                       COMMENTS :
*                                       based on function egressTxFifoAndMac
*
*
*/
static void snetEgressD2dLoopbackTxToRx_ingressSide(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U8                 * dataPtr,
    IN GT_U32                  dataLength
)
{
    DECLARE_FUNC_NAME(snetEgressD2dLoopbackTxToRx_ingressSide);

    SBUF_BUF_ID     bufferId;
    GT_U32          ii;
    IN GT_U32       rxPort;
    IN GT_U32       frameDataSize;
    IN GT_U8        *startOfPacketPtr;
    GT_U32          summaryDataLength = 0;
    GT_U32          currDataLength;
    GT_U32          startByte;
    GT_U32          pchEnabled;

    /* Allow to print to the LOG as if regular packet */
    SIM_OS_MAC(simOsTaskOwnTaskPurposeSet)(
        SIM_OS_TASK_PURPOSE_TYPE_PP_PIPE_PROCESSING_DAEMON_E,NULL);

    /* parse the data from the buffer of the message */

    /* get parameter 1 */
    currDataLength = sizeof(rxPort);
    memcpy(&rxPort,dataPtr,currDataLength);
    dataPtr           += currDataLength;
    summaryDataLength += currDataLength;


    /* get parameter 2 */
    currDataLength = sizeof(frameDataSize);
    memcpy(&frameDataSize,dataPtr,currDataLength);
    dataPtr           += currDataLength;
    summaryDataLength += currDataLength;

    summaryDataLength += frameDataSize;

    if(summaryDataLength != dataLength)
    {
        skernelFatalError(" summaryDataLength[%d] != dataLength[%d] (expected equal) \n",
            summaryDataLength,
            dataLength);
    }

    startOfPacketPtr = dataPtr;

    __LOG(("start processing ingress packet that will bypass the ingress MAC processing of port [%d] packet length[%d] \n",
        rxPort,
        frameDataSize));

    smemRegFldGet(devObjPtr,
        SMEM_SIP6_RXDMA_CHANNEL_PCH_CONFIG_REG(devObjPtr,rxPort),
        0,1,&pchEnabled);
    if(pchEnabled)
    {
        /* the egress was with 1 byte PCH , but ingress is with 8 bytes PCH */
        /* so need to drop 7 bytes of PCH ! (first bytes of the packet) */
        startByte = 7;
        frameDataSize -= startByte;

        __LOG(("The egress was with 1 byte PCH , but ingress is with 8 bytes PCH \n"
               "so need to drop 7 bytes of PCH ! (first bytes of the packet) \n"
               "so ingress packet considered [%d] bytes  \n",
               frameDataSize));
    }
    else
    {
        startByte = 0;
    }


    /* get parameter 3 (the ingress packet) */
    for(ii = 0 ; ii < frameDataSize ; ii++)
    {
        startOfPacketPtr[ii] = dataPtr[startByte + ii];
    }

    /* find the Buffer that hold the info */
    bufferId = sbufGetBufIdByData(devObjPtr->bufPool,startOfPacketPtr);

    if(bufferId == NULL)
    {
        simWarningPrintf(" sbufGetBufIdByData : FAILED to find Buffer \n");
        return ;
    }

    /* update the buffer pointer and size */
    sbufDataSet(bufferId, startOfPacketPtr, frameDataSize);

    /* do the processing */
    snetChtFrameProcess_bypassRxMacLayer(devObjPtr,bufferId,rxPort);

    return;
}

/**
* @internal snetEgressD2dLoopbackTxToRx_egressSide function
* @endinternal
*
* @brief  Do Tx to Rx loopback on D2D of Eagle (insert the egress packet as
*         ingress packet)
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
* @param[in] txFifoPort               - egress Port (tx fifo port) - global DMA number.
* @param[in] frameDataPtr             - pointer to frame data
* @param[in] frameDataSize            - frame data size
*                                       COMMENTS :
*                                       based on function egressTxFifoAndMac
*
*
*/
static void snetEgressD2dLoopbackTxToRx_egressSide
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32               txFifoPort,
    IN GT_U8                *frameDataPtr,
    IN GT_U32               frameDataSize
)
{
    DECLARE_FUNC_NAME(snetEgressD2dLoopbackTxToRx_egressSide);

    SBUF_BUF_ID             bufferId;    /* buffer id */
    GT_U32                  messageSize;   /*message size*/
    GT_U8  * dataPtr;         /* pointer to the data in the buffer */
    GT_U32 dataSize;          /* data size */
    GENERIC_MSG_FUNC genFunc = snetEgressD2dLoopbackTxToRx_ingressSide;/* generic function */
    GT_U32          ii;

    messageSize = sizeof(GENERIC_MSG_FUNC)  +
                  sizeof(txFifoPort)        +/* field <txFifoPort> */
                  sizeof(frameDataSize)     +/* field <frameDataSize> */
                  frameDataSize;             /* actual place for the bytes from frameDataPtr */

    /* allocate buffer from the 'destination' device pool */
    /* get the buffer and put it in the queue */
    bufferId = sbufAlloc(devObjPtr->bufPool, messageSize);
    if (bufferId == NULL)
    {
        simWarningPrintf(" snetEgressD2dLoopbackTxToRx_egressSide : no buffers for loopback \n");
        return ;
    }

    __LOG(("ending processing egress packet that will do loopback to ingress D2D processing of port [%d] packet length[%d] \n",
        txFifoPort,
        frameDataSize));

    /* Get actual data pointer */
    sbufDataGet(bufferId, (GT_U8 **)&dataPtr, &dataSize);

    /* put the name of the function into the message */
    memcpy(dataPtr,&genFunc,sizeof(GENERIC_MSG_FUNC));
    dataPtr+=sizeof(GENERIC_MSG_FUNC);

    /* save parameter 1 */
    memcpy(dataPtr,&txFifoPort,sizeof(txFifoPort));
    dataPtr+=sizeof(txFifoPort);

    /* save parameter 2 */
    memcpy(dataPtr,&frameDataSize,sizeof(frameDataSize));
    dataPtr+=sizeof(frameDataSize);

    /* save parameter 3 (the egress packet) */
    for(ii = 0 ; ii < frameDataSize ; ii++)
    {
        dataPtr[ii] = frameDataPtr[ii];
    }

    if(devObjPtr->numOfPipes > 2)
    {
        /* needed in Falcon to identify the 'TileId' */
        bufferId->pipeId = smemGetCurrentPipeId(devObjPtr);
    }

    /* needed in Falcon to identify the 'mgUnitId' */
    bufferId->mgUnitId = smemGetCurrentMgUnitIndex(devObjPtr);

    /* set source type of buffer                    */
    bufferId->srcType = SMAIN_SRC_TYPE_CPU_E;

    /* set message type of buffer                   */
    bufferId->dataType = SMAIN_MSG_TYPE_GENERIC_FUNCTION_E;

    /* put buffer to queue                          */
    squeBufPut(devObjPtr->queueId, SIM_CAST_BUFF(bufferId));
}



/**
* @internal sip6EgressTxFifoAndMac function
* @endinternal
*
* @brief   let the TxFifo and MAC to egress the packet
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
* @param[in] txFifoPort               - egress Port (tx fifo port) - global DMA number.
* @param[in] frameDataPtr             - pointer to frame data
* @param[in] frameDataSize            - frame data size
*                                       COMMENTS :
*                                       based on function egressTxFifoAndMac
*/
static GT_VOID sip6EgressTxFifoAndMac
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32               txFifoPort,
    IN GT_U8                *frameDataPtr,
    IN GT_U32               frameDataSize
)
{
    DECLARE_FUNC_NAME(sip6EgressTxFifoAndMac);

    GT_BOOL isCpuPort;/* indication that CPU will get the packet */
    GT_BOOL isPortTxDmaTxFifoConfig;  /* indication that the port have pizza slices on eagle txdma and txfifo*/
    GT_BOOL isPortMACConfig;  /*indication that the port have D2D calendars slices from the mac side*/
    GT_U32  mgUnitIndex,orig_mgUnitIndex;/* the MG unit that need to serves the CPU port (if CPU port) */
    GT_BOOL isD2dTxInLoopbackMode;   /*(pointer to) indication that the D2D is in loopback mode.*/

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_TXDMA_E);

    if(descrPtr->prpInfo.prpCmd == PRP_CMD_ADD60_E ||
       descrPtr->prpInfo.prpCmd == PRP_CMD_ADD64_E)
    {
        GT_U32  ii;
        GT_U32  orig_frameDataSize = frameDataSize;
        GT_U32  PRP_trailer_size = 6;
        GT_U32  crc_size = 4;

        frameDataSize -= crc_size;/* remove the 4 bytes 'place holder' for the CRC */

        if(descrPtr->prpInfo.prpCmd == PRP_CMD_ADD64_E && frameDataSize < 64)
        {
            GT_U32 diff = 64 - frameDataSize;

            __LOG(("Do Padding to 64 bytes (without CRC)  (68 with CRC)\n"));

            for(ii = 0 ; ii < diff ; ii ++)
            {
                /* enlarge the padding */
                frameDataPtr[frameDataSize + ii] = 0;
            }

            frameDataSize = 64;
        }

        /* add the 6 bytes PRP trailer after the padding (if exists) and before the 4 bytes CRC */
        for(ii = 0 ; ii < PRP_trailer_size ; ii ++)
        {
            /* copy the 6 bytes from the EREP info */
            frameDataPtr[frameDataSize + ii] = descrPtr->prpInfo.prpTrailerBytes[ii];
        }

        frameDataSize += PRP_trailer_size;/* adding the 6 bytes PRP trailer  */
        frameDataSize += crc_size;/* adding the 4 bytes CRC */

        __LOG(("The packet was ADDED with 6 bytes of PRP trailer , the size was [%d] and changed to [%d] \n",
            orig_frameDataSize,
            frameDataSize));

        descrPtr->calcCrc = GT_TRUE;
    }
    else
    if(descrPtr->prpInfo.prpCmd == PRP_CMD_REMOVE_E)
    {
        GT_U32  orig_frameDataSize = frameDataSize;
        GT_U32  PRP_trailer_size = 6;
        GT_U32  crc_size = 4;
        GT_U32  ii;

        frameDataSize -= crc_size;/* remove the 4 bytes 'place holder' for the CRC */

        frameDataSize -= PRP_trailer_size;/* forget about the 6 bytes PRP trailer */

        if(frameDataSize < 60)
        {
            GT_U32 diff = 60 - frameDataSize;

            __LOG(("Do Padding to 60 bytes after removing the 6 bytes PRP trailer (without CRC)  (68 with CRC)\n"));

            for(ii = 0 ; ii < diff ; ii ++)
            {
                /* enlarge the padding */
                frameDataPtr[frameDataSize + ii] = 0;
            }

            frameDataSize = 60;
        }

        frameDataSize += crc_size;/* adding the 4 bytes CRC */

        __LOG(("The packet was ADDED with 6 bytes of PRP trailer , the size was [%d] and changed to [%d] \n",
            orig_frameDataSize,
            frameDataSize));

        descrPtr->calcCrc = GT_TRUE;
    }

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT___ALLOW_ALL_UNITS___E);/* wild card for LOG without the unit filter */

    /*check DMA number for CPU port*/
    isCpuPort = smemMultiDpUnitIsCpuSdmaPortGet(devObjPtr,descrPtr->egressPhysicalPortInfo.txDmaMacPort,&mgUnitIndex);

    if (isCpuPort == GT_TRUE && descrPtr->isCpuUseSdma)/*check DMA number for CPU port*/
    {
        /* support multi-SDMA CPU ports (via multi-MG units) */
        orig_mgUnitIndex = smemGetCurrentMgUnitIndex(devObjPtr);
        /* support multi-SDMA CPU ports (via multi-MG units) */
        smemSetCurrentMgUnitIndex(devObjPtr,mgUnitIndex);

        /* DMA to CPU - DMA descriptors management */
        __LOG(("DMA to CPU - DMA descriptors management - target port[%d]\n",
            txFifoPort));

        /* this is sdma port. check Eagle TxFifo and TxDma pizza. validate that the channel have at least one slice*/
        isPortTxDmaTxFifoConfig = sip6TxDmaAndTxFifoPizzaIsChannelIdle(devObjPtr,txFifoPort);
        if (GT_FALSE == isPortTxDmaTxFifoConfig)
        {
            /* DMA to CPU - CPU port does not have pizza slices on Txdma and TxFifo pizza */
            __LOG(("DMA to CPU  - packet dropped due to calendars miss configurations - target port[%d]\n",
                  txFifoPort));
            return;
        }
        snetChtDma2Cpu(devObjPtr, descrPtr, frameDataPtr,frameDataSize);

        /* restore the current MG  */
        smemSetCurrentMgUnitIndex(devObjPtr,orig_mgUnitIndex);
    }
    else
    { /* Tx MAC Processing */
        if(devObjPtr->numOfRavens &&
            SMAIN_NOT_VALID_CNS == smemRavenGlobalPortToRavenIndexGet(devObjPtr,txFifoPort))
        {
            __LOG(("Not valid target port[%d] - packet dropped due to not 'mac port' (and not 'active' SDMA)\n",
                txFifoPort));
            return ;
        }

        __LOG(("Tx MAC Processing - target port[%d]\n",
            txFifoPort));

        /* check target port Eagle TxFifo and TxDma pizza. validate that the channel have at least one slice*/
        isPortTxDmaTxFifoConfig = sip6TxDmaAndTxFifoPizzaIsChannelIdle(devObjPtr, txFifoPort);
        if (GT_FALSE == isPortTxDmaTxFifoConfig)
        {
            /* Tx DMA -  port does not have pizza slices on Txdma and TxFifo pizza */
            __LOG(("Tx DMA Processing - packet dropped due to calendars miss configurations - target port[%d]\n",
            txFifoPort));
            return;
        }

        if(devObjPtr->numOfRavens)
        {
             /* check target ports D2D egress calendars configuration. for D2D on eagle side and D2D on Raven side
               validate that the channel have at least one slice in each calendar*/
            isPortMACConfig = sip6MACEgressIsChannelIdle(devObjPtr, txFifoPort , &isD2dTxInLoopbackMode);
            if (GT_FALSE == isPortMACConfig)
            {
                __LOG(("Tx MAC Processing - packet dropped due to calendars miss configurations - target port[%d]\n",
                txFifoPort));
                return;
            }
        }
        else
        {
            isD2dTxInLoopbackMode = GT_FALSE;
        }

        if(isD2dTxInLoopbackMode == GT_TRUE)
        {
            snetEgressD2dLoopbackTxToRx_egressSide(devObjPtr,txFifoPort,frameDataPtr,frameDataSize);
        }
        else
        {
            snetChtTx2Port(devObjPtr,descrPtr,txFifoPort,frameDataPtr,frameDataSize);
        }
    }
}

/**
* @internal snetChtErepProcess_prpForDp function
* @endinternal
*
* @brief   EREP - build PRP trailer to be used by the DP.
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*/
static void snetChtErepProcess_prpForDp
(
    IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr
)
{
    DECLARE_FUNC_NAME(snetChtErepProcess_prpForDp);

    GT_U32  _2bytes,prp_suffix;

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_EREP_E);

    if(descrPtr->prpInfo.prpCmd == PRP_CMD_ADD60_E ||
       descrPtr->prpInfo.prpCmd == PRP_CMD_ADD64_E )
    {
        _2bytes  = descrPtr->up1     << 13 |
                   descrPtr->cfidei1 << 12 |
                   descrPtr->vid1;

        descrPtr->prpInfo.prpTrailerBytes[0] = (GT_U8)(_2bytes >> 8);
        descrPtr->prpInfo.prpTrailerBytes[1] = (GT_U8) _2bytes;

        _2bytes = descrPtr->prpInfo.egress_prpPathId << 12 |
                  (descrPtr->prpInfo.egress_prpLSDUSize & 0xFFF);

        descrPtr->prpInfo.prpTrailerBytes[2] = (GT_U8)(_2bytes >> 8);
        descrPtr->prpInfo.prpTrailerBytes[3] = (GT_U8) _2bytes;

        smemRegFldGet(devObjPtr,
            SMEM_SIP6_30_EREP_PRP_GLOBAL_CONFIG_REG(devObjPtr),0,16,
            &prp_suffix);

        _2bytes = prp_suffix;

        descrPtr->prpInfo.prpTrailerBytes[4] = (GT_U8)(_2bytes >> 8);
        descrPtr->prpInfo.prpTrailerBytes[5] = (GT_U8) _2bytes;

        __LOG(("The PRP Trailer 6 bytes are : %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x \n",
            descrPtr->prpInfo.prpTrailerBytes[0],
            descrPtr->prpInfo.prpTrailerBytes[1],
            descrPtr->prpInfo.prpTrailerBytes[2],
            descrPtr->prpInfo.prpTrailerBytes[3],
            descrPtr->prpInfo.prpTrailerBytes[4],
            descrPtr->prpInfo.prpTrailerBytes[5]
            ));
    }

    return;
}
/**
* @internal snetChtErepProcess function
* @endinternal
*
* @brief   EREP - supported on SIP6 devices
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - local egress port (not global).
* @param[in] packetWasDropped         - packet was dropped: 0 - not dropped, 1 - dropped.
*/
static void snetChtErepProcess
(
  IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
  IN SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
  IN GT_U32                          egressPort,
  IN GT_U32                          packetWasDropped
)
{
    DECLARE_FUNC_NAME(snetChtErepProcess);

    GT_BIT  didReplication = 0;
    GT_U32  regAddr   = 0;  /* register's address */
    GT_U32  fieldData = 0;
    GT_U32  ratio = 0;
    GT_U32  *memPtr;                /* pointer to memory */
    GT_BOOL dropCodeLatchEnable = GT_FALSE;
    GT_U32  sstIdLocation;
    /* SIP 6.10 - EREP convert drop packet mode to TRAP or Analyzer */
    GT_U32  dropPacketMode = 0;                     /* Dropped Packet Mode */
    GT_U32  dropPacketModeForEgressReplicated;  /* Drop packet mode for egress replicated paclets */
    GT_U32  dropPacketAnalyzerIndex;            /* Dropped Packet Analyzer Index */

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_EREP_E);

    if(descrPtr->qcnRx)
    {
        __LOG(("SIP6 : EREP :received qcn packet or qcn generated by local device\n"));
        didReplication = 0;
    }
    else
    {
        if(descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E)
        {
            __LOG(("SIP6 : EREP :No replication, TO_ANALYZER packet\n"));
            didReplication = 0;
        }
        else
        {
            if(packetWasDropped)
            {
                /* SIP 6.10 - EREP convert drop packet mode to TRAP or Analyzer */
                if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
                {
                    regAddr = SMEM_SIP6_EREP_GLB_CFG_REG(devObjPtr);
                    smemRegGet(devObjPtr, regAddr, &fieldData);

                    dropPacketModeForEgressReplicated = SMEM_U32_GET_FIELD(fieldData, 7, 1);

                    /* Packet was not replicated (egress mirrored) by the EREP
                       OR
                       Dropped packet can be trapped to CPU or mirrored to Analyzer even if it is an egress replicated packet */
                    if(descrPtr->localDevPacketSource != SKERNEL_SRC_TX_REPLICATED_TYPE_E || dropPacketModeForEgressReplicated)
                    {
                        dropPacketMode = SMEM_U32_GET_FIELD(fieldData, 2, 2);
                        switch(dropPacketMode)
                        {
                            case 1:
                                /* TRAP Mode */
                                descrPtr->packetCmd = SKERNEL_EXT_PKT_CMD_TRAP_TO_CPU_E;
                                descrPtr->packetCmdAssignedByEgress = 1;
                                break;
                            case 2:
                                /* Mirror-to-Analyzer Mode */
                                dropPacketAnalyzerIndex = SMEM_U32_GET_FIELD(fieldData, 4, 3);
                                if (dropPacketAnalyzerIndex > descrPtr->analyzerIndex)
                                {
                                    descrPtr->analyzerIndex = dropPacketAnalyzerIndex;
                                }
                                break;
                            default:
                                break;
                        }
                    }
                }
            }

            if (descrPtr->analyzerIndex)
            {
                __LOG(("SIP6 : EREP : Do egress Mirroring \n"));
                descrPtr->mirrorType = SKERNEL_MIRROR_TYPE_MIRROR_CNS;
                regAddr = SMEM_SIP6_EREP_SNIFF_CNTR_REG(devObjPtr);
                smemRegFldGet(devObjPtr, regAddr, 0, 32, &fieldData);

                if(fieldData == 0xFFFFFFFF)
                {
                    fieldData = 0;
                }
                else
                {
                    fieldData++;
                }
                smemRegFldSet(devObjPtr, regAddr, 0, 32, fieldData);
                __LOG(("Updated TO_Analyzer Packet Counter \n"));

                /* Get the mirroring ration for the analyzerIndex */
                regAddr = SMEM_SIP6_EREP_EGRESS_STATISTIC_MIRRORING_TO_ANALYZER_PORT_RATIO_REG(devObjPtr, descrPtr->analyzerIndex-1);
                smemRegFldGet(devObjPtr, regAddr, 0, 32, &ratio);

                regAddr = SMEM_SIP6_EREP_EGRESS_ANALYZER_CNTR_REG(devObjPtr, descrPtr->analyzerIndex-1);
                smemRegFldGet(devObjPtr, regAddr, 0, 32, &fieldData);

                /* Increment the counter regardless of ratio and mirror statistics */
                if(fieldData == 0xFFFFFFFF)
                {
                    fieldData = 0;
                }
                else
                {
                    fieldData++;
                }
                smemRegFldSet(devObjPtr, regAddr, 0, 32, fieldData);
                __LOG(("Updated Mirroring TO_Analyzer Packet Counter \n"));

                /* Check from the ratio and sniffer counter, if need to be mirrored */
                if((ratio != 0) && (fieldData % ratio) == 0)
                {
                    snetChtErepProcessEgressMirror(devObjPtr,descrPtr,egressPort,
                        dropPacketMode == 0 ? GT_FALSE : GT_TRUE);
                }

                descrPtr->mirrorType = 0;
                descrPtr->analyzerIndex = 0;
                didReplication = 0;
            }

            if (descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_CPU_E)
            {
                __LOG(("SIP6 : EREP : No Replication, TO_CPU packet \n"));
                didReplication = 0;
            }
            else
            {
                switch (descrPtr->packetCmd)
                {
                    case SKERNEL_EXT_PKT_CMD_MIRROR_TO_CPU_E :
                        __LOG(("SIP6 : EREP : Do egress Mirroring to CPU \n"));
                        regAddr = SMEM_SIP6_EREP_MIRROR_CNTR_REG(devObjPtr);
                        didReplication = 1;
                        break;
                    case SKERNEL_EXT_PKT_CMD_TRAP_TO_CPU_E :
                        __LOG(("SIP6 : EREP : Do egress Trap to CPU \n"));
                        regAddr = SMEM_SIP6_EREP_TRAP_CNTR_REG(devObjPtr);
                        didReplication = 1;
                        break;
                    case SKERNEL_EXT_PKT_CMD_HARD_DROP_E :
                    case SKERNEL_EXT_PKT_CMD_SOFT_DROP_E :
                        __LOG(("SIP6 : EREP : Drop \n"));
                        regAddr   = SMEM_SIP6_EREP_EGRESS_PROBE_PACKET_DROP_CODE_CFG_REG(devObjPtr);
                        memPtr    = smemMemGet(devObjPtr, regAddr);

                        dropCodeLatchEnable = snetFieldValueGet(memPtr, 0, 1);
                        sstIdLocation       = snetFieldValueGet(memPtr, 1, 4);

                        if( (dropCodeLatchEnable) && ((descrPtr->sstId) & (1 << sstIdLocation)))
                        {
                            __LOG(("SIP6 : EREP : Egress Probe packet drop code latch \n"));
                            regAddr  = SMEM_SIP6_EREP_EGRESS_PROBE_PACKET_DROP_CODE_REG(devObjPtr);
                            smemRegFldSet(devObjPtr, regAddr, 0, 8, descrPtr->cpuCode);
                        }

                    default:
                        break;
                }
                if (didReplication)
                {
                    smemRegFldGet(devObjPtr, regAddr, 0, 32, &fieldData);

                    if(fieldData == 0xFFFFFFFF)
                    {
                        fieldData = 0;
                    }
                    else
                    {
                        fieldData++;
                    }

                    smemRegFldSet(devObjPtr, regAddr, 0, 32, fieldData);
                    __LOG(("Updated TRAP/Mirror Packet Counter \n"));
                    snetChtEqToCpuFromEgress(devObjPtr, descrPtr);
                }
            }
            /* QCN trigger case to be added here */
        }
    }

    if(!didReplication)
    {
        __LOG(("SIP6 : EREP : nothing to do (no egress mirroring / egress mirror to CPU) \n"));

        /* HBU Packets outgoing Forward Counter counts all not dropped packets */
        if (packetWasDropped == 0)
        {
            regAddr = SMEM_SIP6_EREP_HBU_PACKET_FORWARD_CNTR_REG(devObjPtr);
            smemRegFldGet(devObjPtr, regAddr, 0, 32, &fieldData);

            if(fieldData == 0xFFFFFFFF)
            {
                fieldData = 0;
            }
            else
            {
                fieldData++;
            }

            smemRegFldSet(devObjPtr, regAddr, 0, 32, fieldData);
            __LOG(("Updated HBU Packets outgoing Forward Counter \n"));
        }
    }


    /* Egress Packet Type Pass/Drop CNC Trigger */
    snetCht3CncCount(
        devObjPtr, descrPtr,
        SNET_SIP6_CNC_CLIENT_EREP_PACKET_TYPE_PASS_DROP_E,
        1);
}

/**
* @internal snetFalconEgressProcess function
* @endinternal
*
* @brief   Falcon : do Egress processing .
*         in the Control pipe : QAG --> HA --> EPCL --> PRE-Q --> EOAM --> EPLR --> ERMRK --> PHA --> EREP --> DP(RxDma).
*         in the Data Path  : --> RxDma --> TXQ-PDX --> (egress tile) TX-SDQ --> (TXFIFO,TXDMA) --> MAC.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
* @param[in] egressPort               - <localDevTargetPort> the egress physical port.
* return : indication that packet was send to the DMA of the port
* COMMENTS :
*/
GT_BOOL snetFalconEgressProcess
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32               egressPort
)
{
    DECLARE_FUNC_NAME(snetFalconEgressProcess);

    GT_BOOL wasDropped;
    GT_BOOL wasTrapped = GT_FALSE;
    GT_U32  txFifoPort;
    GT_U8     *frameDataPtr;    /* pointer to frame data */
    GT_U32    frameDataSize;    /* frame data size */
    GT_U32  pipeId;
    GT_U32  packetAllowedToEgressPort = 1;
    SKERNEL_EXT_PACKET_CMD_ENT  orig_packetCmd = descrPtr->packetCmd;

    wasDropped = snetFalconEgressControlPipe(devObjPtr,descrPtr,egressPort,&frameDataPtr,&frameDataSize);
    if(wasDropped == GT_TRUE)
    {
        /* we not send to the TXQ but the EREP unit is expected to do it's logic ...*/
        /* because replication to CPU/analyzer can still happen */
        descrPtr->packetCmd = snetChtPktCmdResolution(descrPtr->packetCmd,
            SKERNEL_EXT_PKT_CMD_HARD_DROP_E);
        descrPtr->packetCmdAssignedByEgress = 1;
    }

    if(orig_packetCmd != descrPtr->packetCmd)
    {
        switch(descrPtr->packetCmd)
        {
            case SKERNEL_EXT_PKT_CMD_SOFT_DROP_E:  /* was dropped just now */
            case SKERNEL_EXT_PKT_CMD_HARD_DROP_E:  /* was dropped just now */

                if(descrPtr->packetCmd       == SKERNEL_EXT_PKT_CMD_SOFT_DROP_E &&
                   descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E)
                {
                    /* ignore drops */
                    __LOG(("The TO_TRG_SNIFFER ignored soft drops , and allowed to egress the analyzer port \n"));
                    wasDropped = GT_FALSE;
                }
                else
                {
                    /* the original will not egress the TXQ , and a drop is handled at the EREP */
                    __LOG(("packet was dropped, by the 'Egress control pipe' and will not egress port [%d] \n",
                        egressPort));
                    wasDropped = GT_TRUE;
                }
                break;
            case SKERNEL_EXT_PKT_CMD_TRAP_TO_CPU_E:/* was trapped just now */
                /* the original will not egress the TXQ , and a trap is handled at the EREP and EQ */
                __LOG(("packet (will be) trapped, by the 'Egress control pipe' and will not egress port [%d] \n",
                    egressPort));
                wasTrapped = GT_TRUE;
                break;
            default :
                break;
        }
    }
    else
    {
        switch(descrPtr->packetCmd)
        {
            case SKERNEL_EXT_PKT_CMD_SOFT_DROP_E:
            case SKERNEL_EXT_PKT_CMD_HARD_DROP_E:
                /* the original will not egress the TXQ , and a drop is handled at the EREP */
                __LOG(("packet was dropped, by the 'Ingress control pipe' and will not egress port [%d] \n",
                    egressPort));
                wasDropped = GT_TRUE;
                break;
            default :
                break;
        }
    }

    if(wasDropped == GT_TRUE)
    {
        /* packet was dropped */
        packetAllowedToEgressPort = 0;
    }
    else
    if(wasTrapped == GT_TRUE)
    {
        /* (orig) packet was dropped */
        packetAllowedToEgressPort = 0;
    }
    else
    {
        pipeId = smemGetCurrentPipeId(devObjPtr);

        descrPtr->egressPhysicalPortInfo.sip6_txq_couneter_was_incrmented = 0;

        SIM_LOG_PACKET_DESCR_SAVE
        snetFalconTxqProcess(devObjPtr,descrPtr);
        SIM_LOG_PACKET_DESCR_COMPARE("snetFalconTxqProcess");
        if(descrPtr->haAction.drop == SKERNEL_EXT_PKT_CMD_HARD_DROP_E)
        {
            __LOG(("packet was dropped, by the TXQ and will not egress port [%d] \n",
                egressPort));
            /* packet was dropped */
            packetAllowedToEgressPort = 0;
        }

        if(packetAllowedToEgressPort == 0)
        {
            /* we not send to the port but the EREP unit is expected to do it's logic ...*/
            /* because replication to CPU/analyzer can still happen */
        }
        else
        if((descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_FORWARD_E) && (descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_TRAP_TO_CPU_E))
        {
            __LOG(("packet will be send to CPU in EREP and  dropped \n"));
        }
        else
        {
            /* this part of the EREP is building info in descrPtr->prpInfo.prpTrailerBytes[]
               used by the data-path */
            SIM_LOG_PACKET_DESCR_SAVE
            snetChtErepProcess_prpForDp(devObjPtr, descrPtr);
            SIM_LOG_PACKET_DESCR_COMPARE("snetChtErepProcess_prpForDp");

            txFifoPort = descrPtr->egressPhysicalPortInfo.txDmaMacPort;
            __LOG_PARAM(txFifoPort);

            sip6EgressTxFifoAndMac(devObjPtr,descrPtr,txFifoPort,frameDataPtr,frameDataSize);
        }

        /* notify the counters that the packet out of the TXQ (allow CPSS to know that it is safe to 'set down the MAC') */
        if(descrPtr->egressPhysicalPortInfo.sip6_txq_couneter_was_incrmented)
        {
            smemFalconTxqQfcCounterTable1Update(devObjPtr,descrPtr->egressPhysicalPortInfo.txDmaMacPort,GT_FALSE/* decrement*/);
        }


        /* restore the pipeId (that was set by the TXQ) */
        smemSetCurrentPipeId(devObjPtr, pipeId);
    }

    SIM_LOG_PACKET_DESCR_SAVE
    snetChtErepProcess(devObjPtr, descrPtr, egressPort, (!packetAllowedToEgressPort));

    SIM_LOG_PACKET_DESCR_COMPARE("snetChtErepProcess");

    return packetAllowedToEgressPort == 1 ? GT_TRUE/*not dropped*/ : GT_FALSE/*dropped*/;
}

#define SMEM_SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_NAME                     \
     STR(SMEM_SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_SRF_ENABLED_E)          \
    ,STR(SMEM_SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_STREAM_TYPE_E)          \
    ,STR(SMEM_SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_SRF_NUMBER_E)           \
    ,STR(SMEM_SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_EGRESS_PORTS_BITMAP_0_E)\
    ,STR(SMEM_SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_EGRESS_PORTS_BITMAP_1_E)

static char * sip6_30PreqSrfMappingTableFieldsNames[SMEM_SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS___LAST_VALUE___E] =
    {SMEM_SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_NAME};

static SNET_ENTRY_FORMAT_TABLE_STC sip6_30PreqSrfMappingTableFieldsFormat[SMEM_SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS___LAST_VALUE___E] =
{
/* SMEM_SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_SRF_ENABLED_E*/
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_STREAM_TYPE_E*/
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_SRF_NUMBER_E*/
    STANDARD_FIELD_MAC(10),
/* SMEM_SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_EGRESS_PORTS_BITMAP_0_E*/
    STANDARD_FIELD_MAC(32),
/* SMEM_SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_EGRESS_PORTS_BITMAP_1_E*/
    STANDARD_FIELD_MAC(32)
};

#define SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_NAME                    \
     STR(SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_SEQ_HIST_LENGTH_E     )\
    ,STR(SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_RECOV_SEQ_NUM_E       )\
    ,STR(SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_TAKE_NO_SEQUENCE_E    )\
    ,STR(SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_ROGUE_PKTS_CNT_E      )\
    ,STR(SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_FIRST_BUFFER_E        )\
    ,STR(SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_NUMBER_OF_BUFFERS_E   )\
    ,STR(SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_CUR_BUFFER_E          )\
    ,STR(SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_CUR_BIT_E             )

static char * sip6_30PreqSrfConfigTableFieldsNames[SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS___LAST_VALUE___E] =
    {SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_NAME};

static SNET_ENTRY_FORMAT_TABLE_STC sip6_30PreqSrfConfigTableFieldsFormat[SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS___LAST_VALUE___E] =
{
/* SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_SEQ_HIST_LENGTH_E*/
    STANDARD_FIELD_MAC(11),
/* SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_RECOV_SEQ_NUM_E*/
    STANDARD_FIELD_MAC(16),
/* SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_TAKE_NO_SEQUENCE_E*/
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_ROGUE_PKTS_CNT_E*/
    STANDARD_FIELD_MAC(32),
/* SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_FIRST_BUFFER_E*/
    STANDARD_FIELD_MAC(10),
/* SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_NUMBER_OF_BUFFERS_E*/
    STANDARD_FIELD_MAC(4),
/* SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_CUR_BUFFER_E*/
    STANDARD_FIELD_MAC(10),
/* SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_CUR_BIT_E*/
    STANDARD_FIELD_MAC(7)
};


#define SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_NAME                    \
     STR(SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_PASSED_PKTS_CNT_E        )\
    ,STR(SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_DISCARDED_PKTS_CNT_E     )\
    ,STR(SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_OUT_OF_ORDER_PKTS_CNT_E  )\
    ,STR(SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_CLEARED_ZERO_HIST_CNT_E  )\
    ,STR(SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_RESTART_COUNTER_E        )\
    ,STR(SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_RESTART_THRESHOLD_E      )

static char * sip6_30PreqSrfCountersTableFieldsNames[SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS___LAST_VALUE___E] =
    {SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_NAME};

static SNET_ENTRY_FORMAT_TABLE_STC sip6_30PreqSrfCountersTableFieldsFormat[SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS___LAST_VALUE___E] =
{
/* SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_PASSED_PKTS_CNT_E*/
    STANDARD_FIELD_MAC(32),
/* SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_DISCARDED_PKTS_CNT_E*/
    STANDARD_FIELD_MAC(32),
/* SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_OUT_OF_ORDER_PKTS_CNT_E*/
    STANDARD_FIELD_MAC(32),
/* SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_CLEARED_ZERO_HIST_CNT_E*/
    STANDARD_FIELD_MAC(32),
/* SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_RESTART_COUNTER_E*/
    STANDARD_FIELD_MAC(16),
/* SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_RESTART_THRESHOLD_E*/
    STANDARD_FIELD_MAC(16)
};

#define SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_NAME                    \
     STR(SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_NUMBER_OF_RESETS_E         )\
    ,STR(SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_ENABLE_E                )\
    ,STR(SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_BASE_DIFFERENCE_E       )\
    ,STR(SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_DIFFERENCE_E            )\
    ,STR(SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_RESET_TIME_E            )\
    ,STR(SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_TIME_SINCE_LAST_RESET_E )

static char * sip6_30PreqSrfDaemonTableFieldsNames[SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS___LAST_VALUE___E] =
    {SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_NAME};

static SNET_ENTRY_FORMAT_TABLE_STC sip6_30PreqSrfDaemonTableFieldsFormat[SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS___LAST_VALUE___E] =
{
/*SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_NUMBER_OF_RESETS_E*/
    STANDARD_FIELD_MAC(16),
/*SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_ENABLE_E*/
    STANDARD_FIELD_MAC( 1),
/*SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_BASE_DIFFERENCE_E*/
    STANDARD_FIELD_MAC(32),
/*SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_DIFFERENCE_E*/
    STANDARD_FIELD_MAC(20),
/*SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_RESET_TIME_E*/
    STANDARD_FIELD_MAC( 5),
/*SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_TIME_SINCE_LAST_RESET_E*/
    STANDARD_FIELD_MAC( 5)
};


/**
* @internal snetFalconPreqTablesFormatInit function
* @endinternal
*
* @brief   init the format of preq tables.
*
* @param[in] devObjPtr                - pointer to device object.
*/
void snetFalconPreqTablesFormatInit(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr
)
{
    if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
    {
        LION3_TABLES_FORMAT_INIT_MAC(devObjPtr, SKERNEL_TABLE_FORMAT_PREQ_SRF_MAPPING_E,
            sip6_30PreqSrfMappingTableFieldsFormat, sip6_30PreqSrfMappingTableFieldsNames);
        LION3_TABLES_FORMAT_INIT_MAC(devObjPtr, SKERNEL_TABLE_FORMAT_PREQ_SRF_CONFIG_E,
            sip6_30PreqSrfConfigTableFieldsFormat, sip6_30PreqSrfConfigTableFieldsNames);
        LION3_TABLES_FORMAT_INIT_MAC(devObjPtr, SKERNEL_TABLE_FORMAT_PREQ_SRF_COUNTERS_E,
            sip6_30PreqSrfCountersTableFieldsFormat, sip6_30PreqSrfCountersTableFieldsNames);
        LION3_TABLES_FORMAT_INIT_MAC(devObjPtr, SKERNEL_TABLE_FORMAT_PREQ_SRF_DAEMON_E,
            sip6_30PreqSrfDaemonTableFieldsFormat, sip6_30PreqSrfDaemonTableFieldsNames);

    }
}

