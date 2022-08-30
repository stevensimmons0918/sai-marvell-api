// xpsMulticast.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
 * \file xpsMulticast.h
 * \brief This file contains API prototypes and type definitions
 *        for the XPS Multicast Manager
 * \Copyright (c) Marvell [2000-2020]. All rights reservered. Confidential.. ALL RIGHTS RESERVED
 */

#ifndef _xpsMulticast_h_
#define _xpsMulticast_h_

#include "xpTypes.h"
#include "xpsInterface.h"
#include "cpss/dxCh/dxChxGen/l2mll/cpssDxChL2Mll.h"

#define XPS_MCAST_INVALID_MVIF_IDX (-1)
#define XPS_MCAST_INVALID_MDT_ROOT_IDX (-1)
#define XPS_MCAST_INVALID_MDT_NEXT_IDX (0)
#define XPS_MCAST_INVALID_DOMAIN_ID (-1)

#define XPS_MLL_PAIR_RANGE_START 1 // Reserve 0 for assigning default behavior.
#define XPS_MLL_VIDX_RANGE_START 1 // Like 4095, 0 can be reserved to assign any default behavior 
#define XPS_MLL_EVIDX_RANGE_START 4096 // Range 4096 to 16K-1 

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*multicastAgingHandler)(xpDevice_t, uint32_t *,
                                      void *);      ///< multicastAgingHandler


/**
 * \brief This Enum type represents Packet Commands Values
 *        supported by the multicast manager for IPv4/v6 Bridge/Route
 *        Entries.
 */
typedef enum
{
    XP_MC_PKTCMD_FWD = XP_PKTCMD_FWD,     /// FWD PACKET
    XP_MC_PKTCMD_DROP = XP_PKTCMD_DROP,   /// DROP PACKET
    XP_MC_PKTCMD_TRAP = XP_PKTCMD_TRAP,
    XP_MC_PKTCMD_MAX
} xpMulticastPktCmdType_e;

/**
 * \brief This Enum type represents RPF Check Types
 *  supported by the multicast manager for IPv4/v6 Route
 *  Entries.
 */
typedef enum
{
    XP_MC_RPF_CHECK_TYPE_PORT,      ///RPF CHECK TYPE PORT
    XP_MC_RPF_CHECK_TYPE_BD,        ///RPF CHECK TYPE BRDIGE DOMAIN
    XP_MC_RPF_CHECK_TYPE_VIF,       ///RPF CHECK TYPE VIF
    XP_MC_RPF_CHECK_TYPE_PIMBIDIR,  ///RPF CHECK TYPE PIMBIDIR
    XP_MC_RPF_CHECK_TYPE_MAX
} xpMulticastRpfCheckType_e;

/**
 * \brief This Enum type represents RPF Fail Command Values
 *        supported by the multicast manager for IPv4/v6 Route
 *        Entries.
 */
typedef enum
{
    XP_MC_RPF_FAIL_CMD_DEFER_TO_EGRESS = XP_PKTCMD_FWD,   ///RPF CHECK DEFER PACKET TO EGRESS COMMAND
    XP_MC_RPF_FAIL_CMD_TRAP = XP_PKTCMD_TRAP,             ///RPF CHECK TRAP PACKET COMMAND
    XP_MC_RPF_FAIL_CMD_MAX
} xpMulticastRpfFailCmd_e;


/**
 * \brief This structure defines the VIF for multicast
 *        purposes.
 */
typedef struct _xpMulticastVifEntry_
{
    uint32_t
    mdtRootIdx;   ///< Root MDT Index which points to head of the Multicast Node chain.
    //TODO - Need to fix this, its defined as a c++ Vector.
    //    xpPortList_t *                       portList;     ///< List of Ports for Packe Replication
} xpMulticastVifEntry_t;

/**
 * \brief This structure defines the IPv4 multicast Bridge entry.
 */
typedef struct _xpIPv4MulticastBridgeEntry_
{
    uint32_t        bdId;                /// Bridge domain-ID or Instance Id
    uint32_t        vlanId;      /// VLAN Id
    ipv4Addr_t    sourceAddress;         /// Source IP Address
    ipv4Addr_t    groupAddress;          /// Group IP Address

    uint32_t
    multicastVifIdx;         /// VifIdx or xpsMcL2InterfaceListId_t for XPS
    uint32_t    mirrorMask;              /// Mirror Mask
    uint32_t    countMode;               /// Counter Mode
    uint32_t    counterIdx;              /// Counter Index
    uint32_t
    isControl;               /// Is the Mcast Address a Control Mac Address
    uint32_t    isStatic;                /// Is this a Static Entry
    xpMulticastPktCmdType_e    pktCmd;   /// Packet Command
} xpIPv4MulticastBridgeEntry_t;

/**
 * \brief This structure defines the IPv6 multicast Bridge
 *  entry.
 */
typedef struct _xpIPv6MulticastBridgeEntry_
{
    uint32_t        bdId;            ///< VLAN-ID or Instance Id
    uint32_t        vlanId;              ///< VLAN Id
    xpMcastDomainId_t ipv6McL2DomainId;  ///< 7 Bit Id mapping to a Vlan or BD Id
    ipv6Addr_t    sourceAddress;     ///< Source IPV6 Address
    ipv6Addr_t    groupAddress;      ///< Group IPV6 Address

    uint32_t
    multicastVifIdx;     ///< VifIndex or XPSMCL2InterfaceListId_t for XPS
    uint32_t    mirrorMask;          ///< Mirror Mask
    uint32_t    countMode;           ///< Counter Mode
    uint32_t    counterIdx;          ///< Counter Index
    uint32_t    isControl;           ///< Is thie a Control Mac Address
    uint32_t    isStatic;            ///< Is this a Statis Entry
    xpMulticastPktCmdType_e    pktCmd; ///< Packet Command
} xpIPv6MulticastBridgeEntry_t;

/**
 * \brief This structure defines the IPv4 multicast Route entry.
 */
typedef struct _xpIPv4MulticastRouteEntry_
{
    uint32_t        vrfIdx;          ///< Vrf Index or Instance Id
    ipv4Addr_t    sourceAddress;     ///< IPV4 Source IP Address
    ipv4Addr_t    groupAddress;      ///< IPV4 Multicast Group Address

    uint32_t
    multicastVifIdx;     ///< VifIndex or XPSMCL3InterfaceListId_t for XPS
    uint32_t    mirrorMask;          ///< Mirror Mask
    uint32_t    countMode;           ///< Count Mode
    uint32_t    rpfValue;            ///< Rpf Value

    xpMulticastPktCmdType_e    pktCmd;   ///< Packet Command
    xpMulticastRpfFailCmd_e      rpfFailCmd; ///< Rpf Fail Command
    xpMulticastRpfCheckType_e  rpfType;      ///< Rpf Type
} xpIPv4MulticastRouteEntry_t;

/**
 * \brief This structure defines the IPv6 multicast Route
 *        entry.
 */
typedef struct _xpIPv6MulticastRouteEntry_
{
    uint32_t        vrfIdx;                      ///< Vrf Infdex or Instance Id
    xpMcastDomainId_t
    ipv6McL3DomainId;          ///< 7 bit Mcast Domain Id mapping to a Vrf or Instance Id
    ipv6Addr_t    sourceAddress;                 ///< IPV6 Source Ip Address
    ipv6Addr_t    groupAddress;                  ///< Ipv6 Multicast Group Address

    uint32_t
    multicastVifIdx;                 ///< Vif Index or XPSMCL3InterfaceListId-t for XPS
    uint32_t    mirrorMask;                      ///< Mirror Mask
    uint32_t    countMode;                       ///< Count Mode
    uint32_t    rpfValue;                        ///< Rpf Value

    xpMulticastPktCmdType_e    pktCmd;           ///< Packet Command
    xpMulticastRpfFailCmd_e      rpfFailCmd;     ///< Rpf Fail Command
    xpMulticastRpfCheckType_e  rpfType;          ///< Rpf Type
} xpIPv6MulticastRouteEntry_t;

/**
 * \brief This structure defines the IPv4 multicast PIM Bidir
 *        RPF entry.
 */
typedef struct _xpIPv4MulticastPimBidirRpfEntry_
{
    uint32_t        vrfIdx;           ///< Vrf Index
    uint32_t        bdId;             ///< Bridge Domain Id / IP interface Id
    ipv4Addr_t    groupAddress;       ///< Ipv4 Multicast Group Address

    uint32_t    rpfValue;            ///< Rpf Value
} xpIPv4MulticastPimBidirRpfEntry_t;

/**
 * \brief This structure defines the IPv6 multicast PIM Bidir
 *        RPF entry.
 */
