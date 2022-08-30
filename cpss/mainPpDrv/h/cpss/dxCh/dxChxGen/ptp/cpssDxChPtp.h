
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
* @file cpssDxChPtp.h
*
* @brief CPSS DxCh Precision Time Protocol APIs
*
* @version   22
********************************************************************************
*/

#ifndef __cpssDxChPtph
#define __cpssDxChPtph

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/generic/port/cpssPortCtrl.h>

/**
* @enum CPSS_DXCH_PTP_TAI_INSTANCE_ENT
 *
 * @brief Instance of TAI type (contains 2 TAI Units).
 * (APPLICABLE DEVICES: Bobcat2)
*/
typedef enum{

    /** Global Instance Only */
    CPSS_DXCH_PTP_TAI_INSTANCE_GLOBAL_E,

    /** @brief Instance identified by port
     *  There is one instance per several ports typically for 4 ports.
     */
    CPSS_DXCH_PTP_TAI_INSTANCE_PORT_E,

    /** All Instances */
    CPSS_DXCH_PTP_TAI_INSTANCE_ALL_E

} CPSS_DXCH_PTP_TAI_INSTANCE_ENT;

/**
* @enum CPSS_DXCH_PTP_U_BIT_MODE_ENT
 *
 * @brief U bit mode.
 * (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef enum{

    CPSS_DXCH_PTP_TAI_U_BIT_MODE_ASSIGNED_E,

    CPSS_DXCH_PTP_TAI_U_BIT_MODE_COUNTER_E,

    CPSS_DXCH_PTP_TAI_U_BIT_MODE_PROGRAMMED_E

} CPSS_DXCH_PTP_U_BIT_MODE_ENT;


/**
* @enum CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT
 *
 * @brief PulseIn configuration mode.
 * (APPLICABLE DEVICES: Aldrin2; Falcon; AC5P; AC5X; Harrier)
*/
typedef enum{

    /** @brief Pulse IN connected to PTP_PULSE_IN
     */
    CPSS_DXCH_PTP_TAI_PULSE_IN_IS_PTP_PULSE_IN_E,

    /** @brief Pulse IN is connected to CLOCK_IN
     */
    CPSS_DXCH_PTP_TAI_PULSE_IN_IS_PTP_CLOCK_IN_E,

    /** @brief Pulse IN is disabled
     */
    CPSS_DXCH_PTP_TAI_PULSE_IN_DISABLED_E,
} CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT;

/**
* @enum CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_ENT
 *
 * @brief TAI interface type.
 * (APPLICABLE DEVICES: Ironman)
*/
typedef enum{

    /** interface clock */
    CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_CLOCK_E,

    /** interface pulse */
    CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_PULSE_E,

    /** interface Pclk */
    CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_PCLK_E,

} CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_ENT;

/**
* @enum CPSS_DXCH_PTP_TAI_PIN_SELECT_ENT
 *
 * @brief TAI pin selection.
 * (APPLICABLE DEVICES: Ironman)
*/
typedef enum{

    /** pin A */
    CPSS_DXCH_PTP_TAI_PIN_SELECT_A_E,

    /** pin B */
    CPSS_DXCH_PTP_TAI_PIN_SELECT_B_E,

    /** pin C */
    CPSS_DXCH_PTP_TAI_PIN_SELECT_C_E,

    /** no pin is selected */
    CPSS_DXCH_PTP_TAI_PIN_SELECT_NONE_E

} CPSS_DXCH_PTP_TAI_PIN_SELECT_ENT;


/**
* @enum CPSS_DXCH_PTP_TAI_NUMBER_ENT
 *
 * @brief Number of TAI in Instance.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef enum{

    /** TAI0 */
    CPSS_DXCH_PTP_TAI_NUMBER_0_E,

    /** TAI1 */
    CPSS_DXCH_PTP_TAI_NUMBER_1_E,

    /** TAI2 */
    /** (APPLICABLE DEVICES: Ironman) */
    CPSS_DXCH_PTP_TAI_NUMBER_2_E,

    /** TAI3 */
    /** (APPLICABLE DEVICES: Ironman) */
    CPSS_DXCH_PTP_TAI_NUMBER_3_E,

    /** TAI4 */
    /** (APPLICABLE DEVICES: Ironman) */
    CPSS_DXCH_PTP_TAI_NUMBER_4_E,

    /** All TAIs */
    CPSS_DXCH_PTP_TAI_NUMBER_ALL_E

} CPSS_DXCH_PTP_TAI_NUMBER_ENT;

/**
* @struct CPSS_DXCH_PTP_TAI_ID_STC
 *
 * @brief Structure for TAI Identification.
 * (APPLICABLE DEVICES: Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef struct{

    /** @brief TAI Instance
     * (APPLICABLE DEVICES: Bobcat2)
     */
    CPSS_DXCH_PTP_TAI_INSTANCE_ENT taiInstance;

    /** @brief TAI Number
     */
    CPSS_DXCH_PTP_TAI_NUMBER_ENT taiNumber;

    /** @brief Physical Port Number
     * (APPLICABLE DEVICES: Bobcat2)
     */
    GT_PHYSICAL_PORT_NUM portNum;

} CPSS_DXCH_PTP_TAI_ID_STC;

/**
* @enum CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT
 *
 * @brief This enum defines possible TOD (Time Of Day) counter functions.
*/
typedef enum{

    /** @brief Copy the value from the TOD
     *  counter shadow to the TOD counter register
     */
    CPSS_DXCH_PTP_TOD_COUNTER_FUNC_UPDATE_E,

    /** @brief Add the value of the TOD
     *  counter shadow to the TOD counter register
     */
    CPSS_DXCH_PTP_TOD_COUNTER_FUNC_INCREMENT_E,

    /** @brief Copy the value of the TOD
     *  counter to the TOD counter shadow register
     */
    CPSS_DXCH_PTP_TOD_COUNTER_FUNC_CAPTURE_E,

    /** @brief Generate a pulse on the
     *  external interface at the configured time,
     *  determined by the TOD counter shadow
     *  (APPLICABLE DEVICES: Lion2.)
     */
    CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GENERATE_E,

    /** @brief update the value of the TOD Counter
     *  by subtracting a preconfigured offset.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PTP_TOD_COUNTER_FUNC_DECREMENT_E,

    /** @brief as INCREMENT, but the TOD Counter
     *  modified by a preconfigured offset over a given period of time.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_INCREMENT_E,

    /** @brief as DECREMENT, but the TOD Counter
     *  modified by a preconfigured offset over a given period of time.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_DECREMENT_E,

    /** @brief Copy the value from the TOD
     *  counter shadow to the fractional nanosecond drift register
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PTP_TOD_COUNTER_FUNC_FREQUENCE_UPDATE_E,

    /** @brief No Operation.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PTP_TOD_COUNTER_FUNC_NOP_E

} CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT;

/**
* @struct CPSS_DXCH_PTP_TOD_COUNT_STC
 *
 * @brief Structure for TOD Counter.
*/
typedef struct{

    /** nanosecond value of the time stamping TOD counter */
    GT_U32 nanoSeconds;

    /** @brief second value of the time stamping TOD counter
     *  48 LSBits are significant.
     */
    GT_U64 seconds;

    /** @brief fractional nanosecond part of the value.
     *  The fractional nanosecond field is a signed 32-bit two's complement number.
     *  It ranges from -2^31 to (2^31-1) nanosecond fractions, representing the range
     *  -0.5 ns to 0.5 ns.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
     *  It ranges from -2^30 to (2^30) nanosecond fractions,representing the range
     *  -0.25 ns to 0.25 ns.
     *  (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 fracNanoSeconds;

} CPSS_DXCH_PTP_TOD_COUNT_STC;

/**
* @struct CPSS_DXCH_PTP_TIMESTAMP_ENTRY_STC
 *
 * @brief Structure for Captured Timestamp Entry.
*/
typedef struct{

    /** @brief PTP source/target port number
     *  Note: In ingress time stamping this field specifies the
     *  source port of the incoming PTP packet.
     *  In egress time stamping this field specifies the
     *  target port of the outgoing PTP packet.
     */
    GT_U8 portNum;

    /** PTP message type from PTP header. */
    GT_U32 messageType;

    /** The PTP Sequence ID of the PTP packet. */
    GT_U32 sequenceId;

    /** @brief The PTP timestamp value, contains the nanosecond field of
     *  the TOD counter.
     *  Note: Bits 31:30 are always 0.
     */
    GT_U32 timeStampVal;

} CPSS_DXCH_PTP_TIMESTAMP_ENTRY_STC;


/*============== Bobcat2, Caelum, Bobcat3 Data Types =============*/

/**
* @enum CPSS_DXCH_PTP_TAI_TOD_TYPE_ENT
 *
 * @brief This enum defines type of value of TOD (Time Of Day) counter.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef enum{

    /** @brief The value of the TOD
     *  when the trigger generated for external device.
     */
    CPSS_DXCH_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E,

    /** Generate function Mask. */
    CPSS_DXCH_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_MASK_E,

    /** @brief The value to update the TOD
     *  by loading it instead the current TOD
     *  or adding it to the current TOD
     *  or subtracting it from the current TOD.
     */
    CPSS_DXCH_PTP_TAI_TOD_TYPE_LOAD_VALUE_E,

    /** @brief Copy of the value of the TOD
     *  made by Capture command.
     */
    CPSS_DXCH_PTP_TAI_TOD_TYPE_CAPTURE_VALUE0_E,

    /** @brief Copy of the value of the TOD
     *  made by Capture command.
     */
    CPSS_DXCH_PTP_TAI_TOD_TYPE_CAPTURE_VALUE1_E

} CPSS_DXCH_PTP_TAI_TOD_TYPE_ENT;

/**
* @enum CPSS_DXCH_PTP_TAI_CLOCK_MODE_ENT
 *
 * @brief This enum defines the clock mode.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2)
*/
typedef enum{

    /** Clock connection disabled */
    CPSS_DXCH_PTP_TAI_CLOCK_MODE_DISABLED_E,

    /** @brief Generate external clock
     *  relevant to Global TAI Instance only.
     */
    CPSS_DXCH_PTP_TAI_CLOCK_MODE_OUTPUT_E,

    /** Receive clock and update TOD */
    CPSS_DXCH_PTP_TAI_CLOCK_MODE_INPUT_E,

} CPSS_DXCH_PTP_TAI_CLOCK_MODE_ENT;

/**
* @struct CPSS_DXCH_PTP_TAI_TOD_STEP_STC
 *
 * @brief Structure for value of TOD Step.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef struct{

    /** @brief whole nanosecond part of the value
     *  (APPLICABLE RANGES: Bobcat2 0..0xFFFFFFFF;
     *  Caelum, Bobcat3; Aldrin2, Aldrin, AC3X 0..0xFFFF)
     */
    GT_U32 nanoSeconds;

    /** @brief fractional nanosecond part of the value.
     *  Value measured in (1/(2^32)) units.
     */
    GT_U32 fracNanoSeconds;

} CPSS_DXCH_PTP_TAI_TOD_STEP_STC;

/**
* @struct CPSS_DXCH_PTP_TSU_CONTROL_STC
 *
 * @brief Structure for TSU (Time Stamping Unit) Control.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef struct{

    /** @brief GT_TRUE
     *  GT_FALSE - the unit disabled.
     */
    GT_BOOL unitEnable;

    /** @brief select TAI for ingress time stamping.
     *  (APPLICABLE RANGES: 0..1.)
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier)
     */
    CPSS_DXCH_PTP_TAI_NUMBER_ENT taiNumber;

} CPSS_DXCH_PTP_TSU_CONTROL_STC;

/**
* @struct CPSS_DXCH_PTP_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC
 *
 * @brief Structure for TSU (Time Stamping Unit) TX Timestamp Queue Entry.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef struct{

    /** @brief GT_TRUE
     *  GT_FALSE - Queue is empty.
     */
    GT_BOOL entryValid;

    /** @brief This field is used to match between the egress timestamp
     *  queue entry and the per-port timestamp entry.
     *  The same <Queue Entry ID> is also stored in the
     *  per-port timestamp queue.
     */
    GT_U32 queueEntryId;

    /** selected TAI. (APPLICABLE RANGES: 0..1. Ironman: 0..4) */
    GT_U32 taiSelect;

    /** @brief The least significant bit of the <Time Update Counter>
     *  in the TAI, at the time that <Timestamp> is captured.
     */
    GT_U32 todUpdateFlag;

     /** @brief The value of the TOD counter at the times of transmission
     *  or reception of the packet. This is a
     *  32 bit nanosecond timestamp in the range -2^30 to 2^30.
     */
    GT_U32 timestamp;

} CPSS_DXCH_PTP_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC;

/**
* @enum CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_ENT
 *
 * @brief This enum defines the packet counter type.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef enum{

    /** @brief Counts the total number
     *  of packets that are subject to any kind of time stamping action in
     *  the egress time stamping unit (TSU).
     */
    CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_TOTAL_E,

    /** Counts PTP V1 packets */
    CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_PTP_V1_E,

    /** Counts PTP V2 packets */
    CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_PTP_V2_E,

    /** Counts Y1731 packets */
    CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_Y1731_E,

    /** Counts NTP Timestamp packets */
    CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_NTP_TS_E,

    /** Counts NTP received packets */
    CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_NTP_RX_E,

    /** Counts NTP transmitted packets */
    CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_NTP_TX_E,

    /** Counts WAMP packets */
    CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_WAMP_E,

    /** Counts None Action packets */
    CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_NONE_ACTION_E,

    /** Counts Forwarded packets */
    CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_FORWARD_E,

    /** Counts Dropped packets */
    CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_DROP_E,

    /** Counts Captured packets */
    CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_CAPTURE_E,

    /** Counts Add Time Action packets */
    CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_ADD_TIME_E,

    /** @brief Counts Add
     *  Corrected Time Action packets
     */
    CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_ADD_CORR_TIME_E,

    /** @brief Counts Captured
     *  Add Time Action packets
     */
    CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_CAPTURE_ADD_TIME_E,

    /** @brief Counts Captured
     *  Add Corrected Time Action packets
     */
    CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_CAPTURE_ADD_CORR_TIME_E,

    /** @brief Counts
     *  Add Ingress Time Action packets
     */
    CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_ADD_INGRESS_TIME_E,

    /** @brief Counts
     *  Captured Add Ingress Time Action packets
     */
    CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_CAPTURE_ADD_INGRESS_TIME_E,

    /** @brief Counts
     *  Captured Ingress Time Action packets
     */
    CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_CAPTURE_INGRESS_TIME_E,

    /** @brief Counts
     *  Captured packets by index (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PTP_TSU_PACKET_COUNTER_INDEX_E

} CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_ENT;

/**
* @struct CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC
 *
 * @brief Structure Timestamp Tag Global Configuration.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef struct{

    /** @brief GT_TRUE
     *  as timestamp tagged with Extended or Non-Extended Tag.
     *  GT_FALSE - no packets are identified
     *  as timestamp tagged with Extended or Non-Extended Tag.
     */
    GT_BOOL tsTagParseEnable;

    /** @brief GT_TRUE
     *  are identified as timestamp tagged with Hybrid Tag.
     *  GT_FALSE - no packets
     *  are identified as timestamp tagged with Hybrid Tag.
     */
    GT_BOOL hybridTsTagParseEnable;

    /** @brief Ethertype of Extended and Non Extended TS Tags.
     *  (APPLICABLE RANGES: 0..0xFFFF.)
     */
    GT_U32 tsTagEtherType;

    /** @brief Ethertype of Hybrid TS Tags.
     *  (APPLICABLE RANGES: 0..0xFFFF.)
     */
    GT_U32 hybridTsTagEtherType;

} CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC;

