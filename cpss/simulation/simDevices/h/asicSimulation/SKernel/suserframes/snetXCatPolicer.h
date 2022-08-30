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
* @file snetXCatPolicer.h
*
* @brief XCat Policing Engine processing for frame -- simulation
*
* @version   16
********************************************************************************
*/
#ifndef __snetXCatPolicerh
#define __snetXCatPolicerh

#include <asicSimulation/SKernel/smain/smain.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah3Policer.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Get current TOD clock value */
#define SNET_XCAT_IPFIX_TOD_CLOCK_GET_MAC(dev, cycle) \
    SIM_OS_MAC(simOsTickGet)() - (dev)->ipFixTimeStampClockDiff[(cycle)]

/* Check if counters enabled */
#define SNET_XCAT_POLICER_POLICY_COUNT_ENABLE_GET_MAC(dev, descr, direction)  \
    (((direction) == SMAIN_DIRECTION_INGRESS_E) ?                   \
                        (descr)->policerCounterEn :                 \
                        (descr)->policerEgressCntEn)

/* Enable/Disable counters */
#define SNET_XCAT_POLICER_POLICY_COUNT_ENABLE_SET_MAC(dev, descr, direction, enable)  \
    if((direction) == SMAIN_DIRECTION_INGRESS_E) {                  \
        (descr)->policerCounterEn = enable;                         \
    } else {                                                        \
        (descr)->policerEgressCntEn = enable;                       \
    }

/* define policer memory with 1792 entries */
#define POLICER_MEMORY_1792_CNS         1792
/* define policer memory with 256 entries */
#define POLICER_MEMORY_256_CNS         256

/**
* @enum SNET_XCAT_POLICER_IPFIX_COUNT_E
 *
 * @brief This enum defines Policer IPFIX counters
*/
typedef enum{

    /** IPFIX drop packets counters */
    SNET_XCAT_POLICER_IPFIX_DROP_PKTS_COUNT_E,

    /** IPFIX good packets counters */
    SNET_XCAT_POLICER_IPFIX_GOOD_PKTS_COUNT_E,

    /** IPFIX bytes counters */
    SNET_XCAT_POLICER_IPFIX_BYTES_COUNT_E,

    SNET_XCAT_POLICER_IPFIX_UNKNOWN_COUNT_E

} SNET_XCAT_POLICER_IPFIX_COUNT_E;


/* SIP5: Enumeration of Policer 'Traffic Packet Size for Metering and Counting' modes */
typedef enum {
    SNET_LION3_POLICER_PACKET_SIZE_FROM_METER_ENTRY_E,       /* calc using byte count modes in the meter entry */
    SNET_LION3_POLICER_PACKET_SIZE_FROM_BILLING_ENTRY_E,     /* calc using byte count modes in the billing entry (but not for IPFIX !!!! IPFIX uses 'other')*/
    SNET_LION3_POLICER_PACKET_SIZE_FROM_GLOBAL_CONFIG_E      /* calc using byte count modes in global configuration */
} SNET_LION3_POLICER_PACKET_SIZE_MODE_ENT;

/* SIP5: Enumeration of Policer metering modes */
typedef enum {
    SNET_LION3_POLICER_METERING_MODE_SrTCM_E,/*The metering algorithm for this entry is Single-rate Three Color Marking.;*/
    SNET_LION3_POLICER_METERING_MODE_TrTCM_E,/*The metering algorithm for this entry is Two-rate Three Color Marking.;*/
    SNET_LION3_POLICER_METERING_MODE_MEF0_E, /*Two rate metering according to MEF with CF=0;                          */
    SNET_LION3_POLICER_METERING_MODE_MEF1_E,  /*Two rate metering according to MEF with CF=1;                          */
    SNET_LION3_POLICER_METERING_MODE_MEF10_3_START_E,  /* MEF10.3 start of envelope */
    SNET_LION3_POLICER_METERING_MODE_MEF10_3_NOT_START_E  /* MEF10.3 not start envelope member */
} SNET_LION3_POLICER_METERING_MODE_ENT;

/* This structure used fo conformance level calculation. */
/* It replaces the pointer to metering entry to support  */
/* the MEF10.3 envelope of entries.                      */
/* It also used for single metering entries both SIP5    */
/* and legacy devices.                                   */
typedef struct
{
    GT_BOOL valid;
    GT_U32  envelopeSize;         /* 1 - 8 */
    GT_U32  envelopeBaseIndex;    /* used entry# envelopeBaseIndex + packetRank */
    GT_U32  packetRank;           /* 0 - 7 */
    GT_U32  *meterCfgEntryPtr;
    GT_U32  *meterConformSignLevelPtr;
    GT_U32  meterGreenBucketNumber; /* 0 or 1 */
    GT_U32  meterGreenBucketEntryIndex; /* index from table origin */
    GT_U32  *meterGreenBucketEntryPtr;
    GT_U32  meterYellowBucketNumber; /* 0 or 1 */
    GT_U32  meterYellowBucketEntryIndex; /* index from table origin */
    GT_U32  *meterYellowBucketEntryPtr;
} SNET_LION3_POLICER_METERING_DATA_STC;

