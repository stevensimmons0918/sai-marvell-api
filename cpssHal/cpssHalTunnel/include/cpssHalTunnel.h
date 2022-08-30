/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/*******************************************************************************
* @file cpssHalTunnel.h
*
* @brief Internal header which defines API for helpers functions, which are
*        specific for XPS QoS.
*
* @version   01
*******************************************************************************/

#ifndef _cpssHalTunnel_h_
#define _cpssHalTunnel_h_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "cpssDxChNetIf.h"
#include <gtOs/gtGenTypes.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcam.h>
#include <cpss/dxCh/dxChxGen/tti/cpssDxChTti.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgL2Ecmp.h>

#include  <gtOs/gtGenTypes.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/tunnel/cpssDxChTunnelTypes.h>
#include <cpss/dxCh/dxChxGen/tunnel/cpssDxChTunnel.h>
#include "cpssHalUtil.h"



/*DIP Offset is from IPv6 Ether Type start*/
#define CPSS_HAL_TTI_IPV6_DIP_OFFSET 26

#define CPSS_HAL_IPV4_ADDR_LEN  4
#define CPSS_HAL_IPV6_ADDR_LEN  16
#define CPSS_HAL_TTI_META_IPV6_TNL_PROT_ID_OFFSET 21
#define CPSS_HAL_TTI_META_PCL_ID_OFFSET 22

#define CPSSHAL_GEN_TUNNEL_PROFILE_VXLAN_UDP_PORT_FROM_HASH 1
#define CPSSHAL_GEN_TUNNEL_PROFILE_VXLAN_UDP_PORT_FROM_TS   2

typedef enum
{
    CPSS_HAL_TTI_IPV4_PCL_ID = 0x0,
    CPSS_HAL_TTI_UDB_IPV6_PCL_ID = 0x1,
    CPSS_HAL_TTI_UDB_IPV4_VXLAN_PCL_ID = 0x2,
    CPSS_HAL_TTI_UDB_IPV6_VXLAN_PCL_ID = 0x3,
    CPSS_HAL_TTI_UDB_UNI_VXLAN_PCL_ID = 0x4,
} CPSS_HAL_TTI_PCL_ID;


/**
* @internal cpssHalTtiPortLookupEnableSet function
* @endinternal
*
* @brief   This function enables/disables the TTI lookup for the specified key
*         type at the port.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] keyType                  - TTI key type
* @param[in] enable                   - GT_TRUE:  TTI lookup
*                                      GT_FALSE: disable TTI lookup
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalTtiPortLookupEnableSet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               portNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT           keyType,
    IN  GT_BOOL                              enable
);

/**
* @internal cpssHalTtiPortIpv4OnlyTunneledEnableSet function
* @endinternal
*
* @brief   This function enables/disables the IPv4 TTI lookup for only tunneled
*         packets received on port.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - GT_TRUE:  IPv4 TTI lookup only for tunneled packets
*                                      GT_FALSE: disable IPv4 TTI lookup only for tunneled packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalTtiPortIpv4OnlyTunneledEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_BOOL                             enable
);

/**
* @internal cpssHalTtiPortIpv4OnlyMacToMeEnableSet function
* @endinternal
*
* @brief   This function enables/disables the IPv4 TTI lookup for only mac to me
*         packets received on port.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*                                      In eArch devices portNum is default ePort.
* @param[in] enable                   - GT_TRUE:  IPv4 TTI lookup only for mac to me packets
*                                      GT_FALSE: disable IPv4 TTI lookup only for mac to me packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalTtiPortIpv4OnlyMacToMeEnableSet
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_NUM                         portNum,
    IN  GT_BOOL                             enable
);

/**
* @internal cpssHalTtiMacModeGet function
* @endinternal
*
* @brief   This function gets the lookup Mac mode for the specified key type.
*         This setting controls the Mac that would be used for key generation
*         (Source/Destination).
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - TTI key type; valid values:
*                                      CPSS_DXCH_TTI_KEY_IPV4_E
*                                      CPSS_DXCH_TTI_KEY_MPLS_E
*                                      CPSS_DXCH_TTI_KEY_ETH_E
*                                      CPSS_DXCH_TTI_KEY_MIM_E
*
* @param[out] macModePtr               - MAC mode to use; valid values:
*                                      CPSS_DXCH_TTI_MAC_MODE_DA_E
*                                      CPSS_DXCH_TTI_MAC_MODE_SA_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or key type
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssHalTtiMacModeGet
(
    IN  GT_U32                             devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT        keyType,
    OUT CPSS_DXCH_TTI_MAC_MODE_ENT        *macModePtr
);

/**
* @internal cpssHalTtiMacModeSet function
* @endinternal
* @brief   This function sets the lookup Mac mode for the specified key type.
*         This setting controls the Mac that would be used for key generation
*         (Source/Destination).
*
* @param[in] devNum                   - device number
* @param[in] keyType                  - TTI key type; valid values:
*                                      CPSS_DXCH_TTI_KEY_IPV4_E
*                                      CPSS_DXCH_TTI_KEY_MPLS_E
*                                      CPSS_DXCH_TTI_KEY_ETH_E
*                                      CPSS_DXCH_TTI_KEY_MIM_E  (APPLICABLE DEVICES xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
* @param[in] macMode                  - MAC mode to use; valid values:
*                                      CPSS_DXCH_TTI_MAC_MODE_DA_E
*                                      CPSS_DXCH_TTI_MAC_MODE_SA_E
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Not needed for TRILL key
*
*/
GT_STATUS cpssHalTtiMacModeSet
(
    IN  GT_U32                            devNum,
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT        keyType,
    IN  CPSS_DXCH_TTI_MAC_MODE_ENT        macMode
);

