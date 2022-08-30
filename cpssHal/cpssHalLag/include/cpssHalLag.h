/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/


/**
 ********************************************************************************
 * @file cpssHalLag.h
 * @brief Private API declaration for CPSS Lag functions which can be used in XPS layer.
 *
 * @version   01
 ********************************************************************************
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include  <gtOs/gtGenTypes.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>

/**
  * @internal cpssHalSetTrunkHashCrcParameters function
  * @endinternal
  *
  * @brief   Function Relevant mode : ALL modes
  *         Set the CRC hash parameter , relevant for CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_CRC_E .
  *
  * @note   APPLICABLE DEVICES:      Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
  * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3.
  *
  * @param[in] devId                   - The device Id.
  * @param[in] crcMode                  - The CRC mode .
  * @param[in] crcSeed                  - The seed used by the CRC computation .
  *                                      when crcMode is CRC_6 mode : crcSeed (APPLICABLE RANGES: 0..0x3f) (6 bits value)
  *                                      when crcMode is CRC_16 mode : crcSeed (APPLICABLE RANGES: 0..0xffff) (16 bits value)
  *                                      when crcMode is CRC_32 mode : crcSeed (32 bits value)
  *
  * @retval GT_OK                    - on success
  * @retval GT_FAIL                  - on error
  * @retval GT_HW_ERROR              - on hardware error
  * @retval GT_BAD_PARAM             - bad device Id , or crcMode
  * @retval GT_OUT_OF_RANGE          - crcSeed out of range.
  * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
  *
  * @note related to feature 'CRC hash mode'
  *
  */
GT_STATUS cpssHalSetTrunkHashCrcParameters
(
    int                                  devId,
    CPSS_DXCH_TRUNK_LBH_CRC_MODE_ENT     crcMode,
    GT_U32                               crcSeed
);

/**
  * @internal cpssHalSetTrunkHashCrcParameters function
  * @endinternal
  *
  * @brief Set the seed value for pseudo random generator parameter used by the Trunk/L2/L3 ECMP hash mechanism
  * @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
  * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
  *
  * @param[in] devId                   - The device Id.
  * @param[in] hashClient              - hash client
  *                                      (APPLICABLE VALUES: CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_TRUNK_E,
  *                                       CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L2_ECMP_E,
  *                                       CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_L3_ECMP_E)
  * @param[in] seed                    - the seed value for pseudo random generator parameter
  *                                       (APPLICABLE RANGES: 1..0xffffffff)
  *
  * @retval GT_OK                    - on success
  * @retval GT_HW_ERROR              - on hardware error
  * @retval GT_BAD_PARAM             - bad device Id
  * @retval GT_OUT_OF_RANGE          - seed out of range.
  * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
  *
  * @note related to feature 'Random hash mode'
  *
  */
GT_STATUS cpssHalSetRandomHashSeedParameters
(
    int                                  devId,
    CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT hashClient,
    GT_U32                               seed
);