/**
* @enum CPSS_DXCH_PTP_TS_TAG_MODE_ENT
 *
 * @brief This enum defines the Timestamp Tag mode.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef enum{

    /** Timestamps not added to any packets. */
    CPSS_DXCH_PTP_TS_TAG_MODE_NONE_E,

    /** @brief Timestamps (TS Tags) added
     *  to all packets in nonextended format.
     */
    CPSS_DXCH_PTP_TS_TAG_MODE_ALL_NON_EXTENDED_E,

    /** @brief Timestamps (TS Tags) added
     *  to all packets in extended format.
     */
    CPSS_DXCH_PTP_TS_TAG_MODE_ALL_EXTENDED_E,

    /** @brief Timestamps added
     *  only to time protocol packets (PTP, DM) as nonextended TS Tags.
     */
    CPSS_DXCH_PTP_TS_TAG_MODE_NON_EXTENDED_E,

    /** @brief Timestamps added
     *  only to time protocol packets (PTP, DM) as extended TS Tags.
     */
    CPSS_DXCH_PTP_TS_TAG_MODE_EXTENDED_E,

    /** @brief Timestamps added
     *  only to PTP packets, using the reserved field in the PTP header.
     *  Valid only for PTP header of version v2.
     */
    CPSS_DXCH_PTP_TS_TAG_MODE_PIGGYBACK_E,

    /** @brief Timestamps added
     *  only to PTP packets, using the hybrid TS tag.
     *  Valid only for PTP header of version v2.
     */
    CPSS_DXCH_PTP_TS_TAG_MODE_HYBRID_E

} CPSS_DXCH_PTP_TS_TAG_MODE_ENT;

/**
* @struct CPSS_DXCH_PTP_TS_TAG_PORT_CFG_STC
 *
 * @brief Structure Timestamp Port Configuration.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef struct{

    /** @brief Time for ingress timestamp based on :
     *  GT_TRUE - the timestamp tag.
     *  GT_FALSE - the reception time in the current device.
     */
    GT_BOOL tsReceptionEnable;

    /** @brief ingress PTP V2 packets header reserved field Timestamp use:
     *  GT_TRUE - contains Piggy Back Timestamp.
     *  GT_FALSE - contains data not supported by the unit.
     */
    GT_BOOL tsPiggyBackEnable;

    /** Mode of time stamping the egress packets. */
    CPSS_DXCH_PTP_TS_TAG_MODE_ENT tsTagMode;

} CPSS_DXCH_PTP_TS_TAG_PORT_CFG_STC;

/**
* @enum CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT
 *
 * @brief This enum defines the TAI selection mode.
 * Used for selection TAI0 or TAI1 for PTP Header Correction Field.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier)
*/
typedef enum{

    /** @brief use the same TAI as for ingress
     *  Used when the ingress time received by TAI (of current or remote device).
     */
    CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_INGRESS_E,

    /** @brief TAI selected from
     *  pertargetport configuration.
     *  Used when the ingress time received by Timestamp TAG from PHY.
     */
    CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E

} CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT;

/**
* @enum CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_ENT
 *
 * @brief This enum defines Timestamping Mode.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef enum{

    /** Do Action */
    CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_DO_ACTION_E,

    /** Modify Timestamp Tag */
    CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_MODIFY_TAG_E

} CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_ENT;

/**
* @enum CPSS_DXCH_PTP_TS_ACTION_ENT
 *
 * @brief This enum defines the PTP Timestamp Action.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef enum{

    /** no operation (forward) */
    CPSS_DXCH_PTP_TS_ACTION_NONE_E,

    /** drop */
    CPSS_DXCH_PTP_TS_ACTION_DROP_E,

    /** capture */
    CPSS_DXCH_PTP_TS_ACTION_CAPTURE_E,

    /** add time */
    CPSS_DXCH_PTP_TS_ACTION_ADD_TIME_E,

    /** add corrected time */
    CPSS_DXCH_PTP_TS_ACTION_ADD_CORRECTED_TIME_E,

    /** capture and add time */
    CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_TIME_E,

    /** capture and add corrected time */
    CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_CORRECTED_TIME_E,

    /** add ingress time */
    CPSS_DXCH_PTP_TS_ACTION_ADD_INGRESS_TIME_E,

    /** capture ingress time */
    CPSS_DXCH_PTP_TS_ACTION_CAPTURE_INGRESS_TIME_E,

    /** capture and add ingress time */
    CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_INGRESS_TIME_E,

    /** @brief Add ingress & egress time
     *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PTP_TS_ACTION_ADD_INGRESS_EGRESS_TIME_E,

    /** @brief Action Type ALL, This Value is used for TSU counter configuration only.
     *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PTP_TS_ACTION_ALL_E
} CPSS_DXCH_PTP_TS_ACTION_ENT;

/**
* @enum CPSS_DXCH_PTP_TS_PACKET_TYPE_ENT
 *
 * @brief This enum defines the packet type.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef enum{

    /** PTP V1 packets */
    CPSS_DXCH_PTP_TS_PACKET_TYPE_PTP_V1_E,

    /** PTP V2 packets */
    CPSS_DXCH_PTP_TS_PACKET_TYPE_PTP_V2_E,

    /** Y1731 packets */
    CPSS_DXCH_PTP_TS_PACKET_TYPE_Y1731_E,

    /** NTP packets timestaming */
    CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_TS_E,

    /** NTP received packets */
    CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_RX_E,

    /** NTP transmitted packets */
    CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_TX_E,

    /** WAMP packets */
    CPSS_DXCH_PTP_TS_PACKET_TYPE_WAMP_E,

    /** @brief Packet Type ALL, This Value is used for TSU counter configuration only.
     * (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PTP_TS_PACKET_TYPE_ALL_E,

    /** reserved */
    CPSS_DXCH_PTP_TS_PACKET_TYPE_RESERVED_E

} CPSS_DXCH_PTP_TS_PACKET_TYPE_ENT;

/**
* @enum CPSS_DXCH_PTP_TRANSPORT_TYPE_ENT
 *
 * @brief This enum defines PTP Transport.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef enum{

    /** Ethernet */
    CPSS_DXCH_PTP_TRANSPORT_TYPE_ETHERNET_E,

    /** UDP over IPV4 */
    CPSS_DXCH_PTP_TRANSPORT_TYPE_UDP_IPV4_E,

    /** UDP over IPV6 */
    CPSS_DXCH_PTP_TRANSPORT_TYPE_UDP_IPV6_E

} CPSS_DXCH_PTP_TRANSPORT_TYPE_ENT;

/**
* @struct CPSS_DXCH_PTP_TS_CFG_ENTRY_STC
 *
 * @brief Structure Timestamp Configuration Table Entry.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef struct{

    /** Timestamping mode. */
    CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_ENT tsMode;

    /** @brief Offset Profile.
     *  relevant if tsMode==CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_MODIFY_TAG_E
     *  field to upate in Extended and Non-Extended TS Tag
     *  (APPLICABLE RANGES: 0..127.)
     */
    GT_U32 offsetProfile;

    /** @brief Offset Enable.
     *  relevant if tsMode==CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_MODIFY_TAG_E
     *  field to upate in Extended and Non-Extended TS Tag
     *  GT_TRUE - enable, GT_FALSE - disable.
     */
    GT_BOOL OE;

    /** @brief Timestamping Action.
     *  relevant if tsMode==CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_DO_ACTION_E
     */
    CPSS_DXCH_PTP_TS_ACTION_ENT tsAction;

    /** @brief Packet Format.
     *  relevant if tsMode==CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_DO_ACTION_E
     *  or packets egressed without Extended and Non-Extended TS Tag
     */
    CPSS_DXCH_PTP_TS_PACKET_TYPE_ENT packetFormat;

    /** @brief PTP Transport.
     *  relevant if tsMode==CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_DO_ACTION_E
     *  or packets egressed without Extended and Non-Extended TS Tag
     */
    CPSS_DXCH_PTP_TRANSPORT_TYPE_ENT ptpTransport;

    /** @brief The offset relative to the beginning of the L3 header.
     *  The offset is measured in bytes.
     *  relevant if tsMode==CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_DO_ACTION_E
     *  or packets egressed without Extended and Non-Extended TS Tag
     *  (APPLICABLE RANGES: 0..255.)
     */
    GT_U32 offset;

    /** @brief The offset relative to the beginning of the L3 header.
     * (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
     *  The offset is measured in bytes.
     *  relevant if tsMode==CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_DO_ACTION_E
     *  or packets egressed without Extended and Non-Extended TS Tag
     *  offset2 holds the ingress time pointer.
     *  Used for CPSS_DXCH_PTP_TS_ACTION_ADD_INGRESS_EGRESS_TIME_E only.
     *  Odd values are not applicable.
     *  (APPLICABLE RANGES: 0..255.)
     */
    GT_U32 offset2;

    /** @brief The PTP message type. Affects the <Ingress Link Delay En>,
     *  and the <Message Type> in the timestamp queues
     *  relevant if tsMode==CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_DO_ACTION_E
     *  and packetFormat is
     *  CPSS_DXCH_PTP_TS_PACKET_TYPE_PTP_V1_E or
     *  CPSS_DXCH_PTP_TS_PACKET_TYPE_PTP_V2_E.
     *  (APPLICABLE RANGES: 0..15.)
     */
    GT_U32 ptpMessageType;

    /** @brief PTP message domain index.
     *  relevant if tsMode==CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_DO_ACTION_E
     *  and packetFormat is
     *  CPSS_DXCH_PTP_TS_PACKET_TYPE_PTP_V1_E or
     *  CPSS_DXCH_PTP_TS_PACKET_TYPE_PTP_V2_E.
     *  (APPLICABLE RANGES: 0..4.)
     */
    GT_U32 domain;

    /** @brief Ingress Link Delay Enable.
     *  Ingress link delay is considered in the ingress timestamp computation.
     *  GT_TRUE - enable, GT_FALSE - disable.
     *  This attribute is implicitly enabled for non-PTP packets.
     */
    GT_BOOL ingrLinkDelayEnable;

    /** @brief Packet Dispatching Enable.
     *  Typically enabled for 1-step timestamps.
     *  When enabled, the packet is scheduled to be transmitted at
     *  the precise time that was embedded in the packet.
     *  GT_TRUE - enable, GT_FALSE - disable.
     */
    GT_BOOL packetDispatchingEnable;

} CPSS_DXCH_PTP_TS_CFG_ENTRY_STC;

/**
* @struct CPSS_DXCH_PTP_TS_LOCAL_ACTION_ENTRY_STC
 *
 * @brief Structure Timestamp Local Action Table Entry.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef struct{

    /** @brief Timestamping Action.
     *  Supported values are:
     *  CPSS_DXCH_PTP_TS_ACTION_NONE_E
     *  CPSS_DXCH_PTP_TS_ACTION_DROP_E
     *  CPSS_DXCH_PTP_TS_ACTION_CAPTURE_E
     *  CPSS_DXCH_PTP_TS_ACTION_ADD_TIME_E
     *  CPSS_DXCH_PTP_TS_ACTION_ADD_CORRECTED_TIME_E
     *  CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_TIME_E
     *  CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_CORRECTED_TIME_E
     */
    CPSS_DXCH_PTP_TS_ACTION_ENT tsAction;

    /** @brief Ingress Link Delay Enable.
     *  Ingress link delay is considered in the ingress timestamp computation.
     *  GT_TRUE - enable, GT_FALSE - disable.
     *  This attribute is implicitly enabled for non-PTP packets.
     */
    GT_BOOL ingrLinkDelayEnable;

    /** @brief Packet Dispatching Enable.
     *  Typically enabled for 1-step timestamps.
     *  When enabled, the packet is scheduled to be transmitted at
     *  the precise time that was embedded in the packet.
     *  GT_TRUE - enable, GT_FALSE - disable.
     */
    GT_BOOL packetDispatchingEnable;

} CPSS_DXCH_PTP_TS_LOCAL_ACTION_ENTRY_STC;

/**
* @enum CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT
 *
 * @brief This enum defines UDP checksum update modes.
 * Related to updated timestamps inside UDP payload.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef enum{

    /** clear UDP Checksum */
    CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_CLEAR_E,

    /** recalculate UDP Checksum */
    CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_RECALCULATE_E,

    /** keep original UDP Checksum */
    CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_KEEP_E

} CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT;

/**
* @struct CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC
 *
 * @brief Structure Timestamp UDP Checksum Update Modes.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef struct{

    /** UDP Checksum Update Mode for PTP over IPV4. */
    CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT ptpIpv4Mode;

    /** UDP Checksum Update Mode for PTP over IPV6. */
    CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT ptpIpv6Mode;

    /** UDP Checksum Update Mode for NTP over IPV4. */
    CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT ntpIpv4Mode;

    /** UDP Checksum Update Mode for NTP over IPV6. */
    CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT ntpIpv6Mode;

    /** UDP Checksum Update Mode for WAMP over IPV4. */
    CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT wampIpv4Mode;

    /** UDP Checksum Update Mode for WAMP over IPV6. */
    CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT wampIpv6Mode;

} CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC;

/**
* @enum CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_ENT
 *
 * @brief This enum defines PTP domain mode.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef enum{

    /** Disable */
    CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_DISABLE_E,

    /** PTP version 1 */
    CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_PTP_V1_E,

    /** PTP version 2 */
    CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_PTP_V2_E

} CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_ENT;


/**
* @enum CPSS_DXCH_PTP_TAI_CLOCK_FREQ_ENT
 *
 * @brief This enum defines PTP clock frequency in Mhz.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef enum{

    /** APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman */
    CPSS_DXCH_PTP_10_FREQ_E,

    /** APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman */
    CPSS_DXCH_PTP_20_FREQ_E,

    CPSS_DXCH_PTP_25_FREQ_E,

    /** APPLICABLE DEVICES: Harrier; Ironman */
    CPSS_DXCH_PTP_156_25_FREQ_E

}CPSS_DXCH_PTP_TAI_CLOCK_FREQ_ENT;

/**
* @struct CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC
 *
 * @brief Structure for Egress Domain Table Entry.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef struct{

    /** @brief PTP Over Ehernet Timestamping Enable.
     *  GT_TRUE - enable, GT_FALSE - disable.
     */
    GT_BOOL ptpOverEhernetTsEnable;

    /** @brief PTP Over UDP IPV4 Timestamping Enable.
     *  GT_TRUE - enable, GT_FALSE - disable.
     */
    GT_BOOL ptpOverUdpIpv4TsEnable;

    /** @brief PTP Over UDP IPV6 Timestamping Enable.
     *  GT_TRUE - enable, GT_FALSE - disable.
     */
    GT_BOOL ptpOverUdpIpv6TsEnable;

    /** @brief PTP message Timestamping Enable.
     *  Bitmap of 16 bits indexed by PTP Message Type.
     *  1 - enable, 0 - disable.
     *  (APPLICABLE RANGES: 0..0xFFFF.)
     */
    GT_U32 messageTypeTsEnableBmp;

    /** @brief PTP message Timestamping Enable.
     *  Bitmap of 16 bits indexed by 4-bit Transport Specific value.
     *  1 - enable, 0 - disable.
     *  (APPLICABLE RANGES: 0..0xFFFF.)
     */
    GT_U32 transportSpecificTsEnableBmp;

} CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC;

