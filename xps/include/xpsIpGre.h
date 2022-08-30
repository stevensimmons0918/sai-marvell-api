// xpsIpGre.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
 * \file xpsIpGre.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS IpGre Manager
 */

#include "xpsInterface.h"
#include "xpsVlan.h"

#ifndef _xpsIpGre_h_
#define _xpsIpGre_h_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Creates a IpGre tunnel interface.
 *
 * Creates and allocates resources for an IpGre tunnel interface globally.
 * The interface id returned should be used while adding the tunnel to
 * a particular device for a scope.
 *
 * \param [in] scopeId
 * \param [in] lclEpIpAddr
 * \param [in] rmtEpIpAddr
 * \param [out] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsIpGreCreateTunnelInterfaceScope(xpsScope_t scopeId,
                                             ipv4Addr_t lclEpIpAddr, ipv4Addr_t rmtEpIpAddr, xpsInterfaceId_t *tnlIntfId);

/**
 * \brief Adds a IpGre tunnel to a device.
 *
 * Adds a IpGre tunnel interface with the key parameters to a particular device.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsIpGreAddTunnelEntry(xpsDevice_t devId, xpsInterfaceId_t tnlIntfId);

/**
 * \brief Removes an IpGre tunnel from a device.
 *
 * Removes an IpGre tunnel from a device. This Api does NOT de-allocate resources of the tunnel.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsIpGreRemoveTunnelEntry(xpsDevice_t devId,
                                    xpsInterfaceId_t tnlIntfId);

/**
 * \brief De-allocates resources for a IpGre tunnel globally
 *
 * De-allocates resources for a IpGre tunnel globally for a scope
 *
 * \param [in] scopeId
 * \param [in] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsIpGreDestroyTunnelInterfaceScope(xpsScope_t scopeId,
                                              xpsInterfaceId_t tnlIntfId);

/**
 * \brief Sets the Nexthop information for a IpGre tunnel on a device.
 *
 * Sets the Nexthop information for a IpGre tunnel on a device. The nexthop id
 * is the id returned by the L3 manager while adding the nexthop.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 * \param [in] nhId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsIpGreSetTunnelNextHopData(xpsDevice_t devId,
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
XP_STATUS xpsIpGreUpdateTunnelNextHopData(xpsDevice_t devId,
                                          xpsInterfaceId_t tnlIntfId);

/**
 * \brief Sets the configuration parameters for a IpGre tunnel on a device.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 * \param [in] tunnelConfig
 *
 * \return XP_STATUS
 */
XP_STATUS xpsIpGreSetTunnelConfig(xpsDevice_t devId, xpsInterfaceId_t tnlIntfId,
                                  xpsIpGreTunnelConfig_t *tunnelConfig);

/**
 * \brief Gets the configuration parameters for an IpGre tunnel on a device.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 * \param [out] tunnelConfig
 *
 * \return XP_STATUS
 */
XP_STATUS xpsIpGreGetTunnelConfig(xpsDevice_t devId, xpsInterfaceId_t tnlIntfId,
                                  xpsIpGreTunnelConfig_t *tunnelConfig);

/**
 * \brief Returns the IpGre tunnel remote ip addresses for a given tunnel interface.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 * \param [out] rmtEpIpAddr
 *
 * \return XP_STATUS
 */
XP_STATUS xpsIpGreGetTunnelRemoteIp(xpsDevice_t devId,
                                    xpsInterfaceId_t tnlIntfId, ipv4Addr_t *rmtEpIpAddr);

/**
 * \brief Creates a IpGre tunnel interface.
 *
 * Creates and allocates resources for an IpGre tunnel interface globally.
 * The interface id returned should be used while adding the tunnel to
 * a particular device.
 *
 * \param [in] lclEpIpAddr
 * \param [in] rmtEpIpAddr
 * \param [out] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsIpGreCreateTunnelInterface(ipv4Addr_t lclEpIpAddr,
                                        ipv4Addr_t rmtEpIpAddr, xpsInterfaceId_t *tnlIntfId);

/**
 * \brief De-allocates resources for a IpGre tunnel globally
 *
 * De-allocates resources for a IpGre tunnel globally
 *
 * \param [in] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsIpGreDestroyTunnelInterface(xpsInterfaceId_t tnlIntfId);

#ifdef __cplusplus
}
#endif

#endif  //_xpsIpGre_h_
