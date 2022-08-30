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
* @file cpssPxEgress.h
*
* @brief CPSS PX implementation for egress processing.
*
* @version   1
********************************************************************************
*/

#ifndef __cpssPxEgress_h
#define __cpssPxEgress_h

#include <cpss/common/cpssTypes.h>
#include <cpss/px/cpssPxTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @struct CPSS_PX_REGULAR_DSA_FORWARD_STC
 *
 * @brief structure of regular DSA FORWARD (single word) tag parameters
*/
typedef struct{

    /** source packet received tagged. */
    GT_BOOL srcTagged;

    /** @brief HW source device.
     *  (APPLICABLE RANGE: 0..31)
     */
    GT_HW_DEV_NUM hwSrcDev;

    /** @brief source port/trunkId.
     *  (APPLICABLE RANGE: 0..31)
     */
    GT_U32 srcPortOrTrunk;

    /** indication the packet was received from trunk. */
    GT_BOOL srcIsTrunk;

    /** @brief Drop eligible indicator (was Canonical Format Indicator).
     *  (APPLICABLE RANGE: 0..1)
     */
    GT_U32 cfi;

    /** @brief user priority (up): the IEEE 802.1p class of service.
     *  (APPLICABLE RANGE: 0..7)
     */
    GT_U32 up;

    /** @brief VLAN identifier ,specifying the VLAN to which the frame belongs.
     *  (APPLICABLE RANGE: 0..4095)
     */
    GT_U32 vid;

} CPSS_PX_REGULAR_DSA_FORWARD_STC;

/**
* @struct CPSS_PX_EXTENDED_DSA_FORWARD_STC
 *
 * @brief structure of extended DSA FORWARD (two words) tag parameters
*/
typedef struct{

    /** source packet received tagged. */
    GT_BOOL srcTagged;

    /** @brief HW source device.
     *  (APPLICABLE RANGE: 0..31)
     */
    GT_HW_DEV_NUM hwSrcDev;

    /** @brief source port/trunkId.
     *  (APPLICABLE RANGE: 0..127)
     */
    GT_U32 srcPortOrTrunk;

    /** indication the packet was received from trunk. */
    GT_BOOL srcIsTrunk;

    /** @brief Drop eligible indicator (was Canonical Format Indicator).
     *  (APPLICABLE RANGE: 0..1)
     */
    GT_U32 cfi;

    /** @brief user priority (up): the IEEE 802.1p class of service.
     *  (APPLICABLE RANGE: 0..7)
     */
    GT_U32 up;

    /** @brief VLAN identifier ,specifying the VLAN to which the frame belongs.
     *  (APPLICABLE RANGE: 0..4095)
     */
    GT_U32 vid;

    /** @brief Indicates that the packet is Egress filtered as a
     *  registered packet.
     *  GT_FALSE - Packet is egress filtered as an Unregistered
     *  packet and is forwarded to an Egress port
     *  according to its type (Unicast or Multicast) and
     *  the configuration of the Egress port Port<n>
     *  UnkFilterEn if packet is Unicast, and according
     *  to the configuration of Port<n> UnregFilterEn if
     *  the packet is Multicast.
     *  GT_TRUE - Packet is egress filtered as a registered packet
     *  according to the members of the Multicast group.
     *  NOTE:
     *  When this field is 0, the type of the packet
     *  Multicast or Unicast is set according to the
     *  packet's MAC DA[40].
     */
    GT_BOOL egrFilterRegistered;

    /** @brief Drop packet on its source device indicator,
     *  for Fast Stack Fail over Recovery Support.
     *  GT_TRUE: If packet is looped (packetIsLooped = 1)and
     *  SrcDev = OwnDev then packet is dropped
     *  GT_FALSE: in other cases
     */
    GT_BOOL dropOnSource;

    /** @brief Packet is looped indicator, for Fast Stack Fail
     *  over Recovery Support.
     *  GT_FALSE: Packet is not looped and is being routed via its
     *  designated route in the Stack.
     *  GT_TRUE: Packet is looped and its being routed on an
     *  alternative route in the Stack.
     */
    GT_BOOL packetIsLooped;

    /** @brief Indicates whether the packet is routed.
     *  GT_FALSE - Packet has not be Layer 3 routed.
     *  GT_TRUE - Packet has been Layer 3 routed.
     */
    GT_BOOL wasRouted;

    /** @brief Packet's Source ID
     *  (APPLICABLE RANGE: 0..31)
     */
    GT_U32 srcId;

    /** @brief Packet's QoS Profile.
     *  (APPLICABLE RANGE: 0..127)
     */
    GT_U32 qosProfileIndex;

    /** @brief indication to use vidx or (hwTrgDev,trgPort)
     *  vidx - the target vidx represent mutli-destination domain.
     *  NOTE: relevant when useVidx == GT_TRUE
     *  (APPLICABLE RANGE: 0..4095)
     */
    GT_BOOL useVidx;

    GT_U32 trgVidx;

    /** @brief the target port
     *  NOTE: relevant when useVidx == GT_FALSE
     *  (APPLICABLE RANGE: 0..63)
     */
    GT_U32 trgPort;

    /** @brief the target HW device
     *  NOTE: relevant when useVidx == GT_FALSE
     *  (APPLICABLE RANGE: 0..31)
     */
    GT_HW_DEV_NUM hwTrgDev;

} CPSS_PX_EXTENDED_DSA_FORWARD_STC;