/**
* @enum CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_ENT
 *
 * @brief This enum defines ingress PTP packet checking mode.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef enum{

    /** @brief The Basic Mode, checks that:
     *  The <versionPTP> field is either 1 or 2.
     *  The packet is long enough to include the version,
     *  message type, and domain fields in the PTP header.
     */
    CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_BASIC_E,

    /** @brief The Transparent Clock Mode
     *  Additional to the Basic Mode checks it checks that,
     *  the Correction Field
     *  is within the packet length, and within the first 128B.
     */
    CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_TRANS_CLK_E,

    /** @brief The Piggy Back Mode
     *  Additional to the Basic Mode checks it checks that,
     *  the 32-bit Reserved Field
     *  is within the packet length, and within the first 128B.
     */
    CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_PIGGY_BACK_E,

    /** @brief The Boundary Clock Mode
     *  Additional to the Basic Mode checks it checks that,
     *  the Origin Timestamp Field
     *  is within the packet length, and within the first 128B.
     */
    CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_BOUNDRY_CLK_E

} CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_ENT;

/**
* @struct CPSS_DXCH_PTP_INGRESS_EXCEPTION_CFG_STC
 *
 * @brief Structure for Ingress Exception Configuration.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef struct{

    /** @brief enable PTP command assignment when an exception occurs.
     *  GT_TRUE - enable, GT_FALSE - disable.
     */
    GT_BOOL ptpExceptionCommandEnable;

    /** @brief packet command in case of a PTP exception.
     *  valid values
     *  CPSS_PACKET_CMD_FORWARD_E
     *  CPSS_PACKET_CMD_MIRROR_TO_CPU_E
     *  CPSS_PACKET_CMD_TRAP_TO_CPU_E
     *  CPSS_PACKET_CMD_DROP_HARD_E
     *  CPSS_PACKET_CMD_DROP_SOFT_E
     */
    CPSS_PACKET_CMD_ENT ptpExceptionCommand;

    /** @brief the CPU/Drop Code when the PTP exception packet command
     *  is not FORWARD.
     */
    CPSS_NET_RX_CPU_CODE_ENT ptpExceptionCpuCode;

    /** @brief enable PTP version checking.
     *  GT_TRUE - enable, GT_FALSE - disable.
     */
    GT_BOOL ptpVersionCheckEnable;

} CPSS_DXCH_PTP_INGRESS_EXCEPTION_CFG_STC;

/**
* @struct CPSS_DXCH_PTP_EGRESS_EXCEPTION_CFG_STC
 *
 * @brief Structure for Egress Exception Configuration.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef struct{

    /** @brief invalid PTP Packet Command.
     *  valid values for Falcon FORWARD/MIRROR/TRAP/HARD_DROP/SOFT_DROP
     *  valid values for other devices DROP and FORWARD only.
     */
    CPSS_PACKET_CMD_ENT invalidPtpPktCmd;

    /** @brief the CPU/Drop Code when the PTP exception packet command
     *  is not FORWARD.
     *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_NET_RX_CPU_CODE_ENT invalidPtpCpuCode;

    /** @brief invalid Outgoing Piggyback Packet Command.
     *  valid values for Falcon FORWARD/MIRROR/TRAP/HARD_DROP/SOFT_DROP
     *  valid values for other devices DROP and FORWARD only.
     */
    CPSS_PACKET_CMD_ENT invalidOutPiggybackPktCmd;

    /** @brief the CPU/Drop Code when the invalid Outgoing Piggyback Packet Command
      * is not FORWARD.
      * (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
      */
    CPSS_NET_RX_CPU_CODE_ENT invalidOutPiggybackCpuCode;

    /** @brief invalid Ingoing Piggyback Packet Command.
     *  valid values for Falcon FORWARD/MIRROR/TRAP/HARD_DROP/SOFT_DROP
     *  valid values for other devices DROP and FORWARD only.
     */
    CPSS_PACKET_CMD_ENT invalidInPiggybackPktCmd;

    /** @brief the CPU/Drop Code when the invalid Ingoing Piggyback Packet Command
      * is not FORWARD.
      * (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
      */
    CPSS_NET_RX_CPU_CODE_ENT invalidInPiggybackCpuCode;

    /** @brief invalid Timestamp Packet Command.
     *  valid values for Falcon FORWARD/MIRROR/TRAP/HARD_DROP/SOFT_DROP
     *  valid values for other devices DROP and FORWARD only.
     */
    CPSS_PACKET_CMD_ENT invalidTsPktCmd;

    /** @brief the CPU/Drop Code when the invalid Timestamp Packet Command
      * is not FORWARD.
      * (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
      */
    CPSS_NET_RX_CPU_CODE_ENT invalidTsCpuCode;

} CPSS_DXCH_PTP_EGRESS_EXCEPTION_CFG_STC;

/**
* @struct CPSS_DXCH_PTP_EGRESS_EXCEPTION_COUNTERS_STC
 *
 * @brief Structure for Egress Exception Counters.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef struct{

    /** invalid PTP Packet Counter. */
    GT_U32 invalidPtpPktCnt;

    /** invalid Outgoing Piggyback Packet Counter. */
    GT_U32 invalidOutPiggybackPktCnt;

    /** invalid Ingoing Piggyback Packet Counter. */
    GT_U32 invalidInPiggybackPktCnt;

    /** invalid Timestamp Packet Counter. */
    GT_U32 invalidTsPktCnt;

} CPSS_DXCH_PTP_EGRESS_EXCEPTION_COUNTERS_STC;

/**
* @struct CPSS_DXCH_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC
 *
 * @brief Structure for Ingress Timestamp Queue Entry.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef struct{

    /** @brief GT_TRUE
     *  GT_FALSE - Queue is empty.
     */
    GT_BOOL entryValid;

    /** @brief GT_TRUE
     *  GT_FALSE - no PTP exception was triggered
     */
    GT_BOOL isPtpExeption;

    /** Packet Format */
    CPSS_DXCH_PTP_TS_PACKET_TYPE_ENT packetFormat;

    /** selected TAI. (APPLICABLE RANGES: 0..1. Ironman: 0..4) */
    GT_U32 taiSelect;

    /** @brief The least significant bit of the Time Update Counter
     *  in the TAI, at the time that Timestamp is captured
     */
    GT_U32 todUpdateFlag;

    /** @brief Message Type from PTP header.
     *  0 for not PTP packets.
     */
    GT_U32 messageType;

    /** @brief Domain Number from PTP header.
     *  0 for not PTP packets.
     */
    GT_U32 domainNum;

    /** @brief The sequenceID field from the PTP packet header.
     *  For non-PTP packets it is packet's Flow-ID.
     */
    GT_U32 sequenceId;

    /** @brief The value of the TOD counter at the times of transmission
     *  or reception of the packet. This is a
     *  32 bit nanosecond timestamp in the range -2^30 to 2^30.
     */
    GT_32 timestamp;

    /** The local */
    GT_PHYSICAL_PORT_NUM portNum;

} CPSS_DXCH_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC;

/**
* @struct CPSS_DXCH_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC
 *
 * @brief Structure for Egress Timestamp Queue Entry.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef struct{

    /** @brief GT_TRUE
     *  GT_FALSE - Queue is empty.
     */
    GT_BOOL entryValid;

    /** @brief GT_TRUE
     *  GT_FALSE - no PTP exception was triggered
     */
    GT_BOOL isPtpExeption;

    /** Packet Format */
    CPSS_DXCH_PTP_TS_PACKET_TYPE_ENT packetFormat;

    /** @brief Message Type from PTP header.
     *  0 for not PTP packets.
     */
    GT_U32 messageType;

    /** @brief Domain Number from PTP header.
     *  0 for not PTP packets.
     */
    GT_U32 domainNum;

    /** @brief The sequenceID field from the PTP packet header.
     *  For non-PTP packets it is packet's Flow-ID.
     */
    GT_U32 sequenceId;

    /** @brief This field is used to match between the egress timestamp
     *  queue entry and the per-port timestamp entry.
     *  The same <Queue Entry ID> is also stored
     *  in the per-port timestamp queue.
     *  See cpssDxChPtpTsuTxTimestampQueueRead.
     */
    GT_U32 queueEntryId;

    /** The local */
    GT_PHYSICAL_PORT_NUM portNum;

} CPSS_DXCH_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC;

/**
* @enum CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_ENT
 *
 * @brief This enum defines mode for output signal to be either PTP PClk or Sync-E Recovered Clock.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef enum{

    /** @brief PTP clock output mode.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_PCLK_E,

    /** @brief Sync-E recovered clock output mode.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X.)
     */
    CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_RCVR_CLK_E

} CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_ENT;

/**
* @enum CPSS_DXCH_PTP_TAI_CLOCK_SELECT_ENT
 *
 * @brief This enum defines TAI input clock selection mode.
 * (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef enum{

    /** TAI PTP PLL clock selection mode. */
    CPSS_DXCH_PTP_TAI_CLOCK_SELECT_PTP_PLL_E,

    /** TAI Core PLL clock selection mode. */
    CPSS_DXCH_PTP_TAI_CLOCK_SELECT_CORE_PLL_E

} CPSS_DXCH_PTP_TAI_CLOCK_SELECT_ENT;

/**
* @enum CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_ENT
 *
 * @brief This enum defines mode for output(ClockGen) from TAI.
 * External output interface used for generating a clock signal at a configurable clock rate(aka PPS signal).
 * (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef enum{

    /** @brief Unit does not generate an external clock signal.
     */
    CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_DISABLED_E,

    /** @brief Unit generates an external clock signal(aka PPS signal),
     *  driving a clock generation derivative of the internal TOD Counter.
     */
    CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_CLOCK_GEN_E,

    /** @brief Unit generates an external clock signal,
     *  driving a clock reception derivative of the internal TOD Counter. This is debug option only
     *  (APPLICABLE DEVICES: Falcon)
     */
    CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_CLOCK_RECEPTION_E

} CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_ENT;

/**
* @enum CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_ENT
 *
 * @brief This enum defines Clock reception mode.
 * it determines whether the TOD is updated according to the incoming CLK_IN or not.
 * (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef enum{

    /** @brief Unit does not receive an external clock signal.
     */
    CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_DISABLED_E,

    /** @brief Update phase of TOD counter according to an external clock input.
     */
    CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_CLOCK_RECEPTION_E

} CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_ENT;

/**
* @enum CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_ENT
 *
 * @brief This enum defines mode for output(PClkOut) from TAI.
 * External clock interface that can be connected to an external PLL
 * (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef enum{

    /** @brief Disabled.
     */
    CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_DISABLED_E,

    /** @brief P-Clock Counter derivative.
     */
    CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_PCLK_E,

    /** @brief TOD ClockGen derivative(aka PPS signal).
     */
    CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_CLOCK_GEN_E,

    /** @brief TOD CLockRec derivative. This is debug option only.
     *  (APPLICABLE DEVICES: Falcon)
     */
    CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_CLOCK_RECEPTION_E

} CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_ENT;

/**
* @struct CPSS_DXCH_PTP_TSU_FRAME_COUNTER_CFG_ENTRY_STC
 *
 * @brief Structure for TSU (Time Stamping Unit) TimeStamping frame counter configuration Entry.
 * (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef struct{

    /** @brief GT_TRUE  - Counter is enabled for counting
     *         GT_FALSE - Counter is disabled.
     */
    GT_BOOL enable;

    /** @brief This field describes the action field
     */
    CPSS_DXCH_PTP_TS_ACTION_ENT action;

    /** @brief This field describes the format field
     */
    CPSS_DXCH_PTP_TS_PACKET_TYPE_ENT format;


} CPSS_DXCH_PTP_TSU_FRAME_COUNTER_CFG_ENTRY_STC;

/**
* @struct CPSS_DXCH_PTP_TSU_TIMESTAMP_MASKING_PROFILE_STC
 *
 * @brief Structure for TSU (Time Stamping Unit) TimeStamp Masking Profile configuration Entry.
 * (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier)
*/
typedef struct{
    /** @brief The number of bits in the packet timestamp field.
     *  (APPLICABLE VALUES: 16, 32, 48, 64, 80.)
     */
    GT_U32 numOfBits;

    /** @brief The number of least significant bits that are used for the timestamp.
     * The remaining bits are unmodified.
     *  (APPLICABLE VALUES: Must be less than or equal to NumOfBits. and Must not be ZERO)
     */
    GT_U32 numOfLsbits;

    /** @brief The number of bits to shift right before inserting the timestamp into the packet.
     * Must be less than the NumOfBits.
     *  (APPLICABLE RANGE: Multiple of 8 - 0, 8, 16, 24, 32, 40, 48, 56, 64, 72.)
     */
    GT_U32 numOfShifts;
} CPSS_DXCH_PTP_TSU_TIMESTAMP_MASKING_PROFILE_STC;

/**
* @struct CPSS_DXCH_PTP_TS_EGRESS_PIPE_DELAY_VALUES_STC
*
* @brief Structure for PTP egress pipe delay values.
* (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*/
typedef struct{

    /** @brief Egress pipe delay of channel n encrypted time-stamping frames,
     *        represented as an unsigned 30 bit nanosecond value.
     *        (APPLICABLE RANGES: 0..10^9-1.)                                               .
     *        (APPLICABLE DEVICES: AC5P; Harrier; Ironman).
     */
    GT_32  egressPipeDelayEncrypted;

    /** @brief The latency from the FSU till the pin according to
     *        port mode, represented as an unsigned 10 bit nanosecond value.
     *        (APPLICABLE RANGES: 0..1023.)                                                                .
     *        (APPLICABLE DEVICES: Harrier).
     */
    GT_32  egressPipeDelayFsuOffset;

    /** @brief Egress pipe delay, represented as unsigned 30-bit
     *         nanosecond value.
     *        (APPLICABLE RANGES: 0..10^9-1.)
     */
    GT_32  egressPipeDelay;

} CPSS_DXCH_PTP_TS_EGRESS_PIPE_DELAY_VALUES_STC;

/**
* @enum CPSS_DXCH_PTP_REF_CLOCK_SELECT_ENT
*
* @brief This enum defines the reference clock source of PTP PLL.
* (APPLICABLE DEVICES: AC5P)
*/
typedef enum{

    /** Using the Main device clock of 25 MHz. */
    CPSS_DXCH_PTP_REF_CLOCK_SELECT_MAIN_25_MHZ_E,

    /** Using the dedicated PTP PLL. */
    CPSS_DXCH_PTP_REF_CLOCK_SELECT_DEDICATED_PTP_REF_CLK_E,

    /** Using the SerDes differential clock. */
    CPSS_DXCH_PTP_REF_CLOCK_SELECT_SD_DIFFRENTIAL_REF_CLK_E

} CPSS_DXCH_PTP_REF_CLOCK_SELECT_ENT;

/**
* @enum CPSS_DXCH_PTP_TAI_SELECT_UNIT_ENT
*
* @brief TAI Unit for timestamping.
* (APPLICABLE DEVICES: Ironman)
*/
typedef enum{

    CPSS_DXCH_PTP_TAI_SELECT_UNIT_CTSU_E,

    CPSS_DXCH_PTP_TAI_SELECT_UNIT_ERMRK_E

} CPSS_DXCH_PTP_TAI_SELECT_UNIT_ENT;