/**
* @internal snetXCatCommonPolicerMeteringEnvelopeDataGet function
* @endinternal
*
* @brief   Get metering data for envelope of entries
*         Relevant for both ingeress and egress policer.
*/
GT_STATUS snetXCatCommonPolicerMeteringEnvelopeDataGet
(
    IN  SKERNEL_DEVICE_OBJECT                *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC      *descrPtr,
    IN  GT_U32                               *policerMeterEntryPtr,
    IN  GT_U32                               isEgress,
    OUT SNET_LION3_POLICER_METERING_DATA_STC *meteringDataPtr
);

/**
* @internal snetXCatCommonPolicerMeteringSingleDataGet function
* @endinternal
*
* @brief   Get metering data for single entry.
*         Relevant for both ingeress and egress policer.
*/
GT_VOID snetXCatCommonPolicerMeteringSingleDataGet
(
    IN  SKERNEL_DEVICE_OBJECT                *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC      *descrPtr,
    IN  GT_U32                               *policerMeterEntryPtr,
    OUT SNET_LION3_POLICER_METERING_DATA_STC *meteringDataPtr
);

/**
* @internal snetXcatIpfixTimestampFormat function
* @endinternal
*
* @brief   Convert tick clocks to time stamp format
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] clock                    -  value in ticks
*                                      OUTPUT:
*                                      timestampPtr    - pointer to timestamp format value
*                                      RETURN:
*
* @note Time stamp format:
*       [15:8] The eight least significant bits of the seconds field
*       [7:0] The eight most significant bits of the nanoseconds field
*
*/
GT_VOID snetXcatIpfixTimestampFormat
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 clock,
    OUT GT_U32 * timeStampPtr
);

/**
* @internal snetXcatIpfixCounterWrite function
* @endinternal
*
* @brief   Set packets/data units billing counter value
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - packet descriptor (ignored when NULL)
* @param[in] ipFixCounterRegPtr       - pointer to IPFix counter register write memory
*                                      bytes               - byte counter write value
* @param[in] packets                  -  counter write value
* @param[in] stamps                   -  counter write value
* @param[in] drops                    -  counter write value
*                                      OUTPUT:
*                                      RETURN:
*/
GT_VOID snetXcatIpfixCounterWrite
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 * ipFixCounterRegPtr,
    IN GT_U64 * bytesCntPtr,
    IN GT_U32 packets,
    IN GT_U32 stamps,
    IN GT_U32 drops
);

/**
* @internal snetXCatPolicerCounterIncrement function
* @endinternal
*
* @brief   Increment Policer Billing/Policy/VLAN Counters
*/
GT_VOID snetXCatPolicerCounterIncrement
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SMAIN_DIRECTION_ENT direction,
    IN GT_U32 * policerCtrlRegPtr,
    IN GT_U32 * policerMeterEntryPtr,
    IN GT_U32 egressPort,
    IN SKERNEL_CONFORMANCE_LEVEL_ENT cl,
    OUT GT_U32 * bytesCountPtr
);

/**
* @internal snetXCatPolicerBillingCounterIncrement function
* @endinternal
*
* @brief   Increment Billing Policer Counters
*/
GT_VOID snetXCatPolicerBillingCounterIncrement
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SMAIN_DIRECTION_ENT direction,
    IN GT_U32 * policerMeterEntryPtr,
    IN GT_U32 egressPort,
    IN SKERNEL_CONFORMANCE_LEVEL_ENT cl,
    IN GT_U32 bytesCount
);

/**
* @internal snetXCatPolicerMngCounterIncrement function
* @endinternal
*
* @brief   Increment Management Counters
*/
GT_VOID snetXCatPolicerMngCounterIncrement
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 address,
    IN GT_U32 dp,
    IN GT_U32 increment
);

/**
* @internal snetXCatEgressPolicer function
* @endinternal
*
* @brief   Egress Policer Processing, Policer Counters updates
*/
void snetXCatEgressPolicer
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 egressPort
);

/**
* @internal snetXCatPolicerOverrideMeterIndex function
* @endinternal
*
* @brief   Override if need index to policer entry
*/
GT_VOID snetXCatPolicerOverrideMeterIndex
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SMAIN_DIRECTION_ENT direction,
    INOUT GT_U32 * entryIndexPtr,
    IN GT_U32   portIndex
);

