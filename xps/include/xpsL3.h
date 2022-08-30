// xpsL3.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
 * \file xpsL3.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS L3 Route and Next Hop Manager
 */

#ifndef _xpsL3_h_
#define _xpsL3_h_

#include "xpTypes.h"
#include "xpEnums.h"
#include "xpsEnums.h"
#include "xpsCopp.h"
#include "xpsInterface.h"
#include "openXpsL3.h"
#include <fcntl.h>
#include <arpa/inet.h>
#include "cpss/dxCh/dxChxGen/ipLpmEngine/cpssDxChIpLpmTypes.h"
#ifdef __cplusplus
extern "C" {
#endif

#define INVALID_L3_INDEX   0xFFFFFFFF

#define XPS_CPSS_MAX_NUM_OF_LPM_BLOCKS_CNS 20
#define XPS_CPSS_DEFAULT_LPM_DB_ID 0
#define XPS_CPSS_DEFAULT_VRF_ID 0

#define XP_SAI_VRF_RANGE_START 0

typedef struct xpsL3RouteBulkInfo_t
{
    uint32_t vrfId;              ///< VRF
    xpIpPrefixType_t type;       ///< IP Prefix type
    ipv4Addr_t ipv4Addr;         ///< IPv4
    ipv6Addr_t ipv6Addr;         ///< IPv6
    uint32_t   ipMaskLen;        ///< IP Mask
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    bool override;
    uint32_t retStatus;
} xpsL3RouteBulkInfo_t;

typedef enum xpsArpState
{
    ARP_NOT_ACTIVE = 0,         /* Not Used by neibhor and NextHop. Should be destroyed.*/
    ARP_NEIGHBOR_ACTIVE = 1,    /* Arp Pointer is used by neighbor/host Entry*/
    ARP_NEXTHOP_ACTIVE = 2      /* Arp pointer is used by NextHop Entry*/
} xpsArpState_t;

/**
 * \struct xpsArpnfo_t
 *
 * This structure maintains Arp Pointer information
 * for each da Mac
 */
typedef struct xpsArpInfo_t
{
    macAddr_t    keyDaMac;
    /**
     *  @brief The ARP Pointer indicating the routed packet MAC DA
     *
     */
    xpsArpPointer_t arpPointer;
    uint32_t  arpState; /* to indicate if its used by neighbor or next hop. If not used, arp Info should be removed*/
    uint32_t refCount; /*reference count indicates NH/Neigh*/
} xpsArpInfo_t;

typedef struct xpsMac2MeInfo_t
{
    macAddr_t mac;
    xpsInterfaceId_t l3IntfId;
    uint8_t mac2meIndex;
    uint32_t refCount;
} xpsMac2MeInfo_t;

typedef struct xpsL3StatsInfo_s
{
    uint64_t rxPkts;
    uint64_t rxErrPkts;
    uint64_t rxOctets;
    uint64_t rxErrOctets;
    uint64_t txPkts;
    uint64_t txErrPkts;
    uint64_t txOctets;
    uint64_t txErrOctets;
} xpsL3StatsInfo_t;

/* ===========  FUNCTION POINTER DECLARATIONS =========== */
XP_STATUS xpsL3GetRouterAclEnable(xpsDevice_t devId,
                                  xpsInterfaceId_t l3IfId, uint8_t *enable);
XP_STATUS xpsL3GetRouterAclId(xpsDevice_t devId,
                              xpsInterfaceId_t l3IfId, uint32_t *aclId);
XP_STATUS xpsL3SetRouterAclEnable(xpsDevice_t devId,
                                  xpsInterfaceId_t l3IfId, uint8_t enable);
XP_STATUS xpsL3SetRouterAclId(xpsDevice_t devId,
                              xpsInterfaceId_t l3IfId, uint32_t groupId, uint32_t aclId);
/* ====================================================== */

/**
 * \brief This method get arp Index for MAc Da from SW DB
 *
 * \param [in] devId
 * \param [in] arpDaMac
 * \param [out] *routerArpIndex
 *
 * \return XP_STATUS
*/
XP_STATUS xpsArpGetArpPointer(xpsDevice_t devId, macAddr_t arpDaMac,
                              xpsArpPointer_t *routerArpIndex);

/**
 *  xpsIpRouterArpAddrRead function
 * @endinternal
 *
 * @brief   read a ARP MAC address to the router ARP / Tunnel
 *          start / NAT table (for NAT capable devices)
 *
 * @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  None.
 *
 * @param[in] devId                    - the device number
 * @param[in] routerArpIndex           - The Arp Address index (to be inserted later
 *                                      in the UC Route entry Arp nextHopARPPointer
 *                                       field)
 * @param[out] arpMacAddrPtr            - the ARP MAC address to
 *       write
 */
XP_STATUS xpsIpRouterArpAddRead(xpDevice_t devId,
                                xpsArpPointer_t routerArpIndex, macAddr_t *arpDaMac);

/**
 * \brief This method Creates an L3 sub interface for a scope.
 *
 * \param [in] scopeId
 * \param [out] *l3IntfId
 *
 * \return XP_STATUS
*/
XP_STATUS xpsL3CreateSubIntfScope(xpsScope_t scopeId,
                                  xpsInterfaceId_t *l3IntfId);

/**
 * \brief This method destroys L3 sub interface for a scope.
 *
 * \param [in] scopeId
 * \param [in] l3IntfId
 *
 * \return XP_STATUS
*/
XP_STATUS xpsL3DestroySubIntfScope(xpsScope_t scopeId,
                                   xpsInterfaceId_t l3IntfId);

/**
 * \brief This method binds L3 sub interface with the port interface
 *         over which it is created.
 *
 * \param [in] scopeId
 * \param [in] portIntfId
 * \param [in] l3IntfId
 * \param [in] encapId
 *
 * \return XP_STATUS
*/
XP_STATUS xpsL3BindSubIntfScope(xpsScope_t scopeId, xpsInterfaceId_t portIntfId,
                                xpsInterfaceId_t l3IntfId, xpsVlan_t encapId);

/**
 * \brief This method detaches L3 sub interface from the port interface
 *         over which it is created.
 *
 * \param [in] scopeId
 * \param [in] portIntfId
 * \param [in] l3IntfId
 * \param [in] encapId
 *
 * \return XP_STATUS
*/
XP_STATUS xpsL3UnBindSubIntfScope(xpsScope_t scopeId,
                                  xpsInterfaceId_t portIntfId, xpsInterfaceId_t l3IntfId,
                                  xpsVlan_t encapId);
/**
 * \brief This method Creates L3 Interface Over Port for a scope.
 *
 * \param [in] scopeId
 * \param [out] l3IntfId
 *
 * \return XP_STATUS
*/

XP_STATUS xpsL3CreatePortIntfScope(xpsScope_t scopeId,
                                   xpsInterfaceId_t *l3IntfId);

/**
 * \brief This method Removes L3 Interface Over Port for a scope.
 *
 * \param [in] scopeId
 * \param [out] *l3IntfId
 *
 * \return XP_STATUS
*/

XP_STATUS xpsL3DestroyPortIntfScope(xpsScope_t scopeId,
                                    xpsInterfaceId_t l3IntfId);

/**
 * \brief This method Initializes L3 Interface Over Port for a scope.
 *
 * \param [in] scopeId
 * \param [in] portIntfId
 * \param [in] l3IntfId
 *
 * \return XP_STATUS
*/

XP_STATUS xpsL3BindPortIntfScope(xpsScope_t scopeId,
                                 xpsInterfaceId_t portIntfId, xpsInterfaceId_t l3IntfId);

/**
 * \brief This method Initializes L3 Interface Over Port for a scope.
 *
 * \param [in] scopeId
 * \param [in] portIntfId
 * \param [out] l3IntfId
 *
 * \return XP_STATUS
*/

XP_STATUS xpsL3GetBindPortIntfScope(xpsScope_t scopeId,
                                    xpsInterfaceId_t portIntfId, xpsInterfaceId_t *l3IntfId);

/**
 * \brief This method De-Initializes L3 Interface Over Port for a scope.
 *
 * \param [in] scopeId
 * \param [in] portIntfId
 *
 * \return XP_STATUS
*/

XP_STATUS xpsL3UnBindPortIntfScope(xpsScope_t scopeId,
                                   xpsInterfaceId_t portIntfId);


/**
 * \brief This method Creates L3 Interface Over VLAN for a scope.
 *
 * \param [in] scopeId
 * \param [in] vlanId
 * \param [out] l3IntfId
 *
 * \return XP_STATUS
*/

XP_STATUS xpsL3CreateVlanIntfScope(xpsScope_t scopeId, xpsVlan_t vlanId,
                                   xpsInterfaceId_t *l3IntfId);

/**
 * \brief This method Removes L3 Interface Over VLAN for a scope.
 *
 * \param [in] scopeId
 * \param [in] vlanId
 * \param [out] l3IntfId
 *
 * \return XP_STATUS
*/

XP_STATUS xpsL3DestroyVlanIntfScope(xpsScope_t scopeId, xpsVlan_t vlanId,
                                    xpsInterfaceId_t l3IntfId);

/**
 * \brief This method Creates L3 Interface Over Tunnel for a scope.
 *
 * \param [in] scopeId
 * \param [out] l3IntfId
 *
 * \return XP_STATUS
*/

XP_STATUS xpsL3CreateTunnelIntfScope(xpsScope_t scopeId,
                                     xpsInterfaceId_t *l3IntfId);

/**
 * \brief This method Removes L3 Interface Over Tunnel for a scope.
 *
 * \param [in] scopeId
 * \param [out] l3IntfId
 *
 * \return XP_STATUS
*/

XP_STATUS xpsL3DestroyTunnelIntfScope(xpsScope_t scopeId,
                                      xpsInterfaceId_t l3IntfId);

/**
 * \brief This method Initializes L3 Interface Over Tunnel.
 *
 * \param [in] devId
 * \param [in] l3IntfId
 *
 * \return XP_STATUS
*/

XP_STATUS xpsL3InitTunnelIntf(xpsDevice_t devId, xpsInterfaceId_t l3IntfId);

/**
 * \brief This method De-Initializes L3 Interface Over Tunnel.
 *
 * \param [in] devId
 * \param [in] l3IntfId
 *
 * \return XP_STATUS
*/

XP_STATUS xpsL3DeInitTunnelIntf(xpsDevice_t devId, xpsInterfaceId_t l3IntfId);

/**
 * \brief This method Bind L3 Interface To Tunnel.
 *
 * \param [in] devId
 * \param [in] tnnlIntfId
 * \param [in] l3IntfId
 *
 * \return XP_STATUS
*/


XP_STATUS xpsL3BindTunnelIntf(xpsDevice_t devId, xpsInterfaceId_t tnnlIntfId,
                              xpsInterfaceId_t l3IntfId);

/**
 * \brief This method UnBind L3 Interface From Tunnel.
 *
 * \param [in] devId
 * \param [in] tnnlIntfId
 *
 * \return XP_STATUS
*/

XP_STATUS xpsL3UnBindTunnelIntf(xpsDevice_t devId, xpsInterfaceId_t tnnlIntfId);
/**
 * \brief This method Creates L3 Interface Over Vpn for a scope.
 *
 * \param [in] scopeId
 * \param [out] l3IntfId
 *
 * \return XP_STATUS
*/

XP_STATUS xpsL3CreateVpnIntfScope(xpsScope_t scopeId,
                                  xpsInterfaceId_t *l3IntfId);

/**
 * \brief This method Removes L3 Interface Over Vpn for a scope.
 *
 * \param [in] scopeId
 * \param [out] l3IntfId
 *
 * \return XP_STATUS
*/

XP_STATUS xpsL3DestroyVpnIntfScope(xpsScope_t scopeId,
                                   xpsInterfaceId_t l3IntfId);

/**
 * \brief This method Initializes L3 Interface Over Vpn.
 *
 * \param [in] devId
 * \param [in] l3IntfId
 *
 * \return XP_STATUS
*/

XP_STATUS xpsL3InitVpnIntf(xpsDevice_t devId, xpsInterfaceId_t l3IntfId);

/**
 * \brief This method De-Initializes L3 Interface Over Vpn.
 *
 * \param [in] devId
 * \param [in] l3IntfId
 *
 * \return XP_STATUS
*/

XP_STATUS xpsL3DeInitVpnIntf(xpsDevice_t devId, xpsInterfaceId_t l3IntfId);

/**
 * \brief This method Creates VPN Interface association from VPN Label.
 *
 * \param [in] devId
 * \param [in] vpnLabel
 * \param [in] l3IntfId
 *
 * \return XP_STATUS
*/

XP_STATUS xpsL3BindVpnIntfToLabel(xpsDevice_t devId, uint32_t vpnLabel,
                                  xpsInterfaceId_t l3IntfId);

/**
 * \brief This method Removes VPN Interface association from VPN Label.
 *
 * \param [in] devId
 * \param [in] vpnLabel
 *
 * \return XP_STATUS
*/


XP_STATUS xpsL3UnBindVpnIntfFromLabel(xpsDevice_t devId, uint32_t vpnLabel);

/**
 * \brief This method configures the VRF index for routing
 *        interface created on this vpn interface.
 *
 * \param [in] devId
 * \param [in] vpnIntfId
 * \param [in] vrfId
 *
 * \return XP_STATUS
*/
XP_STATUS xpsL3SetVpnIntfVrf(xpsDevice_t devId, xpsInterfaceId_t vpnIntfId,
                             uint32_t vrfId);

/**
 * \brief This method returns the configured VRF index for
 *        the vpn instance.
 *
 * \param [in] devId
 * \param [in] vpnIntfId
 * \param [out] vrfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3GetVpnIntfVrf(xpsDevice_t devId, xpsInterfaceId_t vpnIntfId,
                             uint32_t *vrfId);

/**
 * \brief This method Removes Global Router Mac.
 *
 * \param [in] devId Device ID
 * \param [in] mac Mac Address
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3RemoveIngressRouterMac(xpsDevice_t devId, macAddr_t mac);

/**
 * \brief This method Removes Global Router Mac for a given VLAN.
 *
 * \param [in] devId Device ID
 * \param [in] vlan VLAN-ID
 * \param [in] mac Mac Address
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3RemoveIngressRouterVlanMac(xpsDevice_t devId, xpsVlan_t vlan,
                                          macAddr_t mac);

/**
 * \brief Add a Router MAC for a specific L3 Interface
 *
 * \param [in] devId Device ID
 * \param [in] l3IntfId Layer 3 Interface ID
 * \param [in] mac Mac Address
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3AddIntfIngressRouterMac(xpsDevice_t devId,
                                       xpsInterfaceId_t l3IntfId, macAddr_t mac);

/**
 * \brief This method Removes Per L3 Interface Router Mac.
 *
 * \param [in] devId Device ID
 * \param [in] l3IntfId Layer 3 Interface ID
 * \param [in] mac Mac Address
 *
 * \return XP_STATUS
*/
XP_STATUS xpsL3RemoveIntfIngressRouterMac(xpsDevice_t devId,
                                          xpsInterfaceId_t l3IntfId, macAddr_t mac);

/**
 * \brief This method Get Constant Upper 40 MSB's of Egress Router Mac.
 *
 *
 * \param [in] devId
 * \param [out] macHi
 * \return XP_STATUS
*/
XP_STATUS xpsL3GetEgressRouterMacMSbs(xpsDevice_t devId,  macAddrHigh_t *macHi);

/**
 * \brief This method get LSB of Router/Mc MAC for given  L3 Interface.
 *
 *
 * \param [in] devId
 * \param [in] l3IntfId
 * \param [in] *macSa
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3GetIntfEgressRouterMacLSB(xpsDevice_t devId,
                                         xpsInterfaceId_t l3IntfId, macAddrLow_t *macSa);


/**
 * \brief This method returns the configured VRF index for
 *        the L3 Interface.
 *
 * \param [in] devId
 * \param [in] l3IntfId
 * \param [out] *vrfId
 *
 * \return XP_STATUS
*/
XP_STATUS xpsL3GetIntfVrf(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                          uint32_t *vrfId);


/**
 * \brief This method returns the configured status of IPv4
 *        unicast routing flag on the L3 Interface.
 *
 *
 * \param [in] devId
 * \param [in] l3IntfId
 * \param [out] *enable
 *
 * \return XP_STATUS
*/
XP_STATUS xpsL3GetIntfIpv4UcRoutingEn(xpsDevice_t devId,
                                      xpsInterfaceId_t l3IntfId, uint32_t *enable);


/**
 * \brief This method returns the configured status of IPv6
 *        unicast routing flag on the L3 Interface.
 *
 *
 * \param [in] devId
 * \param [in] l3IntfId
 * \param [out] *enable
 *
 * \return XP_STATUS
*/
XP_STATUS xpsL3GetIntfIpv6UcRoutingEn(xpsDevice_t devId,
                                      xpsInterfaceId_t l3IntfId, uint32_t *enable);

/**
 * \brief This method enables Mpls Label routing lookups
 *        for packets ingressing on this L3 Interface.
 *
 *
 * \param [in] devId
 * \param [in] l3IntfId
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3SetIntfMplsRoutingEn(xpsDevice_t devId,
                                    xpsInterfaceId_t l3IntfId, uint32_t enable);

/**
 * \brief This method returns the configured status of Mpls
 *        routing flag on the L3 Interface.
 *
 *
 * \param [in] devId
 * \param [in] l3IntfId
 * \param [out] *enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3GetIntfMplsRoutingEn(xpsDevice_t devId,
                                    xpsInterfaceId_t l3IntfId, uint32_t *enable);


/**
 * \brief This method enables IPv4 multicast routing lookups
 *        for packets ingressing on this L3 Interface.
 *
 *
 * \param [in] devId
 * \param [in] l3IntfId
 * \param [in] enable
 *
 * \return XP_STATUS
*/
XP_STATUS xpsL3SetIntfIpv4McRoutingEn(xpsDevice_t devId,
                                      xpsInterfaceId_t l3IntfId, uint32_t enable);

/**
 * \brief This method returns the configured status of IPv4
 *        multicast routing flag on the L3 Interface.
 *
 *
 * \param [in] devId
 * \param [in] l3IntfId
 * \param [out] *enable
 *
 * \return XP_STATUS
*/
XP_STATUS xpsL3GetIntfIpv4McRoutingEn(xpsDevice_t devId,
                                      xpsInterfaceId_t l3IntfId, uint32_t *enable);

/**
 * \brief This method enables Ipv6 multicast routing lookups
 *        for packets ingressing on this L3 Interface.
 *
 *
 * \param [in] devId
 * \param [in] l3IntfId
 * \param [in] enable
 *
 * \return XP_STATUS
*/
XP_STATUS xpsL3SetIntfIpv6McRoutingEn(xpsDevice_t devId,
                                      xpsInterfaceId_t l3IntfId, uint32_t enable);

/**
 * \brief This method returns the configured status of IPv6
 *        multicast routing flag on the L3 Interface.
 *
 *
 * \param [in] devId
 * \param [in] l3IntfId
 * \param [out] *enable
 *
 * \return XP_STATUS
*/
XP_STATUS xpsL3GetIntfIpv6McRoutingEn(xpsDevice_t devId,
                                      xpsInterfaceId_t l3IntfId, uint32_t *enable);

/**
 * \brief This method enables IPv4 multicast routing lookups
 *        for packets ingressing on this L3 Interface.
 *
 *
 * \param [in] devId
 * \param [in] l3IntfId
 * \param [in] mode
 *
 * \return XP_STATUS
*/
XP_STATUS xpsL3SetIntfIpv4McRouteMode(xpsDevice_t devId,
                                      xpsInterfaceId_t l3IntfId, xpVlanRouteMcMode_t mode);

/**
 * \brief This method returns the configured status of IPv4
 *        multicast routing flag on the L3 Interface.
 *
 *
 * \param [in] devId
 * \param [in] l3IntfId
 * \param [out] *mode
 *
 * \return XP_STATUS
*/
XP_STATUS xpsL3GetIntfIpv4McRouteMode(xpsDevice_t devId,
                                      xpsInterfaceId_t l3IntfId, xpVlanRouteMcMode_t *mode);

/**
 * \brief This method enables IPv6 multicast routing lookups
 *        for packets ingressing on this L3 Interface.
 *
 *
 * \param [in] devId
 * \param [in] l3IntfId
 * \param [in] mode
 *
 * \return XP_STATUS
*/
XP_STATUS xpsL3SetIntfIpv6McRouteMode(xpsDevice_t devId,
                                      xpsInterfaceId_t l3IntfId, xpVlanRouteMcMode_t mode);

/**
 * \brief This method returns the configured status of IPv6
 *        multicast routing flag on the L3 Interface.
 *
 *
 * \param [in] devId
 * \param [in] l3IntfId
 * \param [out] *mode
 *
 * \return XP_STATUS
*/
XP_STATUS xpsL3GetIntfIpv6McRouteMode(xpsDevice_t devId,
                                      xpsInterfaceId_t l3IntfId, xpVlanRouteMcMode_t *mode);

/**
 * \brief This method enables or disables IPv4 URPF over an L3 interface.
 *
 *
 * \param [in] devId
 * \param [in] l3IntfId
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3SetIpv4UrpfEnable(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                                 uint32_t enable);

/**
 * \brief This method returns the configured value of IPv4
 *        URPF enable flag over an L3 interface.
 *
 *
 * \param [in] devId
 * \param [in] l3IntfId
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3GetIpv4UrpfEnable(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                                 uint32_t *enable);

/**
 * \brief This method sets IPv4 URPF mode over an L3 interface.
 *          in Strict mode ingress vif and Source IP check is performed.
 *          in Loose mode only Source IP check is performed.
 *
 *
 * \param [in] devId
 * \param [in] l3IntfId
 * \param [in] urpfMode
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3SetIpv4UrpfMode(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                               xpsUrpfMode_e urpfMode);

/**
 * \brief This method returns the configured value of IPv4 URPF mode over
 *         an L3 interface.
 *
 *
 * \param [in] devId
 * \param [in] l3IntfId
 * \param [out] urpfMode
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3GetIpv4UrpfMode(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                               xpsUrpfMode_e *urpfMode);

/**
 * \brief This method enables or disables IPv6 URPF over an L3 interface.
 *
 *
 * \param [in] devId
 * \param [in] l3IntfId
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3SetIpv6UrpfEnable(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                                 uint32_t enable);


/**
 * \brief This method returns the configured value of IPv6
 *        URPF enable flag over a L3 interface.
 *
 *
 * \param [in] devId
 * \param [in] l3IntfId
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3GetIpv6UrpfEnable(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                                 uint32_t *enable);

/**
 * \brief This method sets IPv6 URPF mode over a L3 interface.
 *          in Strict mode ingress vif and Source IP check is performed.
 *          in Loose mode only Source IP check is performed.
 *
 *
 * \param [in] devId
 * \param [in] l3IntfId
 * \param [in] urpfMode
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3SetIpv6UrpfMode(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                               xpsUrpfMode_e urpfMode);

/**
 * \brief This method returns the configured value of IPv6 URPF mode over
 *         a L3 interface.
 *
 *
 * \param [in] devId
 * \param [in] l3IntfId
 * \param [out] urpfMode
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3GetIpv6UrpfMode(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                               xpsUrpfMode_e *urpfMode);

/**
 * \brief This method enables/disables counter for ingress/egress packets to an l3 interface.
 *
 *
 * \param [in] devId
 * \param [in] l3IntfId
 * \param [in] dir
 * \param [in] enable
 *
 * \return XP_STATUS
*/
XP_STATUS xpsL3SetIntfCounterEn(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                                xpsDirection_t dir, uint32_t enable);

/**
 * \brief This method gets the enable status of  counter for ingress/egress packets to an l3 interface.
 *
 *
 * \param [in] devId
 * \param [in] l3IntfId
 * \param [in] dir
 * \param [in] enable
 *
 * \return XP_STATUS
*/
XP_STATUS xpsL3GetIntfCounterEn(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                                xpsDirection_t dir, uint32_t *enable);

/**
 * \brief This method sets the counter id for egress packets to an l3 interface.
 *
 *
 * \param [in] devId
 * \param [in] l3IntfId
 * \param [in] counterId
 *
 * \return XP_STATUS
*/
XP_STATUS xpsL3SetIntfEgressCounterId(xpsDevice_t devId,
                                      xpsInterfaceId_t l3IntfId, uint32_t counterId);

/**
 * \brief This method gets the counter id for engress packets to an l3 interface.
 *
 * \param [in] devId
 * \param [in] l3IntfId
 * \param [out] counterId
 *
 * \return XP_STATUS
*/
XP_STATUS xpsL3GetIntfEgressCounterId(xpsDevice_t devId,
                                      xpsInterfaceId_t l3IntfId, uint32_t *counterId);

/**
 *\brief This method gets the egress Vif an L3 interfaces when
 * the L3 interface is built over tunnels and ports. In those
 * cases, the L3 interface has only one endpoint and this API
 * gets portVif or the tunnelVif. If the L3 interface is built
 * over VLAN, this will return the flood vif.
 *
 * \param [in] devId
 * \param [in] l3IntfId
 * \param [in] egressVif
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3GetIntfEgressVif(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                                xpsInterfaceId_t *egressVif);

/**
 * \brief This method sets the Nat mode  over L3 interface
 * \param [in] devId
 * \param [in] l3IntfId
 * \param [in] mode
 */
XP_STATUS xpsL3SetIntfNatMode(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                              xpsVlanNatMode_e mode);

/**
 * \brief This method gets the Nat mode  over L3 interface
 * \param [in] devId
 * \param [in] l3IntfId
 * \param [out] mode
 */
XP_STATUS xpsL3GetIntfNatMode(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                              xpsVlanNatMode_e *mode);

/**
 * \brief This method sets the Nat Scope over L3 Interface
 * \param [in] devId
 * \param [in] l3IntfId
 * \param [in] scope
 */
XP_STATUS xpsL3SetIntfNatScope(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                               uint32_t scope);

/**
 * \brief This method sets the Nat Scope over L3 Interface
 * \param [in] devId
 * \param [in] l3IntfId
 * \param [out] scope
 */
XP_STATUS xpsL3GetIntfNatScope(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                               uint32_t *scope);


XP_STATUS xpsL3InitIpHostScope(xpsScope_t scopeId);

/**
 * \brief This method cleans up from the L3 Host Mgr system specific
 *        references to various primitive managers for a scope.
 *
 * \param [in] scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3DeInitIpHostScope(xpsScope_t scopeId);

/**
 * \brief This method configures and add device specific primitives
 *        required for the L3 Host manager.
 *
 * \param [in] devId
 * \param [in] initType
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3AddIpHostDevice(xpsDevice_t devId, xpsInitType_t initType);

/**
 * \brief This method removes device specific primitives
 *        required for the L3 Host manager.
 *
 * \param [in] devId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3RemoveIpHostDevice(xpsDevice_t devId);

/**
 * \brief This method Removes the L3 Host Entry
 *
 * \param [in] devId
 * \param [in] *pL3HostEntry
 *
 * \return XP_STATUS
 */

XP_STATUS xpsL3RemoveIpHostEntry(xpsDevice_t devId,
                                 xpsL3HostEntry_t *pL3HostEntry);

/**
 * \brief This method Updates the L3 Host Entry
 *
 * \param [in] devId
 * \param [in] *pL3HostEntry
 *
 * \return XP_STATUS
 */

XP_STATUS xpsL3UpdateIpHostEntry(xpsDevice_t devId,
                                 xpsL3HostEntry_t *pL3HostEntry);

/**
 * \brief This method Gets the number of L3 ip Host Entries
 *
 * \param [in] devId
 * \param [in] type Prefix Type
 * \param [out] *numOfValidEntries
 *
 * \return XP_STATUS
 */

XP_STATUS xpsL3GetNumOfValidIpHostEntries(xpsDevice_t devId,
                                          xpIpPrefixType_t type, uint32_t *numOfValidEntries);

/**
 * \brief This method Gets the number of L3 ip Host Entries
 *
 * \param [in] devId
 * \param [out] *numOfValidEntries
 *
 * \return XP_STATUS
 */

XP_STATUS xpsL3GetNumOfValidIpHostEntries(xpsDevice_t devId,
                                          xpIpPrefixType_t type, uint32_t *numOfValidEntries);

/**
 * \brief reads number of valid entries in NH table
 *
 * \param [in] deviceId   Device id
 * \param [out] numOfValidEntries valid entry count in nh table
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3GetNumOfValidNhEntries(xpsDevice_t devId,
                                      uint32_t *numOfValidEntries);

/**
 * \public
 * \brief Gets the size of NH table.
 *
 * This method calculates size of NH table.
 *
 * \param [in] devId
 * \param [out] tsize
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3NhGetTableSize(xpsDevice_t devId, uint32_t *tsize);

/**
 * \brief This method Get the L3 Host Entry and Return the fields
 *
 * \param [in] devId
 * \param [in,out] pL3HostEntry
 *
 * \return XP_STATUS
 */

XP_STATUS xpsL3GetIpHostEntry(xpsDevice_t devId,
                              xpsL3HostEntry_t *pL3HostEntry);

/**
 * \brief This method Get the L3 Host Entry and Return the fields at the provided index.
 *
 * \param [in] devId
 * \param [in] index
 * \param [in,out] pL3HostEntry
 *
 * \return XP_STATUS
 */

XP_STATUS xpsL3GetIpHostEntryByIndex(xpsDevice_t devId, uint32_t index,
                                     xpsL3HostEntry_t *pL3HostEntry);

/**
 * \brief This method Finds the L3 Host Entry and Return the fields
 *
 * \param [in] devId
 * \param [in] *pL3HostEntry
 * \param [out] *index
 *
 * \return XP_STATUS
 */

XP_STATUS xpsL3FindIpHostEntry(xpsDevice_t devId,
                               xpsL3HostEntry_t *pL3HostEntry, int *index);

/**
 * \brief This method flushes the L3 Host Entries
 *
 * \param [in] devId
 * \param [in] pL3HostEntry
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3FlushIpHostEntries(xpDevice_t devId,
                                  xpsL3HostEntry_t *pL3HostEntry);

/**
 * \brief This method flushes the L3 Host Entries and
 *        update SW shadow if updateShadow == 1
 *
 * \param [in] devId
 * \param [in] pL3HostEntry
 * \param [in] updateShadow
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3FlushIpHostEntriesWithShadowUpdate(xpDevice_t devId,
                                                  xpsL3HostEntry_t *pL3HostEntry, uint8_t updateShadow);

/**
 * \brief This method is used to get reason code for an IPv4
 *        host entry.
 *
 * \param [in] devId
 * \param [in] pL3HostEntry
 * \param [out] reasonCode
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3GetIpHostControlEntryReasonCode(xpsDevice_t devId,
                                               xpsL3HostEntry_t *pL3HostEntry, uint32_t *reasonCode);

/**
 * \brief This method initializes L3 Route Mgr with system specific
 *        references to various primitive managers and their initialization
 *        for a scope.
 *
 * \param [in] scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3InitIpRouteScope(xpsScope_t scopeId);

/**
 * \brief This method cleans up from the L3 Route Mgr system specific
 *        references to various primitive managers for a scope.
 *
 * \param [in] scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3DeInitIpRouteScope(xpsScope_t scopeId);

/**
 * \brief This method configures and add device specific primitives
 *        required for the L3 Route manager.
 *
 * \param [in] devId
 * \param [in] initType
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3AddIpRouteDevice(xpsDevice_t devId, xpsInitType_t initType);

/**
 * \brief This method removes device specific primitives
 *        required for the L3 Route manager.
 *
 * \param [in] devId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3RemoveIpRouteDevice(xpsDevice_t devId);


/**
 * \brief This method Removes the L3 Route Entry at the provided index
 *
 * \param [in] devId
 * \param [in] prfxBucketIdx
 * \param [in] *pL3RouteEntry
 *
 * \return XP_STATUS
 */


XP_STATUS xpsL3RemoveIpRouteEntryByIndex(xpsDevice_t devId,
                                         uint32_t prfxBucketIdx, xpsL3RouteEntry_t *pL3RouteEntry);
/**
 * \brief This method Updates the L3 Route Entry at the provided index
 *
 * \param [in] devId
 * \param [in] prfxBucketIdx
 * \param [in] *pL3RouteEntry
 *
 * \return XP_STATUS
 */

XP_STATUS xpsL3UpdateIpRouteEntryByIndex(xpsDevice_t devId,
                                         uint32_t prfxBucketIdx, xpsL3RouteEntry_t *pL3RouteEntry);


/**
 * \brief This method Finds the L3 Route Entry and Return the fields
 *
 * \param [in] devId
 * \param [in,out] pL3RouteEntry
 * \param [out] prfxBucketIdx
 *
 * \return XP_STATUS
 */

XP_STATUS xpsL3FindIpRouteEntry(xpsDevice_t devId,
                                xpsL3RouteEntry_t *pL3RouteEntry, uint32_t *prfxBucketIdx);


/**
 * \brief This method Finds the L3 Route Longest Prefix Match Entry and Return the fields
 *
 * \param [in] devId
 * \param [in,out] pL3RouteEntry
 * \param [out] prfxBucketIdx
 *
 * \return XP_STATUS
 */

XP_STATUS xpsL3FindIpRouteLpmEntry(xpsDevice_t devId,
                                   xpsL3RouteEntry_t *pL3RouteEntry, uint32_t *prfxBucketIdx);

/**
 * \brief This method flushes Route Entries
 *
 * \param [in] devId
 * \param [in] pL3RouteEntry
 *
 * \return XP_STATUS
 */

XP_STATUS xpsL3FlushIpRouteEntries(xpDevice_t devId,
                                   xpsL3RouteEntry_t *pL3RouteEntry);

/**
 * \brief This method Gets the number of L3 ip Route Entries
 *
 * \param [in] devId
 * \param [in] type Prefix Type
 * \param [out] *numOfValidEntries
 *
 * \return XP_STATUS
 */

XP_STATUS xpsL3GetNumOfValidIpRouteEntries(xpsDevice_t devId,
                                           xpIpPrefixType_t type, uint32_t *numOfValidEntries);

/**
 * \brief This displays Route Entries
 *
 * \param [in] devId
 * \param [in] type
 * \param [in] endIndex
 *
 * \return XP_STATUS
 */

XP_STATUS xpsDisplayRouteTable(xpDevice_t devId, xpIpPrefixType_t type,
                               uint32_t endIndex);

/**
 * \brief This method initializes Route Nexthop Mgr with system specific
 *        references to various primitive managers and their initialization
 *        for a scope.
 *
 * \param [in] scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3InitRouteNextHopScope(xpsScope_t scopeId);

/**
 * \brief This method cleans up from the Route Nexthop Mgr system specific
 *        references to various primitive managers for a scope.
 *
 * \param [in] scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3DeInitRouteNextHopScope(xpsScope_t scopeId);

/**
 * \brief This method configures and add device specific primitives
 *        required for the Route Nexthop manager.
 *
 * \param [in] devId
 * \param [in] initType
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3AddRouteNextHopDevice(xpsDevice_t devId, xpsInitType_t initType);

/**
 * \brief This method removes device specific primitives
 *        required for the Route Nexthop manager.
 *
 * \param [in] devId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3RemoveRouteNextHopDevice(xpsDevice_t devId);

/**
 * \brief This method allocates NextHop for given ECMP Size for a scope.
 *
 * \param [in] scopeId
 * \param [out] nhId
 * \param [in] nhEcmpSize
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3CreateRouteNextHopScope(xpsScope_t scopeId, uint32_t nhEcmpSize,
                                       uint32_t *nhId);

/**
* \brief This method allocates NextHop with given nh Id.
*
* \param [in] scopeId
* \param [in] nhId
*
* \return XP_STATUS
*/

XP_STATUS xpsL3CreateRouteNextHopWithIdScope(xpsScope_t scopeId, uint32_t nhId);

/**
* \brief This method allocates NextHop with given nh Id.
*
* \param [in] nhId
*
* \return XP_STATUS
*/

XP_STATUS xpsL3CreateRouteNextHopWithId(uint32_t nhId);

/**
 * \brief This method Set the L3VPN NextHop Entry for given NH Id
 *
 * \param [in] devId
 * \param [in] nhId
 * \param [in] *pL3NextHopEntry
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3SetRouteVpnNextHop(xpsDevice_t devId, uint32_t nhId,
                                  xpsL3NextHopEntry_t *pL3NextHopEntry);

/**
 * \brief This method Get the NextHop Entry for given NH Id
 *
 * \param [in] devId
 * \param [in] nhId
 * \param [out] *pL3NextHopEntry
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3GetRouteNextHop(xpsDevice_t devId, uint32_t nhId,
                               xpsL3NextHopEntry_t *pL3NextHopEntry);

/**
 * \brief This method free the allocated NextHop for a scope.
 *
 * \param [in] scopeId
 * \param [in] nhEcmpSize
 * \param [in] nhId
 *
 * \return XP_STATUS
 */

XP_STATUS xpsL3DestroyRouteNextHopScope(xpsScope_t scopeId, uint32_t nhEcmpSize,
                                        uint32_t nhId);

/**
 * \brief This method allocates Next Hop Group for given ECMP Size for a scope
 *
 * \param [in]  scopeId    Scope ID
 * \param [in]  nhEcmpSize ECMP size
 * \param [out] nhGrpId    Next Hop Group ID
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3CreateNextHopGroupScope(xpsScope_t scopeId, uint32_t nhEcmpSize,
                                       uint32_t *nhGrpId);

/**
 * \brief This method allocates Next Hop Group for given ECMP size
 *
 * \param [in]  nhEcmpSize ECMP size
 * \param [out] nhGrpId    Next Hop Group ID
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3CreateNextHopGroup(uint32_t nhEcmpSize, uint32_t *nhGrpId);

/**
 * \brief This method free the allocated Next Hop Group for given ECMP size for a scope
 *
 * \param [in]  scopeId    Scope ID
 * \param [in]  nhEcmpSize ECMP size
 * \param [out] nhGrpId    Next Hop Group ID
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3DestroyNextHopGroupScope(xpsScope_t scopeId, uint32_t nhEcmpSize,
                                        uint32_t nhGrpId);

/**
 * \brief This method free the allocated Next Hop Group for given ECMP size
 *
 * \param [in]  nhEcmpSize ECMP size
 * \param [out] nhGrpId    Next Hop Group ID
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3DestroyNextHopGroup(uint32_t nhEcmpSize, uint32_t nhGrpId);

/**
 * \brief This is a method to get the L3 Mcast Domain Id from the BD Table entry
 * associated with the VRF
 * \param [in] devId
 * \param [in] intfId
 * \param [out] mcL3DomainId
 * \return XP_STATUS
 *
 */
XP_STATUS xpsL3GetV6McL3DomainIdForInterface(xpDevice_t devId,
                                             xpsInterfaceId_t intfId, xpMcastDomainId_t *mcL3DomainId);

/**
 * \brief This is a method to set the L3 Mcast Domain Id from the BD Table entry
 * associated with the VRF
 * \param [in] devId
 * \param [in] intfId
 * \param [out] mcL3DomainId
 * \return XP_STATUS
 *
 */
XP_STATUS xpsL3SetV6McL3DomainIdForInterface(xpDevice_t devId,
                                             xpsInterfaceId_t intfId, xpMcastDomainId_t mcL3DomainId);

/**
 * \brief This method Creates L3 Interface Over Port.
 *
 * \param [out] l3IntfId
 *
 * \return XP_STATUS
*/
XP_STATUS xpsL3CreatePortIntf(xpsInterfaceId_t *l3IntfId);

/**
 * \brief This method Removes L3 Interface Over Port.
 *
 * \param [out] l3IntfId
 *
 * \return XP_STATUS
*/
XP_STATUS xpsL3DestroyPortIntf(xpsInterfaceId_t l3IntfId);

/**
 * \brief This method Initializes L3 Interface Over Port.
 *
 * \param [in] portIntfId
 * \param [in] l3IntfId
 *
 * \return XP_STATUS
*/
XP_STATUS xpsL3BindPortIntf(xpsInterfaceId_t portIntfId,
                            xpsInterfaceId_t l3IntfId);

/**
 * \brief This method Initializes L3 Interface Over Port.
 *
 * \param [in] portIntfId
 * \param [out] l3IntfId
 *
 * \return XP_STATUS
*/
XP_STATUS xpsL3GetBindPortIntf(xpsInterfaceId_t portIntfId,
                               xpsInterfaceId_t *l3IntfId);

/**
 * \brief This method Creates L3 Interface Over Tunnel.
 *
 * \param [out] l3IntfId
 *
 * \return XP_STATUS
*/
XP_STATUS xpsL3CreateTunnelIntf(xpsInterfaceId_t *l3IntfId);

/**
 * \brief This method Removes L3 Interface Over Tunnel.
 *
 * \param [in] l3IntfId
 *
 * \return XP_STATUS
*/
XP_STATUS xpsL3DestroyTunnelIntf(xpsInterfaceId_t l3IntfId);

/**
 * \brief This method Creates L3 Interface Over Vpn.
 *
 * \param [out] l3IntfId
 *
 * \return XP_STATUS
*/
XP_STATUS xpsL3CreateVpnIntf(xpsInterfaceId_t *l3IntfId);

/**
 * \brief This method Removes L3 Interface Over Vpn.
 *
 * \param [out] l3IntfId
 *
 * \return XP_STATUS
*/
XP_STATUS xpsL3DestroyVpnIntf(xpsInterfaceId_t l3IntfId);

/**
 * \brief This method initializes L3 Host Mgr with system specific
 *        references to various primitive managers and their initialization.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3InitIpHost(void);

/**
 * \brief This method cleans up from the L3 Host Mgr system specific
 *        references to various primitive managers.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3DeInitIpHost(void);

/**
 * \brief This method initializes L3 Route Mgr with system specific
 *        references to various primitive managers and their initialization.
 *
 * \param [in] scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3InitIpRoute(void);

/**
 * \brief This method cleans up from the L3 Route Mgr system specific
 *        references to various primitive managers.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3DeInitIpRoute(void);

/**
 * \brief This method initializes Route Nexthop Mgr with system specific
 *        references to various primitive managers and their initialization.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3InitRouteNextHop(void);

/**
 * \brief This method cleans up from the Route Nexthop Mgr system specific
 *        references to various primitive managers.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3DeInitRouteNextHop(void);

/**
 * Set the ipv4 MTU length for an l2 domain.
 *
 * \param [in] devId
 * \param [in] intfId
 * \param [in] ipv4MtuLen
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3SetIpv4MtuLenForInterface(xpDevice_t devId,
                                         xpsInterfaceId_t intfId, uint32_t ipv4MtuLen);

/**
 * Get the ipv4 MTU length for an l2 domain.
 *
 * \param [in] devId
 * \param [in] intfId
 * \param [out] ipv4MtuLen
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3GetIpv4MtuLenForInterface(xpDevice_t devId,
                                         xpsInterfaceId_t intfId, uint32_t *ipv4MtuLen);

/**
 * Set the ipv6 MTU length for an l2 domain.
 *
 * \param [in] devId
 * \param [in] intfId
 * \param [in] ipv6MtuLen
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3SetIpv6MtuLenForInterface(xpDevice_t devId,
                                         xpsInterfaceId_t intfId, uint32_t ipv6MtuLen);

/**
 * Get the ipv6 MTU length for an l2 domain.
 *
 * \param [in] devId
 * \param [in] intfId
 * \param [out] ipv6MtuLen
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3GetIpv6MtuLenForInterface(xpDevice_t devId,
                                         xpsInterfaceId_t intfId, uint32_t *ipv6MtuLen);

/**
 * Set the MTU Packet length for an l2 domain.
 *
 * \param [in] devId
 * \param [in] intfId
 * \param [in] mtuPktCmd
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3SetMtuPktCmdForInterface(xpDevice_t devId,
                                        xpsInterfaceId_t intfId, uint32_t mtuPktCmd);

/**
 * Get the MTU Packet length for an l2 domain.
 *
 * \param [in] devId
 * \param [in] intfId
 * \param [out] mtuPktCmd
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3GetMtuPktCmdForInterface(xpDevice_t devId,
                                        xpsInterfaceId_t intfId, uint32_t *mtuPktCmd);

/**
 * Get VlanId from l3 Interface Id.
 *
 * \param [in] intfId
 * \param [out] vlanId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3GetVlanIdForL3Interface(xpsInterfaceId_t intfId,
                                       xpsVlan_t* vlanId);

/**
 * Get VlanId from l3 Interface Id for a scope
 *
 * \param [in] scopeId
 * \param [in] intfId
 * \param [out] vlanId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3GetVlanIdForL3InterfaceScope(xpsScope_t scopeId,
                                            xpsInterfaceId_t intfId, xpsVlan_t* vlanId);

/**
* Set the IGMP pkt commad
*
* \param [in] devId
* \param [in] l3IntfId
* \param [in] pktCmd
*
* \return XP_STATUS
*/

XP_STATUS xpsL3SetIgmpCmd(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                          xpsPktCmd_e pktCmd);
/**
* Get the IGMP pkt commad
*
* \param [in] devId
* \param [in] l3IntfId
* \param [out] pktCmd
*
* \return XP_STATUS
*/
XP_STATUS xpsL3GetIgmpCmd(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                          xpsPktCmd_e *pktCmd);

/**
* Set the ICMPv6 pkt commad
*
* \param [in] devId
* \param [in] l3IntfId
* \param [in] pktCmd
*
* \return XP_STATUS
*/

XP_STATUS xpsL3SetIcmpv6Cmd(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                            xpsPktCmd_e pktCmd);
/**
* Get the ICMPv6 pkt commad
*
* \param [in] devId
* \param [in] l3IntfId
* \param [out] pktCmd
*
* \return XP_STATUS
*/
XP_STATUS xpsL3GetIcmpv6Cmd(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                            xpsPktCmd_e *pktCmd);

/**
 * \brief This method sets rehash level for IPv4 Host table
 *
 * \param [in] devId
 * \param [in] numOfRehashLevels
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3SetIpv4RehashLevel(xpDevice_t devId, uint8_t numOfRehashLevels);

/**
 * \brief This method gets rehash level for IPv4 Host table
 *
 * \param [in] devId
 * \param [out] numOfRehashLevels
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3GetIpv4RehashLevel(xpDevice_t devId, uint8_t* numOfRehashLevels);

/**
 * \brief This method sets rehash level for IPv6 Host table
 *
 * \param [in] devId
 * \param [in] numOfRehashLevels
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3SetIpv6RehashLevel(xpDevice_t devId, uint8_t numOfRehashLevels);

/**
 * \brief This method gets rehash level for IPv6 Host table
 *
 * \param [in] devId
 * \param [out] numOfRehashLevels
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3GetIpv6RehashLevel(xpDevice_t devId, uint8_t* numOfRehashLevels);

/**
 * \brief This method sets the number of rehash levels of the table
 *
 * \param [in] devId
 * \param [in] prefixType
 * \param [in] numRehashLevel
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3SetNumRehashLevel(xpsDevice_t devId,
                                 xpsIpPrefixType_e prefixType, uint32_t numRehashLevel);

/**
 * \brief This method gets the number of rehash levels of the table
 *
 * \param [in] devId
 * \param [in] prefixType
 * \param [in] numRehashLevel
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3GetNumRehashLevel(xpsDevice_t devId,
                                 xpsIpPrefixType_e prefixType, uint32_t *numRehashLevel);

/**
 * \public
 * \brief Gets the size of IPv4 host table.
 *
 * This method calculates size of IPv4 host table.
 *
 * \param [in] devId
 * \param [out] tsize
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3Ipv4HostGetTableSize(xpsDevice_t devId, uint32_t *tsize);

/**
 * \public
 * \brief Gets the size of IPv6 host table.
 *
 * This method calculates size of IPv6 host table.
 *
 * \param [in] devId
 * \param [out] tsize
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3Ipv6HostGetTableSize(xpsDevice_t devId, uint32_t *tsize);

/**
 * \brief Clear Bucket State in the Ipv4 host table for given device
 *
 * \param [in] devId
 * \param [in] tblCopyIdx
 *
 * \returns XP_STATUS indicating success or failure
 */
XP_STATUS xpsL3Ipv4HostClearBucketState(xpDevice_t devId, uint8_t tblCopyIdx);

/**
 * \brief Clear Bucket State in the Ipv6 host table for given device
 *
 * \param [in] devId
 * \param [in] tblCopyIdx
 *
 * \returns XP_STATUS indicating success or failure
 */
XP_STATUS xpsL3Ipv6HostClearBucketState(xpDevice_t devId, uint8_t tblCopyIdx);

/**
 * \brief Clear Bucket State in the Ipv4 route table for given device
 *
 * \param [in] devId
 * \param [in] tblCopyIdx
 *
 * \returns XP_STATUS indicating success or failure
 */
XP_STATUS xpsL3Ipv4RouteClearBucketState(xpDevice_t devId, uint8_t tblCopyIdx);

/**
 * \brief Clear Bucket State in the Ipv6 table for given device
 *
 * \param [in] devId
 * \param [in] tblCopyIdx
 *
 * \returns XP_STATUS indicating success or failure
 */
XP_STATUS xpsL3Ipv6RouteClearBucketState(xpDevice_t devId, uint8_t tblCopyIdx);


/**
 * \brief Initing the Routing DB and Setting the Right Parameters for the device
 *
 * \param [in] devId
 * \param [in]
 *
 * \returns XP_STATUS indicating success or failure
 */
XP_STATUS xpsIpRouteInit(xpDevice_t devId);

/**
 * \brief Deiniting the Routing DB and Setting the Right Parameters for the device
 *
 * \param [in] devId
 * \param [in]
 *
 * \returns XP_STATUS indicating success or failure
 */
XP_STATUS xpsIpRouteDeinit(xpDevice_t devId);

XP_STATUS xpsL3WriteIpClassERouteEntry(xpsDevice_t devId,
                                       xpsL3RouteEntry_t *pL3RouteEntry, bool  newEntry);

XP_STATUS xpsL3AddDirectedBroadcast(xpsDevice_t devId,
                                    xpsL3HostEntry_t xpsL3NeighborEntry);

XP_STATUS xpsL3UpdateL3VlanMember(xpsDevice_t devId, xpsVlan_t vlanId,
                                  xpsInterfaceId_t intfId, bool isIpv4, bool isIpv6);

/**
 * \brief Add L3 Route Entry
 *
 * \param [in] devId Device Id of device.
 * \param [in] *pL3RouteEntry Pointer to L3 Route entry structure.
 * \param [out] *prfxBucketIdx Index at which the L3 Route entry is written to hardware.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3AddIpRouteEntry(xpsDevice_t devId,
                               xpsL3RouteEntry_t *pL3RouteEntry, uint32_t *prfxBucketIdx,
                               xpsL3RouteBulkInfo_t *routeInfo);

/**
 * \brief Remove the L3 Route Entry at the specific index
 *
 * \param [in] devId Device Id of device.
 * \param [in] *pL3RouteEntry Pointer to L3 Route entry to be removed from hardware.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3RemoveIpRouteEntry(xpsDevice_t devId,
                                  xpsL3RouteEntry_t *pL3RouteEntry, xpsL3RouteBulkInfo_t *routeInfo);
/**
 * \brief Update a L3 Route Entry
 *
 * \param [in] devId Device Id of device.
 * \param [in] *pL3RouteEntry Pointer to L3 Route entry structure.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3UpdateIpRouteEntry(xpsDevice_t devId,
                                  xpsL3RouteEntry_t *pL3RouteEntry, xpsL3RouteBulkInfo_t *routeInfo);

XP_STATUS xpsL3InitApi(xpsDevice_t devId);

XP_STATUS xpsUpdateIpCtrlTrapOnL3Entry(xpsDevice_t xpsDevId,
                                       xpsCoppCtrlPkt_e pktType,
                                       xpsPktCmd_e pktCmd,
                                       xpsInterfaceId_t l3Intf,
                                       xpsInterfaceInfo_t *l3Info,
                                       bool isIntfDel);
XP_STATUS xpsL3CreateRouteNhHwTable(xpsScope_t scopeId, uint32_t nhEcmpSize,
                                    uint32_t *nhId);
XP_STATUS xpsL3GetRouteNextHopDb(xpsDevice_t devId, uint32_t nhId,
                                 xpsL3NextHopEntry_t *pL3NextHopEntry);

XP_STATUS xpsL3IntfStatsRead(xpsDevice_t xpsDevId,
                             xpsInterfaceId_t l3IntfId,
                             xpsL3StatsInfo_t *xpsRifStatistics);
XP_STATUS xpsL3NextHopMapGet(uint32_t swNhId, uint32_t* hwNhId);

XP_STATUS xpsL3VrfCreate(xpDevice_t devId, uint32_t vrfId,
                         uint32_t *ucNhId, uint32_t *mcNhId);

XP_STATUS xpsL3VrfRemove(xpDevice_t devId, uint32_t vrfId,
                         uint32_t ucNhId, uint32_t mcNhId);
XP_STATUS xpsL3GetMaxVrf(xpDevice_t devId, uint32_t *vrfSize);
XP_STATUS xpsL3UpdateMcNHPktAction(xpDevice_t devId, uint32_t mcNhId,
                                   xpsPktCmd_e cmd);
XP_STATUS xpsL3GetRouteNextHopSwDb(xpsScope_t scopeId, uint32_t nhId,
                                   xpsL3NhSwDbEntry_t **pxpsNhSwDbInfo);
XP_STATUS xpsL3PbrEntrySet(xpDevice_t devId, uint32_t hwLeafId, uint32_t nhId,
                           bool isNhGrp);
XP_STATUS xpsL3NextHopMapRemove(uint32_t swNhId, uint32_t hwNhId);
XP_STATUS xpsL3RetrieveInterfaceMapping(xpsScope_t scopeId,
                                        xpsInterfaceId_t keyIntfId, xpsInterfaceId_t *mappedIntfId);
XP_STATUS xpsL3RouteBulkAdd(xpsL3RouteBulkInfo_t *routeInfo, uint32_t count);
XP_STATUS xpsL3RouteBulkDel(xpsL3RouteBulkInfo_t *routeInfo, uint32_t count);

#ifdef __cplusplus
}
#endif




#endif  //_xpsL3_h_