typedef struct _xpIPv6MulticastPimBidirRpfEntry_
{
    uint32_t        vrfIdx;         ///< VRF Index
    uint32_t        bdId;           ///< Bridge Domain Id
    ipv6Addr_t    groupAddress;     ///< Ipv6 Multicast Group Address

    uint32_t    rpfValue;           ///< Rpf Value
} xpIPv6MulticastPimBidirRpfEntry_t;
/**
 * \typedef xpsMcL2InterfaceListId_t
 * \brief This type defines the L2 Multicast (Snooping)
 *        Interface List Id. This is returned when the user
 *        creates an L2 Interface List. The user can add/remove
 *        L2 Interfaces (port interface, LAG interface, and
 *        various L2 Tunnel interfaces) to/from an L2 Interface
 *        List.
 */
typedef uint32_t xpsMcL2InterfaceListId_t;

/**
 * \typedef xpsMcL3InterfaceListId_t
 * \brief This type defines the L3 Multicast (Routing) Interface
 *        List Id. This is returned when the user
 *        creates an L3 Interface List. The user can add/remove
 *        L3 Interfaces (L3 over Port/LAG/VLAN interface, and
 *        various L3 Tunnel interfaces) to/from an L3 Interface
 *        List.
 */
typedef uint32_t xpsMcL3InterfaceListId_t;

/**
 * \typedef xpsMulticastPktCmdType_e
 * \brief This type defines the packet commands for XPS
 *        multicast.
 */
typedef xpMulticastPktCmdType_e xpsMulticastPktCmdType_e;

/**
 * \typedef xpsMulticastIPv4BridgeEntry_t
 * \brief This type defines the IPv4 Multicast L2 (Snooping)
 *        flow entry for XPS.
 */
typedef xpIPv4MulticastBridgeEntry_t xpsMulticastIPv4BridgeEntry_t;

/**
 * \typedef xpsMulticastIPv4RouteEntry_t
 * \brief This type defines the IPv4 Multicast L3 (Route) flow
 *        entry for XPS.
 */
typedef xpIPv4MulticastRouteEntry_t xpsMulticastIPv4RouteEntry_t;

/**
 * \typedef xpsMulticastIPv4PimBidirRpfEntry_t
 * \brief This type defines the IPv4 Multicast PimBidirRpf flow
 *        entry for XPS.
 */
typedef xpIPv4MulticastPimBidirRpfEntry_t xpsMulticastIPv4PimBidirRpfEntry_t;

/**
 * \typedef xpsMulticastIPv6BridgeEntry_t
 * \brief This type defines the IPv6 Multicast L2 (Snooping)
 *        flow entry for XPS.
 */
typedef xpIPv6MulticastBridgeEntry_t xpsMulticastIPv6BridgeEntry_t;

/**
 * \typedef xpsMulticastIPv6RouteEntry_t
 * \brief This type defines the IPv6 Multicast L3 (Route) flow
 *        entry for XPS.
 */
typedef xpIPv6MulticastRouteEntry_t xpsMulticastIPv6RouteEntry_t;

/**
 * \typedef xpsMcastDomainId_t
 * \brief This type defines the IPv6 Mcast Routing Domain Id
 *
 */
typedef xpMcastDomainId_t xpsMcastDomainId_t;

/**
* \brief This API initializes the multicast manager for a scope.
*        Does multicast specific global level initializations.
*
* \param [in] scopeId
*
* \return XP_STATUS
*/
XP_STATUS xpsMulticastInitScope(xpsScope_t scopeId);

/**
* \brief This API de-initializes the multicast manager for a scope.
*        Does multicast specific global level cleanups.
*
* \param [in] scopeId
*
* \return XP_STATUS
*/
XP_STATUS xpsMulticastDeInitScope(xpsScope_t scopeId);

/**
* \brief This API adds a device to the xps multicast
*        manager. Does multicast specific device level
*        initializations.
*
* \param [in] devId     Device Id of device
* \param [in] initType  Initialization Type
* \return XP_STATUS
*/
XP_STATUS xpsMulticastAddDevice(xpsDevice_t devId, xpsInitType_t initType);

/**
* \brief This API removes a device from the xps multicast
*        manager. Does multicast specific device level cleanups.
*
* \param [in] devId Device Id of device
* \return XP_STATUS
*/
XP_STATUS xpsMulticastRemoveDevice(xpsDevice_t devId);

/**
* \public
* \brief Creates an L2 Multicast Interface List for a scope.
*
*
* \param [in] scopeId   Scope ID of scope
* \param [out] ifListId  L2 Multicast Interface List Id Pointer.
* \param [in] vlanId    VLAN Id to be used for the L2 Interface
*        List.
* \return XP_STATUS Operation status
*/
XP_STATUS xpsMulticastCreateL2InterfaceListScope(xpsScope_t scopeId,
                                                 xpVlan_t vlanId, xpsMcL2InterfaceListId_t *ifListId);

/**
* \brief Creates an L3 Multicast Interface List for a scope.
*
* \param [in] scopeId   Scope ID of scope
* \param [in] ifListId  L3 Multicast Interface List Id Pointer.
* \return XP_STATUS
*/
XP_STATUS xpsMulticastCreateL3InterfaceListScope(xpsScope_t scopeId,
                                                 xpsMcL3InterfaceListId_t *ifListId);

/**
* \public
* \brief Destroys an L2 Multicast Interface List for a scope.
*
* \param [in] scopeId   Scope ID of scope
* \param [in] ifListId  L2 Multicast Interface List Id.
* \return XP_STATUS
*/
XP_STATUS xpsMulticastDestroyL2InterfaceListScope(xpsScope_t scopeId,
                                                  xpsMcL2InterfaceListId_t ifListId);

/**
* \brief Destroys an L3 Multicast Interface List for a scope.
*
*
* \param [in] scopeId   Scope ID of scope
* \param [in] ifListId  L3 Multicast Interface List Id.
* \return XP_STATUS
*/
XP_STATUS xpsMulticastDestroyL3InterfaceListScope(xpsScope_t scopeId,
                                                  xpsMcL3InterfaceListId_t ifListId);

/**
* \brief Sets up an L2 Multicast Interface List in a device.
*
* \param [in] devId           Device Id of device
* \param [in] l2IntfListId    L2 Multicast Interface List Id.
*
* \return XP_STATUS
*/
XP_STATUS xpsMulticastAddL2InterfaceListToDevice(xpsDevice_t devId,
                                                 xpsMcL2InterfaceListId_t l2IntfListId);

/**
* \brief Sets up an L3 Multicast Interface List in a device.
*
* \param [in] devId           Device Id of device
* \param [in] l3IntfListId    L3 Multicast Interface List Id.
*
* \return XP_STATUS
*/
XP_STATUS xpsMulticastAddL3InterfaceListToDevice(xpsDevice_t devId,
                                                 xpsMcL3InterfaceListId_t l3IntfListId);

/**
* \brief Removes an L2 Multicast Interface List from a device.
*
*
* \param [in] devId           Device Id of device
* \param [in] l2IntfListId    L2 Multicast Interface List Id.
*
* \return XP_STATUS
*/
XP_STATUS xpsMulticastRemoveL2InterfaceListFromDevice(xpsDevice_t devId,
                                                      xpsMcL2InterfaceListId_t l2IntfListId);

/**
* \brief Removes an L3 Multicast Interface List from a device.
*
* \param [in] devId           Device Id of device
* \param [in] l3IntfListId    L3 Multicast Interface List Id.
*
* \return XP_STATUS
*/
XP_STATUS xpsMulticastRemoveL3InterfaceListFromDevice(xpsDevice_t devId,
                                                      xpsMcL3InterfaceListId_t l3IntfListId);

/**
* \brief Adds an interface to an L2 Multicast Interface List.This API Adds an interface to an existing L2 Multicast
*        Interface List for a specified device.
*
* \param [in] devId Device Id of device
* \param [in] l2IntfListId  L2 Multicast Interface List Id.
* \param [in] intfId    Interface Id to be added.
* \return XP_STATUS
*/
XP_STATUS xpsMulticastAddInterfaceToL2InterfaceList(xpsDevice_t devId,
                                                    xpsMcL2InterfaceListId_t l2IntfListId, xpsInterfaceId_t intfId);

/**
* \brief Adds an interface to an L3 Multicast Interface List.
*        This API Adds an interface to an existing L3 Multicast
*        Interface List for a specified device.
*
* \param [in] devId       Device Id of device
* \param [in] l3IntfListId    L3 Multicast Interface List Id.
* \param [in] l3IntfId        Interface Id to be added.
* \return XP_STATUS Operation status
*                           - XP_NO_ERR: Successful execution
*                                                       - XP_ERROR: Otherwise
*/
XP_STATUS xpsMulticastAddInterfaceToL3InterfaceList(xpsDevice_t devId,
                                                    xpsMcL3InterfaceListId_t l3IntfListId, xpsInterfaceId_t l3IntfId);

