// xpsPort.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsPort.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS Port Manager
 */

#ifndef _xpsPort_h_
#define _xpsPort_h_

#include "xpTypes.h"
#include "xpEnums.h"
#include "xpsEnums.h"
#include "openXpsPort.h"
#include "gtEnvDep.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \def XPS_GLOBAL_START_PORT
 *
 * This value gives the start port number that can be
 * used to allocate  vif interface for Port
 */
#define XPS_GLOBAL_START_PORT xpsGetGlobalStartPort()

/**
 * \struct xpsPortIntfMap_t
 *
 * This structure maintains portNum and devId information for
 * each interface
 */
typedef struct xpsPortIntfMap
{
    uint32_t portNum;
    xpsDevice_t devId;
    xpsInterfaceId_t intfId;
} xpsPortIntfMap_t;

/**
 * \struct xpsPortCtrlVifMap_t
 *
 * This structure maintains portNum and devId information for
 * each port control interface Id.
 */
typedef struct xpsPortCtrlVifMap
{
    uint32_t portNum;
    xpsDevice_t devId;
    xpsInterfaceId_t portCtrlIntfId;
} xpsPortCtrlVifMap_t;


/**
 * \struct xpsIntfPortMap_t
 *
 * This structure maintains portNum and devId information for
 * each interface
 */
typedef struct xpsIntfPortMap
{
    xpsInterfaceId_t intfId;
    uint32_t portNum;
    xpsDevice_t devId;
} xpsIntfPortMap_t;

/*
 * Port Mgr API Prototypes
 */

/**
 * \brief Inits the Port
 *
 * Creates a new interface and stores its type information
 * inside the database
 *
 * \param [in] devId
 * \param [in] portNum
 * \param [out] * intfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPortInit(xpsDevice_t devId, xpsPort_t portNum,
                      xpsInterfaceId_t *intfId);

/**
 * \brief Get the interface id corresponding to the SCPU port
 *
 * Get the portInfo i.e interfaceId and deviceNum
 * \param [in] devId
 * \param [out] *intfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPortGetSCPUPortIntfId(xpsDevice_t devId, xpsInterfaceId_t *intfId);

/**
 * \brief Get the interface id corresponding to the loopback1
 *
 * \param [in] devId
 * \param [out] *intfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPortGetLoopback1PortIntfId(xpsDevice_t devId,
                                        xpsInterfaceId_t *intfId);

/**
 * \brief Get the interface id corresponding to the loopback0
 *
 * \param [in] devId
 * \param [out] *intfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPortGetLoopback0PortIntfId(xpsDevice_t devId,
                                        xpsInterfaceId_t *intfId);

/**
 * \brief This method checks if a particular port interface exists in a scope.
 *
 * \param [in] scopeId
 * \param [in] portIfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPortIsExistScope(xpsScope_t scopeId, xpsInterfaceId_t portIfId);

/**
 * \brief Get the Device Id and Port Number from the interface
 *        Id in a scope
 *
 * \param [in] scopeId
 * \param [in] intfId
 * \param [out] * devId
 * \param [out] * portNum
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPortGetDevAndPortNumFromIntfScope(xpsScope_t scopeId,
                                               xpsInterfaceId_t intfId, xpsDevice_t *devId, xpsPort_t *portNum);

/**
 * \brief get fieldValue in port config
 *
 * \param [in] intfId
 * \param [in] portIfId
 * \param [in] fNum
 * \param [in] fData
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPortGetField(xpsDevice_t devId, xpsInterfaceId_t portIfId,
                          xpsPortConfigFieldList_t fNum, uint32_t *fData);

/**
 * \brief Set port policinng enable/disable globally
 *
 * By default, port policing enabled globally. This is a global knob to enable/disable the existing policers.
 *
 * \param [in] devId
 * \param [in] enable
 *
 * \return XP_STATUS
*/
XP_STATUS xpsPortSetPolicingEnable(xpsDevice_t devId, xpsInterfaceId_t port,
                                   xpsPolicerType_e policerType, bool enable);


