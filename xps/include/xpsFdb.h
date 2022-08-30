// xpsFdb.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsFdb.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS FDB Manager
 * \Copyright (c) Marvell [2000-2020]. All rights reservered. Confidential.. ALL RIGHTS RESERVED.
 */

#ifndef _xpsFdb_h_
#define _xpsFdb_h_

#include "xpTypes.h"
#include "xpEnums.h"
#include "xpsEnums.h"
#include "openXpsFdb.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief This type defines the type of an FDB entry(static / dynamic).
 */
typedef xpFdbTblEntryType_e xpsFdbEntryType_e;


/**
 * @struct xpsFlushParams
 *
 * @brief This structure defines parameters used in FDB flush
 */

typedef struct
{
    xpsInterfaceId_t intfId;
    xpsVlan_t vlanId;
    xpsFdbEntryType_e entryType;
    xpFdbFlushType_e flushType;
} xpsFlushParams;

/**
 * \brief This defines the type of function pointer that the user can register for MAC learning.
 */
typedef XP_STATUS(*xpFdbLearnHandler)(xpsDevice_t, xpsFdbEntry_t,
                                      xpsFdbEntryAction_e);

/**
 * \brief This method initializes FDB Mgr with system specific
 *        references to various primitive managers and their initialization
 *        for a scope
 *
 * \param [in] scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsFdbInitScope(xpsScope_t scopeId);

/**
 * \brief This method cleans up from the FDB Mgr system specific
 *        references to various primitive managers for a scope.
 *
 * \param [in] scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsFdbDeInitScope(xpsScope_t scopeId);

/**
 * \brief This method configures and add device specific primitives
 *        required for the FDB FL manager.
 *
 * \param [in] devId Device Id of device.
 * \param [in] initType
 *
 * \return XP_STATUS
 */
XP_STATUS xpsFdbAddDeviceDefault(xpsDevice_t devId, xpsInitType_t initType);

/**
 * \brief Get the depth of the FDB table for given device
 *
 * \param [in] devId Device Id of device.
 * \param [out] depth Depth of the FDB table
 *
 * \return XP_STATUS
 */
XP_STATUS xpsFdbGetTableDepth(xpsDevice_t devId, uint32_t* depth);

/**
 * \brief This method removes device specific primitives
 *        required for the FDB FL manager.
 *
 * \param [in] devId Device Id of device.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsFdbRemoveDeviceDefault(xpsDevice_t devId);

/**
 * \brief Trigger the SW Control Aging Cycle for FDB
 *
 * \param [in] devId Device Id of device.
 *
 * \return XP_STATUS
 *
 */
XP_STATUS xpsFdbTriggerAging(xpsDevice_t devId);

/**
 * \brief Enable Aging on the FDB table entry
 *
 * \param [in] devId Device Id of device.
 * \param [in] enable Enable/disable aging for the FDB table.
 * \param [in] index of table entry
 * \return XP_STATUS
 *
 */
XP_STATUS xpsFdbConfigureEntryAging(xpsDevice_t devId, uint32_t enable,
                                    uint32_t index);

/**
 * \brief Get Aging time for the FDB table
 *
 * \param [in] devId Device Id of device.
 * \param [in] *enable
 *
 * \return XP_STATUS
 *
 */
XP_STATUS xpsFdbGetTableAgingStatus(xpsDevice_t devId, uint8_t *enable);

/**
 * \brief Get Aging time for the FDB table
 *
 * \param [in] devId Device Id of device.
 * \param [in] *enable
 * \param [in] index
 *
 * \return XP_STATUS
 *
 */
XP_STATUS xpsFdbGetEntryAgingStatus(xpsDevice_t devId, uint8_t *enable,
                                    uint32_t index);

/**
 * \brief Get Aging time for the FDB table
 *
 * \param [in] devId Device Id of device.
 * \param [in] *agingExpo
 *
 * \return XP_STATUS
 *
 */
