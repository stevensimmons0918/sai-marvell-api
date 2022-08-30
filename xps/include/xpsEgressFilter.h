// xpsEgressFilter.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsEgressFilter.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS Egress filter manager
 */

#ifndef _xpsEgressFilter_h_
#define _xpsEgressFilter_h_
//#include "xpEgressFltMgr.h"
//#include "xpAllocatorMgr.h"
#include "xpsEnums.h"
#include "xpsInterface.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * \brief This method initializes XPS Egress Filter with system specific
 *        references to various primitive managers and their initialization
 *        for a scope.
 *
 * \param [in] scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsEgressFilterInitScope(xpsScope_t scopeId);

/**
 * \brief This method uninitializes XPS Egress Filter for a scope.
 *
 * \param [in] scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsEgressFilterDeInitScope(xpsScope_t scopeId);

/**
 * \brief This method adds device specific primitives for this feature.
 *          This method also initializes all necessary device specific
 *          databases required maintain the relevant states.
 *
 * \param [in] devId
 * \param [in] initType
 *
 * \return XP_STATUS
 */
XP_STATUS xpsEgressFilterAddDevice(xpsDevice_t devId, xpsInitType_t initType);

/**
 * \brief This method removes all device specific primitives for this feature.
 *          This method also frees all used device specific databases by
 *          this manager.
 *
 * \param [in] devId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsEgressFilterRemoveDevice(xpsDevice_t devId);

/**
 * \brief This method allocates an egress filter and returns the id to the user
 *        for a scope.
 *
 * \param [in] scopeId
 * \param [out] egressFilterId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsEgressFilterCreateScope(xpsScope_t scopeId,
                                     xpsEgressFilter_t *egressFilterId);

/**
 * \brief This method destroys a an egress filter for a scope.
 *
 * \param [in] scopeId
 * \param [in] egressFilterId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsEgressFilterDestroyScope(xpsScope_t scopeId,
                                      xpsEgressFilter_t egressFilterId);

/**
 * \brief This method sets the bitMask corresponding to all the ports in the system
 *          present in the device to 1.
 *
 * \param [in] devId
 * \param [in] egressFilterId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsEgressFilterSetAllPort(xpsDevice_t devId,
                                    xpsEgressFilter_t egressFilterId);

/**
 * \brief This method clears the bitMask corresponding to all the ports in the system
 *          present in the device.
 *
 * \param [in] devId
 * \param [in] egressFilterId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsEgressFilterClearAllPort(xpsDevice_t devId,
                                      xpsEgressFilter_t egressFilterId);

/**
 * \brief This method sets the bit corresponding to the input port to 1 in the egressFilter mask.
 *
 * \param [in] devId
 * \param [in] egressFilterId
 * \param [in] devPort
 *
 * \return XP_STATUS
 */
XP_STATUS xpsEgressFilterSetPort(xpsDevice_t devId,
                                 xpsEgressFilter_t egressFilterId, xpsPort_t devPort);

/**
 * \brief This method clears the bit corresponding to the input port in the egressFilter mask.
 *
 * \param [in] devId
 * \param [in] egressFilterId
 * \param [in] devPort
 *
 * \return XP_STATUS
 */
XP_STATUS xpsEgressFilterClearPort(xpsDevice_t devId,
                                   xpsEgressFilter_t egressFilterId, xpsPort_t devPort);

/**
 * \brief This method initializes XPS Egress Filter with system specific
 *        references to various primitive managers and their initialization.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsEgressFilterInit(void);

/**
 * \brief This method uninitializes XPS Egress Filter.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsEgressFilterDeInit(void);

/**
 * \brief This method allocates an egress filter and returns the id to the user.
 *
 * \param [out] egressFilterId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsEgressFilterCreate(xpsEgressFilter_t *egressFilterId);

/**
 * \brief This method destroys a an egress filter.
 *
 * \param [in] egressFilterId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsEgressFilterDestroy(xpsEgressFilter_t egressFilterId);


#ifdef __cplusplus
}
#endif

#endif  //_xpsEgressFilter_h_

