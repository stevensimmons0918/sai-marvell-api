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
 *   FILE : openXpsFdb.h
 *
 *   Abstract: This file defines the apis for FDB Management in OpenXPS.
 ************************************************************************/
/**
 * \file openXpsFdb.h
 * \brief This file contains API prototypes and type definitions
 *        for the openXps FDB Management
 * \Copyright (c) Marvell [2000-2020]
 */

#ifndef _openXpsFdb_h_
#define _openXpsFdb_h_

#include "openXpsTypes.h"
#include "openXpsEnums.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * \struct xpsFdbEntry_t
 * \brief This structure defines the FDB entry format.
 */
typedef struct xpsFdbEntry_t
{
    xpVlan_t        vlanId;        ///< VLAN-ID
    macAddr_t       macAddr;       ///< MAC
    xpPktCmd_e      pktCmd;        ///< Packet Command
    uint8_t         isControl;     ///< Control MAC
    uint8_t         isRouter;      ///< Router MAC
    uint8_t         isStatic;      ///< Static MAC
    xpsInterfaceId_t  intfId;      ///< Interface ID
    uint32_t
    serviceInstId; ///< A Service Instance ID to represent VNI/TNI/ISID for tunnelled packets
} xpsFdbEntry_t;

/**
 * \brief Functional Pointer for FDB MAC Learning Handler registration
 *
 * \param [in] devId
 * \param [in] ingressVif
 * \param [in] reasonCode
 * \param [in] bdId
 * \param [in] buf
 * \param [in] bufSize
 * \param [in] userData
 *
 */
typedef XP_STATUS(*xpsFdbLearnHandler)(xpsDevice_t, uint32_t, uint32_t,
                                       uint32_t, void*, uint16_t, void*);

/**
 * \brief Function pointer for FDB Aging registration
 */
typedef void (*xpFdbAgingHandler)(xpsDevice_t, xpsFdbEntry_t fdbEntry);

/**
 * \brief API to control FDB MAC Aging period
 *
 * \param [in] devId Device Id of device.
 * \param [in] agingExpo Actual aging timeout is 2 ^ agingExpo * Unit Time
 *
 * \return XP_STATUS
 *
 */
XP_STATUS xpsFdbSetAgingTime(xpsDevice_t devId, uint32_t agingExpo);


/**
 * \brief Remove an entry from the FDB table at a specific index
 *
 * \param [in] devId Device Id of device.
 * \param [in] index Index of the FDB entry to be removed from hardware.
 *
 * \return XP_STATUS
 *
 */
XP_STATUS xpsFdbRemoveEntryByIndex(xpsDevice_t devId, uint32_t index);

/**
 * \brief This method is used for classifying L2 control packets
 *        and assigning a unique reason code for control plane
 *        policing (CoPP). It internally adds the MAC entry to
 *        the control MAC table with maskBd set to 1 and add an
 *        entry to the FDB table with special Bd (0). XPC code
 *        uses maskBd for looking up the FDB table with the
 *        special Bd. Reason Code is overloaded on the Vif in
 *        the fdb entry.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] macAddr
 * \param [in] reasonCode
 * \param [out] indexList
 *
 * \return XP_STATUS
 */
XP_STATUS xpsFdbAddControlMacEntry(xpsDevice_t devId, uint32_t vlanId,
                                   macAddr_t macAddr, uint32_t reasonCode, xpsHashIndexList_t *indexList);

/**
 * \brief The method allows to register the FDB learn handler
 *
 * \param [in] devId Device Id of device.
 * \param [in] fdbLearnCb func where fdbLearnHandler is of type XP_STATUS (*xpsFdbLearnHandler)(xpsDevice_t,
 *             uint32_t ingressVif, uint32_t reasonCode, uint32_t bdId, void* buf, uint16_t bufSize , void*)
 * \param [in] *userData User Provided Data
 *
 * \return XP_STATUS
 */
XP_STATUS xpsFdbRegisterLearn(xpsDevice_t devId, xpsFdbLearnHandler fdbLearnCb,
                              void *userData);

/**
 * \brief The method allows to register the FDB aging handler
 *
 * \param [in] devId Device Id of device.
 * \param [in] fdbAgingHandler  User Provided Call back where fdbAgingHandler is of type void (*aging_handler)(xpsDevice_t , uint32_t *, void *)
 * \param [in] userData User Provided Data
 *
 * \return XP_STATUS
 *
*/
extern XP_STATUS(*xpsFdbRegisterAgingHandler)(xpsDevice_t devId,
                                              xpFdbAgingHandler ageHandler);


/**
 * \brief The method allows to unregister the FDB aging handler
 *
 * \param [in] devId Device Id of device.
 *
 * \return XP_STATUS
 *
 */
XP_STATUS xpsFdbUnregisterAgingHandler(xpsDevice_t devId);

/**
 * \brief Enable Aging on the FDB table
 *
 * \param [in] devId Device Id of device.
 * \param [in] enable Enable/disable aging for the FDB table.
 *
 * \return XP_STATUS
 *
 */
XP_STATUS xpsFdbConfigureTableAging(xpsDevice_t devId, uint32_t enable);

/**
 * \brief  Display valid table entries in FDB table in the given range
 *
 * \param [in]  devId
 * \param [out] *numOfValidEntries
 * \param [in]  startIndex
 * \param [in]  endIndex
 * \param [in] *logFile
 * \param [in] detailFormat
 * \param [in] silentMode
 *
 * \return XP_STATUS
 */
XP_STATUS xpsFdbDisplayTable(xpsDevice_t devId, uint32_t *numOfValidEntries,
                             uint32_t startIndex, uint32_t endIndex, char * logFile, uint32_t detailFormat,
                             uint32_t silentMode);

/**
 * \brief This method sets rehash level for FDB table
 *
 * \param [in] devId
 * \param [in] numOfRehashLevels
 *
 * \return XP_STATUS
 */
XP_STATUS xpsFdbSetRehashLevel(xpDevice_t devId, uint8_t numOfRehashLevels);

#ifdef __cplusplus
}
#endif

#endif  //_openXpsFdb_h_