/**
* \brief Adds a Multcast Bridging (L2) interface list to a
*        Multcast L3 Interface List.This API Adds a Multicast Bridging (L2) interface list to an
*        existing Multicast L3 Interface List for a specified device.
*
* \param [in] devId       Device Id of device
* \param [in] l3IntfListId    L3 Multicast Interface List Id.
* \param [in] l2IntfListId    Bridging (L2) Interface list Id to be added.
* \return XP_STATUS
*/
XP_STATUS xpsMulticastAddBridgingInterfaceToL3InterfaceList(xpsDevice_t devId,
                                                            xpsMcL3InterfaceListId_t l3IntfListId, xpsMcL2InterfaceListId_t l2IntfListId);

/**
* \brief Removes an interface from an L2 Multicast Interface
*        List.This API Removes an interface from an existing L2 Multicast
*        Interface List for a specified device.
*
* \param [in] devId Device Id of device
* \param [in] ifListId  L2 Multicast Interface List Id.
* \param [in] intfId    Interface Id to be removed.
* \return XP_STATUS
*/
XP_STATUS xpsMulticastRemoveInterfaceFromL2InterfaceList(xpsDevice_t devId,
                                                         xpsMcL2InterfaceListId_t ifListId, xpsInterfaceId_t intfId);

/**
* \brief Removes an interface from an L3 Multcast Interface
*        List. This API Removes an interface from an existing L3 Multicast
*        Interface List for a specified device.
*
* \param [in] devId           Device Id of device
* \param [in] l3IntfListId    L3 Multicast Interface List Id.
* \param [in] l3IntfId        L3 Interface Id to be removed.
* \return XP_STATUS
*/
XP_STATUS xpsMulticastRemoveInterfaceFromL3InterfaceList(xpsDevice_t devId,
                                                         xpsMcL3InterfaceListId_t l3IntfListId, xpsInterfaceId_t l3IntfId);

/**
* \brief Removes a Multicast Bridging (L2) Interface List from an L3 Multcast Interface List.
*        This API Removes a Multicast Bridging (L2) interface list from
*        an existing L3 Multicast Interface List for a specified
*        device.
*
* \param [in] devId           Device Id of device
* \param [in] l3IntfListId    L3 Multicast Interface List Id.
* \param [in] l2IntfListId    Briding (L2) Interface list Id to be removed.
* \return XP_STATUS
*/
XP_STATUS xpsMulticastRemoveBridgingInterfaceFromL3InterfaceList(
    xpsDevice_t devId, xpsMcL3InterfaceListId_t l3IntfListId,
    xpsMcL2InterfaceListId_t l2IntfListId);

/**
* \brief Inserts an Multicast Bridge entry in the specified
*        device.This method inserts an IPv4 Multicast Bridge entry in the
*        specified device.
*
* \param [in] devId       Device Id of the device in which the
*                         entry needs to be inserted.
* \param [in] entry       Contains entry (key + data) to be written
* \return XP_STATUS
*/
XP_STATUS xpsMulticastAddIPv4BridgeEntry(xpsDevice_t devId,
                                         xpsMulticastIPv4BridgeEntry_t * entry,
                                         uint32_t routeEntryIdx);

/**
* \brief Inserts an Multicast Bridge entry in the specified
*        device.This method inserts an IPv6 Multicast Bridge entry in the
*        specified device.
*
* \param [in] devId        Device Id of the device in which the
*                          entry needs to be inserted.
* \param [in] entry        Contains entry (key + data) to be written
* \param [out] indexList   Index at which the entry is written to hardware is at indexList->index[0]. Indices of entry which are rehashed is present at indexList->index[1], indexList->index[2]... so on
*
* \return XP_STATUS
*/
XP_STATUS xpsMulticastAddIPv6BridgeEntry(xpsDevice_t devId,
                                         xpsMulticastIPv6BridgeEntry_t * entry, xpsHashIndexList_t *indexList);

/**
* \public
* \brief Inserts an IPv4 Multicast Route entry in the specified
*        device. This method inserts an IPv4 Multicast Route entry in the
*        specified device.
*
* \param [in] devId        Device Id of the device in which the
*                          entry needs to be inserted.
* \param [in] entry        Contains entry (key + data) to be written
* \param [out] indexList   Index at which the entry is written to hardware is at indexList->index[0]. Indices of entry which are rehashed is present at indexList->index[1], indexList->index[2]... so on
*
* \return XP_STATUS
*/
XP_STATUS xpsMulticastAddIPv4RouteEntry(xpsDevice_t devId,
                                        xpsMulticastIPv4RouteEntry_t *entry, xpsHashIndexList_t *indexList);

/**
* \public
* \brief Inserts an IPv6 Multicast Route entry in the specified
*        device. This method inserts an IPv6 Multicast Route entry in the
*        specified device.
*
* \param [in] devId       Device Id of the device in which the
*                         entry needs to be inserted.
* \param [in] entry       Contains entry (key + data) to be written
* \param [out] indexList  Index at which the entry is written to hardware is at indexList->index[0]. Indices of entry which are rehashed is present at indexList->index[1], indexList->index[2]... so on
*
* \return XP_STATUS
*/
XP_STATUS xpsMulticastAddIPv6RouteEntry(xpsDevice_t devId,
                                        xpsMulticastIPv6RouteEntry_t *entry, xpsHashIndexList_t *indexList);

/**
* \brief Inserts an IPv4 Multicast PIM BiDir RPF entry in
*        the specified device. This method inserts an IPv4 Multicast PIM BiDir RPF entry in
*        the specified device.
*
* \param [in] devId       Device Id of the device in which the
*                         entry needs to be inserted.
* \param [in] entry       Contains entry (key + data) to be written
* \param [out] indexList  Index at which the entry is written to hardware is at indexList->index[0]. Indices of entry which are rehashed is present at indexList->index[1], indexList->index[2]... so on
*
* \return XP_STATUS
*/
XP_STATUS xpsMulticastAddIPv4PimBiDirRPFEntry(xpsDevice_t devId,
                                              xpsMulticastIPv4PimBidirRpfEntry_t * entry, xpsHashIndexList_t *indexList);

/**
* \brief Removes an IPv4 Multicast Bridge entry from a specified
*        device. This method frees the IPv4 Multicast Bridge Entry from a
*        specified device
*
* \param [in] devId     Device Id of device
* \param [in] entry     entry to remove
* \return XP_STATUS
*/
XP_STATUS xpsMulticastRemoveIPv4BridgeEntry(xpsDevice_t devId,
                                            xpsMulticastIPv4BridgeEntry_t *entry);

/**
* \brief Removes an IPv4 Multicast Route entry from a specified
*        device.This method frees the IPv4 Multicast Route Entry from a
*        specified device
*
* \param [in] devId     Device Id of device
* \param [in] index     Index of the entry to remove
* \return XP_STATUS
*/
XP_STATUS xpsMulticastRemoveIPv4RouteEntry(xpsDevice_t devId, uint32_t index);

/**
* \brief Removes an IPv4 PIM BiDir RPF entry from a specified
*        device. This method frees the IPv4 IPv4 PIM BiDir RPF Entry from a
*        specified device
*
* \param [in] devId     Device Id of device
* \param [in] index     Index of the entry to remove
* \return XP_STATUS
*/
XP_STATUS xpsMulticastRemoveIPv4PimBiDirRPFEntry(xpsDevice_t devId,
                                                 uint32_t index);

/**
* \brief Reads an IPv4 Multicast Bridge entry from the specified
*        device.This method reads an IPv4 Multicast Bridge entry from the
*        specified device.
*
* \param [in] devId    Device Id of the device from which the
*                      entry needs to be read.
* \param [in] index    Index of the entry to be read
* \param [out] entry   Contains entry (key + data) to be read
* \return XP_STATUS
*/
XP_STATUS xpsMulticastGetIPv4BridgeEntry(xpsDevice_t devId, uint32_t index,
                                         xpsMulticastIPv4BridgeEntry_t * entry);

/**
* \brief Reads an IPv6 Multicast Bridge entry from the specified
*        device.This method reads an IPv6 Multicast Bridge entry from the
*        specified device.
*
* \param [in] devId    Device Id of the device from which the
*                      entry needs to be read.
* \param [in] index    Index of the entry to be read
* \param [out] entry   Contains entry (key + data) to be read
* \return XP_STATUS
*/
XP_STATUS xpsMulticastGetIPv6BridgeEntry(xpsDevice_t devId, uint32_t index,
                                         xpsMulticastIPv6BridgeEntry_t * entry);

/**
* \brief Reads an IPv4 Multicast Route entry from the specified
*        device. This method reads an IPv4 Multicast Route entry from the
*        specified device.
*
*  \param [in] devId    Device Id of the device from which the
*  i                    entry needs to be read.
* \param [in] index     Index of the entry to be read
* \param [out] entry    Contains entry (key + data) to be read
* \return XP_STATUS
*/
XP_STATUS xpsMulticastGetIPv4RouteEntry(xpsDevice_t devId, uint32_t index,
                                        xpsMulticastIPv4RouteEntry_t *entry);

