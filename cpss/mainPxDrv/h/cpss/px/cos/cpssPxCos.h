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
* @file cpssPxCos.h
*
* @brief CPSS PX implementation for CoS mapping.
*
* @version   1
********************************************************************************
*/

#ifndef __cpssPxCos_h
#define __cpssPxCos_h

#include <cpss/common/cpssTypes.h>
#include <cpss/px/cpssPxTypes.h>
#include <cpss/common/cos/cpssCosTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @struct CPSS_PX_COS_ATTRIBUTES_STC
 *
 * @brief structure representing CoS Attributes
*/
typedef struct{

    /** @brief (TC) Used to select Priority Queue.
     *  (APPLICABLE RANGE: 0..7)
     */
    GT_U32 trafficClass;

    /** (DP) Used by the egress tail */
    CPSS_DP_LEVEL_ENT dropPrecedence;

    /** @brief (UP) Used by the egress header modification block.
     *  (APPLICABLE RANGE: 0..7)
     */
    GT_U32 userPriority;

    /** @brief (DEI) Used by the egress header modification block.
     *  (APPLICABLE RANGE: 0..1)
     */
    GT_U32 dropEligibilityIndication;

} CPSS_PX_COS_ATTRIBUTES_STC;

/**
* @enum CPSS_PX_COS_MODE_ENT
 *
 * @brief CoS Mode options for obtaining CoS Attributes.
*/
typedef enum{

    /** Use per-port Attributes. */
    CPSS_PX_COS_MODE_PORT_E,

    /** @brief Map CoS Attributes from packet,
     *  using global DSA CoS mapping table.
     */
    CPSS_PX_COS_MODE_PACKET_DSA_E,

    /** @brief Map CoS Attributes from packet,
     *  using perport L2 CoS mapping table.
     */
    CPSS_PX_COS_MODE_PACKET_L2_E,

    /** @brief Map CoS Attributes from packet,
     *  using global L3 mapping table.
     */
    CPSS_PX_COS_MODE_PACKET_L3_E,

    /** @brief Map CoS Attributes from packet,
     *  using global MPLS CoS mapping table.
     */
    CPSS_PX_COS_MODE_PACKET_MPLS_E,

    /** Use perpacket type Attributes in Format Entry. */
    CPSS_PX_COS_MODE_FORMAT_ENTRY_E,

    /** last value (not to be used) */
    CPSS_PX_COS_MODE_LAST_E

} CPSS_PX_COS_MODE_ENT;

/**
* @struct CPSS_PX_COS_FORMAT_ENTRY_STC
 *
 * @brief structure representing one entry of CoS Format Table
*/
typedef struct{

    /** CoS Mode options for obtaining CoS Attributes. */
    CPSS_PX_COS_MODE_ENT cosMode;

    /** @brief Byte offset from anchor.
     *  When cosMode is CPSS_PX_COS_MODE_PACKET_L2_E, anchor is start of packet.
     *  When cosMode is CPSS_PX_COS_MODE_PACKET_L3_E, anchor is start of the L3 header.
     *  When cosMode is CPSS_PX_COS_MODE_PACKET_MPLS_E, anchor is start of outermost MPLS label.
     *  When cosMode is CPSS_PX_COS_MODE_PACKET_DSA_E, anchor is start of packet.
     *  When cosMode is not one of the above, this field is not relevant.
     *  (APPLICABLE RANGE: 0..7)
     */
    GT_U32 cosByteOffset;

    /** @brief Bit offset relative to Byte offset.
     *  (relevant when CoS Mode is 'Packet_xx')
     *  (APPLICABLE RANGE: 0..7)
     */
    GT_U32 cosBitOffset;

    /** @brief Number of CoS bits to extract, starting at the cosByteOffset and cosBitOffset
     *  (relevant when CoS Mode is 'Packet_xx').
     *  (APPLICABLE RANGE: 1..4 when cosMode is CPSS_PX_COS_MODE_PACKET_L2_E)
     *  (APPLICABLE RANGE: 1..6 when cosMode is CPSS_PX_COS_MODE_PACKET_L3_E)
     *  (APPLICABLE RANGE: 1..3 when cosMode is CPSS_PX_COS_MODE_PACKET_MPLS_E)
     *  (APPLICABLE RANGE: 1..7 when cosMode is CPSS_PX_COS_MODE_PACKET_DSA_E)
     */
    GT_U32 cosNumOfBits;

    /** @brief Entry CoS Attributes.
     *  (relevant when CoS Mode is 'Format Entry')
     */
    CPSS_PX_COS_ATTRIBUTES_STC cosAttributes;

} CPSS_PX_COS_FORMAT_ENTRY_STC;

/**
* @internal cpssPxCosFormatEntrySet function
* @endinternal
*
* @brief   Set CoS Mode and Attributes per Packet Type.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] packetType               - Packet Type.
*                                      (APPLICABLE RANGE: 0..31)
* @param[in] cosFormatEntryPtr        - (Pointer to) CoS Format Entry for this packet type.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note packetType is the one obtained from Packet Type lookup.
*
*/
GT_STATUS cpssPxCosFormatEntrySet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  CPSS_PX_PACKET_TYPE                 packetType,
    IN  CPSS_PX_COS_FORMAT_ENTRY_STC        *cosFormatEntryPtr
);

/**
* @internal cpssPxCosFormatEntryGet function
* @endinternal
*
* @brief   Get CoS Mode and Attributes per Packet Type.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] packetType               - Packet Type.
*                                      (APPLICABLE RANGE: 0..31)
*
* @param[out] cosFormatEntryPtr        - (Pointer to) CoS Format Entry for this packet type.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note packetType is the one obtained from Packet Type lookup.
*
*/
GT_STATUS cpssPxCosFormatEntryGet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  CPSS_PX_PACKET_TYPE                 packetType,
    OUT CPSS_PX_COS_FORMAT_ENTRY_STC        *cosFormatEntryPtr
);

