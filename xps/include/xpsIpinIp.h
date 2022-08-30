// xpsIpinIp.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
 * \file xpsIpinIp.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS IpinIp Manager
 */

#ifndef _xpsIpinIp_h_
#define _xpsIpinIp_h_

#include "xpsInterface.h"
#include "xpsTunnel.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Creates a IpinIp  tunnel interface.
 *
 * Creates and allocates resources for an IpinIp tunnel interface globally.
 * The interface id returned should be used while adding the tunnel to
 * a particular device for a scope.
 *
 * \param [in] scopeId
 * \param [in] xpsIpTunnelData_t
 * \param [out] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsIpinIpCreateTunnelInterfaceScope(xpsScope_t scopeId,
                                              xpsIpTunnelData_t *ipTnlData, xpsInterfaceId_t *tnlIntfId);

/**
 * \brief Adds an IpinIp tunnel to a device.
 *
 * Adds an IpInIp tunnel interface with the key parameters to a particular device.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsIpinIpAddTunnelEntry(xpsDevice_t devId,
                                  xpsInterfaceId_t tnlIntfId);

/**
 * \brief Removes an IpinIp tunnel from a device.
 *
 * Removes an IpinIp tunnel from a device. This Api does NOT de-allocate resources of the tunnel.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsIpinIpRemoveTunnelEntry(xpsDevice_t devId,
                                     xpsInterfaceId_t tnlIntfId);

/**
 * \brief De-allocates resources for a IpinIp tunnel globally
 *
 * De-allocates resources for a IpinIp tunnel globally for a scope
 *
 * \param [in] scopeId
 * \param [in] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsIpinIpDestroyTunnelInterfaceScope(xpsScope_t scopeId,
                                               xpsInterfaceId_t tnlIntfId);

/**
 * \brief Sets the Nexthop information for a IpinIp tunnel on a device.
 *
 * Sets the Nexthop information for a IpinIp tunnel on a device. The nexthop id
 * is the id returned by the L3 manager while adding the nexthop.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 * \param [in] nhId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsIpinIpSetTunnelNextHopData(xpsDevice_t devId,
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
XP_STATUS xpsIpinIpUpdateTunnelNextHopData(xpsDevice_t devId,
                                           xpsInterfaceId_t tnlIntfId);

/**
 * \brief Sets the configuration parameters for a IpinIp tunnel on a device.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 * \param [in] tunnelConfig
 *
 * \return XP_STATUS
 */
XP_STATUS xpsIpinIpSetTunnelConfig(xpsDevice_t devId,
                                   xpsInterfaceId_t tnlIntfId, xpsIpinIpTunnelConfig_t *tunnelConfig);

/**
 * \brief Gets the configuration parameters for an IpinIp tunnel on a device.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 * \param [out] tunnelConfig
 *
 * \return XP_STATUS
 */
XP_STATUS xpsIpinIpGetTunnelConfig(xpsDevice_t devId,
                                   xpsInterfaceId_t tnlIntfId, xpsIpinIpTunnelConfig_t *tunnelConfig);

/**
 * \brief Returns the IpinIp remote ip address for a given tunnel interface.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 * \param [out] rmtEpIpAddr
 *
 * \return XP_STATUS
 */
XP_STATUS xpsIpinIpGetTunnelRemoteIp(xpsDevice_t devId,
                                     xpsInterfaceId_t tnlIntfId, ipv4Addr_t *rmtEpIpAddr);

/**
 * \brief Creates a IpinIp  tunnel interface.
 *
 * Creates and allocates resources for an IpinIp tunnel interface globally.
 * The interface id returned should be used while adding the tunnel to
 * a particular device.
 *
 * \param [in] xpsIpTunnelData_t
 * \param [out] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsIpinIpCreateTunnelInterface(xpsIpTunnelData_t *ipTnlData,
                                         xpsInterfaceId_t *tnlIntfId);

/**
 * \brief De-allocates resources for a IpinIp tunnel globally
 *
 * De-allocates resources for a IpinIp tunnel globally
 *
 * \param [in] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsIpinIpDestroyTunnelInterface(xpsInterfaceId_t tnlIntfId);

/**
 * \breif Initialize Global configurations for IPV4_V6 Tunnel.
 * \param [in] ScopeId
 * \ return XP_STATUS
 */
XP_STATUS xpsIpinIpInit(xpsScope_t scopeId);

#ifdef __cplusplus
}
#endif

#endif  //_xpsIpinIp_h_


