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
* @file prvCpssDxChHwFalconPortDeleteWa.c
*
* @brief implement WA for 'port delete' in Falcon
*
* @version   1
********************************************************************************
*/
/* use dxCh_prvCpssPpConfig[dev]  : for debugging MACRO PRV_CPSS_DXCH_PP_MAC(dev)*/
#define CPSS_LOG_IN_MODULE_ENABLE

#include <cpss/generic/cos/cpssCosTypes.h>
#include <cpss/generic/cpssHwInit/cpssHwInit.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/generic/private/prvCpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortTxPizzaResourceFalcon.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define DXCH_HWINIT_GLOVAR(_x) PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.dxChCpssHwInit._x)

/* debug flag for Falcon WA for 'port delete' of part 'channel_reset_part' */
const GT_U32 prvFalconPortDeleteWa_disabled_channel_reset_part = 0;

/*the function can be called only BEFORE 'cpssInitSystem' */
GT_STATUS prvFalconPortDeleteWa_enableSet(IN GT_U32 waNeeded)
{
    DXCH_HWINIT_GLOVAR(prvFalconPortDeleteWa_disabled) = !waNeeded;
    return GT_OK;
}

#ifdef PORT_DELETE_WA_DEBUG
/*the function can be called only BEFORE 'cpssInitSystem' */
GT_STATUS prvFalconPortDeleteWa_disabled_channel_reset_partSet(IN GT_U32 waNeeded_channel_reset_part)
{
    prvFalconPortDeleteWa_disabled_channel_reset_part =
        !waNeeded_channel_reset_part;
    return GT_OK;
}
#endif /* PORT_DELETE_WA_DEBUG */

typedef enum{
    /* do single time 'create' operation (during 'init') */
    PRV_ACTION_CREATE_E,
    /* do do last minute 'initialization' as could not be done before
       (due to order of calles from the application) */
    PRV_ACTION_CREATE_CATCH_UP_E,
    /* packets of 64 bytes : send the traffic 'from the CPU' for the WA */
    PRV_ACTION_SEND_SHORT_PACKETS_E,
    /* packets of 64 bytes : stop the sending of traffic 'from the CPU' (in SDMA 'packet generator' mode) */
    PRV_ACTION_STOP_SEND_SHORT_PACKETS_E,
    /* update the 'HW devNum' in the 'from CPU' eDSA tag (to be aligned to HW devNum of the device) */
    PRV_ACTION_UPDATE_HW_DEV_NUM_E,
    /* packets of 3000 bytes : send the traffic 'from the CPU' for the WA */
    PRV_ACTION_SEND_LONG_PACKETS_E,
    /* packets of 3000 bytes : stop the sending of traffic 'from the CPU' (in SDMA 'packet generator' mode) */
    PRV_ACTION_STOP_SEND_LONG_PACKETS_E,

}PRV_ACTION_ENT;

typedef enum{
    WA_PACKET_TYPE_SHORT_E , /*64 bytes   */
    WA_PACKET_TYPE_LONG_E    /*3000 bytes */
}WA_PACKET_SIZE_ENT;


static GT_STATUS prvFalconPortDeleteWa_PacketAction
(
    IN GT_U8    devNum,
    IN PRV_ACTION_ENT   action,
    IN GT_U32   numOfPacketsToSend
);

extern GT_STATUS hwsD2dMacChannelTxEnableSet
(
    GT_U8   devNum,
    GT_U32  d2dNum,
    GT_U32  channel,
    GT_BOOL enable
);
extern GT_STATUS hwsD2dMacChannelTxEnableGet
(
    GT_U8   devNum,
    GT_U32  d2dNum,
    GT_U32  channel,
    GT_BOOL *enablePtr
);

extern GT_STATUS prvCpssDxChTxFifoSpeedProfileConfigSet
(
    IN  GT_U8                        devNum,
    IN  PRV_CPSS_SPEED_PROFILE_ENT   speedProfile,
    IN  GT_U32                       txIndex,
    IN  GT_U32                       localChannelIndex
);
extern GT_STATUS prvCpssDxChSpeedProfileTxDmaFromPortSpeedGet
(
    IN  CPSS_PORT_SPEED_ENT         speed,
    OUT PRV_CPSS_SPEED_PROFILE_ENT  *speedProfilePtr
);
extern GT_STATUS prvCpssDxChSpeedProfileTxFifoFromPortSpeedGet
(
    IN  CPSS_PORT_SPEED_ENT         speed,
    OUT PRV_CPSS_SPEED_PROFILE_ENT  *speedProfilePtr
);
extern GT_STATUS prvCpssDxChSpeedProfileFromPortSpeedGet
(
    IN  CPSS_PORT_SPEED_ENT         speed,
    OUT PRV_CPSS_SPEED_PROFILE_ENT  *speedProfilePtr
);
extern GT_STATUS prvCpssDxChTxDmaSpeedProfileConfigSet
(
    IN  GT_U8                        devNum,
    IN  CPSS_PORT_SPEED_ENT          speed,
    IN  PRV_CPSS_SPEED_PROFILE_ENT   speedProfile,
    IN  GT_U32                       txIndex,
    IN  GT_U32                       localChannelIndex
);

extern GT_STATUS   prvCpssSip6TxDmaDebugChannelCountersEnableSet
(
    IN GT_U8    devNum,
    IN GT_U32   txDmaUnitIndex,
    IN GT_U32   localDma,
    IN GT_BOOL  enable
);
extern GT_STATUS   prvCpssSip6TxDmaDebugCounterPdsDescCounterGet
(
    IN GT_U8    devNum,
    IN GT_U32   txDmaUnitIndex,
    OUT GT_U32   *realCounterPtr,
    OUT GT_U32   *totalCounterPtr
);

GT_STATUS mvHwsMpfSauOpenDrainSet
(
    GT_U8                   devNum,
    GT_U32                  portNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 openDrain
);
GT_STATUS mvHwsMpfSauOpenDrainGet
(
    GT_U8                   devNum,
    GT_U32                  portNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *openDrainPtr
);
GT_STATUS mvHwsMpfSauStatusPortGet
(
    GT_U8                   devNum,
    GT_U32                  portNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  *dropCntValuePtr
);

static const GT_U32   prvFalconPortDeleteWa_allowOsPrintf = 0;

#ifdef PORT_DELETE_WA_DEBUG
GT_STATUS prvFalconPortDeleteWa_allowOsPrintf_set(IN GT_U32 allowPrint)
{
    prvFalconPortDeleteWa_allowOsPrintf = allowPrint;
    return GT_OK;
}
#endif /* PORT_DELETE_WA_DEBUG */

#define OS_PRINTF(x)    if(prvFalconPortDeleteWa_allowOsPrintf) cpssOsPrintf x

/* function returns MG unit index that can be used for 'TX SDMA traffic' .
    this MG is not used by any CPU-SDMA ports of the application.
   NOTE: this MG must not be muxed with 'CPU port' (from Raven) that is being used.
        (if the muxed 'CPU port'(from Raven) is not used ... this MG can be used)

   the function return 'GT_FULL' if no 'free' MG available.

    application choose <reservedCpuSdmaGlobalQueue> = 0xFFFFFFFF
    but we actually need to select proper SDMA queue
    NOTE : this value is associated with sdmaMgIndex,sdmaMgSdmaQueue
*/
static GT_STATUS getFreeCpuMgUnit(
    IN GT_U8  devNum,
    IN GT_U32 *freeCpuMgUnitPtr,
    IN GT_U8  *internal_reservedCpuSdmaGlobalQueuePtr
)
{
    devNum = devNum;
    *freeCpuMgUnitPtr = 0;
    *internal_reservedCpuSdmaGlobalQueuePtr = 0;

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED,
        "need logic like falcon_initPortDelete_WA(...) ");
}

static GT_STATUS hwFalconDriverWriteRegister
(
    IN GT_U8  devNum,
    IN GT_U32 tileIndex,
    IN GT_U32 regAddr,
    IN GT_U32 regData
)
{
    return prvCpssHwPpPortGroupWriteRegister(devNum,
        (tileIndex * PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile),
        regAddr,
        regData);
}
static GT_STATUS hwFalconDriverReadRegister
(
    IN GT_U8  devNum,
    IN GT_U32 tileIndex,
    IN GT_U32 regAddr,
    IN GT_U32 *regDataPtr
)
{
    return prvCpssHwPpPortGroupReadRegister(devNum,
        (tileIndex * PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile),
        regAddr,
        regDataPtr);
}

/*
    for packet buffer : PB GPC GPR :
    Configure all segments (accept the first one) belonging to this port to point to the first segment
    This will create fifo size of two segments matching 50G speed
*/
static GT_STATUS prvSetPbGpcGprSegmentsWa
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       txIndex,
    IN  GT_U32                       localChannelIndex,
    IN  CPSS_PORT_SPEED_ENT          cpssSpeed
)
{
    GT_STATUS   rc;
    GT_U32  regAddr,ii,iiMax;
    GT_U32  tileIndex = txIndex >>   3;/* 8 DP units in tile */
    GT_U32  gpcIndex  = txIndex &  0x7;/* 8 DP units in tile */
    GT_U32  speedInG;
    GT_U32  firstSegmentIndex;  /* index of the first segment of this channel */

    if(0 != ((1 << tileIndex) & PRV_CPSS_PP_MAC(devNum)->multiPipe.mirroredTilesBmp))
    {
        /* the tiles 1,3 : need 'mirror' conversion */
        gpcIndex = 7 - gpcIndex;
    }

    /* get the FIFO segment-start */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
        sip6_packetBuffer.gpcPacketRead[gpcIndex].fifoSegmentStart[localChannelIndex];
    rc = hwFalconDriverReadRegister(devNum, tileIndex, regAddr, &firstSegmentIndex);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch(cpssSpeed)
    {
        case CPSS_PORT_SPEED_100G_E:
        case CPSS_PORT_SPEED_102G_E:
        case CPSS_PORT_SPEED_106G_E:
            speedInG = 100;
            break;
        case CPSS_PORT_SPEED_200G_E:
            speedInG = 200;
            break;
        case CPSS_PORT_SPEED_400G_E:
        case CPSS_PORT_SPEED_424G_E:
            speedInG = 400;
            break;
        default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED,
            "unknown speed [%d]",cpssSpeed);
    }

    iiMax = (speedInG / 100) * 4;/* number of segments*/

    for(ii = 1/*skip the first one !*/; ii < iiMax; ii++)
    {
        /* write to second segment till the end to point to the first segment (50G settings) */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
            sip6_packetBuffer.gpcPacketRead[gpcIndex].fifoSegmentNext[firstSegmentIndex + ii];

        rc = hwFalconDriverWriteRegister(devNum, tileIndex, regAddr, firstSegmentIndex/*point to first*/);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/*
    when starting the WA :
        1. TXD and TXFIFO are set with 'speed profile 50G'
        2. TXD and TXFIFO  not modify PIZZA to 50G ! (keep old 100/200/400) !!!
        3. PB not need to be changed (keep old 100/200/400) !!!
    when finish the WA :
        1. TXD and TXFIFO are restore the 'speed profile' (100/200/400)
*/
static GT_STATUS prvSetTempoSpeedForWa
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       txIndex,
    IN  GT_U32                       localChannelIndex,
    IN  CPSS_PORT_SPEED_ENT          speed
)
{
    GT_STATUS   rc;
    PRV_CPSS_SPEED_PROFILE_ENT   speedProfile;

    /* get port needed TX DMA profile according to port speed*/
    rc = prvCpssDxChSpeedProfileTxDmaFromPortSpeedGet(speed, &speedProfile);
    if (rc != GT_OK)
    {
         return rc;
    }
    /* update the TxD */
    rc = prvCpssDxChTxDmaSpeedProfileConfigSet(devNum,speed,speedProfile,txIndex,localChannelIndex);
    if(rc != GT_OK)
    {
        return rc;
    }
    /* get port needed TX FIFO profile according to port speed*/
    rc = prvCpssDxChSpeedProfileTxFifoFromPortSpeedGet(speed, &speedProfile);
    if (rc != GT_OK)
    {
         return rc;
    }
    /* update the TxFifo */
    rc = prvCpssDxChTxFifoSpeedProfileConfigSet(devNum,speedProfile,txIndex,localChannelIndex);
    if(rc != GT_OK)
    {
        return rc;
    }
#if 0
    /* no need to modify the PB !!!*/
    rc = prvCpssDxChPortPacketBufferGpcChannelSpeedSet(devNum, portNum, speed);
    if(rc != GT_OK)
    {
        return rc;
    }
#endif /*0*/

    return GT_OK;
}

