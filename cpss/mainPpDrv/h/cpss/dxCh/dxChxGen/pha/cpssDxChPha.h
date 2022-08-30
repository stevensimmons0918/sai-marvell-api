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
* @file cpssDxChPha.h
*
* @brief CPSS declarations relate to PHA (programmable header alteration) in the
*   egress processing , that allows enhanced key technologies such as:
*   Telemetry, NSH metadata, ERSPAN, MPLS, SRv6, VXLAN, SGT, PTP, Unified SR and
*   any new tunnel/shim header that may emerge.
*
*   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
*
*   NOTEs:
*   1. GT_NOT_INITIALIZED will be return for any 'PHA' lib APIs if called before
*       cpssDxChPhaInit(...)
*       (exclude cpssDxChPhaInit(...) itself)
*   2. GT_NOT_INITIALIZED will be return for EPCL APIs trying to enable PHA processing
*       if called before cpssDxChPhaInit(...)
*
* @version   1
********************************************************************************
*/

#ifndef __cpssDxChPhah
#define __cpssDxChPhah

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>




/** Number of supported PHA firmware versions per image */
#define CPSS_DXCH_PHA_FW_NUM_OF_SUPPORTED_VERSIONS_PER_IMAGE_CNS      3

/**
 * @enum: CPSS_DXCH_PHA_FW_IMAGE_ID_ENT
 *
 * @brief: PHA firmware image ID
 *
 *  APPLICABLE DEVICES: FALCON; AC5P; AC5X; Harrier.
 */
typedef enum
{
    /** Default PHA firmware image ID
     *  APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier */
    CPSS_DXCH_PHA_FW_IMAGE_ID_DEFAULT_E,

    /** PHA firmware image ID 01
     *  APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier */
    CPSS_DXCH_PHA_FW_IMAGE_ID_01_E,

    /** PHA firmware image ID 02
     *  APPLICABLE DEVICES: Falcon */
    CPSS_DXCH_PHA_FW_IMAGE_ID_02_E,

    /** Not to be used */
    CPSS_DXCH_PHA_FW_IMAGE_ID_LAST_E
} CPSS_DXCH_PHA_FW_IMAGE_ID_ENT;