/**
 * @struct CPSS_PX_EDSA_FORWARD_STC
 *
 * @brief structure of eDSA FORWARD (4 words) tag parameters
*/
typedef struct{

    /** @brief source packet received tagged.
     * NOTE: Used for eDsa related threads in FORWARD eDSA tag
     */
    GT_BOOL srcTagged;

    /** @brief HW source device.
     *  (APPLICABLE RANGE: 0..1023)
     *  NOTE: Used for eDsa related threads in FORWARD eDSA tag
     */
    GT_HW_DEV_NUM hwSrcDev;

    /** @brief The Tag Protocol Identifier (TPID) index of the packet
     *  (APPLICABLE RANGE: 0..7)
     *  NOTE: Used for eDsa related threads in FORWARD eDSA tag
     */
    GT_U32  tpIdIndex;

    /** @brief Indicates if the packet was received tagged with tag1
     *  NOTE: Used for eDsa related threads in FORWARD eDSA tag
     */
    GT_BOOL tag1SrcTagged;

} CPSS_PX_EDSA_FORWARD_STC;


/**
* @enum CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT
 *
 * @brief The types of header alteration handling on the egress packet.
*/
typedef enum{

    /** @brief 802.1br : Extended Port to Upstream Port (802.1brE2U)
     *  NOTE: supports ECN and PTP related modifications.
     */
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E

    /** @brief 802.1br : Upstream Port to Extended Port (802.1brU2E)
     *  NOTE: supports ECN and PTP related modifications.
     */
    ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E

    /** @brief 802.1br : Upstream Port to Cascade Port (802.1brU2C)
     *  NOTE: supports ECN and PTP related modifications.
     */
    ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E

    /** @brief 802.1br : Cascade Port to Upstream Port (802.1brC2U)
     *  NOTE: supports ECN and PTP related modifications.
     */
    ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CASCADE_PORT_TO_UPSTREAM_PORT_E

    /** @brief 802.1br : Upstream Port to CPU Port (802.1brU2CPU)
     *  NOTE: supports ECN and PTP related modifications.
     */
    ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CPU_PORT_E

    /** @brief 802.1br : CPU Port to Upstream Port (802.1brCPU2U)
     *  NOTE: supports ECN and PTP related modifications.
     */
    ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CPU_PORT_TO_UPSTREAM_PORT_E

    /** @brief DSA : Extended Port Tagged  to Upstream Port (DSAET2U)
     *  NOTE: supports ECN and PTP related modifications.
     */

    ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E

    /** @brief DSA : Extended Port UnTagged to Upstream Port (DSAEU2U)
     *  NOTE: supports ECN and PTP related modifications.
     */
    ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E

    /** @brief DSA : Upstream Port to Extended Port     (DSAU2E)
     *  NOTE: supports ECN and PTP related modifications.
     */
    ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E

    /** @brief DSA : Upstream Port Mirroring to Extended Port(DSAMrr2E)
     *  NOTE: supports ECN and PTP related modifications.
     */
    ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_MIRRORING_TO_EXTENDED_PORT_E

    /** @brief DSA : CPU Port to Upstream Port (DSACPU2U)
     *  NOTE: supports ECN and PTP related modifications.
     */
    ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_CPU_PORT_TO_UPSTREAM_PORT_E

    /** @brief DSA : Port Extender with Channelized Flow Control (DSAQCN)
     *  send CNM message over 8B FWD DSA tag
     *  NOTE: not supports ECN or PTP related modifications.
     */
    ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_E

    /** @brief 802.1br : Upstream Port to IPL Port (802.1brU2IPL)
     *  NOTE: supports ECN and PTP related modifications.
     */
    ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_IPL_PORT_E

    /** @brief 802.1br : IPL Port to IPL Port (802.1brIPL2IPL)
     *  NOTE: supports ECN and PTP related modifications.
     */
    ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_IPL_PORT_TO_IPL_PORT_E

    /** @brief 802.1br : Extended Port to Upstream Port for Untagged Packets (802.1brE2U-Untagged)
     *  NOTE: supports ECN and PTP related modifications.
     */
    ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_UNTAGGED_E

    /** @brief 802.1br : Upstream Port to Extended M4 Port
     *  (802.1brU2E-M4)
     *  NOTE: supports ECN and PTP related modifications.
     */
    ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M4_PORT_E

    /** @brief 802.1br : Upstream Port to Extended M8 Port
     *  (802.1brU2E-M8)
     *  NOTE: supports ECN and PTP related modifications.
     */

    ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M8_PORT_E

    /** @brief discard: mark the descriptor to be dropped.
     *  NOTE: not supports ECN or PTP related modifications.
     */

    ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DROP_PACKET_E

    /** @brief EVB: Extended Port to Upstream Port.
     *  NOTE: not supports ECN or PTP related modifications.
     */

    ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_E2U_PACKET_E

    /** @brief EVB: EVB Upstream Port to Extended Port.
     *  NOTE: not supports ECN or PTP related modifications.
     */

    ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_U2E_PACKET_E

     /** @brief EVB QCN: EVB QCN packet to upstream port (EVBQCN).
     *  NOTE: supports ECN or PTP related modifications.
     */
    ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_QCN_E

     /** @brief PRE DA: Extended Port to Upstream Port (PRE_DA_E2U)
     *  NOTE: supports ECN and PTP related modifications.
     */
    ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_EXTENDED_PORT_TO_UPSTREAM_PORT_E

    /** @brief PRE DA: Upstream Port to Extended Port (PRE_DA_U2E)
     *  NOTE: supports ECN and PTP related modifications.
     */
    ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_UPSTREAM_PORT_TO_EXTENDED_PORT_E


    /** @brief generic : do no modifications on the packet.
     *  NOTE: not supports ECN or PTP related modifications.
     */
    ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DO_NOT_MODIFY_E

    /** @brief 16B DSA : Extended Port UnTagged to Upstream Port (EDSAEU2U)
     *  NOTE: supports ECN and PTP related modifications.
     */
    ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E

    /** @brief 16B DSA : Extended Port Tag1 source tagged to Upstream Port (EDSAET12U)
     *  NOTE: supports ECN and PTP related modifications.
     */
    ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAG1_TAGGED_TO_UPSTREAM_PORT_E

    /** @brief 16B DSA : Extended Port Tagged to Upstream Port (EDSAET2U)
     *  NOTE: supports ECN and PTP related modifications.
     */
    ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E

    /** @brief 16B DSA : Extended Port Double Tagged to Upstream Port (EDSAEDT2U)
     *  NOTE: supports ECN and PTP related modifications.
     */
    ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_DTAGGED_TO_UPSTREAM_PORT_E

    /** last value (not to used) */
    ,CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE___MUST_BE_LAST___E

} CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT;


