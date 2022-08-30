// xpsNat.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
 * \file xpsNat.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS NAT Manager
 */

#ifndef _XPSNAT_H_
#define _XPSNAT_H_

#include "xpsEnums.h"
//#include "xpNatMgr.h"
#include "xpsInit.h"
#ifdef __cplusplus
extern "C" {
#endif



typedef struct __xpsNatEntryKey_t
{
    ipv4Addr_t SrcAddress;
    uint32_t SrcPort;
    ipv4Addr_t DestAddress;
    uint32_t DestPort;
    uint32_t Bd;
    uint32_t Flag;
    uint32_t Protocol;
} xpsNatEntryKey_t;

typedef struct __xpsNatEntryMask_t
{
    ipv4Addr_t SrcAddress;
    uint32_t SrcPort;
    ipv4Addr_t DestAddress;
    uint32_t DestPort;
    uint32_t Bd;
    uint32_t Flag;
    uint32_t Protocol;
} xpsNatEntryMask_t;


typedef struct __xpsNatEntryData_t
{
    ipv4Addr_t SIPAddress;
    uint32_t srcPort;
    ipv4Addr_t DIPAddress;
    uint32_t destPort;
    uint32_t vif;
} xpsNatEntryData_t;

/**
 * \brief This API initializes the NAT mgr. Does NAT specific global level
 *        initialization for a scope.
 *
 * \param [in] scopeId
 *
 * \return XP_STATUS
*/
XP_STATUS xpsNatInitScope(xpsScope_t scopeId);


/**
 * \brief This API de-initializes the NAT mgr. Does NAT specific global
 *     level cleanups for a scope.
 *
 * \param [in] scopeId
 *
 * \return XP_STATUS
 */

XP_STATUS xpsNatDeInitScope(xpsScope_t scopeId);


/**
 * \brief This API adds a device to the NAT manager. Does NAT specific
 *        device level initializations.
 *
 * \param [in] devId Device Id of device
 * \param [in] initType
 *
 * \return XP_STATUS
 */
XP_STATUS xpsNatAddDevice(xpsDevice_t devId, xpsInitType_t initType);

/**
 * \brief This API removes a device from the NAT manager. Does NAT specific
 *    device level cleanups.
 *
 *
 * \param [in] devId Device Id of device
 * \return XP_STATUS
 */
XP_STATUS xpsNatRemoveDevice(xpsDevice_t devId);

/**
 * \brief Set mdt nat config.
 *
 * \param [in] devId Device Id of device
 * \param [in] index index of entry
 * \param [in] value to set MDT
 * \return XP_STATUS
 */
XP_STATUS xpsNatSetMdtConfig(xpsDevice_t devId, uint32_t index, uint32_t value);

/**
 *  \brief Add's an internal NAT entry to the table
 *
 *  param [in] devId Device Id of device
 *  param [in] index Tcam index where the entry will be programmed
 *  param [in] natkey key value to be programmed in NAT table.
 *  param [in] natmask mask for the key for the key in the NAT table.
 *  param [in] natdata data programmed as the action for the NAT table entry.
 *
 *  \return XP_STATUS
 */
XP_STATUS xpsNatAddInternalEntry(xpsDevice_t devId, uint32_t index,
                                 xpsNatEntryKey_t *natkey, xpsNatEntryMask_t* natmask,
                                 xpsNatEntryData_t* natdata);


/**
 * \brief Add's an external NAT entry to the table
 * param [in] devId Device Id of device
 * param [in] index Tcam index where the entry will be programmed.
 * param [in] natkey key value to be programmed in NAT table.
 * param [in] natmask mask for the key for the key in the NAT table.
 * param [in] natdata data programmed as the action for the NAT table entry.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsNatAddExternalEntry(xpsDevice_t devId, uint32_t index,
                                 xpsNatEntryKey_t *natkey, xpsNatEntryMask_t* natmask,
                                 xpsNatEntryData_t *natdata);

/**
 * \brief Add's a filter rule which will set the pktCmd to trap in the data
 * \param [in] devId Device Id of device
 * \param [in] index Tcam index where the entry will be programmed.
 * \param [in] natkey key value to be programmed in NAT table.
 * \param [in] natmask mask for the key for the key in the NAT table.
 * \param [in] natdata data programmed as the action for the NAT table entry.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsNatAddFilterRule(xpsDevice_t devId, uint32_t index,
                              xpsNatEntryKey_t *natkey, xpsNatEntryMask_t* natmask,
                              xpsNatEntryData_t *natdata);

/**
 * \brief Add's a double NAT entry to the table
 * \param [in] devId Device Id of device
 * \param [in] index Tcam index where the entry will be programmed.
 * \param [in] natkey key value to be programmed in NAT table.
 * \param [in] natmask mask for the key for the key in the NAT table.
 * \param [in] natdata data programmed as the action for the NAT table entry.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsNatAddDoubleEntry(xpsDevice_t devId, uint32_t index,
                               xpsNatEntryKey_t *natkey, xpsNatEntryMask_t* natmask,
                               xpsNatEntryData_t *natdata);

/**
 * \brief deletes the entry from the table
 *
 * \param [in] devId devId Device Id of device
 * \param [in] index Tcam index from where the entry will be removed.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsNatDeleteEntry(xpsDevice_t devId, uint32_t index);
//TO DO : xpsNatDeleteEntry need to implement in *.c

/**
 * \brief reads from HW and returns the key,mask and data
 *
 * \param [in] devId devId Device Id of device
 * \param [in] index Tcam index where the entry is read
 * \param [out] natKey key value as read from HW
 * \param [out] natMask mask value as read from HW
 * \param [out] natData data value of the action for the nat entry read from HW
 *
 * \return XP_STATUS
 */
XP_STATUS xpsNatGetEntry(xpsDevice_t devId, uint32_t index,
                         xpsNatEntryKey_t *natKey, xpsNatEntryMask_t *natMask,
                         xpsNatEntryData_t *natData);

/**
 * \brief This API initializes the NAT mgr. Does NAT specific global level
 *        initialization.
 *
 * \return XP_STATUS
*/
XP_STATUS xpsNatInit(void);

/**
 * \brief This API de-initializes the NAT mgr. Does NAT specific global
 *     level cleanups.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsNatDeInit(void);

#ifdef __cplusplus
}
#endif

#endif //_XPSNAT_H_