/**
 * @enum CPSS_DXCH_PHA_THREAD_TYPE_ENT
 *
 * @brief This enumeration lists all the PHA extended info types.
 *
 *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier)
*/
typedef enum{
    /** the threadId is not used
     *  APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier */
    CPSS_DXCH_PHA_THREAD_TYPE_UNUSED_E,

    /** type of thread to handle IOAM Ipv4 packets in the ingress switch in the system
     *  APPLICABLE DEVICES: Falcon  */
    CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV4_E,

    /** type of thread to handle IOAM Ipv6 packets in the ingress switch in the system
     *  APPLICABLE DEVICES: Falcon */
    CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV6_E,

    /** type of thread to handle IOAM Ipv4 packets in the transit switch in the system
     *  APPLICABLE DEVICES: Falcon */
    CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV4_E,

    /** type of thread to handle IOAM Ipv6 packets in the transit switch in the system
     *  APPLICABLE DEVICES: Falcon */
    CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV6_E,

    /** type of thread to handle "INT IOAM Mirroring" in the system
     *  APPLICABLE DEVICES: Falcon */
    CPSS_DXCH_PHA_THREAD_TYPE_INT_IOAM_MIRRORING_E,

    /** type of thread to handle "Tunnel Termination Data traffic" in the system
     *  APPLICABLE DEVICES: Falcon */
    CPSS_DXCH_PHA_THREAD_TYPE_INT_IOAM_EGRESS_SWITCH_E,

    /** type of thread to process Tunnel Start Entry and insert no Entropy Label
     *  APPLICABLE DEVICES: Falcon; Ac5p */
    CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_NO_EL_E,

    /** type of thread to process Tunnel Start Entry and insert one Entropy Label
     *  APPLICABLE DEVICES: Falcon; Ac5p */
    CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_ONE_EL_E,

    /** type of thread to process Tunnel Start Entry and inserts two Entropy Label
     *  APPLICABLE DEVICES: Falcon; Ac5p */
    CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_TWO_EL_E,

    /** type of thread to process Tunnel Start Entry and inserts three Entropy Label
     *  APPLICABLE DEVICES: Falcon; Ac5p */
    CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_THREE_EL_E,

    /** type of thread to process IPv6-UDP Tunnel Start Entry
     *  1. Copy the Tunnel SIP after SHIM label (number of Bytes depends on UDP Dst Port)
     *  2. Update the Tunnel SIP by the PHA Template Data, and UDP port with constant value(6635)
     *  APPLICABLE DEVICES: Falcon   */
    CPSS_DXCH_PHA_THREAD_TYPE_UNIFIED_SR_E,

    /** type of thread to process Classifier NSH over Ethernet in the system
     *  APPLICABLE DEVICES: Falcon   */
    CPSS_DXCH_PHA_THREAD_TYPE_CLASSIFIER_NSH_OVER_ETHERNET_E,

    /** type of thread to process Classifier NSH over VXLAN-GPE in the system
     *  APPLICABLE DEVICES: Falcon   */
    CPSS_DXCH_PHA_THREAD_TYPE_CLASSIFIER_NSH_OVER_VXLAN_GPE_E,

    /** type of thread to process SFF NSH VXLAN-GPE to Ethernet in the system
     *  APPLICABLE DEVICES: Falcon   */
    CPSS_DXCH_PHA_THREAD_TYPE_SFF_NSH_VXLAN_GPE_TO_ETHERNET_E,

    /** type of thread to process SFF NSH Ethernet to VXLAN-GPE in the system
     *  APPLICABLE DEVICES: Falcon   */
    CPSS_DXCH_PHA_THREAD_TYPE_SFF_NSH_ETHERNET_TO_VXLAN_GPE_E,

    /** type of thread to process IOAM Egress Switch IPv6 in the system
     *  APPLICABLE DEVICES: Falcon   */
    CPSS_DXCH_PHA_THREAD_TYPE_IOAM_EGRESS_SWITCH_IPV6_E,

    /** type of thread to process IPv6 SR End Node in the system
     *  APPLICABLE DEVICES: Falcon; Ac5p */
    CPSS_DXCH_PHA_THREAD_TYPE_SRV6_END_NODE_E,

    /** type of thread to process IPv6 SR Penultimate End Node in the system
     *  APPLICABLE DEVICES: Falcon; Ac5p */
    CPSS_DXCH_PHA_THREAD_TYPE_SRV6_PENULTIMATE_END_NODE_E,

    /** type of thread to process IPv6 SR Source Node 1 segment in the system
     *  APPLICABLE DEVICES: Falcon; Ac5p */
    CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_1_SEGMENT_E,

    /** type of thread to process IPv6 SR Source Node First pass 2 or 3 segments in the system
     *  APPLICABLE DEVICES: Falcon; Ac5p */
    CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_FIRST_PASS_2_3_SEGMENTS_E,

    /** type of thread to process IPv6 SR Source Node Second pass 3 segments in the system
     *  APPLICABLE DEVICES: Falcon; Ac5p */
    CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_3_SEGMENTS_E,

    /** type of thread to process IPv6 SR Source Node Second pass 2 segments in the system
     *  APPLICABLE DEVICES: Falcon; Ac5p */
    CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_2_SEGMENTS_E,

    /** Thread to process SGT(Security Group Tag) Tag
     *  Handles Network Port. Add 4 MSB of SGT TAG to the packet
     *  APPLICABLE DEVICES: AC5P; AC5X; Harrier  */
    CPSS_DXCH_PHA_THREAD_TYPE_SGT_NETWORK_ADD_MSB_E,

    /** Thread to process SGT(Security Group Tag) Tag
     *  Handles Network Port. Update 4 MSB of SGT TAG of the packet
     *  APPLICABLE DEVICES: AC5P; AC5X; Harrier  */
    CPSS_DXCH_PHA_THREAD_TYPE_SGT_NETWORK_FIX_E,

    /** Thread to process SGT(Security Group Tag) Tag
     *  Handles Network Port. Remove SGT TAG from the packet
     *  APPLICABLE DEVICES: AC5P; AC5X; Harrier  */
    CPSS_DXCH_PHA_THREAD_TYPE_SGT_NETWORK_REMOVE_E,

    /** Thread to process SGT(Security Group Tag) Tag
     *  Handles cascade port. Update eDSA tag of the packet
     *  APPLICABLE DEVICES: AC5P; AC5X; Harrier  */
    CPSS_DXCH_PHA_THREAD_TYPE_SGT_EDSA_FIX_E,

    /** Thread to process SGT(Security Group Tag) Tag
     *  Handles cascade port. Remove SGT TAG from the packet
     *  APPLICABLE DEVICES: AC5P; AC5X; Harrier  */
    CPSS_DXCH_PHA_THREAD_TYPE_SGT_EDSA_REMOVE_E,

    /** Thread to process SGT(Security Group Tag) Tag
     *  Handles tunnel ports. Update VXLAN GBP header for IPV4 packets
     *  APPLICABLE DEVICES: AC5P; AC5X; Harrier  */
    CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_FIX_IPV4_E,

    /** Thread to process SGT(Security Group Tag) Tag
     *  Handles tunnel ports. Update VXLAN GBP header for IPV6 packets
     *  APPLICABLE DEVICES: AC5P; AC5X; Harrier  */
    CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_FIX_IPV6_E,

    /** Thread to process SGT(Security Group Tag) Tag
     *  Handles tunnel ports. Remove VXLAN GBP header for IPV4 packets
     *  APPLICABLE DEVICES: AC5P; AC5X; Harrier  */
    CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_REMOVE_IPV4_E,

    /** Thread to process SGT(Security Group Tag) Tag
     *  Handles tunnel ports. Remove VXLAN GBP header for IPV6 packets
     *  APPLICABLE DEVICES: AC5P   */
    CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_REMOVE_IPV6_E,

    /** Thread to process PTP Phy 1 Step
     *  APPLICABLE DEVICES: AC5P; AC5X; Harrier   */
    CPSS_DXCH_PHA_THREAD_TYPE_PTP_PHY_1_STEP_E,

    /** type of thread to process Egress mirroring metadata in the system
     *  APPLICABLE DEVICES: AC5P; AC5X; Harrier   */
    CPSS_DXCH_PHA_THREAD_TYPE_EGRESS_MIRRORING_METADATA_E,

    /***** Threads types of Falcon ePort-based CC (Centralized Chassis)
           ERSPAN mirror over IPv4/IPv6 GRE tunnel without loopback port  *****/

    /** Thread to process Ingress/Egress Target Device Mirroring ERSPAN Type II IPv4 tunnel
     *  The Egress port of the ERSPAN packet is from LC (Line Card) port
     *  APPLICABLE DEVICES: Falcon   */
    CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_LC_IPV4_E,

    /** Thread to process Ingress/Egress Target Device Mirroring ERSPAN Type II IPv6 tunnel
     *  The Egress port of the ERSPAN packet is from LC (Line Card) port
     *  APPLICABLE DEVICES: Falcon   */
    CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_LC_IPV6_E,

    /** Thread to process Ingress/Egress Target Device Mirroring ERSPAN Type II IPv4 tunnel
     *  The Egress port of the ERSPAN packet is from Falcon direct port
     *  APPLICABLE DEVICES: Falcon   */
    CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_DIRECT_IPV4_E,

    /** Thread to process Ingress/Egress Target Device Mirroring ERSPAN Type II IPv6 tunnel
     *  The Egress port of the ERSPAN packet is from Falcon direct port
     *  APPLICABLE DEVICES: Falcon   */
    CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_DIRECT_IPV6_E,

    /** Thread to process ERSPAN Type II Source Device Mirroring
     *  APPLICABLE DEVICES: Falcon   */
    CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_SRC_DEV_E,

    /** Thread to process VXLAN GBP packet setting Source Group Policy ID in VXLAN header
       APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier */
    CPSS_DXCH_PHA_THREAD_TYPE_VXLAN_GBP_SOURCE_GROUP_POLICY_ID_E,

    /** Thread to process SRv6 Best Effort Tunnels in the system
     *  APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier   */
    CPSS_DXCH_PHA_THREAD_TYPE_SRV6_BEST_EFFORT_E,

    /** Thread to process SRv6 G-SID Tunnels Single Pass with 1 SRH container in the system
     *  APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier   */
    CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_1_CONTAINER_E,

    /** Thread to process SRv6 G-SID Tunnels First Pass with 2/3 SRH containers in the system
     *  APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier   */
    CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_FIRST_PASS_2_3_CONTAINERS_E,

    /** Thread to process SRv6 G-SID Tunnels Second Pass with 2 SRH containers in the system
     *  APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier   */
    CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_2_CONTAINERS_E,

    /** Thread to process SRv6 G-SID Tunnels Second Pass with 3 SRH containers in the system
     *  APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier   */
    CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_3_CONTAINERS_E,

    /** Thread to process SRv6 G-SID Tunnels IPv6 DIP with the next G-SID from the SRH in the system
     *  APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier   */
    CPSS_DXCH_PHA_THREAD_TYPE_SRV6_END_NODE_COC32_GSID_E,

    /** Thread to process IPv4 packets to increment TTL field and recompute header checksum field
     *  of packet header
     *  APPLICABLE DEVICES: Falcon   */
    CPSS_DXCH_PHA_THREAD_TYPE_IPV4_TTL_INCREMENT_E,

    /** Thread to process IPv6 packets to increment HOP LIMIT field of packet header
     *  APPLICABLE DEVICES: Falcon   */
    CPSS_DXCH_PHA_THREAD_TYPE_IPV6_HOP_LIMIT_INCREMENT_E,

    /** Thread to process ingress TRAP packet and trigger EREP replication by clearing outgoing Mtag command
     *  APPLICABLE DEVICES: Falcon   */
    CPSS_DXCH_PHA_THREAD_TYPE_CLEAR_OUTGOING_MTAG_COMMAND_E,

    /** PHA thread for appending SFLOW for IPv4 encapsulation */
    CPSS_DXCH_PHA_THREAD_TYPE_SFLOW_V5_IPV4_E,

    /** PHA thread for appending SFLOW for IPv6 encapsulation */
    CPSS_DXCH_PHA_THREAD_TYPE_SFLOW_V5_IPV6_E,

    /** PHA thread for SLS testing */
    CPSS_DXCH_PHA_THREAD_TYPE_SLS_E,

    /** PHA thread to handle packet drop */
    CPSS_DXCH_PHA_THREAD_TYPE_DROP_ALL_TRAFFIC_E,

    /** Thread to process ERSPAN Type II Same Device Mirroring over IPv4 tunnel
     *  APPLICABLE DEVICES: Falcon   */
    CPSS_DXCH_PHA_THREAD_TYPE_ERSPAN_TYPE_II_SAME_DEV_IPV4_E,

    /** Thread to process ERSPAN Type II Same Device Mirroring over IPv6 tunnel
     *  APPLICABLE DEVICES: Falcon   */
    CPSS_DXCH_PHA_THREAD_TYPE_ERSPAN_TYPE_II_SAME_DEV_IPV6_E,

    CPSS_DXCH_PHA_THREAD_TYPE___LAST___E  /* not to be used */
}CPSS_DXCH_PHA_THREAD_TYPE_ENT;

