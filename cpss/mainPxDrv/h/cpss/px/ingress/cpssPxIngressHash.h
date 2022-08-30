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
* @file cpssPxIngressHash.h
*
* @brief CPSS PX implementation for packet hash calculation.
*
* @version   1
********************************************************************************
*/

#ifndef __cpssPxIngressHash_h
#define __cpssPxIngressHash_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/px/ingress/cpssPxIngress.h>

/**************************** Constants definitions ***************************/
/* Number of user-defined byte pairs for each hash packet type */
#define CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS 21

/* Number of bits to be selected to form hash */
#define CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS 8

/* Entries count in designated port table */
#define CPSS_PX_INGRESS_HASH_DESIGNATED_PORT_TABLE_ENTRIES_MAX_CNS 256

/**************************** Enum definitions ********************************/
/**
* @enum CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT
 *
 * @brief Hash packet type
*/
typedef enum{

    /** IPV4 with encapsulated and not fragmentes TCP or UDP */
    CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_TCP_UDP_E,

    /** IPV6 with encapsulated and not fragmentes TCP or UDP */
    CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV6_TCP_UDP_E,

    /** IPV4 without TCP/UDP or fragmented TCP/UDP */
    CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV4_NO_TCP_UDP_E,

    /** IPV6 without TCP/UDP or fragmented TCP/UDP */
    CPSS_PX_INGRESS_HASH_PACKET_TYPE_IPV6_NO_TCP_UDP_E,

    /** MPLS with single label stack. */
    CPSS_PX_INGRESS_HASH_PACKET_TYPE_MPLS_SINGLE_LABEL_E,

    /** MPLS with at least two labels in the label stack. */
    CPSS_PX_INGRESS_HASH_PACKET_TYPE_MPLS_MULTI_LABEL_E,

    /** User defined ethertype 1. */
    CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E,

    /** User defined ethertype 2. */
    CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE2_E,

    /** In all other cases */
    CPSS_PX_INGRESS_HASH_PACKET_TYPE_ETHERNET_E,

    CPSS_PX_INGRESS_HASH_PACKET_TYPE_LAST_E,

} CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT;

/**
* @enum CPSS_PX_INGRESS_HASH_MODE_ENT
 *
 * @brief Hash calculation mode
*/
typedef enum{

    /** Calculate hash as CRC32 */
    CPSS_PX_INGRESS_HASH_MODE_CRC32_E,

    /** Select 7 bits with user-defined offset from hash key */
    CPSS_PX_INGRESS_HASH_MODE_SELECTED_BITS_E

} CPSS_PX_INGRESS_HASH_MODE_ENT;

/**
* @enum CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT
 *
 * @brief Link aggregation designated port table indexing mode
*/
typedef enum{

    /** use hash for indexing */
    CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_HASH_E,

    /** use pseudo-random number generator for indexing */
    CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_PRNG_E

} CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT;

/**
* @enum CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT
 *
 * @brief The LAG Designated Port Table mode
*/
typedef enum{

    /** @brief single LAG table;
     *  Entries <0..(n1)> where n represents the LAG table size.
     */
    CPSS_PX_INGRESS_HASH_LAG_TABLE_SINGLE_MODE_E,

    /** @brief two tables;
     *  The table is split to two halves:
     *  Entries <0..(n/2 -1)> are in table number 0
     *  while entries <(n/2)..(n-1)> are in table number 1
     *  where n represents the LAG table size
     */
    CPSS_PX_INGRESS_HASH_LAG_TABLE_DUAL_MODE_E

} CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT;

/******************************* Structures definitions ***********************/
/**
* @struct CPSS_PX_INGRESS_HASH_UDBP_STC
 *
 * @brief This structure defines User Defined Bytes Pair for hash configuration.
*/
typedef struct{

    /** Anchor type. */
    CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT anchor;

    /** @brief Offset relative to selected anchor.
     *  (APPLICABLE RANGES: 0..63).
     *  For PIPE revision A0 (APPLICABLE RANGES: 0..15).
     */
    GT_U32 offset;

    GT_BOOL nibbleMaskArr[4];

} CPSS_PX_INGRESS_HASH_UDBP_STC;