/**
* @struct CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_STC
 *
 * @brief This struct defines the format of header alteration of type :
 * CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E
*/
typedef struct{

    /** 802.1br ETAG information */
    CPSS_802_1BR_ETAG_STC eTag;

    /** The TPID of the VLAN tag that is added by the thread
     *  NOTE: used only for CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_UNTAGGED_E
     */
    GT_U32         vlanTagTpid;
} CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_STC;

/**
* @struct
*         CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_MC_PORT_STC
*
* @brief This struct defines the format of header alteration of type :
* CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M8_PORT_E
* and
* CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M4_PORT_E
*/
typedef struct{

    /** @brief the PCID (802.1br Port Channel ID) of the port.
     *  (APPLICABLE RANGE: 0..4095)
     *  The fields are used by the Upstream port to Extended port
     *  for Multicast with 4 or 8 PCIDs
     */
    GT_U32 pcid[7];
}CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_MC_STC;

/**
* @struct CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_STC
 *
 * @brief This struct defines the format of header alteration of type :
 * CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E
*/
typedef struct{

    /** regular forward DSA tag */
    CPSS_PX_REGULAR_DSA_FORWARD_STC dsaForward;

} CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_STC;


/**
* @struct CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_STC
 *
 * @brief This struct defines the format of header alteration of type :
 * CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E
*/
typedef struct{

    /** regular forward DSA tag */
    CPSS_PX_REGULAR_DSA_FORWARD_STC dsaForward;

} CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_STC;


/**
* @struct CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TO_UPSTREAM_PORT_STC
 *
 *  @brief This struct defines the format of header alteration of type :
 *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E,
 *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_DTAGGED_TO_UPSTREAM_PORT_E
 *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E,
 *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAG1_TAGGED_TO_UPSTREAM_PORT_E
*/
typedef struct{

    /** forward eDSA tag */
    CPSS_PX_EDSA_FORWARD_STC eDsaForward;

} CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TO_UPSTREAM_PORT_STC;


/**
* @struct CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_STC
 *
 * @brief This struct defines the format of header alteration of type :
 * CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_E
*/
typedef struct{

    /** the SA mac address. */
    GT_ETHERADDR macSa;

    /** extended forward DSA tag */
    CPSS_PX_EXTENDED_DSA_FORWARD_STC dsaExtForward;

    /** the CNM TPID (etherType) */
    GT_U16 cnmTpid;

} CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_STC;

/**
* @struct CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_QCN_STC
 *
 * @brief This struct defines the format of header alteration of type :
 * CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_QCN_E
*/
typedef struct{

    /** the SA mac address. */
    GT_ETHERADDR macSa;


    /** the QCN TPID (etherType) */
    GT_U16 qcnTpid;

} CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_QCN_STC;

