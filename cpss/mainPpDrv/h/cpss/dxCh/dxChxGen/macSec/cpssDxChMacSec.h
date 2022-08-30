
/***************************************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                                     *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.                       *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT                      *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE                            *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.                         *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,                           *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.                       *
****************************************************************************************************
*/
/**
****************************************************************************************************
* @file cpssDxChMacSec.h
*
* @brief CPSS declarations relate to MAC Security (or MACsec) feature which is a 802.1AE IEEE
*        industry-standard security technology that provides secure communication for traffic
*        on Ethernet links.
*
*   NOTEs:
*   GT_NOT_INITIALIZED will be return for any 'MACSec' APIs if called before cpssDxChMacSecInit(..)
*   (exclude cpssDxChMacSecInit(...) itself)
*
* @version   1
*****************************************************************************************************
*/

#ifndef __cpssDxChMacSech
#define __cpssDxChMacSech

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>


/**
 --------------------------------------------------------------------------------------------------------------------------------------------------
  CPSS MACSec data definitions
 --------------------------------------------------------------------------------------------------------------------------------------------------
 **/



/*************************** Constants definitions ***********************************/

/* Maximum number of control packet matching rules
   using MAC destination address and EtherType */
#define CPSS_DXCH_MACSEC_CLASSIFY_MAC_DA_ET_MATCH_RULES_COUNT_CNS        8

/* Maximum number of control packet matching rules
   using EtherType and range of MAC destination addresses */
#define CPSS_DXCH_MACSEC_CLASSIFY_MAC_DA_ET_RANGE_MATCH_RULES_COUNT_CNS  2

/* Maximum number of VLAN user priority values */
#define CPSS_DXCH_MACSEC_CLASSIFY_VLAN_UP_MAX_COUNT_CNS                  8

/* Number of used non-control key/mask words */
#define CPSS_DXCH_MACSEC_CLASSIFY_RULE_NON_CTRL_WORD_COUNT_CNS           4

/* Maximum number of active SAs */
#define CPSS_DXCH_MACSEC_SECY_MAX_ACTIVE_SA_CNS                          4

/* Size of SCI in bytes */
#define CPSS_DXCH_MACSEC_SECY_SCI_SIZE_CNS                               8

/* Size of SSCI in bytes */
#define CPSS_DXCH_MACSEC_SECY_SSCI_SIZE_CNS                              4

/* Size of SALT in bytes */
#define CPSS_DXCH_MACSEC_SECY_SALT_SIZE_CNS                              16

/* Number of ECC status counters in Classifier */
#define CPSS_DXCH_MACSEC_CLASSIFY_ECC_NOF_STATUS_COUNTERS_CNS            11

/* Number of ECC status counters in Transformer */
#define CPSS_DXCH_MACSEC_SECY_ECC_NOF_STATUS_COUNTERS_CNS                19

/* Maximum key length in bytes units (256 bits)  */
#define CPSS_DXCH_MACSEC_SECY_MAX_KEY_LENGTH_CNS                         32

/* Maximum Hash key length in bytes units (16 bytes)  */
#define CPSS_DXCH_MACSEC_SECY_MAX_HKEY_LENGTH_CNS                        16

/*  Counter increment disable control values
    Each bit disables all the counters of one counter type.
    Counter types:
        - TCAM counters
        - Channel counters */
#define CPSS_DXCH_MACSEC_CLASSIFY_TCAM_COUNT_INC_DIS_CNS     BIT_0
#define CPSS_DXCH_MACSEC_CLASSIFY_PORT_COUNT_INC_DIS_CNS     BIT_1

/*  Counter increment disable control values
    Each bit disables all the counters of one counter type.
    Counter types:
        - SA counters
        - Interface counters
        - Interface1 counters
        - SecY counters
        - Channel counters
        - RxCAM counters */
/* SA counters Increment disable bit */
#define CPSS_DXCH_MACSEC_SECY_SA_COUNT_INC_DIS_CNS           BIT_0
/* Interface counters Increment disable bit */
#define CPSS_DXCH_MACSEC_SECY_IFC_COUNT_INC_DIS_CNS          BIT_1
/* Interface1 counters Increment disable bit */
#define CPSS_DXCH_MACSEC_SECY_IFC1_COUNT_INC_DIS_CNS         BIT_2
/* SecY counters Increment disable bit */
#define CPSS_DXCH_MACSEC_SECY_SECY_COUNT_INC_DIS_CNS         BIT_3
/* Port counters Increment disable bit */
#define CPSS_DXCH_MACSEC_SECY_PORT_COUNT_INC_DIS_CNS         BIT_4
/* RxCAM counters Increment disable bit */
#define CPSS_DXCH_MACSEC_SECY_RXCAM_COUNT_INC_DIS_CNS        BIT_5







/**
 * @typedef: GT_MACSEC_UNIT_BMP
 *
 * @brief: Defines Data path bitmap to be used by MACSec APIs
 *               (APPLICABLE VALUES: 1; 2; 3.)
 *               1: Data Path 0
 *               2: Data Path 1
 *               3: Data Path 0 and Data Path 1
 *
 *         AC5P: support 2 Data Paths 0 or 1
 *                 Data Path 0 : MAC ports  0..25  , 105-CPU SDMA port
 *                 Data Path 1 : MAC ports  26..51 , 104-CPU network port , 106-CPU SDMA port (104,106 are muxed)
 *
 *         AC5X: single Data Path (so the bmp is ignored !)
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef GT_U32 GT_MACSEC_UNIT_BMP;


/**
 * @enum: CPSS_DXCH_MACSEC_DEVICE_TYPE_ENT
 *
 * @brief: Defines MACSec unit type: Classifier or Transformer
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef enum{

    /* Unit is Classifer (EIP-163) */
    CPSS_DXCH_MACSEC_CLASSIFIER_E,

    /* Unit is Transformer (EIP-164) */
    CPSS_DXCH_MACSEC_TRANSFORMER_E,
} CPSS_DXCH_MACSEC_DEVICE_TYPE_ENT;


/*
 * @enum: CPSS_DXCH_MACSEC_DEVICE_TYPE_ENT
 *
 * @brief: Defines MACSec direction: Either Egress only or Ingress only
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef enum
{
    /* Egress direction */
    CPSS_DXCH_MACSEC_DIRECTION_EGRESS_E,
    /* Ingress direction */
    CPSS_DXCH_MACSEC_DIRECTION_INGRESS_E
} CPSS_DXCH_MACSEC_DIRECTION_ENT;


/**
 * @enum: CPSS_DXCH_MACSEC_CLASSIFY_VPORT_PKT_EXPAND_TYPE_ENT
 *
 * @brief: Packet expansion indication
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef enum{

    /* 0 bytes. No expansion  */
    CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_NO_EXPANSION_E,

    /* Expand packet by 24 bytes  */
    CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_24B_E = 2,

    /* Expand packet by 32 bytes  */
    CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_32B_E,
} CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_ENT;


/**
 * @typedef: CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE
 *
 * @brief: This handle is a reference to a vPort.
 *         It is returned when a vPort is added and it remains valid until the vPort is removed.
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef GT_UINTPTR CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE;


/**
 * @typedef: CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE
 *
 * @brief: This handle is a reference to a TCAM rule.
 *         It is returned when a TCAM rule is added and it remains valid until the rule is removed.
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef GT_UINTPTR CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE;


/**
 * @typedef: CPSS_DXCH_MACSEC_SECY_SA_HANDLE
 *
 * @brief: This handle is a reference to an SA.
 *         It is returned when an SA is added and it remains valid until the SA is removed.
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef GT_UINTPTR CPSS_DXCH_MACSEC_SECY_SA_HANDLE;


/**
 * @enum: CPSS_DXCH_MACSEC_SECY_PORT_TYPE_ENT
 *
 * @brief: types of ports
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef enum
{
    /* COMMON PORT type */
    CPSS_DXCH_MACSEC_SECY_PORT_COMMON_E,
    /* CONTROLLED PORT type */
    CPSS_DXCH_MACSEC_SECY_PORT_CONTROLLED_E = 2,
    /* UNCONTROLLED PORT type */
    CPSS_DXCH_MACSEC_SECY_PORT_UNCONTROLLED_E = 3
} CPSS_DXCH_MACSEC_SECY_PORT_TYPE_ENT;


/**
 * @enum: CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_ET_MATCH_MODE_ENT
 *
 * @brief: Control packet detection match mode
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef enum
{
    /* Outer EtherType. Compare the first EtherType in the frame vs EtherType in control packet rules */
    CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_ET_MATCH_MODE_OUTER_E,
    /* Inner EtherType. Compare EtherType after VLAN stack vs EtherType in control packet rules */
    CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_ET_MATCH_MODE_INNER_E
} CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_ET_MATCH_MODE_ENT;


/**
 * @enum: CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_KAY_ENT
 *
 * @brief: MACSec KaY packet type
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef enum
{
    /* MACsec KaY packets are data packets */
    CPSS_DXCH_MACSEC_CLASSIFY_KAY_DATA_PACKET_E,
    /* MACsec KaY packets are control packets */
    CPSS_DXCH_MACSEC_CLASSIFY_KAY_CONTROL_PACKET_E
} CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_KAY_ENT;


/*
 * @enum: CPSS_DXCH_MACSEC_CLASSIFY_CTRL_PKT_DETECT_KAY_ENT
 *
 * @brief: MACSec KaY packet type
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef enum
{
    /* Disable comparision between packet and match rule */
    CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_DISABLE_COMPARE_E,
    /* Enable comparision between packet and match rule */
    CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_ENABLE_COMPARE_E
} CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_COMPARE_ENT;


/**
 * @struct: CPSS_DXCH_MACSEC_CLASSIFY_PORT_CFG_STC
 *
 * @brief: Classifier device port configuration
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* Indication if the low latency bypass must be enabled (true) or the MACsec mode (false) */
    GT_BOOL bypassMacsecDevice;

    /* Exception cases control in Classifier device */

   /* Enable or disable configuration of exception case  */
    GT_BOOL exceptionCfgEnable;

    /* Facilitates port reset sequence and allows in single write drop all incoming packets for a port
       true  - force dropping for all packets for a given port
       false - do not force dropping
       Valid only if exceptionCfgEn is true otherwise it is skipped */
    GT_BOOL forceDrop;

    /* Allows passing packets that missed the vPort matching,
       true -  allow assigning a default vPort for packet that missed vPort matching
       false - do not change the results of the vPort matching
       Valid only if exceptionCfgEn is true otherwise it is skipped */
    GT_BOOL defaultVPortValid;

    /* Default vPort id value, ignored if fDefaultVPortValid is set to false
       Valid only if exceptionCfgEn is true otherwise it is skipped */
    GT_U32 defaultVPort;
} CPSS_DXCH_MACSEC_CLASSIFY_PORT_CFG_STC;


/**
 * @struct: CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_MATCH_MASK_STC
 *
 * @brief: Classifier control packet detection match mask structure
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* Primary control packet detection match mask */
    CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_COMPARE_ENT primaryMask;

    /* Secondary control packet detection match mask */
    CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_COMPARE_ENT secondaryMask;
} CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_MATCH_MASK_STC;


/**
 * @struct: CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_KAY_MASK_STC
 *
 * @brief: Classifier control packet detection KaY packet mask structure
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* Primary control packet detection KaY mask */
    CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_KAY_ENT primaryKay;

    /* Secondary control packet detection KaY mask */
    CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_KAY_ENT secondaryKay;
} CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_KAY_MASK_STC;


/**
 * @struct: CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_ETHERTYPE_MATCH_MODE_STC
 *
 * @brief: Control packet EtheType match mode structure
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* Primary control packet detection KaY mask */
    CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_ET_MATCH_MODE_ENT primaryMatchMode;

    /* Secondary control packet detection KaY mask */
    CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_ET_MATCH_MODE_ENT secondaryMatchMode;
} CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_ETHERTYPE_MATCH_MODE_STC;


/**
 * @struct: CPSS_DXCH_MACSEC_CLASSIFY_MAC_DA_ET_MATCH_RULE_STC
 *
 * @brief: Classifier Packet match rule using MAC destination address and EtherType
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* Control packet detection mask of MAC DA */
    CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_MATCH_MASK_STC macDaMask;

    /* MAC destination address, 6 bytes. */
    GT_ETHERADDR  macDa;

    /* Control packet detection mask of EtherType */
    CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_MATCH_MASK_STC etherTypeMask;

    /* EtherType */
    GT_U16  etherType;

    /* Control packet match mode used to select which packet EtherType to compare with the EtherType value in the matching rule */
    CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_ETHERTYPE_MATCH_MODE_STC  etherTypeMatchMode;
} CPSS_DXCH_MACSEC_CLASSIFY_MAC_DA_ET_MATCH_RULE_STC;


/**
 * @struct: CPSS_DXCH_MACSEC_CLASSIFY_MAC_DA_RANGE_MATCH_RULE_STC
 *
 * @brief: Classifier packet match rule using range of MAC destination addresses
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* Control packet detection mask of MAC DA range */
    CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_MATCH_MASK_STC macRangeMask;

    /* Start MAC destination address, 6 bytes. */
    GT_ETHERADDR  macDaStart;

    /* End MAC destination address, 6 bytes. */
    GT_ETHERADDR  macDaEnd;
} CPSS_DXCH_MACSEC_CLASSIFY_MAC_DA_RANGE_MATCH_RULE_STC;