/**
* \brief Reads an IPv6 Multicast Route entry from the specified
*        device. This method reads an IPv6 Multicast Route entry from the
*        specified device.
*
*  \param [in] devId    Device Id of the device from which the
*  i                    entry needs to be read.
* \param [in] index     Index of the entry to be read
* \param [out] entry    Contains entry (key + data) to be read
* \return XP_STATUS
*/
XP_STATUS xpsMulticastGetIPv6RouteEntry(xpsDevice_t devId, uint32_t index,
                                        xpsMulticastIPv6RouteEntry_t *entry);

/**
* \brief Reads an IPv4 Multicast PIM BiDir RPF entry from
*        the specified device.This method reads an IPv4 Multicast PIM BiDir RPF entry from
*        the specified device.
*
*  \param [in] devId    Device Id of the device from which the
*                       entry needs to be read.
* \param [in] index     Index of the entry to be read
* \param [out] entry    Contains entry (key + data) to be read
* \return XP_STATUS
*/
XP_STATUS xpsMulticastGetIPv4PimBiDirRPFEntry(xpsDevice_t devId, uint32_t index,
                                              xpsMulticastIPv4PimBidirRpfEntry_t * entry);

/**
* \brief Removes an IPv6 Multicast Bridge entry from a specified
*        device. This method frees the IPv6 Multicast Bridge Entry from a
*        specified device
*
* \param [in] devId     Device Id of device
* \param [in] index     Index of the entry to remove
* \return XP_STATUS
*/
XP_STATUS xpsMulticastRemoveIPv6BridgeEntry(xpsDevice_t devId, uint32_t index);

/**
* \brief Removes an IPv6 Multicast Route entry from a specified
*        device.This method frees the IPv6 Multicast Route Entry from a
*        specified device
*
* \param [in] devId     Device Id of device
* \param [in] index     Index of the entry to remove
* \return XP_STATUS
*/
XP_STATUS xpsMulticastRemoveIPv6RouteEntry(xpsDevice_t devId, uint32_t index);

/**
* \brief Reads an IPv6 Multicast Bridge entry from the specified
*        device.This method reads an IPv6 Multicast Bridge entry from the
*        specified device.
*
* \param [in] devId    Device Id of the device from which the
*                      entry needs to be read.
* \param [in] index    Index of the entry to be read
* \param [out] entry   Contains entry (key + data) to be read
* \return XP_STATUS
*/
XP_STATUS xpsMulticastGetIPv6BridgeEntry(xpsDevice_t devId, uint32_t index,
                                         xpsMulticastIPv6BridgeEntry_t * entry);

/**
* \brief Reads an IPv6 Multicast Route entry from the specified
*        device. This method reads an IPv6 Multicast Route entry from the
*        specified device.
*
*  \param [in] devId    Device Id of the device from which the
*  i                    entry needs to be read.
* \param [in] index     Index of the entry to be read
* \param [out] entry    Contains entry (key + data) to be read
* \return XP_STATUS
*/
XP_STATUS xpsMulticastGetIPv6RouteEntry(xpsDevice_t devId, uint32_t index,
                                        xpsMulticastIPv6RouteEntry_t *entry);

/**
 * \public
 * \brief Trigger the SW Control Aging Cycle for Ipv4 Bridge Multicast Table
 *
 * \param [in] devId
 *
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastTriggerIpv4BridgeTableAging(xpDevice_t devId);

/**
 * \public
 * \brief Enable Aging on the Ipv4 Bridge Multicast table
 *
 * \param [in] devId
 * \param [in] enable
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastConfigureIpv4BridgeTableAging(xpDevice_t devId,
                                                    uint8_t enable);

/**u
 * \public
 * \brief Enable Aging on the Ipv4 Bridge Multicast table
 *
 * \param [in] devId
 * \param [in] enable
 * \param [in] index
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastConfigureIpv4BridgeEntryAging(xpDevice_t devId,
                                                    uint8_t enable, uint32_t index);

/**
 * \public
 * \brief Get Aging time for the Ipv4 Bridge Multicast table
 *
 * \param [in] devId
 * \param [in] &enable
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastGetIpv4BridgeTableAgingStatus(xpDevice_t devId,
                                                    bool *enable);

/**
 * \public
 * \brief Get Aging time for the Ipv4 Bridge Multicast table
 *
 * \param [in] devId
 * \param [out] &enable
 * \param [in] index
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastGetIpv4BridgeEntryAgingStatus(xpDevice_t devId,
                                                    bool *enable, uint32_t index);

/**
 * \public
 * \brief Set Aging time for the Ipv4 Bridge Multicast table
 *
 * \param [in] devId
 * \param [in] sec
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastSetIpv4BridgeTableAgingCycle(xpDevice_t devId,
                                                   uint32_t sec);

/**
 * \public
 * \brief Get Aging time for the Ipv4 Bridge Multicast table
 *
 * \param [in] devId
 * \param [in] sec
 * \returns XP_STATUS indicating success or failure
 *
 */
XP_STATUS xpsMulticastGetIpv4BridgeTableAgingCycle(xpDevice_t devId,
                                                   uint32_t *sec);


/**
 * \public
 * \brief  Register the Aging Handler for the Ipv4 Bridge Multicast Table.
 * Please make sure all the Table that have Sharing Same Age Resources
 * only one Handler is Registered. Use Case of Multi-Pipe
 *
 *
 * \param [in] devId
 * \param [in] func
 * \param [in] userData
 * \returns XP_STATUS  Operation Status
 *
 */
XP_STATUS xpsMulticastRegisterIpv4BridgeTableAgingHandler(xpDevice_t devId,
                                                          multicastAgingHandler func, void *userData);

/**
 * \public
 * \brief  Register the Default Aging Handler for the Ipv4 Bridge Multicast Table.
 * Which ideally removes the aged out entry
 *
 * \param [in] devId
 * \returns XP_STATUS  Operation Status
 *
 */
XP_STATUS xpsMulticastRegisterIpv4BridgeTableDefaultAgingHandler(
    xpDevice_t devId);

/**
 * \public
 * \brief Unregister the Aging Handler for the Ipv4 Bridge Multicast Table.
 *
 * \param [in] devId
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastUnregisterIpv4BridgeTableAgingHandler(xpDevice_t devId);


/**
 * \public
 * \brief Trigger the SW Control Aging Cycle for Ipv4 Bridge Multicast Table
 *
 * \param [in] devId
 *
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastTriggerIpv4RouteTableAging(xpDevice_t devId);

/**
 * \public
 * \brief Enable Aging on the Ipv4 Bridge Multicast table
 *
 * \param [in] devId
 * \param [in] enable
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastConfigureIpv4RouteTableAging(xpDevice_t devId,
                                                   uint8_t enable);

/**
 * \public
 * \brief Enable Aging on the Ipv4 Bridge Multicast table
 *
 * \param [in] devId
 * \param [in] enable
 * \param [in] index
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastConfigureIpv4RouteEntryAging(xpDevice_t devId,
                                                   uint8_t enable, uint32_t index);

/**
 * \public
 * \brief Get Aging time for the Ipv4 Route Multicast table
 *
 * \param [in] devId
 * \param [in] &enable
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastGetIpv4RouteTableAgingStatus(xpDevice_t devId,
                                                   bool *enable);

/**
 * \public
 * \brief Get Aging time for the Ipv4 Route Multicast table
 *
 * \param [in] devId
 * \param [out] &enable
 * \param [in] index
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastGetIpv4RouteEntryAgingStatus(xpDevice_t devId,
                                                   bool *enable, uint32_t index);

/**
 * \public
 * \brief Set Aging time for the Ipv4 Bridge Multicast table
 *
 * \param [in] devId
 * \param [in] sec
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastSetIpv4RouteTableAgingCycle(xpDevice_t devId,
                                                  uint32_t sec);

/**
 * \public
 * \brief Get Aging time for the Ipv4 Bridge Multicast table
 *
 * \param [in] devId
 * \param [in] sec
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastGetIpv4RouteTableAgingCycle(xpDevice_t devId,
                                                  uint32_t *sec);

/**
 * \public
 * \brief  Register the Aging Handler for the Ipv4 Route Multicast Table.
 * Please make sure all the Table that have Sharing Same Age Resources
 * only one Handler is Registered. Use Case of Multi-Pipe
 *
 *
 * \param [in] devId
 * \param [in] func
 * \param [in] userData
 * \returns XP_STATUS  Operation Status
 *
 */
XP_STATUS xpsMulticastRegisterIpv4RouteTableAgingHandler(xpDevice_t devId,
                                                         multicastAgingHandler func, void *userData);

/**
 * \public
 * \brief  Register the Default Aging Handler for the Ipv4 Route Multicast Table.
 * Which ideally removes the aged out entry
 *
 * \param [in] devId
 * \returns XP_STATUS  Operation Status
 *
 */
XP_STATUS xpsMulticastRegisterIpv4RouteTableDefaultAgingHandler(
    xpDevice_t devId);

