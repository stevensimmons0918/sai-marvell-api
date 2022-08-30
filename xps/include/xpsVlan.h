// xpsVlan.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xpsVlan.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS Vlan Manager
 */

#ifndef _xpsVlan_h_
#define _xpsVlan_h_

#include "xpTypes.h"
#include "xpEnums.h"
#include "xpsEnums.h"
#include "openXpsVlan.h"
#include "xpsMulticast.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{

    XPS_PROTOCOL_IPV4_E =0,

    XPS_PROTOCOL_IPV6_E =1
} XPS_PROTOCOL_ENT;

typedef struct xpsServiceIdData
{
    xpsVlan_t vlanId;
    uint8_t baclEn;
    uint8_t raclEn;
    uint32_t baclId;
    uint32_t raclId;
} xpsServiceIdData_t;

#define XPS_VLAN_MAX_IDS ((cpssHalGetSKUMaxVlanIds(0)))

#define XPS_VLAN_MASK  0x1FFF

/**
 * \def XPS_VLAN_MAP_VLANID_TO_BD(vlanId)
 *
 * This Macro will return the BD id for a vlanId
 */
#define XPS_VLAN_MAP_VLANID_TO_BDID(vlanId) (vlanId)

/**
 * \def XPS_VLAN_MAP_BD_TO_VLANID(bd)
 *
 * This Macro will return the vlanId id for a VLAN bdId
 */
#define XPS_VLAN_MAP_BDID_TO_VLANID(bd)  (bd)


/**
 * \brief This method updates memory address in vlan ctx db
 *
 * \param [in] scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanUpdateMemoryAddressInVlanCtxDb(xpsScope_t scopeId);

/**
 * \brief This method is to get vlan flood Vif infomation
 *
 * \param [in] scopeId
 * \param [in] vlanId
 * \param [out] floodVif
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanGetVlanFloodVIF(xpsScope_t scopeId, xpsVlan_t vlanId,
                                 xpVif_t *floodVif);

/**
 * \brief This method initializes xps vlan manager with system specific
 *        references to various primitive managers and their initialization.
 *        This method also registers all necessary databases required
 *        maintain the relevant states with the state manager for a scope.
 *
 * \param [in] scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanInitScope(xpsScope_t scopeId);

/**
 * \brief This method uninitializes xps vlan manager and frees system specific
 *        references to various primitive managers and their initialization.
 *        This method also deregisters all necessary databases required
 *        maintain the relevant states with the state manager for a scope.
 *
 * \param [in] scopeId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanDeInitScope(xpsScope_t scopeId);

/**
 * \brief This method sets rehash level for Port Vlan table
 *
 * \param [in] devId
 * \param [in] numOfRehashLevels
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanSetRehashLevel(xpDevice_t devId, uint8_t numOfRehashLevels);

/**
 * \brief This method gets rehash level for Port Vlan table
 *
 * \param [in] devId
 * \param [out] numOfRehashLevels
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanGetRehashLevel(xpDevice_t devId, uint8_t* numOfRehashLevels);

/**
 * \brief This method adds device specific primitives for this feature.
 *          This method also initializes all necessary device specific
 *          databases required maintain the relevant states.
 *
 * \param [in] devId
 * \param [in] initType
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanAddDevice(xpsDevice_t devId, xpsInitType_t initType);

/**
 * \brief This method removes all device specific primitives for this feature.
 *          This method also frees all used device specific databases by
 *          this manager.
 *
 * \param [in] devId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanRemoveDevice(xpsDevice_t devId);

/**
 * \brief This method checks if a particular vlan exists
 *          in a device and returns the status.
 *
 * \param [in] vlanId
 * \param [in] devId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanIsExistOnDevice(xpsDevice_t devId, xpsVlan_t vlanId);

/**
 * \brief This method checks if the endpoint is configured
 *          in a vlan in a device.
 *
 * \param [in] vlanId
 * \param [in] intfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanIsEndpointExist(xpsDevice_t devId, xpsVlan_t vlanId,
                                 xpsInterfaceId_t intfId);

/**
 * \brief This method checks if the interface is configured
 *          in a vlan in a device.
 *
 * \param [in] vlanId
 * \param [in] intfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanIsIntfExist(xpsDevice_t devId, xpsVlan_t vlanId,
                             xpsInterfaceId_t intfId);

/**
 * \brief This method returns the default vlan instance for a device.
 *
 * \param [in] devId
 * \param [out] *vlanId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanGetDefault(xpsDevice_t devId, xpsVlan_t *vlanId);

/**
 * \brief This method creates a vlan instance on a device, and
 *        writes device specific table entries and fields with their default
 *        values.
 *
 * \param [in] devId
 * \param [in] vlanId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanCreate(xpsDevice_t devId, xpsVlan_t vlanId);

/**
 * \brief This method creates a bridge domain instance on a device, and
 *        writes device specific table entries and fields with their default
 *        values.
 *
 * \param [in] devId
 * \param [out] vlanId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanCreateBd(xpsDevice_t devId, xpsVlan_t *vlanId);

/**
 * \brief This method tells whether the given vlan is .1D bridge vlan or not
 *
 * \param [in] scopeId
 * \param [in] vlanId
 * \param [out] isVlanBd
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanIsBd(xpsScope_t scopeId, xpsVlan_t vlanId, bool *isVlanBd);

/**
 * \brief This method return the bridgePort of the given XPS PORT/LAG associated to the bridge
 *
 * \param [in] scopeId
 * \param [in] vlanId(.1D bridge)
 * \param [in] xps port/lag
 * \param [out] bridgePort
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanGetBridgePortFromInterface(xpsScope_t scopeId,
                                            xpsVlan_t vlanId, xpsInterfaceId_t intfId, xpsInterfaceId_t *bridgePort);

/**
 * \brief This method destroys a vlan instance on a device,
 *        which releases and/or writes device specific table entries.
 *
 * \param [in] devId
 * \param [in] vlanId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanDestroy(xpsDevice_t devId, xpsVlan_t vlanId);


/**
 * \brief This method will display vlans configures along
 *           with its ports and other vlan parameters
 *
 * \param [in] devId
 * \param [in] vlanId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanShow(xpsDevice_t devId, xpsVlan_t vlanId);

/**
 * \brief This method will display a vlan's detailed configurations along
 *           with its ports and other vlan parameters
 *
 * \param [in] devId
 * \param [in] vlanId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanShowDetail(xpsDevice_t devId, xpsVlan_t vlanId);


/**
 * \brief This method removes all VLANs on specified device.
 *
 * \param [in] devId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanDeleteAll(xpsDevice_t devId);

/**
 * \brief This method returns the reference to an interface list
 *          and also returns the number of interfaces.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [out] **intfList
 * \param [out] *numOfIntfs
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanGetInterfaceList(xpsDevice_t devId, xpsVlan_t vlanId,
                                  xpsInterfaceId_t **intfList, uint16_t *numOfIntfs);

/**
 * \brief This method configures the action (packet command)
 *        for ARP broadcast packets, on this bridge domain.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] bridgeMode
 *
 * \return XP_STATUS
 */