/**
* @struct CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_STC
 *
 * @brief This struct defines the format of header alteration of type :
 * CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_EXTENDED_PORT_TO_UPSTREAM_PORT_E
 * CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_UPSTREAM_PORT_TO_EXTENDED_PORT_E
*/
typedef struct{
    /** PTP message type
     *  (APPLICABLE RANGE: 0..15) */
    GT_U32 messageType;

    /** PTP version
     *  (APPLICABLE RANGE: 0..15) */
    GT_U32 ptpVersion;

    /** Domain number
     *  (APPLICABLE RANGE: 0..255) */
    GT_U32 domainNumber;

    /** UDP destination port
     *  (APPLICABLE RANGE: 0..65535) */
    GT_U32 udpDestPort;

    /** IPv4 protocol/IPv6 next header
     *  (APPLICABLE RANGE: 0..255) */
    GT_U32 ipv4ProtocolIpv6NextHeader;

} CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_PTP_DATA_STC;

/**
* @union CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT
 *
 * @brief This union selects one of the header alteration entry information formats.
 * NOTE: next not hold extra info :
 * CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E
 * CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E
 * CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CASCADE_PORT_TO_UPSTREAM_PORT_E
 * CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CPU_PORT_E
 * CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CPU_PORT_TO_UPSTREAM_PORT_E
 * CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_MIRRORING_TO_EXTENDED_PORT_E
 * CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E
 * CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_CPU_PORT_TO_UPSTREAM_PORT_E
 * CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E
 * CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_DTAGGED_TO_UPSTREAM_PORT_E
 * CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E
 * CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAG1_TAGGED_TO_UPSTREAM_PORT_E
 * CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_QCN_E
 * CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_EXTENDED_PORT_TO_UPSTREAM_PORT_E
 * CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_UPSTREAM_PORT_TO_EXTENDED_PORT_E
 * CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DO_NOT_MODIFY_E
 */

typedef union{
    /** @brief info needed   by the '802.1br
     *  relate to CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E
     */
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_STC info_802_1br_E2U;

    /** @brief info needed   by the '802.1br
     *  relate to
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M4_PORT_E
     *  and
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M8_PORT_E
     */
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_MC_STC info_802_1br_U2E_MC;

    /** @brief info needed   by the 'DSA
     *  relate to CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E
     */
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_STC info_dsa_ET2U;

    /** @brief info needed   by the 'DSA
     *  relate to CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E
     */
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_STC info_dsa_EU2U;

    /** @brief info needed  by the eDSA
     *  relate to
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_DTAGGED_TO_UPSTREAM_PORT_E
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAG1_TAGGED_TO_UPSTREAM_PORT_E
     */
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TO_UPSTREAM_PORT_STC info_edsa_E2U;

    /** @brief info needed   by the 'DSA
     *  relate to CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_E
     */
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_STC info_dsa_QCN;

    /** @brief info needed   by the EVB QCN
     *  relates to CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_QCN_E
     */
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_QCN_STC info_evb_QCN;

    /** @brief info needed by the PRE_DA_E2U/PRE_DA_U2E
     *  relate to
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_EXTENDED_PORT_TO_UPSTREAM_PORT_E
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_UPSTREAM_PORT_TO_EXTENDED_PORT_E
     */
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_PTP_DATA_STC info_pre_da_PTP;

    /** dummy field for types that no extra info needed. */
    GT_U32 notNeeded;

} CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT;


/**
* @enum CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT
 *
 * @brief The types of info that the source port entry may hold.
*/
typedef enum{

    /** @brief the source port entry information in 802.1br device.
     *  relevant to operations:
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E
     */
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_E,

    /** @brief the source port entry information in 802.1br device.
     *  relevant to operations:
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_UNTAGGED_E
     */
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_UNTAGGED_E,

    /** @brief the source port entry information in DSA tag device.
     *  relevant to operations:
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_MIRRORING_TO_EXTENDED_PORT_E
     */
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_DSA_E,

    /** @brief the source port entry is NOT used.
     *  relevant to operations:
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CASCADE_PORT_TO_UPSTREAM_PORT_E -
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CPU_PORT_E -
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CPU_PORT_TO_UPSTREAM_PORT_E -
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_CPU_PORT_TO_UPSTREAM_PORT_E
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_E
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DO_NOT_MODIFY_E
     */
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_NOT_USED_E,

    /** @brief the source port entry information in EVB device.
     *  relevant to operations:
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_E2U_PACKET_E
     */
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_EVB_E,

    /** @brief the source port entry information in eDSA tag device.
     *  relevant to operations:
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAG1_TAGGED_TO_UPSTREAM_PORT_E
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_DTAGGED_TO_UPSTREAM_PORT_E
     */
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_EDSA_E,

    /** @brief the source port entry information for pre-DA 2 byte tag device.
     *  relevant to operations:
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_EXTENDED_PORT_TO_UPSTREAM_PORT_E
     */
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_PRE_DA_E,

    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE___MUST_BE_LAST___E

} CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT;

