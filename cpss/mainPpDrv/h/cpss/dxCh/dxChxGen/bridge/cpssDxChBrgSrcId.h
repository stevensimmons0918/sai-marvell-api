
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
* @file cpssDxChBrgSrcId.h
*
* @brief CPSS DxCh Source Id facility API
*
* @version   8
********************************************************************************
*/

#ifndef __cpssDxChBrgSrcId
#define __cpssDxChBrgSrcId

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgSrcId.h>

/**
* @struct CPSS_DXCH_BRG_SRC_ID_TARGET_INDEX_CONFIG_STC
 *
 * @brief A structure to hold Source ID as Target Index related configuration.
*/
typedef struct{

    /** @brief The MSB of the Src-ID where the Target index is assigned.
     *  Its the bit offset of the MSB in the SRC-ID
     *  (APPLICABLE RANGES: 0..11)
     */
    GT_U32 srcIdMsb;

    /** @brief The number of bits of the Src-ID where the Target index is assigned.
     *  (APPLICABLE RANGES: 0..6)
     */
    GT_U32 srcIdLength;

    /** @brief Defines the number of bits in the Target ePort that are muxed with Src-ID bits.
     *         The bits always start in the MSB.
     *  (APPLICABLE RANGES: 0..6)
     */
    GT_U32 trgEportMuxLength;

    /** @brief Defines the number of bits in the Target Device that are muxed with Src-ID bits.
     *         The bits always start in the MSB.
     *  (APPLICABLE RANGES: 0..6)
     */
    GT_U32 trgDevMuxLength;

} CPSS_DXCH_BRG_SRC_ID_TARGET_INDEX_CONFIG_STC;