/**
 * \brief Enable mirroring on a port in a scope, given the interface id and the analyzer id.
 *
 * \param [in] scopeId
 * \param [in] portIfId
 * \param [in] analyzerId
 * \param [in] ingress
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPortEnableMirroringScope(xpsScope_t scopeId,
                                      xpsInterfaceId_t portIfId, uint32_t analyzerId,
                                      bool ingress, bool isErspan, bool enable);

/**
 * \brief Pick the first Port interface id available in a scope in the system
 *
 * \param [in] scopeId
 * \param [out] *portIfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPortGetFirstScope(xpsScope_t scopeId, xpsInterfaceId_t *portIfId);

/**
 * \brief Get the next Port interface id available in a  scope in the system
 *
 * \param [in] scopeId
 * \param [in] portIfId
 * \param [out] *portIfIdNext
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPortGetNextScope(xpsScope_t scopeId, xpsInterfaceId_t portIfId,
                              xpsInterfaceId_t *portIfIdNext);

/**
 * \public
 * \brief Init API for portDb
 *
 * This API will create and initialize the per device port
 * database, that will be used to maintain the portNum to
 * interfaceId relationship
 *
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPortDbInit(void);

/**
 * \brief De-Init API for portDb
 *
 * De-Init API for per device portDb. This API will destroy the
 * portNum to interfaceId database.
 *
 * \return XP_STATUS
 */

XP_STATUS xpsPortDbDeInit(void);

/**
 * \brief This method checks if a particular port interface exists.
 *
 * \param [in] portIfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPortIsExist(xpsInterfaceId_t portIfId);

/**
 * \brief Enable mirroring on a port, given the interface id and the analyzer id.
 *
 * \param [in] portIfId
 * \param [in] analyzerId
 * \param [in] ingress
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPortEnableMirroring(xpsInterfaceId_t portIfId,
                                 uint32_t analyzerId, bool ingress, bool isErspan,
                                 bool enable);

/**
 * \brief Pick the first Port interface id available in the system
 *
 * \param [out] *portIfId
 *
 * \return XP_STATUS
 */

XP_STATUS xpsPortGetFirst(xpsInterfaceId_t *portIfId);

/**
 * \brief Get the next Port interface id available  in the system
 *
 * \param [in] portIfId
 * \param [out] *portIfIdNext
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPortGetNext(xpsInterfaceId_t portIfId,
                         xpsInterfaceId_t *portIfIdNext);

/**
  \brief Enable/Disable randomness to the input of the has
 * generator on a per-source port basis.
 *
 * This is done by replacing the lower 10b of the original hash
 * key with 10b from of packet Timestamp we now have a unique
 * key for every packet even if the packets are identical
 *
 * \param [in] devId   device id
 * \param [in] port    source Port
 * \param [in] enable  1 b value for enable/disable: 1 - Enable
 *
 * \return XP_N0_ERR if there is no error
 */
XP_STATUS xpsPortSetRandomHash(xpsDevice_t devId, xpsPort_t port,
                               uint32_t enable);

/**
 * \brief if sounce Port random hashing is enabled for both A
 * and B, enabled is set to 1, else it is set to 0.
 *
 * \param [in] devId   device id
 * \param [in] port    source Port
 * \param [out] enabled  1 - Enable, 0 - disable
 *
 * \return XP_N0_ERR if there is no error
 */
XP_STATUS xpsPortGetRandomHash(xpsDevice_t devId, xpsPort_t port,
                               uint32_t* enable);

/**
 * \brief Static hashes can be generated by replacing outgoing
 *        hashes (lagA, lagB, l2EcmpA, l2EcmpB, ?) with the
 *        packet source port.
 *
 * \param [in] devId  device Id
 * \param [in] port   port
 * \param [in] hash   LAG/L2ECMP/L3ECMP
 * \param [in] enable 1: enable, 0 - disable
 *
 * \return XP_NO_ERR if the write was successful
 */
XP_STATUS xpsPortSetStaticHash(xpsDevice_t devId, xpsPort_t port,
                               xpHashType_e hashType, uint32_t enable);

/**
 * \brief Read value; if sounce Port static hashing is enabled
 * for both A and B, enabled is set to 1, else it is set to 0.
 *
 * \param [in] devId   device id
 * \param [in] port    source Port
 * \param [in] hash   LAG/L2ECMP/L3ECMP
 * \param [out] enabled  1 - Enable, 0 - disable
 *
 * \return XP_N0_ERR if there is no error
 */
XP_STATUS xpsPortGetStaticHash(xpsDevice_t devId, xpsPort_t port,
                               xpHashType_e hashType, uint32_t* enable);


/**
 * \brief to enable/disable etag keep for a port
 *
 * \param [in] devId   device id
 * \param [in] port    port
 * \param [in] enable  1 - Enable, 0 - disable
 *
 * \return XP_N0_ERR if there is no error
 */
XP_STATUS xpsPortKeepEtag(xpsDevice_t devId, xpsInterfaceId_t portIfId,
                          uint8_t enable);