XP_STATUS xpsFdbGetAgingTime(xpsDevice_t devId, uint32_t *agingExpo);

/**
 * \brief Get the fields of the FDB entry, given the MAC address and Vlan
 *
 * \param [in] devId Device Id of device.
 * \param [in] *fdbEntry Pointer to FDB entry structure, containing the mac address and vlan of the FDB entry.
 *
 * \return XP_STATUS
 *
 */
XP_STATUS xpsFdbGetEntryDefault(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry);

/**
 * \brief Get the no of valid entries in the Fdb table
 *
 * \param [in] devId Device Id of device.
 * \param [out] numofvalidentries Num of entries in the fdb table.
 *
 * \return XP_STATUS
 *
 */
XP_STATUS xpsFdbGetNumOfValidEntries(xpsDevice_t devId,
                                     uint32_t *numOfValidEntries);

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
XP_STATUS xpsFdbGetEntryByIndexDefault(xpsDevice_t devId, uint32_t index,
                                       xpsFdbEntry_t *fdbEntry);

/**
 * \brief This method removes the MAC entry from the control MAC
 *        table and from the FDB table.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] macAddr
 *
 * \return XP_STATUS
 */
XP_STATUS xpsFdbRemoveControlMacEntry(xpsDevice_t devId, uint32_t vlanId,
                                      macAddr_t macAddr);

/**
 * \brief This method gets the reason code programmed for a MAC
 *        entry.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] macAddr
 * \param [out] reasonCode
 *
 * \return XP_STATUS
 */
XP_STATUS xpsFdbGetControlMacEntryReasonCode(xpsDevice_t devId, uint32_t vlanId,
                                             macAddr_t macAddr, uint32_t *reasonCode);

/**
 * \brief This method allows the user to Call the Mac Learning function
 *
 * \param [in] devId Device Id of device.
 * \param [in] xphHdr XP rx header
 * \param [in] buf buffer
 * \param [in] bufSize buffer Size
 * \param [in] *userData User Provided Data
 *
 * \return XP_STATUS
 */
XP_STATUS xpsFdbMacLearnigCb(xpsDevice_t devId, xphRxHdr *xphHdr, void *buf,
                             uint16_t bufSize, void *userData);

/**
 * \brief This method allows the user to register a fdb learn handler api
 *
 * \param [in] devId Device Id of device.
 * \param [in] fdbLearnHandler func where fdbLearnHandler is of type XP_STATUS (*xpFdbLearnHandler)(xpsDevice_t, struct xphRxHdr*, void*, uint16_t)
 * \param [in] *userData User Provided Data
 *
 * \return XP_STATUS
 */
XP_STATUS xpsFdbRegisterLearnHandlerDefault(xpsDevice_t devId,
                                            xpFdbLearnHandler fdbLearnHandler);

XP_STATUS xpsFdbUnRegisterLearnHandlerDefault(xpsDevice_t devId);
/**
 * \brief Handler to get the L2 encap type, given the interface and vlan id.
 *
 * \param [in] devId Device Id of device.
 * \param [in] intfId Interface Id
 * \param [in] bdId Bridge domain ids
 * \param [in] isTagged Info on if the incoming packet is tagged
 * \param [in] tagVlanId Vlan Id present in the vlan tag
 * \param [out] *encapType Encapsulation type for the given interface and vlan id.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsFdbGetL2EncapType(xpsDevice_t devId, uint32_t intfId,
                               uint32_t bdId, uint8_t isTagged, uint32_t tagVlanId, uint32_t* encapType,
                               xpVlan_t *encapVid, uint8_t *transTnl, bool *isBdBridge);

/**
 * \brief Register the default Aging Handler for the Fdb Table.
 *
 * \param [in] devId Device Id of device.
 *
 * \return XP_STATUS
 *
*/
XP_STATUS xpsFdbRegisterDefaultAgingHandler(xpsDevice_t devId);