/**
* @internal snetXCatPolicerCountingEntryFormatGet function
* @endinternal
*
* @brief   Return format of counting entry - Full or Short.
*         If device do not support counting entry format select - this mean "Full" format.
*         See Policer Control1 register, "Counting Entry Format Select" field
*/
GT_U32 snetXCatPolicerCountingEntryFormatGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SMAIN_DIRECTION_ENT direction,
    IN GT_U32 cycle
);


/**
* @internal snetLion2PolicerEArchIndexGet function
* @endinternal
*
* @brief   Override if need index to policer entry
*/
GT_BOOL snetLion2PolicerEArchIndexGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SMAIN_DIRECTION_ENT direction,
    IN GT_U32       egressPort,
    INOUT GT_U32 * entryIndexPtr
);

/**
* @internal snetLion3PolicerPacketSizeGet function
* @endinternal
*
* @brief   Sip5 : Traffic Packet Size for Metering and Counting.
*         Get number of bytes for metering/billing/other .
*/
GT_VOID snetLion3PolicerPacketSizeGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SMAIN_DIRECTION_ENT direction,
    IN SNET_LION3_POLICER_PACKET_SIZE_MODE_ENT packetSizeMode,
    IN GT_U32           *entryPtr,
    OUT GT_U32          *bytesCountPtr
);

/**
* @internal snetLion3PolicerMeterSinglePacketTokenBucketApply function
* @endinternal
*
* @brief   Sip5 : Add logic for meter packet byte count compare and token bucket state.
*         The current state of the bucket is in the "Bucket Size0" and "Bucket Size1" fields.
*         Device updates these field automatically according to "Max Burst Size0" and "Max Burst Size1"
*         The logic is valid with assumption that single packet is used for testing.
*/
GT_VOID snetLion3PolicerMeterSinglePacketTokenBucketApply
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SMAIN_DIRECTION_ENT direction,
    IN SNET_LION3_POLICER_METERING_DATA_STC *meteringDataPtr,
    OUT SNET_CHT3_POLICER_QOS_INFO_STC * qosProfileInfoPtr
);

/**
* @internal snetSip6PolicerMeterMirrorCheck function
* @endinternal
*
* @brief   (sip6) function check if the meter entry request for mirror to analyzer
*          and update the descriptor if needed.
*/
GT_VOID snetSip6PolicerMeterMirrorCheck
(
    IN    SKERNEL_DEVICE_OBJECT           * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN    SNET_CHT3_POLICER_QOS_INFO_STC  * qosProfileInfoPtr,
    IN    GT_BOOL                         isEgress
);

/**
* @internal snetPlrTablesFormatInit function
* @endinternal
*
* @brief   init the format of PLR tables.(IPLR0,1 and EPLR)
*
* @param[in] devObjPtr                - pointer to device object.
*/
void snetPlrTablesFormatInit(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr
);

/**
* @internal snetPlrIpfixLookup1FirstNPacketsEntrySet function
* @endinternal
*
* @brief Set IPFIX entry for First N Packets in the IPLR0/EPLR
*/
GT_VOID snetPlrIpfixLookup1FirstNPacketsEntrySet
(
    IN    SKERNEL_DEVICE_OBJECT           * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN    SMAIN_DIRECTION_ENT               direction,
    OUT   GT_BOOL                         * setMirrorFirstPackets
);

/**
* @internal snetPlrIpfixFirstNPacketsCmdOvrd function
* @endinternal
*
* @brief Set Mirroring for First N Packets in the IPLR0/EPLR
*/
GT_VOID snetPlrIpfixFirstNPacketsCmdOvrd
(
    IN    SKERNEL_DEVICE_OBJECT           * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN    SMAIN_DIRECTION_ENT               direction,
    IN    GT_BOOL                           setMirrorFirstPackets
);

/**
* @internal snetXcatPolicerIsIpfix function
* @endinternal
*
* @brief Function checks if IPFIX should be performed or not
*/
GT_BOOL snetXcatPolicerIsIpfix
(
    IN    SKERNEL_DEVICE_OBJECT           * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

/**
* @internal snetXCatPolicerIPFixCountersFreezeCheck function
* @endinternal
*
* @brief   Check IPFix counters freeze state
*/
GT_BOOL snetXCatPolicerIPFixCountersFreezeCheck
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

/**
* @internal snetXCatPolicerIPFixCounterIncrement function
* @endinternal
*
* @brief   Increment Policer IPFix Counters
*/
GT_BOOL snetXCatPolicerIPFixCounterIncrement
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 policerCntIndex,
    IN GT_U32 bytes
);

/**
* @internal snetXCatPolicerIPFixSampling function
* @endinternal
*
* @brief   IPFIX Per-Flow Packet Sampling
*/
GT_VOID snetXCatPolicerIPFixSampling
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 policerCntIndex
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __snetXCatPolicerh */



