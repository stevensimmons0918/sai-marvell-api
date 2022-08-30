// xpsOpenflow.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsOpenflow.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS Openflow Manager
 */


#ifndef _xpsOpenflow_h_
#define _xpsOpenflow_h_

#include "xpsEnums.h"
//#include "xpOfMgr.h"
//#include "xpOpenFlowKeyByteMask.h"
#include "openXpsOpenflow.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief This method initializes xpOfMgr with system specific
 *        references to various primitive managers and their initialization in a scope.
 *
 * \param [in] scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsOpenflowInitScope(xpsScope_t scopeId);

/**
 * \brief This method uninitializes xpOfMgr in a scope.
 *
 * \param [in] scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsOpenflowDeInitScope(xpsScope_t scopeId);

/**
 * \brief This method adds a device specific primitives for this feature.
 *
 * \param [in] devId  Physical device identifier
 * \param [in] initType  Device initialization mode
 *
 * \return XP_STATUS
 */
XP_STATUS xpsOfAddDevice(xpsDevice_t devId, xpInitType_t initType);

/**
 * \brief This method removes a device specific primitives for this feature.
 *
 * \param [in] devId  Physical device identifier
 *
 * \return XP_STATUS
 */
XP_STATUS xpsOfRemoveDevice(xpsDevice_t devId);

/**
 * \brief Update memory addresses stored in the DLL of a Open flow group db entry
 *
 * \\param [in] scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsOfUpdateMemoryAddressInOfGroupDbEntry(xpsScope_t scopeId);

/**
 * \brief This method returns bands’ statistic of one meter entry.
 *
 * \param [in] devId  Physical device identifier
 * \param [in] dpId  Logical OpenFlow datapath identifier
 * \param [in] tableId  OpenFlow table identifier
 * \param [in] tableType  HW table type
 * \param [in] numOfValidEntries
 * \param [in] startIndex
 * \param [in] endIndex
 * \param [in] logFile
 * \param [in] detailFormat
 * \param [in] silentMode
 * \param [in] tblCopyIdx
 *
 * \return XP_STATUS
 */
XP_STATUS xpsOfDisplayTable(xpDevice_t devId, xpDatapath_t dpId,
                            uint32_t tableId, xpOfTableType_e tableType,
                            uint32_t *numOfValidEntries, uint32_t startIndex, uint32_t endIndex,
                            char* logFile,
                            uint32_t detailFormat, uint32_t silentMode, uint8_t tblCopyIdx);

/**
 * \brief This method initializes xpOfMgr with system specific
 *        references to various primitive managers and their initialization.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsOpenflowInit(void);

/**
 * \brief This method uninitializes xpOfMgr.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsOpenflowDeInit(void);

#ifdef __cplusplus
}
#endif

#endif