/**
 * \brief Flushes all the fdb entries of a specific type
 *
 * \param [in] devId Device Id of device.
 * \param [in] entryType Type of FDB entry that must be flushed.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsFdbFlushEntryDefault(xpsDevice_t devId,
                                  xpsFdbEntryType_e entryType);

/**
 * \brief Flushes all the fdb entries of a specific type and
 *        update SW shadow memory if updateShadow == 1
 *
 * \param [in] devId Device Id of device.
 * \param [in] entryType Type of FDB entry that must be flushed.
 * \param [in] updateShadow - 1: flushEntries and update shadow, 0: Only flushEntries
 *
 * \return XP_STATUS
 */
XP_STATUS xpsFdbFlushEntryWithShadowUpdate(xpsDevice_t devId,
                                           xpsFdbEntryType_e entryType, uint8_t updateShadow);

/**
 * \brief Flushes all the fdb entries for a particular interface, given the interface Id
 *
 * \param [in] devId Device Id of device.
 * \param [in] intfId Interface id of the FDB entry that must be flushed.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsFdbFlushEntryByIntfDefault(xpsDevice_t devId,
                                        xpsInterfaceId_t intfId, xpsFdbEntryType_e entryType);

/**
 * \brief Flushes all the fdb entries for a particular interface, given the interface Id
 *
 * \param [in] devId Device Id of device.
 * \param [in] intfId Interface id of the FDB entry that must be flushed.
 * \param [in] updateShadow - 1: flushEntries and update shadow, 0: Only flushEntries
 *
 * \return XP_STATUS
 */
XP_STATUS xpsFdbFlushEntryByIntfWithShadowUpdate(xpsDevice_t devId,
                                                 xpsInterfaceId_t intfId, xpsFdbEntryType_e entryType, uint8_t updateShadow);

/**
 * \brief Flushes all the fdb entries for a particular vlan
 *
 * \param [in] devId Device Id of device.
 * \param [in] vlanId Vlan id of the FDB entry that must be flushed.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsFdbFlushEntryByVlanDefault(xpsDevice_t devId, xpsVlan_t vlanId,
                                        xpsFdbEntryType_e entryType);

/**
 * \brief Flushes all the fdb entries for a particular vlan
 *
 * \param [in] devId Device Id of device.
 * \param [in] vlanId Vlan id of the FDB entry that must be flushed.
 * \param [in] updateShadow - 1: flushEntries and update shadow, 0: Only flushEntries
 *
 * \return XP_STATUS
 */
XP_STATUS xpsFdbFlushEntryByVlanWithShadowUpdate(xpsDevice_t devId,
                                                 xpsVlan_t vlanId, xpsFdbEntryType_e entryType, uint8_t updateShadow);

/**
 * \brief Flushes all the fdb entries for a particular vlan and interface pair
 *
 * \param [in] devId Device Id of device.
 * \param [in] intfId Interface id of the FDB entry that must be flushed.
 * \param [in] vlanId Vlan id of the FDB entry that must be flushed.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsFdbFlushEntryByIntfVlanDefault(xpsDevice_t devId,
                                            xpsInterfaceId_t intfId, xpsVlan_t vlanId, xpsFdbEntryType_e entryType);

/**
 * \brief Flushes all the fdb entries for a particular vlan and interface pair
 *
 * \param [in] devId Device Id of device.
 * \param [in] intfId Interface id of the FDB entry that must be flushed.
 * \param [in] vlanId Vlan id of the FDB entry that must be flushed.
 * \param [in] updateShadow - 1: flushEntries and update shadow, 0: Only flushEntries
 *
 * \return XP_STATUS
 */
XP_STATUS xpsFdbFlushEntryByIntfVlanWithShadowUpdate(xpsDevice_t devId,
                                                     xpsInterfaceId_t intfId, xpsVlan_t vlanId, xpsFdbEntryType_e entryType,
                                                     uint8_t updateShadow);

/**
 * \brief clear  status if the FDB entry had gone through a source MAC lookup hit
 *
 * \param [in] devId Device Id of device.
 * \param [in] index FDB table index.
 *
 * \return status
 */
