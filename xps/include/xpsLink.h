// xpsLink.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
 * \file xpsLink.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS Link Manager
 */

#ifndef _xpsLink_h_
#define _xpsLink_h_

#include "xpsInit.h"
#include "xpTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief device agnostic link manager initialization for a scope
 *
 * \param [in] scopeId
 *
 * \return [XP_STATUS] status On success XP_NO_ERR. On failure XP_ERR_MEM_ALLOC_ERROR
 */
XP_STATUS xpsLinkInitScope(xpsScope_t scopeId);

/**
 * \brief device agnostic link manager de-initialization for a scope
 *
 * \param [in] scopeId
 *
 * \return [XP_STATUS] status On success XP_NO_ERR. On failure XP_ERR_MEM_ALLOC_ERROR
 */
XP_STATUS xpsLinkDeInitScope(xpsScope_t scopeId);

/**
 * \brief Init the link manager
 *
 * \param [in] devId
 * \param [in] initType
 *
 * \return [XP_STATUS] status On success XP_NO_ERR. On failure XP_ERR_MEM_ALLOC_ERROR
*/
XP_STATUS xpsLinkAddDevice(xpsDevice_t devId, xpsInitType_t initType);

/**
 * \brief Remove device
 *
 * \return [XP_STATUS] status On success XP_NO_ERR.
 */
XP_STATUS xpsLinkRemoveDevice(xpsDevice_t devId);

/**
 * \brief device agnostic link manager initialization
 *
 * \return [XP_STATUS] status On success XP_NO_ERR. On failure XP_ERR_MEM_ALLOC_ERROR
 */
XP_STATUS xpsLinkInit(void);

/**
 * \brief device agnostic link manager de-initialization
 *
 * \return [XP_STATUS] status On success XP_NO_ERR. On failure XP_ERR_MEM_ALLOC_ERROR
 */
XP_STATUS xpsLinkDeInit(void);

/**
 * \public
 * \brief  To get live local/remote fault status
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum port number
 * \param [out] fault
 *             RX_OK: Remote/local fault is not on port
 *             RX_REMOTE: Remote fault
 *             RX_LOCAL: Local fault
 *
 * \return XP_STATUS status On success XP_NO_ERR
 */
XP_STATUS xpsLinkGetFaultLiveStatus(xpDevice_t devId, uint32_t portNum,
                                    xpMacRxFault *fault);

/**
 * \brief Performs auto-negotition/auto tune on provided port list.
 *
 * \param [in] deviceId device Id. Valid values are 0-63
 * \param [in] portsList  List of ports
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsLinkManagerPortANOrAutoTunePerform(xpDevice_t devId,
                                                xpsPortList_t *portsList);

/**
 * \public
 * \brief Enable port AutoNegotitation
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] port port Num
 * \param [in] portANAbility port AN ability
 * \param [in] portANCtrl port AN Control
 * \param [in] ieeeMode25G ieeeMode25G mode
 *
 * \return XP_STATUS status On success XP_NO_ERR
 */
XP_STATUS xpsLinkManagerPortANLtEnable(xpDevice_t devId, uint32_t port,
                                       uint32_t port_an_ability, uint16_t port_an_ctrl, uint8_t ieee_mode_25G,
                                       uint8_t keepPortDown);

/**
 * \public
 * \brief Disable port AutoNegotiation
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] port port Num
 *
 * \return XP_STATUS status On success XP_NO_ERR
 */
XP_STATUS xpsLinkManagerPortANLtDisable(xpDevice_t devId, uint32_t port);

/**
 * \public
 * \brief API to check port status and Tune accordingly. This API handle various cases like cable plug-unplug event
 *        and eyeMatric is not well.
 *
 * \param [in] devId device it. Valid values are 0-63
 * \param [in] portNum port number
 * \param [out] portTuneStatus port Tuned or not status
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsLinkManagerCheckStatusAndTunePort(xpDevice_t devId,
                                               uint32_t portNum, uint8_t *portTuneStatus);

/**
 * \public
 * \brief port auto negotiation state get
 *
 * \param [in] devId device Id
 * \param [in] portNum port number
 * \param [out] state port AN_LT State pointer
 *
 * \return XP_STATUS status On success XP_NO_ERR
 */
XP_STATUS xpsLinkManagerPortANLtStateGet(xpDevice_t devId, uint32_t portNum,
                                         xpPortANLtState *state);

/**
 * \public
 * \brief API to get remote partner AN abilities and AN error control
 *
 * \param [in] devId device Id
 * \param [in] portNum port Num
 * \param [out] portRemoteANAbility remote port speed abilities
 * \param [out] portRemoteANCtrl remote port error control (FEC) abilities
 *
 * \return XP_STATUS status On success XP_NO_ERR
 */
XP_STATUS xpsLinkManagerPortANLtRemoteAbilityGet(xpDevice_t devId,
                                                 uint32_t port,
                                                 uint16_t *portRemoteANAbility, uint16_t *portRemoteANCtrl);

/**
 * \public
 * \brief port auto negotiation Sm function
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] port port Num
 *
 * \return XP_STATUS status On success XP_NO_ERR
 */
XP_STATUS xpsLinkManagerPortANLtStateMachineFunc(xpDevice_t devId,
                                                 uint32_t port);

/**
 * \brief Enable/Disable given port auto tuning.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum  Port number. Valid values are 0-127 and 176
 * \param [in] enable   1: enable the port tuning, 0: disable the port tuning
 *
 *  \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsLinkManagerPortAutoTuneEnable(xpDevice_t devId, uint32_t portNum,
                                           uint8_t enable);

/**
 * \brief To get status of port auto tuning.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] portNum Port number. Valid values are 0-127 and 176.
 * \param [out] status Port Auto tuning status
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsLinkManagerPortAutoTuneStatusGet(xpDevice_t devId,
                                              uint32_t portNum,
                                              uint8_t *status);
/**
 * \brief To Check status of port auto tuning.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] port port Num
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsLinkManagerPortANStatusCheck(xpDevice_t devId, uint32_t port);
/**
 * \brief To Check next page status of port auto tuning.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] port port Num
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsLinkManagerPortANNextPageStatusCheck(xpDevice_t devId,
                                                  uint32_t port);
/**
 * \brief To Check port LT status.
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] port port Num
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsLinkManagerPortLtStatusCheck(xpDevice_t devId, uint32_t port);

/**
 * \brief Checking the given port is valid or not
 *
 * \param [in] devId device id. Valid values are 0-63
 * \param [in] portNum Port number. Valid values are 0 to 127 and 176.
 *
 * \return XP_STATUS On success XP_NO_ERR
 */
XP_STATUS xpsLinkManagerIsPortNumValid(xpDevice_t devId, uint32_t portNum);

#ifdef __cplusplus
}
#endif

#endif //_XPSLINK_H_
