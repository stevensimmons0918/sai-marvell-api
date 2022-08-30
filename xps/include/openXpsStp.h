/************************************************************************
 * Copyright (c) Marvell [2000-2020].
 * All Rights Reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License"); you may
 *   not use this file except in compliance with the License. You may obtain
 *   a copy of the License at http://www.apache.org/licenses/LICENSE-2.0.
 *
 *   THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
 *   CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 *   LIMITATIONS ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
 *   FOR PARTICULAR PURPOSE, MERCHANTABILITY OR NON-INFRINGEMENT.
 *
 *   See the Apache Version 2.0 License for specific language governing
 *   permissions and limitations under the License.
 *
 *   FILE : openXpsStp.h
 *
 *   Abstract: This file defines the apis for Spanning Tree Protocol Management in OpenXPS.
 ************************************************************************/
/**
 * \file openXpsStp.h
 * \brief This file contains API prototypes and type definitions
 *        for the openXps STP Management
 * \Copyright (c) Marvell [2000-2020]
 */

#ifndef _openXpsStp_h_
#define _openXpsStp_h_

#include "openXpsTypes.h"
#include "openXpsEnums.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief This method returns the default STP ID for the device.
 *
 * \param [in] devId Device Id of device
 * \param [out] *stpId Spanning Tree Protocol ID
 *
 * \return XP_STATUS
 */
XP_STATUS xpsStpGetDefault(xpsDevice_t devId, xpsStp_t *stpId);

/**
 * \brief This method configures spanning tree and returns the id to the user.
 *
 * \param [out] *stpId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsStpCreate(xpsStp_t *stpId);

/**
 * \brief This method destroys a spanning tree.
 *
 * \param [in] stpId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsStpDestroy(xpsStp_t stpId);

/**
 * \brief This method configures the spanning tree state for a specific interface.
 *
 * \param [in] devId
 * \param [in] stpId
 * \param [in] intfId
 * \param [in] stpState
 *
 * \return XP_STATUS
 */
XP_STATUS xpsStpSetState(xpsDevice_t devId, xpsStp_t stpId,
                         xpsInterfaceId_t intfId, xpsStpState_e stpState);

/**
 * \brief This method returns the spanning tree state for an interface.
 *
 * \param [in] devId
 * \param [in] stpId
 * \param [in] intfId
 * \param [in] stpState
 *
 * \return XP_STATUS
 */
XP_STATUS xpsStpGetState(xpsDevice_t devId, xpsStp_t stpId,
                         xpsInterfaceId_t intfId, xpsStpState_e *stpState);

/**
 * \fn xpscustSaiUpdateEgressFilter
 * \public
 * \brief This method updates the Egress filter for Lag Port.
 *
 * \param [in] xpsLagId
 *
 * \return void
 */
void xpscustSaiUpdateEgressFilter(xpsInterfaceId_t xpsLagId);
#ifdef __cplusplus
}
#endif

#endif  //_openXpsStp_h_