/**
 * @internal cpssHalSetTrunkHashGlobalMode function
 * @endinternal
 *
 * @brief   Function Relevant mode : ALL modes
 *         Set the general hashing mode of trunk hash generation.
 *
 * @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  None.
 *
 * @param[in] devId                   - the device Id.
 * @param[in] hashMode                 - hash mode
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PARAM             - bad device Id , or hash mode
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssHalSetTrunkHashGlobalMode
(
    int                                  devId,
    CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT  hashMode
);

/**
 * @internal cpssHalGetTrunkHashGlobalMode function
 * @endinternal
 *
 * @brief   Function Relevant mode : ALL modes
 *         Get the general hashing mode of trunk hash generation.
 *
 * @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  None.
 *
 * @param[in] devId                   - the device Id.
 *
 * @param[out] hashModePtr              - (pointer to)hash mode
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PARAM             - bad device Id
 * @retval GT_BAD_PTR               - one of the parameters in NULL pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssHalGetTrunkHashGlobalMode
(
    int                                   devId,
    CPSS_DXCH_TRUNK_LBH_GLOBAL_MODE_ENT  *hashModePtr
);

/**
 * @internal cpssHalAddTrunkMember function
 * @endinternal
 *
 * @brief   Function Relevant mode : High Level mode
 *         This function add member to the trunk in the device.
 *         If member is already in this trunk , function do nothing and
 *         return GT_OK.
 *         Notes about designated trunk table:
 *         If (no designated defined)
 *         re-distribute MC/Cascade trunk traffic among the enabled members,
 *         now taking into account also the added member
 *         else
 *         1. If added member is not the designated member - set its relevant bits to 0
 *         2. If added member is the designated member & it's enabled,
 *         set its relevant bits on all indexes to 1.
 *
 * @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  None.
 *
 * @param[in] devId                   - the device Id on which to add member to the trunk
 * @param[in] trunkId                  - the trunk id.
 * @param[in] memberPtr                - (pointer to)the member to add to the trunk.
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 * @retval GT_NOT_INITIALIZED       - 1. The trunk library was not initialized for the device
 *                                       2. In mode of 'CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E'
 *                                       the 'maxNumOfMembers' for this trunk is 0
 *                                       (cpssDxChTrunkFlexInfoSet(...) was not called ,
 *                                       or was called with maxNumOfMembers = 0)
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PARAM             - bad device Id , or
 *                                       bad trunkId number , or
 *                                       bad member parameters :
 *                                       (device & 0xE0) != 0 means that the HW can't support
 *                                       this value , since HW has 5 bit
 *                                       for device Id
 *                                       (port & 0xC0) != 0 means that the HW can't support
 *                                       this value , since HW has 6 bit
 *                                       for port number
 * @retval GT_BAD_PTR               - one of the parameters in NULL pointer
 * @retval GT_ALREADY_EXIST         - this member already exists in another trunk.
 * @retval GT_FULL                  - trunk already contains maximum supported members
 * @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssHalAddTrunkMember
(
    int                      devId,
    GT_TRUNK_ID              trunkId,
    CPSS_TRUNK_MEMBER_STC    *memberPtr
);

/**
  * @internal cpssHalEnableTrunkMember function
  * @endinternal
  *
  * @brief   Function Relevant mode : High Level mode
  *         This function enable (disabled)existing member of trunk in the device.
  *         If member is already enabled in this trunk , function do nothing and
  *         return GT_OK.
  *         Notes about designated trunk table:
  *         If (no designated defined)
  *         re-distribute MC/Cascade trunk traffic among the enabled members -
  *         now taking into account also the enabled member
  *         else
  *         1. If enabled member is not the designated member - set its relevant bits to 0
  *         2. If enabled member is the designated member set its relevant bits
  *         on all indexes to 1.
  *
  * @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
  * @note   NOT APPLICABLE DEVICES:  None.
  *
  * @param[in] devId                   - the device Id on which to enable member in the trunk
  * @param[in] trunkId                  - the trunk id.
  * @param[in] memberPtr                - (pointer to)the member to enable in the trunk.
  *
  * @retval GT_OK                    - on success
  * @retval GT_FAIL                  - on error
  * @retval GT_NOT_INITIALIZED       - 1. The trunk library was not initialized for the device
  *                                       2. In mode of 'CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E'
  *                                       the 'maxNumOfMembers' for this trunk is 0
  *                                       (cpssDxChTrunkFlexInfoSet(...) was not called ,
  *                                       or was called with maxNumOfMembers = 0)
  * @retval GT_HW_ERROR              - on hardware error
  * @retval GT_BAD_PARAM             - bad device Id , or
  *                                       bad trunkId number , or
  *                                       bad member parameters :
  *                                       (device & 0xE0) != 0 means that the HW can't support
  *                                       this value , since HW has 5 bit
  *                                       for device Id
  *                                       (port & 0xC0) != 0 means that the HW can't support
  *                                       this value , since HW has 6 bit
  *                                       for port number
  * @retval GT_BAD_PTR               - one of the parameters in NULL pointer
  * @retval GT_NOT_FOUND             - this member not found (member not exist) in the trunk
  * @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
  * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
  */
GT_STATUS cpssHalEnableTrunkMember
(
    int                      devId,
    GT_TRUNK_ID              trunkId,
    CPSS_TRUNK_MEMBER_STC    *memberPtr
);

GT_STATUS cpssHalDisableTrunkMember
(
    int                      devId,
    GT_TRUNK_ID              trunkId,
    CPSS_TRUNK_MEMBER_STC    *memberPtr
);

