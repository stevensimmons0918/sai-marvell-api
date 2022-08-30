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
* @file cpssDxChPtpManagerTypes.h
*
* @brief Precision Time Protocol Manager Support - API level data type definitions
*
* @version   1
********************************************************************************
*/
#ifndef __cpssDxChPtpManagerTypes_h
#define __cpssDxChPtpManagerTypes_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/ip/cpssIpTypes.h>
#include <cpss/dxCh/dxChxGen/cpssDxChTypes.h>
#include <cpss/dxCh/dxChxGen/ptp/cpssDxChPtp.h>
#include <cpss/dxCh/dxChxGen/ptp/private/prvCpssDxChPtp.h>
#include <cpss/dxCh/dxChxGen/ptpManager/cpssDxChPtpManagerCommonTypes.h>

/**
* @struct CPSS_DXCH_PTP_MANAGER_DOMAIN_STC
*
* @brief Structure for PTP domain configuration.
*/
typedef struct{

    /** @brief domain index. (APPLICABLE RANGES: 0..3)
     *  the domain number (values up to 255 from the PTP header) is
     *  associated with an internal domain index.
     */
    GT_U32  domainIndex;

    /** @brief PTP protocol (v1 or v2).
     */
    CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_ENT  domainMode;

    /** @brief PTP V1 domain id. applicable only when
     *         domainMode=CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_PTP_V1_E
     *  PTP V1 Header contains 128-bit domain Id.
     *  4 domain Id values are mapped to domainIndex 0-3,
     *  all other domain Id values are mapped to default domain.
     */
    GT_U32  v1DomainIdArr[4];

    /** @brief PTP V2 domain id. applicable only when
     *         domainMode=CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_PTP_V2_E
     *  PTP V2 Header contains 8-bit domain Id.
     *  4 domain Id values are mapped to domainIndex 0-3,
     *  all other domain Id values are mapped to default domain.
     */
    GT_U32  v2DomainId;

} CPSS_DXCH_PTP_MANAGER_DOMAIN_STC;

/**
* @struct CPSS_DXCH_PTP_MANAGER_PTP_OVER_IP_UDP_STC
*
* @brief Structure for PTP over IP/UDP configuration.
*/
typedef struct{

    /** @brief  enable/disable PTP over IP/UDP frames identification
     */
    GT_BOOL ptpOverIpUdpEnable;

    /** @brief UDP destination port0 of PTP over UDP packets.
     *         (APPLICABLE RANGES: 0..0xFFFF)
     *         applicable only when ptpOverIpUdpEnable=GT_TRUE
     */
    GT_U32  udpPort0value;

    /** @brief UDP destination port1 of PTP over UDP packets.
     *         (APPLICABLE RANGES: 0..0xFFFF)
     *         applicable only when ptpOverIpUdpEnable=GT_TRUE
     */
    GT_U32  udpPort1value;

    /** @brief  Timestamp UDP Checksum Update Mode.
     *          applicable only when ptpOverIpUdpEnable=GT_TRUE
     */
    CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC udpCsUpdMode;

} CPSS_DXCH_PTP_MANAGER_PTP_OVER_IP_UDP_STC;

/**
* @struct CPSS_DXCH_PTP_MANAGER_PTP_OVER_ETHERNET_STC
*
* @brief Structure for PTP over Ethernet configuration.
*/
typedef struct{

    /** @brief  enable/disable PTP over Ethernet frames identification
     */
    GT_BOOL ptpOverEthernetEnable;

    /** @brief ethertype0 of PTP over Ethernet packets.
     *         (APPLICABLE RANGES: 0..0xFFFF)
     *         applicable only when ptpOverEthernetEnable=GT_TRUE
     */
    GT_U32  etherType0value;

    /** @brief ethertype1 of PTP over Ethernet packets.
     *         (APPLICABLE RANGES: 0..0xFFFF)
     *         applicable only when ptpOverEthernetEnable=GT_TRUE
     */
    GT_U32  etherType1value;

} CPSS_DXCH_PTP_MANAGER_PTP_OVER_ETHERNET_STC;

/**
* @struct CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC
*
* @brief Structure for captured TOD value.
*/
typedef struct{

    /** @brief  whether captured TOD value is valid
     */
    GT_BOOL todValueIsValid;

    /** @brief captured TOD value
     */
    CPSS_DXCH_PTP_TOD_COUNT_STC todValue;

} CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC;

