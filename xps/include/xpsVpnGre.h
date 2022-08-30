// xpsVpnGre.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsVpnGre.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS VpnGre Manager
 */

#include "xpsInterface.h"
#include "xpsVlan.h"

#ifndef _xpsVpnGre_h_
#define _xpsVpnGre_h_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \struct xpsVpnGreTunnelConfig_t
 *
 * This structure carries the configuration to be applied on a VpnGre tunnel.
 */
typedef struct xpsVpnGreTunnelConfig_t
{
    uint8_t baclEn;
    uint32_t baclId;
    uint8_t raclEn;
    uint32_t raclId;
} xpsVpnGreTunnelConfig_t;

/**
 * \brief Creates a Loose mode VpnGre tunnel interface.
 *
 * Creates and allocates resources for an Loose Mode VpnGre tunnel interface globally.
 * The interface id returned should be used while adding the tunnel to
 * a particular device.
 *
 * \param [in] lclEpIpAddr
 * \param [in] rmtEpIpAddr
 * \param [out] intfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVpnGreCreateLooseModeIpTunnelInterface(ipv4Addr_t lclEpIpAddr,
                                                    ipv4Addr_t rmtEpIpAddr, xpsInterfaceId_t *tnlIntfId);

/**
 * \brief Creates a Loose mode VpnGre tunnel interface for a scope.
 *
 * Creates and allocates resources for an Loose Mode VpnGre tunnel interface globally.
 * The interface id returned should be used while adding the tunnel to
 * a particular device.
 *
 * \param [in] scopeId
 * \param [in] lclEpIpAddr
 * \param [in] rmtEpIpAddr
 * \param [out] intfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVpnGreCreateLooseModeIpTunnelInterfaceScope(xpsScope_t scopeId,
                                                         ipv4Addr_t lclEpIpAddr, ipv4Addr_t rmtEpIpAddr, xpsInterfaceId_t *tnlIntfId);

/**
 * \brief Creates a Strict mode VpnGre tunnel interface.
 *
 * Creates and allocates resources for an Strict Mode VpnGre tunnel interface globally.
 * The interface id returned should be used while adding the tunnel to
 * a particular device.
 *
 * \param [in] lclEpIpAddr
 * \param [in] rmtEpIpAddr
 * \param [out] intfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVpnGreCreateStrictModeTunnelInterface(ipv4Addr_t lclEpIpAddr,
                                                   ipv4Addr_t rmtEpIpAddr, uint32_t vpnLabel, xpsInterfaceId_t *tnlIntfId);

/**
 * \brief Creates a Strict mode VpnGre tunnel interface for a scope.
 *
 * Creates and allocates resources for an Strict Mode VpnGre tunnel interface globally.
 * The interface id returned should be used while adding the tunnel to
 * a particular device.
 *
 * \param [in] scopeId
 * \param [in] lclEpIpAddr
 * \param [in] rmtEpIpAddr
 * \param [out] intfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVpnGreCreateStrictModeTunnelInterfaceScope(xpsScope_t scopeId,
                                                        ipv4Addr_t lclEpIpAddr, ipv4Addr_t rmtEpIpAddr, uint32_t vpnLabel,
                                                        xpsInterfaceId_t *tnlIntfId);

/**
 * \brief Adds a VpnGre tunnel to a device.
 *
 * Adds a VpnGre tunnel interface with the key parameters to a particular device.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVpnGreAddTunnelEntry(xpsDevice_t devId,
                                  xpsInterfaceId_t tnlIntfId);

/**
 * \brief Removes an VpnGre tunnel from a device.
 *
 * Removes an VpnGre tunnel from a device. This Api does NOT de-allocate resources of the tunnel.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVpnGreRemoveTunnelEntry(xpsDevice_t devId,
                                     xpsInterfaceId_t tnlIntfId);

/**
 * \brief De-allocates resources for a VpnGre tunnel globally
 *
 * De-allocates resources for a VpnGre tunnel globally
 *
 * \param [in] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVpnGreDestroyTunnelInterface(xpsInterfaceId_t tnlIntfId);

/**
 * \brief De-allocates resources for a VpnGre tunnel globally for a scope
 *
 * De-allocates resources for a VpnGre tunnel globally
 *
 * \param [in] scopeId
 * \param [in] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVpnGreDestroyTunnelInterfaceScope(xpsScope_t scopeId,
                                               xpsInterfaceId_t tnlIntfId);

/**
 * \brief Sets the Nexthop information for a VpnGre tunnel on a device.
 *
 * Sets the Nexthop information for a VpnGre tunnel on a device. The nexthop id
 * is the id returned by the L3 manager while adding the nexthop.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 * \param [in] nhId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVpnGreSetTunnelNextHopData(xpsDevice_t devId,
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
XP_STATUS xpsVpnGreUpdateTunnelNextHopData(xpsDevice_t devId,
                                           xpsInterfaceId_t tnlIntfId);

/**
 * \brief Sets the configuration parameters for a VpnGre tunnel on a device.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 * \param [in] tunnelConfig
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVpnGreSetTunnelConfig(xpsDevice_t devId,
                                   xpsInterfaceId_t tnlIntfId, xpsVpnGreTunnelConfig_t *tunnelConfig);

/**
 * \brief Gets the configuration parameters for an VpnGre tunnel on a device.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 * \param [out] tunnelConfig
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVpnGreGetTunnelConfig(xpsDevice_t devId,
                                   xpsInterfaceId_t tnlIntfId, xpsVpnGreTunnelConfig_t *tunnelConfig);

/**
 * \brief Returns the VpnGre tunnel remote ip addresses for a given tunnel interface.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 * \param [out] rmtEpIpAddr
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVpnGreGetTunnelRemoteIp(xpsDevice_t devId,
                                     xpsInterfaceId_t tnlIntfId, ipv4Addr_t *rmtEpIpAddr);

#ifdef __cplusplus
}
#endif

#endif  //_xpsVpnGre_h_