/**
 * @struct: CPSS_DXCH_MACSEC_CLASSIFY_MAC_DA_ET_RANGE_MATCH_RULE_STC
 *
 * @brief: Classifier packet match rule using range of MAC destination addresses and EtherType
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* Control packet detection mask of MAC DA range & ET */
    CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_MATCH_MASK_STC macDaRangeEtherTypeMask;

    /* Start MAC destination address, 6 bytes. Only applicable if fMacDaEn is true */
    GT_ETHERADDR  macDaStart;

    /* End MAC destination address, 6 bytes Only applicable if fMacDaEn is true */
    GT_ETHERADDR  macDaEnd;

    /* EtherType */
    GT_U16 etherType;

    /* Control packet match mode used to select which packet EtherType to compare with the EtherType value in the matching rule */
    CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_ETHERTYPE_MATCH_MODE_STC  etherTypeMatchMode;
} CPSS_DXCH_MACSEC_CLASSIFY_MAC_DA_ET_RANGE_MATCH_RULE_STC;


/**
 * @struct: CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_STC
 *
 * @brief: Classifier control Packet Detector settings
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* MAC destination address and EtherType
       Set macDaEtRules[n].macDaEn to false if no update is required,
       corresponding bits in cpMatchEnableMask and scpMatchEnableMask should be set to 0  */
    CPSS_DXCH_MACSEC_CLASSIFY_MAC_DA_ET_MATCH_RULE_STC   macDaEtRules[CPSS_DXCH_MACSEC_CLASSIFY_MAC_DA_ET_MATCH_RULES_COUNT_CNS];

    /* Range of MAC destination address used with EtherType
       Set macDaEtRange[n].range.macRangeEnable to false if no update is required,
       corresponding bits in cpMatchEnableMask and scpMatchEnableMask should be set to 0   */
    CPSS_DXCH_MACSEC_CLASSIFY_MAC_DA_ET_RANGE_MATCH_RULE_STC   macDaEtRange[CPSS_DXCH_MACSEC_CLASSIFY_MAC_DA_ET_RANGE_MATCH_RULES_COUNT_CNS];

    /* Range of MAC destination address (no EtherType)
       Set macDaRange[n].macRangeEnable to false if no update is required,
       corresponding bits in cpMatchEnableMask and scpMatchEnableMask should be set to 0 */
    CPSS_DXCH_MACSEC_CLASSIFY_MAC_DA_RANGE_MATCH_RULE_STC  macDaRange;

    /* Control packet detection mask of MAC DA 44 bits constant */
    CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_MATCH_MASK_STC macDa44BitsConstMask;

    /* 44-bit MAC destination address constant, 6 bytes */
    GT_ETHERADDR  macDa44BitsConst;

    /* Control packet detection mask of MAC DA 48 bits constant */
    CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_MATCH_MASK_STC macDa48BitsConstMask;

    /* 48-bit MAC destination address constant, 6 bytes */
    GT_ETHERADDR macDa48BitsConst;

    /* Control packet detection of MACSec KaY packets. */
    CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_KAY_MASK_STC macSecKayPktMask;
}  CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_STC;


/**
 * @struct: CPSS_DXCH_MACSEC_CLASSIFY_SECTAG_PARSER_STC
 *
 * @brief: Transformer MACsec SecTAG parser control
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* true - compare Ether-type in packet against MACsecTagValue value */
    GT_BOOL compType;

    /* true - check V (Version) bit to be 0 */
    GT_BOOL checkVersion;

    /* true - check if packet is meant to be handled by KaY (C & E bits) */
    GT_BOOL checkKay;

    /* This flag controls key generation for packets with SecTAG after MAC SA.
       true - enable generation of rule key with packet type MACsec (TCI, SCI)
       false - always generate normal rule keys  */
    GT_BOOL lookupUseSci;

    /* Ether-type value used for MACsec tag type comparison */
    GT_U16  macSecTagValue;
} CPSS_DXCH_MACSEC_CLASSIFY_SECTAG_PARSER_STC;


/**
 * @struct: CPSS_DXCH_MACSEC_CLASSIFY_VLAN_PARSER_TAG_STC
 *
 * @brief: Classifier VLAN tags parsing control
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* Enable detection of VLAN tags matching QTag value */
    GT_BOOL parseQTag;

    /* Enable detection of VLAN tags matching STag1 value */
    GT_BOOL parseStag1;

    /* Enable detection of VLAN tags matching STag2 value */
    GT_BOOL parseStag2;

    /* Enable detection of VLAN tags matching STag3 value */
    GT_BOOL parseStag3;

    /* Enable detection of multiple back-2-back VLAN tags (Q-in-Q and beyond) */
    GT_BOOL parseQinQ;
} CPSS_DXCH_MACSEC_CLASSIFY_VLAN_PARSER_TAG_STC;


/**
 * @struct: CPSS_DXCH_MACSEC_CLASSIFY_VLAN_PARSER_STC
 *
 * @brief: Classifier VLAN parsing control
 *         The classification engine is able to detect up to five VLAN tags following
 *         the outer MAC_SA and parse the first two for packet classification
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* 1. Per-channel VLAN parsing settings */

    /* Input header parser VLAN tags parsing settings */
    CPSS_DXCH_MACSEC_CLASSIFY_VLAN_PARSER_TAG_STC  cp;

    /* true  - update the secondary header parser VLAN tags parsing settings with values from the SCP data structure member
       false - no update is required   */
    GT_BOOL scpFlag;

    /* Secondary header parser VLAN tags parsing settings */
    CPSS_DXCH_MACSEC_CLASSIFY_VLAN_PARSER_TAG_STC  scp;

    /* 2. Global VLAN parsing settings */

    /* Enable user priority processing for 802.1s packets. If set, when a 802.1s tag is found,
       take the user priority directly from the PCP field, otherwise take the default user priority.  */
    GT_BOOL sTagUpEnable;

    /* Enable user priority processing for 802.1Q packets. If set, when a 802.1Q tag is found,
       translate the PCP field using UpTable1, otherwise take the default user priority.   */
    GT_BOOL qTagUpEnable;

    /* Default user priority, assigned to non-VLAN packets and
       to VLAN packets for which the VLAN user priority processing is disabled   */
    GT_U8 defaultUp; /*< Allowed values in range 0 .. 7! */

    /* Translation tables to derive the user priority from the PCP field in 802.1Q tags.
       If the PCP field is 0 then take UpTable[0] as the user priority, if PCP=1 then take UpTable[1], etc.  */

    /* Translation table for 1st 802.1Q  tag, allowed values in range 0 .. 7! */
    GT_U8 upTable1[CPSS_DXCH_MACSEC_CLASSIFY_VLAN_UP_MAX_COUNT_CNS];

    /* Translation table for 2nd 802.1Q  tag, allowed values in range 0 .. 7! */
    GT_U8 upTable2[CPSS_DXCH_MACSEC_CLASSIFY_VLAN_UP_MAX_COUNT_CNS];

    /* Ethertype value used for 802.1Q  tag type comparison */
    GT_U16 qTag;

    /* Ethertype value used for 802.1s  tag 1 type comparison */
    GT_U16 sTag1;

    /* Ethertype value used for 802.1s  tag 2 type comparison */
    GT_U16 sTag2;

    /* Ethertype value used for 802.1s  tag 3 type comparison */
    GT_U16 sTag3;
} CPSS_DXCH_MACSEC_CLASSIFY_VLAN_PARSER_STC;


/**
 * @struct: CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_CONTROL_STC
 *
 * @brief: Classifier Device statistics control
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* true - statistics counters are automatically reset on a read operation
       false - no reset on a read operation   */
    GT_BOOL autoStatCntrsReset;

    /* Threshold for the frame counters */
    GT_U64 countFrameThr;

    /* Threshold for the per-port frame counters */
    GT_U64 portCountFrameThr;

    /*  Counter increment disable control values
        Each bit disables all the counters of one counter type.
        Counter types:
            - TCAM counters
            - Port counters
        To disable TCAM counters set it with CPSS_DXCH_MACSEC_CLASSIFY_TCAM_COUNT_INC_DIS_CNS
        To disable port counters set it with CPSS_DXCH_MACSEC_CLASSIFY_PORT_COUNT_INC_DIS_CNS
        To disable both set it with CPSS_DXCH_MACSEC_CLASSIFY_TCAM_COUNT_INC_DIS_CNS | CPSS_DXCH_MACSEC_CLASSIFY_PORT_COUNT_INC_DIS_CNS
        To enable set it to 0  */
    GT_U8 countIncDisCtrl;
} CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_CONTROL_STC;


/**
 * @struct: CPSS_DXCH_MACSEC_SECY_STATISTICS_CONTROL_STC
 *
 * @brief: Transformer Device statistics control
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* true - statistics counters are automatically reset on a read operation
       false - no reset on a read operation */
    GT_BOOL autoStatCntrsReset;

    /* Outbound sequence number threshold value (one global for all SA's) When non-0 the SecY device will
       generate a notification to indicate the threshold event which can be used to start the re-keying procedure.
       If set to zero then only the sequence number roll-over notification will be generated.
       note: This is a global parameter for all the SA's added to one SecY device. */
    GT_U32 seqNrThreshold;

    /* Outbound sequence number threshold value for 64-bit packet numbering */
    GT_U64 seqNrThreshold64;

    /* Threshold for the SA frame counters */
    GT_U64 saCountFrameThr;
    /* Threshold for the SecY frame counters */
    GT_U64 secyCountFrameThr;
    /* Threshold for the IFC frame counters */
    GT_U64 ifcCountFrameThr;
    /* Threshold for the IFC1 frame counters */
    GT_U64 ifc1CountFrameThr;
    /* Threshold for the RxCAM frame counters (Ingress only) */
    GT_U64 rxCamCountFrameThr;

    /* Threshold for the SA octet counters */
    GT_U64 saCountOctetThr;
    /* Threshold for the IFC octet counters */
    GT_U64 ifcCountOctetThr;
    /* Threshold for the IFC1 octet counters */
    GT_U64 ifc1CountOctetThr;

    /*  Counter increment disable control values
        Each bit disables all the counters of one counter type.
        Counter types:
            - SA counters
            - Interface counters
            - Interface1 counters
            - SecY counters
            - Channel counters
            - RxCAM counters (Ingress only)

        To disable SA counters set it with CPSS_DXCH_MACSEC_SECY_SA_COUNT_INC_DIS
        To disable IF counters set it with CPSS_DXCH_MACSEC_SECY_IFC_COUNT_INC_DIS
        To disable IF1 counters set it with CPSS_DXCH_MACSEC_SECY_IFC1_COUNT_INC_DIS
        To disable SECY counters set it with CPSS_DXCH_MACSEC_SECY_SECY_COUNT_INC_DIS
        To disable PORT counters set it with CPSS_DXCH_MACSEC_SECY_PORT_COUNT_INC_DIS
        To disable RcCAM counters set it with CPSS_DXCH_MACSEC_SECY_RXCAM_COUNT_INC_DIS (Ingress only)
        To enable set it to 0  */
    GT_U8 countIncDisCtrl;
} CPSS_DXCH_MACSEC_SECY_STATISTICS_CONTROL_STC;


/**
 * @struct: CPSS_DXCH_MACSEC_SECY_PORT_STAT_CONTROL_STC
 *
 * @brief: Transformer port statistics control (thresholds)
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* Outbound sequence number threshold value
      When non-0 the SecY device will generate a notification to indicate
      the threshold event which can be used to start the re-keying procedure.
      If set to zero then only the sequence number roll-over notification will be generated.
      note This is a global parameter for all the SA's added to one SecY device.   */
    GT_U32 seqNrThreshold;

    /* Outbound sequence number threshold value for 64-bit packet numbering */
    GT_U64 seqNrThreshold64;
} CPSS_DXCH_MACSEC_SECY_PORT_STAT_CONTROL_STC;


/**
 * @struct: CPSS_DXCH_MACSEC_SECY_PORT_RULE_SECTAG_STC
 *
 * @brief: Transformer MACsec SecTAG parsing rules,
 *         these rules classify each packet into one of four categories,
 *         1) Untagged, the packet has no MACsec tag, i.e. the Ether-type differs from 0x88E5.
 *         2) Bad tag, the packet has an invalid MACsec tag
 *         3) KaY tag, the packet has a KaY tag. These packets are generated and/or handled by
 *            application software and no MACsec processing is performed for them by the
 *            Classification device except for straight bypass.
 *         4) Tagged, the packet has a valid MACsec tag that is not KaY.
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* Compare Ether-type in packet against EtherType value.
       false - all packets are treated as MACsec (no packets are classified as untagged).
       default is true */
    GT_BOOL compEtype;

    /* true - check V bit to be 0
       default is true */
    GT_BOOL checkV;

    /* true - check if this is a KaY packet (C and E bits)
       default is true */
    GT_BOOL checkKay;

    /* true - check illegal C and E bits combination (C=1 and E=0)
       default is true */
    GT_BOOL checkCe;

    /* true - check illegal SC/ES/SCB bits combinations
       default is true */
    GT_BOOL checkSc;

    /* true - check if SL (Short Length) field value is out of range
       default is true */
    GT_BOOL checkSl;

    /* true - check PN (Packet Number) is non-zero
       default is true */
    GT_BOOL checkPn;
} CPSS_DXCH_MACSEC_SECY_PORT_RULE_SECTAG_STC;