XP_STATUS xpsVlanSetIpv4McBridgeMode(xpsDevice_t devId, xpsVlan_t vlanId,
                                     xpsVlanBridgeMcMode_e bridgeMode);

/**
 * \brief This method configures the action (packet command)
 *        for ARP broadcast packets, on this bridge domain.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] bridgeMode
 *
 * \return XP_STATUS
 */

XP_STATUS xpsVlanSetIpv6McBridgeMode(xpsDevice_t devId, xpsVlan_t vlanId,
                                     xpsVlanBridgeMcMode_e bridgeMode);

/**
 * \brief This method configures the action (packet command)
 *        for ARP broadcast packets, on this bridge domain.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] unRegMcCmd
 *
 * \return XP_STATUS
 */

XP_STATUS xpsVlanSetUnregMcCmd(xpsDevice_t devId, xpsVlan_t vlanId,
                               xpsPktCmd_e unRegMcCmd);

/**
 * \brief This method returns the action (packet command)
 *        configured for ARP broadcast packets, on this
 *        bridge domain.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [out] *bridgeMode
 *
 * \return XP_STATUS
 */

XP_STATUS xpsVlanGetIpv4McBridgeMode(xpsDevice_t devId, xpsVlan_t vlanId,
                                     xpsVlanBridgeMcMode_e *bridgeMode);

/**
 * \brief This method returns the action (packet command)
 *        configured for ARP broadcast packets, on this
 *        bridge domain.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [out] *bridgeMode
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanGetIpv6McBridgeMode(xpsDevice_t devId, xpsVlan_t vlanId,
                                     xpsVlanBridgeMcMode_e *bridgeMode);

/**
 * \brief This method returns the action (packet command)
 *        configured for ARP broadcast packets, on this
 *        bridge domain.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [out] *unRegMcCmd
 *
 * \return XP_STATUS
 */

XP_STATUS xpsVlanGetUnregMcCmd(xpsDevice_t devId, xpsVlan_t vlanId,
                               xpsPktCmd_e *unRegMcCmd);