/**
* @enum CPSS_PX_EGRESS_SOURCE_PORT_PTP_MODE_ENT
 *
 * @brief For the PTP application: The type of PTP processing on packets ingressed from this port.
*/
typedef enum{

    /** @brief TC processing
     *  NOTE: This value should be set for all source ports on a device which functions as Transparent Clock device.
     */
    CPSS_PX_EGRESS_SOURCE_PORT_PTP_MODE_TC_E    = 0

    /** @brief OC/BC Ingress processing
     *  NOTE: This value should be set for all source ports other than the CPU port on a device which functions as Ordinary Clock/Boundary Clock device.
     */
    ,CPSS_PX_EGRESS_SOURCE_PORT_PTP_MODE_OC_BC_INGRESS_E

    /** @brief OC/BC Egress processing
     *  NOTE: This value should be set only for the CPU port on a device which functions as Ordinary Clock/Boundary Clock device.
     */
    ,CPSS_PX_EGRESS_SOURCE_PORT_PTP_MODE_OC_BC_EGRESS_E

    /** @brief OC/BC Egress processing with time stamp in CF
     *  NOTE: This value should be set only for the CPU port on a device which functions as Ordinary Clock/Boundary Clock device.
     */
    ,CPSS_PX_EGRESS_SOURCE_PORT_PTP_MODE_OC_BC_EGRESS_CF_TIMESTAMP_E

    /** last value (not to be used) */
    ,CPSS_PX_EGRESS_SOURCE_PORT_PTP_MODE___MUST_BE_LAST___E

} CPSS_PX_EGRESS_SOURCE_PORT_PTP_MODE_ENT;

/**
* @struct CPSS_PX_VLAN_TAG_STC
 *
 * @brief structure of regular VLAN (IEEE 802.1Q) tag parameters
*/
typedef struct {
    /** @brief Priority code point  (PCP): the IEEE 802.1p class of service.
    *  (APPLICABLE RANGE: 0..7)
    */
    GT_U32 pcp;

    /** @brief Drop eligible indicator.
     *  (APPLICABLE RANGE: 0..1)
     */
    GT_U32 dei;

    /** @brief VLAN identifier ,specifying the VLAN to which the frame belongs.
     *  (APPLICABLE RANGE: 0..4095)
     */
    GT_U32 vid;

} CPSS_PX_VLAN_TAG_STC;

/**
* @struct CPSS_PX_EGRESS_SRC_PORT_INFO_STC
 *
 * @brief This structure selects one of the source port entry information formats.
 *
*/
typedef struct {
    /** @brief bitmap used for physical source port filtering.
     *  This field should be set for IPL ports. If the IPL is a LAG,
     *  the associated bits for all the ports in that LAG should be set.
     *  (the vector is 16 bits)
     */
    CPSS_PX_PORTS_BMP srcFilteringVector;

    /** @brief default VLAN tag values (PCP, DEI, VLAN-ID) of a port
     *  Used by the Extended port to Upstream Port
     *  for Untagged packets (E2U-Untagged) Thread
     *  to assign the VLAN tag values of the added VLAN.
     */
    CPSS_PX_VLAN_TAG_STC vlanTag;

} CPSS_PX_EGRESS_SRC_PORT_INFO_STC;

/**
* @struct CPSS_PX_EGRESS_SOURCE_PORT_PTP_STC
 *
 * @brief This struct defines the source port PTP information.
 *  used by:
 *      CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_E
 *      CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_DSA_E
 *      CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_EDSA_E
 *      CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_EVB_E
 *      CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_PRE_DA_E
 *      CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_NOT_USED_E
*/
typedef struct {

    /** @brief Processing PTP packet over MPLS.
     *  GT_TRUE     - new mode parsing of PTP packets over MPLS
     *  GT_FALSE    - legacy mode parsing of PTP packets over L2/IPv4/IPv6
     *  NOTE: Set ptpOverMplsEn = GT_FALSE for:
     *      CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M8_PORT_E
     *      CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E
     *      CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_DTAGGED_TO_UPSTREAM_PORT_E
     *      CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E
     *      CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAG1_TAGGED_TO_UPSTREAM_PORT_E
     *      CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_DSA_QCN_E
     */
    GT_BOOL     ptpOverMplsEn;

    /** @brief extra information for PTP.
     */
    CPSS_PX_EGRESS_SOURCE_PORT_PTP_MODE_ENT ptpPortMode;

} CPSS_PX_EGRESS_SOURCE_PORT_PTP_STC;

/**
* @struct CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC
 *
 * @brief This struct defines the source port entry information in 802.1br device.
 * used by: CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_E
*/
typedef struct{

    /** @brief the PCID (802.1br Port Channel ID) of the port.
     *  (APPLICABLE RANGE: 0..4095)
     */
    GT_U32 pcid;

    /** @brief Source port specific info:
     *  - Vlan tag values - used for E2U untagged related thread
     *  - Source port filtering vector - used for IPL related threads
     */
    CPSS_PX_EGRESS_SRC_PORT_INFO_STC srcPortInfo;

    /** @brief indicate that the port is an Upstream port:
     *  upstreamPort = GT_TRUE if port is Uplink,
     *  upstreamPort = GT_FALSE otherwise
     */
    GT_BOOL upstreamPort;

    /** @brief extra information for PTP.
     */
    CPSS_PX_EGRESS_SOURCE_PORT_PTP_STC ptpPortInfo;

} CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC;