/**
 * @struct CPSS_DXCH_PHA_FW_IMAGE_INFO_STC
 *
 * @brief This structure holds information related to PHA firmware image version
*/
typedef struct{

    /** @brief firmware image ID */
    CPSS_DXCH_PHA_FW_IMAGE_ID_ENT  fwImageId;

    /** @brief firmware version number with following format:
              (year<<24) | (month<<16) | (version in month<<8) | patch number) */
    GT_U32  fwVersion;

    /** @brief (array of) last firmware versions that are supported in device for fwImageId
              Firmware version number is represented as in fwVersion  */
    GT_U32  fwVersionsArr[CPSS_DXCH_PHA_FW_NUM_OF_SUPPORTED_VERSIONS_PER_IMAGE_CNS];
}CPSS_DXCH_PHA_FW_IMAGE_INFO_STC;

/**
 * @struct CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_STC
 *
 * @brief This structure defines the fields for the 'template' needed for the
 *      PHA thread to handle IOAM Ipv4/ipv6 packets in the ingress switch in the system
 *
 *
 *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier)
*/
typedef struct{
    /** next are fields in the IOAM packet format */
    GT_U32  IOAM_Trace_Type;/* 16 bits value */
    GT_U32  Maximum_Length; /*  8 bits value */
    GT_U32  Flags;          /*  8 bits value */
    GT_U32  Hop_Lim;        /*  8 bits value */
    GT_U32  node_id;        /* 24 bits value */
    GT_U32  Type1;          /*  8 bits value , 'IOAM' */
    GT_U32  IOAM_HDR_len1;  /*  8 bits value */
    GT_U32  Reserved1;      /*  8 bits value */
    GT_U32  Next_Protocol1; /*  8 bits value */
    /* @brief next fields relevant only to 'e2e' option */
    GT_U32  Type2;          /*  8 bits value , 'IOAM_E2E' */
    GT_U32  IOAM_HDR_len2;  /*  8 bits value */
    GT_U32  Reserved2;      /*  8 bits value */
    GT_U32  Next_Protocol2; /*  8 bits value */
}CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_STC;

#if 0
/**
 * @struct CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_IPV4_STC
 *
 * @brief This structure defines the fields for the 'template' needed for the
 *      PHA thread to handle IOAM Ipv4 packets in the ingress switch in the system
 *
 *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier)
*/
typedef struct {} CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_IPV4_STC
#endif /* commented-out typedef above is required for CPSS XML builder */

typedef CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_STC
        CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_IPV4_STC;

#if 0
/**
 * @struct CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_IPV6_STC
 *
 * @brief This structure defines the fields for the 'template' needed for the
 *      PHA thread to handle IOAM Ipv6 packets in the ingress switch in the system
 *
 *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier)
*/
typedef struct {} CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_IPV6_STC
#endif /* commented-out typedef above is required for CPSS XML builder */

typedef CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_STC
        CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_IPV6_STC;

/**
 * @struct CPSS_DXCH_PHA_THREAD_INFO_TYPE_UNIFIED_SR_STC
 *
 * @brief This structure defines the fields for the 'template' needed for the
 *      PHA thread to handle ipv6-UDP packets in the edge switch in the system
 *
 *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier)
*/
typedef struct{
    GT_IPV6ADDR   srcAddr;       /* IPv6 SIP */
}CPSS_DXCH_PHA_THREAD_INFO_TYPE_UNIFIED_SR_STC;

/**
 * @struct CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_STC
 *
 * @brief This structure defines the fields for the 'template' needed for the
 *      PHA thread to handle IOAM Ipv4/ipv6 packets in the ingress switch in the system
 *
 *  NOTE: the 'Hop_Lim' (8 bits value) is set according to the 'ttl' in the descriptor.
 *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier)
*/
typedef struct{
    /** next are fields in the IOAM packet format */
    GT_U32  node_id;        /* 24 bits value */
}CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_STC;

#if 0
/**
 * @struct CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_IPV4_STC
 *
 * @brief This structure defines the fields for the 'template' needed for the
 *      PHA thread to handle IOAM Ipv4 packets in the transit switch in the system
 *
 *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier)
*/
typedef struct {} CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_IPV4_STC
#endif /* commented-out typedef above is required for CPSS XML builder */

typedef CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_STC
        CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_IPV4_STC;

#if 0
/**
 * @struct CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_IPV6_STC
 *
 * @brief This structure defines the fields for the 'template' needed for the
 *      PHA thread to handle IOAM Ipv6 packets in the transit switch in the system
 *
 *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier)
*/
typedef struct {} CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_IPV6_STC
#endif /* commented-out typedef above is required for CPSS XML builder */

typedef CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_STC
        CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_IPV6_STC;

/**
 * @struct CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_EGRESS_SWITCH_STC
 *
 * @brief This structure defines the fields for the 'template' needed for the
 *      PHA thread to handle IOAM ipv6 packets in the egress switch in the system
 *
 *  NOTE: the 'Hop_Lim' (8 bits value) is set according to the 'ttl' in the descriptor.
 *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier)
*/
typedef struct{
    /** next are fields in the IOAM packet format */
    GT_U32  node_id;        /* 24 bits value */
}CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_EGRESS_SWITCH_STC;

#if 0
/**
 * @struct CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_EGRESS_SWITCH_IPV6_STC
 *
 * @brief This structure defines the fields for the 'template' needed for the
 *      PHA thread to handle IOAM Ipv6 packets in the egress switch in the system
 *
 *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier)
*/
typedef struct {} CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_EGRESS_SWITCH_IPV6_STC
#endif /* commented-out typedef above is required for CPSS XML builder */

typedef CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_EGRESS_SWITCH_STC
        CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_EGRESS_SWITCH_IPV6_STC;