static const GT_U32 sleepIterOnPackets = 1;
static const GT_U32 numIter_sleepOn70Packets = 100;/* no sleeping !*/

#ifdef PORT_DELETE_WA_DEBUG
GT_STATUS sleepIterOnPackets_set(IN GT_U32 newVal)
{
    sleepIterOnPackets = newVal ? newVal : 10/*default*/;
    return GT_OK;
}

GT_STATUS numIter_sleepOn70Packets_set(IN GT_U32 newVal)
{
    numIter_sleepOn70Packets = newVal ? newVal : 20/*default*/;
    return GT_OK;
}
#endif /* PORT_DELETE_WA_DEBUG */

/* the long packets are 3000 bytes long (with the same 64 bytes 'header' as the 'short' packets )*/
#define LONG_PACKET_SIZE 3000

/* pattern for the packet to be used for the WA */
/* note: this not includes the eDSA bytes       */
static const GT_U8 prvFalconPortDeleteWa_packet[64] = {
/* 0.. 9*/ 0x00, 0x33, 0x44, 0x55, 0x66, 0x77, 0x00, 0x33, 0x44, 0x55,
/*10..19*/ 0x66, 0x88, 0x66, 0x66, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
/*20..29*/ 0x07, 0x08, 0x09, 0x0a, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
/*30..39*/ 0x17, 0x18, 0x19, 0x1a, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26,
/*40..49*/ 0x27, 0x28, 0x29, 0x2a, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
/*50..59*/ 0x37, 0x38, 0x39, 0x3a, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
/*60..63*/ 0x47, 0x48, 0x49, 0x4a};