/**
* @struct CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC
 *
 * @brief This struct defines the source port entry information in DSA tag device.
 * used by:
 *  CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_DSA_E
 *  CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_EDSA_E
*/
typedef struct{

    /** @brief the source port number in DSA/EDSA tag (that represent the entry).
     *  DSA:  (APPLICABLE RANGE: 0..15)
     *  EDSA: This field contains the Source ePort assigned to the packet by the ingress device.
     *        Src ePort[16:0] = {Word2[12:3], Word1[30:29], Word0[23:19]}.
     *        !!! Only first 14 bits of ePort used in EDSA related threads.
     *        (APPLICABLE RANGE: 0..16383)
     */
    GT_U32 srcPortNum;

    /** @brief extra information for PTP.
     */
    CPSS_PX_EGRESS_SOURCE_PORT_PTP_STC ptpPortInfo;

} CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC;

/**
* @struct CPSS_PX_EGRESS_SOURCE_PORT_EVB_STC
 *
 * @brief This struct defines the source port entry information in EVB device.
 * used by:
 * CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_EVB_E
*/
typedef struct{

    /** @brief VLAN identifier ,specifying the VLAN to which the frame belongs.
     *  (APPLICABLE RANGE: 0..4095)
     */
    GT_U32 vid;

    /** @brief extra information for PTP.
     */
    CPSS_PX_EGRESS_SOURCE_PORT_PTP_STC ptpPortInfo;

} CPSS_PX_EGRESS_SOURCE_PORT_EVB_STC;

/**
* @struct CPSS_PX_EGRESS_SOURCE_PORT_PRE_DA_STC
 *
 * @brief This struct defines the source port entry information in pre-DA device.
 * used by:
 * CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_PRE_DA_E
*/
typedef struct{

    /** @brief Global source physical port.
     *  (APPLICABLE RANGE: 0..255)
     */
    GT_U32      port;

    /** @brief extra information for PTP.
     */
    CPSS_PX_EGRESS_SOURCE_PORT_PTP_STC ptpPortInfo;

} CPSS_PX_EGRESS_SOURCE_PORT_PRE_DA_STC;

/**
* @union CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT
 *
 * @brief This union selects one of the egress source port entry information formats.
 *
*/
typedef union{
    CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC info_802_1br;

    CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC info_dsa;

    CPSS_PX_EGRESS_SOURCE_PORT_EVB_STC info_evb;

    CPSS_PX_EGRESS_SOURCE_PORT_PRE_DA_STC info_pre_da;

    /** @brief field for types that only PTP extra information is needed.
     *  relate to CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_NOT_USED_E
     */
    CPSS_PX_EGRESS_SOURCE_PORT_PTP_STC ptpPortInfo;

} CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT;



/**
* @enum CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT
 *
 * @brief The types of info that the target port entry may hold.
*/
typedef enum{

    /** @brief the source port entry information in 802.1br device.
     *  relevant to operations:
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CASCADE_PORT_TO_UPSTREAM_PORT_E
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CPU_PORT_E
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CPU_PORT_TO_UPSTREAM_PORT_E
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_MIRRORING_TO_EXTENDED_PORT_E
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_CPU_PORT_TO_UPSTREAM_PORT_E
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_UNTAGGED_E
     */
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_COMMON_E,

    /** @brief the target port entry information for DSA-QCN
     *  is set internally by the CPSS.
     *  relevant to operations:
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_E
     */
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_DSA_QCN_E,

    /** @brief the target port entry
     *  is set internally by the CPSS.
     *  relevant to operations:
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DO_NOT_MODIFY_E
     */
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_DO_NOT_MODIFY_E,

    /** @brief the target port entry information for EVB
     *  is set internally by the CPSS.
     *  relevant to operations:
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_E2U_PACKET_E
     *  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_QCN_E
     */
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_EVB_E,

    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE___MUST_BE_LAST___E

} CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT;

/**
* @struct CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC
 *
 * @brief This struct defines the common target port entry information.
 * used by:
 * CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E
 * CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E
 * CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E
 * CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E
 * CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E
 * CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E
 * CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_MIRRORING_TO_EXTENDED_PORT_E
 * CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_E2U_PACKET_E
 * CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_U2E_PACKET_E
 * CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_QCN_E
 */
