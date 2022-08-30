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
* @file cpssPxIngress.h
*
* @brief CPSS PX implementation for Ingress Processing.
*
* @version   1
********************************************************************************
*/

#ifndef __cpssPxIngress_h
#define __cpssPxIngress_h

#include <cpss/common/cpssTypes.h>
#include <cpss/px/cpssPxTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**************************** Constants definitions *****************************/

/* Maximal number of UDB pairs in packet type key */
#define CPSS_PX_UDB_PAIRS_MAX_CNS                                    4

/* Maximal number of packet bit field offsets in source/destination format table */
#define CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_MAX_CNS              4

/* Number of TPID table entries */
#define CPSS_PX_INGRESS_TPID_TABLE_ENTRIES_MAX_CNS                   4

/* Maximal byte offset of MAC DA in bytes */
#define CPSS_PX_INGRESS_MAC_DA_BYTE_OFFSET_MAX_CNS                  30

/* Number of the packet type key table entries */
#define CPSS_PX_INGRESS_PACKET_TYPE_KEY_TABLE_ENTRIES_MAX_CNS       32

/* Maximal source port profile */
#define CPSS_PX_INGRESS_SOURCE_PORT_PROFILE_MAX_CNS                128

/****************************** Enum definitions ********************************/
/**
* @enum CPSS_PX_INGRESS_ETHERTYPE_ENT
 *
 * @brief Enumeration of packet ethertype
*/
typedef enum{

    /** IPv4 Ethertype */
    CPSS_PX_INGRESS_ETHERTYPE_IPV4_E,

    /** IPv6 Ethertype */
    CPSS_PX_INGRESS_ETHERTYPE_IPV6_E,

    /** MPLS EtherType */
    CPSS_PX_INGRESS_ETHERTYPE_MPLS1_E,

    /** MPLS EtherType2 */
    CPSS_PX_INGRESS_ETHERTYPE_MPLS2_E,

    /** IPv6 extension header */
    CPSS_PX_INGRESS_ETHERTYPE_IPV6_EH_E,

    /** should be last in enumerator */
    CPSS_PX_INGRESS_ETHERTYPE_LAST_E

} CPSS_PX_INGRESS_ETHERTYPE_ENT;

/**
* @enum CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT
 *
 * @brief Enumeration of port map type
*/
typedef enum{

    CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_PORT_FILTERING_E,

    /** the destination port map table related info is used. */
    CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E,

    /** should be last in enumerator */
    CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_LAST

} CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT;

/**
* @enum CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT
 *
 * @brief UDB anchor type.
 * CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E - L2 Anchor - start of packet.
 * CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E - L3 Anchor - start of L3 header.
 * CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L4_E - L4 Anchor - start of L4 header.
 * CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_PTP_E - PTP Anchor - start of PTP header.
*/
typedef enum{

    CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E,

    CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E,

    CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L4_E,

    CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_PTP_E

} CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT;

/********************************* Structures definitions ***********************/
/**
* @struct CPSS_PX_UDB_KEY_STC
 *
 * @brief This structure defines User Defined Byte for packet type key.
*/
typedef struct{

    /** The UDB anchor type in the per */
    CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT udbAnchorType;

    /** @brief Byte offset relative to Anchor Type.
     *  (APPLICABLE RANGES: 0..127)
     *  Comments:
     */
    GT_U32 udbByteOffset;

} CPSS_PX_UDB_KEY_STC;

/**
* @struct CPSS_PX_UDB_PAIR_KEY_STC
 *
 * @brief This structure defines User Defined Bytes Pair for packet type key.
*/
typedef struct{

    /** The UDB anchor type in the per */
    CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT udbAnchorType;

    /** @brief Byte offset relative to Anchor Type.
     *  (APPLICABLE RANGES: 0..63) - Ingress packet classification
     *  (APPLICABLE RANGES: 0..127) - PTP packet classification
     *  Comments:
     */
    GT_U32 udbByteOffset;

} CPSS_PX_UDB_PAIR_KEY_STC;