/**
 * \brief This method returns the action (packet command)
 *        configured for ARP broadcast packets, on this
 *        bridge domain.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [out] *arpBcCmd
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanGetArpBcCmd(xpsDevice_t devId, xpsVlan_t vlanId,
                             xpsPktCmd_e *arpBcCmd);

/**
 * \brief This method configures the action (packet command)
 *        for unknown unicast commands, on this bridge domain.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] unknownUcCmd
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanSetUnknownUcCmd(xpsDevice_t devId, xpsVlan_t vlanId,
                                 xpsPktCmd_e unknownUcCmd);

/**
 * \brief This method returns the status of the action
 *        (packet command) configured for unknown unicast
 *        commands, on this bridge domain.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [out] *unknownUcCmd
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanGetUnknownUcCmd(xpsDevice_t devId, xpsVlan_t vlanId,
                                 xpsPktCmd_e *unknownUcCmd);

/**
 * \brief This method configures the action (packet command)
 *        for broadcast packets, on this bridge domain.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] bcCmd
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanSetBcCmd(xpsDevice_t devId, xpsVlan_t vlanId,
                          xpsPktCmd_e bcCmd);

/**
 * \brief This method returns the status of the action
 *        (packet command) configured for broadcast packets, on
 *        this bridge domain.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [out] *bcCmd
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanGetBcCmd(xpsDevice_t devId, xpsVlan_t vlanId,
                          xpsPktCmd_e *bcCmd);

/**
 * \brief This method configures mirroring for the vlan. Enable
 *        the feature and use the sessionId.
 *
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] enable
 * \param [in] mirrorSession
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanSetMirrorToAnalyzer(xpsDevice_t devId, xpsVlan_t vlanId,
                                     uint32_t enable, uint32_t mirrorSession);

/**
 * \brief This method returns the analyzerId and mirror enable
 *          status configured for the vlan
 *
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] enable
 * \param [in] mirrorSession
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanGetMirrorAnalyzerId(xpsDevice_t devId, xpsVlan_t vlanId,
                                     uint32_t *enable, uint32_t *analyzerId);

/**
 * \brief This method configures the count mode, for an interface.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] intfId
 * \param [in] countMode
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanSetIntfCountMode(xpsDevice_t devId, xpsVlan_t vlanId,
                                  xpsInterfaceId_t intfId, xpsCountMode_e countMode);

/**
 * \brief This method returns the configured count mode for an interface.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] intfId
 * \param [out] *countMode
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanGetIntfCountMode(xpsDevice_t devId, xpsVlan_t vlanId,
                                  xpsInterfaceId_t intfId, xpsCountMode_e *countMode);

/**
 * \brief This method enables/disables hairpinning for an interface
 *          on a vlan instance.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] intfId
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanSetHairpin(xpsDevice_t devId, xpsVlan_t vlanId,
                            xpsInterfaceId_t intfId, uint32_t enable);

/**
 * \brief This method sets the already created vlan instance type
 *          as the primary vlan for a private vlan and initializes
 *          necessary datastructures and primitives required for a private
 *          vlan. This API should be called after creation of the vlan instance.
 *
 * \param [in] devId
 * \param [in] vlanId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPVlanCreatePrimary(xpsDevice_t devId, xpsVlan_t vlanId);

/**
 * \brief This method sets the vlan instance type as a normal vlan
 *           and deletes the private vlan by deleting relevant datastructures
 *           and primitives. This API must be called after deleting all secondary
 *           vlans part of the primary vlan.
 *
 * \param [in] devId
 * \param [in] secVlanId
 * \param [in] priVlanId
 * \param [in] vlanType
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPVlanDestroyPrimary(xpsDevice_t devId, xpsVlan_t vlanId);

/**
 * \brief This method sets the already created vlan instance type
 *          as the community or isolated for a private vlan and initializes
 *          necessary datastructures and primitives required for a private
 *          vlan.
 *
 * \param [in] devId
 * \param [in] secVlanId
 * \param [in] priVlanId
 * \param [in] vlanType
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPVlanCreateSecondary(xpsDevice_t devId, xpsVlan_t secVlanId,
                                  xpsVlan_t priVlanId, xpsPrivateVlanType_e vlanType);

/**
 * \brief This method sets the vlan instance type as a normal vlan
 *           and deletes its association from the private vlan by deleting
 *           relevant datastructures and primitives. This API should be
 *           called after creation of the vlan instance.
 *
 * \param [in] devId
 * \param [in] secVlanId
 * \param [in] priVlanId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPVlanDestroySecondary(xpsDevice_t devId, xpsVlan_t secVlanId,
                                   xpsVlan_t priVlanId);

/**
 * \brief This method adds an interface to a primary vlan instance.
 *
 * \param [in] devId
 * \param [in] priVlanId
 * \param [in] intfId
 * \param [in] tagType
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPVlanAddInterfacePrimary(xpsDevice_t devId, xpsVlan_t priVlanId,
                                      xpsInterfaceId_t intfId, xpsL2EncapType_e tagType);

/**
* \brief This method removes an interface from a primary vlan instance.
*
* \param [in] devId
* \param [in] priVlanId
* \param [in] intfId
*
* \return XP_STATUS
*/
XP_STATUS xpsPVlanRemoveInterfacePrimary(xpsDevice_t devId, xpsVlan_t priVlanId,
                                         xpsInterfaceId_t intfId);