/******************************** Function prototypes *************************/
/**
* @internal cpssPxIngressHashUdeEthertypeSet function
* @endinternal
*
* @brief   Set user-defined ethertypes (in addition to predefined types) for hash
*         packet classificator.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] hashPacketType           - hash packet type. (APPLICABLE VALUES:
*                                      CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E;
*                                      CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE2_E)
* @param[in] etherType                - ethertype.
* @param[in] valid                    - validity of the record.
*                                      GT_TRUE  - ethertype is valid.
*                                      GT_FALSE - ethertype is invalid.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressHashUdeEthertypeSet
(
    IN GT_SW_DEV_NUM                        devNum,
    IN CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT hashPacketType,
    IN GT_U16                               etherType,
    IN GT_BOOL                              valid
);

/**
* @internal cpssPxIngressHashUdeEthertypeGet function
* @endinternal
*
* @brief   Get user-defined ethertypes (in addition to predefined types) for hash packet classificator.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] hashPacketType           - hash packet type. (APPLICABLE VALUES:
*                                      CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE1_E;
*                                      CPSS_PX_INGRESS_HASH_PACKET_TYPE_UDE2_E)
*
* @param[out] etherTypePtr             - (pointer to) ethertype.
* @param[out] validPtr                 - (pointer to) validity of the ethertype.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressHashUdeEthertypeGet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT hashPacketType,
    OUT GT_U16                               *etherTypePtr,
    OUT GT_BOOL                              *validPtr
);

/**
* @internal cpssPxIngressHashPacketTypeEntrySet function
* @endinternal
*
* @brief   Sets up packet hash key configuration for selected packet type
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] hashPacketType           - hash packet type.
* @param[in] udbpArr[CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS] - user-defined byte pairs array of 21 elements.
* @param[in] sourcePortEnable         - enable usage of source port in packet hash.
*                                      GT_TRUE  - source port is used in hash
*                                      calculation.
*                                      GT_FALSE - source port is not used in hash
*                                      calculation.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on offset in udbpArr is out of range
*/
GT_STATUS cpssPxIngressHashPacketTypeEntrySet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT hashPacketType,
    IN  CPSS_PX_INGRESS_HASH_UDBP_STC        udbpArr[CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS],
    IN  GT_BOOL                              sourcePortEnable
);

/**
* @internal cpssPxIngressHashPacketTypeEntryGet function
* @endinternal
*
* @brief   Get packet hash key configuration for selected packet type
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] hashPacketType           - hash packet type.
*
* @param[out] udbpArr[CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS] - user-defined byte pairs array of 21 elements.
* @param[out] sourcePortEnablePtr      - (pointer to) enable usage of source port in
*                                      packet hash.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on invalid anchor field read
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressHashPacketTypeEntryGet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT hashPacketType,
    OUT CPSS_PX_INGRESS_HASH_UDBP_STC        udbpArr[CPSS_PX_INGRESS_HASH_UDBP_MAX_CNS],
    OUT GT_BOOL                              *sourcePortEnablePtr
);

/**
* @internal cpssPxIngressHashPacketTypeHashModeSet function
* @endinternal
*
* @brief   Sets up hash calculation mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] hashPacketType           - hash packet type.
* @param[in] hashMode                 - hash calculation mode
* @param[in] bitOffsetsArr[CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS] - array of offsets (8 elements) of hash bits to be
*                                      extracted from 43 bytes of hash key. Relevant only
*                                      when hashMode == 'SELECTED BITS'
*                                      (APPLICABLE RANGES: 0..343)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - bitOffsetsArr member is out of range
*/
GT_STATUS cpssPxIngressHashPacketTypeHashModeSet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT hashPacketType,
    IN  CPSS_PX_INGRESS_HASH_MODE_ENT        hashMode,
    IN  GT_U32                               bitOffsetsArr[CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS]
);