/**
* @enum CPSS_DXCH_PTP_TAI_OUTPUT_INTERFACE_MODE_ENT
*
* @brief PTP output interface mode.
*/
typedef enum{

    /** @brief there is no output signal configured.
     */
    CPSS_DXCH_PTP_TAI_OUTPUT_INTERFACE_MODE_DISABLED_E,

    /** @brief PTP clock output mode.
     */
    CPSS_DXCH_PTP_TAI_OUTPUT_INTERFACE_MODE_PCLK_E,

    /** @brief PPS master output mode.
     */
    CPSS_DXCH_PTP_TAI_OUTPUT_INTERFACE_MODE_PPS_MASTER_E,

} CPSS_DXCH_PTP_TAI_OUTPUT_INTERFACE_MODE_ENT;

/**
* @struct CPSS_DXCH_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC
*
* @brief Structure for clock output interface configuration.
*/
typedef struct{

    /** @brief TAI number.
     */
    CPSS_DXCH_PTP_TAI_NUMBER_ENT    taiNumber;

    /** @brief PTP output interface mode.
     */
    CPSS_DXCH_PTP_TAI_OUTPUT_INTERFACE_MODE_ENT    outputInterfaceMode;

    /** @brief PTP clock nano seconds cycle.
     *  applicable only if outputInterfaceMode == CPSS_DXCH_PTP_TAI_OUTPUT_INTERFACE_MODE_PCLK_E
     */
    GT_U32 nanoSeconds;

} CPSS_DXCH_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC;

/**
* @enum CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT
*
* @brief PTP message type.
*/
typedef enum{

    /** Sync */
    CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E,

    /** Delay_request */
    CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_REQUEST_E,

    /** Pdelay_request */
    CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_PDELAY_REQUEST_E,

    /** Pdelay_Response */
    CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_PDELAY_RESPONSE_E,

    /** Reserved4 */
    CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_RESERVED4_E,

    /** Reserved5 */
    CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_RESERVED5_E,

    /** Reserved6 */
    CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_RESERVED6_E,

    /** Reserved7 */
    CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_RESERVED7_E,

    /** Follow_up */
    CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_FOLLOW_UP_E,

    /** Delay_response */
    CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_RESPONSE_E,

    /** Pdelay_Response_follow_up */
    CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_PDELAY_RESPONSE_FOLLOW_UP_E,

    /** Announce */
    CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ANNOUNCE_E,

    /** Signaling */
    CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SIGNALING_E,

    /** Management */
    CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_MANAGMENT_E,

    /** Reserved14 */
    CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_RESERVED14_E,

    /** Reserved15 */
    CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_RESERVED15_E,

    CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_LAST_E,

} CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT;

/* define max number for PTP message type */
#define CPSS_DXCH_PTP_MANAGER_MAX_MESSAGE_TYPE_CNS  CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_LAST_E

/**
* @enum CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_ENT
*
* @brief PTP enhanced timestamping mode.
*/
typedef enum{

    /** @brief When no enhanced timestamping mode is used, backward compatible.
     */
    CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_STANDARD_E,

    /** @brief When using Copper PHY where correction is implemented
     *         by them. The information is passed as proprietary 8B
     *         header instead of the frame preamble.
     */
    CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_USGMII_E,

    /** @brief When using encryption for PTP packets.
     *         Utilizing the "timestamp to the future" capability.
     */
    CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_MACSEC_E,

    /** @brief (Alignment Marker),used for multi-lane or when having RSFEC.
     */
    CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_AM_E

} CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_ENT;

/**
* @struct CPSS_DXCH_PTP_MANAGER_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC
*
*  @brief Structure for TSU (Time Stamping Unit) TX Timestamp Queue Entry.
*/
typedef struct{

    /** @brief whether entry is valid.
     *  GT_TRUE - Entry is valid.
     *  GT_FALSE - Queue is empty.
     */
    GT_BOOL entryValid;

    /** @brief This field is used to match between the egress timestamp
     *  queue entry and the per-port timestamp entry.
     *  The same <Queue Entry ID> is also stored in the
     *  per-port timestamp queue.
     */
    GT_U32 queueEntryId;

    /** selected TAI. (APPLICABLE RANGES: 0..1.) */
    GT_U32 taiSelect;

    /** @brief The least significant bit of the <Time Update Counter>
     *  in the TAI, at the time that <Timestamp> is captured.
     */
    GT_U32 todUpdateFlag;

     /** @brief The 2 lsbits of the seconds value of the TOD
     *  counter at the times of transmission/reception of the packet.
     *  (APPLICABLE RANGES: 0..3)
     */
    GT_U32 seconds;

    /** @brief The 30 lsbits of the nanoseconds value of the TOD
     *  counter at the times of transmission/reception of the
     *  packet. (APPLICABLE RANGES: 0..2^30-1)
     */
    GT_U32 nanoSeconds;

} CPSS_DXCH_PTP_MANAGER_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC;

