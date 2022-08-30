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
 *   FILE : openXpsPacketDrv.h
 *
 *   Abstract: This file defines the apis for Packet Driver Management in OpenXPS.
 ************************************************************************/
/**
 * \file openXpsPacketDrv.h
 * \brief This file contains API prototypes and type definitions
 *        for the openXps Packet Driver Management
 * \Copyright (c) Marvell [2000-2020]
 */
#ifndef _OPENXPSPACKETDRV_H_
#define _OPENXPSPACKETDRV_H_

#include "openXpsTypes.h"
#include "openXpsEnums.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Send packet on the current interface
 *
 * In DMA mode, this function will setup the descriptor from information provided in
 * buffer and will configure the DMA and trigger it to start transfer of packets
 * In Ethernet mode, this function will send buffer over Ethernet Interface.
 * There are two tx modes: SYNC and ASYNC. In the SYNC mode the system waits till
 * tx is completed. In the ASYNC mode the system moves ahead after the completion
 * of packet transmission. The customer OS is notified about Tx completion via
 * interrupt.Only ASYNC mode is used here
 *
 * \param [in] devId device Id. Valid values are 0-63
 * \param [in] **pktInfo Packet information
 * \param [in/out] numOfPkt PktCopied
 * \param [in] outPort
 * \param [in] sendMode
 *
 * \return [XP_STATUS] On success XP_NO_ERR
 */

XP_STATUS xpsPacketDriverSend(xpsDevice_t devId,
                              const xpPacketInfo** const pktInfo, \
                              uint32_t *numOfPkt, uint32_t outPort, txMode sendMode);

/**
 * \brief Enable/Disable CPU Rx packet printing .
 *
 * \param [in] printPktCnt : Number of packets to be printed
 *
 * \return [XP_STATUS] On success XP_NO_ERR
*/
XP_STATUS xpsPacketDriverReceive(int printPktCnt);

/**
 * \brief xpsNetdevXpnetIdAllocate returns xpnetId
 *
 * \param [in] devId: device Id. Valid values are 0-63
 * \param [out] xpnetId: xpnet identificator
 *
 * \return [XP_STATUS]
 */
XP_STATUS xpsNetdevXpnetIdAllocate(xpsDevice_t devId, uint32_t *xpnetId);

/**
 * \brief xpsNetdevXpnetIdAllocateWith returns specified xpnetId
 *
 * \param [in] devId: device Id. Valid values are 0-63
 * \param [in] xpnetId: xpnet identificator
 *
 * \return [XP_STATUS]
 */
XP_STATUS xpsNetdevXpnetIdAllocateWith(xpsDevice_t devId, uint32_t xpnetId);

/**
 * \brief xpsNetdevXpnetIdFree frees xpnetId
 *
 * \param [in] devId: device Id. Valid values are 0-63
 * \param [in] xpnetId: xpnet identificator
 *
 * \return [XP_STATUS]
 */
XP_STATUS xpsNetdevXpnetIdFree(xpsDevice_t devId, uint32_t xpnetId);

/**
 * \brief xpsNetdevIfCreate creates a netdev interface
 *
 * \param [in] devId: device Id. Valid values are 0-63
 * \param [in] xpnetId: xpnet identificator
 * \param [in] ifName: interface name
 *
 * \return [XP_STATUS]
 */
XP_STATUS xpsNetdevIfCreate(xpsDevice_t devId, uint32_t xpnetId, char *ifName);

/**
 * \brief xpsNetdevIfDelete deletes a netdev interface
 *
 * \param [in] devId: device Id. Valid values are 0-63
 * \param [in] xpnetId: xpnet identificator
 *
 * \return [XP_STATUS]
 */
XP_STATUS xpsNetdevIfDelete(xpsDevice_t devId, uint32_t xpnetId);

/**
 * \brief xpsNetdevTrapSet assign/unassign trap
 *
 * \param [in] devId: device Id. Valid values are 0-63
 * \param [in] trapId: trap indentificator
 * \param [in] rCode: reason code
 * \param [in] ch: channel
 * \param [in] fd: socket descriptor
 * \param [in] flag: add/remove
 *
 * \return [XP_STATUS]
 */
XP_STATUS xpsNetdevTrapSet(xpsDevice_t devId, uint32_t trapId, uint32_t rCode,
                           uint32_t ch, int32_t fd, bool flag);

/**
 * \brief xpsNetdevTrapSet assign/unassign trap
 *
 * \param [in] devId: device Id. Valid values are 0-63
 * \param [in] trapId: trap indentificator
 * \param [out] fd: socket descriptor
 *
 * \return [XP_STATUS]
 */
XP_STATUS xpsNetdevTrapGet(xpsDevice_t devId, uint32_t trapId, int32_t* fd);

/**
 * \brief xpsNetdevDefaultTrapSet assign/unassign wildcard (default) trap
 *
 * \param [in] devId: device Id. Valid values are 0-63
 * \param [in] xpnetId: netdev indentificator
 *
 * \return [XP_STATUS]
 */
XP_STATUS xpsNetdevDefaultTrapSet(xpsDevice_t devId, uint32_t xpnetId);

/**
 * \brief xpsNetdevLinkStatusSet changes interface
 *        operational status
 *
 * \param [in] devId: device Id. Valid values are 0-63
 * \param [in] xpnetId: xpnet identificator
 * \param [in] status: Up/Down
 *
 * \return [XP_STATUS]
 */
XP_STATUS xpsNetdevLinkStatusSet(xpsDevice_t devId,
                                 uint32_t xpnetId, bool status);
XP_STATUS xpsPacketDriverUpdateMacNetDevDb(uint32_t xpnetId);

#ifdef __cplusplus
}
#endif

#endif //_OPENXPSPACKETDRV_H_