/* the function manage built and send of packet from the HOST CPU .
    according to 'action'
*/
static GT_STATUS prvFalconPortDeleteWa_PacketAction
(
    IN GT_U8    devNum,
    IN PRV_ACTION_ENT   action,
    IN GT_U32   numOfPacketsToSend
)
{
    GT_STATUS   rc;
    GT_U32      ii;
    GT_HW_DEV_NUM               hwDevNum; /* HW device number */
    CPSS_DXCH_NET_DSA_FROM_CPU_STC  *fromCpuInfoPtr;
    PRV_CPSS_DXCH_WA_FALCON_PORT_DELETE_STC *prvInfoPtr;
    GT_U8   globalQueue;

    prvInfoPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.falconPortDeleteInfoPtr;

    if(prvInfoPtr->useInternalSramInMg == GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED,
            "not implemented working in internal SRAM of MG ! ");
    }

    if(action == PRV_ACTION_CREATE_E)
    {
        if(prvInfoPtr->isPacketInCpuValid == GT_TRUE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                "already created");
        }

        cpssOsMemSet(&prvInfoPtr->pcktParams, 0, sizeof(prvInfoPtr->pcktParams));

        prvInfoPtr->pcktParams.packetIsTagged = GT_FALSE;/* the buffer hold no tag */
        prvInfoPtr->pcktParams.cookie         = 0;

        prvInfoPtr->pcktParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E;
        prvInfoPtr->pcktParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
        prvInfoPtr->pcktParams.dsaParam.commonParams.vid = 4095;/* don't care*/

        prvInfoPtr->pcktParams.sdmaInfo.recalcCrc = GT_TRUE;
        prvInfoPtr->pcktParams.sdmaInfo.txQueue = prvInfoPtr->internal_reservedCpuSdmaGlobalQueue[WA_PACKET_TYPE_SHORT_E];
        prvInfoPtr->pcktParams.sdmaInfo.evReqHndl = 0;
        prvInfoPtr->pcktParams.sdmaInfo.invokeTxBufferQueueEvent = GT_FALSE;


        hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(devNum);

        fromCpuInfoPtr = &prvInfoPtr->pcktParams.dsaParam.dsaInfo.fromCpu;
        fromCpuInfoPtr->dstInterface.type = CPSS_INTERFACE_PORT_E;
        fromCpuInfoPtr->dstInterface.devPort.hwDevNum = hwDevNum;
        fromCpuInfoPtr->dstInterface.devPort.portNum = prvInfoPtr->applicationInfo.reservedPortNum;
        fromCpuInfoPtr->tc              = 7;
        fromCpuInfoPtr->dp              = CPSS_DP_GREEN_E;
        fromCpuInfoPtr->egrFilterEn     = GT_FALSE;
        fromCpuInfoPtr->cascadeControl  = GT_FALSE;
        fromCpuInfoPtr->egrFilterRegistered  = GT_FALSE;
        fromCpuInfoPtr->srcId           = 0;
        fromCpuInfoPtr->srcHwDev        = hwDevNum;
        fromCpuInfoPtr->extDestInfo.devPort.dstIsTagged = GT_FALSE;
        fromCpuInfoPtr->extDestInfo.devPort.mailBoxToNeighborCPU = GT_FALSE;
        fromCpuInfoPtr->isTrgPhyPortValid = GT_TRUE;
        fromCpuInfoPtr->dstEport = 0;/* not relevant */
        fromCpuInfoPtr->tag0TpidIndex = 0;

        prvInfoPtr->buffLenList[0] = sizeof(prvFalconPortDeleteWa_packet);
        /* allocate memory in the SDMA for this packet */
        prvInfoPtr->buffList[0] = cpssOsCacheDmaMalloc(prvInfoPtr->buffLenList[0]);
        cpssOsMemCpy(prvInfoPtr->buffList[0],prvFalconPortDeleteWa_packet,prvInfoPtr->buffLenList[0]);
        if(prvInfoPtr->buffList[0] == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_CREATE_ERROR,
                "failed to allocate 'cpssOsCacheDmaMalloc' for [%d] bytes",
                prvInfoPtr->buffLenList[0]);
        }

        prvInfoPtr->longBuffLenList[0] = LONG_PACKET_SIZE;/* 3000 bytes */
        /* allocate memory in the SDMA for this packet */
        prvInfoPtr->longBuffList[0] = cpssOsCacheDmaMalloc(prvInfoPtr->longBuffLenList[0]);
        /* copy only the 64 bytes from the 'short packet' (the rest of the packet is don't care) */
        cpssOsMemCpy(prvInfoPtr->longBuffList[0],prvFalconPortDeleteWa_packet,sizeof(prvFalconPortDeleteWa_packet));
        if(prvInfoPtr->longBuffList[0] == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_CREATE_ERROR,
                "failed to allocate 'cpssOsCacheDmaMalloc' for [%d] bytes",
                prvInfoPtr->longBuffLenList[0]);
        }

        if(prvInfoPtr->useBurstSdmaMode == GT_TRUE)
        {
            if(PRV_CPSS_DXCH_PP_MAC(devNum)->netIf.initDone == GT_TRUE)
            {
                /* add the short packets to 'internal_reservedCpuSdmaGlobalQueue[WA_PACKET_TYPE_SHORT_E]' */
                prvInfoPtr->pcktParams.sdmaInfo.txQueue = prvInfoPtr->internal_reservedCpuSdmaGlobalQueue[WA_PACKET_TYPE_SHORT_E];
                rc = cpssDxChNetIfSdmaTxGeneratorPacketAdd(devNum,
                                                       CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                       &prvInfoPtr->pcktParams,
                                                       prvInfoPtr->buffList[0],
                                                       prvInfoPtr->buffLenList[0],
                                                       &prvInfoPtr->sdmaBurstPacketId[WA_PACKET_TYPE_SHORT_E]);
                if (rc != GT_OK)
                {
                    return rc;
                }

                if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.falconPortDeleteInfoPtr->isWaNeeded_channel_reset_part == GT_TRUE)
                {
                    /* add the long packets to 'internal_reservedCpuSdmaGlobalQueue[WA_PACKET_TYPE_LONG_E]' */
                    prvInfoPtr->pcktParams.sdmaInfo.txQueue = prvInfoPtr->internal_reservedCpuSdmaGlobalQueue[WA_PACKET_TYPE_LONG_E];
                    rc = cpssDxChNetIfSdmaTxGeneratorPacketAdd(devNum,
                                                           CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                           &prvInfoPtr->pcktParams,
                                                           prvInfoPtr->longBuffList[0],
                                                           prvInfoPtr->longBuffLenList[0],
                                                           &prvInfoPtr->sdmaBurstPacketId[WA_PACKET_TYPE_LONG_E]);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
            }
            else
            {
                /* the function called before the netIf init !!!
                   ... ok ... lets wait with the WA ... */
                prvInfoPtr->sdmaBurstPacketId[WA_PACKET_TYPE_SHORT_E] = GT_NA;
                prvInfoPtr->sdmaBurstPacketId[WA_PACKET_TYPE_LONG_E] = GT_NA;
            }
        }
        prvInfoPtr->isPacketInCpuValid = GT_TRUE;
    }
    else
    if (action == PRV_ACTION_CREATE_CATCH_UP_E)
    {
        if(prvInfoPtr->createCatchUpDone == GT_TRUE)
        {
            /* was already done */
            return GT_OK;
        }

        prvInfoPtr->createCatchUpDone = GT_TRUE;

        /* do catch up for issues not completed during the 'create' ..
           as it was not completed during initialization */
        if(prvInfoPtr->useBurstSdmaMode == GT_TRUE)
        {
            if(prvInfoPtr->sdmaBurstPacketId[WA_PACKET_TYPE_SHORT_E] == GT_NA)
            {
                /* add the short packets to 'internal_reservedCpuSdmaGlobalQueue[WA_PACKET_TYPE_SHORT_E]' */
                prvInfoPtr->pcktParams.sdmaInfo.txQueue = prvInfoPtr->internal_reservedCpuSdmaGlobalQueue[WA_PACKET_TYPE_SHORT_E];
                rc = cpssDxChNetIfSdmaTxGeneratorPacketAdd(devNum,
                                                       CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                       &prvInfoPtr->pcktParams,
                                                       prvInfoPtr->buffList[0],
                                                       prvInfoPtr->buffLenList[0],
                                                       &prvInfoPtr->sdmaBurstPacketId[WA_PACKET_TYPE_SHORT_E]);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }

            if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.falconPortDeleteInfoPtr->isWaNeeded_channel_reset_part == GT_TRUE)
            {
                if(prvInfoPtr->sdmaBurstPacketId[WA_PACKET_TYPE_LONG_E] == GT_NA)
                {
                    /* add the long packets to 'internal_reservedCpuSdmaGlobalQueue[WA_PACKET_TYPE_LONG_E]' */
                    prvInfoPtr->pcktParams.sdmaInfo.txQueue = prvInfoPtr->internal_reservedCpuSdmaGlobalQueue[WA_PACKET_TYPE_LONG_E];
                    rc = cpssDxChNetIfSdmaTxGeneratorPacketAdd(devNum,
                                                           CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                           &prvInfoPtr->pcktParams,
                                                           prvInfoPtr->longBuffList[0],
                                                           prvInfoPtr->longBuffLenList[0],
                                                           &prvInfoPtr->sdmaBurstPacketId[WA_PACKET_TYPE_LONG_E]);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
            }
        }
    }
    else
    if(action == PRV_ACTION_UPDATE_HW_DEV_NUM_E)
    {
        if(prvInfoPtr->isPacketInCpuValid == GT_FALSE)
        {
            /* no need to update the data */
            return GT_OK;
        }

        hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(devNum);
        fromCpuInfoPtr = &prvInfoPtr->pcktParams.dsaParam.dsaInfo.fromCpu;
        fromCpuInfoPtr->dstInterface.devPort.hwDevNum = hwDevNum;

        if(prvInfoPtr->useBurstSdmaMode == GT_TRUE &&
           prvInfoPtr->sdmaBurstPacketId[WA_PACKET_TYPE_SHORT_E] != GT_NA/* GT_NA is not valid*/)
        {
            prvInfoPtr->pcktParams.sdmaInfo.txQueue = prvInfoPtr->internal_reservedCpuSdmaGlobalQueue[WA_PACKET_TYPE_SHORT_E];
            rc = cpssDxChNetIfSdmaTxGeneratorPacketUpdate(devNum,
                               CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                               prvInfoPtr->sdmaBurstPacketId[WA_PACKET_TYPE_SHORT_E],
                               &prvInfoPtr->pcktParams,
                               prvInfoPtr->buffList[0],
                               prvInfoPtr->buffLenList[0]);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        if(prvInfoPtr->useBurstSdmaMode == GT_TRUE &&
           prvInfoPtr->sdmaBurstPacketId[WA_PACKET_TYPE_LONG_E] != GT_NA/* GT_NA is not valid*/)
        {
            prvInfoPtr->pcktParams.sdmaInfo.txQueue = prvInfoPtr->internal_reservedCpuSdmaGlobalQueue[WA_PACKET_TYPE_LONG_E];
            rc = cpssDxChNetIfSdmaTxGeneratorPacketUpdate(devNum,
                               CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                               prvInfoPtr->sdmaBurstPacketId[WA_PACKET_TYPE_LONG_E],
                               &prvInfoPtr->pcktParams,
                               prvInfoPtr->longBuffList[0],
                               prvInfoPtr->longBuffLenList[0]);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    else
    if(action == PRV_ACTION_SEND_SHORT_PACKETS_E || action == PRV_ACTION_SEND_LONG_PACKETS_E)
    {
        if(prvInfoPtr->isPacketInCpuValid == GT_FALSE)
        {
            /* no need to update the data */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED,
                "The packet to send was not initialized ! (need to check order of operations)");
        }

        if(prvInfoPtr->useBurstSdmaMode == GT_TRUE)
        {
            if(action == PRV_ACTION_SEND_SHORT_PACKETS_E)
            {
                globalQueue = prvInfoPtr->internal_reservedCpuSdmaGlobalQueue[WA_PACKET_TYPE_SHORT_E];
            }
            else/*PRV_ACTION_SEND_LONG_PACKETS_E*/
            {
                globalQueue = prvInfoPtr->internal_reservedCpuSdmaGlobalQueue[WA_PACKET_TYPE_LONG_E];
            }
            /* trigger burst from the SDMA */
            rc = cpssDxChNetIfSdmaTxGeneratorEnable(devNum,
                                                    CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                    globalQueue,
                                                    GT_TRUE,/*burstEnable*/
                                                    numOfPacketsToSend/*burstPacketsNumber*/);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        else
        {
            for(ii = 0 ; ii < numOfPacketsToSend ; ii++)
            {
                rc = cpssDxChNetIfSdmaSyncTxPacketSend(devNum,
                    &prvInfoPtr->pcktParams,
                    prvInfoPtr->buffList,
                    prvInfoPtr->buffLenList,
                    1);/* single buffer */
                if(rc != GT_OK)
                {
                    return rc;
                }

                if(((ii % numIter_sleepOn70Packets) == (numIter_sleepOn70Packets-1)) &&
                    cpssDeviceRunCheck_onEmulator())
                {
                    OS_PRINTF(("Emulator : sent %d packets (out of %d) \n",
                        ii,numOfPacketsToSend));

                    /* do not burst the emulator , that may stuck the PEX */
                    cpssOsTimerWkAfter(sleepIterOnPackets);
                }
            }
        }
    }
    else
    if(action == PRV_ACTION_STOP_SEND_SHORT_PACKETS_E || action == PRV_ACTION_STOP_SEND_LONG_PACKETS_E)
    {
        if(prvInfoPtr->useBurstSdmaMode == GT_TRUE)
        {
            if(action == PRV_ACTION_STOP_SEND_SHORT_PACKETS_E)
            {
                globalQueue = prvInfoPtr->internal_reservedCpuSdmaGlobalQueue[WA_PACKET_TYPE_SHORT_E];
            }
            else/*PRV_ACTION_STOP_SEND_LONG_PACKETS_E*/
            {
                globalQueue = prvInfoPtr->internal_reservedCpuSdmaGlobalQueue[WA_PACKET_TYPE_LONG_E];
            }

            /* disable the rest of the burst */
            rc = cpssDxChNetIfSdmaTxGeneratorDisable(devNum,
                    CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                    globalQueue);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}
/* the number of packets needed for the WA */
#define NUM_PACKETS_FOR_TX_FIFO     70
#define NUM_PACKETS_FOR_SDMA_BURST (NUM_PACKETS_FOR_TX_FIFO * 10)
static const GT_U32   prvFalconPortDeleteWa_numOfPacketsInBurst = NUM_PACKETS_FOR_SDMA_BURST;
static const GT_U32   prvFalconPortDeleteWa_numOfPacketsToWaitInTxDma = NUM_PACKETS_FOR_TX_FIFO;


/* the number of packets needed for the WA for the long packets */
#define NUM_PACKETS_FOR_TX_FIFO_3000B       4
#define NUM_PACKETS_FOR_SDMA_BURST_3000B (NUM_PACKETS_FOR_TX_FIFO_3000B * 3)
static const GT_U32   prvFalconPortDeleteWa_numOfPacketsInBurst_3000B = NUM_PACKETS_FOR_SDMA_BURST_3000B;
static const GT_U32   prvFalconPortDeleteWa_numOfPacketsToWaitInTxDma_3000B = NUM_PACKETS_FOR_TX_FIFO_3000B;

#ifdef PORT_DELETE_WA_DEBUG

GT_STATUS  prvFalconPortDeleteWa_numOfPacketsInBurst_set(IN GT_U32 num)
{
    prvFalconPortDeleteWa_numOfPacketsInBurst = num ? num : NUM_PACKETS_FOR_SDMA_BURST/*default*/;
    return GT_OK;
}

GT_STATUS  prvFalconPortDeleteWa_numOfPacketsToWaitInTxDma_set(IN GT_U32 num)
{
    prvFalconPortDeleteWa_numOfPacketsToWaitInTxDma = num ? num : NUM_PACKETS_FOR_TX_FIFO/*default*/;
    return GT_OK;
}

GT_STATUS  prvFalconPortDeleteWa_numOfPacketsInBurst_3000B_set(IN GT_U32 num)
{
    prvFalconPortDeleteWa_numOfPacketsInBurst_3000B = num ? num : NUM_PACKETS_FOR_SDMA_BURST_3000B/*default*/;
    return GT_OK;
}

GT_STATUS  prvFalconPortDeleteWa_numOfPacketsToWaitInTxDma_3000B_set(IN GT_U32 num)
{
    prvFalconPortDeleteWa_numOfPacketsToWaitInTxDma_3000B = num ? num : NUM_PACKETS_FOR_TX_FIFO_3000B/*default*/;
    return GT_OK;
}
#endif /* PORT_DELETE_WA_DEBUG */

/* logic based on prvCpssDxChPortDisableWait(...) */
static GT_STATUS waitForPortToDrainInTxq(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum
)
{
    GT_STATUS   rc;
    GT_U32 timeout;
    GT_U32 portTxqBufNum = GT_NA; /* number of not treated TXQ buffers */
    GT_U32 prev_portTxqBufNum; /* previous number of not treated TXQ buffers */

    for(timeout = 100; timeout > 0; timeout--)
    {
        do
        {
            prev_portTxqBufNum = portTxqBufNum;
            rc = prvCpssFalconTxqUtilsPortTxBufNumberGet(devNum, portNum, &portTxqBufNum);
            if(rc != GT_OK)
            {
                OS_PRINTF(("prvCpssFalconTxqUtilsPortTxBufNumberGet rc [%d] \n",rc));
                return rc;
            }

            OS_PRINTF(("portTxqBufNum[%d] \n",portTxqBufNum));

            if(0 == portTxqBufNum)
            {
                break;
            }
            else
            {
                cpssOsTimerWkAfter(10);
            }
        } while (portTxqBufNum < prev_portTxqBufNum);
        if(0 == portTxqBufNum)
        {
            break;
        }
    }

    if(0 == timeout)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT,
            "wait for TXQ to drain failed (on 'orig port') but actually traffic to 'reserved port' ! ");
    }

    if(cpssDeviceRunCheck_onEmulator())
    {
        /* allow the packet after the TXQ to do : txdma,txfifo,D2D ... and then Raven */
        cpssOsTimerWkAfter(50);
    }
    else
    {
        cpssOsTimerWkAfter(1);
    }

    return GT_OK;
}

typedef struct{
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_BOOL isSlowProcess;
    GT_U32  maxIterations;
    GT_U32  maxIterations_burstId;
    GT_U32  macPortNum;
    GT_U32  dpIndex;
    GT_U32  localDmaNum;
    GT_U32  d2dNumEagle;
    GT_U32  channelEagle;
    GT_BOOL origD2dTxEnable;
    GT_BOOL origOpenDrain;
    CPSS_PORT_TX_DROP_PROFILE_SET_ENT   origTailDropProfile;
    CPSS_PORT_INTERFACE_MODE_ENT    cpssIfMode;
    CPSS_PORT_SPEED_ENT             cpssSpeed;
    MV_HWS_PORT_STANDARD            hwsIfMode;
    GT_BOOL onExitNeedToStopFromCpu;
}PORT_DELETE_INFO_STC;


static GT_STATUS internal_prvFalconPortDeleteWa_start_configSet
(
    IN GT_U8                        devNum,
    INOUT PORT_DELETE_INFO_STC      *extraWaInfoPtr
)
{
    GT_STATUS                       rc;
    PRV_CPSS_DXCH_WA_FALCON_PORT_DELETE_STC *prvInfoPtr;
    GT_U32                          txDmaPortNum;
    GT_U32                          wordsArr[8];
    GT_U32                          sauDropCounter;
    GT_U32                          pdsDescCounter;

    prvInfoPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.falconPortDeleteInfoPtr;

#ifdef ASIC_SIMULATION
    extraWaInfoPtr->isSlowProcess = GT_TRUE;
    extraWaInfoPtr->maxIterations = 1000;
    extraWaInfoPtr->maxIterations_burstId = 3;
#else
    extraWaInfoPtr->isSlowProcess = cpssDeviceRunCheck_onEmulator() ? GT_TRUE : GT_FALSE;
    extraWaInfoPtr->maxIterations = 1000;
    extraWaInfoPtr->maxIterations_burstId = 3;
#endif /*ASIC_SIMULATION*/

    prvInfoPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.falconPortDeleteInfoPtr;

    /* do last minute 'initialization' as could not be done before
       (due to order of calles from the application) */
    rc = prvFalconPortDeleteWa_PacketAction(devNum,PRV_ACTION_CREATE_CATCH_UP_E,0/*ignored*/);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* set the 'reserved port' as 'force link up' in the 'egf link filter' */
    rc = cpssDxChBrgEgrFltPortLinkEnableSet(devNum,
        prvInfoPtr->applicationInfo.reservedPortNum,
        CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* NOTE: TXDMA and TXFIFO are 1:1 in ports         */
    /* convert the physical port to TX DMA port number */
    txDmaPortNum = PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[extraWaInfoPtr->portNum].
        portMap.txDmaNum;

    extraWaInfoPtr->macPortNum   = PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[extraWaInfoPtr->portNum].
        portMap.macNum;

    /* convert the global DMA port to global DP[] and local DMA */
    rc = DMA_GLOBALNUM_TO_LOCAL_NUM_IN_DP_CONVERT_FUNC(devNum)(devNum, txDmaPortNum, &extraWaInfoPtr->dpIndex, &extraWaInfoPtr->localDmaNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* the D2D     is 1:1 with the TxFifo index (DP[index]) */
    /* the channel is 1:1 with the TxFifo DMA   (local DMA) */

    /* logic of : PRV_HWS_D2D_CONVERT_PORT_TO_D2D_IDX_MAC(...) */
    extraWaInfoPtr->d2dNumEagle  = extraWaInfoPtr->dpIndex << 1;/* the 'even' D2Ds are 'eagle side' and the 'odd' are 'Raven side' */
    /* logic of : PRV_HWS_D2D_CONVERT_PORT_TO_CHANNEL_MAC(...) */
    extraWaInfoPtr->channelEagle = extraWaInfoPtr->localDmaNum;

    /* check orig value of Tx_enable at the D2D */
    /* <D2D>Die2Die \<D2D> Qnm_Die2Die \D2D \MAC_TX \MAC TX Channel %n */
    /* <TX_ENABLE_%n>   */
    rc = hwsD2dMacChannelTxEnableGet(devNum,extraWaInfoPtr->d2dNumEagle,extraWaInfoPtr->channelEagle,&extraWaInfoPtr->origD2dTxEnable);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,
            "hwsD2dMacChannelTxEnableGet : d2dNumEagle[%d] channelEagle[%d] 'disable' operation failed ",
                extraWaInfoPtr->d2dNumEagle,extraWaInfoPtr->channelEagle);
    }

    if(extraWaInfoPtr->origD2dTxEnable != GT_FALSE)
    {
        /* Stop TXF from sending out traffic on low port index */
        /* <D2D>Die2Die \<D2D> Qnm_Die2Die \D2D \MAC_TX \MAC TX Channel %n */
        /* <TX_ENABLE_%n>   */
        rc = hwsD2dMacChannelTxEnableSet(devNum,extraWaInfoPtr->d2dNumEagle,extraWaInfoPtr->channelEagle,GT_FALSE/*disable*/);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,
                "hwsD2dMacChannelTxEnableSet : d2dNumEagle[%d] channelEagle[%d] 'disable' operation failed ",
                    extraWaInfoPtr->d2dNumEagle,extraWaInfoPtr->channelEagle);
        }
    }

    /**************************************************************/
    /* let the 'reserved port' to use the same <queue_base_index> */
    /*  as the 'portNum' is using                                 */
    /**************************************************************/
    rc = prvCpssDxChReadTableEntry(devNum,
        CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_E,
        extraWaInfoPtr->portNum,
        wordsArr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChWriteTableEntry(devNum,
        CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_E,
        prvInfoPtr->applicationInfo.reservedPortNum,
        wordsArr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* save the orig profile of the port ... because we are going to change it.
       it is changed because the 'reserved' port need no tail drops , but it is
       according to <queue_base_index> */
    rc = cpssDxChPortTxBindPortToDpGet(devNum,
            extraWaInfoPtr->portNum ,
            &extraWaInfoPtr->origTailDropProfile);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* bind the reserved port to the profile */
    /* the implementation of cpssDxChPortTxBindPortToDpSet(...) requires to
       update the 'profile' according to the <queue_base_index> of the port !

       so we must call this API every time
    */
    rc = cpssDxChPortTxBindPortToDpSet(devNum,
            prvInfoPtr->applicationInfo.reservedPortNum ,
            prvInfoPtr->applicationInfo.reservedTailDropProfile);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.falconPortDeleteInfoPtr->isWaNeeded_channel_reset_part == GT_TRUE)
    {
        /*
            for packet buffer : PB GPC GPR :
            Configure all segments (accept the first one) belonging to this port to point to the first segment
            This will create fifo size of two segments matching 50G speed
        */
        rc = prvSetPbGpcGprSegmentsWa(devNum,extraWaInfoPtr->dpIndex,extraWaInfoPtr->localDmaNum,extraWaInfoPtr->cpssSpeed);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* set the port into 50G speed , because we want this port to 'point' only to
       descriptors within 'single port' and not to 'other possible ports' */
    /*
        1. TXD and TXFIFO are set with'speed profile 50G'
        2. TXD and TXFIFO  not modify PIZZA to 50G ! (keep old 100/200/400) !!!
        3. PB not need to be changed (keep old 100/200/400) !!!
    */
    rc = prvSetTempoSpeedForWa(devNum,extraWaInfoPtr->dpIndex,extraWaInfoPtr->localDmaNum,CPSS_PORT_SPEED_50000_E);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* get the <hwsIfMode> value */
    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,extraWaInfoPtr->cpssIfMode, extraWaInfoPtr->cpssSpeed, &extraWaInfoPtr->hwsIfMode);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Set the port to open drain in the SAU */
    /* 'open drain' meaning that the SAU will drop packets , and will not send
       them to the MAC .
       because we not want to 'see' the 'from cpu' packets egress the MAC
    */

    /* Read counter to clear
        /Cider/EBU-IP/GOP/GOP MTIP/MPFS/MPFS {Current}/MPFS/MPFS Units/SAU Status
    */
    rc = mvHwsMpfSauStatusPortGet(devNum,extraWaInfoPtr->macPortNum,extraWaInfoPtr->hwsIfMode,&sauDropCounter);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Put in open drain the SAU :
        /Cider/EBU-IP/GOP/GOP MTIP/MPFS/MPFS {Current}/MPFS/MPFS Units/SAU Control
        Set bits [31:30] to 2'b01
    */
    rc = mvHwsMpfSauOpenDrainGet(devNum,extraWaInfoPtr->macPortNum,extraWaInfoPtr->hwsIfMode,&extraWaInfoPtr->origOpenDrain);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,
            "mvHwsMpfSauOpenDrainGet failed macPortNum[%d] , hwsIfMode[%d]",
            extraWaInfoPtr->macPortNum,extraWaInfoPtr->hwsIfMode);
    }

    if(extraWaInfoPtr->origOpenDrain == GT_FALSE)
    {
        /* Raven \<GOP TAP 0>GOP TAP 0 \<GOP TAP 0> <MPF>MPF \<MPF> <MPFS> MPFS \MPFS Units %a %b\PAU Control  pautx_enable*/
        rc = mvHwsMpfSauOpenDrainSet(devNum,extraWaInfoPtr->macPortNum,extraWaInfoPtr->hwsIfMode,GT_TRUE/*openDrain*/);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,
                "mvHwsMpfSauOpenDrainSet failed macPortNum[%d] , hwsIfMode[%d]",
                extraWaInfoPtr->macPortNum,extraWaInfoPtr->hwsIfMode);
        }
    }

    /* enable + reset the TxDma counters so we can wait for 70 packets to arrive
        from the SDMA in MG to the TxDma
    */
    rc = prvCpssSip6TxDmaDebugChannelCountersEnableSet(devNum,extraWaInfoPtr->dpIndex,extraWaInfoPtr->localDmaNum,GT_TRUE/*enable + reset counters*/);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,
            "prvCpssSip6TxDmaDebugChannelCountersEnableSet failed extraWaInfoPtr->dpIndex[%d] , extraWaInfoPtr->localDmaNum[%d]",
            extraWaInfoPtr->dpIndex,extraWaInfoPtr->localDmaNum);
    }

    /* get the pdsDescCounter and make sure it hold value 0 ,
       (as the previous function triggered also 'clear counters')*/
    rc = prvCpssSip6TxDmaDebugCounterPdsDescCounterGet(devNum,extraWaInfoPtr->dpIndex,&pdsDescCounter,NULL);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,
            "prvCpssSip6TxDmaDebugCounterPdsDescCounterGet failed extraWaInfoPtr->dpIndex[%d] , extraWaInfoPtr->localDmaNum[%d]",
            extraWaInfoPtr->dpIndex,extraWaInfoPtr->localDmaNum);
    }

    if(pdsDescCounter != 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                "The pdsDescCounter hold value[%d] in TXDMA[%d] but must be ZERO",
                pdsDescCounter,
                extraWaInfoPtr->dpIndex);
    }

    return GT_OK;
}