/**
 * \public
 * \brief Unregister the Aging Handler for the Ipv4 Routee Multicast Table.
 *
 * \param [in] devId
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastUnregisterIpv4RouteTableAgingHandler(xpDevice_t devId);


/**
 * \public
 * \brief Trigger the SW Control Aging Cycle for Ipv6 Bridge Multicast Table
 *
 * \param [in] devId
 *
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastTriggerIpv6BridgeTableAging(xpDevice_t devId);

/**
 * \public
 * \brief Enable Aging on the Ipv6 Bridge Multicast table
 *
 * \param [in] devId
 * \param [in] enable
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastConfigureIpv6BridgeTableAging(xpDevice_t devId,
                                                    bool enable);

/**u
 * \public
 * \brief Enable Aging on the Ipv6 Bridge Multicast table
 *
 * \param [in] devId
 * \param [in] enable
 * \param [in] index
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastConfigureIpv6BridgeEntryAging(xpDevice_t devId,
                                                    bool enable, uint32_t index);

/**
 * \public
 * \brief Get Aging time for the Ipv6 Bridge Multicast table
 *
 * \param [in] devId
 * \param [in] enable
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastGetIpv6BridgeTableAgingStatus(xpDevice_t devId,
                                                    bool *enable);

/**
 * \public
 * \brief Get Aging time for the Ipv6 Bridge Multicast table
 *
 * \param [in] devId
 * \param [out] &enable
 * \param [in] index
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastGetIpv6BridgeEntryAgingStatus(xpDevice_t devId,
                                                    bool *enable, uint32_t index);

/**
 * \public
 * \brief Set Aging time for the Ipv6 Bridge Multicast table
 *
 * \param [in] devId
 * \param [in] sec
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastSetIpv6BridgeTableAgingCycle(xpDevice_t devId,
                                                   uint32_t sec);

/**
 * \public
 * \brief Get Aging time for the Ipv6 Bridge Multicast table
 *
 * \param [in] devId
 * \param [in] sec
 * \returns XP_STATUS indicating success or failure
 *
 */
XP_STATUS xpsMulticastGetIpv6BridgeTableAgingCycle(xpDevice_t devId,
                                                   uint32_t *sec);


/**
 * \public
 * \brief  Register the Aging Handler for the Ipv6 Bridge Multicast Table.
 * Please make sure all the Table that have Sharing Same Age Resources
 * only one Handler is Registered. Use Case of Multi-Pipe
 *
 *
 * \param [in] devId
 * \param [in] func
 * \param [in] userData
 * \returns XP_STATUS  Operation Status
 *
 */
XP_STATUS xpsMulticastRegisterIpv6BridgeTableAgingHandler(xpDevice_t devId,
                                                          multicastAgingHandler func, void *userData);

/**
 * \public
 * \brief  Register the Default Aging Handler for the Ipv6 Bridge Multicast Table.
 * Which ideally removes the aged out entry
 *
 * \param [in] devId
 * \returns XP_STATUS  Operation Status
 *
 */
XP_STATUS xpsMulticastRegisterIpv6BridgeTableDefaultAgingHandler(
    xpDevice_t devId);

/**
 * \public
 * \brief Unregister the Aging Handler for the Ipv6 Bridge Multicast Table.
 *
 * \param [in] devId
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastUnregisterIpv6BridgeTableAgingHandler(xpDevice_t devId);


/**
 * \public
 * \brief Trigger the SW Control Aging Cycle for Ipv6 Bridge Multicast Table
 *
 * \param [in] devId
 *
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastTriggerIpv6RouteTableAging(xpDevice_t devId);

/**
 * \public
 * \brief Enable Aging on the Ipv6 Bridge Multicast table
 *
 * \param [in] devId
 * \param [in] enable
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastConfigureIpv6RouteTableAging(xpDevice_t devId,
                                                   bool enable);

/**
 * \public
 * \brief Enable Aging on the Ipv6 Bridge Multicast table
 *
 * \param [in] devId
 * \param [in] enable
 * \param [in] index
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastConfigureIpv6RouteEntryAging(xpDevice_t devId,
                                                   bool enable, uint32_t index);

/**
 * \public
 * \brief Get Aging time for the Ipv6 Route Multicast table
 *
 * \param [in] devId
 * \param [in] &enable
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastGetIpv6RouteTableAgingStatus(xpDevice_t devId,
                                                   bool *enable);

/**
 * \public
 * \brief Get Aging time for the Ipv6 Route Multicast table
 *
 * \param [in] devId
 * \param [out] &enable
 * \param [in] index
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastGetIpv6RouteEntryAgingStatus(xpDevice_t devId,
                                                   bool *enable, uint32_t index);

/**
 * \public
 * \brief Set Aging time for the Ipv6 Bridge Multicast table
 *
 * \param [in] devId
 * \param [in] sec
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastSetIpv6RouteTableAgingCycle(xpDevice_t devId,
                                                  uint32_t sec);

/**
 * \public
 * \brief Get Aging time for the Ipv6 Bridge Multicast table
 *
 * \param [in] devId
 * \param [in] *sec
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastGetIpv6RouteTableAgingCycle(xpDevice_t devId,
                                                  uint32_t *sec);

/**
 * \public
 * \brief  Register the Aging Handler for the Ipv6 Route Multicast Table.
 * Please make sure all the Table that have Sharing Same Age Resources
 * only one Handler is Registered. Use Case of Multi-Pipe
 *
 *
 * \param [in] devId
 * \param [in] func
 * \param [in] userData
 * \returns XP_STATUS  Operation Status
 *
 */
XP_STATUS xpsMulticastRegisterIpv6RouteTableAgingHandler(xpDevice_t devId,
                                                         multicastAgingHandler func, void *userData);

/**
 * \public
 * \brief  Register the Default Aging Handler for the Ipv6 Route Multicast Table.
 * Which ideally removes the aged out entry
 *
 * \param [in] devId
 * \returns XP_STATUS  Operation Status
 *
 */
XP_STATUS xpsMulticastRegisterIpv6RouteTableDefaultAgingHandler(
    xpDevice_t devId);

/**
 * \public
 * \brief Unregister the Aging Handler for the Ipv6 Routee Multicast Table.
 *
 * \param [in] devId
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastUnregisterIpv6RouteTableAgingHandler(xpDevice_t devId);


/** \public
* \brief Get multicast VIF used for an L2 interface list for a scope.
*
* This method returns a VIF_ID that is used for multicast
* transmission from an interface list
*
* \param [in]  scopeId     Scope ID of scope
* \param [in]  ifListId    interface list id
* \param [in]  encapType   encapsulation type
* \param [out] mcVif       multicast VIF_ID
* \return XP_STATUS
*/
XP_STATUS xpsMulticastGetVifFromL2InterfaceListScope(xpsScope_t scopeId,
                                                     xpsMcL2InterfaceListId_t ifListId, xpL2EncapType_e encapType, xpVif_t *mcVif);

/**
* \public
* \brief Get multicast VIF used for an L3 interface list for a scope.
*
* This method returns a VIF_ID that is used for multicast
* transmission from an interface list from l3 interface
*
* \param [in]  scopeId     Scope ID of scope
* \param [in]  ifListId    interface list id
* \param [in]  l3IntfId    id of outgoing L3 interface in list
* \param [in]  encapType   encapsulation type
* \param [out] mcVif       multicast VIF_ID
* \return XP_STATUS
*/
XP_STATUS xpsMulticastGetVifFromL3InterfaceListScope(xpsScope_t scopeId,
                                                     xpsMcL3InterfaceListId_t ifListId, xpsInterfaceId_t l3IntfId,
                                                     xpL3EncapType_e encapType, xpVif_t *mcVif);

/**
 * \public
 * \brief Trigger the SW Control Aging Cycle for Ipv6 Bridge Multicast Table
 *
 * \param [in] devId
 *
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastTriggerIpv6BridgeTableAging(xpDevice_t devId);

/**
 * \public
 * \brief Enable Aging on the Ipv6 Bridge Multicast table
 *
 * \param [in] devId
 * \param [in] enable
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastConfigureIpv6BridgeTableAging(xpDevice_t devId,
                                                    bool enable);

/**u
 * \public
 * \brief Enable Aging on the Ipv6 Bridge Multicast table
 *
 * \param [in] devId
 * \param [in] enable
 * \param [in] index
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastConfigureIpv6BridgeEntryAging(xpDevice_t devId,
                                                    bool enable, uint32_t index);

/**
 * \public
 * \brief Get Aging time for the Ipv6 Bridge Multicast table
 *
 * \param [in] devId
 * \param [in] &enable
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastGetIpv6BridgeTableAgingStatus(xpDevice_t devId,
                                                    bool *enable);

/**
 * \public
 * \brief Get Aging time for the Ipv6 Bridge Multicast table
 *
 * \param [in] devId
 * \param [out] &enable
 * \param [in] index
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastGetIpv6BridgeEntryAgingStatus(xpDevice_t devId,
                                                    bool *enable, uint32_t index);

/**
 * \public
 * \brief Set Aging time for the Ipv6 Bridge Multicast table
 *
 * \param [in] devId
 * \param [in] sec
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastSetIpv6BridgeTableAgingCycle(xpDevice_t devId,
                                                   uint32_t sec);

/**
 * \public
 * \brief Get Aging time for the Ipv6 Bridge Multicast table
 *
 * \param [in] devId
 * \param [in] *sec
 * \returns XP_STATUS indicating success or failure
 *
 */