/**
 * @struct: CPSS_DXCH_MACSEC_SECY_PORT_CFG_STC
 *
 * @brief: Transformer SecY Port configuration parameters
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* Indication if to bypass MACSec (true) for port or to set MACsec mode (false)  */
    GT_BOOL bypassMacsecDevice;

    /* Port statistics control settings (threshold values) */
    CPSS_DXCH_MACSEC_SECY_PORT_STAT_CONTROL_STC statCtrl;

    /* Mode for packet sequence number threshold comparison:
          false - comparison is greater or equal,
          true  - comparison is strictly equal.  */
    GT_BOOL pktNumThrStrictCompareModeEnable;

    /* MACsec SecTAG parsing rules */
    CPSS_DXCH_MACSEC_SECY_PORT_RULE_SECTAG_STC ruleSecTag;
} CPSS_DXCH_MACSEC_SECY_PORT_CFG_STC;


/**
 * @struct: CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC
 *
 * @brief: Classifier vPort (vPort policy) data structure
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* SecTag offset
       - For egress only. For ingress SecTAG location is determined by the parser
       - 7-bit wide: SecTAG location relative to the start of the frame. */
    GT_U8 secTagOffset;

    /* Packet expansion indication
       - For egress only. For ingress it should be configured to no expansion ('zero')
       - If the packet is classified as a control packet, this field is ignored and no expansion is indicated on the output interface. */
    CPSS_DXCH_MACSEC_CLASSIFY_PKT_EXPAND_TYPE_ENT pktExpansion;
} CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC;


/**
 * @struct: CPSS_DXCH_MACSEC_SECY_SA_EGR_STC
 *
 * @brief: Transformer SA parameters for Egress action type
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* true  - SA is in use, packets classified for it can be transformed
       false - SA not in use, packets classified for it can not be transformed   */
    GT_BOOL saInUse;

    /* The number of bytes (in the range of 0-63) that are authenticated but not encrypted following the SecTAG in the encrypted packet. */
    GT_U8 confidentialityOffset;

    /* true  - enable frame protection,
       false - bypass frame through device */
    GT_BOOL protectFrames;

    /* true  - inserts explicit SCI in the packet,
       false - use implicit SCI (not transferred) */
    GT_BOOL includeSci;

    /* true  - enable ES bit in the generated SecTAG
       false - disable ES bit in the generated SecTAG */
    GT_BOOL useEs;

    /* true  - enable SCB bit in the generated SecTAG
       false - disable SCB bit in the generated SecTAG */
    GT_BOOL useScb;

    /* true  - enable confidentiality protection
       false - disable confidentiality protection */
    GT_BOOL confProtect;

    /* true  - allow data (non-control) packets.
       false - drop data packets.*/
    GT_BOOL controlledPortEnable;

    /* Specifies number of bytes from the start of the frame to be bypassed without MACsec protection */
    GT_U8 preSecTagAuthStart;

    /* Specifies number of bytes to be authenticated in the pre-SecTAG area.*/
    GT_U8 preSecTagAuthLength;
} CPSS_DXCH_MACSEC_SECY_SA_EGR_STC;


/**
 * @enum: CPSS_DXCH_MACSEC_SECY_VALIDATE_FRAMES_ENT
 *
 * @brief: Transformer Ingress tagged frame validation options
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef enum
{
    /* Frame validate disable */
    CPSS_DXCH_MACSEC_SECY_FRAME_VALIDATE_DISABLE_E,
    /* Frame validate check   */
    CPSS_DXCH_MACSEC_SECY_FRAME_VALIDATE_CHECK_E,
    /* Frame validate strict  */
    CPSS_DXCH_MACSEC_SECY_FRAME_VALIDATE_STRICT_E
} CPSS_DXCH_MACSEC_SECY_VALIDATE_FRAMES_ENT;


/**
 * @struct: CPSS_DXCH_MACSEC_SECY_SA_ING_STC
 *
 * @brief: Transformer SA parameters for Ingress action type
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* true  - SA is in use, packets classified for it can be transformed
       false - SA not in use, packets classified for it can not be transformed */
    GT_BOOL saInUse;

    /* The number of bytes (in the range of 0-127) that are authenticated but not encrypted following the SecTAG in the encrypted packet.
       Values 65-127 are reserved in hardware < 4.0 and should not be used there.     */
    GT_U8 confidentialityOffset;

    /* true  - enable replay protection
       false - disable replay protection */
    GT_BOOL replayProtect;

    /* MACsec frame validation level (tagged). */
    CPSS_DXCH_MACSEC_SECY_VALIDATE_FRAMES_ENT validateFramesTagged;

    /* SCI to which ingress SA applies (8 bytes). */
    GT_U8 sciArr[CPSS_DXCH_MACSEC_SECY_SCI_SIZE_CNS];

    /* Association number to which ingress SA applies. */
    GT_U8 an;

    /* true - allow tagged packets.
      false - drop tagged packets. */
    GT_BOOL allowTagged;

    /* true - allow untagged packets.
       false - drop untagged packets. */
    GT_BOOL allowUntagged;

    /* true  - enable validate untagged packets.
       false - disable validate untagged packets. */
    GT_BOOL validateUntagged;

    /* pre-Sectag Auth start  */
    GT_U8 preSecTagAuthStart;

    /* pre-Sectag Auth length */
    GT_U8 preSecTagAuthLength;

    /* For situations when RxSC is not found or saInUse=false with validation level that allows packet to be sent
       to the Controlled port with the SecTAG/ICV removed, this flag represents a policy to allow SecTAG retaining.
       true - SecTAG is retained.   */
    GT_BOOL retainSecTag;

    /* true - ICV is retained (allowed only when retainSecTAG is true). */
    GT_BOOL retainIcv;
} CPSS_DXCH_MACSEC_SECY_SA_ING_STC;


/**
 * @struct: CPSS_DXCH_MACSEC_SECY_SA_BYPASS_DROP_STC
 *
 * @brief: Transformer SA parameters for Bypass/Drop action type
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* true  - enable statistics counting for the associated SA
       false - disable statistics counting for the associated SA */
    GT_BOOL saInUse;
} CPSS_DXCH_MACSEC_SECY_SA_BYPASS_DROP_STC;


/**
 * @struct: CPSS_DXCH_MACSEC_SECY_SA_CRYPT_AUTH_STC
 *
 * @brief: Transformer SA parameters for Crypt-Authenticate action type
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* true  - message has length 0
       false - message has length > 0 */
    GT_BOOL zeroLengthMessage;

    /* The number of bytes (in the range of 0-255) that are authenticated but not encrypted (AAD length). */
    GT_U8 confidentialityOffset;

    /* IV loading mode:
       0: The IV is fully loaded via the transform record.
       1: The full IV is loaded via the input frame. This IV is located in front of the frame and is
          considered to be part of the bypass data, however it is not part to the result frame.
       2: The full IV is loaded via the input frame. This IV is located at the end of the bypass data
          and is considered to be part of the bypass data, and it also part to the result frame.
       3: The first three IV words are loaded via the input frame, the counter value of the IV is set to one.
          The three IV words are located in front of the frame and are considered to be part of the bypass data,
          however it is not part to the result frame. */
    GT_U8 ivMode;

    /* true  - append the calculated ICV
       false - don't append the calculated ICV */
    GT_BOOL icvAppend;

    /* true  - enable ICV verification
       false - disable ICV verification */
    GT_BOOL icvVerify;

    /* true  - enable confidentiality protection (AES-GCM/CTR operation)
       false - disable confidentiality protection (AES-GMAC operation) */
    GT_BOOL confProtect;
} CPSS_DXCH_MACSEC_SECY_SA_CRYPT_AUTH_STC;


/**
 * @enum: CPSS_DXCH_MACSEC_SECY_SA_ACTION_ENT
 *
 * @brief: Transformer SA action type:
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef enum
{
    /* SA action bypass */
    CPSS_DXCH_MACSEC_SECY_SA_ACTION_BYPASS_E,
    /* SA action drop */
    CPSS_DXCH_MACSEC_SECY_SA_ACTION_DROP_E,
    /* SA action ingress */
    CPSS_DXCH_MACSEC_SECY_SA_ACTION_INGRESS_E,
    /* SA action egress */
    CPSS_DXCH_MACSEC_SECY_SA_ACTION_EGRESS_E,
    /* SA action crypt-auth */
    CPSS_DXCH_MACSEC_SECY_SA_ACTION_CRYPT_AUTH_E
} CPSS_DXCH_MACSEC_SECY_SA_ACTION_ENT;


/**
 * @struct: CPSS_DXCH_MACSEC_SECY_SA_PARAM_UNT
 *
 * @brief: Transformer SecY SA parameters union that contains data required to add a new SA
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef union
{
    /* SA parameters for Egress action type */
    CPSS_DXCH_MACSEC_SECY_SA_EGR_STC         egress;

    /* SA parameters for Ingress action type */
    CPSS_DXCH_MACSEC_SECY_SA_ING_STC         ingress;

    /* SA parameters for Bypass/Drop action type */
    CPSS_DXCH_MACSEC_SECY_SA_BYPASS_DROP_STC bypassDrop;

    /* SA parameters for Crypt-Authenticate action type */
    CPSS_DXCH_MACSEC_SECY_SA_CRYPT_AUTH_STC  cryptAuth;
} CPSS_DXCH_MACSEC_SECY_SA_PARAM_UNT;


/**
 * @struct: CPSS_DXCH_MACSEC_SECY_SA_STC
 *
 * @brief: Transformer SecY SA data structure that contains data required to add a new SA
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* SA parameters */
    CPSS_DXCH_MACSEC_SECY_SA_PARAM_UNT params;

    /* SA action type */
    CPSS_DXCH_MACSEC_SECY_SA_ACTION_ENT actionType;

    /* Destination port */
    CPSS_DXCH_MACSEC_SECY_PORT_TYPE_ENT destPort;
} CPSS_DXCH_MACSEC_SECY_SA_STC;

/**
 * @struct: CPSS_DXCH_MACSEC_CLASSIFY_RULE_KEY_MASK_STC
 *
 * @brief: Classifier vPort matching rule Key/Mask data structure
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /** @brief Packet type, 2 bit field with following values
     * 0 : packet type is untagged, VLAN, etc
     * 3 : packet type is MACSEC (Ingress direction only)
     */
    GT_U8  packetType;

    /*    Bit 0 = 1 : No VLAN tags
          Bit 1 = 1 : 1 VLAN tag
          Bit 2 = 1 : 2 VLAN tags
          Bit 3 = 1 : 3 VLAN tags
          Bit 4 = 1 : 4 VLAN tags
          Bit 5 = 0 : Reserved, must be written with zero */
    GT_U8 numTags;

    /* Port physical number
       Width of this field depends on number of supported ports */
    GT_PHYSICAL_PORT_NUM portNum;
} CPSS_DXCH_MACSEC_CLASSIFY_RULE_KEY_MASK_STC;


/**
 * @struct: CPSS_DXCH_MACSEC_CLASSIFY_RULE_POLICY_STC
 *
 * @brief: Classifier vPort matching rule policy
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* Priority value that is used to resolve multiple rule matches.
       When multiple rules are hit by a packet simultaneously, the rule with the higher priority value will be returned.
       If multiple rules with an identical priority value are hit, the rule with the lowest rule index is used. */
    GT_U8 rulePriority;

    /* true : drop the packet */
    GT_BOOL drop;

    /* true : process the packet as control */
    GT_BOOL controlPacket;
} CPSS_DXCH_MACSEC_CLASSIFY_RULE_POLICY_STC;


/**
 * @struct: CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC
 *
 * @brief: Classifier vPort matching rule data structure
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* Sets matching values */
    CPSS_DXCH_MACSEC_CLASSIFY_RULE_KEY_MASK_STC key;

    /* Mask for matching values, can be used to mask out irrelevant Key bits */
    CPSS_DXCH_MACSEC_CLASSIFY_RULE_KEY_MASK_STC mask;

    /* Data[0] : MAC Destination Address least significant bytes (3..0)
       Data[1] : MAC Destination Address most significant bytes (5, 4)
       Data[2] : Packet data (EtherType, VLAN tag)
       Data[3] : Packet data (EtherType, VLAN tag)
       See TCAM packet data fields description in the EIP-163 Programmer Manual */
    GT_U32 data [CPSS_DXCH_MACSEC_CLASSIFY_RULE_NON_CTRL_WORD_COUNT_CNS];

    /* Mask for data values, can be used to mask out irrelevant Data bits */
    GT_U32 dataMask [CPSS_DXCH_MACSEC_CLASSIFY_RULE_NON_CTRL_WORD_COUNT_CNS];

    /* Rule policy */
    CPSS_DXCH_MACSEC_CLASSIFY_RULE_POLICY_STC policy;
} CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC;


/**
 * @enum: CPSS_DXCH_MACSEC_SECY_SA_BUILDER_DIRECTION_ENT
 *
 * @brief: Specify Transformer device direction: egress (encrypt) or ingress (decrypt)
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef enum
{
    /* SA build direction egress  */
    CPSS_DXCH_MACSEC_SECY_SAB_DIRECTION_EGRESS_E,
    /* SA build direction ingress  */
    CPSS_DXCH_MACSEC_SECY_SAB_DIRECTION_INGRESS_E
} CPSS_DXCH_MACSEC_SECY_SA_BUILDER_DIRECTION_ENT;