static GT_STATUS internal_prvFalconPortDeleteWa_start_sendTrafficFromCpuToFillTxFifo
(
    IN GT_U8                        devNum,
    INOUT PORT_DELETE_INFO_STC      *extraWaInfoPtr,
    IN GT_U32                       packetType
)
{
    GT_STATUS   rc;
    GT_U32      numOfPacketsInBurst;
    GT_U32      numOfPacketsToWaitInTxDma;
    GT_U32      prevPdsDescCounter,pdsDescCounter;
    GT_U32      burstId;
    GT_U32      numIterations;
    PRV_CPSS_DXCH_WA_FALCON_PORT_DELETE_STC *prvInfoPtr;

    prvInfoPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.falconPortDeleteInfoPtr;

    if(packetType == WA_PACKET_TYPE_SHORT_E)
    {
        OS_PRINTF(("start 'from cpu' short packets \n"));
        numOfPacketsInBurst       = prvFalconPortDeleteWa_numOfPacketsInBurst;
        numOfPacketsToWaitInTxDma = prvFalconPortDeleteWa_numOfPacketsToWaitInTxDma;
    }
    else/*WA_PACKET_TYPE_LONG_E*/
    {
        OS_PRINTF(("start 'from cpu'long packets \n"));
        numOfPacketsInBurst       = prvFalconPortDeleteWa_numOfPacketsInBurst_3000B;
        numOfPacketsToWaitInTxDma = prvFalconPortDeleteWa_numOfPacketsToWaitInTxDma_3000B;

        /* restore the EGF for the 'long packets' */
        (void)cpssDxChBrgEgrFltPortLinkEnableSet(devNum,
            prvInfoPtr->applicationInfo.reservedPortNum,
            CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E);

        /* re-enable + reset the TxDma counters so we can wait for 4 packets to arrive
            from the SDMA in MG to the TxDma
        */
        rc = prvCpssSip6TxDmaDebugChannelCountersEnableSet(devNum,extraWaInfoPtr->dpIndex,extraWaInfoPtr->localDmaNum,GT_TRUE/*enable + reset counters*/);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,
                "prvCpssSip6TxDmaDebugChannelCountersEnableSet failed extraWaInfoPtr->dpIndex[%d] , extraWaInfoPtr->localDmaNum[%d]",
                extraWaInfoPtr->dpIndex,extraWaInfoPtr->localDmaNum);
        }
    }

    OS_PRINTF(("trigger MG to send [%d] packets , wait for first[%d] packets \n",
        numOfPacketsInBurst ,
        numOfPacketsToWaitInTxDma));

    /* NOTE : from this point 'on error' must use 'clean exit' :
        goto clean_exit_lbl;

        to properly restore values
    */

    burstId = 0;
    pdsDescCounter = 0;/* reset it only once (not every 'do' of packet sending)*/

    do{
        numIterations = 0;

        /*************************************/
        /* send the needed number of packets */
        /*************************************/
        extraWaInfoPtr->onExitNeedToStopFromCpu = GT_TRUE;
        OS_PRINTF(("start 'from cpu' burstId [%d] \n",burstId));
        if(packetType == WA_PACKET_TYPE_SHORT_E)
        {
            rc = prvFalconPortDeleteWa_PacketAction(devNum,PRV_ACTION_SEND_SHORT_PACKETS_E,numOfPacketsInBurst);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
        else/*WA_PACKET_TYPE_LONG_E*/
        {
            rc = prvFalconPortDeleteWa_PacketAction(devNum,PRV_ACTION_SEND_LONG_PACKETS_E,numOfPacketsInBurst);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        /*********************************************/
        /* check how many packets arrived to TxDma   */
        /*********************************************/
        do{
            prevPdsDescCounter = pdsDescCounter;
            /* note : this counter is 'RO' (so not clear on read) */
            rc = prvCpssSip6TxDmaDebugCounterPdsDescCounterGet(devNum,extraWaInfoPtr->dpIndex,&pdsDescCounter,NULL);
            if(rc != GT_OK)
            {
                return rc;
            }

            OS_PRINTF(("burstId[%d] numIterations[%d] pdsDescCounter [%d] \n",burstId,numIterations,pdsDescCounter));

            if(pdsDescCounter < prevPdsDescCounter)
            {
                /* should no happen because the counter is 'RO' !!! (not 'ROC') */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                    "The pdsDescCounter value[%d] in TXDMA[%d] but previous read hold value [%d] (the counter must not 'go down')",
                        pdsDescCounter,
                        extraWaInfoPtr->dpIndex,
                        prevPdsDescCounter);
            }

            /* we need to wait for the packets to arrive*/
            if((numIterations % 10) == 9)
            {
                if(extraWaInfoPtr->isSlowProcess == GT_TRUE)
                {
                    cpssOsTimerWkAfter(50);
                }
                else
                {
                    cpssOsTimerWkAfter(1);
                }
            }

        }while(((numIterations++) < extraWaInfoPtr->maxIterations) &&
               /*(pdsDescCounter == prevPdsDescCounter) &&*/
               (pdsDescCounter < numOfPacketsToWaitInTxDma));

        if(PRV_CPSS_PP_MAC(devNum)->isGmDevice)
        {
            /* currently the GM not implement this counter */
            pdsDescCounter = numOfPacketsToWaitInTxDma;
            numIterations  = 1;/* do not fail on 'GT_TIMEOUT' */
        }

        if(
           burstId       > extraWaInfoPtr->maxIterations_burstId)
        {
            /* no packets arrived on the last burst ??? */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT,
                "The pdsDescCounter hold value[%d] in TXDMA[%d] and not reached [%d] , and on burst[%d] got no packets at all !",
                    pdsDescCounter,
                    extraWaInfoPtr->dpIndex,
                    numOfPacketsToWaitInTxDma,
                    burstId);
        }

        burstId++;

    }while(pdsDescCounter < numOfPacketsToWaitInTxDma);

    if(packetType == WA_PACKET_TYPE_SHORT_E)
    {
        OS_PRINTF(("ended 'from cpu' short packets after burstId [%d] \n",burstId-1));
    }
    else /*WA_PACKET_TYPE_LONG_E*/
    {
        OS_PRINTF(("ended 'from cpu' long packets after burstId [%d] \n",burstId-1));
    }

    return GT_OK;
}