/**
* @internal cpssDxChPtpEtherTypeSet function
* @endinternal
*
* @brief   Configure ethertype0/ethertype1 of PTP over Ethernet packets.
*         A packet is identified as PTP over Ethernet if its EtherType matches
*         one of the configured values.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] etherType                - PTP EtherType index.
*                                      (APPLICABLE RANGES: 0..1.)
* @param[in] etherType                - PTP EtherType0 or EtherType1 according to the index.
*                                      (APPLICABLE RANGES: 0..0xFFFF.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or etherTypeIndex
* @retval GT_OUT_OF_RANGE          - on wrong etherType
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpEtherTypeSet
(
    IN GT_U8    devNum,
    IN GT_U32   etherTypeIndex,
    IN GT_U32   etherType
);


/**
* @internal cpssDxChPtpEtherTypeGet function
* @endinternal
*
* @brief   Get ethertypes of PTP over Ethernet packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] etherTypeIndex           - PTP EtherType index.
*                                      (APPLICABLE RANGES: 0..1.)
*
* @param[out] etherTypePtr             - (pointer to) PTP EtherType0 or EtherType1 according
*                                      to the index
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or etherTypeIndex
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpEtherTypeGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   etherTypeIndex,
    OUT GT_U32   *etherTypePtr
);

/**
* @internal cpssDxChPtpUdpDestPortsSet function
* @endinternal
*
* @brief   Configure UDP destination port0/port1 of PTP over UDP packets.
*         A packet is identified as PTP over UDP if it is a UDP packet, whose
*         destination port matches one of the configured ports.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] udpPortIndex             - UDP port index.
*                                      (APPLICABLE RANGES: 0..1.)
* @param[in] udpPortNum               - UDP port1/port0 number according to the index,
*                                      (APPLICABLE RANGES: 0..0xFFFF.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or udpPortIndex
* @retval GT_OUT_OF_RANGE          - on wrong udpPortNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpUdpDestPortsSet
(
    IN GT_U8    devNum,
    IN GT_U32   udpPortIndex,
    IN GT_U32   udpPortNum
);

/**
* @internal cpssDxChPtpUdpDestPortsGet function
* @endinternal
*
* @brief   Get UDP destination port0/port1 of PTP over UDP packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] udpPortIndex             - UDP port index
*                                      (APPLICABLE RANGES: 0..1.)
*
* @param[out] udpPortNumPtr            - (pointer to) UDP port0/port1 number, according
*                                      to the index
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or udpPortIndex
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpUdpDestPortsGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   udpPortIndex,
    OUT GT_U32   *udpPortNumPtr
);

/**
* @internal cpssDxChPtpMessageTypeCmdSet function
* @endinternal
*
* @brief   Configure packet command per PTP message type.
*         The message type is extracted from the PTP header.
*
* @note   APPLICABLE DEVICES:     Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*                                      (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] domainIndex              - domain index
*                                      (APPLICABLE RANGES: 0..4.)
*                                      (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] messageType              - message type
*                                      (APPLICABLE RANGES: 0..15.)
* @param[in] command                  - assigned  to the packet.
*                                      (APPLICABLE VALUES: FORWARD, MIRROR, TRAP, HARD_DROP, SOFT_DROP)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum, messageType or command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpMessageTypeCmdSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   domainIndex,
    IN GT_U32                   messageType,
    IN CPSS_PACKET_CMD_ENT      command
);

/**
* @internal cpssDxChPtpMessageTypeCmdGet function
* @endinternal
*
* @brief   Get packet command per PTP message type.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*                                      (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] domainIndex              - domain index
*                                      (APPLICABLE RANGES: 0..4.)
*                                      (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] messageType              - message type
*                                      (APPLICABLE RANGES: 0..15.)
*
* @param[out] commandPtr               - (pointer to) assigned command to the packet.
*                                      (APPLICABLE VALUES: FORWARD, MIRROR, TRAP, HARD_DROP, SOFT_DROP)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or messageType
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on bad value found in HW
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpMessageTypeCmdGet
(
    IN  GT_U8                    devNum,
    IN  GT_PHYSICAL_PORT_NUM     portNum,
    IN  GT_U32                   domainIndex,
    IN  GT_U32                   messageType,
    OUT CPSS_PACKET_CMD_ENT      *commandPtr
);

/**
* @internal cpssDxChPtpCpuCodeBaseSet function
* @endinternal
*
* @brief   Set CPU code base assigned to trapped and mirrored PTP packets.
*         A PTP packet's CPU code is computed as base CPU code + messageType
*         (where message type is taken from PTP header).
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] cpuCode                  - The base of CPU code assigned to PTP packets mirrored
*                                      or trapped to CPU.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or cpuCode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpCpuCodeBaseSet
(
    IN GT_U8                        devNum,
    IN CPSS_NET_RX_CPU_CODE_ENT     cpuCode
);

/**
* @internal cpssDxChPtpCpuCodeBaseGet function
* @endinternal
*
* @brief   Get CPU code base assigned to trapped and mirrored PTP packets.
*         A PTP packet's CPU code is computed as base CPU code + messageType
*         (where message type is taken from PTP header).
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] cpuCodePtr               - (pointer to) The base of CPU code assigned to PTP
*                                      packets mirrored or trapped to CPU.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpCpuCodeBaseGet
(
    IN  GT_U8                        devNum,
    OUT CPSS_NET_RX_CPU_CODE_ENT     *cpuCodePtr
);

/**
* @internal cpssDxChPtpTodCounterFunctionSet function
* @endinternal
*
* @brief   The function configures type of TOD counter action that will be
*         performed once triggered by cpssDxChPtpTodCounterFunctionTriggerSet()
*         API or by pulse from an external interface.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Tx, Rx or both directions
*                                      (APPLICABLE DEVICES Lion2.)
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] function                 - One of the four possible TOD counter functions
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTodCounterFunctionSet
(
    IN GT_U8                                    devNum,
    IN CPSS_PORT_DIRECTION_ENT                  direction,
    IN CPSS_DXCH_PTP_TAI_ID_STC                 *taiIdPtr,
    IN CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT       function
);

/**
* @internal cpssDxChPtpTodCounterFunctionGet function
* @endinternal
*
* @brief   Get type of TOD counter action that will be performed once triggered by
*         cpssDxChPtpTodCounterFunctionTriggerSet() API or by pulse from an
*         external interface.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Tx, Rx or both directions
*                                      (APPLICABLE DEVICES Lion2.)
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] functionPtr              - (pointer to) One of the four possible TOD counter
*                                      functions
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTodCounterFunctionGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_PORT_DIRECTION_ENT                 direction,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    OUT CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT      *functionPtr
);

/**
* @internal cpssDxChPtpTodCounterFunctionTriggerSet function
* @endinternal
*
* @brief   Trigger TOD (Time of Day) counter function accorging to the function set
*         by cpssDxChPtpTodCounterFunctionSet().
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Tx, Rx or both directions
*                                      (APPLICABLE DEVICES Lion2.)
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - when previous TOD triggered action is not finished yet
* @retval GT_BAD_PARAM             - on wrong devNum or direction
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTodCounterFunctionTriggerSet
(
    IN GT_U8                       devNum,
    IN CPSS_PORT_DIRECTION_ENT     direction,
    IN  CPSS_DXCH_PTP_TAI_ID_STC   *taiIdPtr
);

/**
* @internal cpssDxChPtpTodCounterFunctionTriggerStatusGet function
* @endinternal
*
* @brief   Get status of trigger TOD (Time of Day) counter function.
*
* @note   APPLICABLE DEVICES:     Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] finishPtr                - (pointer to) GT_TRUE - The trigger is on, the TOD
*                                      function is not finished.
*                                      GT_FALSE - The trigger is off, the TOD function is
*                                      finished.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or direction
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTodCounterFunctionTriggerStatusGet
(
    IN GT_U8                       devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC   *taiIdPtr,
    OUT GT_BOOL                    *enablePtr
);

/**
* @internal cpssDxChPtpTaiClockModeSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Clock Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier)
* @param[in] clockMode                - clock interface using mode.
*                                      Output mode relevant to Global TAI Instance only.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiClockModeSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    IN  CPSS_DXCH_PTP_TAI_CLOCK_MODE_ENT        clockMode
);

/**
* @internal cpssDxChPtpTaiClockModeGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Clock Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier)
*
* @param[out] clockModePtr             - (pointer to) clock interface using mode.
*                                      Output mode relevant to Global TAI Instance only.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiClockModeGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    OUT CPSS_DXCH_PTP_TAI_CLOCK_MODE_ENT        *clockModePtr
);

/**
* @internal cpssDxChPtpTaiInternalClockGenerateEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Internal Clock Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] enable                   - Internal Clock Generate Enable.
*                                      GT_TRUE - the internal clock generator
*                                      generates a clock signal
*                                      GT_FALSE - the internal clock not generated.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiInternalClockGenerateEnableSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    IN  GT_BOOL                                 enable
);

/**
* @internal cpssDxChPtpTaiInternalClockGenerateEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Internal Clock Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] enablePtr                - (pointer to) Internal Clock Generate Enable.
*                                      GT_TRUE - the internal clock generator
*                                      generates a clock
*                                      GT_FALSE - the internal clock not generated.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiInternalClockGenerateEnableGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    OUT GT_BOOL                                 *enablePtr
);

/**
* @internal cpssDxChPtpTaiPClkDriftAdjustEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) PTP Clock Adjust Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] enable                   - PTP PClock Drift Adjust Enable.
*                                      GT_TRUE - the PClk is affected by the <Drift Adjustment> or not.
*                                      GT_FALSE - the PClk reflects the frequency of the free running TOD,
*                                      without any frequency adjustments.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiPClkDriftAdjustEnableSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    IN  GT_BOOL                                 enable
);

/**
* @internal cpssDxChPtpTaiPClkDriftAdjustEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) PTP Clock Adjust Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] enablePtr                - (pointer to) PTP PClock Drift Adjust Enable.
*                                      GT_TRUE - the PClk is affected by the <Drift Adjustment> or not.
*                                      GT_FALSE - the PClk reflects the frequency of the free running TOD,
*                                      without any frequency adjustments.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiPClkDriftAdjustEnableGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    OUT GT_BOOL                                 *enablePtr
);

/**
* @internal cpssDxChPtpTaiCaptureOverrideEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Capture Override Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] enable                   - Capture Override Enable.
*                                      When both TOD Capture registers are valid and capture
*                                      trigger arrives:
*                                      GT_TRUE  - the TOD Capture Value0 is overwritten.
*                                      GT_FALSE - the TOD Capture Value0 unchanged.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiCaptureOverrideEnableSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    IN  GT_BOOL                                 enable
);

/**
* @internal cpssDxChPtpTaiCaptureOverrideEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Capture Override Enable
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] enablePtr                - (pointer to) Capture Override Enable.
*                                      When both TOD Capture registers are valid and capture
*                                      trigger arrives:
*                                      GT_TRUE  - the TOD Capture Value0 is overwritten.
*                                      GT_FALSE - the TOD Capture Value0 unchanged.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiCaptureOverrideEnableGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    OUT GT_BOOL                                 *enablePtr
);

/**
* @internal cpssDxChPtpTaiInputTriggersCountEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Count Input Triggers Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] enable                   - Count Input Triggers Enable.
*                                      GT_TRUE  - each incoming trigger is counted.
*                                      GT_FALSE - incoming triggers not counted.
*                                      see cpssDxChPtpTaiIncomingTriggerCounterSet/Get
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiInputTriggersCountEnableSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    IN  GT_BOOL                                 enable
);

/**
* @internal cpssDxChPtpTaiInputTriggersCountEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Count Input Triggers Enable
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] enablePtr                - (pointer to) Count Input Triggers Enable.
*                                      GT_TRUE  - each incoming trigger is counted.
*                                      GT_FALSE - incoming triggers not counted.
*                                      see cpssDxChPtpTaiIncomingTriggerCounterSet/Get
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiInputTriggersCountEnableGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    OUT GT_BOOL                                 *enablePtr
);

/**
* @internal cpssDxChPtpTaiExternalPulseWidthSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) External Pulse Width.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] extPulseWidth            - External Pulse Width.
*                                      width of the pulse in nanoseconds of generated output trigger.
*                                      (APPLICABLE RANGES: 0..2^28-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on bad extPulseWidth
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiExternalPulseWidthSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    IN  GT_U32                                  extPulseWidth
);

/**
* @internal cpssDxChPtpTaiExternalPulseWidthGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) External Pulse Width
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] extPulseWidthPtr         -  (pointer to) External Pulse Width.
*                                      width of the pulse in nanoseconds of generated output trigger.
*                                      (APPLICABLE RANGES: 0..2^28-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiExternalPulseWidthGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    OUT GT_U32                                  *extPulseWidthPtr
);

/**
* @internal cpssDxChPtpTaiTodSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) TOD values.
*         The TOD will be updated by triggering an appropriate function.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] todValueType             - type of TOD value.
*                                      Valid types are
*                                      CPSS_DXCH_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E,
*                                      CPSS_DXCH_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_MASK_E,
*                                      CPSS_DXCH_PTP_TAI_TOD_TYPE_LOAD_VALUE_E.
* @param[in] todValuePtr              - (pointer to) TOD value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API does not activate any triggers, other APIs does it
*
*/
GT_STATUS cpssDxChPtpTaiTodSet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    IN  CPSS_DXCH_PTP_TAI_TOD_TYPE_ENT        todValueType,
    IN  CPSS_DXCH_PTP_TOD_COUNT_STC           *todValuePtr
);

/**
* @internal cpssDxChPtpTaiTodGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD values.
*         The TOD was captured by triggering an appropriate function.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] todValueType             - type of TOD value.
*
* @param[out] todValuePtr              - (pointer to) TOD value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiTodGet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    IN  CPSS_DXCH_PTP_TAI_TOD_TYPE_ENT        todValueType,
    OUT CPSS_DXCH_PTP_TOD_COUNT_STC           *todValuePtr
);

/**
* @internal cpssDxChPtpTaiOutputTriggerEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Output Trigger Generation Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] enable                   -   output trigger generation.
*                                      GT_TRUE  - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note use cpssDxChPtpTaiTodSet with
*       CPSS_DXCH_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E and
*       the time to trigger generation.
*
*/
GT_STATUS cpssDxChPtpTaiOutputTriggerEnableSet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    IN  GT_BOOL                               enable
);

/**
* @internal cpssDxChPtpTaiOutputTriggerEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Output Trigger Generation Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] enablePtr                - (pointer to) enable output trigger generation.
*                                      GT_TRUE  - enable GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiOutputTriggerEnableGet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    OUT GT_BOOL                               *enablePtr
);

/**
* @internal cpssDxChPtpTaiTodStepSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) TOD Step.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] todStepPtr               - (pointer to) TOD Step.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiTodStepSet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC          *taiIdPtr,
    IN  CPSS_DXCH_PTP_TAI_TOD_STEP_STC    *todStepPtr
);

/**
* @internal cpssDxChPtpTaiTodStepGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD Step.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] todStepPtr               - (pointer to) TOD Step.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiTodStepGet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC          *taiIdPtr,
    OUT CPSS_DXCH_PTP_TAI_TOD_STEP_STC    *todStepPtr
);

/**
* @internal cpssDxChPtpTaiFractionalNanosecondDriftSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Fractional Nanosecond Drift.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
* @param[in] fracNanoSecond           - (pointer to) fractional nano seconds drift.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiFractionalNanosecondDriftSet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    IN  GT_32                                 fracNanoSecond
);

/**
* @internal cpssDxChPtpTaiFractionalNanosecondDriftGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Fractional Nanosecond Drift.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] fracNanoSecondPtr        - (pointer to) fractional nano seconds drift.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiFractionalNanosecondDriftGet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    OUT GT_32                                 *fracNanoSecondPtr
);

/**
* @internal cpssDxChPtpTaiPClkCycleSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) PClock Cycle.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] nanoSeconds              - nano seconds cycle.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiPClkCycleSet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    IN  GT_U32                                nanoSeconds
);

/**
* @internal cpssDxChPtpTaiPClkCycleGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) PClock Cycle.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] nanoSecondsPtr           - (pointer to) nano seconds cycle.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiPClkCycleGet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    OUT GT_U32                                *nanoSecondsPtr
);

/**
* @internal cpssDxChPtpTaiClockCycleSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Clock Cycle.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] seconds                  -  part of cycle.
*                                      (APPLICABLE RANGES: 0..3.)
* @param[in] nanoSeconds              - nano seconds part of cycle.
*                                      (APPLICABLE RANGES: 0..2^28-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiClockCycleSet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    IN  GT_U32                                seconds,
    IN  GT_U32                                nanoSeconds
);

/**
* @internal cpssDxChPtpTaiClockCycleGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Clock Cycle.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] secondsPtr               - (pointer to) seconds part of cycle.
*                                      (APPLICABLE RANGES: 0..3.)
* @param[out] nanoSecondsPtr           - (pointer to) nano seconds part of cycle.
*                                      (APPLICABLE RANGES: 0..2^28-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiClockCycleGet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    OUT GT_U32                                *secondsPtr,
    OUT GT_U32                                *nanoSecondsPtr
);


/**
* @internal cpssDxChPtpTaiTodCaptureStatusGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD Capture Status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] captureIndex             - Capture Index.
*                                      (APPLICABLE RANGES: 0..1.)
*
* @param[out] validPtr                 - (pointer to)Capture is valid.
*                                      GT_TRUE - valid, GT_FALSE - invalid
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiTodCaptureStatusGet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    IN  GT_U32                                captureIndex,
    OUT GT_BOOL                               *validPtr
);

/**
* @internal cpssDxChPtpTaiTodUpdateCounterGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD Update Counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] valuePtr                 - (pointer to)TOD Update Counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Counter is Clear On Read. Value incremented at each
*       TOD Update/Increment/Decrement function.
*
*/
GT_STATUS cpssDxChPtpTaiTodUpdateCounterGet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    OUT GT_U32                                *valuePtr
);

