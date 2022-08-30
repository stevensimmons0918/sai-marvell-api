// xpsFdbMgr.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsFdbMgr.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS FDB Manager
 */

#ifndef _xpsFdbMgr_h_
#define _xpsFdbMgr_h_

#include "xpTypes.h"
#include "xpEnums.h"
#include "xpsEnums.h"
#include "openXpsFdb.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * \brief Generic Init API for initializing the function pointers for 'fdb manager'
 *
 * \returns XP_STATUS indicating success or failure
 */
XP_STATUS xpsFdbInitApiFdbMgr(xpsDevice_t devId);


/**
 * \brief This method configures and add device specific primitives
 *        required for the FDB Manager implemenation.
 *
 * \param [in] devId Device Id of device.
 * \param [in] initType
 *
 * \return XP_STATUS
 */

XP_STATUS xpsFdbAddDeviceFdbMgr(xpsDevice_t devId, xpsInitType_t initType);


/**
 * \brief This method removes device specific primitives
 *        required for the FDB manager implementation.
 *
 * \param [in] devId Device Id of device.
 *
 * \return XP_STATUS
 */

XP_STATUS xpsFdbRemoveDeviceFdbMgr(xpsDevice_t devId);


/**
 * \brief This method allows the user to register a fdb learn handler api
 *
 * \param [in] devId Device Id of device.
 * \param [in] fdbLearnHandler func where fdbLearnHandler is of type XP_STATUS (*xpFdbLearnHandler)(xpsDevice_t, struct xphRxHdr*, void*, uint16_t)
 * \param [in] *userData User Provided Data
 *
 * \return XP_STATUS
 */

XP_STATUS xpsFdbRegisterLearnHandlerFdbMgr(xpsDevice_t devId,
                                           xpFdbLearnHandler fdbLearnHandler);

XP_STATUS xpsFdbUnRegisterLearnHandlerFdbMgr(xpsDevice_t devId);

XP_STATUS xpsFdbSetResetLearnHandlerFdbMgr(uint32_t isSet);


/**
 * \brief Find the index where a FDB entry is programmed, for a specific MAC and VLAN
 *
 * \param [in] devId Device Id of device.
 * \param [in] *fdbEntry Pointer to FDB entry structure, containing the MAC and the VLAN of the FDB entry.
 * \param [out] *index Index at which the FDB entry was found.
 *
 * \return XP_STATUS
 */

XP_STATUS xpsFdbFindEntryFdbMgr(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry,
                                uint32_t *index);


/**
 * \brief Add an entry into the FDB table
 *
 * \param [in] devId Device Id of device.
 * \param [in] fdbEntry Pointer to FDB entry structure.
 *
 * \return XP_STATUS
 *
 */
XP_STATUS xpsFdbAddEntryFdbMgr(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry,
                               xpsHashIndexList_t *indexList);


/**
 * \private
 * \brief Add an entry into the FDB table
 *
 * \param [in] devId
 * \param [in] *fdbEntry
 * \param [out] *indexList
 *
 * \returns XP_STATUS indicating success or failure
 *
 */
XP_STATUS xpsFdbAddHwEntryFdbMgr(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry,
                                 xpsHashIndexList_t *indexList);


/**
 * \brief Write an entry into the FDB table at a specific index
 *
 * \param [in] devId Device Id of device.
 * \param [in] index Index at which the FDB entry will be written to hardware.
 * \param [in] *fdbEntry Pointer to FDB entry structure that must be written to hardware.
 *
 * \return XP_STATUS
 *
 */
XP_STATUS xpsFdbWriteEntryFdbMgr(xpsDevice_t devId,  uint32_t index,
                                 xpsFdbEntry_t *fdbEntry);



/**
 * \brief Remove an entry from the FDB table for a specific MAC and VLAN
 *
 * \param [in] devId Device Id of device.
 * \param [in] *fdbEntry Pointer to FDB entry structure, containing the MAC and the VLAN of the FDB entry to be removed.
 *
 * \return XP_STATUS
 *
 */
XP_STATUS xpsFdbRemoveEntryFdbMgr(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry);


/**
 * \brief Get the fields of the FDB entry, given the fdb entry index
 *
 * \param [in] devId Device Id of device.
 * \param [in] index Index of the FDB entry.
 * \param [in] *fdbEntry Pointer to the FDB entry structure.
 *
 * \return XP_STATUS
 *
 */
XP_STATUS xpsFdbGetEntryByIndexFdbMgr(xpsDevice_t devId, uint32_t index,
                                      xpsFdbEntry_t *fdbEntry);


/**
 * \brief Get the fields of the FDB entry, given the MAC address and Vlan
 *
 * \param [in] devId Device Id of device.
 * \param [in] *fdbEntry Pointer to FDB entry structure, containing the mac address and vlan of the FDB entry.
 *
 * \return XP_STATUS
 *
 */
XP_STATUS xpsFdbGetEntryFdbMgr(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry);


/**
 * \brief This method sets the attribute of an FDB entry, given the Fdb entry Mac address and Vlan.
 *
 * \param [in] devId Device Id of device.
 * \param [in] *fdbEntry Pointer to FDB entry structure, containing the mac address and vlan of the FDB entry.
 * \param [in] field Name of the FDB attribute that needs to be set
 * \param [in] *data Value of the FDB attribute that needs to be set
 *
 * \return XP_STATUS
 */
XP_STATUS xpsFdbSetAttributeFdbMgr(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry,
                                   xpsFdbAttribute_e field, void *data);


/**
 * \brief This method gets the attribute of an FDB entry, given the Fdb entry Mac address and Vlan.
 *
 * \param [in] devId Device Id of device.
 * \param [in] *fdbEntry Pointer to FDB entry structure, containing the mac address and vlan of the FDB entry.
 * \param [in] field Name of the FDB attribute that needs to be obtained
 * \param [out] *data Value of the FDB attribute obtained.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsFdbGetAttributeFdbMgr(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry,
                                   xpsFdbAttribute_e field, void *data);

/**
 * \brief This method gets the used count for MAC entries
 *
 * \param [in] devId Device Id of device.
 * \param [out] *usedCount Value of the count
 *
 * \return XP_STATUS
 */
XP_STATUS xpsFdbGetEntriesCountFdbMgr(xpsDevice_t devId, uint32_t *usedCount);

uint32_t xpsFdbGetManagerId(void);

XP_STATUS xpsFdbGetEntryByKeyFdbMgr(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry);

XP_STATUS xpsFdbFdbMgrBrgSecurBreachCommandSet(xpsDevice_t devId,
                                               xpsPktCmd_e command);
#ifdef __cplusplus
}
#endif

#endif  //_xpsFdbMgr_h_