typedef struct{

    /** @brief TPID (etherType).
     *  TPID for Upstream ports is used by the Extended port to Upstream port (802.1br-E2U)
     *  Thread to assign the E-Tag's TPID.
     *  TPID for Downstream ports is used by the Upstream port to Extended port (802.1br-U2E)
     *  Thread to check if the packet has a VLAN tag.
     *  TPID for Downstream ports is used by the Upstream port to Extended port (DSA-U2E)
     *  Thread, and by the Mirroring to Extended port (DSA-Mrr2E) Thread
     *  to assign the TPID of the VLAN tag.
     *  TPID for Upstream ports is used by the Extended port to Upstream port (EVB-E2U)
     *  Thread to assign the EVB TPID.
     */
    GT_U16 tpid;

    /** @brief the PCID (802.1br Port Channel ID) of the port.
     *  A PCID is configured for each Cascade Port, and is used by
     *  the Upstream port to Cascade Port (802.1br-U2C) Thread.
     *  The thread removes the E-Tag if the E-Tag <E-CID> field is
     *  equal to this field.
     *  (APPLICABLE RANGE: 0..4095)
     */
    GT_U32 pcid;

    /** @brief The egress delay in nanoseconds, from the timestamping
     *  point until the actual interface. (20 bits field)
     *  (APPLICABLE RANGE: 0..1048575)
     */
    GT_U32 egressDelay;

} CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC;

/**
* @union CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT
 *
 * @brief This union selects one of the egress target port entry information formats.
 *
*/

typedef union{
    /** @brief info needed for target port in common cases : 802.1br and DSA.
     *  related to :
     *  CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_COMMON_E
     */
    CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC info_common;

    /** @brief dummy field for types that no extra info needed.
     *  relate to :
     *  CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_DSA_QCN_E
     *  CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_DO_NOT_MODIFY_E
     */
    GT_U32 notNeeded;

} CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT;

/**
* @internal cpssPxEgressBypassModeSet function
* @endinternal
*
* @brief   Enable/disable bypass of the egress processing and option to disable
*         PPA clock for power saving when 'bypass' the egress processing.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] phaBypassEnable          GT_TRUE  -     bypass the programmable header alteration (PHA).
*                                      GT_FALSE - not bypass the programmable header alteration (PHA).
* @param[in] ppaClockEnable           - The PPA clock can be disabled for power saving.
*                                      The PPA clock can be disabled only when the phaBypassEnable == GT_TRUE.
*                                      GT_TRUE  - the PPA clock is enabled.
*                                      GT_FALSE - the PPA clock is disabled.(for power saving).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxEgressBypassModeSet
(
    IN GT_SW_DEV_NUM     devNum,
    IN GT_BOOL           phaBypassEnable,
    IN GT_BOOL           ppaClockEnable
);

/**
* @internal cpssPxEgressBypassModeGet function
* @endinternal
*
* @brief   Get bypass enable/disable of the egress processing and is the PPA clock
*         disable for power saving when 'bypass' the egress processing.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @param[out] phaBypassEnablePtr       - (pointer to)
*                                      GT_TRUE  -     bypass the programmable header alteration (PHA).
*                                      GT_FALSE - not bypass the programmable header alteration (PHA).
* @param[out] ppaClockEnablePtr        - (pointer to)
*                                      GT_TRUE  - the PPA clock is enabled.
*                                      GT_FALSE - the PPA clock is disabled.(for power saving).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxEgressBypassModeGet
(
    IN  GT_SW_DEV_NUM     devNum,
    OUT GT_BOOL          *phaBypassEnablePtr,
    OUT GT_BOOL          *ppaClockEnablePtr
);

/**
* @internal cpssPxEgressTimestampModeSet function
* @endinternal
*
* @brief   Set if the time of day (TOD) information used instead of source and target port
*         configuration for specific packet type.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] packetType               - the packet type (that was assigned by ingress processing).
*                                      (APPLICABLE RANGE: 0..31)
* @param[in] useTod                   - GT_TRUE  - the time of day (TOD) information instead of .
*                                      GT_FALSE - the source and target port information instead of the time of day (TOD).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong or device or packetType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxEgressTimestampModeSet
(
    IN GT_SW_DEV_NUM        devNum,
    IN CPSS_PX_PACKET_TYPE  packetType,
    IN GT_BOOL              useTod
);

/**
* @internal cpssPxEgressTimestampModeGet function
* @endinternal
*
* @brief   Get if the time of day (TOD) information used instead of source and target port
*         configuration for specific packet type.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] packetType               - the packet type (that was assigned by ingress processing).
*                                      (APPLICABLE RANGE: 0..31)
*
* @param[out] useTodPtr                - (pointer to)
*                                      GT_TRUE  - the time of day (TOD) information instead of .
*                                      GT_FALSE - the source and target port information instead of the time of day (TOD).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong or device or packetType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxEgressTimestampModeGet
(
    IN GT_SW_DEV_NUM        devNum,
    IN CPSS_PX_PACKET_TYPE  packetType,
    OUT GT_BOOL             *useTodPtr
);

/**
* @internal cpssPxEgressHeaderAlterationEntrySet function
* @endinternal
*
* @brief   Set per target port and per packet type the header alteration entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - target physical port number.
* @param[in] packetType               - the packet type (that was assigned by ingress processing).
*                                      (APPLICABLE RANGE: 0..31)
* @param[in] operationType            - the type of operation that this entry need to do.
*                                      NOTE: this is the type of the firmware thread that should handle the egress packet.
* @param[in] operationInfoPtr         - (pointer to) union of operation information related to the 'operationType'
*                                      NOTE: this is the template that is needed by the firmware thread
*                                      that should handle the egress packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or packetType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxEgressHeaderAlterationEntrySet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PX_PACKET_TYPE  packetType,
    IN CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT   operationType,
    IN CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT  *operationInfoPtr
);

/**
* @internal cpssPxEgressHeaderAlterationEntryGet function
* @endinternal
*
* @brief   Get per target port and per packet type the header alteration entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - target physical port number.
* @param[in] packetType               - the packet type (that was assigned by ingress processing).
*                                      (APPLICABLE RANGE: 0..31)
*
* @param[out] operationTypePtr         - (pointer to) the type of operation that this entry need to do.
*                                      NOTE: this is the type of the firmware thread that should handle the egress packet.
* @param[out] operationInfoPtr         - (pointer to) union of operation information related to the 'operationType'
*                                      NOTE: this is the template that is needed by the firmware thread
*                                      that should handle the egress packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or packetType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxEgressHeaderAlterationEntryGet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PX_PACKET_TYPE  packetType,
    OUT CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT   *operationTypePtr,
    OUT CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT  *operationInfoPtr
);

/**
* @internal cpssPxEgressSourcePortEntrySet function
* @endinternal
*
* @brief   Set per source port the entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - source physical port number.
* @param[in] infoType                 - the type of source port info.
* @param[in] portInfoPtr              - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or infoType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxEgressSourcePortEntrySet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT infoType,
    IN CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT      *portInfoPtr
);

/**
* @internal cpssPxEgressSourcePortEntryGet function
* @endinternal
*
* @brief   Get per source port the entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - source physical port number.
*
* @param[out] infoTypePtr              - (pointer to) the type of source port info.
* @param[out] portInfoPtr              - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the port was not initialized with valid info
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxEgressSourcePortEntryGet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT *infoTypePtr,
    OUT CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT      *portInfoPtr
);


/**
* @internal cpssPxEgressTargetPortEntrySet function
* @endinternal
*
* @brief   Set per target port the entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - target physical port number.
* @param[in] infoType                 - the type of target port info.
* @param[in] portInfoPtr              - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxEgressTargetPortEntrySet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT infoType,
    IN CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT     *portInfoPtr
);

/**
* @internal cpssPxEgressTargetPortEntryGet function
* @endinternal
*
* @brief   Get per target port the entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - target physical port number.
*
* @param[out] infoTypePtr              - (pointer to) the type of target port info.
* @param[out] portInfoPtr              - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the port was not initialized with valid info
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxEgressTargetPortEntryGet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT *infoTypePtr,
    OUT CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT      *portInfoPtr
);

/**
* @internal cpssPxEgressVlanTagStateEntrySet function
* @endinternal
*
* @brief   Sets VLAN tag state entry for target ports.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] vlanId                   - VLAN Id
*                                      (APPLICABLE RANGE: 0..4095)
* @param[in] portsTagging             - bitmap of physical ports tagged in the VLAN.
*                                      Every bit in bitmap represents VLAN tag state per target port:
*                                      0 - Egress tag state is untagged. Packet's VLAN tag is removed
*                                      1 - Egress tag state is tagged. Packet's VLAN tag is not removed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxEgressVlanTagStateEntrySet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32               vlanId,
    IN CPSS_PX_PORTS_BMP    portsTagging
);

/**
* @internal cpssPxEgressVlanTagStateEntryGet function
* @endinternal
*
* @brief   Gets VLAN tag state entry for target ports.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] vlanId                   - VLAN Id
*                                      (APPLICABLE RANGE: 0..4095)
*
* @param[out] portsTaggingPtr          - (pointer to) bitmap of physical ports tagged in the VLAN.
*                                      Every bit in bitmap represents VLAN tag state per target port:
*                                      0 - Egress tag state is untagged. Packet's VLAN tag is removed
*                                      1 - Egress tag state is tagged. Packet's VLAN tag is not removed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxEgressVlanTagStateEntryGet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_U32               vlanId,
    OUT CPSS_PX_PORTS_BMP   *portsTaggingPtr
);

/**
* @internal cpssPxEgressQcnVlanTagEntrySet function
* @endinternal
*
* @brief   Sets QCN VLAN tag entry for congested ports.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                - device number.
* @param[in] port                  - physical port.
*                                   (APPLICABLE RANGE: 0..16)
* @param[in] vlanTagPtr            - (pointer to) VLAN tag added to QCN packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxEgressQcnVlanTagEntrySet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_PX_VLAN_TAG_STC *vlanTagPtr
);

/**
* @internal cpssPxEgressQcnVlanTagEntryGet function
* @endinternal
*
* @brief   Gets QCN VLAN tag state entry for target ports.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                - device number.
* @param[in] port                  - physical port.
*                                   (APPLICABLE RANGE: 0..16)
* @param[out] vlanTagPtr            - (pointer to) VLAN tag added to QCN packet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxEgressQcnVlanTagEntryGet
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_PX_VLAN_TAG_STC *vlanTagPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPxEgress_h */


