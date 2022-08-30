/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
 ********************************************************************************
 * @file cpssHalVlan.h
 * @brief Private API declarations for CPSS Vlan functions which can be used in XPS layer.
 *
 * @version   01
 ********************************************************************************
 */


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include  <gtOs/gtGenTypes.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgVlan.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgGen.h>

GT_STATUS cpssHalUpdateMaxVlanCountInHw(int devId, GT_U16 maxVlanId);
/**
 * @internal cpssHalEnableBrgVlanPortIngFlt function
 * @endinternal
 *
 * @brief   Enable/disable Ingress Filtering for specific port
 *
 * @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  None.
 *
 * @param[in] devId                   - device Id
 * @param[in] portNum                  - port number
 * @param[in] enable                   - GT_TRUE,  ingress filtering
 *                                      GT_FALSE, disable ingress filtering
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error.
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PARAM             - wrong devId or port
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssHalEnableBrgVlanPortIngFlt
(
    int          devId,
    GT_PORT_NUM  portNum,
    GT_BOOL      enable
);

/**
 * @internal cpssHalWriteBrgVlanEntry function
 * @endinternal
 *
 * @brief   Builds and writes vlan entry to HW.
 *
 * @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  None.
 *
 * @param[in] devId                   - device Id
 * @param[in] vlanId                   - VLAN Id
 * @param[in] portsMembersPtr          - (pointer to) bitmap of physical ports members in vlan
 *                                      CPU port supported
 * @param[in] portsTaggingPtr          - (pointer to) bitmap of physical ports tagged in the vlan -
 *                                      The parameter is relevant for DxCh1, DxCh2
 *                                      and DxCh3 devices.
 *                                      The parameter is relevant for xCat and
 *                                      above devices without TR101 feature support
 *                                      The parameter is not relevant and ignored for
 *                                      xCat and above devices with TR101 feature
 *                                      support.
 * @param[in] vlanInfoPtr              - (pointer to) VLAN specific information
 * @param[in] portsTaggingCmdPtr       - (pointer to) physical ports tagging commands in the vlan -
 *                                      The parameter is relevant only for xCat and
 *                                      above with TR101 feature support.
 *                                      The parameter is not relevant and ignored for
 *                                      other devices.
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error.
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PARAM             - wrong devId or vid
 * @retval GT_OUT_OF_RANGE          - length of portsMembersPtr, portsTaggingPtr
 * @retval or vlanInfoPtr           ->stgId is out of range
 * @retval GT_BAD_PTR               - one of the parameters is NULL pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssHalWriteBrgVlanEntry
(
    int                                   devId,
    GT_U16                                vlanId,
    CPSS_PORTS_BMP_STC                    *portsMembersPtr,
    CPSS_PORTS_BMP_STC                    *portsTaggingPtr,
    CPSS_DXCH_BRG_VLAN_INFO_STC           *vlanInfoPtr,
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC  *portsTaggingCmdPtr
);

/**
 * @internal cpssHalReadBrgVlanEntry function
 * @endinternal
 *
 * @brief   Read vlan entry.
 *
 * @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  None.
 *
 * @param[in] devId                   - device Id
 * @param[in] vlanId                   - VLAN Id
 *
 * @param[out] portsMembersPtr          - (pointer to) bitmap of physical ports members in vlan
 *                                      CPU port supported
 * @param[out] portsTaggingPtr          - (pointer to) bitmap of physical ports tagged in the vlan -
 *                                      The parameter is relevant for DxCh1, DxCh2
 *                                      and DxCh3 devices.
 *                                      The parameter is relevant for xCat and
 *                                      above devices without TR101 feature support
 *                                      The parameter is not relevant and ignored for
 *                                      xCat and above devices with TR101 feature
 *                                      support.
 * @param[out] vlanInfoPtr              - (pointer to) VLAN specific information
 * @param[out] isValidPtr               - (pointer to) VLAN entry status
 *                                      GT_TRUE = Vlan is valid
 *                                      GT_FALSE = Vlan is not Valid
 * @param[out] portsTaggingCmdPtr       - (pointer to) physical ports tagging commands in the vlan -
 *                                      The parameter is relevant only for xCat and
 *                                      above with TR101 feature support.
 *                                      The parameter is not relevant and ignored for
 *                                      other devices.
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error.
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PARAM             - wrong devId or vid
 * @retval GT_BAD_PTR               - one of the parameters is NULL pointer
 * @retval GT_TIME              - after max number of retries checking if PP ready
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssHalReadBrgVlanEntry
(
    int                                  devId,
    GT_U16                               vlanId,
    CPSS_PORTS_BMP_STC                   *portsMembersPtr,
    CPSS_PORTS_BMP_STC                   *portsTaggingPtr,
    CPSS_DXCH_BRG_VLAN_INFO_STC          *vlanInfoPtr,
    GT_BOOL                              *isValidPtr,
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC *portsTaggingCmdPtr,
    CPSS_PORTS_BMP_STC                   *globalPortMembers
);

/**
 * @internal cpssHalSetBrgVlanMruProfileIdx function

 * @endinternal
 *
 * @brief   Set Maximum Receive Unit MRU profile index for a VLAN.
 *         MRU VLAN profile sets maximum packet size that can be received
 *         for the given VLAN.
 *         Value of MRU for profile is set by cpssDxChBrgVlanMruProfileValueSet.
 *
 * @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
 *
 * @param[in] devId                   - device Id
 * @param[in] vlanId                   - vlan id
 * @param[in] mruIndex                 - MRU profile index (APPLICABLE RANGES: 0..7)
 *
 * @retval GT_OK                    - on success.
 * @retval GT_FAIL                  - on error
 * @retval GT_BAD_PARAM             - on wrong input parameters
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssHalSetBrgVlanMruProfileIdx
(
    int       devId,
    GT_U16    vlanId,
    GT_U32    mruIndex
);

/**
 * @internal cpssHalSetBrgVlanMruProfileValue function

 * @endinternal
 *
 * @brief   Set Maximum Receive Unit MRU profile.
 *
 * @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
 *
 * @param[in] devId                   - device Id
 * @param[in] mruIndex                 - MRU profile index (APPLICABLE RANGES: 0..7)
 * @param[in] mruIndex                  - MRU value.
 *
 * @retval GT_OK                    - on success.
 * @retval GT_FAIL                  - on error
 * @retval GT_BAD_PARAM             - on wrong input parameters
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssHalSetBrgVlanMruProfileValue
(
    int       devId,
    IN GT_U32    mruIndex,
    IN GT_U32    mruValue
);

/**
 * @internal cpssHalSetBrgVlanForwardingId function
 * @endinternal
 *
 * @brief   Set Forwarding ID value.
 *         Used for virtual birdges per eVLAN.
 *         Used by the bridge engine for entry lookup and entry match (replaces VID
 *         in bridge entry)
 *
 * @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
 *
 * @param[in] devId                   - device Id
 * @param[in] vlanId                   - VLAN ID which ingress VLAN mirroring is enabled/disabled..
 * @param[in] fidValue                 - Forwarding ID value
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - on wrong input parameters
 * @retval GT_HW_ERROR              - failed to write to hardware
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssHalSetBrgVlanForwardingId
(
    int     devId,
    GT_U16  vlanId,
    GT_U32  fidValue
);

/**
 * @internal cpssHalBindBrgVlanToStpId function
 * @endinternal
 *
 * @brief   Bind VLAN to STP Id.
 *
 * @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  None.
 *
 * @param[in] devId                   - device Id
 * @param[in] vlanId                   - vlan Id
 * @param[in] stpId                    - STP Id
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PARAM             - wrong devId or vlanId
 * @retval GT_OUT_OF_RANGE          - out of range stpId
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssHalBindBrgVlanToStpId
(
    int      devId,
    GT_U16   vlanId,
    GT_U16   stpId
);

/**
  * @internal cpssHalInvalidateBrgVlanEntry function
  * @endinternal
  *
  * @brief   This function invalidates VLAN entry.
  *
  * @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
  * @note   NOT APPLICABLE DEVICES:  None.
  *
  * @param[in] devId                   - device Id.
  * @param[in] vlanId                   - VLAN id
  *
  * @retval GT_OK                    - on success
  * @retval GT_FAIL                  - otherwise
  * @retval GT_BAD_PARAM             - wrong devId or vid
  * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
  */