XP_STATUS xpsMulticastGetIpv6BridgeTableAgingCycle(xpDevice_t devId,
                                                   uint32_t *sec);


/**
 * \public
 * \brief  Register the Aging Handler for the Ipv6 Bridge Multicast Table.
 * Please make sure all the Table that have Sharing Same Age Resources
 * only one Handler is Registered. Use Case of Multi-Pipe
 *
 *
 * \param [in] devId
 * \param [in] func
 * \param [in] userData
 * \returns XP_STATUS  Operation Status
 *
 */
XP_STATUS xpsMulticastRegisterIpv6BridgeTableAgingHandler(xpDevice_t devId,
                                                          multicastAgingHandler func, void *userData);

/**
 * \public
 * \brief  Register the Default Aging Handler for the Ipv6 Bridge Multicast Table.
 * Which ideally removes the aged out entry
 *
 * \param [in] devId
 * \returns XP_STATUS  Operation Status
 *
 */
//XP_STATUS xpsMulticastRegisterIpv6BridgeTableDefaultAgingHandler (xpDevice_t devId);

/**
 * \public
 * \brief Unregister the Aging Handler for the Ipv6 Bridge Multicast Table.
 *
 * \param [in] devId
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastUnregisterIpv6BridgeTableAgingHandler(xpDevice_t devId);


/**
 * \public
 * \brief Trigger the SW Control Aging Cycle for Ipv6 Bridge Multicast Table
 *
 * \param [in] devId
 *
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastTriggerIpv6RouteTableAging(xpDevice_t devId);

/**
 * \public
 * \brief Enable Aging on the Ipv6 Bridge Multicast table
 *
 * \param [in] devId
 * \param [in] enable
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastConfigureIpv6RouteTableAging(xpDevice_t devId,
                                                   bool enable);

/**
 * \public
 * \brief Enable Aging on the Ipv6 Bridge Multicast table
 *
 * \param [in] devId
 * \param [in] enable
 * \param [in] index
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastConfigureIpv6RouteEntryAging(xpDevice_t devId,
                                                   bool enable, uint32_t index);

/**
 * \public
 * \brief Get Aging time for the Ipv6 Route Multicast table
 *
 * \param [in] devId
 * \param [in] &enable
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastGetIpv6RouteTableAgingStatus(xpDevice_t devId,
                                                   bool *enable);

/**
 * \public
 * \brief Get Aging time for the Ipv6 Route Multicast table
 *
 * \param [in] devId
 * \param [out] &enable
 * \param [in] index
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastGetIpv6RouteEntryAgingStatus(xpDevice_t devId,
                                                   bool *enable, uint32_t index);

/**
 * \public
 * \brief Set Aging time for the Ipv6 Bridge Multicast table
 *
 * \param [in] devId
 * \param [in] sec
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastSetIpv6RouteTableAgingCycle(xpDevice_t devId,
                                                  uint32_t sec);

/**
 * \public
 * \brief Updates the packet command field in the IPv4 Multicast
 *        Bridge Entry at the specified index in a device.
 *
 * \param [in] devId        Device Id of device
 * \param [in] index        Index of the entry
 * \param [in] pktCmd       Packet Command to set
 * \return XP_STATUS
 */
XP_STATUS xpsMulticastSetIpv4MulticastBridgeEntryPktCmd(uint8_t devId,
                                                        uint32_t index, xpMulticastPktCmdType_e pktCmd);

/**
 * \public
 * \brief Updates the VIF Index field in the IPv4 Multicast
 *        Bridge Entry at the specified index in a device.
 *
 * \param [in] devId        Device Id of device
 * \param [in] index        Index of the entry
 * \param [in] vifIdx       The Vif Index to set
 * \return XP_STATUS
 */
XP_STATUS xpsMulticastSetIpv4MulticastBridgeEntryVifIdx(uint8_t devId,
                                                        uint32_t index, uint32_t vifIdx);

/**
 * \public
 * \brief Updates the packet command field in the IPv6 Multicast
 *        Bridge Entry at the specified index in a device.
 *
 * \param [in] devId        Device Id of device
 * \param [in] index        Index of the entry
 * \param [in] pktCmd       Packet Command to set
 * \return XP_STATUS
 */
XP_STATUS xpsMulticastSetIpv6MulticastBridgeEntryPktCmd(uint8_t devId,
                                                        uint32_t index, xpMulticastPktCmdType_e pktCmd);

/**
 * \public
 * \brief Updates the VIF Index field in the IPv6 Multicast
 *        Bridge Entry at the specified index in a device.
 *
 * \param [in] devId        Device Id of device
 * \param [in] index        Index of the entry
 * \param [in] vifIdx       The Vif Index to set
 * \return XP_STATUS
 */
XP_STATUS xpsMulticastSetIpv6MulticastBridgeEntryVifIdx(uint8_t devId,
                                                        uint32_t index, uint32_t vifIdx);

/**
 * \public
 * \brief Updates the packet command field in the IPv4 Multicast
 *        Route Entry at the specified index in a device.
 *
 * \param [in] devId        Device Id of device
 * \param [in] index        Index of the entry
 * \param [in] pktCmd       Packet Command to set
 * \return XP_STATUS
 */
XP_STATUS xpsMulticastSetIpv4MulticastRouteEntryPktCmd(uint8_t devId,
                                                       uint32_t index, xpMulticastPktCmdType_e pktCmd);

/**
 * \public
 * \brief Updates the VIF Index field in the IPv4 Multicast
 *        Route Entry at the specified index in a device.
 *
 * \param [in] devId        Device Id of device
 * \param [in] index        Index of the entry
 * \param [in] vifIdx       The Vif Index to set
 * \return XP_STATUS
 */
XP_STATUS xpsMulticastSetIpv4MulticastRouteEntryVifIdx(uint8_t devId,
                                                       uint32_t index, uint32_t vifIdx);

/**
 * \public
 * \brief Updates the packet command field in the IPv6 Multicast
 *        Route Entry at the specified index in a device.
 *
 * \param [in] devId        Device Id of device
 * \param [in] index        Index of the entry
 * \param [in] pktCmd       Packet Command to set
 * \return XP_STATUS
 */
XP_STATUS xpsMulticastSetIpv6MulticastRouteEntryPktCmd(uint8_t devId,
                                                       uint32_t index, xpMulticastPktCmdType_e pktCmd);

/**
 * \public
 * \brief Updates the VIF Index field in the IPv6 Multicast
 *        Route Entry at the specified index in a device.
 *
 * \param [in] devId        Device Id of device
 * \param [in] index        Index of the entry
 * \param [in] vifIdx       The Vif Index to set
 * \return XP_STATUS
 */
XP_STATUS xpsMulticastSetIpv6MulticastRouteEntryVifIdx(uint8_t devId,
                                                       uint32_t index, uint32_t vifIdx);

/**
 * \public
 * \brief Updates the rpf params in the IPv4 Multicast
 *        Route Entry at the specified index in a device.
 *
 * \param [in] devId        Device Id of device
 * \param [in] index        Index of the entry
 * \param [in] rpfType      xpMulticastRpfCheckType to be set
 * \param [in] rpfValue     RPF value to be programmed
 * \param [in] rpfFailCmd   RPF fail cmd to be set
 * \return XP_STATUS
 */
XP_STATUS xpsMulticastSetIpv4MulticastRouteEntryRpfParams(uint8_t devId,
                                                          uint32_t index, xpMulticastRpfCheckType_e rpfType, uint32_t rpfValue,
                                                          xpMulticastRpfFailCmd_e rpfFailCmd);

/**
 * \public
 * \brief Updates the rpf params in the IPv6 Multicast
 *        Route Entry at the specified index in a device.
 *
 * \param [in] devId        Device Id of device
 * \param [in] index        Index of the entry
 * \param [in] rpfType      xpMulticastRpfCheckType to be set
 * \param [in] rpfValue     RPF value to be programmed
 * \param [in] rpfFailCmd   RPF fail cmd to be set
 * \return XP_STATUS
 */
XP_STATUS xpsMulticastSetIpv6MulticastRouteEntryRpfParams(uint8_t devId,
                                                          uint32_t index, xpMulticastRpfCheckType_e rpfType, uint32_t rpfValue,
                                                          xpMulticastRpfFailCmd_e rpfFailCmd);

/**
 * \public
 * \brief Get Aging time for the Ipv6 Bridge Multicast table
 *
 * \param [in] devId
 * \param [in] *sec
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastGetIpv6RouteTableAgingCycle(xpDevice_t devId,
                                                  uint32_t *sec);

/**
 * \public
 * \brief  Register the Aging Handler for the Ipv6 Route Multicast Table.
 * Please make sure all the Table that have Sharing Same Age Resources
 * only one Handler is Registered. Use Case of Multi-Pipe
 *
 *
 * \param [in] devId
 * \param [in] func
 * \param [in] userData
 * \returns XP_STATUS  Operation Status
 *
 */