/**
 * @struct: CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC
 *
 * @brief: Transform Record parameters
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* Association Number  (APPLICABLE RANGE: 0..3)  */
    GT_U8   an;

    /* MACsec key */
    GT_U8   keyArr[CPSS_DXCH_MACSEC_SECY_MAX_KEY_LENGTH_CNS];

    /* MACsec key size */
    GT_U32  keyByteCount;

    /* SCI (8B) */
    GT_U8   sciArr[CPSS_DXCH_MACSEC_SECY_SCI_SIZE_CNS];

    /* Extended packet numbering: if true then use 64 bits sequence number
                                  if false then use 32 bits sequence number   */
    GT_BOOL seqTypeExtended;

    /* Sequence number low */
    GT_U32  seqNumLo;

    /* Sequence number high. Appilcable only if seqTypeExtended is set to True */
    GT_U32  seqNumHi;

    /* SSCI (4B). Appilcable only if seqTypeExtended is set to True */
    GT_U8   ssciArr[CPSS_DXCH_MACSEC_SECY_SSCI_SIZE_CNS];

    /* SALT (16B). Appilcable only if seqTypeExtended is set to True */
    GT_U8   saltArr[CPSS_DXCH_MACSEC_SECY_SALT_SIZE_CNS];

    /* Replay window size: specifies the window size for ingress sequence number checking. Value 0 is enforced for strict ordering.
       If seqTypeExtended is set to true:  (APPLICABLE RANGE: 0..2^30)
       If seqTypeExtended is set to false: (APPLICABLE RANGE: 0..(2^32 - 1) )   */
    GT_U32  seqMask;

    /* Custom Hash key enable flag
       If true then Hash key can be cusomized and set in customHkeyArr[]
       If false then Hash key will be automatically generated */
    GT_BOOL customHkeyEnable;

    /* 128-bit key for the authentication operation
       Applicable only if customHkeyEnable is set to true */
    GT_U8   customHkeyArr[CPSS_DXCH_MACSEC_SECY_MAX_HKEY_LENGTH_CNS];
} CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC;



/************************
 Statistics
 ************************/

/**
 * @struct: CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_PORT_STC
 *
 * @brief: Classifier Per-port statistics counters
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* Number of packets that hit multiple TCAM entries */
    GT_U64 tcamHitMultiple;
    /* Number of packets that were dropped by header parser */
    GT_U64 headerParserDroppedPkts;
    /* Number of packets that missed TCAM lookup */
    GT_U64 tcamMiss;
    /* Number of control packets */
    GT_U64 pktsCtrl;
    /* Number of data packets */
    GT_U64 pktsData;
    /* Number of packets that were dropped*/
    GT_U64 pktsDropped;
    /* Number of packets marked as error packets in input  */
    GT_U64 pktsErrIn;
} CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_PORT_STC ;


/**
 * @struct: CPSS_DXCH_MACSEC_SECY_STAT_SA_E_STC
 *
 * @brief: Transformer SA Egress counters
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* Packet counters - Encrypted Protected */
    GT_U64 outPktsEncryptedProtected;
    /* Packet counters - TooLong */
    GT_U64 outPktsTooLong;
    /* Packet counters - SA Not In Use */
    GT_U64 outPktsSANotInUse;
    /* Octet counters - Encrypted Protected */
    GT_U64 outOctetsEncryptedProtected;
} CPSS_DXCH_MACSEC_SECY_STAT_SA_E_STC;


/**
 * @struct: CPSS_DXCH_MACSEC_SECY_STAT_SA_I_STC
 *
 * @brief: Transformer SA Ingress counters
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* Packet counters - Unchecked */
    GT_U64 inPktsUnchecked;
    /* Packet counters - Delayed */
    GT_U64 inPktsDelayed;
    /* Packet counters - Late */
    GT_U64 inPktsLate;
    /* Packet counters - OK */
    GT_U64 inPktsOK;
    /* Packet counters - Invalid */
    GT_U64 inPktsInvalid;
    /* Packet counters - Not Valid */
    GT_U64 inPktsNotValid;
    /* Packet counters - Not Using SA */
    GT_U64 inPktsNotUsingSA;
    /* Packet counters - Unused SA */
    GT_U64 inPktsUnusedSA;
    /* Octet counters - Decrypted */
    GT_U64 inOctetsDecrypted;
    /* Octet counters - Validated */
    GT_U64 inOctetsValidated;
} CPSS_DXCH_MACSEC_SECY_STAT_SA_I_STC;


/**
 * @struct: CPSS_DXCH_SECY_MACSEC_STAT_SA_UNT
 *
 * @brief: Union for Transformer SA Statistics counters for both Ingress & Egress
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef union
{
    /* SA Statistics for Egress */
    CPSS_DXCH_MACSEC_SECY_STAT_SA_E_STC egress;
    /* SA Statistics for Ingress */
    CPSS_DXCH_MACSEC_SECY_STAT_SA_I_STC ingress;
} CPSS_DXCH_SECY_MACSEC_STAT_SA_UNT;


/**
 * @struct: CPSS_DXCH_MACSEC_SECY_STAT_SECY_E_STC
 *
 * @brief: Transformer SECY Egress counters
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* Packet counters - Transform Error */
    GT_U64 outPktsTransformError;
    /* Packet counters - Control */
    GT_U64 outPktsControl;
    /* Packet counters - Untagged */
    GT_U64 outPktsUntagged;
} CPSS_DXCH_MACSEC_SECY_STAT_SECY_E_STC;


/**
 * @struct: CPSS_DXCH_MACSEC_SECY_STAT_SECY_I_STC
 *
 * @brief: Transformer SECY Ingress counters
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* Packet counters - Transform Error */
    GT_U64 inPktsTransformError;
    /* Packet counters - Control */
    GT_U64 inPktsControl;
    /* Packet counters - Untagged */
    GT_U64 inPktsUntagged;
    /* Packet counters - No Tag */
    GT_U64 inPktsNoTag;
    /* Packet counters - Bad Tag */
    GT_U64 inPktsBadTag;
    /* Packet counters - No SCI */
    GT_U64 inPktsNoSCI;
    /* Packet counters - Unknown SCI */
    GT_U64 inPktsUnknownSCI;
    /* Packet counters - Tagged Ctrl */
    GT_U64 inPktsTaggedCtrl;
} CPSS_DXCH_MACSEC_SECY_STAT_SECY_I_STC;


/**
 * @struct: CPSS_DXCH_SECY_MACSEC_STAT_SECY_UNT
 *
 * @brief: Transformer Union for SecY Statistics counters for both Ingress & Egress
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef union
{
    /* Egress SecY Statistics */
    CPSS_DXCH_MACSEC_SECY_STAT_SECY_E_STC egress;
    /* Ingress SecY Statistics */
    CPSS_DXCH_MACSEC_SECY_STAT_SECY_I_STC ingress;
} CPSS_DXCH_SECY_MACSEC_STAT_SECY_UNT;


/**
 * @struct: CPSS_DXCH_MACSEC_SECY_STAT_IFC_E_STC
 *
 * @brief: Transformer IFC (interface) Egress statistics
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* Packet counters - Unicast Uncontrolled */
    GT_U64 outPktsUnicastUncontrolled;
    /* Packet counters - Multicast Uncontrolled */
    GT_U64 outPktsMulticastUncontrolled;
    /* Packet counters - Broadcast Uncontrolled */
    GT_U64 outPktsBroadcastUncontrolled;

    /* Packet counters - Unicast Controlled */
    GT_U64 outPktsUnicastControlled;
    /* Packet counters - Multicast Controlled */
    GT_U64 outPktsMulticastControlled;
    /* Packet counters - Broadcast Controlled */
    GT_U64 outPktsBroadcastControlled;

    /* Octet counters - Uncontrolled */
    GT_U64 outOctetsUncontrolled;
    /* Octet counters - Controlled */
    GT_U64 outOctetsControlled;
    /* Octet counters - Common */
    GT_U64 outOctetsCommon;
} CPSS_DXCH_MACSEC_SECY_STAT_IFC_E_STC;


/**
 * @struct: CPSS_DXCH_MACSEC_SECY_STAT_IFC_I_STC
 *
 * @brief: Transformer IFC (interface) Ingress statistics
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* Packet counters - Unicast uncontrolled */
    GT_U64 inPktsUnicastUncontrolled;
    /* Packet counters - Multicast Uncontrolled */
    GT_U64 inPktsMulticastUncontrolled;
    /* Packet counters - Broadcast Uncontrolled */
    GT_U64 inPktsBroadcastUncontrolled;

    /* Packet counters - Unicast Controlled */
    GT_U64 inPktsUnicastControlled;
    /* Packet counters - Multicast Controlled */
    GT_U64 inPktsMulticastControlled;
    /* Packet counters - Broadcast Controlled */
    GT_U64 inPktsBroadcastControlled;

    /* Octet counters - Uncontrolled */
    GT_U64 inOctetsUncontrolled;
    /* Octet counters - Controlled */
    GT_U64 inOctetsControlled;
} CPSS_DXCH_MACSEC_SECY_STAT_IFC_I_STC;


/**
 * @struct: CPSS_DXCH_MACSEC_SECY_STAT_IFC_UNT
 *
 * @brief: Transformer Union for Ifc Statistics counters for both Ingress & Egress
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef union
{
    /* Ifc Statistics counters for Egress */
    CPSS_DXCH_MACSEC_SECY_STAT_IFC_E_STC egress;
    /* Ifc Statistics counters for Ingress */
    CPSS_DXCH_MACSEC_SECY_STAT_IFC_I_STC ingress;
} CPSS_DXCH_MACSEC_SECY_STAT_IFC_UNT;



/************************
 Debug
 ************************/

/**
 * @struct: CPSS_DXCH_MACSEC_CLASSIFY_PKT_PROCESSING_DEBUG_STC
 *
 * @brief: Classifier Debug registers for packet classification.
 *         Can be read to find out how the last packet was classified.
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* Control packet matching status */
    GT_U32 cpMatchDebug;
    /* TCAM matching status */
    GT_U32 tcamDebug;
    /* SecTAG parser status */
    GT_U32 secTagDebug;
    /* Parsed destination address low */
    GT_U32 parsedDaLo;
    /* Parsed destination address high */
    GT_U32 parsedDaHi;
    /* Parsed source address low */
    GT_U32 parsedSaLo;
    /* Parsed source address high + ether type */
    GT_U32 parsedSaHi;
    /* Parsed SecTAG low */
    GT_U32 parsedSecTagLo;
    /* Parsed SecTAG high */
    GT_U32 parsedSecTagHi;
    /* TCAM lookup result (vPort policy) */
    GT_U32 debugFlowLookup;
} CPSS_DXCH_MACSEC_CLASSIFY_PKT_PROCESSING_DEBUG_STC;


/**
 * @struct: CPSS_DXCH_MACSEC_SECY_PKT_PROCESSING_DEBUG_STC
 *
 * @brief: Transformer Debug registers for packet classification.
 *         Can be read to find out how the last packet was classified.
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* Parsed destination address low*/
    GT_U32 parsedDaLo;
    /* Parsed destination address high*/
    GT_U32 parsedDaHi;
    /* Parsed source address low*/
    GT_U32 parsedSaLo;
    /* Parsed source address high + ether type*/
    GT_U32 parsedSaHi;
    /* Parsed source SecTAG low*/
    GT_U32 parsedSecTagLo;
    /* Parsed source SecTAG high*/
    GT_U32 parsedSecTagHi;
    /* Parsed source SCI low*/
    GT_U32 parsedSciLo;
    /* Parsed source SCI high*/
    GT_U32 parsedSciHi;
    /* various fields related to SecTAG */
    GT_U32 secTagDebug;
    /* SCI for RxCAM lookup low*/
    GT_U32 rxCamSciLo;
    /* SCI for RxCAM lookup high*/
    GT_U32 rxCamSciHi;
    /* various fields related to RxCAM lookup*/
    GT_U32 parserInDebug;
} CPSS_DXCH_MACSEC_SECY_PKT_PROCESSING_DEBUG_STC;


/**
 * @struct: CPSS_DXCH_MACSEC_ECC_COUNTERS_STC
 *
 * @brief: ECC counters information
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* Number of correctable errors. */
    GT_U32 correctableCount;

    /* Number of uncorrectable errors.*/
    GT_U32 uncorrectableCount;

    /* Threshold for correctable errors exceeded */
    GT_BOOL correctableThr;

    /* Threshold for uncorrectable errors exceeded */
    GT_BOOL uncorrectableThr;
} CPSS_DXCH_MACSEC_ECC_COUNTERS_STC;


/**
 * @struct: CPSS_DXCH_MACSEC_CLASSIFY_UNIT_STATUS_STC
 *
 * @brief: Classyfier overall unit status
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* Array of ECC status counters */
    CPSS_DXCH_MACSEC_ECC_COUNTERS_STC  eccCountersInfo[CPSS_DXCH_MACSEC_CLASSIFY_ECC_NOF_STATUS_COUNTERS_CNS];

    /* Packet parser debug information */
    CPSS_DXCH_MACSEC_CLASSIFY_PKT_PROCESSING_DEBUG_STC pktProcessDebug;
} CPSS_DXCH_MACSEC_CLASSIFY_UNIT_STATUS_STC;


/**
 * @struct: CPSS_DXCH_MACSEC_SECY_UNIT_STATUS_STC
 *
 * @brief: Transformer overall unit status
 *
 *  APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman.
 */
typedef struct
{
    /* Array of ECC status counters */
    CPSS_DXCH_MACSEC_ECC_COUNTERS_STC  eccCountersInfo[CPSS_DXCH_MACSEC_SECY_ECC_NOF_STATUS_COUNTERS_CNS];


    /* Packet parser debug information */
    CPSS_DXCH_MACSEC_SECY_PKT_PROCESSING_DEBUG_STC pktProcessDebug;
} CPSS_DXCH_MACSEC_SECY_UNIT_STATUS_STC;