/**
* @internal cpssPxIngressHashPacketTypeHashModeGet function
* @endinternal
*
* @brief   Gets hash calculation mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] hashPacketType           - hash packet type.
*
* @param[out] hashModePtr              - (pointer to) hash calculation mode.
* @param[out] bitOffsetsArr[CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS] - (pointer to) array of offsets (8 elements) of hash
*                                      bits to be extracted from 43 bytes of hash key.
*                                      Relevant only when hashMode == 'SELECTED_BITS'.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressHashPacketTypeHashModeGet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT hashPacketType,
    OUT CPSS_PX_INGRESS_HASH_MODE_ENT        *hashModePtr,
    OUT GT_U32                               bitOffsetsArr[CPSS_PX_INGRESS_HASH_SELECTED_BITS_MAX_CNS]
);

/**
* @internal cpssPxIngressHashSeedSet function
* @endinternal
*
* @brief   Sets up CRC32 seed for hash calculation
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] crc32Seed                - seed for CRC32 packet hash calculation.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressHashSeedSet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  GT_U32                               crc32Seed
);

/**
* @internal cpssPxIngressHashSeedGet function
* @endinternal
*
* @brief   Get CRC32 seed for hash calculation
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] crc32SeedPtr             - (pointer to) seed for CRC32 packet hash
*                                      calculation.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressHashSeedGet
(
    IN  GT_SW_DEV_NUM                        devNum,
    OUT GT_U32                               *crc32SeedPtr
);

/**
* @internal cpssPxIngressHashPortIndexModeSet function
* @endinternal
*
* @brief   Sets up LAG designated port table indexing mode for selected source
*         port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] indexMode                - designated port table indexing mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressHashPortIndexModeSet
(
    IN  GT_SW_DEV_NUM                                                 devNum,
    IN  GT_PHYSICAL_PORT_NUM                                          portNum,
    IN  CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT                      indexMode
);

/**
* @internal cpssPxIngressHashPortIndexModeGet function
* @endinternal
*
* @brief   Gets LAG designated port table indexing mode for selected source
*         port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*
* @param[out] indexModePtr             - designated port table indexing mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressHashPortIndexModeGet
(
    IN  GT_SW_DEV_NUM                                              devNum,
    IN  GT_PHYSICAL_PORT_NUM                                       portNum,
    OUT CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT                   *indexModePtr
);

/**
* @internal cpssPxIngressHashDesignatedPortsEntrySet function
* @endinternal
*
* @brief   Sets entry in LAG designated ports table.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] entryIndex               - entry index
*                                      (APPLICABLE RANGES: 0..255).
* @param[in] portsBmp                 - member port bitmap.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressHashDesignatedPortsEntrySet
(
    IN  GT_SW_DEV_NUM                                            devNum,
    IN  GT_U32                                                   entryIndex,
    IN  CPSS_PX_PORTS_BMP                                        portsBmp
);

/**
* @internal cpssPxIngressHashDesignatedPortsEntryGet function
* @endinternal
*
* @brief   Gets entry in LAG designated ports table.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] entryIndex               - entry index
*                                      (APPLICABLE RANGES: 0..255).
*
* @param[out] portsBmpPtr              - (pointer to) member port bitmap.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxIngressHashDesignatedPortsEntryGet
(
    IN  GT_SW_DEV_NUM                                            devNum,
    IN  GT_U32                                                   entryIndex,
    OUT CPSS_PX_PORTS_BMP                                        *portsBmpPtr
);

/**
* @internal cpssPxIngressHashLagTableModeSet function
* @endinternal
*
* @brief   Sets global LAG Designated Port Table mode
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] lagTableMode             - LAG table mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable starting from revision A1
*
*/
GT_STATUS cpssPxIngressHashLagTableModeSet
(
    IN GT_SW_DEV_NUM                                devNum,
    IN CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT      lagTableMode
);

/**
* @internal cpssPxIngressHashLagTableModeGet function
* @endinternal
*
* @brief   Gets global LAG Designated Port Table mode
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] lagTableModePtr          - (pointer to) LAG table mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable starting from revision A1
*
*/
GT_STATUS cpssPxIngressHashLagTableModeGet
(
    IN GT_SW_DEV_NUM                                devNum,
    OUT CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT    *lagTableModePtr
);

/**
* @internal cpssPxIngressHashPacketTypeLagTableNumberSet function
* @endinternal
*
* @brief   Sets LAG table number for specific packet type
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] packetType               - index of the packet type key in the table.
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] lagTableNumber           - the LAG table number.
*                                      (APPLICABLE RANGES: 0..1)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable starting from revision A1
*
*/
GT_STATUS cpssPxIngressHashPacketTypeLagTableNumberSet
(
    IN GT_SW_DEV_NUM                                devNum,
    IN CPSS_PX_PACKET_TYPE                          packetType,
    IN GT_U32                                       lagTableNumber
);

/**
* @internal cpssPxIngressHashPacketTypeLagTableNumberGet function
* @endinternal
*
* @brief   Gets LAG table number for specific packet type
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] packetType               - index of the packet type key in the table.
*                                      (APPLICABLE RANGES: 0..31)
*
* @param[out] lagTableNumberPtr        - (pointer to) the LAG table number.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Applicable starting from revision A1
*
*/
GT_STATUS cpssPxIngressHashPacketTypeLagTableNumberGet
(
    IN GT_SW_DEV_NUM                                devNum,
    IN CPSS_PX_PACKET_TYPE                          packetType,
    OUT GT_U32                                     *lagTableNumberPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPxIngressHash_h */

