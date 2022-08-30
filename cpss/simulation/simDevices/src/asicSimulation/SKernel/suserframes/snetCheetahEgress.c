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
* @file snetCheetahEgress.c
*
* @brief This is a external API definition for egress frame processing.
*
*
* @version   244
********************************************************************************
*/

#include <asicSimulation/SKernel/smain/smain.h>
#include <asicSimulation/SKernel/suserframes/snet.h>
#include <asicSimulation/SKernel/skernel.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahEgress.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah3CentralizedCnt.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahEq.h>
#include <asicSimulation/SKernel/suserframes/snetXCat.h>
#include <asicSimulation/SKernel/suserframes/snetLion.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah3EPcl.h>
#include <asicSimulation/SKernel/suserframes/snetXCatPolicer.h>
#include <asicSimulation/SKernel/suserframes/snetLion2Oam.h>
#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SLog/simLogInfoTypePacket.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahTxQ.h>
#include <asicSimulation/SKernel/smain/sRemoteTm.h>
#include <asicSimulation/SKernel/suserframes/snetFalconEgress.h>
#include <asicSimulation/SKernel/suserframes/snetFalconTxQ.h>


#include <common/Utils/Math/sMath.h>

static GT_BOOL connectedPortToPhyEgress[128] = {GT_FALSE};
GT_STATUS wmSetPortConnectedToPhyEgress(IN GT_U32 portNum, IN GT_BOOL connected)
{
    connectedPortToPhyEgress[portNum] = connected;

    return GT_OK;
}

typedef enum{
    TXQ_COUNTE_MODE_ALL_E,
    TXQ_COUNTE_MODE_INGRESS_DEVICE_ONLY_E,/*ingress device*/
    TXQ_COUNTE_MODE_EGRESS_DEVICE_ONLY_E/*egress device*/
}TXQ_COUNTE_MODE_ENT;

typedef enum{
    TIMESTAMP_TIME_ADD_MODE_EGRESS_E,       /* time of transmission timestamp */
    TIMESTAMP_TIME_ADD_MODE_CORRECTED_E,    /* time of residence timestamp */
    TIMESTAMP_TIME_ADD_MODE_INGRESS_E,      /* time of reception timestamp */
    TIMESTAMP_TIME_ADD_MODE_INGRESS_EGRESS_E/* SIP-6: time of reception and transmission timestamp */
}TIMESTAMP_TIME_ADD_MODE_ENT;

static GT_VOID snetChtEgressDev_part2
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32   egressPort,
    IN GT_U32   egressVlanTagMode,
    IN GT_U32 destPorts[],
    IN GT_U8 destVlanTagged[],
    IN GT_U32 destPortsLb[]

);

static void snetLion3EgressEportQosRemarkingInfoGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    OUT SNET_CHEETAH_AFTER_HA_COMMON_ACTION_STC *eportQosInfoPtr,
    IN GT_BIT egressIsMpls,
    IN GT_BIT egressIsIp,
    IN GT_BIT in_IsMpls,
    IN GT_BIT in_IsIp
);

static void snetChtMacMibCountUpdate
(
  IN SKERNEL_DEVICE_OBJECT * devObjPtr,
  IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 egressPort,
    IN GT_U8 * frameData,
    IN GT_U32  frameSize
);
static void snetChtTxMacCountUpdate
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 port,
    IN GT_U8 * frameData,
    IN GT_U32  frameSize
);
static void snetChtCpuCountUpdate
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);
static void snetChtTxQClass
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);
static void snetChtTxQGetDpv
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    INOUT GT_U32 destPorts[],
    INOUT GT_U8 destVlanTagged[],
    INOUT SKERNEL_STP_ENT stpVector[],
    IN SKERNEL_EGR_PACKET_TYPE_ENT packetType,
    INOUT SKERNEL_PORTS_BMP_STC *pktFilteredPortsBmpPtr,
    IN GT_U32 oamLoopbackFilterEn[]
);
static void snetChtTxQGetVidxInfo
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SKERNEL_EGR_PACKET_TYPE_ENT packetType,
    OUT GT_U32 destPorts[],
    OUT GT_U8 destVlanTagged[],
    OUT SKERNEL_STP_ENT stpVector[]
);
static void snetChtTxQFilters
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    INOUT GT_U32 destPorts[],
    INOUT SKERNEL_STP_ENT stpVector[],
    IN SKERNEL_EGR_PACKET_TYPE_ENT packetType,
    INOUT SKERNEL_PORTS_BMP_STC *pktFilteredPortsBmpPtr,
    IN GT_U32 oamLoopbackFilterEn[],
    OUT SKERNEL_PORTS_BMP_STC * forwardRestrictionBmpPtr
);
static void snetChtTxQPortLinkStat
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    INOUT GT_U32 destPorts[],
    OUT GT_U32 * pcktCountBeforeFltr_PTR,
    OUT GT_U32 backupDestPort[],
    OUT SKERNEL_PORTS_BMP_STC  * saveDestPortPtr,
    IN GT_U32 oamLoopbackFilterEn[]
);
static void snetChtTxGetTrunkDesignatedPorts
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN TRUNK_HASH_MODE_ENT    trunkHashMode,
    OUT SKERNEL_PORTS_BMP_STC * designatedPortsBmpPtr
);
static void snetChtTxQOamFilter
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SKERNEL_EGR_PACKET_TYPE_ENT packetType,
    INOUT GT_U32 destPorts[],
    IN GT_U32 oamLoopbackFilterEn[]
);
static void snetChtTxQEgressMeshIdFilters
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SKERNEL_EGR_PACKET_TYPE_ENT packetType,
    INOUT GT_U32 destPorts[]
);
static void snetChtTxQEgressVid1MismatchFilter
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    INOUT GT_U32 destPorts[]
);
static void snetChtTxQSSTFilters
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    INOUT GT_U32 destPorts[]
);
static void snetChtTxQUnknUnregFilters
(
  IN SKERNEL_DEVICE_OBJECT * devObjPtr,
  IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
  INOUT GT_U32 destPorts[],
  IN SKERNEL_EGR_PACKET_TYPE_ENT packetType
);
static GT_U32 snetChtTxQGetEgrPortForTrgDevice
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_BOOL  calledFromTxdUnit
);
static void snetChtTxQFromCpuExcludePortOrTrunk
(
  IN SKERNEL_DEVICE_OBJECT * devObjPtr,
  IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
  INOUT GT_U32 destPorts[]
);
static void snetCht3TxqDestBackUpPorts
(
  IN SKERNEL_DEVICE_OBJECT * devObjPtr,
  IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
  IN GT_U32 backupDestPort[],
  INOUT GT_U32 destPorts[]
);

static void snetChtTxQUcastVlanFilter
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SKERNEL_EGR_PACKET_TYPE_ENT packetType,
    INOUT GT_U32 destPorts[],
    IN SKERNEL_STP_ENT stpVector[],
    IN GT_U32 egressPort1,
    INOUT SKERNEL_PORTS_BMP_STC *pktFilteredPortsBmpPtr,
    IN GT_U32 oamLoopbackFilterEn[]
);
static void snetChtTxQCounterSets
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 destPorts[],
    IN GT_U32 destPortsLb[],
    IN SKERNEL_PORTS_BMP_STC *pktFilteredPortsBmpPtr,
    IN SKERNEL_EGR_PACKET_TYPE_ENT packetType,
    IN SKERNEL_PORTS_BMP_STC *forwardRestrictionBmpPtr,
    IN TXQ_COUNTE_MODE_ENT counteMode,
    IN GT_U32   egressPort
);
static void snetSip6EgfEftCounterSets
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 destPorts[],
    IN SKERNEL_PORTS_BMP_STC *pktFilteredPortsBmpPtr,
    IN SKERNEL_PORTS_BMP_STC *forwardRestrictionBmpPtr
);
static void snetChtTxQSrcPortFilters
(
  IN SKERNEL_DEVICE_OBJECT * devObjPtr,
  IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
  INOUT GT_U32 destPorts[],
  IN SKERNEL_EGR_PACKET_TYPE_ENT packetType
);
static void snetChtTxQTrunkFilters
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    INOUT GT_U32 destPorts[],
    IN SKERNEL_EGR_PACKET_TYPE_ENT packetType
);
static void snetChtTxQStpFilters
(
  IN SKERNEL_DEVICE_OBJECT * devObjPtr,
  IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
  INOUT GT_U32 destPorts[],
  IN SKERNEL_STP_ENT stpVector[],
  IN GT_U32 oamLoopbackFilterEn[]
);

static void snetChtTxQPortFrwrdRestriction
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    INOUT GT_U32 destPorts[],
    IN GT_U32 oamLoopbackFilterEn[],
    OUT SKERNEL_PORTS_BMP_STC * forwardRestrictionBmpPtr
);
static void snetChtTxQOamLoopbackFilterEnableGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    OUT GT_U32 oamLoopbackFilterEn[]
);

static void egressTxFifoAndMac
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32               txFifoPort,
    IN GT_U8                *frameDataPtr,
    IN GT_U32               frameDataSize
);

static GT_VOID snetLion3PtpUpdateCounters
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32   macPort
);

static GT_VOID snetlion3TimestampPortEgressQueueInsertEntry
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 macPort,
    IN GT_U32 queueNum,
    IN GT_U32 *queueEntryPtr
);


extern GT_U32 smainIsEmulateRemoteTm
(
    void
);

static void ipHeaderEcnSet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    OUT GT_U8  *l3StartOffsetPtr
);

static void snetSip5FwdToLb
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32  destPorts[],
    OUT GT_U32 destPortsLb[]
);

static GT_VOID snetSip6_30PtpMacCorrectionFieldUpdate
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 macPort
);

static GT_VOID snetlion3PtpUdpChecksum
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

static GT_VOID snetSip6_30TimestampPortMacEgressQueueCapture
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32   egressPort,
    IN GT_U32   macPort,
    IN SNET_PHY_SWITCH_USGMII_PCH_PTP_INFO_STC * pchInfoPtr
);

/* check drop recommendations */
#define TM_DROP_CHECK_DROP_RECOMMENDATIONS_MAC(_outDropInfo,_dropField,_dropMasking,_bitIndex,_dropCommand) \
    if(_outDropInfo._dropField)                                     \
    {                                                               \
        if(SMEM_U32_GET_FIELD(_dropMasking , _bitIndex , 1))        \
        {                                                           \
            /* need to drop the packet */                           \
            dropCommand = GT_TRUE;                                  \
        }                                                           \
        else                                                        \
        {                                                           \
            __LOG(("Ignore [%s] due to drop mask of bit[%d] \n" ,   \
                #_dropField ,                                       \
                _bitIndex ));                                       \
        }                                                           \
    }

#define SNET_CHT_SECOND_PORT_FAST_STACK_FAILOVE_GET_MAC(dev, entry) \
    ((dev)->supportEArch ? SMEM_U32_GET_FIELD(entry[0], 0, devObjPtr->flexFieldNumBitsSupport.phyPort) : \
        (dev->txqRevision != 0) ? \
        SMEM_U32_GET_FIELD(entry[0], 0, 6) : \
        SMEM_U32_GET_FIELD(entry[0], 0, 5))

#define SNET_CHT_DP_TO_CFI_VALUE_GET_MAC(dev, entry, dp) \
    (dev->txqRevision != 0) ? \
        SMEM_U32_GET_FIELD(entry[0], (7 + (dp)), 1) : \
        SMEM_U32_GET_FIELD(entry[0], ((dp)), 1)

/* macro to modify up,vid in given tag */
#define MODIFY_VID_UP_MAC(tagPtr,_modifyVid,_vid,_modifyUp,_up)  \
        if(_modifyUp)                                  \
        {                                              \
            __LOG(("MODIFY_UP: %d \n",_up));           \
            (tagPtr)[2] &= ~(0x7 << 5);                \
            (tagPtr)[2] |= (_up) << 5;                 \
        }                                              \
                                                       \
        if(_modifyVid)                                 \
        {                                              \
            __LOG(("MODIFY_VID: 0x%3.3x \n",_vid));    \
            (tagPtr)[2] &= ~(0xf);                     \
            (tagPtr)[2] |= (_vid) >> 8;                \
            (tagPtr)[3]  = (_vid) & 0xFF;              \
        }

/* convert from timestamp queue direction and number to queue index */
#define TS_QUEUE_NUMBERING_TO_QUEUE_INDEX_MAC(direction, queueNum) \
        ((direction == SMAIN_DIRECTION_INGRESS_E)?(queueNum):(queueNum+2))

#define BIT_31 0x80000000

/* Choose between target port and loopback port. If lbPort's bit 31 is 0
 * target port trgPort should be taken as destination port. Otherwise
 * loopback port number (lbPort's bits 30:0) should be taken instead.
 * See snetSip5FwdToLb destPortsLb-parameter description for more details.
 */
#define CHOOSE_TRG_OR_LB_PORT_MAC(trgPort, lbPort) \
    ((lbPort) &  BIT_31 ? (lbPort)-BIT_31 : (trgPort))

/* type enum : MAC_PORT_REASON_TYPE_ENT
    the reasons for getting the mac of a port.

   MAC_PORT_REASON_TYPE_EGF_TO_MAC_FOR_LINK_STATUS_E - the EGF need to filter the ports that are 'down'
                    so it need to know the 'MAC' that represent his port (egf port).

   MAC_PORT_REASON_TYPE_TXQ_DQ_TO_TXDMA_E - the TXQ_DQ port number need to be converted to
            TxDma port number (the same as 'MAC' port)

            NOTE: sip5 only because the TXQ did not have in legacy devices such
                mapping to TxDMA port.

            NOTE: not relevant to SIP6

   MAC_PORT_REASON_TYPE_FINAL_MAC_E - the final packet send need to have the actual
            MAC that send the packet

            NOTE: in Lion2 the ports 9,11 supports 'dual mac' for ports 12,14.
                  for other devices the map is 1 to 1

   BM_CHECK_INGRESS_SRC_PORT_E - (sip5) the BM (buffer management) need to check mapping
            between the 'local dev src port' to the 'rxdma src port'
            when no match packet may be dropped by the HW / cause buffers 'underrun'
            causing buffers to be occupied and not be available for others ....
            for simulation we will drop such packet with LOG info.
            (finding reason for such issue on HW may take MORE then few hours)
            NOTE: not relevant to SIP6

*/
typedef enum{
    MAC_PORT_REASON_TYPE_EGF_TO_MAC_FOR_LINK_STATUS_E,
    MAC_PORT_REASON_TYPE_TXQ_DQ_TO_TXDMA_E,
    MAC_PORT_REASON_TYPE_FINAL_MAC_E,

    BM_CHECK_INGRESS_SRC_PORT_E
}MAC_PORT_REASON_TYPE_ENT;

/* ERMRK : Cumulative Scale Rate TAI<<%i>> register0/1 */
#define SMEM_LION3_ERMRK_CUMULATIVE_SCALE_RATE_CONFIG_REG(dev,tai,index)                      \
    (index == 0) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->ERMRK.cumulativeScaleRateTaiReg0[tai] : \
                   SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->ERMRK.cumulativeScaleRateTaiReg1[tai]

/* GOP, PTP - PTP timestamping Precision Control register */
#define SMEM_SIP6_10_GOP_PTP_TS_PRECISION_CTRL_REG(dev,port) \
    SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->GOP_PTP.PTP[port].timestampingPrecisionControl

/* USX unit */
#define USX_PORT_EXT(dev,_macPort)\
    SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->sip6_MTI[_macPort].USX_MAC_PORT
#define USX_PORT_GLOBAL(dev,_representativePort)\
    SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->sip6_MTI_EXTERNAL[_representativePort].USX_GLOBAL

/**
* @internal snetChtTxMacPortGet function
* @endinternal
*
* @brief   Get TX MAC number.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - the descriptor. relevant when != NULL and needed for:
*                                       reason == MAC_PORT_REASON_TYPE_TXQ_DQ_TO_TXDMA_E
* @param[in] reason                   - the  to get the 'MAC port' for this 'egressPort'
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
static GT_BOOL snetChtTxMacPortGet
(
    IN SKERNEL_DEVICE_OBJECT   *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN MAC_PORT_REASON_TYPE_ENT reason,
    IN GT_U32                  egressPort,
    IN GT_BOOL                 needGlobalToLocalConvert,
    OUT SKERNEL_DEVICE_OBJECT  **txDmaDevObjPtrPtr,
    OUT GT_U32                 *txDmaLocalPortPtr
)
{
    DECLARE_FUNC_NAME(snetChtTxMacPortGet);

    GT_U32  localPort; /* local port */
    GT_U32  portGroupId;/* port group for the local port */
    SKERNEL_DEVICE_OBJECT  *txDmaDevObjPtr;/*pointer to the 'TX DMA port' egress device object*/
    GT_U32  txqPort;
    GT_U32  txDmaPort;
    GT_U32  portToUse = egressPort;
    GT_U32  dmaNumOfCpuPort;/* the SDMA CPU port number */
    GT_U32  bmaSrcRxDmaPortNumber;/* sip5 : BMA unit src port that packet should be associated with */
    GT_U32  regAddr;
    GT_U32  localRxDmaPort;/*number of DMA in specific RxDMA unit */

    dmaNumOfCpuPort = devObjPtr->dmaNumOfCpuPort ? devObjPtr->dmaNumOfCpuPort : SNET_CHT_CPU_PORT_CNS;

    if(reason == BM_CHECK_INGRESS_SRC_PORT_E)
    {
        /*sip5 only */
        smemRegGet(devObjPtr,
            SMEM_BOBCAT2_BMA_PORT_MAPPING_TBL_MEM(devObjPtr,descrPtr->localDevSrcPort),
            &bmaSrcRxDmaPortNumber);

        __LOG_PARAM(bmaSrcRxDmaPortNumber);

        if(bmaSrcRxDmaPortNumber !=
            descrPtr->ingressRxDmaPortNumber_forBma)
        {
            __LOG(("ERROR : the device will try to free buffer from port 'bmaSrcRxDmaPortNumber'[%d] \n",
                bmaSrcRxDmaPortNumber));
            __LOG(("that was not allocated by this port (was on ingressRxDmaPortNumber_forBma [%d]) \n",
                descrPtr->ingressRxDmaPortNumber_forBma));

            return GT_FALSE;
        }

        /* no error . BMA is 'synched' with the other units */
        return GT_TRUE;
    }
    else
    if(descrPtr != NULL &&
       reason == MAC_PORT_REASON_TYPE_TXQ_DQ_TO_TXDMA_E)
    {
        GT_U32  global_egressTxqDqIndexId = 0;/* the global TXQ_DQ unit Id */
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /* convert the egress port to TX port , now we can convert the Tx port to TxDma port */
            txqPort = descrPtr->txqDestPorts[egressPort];

            if(devObjPtr->multiDataPath.numTxqDq)
            {
                GT_U32  numPortsPerDq = devObjPtr->multiDataPath.txqDqNumPortsPerDp;
                GT_U32  globalTxqDqPortNum = txqPort;
                GT_U32  localTxqDqPortNum = txqPort % numPortsPerDq;
                /* get 'hard wired' connection between the TXQ_PORT and the 'serving' TXQ_DQ */
                GT_U32  dqIndex = txqPort / numPortsPerDq;
                GT_U32  global_egressDpIndexId;/* the global DP unit Id (for TXDMA and TXFIFO)*/

                global_egressTxqDqIndexId = dqIndex;/* global DQ[index] */
                if(devObjPtr->multiDataPath.dqInfo[dqIndex].use_egressDpIndex)
                {
                    /* get 'hard wired' connection between the TXQ_PORT and the 'serving' TXDMA */
                   global_egressDpIndexId =
                        devObjPtr->multiDataPath.dqInfo[dqIndex].egressDpIndex;
                }
                else
                {
                    global_egressDpIndexId = dqIndex;
                }

                /* convert to local DQ in the egress PIPE */
                descrPtr->egressPhysicalPortInfo.egressTxqDqIndexId = global_egressTxqDqIndexId % devObjPtr->multiDataPath.numTxqDq;
                descrPtr->egressPhysicalPortInfo.egressDpIndexId = global_egressDpIndexId % devObjPtr->multiDataPath.numTxqDq;
                descrPtr->egressPhysicalPortInfo.egressPipeId    = global_egressDpIndexId / devObjPtr->multiDataPath.numTxqDq;



                __LOG(("txqPort[%d] is considered in (global) TXQ_DQ[%d] and to (global) TXDMA[%d] \n",
                    txqPort ,
                    global_egressTxqDqIndexId,
                    global_egressDpIndexId));

                __LOG_PARAM(descrPtr->egressPhysicalPortInfo.egressPipeId);
                __LOG_PARAM(descrPtr->egressPhysicalPortInfo.egressDpIndexId);
                __LOG_PARAM(descrPtr->egressPhysicalPortInfo.egressTxqDqIndexId);

                /* set new PIPE id for the device */
                smemSetCurrentPipeId(devObjPtr,descrPtr->egressPhysicalPortInfo.egressPipeId);
                /* set new PIPE id for the device */
                /* convert the TXQ port to local TXQ port in the pipe */

                __LOG_PARAM(globalTxqDqPortNum);
                __LOG_PARAM(localTxqDqPortNum);

                txqPort = localTxqDqPortNum;
            }
            else
            {
                descrPtr->egressPhysicalPortInfo.egressPipeId = 0;
                descrPtr->egressPhysicalPortInfo.egressDpIndexId = 0;
                descrPtr->egressPhysicalPortInfo.egressTxqDqIndexId = 0;
            }

            smemRegFldGet(devObjPtr,
                SMEM_LION3_TXQ_DQ_TXQ_PORT_TO_TX_DMA_MAP_REG(devObjPtr,txqPort,descrPtr->egressPhysicalPortInfo.egressTxqDqIndexId),
                    0, 8, &txDmaPort);

            if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))/* the <txDmaPort> is local port in the DP[] */
            {
                /* the caller of this function expect to get 'global' device port number */
                /* so lets convert it */
                smemConvertPipeIdAndLocalPortToGlobal_withPipeAndDpIndex(
                    devObjPtr,
                    descrPtr->egressPhysicalPortInfo.egressPipeId,
                    descrPtr->egressPhysicalPortInfo.egressDpIndexId,
                    txDmaPort,
                    GT_FALSE,/* CPU port not muxed ... not relevant */
                    &txDmaPort);
            }

            __LOG(("Map egressPort[%d] to (local)txqPort[%d] to (local)txDmaPort[%d] in (global)DQ[%d] \n",
                egressPort,txqPort,txDmaPort,global_egressTxqDqIndexId));


            portToUse = txDmaPort;

            if(devObjPtr->supportCutThroughFastToSlow)
            {
                /* 4 ports in register */
                regAddr =
                    SMEM_SIP5_20_TXDMA_LOCAL_DEV_SRC_PORT_TO_DMA_NUMBER_MAP_REG(dev,
                    descrPtr->egressPhysicalPortInfo.egressDpIndexId,
                    (descrPtr->localDevSrcPort >> 2));

                smemRegFldGet(devObjPtr, regAddr , 8*((descrPtr->localDevSrcPort & 0x3)) , 8,
                    &localRxDmaPort);

                if(localRxDmaPort !=
                    descrPtr->ingressRxDmaPortNumber_forBma)
                {
                    __LOG(("WARNING : if the device is in CT mode (cut through) it will get wrong (local) Src RxDMA [%d] for 'speed purpose' ('fast to slow') (good value is[%d]) for src port[%d]\n",
                        localRxDmaPort,
                        descrPtr->ingressRxDmaPortNumber_forBma,
                        descrPtr->localDevSrcPort));
                }
            }
        }
        else
        {
            /* no conversion needed */
        }
    }
    else
    if(reason == MAC_PORT_REASON_TYPE_EGF_TO_MAC_FOR_LINK_STATUS_E)
    {
        /* for SIP5 the egressPort is already mapped from 'egf port' to 'MAC number'
            after using SMEM_LION2_EGF_EFT_VIRTUAL_2_PHYSICAL_PORT_REMAP_REG */

        /* for legacy devices the egressPort not need any convert */

    }
    else
    if(reason == MAC_PORT_REASON_TYPE_FINAL_MAC_E)
    {
        /* use what was already calculated for MAC_PORT_REASON_TYPE_TXQ_DQ_TO_TXDMA_E */

        /* the packet finally egress and need to have its final MAC number
            no conversion needed */
        if((SNET_CHT_CPU_PORT_CNS == egressPort) &&
           (devObjPtr->cpuGopPortNumber != 0))
        {
            __LOG(("MAC for egress CPU Port[%d] is [%d] \n",
                egressPort,devObjPtr->cpuGopPortNumber));

            portToUse = devObjPtr->cpuGopPortNumber;
        }
        else if(SMEM_CHT_IS_SIP5_GET(devObjPtr) && (descrPtr != NULL))
        {
            if(txDmaDevObjPtrPtr)
            {
                *txDmaDevObjPtrPtr = descrPtr->egressPhysicalPortInfo.txDmaDevObjPtr;
                if(descrPtr->egressPhysicalPortInfo.txDmaDevObjPtr == NULL)
                {
                    return GT_FALSE;
                }
            }

            *txDmaLocalPortPtr = descrPtr->egressPhysicalPortInfo.txDmaMacPort;
            return GT_TRUE;
        }
    }
    else
    {
        skernelFatalError("snetChtTxMacPortGet: unsupported reason to get MAC port of egress port \n");
    }

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr) &&
       devObjPtr->portGroupSharedDevObjPtr &&
        needGlobalToLocalConvert == GT_TRUE)
    {
        /* the port is global port and we need to convert to the relevant
           port group and local port */
        portGroupId = SMEM_CHT_PORT_GROUP_ID_FROM_GLOBAL_PORT_MAC(devObjPtr,portToUse);
        /* convert to local port */
        localPort = SMEM_CHT_LOCAL_PORT_FROM_GLOBAL_PORT_MAC(devObjPtr,portToUse);

        txDmaDevObjPtr = devObjPtr->portGroupSharedDevObjPtr->coreDevInfoPtr[portGroupId].devObjPtr;
    }
    else
    {
        localPort = portToUse;
        txDmaDevObjPtr = devObjPtr;
    }

    if(devObjPtr->supportTrafficManager &&
       devObjPtr->dmaNumOfTmPort == localPort &&
       reason == MAC_PORT_REASON_TYPE_TXQ_DQ_TO_TXDMA_E)
    {
        /* when map txq_dq port to DMA of TM port , there is no need to check
           the existence of the port as this DMA always exists
           (for device with TM) */
    }
    else
    if(dmaNumOfCpuPort == localPort &&
       reason == MAC_PORT_REASON_TYPE_TXQ_DQ_TO_TXDMA_E)
    {
        /* when map txq_dq port to DMA of CPU port , there is no need to check
           the existence of the port as this DMA always exists */
    }
    else
    if(txDmaDevObjPtr->portsArr[localPort].state ==
        SKERNEL_PORT_STATE_NOT_EXISTS_E)
    {
        /* port not exists ! can't access it's MAC registers */
        if(txDmaDevObjPtrPtr)
        {
            *txDmaDevObjPtrPtr = NULL;/* try to catch ERROR if trying to use this port */
        }
        *txDmaLocalPortPtr = 0;

        return GT_FALSE;
    }

    *txDmaLocalPortPtr = snetChtExtendedPortMacGet(devObjPtr, localPort, GT_FALSE/*Tx*/);

    if(txDmaDevObjPtrPtr)
    {
        *txDmaDevObjPtrPtr = txDmaDevObjPtr;
    }

    return GT_TRUE;
}

/**
* @internal snetChtEgfEgressEPortEntryGet function
* @endinternal
*
* @brief   function of the EGF unit
*         get pointer to the egress EPort table
*         Set value into descrPtr->eArchExtInfo.egfShtEgressEPortTablePtr
*         Set value into descrPtr->eArchExtInfo.egfQagEgressEPortTablePtr
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
* @param[in] globalEgressPort         - physical egress port -- Global port number !!!
*                                      (referred as <LocalDevTrgPhyPort> in documentation ,
*                                      but it is not part of the 'descriptor' !)
* @param[in] skipLogInfo              - indication to skip LOG info
*                                      GT_TRUE - skip LOG info in the function
*                                      GT_FALSE - allow LOG info in the function
*/
void snetChtEgfEgressEPortEntryGet
(
   IN SKERNEL_DEVICE_OBJECT * devObjPtr,
   IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
   IN GT_U32    globalEgressPort,
   IN GT_BOOL   skipLogInfo
)
{
    DECLARE_FUNC_NAME(snetChtEgfEgressEPortEntryGet);

    GT_U32  egfEportTableIndex;/* table index*/
    GT_U32  regAddr;/* register address */
    SKERNEL_FRAME_CHEETAH_DESCR_STC     *txqOrigDescPtr;

    descrPtr->eArchExtInfo.egfQagEgressEPortTable_index = 0;
    descrPtr->eArchExtInfo.egfShtEgressEPortTablePtr = NULL;
    descrPtr->eArchExtInfo.egfQagEgressEPortTablePtr = NULL;

    if(devObjPtr->supportEArch == 0 || /* device not support eArch */
       devObjPtr->unitEArchEnable.txq == 0)/*TXQ not eArch-Disabled*/
    {
        skernelFatalError("snetChtEgfEgressEPortEntryGet: TXQ not eArch-Enabled ! \n");
    }

    descrPtr->eArchExtInfo.egfShtEgressClass = SHT_PACKET_CLASS_3_E;

    if (descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_FORWARD_E ||
        descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_FROM_CPU_E )
    {
        /*Assign Egress Attributes Locally*/
        if((descrPtr->useVidx == 0) && ((descrPtr->trgDev != descrPtr->ownDev) && (descrPtr->srcDev == descrPtr->ownDev)))
        {
            GT_BOOL assignTrgEPortAttributesLocally;
            GT_U32               regAddr;
            GT_U32               regData;

            regAddr = SMEM_LION2_EGF_SHT_EGRESS_EPORT_TBL_MEM(devObjPtr,descrPtr->trgEPort);
            smemRegGet(devObjPtr, regAddr, &regData);
            assignTrgEPortAttributesLocally = SMEM_U32_GET_FIELD(regData, 28, 1);
            __LOG(("assignTrgEPortAttributesLocally %d for target eport %d\n",assignTrgEPortAttributesLocally, descrPtr->trgEPort));
            descrPtr->eArchExtInfo.assignTrgEPortAttributesLocally |= assignTrgEPortAttributesLocally;
        }

        if((descrPtr->useVidx == 0) &&
           (SKERNEL_IS_MATCH_DEVICES_MAC(descrPtr->trgDev, descrPtr->ownDev,
                                              devObjPtr->dualDeviceIdEnable.txq) ||
            descrPtr->eArchExtInfo.assignTrgEPortAttributesLocally ))
        {
            egfEportTableIndex =  descrPtr->trgEPort;
            descrPtr->eArchExtInfo.egfShtEgressClass = SHT_PACKET_CLASS_1_E;

            if(skipLogInfo == GT_FALSE)
            {
                __LOG_PARAM(egfEportTableIndex);
                __LOG(("egfShtEgressClass = SHT_PACKET_CLASS_1_E;\n"));
            }
        }
        else
        {
            egfEportTableIndex =  globalEgressPort;
            descrPtr->eArchExtInfo.egfShtEgressClass = SHT_PACKET_CLASS_2_E;
            if(skipLogInfo == GT_FALSE)
            {
                __LOG_PARAM(egfEportTableIndex);
                __LOG(("egfShtEgressClass = SHT_PACKET_CLASS_2_E;\n"));
            }

        }
    }
#if 0 /* fix for test prvTgfEgressMirrorEportVsPhysicalPort */
    else if(descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E && descrPtr->rxSniff == 0)
    {
        if(descrPtr->txqToEq)
        {
            txqOrigDescPtr = descrPtr->origDescrPtr;
            if(txqOrigDescPtr->useVidx == 0)
            {
                egfEportTableIndex = txqOrigDescPtr->trgEPort;
            }
            else
            {
                egfEportTableIndex = descrPtr->sniffTrgPort;
            }
        }
        else
        {
            /* egress mirroring for packet from DSA */
            /* the egress eport table should not be accessed */
            descrPtr->eArchExtInfo.egfQagEgressEPortTablePtr = NULL;
            return;
        }
    }
#endif /*0*/
    else if(descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_CPU_E && descrPtr->srcTrg == 1)
    {
        if(descrPtr->txqToEq)
        {
            txqOrigDescPtr = descrPtr->origDescrPtr;
            egfEportTableIndex = txqOrigDescPtr->trgEPort;
            if(skipLogInfo == GT_FALSE)
            {
                __LOG_PARAM(egfEportTableIndex);
            }
        }
        else
        {
            if(skipLogInfo == GT_FALSE)
            {
                __LOG((" TO_CPU packet from DSA cascade port \n"));
                __LOG((" the egress eport table should not be accessed \n"));
            }
            descrPtr->eArchExtInfo.egfQagEgressEPortTablePtr = NULL;
            return;
        }
    }
    else
    {
        if(skipLogInfo == GT_FALSE)
        {
            __LOG((" the egress eport table should not be accessed \n"));
        }
        descrPtr->eArchExtInfo.egfQagEgressEPortTablePtr = NULL;
        return;
    }

    /* EGF-QAG - Egress ePort Table */
    regAddr = SMEM_LION2_EGF_QAG_EGRESS_EPORT_TBL_MEM(devObjPtr,egfEportTableIndex);
    descrPtr->eArchExtInfo.egfQagEgressEPortTablePtr = smemMemGet(devObjPtr, regAddr);
    descrPtr->eArchExtInfo.egfQagEgressEPortTable_index = egfEportTableIndex;

    /* EGF-SHT - Egress ePort Table */
    if(descrPtr->useVidx)
    {
        if(skipLogInfo == GT_FALSE)
        {
            __LOG(("EGF-SHT: the eport table is not accessed for physical port[%d] , rather the 'registers' are accessed \n",
                globalEgressPort));
        }
        descrPtr->eArchExtInfo.egfShtEgressEPortTablePtr = NULL;
    }
    else
    {
        regAddr = SMEM_LION2_EGF_SHT_EGRESS_EPORT_TBL_MEM(devObjPtr,egfEportTableIndex);
        descrPtr->eArchExtInfo.egfShtEgressEPortTablePtr = smemMemGet(devObjPtr, regAddr);
    }
}

/**
* @internal snetChtTxqDqPerPortInfoGet function
* @endinternal
*
* @brief   Txq.dq - get indication about the index of per port and use of second register
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - egress port (local port on multi-port group device)
*                                      CPU port is port 63
*
* @param[out] isSecondRegisterPtr      - pointer to 'use the second register'
* @param[out] outputPortBitPtr         - pointer to the bit index for the egress port
*/
void snetChtTxqDqPerPortInfoGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN    SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN GT_U32   egressPort,
    OUT GT_BOOL     *isSecondRegisterPtr,
    OUT GT_U32     *outputPortBitPtr
)
{
    *isSecondRegisterPtr = GT_FALSE;

    if(devObjPtr->txqRevision == 0)
    {
        *outputPortBitPtr = egressPort & 0x1f;
        return;
    }

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* the DQ supports 72 TXQ_ports , but need to convert the egressPort
           (physical port) to TXQ_PORT */
        /* convert 256 range into 72 values */
        *outputPortBitPtr = descrPtr->txqDestPorts[egressPort];

        if(devObjPtr->portGroupSharedDevObjPtr) SIM_TBD_BOOKMARK
        {
            /* make Lion3 like Lion2 */
            *outputPortBitPtr =  (*outputPortBitPtr) & 0xf;/*4 bits*/
        }

    }
    else
    {
        if(devObjPtr->portGroupSharedDevObjPtr)
        {
            /* the DQ supports only local ports */
            *outputPortBitPtr =  egressPort & 0xf;/*4 bits*/
        }
        else if(egressPort < 32)
        {
            *outputPortBitPtr = egressPort;
        }
        else /* egressPort >=32 */
        {
            *isSecondRegisterPtr = GT_TRUE;
            *outputPortBitPtr = egressPort & 0x1f;
        }
    }

    return;
}

/**
* @internal snetChtTxQPhase1 function
* @endinternal
*
* @brief   Create destination ports vector
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in,out] destPorts[]          - number of egress port. out array items
*                                       values can be out of {0,1}.
* @param[in,out] destVlanTagged[]     - send frame with tag.
* @param[in,out] destPortsLb[]        - destPortsLb[N] is loopback port for
*                                       target port N if destPorts[N] = 1.
*
* @retval GT_U32                   - number of outgoing ports
*/
static GT_U32 snetChtTxQPhase1
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    INOUT GT_U32 destPorts[],
    INOUT GT_U8 destVlanTagged[],
    INOUT GT_U32 destPortsLb[]
)
{
    DECLARE_FUNC_NAME(snetChtTxQPhase1);

    SKERNEL_EGR_PACKET_TYPE_ENT packetType; /* type of packet  */
    SKERNEL_STP_ENT stpVector[SKERNEL_CHEETAH_EGRESS_MAX_PORT_CNS]; /* STP port state vector */
    GT_U32  port;       /*port iterator*/
    GT_U32 outPorts;  /* number of output ports */
    GT_U32 oamLoopbackFilterEn[SKERNEL_CHEETAH_EGRESS_MAX_PORT_CNS];/* array of OAM loop enabled*/
    TXQ_COUNTE_MODE_ENT txqCountMode;/* TXQ count mode */
    SKERNEL_PORTS_BMP_STC pktFilteredPortsBmp; /* bitmap of filtered egress ports */
    SKERNEL_PORTS_BMP_STC forwardRestrictionBmp; /*bitmap of forwarding restriction ports*/
    SKERNEL_PORTS_BMP_STC  *pktFilteredPortsBmpPtr = &pktFilteredPortsBmp;
    SKERNEL_PORTS_BMP_STC  *forwardRestrictionBmpPtr = &forwardRestrictionBmp;
    GT_U32    fldValue, regAddress;/*register value*/
    GT_BOOL   dropInEftEnable; /* Egress filtering enable status for <pktCmd>=HARD/SOFT DROP */
    SIM_LOG_TARGET_ARR_DECLARE(destPorts);
    SIM_LOG_TARGET_ARR_DECLARE(destPortsLb);
    SIM_LOG_TARGET_ARR_DECLARE(oamLoopbackFilterEn);
    SIM_LOG_TARGET_BMP_PORTS_DECLARE(pktFilteredPortsBmpPtr);
    SIM_LOG_TARGET_BMP_PORTS_DECLARE(forwardRestrictionBmpPtr);

    /* initialize the arrays and bitmaps */
    memset(stpVector,0,sizeof(stpVector));
    memset(oamLoopbackFilterEn,0,sizeof(oamLoopbackFilterEn));
    SKERNEL_PORTS_BMP_CLEAR_MAC(&pktFilteredPortsBmp);
    SKERNEL_PORTS_BMP_CLEAR_MAC(&forwardRestrictionBmp);
    SIM_LOG_PACKET_DESCR_SAVE

    /* update the 'routed' and the 'doRouterHa' according to modify macDa,Sa
       the fields of 'modify macDa,Sa' only added in Lion-B devices so we have
       'double' terminology
    */

    /* Modify MAC DA and MAC SA for tunnel terminated packets with IP passanger redirected to egress interface */
    if (SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        if (descrPtr->ttiRedirectCmd == PCL_TTI_ACTION_REDIRECT_CMD_OUT_IF_E &&
            descrPtr->tunnelTerminated == 1 &&
            descrPtr->innerPacketType == SKERNEL_INNER_PACKET_TYPE_IP)
        {
            descrPtr->modifyMacSa = 1;
            descrPtr->modifyMacDa = 1;
        }
    }

    descrPtr->routed     |= descrPtr->modifyMacSa;
    descrPtr->doRouterHa |= descrPtr->modifyMacDa;

    /* for dual device Id update the 'Own device number' */
    if(devObjPtr->dualDeviceIdEnable.txq)
    {
        smemRegFldGet(devObjPtr, SMEM_CHT_GLB_CTRL_REG(devObjPtr), 4, 5, &fldValue);
        if(fldValue != descrPtr->ownDev)
        {
            __LOG(("for dual device Id update the 'Own device number' [%d] \n",
                fldValue));
        }

        /* ownDevNum[4:0] */
        descrPtr->ownDev = fldValue;
    }

    /* packet type classification */
    snetChtTxQClass(devObjPtr,descrPtr);
    packetType = descrPtr->egressPacketType;

    /* get the OAM loopback enable array */
    __LOG(("get the OAM loopback enable array"));

    /* save the oamLoopbackFilterEn for the logger  (for compare changes) */
    SIM_LOG_TARGET_ARR_SAVE(oamLoopbackFilterEn);
    snetChtTxQOamLoopbackFilterEnableGet(devObjPtr, descrPtr,oamLoopbackFilterEn);
    /* compare the changes in oamLoopbackFilterEn */
    SIM_LOG_TARGET_ARR_COMPARE(oamLoopbackFilterEn);


    /* save the destPorts for the logger  (for compare changes) */
    SIM_LOG_TARGET_ARR_SAVE(destPorts);
    SIM_LOG_TARGET_BMP_PORTS_SAVE(pktFilteredPortsBmpPtr);

    /* Get the status of egress filtering */
    dropInEftEnable = 1;
    if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
    {
        regAddress = SMEM_LION3_EGF_EFT_EGR_FILTERS_GLOBAL_REG(devObjPtr);
        smemRegGet(devObjPtr, regAddress, &fldValue);
        dropInEftEnable = SMEM_U32_GET_FIELD(fldValue, 16, 1);
    }

    if(dropInEftEnable == 0 && (descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_HARD_DROP_E ||
                                descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_SOFT_DROP_E))
    {
        outPorts = 1;
        destPorts[descrPtr->trgEPort] = 1;
        return outPorts;
    }

    /* get destination port vector */
    __LOG(("get destination port vector"));
    snetChtTxQGetDpv(devObjPtr, descrPtr, destPorts, destVlanTagged,
                     stpVector, packetType,
                     &pktFilteredPortsBmp, oamLoopbackFilterEn);

    /* compare the changes in destPorts */
    SIM_LOG_TARGET_ARR_COMPARE(destPorts);
    SIM_LOG_TARGET_BMP_PORTS_COMPARE(pktFilteredPortsBmpPtr);

    /* save the destPorts for the logger  (for compare changes) */
    SIM_LOG_TARGET_ARR_SAVE(destPorts);
    SIM_LOG_TARGET_BMP_PORTS_SAVE(pktFilteredPortsBmpPtr);
    SIM_LOG_TARGET_BMP_PORTS_SAVE(forwardRestrictionBmpPtr);

    SIM_LOG_PACKET_DESCR_COMPARE("snetChtTxQPhase1 - after snetChtTxQGetDpv :");
    SIM_LOG_PACKET_DESCR_SAVE

    /* packet filtering and port forwarding restriction */
    __LOG(("packet filtering and port forwarding restriction"));

    snetChtTxQFilters(devObjPtr, descrPtr, destPorts,
                    stpVector, packetType,
                    &pktFilteredPortsBmp, oamLoopbackFilterEn, &forwardRestrictionBmp);

    /* compare the changes in destPorts */
    SIM_LOG_TARGET_ARR_COMPARE(destPorts);
    SIM_LOG_TARGET_BMP_PORTS_COMPARE(pktFilteredPortsBmpPtr);
    SIM_LOG_TARGET_BMP_PORTS_COMPARE(forwardRestrictionBmpPtr);

    SIM_LOG_PACKET_DESCR_COMPARE("snetChtTxQPhase1 - after snetChtTxQFilters :");

    if (SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* packet forwarding to loopback/service port */
        __LOG(("packet forwarding to loopback/service port"));

        SIM_LOG_TARGET_ARR_SAVE(destPortsLb);

        snetSip5FwdToLb(devObjPtr, descrPtr, destPorts, destPortsLb);

        /* compare the changes in destPorts */
        SIM_LOG_TARGET_ARR_COMPARE(destPortsLb);
    }

    /* at the end of the EGF (egress filtering) - count the number of ports */
    outPorts = 0;
    for(port = 0 ; port < SKERNEL_DEV_EGRESS_MAX_PORT_CNS(devObjPtr); port++)
    {
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /* loop on supported ports */
            if(port >= SNET_SIP5_EGR_MAX_PHY_PORTS_MAC(devObjPtr))
            {
                break;
            }
        }
        else
        {
            if(port > SNET_CHT_CPU_PORT_CNS)
            {
                break;
            }
        }

        if (destPorts[port] > 0)
        {
            outPorts ++;
        }
    }

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* count only the relevant to 'ingress' device */
        txqCountMode = TXQ_COUNTE_MODE_INGRESS_DEVICE_ONLY_E;
    }
    else
    {
        /* count all relevant to the device */
        txqCountMode = TXQ_COUNTE_MODE_ALL_E;
    }

    descrPtr->queue_dp       = descrPtr->dp;
    descrPtr->queue_priority = descrPtr->tc;
    __LOG_PARAM(descrPtr->queue_dp );
    __LOG_PARAM(descrPtr->queue_priority);

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        /* Bridge Egress Filtered packets counting */
        snetSip6EgfEftCounterSets(
            devObjPtr, descrPtr, destPorts,
            &pktFilteredPortsBmp, &forwardRestrictionBmp);
        /* Egress eVLAN Egress-Filter Pass/Drop CNC Client */
        snetCht3CncCount(descrPtr->ingressDevObjPtr, descrPtr,
                         SNET_SIP6_CNC_CLIENT_EGF_EVLAN_E, outPorts);
    }
    else
    {
        /* TXQ counter sets counting on the device */
        snetChtTxQCounterSets(devObjPtr, descrPtr, destPorts, destPortsLb,
              &pktFilteredPortsBmp, packetType, &forwardRestrictionBmp,
              txqCountMode,
              SMAIN_NOT_VALID_CNS/* not relevant*/);

        /* Egress VLAN Pass/Drop CNC Trigger - counting is done in block attached to */
        /* ingress device                                                            */
        snetCht3CncCount(descrPtr->ingressDevObjPtr, descrPtr,
                         SNET_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E, outPorts);
    }

    return outPorts;
}

/**
* @internal snetChtTxQSniffAndStcDuplicate function
* @endinternal
*
* @brief   Make copy of packet for egress mirror or STC.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - target port.
* @param[in] txSampled                -  (0 - mirror , 1 - STC)
* @param[in] egressTagged             - tagged/untagged port.
*/
static void snetChtTxQSniffAndStcDuplicate
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 egressPort,
    IN GT_U32 txSampled,
    IN GT_U32 egressTagged
)
{
    DECLARE_FUNC_NAME(snetChtTxQSniffAndStcDuplicate);

    SKERNEL_FRAME_CHEETAH_DESCR_STC * txSniffDescPtr;/* mirror descriptor */
    GT_U32  origPipeId;/* support multi-pipe device */

    /* duplicate descriptor from the ingress core */
    txSniffDescPtr = snetChtEqDuplicateDescr(descrPtr->ingressDevObjPtr,descrPtr);

    txSniffDescPtr->trgTagged = egressTagged ;
    txSniffDescPtr->txSampled = txSampled;

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        txSniffDescPtr->egressTrgPort = egressPort;/* this is global port */

        if(txSniffDescPtr->useVidx)
        {
            /* save the target phy port of the orig packet used by HA for access tables 'phy port 1' ' eport 1' */
            txSniffDescPtr->eArchExtInfo.srcTrgEPort = egressPort;
        }
        else
        {
            /* save the target eport of the orig packet used by HA for access tables 'phy port 1' ' eport 1' */
            txSniffDescPtr->eArchExtInfo.srcTrgEPort = descrPtr->trgEPort;
        }
    }
    else
    {
        /* Convert egress local port to global port on TxQ mirroring */
        txSniffDescPtr->egressTrgPort =
                SMEM_CHT_GLOBAL_PORT_FROM_LOCAL_PORT_MAC(devObjPtr, egressPort);
    }

    if(devObjPtr->dualDeviceIdEnable.txq)
    {
        /* the TX mirroring port need to use range 0..127 and not 0..63 */
        SMEM_U32_SET_FIELD(txSniffDescPtr->ownDev,0,1,
            SMEM_U32_GET_FIELD(devObjPtr->portGroupId,2,1));
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

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        if(devObjPtr->portGroupSharedDevObjPtr)
        {
            /* the TXQ send the descriptor to the EQ of the egress core */
            __LOG(("the TXQ send the descriptor to the EQ of the egress core \n"));
        }
        else
        {
            __LOG(("the TXQ send the descriptor to the EQ \n"));
        }

        /* support multi-pipe device */
        origPipeId = smemGetCurrentPipeId(devObjPtr);
        if(devObjPtr->numOfPipes)
        {
            __LOG(("the TXQ send the 'Tx mirror' descriptor to the EQ of pipe[%d] according to EGRESS DQ \n",
                origPipeId));
        }

        snetChtEqTxMirror (devObjPtr, txSniffDescPtr);

        /* restore pipeId ... as the EQ may send the packet to other egress pipe ! */
        smemSetCurrentPipeId(devObjPtr,origPipeId);
    }
    else
    {
        if(devObjPtr->portGroupSharedDevObjPtr)
        {
            /* the TXQ send the descriptor back to the EQ of the ingress core */
            __LOG(("the TXQ send the descriptor back to the EQ of the ingress core \n"));
        }
        else
        {
            __LOG(("the TXQ send the descriptor to the EQ \n"));
        }
        snetChtEqTxMirror (/*devObjPtr*//**/txSniffDescPtr->ingressDevObjPtr/**/,txSniffDescPtr);
    }

    /* Update TX mirrored flag in packet descriptor */
    __LOG(("Update TX mirrored flag in packet descriptor \n"));
/*    descrPtr->txMirrorDone = txSniffDescPtr->txMirrorDone;*/
    descrPtr->bmpsEqUnitsGotTxqMirror = txSniffDescPtr->bmpsEqUnitsGotTxqMirror;
}

/**
* @internal snetChtTxQStc function
* @endinternal
*
* @brief   egress STC
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - target port.
* @param[in] egressTagged             - tagged/untagged port.
*                                       None
*
* @note make function similar to snetChtEqStc(...) of ingress STC
*
*/
static void snetChtTxQStc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 egressPort,
    IN GT_U32 egressTagged
)
{
    DECLARE_FUNC_NAME(snetChtTxQStc);

    GT_U32 regAddr;                 /* Register address */
    GT_U32 * regPtr;                /* Register entry pointer */
    GT_U32  egrStcCntrl;            /* content of egress filtering register */
    GT_U32 stcCnt,newStcCnt;        /* Countdown port counter */
    GT_U32  stcLimitRegData;        /* Egress STC table */
    GT_BOOL  doStc = GT_FALSE;      /* Do egress sampling to CPU */
    GT_U32  newLimValRdy;           /*Port<n>Egress STC New Limit Value Ready*/
    GT_U32 fieldVal;                /* Register field value */
    GT_U32         outputPortBit;  /* the bit index for the egress port  */
    GT_BOOL        isSecondRegister;/* is the per port config is in second register -- for bmp of ports */
    GT_U32  reloadMode;             /*reload mode*/
    GT_BOOL loadNewValue = GT_FALSE;/*load new value */
    GT_U32  stcAnalyzerIndex;
    GT_U32  bitIndex; /*bit index */
    GT_BIT  supportInterrupt = 0;/* indication that support interrupt. sip5.10 and above do not do interrupt in continuous mode */
    GT_U32  dqUnitInPipe;/* the DP unit in the pipe that handle packet */
    GT_U32  localTxqPortNum;/* local DQ port number to the 'PIPE' */
    GT_U32  txqPortNum;/* global DQ port number to the 'PIPE' */

    dqUnitInPipe = descrPtr->egressPhysicalPortInfo.egressTxqDqIndexId;

    if(devObjPtr->txqRevision == 0)
    {
        outputPortBit = egressPort;
    }
    else
    {
        snetChtTxqDqPerPortInfoGet(devObjPtr,descrPtr,egressPort,&isSecondRegister,&outputPortBit);

        if(devObjPtr->multiDataPath.txqDqNumPortsPerDp)
        {
            txqPortNum = outputPortBit;

            /* get new (local) txqPortNum */
            localTxqPortNum = txqPortNum % devObjPtr->multiDataPath.txqDqNumPortsPerDp;

            outputPortBit = localTxqPortNum;
        }
    }

    regAddr = SMEM_CHT_STAT_EGRESS_MIRROR_REG(devObjPtr,dqUnitInPipe);
    smemRegGet (devObjPtr , regAddr , &egrStcCntrl );
    /* Egress STC Enable */
    fieldVal = SMEM_U32_GET_FIELD(egrStcCntrl, 0, 1);
    if (fieldVal && (egressPort != SNET_CHT_CPU_PORT_CNS))
    {
        regAddr = SMEM_CHT_STC_TABLE0_REG(devObjPtr, outputPortBit,dqUnitInPipe);
        regPtr = smemMemGet(devObjPtr, regAddr);

        /* Port<n> Egress STC Table Entry Word0  */
        stcLimitRegData = regPtr[0];

        /* Egress STC Reload Mode */
        reloadMode = SMEM_U32_GET_FIELD(egrStcCntrl, 1, 1);

        /* SIP 5.10 and above generates interrupts for Triggered mode only */
        supportInterrupt = (SMEM_CHT_IS_SIP5_10_GET(devObjPtr) && (reloadMode == 0)) ? 0 : 1;

        /* Port<n> Egress STC New Limit Value Ready */
        newLimValRdy = SMEM_U32_GET_FIELD(stcLimitRegData, 30, 1);

        /* Port<n> Egress STC Counter */
        stcCnt = SMEM_U32_GET_FIELD(regPtr[1], 0, 30);
        if (stcCnt)
        {
            if ((--stcCnt) == 0)
            {
                /* in this case we not reload new value ! */
                __LOG(("stcCnt == 0 (after decrement) No reload of new value \n"));
                __LOG(("state that Egress STC is needed \n"));
                doStc = GT_TRUE;
            }
        }
        else /*stcCnt == 0*/
        {
            if(newLimValRdy)
            {
                /* about : <newLimValRdy> : when the countdown counter at 0 , but the <newLimValRdy> is 1 ,
                   this is the time to load the new counter from word0 into word1 */
                loadNewValue = GT_TRUE;
            }
            else if(reloadMode == 0)/*continuous*/
            {
                loadNewValue = GT_TRUE;
            }
            __LOG(("stcCnt == 0 , so reload new value \n"));
        }

        if(loadNewValue == GT_TRUE)
        {
            /* get the new count Port<n>egress STC Limit */
            newStcCnt = SMEM_U32_GET_FIELD(stcLimitRegData, 0, 30);
            __LOG(("got the new count Port<n>egress STC Limit [0x%x] \n",
                newStcCnt));
            if(newStcCnt)
            {
                __LOG(("egress STC:the device load the new value and also decrement (due to current packet) \n"));

                if ((--newStcCnt) == 0)
                {
                    /* take care of rate value 1 case. Each packet goes to CPU */
                    __LOG(("state that Egress STC is needed \n"));
                    doStc = GT_TRUE;
                }

                /* Generate interrupt "Egress STC Interrupt Cause Register" */
                if(supportInterrupt == 0)
                {
                    /* SIP5.10 and above devices do not generate interrupts in continuous mode */
                    __LOG(("Load new limit but without generate interrupt \n"));
                }
                else if(devObjPtr->myInterruptsDbPtr)
                {
                    if(SMEM_CHT_IS_SIP5_15_GET(devObjPtr) == 0)
                    {
                        bitIndex = (outputPortBit % 18) + 1;
                    }
                    else
                    {
                        bitIndex = (outputPortBit % 31) + 1;
                    }
                    snetChetahDoInterrupt(devObjPtr,
                                          SMEM_CHT_STC_INT_CAUSE_REG(devObjPtr,outputPortBit,dqUnitInPipe),
                                          SMEM_CHT_STC_INT_MASK_REG(devObjPtr,outputPortBit,dqUnitInPipe),
                                          (1 << bitIndex),/* bmp of the bitIndex in SMEM_CHT_INGRESS_STC_INT_CAUSE_REG() */
                                          SMEM_CHT_TXQ_ENGINE_INT(devObjPtr));
                }
                else
                {
                    /* Interrupt Cause register STC bit */                            SIM_TBD_BOOKMARK
                    smemRegFldSet(devObjPtr, SMEM_CHT_STC_INT_CAUSE_REG(devObjPtr,0,dqUnitInPipe), (outputPortBit+1) %32, 1, 1);
                    /* Get summary bit */                                             SIM_TBD_BOOKMARK
                    smemRegFldGet(devObjPtr, SMEM_CHT_STC_INT_MASK_REG (devObjPtr,0,dqUnitInPipe), (outputPortBit+1) %32, 1, &fieldVal);
                    if(fieldVal)
                    {
                        /* Sum of all STC interrupts */
                        __LOG(("Sum of all STC interrupts"));
                        smemRegFldSet(devObjPtr, SMEM_CHT_STC_INT_CAUSE_REG(devObjPtr,0,dqUnitInPipe), 0, 1, 1);
                    }

                    /* Call interrupt */
                    snetChetahDoInterruptLimited(devObjPtr,
                                       SMEM_CHT_TX_QUEUE_INTERRUPT_CAUSE_REG(devObjPtr,dqUnitInPipe),
                                       SMEM_CHT_TX_QUEUE_INTERRUPT_MASK_REG(devObjPtr,dqUnitInPipe),
                                       SMEM_CHT_EGR_STC_SUM_INT(devObjPtr),
                                       SMEM_CHT_TXQ_ENGINE_INT(devObjPtr),
                                       fieldVal);
                }
            }

            /* since we reload new limit -->
               need to update EgressSTCNewLimValRdy regardless to trigger/continuous mode */
            SMEM_U32_SET_FIELD(regPtr[0], 30, 1, 0);
        }
        else
        {
            newStcCnt = stcCnt;
        }

        /* Set Port<n>Egress STC Counter */
        SMEM_U32_SET_FIELD(regPtr[1], 0, 30, newStcCnt);

        smemMemSet(devObjPtr, regAddr, regPtr, 3);

    }

    if(doStc == GT_TRUE)
    {
        /* add option to do egress STC */
        descrPtr->mirrorType |= SKERNEL_MIRROR_TYPE_STC_CNS;

        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            regAddr = SMEM_LION3_TXQ_DQ_STC_ANALYZER_INDEX_REG(devObjPtr,dqUnitInPipe);
            smemRegFldGet(devObjPtr,regAddr,0,4,&stcAnalyzerIndex);

            __LOG_PARAM(stcAnalyzerIndex);
            if(stcAnalyzerIndex >= descrPtr->analyzerIndex)
            {
                /* remove the option to do egress mirror */
                descrPtr->mirrorType &= ~SKERNEL_MIRROR_TYPE_MIRROR_CNS;

                if(descrPtr->analyzerIndex)
                {
                    __LOG(("NOTE: egress STC Analyzer Index [%d] higher then egress mirror index [%d] , so ONLY egress STC is replicated \n",
                        stcAnalyzerIndex,
                        descrPtr->analyzerIndex));
                }
                else
                {
                    /* egress STC with out egress mirror */
                }
            }
            else
            {
                /* remove the option to do egress STC */
                descrPtr->mirrorType &= ~SKERNEL_MIRROR_TYPE_STC_CNS;

                __LOG(("WARNING : egress STC Analyzer Index lower then egress mirror reasons , so the egress STC is NOT replicated (only egress mirror) \n"));

                doStc = GT_FALSE;
            }
        }
        else
        {
            /* remove the option to do egress mirror */
            descrPtr->mirrorType &= ~SKERNEL_MIRROR_TYPE_MIRROR_CNS;

            if(descrPtr->analyzerIndex)
            {
                __LOG(("NOTE: egress STC has priority over egress mirror index [%d] , so ONLY egress STC is replicated \n",
                    descrPtr->analyzerIndex));
            }
        }
    }

    if (doStc == GT_TRUE)
    {

        __LOG(("Do egress STC \n"));
        snetChtTxQSniffAndStcDuplicate(devObjPtr,descrPtr,egressPort,1/*txSampled*/,egressTagged);

        /* update the sampled to CPU counter */
        regAddr = SMEM_CHT_STC_TABLE2_REG(devObjPtr, outputPortBit,dqUnitInPipe);
        smemRegFldGet(devObjPtr , regAddr, 0, 16, &fieldVal);
        fieldVal++;
        smemRegFldSet(devObjPtr, regAddr, 0 , 16, fieldVal);
    }
    else
    {
        __LOG(("NOT Doing egress STC \n"));
    }
}
/**
* @internal snetChtTxQSniffAndStc function
* @endinternal
*
* @brief   egress mirroring and egress STC
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - target port.
* @param[in] egressTagged             - tagged/untagged port.
*/
static void snetChtTxQSniffAndStc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 egressPort,
    IN GT_U32 egressTagged
)
{
    DECLARE_FUNC_NAME(snetChtTxQSniffAndStc);

    GT_U32  *memPtr;                /* pointer to memory */
    GT_U32  regAddr;                /* address of registers */
    GT_U32  txSniffEn;              /* Enable/Disable to analyser port */
    GT_U32  statRatio;              /* Statistic mirroring to analyser port */
    GT_U32  EgressCTMEn;            /* Enables CPU-triggered egress mirroring*/
    GT_U32  EgressCTMPort;          /* The port on which CPU-triggered egress mir*/
    GT_U32  EgressCTMTrigger;       /* The bit triggers the egress mirroing */
    GT_U32  fldVal;                 /* field value */
    GT_U32* txSniffCntrPtr;         /* tx sniffer counter  */
    GT_U32         outputPortBit;  /* the bit index for the egress port  */
    GT_BOOL        isSecondRegister;/* is the per port config is in second register -- for bmp of ports */
    GT_U32  egrStcCntrl;            /* content of egress filtering register */
    GT_U32  value;
    GT_U32  dqUnitInPipe;/* the DP unit in the pipe that handle packet */

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        skernelFatalError("SIP6 : logic moved to function 'sip6EgressMirror' \n");
        return;
    }

    dqUnitInPipe = descrPtr->egressPhysicalPortInfo.egressTxqDqIndexId;

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
        {
            regAddr = SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTE_TBL_MEM(devObjPtr,egressPort);
            memPtr = smemMemGet(devObjPtr, regAddr);

            value = SMEM_SIP5_20_EGF_QAG_PORT_TARGET_FIELD_GET(devObjPtr,memPtr,egressPort,
                SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_TARGET_PHYSICAL_PORT_EVLAN_MIRRORING_ENABLE_E);
        }
        else
        {
            regAddr = SMEM_LION3_EGF_QAG_EVLAN_MIRR_ENABLE_REG(devObjPtr);
            memPtr = smemMemGet(devObjPtr, regAddr);
            value = snetFieldValueGet(memPtr,egressPort,1);
        }

        if(0 == value)
        {
            if(descrPtr->analyzerIndex)
            {
                __LOG(("EGF_QAG: the eVlan mirroring is not enabled on egress port[%d] although the vlan analyzer index is[%d] (so ignoring the 'Egress Vlan mirroring'!) \n",
                    egressPort,
                    descrPtr->analyzerIndex));
            }

            /* the port is not allowed to send 'eVlan mirroring' */
            descrPtr->analyzerIndex = 0;
        }

        /* 8 ports in register */
        regAddr = SMEM_LION_TXQ_EGR_ANALYZER_EN_REG(devObjPtr,(egressPort/8),dqUnitInPipe);
        /* 3 bits per port */
        smemRegFldGet(devObjPtr , regAddr, (egressPort % 8) * 3, 3, &txSniffEn);

        if(txSniffEn)
        {
            __LOG(("TXQ_DQ: Per egress physical port [%d] - Mirror To Analyzer Index [%d] \n",
                egressPort,
                txSniffEn));
        }

        /*Mirror To Analyzer Index*/
        snetXcatEgressMirrorAnalyzerIndexSelect(devObjPtr,descrPtr,txSniffEn);

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

        /* check if any of egress mirror reasons enabled */
        txSniffEn = descrPtr->analyzerIndex ? 1 : 0;
    }
    else
    if(devObjPtr->txqRevision == 0)
    {
        outputPortBit = egressPort;
        regAddr = SMEM_CHT_TXQ_CONFIG_REG(devObjPtr, outputPortBit);
        smemRegFldGet(devObjPtr , regAddr, 20, 1, &txSniffEn);
    }
    else
    {
        snetChtTxqDqPerPortInfoGet(devObjPtr,descrPtr,egressPort,&isSecondRegister,&outputPortBit);

        regAddr = SMEM_LION_TXQ_EGR_ANALYZER_EN_REG(devObjPtr,0/*dummy parameter*/,dqUnitInPipe);
        if(isSecondRegister == GT_TRUE)
        {
            regAddr += 0x4;
        }
        smemRegFldGet(devObjPtr , regAddr, outputPortBit, 1, &txSniffEn);
    }

    descrPtr->mirrorType = 0;

    /* check egress STC before mirroring as it may have priority over the mirroring */
    /* need to check if egress STC hold priority over the egress mirror */
    /* NOTE: function snetChtTxQStc may remove the flag SKERNEL_MIRROR_TYPE_MIRROR_CNS */
    __LOG(("check egress STC \n"));
    snetChtTxQStc(devObjPtr,descrPtr,egressPort,egressTagged);


    txSniffCntrPtr = CHT_INTERNAL_MEM_PTR(devObjPtr,CHT_INTERNAL_SIMULATION_USE_MEM_TX_STATISTICAL_SNIFF_COUNTER_E);

    regAddr = SMEM_CHT_STAT_EGRESS_MIRROR_REG(devObjPtr,dqUnitInPipe);
    smemRegGet (devObjPtr , regAddr , &egrStcCntrl );
    if (txSniffEn)
    {
        __LOG(("tx Sniff Enabled \n"));

        if (egrStcCntrl & (1 << 4))/*egress stat mirror to analyzer enable bit*/
        {
            __LOG(("Statistical Mirroring Enabled \n"));

            statRatio = (egrStcCntrl & 0xFFE0) >> 5; /* bits[5:15] */
            if (statRatio)
            {
                (*txSniffCntrPtr)++;
                if ((*txSniffCntrPtr) % statRatio)
                {
                    txSniffEn = 0 ;
                }
            }
            else
            {
                txSniffEn = 0 ;
                (*txSniffCntrPtr)++;
            }

            if(txSniffEn == 0)
            {
                __LOG(("Statistical Mirroring not reached (%d out of %d) \n",
                    ((*txSniffCntrPtr) - 1) , statRatio));
            }
        }
    }

    if(0 == SMEM_CHT_IS_SIP5_GET(devObjPtr))/* what is the 'Analyzer index' for this feature ? */
    {
        /*  CPU-Triggered Egress Mirroring
            CPU-triggered egress mirroring allows the CPU to trigger a specified port on the device to mark the
            next packet enqueued for transmission on the port for egress mirroring. The packet is duplicated by
            the pre-egress engine and sent to the destination egress analyzer port */
        EgressCTMEn = (egrStcCntrl >> 17) & 0x1;
        EgressCTMTrigger = (egrStcCntrl >> 16) & 0x1;
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /* 7 bits */
            EgressCTMPort = (egrStcCntrl >> 18) & 0x7F;
        }
        else
        {
            EgressCTMPort = (egrStcCntrl >> 18) & 0x3F;
        }

        if ( (EgressCTMEn) &&
             (EgressCTMPort == egressPort) &&
             (EgressCTMTrigger == 1) )
        {
            __LOG(("CPU-Triggered Egress Mirroring triggered \n"));

            txSniffEn = 1;
            egrStcCntrl = (egrStcCntrl & (~(1 << 16)));/*turn off bit 16*/
            smemRegGet (devObjPtr , regAddr , &egrStcCntrl );
        }
    }

    if (txSniffEn == 1)
    {
        if(descrPtr->mirrorType & SKERNEL_MIRROR_TYPE_STC_CNS)
        {
            __LOG(("already did egress STC , so no more replications \n"));
        }
        else
        {
            __LOG(("Do egress Mirroring \n"));
            snetChtTxQSniffAndStcDuplicate(devObjPtr,descrPtr,egressPort,0/*txSampled=0*/,egressTagged);
        }
    }

    return;
}

#define __LOG_LEGACY_TXQ_MIB_COUNTER(counterName,setId,oldValue,addedValue) \
    __LOG(("TXQ mib counter [%s] of set[%d] , incremented by [0x%x] from value of [0x%8.8x] \n", \
        counterName , setId , addedValue , oldValue))

/**
* @internal snetChtTxQCounterSets function
* @endinternal
*
* @brief   Update egress counters
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] destPorts[]              - destination port vector.
* @param[in] destPortsLb[]            - valuable only if destPort[N] == 1.
*                                    destPortsLb[N] format is:
*                                    bit [31] - if the packet destined to
*                                    port N is instead forwarded (1) or not (0)
*                                    to a loopback/service port.
*                                    bits[30:0] - a loopback/service port number.
*                                    Is valuable only if bit 31 is set.
* @param[in] packetType               - type of packet.
* @param[in] forwardRestrictionBmpPtr - pointer to forwarding restrictions ports bitmap
* @param[in] egressPort               - egress port - relevant only when destPorts == NULL
*                                       None
*/
static void snetChtTxQCounterSets
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 destPorts[],
    IN GT_U32 destPortsLb[],
    IN SKERNEL_PORTS_BMP_STC *pktFilteredPortsBmpPtr,
    IN SKERNEL_EGR_PACKET_TYPE_ENT packetType,
    IN SKERNEL_PORTS_BMP_STC *forwardRestrictionBmpPtr,
    IN TXQ_COUNTE_MODE_ENT counteMode,
    IN GT_U32   egressPort
)
{
    DECLARE_FUNC_NAME(snetChtTxQCounterSets);

    GT_U32 counterRegAddr0,counterRegAddr1;/*address of the counters registers*/
    GT_U32 egrFltrCount0, egrFltrCount1; /* egress filtering counter */
    GT_U32 egrFwdRestr0, egrFwdRestr1; /* egress filtering forward restriction counter */
    GT_U32 outCount1 = 0, outCount0 = 0;/* egress counters */
    GT_U32 set1CntrlRegData , set0CntrlRegData; /* Txq MIB Counters config register */
    GT_U32 set1CntrlRegData_port , set0CntrlRegData_port; /* Txq MIB Counters port config register */
    /* The TC of the packets counted by this set*/
    GT_U32 Set0TcCntMode , Set0TC,Set1TcCntMode, Set1TC;
    /* The VID of the packets counted by this set*/
    GT_U32 Set0VlanCntMode ,Set1VID , Set0VID , Set1VlanCntMode;
    /* The egress port number of packets counted by this set*/
    GT_U32 Set0PortCntMode,Set0Port,Set1Port,Set1PortCntMode  ;
    /* The DP of the packets counted by this set*/
    GT_U32 Set0DpCntMode , Set0DP , Set1DpCntMode , Set1DP;
    GT_U32 port /* egress port number */;
    GT_U32 portInx; /* port iterator number */
    GT_U32 Counter ; /* counter */
    GT_BIT  countEgress = (counteMode == TXQ_COUNTE_MODE_ALL_E || counteMode == TXQ_COUNTE_MODE_EGRESS_DEVICE_ONLY_E) ? 1:0;
    GT_BIT  countIngress = (counteMode == TXQ_COUNTE_MODE_ALL_E || counteMode == TXQ_COUNTE_MODE_INGRESS_DEVICE_ONLY_E) ? 1:0;
    GT_BIT  egressDoCount ;/*indication to egress count */
    GT_U32  txqMibPort;/*TXQ mib port for 'eBrdige Egress Counter Set' */
    GT_U32  txqMibTc = descrPtr->tc;/*TXQ mib TC (not using descrPtr->queue_priority) */
    GT_U32  txqMibDp; /* value of the DP in the egress pipe */
    char*   counterNameStr="";

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        skernelFatalError("SIP6 : logic moved to function 'snetSip6EgfEftCounterSets' and to 'snetPreqCounterSets' \n");
        return;
    }

    if(devObjPtr->txqRevision || SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* Lion2 (and above) TQX supports 3 colors ! (according to behavior on Lion2 HW) */
        txqMibDp = descrPtr->queue_dp;

        if (SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
        {
            /* the TC after remap is used for SIP_5_20 and above */
            txqMibTc = descrPtr->queue_priority;
            __LOG(("SIP 5.20:  txqMibTc use queue_priority[%d] instead of tc[%d]\n", descrPtr->queue_priority, descrPtr->tc));
        }

        if(descrPtr->queue_dp != descrPtr->dp)
        {
            if (SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
            {
                __LOG(("The DP used for TXQ mib counters is the DP after the remap\n"));
            }
            else
            {
                /* could be in sip5 only as in Lion2 no DP remap */
                __LOG(("The DP used for TXQ mib counters is the DP after the remap (unlike TC)\n"));
            }
        }

        if(descrPtr->queue_priority != descrPtr->tc)
        {
            if (SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
            {
                __LOG(("The TC used for TXQ mib counters is the TC after the remap\n"));
            }
            else
            {
                __LOG(("The TC used for TXQ mib counters is the TC BEFORE the remap (unlike DP) \n"));
            }
        }

    }
    else
    {
        /* get DP of egress pipe.
           In the CH3 and above devices DP in the ingress pipe has 3 values.
           But Egress operates only with 2 DP levels:
           For CH3 - Green and Yellow. Ingress Red and Green values are handled as Green.
           For CH1 - Green and Red */
        if (SKERNEL_IS_CHEETAH3_DEV(devObjPtr))
        {
            txqMibDp = (descrPtr->dp == 1) ? 1 : 0;
        }
        else
        {
            txqMibDp = (descrPtr->dp == 0) ? 0 : 1;
        }
    }

    __LOG_PARAM(txqMibTc);
    __LOG_PARAM(txqMibDp);

    smemRegGet(devObjPtr,
               SMEM_CHT_TXQ_MIB_COUNTERS_SET0_CONFIG_REG(devObjPtr),
               &set0CntrlRegData);

    smemRegGet(devObjPtr,
               SMEM_CHT_TXQ_MIB_COUNTERS_SET1_CONFIG_REG(devObjPtr),
               &set1CntrlRegData);

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        smemRegGet(devObjPtr,
                   SMEM_LION3_TXQ_MIB_COUNTERS_PORT_CONFIG_SET_N_CONFIG_REG(devObjPtr,0),
                   &set0CntrlRegData_port);

        smemRegGet(devObjPtr,
                   SMEM_LION3_TXQ_MIB_COUNTERS_PORT_CONFIG_SET_N_CONFIG_REG(devObjPtr,1),
                   &set1CntrlRegData_port);

        /*Set 0*/
        Set0TcCntMode   = SMEM_U32_GET_FIELD(set0CntrlRegData,2,1);
        Set0TC          = SMEM_U32_GET_FIELD(set0CntrlRegData,26,3);
        Set0VlanCntMode = SMEM_U32_GET_FIELD(set0CntrlRegData,1,1);
        Set0VID         = SMEM_U32_GET_FIELD(set0CntrlRegData,10,16);
        Set0DpCntMode   = SMEM_U32_GET_FIELD(set0CntrlRegData,3,1);
        Set0DP          = SMEM_U32_GET_FIELD(set0CntrlRegData,29,2);

        Set0PortCntMode = SMEM_U32_GET_FIELD(set0CntrlRegData_port,0,1);
        Set0Port        = SMEM_U32_GET_FIELD(set0CntrlRegData_port,4,17);

        /*Set 1*/
        Set1TcCntMode   = SMEM_U32_GET_FIELD(set1CntrlRegData,2,1);
        Set1TC          = SMEM_U32_GET_FIELD(set1CntrlRegData,26,3);
        Set1VlanCntMode = SMEM_U32_GET_FIELD(set1CntrlRegData,1,1);
        Set1VID         = SMEM_U32_GET_FIELD(set1CntrlRegData,10,16);
        Set1DpCntMode   = SMEM_U32_GET_FIELD(set1CntrlRegData,3,1);
        Set1DP          = SMEM_U32_GET_FIELD(set1CntrlRegData,29,2);

        Set1PortCntMode = SMEM_U32_GET_FIELD(set1CntrlRegData_port,0,1);
        Set1Port        = SMEM_U32_GET_FIELD(set1CntrlRegData_port,4,17);


    }
    else
    {
        Set0TcCntMode   = (set0CntrlRegData & 0x4) >> 2;
        Set0TC          = (set0CntrlRegData >> 22) & 0x7;
        Set0VlanCntMode = (set0CntrlRegData & 0x2) >> 1;
        Set0VID         = (set0CntrlRegData >> 10) & 0xfff;
        Set0PortCntMode = (set0CntrlRegData & 0x1);
        Set0Port        = (set0CntrlRegData >> 4) & 0x3f;
        Set0DpCntMode   = (set0CntrlRegData & 0x8) >> 3;
        Set0DP          = (set0CntrlRegData >> 25) & (devObjPtr->txqRevision ? 0x3 : 0x1);/*allow 1 or 2 bits*/
        Set1TcCntMode   = (set1CntrlRegData & 0x4) >> 2;
        Set1TC          = (set1CntrlRegData >> 22) & 0x7;
        Set1VlanCntMode = (set1CntrlRegData & 0x2) >> 1;
        Set1VID         = (set1CntrlRegData >> 10) & 0xfff;
        Set1PortCntMode = (set1CntrlRegData & 0x1);
        Set1Port        = (set1CntrlRegData >> 4) & 0x3f;
        Set1DpCntMode   = (set1CntrlRegData & 0x8) >> 3;
        Set1DP          = (set1CntrlRegData >> 25) & (devObjPtr->txqRevision ? 0x3 : 0x1);/*allow 1 or 2 bits*/
    }

    if(simLogIsOpenFlag)
    {
        __LOG_PARAM(Set0TcCntMode      );
        __LOG_PARAM(Set0TC             );
        __LOG_PARAM(Set0VlanCntMode    );
        __LOG_PARAM(Set0VID            );
        __LOG_PARAM(Set0PortCntMode    );
        __LOG_PARAM(Set0Port           );
        __LOG_PARAM(Set0DpCntMode      );
        __LOG_PARAM(Set0DP             );
        __LOG_PARAM(Set1TcCntMode      );
        __LOG_PARAM(Set1TC             );
        __LOG_PARAM(Set1VlanCntMode    );
        __LOG_PARAM(Set1VID            );
        __LOG_PARAM(Set1PortCntMode    );
        __LOG_PARAM(Set1Port           );
        __LOG_PARAM(Set1DpCntMode      );
        __LOG_PARAM(Set1DP             );
    }

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* the ports are global and not need conversion */
    }
    else
    if(devObjPtr->portGroupSharedDevObjPtr && devObjPtr->txqRevision)
    {
        /* lion B :
           in simulation this egress port group see only it's local ports , but the
           field of Set0Port is 'global port'.
           so need to check that this port belongs to this port group or not.
        */

        if(Set0PortCntMode && (Set0Port != SNET_CHT_CPU_PORT_CNS))
        {
            if(SMEM_CHT_PORT_GROUP_ID_FROM_GLOBAL_PORT_MAC(devObjPtr,Set0Port) ==
               SMEM_CHT_PORT_GROUP_ID_MASK_CORE_MAC(devObjPtr, devObjPtr->portGroupId, devObjPtr->dualDeviceIdEnable.txq))
            {
                Set0Port = SMEM_CHT_LOCAL_PORT_FROM_GLOBAL_PORT_MAC(devObjPtr,Set0Port);
            }
            else
            {
                /* make sure not match in this port group */
                Set0Port =  SNET_CHT_NULL_PORT_CNS;
            }
        }

        if(Set1PortCntMode && (Set1Port != SNET_CHT_CPU_PORT_CNS))
        {
            if(SMEM_CHT_PORT_GROUP_ID_FROM_GLOBAL_PORT_MAC(devObjPtr,Set1Port) ==
               SMEM_CHT_PORT_GROUP_ID_MASK_CORE_MAC(devObjPtr, devObjPtr->portGroupId, devObjPtr->dualDeviceIdEnable.txq))
            {
                Set1Port = SMEM_CHT_LOCAL_PORT_FROM_GLOBAL_PORT_MAC(devObjPtr,Set1Port);
            }
            else
            {
                /* make sure not match in this port group */
                Set1Port =  SNET_CHT_NULL_PORT_CNS;
            }
        }

    }

    /* packet counted by egress ports for regular counters          */
    /* packet counted by excluded egress ports for filtered counter */
    port      = 0;
    outCount0 = 0;
    outCount1 = 0;
    egrFltrCount0 = 0;
    egrFltrCount1 = 0;
    egrFwdRestr0 = 0;
    egrFwdRestr1 = 0;

    if(countEgress && destPorts == NULL)/* SIP5 only */
    {
        if(descrPtr->useVidx == 0 && descrPtr->outGoingMtagCmd != SKERNEL_MTAG_CMD_TO_CPU_E)
        {
            /* logic taken from GM */
            txqMibPort = descrPtr->trgEPort;
            __LOG(("TXQ Mib Port is the target ePort [0x%x] \n" ,
                txqMibPort));
        }
        else
        {
            txqMibPort = egressPort;
            __LOG(("TXQ Mib Port is the <Local Dev Trg Phy Port> [0x%x] \n" ,
                txqMibPort));
        }

        if (!((Set0PortCntMode == 1 ) && (Set0Port != txqMibPort)))
        {
            outCount0 ++;
        }

        if (!((Set1PortCntMode == 1 ) && (Set1Port != txqMibPort)))
        {
            outCount1 ++;
        }

        snetLionResourceHistogramCount(devObjPtr, descrPtr);
    }
    else
    {
        for(portInx = 0 ; portInx < SKERNEL_DEV_EGRESS_MAX_PORT_CNS(devObjPtr); portInx++)
        {
            port = destPortsLb ?
                CHOOSE_TRG_OR_LB_PORT_MAC(portInx, destPortsLb[portInx]) :
                portInx;

            if(countEgress)
            {
                egressDoCount = 0;
                if (destPorts[portInx])
                {
                    egressDoCount = 1;
                }

                if(egressDoCount)
                {
                    if (!((Set0PortCntMode == 1 ) && (Set0Port != port)))
                    {
                        outCount0 ++;
                    }

                    if (!((Set1PortCntMode == 1 ) && (Set1Port != port)))
                    {
                        outCount1 ++;
                    }
                    snetLionResourceHistogramCount(devObjPtr, descrPtr);
                }
            }

            if(countIngress)
            {
                if (pktFilteredPortsBmpPtr &&
                    SKERNEL_PORTS_BMP_IS_PORT_SET_MAC(pktFilteredPortsBmpPtr,port))
                {
                    /* the egress port related Counter Set condition not relevant */
                    /* to filtered packet - it has no destination port            */
                    egrFltrCount0 ++;
                    egrFltrCount1 ++;
                }

                if(forwardRestrictionBmpPtr &&
                   SKERNEL_PORTS_BMP_IS_PORT_SET_MAC(forwardRestrictionBmpPtr, port))
                {
                    /* update egress forward restriction packet counters */
                    egrFwdRestr0++;
                    egrFwdRestr1++;
                }
            }

            if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
            {
                /* all ports exists and not need skip any of them */
                if(port >= SNET_SIP5_EGR_MAX_PHY_PORTS_MAC(devObjPtr))
                {
                    break;
                }
            }
            else
            if (port == SNET_CHEETAH_MAX_PORT_NUM(devObjPtr))
            {
                port = (SNET_CHT_CPU_PORT_CNS - 1) ;  /* skip not existed port */
            }
            else if(port == SNET_CHT_CPU_PORT_CNS)
            {
                /* no more ports in legacy devices */
                break;
            }
        }
    }

    /* check Set0 not-port conditions and clearing counter */
    if ( (Set0TcCntMode == 1) && (txqMibTc != Set0TC) )
    {
        egrFltrCount0 =  outCount0 = egrFwdRestr0 = 0;
    }
    else  if ( (Set0DpCntMode == 1) && (txqMibDp != Set0DP) )
    {
        egrFltrCount0 = outCount0 = egrFwdRestr0 = 0;
    }
    else  if ( (Set0VlanCntMode == 1) && (descrPtr->eVid != Set0VID) )
    {
        egrFltrCount0 = outCount0 = egrFwdRestr0 = 0;
    }

    /* check Set1 not-port conditions and clearing counter */
    if ( (Set1TcCntMode == 1) && (txqMibTc != Set1TC) )
    {
        egrFltrCount1 =  outCount1 = egrFwdRestr1 = 0;
    }
    else  if ( (Set1DpCntMode == 1) && (txqMibDp != Set1DP) )
    {
        egrFltrCount1 = outCount1 = egrFwdRestr1 = 0;
    }
    else  if ( (Set1VlanCntMode == 1) && (descrPtr->eVid != Set1VID) )
    {
        egrFltrCount1 = outCount1 = egrFwdRestr1 = 0;
    }

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* Reg is turned to Internal in SIP5, because the EGF no longer drops
           packets in the "egf distributor" stage (the signal egf2txq_egr_fwd_clr_ is always OFF) */
        if(egrFwdRestr0 || egrFwdRestr1)
        {
            /* The <Egress Forward Restricted Dropped Pkts> will not count packets in SIP 5,
               instead the packets that were counted in it will now be counted in the <Bridge Egress Filtered> */
            __LOG(("The <Egress Forward Restricted Dropped Pkts> will not count packets "
                "instead the packets that were counted in it will now be counted in the <Bridge Egress Filtered> \n"));

            /* let <Bridge Egress Filtered> count it instead of <Egress Forward Restricted Dropped Pkts> */
            egrFltrCount0 += egrFwdRestr0;
            egrFltrCount1 += egrFwdRestr1;

            egrFwdRestr0 = 0;
            egrFwdRestr1 = 0;
        }
    }

    if(countEgress && (outCount0 || outCount1))
    {
        if ( (packetType == SKERNEL_EGR_PACKET_CNTRL_UCAST_E) ||
             (packetType == SKERNEL_EGR_PACKET_CNTRL_MCAST_E) )
        {
            /* update control packets counter */
            __LOG(("update control packets counter"));
            counterRegAddr0 = SMEM_CHT_SET0_CONTROL_PACKET_CNTR_REG(devObjPtr);
            counterRegAddr1 = SMEM_CHT_SET1_CONTROL_PACKET_CNTR_REG(devObjPtr);

            counterNameStr = "setCtrlPktCntr";
        }
        else
        {
            if (descrPtr->macDaType == SKERNEL_UNICAST_MAC_E)
            {
                /* update unicast packet counter */
                __LOG(("update unicast packet counter"));
                counterRegAddr0 = SMEM_CHT_SET0_UNICAST_PACKET_CNTR_REG(devObjPtr);
                counterRegAddr1 = SMEM_CHT_SET1_UNICAST_PACKET_CNTR_REG(devObjPtr);

                counterNameStr = "setOutgoingUcPktCntr";
            }
            else if (descrPtr->macDaType == SKERNEL_MULTICAST_MAC_E)
            {
                /* update multicast packet counter */
                __LOG(("update multicast packet counter"));
                counterRegAddr0 = SMEM_CHT_SET0_MULTICAST_PACKET_CNTR_REG(devObjPtr);
                counterRegAddr1 = SMEM_CHT_SET1_MULTICAST_PACKET_CNTR_REG(devObjPtr);

                counterNameStr = "setOutgoingMcPktCntr";
            }
            else   /* update broadcast packet counter */
            {
                __LOG(("update broadcast packet counter"));
                counterRegAddr0 = SMEM_CHT_SET0_BROADCAST_PACKET_CNTR_REG(devObjPtr);
                counterRegAddr1 = SMEM_CHT_SET1_BROADCAST_PACKET_CNTR_REG(devObjPtr);

                counterNameStr = "setOutgoingBcPktCntr";
            }
        }

        if(outCount0)
        {
            smemRegGet(devObjPtr,counterRegAddr0, &Counter);
            __LOG_LEGACY_TXQ_MIB_COUNTER(counterNameStr,0,Counter,outCount0);
            Counter += outCount0;
            smemRegSet(devObjPtr,counterRegAddr0, Counter);
        }

        if(outCount1)
        {
            smemRegGet(devObjPtr,counterRegAddr1, &Counter);
            __LOG_LEGACY_TXQ_MIB_COUNTER(counterNameStr,1,Counter,outCount1);
            Counter += outCount1;
            smemRegSet(devObjPtr,counterRegAddr1, Counter);
        }
    }

    if(countIngress)
    {
        /* If a packet is filtered for more than one reason, the packet is counted only once
           (bridgeEgressFilteredPkts - highest precedence) */
        if(egrFltrCount0 || egrFltrCount1)
        {
            /* update egress filtered counter for any packets */
            __LOG(("update egress filtered counter for any packets"));
            counterRegAddr0 = SMEM_CHT_SET0_BRIDGE_FILTERED_CNTR_REG(devObjPtr);
            counterRegAddr1 = SMEM_CHT_SET1_BRIDGE_FILTERED_CNTR_REG(devObjPtr);

            counterNameStr = "setBridgeEgrFilteredPktCntr";

            if(egrFltrCount0)
            {
                smemRegGet(devObjPtr,counterRegAddr0, &Counter);
                __LOG_LEGACY_TXQ_MIB_COUNTER(counterNameStr,0,Counter,egrFltrCount0);
                Counter += egrFltrCount0;
                smemRegSet(devObjPtr,counterRegAddr0, Counter);
            }

            if(egrFltrCount1)
            {
                smemRegGet(devObjPtr,counterRegAddr1, &Counter);
                __LOG_LEGACY_TXQ_MIB_COUNTER(counterNameStr,1,Counter,egrFltrCount1);
                Counter += egrFltrCount1;
                smemRegSet(devObjPtr,counterRegAddr1, Counter);
            }

        }
        else
        if(egrFwdRestr0 || egrFwdRestr1)
        {
            /* update egress forward restriction packet counter */
            __LOG(("update egress forward restriction packet counter"));
            counterRegAddr0 = SMEM_CHT_SET0_EGR_FWD_RESTRICTIONS_DROPPED_PACKET_CNTR_REG(devObjPtr);
            counterRegAddr1 = SMEM_CHT_SET1_EGR_FWD_RESTRICTIONS_DROPPED_PACKET_CNTR_REG(devObjPtr);

            counterNameStr = "setEgrForwardingRestrictionDroppedPktsCntr";

            if(egrFwdRestr0)
            {
                smemRegGet(devObjPtr,counterRegAddr0, &Counter);
                __LOG_LEGACY_TXQ_MIB_COUNTER(counterNameStr,0,Counter,egrFwdRestr0);
                Counter += egrFwdRestr0;
                smemRegSet(devObjPtr,counterRegAddr0, Counter);
            }

            if(egrFwdRestr1)
            {
                smemRegGet(devObjPtr,counterRegAddr1, &Counter);
                __LOG_LEGACY_TXQ_MIB_COUNTER(counterNameStr,1,Counter,egrFwdRestr1);
                Counter += egrFwdRestr1;
                smemRegSet(devObjPtr,counterRegAddr1, Counter);
            }
        }
    }

    return;
}

#define __LOG_SIP6_EGF_EFT_MIB_COUNTER(counterName,setId,oldValue,addedValue) \
    __LOG(("EGF_EFT mib counter [%s] for set[%d], incremented by [0x%x] from value of [0x%8.8x] \n", \
        #counterName , setId ,addedValue , oldValue))

/**
* @internal snetSip6EgfEftCounterSets function
* @endinternal
*
* @brief   Update egress counters of EGF_EFT unit
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] destPorts[]              - egress ports map array.
* @param[in] pktFilteredPortsBmpPtr   - pointer to filtered ports bitma
* @param[in] forwardRestrictionBmpPtr - pointer to forwarding restrictions ports bitmap
*                                       None
*
* @note code taken from snetChtTxQCounterSets(...)
*
*/
static void snetSip6EgfEftCounterSets
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 destPorts[],
    IN SKERNEL_PORTS_BMP_STC *pktFilteredPortsBmpPtr,
    IN SKERNEL_PORTS_BMP_STC *forwardRestrictionBmpPtr
)
{
    DECLARE_FUNC_NAME(snetSip6EgfEftCounterSets);

    GT_U32 counterRegAddr0,counterRegAddr1;/*address of the counters registers*/
    GT_U32 outCount1 = 0, outCount0 = 0;/* egress counters */
    GT_U32 set1CntrlRegData , set0CntrlRegData; /* Txq MIB Counters config register */
    GT_U32 set1CntrlRegData_port , set0CntrlRegData_port; /* Txq MIB Counters port config register */
    /* The TC of the packets counted by this set*/
    GT_U32 Set0TcCntMode , Set0TC,Set1TcCntMode, Set1TC;
    /* The VID of the packets counted by this set*/
    GT_U32 Set0VlanCntMode ,Set1VID , Set0VID , Set1VlanCntMode;
    /* The egress port number of packets counted by this set*/
    GT_U32 Set0PortCntMode,Set0Port,Set1Port,Set1PortCntMode  ;
    /* The DP of the packets counted by this set*/
    GT_U32 Set0DpCntMode , Set0DP , Set1DpCntMode , Set1DP;
    GT_U32 Counter ; /* counter */
    GT_U32  txqMibTc;/* using descrPtr->tc */
    GT_U32  txqMibDp; /* value of the DP in the egress pipe */
    GT_U32  port;     /* work port variable for loops */
    GT_U32  egrFltrCount; /* amount of egress ports filtered the packet filtered from */
    GT_BOOL mcFltr0 = GT_FALSE; /* Filter flag of multicast packet for counter set 0 */
    GT_BOOL mcFltr1 = GT_FALSE; /* Filter flag of multicast packet for counter set 1 */

    txqMibDp = descrPtr->dp;
    txqMibTc = descrPtr->tc;
    __LOG((
           "SIP 6:  txqMibTc use tc[%d] dp[%d] instead of queue_priority[%d],queue_dp[%d] (unlike sip5.20)\n",
           descrPtr->tc,descrPtr->dp,descrPtr->queue_priority,descrPtr->queue_dp));
    if(descrPtr->queue_dp != descrPtr->dp)
    {
        __LOG(("The DP       used for EGF-EFT mib counters is the DP before the remap\n"));
    }
    if(descrPtr->queue_priority != descrPtr->tc)
    {
        __LOG(("The Priority used for EGF-EFT mib counters is the TC before the remap\n"));
    }

    __LOG_PARAM(txqMibTc);
    __LOG_PARAM(txqMibDp);

    smemRegGet(
        devObjPtr,
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.egrMIBCntrs.egrMIBCntrsSetConfig[0],
        &set0CntrlRegData);

    smemRegGet(
        devObjPtr,
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.egrMIBCntrs.egrMIBCntrsSetConfig[1],
        &set1CntrlRegData);

    smemRegGet(
        devObjPtr,
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.egrMIBCntrs.egrMIBCntrsPortSetConfig[0],
        &set0CntrlRegData_port);

    smemRegGet(
        devObjPtr,
        SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.egrMIBCntrs.egrMIBCntrsPortSetConfig[1],
        &set1CntrlRegData_port);

    /*Set 0*/
    Set0TcCntMode   = SMEM_U32_GET_FIELD(set0CntrlRegData,1,1);
    Set0TC          = SMEM_U32_GET_FIELD(set0CntrlRegData,16,3);
    Set0VlanCntMode = SMEM_U32_GET_FIELD(set0CntrlRegData,0,1);
    Set0VID         = SMEM_U32_GET_FIELD(set0CntrlRegData,3,13);
    Set0DpCntMode   = SMEM_U32_GET_FIELD(set0CntrlRegData,2,1);
    Set0DP          = SMEM_U32_GET_FIELD(set0CntrlRegData,19,2);

    Set0PortCntMode = SMEM_U32_GET_FIELD(set0CntrlRegData_port,0,1);
    Set0Port        = SMEM_U32_GET_FIELD(set0CntrlRegData_port,4,14);

    /*Set 1*/
    Set1TcCntMode   = SMEM_U32_GET_FIELD(set1CntrlRegData,1,1);
    Set1TC          = SMEM_U32_GET_FIELD(set1CntrlRegData,16,3 );
    Set1VlanCntMode = SMEM_U32_GET_FIELD(set1CntrlRegData,0,1);
    Set1VID         = SMEM_U32_GET_FIELD(set1CntrlRegData,3,13);
    Set1DpCntMode   = SMEM_U32_GET_FIELD(set1CntrlRegData,2,1);
    Set1DP          = SMEM_U32_GET_FIELD(set1CntrlRegData,19,2);

    Set1PortCntMode = SMEM_U32_GET_FIELD(set1CntrlRegData_port,0,1);
    Set1Port        = SMEM_U32_GET_FIELD(set1CntrlRegData_port,4,14);

    if(simLogIsOpenFlag)
    {
        __LOG_PARAM(Set0TcCntMode      );
        __LOG_PARAM(Set0TC             );
        __LOG_PARAM(Set0VlanCntMode    );
        __LOG_PARAM(Set0VID            );
        __LOG_PARAM(Set0PortCntMode    );
        __LOG_PARAM(Set0Port           );
        __LOG_PARAM(Set0DpCntMode      );
        __LOG_PARAM(Set0DP             );
        __LOG_PARAM(Set1TcCntMode      );
        __LOG_PARAM(Set1TC             );
        __LOG_PARAM(Set1VlanCntMode    );
        __LOG_PARAM(Set1VID            );
        __LOG_PARAM(Set1PortCntMode    );
        __LOG_PARAM(Set1Port           );
        __LOG_PARAM(Set1DpCntMode      );
        __LOG_PARAM(Set1DP             );
    }

    egrFltrCount = 0;
    if(descrPtr->useVidx)
    {
        mcFltr0 = mcFltr1 = GT_TRUE;
    }

    for(port = 0 ; port < SKERNEL_CHEETAH_EGRESS_MAX_PORT_CNS; port++)
    {
        if (pktFilteredPortsBmpPtr &&
            SKERNEL_PORTS_BMP_IS_PORT_SET_MAC(pktFilteredPortsBmpPtr,port))
        {
            /* the egress port related Counter Set condition not relevant */
            /* to filtered packet - it has no destination port            */
            egrFltrCount ++;
        }
        else if(forwardRestrictionBmpPtr &&
           SKERNEL_PORTS_BMP_IS_PORT_SET_MAC(forwardRestrictionBmpPtr, port))
        {
            /* update egress forward restriction packet counters */
            egrFltrCount ++;
        }

        if(destPorts[port] == 1)
        {
            mcFltr0 = mcFltr1 = GT_FALSE;
        }

        if(port >= SNET_SIP5_EGR_MAX_PHY_PORTS_MAC(devObjPtr))
        {
            break;
        }
    }

    /* default to be overriden */
    outCount0 = egrFltrCount;
    outCount1 = egrFltrCount;

    /* (Set0PortCntMode && (Set0Port != txqMibPort)) not relevant for egress filtered packet counter */
    if (Set0VlanCntMode && (descrPtr->eVid != Set0VID))
    {
        outCount0 = 0;
        mcFltr0 = GT_FALSE;
    }
    if (Set0TcCntMode && (txqMibTc != Set0TC))
    {
        outCount0 = 0;
        mcFltr0 = GT_FALSE;
    }
    if (Set0DpCntMode && (txqMibDp != Set0DP))
    {
        outCount0 = 0;
        mcFltr0 = GT_FALSE;
    }

    /* (Set1PortCntMode && (Set1Port != txqMibPort)) not relevant for egress filtered packet counter */
    if (Set1VlanCntMode && (descrPtr->eVid != Set1VID))
    {
        outCount1 = 0;
        mcFltr1 = GT_FALSE;
    }
    if (Set1TcCntMode && (txqMibTc != Set1TC))
    {
        outCount1 = 0;
        mcFltr1 = GT_FALSE;
    }
    if (Set1DpCntMode && (txqMibDp != Set1DP))
    {
        outCount1 = 0;
        mcFltr1 = GT_FALSE;
    }

    if (outCount0 || outCount1)
    {
        /* egress filtered packet counter */
        counterRegAddr0 =
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.egrMIBCntrs.egrMIBBridgeEgrFilteredPktCntr[0];
        counterRegAddr1 =
            SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.egrMIBCntrs.egrMIBBridgeEgrFilteredPktCntr[1];

        if(outCount0)
        {
            smemRegGet(devObjPtr,counterRegAddr0, &Counter);
            __LOG_SIP6_EGF_EFT_MIB_COUNTER(egrMIBBridgeEgrFilteredPktCntr,0,Counter,outCount0);
            Counter += outCount0;
            smemRegSet(devObjPtr,counterRegAddr0, Counter);
        }

        if(outCount1)
        {
            smemRegGet(devObjPtr,counterRegAddr1, &Counter);
            __LOG_SIP6_EGF_EFT_MIB_COUNTER(egrMIBBridgeEgrFilteredPktCntr,1,Counter,outCount1);
            Counter += outCount1;
            smemRegSet(devObjPtr,counterRegAddr1, Counter);
        }
    }

    if (mcFltr0 || mcFltr1)
    {
        counterRegAddr0 = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.
            egrMIBCntrs.egrMIBMcFilteredPktCntr[0];
        counterRegAddr1 = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.
            egrMIBCntrs.egrMIBMcFilteredPktCntr[1];

        if(mcFltr0)
        {
            smemRegGet(devObjPtr,counterRegAddr0, &Counter);
            __LOG_SIP6_EGF_EFT_MIB_COUNTER(egrMIBMcFilteredPktCntr,0,Counter,1);
            Counter += 1;
            smemRegSet(devObjPtr,counterRegAddr0, Counter);
        }

        if(mcFltr1)
        {
            smemRegGet(devObjPtr,counterRegAddr1, &Counter);
            __LOG_SIP6_EGF_EFT_MIB_COUNTER(egrMIBMcFilteredPktCntr,1,Counter,1);
            Counter += 1;
            smemRegSet(devObjPtr,counterRegAddr1, Counter);
        }
    }
    return;
}


/**
* @internal snetChtTxQClass function
* @endinternal
*
* @brief   packet type classification. (into descrPtr->egressPacketType)
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*/
static void snetChtTxQClass
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    SKERNEL_MTAG_CMD_ENT outCmd; /* outgoing command */
    SKERNEL_EGR_PACKET_TYPE_ENT packetType; /* type of packet UC , BC or MC */
    DECLARE_FUNC_NAME(snetChtTxQClass);

    /* analyse the outgoing marvell tag command */
    outCmd = descrPtr->outGoingMtagCmd ;
    if (outCmd == SKERNEL_MTAG_CMD_TO_CPU_E)
    {
        packetType = SKERNEL_EGR_PACKET_CNTRL_UCAST_E;
    }
    else if (outCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E)
    {
        if (descrPtr->useVidx == 0)/* UC packet*/
        {
            packetType = SKERNEL_EGR_PACKET_CNTRL_UCAST_E;
        }
        else
        {/* MC packet */
            packetType = SKERNEL_EGR_PACKET_CNTRL_MCAST_E;
        }
    }
    else if (outCmd == SKERNEL_MTAG_CMD_FROM_CPU_E)
    {
        if (descrPtr->egressFilterEn == 0)
        {/*control packet*/
            if (descrPtr->useVidx == 0)/* UC packet*/
            {
                packetType = SKERNEL_EGR_PACKET_CNTRL_UCAST_E;
            }
            else
            {/* MC packet */
                packetType = SKERNEL_EGR_PACKET_CNTRL_MCAST_E;
            }
        }
        else
        {/* ethernet packet*/
            if (descrPtr->useVidx == 0)/* UC packet*/
            {
                packetType = SKERNEL_EGR_PACKET_BRG_UCAST_E;
            }
            else
            {/* MC packet */
                if (descrPtr->egressFilterRegistered == 1)
                {
                    packetType = SKERNEL_EGR_PACKET_BRG_REG_MCAST_BCAST_E;
                }
                else
                {
                    if (descrPtr->macDaType == SKERNEL_UNICAST_MAC_E)
                    {
                        packetType = SKERNEL_EGR_PACKET_BRG_UNK_UCAST_E;
                    }
                    else
                    {
                        packetType = SKERNEL_EGR_PACKET_BRG_UNREG_MCAST_E;
                    }
                }
            }
        }
    }
    else
    {
        if (descrPtr->useVidx == 0)
        {
            if (descrPtr->routed == 0)
            {
                packetType = SKERNEL_EGR_PACKET_BRG_UCAST_E;
            }
            else
            {
                packetType = SKERNEL_EGR_PACKET_ROUT_UCAST_E;
            }
        }
        else
        {
            if (descrPtr->routed)
            {
                packetType = SKERNEL_EGR_PACKET_ROUT_MCAST_E;
            }
            else
            {
                if (descrPtr->egressFilterRegistered == 1)
                {
                    packetType = SKERNEL_EGR_PACKET_BRG_REG_MCAST_BCAST_E;
                }
                else
                {
                    if (descrPtr->macDaType == SKERNEL_UNICAST_MAC_E)
                    {
                        packetType = SKERNEL_EGR_PACKET_BRG_UNK_UCAST_E;
                    }
                    else
                    {
                        packetType = SKERNEL_EGR_PACKET_BRG_UNREG_MCAST_E;
                    }
                }
            }
        }
    }

    __LOG(("packet type classification : [%s] \n" ,
        packetType ==   SKERNEL_EGR_PACKET_CNTRL_UCAST_E            ?     "SKERNEL_EGR_PACKET_CNTRL_UCAST_E"           :
        packetType ==   SKERNEL_EGR_PACKET_CNTRL_MCAST_E            ?     "SKERNEL_EGR_PACKET_CNTRL_MCAST_E"           :
        packetType ==   SKERNEL_EGR_PACKET_BRG_UCAST_E              ?     "SKERNEL_EGR_PACKET_BRG_UCAST_E"             :
        packetType ==   SKERNEL_EGR_PACKET_BRG_REG_MCAST_BCAST_E    ?     "SKERNEL_EGR_PACKET_BRG_REG_MCAST_BCAST_E"   :
        packetType ==   SKERNEL_EGR_PACKET_BRG_UNK_UCAST_E          ?     "SKERNEL_EGR_PACKET_BRG_UNK_UCAST_E"         :
        packetType ==   SKERNEL_EGR_PACKET_BRG_UNREG_MCAST_E        ?     "SKERNEL_EGR_PACKET_BRG_UNREG_MCAST_E"       :
        packetType ==   SKERNEL_EGR_PACKET_ROUT_UCAST_E             ?     "SKERNEL_EGR_PACKET_ROUT_UCAST_E"            :
        packetType ==   SKERNEL_EGR_PACKET_ROUT_MCAST_E             ?     "SKERNEL_EGR_PACKET_ROUT_MCAST_E"            :
                "??? unknown ???"
        ));


    descrPtr->egressPacketType = packetType;

}

/**
* @internal snetChtTxQOamLoopbackFilterEnableGet function
* @endinternal
*
* @brief   Get the array of ports for OAM looped .
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*
* @param[out] oamLoopbackFilterEn[]    - array of OAM loop enabled
*/
static void snetChtTxQOamLoopbackFilterEnableGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    OUT GT_U32 oamLoopbackFilterEn[]
)
{
    DECLARE_FUNC_NAME(snetChtTxQOamLoopbackFilterEnableGet);

    GT_U32 port;            /* port number */
    GT_U32 regAddr,*regPtr; /* register address,register pointer*/
    GT_U32 localPort;       /* local Port number */
    GT_U32 startPort;            /* Start iteration port */
    GT_U32 endPort;              /* End iteration port */
    SKERNEL_PORTS_BMP_STC egressPortsBmp;
    GT_U32  globalOamLoopbackFilterEn;/* check if EGF filter the OAM looped packets */
    GT_U32  regValue;

    startPort = SNET_CHT_EGR_TXQ_START_PORT_MAC(devObjPtr);
    endPort = SNET_CHT_EGR_TXQ_END_PORT_MAC(devObjPtr);

    regAddr = SMEM_CHT_TXQ_OAM_FIRST_REG_LOOPBACK_FILTER_EN_REG(devObjPtr);
    regPtr = smemMemGet(devObjPtr, regAddr);

    SKERNEL_FILL_PORTS_BITMAP_MAC(devObjPtr, &egressPortsBmp, regPtr);


    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        smemRegGet(devObjPtr, SMEM_LION_TXQ_EGR_FILTER_GLOBAL_EN_REG(devObjPtr),&regValue);

        /* check the 'filter Enable' and the 'global filter enable' */
        if(SMEM_U32_GET_FIELD(regValue,0,1) &&
           SMEM_U32_GET_FIELD(regValue,11,1) )
        {
            globalOamLoopbackFilterEn = 1;
        }
        else
        {
            globalOamLoopbackFilterEn = 0;
        }

        if(globalOamLoopbackFilterEn == 0)
        {
            __LOG(("EGF disabled from filter the OAM looped packets \n"));
            return;
        }
    }

    /* fill the egress OAM loop enabled array */
    __LOG(("fill the egress OAM loop enabled array \n"));
    for (port = startPort,localPort = 0; port < endPort; port++ , localPort++)
    {
        oamLoopbackFilterEn[localPort] = SKERNEL_PORTS_BMP_IS_PORT_SET_MAC(&egressPortsBmp, port);

        if(oamLoopbackFilterEn[localPort])
        {
            __LOG(("Port[%d] hold egress OAM loop enabled \n"));
        }
    }

    oamLoopbackFilterEn[SNET_CHT_CPU_PORT_CNS] =
        SKERNEL_PORTS_BMP_IS_PORT_SET_MAC(&egressPortsBmp, SNET_CHT_CPU_PORT_CNS);
}

/**
* @internal snetChtTxQGetDpv function
* @endinternal
*
* @brief   Get destination vector for egress ports.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in,out] destPorts[]              - destination ports.
* @param[in,out] destVlanTagged[]         - destination vlan tagged.
* @param[in,out] stpVector[]              - vector of STP port state.
* @param[in] packetType               - send frame with tag.
* @param[in,out] pktFilteredPortsBmpPtr   - bitmap of filtered egress ports
*/
static void snetChtTxQGetDpv
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    INOUT GT_U32 destPorts[],
    INOUT GT_U8 destVlanTagged[],
    INOUT SKERNEL_STP_ENT stpVector[],
    IN SKERNEL_EGR_PACKET_TYPE_ENT packetType,
    INOUT SKERNEL_PORTS_BMP_STC *pktFilteredPortsBmpPtr,
    INOUT GT_U32 oamLoopbackFilterEn[]
)
{
    DECLARE_FUNC_NAME(snetChtTxQGetDpv);

    GT_U32  egressPort ;  /* outgoing port */
    GT_U32  localPort ; /* local port */

    SIM_LOG_TARGET_ARR_DECLARE(destPorts);

    /* save the destPorts for the logger  (for compare changes) */
    SIM_LOG_TARGET_ARR_SAVE(destPorts);

    snetChtTxQGetVidxInfo(devObjPtr, descrPtr, packetType, destPorts,
                     destVlanTagged, stpVector);

    /* compare the changes in destPorts */
    SIM_LOG_TARGET_ARR_COMPARE(destPorts);

    /* Unicast destination packets */
    if (descrPtr->useVidx == 0)
    {

        /* the functions are relevant only for UC packets */
        egressPort = snetChtTxQGetEgrPortForTrgDevice(devObjPtr, descrPtr, GT_FALSE);
        __LOG_PARAM(egressPort);

        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /* the EGF supports full 256 ports and not need to convert to local port */
        }
        else
        if(devObjPtr->portGroupSharedDevObjPtr)
        {
            /* convert target port to local port for the egress port group */
            localPort =
                SMEM_CHT_LOCAL_PORT_FROM_GLOBAL_PORT_MAC(devObjPtr,egressPort);

            __LOG(("convert target port[%d] to local port[%d] for the egress port group \n",
                egressPort,localPort));

            egressPort = localPort;
        }

        /* save the destPorts for the logger  (for compare changes) */
        SIM_LOG_TARGET_ARR_SAVE(destPorts);

        snetChtTxQUcastVlanFilter(
            devObjPtr, descrPtr, packetType, destPorts,
            stpVector, egressPort, pktFilteredPortsBmpPtr, oamLoopbackFilterEn);

        /* compare the changes in destPorts */
        SIM_LOG_TARGET_ARR_COMPARE(destPorts);
    }
}

/**
* @internal snetChtEgfEftEportVlanEgressFiltering function
* @endinternal
*
* @brief   SIP5: EGF - EFT,SHT : Eport Vlan Egress Filtering.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - output port
*
* @retval filter                   - GT_TRUE - need to be filtered
* @retval GT_FALSE                 - NOT need to be filtered
*/
static GT_BOOL snetChtEgfEftEportVlanEgressFiltering
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 egressPort
)
{
    DECLARE_FUNC_NAME(snetChtEgfEftEportVlanEgressFiltering);

    GT_U32  fldValue;/*field value*/
    GT_U32  regAddr; /* register address*/
    GT_U32  *memPtr;/*pointer to memory*/
    GT_U32  vidIndex; /* index into vid mapper and the result from vid mapper */
    GT_U32  trgEPort = descrPtr->trgEPort;/*target eport*/
    GT_U32  xIndex;/* 'X' */
    GT_U32  yIndex;/* 'Y' */
    GT_BOOL isMember;/* is the eport member in the evlan */
    GT_BOOL filter;/* do we need to filter */

    snetChtEgfEgressEPortEntryGet(devObjPtr,descrPtr,egressPort,GT_FALSE);/*global port*/

    if(descrPtr->eArchExtInfo.egfShtEgressEPortTablePtr == NULL)
    {
        return GT_FALSE;
    }

    /* Egress Eport Vlan Filter Enable */
    fldValue = snetFieldValueGet(descrPtr->eArchExtInfo.egfShtEgressEPortTablePtr,10,1);
    if(fldValue == 0)
    {
        __LOG(("Egress Eport Vlan Filter Disabled \n"));
        return GT_FALSE;
    }

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        __LOG(("Sip 6.0 : WARNING : The Egress 'Eport Vlan Filter' was removed , the EPORT MUST not enable access to it \n"));
        __LOG(("Sip 6.0 : WARNING : The Egress 'Eport Vlan Filter' was removed , ignore the request for it \n"));
        return GT_FALSE;
    }


    /* ePort VLAN Egress Filtering VLAN Mode */
    regAddr = SMEM_LION3_EGF_EFT_EGR_FILTERS_GLOBAL_REG(devObjPtr);
    smemRegFldGet(devObjPtr, regAddr, 12, 2, &fldValue);

    if(fldValue == 0)
    {
        /*Tag 1 VLAN ID is used*/
        vidIndex = descrPtr->vid1;
        __LOG(("Tag 1 VLAN ID is used, vidIndex[0x%3.3x] \n",
            vidIndex));
    }
    else if(fldValue == 1)
    {
        /* eVLAN is used*/
        vidIndex = descrPtr->eVid;
        __LOG(("eVLAN is used, vidIndex[0x%3.3x] \n",
            vidIndex));
    }
    else if(fldValue == 2)
    {
        /* The original VLAN is used */
        vidIndex = descrPtr->vid0Or1AfterTti;
        __LOG(("The original VLAN is used, vidIndex[0x%3.3x] \n",
            vidIndex));
    }
    else
    {
        __LOG(("ERROR : ePort VLAN Egress Filtering VLAN Mode : bad value[%d] \n",
            fldValue));
        return GT_FALSE;
    }

    /* map the vid index to 12 bits vid */
    regAddr = SMEM_LION2_EGF_SHT_VID_MAPPER_TBL_MEM(devObjPtr,vidIndex);
    smemRegFldGet(devObjPtr, regAddr, 0, 12, &vidIndex);

    __LOG(("New vidIndex[0x%3.3x] from the 'vid mapper' \n",
        vidIndex));

    /*Eport Vlan Egress Filtering Access Mode*/
    regAddr = SMEM_LION3_EGF_SHT_EVLAN_EGR_FILTERING_REG(devObjPtr);
    smemRegFldGet(devObjPtr, regAddr, 0, 3, &fldValue);


    /*For accessing the filtering table X=Row, Y=Column.
        Y = ePort[7:0], X is a function of the configuration mode:
        0 = For {4K VID, 256 ePort}. X={VLAN_INDEX[11:0]}
        1 = For {2K VID, 512 ePort}. X={VLAN_INDEX[10:0], ePort[8]}
        2 = For {1K VID, 1K ePort}. X={VLAN_INDEX[9:0], ePort[9:8]}
        3 = For {512 VID, 2K ePort}. X={VLAN_INDEX[8:0], ePort[10:8]}
        4 = For {256 VID, 4K ePort}. X={VLAN_INDEX[7:0], ePort[11:8]}
        5 = For {128 VID, 8K ePort}. X={VLAN_INDEX[6:0], ePort[12:8]}
        6 = For {64 VID, 16K ePort}. X={VLAN_INDEX[5:0], ePort[13:8]}
        7 = For {32 VID, 32K ePort}. X={VLAN_INDEX[4:0], ePort[14:8]
    */

    yIndex = SMEM_U32_GET_FIELD(trgEPort,0,8);
    xIndex = 0;

    /* the LSBits are from the trgEPort */
    SMEM_U32_SET_FIELD(xIndex,0,fldValue,
            SMEM_U32_GET_FIELD(trgEPort, 8, fldValue));

    /* the MSBits are from the vidIndex */
    SMEM_U32_SET_FIELD(xIndex,fldValue,(12-fldValue),vidIndex);

    regAddr = SMEM_LION2_EGF_SHT_EPORT_EVLAN_FILTER_TBL_MEM(devObjPtr,0);
    memPtr = smemMemGet(devObjPtr, regAddr);
    /* xIndex is given as index to entry of 256 bits */
    /* yIndex is given as bit index inside the entry of 256 bits */
    fldValue = snetFieldValueGet(memPtr,(xIndex*256) + yIndex,1);

    if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
    {
        /******************************/
        /* this is for LOG info only */
        /* convert xIndex,yIndex from format of 256 bits per entry to 64 bits per entry */
        /******************************/
        /* xIndex is given as index to entry of 64 bits */
        /* yIndex is given as bit index inside the entry of 64 bits */
        /******************************/
        xIndex = (xIndex << 2) + (yIndex>>6);
        yIndex = yIndex & 0x3f;
    }

    isMember = fldValue ? GT_TRUE : GT_FALSE;

    __LOG(("accessed the eport evlan filter table with Line[0x%4.4x] Bit[0x%4.4x] and eport is %s MEMBER in the evlan \n",
        xIndex,
        yIndex,
        isMember ? "" : "NOT"));


    /* the filter is the opposite to the 'is member' */
    filter = fldValue ? GT_FALSE : GT_TRUE;

    return filter;
}

/**
* @internal snetChtTxQUcastVlanFilter function
* @endinternal
*
* @brief   Unicast VLAN filtering - leave only egressPort in destPorts
*         or clear it if egress port VLAN filtered
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] packetType               - type of packet.
* @param[in,out] destPorts[]              - destination ports.
* @param[in] stpVector[]              - stp ports vector.
* @param[in] egressPort               - output port
* @param[in,out] pktFilteredPortsBmpPtr   - bitmap of filtered egress ports
*/
static void snetChtTxQUcastVlanFilter
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SKERNEL_EGR_PACKET_TYPE_ENT packetType,
    INOUT GT_U32 destPorts[],
    IN SKERNEL_STP_ENT stpVector[],
    IN GT_U32 egressPort,
    INOUT SKERNEL_PORTS_BMP_STC *pktFilteredPortsBmpPtr,
    IN GT_U32 oamLoopbackFilterEn[]
)
{
    DECLARE_FUNC_NAME(snetChtTxQUcastVlanFilter);

    GT_U8 sendToEgress=1;   /* enable/disable egress filtering ? */
    GT_U32 port;            /* port number */
    GT_U32 regAddr;         /* register address,unicast egress filtering */
    GT_U32 fldValue=0;      /* egress filtering bit value */
    GT_U32  tunnelEgressFilter; /* egress filtering bit value */
    GT_U32  shtGlobalConfRegVal;/* register value */
    GT_U32  localDevSrcPort;

    localDevSrcPort = descrPtr->localPortGroupPortAsGlobalDevicePort;/* local port(not global) */

    if(devObjPtr->txqRevision != 0)
    {
        /* SHT Global Configurations */
        __LOG(("SHT Global Configurations"));
        smemRegGet(devObjPtr, SMEM_LION_TXQ_SHT_GLOBAL_CONF_REG(devObjPtr), &shtGlobalConfRegVal);
    }
    else
    {
        /* Egress Filtering Register0 */
        __LOG(("Access Egress Filtering Register(s)"));
        smemRegGet(devObjPtr, SMEM_CHT_EGRESS_UNKN_UC_FILTER_EN_REG(devObjPtr), &shtGlobalConfRegVal);
    }

    if (packetType != SKERNEL_EGR_PACKET_CNTRL_UCAST_E)
    {
        if (destPorts[egressPort] == 0)/* egress port not in vlan */
        {
            if (packetType == SKERNEL_EGR_PACKET_ROUT_UCAST_E)
            {
                /* Enables egress filtering on routed Unicast packets */
                fldValue = (devObjPtr->txqRevision != 0) ?
                    SMEM_U32_GET_FIELD(shtGlobalConfRegVal, 1, 1) :
                    SMEM_U32_GET_FIELD(shtGlobalConfRegVal, 28, 1);

                if(fldValue)
                {
                    __LOG(("Enables egress filtering on routed Unicast packets"));
                }
            }
            else
            {
                /* Enables egress filtering on bridged known Unicast packets */
                fldValue = (devObjPtr->txqRevision != 0) ?
                    SMEM_U32_GET_FIELD(shtGlobalConfRegVal, 0, 1) :
                    SMEM_U32_GET_FIELD(shtGlobalConfRegVal, 29, 1);

                if(fldValue)
                {
                    __LOG(("Enables egress filtering on bridged known Unicast packets"));
                }
            }

            if (fldValue)
            {
                sendToEgress = 0;
            }
        }

        if(destPorts[egressPort])
        {
            if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
            {
                /* Egress Eport Vlan Filter Enable */
                fldValue =
                    snetChtEgfEftEportVlanEgressFiltering(devObjPtr,descrPtr,
                        egressPort);
                if(fldValue)
                {
                    sendToEgress = 0;

                    __LOG(("Egress Eport Vlan Filtered the packet \n"));
                }
                else
                {
                    __LOG(("Egress Eport Vlan NOT Filtered the packet"));
                }
            }
        }

        if (sendToEgress)
        {
            if (packetType == SKERNEL_EGR_PACKET_ROUT_UCAST_E)
            {
                /* Enables Span State egress filtering on routed packets */
                fldValue = (devObjPtr->txqRevision != 0) ?
                    SMEM_U32_GET_FIELD(shtGlobalConfRegVal, 2, 1) :
                    SMEM_U32_GET_FIELD(shtGlobalConfRegVal, 31, 1);

                if(fldValue)
                {
                    __LOG(("Enables Span State egress filtering on routed packets"));
                }
            }
            else
            {
                /* bridged packets always check STP state*/
                __LOG(("bridged packets always check STP state"));
                fldValue = 1;
            }

            if (fldValue)
            {
                if ( (stpVector[egressPort] != SKERNEL_STP_DISABLED_E) &&
                     (stpVector[egressPort] != SKERNEL_STP_FORWARD_E) )
                {
                    sendToEgress = 0;
                }
            }
        }
    }

    for (port = 0; port < SKERNEL_DEV_EGRESS_MAX_PORT_CNS(devObjPtr); port++)
    {
        destPorts[port] = 0;
    }

    if (oamLoopbackFilterEn[localDevSrcPort] &&
        localDevSrcPort == egressPort &&
        descrPtr->egressOnIngressPortGroup)
    {
        sendToEgress = 1 ;/* no filtering when in OAM loopback mode */
    }

    if (descrPtr->tunnelStart)
    {
        if(devObjPtr->txqRevision != 0)
        {
            regAddr = SMEM_LION_TXQ_EGR_FILTER_GLOBAL_EN_REG(devObjPtr);
            if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
            {
                smemRegGet(devObjPtr, regAddr, &fldValue);

                /* check the 'filter Enable' and the 'global filter enable' */
                if(SMEM_U32_GET_FIELD(fldValue,0,1) &&
                   SMEM_U32_GET_FIELD(fldValue,7,1) )
                {
                    tunnelEgressFilter = 1;
                }
                else
                {
                    tunnelEgressFilter = 0;
                }
            }
            else
            {
                smemRegFldGet(devObjPtr, regAddr, 8, 1, &tunnelEgressFilter);
            }
        }
        else
        {
            /* Enable/Disable Egress filtering for Tunnel Start packets */
            regAddr = SMEM_CHT2_TUNNEL_EGRESS_CONFIG_REG(devObjPtr);
            smemRegFldGet(devObjPtr, regAddr, 25, 1, &tunnelEgressFilter);
        }

        if (tunnelEgressFilter == 0)
        {
            sendToEgress = 1;/* no filtering on Tunnel start */
        }
    }

    destPorts[egressPort] = sendToEgress;

    if (sendToEgress == 0)
    {
        /* packet egress filtered */
        __LOG(("packet egress filtered on port[%d] \n",
            egressPort));
        SKERNEL_PORTS_BMP_ADD_PORT_MAC(
            pktFilteredPortsBmpPtr, egressPort);
    }
}

/**
* @internal snetChtTxQGetVidxInfo function
* @endinternal
*
* @brief   Get the vlan information and stp
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] packetType               - type of packet.
*
* @param[out] destPorts[]              - destination ports.
* @param[out] destVlanTagged[]         - destination vlan tagged.
* @param[out] stpVector[]              - stp ports vector.
*/
static void snetChtTxQGetVidxInfo
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SKERNEL_EGR_PACKET_TYPE_ENT packetType,
    OUT GT_U32 destPorts[],
    OUT GT_U8 destVlanTagged[],
    OUT SKERNEL_STP_ENT stpVector[]
)
{
    DECLARE_FUNC_NAME(snetChtTxQGetVidxInfo);

    GT_U32 vid;  /* vid of packet */
    GT_U32 regAddress;   /* address of vlan and stg table */
    GT_U32 fldValue;
    GT_U32 i;
    GT_U32 * vlanEntryPtr, * stgEntryPtr;/* vlan and stg entry content */
    GT_U32 stgGroupEntry;/* stg group entry number */
    GT_U32 startBit;/* first bit of relevant vlan entry */
    GT_U32 stgEnrtyIndex=0;/* stg group entry index , received from vlan table*/
    GT_U32 port;           /* port number */
    GT_U32 checkVidxGroup=0;
    GT_U32 wordIndex;
    GT_U32 bitIndex;
    GT_U32 bpe802_1br_McSourceFilteringBmp;
    GT_BIT bpeEnabled; /* Enable for IEEE 802.1BR Multicast Source Filtering.*/
    GT_BIT bpeMcGroups;
    GT_U32 vlanMembers = 0;
    GT_U32  bypassStpFiltering = 0;/* do we bypass the STP filtering */

    if(descrPtr->tunnelStart)
    {
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /* the functionality of ethernetOverXPassengerTagMode,ethernetOverXPassengerTagged
            removed.
                instead the application can configure the target ePort
                (that associated with) to have specific 'egress tag mode'
            */
        }
        else
        if(devObjPtr->txqRevision != 0)
        {
            regAddress = SMEM_LION_TXQ_DISTR_GENERAL_CONF_REG(devObjPtr);
            smemRegGet (devObjPtr , regAddress , &fldValue );
            descrPtr->ethernetOverXPassengerTagMode = SMEM_U32_GET_FIELD(fldValue, 11, 1);
            descrPtr->ethernetOverXPassengerTagged = SMEM_U32_GET_FIELD(fldValue, 12, 1);
        }
        else
        {
            regAddress = SMEM_CHT2_TUNNEL_EGRESS_CONFIG_REG(devObjPtr);
            smemRegGet (devObjPtr , regAddress , &fldValue );
            descrPtr->ethernetOverXPassengerTagMode = SMEM_U32_GET_FIELD(fldValue, 26, 1);
            descrPtr->ethernetOverXPassengerTagged = SMEM_U32_GET_FIELD(fldValue, 27, 1);
        }
    }

    if(SKERNEL_IS_XCAT_REVISON_A1_DEV(devObjPtr))
    {
        /* TR-101 Configuration */
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            smemRegFldGet(devObjPtr, SMEM_CHT3_ROUTE_HA_GLB_CNF_REG(devObjPtr),
                          12, 1, &descrPtr->tr101ModeEn);
        }
        else
        {
            smemRegFldGet(devObjPtr, SMEM_CHT3_ROUTE_HA_GLB_CNF_REG(devObjPtr),
                          11, 1, &descrPtr->tr101ModeEn);
        }

        if(descrPtr->tr101ModeEn == 0)
        {
            __LOG(("TR-101 NOT enabled \n"));
        }
    }


    if(devObjPtr->txqRevision)
    {
        snetLionTxQGetEgressVidxInfo(devObjPtr, descrPtr, packetType, destPorts,
                                     destVlanTagged, stpVector);
        return;
    }

    /* support 802.1d bridge (Vlan-unaware mode) */
    vid = descrPtr->basicMode ? 1 : descrPtr->eVid;

    __LOG(("access vlan entry [0x%x] \n",vid));

    /* read vlan entry from VLT table , 0x0A000000 */
    regAddress = SMEM_CHT_VLAN_TBL_MEM(devObjPtr, vid);
    vlanEntryPtr = smemMemGet(devObjPtr, regAddress);

    /* check validity of VLAN */
    if ((vlanEntryPtr[0] & 0x1)==0)
    {
        __LOG(("egress VLAN [%d] NOT valid \n",
            vid));
        return ;
    }
    stgGroupEntry = vlanEntryPtr[2] >> 24;

    /* read the vlan entry fldValue_PTR[0..3] = word 0..3 */

    /* first 27 ports are in a roll */
    startBit=24;/*starting from bit 24 in word 0 */
    for(port = 0 ; port < 27 ; port++)
    {
        wordIndex = (startBit + port*2) >> 5;
        bitIndex  = (startBit + port*2) & 0x1f;

        destPorts[port] = SMEM_U32_GET_FIELD(vlanEntryPtr[wordIndex],bitIndex,1);
        if (destPorts[port])
        {
            vlanMembers |= 1 << port;
        }

        destVlanTagged[port] = SMEM_U32_GET_FIELD(vlanEntryPtr[wordIndex],bitIndex+1,1);

        if(descrPtr->tr101ModeEn)
        {
            wordIndex += 3;
            if(bitIndex < 30)
            {
                /* Get word's two most bits */
                destVlanTagged[port] |=
                    (SMEM_U32_GET_FIELD(vlanEntryPtr[wordIndex], bitIndex+1, 2) << 1);
            }
            else
            { /* There are two bits in two different words */
                destVlanTagged[port] |=
                    (SMEM_U32_GET_FIELD(vlanEntryPtr[wordIndex], bitIndex+1, 1) << 1);
                destVlanTagged[port] |=
                    (SMEM_U32_GET_FIELD(vlanEntryPtr[wordIndex+1], 0, 1) << 2);
            }
        }
    }

    if(SNET_CHEETAH_MAX_PORT_NUM(devObjPtr) > 27)
    {
        if(SKERNEL_IS_XCAT_REVISON_A1_DEV(devObjPtr))
        {/* xcat moved the place for port 27 */
            wordIndex = 3;
            bitIndex  = 23;

            destPorts[port] =
                SMEM_U32_GET_FIELD(vlanEntryPtr[wordIndex], bitIndex, 1);
            destVlanTagged[port] =
                SMEM_U32_GET_FIELD(vlanEntryPtr[wordIndex], bitIndex + 1, 1);
        }
        else
        {
            /*port 28 somewhere else */
            wordIndex = 3;
            bitIndex  = 8;

            destPorts[port] =
                SMEM_U32_GET_FIELD(vlanEntryPtr[wordIndex],bitIndex,1);
            destVlanTagged[port] =
                SMEM_U32_GET_FIELD(vlanEntryPtr[wordIndex],bitIndex+1,1);
        }

        if(descrPtr->tr101ModeEn)
        {
            wordIndex = 5;
            bitIndex  = 15;
            destVlanTagged[port] |=
                (SMEM_U32_GET_FIELD(vlanEntryPtr[wordIndex], bitIndex, 2) << 1);
        }

        if (destPorts[port])
        {
            vlanMembers |= 1 << port;
        }
    }

    __LOG_PARAM(vlanMembers);
    if (SMEM_U32_GET_FIELD(vlanEntryPtr[0], 2, 1))
    {
        __LOG_PARAM("CPU port is vlan member \n")
    }

    /* For XCat chip family local switching for multi-destination packets
    received on a non-trunk port is enabled/disabled per egress port
    and per VLAN */
    if(SKERNEL_IS_XCAT_REVISON_A1_DEV(devObjPtr) ||
       SKERNEL_DEVICE_FAMILY_LION_PORT_GROUP_ONLY(devObjPtr))/* lion A*/
    {
        descrPtr->egressVlanInfo.mcLocalEn =
            SMEM_U32_GET_FIELD(vlanEntryPtr[3], 20, 1); /*116*/
        descrPtr->egressVlanInfo.portIsolationVlanCmd =
            SMEM_U32_GET_FIELD(vlanEntryPtr[3], 21, 2); /*117,118*/
    }
    else
    {
        descrPtr->egressVlanInfo.mcLocalEn = 1;
        descrPtr->egressVlanInfo.portIsolationVlanCmd = 0;/* not relevant to this device */
    }

    if(descrPtr->pktIsLooped == 0)
    {
        switch(packetType)
        {
            case SKERNEL_EGR_PACKET_CNTRL_UCAST_E:
                 __LOG(("NOTE: CNTRL_UCAST not subject to egress STP filter \n"));
                 bypassStpFiltering = 1;
                break;
            case SKERNEL_EGR_PACKET_CNTRL_MCAST_E:
                 __LOG(("NOTE: CNTRL_MCAST not subject to egress STP filter \n"));
                 bypassStpFiltering = 1;
                break;
            default:
                break;
        }
    }

    if(bypassStpFiltering == 0)
    {
        /*get stp vector info. */
        __LOG(("access STG entry [0x%x] \n",stgGroupEntry));
        port = 0;
        regAddress = SMEM_CHT_STP_TBL_MEM(devObjPtr, stgGroupEntry);
        stgEntryPtr = smemMemGet(devObjPtr,regAddress);
        for (stgEnrtyIndex = 0; stgEnrtyIndex < 2; stgEnrtyIndex++)
        {
            for (i = 0  ; i <= 31  ; i += 2 )
            {
                if ( port >= SNET_CHEETAH_MAX_PORT_NUM(devObjPtr) )
                    break ;
                stpVector[port] = (stgEntryPtr[stgEnrtyIndex]  >> i) & 0x3;
                port++;
            }
        }
    }

    /* 1. check if packet is multicast , if set only vidx group is considered */
    if (descrPtr->useVidx)
    {
        __LOG(("packet is multi-destination , vidx group is considered \n"));
        checkVidxGroup = 1;
    }
    else if(descrPtr->eVidx == 0xFFF)
    {
        /* the VIDX 0xFFF is checked on single AND multi destination traffic !!! */
        __LOG(("the VIDX 0xFFF is checked on single AND multi destination traffic !!!"));
        checkVidxGroup = 1;
    }

    if ( checkVidxGroup )
    {

        GT_U32 vidxIndex = descrPtr->eVidx;

        if(devObjPtr->support802_1br_PortExtender && descrPtr->useVidx)
        {
            smemRegFldGet(devObjPtr,
                SMEM_XCAT3_EGRESS_AND_TXQ_802_1_BR_GLOBAL_CONFIG_REG (devObjPtr), 0, 1, &bpeEnabled);
            smemRegFldGet(devObjPtr,
                SMEM_XCAT3_EGRESS_AND_TXQ_802_1_BR_GLOBAL_CONFIG_REG (devObjPtr), 1, 1, &bpeMcGroups);
            if(bpeEnabled == 1 && bpeMcGroups == 0)
            {
                GT_U32  GRP = SMEM_U32_GET_FIELD(descrPtr->vid0Or1AfterTti,0,2);
                if(GRP != 0)/* <GRP != 0> */
                {
                    /* build index of 14 bits with values 0..12k */
                    vidxIndex = descrPtr->eVidx + ((GRP - 1) << 12);
                    __LOG(("BPE: access VIDX table in index [0x%x] (GRP[0x%x] , descrPtr->eVidx[0x%x])\n",
                        vidxIndex,GRP,descrPtr->eVidx));
                }
                else
                {
                    /* build index of 14 bits with values 0..4k */
                    vidxIndex = descrPtr->eVidx;
                    __LOG(("BPE: access VIDX table in index [0x%x] (GRP[0]) \n",
                        vidxIndex));
                }
            }
        }

        __LOG(("access VIDX entry [0x%x] \n",vidxIndex));
        regAddress = SMEM_CHT_MCST_TBL_MEM(devObjPtr, vidxIndex);

        smemRegGet (devObjPtr , regAddress , &fldValue );

        /* take care of CPU Port member for multicast packets.
           Check bit#0 in VIDX and bit#2 in VLAN entry. */
        destPorts[SNET_CHT_CPU_PORT_CNS] = SMEM_U32_GET_FIELD(fldValue, 0, 1) &
                                           SMEM_U32_GET_FIELD(vlanEntryPtr[0], 2, 1);

        {
            GT_U32  vidxMembers = (fldValue >> 1);/* ignore CPU port*/

            __LOG_PARAM(vidxMembers);
            if (SMEM_U32_GET_FIELD(fldValue, 0, 1))
            {
                __LOG_PARAM("CPU port is vidx member \n")
            }

            __LOG(("bmp of ports after VLAN & VIDX are[0x%8.8x] \n",
                (vidxMembers & vlanMembers)));

            if(destPorts[SNET_CHT_CPU_PORT_CNS])
            {
                __LOG_PARAM("CPU port is vlan and vidx member \n")
            }

        }

        fldValue >>= 1;/* skip the cpu bit */
        for (i = 0 ; i < SNET_CHEETAH_MAX_PORT_NUM(devObjPtr) ; i++ )
        {
            destPorts[i] = destPorts[i] & (fldValue >> i);
        }
    }

    /* JIRA : ALLEYCAT3-521 - UC packets are filtered by the ECID filter
        the device should do it only for useVidx = 1 but
        it is done also for useVidx = 0 */
    if(devObjPtr->support802_1br_PortExtender
        /*&& descrPtr->useVidx*/)
    {
        /* This is the 802.1BR multicast source filtering */
        regAddress = SMEM_XCAT3_EGRESS_AND_TXQ_ECID_TBL_MEM(devObjPtr, descrPtr->vid1);
        smemRegGet (devObjPtr , regAddress , &bpe802_1br_McSourceFilteringBmp );

        __LOG_PARAM(descrPtr->vid1);
        __LOG_PARAM(bpe802_1br_McSourceFilteringBmp);

        for (i = 0 ; i <= SNET_CHEETAH_MAX_PORT_NUM(devObjPtr) ; i++ )
        {
            if (i == SNET_CHEETAH_MAX_PORT_NUM(devObjPtr))
            {
                i = SNET_CHT_CPU_PORT_CNS;
                bitIndex = 28;
            }
            else
            {
                bitIndex = i;
            }

            if(destPorts[i] && ((bpe802_1br_McSourceFilteringBmp >> bitIndex) & 1))
            {
                if (descrPtr->useVidx == 0)
                {
                    /* JIRA : ALLEYCAT3-521 - UC packets are filtered by the ECID filter */
                    __LOG(("ERROR : BPE : 802.1BR multicast source filtering : on UC traffic port[%d] although should not be filtered \n",i));
                }
                else
                {
                    __LOG(("BPE : 802.1BR multicast source filtering : filtered port [%d] \n", i));
                }
                destPorts[i] = 0;
            }
        }
    }
}

/**
* @internal snetChtEgfCpuPortMapGet function
* @endinternal
*
* @brief   SIP5 : Get 'local dev CPU port' to represent cpuCode for 'TO_CPU' packets
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*
* @note The function is relevant only for sip5 'TO_CPU' packets.
*
*/
static GT_U32 snetChtEgfCpuPortMapGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetChtEgfCpuPortMapGet);
    GT_U32  numPortsInRegister;
    GT_U32  numBitsPerPort;
    GT_U32  index,startBit;
    GT_U32  localDevCpuPort;
    GT_U32  regAddr;

    numPortsInRegister = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ?  2 : 4;
    numBitsPerPort     = SMEM_CHT_IS_SIP6_30_GET(devObjPtr) ?  6 :
                         SMEM_CHT_IS_SIP6_10_GET(devObjPtr) ?  7 :
                         SMEM_CHT_IS_SIP6_GET(devObjPtr)    ? 10 :
                         SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ?  9 : 8;
    index    =  descrPtr->cpuCode / numPortsInRegister;
    startBit = (descrPtr->cpuCode % numPortsInRegister) * numBitsPerPort;

    regAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_eft.global.cpuPortMapOfReg[index];

    smemRegFldGet(devObjPtr, regAddr, startBit , numBitsPerPort, &localDevCpuPort);
    __LOG_PARAM(localDevCpuPort);

    if(localDevCpuPort != SNET_CHT_CPU_PORT_CNS)/* the default */
    {
        __LOG(("cpuCode[%d] : Selected specific 'CPU port' [%d] instead of 'default CPU port' [63]\n",
            descrPtr->cpuCode,
            localDevCpuPort));
    }
    else
    {
        __LOG(("cpuCode[%d] : Keep the 'default CPU port' [63]\n",
            descrPtr->cpuCode));
    }


    return localDevCpuPort;
}

/**
* @internal snetChtTxQDeviceMapTableAccess function
* @endinternal
*
* @brief   Get egress port for unicast packet (using the device map table)
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] calledFromTxdUnit        - is function called from TXD unit(TX distributor -- multi-port groups)
*                                      GT_TRUE - called from TXD unit. in this case if the
*                                      target it 'Cascade trunk', function returns the 'trunkId'
*                                      and not select a trunk member !
*                                      GT_FALSE - not called from TXD unit.
*                                      NOTE: TXD is relevant only to multi-port groups device.
* @param[in] trgDev                   - the target device
* @param[in] trgPort                  - the target port
*
* @note The function is relevant for single destination packets only.
*
*/
static GT_U32 snetChtTxQDeviceMapTableAccess
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 trgDev,
    IN GT_U32 trgPort,
    IN GT_BOOL  calledFromTxdUnit
)
{
    DECLARE_FUNC_NAME(snetChtTxQDeviceMapTableAccess);

    SKERNEL_MTAG_CMD_ENT outCmd;              /* Marvell tag command */
    GT_U32               regAddr;             /* Egress Filtering Table Entry*/
    GT_U32               regData;             /* Register data */
    GT_U32             * regPtr;              /* Register data pointer */
    GT_U32               egrPortOrTrunkId;    /* Egress port or trunk number */
    GT_U32               port;                /* Port loop */
    SKERNEL_PORTS_BMP_STC nonTrunkMembersBmp;  /* Cascade trunk members ports */
    SKERNEL_PORTS_BMP_STC desTrunkPort;        /* Designated trunk ports */
    SKERNEL_PORTS_BMP_STC sumDesCascadeTrunkBmp; /* Summary designated trunk port */
    TRUNK_HASH_MODE_ENT cscdTrunkMode;  /* cascade trunk mode */
    GT_U32               startPort;            /* Start iteration port */
    GT_U32               endPort;              /* End iteration port */
    SKERNEL_PORTS_BMP_STC egressPortsBmp;
    GT_U32              srcPort;
    GT_U32              srcDev;
    GT_U32              uplinkIsTrunk;/*Uplink Is Trunk*/
    GT_U32              cascadeTrunkHashMode;/*cascade trunk hash mode */
    GT_U32              numPortsInBmp,firstPortInBmp;
    SKERNEL_PORTS_BMP_STC trunkMembersBmp;  /* Cascade trunk members ports */


    startPort = SNET_CHT_EGR_TXQ_START_PORT_MAC(devObjPtr);
    endPort = SNET_CHT_EGR_TXQ_END_PORT_MAC(devObjPtr);

    SKERNEL_PORTS_BMP_CLEAR_MAC(&sumDesCascadeTrunkBmp);
    SKERNEL_PORTS_BMP_CLEAR_MAC(&nonTrunkMembersBmp);
    SKERNEL_PORTS_BMP_CLEAR_MAC(&desTrunkPort);

    /* Analyse the outgoing marvell tag command */
    outCmd = descrPtr->outGoingMtagCmd ;

    if(devObjPtr->deviceMapTableUseDstPort == 0)
    {
        __LOG_PARAM(trgDev);
        regAddr = SMEM_CHT_DEVICE_MAP_TABLE_ENTRY(devObjPtr, trgDev);
    }
    else
    {
        if(SKERNEL_IS_LION_REVISON_B0_DEV(devObjPtr))
        {
            if (descrPtr->origIsTrunk)
            {
                srcPort = descrPtr->origSrcEPortOrTrnk >> 1;
            }
            else
            {
                srcPort = descrPtr->origSrcEPortOrTrnk;
            }

            if(devObjPtr->supportEArch)
            {
                trgPort =
                    (devObjPtr->unitEArchEnable.txq || descrPtr->eArchExtInfo.isTrgPhyPortValid == 0) ?
                    descrPtr->trgEPort :
                    descrPtr->eArchExtInfo.trgPhyPort;

                if(outCmd == SKERNEL_MTAG_CMD_TO_CPU_E)
                {
                    trgPort = snetChtEgfCpuPortMapGet(devObjPtr,descrPtr);
                }
            }

            srcDev = descrPtr->srcDev;

            __LOG_PARAM(trgDev);
            __LOG_PARAM(trgPort);
            __LOG_PARAM(srcDev);
            __LOG_PARAM(srcPort);
            regAddr = snetLionTxQGetDeviceMapTableAddress(devObjPtr, descrPtr,
                                                          trgDev,trgPort,
                                                          srcDev,srcPort);
            if(regAddr == SMAIN_NOT_VALID_CNS)
            {
                __LOG(("The device map table will not be accessed \n"));
                /* indication that the device map table will not be accessed */
                return SMAIN_NOT_VALID_CNS;
            }
        }
        else
        {
            __LOG_PARAM(trgDev);
            __LOG_PARAM(trgPort);
            /* the device map table is accessed using the dst port and not only the
               dst device*/
            regAddr = SMEM_CHT_DEVICE_MAP_TABLE_WITH_DST_PORT_TBL_MEM(devObjPtr,
                                                                    trgDev,
                                                                    trgPort);
        }
    }

    smemRegGet(devObjPtr, regAddr, &regData);

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        GT_U32  numBitsPort  = SMEM_CHT_IS_SIP6_10_GET(devObjPtr) ?  8 :
                               SMEM_CHT_IS_SIP6_GET(devObjPtr)    ? 10 :
                              (SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 9 : 8);
        GT_U32  numBitsTrunk = SMEM_CHT_IS_SIP5_10_GET(devObjPtr) ? 8 : 12;
        GT_U32  numBitsTrunkPort = MAX(numBitsTrunk,numBitsPort);

        /*Assign Egress Attributes Locally*/
        descrPtr->eArchExtInfo.assignTrgEPortAttributesLocally =
            SMEM_U32_GET_FIELD(regData, 0, 1);
        __LOG(("assignTrgEPortAttributesLocally %d from Dev map table\n",descrPtr->eArchExtInfo.assignTrgEPortAttributesLocally));

        uplinkIsTrunk = SMEM_U32_GET_FIELD(regData, 1, 1);
        egrPortOrTrunkId = SMEM_U32_GET_FIELD(regData, 2, numBitsTrunkPort);
        if(uplinkIsTrunk == 0)
        {
            egrPortOrTrunkId &= (1<<numBitsPort)-1;/*8 bits port*/
        }
        else
        {
            egrPortOrTrunkId &= (1<<numBitsTrunk)-1;/*8 bits trunk*/
        }
        cascadeTrunkHashMode = SMEM_U32_GET_FIELD(regData, (2+numBitsTrunkPort), 2);

        if(cascadeTrunkHashMode == 0)
        {
            cscdTrunkMode = TRUNK_HASH_MODE_USE_PACKET_HASH_E;
        }
        else if (cascadeTrunkHashMode == 1)
        {
            cscdTrunkMode = TRUNK_HASH_MODE_USE_LOCAL_SRC_PORT_HASH_E;
        }
        else if (cascadeTrunkHashMode == 2)
        {
            cscdTrunkMode = TRUNK_HASH_MODE_USE_GLOBAL_DST_PORT_HASH_E;
        }
        else /*if (cascadeTrunkHashMode == 3)*/
        {
            /* DynamicLoadBalancing
            Unicast
                When enabled, a multicast packet which has a tunnel start will undergo
                regular filtering and replication.
                0x0 - Multicast packet will be processed normally.
                0x1 - Perform filtering on tunnel start multicast traffic even if
                <TunnelStartEgressFilterEn> is disabled.
                - A logical AND will be performed between the trunk members from the "not trunk members table"
                and the queues which did not pass the Xoff threshold (according to the priority of the packet)
                from the queue status indications.
                - If the result is 0 a logical AND will be performed between the trunk members from the "not
                trunk members table" and the designated members table, access to the designated members
                table is based on the hash.
                - From the result the first port which fulfills the both requirements will be selected.
            Multicast:
                - Current filtering will be performed using the designated members table
            */

            /**********************************************/
            /*for simulation the behavior is 'packet hash'*/
            /**********************************************/
            cscdTrunkMode = TRUNK_HASH_MODE_USE_PACKET_HASH_E;
        }
    }
    else
    {
        uplinkIsTrunk = SMEM_U32_GET_FIELD(regData, 0, 1);
        egrPortOrTrunkId = SMEM_U32_GET_FIELD(regData, 1, 7);
        if(uplinkIsTrunk == 0)
        {
            egrPortOrTrunkId &= 0x3f;/*6 bits port*/
        }

        if(SKERNEL_IS_CHEETAH1_ONLY_DEV(devObjPtr) ||
           SMEM_U32_GET_FIELD(regData, 8, 1) == 0)
        {
            cscdTrunkMode = TRUNK_HASH_MODE_USE_PACKET_HASH_E;

            if(devObjPtr->supportCascadeTrunkHashDstPort)
            {
                if(SMEM_U32_GET_FIELD(regData, 9, 1) == 1)
                {
                    cscdTrunkMode = TRUNK_HASH_MODE_USE_GLOBAL_DST_PORT_HASH_E;
                }
            }
        }
        else
        {
            cscdTrunkMode = TRUNK_HASH_MODE_USE_LOCAL_SRC_PORT_HASH_E;
        }
    }

    __LOG_PARAM(cscdTrunkMode);
    __LOG_PARAM(uplinkIsTrunk);
    __LOG_PARAM(egrPortOrTrunkId);


    /* The uplink for this device is a port */
    if (uplinkIsTrunk == 0)
    {
        port = egrPortOrTrunkId; /* egress port id */

        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /* no modifications to the port */
        }
        else
        if(devObjPtr->numOfTxqUnits >= 2)
        {
            /* at this stage the port is 'in context of hemisphere' */

            /* make it as global port number of the device */
            port |=  (descrPtr->txqId << 6);
        }

        __LOG(("Forwarding Single Destination Traffic over a Cascade Port [%d]",
            port));

        return port;
    }
    else
    {
        if (egrPortOrTrunkId == 0)
        {
            __LOG(("WARNING : the 'device map table' directed the packet to 'local trunk 0' (non-exist trunk)  --> packet is DROPPED \n"));
        }


        if(calledFromTxdUnit == GT_TRUE)
        {
            __LOG(("(called from the TXD unit) Forwarding Single Destination Traffic over a Cascade Trunk [%d]",
                egrPortOrTrunkId));

            /* when called from the TXD unit we need to return the trunkId , and not
               to select a port */
            return  egrPortOrTrunkId;   /* egress trunk id */
        }
    }


    if((!SKERNEL_IS_CHEETAH1_ONLY_DEV(devObjPtr)) &&
        ((outCmd == SKERNEL_MTAG_CMD_TO_CPU_E) ||
         (outCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E)))
    {
        /* use multi destination hash mode */
        cscdTrunkMode = TRUNK_HASH_MODE_USE_MULIT_DESTINATION_HASH_SETTINGS_E;
        __LOG(("'to_cpu'/'to_analyzer' alwayes use cscdTrunkMode = 'MULIT_DESTINATION_HASH_SETTINGS' \n"));
    }

    __LOG(("Forwarding Single Destination Traffic over a Cascade Trunk [%d]",
        egrPortOrTrunkId));
    regAddr =
        SMEM_CHT_NON_TRUNK_MEMBERS_TBL_MEM(devObjPtr, egrPortOrTrunkId);

    regPtr = smemMemGet(devObjPtr, regAddr);
    SKERNEL_FILL_PORTS_BITMAP_MAC(devObjPtr,&nonTrunkMembersBmp,regPtr);

    snetChtTxGetTrunkDesignatedPorts(devObjPtr, descrPtr, cscdTrunkMode, &desTrunkPort);

    SKERNEL_PORTS_BMP_MEMSET_MAC(&trunkMembersBmp,0xffffffff);
    /* trunkMembersBmp = ~ nonTrunkMembersBmp */
    SKERNEL_PORTS_BITMAP_OPERATORS_MAC(devObjPtr,
        SKERNEL_BITWISE_OPERATOR_NOT_E,&nonTrunkMembersBmp,
        SKERNEL_BITWISE_OPERATOR_AND_E,&trunkMembersBmp,
        &trunkMembersBmp);

    /*sumDesCascadeTrunkBmp.ports = (~nonTrunkMembersBmp.ports) & desTrunkPort.ports;*/
    SKERNEL_PORTS_BITMAP_OPERATORS_MAC(devObjPtr,
        SKERNEL_BITWISE_OPERATOR_NOT_E,&nonTrunkMembersBmp,
        SKERNEL_BITWISE_OPERATOR_AND_E,&desTrunkPort,
        &sumDesCascadeTrunkBmp);


    if(SKERNEL_PORTS_BMP_IS_EMPTY_MAC(devObjPtr,&sumDesCascadeTrunkBmp))
    {
        __LOG(("WARNING : the 'device map table' directed the packet to 'local trunk' [%d] with no members --> packet is DROPPED \n" ,
            egrPortOrTrunkId));

        /* when sumDesCascadeTrunkBmp == 0 it means that the packet should not egress any port */
        /* so we set the destination as 'port not exists' */
        return SNET_CHT_NULL_PORT_CNS;
    }

    SKERNEL_FILL_PORTS_BITMAP_MAC(devObjPtr,
                                  &egressPortsBmp, sumDesCascadeTrunkBmp.ports);

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* do not override the previous assignment */
    }
    else
    if(devObjPtr->supportTxQGlobalPorts)
    {
        /* need to choose a port from all 64 ports of the hemisphere */
        /* even though this is multi-port groups device */
        startPort = 0;
        endPort   = 64;
    }


    LOG_BMP_ARR("cascade trunk members",&trunkMembersBmp.ports[0],endPort);

    numPortsInBmp = 0;
    firstPortInBmp = SMAIN_NOT_VALID_CNS;

    /* Select the designated cascade trunk port */
    __LOG(("Select the designated cascade trunk port"));
    for (port = startPort; port < endPort; port++)
    {
        if (SKERNEL_PORTS_BMP_IS_PORT_SET_MAC(&egressPortsBmp, port))
        {
            if(firstPortInBmp == SMAIN_NOT_VALID_CNS)
            {
                firstPortInBmp = port;
            }
            numPortsInBmp++;
        }
    }

    if (numPortsInBmp >= 2)
    {
        __LOG(("ERROR : (config ERROR) : the 'UC to remote device' via local trunk [%d] 'see' [%d] ports , instead just single one \n"
                "such configuration on HW will cause buffer management errors and could lead to lost of buffers \n",
                egrPortOrTrunkId,numPortsInBmp));

        __LOG(("WARNING : simulation will send the packet only to 'first port' in the BMP \n"));

        __LOG(("The Ports BMP is: \n"
              "0x%8.8x (255..224) "
              "0x%8.8x (223..192) "
              "0x%8.8x (191..160) "
              "0x%8.8x (159..128) \n"
              "0x%8.8x (127..96) "
              "0x%8.8x (95..64) "
              "0x%8.8x (63..32) "
              "0x%8.8x (31..0) \n"
              ,
              egressPortsBmp.ports[7],
              egressPortsBmp.ports[6],
              egressPortsBmp.ports[5],
              egressPortsBmp.ports[4],
              egressPortsBmp.ports[3],
              egressPortsBmp.ports[2],
              egressPortsBmp.ports[1],
              egressPortsBmp.ports[0]
              ));

    }

    port = firstPortInBmp;

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* no modifications to the port */
    }
    else
    if(devObjPtr->numOfTxqUnits >= 2)
    {
        /* at this stage the port is 'in context of hemisphere' */
        /* make it as global port number of the device */
        port |=  (descrPtr->txqId << 6);
    }

    __LOG(("local Port [%d] selected to represent local trunk[%d] ",
        port,egrPortOrTrunkId));

    return port;
}


/**
* @internal snetChtTxQGetEgrPortForTrgDevice function
* @endinternal
*
* @brief   Get egress port for unicast packet
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] calledFromTxdUnit        - is function called from TXD unit(TX distributor -- multi-port groups)
*                                      GT_TRUE - called from TXD unit. in this case if the
*                                      target it 'Cascade trunk', function returns the 'trunkId'
*                                      and not select a trunk member !
*                                      GT_FALSE - not called from TXD unit.
*                                      NOTE: TXD is relevant only to multi-port groups device.
*
* @note The function is relevant for single destination packets only.
*
*/
static GT_U32 snetChtTxQGetEgrPortForTrgDevice
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_BOOL  calledFromTxdUnit
)
{
    DECLARE_FUNC_NAME(snetChtTxQGetEgrPortForTrgDevice);

    SKERNEL_MTAG_CMD_ENT outCmd;              /* Marvell tag command */
    GT_U32               trgDev;              /* Target device */
    GT_U32               regData;             /* Register data */
    GT_U32               targetPort;          /* target port (global port on the device) */
    GT_U32               trgEPort;/* target EPort*/
    GT_U32               startBit;/* start bit */
    GT_U32               coreIdNumBits;/* number of bits for the core Id */
    GT_U32               cpuTargetPipe;/* CPU target core Id */
    GT_U32               srcPort;/* source port for 'use device map table for local traffic'*/

    /* Analyse the outgoing marvell tag command */
    outCmd = descrPtr->outGoingMtagCmd ;

    if ( (outCmd == SKERNEL_MTAG_CMD_FORWARD_E) ||
         (outCmd == SKERNEL_MTAG_CMD_FROM_CPU_E) )
    {
        trgDev = descrPtr->trgDev;
        trgEPort = descrPtr->trgEPort;

        if(SMEM_CHT_IS_SIP5_GET(devObjPtr) &&
            descrPtr->eArchExtInfo.isTrgPhyPortValid == 1)
        {
            targetPort = descrPtr->eArchExtInfo.trgPhyPort ; /*in eArch - use the trgPhyPort*/
        }
        else
        {
            targetPort = trgEPort;/* in legacy device use 'trgEPort' that was set with 'physical port'*/
        }
    }
    else if (outCmd == SKERNEL_MTAG_CMD_TO_CPU_E)
    {
        trgDev = descrPtr->cpuTrgDev;
        trgEPort = SMEM_CHT_IS_SIP5_GET(devObjPtr) ? snetChtEgfCpuPortMapGet(devObjPtr,descrPtr) : descrPtr->trgEPort;/*SNET_CHT_CPU_PORT_CNS*/
        targetPort = trgEPort;
    }
    else
    {
        trgDev = descrPtr->sniffTrgDev;
        trgEPort = descrPtr->sniffTrgPort;
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr) &&
            descrPtr->eArchExtInfo.isTrgPhyPortValid == 1)
        {
            targetPort = descrPtr->eArchExtInfo.trgPhyPort ; /*in eArch - use the trgPhyPort*/
        }
        else
        {
            targetPort = trgEPort;/* in legacy device use 'trgEPort' that was set with 'physical port'*/
        }
    }

    if (SKERNEL_IS_MATCH_DEVICES_MAC(trgDev, descrPtr->ownDev,
                                     devObjPtr->dualDeviceIdEnable.txq))
    {
        if(devObjPtr->txqRevision == 0 ||
           devObjPtr->supportDevMapTableOnOwnDev == 0)
        {
            /* the device not supports accessing the device map table on local traffic  */
            __LOG(("the device map table is not accessed for LOCAL DEVICE traffic \n"
                   "the TARGET port on the LOCAL DEVICE is [%d] \n",
                   targetPort));
            return targetPort;
        }

        if (outCmd == SKERNEL_MTAG_CMD_TO_CPU_E && calledFromTxdUnit == GT_TRUE)
        {
            /* relevant only to multi-core devices */
            /* Check to which port group to handle the 'to_cpu' */

            startBit = 0;
            smemRegGet(devObjPtr, SMEM_LION_TXQ_CPU_PORT_DISRIBUTE_REG(devObjPtr), &regData);

            if(SMEM_U32_GET_FIELD(regData, startBit, 1) == 0)/*CpuPortMode */
            {
                __LOG(("The 'to_cpu' is in GLOBAL MODE \n"));

                if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
                {
                    coreIdNumBits = 4;
                }
                else
                {
                    coreIdNumBits = 2;
                }

                cpuTargetPipe = SMEM_U32_GET_FIELD(regData, (startBit + 1), coreIdNumBits);
                /* build the target port from the cpuTargetPipe */
                targetPort = cpuTargetPipe  << CORE_ID_NUM_BITS_LEFT_SHIFT_FOR_GLOBAL_PORT_CNS;

                /* set port with 2 or 4 MSB according to the CpuTargetPipe*/
                __LOG(("The 'TO_CPU' is in GLOBAL MODE, set port with %d MSB according to the CpuTargetPipe[%d] \n",coreIdNumBits,cpuTargetPipe));
                if((devObjPtr->portGroupId & SMEM_BIT_MASK(coreIdNumBits))/* use only 2 or 4 bits */
                    != cpuTargetPipe)
                {
                    __LOG(("the TO_CPU representative port' is [%d] \n",
                            targetPort));
                    return targetPort;
                }
            }
            else
            {
                __LOG(("The 'TO_CPU' is in LOCAL MODE , so send to this core CPU port \n"));
            }

            if(descrPtr->forceToCpuTrgPortOnHemisphare0)/* not relevant to sip5 */
            {
                targetPort = ((devObjPtr->portGroupId & 0x3) << 4);

                /* return port as if the core belongs to hemisphere 0*/
                __LOG(("return port as if the core belongs to hemisphere 0 \n"
                       "the TO_CPU representative port' is [%d] \n",
                        targetPort));
                return targetPort;
            }
        }

        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            srcPort = descrPtr->localPortGroupPortAsGlobalDevicePort;/*global port*/
        }
        else
        {
            srcPort = descrPtr->localDevSrcPort;/*local port*/
        }

        if(GT_FALSE == snetLionTxqDeviceMapTableAccessCheck(devObjPtr, descrPtr,
                                                            srcPort,
                                                            trgDev, trgEPort))
        {
            __LOG(("the device map table is not accessed for LOCAL DEVICE traffic \n"
                   "the TARGET port on the LOCAL DEVICE is [%d] \n",
                   targetPort));

            return targetPort; /* target port (global port on the device) */
        }
    }

    /* access the device map table */
    __LOG(("access the device map table \n"));
    targetPort = snetChtTxQDeviceMapTableAccess(devObjPtr, descrPtr, trgDev,
                                          trgEPort, calledFromTxdUnit);

    if(targetPort == SMAIN_NOT_VALID_CNS)
    {
        /* indication that the device map table WAS not accessed */
        SIM_TBD_BOOKMARK
        targetPort =  SNET_CHT_NULL_PORT_CNS;
        __LOG(("the access to the device map table is disabled -->"
            "use DUMMY implementation treat packet to be DROPPED - use port [%d] \n",
            targetPort));
    }

    __LOG(("the TARGET port on the LOCAL DEVICE from device map table is [%d] \n",
            targetPort));

    return targetPort;

}

/**
* @internal snetChtTxQFilters function
* @endinternal
*
* @brief   filters transmit packet.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in,out] destPorts[]              - number of egress port.
* @param[in,out] stpVector[]              - stp state port vector.
* @param[in] packetType               - type of packet.
* @param[in,out] pktFilteredPortsBmpPtr   - bitmap of filtered egress ports
*
* @param[out] forwardRestrictionBmpPtr - forwarding restriction ports
*/
static void snetChtTxQFilters
(
  IN SKERNEL_DEVICE_OBJECT * devObjPtr,
  IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
  INOUT GT_U32 destPorts[],
  INOUT SKERNEL_STP_ENT stpVector[],
  IN SKERNEL_EGR_PACKET_TYPE_ENT packetType,
  INOUT SKERNEL_PORTS_BMP_STC *pktFilteredPortsBmpPtr,
  IN GT_U32 oamLoopbackFilterEn[],
  OUT SKERNEL_PORTS_BMP_STC *forwardRestrictionBmpPtr
)
{
    DECLARE_FUNC_NAME(snetChtTxQFilters);

    GT_U32                 backupDestPort[SKERNEL_CHEETAH_EGRESS_MAX_PORT_CNS];
    SKERNEL_PORTS_BMP_STC  saveDestPort; /* bitmap of egress ports */
    GT_U32                 port; /* current port */
    GT_U32                 maxPorts;/* max ports to loop on*/
    GT_U32                 egrPortAmount;
    SIM_LOG_TARGET_ARR_DECLARE(destPorts);
    SIM_LOG_TARGET_ARR_DECLARE(backupDestPort);

    /* initiation of backupDestPort vector  */
    memset(backupDestPort,0,sizeof(backupDestPort));

    SIM_LOG_TARGET_ARR_SAVE(destPorts);
    SIM_LOG_TARGET_ARR_SAVE(backupDestPort);

    SKERNEL_PORTS_BMP_CLEAR_MAC(&saveDestPort);

    snetChtTxQPortLinkStat( devObjPtr, descrPtr, destPorts,
                       &egrPortAmount, backupDestPort, &saveDestPort,
                       oamLoopbackFilterEn);

    SIM_LOG_TARGET_ARR_COMPARE(destPorts);
    SIM_LOG_TARGET_ARR_COMPARE(backupDestPort);

    if (egrPortAmount == 0)
    {
        __LOG(("snetChtTxQPortLinkStat : no ports to send to \n"));
    }

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* the EGF see all 256 ports */
        maxPorts = SNET_SIP5_EGR_MAX_PHY_PORTS_MAC(devObjPtr);
    }
    else
    {
        maxPorts = SNET_CHEETAH_MAX_PORT_NUM(devObjPtr);
    }

    SIM_LOG_TARGET_ARR_SAVE(destPorts);

    /* Port Egress Forwarding Restriction for cht2 */
    snetChtTxQPortFrwrdRestriction(devObjPtr, descrPtr, destPorts,
                                   oamLoopbackFilterEn, forwardRestrictionBmpPtr);
    SIM_LOG_TARGET_ARR_COMPARE(destPorts);
    SIM_LOG_TARGET_ARR_SAVE(destPorts);

    if (descrPtr->useVidx)
    {
        /* source port filtering */
        __LOG(("check for source port filtering \n"));
        snetChtTxQSrcPortFilters(devObjPtr, descrPtr, destPorts, packetType);
        SIM_LOG_TARGET_ARR_COMPARE(destPorts);
        SIM_LOG_TARGET_ARR_SAVE(destPorts);

        /* trunk related filters */
        __LOG(("check for trunk related filters \n"));
        snetChtTxQTrunkFilters(devObjPtr, descrPtr, destPorts, packetType);
        SIM_LOG_TARGET_ARR_COMPARE(destPorts);
        SIM_LOG_TARGET_ARR_SAVE(destPorts);
    }

    if(devObjPtr->supportPortIsolation)
    {
        /* Port Isolation */
        __LOG(("check for Port Isolation filters \n"));
        snetXCatTxQPortIsolationFilters(devObjPtr, descrPtr, destPorts);
        SIM_LOG_TARGET_ARR_COMPARE(destPorts);
        SIM_LOG_TARGET_ARR_SAVE(destPorts);
    }

    /* SrcId egress filtering for multi- and single- destination traffic */
    __LOG(("check for SrcId egress filtering \n"));
    snetChtTxQSSTFilters(devObjPtr, descrPtr, destPorts);
    SIM_LOG_TARGET_ARR_COMPARE(destPorts);
    SIM_LOG_TARGET_ARR_SAVE(destPorts);

    /* OAM egress filtering for multi- and single- destination traffic */
    __LOG(("check for OAM egress filtering \n"));
    snetChtTxQOamFilter(devObjPtr, descrPtr, packetType,destPorts,oamLoopbackFilterEn);
    SIM_LOG_TARGET_ARR_COMPARE(destPorts);
    SIM_LOG_TARGET_ARR_SAVE(destPorts);

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        if((descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_FORWARD_E)||
           (descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_FROM_CPU_E))
        {
            /* Egress MeshId filtering for multi- and single- destination traffic */
            __LOG(("check for Egress MeshId filtering \n"));
            snetChtTxQEgressMeshIdFilters(devObjPtr, descrPtr, packetType, destPorts);
            SIM_LOG_TARGET_ARR_COMPARE(destPorts);
            SIM_LOG_TARGET_ARR_SAVE(destPorts);

            if(SMEM_CHT_IS_SIP5_10_GET(devObjPtr))
            {
                /* Egress VID1 Mismatch filtering for multi- and single- destination traffic */
                __LOG(("check for Egress VID1 Mismatch filtering \n"));
                snetChtTxQEgressVid1MismatchFilter(devObjPtr, descrPtr, destPorts);
                SIM_LOG_TARGET_ARR_COMPARE(destPorts);
                SIM_LOG_TARGET_ARR_SAVE(destPorts);
            }
        }
    }

    /* Single destination packets are not subject to further processing */
    if(descrPtr->useVidx)
    {
        if(descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_FROM_CPU_E)
        {
            /* filters relevant to FROM_CPU */
            /* exclude port/trunk filter (relevant to 'from cpu' with 'use vidx') */
            snetChtTxQFromCpuExcludePortOrTrunk(devObjPtr, descrPtr, destPorts);
            SIM_LOG_TARGET_ARR_COMPARE(destPorts);
            SIM_LOG_TARGET_ARR_SAVE(destPorts);
        }

        if (packetType != SKERNEL_EGR_PACKET_CNTRL_MCAST_E)
        {
            /* stp filters */
            __LOG(("check for stp filters \n"));
            snetChtTxQStpFilters(devObjPtr, descrPtr, destPorts, stpVector,oamLoopbackFilterEn);
            SIM_LOG_TARGET_ARR_COMPARE(destPorts);
            SIM_LOG_TARGET_ARR_SAVE(destPorts);

            if (packetType != SKERNEL_EGR_PACKET_BRG_REG_MCAST_BCAST_E)
            {  /* unknown unicast/multicast filter */
                __LOG(("check for unknown unicast/multicast filter \n"));
                snetChtTxQUnknUnregFilters(devObjPtr,descrPtr,destPorts,packetType);
                SIM_LOG_TARGET_ARR_COMPARE(destPorts);
                SIM_LOG_TARGET_ARR_SAVE(destPorts);
            }
        }
        __LOG(("check for 'backup ports' to replace link DOWN ports \n"));
        snetCht3TxqDestBackUpPorts(
            devObjPtr, descrPtr, backupDestPort, destPorts);
        SIM_LOG_TARGET_ARR_COMPARE(destPorts);
        SIM_LOG_TARGET_ARR_SAVE(destPorts);
    }

    /* remove new added destination ports from pktFilteredPortsBmpPtr */
    for (port = 0; port < maxPorts; port++)
    {
        if (destPorts[port] != 0)
        {
            SKERNEL_PORTS_BMP_DEL_PORT_MAC(pktFilteredPortsBmpPtr, port);
        }
    }

    /* add filtered ports to  pktFilteredPortsBmpPtr */
    for (port = 0; port < maxPorts; port++)
    {
        if ((destPorts[port] == 0) &&
            SKERNEL_PORTS_BMP_IS_PORT_SET_MAC(&saveDestPort, port))
        {
            SKERNEL_PORTS_BMP_ADD_PORT_MAC(pktFilteredPortsBmpPtr, port);
        }
    }
}

typedef enum{
    SNET_CHT_EGF_SHT_TABLE_NON_TRUNK_MEMBERS_2_TYPE_E,
    SNET_CHT_EGF_SHT_TABLE_SRC_ID_TYPE_E
}SNET_CHT_EGF_SHT_TABLE_TYPE_ENT;

/**
* @internal snetFalconEgfShtEntryBmpGet function
* @endinternal
*
* @brief   EGF-SHT : falcon specific : fill the bmp of ports that relate to table entry
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] tableType                - the table type
* @param[in] origIndex                - the index to use to the table
* @param[in] endPortPtr               - (pointer to) initialized last port in the bitmap
*
* @param[out] egressPortBmpPtr         - Egress ports bitmap
* @param[out] endPortPtr               - (pointer to) updated last port in the bitmap
*/
static void snetFalconEgfShtEntryBmpGet(
  IN SKERNEL_DEVICE_OBJECT              * devObjPtr,
  IN SKERNEL_FRAME_CHEETAH_DESCR_STC    * descrPtr,
  IN SNET_CHT_EGF_SHT_TABLE_TYPE_ENT    tableType,
  IN GT_U32                             origIndex,
  OUT SKERNEL_PORTS_BMP_STC             *egressPortBmpPtr,
  INOUT GT_U32                          *endPortPtr
)
{
    GT_U32 regAddr,*regPtr;
    GT_CHAR*  name   = tableType == SNET_CHT_EGF_SHT_TABLE_NON_TRUNK_MEMBERS_2_TYPE_E ?
                        "egress trunk filtering" :
                        "egress SST";
    GT_U32  maxLines = tableType == SNET_CHT_EGF_SHT_TABLE_NON_TRUNK_MEMBERS_2_TYPE_E ?
                        (1 << devObjPtr->flexFieldNumBitsSupport.trunkId) :
                        (1 << devObjPtr->flexFieldNumBitsSupport.sstId);

    GT_U32  numLines = maxLines / (1 << (descrPtr->tableAccessMode.EGF_SHT_TableMode));
    /* TOTAL 2K lines of 128 bits */
    /* mode 64 have 2 entries per line (1/2 line)*/
    /* mode 128 have 1 line per STG */
    /* mode 256,512,1024 have 2,4,8 lines per STG */
    /* in mode  64  port : entry 1 : hold 1/2 line: line  0 (64 bits out of 128) */
    /* in mode 128  port : entry 1 : hold 1 line  : lines 1 */
    /* in mode 256  port : entry 1 : hold 2 lines : lines 1 ,256+1 */
    /* in mode 512  port : entry 1 : hold 4 lines : lines 1 ,256+1,,512+1,768+1*/
    /* in mode 1024 port : entry 1 : hold 8 lines : lines 1,256+1,512+1,768+1 ,1K+1,1K+256+1,1K+512+1,1K+768+1 */
    GT_U32  numLinesPerEntry = descrPtr->tableAccessMode.EGF_SHT_TableMode == 0 ? 1 /*0.5*/:
                                  (1 << (descrPtr->tableAccessMode.EGF_SHT_TableMode - 1));
    GT_U32  ii,jj;
    GT_U32  currIndex = origIndex;
    GT_U32  numPortsToJump = SHT_TABLE_SIP6_LINE_WIDTH ;
    GT_U32  jjStart = 0,jjMax = numPortsToJump / 32;
    GT_U32 chunkSize = SHT_NON_VLAN_TABLE_SIP6_CHUNK_SIZE;
    GT_U32 wordStart = 0;

    currIndex  = (descrPtr->tableAccessMode.EGF_SHT_TableMode == 0? currIndex/2 :
                  (((origIndex/chunkSize)* (chunkSize * numLinesPerEntry)) + (origIndex%chunkSize)));

    *endPortPtr = (SHT_TABLE_SIP6_LINE_WIDTH/2)/*64*/ << descrPtr->tableAccessMode.EGF_SHT_TableMode;

    SKERNEL_HALF_TABLE_MODE_VIOLATION_CHECK_AND_REPORT_MAC(devObjPtr,descrPtr,name,
        origIndex,currIndex,numLines,
        numLinesPerEntry,SHT_NON_VLAN_TABLE_SIP6_CHUNK_SIZE);

    if(descrPtr->tableAccessMode.EGF_SHT_TableMode == 0)
    {
        jjMax = 2;/* only 2 words (64 bits) */
        if(origIndex & 1)
        {
            wordStart = 2;/* jump 2 words (64 bits) */
        }
    }

    for( ii = 0 ; ii < numLinesPerEntry; ii++)
    {
        if(tableType == SNET_CHT_EGF_SHT_TABLE_NON_TRUNK_MEMBERS_2_TYPE_E)
        {
            regAddr = SMEM_LION2_EGF_SHT_NON_TRUNK_MEMBERS_2_TBL_MEM(devObjPtr, currIndex);
        }
        else
        {
            regAddr = SMEM_CHT_SST_EGRESS_FILTERING_TBL_MEM(devObjPtr, currIndex);
        }

        regPtr = smemMemGet(devObjPtr, regAddr);

        for(jj = 0 ; jj < jjMax; jj++)
        {
            egressPortBmpPtr->ports[jj+jjStart] = regPtr[jj + wordStart];
        }

        currIndex += chunkSize;
        jjStart   += jjMax;
    }
}

/**
* @internal snetChtEgfShtEntryBmpGet function
* @endinternal
*
* @brief   EGF-SHT : fill the bmp of ports that relate to table entry
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] tableType                - the table type
* @param[in] origIndex                - the index to use to the table
* @param[in] endPortPtr               - (pointer to) initialized last port in the bitmap
*
* @param[out] egressPortBmpPtr         - Egress ports bitmap
* @param[out] endPortPtr               - (pointer to) updated last port in the bitmap
*/
static void snetChtEgfShtEntryBmpGet
(
  IN SKERNEL_DEVICE_OBJECT * devObjPtr,
  IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
  IN SNET_CHT_EGF_SHT_TABLE_TYPE_ENT    tableType,
  IN GT_U32                     origIndex,
  OUT SKERNEL_PORTS_BMP_STC *egressPortBmpPtr,
  INOUT GT_U32  *endPortPtr
)
{
    GT_U32 regAddr,*regPtr;

    switch(tableType)
    {
        case SNET_CHT_EGF_SHT_TABLE_NON_TRUNK_MEMBERS_2_TYPE_E:
            if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
            {
                /* use 'table 2' for regular trunk filtering */
                regAddr =
                    SMEM_LION2_EGF_SHT_NON_TRUNK_MEMBERS_2_TBL_MEM(devObjPtr, origIndex);
                regPtr = smemMemGet(devObjPtr, regAddr);
                /* fill the info (128 ports) */
                SKERNEL_FILL_FIRST_HALF_PORTS_BITMAP_MAC(devObjPtr, egressPortBmpPtr, regPtr);
            }
            else
            if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
            {
                snetFalconEgfShtEntryBmpGet(devObjPtr,descrPtr,tableType,origIndex,egressPortBmpPtr,endPortPtr);
                return;
            }
            else
            {
                if(descrPtr->tableAccessMode.EGF_SHT_TableMode)
                {
                    SKERNEL_HALF_TABLE_MODE_VIOLATION_CHECK_AND_REPORT_MAC(devObjPtr,descrPtr,"egress trunk filtering",
                        origIndex,origIndex,(GT_U32)(1 << (devObjPtr->flexFieldNumBitsSupport.trunkId)),
                        2,0);
                }

                if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
                {
                    /* use 'table 2' for regular trunk filtering */
                    regAddr =
                        SMEM_LION2_EGF_SHT_NON_TRUNK_MEMBERS_2_TBL_MEM(devObjPtr, origIndex);
                }
                else
                {
                    regAddr =
                        SMEM_CHT_NON_TRUNK_MEMBERS_TBL_MEM(devObjPtr, origIndex);
                }
                regPtr = smemMemGet(devObjPtr, regAddr);

                /* fill first half the info (256 ports) */
                SKERNEL_FILL_FIRST_HALF_PORTS_BITMAP_MAC(devObjPtr, egressPortBmpPtr, regPtr);

                if(descrPtr->tableAccessMode.EGF_SHT_TableMode == 1 &&
                   SNET_SIP5_EGR_MAX_PHY_PORTS_MAC(devObjPtr) > 256)
                {
                    /* use 'table 2' for regular trunk filtering
                       jump half table to get the rest of the entry
                    */
                    regAddr =
                        SMEM_LION2_EGF_SHT_NON_TRUNK_MEMBERS_2_TBL_MEM(devObjPtr,
                            origIndex +
                            (1<<(devObjPtr->flexFieldNumBitsSupport.trunkId-1)));
                    regPtr = smemMemGet(devObjPtr, regAddr);

                    /* fill second half the info */
                    SKERNEL_FILL_SECOND_HALF_PORTS_BITMAP_MAC(devObjPtr, egressPortBmpPtr, regPtr);
                }
            }
        break;

        case SNET_CHT_EGF_SHT_TABLE_SRC_ID_TYPE_E:
            if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
            {
                regAddr =
                    SMEM_CHT_SST_EGRESS_FILTERING_TBL_MEM(devObjPtr, origIndex);
                /* Ports bitmap indicating if the packet with this Source-ID
                   may be forwarded to this port */
                regPtr = smemMemGet(devObjPtr, regAddr);
                /* fill the info (128 ports) */
                SKERNEL_FILL_FIRST_HALF_PORTS_BITMAP_MAC(devObjPtr, egressPortBmpPtr, regPtr);
            }
            else
            if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
            {
                snetFalconEgfShtEntryBmpGet(devObjPtr,descrPtr,tableType,origIndex,egressPortBmpPtr,endPortPtr);
                return;
            }
            else
            {
                if(descrPtr->tableAccessMode.EGF_SHT_TableMode)
                {
                    SKERNEL_HALF_TABLE_MODE_VIOLATION_CHECK_AND_REPORT_MAC(devObjPtr,descrPtr,"egress SST",
                        origIndex,origIndex,
                        (GT_U32)(1 << (devObjPtr->flexFieldNumBitsSupport.sstId)),
                        2,0);
                }

                regAddr =
                    SMEM_CHT_SST_EGRESS_FILTERING_TBL_MEM(devObjPtr, origIndex);
                /* Ports bitmap indicating if the packet with this Source-ID
                   may be forwarded to this port */
                regPtr = smemMemGet(devObjPtr, regAddr);

                /* fill first half the info (256 ports) */
                SKERNEL_FILL_FIRST_HALF_PORTS_BITMAP_MAC(devObjPtr, egressPortBmpPtr, regPtr);

                if(descrPtr->tableAccessMode.EGF_SHT_TableMode == 1 &&
                   SNET_SIP5_EGR_MAX_PHY_PORTS_MAC(devObjPtr) > 256)
                {
                    /*
                       jump half table to get the rest of the entry
                    */
                    regAddr =
                        SMEM_CHT_SST_EGRESS_FILTERING_TBL_MEM(devObjPtr,
                            origIndex +
                            (1<<(devObjPtr->flexFieldNumBitsSupport.sstId-1)));
                    regPtr = smemMemGet(devObjPtr, regAddr);

                    /* fill second half the info */
                    SKERNEL_FILL_SECOND_HALF_PORTS_BITMAP_MAC(devObjPtr, egressPortBmpPtr, regPtr);
                }
            }
        break;

        default:
            return;
    }

    return;
}


/**
* @internal snetChtTxQFromCpuExcludePortOrTrunk function
* @endinternal
*
* @brief   Exclude Port or Trunk for From CPU "to vidx" packet with exclude Port/Trunk
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in,out] destPorts[]              - number of egress port.
*/
static void snetChtTxQFromCpuExcludePortOrTrunk
(
  IN SKERNEL_DEVICE_OBJECT * devObjPtr,
  IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
  INOUT GT_U32 destPorts[]
)
{
    DECLARE_FUNC_NAME(snetChtTxQFromCpuExcludePortOrTrunk);

    GT_U32 port;            /* port index */
    GT_U32 localPort;       /* local Port number */
    GT_U32 startPort;       /* Start iteration port */
    GT_U32 endPort;         /* End iteration port */
    SKERNEL_PORTS_BMP_STC egressPortBmp; /* Egress ports bitmap */
    GT_U32  srcFilter_isTrunk;/* the isTrunk to use for src filtering */
    GT_U32  srcFilter_portTrunk;/* the port/trunk to use for src filtering */
    GT_U32  doFilter;

    if(descrPtr->marvellTaggedExtended == SKERNEL_EXT_DSA_TAG_1_WORDS_E)
    {
        /* no indication of excluded info in this DSA */
        return;
    }

    /*from_cpu :  MC excluded filtering for FromCpu */
    srcFilter_isTrunk   = descrPtr->excludeIsTrunk;
    srcFilter_portTrunk = descrPtr->excludeIsTrunk ?
                                descrPtr->excludedTrunk :
                                descrPtr->excludedPort;

    if(srcFilter_isTrunk && srcFilter_portTrunk == 0)
    {
        /* indication the that 'no filter' needed */
        __LOG(("no filter because :  excludeIsTrunk with excludedTrunk == 0 \n"));
        return;
    }


    if(srcFilter_isTrunk == 0 &&
       descrPtr->eArchExtInfo.fromCpu.excludedIsPhyPort == 1)
    {
        /* set values to make sure 'no match' of the ePort */
    }
    else
    {
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr) && srcFilter_isTrunk == 0)
        {
            __LOG(("no filter because : excludedIsPhyPort == 0 \n"));
            __LOG(("excluding eport is relevant only to the L2 MLL (and not to EGF) \n"));
            return;
        }

        /* legacy devices can be considered as filtering the EPort !!! */
    }

    startPort = SNET_CHT_EGR_TXQ_START_PORT_MAC(devObjPtr);
    endPort = SNET_CHT_EGR_TXQ_END_PORT_MAC(devObjPtr);

    if (srcFilter_isTrunk)    /* excluded trunk */
    {
        __LOG(("%s : do src filtering of 'excluded trunk' from FROM_CPU to trunkId[%d] \n",
            SMEM_CHT_IS_SIP5_GET(devObjPtr) ?  "NON_TRUNK_MEMBERS_2" : "NON_TRUNK_MEMBERS",
            srcFilter_portTrunk));

        /* get the BMP of 'NON_TRUNK_MEMBERS_2' (NOTE: in non-sip5 this is NON_TRUNK_MEMBERS) */
        snetChtEgfShtEntryBmpGet(devObjPtr,descrPtr,
            SNET_CHT_EGF_SHT_TABLE_NON_TRUNK_MEMBERS_2_TYPE_E,
            srcFilter_portTrunk,
            &egressPortBmp,&endPort);

        for (port = startPort,localPort = 0; port < endPort; port++ , localPort++)
        {
            if(SKERNEL_PORTS_BMP_IS_PORT_SET_MAC(&egressPortBmp, port) == 0)
            {
                destPorts[localPort] = 0;
                __LOG(("excluded port [%d] (of orig trunk) \n" ,localPort));
            }
        }
    }
    else  /* excluded port */
    {
        if(descrPtr->excludedDevice != descrPtr->ownDev)
        {
            /* the filter is not on this device */
            __LOG(("no filter because :  excludedDevice[%d] != descrPtr->ownDev[%d]  \n",
                descrPtr->excludedDevice ,
                descrPtr->ownDev));
            return;
        }

        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /* no conversions needed */
            localPort = srcFilter_portTrunk;
            doFilter = 1;
        }
        else
        {
            localPort = SMEM_CHT_LOCAL_PORT_FROM_GLOBAL_PORT_MAC(devObjPtr,srcFilter_portTrunk);

            if(devObjPtr->portGroupSharedDevObjPtr == NULL)
            {
                doFilter = 1;
            }
            else
            {
                doFilter = (SMEM_CHT_PORT_GROUP_ID_FROM_GLOBAL_PORT_MAC(devObjPtr,srcFilter_portTrunk) ==
                            SMEM_CHT_PORT_GROUP_ID_MASK_CORE_MAC(devObjPtr, devObjPtr->portGroupId, devObjPtr->dualDeviceIdEnable.txq)) ?
                            1 : 0;
            }
        }

        if (doFilter)
        {
            destPorts[localPort] = 0 ;
            __LOG(("excluded port [%d] \n" ,localPort));
        }
    }
}

/**
* @internal snetChtIsTxPortLinkUp function
* @endinternal
*
* @brief   Is the MAC port considered link UP/DOWN
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] calledFromEgf            - indication that called from EGF or from Egress MAC
* @param[in] macPort                  -  number of egress MAC port.
*
*   return values:
*           GT_TRUE  - the TX MAC port considered 'link UP'
*           GT_FALSE - the TX MAC port considered 'link DOWN'
*
*/
static GT_BOOL snetChtIsTxPortLinkUp
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_BOOL                  calledFromEgf,
    IN GT_U32                   macPort
)
{
    DECLARE_FUNC_NAME(snetChtIsTxPortLinkUp);

    GT_U32  fldValue,uniDirectionalEn;

    fldValue = snetChtPortMacFieldGet(devObjPtr, macPort,
        SNET_CHT_PORT_MAC_FIELDS_open_drain_E);
    if(fldValue)
    {
        /* the MAC will not 'see' the traffic ... it is going 'down the train' */
        __LOG(("WARNING: packet DROPPED at Egress MAC [%d] due to 'open drain' \n",
            macPort));

        /* increment the open drain drop counter */
        snetChtPortMacFieldSet(devObjPtr, macPort,
            SNET_CHT_PORT_MAC_FIELDS_open_drain_counter_increment_E,0/*don't care*/);

        return GT_FALSE;
    }


    if(SKERNEL_IS_CHEETAH3_DEV(devObjPtr))
    {
        uniDirectionalEn = snetChtPortMacFieldGet(devObjPtr, macPort,
            SNET_CHT_PORT_MAC_FIELDS_Unidirectional_enable_support_802_3ah_OAM_E);
    }
    else
    {
        uniDirectionalEn = 0;
    }

    fldValue = snetChtPortMacFieldGet(devObjPtr, macPort,
        SNET_CHT_PORT_MAC_FIELDS_LinkState_E);
    /* LinkUp */
    if (fldValue != 1)
    {
        if(uniDirectionalEn == 0)
        {
            if(calledFromEgf == GT_FALSE)
            {
                __LOG(("WARNING: packet DROPPED at Egress MAC [%d] due to 'LINK DOWN' (should have been dropped by the 'EGF')\n",
                    macPort));
            }
            return GT_FALSE;
        }
        else
        {
            if(calledFromEgf == GT_FALSE)
            {
                __LOG(("packet successfully egress the Egress MAC [%d] even though 'LINK DOWN' due to 'uniDirectionalEn = 1' \n",
                    macPort));
            }
        }
    }

    if (CHT_IS_MTI_MAC_USED_MAC(devObjPtr, macPort))
    {
        /* check MTI port TX path enable status */
        fldValue = snetChtPortMacFieldGet(devObjPtr, macPort,
            SNET_CHT_PORT_MAC_FIELDS_tx_path_en_E);
    }
    else
    {
        /* check Port Enable bit for regular ports and MII/RGMII CPU Ports */
        fldValue = snetChtPortMacFieldGet(devObjPtr, macPort,
            SNET_CHT_PORT_MAC_FIELDS_PortEn_E);
    }
    /* PortEn */
    if (fldValue != 1)
    {
        if(uniDirectionalEn == 0)
        {
            if(calledFromEgf == GT_FALSE)
            {
                __LOG(("WARNING: packet DROPPED at Egress MAC [%d] due to 'PORT DISABLED' (should have been dropped by the 'EGF') \n",
                    macPort));
            }
            return GT_FALSE;
        }
        else
        {
            if(calledFromEgf == GT_FALSE)
            {
                __LOG(("packet successfully egress the Egress MAC [%d] even though 'PORT DISABLED' due to 'uniDirectionalEn = 1' \n",
                    macPort));
            }
        }
    }

    return GT_TRUE;
}


typedef enum{
    FORCE_LINK_UP_E,
    FORCE_LINK_DOWN_E,
    LINK_ACCORDING_TO_MAC_E,
    LINK_UNKNOWN_E
}LINK_CASE_ENT;

/**
* @internal snetCht3TxqDestBackUpPorts function
* @endinternal
*
* @brief   the"real ports" for backup only port that are linked up but maybe egressed
*         from other reason can be backup
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in,out] destPorts[]              - number of egress port.
*                                      backup ports    - pores that are linked up but where egressed later
*/
static void snetCht3TxqDestBackUpPorts
(
  IN SKERNEL_DEVICE_OBJECT * devObjPtr,
  IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
  IN GT_U32 backupDestPort[],
  INOUT GT_U32 destPorts[]
)
{
    DECLARE_FUNC_NAME(snetCht3TxqDestBackUpPorts);

    GT_U32  port;              /* Port */
    GT_U32  regAddress;        /* Register address */
    GT_U32 *regPtr;            /* Pointer to register data */
    GT_U32  loopPort;          /* Port */
    GT_U32  linkUP;            /* Link up/down */
    SKERNEL_DEVICE_OBJECT * loopPortDevObjPtr;/* the target core that is may be not the current core */
    GT_U32 globalLoopPort;/* the global port of the backup port */
    GT_U32 targetPortGroupId;/* the target port groupId for the backup port */
    GT_U32      macPort;     /* MAC Port number */
    GT_U32 physicalPort;     /* physical Port number */
    GT_U32 physicalPortLinkStatusMask;/*Sets the link status for the port, for local ports on this device use 0, otherwise the configuration must implement the link state.
                         0x0 = LinkState; LinkState; Filtering is performed based on the port link state in
                         0x1 = ForceLinkUp; ForceLinkUp; Packets to this physical port are not filtered
                         0x2 = ForceLinkDown; ForceLinkDown; Packets to this physical port are always filtered
                        */
    SKERNEL_DEVICE_OBJECT  *txDmaDevObjPtr;/*pointer to the 'TX DMA port' egress device object*/
    LINK_CASE_ENT   linkCase;

    if(! SKERNEL_IS_CHEETAH3_DEV(devObjPtr))
    {
        return;
    }

    for (port = 0; port < SKERNEL_DEV_EGRESS_MAX_PORT_CNS(devObjPtr); port++)
    {
        if (backupDestPort[port] == 0 || destPorts[port] == 0)
        {
            continue;
        }

        simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_EGF_EFT_SHT_QAG_E);

        /* port was not filtered by any TxQ filtering -- but this port is
           actually "down"/"tx disabled" .


           so we will replace this port with it's "backup" port ,
           (that also must be "UP"and "tx enabled")
        */

        destPorts[port] = 0;
        regAddress = SMEM_CHT3_SECONDARY_TARGET_PORT_MAP_TBL_MEM (devObjPtr, port);
        regPtr = smemMemGet(devObjPtr, regAddress);
        loopPort =
            SNET_CHT_SECOND_PORT_FAST_STACK_FAILOVE_GET_MAC(devObjPtr, regPtr);

        __LOG(("target port [%d] checked to be replaced by port [%d] as 'backup' \n",
            port,
            loopPort));

        /* set the target port group which represents the backup port */
        loopPortDevObjPtr = devObjPtr;

        globalLoopPort = loopPort;
        targetPortGroupId = 0;

        if(SMEM_CHT_IS_SIP5_GET(loopPortDevObjPtr))
        {
            /* no need for port group,local port conversion */
            targetPortGroupId = devObjPtr->portGroupId;
        }
        else
        if(devObjPtr->portGroupSharedDevObjPtr)
        {
            targetPortGroupId = SMEM_CHT_PORT_GROUP_ID_FROM_GLOBAL_PORT_MAC(devObjPtr,globalLoopPort);
            /* convert to local port */
            loopPort = SMEM_CHT_LOCAL_PORT_FROM_GLOBAL_PORT_MAC(devObjPtr,globalLoopPort);

            if((targetPortGroupId != devObjPtr->portGroupId) && (targetPortGroupId < devObjPtr->portGroupSharedDevObjPtr->numOfCoreDevs))
            {
                loopPortDevObjPtr = devObjPtr->portGroupSharedDevObjPtr->coreDevInfoPtr[targetPortGroupId].devObjPtr;
            }
        }

        if(SMEM_CHT_IS_SIP5_GET(loopPortDevObjPtr))
        {
            if(!SMEM_CHT_IS_SIP5_20_GET(loopPortDevObjPtr))
            {
                GT_U32  numPortsInReg  = 4;
                GT_U32  numBitsPerPort = 8;

                regAddress = SMEM_LION2_EGF_EFT_VIRTUAL_2_PHYSICAL_PORT_REMAP_REG(loopPortDevObjPtr,(port/numPortsInReg));
                /* map 'virtual port' to 'physical port' for the link filter */
                smemRegFldGet(loopPortDevObjPtr, regAddress, numBitsPerPort*(port%numPortsInReg), numBitsPerPort, &physicalPort);

                __LOG(("map 'virtual port' [%d] to 'physical port' [%d] for the link filter \n",
                        port,
                        physicalPort));

                regAddress = SMEM_LION2_EGF_EFT_PHYSICAL_PORT_LINK_STATUS_MASK_REG(loopPortDevObjPtr,(physicalPort/16));
                /* get link filter mode */
                smemRegFldGet(loopPortDevObjPtr, regAddress, 2*(physicalPort%16), 2, &physicalPortLinkStatusMask);
                __LOG(("link filter mode for 'physical port' [%d] is [%s] \n",
                        physicalPort,
                        physicalPortLinkStatusMask == 0 ? "according to MAC register" :
                        physicalPortLinkStatusMask == 1 ? "force link UP" :
                        physicalPortLinkStatusMask == 2 ? "force link DOWN" :
                        " unknown "
                        ));

                linkCase = (physicalPortLinkStatusMask == 0) ? LINK_ACCORDING_TO_MAC_E:
                           (physicalPortLinkStatusMask == 1) ? FORCE_LINK_UP_E:
                           (physicalPortLinkStatusMask == 2) ? FORCE_LINK_DOWN_E:
                                                               LINK_UNKNOWN_E;
            }
            else
            {
                /* there is no mapping between virtual port to physical port */
                /* and there is only single bit for per port */
                physicalPort = port;
                __LOG(("(sip5.20: no mapping) : map 'virtual port' [%d] to 'physical port' [%d] for the link filter \n",
                        port,
                        physicalPort));

                regAddress = SMEM_LION2_EGF_EFT_PHYSICAL_PORT_LINK_STATUS_MASK_REG(loopPortDevObjPtr,(physicalPort/32));
                /* get link filter mode */
                smemRegFldGet(loopPortDevObjPtr, regAddress, (physicalPort%32), 1, &physicalPortLinkStatusMask);
                __LOG(("link filter mode for 'Virtual port' [%d] is [%s] \n",
                        physicalPort,
                        physicalPortLinkStatusMask == 0 ? "force link DOWN" :
                                                          "force link UP"
                        ));

                linkCase = (physicalPortLinkStatusMask == 0) ? FORCE_LINK_DOWN_E:
                                                               FORCE_LINK_UP_E;
            }
        }
        else
        {
            physicalPort = port;
            linkCase = LINK_ACCORDING_TO_MAC_E;/* for generic code */
        }

        if(linkCase == LINK_ACCORDING_TO_MAC_E)
        {
            if(GT_FALSE == snetChtTxMacPortGet(loopPortDevObjPtr,
                descrPtr,
                MAC_PORT_REASON_TYPE_EGF_TO_MAC_FOR_LINK_STATUS_E,
                physicalPort,
                GT_TRUE,
                &txDmaDevObjPtr,
                &macPort))
            {
                /* MAC port not exists */

                destPorts[port] = 0;

                __LOG(("port[%d] - FILTERED. Reason: associated MAC not exists \n", port));

                continue;
            }

            simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_PORT_MAC_EGRESS_E);
            if(GT_FALSE == snetChtIsTxPortLinkUp(loopPortDevObjPtr,GT_TRUE,macPort))
            {
                /* MAC considered 'link DOWN' */
                linkUP = 0;
            }
            else
            {
                linkUP = 1;
            }
            simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT___RESTORE_PREVIOUS_UNIT___E);
        }
        else if (linkCase == FORCE_LINK_UP_E)/*ForceLinkUp*/
        {
            __LOG(("port [%d] treated as 'Force Link Up' \n",
                port));
            linkUP = 1;
        }
        else  if (linkCase == FORCE_LINK_DOWN_E)/*ForceLinkDown*/
        {
            __LOG(("port [%d] treated as 'Force Link Down' \n",
                port));
            linkUP = 0;
        }
        else /* linkCase == LINK_UNKNOWN_E*/
        {
            __LOG(("WARNING : LINK_UNKNOWN_E : port [%d] treated as 'Link down' \n",
                port));
            linkUP = 0;/* bad value of state - treat as link down */
        }

        if(targetPortGroupId == devObjPtr->portGroupId)
        {
            /* enable the port only if it is enable and the link is up */
            if(linkUP)
            {
                __LOG(("local port %d replaced by backup port %d",
                    port,loopPort));
            }

            destPorts[loopPort] = linkUP;
        }
        else
        {
            /* do replication to the target port group to egress the packet */
            __LOG(("local port %d replaced by global backup port %d -- NOT SUPPORTED BY SIMULATION - not in the same port group \n",
                    port,
                    globalLoopPort));
        }
    }

}

/**
* @internal snetChtTxQUnknUnregFilters function
* @endinternal
*
* @brief   Filter Unknown unicast and Unregistered Multicast Broadcast
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in,out] destPorts[]              - number of egress port.
* @param[in] packetType               - type of packet
*/
static void snetChtTxQUnknUnregFilters
(
  IN SKERNEL_DEVICE_OBJECT * devObjPtr,
  IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
  INOUT GT_U32 destPorts[],
  IN SKERNEL_EGR_PACKET_TYPE_ENT packetType
)
{
    DECLARE_FUNC_NAME(snetChtTxQUnknUnregFilters);

    GT_U32    regAddr;                /* register address */
    SKERNEL_PORTS_BMP_STC  excludePortBmp;  /* exclude port bmp of trunk */
    GT_U32  * regPtr;                 /* pointer to register data */
    GT_U32    port;                   /* port index */
    GT_U32    cpuFilterEn;            /* enables filtering to CPU port  */
    GT_U32    localPort;              /* local Port number */
    GT_U32    startPort;              /* Start iteration port */
    GT_U32    endPort;                /* End iteration port */

    if (packetType != SKERNEL_EGR_PACKET_BRG_UNK_UCAST_E &&
        packetType != SKERNEL_EGR_PACKET_BRG_UNREG_MCAST_E)
    {
        /* no filtering */
        __LOG(("no 'Unkn Unreg Filters' for this packet type \n"));
        return;
    }

    startPort = SNET_CHT_EGR_TXQ_START_PORT_MAC(devObjPtr);
    endPort = SNET_CHT_EGR_TXQ_END_PORT_MAC(devObjPtr);

    SKERNEL_PORTS_BMP_CLEAR_MAC(&excludePortBmp);

    if (packetType == SKERNEL_EGR_PACKET_BRG_UNK_UCAST_E)
    {
        /* filter unknown unicast packets */
        regAddr = SMEM_CHT_EGRESS_UNKN_UC_FILTER_EN_REG(devObjPtr);
        regPtr = smemMemGet(devObjPtr, regAddr);

        SKERNEL_FILL_PORTS_BITMAP_MAC(devObjPtr, &excludePortBmp, regPtr);

        if ((SKERNEL_IS_CHEETAH1_ONLY_DEV(devObjPtr)))
        {
            cpuFilterEn = SMEM_U32_GET_FIELD(regPtr[0], 27, 1);
            SMEM_U32_SET_FIELD(excludePortBmp.ports[1], 31, 1, cpuFilterEn);
        }
    }
    else if (packetType == SKERNEL_EGR_PACKET_BRG_UNREG_MCAST_E)
    {
        if(descrPtr->macDaType == SKERNEL_MULTICAST_MAC_E)
        { /* filter unknown multicast packets */
            __LOG(("filter unknown multicast packets"));
            regAddr = SMEM_CHT_EGRESS_UNKN_MC_FILTER_EN_REG(devObjPtr);
            regPtr = smemMemGet(devObjPtr, regAddr);

            SKERNEL_FILL_PORTS_BITMAP_MAC(devObjPtr, &excludePortBmp, regPtr);

            if ((SKERNEL_IS_CHEETAH1_ONLY_DEV(devObjPtr)))
            {
                cpuFilterEn = SMEM_U32_GET_FIELD(regPtr[0], 27, 1);
                SMEM_U32_SET_FIELD(excludePortBmp.ports[1], 31, 1, cpuFilterEn);
            }
        }
        /* macDaType can be SKERNEL_BROADCAST_MAC_E or SKERNEL_BROADCAST_ARP_E */
        else
        { /* filter unregistered broadcast packets */
            __LOG(("filter unregistered broadcast packets"));
            if ((!SKERNEL_IS_CHEETAH1_ONLY_DEV(devObjPtr)))
            { /* cheetah2/3 */
                regAddr = SMEM_CHT_EGRESS_UNKN_BC_FILTER_EN_REG(devObjPtr);
                regPtr = smemMemGet(devObjPtr, regAddr);
                SKERNEL_FILL_PORTS_BITMAP_MAC(devObjPtr, &excludePortBmp, regPtr);
            }
        }
    }

    for (port = startPort,localPort = 0; port < endPort; port++ , localPort++)
    {
        destPorts[localPort] &=
            ~SKERNEL_PORTS_BMP_IS_PORT_SET_MAC(&excludePortBmp, port);
    }

    if(port < SNET_CHT_CPU_PORT_CNS)
    {
        localPort = SNET_CHT_CPU_PORT_CNS;
        port = SNET_CHT_CPU_PORT_CNS;

        destPorts[localPort] &=
                ~SKERNEL_PORTS_BMP_IS_PORT_SET_MAC(&excludePortBmp, port);
    }

}


/**
* @internal snetChtTxQStpFilters function
* @endinternal
*
* @brief   Check STP port state fro every port in device .
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in,out] destPorts[]              - number of egress port.
* @param[in] stpVector[]              - send frame with tag.
*/
static void snetChtTxQStpFilters
(
  IN SKERNEL_DEVICE_OBJECT * devObjPtr,
  IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
  INOUT GT_U32 destPorts[],
  IN SKERNEL_STP_ENT stpVector[],
  IN GT_U32 oamLoopbackFilterEn[]
)
{
    DECLARE_FUNC_NAME(snetChtTxQStpFilters);

    GT_U32  port;       /* port index */
    GT_U32  tunnelEgressFilter;
    GT_U32  regAddr;
    GT_U32  localPort;       /* local Port number */
    GT_U32  startPort;       /* Start iteration port */
    GT_U32  endPort;         /* End iteration port */
    GT_U32  localDevSrcPort;
    GT_U32  fldValue;

    localDevSrcPort = descrPtr->localPortGroupPortAsGlobalDevicePort;/* local port(not global) */

    if (oamLoopbackFilterEn[localDevSrcPort] &&
        descrPtr->egressOnIngressPortGroup)
    {
        return;
    }

    if (descrPtr->tunnelStart)
    { /* Enable/Disable Egress filtering for Tunnel Start packets */
        /* Egress Filtering Register0 */
        if(devObjPtr->txqRevision != 0)
        {
            regAddr = SMEM_LION_TXQ_EGR_FILTER_GLOBAL_EN_REG(devObjPtr);
            if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
            {
                smemRegGet(devObjPtr, regAddr, &fldValue);

                /* check the 'filter Enable' and the 'global filter enable' */
                if(SMEM_U32_GET_FIELD(fldValue,0,1) &&
                   SMEM_U32_GET_FIELD(fldValue,7,1) )
                {
                    tunnelEgressFilter = 1;
                }
                else
                {
                    tunnelEgressFilter = 0;
                }
            }
            else
            {
                smemRegFldGet(devObjPtr, regAddr, 8, 1, &tunnelEgressFilter);
            }
        }
        else
        {
            regAddr = SMEM_CHT2_TUNNEL_EGRESS_CONFIG_REG(devObjPtr);
            smemRegFldGet(devObjPtr, regAddr, 25, 1, &tunnelEgressFilter);
        }
        if (tunnelEgressFilter == 0)
        {
            __LOG(("no filtering on Tunnel start"));
            return;/* no filtering on Tunnel start */
        }
    }

    /* Every port not in STP forward state needs to be blocked */
    startPort = SNET_CHT_EGR_TXQ_START_PORT_MAC(devObjPtr);
    endPort = SNET_CHT_EGR_TXQ_END_PORT_MAC(devObjPtr);
    for (port = startPort,localPort = 0; port < endPort; port++ , localPort++)
    {
        if (destPorts[localPort])
        {
            if ( (stpVector[localPort] != SKERNEL_STP_FORWARD_E) &&
                 (stpVector[localPort] != SKERNEL_STP_DISABLED_E) )
            {
                __LOG(("localPort [%d] not in STP forward state needs to be blocked \n",
                    localPort));
                destPorts[localPort] = 0 ;
            }
        }
    }
}

/**
* @internal snetChtTxQTrunkFilters function
* @endinternal
*
* @brief   Trunk filters for multicast packets
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in,out] destPorts[]              - number of egress port.
* @param[in] packetType               - Type of packet.
*/
static void snetChtTxQTrunkFilters
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    INOUT GT_U32 destPorts[],
    IN SKERNEL_EGR_PACKET_TYPE_ENT packetType
)
{
    DECLARE_FUNC_NAME(snetChtTxQTrunkFilters);

    SKERNEL_PORTS_BMP_STC designatedTrunkPort;/* Designated trunk port */
    GT_U32 regAddr;               /* Register address */
    GT_U32 port;                  /* Port index */
    GT_U32 * regPtr;              /* Register pointer */
    TRUNK_HASH_MODE_ENT hashMode; /* Trunk hash mode */
    GT_U32 doubleTrunkSrcFilterEn;/* Double filtering */
    GT_U32 needDoubleTrunkFilter; /* Do we need double filter on cascade trunk */
    GT_U32 localPort;             /* local Port number */
    GT_U32 startPort;             /* Start iteration port */
    GT_U32 endPort;               /* End iteration port */
    SKERNEL_PORTS_BMP_STC egressPortsBmp;
    GT_U32 fldValue;
    GT_BOOL skipOrigTrunkFilter = GT_FALSE;
    GT_BOOL skipLocalTrunkFilter = GT_FALSE;
    GT_U32  origIsTrunk;
    GT_U32  origTrunkId;
    GT_U32  trunkId_non_trunk1;/* access to non-trunk1 */
    GT_U32  trunkId_non_trunk2;/* access to non-trunk2 */

    startPort = SNET_CHT_EGR_TXQ_START_PORT_MAC(devObjPtr);
    endPort = SNET_CHT_EGR_TXQ_END_PORT_MAC(devObjPtr);

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        if(0 == descrPtr->eArchExtInfo.phySrcMcFilterEn)
        {
            if(descrPtr->eArchExtInfo.origSrcPhyIsTrunk)
            {
                __LOG(("WARNING : Not trying to FILTER on orig src physical trunkId[%d] because descrPtr->eArchExtInfo.phySrcMcFilterEn [0] \n",
                    descrPtr->eArchExtInfo.origSrcPhyPortTrunk));
            }

            if(descrPtr->localDevSrcTrunkId)
            {
                __LOG(("WARNING : Not trying to FILTER on local dev trunkId[%d] because descrPtr->eArchExtInfo.phySrcMcFilterEn [0] \n",
                    descrPtr->localDevSrcTrunkId));
            }

            goto designated_filter__lbl;
        }

        if(descrPtr->marvellTagged == 1 &&
           descrPtr->eArchExtInfo.origSrcPhyIsTrunk &&
           descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_FORWARD_E)
        {
            origTrunkId = descrPtr->eArchExtInfo.origSrcPhyPortTrunk;
            origIsTrunk = 1;
            __LOG(("Used orig src trunkId from descrPtr->eArchExtInfo.origSrcPhyPortTrunk [%d] \n",
                origTrunkId));
        }
        else
        {
            origIsTrunk = 0;
            origTrunkId = descrPtr->origSrcEPortOrTrnk;
            __LOG(("No orig src trunkId indication \n"));
        }
    }
    else
    {
        origIsTrunk = descrPtr->origIsTrunk;
        origTrunkId = descrPtr->origSrcEPortOrTrnk;
    }


    if(descrPtr->localDevSrcTrunkId && origIsTrunk &&
        descrPtr->localDevSrcTrunkId != origTrunkId)
    {
        needDoubleTrunkFilter = 1;
    }
    else
    {
        needDoubleTrunkFilter = 0;
    }

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr) && origIsTrunk)
    {
        /* this will force the code to access 'non trunk members 2' table ,
           to filter the user trunk */
        needDoubleTrunkFilter = 1;
    }

    /* Egress Filtering Register0 */
    if(devObjPtr->txqRevision != 0)
    {
        regAddr = SMEM_LION_TXQ_EGR_FILTER_GLOBAL_EN_REG(devObjPtr);
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            smemRegGet(devObjPtr, regAddr, &fldValue);

            /* check the 'filter Enable' and the 'global filter enable' */
            if(SMEM_U32_GET_FIELD(fldValue,0,1) &&
               SMEM_U32_GET_FIELD(fldValue,4,1) )
            {
                doubleTrunkSrcFilterEn = 1;
            }
            else
            {
                doubleTrunkSrcFilterEn = 0;
            }
        }
        else
        {
            smemRegFldGet(devObjPtr, regAddr, 4, 1, &doubleTrunkSrcFilterEn);
        }
    }
    else
    {
        regAddr = SMEM_CHT_EGRESS_UNKN_UC_FILTER_EN_REG(devObjPtr);
        smemRegFldGet(devObjPtr, regAddr, 30, 1, &doubleTrunkSrcFilterEn);
    }


    if(origIsTrunk || descrPtr->localDevSrcTrunkId)
    {
        trunkId_non_trunk1 = descrPtr->localDevSrcTrunkId;
        trunkId_non_trunk2 = descrPtr->marvellTagged ?
                                 origTrunkId :
                                 descrPtr->localDevSrcTrunkId;

        if(SMEM_CHT_IS_SIP5_GET(devObjPtr) &&
           descrPtr->localDevSrcTrunkId >= 256)
        {
            __LOG(("src 'local' trunk [%d] NOT filtered due to '>=256' \n",
                descrPtr->localDevSrcTrunkId));

            trunkId_non_trunk1 = 0;/* the table will not be accesses (due to out of range) */
        }

        if(descrPtr->pktIsLooped)
        {
            /* looped back packet not subject to this filter  */
            __LOG(("src trunk [%d] NOT filtered due to 'pktIsLooped = 1' \n",
                origTrunkId));
            skipOrigTrunkFilter = GT_TRUE;
        }
        else
        if(descrPtr->doRouterHa == GT_TRUE)
        {
            __LOG(("src trunk [%d] NOT filtered due to 'doRouterHa = 1' \n",
                origTrunkId));
            skipOrigTrunkFilter = GT_TRUE;
        }
        else
        if(descrPtr->routed == 1)
        {
            __LOG(("src trunk [%d] NOT filtered due to 'routed = 1' \n",
                origTrunkId));
            skipOrigTrunkFilter = GT_TRUE;
        }
        else
        if((descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E) &&
           (descrPtr->srcTrgDev == descrPtr->ownDev))
        {
            __LOG(("src trunk [%d] NOT filtered due to  descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E on 'own' device \n",
                origTrunkId));
            skipOrigTrunkFilter = GT_TRUE;
        }

        if(SMEM_CHT_IS_SIP5_GET(devObjPtr) &&
           (descrPtr->marvellTagged == 0))
        {
            if(devObjPtr->errata.egfOrigSrcTrunkFilter255)
            {
                /* packets with non-DSA tag access only to the 'non-trunk1' table */
                __LOG(("(non-dsa) src 'local' trunk [%d] NON_TRUNK_MEMBERS_2 NOT accessed (try only NON_TRUNK_MEMBERS_2) \n",
                    descrPtr->localDevSrcTrunkId));
                trunkId_non_trunk2 = 0;/* not accessed */

                if(trunkId_non_trunk1 == 0 &&
                   trunkId_non_trunk2 == 0)
                {
                    __LOG(("(non-DSA) src trunk [%d] (>255) NOT filtered due to Erratum 3067200 (JIRA: EGF-1008) \n",
                        descrPtr->localDevSrcTrunkId));
                }
            }
            else
            {
                /* packets with non-DSA tag access only to the 'non-trunk2' table */
                /* part of fix of : JIRA: EGF-1008 :
                    For non-DSA packets received on network port we can use only 255 trunks */
                __LOG(("(non-dsa) src 'local' trunk [%d] NON_TRUNK_MEMBERS NOT accessed (try only NON_TRUNK_MEMBERS_2) \n",
                    descrPtr->localDevSrcTrunkId));
                trunkId_non_trunk1 = 0;/* not accessed */
            }
        }

        if(skipOrigTrunkFilter == GT_FALSE &&
           trunkId_non_trunk2 > 0 &&
           (needDoubleTrunkFilter == 0 || doubleTrunkSrcFilterEn))
        {
            __LOG(("%s : do src filtering of 'Orig trunk' of trunkId[%d] \n",
                SMEM_CHT_IS_SIP5_GET(devObjPtr) ?  "NON_TRUNK_MEMBERS_2" : "NON_TRUNK_MEMBERS",
                trunkId_non_trunk2));

            /* get the BMP of 'NON_TRUNK_MEMBERS_2' (NOTE: in non-sip5 this is NON_TRUNK_MEMBERS) */
            snetChtEgfShtEntryBmpGet(devObjPtr,descrPtr,
                SNET_CHT_EGF_SHT_TABLE_NON_TRUNK_MEMBERS_2_TYPE_E,
                trunkId_non_trunk2,
                &egressPortsBmp,&endPort);

            for (port = startPort,localPort = 0; port < endPort; port++ , localPort++)
            {
                destPorts[localPort] &=
                    SKERNEL_PORTS_BMP_IS_PORT_SET_MAC(&egressPortsBmp, port);
            }
        }

        if(descrPtr->localDevSrcTrunkId)
        {
            if(descrPtr->pktIsLooped)
            {
                /* looped back packet not subject to this filter  */
                __LOG(("src 'local' [%d] NOT filtered due to 'pktIsLooped = 1' \n",
                    origTrunkId));
                skipLocalTrunkFilter = GT_TRUE;
            }
            else
            if(descrPtr->doRouterHa == GT_TRUE)
            {
                __LOG(("src 'local' [%d] NOT filtered due to 'doRouterHa = 1' \n",
                    origTrunkId));
                skipLocalTrunkFilter = GT_TRUE;
            }
            /* ALLOW to filter : 0 the packet was routed by a previous device.
               This is because we do NOT want to send the packet back out the same
               cascade port that it was received */
                /*
                else
                if(descrPtr->routed == 1)
                {
                    __LOG(("src 'local' [%d] NOT filtered due to 'routed = 1' \n",
                        origTrunkId));
                    skipLocalTrunkFilter = GT_TRUE;
                }*/
            else
            if((descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E) &&
                (descrPtr->srcTrgDev == descrPtr->ownDev))
            {
                __LOG(("src 'local' [%d] NOT filtered due to  descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E on 'own' device \n",
                    origTrunkId));
                skipLocalTrunkFilter = GT_TRUE;
            }

            if(skipLocalTrunkFilter == GT_FALSE &&
               trunkId_non_trunk1 > 0)
            {
                /* filter the members of the local/cascade trunk , from being flooded back
                   to the cascade trunk */
                regAddr =
                    SMEM_CHT_NON_TRUNK_MEMBERS_TBL_MEM(devObjPtr, trunkId_non_trunk1);
                regPtr = smemMemGet(devObjPtr, regAddr);

                __LOG(("%s : do src filtering of 'local trunk' of trunkId[%d] \n",
                    "NON_TRUNK_MEMBERS",
                    descrPtr->localDevSrcTrunkId));

                SKERNEL_FILL_PORTS_BITMAP_MAC(devObjPtr, &egressPortsBmp, regPtr);
                for (port = startPort,localPort = 0; port < endPort; port++ , localPort++)
                {
                    destPorts[localPort] &=
                        SKERNEL_PORTS_BMP_IS_PORT_SET_MAC(&egressPortsBmp, port);
                }
            }
        }
    }

designated_filter__lbl:
    hashMode = 0;/* not relevant to useVidx = 1 */

    __LOG(("Get designated ports for the 'flood' \n"));
    /* get designated ports of trunk , exclude all '0' members of      *
     *  designatedTrunkPorts from destPorts                            */
    snetChtTxGetTrunkDesignatedPorts(devObjPtr, descrPtr, hashMode, &designatedTrunkPort);

    for (port = startPort,localPort = 0; port < endPort; port++ , localPort++)
    {
        destPorts[localPort] &=
            SKERNEL_PORTS_BMP_IS_PORT_SET_MAC(&designatedTrunkPort, port);
    }
}


/**
* @internal snetChtTxQOamFilter function
* @endinternal
*
* @brief   OAM filter algorithm
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] packetType               - type of packet.
* @param[in,out] destPorts[]              - number of egress port.
*
* @note when the egress port in OAM loopback mode, only OAM frames that loop back
*       should egress from it.
*
*/
static void snetChtTxQOamFilter
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SKERNEL_EGR_PACKET_TYPE_ENT packetType,
    INOUT GT_U32 destPorts[],
    IN GT_U32 oamLoopbackFilterEn[]
)
{
    DECLARE_FUNC_NAME(snetChtTxQOamFilter);

    GT_U32  port;
    GT_U32  localDevSrcPort;
    GT_U32 localPort;              /* local Port number */
    GT_U32 startPort;             /* Start iteration port */
    GT_U32 endPort;               /* End iteration port */

    startPort = SNET_CHT_EGR_TXQ_START_PORT_MAC(devObjPtr);
    endPort = SNET_CHT_EGR_TXQ_END_PORT_MAC(devObjPtr);

    localDevSrcPort = descrPtr->localPortGroupPortAsGlobalDevicePort;/* local port(not global) */

    if(descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_FROM_CPU_E &&
       descrPtr->egressFilterEn == 0)
    {
        /* no egress OAM filter on "from cpu" */
        __LOG(("no egress OAM filter on 'from cpu'"));
        return;
    }

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* adjust to loop of "port <= endPort" */
        endPort -= 1;
    }

    for (port = startPort,localPort = 0; port <= endPort; port++ , localPort++)
    {
        if((port == endPort) && (endPort < SNET_CHT_CPU_PORT_CNS))
        {
            /* jump to cpu port */
            __LOG(("jump to cpu port"));
            localPort = SNET_CHT_CPU_PORT_CNS;
        }

        if(destPorts[localPort] == 0)
        {
            /* port was not going to get it anyway */
            continue;
        }

        if(oamLoopbackFilterEn[localDevSrcPort] &&
           (localDevSrcPort == localPort) && descrPtr->egressOnIngressPortGroup)
        {
            /* we let the src port a chance to get the loop back */
            __LOG(("we let the src port a chance to get the loop back"));
            continue;
        }

        /* when the egress port in OAM loopback mode, only OAM frames that
           loop back should egress from it */
        if(oamLoopbackFilterEn[localPort] == 1)
        {
            /* we allow only to OAM loopback , from this src port */
            __LOG(("we allow only to OAM loopback , from this src port"));

            destPorts[localPort] = 0;
        }
    }


    return;
}

/**
* @internal snetLion3EgfShtMeshIdFilterGet function
* @endinternal
*
* @brief   SIP5 :
*         Get if 'Egress MeshId filter' enabled on the port
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - egress port.
* @param[in] ingressMeshId            - ingress MeshId calculated from sstId
*
* @retval GT_TRUE                  - the EGF will   filter the port
* @retval GT_FALSE                 - the EGF will NOT filter this port
*/
static GT_U32 snetLion3EgfShtMeshIdFilterGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32                   egressPort,
    GT_U32                      ingressMeshId
)
{
    GT_U32 regAddress;      /* Register's address */
    GT_U32 egressMeshId;    /* Register's field value */
    GT_U32 *memPtr;

    /*IngressMESH-ID= SrcID[S - 1 + L : L]
    Where:
    SrcID= InDesc<SST_ID>
    L= Global<MESH ID offset>
    S= Global<MESH ID size>
    EgressMESH-ID= ePort<MESH ID>
    Packets will be filtered if ingressMESH-ID<> 0 and ingressMESH-ID == egressMESH-ID.
    */
    if(descrPtr->useVidx == 0)
    {
        snetChtEgfEgressEPortEntryGet(devObjPtr,descrPtr,egressPort,GT_FALSE/*global port*/);

        if(descrPtr->eArchExtInfo.egfShtEgressEPortTablePtr)
        {
            /* use the ePort table */
            egressMeshId = snetFieldValueGet(descrPtr->eArchExtInfo.egfShtEgressEPortTablePtr,11,4);

            if ((ingressMeshId != 0x0) && (ingressMeshId == egressMeshId))
                return GT_TRUE;

            return GT_FALSE;
        }

    }

    regAddress = SMEM_LION3_EGF_SHT_EPORT_MESH_ID_REG(devObjPtr);

    memPtr = smemMemGet(devObjPtr, regAddress);
    egressMeshId = snetFieldValueGet(memPtr, egressPort*4 ,4);

    if ((ingressMeshId != 0x0) && (ingressMeshId == egressMeshId))
        return GT_TRUE;

    return GT_FALSE;
}

/**
* @internal snetChtTxQEgressMeshIdFilters function
* @endinternal
*
* @brief   Egress Mesh Id filter algorithm
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] packetType               - type of packet.
* @param[in,out] destPorts[]              - number of egress port.
*/
static void snetChtTxQEgressMeshIdFilters
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SKERNEL_EGR_PACKET_TYPE_ENT packetType,
    INOUT GT_U32 destPorts[]
)
{
    DECLARE_FUNC_NAME(snetChtTxQEgressMeshIdFilters);

    GT_U32  regAddress;      /* Register's address */
    GT_U32  port;
    GT_U32  localPort;              /* local Port number */
    GT_U32  startPort;             /* Start iteration port */
    GT_U32  endPort;               /* End iteration port */
    GT_U32  filterEgressMeshId=GT_FALSE;
    GT_U32  meshIdConfig;    /* Register's field value */
    GT_U32  offset;
    GT_U32  numOfBits;
    GT_U32  ingressMeshId;

    regAddress = SMEM_LION3_EGF_SHT_MESH_ID_CONFIGS_REG(devObjPtr);
    smemRegGet(devObjPtr, regAddress,&meshIdConfig);

    offset = SMEM_U32_GET_FIELD(meshIdConfig,0,4);
    numOfBits = SMEM_U32_GET_FIELD(meshIdConfig,4,3);
    ingressMeshId = SMEM_U32_GET_FIELD(descrPtr->sstId,offset,numOfBits);

    __LOG(("The ingress mesh-ID [0x%8.8x] is build from bit[%d] num of bits[%d] of sstId[0x%8.8x] \n" ,

            ingressMeshId , offset , numOfBits , descrPtr->sstId ));

    if(ingressMeshId == 0)
        return;

    startPort = SNET_CHT_EGR_TXQ_START_PORT_MAC(devObjPtr);
    endPort = SNET_CHT_EGR_TXQ_END_PORT_MAC(devObjPtr);

    for (port = startPort,localPort = 0; port < endPort; port++ , localPort++)
    {
        if(destPorts[localPort] == 0)
        {
            /* port was not going to get it anyway */
            continue;
        }

        /* check Egress Mesh Id filter */
        filterEgressMeshId = snetLion3EgfShtMeshIdFilterGet(devObjPtr,descrPtr,localPort,ingressMeshId);

        if( filterEgressMeshId == 1)
        {
            /* Packets are filtered basd on ePot Mesh-Id value and the
               packets Mesh-id.
               The packet is filtered if the packets Mesh-Id is not
               0 and it is equal to the target ePort Mesh-Id value */
             __LOG(("Egress Physical Port [%d] is filtered due to Mesh-Id[0x%8.8x] check \n" ,
                    localPort , ingressMeshId));

            destPorts[localPort] = 0;
        }
    }

    return;
}

/**
* @internal snetLion3EgfShtVid1MismatchFilterGet function
* @endinternal
*
* @brief   SIP5.10 :
*         Get if 'Egress VID1 mismatch filter' enabled on the port
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - egress port.
*
* @retval GT_TRUE                  - the EGF will   filter the port
* @retval GT_FALSE                 - the EGF will NOT filter this port
*/
static GT_U32 snetLion3EgfShtVid1MismatchFilterGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32                   egressPort
)
{
    GT_U32 regAddress;  /* Register's address */
    GT_U32 portVid1;    /* Register's field value */
    GT_U32 dropOnVid1mismatchEnable; /* enable Drop packet on VID1 mismatch */
    GT_U32 globalOffset; /* VID1 field global offset */
    GT_U32 *memPtr;
                                                            /*called in loop from snetChtTxQEgressVid1MismatchFilter */
    snetChtEgfEgressEPortEntryGet(devObjPtr,descrPtr,egressPort,GT_FALSE/*global port*/);

    if (descrPtr->eArchExtInfo.egfShtEgressClass == SHT_PACKET_CLASS_1_E)
    {
        /* unicast destination Forward or FROM_CPU on own device.
           use the ePort table */
        dropOnVid1mismatchEnable = snetFieldValueGet(descrPtr->eArchExtInfo.egfShtEgressEPortTablePtr,15,1);
        if (dropOnVid1mismatchEnable == 0)
        {
            return GT_FALSE;
        }

        portVid1 = snetFieldValueGet(descrPtr->eArchExtInfo.egfShtEgressEPortTablePtr,16,12);
        if (portVid1 != descrPtr->vid1)
            return GT_TRUE;
    }
    else if (descrPtr->eArchExtInfo.egfShtEgressClass == SHT_PACKET_CLASS_2_E)
    {
        /* unicast destination Forward or FROM_CPU on remote device OR
           multidestination Forward or FROM_CPU.
           use per physical port registers. */
        regAddress = SMEM_LION3_EGF_SHT_EPORT_DROP_ON_EPORT_VID1_MISMATCH_REG(devObjPtr);

        memPtr = smemMemGet(devObjPtr, regAddress);
        dropOnVid1mismatchEnable = snetFieldValueGet(memPtr, egressPort, 1);
        if (dropOnVid1mismatchEnable == 0)
        {
            return GT_FALSE;
        }

        regAddress = SMEM_LION3_EGF_SHT_EPORT_ASSOCIATED_VID1_REG(devObjPtr);

        memPtr = smemMemGet(devObjPtr, regAddress);

        /* each register hold VID1 of two ports in bits 0..11 and 12..23 */
        globalOffset = (egressPort / 2) * 32 + ((egressPort & 1) * 12);
        portVid1 = snetFieldValueGet(memPtr, globalOffset, 12);

        if (portVid1 != descrPtr->vid1)
            return GT_TRUE;
    }

    return GT_FALSE;
}

/**
* @internal snetChtTxQEgressVid1MismatchFilter function
* @endinternal
*
* @brief   Egress ePort VID1 mismatch filter algorithm
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in,out] destPorts[]              - number of egress port.
*/
static void snetChtTxQEgressVid1MismatchFilter
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    INOUT GT_U32 destPorts[]
)
{
    DECLARE_FUNC_NAME(snetChtTxQEgressVid1MismatchFilter);

    GT_U32  port;                  /* port iterator        */
    GT_U32  localPort;             /* local Port number    */
    GT_U32  startPort;             /* Start iteration port */
    GT_U32  endPort;               /* End iteration port   */
    GT_U32  filterVid1Mismatch=GT_FALSE;

    startPort = SNET_CHT_EGR_TXQ_START_PORT_MAC(devObjPtr);
    endPort = SNET_CHT_EGR_TXQ_END_PORT_MAC(devObjPtr);

    for (port = startPort,localPort = 0; port < endPort; port++ , localPort++)
    {
        if(destPorts[localPort] == 0)
        {
            /* port was not going to get it anyway */
            continue;
        }

        /* check VID1 mismatch filter */
        filterVid1Mismatch = snetLion3EgfShtVid1MismatchFilterGet(devObjPtr,descrPtr,localPort);
        if( filterVid1Mismatch == GT_TRUE)
        {
            /* Packets are filtered based on ePot assigned VID1 value and the
               packets VID1.
               The packet is filtered if the feature enabled on ePort and
               packet's VID1 differs from the target ePort VID1 value */
             __LOG(("Egress Physical Port [%d] is filtered due to VID1[%d] mismatch check\n" ,
                    localPort , descrPtr->vid1));

            destPorts[localPort] = 0;
        }
    }

    return;
}

/**
* @internal snetChtTxQSSTFilters function
* @endinternal
*
* @brief   SST filter algorithm and "odd Source-ID values" filter algorithm.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in,out] destPorts[]              - number of egress port.
*/
static void snetChtTxQSSTFilters
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    INOUT GT_U32 destPorts[]
)
{
    DECLARE_FUNC_NAME(snetChtTxQSSTFilters);

    GT_U32 regAddr;                 /* register address */
    GT_U32 port;                    /* port index */
    GT_U32 cpuSrcIdEn;              /* CPU port is member of source ID group */
    GT_BIT ucSrcIdFilterEn;         /* Unicast SrcId Egress Filtering enable for the egress port */
    GT_BIT oddSrcIdFilterEn;         /* Unicast SrcId Egress Filtering enable for the egress port */
    SKERNEL_PORTS_BMP_STC ucSrcIdPortsEnableBmp;    /* Unicast SrcId Egress Filtering ports enable bitmap */
    SKERNEL_PORTS_BMP_STC sstMembersBmp;      /* SrcId Egress Filtering table */
    GT_U32 * regPtr;                /* Register data pointer */
    GT_U32 localPort;              /* local Port number */
    GT_U32 startPort;               /* Start iteration port */
    GT_U32 endPort;                 /* End iteration port */
    GT_U32 sstId;
    GT_U32 egressEPort ;  /* outgoing port */
    GT_BIT treatMllReplicatedAsMcEn;         /* treat MLL replicated packet as MC for UC target packets with UseVIDX = 0 */

    startPort = SNET_CHT_EGR_TXQ_START_PORT_MAC(devObjPtr);
    endPort = SNET_CHT_EGR_TXQ_END_PORT_MAC(devObjPtr);

    /* Enable srcID filtering for multi destination traffic */
    ucSrcIdFilterEn = cpuSrcIdEn = 1;
    SKERNEL_PORTS_BMP_MEMSET_MAC(&ucSrcIdPortsEnableBmp,0xffffffff)

    if(descrPtr->validSrcIdBmp)
    {
        /*srcId filter bitmap --used for "HOT spot applications and Private WLANs" */
        /* CAPWAP related */

        if(((1<<(descrPtr->sstId)) & descrPtr->srcIdBmp) == 0)
        {
            /* we need to filter frames that has srcId that is not in the "srcId bmp" */
            __LOG(("we need to filter frames that has srcId that is not in the 'srcId bmp'"));
            for (port = startPort,localPort = 0; port < endPort; port++ , localPort++)
            {
                destPorts[localPort] = 0;
            }
            /* DON'T change the "CPU port" , because for some reason it may
               "trapped"/"mirror" to CPU */
        }

        /*****************************************************/
        /* we are done with "srcId filtering" for this frame */
        /*****************************************************/

        __LOG(("we are done with 'srcId filtering' for this frame"));
        return;
    }

    treatMllReplicatedAsMcEn = 0;
    if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr) && descrPtr->useVidx == 0)
    {
        /* Treat MLL replicated packets as MC Global Enable */
        /* Source-ID Egress filtering can be enabled/disabled for UC (single
         * target) packets. For certain cases we need packets replicated by MLL
         * to be treated as multicast even when the target is unicast (UseVIDX = 0)
         * This configuration bit is used to force the Source-ID egress
         * filtering to treat packets replicated by MLL with single target
         * destination to be treated as multicast.
         */
        smemRegFldGet(devObjPtr,
                      SMEM_LION_TXQ_SHT_GLOBAL_CONF_REG(devObjPtr), 28, 1,
                      &treatMllReplicatedAsMcEn);
        if (treatMllReplicatedAsMcEn)
        {
            __LOG(("Treat MLL replicated packets as MC Global Enable"));
        }
    }

    /* For single destination traffic srcID filtering is treated according
       to TXQ queue configuration registers and SST egress configuration for
       Ch/Ch2/Ch3 respectively */
    if (descrPtr->useVidx == 0 && treatMllReplicatedAsMcEn == 0)
    {
        if(devObjPtr->txqRevision == 0)
        {
            /* UC Source-ID Filter Global Enable */
            __LOG(("UC Source-ID Filter Global Enable"));
            smemRegFldGet(devObjPtr,
                          SMEM_CHT_TX_QUEUE_EXTEND_CTRL_REG(devObjPtr), 29, 1,
                          &ucSrcIdFilterEn);
            /* UC Source-ID Filter Disable */
            if (ucSrcIdFilterEn == 0)
            {
                __LOG(("UC Source-ID Filter Disable"));
                return;
            }
        }

        /* Global configuration of Source ID filter enabled.
           Need check for CH2 and above devices per port enabler
           of the feature */
        if ((!SKERNEL_IS_CHEETAH1_ONLY_DEV(devObjPtr)))
        {
          /*Ports bitmap indicates if Source-ID filtering is performed on Unicast
            packets forwarded to this port*/
            regAddr = SMEM_CHT_SST_EGRESS_CONFIG_REG(devObjPtr);
            regPtr = smemMemGet(devObjPtr, regAddr);
            /* Source-ID filtering is not performed on Unicast packets
               forwarded to this port */
            SKERNEL_FILL_PORTS_BITMAP_MAC(devObjPtr, &ucSrcIdPortsEnableBmp, regPtr);

            /* Check UC Src ID Filter on CPU port */
            cpuSrcIdEn =
                SKERNEL_PORTS_BMP_IS_PORT_SET_MAC(&ucSrcIdPortsEnableBmp, SNET_CHT_CPU_PORT_CNS);
            if(cpuSrcIdEn)
            {
                __LOG(("UC Src ID Filter on CPU port : enabled \n"));
            }
        }

        /* check for "odd Source-ID" filter */
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /* the functions are relevant only for UC packets */
            egressEPort = snetChtTxQGetEgrPortForTrgDevice(devObjPtr, descrPtr, GT_TRUE);
            snetChtEgfEgressEPortEntryGet(devObjPtr,descrPtr,egressEPort,GT_FALSE/*global port*/);
            if(descrPtr->eArchExtInfo.egfShtEgressEPortTablePtr)
            {
                /* use the ePort table */
                oddSrcIdFilterEn = snetFieldValueGet(descrPtr->eArchExtInfo.egfShtEgressEPortTablePtr,9,1);

                if((oddSrcIdFilterEn) && (descrPtr->sstId&1))
                {
                    __LOG(("UC Packet filtered due odd Source-ID on ePort [%d]\n", egressEPort));
                    destPorts[egressEPort] = 0;
                }
            }
        }
    }

    sstId = descrPtr->sstId;

    /* SrcID<n> Egress Filtering Table Entry */
    __LOG(("SrcID<%d> Egress Filtering Table Entry \n",sstId));

    /* get the BMP of 'SRC_ID' */
    snetChtEgfShtEntryBmpGet(devObjPtr,descrPtr,
        SNET_CHT_EGF_SHT_TABLE_SRC_ID_TYPE_E,
        sstId,
        &sstMembersBmp,&endPort);

    /* Exclude all SST filters port, for multi destination traffic srcID filter
       is always enabled (ucSrcIdFilterEn == 0xffffffff) */
    for (port = startPort,localPort = 0; port < endPort; port++ , localPort++)
    {
        /* check for "odd Source-ID" filter */
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            regAddr = SMEM_LION3_EGF_SHT_EPORT_ODD_ONLY_SRC_ID_FILTER_ENABLE_REG(devObjPtr);
            regPtr = smemMemGet(devObjPtr, regAddr);
            oddSrcIdFilterEn = snetFieldValueGet(regPtr,port,1);

            if((oddSrcIdFilterEn) && (sstId&1))
            {
                __LOG(("Packet filtered due odd Source-ID on port [%d]\n", port));
                destPorts[localPort] = 0;
            }
        }

        ucSrcIdFilterEn =
            SKERNEL_PORTS_BMP_IS_PORT_SET_MAC(&ucSrcIdPortsEnableBmp, port);

        if (ucSrcIdFilterEn)
        {
            destPorts[localPort] &=
                SKERNEL_PORTS_BMP_IS_PORT_SET_MAC(&sstMembersBmp, port);
        }
    }

    destPorts[SNET_CHT_CPU_PORT_CNS] &= cpuSrcIdEn;
}

/**
* @internal snetLion3EgfShtForwardRestriction function
* @endinternal
*
* @brief   SIP5 :
*         Get the Forward restriction value.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*                                      restrictionType - the restriction type
* @param[in] egressPort               - egress port.
*
* @retval the restriction value : 0 - Not filter
* @retval 1                        - filter
*/
static GT_BIT snetLion3EgfShtForwardRestriction
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32                   egressPort
)
{
    DECLARE_FUNC_NAME(snetLion3EgfShtForwardRestriction);

    GT_U32 regAddress;  /* Register's address */
    GT_U32 * regPtr ;   /* registers  pointer */
    GT_BIT filter;
    GT_U32  bitIndex;

    if(descrPtr->useVidx == 0)
    {                                                            /*called from loop in snetChtTxQPortFrwrdRestriction*/
        snetChtEgfEgressEPortEntryGet(devObjPtr,descrPtr,egressPort,GT_FALSE/*global port*/);

        if(descrPtr->eArchExtInfo.egfShtEgressEPortTablePtr)
        {
            if (descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_FROM_CPU_E)
            {
                /*From CPU Forward Restricted*/
                bitIndex = 0;
            }
            else if (descrPtr->doRouterHa == 0)
            {
                /*Bridged Forward Restricted*/
                bitIndex = 1;
            }
            else
            {
                /*Routed Forward Restricted*/
                bitIndex = 2;
            }

            filter = snetFieldValueGet(descrPtr->eArchExtInfo.egfShtEgressEPortTablePtr,bitIndex,1);

            goto done_lbl;
        }
    }

    if (descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_FROM_CPU_E)
    {
        regAddress = SMEM_CHT_FROM_CPU_FILTER_CONF_REG(devObjPtr);
    }
    else if (descrPtr->doRouterHa == 0)
    {
        regAddress = SMEM_CHT_BRIDGE_FILTER_CONF_REG(devObjPtr);
    }
    else
    {
        regAddress = SMEM_CHT_ROUTE_FILTER_CONF_REG(devObjPtr);
    }

    regPtr = smemMemGet(devObjPtr, regAddress);
    filter = snetFieldValueGet(regPtr, egressPort, 1);

done_lbl:

    if(filter)
    {
        if (descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_FROM_CPU_E)
        {
            /*From CPU Forward Restricted*/
            __LOG(("Filtered : 'From CPU' due to Forward Restricted (egress port %d) \n",
                egressPort));
        }
        else if (descrPtr->doRouterHa == 0)
        {
            /*Bridged Forward Restricted*/
            __LOG(("Filtered : Bridged replication due to Forward Restricted(egress port %d) \n",
                egressPort));
        }
        else
        {
            /*Routed Forward Restricted*/
            __LOG(("Filtered : Routed replication due to Forward Restricted(egress port %d) \n",
                egressPort));
        }
    }

    return filter;
}


/**
* @internal snetChtTxQPortFrwrdRestriction function
* @endinternal
*
* @brief   forwarding restriction filter for cheetah2 simulation
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in,out] destPorts[]              - number of egress port.
*
* @param[out] forwardRestrictionBmpPtr - pointer to bitmap of forwarding restriction ports
*
* @note 8.2.5 Cht2 , Port Ingress/Egress Forwarding Restriction
*
*/
static void snetChtTxQPortFrwrdRestriction
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    INOUT GT_U32 destPorts[],
    IN GT_U32 oamLoopbackFilterEn[],
    OUT SKERNEL_PORTS_BMP_STC * forwardRestrictionBmpPtr
)
{
    DECLARE_FUNC_NAME(snetChtTxQPortFrwrdRestriction);

    GT_U32  regAddr;            /* register address */
    GT_U32  * regPtr;           /* register/memory data pointer */
    GT_U32  port;               /* port loop index */
    GT_U32  localDevSrcPort;
    GT_U32  localPort;          /* local Port number */
    GT_U32  startPort;          /* Start iteration port */
    GT_U32  endPort;            /* End iteration port */
    SKERNEL_PORTS_BMP_STC egressPortsBmp;   /* Egress ports bitmap */
    GT_BIT  filter;/* filter packet or not --> 1 - filter ... 0 - not filter */

    startPort = SNET_CHT_EGR_TXQ_START_PORT_MAC(devObjPtr);
    endPort = SNET_CHT_EGR_TXQ_END_PORT_MAC(devObjPtr);

    localDevSrcPort = descrPtr->localPortGroupPortAsGlobalDevicePort;/* local port(not global) */

    if ((!SKERNEL_IS_CHEETAH1_ONLY_DEV(devObjPtr)) &&
         (descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_FORWARD_E ||
          descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_FROM_CPU_E ||
          descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_MIRROR_TO_CPU_E ||
          descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_TRAP_TO_CPU_E))
    {
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            for (port = startPort,localPort = 0; port < endPort; port++ , localPort++)
            {
                if(destPorts[localPort] == 0)
                {
                    /* no need to check non target port */
                    continue;
                }

                filter = snetLion3EgfShtForwardRestriction(devObjPtr,descrPtr,port);
                if(filter)
                {
                    destPorts[localPort] = 0;
                    /* Add forward restriction port to bitmap */
                    SKERNEL_PORTS_BMP_ADD_PORT_MAC(forwardRestrictionBmpPtr, localPort);
                }
            }
        }
        else
        {
            if (descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_FROM_CPU_E)
            {
                regAddr = SMEM_CHT_FROM_CPU_FILTER_CONF_REG(devObjPtr);
            }
            else if (descrPtr->doRouterHa == 0)
            {
                regAddr = SMEM_CHT_BRIDGE_FILTER_CONF_REG(devObjPtr);
            }
            else
            {
                regAddr = SMEM_CHT_ROUTE_FILTER_CONF_REG(devObjPtr);
            }

            regPtr = smemMemGet(devObjPtr, regAddr);

            SKERNEL_FILL_PORTS_BITMAP_MAC(devObjPtr, &egressPortsBmp, regPtr);
            /* Fill forward restriction ports bitmap */
            __LOG(("Fill forward restriction ports bitmap"));
            SKERNEL_FILL_PORTS_BITMAP_MAC(devObjPtr, forwardRestrictionBmpPtr, regPtr);

            for (port = startPort,localPort = 0; port < endPort; port++ , localPort++)
            {
                destPorts[localPort] &=
                    ~SKERNEL_PORTS_BMP_IS_PORT_SET_MAC(&egressPortsBmp, port);
            }
            if(port < SNET_CHT_CPU_PORT_CNS)
            {
                localPort = SNET_CHT_CPU_PORT_CNS;
                port = SNET_CHT_CPU_PORT_CNS;
                destPorts[localPort] &=
                    ~SKERNEL_PORTS_BMP_IS_PORT_SET_MAC(&egressPortsBmp, port);
            }
        }
    }

    /*CHT3 OAM loopback enables */
    __LOG(("CHT3 OAM loopback enables"));
    if (oamLoopbackFilterEn[localDevSrcPort] &&
        descrPtr->egressOnIngressPortGroup)
    {
        for (port = startPort,localPort = 0; port < endPort; port++ , localPort++)
        {
            destPorts[localPort] =
            ((descrPtr->packetCmd != SKERNEL_EXT_PKT_CMD_TRAP_TO_CPU_E) &&
             (descrPtr->packetCmd != SKERNEL_EXT_PKT_CMD_FROM_CPU_E) &&
             (localDevSrcPort!= localPort) && descrPtr->egressOnIngressPortGroup) ?
                                  0 : destPorts[localPort];
        }

        if(port < SNET_CHT_CPU_PORT_CNS)
        {
            localPort = SNET_CHT_CPU_PORT_CNS;
            port = SNET_CHT_CPU_PORT_CNS;
            destPorts[localPort] =
                 ((descrPtr->packetCmd != SKERNEL_EXT_PKT_CMD_TRAP_TO_CPU_E) &&
                  (descrPtr->packetCmd != SKERNEL_EXT_PKT_CMD_FROM_CPU_E) &&
                  (localDevSrcPort != localPort) && descrPtr->egressOnIngressPortGroup) ?
                                               0 : destPorts[port];
        }

    }


}


/**
* @internal snetChtTxGetTrunkDesignatedPorts function
* @endinternal
*
* @brief   Trunk designated ports bitmap
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] trunkHashMode            - cascade trunk mode
*
* @param[out] designatedPortsBmpPtr    - pointer to designated port bitmap
*/
static void snetChtTxGetTrunkDesignatedPorts
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN TRUNK_HASH_MODE_ENT    trunkHashMode,
    OUT SKERNEL_PORTS_BMP_STC * designatedPortsBmpPtr
)
{
    DECLARE_FUNC_NAME(snetChtTxGetTrunkDesignatedPorts);

    GT_U32 regAddr;    /* register address */
    GT_U32 fldValue;   /* bit value */
    GT_U32 distTrunkHashMode; /* 0x0 = FirstLine , 0x1 = FunctionResult */
    GT_U32 * regPtr;   /* register data pointer */
    GT_U32 entry=0;    /* calculated entry number */
    GT_U32 vid = descrPtr->eVid; /* packet vid */
    GT_U32 splitDesignatedTableEnable; /* indication of split UC,MC designated table */


    if(devObjPtr->txqRevision != 0)
    {
        snetLionTxQGetTrunkDesignatedPorts(devObjPtr, descrPtr, trunkHashMode , designatedPortsBmpPtr);
        return;
    }

    smemRegFldGet(devObjPtr,
                  SMEM_CHT_TX_QUEUE_CTRL_REG(devObjPtr), 0, 1, &distTrunkHashMode);
    if (distTrunkHashMode)
    {
        if(descrPtr->useVidx == 0 /*cascade trunk*/&&
            trunkHashMode != TRUNK_HASH_MODE_USE_MULIT_DESTINATION_HASH_SETTINGS_E)
        {
            __LOG(("trg is cascade trunk ,trunkHashMode = [%d]",trunkHashMode));
            switch(trunkHashMode)
            {
                case TRUNK_HASH_MODE_USE_PACKET_HASH_E:
                    smemRegFldGet(devObjPtr,
                                  SMEM_CHT_TX_QUEUE_EXTEND_CTRL_REG(devObjPtr), 24, 1,
                                  &fldValue);
                    if (fldValue == 0)
                    {
                        __LOG(("use descrPtr->pktHash \n"));
                        entry = descrPtr->pktHash;
                    }
                    else
                    { /*  vid[2:0]  ^  vid[5:3] ^  vid[8:6] ^ vid[11:9]    ^ hash_index[2:0]*/
                        __LOG(("use vid[2:0]  ^  vid[5:3] ^  vid[8:6] ^ vid[11:9]    ^ hash_index[2:0] \n"));
                        entry = ( (vid) ^ (vid >> 3) ^ (vid >> 6) ^ (vid >> 9) ^
                                 (descrPtr->pktHash) );
                    }
                    break;
                case TRUNK_HASH_MODE_USE_GLOBAL_SRC_PORT_HASH_E:
                    __LOG(("use descrPtr->origSrcEPortOrTrnk"));
                    entry =  descrPtr->origSrcEPortOrTrnk;  /* same for orig port/trunk */
                    break;
                case TRUNK_HASH_MODE_USE_GLOBAL_DST_PORT_HASH_E:
                    __LOG(("use descrPtr->trgEPort"));
                    entry =  descrPtr->trgEPort;
                    break;
                case TRUNK_HASH_MODE_USE_LOCAL_SRC_PORT_HASH_E:
                    __LOG(("use descrPtr->localDevSrcPort"));
                    entry =  descrPtr->localDevSrcPort; /* use 'local' port */
                    break;
                default:
                    __LOG(("use entry 0"));
                    break;
            }
        }
        else /*descrPtr->useVidx*/
        {
            smemRegFldGet(devObjPtr,
                          SMEM_CHT_TX_QUEUE_EXTEND_CTRL_REG(devObjPtr), 24, 1,
                          &fldValue);
            if (fldValue == 0)
            {
                __LOG(("use descrPtr->pktHash \n"));
                entry = descrPtr->pktHash;
            }
            else
            { /*  vid[2:0]  ^  vid[5:3] ^  vid[8:6] ^ vid[11:9]    ^ hash_index[2:0]*/
                __LOG(("use vid[2:0]  ^  vid[5:3] ^  vid[8:6] ^ vid[11:9]    ^ hash_index[2:0] \n"));
                entry = ( (vid) ^ (vid >> 3) ^ (vid >> 6) ^ (vid >> 9) ^
                         (descrPtr->pktHash) );
            }

            if(devObjPtr->supportMcTrunkHashSrcInfo)
            {
                GT_U32 vidx = descrPtr->eVidx;
                /* Transmit Queue Extended Control1 */
                smemRegFldGet(devObjPtr,
                              SMEM_XCAT_TXQ_EXTENDED_CONTROL1_REG(devObjPtr),
                              8, 1,
                              &fldValue);

                /* The index to the Designated LAG Members Table for multicast packet is calculated according to source info
                (VID, VIFX and OrigSourcePort/Trunk). */
                if(fldValue)
                {
                    __LOG(("use descrPtr->pktHash \n"));
                    __LOG(("use vid[2:0]  ^  vid[5:3] ^  vid[8:6] ^ vid[11:9] ^     \n"
                           "    vidx[2:0] ^  vidx[5:3] ^  vidx[8:6] ^ vidx[11:9] ^  \n"
                           "    origSrcEPortOrTrnk[2:0] ^ origSrcEPortOrTrnk[5:3]   \n"));
                    /*  vid[2:0]  ^  vid[5:3] ^  vid[8:6] ^ vid[11:9] ^
                        vidx[2:0]  ^  vidx[5:3] ^  vidx[8:6] ^ vidx[11:9] ^*/
                    entry = ( (vid) ^ (vid >> 3) ^ (vid >> 6) ^ (vid >> 9) ^
                              (vidx) ^ (vidx >> 3) ^ (vidx >> 6) ^ (vidx >> 9) ^
                              (descrPtr->origSrcEPortOrTrnk) ^ (descrPtr->origSrcEPortOrTrnk >> 3) );
                }
            }

        }
    }

    entry &= 7;

    if(0 == devObjPtr->supportSplitDesignatedTrunkTable)
    {
        __LOG(("Use 'shared' UC,MC designated table \n"));

        regAddr = SMEM_CHT_TRUNK_DESIGNATED_REG(devObjPtr, entry);
    }
    else /* xcat3 */
    {
        smemRegFldGet(devObjPtr,
            SMEM_XCAT3_EGRESS_AND_TXQ_802_1_BR_GLOBAL_CONFIG_REG (devObjPtr), 2, 1, &splitDesignatedTableEnable);

        if(splitDesignatedTableEnable)
        {
            if(descrPtr->useVidx)
            {
                __LOG(("Use 'dedicated' MC designated table (high 8 entries) \n"));

                entry += 8;
            }
            else
            {
                __LOG(("Use 'dedicated' UC designated table (low 8 entries) \n"));
            }

            regAddr = SMEM_CHT_TRUNK_DESIGNATED_REG(devObjPtr, entry);
        }
        else
        {
            __LOG(("Use 'shared' UC,MC designated table (legacy mode - 8 entries) \n"));
            regAddr = SMEM_CHT_TRUNK_DESIGNATED_OLD_REG(devObjPtr, entry);
        }
    }

    __LOG(("final index[%d] to the designated port table \n",
        entry));

    regPtr = smemMemGet(devObjPtr, regAddr);

    SKERNEL_FILL_PORTS_BITMAP_MAC(devObjPtr, designatedPortsBmpPtr, regPtr);
}

/**
* @internal snetChtTxQPortLinkStat function
* @endinternal
*
* @brief   check how many ports are valid
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in,out] destPorts[]              - number of egress port.
* @param[in] oamLoopbackFilterEn[]    - array of OAM loop enabled
*
* @param[out] pcktCountBeforeFltr_PTR  - counters of filtered ports.
* @param[out] saveDestPortPtr          - destination ports saved for egress ports filtering
*/
static void snetChtTxQPortLinkStat
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    INOUT GT_U32 destPorts[],
    OUT GT_U32 * pcktCountBeforeFltr_PTR,
    OUT GT_U32 backupDestPort[],
    OUT SKERNEL_PORTS_BMP_STC  * saveDestPortPtr,
    IN GT_U32 oamLoopbackFilterEn[]
)
{
    DECLARE_FUNC_NAME(snetChtTxQPortLinkStat);

    GT_U32      port;        /* port index */
    GT_U32      maxPorts;/* max ports to loop on*/
    GT_U32      regAddress;  /* register address */
    GT_U32      linkUP;      /* link up/down */
    GT_U32      txQEnable;   /* Transmit queue enable/disable */
    GT_U32      fastFailoverEn = 0;
    GT_U32      termFastFailover = 0;
    GT_U32      ingressPortIsLooped;/* ingress port is looped */
    GT_U32      secondaryPortForIngressPort;/* the secondary port that ass traffic from looped port should be sent to */
    GT_U32      macPort;     /* MAC Port number */
    GT_U32      physicalPort;/* physical Port number */
    GT_U32 physicalPortLinkStatusMask;/*Sets the link status for the port, for local ports on this device use 0, otherwise the configuration must implement the link state.
                         0x0 = LinkState; LinkState; Filtering is performed based on the port link state in
                         0x1 = ForceLinkUp; ForceLinkUp; Packets to this physical port are not filtered
                         0x2 = ForceLinkDown; ForceLinkDown; Packets to this physical port are always filtered
                        */
    GT_U32      regValue;
    GT_U32      linkDownFilterEn;/*enables egress filtering of target ports which links are down*/
    SKERNEL_DEVICE_OBJECT  *txDmaDevObjPtr;/*pointer to the 'TX DMA port' egress device object*/
    GT_U32      *regPtr;
    LINK_CASE_ENT   linkCase;

    /* Before TxQ filtering check how many ports are 'UP' from the ports that need to get the traffic */
    *pcktCountBeforeFltr_PTR = 0;

    /* Controls the enabling and disabling of the TxQ block */
    if(devObjPtr->txqRevision != 0)
    {
        regAddress = SMEM_LION_TXQ_EGR_FILTER_GLOBAL_EN_REG(devObjPtr);
        smemRegGet(devObjPtr, regAddress,&regValue);

        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /* check the 'filter Enable' and the 'global filter enable' */
            if(SMEM_U32_GET_FIELD(regValue,0,1) &&
               SMEM_U32_GET_FIELD(regValue,1,1) )
            {
                termFastFailover = 1;
            }
            else
            {
                termFastFailover = 0;
            }

            /* check the 'filter Enable' and the 'global filter enable' */
            if(SMEM_U32_GET_FIELD(regValue,0,1) &&
               SMEM_U32_GET_FIELD(regValue,9,1) )
            {
                linkDownFilterEn = 1;
            }
            else
            {
                linkDownFilterEn = 0;
            }

            /* check the 'filter Enable' and the 'global filter enable' */
            if(SMEM_U32_GET_FIELD(regValue,0,1) &&
               SMEM_U32_GET_FIELD(regValue,11,1) )
            {
                fastFailoverEn = 1;
            }
            else
            {
                fastFailoverEn = 0;
            }

            regAddress = SMEM_LION3_EGF_EFT_EGR_FILTERS_GLOBAL_REG(devObjPtr);
            smemRegGet(devObjPtr, regAddress,&regValue);
            /* this is EGF and not TXQ ... */
            txQEnable =  SMEM_U32_GET_FIELD(regValue, 0, 1);
        }
        else
        {
            txQEnable = SMEM_U32_GET_FIELD(regValue, 0, 1);
            termFastFailover = SMEM_U32_GET_FIELD(regValue, 2, 1);
            linkDownFilterEn = 1;/*allow generic code*/
        }
    }
    else
    {
        smemRegFldGet(devObjPtr,
                SMEM_CHT_TX_QUEUE_CTRL_REG(devObjPtr), 1, 1, &txQEnable);
        if(SKERNEL_IS_CHEETAH3_DEV(devObjPtr))
        {
            smemRegFldGet(devObjPtr, SMEM_CHT3_DP2CFI_REG(devObjPtr), 4, 1, &termFastFailover);
        }

        linkDownFilterEn = 1;/*allow generic code*/
    }

    if (txQEnable == GT_FALSE)
    {
        __LOG(("TXQ is disabled \n"));
        return;
    }

    if(SKERNEL_IS_CHEETAH3_DEV(devObjPtr))
    {
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /* already got value for fastFailoverEn */
        }
        else
        {
            smemRegFldGet(devObjPtr, SMEM_CHT3_FAST_STACK_REG(devObjPtr), 0, 1, &fastFailoverEn);
        }
    }

    __LOG_PARAM(termFastFailover);

    if(descrPtr->useVidx == 0 &&
       descrPtr->trgDev == descrPtr->srcDev &&
       termFastFailover)
    {
        ingressPortIsLooped = 0;
    }
    else
    {
        ingressPortIsLooped = descrPtr->pktIsLooped;
    }

    __LOG_PARAM(ingressPortIsLooped);
    if(ingressPortIsLooped != descrPtr->pktIsLooped)
    {
        /* update the descriptor */
        descrPtr->pktIsLooped = ingressPortIsLooped;

        __LOG_PARAM(descrPtr->pktIsLooped);
    }

    if(descrPtr->egressOnIngressPortGroup &&
       descrPtr->pktIsLooped)
    {
        regAddress = SMEM_CHT3_SECONDARY_TARGET_PORT_MAP_TBL_MEM (devObjPtr, descrPtr->localDevSrcPort);
        regPtr = smemMemGet(devObjPtr, regAddress);
        secondaryPortForIngressPort =
            SNET_CHT_SECOND_PORT_FAST_STACK_FAILOVE_GET_MAC(devObjPtr, regPtr);

        __LOG(("packets coming from port [%d] are directed to port [%d] as 'Fast Stack Failover Secondary Target Port' \n",
            descrPtr->localDevSrcPort,
            secondaryPortForIngressPort));

        /* force the target ports to add this port (all other will be removed later) */
        destPorts[secondaryPortForIngressPort] = 1;
    }
    else
    {
        secondaryPortForIngressPort = 0xFFFFFFFF;/* will cause no match */
    }


    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* the EGF see all 256 ports */
        maxPorts = SNET_SIP5_EGR_MAX_PHY_PORTS_MAC(devObjPtr);
    }
    else
    {
        maxPorts = SNET_CHEETAH_MAX_PORT_NUM(devObjPtr);
    }
    __LOG_PARAM(maxPorts);
    __LOG_PARAM(devObjPtr->limitedResources.phyPort);
    __LOG_PARAM(devObjPtr->flexFieldNumBitsSupport.phyPort);

    for (port = 0; port < maxPorts; port++)
    {
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /* the EGF see all 256 ports , and not know which are 'valid' */
        }
        else
        if(IS_CHT_VALID_PORT(devObjPtr,port) == GT_FALSE)
        {
            /* Skip not existed ports */
            continue;
        }

        if (destPorts[port] == 0)
        {
            /* Skip ports that already filtered */
            continue;
        }

        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            if(!SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
            {
                GT_U32  numPortsInReg  = 4;
                GT_U32  numBitsPerPort = 8;

                regAddress = SMEM_LION2_EGF_EFT_VIRTUAL_2_PHYSICAL_PORT_REMAP_REG(devObjPtr,(port/numPortsInReg));
                /* map 'virtual port' to 'physical port' for the link filter */
                smemRegFldGet(devObjPtr, regAddress, numBitsPerPort*(port%numPortsInReg), numBitsPerPort, &physicalPort);
                __LOG(("map 'virtual port' [%d] to 'physical port' [%d] for the link filter \n",
                        port,
                        physicalPort));

                regAddress = SMEM_LION2_EGF_EFT_PHYSICAL_PORT_LINK_STATUS_MASK_REG(devObjPtr,(physicalPort/16));
                /* get link filter mode */
                smemRegFldGet(devObjPtr, regAddress, 2*(physicalPort%16), 2, &physicalPortLinkStatusMask);
                __LOG(("link filter mode for 'physical port' [%d] is [%s] \n",
                        physicalPort,
                        physicalPortLinkStatusMask == 0 ? "according to MAC register" :
                        physicalPortLinkStatusMask == 1 ? "force link UP" :
                        physicalPortLinkStatusMask == 2 ? "force link DOWN" :
                        " unknown "
                        ));
                linkCase = (physicalPortLinkStatusMask == 0) ? LINK_ACCORDING_TO_MAC_E:
                           (physicalPortLinkStatusMask == 1) ? FORCE_LINK_UP_E:
                           (physicalPortLinkStatusMask == 2 ) ? FORCE_LINK_DOWN_E:
                                                               LINK_UNKNOWN_E;
            }
            else
            {
                /* there is no mapping between virtual port to physical port */
                /* and there is only single bit for per port */
                physicalPort = port;
                __LOG(("(sip5.20: no mapping) : map 'virtual port' [%d] to 'physical port' [%d] for the link filter \n",
                        port,
                        physicalPort));

                regAddress = SMEM_LION2_EGF_EFT_PHYSICAL_PORT_LINK_STATUS_MASK_REG(devObjPtr,(physicalPort/32));
                /* get link filter mode */
                smemRegFldGet(devObjPtr, regAddress, (physicalPort%32), 1, &physicalPortLinkStatusMask);
                __LOG(("link filter mode for 'Virtual port' [%d] is [%s] \n",
                        physicalPort,
                        physicalPortLinkStatusMask == 0 ? "force link DOWN" :
                                                          "force link UP"
                        ));

                linkCase = (physicalPortLinkStatusMask == 0) ? FORCE_LINK_DOWN_E:
                                                               FORCE_LINK_UP_E;
            }
        }
        else
        {
            physicalPort = port;
            linkCase = LINK_ACCORDING_TO_MAC_E;/* for generic code */
        }

        macPort = physicalPort;

        if(linkCase == LINK_ACCORDING_TO_MAC_E)
        {
            if(GT_FALSE == snetChtTxMacPortGet(devObjPtr,
                descrPtr,
                MAC_PORT_REASON_TYPE_EGF_TO_MAC_FOR_LINK_STATUS_E,
                physicalPort,
                GT_TRUE,
                &txDmaDevObjPtr,
                &macPort))
            {
                /* MAC port not exists */

                destPorts[port] = 0;

                __LOG(("port[%d] - FILTERED. Reason: associated MAC not exists \n", port));

                continue;
            }

            simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_PORT_MAC_EGRESS_E);
            if(GT_FALSE == snetChtIsTxPortLinkUp(txDmaDevObjPtr,GT_TRUE,macPort))
            {
                /* MAC considered 'link DOWN' */
                linkUP = 0;
            }
            else
            {
                linkUP = 1;
            }
            simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT___RESTORE_PREVIOUS_UNIT___E);

        }
        else if (linkCase == FORCE_LINK_UP_E)/*ForceLinkUp*/
        {
            __LOG(("port [%d] treated as 'Force Link Up' \n",
                port));

            txDmaDevObjPtr = devObjPtr;
            linkUP = 1;
        }
        else  if (linkCase == FORCE_LINK_DOWN_E)/*ForceLinkDown*/
        {
            __LOG(("port [%d] treated as 'Force Link Down' \n",
                port));

            txDmaDevObjPtr = devObjPtr;
            linkUP = 0;
        }
        else
        {
            __LOG(("WARNING : bad state LINK_UNKNOWN_E : port [%d] treated as 'Link down' \n",
                port));

            txDmaDevObjPtr = devObjPtr;
            linkUP = 0;/* bad value of state - treat as link down */
        }

        __LOG(("port[%d] : LINK [%s] \n",
            port,
            linkUP ? "UP" : "DOWN"));

        /* enable the port only if it is enable and the link is up */

        if(linkDownFilterEn)/* check if need to filter the LINK DOWN ports */
        {
            if(descrPtr->useVidx == 0)
            {
                /* Only for single-destination traffic it's destination port is eligable for egress filtering */
                if (linkUP == 0)
                {
                    __LOG(("Single-destination port[%d] : disabled and/or link is down. Save port before link status filtering \n", port));
                    SKERNEL_PORTS_BMP_ADD_PORT_MAC(saveDestPortPtr, port);
                }
            }
            destPorts[port] &= linkUP;
        }

        if(linkUP)
        {
            (*pcktCountBeforeFltr_PTR)++;
        }

        if(port == secondaryPortForIngressPort)
        {
            backupDestPort[port] = 1;
            (*pcktCountBeforeFltr_PTR)++;
        }
        else if((secondaryPortForIngressPort != 0xFFFFFFFF) &&
                destPorts[port])
        {
            __LOG(("port [%d] filtered because traffic unconditionally go to secondary port [%d] \n",
            port,
            secondaryPortForIngressPort));

            destPorts[port] = 0;
            (*pcktCountBeforeFltr_PTR)--;
        }
    }

    if(secondaryPortForIngressPort != 0xFFFFFFFF)
    {
        if(port < SNET_CHT_CPU_PORT_CNS)
        {
            port = SNET_CHT_CPU_PORT_CNS;
            if(destPorts[port])
            {
                (*pcktCountBeforeFltr_PTR)++;
            }
        }

        /* global port 14 enabled always in Lion2 B0 */
        if(devObjPtr->supportTxQPort14LinkUp)
        {
            port = 14;
            destPorts[port] = 1;
            (*pcktCountBeforeFltr_PTR)++ ;
        }
    }
}

/**
* @internal lion3EgfShtSrcPortIgnorePhySrcMcFilterEnFilter function
* @endinternal
*
* @brief   get filter of Phy Src Mc for given srcPort.
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 -  pointer to the frame's descriptor.
* @param[in] srcPort                  -  the  to check if need to be filtered
*
* @retval GT_TRUE                  - need to filter the srcPort
* @retval GT_FALSE                 - not need to filter the srcPort
*/
static GT_BOOL lion3EgfShtSrcPortIgnorePhySrcMcFilterEnFilter
(
  IN SKERNEL_DEVICE_OBJECT * devObjPtr,
  IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
  IN GT_U32     srcPort
)
{
    DECLARE_FUNC_NAME(lion3EgfShtSrcPortIgnorePhySrcMcFilterEnFilter);

    GT_U32 fieldVal;                /* Register field value */
    GT_U32 regAddress;              /* Register/table address */
    GT_U32 * regPtr;                /* Pointer to register/table data */
    GT_BOOL  phySrcMcFilterEn;/* sip5 phy Src Mc Filter En*/

    regAddress = SMEM_LION3_EGF_SHT_IGNORE_PHY_SRC_MC_FILTER_EN_REG(devObjPtr);
    regPtr = smemMemGet(devObjPtr, regAddress);
    /* Used to enable Multicast filtering over cascade ports even if the
       packet is assigned <Phy Src MC Filter En> = 0.
       This value should be set to 1 over cascade ports.
    */
    fieldVal = snetFieldValueGet(regPtr,srcPort,1);

    if(fieldVal || descrPtr->eArchExtInfo.phySrcMcFilterEn)
    {
        /* we need to filter the localDevSrcPort */
        phySrcMcFilterEn = GT_TRUE;
    }
    else
    {
        phySrcMcFilterEn = GT_FALSE;
        __LOG(("src physical port [%d] NOT filtered by PHY_SRC_MC_FILTER \n",
            srcPort));
    }

    return phySrcMcFilterEn;
}

/**
* @internal snetChtTxQSrcPortFilters function
* @endinternal
*
* @brief   port MAC status filters , called only for useVidx = 1.
*         rest of the filters are in snetChtTxQFromCpuExcludePortOrTrunk
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 -  pointer to the frame's descriptor.
*                                      egressPort   -  number of egress port.
* @param[in] packetType               -  type of packet
*/
static void snetChtTxQSrcPortFilters
(
  IN SKERNEL_DEVICE_OBJECT * devObjPtr,
  IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
  INOUT GT_U32 destPorts[],
  IN SKERNEL_EGR_PACKET_TYPE_ENT packetType
)
{
    DECLARE_FUNC_NAME(snetChtTxQSrcPortFilters);

    GT_U32 fieldVal;                /* Register field value */
    GT_U32  localDevSrcPort;
    GT_U32 regAddress;              /* Register/table address */
    GT_U32 * regPtr;                /* Pointer to register/table data */
    SKERNEL_PORTS_BMP_STC egressPortsBmp;   /* Egress ports bitmap */
    GT_U32  dsaSrcPort;/* dsa src port (on own device)*/
    GT_U32  doFilter;/* indication to do filter */
    GT_U32  dsa_origSrcPhyIsTrunk;

    localDevSrcPort = descrPtr->localPortGroupPortAsGlobalDevicePort;/* local port(not global) */

    __LOG(("check if need to do next filters: \n"
           "1. Local Dev Src Port Filter \n"
           "2. Original Source Port Filter \n"));

    if (packetType == SKERNEL_EGR_PACKET_CNTRL_UCAST_E)
    {
        __LOG(("packet classified as control unicast is not subject to those filters \n"));
        return;
    }

    if(descrPtr->pktIsLooped)
    {
        /* looped back packet not subject to this filter  */
        __LOG(("'pktIsLooped = 1' packet not subject to those filters \n"));
        return;
    }

    /* Filter name : Local Dev Src Port Filter */
    /*
        from sip5:
        (Desc<UseVIDX> == 1) &&
        (Desc<Local Dev Src Trunk ID> == 0) &&
        (Desc<Do Route HA> == 0) &&
        (Desc<Outgoing Mtag Cmd> != TO_ANALYZER || Desc<Src Dev Is Own> == 0) &&
        (Conf<Src Port Filter En> == 1 && Conf<EFTGlobalEgressFilterEn> == 1) &&
        (packet is not looped && Internal<Pkt Class> != Unicast Control) &&
        (SourcePortsInGlobalBitmap<MC Local En> == 0 || eVLAN<Local Switching En> == 0) &&
        (Desc<Phy Src Mc Filter En> ==1 || SourcePortsInGlobalBitmap<Ignore Phy Src Mc Filter En> == 1)
    */
    if (descrPtr->localDevSrcTrunkId == 0 &&  /* no source trunk */
        descrPtr->egressOnIngressPortGroup)
    {
        doFilter = 0;

        /* Routed MLL frames not filter ingress port, unless in the same ingress vlan  */
        regAddress = SMEM_CHT_MULTICAST_LCL_ENABLE_REG(devObjPtr);
        regPtr = smemMemGet(devObjPtr, regAddress);
        SKERNEL_FILL_PORTS_BITMAP_MAC(devObjPtr, &egressPortsBmp, regPtr);

        if (SKERNEL_PORTS_BMP_IS_PORT_SET_MAC(&egressPortsBmp, localDevSrcPort) == 0)
        {
            /* MC local filter by Port */
            doFilter = 1;
        }
        else
        {
            __LOG(("src physical port (localDevSrcPort) [%d] ALLOW local switching \n",
                localDevSrcPort));
        }

        if (descrPtr->egressVlanInfo.mcLocalEn == 0)
        {
            /* For XCat chip family local switching for multi-destination packets
            received on a non-trunk port is enabled/disabled per egress port
            and per VLAN */
            doFilter = 1;
        }
        else
        {
            __LOG(("egress Vlan [%d] ALLOW local switching \n",
                descrPtr->eVid));
        }

        if(doFilter && SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            smemRegGet(devObjPtr, SMEM_LION_TXQ_EGR_FILTER_GLOBAL_EN_REG(devObjPtr),&fieldVal);

            /* check the 'Source Port Filtering Enable' and the 'global filter enable' */
            if(SMEM_U32_GET_FIELD(fieldVal,0,1) &&
               SMEM_U32_GET_FIELD(fieldVal,3,1) )
            {
                doFilter = 1;
            }
            else
            {
                doFilter = 0;
                __LOG(("'Source Port Filtering Enable' ALLOW local switching \n"));
            }
        }


        if(descrPtr->doRouterHa == 1)
        {
            __LOG(("src physical port (localDevSrcPort) [%d] NOT filtered due to doRouterHa \n",
                localDevSrcPort));
            doFilter = 0;
        }
        else
        if((descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E) &&
           (descrPtr->srcTrgDev == descrPtr->ownDev))
        {
            __LOG(("src physical port (localDevSrcPort) [%d] NOT filtered due to descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E on 'own' device \n",
                localDevSrcPort));
            doFilter = 0;
        }


        if(doFilter && SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /* the filter can be disabled by the 'origSrcEport'
               (by setting descrPtr->eArchExtInfo.phySrcMcFilterEn = 0)
                but can be forced by the ingress physical port */
            doFilter =
                lion3EgfShtSrcPortIgnorePhySrcMcFilterEnFilter(devObjPtr,descrPtr,
                    localDevSrcPort);
        }

        if(doFilter)
        {
            destPorts[localDevSrcPort] = 0; /* filter source local port */
            __LOG(("src physical port (localDevSrcPort) [%d] filtered \n",
                localDevSrcPort));
        }
    }

    /* Filter name : Original Source Port Filter */
    if(descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_FROM_CPU_E)
    {
        /* to be handled by snetChtTxQFromCpuExcludePortOrTrunk(...) */
        return;
    }

    dsa_origSrcPhyIsTrunk = SMEM_CHT_IS_SIP5_GET(devObjPtr) ?
        descrPtr->eArchExtInfo.origSrcPhyIsTrunk :
        descrPtr->origIsTrunk;

    if ( (descrPtr->srcDev == descrPtr->ownDev) && /* fixed from descrPtr->srcDevIsOwn to support dual device Id*/
         (dsa_origSrcPhyIsTrunk == 0) &&
         (descrPtr->routed == 0) &&
         (descrPtr->marvellTagged == 1))
    {

        /* Extended filter for Multi-destination DSA tagged packet that was received by the local device */
        __LOG(("Extended filter for Multi-destination DSA tagged packet that was received by the local device"));

        if (SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /* use the 'original' src physical port */
            dsaSrcPort = descrPtr->eArchExtInfo.origSrcPhyPortTrunk;
            __LOG_PARAM(descrPtr->eArchExtInfo.origSrcPhyPortTrunk);
        }
        else
        if((devObjPtr)->portGroupSharedDevObjPtr)
        {
            /* the original src port is on this device */
            dsaSrcPort = SMEM_CHT_LOCAL_PORT_FROM_GLOBAL_PORT_MAC(devObjPtr,descrPtr->origSrcEPortOrTrnk);

            if(SMEM_CHT_PORT_GROUP_ID_MASK_CORE_MAC(devObjPtr, devObjPtr->portGroupId, devObjPtr->dualDeviceIdEnable.txq) !=
               SMEM_CHT_PORT_GROUP_ID_FROM_GLOBAL_PORT_MAC(devObjPtr,descrPtr->origSrcEPortOrTrnk))
            {
                /* the source port not belongs to this port group */
                __LOG(("the source port not belongs to this port group"));
                return;
            }
        }
        else
        {
            dsaSrcPort = descrPtr->origSrcEPortOrTrnk;
        }

        if(devObjPtr->txqRevision != 0)
        {
            regAddress = SMEM_LION_TXQ_EGR_FILTER_GLOBAL_EN_REG(devObjPtr);
            if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
            {
                smemRegGet(devObjPtr, regAddress,&fieldVal);

                /* check the 'filter Enable' and the 'global filter enable' */
                if(SMEM_U32_GET_FIELD(fieldVal,0,1) &&
                   SMEM_U32_GET_FIELD(fieldVal,2,1) )
                {
                    fieldVal = 1;
                }
                else
                {
                    fieldVal = 0;
                }
            }
            else
            {
                smemRegFldGet(devObjPtr, regAddress, 3, 1, &fieldVal);
            }
        }
        else
        {
            regAddress = SMEM_CHT_TX_QUEUE_EXTEND_CTRL_REG(devObjPtr);
            smemRegFldGet(devObjPtr, regAddress, 5, 1, &fieldVal);
        }

        if(fieldVal == 0)
        {
            __LOG(("Original Source Port Filter : NOT Filtered port [%d] (that came from DSA tag) due to global config \n",
                dsaSrcPort));
            return;
        }

        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /* the filter can be disabled by the 'origSrcEport'
               (by setting descrPtr->eArchExtInfo.phySrcMcFilterEn = 0)
                but can be forced by the 'orig' ingress physical port */
            doFilter =
                lion3EgfShtSrcPortIgnorePhySrcMcFilterEnFilter(devObjPtr,descrPtr,
                    dsaSrcPort);
            if(doFilter == 0)
            {
                return;
            }
        }

        destPorts[dsaSrcPort] = 0 ;
        __LOG(("Original Source Port Filter : Filtered port [%d] (that came from DSA tag) \n",
            dsaSrcPort));
    }
}

/**
* @internal snetRavenLmuProcessing function
* @endinternal
*
* @brief   Raven's latency monitoring processing
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in,out] descrPtr             -  pointer to the frame's descriptor.
* @param[in] egressPort               -  number of egress port.
*
*/
GT_VOID snetRavenLmuProcessing
(
    IN SKERNEL_DEVICE_OBJECT              * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32                               macPort
)
{
    DECLARE_FUNC_NAME(snetRavenLmuProcessing);

    GT_U32 regAddr;
    GT_U32 *memPtr;
    GT_U32 lmuNum;
    GT_U32 ravenNum;
    GT_U32 cfgRangeMin;
    GT_U32 cfgRangeMax;
    GT_U32 cfgNotificationThresh;
    GT_U64 statInRange;
    GT_U32 statOutOfRange;
    GT_U32 statMinLatency;
    GT_U32 statMaxLatency;
    GT_U32 statAvgLatency;
    GT_U32 oldStatMinLatency;
    GT_U32 oldStatMaxLatency;
    GT_U32 oldStatAvgLatency;
    GT_U32 latency;
    GT_U64 one;
    GT_U32 coef;
    GT_U32 chanEnable;
    GT_U32 channel;
    GT_U32 globalRavenNum,localRavenNumInPipe;
    GT_U32 currTimeInMilli,currTimeStamp;

    if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr) && devObjPtr->numOfLmus == 0)
    {
        __LOG(("The device not supports Latency monitoring (LMU) processing \n"));
        return;
    }

    if(descrPtr->lmuEn == GT_FALSE)
    {
        __LOG(("Latency monitoring disabled\n"));
        return;
    }

    /* get LMU enable per channel (macPort) */
    chanEnable = snetChtPortMacFieldGet(devObjPtr, macPort,
        SNET_CHT_PORT_MAC_FIELDS_LMU_enable_E);

    if(chanEnable == 0)
    {
        __LOG(("macPort [%d] : Latency monitoring is disabled \n",macPort));
        return;
    }

    one.l[0] = 1;
    one.l[1] = 0;

    if(devObjPtr->numOfRavens == 0)/*Hawk*/
    {
        ENHANCED_PORT_INFO_STC portInfo;

        devObjPtr->devMemPortInfoGetPtr(devObjPtr,SMEM_UNIT_TYPE_PCA_LMU_CHANNEL_E  ,macPort,&portInfo);

        ravenNum = 0;/* ignored */
        lmuNum   = portInfo.simplePortInfo.unitIndex;
        channel  = portInfo.simplePortInfo.indexInUnit;
        __LOG(("Latency monitoring channel is Enabled (lmuNum=[%d], channel=[%d]) for macPort [%d] \n",
               lmuNum, channel, macPort));
    }
    else
    {
        /* Convert physical port to Raven port, channel and local Raven indexes */
        smemRavenGlobalPortConvertGet(devObjPtr,macPort,&globalRavenNum,&localRavenNumInPipe,NULL,&channel,&lmuNum,NULL);
        ravenNum = localRavenNumInPipe;

        __LOG(("Latency monitoring channel is Enabled (global ravenNum=[%d] , lmuNum=[%d], channel=[%d]) for macPort [%d] \n",
               globalRavenNum, lmuNum, channel, macPort));
    }

    regAddr = SMEM_SIP6_LATENCY_MONITORING_CFG_TBL_MEM(devObjPtr,
        ravenNum,lmuNum, descrPtr->lmuProfile);
    memPtr = smemMemGet(devObjPtr, regAddr);
    cfgRangeMin = snetFieldValueGet(memPtr, 60, 30);
    cfgRangeMax = snetFieldValueGet(memPtr, 30, 30);
    cfgNotificationThresh = snetFieldValueGet(memPtr, 0, 30);
    __LOG(("Latency profile configuration for profile# %d\n"
           "Range: [%lu..%lu]\n"
           "Notification threshold: %lu\n",
           descrPtr->lmuProfile, cfgRangeMin, cfgRangeMax,
           cfgNotificationThresh));

    regAddr = SMEM_SIP6_LATENCY_MONITORING_STAT_TBL_MEM(devObjPtr,
        ravenNum,lmuNum, descrPtr->lmuProfile);

    memPtr = smemMemGet(devObjPtr, regAddr);
    statOutOfRange      = snetFieldValueGet(memPtr, 0,   32);
    statInRange.l[0]    = snetFieldValueGet(memPtr, 32,  32);
    statInRange.l[1]    = snetFieldValueGet(memPtr, 64,  32);
    oldStatAvgLatency   = snetFieldValueGet(memPtr, 96,  30);
    oldStatMaxLatency   = snetFieldValueGet(memPtr, 126, 30);
    oldStatMinLatency   = snetFieldValueGet(memPtr, 156, 30);

    /* the current time in milisec */
    currTimeInMilli = SIM_OS_MAC(simOsTickGet)();
    /* convert the current time to timestamp format (2 bits seconds , 30 bits nano) */
    /* since the descrPtr->packetTimestamp hold same format */
    currTimeStamp   = SNET_CONVERT_MILLISEC_TO_TIME_STAMP_MAC(currTimeInMilli);

    latency = 500 + (rand() & 0x3F) + (currTimeStamp - descrPtr->packetTimestamp);
    if((latency >= cfgRangeMin) && (latency <= cfgRangeMax))
    {
        statInRange = prvSimMathAdd64(statInRange, one);
    }
    else
    {
        statOutOfRange += 1;
    }

    regAddr = SMEM_SIP6_LMU_AVERAGE_COEFFICIENT_REG(devObjPtr,
        ravenNum, lmuNum);
    smemRegFldGet(devObjPtr, regAddr, 0, 5, &coef);

    statMaxLatency = (latency > oldStatMaxLatency) ? latency : oldStatMaxLatency;
    statMinLatency = (latency < oldStatMinLatency) ? latency : oldStatMinLatency;
    if(oldStatAvgLatency != 0)
    {
        statAvgLatency = ((oldStatAvgLatency << coef) - oldStatAvgLatency + latency) >> coef;
    }
    else
    {
        statAvgLatency = latency;
    }

    __LOG_PARAM(statOutOfRange);
    __LOG_PARAM(statInRange.l[0]);
    __LOG_PARAM(statInRange.l[1]);
    __LOG_PARAM(statAvgLatency);
    __LOG_PARAM(statMaxLatency);
    __LOG_PARAM(statMinLatency);
    snetFieldValueSet(memPtr, 0,   32, statOutOfRange);
    snetFieldValueSet(memPtr, 32,  32, statInRange.l[0]);
    snetFieldValueSet(memPtr, 64,  32, statInRange.l[1]);
    snetFieldValueSet(memPtr, 96,  30, statAvgLatency);
    snetFieldValueSet(memPtr, 126, 30, statMaxLatency);
    snetFieldValueSet(memPtr, 156, 30, statMinLatency);

    if(latency > cfgNotificationThresh)
    {
        /* TODO: Generate interrupt */
    }
}

static GT_U32   debug_performance_bypass_tx_mif_mapping = 0;
GT_STATUS debug_performance_bypass_tx_mif_mapping_set(GT_U32 bypass)
{
    debug_performance_bypass_tx_mif_mapping = bypass;
    return GT_OK;
}

/**
* @internal snetSip6_30PtpUsxgmiiPchTxProcessing function
* @endinternal
*
* @brief   USXGMII PCH egress processing.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] macPort                  - target mac port number.
* @param[out] pchInfoPtr              - pointer to PCH PTP information.
*
*/
static GT_VOID snetSip6_30PtpUsxgmiiPchTxProcessing
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 macPort,
    OUT SNET_PHY_SWITCH_USGMII_PCH_PTP_INFO_STC * pchInfoPtr
)
{
    if (!SMEM_CHT_IS_SIP6_30_GET(devObjPtr) || devObjPtr->supportPtp != 1)
        return;

#if 0 /*CPSS_TBD_BOOKMARK_IRONMAN*/
    DECLARE_FUNC_NAME(snetSip6_30PtpUsxgmiiPchTxProcessing);

    GT_U32 regAddr,regAddr1;    /* Register address */
    GT_U32 *regPtr,*regPtr1;    /* Register pointer */
    SMEM_UNIT_TYPE_ENT     unitType;
    ENHANCED_PORT_INFO_STC portInfo;
    GT_U32  sip6_MTI_EXTERNAL_representativePort;

    if (!SMEM_CHT_IS_SIP6_30_GET(devObjPtr) || devObjPtr->supportPtp != 1)
    {
        descrPtr->egressPchInfoValid = 0;
        return;
    }

    /* check USXGMII Tx processing condition */

    regAddr = SMEM_SIP6_10_GOP_PTP_TS_PRECISION_CTRL_REG(devObjPtr,macPort);
    regPtr = smemMemGet(devObjPtr, regAddr);

    /* bit 3 - Channel %n Egress TS USX PCH Insertion; When this bit is enabled, channel %n frames are transmitted with in-band PCH, assembled
       by the egress port MAC layer. This mode is relevant to USX-GMII port modes, when time stamping is done by an external PHY device. */
    if (SMEM_U32_GET_FIELD(regPtr[0], 3, 1) == 1)
    {
        __LOG(("Channel [%d] is not enabled for Egress TS USX PCH Insertion.\n",macPort));
        descrPtr->egressPchInfoValid = 0;
        return;
    }

    /* build PCH information */
    regAddr = USX_PORT_EXT(devObjPtr,macPort).portUsxPchControl;
    regPtr = smemMemGet(devObjPtr, regAddr);

    unitType = SMEM_MTI_UNIT_TYPE_GET(devObjPtr,macPort);
    devObjPtr->devMemPortInfoGetPtr(devObjPtr,unitType,macPort,&portInfo);
    sip6_MTI_EXTERNAL_representativePort = portInfo.sip6_MTI_EXTERNAL_representativePortIndex;
    regAddr1 = USX_PORT_GLOBAL(devObjPtr,sip6_MTI_EXTERNAL_representativePort).globalUsxPchControl;
    regPtr1 = smemMemGet(devObjPtr, regAddr1);

    pchInfoPtr->packetType = SMEM_U32_GET_FIELD(regPtr1[0], 6, 2);
    pchInfoPtr->subPortId = SMEM_U32_GET_FIELD(regPtr[0], 2, 4);
    pchInfoPtr->extentionType = SMEM_U32_GET_FIELD(regPtr[0], 6, 2);
    pchInfoPtr->ptpTimestampEnable = 1; /* dummy */
    if(SMEM_U32_GET_FIELD(regPtr1[0], 0, 1) == 0 /* Q_ENTRY_ID */)
    {
        memcpy(&pchInfoPtr->signature,&descrPtr->ptpPhyInfo,sizeof(SNET_PTP_PHY_INFO_STC));
    }
    else /* CONFIG_CTR */
    {
        regAddr = USX_PORT_EXT(devObjPtr,macPort).portUsxPchSignatureControl;
        regPtr = smemMemGet(devObjPtr, regAddr);
        pchInfoPtr->signature.mchSignature = SMEM_U32_GET_FIELD(regPtr[0], 0, 16);
        if(pchInfoPtr->ptpTimestampEnable)
        {
            /* allow counter wraps around */
            pchInfoPtr->frameCount++;
        }
    }

    descrPtr->egressPchInfoValid = 1;
    __LOG(("Egress PCH information is valid.\n"));
#endif
    return;
}

/**
* @internal snetSip6_30PtpEgressPhyBuildUsxgmiiPch function
* @endinternal
*
* @brief   Egress PHY build USXGMII PCH PTP header.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] macPort                  - target mac port number.
* @param[in] pchInfoPtr               - pointer to PCH PTP information.
* @param[in] frameDataPtr             - pointer to start of frame.
*
*/
static GT_VOID snetSip6_30PtpEgressPhyBuildUsxgmiiPch
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 macPort,
    IN SNET_PHY_SWITCH_USGMII_PCH_PTP_INFO_STC * pchInfoPtr,
    IN GT_U8 * frameDataPtr
)
{
    DECLARE_FUNC_NAME(snetSip6_30PtpEgressPhyBuildUsxgmiiPch);

    GT_U8  *ptpStart;           /* pointer to start of timestamp related packet */
    GT_U32 taiGroup;            /* the TAI group used for that port */
    SNET_TOD_TIMER_STC deltaTimer;
    GT_U64 correctionField,oldCorrectionField;
    GT_U64 deltaTimer64;
    SNET_TOD_TIMER_STC tai4TimeCounter;
    GT_U32 regAddr;    /* Register address */
    GT_U32 *regPtr;    /* Register pointer */

    if (!SMEM_CHT_IS_SIP6_30_GET(devObjPtr) || devObjPtr->supportPtp != 1)
        return;

    taiGroup = SNET_LION3_PORT_NUM_TO_TOD_GROUP_CONVERT_MAC(devObjPtr ,macPort);

    /* sample Timestamp for TAI4 time */
    snetLion3PtpTodGetTimeCounter(descrPtr->egressPhysicalPortInfo.txDmaDevObjPtr,
                                  taiGroup,
                                  4,
                                  &tai4TimeCounter);

    if (pchInfoPtr->signature.step == SNET_PTP_PHY_STEP_TYPE_TWO_STEP_E)
    {
        regAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[taiGroup][4].phySignature;
        regPtr = smemMemGet(devObjPtr, regAddr);
        SMEM_U32_SET_FIELD(regPtr[0], 0, 16, pchInfoPtr->signature.mchSignature);

        regAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[taiGroup][4].phyTimestampSecLow;
        regPtr = smemMemGet(devObjPtr, regAddr);
        regPtr[0] = tai4TimeCounter.secondTimer.l[0];

        regAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[taiGroup][4].phyTimestampSecHigh;
        regPtr = smemMemGet(devObjPtr, regAddr);
        regPtr[0] = tai4TimeCounter.secondTimer.l[1];

        regAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TAI.TAI[taiGroup][4].phyTimestampNanoSec;
        regPtr = smemMemGet(devObjPtr, regAddr);
        regPtr[0] = tai4TimeCounter.nanoSecondTimer;

    }
    else /* SNET_PTP_PHY_STEP_TYPE_ONE_STEP_E */
    {
        /* PTP timestamping based on Local Action Table */
       ptpStart = frameDataPtr + (pchInfoPtr->signature.cfOffset << 1); /* convert from 2B resolution; nanoSec part */

        /* correction field header */

        /* 0    15 16                       63
        +--------+--------+--------+--------+
        | frac   |                          |
        | NanoSec|         NanoSec(48bit)   |
        +--------+--------+--------+--------+*/

        oldCorrectionField.l[0] = *(ptpStart) & 0xFFFFFFFF;
        oldCorrectionField.l[1] = *(ptpStart+4) & 0xFFFF;

        /* check for a WA in the nanoSec */
        if((tai4TimeCounter.nanoSecondTimer & 1) > pchInfoPtr->signature.ingTimeSecLsb)
        {
            /* no WA or no need to check for WA */
            deltaTimer.nanoSecondTimer = tai4TimeCounter.nanoSecondTimer -
                                         pchInfoPtr->timestamp.nanoSecondTimer;
        }
        else
        {
            deltaTimer.nanoSecondTimer = (10^9) -
                                         (tai4TimeCounter.nanoSecondTimer -
                                          pchInfoPtr->timestamp.nanoSecondTimer);
        }

        deltaTimer64.l[0] = deltaTimer.nanoSecondTimer;
        deltaTimer64.l[1] = 0;
        correctionField.l[0] = 0;
        correctionField.l[1] = 0;

        correctionField = prvSimMathAdd64(oldCorrectionField,deltaTimer64);

        /* check for WA in correction field; if there is WA put error value instead of new correction value. */
        if (correctionField.l[1] > 0xFFFF)
        {
            /* error value = 0 in msbit, 1 in all other bits */
            correctionField.l[0] = 0xFFFFFFFF;
            correctionField.l[1] = 0x7FFFFFFF;
             __LOG(("WA in correction field from phy. Error value will be set instead of new correction value.\n"));
        }

        *(ptpStart++) = (correctionField.l[0] >> 24) & 0xFF;
        *(ptpStart++) = (correctionField.l[0] >> 16) & 0xFF;
        *(ptpStart++) = (correctionField.l[0] >> 8) & 0xFF;
        *(ptpStart++) = (correctionField.l[0] >> 0) & 0xFF;
        *(ptpStart++) = (correctionField.l[1] >> 8) & 0xFF;
        *(ptpStart++) = (correctionField.l[1] >> 0) & 0xFF;
        __LOG(("phy updated new correction field: [0][%d] [1][%d].\n",correctionField.l[0],correctionField.l[1]));

        /* update udp trailer */
        if (pchInfoPtr->signature.ChecksumUpdate == GT_TRUE)
        {
            snetlion3PtpUdpChecksum(devObjPtr, descrPtr);
        }
    }

    return;
}

/**
* @internal snetChtTx2Port function
* @endinternal
*
* @brief   Forward frame to target port
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in,out] descrPtr                 -    pointer to the frame's descriptor.
* @param[in] egressPort               -   number of egress port.
* @param[in] frameDataPtr             - pointer to frame data
*                                      frameDataSize - frame data size
*/
static GT_BOOL internal_snetChtTx2Port
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 egressPort,
    IN GT_U8 *frameDataPtr,
    IN GT_U32 FrameDataSize
)
{
    DECLARE_FUNC_NAME(internal_snetChtTx2Port);

    GT_U32 portInLoopBack;
    GT_U32  macPort;   /* MAC Port number */
    GT_U32  slanPortNum;/* the port that hold the slan */
    GT_U32  PaddingDis;/*is padding of the MAC disabled */
    GT_U32  isPreemptiveChannel = 0;
    GT_U32 regAddr;  /* Register address */
    GT_U32 *regPtr;  /* Register entry pointer */
    SNET_PHY_SWITCH_USGMII_PCH_PTP_INFO_STC pchInfo;

    memset(&pchInfo,0,sizeof(SNET_PHY_SWITCH_USGMII_PCH_PTP_INFO_STC));
    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_PORT_MAC_EGRESS_E);

    if(GT_FALSE == snetChtTxMacPortGet(devObjPtr,
            descrPtr,
            MAC_PORT_REASON_TYPE_FINAL_MAC_E,
            egressPort,
            GT_FALSE,
            NULL,
            &macPort))
    {
        /* MAC port not exists */
        return GT_FALSE;
    }


    if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr) && !debug_performance_bypass_tx_mif_mapping)
    {
        GT_U32  mif_is_clock_enable;
        GT_U32  mif_is_channel_force_link_down;

        /* NOTE: the macPort got the descrPtr->egressPhysicalPortInfo.txDmaMacPort that hold the txDMA.
           but we need to convert it to 'real mac number'
        */
        snetHawkMifTxDmaToMacConvert(devObjPtr,
            descrPtr->egressPhysicalPortInfo.txDmaMacPort,
            &macPort,
            &isPreemptiveChannel,
            &mif_is_clock_enable,
            &mif_is_channel_force_link_down);
        if(!mif_is_clock_enable)
        {
            __LOG(("packet dropped : <mif_is_clock_enable> == 0 \n"));
            return GT_FALSE;
        }
        if(mif_is_channel_force_link_down)
        {
            __LOG(("packet dropped : <mif_is_channel_force_link_down> == 1 \n"));
            return GT_FALSE;
        }

        if(macPort != descrPtr->egressPhysicalPortInfo.txDmaMacPort)
        {
            __LOG_PARAM(descrPtr->egressPhysicalPortInfo.txDmaMacPort);
            __LOG_PARAM(macPort);
        }

        /* from this point till end of this functions ... all the 'sub functions' (in the 'MAC') can do 'Get'
            to retrieve the isPreemptiveChannel of this task (in this device) */
        smemSetCurrentTaskExtParamValue(devObjPtr,TASK_EXT_PARAM_INDEX_IS_PREEMPTIVE_CHANNEL,isPreemptiveChannel);
    }


    if(GT_FALSE == snetChtIsTxPortLinkUp(devObjPtr,GT_FALSE,macPort))
    {
        /* MAC considered 'link DOWN' */
        return GT_FALSE;
    }

    if(descrPtr->haToEpclInfo.paddingZeroForLess64BytesLength)
    {
        PaddingDis = snetChtPortMacFieldGet(devObjPtr, macPort,
            SNET_CHT_PORT_MAC_FIELDS_PaddingDis_E);

        if(PaddingDis == 1)
        {
            __LOG(("Egress MAC : 64bytes count padding disabled , although egress pipe considered 64bytes !!! \n"));
            /* remove the padding from the packet */
            FrameDataSize -= descrPtr->haToEpclInfo.paddingZeroForLess64BytesLength;
        }
    }

    portInLoopBack = snetChtPortMacFieldGet(devObjPtr, macPort,
        SNET_CHT_PORT_MAC_FIELDS_portInLoopBack_E);
    if (portInLoopBack && CHT_IS_MTI_MAC_USED_MAC(devObjPtr, macPort))
    {
        /* check MTI port RX path enable status */
        if (0 == snetChtPortMacFieldGet(devObjPtr, macPort,
            SNET_CHT_PORT_MAC_FIELDS_rx_path_en_E))
        {
            /* the Rx of the port is disabled , the packet is going to be dropped
               at ingress without any mac counters indications !

               but in simulation it is implemented by using 'buffer' that send the packet to 'skernel' task.
               this is a waist of time , and waist of 'buffer' that may be needed by other 'real' loopback ports.

               so ... to make it more efficient ... consider it as 'not loopback'

               This is MANDATORY when we have MANY ports in LB and Rx disabled
            */

            __LOG(("The MAC[%d] is in loopback mode , but the 'Rx' is disabled , so simulation will ignore the loopback . meaning will only egress it without the loopback (for better performance) \n",
                macPort));

            portInLoopBack = 0;
        }
    }

    if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
    {
        /* Precision Mode (Mux) Select */
        regAddr = SMEM_SIP6_10_GOP_PTP_TS_PRECISION_CTRL_REG(devObjPtr,macPort);
        regPtr = smemMemGet(devObjPtr, regAddr);

        if (descrPtr->mtiCfUpdate)
        {
            /* MAC will update CF in PTP header */
            snetSip6_30PtpMacCorrectionFieldUpdate(devObjPtr,descrPtr,macPort);

            descrPtr->ptpPrecisionMode = SKERNEL_PTP_PRECISION_MODE_CF_E;

            /* bit 6: Channel %n Egress MCH Enable: When this bit is enabled, channel %n egress port PHY/MAC timestamp the packet in MCH mode.
               bit 7: Channel %n Egress MCH Capture Step: 2-step; The PHY/MAC captures the PTP packet egress time in its timestamp queue */
            if (SMEM_U32_GET_FIELD(regPtr[0], 6, 2) != 3)
            {
                /* if the port is enabled for 2 step MCH, override the precision mode to USX */
                __LOG(("Override precision Mode to USX: Channel [%d] is not enabled for Egress TS Correction.\n",macPort));
                descrPtr->ptpPrecisionMode = SKERNEL_PTP_PRECISION_MODE_USX_E;
            }
        }
        else
        {
            /* bit 3: Channel %n Egress TS USX PCH Insertion: When this bit is enabled, channel %n frames are transmitted with in-band PCH,
                      assembled by the egress port MAC layer.*/
            if ((SMEM_U32_GET_FIELD(regPtr[0], 3, 1) == 1) && descrPtr->macTimestampingEnable)
            {
                descrPtr->ptpPrecisionMode = SKERNEL_PTP_PRECISION_MODE_USX_E;
            }
            else
            {
                descrPtr->ptpPrecisionMode = SKERNEL_PTP_PRECISION_MODE_TS_DISABLE_E;
            }
        }

    }

    /* if targetPort is connected to PHY && port is USXGMII && CF was not done in the MAC */
    if(connectedPortToPhyEgress[egressPort] && devObjPtr->portsArr[egressPort].state == SKERNEL_PORT_STATE_MTI_USX_E && descrPtr->ptpPrecisionMode == SKERNEL_PTP_PRECISION_MODE_USX_E)
    {
        /* MAC creates USXGMII PCH for the PHY */
        snetSip6_30PtpUsxgmiiPchTxProcessing(devObjPtr,descrPtr,macPort,&pchInfo);
    }


    if(descrPtr->timestampActionInfo.doEgressTimeStampEgressPort == GT_TRUE)
    {
        __LOG(("Port (TSU) Egress timestamp queue"));
        snetlion3TimestampPortEgressQueueInsertEntry(devObjPtr,macPort,
            descrPtr->timestampActionInfo.egressInfo.egressQueueNum,
            descrPtr->timestampActionInfo.egressInfo.egress_timestampQueueEntry);
    }

    /* Capture to timestamp port MAC egress queue */
    snetSip6_30TimestampPortMacEgressQueueCapture(devObjPtr,descrPtr,egressPort,macPort,&pchInfo);

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        SCIB_SEM_TAKE;
        snetRavenLmuProcessing(devObjPtr, descrPtr, macPort);
        SCIB_SEM_SIGNAL;
    }

    /* if targetPort is connected to PHY && port is USXGMII && CF was not done in the MAC */
    if(connectedPortToPhyEgress[egressPort] && devObjPtr->portsArr[egressPort].state == SKERNEL_PORT_STATE_MTI_USX_E && descrPtr->ptpPrecisionMode == SKERNEL_PTP_PRECISION_MODE_USX_E)
    {
        simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_PHY_EGRESS_E);

        /* EGRESS PHY builds USXGMII PCH && PHY update CF in PTP header */
        snetSip6_30PtpEgressPhyBuildUsxgmiiPch(devObjPtr,descrPtr,macPort,&pchInfo,frameDataPtr);
    }


    /* protect TX MAC MIB counters from simultaneous access with management. */
    SCIB_SEM_TAKE;

    snetChtTxMacCountUpdate(devObjPtr,descrPtr,macPort,frameDataPtr, FrameDataSize);

    if (SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* update MAC counter relate to PTP */
        snetLion3PtpUpdateCounters(devObjPtr, descrPtr, macPort);
    }


    SCIB_SEM_SIGNAL;


    /* check for EEE support */
    snetBobcat2EeeProcess(devObjPtr,macPort,SMAIN_DIRECTION_EGRESS_E);

    /* Calculate CRc */
    if(descrPtr->calcCrc)
    {
        __LOG(("Calculate CRc \n"));
        smainUpdateFrameFcs(devObjPtr, frameDataPtr, FrameDataSize);
    }

    slanPortNum = devObjPtr->supportExtPortMac ?
                egressPort : /* patch for Lion2 (and xCat3) that not use same slan for ports 9,12 and same slan for ports 11 and 14 */
                macPort;

    smainFrame2PortSendWithSrcPortGroup(
                devObjPtr,
                slanPortNum,
                frameDataPtr,
                FrameDataSize,
                portInLoopBack ? GT_TRUE : GT_FALSE,
                descrPtr->ingressDevObjPtr->portGroupId);

    return GT_TRUE;
}

/**
* @internal snetChtTx2Port function
* @endinternal
*
* @brief   Forward frame to target port
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in,out] descrPtr                 -    pointer to the frame's descriptor.
* @param[in] egressPort               -   number of egress port.
* @param[in] frameDataPtr             - pointer to frame data
*                                      frameDataSize - frame data size
*/
GT_BOOL snetChtTx2Port
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 egressPort,
    IN GT_U8 *frameDataPtr,
    IN GT_U32 FrameDataSize
)
{
    GT_BOOL value = internal_snetChtTx2Port(devObjPtr,descrPtr,
        egressPort,frameDataPtr,FrameDataSize);

    /* restore to 0 any value that was set inside internal_snetChtTx2Port(...) */
    smemSetCurrentTaskExtParamValue(devObjPtr,TASK_EXT_PARAM_INDEX_IS_PREEMPTIVE_CHANNEL,0);

    return value;
}

/**
* @internal snetChtTxMacCountUpdate function
* @endinternal
*
* @brief   Global update counters function (MAC MIB Counters + CPU port)
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] port                     - counter's port
*/
static void snetChtTxMacCountUpdate
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 port,
    IN GT_U8 * frameData,
    IN GT_U32  frameSize
)
{
  /* there is no dedicated CPU port MAC for SIP5 devices */
  if ((port == SNET_CHT_CPU_PORT_CNS) && !(devObjPtr->cpuPortNoSpecialMac))
  {
    snetChtCpuCountUpdate(devObjPtr);
  }
  else
  {
    snetChtMacMibCountUpdate(devObjPtr, descrPtr, port,frameData,frameSize);
  }
}

/**
* @internal snetChtCpuCountUpdate function
* @endinternal
*
* @brief   CPU Port Threshold and MIB Count Enable Registers
*
* @param[in] devObjPtr                - pointer to device object.
*                                      descrPtr   - pointer to the frame's descriptor.
*/
static void snetChtCpuCountUpdate
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32 regAddress;
    GT_U32 fldValue;

    regAddress = SMEM_CHT_CPU_PORT_GOOD_FRM_CNT_REG(devObjPtr);
    smemRegFldGet(devObjPtr, regAddress, 0, 32, &fldValue);
    /* GoodFrames ReceivedCnt */
    smemRegFldSet(devObjPtr, regAddress, 0, 32, ++fldValue);

    regAddress = SMEM_CHT_CPU_PORT_GOOD_OCT_CNT_REG(devObjPtr);
    smemRegFldGet(devObjPtr, regAddress, 0, 32, &fldValue);
    /* GoodOctets ReceivedCnt */
    smemRegFldSet(devObjPtr, regAddress, 0, 32, ++fldValue);
}


/**
* @internal snetChtMacMibCountUpdate function
* @endinternal
*
* @brief   Update Tx MAC counters
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - MIB counter's port
*/
static void snetChtMacMibCountUpdate
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 egressPort,
    IN GT_U8 * frameData,
    IN GT_U32  frameSize

)
{
    DECLARE_FUNC_NAME(snetChtMacMibCountUpdate);

    GT_U32 baseAddr;
    GT_U32 fldValue;
    SKERNEL_MAC_TYPE_ENT macDaType;/* mac DA type of the egress */
    GT_U8  *egressMacDaPtr;/*pointer to egress mac DA */
    GT_U32  *baseMibPtr;/*base memory of the MIB counters of the port*/
    SNET_CHT_PORT_MIB_COUNTERS_ENT mibCounter;
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_ENT mibCounter_100G;
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_ENT mtiCounterType;
    GT_BOOL isCgPort = CHT_IS_100G_MAC_USED_MAC(devObjPtr,egressPort);
    GT_BOOL isMtiPort = CHT_IS_MTI_MAC_USED_MAC(devObjPtr,egressPort);

    if(isMtiPort || isCgPort)
    {
        fldValue = 1;/* no such field .. consider as 'enable' counting */
        __LOG(("CG/MTI port MIB counters always allowed to be update for mac [%d] \n" , egressPort));
    }
    else
    {
        smemRegFldGet(devObjPtr, SMEM_CHT_MAC_CONTROL0_REG(devObjPtr, egressPort),
                    15, 1, &fldValue);

        if(IS_CHT_HYPER_GIGA_PORT(devObjPtr,egressPort))
        {
            /* field is 'MIBCntDis' */
            fldValue = 1 - fldValue;
        }
    }

    /* Disable MIB counters update for this port */
    if (fldValue == 0)
    {
        __LOG(("MIB counters are not allowed to be update for mac [%d] (update is disabled)\n" , egressPort));
        return;
    }

    if(!isMtiPort)
    {
        /* address of the start of counters for this port */
        baseAddr = CHT_MAC_PORTS_MIB_COUNTERS_BASE_ADDRESS_CNS(devObjPtr,egressPort);

        baseMibPtr = smemMemGet(devObjPtr,baseAddr);
    }
    else
    {
        baseMibPtr = NULL;
    }

    egressMacDaPtr = frameData;

    if (SGT_MAC_ADDR_IS_MCST(egressMacDaPtr))
    {
        if (SGT_MAC_ADDR_IS_BCST(egressMacDaPtr))
        {
            macDaType = SKERNEL_BROADCAST_MAC_E;
            __LOG(("egress macDaType = SKERNEL_BROADCAST_MAC_E \n"));
        }
        else
        {
            macDaType = SKERNEL_MULTICAST_MAC_E;
            __LOG(("egress macDaType = SKERNEL_MULTICAST_MAC_E \n"));
        }
    }
    else
    {
        macDaType = SKERNEL_UNICAST_MAC_E;
        __LOG(("egress macDaType = SKERNEL_UNICAST_MAC_E \n"));
    }

    mibCounter = SNET_CHT_PORT_MIB_COUNTERS____LAST____E;
    mibCounter_100G = SNET_CHT_PORT_MIB_COUNTERS_100G_PORT____LAST____E;
    mtiCounterType = SNET_CHT_PORT_MTI_STATISTIC_COUNTERS____LAST____E;
    switch (macDaType)
    {
        case SKERNEL_MULTICAST_MAC_E:
            mibCounter = SNET_CHT_PORT_MIB_COUNTERS_MULTICAST_FRAMES_SENT_E;
            mibCounter_100G = SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_ifOutMulticastPkts_E;
            mtiCounterType = SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_ifOutMulticastPkts_E;
            break;
        case SKERNEL_BROADCAST_MAC_E:
            mibCounter = SNET_CHT_PORT_MIB_COUNTERS_BROADCAST_FRAMES_SENT_E;
            mibCounter_100G = SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_ifOutBroadcastPkts_E;
            mtiCounterType = SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_ifOutBroadcastPkts_E;
            break;
        case SKERNEL_UNICAST_MAC_E:
            mibCounter = SNET_CHT_PORT_MIB_COUNTERS_UNICAST_FRAME_SENT_E;
            mibCounter_100G = SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_ifOutUcastPkts_E;
            mtiCounterType = SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_ifOutUcastPkts_E;
            break;

        default:
            /* can NOT get here ! */
            break;
    }

    if (baseMibPtr == NULL || CHT_IS_MTI_MAC_USED_MAC(devObjPtr,egressPort))
    {
        /* MC/BC/UC frame counter */
        snetFalconPortStatisticCounterUpdate(devObjPtr,egressPort,
                                             mtiCounterType, 1);
        /* Good octets sent */
        snetFalconPortStatisticCounterUpdate(devObjPtr,egressPort,
                SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_etherStatsOctets_E,
                frameSize);

        snetFalconPortStatisticCounterUpdate(devObjPtr,egressPort,
                SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_OctetsTransmittedOK_E,
                frameSize);

        snetFalconPortStatisticCounterUpdate(devObjPtr,egressPort,
                SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_aFramesTransmittedOK_E,
                1);

        snetFalconPortStatisticCounterUpdate(devObjPtr,egressPort,
                SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_etherStatsPkts_E,
                1);
    }
    else
    if(CHT_IS_100G_MAC_USED_MAC(devObjPtr,egressPort))
    {
        /* MC/BC/UC frame counter */
        snetChtPortMibOffsetUpdate_100GMac(devObjPtr,egressPort,baseMibPtr,
                mibCounter_100G,
                1);
        /* Good octets sent */
        snetChtPortMibOffsetUpdate_100GMac(devObjPtr,egressPort,baseMibPtr,
                SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_ifOutOctets_E,
                frameSize);
    }
    else
    {
        /* MC/BC/UC frame counter */
        snetChtPortMibOffsetUpdate(devObjPtr,egressPort,baseMibPtr,
                mibCounter,
                1);

        /* Good octets sent */
        snetChtPortMibOffsetUpdate(devObjPtr,egressPort,baseMibPtr,
                SNET_CHT_PORT_MIB_COUNTERS_GOOD_OCTETS_SENT_E,
                frameSize);
    }

    /* get Tx Histogram En */
    fldValue = snetChtPortMacFieldGet(devObjPtr, egressPort,
        SNET_CHT_PORT_MAC_FIELDS_histogram_tx_en_E);

    if (fldValue)
    {
        __LOG(("update Tx Histogram \n"));

        mibCounter = SNET_CHT_PORT_MIB_COUNTERS____LAST____E;
        mtiCounterType = SNET_CHT_PORT_MTI_STATISTIC_COUNTERS____LAST____E;

        switch (SNET_GET_NUM_OCTETS_IN_FRAME(frameSize))
        {
            case SNET_FRAMES_1024_TO_MAX_OCTETS:
                if(isCgPort)
                {
                    /* not supported */
                }
                else
                if(isMtiPort)
                {
                    mtiCounterType = (frameSize <= 1518) ?
                                      SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_etherStatsPkts1024to1518Octets_E :
                                      SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_etherStatsPkts1519toMaxOctets_E;
                }
                else
                {
                    mibCounter = snetChtMacCounter1024OrMoreGet(devObjPtr,
                                descrPtr,frameSize,egressPort);
                }
                break;
            case SNET_FRAMES_512_TO_1023_OCTETS:
                __LOG(("increment counter of: 512_TO_1023_OCTETS \n"));
                if(isCgPort)
                {
                    /* not supported */
                }
                else
                if(isMtiPort)
                {
                    mtiCounterType = SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_etherStatsPkts512to1023Octets_E;
                }
                else
                {
                    mibCounter = SNET_CHT_PORT_MIB_COUNTERS_FRAMES_512_TO_1023_OCTETS_E;
                }
                break;
            case SNET_FRAMES_256_TO_511_OCTETS:
                __LOG(("increment counter of: 256_TO_511_OCTETS \n"));
                if(isCgPort)
                {
                    /* not supported */
                }
                else
                if(isMtiPort)
                {
                    mtiCounterType = SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_etherStatsPkts256to511Octets_E;
                }
                else
                {
                    mibCounter = SNET_CHT_PORT_MIB_COUNTERS_FRAMES_256_TO_511_OCTETS_E;
                }
                break;
            case SNET_FRAMES_128_TO_255_OCTETS:
                __LOG(("increment counter of: 128_TO_255_OCTETS \n"));
                if(isCgPort)
                {
                    /* not supported */
                }
                else
                if(isMtiPort)
                {
                    mtiCounterType = SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_etherStatsPkts128to255Octets_E;
                }
                else
                {
                    mibCounter = SNET_CHT_PORT_MIB_COUNTERS_FRAMES_128_TO_255_OCTETS_E;
                }
                break;
            case SNET_FRAMES_65_TO_127_OCTETS:
                __LOG(("increment counter of: 65_TO_127_OCTETS \n"));
                if(isCgPort)
                {
                    /* not supported */
                }
                else
                if(isMtiPort)
                {
                    mtiCounterType = SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_etherStatsPkts65to127Octets_E;
                }
                else
                {
                    mibCounter = SNET_CHT_PORT_MIB_COUNTERS_FRAMES_65_TO_127_OCTETS_E;
                }
                break;
            case SNET_FRAMES_64_OCTETS:
                __LOG(("increment counter of: 64_OCTETS \n"));
                if(isCgPort)
                {
                    /* not supported */
                }
                else
                if(isMtiPort)
                {
                    mtiCounterType = SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_etherStatsPkts64Octets_E;
                }
                else
                {
                    mibCounter = SNET_CHT_PORT_MIB_COUNTERS_FRAMES_64_OCTETS_E;
                }
                break;
            default:
                break;
        }

        if (isMtiPort)
        {
            if (mtiCounterType != SNET_CHT_PORT_MTI_STATISTIC_COUNTERS____LAST____E)
            {
                snetFalconPortStatisticCounterUpdate(devObjPtr, egressPort,
                                                     mtiCounterType,
                                                     1);
            }
        }
        else
        {
            if (baseMibPtr != NULL && mibCounter != SNET_CHT_PORT_MIB_COUNTERS____LAST____E)
            {
                snetChtPortMibOffsetUpdate(devObjPtr,egressPort,baseMibPtr,
                        mibCounter,
                        1);
            }
        }
    }
    else
    {
        __LOG(("do not update Tx Histogram \n"));
    }
}

/**
* @internal snetChtTxQDpToCfi function
* @endinternal
*
* @brief   cfi value according to dp value
*         supported from ch3
*/
static GT_VOID snetChtTxQDpToCfi
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 targetPort
)

{
    DECLARE_FUNC_NAME(snetChtTxQDpToCfi);

    GT_U32 dpToCfiEn=0;    /* Port <n> DP To CFI Enable */
    GT_U32 dpToCfi1En=0;   /* Port <n> DP To CFI1 Enable */
    GT_U32 regAddr;        /* Register address */
    GT_U32 * regPtr;       /* Register pointer */
    SKERNEL_PORTS_BMP_STC egressPortsBmp;   /* Egress ports bitmap */

    if (! SKERNEL_IS_CHEETAH3_DEV(devObjPtr))
    {
        return;
    }

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* Logic of DP-To-Cfi moved to HA (per eport) and ERMRK (that do the remark) on outer tag */
        return;
    }

    regAddr = SMEM_CHT3_DP2CFI_ENABLE_REG(devObjPtr);
    regPtr = smemMemGet(devObjPtr, regAddr);

    SKERNEL_FILL_PORTS_BITMAP_MAC(devObjPtr, &egressPortsBmp, regPtr);

    /* Port <n> DP To CFI Enable */
    dpToCfiEn = SKERNEL_PORTS_BMP_IS_PORT_SET_MAC(&egressPortsBmp, targetPort);

    if (dpToCfiEn)
    {
        regAddr = SMEM_CHT3_DP2CFI_REG(devObjPtr);
        regPtr = smemMemGet(devObjPtr, regAddr);

        /* DP To CFI Value  */
        descrPtr->cfidei =
            SNET_CHT_DP_TO_CFI_VALUE_GET_MAC(devObjPtr, regPtr, descrPtr->dp);
        __LOG(("Mapped DP[%d] To CFI [%d]",
            descrPtr->dp,descrPtr->cfidei));
    }

    if(devObjPtr->support802_1br_PortExtender &&
       descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_FORWARD_E)
    {
        regAddr = SMEM_XCAT3_EGRESS_AND_TXQ_DP2CFI1_ENABLE_REG(devObjPtr);
        regPtr = smemMemGet(devObjPtr, regAddr);
        SKERNEL_FILL_PORTS_BITMAP_MAC(devObjPtr, &egressPortsBmp, regPtr);
        /* Port <n> DP To CFI Enable */
        dpToCfi1En = SKERNEL_PORTS_BMP_IS_PORT_SET_MAC(&egressPortsBmp, targetPort);

        if(dpToCfi1En)
        {
            regAddr = SMEM_CHT3_DP2CFI_REG(devObjPtr);
            regPtr = smemMemGet(devObjPtr, regAddr);

            /* Desc<CFI1> = DP2CFI mapping table*/
            descrPtr->cfidei1 =
                SNET_CHT_DP_TO_CFI_VALUE_GET_MAC(devObjPtr, regPtr, descrPtr->dp);
            __LOG(("Mapped DP[%d] To CFI1 [%d]",
                descrPtr->dp,descrPtr->cfidei1));
        }
    }
}


/**
* @internal snetChtEgfQagVidAssignment function
* @endinternal
*
* @brief   the TXQ per port (EPort) Assignment of Vid
*         relevant for EArch devices
*/
void snetChtEgfQagVidAssignment
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32               egressPort
)
{
    DECLARE_FUNC_NAME(snetChtEgfQagVidAssignment);

    GT_U32 fldVal;/* field value */
    GT_U32  evidNumBits;
    GT_U32  startBitAfterEvid;

    if (devObjPtr->supportEArch == 0 || devObjPtr->unitEArchEnable.txq == 0)
    {
        return;
    }

    snetChtEgfEgressEPortEntryGet(devObjPtr,descrPtr,egressPort,GT_FALSE);

    if(descrPtr->eArchExtInfo.egfQagEgressEPortTablePtr == NULL)
    {
        __LOG(("no EPort accessing \n"));
        return;
    }

    if(SMEM_CHT_IS_SIP5_10_GET(devObjPtr) == 0)
    {
        evidNumBits = 16;
    }
    else
    {
        evidNumBits = 13;
    }

    startBitAfterEvid = 12 + evidNumBits;

    /*eVLAN Command*/
    fldVal = snetFieldValueGet(descrPtr->eArchExtInfo.egfQagEgressEPortTablePtr, 8 , 1);
    if(fldVal)
    {
        /* Assign new eVLAN to the egress packet */
        __LOG(("Assign new eVLAN to the egress packet"));
        descrPtr->eVid =
            snetFieldValueGet(descrPtr->eArchExtInfo.egfQagEgressEPortTablePtr, 12 , evidNumBits);
    }

    /*VID1 Command*/
    fldVal = snetFieldValueGet(descrPtr->eArchExtInfo.egfQagEgressEPortTablePtr, 9 , 1);
    if(fldVal)
    {
        __LOG(("Assign new Vid1 to the egress packet"));
        descrPtr->vid1 =
            snetFieldValueGet(descrPtr->eArchExtInfo.egfQagEgressEPortTablePtr, startBitAfterEvid , 12);
    }

    /*UP0 Command*/
    fldVal = snetFieldValueGet(descrPtr->eArchExtInfo.egfQagEgressEPortTablePtr, 10 , 1);
    if(fldVal)
    {
        __LOG(("Assign new UP0 to the egress packet"));
        descrPtr->up =
            snetFieldValueGet(descrPtr->eArchExtInfo.egfQagEgressEPortTablePtr, (startBitAfterEvid+12) , 3);
    }

    /*UP1 Command*/
    fldVal = snetFieldValueGet(descrPtr->eArchExtInfo.egfQagEgressEPortTablePtr, 11 , 1);
    if(fldVal)
    {
        __LOG(("Assign new UP1 to the egress packet"));
        descrPtr->up1 =
            snetFieldValueGet(descrPtr->eArchExtInfo.egfQagEgressEPortTablePtr, (startBitAfterEvid+12+3) , 3);
    }

    return;
}

/******************************************/
/**  ------  Epcl entry point  --------  **/
/******************************************/

/**
* @internal snetChtEpclProcess function
* @endinternal
*
* @brief   EPCL - supported from ch2
*         egress PCL
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - local egress port (not global).
*
* @retval GT_OK                    - it is ok to transmit the packet
* @retval otherwise                - the packet was dropped , and should not be transmitted
*/
static void snetChtEpclProcess
(
  IN SKERNEL_DEVICE_OBJECT * devObjPtr,
  IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
  IN GT_U32   egressPort
)
{
    DECLARE_FUNC_NAME(snetChtEpclProcess);

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_EPCL_E);

    if(SKERNEL_IS_CHEETAH1_ONLY_DEV(devObjPtr))
    {
        /* no EPCL support */
        __LOG(("no EPCL support \n"));
        return;
    }

    if(SKERNEL_IS_XCAT_REVISON_A1_DEV(devObjPtr))
    {
        snetXCatEPcl(devObjPtr, descrPtr, egressPort);
    }
    else if (SKERNEL_IS_CHEETAH3_DEV(devObjPtr))
    {
        snetCht3EPcl(devObjPtr, descrPtr, egressPort);
    }
    else
    {
        snetCht2EPcl(devObjPtr, descrPtr, egressPort);
    }
}


/******************************************/
/**  ------  Eplr entry point  --------  **/
/******************************************/


/**
* @internal snetChtEplrProcess function
* @endinternal
*
* @brief   EPLR - supported from xcat-A1
*         egress Policer
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - local egress port (not global).
*/
static void snetChtEplrProcess
(
  IN SKERNEL_DEVICE_OBJECT * devObjPtr,
  IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
  IN GT_U32   egressPort
)
{
    DECLARE_FUNC_NAME(snetChtEplrProcess);

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_EPLR_E);

    if(!SKERNEL_IS_XCAT_REVISON_A1_DEV(devObjPtr))
    {
        /* no EPLR support */
        __LOG(("no EPLR support \n"));
        return ;
    }

    snetXCatEgressPolicer(devObjPtr,descrPtr,egressPort);
    /* reset the field as it hold no meaning outside the POLICER and it makes 'diff' for the LOGGER */
    descrPtr->policerCycle = 0;
}
/********************************************/
/**  ------  common Egress logic --------  **/
/********************************************/


/**
* @internal snetChtSdmaGenInterrupt function
* @endinternal
*
* @brief   Generate interrupt for SDMA buffer .
*
* @param[in] devObjPtr                - pointer to device.
* @param[in] rxDesc                   - pointer to descriptor.
* @param[in] tc                       - Rx descriptor to fill.
* @param[in] notErrBit                - GT_FALSE - RX resource error
*                                      GT_TRUE  - NO resource error
*                                       GT_OK if packet transmitted successfully,
*                                       GT_FAIL on resource error.
*/
static void snetChtSdmaGenInterrupt
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SNET_CHT_RX_DESC *rxDesc,
    IN GT_U32 tc,
    IN GT_BOOL  notErrBit
)
{
    DECLARE_FUNC_NAME(snetChtSdmaGenInterrupt);

    GT_U32 firstBit ; /* bit of interrupt in the register */
    GT_U32 doInterrupt;/* do we need to interrupt */

    if(notErrBit == GT_FALSE)
    {
        /* on error we set the interrupt */
        doInterrupt = 1;
        firstBit = 11  + tc;

        /* on this case the rxDesc is not relevant because it is owned by CPU
         or it is NULL */
    }
    else
    if (rxDesc == NULL)
    {
        return;
    }
    else
    {
        /* get interrupt enable bit */
        doInterrupt = SNET_CHT_RX_DESC_GET_EI_FIELD(rxDesc);
        firstBit = 2 + tc;
    }

    if (doInterrupt == 0)
    {
        __LOG(("The descriptor for packets to CPU stated 'no send interrupt' to CPU ('EI' (enable interrupt) bit 29 word 1) \n"));
        return;
    }

    snetChetahDoInterrupt(devObjPtr,
                           SMEM_CHT_SDMA_INT_CAUSE_REG(devObjPtr),
                           SMEM_CHT_SDMA_INT_MASK_REG(devObjPtr),
                           (1 << firstBit),
                           SMEM_CHT_RX_SDMA_SUM_INT(devObjPtr));
}

/**
* @internal snetChtSendPacketToCpuSdma function
* @endinternal
*
* @brief   Send the packet to the SDMA of the CPU
*
* @param[in] devObjPtr                - pointer to device.
* @param[in] startDmaAddr             - start DMA address to work with
* @param[in] tc                       - traffic class
* @param[in] dataPtr                  - pointer to data
* @param[in] dataSize                 - size of data (dataPtr)
*                                       void
*/
static void snetChtSendPacketToCpuSdma
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  startDmaAddr,
    IN GT_U32                  tc,
    IN GT_U8                 * dataPtr,
    IN GT_U32                  dataSize
)
{
    DECLARE_FUNC_NAME(snetChtSendPacketToCpuSdma);

    GT_U32  buffLen = 0;                /* Length of the current buffer */
    GT_U32  sentBytes = 0;              /* Number of bytes already sent */
    GT_U32  regAddress;                 /* Register address */
    GT_U32  fldValue;                   /* Register field value */
    SNET_CHT_RX_DESC firstDescrInfo;    /* First RX descriptor */
    SNET_CHT_RX_DESC curDescrInfo;      /* RX descriptor */
    SNET_CHT_RX_DESC * rxDesc;          /* Pointer to RX descriptor */
    GT_U32  currentDmaAddr;             /* Current RX descriptor address */
    SNET_CHT_RX_DESC * prevDescPtr = NULL;     /* Pointer to previous descriptor */
    GT_U32      resourceErrorOnAbortMode = 0; /* resource error on 'Abort mode' */
    GT_U32      firstResourceError = 1;/* the first time of resource error */
    GT_U32      abortMode;/* abort mode : 0 - retry , 1 - abort */
    GT_U32      errCount;
    GT_U32      offset;
    static      GT_U32 droppedCounter = 0;

    /* Start from first descriptor address */
    currentDmaAddr = startDmaAddr;
    rxDesc = &firstDescrInfo;

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr) ||
       SKERNEL_IS_XCAT3_BASED_DEV(devObjPtr))
    {
        regAddress = SMEM_CHT_RES_ERR_COUNT_AND_MODE_REG(devObjPtr, tc);
        smemRegGet(devObjPtr, regAddress , &fldValue );

        abortMode = (SMEM_U32_GET_FIELD(fldValue, 8, 1) || /*<Wait AbortTC> = 1 'abort' , 0 'wait'*/
                     SMEM_U32_GET_FIELD(fldValue, 9, 1) == 0) ? /*<RetryTC> = 0 'No_Retry' , 1 'Retry' */
                     1 : 0;
    }
    else
    {
        regAddress = SMEM_CHT_SDMA_CONFIG_REG(devObjPtr);
        smemRegGet(devObjPtr, regAddress , &fldValue );

        abortMode = SMEM_U32_GET_FIELD(fldValue, 8 + tc , 1);

    }

    /* Fill descriptors if there is a data to send */
    while (1)
    {
        /* Read the DMA for needed value, put data into current descriptor */
        __LOG(("Read the DMA for needed value, put data into current descriptor"));
        snetChtPerformScibDmaRead(SNET_CHT_DMA_CLIENT_PACKET_TO_CPU_E,
                    devObjPtr->deviceId,currentDmaAddr,
                    NUM_BYTES_TO_WORDS(sizeof(SNET_CHT_RX_DESC)),
                    (GT_U32 *)rxDesc, SCIB_DMA_WORDS);

        /* Resource error - SNET_CHT_RX_DESC_GET_OWN_BIT buffers left  */
        if(currentDmaAddr == 0 ||
           SNET_CHT_RX_DESC_GET_OWN_BIT(rxDesc) == SNET_CHT_RX_DESC_CPU_OWN)
        {
            if(devObjPtr->needToDoSoftReset)
            {
                /* no need to handle errors */
                return;
            }

            if(firstResourceError == 1)
            {
                /* Resource error interrupt */
                __LOG(("Resource error interrupt"));
                snetChtSdmaGenInterrupt(devObjPtr, rxDesc, tc, GT_FALSE);

                 /* If the resource error occurred upon a real packet
                (and not upon a speculative pre-fetch), A 8-bit counter is
                incremented (per queue) to count the number of resource error events */
                if(SMEM_CHT_IS_SIP5_GET(devObjPtr) ||
                   SKERNEL_IS_XCAT3_BASED_DEV(devObjPtr))
                {
                    regAddress = SMEM_CHT_RES_ERR_COUNT_AND_MODE_REG(devObjPtr, tc);
                                        offset = 0;
                }
                else
                {
                    regAddress = SMEM_CHT_RES_ERR_COUNT_REG(devObjPtr, tc);
                                        offset = (tc%4)*8;
                }
                smemRegGet(devObjPtr, regAddress , &fldValue );
                errCount = SMEM_U32_GET_FIELD(fldValue, offset, 8);
                errCount++;
                smemRegFldSet(devObjPtr , regAddress , offset, 8 , errCount);
                /* indicate the interrupt was sent to the CPU to indicate the Resource error */
                /* Don't send more than single interrupt about this packet */
                firstResourceError = 0;
             }

            /*<Retry AbortTC 0..7>*/
            if(0 == abortMode)
            {/*retry*/
                /* let the Application time to process the packets , so the list will be free */
                SIM_OS_MAC(simOsSleep)(1);



                continue;
            }

            if(resourceErrorOnAbortMode < 2)
            {
                resourceErrorOnAbortMode ++ ;

#ifdef LINUX
                /* try to reduce number of fails in Linux simulation */
                /* it is abort mode ... but let it do one more try ... */
                SIM_OS_MAC(simOsSleep)(10);
                continue;
#endif /*defined ASIC_SIMULATION && defined LINUX*/
            }

            resourceErrorOnAbortMode += 2;

            {
                droppedCounter++;
                simWarningPrintf("snetChtSendPacketToCpuSdma : abort mode , packet #%d dropped \n",droppedCounter);
            }

            if(prevDescPtr == NULL)
            {
                /* in simulation we not do 'prefetch' so we recognize
                   resource error only when packet to CPU needed.*/
                /* this is the first descriptor that is already lack of resource
                so no need to update any other descriptor --> exit the function */

                return;
            }

            rxDesc = prevDescPtr;

            break ;/*we need to update the first,last descriptors */
        }
        firstResourceError = 1;/* allow another resource error on another descriptor */
        /* Buffer size in quantities of 8 bytes */
        buffLen = SNET_CHT_RX_DESC_GET_BUFF_SIZE_FIELD(rxDesc);

        /* Trim buffer size if data size less than descriptor buffer size */
        buffLen = (buffLen > (dataSize - sentBytes)) ?
                            (dataSize - sentBytes) : buffLen;

        /* Write data into the DMA buffer */
        snetChtPerformScibDmaWrite(SNET_CHT_DMA_CLIENT_PACKET_TO_CPU_E,
                    devObjPtr->deviceId,rxDesc->buffPointer,
                     NUM_BYTES_TO_WORDS(buffLen),
                     (GT_U32*)&(dataPtr[sentBytes]), TX_BYTE_ORDER_MAC(devObjPtr));

        /* the handling of the 'own bit' for the first descriptor must be done
           only after the entire descriptors are handled */
        if (rxDesc != &firstDescrInfo)
        {
            /* CPU is owned the buffer */
            SNET_CHT_RX_DESC_SET_OWN_BIT(rxDesc, SNET_CHT_RX_DESC_CPU_OWN);
            /* Write back to DMA the changes in the descriptor(rxDesc->word1 */
            snetChtPerformScibDmaWrite(SNET_CHT_DMA_CLIENT_PACKET_TO_CPU_E,
                devObjPtr->deviceId,currentDmaAddr, 1, (GT_U32 *)rxDesc, SCIB_DMA_WORDS);
            /* Rx SDMA Buffer Write Done Interrupt */
            snetChtSdmaGenInterrupt(devObjPtr, rxDesc, tc, GT_TRUE);
        }

        /* Increment send bytes counter */
        sentBytes += buffLen;
        if (sentBytes == dataSize)
        {
            /* we are done with this frame bytes */
            __LOG(("we are done with this frame bytes"));
            break;
        }
        else if(sentBytes > dataSize)
        {
            /* should not happen */
            skernelFatalError(" snetChtSendPacketToCpuSdma: bad management on byte send \n");
        }

        prevDescPtr = rxDesc;

        /* Move to the next descriptor */
        currentDmaAddr = rxDesc->nextDescPointer;
        rxDesc = &curDescrInfo;
    }

    if(resourceErrorOnAbortMode <= 2)
    {
        /* the resource error on abort mode was 'ignored' because we did 'retry' for one time */
        /* so treat it as if 'no error' */
        resourceErrorOnAbortMode = 0;
    }


    if(resourceErrorOnAbortMode)
    {
        /* Set the Resource Error for the first descriptor */
        __LOG(("Set the Resource Error for the first descriptor \n"));
        SMEM_U32_SET_FIELD(firstDescrInfo.word1,28,1,1);
    }

    /* Set the last bit for the current descriptor */
    SNET_CHT_RX_DESC_SET_LAST_BIT(rxDesc, 1);
    if(rxDesc != &firstDescrInfo)
    {
        /* Write back to DMA the changes in the descriptor(rxDesc->word1, rxDesc->word2)*/
        __LOG(("Write back to DMA the changes in the descriptor(rxDesc->word1, rxDesc->word2) \n"));
        snetChtPerformScibDmaWrite(SNET_CHT_DMA_CLIENT_PACKET_TO_CPU_E,
            devObjPtr->deviceId,currentDmaAddr, 1, (GT_U32 *)rxDesc, SCIB_DMA_WORDS);
    }

    if(resourceErrorOnAbortMode == 0)
    {
        /* Update the next descriptor to handle */
        regAddress = SMEM_CHT_CUR_RX_DESC_POINTER_REG(devObjPtr, tc);
        smemRegSet(devObjPtr, regAddress, rxDesc->nextDescPointer);
    }

    /* Restore descriptor info for the first descriptor */
    currentDmaAddr = startDmaAddr;
    rxDesc = &firstDescrInfo;

    /* Set byte count in buffer */
    __LOG(("Set byte count [%d] in buffer \n" , dataSize));
    SNET_CHT_RX_DESC_SET_BYTE_COUNT_FIELD(rxDesc, dataSize);

    /* the last bit must be set in different transaction even when the last and
    first descriptors are the same one.
    because the info needed in the first descriptor also has the 'byte count'
    but it is on different word (rxDesc->word2) then the first and last bits
    (rxDesc->word1) */

    /* Write back to DMA the changes in the descriptor(rxDesc->word1, rxDesc->word2)*/
    /* this update the 'byte count' and maybe the 'last bit' */
    snetChtPerformScibDmaWrite(SNET_CHT_DMA_CLIENT_PACKET_TO_CPU_E,
        devObjPtr->deviceId,currentDmaAddr, 2, (GT_U32 *)rxDesc, SCIB_DMA_WORDS);


    /* the next action must be the last one as it release the entire frame , so
       CPU can start handling */

    if(resourceErrorOnAbortMode == 0)
    {
        /* CPU is owned the buffer */
        __LOG(("set CPU to owned the buffer \n"));
        SNET_CHT_RX_DESC_SET_OWN_BIT(rxDesc, SNET_CHT_RX_DESC_CPU_OWN);
    }
    /* Set the first bit for the current descriptor */
    SNET_CHT_RX_DESC_SET_FIRST_BIT(rxDesc, 1);

    __LOG(("Write back to DMA the changes in the descriptor \n"));
    /* Write back to DMA the changes in the descriptor(rxDesc->word1)*/
    snetChtPerformScibDmaWrite(SNET_CHT_DMA_CLIENT_PACKET_TO_CPU_E,
        devObjPtr->deviceId,currentDmaAddr, 1, (GT_U32 *)rxDesc, SCIB_DMA_WORDS);

    if(resourceErrorOnAbortMode == 0)
    {
        /* Upon every packet transmission, interrupt is invoked */
        __LOG(("invoke interrupt about 'Packet to CPU' \n"));
        snetChtSdmaGenInterrupt(devObjPtr, rxDesc, tc, GT_TRUE);

        /* Set the byte count */
        regAddress = SMEM_CHT_BYTE_COUNT_REG(devObjPtr, tc);
        smemRegGet(devObjPtr, regAddress, &fldValue);
        __LOG(("SDMA queue[%d] : Increment SDMA byte count counter by[%d] from[%d] \n",
            tc,dataSize,fldValue));
        fldValue += dataSize;
        smemRegSet(devObjPtr, regAddress, fldValue);

        /* Set the packet count */
        regAddress = SMEM_CHT_PACKET_COUNT_REG(devObjPtr, tc);
        smemRegGet(devObjPtr, regAddress, &fldValue);
        __LOG(("SDMA queue[%d] : Increment SDMA packet count counter by 1 from[%d] \n",
            tc,fldValue));
        fldValue++;
        smemRegSet(devObjPtr, regAddress, fldValue);
    }
    else
    {
        __LOG(("no interrupt and not update counters due to 'Resource error on abort mode' \n"));
    }

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT___ALLOW_ALL_UNITS___E);/* wild card for LOG without the unit filter */

    if(sentBytes != dataSize)
    {
        __LOG(("Final Egress packet (to CPU DMA) ONLY [%d] bytes out of[%d] bytes \n",
            sentBytes,dataSize));
    }
    else
    {
        __LOG(("Final Egress packet (to CPU DMA) [%d] bytes \n",
            sentBytes));
    }
    /* dump to log the packet that egress the port*/
    simLogPacketDump(devObjPtr,
        GT_FALSE ,
        smemMultiDpGlobalCpuSdmaNumFromCurrentMgGet(devObjPtr),
        dataPtr,
        sentBytes);

    return;
}

/**
* @internal snetChtDma2Cpu function
* @endinternal
*
* @brief   Send frame to CPU by DMA
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] frameData                - pointer to the data
* @param[in] frameSize                - data size
*/
void snetChtDma2Cpu
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U8        *          frameData,
    IN GT_U32                  frameSize
)
{
    DECLARE_FUNC_NAME(snetChtDma2Cpu);

/*   SNET_CHT_RX_DESC * curDescrPtr;*/
    GT_U32 regValue, regAddr, RxDisQ;

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_TXDMA_E);/*consider as TXDMA of the CPU port */

    __LOG(("SDMA to CPU : on queue[%d] started \n",
        descrPtr->tc));

    smemRegFldGet(devObjPtr,SMEM_CHT_RX_QUEUE_EN_REG(devObjPtr), descrPtr->tc + 8,1,&RxDisQ);
    if (!RxDisQ)
    {
        regAddr = SMEM_CHT_CUR_RX_DESC_POINTER_REG(devObjPtr, descrPtr->tc);
        smemRegGet(devObjPtr, regAddr,  &regValue);
/*     curDescrPtr = (SNET_CHT_RX_DESC *)regValue;*/

        if (devObjPtr->crcBytesAdd)
        {
            /* Add 4 bytes to packet header for CRC (used in RTG/RDE tests) */
            __LOG(("SDMA to CPU : Add 4 bytes to packet header for CRC \n"));
            frameSize += 4;
        }
         /* Set constant CRC in Cheetah family devices */
        __LOG(("SDMA to CPU : Set constant CRC 0x55555555 (NOT recalc actual CRC) \n"));
        frameData[frameSize-4] = 0x55;
        frameData[frameSize-3] = 0x55;
        frameData[frameSize-2] = 0x55;
        frameData[frameSize-1] = 0x55;

        snetChtSendPacketToCpuSdma(devObjPtr, regValue, descrPtr->tc ,
                                   frameData, frameSize);
    }
    else
    {
        __LOG(("SDMA to CPU : queue[%d] is disabled (so not send packet to CPU) \n",
            descrPtr->tc));
    }

    __LOG(("SDMA to CPU : on queue[%d] Ended \n",
        descrPtr->tc));

}

/**
* @internal snetlion3PtpUdpChecksumCalc function
* @endinternal
*
* @brief   UDP Checksum calculation
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*                                      OUTPUT:
*                                      None
*/
static GT_VOID snetlion3PtpUdpChecksumCalc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetlion3PtpUdpChecksumCalc);
    GT_U8 *ipHeaderPtr;
    GT_U8 *udpHeaderPtr;
    GT_U16 udpLength;
    GT_U16 udpChecksum;
    GT_U8  ipPseudoHeader[42];
    GT_U16 ipPseudoHeaderLength;

    if( descrPtr->ptpUdpChecksumMode == SNET_PTP_UDP_CHECKSUM_MODE_NOP_E )
    {
        __LOG(("NOP UDP checksum field for PTPoUDP"));
        return;
    }

    if(NULL == descrPtr->l4StartOffsetPtr)
    {
        __LOG(("L4 Header Not Valid"));
        return;
    }

    udpHeaderPtr = descrPtr->haToEpclInfo.l3StartOffsetPtr +
                        (descrPtr->l4StartOffsetPtr - descrPtr->payloadPtr);

    /* clear UDP checksum field */
    udpHeaderPtr[6] = 0x0;
    udpHeaderPtr[7] = 0x0;

    if (descrPtr->ptpUdpChecksumMode == SNET_PTP_UDP_CHECKSUM_MODE_CLEAR_E )
    {
        __LOG(("Clear UDP checksum field for PTPoUDP"));
        return;
    }

    /* descrPtr->ptpUdpChecksumMode == SNET_PTP_UDP_CHECKSUM_MODE_UPDATE_E */
    __LOG(("Update UDP checksum field for PTPoUDP"));

    /* Calculate checksum of UDP part only - checksum field cleared and lengt5h derived from header */
    /* the not operation to cancel the final one complementry arithmetics */
    udpLength = (udpHeaderPtr[4]<<8) + udpHeaderPtr[5];
    udpChecksum = ~checkSumCalc(udpHeaderPtr, udpLength);

    ipHeaderPtr = descrPtr->haToEpclInfo.l3StartOffsetPtr;

    if( (ipHeaderPtr[0] & 0xF0) == 0x40 )
    {
        /* IPv4 pseudo header */
        /* 0      7 8     15 16    23 24    31
           +--------+--------+--------+--------+
           |          source address           |
           +--------+--------+--------+--------+
           |        destination address        |
           +--------+--------+--------+--------+
           |  zero  |protocol|   UDP length    |
           +--------+--------+--------+--------+
        */

        ipPseudoHeaderLength = 12;

        memcpy(&ipPseudoHeader[0],&ipHeaderPtr[12],8);
        ipPseudoHeader[8] = 0x0;
        ipPseudoHeader[9] = 17;
        ipPseudoHeader[10] = udpHeaderPtr[4];
        ipPseudoHeader[11] = udpHeaderPtr[5];
    }
    else if( (ipHeaderPtr[0] & 0xF0) == 0x60 )
    {
        /* IPv6 pseudo header */
        /* 0      7 8     15 16    23 24    31
           +--------+--------+--------+--------+
           |                                   |
           +                                   +
           |                                   |
           +         source address            +
           |                                   |
           +                                   +
           |                                   |
           +--------+--------+--------+--------+
           |                                   |
           +                                   +
           |                                   |
           +       destination address         +
           |                                   |
           +                                   +
           |                                   |
           +--------+--------+--------+--------+
           |             UDP length            |
           +--------+--------+--------+--------+
           |           zero           |Next Hd.|
           +--------+--------+--------+--------+
        */

        ipPseudoHeaderLength = 40;

        memcpy(&ipPseudoHeader[0],&ipHeaderPtr[8],32);
        ipPseudoHeader[32] = 0x0;
        ipPseudoHeader[33] = 0x0;
        ipPseudoHeader[34] = udpHeaderPtr[4];
        ipPseudoHeader[35] = udpHeaderPtr[5];
        ipPseudoHeader[36] = 0x0;
        ipPseudoHeader[37] = 0x0;
        ipPseudoHeader[38] = 0x0;
        ipPseudoHeader[39] = 17;
    }
    else
    {
        __LOG(("Unrecognized IPvX version - not IPv4 or IPv6"));
        return;
    }

    /* append the previously calculated UDP "only" checksum */
    ipPseudoHeader[ipPseudoHeaderLength] = (udpChecksum & 0xFF00)>>8;
    ipPseudoHeader[ipPseudoHeaderLength+1] = (udpChecksum & 0xFF);

    udpChecksum = checkSumCalc(ipPseudoHeader, (GT_U16)(ipPseudoHeaderLength+2));

    if( udpChecksum == 0x0 )
    {
        udpChecksum = 0xFFFF; /* ~0x0 */
    }

    /* update UDP checksum field */
    udpHeaderPtr[6] = (udpChecksum & 0xFF00)>>8;
    udpHeaderPtr[7] = (udpChecksum & 0xFF);
}

/**
* @internal snetlion3PtpUdpChecksum function
* @endinternal
*
* @brief   UDP Checksum mode and value setting
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*                                      OUTPUT:
*                                      None
*/
static GT_VOID snetlion3PtpUdpChecksum
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetlion3PtpUdpChecksum);

    GT_U32 regAddr;             /* Register address */
    GT_U32 *regPtr;             /* Register pointer */
    GT_U32 startBit;

    startBit = 0xFFFFFFFF;

    switch(descrPtr->timestampActionInfo.packetFormat)
    {
        case SKERNEL_TIMESTAMP_PACKET_FORMAT_PTPV2_E:
        case SKERNEL_TIMESTAMP_PACKET_FORMAT_PTPV1_E:
            if( descrPtr->ptpTriggerType == SKERNEL_PTP_TRIGGER_TYPE_PTP_OVER_IPV4_UDP_E )
            {
                startBit = 0;
            }
            else if (descrPtr->ptpTriggerType == SKERNEL_PTP_TRIGGER_TYPE_PTP_OVER_IPV6_UDP_E)
            {
                startBit = 2;
            }
            break;
        case SKERNEL_TIMESTAMP_PACKET_FORMAT_NTPR_E:
        case SKERNEL_TIMESTAMP_PACKET_FORMAT_NTPT_E:
            if( descrPtr->ptpTriggerType == SKERNEL_PTP_TRIGGER_TYPE_PTP_OVER_IPV4_UDP_E )
            {
                startBit = 4;
            }
            else if (descrPtr->ptpTriggerType == SKERNEL_PTP_TRIGGER_TYPE_PTP_OVER_IPV6_UDP_E)
            {
                startBit = 6;
            }
            break;
        case SKERNEL_TIMESTAMP_PACKET_FORMAT_WAMP_E:
            if( descrPtr->ptpTriggerType == SKERNEL_PTP_TRIGGER_TYPE_PTP_OVER_IPV4_UDP_E )
            {
                startBit = 8;
            }
            else if (descrPtr->ptpTriggerType == SKERNEL_PTP_TRIGGER_TYPE_PTP_OVER_IPV6_UDP_E)
            {
                startBit = 10;
            }
            break;
        default:
            break;
    }

    if( startBit == 0xFFFFFFFF )
    {
        __LOG(("PTP type - over Ethernet or over UDP - was not triggered"));
        descrPtr->ptpUdpChecksumMode = SNET_PTP_UDP_CHECKSUM_MODE_NOP_E;
    }
    else
    {
        __LOG(("PTP type - over Ethernet or over UDP - was triggered"));
        regAddr = SMEM_LION3_ERMRK_TS_UDP_CHECKSUM_MODE_REG(devObjPtr);
        regPtr = smemMemGet(devObjPtr, regAddr);
        descrPtr->ptpUdpChecksumMode = SMEM_U32_GET_FIELD(*regPtr, startBit, 2);
    }

    snetlion3PtpUdpChecksumCalc(devObjPtr, descrPtr);
}


/**
* @internal snetlion3PtpPiggybackApply function
* @endinternal
*
* @brief   Update timestamp into 4B <reserved> field the PTP
*          header.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - target port.
*
*/
static GT_VOID snetlion3PtpPiggybackApply
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 egressPort
)
{
    DECLARE_FUNC_NAME(snetlion3PtpPiggybackApply);

    GT_U8  *ptpStart;           /* pointer to start of timestamp related packet */
    GT_U32 regAddr;             /* Register address */
    GT_U32 *regPtr;             /* Register pointer */
    GT_BOOL set_T_U_bits;       /* indication whether to add T,U as 2LSBits to timestamp nano second */
    GT_U32  nanoSecondTimer;
    GT_BIT  isCpuPort;

    if(descrPtr->isCpuUseSdma)/*general indication that not mean that current descriptor is to the SDMA of CPU */
    {
        /*check DMA number for CPU port*/
        if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {
            /*check DMA number for CPU port*/
            isCpuPort = smemMultiDpUnitIsCpuSdmaPortGet(devObjPtr,descrPtr->egressPhysicalPortInfo.txDmaMacPort,NULL/*mgUnitIndex*/);
        }
        else/*sip5*/
        {
            GT_U32 dmaNumOfCpuPort = devObjPtr->dmaNumOfCpuPort ? devObjPtr->dmaNumOfCpuPort : SNET_CHT_CPU_PORT_CNS;
            isCpuPort = (descrPtr->egressPhysicalPortInfo.txDmaMacPort == dmaNumOfCpuPort) ? GT_TRUE : GT_FALSE;
        }

        if(isCpuPort)
        {
            __LOG(("ingress Timestamp not relevant to CPU SDMA port (only to physical ports)\n"));
            return;
        }
    }

    if ((descrPtr->timestampTagged[SMAIN_DIRECTION_EGRESS_E] == SKERNEL_TIMESTAMP_TAG_TYPE_PIGGY_BACKED_E) &&
        (descrPtr->timestampActionInfo.packetFormat == SKERNEL_TIMESTAMP_PACKET_FORMAT_PTPV2_E))
    {
        __LOG(("Ingress Timestamp was piggybacked onto the 1588v2 packet, using the 4B <reserved>. field in the PTP header.\n"));
        /* PTP timestamping based on Local Action Table */
        ptpStart = descrPtr->haToEpclInfo.l3StartOffsetPtr +
                   (descrPtr->ptpGtsInfo.ptpMessageHeaderPtr - descrPtr->payloadPtr);

        ptpStart += 16; /* reserved field */
        nanoSecondTimer = descrPtr->timestamp[SMAIN_DIRECTION_INGRESS_E].nanoSecondTimer;

        regAddr =  SMEM_LION3_PTP_TARGET_PORT_TBL_MEM(devObjPtr, egressPort);
        regPtr = smemMemGet(devObjPtr, regAddr);
        if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
        {
            /* bit 321 - Piggyback TST Mode */
            if (SMEM_U32_GET_FIELD(regPtr[10], 1, 1) == 0)
            {
                __LOG(("Piggyback TST Mode: MODE_T_U; use T,U as LSBits\n"));
                set_T_U_bits = GT_TRUE;
            }
            else
            {
                __LOG(("Piggyback TST Mode: MODE_1NS_RES; use ns LSBits\n"));
                set_T_U_bits = GT_FALSE;
            }
        }
        else
        {
            set_T_U_bits = GT_TRUE;
        }

        if (set_T_U_bits)
        {
            /* The 2 lsbits are U & T */
            SMEM_U32_SET_FIELD(nanoSecondTimer,0,1,descrPtr->timestampTagInfo[SMAIN_DIRECTION_INGRESS_E].U);
            SMEM_U32_SET_FIELD(nanoSecondTimer,1,1,descrPtr->timestampTagInfo[SMAIN_DIRECTION_INGRESS_E].T);
            __LOG(("Piggyback TST Mode: U bit [%d] T bit [%d].\n",descrPtr->timestampTagInfo[SMAIN_DIRECTION_INGRESS_E].U,descrPtr->timestampTagInfo[SMAIN_DIRECTION_INGRESS_E].T));
        }

        *(ptpStart++) = (nanoSecondTimer >> 24) & 0xFF;
        *(ptpStart++) = (nanoSecondTimer >> 16) & 0xFF;
        *(ptpStart++) = (nanoSecondTimer >> 8) & 0xFF;
        *(ptpStart)   = (nanoSecondTimer >> 0) & 0xFF;
    }
    else
    {
        __LOG(("Ingress Timestamp was not piggybacked onto the 1588v2 packet.\n"));
    }
}

/**
* @internal snetSip6_30PtpMacCorrectionFieldUpdate function
* @endinternal
*
* @brief   Update CF field in the PTP header.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] macPort                  - target mac port number.
*
*/
static GT_VOID snetSip6_30PtpMacCorrectionFieldUpdate
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 macPort
)
{
    DECLARE_FUNC_NAME(snetSip6_30PtpMacCorrectionFieldUpdate);

    GT_U8  *ptpStart;           /* pointer to start of timestamp related packet */
    GT_U32 taiGroup;            /* the TAI group used for that port */
    SNET_TOD_TIMER_STC tai4TimeCounter;
    SNET_TOD_TIMER_STC deltaTimer;
    GT_U64 correctionField,oldCorrectionField;
    GT_U64 deltaTimer64;

    if (!SMEM_CHT_IS_SIP6_30_GET(devObjPtr) || devObjPtr->supportPtp != 1)
        return;

    taiGroup = SNET_LION3_PORT_NUM_TO_TOD_GROUP_CONVERT_MAC(devObjPtr ,macPort);

    /* sample Timestamp for TAI4 time */
    snetLion3PtpTodGetTimeCounter(descrPtr->egressPhysicalPortInfo.txDmaDevObjPtr,
                                  taiGroup,
                                  4,
                                  &tai4TimeCounter);

    /* PTP timestamping based on Local Action Table */
    ptpStart = descrPtr->haToEpclInfo.l3StartOffsetPtr +
               (descrPtr->ptpGtsInfo.ptpMessageHeaderPtr - descrPtr->payloadPtr);
    ptpStart += 10; /* correction field offset; nanoSec part */

    /* correction field header */

    /* 0    15 16                       63
    +--------+--------+--------+--------+
    | frac   |                          |
    | NanoSec|         NanoSec(48bit)   |
    +--------+--------+--------+--------+*/

    oldCorrectionField.l[0] = *(ptpStart) & 0xFFFFFFFF;
    oldCorrectionField.l[1] = *(ptpStart+4) & 0xFFFF;

    /* check for a WA in the nanoSec */
    if(tai4TimeCounter.nanoSecondTimer > descrPtr->tai4Timestamp[SMAIN_DIRECTION_EGRESS_E].nanoSecondTimer)
    {
        deltaTimer.nanoSecondTimer = tai4TimeCounter.nanoSecondTimer -
                                     descrPtr->tai4Timestamp[SMAIN_DIRECTION_EGRESS_E].nanoSecondTimer;
    }
    else
    {
        deltaTimer.nanoSecondTimer = (10^9) -
                                     (tai4TimeCounter.nanoSecondTimer -
                                      descrPtr->tai4Timestamp[SMAIN_DIRECTION_EGRESS_E].nanoSecondTimer);
    }

    deltaTimer64.l[0] = deltaTimer.nanoSecondTimer;
    deltaTimer64.l[1] = 0;
    correctionField.l[0] = 0;
    correctionField.l[1] = 0;

    correctionField = prvSimMathAdd64(oldCorrectionField,deltaTimer64);

    /* check for WA in correction field; if there is WA put error value instead of new correction value. */
    if (correctionField.l[1] > 0xFFFF)
    {
        /* error value = 0 in msbit, 1 in all other bits */
        correctionField.l[0] = 0xFFFFFFFF;
        correctionField.l[1] = 0x7FFFFFFF;
         __LOG(("WA in correction field. Error value will be set instead of new correction value.\n"));
    }

    *(ptpStart++) = (correctionField.l[0] >> 24) & 0xFF;
    *(ptpStart++) = (correctionField.l[0] >> 16) & 0xFF;
    *(ptpStart++) = (correctionField.l[0] >> 8) & 0xFF;
    *(ptpStart++) = (correctionField.l[0] >> 0) & 0xFF;
    *(ptpStart++) = (correctionField.l[1] >> 8) & 0xFF;
    *(ptpStart++) = (correctionField.l[1] >> 0) & 0xFF;
    __LOG(("new correction field: [0][%d] [1][%d].\n",correctionField.l[0],correctionField.l[1]));

     /* update udp trailer */
    if (descrPtr->ptpUdpChecksumUpdateEnable == GT_TRUE)
    {
        snetlion3PtpUdpChecksum(devObjPtr, descrPtr);
    }
}

/**
* @internal snetSip6_30PtpUpdateFollowUpMessageTlv function
* @endinternal
*
* @brief   Update cumulative scaled rate offset field the PTP
*          follow-up information TLV header.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*
*/
static GT_VOID snetSip6_30PtpUpdateFollowUpMessageTlv
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetSip6_30PtpUpdateFollowUpMessageTlv);

    GT_U8  *ptpStart;           /* pointer to start of timestamp related packet */
    GT_U32 regAddr;             /* Register address */
    GT_U32 *regPtr;             /* Register pointer */
    GT_BOOL followUpTlvUpdateEn = 0; /* indication whether selected tai is enabled for FollowUp message TLV update */
    GT_BOOL syncTlvUpdateEn = 0;     /* indication whether selected tai is enabled for Sync message TLV update */
    GT_BOOL pktIsPtpFollowup = 0;    /* indication whether this is a followup message */
    GT_BOOL pktIsPtpSync = 0;        /* indication whether this is a sync message */

    if (!SMEM_CHT_IS_SIP6_30_GET(devObjPtr) || (descrPtr->timestampActionInfo.packetFormat != SKERNEL_TIMESTAMP_PACKET_FORMAT_PTPV2_E))
        return;

    /* TAI<4> when used for 802.1AS (multi-domain) is dedicated for 2-step timestemping;
       as such it's an illegal config to use TAI<4> (for Sync/FollowUp) and enable FollowUp TLV Update. */
    if(descrPtr->ptpTaiSelect == 4)
    {
        __LOG(("When TAI<4>, cumulative scaled rate offset value in TLV header is set by the CPU.\n"));
        return;
    }

    regAddr = SMEM_LION3_ERMRK_CUMULATIVE_SCALE_RATE_CONFIG_REG(devObjPtr,descrPtr->ptpTaiSelect,0);
    regPtr = smemMemGet(devObjPtr, regAddr);

    /* whether this is a followup message based on desc<is_ptp> and local action table, per PTP message type
       or based on desc<timestamp_en> and TimestampCfg table, per message type */
    pktIsPtpFollowup = (descrPtr->isPtp && (descrPtr->ptpGtsInfo.gtsEntry.msgType == 8 /* MSG_TYPE_FOLLOW_UP */)) ||
                       (descrPtr->oamInfo.timeStampEnable && (descrPtr->timestampActionInfo.ptpMessageType == 8 /* MSG_TYPE_FOLLOW_UP */));

    /* whether this is a sync message based on desc<is_ptp> and local action table, per PTP message type
       or based on desc<timestamp_en> and TimestampCfg table, per message type */
    pktIsPtpSync = (descrPtr->isPtp && (descrPtr->ptpGtsInfo.gtsEntry.msgType == 0 /* MSG_TYPE_SYNC */)) ||
                       (descrPtr->oamInfo.timeStampEnable && (descrPtr->timestampActionInfo.ptpMessageType == 0 /* MSG_TYPE_SYNC */));

    if(pktIsPtpFollowup)
    {
        /* the selected TAI is enabled for FollowUp message TLV update */
        if (SMEM_U32_GET_FIELD(regPtr[0], 1, 1) == 1)
        {
            followUpTlvUpdateEn = 1;
            __LOG(("selected TAI [%d] is enabled for FOLLOW_UP message TLV update.\n",descrPtr->ptpTaiSelect));
        }
    }
    if(pktIsPtpSync)
    {
        /* the selected TAI is enabled for Sync message TLV update */
        if (SMEM_U32_GET_FIELD(regPtr[0], 0, 1) == 1)
        {
            syncTlvUpdateEn = 1;
            __LOG(("selected TAI [%d] is enabled for SYNC message TLV update.\n",descrPtr->ptpTaiSelect));
        }
    }

    if(followUpTlvUpdateEn || syncTlvUpdateEn)
    {
        /* PTP timestamping based on Local Action Table */
        ptpStart = descrPtr->haToEpclInfo.l3StartOffsetPtr +
                   (descrPtr->ptpGtsInfo.ptpMessageHeaderPtr - descrPtr->payloadPtr);
        ptpStart += 54; /* cumulative scaled rate offset */

        regAddr = SMEM_LION3_ERMRK_CUMULATIVE_SCALE_RATE_CONFIG_REG(devObjPtr,descrPtr->ptpTaiSelect,1);
        regPtr = smemMemGet(devObjPtr, regAddr);

        *(ptpStart) = SMEM_U32_GET_FIELD(regPtr[0], 0, 31);
        __LOG(("cumulative scaled rate offset [%d] was updated in the 1588v2 packet according to selected TAI [%d].\n",SMEM_U32_GET_FIELD(regPtr[0], 0, 31),descrPtr->ptpTaiSelect));
    }
}

/*
* @internal snetlion3PtpTimestampingUpdateDelays function
* @endinternal
*
* @brief   Packet Timestamping
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - target port.
* @param[out] ingressTimestampDataPtr - updated ingress timestamp with ingress delays
* @param[out] egressTimestampDataPtr  - updated egress timestamp with egress delay
*
*/
static GT_VOID snetlion3PtpTimestampingUpdateDelays
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 egressPort,
    OUT SNET_TOD_TIMER_STC *ingressTimestampDataPtr,
    OUT SNET_TOD_TIMER_STC *egressTimestampDataPtr
)
{
    DECLARE_FUNC_NAME(snetlion3PtpTimestampingUpdateDelays);

    GT_U32 regAddr;             /* Register address */
    GT_U32 *regPtr;             /* Register pointer */
    GT_U32 value;

    *ingressTimestampDataPtr = descrPtr->timestamp[SMAIN_DIRECTION_INGRESS_E];
    *egressTimestampDataPtr = descrPtr->timestamp[SMAIN_DIRECTION_EGRESS_E];

    if(!(SMEM_CHT_IS_SIP6_30_GET(devObjPtr)))
    {
        /* egress pipe delay */
        regAddr =  SMEM_LION3_PTP_TARGET_PORT_TBL_MEM(devObjPtr, egressPort);
        regPtr = smemMemGet(devObjPtr, regAddr);
        /* egress pipe delay, represented as an unsigned 30 bit nanosecond value; range 0 to 10^9-1. */
        value = SMEM_U32_GET_FIELD(regPtr[0], 2, 30);
        if (value) {
            egressTimestampDataPtr->nanoSecondTimer += value;
            __LOG(("egress pipe delay [%d]\n per egress port [%d]",value,egressPort));
            __LOG(("egressTimestamp.nanoSecondTimer was update [%d]\n",egressTimestampDataPtr->nanoSecondTimer));
        }
    }

    if (descrPtr->ptpActionIsLocal)
    {
        /* ingress port delay */
        regAddr =  SMEM_LION3_PTP_SOURCE_PORT_TBL_MEM(devObjPtr, descrPtr->localDevSrcPort);
        regPtr = smemMemGet(devObjPtr, regAddr);
        /* ingress port delay, represented as a signed 32-bit nanosecond value; range -10^9 to 10^9-1. */
        value = regPtr[0];
        __LOG(("ingress port delay [%d] per source port [%d]\n",value,descrPtr->localDevSrcPort));
        if (value & 0x80000000) /* negative value */
        {
            ingressTimestampDataPtr->nanoSecondTimer += value & 0x7FFFFFFF;/* remove the MSBits and use as value to add */
        }
        else
        {
            ingressTimestampDataPtr->nanoSecondTimer -= value; /* use the value to reduce */
        }
        /* ingress link delay */
        if (descrPtr->timestampActionInfo.ingressLinkDelayEnable)
        {
            /* ingress link delay, represented as a signed 32-bit nanosecond value; range -10^9 to 10^9-1. */
            value = regPtr[1 + descrPtr->ptpDomain];
            __LOG(("ingress link delay [%d] per source port [%d] and ptp domain [%d]\n",value,descrPtr->localDevSrcPort,descrPtr->ptpDomain));
            if (value & 0x80000000) /* negative value */
            {
                ingressTimestampDataPtr->nanoSecondTimer += value & 0x7FFFFFFF;/* remove the MSBits and use as value to add */
            }
            else
            {
                ingressTimestampDataPtr->nanoSecondTimer -= value; /* use the value to reduce */
            }
        }
        __LOG(("ingressTimestamp.nanoSecondTimer was update [%d]\n",ingressTimestampDataPtr->nanoSecondTimer));
    }
}

/**
* @internal snetlion3PtpTimestampingAction function
* @endinternal
*
* @brief   Packet Timestamping
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - target port.
*                                      OUTPUT:
*                                      None
*/
static GT_VOID snetlion3PtpTimestampingAction
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 egressPort
)
{
    DECLARE_FUNC_NAME(snetlion3PtpTimestampingAction);

    TIMESTAMP_TIME_ADD_MODE_ENT tsMode; /* kind of timestamping */
    SNET_TOD_TIMER_STC ingressTimestampData;   /* the values to use for ingress timestamping */
    SNET_TOD_TIMER_STC egressTimestampData;    /* the values to use for egress timestamping */
    SNET_TOD_TIMER_STC tai4EgressTimestampData;/* the values to use for egress tai4 timestamping */
    SNET_TOD_TIMER_STC timestampData;    /* the values to use for timestamping */
    SNET_TOD_TIMER_STC timestampData1;   /* the values to use for timestamping */
    SNET_TOD_TIMER_STC  deltaTimer;
    GT_U8  *ptpStart;           /* pointer to start of timestamp related packet */
    GT_U8  *ptpStart1 = NULL;          /* pointer to start of timestamp related packet */
    GT_U32 tsByteLength;        /* timestamp length */
    GT_U64 secondTimer;
    GT_BOOL pktUsxEn = 0;       /* packet enabled for USXGMII PTP processing (precision mode is USX) */
    GT_BOOL pktMchEn = 0;       /* packet enabled for MCH feature */
    GT_BOOL pktMchTwoStepEn = 0; /* two step MCH */
    GT_BOOL pktMchOneStepEn = 0; /* one step MCH */
    GT_U32 regAddr;             /* Register address */
    GT_U32 *regPtr;             /* Register pointer */
    GT_U32 macPort;             /* the MAC port number */
    GT_U64 correctionField,oldCorrectionField;
    GT_U64 deltaTimer64;
    SNET_PTP_UDP_CHECKSUM_MODE_ENT ptpUdpChecksumMode;
    GT_U32 startBit;

    memset(&deltaTimer,0,sizeof(deltaTimer));
    memset(&deltaTimer64,0,sizeof(deltaTimer64));
    memset(&correctionField,0,sizeof(correctionField));

    /* update checksum */
    if (descrPtr->ptpTriggerType == SKERNEL_PTP_TRIGGER_TYPE_PTP_OVER_IPV4_UDP_E || descrPtr->ptpTriggerType == SKERNEL_PTP_TRIGGER_TYPE_PTP_OVER_IPV6_UDP_E)
    {
        regAddr = SMEM_LION3_ERMRK_TS_UDP_CHECKSUM_MODE_REG(devObjPtr);
        regPtr = smemMemGet(devObjPtr, regAddr);
        if( descrPtr->ptpTriggerType == SKERNEL_PTP_TRIGGER_TYPE_PTP_OVER_IPV4_UDP_E )
        {
            startBit = 0;
        }
        else
        {
            startBit = 2;
        }
        ptpUdpChecksumMode = SMEM_U32_GET_FIELD(*regPtr, startBit, 2);
        if (ptpUdpChecksumMode == SNET_PTP_UDP_CHECKSUM_MODE_UPDATE_E)
        {
            descrPtr->ptpUdpChecksumUpdateEnable = GT_TRUE;
        }
        else
        {
            descrPtr->ptpUdpChecksumUpdateEnable = GT_FALSE;
        }
    }

    ingressTimestampData = descrPtr->timestamp[SMAIN_DIRECTION_INGRESS_E];
    egressTimestampData = descrPtr->timestamp[SMAIN_DIRECTION_EGRESS_E];
    tai4EgressTimestampData = descrPtr->tai4Timestamp[SMAIN_DIRECTION_EGRESS_E];

    /*fix VC10 warning : warning C4700: uninitialized local variable 'timestampData1' used  */
    timestampData1 = ingressTimestampData;

    if (SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
    {
        macPort = descrPtr->egressPhysicalPortInfo.txDmaMacPort;
        regAddr = SMEM_SIP6_10_GOP_PTP_TS_PRECISION_CTRL_REG(devObjPtr,macPort);
        regPtr = smemMemGet(devObjPtr, regAddr);

        if ((SMEM_U32_GET_FIELD(regPtr[0], 3, 1) == 1) /* port enabled for USXGMII */ &&
            (descrPtr->macTimestampingEnable == GT_TRUE) /* timestamp required */ &&
            (descrPtr->timestampMaskProfile == 0) /* masking profile disabled */ &&
            (descrPtr->timestampActionInfo.packetFormat == SKERNEL_TIMESTAMP_PACKET_FORMAT_PTPV2_E))
        {
            __LOG(("port [%d] is enabled for USXGMII PTP processing.\n",egressPort));
            pktUsxEn = 1;
        }

        if ((SMEM_U32_GET_FIELD(regPtr[0], 6, 1) == 1) /* port enabled for MCH */ && pktUsxEn)
        {
            __LOG(("port [%d] is enabled for MCH PTP processing.\n",egressPort));
            pktMchEn = 1;
        }

        if (pktMchEn /* Packet enabled for MCH timestamping */ &&
            (descrPtr->timestampActionInfo.action == SKERNEL_TIMESTAMP_ACTION_CAPTURE_E) /* packet egress time should be captured in TSQ */ &&
            (SMEM_U32_GET_FIELD(regPtr[0], 7, 1) == 1 /* TWO_STEP MCH */))
        {
            pktMchTwoStepEn = 1;
            __LOG(("port [%d] is set to two step MCH for a capture action.\n",egressPort));
        }

        if (SMEM_U32_GET_FIELD(regPtr[0], 0, 1) == 0 /* mac_cf_update_en */ &&
            (descrPtr->timestampActionInfo.packetFormat == SKERNEL_TIMESTAMP_PACKET_FORMAT_PTPV2_E &&
             descrPtr->macTimestampingEnable && (descrPtr->timestampMaskProfile == 0) &&
             ((descrPtr->timestampActionInfo.action == SKERNEL_TIMESTAMP_ACTION_ADD_TIME_E) ||
              (descrPtr->timestampActionInfo.action == SKERNEL_TIMESTAMP_ACTION_ADD_CORRECTED_TIME_E) ||
              (descrPtr->timestampActionInfo.action == SKERNEL_TIMESTAMP_ACTION_CAPTURE_ADD_TIME_E) ||
              (descrPtr->timestampActionInfo.action == SKERNEL_TIMESTAMP_ACTION_CAPTURE_ADD_CORRECTED_TIME_E) ||
              (descrPtr->timestampActionInfo.action == SKERNEL_TIMESTAMP_ACTION_CAPTURE_E))))
        {
            __LOG(("Channel [%d] is enabled for Egress TS Correction.\n",macPort));
            descrPtr->mtiCfUpdate = 1;
        }
        else
        {
            __LOG(("Channel [%d] is not enabled for Egress TS Correction.\n",macPort));
            descrPtr->mtiCfUpdate = 0;
        }

        if (pktMchEn /* Packet enabled for MCH timestamping */ &&
            (!descrPtr->mtiCfUpdate) /* Packet is disabled for MAC CF update (if enabled, MAC is doing the one step, rather than a PHY) */ &&
            ((descrPtr->timestampActionInfo.action != SKERNEL_TIMESTAMP_ACTION_CAPTURE_E) /* egress timestamping other than CAPTURE */ ||
             (SMEM_U32_GET_FIELD(regPtr[0], 7, 1) == 0 /* port is set to one step MCH for a capture action */)))
        {
            pktMchOneStepEn = 1;
            __LOG(("port [%d] is set to one step MCH for a capture action.\n",egressPort));
        }

        if (pktMchOneStepEn &&
           ((descrPtr->timestampActionInfo.action == SKERNEL_TIMESTAMP_ACTION_ADD_TIME_E) ||
           (descrPtr->timestampActionInfo.action == SKERNEL_TIMESTAMP_ACTION_CAPTURE_E) ||
           (descrPtr->timestampActionInfo.action == SKERNEL_TIMESTAMP_ACTION_CAPTURE_ADD_TIME_E)))
        {

             /* check for a WA in the nanoSec */
            if(egressTimestampData.nanoSecondTimer > tai4EgressTimestampData.nanoSecondTimer)
            {
                deltaTimer.nanoSecondTimer = egressTimestampData.nanoSecondTimer -
                                             tai4EgressTimestampData.nanoSecondTimer;
                deltaTimer.secondTimer = egressTimestampData.secondTimer;
            }
            else
            {
                deltaTimer.nanoSecondTimer = (10^9) -
                                             (egressTimestampData.nanoSecondTimer -
                                              tai4EgressTimestampData.nanoSecondTimer);
                secondTimer.l[0] = 1;
                secondTimer.l[1] = 0;
                deltaTimer.secondTimer = prvSimMathSub64(deltaTimer.secondTimer,
                                                         secondTimer);
            }

            /* update values */
            egressTimestampData.secondTimer = deltaTimer.secondTimer;
            egressTimestampData.nanoSecondTimer = deltaTimer.nanoSecondTimer;
        }

        /* build PTP PHY signature */
        if(pktMchTwoStepEn)
        {
            /* {3'b0,Marvell_PCH<QueueSelect>,Marvell_PCH<QueueEntryId<[9:0]>} */
            descrPtr->ptpPhyInfo.mchSignature = (descrPtr->timestampActionInfo.egressInfo.egressQueueNum & 1) << 10 | (descrPtr->timestampActionInfo.egressInfo.egressQueueEntryId & 0x3FF);
            descrPtr->ptpPhyInfo.mchSignature |= (1 << 14); /* TWO STEP */
            descrPtr->ptpPhyInfo.tstfEnable = GT_FALSE;
            descrPtr->ptpPhyInfo.step = SNET_PTP_PHY_STEP_TYPE_TWO_STEP_E;

        }
        else
        {
            descrPtr->ptpPhyInfo.mchSignature = tai4EgressTimestampData.secondTimer.l[0] & 1;
            descrPtr->ptpPhyInfo.mchSignature |= ((descrPtr->ptpUdpChecksumUpdateEnable == GT_TRUE ? 1 : 0) <<1);
            descrPtr->ptpPhyInfo.mchSignature |= (descrPtr->ptpEgressTaiSel & 3) << 2;
            if ((descrPtr->ptpCfOffset % 2) != 0)
            {
                __LOG(("wrong calculation: ptpCfOffset can't be devided in 2.\n",descrPtr->ptpCfOffset));
            }
            descrPtr->ptpPhyInfo.mchSignature |= (((descrPtr->ptpCfOffset >> 1) & 0x7F) << 4); /* The PTP header CF field offset, in 2B resolution. field range is 0-254B.*/
            descrPtr->ptpPhyInfo.tstfEnable = GT_FALSE;
            descrPtr->ptpPhyInfo.step = SNET_PTP_PHY_STEP_TYPE_ONE_STEP_E;
            descrPtr->ptpPhyInfo.ingTimeSecLsb = tai4EgressTimestampData.secondTimer.l[0] & 1;
            descrPtr->ptpPhyInfo.ChecksumUpdate = descrPtr->ptpUdpChecksumUpdateEnable;
            descrPtr->ptpPhyInfo.taiSelect = descrPtr->ptpEgressTaiSel;
            descrPtr->ptpPhyInfo.cfOffset = (((descrPtr->ptpCfOffset >> 1) & 0x7F) << 4);
        }
    }

    switch(descrPtr->timestampActionInfo.action)
    {
        case SKERNEL_TIMESTAMP_ACTION_ADD_TIME_E:
        case SKERNEL_TIMESTAMP_ACTION_CAPTURE_ADD_TIME_E:
            tsMode = TIMESTAMP_TIME_ADD_MODE_EGRESS_E;
            timestampData = egressTimestampData;
            /* enable MAC modification */
            descrPtr->macTimestampingEnable = 1;
            break;
        case SKERNEL_TIMESTAMP_ACTION_ADD_CORRECTED_TIME_E:
        case SKERNEL_TIMESTAMP_ACTION_CAPTURE_ADD_CORRECTED_TIME_E:
            tsMode = TIMESTAMP_TIME_ADD_MODE_CORRECTED_E;

            /* check for a WA in the nanoSec */
            if( egressTimestampData.nanoSecondTimer > ingressTimestampData.nanoSecondTimer )
            {
                timestampData.nanoSecondTimer =
                    egressTimestampData.nanoSecondTimer -
                    ingressTimestampData.nanoSecondTimer;

                timestampData.secondTimer = egressTimestampData.secondTimer;
            }
            else
            {
                timestampData.nanoSecondTimer = (10^9) -
                    ( egressTimestampData.nanoSecondTimer -
                      ingressTimestampData.nanoSecondTimer );

                secondTimer.l[0] = 1;
                secondTimer.l[1] = 0;
                timestampData.secondTimer =
                prvSimMathSub64(egressTimestampData.secondTimer,
                                secondTimer);
            }
            /* convert nanoSec to 64bits */
            deltaTimer64.l[0] = timestampData.nanoSecondTimer;
            deltaTimer64.l[1] = 0;

            /* allow negative values; no need for WA */
            if(pktMchOneStepEn)
            {
                timestampData.nanoSecondTimer -= tai4EgressTimestampData.nanoSecondTimer;
            }
            break;
        case SKERNEL_TIMESTAMP_ACTION_ADD_INGRESS_TIME_E:
        case SKERNEL_TIMESTAMP_ACTION_CAPTURE_ADD_INGRESS_TIME_E:
            tsMode = TIMESTAMP_TIME_ADD_MODE_INGRESS_E;
            timestampData = ingressTimestampData;
            break;

        case SKERNEL_TIMESTAMP_ACTION_ADD_INGRESS_EGRESS_TIME_E:
            tsMode = TIMESTAMP_TIME_ADD_MODE_INGRESS_EGRESS_E;
            timestampData = egressTimestampData;
            timestampData1 = ingressTimestampData;
            break;

        default:
            return;
    }

    if( descrPtr->oamInfo.timeStampEnable )
    {
        /* Timestamp based on Timestamp Offset Table */
        ptpStart = descrPtr->haToEpclInfo.l3StartOffsetPtr + descrPtr->timestampActionInfo.offset;
        ptpStart1 = descrPtr->haToEpclInfo.l3StartOffsetPtr + descrPtr->timestampActionInfo.offset2;
    }
    else if ( descrPtr->ptpActionIsLocal )
    {
        /* PTP timestamping based on Local Action Table */
        ptpStart = descrPtr->haToEpclInfo.l3StartOffsetPtr +
                    (descrPtr->ptpGtsInfo.ptpMessageHeaderPtr - descrPtr->payloadPtr);
    }
    else
    {
        return;
    }

    tsByteLength = 0;

    if(tsMode == TIMESTAMP_TIME_ADD_MODE_INGRESS_E)
    {
        switch(descrPtr->timestampActionInfo.packetFormat)
        {
            case SKERNEL_TIMESTAMP_PACKET_FORMAT_NTPR_E:
                ptpStart += 32;
                GT_ATTR_FALLTHROUGH;
            case SKERNEL_TIMESTAMP_PACKET_FORMAT_Y1731_E:
            case SKERNEL_TIMESTAMP_PACKET_FORMAT_NTP_E:
                tsByteLength = 8;
                break;
            default:
                break;
        }
    }
    else if(tsMode == TIMESTAMP_TIME_ADD_MODE_EGRESS_E)
    {
        switch(descrPtr->timestampActionInfo.packetFormat)
        {
            case SKERNEL_TIMESTAMP_PACKET_FORMAT_PTPV2_E:
                ptpStart += 34; /* pointer to requestReceiptTimestamp/originTimestamp */
                tsByteLength = 10;

                if(descrPtr->haToEpclInfo.tunnelStartL2StartOffsetPtr != NULL)
                {
                    descrPtr->ptpCfOffset = ptpStart - descrPtr->haToEpclInfo.tunnelStartL2StartOffsetPtr;
                }
                else
                {
                    descrPtr->ptpCfOffset = ptpStart - descrPtr->haToEpclInfo.macDaSaPtr;
                }
                break;
            case SKERNEL_TIMESTAMP_PACKET_FORMAT_PTPV1_E:
                ptpStart += 40;
                tsByteLength = 8;
                break;
            case SKERNEL_TIMESTAMP_PACKET_FORMAT_Y1731_E:
            case SKERNEL_TIMESTAMP_PACKET_FORMAT_NTP_E:
                tsByteLength = 8;
                break;
            case SKERNEL_TIMESTAMP_PACKET_FORMAT_NTPT_E:
                ptpStart += 36;
                tsByteLength = 8;
                break;
            case SKERNEL_TIMESTAMP_PACKET_FORMAT_WAMP_E:
                ptpStart += 4;
                tsByteLength = 8;
                break;
            default:
                break;
        }
    }
    else if(tsMode == TIMESTAMP_TIME_ADD_MODE_CORRECTED_E)
    {
        switch(descrPtr->timestampActionInfo.packetFormat)
        {
            case SKERNEL_TIMESTAMP_PACKET_FORMAT_PTPV2_E:
                ptpStart += 10; /* pointer to CF; nanoSec part */
                tsByteLength = 6;

                 /* correction field header */

                /* 0    15 16                       63
                +--------+--------+--------+--------+
                | frac   |                          |
                | NanoSec|         NanoSec(48bit)   |
                +--------+--------+--------+--------+*/

                oldCorrectionField.l[0] = *(ptpStart) & 0xFFFFFFFF;
                oldCorrectionField.l[1] = *(ptpStart+4) & 0xFFFF;

                correctionField = prvSimMathAdd64(oldCorrectionField,deltaTimer64);

                /* check for WA in correction field; if there is WA put error value instead of new correction value. */
                if (correctionField.l[1] > 0xFFFF)
                {
                    /* error value = 0 in msbit, 1 in all other bits */
                    correctionField.l[0] = 0xFFFFFFFF;
                    correctionField.l[1] = 0x7FFFFFFF;
                     __LOG(("WA in correction field. Error value will be set instead of new correction value.\n"));
                }

                if(descrPtr->haToEpclInfo.tunnelStartL2StartOffsetPtr != NULL)
                {
                    descrPtr->ptpCfOffset = ptpStart - descrPtr->haToEpclInfo.tunnelStartL2StartOffsetPtr;
                }
                else
                {
                    descrPtr->ptpCfOffset = ptpStart - descrPtr->haToEpclInfo.macDaSaPtr;
                }
                break;
            default:
                break;
        }
    }
    else if(tsMode == TIMESTAMP_TIME_ADD_MODE_INGRESS_EGRESS_E)
    {
        switch(descrPtr->timestampActionInfo.packetFormat)
        {
            case SKERNEL_TIMESTAMP_PACKET_FORMAT_Y1731_E:
            case SKERNEL_TIMESTAMP_PACKET_FORMAT_NTPT_E:
                tsByteLength = 8;
                break;
            default:
                break;
        }
    }

    switch(tsByteLength)
    {
        case 10:
            *(ptpStart++) = (timestampData.secondTimer.l[1]>>8) & 0xff;
            *(ptpStart++) = (timestampData.secondTimer.l[1]) & 0xff;
            GT_ATTR_FALLTHROUGH;
        case 8:
            *(ptpStart++) = (timestampData.secondTimer.l[0]>>24) & 0xff;
            *(ptpStart++) = (timestampData.secondTimer.l[0]>>16) & 0xff;
            *(ptpStart++) = (timestampData.secondTimer.l[0]>>8) & 0xff;
            *(ptpStart++) = (timestampData.secondTimer.l[0]) & 0xff;
            *(ptpStart++) = SMEM_U32_GET_FIELD(timestampData.nanoSecondTimer, 24, 8);
            *(ptpStart++) = SMEM_U32_GET_FIELD(timestampData.nanoSecondTimer, 16, 8);
            *(ptpStart++) = SMEM_U32_GET_FIELD(timestampData.nanoSecondTimer, 8, 8);
            *(ptpStart) = SMEM_U32_GET_FIELD(timestampData.nanoSecondTimer, 0, 8);
            if(tsMode == TIMESTAMP_TIME_ADD_MODE_INGRESS_EGRESS_E && ptpStart1)
            {
                *(ptpStart1++) = (timestampData1.secondTimer.l[0]>>24) & 0xff;
                *(ptpStart1++) = (timestampData1.secondTimer.l[0]>>16) & 0xff;
                *(ptpStart1++) = (timestampData1.secondTimer.l[0]>>8) & 0xff;
                *(ptpStart1++) = (timestampData1.secondTimer.l[0]) & 0xff;
                *(ptpStart1++) = SMEM_U32_GET_FIELD(timestampData1.nanoSecondTimer, 24, 8);
                *(ptpStart1++) = SMEM_U32_GET_FIELD(timestampData1.nanoSecondTimer, 16, 8);
                *(ptpStart1++) = SMEM_U32_GET_FIELD(timestampData1.nanoSecondTimer, 8, 8);
                *(ptpStart1) = SMEM_U32_GET_FIELD(timestampData1.nanoSecondTimer, 0, 8);
            }
            if(descrPtr->lmuEn)
            {
                __LOG(("Timestamped packets are excluded from latency monitoring measurements"));
                descrPtr->lmuEn = 0;
            }
            break;
        case 6: /* update correction  field nanoSec (48bits) part in PTP header.*/
            *(ptpStart++) = (correctionField.l[0] >> 24) & 0xFF;
            *(ptpStart++) = (correctionField.l[0] >> 16) & 0xFF;
            *(ptpStart++) = (correctionField.l[0] >> 8) & 0xFF;
            *(ptpStart++) = (correctionField.l[0] >> 0) & 0xFF;
            *(ptpStart++) = (correctionField.l[1] >> 8) & 0xFF;
            *(ptpStart++) = (correctionField.l[1] >> 0) & 0xFF;
            break;
        default:
            __LOG(("No packet timestamping"));
            break;
    }
}

/**
* @internal snetlion3TimestampPortEgressQueueInterruptEvent function
* @endinternal
*
* @brief   Setting interrupt for timestamp port egress queue event - full or new entry.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] macPort                  - Egress MAC port.
* @param[in] interruptBit             - the interrupt bit to set.
*                                      OUTPUT:
*                                      None
*/
static GT_VOID snetlion3TimestampPortEgressQueueInterruptEvent
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 macPort,
    IN GT_U32 interruptBit
)
{
    DECLARE_FUNC_NAME(snetlion3TimestampPortEgressQueueInterruptEvent);
    GT_U32 causeRegAddr = SMEM_LION3_GOP_PTP_INT_CAUSE_REG(devObjPtr, macPort);

    __LOG_PARAM(macPort);
    __LOG_PARAM(causeRegAddr);
    /* Generate per- port Timestamp queue interrupt */
    snetChetahDoInterrupt(devObjPtr,
                          SMEM_LION3_GOP_PTP_INT_CAUSE_REG(devObjPtr, macPort),
                          SMEM_LION3_GOP_PTP_INT_MASK_REG(devObjPtr, macPort),
                          (1<<interruptBit),
                          (GT_U32)SMEM_LION3_GOP_SUM_INT(devObjPtr));
}

/**
* @internal snetlion3TimestampQueueInterruptEvent function
* @endinternal
*
* @brief   Setting interrupt for timestamp queue event - full or new entry.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] interruptBit             - the interrupt bit to set.
*                                      OUTPUT:
*                                      None
*/
static GT_VOID snetlion3TimestampQueueInterruptEvent
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 interruptBit
)
{
    /* Generate ERMRK Timestamp queue interrupt */
    snetChetahDoInterrupt(devObjPtr,
                          SMEM_LION3_ERMRK_INTERRUPT_CAUSE_REG(devObjPtr),
                          SMEM_LION3_ERMRK_INTERRUPT_MASK_REG(devObjPtr),
                          (1<<interruptBit),
                          (GT_U32)SMEM_LION3_ERMRK_SUM_INT(devObjPtr));
}

/**
* @internal snetlion3TimestampPortEgressQueueInsertEntry function
* @endinternal
*
* @brief   Insert entry to timestamp port egress queue (result of capture action)
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] macPort                  - Egress mac port
* @param[in] queueNum                 - 0 or 1.
* @param[in] queueEntryPtr            - pointer to queue entry data.
*                                      OUTPUT:
*                                      None
*/
static GT_VOID snetlion3TimestampPortEgressQueueInsertEntry
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 macPort,
    IN GT_U32 queueNum,
    IN GT_U32 *queueEntryPtr
)
{
    DECLARE_FUNC_NAME(snetlion3TimestampPortEgressQueueInsertEntry);


    SKERNEL_TIMESTAMP_QUEUE_STC *queuePtr;
    GT_U32 regAddr;
    GT_U32 *regPtr;

    queuePtr = &devObjPtr->portsArr[macPort].timestampEgressQueue[queueNum];

    SCIB_SEM_TAKE;
    if( queuePtr->isFull == GT_TRUE )
    {

        /* Queue Full interrupt */
          /*bobcat 2 errata 4402776: full interrupt does not work*/
        if (devObjPtr->errata.fullQueuesPtpPerPort)
        {
            __LOG(("NOTE: Although should be Queue Full interrupt, due to errata, bobcat2 does not generate this interrupt \n"));
        }
        else
        {

            __LOG(("Queue Full interrupt"));
            snetlion3TimestampPortEgressQueueInterruptEvent(devObjPtr, macPort, (7+queueNum));
        }

        __LOG_PARAM(queuePtr->overwriteEnable);

        if( queuePtr->overwriteEnable == GT_FALSE )
        {
            SCIB_SEM_SIGNAL;
            return;
        }

        __LOG(("MAC port [%d] PTP queue full but allow override .. so adding new PTP timestamp entry \n",
               macPort));

        queuePtr->removeIndex = (queuePtr->removeIndex + 1)%(queuePtr->actualSize);
    }

    /* New insert interrupt */
    __LOG(("New insert interrupt MAC ptp \n"));
    __LOG(("Insert new message MAC[%d] to PTP queue[%d] interrupt ",macPort,queueNum));
    snetlion3TimestampPortEgressQueueInterruptEvent(devObjPtr, macPort, (5+queueNum));

    memcpy(queuePtr->theQueue[queuePtr->insertIndex], queueEntryPtr, 12);

    if( queuePtr->isEmpty == GT_TRUE )
    {
        regAddr = SMEM_LION3_GOP_PTP_TX_TS_QUEUE_REG0_REG(devObjPtr,macPort,queueNum);
        regPtr = smemMemGet(devObjPtr, regAddr);
        *regPtr = queueEntryPtr[0];

        regAddr = SMEM_LION3_GOP_PTP_TX_TS_QUEUE_REG1_REG(devObjPtr,macPort,queueNum);
        regPtr = smemMemGet(devObjPtr, regAddr);
        *regPtr = queueEntryPtr[1];

        if (!SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {
            regAddr = SMEM_LION3_GOP_PTP_TX_TS_QUEUE_REG2_REG(devObjPtr,macPort,queueNum);
            regPtr = smemMemGet(devObjPtr, regAddr);
            *regPtr = queueEntryPtr[2];
        }
    }

    queuePtr->isEmpty = GT_FALSE;

    queuePtr->insertIndex = (queuePtr->insertIndex + 1)%(queuePtr->actualSize);

    if( queuePtr->insertIndex == queuePtr->removeIndex )
    {
        queuePtr->isFull = GT_TRUE;
    }
    SCIB_SEM_SIGNAL;
}

/**
* @internal snetlion3TimestampPortEgressQueueRemoveEntry function
* @endinternal
*
* @brief   Remove entry from timestamp port egress queue (due to CPU read)
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] macPort                  - Egress MAC port.
* @param[in] queueNum                 - 0 or 1.
*                                      OUTPUT:
*                                      None
*/
GT_VOID snetlion3TimestampPortEgressQueueRemoveEntry
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 macPort,
    IN GT_U32 queueNum
)
{

    DECLARE_FUNC_NAME(snetlion3TimestampPortEgressQueueRemoveEntry);

    SKERNEL_TIMESTAMP_QUEUE_STC *queuePtr;
    GT_U32 regAddr;
    GT_U32 *regPtr;
    queuePtr = &devObjPtr->portsArr[macPort].timestampEgressQueue[queueNum];

    SCIB_SEM_TAKE;
    if( queuePtr->isEmpty == GT_TRUE )
    {
        __LOG(("port Queue is empty for port mac [%d], queue num %d\n",macPort,queueNum));
        SCIB_SEM_SIGNAL;
        return;
    }

    queuePtr->removeIndex = (queuePtr->removeIndex + 1)%(queuePtr->actualSize);

    if( queuePtr->removeIndex == queuePtr->insertIndex )
    {
        __LOG(("port Queue became empty due to last read for port mac [%d], queue num %d\n",macPort,queueNum));
        queuePtr->isEmpty = GT_TRUE;

        regAddr = SMEM_LION3_GOP_PTP_TX_TS_QUEUE_REG0_REG(devObjPtr,macPort,queueNum);
        regPtr = smemMemGet(devObjPtr, regAddr);
        *regPtr = 0x0;

        regAddr = SMEM_LION3_GOP_PTP_TX_TS_QUEUE_REG1_REG(devObjPtr,macPort,queueNum);
        regPtr = smemMemGet(devObjPtr, regAddr);
        *regPtr = 0x0;

        if (!SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {
            regAddr = SMEM_LION3_GOP_PTP_TX_TS_QUEUE_REG2_REG(devObjPtr,macPort,queueNum);
            regPtr = smemMemGet(devObjPtr, regAddr);
            *regPtr = 0x0;
        }
    }
    else
    {
        __LOG(("Retrieve entry from port queue for port mac [%d], queue num %d\n",macPort,queueNum));

        regAddr = SMEM_LION3_GOP_PTP_TX_TS_QUEUE_REG0_REG(devObjPtr,macPort,queueNum);
        regPtr = smemMemGet(devObjPtr, regAddr);
        *regPtr = queuePtr->theQueue[queuePtr->removeIndex][0];
        regAddr = SMEM_LION3_GOP_PTP_TX_TS_QUEUE_REG1_REG(devObjPtr,macPort,queueNum);
        regPtr = smemMemGet(devObjPtr, regAddr);
        *regPtr = queuePtr->theQueue[queuePtr->removeIndex][1];

        if (!SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {
            regAddr = SMEM_LION3_GOP_PTP_TX_TS_QUEUE_REG2_REG(devObjPtr,macPort,queueNum);
            regPtr = smemMemGet(devObjPtr, regAddr);
            *regPtr = queuePtr->theQueue[queuePtr->removeIndex][2];
        }
    }

    queuePtr->isFull = GT_FALSE;
    SCIB_SEM_SIGNAL;
}

/**
* @internal snetlion3TimestampQueueInsertEntry function
* @endinternal
*
* @brief   Insert entry to timestamp queue (result of capture action)
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] direction                - ingress or egress.
* @param[in] queueNum                 - 0 or 1.
* @param[in] queueEntryPtr            - pointer to queue entry data.
*                                      OUTPUT:
* @param[in] insertPtpTimestamp       - 0 - the timestamp should not enter to the ptp mac queue
*                                      1 - the timestamp should enter to the ptp mac queue
*/
static GT_VOID snetlion3TimestampQueueInsertEntry
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SMAIN_DIRECTION_ENT   direction,
    IN GT_U32 queueNum,
    IN GT_U32 *queueEntryPtr,
    OUT GT_BOOL *insertPtpTimestamp
)
{
    DECLARE_FUNC_NAME(snetlion3TimestampQueueInsertEntry);

    SKERNEL_TIMESTAMP_QUEUE_STC *queuePtr;
    GT_U32 currentPipeId;
    GT_U32 queueIndex;
    GT_U32 queueWidthInWords;
    GT_U32 regAddr;
    GT_U32 *regPtr;
    GT_U32 ii;

    /* support multi-pipe device */
    currentPipeId = smemGetCurrentPipeId(devObjPtr);
    queueIndex = TS_QUEUE_NUMBERING_TO_QUEUE_INDEX_MAC(direction,queueNum);
    queueWidthInWords = (direction == SMAIN_DIRECTION_EGRESS_E) ? 2 : 3;

    queuePtr = &devObjPtr->timestampQueue[direction][queueNum][currentPipeId];

    SCIB_SEM_TAKE;
    if( queuePtr->isFull == GT_TRUE )
    {
        /* Queue Full interrupt */
        /*bobcat 2 errata 4683196: full interrupt does not work*/
        if (devObjPtr->errata.fullQueuesPtpErmrk)
        {
          __LOG(("NOTE: Although should be Queue Full interrupt, due to errata, bobcat2 does not generate this interrupt \n"));
        }
        else
        {
            __LOG(("Queue Full interrupt \n"));
            snetlion3TimestampQueueInterruptEvent(devObjPtr, (7+queueIndex));
        }

        if( queuePtr->overwriteEnable == GT_FALSE )
        {
            SCIB_SEM_SIGNAL;
            *insertPtpTimestamp = GT_FALSE;
            return;
        }

        queuePtr->removeIndex = (queuePtr->removeIndex + 1)%(queuePtr->actualSize);
    }

    *insertPtpTimestamp = GT_TRUE;
    /* New insert interrupt */
    __LOG(("New insert interrupt ermrk \n"));
    snetlion3TimestampQueueInterruptEvent(devObjPtr, (3+queueIndex));

    memcpy(queuePtr->theQueue[queuePtr->insertIndex], queueEntryPtr, 12);
    if( queuePtr->isEmpty == GT_TRUE )
    {
        for( ii = 0; ii < queueWidthInWords ; ii++)
        {
            regAddr = SMEM_LION3_ERMRK_TS_QUEUE_ENTRY_WORD_REG(devObjPtr,queueIndex,ii);
            regPtr = smemMemGet(devObjPtr, regAddr);
            *regPtr = queuePtr->theQueue[queuePtr->insertIndex][ii];

        }
    }

    queuePtr->isEmpty = GT_FALSE;

    queuePtr->insertIndex = (queuePtr->insertIndex + 1)%(queuePtr->actualSize);


    if( queuePtr->insertIndex == queuePtr->removeIndex )
    {
        queuePtr->isFull = GT_TRUE;
    }
    SCIB_SEM_SIGNAL;
}

/**
* @internal snetlion3TimestampQueueRemoveEntry function
* @endinternal
*
* @brief   Remove entry from timestamp queue (due to CPU read)
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] direction                - ingress or egress.
* @param[in] queueNum                 - 0 or 1.
*                                      OUTPUT:
*                                      None
*/
GT_VOID snetlion3TimestampQueueRemoveEntry
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SMAIN_DIRECTION_ENT   direction,
    IN GT_U32 queueNum
)
{

    DECLARE_FUNC_NAME(snetlion3TimestampQueueRemoveEntry);

    SKERNEL_TIMESTAMP_QUEUE_STC *queuePtr;
    GT_U32 currentPipeId;
    GT_U32 queueIndex;
    GT_U32 queueWidthInWords;
    GT_U32 regAddr;
    GT_U32 *regPtr;
    GT_U32 ii;

     /* support multi-pipe device */
    currentPipeId = smemGetCurrentPipeId(devObjPtr);
    queueIndex = TS_QUEUE_NUMBERING_TO_QUEUE_INDEX_MAC(direction,queueNum);
    queueWidthInWords = (direction == SMAIN_DIRECTION_EGRESS_E) ? 2 : 3;

    queuePtr = &devObjPtr->timestampQueue[direction][queueNum][currentPipeId];

    SCIB_SEM_TAKE;
    if( queuePtr->isEmpty == GT_TRUE )
    {
        __LOG(("Queue is empty \n"));

        SCIB_SEM_SIGNAL;
        return;
    }

    queuePtr->removeIndex = (queuePtr->removeIndex + 1)%(queuePtr->actualSize);

    if( queuePtr->removeIndex == queuePtr->insertIndex )
    {
       __LOG(("Queue became empty due to last read"));
       queuePtr->isEmpty = GT_TRUE;
       for( ii = 0; ii < queueWidthInWords ; ii++)
       {
           regAddr = SMEM_LION3_ERMRK_TS_QUEUE_ENTRY_WORD_REG(devObjPtr,queueIndex,ii);
           regPtr = smemMemGet(devObjPtr, regAddr);
           *regPtr = 0;

       }
    }
    else
    {
        __LOG_PARAM(currentPipeId);
        __LOG_PARAM(queueWidthInWords);
        __LOG(("Retrieve entry from queue"));
        for( ii = 0; ii < queueWidthInWords ; ii++)
        {
            regAddr = SMEM_LION3_ERMRK_TS_QUEUE_ENTRY_WORD_REG(devObjPtr,queueIndex,ii);
            __LOG_PARAM(ii);
            __LOG_PARAM(regAddr);
            regPtr = smemMemGet(devObjPtr, regAddr);
            *regPtr = queuePtr->theQueue[queuePtr->removeIndex][ii];
            __LOG(("seccued reg_ptr"));
        }
    }

    queuePtr->isFull = GT_FALSE;
    SCIB_SEM_SIGNAL;
}

/**
* @internal snetlion3TimestampQueueCapture function
* @endinternal
*
* @brief   capture to timestamp queues
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - egress port.
*                                      OUTPUT:
*                                      None
*/
static GT_VOID snetlion3TimestampQueueCapture
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32   egressPort
)
{
    DECLARE_FUNC_NAME(snetlion3TimestampQueueCapture);

    GT_U32 regAddr;             /* Register address */
    GT_U32 *regPtr;             /* Register pointer */

    SKERNEL_TIMESTAMP_PACKET_FORMAT_ENT packetFormat = descrPtr->timestampActionInfo.packetFormat;
    GT_U32 queueMessageType;
    GT_U32 queueDomainNumber;
    GT_U32 queueSeqFlowId;
    GT_U32 queueNumber; /* 0 or 1 */
    GT_U32 queueEntryId;
    GT_U32 timestampQueueEntry[3];
    GT_U32 taiGroup;
    GT_U32 portNumSize;
    GT_BOOL insertPtpTimestamp;
    SNET_TOD_TIMER_STC ingressTimestampData;   /* the values to use for ingress timestamping */
    SNET_TOD_TIMER_STC egressTimestampData;    /* the values to use for egress timestamping */

    /* update delays for timestamping */
    snetlion3PtpTimestampingUpdateDelays(devObjPtr,descrPtr,egressPort,&ingressTimestampData,&egressTimestampData);

    if( (packetFormat == SKERNEL_TIMESTAMP_PACKET_FORMAT_PTPV1_E) ||
        (packetFormat == SKERNEL_TIMESTAMP_PACKET_FORMAT_PTPV2_E) )
    {
        if( descrPtr->oamInfo.timeStampEnable )
        {
            queueMessageType = descrPtr->timestampActionInfo.ptpMessageType;
            /* queueDomainNumber = descrPtr->timestampActionInfo.ptpDomain */
        }
        else
        {
            queueMessageType = descrPtr->ptpGtsInfo.gtsEntry.msgType;
            /* queueDomainNumber = descrPtr->ptpDomain; */
        }

        queueDomainNumber = descrPtr->ptpDomain;
        queueSeqFlowId = descrPtr->ptpGtsInfo.gtsEntry.seqID;
    }
    else
    {
        queueMessageType = 0;
        queueDomainNumber = 0;
        queueSeqFlowId = descrPtr->flowId;
    }

    /* select queue #0 or #1 */
    regAddr = SMEM_LION3_ERMRK_TS_QUEUE_MESSAGE_TYPE_REG(devObjPtr);
    regPtr = smemMemGet(devObjPtr, regAddr);
    queueNumber = SMEM_U32_GET_FIELD(*regPtr, queueMessageType, 1);

    memset(timestampQueueEntry, 0x0, 12);

    /* PSU timestamp queues (ingress & egress) */
    if( (descrPtr->timestampActionInfo.action ==
                    SKERNEL_TIMESTAMP_ACTION_CAPTURE_E) ||
        (descrPtr->timestampActionInfo.action ==
                    SKERNEL_TIMESTAMP_ACTION_CAPTURE_ADD_TIME_E) ||
        (descrPtr->timestampActionInfo.action ==
                    SKERNEL_TIMESTAMP_ACTION_CAPTURE_ADD_CORRECTED_TIME_E) )
    {
        /* Egress timestamp queue */

        /* Valid */
        snetFieldValueSet(timestampQueueEntry, 0, 1, 1);
        /* Is PTP Exception */
        snetFieldValueSet(timestampQueueEntry, 1, 1, descrPtr->isPtpException);
        /* Packet Format */
        snetFieldValueSet(timestampQueueEntry, 2, 3, packetFormat);
        /* Message Type */
        snetFieldValueSet(timestampQueueEntry, 5, 4, queueMessageType);
        /* Domain Number */
        snetFieldValueSet(timestampQueueEntry, 9, 8, queueDomainNumber);
        /* Sequence ID or Flow ID */
        snetFieldValueSet(timestampQueueEntry, 17, 16, queueSeqFlowId);
        /* Queue Entry ID */
        regAddr = SMEM_LION3_ERMRK_TS_QUEUE_ENTRY_ID_REG(devObjPtr);
        regPtr = smemMemGet(devObjPtr, regAddr);
        queueEntryId = SMEM_U32_GET_FIELD(*regPtr, queueNumber*10, 10);
        snetFieldValueSet(timestampQueueEntry, 33, 10, queueEntryId);
        SMEM_U32_SET_FIELD(*regPtr, queueNumber*10, 10, (queueEntryId+1)%0x400);

        /* Port Number is 10 bits in SIP_6, 9 bits starting from SIP_20,
           8 bits for previous devices */
        portNumSize = (SMEM_CHT_IS_SIP6_GET(devObjPtr)) ? 10 :
                       ((SMEM_CHT_IS_SIP5_20_GET(devObjPtr)) ? 9 : 8);

        /* Physical Port*/
        snetFieldValueSet(timestampQueueEntry, 43, portNumSize, egressPort);

        __LOG(("ERMRK Egress timestamp queue"));
        snetlion3TimestampQueueInsertEntry(devObjPtr,
                                           SMAIN_DIRECTION_EGRESS_E,
                                           queueNumber,
                                           timestampQueueEntry,
                                           &insertPtpTimestamp);

        /* causes MAC to insert the corresponding timestamp into its queue */
        descrPtr->macTimestampingEnable = 1;

        /* per Port Egress timestamp queue (done in TSU) */

        memset(timestampQueueEntry, 0x0, 12);
        /* Valid */
        snetFieldValueSet(timestampQueueEntry, 0, 1, 1);
        /* Queue Entry ID */
        snetFieldValueSet(timestampQueueEntry, 1, 10, queueEntryId);
        /* TAI select */
        if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
        {
            snetFieldValueSet(timestampQueueEntry, 13, 3, descrPtr->ptpEgressTaiSel);
        }
        else
        {
            snetFieldValueSet(timestampQueueEntry, 11, 1, descrPtr->ptpEgressTaiSel);
        }

        if(devObjPtr->supportSingleTai == 0)
        {
            taiGroup = SNET_LION3_PORT_NUM_TO_TOD_GROUP_CONVERT_MAC(devObjPtr ,egressPort);
        }
        else
        {
            if (descrPtr->ptpEgressTaiSel != 0)
            {
                __LOG(("single TAI, ptpEgressTaiSel found not null \n"));
                descrPtr->ptpEgressTaiSel = 0;
            }
            taiGroup = 0;
        }
        regAddr = SMEM_LION3_GOP_TAI_UPDATE_COUNTER_REG(devObjPtr, taiGroup, descrPtr->ptpEgressTaiSel);
        regPtr = smemMemGet(devObjPtr, regAddr);

        /* TOD update flag */
        snetFieldValueSet(timestampQueueEntry, 12, 1, SMEM_U32_GET_FIELD(*regPtr, 0,  1));

        if (!SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {
            /* Timestamp */
            snetFieldValueSet(timestampQueueEntry, 13, 3,
                              SMEM_U32_GET_FIELD(egressTimestampData.nanoSecondTimer, 0, 3));
            snetFieldValueSet(timestampQueueEntry, 32, 16,
                              SMEM_U32_GET_FIELD(egressTimestampData.nanoSecondTimer, 3, 16));
            snetFieldValueSet(timestampQueueEntry, 64, 13,
                              SMEM_U32_GET_FIELD(egressTimestampData.nanoSecondTimer, 19,13));
        }
        else
        {
            /* Timestamp */
            snetFieldValueSet(timestampQueueEntry, 32, 3,
                              SMEM_U32_GET_FIELD(egressTimestampData.nanoSecondTimer, 0, 3));
            snetFieldValueSet(timestampQueueEntry, 35, 16,
                              SMEM_U32_GET_FIELD(egressTimestampData.nanoSecondTimer, 3, 16));
            snetFieldValueSet(timestampQueueEntry, 51, 13,
                              SMEM_U32_GET_FIELD(egressTimestampData.nanoSecondTimer, 19,13));
        }

        if (GT_TRUE == insertPtpTimestamp)
        {
            descrPtr->timestampActionInfo.doEgressTimeStampEgressPort = GT_TRUE;
        }
        else
        {
            descrPtr->timestampActionInfo.doEgressTimeStampEgressPort = GT_FALSE;
        }

        descrPtr->timestampActionInfo.egressInfo.egressQueueNum = queueNumber;
        descrPtr->timestampActionInfo.egressInfo.egressQueueEntryId = queueEntryId;
        memcpy(&descrPtr->timestampActionInfo.egressInfo.egress_timestampQueueEntry,
               &timestampQueueEntry,
               sizeof(timestampQueueEntry));
    }
    else if( (descrPtr->timestampActionInfo.action ==
                    SKERNEL_TIMESTAMP_ACTION_CAPTURE_ADD_INGRESS_TIME_E) ||
             (descrPtr->timestampActionInfo.action ==
                    SKERNEL_TIMESTAMP_ACTION_CAPTURE_INGRESS_TIME_E) )
    {
        /* Ingress timestamp queue */
        /* Valid */
        snetFieldValueSet(timestampQueueEntry, 0, 1, 1);
        /* Is PTP Exception */
        snetFieldValueSet(timestampQueueEntry, 1, 1, descrPtr->isPtpException);
        /* Packet Format */
        snetFieldValueSet(timestampQueueEntry, 2, 3, packetFormat);
        /* TAI select */
        snetFieldValueSet(timestampQueueEntry, 5, 1, descrPtr->ptpTaiSelect);
        if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
        {
            snetFieldValueSet(timestampQueueEntry, 77, 2, (descrPtr->ptpTaiSelect >> 1));
        }
        /* TOD update flag */
        snetFieldValueSet(timestampQueueEntry, 6, 1, descrPtr->ptpUField);
        /* Message Type */
        snetFieldValueSet(timestampQueueEntry, 7, 4, queueMessageType);
        /* Domain Number */
        snetFieldValueSet(timestampQueueEntry, 11, 8, queueDomainNumber);
        /* Sequence ID or Flow ID */
        snetFieldValueSet(timestampQueueEntry, 19, 16, queueSeqFlowId);
        /* Timestamp */
        snetFieldValueSet(timestampQueueEntry, 35, 32,
                          ingressTimestampData.nanoSecondTimer);
        /* Physical Port*/
        snetFieldValueSet(timestampQueueEntry, 67, 8, egressPort);
        __LOG(("ERMRK Ingress timestamp queue"));
        snetlion3TimestampQueueInsertEntry(devObjPtr,
                                           SMAIN_DIRECTION_INGRESS_E,
                                           queueNumber,
                                           timestampQueueEntry,
                                           &insertPtpTimestamp);
    }
}
#if 0 /*CPSS_TBD_BOOKMARK_IRONMAN*/
/**
* @internal snetSip6_30TimestampPortMacEgressQueueInsertEntry function
* @endinternal
*
* @brief   Insert entry to timestamp port MAC egress queue (result of capture action)
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] macPort                  - Egress mac port
* @param[in] queueEntryPtr            - pointer to queue entry data.
*                                      OUTPUT:
*                                      None
*/
static GT_VOID snetSip6_30TimestampPortMacEgressQueueInsertEntry
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 macPort,
    IN GT_U32 *queueEntryPtr
)
{
    DECLARE_FUNC_NAME(snetSip6_30TimestampPortMacEgressQueueInsertEntry);


    SKERNEL_TIMESTAMP_QUEUE_STC *queuePtr;
    GT_U32 regAddr;
      GT_U32 *regPtr;

    queuePtr = &devObjPtr->portsArr[macPort].timestampMacEgressQueue;

    SCIB_SEM_TAKE;
    if( queuePtr->isFull == GT_TRUE )
    {
        /* Queue Full interrupt */
        __LOG(("TSQ FIFO overflow indication interrupt MAC[%d]",macPort));
        snetlion3TimestampPortEgressQueueInterruptEvent(devObjPtr, macPort, 22);

        __LOG_PARAM(queuePtr->overwriteEnable);

        if( queuePtr->overwriteEnable == GT_FALSE )
        {
            SCIB_SEM_SIGNAL;
            return;
        }

        __LOG(("MAC port [%d] Mac Egress PTP queue full but allow override .. so adding new PTP timestamp entry \n",
               macPort));

        queuePtr->removeIndex = (queuePtr->removeIndex + 1)%(queuePtr->actualSize);
    }

    /* New insert interrupt */
    __LOG(("New insert interrupt MAC ptp \n"));
    __LOG(("Indicates that Timestamp and Signature were written to the TSQ FIFO interrupt MAC[%d]",macPort));
    snetlion3TimestampPortEgressQueueInterruptEvent(devObjPtr, macPort, 21);

    memcpy(queuePtr->theQueue[queuePtr->insertIndex], queueEntryPtr, 8);

    if( queuePtr->isEmpty == GT_TRUE )
    {
        regAddr = USX_PORT_EXT(devObjPtr,macPort).tsqTimestamp;
        regPtr = smemMemGet(devObjPtr, regAddr);
        *regPtr = queueEntryPtr[0];

        regAddr = USX_PORT_EXT(devObjPtr,macPort).tsqSignature;
        regPtr = smemMemGet(devObjPtr, regAddr);
        *regPtr = queueEntryPtr[1];
    }

    queuePtr->isEmpty = GT_FALSE;

    queuePtr->insertIndex = (queuePtr->insertIndex + 1)%(queuePtr->actualSize);

    if( queuePtr->insertIndex == queuePtr->removeIndex )
    {
        queuePtr->isFull = GT_TRUE;
    }
    SCIB_SEM_SIGNAL;
}

/**
* @internal snetSip6_30TimestampPortMacEgressQueueRemoveEntry function
* @endinternal
*
* @brief   Remove entry from timestamp port MAC egress queue (due to CPU read)
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] macPort                  - Egress MAC port.
*                                      OUTPUT:
*                                      None
*/
GT_VOID snetSip6_30TimestampPortMacEgressQueueRemoveEntry
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 macPort
)
{

    DECLARE_FUNC_NAME(snetSip6_30TimestampPortMacEgressQueueRemoveEntry);

    SKERNEL_TIMESTAMP_QUEUE_STC *queuePtr;
    GT_U32 regAddr;
    GT_U32 *regPtr;
    queuePtr = &devObjPtr->portsArr[macPort].timestampMacEgressQueue;

    SCIB_SEM_TAKE;
    if( queuePtr->isEmpty == GT_TRUE )
    {
        __LOG(("Mac Egress PTP port Queue is empty for port mac [%d]\n",macPort));
        SCIB_SEM_SIGNAL;
        return;
    }

    queuePtr->removeIndex = (queuePtr->removeIndex + 1)%(queuePtr->actualSize);

    if( queuePtr->removeIndex == queuePtr->insertIndex )
    {
        __LOG(("Mac Egress PTP port Queue became empty due to last read for port mac [%d]\n",macPort));
        queuePtr->isEmpty = GT_TRUE;
        regAddr = USX_PORT_EXT(devObjPtr,macPort).tsqTimestamp;
        regPtr = smemMemGet(devObjPtr, regAddr);
        *regPtr = 0x0;

        regAddr = USX_PORT_EXT(devObjPtr,macPort).tsqSignature;
        regPtr = smemMemGet(devObjPtr, regAddr);
        *regPtr = 0x0;
    }
    else
    {
        __LOG(("Retrieve entry from Mac Egress PTP port queue for port mac [%d]\n",macPort));

        regAddr = USX_PORT_EXT(devObjPtr,macPort).tsqTimestamp;
        regPtr = smemMemGet(devObjPtr, regAddr);
        *regPtr = queuePtr->theQueue[queuePtr->removeIndex][0];
        regAddr = USX_PORT_EXT(devObjPtr,macPort).tsqSignature;
        regPtr = smemMemGet(devObjPtr, regAddr);
        *regPtr = queuePtr->theQueue[queuePtr->removeIndex][1];
    }

    queuePtr->isFull = GT_FALSE;
    SCIB_SEM_SIGNAL;
}
#endif
/**
* @internal snetSip6_30TimestampPortMacEgressQueueCapture function
* @endinternal
*
* @brief   capture to Mac Egress timestamp queues
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - egress port.
* @param[in] macPort                  - MAC Port number.
* @param[in] pchInfoPtr               - pointer to PCH PTP information.
*                                      OUTPUT:
*                                      None
*/
static GT_VOID snetSip6_30TimestampPortMacEgressQueueCapture
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32   egressPort,
    IN GT_U32   macPort,
    IN SNET_PHY_SWITCH_USGMII_PCH_PTP_INFO_STC * pchInfoPtr
)
{
    if (!SMEM_CHT_IS_SIP6_30_GET(devObjPtr) || devObjPtr->supportPtp != 1)
        return;

#if 0 /*CPSS_TBD_BOOKMARK_IRONMAN*/
    DECLARE_FUNC_NAME(snetSip6_30TimestampPortMacEgressQueueCapture);

    GT_U32 regAddr;             /* Register address */
    GT_U32 *regPtr;             /* Register pointer */
    GT_U32 timestampQueueEntry[2];
    GT_U32 taiGroup;
    SNET_TOD_TIMER_STC tai4TimeCounter;
    GT_BOOL mac_tsq_capture_en = GT_FALSE;

    regAddr = USX_PORT_EXT(devObjPtr,macPort).portControl;
    regPtr = smemMemGet(devObjPtr, regAddr);

    /* check condition for capturing the egress time of a packet in the MAC */
    if (SMEM_U32_GET_FIELD(regPtr[0], 9, 1) /* port enabled for MAC time capture */ &&
        connectedPortToPhyEgress[egressPort] && devObjPtr->portsArr[egressPort].state == SKERNEL_PORT_STATE_MTI_USX_E &&
        descrPtr->ptpPrecisionMode == SKERNEL_PTP_PRECISION_MODE_USX_E /* PTP mdoe is USX */ &&
        descrPtr->ptpPhyInfo.tstfEnable == GT_FALSE /* MCH is not PHY TSTF */ &&
        descrPtr->ptpPhyInfo.step == SNET_PTP_PHY_STEP_TYPE_TWO_STEP_E /* MCH is "two step MCH" */ &&
        descrPtr->timestampActionInfo.action == SKERNEL_TIMESTAMP_ACTION_CAPTURE_E)
    {
        mac_tsq_capture_en = GT_TRUE;
    }

    if (mac_tsq_capture_en == GT_FALSE)
        return;

    memset(timestampQueueEntry, 0x0, 8);

    taiGroup = SNET_LION3_PORT_NUM_TO_TOD_GROUP_CONVERT_MAC(devObjPtr ,macPort);

    /* sample Timestamp for TAI4 time */
    snetLion3PtpTodGetTimeCounter(devObjPtr,
                                  taiGroup,
                                  4,
                                  &tai4TimeCounter);

    /* Timestamp */
    snetFieldValueSet(timestampQueueEntry, 0, 30, SMEM_U32_GET_FIELD(tai4TimeCounter.nanoSecondTimer, 0, 30));
    snetFieldValueSet(timestampQueueEntry, 30, 2, SMEM_U32_GET_FIELD(tai4TimeCounter.secondTimer.l[0], 0, 2));

    /* TSQ Signature Value */
    snetFieldValueSet(timestampQueueEntry, 32, 14, SMEM_U32_GET_FIELD((GT_U32)&pchInfoPtr->signature.mchSignature, 0, 14));

    /* TSQ Valid */
    snetFieldValueSet(timestampQueueEntry, 48, 1, 1);

    __LOG(("Port Mac Egress timestamp queue"));
    return snetSip6_30TimestampPortMacEgressQueueInsertEntry(devObjPtr,
                                                             egressPort,
                                                             timestampQueueEntry);
#endif
}

/**
* @internal snetlion3TimestampTodExtraction function
* @endinternal
*
* @brief   TAI Selection and TOD Extraction
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*                                      macPort     - egress MAC port.
*                                      OUTPUT:
*                                      None
*/
static GT_VOID snetlion3TimestampTodExtraction
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32   egressPort
)
{
    DECLARE_FUNC_NAME(snetlion3TimestampTodExtraction);

    GT_U32 regAddr;     /* Register address */
    GT_U32 *regPtr;     /* Register pointer */
    GT_U32 macPort;    /* the MAC port number */
    GT_U32 taiGroup;    /* the TAI group used for that port */

    if ((descrPtr->timestampActionInfo.action ==
            SKERNEL_TIMESTAMP_ACTION_ADD_CORRECTED_TIME_E) ||
         (descrPtr->timestampActionInfo.action ==
            SKERNEL_TIMESTAMP_ACTION_CAPTURE_ADD_CORRECTED_TIME_E))
    {
        if (!SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {
            __LOG(("SIP6: Egress TAI number always equals Ingress one"));
            descrPtr->ptpEgressTaiSel = descrPtr->ptpTaiSelect;
        }
        else
        {
            regAddr =  SMEM_LION3_PTP_TARGET_PORT_TBL_MEM(devObjPtr, egressPort);
            regPtr = smemMemGet(devObjPtr, regAddr);
            if (SMEM_U32_GET_FIELD(*regPtr, 0, 1) == 0)
            {
                __LOG(("Egress TAI number equals Ingress one"));
                descrPtr->ptpEgressTaiSel = descrPtr->ptpTaiSelect;
            }
            else
            {
                __LOG(("Egress TAI number selected separately"));
                descrPtr->ptpEgressTaiSel = SMEM_U32_GET_FIELD(*regPtr, 1, 1);
            }
        }
    }

    macPort = descrPtr->egressPhysicalPortInfo.txDmaMacPort;

    taiGroup = SNET_LION3_PORT_NUM_TO_TOD_GROUP_CONVERT_MAC(devObjPtr ,macPort);

    /* sample Timestamp for egress TAI<n> */
    snetLion3PtpTodGetTimeCounter(descrPtr->egressPhysicalPortInfo.txDmaDevObjPtr,
                                  taiGroup,
                                  descrPtr->ptpEgressTaiSel,
                                  &descrPtr->timestamp[SMAIN_DIRECTION_EGRESS_E]);

    if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
    {
       /* sample Timestamp for TAI4 time */
       snetLion3PtpTodGetTimeCounter(descrPtr->egressPhysicalPortInfo.txDmaDevObjPtr,
                                     taiGroup,
                                     4,
                                     &descrPtr->tai4Timestamp[SMAIN_DIRECTION_EGRESS_E]);
   }
}


/**
* @internal snetLion3PtpUpdateCounters function
* @endinternal
*
* @brief   Update action type and packet format counters
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*                                      egressPort  - egress port.
*                                      OUTPUT:
*                                      None
*/
static GT_VOID snetLion3PtpUpdateCounters
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32   macPort
)
{
    DECLARE_FUNC_NAME(snetLion3PtpUpdateCounters);

    GT_U32 regAddr;      /* Register address */
    GT_U32 *regPtr;      /* Register pointer */
    GT_U32 fldValue;
    GT_U32 tsuEnableBit; /* bit number for TSU unit enable */
    GT_U32 index;
    GT_U32 pchTimestampAction;

    if( descrPtr->timestampActionInfo.doCountIngressTimeStampEgressPort == GT_FALSE )
    {
        __LOG(("Timestamp action not require counting on egress MAC port [%d] \n", macPort));
        return;
    }

    if( descrPtr->oamInfo.timeStampEnable )
    {
        /* PTP general control register */
        regAddr = SMEM_LION3_GOP_PTP_GENERAL_CTRL_REG(devObjPtr, macPort);
        regPtr = smemMemGet(devObjPtr, regAddr);
        /* Timestamping unit enable */
        tsuEnableBit = (SMEM_CHT_IS_SIP6_GET(devObjPtr)) ? 1 : 0;
        fldValue = SMEM_U32_GET_FIELD(*regPtr, tsuEnableBit, 1);

        if(fldValue == 0)
        {
            __LOG(("Timestamp unit disabled on egress MAC port [%d] \n", macPort));
            return;
        }
    }

    if (!SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        switch(descrPtr->timestampActionInfo.packetFormat)
        {
            case SKERNEL_TIMESTAMP_PACKET_FORMAT_PTPV2_E:
                regAddr = SMEM_LION3_GOP_PTP_PTPv2_PKT_CNTR_REG(devObjPtr,macPort);
                break;
            case SKERNEL_TIMESTAMP_PACKET_FORMAT_PTPV1_E:
                regAddr = SMEM_LION3_GOP_PTP_PTPv1_PKT_CNTR_REG(devObjPtr,macPort);
                break;
            case SKERNEL_TIMESTAMP_PACKET_FORMAT_Y1731_E:
                regAddr = SMEM_LION3_GOP_PTP_Y1731_PKT_CNTR_REG(devObjPtr,macPort);
                break;
            case SKERNEL_TIMESTAMP_PACKET_FORMAT_NTP_E:
                regAddr = SMEM_LION3_GOP_PTP_NTPTS_PKT_CNTR_REG(devObjPtr,macPort);
                break;
            case SKERNEL_TIMESTAMP_PACKET_FORMAT_NTPR_E:
                regAddr = SMEM_LION3_GOP_PTP_NTPRX_PKT_CNTR_REG(devObjPtr,macPort);
                break;
            case SKERNEL_TIMESTAMP_PACKET_FORMAT_NTPT_E:
                regAddr = SMEM_LION3_GOP_PTP_NTPTX_PKT_CNTR_REG(devObjPtr,macPort);
                break;
            case SKERNEL_TIMESTAMP_PACKET_FORMAT_WAMP_E:
                regAddr = SMEM_LION3_GOP_PTP_WAMP_PKT_CNTR_REG(devObjPtr,macPort);
                break;
            default:
                __LOG(("ERROR : unknown packetFormat[%d]\n",
                        descrPtr->timestampActionInfo.packetFormat));
                return;
        }

        __LOG((" update counter per packetFormat \n"));
        regPtr = smemMemGet(devObjPtr, regAddr);
        fldValue = SMEM_U32_GET_FIELD(*regPtr, 0, 8);
        SMEM_U32_SET_FIELD(*regPtr, 0, 8, (fldValue+1)%0x100);

        __LOG((" update counter per packetFormat by 1 from value[%d] \n",fldValue));
    }

    if( (descrPtr->timestampActionInfo.packetFormat ==
                SKERNEL_TIMESTAMP_PACKET_FORMAT_PTPV1_E) ||
        (descrPtr->timestampActionInfo.packetFormat ==
                SKERNEL_TIMESTAMP_PACKET_FORMAT_PTPV2_E) )
    {
        regAddr = SMEM_LION3_GOP_PTP_TOTAL_PTP_PKTS_CNTR_REG(devObjPtr,macPort);
        regPtr = smemMemGet(devObjPtr, regAddr);
        fldValue = SMEM_U32_GET_FIELD(*regPtr, 0, 8);
        SMEM_U32_SET_FIELD(*regPtr, 0, 8, (fldValue+1)%0x100);

        __LOG((" update counter : PTP_TOTAL_PTP_PKTS_CNTR by 1 from value[%d] \n",fldValue));
    }

    if (!SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        GT_CHAR *actionNamePtr;
        switch(descrPtr->timestampActionInfo.action)
        {
            case SKERNEL_TIMESTAMP_ACTION_NONE_E:
                regAddr = SMEM_LION3_GOP_PTP_NONE_ACTION_PKT_CNTR_REG(devObjPtr,macPort);
                actionNamePtr = STR(SKERNEL_TIMESTAMP_ACTION_NONE_E);
                break;
            case SKERNEL_TIMESTAMP_ACTION_FORWARD_E:
                regAddr = SMEM_LION3_GOP_PTP_FORWARD_ACTION_PKT_CNTR_REG(devObjPtr,macPort);
                actionNamePtr = STR(SKERNEL_TIMESTAMP_ACTION_FORWARD_E);
                break;
            case SKERNEL_TIMESTAMP_ACTION_DROP_E:
                regAddr = SMEM_LION3_GOP_PTP_DROP_ACTION_PKT_CNTR_REG(devObjPtr,macPort);
                actionNamePtr = STR(SKERNEL_TIMESTAMP_ACTION_DROP_E);
                break;
            case SKERNEL_TIMESTAMP_ACTION_CAPTURE_E:
                regAddr = SMEM_LION3_GOP_PTP_CAPTURE_ACTION_PKT_CNTR_REG(devObjPtr,macPort);
                actionNamePtr = STR(SKERNEL_TIMESTAMP_ACTION_CAPTURE_E);
                break;
            case SKERNEL_TIMESTAMP_ACTION_ADD_TIME_E:
                regAddr = SMEM_LION3_GOP_PTP_ADD_TIME_ACTION_PKT_CNTR_REG(devObjPtr,macPort);
                actionNamePtr = STR(SKERNEL_TIMESTAMP_ACTION_ADD_TIME_E);
                break;
            case SKERNEL_TIMESTAMP_ACTION_ADD_CORRECTED_TIME_E:
                regAddr = SMEM_LION3_GOP_PTP_ADD_CORRECTED_TIME_ACTION_PKT_CNTR_REG(devObjPtr,macPort);
                actionNamePtr = STR(SKERNEL_TIMESTAMP_ACTION_ADD_CORRECTED_TIME_E);
                break;
            case SKERNEL_TIMESTAMP_ACTION_CAPTURE_ADD_TIME_E:
                regAddr = SMEM_LION3_GOP_PTP_CAPTURE_ADD_TIME_PKT_CNTR_REG(devObjPtr,macPort);
                actionNamePtr = STR(SKERNEL_TIMESTAMP_ACTION_CAPTURE_ADD_TIME_E);
                break;
            case SKERNEL_TIMESTAMP_ACTION_CAPTURE_ADD_CORRECTED_TIME_E:
                regAddr = SMEM_LION3_GOP_PTP_CAPTURE_ADD_CORRECTED_TIME_ACTION_PKT_CNTR_REG(devObjPtr,macPort);
                actionNamePtr = STR(SKERNEL_TIMESTAMP_ACTION_CAPTURE_ADD_CORRECTED_TIME_E);
                break;
            case SKERNEL_TIMESTAMP_ACTION_ADD_INGRESS_TIME_E:
                regAddr = SMEM_LION3_GOP_PTP_ADD_INGRESS_TIME_ACTION_PKT_CNTR_REG(devObjPtr,macPort);
                actionNamePtr = STR(SKERNEL_TIMESTAMP_ACTION_ADD_INGRESS_TIME_E);
                break;
            case SKERNEL_TIMESTAMP_ACTION_CAPTURE_ADD_INGRESS_TIME_E:
                regAddr = SMEM_LION3_GOP_PTP_CAPTURE_ADD_INGRESS_TIME_ACTION_PKT_CNTR_REG(devObjPtr,macPort);
                actionNamePtr = STR(SKERNEL_TIMESTAMP_ACTION_CAPTURE_ADD_INGRESS_TIME_E);
                break;
            case SKERNEL_TIMESTAMP_ACTION_CAPTURE_INGRESS_TIME_E:
                regAddr = SMEM_LION3_GOP_PTP_CAPTURE_INGRESS_TIME_ACTION_PKT_CNTR_REG(devObjPtr,macPort);
                actionNamePtr = STR(SKERNEL_TIMESTAMP_ACTION_CAPTURE_INGRESS_TIME_E);
                break;
            default:
                __LOG(("ERROR : unknown action[%d]\n",
                        descrPtr->timestampActionInfo.action));
                return;
        }

        regPtr = smemMemGet(devObjPtr, regAddr);
        fldValue = SMEM_U32_GET_FIELD(*regPtr, 0, 8);
        SMEM_U32_SET_FIELD(*regPtr, 0, 8, (fldValue+1)%0x100);

        __LOG((" update counter [%s] per time stamp action type by 1 from value[%d] \n",
            actionNamePtr,fldValue));
    }
    else
    {
        pchTimestampAction = (GT_U32)descrPtr->timestampActionInfo.action;
        if (pchTimestampAction == SKERNEL_TIMESTAMP_ACTION_ADD_INGRESS_EGRESS_TIME_E)
        {
            /* this code passed to TSU in PCH */
            pchTimestampAction = SKERNEL_TIMESTAMP_ACTION_ADD_TIME_E;
        }
        for(index=0; index<4; index++)
        {
            /* Check if the action and format combination counter is enabled */
            regAddr = SMEM_FALCON_GOP_PTP_CAPTURE_INGRESS_FRAME_CNTR_CTRL_REG(devObjPtr, macPort, index);
            regPtr = smemMemGet(devObjPtr, regAddr);
            if((SMEM_U32_GET_FIELD(*regPtr, 11, 1)) &&
               ((descrPtr->timestampMaskProfile == 0) ||
                (SMEM_U32_GET_FIELD(*regPtr, 8, 3) == descrPtr->timestampMaskProfile)) &&
               ((descrPtr->timestampActionInfo.packetFormat == SKERNEL_TIMESTAMP_PACKET_FORMAT_ALL_E) ||
                (SMEM_U32_GET_FIELD(*regPtr, 0, 4) == (GT_U32)descrPtr->timestampActionInfo.packetFormat)) &&
               ((descrPtr->timestampActionInfo.action == SKERNEL_TIMESTAMP_ACTION_ALL_E) ||
                (SMEM_U32_GET_FIELD(*regPtr, 4, 4) == pchTimestampAction)))
            {
                /* Increment the counter */
                regAddr = SMEM_FALCON_GOP_PTP_CAPTURE_INGRESS_FRAME_CNTR_REG(devObjPtr, macPort, index);
                regPtr = smemMemGet(devObjPtr, regAddr);
                __LOG((" update counter CAPTURE_INGRESS_FRAME index[%d] by 1 from value[%d] \n",
                    index,*regPtr));
                *regPtr += 1;
            }
        }
    }
}

/**
* @internal snetlion3PtpActionAssign function
* @endinternal
*
* @brief   Timestamp action assignment
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - egress port.
*                                      OUTPUT:
*                                      None
*/
static GT_VOID snetlion3PtpActionAssign
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32   egressPort
)
{
    DECLARE_FUNC_NAME(snetlion3PtpActionAssign);

    GT_U32 regAddr;                         /* Register address */
    GT_U32 * regPtr;                        /* Register pointer */
    GT_U32 fldValue;                        /* Register field value */
    GT_BOOL doCount;

    doCount = GT_FALSE;

    if( descrPtr->oamInfo.timeStampEnable )
    {
        /* Timestamp Offset Table */
        regAddr =  SMEM_LION2_OAM_TIMESTAMP_OFFSET_TBL_MEM(devObjPtr,
                                                           descrPtr->oamInfo.offsetIndex);
        regPtr = smemMemGet(devObjPtr, regAddr);

        fldValue = SMEM_U32_GET_FIELD(*regPtr, 0, 1);

        if( fldValue &&
            ((descrPtr->timestampTagged[SMAIN_DIRECTION_EGRESS_E] ==
                SKERNEL_TIMESTAMP_TAG_TYPE_NON_EXTENDED_E) ||
              (descrPtr->timestampTagged[SMAIN_DIRECTION_INGRESS_E] ==
                SKERNEL_TIMESTAMP_TAG_TYPE_EXTENDED_E)) )
        {
            /* Modify TST is relevant only to non-extended or extended TST */
            /* Modify TST with OE and profile offset fields */
            __LOG(("Modify TST in Timestamp Offset table"));
            descrPtr->haToEpclInfo.timeStampTagPtr[3] =
                    ((SMEM_U32_GET_FIELD(*regPtr, 8, 1)) << 7) +
                      SMEM_U32_GET_FIELD(*regPtr, 1, 7);
            descrPtr->timestampActionInfo.action = SKERNEL_TIMESTAMP_ACTION_NONE_E;
        }
        else
        {
            __LOG(("Do Action in Timestamp Offset table"));
            /* Do action case */
            descrPtr->timestampActionInfo.action = SMEM_U32_GET_FIELD(*regPtr, 1, 4);
            descrPtr->timestampActionInfo.packetFormat = SMEM_U32_GET_FIELD(*regPtr, 5, 3);
            descrPtr->timestampActionInfo.transportType = SMEM_U32_GET_FIELD(*regPtr, 8, 2);
            if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
            {
                descrPtr->timestampActionInfo.offset = SMEM_U32_GET_FIELD(*regPtr, 10, 7);
                /* ingress offset is offset2 << 1 */
                descrPtr->timestampActionInfo.offset2 = (SMEM_U32_GET_FIELD(*regPtr, 27, 5) |
                                                         (SMEM_U32_GET_FIELD(regPtr[1], 0, 2) << 5)) << 1;
                descrPtr->timestampActionInfo.ptpMessageType = SMEM_U32_GET_FIELD(*regPtr, 18, 4);
                descrPtr->timestampActionInfo.ptpDomain = SMEM_U32_GET_FIELD(*regPtr, 22, 3);
                descrPtr->timestampActionInfo.ingressLinkDelayEnable = SMEM_U32_GET_FIELD(*regPtr, 25, 1);
                descrPtr->timestampActionInfo.packetDispatchingEnable = SMEM_U32_GET_FIELD(*regPtr, 26, 1);
            }
            else if(SMEM_CHT_IS_SIP5_10_GET(devObjPtr) == 0)
            {
                descrPtr->timestampActionInfo.offset = SMEM_U32_GET_FIELD(*regPtr, 10, 7);
                descrPtr->timestampActionInfo.ptpMessageType = SMEM_U32_GET_FIELD(*regPtr, 17, 4);
                descrPtr->timestampActionInfo.ptpDomain = SMEM_U32_GET_FIELD(*regPtr, 21, 3);
            }
            else
            {
                descrPtr->timestampActionInfo.offset = SMEM_U32_GET_FIELD(*regPtr, 10, 8);
                descrPtr->timestampActionInfo.ptpMessageType = SMEM_U32_GET_FIELD(*regPtr, 18, 4);
                descrPtr->timestampActionInfo.ptpDomain = SMEM_U32_GET_FIELD(*regPtr, 22, 3);
                descrPtr->timestampActionInfo.ingressLinkDelayEnable = SMEM_U32_GET_FIELD(*regPtr, 25, 1);
                descrPtr->timestampActionInfo.packetDispatchingEnable = SMEM_U32_GET_FIELD(*regPtr, 26, 1);
            }

            doCount = GT_TRUE;
        }
    }
    else if (descrPtr->ptpActionIsLocal)
    {
        if( descrPtr->ptpGtsInfo.gtsEntry.ptpVersion == 1)
        {
            __LOG(("PTPv1 in PTP Local Action Table"));
            descrPtr->timestampActionInfo.packetFormat =
                SKERNEL_TIMESTAMP_PACKET_FORMAT_PTPV1_E;
        }
        else
        {
            __LOG(("PTPv2 in PTP Local Action Table"));
            descrPtr->timestampActionInfo.packetFormat =
                SKERNEL_TIMESTAMP_PACKET_FORMAT_PTPV2_E;
        }

        /* PTP Local Action table */
        regAddr = SMEM_LION3_PTP_LOCAL_ACTION_TBL_MEM(devObjPtr,
                                                      descrPtr->ptpDomain,
                                                      descrPtr->ptpGtsInfo.gtsEntry.msgType,
                                                      egressPort);
        regPtr = smemMemGet(devObjPtr, regAddr);

        descrPtr->timestampActionInfo.action =
            (SMEM_CHT_IS_SIP5_20_GET(devObjPtr) == 0) ?
             SMEM_U32_GET_FIELD(*regPtr, 1, 4) :
             SMEM_U32_GET_FIELD(*regPtr, (descrPtr->ptpDomain*6) + 1, 4);

        descrPtr->timestampActionInfo.packetDispatchingEnable =
            (SMEM_CHT_IS_SIP5_20_GET(devObjPtr) == 0) ?
             SMEM_U32_GET_FIELD(*regPtr, 5, 1) :
             SMEM_U32_GET_FIELD(*regPtr, (descrPtr->ptpDomain*6) + 5, 1);

        descrPtr->timestampActionInfo.ingressLinkDelayEnable =
            (SMEM_CHT_IS_SIP5_20_GET(devObjPtr) == 0) ?
             SMEM_U32_GET_FIELD(*regPtr, 0, 1) :
             SMEM_U32_GET_FIELD(*regPtr, (descrPtr->ptpDomain*6), 1);

        doCount = GT_TRUE;
    }
    else
    {
        descrPtr->timestampActionInfo.action = SKERNEL_TIMESTAMP_ACTION_NONE_E;
    }

    if( descrPtr->timestampActionInfo.action == SKERNEL_TIMESTAMP_ACTION_DROP_E )
    {
        descrPtr->haAction.drop = SKERNEL_EXT_PKT_CMD_HARD_DROP_E;
        __LOG_PARAM(descrPtr->haAction.drop);
    }

    if( doCount == GT_TRUE )
    {
        descrPtr->timestampActionInfo.doCountIngressTimeStampEgressPort = GT_TRUE;
    }
    else
    {
        descrPtr->timestampActionInfo.doCountIngressTimeStampEgressPort = GT_FALSE;
    }

    /* Update MAC modification */
    if (descrPtr->timestampActionInfo.packetFormat == SKERNEL_TIMESTAMP_PACKET_FORMAT_PTPV2_E &&
        ((descrPtr->timestampActionInfo.action == SKERNEL_TIMESTAMP_ACTION_ADD_CORRECTED_TIME_E) ||
         (descrPtr->timestampActionInfo.action == SKERNEL_TIMESTAMP_ACTION_CAPTURE_ADD_CORRECTED_TIME_E)))
    {
        regAddr =  SMEM_LION3_PTP_TARGET_PORT_TBL_MEM(devObjPtr, egressPort);
        regPtr = smemMemGet(devObjPtr, regAddr);
        /* bit 320 - Trg Correction Field Piggyback En */
        if (SMEM_U32_GET_FIELD(regPtr[10], 0, 1) == 1)
        {
            descrPtr->macTimestampingEnable = 0;
            __LOG(("the MAC does not need to change the correction field\n"));
        }
        else
        {
            descrPtr->macTimestampingEnable = 1;
            __LOG(("the MAC needs to change the correction field\n"));
        }
    }
}

/**
* @internal snetLion3PtpPsuAndTsuProcess function
* @endinternal
*
* @brief   Timestamp action and queueing unit logic
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - egress port.
*                                      OUTPUT:
*                                      None
*
* @note It is safe to assume that the application must prevent a situation
*       where one packet is triggered for both timestamping and loss measurement
*       counter capture.
*
*/
GT_VOID snetLion3PtpPsuAndTsuProcess
(
IN SKERNEL_DEVICE_OBJECT * devObjPtr,
IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
IN GT_U32   egressPort
)
{

    DECLARE_FUNC_NAME(snetLion3PtpPsuAndTsuProcess);

    if(!SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        return;
    }
    __LOG(("PTP PSU & TSU processing started"));

    /* Action assignment */
    snetlion3PtpActionAssign(devObjPtr, descrPtr, egressPort);

    /* TOD Extraction */
    snetlion3TimestampTodExtraction(devObjPtr, descrPtr, egressPort);

    /* Capture to timestamp queues */
    snetlion3TimestampQueueCapture(devObjPtr, descrPtr, egressPort);

    /* Do timestamp action */
    snetlion3PtpTimestampingAction(devObjPtr, descrPtr, egressPort);

    /* Update 4B <reserved> field in PTP header */
    snetlion3PtpPiggybackApply(devObjPtr, descrPtr, egressPort);

    /* updating follow-up message TLV */
    snetSip6_30PtpUpdateFollowUpMessageTlv(devObjPtr, descrPtr);

    /* update UDP header+trailer; should be last operation */
    snetlion3PtpUdpChecksum(devObjPtr, descrPtr);
}

/**
* @internal ipHeaderDscpRemark function
* @endinternal
*
* @brief   do egress final IP header modifications for the DSCP (after EPCL and EPLR)
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] dscp                     - the new DSCP
* @param[in] l3StartOffsetPtr         - the start of the ip header
*
* @param[out] l3StartOffsetPtr
*/
static void ipHeaderDscpRemark
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32   dscp,
    IN GT_BIT   isIPv4,
    OUT GT_U8  *l3StartOffsetPtr
)
{
    DECLARE_FUNC_NAME(ipHeaderDscpRemark);

    GT_U32   oldDscp;
    GT_U32   ipCheckSum;

    if(isIPv4 == 1)
    {
        oldDscp = (l3StartOffsetPtr[1] >> 2);
        __LOG(("in the Ipv4 header modify DSCP from [%d] to [%d] \n" ,
            oldDscp,/*old DSCP*/
            dscp));/*new DSCP*/
        l3StartOffsetPtr[1] = (GT_U8)(dscp << 2);

        /* set check sum to 0 prior to re-calculation */
        l3StartOffsetPtr[10] = 0;
        l3StartOffsetPtr[11] = 0;

        /* calculate IP packet checksum */
        ipCheckSum = ipV4CheckSumCalc(l3StartOffsetPtr,
                                     (GT_U16)((l3StartOffsetPtr[0] & 0xF) * 4));
        __LOG(("re-calculate IPv4 header checksum [0x%4.4x] \n" , ipCheckSum));
        /* checksum of the IP header and IP options */
        l3StartOffsetPtr[10] = (GT_U8)(ipCheckSum >> 8);
        l3StartOffsetPtr[11] = (GT_U8)(ipCheckSum);
    }
    else
    {
        oldDscp = ((l3StartOffsetPtr[0] & 0xF) << 2) | /*4 MSBits*/
                   (l3StartOffsetPtr[1] >> 6);         /*2 LSBits*/
        __LOG(("in the Ipv6 header modify DSCP from [%d] to [%d] \n" ,
            oldDscp,/*old DSCP*/
            dscp));/*new DSCP*/

        /* set 4 MSB of DSCP to byte 0 LSB */
        l3StartOffsetPtr[0] &= 0xF0;
        l3StartOffsetPtr[0] |= ((GT_U8)(dscp >> 2));

        /* set 2 LSB of DSCP to byte 1 MSB */
        l3StartOffsetPtr[1] &= 0x3F;
        l3StartOffsetPtr[1] |= ((GT_U8)((dscp & 0x3) << 6));
    }
}

/**
* @internal finalIpHeaderDscpRemark function
* @endinternal
*
* @brief   do egress final IP header modifications for the DSCP (after EPCL and EPLR)
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - egress port.
* @param[in] dscp                     - the new DSCP
* @param[in] modifyDscp               - type of DSCP modification
*/
static void finalIpHeaderDscpRemark
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32  egressPort,
    IN GT_U32  dscp,
    EPCL_ACTION_MODIFY_DSCP_EXP_ENT modifyDscp
)
{
    DECLARE_FUNC_NAME(finalIpHeaderDscpRemark);

    GT_BIT  inner_isIp = 0;
    GT_BIT  inner_isIPv4 = 0;
    GT_BIT  outer_isIp = 0;
    GT_BIT  outer_isIPv4 = 0;
    GT_U8   *l3StartOffsetPtr = NULL; /* pointer to l3 header start (after the 2 bytes of 'ethertype')*/
    GT_U32  isIpv4 = 0;

    if(descrPtr->outerPacketType == SKERNEL_EGRESS_PACKET_IPV4_E)
    {
        /* in case of TS : the TS is ipv4 */
        /* in case of not TS : packet is ipv4 */
        outer_isIp = 1;
        outer_isIPv4 = 1;
    }
    else
    if (descrPtr->outerPacketType == SKERNEL_EGRESS_PACKET_IPV6_E)
    {
        /* in case of TS : the TS is ipv6 */
        /* in case of not TS : packet is ipv6 */
        outer_isIp = 1;
        outer_isIPv4 = 0;
    }

    if(descrPtr->tunnelStart)
    {
        /* in case of TS : the TS is not IP but passenger is ipv4/6 */
        /* in case of not TS : the packet is mpls/other (not ipv4/6) */
        inner_isIp = descrPtr->isIp;
        inner_isIPv4 = descrPtr->isIPv4;
    }

    __LOG_PARAM(outer_isIp      );
    __LOG_PARAM(outer_isIPv4    );
    __LOG_PARAM(inner_isIp      );
    __LOG_PARAM(inner_isIPv4    );

    if((inner_isIp ) &&     /*inner is ip*/
       (outer_isIp == 0) && /*outer not ip */
       modifyDscp == EPCL_ACTION_MODIFY_OUTER_DSCP_EXP_E)/*modify outer*/
    {
        /* only inner header exists */
        /* so treat the modify 'outer' as modify 'inner' */
        __LOG(("only inner header exists \n"
               "so treat the modify 'outer' as modify 'inner' \n"));
        l3StartOffsetPtr = descrPtr->haToEpclInfo.l3StartOffsetPtr;
        isIpv4 = inner_isIPv4;
    }
    else
    if((inner_isIp == 0) &&     /*inner not ip*/
       (outer_isIp ) &&         /*outer is ip */
       modifyDscp == EPCL_ACTION_MODIFY_INNER_DSCP_E)/*modify inner*/
    {
        /* only outer header exists */
        /* so treat the modify 'outer' as modify 'inner' */
        __LOG(("only outer header exists \n"
               "so treat the modify 'inner' as configuration ERROR !!! \n"));
        l3StartOffsetPtr = NULL;
    }
    else
    if (modifyDscp == EPCL_ACTION_MODIFY_INNER_DSCP_E &&
        inner_isIp)
    {
        /*inner*/
        __LOG(("The inner passenger is IP \n"));
        l3StartOffsetPtr = descrPtr->haToEpclInfo.l3StartOffsetPtr;
        isIpv4 = inner_isIPv4;
    }
    else
    if (modifyDscp == EPCL_ACTION_MODIFY_OUTER_DSCP_EXP_E &&
        outer_isIp)
    {
        /*outer*/
        if(descrPtr->tunnelStart)
        {
            __LOG(("The TS header is IP \n"));
            l3StartOffsetPtr = descrPtr->haToEpclInfo.tunnelStartL3StartOffsetPtr;
        }
        else
        {
            __LOG(("The packet header is IP \n"));
            l3StartOffsetPtr = descrPtr->haToEpclInfo.l3StartOffsetPtr;
        }
        isIpv4 = outer_isIPv4;
    }

    if(l3StartOffsetPtr == NULL)
    {
        __LOG(("Warning the IP header not exists from DSCP modifications \n"));

        return;
    }

    __LOG_PARAM(isIpv4);

    /* do the remark */
    ipHeaderDscpRemark(devObjPtr,descrPtr,dscp,isIpv4,l3StartOffsetPtr);
}

/**
* @internal ipHeaderEcnSet function
* @endinternal
*
* @brief   do egress final IP header modifications for the ECN (after EPCL and EPLR)
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*
* @param[out] l3StartOffsetPtr         - the start of the ip header
*/
static void ipHeaderEcnSet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    OUT GT_U8  *l3StartOffsetPtr
)
{
    DECLARE_FUNC_NAME(ipHeaderEcnSet);
    GT_U16 ipCheckSum;
    GT_U32 newEcn = 3;

    if(descrPtr ->l3NotValid)
    {
        __LOG(("Egress packet not subject to ECN modification because IP "
            "header is not valid \n"));
        return;
    }

    __LOG(("ECN is updated in the IP header to value[%d] \n", newEcn));

    if(descrPtr->isIPv4 == 1)
    {
        l3StartOffsetPtr[10] = 0;
        l3StartOffsetPtr[11] = 0;
        l3StartOffsetPtr[1] |= (GT_U8)(newEcn);
        ipCheckSum = ipV4CheckSumCalc(l3StartOffsetPtr,
            (GT_U16)((l3StartOffsetPtr[0] & 0xF) * 4));
        __LOG(("re-calculate IPv4 header checksum after ECN set [0x%4.4x] \n",
            ipCheckSum));
        l3StartOffsetPtr[10] = (GT_U8)(ipCheckSum >> 8);
        l3StartOffsetPtr[11] = (GT_U8)(ipCheckSum);
    }
    else
    {
        l3StartOffsetPtr[1] |= (GT_U8)(newEcn << 4);
    }
}

/**
* @internal finalMplsHeaderExpRemark function
* @endinternal
*
* @brief   do egress final EXP header modifications for the EXP (after EPCL and EPLR)
*         ALWAYS on the most outer label
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - egress port.
* @param[in] exp                      - the new EXP
*                                      doTunnelHeader  - indication to do tunnel start header modifications
*/
static void finalMplsHeaderExpRemark
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32  egressPort,
    IN GT_U32  exp
)
{
    DECLARE_FUNC_NAME(finalMplsHeaderExpRemark);

    GT_U8   *l3StartOffsetPtr; /* pointer to l3 header start (after the 2 bytes of 'ethertype')*/
    GT_U32  oldExp;

    if(descrPtr->haToEpclInfo.tunnelStartL3StartOffsetPtr &&
       descrPtr->outerPacketType == SKERNEL_EGRESS_PACKET_MPLS_E)
    {
        __LOG(("modify EXP on the tunnel start header \n"));
        l3StartOffsetPtr = descrPtr->haToEpclInfo.tunnelStartL3StartOffsetPtr;
    }
    else if(descrPtr->mpls)
    {
        __LOG(("Use EXP from the packet \n"));
        l3StartOffsetPtr = descrPtr->haToEpclInfo.l3StartOffsetPtr;
    }
    else
    {
        /* should not get here */
        __LOG(("None MPLS ... \n"));
        return;
    }

    oldExp = (l3StartOffsetPtr[2] >> 1) & 0x7;
    __LOG(("in the MPLS outer label modify EXP from [%d] to [%d] \n" ,
        oldExp,/*old EXP*/
        exp));/*new EXP*/

    l3StartOffsetPtr[2] &= 0xf1;/* clear bits 1..3 --> the EXP in the outer label */
    l3StartOffsetPtr[2] |= exp << 1;

    return;
}


/**
* @internal snetLion3EgressEportQosDpToCfiMap function
* @endinternal
*
* @brief   do egress final packet modifications (after EPCL and EPLR)
*         for DP-To-Cfi mapping.
*         sip5 only
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] vlanTagPtr               - pointer to the vlan tag (that needs update)
*
* @param[out] vlanTagPtr               - pointer to the modified vlan tag (set the cfi according to DP mapping)
*/
static void snetLion3EgressEportQosDpToCfiMap
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U8    *vlanTagPtr
)
{
    DECLARE_FUNC_NAME(snetLion3EgressEportQosDpToCfiMap);

    GT_U32  regValue;
    GT_U32  newCfiBit;
    GT_U32  oldCfiBit;

    smemRegGet(devObjPtr,SMEM_LION3_ERMRK_DP2CFI_TBL_REG(devObjPtr),&regValue);

    newCfiBit = SMEM_U32_GET_FIELD(regValue,descrPtr->dp,1);

    /* Set CFI bit in DSA tag */
    if((descrPtr->egrMarvellTagType != MTAG_TYPE_NONE_E) &&
            (descrPtr->egrMarvellTagType >= MTAG_EXTENDED_E))
    {
        /* CFI Bit (FORWARD/FROM_CPU/TO_ANALYZER) - WORD0[16]
         * CFI Bit (TO_CPU)                       - WORD1[30] */
        if(descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_CPU_E)
        {
            oldCfiBit = (vlanTagPtr[4] >> 6) & 1;
            vlanTagPtr[4] &= 0xBF;
            vlanTagPtr[4] |= (newCfiBit << 6);
        }
        else
        {
            oldCfiBit = (vlanTagPtr[1]) & 1;
            vlanTagPtr[1] &= 0xFE;
            vlanTagPtr[1] |= newCfiBit;
        }
    }
    /* Set CFI bit in VLAN TAG */
    else
    {
        /*the CFI bit is in byte 2 bit 4 */
        oldCfiBit = (vlanTagPtr[2] >> 4) & 1;/*get the old value*/
        vlanTagPtr[2] &= 0xEF;/*clear bit 4 */
        vlanTagPtr[2] |= (newCfiBit << 4);/*set bit 4 according to the new CFI value */
    }

    if(newCfiBit != oldCfiBit)
    {
        __LOG(("the DP to CFI : Set new CFI bit [%d] override old value[%d] \n",
               newCfiBit,oldCfiBit));
    }
    else
    {
        __LOG(("the DP to CFI : not changed the value \n"));
    }

}


/**
* @internal egressTmQMapper function
* @endinternal
*
* @brief   map PP parameters into TM parameters
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - egress port.
*                                       none.
*/
static void egressTmQMapper
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32  egressPort
)
{
    DECLARE_FUNC_NAME(egressTmQMapper);

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_TRAFFIC_MANAGER_QUEUE_MAPPER_E);

    __LOG(("TM Q mapper : demo : use 'hard coded' logic 3 bits TC + 8 bits local egress port \n"));

    /* assume tmTc is 1:1 to tc */
    descrPtr->trafficManagerTc = descrPtr->tc;
    /* dummy mapping that should match the value that the 'appDemo' should set */
    descrPtr->tmQueueId = (egressPort & 0xFF) << 3 | (descrPtr->trafficManagerTc & 0x7);

    __LOG_PARAM(descrPtr->trafficManagerTc);
    __LOG_PARAM(descrPtr->tmQueueId);
}

/**
* @internal egressTmDropUnit function
* @endinternal
*
* @brief   do TM 'drop query' and get 'drop response' from the TM unit (engine)
*         to know if pass/drop the packet
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - egress port.
* @param[in] frameDataPtr             - pointer to frame data
* @param[in] frameDataSize            - frame data size
*                                       do we need to drop the packet.
*
* @retval GT_TRUE                  - need to drop the packet.
* @retval GT_FALSE                 - packet can continue the egress pipe.
*/
static GT_BOOL egressTmDropUnit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32  egressPort,
    IN GT_U8   *frameDataPtr,
    IN GT_U32  frameDataSize
)
{
    DECLARE_FUNC_NAME(egressTmDropUnit);
    GT_U32  regAddr;/* register address */
    GT_U32  *memPtr;/*pointer to memory*/
    GT_U32  queueProfileID;
    GT_U32  dropMasking;
    SREMOTE_TM_PP_TO_TM_INPUT_DROP_INFO_STC  inDropInfo;
    SREMOTE_TM_PP_TO_TM_OUTPUT_DROP_INFO_STC outDropInfo;
    GT_BOOL dropCommand = GT_FALSE;

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_TRAFFIC_MANAGER_DROP_AND_STATISTICS_E);

    /*check if the egress port is subject to TM logic */
    /* macro already knows that 32 ports in register */
    regAddr = SMEM_BOBCAT2_TM_DROP_PORTS_INTERFACE_REG(devObjPtr,egressPort);
    /* 32 ports in register */
    smemRegFldGet(devObjPtr, regAddr,(egressPort % 32),1,&descrPtr->trafficManagerEnabled);

    __LOG_PARAM(descrPtr->trafficManagerEnabled);
    if(descrPtr->trafficManagerEnabled)
    {
        __LOG(("Port [%d] is connected to the MAC through the TM \n",
            egressPort));
    }
    else
    {
        /*Port is connected directly to the MAC*/

        /* do no more processing in this unit */
        return GT_FALSE;
    }


    if(0 == sRemoteTmUsed)
    {
        descrPtr->trafficManagerEnabled = 0;

        __LOG(("ERROR : simulation not do 'TM engine' logic ... must initiate remote TM simulator \n"));
        return GT_FALSE;
    }

    /* 16 profile in entry */
    regAddr = SMEM_BOBCAT2_TM_DROP_QUEUE_PROFILE_ID_TBL_MEM(devObjPtr, (descrPtr->tmQueueId/16));
    memPtr = smemMemGet(devObjPtr,regAddr);
    queueProfileID = snetFieldValueGet(memPtr,3*(descrPtr->tmQueueId%16) , 3);

    __LOG_PARAM(queueProfileID);

    regAddr = SMEM_BOBCAT2_TM_DROP_DROP_MASKING_TBL_MEM(devObjPtr, queueProfileID);
    memPtr = smemMemGet(devObjPtr,regAddr);
    dropMasking = *memPtr;

    __LOG_PARAM(dropMasking);

    memset(&inDropInfo,0,sizeof(inDropInfo));
    memset(&outDropInfo,0,sizeof(outDropInfo));

    /*  Packet CoS (Mapping the Traffic Manager TC to a packet CoS by using the
        Traffic Manager TC to CoS mapping*/
    /* macro already knows that 8 tmTc in register */
    regAddr = SMEM_BOBCAT2_TM_DROP_TM_TC_TO_TM_COS_REG(devObjPtr,descrPtr->trafficManagerTc);
    /* 3 bits cos per tmTc */
    smemRegFldGet(devObjPtr, regAddr,(descrPtr->trafficManagerTc % 8)*3,3,&inDropInfo.cos);

    inDropInfo.dropProbabilitySelect = 0;/*??*/
    /*Packet color (drop precedence that may be received as a result of Egress Policer operation)*/
    inDropInfo.color    = descrPtr->dp;
    /*The queue to which the packet was intended to be enqueued*/
    inDropInfo.queue    = descrPtr->tmQueueId;



    __LOG_PARAM(inDropInfo.dropProbabilitySelect);
    __LOG_PARAM(inDropInfo.cos);
    __LOG_PARAM(inDropInfo.color);
    __LOG_PARAM(inDropInfo.queue);

    /* send to TM query about the 'drop' for 'input info' and get reply with
       pass/drop + drop recommendations. */
    sRemoteTmPacketDropInfoGet(devObjPtr->deviceId,&inDropInfo,&outDropInfo);

    __LOG_PARAM(outDropInfo.queue                  );
    __LOG_PARAM(outDropInfo.tailQueueRecommendation);
    __LOG_PARAM(outDropInfo.wredQueueRecommendation);
    __LOG_PARAM(outDropInfo.tailAnodeRecommendation);
    __LOG_PARAM(outDropInfo.wredAnodeRecommendation);
    __LOG_PARAM(outDropInfo.tailBnodeRecommendation);
    __LOG_PARAM(outDropInfo.wredBnodeRecommendation);
    __LOG_PARAM(outDropInfo.tailCnodeRecommendation);
    __LOG_PARAM(outDropInfo.wredCnodeRecommendation);
    __LOG_PARAM(outDropInfo.tailPortRecommendation );
    __LOG_PARAM(outDropInfo.wredPortRecommendation );
    __LOG_PARAM(outDropInfo.bufferRsv              );
    __LOG_PARAM(outDropInfo.outOfRsv               );
    __LOG_PARAM(outDropInfo.dropProbability        );
    __LOG_PARAM(outDropInfo.port                   );
    __LOG_PARAM(outDropInfo.aNode                  );
    __LOG_PARAM(outDropInfo.bNode                  );
    __LOG_PARAM(outDropInfo.cNode                  );

    if(descrPtr->dp == 2)
    {
        /* RedPacketsDropMode -
            This mode defines whether to drop all RED packets or to drop RED packets
           according to the Traffic Manager recommendation */
        if(SMEM_U32_GET_FIELD(dropMasking , 0 , 1))
        {
            /*Drop RED packets according to TM response*/
        }
        else
        {
            /*Drop all RED packets*/
            __LOG(("RedPacketsDropMode : Drop all RED packets (we have descrPtr->dp == 2) \n"));
            dropCommand = GT_TRUE;
        }
    }

    if(dropCommand == GT_FALSE)
    {
        /* check drop recommendations */
        TM_DROP_CHECK_DROP_RECOMMENDATIONS_MAC(outDropInfo,tailQueueRecommendation,dropMasking,11,dropCommand);
        TM_DROP_CHECK_DROP_RECOMMENDATIONS_MAC(outDropInfo,wredQueueRecommendation,dropMasking,10,dropCommand);

        TM_DROP_CHECK_DROP_RECOMMENDATIONS_MAC(outDropInfo,tailAnodeRecommendation,dropMasking, 9,dropCommand);
        TM_DROP_CHECK_DROP_RECOMMENDATIONS_MAC(outDropInfo,wredAnodeRecommendation,dropMasking, 8,dropCommand);

        TM_DROP_CHECK_DROP_RECOMMENDATIONS_MAC(outDropInfo,tailBnodeRecommendation,dropMasking, 7,dropCommand);
        TM_DROP_CHECK_DROP_RECOMMENDATIONS_MAC(outDropInfo,wredBnodeRecommendation,dropMasking, 6,dropCommand);

        TM_DROP_CHECK_DROP_RECOMMENDATIONS_MAC(outDropInfo,tailCnodeRecommendation,dropMasking, 5,dropCommand);
        TM_DROP_CHECK_DROP_RECOMMENDATIONS_MAC(outDropInfo,wredCnodeRecommendation,dropMasking, 4,dropCommand);

        TM_DROP_CHECK_DROP_RECOMMENDATIONS_MAC(outDropInfo,tailPortRecommendation,dropMasking, 3,dropCommand);
        TM_DROP_CHECK_DROP_RECOMMENDATIONS_MAC(outDropInfo,wredPortRecommendation,dropMasking, 2,dropCommand);

        /*Indicates whether to use the OOR (Out of resources) for the dropping decision.*/
        TM_DROP_CHECK_DROP_RECOMMENDATIONS_MAC(outDropInfo,outOfRsv,dropMasking, 1,dropCommand);
    }

    /* save the egress port for later use (after packet returns from TM unit) */
    descrPtr->trafficManagerCos   = inDropInfo.cos;
    descrPtr->trafficManagerColor = inDropInfo.color;

    if(smainIsEmulateRemoteTm())
    {
        /* TRICK : use info of original egress port */
        __LOG(("smainIsEmulateRemoteTm() : TRICK : set trafficManagerFinalPort = egressPort[%d] \n",
            egressPort));
        descrPtr->trafficManagerFinalPort   = egressPort;
    }
    else
    {
        descrPtr->trafficManagerFinalPort   = outDropInfo.port;
    }

    __LOG_PARAM(dropCommand);
    __LOG_PARAM(descrPtr->trafficManagerFinalPort);

    if(egressPort != descrPtr->trafficManagerFinalPort)
    {
        __LOG(("Warning: Egress port is [%d] but TM calculated port [%d] \n",
            egressPort ,
            descrPtr->trafficManagerFinalPort));
    }

    return dropCommand;
}
/**
* @internal snetChtEgressFinalPacketAlteration function
* @endinternal
*
* @brief   do egress final packet modifications (after EPCL and EPLR)
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - egress port.
*/
void snetChtEgressFinalPacketAlteration
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32  egressPort
)
{
    DECLARE_FUNC_NAME(snetChtEgressFinalPacketAlteration);

    GT_U32  up=0,dscp=0,exp=0,vlanId=0;
    GT_BIT  modifyUp=0,modifyVid=0;
    EPCL_ACTION_MODIFY_DSCP_EXP_ENT modifyDscp = EPCL_ACTION_MODIFY_NO_DSCP_EXP_E;
    EPCL_ACTION_MODIFY_DSCP_EXP_ENT modifyExp = EPCL_ACTION_MODIFY_NO_DSCP_EXP_E;
    SNET_CHEETAH_AFTER_HA_COMMON_ACTION_STC eportQosInfo;
    SNET_CHEETAH_AFTER_HA_COMMON_ACTION_STC *eportQosInfoPtr;
    GT_BIT egressIsMpls = 0;/*indication that egress packet is MPLS */
    GT_BIT egressIsIp = 0;/*indication that egress packet is IP */
    GT_BIT in_IsMpls = 0;/*indication that egress packet passenger is MPLS */
    GT_BIT in_IsIp = 0;  /*indication that egress packet passenger is IP */

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_ERMRK_E);

    __LOG(("do egress final packet modifications (after EPCL and EPLR) - target global port[%d]\n",
        egressPort));

    if(descrPtr->outerPacketType == SKERNEL_EGRESS_PACKET_IPV4_E)
    {
        __LOG(("egress as ipv4 packet \n"));
        egressIsIp = 1;
    }
    else
    if(descrPtr->outerPacketType == SKERNEL_EGRESS_PACKET_IPV6_E)
    {
        __LOG(("egress as ipv6 packet \n"));
        egressIsIp = 1;
    }
    else
    if(descrPtr->outerPacketType == SKERNEL_EGRESS_PACKET_MPLS_E)
    {
        __LOG(("egress as mpls packet \n"));
        egressIsMpls = 1;
    }
    else
    {
        __LOG(("egress as non mpls/ip packet \n"));
    }

    if(descrPtr->tunnelStart)
    {
        if(descrPtr->mpls)
        {
            __LOG(("passenger is MPLS \n"));
            in_IsMpls = 1;
        }
        else
        if(descrPtr->isIp)
        {
            __LOG(("passenger is IP \n"));
            in_IsIp = 1;
        }
    }
    else
    {
        in_IsMpls = egressIsMpls;
        in_IsIp = egressIsIp;
    }

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr) == 0)
    {
        __LOG(("legacy device was not intended to modify anything in the TS (except MPLS-LSR)\n"));
        egressIsMpls = 0;
        egressIsIp = 0;

        /* NOTE: for MPLS LSR the packet ingress as MPLS (and added TS MPLS after TTI removed some of it ...)*/
        in_IsMpls = descrPtr->mpls;
        in_IsIp = descrPtr->isIp;
    }

    __LOG_PARAM(egressIsMpls);
    __LOG_PARAM(egressIsIp);
    __LOG_PARAM(in_IsMpls);
    __LOG_PARAM(in_IsIp);

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        eportQosInfoPtr = &eportQosInfo;
        memset(&eportQosInfo,0,sizeof(eportQosInfo));
        snetLion3EgressEportQosRemarkingInfoGet(devObjPtr,descrPtr,eportQosInfoPtr,
            egressIsMpls,egressIsIp,in_IsMpls,in_IsIp);
    }
    else
    {
        eportQosInfoPtr = NULL;
    }

    if(egressIsIp && descrPtr->markEcn)
    {
        __LOG(("Set ECN field of IP header \n"));
        ipHeaderEcnSet(devObjPtr, descrPtr,
            descrPtr->haToEpclInfo.l3StartOffsetPtr);
    }

    /* check for EPCL changes */
    if(descrPtr->haToEpclInfo.outerVlanTagPtr)
    {
        __LOG(("Check for modifications in the most OUTER Vlan tag \n"));

        if(eportQosInfoPtr && eportQosInfoPtr->modifyUp == EPCL_ACTION_MODIFY_OUTER_TAG_E)
        {
            /* the eport already calculated the final UP */
            __LOG(("Enable Modify UP on outer TAG after new UP mapping (by eport) \n"));
            up = eportQosInfoPtr->up;
            modifyUp = 1;
        }
        else
        {
            /* EPLR */
            if(descrPtr->eplrAction.modifyUp == EPCL_ACTION_MODIFY_OUTER_TAG_E)
            {
                    __LOG(("Enable Modify UP on outer TAG due to EPLR \n"));
                up = descrPtr->eplrAction.up;
                modifyUp = 1;
            }
            /* EPCL */
            else
            if(descrPtr->epclAction.modifyUp == EPCL_ACTION_MODIFY_OUTER_TAG_E)
            {
                __LOG(("Enable Modify UP on outer TAG due to EPCL \n"));
                up = descrPtr->epclAction.up;
                modifyUp = 1;
            }
        }

        /* EPCL */
        if(descrPtr->epclAction.modifyVid0 == EPCL_ACTION_MODIFY_OUTER_TAG_E)
        {
            __LOG(("Enable Modify VID on outer TAG due to EPCL \n"));
            modifyVid = 1;
            vlanId = descrPtr->epclAction.vid0;
        }

        MODIFY_VID_UP_MAC(descrPtr->haToEpclInfo.outerVlanTagPtr,
            modifyVid , vlanId ,  modifyUp ,  up);

        if(SMEM_CHT_IS_SIP5_GET(devObjPtr) &&
           descrPtr->haToEpclInfo.qos.egressDpToCfiMapEn)
        {
            /* do DP to CFI remap */
            snetLion3EgressEportQosDpToCfiMap(devObjPtr,descrPtr,
                descrPtr->haToEpclInfo.outerVlanTagPtr);

        }
    }

    /* only SIP5 can modify the vid0 of the passenger/packet (in case of TS)
       legacy devices can modify only 'outer tag' */
    if(descrPtr->haToEpclInfo.vlanTag0Ptr &&
        SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        __LOG(("Check for modifications in TAG0 Vlan tag \n"));
        modifyVid = 0;
        modifyUp  = 0;

        /* EPCL */
        if(descrPtr->epclAction.modifyUp == EPCL_ACTION_MODIFY_TAG0_E)
        {
            __LOG(("Enable Modify UP on tag0 due to EPCL \n"));
            up = descrPtr->epclAction.up;
            modifyUp = 1;
        }

        if(descrPtr->epclAction.modifyVid0 == EPCL_ACTION_MODIFY_TAG0_E)
        {
            __LOG(("Enable Modify VID on tag0 due to EPCL \n"));
            modifyVid = 1;
            vlanId = descrPtr->epclAction.vid0;
        }

        if(descrPtr->eplrAction.modifyUp == EPCL_ACTION_MODIFY_TAG0_E)
        {
            __LOG(("Enable Modify UP on tag0 due to EPLR \n"));
            up = descrPtr->eplrAction.up;
            modifyUp = 1;
        }

        MODIFY_VID_UP_MAC(descrPtr->haToEpclInfo.vlanTag0Ptr,
            modifyVid , vlanId ,  modifyUp ,  up);
    }

    /* only SIP5 can modify the vid1 of the passenger/packet (in case of TS)
       legacy devices can modify only 'outer tag' */
    if(descrPtr->haToEpclInfo.vlanTag1Ptr &&
        SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        __LOG(("Check for modifications in TAG1 Vlan tag \n"));

        modifyVid = 0;
        modifyUp  = 0;

        /* EPCL */
        /*Enable Modify UP1*/
        if(descrPtr->epclAction.modifyUp1)
        {
            __LOG(("Enable Modify UP1 on tag1 due to EPCL \n"));
            up = descrPtr->epclAction.up1;
            modifyUp = 1;
        }

        if(descrPtr->epclAction.vlan1Cmd)
        {
            __LOG(("Enable Modify VID1 on tag1 due to EPCL \n"));
            modifyVid = 1;
            vlanId = descrPtr->epclAction.vid1;
        }

        MODIFY_VID_UP_MAC(descrPtr->haToEpclInfo.vlanTag1Ptr,
            modifyVid , vlanId ,  modifyUp ,  up);
    }

    /* check for changes in the ip header of the passenger / TS */
    if(in_IsIp || egressIsIp)
    {
        __LOG(("Check for modifications of DSCP in IP HEADER \n"));

        if(eportQosInfoPtr && eportQosInfoPtr->modifyDscp)
        {
            /* the eport already calculated the final UP */
            __LOG(("Enable Modify DSCP after new DSCP mapping (by eport) \n"));
            dscp = eportQosInfoPtr->dscp;
            modifyDscp = eportQosInfoPtr->modifyDscp;
        }
        else
        {
            /* EPLR */
            if(descrPtr->eplrAction.modifyDscp)
            {
                __LOG(("Enable Modify DSCP due to EPLR \n"));
                dscp = descrPtr->eplrAction.dscp;
                modifyDscp = descrPtr->eplrAction.modifyDscp;
            }
            /* EPCL */
            else
        if(descrPtr->epclAction.modifyDscp)
        {
                __LOG(("Enable Modify DSCP due to EPCL \n"));
            dscp = descrPtr->epclAction.dscp;
                modifyDscp = descrPtr->epclAction.modifyDscp;
        }
        }

        if(modifyDscp != EPCL_ACTION_MODIFY_NO_DSCP_EXP_E)
        {
            finalIpHeaderDscpRemark(devObjPtr, descrPtr,
                egressPort,dscp,
                modifyDscp);
        }
        else
        {
            __LOG(("No DSCP remark needed \n"));
        }
    }

    if(egressIsMpls || in_IsMpls)
    {
        __LOG(("Check for modifications of EXP in MPLS most outer label \n"));

        if(eportQosInfoPtr && eportQosInfoPtr->modifyExp)
        {
            /* the eport already calculated the final exp */
            __LOG(("Enable Modify exp after new exp mapping (by eport) \n"));
            exp = eportQosInfoPtr->exp;
            modifyExp = eportQosInfoPtr->modifyExp;
        }
        else
        {
            /* EPLR */
            if(descrPtr->eplrAction.modifyExp)
            {
                __LOG(("Enable Modify EXP due to EPLR \n"));
                exp = descrPtr->eplrAction.exp;
                modifyExp = descrPtr->eplrAction.modifyExp;
            }
            /* EPCL */
            else
        if(descrPtr->epclAction.modifyExp)
        {
                __LOG(("Enable Modify EXP due to EPCL \n"));
            exp = descrPtr->epclAction.exp;
                modifyExp = descrPtr->epclAction.modifyExp;
        }
        }

        if(modifyExp == EPCL_ACTION_MODIFY_INNER_DSCP_E)
        {
            /* configuration error : ignore modification */
            __LOG(("configuration ERROR : try to modify 'inner MPLS' ignore modification \n"));
        }
        else
        if(modifyExp == EPCL_ACTION_MODIFY_OUTER_DSCP_EXP_E) /* modify EXP regardless to LSR ! */
        {
            finalMplsHeaderExpRemark(devObjPtr, descrPtr,
                    egressPort,exp);
        }
        else
        {
            __LOG(("No EXP remark needed \n"));
        }
    }

    if((descrPtr->egrMarvellTagType != MTAG_TYPE_NONE_E && descrPtr->egrMarvellTagType >= MTAG_EXTENDED_E) &&
        descrPtr->qos.egressExtendedMode )
    {
        __LOG(("egressExtendedMode : update DSA with DP[%d] \n",
            descrPtr->dp));
        /* we need to set DP and optional the override TC that in this mode replace the qosProfileField in the DSA */
        /* 1. the DP was unknown when the DSA was built (the EPLR is setting it, after the HA ...) */
        /* DP is in mtag[1],13,2 --> this is byte 6 bits 6..5 */
        descrPtr->haToEpclInfo.outerVlanTagPtr[6] &= 0x9f; /* clear bits 6..5 */
        descrPtr->haToEpclInfo.outerVlanTagPtr[6] |= descrPtr->dp << 5;

        /* 2. the TC may have been changed by the EPLR (the EPCL have no such capability */
        /* TC is in mtag[1],15,3 --> this is byte 6 bit 7 and byte 5 bits 1..0 */
        if(descrPtr->eplrAction.modifyTc)
        {
            __LOG(("egressExtendedMode : update DSA with TC[%d] \n",
                descrPtr->tc));

            descrPtr->haToEpclInfo.outerVlanTagPtr[6] &= 0x7f; /* clear bit 7 */
            descrPtr->haToEpclInfo.outerVlanTagPtr[6] |= (descrPtr->eplrAction.tc & 1) << 7;

            descrPtr->haToEpclInfo.outerVlanTagPtr[5] &= 0xFC; /* clear bits 6..5 */
            descrPtr->haToEpclInfo.outerVlanTagPtr[5] |= ((descrPtr->eplrAction.tc >> 1) & 3) << 0;
        }
    }

    if (descrPtr->haToEpclInfo.forcedFromCpuDsaPtr)
    {
        ASSERT_PTR(descrPtr->haToEpclInfo.outerVlanTagPtr);
        __LOG(("update forced FROM_CPU DSA vlan info from outer vlan tag \n"));

        up      = descrPtr->haToEpclInfo.outerVlanTagPtr[2] >> 5 & 0x7;
        vlanId  = ((descrPtr->haToEpclInfo.outerVlanTagPtr[2] & 0xf) << 8) |
                   (descrPtr->haToEpclInfo.outerVlanTagPtr[3]);

        MODIFY_VID_UP_MAC(descrPtr->haToEpclInfo.forcedFromCpuDsaPtr, 1, vlanId,  1,  up);
    }



    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* Perform OAM packet changes */
        __LOG(("Perform OAM packet changes"));
        snetLion2OamPsuPacketModify(devObjPtr,descrPtr);
        /* Perform PTP action, queuing and packet changes */
        __LOG(("Perform PTP action, queuing and packet changes"));
        snetLion3PtpPsuAndTsuProcess(devObjPtr, descrPtr, egressPort);
    }

}

/**
* @internal snetLion3EgressEportQosRemarkingInfoGet function
* @endinternal
*
* @brief   Lion3 Get EPort QOS rearming info
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressIsMpls             - indication that egress packet is MPLS
* @param[in] egressIsIp               - indication that egress packet is IP (ipv4/6)
* @param[in] in_IsMpls                -  indication that egress packet passenger is MPLS
* @param[in] in_IsIp                  -  indication that egress packet passenger is IP
*
* @param[out] eportQosInfoPtr          - pointer to final QOS remarking info
*/
static void snetLion3EgressEportQosRemarkingInfoGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    OUT SNET_CHEETAH_AFTER_HA_COMMON_ACTION_STC *eportQosInfoPtr,
    IN GT_BIT egressIsMpls,
    IN GT_BIT egressIsIp,
    IN GT_BIT in_IsMpls,
    IN GT_BIT in_IsIp
)
{
    DECLARE_FUNC_NAME(snetLion3EgressEportQosRemarkingInfoGet);

    GT_U32  qosMapTableIndex;/* index 0..11 of the 12 qos tables */
    GT_U32  qosMapTableLine;/* index with in QoS Map Table */
    GT_BIT  egressTcDpMapEn,egressUpMapEn,egressDscpMapEn,egressExpMapEn;
    GT_U32  tcDpCombination;
    GT_U32  *memPtr;
    GT_U32  startBit;
    GT_U32  upBeforeRemap,expBeforeRemap,dscpBeforeRemap;
    GT_U8   *l3StartOffsetPtr; /* pointer to l3 header start (after the 2 bytes of 'ethertype')*/

    qosMapTableIndex = descrPtr->haToEpclInfo.qos.qosMapTableIndex;

    if(qosMapTableIndex >= 12)
    {
        __LOG(("ERROR : qosMapTableIndex [%d] must be less than 12 \n",
            qosMapTableIndex));

        return;
    }

    egressExpMapEn  = descrPtr->haToEpclInfo.qos.egressExpMapEn;
    egressUpMapEn   = descrPtr->haToEpclInfo.qos.egressUpMapEn;
    egressDscpMapEn = descrPtr->haToEpclInfo.qos.egressDscpMapEn;
    egressTcDpMapEn = descrPtr->haToEpclInfo.qos.egressTcDpMapEn;

    if(egressExpMapEn && egressIsMpls == 0 && in_IsMpls == 0)
    {
        if(descrPtr->haToEpclInfo.tunnelStartL3StartOffsetPtr)
        {
            __LOG(("The TS nor the passenger are MPLS \n"));
        }
        else
        {
            __LOG(("The packet is not MPLS \n"));
        }

        __LOG(("ignore egressExpMapEn for non MPLS \n"));
        egressExpMapEn = 0;
    }

    if(egressDscpMapEn && (in_IsIp == 0 && egressIsIp == 0))
    {
        if(descrPtr->haToEpclInfo.tunnelStartL3StartOffsetPtr)
        {
            __LOG(("The TS nor the passenger are IP \n"));
        }
        else
        {
            __LOG(("The packet is not IP \n"));
        }

        __LOG(("ignore egressDscpMapEn for non IP \n"));
        egressDscpMapEn = 0;
    }

    __LOG_PARAM(qosMapTableIndex);
    __LOG_PARAM(egressTcDpMapEn);
    __LOG_PARAM(egressExpMapEn);
    __LOG_PARAM(egressDscpMapEn);
    __LOG_PARAM(egressUpMapEn);


    if(egressTcDpMapEn)
    {
        /*
            NOTE: this mode may ignore the EPCL/EPCL modifications of up,exp,dscp !
        */

        if(egressExpMapEn || egressDscpMapEn || egressUpMapEn)
        {
            __LOG(("Start Map {TC, DP} to {UP, EXP, DSCP} \n"));

            __LOG_PARAM(descrPtr->tc);
            __LOG_PARAM(descrPtr->dp);

            /*give priority to TC,DP mapping*/
            tcDpCombination = (descrPtr->tc << 2) + descrPtr->dp;
            qosMapTableLine = 192 + (16* qosMapTableIndex) + (tcDpCombination /2); /*2 entries in each line*/
            __LOG_PARAM(tcDpCombination);
            __LOG(("Use QoS Map Table index [%d] (index[%d] relative to index 192) \n",
                qosMapTableLine,
                (qosMapTableLine - 192)));

            memPtr = smemMemGet(devObjPtr,SMEM_LION3_QOS_MAP_TBL_MEM(devObjPtr, qosMapTableLine));

            startBit = 12 * (tcDpCombination & 1);/* bit 0 or 12 */ /*2 entries in each line*/

            if(egressDscpMapEn)
            {
                eportQosInfoPtr->dscp = SMEM_U32_GET_FIELD(memPtr[0] , startBit + 0 , 6);
                __LOG(("New DSCP[%d] \n",
                    eportQosInfoPtr->dscp));
            }

            if(egressExpMapEn)
            {
                eportQosInfoPtr->exp  = SMEM_U32_GET_FIELD(memPtr[0] , startBit + 6 , 3);
                __LOG(("New EXP[%d] \n",
                    eportQosInfoPtr->exp));
            }

            if(egressUpMapEn)
            {
                eportQosInfoPtr->up   = SMEM_U32_GET_FIELD(memPtr[0] , startBit + 9 , 3);
                __LOG(("New UP[%d] \n",
                    eportQosInfoPtr->up));
            }
        }
        else
        {
            __LOG(("NO need to Map {TC, DP} to {UP, EXP, DSCP} as none of those enabled \n"));
        }
    }
    else
    {
        if(egressUpMapEn)
        {
            if(descrPtr->eplrAction.modifyUp == EPCL_ACTION_MODIFY_OUTER_TAG_E)
            {
                __LOG((" remap UP from EPLR \n"));
                upBeforeRemap = descrPtr->eplrAction.up;
            }
            else if(descrPtr->epclAction.modifyUp == EPCL_ACTION_MODIFY_OUTER_TAG_E)
            {
                __LOG((" remap UP from EPCL \n"));
                upBeforeRemap = descrPtr->epclAction.up;
            }
            else
            {
                if(descrPtr->haToEpclInfo.outerVlanTagPtr)
                {
                    __LOG((" remap UP from outer tag (not EPCL,not EPLR) \n"));
                    upBeforeRemap = descrPtr->haToEpclInfo.outerVlanTagPtr[2] >> 5;
                }
                else
                {
                    /* this case is not relevant as the packet not hold outer tag */
                    __LOG(("Note : This case is not relevant as the packet not hold outer tag \n"));
                    __LOG((" remap UP from descriptor (not EPCL,not EPLR) \n"));
                    upBeforeRemap = descrPtr->up;
                }
            }

            memPtr = smemMemGet(devObjPtr,SMEM_LION3_ERMRK_UP2UP_MAP_TBL_REG(devObjPtr, qosMapTableIndex));
            startBit = upBeforeRemap * 3;
            eportQosInfoPtr->up = SMEM_U32_GET_FIELD(memPtr[0] , startBit , 3);

            __LOG((" remap UP[%d] to new UP[%d] \n",
                upBeforeRemap,eportQosInfoPtr->up));
        }

        if(egressDscpMapEn)
        {
            if(descrPtr->eplrAction.modifyDscp)
            {
                __LOG((" remap DSCP from EPLR \n"));
                dscpBeforeRemap = descrPtr->eplrAction.dscp;
            }
            else if(descrPtr->epclAction.modifyDscp)
            {
                __LOG((" remap DSCP from EPCL \n"));
                dscpBeforeRemap = descrPtr->epclAction.dscp;
            }
            else
            {
                __LOG((" remap (outer) DSCP from egress packet (not EPCL,not EPLR) \n"));

                if(descrPtr->haToEpclInfo.tunnelStartL3StartOffsetPtr && egressIsIp)
                {
                    __LOG(("Use DSCP from the tunnel start header \n"));
                    l3StartOffsetPtr = descrPtr->haToEpclInfo.tunnelStartL3StartOffsetPtr;
                }
                else if(in_IsIp)
                {
                    __LOG(("Use DSCP from the packet \n"));
                    l3StartOffsetPtr = descrPtr->haToEpclInfo.l3StartOffsetPtr;
                }
                else
                {
                    /* must not happen */
                    skernelFatalError("snetLion3EgressEportQosRemarkingInfoGet: no IP header \n");

                    return ;
                }

                if(devObjPtr->errata.tunnelStartQosRemarkUsePassenger &&
                  (descrPtr->haToEpclInfo.tunnelStartL3StartOffsetPtr &&  /*tunnel start*/
                   egressIsIp)) /* the tunnel is IP*/
                {
                    l3StartOffsetPtr = descrPtr->haToEpclInfo.l3StartOffsetPtr;
                    if(descrPtr->l2Encaps == SKERNEL_LLC_E && descrPtr->l2Valid)
                    {
                        __LOG(("WA for LLC packet , move the L3 pointer by one \n"));
                        l3StartOffsetPtr++;
                    }

                    if((in_IsIp == 1) && /* in HW (unlike GM) never considered ipv4 when 'in_IsIp = 0' */
                        /*emulate the errata : look at the L3 as 'ipv4/6'*/
                        (l3StartOffsetPtr[0] >> 4) == 4)/*IP version*/
                    {
#if 0 /* unreachable code due to (in_IsIp == 1) */
                        if((in_IsIp == 0)/*the passenger is not ip*/)
                        {
                            __LOG(("NOTE: ERROR : Errata : Ipv4 \n"));
                        }
                        else
#endif
                        {
                            __LOG(("NOTE: ERROR : Errata : use Ipv4 passenger DSCP !!! \n"));
                        }
                        /*IPv4*/
                        dscpBeforeRemap = l3StartOffsetPtr[1] >> 2;
                    }
                    else/*IPv6*/
                    {
                        if((in_IsIp == 0)/*the passenger is not ip*/)
                        {
                            __LOG(("NOTE: ERROR : Errata : Ipv6 \n"));
                        }
                        else
                        {
                            __LOG(("NOTE: ERROR : Errata : use Ipv6 passenger DSCP !!! \n"));
                        }
                        dscpBeforeRemap = ((l3StartOffsetPtr[0] & 0xF) << 2) |
                                          (l3StartOffsetPtr[1] >> 6);
                    }

                    if((in_IsIp == 0)/*the passenger is not ip*/)
                    {
                        __LOG(("NOTE: ERROR : Errata : egress packet is IP but passenger is not , use DSCP[0x%x] as if passenger is IPv4/6 for 'base DSCP' (instead of from TS) \n",
                            dscpBeforeRemap));
                    }
                    else
                    {
                        __LOG(("NOTE: ERROR : Errata : egress packet is IP and passenger is IP not , use DSCP[0x%x] from IP passenger as 'base DSCP' (instead of from TS) \n",
                            dscpBeforeRemap));
                    }
                }
                else
                if(descrPtr->outerPacketType == SKERNEL_EGRESS_PACKET_IPV4_E)
                {
                    __LOG(("egress packet it ipv4 \n"));
                    dscpBeforeRemap = l3StartOffsetPtr[1] >> 2;
                }
                else if(descrPtr->outerPacketType == SKERNEL_EGRESS_PACKET_IPV6_E)
                {
                    __LOG(("egress packet it ipv6 \n"));
                    /* get 4 MSB of DSCP to byte 0 LSB */
                    /* get 2 LSB of DSCP to byte 1 MSB */
                    dscpBeforeRemap = ((l3StartOffsetPtr[0] & 0xF) << 2) | /*4 MSBits*/
                                       (l3StartOffsetPtr[1] >> 6);         /*2 LSBits*/
                }
                else
                {
                    __LOG(("egress packet it NOT ip \n"));
                    __LOG(("Note : This case is not relevant as the packet not hold OUTER IP header \n"));
                    __LOG((" remap dscp from descriptor (not EPCL,not EPLR) \n"));
                    dscpBeforeRemap = descrPtr->dscp;
                }
            }

            qosMapTableLine = (16* qosMapTableIndex) + dscpBeforeRemap/4; /*4 entries in each line*/
            __LOG(("Use QoS Map Table index [%d] \n",
                qosMapTableLine));

            memPtr = smemMemGet(devObjPtr,SMEM_LION3_QOS_MAP_TBL_MEM(devObjPtr, qosMapTableLine));

            startBit = (dscpBeforeRemap % 4) * 6;/*4 entries in each line*/
            eportQosInfoPtr->dscp = SMEM_U32_GET_FIELD(memPtr[0] , startBit , 6);

            __LOG((" remap DSCP[%d] to new DSCP[%d] \n",
                dscpBeforeRemap,eportQosInfoPtr->dscp));
        }

        if(egressExpMapEn)
        {
            if(descrPtr->eplrAction.modifyExp)
            {
                __LOG((" remap EXP from EPLR \n"));
                expBeforeRemap = descrPtr->eplrAction.exp;
            }
            else if(descrPtr->epclAction.modifyExp)
            {
                __LOG((" remap EXP from EPCL \n"));
                expBeforeRemap = descrPtr->epclAction.exp;
            }
            else
            {
                __LOG((" remap EXP from egress pipe (not EPCL,not EPLR) \n"));

                if(descrPtr->haToEpclInfo.tunnelStartL3StartOffsetPtr && egressIsMpls)
                {
                    __LOG(("Use (outer) EXP from the tunnel start header \n"));
                    l3StartOffsetPtr = descrPtr->haToEpclInfo.tunnelStartL3StartOffsetPtr;
                }
                else if(in_IsMpls)
                {
                    __LOG(("Use (outer) EXP from the packet \n"));
                    l3StartOffsetPtr = descrPtr->haToEpclInfo.l3StartOffsetPtr;
                }
                else
                {
                    /* must not happen */
                    skernelFatalError("snetLion3EgressEportQosRemarkingInfoGet: no IP header \n");

                    return ;
                }

                if(devObjPtr->errata.tunnelStartQosRemarkUsePassenger &&
                  (descrPtr->haToEpclInfo.tunnelStartL3StartOffsetPtr &&
                    egressIsMpls && (in_IsMpls == 0)))
                {
                    __LOG(("NOTE: ERROR : Errata : egress packet is MPLS but passenger is not , use EXP = 0 as 'base EXP' \n"));
                    expBeforeRemap = 0;
                }
                else
                {
                    expBeforeRemap = ((l3StartOffsetPtr[2] >> 1) & 0x7);/* get bits 1..3 --> the EXP in the outer label (of TS) */
                }
            }


            memPtr = smemMemGet(devObjPtr,SMEM_LION3_ERMRK_EXP2EXP_MAP_TBL_REG(devObjPtr, qosMapTableIndex));
            startBit = expBeforeRemap * 3;
            eportQosInfoPtr->exp = SMEM_U32_GET_FIELD(memPtr[0] , startBit , 3);

            __LOG((" remap EXP[%d] to new EXP[%d] \n",
                expBeforeRemap,eportQosInfoPtr->exp));
        }
    }

    if(egressUpMapEn)
    {
        eportQosInfoPtr->modifyUp = EPCL_ACTION_MODIFY_OUTER_TAG_E;
    }

    if(egressExpMapEn)
    {
        eportQosInfoPtr->modifyExp = EPCL_ACTION_MODIFY_OUTER_DSCP_EXP_E;
    }

    if(egressDscpMapEn)
    {
        if(descrPtr->eplrAction.modifyDscp)
        {
            eportQosInfoPtr->modifyDscp = descrPtr->eplrAction.modifyDscp;
        }
        else if(descrPtr->epclAction.modifyDscp)
        {
            eportQosInfoPtr->modifyDscp = descrPtr->epclAction.modifyDscp;
        }
        else
        {
            eportQosInfoPtr->modifyDscp = EPCL_ACTION_MODIFY_OUTER_DSCP_EXP_E;/*outer(of TS)*/
        }
    }


}

/**
* @internal snetChtEgfTargetPortMapperToTqx function
* @endinternal
*
* @brief   SIP5 :
*         convert the target physical ports , to TXQ ports.
*         and get the hemisphere that need to handle the packet.
*         fill descrPtr->txqDestPorts[]
*         and descrPtr->bmpOfHemisphereMapperPtr
* @param[in] devObjPtr             - pointer to device object.
* @param[in] descrPtr             - Cht frame descriptor
* @param[in] destPorts[]          - number of egress port.
* @param[in] destPortsLb[]        - valuable only if destPort[N] == 1.
*                                   destPortsLb[N] format is:
*                                   bit [31] - if the packet destined to
*                                   port N is instead forwarded (1) or not (0)
*                                   to a loopback/service port.
*                                   bits[30:0] - a loopback/service port number.
*                                    Is valuable only if bit 31 is set.
*/
static GT_VOID snetChtEgfTargetPortMapperToTqx
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32               destPorts[],
    IN GT_U32               destPortsLb[]
)
{
    DECLARE_FUNC_NAME(snetChtEgfTargetPortMapperToTqx);

    GT_U32  portInx;
    GT_U32  physicalPort; /* physical port iterator */
    GT_U32  regAddr;/* register address */
    GT_U32 * regPtr;/* Register entry pointer */
    SKERNEL_DEVICE_OBJECT * targetHemisphereDevObjPtr;/* the device object that
                    represents the 'egress hemisphere' */
    GT_U32 hemispherePortGroups[2]={0,4};/*the port groups that represents the hemispheres */
    GT_U32 currentHemisphere = 0;/* current hemisphere 0 or 1*/

    if(!SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        /* get the bmp of convert port to hemisphere (0 or 1) */
        regAddr = SMEM_LION3_EGF_EFT_EGR_PORT_TO_HEMISPHERE_MAP_REG(devObjPtr);
        regPtr = smemMemGet(devObjPtr, regAddr);
        SKERNEL_FILL_PORTS_BITMAP_MAC(devObjPtr, descrPtr->bmpOfHemisphereMapperPtr, regPtr);
    }

    memset(descrPtr->txqDestPorts, 0, sizeof(descrPtr->txqDestPorts));
    for(portInx = 0 ; portInx < SKERNEL_DEV_EGRESS_MAX_PORT_CNS(devObjPtr); portInx++)
    {
        if(0 == destPorts[portInx])
        {
            continue;
        }

        physicalPort = CHOOSE_TRG_OR_LB_PORT_MAC(portInx, destPortsLb[portInx]);

        if(devObjPtr->portGroupSharedDevObjPtr)
        {
            currentHemisphere =
                SKERNEL_PORTS_BMP_IS_PORT_SET_MAC(descrPtr->bmpOfHemisphereMapperPtr,physicalPort) ?
                    1 : /* Hemisphere 1 */
                    0;  /* Hemisphere 0 */
            targetHemisphereDevObjPtr =
                devObjPtr->portGroupSharedDevObjPtr->coreDevInfoPtr[hemispherePortGroups[currentHemisphere]].devObjPtr;
        }
        else
        {
            targetHemisphereDevObjPtr = devObjPtr;
        }

        regPtr = smemMemGet(devObjPtr,
            SMEM_LION2_EGF_QAG_TARGET_PORT_MAPPER_TBL_MEM(targetHemisphereDevObjPtr,physicalPort));

        /* set the value of the TXQ port */
        descrPtr->txqDestPorts[physicalPort]  =  SMEM_U32_GET_FIELD(regPtr[0],0,SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 10 : 8);

        #if 0 /* allow 'tagging' of the editor to find the 'sip6_queue_group_index'
                when doing 'references' about it */
            for sip6 : SMEM_LION2_EGF_QAG_TARGET_PORT_MAPPER_TBL_MEM is the source for :
            descrPtr->egressPhysicalPortInfo.sip6_queue_group_index
        #endif /*0*/

        if(descrPtr->txqDestPorts[physicalPort] >= devObjPtr->txqNumPorts)
        {
            skernelFatalError("ERROR : txqPortNum[%d] >= txqNumPorts[%d] ... so not send packet to port [%d] \n",
                descrPtr->txqDestPorts[physicalPort],
                devObjPtr->txqNumPorts,
                physicalPort);

            continue;
        }

        if(devObjPtr->portGroupSharedDevObjPtr)
        {
            __LOG(("EGF : convert target physical port[%d] to TXQ port[%d] in hemisphere [%d] \n",
                physicalPort,
                descrPtr->txqDestPorts[physicalPort],
                currentHemisphere));
        }
        else
        {
            if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
            {
                __LOG(("EGF : convert target physical port[%d] to Group of Queues[%d] \n",
                    physicalPort,
                    descrPtr->txqDestPorts[physicalPort]));

            }
            else
            {
                __LOG(("EGF : convert target physical port[%d] to TXQ port[%d] \n",
                    physicalPort,
                    descrPtr->txqDestPorts[physicalPort]));
            }
        }
    }
}

/**
* @internal snetChtEgfQagVlanTag1Removed function
* @endinternal
*
* @brief   SIP5 :
*         check logic for 'Tag1 removal' .
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
*                                       the tag state of the egress port.
*                                       COMMENTS :
*/
static SKERNEL_FRAME_TR101_VLAN_EGR_TAG_STATE_ENT snetChtEgfQagVlanTag1Removed
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32               egressPort,
    IN SKERNEL_FRAME_TR101_VLAN_EGR_TAG_STATE_ENT  tagState
)
{
    DECLARE_FUNC_NAME(snetChtEgfQagVlanTag1Removed);

    GT_U32  regAddr;/* register address */
    GT_U32 * regPtr;/* Register entry pointer */
    GT_U32 value;
    GT_U32  evidNumBits;
    GT_U32  startBitAfterEvid;
    GT_U32  ingWithTag1;

    snetChtEgfEgressEPortEntryGet(devObjPtr,descrPtr,egressPort,GT_FALSE/*global port*/);

    if(SMEM_CHT_IS_SIP5_10_GET(devObjPtr) == 0)
    {
        evidNumBits = 16;
    }
    else
    {
        evidNumBits = 13;
    }

    ingWithTag1 = descrPtr->tagSrcTagged[SNET_CHT_TAG_1_INDEX_CNS];/* was according to TAG1_EXIST_MAC(descrPtr) */

    startBitAfterEvid = 12 + evidNumBits;

    if(descrPtr->eArchExtInfo.egfQagEgressEPortTablePtr != NULL)
    {
        /*Remove Vlan Tag 1 If Rx Without Tag 1*/
        value = snetFieldValueGet(descrPtr->eArchExtInfo.egfQagEgressEPortTablePtr, (startBitAfterEvid+12+3+3) , 1);

        if(value)
        {
            __LOG(("The target eport controls if to removed tag 1 or not (not global config) \n"));

            if(ingWithTag1)
            {
                __LOG(("per ePort : do not remove tag1 when came with tag 1 \n"));
                return tagState;
            }
            goto done_lbl;
        }
        else
        {
            __LOG(("The target eport allow global config to control if to removed tag 1 or not \n"));
        }

    }

    if(descrPtr->vid1 != 0)
    {
        __LOG(("not remove tag 0 when descrPtr->vid1 != 0 \n"));
        return tagState;
    }

    regAddr = SMEM_LION_TXQ_DISTR_GENERAL_CONF_REG(devObjPtr);
    regPtr = smemMemGet(devObjPtr, regAddr);
    /*Remove Vlan Tag1 If Zero*/
    value = SMEM_U32_GET_FIELD(*regPtr, 13, 2);
    if(value == 0)
    {
        __LOG(("global : do not remove vlan1 tag \n"));
        return tagState;
    }
    else if(value == 1)
    {
        /*remove vlan1 tag if vid1= 0x0*/
        __LOG(("global : remove vlan1 tag ONLY if vid1= 0x0 \n"));
    }
    else if(value == 2)
    {
        /*remove vlan1 tag is vid1=0x0 AND original packet arrived without tag1 (Untagged/Tag0/DSA)*/
        if(ingWithTag1)
        {
            __LOG(("global : do not remove tag1 when came with tag 1 \n"));
            return tagState;
        }
    }
    else
    if(value == 3 && SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
    {
        __LOG(("global :  Remove Tag1 if packet arrived without tag1 and Source ID bit<Src-ID bit - Add Tag1>=0 \n"));

        if(ingWithTag1)
        {
            __LOG(("global : do not remove tag1 when came with tag 1 \n"));
            return tagState;
        }

        value = SMEM_U32_GET_FIELD(*regPtr, 20, 4);

        __LOG(("<Src ID Bit Add Tag1> = [%d] \n",value));
        if(value < 12)
        {
            __LOG(("Source ID bit<Src-ID bit - Add Tag1> = [%d] \n",
                SMEM_U32_GET_FIELD(descrPtr->sstId,value,1)));
        }
        else
        {
            __LOG(("global : do not remove tag1 because <Src ID Bit Add Tag1> = [%d] >= 12 \n",
                value));
            return tagState;
        }

        /*RemoveNoTagAndNotAssigned; Remove Tag1 if packet arrived without tag1
           and Source ID bit<Src-ID bit - Add Tag1>=0 */
        if(SMEM_U32_GET_FIELD(descrPtr->sstId,value,1) == 1)
        {
            __LOG(("global : do not remove tag1 when Source ID bit<Src-ID bit - Add Tag1> == 1 \n"));
            return tagState;
        }

        __LOG(("Decision : Remove tag1 since came without it and Source ID bit<Src-ID bit - Add Tag1> == 0 \n"));
    }


done_lbl:

    switch(tagState)
    {
        case SKERNEL_FRAME_TR101_VLAN_EGR_TAG_TAG1_E:
            __LOG(("Tag1 Removed : changed from 'tag 1' to 'untagged' \n"));
            tagState = SKERNEL_FRAME_TR101_VLAN_EGR_TAG_UNTAGGED_E;
            break;
        case SKERNEL_FRAME_TR101_VLAN_EGR_TAG_OUT_TAG0_IN_TAG1_E:
            __LOG(("Tag1 Removed : changed from 'outer tag 0 inner tag 1' to 'tag 0' \n"));
            tagState = SKERNEL_FRAME_TR101_VLAN_EGR_TAG_TAG0_E;
            break;
        case SKERNEL_FRAME_TR101_VLAN_EGR_TAG_OUT_TAG1_IN_TAG0_E:
            __LOG(("Tag1 Removed : changed from 'outer tag 1 inner tag 0' to 'tag 0' \n"));
            tagState = SKERNEL_FRAME_TR101_VLAN_EGR_TAG_TAG0_E;
            break;
        default:
            /* no modification needed */
            __LOG(("Tag1 not exists on egress packet -- no modification needed \n"));
            break;
    }


    return tagState;
}

/**
* @internal snetSip6_30EgfQagVlanTag0Removed function
* @endinternal
*
* @brief   SIP6_30 :
*         check logic for 'Tag0 removal' .
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
*
* return :the tag state of the egress port.
*
* COMMENTS :
*/
static SKERNEL_FRAME_TR101_VLAN_EGR_TAG_STATE_ENT snetSip6_30EgfQagVlanTag0Removed
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32               egressPort,
    IN SKERNEL_FRAME_TR101_VLAN_EGR_TAG_STATE_ENT  tagState
)
{
    DECLARE_FUNC_NAME(snetSip6_30EgfQagVlanTag0Removed);

    GT_U32 value;

    snetChtEgfEgressEPortEntryGet(devObjPtr,descrPtr,egressPort,GT_FALSE/*global port*/);
    if(descrPtr->eArchExtInfo.egfQagEgressEPortTablePtr == NULL)
    {
        return tagState;
    }
    /* Remove Vlan Tag 0 If Rx Without Tag 0 */
    value = snetFieldValueGet(descrPtr->eArchExtInfo.egfQagEgressEPortTablePtr, 46 , 1);
    if(value == 0)
    {
        __LOG(("The target eport mode : Don't remove Vlan Tag 0 If Rx Without Tag 0 \n"));
        return tagState;
    }
    __LOG(("The target eport mode : Remove Vlan Tag 0 If Rx Without Tag 0 \n"));

    if(descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_FROM_CPU_E &&
       descrPtr->useVidx == 0 &&
       tagState == SKERNEL_FRAME_TR101_VLAN_EGR_TAG_TAG0_E)
    {
        /* there is a 'bug' in WM that the descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS]
            is valid also when the incoming packet is with DSA 'from_CPU' and use_vidx=0.
            in this DSA there is no 'srcTagged' but only 'trgTagged'

            so here the WM need to behave like GM and HW , that the trgTagged is set to 0 for suck packet.
            due to jira : EGF-1210 : Remove VLAN Tag 0 If Rx Without Tag 0 is applied to From CPU UC packets
        */
        __LOG(("WARNING : Remove VLAN Tag 0 If Rx Without Tag 0 is applied to From CPU UC packets (although should not!) \n"));
        descrPtr->trgTagged = 0;

        /* the 'fix' for this JIRA should be :
        if(descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_FROM_CPU_E &&
           descrPtr->useVidx == 0)
        {
            __LOG(("Remove VLAN Tag 0 If Rx Without Tag 0 is NOT applied on 'From CPU' UC packets \n"));
            return tagState;
        }
        */
    }
    else
    /*descrPtr->origSrcTagged*/
    if(descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS])/* was according to TAG0_EXIST_MAC(descrPtr) */
    {
        __LOG(("per ePort : do not remove tag0 since came with tag 0 \n"));
        return tagState;
    }

    switch(tagState)
    {
        case SKERNEL_FRAME_TR101_VLAN_EGR_TAG_TAG0_E:
            __LOG(("Tag0 Removed : changed from 'tag 0' to 'untagged' \n"));
            tagState = SKERNEL_FRAME_TR101_VLAN_EGR_TAG_UNTAGGED_E;
            break;
        case SKERNEL_FRAME_TR101_VLAN_EGR_TAG_OUT_TAG0_IN_TAG1_E:
            __LOG(("Tag0 Removed : changed from 'outer tag 0 inner tag 1' to 'tag 1' \n"));
            tagState = SKERNEL_FRAME_TR101_VLAN_EGR_TAG_TAG1_E;
            break;
        case SKERNEL_FRAME_TR101_VLAN_EGR_TAG_OUT_TAG1_IN_TAG0_E:
            __LOG(("Tag0 Removed : changed from 'outer tag 1 inner tag 0' to 'tag 1' \n"));
            tagState = SKERNEL_FRAME_TR101_VLAN_EGR_TAG_TAG1_E;
            break;
        default:
            /* no modification needed */
            __LOG(("Tag0 not exists on egress packet -- no modification needed \n"));
            break;
    }

    return tagState;
}

/**
* @internal snetChtEgfQagVlanTagModeGet function
* @endinternal
*
* @brief   SIP5 :
*         get the egress tag mode for the packet that egress the port.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
*                                       the tag state of the egress port.
*                                       COMMENTS :
*/
GT_U32 snetChtEgfQagVlanTagModeGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32               egressPort
)
{
    DECLARE_FUNC_NAME(snetChtEgfQagVlanTagModeGet);

    GT_U32  tagState; /* the egress tag state */
    GT_U32  origTagState;/* the egress tag state - before remove tag1 if needed*/
    GT_U32  regAddr;/* register address */
    GT_U32 * regPtr;/* Register entry pointer */
    GT_U32 index;/* index to the eVLAN Descriptor Assignment Attributes Table */
    GT_U32 value;
    GT_U32 memoryIndex;
    GT_U32 bitOffset;
    GT_U32 uMtagCmd          = descrPtr->outGoingMtagCmd;

    /*if ((GET_PRC->GetDescIn("outgoing_mtag_cmd") == MTAG_CMD_TO_TARGET_SNIFFER) ||
          (GET_PRC->GetDescIn("outgoing_mtag_cmd") == MTAG_CMD_TO_CPU) ||
          ((GET_PRC->GetDescIn("outgoing_mtag_cmd") == MTAG_CMD_FROM_CPU) &&
           (GET_PRC->GetDescIn("use_vidx") == 0x0))) {
        uEgressTagState = GET_PRC->GetDescIn("egress_tag_state");
      }
    */
    if(uMtagCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E ||
       uMtagCmd == SKERNEL_MTAG_CMD_TO_CPU_E ||
       (uMtagCmd == SKERNEL_MTAG_CMD_FROM_CPU_E && descrPtr->useVidx == 0))
    {
        tagState = descrPtr->trgTagged;
        __LOG(("'control packet' keep tag state by EQ from 'ingress' <descrPtr->trgTagged> the egress tag state [%d]  \n",
            tagState));

        goto done_lbl;
    }

    snetChtEgfEgressEPortEntryGet(devObjPtr,descrPtr,egressPort,GT_FALSE/*global port*/);

    if(descrPtr->eArchExtInfo.egfQagEgressEPortTablePtr != NULL)
    {
        /*Eport Tag State Mode*/
        if(snetFieldValueGet(descrPtr->eArchExtInfo.egfQagEgressEPortTablePtr, 4 , 1))
        {
            tagState = snetFieldValueGet(descrPtr->eArchExtInfo.egfQagEgressEPortTablePtr, 5 , 3);
            __LOG(("Take the egress ePort tag state [%d] from <Eport Tag State> \n",
                tagState));

            goto done_lbl;

        }

        __LOG(("Take the egress Port tag state from the eVLAN entry <Tag State> \n"));
    }

    if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
    {
        regAddr = SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTE_TBL_MEM(devObjPtr,egressPort);
        regPtr = smemMemGet(devObjPtr, regAddr);

        value = SMEM_SIP5_20_EGF_QAG_PORT_TARGET_FIELD_GET(devObjPtr,regPtr,egressPort,
            SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_USE_VLAN_TAG_1_FOR_TAG_STATE_E);
    }
    else
    {
        regAddr = SMEM_LION3_EGF_QAG_USE_VLAN_TAG_1_FOR_TAG_STATE_REG(devObjPtr);
        regPtr = smemMemGet(devObjPtr, regAddr);
        value = snetFieldValueGet(regPtr,egressPort,1);
    }

    if(value)
    {
        index = descrPtr->vid1;
        __LOG(("vlan mode : use vid1 as index [0x%4.4x] \n",
            index));
    }
    else
    {
        index = descrPtr->eVid;
        __LOG(("vlan mode : use eVid as index [0x%4.4x] \n",
            index));
    }

    if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
    {
        #define NUM_PORT_PER_LINE_CNS   16
        GT_U32  vlanNumLinesPerEntry = devObjPtr->limitedResources.phyPort / NUM_PORT_PER_LINE_CNS;

        /* only 16 ports per entry , but we have 128 ports in vlan  */
        memoryIndex = (index * vlanNumLinesPerEntry) + (egressPort / NUM_PORT_PER_LINE_CNS);
        bitOffset = ((egressPort % NUM_PORT_PER_LINE_CNS) * 3);
    }
    else
    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        GT_U32  numEVlans =
            descrPtr->tableAccessMode.EGF_SHT_TableMode == 0 ?
                devObjPtr->limitedResources.eVid :
                devObjPtr->limitedResources.eVid / (1 << (descrPtr->tableAccessMode.EGF_SHT_TableMode-1));
        GT_U32 vlanNumLinesPerEntry = descrPtr->tableAccessMode.EGF_SHT_TableMode == 0 ? 1 :
                                       (1 << (descrPtr->tableAccessMode.EGF_SHT_TableMode - 1));
        GT_U32  numPortsPerVlan = 64 << descrPtr->tableAccessMode.EGF_SHT_TableMode;
        SKERNEL_HALF_TABLE_MODE_VIOLATION_CHECK_AND_REPORT_MAC(devObjPtr,descrPtr,"egress vlan tag state",
            index,index,numEVlans,
            vlanNumLinesPerEntry,3);

        /* only 16 ports per entry , but we have 128 ports in vlan  */
        memoryIndex = (index * 8 * numPortsPerVlan / 128) + (egressPort / 16);
        bitOffset = ((egressPort % 16) * 3);

    }
    else
    if(SMEM_CHT_IS_SIP5_25_GET(devObjPtr))
    {
        /* only 16 ports per entry , but we have 128 ports in vlan  */
        memoryIndex = (index * 8) + (egressPort / 16);
        bitOffset = ((egressPort % 16) * 3);
    }
    else
    if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
    {
        if(descrPtr->tableAccessMode.EGF_QAG_TableMode)
        {
            SKERNEL_HALF_TABLE_MODE_VIOLATION_CHECK_AND_REPORT_MAC(devObjPtr,descrPtr,"egress vlan tag state",
                index,index,
                (GT_U32)(1 << (devObjPtr->flexFieldNumBitsSupport.eVid)),
                2,0);

            /* only 16 ports per entry , but we have 512=(32*16) ports in vlan */
            memoryIndex = (index * 32) + (egressPort /  16);
            bitOffset = ((egressPort % 16) * 3);
        }
        else
        {
            /* only 16 ports per entry , but we have 256 ports in vlan */
            memoryIndex = (index * 16) + (egressPort / 16);
            bitOffset = ((egressPort % 16) * 3);
        }
    }
    else
    {
        memoryIndex = index;
        bitOffset = (egressPort*3);
    }

    regAddr = SMEM_LION2_EGF_QAG_EVLAN_DESCRIPTOR_ASSIGNMENT_ATTRIBUTES_TBL_MEM(devObjPtr,memoryIndex);
    regPtr = smemMemGet(devObjPtr, regAddr);

    /* each port hold 3 bits tag state */
    tagState =  snetFieldValueGet(regPtr,bitOffset,3);

    /* return the tag state of the port */
    __LOG(("Egress tag state[%d] for egress port[%d] from vlan entry index[0x%4.4x] \n",
        tagState ,
        egressPort,
        index));

done_lbl:

    if( tagState >= SKERNEL_FRAME_TR101_VLAN_EGR_TAG_PUSH_TAG0_E )
    {
        __LOG(("Push, Pop & DoNotModify tag States are not effected by Tag1 removal logic"));

        if ( SKERNEL_FRAME_TR101_VLAN_EGR_TAG_DO_NOT_MODIFIED_E == tagState )
        {
            if(0 == descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS] &&
               0 == descrPtr->tagSrcTagged[SNET_CHT_TAG_1_INDEX_CNS])
            {
                __LOG(("Packet originally ingress without TAG0 without TAG1 \n"));
                tagState = SKERNEL_FRAME_TR101_VLAN_INGR_TAG_UNTAGGED_E;
            }
            else
            if(0 == descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS] &&
               1 == descrPtr->tagSrcTagged[SNET_CHT_TAG_1_INDEX_CNS])
            {
                __LOG(("Packet originally ingress without TAG0 with    TAG1 \n"));
                tagState = SKERNEL_FRAME_TR101_VLAN_INGR_TAG_TAG1_E;
            }
            else
            if(1 == descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS] &&
               0 == descrPtr->tagSrcTagged[SNET_CHT_TAG_1_INDEX_CNS])
            {
                __LOG(("Packet originally ingress with    TAG0 without TAG1 \n"));
                tagState = SKERNEL_FRAME_TR101_VLAN_INGR_TAG_TAG0_E;
            }
            else /*
            if(1 == descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS] &&
               1 == descrPtr->tagSrcTagged[SNET_CHT_TAG_1_INDEX_CNS]) */
            {
                if(descrPtr->tag0IsOuterTag)
                {
                    __LOG(("Packet originally ingress with outer TAG0 with inner TAG1 \n"));
                    tagState = SKERNEL_FRAME_TR101_VLAN_INGR_TAG_OUT_TAG0_IN_TAG1_E;
                }
                else
                {
                    __LOG(("Packet originally ingress with outer TAG1 with inner TAG0 \n"));
                    tagState = SKERNEL_FRAME_TR101_VLAN_INGR_TAG_OUT_TAG1_IN_TAG0_E;
                }
            }

            /* tag state configured to DoNotModify thus setting actual egress Tag State to be the ingress one */
            __LOG(("Do Not Modify : Setting actual Egress tag state[%d] for egress port[%d] since configured as DoNotModify\n",
                   tagState ,
                   egressPort));
        }
    }
    else
    {
        origTagState = tagState;

        /* check if the tag changes due to logic for 'Tag1 removal'*/
        tagState =
            snetChtEgfQagVlanTag1Removed(devObjPtr,descrPtr,egressPort,tagState);

        if(origTagState != tagState)
        {
            __LOG(("Egress tag state changed from[%d] to [%d] after 'Tag1 removal' logic \n",
                   origTagState,tagState));
        }


        if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
        {
            origTagState = tagState;
            /* check if the tag changes due to logic for 'Tag0 removal'*/
            tagState =
                snetSip6_30EgfQagVlanTag0Removed(devObjPtr,descrPtr,egressPort,tagState);

            if(origTagState != tagState)
            {
                __LOG(("Egress tag state changed from[%d] to [%d] after 'Tag0 removal' logic \n",
                       origTagState,tagState));
            }
        }

    }


    /* return the tag state of the port */
    return tagState;
}

static void snetSip6QueueRangeCheck(

    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32               queueOffset
)
{
    QUEUE_GROUP_MAP_ENTRY_STC queueMapInfo;
    GT_U32  localDma,sdqLocalQueue;
    GT_U32  sdqIndex;
    GT_U32  regAddr,regValue;
    GT_U32  minValue,maxValue;
    GT_U32 dmaEnable;

    DECLARE_FUNC_NAME(snetSip6QueueRangeCheck);

    /*1.Get the  port*/
    snetFalconTxqPdxQueueGroupMap(devObjPtr,descrPtr,&queueMapInfo);

    sdqIndex = queueMapInfo.queue_pds_index;
    localDma = queueMapInfo.dp_core_local_trg_port;

    /*2.Get  number of queues for port*/
     regAddr = SMEM_SIP6_TXQ_SDQ_GLOBAL_PORT_ENABLE_REG(devObjPtr,sdqIndex,localDma);
     smemRegFldGet(devObjPtr,regAddr,0,1,&dmaEnable);

     if(0 == dmaEnable)
     {
         __LOG(("NOTE: local DMA[%d] is DISABLED \n",
             localDma));
         /* if DMA is disabled no point to check ,it will be thrown anyway*/
         return;
     }


     regAddr = SMEM_SIP6_TXQ_SDQ_PORT_CONFIG_PORT_RANGE_LOW_REG (devObjPtr,sdqIndex,localDma);
     smemRegGet(devObjPtr,regAddr,&regValue);
     minValue = SMEM_U32_GET_FIELD(regValue,0,9);
     regAddr = SMEM_SIP6_TXQ_SDQ_PORT_CONFIG_PORT_RANGE_HIGH_REG(devObjPtr,sdqIndex,localDma);
     smemRegGet(devObjPtr,regAddr,&regValue);
     maxValue = SMEM_U32_GET_FIELD(regValue,0,9);

     sdqLocalQueue = minValue+queueOffset;

     if(sdqLocalQueue > maxValue)
     {
         __LOG(("Check queue offset in range - failed!!! Descriptor offset  %d > Max port offset %d \n",
             sdqLocalQueue , maxValue));
     }
     else
     {
         __LOG(("Check queue offset in range - pass.\n",
             sdqLocalQueue , maxValue));
     }

}


/**
* @internal snetSip5_20EgfQagTcDpRemap function
* @endinternal
*
* @brief   SIP5_20 :
*         set TC and DP Remapping , for all traffic.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
* @param[in] egressPort               - output port
*                                       COMMENTS :
*/
void snetSip5_20EgfQagTcDpRemap(

    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32               egressPort
)
{
    DECLARE_FUNC_NAME(snetSip5_20EgfQagTcDpRemap);

    GT_U32  *memPtr;                /* pointer to memory */
    GT_U32   regAddr;              /* register address */
    GT_U32   tcDpIndex;            /* index for tc and dp remapping tables*/
    GT_U32 regOffset;
    GT_U32 value;

    GT_U32 uSrcPort          = descrPtr->localDevSrcPort;/* GET_PRC->GetDescIn("local_dev_src_port");*/
    GT_U32 uTrgPhyPort       ;
    GT_U32 mCast             ;
    GT_U32 uMtagCmd          = descrPtr-> outGoingMtagCmd;/*GET_PRC->GetDescIn("outgoing_mtag_cmd");*/
    GT_U32 uNwTc             = descrPtr->tc;/*GetNewTc();*/
    GT_U32 uNwDp             = descrPtr->dp;/*GET_PRC->GetDescIn("dp");*/
    GT_U32 uTrgPortTcProfile ;
    GT_U32 uSrcPortTcProfile ;
    GT_U32 uPrioTableAccessMode = 0;
    GT_U32 vlanQOffset = 0;
    GT_U32 remapBits;

    __LOG(("do {tc,dp} remap on egress port[%d] \n",egressPort));

    snetChtEgfEgressEPortEntryGet(devObjPtr,descrPtr,egressPort,GT_FALSE);/*global port*/
    /* egfQagEgressEPortTable_index was set by snetChtEgfEgressEPortEntryGet() */
    uTrgPhyPort = egressPort;/*(GetTrgPort() != QAG_NUM_OF_NULL_DIST_LIST) ? GetTrgPort() : 0x0;*/

    /*(((GET_PRC->GetDescIn("outgoing_mtag_cmd") != MTAG_CMD_ TO_CPU) && (GET_PRC->GetDescIn("use_vidx") == 0x1)) || (GET_PRC->GetDescIn("rep") == 0x1))*/
    if(descrPtr->isMultiTargetReplication)
    {
        __LOG((" replication treated as 'multi destination' \n"));
        mCast = 1;
    }
    else
    if(descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_CPU_E)
    {
        __LOG((" FROM_CPU treated as 'single destination' (regardless to useVidx) \n"));
        mCast = 0;
    }
    else
    if(descrPtr->useVidx == 1)
    {
        __LOG((" useVidx == 1 treated as 'multi destination' \n"));
        mCast = 1;
    }
    else
    {
        __LOG((" useVidx == 0 treated as 'single destination' \n"));
        mCast = 0;
    }

    regAddr = SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTE_TBL_MEM(devObjPtr,egressPort);
    memPtr = smemMemGet(devObjPtr, regAddr);
    /*GET_QAG_TABLE->Get(PORT_TARGET_ATTRIBUTES_TABLE, uTrgPhyPort, "target_physical_port_tc_profile");*/
    uTrgPortTcProfile = SMEM_SIP5_20_EGF_QAG_PORT_TARGET_FIELD_GET(devObjPtr,memPtr,uTrgPhyPort,
    SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_TARGET_PHYSICAL_PORT_TC_PROFILE_E);

    regAddr = SMEM_SIP5_20_EGF_QAG_PORT_SOURCE_ATTRIBUTE_TBL_MEM(devObjPtr,uSrcPort);
    memPtr = smemMemGet(devObjPtr, regAddr);
    /*Source Physical Port TC Profile*/
    /*GET_QAG_TABLE->Get(PORT_SOURCE_ATTRIBUTES_TABLE, uSrcPort, "source_physical_port_tc_profile");*/
    uSrcPortTcProfile = snetFieldValueGet(memPtr, 2, 2);
    __LOG_PARAM_WITH_NAME("Source_Physical_Port_TC_Profile :",uTrgPortTcProfile);

    __LOG_PARAM(uNwTc);
    __LOG_PARAM(uNwDp);
    __LOG_PARAM(uMtagCmd);
    __LOG_PARAM(mCast);
    __LOG_PARAM(uTrgPortTcProfile);
    __LOG_PARAM(uSrcPortTcProfile);

    regAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.distributor.distributorGeneralConfigs;
    regOffset = 18;

    /*
    write to the register :mode 1
    */
    if (SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        smemRegFldGet(devObjPtr, regAddr, regOffset, 2, &value);
        uPrioTableAccessMode = value;
    }

/* Setting Fabric Priority Mapping.
  unsigned uHash = 0x0;

  setbits(uHash, 0, uSrcPortTcProfile, 2);
  setbits(uHash, 2, uTrgPortTcProfile, 2);
  setbits(uHash, 4, uCast, 1);
  setbits(uHash, 5, uMtagCmd, 2);
  setbits(uHash, 7, uNwDp, 2);
  setbits(uHash, 9, uNwTc, 3);
*/
    switch (uPrioTableAccessMode)
    {
    case SNET_CHT_PRIO_TABLE_ACCESS_MODE_2B_SRC_2B_TRG_E:
        tcDpIndex = uNwTc << 9 |
                uNwDp << 7 |
                uMtagCmd << 5 |
                mCast << 4 |
                uTrgPortTcProfile << 2 |
                uSrcPortTcProfile ;
            break;

    case SNET_CHT_PRIO_TABLE_ACCESS_MODE_4B_TRG:
        tcDpIndex = uNwTc << 9 |
                uNwDp << 7 |
                uMtagCmd << 5 |
                mCast << 4 |
                uTrgPortTcProfile;
            break;

    case SNET_CHT_PRIO_TABLE_ACCESS_MODE_1B_SRC_3B_TRG:
        tcDpIndex = uNwTc << 9 |
                uNwDp << 7 |
                uMtagCmd << 5 |
                mCast << 4 |
                uTrgPortTcProfile << 1 |
                uSrcPortTcProfile ;
            break;
    default:
                tcDpIndex = uNwTc << 9 |
                uNwDp << 7 |
                uMtagCmd << 5 |
                mCast << 4 |
                uTrgPortTcProfile << 2 |
                uSrcPortTcProfile ;
            break;

    }

    __LOG_PARAM_WITH_NAME("Index for accessing 'TC,DP mapper table' ",tcDpIndex);
    __LOG_PARAM_WITH_NAME("Prio table access mode ",uPrioTableAccessMode);

    regAddr = SMEM_SIP5_20_EGF_QAG_TC_DP_MAPPER_TBL_MEM(devObjPtr,tcDpIndex);
    memPtr = smemMemGet(devObjPtr, regAddr);
    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        descrPtr->queue_priority = snetFieldValueGet(memPtr, 2, 4);
    }
    else
    {
        descrPtr->queue_priority = snetFieldValueGet(memPtr, 2, 3);
    }
    descrPtr->queue_dp = snetFieldValueGet(memPtr, 0, 2);

    __LOG_PARAM(descrPtr->queue_priority);
    __LOG_PARAM(descrPtr->queue_dp);

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        descrPtr->egressPhysicalPortInfo.txDmaDevObjPtr = devObjPtr;


        if(SMEM_CHT_IS_SIP6_20_GET(devObjPtr))
        {
            /*check if  VLAN Q offset is needed */
            regAddr = SMEM_LION2_EGF_QAG_TARGET_PORT_MAPPER_TBL_MEM(devObjPtr,egressPort);
            memPtr = smemMemGet(devObjPtr, regAddr);
            remapBits = snetFieldValueGet(memPtr, 15, 2);
            __LOG_PARAM(remapBits);

            if(remapBits==0x1||remapBits==0x3)
            {
                /*get VLAN Q offset*/
                regAddr = SMEM_SIP6_20_EGF_QAG_VLAN_Q_OFFSET_TBL_MEM(devObjPtr,descrPtr->eVid);
                memPtr = smemMemGet(devObjPtr, regAddr);
                vlanQOffset = snetFieldValueGet(memPtr, 0, 8);
                __LOG_PARAM(descrPtr->eVid);
                __LOG_PARAM(vlanQOffset);
            }

            switch(remapBits)
            {
                case 0x1:
                 descrPtr->queue_priority = vlanQOffset;
                 break;
                case 0x3:
                 descrPtr->queue_priority += vlanQOffset;
                 break;
                case 0x2:
                 /*descrPtr->queue_priority  <= stay the same*/
                 break;
                default:
                  skernelFatalError("Target Port Mapper table index %d remap according to queue priority and VLAN is 0x0 .Valid values 0x1-0x3\n\n \n",egressPort);
                  break;
            }

        }


        /* save values in the descriptor in dedicated sip6 fields ,
                 for clarity of usage ! (and LOG indications) */
              descrPtr->egressPhysicalPortInfo.sip6_queue_group_index  = descrPtr->txqDestPorts[egressPort];
              descrPtr->egressPhysicalPortInfo.sip6_queue_group_offset = descrPtr->queue_priority;

              __LOG_PARAM(descrPtr->egressPhysicalPortInfo.sip6_queue_group_index);
              __LOG_PARAM(descrPtr->egressPhysicalPortInfo.sip6_queue_group_offset);


        if(SMEM_CHT_IS_SIP6_20_GET(devObjPtr))
        {
         snetSip6QueueRangeCheck(devObjPtr,descrPtr,descrPtr->egressPhysicalPortInfo.sip6_queue_group_offset);
        }
    }

}

/**
* @internal snetChtEgfQagStackTcDpRemap function
* @endinternal
*
* @brief   SIP5 :
*         set TC and DP Remapping on stack egress ports.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
* @param[in] egressPort               - output port
*                                       COMMENTS :
*/
static void snetChtEgfQagStackTcDpRemap(

    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32               egressPort
)

{
    DECLARE_FUNC_NAME(snetChtEgfQagStackTcDpRemap);

    GT_U32 dstPortStackEn = 0;     /* type of destination port: network or stack */
    GT_U32 srcPortStackEn = 0;     /* type of src port: network or stack */
    GT_U32   regAddr;              /* register address */
    GT_U32   oldTc;                /* packet's TC before remapping*/
    GT_U32   oldDp;                /* packet DP before remapping */
    SKERNEL_MTAG_CMD_ENT  mtagCmd; /* Marvell tag command */
    GT_U32   tcDpIndex;            /* index for tc and dp remapping tables*/

    regAddr = SMEM_LION3_EGF_QAG_STACK_REMAP_EN_REG(devObjPtr,egressPort/32);

    /* Read the egress port remap operation status  */
    smemRegFldGet(devObjPtr, regAddr, egressPort%32,1,&dstPortStackEn);

    if(dstPortStackEn == 0)
    {
        __LOG(("Stack {tc,dp} remap disabled on egress port[%d] \n",egressPort));
        return;
    }
    __LOG(("Stack {tc,dp} remap enabled on egress port[%d] \n",egressPort));

    /* Check if the src port is stack one */
    regAddr = SMEM_LION3_EGF_QAG_STACK_REMAP_EN_REG(devObjPtr,descrPtr->localDevSrcPort/32);
    smemRegFldGet(devObjPtr, regAddr, descrPtr->localDevSrcPort%32,1,&srcPortStackEn);

    oldTc = descrPtr->tc;
    oldDp = descrPtr->dp;
    mtagCmd = descrPtr-> outGoingMtagCmd;
    __LOG_PARAM(oldTc);
    __LOG_PARAM(oldDp);
    __LOG_PARAM(mtagCmd);
    __LOG_PARAM(srcPortStackEn);

    /* Calculate index for both tc and dp mapping.*/
    tcDpIndex = (oldTc << 5) | (srcPortStackEn << 4) | (mtagCmd << 2) | oldDp;
    __LOG_PARAM(tcDpIndex);

    /* get new mapped TC value*/
    regAddr = SMEM_LION3_EGF_QAG_TC_REMAP_REG(devObjPtr,tcDpIndex/8);
    smemRegFldGet(devObjPtr, regAddr, (tcDpIndex%8)*3,3,&descrPtr->queue_priority);

    /* get new mapped DP value*/
    regAddr = SMEM_CHT3_DP2CFI_ENABLE_REG(devObjPtr);
    regAddr = regAddr + (tcDpIndex/16) * 0x4;
    smemRegFldGet(devObjPtr, regAddr, (tcDpIndex%16)*2,2,&descrPtr->queue_dp);
    __LOG_PARAM(descrPtr->queue_priority);
    __LOG_PARAM(descrPtr->queue_dp);
}

/**
* @internal txqToPortQueueDisableCheck function
* @endinternal
*
* @brief   Analyse egressPort for disabled TC
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - egress port
* @param[in] destVlanTagged           - egress tag state
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
static GT_BOOL txqToPortQueueDisableCheck
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN    SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN    GT_U32                           egressPort,
    IN    GT_U8                            destVlanTagged
)
{
    DECLARE_FUNC_NAME(txqToPortQueueDisableCheck);

    GT_BOOL isSecondRegister;/* is the per port config is in second register -- for bmp of ports */
    GT_U32  outputPortBit;  /* the bit index for the egress port */
    GT_U32  transmissionEnabled;
    GT_U32  regAddr;
    GT_U32  txqPortNum;
    GT_U32  dqUnitInPipe;/* the DP unit in the pipe that handle packet */
    GT_U32  localTxqPortNum;/* local DQ port number to the 'PIPE' */

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* calc current descrPtr->queue_priority */

        snetChtTxqDqPerPortInfoGet(devObjPtr, descrPtr, egressPort, &isSecondRegister, &outputPortBit);
        txqPortNum = outputPortBit;

        if(devObjPtr->multiDataPath.txqDqNumPortsPerDp)
        {
            /* Update 'currentPipeId' and get new (local) txqPortNum */
            localTxqPortNum = txqPortNum % devObjPtr->multiDataPath.txqDqNumPortsPerDp;
            dqUnitInPipe = (txqPortNum /devObjPtr->multiDataPath.txqDqNumPortsPerDp) %
                devObjPtr->multiDataPath.numTxqDq;

            if(devObjPtr->numOfPipes)
            {
                __LOG(("CurrentPipeId[%d] \n",smemGetCurrentPipeId(devObjPtr)));
            }
            __LOG_PARAM(dqUnitInPipe);
        }
        else
        {
            dqUnitInPipe = 0;
            localTxqPortNum = txqPortNum;
        }
        __LOG_PARAM(localTxqPortNum);

        regAddr = SMEM_LION_DEQUEUE_ENABLE_REG(devObjPtr, localTxqPortNum , dqUnitInPipe);
        smemRegFldGet(devObjPtr, regAddr, descrPtr->queue_priority, 1, &transmissionEnabled);
    }
    else
    {
        if(egressPort == SNET_CHT_CPU_PORT_CNS)
        {
            /* the CPU not hold TXQ port */
            return GT_TRUE;
        }

        /* check is TC disabled for this port */
        if(devObjPtr->txqRevision == 0)
        {
            /* single core devices */
            regAddr = SMEM_CHT_TXQ_CONFIG_REG(devObjPtr, egressPort);
            smemRegFldGet(devObjPtr, regAddr, NUM_OF_TRAFFIC_CLASSES+descrPtr->queue_priority, 1,
                                                               &transmissionEnabled);
        }
        else
        {
            /* multi core devices */
            snetChtTxqDqPerPortInfoGet(devObjPtr, descrPtr, egressPort, &isSecondRegister, &outputPortBit);
            regAddr = SMEM_LION_DEQUEUE_ENABLE_REG(devObjPtr, outputPortBit , 0);
            if(isSecondRegister == GT_TRUE)
            {
                regAddr +=4;
            }
            smemRegFldGet(devObjPtr, regAddr, descrPtr->queue_priority, 1, &transmissionEnabled);
        }


        txqPortNum = egressPort;/*no conversion*/
    }

    if(!transmissionEnabled)
    {
        __LOG(("WARNING: target physical port[%d] (mapped to tx_port[%d]) tc[%d] is disabled for transmit , so descriptor is enqueue \n",
            egressPort,
            txqPortNum,
            descrPtr->queue_priority));

        /* enqueue packet */
        simTxqEnqueuePacket(devObjPtr, descrPtr, egressPort , txqPortNum, destVlanTagged);

        return GT_FALSE;
    }

    return GT_TRUE;
}

/**
* @internal txqToPortTillTxFifo function
* @endinternal
*
* @brief   let the TXQ to handle the packet to the egress port
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
* @param[in] egressPort               - egress Port
* @param[in] egressVlanTagMode        - the port's egress vlan tag mode (from vlan entry)
*                                       final egress TxFifo port.
*                                       when the TM (of bobcat2) is not involved ... this is always the
*                                       'egress local port' derived from 'egressPort'
*                                       value SMAIN_NOT_VALID_CNS meaning that packet was dropped.
*                                       COMMENTS :
*/
static GT_U32 txqToPortTillTxFifo
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32               egressPort,
    IN GT_U32               egressVlanTagMode
)
{
    DECLARE_FUNC_NAME(txqToPortTillTxFifo);

    GT_U8     *frameDataPtr;    /* pointer to frame data */
    GT_U32    frameDataSize;    /* frame data size */
    GT_BOOL   isPciSdma = GT_FALSE; /* send to CPU with SDMA mechanism */
    SKERNEL_DEVICE_OBJECT *hemisphereDevObjPtr;/*the 'hemisphere' device object */
    GT_U32 currentHemisphere;/* current hemisphere 0 or 1*/
    GT_U32 hemispherePortGroups[2]={0,4};/*the port groups that represents the hemispheres */
    GT_U32 egrTagMode;/*egress vlan tag mode */
    GT_U32 localPort;/* local port (local to the port group) .
                        on SIP5 need to convert from egressPort.
                        but on Lion,Lion2 - egressPort --> is already 'local port' */
    GT_U32 portGroupId;/*egress port group*/
    GT_BOOL dropPacket = GT_FALSE;/* indication that the 'TM drop' unit drops the packet */
    GT_U32 trgPortPcid; /* Target Port PCID */
    GT_BIT bpeEnabled; /* Enable for IEEE 802.1BR Multicast Source Filtering.*/
    GT_BOOL queueEnabled;

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        skernelFatalError("SIP6 : logic moved to function 'snetFalconEgfQag' \n");
        return SMAIN_NOT_VALID_CNS;
    }

    hemisphereDevObjPtr = devObjPtr;
    descrPtr->cpuPortGroupDevObjPtr = devObjPtr;

    __LOG_PARAM(egressPort);

    if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
    {
        /* not only for stacking */
        snetSip5_20EgfQagTcDpRemap(devObjPtr,descrPtr,egressPort);
    }
    else
    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        snetChtEgfQagStackTcDpRemap(devObjPtr,descrPtr,egressPort);
    }

    /* get the device object and the macPort for this egress physical port */
    if(GT_FALSE == snetChtTxMacPortGet(devObjPtr,
        descrPtr,
        MAC_PORT_REASON_TYPE_TXQ_DQ_TO_TXDMA_E,
        egressPort,GT_TRUE,
        &descrPtr->egressPhysicalPortInfo.txDmaDevObjPtr,
        &descrPtr->egressPhysicalPortInfo.txDmaMacPort))
    {
        __LOG(("MAC port [%d] not exists \n",
            egressPort));
        /* MAC port not exists */
        return SMAIN_NOT_VALID_CNS;
    }

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_TXQ_E);

    SIM_LOG_PACKET_DESCR_SAVE

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr) &&
        devObjPtr->portGroupSharedDevObjPtr)
    {
        /* convert 'global port' to 'local port' */
        localPort = SMEM_CHT_LOCAL_PORT_FROM_GLOBAL_PORT_MAC( devObjPtr,egressPort);
    }
    else
    {
        /* the port is already 'local port' */
        localPort = egressPort;
    }

    /* save the local port */
    descrPtr->egressPhysicalPortInfo.localPortIndex =  localPort;

    /* save the global port */
    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        descrPtr->egressPhysicalPortInfo.globalPortIndex = egressPort;
    }
    else
    {
        /* convert 'local port' to 'global port' */
        descrPtr->egressPhysicalPortInfo.globalPortIndex = SMEM_CHT_GLOBAL_PORT_FROM_LOCAL_PORT_MAC(devObjPtr,localPort);
    }

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        if(devObjPtr->portGroupSharedDevObjPtr)
        {
            currentHemisphere =
                SKERNEL_PORTS_BMP_IS_PORT_SET_MAC(descrPtr->bmpOfHemisphereMapperPtr,egressPort) ?
                    1 : /* Hemisphere 1 */
                    0;  /* Hemisphere 0 */
            portGroupId = hemispherePortGroups[currentHemisphere];

            /* the device object that represents the 'egress hemisphere' */
            hemisphereDevObjPtr =
                devObjPtr->portGroupSharedDevObjPtr->coreDevInfoPtr[portGroupId].devObjPtr;

            __LOG(("handle target port[%d] by hemisphere[%d] \n",
                egressPort,
                currentHemisphere));

            portGroupId = 0;/* TEMP code */
            descrPtr->cpuPortGroupDevObjPtr =
                devObjPtr->portGroupSharedDevObjPtr->coreDevInfoPtr[portGroupId].devObjPtr;

            /* the device object that represents the 'egress portGroup' */
            portGroupId = SMEM_CHT_PORT_GROUP_ID_FROM_GLOBAL_PORT_MAC(devObjPtr,egressPort);
            devObjPtr =
                devObjPtr->portGroupSharedDevObjPtr->coreDevInfoPtr[portGroupId].devObjPtr;
        }

        if(descrPtr->continueFromTxqDisabled)
        {
            __LOG(("continueFromTxqDisabled : TXQ counter sets already counted for this descriptor (before the 'enqueue') \n"));
        }
        else
        {
            /* TXQ counter sets counting on the egress device */
            snetChtTxQCounterSets(hemisphereDevObjPtr, descrPtr, NULL/*destPorts*/,
                  NULL/*destPortsLb*/, NULL, descrPtr->egressPacketType, NULL,
                  TXQ_COUNTE_MODE_EGRESS_DEVICE_ONLY_E,
                  egressPort);
        }

        /* check for TXQ VID reassignment */
        snetChtEgfQagVidAssignment(hemisphereDevObjPtr, descrPtr, egressPort);

        /* the egress vlan tag mode is determined by different table then
           the 'vlan table' that used for 'vlan attributes and  members' !!*/
        egrTagMode =
            snetChtEgfQagVlanTagModeGet(hemisphereDevObjPtr, descrPtr,egressPort);
    }
    else
    {
        egrTagMode = egressVlanTagMode;

        if(devObjPtr->support802_1br_PortExtender &&
           descrPtr->useVidx == 0)
        {

            /* Read the Target Port(PCID)  */
            smemRegFldGet(devObjPtr,
                SMEM_XCAT3_EGRESS_AND_TXQ_PCID_TARGET_PORT_P_REG(devObjPtr, egressPort), 0, 12, &trgPortPcid);
            smemRegFldGet(devObjPtr,
                SMEM_XCAT3_EGRESS_AND_TXQ_802_1_BR_GLOBAL_CONFIG_REG (devObjPtr), 0, 1, &bpeEnabled);
            if(bpeEnabled == 1)
            {
                /*If Desc<vid1> = Target Port(PCID) then remove tag1 from the tag state.
                {tag1} => {untagged}; {outer tag1, inner tag0}=> {tag0}; {outer tag0, inner tag1}=>{tag0}*/

                if(descrPtr->vid1 == trgPortPcid)
                {
                    switch(egrTagMode)
                    {
                        case SKERNEL_FRAME_TR101_VLAN_EGR_TAG_TAG1_E:
                            __LOG(("Tag1 Removed (remove ETag): changed from 'tag 1' to 'untagged' \n"));
                            egrTagMode = SKERNEL_FRAME_TR101_VLAN_EGR_TAG_UNTAGGED_E;
                            break;
                        case SKERNEL_FRAME_TR101_VLAN_EGR_TAG_OUT_TAG0_IN_TAG1_E:
                            __LOG(("Tag1 Removed (remove ETag): changed from 'outer tag 0 inner tag 1' to 'tag 0' \n"));
                            egrTagMode = SKERNEL_FRAME_TR101_VLAN_EGR_TAG_TAG0_E;
                            break;
                        case SKERNEL_FRAME_TR101_VLAN_EGR_TAG_OUT_TAG1_IN_TAG0_E:
                            __LOG(("Tag1 Removed (remove ETag): changed from 'outer tag 1 inner tag 0' to 'tag 0' \n"));
                            egrTagMode = SKERNEL_FRAME_TR101_VLAN_EGR_TAG_TAG0_E;
                            break;
                        default:
                            /* no modification needed */
                            __LOG(("Tag1 not exists on egress packet -- no modification needed \n"));
                            break;
                    }
                }
            }
        }
    }

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

    /* 1. Filter disabled TC
     * 2. Enqueue frame(if need).
     */
    queueEnabled = txqToPortQueueDisableCheck(devObjPtr,descrPtr,
        egressPort,egressVlanTagMode);

    if(queueEnabled == GT_FALSE)
    {
        __LOG(("NOTE: the queue of 'TXQ-PORT' [%d] is disabled ('physical port'[%d]) no more TXQ processing \n",
            descrPtr->txqDestPorts[egressPort],
            egressPort));
        return (SMAIN_NOT_VALID_CNS - 1);
    }


    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        __LOG(("NOTE: sip 5 : Egress Queue Pass/Tail-Drop CNC Trigger , using 'TXQ-PORT' [%d] (not 'physical port'[%d]) \n",
            descrPtr->txqDestPorts[egressPort],
            egressPort));

        /* Egress Queue Pass/Tail-Drop CNC Trigger */
        snetCht3CncCount(devObjPtr, descrPtr,
                         SNET_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_AND_QCN_PASS_DROP_E,
                         descrPtr->txqDestPorts[egressPort]);
    }
    else
    {
        /* Egress Queue Pass/Tail-Drop CNC Trigger */
        snetCht3CncCount(devObjPtr, descrPtr,
                         SNET_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_AND_QCN_PASS_DROP_E,
                         egressPort);
    }

    /* egress mirroring per physical port and per ePort and STC and replication */
    snetChtTxQSniffAndStc(devObjPtr,descrPtr,egressPort,egrTagMode);

    /* DP to CFI */
    snetChtTxQDpToCfi(hemisphereDevObjPtr, descrPtr, egressPort);


    /* check that the BMA unit knows the same info as the original rxDma info */
    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        if(GT_FALSE == snetChtTxMacPortGet(devObjPtr,
            descrPtr,
            BM_CHECK_INGRESS_SRC_PORT_E,
            egressPort/* don't care */,
            GT_TRUE,
            NULL,/* don't care */
            NULL/* don't care */))
        {
            /* packet is dropped by BM/BMA */
            __LOG(("(sip5) the BM (buffer management) need to check mapping \n"));
            __LOG(("between the 'local dev src port' to the 'rxdma src port' \n"));
            __LOG(("when no match packet may be dropped by the HW / cause buffers 'underrun' \n"));
            __LOG(("causing buffers to be occupied and not be available for others ....  \n"));
            __LOG(("for simulation we will drop (filter) such packet with LOG info.  \n"));
            __LOG(("(finding reason for such issue on HW may take MORE than few hours)  \n"));

            __LOG(("ERROR : packet is dropped by BM/BMA - target port[%d] (filtered)\n",
                egressPort));

                        /* BMA error */
            return SMAIN_NOT_VALID_CNS;
        }
    }

    SIM_LOG_PACKET_DESCR_COMPARE("txqToPortTillTxFifo - before HA unit");


    __LOG(("do HA unit - target port[%d]\n",
        egressPort));

    SIM_LOG_PACKET_DESCR_SAVE
    snetChtHaMain(devObjPtr, descrPtr, egressPort, (GT_U8)egrTagMode,
                       &frameDataPtr, &frameDataSize, &isPciSdma);
    descrPtr->isCpuUseSdma = isPciSdma;
    SIM_LOG_PACKET_DESCR_COMPARE("snetChtHaMain");
    if(descrPtr->haAction.drop == SKERNEL_EXT_PKT_CMD_HARD_DROP_E)
    {
        __LOG(("WARNING : packet is dropped by HA - target port[%d]\n",
            egressPort));

        return SMAIN_NOT_VALID_CNS;
    }

    __LOG(("do EPCL unit - target port[%d]\n",
        egressPort));

    SIM_LOG_PACKET_DESCR_SAVE
    snetChtEpclProcess(devObjPtr,descrPtr,egressPort);
    SIM_LOG_PACKET_DESCR_COMPARE("snetChtEpclProcess");

    if(descrPtr->epclAction.drop == SKERNEL_EXT_PKT_CMD_HARD_DROP_E)
    {
        __LOG(("WARNING : packet is dropped by EPCL - target port[%d]\n",
            egressPort));

        return SMAIN_NOT_VALID_CNS;
    }

    if(devObjPtr->supportTrafficManager)
    {
        SIM_LOG_PACKET_DESCR_SAVE
        /* TmQMapper unit */
        egressTmQMapper(devObjPtr,descrPtr,egressPort);
        SIM_LOG_PACKET_DESCR_COMPARE("egressTmQMapper");
    }

    /* do EPLR after EPCL */
    SIM_LOG_PACKET_DESCR_SAVE
    snetChtEplrProcess(devObjPtr, descrPtr, egressPort);
    SIM_LOG_PACKET_DESCR_COMPARE("snetChtEplrProcess");

    if(descrPtr->eplrAction.drop == SKERNEL_EXT_PKT_CMD_HARD_DROP_E)
    {
        __LOG(("WARNING : packet is dropped by EPLR - target port[%d]\n",
            egressPort));

        return SMAIN_NOT_VALID_CNS;
    }

    if(devObjPtr->supportTrafficManager)
    {
        /*TM Drop and Statistics Unit*/
        SIM_LOG_PACKET_DESCR_SAVE
        dropPacket = egressTmDropUnit(devObjPtr,descrPtr,egressPort,frameDataPtr, frameDataSize);
        SIM_LOG_PACKET_DESCR_COMPARE("egressTmDropUnit");
        if(dropPacket == GT_TRUE)
        {
            __LOG(("WARNING : packet is dropped by 'TM drop unit' - target port[%d]\n",
                egressPort));

            return SMAIN_NOT_VALID_CNS;
        }
    }

    /* do egress final packet modifications (after EPCL and EPLR) */
    SIM_LOG_PACKET_DESCR_SAVE
    snetChtEgressFinalPacketAlteration(devObjPtr, descrPtr, egressPort);
    SIM_LOG_PACKET_DESCR_COMPARE("snetChtEgressFinalPacketAlteration");

    if(descrPtr->haAction.drop == SKERNEL_EXT_PKT_CMD_HARD_DROP_E)
    {
        __LOG(("WARNING : packet is dropped by due to PTP checks - target port[%d]\n",
            egressPort));

        return SMAIN_NOT_VALID_CNS;
    }

    if(descrPtr->tables_read_error)
    {
        __LOG(("'tables_read_error' ERROR detected : packet is dropped - target port[%d]\n"
               "check for the 'violated' table previously in the LOG \n",
            egressPort));

        return SMAIN_NOT_VALID_CNS;
    }


    return localPort;
}


/**
* @internal egressSendToTmUnit function
* @endinternal
*
* @brief   let the TM unit to start process the packet.
*         the TM unit is operating asynchronous to the PP and packet will egress
*         from it in other context !!!
*         see function smainReceivedPacketDoneFromTm(...)
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
* @param[in] txFifoPort               - egress Port (tx fifo port)
* @param[in] frameDataPtr             - pointer to frame data
* @param[in] frameDataSize            - frame data size
*                                       none
*                                       COMMENTS :
*/
static void egressSendToTmUnit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32               txFifoPort,
    IN GT_U8                *frameDataPtr,
    IN GT_U32               frameDataSize
)
{
    DECLARE_FUNC_NAME(egressSendToTmUnit);

    SBUF_BUF_ID             bufferId;    /* buffer */
    SBUF_BUF_STC            *bufferInfoPtr;
    GT_U32                  bufferSize;
    GT_U32                  numOfBuffers;
    SREMOTE_TM_PP_TO_TM_PACKET_INFO_STC tmPacketInfo;
    GT_U8                   *dataPtr;/*pointer to data in the buffer */
    GT_U32                  maxPedding = 8;/* max number of bytes for padding 8 bytes alignment for 'pointer to descriptor'*/
    GT_U32                  numBytesPadding;/* number of bytes for padding 8 bytes alignment for 'pointer to descriptor'*/

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_TRAFFIC_MANAGER_ENGINE_E);

    if(devObjPtr->tmInfo.bufPool_DDR3_TM == NULL)
    {
        /* each packet should support max packet + descriptor */
        bufferSize = (SBUF_DATA_SIZE_CNS + sizeof(SKERNEL_FRAME_CHEETAH_DESCR_STC));
        numOfBuffers = (16*1024);
        __LOG(("Allocating memory to emulate the 'DDR3' of the TM to support [%d] packets (each up to [%d] bytes) \n",\
            numOfBuffers,
            SBUF_DATA_SIZE_CNS));
        /* create buffers pool */
        devObjPtr->tmInfo.bufPool_DDR3_TM =
            sbufPoolCreateWithBufferSize(numOfBuffers,bufferSize);
    }
    /*set actual buffer needed*/
    bufferSize = frameDataSize +
                sizeof(SKERNEL_FRAME_CHEETAH_DESCR_STC) +
                maxPedding;/* max padding */

    /* Get buffer */
    bufferId = sbufAlloc(devObjPtr->tmInfo.bufPool_DDR3_TM, bufferSize);
    if(bufferId == NULL)
    {
        __LOG(("the bufPool_DDR3_TM is full and no free buffer !!! \n"
               "so packet is not sent to the TM !!! (and dropped) \n"));
        return;
    }

    bufferInfoPtr = (SBUF_BUF_STC*)bufferId;

    /* set on the buffer the packet + the descriptor */
    bufferInfoPtr->actualDataPtr = bufferInfoPtr->data;
    bufferInfoPtr->actualDataSize = bufferSize;

    dataPtr = bufferInfoPtr->data;

    /* copy egress packet to buffer , and copy descriptor too */

    /* copy packet length */
    memcpy(dataPtr,&frameDataSize,sizeof(frameDataSize));
    dataPtr += sizeof(frameDataSize);

    /* copy packet */
    memcpy(dataPtr,frameDataPtr,frameDataSize);
    dataPtr += frameDataSize;

    /* calc padding for alignment for 'pointer' --> make 8 bytes alignment */
    numBytesPadding = (maxPedding - (((GT_UINTPTR)dataPtr) % maxPedding)) % maxPedding;
    /* add the padding */
    dataPtr += numBytesPadding;

    /* copy descriptor */
    memcpy(dataPtr ,descrPtr,
        sizeof(SKERNEL_FRAME_CHEETAH_DESCR_STC));

    memset(&tmPacketInfo,0,sizeof(tmPacketInfo));

    tmPacketInfo.cookiePtr = bufferInfoPtr;
    tmPacketInfo.color  = descrPtr->trafficManagerColor;
    tmPacketInfo.cos    = descrPtr->trafficManagerCos;
    tmPacketInfo.pk_len = frameDataSize;
    tmPacketInfo.queue  = descrPtr->tmQueueId;

    __LOG_PARAM(tmPacketInfo.cookiePtr);
    __LOG_PARAM(tmPacketInfo.color);
    __LOG_PARAM(tmPacketInfo.cos);
    __LOG_PARAM(tmPacketInfo.pk_len);
    __LOG_PARAM(tmPacketInfo.queue);

    __LOG((SIM_LOG_IMPORTANT_INFO_STR(SIM_LOG_OUT_REPLICATION_TO_TM_UNIT_STR)
            "send processing frame to TM unit: deviceName[%s],deviceId[%d], \n"
            "                    portGroupId[%d],tmFinalPort[%d],tmQueueId[%d] \n",
            devObjPtr->deviceName,
            devObjPtr->deviceId,
            devObjPtr->portGroupId ,
            descrPtr->trafficManagerFinalPort,
            descrPtr->tmQueueId));

    /* the skernel task is going to decrement the orig packet ,
       we need to indicate about this still processed packet by remote TM  */
    skernelNumOfPacketsInTheSystemSet(GT_TRUE);

    /* the TM unit is in 'Remote simulation' so we will send it a message */
    sRemoteTmPacketSendToTm(devObjPtr->deviceId,&tmPacketInfo);


    /* NOTE:
        the TM unit is operating asynchronous to the PP and packet will egress
        from it in other context !!!
        see smainReceivedPacketDoneFromTm(...) that will send message to
        sKernel task to allow snetChtEgressAfterTmUnit(...) finish the
        egress pipe after the TM done.
    */
}

/**
* @internal snetChtEgressAfterTmUnit function
* @endinternal
*
* @brief   finish processing the packet in the egress pipe after the TM unit
*         finished with the packet.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] dataPtr                  - pointer to TM attached info.
* @param[in] dataLength               - length of the data
*
* @note this function called in context of smain task but hold buffer with
*       packet + descriptor that where send from the same task but 'long time ago'
*
*/
GT_VOID snetChtEgressAfterTmUnit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U8                 * dataPtr,
    IN GT_U32                  dataLength
)
{
    DECLARE_FUNC_NAME(snetChtEgressAfterTmUnit);

    GT_VOID *cookiePtr;/* the cookie that was attached to the packet */
    GT_U32   tmFinalPort;/*the TM final port*/
    SBUF_BUF_ID  bufferId;    /* buffer */
    SBUF_BUF_STC *bufferInfoPtr;
    GT_U8   *frameDataPtr; /* pointer to frame data  (on the buffer)*/
    GT_U32  frameDataSize; /* frame data size       */
    SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr;/*pointer to descriptor (on the buffer)*/
    GT_U8   *dataInBufferPtr;/*pointer to data in the buffer */
    GT_U32  maxPedding = 8;/* max number of bytes for padding 8 bytes alignment for 'pointer to descriptor'*/
    GT_U32  numBytesPadding;/* number of bytes for padding 8 bytes alignment for 'pointer to descriptor'*/
    GT_U32  txFifoPort;/*egress Port (tx fifo port)*/
    GT_U32  tmTargetPortInterafce;/*TM Target Port Interafce : Ethernet / Interlaken*/
    GT_U32  *memPtr;                /* pointer to memory */
    GT_U32  regAddr;                /* address of registers */

    SIM_OS_MAC(simOsTaskOwnTaskPurposeSet)(
        SIM_OS_TASK_PURPOSE_TYPE_PP_PIPE_PROCESSING_DAEMON_E,NULL);

    /* parse the data from the buffer of the message */
    /* get parameter 1 */
    memcpy(&cookiePtr,dataPtr,sizeof(cookiePtr));
    dataPtr+=sizeof(cookiePtr);
    /* save parameter 2 */
    memcpy(&tmFinalPort,dataPtr,sizeof(tmFinalPort));
    dataPtr+=sizeof(tmFinalPort);

    /* the sender that built this cookie is function egressSendToTmUnit(...) */
    /* we need to here the same logic */
    bufferId = cookiePtr;
    bufferInfoPtr = (SBUF_BUF_STC*)bufferId;

    dataInBufferPtr = bufferInfoPtr->data;
    /* get egress packet from the buffer , and the descriptor too */

    /* get packet length */
    memcpy(&frameDataSize,dataInBufferPtr,sizeof(frameDataSize));
    dataInBufferPtr += sizeof(frameDataSize);

    /* buffer start with the packet */
    frameDataPtr  = dataInBufferPtr;
    /* calculate the alignment to the descriptor (as pointer) */
    dataInBufferPtr += frameDataSize;
    /* calc padding for alignment for 'pointer' --> make 8 bytes alignment */
    numBytesPadding = (maxPedding - (((GT_UINTPTR)dataInBufferPtr) % maxPedding)) % maxPedding;

    dataInBufferPtr += numBytesPadding;

    /* buffer continue with descriptor (after the packet + alignment) */
    descrPtr = (SKERNEL_FRAME_CHEETAH_DESCR_STC*)dataInBufferPtr;

    if(smainIsEmulateRemoteTm())
    {
        __LOG(("smainIsEmulateRemoteTm() : TRICK : set trafficManagerFinalPort = original egressPort[%d] \n",
            tmFinalPort));
        /* use info that the TM_DROP unit returned , because the emulation does
           not know the final egress port */
        tmFinalPort = descrPtr->trafficManagerFinalPort;
    }

    __LOG((SIM_LOG_IMPORTANT_INFO_STR(SIM_LOG_IN_REPLICATION_FROM_TM_UNIT_STR)
            "start processing frame returned from TM unit: deviceName[%s],deviceId[%d], \n"
            "                    portGroupId[%d],tmFinalPort[%d] \n",
            devObjPtr->deviceName,
            devObjPtr->deviceId,
            devObjPtr->portGroupId ,
            tmFinalPort));

    /***********************************************/
    /* now we can let the egress pipe finish       */
    /*  processing this packet                     */
    /***********************************************/

    __LOG_PARAM(frameDataSize);
    __LOG_PARAM(numBytesPadding);
    __LOG_PARAM(tmFinalPort);

    /* each entry hold 4 ports , 8 bits per port */
    /* the macro already know about the 4 ports in entry */
    regAddr =
        SMEM_BOBCAT2_TM_EGRESS_GLUE_TARGET_INTERFACE_TBL_MEM(devObjPtr,tmFinalPort);
    memPtr = smemMemGet(devObjPtr, regAddr);
    /* 7 bits out of 8 are the new port number */
    txFifoPort = snetFieldValueGet(memPtr, (tmFinalPort % 4)*8, 7);
    /* 1 last bit out of 8 is TM Target Port Interafce : Ethernet / Interlaken */
    tmTargetPortInterafce = snetFieldValueGet(memPtr, ((tmFinalPort % 4)*8) + 7, 1);

    __LOG_PARAM(txFifoPort);
    __LOG_PARAM(tmTargetPortInterafce);

    /* update the mac port (because it was equal to devObjPtr->dmaNumOfTmPort)*/
    descrPtr->egressPhysicalPortInfo.txDmaMacPort = txFifoPort;

    __LOG_PARAM(descrPtr->egressPhysicalPortInfo.txDmaMacPort);

    if(tmTargetPortInterafce != 0)/*Interlaken*/
    {
        __LOG(("simulation not supports Interlaken port (sending to ethernet port !!!) \n"));
    }

    if(devObjPtr->portGroupSharedDevObjPtr)
    {
        /* protect the egress processing , since this called in the context of
           the task that represent the ingress port group , and since there are more
           then one ingress port group , we need to keep the egress device buffers
           protected */
        SIM_OS_MAC(simOsMutexLock)(devObjPtr->portGroupSharedDevObjPtr->protectEgressMutex);
    }

    /* call TxFifo + MAC */
    egressTxFifoAndMac(devObjPtr,descrPtr,txFifoPort,frameDataPtr,frameDataSize);

    if(devObjPtr->portGroupSharedDevObjPtr)
    {
        SIM_OS_MAC(simOsMutexUnlock)(devObjPtr->portGroupSharedDevObjPtr->protectEgressMutex);
    }

    __LOG((SIM_LOG_OUT_REPLICATION_STR,
        "Ended processing replication (that returned from TM unit) that egress: "
        "   deviceName[%s],deviceId[%d], \n"
        "   portGroupId[%d],port[%d] \n",
            devObjPtr->deviceName,
            devObjPtr->deviceId,
            devObjPtr->portGroupId ,
            txFifoPort));

    /* we called from skernel task and we need to decrement this packet as system finished with it
        after returned from the remote TM  */
    skernelNumOfPacketsInTheSystemSet(GT_FALSE);

    /* free the buffer that hold the egress packet + descriptor that was
       allocated during egressSendToTmUnit(...) */
    sbufFree(devObjPtr->tmInfo.bufPool_DDR3_TM, bufferId);

}


/**
* @internal egressTxFifoAndMac function
* @endinternal
*
* @brief   let the TxFifo and MAC to egress the packet
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
* @param[in] txFifoPort               - egress Port (tx fifo port)
* @param[in] frameDataPtr             - pointer to frame data
* @param[in] frameDataSize            - frame data size
*                                       COMMENTS :
*/
static void egressTxFifoAndMac
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32               txFifoPort,
    IN GT_U8                *frameDataPtr,
    IN GT_U32               frameDataSize
)
{
    DECLARE_FUNC_NAME(egressTxFifoAndMac);
    GT_BOOL isCpuPort;/* indication that CPU will get the packet */
    GT_U32  dmaNumOfCpuPort;/* the SDMA CPU port number */

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_TXFIFO_E);

    dmaNumOfCpuPort = devObjPtr->dmaNumOfCpuPort ? devObjPtr->dmaNumOfCpuPort : SNET_CHT_CPU_PORT_CNS;

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT___ALLOW_ALL_UNITS___E);/* wild card for LOG without the unit filter */

    /*check DMA number for CPU port*/
    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        isCpuPort = (descrPtr->egressPhysicalPortInfo.txDmaMacPort == dmaNumOfCpuPort) ? GT_TRUE : GT_FALSE;
    }
    else
    {
        isCpuPort = (txFifoPort == dmaNumOfCpuPort) ? GT_TRUE : GT_FALSE;
    }

    if(descrPtr->cpuPortGroupDevObjPtr == NULL)
    {
        /* support for PIPE device that currently not hold SDMA support for CPU port */
        isCpuPort = GT_FALSE;
    }

    if (isCpuPort == GT_TRUE)/*check DMA number for CPU port*/
    {
        if(descrPtr->isCpuUseSdma)
        {
            /* DMA to CPU - DMA descriptors management */
            __LOG(("DMA to CPU - DMA descriptors management - target port[%d]\n",
                txFifoPort));
            snetChtDma2Cpu(descrPtr->cpuPortGroupDevObjPtr, descrPtr, frameDataPtr,frameDataSize);
        }
        else
        {
            __LOG(("SGMII to CPU - target port[%d] \n",
                txFifoPort));
            snetChtTx2Port(descrPtr->cpuPortGroupDevObjPtr,descrPtr,txFifoPort,frameDataPtr,frameDataSize);
        }
    }
    else
    { /* Tx MAC Processing */
        __LOG(("Tx MAC Processing - target port[%d]\n",
            txFifoPort));
        snetChtTx2Port(devObjPtr,descrPtr,txFifoPort,frameDataPtr,frameDataSize);
    }
}
/**
* @internal txqToPort function
* @endinternal
*
* @brief   let the TXQ to handle the packet to the egress port
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
* @param[in] egressPort               - egress Port
* @param[in] egressVlanTagMode        - the port's egress vlan tag mode (from vlan entry)
*                                       indication that packet was send to the DMA of the port
*                                       COMMENTS :
*/
static GT_BOOL txqToPort
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32               egressPort,
    IN GT_U32               egressVlanTagMode
)
{
    DECLARE_FUNC_NAME(txqToPort);
    GT_U32  txFifoPort;
    GT_U8  *frameDataPtr;/* pointer to egress buffer (that hold egress packet) */
    GT_U32  frameDataSize;/*number of bytes to send from egress buffer*/

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        skernelFatalError("SIP6 : logic moved to function 'snetFalconEgressProcess' \n");
        return GT_FALSE;
    }

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_TXQ_E);

    /* call part 1 - TXQ + HA + EPCL + EOAM + EPLR + TM_Drop + EKRMK */
    /* the TM_drop unit can change the egress port */
    txFifoPort = txqToPortTillTxFifo(devObjPtr,descrPtr,egressPort,egressVlanTagMode);
    if( txFifoPort == SMAIN_NOT_VALID_CNS)
    {
        __LOG(("packet was dropped, will not egress port [%d] \n",
            egressPort));
        /* packet was dropped */
        return GT_FALSE;
    }
    else
    if( txFifoPort == (SMAIN_NOT_VALID_CNS - 1))
    {
        /* packet was queued ... no more processing for it */
        return GT_FALSE;
    }

    frameDataSize = descrPtr->egressByteCount;
    frameDataPtr = devObjPtr->egressBuffer;

    if(descrPtr->trafficManagerEnabled)
    {
        /* call TM unit to start process the packet */
        SIM_LOG_PACKET_DESCR_SAVE
        egressSendToTmUnit(devObjPtr,descrPtr,txFifoPort,frameDataPtr,frameDataSize);
        SIM_LOG_PACKET_DESCR_COMPARE("egressSendToTmUnit");

        /* egressTxFifoAndMac(...) for this packet will be called when the
            TM engine will indicate that processing it was done. */
        __LOG(("packet that should egress port[%d] sent to TM and will egress at unknown time !!! \n",
            txFifoPort));
    }
    else
    {
        if(devObjPtr->supportTrafficManager &&
           devObjPtr->dmaNumOfTmPort == descrPtr->egressPhysicalPortInfo.txDmaMacPort)
        {
            /* there is no such MAC , update it to be like the txFifo port ....
               avoid fatal error inside snetChtTx2Port(...) */
            descrPtr->egressPhysicalPortInfo.txDmaMacPort = txFifoPort;
        }
        if (SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /* the txDmaMacPort hold the the txFifo port but as 'global device port' */
            txFifoPort = descrPtr->egressPhysicalPortInfo.txDmaMacPort;
        }

        /* call part 2  - TxFifo + MAC */
        egressTxFifoAndMac(devObjPtr,descrPtr,txFifoPort,frameDataPtr,frameDataSize);
    }

    return GT_TRUE;
}

/**
* @internal snetChtEgressDev function
* @endinternal
*
* @brief   Egress processing main routine
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
*                                       COMMENTS :
*
* @retval The destPorts are local port for multi -core. (except destPorts[63] is for CPU port)
* @retval Meaning that when devObjPtr ->portGroupId = 2 And destPorts[4]
* @retval the global port number is = 16  devObjPtr ->portGroupId + local port (16 2) + 4 = 36
* @retval Meaning that when devObjPtr ->portGroupId = 0 And destPorts[0] = 1 global port 0 (0x00)
* @retval Meaning that when devObjPtr ->portGroupId = 1 And destPorts[2] = 1 global port 18 (0x12)
* @retval Meaning that when devObjPtr ->portGroupId = 2 And destPorts[4] = 1 global port 36 (0x24)
* @retval Meaning that when devObjPtr ->portGroupId = 3 And destPorts[10] = 1 global port 58 (0x3A)
*/
GT_VOID snetChtEgressDev
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetChtEgressDev);

    GT_U32    destPorts[SKERNEL_CHEETAH_EGRESS_MAX_PORT_CNS];        /* destination untagged port array */
    GT_U8     destVlanTagged[SKERNEL_CHEETAH_EGRESS_MAX_PORT_CNS];   /* destination tagged port array */
    GT_U32    destPortsLb[SKERNEL_CHEETAH_EGRESS_MAX_PORT_CNS]; /* destination lopback ports */
    GT_U32    outPorts;             /* summery of output ports to send the frame */

    SIM_LOG_TARGET_ARR_DECLARE(destPorts);

    /* initiation of ports vector and vlan vector */
    memset(destPorts,0,sizeof(destPorts));
    memset(destVlanTagged,0,sizeof(destVlanTagged));
    memset(destPortsLb,0,sizeof(destPortsLb));
    if(devObjPtr->portGroupSharedDevObjPtr)
    {
        /* protect the egress processing , since this called in the context of
           the task that represent the ingress port group , and since there are more
           then one ingress port group , we need to keep the egress device buffers
           protected */
        SIM_OS_MAC(simOsMutexLock)(devObjPtr->portGroupSharedDevObjPtr->protectEgressMutex);
    }

    /* save the destPorts for the logger  (for compare changes) */
    SIM_LOG_TARGET_ARR_SAVE(destPorts);

    /* 1. Egress Filtering of frames .
    2. Resolve final destination ports .
    3. Vector of send frames .
    4. Tx queue mib counters .
    5. TC assignment for packets over cascading ports. */

    SIM_LOG_PACKET_DESCR_SAVE

    outPorts = snetChtTxQPhase1(devObjPtr,descrPtr,
                                destPorts, destVlanTagged, destPortsLb);
    SIM_LOG_PACKET_DESCR_COMPARE("snetChtTxQPhase1");

    if (outPorts == 0)
    {
        __LOG(("snetChtTxQPhase1 Ended : no ports to send to \n"));
        goto exit_cleanly_lbl;
    }

    /* compare the changes in destPorts */
    SIM_LOG_TARGET_ARR_COMPARE(destPorts);

    /* save pointers to the info */
    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        SKERNEL_PORTS_BMP_CLEAR_MAC(descrPtr->bmpOfHemisphereMapperPtr);
        /* the EGF hold mapper from target physical port to TXQ port
           (mapping from 0..255 to 0..71) */
        snetChtEgfTargetPortMapperToTqx(devObjPtr,descrPtr,destPorts, destPortsLb);
    }

    /* save the new state of destPorts */
    SIM_LOG_TARGET_ARR_SAVE(destPorts);

    if (outPorts == 0)
    {
        __LOG(("snetChtTxQPhase2 Ended : no ports to send to \n"));
        goto exit_cleanly_lbl;
    }

    /* compare the changes in destPorts */
    SIM_LOG_TARGET_ARR_COMPARE(destPorts);
    snetChtEgressDev_part2(devObjPtr,descrPtr,
                           SMAIN_NOT_VALID_CNS,
                           SMAIN_NOT_VALID_CNS,
                           destPorts,
                           destVlanTagged,
                           destPortsLb);

exit_cleanly_lbl:

    if(devObjPtr->portGroupSharedDevObjPtr)
    {
        SIM_OS_MAC(simOsMutexUnlock)(devObjPtr->portGroupSharedDevObjPtr->protectEgressMutex);
    }

    return;
}

/**
* @internal snetChtEgressDev_part2 function
* @endinternal
*
* @brief   Egress processing main routine - part 2
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
* @param[in] egressPort               - egress port when != SMAIN_NOT_VALID_CNS
* @param[in] egressVlanTagMode        - egress port tag state when != SMAIN_NOT_VALID_CNS
* @param[in] destPorts[]              - array of target ports (valid when != NULL)
* @param[in] destVlanTagged[]         - array of egress ports tag state (valid when != NULL)
* @param[in] destPortsLb[]            - valuable only if destPort[N] == 1.
*                                   destPortsLb[N] format is:
*                                   bit [31] - if the packet destined to
*                                   port N is instead forwarded (1) or not (0)
*                                   to a loopback/service port.
*                                   bits[30:0] - a loopback/service port number.
*                                    Is valuable only if bit 31 is set.

*
* @retval The destPorts are local port for multi -core. (except destPorts[63] is for CPU port)
* @retval Meaning that when devObjPtr ->portGroupId = 2 And destPorts[4]
* @retval the global port number is = 16  devObjPtr ->portGroupId + local port (16 2) + 4 = 36
* @retval Meaning that when devObjPtr ->portGroupId = 0 And destPorts[0] = 1 global port 0 (0x00)
* @retval Meaning that when devObjPtr ->portGroupId = 1 And destPorts[2] = 1 global port 18 (0x12)
* @retval Meaning that when devObjPtr ->portGroupId = 2 And destPorts[4] = 1 global port 36 (0x24)
* @retval Meaning that when devObjPtr ->portGroupId = 3 And destPorts[10] = 1 global port 58 (0x3A)
*/
static GT_VOID snetChtEgressDev_part2
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32   egressPort,
    IN GT_U32   egressVlanTagMode,
    IN GT_U32 destPorts[],
    IN GT_U8 destVlanTagged[],
    IN GT_U32 destPortsLb[]
)
{
    DECLARE_FUNC_NAME(snetChtEgressDev_part2);

    GT_U32    portInx = 0; /* port index loop */
    GT_U32    port;        /* port number */
    SKERNEL_FRAME_CHEETAH_DESCR_STC *newDescPtr;/* (pointer to)new descriptor info */
    GT_BIT    isFirst = 1; /*is first packet sent */
    /*GT_BIT    txMirrorDone = 0;*//* Packet has been TX mirrored */
    GT_U32    maxPortNum;
    GT_BIT  dropped;/* indication that replication was dropped */
    GT_U32  bmpsEqUnitsGotTxqMirror = 0;

    if(devObjPtr->support_remotePhysicalPortsTableMode)
    {
        /* need to set the descriptor before generating the 'duplication in :
            snetChtEqDuplicateDescr(...) */
        if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {                                   /* this is EGF_qag register (not TXQ) */
            smemRegFldGet(devObjPtr, SMEM_LION_TXQ_DISTR_GENERAL_CONF_REG(devObjPtr), 15, 3,
                &descrPtr->tableAccessMode.EGF_QAG_TableMode);
            __LOG_PARAM(descrPtr->tableAccessMode.EGF_QAG_TableMode);
        }
        else
        if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
        {
            /*<table_access_mode>*/
            smemRegFldGet(devObjPtr, SMEM_LION_TXQ_DISTR_GENERAL_CONF_REG(devObjPtr),15 ,1,
                &descrPtr->tableAccessMode.EGF_QAG_TableMode);
            __LOG_PARAM(descrPtr->tableAccessMode.EGF_QAG_TableMode);
        }

    }

    /* Get pointer to duplicated descriptor (after TXQ modifications) */
    /* duplicate descriptor from the ingress core */
    newDescPtr = snetChtEqDuplicateDescr(descrPtr->ingressDevObjPtr,descrPtr);

    if(egressPort != SMAIN_NOT_VALID_CNS)
    {
        portInx = egressPort;
        maxPortNum = portInx + 1;

        __LOG(("handle specific egress port[0x%x] \n",
            egressPort));
    }
    else
    {
        maxPortNum = SKERNEL_DEV_EGRESS_MAX_PORT_CNS(devObjPtr);
        __LOG(("start loop on multiple egress ports to send packets out of the device \n"));
    }

    while (portInx < maxPortNum)
    {
        if ((destPorts == NULL) || (destPorts && destPorts[portInx]) )
        {
            if(isFirst == 0)
            {
                /* on each sending (except for first one) ... need to copy values
                   saved from the original descriptor */
                *descrPtr = *newDescPtr;
                /* due TXQ sniffing , need to restore that current port may also do TX sniff */
                /*descrPtr->txMirrorDone = txMirrorDone;*/
                descrPtr->bmpsEqUnitsGotTxqMirror = bmpsEqUnitsGotTxqMirror;
            }

            isFirst = 0;

            if(egressPort == SMAIN_NOT_VALID_CNS && destVlanTagged)
            {
                egressVlanTagMode = destVlanTagged[portInx];
            }

            descrPtr->egressPhysicalPortInfo.egressVlanTagMode = egressVlanTagMode;

            /* sip5: make updating of eVidx relevant to multi-target packets
               forwarded to loopback/service port.  */
            if (descrPtr->useVidx && descrPtr->useLbVidx)
            {
                __LOG_PARAM(descrPtr->eVidx);
                __LOG(("eVidx is updated  because the multi-target packet is redirected"
                       " to loopback/service port and"
                       " <Multi-Target eVIDX Mapping Enable> is enabled\n"));

                descrPtr->eVidx = descrPtr->lbVidxOffset + portInx;
                __LOG_PARAM(descrPtr->eVidx);
            }

            port = destPorts ?
                CHOOSE_TRG_OR_LB_PORT_MAC(portInx, destPortsLb[portInx]) :
                portInx;

            if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
            {
                if(GT_FALSE == snetFalconEgressProcess(devObjPtr,descrPtr,port))
                {
                    dropped = 1;
                }
                else
                {
                    dropped = 0;
                }
            }
            else
            {
                if(GT_FALSE == txqToPort(devObjPtr,descrPtr,port,egressVlanTagMode))
                {
                    dropped = 1;
                }
                else
                {
                    dropped = 0;
                }
            }


            simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT___ALLOW_ALL_UNITS___E);/* wild card for LOG without the unit filter */

            if(descrPtr->trafficManagerEnabled == 0 || dropped)
            {
                __LOG((SIM_LOG_OUT_REPLICATION_STR,
                    "Ended processing replication that [%s]: "
                    "   deviceName[%s],deviceId[%d], \n"
                    "   portGroupId[%d],port[%d] \n",
                        dropped ? "DROPPED" : "egress",
                        devObjPtr->deviceName,
                        devObjPtr->deviceId,
                        devObjPtr->portGroupId ,
                        port));
            }
            else/* sent to TM and not dropped */
            {
                __LOG((SIM_LOG_OUT_REPLICATION_STR,
                    "Processing replication was sent to Traffic Manager(TM): "
                    "   deviceName[%s],deviceId[%d], \n"
                    "   portGroupId[%d],port[%d] \n",
                        devObjPtr->deviceName,
                        devObjPtr->deviceId,
                        devObjPtr->portGroupId ,
                        port));
            }

            /* after TXQ sniffing , need to save the info for the next port that may also do TX sniff */
            /*txMirrorDone = descrPtr->txMirrorDone;*/
            bmpsEqUnitsGotTxqMirror = descrPtr->bmpsEqUnitsGotTxqMirror;
        }

/* done with egress port - jump to next one */
        portInx++;

        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /* nothing more to do */
        }
        else
        {
            if(portInx == SNET_CHEETAH_MAX_PORT_NUM(devObjPtr))
            {
                /* support CAPWAP CONTROL frames that send to the Embedded CPU */
                if(devObjPtr->supportEmbeddedCpu == GT_TRUE)
                {
                    /* skip ports to the embedded CPU port */
                    portInx = SNET_CHT_EMBEDDED_CPU_PORT_CNS;
                }
                else
                {
                    /* skip ports to the CPU port */
                    portInx = SNET_CHT_CPU_PORT_CNS;
                }
            }
            else if(portInx == (SNET_CHT_EMBEDDED_CPU_PORT_CNS + 1))
            {
                /* skip ports to the CPU port */
                portInx = SNET_CHT_CPU_PORT_CNS;
            }
            else if(portInx == SNET_CHT_CPU_PORT_CNS)
            {
                /* no more ports to process */
                break;
            }
        }
    }/* while */

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT___ALLOW_ALL_UNITS___E);/* wild card for LOG without the unit filter */

    __LOG(("end loop on egress ports to send packets \n"));
}

/**
* @internal snetChtEgressDev_afterEnqueFromTqxDq function
* @endinternal
*
* @brief   Final Egress proceeding for descriptor that was stack in the queue of
*         TXQ_DQ port
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
* @param[in] egressPort               - egress port
*                                       COMMENTS :
*/
GT_VOID snetChtEgressDev_afterEnqueFromTqxDq
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32   egressPort,
    IN GT_U32   destVlanTagged
)
{
    if(devObjPtr->portGroupSharedDevObjPtr)
    {
        /* protect the egress processing , since this called in the context of
           the task that represent the ingress port group , and since there are more
           then one ingress port group , we need to keep the egress device buffers
           protected */
        SIM_OS_MAC(simOsMutexLock)(devObjPtr->portGroupSharedDevObjPtr->protectEgressMutex);
    }

    /* save pointers to the info */
    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        GT_U32    destPorts[SKERNEL_CHEETAH_EGRESS_MAX_PORT_CNS]; /* destination untagged port array */
        GT_U32  destPortsLb[SKERNEL_CHEETAH_EGRESS_MAX_PORT_CNS]; /* destPortsLb[N]  is a loopback/service port of physical port N */


        /* initiation of ports vector and vlan vector */
        memset(destPorts,0,sizeof(destPorts));
        memset(destPortsLb,0,sizeof(destPortsLb));
        destPorts[egressPort] = 1;


        SKERNEL_PORTS_BMP_CLEAR_MAC(descrPtr->bmpOfHemisphereMapperPtr);
        /* the EGF hold mapper from target physical port to TXQ port
           (mapping from 0..255 to 0..71) */
        snetChtEgfTargetPortMapperToTqx(devObjPtr,descrPtr,destPorts, destPortsLb);
    }
    snetChtEgressDev_part2(devObjPtr,descrPtr,egressPort,destVlanTagged,NULL,NULL,NULL);

    if(devObjPtr->portGroupSharedDevObjPtr)
    {
        SIM_OS_MAC(simOsMutexUnlock)(devObjPtr->portGroupSharedDevObjPtr->protectEgressMutex);
    }

    return;
}

/**
* @internal snetChtEgressPortGroupSharedConverSrcPortInfo function
* @endinternal
*
* @brief   for multi-port groups device --> convert src port info
*
* @param[in] ingressPipeDevObjPtr     - pointer to 'Ingress pipe' port group device object.
* @param[in] descrPtr                 - the descriptor for the 'Egress pipe' device
*/
static GT_VOID snetChtEgressPortGroupSharedConverSrcPortInfo
(
    IN SKERNEL_DEVICE_OBJECT             * ingressPipeDevObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr
)
{
    GT_U32  localSrcPort = descrPtr->localDevSrcPort;/* still value of 'local port' */
    GT_U32  globalSrcPort;/* global port num*/

    globalSrcPort = descrPtr->localPortGroupPortAsGlobalDevicePort;

    /* convert ingress local src port to src egress global port */
    descrPtr->localDevSrcPort   = globalSrcPort;
    descrPtr->origSrcEPortOrTrnk = descrPtr->localPortTrunkAsGlobalPortTrunk;

    /* convert global port to local port for the use of the :
        multicast source filtering , OAM loop back filter
    */
    descrPtr->localPortGroupPortAsGlobalDevicePort = localSrcPort;/* local port(not global) */

    if(descrPtr->origIsTrunk || descrPtr->marvellTagged)
    {
        descrPtr->localPortTrunkAsGlobalPortTrunk   = descrPtr->origSrcEPortOrTrnk;
    }
    else
    {
        descrPtr->localPortTrunkAsGlobalPortTrunk   = localSrcPort;

        if(ingressPipeDevObjPtr->supportEArch)
        {
            if(ingressPipeDevObjPtr->unitEArchEnable.txq)
            {
                descrPtr->origSrcEPortOrTrnk = descrPtr->eArchExtInfo.localDevSrcEPort;
            }
            else
            {
                descrPtr->origSrcEPortOrTrnk = globalSrcPort;
            }
        }
    }
}


/**
* @internal snetChtEgressPortGroupSharedEgressPortGroup function
* @endinternal
*
* @brief   Egress processing for specific egress 'Port group' device
*
* @param[in] portGroupSharedDevObjPtr - pointer to device object. --> the  device
* @param[in] ingressPipeDevObjPtr     - pointer to 'Ingress pipe' port group device object. --> the portGroupSharedDevObjPtr device
* @param[in] descrPtr                 - the descriptor for the 'Egress pipe' device
* @param[in] egressPortGroupId        - the index of the port group device in the array of the portGroupSharedDevObjPtr
*/
static GT_VOID snetChtEgressPortGroupSharedEgressPortGroup
(
    IN SKERNEL_DEVICE_OBJECT             * portGroupSharedDevObjPtr,
    IN SKERNEL_DEVICE_OBJECT             * ingressPipeDevObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr,
    IN GT_U32                            egressPortGroupId
)
{
    SKERNEL_DEVICE_OBJECT             * egressPipeDevObjPtr;

    descrPtr->egressOnIngressPortGroup = ((egressPortGroupId == ingressPipeDevObjPtr->portGroupId) ?
                                        1 : 0);
    /* get the egress port group pointer */
    egressPipeDevObjPtr = portGroupSharedDevObjPtr->coreDevInfoPtr[egressPortGroupId].devObjPtr;

    snetChtEgressDev(egressPipeDevObjPtr,descrPtr);
}


/**
* @internal snetChtTxqDistributorSpecific function
* @endinternal
*
* @brief   Egress processing main routine - that the portGroupSharedDevObjPtr manage
*         the function do:
*         -Port Numbers
*         Change source port to global number
*         Change target port to local number
*         - For single destination --> call specific port group device.
*         snetChtEgressDevice(port group)
*         - For multi destination --> call all port groups in loop
*         snetChtEgressDevice(currSon) , start with the Ingress pipe
*         device , Duplicate descriptors so pipes not influence each other.
* @param[in] portGroupSharedDevObjPtr - pointer to device object. --> the  device
* @param[in] ingressPipeDevObjPtr     - pointer to 'Ingress pipe' port group device object. --> the portGroupSharedDevObjPtr device
* @param[in] origDescrPtr             - Cht frame descriptor  --> the original descriptor of the 'Ingress
*                                      pipe' device
*                                      txqId  -  the TXQ unit to send the descriptor to handle
*                                      relevant in multi-TXQs
*/
static GT_VOID snetChtTxqDistributorSpecific
(
    IN SKERNEL_DEVICE_OBJECT             * portGroupSharedDevObjPtr,
    IN SKERNEL_DEVICE_OBJECT             * ingressPipeDevObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC   * origDescrPtr
)
{
    GT_U32      ii;
    GT_U32      trgPort;
    SKERNEL_FRAME_CHEETAH_DESCR_STC  *copyDescriptorPtr, *descrPtr;
    GT_U32      egressPortGroupId;
    GT_BOOL     calledFromTxdUnit;
    GT_U32      portGroupStart,portGroupLast;
    SKERNEL_DEVICE_OBJECT *egressHemisphereDevObjPtr;/*core device that represents the egress hemisphere*/

    /* get pointer to new descriptor (from the ingress port group pool)-
       don't use descriptor on the 'stack' it is very big that may cause 'stack overflow' */
    copyDescriptorPtr = snetChtEqDuplicateDescr(ingressPipeDevObjPtr, origDescrPtr);

    /* set the descriptor pointer to the copy */
    descrPtr = copyDescriptorPtr;

    if (origDescrPtr->useVidx == 0)/* non-multi destination */
    {
        if(ingressPipeDevObjPtr->numOfTxqUnits < 2)/* in SIP5 the EGF is accessed on the ingress PIPE (ingress core)*/
        {
            egressHemisphereDevObjPtr = ingressPipeDevObjPtr;
        }
        else /*Lion2*/
        {
            /* devices with more than 64 ports */

            /*core device that represents the egress hemisphere*/
            egressHemisphereDevObjPtr = portGroupSharedDevObjPtr->coreDevInfoPtr[descrPtr->txqId << 2].devObjPtr ;
            if(egressHemisphereDevObjPtr == NULL)
            {
                skernelFatalError("snetChtTxqDistributorSpecific: egressHemisphereDevObjPtr is null pointer \n");
            }
        }

        /* the functions are relevant only for UC packets */
        if(descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_CPU_E  ||
           descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E)
        {
            /* we need to know which pipe will get the packet */
            trgPort = snetChtTxQGetEgrPortForTrgDevice(egressHemisphereDevObjPtr, descrPtr, GT_TRUE);
            if(trgPort != SNET_CHT_CPU_PORT_CNS)
            {
                /* we change the port group to handle the 'to_cpu' */
                /* the macro of SMEM_CHT_PORT_GROUP_ID_FROM_GLOBAL_PORT_MAC will assume
                    that it should to other port group on port 0 */
            }
            else
            {
                /* the ingress device should handle the 'to_cpu' */
                /* the macro of SMEM_CHT_PORT_GROUP_ID_FROM_GLOBAL_PORT_MAC will assume
                    that it stay on the same port group because it has 'cpu port' number */
            }
        }
        else
        {
            /* in lion A we need to figure for remote device that is connected
               via the cascade trunk (internal trunk) , the trunkId and not the
               final port in the trunk , because Lion A is limited per port group
               to local ports of the port group in internal trunk */
            calledFromTxdUnit = egressHemisphereDevObjPtr->supportTxQGlobalPorts == 0 ?
                GT_TRUE: /*lion A */
                GT_FALSE;/*lion B */
            /* we need to know which pipe will get the packet */
            trgPort = snetChtTxQGetEgrPortForTrgDevice(egressHemisphereDevObjPtr, descrPtr, calledFromTxdUnit);
        }

        /* get the portGroupId of the egress device */
        egressPortGroupId = SMEM_CHT_PORT_GROUP_ID_FROM_GLOBAL_PORT_MAC(ingressPipeDevObjPtr, trgPort);

        if(ingressPipeDevObjPtr->dualDeviceIdEnable.txq)
        {
            /* in dual devId mode the MSB of the 'FULLY global egress port' is the actual egress Txq-unit ID*/
            egressPortGroupId |= (descrPtr->txqId << 2);
        }


        if(egressPortGroupId >= portGroupSharedDevObjPtr->numOfCoreDevs)
        {
            simWarningPrintf(" snetChtTxqDistributorSpecific : out of range port group device ?! \n");

            /* stop the egress processing */
            return;
        }
        else if(portGroupSharedDevObjPtr->coreDevInfoPtr[egressPortGroupId].validPortGroup == 0)
        {
            simWarningPrintf(" snetChtTxqDistributorSpecific : not exist port group device ?! \n");

            /* stop the egress processing */
            return;
        }

        snetChtEgressPortGroupSharedEgressPortGroup(portGroupSharedDevObjPtr,
                                                    ingressPipeDevObjPtr,
                                                    descrPtr, egressPortGroupId);
    }
    else
    {
        if(ingressPipeDevObjPtr->numOfTxqUnits < 2)/* Value 0,1 - means single TXQ */
        {
            portGroupStart = ingressPipeDevObjPtr->portGroupId;/* start loop from the local port group */
            portGroupLast = portGroupStart +  portGroupSharedDevObjPtr->numOfCoreDevs;
        }
        else
        {
            /* support several TXQ units */
            /* the start port group is in the scope of the TXQ unit */

            /* calculate the first port group of the TXQ unit */
            portGroupStart = (descrPtr->txqId) *
                (portGroupSharedDevObjPtr->numOfCoreDevs / ingressPipeDevObjPtr->numOfTxqUnits);

            /* calculate the last port group of the TXQ unit */
            portGroupLast = portGroupStart +
                (portGroupSharedDevObjPtr->numOfCoreDevs / ingressPipeDevObjPtr->numOfTxqUnits);
        }

        for(ii = portGroupStart ; ii < portGroupLast ; ii++)
        {
            egressPortGroupId = ii % portGroupSharedDevObjPtr->numOfCoreDevs;

            if(portGroupSharedDevObjPtr->coreDevInfoPtr[egressPortGroupId].validPortGroup == 0)
            {
                /* skip non valid/exists port groups */
                continue;
            }

            /* copy the original descriptor content */
            *copyDescriptorPtr = *origDescrPtr;

            snetChtEgressPortGroupSharedEgressPortGroup(portGroupSharedDevObjPtr,
                                                        ingressPipeDevObjPtr,
                                                        descrPtr, egressPortGroupId);

        }
    }

    /* save the info from the 'Temporary descriptor' */
    origDescrPtr->floodToNextTxq = copyDescriptorPtr->floodToNextTxq;
}

/**
* @internal snetChtTxqDistributorDeviceMapTableAccessCheck function
* @endinternal
*
* @brief   Check if need to access the Device map table of the TXQ distributor.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] srcPort                  - the source port
*                                      Global for port group shared device.
*                                      Local(the same value as localDevSrcPort)
*                                      for non-port group device.
* @param[in] trgDev                   - the target device
* @param[in] trgPort                  - the target port
*
* @retval GT_TRUE                  - device map need to be accessed
* @retval GT_FALSE                 - device map NOT need to be accessed
*
* @note use similar logic to snetLionTxqDeviceMapTableAccessCheck(...)
*
*/
static GT_BOOL snetChtTxqDistributorDeviceMapTableAccessCheck
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 srcPort,
    IN GT_U32 trgDev,
    IN GT_U32 trgPort
)
{
    GT_U32 regAddr;             /* Register address */
    GT_U32 * regPtr;            /* Register data pointer */
    GT_U32 fldValue;            /* Register field value */

    /* The target device is the local device (own device)*/
    if (SKERNEL_IS_MATCH_DEVICES_MAC(trgDev, descrPtr->ownDev,
                                     devObjPtr->dualDeviceIdEnable.txq))
    {
        if(!devObjPtr->supportDevMapTableOnOwnDev)
        {
            return GT_FALSE;
        }

        regAddr = SMEM_LION2_TXQ_DISTRIBUTOR_LOCAL_SRC_PORT_MAP_OWN_DEV_EN_REG(devObjPtr);
        regPtr = smemMemGet(devObjPtr, regAddr);

        fldValue = snetFieldValueGet(regPtr, srcPort, 1);
        if(fldValue == 0)
        {
            /* The local source port is disabled for device map lookup for local device
            according to the per-source-port configuration. */
            return GT_FALSE;
        }

        regAddr = SMEM_LION2_TXQ_DISTRIBUTOR_LOCAL_TRG_PORT_MAP_OWN_DEV_EN_REG(devObjPtr);
        regPtr = smemMemGet(devObjPtr, regAddr);

        fldValue = snetFieldValueGet(regPtr,trgPort,1);
        if(fldValue == 0)
        {
            /* The target port is disabled for device map lookup for local device
            according to the per-destination-port configuration. */
            return GT_FALSE;
        }
    }

    return GT_TRUE;
}

/**
* @internal snetChtTxqDistributorDeviceMapTableAddrConstruct function
* @endinternal
*
* @brief   TXQ distributor for Lion2 , access to the 'TXQ distributor Device Map Table'
*         to get the TXQ unit to serve the single destination packet
*         The device map table is accessed according to the <Dist Dev Map Table Addr Construct> register.
*         Get index into 'Device map table'
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] trgDev                   - target device
* @param[in] trgPort                  - target port
* @param[in] srcDev                   - source device
* @param[in] srcPort                  - source port
*                                       Device Map Entry bit index
*
* @note NOTE: the multi core SIP5 device will not get to this function !!!!
*       as it will use function snetChtEgressDev instead !
*
*/
static GT_U32 snetChtTxqDistributorDeviceMapTableAddrConstruct
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 trgDev,
    IN GT_U32 trgPort,
    IN GT_U32 srcDev,
    IN GT_U32 srcPort
)
{
    GT_U32 regAddr;             /* Register address */
    GT_U32 * regPtr;            /* Register data pointer */
    GT_U32 fldValue;            /* Register field value */
    GT_U32 entryIndex;          /* Table entry index */

    regAddr = SMEM_LION2_TXQ_DISTRIBUTOR_DEV_MAP_MEM_CONSTRUCT_REG(devObjPtr);
    regPtr = smemMemGet(devObjPtr, regAddr);

    fldValue = SMEM_U32_GET_FIELD(regPtr[0], 0, 4);

    entryIndex = 0;

    switch(fldValue)
    {
        case 0:
            entryIndex = trgDev;
            break;
        case 1:
            entryIndex = trgDev << 6 | (trgPort & 0x3f);
            break;
        case 2:
            entryIndex = trgDev << 5 | (srcDev & 0x1f);
            break;
        case 3:
            entryIndex = trgDev << 6 | (srcPort & 0x3f);
            break;
        case 4:
            entryIndex = trgDev << 6 |
                         (srcDev & 0x7) << 3 | (srcPort & 0x7);
            break;
        case 5:
            entryIndex = trgDev << 6 |
                         (srcDev & 0xf) << 2 | (srcPort & 0x3);
            break;
        case 6:
            entryIndex = trgDev << 6 |
                         (srcDev & 0x1f) << 1 | (srcPort & 0x1);
            break;
        case 7:
            entryIndex = trgDev << 6 |
                         (srcDev & 0x3) << 4 | (srcPort & 0xf);
            break;
        case 8:
            entryIndex = trgDev << 6 |
                         (descrPtr->srcDev & 0x1) << 5 | (srcPort & 0x1f);
            break;
        default:
            break;
    }

    return entryIndex;
}

/**
* @internal snetChtTxqDistributorDeviceMapTableAccess function
* @endinternal
*
* @brief   TXQ distributor for Lion2 , access to the 'TXQ distributor Device Map Table'
*         to get the TXQ unit to serve the single destination packet
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] trgDev                   - the target device
* @param[in] trgPort                  - the target port
*
* @note The function is relevant for single destination packets only.
*       NOTE: the multi core SIP5 device will not get to this function !!!!
*       as it will use function snetChtEgressDev instead !
*
*/
static GT_U32 snetChtTxqDistributorDeviceMapTableAccess
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 trgDev,
    IN GT_U32 trgPort
)
{
    GT_U32    regAddr; /* Egress Filtering Table Entry*/
    GT_U32   *regPtr;  /* Register data pointer */
    GT_U32    srcPort;
    GT_U32    srcDev;
    GT_U32    bitIndex;/* start bit index of the TXQ unit in the table of:
              "TXQ distributor - device map table" */

    if (descrPtr->origIsTrunk)
    {
        srcPort = descrPtr->origSrcEPortOrTrnk >> 1;
    }
    else
    {
        srcPort = descrPtr->origSrcEPortOrTrnk;
    }

    srcDev = descrPtr->srcDev;

    bitIndex = snetChtTxqDistributorDeviceMapTableAddrConstruct(devObjPtr, descrPtr,
                                                  trgDev,trgPort,
                                                  srcDev,srcPort);

    /* start table address */
    regAddr = SMEM_LION2_TXQ_DISTRIBUTOR_DEVICE_MAP_TABLE_ENTRY(devObjPtr, 0);

    /* pointer to start of memory*/
    regPtr = smemMemGet(devObjPtr,regAddr);

    return snetFieldValueGet(regPtr ,
            bitIndex * (devObjPtr->numOfTxqUnits - 1) ,
            (devObjPtr->numOfTxqUnits - 1));
}

/**
* @internal snetChtTxqDistributorTxQUnitGet function
* @endinternal
*
* @brief   TXQ distributor for Lion2 , get the TXQ unit to serve the single
*         destination packet
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*
* @note The function is relevant for single destination packets only.
*       and for devices with multi-TXQs (Lion2)
*       NOTE: the multi core SIP5 device will not get to this function !!!!
*       as it will use function snetChtEgressDev instead !
*
*/
static GT_U32 snetChtTxqDistributorTxQUnitGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetChtTxqDistributorTxQUnitGet);

    SKERNEL_MTAG_CMD_ENT outCmd;              /* Marvell tag command */
    GT_U32               trgDev;              /* Target device */
    GT_U32               txqId;/* the TXQ to send the descriptor to handle */
    GT_U32               trgEPort;/* target EPort*/

    /* Analyse the outgoing marvell tag command */
    outCmd = descrPtr->outGoingMtagCmd ;

    if ( (outCmd == SKERNEL_MTAG_CMD_FORWARD_E) ||
         (outCmd == SKERNEL_MTAG_CMD_FROM_CPU_E) )
    {
        trgDev = descrPtr->trgDev;
        trgEPort = descrPtr->trgEPort;
    }
    else if (outCmd == SKERNEL_MTAG_CMD_TO_CPU_E)
    {
        trgDev = descrPtr->cpuTrgDev;
        trgEPort = descrPtr->trgEPort;/*SNET_CHT_CPU_PORT_CNS*/
    }
    else
    {
        trgDev = descrPtr->sniffTrgDev;
        trgEPort = descrPtr->sniffTrgPort;
    }

    if (SKERNEL_IS_MATCH_DEVICES_MAC(trgDev,descrPtr->ownDev,
            devObjPtr->dualDeviceIdEnable.txq))
    {
        if(GT_FALSE == snetChtTxqDistributorDeviceMapTableAccessCheck(devObjPtr, descrPtr,
                                                            descrPtr->localDevSrcPort,
                                                            trgDev, trgEPort))
        {
            __LOG(("the device map table WILL NOT BE accessed for LOCAL DEVICE traffic \n"));

            /* not using the device map table -->
               choose hemisphere according to target device and/or port */
            /* Lion2 */
            /* TXQ_ID = 'trgDev[0]' (bit 0 ' lsb) */
            txqId = trgDev & (devObjPtr->numOfTxqUnits - 1);
            if(descrPtr->forceToCpuTrgPortOnHemisphare0)
            {
                txqId = 0;
            }

            __LOG(("the selected TXQ Hemisphere is [%d] \n",
                txqId));

            return txqId; /* local TXQ unit */
        }
    }

    __LOG(("access the TXQ distributor device map table : with trgDev [%d] trgEPort[%d] \n",
        trgDev,trgEPort));
    /* access the TXQ distributor device map table */
    txqId = snetChtTxqDistributorDeviceMapTableAccess(devObjPtr, descrPtr, trgDev,
                                        trgEPort);

    __LOG(("After access the TXQ distributor device map table : the selected TXQ Hemisphere is [%d] \n",
        txqId));
    return txqId; /* local TXQ unit */
}

/**
* @internal snetChtTxqDistributor function
* @endinternal
*
* @brief   TXQ distributed for multi-port group device.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
*/
static GT_VOID snetChtTxqDistributor
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetChtTxqDistributor);


    GT_U32  txqIdFirst;/* the TXQ to start with when flooding */
    GT_U32  ii;/*iterator*/

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* in SIP5 the EGF is accessed on the ingress PIPE (ingress core)*/
        descrPtr->egressOnIngressPortGroup = 1;
        snetChtEgressDev(devObjPtr,descrPtr);
    }
    else
    if(devObjPtr->numOfTxqUnits < 2) /* Value 0,1 - means single TXQ */
    {
        /*(legacy devices : lionB,LionA)*/
        snetChtTxqDistributorSpecific(devObjPtr->portGroupSharedDevObjPtr,
                                devObjPtr,
                                descrPtr);
    }
    else  /*Lion2*/
    {
        if(descrPtr->useVidx)
        {
            /*
                1. save copy of descriptor , if need to send to more than single TXQ unit
                2. Call with TXQ_ID = 'ownDevice[0]' (bit 0 ' lsb)
                3. if <flood_to_next_TXQ> == 1 , call the 'next TXQ'
            */
            /* take 'src' TXQ from 'ownDev' (like Lion2) */
            txqIdFirst = descrPtr->ownDev & (devObjPtr->numOfTxqUnits - 1);

            for(ii = txqIdFirst ;  /* start loop from the local TXQ */
                ii < (txqIdFirst + devObjPtr->numOfTxqUnits) ;
                ii ++)
            {
                descrPtr->txqId = ii % devObjPtr->numOfTxqUnits;

                __LOG(("Egress pipe flooding : flood hemisphere[%d] \n",
                        descrPtr->txqId));

                snetChtTxqDistributorSpecific(devObjPtr->portGroupSharedDevObjPtr,
                                        devObjPtr,
                                        descrPtr);

                if(descrPtr->floodToNextTxq == 0)
                {
                    /* this TQX decided that the next TXQ should not flood the packet ! */
                    __LOG(("this TQX decided that the next TXQ should not flood the packet !"));
                    break;
                }

            }
        }
        else
        {
            /*
                Call with TXQ_ID :
                1. if need to used device map table then from from "Pre-Device-Map-Table"
                2. else according to <ownDev[0]>
            */
            descrPtr->txqId = snetChtTxqDistributorTxQUnitGet(devObjPtr,descrPtr);
            __LOG(("Egress pipe single destination : use hemisphere[%d] \n",
                    descrPtr->txqId));
            snetChtTxqDistributorSpecific(devObjPtr->portGroupSharedDevObjPtr,
                                    devObjPtr,
                                    descrPtr);
        }
    }
}

/**
* @internal snetChtEgressGetPortsBmpFromMem function
* @endinternal
*
* @brief   Egress Pipe : build bmp of ports from the pointer to the memory (registers/table entry)
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] memPtr                   - pointer to the memory
* @param[in] fillHalfPorts            - 0 - fill all
*                                      1 - fill first half ports
*                                      2 - fill second half ports
*                                      >=64 --> the number of ports needed (from first part)
* @param[out] portsBmpPtr              - pointer to the ports bmp
*                                      COMMENTS :
*/
GT_VOID snetChtEgressGetPortsBmpFromMem
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                 *memPtr,
    OUT SKERNEL_PORTS_BMP_STC *portsBmpPtr,
    IN GT_U32                  fillHalfPorts
)
{
    GT_U32 iiMax;
    GT_U32 ii;
    GT_U32 iiStart = 0;
    GT_U32 numOfPorts;

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        numOfPorts = MAX(256,SNET_SIP5_EGR_MAX_PHY_PORTS_MAC(devObjPtr));
        /* support minimum 256 ports (cpu port 63)*/
        iiMax = (numOfPorts+31) / 32;

        if(fillHalfPorts == 1)/*first half*/
        {
            if(numOfPorts > 256)
            {
                for(ii = (iiMax/2) ; ii < iiMax; ii++)
                {
                    /* reset the second half , because second half may not be called !!! */
                    (portsBmpPtr)->ports[ii] = 0;
                }

                /*will get first 256 ports*/
                iiMax /= 2;
            }
            else
            {
                /*will get all 256 ports*/
            }
        }
        else
        if(fillHalfPorts == 2)/*second half*/
        {
            if(numOfPorts > 256)
            {
                /*will get second 256 ports*/
                iiStart = iiMax / 2;
            }
            else
            {
                /* do nothing because when called with 'first' half already got 256 ports */
                return;
            }
        }
        else
        if(fillHalfPorts >= 64)
        {
            iiMax = fillHalfPorts / 32;
        }
    }
    else if(devObjPtr->txqRevision)
    {
        /* support 64 ports (cpu port 63)*/
        iiMax = 2;
    }
    else
    {
        /* support 32 ports (cpu port 'duplicated' from 31 to 63) */
        portsBmpPtr->ports[1] = portsBmpPtr->ports[0] = memPtr[0];
        /* no more to do */
        return;
    }

    for(ii = iiStart ; ii < iiMax; ii++)
    {
        (portsBmpPtr)->ports[ii] = memPtr[ii-iiStart];
    }
}

/**
* @internal snetChtEgressPortsBmpOperators function
* @endinternal
*
* @brief   Egress Pipe :
*         BMP operators :
*         operator1 - operator on bmp 1
*         operator2 - operator on bmp 2
*         resultBmpPtr - the bmp for the result of the operators.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] operator1                - operator on bmp1
* @param[in] bmp1Ptr                  - pointer to bmp 1
* @param[in] operator2                - operator on bmp2
* @param[in] bmp2Ptr                  - pointer to bmp 2
*
* @param[out] resultBmpPtr             - pointer to the ports bmp that is the 'result'
*                                      COMMENTS :
*/
GT_VOID snetChtEgressPortsBmpOperators
(
    IN SKERNEL_DEVICE_OBJECT        * devObjPtr,
    IN SKERNEL_BITWISE_OPERATOR_ENT operator1,
    IN SKERNEL_PORTS_BMP_STC        *bmp1Ptr,
    IN SKERNEL_BITWISE_OPERATOR_ENT operator2,
    IN SKERNEL_PORTS_BMP_STC        *bmp2Ptr,
    OUT SKERNEL_PORTS_BMP_STC       *resultBmpPtr
)
{
    GT_U32 iiMax;
    GT_U32 ii;

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* support 256 ports (cpu port 63)*/
        iiMax = 8;
        if(devObjPtr->flexFieldNumBitsSupport.phyPort > 8)
        {
            iiMax <<= (devObjPtr->flexFieldNumBitsSupport.phyPort - 8);
        }
    }
    else if(devObjPtr->txqRevision)
    {
        /* support 64 ports (cpu port 63)*/
        iiMax = 2;
    }
    else
    {
        /* support 32 ports (cpu port 'duplicated' from 31 to 63) */
        iiMax = 2;
    }

    for(ii = 0 ; ii < iiMax; ii++)
    {
        resultBmpPtr->ports[ii] =
            skernelBitwiseOperator(
                (bmp1Ptr)->ports[ii],operator1,
                (bmp2Ptr)->ports[ii],operator2);
    }
}

/**
* @internal snetChtEgressPortsBmpIsEmpty function
* @endinternal
*
* @brief   Egress Pipe : is BMP empty
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] bmpPtr                   - pointer to bmp to check
*
* @retval 1                        - empty
* @retval 0                        - not empty
*                                       COMMENTS :
*/
GT_BIT snetChtEgressPortsBmpIsEmpty
(
    IN SKERNEL_DEVICE_OBJECT        * devObjPtr,
    IN SKERNEL_PORTS_BMP_STC        *bmpPtr
)
{
    GT_U32 iiMax;
    GT_U32 ii;

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* support 256 ports (cpu port 63)*/
        iiMax = 8;
        if(devObjPtr->flexFieldNumBitsSupport.phyPort > 8)
        {
            iiMax <<= (devObjPtr->flexFieldNumBitsSupport.phyPort - 8);
        }
    }
    else if(devObjPtr->txqRevision)
    {
        /* support 64 ports (cpu port 63)*/
        iiMax = 2;
    }
    else
    {
        /* support 32 ports (cpu port 'duplicated' from 31 to 63) */
        iiMax = 1;
    }

    for(ii = 0 ; ii < iiMax; ii++)
    {
        if(bmpPtr->ports[ii])
        {
            /* not empty*/
            return 0;
        }
    }

    /*empty*/
    return 1;
}




/**
* @internal snetChtEgress function
* @endinternal
*
* @brief   Egress processing main routine
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
*/
GT_VOID snetChtEgress
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetChtEgress);

    SKERNEL_FRAME_CHEETAH_DESCR_STC *origDescPtr;/* (pointer to)original descriptor info */
    SKERNEL_EXT_PACKET_CMD_ENT  updatedPacketCmd;/* updated packet command */

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_EGF_EFT_SHT_QAG_E);

    /* save descriptor info : start of fix CQ#120018 , CQ#120510 */
    /* Get pointer to duplicated descriptor */
    /* duplicate descriptor from the ingress core */
    origDescPtr = snetChtEqDuplicateDescr(descrPtr->ingressDevObjPtr,descrPtr);

    descrPtr->selectedAnalyzerIndex = descrPtr->analyzerIndex;
    descrPtr->analyzerIndex = 0;

    /* any changes in the descriptor (descrPtr) are on a copy that not harm the 'original'
       descriptor (origDescPtr)

        the original descriptor will be restored at end of the function
    */

    if (descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_CPU_E)
    {
        /* this (copy of) packet to cpu must not do any tunnel start modification */
        __LOG(("this (copy of) packet to cpu must not do any tunnel start modification"));
        descrPtr->tunnelStart = 0;
    }

    if((0 == SMEM_CHT_IS_SIP5_GET(devObjPtr)) &&
       devObjPtr->portGroupSharedDevObjPtr)
    {
        /* convert src port info (from local to global) */
        __LOG(("convert src port info (from local to global)"));
        snetChtEgressPortGroupSharedConverSrcPortInfo(devObjPtr,descrPtr);

        /* the current device is the 'Ingress pipe' port group , so we need to let
           the portGroupSharedDevObjPtr device to decide how to handle the 'txq' and how to
           sent to 'Egress pipe' port group
        */
        snetChtTxqDistributor(devObjPtr,descrPtr);

    }
    else
    {
        descrPtr->egressOnIngressPortGroup = 1;/* always set to 1 for non-port group devices */

        snetChtEgressDev(devObjPtr,descrPtr);
    }

    updatedPacketCmd = descrPtr->packetCmd;
    /* restore descriptor info : end of fix CQ#120018 , CQ#120510 */
    *descrPtr = *origDescPtr;
    /* but save info for : drop/mirror_to_cpu */
    descrPtr->packetCmd = updatedPacketCmd;
    descrPtr->packetCmdAssignedByEgress = 1;

    return;
}

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
)
{
    return snetChtTxMacPortGet(
        devObjPtr,descrPtr,
        MAC_PORT_REASON_TYPE_TXQ_DQ_TO_TXDMA_E,
        egressPort,needGlobalToLocalConvert,
        txDmaDevObjPtrPtr,txDmaLocalPortPtr);
}

/**
* @internal snetChtTxQCounterSets_forPipeDevice function
* @endinternal
*
* @brief   needed by PIPE device : Update egress counters
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
)
{
    snetChtTxQCounterSets(devObjPtr,descrPtr,destPorts, NULL /*destPortsLb*/,
                          pktFilteredPortsBmpPtr, packetType,
                          forwardRestrictionBmpPtr,counteMode,egressPort);
}

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
)
{
    return txqToPortQueueDisableCheck(devObjPtr,descrPtr,egressPort,0/*dont care*/);
}

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
)
{
    egressTxFifoAndMac(devObjPtr,descrPtr,txFifoPort,frameDataPtr,frameDataSize);
}

/**
* @internal snetSip5FwdToLbProfileGet
* @endinternal
*
* @brief   get source or target loopback profile of physical port
*
* @param[in] devObjPtr - pointer to device object.
* @param[in] phyPort   -  physical port number
* @param[in] isSrc     - GT_TRUE : get a source loopback profile
*                        GT_FALSE: get a target loopbackt profile
*
* @return a loopback profile number (0..3)
*/
static GT_U32 snetSip5FwdToLbProfileGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 phyPort,
    IN GT_BOOL isSrc
)
{
    DECLARE_FUNC_NAME(snetSip5FwdToLbProfileGet);

    GT_U32 profile;
    GT_U32 addr;
    GT_U32 *memPtr;
    GT_U32 fldId;

    if (SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
    {
        if (isSrc)
        {
            addr  = SMEM_SIP5_20_EGF_QAG_PORT_SOURCE_ATTRIBUTE_TBL_MEM(devObjPtr, phyPort);
            fldId = SMEM_SIP5_20_EGF_QAG_PORT_SOURCE_ATTRIBUTES_TABLE_FIELDS_SOURCE_PHYSICAL_PORT_LOOPBACK_PROFILE_E;
        }
        else
        {
            addr  = SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTE_TBL_MEM(devObjPtr, phyPort);
            fldId = SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_TARGET_PHYSICAL_PORT_LOOPBACK_PROFILE_E;
        }
        memPtr = smemMemGet(devObjPtr, addr);
        profile = SMEM_SIP5_20_EGF_QAG_PORT_SOURCE_FIELD_GET(devObjPtr, memPtr, phyPort, fldId);
    }
    else
    {
        addr = (isSrc ?
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.distributor.perSrcPhyPortLbProfile[phyPort / 16] :
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.distributor.perTrgPhyPortLbProfile[phyPort / 16]);
        smemRegFldGet(devObjPtr, addr, phyPort % 16 * 2, 2, &profile);
    }

    __LOG(("physical port [%d] %s loopback profile is #%d\n",
           phyPort, (isSrc? "source" : "target"), profile));

    return profile;
}

/**
* @internal snetSip5FwdToLbTriggerGenericCheck
* @endinternal
*
* @brief check 'Forwarding to Loopback/Service port'  generic trigger
*
* @param[in] devObjPtr - pointer to device object.
* @param[in] srcProfile - a source loopback profile (0..3)
* @param[in] trgProfile - a target loopback profile (0..3)
* @param[in] tc         - traffic class (used for devices >= sip5.10)
*
* @retval 0 - if generic trigger is failed
* @retval 1 - if generic trigger is passed
*/
static GT_BIT snetSip5FwdToLbTriggerGenericCheck
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 srcProfile,
    IN GT_U32 trgProfile,
    IN GT_U32 tc
)
{
    DECLARE_FUNC_NAME(snetSip5FwdToLbTriggerGenericCheck);

    GT_U32 value;
    /* check 'forwarding to loopback/service port' generic trigger  */
    if (srcProfile >=4 || trgProfile >= 4)
    {
        skernelFatalError(
            "snetSip5FwdToLbTriggerGenericCheck: impossible loopback profile(0..3): src %d, trg %d\n",
            srcProfile, trgProfile);
    }

    if (SMEM_CHT_IS_SIP5_10_GET(devObjPtr))
    {
        smemRegFldGet(devObjPtr,
                      SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.distributor.loopbackEnableConfig[srcProfile],
                      trgProfile * 8 + tc, 1, &value);
    }
    else
    {
        /* bobcat2 a0 */
        smemRegFldGet(devObjPtr,
                      SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.distributor.loopbackEnableConfig[0],
                      (srcProfile * 4 + trgProfile), 1, &value);
    }

    __LOG(("'forwarding to loopback/service port' status for profiles {src: %d, trg: %d} is %d \n",
           srcProfile,  trgProfile, value));

    return (value ? GT_TRUE : GT_FALSE);
}


/**
* @internal snetSip5FwdToLbTriggerFwdAndFromCpuCheck
* @endinternal
*
* @brief check 'Forwarding to Loopback/Service port' per-packet-type-trigger
*        for FORWARD/FROM_CPU packet type and return a loopback port if enabled.
*
* @param[in] devObjPtr     - pointer to device object.
* @param[in] isMultiTarget - GT_TRUE : check the trigger for multi-target packets
*                            GT_FALSE: check the trigger for single-target packet
* @param[in] isFromCpu     - GT_TRUE : check the trigger for FROM_CPU packet type
*                            GT_FALSE: check the trigger for FORWARD packet type
* @param[in] targetPort    - a target physical port.
*
* @param[out] lbPortPtr    - (pointer to) a loopback port number.
*                            Is relevant only if the trigger is enabled.
*                            Can be NULL
* @param[out] lbTxqPortPtr - (pointer to) a loopback TxQ port number.
*                            Is relevant only if the trigger is enabled.
*                            Is relevant for sip5 < sip5.20 devices only.
*                            Can be NULL
*
* @retval 0 - if per-packet-type trigger is failed
* @retval 1 - if per-packet-type trigger is passed
*/
static GT_BIT snetSip5FwdToLbTriggerFwdAndFromCpuCheck
(
    IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN GT_BOOL                         isMultiTarget,
    IN GT_BOOL                         isFromCpu,
    IN GT_U32                          targetPort,
    OUT GT_U32                         *lbPortPtr,
    OUT GT_U32                         *lbTxqPortPtr
)
{
    DECLARE_FUNC_NAME(snetSip5FwdToLbTriggerFwdAndFromCpuCheck);

    GT_U32 addr;
    GT_U32 *memPtr;
    GT_U32 value;
    GT_BIT enabled;
    GT_U32 lbPort;
    GT_U32 lbTxqPort;



    __LOG(("check 'forwarding to loopback/service port' per-packet-type trigger (FORWARD/FROM_CPU)\n"));
    __LOG_PARAM(isFromCpu);
    __LOG_PARAM(isMultiTarget);

    smemRegGet(devObjPtr,
               SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.distributor.loopbackForwardingGlobalConfig,
               &value);
    if (isMultiTarget)
    {
        enabled = SMEM_U32_GET_FIELD(value, 15, 1);
    }
    else
    {
        enabled = SMEM_U32_GET_FIELD(value, 14, 1);
    }
    if (isFromCpu)
    {
        enabled &= SMEM_U32_GET_FIELD(value, 16, 1);
    }

    if (enabled)
    {
        __LOG(("'Forwarding to loopback/service port' per-packet-type trigger is passed.\n"))
        if (SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
        {
            addr   = SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTE_TBL_MEM(devObjPtr, targetPort);
            memPtr = smemMemGet(devObjPtr, addr);
            lbPort = SMEM_SIP5_20_EGF_QAG_PORT_TARGET_FIELD_GET(
                devObjPtr, memPtr, targetPort,
                SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_LOOPBACK_PORT_E);
        }
        else
        {
            /* SIP 5.0, SIP 5.1x */
            addr   = SMEM_TABLE_ENTRY_INDEX_GET_MAC(devObjPtr, egfQagFwdFromCpuToLbMapper, targetPort);
            smemRegGet(devObjPtr, addr, &value);
            lbPort    = SMEM_U32_GET_FIELD(value, 0, 8);
            lbTxqPort = SMEM_U32_GET_FIELD(value, 8, 8);

            if (lbTxqPortPtr)
            {
                *lbTxqPortPtr = lbTxqPort;
            }

        }
        if (lbPortPtr)
        {
            *lbPortPtr = lbPort;
        }

        __LOG_PARAM(lbPort);
    }
    else
    {
        __LOG(("'Forwarding to loopback/service port' per-packet-type trigger is failed.\n"))
    }

    return enabled;
}


/**
* @internal snetSip5FwdToLbTriggerToAnalyzerCheck
* @endinternal
*
* @brief check 'Forwarding to Loopback/Service port' per-packet-type-trigger
*        for TO_ANALYZER packet type and return a loopback port if enabled.
*
* @param[in] devObjPtr     - pointer to device object.
* @param[in] analyzerIndex - an analyzer index
*
* @param[out] lbPortPtr    - (pointer to) a loopback port number.
*                            Is relevant only if the trigger is enabled.
*                            Can be NULL
* @param[out] lbTxqPortPtr - (pointer to) a loopback TxQ port number.
*                            Is relevant only if the trigger is enabled.
*                            Is relevant for sip5 < sip5.20 devices only.
*                            Can be NULL
*
* @retval 0 - if per-packet-type trigger is failed
* @retval 1 - if per-packet-type trigger is passed
*/
static GT_BIT snetSip5FwdToLbTriggerToAnalyzerCheck
(
    IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN GT_U32                          analyzerIndex,
    OUT GT_U32                         *lbPortPtr,
    OUT GT_U32                         *lbTxqPortPtr
)
{
    DECLARE_FUNC_NAME(snetSip5FwdToLbTriggerToAnalyzerCheck);

    GT_U32 value;
    GT_BIT enabled;
    GT_U32 lbPort;
    GT_U32 lbTxqPort = 0;

    __LOG(("check 'forwarding to loopback/service port' per-packet-type trigger (TO_ANALYZER)\n"));
    __LOG_PARAM(analyzerIndex);

    smemRegGet(devObjPtr,
               SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.distributor.toAnalyzerLoopback[analyzerIndex],
               &value);
    if (SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        enabled = SMEM_U32_GET_FIELD(value, 0, 1);
        lbPort  = SMEM_U32_GET_FIELD(value, 2, 10);

    }
    else if (SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
    {
        enabled = SMEM_U32_GET_FIELD(value, 0, 1);
        lbPort  = SMEM_U32_GET_FIELD(value, 2, 9);
    }
    else
    {
        /* sip5, sip5.1x */
        enabled     = SMEM_U32_GET_FIELD(value, 16, 1);
        lbPort      = SMEM_U32_GET_FIELD(value, 0, 8);
        lbTxqPort   = SMEM_U32_GET_FIELD(value, 8, 8);
    }

    if (enabled)
    {
        __LOG(("'Forwarding to loopback/service port' per-packet-type trigger is passed.\n"));
        __LOG_PARAM(lbPort);

        if (lbPortPtr)
        {
            *lbPortPtr = lbPort;
        }

        if (!(SMEM_CHT_IS_SIP5_20_GET(devObjPtr)))
        {
            /* sip5, sip5.1x */
            if (lbTxqPortPtr)
            {
                *lbTxqPortPtr = lbTxqPort;
            }
        }
    }
    else
    {
        __LOG(("'Forwarding to loopback/service port' per-packet-type trigger is failed.\n"))
    }

    return enabled;
}


/**
* @internal snetSip5FwdToLbTriggerToCpuCheck
* @endinternal
*
* @brief check 'Forwarding to Loopback/Service port' per-packet-type-trigger
*        for TO_CPU packet type and return a loopback port if enabled.
*
* @param[in] devObjPtr     - pointer to device object.
* @param[in] cpuCode       - CPU code
*
* @param[out] lbPortPtr    - (pointer to) a loopback port number.
*                            Is relevant only if the trigger is enabled.
*                            Can be NULL
* @param[out] lbTxqPortPtr - (pointer to) a loopback TxQ port number.
*                            Is relevant only if the trigger is enabled.
*                            Is relevant for sip5 < sip5.20 devices only.
*                            Can be NULL
*
* @retval 0 - if per-packet-type trigger is failed
* @retval 1 - if per-packet-type trigger is passed
*/
static GT_BIT snetSip5FwdToLbTriggerToCpuCheck
(
    IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN SNET_CHEETAH_CPU_CODE_ENT        cpuCode,
    OUT GT_U32                         *lbPortPtr,
    OUT GT_U32                         *lbTxqPortPtr
)
{
    DECLARE_FUNC_NAME(snetSip5FwdToLbTriggerToCpuCheck);

    GT_BIT enabled;
    GT_U32 lbPort;
    GT_U32 lbTxqPort;
    GT_U32 addr;
    GT_U32 *memPtr;

    __LOG(("check 'forwarding to loopback/service port' per-packet-type trigger (TO_CPU)\n"));
    __LOG_PARAM(cpuCode);

    addr   = SMEM_SIP5_EGF_QAG_CPU_CODE_TO_LB_MAPPER_TBL_MEM(devObjPtr, cpuCode);
    memPtr = smemMemGet(devObjPtr, addr);

    enabled =  SMEM_SIP5_EGF_QAG_CPU_CODE_TO_LB_MAPPER_FIELD_GET(
        devObjPtr, memPtr, cpuCode,
        SMEM_SIP5_EGF_QAG_CPU_CODE_TO_LB_MAPPER_TABLE_FIELDS_LOOPBACK_ENABLE_E);

    lbPort =  SMEM_SIP5_EGF_QAG_CPU_CODE_TO_LB_MAPPER_FIELD_GET(
        devObjPtr, memPtr, cpuCode,
        SMEM_SIP5_EGF_QAG_CPU_CODE_TO_LB_MAPPER_TABLE_FIELDS_LOOPBACK_PORT_E);

    lbTxqPort = SMEM_SIP5_EGF_QAG_CPU_CODE_TO_LB_MAPPER_FIELD_GET(
        devObjPtr, memPtr, cpuCode,
        SMEM_SIP5_EGF_QAG_CPU_CODE_TO_LB_MAPPER_TABLE_FIELDS_TXQ_PORT_E);

    if (enabled)
    {
        __LOG(("'Forwarding to loopback/service port' per-packet-type trigger is passed.\n"));
        __LOG_PARAM(cpuCode);
        __LOG_PARAM(lbPort);

        if (lbPortPtr)
        {
            *lbPortPtr = lbPort;
        }

        if (!(SMEM_CHT_IS_SIP5_20_GET(devObjPtr)))
        {
            /* sip5, sip5.1x */
            if (lbTxqPortPtr)
            {
                *lbTxqPortPtr = lbTxqPort;
            }
        }
    }
    else
    {
        __LOG(("'Forwarding to loopback/service port' per-packet-type trigger is failed.\n"));
    }

    return enabled;
}

/**
* @internal snetSip5FwdToLb function
* @endinternal
*
* @brief   change a packet's destination (if configured): make it forwarded to a
*          loopback/service port instead of local physical port
*
* @param[in] devObjPtr             - pointer to device object.
* @param[in] descrPtr              - pointer to the frame's descriptor.
* @param[in] destPorts[]           - number of egress port.
* @param[out] destPortsLb[]        - valuable only if destPort[N] == 1.
*                                    destPortsLb[N] format is:
*                                    bit [31] - if the packet destined to
*                                    port N is instead forwarded (1) or not (0)
*                                    to a loopback/service port.
*                                    bits[30:0] - a loopback/service port number.
*                                    Is valuable only if bit 31 is set.
*/
static void snetSip5FwdToLb
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    INOUT GT_U32 destPorts[],
    OUT   GT_U32 destPortsLb[]
)
{
    DECLARE_FUNC_NAME(snetSip5FwdToLb);

    SKERNEL_MTAG_CMD_ENT pktCmd;
    GT_U32 phyPort;       /* current port */
    GT_U32 maxPorts;      /* max ports to loop on*/
    GT_U32 srcProfile;    /* source loopback profile */
    GT_U32 trgProfile;    /* target loopback profile */
    GT_BIT isLbEnabled;
    GT_U32 lbPort;
    GT_U32 value;

    /* check generic forwarding to loopback trigger */
    __LOG(("check forwarding to loopback generic trigger\n"));

    /* get source loopback profile of source physical port */
    phyPort    = descrPtr->localDevSrcPort;
    srcProfile = snetSip5FwdToLbProfileGet(devObjPtr, phyPort, GT_TRUE);

    if (descrPtr->useVidx)
    {
        /* get 'Multi-Target eVIDX Mapping Enable */
        smemRegGet(devObjPtr,
                   SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.distributor.loopbackForwardingGlobalConfig,
                   &value);
        descrPtr->useLbVidx     = SMEM_U32_GET_FIELD(value, 1, 1);
        descrPtr->lbVidxOffset  = SMEM_U32_GET_FIELD(value, 2, 12);
    }

    maxPorts = SKERNEL_DEV_EGRESS_MAX_PORT_CNS(devObjPtr);
    for (phyPort = 0; phyPort < maxPorts; phyPort++)
    {
        if (!destPorts[phyPort])
        {
            continue;
        }

        /* get target loopback profile of target physical port */
        trgProfile = snetSip5FwdToLbProfileGet(devObjPtr, phyPort, GT_FALSE);

        isLbEnabled = snetSip5FwdToLbTriggerGenericCheck(devObjPtr, srcProfile, trgProfile, descrPtr->tc);
        if  (!isLbEnabled)
        {
            __LOG(("forwarding to loopback generic trigger is failed\n"));
            continue;
        }

        /* check 'forwarding to loopback/service port' per-packet-type trigger
           and get a loopback port number */
        __LOG(("check forwarding to loopback per-packet-type trigger\n"));
        __LOG_PARAM(descrPtr->outGoingMtagCmd);

        pktCmd = descrPtr->outGoingMtagCmd;
        switch (pktCmd)
        {
            /* loopback TXQ Port in SIP5..SIP6.16 is assigned explicitly, regardless of physical port .
               A support of this speciality is not added to WM (CPSS doesn't support it too).
               So we pass NULL to packet type trigger checking functions below.
            */
            case SKERNEL_MTAG_CMD_FROM_CPU_E:
                isLbEnabled = snetSip5FwdToLbTriggerFwdAndFromCpuCheck(
                    devObjPtr, descrPtr->useVidx, GT_TRUE, phyPort,
                    &lbPort, NULL);
                break;
            case SKERNEL_MTAG_CMD_FORWARD_E:
                isLbEnabled = snetSip5FwdToLbTriggerFwdAndFromCpuCheck(
                    devObjPtr, descrPtr->useVidx, GT_FALSE, phyPort,
                    &lbPort, NULL);
                break;
            case SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E:
                isLbEnabled = snetSip5FwdToLbTriggerToAnalyzerCheck(
                    devObjPtr, descrPtr->selectedAnalyzerIndex,
                    &lbPort, NULL);
                break;
            case SKERNEL_MTAG_CMD_TO_CPU_E:
                isLbEnabled = snetSip5FwdToLbTriggerToCpuCheck(
                    devObjPtr, descrPtr->cpuCode, &lbPort, NULL);
                break;
            default:
                skernelFatalError("snetSip5FwdToLbIsPktTypeTriggerEn: unknown packet type %d\n",
                                  pktCmd);
                isLbEnabled = 0;
                return;
        }

        if (isLbEnabled)
        {
            __LOG(("forwarding to loopback per-packet-type trigger is passed\n"));
            __LOG_PARAM_WITH_NAME("Loopback port", lbPort);
            destPortsLb[phyPort] = lbPort | BIT_31;

            /* save 'Disable Egress Mirroring to Loopback' into packet descriptior */
            smemRegFldGet(devObjPtr,
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->EGF_qag.distributor.loopbackForwardingGlobalConfig,
                0, 1, &descrPtr->disableErgMirrOnLb);

        }
        else
        {
            __LOG(("forwarding to loopback per-packet-type trigger is failed\n"));
        }
    }
}

/**
* @internal snetChtEgressCommandAndCpuCodeResolution function
* @endinternal
*
* @brief   1. resolve from old and current commands the new packet command
*         2. resolve from old and new commands the new packet cpu code (relevant
*         to copy that goes to CPU)
* @param[in] devObjPtr                - (pointer to) device object.
* @param[in,out] descrPtr                 - (pointer to) frame descriptor
* @param[in] prevCmd                  - previous command
* @param[in] currCmd                  - current command
* @param[in] prevCpuCode              - previous cpu code
* @param[in] currCpuCode              - current cpu code
* @param[in,out] descrPtr            - (pointer to) frame descriptor
*/
extern void snetChtEgressCommandAndCpuCodeResolution
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr,
    IN SKERNEL_EXT_PACKET_CMD_ENT prevCmd,
    IN SKERNEL_EXT_PACKET_CMD_ENT currCmd,
    IN GT_U32                     prevCpuCode,
    IN GT_U32                     currCpuCode
)
{
    DECLARE_FUNC_NAME(snetChtEgressCommandAndCpuCodeResolution);
    __LOG((" Apply Packet command and CPU Code Resolution \n"));
    /* Apply packet command */
    descrPtr->packetCmd = snetChtPktCmdResolution(prevCmd,currCmd);
    descrPtr->packetCmdAssignedByEgress = 1;
    /* allow to do resolution on the CPU/DROP code */
    snetChtCpuCodeResolution(devObjPtr,descrPtr,
            prevCmd,currCmd,descrPtr->packetCmd,
            prevCpuCode,currCpuCode);

}

/* check and fix access violation to table in 'half table size mode' */
void  skernelHalfTableModeViolationCheckAndReportMac(
  IN SKERNEL_DEVICE_OBJECT * devObjPtr,
  IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
  IN GT_CHAR*                          tableNameStr,
  IN GT_U32                            origIndexFromCaller,
  INOUT GT_U32                         *indexToAccessPtr,
  IN GT_U32                            fullTableSize,
  IN GT_U32                            numEntriesPerLine,
  IN GT_U32                            chunkSize
)
{
    DECLARE_FUNC_NAME(skernelHalfTableModeViolationCheckAndReportMac);
    GT_U32  indexToAccess = *indexToAccessPtr;
    GT_U32  didError = 0;
    GT_U32  newIndex;

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr) && (chunkSize > 0))
    {
        if ((chunkSize == 3 /*egress vlan tag state*/) &&  (indexToAccess > fullTableSize))
        {
            didError = 1;
        }
        else
        if ((indexToAccess + (numEntriesPerLine-1) * chunkSize) >= (fullTableSize*numEntriesPerLine)/* egf sht tables*/)
        {
            didError = 1;
        }
    }
    else /*BC3 or falcon L2i tables or egf qag tables */
    {
        if((indexToAccess) >= (GT_U32)((fullTableSize)/(numEntriesPerLine)))
        {
            didError = 1;
        }
    }

    if(didError)
    {
        newIndex = indexToAccess % (fullTableSize/numEntriesPerLine);

        simForcePrintf("ERROR : HALF_TABLE_MODE_VIOLATION : [%s][%d] is >= max supported entries[%d] ! (num entries in the 'ports mode')\n"
                       " actual_index[%d] , do 'wraparound' use index [%d] \n"
                       " PACKET WILL BE DROPPED at the end of the egress pipe ! \n",
            tableNameStr,
            origIndexFromCaller,/* 'entry index' by the caller (that not aware to 'modes' and line format) */
            fullTableSize,
            indexToAccess,/*old index*/
            newIndex      /*new index*/
            );


         /* this will cause to drop the packet */
         descrPtr->tables_read_error = 1;
         __LOG_PARAM(descrPtr->tables_read_error);
         __LOG(("tables_read_error : table [%s] index[%d] \n",
            tableNameStr,
            indexToAccess));

        indexToAccess = newIndex;
        __LOG_PARAM(indexToAccess);

        *indexToAccessPtr = indexToAccess;
    }

}

