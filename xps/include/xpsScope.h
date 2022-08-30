// xpsScope.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsScope.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS Scope Manager
 */

#ifndef _xpsScope_h_
#define _xpsScope_h_

#include "xpsEnums.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief This method creates and initializes the scope manager. This method
 *         also registers all necessary  databases required maintain the relevant
 *         states with the state manager.
 *
 * \param [in] initType
 *
 * \return XP_STATUS
 */
XP_STATUS xpsScopeInit(xpsInitType_t initType);

/**
 * \brief This method uninitializes xps scope manager. This method also
 *         deregisters databases with the state manager.
 *
 *
 * \return XP_STATUS
 */
XP_STATUS xpsScopeDeInit();

/**
 * \brief This method adds device to a particular scope. This method
 *         also initializes all necessary device specific databases
 *         required to maintain the relevant states.
 *
 * \param [in] devId
 * \param [in] scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsScopeAddDevice(xpsDevice_t devId, xpsScope_t scopeId);


/**
 * \brief This method removes a device from the current scope and adds
 *         in the default scope. This method also frees all used device
 *         specific databases used by this manager.
 *
 * \param [in] devId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsScopeRemoveDevice(xpsDevice_t devId);


/**
 * \brief This method returns the scopeId to which a device belongs.
 *
 * \param [in] devId
 *
 * \param [out] *scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsScopeGetScopeId(xpsDevice_t devId, xpsScope_t *scopeId);

/**
 * \brief This method returns the device Id with the least numerical value
 *         belonging to the given scope.
 *
 * \param [in] scopeId
 *
 * \param [out] *firstDevId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsScopeGetFirstDevice(xpsScope_t scopeId, xpsDevice_t *firstDevId);

/**
 * \brief This method returns the next deviceId in the current scope given
 *         current device Id.
 *         Note: This is not in the order in which devices were added to the
 *         scope, but this is the next devId in the numerically ascending
 *         order.
 *
 * \param [in] scopeId
 * \param [in] curDevId
 *
 * \param [out] *nextDevId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsScopeGetNextDevice(xpsScope_t scopeId, xpsDevice_t curDevId,
                                xpsDevice_t *nextDevId);

/**
 * \brief This method returns the next unused scope index.
 *
 *
 * \param [out] *freeScope
 *
 * \return XP_STATUS
 */
XP_STATUS xpsScopeGetFreeScope(xpsScope_t *freeScope);

#ifdef __cplusplus
}
#endif

#endif  //_xpsScope_h_