/**
 * @struct CPSS_DXCH_PHA_THREAD_INFO_TYPE_CLASSIFIER_NSH_OVER_VXLAN_GPE_STC
 *
 * @brief This structure defines the fields for the 'template' needed for the
 *      PHA thread to handle Classifier NSH over VXLAN-GPE packets in the system
 *
*  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier)
*/
typedef struct{
    /** @brief From standard:
        The source Node ID is an identifier indicating the source device where
        the original traffic initially entered the Service Function Chain.
        This identifier is unique within an SFC-enabled domain.
        (APPLICABLE RANGES: 0..0xFFF)
    */
    GT_U32  source_node_id;

    /** @brief reserved field:
        Available for future use
    */
    GT_U32  reserved;
}CPSS_DXCH_PHA_THREAD_INFO_TYPE_CLASSIFIER_NSH_OVER_VXLAN_GPE_STC;


/**
 * @struct CPSS_DXCH_PHA_THREAD_INFO_TYPE_SRV6_SRC_NODE_STC
 *
 * @brief This structure defines the fields for the 'template' needed for PHA
 *        SRv6 Source Node threads of type: SRv6 source node 1 segment
 *                                          SRv6 source node first pass 2 or 3 segments
 *
 *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier)
*/
typedef struct{
    GT_IPV6ADDR   srcAddr;       /* IPv6 SIP */
}CPSS_DXCH_PHA_THREAD_INFO_TYPE_SRV6_SRC_NODE_STC;

/**
 * @struct CPSS_DXCH_PHA_THREAD_INFO_TYPE_SGT_NETWORK_STC
 *
 * @brief This structure defines the fields for the 'template' needed for the
 *      PHA thread to handle SGT_ADD_MSB and SGT_FIX in the system
 *
*  (APPLICABLE DEVICES: AC5P; AC5X; Harrier)
*/
typedef struct{
    /** @brief Ether type value for the SGT Tag e.g - 0x8909 */
    GT_U16  etherType;

    /** @brief Version for the SGT Tag
        (APPLICABLE RANGES: 0..255)
    */
    GT_U8  version;

    /** @brief  Length value for the SGT Tag
        (APPLICABLE RANGES: 0..4095)
    */
    GT_U16  length;

    /** @brief  Supported option/flags
        (APPLICABLE RANGES: 0..4095)
    */
    GT_U16  optionType;
}CPSS_DXCH_PHA_THREAD_INFO_TYPE_SGT_NETWORK_STC;
/**
 * @struct CPSS_DXCH_PHA_THREAD_INFO_TYPE_SFLOW_V5_STC
 *
 * @brief This structure defines the fields for the 'template' needed for the
 *      PHA thread to handle SFLOW type5 Header fields in the system
 *
 *  (APPLICABLE DEVICES: Falcon)
 */
typedef struct {
    /**@brief the ipv4 address of the Agent*/
    GT_IPADDR sflowAgentIpAddr;

    /**@brief the sflow data format identifier
      - The most significant 20 bits correspond to the SMI Private Enterprise
      Code
      - The least significant 12 bits are a structure format number assigned
      by the enterprise. */
    GT_U32 sflowDataFormat;
}  CPSS_DXCH_PHA_THREAD_INFO_TYPE_SFLOW_V5_STC;


/**
 * @struct CPSS_DXCH_PHA_SHARED_MEMORY_ERSPAN_L2_STC
 *
 * @brief This structure defines the fields for the 'template' needed for the
 *      PHA thread to handle ERSPAN L2 Header fields in the system
 *
*  (APPLICABLE DEVICES: Falcon)
*/
typedef struct{
    /** @brief L2 Mac DA*/
    GT_ETHERADDR macDa;

    /** @brief L2 Mac SA*/
    GT_ETHERADDR macSa;

    /**@brief VLAN tag TPID
     */
    GT_U16 tpid;

    /** @brief VLAN Tag UP
     *  (APPLICABLE RANGE: 0..7)
     */
    GT_U16  up;

    /** @brief VLAN Tag CFI
     *  (APPLICABLE RANGE: 0..1)
     */
    GT_U16  cfi;

    /** @brief  VLAN ID for the VLAN Tag
     *  (APPLICABLE RANGE: 0..4095)
     */
    GT_U16  vid;
}CPSS_DXCH_PHA_SHARED_MEMORY_ERSPAN_L2_STC;

/**
 * @struct CPSS_DXCH_PHA_SHARED_MEMORY_ERSPAN_IPV4_STC
 *
 * @brief This structure defines the fields for the 'template' needed for the
 *      PHA thread to handle ERSPAN IPV4 Header fields in the system
 *
*  (APPLICABLE DEVICES: Falcon)
*/
typedef struct{
    /** @brief IPv4 DSCP
     *  (APPLICABLE RANGE: 0..63)
     */
    GT_U8 dscp;

    /** @brief IPv4 Flags
     *  (APPLICABLE RANGE: 0..7)
     */
    GT_U8 flags;

    /** @brief IPv4 TTL
     */
    GT_U8  ttl;

    /** @brief IPv4 SIP */
    GT_IPADDR sipAddr;

    /** @brief IPv4 DIP */
    GT_IPADDR dipAddr;

}CPSS_DXCH_PHA_SHARED_MEMORY_ERSPAN_IPV4_STC;

/**
 * @struct CPSS_DXCH_PHA_SHARED_MEMORY_ERSPAN_IPV6_STC
 *
 * @brief This structure defines the fields for the 'template' needed for the
 *      PHA thread to handle ERSPAN IPV6 Header fields in the system
 *
*  (APPLICABLE DEVICES: Falcon)
*/
typedef struct{
    /** @brief IPv6 Traffic Class
     */
    GT_U8  tc;

    /** @brief IPv6 Flow Label
     *  (APPLICABLE RANGE: 0..1M-1)
     */
    GT_U32  flowLabel;

    /** @brief IPv6 Hop Limit
     */
    GT_U8  hopLimit;

    /** @brief IPv6 SIP */
    GT_IPV6ADDR sipAddr;

    /** @brief IPv6 SDIP */
    GT_IPV6ADDR dipAddr;

}CPSS_DXCH_PHA_SHARED_MEMORY_ERSPAN_IPV6_STC;

/**
 * @struct CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_UNT
 *
 * @brief This union is for all the PHA shared info types.
 *
 *  (APPLICABLE DEVICES: Falcon)
*/
typedef union{
    /** @brief ERSPAN shared info type for ipv4 GRE tunnel */
    CPSS_DXCH_PHA_SHARED_MEMORY_ERSPAN_IPV4_STC ipv4;

    /** @brief ERSPAN shared info type for ipv6 GRE tunnel */
    CPSS_DXCH_PHA_SHARED_MEMORY_ERSPAN_IPV6_STC ipv6;
}CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_UNT;

/**
 * @struct CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC
 *
 * @brief This structure defines the fields for the 'template' needed for the
 *      PHA thread to handle ERSPAN L2/IP Header fields and Session ID in the system
 *
*  (APPLICABLE DEVICES: Falcon)
*/
typedef struct{

    /** @brief ERSPAN L2 shared info type */
    CPSS_DXCH_PHA_SHARED_MEMORY_ERSPAN_L2_STC l2Info;

    /** @brief type of IP stack used */
    CPSS_IP_PROTOCOL_STACK_ENT protocol;

    /** @brief union of IP stack */
    CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_UNT ipInfo;

    /** @brief ERSPAN Ingress Session ID
     *  (APPLICABLE RANGE: 0..1023)
     */
    GT_U16 ingressSessionId;

    /** @brief ERSPAN Egress Session ID
     *  (APPLICABLE RANGE: 0..1023)
     */
    GT_U16 egressSessionId;

}CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC;