/**
* @struct CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_STC
 *
 * @brief This structure defines packet bit field offset in source/destination format table.
*/
typedef struct{

    /** @brief The Byte Offset relative to the start of the packet.
     *  (APPLICABLE RANGES: 0..63)
     */
    GT_U32 byteOffset;

    /** @brief The start bit in the byte specified by the Byte Offset.
     *  (APPLICABLE RANGES: 0..7)
     */
    GT_U32 startBit;

    /** @brief The number of bits to take starting from the start bit.
     *  (APPLICABLE RANGES: 0..8)
     *  Comments:
     */
    GT_U32 numBits;

} CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_STC;

/**
* @struct CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC
 *
 * @brief This structure defines port packet type key.
*/
typedef struct{

    /** @brief source port profile
     *  (APPLICABLE RANGES: 0..127)
     */
    GT_U32 srcPortProfile;

    CPSS_PX_UDB_PAIR_KEY_STC portUdbPairArr[CPSS_PX_UDB_PAIRS_MAX_CNS];

} CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC;

/**
* @struct CPSS_PX_UDB_PAIR_DATA_STC
 *
 * @brief This structure defines 2 consecutive UDB bytes.
*/
typedef struct{

    GT_U8 udb[2];

} CPSS_PX_UDB_PAIR_DATA_STC;

/**
* @struct CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC
 *
 * @brief Packet Type Key Table format
*/
typedef struct{

    /** Ethernet Destination MAC address. */
    GT_ETHERADDR macDa;

    /** @brief If packet is not DSA
     *  at immediate after the MAC SA.
     *  If packet is DSA-tagged, this is the EtherType/LEN field
     *  immediately after the DSA tag
     */
    GT_U16 etherType;

    /** @brief If packet is LLC non
     *  matches the LLC DSAP-SSAP
     *  GT_TRUE  - packet is LLC non-SNAP
     *  GT_FALSE - packet is the Ethernet V2 or LLC with SNAP packet.
     */
    GT_BOOL isLLCNonSnap;

    /** @brief source port profile index
     *  (APPLICABLE RANGES: 0..127)
     */
    GT_U32 profileIndex;

    CPSS_PX_UDB_PAIR_DATA_STC udbPairsArr[CPSS_PX_UDB_PAIRS_MAX_CNS];

    /** @brief matching IP2ME index (APPLICABLE RANGES: 0..6).
     *  Applicable starting from revision A1.
     */
    GT_U32 ip2meIndex;

} CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC;

/**
* @struct CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC
 *
 * @brief This structure defines packet type format in Source/Destination format table.
*/
typedef struct{

    CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_STC bitFieldArr[CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_MAX_CNS];

    /** @brief This is a signed constant used to compute the Source/Destination index.
     *  Destination table -
     *  (APPLICABLE RANGES: -4096..+4095)
     *  Source table -
     *  (APPLICABLE RANGES: -2048..2047)
     */
    GT_32 indexConst;

    /** @brief The maximum acceptable value
     *  for the resulting Source/Destination index.
     *  If greater than this value, the packet is forwarded
     *  according to the Exception Forwarding PortMap.
     *  Destination table -
     *  (APPLICABLE RANGES: 0..8191)
     *  Source table -
     *  (APPLICABLE RANGES: 0..4095)
     *  Comments:
     *  The srcTrgIndexConst is used for the following:
     *  Partitioning the Port Map Table into different logical forwarding tables.
     *  For example, UC table and MC table.
     *  Note: a negative constant may be required for BR MC,
     *  where the range of <GRP,E-CID> is <4096, 16383>.
     *  We need a negative offset for the index in the Port Map Table
     *  to locate the MC destinations in the Port Map Table at an index less than 4K.
     *  Forwarding packets with a given MAC DA to the CPU.
     *  For example, MAC2ME or some control protocol MAC address.
     *  In this use case, all the Bit-Fields have <Number-bits>==0,
     *  resulting in srcTrgIndex = srcTrgIndexConst.
     */
    GT_U32 indexMax;

} CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC;