/**
 --------------------------------------------------------------------------------------------------------------------------------------------------
  CPSS MACSec APIs declarations
 --------------------------------------------------------------------------------------------------------------------------------------------------
 **/


/**********************************
 Device configuration updates APIs
 **********************************/

/**
* @internal cpssDxChMacSecInit function
* @endinternal
*
* @brief   MACSec devices initialization which include:
*          - Enable MACSec devices for work. Enable access to MACSec registers.
*          - Set initial configurations for Classifier & Transformer devices
*               - initialize all channels to pass packets in low-latency bypass mode (disable MACSec mode)
*               - clear all statistics counters and set all threshold counters to zero
*               - configure the statistics module to clear-on-read
*               - ECC thresholds are set to zero
*               - Automatic EOP insertion is disabled
*
*   NOTEs: - This API must be executed first before any other MACSec APIs
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2; CPSS_DATA_PATH_UNAWARE_MODE_CNS.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - use value of CPSS_DATA_PATH_UNAWARE_MODE_CNS to select all data paths.
*                                   - for non multi data paths device this parameter is IGNORED.
*
* @retval GT_OK                    - on success
* @retval GT_ALREADY_EXIST         - if MACSec DB was already initialized
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChMacSecInit
(
    IN  GT_U8               devNum,
    IN  GT_MACSEC_UNIT_BMP  unitBmp
);


/**
* @internal cpssDxChMacSecPortClassifyConfigSet function
* @endinternal
*
* @brief   Configuration settings per port in MACSec classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] portNum              - physical port number
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] portCfgPtr           - (pointer to) port configuratios parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecPortClassifyConfigSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_PORT_CFG_STC  *portCfgPtr
);


/**
* @internal cpssDxChMacSecPortClassifyConfigGet function
* @endinternal
*
* @brief   Get configuration settings per port from MACSec classifier
*          note: exceptionCfgEnable flag is ignored by function and all
*                exception parameters are read in any case
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum               - device number.
* @param[in]  portNum              - physical port number
* @param[in]  direction            - select Egress or Ingress MACSec classifier
* @param[OUT] portCfgPtr           - (pointer to) port configurations parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecPortClassifyConfigGet
(
    IN   GT_U8                                   devNum,
    IN   GT_PHYSICAL_PORT_NUM                    portNum,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    OUT  CPSS_DXCH_MACSEC_CLASSIFY_PORT_CFG_STC  *portCfgPtr
);


/**
* @internal cpssDxChMacSecPortClassifyControlPktDetectConfigSet function
* @endinternal
*
* @brief   Control packet detection settings per port in MACSec classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] portNum              - physical port number
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] ctrlPktDetectPtr     - (pointer to) control packet detection parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecPortClassifyControlPktDetectConfigSet
(
    IN  GT_U8                                      devNum,
    IN  GT_PHYSICAL_PORT_NUM                       portNum,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT             direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_STC  *ctrlPktDetectPtr
);


/**
* @internal cpssDxChMacSecPortClassifyControlPktDetectConfigGet function
* @endinternal
*
* @brief   Get control packet detection settings per port from MACSec classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] portNum              - physical port number
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[out] ctrlPktDetectPtr    - (pointer to) control packet detection parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecPortClassifyControlPktDetectConfigGet
(
    IN  GT_U8                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT              direction,
    OUT CPSS_DXCH_MACSEC_CLASSIFY_CONTROL_PKT_STC   *ctrlPktDetectPtr
);


/**
* @internal cpssDxChMacSecPortClassifyHdrParserSecTagConfigSet function
* @endinternal
*
* @brief   SecTAG parser settings per port in MACSec classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] portNum              - physical port number
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] secTagParserPtr      - (pointer to) sectag parser parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecPortClassifyHdrParserSecTagConfigSet
(
    IN  GT_U8                                        devNum,
    IN  GT_PHYSICAL_PORT_NUM                         portNum,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT               direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_SECTAG_PARSER_STC  *secTagParserPtr
);


/**
* @internal cpssDxChMacSecPortClassifyHdrParserSecTagConfigGet function
* @endinternal
*
* @brief   Get SecTAG parser settings per port from MACSec classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] portNum              - physical port number
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[out] secTagParserPtr     - (pointer to) sectag parser parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecPortClassifyHdrParserSecTagConfigGet
(
    IN  GT_U8                                        devNum,
    IN  GT_PHYSICAL_PORT_NUM                         portNum,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT               direction,
    OUT CPSS_DXCH_MACSEC_CLASSIFY_SECTAG_PARSER_STC  *secTagParserPtr
);


/**
* @internal cpssDxChMacSecPortClassifyHdrParserVlanConfigSet function
* @endinternal
*
* @brief   VLAN parser settings per port in MACSec classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] portNum              - physical port number
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] vlanParserPtr        - (pointer to) VLAN parser parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecPortClassifyHdrParserVlanConfigSet
(
    IN  GT_U8                                      devNum,
    IN  GT_PHYSICAL_PORT_NUM                       portNum,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT             direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_VLAN_PARSER_STC  *vlanParserPtr
);


/**
* @internal cpssDxChMacSecPortClassifyHdrParserVlanConfigGet function
* @endinternal
*
* @brief   Get VLAN parser settings per port from MACSec classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] portNum              - physical port number
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[out] vlanParserPtr       - (pointer to) VLAN parser parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecPortClassifyHdrParserVlanConfigGet
(
    IN  GT_U8                                      devNum,
    IN  GT_PHYSICAL_PORT_NUM                       portNum,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT             direction,
    OUT CPSS_DXCH_MACSEC_CLASSIFY_VLAN_PARSER_STC  *vlanParserPtr
);


/**
* @internal cpssDxChMacSecClassifyStatisticsConfigSet function
* @endinternal
*
* @brief   Statistics control settings per MACSec classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2; CPSS_DATA_PATH_UNAWARE_MODE_CNS.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - use value of CPSS_DATA_PATH_UNAWARE_MODE_CNS to select all data paths.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] statisticsCfgPtr     - (pointer to) statistics control parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecClassifyStatisticsConfigSet
(
    IN  GT_U8                                             devNum,
    IN  GT_MACSEC_UNIT_BMP                                unitBmp,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_CONTROL_STC  *statisticsCfgPtr
);


/**
* @internal cpssDxChMacSecClassifyStatisticsConfigGet function
* @endinternal
*
* @brief   Get statistics control settings per MACSec classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2; CPSS_DATA_PATH_UNAWARE_MODE_CNS.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - use value of CPSS_DATA_PATH_UNAWARE_MODE_CNS to select all data paths.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[out] statisticsCfgPtr    - (pointer to) statistics control parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecClassifyStatisticsConfigGet
(
    IN  GT_U8                                             devNum,
    IN  GT_MACSEC_UNIT_BMP                                unitBmp,
    OUT CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_CONTROL_STC  *statisticsCfgPtr
);


/**
* @internal cpssDxChMacSecPortSecyConfigSet function
* @endinternal
*
* @brief  Port configuration settings in MACSec Transformer
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] portNum              - physical port number
* @param[in] direction            - select Egress or Ingress MACSec transformer
* @param[in] portCfgPtr           - (pointer to) channel configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecPortSecyConfigSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT      direction,
    IN  CPSS_DXCH_MACSEC_SECY_PORT_CFG_STC  *portCfgPtr
);


/**
* @internal cpssDxChMacSecPortSecyConfigGet function
* @endinternal
*
* @brief   Get port settings specified per port from Transformer device
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] portNum              - physical port number
* @param[in] direction            - select Egress or Ingress MACSec transformer
* @param[out] portCfgPtr          - (pointer to) port configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecPortSecyConfigGet
(
    IN  GT_U8                                devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT       direction,
    OUT CPSS_DXCH_MACSEC_SECY_PORT_CFG_STC   *portCfgPtr
);


/**
* @internal cpssDxChMacSecSecyStatisticsConfigSet function
* @endinternal
*
* @brief   Statistics control settings in MACSec Transformer
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2; CPSS_DATA_PATH_UNAWARE_MODE_CNS.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - use value of CPSS_DATA_PATH_UNAWARE_MODE_CNS to select all data paths.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] statisticsCtrlPtr    - (pointer to) statistic configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecSecyStatisticsConfigSet
(
    IN  GT_U8                                          devNum,
    IN  GT_MACSEC_UNIT_BMP                             unitBmp,
    IN  CPSS_DXCH_MACSEC_SECY_STATISTICS_CONTROL_STC   *statisticsCtrlPtr
);


/**
* @internal cpssDxChMacSecSecyStatisticsConfigGet function
* @endinternal
*
* @brief   Get statistics control settings from MACSec Transformer
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2; CPSS_DATA_PATH_UNAWARE_MODE_CNS.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - use value of CPSS_DATA_PATH_UNAWARE_MODE_CNS to select all data paths.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[out] statisticsCtrlPtr   - (pointer to) statistic configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecSecyStatisticsConfigGet
(
    IN  GT_U8                                          devNum,
    IN  GT_MACSEC_UNIT_BMP                             unitBmp,
    OUT CPSS_DXCH_MACSEC_SECY_STATISTICS_CONTROL_STC   *statisticsCtrlPtr
);



/**********************************
 vPort and SA APIs
 **********************************/


