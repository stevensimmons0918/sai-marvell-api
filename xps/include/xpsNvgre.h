// xpsNvgre.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
 * \file xpsNvgre.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS Nvgre Manager
 */

#include "xpsInterface.h"
#include "xpsVlan.h"

#ifndef _xpsNvgre_h_
#define _xpsNvgre_h_

#ifdef __cplusplus
extern "C" {
#endif



/**
 * \struct xpsNvgreTunnelConfig_t
 *
 * This structure carries the configuration to be applied on a Nvgre tunnel.
 */
typedef struct xpsNvgreTunnelConfig_t
{
    uint8_t paclEn;
    uint32_t paclId;
} xpsNvgreTunnelConfig_t;

/**
 * \brief Creates a Nvgre tunnel interface.
 *
 * Creates and allocates resources for a Nvgre tunnel interface in a scope.
 * The interface id returned should be used while adding the tunnel to
 * a particular device.
 *
 * \param [in] scopeId
 * \param [in] lclEpIpAddr
 * \param [in] rmtEpIpAddr
 * \param [out] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsNvgreCreateTunnelInterfaceScope(xpsScope_t scopeId,
                                             ipv4Addr_t lclEpIpAddr, ipv4Addr_t rmtEpIpAddr, xpsInterfaceId_t *tnlIntfId);

/**
 * \brief Adds a Nvgre tunnel to a device.
 *
 * Adds a Nvgre tunnel interface with the key parameters to a particular device.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsNvgreAddTunnelEntry(xpsDevice_t devId, xpsInterfaceId_t tnlIntfId);

/**
 * \brief Removes a Nvgre tunnel from a device.
 *
 * Removes a Nvgre tunnel from a device. This Api does NOT de-allocate resources of the tunnel.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsNvgreRemoveTunnelEntry(xpsDevice_t devId,
                                    xpsInterfaceId_t tnlIntfId);

/**
 * \brief De-allocates resources for a Nvgre tunnel in a scope
 *
 * De-allocates resources for a Nvgre tunnel in a scope
 *
 * \param [in] scopeId
 * \param [in] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsNvgreDestroyTunnelInterfaceScope(xpsScope_t scopeId,
                                              xpsInterfaceId_t tnlIntfId);

/**
 * \brief Sets the Nexthop information for a Nvgre tunnel on a device.
 *
 * Sets the Nexthop information for a Nvgre tunnel on a device. The nexthop id
 * is the id returned by the L3 manager while adding the nexthop.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 * \param [in] nhId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsNvgreSetTunnelNextHopData(xpsDevice_t devId,
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
XP_STATUS xpsNvgreUpdateTunnelNextHopData(xpsDevice_t devId,
                                          xpsInterfaceId_t tnlIntfId);

/**
 * \brief Adds a Nvgre local nve entry to a device.
 *
 * \param [in] devId
 * \param [in] localIp
 *
 * \return XP_STATUS
 */
XP_STATUS xpsNvgreAddLocalNve(xpsDevice_t devId, ipv4Addr_t localIp);

/**
 * \brief Removes a Nvgre local nve entry to a device.
 *
 * \param [in] devId
 * \param [in] localIp
 *
 * \return XP_STATUS
 */
XP_STATUS xpsNvgreRemoveLocalNve(xpsDevice_t devId, ipv4Addr_t localIp);

/**
 * \brief Sets the configuration parameters for a Nvgre tunnel on a device.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 * \param [in] tunnelConfig
 *
 * \return XP_STATUS
 */
XP_STATUS xpsNvgreSetTunnelConfig(xpsDevice_t devId, xpsInterfaceId_t tnlIntfId,
                                  xpsNvgreTunnelConfig_t *tunnelConfig);

/**
 * \brief Gets the configuration parameters for a Nvgre tunnel on a device.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 * \param [out] tunnelConfig
 *
 * \return XP_STATUS
 */
XP_STATUS xpsNvgreGetTunnelConfig(xpsDevice_t devId, xpsInterfaceId_t tnlIntfId,
                                  xpsNvgreTunnelConfig_t *tunnelConfig);

/**
 * \brief Returns the Nvgre tunnel remote Ip for a given tunnel interface.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 * \param [out] rmtEpIpAddr
 *
 * \return XP_STATUS
 */
XP_STATUS xpsNvgreGetTunnelRemoteIp(xpsDevice_t devId,
                                    xpsInterfaceId_t tnlIntfId, ipv4Addr_t *rmtEpIpAddr);

/**
 * \brief Add a given tni and vlan into the tunnel id table
 *
 * \param [in] devId
 * \param [in] tni
 * \param [in] vlanId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsNvgreAddTni(xpsDevice_t devId, uint32_t tni, xpsVlan_t vlanId);

/**
 * \brief Remove a given tni from the tunnel id table
 *
 * \param [in] devId
 * \param [in] tni
 *
 * \return XP_STATUS
 */
XP_STATUS xpsNvgreRemoveTni(xpsDevice_t devId, uint32_t tni);

/**
 * \brief Adds a NVGRE Multicast tunnel to a device.
 *
 * Adds a NVGRE Multicast tunnel interface with the key
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
XP_STATUS xpsNvgreAddMcTunnelEntry(xpsDevice_t devId,
                                   xpsInterfaceId_t tnlIntfId, ipv4Addr_t lclEpIpAddr, ipv4Addr_t rmtEpIpAddr,
                                   xpsInterfaceId_t l3IntfId, xpsInterfaceId_t portIntfId);

/**
 * \brief Creates a Nvgre tunnel interface.
 *
 * Creates and allocates resources for a Nvgre tunnel interface globally.
 * The interface id returned should be used while adding the tunnel to
 * a particular device.
 *
 * \param [in] lclEpIpAddr
 * \param [in] rmtEpIpAddr
 * \param [out] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsNvgreCreateTunnelInterface(ipv4Addr_t lclEpIpAddr,
                                        ipv4Addr_t rmtEpIpAddr, xpsInterfaceId_t *tnlIntfId);

/**
 * \brief De-allocates resources for a Nvgre tunnel globally
 *
 * De-allocates resources for a Nvgre tunnel  globally
 *
 * \param [in] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsNvgreDestroyTunnelInterface(xpsInterfaceId_t tnlIntfId);

#ifdef __cplusplus
}
#endif

#endif  //_xpsNvgre_h_