XP_STATUS xpsFdbClearSourceMacHit(xpsDevice_t devId, uint32_t index);

/**
 * \brief Get status if the FDB entry had gone through a source MAC lookup hit
 *
 * \param [in] devId Device Id of device.
 * \param [in] index FDB table index.
 * \param [out] hwFlagStatus status of mac source look up bit
 *
 * \return status
 */
XP_STATUS xpsFdbGetSourceMacHit(xpsDevice_t devId, uint32_t index,
                                uint8_t *hwFlagStatus);


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
XP_STATUS xpsFdbSetAttributeDefault(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry,
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
XP_STATUS xpsFdbGetAttributeDefault(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry,
                                    xpsFdbAttribute_e field, void *data);

/**
 * \brief This method sets the attribute of an FDB entry, given the index of the Fdb entry.
 *
 * \param [in] devId Device Id of device.
 * \param [in] index Index of the FDB entry.
 * \param [in] field Name of the FDB attribute that needs to be set
 * \param [in] *data Value of the FDB attribute that needs to be set
 *
 * \return XP_STATUS
 */
XP_STATUS xpsFdbSetAttributeByIndex(xpsDevice_t devId, uint32_t index,
                                    xpsFdbAttribute_e field, void *data);

/**
 * \brief This method gets the attribute of an FDB entry, given the index of the Fdb entry.
 *
 * \param [in] devId Device Id of device.
 * \param [in] index Index of the FDB entry.
 * \param [in] field Name of the FDB attribute that needs to be obtained
 * \param [out] *data Value of the FDB attribute obtained.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsFdbGetAttributeByIndex(xpsDevice_t devId, uint32_t index,
                                    xpsFdbAttribute_e field, void *data);

/**
 * \brief This method initializes FDB Mgr with system specific
 *        references to various primitive managers and their initialization.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsFdbInit(void);

/**
 * \brief This method cleans up from the FDB Mgr system specific
 *        references to various primitive managers.
 *
 * \return XP_STATUS
 */

XP_STATUS xpsFdbDeInit(void);

/**
 * \brief Get MAC learning rate
 *
 * \param [in] devId
 * \param [out] rate
 *
 * \return MAC learning rate
 */
XP_STATUS xpsFdbGetMacRate(xpsDevice_t devId, uint64_t *rate);
/**
 * \public
 * \brief Gets the size of Fdb table.
 *
 * This method calculates size of Fdb table.
 *
 * \param [in] devId
 * \param [out] tsize
 *
 * \return XP_STATUS
 */
XP_STATUS xpsFdbGetTableSize(xpsDevice_t devId, uint32_t *tsize);
/**
 * \public
 * \brief Reads valid entry from Fdb table at the index.
 *
 * This method reads entry  of Fdb table at the index only if the entry is valid.
 *
 * \param [in] devId
 * \param [in] index
 * \param [out] fdbEntry
 *
 * \return XP_STATUS
 */

XP_STATUS xpsFdbGetValidEntryByIndex(xpsDevice_t devId, uint32_t index,
                                     xpsFdbEntry_t *fdbEntry);
/**
 * \public
 * \brief counts number of valid entries in Fdb table.
 *
 * \param [in] devId
 * \param [out] count.
 *
 * \return XP_STATUS
 */

XP_STATUS xpsCountFdbObjects(xpsDevice_t xpsDevId, uint32_t* count);

/**
 * \brief This method gets rehash level for FDB table
 *
 * \param [in] devId
 * \param [out] numOfRehashLevels
 *
 * \return XP_STATUS
 */
XP_STATUS xpsFdbGetRehashLevel(xpDevice_t devId, uint8_t* numOfRehashLevels);

/**
 * \brief clear  status if the FDB entry had gone through a source MAC lookup hit
 *
 * \param [in] devId Device Id of device.
 * \param [in] index FDB table index.
 *
 * \return status
 */
XP_STATUS xpsFdbClearSourceMacHit(xpsDevice_t devId, uint32_t index);
/**
 * \brief Get status if the FDB entry had gone through a source MAC lookup hit
 *
 * \param [in] devId Device Id of device.
 * \param [in] index FDB table index.
 * \param [out] hwFlagStatus status of mac source look up bit
 *
 * \return status
 */
XP_STATUS xpsFdbGetSourceMacHit(xpsDevice_t devId, uint32_t index,
                                uint8_t *hwFlagStatus);

/**
 * \brief This method sets rehash level for FDB table
 *
 * \param [in] devId
 * \param [in] numOfRehashLevels
 *
 * \return XP_STATUS
 */
XP_STATUS xpsFdbSetRehashLevel(xpDevice_t devId, uint8_t numOfRehashLevels);

/**
 * \brief This method gets rehash level for FDB table
 *
 * \param [in] devId
 * \param [out] numOfRehashLevels
 *
 * \return XP_STATUS
 */
XP_STATUS xpsFdbGetRehashLevel(xpDevice_t devId, uint8_t* numOfRehashLevels);


/**
 * \brief L2 learn packet parser which returns FDB entry ready
 *        for adding to FDB table.
 *
 * \param [in]  devId      Device ID.
 * \param [in]  xphHdr     Packet header.
 * \param [in]  buf        Buffer pointer.
 * \param [in]  bufSize    Buffer size.
 * \param [out] fdbEntry   FDB entry.
 * \param [out] reasonCode Reason code.
 *
 * \return status.
 */
XP_STATUS xpsFdbParseLearnPacket(xpDevice_t devId, xphRxHdr *xphHdr, void *buf,
                                 uint16_t bufSize, xpsFdbEntry_t *fdbEntry, uint32_t *reasonCode);

/**
 * \brief Clear Bucket State in the table for given device
 *
 * \param [in] devId
 * \param [in] tblCopyIdx
 *
 * \returns XP_STATUS indicating success or failure
 */
XP_STATUS xpsFdbClearBucketState(xpDevice_t devId, uint8_t tblCopyIdx);


/**
 * \brief Configure vlanaware mode
 *
 * \param [in] devId
 *
 * \returns XP_STATUS indicating success or failure
 */
XP_STATUS xpsFdbDefaults(xpDevice_t devId);


/**
 * \brief Preventing security breach by MAC
 *
 * \param [in] devId
 *
 * \returns XP_STATUS indicating success or failure
 */
XP_STATUS xpsFdbMacSecurity(xpDevice_t devId);

/**
 * \brief Set Msg Rate Limit to update CPU
 *
 * \param [in] devId
 * \param [in] rateLimit
 * \param [in] enable
 *
 * \returns XP_STATUS indicating success or failure
 */
XP_STATUS xpsFdbSetMsgRateLimit(xpsDevice_t devId, uint32_t rateLimit,
                                uint8_t enable);

/**
 * \brief Get Number of FDB entries count
 *
 * \param [in] devId
 * \param [out] usedCount
 *
 * \returns XP_STATUS indicating success or failure
 */
XP_STATUS xpsFdbGetEntriesCountDefault(xpsDevice_t devId, uint32_t *usedCount);


/**
 * \brief Generic Init API for initialising the function pointers
 *
 * \returns XP_STATUS indicating success or failure
 */
XP_STATUS xpsFdbInitApi(xpsDevice_t devId);


/**
 * \brief Find the index where a FDB entry is programmed, for a specific MAC and VLAN
 *
 * \param [in] devId Device Id of device.
 * \param [in] *fdbEntry Pointer to FDB entry structure, containing the MAC and the VLAN of the FDB entry.
 * \param [out] *index Index at which the FDB entry was found.
 *
 * \return XP_STATUS
 */

XP_STATUS xpsFdbFindEntryDefault(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry,
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
XP_STATUS xpsFdbAddEntryDefault(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry,
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
XP_STATUS xpsFdbAddHwEntryDefault(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry,
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
XP_STATUS xpsFdbWriteEntryDefault(xpsDevice_t devId,  uint32_t index,
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
XP_STATUS xpsFdbRemoveEntryDefault(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry);


XP_STATUS xpsFdbInitApiDefault(xpsDevice_t devId);

XP_STATUS xpsFdbSetResetLearnHdl(uint32_t isSet);


/*===========================================================*/
/*              FUNCTION POINTERS DECLARATIONS               */
/*===========================================================*/

extern XP_STATUS(*xpsFdbAddDevice)(xpsDevice_t devId, xpsInitType_t initType);
extern XP_STATUS(*xpsFdbRemoveDevice)(xpsDevice_t devId);

extern XP_STATUS(*xpsFdbRegisterLearnHandler)(xpsDevice_t devId,
                                              xpFdbLearnHandler fdbLearnHandler);
extern XP_STATUS(*xpsFdbUnregisterLearnHandler)(xpsDevice_t devId);
extern XP_STATUS(*xpsFdbRegisterAgingHandler)(xpsDevice_t devId,
                                              xpFdbAgingHandler ageHandler);

/* (pointer to) FDB flush functions : global/interface/vlan/combination */
extern XP_STATUS(*xpsFdbFlushEntry)(xpsDevice_t devId,
                                    xpsFdbEntryType_e entryType);
extern XP_STATUS(*xpsFdbFlushEntryByIntf)(xpsDevice_t devId,
                                          xpsInterfaceId_t intfId, xpsFdbEntryType_e entryType);
extern XP_STATUS(*xpsFdbFlushEntryByVlan)(xpsDevice_t devId, xpsVlan_t vlanId,
                                          xpsFdbEntryType_e entryType);
extern XP_STATUS(*xpsFdbFlushEntryByIntfVlan)(xpsDevice_t devId,
                                              xpsInterfaceId_t intfId, xpsVlan_t vlanId, xpsFdbEntryType_e entryType);

extern XP_STATUS(*xpsFdbFindEntry)(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry,
                                   uint32_t *index);
extern XP_STATUS(*xpsFdbAddEntry)(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry,
                                  xpsHashIndexList_t *indexList);
extern XP_STATUS(*xpsFdbAddHwEntry)(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry,
                                    xpsHashIndexList_t *indexList);
extern XP_STATUS(*xpsFdbWriteEntry)(xpsDevice_t devId,  uint32_t index,
                                    xpsFdbEntry_t *fdbEntry);
extern XP_STATUS(*xpsFdbRemoveEntry)(xpsDevice_t devId,
                                     xpsFdbEntry_t *fdbEntry);
extern XP_STATUS(*xpsFdbGetEntryByIndex)(xpsDevice_t devId, uint32_t index,
                                         xpsFdbEntry_t *fdbEntry);
extern XP_STATUS(*xpsFdbGetEntry)(xpsDevice_t devId, xpsFdbEntry_t *fdbEntry);
extern XP_STATUS(*xpsFdbSetAttribute)(xpsDevice_t devId,
                                      xpsFdbEntry_t *fdbEntry, xpsFdbAttribute_e field, void *data);
extern XP_STATUS(*xpsFdbGetAttribute)(xpsDevice_t devId,
                                      xpsFdbEntry_t *fdbEntry, xpsFdbAttribute_e field, void *data);
extern XP_STATUS(*xpsFdbGetEntriesCount)(xpsDevice_t devId,
                                         uint32_t *usedCount);
extern XP_STATUS xpsFdbFdbMgrBrgSecurBreachCommandSet(xpsDevice_t devId,
                                                      xpsPktCmd_e command);
extern XP_STATUS xpsFdbFdbMgrBrgSecurBreachCommandGet(xpsDevice_t devId,
                                                      xpsPktCmd_e *command);

void SetFdbFlushInProgress(bool val);
#ifdef __cplusplus
}
#endif

#endif  //_xpsFdb_h_