GT_STATUS cpssHalInvalidateBrgVlanEntry
(
    int      devId,
    GT_U16   vlanId
);

/**
  * @internal cpssHalSetBrgVlanUnkUnregFilter function
  * @endinternal
  *
  * @brief   Set per VLAN filtering command for specified Unknown or Unregistered
  *         packet type
  *
  * @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
  * @note   NOT APPLICABLE DEVICES:  None.
  *
  * @param[in] devId                   - device Id.
  * @param[in] vlanId                   - VLAN ID
  * @param[in] packetType               - packet type
  * @param[in] cmd                      - command for the specified packet type
  *
  * @retval GT_OK                    - on success
  * @retval GT_FAIL                  - on error
  * @retval GT_BAD_PARAM             - wrong devId or vlanId
  * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
  */
GT_STATUS cpssHalSetBrgVlanUnkUnregFilter
(
    int                                  devId,
    GT_U16                               vlanId,
    CPSS_DXCH_BRG_VLAN_PACKET_TYPE_ENT   packetType,
    CPSS_PACKET_CMD_ENT                  cmd

);

/**
 * @internal cpssHalSetBrgGenArpBcastToCpuCmd function
 * @endinternal
 *
 * @brief   Enables trapping or mirroring to CPU ARP Broadcast packets for all VLANs or ports
 *
 * @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  None.
 *
 * @param[in] devId                   - device Id
 * @param[in] cmdMode                  - command interface mode
 *                                      (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
 * @param[in] cmd                      - supported commands:
 *                                      CPSS_PACKET_CMD_FORWARD_E , CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
 *                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PARAM             - on wrong device Id, command mode or control packet command
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 * @note For ARP broadcast received on VLAN - IPv4/IPv6 Control Traffic To CPU must be enabled by function
 *       cpssDxChBrgVlanIpCntlToCpuSet
 *       For ARP broadcast received on port - ARP Broadcast Trap To CPU must be enabled by function
 *       cpssDxChBrgGenArpTrapEnable
 *
 */
GT_STATUS cpssHalSetBrgGenArpBcastToCpuCmd
(
    int                              devId,
    CPSS_DXCH_ARP_BCAST_CMD_MODE_ENT cmdMode,
    CPSS_PACKET_CMD_ENT              cmd
);

/**
  * @internal cpssHalSetBrgVlanUnknownMacSaCommand function
  * @endinternal
  *
  * @brief   Set bridge forwarding decision for packets with unknown Source Address.
  *         Only relevant in controlled address learning mode, when <Auto-learning
  *         enable> = Disabled.
  *
  * @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
  * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
  *
  * @param[in] devId                   - device Id
  * @param[in] vlanId                   - VLAN ID
  * @param[in] cmd                      - supported commands:
  *                                      CPSS_PACKET_CMD_FORWARD_E , CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
  *                                      CPSS_PACKET_CMD_TRAP_TO_CPU_E , CPSS_PACKET_CMD_DROP_HARD_E
  *                                      CPSS_PACKET_CMD_DROP_SOFT_E
  *
  * @retval GT_OK                    - on success.
  * @retval GT_BAD_PARAM             - on wrong input parameters
  * @retval GT_HW_ERROR              - failed to write to hardware
  * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
  */
GT_STATUS cpssHalSetBrgVlanUnknownMacSaCommand
(
    int                  devId,
    GT_U16               vlanId,
    CPSS_PACKET_CMD_ENT  cmd
);


/**
 * @internal cpssHalEnableBrgVlanIgmpSnooping function
 * @endinternal
 *
 * @brief   Enable/Disable IGMP trapping or mirroring to the CPU according to the
 *         global setting.
 *
 * @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  None.
 *
 * @param[in] devId                   - device Id
 * @param[in] vlanId                   - the VLAN-ID in which IGMP trapping is enabled/disabled.
 * @param[in] enable                   - GT_TRUE - enable
 *                                      GT_FALSE - disable
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PARAM             - on on bad device Id
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssHalEnableBrgVlanIgmpSnooping
(
    int     devId,
    GT_U16  vlanId,
    GT_BOOL enable
);

/**
 * @internal cpssHalEnableBrgVlanIpV6IcmpToCpu function
 * @endinternal
 *
 * @brief   Enable/Disable ICMPv6 trapping or mirroring to
 *         the CPU, according to global ICMPv6 message type
 *
 * @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  None.
 *
 * @param[in] devId                   - device Id
 * @param[in] vlanId                   - VLAN ID
 * @param[in] enable                   - GT_TRUE - enable
 *                                      GT_FALSE - disable
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on error
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PARAM             - on bad device Id or vlanId
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssHalEnableBrgVlanIpV6IcmpToCpu
(
    int        devId,
    GT_U16     vlanId,
    GT_BOOL    enable
);

/**
 * @internal cpssHalEnableBrgVlanNaToCpu function
 * @endinternal
 *
 * @brief   Enable/Disable New Address (NA) Message Sending to CPU per VLAN .
 *         To send NA to CPU both VLAN and port must be set to send NA to CPU.
 *
 * @note   APPLICABLE DEVICES:      DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond.
 *
 * @param[in] devId                    - device Id
 * @param[in] vlanId                   - vlan id
 * @param[in] enable                   - GT_TRUE  -  New Address Message Sending to CPU
 *                                      GT_FALSE - disable New Address Message Sending to CPU
 *
 * @retval GT_OK                    - on success.
 * @retval GT_FAIL                  - on error
 * @retval GT_BAD_PARAM             - on wrong input parameters
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS cpssHalEnableBrgVlanNaToCpu
(
    int       devId,
    GT_U16    vlanId,
    GT_BOOL   enable
);

GT_STATUS cpssHalBrgVlanIngressTpidProfileSet
(
    int       devId,
    GT_U32               profile,
    CPSS_ETHER_MODE_ENT  ethMode,
    GT_U32               tpidBmp
);

GT_STATUS cpssHalVlanUnkUnregFilterSet
(
    int       devId,
    GT_U16                               vlanId,
    CPSS_DXCH_BRG_VLAN_PACKET_TYPE_ENT   packetType,
    CPSS_PACKET_CMD_ENT                  cmd
);

GT_STATUS cpssHalBrgVlanIpmBridgingModeSet
(
    int       devId,
    GT_U16    vlanId,
    CPSS_IP_PROTOCOL_STACK_ENT  ipVer,
    CPSS_BRG_IPM_MODE_ENT       ipmMode
);

GT_STATUS cpssHalBrgVlanIpmBridgingEnable
(
    int       devId,
    GT_U16    vlanId,
    CPSS_IP_PROTOCOL_STACK_ENT  ipVer,
    GT_BOOL enable
);

GT_STATUS cpssHalBrgVlanIpMcRouteEnable
(
    int       devId,
    GT_U16    vlanId,
    CPSS_IP_PROTOCOL_STACK_ENT  ipVer,
    GT_BOOL enable
);

GT_STATUS cpssHalVlanMemberAdd
(
    uint32_t                devId,
    GT_U16                  vlanId,
    uint32_t                portNum,
    GT_BOOL                 isTagged,
    CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT portTaggingCmd
);

GT_STATUS cpssHalBrgVlanPortDelete
(
    uint32_t                devId,
    GT_U16                  vlanId,
    uint32_t                portNum
);

GT_STATUS cpssHalVlanTranslationEntryWrite(
    int devId,
    GT_U16 vlanId,
    CPSS_DIRECTION_ENT direction,
    GT_U16 transVlanId
);

GT_STATUS cpssHalVlanTranslationEntryRead(
    int devId,
    GT_U16 vlanId,
    CPSS_DIRECTION_ENT direction,
    GT_U16 *transVlanId
);

GT_STATUS cpssHalVlanPortTranslationEnableSet
(
    uint32_t                cpssDevId,
    uint32_t                cpssPortNum,
    CPSS_DIRECTION_ENT      direction,
    CPSS_DXCH_BRG_VLAN_TRANSLATION_ENT value
);

GT_STATUS cpssHalBrgFdbNaToCpuPerPortSet
(
    GT_U32    devId,
    GT_PORT_NUM  portNum,
    GT_BOOL  enable
);

#ifdef __cplusplus
}
#endif