/**
* @internal cpssHalTunnelTerminationEntryAdd function
* @endinternal
*
* @brief   Write Rule to Tunnel Table.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
*                                      if rule with given rule Id already exists it overridden only
*                                      when the same priotity specified, otherwize GT_BAD_PARAM returned.
* @param[in] ruleAttributesPtr        - (pointer to)rule attributes (for priority driven vTCAM - priority)
* @param[in] ruleTypePtr              -     (pointer to)type of contents of rule
* @param[in] ruleDataPtr              -     (pointer to)data of contents of rule
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_FOUND             - vTcam with given Id not found
*/
GT_STATUS cpssHalTunnelTerminationEntryAdd
(
    IN  GT_U32                                    vTcamMngId,
    IN  GT_U32                                    vTcamId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ID              ruleId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC  *ruleAttributesPtr,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC        *ruleTypePtr,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC        *ruleDataPtr
);

/**
* @internal cpssHalTunnelTerminationActionUpdate function
* @endinternal
*
* @brief   Update Rule Action for Tunnel
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
* @param[in] actionTypePtr            - (pointer to)type of contents of action
* @param[in] actionDataPtr            - (pointer to)data of contents of action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
*/
GT_STATUS cpssHalTunnelTerminationActionUpdate
(
    GT_U32                                        vTcamMngId,
    GT_U32                                        vTcamId,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID                ruleId,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_STC   *actionTypePtr,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_STC   *actionDataPtr
);

/**
* @internal cpssHalTunnelTerminationActionGet function
* @endinternal
*
* @brief   Get Rule Action.
*
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
* @param[in] actionTypePtr            - (pointer to)type of contents of action
*
* @param[out] actionDataPtr            - (pointer to)data of contents of action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
*/
GT_STATUS cpssHalTunnelTerminationActionGet
(
    GT_U32                                        vTcamMngId,
    GT_U32                                        vTcamId,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID                ruleId,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_STC   *actionTypePtr,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_STC   *actionDataPtr
);


GT_STATUS cpssHalTunnelTerminationRuleGet
(
    GT_U32                                      vTcamMngId,
    GT_U32                                      vTcamId,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID              ruleId,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC        *ruleTypePtr,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC        *ruleDataPtr
);

/**
* @internal cpssHalTunnelTerminationEntryDelete function
* @endinternal
*
* @brief   Delete Rule From internal Tunnel Table
*
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
*/
GT_STATUS cpssHalTunnelTerminationEntryDelete
(
    IN   GT_U32                           vTcamMngId,
    IN   GT_U32                           vTcamId,
    IN   CPSS_DXCH_VIRTUAL_TCAM_RULE_ID     ruleId
);

/**
* @internal cpssHalTunnelTerminationIPv6KeySet function
* @endinternal
*
* @brief   Set Tunnel TTI params for UDB IPV6 Key
*
*
* @param[in] devId                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
*/

GT_STATUS cpssHalTunnelTerminationIPv6KeySet(GT_U32 devId);

/**
* @internal cpssHalTunnelVxlanIPv4TTIKeySet function
* @endinternal
*
* @brief   Set Tunnel TTI params for UDB IPV4 Key
*
*
* @param[in] devId                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
*/

GT_STATUS cpssHalTunnelVxlanIPv4TTIKeySet(GT_U32 devId);

/**
* @internal cpssHalTunnelVxlanIPv6TTIKeySet function
* @endinternal
*
* @brief   Set Tunnel TTI params for UDB IPV6 Key
*
*
* @param[in] devId                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
*/

GT_STATUS cpssHalTunnelVxlanIPv6TTIKeySet(GT_U32 devId);

GT_STATUS cpssHalTunnelTunnelStartEntryWrite
(
    int                                   devId,
    GT_U32                                entryId,
    CPSS_TUNNEL_TYPE_ENT                  tunnelType,
    CPSS_DXCH_TUNNEL_START_CONFIG_UNT    *entry_PTR
);

GT_STATUS cpssHalTunnelTunnelStartEntryRead
(
    int                                   devId,
    GT_U32                                entryId,
    CPSS_TUNNEL_TYPE_ENT                 *tunnelType_PTR,
    CPSS_DXCH_TUNNEL_START_CONFIG_UNT    *entry_PTR
);


GT_STATUS cpssHalTunnelStartEcnModeSet(int devId,
                                       CPSS_DXCH_TUNNEL_START_ECN_MODE_ENT ecnMode);
GT_STATUS cpssHalTunnelStartEcnModeGet(int devId,
                                       CPSS_DXCH_TUNNEL_START_ECN_MODE_ENT *ecnMode);

GT_STATUS cpssHalTunnelVlanToVniMapSet
(
    int    devId,
    GT_U32 vlan,
    GT_U32 vni
);

GT_STATUS cpssHalTunnelVxlanInit
(
    int    devId
);

GT_STATUS cpssHalTunnelEPortToPhyPortMapSet
(
    IN GT_U8              devId,
    IN GT_PORT_NUM        ePort,
    IN CPSS_INTERFACE_TYPE_ENT type,
    IN GT_U32             intfNum
);

GT_STATUS cpssHalTunnelEPorTMtuProfileSet
(
    IN GT_U8  devId,
    IN GT_U32 portNum,
    IN GT_U32 ePort
);

GT_STATUS cpssHalTunnelEPorTSrcMacClear
(
    IN GT_U8  devId,
    IN GT_U32 ePort
);
GT_STATUS cpssHalTunnelEPorTSrcMacSet
(
    IN GT_U8  devId,
    IN GT_U32 macSaTableIndex,
    IN GT_U32 ePort
);

GT_STATUS cpssHalTunnelEPortToPhyPortMapClear
(
    IN GT_U8              devId,
    IN GT_PORT_NUM        ePort
);

GT_STATUS cpssHalTunnelToEPortMapSet
(
    IN GT_U8              devId,
    IN GT_PORT_NUM        ePort,
    IN GT_U32             tunnelStartIndex
);

GT_STATUS cpssHalTunnelToEPortMapClear
(
    IN GT_U8              devId,
    IN GT_PORT_NUM        ePort
);

GT_STATUS cpssHalEportToPhysicalPortTargetMappingTableGet
(
    uint32_t  devId,
    uint32_t  portNum,
    CPSS_INTERFACE_INFO_STC *physicalInfoPtr
);

GT_STATUS cpssHalTunnelL2EcmpLttTableSet
(
    IN GT_U8              devId,
    IN GT_PORT_NUM        ePort,
    IN CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC *info
);

GT_STATUS cpssHalTunnelL2EcmpTableSet
(
    IN GT_U8              devId,
    IN GT_U32             index,
    IN CPSS_DXCH_BRG_L2_ECMP_ENTRY_STC     *ecmpEntryPtr
);
GT_STATUS cpssHalGlobalEportRangeUpdate(uint32_t devId,
                                        CPSS_HAL_EPORT_TYPE type,
                                        uint32_t minVal, uint32_t maxVal);
GT_STATUS cpssHalTunnelSetEPortMeshId(uint32_t devId, uint32_t ePort,
                                      uint32_t meshId);
GT_STATUS cpssHalTunnelEnableMeshIdFiltering(uint32_t devId,
                                             GT_BOOL enable);
#ifdef __cplusplus
}
#endif

#endif //_cpssHalTunnel_h_
