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
 *   FILE : openXpsL3.h
 *
 *   Abstract: This file defines the apis for L3 Management in OpenXPS.
 ************************************************************************/
/**
 * \file openXpsL3.h
 * \brief This file contains API prototypes and type definitions
 *        for the openXps L3 Management
 * \Copyright (c) Marvell [2000-2020]
 */

#ifndef _openXpsL3_h_
#define _openXpsL3_h_

#include "xpTypes.h"
#include "openXpsTypes.h"
#include "openXpsEnums.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \struct xpsL3NextHopData_t
 * \brief This structure defines the L3
 *        Next-Hop data
 */
typedef struct xpsL3NextHopData_t
{
    xpsInterfaceId_t l3InterfaceId;
    macAddr_t macDa;
    xpsInterfaceId_t egressIntfId;
} xpsL3NextHopData_t;

/**
 * \struct xpsL3NextHopEntry_t
 * \brief This structure defines the L3
 *        Next-Hop entry
 */
typedef struct xpsL3NextHopEntry_t
{
    xpPktCmd_e pktCmd;          ///< Packet forwarding processing command
    uint32_t serviceInstId;     ///< A Service Instance ID to represent VNI/TNI/ISID for tunnelled packets
    uint32_t vpnLabel;          ///< VPN Label
    uint32_t propTTL;           ///< TTL
    xpsL3NextHopData_t nextHop; ///< Next-Hop Data
    uint32_t reasonCode;        ///< Reason Code
    uint32_t eVlan;             ///< For L3Tunnel Purposes
} xpsL3NextHopEntry_t;

/**
 * \struct xpsL3HostEntry_t
 * \brief This structure defines the L3
 *        host entry
 */
typedef struct xpsL3HostEntry_t
{
    uint32_t vrfId;              ///< VRF
    xpIpPrefixType_t type;       ///< IP Prefix type
    ipv4Addr_t ipv4Addr;         ///< IPv4
    ipv6Addr_t ipv6Addr;         ///< IPv6
    xpsL3NextHopEntry_t nhEntry; ///< Next-Hop Entry
} xpsL3HostEntry_t;

/**
 * \struct xpsL3RouteEntry_t
 * \brief This structure defines the L3
 *        routing entry
 */
typedef struct xpsL3RouteEntry_t
{
    uint32_t vrfId;              ///< VRF
    xpIpPrefixType_t type;       ///< IP Prefix type
    ipv4Addr_t ipv4Addr;         ///< IPv4
    ipv6Addr_t ipv6Addr;         ///< IPv6
    uint32_t   ipMaskLen;        ///< IP Mask
    uint32_t   nhEcmpSize;       ///< Next-Hop size
    uint32_t   nhId;             ///< Next-Hop ID
} xpsL3RouteEntry_t;

typedef struct
{
    //key
    uint32_t   swNhId;             ///< Next-Hop ID used in software
    //data
    uint32_t   hwNhId;             ///< Next-Hop ID from hw nh table
    xpsL3NextHopEntry_t nhEntry; ///< Next-Hop Entry
    uint32_t hwRefCnt; //this indicates the count used by the route entries
    uint32_t   hwPbrLeafId;             ///< PBR Leaf ID from hw LPM table
} xpsL3NhSwDbEntry_t;