/**
 * \fn xpsPortAddEtag
 * \brief To add instruction to add E-tag on port
 *
 * \param [in] xpDevice_t devId
 * \param [in] xpPort_t portNum
 * \param [in] unit8_t enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPortAddEtag(xpsDevice_t devId, xpsInterfaceId_t portIfId,
                         uint8_t enable);

/**
 * \fn xpsPortEnableLongEtagMode
 * \brief To add instruction to enable long E-tag on port
 *
 * \param [in] xpDevice_t devId
 * \param [in] xpPort_t portNum
 * \param [in] unit8_t enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPortEnableLongEtagMode(xpsDevice_t devId,
                                    xpsInterfaceId_t portIfId, uint8_t enable);

XP_STATUS xpsPortGetPortIntfCount(xpsScope_t scopeId, uint32_t *count);

/**
 * \brief Get device maximum port number
 *
 * \param [in]  devId       device id
 * \param [out] maxPortNum  maximum port number
 *
 * \return XP_N0_ERR if there is no error
 */
XP_STATUS xpsPortGetMaxNum(xpsDevice_t devId, uint32_t *maxPortNum);

/**
 * \brief This method configures the action to be taken when saMac miss happens
 *
 * \param [in] devId        device id
 * \param [in] portId       bridgePortId
 * \param [in] saMissCmd    xpsPktCmd_e
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPortBridgePortSetMacSaMissCmd(xpsDevice_t devId,
                                           xpsInterfaceId_t portId, xpsPktCmd_e saMissCmd);

/**
 * \brief Dump port info
 *
 * \param [in] devId    device id
 * \param [in] portNum  port number
 * \param [in] detail
 * \return XP_N0_ERR if there is no error
 */
XP_STATUS xpsPortDump(xpsDevice_t devId, xpsInterfaceId_t portIfId,
                      bool detail);

/**
 * \brief Get Lane Swap Info
 *
 * \param [in] devId    device id
 * \param [in] portNum  port number
 * \param [in] index
 * \param [out] channelSwapNum
 * \return XP_N0_ERR if there is no error
 */
XP_STATUS xpsLinkManagerPlatformGetSerdesLaneSwapInfo(xpsDevice_t devId,
                                                      uint32_t portNum, uint32_t index, uint32_t* channelSwapNum);

/**
 * \brief This method returns Start Port number
 */

uint32_t xpsGetGlobalStartPort(void);

/**
 * \brief Get per device maximum port number
 *
 * \param [in]  devId       device id
 * \param [out] maxPortNum  maximum port number per device
 *
 * \return XP_N0_ERR if there is no error
 */
XP_STATUS xpsPortGetPerDeviceMaxNum(xpsDevice_t devId, uint8_t *maxPortNum);

/**
 * \brief Set Port Src Mac first 40 bits
 *
 * \param [in]  devId       device id
 * \param [in]  mac         Src MAC (40 bits)
 *
 * \return XP_N0_ERR if there is no error
 */

XP_STATUS xpsPortSAMacBaseSet(xpsDevice_t devId, macAddr_t mac);

/**
 * \brief Set Port Src Mac LSB.
 *
 * \param [in]  devId       device id
 * \param [in]  macLsb      src MAC LSB
 *
 * \return XP_N0_ERR if there is no error
 */

XP_STATUS xpsPortSAMacLsbSet(xpsDevice_t devId, uint32_t portNum,
                             uint8_t macLsb);

/**
 * \brief to set fdb learn mode for a port
 *
 * \param [in] devId   device id
 * \param [in] port    port
 * \param [in] mode    mode
 *
 * \return XP_N0_ERR if there is no error
 */
XP_STATUS xpsPortSetFdbLearnMode(GT_U8 devId, GT_U32 port,
                                 uint32_t learnMode);

/**
 * \brief to get fdb learn mode for a port
 *
 * \param [in] devId   device id
 * \param [in] port    port
 * \param [in] mode    mode
 *
 * \return XP_N0_ERR if there is no error
 */
XP_STATUS xpsPortGetFdbLearnMode(GT_U8 devId, GT_U32 port,
                                 uint32_t *learnMode);


XP_STATUS xpsPortUpdateTunnelMtu(xpsDevice_t devId, uint32_t portNum);
void xpsPortFecParamSet(xpsDevice_t devId, xpsPort_t port, uint32_t fecMode);

#ifdef __cplusplus
}
#endif

#endif //_xpsPort_h_