/**
 * \brief This method adds an interface to a community vlan instance.
 *
 * \param [in] devId
 * \param [in] priVlanId
 * \param [in] secVlanId
 * \param [in] intfId
 * \param [in] tagType
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPVlanAddInterfaceCommunity(xpsDevice_t devId, xpsVlan_t priVlanId,
                                        xpsVlan_t secVlanId, xpsInterfaceId_t intfId, xpsL2EncapType_e tagType);

/**
 * \brief This method removes an interface from a community
 *          vlan instance.
 *
 * \param [in] devId
 * \param [in] priVlanId
 * \param [in] secVlanId
 * \param [in] intfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPVlanRemoveInterfaceCommunity(xpsDevice_t devId,
                                           xpsVlan_t priVlanId, xpsVlan_t secVlanId, xpsInterfaceId_t intfId);

/**
 * \brief This method adds an interface to an isolated vlan instance.
 *
 * \param [in] devId
 * \param [in] priVlanId
 * \param [in] secVlanId
 * \param [in] intfId
 * \param [in] tagType
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPVlanAddInterfaceIsolated(xpsDevice_t devId, xpsVlan_t priVlanId,
                                       xpsVlan_t secVlanId, xpsInterfaceId_t intfId, xpsL2EncapType_e tagType);

/**
 * \brief This method removes an interface from an isolated vlan instance.
 *
 * \param [in] devId
 * \param [in] priVlanId
 * \param [in] secVlanId
 * \param [in] intfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsPVlanRemoveInterfaceIsolated(xpsDevice_t devId,
                                          xpsVlan_t priVlanId, xpsVlan_t secVlanId, xpsInterfaceId_t intfId);

/**
 * \brief This method returns the stg index bound to
 *        this vlan (bridge domain).
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [out] *stpId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanGetStp(xpsDevice_t devId, xpsVlan_t vlanId, xpsStp_t *stpId);

/**
 * \brief This method configures per-vlan spanning tree state for a port and spanning
 *        tree state bitmask in egress filter id table entry.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] intfId
 * \param [in] stpState
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanSetStpState(xpsDevice_t devId, xpsVlan_t vlanId,
                             xpsInterfaceId_t intfId, xpsStpState_e stpState);

/**
 * \brief This method returns per-vlan spanning tree state for a port.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] intfId
 * \param [out] *stpState
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanGetStpState(xpsDevice_t devId, xpsVlan_t vlanId,
                             xpsInterfaceId_t intfId, xpsStpState_e *stpState);

/**
 * \brief Create a service instance entry with a given id
 *          and a given type.
 *
 * \param [in] devId
 * \param [in] serviceId
 * \param [in] type
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanServiceInstanceCreate(xpsDevice_t devId, uint32_t serviceId,
                                       xpsServiceInstanceType_e type, xpsServiceIdData_t *data);

/**
 * \brief Delete a service instance entry with a given id
 *          and a given type.
 *
 *
 * \param [in] devId
 * \param [in] serviceId
 * \param [in] type
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanServiceInstanceRemove(xpsDevice_t devId, uint32_t serviceId,
                                       xpsServiceInstanceType_e type);

/**
 * \brief Set the service instace configurations.
 *
 * \param [in] devId
 * \param [in] serviceId
 * \param [in] type
 * \param [in] *data
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanServiceInstanceSetConfig(xpsDevice_t devId, uint32_t serviceId,
                                          xpsServiceInstanceType_e type, xpsServiceIdData_t *data);

/**
 * \brief API returns the service instace configurations.
 *
 * \param [in] devId
 * \param [in] serviceId
 * \param [in] type
 * \param [out] *data
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanServiceInstanceGetConfig(xpsDevice_t devId, uint32_t serviceId,
                                          xpsServiceInstanceType_e type, xpsServiceIdData_t *data);

/**
 * \brief Sets global control MAC with mask.
 *
 * \param [in] devId
 * \param [in] mac
 * \param [in] mask
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanSetGlobalControlMacWithMask(xpsDevice_t devId, macAddr_t mac,
                                             macAddr_t mask);

/**
 * \brief Clears global control MAC with mask.
 *
 * \param [in] devId
 * \param [in] mac
 * \param [in] mask
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanClearGlobalControlMacWithMask(xpsDevice_t devId, macAddr_t mac,
                                               macAddr_t mask);

/**
 * \brief Set the per VLAN Control MAC. Applicable for specific VLAN
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] mac
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanSetPerVlanControlMac(xpsDevice_t devId, xpsVlan_t vlanId,
                                      macAddr_t mac);

/**
 * \brief Set the per VLAN Control MAC. Applicable for specific VLAN
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] mac
 * \param [in] macMask
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanSetPerVlanControlMacWithMask(xpsDevice_t devId,
                                              xpsVlan_t vlanId, macAddr_t mac, macAddr_t macMask);

/**
 * \brief Clear the per VLAN Control MAC. Applicable for specific VLAN
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] mac
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanClearPerVlanControlMac(xpsDevice_t devId, xpsVlan_t vlanId,
                                        macAddr_t mac);

/**
 * \brief Clear the per VLAN Control MAC. Applicable for specific VLAN
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] mac
 * \param [in] macMask
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanClearPerVlanControlMacWithMask(xpsDevice_t devId,
                                                xpsVlan_t vlanId, macAddr_t mac, macAddr_t macMask);

/**
 * \brief Set a global control mac with the user defined
 *        pipeline action bit set
 *
 *
 * \param [in] devId
 * \param [in] mac
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanSetGlobalControlMacWithAction(xpsDevice_t devId,
                                               macAddr_t mac);

/**
 * \public
 * \brief This method enables the bridge Acl
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] enable
 * \return XP_STATUS
 */