static GT_STATUS internal_prvFalconPortDeleteWa_start_drainTheFromCpuTraffic
(
    IN GT_U8                        devNum,
    INOUT PORT_DELETE_INFO_STC      *extraWaInfoPtr,
    IN GT_U32                       packetType
)
{
    GT_STATUS   rc;
    GT_U32      pdsDescCounter;
    GT_U32      sauDropCounter,sauTotalDropCounter;
    GT_U32      numIterations;
    PRV_CPSS_DXCH_WA_FALCON_PORT_DELETE_STC *prvInfoPtr;

    prvInfoPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.falconPortDeleteInfoPtr;

    if(packetType == WA_PACKET_TYPE_SHORT_E)
    {
        OS_PRINTF(("stop the rest of 'from cpu' of short packets \n"));
        (void)prvFalconPortDeleteWa_PacketAction(devNum,PRV_ACTION_STOP_SEND_SHORT_PACKETS_E,0/*don't care*/);
    }
    else/*WA_PACKET_TYPE_LONG_E*/
    {
        OS_PRINTF(("stop the rest of 'from cpu' of long packets \n"));
        (void)prvFalconPortDeleteWa_PacketAction(devNum,PRV_ACTION_STOP_SEND_LONG_PACKETS_E,0/*don't care*/);
    }

    /* set the 'reserved port' as 'force link DOWN' in the 'egf link filter' */
    /* because the burst was for '700' although we needed only '70'          */
    /* so maybe we can kill some of them                                     */
    (void)cpssDxChBrgEgrFltPortLinkEnableSet(devNum,
        prvInfoPtr->applicationInfo.reservedPortNum,
        CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E);

    extraWaInfoPtr->onExitNeedToStopFromCpu = GT_FALSE;/* we stopped it ... so the 'clean up' not need to */

    OS_PRINTF(("start to wait for traffic to drain from all places \n"));

    /*  to allow the TXQ to drain we must do 2 things :
        1. stop traffic to get to it (EGF filter 'force link down')
        2. open the D2D to drain the TxDma,TxFifo (D2D TxEnable = GT_TRUE)
            NOTE: the D2D issue is not simulated by WM !
    */

    /* (in the D2D) enable TXF to re-sending out traffic , to allow the packets of the WA got
        leave the eagle ... but the SAU at the Raven is still in 'open drain'
        and it will drop those packets

        NOTE: we enable it after getting the 'short' packets back , to allow drain of them.
              the long packet should not be send while the D2D is TX_disabled !
    */
    (void)hwsD2dMacChannelTxEnableSet(devNum,extraWaInfoPtr->d2dNumEagle,extraWaInfoPtr->channelEagle,GT_TRUE/*enable*/);

    /* wait for TXQ to drain on 'orig port' but actually traffic to 'reserved port' !*/
    /* use the 'orig port' because it hold 'full mapping' that this function need    */
    rc = waitForPortToDrainInTxq(devNum,extraWaInfoPtr->portNum);

    OS_PRINTF(("no more traffic in the TXQ (EGF 'link down') and TXQ empty \n"));

    /* read the PDS counter again : to see the packets that came after the first 70 packets */
    (void)prvCpssSip6TxDmaDebugCounterPdsDescCounterGet(devNum,extraWaInfoPtr->dpIndex,&pdsDescCounter,NULL);

    OS_PRINTF(("(after TXQ drained) pdsDescCounter [%d] \n",
        pdsDescCounter));

    /* disable the TxDma counters.
    */
    (void)prvCpssSip6TxDmaDebugChannelCountersEnableSet(devNum,extraWaInfoPtr->dpIndex,extraWaInfoPtr->localDmaNum/*don't care*/,GT_FALSE/*disable*/);

    /*
       need to wait for the traffic from the TxFifo ...
       via the D2D to the Raven D2D ...
       to the PUA , that is dropped ('open drain')
    */
    sauTotalDropCounter = 0;
    numIterations = 0;
    do{
        /* Read counter to
            /Cider/EBU-IP/GOP/GOP MTIP/MPFS/MPFS {Current}/MPFS/MPFS Units/SAU Status
        */
        OS_PRINTF(("start MPFS SAU Status numIterations : [%d] \n",numIterations));

        (void)mvHwsMpfSauStatusPortGet(devNum,extraWaInfoPtr->macPortNum,extraWaInfoPtr->hwsIfMode,&sauDropCounter);

        OS_PRINTF(("sauDropCounter [%d] \n",sauDropCounter));

        if(PRV_CPSS_PP_MAC(devNum)->isGmDevice)
        {
            /* the GM not aware to Raven units */
            sauDropCounter = pdsDescCounter;/* make the first iteration to stop */
        }

        sauTotalDropCounter += sauDropCounter;
        numIterations ++;

        /* we need to wait for the packets to arrive*/
        if(((numIterations % 20) == 19) &&
            extraWaInfoPtr->isSlowProcess == GT_TRUE)
        {
            cpssOsTimerWkAfter(10);
        }

        if(numIterations > extraWaInfoPtr->maxIterations)
        {
            /* no packets arrived on the last burst ??? */
            CPSS_LOG_ERROR_MAC(
                "GT_TIMEOUT : The sauTotalDropCounter hold value[%d] in SAU of macPortNum[%d] and not reached [%d] ",
                    sauTotalDropCounter,
                    extraWaInfoPtr->macPortNum,
                    pdsDescCounter);
            break;
        }

    }while(sauTotalDropCounter < pdsDescCounter);

    OS_PRINTF(("ended MPFS SAU Status : numIterations[%d] \n",numIterations));

    return rc;
}