/**
* @internal cpssDxChPtpTaiIncomingTriggerCounterSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Incoming Trigger Counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] value                    -  to set.
*                                      (APPLICABLE RANGES: 0..255.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiIncomingTriggerCounterSet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    IN  GT_U32                                value
);

/**
* @internal cpssDxChPtpTaiIncomingTriggerCounterGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Incoming Trigger Counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] valuePtr                 - (pointer to)value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiIncomingTriggerCounterGet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    OUT GT_U32                                *valuePtr
);

/**
* @internal cpssDxChPtpTaiIncomingClockCounterSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Incoming Clock Counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] enable                   -  counting.
*                                      GT_TRUE - enable, GT_FALSE - disable.
* @param[in] value                    -  to set.
*                                      (APPLICABLE DEVICES Bobcat2; Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      (APPLICABLE RANGES: 0..2^16-1, Bobcat2,
*                                                          0..2^32-1, Falcon; AC5P; AC5X; Harrier)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counter became Read Only and Clear on Read starting from Caelum devices untill Falcon devices.
*
*/
GT_STATUS cpssDxChPtpTaiIncomingClockCounterSet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    IN  GT_BOOL                               enable,
    IN  GT_U32                                value
);

/**
* @internal cpssDxChPtpTaiIncomingClockCounterGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Incoming Clock Counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] enablePtr                - (pointer to)enable counting.
*                                      GT_TRUE - enable, GT_FALSE - disable.
* @param[out] valuePtr                 - (pointer to)value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiIncomingClockCounterGet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC              *taiIdPtr,
    OUT GT_BOOL                               *enablePtr,
    OUT GT_U32                                *valuePtr
);


/**
* @internal cpssDxChPtpTaiGracefulStepSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Graceful Step.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] gracefulStep             - Graceful Step in nanoseconds.
*                                      (APPLICABLE RANGES: 0..31.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on wrong gracefulStep
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiGracefulStepSet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC          *taiIdPtr,
    IN  GT_U32                             gracefulStep
);

/**
* @internal cpssDxChPtpTaiGracefulStepGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Graceful Step.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
*                                      Single TAI unit must be specified.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) taiIdPtr is ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] gracefulStepPtr          - (pointer to) Graceful Step in nanoseconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiGracefulStepGet
(
    IN  GT_U8                             devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC          *taiIdPtr,
    OUT GT_U32                            *gracefulStepPtr
);

/**
* @internal cpssDxChPtpTsuControlSet function
* @endinternal
*
* @brief   Set TSU (Time Stamp Unit) Control.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] controlPtr               - (pointer to) control structure.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsuControlSet
(
    IN  GT_U8                             devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    IN  CPSS_DXCH_PTP_TSU_CONTROL_STC     *controlPtr
);

/**
* @internal cpssDxChPtpTsuControlGet function
* @endinternal
*
* @brief   Get TSU (Time Stamp Unit) Control.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*
* @param[out] controlPtr               - (pointer to) control structure.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsuControlGet
(
    IN  GT_U8                             devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    OUT CPSS_DXCH_PTP_TSU_CONTROL_STC     *controlPtr
);

/**
* @internal cpssDxChPtpTsuTxTimestampQueueRead function
* @endinternal
*
* @brief   Read TSU (Time Stamp Unit) TX Timestamp Queue Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] queueNum                 - queue number.
*                                      (APPLICABLE RANGES: 0..1.)
*
* @param[out] entryPtr                 - (pointer to) TX Timestamp Queue Entry structure.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Entry deleted from the Queue by reading it.
*
*/
GT_STATUS cpssDxChPtpTsuTxTimestampQueueRead
(
    IN  GT_U8                                              devNum,
    IN  GT_PHYSICAL_PORT_NUM                               portNum,
    IN  GT_U32                                             queueNum,
    OUT CPSS_DXCH_PTP_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC     *entryPtr
);

/**
* @internal cpssDxChPtpTsuCountersClear function
* @endinternal
*
* @brief   Clear All TSU (Time Stamp Unit) Packet counters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsuCountersClear
(
    IN  GT_U8                             devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum
);

/**
* @internal cpssDxChPtpTsuPacketCounterGet function
* @endinternal
*
* @brief   Get TSU (Time Stamp Unit) packet counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] counterType              - counter type (see enum)
* @param[in] Index                    - counter index. (APPLICABLE DEVICES: Falcon, AC5P, AC5X, Harrier, Ironman.
*                                       Falcon: APPLICABLE RANGE: 0..3.
*                                       AC5P, Harrier: APPLICABLE RANGE: 0..31.
*                                       AC5X,Ironman: APPLICABLE RANGE: 0..63.)
*
* @param[out] valuePtr                 - (pointer to) counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsuPacketCounterGet
(
    IN  GT_U8                                     devNum,
    IN  GT_PHYSICAL_PORT_NUM                      portNum,
    IN  CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_ENT counterType,
    IN  GT_U32                                    index,
    OUT GT_U32                                    *valuePtr
);

/**
* @internal cpssDxChPtpTsuNtpTimeOffsetGet function
* @endinternal
*
* @brief   Get TSU (Time Stamp Unit) NTP Time Offset.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*
* @param[out] ntpTimeOffsetPtr         - (pointer to) NTP PTP Time Offset measured in seconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsuNtpTimeOffsetGet
(
    IN  GT_U8                             devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    OUT GT_U32                            *ntpTimeOffsetPtr
);

/**
* @internal cpssDxChPtpTsTagGlobalCfgSet function
* @endinternal
*
* @brief   Set Timestamp Global Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] tsTagGlobalCfgPtr        - (pointer to) TS Tag Global Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsTagGlobalCfgSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC *tsTagGlobalCfgPtr
);

/**
* @internal cpssDxChPtpTsTagGlobalCfgGet function
* @endinternal
*
* @brief   Get Timestamp Global Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] tsTagGlobalCfgPtr        - (pointer to) TS Tag Global Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsTagGlobalCfgGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC *tsTagGlobalCfgPtr
);

/**
* @internal cpssDxChPtpTsTagPortCfgSet function
* @endinternal
*
* @brief   Set Timestamp Port Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] tsTagPortCfgPtr          - (pointer to) TS Tag Port Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsTagPortCfgSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_PTP_TS_TAG_PORT_CFG_STC   *tsTagPortCfgPtr
);

/**
* @internal cpssDxChPtpTsTagPortCfgGet function
* @endinternal
*
* @brief   Get Timestamp Port Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*
* @param[out] tsTagPortCfgPtr          - (pointer to) TS Tag Port Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsTagPortCfgGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT CPSS_DXCH_PTP_TS_TAG_PORT_CFG_STC   *tsTagPortCfgPtr
);

/**
* @internal cpssDxChPtpTsDelayIngressPortDelaySet function
* @endinternal
*
* @brief   Set Ingress Port Delay Correction.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] ingressDelayCorr         - Ingress Port Delay Correction.
*                                      (APPLICABLE RANGES: -10^9..10^9-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDelayIngressPortDelaySet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_32                   ingressDelayCorr
);

/**
* @internal cpssDxChPtpTsDelayIngressPortDelayGet function
* @endinternal
*
* @brief   Get Ingress Port Delay Correction.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*
* @param[out] ingressDelayCorrPtr      - (pointer to) Ingress Port Delay Correction.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDelayIngressPortDelayGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_32                   *ingressDelayCorrPtr
);

/**
* @internal cpssDxChPtpTsDelayIngressLinkDelaySet function
* @endinternal
*
* @brief   Set Ingress Link Delay.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] domainProfile            - domain profile
*                                      (APPLICABLE RANGES: 0..8.)
* @param[in] ingressDelay             - Ingress Port Delay.
*                                      (APPLICABLE RANGES: -10^9..10^9-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDelayIngressLinkDelaySet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  domainProfile,
    IN  GT_32                   ingressDelay
);

/**
* @internal cpssDxChPtpTsDelayIngressLinkDelayGet function
* @endinternal
*
* @brief   Get Ingress Link Delay.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] domainProfile            - domain profile
*                                      (APPLICABLE RANGES: 0..8.)
*
* @param[out] ingressDelayPtr          - (pointer to) Ingress Port Delay.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDelayIngressLinkDelayGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  domainProfile,
    OUT GT_32                   *ingressDelayPtr
);

/**
* @internal cpssDxChPtpTsDelayEgressPipeDelaySet function
* @endinternal
*
* @brief   Set Egress Pipe Delay Corrections.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (not supported for CPU ports).
* @param[in] egrDelayValPtr           - (pointer to) Egress Pipe Delay Corrections.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDelayEgressPipeDelaySet
(
    IN  GT_U8                                           devNum,
    IN  GT_PHYSICAL_PORT_NUM                            portNum,
    IN  CPSS_DXCH_PTP_TS_EGRESS_PIPE_DELAY_VALUES_STC   *egrDelayValPtr
);

/**
* @internal cpssDxChPtpTsDelayEgressPipeDelayGet function
* @endinternal
*
* @brief   Get Egress Pipe Delay Corrections.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (not supported for CPU ports).
*
* @param[out] egrDelayValPtr          - (pointer to) Egress Pipe Delay Corrections.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDelayEgressPipeDelayGet
(
    IN  GT_U8                                           devNum,
    IN  GT_PHYSICAL_PORT_NUM                            portNum,
    OUT CPSS_DXCH_PTP_TS_EGRESS_PIPE_DELAY_VALUES_STC   *egrDelayValPtr
);

/**
* @internal cpssDxChPtpTsDelayEgressAsymmetryCorrectionSet function
* @endinternal
*
* @brief   Set Egress Asymmetry Correction.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] domainProfile            - domain profile.
*                                      (APPLICABLE RANGES: 0..8.)
* @param[in] egrAsymmetryCorr         - Egress Asymmetry Correction.
*                                      (APPLICABLE RANGES: -10^9..10^9-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDelayEgressAsymmetryCorrectionSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  domainProfile,
    IN  GT_32                   egrAsymmetryCorr
);

/**
* @internal cpssDxChPtpTsDelayEgressAsymmetryCorrectionGet function
* @endinternal
*
* @brief   Get Egress Asymmetry Correction.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] domainProfile            - domain profile.
*                                      (APPLICABLE RANGES: 0..8.)
*
* @param[out] egrAsymmetryCorrPtr      - (pointer to) Egress Asymmetry Correction.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDelayEgressAsymmetryCorrectionGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  domainProfile,
    OUT GT_32                   *egrAsymmetryCorrPtr
);

/**
* @internal cpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableSet function
* @endinternal
*
* @brief   Set Ingress Correction Field Piggyback Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] ingrCorrFldPBEnable      - Ingress Correction Field Piggyback Enable.
*                                      GT_TRUE  - Correction field of PTP header contains
*                                      ingress timestamp.
*                                      GT_FALSE - Correction field of PTP header reserved.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 ingrCorrFldPBEnable
);

/**
* @internal cpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableGet function
* @endinternal
*
* @brief   Get Ingress Correction Field Piggyback Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*
* @param[out] ingrCorrFldPBEnablePtr   - (pointer to) Ingress Correction Field Piggyback Enable.
*                                      GT_TRUE  - Correction field of PTP header contains
*                                      ingress timestamp.
*                                      GT_FALSE - Correction field of PTP header reserved.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *ingrCorrFldPBEnablePtr
);

/**
* @internal cpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableSet function
* @endinternal
*
* @brief   Set Egress Correction Field Piggyback Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] egrCorrFldPBEnable       - Egress Correction Field Piggyback Enable.
*                                      GT_TRUE  - Correction field of PTP header contains
*                                      ingress timestamp.
*                                      GT_FALSE - Correction field of PTP header reserved.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 egrCorrFldPBEnable
);

/**
* @internal cpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableGet function
* @endinternal
*
* @brief   Get Egress Correction Field Piggyback Enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*
* @param[out] egrCorrFldPBEnablePtr    - (pointer to) Egress Correction Field Piggyback Enable.
*                                      GT_TRUE  - Correction field of PTP header contains
*                                      ingress timestamp.
*                                      GT_FALSE - Correction field of PTP header reserved.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *egrCorrFldPBEnablePtr
);

/**
* @internal cpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeSet function
* @endinternal
*
* @brief   Set Egress Time Correction TAI Select Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] egrTimeCorrTaiSelMode    - Egress Time Correction TAI Select Mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeSet
(
    IN  GT_U8                                     devNum,
    IN  GT_PHYSICAL_PORT_NUM                      portNum,
    IN  CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT   egrTimeCorrTaiSelMode
);

/**
* @internal cpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeGet function
* @endinternal
*
* @brief   Get Egress Time Correction TAI Select Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*
* @param[out] egrTimeCorrTaiSelModePtr - (pointer to)Egress Time Correction TAI Select Mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeGet
(
    IN  GT_U8                                     devNum,
    IN  GT_PHYSICAL_PORT_NUM                      portNum,
    OUT CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT   *egrTimeCorrTaiSelModePtr
);

/**
* @internal cpssDxChPtpTsDelayEgressTimestampTaiSelectSet function
* @endinternal
*
* @brief   Set Egress Timestamp TAI Select.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Caelum; Aldrin; AC3X; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] egrTsTaiNum              - Egress Timestamp TAI Number.
*                                      (APPLICABLE RANGES: 0..1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDelayEgressTimestampTaiSelectSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  egrTsTaiNum
);

/**
* @internal cpssDxChPtpTsDelayEgressTimestampTaiSelectGet function
* @endinternal
*
* @brief   Get Egress Timestamp TAI Select.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Caelum; Aldrin; AC3X; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*
* @param[out] egrTsTaiNumPtr           - (pointer to)Egress Timestamp TAI Number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDelayEgressTimestampTaiSelectGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *egrTsTaiNumPtr
);

/**
* @internal cpssDxChPtpTsCfgTableSet function
* @endinternal
*
* @brief   Set Timestamp Configuration Table Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] entryIndex               - entry index.
*                                      (APPLICABLE RANGES: 0..127.)
* @param[in] entryPtr                 - (pointer to)Timestamp Configuration Table Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsCfgTableSet
(
    IN  GT_U8                          devNum,
    IN  GT_U32                         entryIndex,
    IN  CPSS_DXCH_PTP_TS_CFG_ENTRY_STC *entryPtr
);

/**
* @internal cpssDxChPtpTsCfgTableGet function
* @endinternal
*
* @brief   Get Timestamp Configuration Table Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] entryIndex               - entry index.
*                                      (APPLICABLE RANGES: 0..127.)
*
* @param[out] entryPtr                 - (pointer to)Timestamp Configuration Table Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsCfgTableGet
(
    IN  GT_U8                          devNum,
    IN  GT_U32                         entryIndex,
    OUT CPSS_DXCH_PTP_TS_CFG_ENTRY_STC *entryPtr
);

/**
* @internal cpssDxChPtpTsLocalActionTableSet function
* @endinternal
*
* @brief   Set Timestamp Local Action Table Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] domainProfile            - domain profile.
*                                      (APPLICABLE RANGES:
*                                      0..4.)
* @param[in] messageType              - message type
*                                      (APPLICABLE RANGES: 0..15.)
* @param[in] entryPtr                 - (pointer to)Timestamp Local Action Table Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsLocalActionTableSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U32                                  domainProfile,
    IN  GT_U32                                  messageType,
    IN  CPSS_DXCH_PTP_TS_LOCAL_ACTION_ENTRY_STC *entryPtr
);

/**
* @internal cpssDxChPtpTsLocalActionTableGet function
* @endinternal
*
* @brief   Get Timestamp Local Action Table Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] domainProfile            - domain profile.
*                                      (APPLICABLE RANGES: 0..4)
* @param[in] messageType              - message type
*                                      (APPLICABLE RANGES: 0..15.)
*
* @param[out] entryPtr                 - (pointer to)Timestamp Local Action Table Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsLocalActionTableGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U32                                  domainProfile,
    IN  GT_U32                                  messageType,
    OUT CPSS_DXCH_PTP_TS_LOCAL_ACTION_ENTRY_STC *entryPtr
);

/**
* @internal cpssDxChPtpTsUdpChecksumUpdateModeSet function
* @endinternal
*
* @brief   Set Timestamp UDP Checksum Update Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] udpCsUpdModePtr          - (pointer to)Timestamp UDP Checksum Update Mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsUdpChecksumUpdateModeSet
(
    IN  GT_U8                                         devNum,
    IN  CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC *udpCsUpdModePtr
);

/**
* @internal cpssDxChPtpTsUdpChecksumUpdateModeGet function
* @endinternal
*
* @brief   Get Timestamp UDP Checksum Update Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] udpCsUpdModePtr          - (pointer to)Timestamp UDP Checksum Update Mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsUdpChecksumUpdateModeGet
(
    IN  GT_U8                                         devNum,
    OUT CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC *udpCsUpdModePtr
);

/**
* @internal cpssDxChPtpOverEthernetEnableSet function
* @endinternal
*
* @brief   Set enable PTP over Ethernet packet.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpOverEthernetEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_BOOL   enable
);

/**
* @internal cpssDxChPtpOverEthernetEnableGet function
* @endinternal
*
* @brief   Get enable PTP over Ethernet packet.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpOverEthernetEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChPtpOverUdpEnableSet function
* @endinternal
*
* @brief   Set enable PTP over UDP packet.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpOverUdpEnableSet
(
    IN  GT_U8     devNum,
    IN  GT_BOOL   enable
);

/**
* @internal cpssDxChPtpOverUdpEnableGet function
* @endinternal
*
* @brief   Get enable PTP over UDP packet.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpOverUdpEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChPtpDomainModeSet function
* @endinternal
*
* @brief   Set PTP domain mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] domainIndex              - domain number.
*                                      (APPLICABLE RANGES: 0..3.)
* @param[in] domainMode               - domain mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpDomainModeSet
(
    IN  GT_U8                                  devNum,
    IN  GT_U32                                 domainIndex,
    IN  CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_ENT  domainMode
);

/**
* @internal cpssDxChPtpDomainModeGet function
* @endinternal
*
* @brief   Get PTP domain mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] domainIndex              - domain index.
*                                      (APPLICABLE RANGES: 0..3.)
*
* @param[out] domainModePtr            - (pointer to) domain mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpDomainModeGet
(
    IN  GT_U8                                  devNum,
    IN  GT_U32                                 domainIndex,
    OUT CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_ENT  *domainModePtr
);

/**
* @internal cpssDxChPtpDomainV1IdSet function
* @endinternal
*
* @brief   Set PTP V1 domain Id.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] domainIndex              - domain index.
*                                      (APPLICABLE RANGES: 0..3.)
* @param[in] domainIdArr[4]           - domain Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP V1 Header contains 128-bit domain Id.
*       4 domain Id values mapped to domainIndex 0-3
*       all other domain Id values mapped to default domain
*
*/
GT_STATUS cpssDxChPtpDomainV1IdSet
(
    IN  GT_U8        devNum,
    IN  GT_U32       domainIndex,
    IN  GT_U32       domainIdArr[4]
);

/**
* @internal cpssDxChPtpDomainV1IdGet function
* @endinternal
*
* @brief   Get PTP V1 domain Id.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] domainIndex              - domain index.
*                                      (APPLICABLE RANGES: 0..3.)
*
* @param[out] domainIdArr[4]           - domain Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP V1 Header contains 128-bit domain Id.
*       4 domain Id values mapped to domainIndex 0-3
*       all other domain Id values mapped to default domain
*
*/
GT_STATUS cpssDxChPtpDomainV1IdGet
(
    IN  GT_U8        devNum,
    IN  GT_U32       domainIndex,
    OUT GT_U32       domainIdArr[4]
);

/**
* @internal cpssDxChPtpDomainV2IdSet function
* @endinternal
*
* @brief   Set PTP V2 domain Id.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] domainIndex              - domain index.
*                                      (APPLICABLE RANGES: 0..3.)
* @param[in] domainId                 - domain Id
*                                      (APPLICABLE RANGES: 0..255.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP V2 Header contains 8-bit domain Id.
*       4 domain Id values mapped to domainIndex 0-3
*       all other domain Id values mapped to default domain
*
*/
GT_STATUS cpssDxChPtpDomainV2IdSet
(
    IN  GT_U8        devNum,
    IN  GT_U32       domainIndex,
    IN  GT_U32       domainId
);

/**
* @internal cpssDxChPtpDomainV2IdGet function
* @endinternal
*
* @brief   Get PTP V2 domain Id.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] domainIndex              - domain index.
*                                      (APPLICABLE RANGES: 0..3.)
*
* @param[out] domainIdPtr              - (pointer to) domain Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP V2 Header contains 8-bit domain Id.
*       4 domain Id values mapped to domainIndex 0-3
*       all other domain Id values mapped to default domain
*
*/
GT_STATUS cpssDxChPtpDomainV2IdGet
(
    IN  GT_U8        devNum,
    IN  GT_U32       domainIndex,
    OUT GT_U32       *domainIdPtr
);

/**
* @internal cpssDxChPtpEgressDomainTableSet function
* @endinternal
*
* @brief   Set Egress Domain Table Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] domainIndex              - domain index.
*                                      (APPLICABLE RANGES: 0..4.)
* @param[in] entryPtr                 - (pointer to) Domain Table Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpEgressDomainTableSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U32                                  domainIndex,
    IN  CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC   *entryPtr
);

/**
* @internal cpssDxChPtpEgressDomainTableGet function
* @endinternal
*
* @brief   Get Egress Domain Table Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] domainIndex              - domain index.
*                                      (APPLICABLE RANGES: 0..4.)
*
* @param[out] entryPtr                 - (pointer to) Egress Domain Table Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpEgressDomainTableGet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U32                                  domainIndex,
    OUT CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC   *entryPtr
);

/**
* @internal cpssDxChPtpIngressExceptionCfgSet function
* @endinternal
*
* @brief   Set PTP packet Ingress Exception Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] ingrExceptionCfgPtr      - (pointer to) PTP packet Ingress Exception Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpIngressExceptionCfgSet
(
    IN  GT_U8                                     devNum,
    IN  CPSS_DXCH_PTP_INGRESS_EXCEPTION_CFG_STC   *ingrExceptionCfgPtr
);

/**
* @internal cpssDxChPtpIngressExceptionCfgGet function
* @endinternal
*
* @brief   Get PTP packet Ingress Exception Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] ingrExceptionCfgPtr      - (pointer to) PTP packet Ingress Exception Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpIngressExceptionCfgGet
(
    IN  GT_U8                                     devNum,
    OUT CPSS_DXCH_PTP_INGRESS_EXCEPTION_CFG_STC   *ingrExceptionCfgPtr
);

/**
* @internal cpssDxChPtpIngressPacketCheckingModeSet function
* @endinternal
*
* @brief   Set PTP packet Ingress Checking Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] messageType              - message type
*                                      (APPLICABLE RANGES: 0..15.)
* @param[in] domainIndex              - domain index
*                                      (APPLICABLE RANGES: 0..4.)
* @param[in] checkingMode             - PTP packet ingress checking mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum, messageType or command
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpIngressPacketCheckingModeSet
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   domainIndex,
    IN  GT_U32                                   messageType,
    IN  CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_ENT  checkingMode
);

/**
* @internal cpssDxChPtpIngressPacketCheckingModeGet function
* @endinternal
*
* @brief   Get PTP packet Ingress Checking Mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] domainIndex              - domain index
*                                      (APPLICABLE RANGES: 0..4.)
* @param[in] messageType              - message type
*                                      (APPLICABLE RANGES: 0..15.)
*
* @param[out] checkingModePtr          - (pointer to)PTP packet ingress checking mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or messageType
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on bad value found in HW
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpIngressPacketCheckingModeGet
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   domainIndex,
    IN  GT_U32                                   messageType,
    OUT CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_ENT  *checkingModePtr
);

/**
* @internal cpssDxChPtpIngressExceptionCounterGet function
* @endinternal
*
* @brief   Get PTP Ingress Exception packet Counter.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] counterPtr               - (pointer to) PTP Ingress Exception packet Counter.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counter is Clear On Read.
*
*/
GT_STATUS cpssDxChPtpIngressExceptionCounterGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *counterPtr
);

/**
* @internal cpssDxChPtpEgressExceptionCfgSet function
* @endinternal
*
* @brief   Set PTP packet Egress Exception Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] egrExceptionCfgPtr       - (pointer to) PTP packet Egress Exception Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpEgressExceptionCfgSet
(
    IN  GT_U8                                     devNum,
    IN  CPSS_DXCH_PTP_EGRESS_EXCEPTION_CFG_STC   *egrExceptionCfgPtr
);

/**
* @internal cpssDxChPtpEgressExceptionCfgGet function
* @endinternal
*
* @brief   Get PTP packet Egress Exception Configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] egrExceptionCfgPtr       - (pointer to) PTP packet Egress Exception Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpEgressExceptionCfgGet
(
    IN  GT_U8                                    devNum,
    OUT CPSS_DXCH_PTP_EGRESS_EXCEPTION_CFG_STC   *egrExceptionCfgPtr
);

/**
* @internal cpssDxChPtpEgressExceptionCountersGet function
* @endinternal
*
* @brief   Get PTP packet Egress Exception Counters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] egrExceptionCntPtr       - (pointer to) PTP packet Egress Exception Counters.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counters are Clear On Read.
*
*/
GT_STATUS cpssDxChPtpEgressExceptionCountersGet
(
    IN  GT_U8                                         devNum,
    OUT CPSS_DXCH_PTP_EGRESS_EXCEPTION_COUNTERS_STC   *egrExceptionCntPtr
);

/**
* @internal cpssDxChPtpTsIngressTimestampQueueEntryRead function
* @endinternal
*
* @brief   Read Ingress Timestamp Queue Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] queueNum                 - Queue Number
*                                      (APPLICABLE RANGES: 0..1.)
*
* @param[out] tsQueueEntryPtr          - (pointer to) Ingress Timestamp Queue Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Queue read causes deleting the entry from the queue.
*       The next read will retrieve the next entry.
*
*/
GT_STATUS cpssDxChPtpTsIngressTimestampQueueEntryRead
(
    IN  GT_U8                                                devNum,
    IN  GT_U32                                               queueNum,
    OUT CPSS_DXCH_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC   *tsQueueEntryPtr
);

/**
* @internal cpssDxChPtpTsEgressTimestampQueueEntryRead function
* @endinternal
*
* @brief   Read Egress Timestamp Queue Entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] queueNum                 - Queue Number
*                                      (APPLICABLE RANGES: 0..1.)
*
* @param[out] tsQueueEntryPtr          - (pointer to) Egress Timestamp Queue Entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Queue read causes deleting the entry from the queue.
*       The next read will retrieve the next entry.
*
*/
GT_STATUS cpssDxChPtpTsEgressTimestampQueueEntryRead
(
    IN  GT_U8                                                devNum,
    IN  GT_U32                                               queueNum,
    OUT CPSS_DXCH_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC    *tsQueueEntryPtr
);

/**
* @internal cpssDxChPtpTsMessageTypeToQueueIdMapSet function
* @endinternal
*
* @brief   Set PTP Message Type To Queue Id Map.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] idMapBmp                 - PTP Message Type To Queue Id Map.
*                                      16 bits, bit per message type
*                                      0 - queue0, 1 - queue1
*                                      (APPLICABLE RANGES: 0..0xFFFF.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsMessageTypeToQueueIdMapSet
(
    IN  GT_U8        devNum,
    IN  GT_U32       idMapBmp
);

/**
* @internal cpssDxChPtpTsMessageTypeToQueueIdMapGet function
* @endinternal
*
* @brief   Get PTP Message Type To Queue Id Map.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] idMapBmpPtr              - (pointer to) PTP Message Type To Queue Id Map.
*                                      16 bits, bit per message type
*                                      0 - queue0, 1 - queue1
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsMessageTypeToQueueIdMapGet
(
    IN  GT_U8        devNum,
    OUT GT_U32       *idMapBmpPtr
);


/**
* @internal cpssDxChPtpTsQueuesSizeSet function
* @endinternal
*
* @brief   Set Timestamp Queues Size.
*         Related to all 2 ingress and 2 egress Timestamp Queues.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] queueSize                - The size of each PTP Queue.
*                                      (APPLICABLE RANGES: 0..256.)
*                                      If Timestamp Queues Override is Enable,
*                                      the size should be (maximal - 2) == 254.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsQueuesSizeSet
(
    IN  GT_U8       devNum,
    IN  GT_U32      queueSize
);

/**
* @internal cpssDxChPtpTsQueuesSizeGet function
* @endinternal
*
* @brief   Get Timestamp Queues Size.
*         Related to all 2 ingress and 2 egress Timestamp Queues.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] queueSizePtr             - (pointer to)The size of each PTP Queue.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsQueuesSizeGet
(
    IN  GT_U8       devNum,
    OUT GT_U32      *queueSizePtr
);

/**
* @internal cpssDxChPtpTsDebugQueuesEntryIdsClear function
* @endinternal
*
* @brief   Set Timestamp Queues Current Entry Ids Clear.
*         For Debug only.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDebugQueuesEntryIdsClear
(
    IN  GT_U8       devNum
);

/**
* @internal cpssDxChPtpTsDebugQueuesEntryIdsGet function
* @endinternal
*
* @brief   Get Timestamp Queues Current Entry IDs.
*         For Debug only.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] queueEntryId0Ptr         - (pointer to)The PTP Queue0 current entry Id.
* @param[out] queueEntryId1Ptr         - (pointer to)The PTP Queue1 current entry Id.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsDebugQueuesEntryIdsGet
(
    IN  GT_U8       devNum,
    OUT GT_U32      *queueEntryId0Ptr,
    OUT GT_U32      *queueEntryId1Ptr
);

/**
* @internal cpssDxChPtpTsNtpTimeOffsetSet function
* @endinternal
*
* @brief   Set TS NTP Time Offset.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] ntpTimeOffset            - NTP PTP Time Offset measured in seconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The same value should be configured per port.
*       See cpssDxChPtpTsuNtpTimeOffsetSet.
*
*/
GT_STATUS cpssDxChPtpTsNtpTimeOffsetSet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            ntpTimeOffset
);