/**
* @internal cpssDxChBrgSrcIdGroupPortAdd function
* @endinternal
*
* @brief   Add a port to Source ID group. Packets assigned with this Source ID
*         may be forwarded to this port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] sourceId                 - Source ID  number that the port is added to
* @param[in] portNum                  - Physical port number, CPU port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum, sourceId
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdGroupPortAdd
(
    IN GT_U8                    devNum,
    IN GT_U32                   sourceId,
    IN GT_PHYSICAL_PORT_NUM     portNum
);

/**
* @internal cpssDxChBrgSrcIdGroupPortDelete function
* @endinternal
*
* @brief   Delete a port from Source ID group. Packets assigned with this
*         Source ID will not be forwarded to this port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] sourceId                 - Source ID  number that the port is deleted from
* @param[in] portNum                  - Physical port number, CPU port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum, sourceId
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdGroupPortDelete
(
    IN GT_U8                    devNum,
    IN GT_U32                   sourceId,
    IN GT_PHYSICAL_PORT_NUM     portNum
);

/**
* @internal cpssDxChBrgSrcIdGroupEntrySet function
* @endinternal
*
* @brief   Set entry in Source ID Egress Filtering table.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] sourceId                 - Source ID  number
* @param[in] cpuSrcIdMember           - GT_TRUE - CPU is member of of Src ID group.
*                                      GT_FALSE - CPU isn't member of of Src ID group.
* @param[in] portsMembersPtr          - pointer to the bitmap of ports that are
*                                      Source ID Members of specified PP device.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, sourceId, ports bitmap value
* @retval GT_BAD_PTR               - portsMembersPtr is NULL pointer
* @retval GT_OUT_OF_RANGE          - length of portsMembersPtr is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdGroupEntrySet
(
    IN GT_U8               devNum,
    IN GT_U32              sourceId,
    IN GT_BOOL             cpuSrcIdMember,
    IN CPSS_PORTS_BMP_STC  *portsMembersPtr
);

/**
* @internal cpssDxChBrgSrcIdGroupEntryGet function
* @endinternal
*
* @brief   Get entry in Source ID Egress Filtering table.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] sourceId                 - Source ID number
*
* @param[out] cpuSrcIdMemberPtr        - GT_TRUE - CPU is member of of Src ID group.
*                                      GT_FALSE - CPU isn't member of of Src ID group.
* @param[out] portsMembersPtr          - pointer to the bitmap of ports are Source ID Members
*                                      of specified PP device.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, sourceId
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdGroupEntryGet
(
    IN  GT_U8               devNum,
    IN  GT_U32              sourceId,
    OUT GT_BOOL             *cpuSrcIdMemberPtr,
    OUT CPSS_PORTS_BMP_STC  *portsMembersPtr
);


/**
* @internal cpssDxChBrgSrcIdPortDefaultSrcIdSet function
* @endinternal
*
* @brief   Configure Port's Default Source ID.
*         The Source ID is used for source based egress filtering.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number, CPU port
* @param[in] defaultSrcId             - Port's Default Source ID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum, defaultSrcId
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdPortDefaultSrcIdSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_U32       defaultSrcId
);

/**
* @internal cpssDxChBrgSrcIdPortDefaultSrcIdGet function
* @endinternal
*
* @brief   Get configuration of Port's Default Source ID.
*         The Source ID is used for Source based egress filtering.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number, CPU port
*
* @param[out] defaultSrcIdPtr          - Port's Default Source ID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdPortDefaultSrcIdGet
(
    IN  GT_U8           devNum,
    IN  GT_PORT_NUM     portNum,
    OUT GT_U32          *defaultSrcIdPtr
);

/**
* @internal cpssDxChBrgSrcIdGlobalUcastEgressFilterSet function
* @endinternal
*
* @brief   Enable or disable Source ID egress filter for unicast
*         packets. The Source ID filter is configured by
*         cpssDxChBrgSrcIdGroupPortAdd and cpssDxChBrgSrcIdGroupPortDelete.
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE -   Source ID filtering on unicast packets.
*                                      Unicast packet is dropped if egress port is not
*                                      member in the Source ID group.
*                                      - GT_FALSE - disable Source ID filtering on unicast packets.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdGlobalUcastEgressFilterSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable
);

/**
* @internal cpssDxChBrgSrcIdGlobalUcastEgressFilterGet function
* @endinternal
*
* @brief   Get Source ID egress filter configuration for unicast
*         packets. The Source ID filter is configured by
*         cpssDxChBrgSrcIdGroupPortAdd and cpssDxChBrgSrcIdGroupPortDelete.
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - GT_TRUE -  enable Source ID filtering on unicast packets.
*                                      Unicast packet is dropped if egress port is not
*                                      member in the Source ID group.
*                                      - GT_FALSE - disable Source ID filtering on unicast packets.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdGlobalUcastEgressFilterGet
(
    IN GT_U8     devNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChBrgSrcIdGlobalSrcIdAssignModeSet function
* @endinternal
*
* @brief   Set Source ID Assignment mode.
*
* @note   APPLICABLE DEVICES:       xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   None.
*
* @param[in] devNum                   - device number
* @param[in] mode                     - the assignment  of the packet Source ID.
*                                      CPSS_BRG_SRC_ID_ASSIGN_MODE_FDB_DA_PORT_DEFAULT_E mode is
*                                      supported for xCat3 and above.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, mode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdGlobalSrcIdAssignModeSet
(
    IN GT_U8                            devNum,
    IN CPSS_BRG_SRC_ID_ASSIGN_MODE_ENT  mode
);

/**
* @internal cpssDxChBrgSrcIdGlobalSrcIdAssignModeGet function
* @endinternal
*
* @brief   Get Source ID Assignment mode.
*
* @note   APPLICABLE DEVICES:       xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   None.
*
* @param[in] devNum                   - device number
*
* @param[out] modePtr                  - the assignment mode of the packet Source ID
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdGlobalSrcIdAssignModeGet
(
    IN  GT_U8                            devNum,
    OUT CPSS_BRG_SRC_ID_ASSIGN_MODE_ENT  *modePtr
);

/**
* @internal cpssDxChBrgSrcIdPortUcastEgressFilterSet function
* @endinternal
*
* @brief   Per Egress Port enable or disable Source ID egress filter for unicast
*         packets. The Source ID filter is configured by
*         cpssDxChBrgSrcIdGroupPortAdd and cpssDxChBrgSrcIdGroupPortDelete.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - Physical port number/all ports wild card, CPU port
* @param[in] enable                   - GT_TRUE -  Source ID filtering on unicast packets
*                                      forwarded to this port. Unicast packet is dropped
*                                      if egress port is not member in the Source ID group.
*                                      - GT_FALSE - disable Source ID filtering on unicast packets
*                                      forwarded to this port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdPortUcastEgressFilterSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_BOOL                  enable
);

/**
* @internal cpssDxChBrgSrcIdPortUcastEgressFilterGet function
* @endinternal
*
* @brief   Get Per Egress Port Source ID egress filter configuration for unicast
*         packets. The Source ID filter is configured by
*         cpssDxChBrgSrcIdGroupPortAdd and cpssDxChBrgSrcIdGroupPortDelete.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - Physical port number/all ports wild card, CPU port
*
* @param[out] enablePtr                - GT_TRUE - enable Source ID filtering on unicast packets
*                                      forwarded to this port. Unicast packet is dropped
*                                      if egress port is not member in the Source ID group.
*                                      - GT_FALSE - disable Source ID filtering on unicast packets
*                                      forwarded to this port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdPortUcastEgressFilterGet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    OUT GT_BOOL                 *enablePtr
);


/**
* @internal cpssDxChBrgSrcIdPortSrcIdForceEnableSet function
* @endinternal
*
* @brief   Set Source ID Assignment force mode per Ingress Port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number, CPU port
* @param[in] enable                   - enable/disable SourceID force mode
*                                      GT_TRUE - enable Source ID force mode
*                                      GT_FALSE - disable Source ID force mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdPortSrcIdForceEnableSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable
);

/**
* @internal cpssDxChBrgSrcIdPortSrcIdForceEnableGet function
* @endinternal
*
* @brief   Get Source ID Assignment force mode per Ingress Port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number, CPU port
*
* @param[out] enablePtr                - (pointer to) Source ID Assignment force mode state
*                                      GT_TRUE - Source ID force mode is enabled
*                                      GT_FALSE - Source ID force mode is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdPortSrcIdForceEnableGet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    OUT GT_BOOL     *enablePtr
);

/**
* @internal cpssDxChBrgSrcIdEtagTypeLocationSet function
* @endinternal
*
* @brief   Set the Source-ID bit which indicates the type of the
*          pushed E-Tag (Unicast E-Tag or Multicast E-Tag). This
*          configuration is used for Unicast Local Switching in
*          802.1BR Port Extender.
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] bitLocation              - bit location
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or bitLocation
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdEtagTypeLocationSet
(
    IN GT_U8        devNum,
    IN GT_U32       bitLocation
);

/**
* @internal cpssDxChBrgSrcIdEtagTypeLocationGet function
* @endinternal
*
* @brief   Get the Source-ID bit which indicates the type of the
*          pushed E-Tag (Unicast E-Tag or Multicast E-Tag). This
*          configuration is used for Unicast Local Switching in
*          802.1BR Port Extender.
*
* @note   APPLICABLE DEVICES:      Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2;
*         Caelum; Aldrin; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
*
* @param[out] bitLocationPtr          - (pointer to) bit location
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdEtagTypeLocationGet
(
    IN GT_U8        devNum,
    OUT GT_U32      *bitLocationPtr
);

/**
* @internal cpssDxChBrgSrcIdAddTag1LocationSet function
* @endinternal
*
* @brief   Set the Source-ID bit which indicates that Tag1 should be added to the.
*        NOTE: Info for 802.1cb
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number
* @param[in] bitLocation              - bit location
*                                       (APPLICABLE VALUES : 0..15)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum or bitLocation
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdAddTag1LocationSet
(
    IN GT_U8        devNum,
    IN GT_U32       bitLocation
);

/**
* @internal cpssDxChBrgSrcIdAddTag1LocationGet function
* @endinternal
*
* @brief   Get the Source-ID bit which indicates that Tag1 should be added to the.
*        NOTE: Info for 802.1cb
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                   - device number
*
* @param[out] bitLocationPtr          - (pointer to) bit location
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdAddTag1LocationGet
(
    IN GT_U8        devNum,
    OUT GT_U32      *bitLocationPtr
);

/**
* @internal cpssDxChBrgSrcIdPortOddOnlyFilterEnableSet function
* @endinternal
*
* @brief   Enable/Disable Source ID filtering for odd Source-ID values.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress ePort number
* @param[in] enable                   - GT_FALSE - Disable drop for odd source ID values.
*                                       GT_TRUE - Drop packets with odd source ID values.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdPortOddOnlyFilterEnableSet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    IN  GT_BOOL          enable
);

/**
* @internal cpssDxChBrgSrcIdPortOddOnlyFilterEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable state of Source ID filtering for odd Source-ID values .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - egress ePort number
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE - Disable drop for odd source ID values.
*                                      GT_TRUE - Drop packets with odd source ID values.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdPortOddOnlyFilterEnableGet
(
    IN  GT_U8            devNum,
    IN  GT_PORT_NUM      portNum,
    OUT GT_BOOL          *enablePtr
);

/**
* @internal cpssDxChBrgSrcIdTargetIndexConfigSet function
* @endinternal
*
* @brief   Set Target Index related configuration.
*
* @note   APPLICABLE DEVICES:       AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] configPtr                - (pointer to) Target Index related configuration.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, parameters
* @retval GT_OUT_OF_RANGE          - on out of range parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdTargetIndexConfigSet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_BRG_SRC_ID_TARGET_INDEX_CONFIG_STC *configPtr
);

/**
* @internal cpssDxChBrgSrcIdTargetIndexConfigGet function
* @endinternal
*
* @brief   Get Target Index related configuration.
*
* @note   APPLICABLE DEVICES:       AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[out] configPtr                - (pointer to) Target Index related configuration.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on different src-id msb in SGT and HA regs
*/
GT_STATUS cpssDxChBrgSrcIdTargetIndexConfigGet
(
    IN GT_U8                            devNum,
    OUT CPSS_DXCH_BRG_SRC_ID_TARGET_INDEX_CONFIG_STC *configPtr
);