static GT_STATUS internal_prvFalconPortDeleteWa_start_configRestore
(
    IN GT_U8                        devNum,
    INOUT PORT_DELETE_INFO_STC      *extraWaInfoPtr
)
{
    /*******************************/
    /* start to restore operations */
    /*******************************/
    OS_PRINTF(("start Restore configuration \n"));

    /* NOTE : from this point must use 'rc1' and not damage the 'rc' of previous assignment */
    if(extraWaInfoPtr->onExitNeedToStopFromCpu == GT_TRUE)
    {
        OS_PRINTF(("stop the rest of 'from cpu' \n"));
        (void)prvFalconPortDeleteWa_PacketAction(devNum,PRV_ACTION_STOP_SEND_SHORT_PACKETS_E,0/*don't care*/);
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.falconPortDeleteInfoPtr->isWaNeeded_channel_reset_part == GT_TRUE)
        {
            (void)prvFalconPortDeleteWa_PacketAction(devNum,PRV_ACTION_STOP_SEND_LONG_PACKETS_E ,0/*don't care*/);
        }
    }

    /***********************************************************/
    /* restore : change the 'open drain' to 'normal' operation */
    /***********************************************************/
    if(extraWaInfoPtr->origOpenDrain == GT_FALSE)
    {
        (void)mvHwsMpfSauOpenDrainSet(devNum,extraWaInfoPtr->macPortNum,extraWaInfoPtr->hwsIfMode,extraWaInfoPtr->origOpenDrain);
    }

    /***********************************************************/
    /* restore : change the d2dTxEnable as was before the WA   */
    /***********************************************************/
    if(extraWaInfoPtr->origD2dTxEnable == GT_FALSE)
    {
        (void)hwsD2dMacChannelTxEnableSet(devNum,extraWaInfoPtr->d2dNumEagle,extraWaInfoPtr->channelEagle,extraWaInfoPtr->origD2dTxEnable);
    }

    /* restore TXD and TXFIFO  to proper 'speed profile' (according to orig speed)*/
    (void)prvSetTempoSpeedForWa(devNum,extraWaInfoPtr->dpIndex,extraWaInfoPtr->localDmaNum,extraWaInfoPtr->cpssSpeed);

    /* restore orig tail drop (was shared with 'reserved' port) */
    (void)cpssDxChPortTxBindPortToDpSet(devNum,
            extraWaInfoPtr->portNum,
            extraWaInfoPtr->origTailDropProfile);

    return GT_OK;
}


/**
* @internal internal_prvFalconPortDeleteWa_start function
* @endinternal
*
* @brief   the function that implements the WA for the port that is going down.
*           NOTE:
*           1. we are expected to be called at the end of prvCpssDxChPortDisableWait(...)
*               meaning when the 'EGF link filter' of the port is
*               already 'force down' , and the TXQ is drained.
*               BUT !! the TXQ still hold the PIZZA and resources of this 'port'
*           2. if the speed of the port was not >= 100G ... no need to call
*           this function to do the WA
*
******** function start
*       logic of this function:
*       0. do nothing for other than 100G/200G/400G
*       1. set the 'reserved port' as 'force link up' in the 'egf link filter'
*           NOTE: when it was done during 'prvFalconPortDeleteWa_initParams' ,
*           it was 'too early' , because the 'appDemo' did link down to all the ports
*           after calling 'prvFalconPortDeleteWa_initParams'
*       2. get orig value of Tx_enable at the D2D
*          set to 'disable' if 'enabled'
*       3. copy the <queue_group_index> of 'portNum' to this of 'reserved port'
*       4.  save the orig value of 'tail drop profile ' in the <queue_base_index>
*           of the 'portNum'
*       5. update tail drop profile to the 'reserved profile' in the <queue_base_index>
*           of the 'portNum'/'reserved port'
*           NOTE: this to disable reasons for congestion/'tail drop' drops
*       6. change the TXD,TxFifo profile speed to 50G (so the new packets from CPU will fill
*           the queue with 'pointers' only to the valid range of 'current port' (in TxFifo))
*          NOTE: only those places (no pizza,no PB,no D2D...)
*       7. read the 'SAU status' (drops counter) to clear it.
*       8.  get orig value of 'open drain'
*           set 'SAU control' to 'open drain' (to drop packets from getting to the MTI MAC)
*       9. enable + reset the TxDma counters so we can wait for 70 packets to arrive
*           from the SDMA in MG to the TxDma
*           TxDMA IP TLU/Units/TXD/Debug
*       10. get the pdsDescCounter (from TXD) and make sure it hold value 0 ,
*           (as the previous function triggered also 'clear counters')
*   ======
*       11. send the needed number of packets , from MG SDMA. (70 packets)
*       12. read counter : how many packets arrived to TxDma.
*           while counter not at '70' ... read the counter...
*            until max_iterations (on counter) --> return to 11
*
*            NOTE: 1. the '70' is sent again but the 'check of counter' is considering
*                the amount of packet that got to the TXD from the PDS
*                  2. until max_bursts (of 70) --> ERROR !
********
*       13. (the TXD got all packets from MG SDMA)
*           disable the TxDma counters
*       14.  (in the D2D) enable TXF to re-sending out traffic , to allow the packets
*           of the WA got leave the eagle ... but the SAU at the Raven is still in 'open drain'
*           and it will drop those packets
*       15. wait for the traffic from the TxFifo ... via the D2D to the Raven D2D ...
*            to the PUA , that is dropped ('open drain')
*           Read 'SAU status' (drop counter) and wait till 'total_sauDropCounter == pdsDescCounter'
*            until max_iterations (on counter) --> ERROR !
********
*       16. (start to restore)
*           restore the 'open drain' as was before the WA
*       17. restore the d2dTxEnable as was before the WA
*       18. restore TXD and TXFIFO  to proper 'speed profile' (according to orig speed)
*       19. restore orig tail drop (was shared with 'reserved' port)
*
******** function ended
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; xCat3; AC5x; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] portNum               - the physical port number that is doing 'PORT DELETE'
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note None
*
*/
static GT_STATUS internal_prvFalconPortDeleteWa_start
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    OUT  GT_BOOL                    *doTimeMeasurePtr
)
{
    GT_STATUS                       rc_final,rc1,rc2[2],rc3[2],rc4;
    CPSS_PORT_INTERFACE_MODE_ENT    cpssIfMode;
    CPSS_PORT_SPEED_ENT             cpssSpeed;
    PORT_DELETE_INFO_STC            extraWaInfo;
    GT_U32                          ii,iiMax;
    GT_U32                          macPortNum;/*the MAC number of the physical port*/

    *doTimeMeasurePtr = GT_FALSE;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_CHECK_MAC(devNum,portNum);/* check that we can access the DB of portsMapInfoShadowArr[portNum] */

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.falconPortDeleteInfoPtr == NULL ||
       PRV_CPSS_DXCH_PP_MAC(devNum)->port.falconPortDeleteInfoPtr->isWaNeeded == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE,
            "Function only for Falcon");
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[portNum].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"portNum[%d] is not valid",
            portNum);
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[portNum].
        portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
    {
        /* no need for the WA for 'remote port' or 'CPU SDMA port' */
        return GT_OK;
    }

    macPortNum   = PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[portNum].
        portMap.macNum;
    /* convert the CPSS speed,interface to 'HWS interface mode' */
    cpssSpeed  = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, macPortNum);
    cpssIfMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, macPortNum);

    if(cpssSpeed != CPSS_PORT_SPEED_100G_E &&
       cpssSpeed != CPSS_PORT_SPEED_200G_E &&
       cpssSpeed != CPSS_PORT_SPEED_400G_E &&
       cpssSpeed != CPSS_PORT_SPEED_424G_E &&
       cpssSpeed != CPSS_PORT_SPEED_102G_E &&
       cpssSpeed != CPSS_PORT_SPEED_106G_E)
    {
        /* the WA not needed */
        return GT_OK;
    }

    *doTimeMeasurePtr = GT_TRUE;

    OS_PRINTF(("Started the WA on port [%d] down from speed[%s] \n",portNum,
        (cpssSpeed == CPSS_PORT_SPEED_100G_E ? "100G":
         cpssSpeed == CPSS_PORT_SPEED_106G_E ? "106G":
         cpssSpeed == CPSS_PORT_SPEED_102G_E ? "102G":
         cpssSpeed == CPSS_PORT_SPEED_200G_E ? "200G":
         cpssSpeed == CPSS_PORT_SPEED_400G_E ? "400G":
         cpssSpeed == CPSS_PORT_SPEED_424G_E ? "424G": "???")
        ));

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[portNum].
        portMap.txqNum == GT_NA)
    {
        /* cascade port towards 'remote physical ports' hold no TXQ port . */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED,
            "not implemented support for cascade port [%d] towards 'remote physical ports' ",
                portNum);
    }

    cpssOsMemSet(&extraWaInfo,0,sizeof(extraWaInfo));
    extraWaInfo.portNum = portNum;
    extraWaInfo.cpssSpeed  = cpssSpeed;
    extraWaInfo.cpssIfMode = cpssIfMode;

    rc_final = GT_OK;

    rc1 = internal_prvFalconPortDeleteWa_start_configSet(devNum,
            &extraWaInfo);
    if(rc1 != GT_OK)
    {
        OS_PRINTF(("Error detected during internal_prvFalconPortDeleteWa_start_configSet portNum[%d] rc = [%d] \n",
        portNum,rc1));
    }

    rc2[1] = rc3[1] = GT_OK;/* in case we not set it */
    /* 'short' and 'long' sending */
    iiMax = (PRV_CPSS_DXCH_PP_MAC(devNum)->port.falconPortDeleteInfoPtr->isWaNeeded_channel_reset_part == GT_TRUE) ?
            WA_PACKET_TYPE_LONG_E :
            WA_PACKET_TYPE_SHORT_E;

    for(ii = WA_PACKET_TYPE_SHORT_E ; ii <= iiMax ; ii++)
    {
        rc2[ii] = internal_prvFalconPortDeleteWa_start_sendTrafficFromCpuToFillTxFifo(devNum,
                &extraWaInfo,
                ii);
        if(rc2[ii] != GT_OK)
        {
            OS_PRINTF(("Error detected during internal_prvFalconPortDeleteWa_start_sendTrafficFromCpuToFillTxFifo short packets on portNum[%d] rc = [%d] \n",
            portNum,rc2[ii]));
        }

        rc3[ii] = internal_prvFalconPortDeleteWa_start_drainTheFromCpuTraffic(devNum,
                &extraWaInfo,
                ii);
        if(rc3[ii] != GT_OK)
        {
            OS_PRINTF(("Error detected during internal_prvFalconPortDeleteWa_start_drainTheFromCpuTraffic short packets on portNum[%d] rc = [%d] \n",
            portNum,rc3[ii]));
        }
    }

    rc4 = internal_prvFalconPortDeleteWa_start_configRestore(devNum,
            &extraWaInfo);
    if(rc4 != GT_OK)
    {
        OS_PRINTF(("Error detected during internal_prvFalconPortDeleteWa_start_configRestore portNum[%d] rc = [%d] \n",
        portNum,rc4));
    }

    rc_final = rc1 + rc2[0] + rc2[1] + rc3[0] + rc3[1] + rc4;


    if(rc_final != GT_OK)
    {
        if(rc1 != GT_OK)
        {
            rc_final = rc1;
        }
        else
        if(rc2[0] != GT_OK)
        {
            rc_final = rc2[0];
        }
        else
        if(rc3[0] != GT_OK)
        {
            rc_final = rc3[0];
        }
        else
        if(rc2[1] != GT_OK)
        {
            rc_final = rc2[1];
        }
        else
        if(rc3[1] != GT_OK)
        {
            rc_final = rc3[1];
        }
        else
        {
            rc_final = rc4;
        }

        OS_PRINTF(("Error detected during 'PORT DELETE' WA portNum[%d] rc = [%d] \n",
            portNum,
            rc_final));

        CPSS_LOG_ERROR_AND_RETURN_MAC(rc_final,"The 'PORT DELETE' WA failed (portNum[%d])",
            portNum);
    }

    OS_PRINTF(("SUCCESS for 'PORT DELETE' WA portNum[%d] \n",portNum));

    return GT_OK;
}