/**
* @internal cpssPxCosPortAttributesSet function
* @endinternal
*
* @brief   Set CoS Attributes per Port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] portNum                  - Source port number.
* @param[in] cosAttributesPtr         - (Pointer to) CoS Attributes for this source port.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mapping used on packet types whose CoS mode is 'Port'.
*
*/
GT_STATUS cpssPxCosPortAttributesSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
);

/**
* @internal cpssPxCosPortAttributesGet function
* @endinternal
*
* @brief   Get CoS Attributes per Port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] portNum                  - Source port number.
*
* @param[out] cosAttributesPtr         - (Pointer to) CoS Attributes for this source port.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mapping used on packet types whose CoS mode is 'Port'.
*
*/
GT_STATUS cpssPxCosPortAttributesGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
);

/**
* @internal cpssPxCosPortL2MappingSet function
* @endinternal
*
* @brief   Set CoS L2 Attributes mapping per Port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] portNum                  - Source port number.
* @param[in] l2Index                  - L2 CoS bits extracted from packet.
*                                      (APPLICABLE RANGE: 0..15)
* @param[in] cosAttributesPtr         - (Pointer to) CoS L2 Attributes for this source port.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mapping used on packet types whose CoS mode is 'Packet_L2'.
*
*/
GT_STATUS cpssPxCosPortL2MappingSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U32                      l2Index,
    IN  CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
);

/**
* @internal cpssPxCosPortL2MappingGet function
* @endinternal
*
* @brief   Get CoS L2 Attributes mapping per Port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] portNum                  - Source port number.
* @param[in] l2Index                  - L2 CoS bits extracted from packet.
*                                      (APPLICABLE RANGE: 0..15)
*
* @param[out] cosAttributesPtr         - (Pointer to) CoS L2 Attributes for this source port.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mapping used on packet types whose CoS mode is 'Packet_L2'.
*
*/
GT_STATUS cpssPxCosPortL2MappingGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U32                      l2Index,
    OUT CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
);

/**
* @internal cpssPxCosL3MappingSet function
* @endinternal
*
* @brief   Set CoS Attributes mapping per L3 DSCP.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] l3Index                  - L3 DSCP bits extracted from packet.
*                                      (APPLICABLE RANGE: 0..63)
* @param[in] cosAttributesPtr         - (Pointer to) CoS Attributes for this L3 DSCP index.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mapping used on packet types whose CoS mode is 'Packet_L3'.
*
*/
GT_STATUS cpssPxCosL3MappingSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      l3Index,
    IN  CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
);

/**
* @internal cpssPxCosL3MappingGet function
* @endinternal
*
* @brief   Get CoS Attributes mapping per L3 DSCP.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] l3Index                  - L3 DSCP bits extracted from packet.
*                                      (APPLICABLE RANGE: 0..63)
*
* @param[out] cosAttributesPtr         - (Pointer to) CoS Attributes for this L3 DSCP index.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mapping used on packet types whose CoS mode is 'Packet_L3'.
*
*/
GT_STATUS cpssPxCosL3MappingGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      l3Index,
    OUT CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
);

/**
* @internal cpssPxCosMplsMappingSet function
* @endinternal
*
* @brief   Set CoS Attributes mapping per MPLS EXP.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] mplsIndex                - MPLS EXP bits extracted from packet.
*                                      (APPLICABLE RANGE: 0..7)
* @param[in] cosAttributesPtr         - (Pointer to) CoS Attributes for this MPLS EXP index.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mapping used on packet types whose CoS mode is 'Packet_MPLS'.
*
*/
GT_STATUS cpssPxCosMplsMappingSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      mplsIndex,
    IN  CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
);

/**
* @internal cpssPxCosMplsMappingGet function
* @endinternal
*
* @brief   Get CoS Attributes mapping per MPLS EXP.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] mplsIndex                - MPLS EXP bits extracted from packet.
*                                      (APPLICABLE RANGE: 0..7)
*
* @param[out] cosAttributesPtr         - (Pointer to) CoS Attributes for this MPLS EXP index.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mapping used on packet types whose CoS mode is 'Packet_MPLS'.
*
*/
GT_STATUS cpssPxCosMplsMappingGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      mplsIndex,
    OUT CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
);

/**
* @internal cpssPxCosDsaMappingSet function
* @endinternal
*
* @brief   Set CoS Attributes mapping from DSA header.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] dsaIndex                 - CoS bits extracted from packet DSA header.
*                                      (APPLICABLE RANGE: 0..127)
* @param[in] cosAttributesPtr         - (Pointer to) CoS Attributes for this index.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mapping used on packet types whose CoS mode is 'Packet_DSA'.
*
*/
GT_STATUS cpssPxCosDsaMappingSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      dsaIndex,
    IN  CPSS_PX_COS_ATTRIBUTES_STC  *cosAttributesPtr
);

/**
* @internal cpssPxCosDsaMappingGet function
* @endinternal
*
* @brief   Get CoS Attributes mapping from DSA header.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] dsaIndex                 - CoS bits extracted from packet DSA header.
*                                      (APPLICABLE RANGE: 0..127)
*
* @param[out] cosAttributesPtr         - (Pointer to) CoS Attributes for this index.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mapping used on packet types whose CoS mode is 'Packet_DSA'.
*
*/
GT_STATUS cpssPxCosDsaMappingGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      dsaIndex,
    OUT CPSS_PX_COS_ATTRIBUTES_STC	*cosAttributesPtr
);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPxCos_h */