/**
 * \brief This method Add Global Router Mac.
 *
 * \param [in] devId Device ID
 * \param [in] mac Mac Address
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3AddIngressRouterMac(xpsDevice_t devId, macAddr_t mac);

/**
 * \brief This method Add Global Router Mac for a given VLAN.
 *
 * \param [in] devId Device ID
 * \param [in] vlan VLAN-ID
 * \param [in] mac Mac Address
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3AddIngressRouterVlanMac(xpsDevice_t devId, xpsVlan_t vlan,
                                       macAddr_t mac);

/**
 * \brief Set constant 40 MSB for a device Router Egress MAC.
 *
 * \param [in] devId Device ID
 * \param [in] macHi 40 MSB bits of the Egress Router MAC
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3SetEgressRouterMacMSbs(xpsDevice_t devId,  macAddrHigh_t macHi);

/**
 * \brief Set constant 8 LSB for a device Router Egress MAC per L3 Interface.
 *
 * \param [in] devId Device ID
 * \param [in] l3IntfId Layer 3 Interface ID
 * \param [in] macSa Mac Source Address
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3SetIntfEgressRouterMacLSB(xpsDevice_t devId,
                                         xpsInterfaceId_t l3IntfId, macAddrLow_t macSa);

/**
 * \brief Set Egress MAC configuration for the l3 interface.
 *
 * \param [in] devId Device ID
 * \param [in] l3IntfId Either vlan, port or lag based L3 interface
 * \param [in] egressMac Egress mac address
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3SetEgressRouterMac(xpsDevice_t devID, xpsInterfaceId_t l3IntfId,
                                  macAddr_t egressMac);

/**
 * \brief Remove Egress MAC configuration for the l3 interface.
 *
 * \param [in] devId Device ID
 * \param [in] l3IntfId Either vlan, port or lag based L3 interface
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3RemoveEgressRouterMac(xpsDevice_t devID,
                                     xpsInterfaceId_t l3IntfId);

/**
 * \brief Configures the VRF index for routing
 *        interface created on a specific L3 Interface.
 *
 * \param [in] devId
 * \param [in] l3IntfId
 * \param [in] vrfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3SetIntfVrf(xpsDevice_t devId, xpsInterfaceId_t l3IntfId,
                          uint32_t vrfId);

/**
 * \brief Enable IPv4 unicast routing lookups
 *        for packets ingressing on a specific L3 Interface.
 *
 * \param [in] devId
 * \param [in] l3IntfId
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3SetIntfIpv4UcRoutingEn(xpsDevice_t devId,
                                      xpsInterfaceId_t l3IntfId, uint32_t enable);

/**
 * \brief Enable IPv6 unicast routing lookups
 *        for packets ingressing on a specific L3 Interface.
 *
 * \param [in] devId
 * \param [in] l3IntfId
 * \param [in] enable
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3SetIntfIpv6UcRoutingEn(xpsDevice_t devId,
                                      xpsInterfaceId_t l3IntfId, uint32_t enable);


/**
 * \brief Add a L3 Host Entry
 *
 * \param [in] devId Device Id of device.
 * \param [in] pL3HostEntry Pointer to L3 Host entry structure.
 * \param [out] indexList Index at which the entry is written to hardware is at indexList->index[0]. Indices of entry which are rehashed is present at indexList->index[1], indexList->index[2]... so on
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3AddIpHostEntry(xpsDevice_t devId, xpsL3HostEntry_t *pL3HostEntry,
                              xpsHashIndexList_t *indexList);

/**
 * \brief This method is used for classifying IP host (myIP)
 *        control packets and assigning a unique reason code for
 *        control plane policing (CoPP). It internally adds a L3
 *        host entry. Reason Code is overloaded on the egressVif
 *        in the host entry.
 *
 * \param [in] devId Device Id of device.
 * \param [in] pL3HostEntry Pointer to L3 Host entry structure.
 * \param [out] indexList Index at which the entry is written to hardware is at indexList->index[0]. Indices of entry which are rehashed is present at indexList->index[1], indexList->index[2]... so on
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3AddIpHostControlEntry(xpsDevice_t devId,
                                     xpsL3HostEntry_t *pL3HostEntry, xpsHashIndexList_t *indexList);

/**
 * \brief This method Updates the L3 Host Entry at the provided index
 *
 * \param [in] devId
 * \param [in] index
 * \param [in] *pL3HostEntry
 *
 * \return XP_STATUS
 */

XP_STATUS xpsL3UpdateIpHostEntryByIndex(xpsDevice_t devId, uint32_t index,
                                        xpsL3HostEntry_t *pL3HostEntry);

