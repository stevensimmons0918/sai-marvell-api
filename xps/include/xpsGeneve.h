// xpsGeneve.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsGeneve.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS Geneve Manager
 */

#include "xpsInterface.h"
#include "xpsVlan.h"

#ifndef _xpsGeneve_h_
#define _xpsGeneve_h_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \struct xpsGeneveTunnelConfig_t
 *
 * This structure carries the configuration to be applied on a Geneve tunnel.
 */
typedef struct xpsGeneveTunnelConfig_t
{
    uint8_t paclEn;
    uint32_t paclId;
} xpsGeneveTunnelConfig_t;

/**
 * \enum xpsGeneveFormatType_t
 *
 * This enum defines Geneve option formats.
 */
typedef enum xpsGeneveFormatType_t
{
    XPS_IP_GENEVE_FIXED_HDR_FORMAT = 0,
    XPS_IP_GENEVE_FLOW_ID_HDR_FORMAT,
    XPS_IP_GENEVE_FLOW_TEMPLATE_ID_HDR_FORMAT,
    XPS_IP_GENEVE_PACKET_TRAKKER_HDR_FORMAT,
    XPS_IP_GENEVE_MAX_FORMAT
} xpsGeneveFormatType_t;

/**
 * \brief Creates a Geneve tunnel interface.
 *
 * Creates and allocates resources for a Geneve tunnel interface globally.
 * The interface id returned should be used while adding the tunnel to
 * a particular device for a scope.
 *
 * \param [in] scopeId
 * \param [in] lclEpIpAddr
 * \param [in] rmtEpIpAddr
 * \param [in] optionFormat
 * \param [out] intfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsGeneveCreateTunnelInterfaceScope(xpsScope_t scopeId,
                                              ipv4Addr_t lclEpIpAddr, ipv4Addr_t rmtEpIpAddr,
                                              xpsGeneveFormatType_t optionFormat, xpsInterfaceId_t *intfId);

/**
 * \brief Adds a Geneve tunnel to a device.
 *
 * Adds a Geneve tunnel interface with the key parameters to a particular device.
 *
 * \param [in] devId
 * \param [in] intfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsGeneveAddTunnelEntry(xpsDevice_t devId, xpsInterfaceId_t intfId);

/**
 * \brief Removes a Geneve tunnel from a device.
 *
 * Removes a Geneve tunnel from a device. This Api does NOT de-allocate resources of the tunnel.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsGeneveRemoveTunnelEntry(xpsDevice_t devId,
                                     xpsInterfaceId_t tnlIntfId);

/**
 * \brief De-allocates resources for a Geneve tunnel globally for a scope.
 *
 * De-allocates resources for a Geneve tunnel globally for a scope.
 *
 * \param [in] scopeId
 * \param [in] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsGeneveDestroyTunnelInterfaceScope(xpsScope_t scopeId,
                                               xpsInterfaceId_t tnlIntfId);

/**
 * \brief Sets the Nexthop information for a Geneve tunnel on a device.
 *
 * Sets the Nexthop information for a Geneve tunnel on a device. The nexthop id
 * is the id returned by the L3 manager while adding the nexthop.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 * \param [in] nhId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsGeneveSetTunnelNextHopData(xpsDevice_t devId,
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
XP_STATUS xpsGeneveUpdateTunnelNextHopData(xpsDevice_t devId,
                                           xpsInterfaceId_t tnlIntfId);

/**
 * \brief Adds a Geneve local Endpoint entry to a device.
 *
 * \param [in] devId
 * \param [in] localIp
 *
 * \return XP_STATUS
 */
XP_STATUS xpsGeneveAddLocalEndpoint(xpsDevice_t devId, ipv4Addr_t localIp);

/**
 * \brief Removes a Geneve local Endpoint entry to a device.
 *
 * \param [in] devId
 * \param [in] localIp
 *
 * \return XP_STATUS
 */
XP_STATUS xpsGeneveRemoveLocalEndpoint(xpsDevice_t devId, ipv4Addr_t localIp);

/**
 * \brief Binds a new option to an existing Geneve tunnel and returns the new optional interface id created
 *
 * \param [in] devId
 * \param [in] baseIntfId
 * \param [in] optIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsGeneveTunnelBindOption(xpsDevice_t devId,
                                    xpsInterfaceId_t baseIntfId, xpsInterfaceId_t optIntfId);

/**
 * \brief Sets the configuration parameters for a Geneve tunnel on a device.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 * \param [in] tunnelConfig
 *
 * \return XP_STATUS
 */
XP_STATUS xpsGeneveSetTunnelConfig(xpsDevice_t devId,
                                   xpsInterfaceId_t tnlIntfId, xpsGeneveTunnelConfig_t *tunnelConfig);

/**
 * \brief Gets the configuration parameters for a Geneve tunnel on a device.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 * \param [out] tunnelConfig
 *
 * \return XP_STATUS
 */
XP_STATUS xpsGeneveGetTunnelConfig(xpsDevice_t devId,
                                   xpsInterfaceId_t tnlIntfId, xpsGeneveTunnelConfig_t *tunnelConfig);

/**
 * \brief Returns the remote ip address for a geneve tunnel interface.
 *
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 * \param [out] rmtEpIpAddr
 *
 * \return XP_STATUS
 */
XP_STATUS xpsGeneveGetTunnelRemoteIp(xpsDevice_t devId,
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
XP_STATUS xpsGeneveAddVni(xpsDevice_t devId, uint32_t vni, xpsVlan_t vlanId);

/**
 * \brief Remove a given vni from the tunnel id table
 *
 * \param [in] devId
 * \param [in] vni
 *
 * \return XP_STATUS
 */
XP_STATUS xpsGeneveRemoveVni(xpsDevice_t devId, uint32_t vni);

/**
 * \brief Adds a Geneve Multicast tunnel to a device.
 *
 * Adds a Geneve Multicast tunnel interface with the key
 * parameters to a particular device.
 *
 * \param [in] devId The device to be programmed.
 * \param [in] tnlIntfId The interface ID of the tunnel.
 * \param [in] lclEpIpAddr Local IP Address to be used for the
 *        tunnel.
 * \param [in] rmtEpIpAddr Remote IP address to be used for the
 *        tunnel.
 * \param [in] l3IntfId Outgoing Layer 3 interface ID.
 * \param [in] portIntfId Outgoing Port/LAG interface ID.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsGeneveAddMcTunnelEntry(xpsDevice_t devId,
                                    xpsInterfaceId_t tnlIntfId, ipv4Addr_t lclEpIpAddr, ipv4Addr_t rmtEpIpAddr,
                                    xpsInterfaceId_t l3IntfId, xpsInterfaceId_t portIntfId);

/**
 * \brief Creates a Geneve tunnel interface.
 *
 * Creates and allocates resources for a Geneve tunnel interface globally.
 * The interface id returned should be used while adding the tunnel to
 * a particular device.
 *
 * \param [in] lclEpIpAddr
 * \param [in] rmtEpIpAddr
 * \param [in] optionFormat
 * \param [out] intfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsGeneveCreateTunnelInterface(ipv4Addr_t lclEpIpAddr,
                                         ipv4Addr_t rmtEpIpAddr, xpsGeneveFormatType_t optionFormat,
                                         xpsInterfaceId_t *intfId);

/**
 * \brief De-allocates resources for a Geneve tunnel globally
 *
 * De-allocates resources for a Geneve tunnel globally
 *
 * \param [in] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsGeneveDestroyTunnelInterface(xpsInterfaceId_t tnlIntfId);


#ifdef __cplusplus
}
#endif

#endif  //_xpsGeneve_h_