/**
* @internal prvCpssDxChFalconPortDeleteWa_start function
* @endinternal
*
* @brief   the function that implements the WA for the port that is going down.
*           NOTE:
*           1. we are expected to be called at the end of prvCpssDxChPortDisableWait(...)
*               meaning when the 'EGF link filter' of the port is
*               already 'force down' , and the TXQ is drained.
*               BUT !! the TXQ still hold the PIZZA and resources of this 'port'
*           2. if the speed of the port was not >= 100G ... no need to call
*           this function to do the WA
*
*       logic of this function:
*       1. copy the <queue_group_index> of 'portNum' to this of 'reserved port'
*       2. change the TxFifo speed to 50G (so the new packets from CPU will fill
*           the queue with 'pointers' only to the valid range of 'current port' (in TxFifo))
*       3. disable reasons for congestion/'tail drop' drops.
*       3. send the needed amount of packets
*       4. check that the 'counters' that state that WA can be over.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; xCat3; AC5x; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] portNum               - the physical port number that is doing 'PORT DELETE'
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note None
*
*/
GT_STATUS prvCpssDxChFalconPortDeleteWa_start
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum
)
{
    GT_STATUS   rc,rc1;
    GT_U32  secondsStart, secondsEnd,
            nanoSecondsStart, nanoSecondsEnd,
            seconds, nanoSec; /* time of init */
    GT_BOOL doTimeMeasure;

    if(DXCH_HWINIT_GLOVAR(prvFalconPortDeleteWa_disabled))
    {
        /**************************************/
        /* the WA is disabled (debug purposes)*/
        /**************************************/
        return GT_OK;
    }

    rc1 = cpssOsTimeRT(&secondsStart,&nanoSecondsStart);
    if(rc1 != GT_OK)
    {
        return rc1;
    }

    rc = internal_prvFalconPortDeleteWa_start(devNum,portNum,&doTimeMeasure);
    if(doTimeMeasure == GT_FALSE)
    {
        return rc;
    }

    /* WA time measurement */
    rc1 = cpssOsTimeRT(&secondsEnd,&nanoSecondsEnd);
    if(rc1 != GT_OK)
    {
        return rc1;
    }

    seconds = secondsEnd-secondsStart;
    if(nanoSecondsEnd >= nanoSecondsStart)
    {
        nanoSec = nanoSecondsEnd-nanoSecondsStart;
    }
    else
    {
        nanoSec = (1000000000 - nanoSecondsStart) + nanoSecondsEnd;
        seconds--;
    }
    OS_PRINTF(("prvFalconPortDeleteWa_start time: %d sec., %d nanosec. (rc[%d])\n",
        seconds, nanoSec,rc));

    return rc;
}

/**
* @internal prvCpssDxChFalconPortDeleteWa_updateHwDevNum function
* @endinternal
*
* @brief   state that the HW devNum was updated .
*           called only after PRV_CPSS_HW_DEV_NUM_MAC(devNum) was updated.
*           needed by the 'PORT DELETE' workaround (WA) in Falcon.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; xCat3; AC5x; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note None
*
*/
GT_STATUS prvCpssDxChFalconPortDeleteWa_updateHwDevNum
(
    IN GT_U8    devNum
)
{
    if(DXCH_HWINIT_GLOVAR(prvFalconPortDeleteWa_disabled))
    {
        /**************************************/
        /* the WA is disabled (debug purposes)*/
        /**************************************/
        return GT_OK;
    }

    return prvFalconPortDeleteWa_PacketAction(devNum,PRV_ACTION_UPDATE_HW_DEV_NUM_E,0);
}

/**
* @internal prvCpssDxChFalconPortDeleteWa_PacketBufferInit function
* @endinternal
*
* @brief Initialization for the WA for the PB (packet buffer)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum         - device number
* @param[in] tileIndex      - index of tile
*                             (APPLICABLE RANGES: 0..3)
* @param[in] gpcIndex       - index of GPC (Group of Port Channels)
*                             (APPLICABLE RANGES: 0..7)
*
* @retval  GT_OK                    - on success
* @retval  GT_BAD_PARAM             - on wrong parameters
* @retval  GT_OUT_OF_RANGE          - on out of parameters
* @retval  GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval  GT_HW_ERROR              - on hardware error
* @retval  GT_FAIL                  - otherwise
*
* @note
*/
GT_STATUS prvCpssDxChFalconPortDeleteWa_PacketBufferInit
(
    IN  GT_U8                                                 devNum,
    IN  GT_U32                                                tileIndex,
    IN  GT_U32                                                gpcIndex
)
{
    GT_STATUS   rc;
    GT_U32  channelIndex,regAddr,regVal,ii;

    if(DXCH_HWINIT_GLOVAR(prvFalconPortDeleteWa_disabled))
    {
        /**************************************/
        /* the WA is disabled (debug purposes)*/
        /**************************************/
        return GT_OK;
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.falconPortDeleteInfoPtr->isWaNeeded_channel_reset_part == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
            "should not be called ! caller must do 'non-WA' stuff");
    }
    /* this is single time operation after OOR (out of reset) that need to be done */
    /* Part 1: Only after reset we should assert channel reset to all channels 7 times (at least 100 ns between resets) */
    /* We need only 7 times since after OOR we sent credits for 50G channel on all 8 channels              */
    /* This will cause GPR to send credits to all channels, that are sufficient for every speed up to 400G.*/
    /* The reset will reset the GPR internal credits so no need to worry from overflow inside GPR          */
    /* reset channel after changing configuration */

    regVal = 1;

    /* do it 7 times */
    for(ii = 0 ; ii < 7 ; ii++)
    {
        /* on the 8 channels */
        for(channelIndex = 0 ; channelIndex < 8; channelIndex++)
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                sip6_packetBuffer.gpcPacketRead[gpcIndex].channelReset[channelIndex];

            rc = hwFalconDriverWriteRegister(devNum, tileIndex, regAddr, regVal);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

typedef struct{
    /* fields in 'reorder 0' */
    GT_U32  ReorderChannelCreditCounterHeadTail;/*  0.. 7 */
    GT_U32  ReorderChannelCreditCounterLevel0;  /*  8..15 */
    GT_U32  ReorderChannelCreditCounterLevel1;  /* 16..23 */
    GT_U32  ReorderChannelCreditCounterLevel2;  /* 16..23 */
    /* fields in 'reorder 1' */
    GT_U32  ReorderChannelCreditCounterCell;    /*  0.. 7 */

    /* fields in 'command queue 0' */
    GT_U32  CommandQueueCreditCounterHeadTail;  /*  0.. 7 */
    GT_U32  CommandQueueCreditCounterLevel0;    /*  8..15 */
    GT_U32  CommandQueueCreditCounterLevel1;    /* 16..23 */
    GT_U32  CommandQueueCreditCounterLevel2;    /* 16..23 */
    /* fields in 'command queue 1' */
    GT_U32  CommandQueueCreditCounterCell;      /*  0.. 9 */
}GPC_VALUES_STC;

static GPC_VALUES_STC gpcValues_25G = {
    /* fields in 'reorder 0' */
     3/* ReorderChannelCreditCounterHeadTail;*  0.. 7 */
    ,1/* ReorderChannelCreditCounterLevel0;  *  8..15 */
    ,1/* ReorderChannelCreditCounterLevel1;  * 16..23 */
    ,1/* ReorderChannelCreditCounterLevel2;  * 16..23 */
    /* fields in 'reorder 1' */
    ,7/* ReorderChannelCreditCounterCell;    *  0.. 7 */

    /* fields in 'command queue 0' */
    ,3/* CommandQueueCreditCounterHeadTail;  *  0.. 7 */ /* not scaled */
    ,3/* CommandQueueCreditCounterLevel0;    *  8..15 */
    ,3/* CommandQueueCreditCounterLevel1;    * 16..23 */
    ,3/* CommandQueueCreditCounterLevel2;    * 16..23 */
    /* fields in 'command queue 1' */
    ,17/* CommandQueueCreditCounterCell;      *  0.. 9 */

};


/**
* @internal prvCpssDxChFalconPortDeleteWa_PacketBufferPortCreate function
* @endinternal
*
* @brief  Do 'port create'  WA configurations that relate to packet buffer (PB)
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; xCat3; AC5x; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileIndex      - index of tile
*                             (APPLICABLE RANGES: 0..3)
* @param[in] gpcIndex       - index of GPC (Group of Port Channels)
*                             (APPLICABLE RANGES: 0..7)
* @param[in] channelIndex   - channel index.
*                             channels 0..7 related to network ports - configured only at runtime
*                             channel 8 related to CPU port - initialized to 25G speed,
*                             can be changed at runtime,
*                             channel 9 related to TXQ - initialized to 50G speed,
*                             cannot be changed at runtime,
*                             (APPLICABLE RANGES: 0..9)
* @param[in] speed          - port speed enum value
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note None
*
*/
GT_STATUS prvCpssDxChFalconPortDeleteWa_PacketBufferPortCreate
(
    IN GT_U8                        devNum,
    IN  GT_U32                      tileIndex,
    IN  GT_U32                      gpcIndex,
    IN  GT_U32                      channelIndex,
    IN  CPSS_PORT_SPEED_ENT         cpssSpeed
)
{
    GT_STATUS   rc;
    GT_U32  regAddr,regVal;
    GT_U32  speedInG;
    GT_U32  factorFromPattern;
    GPC_VALUES_STC  valuesInfo;

    if(DXCH_HWINIT_GLOVAR(prvFalconPortDeleteWa_disabled))
    {
        /**************************************/
        /* the WA is disabled (debug purposes)*/
        /**************************************/
        return GT_OK;
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.falconPortDeleteInfoPtr->isWaNeeded_channel_reset_part == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"should not be called !");
    }

    /* convert enum speed value to speed measured in 1G units. */
    /* round up the value to support speeds less than 1G as 1G */
    speedInG = ((prvCpssCommonPortSpeedEnumToMbPerSecConvert(cpssSpeed) + 999) / 1000);
    if (speedInG < 1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
            "function to be called only with valid speed");
    }

    if (speedInG < 100)
    {
        speedInG = 50;
    }
    else if (speedInG < 200)
    {
        speedInG = 100;
    }
    else if (speedInG < 400)
    {
        speedInG = 200;
    }
    else
    {
        speedInG = 400;
    }

    factorFromPattern = speedInG / 25;

    valuesInfo = gpcValues_25G;/* copy 25G values */
    /* apply the factor */
    valuesInfo.ReorderChannelCreditCounterHeadTail *= factorFromPattern;
    valuesInfo.ReorderChannelCreditCounterLevel0   *= factorFromPattern;
    valuesInfo.ReorderChannelCreditCounterLevel1   *= factorFromPattern;
    valuesInfo.ReorderChannelCreditCounterLevel2   *= factorFromPattern;
    valuesInfo.ReorderChannelCreditCounterCell     *= factorFromPattern;
    /* valuesInfo.CommandQueueCreditCounterHeadTail not scaled */
    valuesInfo.CommandQueueCreditCounterLevel0     *= factorFromPattern;
    valuesInfo.CommandQueueCreditCounterLevel1     *= factorFromPattern;
    valuesInfo.CommandQueueCreditCounterLevel2     *= factorFromPattern;
    valuesInfo.CommandQueueCreditCounterCell       *= factorFromPattern;

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                sip6_packetBuffer.gpcPacketRead[gpcIndex].
                debug.credits.creditCntControl;
    regVal = channelIndex;
    rc = hwFalconDriverWriteRegister(devNum, tileIndex, regAddr, regVal);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* update the 10 filfos :
       1. 4 in first register
       2. 1 in next register
       3. 4 in next register
       4. 1 in next register
    */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                sip6_packetBuffer.gpcPacketRead[gpcIndex].
                debug.credits.reorderChannelCreditCnt0;
    regVal =
        valuesInfo.ReorderChannelCreditCounterHeadTail <<  0 |
        valuesInfo.ReorderChannelCreditCounterLevel0   <<  8 |
        valuesInfo.ReorderChannelCreditCounterLevel1   << 16 |
        valuesInfo.ReorderChannelCreditCounterLevel2   << 24 ;
    rc = hwFalconDriverWriteRegister(devNum, tileIndex, regAddr, regVal);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                sip6_packetBuffer.gpcPacketRead[gpcIndex].
                debug.credits.reorderChannelCreditCnt1;
    regVal = valuesInfo.ReorderChannelCreditCounterCell;
    rc = hwFalconDriverWriteRegister(devNum, tileIndex, regAddr, regVal);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                sip6_packetBuffer.gpcPacketRead[gpcIndex].
                debug.credits.commandQueueCreditCnt0;
    regVal =
        valuesInfo.CommandQueueCreditCounterHeadTail <<  0 |
        valuesInfo.CommandQueueCreditCounterLevel0   <<  8 |
        valuesInfo.CommandQueueCreditCounterLevel1   << 16 |
        valuesInfo.CommandQueueCreditCounterLevel2   << 24 ;
    rc = hwFalconDriverWriteRegister(devNum, tileIndex, regAddr, regVal);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->
                sip6_packetBuffer.gpcPacketRead[gpcIndex].
                debug.credits.commandQueueCreditCnt1;
    regVal = valuesInfo.CommandQueueCreditCounterCell;
    rc = hwFalconDriverWriteRegister(devNum, tileIndex, regAddr, regVal);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}