/**
* @internal cpssDxChMacSecClassifyVportAdd function
* @endinternal
*
* @brief   Adds a new vPort (vPort policy) set for one MACSec Classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] vPortParamsPtr       - (pointer to) data for the new vPort
* @param[out] vPortHandlePtr      - (pointer to) vPort handle
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecClassifyVportAdd
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     *vPortParamsPtr,
    OUT CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  *vPortHandlePtr
);


/**
* @internal cpssDxChMacSecSecySaAdd function
* @endinternal
*
* @brief   Adds new SA set for one MACSec Transformer
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec transformer
* @param[in] vPortId              - vPort number to which the SA applies.
*                                   (APPLICABLE RANGES: AC5P 0..255)
*                                   (APPLICABLE RANGES: AC5X; Harrier; Ironman 0..63)
* @param[in] saParamsPtr          - pointer to a memory location where the data for the new SA is stored
* @param[in] trRecParamsPtr       - (pointer to) SecY Transform Record parameters
*                                    Ignored for bypass and drop actions
* @param[out] saHandlePtr         - (pointer to) SA handle
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecSecySaAdd
(
    IN  GT_U8                               devNum,
    IN  GT_MACSEC_UNIT_BMP                  unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT      direction,
    IN  GT_U32                              vPortId,
    IN  CPSS_DXCH_MACSEC_SECY_SA_STC        *saParamsPtr,
    IN  CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC *trRecParamsPtr,
    OUT CPSS_DXCH_MACSEC_SECY_SA_HANDLE     *saHandlePtr
);


/**
* @internal cpssDxChMacSecSecySaChainSet function
* @endinternal
*
* @brief   Adds a new SA transform record and chain it to the current active SA
*
*   NOTEs: - This API should be used on the Egress direction only
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] activeSaHandle       - SA handle of the current active SA
* @param[in] newTrRecParamsPtr    - (pointer to) new Transform Record parameters
* @param[out] newSaHandlePtr      - (pointer to) new SA handle
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecySaChainSet
(
    IN  GT_U8                               devNum,
    IN  GT_MACSEC_UNIT_BMP                  unitBmp,
    IN  CPSS_DXCH_MACSEC_SECY_SA_HANDLE     activeSaHandle,
    IN  CPSS_DXCH_MACSEC_SECY_TR_PARAMS_STC *newTrRecParamsPtr,
    OUT CPSS_DXCH_MACSEC_SECY_SA_HANDLE     *newSaHandlePtr
);


/**
* @internal cpssDxChMacSecSecySaChainGet function
* @endinternal
*
* @brief   Return the SA handle of the SA that was chained to the given SA.
*          Return SecY_SAHandle_NULL if no SA handle is chained to the given SA handle.
*
*   NOTEs: - This API should be used on the Egress direction only
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] currentSaHandle      - SA handle of the SA for which to find the chained SA
* @param[out] nextSaHandlePtr     - (pointer to) SA handle of the chained SA
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecySaChainGet
(
    IN  GT_U8                               devNum,
    IN  GT_MACSEC_UNIT_BMP                  unitBmp,
    IN  CPSS_DXCH_MACSEC_SECY_SA_HANDLE     currentSaHandle,
    OUT CPSS_DXCH_MACSEC_SECY_SA_HANDLE     *nextSaHandlePtr
);


/**
* @internal cpssDxChMacSecClassifyVportRemove function
* @endinternal
*
* @brief   Removes an already added vPort (vPort policy) from one MACSec classifier
*
*   NOTEs: - vPort can be removed from MACSec classifier instance only after it has been added to it
*          - Before this function can be called all the Rules associated with this vPort must be removed.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] vPortHandle          - the vPort handle to remove
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyVportRemove
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle
);


/**
* @internal cpssDxChMacSecSecySaRemove function
* @endinternal
*
* @brief   Removes SA that was already added from one MACSec Transformer
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec transformer
* @param[in] saHandle             - the SA handle to remove
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecySaRemove
(
    IN  GT_U8                             devNum,
    IN  GT_MACSEC_UNIT_BMP                unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT    direction,
    IN  CPSS_DXCH_MACSEC_SECY_SA_HANDLE   saHandle
);


/**
* @internal cpssDxChMacSecClassifyVportSet function
* @endinternal
*
* @brief   Updates a vPort (vPort policy) for an already added vPort for a MACSec Classifier instance
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] vPortHandle          - vPort handle for vPort to be updated.
* @param[in] vPortParamsPtr       - (pointer to) data for the vPort.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyVportSet
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     *vPortParamsPtr
);


/**
* @internal cpssDxChMacSecClassifyVportGet function
* @endinternal
*
* @brief   Read the vPort policy of the given vPort from the required MACSec Classifer
*   NOTEs: - A vPort can be read from a MACSec classifier instance only after this vPort has been added to it
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in]  vPortHandle         - vPort handle for vPort to be updated
* @param[out] vPortDataPtr        - (pointer to) data for the vPort.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyVportGet
(
    IN   GT_U8                                   devNum,
    IN   GT_MACSEC_UNIT_BMP                      unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN   CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle,
    OUT  CPSS_DXCH_MACSEC_CLASSIFY_VPORT_STC     *vPortDataPtr
);


/**
* @internal cpssDxChMacSecSecySaSet function
* @endinternal
*
* @brief   Updates SA flow parameters for an already added SA for a MACSec Transformer instance
*
*   NOTEs: This API doesn't update the SA transform record data and can't be used to update the key in the transform record
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec transformer
* @param[in] saHandle             - SA handle of the SA to be updated.
* @param[in] saParamsPtr          - (pointer to) new parameters for the SA.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecySaSet
(
    IN  GT_U8                             devNum,
    IN  GT_MACSEC_UNIT_BMP                unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT    direction,
    IN  CPSS_DXCH_MACSEC_SECY_SA_HANDLE   saHandle,
    IN  CPSS_DXCH_MACSEC_SECY_SA_STC      *saParamsPtr
);


/**
* @internal cpssDxChMacSecSecySaGet function
* @endinternal
*
* @brief   Read the SA flow parameters for an already added SA for a MACSec Transformer instance.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec transformer
* @param[in]  saHandle            - SA handle of the SA to be read.
* @param[out] saParamsPtr         - (pointer to) parameters of the SA
* @param[out] vPortIdPtr          - (pointer to) variable represent the vPort associated with the SA
* @param[out] sciPtr              - (pointer to) SCI of an inbound SA.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecySaGet
(
    IN   GT_U8                              devNum,
    IN   GT_MACSEC_UNIT_BMP                 unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT     direction,
    IN   CPSS_DXCH_MACSEC_SECY_SA_HANDLE    saHandle,
    OUT  CPSS_DXCH_MACSEC_SECY_SA_STC       *saParamsPtr,
    OUT  GT_U32                             *vPortIdPtr,
    OUT  GT_U8                              sciArr[CPSS_DXCH_MACSEC_SECY_SCI_SIZE_CNS]
);


/**
* @internal cpssDxChMacSecClassifyVportIndexGet function
* @endinternal
*
* @brief   Get the vPort index from an existing vPort, using the vPort handle
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number
* @param[in]  vPortHandle         - this handle is a reference to a vPort
* @param[out] vPortIndexPtr       - (pointer to) vPort index
*
* @retval GT_OK                   - on success
* @retval GT_BAD_PARAM            - on wrong parameter
* @retval GT_BAD_PTR              - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED      - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyVportIndexGet
(
    IN   GT_U32                                  devNum,
    IN   CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle,
    OUT  GT_U32                                  *vPortIndexPtr
);


/**
* @internal cpssDxChMacSecClassifyVportHandleGet function
* @endinternal
*
* @brief   Get the vPort handle from an existing vPort, using the vPort index.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in]  vPortIndex          - vPort index for which the vPort handle must be returned.
*                                   (APPLICABLE RANGES: AC5P 0..255)
*                                   (APPLICABLE RANGES: AC5X; Harrier; Ironman 0..63)
* @param[out] vPortHandlePtr      - (pointer to) vPort handle.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyVportHandleGet
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  GT_U32                                  vPortIndex,
    OUT CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  *vPortHandlePtr
);


/**
* @internal cpssDxChMacSecSecySaIndexGet function
* @endinternal
*
* @brief   Get the SA index and SC index from an existing SA, using the SA handle.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum               - device number.
* @param[in]  saHandle             - SA handle of the SA to get the index from.
* @param[out] saIndexPtr           - (pointer to) SA index.
* @param[out] scIndexPtr           - (pointer to) SC index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecySaIndexGet
(
    IN   GT_U8                              devNum,
    IN   CPSS_DXCH_MACSEC_SECY_SA_HANDLE    saHandle,
    OUT  GT_U32                             *saIndexPtr,
    OUT  GT_U32                             *scIndexPtr
);


/**
* @internal cpssDxChMacSecSecySaHandleGet function
* @endinternal
*
* @brief   Get the SA handle from an existing SA, using the SA index.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction           - select Egress or Ingress MACSec transformer
* @param[in]  saIndex             - Secure Association index for which the SA handle must be returned.
*                                   (APPLICABLE RANGES: AC5P 0..511)
*                                   (APPLICABLE RANGES: AC5X; Harrier; Ironman 0..127)
* @param[out] saHandlePtr         - (pointer to) SA handle.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecySaHandleGet
(
    IN   GT_U8                              devNum,
    IN   GT_MACSEC_UNIT_BMP                 unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT     direction,
    IN   GT_U32                             saIndex,
    OUT  CPSS_DXCH_MACSEC_SECY_SA_HANDLE    *saHandlePtr
);


/**
* @internal cpssDxChMacSecSecySaActiveGet function
* @endinternal
*
* @brief   For Egress return the handle of the currently active egress SA for a given vPort.
*          For Ingress return the handles of the currently active ingress SA's for a given vPort and SCI.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum              - device number.
* @param[in] unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                  - 1: select DP0, 2: select DP1.
*                                  - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                - for non multi data paths device this parameter is IGNORED.
* @param[in] direction           - select Egress or Ingress MACSec transformer
* @param[in]  vPortId            - vPort number to which the SA applies.
*                                   (APPLICABLE RANGES: AC5P 0..255)
*                                   (APPLICABLE RANGES: AC5X; Harrier; Ironman 0..63)
* @param[in]  sciArr             - (array of) SCI for which the SA is desired (only in Ingress).
* @param[out] activeSaHandleArr  - For Egress: (array of) one currently active egress SA for the given vPort.
*                                  For Ingress: (array of) four currently active ingress SAs for the given vPort and sci,
*                                                one for each AN from 0 to 3. Any SA that is not active is represented by null.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecySaActiveGet
(
    IN  GT_U8                             devNum,
    IN  GT_MACSEC_UNIT_BMP                unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT    direction,
    IN  GT_U32                            vPortId,
    IN  GT_U8                             sciArr[CPSS_DXCH_MACSEC_SECY_SCI_SIZE_CNS],
    OUT CPSS_DXCH_MACSEC_SECY_SA_HANDLE   activeSaHandleArr[CPSS_DXCH_MACSEC_SECY_MAX_ACTIVE_SA_CNS]
);

/**
* @internal cpssDxChMacSecSecySaNextPnUpdate function
* @endinternal
*
* @brief   Updates sequence number of nextPN for a MACSec Transformer instance
*
*   NOTEs: Applies for ingress only
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] saHandle             - SA handle of the SA to be updated.
* @param[in] nextPnLo             - Least significant 32 bits of new NextPN.
*                                   For 32-bit sequence numbers it is the entire sequence number.
* @param[in] nextPnHi             - Most significant 32 bits of new NextPN in case of 64-bit sequnce numbers
*                                   Must be cleared to zero for 32-bit sequence numbers
* @param[out] nextPnWrittenPtr    - (pointer to) flag to indicate that NextPN was actually written.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecySaNextPnUpdate
(
    IN  GT_U8                             devNum,
    IN  GT_MACSEC_UNIT_BMP                unitBmp,
    IN  CPSS_DXCH_MACSEC_SECY_SA_HANDLE   saHandle,
    IN  GT_U32                            nextPnLo,
    IN  GT_U32                            nextPnHi,
    OUT GT_BOOL                           *nextPnWrittenPtr
);

/**
* @internal cpssDxChMacSecSecySaNextPnGet function
* @endinternal
*
* @brief   Reads the nextPN field for a specific SA
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  saHandle            - SA handle of the SA to be updated.
* @param[in]  direction           - select Egress or Ingress MACSec transformer
* @param[out] nextPnLoPtr         - (pointer to) least significant 32 bits of nextPN.
*                                   For 32-bit sequence numbers it is the entire sequence number.
* @param[out] nextPnHiPtr         - (pointer to) most significant 32 bits of nextPN in case of 64-bit sequence numbers
*                                   Will be cleared to zero for 32-bit sequence numbers
* @param[out] extendedPnPtr       - (pointer to) flag to indicate that sequence number is 64-bit.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecySaNextPnGet
(
    IN  GT_U8                             devNum,
    IN  GT_MACSEC_UNIT_BMP                unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT    direction,
    IN  CPSS_DXCH_MACSEC_SECY_SA_HANDLE   saHandle,
    OUT GT_U32                            *nextPnLoPtr,
    OUT GT_U32                            *nextPnHiPtr,
    OUT GT_BOOL                           *extendedPnPtr
);


/**********************************
 Rule APIs
 **********************************/

/**
* @internal cpssDxChMacSecClassifyRuleAdd function
* @endinternal
*
* @brief   Adds a new rule for matching a packet to a vPort identified by vPortHandle
*          for one MACSec classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction           - select Egress or Ingress MACSec classifier
* @param[in]  vPortHandle         - vPort handle for the vPort where the packet matching rule must be added.
* @param[in]  ruleParamsPtr       - (pointer to) data for the rule.
* @param[out] ruleHandlePtr       - (pointer to) rule handle.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyRuleAdd
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC      *ruleParamsPtr,
    OUT CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   *ruleHandlePtr
);


/**
* @internal cpssDxChMacSecClassifyRuleRemove function
* @endinternal
*
* @brief   Removes an already added rule from one MACSec classifier instance
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] ruleHandle           - rule handle for the rule to be removed.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyRuleRemove
(
    IN  GT_U8                                  devNum,
    IN  GT_MACSEC_UNIT_BMP                     unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT         direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE  ruleHandle
);


/**
* @internal cpssDxChMacSecClassifyRuleSet function
* @endinternal
*
* @brief   Updates an existing packet matching rule for one MACSec classifier instance
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction           - select Egress or Ingress MACSec classifier
* @param[in]  vPortHandle         - vPort handle for the vPort where the packet matching rule must be added.
* @param[in]  ruleHandle          - Rule handle for rule to be updated.
* @param[in]  ruleParamsPtr       - (pointer to) data for the rule.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyRuleSet
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  vPortHandle,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandle,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC      *ruleParamsPtr
);


/**
* @internal cpssDxChMacSecClassifyRuleGet function
* @endinternal
*
* @brief   Read an existing packet matching rule from one MACSec classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction           - select Egress or Ingress MACSec classifier
* @param[in]  ruleHandle          - rule handle for rule to be read.
* @param[out] vPortHandlePtr      - (pointer to) vPort handle that is associated with current rule.
* @param[out] ruleDataPtr         - (pointer to) data for the rule.
* @param[out] ruleEnablePtr       - (pointer to) flag indicating whether the rule was enabled.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyRuleGet
(
    IN   GT_U8                                   devNum,
    IN   GT_MACSEC_UNIT_BMP                      unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN   CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandle,
    OUT  CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE  *vPortHandlePtr,
    OUT  CPSS_DXCH_MACSEC_CLASSIFY_RULE_STC      *ruleDataPtr,
    OUT  GT_BOOL                                 *ruleEnablePtr
);


/**
* @internal cpssDxChMacSecClassifyRuleEnable function
* @endinternal
*
* @brief   Enables or disable an existing rule in one MACSec classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] ruleHandle           - Rule handle for rule to be enabled/disabled.
* @param[in] enable               - GT_TRUE for enable or GT_FALSE for disable rule
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyRuleEnable
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandle,
    IN  GT_BOOL                                 enable
);


/**
* @internal cpssDxChMacSecClassifyRuleEnableDisable function
* @endinternal
*
* @brief   This API offers a combined functionality to be performed with a single write action on an existing
*          rules in one MACSec Classifier device
*          - Enabling or disabling of an entry
*          - Enabling one and disabling another entry at the same time
*          - Enable or disable all entries.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] ruleHandleEnable     - rule handle for rule to be enabled. Will be ignored if equal to 0.
* @param[in] ruleHandleDisable    - rule handle for rule to be disabled. Will be ignored if equal to 0.
* @param[in] enableAll            - When set to true all rules will be enabled. Takes precedence over the other parameters.
* @param[in] disableAll           - When set to true all rules will be disabled. Takes precedence over the other parameters, except enableAll.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyRuleEnableDisable
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandleEnable,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandleDisable,
    IN  GT_BOOL                                 enableAll,
    IN  GT_BOOL                                 disableAll
);


/**
* @internal cpssDxChMacSecClassifyRuleIndexGet function
* @endinternal
*
* @brief   Get the Rule index from an existing Rule, using the rule handle.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum               - device number.
* @param[in]  ruleHandle           - rule handle of the rule to get the index from.
* @param[out] ruleIndexPtr         - (pointer to) rule index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyRuleIndexGet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   ruleHandle,
    OUT GT_U32                                  *ruleIndexPtr
);


/**
* @internal cpssDxChMacSecClassifyRuleHandleGet function
* @endinternal
*
* @brief   Get the Rule handle from an existing Rule, using the Rule index.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] ruleIndex            - rule index for which the Rule handle must be returned.
*                                   (APPLICABLE RANGES: AC5P 0..511)
*                                   (APPLICABLE RANGES: AC5X; Harrier; Ironman 0..127)
* @param[out] ruleHandlePtr       - (pointer to) rule handle.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyRuleHandleGet
(
    IN  GT_U8                                   devNum,
    IN  GT_MACSEC_UNIT_BMP                      unitBmp,
    IN  CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN  GT_U32                                  ruleIndex,
    OUT CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE   *ruleHandlePtr
);



/**********************************
 Statistics APIs
 **********************************/

