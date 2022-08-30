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
 *   FILE : openXpsInit.h
 *
 *   Abstract: This file defines the apis for Initialization Management in OpenXPS.
 ************************************************************************/
/**
 * \file openXpsInit.h
 * \brief This file contains API prototypes and type definitions
 *        for the openXps Initialization Management
 * \Copyright (c) Marvell [2000-2020]
 */

#ifndef _openXpsInit_h_
#define _openXpsInit_h_

#include "xpEnums.h"
#include "openXpsTypes.h"
#include "openXpsEnums.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *\brief This type defines an enumeration of the
 *       system specific (global) range profiles
 *       defined in the SDK.
 *       Additional Customized range profiles can be
 *       defined and managed within the SDK.
 */
typedef enum xpsRangeProfileType_t
{
    RANGE_FULL_FEATURED = 0,
    RANGE_FULL_FEATURED_2,
    RANGE_ROUTE_CENTRIC,
    RANGE_TUNNEL_CENTRIC,
    RANGE_SAI_QACL,
    RANGE_SEGMENT_ROUTE,
    RANGE_PROFILE_MAX
} xpsRangeProfileType_t;

/**
 *\struct xpsDevConfigStruct_t
 *\brief This type defines device configurations
 */
typedef struct xpsDevConfigStruct_t
{
    XP_PROFILE_TYPE profileType;        ///< Profile selection
    xpSkuMode_e mode;                   ///< SKU selection
    xpsDalType_e
    dalType;               ///< Driver Abstration Layer (DAL) type selection
    uint8_t skuInitSkip;                ///< Skip SKU initialization
} xpsDevConfigStruct_t;

/**
 * \brief Defines XDK initialization routine.
 *
 * This API initializes complete SDK and required data structures, managers.
 * This is a device agnostic (global) initialization.
 *
 * \param [in] rpType
 * \param [in] initType
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSdkInit(XP_DEV_TYPE_T devType, xpsRangeProfileType_t rpType,
                     xpsInitType_t initType);

/**
 * \brief This API initializes a XP device identified by its
 *        device ID.
 *
 * This API will reset the device and initialize the device to a
 * configuration defined within the xpDevConfigStruct passed in
 * as an argument.
 *
 * \param devId
 * \param initType
 * \param mode
 * \param packetInterface
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSdkDeviceDefaultInit(xpsDevice_t devId, xpsInitType_t initType,
                                  xpSkuMode_e mode, xpPacketInterface packetInterface);

/**
 * \brief Initializes a device per default devConfigStruct
 *  identified by its' device ID.
 *
 * This API will reset the device and initialize the device to a configuration
 * defined with xpsDevConfigStruct_t.
 *
 * \param [in] devId
 * \param [in] initType - A type of initialization supported by the SDK (Cold, Warm, Warm_Sync_Shadow, Warm_Sync_Hardware).
 * \param [in] devConfig - the configuration the device to be initialized with
 * \param [in] packetInterface
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSdkDefaultDevInit(xpsDevice_t devId, xpsInitType_t initType,
                               xpsDevConfigStruct_t *devConfig, xpPacketInterface packetInterface);

/**
 * \brief Remove the device, and free corresponding data
 * structures maintained in the SDK.
 *
 * This API will remove the device and de-initialize its corresponding data
 * structures and software state.
 *
 * \param [in] devId
 * \return XP_STATUS
 */
XP_STATUS xpsSdkDevDeInit(xpsDevice_t devId);

/**
 * \brief Initialize the logger.
 *
 * This API will initialize the logger submodule within the SDK.
 * \return const char **
 */
const char **xpsSdkLoggerInit(void);

/**
 * \brief This API will enable logging on the console.
 *
 *
 * \param [in] void
 * \return int
 */
XP_STATUS xpsSdkLogConsole(void);

/**
 * \brief Enable logging to a file.
 *
 * This API will enable logging to file.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSdkLogToFile(const char*);

/**
 * \brief Log the SDK version.
 *
 * This API will log the SDK version string.
 *
 * \return STATUS
 */
XP_STATUS xpsSdkLogVersion(void);

/**
 * \brief This API sets SDK logging options.
 *
 * This API initializes SDK's logging options.
 *
 * \param [in] id
 * \param [in] param
 * \return STATUS
 */
XP_STATUS xpsSdkSetLoggerOptions(uint32_t id, char *param);

/**
 * \brief Initialize debug shell server.
 *
 * This API initializes debug shell server.
 *
 * \return STATUS
 */
XP_STATUS xpsShellInit(void);

/**
 * \brief De-initialize debug shell server.
 *
 * This API de-initializes debug shell server.
 *
 * \return STATUS
 */
XP_STATUS xpsShellDeInit(void);

/**
 * \brief xpsServiceCpuEnable Enable SCPU
 * \@param[in] devId
 * \@param[in] enable
 *
 * \return STATUS
 */
XP_STATUS xpsServiceCpuEnable(xpDevice_t devId, bool enable);

/**
 * \brief xpsServiceCpuLoadFirmware Load firmware to SCPU
 * \@param[in] devId
 * \@param[in] SCPU firmware File
 *
 * \return STATUS
 */
XP_STATUS xpsServiceCpuLoadFirmware(xpDevice_t devId, const char* scpuFwPath);

/**
 * \brief xpsLedModeSet
 * \@param[in] devId
 * \@param[in] ledMode
 *
 * \return STATUS
 */
XP_STATUS xpsLedModeSet(xpDevice_t devId, const char* ledModeStr);

XP_STATUS xpsCreatePortMappingProfile(XP_DEV_TYPE_T devType, int numOfPorts);
XP_STATUS xpsUpdatePortMappingProfile(XP_DEV_TYPE_T devType,
                                      int logicalPortNumber, int externalUsrPortIdx, char * infModeStr, int speed);

#ifdef __cplusplus
}
#endif

#endif  //_openXpsInit_h_
