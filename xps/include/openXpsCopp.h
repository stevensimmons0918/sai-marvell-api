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
 *   FILE : openXpsCopp.h
 *
 *   Abstract: This file defines the apis for control plane policing in OpenXPS.
 ************************************************************************/
/**
 * \file openXpsCopp.h
 * \brief This file contains API prototypes and type definitions
 *        for the openXps control plane policing
 * \Copyright (c) Marvell [2000-2020]
 */

#ifndef _openXpsCopp_h_
#define _openXpsCopp_h_

#include "openXpsTypes.h"
#include "openXpsEnums.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief This method adds a COPP entry with key {sourcePort,
 *        reasonCode}
 *
 * \param [in] devId
 * \param [in] sourcePort
 * \param [in] reasonCode
 * \param [in] entry
 * \param [out] indexList
 *
 * \return XP_STATUS
 */
XP_STATUS xpsCoppAddEntry(xpDevice_t devId, uint32_t sourcePort,
                          uint32_t reasonCode, xpCoppEntryData_t entry, xpsHashIndexList_t *indexList);

/**
 * \brief This method removes COPP entry with key {sourcePort,
 *        reasonCode}
 *
 * \param [in] devId
 * \param [in] sourcePort
 * \param [in] reasonCode
 *
 * \return XP_STATUS
 */
XP_STATUS xpsCoppRemoveEntry(xpDevice_t devId, uint32_t sourcePort,
                             uint32_t reasonCode);

#ifdef __cplusplus
}
#endif

#endif  //_openXpsCopp_h_