XP_STATUS xpsVlanSetBridgeAclEnable(xpsDevice_t devId, xpsVlan_t vlanId,
                                    uint8_t enable);

/**
 * \public
 * \brief This method returns the configured value of
 *          bridge Acl enable flag
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [out] *enable
 * \return XP_STATUS
 */
XP_STATUS xpsVlanGetBridgeAclEnable(xpsDevice_t devId, xpsVlan_t vlanId,
                                    uint8_t *enable);

/**
 * \public
 * \brief This method sets bridge ACL ID
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] aclId
 * \return XP_STATUS
 */
XP_STATUS xpsVlanSetBridgeAclId(xpsDevice_t devId, xpsVlan_t vlanId,
                                uint32_t aclId);

/**
 * \public
 * \brief This method returns the configured value of
 *          bridge AclId
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [out] *aclId
 * \return XP_STATUS
 */
XP_STATUS xpsVlanGetBridgeAclId(xpsDevice_t devId, xpsVlan_t vlanId,
                                uint32_t *aclId);

/**
 * \brief Pick the first VLAN id available on a device
 *
 * \param [in] devId
 * \param [out] *vlanId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanGetFirst(xpsDevice_t devId, xpsVlan_t *vlanId);

/**
 * \brief Get the next VLAN id available on a device
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [out] *vlanIdNext
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanGetNext(xpsDevice_t devId, xpsVlan_t vlanId,
                         xpsVlan_t *vlanIdNext);

/**
 * \public
 * \brief his method enables redirect to egress interface per
 *        VLAN/Interface pair
 *
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] intfId
 * \param [in] enRedirect
 * \param [in] destIntfId
 * \param [in] encapType
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanSetRedirectToInterface(xpsDevice_t devId, xpsVlan_t vlanId,
                                        xpsInterfaceId_t intfId, uint32_t enRedirect, xpsInterfaceId_t destIntfId,
                                        xpVlanEncap_t encapType);

/**
 * \brief This method returns OpenFlow processing state at this VLAN/Interface pair.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] intfId
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanGetOpenFlowEnable(xpsDevice_t devId, xpsVlan_t vlanId,
                                   xpsInterfaceId_t intfId, uint32_t *enable);

/**
 * \brief Get the current configured FDB entriess in a vlan.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] fdbLimit
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanGetFdbCount(xpsDevice_t devId, xpsVlan_t vlanId,
                             uint32_t *numFdbEntries);

/**
 * \brief Set the maximum FDB entries that can be configured on a vlan.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] fdbLimit
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanSetFdbLimit(xpsDevice_t devId, xpsVlan_t vlanId,
                             uint32_t fdbLimit);

/**
 * \brief Get the maximum possible FDB entries that can be configured
 *          for a vlan.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [out] *fdbLimit
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanGetFdbLimit(xpsDevice_t devId, xpsVlan_t vlanId,
                             uint32_t *fdbLimit);

/**
 * \public
 * \brief Enables/disable the BUM traffic policer on a Vlan.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanSetBumPolicerEn(xpsDevice_t devId, xpsVlan_t vlanId,
                                 uint32_t enable);

/**
 * \public
 * \brief Retrieves the enable status of the BUM traffic policer on a Vlan.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanGetBumPolicerEn(xpsDevice_t devId, xpsVlan_t vlanId,
                                 uint32_t *enable);

/**
 * \public
 * \brief Sets the BUM traffic policer id for a Vlan.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] policerId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanSetBumPolicerId(xpsDevice_t devId, xpsVlan_t vlanId,
                                 uint32_t policerId);

/**
 * \public
 * \brief Retrieves the BUM policer id configured on a vlan.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [out] policerId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanGetBumPolicerId(xpsDevice_t devId, xpsVlan_t vlanId,
                                 uint32_t *policerId);

/**
 *                          for QinQ (802.1ad)
 *
 * \param [in] devId
 * \param [out] tpId
 *
 * \return XP_STATUS
 */

