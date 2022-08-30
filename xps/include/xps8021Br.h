// xps8021Br.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \file xps8021Br.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS 802.1BR support
 * \Copyright (c) Marvell [2000-2020]. All rights reservered. Confidential.. ALL RIGHTS RESERVED.
 */


#ifndef _xpsBr_h_
#define _xpsBr_h_

#include "xpsInit.h"
#include "xpsMulticast.h"

#ifdef __cplusplus
extern "C" {
#endif



/*
 * XPS 802.1Br Public APIs
 */
/**
 * \public
 * \brief Creates group for External Port Extender
 *
 * \param [in] numOfPorts Max number or ports in group
 * \param [out] groupId Group Id
 *
 * \return XP_STATUS
 */
XP_STATUS xps8021BrCreatePortExtenderGroup(uint16_t numOfPorts,
                                           xpsPeg_t *groupId);

/*
 * XPS 802.1Br Public APIs
 */
/**
 * \public
 * \brief Creates group for External Port Extender for a given scope
 * \param [in] scopeId
 * \param [in] numOfPorts Max number or ports in group
 * \param [out] groupId Group Id
 *
 * \return XP_STATUS
 */

XP_STATUS xps8021BrCreatePortExtenderGroupScope(xpsScope_t scopeId,
                                                uint16_t numOfPorts, xpsPeg_t *groupId);

/**
 * \public
 * \brief Deletes group for External Port Extender
 *
 * \param [in] groupId Group Id
 *
 * \return XP_STATUS
 */
XP_STATUS xps8021BrDeletePortExtenderGroup(xpsPeg_t groupId);

/**
 * \public
 * \brief Deletes group for External Port Extender for a given scope.
 * \param [in] scopeId
 * \param [in] groupId Group Id
 *
 * \return XP_STATUS
 */
XP_STATUS xps8021BrDeletePortExtenderGroupScope(xpsScope_t scopeId,
                                                xpsPeg_t groupId);

/**
 * \public
 * \brief Creates Extended port in the group for a scope
 *
 * \param [in]  scopeId Scope Id
 * \param [in]  groupId Group Id
 * \param [in]  maxCascadePorts Max number of cascade ports
 * \param [out] intfId Interface Id of extended port
 *
 * \return XP_STATUS
 */
XP_STATUS xps8021BrCreateExtendedPortScope(xpsScope_t scopeId, xpsPeg_t groupId,
                                           uint32_t maxCascadePorts, xpsInterfaceId_t *intfId);

/**
 * \public
 * \brief Init Extended port on the device
 *
 * \param [in]  devId  Index of Device
 * \param [in]  groupId Group Id
 * \param [in]  extendedPort Interface Id of extended port
 *
 * \return XP_STATUS
 */
XP_STATUS xps8021BrInitExtendedPort(xpsDevice_t devId, xpsPeg_t groupId,
                                    xpsInterfaceId_t extendedPort);

/**
 * \public
 * \brief Binds Extended port to Cascade Port in a scope
 *
 * \param [in]  scopeId Scope Id
 * \param [in]  groupId Group Id
 * \param [in]  extendedPort Interface Id of extended port
 * \param [in]  cascadePort Interface Id of cascade port
 *
 * \note  If cascadePort is on device that was not used by this
 *        extendedPort before bind won't work properly without
 *        call of xps8021BrInitExtendedPort on that device first
 *
 * \return XP_STATUS
 */
XP_STATUS xps8021BrBindExtendedPortScope(xpsScope_t scopeId, xpsPeg_t groupId,
                                         xpsInterfaceId_t extendedPort, xpsInterfaceId_t cascadePort);

/**
 * \public
 * \brief Unbinds Extended port to Cascade Port in a scope
 *
 * \param [in]  scopeId Scope Id
 * \param [in]  groupId Group Id
 * \param [in]  extendedPort Interface Id of extended port
 * \param [in]  cascadePort Interface Id of cascade port
 *
 * \return XP_STATUS
 */
XP_STATUS xps8021BrUnBindExtendedPortScope(xpsScope_t scopeId, xpsPeg_t groupId,
                                           xpsInterfaceId_t extendedPort, xpsInterfaceId_t cascadePort);

/**
 * \public
 * \brief Deletes Extended port in a scope
 *
 * \param [in]  scopeId Scope Id
 * \param [in]  groupId Group Id
 * \param [in]  intfId Interface Id of extended port
 *
 * \return XP_STATUS
 */
XP_STATUS xps8021BrDeleteExtendedPortScope(xpsScope_t scopeId, xpsPeg_t groupId,
                                           xpsInterfaceId_t intfId);

/**
 * \public
 * \brief Check if provided interfaceId corresponds to correct
 *        Extended port in a scope
 *
 * \param [in]  scopeId Scope Id
 * \param [in]  groupId Group Id
 * \param [in]  intfId Interface Id of extended port
 *
 * \return XP_STATUS {XP_NO_ERR if valid, error otherwise}
 */
XP_STATUS xps8021BrIsExtendedPortValidScope(xpsScope_t scopeId,
                                            xpsPeg_t groupId, xpsInterfaceId_t intfId);

/**
 * \public
 * \brief Check if Extended port is bound to the cascade port in a scope
 *
 * \param [in]  scopeId Scope Id
 * \param [in]  extendedPort Interface Id of extended port
 * \param [in]  cascadePort Interface Id of cascade port
 *
 * \return XP_STATUS {XP_NO_ERR if bound, error otherwise}
 */
XP_STATUS xps8021BrIsCascadePortBindToExtendedPortScope(xpsScope_t scopeId,
                                                        xpsInterfaceId_t extendedPort, xpsInterfaceId_t cascadePort);

/**
 * \public
 * \brief Gets multicast interface id for a vlan in a scope
 *
 *\param [in]  scopeId Scope Id
 * \param [in]  vlan VLAN Id
 * \param [in]  encapType encapsulation type (tagged, untagged)
 * \param [out] mcIntf Interface Id of vlan multicast port
 *
 * \return XP_STATUS {XP_NO_ERR if bound, error otherwise}
 */
XP_STATUS xps8021BrGetVlanMulticastInterfaceScope(xpsScope_t scopeId,
                                                  xpsVlan_t vlan, xpL2EncapType_e encapType, xpsInterfaceId_t *mcIntf);

/**
 * \public
 * \brief Get multicast VIF used for a interface list in a scope
 *
 *\param [in]  scopeId Scope Id
 * This method returns a VIF_ID that is used for multicast
 * transmission from an interface list
 *
 * \param [in]  ifListId Id of multicast L2 list
 * \param [in]  encapType encapsulation type (tagged, untagged)
 * \param [out] mcIntf Interface Id of L2 multicast port
 *
 * \return XP_STATUS
 */
XP_STATUS xps8021BrGetL2MulticastInterfaceScope(xpsScope_t scopeId,
                                                xpsMcL2InterfaceListId_t ifListId, xpL2EncapType_e encapType,
                                                xpsInterfaceId_t *mcIntf);

/**
 * \public
 * \brief Get multicast VIF used for an L3 interface list
 *
 * This method returns a VIF_ID that is used for multicast
 * transmission from an interface list from l3 interface
 *
 *\param [in]  scopeId Scope Id
 * \param [in]  ifListId Id of multicast L3 list
 * \param [in]  l3IntfId Id of L3 interface
 * \param [in]  encapType encapsulation type (tagged, untagged)
 * \param [out] mcIntf Interface Id of L3 multicast port
 *
 * \return XP_STATUS
 */
XP_STATUS xps8021BrGetL3MulticastInterfaceScope(xpsScope_t scopeId,
                                                xpsMcL3InterfaceListId_t ifListId, xpsInterfaceId_t l3IntfId,
                                                xpL3EncapType_e encapType, xpsInterfaceId_t *mcIntf);

/**
 * \public
 * \brief Creates Extended port in the group
 *
 * \param [in]  groupId Group Id
 * \param [in]  maxCascadePorts Max number of cascade ports
 * \param [out] intfId Interface Id of extended port
 *
 * \return XP_STATUS
 */
XP_STATUS xps8021BrCreateExtendedPort(xpsPeg_t groupId,
                                      uint32_t maxCascadePorts, xpsInterfaceId_t *intfId);

/**
 * \public
 * \brief Binds Extended port to Cascade Port
 *
 * \param [in]  groupId Group Id
 * \param [in]  extendedPort Interface Id of extended port
 * \param [in]  cascadePort Interface Id of cascade port
 *
 * \note  If cascadePort is on device that was not used by this
 *        extendedPort before bind won't work properly without
 *        call of xps8021BrInitExtendedPort on that device first
 *
 * \return XP_STATUS
 */
XP_STATUS xps8021BrBindExtendedPort(xpsPeg_t groupId,
                                    xpsInterfaceId_t extendedPort, xpsInterfaceId_t cascadePort);

/**
 * \public
 * \brief Unbinds Extended port to Cascade Port
 *
 * \param [in]  groupId Group Id
 * \param [in]  extendedPort Interface Id of extended port
 * \param [in]  cascadePort Interface Id of cascade port
 *
 * \return XP_STATUS
 */
XP_STATUS xps8021BrUnBindExtendedPort(xpsPeg_t groupId,
                                      xpsInterfaceId_t extendedPort, xpsInterfaceId_t cascadePort);

/**
 * \public
 * \brief Deletes Extended port
 *
 * \param [in]  groupId Group Id
 * \param [in]  intfId Interface Id of extended port
 *
 * \return XP_STATUS
 */
XP_STATUS xps8021BrDeleteExtendedPort(xpsPeg_t groupId,
                                      xpsInterfaceId_t intfId);

/**
 * \public
 * \brief Check if provided interfaceId corresponds to correct
 *        Extended port
 *
 * \param [in]  groupId Group Id
 * \param [in]  intfId Interface Id of extended port
 *
 * \return XP_STATUS {XP_NO_ERR if valid, error otherwise}
 */
XP_STATUS xps8021BrIsExtendedPortValid(xpsPeg_t groupId,
                                       xpsInterfaceId_t intfId);

/**
 * \public
 * \brief Check if Extended port is bound to the cascade port
 *
 * \param [in]  extendedPort Interface Id of extended port
 * \param [in]  cascadePort Interface Id of cascade port
 *
 * \return XP_STATUS {XP_NO_ERR if bound, error otherwise}
 */
XP_STATUS xps8021BrIsCascadePortBindToExtendedPort(xpsInterfaceId_t
                                                   extendedPort, xpsInterfaceId_t cascadePort);

/**
 * \public
 * \brief Gets multicast interface id for a vlan
 *
 * \param [in]  vlan VLAN Id
 * \param [in]  encapType encapsulation type (tagged, untagged)
 * \param [out] mcIntf Interface Id of vlan multicast port
 *
 * \return XP_STATUS {XP_NO_ERR if bound, error otherwise}
 */
XP_STATUS xps8021BrGetVlanMulticastInterface(xpsVlan_t vlan,
                                             xpL2EncapType_e encapType, xpsInterfaceId_t *mcIntf);

/**
 * \public
 * \brief Get multicast VIF used for a interface list
 *
 * This method returns a VIF_ID that is used for multicast
 * transmission from an interface list
 *
 * \param [in]  ifListId Id of multicast L2 list
 * \param [in]  encapType encapsulation type (tagged, untagged)
 * \param [out] mcIntf Interface Id of L2 multicast port
 *
 * \return XP_STATUS
 */
XP_STATUS xps8021BrGetL2MulticastInterface(xpsMcL2InterfaceListId_t ifListId,
                                           xpL2EncapType_e encapType, xpsInterfaceId_t *mcIntf);

/**
 * \public
 * \brief Get multicast VIF used for an L3 interface list
 *
 * This method returns a VIF_ID that is used for multicast
 * transmission from an interface list from l3 interface
 *
 * \param [in]  ifListId Id of multicast L3 list
 * \param [in]  l3IntfId Id of L3 interface
 * \param [in]  encapType encapsulation type (tagged, untagged)
 * \param [out] mcIntf Interface Id of L3 multicast port
 *
 * \return XP_STATUS
 */
XP_STATUS xps8021BrGetL3MulticastInterface(xpsMcL3InterfaceListId_t ifListId,
                                           xpsInterfaceId_t l3IntfId, xpL3EncapType_e encapType, xpsInterfaceId_t *mcIntf);

#ifdef __cplusplus
}
#endif

#endif  //_xpsBr_h_