/**
* @internal cpssDxChMacSecClassifyStatisticsTcamHitsGet function
* @endinternal
*
* @brief   This API reads the MACSec classifier TCAM hits statistics.
*          TCAM Hit counter per each TCAM entry using the TCAM statistics module.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                 - device number.
* @param[in]  unitBmp                - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                       - 1: select DP0, 2: select DP1.
*                                       - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                       - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction              - select Egress or Ingress MACSec classifier
* @param[in]  ruleId                 - The statistics counter index (number) to read the statistics for.
*                                      (APPLICABLE RANGES: AC5P 0..511)
*                                      (APPLICABLE RANGES: AC5X; Harrier; Ironman 0..127)
* @param[in]  syncEnable             - If true, ensure that all packets available in this MACSec unit at the time
*                                      of the API call are processed before the statistics are read
* @param[out] statTcamHitsCounterPtr - (pointer to) classifier Statistics counter per TCAM entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyStatisticsTcamHitsGet
(
    IN   GT_U8                            devNum,
    IN   GT_MACSEC_UNIT_BMP               unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT   direction,
    IN   GT_U32                           ruleId,
    IN   GT_BOOL                          syncEnable,
    OUT  GT_U64                           *statTcamHitsCounterPtr
);


/**
* @internal cpssDxChMacSecPortClassifyStatisticsGet function
* @endinternal
*
* @brief   This API reads the MACSec classifier per-port statistics.
*          Port counters are counting events that for debugging purpose should be accounted per port.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  portNum             - The physical port number to read the statistics for.
* @param[in]  direction           - select Egress or Ingress MACSec classifier
* @param[in]  syncEnable          - If true, ensure that all packets available in this MACSec unit at the time
*                                   of the API call are processed before the statistics are read
* @param[out] portStatPtr         - (pointer to) Classifier statistics counter identified by portNum.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecPortClassifyStatisticsGet
(
    IN   GT_U8                                          devNum,
    IN   GT_PHYSICAL_PORT_NUM                           portNum,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT                 direction,
    IN   GT_BOOL                                        syncEnable,
    OUT  CPSS_DXCH_MACSEC_CLASSIFY_STATISTICS_PORT_STC  *portStatPtr
);


/**
* @internal cpssDxChMacSecSecyStatisticsSaGet function
* @endinternal
*
* @brief   Reads egress or ingress SA statistics from a MACSec Transformer
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec transformer
* @param[in]  saHandle            - SA handle of the SA for which the statistics must be read.
* @param[in]  syncEnable          - If true, ensure that all packets available in this MACSec unit at the time
*                                   of the API call are processed before the statistics are read
* @param[out] statSaPtr           - (pointer to) egress or ingress SA statistics.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecyStatisticsSaGet
(
    IN   GT_U8                              devNum,
    IN   GT_MACSEC_UNIT_BMP                 unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT     direction,
    IN   CPSS_DXCH_MACSEC_SECY_SA_HANDLE    saHandle,
    IN   GT_BOOL                            syncEnable,
    OUT  CPSS_DXCH_SECY_MACSEC_STAT_SA_UNT  *statSaPtr
);


/**
* @internal cpssDxChMacSecSecyStatisticsSecyGet function
* @endinternal
*
* @brief   Reads egress or ingress SecY statistics from a MACSec Transformer
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec transformer
* @param[in]  vPort               - vPort for which the SecY statistics must be read.
*                                   (APPLICABLE RANGES: AC5P 0..255)
*                                   (APPLICABLE RANGES: AC5X; Harrier; Ironman 0..63)
* @param[in]  syncEnable          - If true, ensure that all packets available in this MACSec unit at the time
*                                   of the API call are processed before the statistics are read
* @param[out] statSecyPtr         - (pointer to) egress or ingress SecY statistics.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecyStatisticsSecyGet
(
    IN   GT_U8                                devNum,
    IN   GT_MACSEC_UNIT_BMP                   unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT       direction,
    IN   GT_U32                               vPort,
    IN   GT_BOOL                              syncEnable,
    OUT  CPSS_DXCH_SECY_MACSEC_STAT_SECY_UNT  *statSecyPtr
);


/**
* @internal cpssDxChMacSecSecyStatisticsIfcGet function
* @endinternal
*
* @brief   Reads egress or ingress interface statistics from a MACSec Transformer
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec transformer
* @param[in]  vPort               - vPort for which the SecY statistics must be read.
*                                   (APPLICABLE RANGES: AC5P 0..255)
*                                   (APPLICABLE RANGES: AC5X; Harrier; Ironman 0..63)
* @param[in]  syncEnable          - If true, ensure that all packets available in this MACSec unit at the time
*                                   of the API call are processed before the statistics are read
* @param[out] statIfcPtr          - (pointer to) egress or ingress interface statistics.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecyStatisticsIfcGet
(
    IN   GT_U8                               devNum,
    IN   GT_MACSEC_UNIT_BMP                  unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT      direction,
    IN   GT_U32                              vPort,
    IN   GT_BOOL                             syncEnable,
    OUT  CPSS_DXCH_MACSEC_SECY_STAT_IFC_UNT  *statIfcPtr
);


/**
* @internal cpssDxChMacSecSecyStatisticsRxCamGet function
* @endinternal
*
* @brief   Reads RxCAM statistics (ingress only) from a MACSec Transformer
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  scIndex             - Index of the Secure Channel for which the RxCAM statistics must be read
*                                   (APPLICABLE RANGES: AC5P 0..255)
*                                   (APPLICABLE RANGES: AC5X; Harrier; Ironman 0..63)
* @param[in]  syncEnable          - If true, ensure that all packets available in this MACSec unit at the time
*                                   of the API call are processed before the statistics are read
* @param[out] statRxCamCounterPtr - (pointer to) ingress RxCAM statistics.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecyStatisticsRxCamGet
(
    IN   GT_U8                devNum,
    IN   GT_MACSEC_UNIT_BMP   unitBmp,
    IN   GT_U32               scIndex,
    IN   GT_BOOL              syncEnable,
    OUT  GT_U64               *statRxCamCounterPtr
);


/**
* @internal cpssDxChMacSecSecyStatisticsVportClear function
* @endinternal
*
* @brief   Clear the SecY and IFC statistics counters belonging to a given vPort
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum              - device number.
* @param[in] unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction           - select Egress or Ingress MACSec transformer
* @param[in] vPort               - vPort number for which to clear the statistics
*                                   (APPLICABLE RANGES: AC5P 0..255)
*                                   (APPLICABLE RANGES: AC5X; Harrier; Ironman 0..63)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecyStatisticsVportClear
(
    IN   GT_U8                            devNum,
    IN   GT_MACSEC_UNIT_BMP               unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT   direction,
    IN   GT_U32                           vPort
);



/**********************************
 Debug APIs
 **********************************/

/**
* @internal cpssDxChMacSecClassifyLimitsGet function
* @endinternal
*
* @brief   Returns maximum number of ports,vPorts and TCAM rules for MACSec classifier
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[out] maxPortNumPtr       - (pointer to) maximum number of ports. Skipped if the pointer is NULL.
* @param[out] maxVportNumPtr      - (pointer to) maximum number of vPorts. Skipped if the pointer is NULL.
* @param[out] maxRuleNumPtr       - (pointer to) maximum number of rules. Skipped if the pointer is NULL.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecClassifyLimitsGet
(
    IN  GT_U8                   devNum,
    IN  GT_MACSEC_UNIT_BMP      unitBmp,
    OUT GT_U32                  *maxPortNumPtr,
    OUT GT_U32                  *maxVportNumPtr,
    OUT GT_U32                  *maxRuleNumPtr
);


/**
* @internal cpssDxChMacSecSecyLimitsGet function
* @endinternal
*
* @brief   Returns the maximum number of ports, vPorts, SA and SC for MACSec transformer
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum               - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[out] maxPortNumPtr       - (pointer to) maximum number of ports. Skipped if the pointer is NULL.
* @param[out] maxVportNumPtr      - (pointer to) maximum number of vPorts. Skipped if the pointer is NULL.
* @param[out] maxSaNumPtr         - (pointer to) maximum number of SAs. Skipped if the pointer is NULL.
* @param[out] maxScNumPtr         - (pointer to) maximum number of SCs. Skipped if the pointer is NULL.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - not initialized
*/
GT_STATUS cpssDxChMacSecSecyLimitsGet
(
    IN  GT_U8                   devNum,
    IN  GT_MACSEC_UNIT_BMP      unitBmp,
    OUT GT_U32                  *maxPortNumPtr,
    OUT GT_U32                  *maxVportNumPtr,
    OUT GT_U32                  *maxSaNumPtr,
    OUT GT_U32                  *maxScNumPtr
);