/**
* @internal cpssDxChPtpTsNtpTimeOffsetGet function
* @endinternal
*
* @brief   Get TS NTP Time Offset.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @param[out] ntpTimeOffsetPtr         - (pointer to) NTP PTP Time Offset measured in seconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsNtpTimeOffsetGet
(
    IN  GT_U8                             devNum,
    OUT GT_U32                            *ntpTimeOffsetPtr
);

/**
* @internal cpssDxChPtpTaiPtpPulseInterfaceSet function
* @endinternal
*
* @brief   Set TAI PTP pulse interface.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Ironman.
*
* @param[in] devNum                   - device number.
* @param[in] pulseInterfaceDirection  - PTP pulse interface direction.
*                                      (APPLICABLE VALUES:
*                                      CPSS_DIRECTION_INGRESS_E;
*                                      CPSS_DIRECTION_EGRESS_E.)
*                                      For Bobcat3: (APPLICABLE VALUES:
*                                      CPSS_DIRECTION_EGRESS_E.)
* @param[in] taiNumber                - TAI number selection.
*                                      (relevant for PTP output pulse)
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) parameter ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP pulse interface used for generating/receiving discrete pulses
*       that trigger a time-related operation such as Update or Capture.
*
*/
GT_STATUS cpssDxChPtpTaiPtpPulseInterfaceSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DIRECTION_ENT                      pulseInterfaceDirection,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT            taiNumber
);

/**
* @internal cpssDxChPtpTaiPtpPulseInterfaceGet function
* @endinternal
*
* @brief   Get TAI PTP pulse interface.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Ironman.
*
* @param[in] devNum                   - device number
*
* @param[out] pulseInterfaceDirectionPtr - (pointer to) PTP pulse interface direction.
* @param[out] taiNumberPtr             - (pointer to) TAI number selection.
*                                      (relevant for PTP output pulse)
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) parameter ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP pulse interface used for generating/receiving discrete pulses
*       that trigger a time-related operation such as Update or Capture.
*
*/
GT_STATUS cpssDxChPtpTaiPtpPulseInterfaceGet
(
    IN  GT_U8                                devNum,
    OUT CPSS_DIRECTION_ENT                   *pulseInterfaceDirectionPtr,
    OUT CPSS_DXCH_PTP_TAI_NUMBER_ENT         *taiNumberPtr
);

/**
* @internal cpssDxChPtpTaiPClkInterfaceSet function
* @endinternal
*
* @brief   Set TAI PTP clock interface.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI Number.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) parameter ignored.
*                                      (APPLICABLE DEVICES Bobcat2, Falcon; AC5P; AC5X; Harrier)
* @param[in] clockInterfaceDirection  - PTP clock interface direction.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP clock interface used for generating/receiving a periodic pulse
*       such as 1 PPS signal.
*
*/
GT_STATUS cpssDxChPtpTaiPClkInterfaceSet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT         taiNumber,
    IN  CPSS_DIRECTION_ENT                   clockInterfaceDirection
);

/**
* @internal cpssDxChPtpTaiPClkInterfaceGet function
* @endinternal
*
* @brief   Get TAI PTP clock interface.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) parameter ignored.
*                                      (APPLICABLE DEVICES Bobcat2, Falcon; AC5P; AC5X; Harrier)
*
* @param[out] clockInterfaceDirectionPtr - (pointer to) PTP clock interface direction.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note PTP clock interface used for generating/receiving a periodic pulse
*       such as 1 PPS signal.
*
*/
GT_STATUS cpssDxChPtpTaiPClkInterfaceGet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT         taiNumber,
    OUT CPSS_DIRECTION_ENT                   *clockInterfaceDirectionPtr
);

