// xpsPbb.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsPbb.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS Pbb Manager
 */

#include "xpsInterface.h"
#include "xpsVlan.h"

#ifndef _xpsPbb_h_
#define _xpsPbb_h_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \struct xpsPbbTunnelConfig_t
 *
 * This structure carries the configuration to be applied on a Pbb tunnel.
 */
typedef struct xpsPbbTunnelConfig_t
{
    uint8_t paclEn;
    uint32_t paclId;
} xpsPbbTunnelConfig_t;

/**
 * \brief Creates a PBB tunnel interface.
 *
 * Creates and allocates resources for a PBB tunnel interface globally.
 * The interface id returned should be used while adding the tunnel to
 * a particular device.
 *
 * \param [out] pbbTnlId
 * \param [in] bSa
 * \param [in] bDa
 * \param [in] bTag
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPbbCreateTunnelInterface(macAddr_t bSa, macAddr_t bDa,
                                      uint32_t bTag, xpsInterfaceId_t *pbbTnlId);

/**
 * \brief Creates a Pbb tunnel interface.
 *
 * Creates and allocates resources for a Pbb tunnel interface per scope.
 * The interface id returned should be used while adding the tunnel to
 * a particular device.
 *
 * \param [in] scopeId
 * \param [in] bSa
 * \param [in] bDa
 * \param [in] bTag
 * \param [out] pbbTnlId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPbbCreateTunnelInterfaceScope(xpsScope_t scopeId, macAddr_t bSa,
                                           macAddr_t bDa, uint32_t bTag, xpsInterfaceId_t *pbbTnlId);

/**
 * \brief Adds a Pbb tunnel to a device.
 *
 * Adds a Pbb tunnel interface with the key parameters to a particular device.
 *
 * \param [in] devId
 * \param [in] intfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPbbAddTunnelEntry(xpsDevice_t devId, xpsInterfaceId_t intfId);

/**
 * \brief Bind a port to a Pbb tunnel.
 *
 * Adds an outgoing port for  Pbb tunnel interface.
 *
 * \param [in] devId
 * \param [in] pbbTnlId
 * \param [in] port
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPbbBindPortToTunnel(xpsDevice_t devId, xpsInterfaceId_t pbbTnlId,
                                 xpsPort_t port);

/**
 * \brief Removes a Pbb tunnel from a device.
 *
 * Removes a Pbb tunnel from a device. This Api does NOT de-allocate resources of the tunnel.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPbbRemoveTunnelEntry(xpsDevice_t devId,
                                  xpsInterfaceId_t tnlIntfId);

/**
 * \brief De-allocates resources for a Pbb tunnel per scope
 *
 * De-allocates resources for a Pbb tunnel globally
 *
 * \param [in] scopeId
 * \param [in] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPbbDestroyTunnelInterfaceScope(xpsScope_t scopeId,
                                            xpsInterfaceId_t tnlIntfId);

/**
 * \brief Add a given isid and vlan into the tunnel id table
 *
 * \param [in] devId
 * \param [in] isid
 * \param [in] vlanId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPbbAddIsid(xpsDevice_t devId, uint32_t isid, xpsVlan_t vlanId);

/**
 * \brief Remove a given isid from the tunnel id table
 *
 * \param [in] devId
 * \param [in] isid
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPbbRemoveIsid(xpsDevice_t devId, uint32_t isid);

/**
 * \brief De-allocates resources for a Pbb tunnel globally
 *
 * De-allocates resources for a Pbb tunnel globally
 *
 * \param [in] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPbbDestroyTunnelInterface(xpsInterfaceId_t tnlIntfId);

#ifdef __cplusplus
}
#endif

#endif  //_xpsPbb_h_
