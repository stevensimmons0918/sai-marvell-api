// xpsSflow.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsSflow.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS Sflow Manager
 */

#ifndef _xpsSflow_h_
#define _xpsSflow_h_

//#include "xpSflowMgr.h"
#include "xpsInit.h"
#include "xpsInterface.h"
#include "xpsPort.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief This method initializes Sflow Mgr with system specific
 *        references to various primitive managers and their initialization in a scope.
 *
 * \param [in] scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSflowInitScope(xpsScope_t scopeId);

/**
 * \brief This method cleans up from the Sflow Mgr system specific
 *        references to various primitive managers in a scope.
 *
 * \param [in] scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSflowDeInitScope(xpsScope_t scopeId);

/**
 * \brief This method configures and add device specific primitives
 *        required for the Sflow FL manager.
 *
 * \param [in] devId
 * \param [in] initType
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSflowAddDevice(xpsDevice_t devId, xpsInitType_t initType);

/**
 * \brief This method removes device specific primitives
 *        required for the Sflow FL manager.
 *
 * \param [in] devId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSflowRemoveDevice(xpsDevice_t devId);

/**
 * \public
 * \brief Updates sampling fields in the entry.
 *
 * This sets the sampling parameters for a particular port in
 * the device
 *
 * \param [in] scopeId
 * \param [in] portIntfId
 * \param [in] client
 * \param [in] type
 * \param [in] nSample
 * \param [in] mBase
 * \param [in] mExpo
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSflowSetPortSamplingConfigScope(xpsScope_t scopeId,
                                             xpsInterfaceId_t portIntfId, xpAcmClient_e client,
                                             uint32_t nSample, uint32_t mBase, uint32_t mExpo);

/**
 * \public
 * \brief Retrieves fields in the entry.
 *
 * This gets the sampling parameters for a specific port id in
 * the device
 *
 * \param [in] scopeId
 * \param [in] portIntfId
 * \param [in] client
 * \param [in] type
 * \param [out] *nSample
 * \param [out] *mBase
 * \param [out] *mExpo
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSflowGetPortSamplingConfigScope(xpsScope_t scopeId,
                                             xpsInterfaceId_t portIntfId, xpAcmClient_e client,
                                             uint32_t *nSample, uint32_t *mBase, uint32_t *mExpo);

/**
 * \public
 * \brief Enables port sampling on a particular port.
 *
 * \param [in] scopeId
 * \param [in] portIntfId
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSflowEnablePortSamplingScope(xpsScope_t scopeId,
                                          xpsInterfaceId_t portIntfId, uint16_t enable);

/**
 * \public
 * \brief Sets sflow pkt cmd for all sampling flows.
 *
 * \param [in] devId
 * \param [in] pktCmd
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSflowSetPktCmd(xpsDevice_t devId, xpPktCmd_e pktCmd);

/**
* \public
* \brief Retrieves sflow pkt cmd.
*
* \param [in] devId
* \param [out] *pktCmd
*
* \return XP_STATUS
*/
XP_STATUS xpsSflowGetPktCmd(xpsDevice_t devId, xpPktCmd_e *pktCmd);

/**
 * \public
 * \brief Sets sflow interface id for all sampling flows.
 *
 * \param [in] devId
 * \param [in] intfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSflowSetIntfId(xpsDevice_t devId, xpsInterfaceId_t intfId);

/**
 * \public
 * \brief Retrieves sflow interface id.
 *
 * \param [in] devId
 * \param [out] *intfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSflowGetIntfId(xpsDevice_t devId, xpsInterfaceId_t *intfId);

/**
 * \public
 * \brief Creates  sflow in a scope
 *
 * \param [in] scopeId
 * \param [out] *xpsSflowId
 *
 * \return XP_STATUS
 */

XP_STATUS xpsSflowCreateScope(xpsScope_t scopeId, uint32_t* xpsSflowId);

/**
 * \public
 * \brief Destroys  sflow in a scope
 *
 * \param [in] scopeId
 * \param [in] xpsSflowId
 *
 * \return XP_STATUS
 */

XP_STATUS xpsSflowDestroyScope(xpsScope_t scopeId, uint32_t xpsSflowId);

/**
 *
 * \public
 * \brief Enables/disables  Sflow globally on a device.
 *
 * \param [in] devId
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSflowSetEnable(xpsDevice_t devId, uint32_t enable);

/**
 * \brief This method initializes Sflow Mgr with system specific
 *        references to various primitive managers and their initialization.
 *
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSflowInit(void);

/**
 * \brief This method cleans up from the Sflow Mgr system specific
 *        references to various primitive managers.
 *
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSflowDeInit(void);

/**
 * \public
 * \brief Updates sampling fields in the entry.
 *
 * This sets the sampling parameters for a particular port in
 * the device
 *
 * \param [in] portIntfId
 * \param [in] client
 * \param [in] type
 * \param [in] nSample
 * \param [in] mBase
 * \param [in] mExpo
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSflowSetPortSamplingConfig(xpsInterfaceId_t portIntfId,
                                        xpAcmClient_e client,
                                        uint32_t nSample, uint32_t mBase, uint32_t mExpo);

/**
 * \public
 * \brief Retrieves fields in the entry.
 *
 * This gets the sampling parameters for a specific port id in
 * the device
 *
 * \param [in] portIntfId
 * \param [in] client
 * \param [in] type
 * \param [out] *nSample
 * \param [out] *mBase
 * \param [out] *mExpo
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSflowGetPortSamplingConfig(xpsInterfaceId_t portIntfId,
                                        xpAcmClient_e client,
                                        uint32_t *nSample, uint32_t *mBase, uint32_t *mExpo);

/**
 * \public
 * \brief Enables port sampling on a particular port.
 *
 * \param [in] portIntfId
 * \param [in] enable
 *
 * \return XP_STATUS
 */

XP_STATUS xpsSflowEnablePortSampling(xpsInterfaceId_t portIntfId,
                                     uint16_t enable);

/**
 * \public
 * \brief Creates  sflow
 *
 * \param [out] *xpsSflowId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSflowCreate(uint32_t* xpsSflowId);

/**
 * \public
 * \brief Destroys  sflow
 *
 * \param [in] xpsSflowId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSflowDestroy(uint32_t xpsSflowId);

#ifdef __cplusplus
}
#endif

#endif  //_xpsSflow_h_