XP_STATUS xpsVlanGetStagTpid(xpsDevice_t devId, uint16_t *tpId);

/**
 *                          for QinQ (802.1ad)
 *
 * \param [in] devId
 * \param [in] tpId
 * \param [in] dir
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanSetStagTpid(xpsDevice_t devId, uint16_t tpId,
                             xpsDirection_t dir);

/**
 * \public
 * \brief Sets the counter mode on a Vlan.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] mode
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanSetCountMode(xpsDevice_t devId, xpsVlan_t vlanId,
                              xpsVlanCountMode_e mode);

/**
 * \public
 * \brief Get the count mode of a Vlan.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [out] mode
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanGetCountMode(xpsDevice_t devId, xpsVlan_t vlanId,
                              xpsVlanCountMode_e *mode);

/**
 * \public
 * \brief Sets the egress counter Id on a Vlan.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] counterId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanSetEgressCounterId(xpsDevice_t devId, xpsVlan_t vlanId,
                                    uint32_t counterId);

/**
 * \public
 * \brief Gets the egress counter Id on a Vlan.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [out] counterId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanGetEgressCounterId(xpsDevice_t devId, xpsVlan_t vlanId,
                                    uint32_t *counterId);

/**
 * \public
 * \brief Enables/disables counting on a vlan in ingress or egress globally.
 *
 * By default, counting is enabled globally(provided that counting is enabled and counter id set per-vlan).
 *  On disable, this Api shall make the existing vlan counters to stop counting.
 * On enable, this Api shall make the vlan counters to resume only for vlans where counter is enabled and counter id is set.
 *
 * \param [in] devId
 * \param [in] dir
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanSetCountingEnable(xpsDevice_t devId, xpsDirection_t dir,
                                   uint32_t enable);

/**
 * \public
 * \brief Enables/disables counting globally for vlan interfaces.
 *
 * By default, counting is enabled globally(provided that counting is enabled and counter id set per-vlan interface).
 *  On disable, this Api shall make the existing vlan interface counters to stop counting.
 * On enable, this Api shall make the vlan interface  counters to resume only for vlan interfaces where counter is enabled and counter id is set.
 *
 * WARNING: Same hardware resource is shared between vlan interface counting and ingress tunnel termination counting. So, so disabling/enabling
 * vlan interfaces counting shall disables/enables tunnel termination counting as well.
 *
 * \param [in] devId
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanSetIntfCountingEnable(xpsDevice_t devId, uint32_t enable);

/**
 * \public
 * \brief Enables/disables vlan Bum policing globally.
 *
 * By default, vlan Bum policing is enabled globally(provided that Bum policing is enabled and policer id set per-vlan).
 *  On disable, this Api shall make the existing vlan Bum policers to stop policing.
 * On enable, this Api shall make the vlan Bum policers to resume only for vlans where Bum policing is enabled and policer id is set.
 *
 * \param [in] devId
 * \param [out] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanSetBumPolicingEnable(xpsDevice_t devId, uint32_t enable);

/**
 * \public
 * \brief Sets the Multicast L2 interface list for Broadcast/Unknown unicast bridged packets.
 *
 * This sets the L2 interface list as Broadcast/Unknown unicast flood list. The L2 interface list needs to be built
 * by xpsMulticast manager.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] listId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanSetFloodL2InterfaceList(xpsDevice_t devId, xpsVlan_t vlanId,
                                         xpsMcL2InterfaceListId_t listId);

/**
 * \public
 * \brief Gets the Multicast L2 interface list id for Broadcast/Unknown unicast bridged packets.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [out] listId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanGetFloodL2InterfaceList(xpsDevice_t devId, xpsVlan_t vlanId,
                                         xpsMcL2InterfaceListId_t *listId);

/**
 * \public
 * \brief Resets the Broadcast/Unknown unicast list to default value.
 *
 * \param [in] devId
 * \param [in] vlanId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanResetFloodL2InterfaceList(xpsDevice_t devId, xpsVlan_t vlanId);

/**
 * \public
 * \brief Sets the Multicast L2 interface list for unregistered IPv4 multicast bridged packets.
 *
 * This sets the L2 interface list as IPv4 unregistered multicast flood list. The L2 interface
 * list needs to be built by xpsMulticast manager. IPv4 unregistered multicast flood list is set
 * to broadcast/unknown unicast list by default during vlan creation.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] listId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanSetIPv4UnregMcastL2InterfaceList(xpsDevice_t devId,
                                                  xpsVlan_t vlanId, xpsMcL2InterfaceListId_t listId);

/**
 * \public
 * \brief Gets the Multicast L2 interface list id for unregistered IPv4 multicast bridged packets.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [out] listId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanGetIPv4UnregMcastL2InterfaceList(xpsDevice_t devId,
                                                  xpsVlan_t vlanId, xpsMcL2InterfaceListId_t *listId);

/**
 * \public
 * \brief Resets the unregistered IPv4 multicast list to the Broadcast/Unknown unicast list.
 *
 * \param [in] devId
 * \param [in] vlanId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanResetIPv4UnregMcastL2InterfaceList(xpsDevice_t devId,
                                                    xpsVlan_t vlanId);

/**
 * \public
 * \brief Sets the Multicast L2 interface list for unregistered IPv6 multicast bridged packets.
 *
 * This sets the L2 interface list as IPv6 unregistered multicast flood list. The L2 interface
 * list needs to be built by xpsMulticast manager. IPv6 unregistered multicast flood list is set
 * to broadcast/unknown unicast list by default during vlan creation.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] listId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanSetIPv6UnregMcastL2InterfaceList(xpsDevice_t devId,
                                                  xpsVlan_t vlanId, xpsMcL2InterfaceListId_t listId);

/**
 * \public
 * \brief Gets the Multicast L2 interface list id for unregistered IPv6 multicast bridged packets.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [out] listId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanGetIPv6UnregMcastL2InterfaceList(xpsDevice_t devId,
                                                  xpsVlan_t vlanId, xpsMcL2InterfaceListId_t *listId);

/**
 * \public
 * \brief Resets the unregistered IPv6 multicast list to the Broadcast/Unknown unicast list.
 *
 * \param [in] devId
 * \param [in] vlanId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanResetIPv6UnregMcastL2InterfaceList(xpsDevice_t devId,
                                                    xpsVlan_t vlanId);


/**
 * \fn xpsVlanGetPvid
 * \public
 * \brief Gets the  PVID for a given interface
 *
 * \param [in] xpsDevice_t devId
 * \param [in] xpsInterfaceId_t intfId
 * \param [out] xpsVlan_t *pvid
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanGetPvid(xpsDevice_t devId, xpsInterfaceId_t intfId,
                         xpsVlan_t *pvid);
/**
 * \fn xpsVlanUpdatePvid
 * \public
 * \brief Update PVID as vlanId, for portList in given interface
 *
 * \param [in] xpsDevice_t devId
 * \param [in] xpsInterfaceId_t intfId
 * \param [in] xpsVlan_t vlanId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanUpdatePvid(xpsDevice_t devId, xpsInterfaceId_t intfId,
                            xpsVlan_t vlanId);

/**
 * \brief This method initializes xps vlan manager with system specific
 *        references to various primitive managers and their initialization.
 *        This method also registers all necessary databases required
 *        maintain the relevant states with the state manager.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanInit();

/**
 * \brief This method uninitializes xps vlan manager and frees system specific
 *        references to various primitive managers and their initialization.
 *        This method also deregisters all necessary databases required
 *        maintain the relevant states with the state manager.
 *
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanDeInit();
/**
 * \brief Counts number of Vlan Objects in stateDb.
 *
 * \param [in] xpsDevId
 * \param [out] count
 *
 * \return XP_STATUS
 */

