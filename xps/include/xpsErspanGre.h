// xpsErspanGre.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsErspanGre.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS ErspanGre Manager
 */

#include "xpsInterface.h"
#include "xpsVlan.h"

#ifndef _xpsErspanGre_h_
#define _xpsErspanGre_h_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Creates a ErspanGre tunnel interface.
 *
 * Creates and allocates resources for an ErspanGre tunnel interface globally.
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
XP_STATUS xpsErspanGreCreateTunnelInterfaceScope(xpsScope_t scopeId,
                                                 inetAddr_t *lclEpIpAddr, inetAddr_t *rmtEpIpAddr, xpsInterfaceId_t *tnlIntfId);

/**
 * \brief Adds a ErspanGre tunnel to a device.
 *
 * Adds a ErspanGre tunnel interface with the key parameters to a particular device.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsErspanGreAddTunnelEntry(xpsDevice_t devId,
                                     xpsInterfaceId_t tnlIntfId);


/**
 * \brief Removes an ErspanGre tunnel from a device.
 *
 * Removes an ErspanGre tunnel from a device. This Api does NOT de-allocate resources of the tunnel.
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsErspanGreRemoveTunnelEntry(xpsDevice_t devId,
                                        xpsInterfaceId_t tnlIntfId);

/**
 * \brief De-allocates resources for a ErspanGre tunnel globally for a scope.
 *
 * De-allocates resources for a ErspanGre tunnel globally
 *
 * \param [in] scopeId
 * \param [in] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsErspanGreDestroyTunnelInterfaceScope(xpsScope_t scopeId,
                                                  xpsInterfaceId_t tnlIntfId);

/**
 * \brief Creates a ErspanGre tunnel interface.
 *
 * Creates and allocates resources for an ErspanGre tunnel interface globally.
 * The interface id returned should be used while adding the tunnel to
 * a particular device.
 *
 * \param [in] lclEpIpAddr
 * \param [in] rmtEpIpAddr
 * \param [out] intfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsErspanGreCreateTunnelInterface(inetAddr_t *lclEpIpAddr,
                                            inetAddr_t *rmtEpIpAddr, xpsInterfaceId_t *tnlIntfId);

/**
 * \brief De-allocates resources for a ErspanGre tunnel globally
 *
 * De-allocates resources for a ErspanGre tunnel globally
 *
 * \param [in] tnlIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsErspanGreDestroyTunnelInterface(xpsInterfaceId_t tnlIntfId);

/**
 * \brief Set/Update ErspanGre tunnel configuration to hw
 *
 * Set/Update ErspanGre tunnel configuration to hw
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 * \param [in] data_PTR
 *
 * \return XP_STATUS
 */
XP_STATUS xpsErspanGreTunnelCfgSet(xpsDevice_t devId,
                                   xpsInterfaceId_t tnlIntfId, xpsIpGreTunnelConfig_t * data_PTR);


#ifdef __cplusplus
}
#endif

#endif  //_xpsErspanGre_h_