/**
* @internal cpssDxChBrgSrcIdPortTargetConfigSet function
* @endinternal
*
* @brief   Configure Target Physical Port's Target Source ID assignment.
*          Target Index is in Source ID field bits defined by cpssDxChBrgSrcIdTargetIndexConfigSet.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] enable                   - enable/disable assignment of Target Index per target physical port
*                                       GT_FALSE - disable assignment
*                                       GT_TRUE  - enable assignment
* @param[in] trgIndex                 - Port's Target Index.
*                                       APPLICABLE RANGES 1..63
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum, trgIndex
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on wrong trgIndex value
*/
GT_STATUS cpssDxChBrgSrcIdPortTargetConfigSet
(
    IN GT_U8                 devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_BOOL               enable,
    IN GT_U32                trgIndex
);

/**
* @internal cpssDxChBrgSrcIdPortTargetConfigGet function
* @endinternal
*
* @brief   Get Target Physical Port's Target Source ID assignment.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[out] enablePtr               - (pointer to) enable/disable assignment of Target Index per target physical port
*                                       GT_FALSE - disable assignment
*                                       GT_TRUE  - enable assignment
* @param[out] trgIndexPtr              - (pointer to) Port's Target Index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdPortTargetConfigGet
(
    IN GT_U8                 devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    OUT GT_BOOL              *enablePtr,
    OUT GT_U32               *trgIndexPtr
);

/**
* @internal cpssDxChBrgSrcIdNotOverrideFromCpuEnableSet function
* @endinternal
*
* @brief   Set Do Not Override FROM_CPU Source-ID mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable/disable Do Not Override FROM_CPU Source-ID mode
*                                      GT_TRUE - enable Do Not Override FROM_CPU Source-ID mode
*                                      GT_FALSE - disable Do Not Override FROM_CPU Source-ID mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdNotOverrideFromCpuEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
);

/**
* @internal cpssDxChBrgSrcIdNotOverrideFromCpuEnableGet function
* @endinternal
*
* @brief   Get status of Do Not Override FROM_CPU Source-ID mode.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[out] enablePtr               - (pointer to) enable/disable status of Do Not Override FROM_CPU Source-ID mode
*                                      GT_TRUE - Do Not Override FROM_CPU Source-ID mode is enabled
*                                      GT_FALSE - Do Not Override FROM_CPU Source-ID mode is disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdNotOverrideFromCpuEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
);

/**
* @internal cpssDxChBrgSrcIdTreatMllReplicatedAsMcEnableSet function
* @endinternal
*
* @brief  Enable/disable Source-ID Egress Filtering for all the multi-destination traffic replicated by the L2MLL.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable/disable Source-ID Egress Filtering for all the multi-destination traffic replicated by L2MLL
*                                      GT_TRUE - If packet was replicated by MLL, Source-ID Egress Filtering considers the
*                                      packet as multicast (even if target is unicast).
*                                      GT_FALSE - If packet has single target destination, Source-ID Egress Filtering treats
*                                      packet as unicast (even if replicated by MLL).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdTreatMllReplicatedAsMcEnableSet
(
    IN GT_U8        devNum,
    IN GT_BOOL      enable
);

/**
* @internal cpssDxChBrgSrcIdTreatMllReplicatedAsMcEnableGet function
* @endinternal
*
* @brief  Get status of enable/disable Source-ID Egress Filtering for all the multi-destination traffic replicated by the L2MLL.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number
* @param[out] enablePtr               - (pointer to) status of Source-ID Egress Filtering for all the multi-destination traffic replicated by L2MLL
*                                      GT_TRUE - If packet was replicated by MLL, Source-ID Egress Filtering considers the
*                                      packet as multicast (even if target is unicast).
*                                      GT_FALSE - If packet has single target destination, Source-ID Egress Filtering treats
*                                      packet as unicast (even if replicated by MLL).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChBrgSrcIdTreatMllReplicatedAsMcEnableGet
(
    IN  GT_U8    devNum,
    OUT GT_BOOL  *enablePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ____cpssDxChBrgSrcId */


