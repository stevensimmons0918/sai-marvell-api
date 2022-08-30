// xpsInit.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsInit.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS Init Manager
 */

#ifndef _xpsInit_h_
#define _xpsInit_h_

#include "xpTypes.h"
#include "xpEnums.h"
#include "xpsEnums.h"
#include "openXpsInit.h"

#ifdef __cplusplus
extern "C" {
#endif

#define portIdxToMacPortMappingForFalconDb12_8_MAC(_idx)\
        (_idx >= 224)?\
        (_idx - 224) : (_idx + 32)

#define portIdxToMacPortMappingForFalconDb6_4_MAC(_idx) _idx

#define externalUserPortIdxToMacPortMapping_MAC(_devtype,_idx)\
    (IS_DEVICE_FALCON_12_8(_devtype))?\
    portIdxToMacPortMappingForFalconDb12_8_MAC(_idx):portIdxToMacPortMappingForFalconDb6_4_MAC(_idx)

/* Packet Driver config structure */
typedef struct xpPacketDriverConfig
{
    xpPacketInterface packetInterface;  /* packetdriver interface  */
    uint8_t rxConfigMode;   /* packet driver rx config mode */
} xpPcktDrvConfig;
/**
 * \brief xpDevConfigStruct
 * device config structure xpDevConfigStruct
 */
typedef struct xpDevConfigStruct
{
    XP_PIPE_MODE
    pipeLineMode;          ///< pipe line no (1,2 or 4 and default mode is 1)
    XP_PROFILE_TYPE profileType;        ///< Select Profile
    uint32_t performanceMode;         ///< performance mode
    XP_DAL_TYPE_T dalType;              ///< dal type
    uint8_t shadowNeeded;        ///< Flag : shadow needed
    uint8_t shadowWm;        ///< Flag : shadow wm
    uint8_t staticTablesNeeded;        ///< Flag: static tables needed
    uint8_t isRemote;        ///< Flag: is remote
    uint8_t simulation;                 ///< Flag : used for simulation hooks
    uint8_t emuMode;               ///< Flag :  used for emulation mode configuration
    uint8_t portInitAllowed;            ///< Flag : initialize ports by xpLinkManager
    uint8_t keepPortDown;            ///< Flag : keep ports down after init by xpLinkManager
    uint32_t initialPortVifVal;          ///< Initial value for port vif
    xpPcktDrvConfig *pcktDrvConfig; ///< packet driver config
    xpDmaConfigInfo *dmaConfig;         //< Configurations specific to DMA
} xpDevConfigStruct;

extern xpDevConfigStruct devDefaultConfig;
typedef struct xpDevConfigStruct xpDevConfigStruct;


/**
 *\brief This type defines an emnumeration of all device types
 */
typedef XP_DEV_TYPE_T xpsDevType_t;

/**
 *\public
 *\brief This type defines an enumeration of all xp-chip profiles
 *       supported by sdk.
 */
typedef XP_PROFILE_TYPE xpsDevProfileType_t;

/**
 *\public
 *\brief This type defines an enumeration of all the port speeds
 *       supported by sdk.
 */
typedef XP_SPEED xpsPortSpeedArr_t[XP_MAX_PHY_PORTS];

/**
 * \struct xpsDevConfig_t
 * \brief Global structure for a device config.
 */
typedef struct
{
    xpsDevProfileType_t devProfile;
    xpsPortSpeedArr_t portSpeed;
} xpsDevConfig_t;

typedef struct xpCfgTblEntryCountStruct
{
    uint32_t vlanIds;
    uint32_t lagIds;
    uint32_t lagMbrCnt;
    uint32_t mac2meIds;
    uint32_t ipv4RouteEntries;
    uint32_t ipv6RouteEntries;
    uint32_t nextHopEntries;
    uint32_t nextHopGroupEntries;
    uint32_t arpEntries;
    uint32_t fdbEntries;
    uint32_t aclTables;
    uint32_t aclTableGroups;
    uint32_t aclEntries;
    uint32_t aclCounters;

} xpCfgTblEntryCountStruct;

/**
 * \brief This API to get Cfg Table Entry Count
 * \param[in] devId
 *
 * \return uint8_t
 */
XP_STATUS xpsCpssCfgTableNumEntriesGet(xpsDevice_t devId,
                                       xpCfgTblEntryCountStruct *tblEntryCountPtr);

/**
 * \brief This API defines SDK de-initialization and cleanup routine.
 *
 * This API de-initializes SDK and frees required data structures, and
 *  managers. This is a device agnostic (global) de-initialization.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSdkDeInit();

/**
 * \brief This API defines SDK initialization routine for a scope.
 *
 * This API initializes complete SDK and required data structures, managers
 * required for a particular scope.
 *
 * \param [in] devType
 * \param [in] scopeId
 * \param [in] rpType
 * \param [in] initType
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSdkInitScope(XP_DEV_TYPE_T devType, xpsScope_t scopeId,
                          xpsRangeProfileType_t rpType, xpsInitType_t initType);

/**
 * \brief This API defines SDK de-initialization and cleanup routine for a scope.
 *
 * This API de-initializes SDK and frees required data structures, and
 *  managers for a particular scope.
 *
 * \param [in] scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSdkDeInitScope(xpsScope_t scopeId);

/**
 * \brief This API initializes a XP device identified by its device ID.
 *
 * This API will reset the device and initialize the device to a configuration
 * defined within the xpDevConfigStruct passed in as an argument. The argument
 * xpInitType_t is an enumeration of the type of initialization supported by
 * the SDK (Cold, Warm, Warm_Sync_Shadow, Warm_Sync_Hardware).
 *
 * \param [in] devId
 * \param [in] initType
 * \param [in] dalType
 * \param [in] rtype
 * \param [in] clkFreq
 * \param [in] packetInterface
 * \return XP_STATUS
 */
XP_STATUS xpsSdkDeviceInit(xpsDevice_t devId, xpsInitType_t initType,
                           XP_DAL_TYPE_T dalType, xpsRangeProfileType_t rtype, xpCoreClkFreq_t clkFreq,
                           xpPacketInterface packetInterface);

/**
 * \brief This API initializes a XP device identified by its device ID.
 *
 * This API will reset the device and initialize the device to a configuration
 * defined within the xpDevConfigStruct passed in as an argument. The argument
 * xpInitType_t is an enumeration of the type of initialization supported by
 * the SDK (Cold, Warm, Warm_Sync_Shadow, Warm_Sync_Hardware).
 *
 * \param [in] devId
 * \param [in] initType
 * \param [in] *devConfigStruct
 * \return XP_STATUS
 */
XP_STATUS xpsSdkDevInit(xpsDevice_t devId, xpsInitType_t initType,
                        xpDevConfigStruct *devConfigStruct);

/**
 * \brief This method cleanups the XDK for a graceful termination
 *        after a process restart or ISSU.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsProcessTerminationHandler();

/**
 * \brief This API loads a profile (personality).
 *
 * This API loads a personality identified by xpDevProfileType_t enumeration.

 * \param [in] devId
 * \param [in] profileType
 * \return XP_STATUS
 */
XP_STATUS xpsSdkDevLoadProfile(xpsDevice_t devId,
                               xpsDevProfileType_t profileType);

/**
 * \brief This API will enable logging on the console.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSdkLogConsole(void);

/**
 * \brief This API will disable logging on console.
 * \return XP_STATUS
 */
XP_STATUS xpsSdkDisableLogConsole(void);

/**
 * \brief This API will be used to set log level per module in xdk.
 * \param moduleName
 * \param logLevel
 * \return XP_STATUS
 */
XP_STATUS xpsSdkSetModuleLogLevel(char* moduleName, char* logLevel);

/**
 * \brief This API will be used to enable or disable logging on console per module in xdk.
 * \param[in] moduleName
 * \param[in] enable
 * \return XP_STATUS
 */
XP_STATUS xpsSdkEnableOrDisableLogModule(char* moduleName, uint8_t enable);

/**
 * \brief This API will be used to maintain thread block list
 * \param [in] moduleName
 * \param [in] threadMgmtId
 * \param [in] mask
 * \return XP_STATUS
 */
XP_STATUS xpsSdkThreadLogRegister(char* moduleName, uint32_t threadMgmtId,
                                  uint8_t mask);

/**
 * \brief This API will be used to maintain thread block list
 * \param [in] moduleName
 * \param [in] threadMgmtId
 * \param [in] mask
 * \return XP_STATUS
 */
XP_STATUS xpsSdkThreadLogStatusSet(char* moduleName, uint32_t threadMgmtId,
                                   uint8_t mask);

/**
 * \brief This API will be used to enable disable link polling thread log
 * \param [in] moduleName
 * \param [in] threadMgmtId
 * \param [out] mask
 * \return XP_STATUS
 */
XP_STATUS xpsSdkThreadLogStatusGet(char* moduleName, uint32_t threadMgmtId,
                                   uint8_t* mask);

/**
 * \brief This API begins the transaction fifo redirection.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsTransFifoRdrBegin(xpDevice_t devId);


/**
 * \brief This API ends the transaction fifo redirection.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsTransFifoRdrEnd(xpDevice_t devId);

/**
 * \brief This API set the initType.
 * \param[in] initType
 *
 * \return void
 */
void xpsSetInitType(xpsInitType_t initType);

/**
 * \brief This API get the initType.
 *
 * \return xpsInitType_t
 */
xpsInitType_t xpsGetInitType();

/**
 * \brief This API to check Device initialization
 * \param[in] devId
 *
 * \return uint8_t
 */
uint8_t xpsIsDevInitDone(xpDevice_t devId);

/**
 * \brief This API checks xpShell Server thread is avtive or not
 *
 *
 * \return uint8_t
 */
bool xpsIsXpshellthreadActive(void);

/**
 * \brief This API for api  initialization
 * \param[in] devId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsSdkDevInitApi(xpsDevice_t devId);

void xpsLinkStateNotify(void (*func)(xpsDevice_t, uint32_t, int));

void xpsResetSwitch(xpsDevice_t devId);

#ifdef __cplusplus
}
#endif

#endif  //_xpsInit_h_