/**
* @struct CPSS_PX_INGRESS_TPID_ENTRY_STC
 *
 * @brief This structure defines global TPID configurations
*/
typedef struct{

    /** TPID value. */
    GT_U16 val;

    /** @brief TPID size.
     *  The units of this input parameter is in bytes.
     *  Only even values are applicable.
     *  (APPLICABLE RANGES: 2..24)
     */
    GT_U32 size;

    /** @brief TPID entry status
     *  GT_TRUE   - TPID is valid,
     *  GT_FALSE  - TPID is not valid
     *  Comments:
     */
    GT_BOOL valid;

} CPSS_PX_INGRESS_TPID_ENTRY_STC;

/**
* @struct CPSS_PX_INGRESS_IP2ME_ENTRY_STC
 *
 * @brief This structure defines IP2ME configurations
*/
typedef struct {
    GT_BOOL     valid;
    GT_U32      prefixLength;
    GT_BOOL     isIpv6;
    union {
        GT_IPADDR   ipv4Addr;
        GT_IPV6ADDR ipv6Addr;
    } ipAddr;
} CPSS_PX_INGRESS_IP2ME_ENTRY_STC;

/********************************* Function prototypes *************************/
/**
* @internal cpssPxIngressTpidEntrySet function
* @endinternal
*
* @brief   Set global TPID table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] entryIndex               - entry index for TPID table (APPLICABLE RANGES: 0..3).
* @param[in] tpidEntryPtr             - (pointer to) global TPID entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressTpidEntrySet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_U32                          entryIndex,
    IN  CPSS_PX_INGRESS_TPID_ENTRY_STC  *tpidEntryPtr
);

/**
* @internal cpssPxIngressTpidEntryGet function
* @endinternal
*
* @brief   Get global TPID table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] entryIndex               - entry index for TPID table (APPLICABLE RANGES: 0..3).
*
* @param[out] tpidEntryPtr             - (pointer to) global TPID entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressTpidEntryGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_U32                          entryIndex,
    OUT CPSS_PX_INGRESS_TPID_ENTRY_STC  *tpidEntryPtr
);

/**
* @internal cpssPxIngressEtherTypeSet function
* @endinternal
*
* @brief   Set global ethertype configuration.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] configType               - type of ethertype to be configured
* @param[in] etherType                - EtherType for protocol recognition.
* @param[in] valid                    - EtherType entry status
*                                      GT_TRUE     - EtherType is valid,
*                                      GT_FALSE    - EtherType is not valid.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressEtherTypeSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_PX_INGRESS_ETHERTYPE_ENT   configType,
    IN  GT_U16                          etherType,
    IN  GT_BOOL                         valid
);

/**
* @internal cpssPxIngressEtherTypeGet function
* @endinternal
*
* @brief   Get global ethertype configuration.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] configType               - type of ethertype
*
* @param[out] etherTypePtr             - (pointer to) EtherType for protocol recognition.
* @param[out] validPtr                 - (pointer to) EtherType entry status
*                                      GT_TRUE     - EtherType is valid,
*                                      GT_FALSE    - EtherType is not valid.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressEtherTypeGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_PX_INGRESS_ETHERTYPE_ENT   configType,
    OUT GT_U16                          *etherTypePtr,
    OUT GT_BOOL                         *validPtr
);

/**
* @internal cpssPxIngressPortMacDaByteOffsetSet function
* @endinternal
*
* @brief   Set byte offset of MAC DA on specified port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
* @param[in] offset                   - byte  of MAC DA.
*                                      Only even values are applicable.
*                                      (APPLICABLE RANGES: 0..30)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPortMacDaByteOffsetSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          offset
);

/**
* @internal cpssPxIngressPortMacDaByteOffsetGet function
* @endinternal
*
* @brief   Get byte offset of MAC DA for specified port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
*
* @param[out] offsetPtr                - (pointer to) byte offset of MAC DA.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPortMacDaByteOffsetGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_U32                          *offsetPtr
);

/**
* @internal cpssPxIngressPortPacketTypeKeySet function
* @endinternal
*
* @brief   Set port packet type key generation info:
*         - source profile
*         - UDB pairs information
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
* @param[in] portKeyPtr               - (pointer to) port packet type key.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPortPacketTypeKeySet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    IN  CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC    *portKeyPtr
);

/**
* @internal cpssPxIngressPortPacketTypeKeyGet function
* @endinternal
*
* @brief   Get packet type key generation info:
*         - source profile
*         - UDB pairs information
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
*
* @param[out] portKeyPtr               - (pointer to) packet type key.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPortPacketTypeKeyGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    OUT CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC    *portKeyPtr
);

/**
* @internal cpssPxIngressPacketTypeKeyEntrySet function
* @endinternal
*
* @brief   Set packet type key table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] packetType               - index of the packet type key in the table.
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] keyDataPtr               - (pointer to) 17 bytes packet type key data.
* @param[in] keyMaskPtr               - (pointer to) 17 bytes packet type key mask.
*                                      The packet type key mask is AND styled one.
*                                      Mask bit's 0 means don't care bit (corresponding bit in
*                                      the pattern is not using in the TCAM like lookup).
*                                      Mask bit's 1 means that corresponding bit in the
*                                      pattern is using in the TCAM like lookup.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPacketTypeKeyEntrySet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PACKET_TYPE                         packetType,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyDataPtr,
    IN  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyMaskPtr
);

/**
* @internal cpssPxIngressPacketTypeKeyEntryGet function
* @endinternal
*
* @brief   Get packet type key table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] packetType               - index of the packet type key in the table.
*                                      (APPLICABLE RANGES: 0..31)
*
* @param[out] keyDataPtr               - (pointer to) 17 bytes packet type key pattern.
* @param[out] keyMaskPtr               - (pointer to) 17 bytes packet type key mask.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPacketTypeKeyEntryGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PACKET_TYPE                         packetType,
    OUT CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyDataPtr,
    OUT CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *keyMaskPtr
);

/**
* @internal cpssPxIngressPortMapPacketTypeFormatEntrySet function
* @endinternal
*
* @brief   Set Source/Destination information for packet type in the format table,
*         used to extract the information from the packet forwarding tag (DSA or E-Tag).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] tableType                - Source/Destination port map table.
* @param[in] packetType               - index of the packet type format.
*                                      in the Source/Destination format table.
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] packetTypeFormatPtr      - (pointer to) Source/Destination packet type format
*                                      in the Source/Destination format table.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPortMapPacketTypeFormatEntrySet
(
    IN  GT_SW_DEV_NUM                                       devNum,
    IN  CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT             tableType,
    IN  CPSS_PX_PACKET_TYPE                                 packetType,
    IN  CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     *packetTypeFormatPtr
);

/**
* @internal cpssPxIngressPortMapPacketTypeFormatEntryGet function
* @endinternal
*
* @brief   Get Source/Destination information for packet type in the format table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] tableType                - Source/Destination port map table.
* @param[in] packetType               - index of the packet type format
*                                      in the Source/Destination format table.
*                                      (APPLICABLE RANGES: 0..31)
*
* @param[out] packetTypeFormatPtr      - (pointer to) Source/Destination packet type format
*                                      in the Source/Destination format table.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPortMapPacketTypeFormatEntryGet
(
    IN  GT_SW_DEV_NUM                                       devNum,
    IN  CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT             tableType,
    IN  CPSS_PX_PACKET_TYPE                                 packetType,
    OUT CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC     *packetTypeFormatPtr
);

/**
* @internal cpssPxIngressPortMapEntrySet function
* @endinternal
*
* @brief   Set Source/Destination port map table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] tableType                - Source/Destination port map table.
* @param[in] entryIndex               - port map entry index.
*                                      Index calculated by packet type source/destination format.
*                                      Destination table -
*                                      (APPLICABLE RANGES: 0..8191)
*                                      Source table -
*                                      (APPLICABLE RANGES: 0..4095)
* @param[in] portsBmp                 - the bitmap of ports.
* @param[in] bypassLagDesignatedBitmap
*                                      - enables bypass of LAG Designated Port selection for this entry.
*                                      Relevant only for destination port map table.
*                                      GT_TRUE     -  The LAG Designated Port selection is bypassed for this entry
*                                      GT_FALSE    -  The LAG Designated Port selection is applied for this entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPortMapEntrySet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT     tableType,
    IN  GT_U32                                      entryIndex,
    IN  CPSS_PX_PORTS_BMP                           portsBmp,
    IN  GT_BOOL                                     bypassLagDesignatedBitmap
);

/**
* @internal cpssPxIngressPortMapEntryGet function
* @endinternal
*
* @brief   Gets Source/Destination port map table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] tableType                - Source/Destination port map table.
* @param[in] entryIndex               - port map entry index.
*                                      Index calculated by packet type source/destination format.
*                                      Destination table -
*                                      (APPLICABLE RANGES: 0..8191)
*                                      Source table -
*                                      (APPLICABLE RANGES: 0..4095)
*
* @param[out] portsBmpPtr              - (pointer to) the bitmap of ports.
* @param[out] bypassLagDesignatedBitmapPtr
*                                      - (pointer to) the bypass status of LAG Designated Port selection for this entry.
*                                      Relevant only for destination port map table.
*                                      GT_TRUE     -  The LAG Designated Port selection is bypassed for this entry
*                                      GT_FALSE    -  The LAG Designated Port selection is applied for this entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPortMapEntryGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT     tableType,
    IN  GT_U32                                      entryIndex,
    OUT CPSS_PX_PORTS_BMP                           *portsBmpPtr,
    OUT GT_BOOL                                     *bypassLagDesignatedBitmapPtr
);

/**
* @internal cpssPxIngressPortTargetEnableSet function
* @endinternal
*
* @brief   Enable/disable forwarding to the target port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
* @param[in] enable                   -  or disable forwarding to the target port.
*                                      GT_TRUE - Enables packet forwarding to the target port,
*                                      GT_FALSE - Filters all traffic to the target port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPortTargetEnableSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         enable
);

/**
* @internal cpssPxIngressPortTargetEnableGet function
* @endinternal
*
* @brief   Get state of the forwarding target port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
*
* @param[out] enablePtr                - (pointer to) state of the target port.
*                                      GT_TRUE - Enables packet forwarding to the target port,
*                                      GT_FALSE - Filters all traffic to the target port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPortTargetEnableGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_BOOL                         *enablePtr
);

/**
* @internal cpssPxIngressPacketTypeErrorGet function
* @endinternal
*
* @brief   Get unmatched packet type error counter and key.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] errorCounterPtr          - (pointer to) unmatched packet type counter.
* @param[out] errorKeyPtr              - (pointer to) unmatched packet type key.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPacketTypeErrorGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    OUT GT_U32                                      *errorCounterPtr,
    OUT CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  *errorKeyPtr
);

/**
* @internal cpssPxIngressForwardingPortMapExceptionSet function
* @endinternal
*
* @brief   Set exception Source/Destination forwarding port map.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] tableType                - Source/Destination port map table.
* @param[in] portsBmp                 - the bitmap of ports.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressForwardingPortMapExceptionSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT     tableType,
    IN  CPSS_PX_PORTS_BMP                           portsBmp
);

/**
* @internal cpssPxIngressForwardingPortMapExceptionGet function
* @endinternal
*
* @brief   Get exception Source/Destination port map.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] tableType                - Source/Destination port map table.
*
* @param[out] portsBmpPtr              - (pointer to) the bitmap of ports.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressForwardingPortMapExceptionGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT     tableType,
    OUT CPSS_PX_PORTS_BMP                           *portsBmpPtr
);

/**
* @internal cpssPxIngressPortMapExceptionCounterGet function
* @endinternal
*
* @brief   Get Source/Destination index exception counter.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] tableType                - Source/Destination port map table.
*
* @param[out] errorCounterPtr          - (pointer to) Source/Destination index exception counter.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPortMapExceptionCounterGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT     tableType,
    OUT GT_U32                                      *errorCounterPtr
);

/**
* @internal cpssPxIngressPacketTypeKeyEntryEnableSet function
* @endinternal
*
* @brief   Enable/disable packet type key table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] packetType               - index of the packet type key in the table.
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] enable                   - Enable or disable packet type key table entry
*                                      GT_TRUE - enable packet type key table entry,
*                                      GT_FALSE - disable packet type key table entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPacketTypeKeyEntryEnableSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PACKET_TYPE                         packetType,
    IN  GT_BOOL                                     enable
);

/**
* @internal cpssPxIngressPacketTypeKeyEntryEnableGet function
* @endinternal
*
* @brief   Get status of the packet type key table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] packetType               - index of the packet type key in the table.
*                                      (APPLICABLE RANGES: 0..31)
*
* @param[out] enablePtr                - (pointer to) state of the packet type key table entry
*                                      GT_TRUE - enable packet type key table entry,
*                                      GT_FALSE - disable packet type key table entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPacketTypeKeyEntryEnableGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PACKET_TYPE                         packetType,
    OUT GT_BOOL                                     *enablePtr
);

/**
* @internal cpssPxIngressPortDsaTagEnableSet function
* @endinternal
*
* @brief   Enable/disable the source port for DSA-tagged packets.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
* @param[in] enable                   -  or disable the source port for DSA-tagged packets
*                                      GT_TRUE   - enable the port for DSA-tagged packets,
*                                      GT_FALSE  - disable the port for DSA-tagged packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPortDsaTagEnableSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         enable
);

/**
* @internal cpssPxIngressPortDsaTagEnableGet function
* @endinternal
*
* @brief   Get DSA-tagged packets receiving status on the source port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
*
* @param[out] enablePtr                - (pointer to) DSA-tagged packets receiving status on the source port
*                                      GT_TRUE   - enable the port for DSA-tagged packets,
*                                      GT_FALSE  - disable the port for DSA-tagged packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPortDsaTagEnableGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_BOOL                         *enablePtr
);

/**
* @internal cpssPxIngressPortRedirectSet function
* @endinternal
*
* @brief   Set redirecting of the all packets from ingress port to list of
*         egress ports.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - ingress physical port number.
* @param[in] targetPortsBmp           - the bitmap of egress ports.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressPortRedirectSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PX_PORTS_BMP               targetPortsBmp
);

/**
* @internal cpssPxIngressFilteringEnableSet function
* @endinternal
*
* @brief   Set port filtering enabled state.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] direction                - direction.
* @param[in] packetType               - packet type.
* @param[in] enable                   -  or disable port filtering
*                                      GT_TRUE   - port filtering is enabled
*                                      GT_FALSE  - port filtering is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable starting from revision A1.
*
*/
GT_STATUS cpssPxIngressFilteringEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  CPSS_DIRECTION_ENT      direction,
    IN  CPSS_PX_PACKET_TYPE     packetType,
    IN  GT_BOOL                 enable
);