/**
* @internal prvCpssDxChFalconPortDeleteWa_initParams_tailDrop function
* @endinternal
*
* @brief   init the tail drop parameters for the WA:
*   The purpose is create profile that set tail drop limits to maximal possible value
*
    cpssDxChPortTxTailDropWredProfileSet :
        guaranteedLimit 0xfffff
        wredSize  0x0
        probability - do not care since no wred


    cpssDxChPortTx4TcTailDropProfileSet :
        dp0QueueAlpha = dp1QueueAlpha= dp2QueueAlpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_32_E
        dp0MaxBuffNum = dp1MaxBuffNum= dp2MaxBuffNum = not applicable for Falcon
        dp0MaxDescrNum = dp1MaxDescrNum = dp2MaxDescrNum = not applicable for Falcon
        tcMaxBuffNum = 0xfffff

* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; xCat3; AC5x; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] infoPtr                  - (pointer to) the needed info
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note None
*
*/
static GT_STATUS prvCpssDxChFalconPortDeleteWa_initParams_tailDrop(
    IN GT_U8                        devNum
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_WA_FALCON_PORT_DELETE_STC *prvInfoPtr;
    CPSS_PORT_TAIL_DROP_WRED_PARAMS_STC  tailDropWredProfileParams;
    CPSS_PORT_TX_DROP_PROFILE_SET_ENT   waProfile;
    CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS_STC tailDropProfileParams;
    GT_U8  tc;

    prvInfoPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.falconPortDeleteInfoPtr;
    waProfile = prvInfoPtr->applicationInfo.reservedTailDropProfile;

    tailDropWredProfileParams.guaranteedLimit = 0xfffff;
    tailDropWredProfileParams.wredSize        = 0;
    tailDropWredProfileParams.probability     = CPSS_PORT_TX_TAIL_DROP_PROBABILITY_100_E; /* do not care since no wred */

    rc = cpssDxChPortTxTailDropWredProfileSet(devNum,waProfile,&tailDropWredProfileParams);
    if(rc != GT_OK)
    {
        return rc;
    }

    cpssOsMemSet(&tailDropProfileParams,0,sizeof(tailDropProfileParams));

    tailDropProfileParams.dp0QueueAlpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_32_E;
    tailDropProfileParams.dp1QueueAlpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_32_E;
    tailDropProfileParams.dp2QueueAlpha = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_32_E;
    tailDropProfileParams.tcMaxBuffNum  = 0xfffff;

    for(tc = 0 ; tc < 8 ; tc++)
    {
        rc = cpssDxChPortTx4TcTailDropProfileSet(devNum, waProfile, tc , &tailDropProfileParams);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChFalconPortDeleteWa_initParams_mgSdmaPackets function
* @endinternal
*
* @brief   init the MG SDMA TX for sending packets for the WA:
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; xCat3; AC5x; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] infoPtr                  - (pointer to) the needed info
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note None
*
*/
static GT_STATUS prvCpssDxChFalconPortDeleteWa_initParams_mgSdmaPackets
(
    IN GT_U8                        devNum
)
{
    GT_STATUS   rc;

    rc = prvFalconPortDeleteWa_PacketAction(devNum,PRV_ACTION_CREATE_E,0/*ignored*/);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChFalconPortDeleteWa_initParams function
* @endinternal
*
* @brief   state the dedicated resources needed by the 'PORT DELETE' workaround (WA)
*           in Falcon.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; xCat3; AC5x; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] infoPtr                  - (pointer to) the needed info
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note None
*
*/
GT_STATUS prvCpssDxChFalconPortDeleteWa_initParams(
    IN GT_U8                        devNum,
    IN CPSS_DXCH_HW_PP_IMPLEMENT_WA_INIT_FALCON_PORT_DELETE_STC *infoPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_WA_FALCON_PORT_DELETE_STC *prvInfoPtr;
    GT_U32      netIfNum;
    GT_U32      mgUnitId;/*the MG to serve the SDMA operation*/
    GT_U32      sdmaMgSdmaQueue;/* the (local) SDMA queue (0..7) to use for the sending. NOTE: currently we expect value 7 only ! */
    GT_U32      ii;/* iterator */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,  CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E
                                          | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    CPSS_NULL_PTR_CHECK_MAC(infoPtr);
    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_CHECK_MAC(devNum,infoPtr->reservedPortNum);
    PRV_CPSS_DXCH_DROP_PROFILE_CHECK_MAC(infoPtr->reservedTailDropProfile,devNum);

    if(DXCH_HWINIT_GLOVAR(prvFalconPortDeleteWa_disabled))
    {
        /**************************************/
        /* the WA is disabled (debug purposes)*/
        /**************************************/
        return GT_OK;
    }

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.falconPortDeleteInfoPtr == NULL ||
       PRV_CPSS_DXCH_PP_MAC(devNum)->port.falconPortDeleteInfoPtr->isWaNeeded == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE,
            "Function only for Falcon");
    }

    OS_PRINTF((
        "initialize the 'port delete' WA : reservedPortNum[%d] ,reservedCpuSdmaGlobalQueue[%d,%d] , reservedTailDropProfile[CPSS_PORT_TX_DROP_PROFILE_%d_E]\n",
        infoPtr->reservedPortNum,
        infoPtr->reservedCpuSdmaGlobalQueue[WA_PACKET_TYPE_SHORT_E],
        infoPtr->reservedCpuSdmaGlobalQueue[WA_PACKET_TYPE_LONG_E],
        (infoPtr->reservedTailDropProfile+1)));

    prvInfoPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.falconPortDeleteInfoPtr;

    if(infoPtr->reservedCpuSdmaGlobalQueue[WA_PACKET_TYPE_SHORT_E] != 0xFFFFFFFF)
    {
        /*************************************************************/
        /* the application gave specific queues to be used by the WA */
        /*************************************************************/

        for(ii = 0 ; ii < 2 ; ii++)
        {
            /* make sure that the CPU SDMA Queue is valid (that application mapped !) */
            PRV_CPSS_CHECK_SDMA_Q_MAC(devNum,infoPtr->reservedCpuSdmaGlobalQueue[ii]);

            netIfNum    = infoPtr->reservedCpuSdmaGlobalQueue[ii]>>3;
            /************************************/
            /* convert netIfNum to mgUnitId     */
            /************************************/
            PRV_CPSS_DXCH_NETIF_CHECK_NETIF_NUM_AND_CONVERT_TO_MG_UNIT_ID_MAC(devNum,netIfNum,mgUnitId);
            sdmaMgSdmaQueue = infoPtr->reservedCpuSdmaGlobalQueue[ii] & 7;

            prvInfoPtr->internal_reservedCpuSdmaGlobalQueue[ii] = (GT_U8)infoPtr->reservedCpuSdmaGlobalQueue[ii];

            prvInfoPtr->sdmaMgIndex[ii]       = mgUnitId;
            prvInfoPtr->sdmaMgSdmaQueue[ii]   = sdmaMgSdmaQueue;
            prvInfoPtr->sdmaBurstPacketId[ii] = GT_NA;/* not initialized yet */
        }
    }
    else
    {
        /* The application is aware that there is at least one CPU SDMA port
           that is not used and we can use 2 queues */
        rc = getFreeCpuMgUnit(devNum,&mgUnitId,prvInfoPtr->internal_reservedCpuSdmaGlobalQueue);
        if(rc != GT_OK)
        {
            return rc;
        }

        sdmaMgSdmaQueue = 7;

        /**************************************************/
        /* we need to initialize the SDMA of this MG unit */
        /**************************************************/
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED,
            "not implemented the initialize the SDMA [%d] of this MG unit [%d] ",
                sdmaMgSdmaQueue,
                mgUnitId);
    }

    /*************************************************************/
    /* make sure that the port was not mapped by the application */
    /*************************************************************/
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[infoPtr->reservedPortNum].valid == GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "reservedPortNum [%d] must not be used , but it is bound to DMA[%d] ",
            infoPtr->reservedPortNum,
            PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[infoPtr->reservedPortNum].portMap.rxDmaNum);
    }

    /* save the info from the application */
    prvInfoPtr->applicationInfo = *infoPtr;

    /* start the implementation by sending traffic from the CPU */
    /* NOTE: in the future ... we may switch to 'GT_TRUE'       */
    prvInfoPtr->useInternalSramInMg = GT_FALSE;

#ifdef ASIC_SIMULATION
    /* in WM and GM : the only supported solution is by sending traffic from the CPU */
    prvInfoPtr->useInternalSramInMg = GT_FALSE;
#endif

    prvInfoPtr->useBurstSdmaMode = GT_TRUE;


    /* init the tail drop profile */
    rc = prvCpssDxChFalconPortDeleteWa_initParams_tailDrop(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* init the packet for the SDMA */
    rc = prvCpssDxChFalconPortDeleteWa_initParams_mgSdmaPackets(devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    OS_PRINTF((
        "'port delete' WA internal info : useBurstSdmaMode[%d] ,useInternalSramInMg[%d] , {sdmaMgIndex[%d], sdmaMgSdmaQueue[%d]} , {sdmaMgIndex[%d], sdmaMgSdmaQueue[%d]}\n",
        prvInfoPtr->useBurstSdmaMode,
        prvInfoPtr->useInternalSramInMg,
        prvInfoPtr->sdmaMgIndex[0],
        prvInfoPtr->sdmaMgSdmaQueue[0],
        prvInfoPtr->sdmaMgIndex[1],
        prvInfoPtr->sdmaMgSdmaQueue[1]
        ));

    return GT_OK;
}