/**
 * @internal cpssHalRemoveTrunkMember function
 * @endinternal
 *
 * @brief   Function Relevant mode : High Level mode
 *         This function remove member from a trunk in the device.
 *         If member not exists in this trunk , function do nothing and
 *         return GT_OK.
 *         Notes about designated trunk table:
 *         If (no designated defined)
 *         re-distribute MC/Cascade trunk traffic among the enabled members -
 *         now taking into account also the removed member
 *         else
 *         1. If removed member is not the designated member - nothing to do
 *         2. If removed member is the designated member set its relevant bits
 *         on all indexes to 1.
 *
 * @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  None.
 *
 * @param[in] devId                   - the device Id on which to remove member from the trunk
 * @param[in] trunkId                  - the trunk id.
 * @param[in] memberPtr                - (pointer to)the member to remove from the trunk.
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 * @retval GT_NOT_INITIALIZED       -the trunk library was not initialized for the device
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PARAM             - bad device Id , or
 *                                       bad trunkId number , or
 *                                       bad member parameters :
 *                                       (device & 0xE0) != 0 means that the HW can't support
 *                                       this value , since HW has 5 bit
 *                                       for device Id
 *                                       (port & 0xC0) != 0 means that the HW can't support
 *                                       this value , since HW has 6 bit
 *                                       for port number
 * @retval GT_BAD_PTR               - one of the parameters in NULL pointer
 * @retval GT_BAD_STATE             - the trunk-type is not one of: CPSS_TRUNK_TYPE_FREE_E or CPSS_TRUNK_TYPE_REGULAR_E
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssHalRemoveTrunkMember
(
    int                      devId,
    GT_TRUNK_ID              trunkId,
    CPSS_TRUNK_MEMBER_STC    *memberPtr
);

/**
  * @internal cpssHalSetTrunkHashIpAddMacMode function
  * @endinternal
  *
  * @brief   Function Relevant mode : ALL modes
  *         Set the use of mac address bits to trunk hash calculation when packet is
  *         IP and the "Ip trunk hash mode enabled".
  *         Note:
  *         1. Not relevant to NON Ip packets.
  *         2. Not relevant to multi-destination packets (include routed IPM).
  *         3. Not relevant when cpssDxChTrunkHashGlobalModeSet(devId,
  *         CPSS_DXCH_TRUNK_LBH_INGRESS_PORT_E)
  *
  * @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
  * @note   NOT APPLICABLE DEVICES:  None.
  *
  * @param[in] devId                   - the device Id.
  * @param[in] enable                   - enable/disable feature
  *                                      GT_FALSE - If the packet is an IP packet MAC data is not
  *                                      added to the Trunk load balancing hash.
  *                                      GT_TRUE - The following function is added to the trunk load
  *                                      balancing hash:
  *                                      MACTrunkHash = MAC_SA[5:0]^MAC_DA[5:0].
  *                                      NOTE: When the packet is not an IP packet and
  *                                      <TrunkLBH Mode> = 0, the trunk load balancing
  *                                      hash = MACTrunkHash, regardless of this setting.
  *                                      If the packet is IPv4/6-over-X tunnel-terminated,
  *                                      the mode is always GT_FALSE (since there is no
  *                                      passenger packet MAC header).
  *
  * @retval GT_OK                    - on success
  * @retval GT_FAIL                  - on error
  * @retval GT_HW_ERROR              - on hardware error
  * @retval GT_BAD_PARAM             - bad device Id
  * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
  */
GT_STATUS cpssHalSetTrunkHashIpAddMacMode
(
    int      devId,
    GT_BOOL  enable
);

/**
 * @internal cpssHalSetTrunkHashIpMode function
 * @endinternal
 *
 * @brief   Function Relevant mode : ALL modes
 *         Enable/Disable the device from considering the IP SIP/DIP information,
 *         when calculation the trunk hashing index for a packet.
 *         Relevant when the is IPv4 or IPv6 and <TrunkHash Mode> = 0.
 *         Note:
 *         1. Not relevant to NON Ip packets.
 *         2. Not relevant to multi-destination packets (include routed IPM).
 *         3. Not relevant when cpssDxChTrunkHashGlobalModeSet(devId,
 *         CPSS_DXCH_TRUNK_LBH_INGRESS_PORT_E)
 *
 * @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  None.
 *
 * @param[in] devId                   - the device Id.
 * @param[in] enable                   - GT_FALSE - IP data is not added to the trunk load balancing
 *                                      hash.
 *                                      GT_TRUE - The following function is added to the trunk load
 *                                      balancing hash, if the packet is IPv6.
 *                                      IPTrunkHash = according to setting of API
 *                                      cpssDxChTrunkHashIpv6ModeSet(...)
 *                                      else packet is IPv4:
 *                                      IPTrunkHash = SIP[5:0]^DIP[5:0]^SIP[21:16]^DIP[21:16].
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PARAM             - bad device Id
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssHalSetTrunkHashIpMode
(
    int     devId,
    GT_BOOL enable
);

/**
  * @internal cpssHalSetTrunkHashL4Mode function
  * @endinternal
  *
  * @brief   Function Relevant mode : ALL modes
  *         Enable/Disable the device from considering the L4 TCP/UDP
  *         source/destination port information, when calculation the trunk hashing
  *         index for a packet.
  *         Note:
  *         1. Not relevant to NON TCP/UDP packets.
  *         2. The Ipv4 hash must also be enabled , otherwise the L4 hash mode
  *         setting not considered.
  *         3. Not relevant to multi-destination packets (include routed IPM).
  *         4. Not relevant when cpssDxChTrunkHashGlobalModeSet(devId,
  *         CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E)
  *
  * @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
  * @note   NOT APPLICABLE DEVICES:  None.
  *
  * @param[in] devId                   - the device Id.
  * @param[in] hashMode                 - L4 hash mode.
  *
  * @retval GT_OK                    - on success
  * @retval GT_FAIL                  - on error
  * @retval GT_HW_ERROR              - on hardware error
  * @retval GT_BAD_PARAM             - bad device Id
  * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
  */
GT_STATUS cpssHalSetTrunkHashL4Mode
(
    int                             devId,
    CPSS_DXCH_TRUNK_L4_LBH_MODE_ENT hashMode
);

/**
  * @internal cpssHalSetTrunkHashIpv6Mode function
  * @endinternal
  *
  * @brief   Function Relevant mode : ALL modes
  *         Set the hash generation function for Ipv6 packets.
  *
  * @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
  * @note   NOT APPLICABLE DEVICES:  None.
  *
  * @param[in] devId                   - the device Id.
  * @param[in] hashMode                 - the Ipv6 hash mode.
  *
  * @retval GT_OK                    - on success
  * @retval GT_FAIL                  - on error
  * @retval GT_HW_ERROR              - on hardware error
  * @retval GT_BAD_PARAM             - bad device Id , or hash mode
  * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
  */
GT_STATUS cpssHalSetTrunkHashIpv6Mode
(
    int                                  devId,
    CPSS_DXCH_TRUNK_IPV6_HASH_MODE_ENT   hashMode
);

/**
 * @internal cpssHalSetTrunkHashMplsModeEnable function
 * @endinternal
 *
 * @brief   Function Relevant mode : ALL modes
 *         Enable/Disable the device from considering the MPLS information,
 *         when calculating the trunk hashing index for a packet.
 *         Note:
 *         1. Not relevant to NON-MPLS packets.
 *         2. Relevant when cpssDxChTrunkHashGlobalModeSet(devId,
 *         CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_E)
 *
 * @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
 *
 * @param[in] devId                   - the device Id.
 * @param[in] enable                   - GT_FALSE - MPLS parameter are not used in trunk hash index
 *                                      GT_TRUE  - The following function is added to the trunk load
 *                                      balancing hash:
 *                                      MPLSTrunkHash = (mpls_label0[5:0] & mpls_trunk_lbl0_mask) ^
 *                                      (mpls_label1[5:0] & mpls_trunk_lbl1_mask) ^
 *                                      (mpls_label2[5:0] & mpls_trunk_lbl2_mask)
 *                                      NOTE:
 *                                      If any of MPLS Labels 0:2 do not exist in the packet,
 *                                      the default value 0x0 is used for TrunkHash calculation
 *                                      instead.
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PARAM             - bad device Id
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssHalSetTrunkHashMplsModeEnable
(
    int  devId,
    GT_BOOL enable
);

GT_STATUS cpssHalSetHashNumBits
(
    int devId,
    GT_U32 startBit,
    GT_U32 numOfBits,
    CPSS_DXCH_TRUNK_HASH_CLIENT_TYPE_ENT hashClient
);

GT_STATUS cpssHalTrunkEcmpLttTableRandomEnableSet
(
    int devId,
    GT_TRUNK_ID trunkId,
    GT_BOOL enable
);


#ifdef __cplusplus
}
#endif