XP_STATUS xpsCountVlanObjects(xpsDevice_t xpsDevId, uint32_t *count);

/**
 * \brief Counts number of VlanMember Objects in stateDb.
 *
 * \param [in] xpsDevId
 * \param [out] count
 *
 * \return XP_STATUS
 */

XP_STATUS xpsCountVlanMemberObjects(xpsDevice_t xpsDevId, uint32_t *count);

/**
 * \brief gets vlanId's of Objects in stateDb.
 *
 * \param [in] xpsDevId
 * \param [out] vlanIdList. Memory allocation to be done by caller.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanGetVlanIdList(xpsDevice_t xpsDevId, xpsVlan_t* vlanId);
/**
 * \brief gets vlanMemberId's of Objects in stateDb.
 *
 * \param [in] xpsDevId
 * \param [out] vlanMemberIdList. Memory allocation to be done by caller.
 *
 * \return XP_STATUS
 */

XP_STATUS xpsVlanGetVlanMemberIdList(xpsDevice_t xpsDevId,
                                     uint32_t* vlanMemberId);


/**
 * \brief This method sets rehash level for Port Vlan table
 *
 * \param [in] devId
 * \param [in] numOfRehashLevels
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanSetRehashLevel(xpDevice_t devId, uint8_t numOfRehashLevels);

/**
 * \brief This method gets rehash level for Port Vlan table
 *
 * \param [in] devId
 * \param [out] numOfRehashLevels
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanGetRehashLevel(xpDevice_t devId, uint8_t* numOfRehashLevels);

/**
 * \brief This method configures the action (packet command)
 *        for IGMP packets, on this bridge domain.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] igmpCmd
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanSetIgmpCmd(xpsDevice_t devId, xpsVlan_t vlanId,
                            xpsPktCmd_e igmpCmd);

/**
 * \brief This method returns the status of the action
 *        (packet command) configured for IGMP packets, on
 *        this bridge domain.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [out] *igmpCmd
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanGetIgmpCmd(xpsDevice_t devId, xpsVlan_t vlanId,
                            xpsPktCmd_e *igmpCmd);

/**
 * \brief This method configures the action (packet command)
 *        for ICMPv6 packets, on this bridge domain.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] icmpv6Cmd
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanSetIcmpv6Cmd(xpsDevice_t devId, xpsVlan_t vlanId,
                              xpsPktCmd_e icmpv6Cmd);

/**
 * \brief This method returns the status of the action
 *        (packet command) configured for ICMPv6 packets, on
 *        this bridge domain.
 *
 * \param [in] devId
 * \param [in] vlanId
 * \param [out] *icmpv6Cmd
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanGetIcmpv6Cmd(xpsDevice_t devId, xpsVlan_t vlanId,
                              xpsPktCmd_e *icmpv6Cmd);

/**
 * \brief This method is to get vlan flood Vif infomation
 *
 * \param [in] scopeId
 * \param [in] vlanId
 * \param [out] floodVif
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanGetVlanFloodVIF(xpsScope_t scopeId, xpsVlan_t vlanId,
                                 xpVif_t *floodVif);

/**
 * \fn xpsVlanUpdatePvid
 * \public
 * \brief Update PVID as vlanId, for portList in given interface
 *
 * \param [in] xpsDevice_t devId
 * \param [in] xpsInterfaceId_t intfId
 * \param [in] xpsVlan_t vlanId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanUpdatePvid(xpsDevice_t devId, xpsInterfaceId_t intfId,
                            xpsVlan_t vlanId);

/**
 * \fn xpsVlanGetIntfTagType
 * \public
 * \brief Get VLAN tag type for specified interface
 *
 * \param [in]  vlanId
 * \param [in]  intfId
 * \param [out] *tagType
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanGetIntfTagType(xpsVlan_t vlanId, xpsInterfaceId_t intfId,
                                xpsL2EncapType_e *tagType);

/**
 * \fn xpsVlanGetIntfIndex
 * \public
 * \brief Get portVlanIvif Index for specified interface, vlan
 *
 * \param [in]  vlanId
 * \param [in]  intfId
 * \param [out] *pVlanIvifIdx
 *
 * \return XP_STATUS
 */