/**
* @struct CPSS_DXCH_PTP_MANAGER_SIGNATURE_QUEUE_ENTRY_ID_STC
*
*  @brief Structure for MCH Signature mode Q_ENTRY_ID.
*         use the {QueueSelect [0], QueueEntryId [9:0]}, the serial index,
*         matching the one stored in the ERMRK egress timestamp queue for this packet.
*/
typedef struct{

    /** @brief queue entry identification.
     */
    GT_U32 queueEntryId;

    /** @brief queue select; 0 - queue0, 1 - queue1
     *  (APPLICABLE RANGES: 0..1.)
     */
    GT_U32 queueSelect;

}CPSS_DXCH_PTP_MANAGER_SIGNATURE_QUEUE_ENTRY_ID_STC;

/**
* @struct CPSS_DXCH_PTP_MANAGER_SIGNATURE_CTR_STC
*
*  @brief Structure for MCH Signature mode CONFIG_CTR.
*         use the counter value as the serial index, matching
*         the packet.
*/
typedef struct{

    /** @brief signature counter (14bits).
     */
    GT_U32 signatureCtr;

}CPSS_DXCH_PTP_MANAGER_SIGNATURE_CTR_STC;

/**
* @union CPSS_DXCH_PTP_MANAGER_SIGNATURE_UNT
*
* @brief Union for signature modes.
*
*/
typedef union{

    CPSS_DXCH_PTP_MANAGER_SIGNATURE_QUEUE_ENTRY_ID_STC  signatureQueueEntryId;

    CPSS_DXCH_PTP_MANAGER_SIGNATURE_CTR_STC             signatureCtr;

} CPSS_DXCH_PTP_MANAGER_SIGNATURE_UNT;

/**
* @enum CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_ENT
*
* @brief This enum defines MCH signature mode.
* (APPLICABLE DEVICES: Ironman)
*/

typedef enum{

    /** MCH Signature mode Q_ENTRY_ID */
    CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_QUEUE_ENTRY_ID_E,

    /** MCH Signature mode CONFIG_CTR */
    CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_COUNTER_E

} CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_ENT;

/**
* @struct CPSS_DXCH_PTP_MANAGER_MAC_TX_TIMESTAMP_QUEUE_ENTRY_STC
*
*  @brief Structure for MAC TX Timestamp Queue Entry.
*/
typedef struct{

    /** @brief whether entry is valid.
     *  GT_TRUE - Entry is valid.
     *  GT_FALSE - Queue is empty.
     */
    GT_BOOL entryValid;

    /** @brief This field is used to match between the timestamp stored
     *  in the MAC to the packet.
     */
    CPSS_DXCH_PTP_MANAGER_SIGNATURE_UNT signature;

     /** @brief The 2 lsbits of the seconds value of the TOD
     *  counter at the time of transmission/reception of the packet.
     *  (APPLICABLE RANGES: 0..3)
     */
    GT_U32 seconds;

    /** @brief The 30 lsbits of the nanoseconds value of the TOD
     *  counter at the time of transmission/reception of the packet.
     *  (APPLICABLE RANGES: 0..2^30-1)
     */
    GT_U32 nanoSeconds;

} CPSS_DXCH_PTP_MANAGER_MAC_TX_TIMESTAMP_QUEUE_ENTRY_STC;

/**
* @enum CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ENT
*
* @brief This enum defines PTP operation modes.
* (APPLICABLE DEVICES: Ironman)
*/
typedef enum{


    CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_STEP_IN_SWITCH_PHY_E, /* RONI TBD add description from Ayal */

    CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_STEP_IN_SWITCH_E,

    CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_TWO_STEP_PHY_MARVELL_FORMAT_E,

    CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_TWO_STEP_PHY_STANDARD_FORMAT_E,

    CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_TWO_STEP_IN_SWITCH_E,

    CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_TWO_STEP_IN_MAC_E

} CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ENT;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPtpManagerTypes_h */


