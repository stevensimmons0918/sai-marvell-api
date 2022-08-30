// xpsSr.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsSr.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS Tunnel Manager
 */

#include "xpsInterface.h"
#include "xpsState.h"
#include "xpTypes.h"
//#include "xpTunnelMgr.h"
#include "xpsTunnel.h"

#ifndef _xpsSr_h_
#define _xpsSr_h_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief API that initializes the XPS SR Mgr
 *
 * This API will register for the state databases that are
 * needed by the SR Mgr code base for a scope.
 *
 * \param [in] scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSrInitScope(xpsScope_t scopeId);

/**
 * \brief API to De-Init the XPS SR Mgr
 *
 * This API will Deregister all state databases for SR Mgr for a scope.
 *
 * \param [in] scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSrDeInitScope(xpsScope_t scopeId);

/**
 * \brief API that initializes the XPS SR Mgr
 *
 * This API will register for the state databases that are
 * needed by the SR Mgr code base
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSrInit(void);

/**
 * \brief API to De-Init the XPS SR Mgr
 *
 * This API will Deregister all state databases for SR Mgr
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSrDeInit(void);

/**
 * \brief This method configure the device specific entries
 *
 * \param [in] devId
 *
 * \return XP_STATUS
 */

XP_STATUS xpsSrAddDevice(xpsDevice_t devId);

/**
 * \brief Creates a SR interface.
 *
 * Creates and allocates resources for a SR tunnel interface.
 * The interface id returned should be used while adding the tunnel to
 * a particular device.
 *
 * \param [in] devId Device Id. Valid value is 0-63.
 * \param [in] srhData
 * \param [out] intfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSrCreateInterface(xpsDevice_t devId, xpsInterfaceId_t *srIntfId,
                               xpsSrhData_t srhData);

/**
 * \brief Get SR data using interface.
 *
 * Returns SR data using SR interface.
 *
 * \param [in] devId Device Id. Valid value is 0-63.
 * \param [in] intfId
 * \param [out] srhdata
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSrGetData(xpsDevice_t devId, xpsInterfaceId_t intfId,
                       xpsSrhData_t *srhData);

/**
 * \brief Delete a SR interface.
 *
 * Deletes and deallocates resources for a SR tunnel interface per scope.
 *
 * \param [in] devId Device Id. Valid value is 0-63.
 * \param [out] intfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSrInterfaceDelete(xpsDevice_t devId, xpsInterfaceId_t srIntfId);

/**
 * \brief It originates the SR tunnel.
 *
 * \param [in] devId Device Id. Valid value is 0-63.
 * \param [in] intfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSrOriginationEntry(xpsDevice_t devId, xpsInterfaceId_t srIntId);

/**
 * \brief It terminates the SR tunnel.
 *
 * \param [in] devId Device Id. Valid value is 0-63.
 * \param [in] intfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSrTerminationEntry(xpsDevice_t devId, xpsInterfaceId_t srIntId);
/**
 * \brief Progrmas the next hop data.
 *
 * \param [in] devId Device Id. Valid value is 0-63.
 * \param [in] srVifId
 * \param [in] nhId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSrSetNextHopData(xpsDevice_t devId, xpsInterfaceId_t srVifId,
                              uint32_t nhId);

/**
 * \brief Programs the localSid table entry.
 *
 * \param [in] devId Device Id. Valid value is 0-63.
 * \param [in] sidEntry
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSrSetLocalSid(xpsDevice_t devId,  xpsSidEntry sidEntry);
#ifdef __cplusplus
}
#endif

#endif  //_xpsSr_h_