XP_STATUS xpsVlanGetIntfIndex(xpsDevice_t devId, xpsVlan_t vlanId,
                              xpsInterfaceId_t intfId, uint32_t *pVlanIvifIdx);

/**
 * \fn xpscustRemoveLagFromVlan
 * \public
 * \brief Remove the Lag Entry From Vlan
 *
 * \param [in] scopeId
 * \param [in] lagIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpscustRemoveLagFromVlan(xpsScope_t scopeId,
                                   xpsInterfaceId_t lagIntfId);

/**
 * \brief This method creates default Vlan with vlanId 1 on the device.
 *
 * \param [in] deviceId
 *
 * \return XP_STATUS
 */


XP_STATUS xpsVlanInitDefaultVlan(xpsDevice_t devId);



XP_STATUS xpsVlanSetRoutingEnable(xpsDevice_t devId, xpsVlan_t vlanId,
                                  XPS_PROTOCOL_ENT protocol, bool isMcast, bool enable);

XP_STATUS xpsVlanGetRoutingEnable(xpsDevice_t devId, xpsVlan_t vlan,
                                  XPS_PROTOCOL_ENT protocol, bool isMcast,
                                  bool *enable);
XP_STATUS xpsVlanSetVniInVlanDb(xpsScope_t scopeId, xpsVlan_t vlanId,
                                uint32_t vniId);
XP_STATUS xpsVlanGetVniInVlanDb(xpsScope_t scopeId, xpsVlan_t vlanId,
                                uint32_t *vniId);
#ifdef __cplusplus
}
#endif

#endif  //_xpsVlan_h_