/**
 * @struct CPSS_DXCH_PHA_THREAD_INFO_TYPE_VXLAN_GBP_SOURCE_GROUP_POLICY_ID_STC
 *
 * @brief This structure defines the fields for the 'template' needed for the
 *      PHA thread to set VXLAN-GBP header Group Policy ID in the system
 *
*  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier)
*/
typedef struct{
    /** @brief defines first bit of Copy Reserved those are used for Policy ID insertion
     *  (APPLICABLE RANGES: Falcon 0..17)
     *  (APPLICABLE RANGES: AC5X; Harrier; AC5P 0..18)
     */
    GT_U8  copyReservedLsb;

    /** @brief defines last bit of Copy Reserved those are used for Policy ID insertion
     *  (APPLICABLE RANGES: Falcon 0..17)
     *  (APPLICABLE RANGES: AC5X; Harrier; AC5P 0..18)
     *  Note:
     *  copyReservedMsb should be >= copyReservedLsb
     */
    GT_U8  copyReservedMsb;
}CPSS_DXCH_PHA_THREAD_INFO_TYPE_VXLAN_GBP_SOURCE_GROUP_POLICY_ID_STC;

/**
 * @struct CPSS_DXCH_PHA_THREAD_INFO_TYPE_SRV6_COC32_GSID_STC
 *
 * @brief This structure defines the fields for the 'template' needed for the
 *      PHA thread to handle ipv6 packets with SRH consisting of G-SID
 *      containers in the system
 *
 *  NOTE: the IPv6 DIP (128 bits value) is set according to the common prefix
 *  and G-SID.
 *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier)
*/
typedef struct{
    /** common prefix length in the IPv6 packet with SRH segments consisting of
     * G-SID Containers */
    GT_U8  dipCommonPrefixLength; /* 8 bits value */
}CPSS_DXCH_PHA_THREAD_INFO_TYPE_SRV6_COC32_GSID_STC;

/**
 * @struct CPSS_DXCH_PHA_THREAD_INFO_UNT
 *
 * @brief This union if for all the PHA extended info types.
 *
 *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier)
*/
typedef union{
    /** dummy field for PHA types that no extra info needed:
        CPSS_DXCH_PHA_THREAD_TYPE_UNUSED_E,
        CPSS_DXCH_PHA_THREAD_TYPE_INT_IOAM_MIRRORING_E,
        CPSS_DXCH_PHA_THREAD_TYPE_INT_IOAM_EGRESS_SWITCH_E,
        CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_NO_EL_E,
        CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_ONE_EL_E,
        CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_TWO_EL_E,
        CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_THREE_EL_E,
        CPSS_DXCH_PHA_THREAD_TYPE_CLASSIFIER_NSH_OVER_ETHERNET_E,
        CPSS_DXCH_PHA_THREAD_TYPE_SFF_NSH_VXLAN_GPE_TO_ETHERNET_E,
        CPSS_DXCH_PHA_THREAD_TYPE_SFF_NSH_ETHERNET_TO_VXLAN_GPE_E,
        CPSS_DXCH_PHA_THREAD_TYPE_SRV6_END_NODE_E,
        CPSS_DXCH_PHA_THREAD_TYPE_SRV6_PENULTIMATE_END_NODE_E,
        CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_3_SEGMENTS_E,
        CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_2_SEGMENTS_E,
        CPSS_DXCH_PHA_THREAD_TYPE_PTP_PHY_1_STEP_E,
        CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_LC_IPV4_E,
        CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_LC_IPV6_E,
        CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_DIRECT_IPV4_E,
        CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_DIRECT_IPV6_E,
        CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_SRC_DEV_E,
        CPSS_DXCH_PHA_THREAD_TYPE_SRV6_BEST_EFFORT_E,
        CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_2_CONTAINERS_E,
        CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_3_CONTAINERS_E,
        CPSS_DXCH_PHA_THREAD_TYPE_IPV4_TTL_INCREMENT_E,
        CPSS_DXCH_PHA_THREAD_TYPE_IPV6_HOP_LIMIT_INCREMENT_E,
        CPSS_DXCH_PHA_THREAD_TYPE_CLEAR_OUTGOING_MTAG_COMMAND_E,
        CPSS_DXCH_PHA_THREAD_TYPE_SLS_E */
    GT_U32 notNeeded;

    /** template for thread to handle IOAM Ipv4 packets in the ingress switch in the system */
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_IPV4_STC  ioamIngressSwitchIpv4;
    /** template for thread to handle IOAM Ipv6 packets in the ingress switch in the system */
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_IPV6_STC  ioamIngressSwitchIpv6;
    /** template for thread to handle IOAM Ipv4 packets in the transit switch in the system */
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_IPV4_STC  ioamTransitSwitchIpv4;
    /** template for thread to handle IOAM Ipv6 packets in the transit switch in the system */
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_IPV6_STC  ioamTransitSwitchIpv6;
    /** template for thread to handle IOAM Ipv6 packets in the egress switch in the system */
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_EGRESS_SWITCH_IPV6_STC  ioamEgressSwitchIpv6;
    /** template for thread to handle Ipv6-UDP packets in edge switch */
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_UNIFIED_SR_STC  unifiedSRIpv6;
    /** template for thread to handle Classifier NSH over VXLAN-GPE packets in the system */
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_CLASSIFIER_NSH_OVER_VXLAN_GPE_STC  classifierNshOverVxlanGpe;
    /** template for threads of SRv6 Source Node type:
        CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_1_SEGMENT_E,
        CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_FIRST_PASS_2_3_SEGMENTS_E
        CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_1_CONTAINER_E
        CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_FIRST_PASS_2_3_CONTAINERS_E */
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_SRV6_SRC_NODE_STC  srv6SrcNode;
    /** template for threads of SGT :
     * CPSS_DXCH_PHA_THREAD_TYPE_SGT_NETWORK_ADD_MSB_E,
     * CPSS_DXCH_PHA_THREAD_TYPE_SGT_NETWORK_FIX_E, */
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_SGT_NETWORK_STC  sgtNetwork;
    /** template for thread to handle VXLAN-GBP packets in the system */
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_VXLAN_GBP_SOURCE_GROUP_POLICY_ID_STC  vxlanGbpSourceGroupPolicyId;

    /** template for thread to handle SRv6 CoC32 G-SID based packets in the system :
     * CPSS_DXCH_PHA_THREAD_TYPE_SRV6_END_NODE_COC32_GSID_E */
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_SRV6_COC32_GSID_STC  srv6Coc32GsidCommonPrefix;

    /** template for thread to handle SFLOW mirror packets in the system */
    CPSS_DXCH_PHA_THREAD_INFO_TYPE_SFLOW_V5_STC  sflowV5Mirror;
}CPSS_DXCH_PHA_THREAD_INFO_UNT;

/**
 * @enumeration CPSS_DXCH_PHA_BUSY_STALL_MODE_ENT
 *
 * @brief This enumeration lists all the PHA Busy Stall Modes.
 * The action taken when the PHA is busy
 *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier)
 *
*/
typedef enum{
    /** Back-pressure to Control Pipe */
    CPSS_DXCH_PHA_BUSY_STALL_MODE_PUSH_BACK_E,
    /** Send to Bypass path and Drop the packet */
    CPSS_DXCH_PHA_BUSY_STALL_MODE_DROP_E,
    /** Send to PHA. PHA will not modify the packet */
    CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E
}CPSS_DXCH_PHA_BUSY_STALL_MODE_ENT;


/**
 * @enumeration CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_ENT
 *
 * @brief This enumeration for the Header Modification Violation Reason -
 *      Type describes which exact header size violation occurred.
 *
 *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier)
 *
*/
typedef enum{
    /** brief FW tries to increase header above the allowed expansion -
        accessible window size dependent. */
    CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_FW_INC_OVERSIZE_E,
    /** brief Total PHA output header exceeds 255B. */
    CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_HEADER_OVERSIZE_E,
    /** brief FW tries to decrease accessible header to a zero or negative length-
        accessible window size dependent. */
    CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_FW_DEC_VIOLATION_E,
    /** brief Total PHA output header is below 2B. */
    CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_HEADER_UNDERSIZE_E
}CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_ENT;


/**
 * @struct CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC
 *
 * @brief This structure if for the PHA common info per thread-Id.
 *
 *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier)
*/
typedef struct{
    /** @brief Statistical processing decision threshold. If 0, packet is always processed by the PHA.
     *   If 255 the packet has a 1 in 256 chance of being processed
    */
    GT_U32                                          statisticalProcessingFactor;
    /** @brief The action taken when the PHA is busy.
    */
    CPSS_DXCH_PHA_BUSY_STALL_MODE_ENT               busyStallMode;
    /** @brief Drop Code to set when the packet is dropped due to PHA overload */
    CPSS_NET_RX_CPU_CODE_ENT                        stallDropCode;
}CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC;

/**
* @struct CPSS_DXCH_PHA_ERROR_CODE_CONFIG_STC
 *
 * @brief PHA Error code configurations
*/
typedef struct{

    /** @brief Drop code to attach to packets incurring tables read SER errors */
    CPSS_NET_RX_CPU_CODE_ENT  tablesReadErrorDropCode;

    /**  @brief Drop code to attach to non-zero thread number packets
     *   arriving when PPA clock is down
     */
    CPSS_NET_RX_CPU_CODE_ENT ppaClockDownErrorDropCode;

} CPSS_DXCH_PHA_ERROR_CODE_CONFIG_STC;

/**
* @internal cpssDxChPhaInit function
* @endinternal
*
* @brief   Init the PHA unit in the device.
*
*   NOTEs:
*   1. GT_NOT_INITIALIZED will be return for any 'PHA' lib APIs if called before
*       cpssDxChPhaInit(...)
*       (exclude cpssDxChPhaInit(...) itself)
*   2. GT_NOT_INITIALIZED will be return for EPCL APIs trying to enable PHA processing
*       if called before cpssDxChPhaInit(...)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number.
* @param[in] packetOrderChangeEnable  - Enable/Disable the option for packet order
*                                      change between heavily processed flows and lightly processed flows
*                                      GT_TRUE  - packet order is not maintained
*                                      GT_FALSE  - packet order is maintained
* @param[in] phaFwImageId             - PHA firmware image ID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_ALREADY_EXIST         - the library was already initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - firmware address check fail. One of firmware threads holds bad address
*/
GT_STATUS cpssDxChPhaInit
(
    IN  GT_U8                           devNum,
    IN GT_BOOL                          packetOrderChangeEnable,
    IN CPSS_DXCH_PHA_FW_IMAGE_ID_ENT    phaFwImageId
);

/**
* @internal cpssDxChPhaFwImageIdGet function
* @endinternal
*
* @brief  Get PHA firmware image ID
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in] devNum                - device number.
* @param[out] phaFwImageIdPtr      - (pointer to) the PHA firmware image ID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaFwImageIdGet
(
    IN GT_U8                           devNum,
    OUT CPSS_DXCH_PHA_FW_IMAGE_ID_ENT  *phaFwImageIdPtr
);

/**
* @internal cpssDxChPhaThreadIdEntrySet function
* @endinternal
*
* @brief   Set the entry in the PHA Thread-Id table.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in] devNum                   - device number.
* @param[in] phaThreadId              - the thread-Id.
*                                      (APPLICABLE RANGE: 1..255)
* @param[in] commonInfoPtr            - (pointer to) the common information needed for this threadId.
* @param[in] extType                  - the type of operation that this entry need to do.
*                                      NOTE: this is the type of the firmware thread that should handle the egress packet.
* @param[in] extInfoPtr               - (pointer to) union of operation information related to the 'operationType'
*                                      NOTE: this is the template that is needed by the firmware thread
*                                      that should handle the egress packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong phaThreadId number or device or extType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaThreadIdEntrySet
(
    IN GT_U8                devNum,
    IN GT_U32               phaThreadId,
    IN CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC *commonInfoPtr,
    IN CPSS_DXCH_PHA_THREAD_TYPE_ENT    extType,
    IN CPSS_DXCH_PHA_THREAD_INFO_UNT    *extInfoPtr
);

/**
* @internal cpssDxChPhaThreadIdEntryGet function
* @endinternal
*
* @brief   Get the entry in the PHA Thread-Id table.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; .
*
* @param[in] devNum                   - device number.
* @param[in] phaThreadId              - the thread-Id.
*                                      (APPLICABLE RANGE: 1..255)
* @param[out] commonInfoPtr            - (pointer to) the common information needed for this threadId.
* @param[out] extTypePtr               - (pointer to) the type of operation that this entry need to do.
*                                      NOTE: this is the type of the firmware thread that should handle the egress packet.
* @param[out] extInfoPtr               - (pointer to) union of operation information related to the 'operationType'
*                                      NOTE: this is the template that is needed by the firmware thread
*                                      that should handle the egress packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong phaThreadId number or device or extType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaThreadIdEntryGet
(
    IN GT_U8                devNum,
    IN GT_U32               phaThreadId,
    OUT CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC *commonInfoPtr,
    OUT CPSS_DXCH_PHA_THREAD_TYPE_ENT    *extTypePtr,
    OUT CPSS_DXCH_PHA_THREAD_INFO_UNT    *extInfoPtr
);

/**
* @internal cpssDxChPhaPortThreadIdSet function
* @endinternal
*
* @brief   Per target port ,set the associated thread-Id.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - the target port number
* @param[in] enable                   - enable/disable the use of threadId for the target port.
* @param[in] phaThreadId              - the associated thread-Id
*                                       NOTE: relevant only when enable = GT_TRUE
*                                      (APPLICABLE RANGE: 1..255)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port or phaThreadId
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPhaPortThreadIdSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable,
    IN GT_U32       phaThreadId
);


/**
* @internal cpssDxChPhaPortThreadIdGet function
* @endinternal
*
* @brief   Per target port ,get the associated thread-Id.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - the target port number
* @param[in] enablePtr                - (pointer to)enable/disable the use of threadId for the target port.
* @param[in] phaThreadIdPtr           - (pointer to)the associated thread-Id
*                                       NOTE: relevant only when enable = GT_TRUE
*                                      (APPLICABLE RANGE: 1..255)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or port
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPhaPortThreadIdGet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    OUT GT_BOOL     *enablePtr,
    OUT GT_U32      *phaThreadIdPtr
);

/**
 * @struct CPSS_DXCH_PHA_PORT_ENTRY_TYPE_ERSPAN_SAME_DEV_MIRROR_STC
 *
 * @brief This structure contains the port format for the ERSPAN type II
 *        same dev mirroring use case
 *
 *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier)
*/
typedef struct{
        /**Index associated with the ERSPAN source/destination port
         *Used as index for accessing the PHA physical port table
         *APPLICABLE RANGE 0..65535*/
        GT_U32 erspanIndex;
}CPSS_DXCH_PHA_PORT_ENTRY_TYPE_ERSPAN_SAME_DEV_MIRROR_STC;

/**
 * @enum CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT
 *
 * @brief This enumeration lists all the PHA source port entry
 *        types.
 *
 *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier)
*/
typedef enum{
    CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNUSED_E,
    /** port format for ERSPAN type II same dev mirroring use case */
    CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ERSPAN_TYPE_II_E
}CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT;

/**
 * @enum CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT
 *
 * @brief This enumeration lists all the PHA target port entry
 *        types.
 *
 *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier)
*/
typedef enum{
    CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNUSED_E,
    /** port format for ERSPAN type II same dev mirroring use case */
    CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ERSPAN_TYPE_II_E
}CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT;

/**
 * @union CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT
 *
 * @brief This union contains all the PHA source port entry
 *        types correspondig to CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT
 *
 *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier)
*/
typedef union{
    /** default type. Not used*/
    GT_U32 rawFormat;
    /** port format for ERSPAN type II same dev mirroring use case */
    CPSS_DXCH_PHA_PORT_ENTRY_TYPE_ERSPAN_SAME_DEV_MIRROR_STC   erspanSameDevMirror;
}CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT;

/**
 * @union CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT
 *
 * @brief This union contains all the PHA target port entry
 *        types correspondig to CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT
 *
 *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier)
*/
typedef union{
    /** default type. Not used*/
    GT_U32 rawFormat;
    /** port format for ERSPAN type II same dev mirroring use case */
    CPSS_DXCH_PHA_PORT_ENTRY_TYPE_ERSPAN_SAME_DEV_MIRROR_STC   erspanSameDevMirror;
}CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT;

/**
* @internal cpssDxChPhaSourcePortEntrySet function
* @endinternal
*
* @brief   Set per source physical port the entry.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - source physical port number.
* @param[in] infoType              - the type of source port info.
* @param[in] portInfoPtr           - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or infoType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaSourcePortEntrySet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT  infoType,
    IN CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT  *portInfoPtr
);

/**
* @internal cpssDxChPhaSourcePortEntryGet function
* @endinternal
*
* @brief   Get per source physical port the entry.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - source physical port number.
* @param[out] infoTypePtr          - (pointer to) the type of source port info.
* @param[out] portInfoPtr          - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaSourcePortEntryGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT  *infoTypePtr,
    OUT CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT  *portInfoPtr
);



/**
* @internal cpssDxChPhaTargetPortEntrySet function
* @endinternal
*
* @brief   Set per target physical port the entry.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - target physical port number.
* @param[in] infoType              - the type of target port info.
* @param[in] portInfoPtr           - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or infoType
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaTargetPortEntrySet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT  infoType,
    IN CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT  *portInfoPtr
);


/**
* @internal cpssDxChPhaTargetPortEntryGet function
* @endinternal
*
* @brief   Get per target physical port the entry.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                - device number.
* @param[in] portNum               - target physical port number.
* @param[out] infoTypePtr          - the type of target port info.
* @param[out] portInfoPtr          - (pointer to) the entry info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaTargetPortEntryGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT  *infoTypePtr,
    OUT CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT  *portInfoPtr
);

/**
* @internal cpssDxChPhaHeaderModificationViolationInfoSet function
* @endinternal
*
* @brief   Set the packet Command and the Drop code for case of header modification
*           size violations of header pointers consistency checks
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum          - device number.
* @param[in] dropCode        - Drop code to attach to packets causing header modification
*                              size violations of header pointers consistency checks.
* @param[in] packetCommand   - Packet command to attach to packets violating header
*                              size checks or header pointer consistency checks
*                              Valid values : CPSS_PACKET_CMD_FORWARD_E /
*                                             CPSS_PACKET_CMD_DROP_HARD_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or dropCode or packetCommand
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaHeaderModificationViolationInfoSet
(
    IN  GT_U8                       devNum,
    IN  CPSS_NET_RX_CPU_CODE_ENT    dropCode,
    IN  CPSS_PACKET_CMD_ENT         packetCommand
);

/**
* @internal cpssDxChPhaHeaderModificationViolationInfoGet function
* @endinternal
*
* @brief   Get the packet Command and the Drop code for case of header modification
*           size violations of header pointers consistency checks
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum          - device number.
* @param[out] dropCodePtr    - (pointer to) Drop code to attach to packets causing header modification
*                              size violations of header pointers consistency checks.
* @param[out] packetCommandPtr - (pointer to) Packet command to attach to packets violating header
*                              size checks or header pointer consistency checks
*                              Valid values : CPSS_PACKET_CMD_FORWARD_E /
*                                             CPSS_PACKET_CMD_DROP_HARD_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaHeaderModificationViolationInfoGet
(
    IN  GT_U8                       devNum,
    OUT CPSS_NET_RX_CPU_CODE_ENT    *dropCodePtr,
    OUT CPSS_PACKET_CMD_ENT         *packetCommandPtr
);

/**
* @internal cpssDxChPhaHeaderModificationViolationCapturedGet function
* @endinternal
*
* @brief  Get the thread ID of the first header size violating packet , and the violation type.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum          - device number.
* @param[out] capturedThreadIdPtr   - (pointer to) The thread Id of the first header size violating packet
* @param[out] violationTypePtr      - (pointer to) Type describes which exact header size violation occurred
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NO_MORE               - when no more modification violations to get info about
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaHeaderModificationViolationCapturedGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                     *capturedThreadIdPtr,
    OUT CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_ENT  *violationTypePtr
);



/**
* @internal cpssDxChPhaStatisticalProcessingCounterThreadIdSet function
* @endinternal
*
* @brief   set the PHA thread-Id to count it's statistical processing cancellations.
*           When zero, all PHA threads are counted (i.e. non-zero).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] phaThreadId              - the associated thread-Id
*                                      (APPLICABLE RANGE: 0..255)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or phaThreadId
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPhaStatisticalProcessingCounterThreadIdSet
(
    IN GT_U8        devNum,
    IN GT_U32       phaThreadId
);

/**
* @internal cpssDxChPhaStatisticalProcessingCounterThreadIdGet function
* @endinternal
*
* @brief   get the PHA thread-Id to count it's statistical processing cancellations.
*           When zero, all PHA threads are counted (i.e. non-zero).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[out] phaThreadIdPtr           - (pointer to) the associated thread-Id
*                                      (APPLICABLE RANGE: 0..255)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPhaStatisticalProcessingCounterThreadIdGet
(
    IN  GT_U8        devNum,
    OUT GT_U32       *phaThreadIdPtr
);

/**
* @internal cpssDxChPhaStatisticalProcessingCounterGet function
* @endinternal
*
* @brief   Get the PHA counter value of statistical processing cancellations.
*           Note: the counter is cleared after read (by HW).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                   - device number
* @param[out] counterPtr              - (pointer)the counter
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChPhaStatisticalProcessingCounterGet
(
    IN  GT_U8        devNum,
    OUT GT_U64       *counterPtr
);

/**
* @internal cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet function
* @endinternal
*
* @brief   Set the ERSPAN entry in the PHA shared memory table.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number.
* @param[in] analyzerIndex            - analyzer index.
*                                      (APPLICABLE RANGE: 0..6)
* @param[in] entryPtr                 - (pointer to) thread template in PHA shared memory
*                                      NOTE: this is the template that is needed by the firmware thread
*                                      for IP header and Session ID for ERSPAN packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong analyzerIndex or device
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet
(
    IN GT_U8                                               devNum,
    IN GT_U32                                              analyzerIndex,
    IN CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC    *entryPtr
);

/**
* @internal cpssDxChPhaSharedMemoryErspanIpTemplateEntryGet function
* @endinternal
*
* @brief   Get the ERSPAN entry from the PHA shared memory table.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number.
* @param[in] analyzerIndex            - analyzer index.
*                                      (APPLICABLE RANGE: 0..6)
* @param[in] entryPtr                 - (pointer to) thread template in PHA shared memory
*                                      NOTE: this is the template that is needed by the firmware thread
*                                      for IP header and Session ID for ERSPAN packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong analyzerIndex or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaSharedMemoryErspanIpTemplateEntryGet
(
    IN  GT_U8                                               devNum,
    IN  GT_U32                                              analyzerIndex,
    OUT CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC    *entryPtr
);

/**
* @internal cpssDxChPhaSharedMemoryErspanGlobalConfigSet function
* @endinternal
*
* @brief   Set the ERSPAN device id and VoQ in the PHA shared memory table.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number.
* @param[in] erspanDevId              - ERSPAN device id.
*                                      (APPLICABLE RANGE: 0..7)
* @param[in] isVoQ                    - ERSPAN egress LC port working with/without VoQ.
*                                      GT_TRUE  - LC port working with VoQ
*                                      GT_FALSE - LC port working without VoQ
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaSharedMemoryErspanGlobalConfigSet
(
    IN GT_U8                                               devNum,
    IN GT_U16                                              erspanDevId,
    IN GT_BOOL                                             isVoQ
);

/**
* @internal cpssDxChPhaSharedMemoryErspanGlobalConfigGet function
* @endinternal
*
* @brief   Get the ERSPAN device id and VoQ from the PHA shared memory table.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number.
* @param[out] erspanDevIdPtr          - (pointer to) ERSPAN device id.
* @param[out] isVoQPtr                - (pointer to) ERSPAN egress LC port working with/without VoQ.
*                                      GT_TRUE  - LC port working with VoQ
*                                      GT_FALSE - LC port working without VoQ
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPhaSharedMemoryErspanGlobalConfigGet
(
    IN  GT_U8                                               devNum,
    OUT GT_U16                                              *erspanDevIdPtr,
    OUT GT_BOOL                                             *isVoQPtr
);

/**
* @internal cpssDxChPhaErrorsConfigSet function
* @endinternal
*
* @brief   Set PHA error code configurations
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin;
*                                  AC3X; Bobcat3; Aldrin2; Ironman
*
* @param[in] devNum                   - device number
* @param[in] errorCodePtr             - (pointer to) PHA error code configuration
*
* @retval GT_OK                       - on success
* @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
* @retval GT_HW_ERROR                 - on hardware error
* @retval GT_BAD_PARAM                - on wrong device or errorCode
* @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*/
GT_STATUS cpssDxChPhaErrorsConfigSet
(
    IN  GT_U8                                  devNum,
    IN  CPSS_DXCH_PHA_ERROR_CODE_CONFIG_STC    *errorCodePtr
);