XP_STATUS xpsMulticastRegisterIpv6RouteTableAgingHandler(xpDevice_t devId,
                                                         multicastAgingHandler func, void *userData);

/**
 * \public
 * \brief  Register the Default Aging Handler for the Ipv6 Route Multicast Table.
 * Which ideally removes the aged out entry
 *
 * \param [in] devId
 * \returns XP_STATUS  Operation Status
 *
 */
//XP_STATUS xpsMulticastRegisterIpv6RouteTableDefaultAgingHandler (xpDevice_t devId);

/**
 * \public
 * \brief Unregister the Aging Handler for the Ipv6 Routee Multicast Table.
 *
 * \param [in] devId
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastUnregisterIpv6RouteTableAgingHandler(xpDevice_t devId);

/**
 * \public
 * \brief Allocates a Mcast L3 Domain Id for IPV6 Multicast for a scope. It would be
 * a number between 0-127. This would be allocated per Multicast enabled VRF
 * \param [in] scopeId           Scope ID of scope
 * \param [out] mcL3DomainId
 * \returns XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastAllocateIpv6L3DomainIdScope(xpsScope_t scopeId,
                                                  xpsMcastDomainId_t *mcL3DomainId);

/**
 * \public
 * \brief Release a Mcast L3 Domain Id for IPV6 Mcast for a scope. It would be a number
 * between 0-127. This is allocated per Multicast Enabled VRF
 * \param [in] scopeId           Scope ID of scope
 * \param [in]
 *\returns XP_STATUS Operation status
 *
 */
XP_STATUS xpsMulticastReleaseIpv6L3DomainIdScope(xpsScope_t scopeId,
                                                 xpsMcastDomainId_t mcL3DomainId);

/**
 * \public
 * \brief Create if L3DomainId not allocated for the vlan or else use to get the L3DomainId for IPV6 Mcast.
 * \param [in] devId
 * \param [in] vrfId
 * \param [in] mcL3DomainId
 *\returns XP_STATUS Operation status
 *
 */
XP_STATUS xpsMcGetCreateMcL3DomainId(xpDevice_t devId, uint32_t vrfId,
                                     xpMcastDomainId_t *mcL3DomainId);

/**
 * \public
 * \brief Release and destroy Mcast L3 Domain Id for IPV6 Mcast.
 * \param [in] devId
 * \param [in] vrfId
 *\returns XP_STATUS Operation status
 *
 */
XP_STATUS xpsMcReleaseDestroyL3DomainId(xpDevice_t devId, uint32_t vrfId);

/**
 * \public
 * \brief Destroy a Mcast L3 Domain Id for IPV6 Mcast for a scope.
 * \param [in] scopeId Scope ID of scope
 * \param [in] vrfId
 *\returns XP_STATUS Operation status
 *
 */
XP_STATUS xpsMcDestroyL3DomainIdScope(xpsScope_t scopeId, uint32_t vrfId);

/**
 * \public
 * \brief Create if L2DomainId not allocated for the vlan or else use to get the L2DomainId for IPV6 Mcast.
 * \param [in] devId
 * \param [in] vlanId
 * \param [in] mcL2DomainId
 *\returns XP_STATUS Operation status
 *
 */
XP_STATUS xpsMcGetCreateMcL2DomainId(xpDevice_t devId, uint32_t vlanId,
                                     xpMcastDomainId_t *mcL2DomainId);

/**
 * \public
 * \brief Release and destroy Mcast L2 Domain Id for IPV6 Mcast.
 * \param [in] devId
 * \param [in] vlanId
 *\returns XP_STATUS Operation status
 *
 */
XP_STATUS xpsMcReleaseDestroyL2DomainId(xpDevice_t devId, uint32_t vlanId);

/**
* \brief This API initializes the multicast manager. Does
*        multicast specific global level initializations.
*
* \return XP_STATUS
*/
XP_STATUS xpsMulticastInit();

/**
* \brief This API de-initializes the multicast manager. Does
*        multicast specific global level cleanups.
*
* \return XP_STATUS
*/
XP_STATUS xpsMulticastDeInit();

/**
* \public
* \brief Creates an L2 Multicast Interface List.
*
*
* \param [out] ifListId  L2 Multicast Interface List Id Pointer.
* \param [in] vlanId    VLAN Id to be used for the L2 Interface
*        List.
* \return XP_STATUS Operation status
*/
XP_STATUS xpsMulticastCreateL2InterfaceList(xpVlan_t vlanId,
                                            xpsMcL2InterfaceListId_t *ifListId);

/**
* \brief Creates an L3 Multicast Interface List.
*
* \param [in] ifListId  L3 Multicast Interface List Id Pointer.
* \return XP_STATUS
*/
XP_STATUS xpsMulticastCreateL3InterfaceList(xpsMcL3InterfaceListId_t *ifListId);

/**
* \public
* \brief Destroys an L2 Multicast Interface List.
*
* \param [in] ifListId  L2 Multicast Interface List Id.
* \return XP_STATUS
*/
XP_STATUS xpsMulticastDestroyL2InterfaceList(xpsMcL2InterfaceListId_t ifListId);

/**
* \brief Destroys an L3 Multicast Interface List.
*
*
* \param [in] ifListId  L3 Multicast Interface List Id.
* \return XP_STATUS
*/
XP_STATUS xpsMulticastDestroyL3InterfaceList(xpsMcL3InterfaceListId_t ifListId);

/** \public
* \brief Get multicast VIF used for an L2 interface list
*
* This method returns a VIF_ID that is used for multicast
* transmission from an interface list
*
* \param [in]  ifListId    interface list id
* \param [in]  encapType   encapsulation type
* \param [out] mcVif       multicast VIF_ID
* \return XP_STATUS
*/
XP_STATUS xpsMulticastGetVifFromL2InterfaceList(xpsMcL2InterfaceListId_t
                                                ifListId, xpL2EncapType_e encapType, xpVif_t *mcVif);

/**
* \public
* \brief Get multicast VIF used for an L3 interface list
*
* This method returns a VIF_ID that is used for multicast
* transmission from an interface list from l3 interface
*
* \param [in]  ifListId    interface list id
* \param [in]  l3IntfId    id of outgoing L3 interface in list
* \param [in]  encapType   encapsulation type
* \param [out] mcVif       multicast VIF_ID
* \return XP_STATUS
*/
XP_STATUS xpsMulticastGetVifFromL3InterfaceList(xpsMcL3InterfaceListId_t
                                                ifListId, xpsInterfaceId_t l3IntfId, xpL3EncapType_e encapType, xpVif_t *mcVif);

/**
 * \public
 * \brief Allocates a Mcast L3 Domain Id for IPV6 Multicast. It would be
 * a number between 0-127. This would be allocated per Multicast enabled VRF
 * \param [out] mcL3DomainId
 * \return XP_STATUS Operation Status
 *
 */
XP_STATUS xpsMulticastAllocateIpv6L3DomainId(xpsMcastDomainId_t *mcL3DomainId);

/**
 * \public
 * \brief Release a Mcast L3 Domain Id for IPV6 Mcast. It would be a number
 * between 0-127. This is allocated per Multicast Enabled VRF
 * \param [in] mcL3DomainId
 * \return XP_STATUS Operation status
 *
 */
XP_STATUS xpsMulticastReleaseIpv6L3DomainId(xpsMcastDomainId_t mcL3DomainId);

/**
 * \public
 * \brief Destroy a Mcast L3 Domain Id for IPV6 Mcast.
 * \param [in] vrfId
 * \return XP_STATUS Operation status
 *
 */
XP_STATUS xpsMcDestroyL3DomainId(uint32_t vrfId);

/**
* \public
* \brief Inserts an IPv4 Multicast Route Control entry in the specified
*        device. This method inserts an IPv4 Multicast Route Controlentry in the
*        specified device.
*
*  \param [in] devId          Device Id of the device in which the
*                             entry needs to be inserted.
*  \param [in] vrfId          vrf Index
*  \param [in] groupAddress   IPv4 group address
*  \param [in] reasonCode     reason code
*  \param [in] pktCmd         packet command only TRAP/DROP supported
*  \param [out] indexList     Index at which the entry is written to hardware is at indexList->index[0]. Indices of entry which are rehashed is present at indexList->index[1], indexList->index[2]... so on
*
*  \return XP_STATUS
*/
XP_STATUS xpsMulticastAddIpv4RouteControlEntry(xpsDevice_t devId,
                                               uint32_t vrfId, ipv4Addr_t groupAddress, uint32_t reasonCode,
                                               xpsMulticastPktCmdType_e pktCmd, xpsHashIndexList_t *indexList);