/**
* @internal cpssPxIngressFilteringEnableGet function
* @endinternal
*
* @brief   Get port filtering enabled state.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] direction                - direction: ingress or egress.
* @param[in] packetType               - packet type.
*
* @param[out] enablePtr                - (pointer to) enabled state
*                                      GT_TRUE   - port filtering is enabled
*                                      GT_FALSE  - port filtering is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable starting from revision A1.
*
*/
GT_STATUS cpssPxIngressFilteringEnableGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  CPSS_DIRECTION_ENT      direction,
    IN  CPSS_PX_PACKET_TYPE     packetType,
    OUT GT_BOOL                *enablePtr
);

/**
* @internal cpssPxIngressIp2MeEntrySet function
* @endinternal
*
* @brief   Set IP2ME table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] entryIndex               - entry index for IP2ME table (APPLICABLE RANGES: 1..7).
* @param[in] ip2meEntryPtr            - (pointer to) IP2ME entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable starting from revision A1.
*
*/
GT_STATUS cpssPxIngressIp2MeEntrySet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_U32                          entryIndex,
    IN  CPSS_PX_INGRESS_IP2ME_ENTRY_STC *ip2meEntryPtr
);

/**
* @internal cpssPxIngressIp2MeEntryGet function
* @endinternal
*
* @brief   Get IP2ME table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] entryIndex               - entry index for IP2ME table (APPLICABLE RANGES: 1..7).
*
* @param[out] ip2meEntryPtr            - (pointer to) IP2ME entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable starting from revision A1.
*
*/
GT_STATUS cpssPxIngressIp2MeEntryGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_U32                          entryIndex,
    OUT CPSS_PX_INGRESS_IP2ME_ENTRY_STC *ip2meEntryPtr
);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPxIngress_h */