/**
* @internal cpssDxChPhaErrorsConfigGet function
* @endinternal
*
* @brief   Get PHA error code configurations
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin;
*                                  AC3X; Bobcat3; Aldrin2; Ironman
*
* @param[in] devNum                   - device number
*
* @param[out] errorCodePtr            - (pointer to) PHA error code configuration
*
* @retval GT_OK                       - on success
* @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
* @retval GT_HW_ERROR                 - on hardware error
* @retval GT_BAD_PARAM                - on wrong device or errorCode
* @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
*/
GT_STATUS cpssDxChPhaErrorsConfigGet
(
    IN   GT_U8                                  devNum,
    OUT  CPSS_DXCH_PHA_ERROR_CODE_CONFIG_STC    *errorCodePtr
);

/**
* @internal cpssDxChPhaFwImageInfoGet function
* @endinternal
*
* @brief  Get information related to PHA firmware image version
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in]   devNum           - device number
* @param[out] fwImageInfoPtr    - (pointer to) firmware image information
*
* @retval GT_OK                     - on success
* @retval GT_BAD_PARAM              - on wrong device
* @retval GT_BAD_PTR                - on NULL pointer
* @retval GT_NOT_SUPPORTED          - firmware image ID that was read from PHA memory is not supported
* @retval GT_FAIL                   - on hardware error
* @retval GT_HW_ERROR               - on hardware error
* @retval GT_NOT_INITIALIZED        - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
*/
GT_STATUS cpssDxChPhaFwImageInfoGet
(
    IN  GT_U8                            devNum,
    OUT CPSS_DXCH_PHA_FW_IMAGE_INFO_STC  *fwImageInfoPtr
);

/**
* @internal cpssDxChPhaFwImageUpgrade function
* @endinternal
*
* @brief  Upgrade PHA firmware by switching current firmware version with new one
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in]  devNum                   - device number
* @param[in]  packetOrderChangeEnable  - Enable/Disable the option for packet order
*                                        change between heavily processed flows and lightly processed flows
*                                        GT_TRUE  - packet order is not maintained
*                                        GT_FALSE - packet order is maintained
* @param[out] fwImageInfoPtr           - (pointer to) firmware information of the upgraded firmware that is requested to be loaded
*
* @retval GT_OK                     - on success
* @retval GT_BAD_PARAM              - on wrong device or invalid fw version
* @retval GT_BAD_PTR                - on NULL pointer
* @retval GT_FAIL                   - on hardware error
* @retval GT_HW_ERROR               - on hardware error
* @retval GT_NOT_INITIALIZED        - the PHA library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
* @retval GT_BAD_STATE              - firmware address check fail. One of firmware threads holds bad address
*/
GT_STATUS cpssDxChPhaFwImageUpgrade
(
    IN  GT_U8                            devNum,
    IN  GT_BOOL                          packetOrderChangeEnable,
    IN  CPSS_DXCH_PHA_FW_IMAGE_INFO_STC  *fwImageInfoPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPhah */


