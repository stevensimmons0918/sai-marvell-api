// xpsVxlan.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsVxlan.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS Vxlan Manager
 */

#include "xpsInterface.h"
#include "xpsVlan.h"

#ifndef _xpsVxlan_h_
#define _xpsVxlan_h_

#ifdef __cplusplus
extern "C" {
#endif


#define VXLAN_UDP_DST_PORT 4789

typedef enum
{
    XPS_VXLAN_UDP_PORT_HASH = 0,
    XPS_VXLAN_UDP_PORT_STATIC
} xpsVxlanUdpSrcPortMode_t;

typedef enum xpsVxlanMapType_e
{
    VXLAN_MAP_NONE,
    VXLAN_MAP_VLAN_TO_VNI,
    VXLAN_MAP_VNI_TO_VLAN,
    VXLAN_MAP_VRF_TO_VNI,
    VXLAN_MAP_VNI_TO_VRF
} xpsVxlanMapType_e;


typedef struct xpsVxlanVniMapDbEntry_t
{
    uint32_t vniId;
    xpsVxlanMapType_e mapType;
    uint16_t value; // can be vlan-id/vrf-id/BD-id
} xpsVxlanVniMapDbEntry_t;

typedef struct xpsVniMember_t
{
    xpsInterfaceId_t intfId; //XPS Port/trunk/tunnel Id.
    xpsInterfaceType_e intfType;
    uint32_t ttiRuleId;
} xpsVniMember_t;

#define XPS_MAX_VNI_MEMBERS_GROUP 16

typedef struct xpsVniDbEntry_t
{
    uint32_t vniId;
    uint32_t hwEVlanId;
    uint32_t hwEVidxId;
    uint32_t hwL2MllPairIdx;
    uint32_t refCnt;
    uint32_t numOfIntfs;
    xpsVniMember_t intfList[XPS_MAX_VNI_MEMBERS_GROUP]; //holds ports/trunk info.
    uint32_t isL3Vni;
    uint32_t vrfId;
} xpsVniDbEntry_t;



typedef struct
{
    xpsInterfaceId_t tnlIntfId;
    //    xpsInterfaceId_t underlayIntf;
    bool isLearnEnable;
    xpsRBTree_t         *vniList;
} xpsVxlanDbEntry_t;

/**
 * \struct xpsVxlanTunnelConfig_t
 *
 * This structure carries the configuration to be applied on a Vxlan tunnel.
 */
typedef struct xpsVxlanTunnelConfig_t
{
    macAddr_t dstMacAddr;
    uint16_t vlanId;
    XpIpTunnelMode_t ttlMode;
    uint8_t  ttl;
    XpIpTunnelMode_t dscpMode;
    uint8_t  dscp;
    xpsVxlanUdpSrcPortMode_t udpSrcPortMode;
    uint32_t  udpSrcPort;
    uint32_t  udpDstPort;
} xpsVxlanTunnelConfig_t;

/**
 * \brief Creates a Vxlan tunnel interface.
 *
 * Creates and allocates resources for a Vxlan tunnel interface per scope.
 * The interface id returned should be used while adding the tunnel to
 * a particular device.
 *
 * \param [in] scopeId
 * \param [out] intfId
 * \param [in] lclEpIpAddr
 * \param [in] rmtEpIpAddr
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVxlanCreateTunnelInterfaceScope(xpsScope_t scopeId,
                                             inetAddr_t *lclEpIpAddr,
                                             inetAddr_t *rmtEpIpAddr,
                                             xpsInterfaceId_t *tnlIntfId);

/**
 * \brief Adds a Vxlan tunnel to a device.
 *
 * Adds a Vxlan tunnel interface with the key parameters to a particular device.
 *
 * \param [in] devId
 * \param [in] intfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVxlanAddTunnelEntry(xpsDevice_t devId, xpsInterfaceId_t tnlIntfId,
                                 xpsInterfaceId_t underlayIntf);

/**
 * \brief Removes a Vxlan tunnel from a device.
 *
 * Removes a Vxlan tunnel from a device. This Api does NOT de-allocate resources of the tunnel.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVxlanRemoveTunnelEntry(xpsDevice_t devId,
                                    xpsInterfaceId_t tnlIntfId);

/**
 * \brief De-allocates resources for a Vxlan tunnel per scope
 *
 * De-allocates resources for a Vxlan tunnel globally
 *
 * \param [in] scopeId
 * \param [in] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVxlanDestroyTunnelInterfaceScope(xpsScope_t scopeId,
                                              xpsInterfaceId_t tnlIntfId);

/**
 * \brief Sets the Nexthop information for a Vxlan tunnel on a device.
 *
 * Sets the Nexthop information for a Vxlan tunnel on a device. The nexthop id
 * is the id returned by the L3 manager while adding the nexthop.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 * \param [in] nhId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVxlanSetTunnelNextHopData(xpsDevice_t devId,
                                       xpsInterfaceId_t tnlIntfId, uint32_t nhId);

/**
 * \brief Updates the nexthop information to the tunnel entry
 *
 * This Api updates the tunnel entry with the latest data programmed on  nexthop of the tunnel.
 * This Api needs to be called after the nexthop is updated with newer data using L3 apis.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVxlanUpdateTunnelNextHopData(xpsDevice_t devId,
                                          xpsInterfaceId_t tnlIntfId);

/**
 * \brief Sets the UDP port number for Vxlan tunnel
 *
 * \param [in] devId
 * \param [in] udpPort
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVxlanSetUdpPort(xpsDevice_t devId, uint32_t udpPort);

/**
 * \brief Gets the UDP port number for Vxlan tunnel
 *
 * \param [in] devId
 * \param [out] udpPort
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVxlanGetUdpPort(xpsDevice_t devId, uint32_t *udpPort);

/**
 * \brief Adds a Vxlan local Vtep entry to a device.
 *
 * \param [in] devId
 * \param [in] localIp
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVxlanAddLocalVtep(xpsDevice_t devId, ipv4Addr_t localIp);

/**
 * \brief Removes a Vxlan local Vtep entry to a device.
 *
 * \param [in] devId
 * \param [in] localIp
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVxlanRemoveLocalVtep(xpsDevice_t devId, ipv4Addr_t localIp);

/**
 * \brief Sets the configuration parameters for a Vxlan tunnel on a device.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 * \param [in] tunnelConfig
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVxlanSetTunnelConfig(xpsDevice_t devId, xpsInterfaceId_t tnlIntfId,
                                  xpsVxlanTunnelConfig_t *tunnelConfig);

/**
 * \brief Gets the configuration parameters for a Vxlan tunnel on a device.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 * \param [out] tunnelConfig
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVxlanGetTunnelConfig(xpsDevice_t devId, xpsInterfaceId_t tnlIntfId,
                                  xpsVxlanTunnelConfig_t *tunnelConfig);

/**
 * \brief Returns the remote ip of the Vxlan tunnel configured on a device.
 *
 * Returns the Vxlan tunnel key configured on a device.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 * \param [out] rmtEpIpAddr
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVxlanGetTunnelRemoteIp(xpsDevice_t devId,
                                    xpsInterfaceId_t tnlIntfId, ipv4Addr_t *rmtEpIpAddr);

/**
 * \brief Add a given vni and vlan into the tunnel id table
 *
 * \param [in] devId
 * \param [in] vni
 * \param [in] vlanId
 *
 * \return XP_STATUS
 */
//XP_STATUS xpsVxlanAddVni(xpsDevice_t devId, uint32_t vni, xpsVlan_t vlanId);

/**
 * \brief Remove a given vni from the tunnel id table
 *
 * \param [in] devId
 * \param [in] vni
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVxlanRemoveVni(xpsDevice_t devId, uint32_t vni);

/**
 * \brief Adds a Vxlan Multicast tunnel to a device.
 *
 * Adds a Vxlan Multicast tunnel interface with the key
 * parameters to a particular device.
 *
 * \param [in] devId: The device to be programmed.
 * \param [in] tnlIntfId: The interface ID of the tunnel.
 * \param [in] lclEpIpAddr: Local IP Address to be used for the
 *        tunnel.
 * \param [in] rmtEpIpAddr: Remote IP address to be used for the
 *        tunnel.
 * \param [in] l3IntfId: Outgoing Layer 3 interface ID.
 * \param [in] portIntfId: Outgoing Port/LAG interface ID.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVxlanAddMcTunnelEntry(xpsDevice_t devId,
                                   xpsInterfaceId_t tnlIntfId, ipv4Addr_t lclEpIpAddr, ipv4Addr_t rmtEpIpAddr,
                                   xpsInterfaceId_t l3IntfId, xpsInterfaceId_t portIntfId);

/**
 * \brief Creates a Vxlan tunnel interface.
 *
 * Creates and allocates resources for a Vxlan tunnel interface globally.
 * The interface id returned should be used while adding the tunnel to
 * a particular device.
 *
 * \param [out] intfId
 * \param [in] lclEpIpAddr
 * \param [in] rmtEpIpAddr
 *
 * \return XP_STATUS
 */

XP_STATUS xpsVxlanCreateTunnelInterface(inetAddr_t *lclEpIpAddr,
                                        inetAddr_t *rmtEpIpAddr,
                                        xpsInterfaceId_t *tnlIntfId);

/**
 * \brief De-allocates resources for a Vxlan tunnel globally
 *
 * De-allocates resources for a Vxlan tunnel globally
 *
 * \param [in] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVxlanDestroyTunnelInterface(xpsInterfaceId_t tnlIntfId);

XP_STATUS xpsVxlanInit(void);

XP_STATUS xpsVxlanDeInit(void);

XP_STATUS xpsVxlanInitScope(xpsScope_t scopeId);

XP_STATUS xpsVxlanDeInitScope(xpsScope_t scopeId);

XP_STATUS xpsVxlanAddTunnelTermination(
    xpsDevice_t devId,
    xpsInterfaceId_t tnlIntfId,
    uint32_t tnlTermId,
    inetAddr_t *srcIp,
    inetAddr_t *dstIp,
    uint32_t vniId,
    xpsVlan_t vlanId,
    XpIpTunnelMode_t dcpTtlMode,
    XpIpTunnelMode_t dcpDscpMode,
    uint16_t udpDstPort
);

XP_STATUS xpsVxlanRemoveTunnelTermination(
    xpsDevice_t devId,
    xpsInterfaceId_t tnlIntfId,
    uint32_t tnlTermId,
    bool isIpv6
);

XP_STATUS xpsVxlanAddMemberToVniDbEntry(xpsDevice_t devId, uint32_t vniId,
                                        xpsVlan_t vlanId, xpsInterfaceId_t intfId);
XP_STATUS xpsVxlanRemoveMemberToVniDbEntry(xpsDevice_t devId, uint32_t vniId,
                                           xpsVlan_t vlanId, xpsInterfaceId_t intfId);

XP_STATUS xpsVxlanVniCreateInHw(xpsDevice_t devId, uint32_t vniId,
                                uint32_t *hwEVlanId,
                                uint32_t *hwEVidxId, bool isL3Vni, uint32_t vrfId);
XP_STATUS xpsVxlanVniDeleteInHw(xpsDevice_t devId, uint32_t vniId,
                                uint32_t hwEVlanId,
                                uint32_t hwEVidxId, bool isL3Vni);

XP_STATUS xpsVxlanVniCreateDbEntry(xpsDevice_t devId, uint32_t vniId);
XP_STATUS xpsVxlanVniDeleteDbEntry(xpsDevice_t devId, uint32_t vniId);

XP_STATUS xpsVxlanAddVlanToVniDbEntry(xpsDevice_t devId, uint32_t vniId,
                                      xpsVxlanMapType_e mapType,
                                      xpsVlan_t vlanId);
XP_STATUS xpsVxlanRemoveVlanToVniDbEntry(xpsDevice_t devId, uint32_t vniId,
                                         xpsVxlanMapType_e mapType,
                                         xpsVlan_t vlanId);
XP_STATUS xpsVniGetDbEntry(xpsScope_t scopeId, uint32_t vniId,
                           xpsVniDbEntry_t **vniDbEntry);

XP_STATUS xpsVxlanTunnelDelFromVniList(xpsDevice_t devId,
                                       xpsInterfaceId_t tnlIntfId, uint32_t vniId);
XP_STATUS xpsVxlanTunnelAddToVniList(xpsDevice_t devId,
                                     xpsInterfaceId_t tnlIntfId,
                                     uint32_t vniId);
XP_STATUS xpsVxlanUpdateNNIPortOnLagRIf(xpsDevice_t devId,
                                        xpsInterfaceInfo_t *l3IntfInfo,
                                        xpsInterfaceId_t portIntfId,
                                        bool add);
XP_STATUS xpsVxlanUpdateNNIPortOnSVI(xpsDevice_t devId,
                                     xpsInterfaceInfo_t *l3IntfInfo,
                                     xpsInterfaceId_t portIntfId,
                                     bool add);

XP_STATUS xpsVxlanTunnelLearnModeSet(xpsDevice_t devId,
                                     xpsInterfaceId_t tnlIntfId, xpPktCmd_e pktCmd);

XP_STATUS xpsVxlanGetTunnelDb(xpsScope_t scopeId,
                              xpsInterfaceId_t tnlIntfId,
                              xpsVxlanDbEntry_t **info);

XP_STATUS xpsVxlanGetTunnelEPort(xpsDevice_t devId,
                                 xpsInterfaceId_t tnlIntfId,
                                 uint32_t *ePort);

XP_STATUS xpsVxlanGetTunnelHwEVlanId(xpsDevice_t devId,
                                     xpsVlan_t vlanId,
                                     xpsVlan_t *hwEVlanId);

XP_STATUS xpsVxlanGetTunnelVlanByHwEVlanId(xpsDevice_t devId,
                                           xpsVlan_t hwEVlanId,
                                           xpsVlan_t *vlanId);

XP_STATUS xpsVxlanGetTunnelIdbyEPort(xpsDevice_t devId,
                                     xpsVlan_t vlanId,
                                     uint32_t ePort,
                                     xpsInterfaceId_t *tnlIntfId);


#ifdef __cplusplus
}
#endif

#endif  //_xpsVxlan_h_


