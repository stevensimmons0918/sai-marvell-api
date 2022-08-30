// xpsInt.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
 * \file xpInt.h
 * \brief
 *
 */

#ifndef _XPINT_H_
#define _XPINT_H_

//#include "xpIntMgr.h"
#include "xpTypes.h"
#ifdef __cplusplus
extern "C" {
#endif

/**

 * \brief This method initializes xpIntMgr with system specific
 *        references to various primitive managers.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsIntInit();

XP_STATUS xpsIntInitScope(xpsScope_t scopeId);

/**

 * \brief This method de-initializes xpIntMgr with system specific
 *        references to various primitive managers.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsIntDeInit();

XP_STATUS xpsIntDeInitScope(xpsScope_t scopeId);

/**

 * \brief This method adds device specific primitives for xpIntMgr.
 *
 * \param [in] devId
 * \param [in] initType
 *
 * \return XP_STATUS
 */
XP_STATUS xpsIntAddDevice(xpDevice_t devId, xpInitType_t initType);

/**

 * \brief This method adds device specific primitives for xpIntMgr.
 *
 * \param [in] devId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsIntRemoveDevice(xpDevice_t devId);

/**

 * \brief This method make INT entries in the table
 *
 *
 * \param [in] devId
 * \param [in] portVif of INT
 *
 * \return XP_STATUS
 */
XP_STATUS xpsIntRegisterIntIntf(xpDevice_t devId, uint16_t portNum);

/**
* \public
* \brief Get TxQ Queue Congestion in Percentage
*
* \param [in] devId device id. Valid values are 0-63
* \param [in] portNum port number ranges from 0 to 71
* \param [in] queueNum queue number ranges from 0 to 15
* \param [out] queueCongestion 32-bit queue congestion on defined queueNum
*
* \return XP_STATUS
*/
XP_STATUS xpsIntGetQueueCongestion(xpDevice_t devId, uint16_t portNum,
                                   uint16_t queueNum, uint8_t *queueCongestion);

/**
* \public
* \brief This method set the node as either sink Node or normal Node
*
* \param [in] devId device id. Valid values are 0-63
* \param [in] portNum port number ranges from 0 to 71
* \param [in] sinkMode 1- To set as sink, 0- To set node as Normal node
*
* \return XP_STATUS
*/
XP_STATUS xpsIntSetAsIntCollector(xpDevice_t devId, uint16_t portNum,
                                  bool sinkMode);

/**
* \public
* \brief This method INT entries
*
* \param [in] devId device id. Valid values are 0-63
* \param [out] numOfValidEntries
* \param [in] startIndex
* \param [in] endIndex
* \param [out] logFile
* \param [in] detailFormat
* \param [in] silentMode
*
* \return XP_STATUS
*/
XP_STATUS xpsIntDisplayTable(xpDevice_t devId, uint32_t *numOfValidEntries,
                             uint32_t startIndex, uint32_t endIndex, char * logFile, uint32_t detailFormat,
                             uint32_t silentMode);

/**
* \public
* \brief This method make INT entries in the table
*
* This method inserts an entry in HW table. The index list
* of the inserted entry is returned to user.
*
* \param [in] devId device id. Valid values are 0-63
* \param [in] eVif egress vif
* \param [in] intEntryData data to be programed in hw entry
* \param [out]  index
* \param [out]  reHashIndex
*
* \return XP_STATUS
*/
XP_STATUS xpsIntAddEntry(xpDevice_t devId, uint32_t eVif,
                         xpIntEntryData_t intEntryData, uint32_t *index, uint32_t *reHashIndex);

/**
* \public
* \brief This method to lookup into INT table
*
* \param [in] devId device id. Valid values are 0-63
* \param [in] eVif EgressVif
* \param [out] intEntryData
* \param [out] index
*
* \return XP_STATUS
*/
XP_STATUS xpsIntLookup(xpDevice_t devId, uint32_t eVif,
                       xpIntEntryData_t intEntryData, int32_t *index);

/**
* \public
* \brief This method to update the 32-bit INT metadata to fetch current values of Metadat
*
* \param [in] devId device id. Valid values are 0-63
*
* \return XP_STATUS
*/
XP_STATUS xpsIntUpdateMetadata(xpDevice_t devId);

/**
* \public
* \brief This method set the value of switchId
*
* \param [in] devId device id. Valid values are 0-63
* \param [in] swithId 32-bit switchId
*
* \return XP_STATUS
*/
XP_STATUS xpsIntSetSwitchId(xpDevice_t devId, uint32_t switchId);

/**
* \public
* \brief This method get the value of switchId
*
* \param [in] devId device id. Valid values are 0-63
* \param [out] swithId 32-bit switchId
*
* \return XP_STATUS
*/
XP_STATUS xpsIntGetSwitchId(xpDevice_t devId, uint32_t *switchId);

#ifdef __cplusplus
}
#endif

#endif //_XPINT_H_