/**
 * \brief Remove the L3 Host Entry at the specific index
 *
 * \param [in] devId Device Id of device.
 * \param [in] index Index of the Host entry to be removed from hardware.
 * \param [in] pL3PrefixType Prefix Type
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3RemoveIpHostEntryByIndex(xpsDevice_t devId, uint32_t index,
                                        xpIpPrefixType_t pL3PrefixType);

/**
 * \brief Add L3 Route Entry by SubTrieMode
 *
 * \param [in] devId Device Id of device.
 * \param [in] *pL3RouteEntry Pointer to L3 Route entry structure.
 * \param [out] *prfxBucketIdx Index at which the L3 Route entry is written to hardware.
 * \param [in] subTrieMode type of route subTrieMode
 * \return XP_STATUS
 */
XP_STATUS xpsL3AddIpRouteEntryBySubTrieMode(xpsDevice_t devId,
                                            xpsL3RouteEntry_t *pL3RouteEntry, uint32_t *prfxBucketIdx,
                                            xpLpmSubTrieMode subTrieMode);

/**
 * \brief Allocate Next-Hop for given ECMP Size
 *
 * \param [out] nhId Next-Hop ID
 * \param [in] nhEcmpSize ECMP Group size
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3CreateRouteNextHop(uint32_t nhEcmpSize, uint32_t *nhId);

/**
 * \brief Set the NextHop Entry for a specific Next-Hop ID and The linked groups
 *
 * \param [in] devId Device Id of device.
 * \param [in] nhId Next-Hop Id of device.
 * \param [in] *pL3NextHopEntry Pointer to L3 Next-Hop entry structure.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3SetRouteNextHopAndNextHopGroup(xpsDevice_t devId, uint32_t nhId,
                                              xpsL3NextHopEntry_t *pL3NextHopEntry);

/**
 * \brief Set the NextHop Entry for a specific Next-Hop ID
 *
 * \param [in] devId Device Id of device.
 * \param [in] nhId Next-Hop Id of device.
 * \param [in] *pL3NextHopEntry Pointer to L3 Next-Hop entry structure.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3SetRouteNextHop(xpsDevice_t devId, uint32_t nhId,
                               xpsL3NextHopEntry_t *pL3NextHopEntry);

XP_STATUS xpsL3SetRouteNextHopDb(xpsDevice_t devId, uint32_t nhId,
                                 xpsL3NextHopEntry_t *pL3NextHopEntry);
/**
 * \brief Clear the NextHop Entry for given Next-Hop ID in a device.
 *
 * \param [in] devId Device Id of device.
 * \param [in] nhId Next-Hop Id of device.
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3ClearRouteNextHop(xpsDevice_t devId, uint32_t nhId);

/**
 * \brief Free the allocated Next-Hop
 *
 * \param [in] nhId Next-Hop Id of device
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3DestroyRouteNextHop(uint32_t nhEcmpSize, uint32_t nhId);

/**
 * \brief Create L3 Interface over a specific VLAN.
 *
 * \param [in] vlanId VLAN-ID
 * \param [out] l3IntfId Pointer to the allocated L3 Interface ID
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3CreateVlanIntf(xpsVlan_t vlanId, xpsInterfaceId_t *l3IntfId);

/**
 * \brief Removes L3 Interface from a specific VLAN.
 *
 * \param [in] vlanId
 * \param [in] l3IntfId L3 Interface ID
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3DestroyVlanIntf(xpsVlan_t vlanId, xpsInterfaceId_t l3IntfId);

/**
 * \brief Set hashable fields for a device
 * \deprecated use \ref xpsGlobalSwitchControlSetHashFields
 * \param [in] deviceId   Device id
 * \param [in] fields  array of fields
 * \param [in] size
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3SetHashFields(xpsDevice_t deviceId, xpHashField* fields,
                             size_t size);

/**
 * \brief This method Creates an L3 sub interface.
 *
 * \param [out] *l3IntfId L3 Interface ID
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3CreateSubIntf(xpsInterfaceId_t *l3IntfId);

/**
 * \brief This method destroys L3 sub interface.
 *
 * \param [in] l3IntfId L3 Interface ID
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3DestroySubIntf(xpsInterfaceId_t l3IntfId);

/**
 * \brief This method binds L3 sub interface with the port interface
 *         over which it is created.
 *
 * \param [in] portIntfId Port Interface ID
 * \param [in] l3IntfId L3 Interface ID
 * \param [in] encapId VLAN-ID
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3BindSubIntf(xpsInterfaceId_t portIntfId,
                           xpsInterfaceId_t l3IntfId, xpsVlan_t encapId);

/**
 * \brief This method detaches L3 sub interface from the port interface
 *         over which it is created.
 *
 * \param [in] portIntfId Port Interface ID
 * \param [in] l3IntfId L3 Interface ID
 * \param [in] encapId Vlan-ID
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3UnBindSubIntf(xpsInterfaceId_t portIntfId,
                             xpsInterfaceId_t l3IntfId, xpsVlan_t encapId);

/**
 * \brief This method Initializes L3 sub interface.
 *
 * \param [in] devId Device Id of device.
 * \param [in] l3IntfId L3 Interface ID
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3InitSubIntf(xpsDevice_t devId, xpsInterfaceId_t l3IntfId);

/**
 * \brief This method De-Initializes L3 sub interface.
 *
 * \param [in] devId Device Id of device.
 * \param [in] l3IntfId L3 Interface ID
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3DeInitSubIntf(xpsDevice_t devId, xpsInterfaceId_t l3IntfId);

/**
 * \brief This method Creates L3 Interface Over Port.
 *
 * \param [in] tnnlIntfId
 * \param [out] l3IntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3CreatePortIntf(xpsInterfaceId_t *l3IntfId);

/**
 * \brief This method Removes L3 Interface Over Port.
 *
 * \param [out] l3IntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3DestroyPortIntf(xpsInterfaceId_t l3IntfId);

/**
 * \brief This method Initializes L3 Interface Over Port.
 *
 * \param [in] portIntfId
 * \param [in] l3IntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3BindPortIntf(xpsInterfaceId_t portIntfId,
                            xpsInterfaceId_t l3IntfId);

/**
 * \brief This method De-Initializes L3 Interface Over Port.
 *
 * \param [in] portIntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3UnBindPortIntf(xpsInterfaceId_t portIntfId);

/**
 * \brief This method Initializes L3 Interface Over Port.
 *
 * \param [in] devId
 * \param [in] l3IntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3InitPortIntf(xpsDevice_t devId, xpsInterfaceId_t l3IntfId);

/**
 * \brief This method De-Initializes L3 Interface Over Port.
 *
 * \param [in] devId
 * \param [in] l3IntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3DeInitPortIntf(xpsDevice_t devId, xpsInterfaceId_t l3IntfId);

/**
 * \brief This method sets rehash level for IPv4 Host table
 *
 * \param [in] devId
 * \param [in] numOfRehashLevels
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3SetIpv4RehashLevel(xpDevice_t devId, uint8_t numOfRehashLevels);

/**
 * \brief This method sets rehash level for IPv6 Host table
 *
 * \param [in] devId
 * \param [in] numOfRehashLevels
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3SetIpv6RehashLevel(xpDevice_t devId, uint8_t numOfRehashLevels);

/**
 * Sets the number of rehash level for the route table.
 *
 * When rehashing happens while adding an route entry, this configuration is used to
 * determine the maximum level of recursions that can be performed.
 *
 * \param [in] devId
 * \param [in] prefixType Prefix Type
 * \param [in] numRehashLevel
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3SetNumRehashLevel(xpsDevice_t devId,
                                 xpsIpPrefixType_e prefixType, uint32_t numRehashLevel);

XP_STATUS xpsL3InitApi(xpsDevice_t devId);

#ifdef __cplusplus
}
#endif

#endif  //_openXpsL3_h_