/**
* @internal cpssDxChPtpTaiPClkOutputInterfaceSet function
* @endinternal
*
* @brief   Set TAI PTP PClk OUT from TAI1/TAI0.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number - used as clock?s source for PClk output mode.
*                                      (relevant for "PTP" mode)
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) parameter ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier)
* @param[in] pclkRcvrClkMode          - clock output interface mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiPClkOutputInterfaceSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber,
    IN  CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_ENT    pclkRcvrClkMode
);

/**
* @internal cpssDxChPtpTaiPClkOutputInterfaceGet function
* @endinternal
*
* @brief   Get TAI PTP PClk OUT from TAI1/TAI0.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Ironman.
*
* @param[in] devNum                   - device number
*
* @param[out] taiNumberPtr             - (pointer to) TAI number - used as clock?s source for PClk output mode.
*                                      (relevant for "PTP" mode)
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) parameter ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier)
* @param[out] pclkRcvrClkModePtr       - (pointer to) clock output interface mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiPClkOutputInterfaceGet
(
    IN  GT_U8                                       devNum,
    OUT CPSS_DXCH_PTP_TAI_NUMBER_ENT                *taiNumberPtr,
    OUT CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_ENT    *pclkRcvrClkModePtr
);

/**
* @internal cpssDxChPtpTaiInputClockSelectSet function
* @endinternal
*
* @brief   Configures input TAI clock's selection.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Caelum; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) parameter ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.)
* @param[in] clockSelect              - input clock selection mode.
* @param[in] clockFrequency           - Frequency of the PTP PLL clock (default is 25).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiInputClockSelectSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT        taiNumber,
    IN  CPSS_DXCH_PTP_TAI_CLOCK_SELECT_ENT  clockSelect,
    IN  CPSS_DXCH_PTP_TAI_CLOCK_FREQ_ENT    clockFrequency
);

/**
* @internal cpssDxChPtpTaiInputClockSelectGet function
* @endinternal
*
* @brief   Get input TAI clock's selection.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Caelum; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] taiNumber                - TAI number.
*                                      For devices that have single TAI unit (Caelum, Aldrin, AC3X) parameter ignored.
*                                      (APPLICABLE DEVICES Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] clockSelectPtr           - (pointer to) input clock selection mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiInputClockSelectGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT        taiNumber,
    OUT CPSS_DXCH_PTP_TAI_CLOCK_SELECT_ENT  *clockSelectPtr
);

/**
* @internal cpssDxChPtpTaiTodCounterFunctionAllTriggerSet function
* @endinternal
*
* @brief   Triggeres TOD update operation for all TAI units.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiTodCounterFunctionAllTriggerSet
(
    IN GT_U8                    devNum
);

/**
* @internal cpssDxChPtpPortUnitResetSet function
* @endinternal
*
* @brief   Reset/unreset Port PTP unit.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] resetTxUnit              - GT_TRUE - reset PTP trasmit unit,
*                                      GT_FALSE - unreset PTP trasmit unit
*                                      For Bobcat2 related to both transmit and receive units
* @param[in] resetRxUnit              - GT_TRUE - reset PTP receive unit
*                                      GT_FALSE - unreset PTP receive unit
*                                      (APPLICABLE DEVICES Caelum; Aldrin; AC3X; Bobcat3; Aldrin2)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpPortUnitResetSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         resetTxUnit,
    IN  GT_BOOL                         resetRxUnit
);

/**
* @internal cpssDxChPtpPortUnitResetGet function
* @endinternal
*
* @brief   Get Reset/unreset Port PTP unit state.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] resetTxUnitPtr           - (pointer to)
*                                      GT_TRUE - reset PTP trasmit unit,
*                                      GT_FALSE - unreset PTP trasmit unit
*                                      For Bobcat2 related to both transmit and receive units
* @param[out] resetRxUnitPtr           - (pointer to)
*                                      GT_TRUE - reset PTP receive unit
*                                      GT_FALSE - unreset PTP receive unit
*                                      (APPLICABLE DEVICES Caelum; Aldrin; AC3X; Bobcat3; Aldrin2)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpPortUnitResetGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_BOOL                         *resetTxUnitPtr,
    OUT GT_BOOL                         *resetRxUnitPtr
);

/**
* @internal cpssDxChPtpPortTxPipeStatusDelaySet function
* @endinternal
*
* @brief   Set PTP Tx Pipe Status Delay.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] delay                    -  of empty egress pipe (in core clock cycles).
*                                      (APPLICABLE RANGE: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range parameter value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpPortTxPipeStatusDelaySet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          delay
);

/**
* @internal cpssDxChPtpPortTxPipeStatusDelayGet function
* @endinternal
*
* @brief   Get PTP Tx Pipe Status Delay.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] delayPtr                 - (pointer to)delay of empty egress pipe (in core clock cycles).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpPortTxPipeStatusDelayGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_U32                          *delayPtr
);

/**
* @internal cpssDxChPtpTaiPulseInModeSet function
* @endinternal
*
* @brief  Set TAI PTP pulse mode.
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Ironman.
*
* @param[in] devNum                - device number.
* @param[in] pulseMode             - TAI PulseIN signal mode
* @param[in] taiNumber             - TAI number selection.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTaiPulseInModeSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT            taiNumber,
    IN  CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT     pulseMode
);

/**
* @internal cpssDxChPtpTaiPulseInModeGet function
* @endinternal
*
* @brief  Get TAI PTP pulse mode.
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Ironman.
*
* @param[in] devNum                - device number.
* @param[in] taiNumber             - TAI number selection.
* @param[OUT] pulseModePtr         - (pointer to) TAI PulseIN signal mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTaiPulseInModeGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT            taiNumber,
    OUT CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT     *pulseModePtr
);

/**
* @internal cpssDxChPtpTsuTSFrameCounterControlSet function
* @endinternal
*
* @brief  Set the Time Stamping Frame Counter Configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - port number.
* @param[in] index                 - port counter index.
*                                    Falcon: APPLICABLE RANGE: 0..3.
*                                    AC5P,Harrier: APPLICABLE RANGE: 0..31.
*                                    AC5X,Ironman: APPLICABLE RANGE: 0..63.)
* @param[in] frameCounterConfigPtr - (pointer to) frame counter Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTsuTSFrameCounterControlSet
(

    IN GT_U8                                            devNum,
    IN GT_PHYSICAL_PORT_NUM                             portNum,
    IN GT_U32                                           index,
    IN CPSS_DXCH_PTP_TSU_FRAME_COUNTER_CFG_ENTRY_STC   *frameCounterConfigPtr
);

/**
* @internal cpssDxChPtpTsuTSFrameCounterControlGet function
* @endinternal
*
* @brief  Get the Time Stamping Frame Counter Configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - port number.
* @param[in] index                 - port counter index.
*                                    Falcon: APPLICABLE RANGE: 0..3.
*                                    AC5P,Harrier: APPLICABLE RANGE: 0..31.
*                                    AC5X,Ironman: APPLICABLE RANGE: 0..63.)
* @param[OUT] frameCounterConfigPtr- (pointer to) frame counter Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTsuTSFrameCounterControlGet
(
    IN  GT_U8                                            devNum,
    IN  GT_PHYSICAL_PORT_NUM                             portNum,
    IN  GT_U32                                           index,
    OUT CPSS_DXCH_PTP_TSU_FRAME_COUNTER_CFG_ENTRY_STC   *frameCounterConfigPtr
);


/**
* @internal cpssDxChPtpTsuTsFrameCounterIndexSet function
* @endinternal
*
* @brief  Set the TSU Time Stamping Frame Counter value.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - port number.
* @param[in] index                 - port counter index.
*                                    Falcon: APPLICABLE RANGE: 0..3.
*                                    AC5P,Harrier: APPLICABLE RANGE: 0..31.
*                                    AC5X,Ironman: APPLICABLE RANGE: 0..63.)
* @param[in] frameCounterPtr      - Time Frame Counter Value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTsuTsFrameCounterIndexSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U32                                  index,
    IN  GT_U32                                  frameCounter
);

/**
* @internal cpssDxChPtpTsuTimeStampMaskingProfileSet function
* @endinternal
*
* @brief  Set the Timestamp Masking Profile Configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - port number.
* @param[in] index                 - Masking Profile index.
*                                    (APPLICABLE RANGES: 1..7.)
* @param[in] maskingProfilePtr     - (pointer to) Timestamp Masking Profile Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTsuTimeStampMaskingProfileSet
(

    IN GT_U8                                            devNum,
    IN GT_PHYSICAL_PORT_NUM                             portNum,
    IN GT_U32                                           index,
    IN CPSS_DXCH_PTP_TSU_TIMESTAMP_MASKING_PROFILE_STC  *maskingProfilePtr
);

/**
* @internal cpssDxChPtpTsuTimeStampMaskingProfileGet function
* @endinternal
*
* @brief  Get the Timestamp Masking profile Configuration.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - port number.
* @param[in] index                 - Masking Profile index.
*                                    (APPLICABLE RANGES: 1..7.)
* @param[OUT] maskingProfilePtr     - (pointer to) Timestamp Masking Profile Configuration.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTsuTimeStampMaskingProfileGet
(
    IN  GT_U8                                            devNum,
    IN  GT_PHYSICAL_PORT_NUM                             portNum,
    IN  GT_U32                                           index,
    OUT CPSS_DXCH_PTP_TSU_TIMESTAMP_MASKING_PROFILE_STC *maskingProfilePtr
);

/**
* @internal cpssDxChPtpTaiClockReceptionEdgeSet function
* @endinternal
*
* @brief  Set the Clock Reception Edge.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] taiIdPtr              - (pointer to) TAI Units identification.
* @param[in] edge                  - Rising edge time of reception clock, in nanoseconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTaiClockReceptionEdgeSet
(
    IN GT_U8                                   devNum,
    IN CPSS_DXCH_PTP_TAI_ID_STC               *taiIdPtr,
    IN GT_U32                                  edge
);

/**
* @internal cpssDxChPtpTaiClockReceptionEdgeGet function
* @endinternal
*
* @brief  Get the Clock Reception Edge.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] taiIdPtr              - (pointer to) TAI Units identification.
* @param[OUT] edgePtr               - (pointer to) Rising edge time of reception clock, in nanoseconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTaiClockReceptionEdgeGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC               *taiIdPtr,
    OUT GT_U32                                 *edgePtr
);

/**
* @internal cpssDxChPtpTaiClockReceptionModeSet function
* @endinternal
*
* @brief  Set the Clock Reception Mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] taiIdPtr              - (pointer to) TAI Units identification.
* @param[in] clockReceptionMode    - Clock Reception Mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTaiClockReceptionModeSet
(
    IN GT_U8                                         devNum,
    IN CPSS_DXCH_PTP_TAI_ID_STC                     *taiIdPtr,
    IN CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_ENT    clockReceptionMode
);

/**
* @internal cpssDxChPtpTaiClockReceptionModeGet function
* @endinternal
*
* @brief  Get the Clock Reception Mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] taiIdPtr              - (pointer to) TAI Units identification.
* @param[OUT] clockReceptionModePtr- (pointer to) Clock Reception Mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTaiClockReceptionModeGet
(
    IN  GT_U8                                         devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                     *taiIdPtr,
    OUT CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_ENT   *clockReceptionModePtr
);

/**
* @internal cpssDxChPtpTaiClockGenerationEdgeSet function
* @endinternal
*
* @brief  Set the Clock Generation Edge.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] taiIdPtr              - (pointer to) TAI Units identification.
* @param[in] edge                  - Rising edge time of generation clock, in nanoseconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTaiClockGenerationEdgeSet
(

    IN GT_U8                                   devNum,
    IN CPSS_DXCH_PTP_TAI_ID_STC               *taiIdPtr,
    IN GT_U32                                  edge
);

/**
* @internal cpssDxChPtpTaiClockGenerationEdgeGet function
* @endinternal
*
* @brief  Get the Clock Generation Edge.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - device number.
* @param[in] taiIdPtr               - (pointer to) TAI Units identification.
* @param[OUT] edgePtr               - (pointer to) Rising edge time of generation clock, in nanoseconds.
*
* @retval GT_OK                     - on success
* @retval GT_FAIL                   - on error
* @retval GT_HW_ERROR               - on hardware error
* @retval GT_BAD_PARAM              - on wrong parameter
* @retval GT_BAD_PTR                - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTaiClockGenerationEdgeGet
(

    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC               *taiIdPtr,
    OUT GT_U32                                 *edgePtr
);

/**
* @internal cpssDxChPtpTaiClockGenerationModeSet function
* @endinternal
*
* @brief  Set the Clock Generation Mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] taiIdPtr              - (pointer to) TAI Units identification.
* @param[in] clockGenerationMode   - Clock Generation Mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTaiClockGenerationModeSet
(

    IN GT_U8                                   devNum,
    IN CPSS_DXCH_PTP_TAI_ID_STC               *taiIdPtr,
    IN CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_ENT    clockGenerationMode
);

/**
* @internal cpssDxChPtpTaiClockGenerationModeGet function
* @endinternal
*
* @brief  Get the Clock Generation Mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                 - device number.
* @param[in] taiIdPtr               - (pointer to) TAI Units identification.
* @param[OUT] clockGenerationModePtr- (pointer to) Clock Genartion Mode.
*
* @retval GT_OK                     - on success
* @retval GT_FAIL                   - on error
* @retval GT_HW_ERROR               - on hardware error
* @retval GT_BAD_PARAM              - on wrong parameter
* @retval GT_BAD_PTR                - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTaiClockGenerationModeGet
(

    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC               *taiIdPtr,
    OUT CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_ENT   *clockGenerationModePtr
);

/**
* @internal cpssDxChPtpTaiPClkSelectionModeSet function
* @endinternal
*
* @brief  Set the PClock Selection Mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] taiIdPtr              - (pointer to) TAI Units identification.
* @param[in] pClkSelMode           - PClk Selection Mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTaiPClkSelectionModeSet
(

    IN GT_U8                                   devNum,
    IN CPSS_DXCH_PTP_TAI_ID_STC               *taiIdPtr,
    IN CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_ENT    pClkSelMode
);

/**
* @internal cpssDxChPtpTaiPClkSelectionModeGet function
* @endinternal
*
* @brief  Get the PClock Selection Mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number.
* @param[in] taiIdPtr              - (pointer to) TAI Units identification.
* @param[OUT] pClkSelModePtr       - (pointer to) PClk Selection Mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTaiPClkSelectionModeGet
(

    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC               *taiIdPtr,
    OUT CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_ENT    *pClkSelModePtr
);

/**
* @internal cpssDxChPtpTaiCapturePreviousModeEnableSet function
* @endinternal
*
* @brief  Set TAI (Time Application Interface) Capture Previous Mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
* @param[in] enable                   - Capture Previous Mode Enable.
*                                       GT_TRUE  - Every TOD Operation will also trigger the capture operation.
*                                       GT_FALSE - Disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiCapturePreviousModeEnableSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    IN  GT_BOOL                                 enable
);

/**
* @internal cpssDxChPtpTaiCapturePreviousModeEnableGet function
* @endinternal
*
* @brief  Get TAI (Time Application Interface) Capture Previous Mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
* @param[OUT] enable                  - (pointer to)Capture Previous Mode Enable.
*                                       GT_TRUE  - Every TOD Operation will also trigger the capture operation.
*                                       GT_FALSE - Disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiCapturePreviousModeEnableGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                *taiIdPtr,
    OUT GT_BOOL                                 *enablePtr
);

/**
* @internal cpssDxChPtpUBitModeSet function
* @endinternal
*
* @brief  Set U-bit Mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
* @param[OUT] uBitMode                - U-bit mode to set.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpUBitModeSet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                 *taiIdPtr,
    IN  CPSS_DXCH_PTP_U_BIT_MODE_ENT             uBitMode
);

/**
* @internal cpssDxChPtpUBitModeGet function
* @endinternal
*
* @brief  Get U-bit Mode.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] taiIdPtr                 - (pointer to) TAI Units identification.
* @param[OUT] uBitModePtr                  - (pointer to)Capture U-bit mode.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpUBitModeGet
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_PTP_TAI_ID_STC                 *taiIdPtr,
    OUT  CPSS_DXCH_PTP_U_BIT_MODE_ENT            *uBitModePtr
);

/**
* @internal cpssDxChPtpPLLBypassEnableSet function
* @endinternal
*
* @brief   Enable/Disable PLL bypass.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] direction                - Tx, Rx or both directions
* @param[in] enable                   - GT_TRUE - PLL is in bypass mode.
*                                      GT_FALSE - PLL in normal mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS cpssDxChPtpPLLBypassEnableSet
(
    IN  GT_U8           devNum,
    IN  GT_BOOL         enable
);

/**
* @internal cpssDxChPtpPLLBypassEnableGet function
* @endinternal
*
* @brief   Get PLL bypass status (enable/disable).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] enablePtr                - (Pointer to) GT_TRUE - PLL is in bypass mode.
*                                                    GT_FALSE - PLL in normal mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpPLLBypassEnableGet
(
    IN  GT_U8           devNum,
    OUT GT_BOOL         *enablePtr
);

/**
* @internal cpssDxChPtpTsuBypassEnableSet function
* @endinternal
*
* @brief   Set TSU (Timestamping Unit) bypass enable.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] tsuBypassEnable          - Timestamping unit bypass enable.
*                                       GT_TRUE:  the TSU is bypassed, allowing
*                                                 egress traffic with reduced latency.
*                                       GT_FALSE: all egress traffic passes through
*                                                 CTSU pipeline, even if not processed.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsuBypassEnableSet
(
    IN  GT_U8                             devNum,
    IN  GT_BOOL                           tsuBypassEnable
);

/**
* @internal cpssDxChPtpTsuBypassEnableGet function
* @endinternal
*
* @brief   Get TSU (Timestamping Unit) bypass (enable/disable) state.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[out] tsuBypassEnablePtr      - (pointer to) Timestamping unit bypass enable.
*                                       GT_TRUE:  the TSU is bypassed, allowing
*                                                 egress traffic with reduced latency.
*                                       GT_FALSE: all egress traffic passes through
*                                                 TSU pipeline, even if not processed.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsuBypassEnableGet
(
    IN  GT_U8                             devNum,
    OUT GT_BOOL                           *tsuBypassEnablePtr
);

 /**
* @internal cpssDxChPtpTsuEnableSet function
* @endinternal
*
* @brief   Set TSU (Time Stamp Unit) enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] tsuEnable               - Timestamp unit enable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsuEnableSet
(
    IN  GT_U8                             devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    IN  GT_BOOL                           tsuEnable
);


/**
* @internal cpssDxChPtpTsuEnableGet function
* @endinternal
*
* @brief   Get TSU (Time Stamp Unit) enable.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[out] tsuEnablePtr            - (pointer to) timestamp unit enable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTsuEnableGet
(
    IN  GT_U8                             devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    OUT GT_BOOL                           *tsuEnablePtr
);


/**
* @internal cpssDxChPtpRxTaiSelectSet function
* @endinternal
*
* @brief   Set RX TAI select.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] rxTaiSelect              - select TAI for ingress time stamping (APPLICABLE RANGES: 0..1.).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpRxTaiSelectSet
(
    IN  GT_U8                             devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    IN  GT_U32                            rxTaiSelect
);

/**
* @internal cpssDxChPtpRxTaiSelectGet function
* @endinternal
*
* @brief   Get RX TAI select.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] rxTaiSelectPtr           - (pointer to) rx TAI select.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpRxTaiSelectGet
(
    IN  GT_U8                             devNum,
    IN  GT_PHYSICAL_PORT_NUM              portNum,
    OUT  GT_U32                           *rxTaiSelectPtr
);

/**
* @internal cpssDxChPtpRefClockSourceSelectSet function
* @endinternal
*
* @brief   Configures reference clock source of PTP PLL.
*
* @note   APPLICABLE DEVICES:      AC5P.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] refClockSelect           - reference clock source selection mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpRefClockSourceSelectSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_PTP_REF_CLOCK_SELECT_ENT  refClockSelect
);

/**
* @internal cpssDxChPtpRefClockSourceSelectGet function
* @endinternal
*
* @brief   Get reference clock source of PTP PLL.
*
* @note   APPLICABLE DEVICES:      AC5P.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[out] refClockSelectPtr       - (pointer to) reference clock source selection mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - on bad value found in HW
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpRefClockSourceSelectGet
(
    IN  GT_U8                               devNum,
    OUT CPSS_DXCH_PTP_REF_CLOCK_SELECT_ENT  *refClockSelectPtr
);

/**
* @internal cpssDxChPtpPortTaiSerRxFrameLatencyDbgGet function
* @endinternal
*
* @brief   Debug API: Get SER-TX to SER-RX latency per
*          {port,taiNumber}.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] taiNumber                - PTP TAI number.
* @param[out] nanoSecondsPtr          - (pointer to) nano seconds part of SER-RX frame latency.
* @param[out] fracNanoSecondsPtr      - (pointer to) frac nano seconds part of SER-RX frame latency.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiSerRxFrameLatencyDbgGet
(
  IN  GT_U8                             devNum,
  IN  GT_PHYSICAL_PORT_NUM              portNum,
  IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT      taiNumber,
  OUT GT_U32                            *nanoSecondsPtr,
  OUT GT_U32                            *fracNanoSecondsPtr
);

/**
* @internal cpssDxChPtpTaiTodStepDbgSet function
* @endinternal
*
* @brief   Debug API: Set TAI (Time Application Interface) TOD Step
*          per {port,taiNumber}.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] taiNumber                - PTP TAI number.
* @param[in] todStepPtr               - (pointer to) TOD Step.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiTodStepDbgSet
(
  IN  GT_U8                             devNum,
  IN  GT_PHYSICAL_PORT_NUM              portNum,
  IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT      taiNumber,
  IN  CPSS_DXCH_PTP_TAI_TOD_STEP_STC    *todStepPtr
);

/**
* @internal cpssDxChPtpNonPtpPacketTaiSelectSet function
* @endinternal
*
* @brief   Set TAI select in case packet is not a PTP packet.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - physical device number
* @param[in] taiSelect                - select TAI for timestamping for non PTP packets
*                                       (APPLICABLE RANGES: 0..4.).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpNonPtpPacketTaiSelectSet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            taiSelect
);

/**
* @internal cpssDxChPtpNonPtpPacketTaiSelectGet function
* @endinternal
*
* @brief   Get TAI select in case packet is not a PTP packet.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - physical device number
* @param[out] taiSelectPtr            - (pointer to) select TAI for timestamping for non PTP packets
*                                       (APPLICABLE RANGES: 0..4.).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpNonPtpPacketTaiSelectGet
(
    IN  GT_U8                             devNum,
    OUT  GT_U32                           *taiSelectPtr
);

/**
* @internal cpssDxChPtpTaiSelectDbgSet function
* @endinternal
*
* @brief   Set TAI select in unit CTSU/ERMRK.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress port number.
* @param[in] taiUnit                  - select TAI unit timestamping.
* @param[in] taiSelect                - select TAI for timestamping
*                                       for unit CTSU (APPLICABLE RANGES: 0..3.).
*                                       for unit ERMRK (APPLICABLE RANGES: 0..1.).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiSelectDbgSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_PTP_TAI_SELECT_UNIT_ENT   taiUnit,
    IN  GT_U32                              taiSelect
);

/**
* @internal cpssDxChPtpTaiOutputInterfaceSet function
* @endinternal
*
* @brief   Set TAI Output interface configuration.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number.
* @param[in] pinId                    - pin identification.
*                                       (APPLICABLE VALUES: A,B,C).
* @param[in] interfaceType            - interface type.
* @param[in] taiSelect                - select TAI for driving the output signal.
*                                       (APPLICABLE RANGES: 0..4.).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTaiOutputInterfaceSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_PIN_SELECT_ENT        pinId,
    IN  CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_ENT    interfaceType,
    IN  GT_U32                                  taiSelect
);

/**
* @internal cpssDxChPtpTaiOutputInterfaceGet function
* @endinternal
*
* @brief   Get TAI Output interface configuration.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number.
* @param[in] pinId                    - pin identification.
*                                       (APPLICABLE VALUES: A,B,C).
* @param[in] interfaceType            - interface type.
* @param[in] taiSelectPtr             - (pointer to) select TAI for driving the output signal.
*                                       (APPLICABLE RANGES: 0..4.).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTaiOutputInterfaceGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_PIN_SELECT_ENT        pinId,
    IN  CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_ENT    interfaceType,
    OUT GT_U32                                  *taiSelectPtr
);

/**
* @internal cpssDxChPtpTaiInputInterfaceSet function
* @endinternal
*
* @brief   Set TAI Input interface configuration.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number.
* @param[in] pinId                    - pin identification.
*                                       (APPLICABLE VALUES: A,B,C are valid when interfaceType == CLOCK/PULSE;.
*                                                           NONE is valid when interfaceType == CLOCK;
*                                                           SOFTWARE_PULSE is valid when interfaceType == PULSE).
* @param[in] interfaceType            - interface type.
*                                       (APPLICABLE VALUES: CLOCK, PULSE).
* @param[in] taiSelectBmp             - bitmap of selected TAIs for receiving the input signal.
*                                       1 - enable, 0 - disable. (APPLICABLE RANGES: 0..0x1F.).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTaiInputInterfaceSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_PIN_SELECT_ENT        pinId,
    IN  CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_ENT    interfaceType,
    IN  GT_TAI_SELECT_BMP                       taiSelectBmp
);

/**
* @internal cpssDxChPtpTaiInputInterfaceGet function
* @endinternal
*
* @brief   Get TAI Input interface configuration.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number.
* @param[in] pinId                    - pin identification.
*                                       (APPLICABLE VALUES: A,B,C are valid when interfaceType == CLOCK/PULSE;.
*                                                           NONE is valid when interfaceType == CLOCK;
*                                                           SOFTWARE_PULSE is valid when interfaceType == PULSE).
* @param[in] interfaceType            - interface type.
*                                       (APPLICABLE VALUES: CLOCK, PULSE).
* @param[out] taiSelectBmpPtr         - (pointer to) bitmap of selected TAIs for receiving the input signal.
*                                       1 - enable, 0 - disable. (APPLICABLE RANGES: 0..0x1F.).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTaiInputInterfaceGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_PIN_SELECT_ENT        pinId,
    IN  CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_ENT    interfaceType,
    OUT GT_TAI_SELECT_BMP                       *taiSelectBmpPtr
);

/**
* @internal cpssDxChPtpTaiTodCounterFunctionGroupTriggerSet function
* @endinternal
*
* @brief   Triggeres TOD update operation for selected TAI units.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number.
* @param[in] taiSelectBmp             - bitmap of selected TAIs for triggering TOD operation.
*                                       1 - enable, 0 - disable. (APPLICABLE RANGES: 0..0x1F.).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPtpTaiTodCounterFunctionGroupTriggerSet
(
    IN GT_U8                                    devNum,
    IN GT_TAI_SELECT_BMP                        taiSelectBmp
);

/**
* @internal cpssDxChPtpTaiTodCounterFunctionGroupTriggerGet function
* @endinternal
*
* @brief   Get selected TAI units for triggering TOD update operation.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number.
*
* @param[out] taiSelectBmpPtr         - (pointer to) bitmap of selected TAIs for triggering TOD operation.
*                                       1 - enable, 0 - disable. (APPLICABLE RANGES: 0..0x1F.).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPtpTaiTodCounterFunctionGroupTriggerGet
(
    IN  GT_U8                                   devNum,
    OUT GT_TAI_SELECT_BMP                       *taiSelectBmpPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPtp */
