/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

#ifndef __cpssHalL3__
#define __cpssHalL3__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/tti/cpssDxChTtiTypes.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIp.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/cpssDxChIpLpm.h>
#include <cpss/dxCh/dxChxGen/tti/cpssDxChTti.h>

GT_STATUS cpssHalL3IpRouterArpAddWrite(int devId,
                                       xpsArpPointer_t routerArpIndex, macAddr_t arpDaMac);
GT_STATUS cpssHalL3BindPortIntf(GT_U8 cpssDevId,    GT_U32 cpssPortNum,
                                GT_U16 vlanId);
GT_STATUS cpssHalL3SetEgressRouterMac(GT_U8 cpssDevId, GT_BOOL isVlan,
                                      GT_U32 cpssPortNum, macAddr_t egressMac);
GT_STATUS cpssHalL3UnBindPortIntf(GT_U8 cpssDevId,    GT_U32 cpssPortNum);
GT_STATUS cpssHalL3RemoveEgressRouterMac(GT_U8 cpssDevId, GT_BOOL isVlan,
                                         GT_U32 cpssPortNum);
GT_STATUS cpssHalL3UpdtIntfIpUcRoutingEn(GT_U8 cpssDevId, GT_U32 cpssPortNum,
                                         CPSS_IP_PROTOCOL_STACK_ENT protocol, GT_BOOL enable, GT_BOOL isSVI);
GT_STATUS cpssHalL3SetMtuProfileLimitValue(uint32_t devId,
                                           GT_U32 mtuProfileIndex, GT_U32 mtu);
GT_STATUS cpssHalL3SetPortEgressMac(GT_U8 cpssDevId,  GT_U32 cpssPortNum,
                                    macAddr_t egressMac);


GT_STATUS cpssHalWriteL3IpRouterArpAddress
(
    int      devId,
    xpsArpPointer_t routerArpIndex,
    macAddr_t arpDaMac
);

/**
 *
 * \brief unmask an IP header error due to SIP=DIP
 *
 *
 *
 * \param [in] devId
 *
 * \return GT_STATUS
 */
GT_STATUS cpssHalL3IpHeaderErrorMaskSet(int devId);

/**
* @internal cpssHalSetTtiPortGroupMacToMe function
* @endinternal
*
* @brief   This function sets a TTI MacToMe entry.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @param[in] devId                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion; Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] entryIndex               - Index of mac and vlan in MacToMe table.
*                                      (APPLICABLE RANGES:
*                                      xCat3; Lion2: 0..7
*                                      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2: 0..127)
* @param[in] valuePtr                 - points to Mac To Me and Vlan To Me
* @param[in] maskPtr                  - points to mac and vlan's masks
* @param[in] interfaceInfoPtr         - points to source interface info (APPLICABLE DEVICES Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device id or wrong vlan/mac values
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS cpssHalSetTtiPortGroupMacToMe
(
    int                                            devId,
    GT_PORT_GROUPS_BMP                             portGroupsBmp,
    GT_U32                                         entryIndex,
    CPSS_DXCH_TTI_MAC_VLAN_STC                         *valuePtr,
    CPSS_DXCH_TTI_MAC_VLAN_STC                      *maskPtr,
    CPSS_DXCH_TTI_MAC_TO_ME_SRC_INTERFACE_INFO_STC  *interfaceInfoPtr
);


/**
* @internal cpssHalWriteIpUcRouteEntries function
* @endinternal
*
* @brief   Writes an array of uc route entries to hw.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devId                   - the device number
* @param[in] baseRouteEntryIndex      - the index from which to write the array
* @param[in] routeEntriesArray        - the uc route entries array
* @param[in] numOfRouteEntries        - the number route entries in the array (= the
*                                      number of route entries to write)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on devNum not active.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In case uRPF ECMP/QOS is globally enabled, then for ECMP/QOS block
*       with x route entries, additional route entry should be included after
*       the block with the uRPF route entry information.
*
*/


GT_STATUS cpssHalWriteIpUcRouteEntries
(
    int                             devId,
    GT_U32                          baseRouteEntryIndex,
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC *routeEntriesArray,
    GT_U32                          numOfRouteEntries
);


/**
* @internal cpssHalEnableIpRouting function
* @endinternal
*
* @brief   globally enable/disable routing.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devId                   - the device number
* @param[in] enableRouting            - enable /disable global routing
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - wrong devNum
*
* @note the ASIC defualt is routing enabled.
*
*/


GT_STATUS cpssHalEnableIpRouting
(
    int      devId,
    GT_BOOL  enableRouting
);



GT_STATUS cpssHalBrgVlanIpUcRouteEnable
(
    int devId,
    xpsVlan_t vlanId
);

/**
* @internal cpssHalL3SetIPMtuUcRouteEntries function
* @endinternal
*
* @brief   set the mtu profile index for entire unicast nexthop
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devId                   - the device number
* @param[in] mtuProfileIndex            - MTU profile index
* @param[in] cpssIntfType         -     interface type (vlan, trunk, physical port)
* @param[in] cpssIfNum          -  interface Id (trunk ID, vlan ID, Phsyical port ID)
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - wrong devNum
*
* @note
**/

GT_STATUS cpssHalL3SetIPMtuUcRouteEntries
(
    GT_U8      devId,
    GT_U32 mtuProfileIndex,
    CPSS_INTERFACE_TYPE_ENT cpssIntfType,
    GT_PORT_NUM     cpssIfNum
);

GT_STATUS cpssHalCncL3Init(GT_U8 devId);

GT_STATUS cpssHalIpExceptionCommandSet
(
    GT_U32 devId,
    CPSS_DXCH_IP_EXCEPTION_TYPE_ENT excep,
    CPSS_IP_PROTOCOL_STACK_ENT proto,
    CPSS_PACKET_CMD_ENT pktCmd
);

GT_STATUS cpssHalBrgVlanVrfIdSet(GT_U32 devId, GT_U16 vlanId, GT_U32 vrfId);

GT_STATUS cpssHalIpLpmVirtualRouterAdd
(
    GT_U32 lpmDBId,
    GT_U32 vrId,
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC *vrConfigPtr
);
GT_STATUS cpssHalIpLpmVirtualRouterDel(GT_U32 lpmDBId, GT_U32 vrId);

GT_STATUS cpssHalLpmLeafEntryWrite(GT_U32  devId, GT_U32 leafIndex,
                                   CPSS_DXCH_LPM_LEAF_ENTRY_STC *leafPtr);
GT_STATUS cpssHalIpPbrBypassRouterTriggerRequirementsEnableSet(GT_U32  devId,
                                                               GT_BOOL enable);
GT_STATUS cpssHalL3GetMacSaTableIndex(GT_ETHERADDR* macAddrPtr, GT_U32* index);
GT_STATUS cpssHalL3FreeMacSaTableIndex(GT_U32 index);
GT_STATUS cpssHalL3EnableUcRoutingOnPort(GT_U8 cpssDevId, GT_U32 cpssPortNum,
                                         CPSS_IP_PROTOCOL_STACK_ENT protocol, GT_BOOL enable);
GT_STATUS cpssHalL3EnableBridgeFdbRoutingOnPort(GT_U8 cpssDevId,
                                                GT_U32 cpssPortNum,
                                                CPSS_IP_PROTOCOL_STACK_ENT protocol, GT_BOOL enable);


#ifdef __cplusplus
}
#endif

#endif