/**
* \brief This method get the Reason code for IPv4 Multicast Route Control Entry from a
*        specified device
*
* \param [in] devId     Device Id of device
* \param [in] index     Index of the entry to get reason code
* \param [out] reasonCode  Reason code
* \return XP_STATUS
*/
XP_STATUS xpsMulticastGetIpv4RouteControlEntryReasonCode(xpDevice_t devId,
                                                         uint32_t index, uint32_t *reasonCode);

/**
* \public
* \brief Inserts an IPv6 Multicast Route Control entry in the specified
*        device. This method inserts an IPv4 Multicast Route Controlentry in the
*        specified device.
*
*  \param [in] devId             Device Id of the device in which the
*                                entry needs to be inserted.
*  \param [in] ipv6McL3DomainId  IPv6 multicast domain
*  \param [in] groupAddress      IPv6 group address
*  \param [in] reasonCode        reason code
*  \param [in] pktCmd            packet command only TRAP/DROP supported
*  \param [out] indexList        Index at which the entry is written to hardware is at indexList->index[0]. Indices of entry which are rehashed is present at indexList->index[1], indexList->index[2]... so on
*
*  \return XP_STATUS
*/
XP_STATUS xpsMulticastAddIpv6RouteControlEntry(xpsDevice_t devId,
                                               xpMcastDomainId_t ipv6McL3DomainId, ipv6Addr_t groupAddress,
                                               uint32_t  reasonCode, xpsMulticastPktCmdType_e pktCmd,
                                               xpsHashIndexList_t *indexList);

/**
* \brief This method get the Reason code for IPv6 Multicast Route Control Entry from a
*        specified device
*
* \param [in] devId        Device Id of device
* \param [in] index        Index of the entry to get reason code
* \param [out] reasonCode  Reason code
* \return XP_STATUS
*/
XP_STATUS xpsMulticastGetIpv6RouteControlEntryReasonCode(xpDevice_t devId,
                                                         uint32_t index, uint32_t *reasonCode);

/**
 * \brief This method sets rehash level for IPv4 Bridge table
 *
 * \param [in] devId
 * \param [in] numOfRehashLevels
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMulticastSetIpv4BridgeRehashLevel(xpDevice_t devId,
                                               uint8_t numOfRehashLevels);

/**
 * \brief This method gets rehash level for IPv4 Bridge table
 *
 * \param [in] devId
 * \param [out] numOfRehashLevels
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMulticastGetIpv4BridgeRehashLevel(xpDevice_t devId,
                                               uint8_t* numOfRehashLevels);

/**
 * \brief This method sets rehash level for IPv6 Bridge table
 *
 * \param [in] devId
 * \param [in] numOfRehashLevels
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMulticastSetIpv6BridgeRehashLevel(xpDevice_t devId,
                                               uint8_t numOfRehashLevels);

/**
 * \brief This method gets rehash level for IPv6 Bridge table
 *
 * \param [in] devId
 * \param [out] numOfRehashLevels
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMulticastGetIpv6BridgeRehashLevel(xpDevice_t devId,
                                               uint8_t* numOfRehashLevels);


/**
 * \brief This method sets rehash level for IPv4 Route table
 *
 * \param [in] devId
 * \param [in] numOfRehashLevels
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMulticastSetIpv4RouteRehashLevel(xpDevice_t devId,
                                              uint8_t numOfRehashLevels);

/**
 * \brief This method gets rehash level for IPv4 Route table
 *
 * \param [in] devId
 * \param [out] numOfRehashLevels
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMulticastGetIpv4RouteRehashLevel(xpDevice_t devId,
                                              uint8_t* numOfRehashLevels);

/**
 * \brief This method sets rehash level for IPv6 Route table
 *
 * \param [in] devId
 * \param [in] numOfRehashLevels
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMulticastSetIpv6RouteRehashLevel(xpDevice_t devId,
                                              uint8_t numOfRehashLevels);

/**
 * \brief This method gets rehash level for IPv6 Route table
 *
 * \param [in] devId
 * \param [out] numOfRehashLevels
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMulticastGetIpv6RouteRehashLevel(xpDevice_t devId,
                                              uint8_t* numOfRehashLevels);

/**
 * \brief This method sets rehash level for IPv4 PIMBiDirRPF table
 *
 * \param [in] devId
 * \param [in] numOfRehashLevels
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMulticastSetIpv4PIMBiDirRPFRehashLevel(xpDevice_t devId,
                                                    uint8_t numOfRehashLevels);

/**
 * \brief This method gets rehash level for IPv4 PIMBiDirRPF table
 *
 * \param [in] devId
 * \param [out] numOfRehashLevels
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMulticastGetIpv4PIMBiDirRPFRehashLevel(xpDevice_t devId,
                                                    uint8_t* numOfRehashLevels);

/**
 * \brief This method sets rehash level for IPv6 PIMBiDirRPF table
 *
 * \param [in] devId
 * \param [in] numOfRehashLevels
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMulticastSetIpv6PIMBiDirRPFRehashLevel(xpDevice_t devId,
                                                    uint8_t numOfRehashLevels);

/**
 * \brief This method gets rehash level for IPv6 PIMBiDirRPF table
 *
 * \param [in] devId
 * \param [out] numOfRehashLevels
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMulticastGetIpv6PIMBiDirRPFRehashLevel(xpDevice_t devId,
                                                    uint8_t* numOfRehashLevels);

/**
 * \brief This method gets Index of IPv4 Bridge table entry
 *
 * \param [in] devId   Device Id
 * \param [in] entry   Contains entry (key + data) to be looked for
 * \param [out] index  Index of found entry, or -1 if not found
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMulticastGetIPv4BridgeEntryIndex(xpsDevice_t devId,
                                              xpsMulticastIPv4BridgeEntry_t *entry, uint32_t *index);

/**
 * \brief This method gets Index of IPv6 Bridge table entry
 *
 * \param [in] devId   Device Id
 * \param [in] entry   Contains entry (key + data) to be looked for
 * \param [out] index  Index of found entry, or -1 if not found
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMulticastGetIPv6BridgeEntryIndex(xpsDevice_t devId,
                                              xpsMulticastIPv6BridgeEntry_t *entry, uint32_t *index);

/**
 * \brief This method gets Index of IPv4 Route table entry
 *
 * \param [in] devId   Device Id
 * \param [in] entry   Contains entry (key + data) to be looked for
 * \param [out] index  Index of found entry, or -1 if not found
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMulticastGetIPv4RouteEntryIndex(xpsDevice_t devId,
                                             xpsMulticastIPv4RouteEntry_t *entry, uint32_t *index);

/**
 * \brief This method gets Index of IPv6 Route table entry
 *
 * \param [in] devId   Device Id
 * \param [in] entry   Contains entry (key + data) to be looked for
 * \param [out] index  Index of found entry, or -1 if not found
 *
 * \return XP_STATUS
 */
XP_STATUS xpsMulticastGetIPv6RouteEntryIndex(xpsDevice_t devId,
                                             xpsMulticastIPv6RouteEntry_t *entry, uint32_t *index);

XP_STATUS xpsMulticastAllocateVidx(xpsScope_t scopeId, uint32_t *vidx);
XP_STATUS xpsMulticastReleaseVidx(xpsScope_t scopeId, uint32_t vidx);

XP_STATUS xpsMulticastIpMLLPairCreateFirstOnly(xpsDevice_t devId,
                                               xpsMcL2InterfaceListId_t ifListId,
                                               uint32_t xpsVidx);

XP_STATUS xpsL2McReleaseOldIpMllPairIdx(xpsDevice_t devId,
                                        xpsMcL2InterfaceListId_t prevBaseIdx);
XP_STATUS xpsMulticastIpRouteEntryCreate(xpsDevice_t devId,
                                         xpsMcL2InterfaceListId_t ifListId);

XP_STATUS xpsMcL2GetRouteEntryIdx(xpsDevice_t devId,
                                  xpsMcL2InterfaceListId_t l2IntfListId,
                                  uint32_t *idx);

XP_STATUS xpsMulticastAddRemoveLagPortToL2Mc(xpsDevice_t devId,
                                             xpsInterfaceId_t lagIntfId,
                                             xpsInterfaceId_t portIntfId, bool isDel);

XP_STATUS xpsL2McMLLPairWrite(xpsDevice_t devId,
                              void *ctx,
                              CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT format,
                              CPSS_INTERFACE_TYPE_ENT firstNodeEgrType,
                              CPSS_INTERFACE_TYPE_ENT secondNodeEgrType,
                              uint32_t firstNodeEgrId,
                              uint32_t secondNodeEgrId,
                              uint32_t firstNodeMeshId,
                              uint32_t secondNodeMeshId,
                              uint16_t mllPairIdx,
                              uint16_t mllPairNextIdx,
                              GT_BOOL isLast);
XP_STATUS xpsMcReleaseOldL2MllPairIdx(xpsDevice_t devId,
                                      uint32_t prevBaseIdx);


#ifdef __cplusplus
}
#endif

#endif  //_xpsMulticast_h_