/**
* @internal cpssDxChMacSecSecyGlobalCfgDump function
* @endinternal
*
* @brief   Provide a diagnostics dump which includes configuration setting and status for MACSec transformer
*           - Configuration information
*           - ECC setting and status (ECCCorrectable & ECCUncorrectable counters)
*           - Global packets in-flight status.
*           - Various debug header parser state registers.
*           - Statistics summary status registers.
*           - number of vPorts, SCs, SAs and ports
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec transformer
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecyGlobalCfgDump
(
    IN   GT_U8                            devNum,
    IN   GT_MACSEC_UNIT_BMP               unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT   direction
);


/**
* @internal cpssDxChMacSecPortSecyDump function
* @endinternal
*
* @brief   Provide a diagnostics dump which includes configuration setting and status for MACSec transformer
*          - Port configuration
*          - Packet in-flight status of this port
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec transformer
* @param[in] portNum              - Port physical number for which diagnostics information is desired
* @param[in] allPorts             - Dump the information of all ports instead of the one selected with portNum
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecPortSecyDump
(
    IN   GT_U8                             devNum,
    IN   GT_MACSEC_UNIT_BMP                unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT    direction,
    IN   GT_PHYSICAL_PORT_NUM              portNum,
    IN   GT_BOOL                           allPorts
);


/**
* @internal cpssDxChMacSecSecyVportDump function
* @endinternal
*
* @brief   Provide a diagnostics dump which includes configuration setting and status for MACSec transformer
*          - Per-vPort. Parameters related to the SA which are not in the SA record,
*          - For ingress configuration: all RxCAM entries associated with the vPort and up to four SA indexes
*            associated with each SCI. RxCAM statistics for the entries involved.
*          - For egress configuration: the SA the index of the egress SA plus the index of any chained SA .
*          - Optionally all SA-related information for each of the SAs associated with the vPort.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec transformer
* @param[in] vPortId              - vPort identifier for which diagnostics information is desired
*                                   (APPLICABLE RANGES: AC5P 0..255)
*                                   (APPLICABLE RANGES: AC5X; Harrier; Ironman 0..63)
* @param[in] allvPorts            - Dump the information of all vPorts instead of the one selected with vPortId
* @param[in] includeSa            - Dump the information for all SAs associated with this particular vPort
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecyVportDump
(
    IN   GT_U8                            devNum,
    IN   GT_MACSEC_UNIT_BMP               unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT   direction,
    IN   GT_U32                           vPortId,
    IN   GT_BOOL                          allvPorts,
    IN   GT_BOOL                          includeSa
);


/**
* @internal cpssDxChMacSecSecySaDump function
* @endinternal
*
* @brief   Provide a diagnostics dump which includes configuration setting and status for MACSec transformer
*          - The contents of the SA record.
*          - SA statistics.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec transformer
* @param[in] saHandle             - SA handle for which diagnostics information is desired
* @param[in] allSas               - Dump the information of all SAs instead of the one selected with saHandle.
*                                   saHandle may be a null handle if this parameter is true.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecySaDump
(
    IN   GT_U8                             devNum,
    IN   GT_MACSEC_UNIT_BMP                unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT    direction,
    IN   CPSS_DXCH_MACSEC_SECY_SA_HANDLE   saHandle,
    IN   GT_BOOL                           allSas
);


/**
* @internal cpssDxChMacSecClassifyGlobalCfgDump function
* @endinternal
*
* @brief   Provide a diagnostics dump from MACSec Classifier which includes configuration setting and status for
*          - Configuration information (control packet detection, SecTag parser,vlan parser and statistics).
*          - ECC setting and status (ECCCorrectable & ECCUncorrectable counters)
*          - Various header parser state registers.
*          - Summary status registers.
*          - Number of vPorts, Rules and ports.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyGlobalCfgDump
(
    IN   GT_U8                            devNum,
    IN   GT_MACSEC_UNIT_BMP               unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT   direction
);


/**
* @internal cpssDxChMacSecPortClassifyDump function
* @endinternal
*
* @brief   Provide a diagnostics dump from MACSec Classifier which includes configuration setting and status for
*          - All information that can be obtained per port.
*          - Per port statistics.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] portNum              - Port physical number for which diagnostics information is desired
* @param[in] allPorts             - Dump the information of all channels instead of the one selected with portNum
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecPortClassifyDump
(
    IN   GT_U8                            devNum,
    IN   GT_MACSEC_UNIT_BMP               unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT   direction,
    IN   GT_PHYSICAL_PORT_NUM             portNum,
    IN   GT_BOOL                          allPorts
);


/**
* @internal cpssDxChMacSecClassifyVportDump function
* @endinternal
*
* @brief   Provide a diagnostics dump from MACSec Classifier which includes configuration setting and status for
*          - vPort policy.
*          - List of rules associated with this vPort.
*          - Optionally all information releated to those rules.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] vPortHandle          - vPort handle for which diagnostics information is desired
* @param[in] allvPorts            - Dump the information of all vPorts instead of the one selected with vPortHandle
* @param[in] includeRule          - Dump the information for all Rules associated with this particular vPort.
*                                   vPortHandle may be a null handle if this parameter is true.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyVportDump
(
    IN   GT_U8                                    devNum,
    IN   GT_MACSEC_UNIT_BMP                       unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT           direction,
    IN   CPSS_DXCH_MACSEC_CLASSIFY_VPORT_HANDLE   vPortHandle,
    IN   GT_BOOL                                  allvPorts,
    IN   GT_BOOL                                  includeRule
);


/**
* @internal cpssDxChMacSecClassifyRuleDump function
* @endinternal
*
* @brief   Provide a diagnostics dump from MACSec Classifier which includes configuration setting and status for
*          - The content of the rule (TCAM contents and vPort policy).
*          - flag to indicate whether rule is enabled.
*          - TCAM statistics.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in] unitBmp              - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in] direction            - select Egress or Ingress MACSec classifier
* @param[in] ruleHandle           - Rule handle for which diagnostics information is desired
* @param[in] allRules             - Dump the information of all rules instead of the one selected with ruleHandle
*                                   ruleHandle may be a null handle if this parameter is true.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyRuleDump
(
    IN   GT_U8                                    devNum,
    IN   GT_MACSEC_UNIT_BMP                       unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT           direction,
    IN   CPSS_DXCH_MACSEC_CLASSIFY_RULE_HANDLE    ruleHandle,
    IN   GT_BOOL                                  allRules
);

/**
* @internal cpssDxChMacSecClassifyStatusGet function
* @endinternal
*
* @brief   Reads status information from Classifier unit
*          - Get ECC status
*          - Get parser debug
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction           - select Egress or Ingress MACSec classifier
* @param[in]  getEccStatus        - Set to GT_TRUE to get ECC status information otherwise set to GT_FALSE
* @param[in]  getPktProcessDebug  - Set to GT_TRUE to get packet processing debug information otherwise set to GT_FALSE
* @param[out] deviceStatusPtr     - (pointer to) unit's status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecClassifyStatusGet
(
    IN   GT_U8                                       devNum,
    IN   GT_MACSEC_UNIT_BMP                          unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT              direction,
    IN   GT_BOOL                                     getEccStatus,
    IN   GT_BOOL                                     getPktProcessDebug,
    OUT  CPSS_DXCH_MACSEC_CLASSIFY_UNIT_STATUS_STC   *unitStatusPtr
);

/**
* @internal cpssDxChMacSecSecyStatusGet function
* @endinternal
*
* @brief   Reads status information from Transformer unit
*          - Get ECC status
*          - Get parser debug
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction           - select Egress or Ingress MACSec classifier
* @param[in]  getEccStatus        - Set to GT_TRUE to get ECC status information otherwise set to GT_FALSE
* @param[in]  getPktProcessDebug  - Set to GT_TRUE to get packet processing debug information otherwise set to GT_FALSE
* @param[out] deviceStatusPtr     - (pointer to) unit's status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecyStatusGet
(
    IN   GT_U8                                   devNum,
    IN   GT_MACSEC_UNIT_BMP                      unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    IN   GT_BOOL                                 getEccStatus,
    IN   GT_BOOL                                 getPktProcessDebug,
    OUT  CPSS_DXCH_MACSEC_SECY_UNIT_STATUS_STC   *unitStatusPtr
);

/**
* @internal cpssDxChMacSecSAExpiredSummaryGet function
* @endinternal
*
* @brief  Reads list of SA expired indexes.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                     - device number.
* @param[in]  unitBmp                    - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                          - 1: select DP0, 2: select DP1.
*                                          - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                          - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction                  - select Egress or Ingress MACSec transformer
* @param[out] indexesArr                 - list of SA expired summary indexes, maximum allowed indexes is
*                                          512 for AC5P; Harrier
*                                          128 for AC5X
* @param[out] indexesArrSizePtr          - (pointer to) total count of expired SA summary indexes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSAExpiredSummaryGet
(
    IN   GT_U8                                   devNum,
    IN   GT_MACSEC_UNIT_BMP                      unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    OUT  GT_U32                                  indexesArr[], /*maxArraySize=512*/
    OUT  GT_U32                                  *indexesArrSizePtr
);

/**
* @internal cpssDxChMacSecSAPNThresholdSummaryGet function
* @endinternal
*
* @brief   Reads list of SA indexes where packet number processed have crossed the threshold.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                          - device number.
* @param[in]  unitBmp                         - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                               - 1: select DP0, 2: select DP1.
*                                               - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                               - for non multi data paths device this parameter is IGNORED.
* @param[out] indexesArr                      - list of SA indexes where packet number processed is more that the
*                                               threshold, maximum allowed indexes is
*                                               512 for AC5P; Harrier
*                                               128 for AC5X
* @param[out] indexesArrSizePtr               - (pointer to) total count of SA indexes where packet number
*                                               statistics counter crossed the threshold
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*
* @note Note: Applicable only to Egress MACSec transformer unit
*/
GT_STATUS cpssDxChMacSecSAPNThresholdSummaryGet
(
    IN   GT_U8                                   devNum,
    IN   GT_MACSEC_UNIT_BMP                      unitBmp,
    OUT  GT_U32                                  indexesArr[], /*maxArraySize=512*/
    OUT  GT_U32                                  *indexesArrSizePtr
);

/**
* @internal cpssDxChMacSecSACountSummaryGet function
* @endinternal
*
* @brief   Reads list of SA indexes whose statistics counter have crossed the threshold.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                          - device number.
* @param[in]  unitBmp                         - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                               - 1: select DP0, 2: select DP1.
*                                               - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                             - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction                       - select Egress or Ingress MACSec transformer
* @param[out] indexesArr                      - list of SA indexes whose statistics count is more than the threshold,
*                                               maximum allowed indexes is
*                                               512 for AC5P; Harrier
*                                               128 for AC5X
* @param[out] indexesArrSizePtr               - (pointer to) total count of SA indexes whose statistics has crossed the thteshold
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSACountSummaryGet
(
    IN   GT_U8                                   devNum,
    IN   GT_MACSEC_UNIT_BMP                      unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    OUT  GT_U32                                  indexesArr[], /*maxArraySize=512*/
    OUT  GT_U32                                  *indexesArrSizePtr
);

/**
* @internal cpssDxChMacSecSecYCountSummaryGet function
* @endinternal
*
* @brief   Reads list of SecY vPort indexes whose statistics counter have crossed the threshold.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                             - device number.
* @param[in]  unitBmp                            - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                                  - 1: select DP0, 2: select DP1.
*                                                  - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                                  - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction                          - select Egress or Ingress MACSec transformer
* @param[out] indexesArr                         - list of SecY vPort indexes whose statistics have crossed the threshold,
*                                                  maximum allowed indexes is
*                                                  256 for AC5P; Harrier
*                                                  64 for AC5X
* @param[out] indexesArrSizePtr                  - (pointer to) total count of SecY vPort indexes whose statistics counters crossed the threshold
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecSecYCountSummaryGet
(
    IN   GT_U8                                   devNum,
    IN   GT_MACSEC_UNIT_BMP                      unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    OUT  GT_U32                                  indexesArr[], /*maxArraySize=512*/
    OUT  GT_U32                                  *indexesArrSizePtr
);

/**
* @internal cpssDxChMacSecIfc0CountSummaryGet function
* @endinternal
*
* @brief   Reads list of Ifc0 indexes whose statistics counter have crossed the threshold.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                            - device number.
* @param[in]  unitBmp                           - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                                 - 1: select DP0, 2: select DP1.
*                                                 - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                                 - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction                         - select Egress or Ingress MACSec transformer
* @param[out] indexesArr                        - list of Ifc0 indexes whose statistics have crossed the threshold,
*                                                 maximum allowed indexes is
*                                                 256 for AC5P; Harrier
*                                                 64 for AC5X
* @param[out] indexesArrSizePtr                 - (pointer to) total count of Ifc0 indexes whose statistics counters crossed the threshold
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecIfc0CountSummaryGet
(
    IN   GT_U8                                   devNum,
    IN   GT_MACSEC_UNIT_BMP                      unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    OUT  GT_U32                                  indexesArr[], /*maxArraySize=256*/
    OUT  GT_U32                                  *indexesArrSizePtr
);

/**
* @internal cpssDxChMacSecIfc1CountSummaryGet function
* @endinternal
*
* @brief   Reads list of Ifc1 indexes whose statistics counter have crossed the threshold.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                            - device number.
* @param[in]  unitBmp                           - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                                 - 1: select DP0, 2: select DP1.
*                                                 - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                                 - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction                         - select Egress or Ingress MACSec transformer
* @param[out] indexesArr                        - list of Ifc1 indexes whose statistics have crossed the threshold,
*                                                 maximum allowed indexes is
*                                                 256 for AC5P; Harrier
*                                                 64 for AC5X
* @param[out] indexesArrSizePtr                 - (pointer to) total count of Ifc1 indexes whose statistics counters crossed the threshold
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*
* @note Note: Applicable only to Ingress MACSec transformer unit
*/
GT_STATUS cpssDxChMacSecIfc1CountSummaryGet
(
    IN   GT_U8                                   devNum,
    IN   GT_MACSEC_UNIT_BMP                      unitBmp,
    OUT  GT_U32                                  indexesArr[], /*maxArraySize=256*/
    OUT  GT_U32                                  *indexesArrSizePtr
);

/**
* @internal cpssDxChMacSecRxCamCountSummaryGet function
* @endinternal
*
* @brief   Reads list of RxCAM indexes whose RxCAM hit counter have crossed the threshold.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                             - device number.
* @param[in]  unitBmp                            - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                                  - 1: select DP0, 2: select DP1.
*                                                  - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                                  - for non multi data paths device this parameter is IGNORED.
* @param[out] indexesArr                         - list of RxCam hit indexes whose statistics have crossed the threshold,
*                                                  maximum allowed indexes is
*                                                  256 for AC5P; Harrier
*                                                  64 for AC5X
* @param[out] indexesArrSizePtr                  - (pointer to) total count of RxCam hit counter indexes whose statistics
*                                                  counters have crossed the threshold
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*
* @note Note: Applicable only to Ingress MACSec transformer unit
*/
GT_STATUS cpssDxChMacSecRxCamCountSummaryGet
(
    IN   GT_U8                                   devNum,
    IN   GT_MACSEC_UNIT_BMP                      unitBmp,
    OUT  GT_U32                                  indexesArr[], /*maxArraySize=256*/
    OUT  GT_U32                                  *indexesArrSizePtr
);

/**
* @internal cpssDxChMacSecCfyePortCountSummaryGet function
* @endinternal
*
* @brief   Reads CfyE counter summary registers and returns list of physical ports.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum              - device number.
* @param[in]  unitBmp             - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                   - 1: select DP0, 2: select DP1.
*                                   - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                   - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction           - select Egress or Ingress MACSec classifier
* @param[out] portsArr            - list of physical ports for which summary counters is read, maximum allowed indexes
*                                   is as per the maximum number of physical ports supported in the device
* @param[out] portsArrSizePtr     - (pointer to) total count of physical ports returned
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecCfyePortCountSummaryGet
(
    IN   GT_U8                                              devNum,
    IN   GT_MACSEC_UNIT_BMP                                 unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT                     direction,
    IN   GT_PHYSICAL_PORT_NUM                               portsArr[], /*maxArraySize=128*/
    OUT  GT_U32                                             *portsArrSizePtr
);

/**
* @internal cpssDxChMacSecCfyeTcamCountSummaryGet function
* @endinternal
*
* @brief   Reads TCAM counters whose rule statistics have crossed the threshold.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in]  devNum                            - device number.
* @param[in]  unitBmp                           - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                                 - 1: select DP0, 2: select DP1.
*                                                 - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                                 - for non multi data paths device this parameter is IGNORED.
* @param[in]  direction                         - select Egress or Ingress MACSec classifier
* @param[out] indexArr                          - list of read TCAM counter summary registers whose statistics have crossed
*                                                 the threshold, maximum allowed indexes is
*                                                 512 for AC5P; Harrier
*                                                 128 for AC5X
* @param[out] indexesArrSizePtr                 - (pointer to) total count of tcam counter indexes whose statistics
*                                                 have crossed the threshold
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS cpssDxChMacSecCfyeTcamCountSummaryGet
(
    IN   GT_U8                                   devNum,
    IN   GT_MACSEC_UNIT_BMP                      unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT          direction,
    OUT  GT_U32                                  indexArr[], /*maxArraySize=512*/
    OUT  GT_U32                                  *indexesArrSizePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChMacSech */


