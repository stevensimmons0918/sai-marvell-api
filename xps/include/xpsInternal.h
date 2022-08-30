// xpsInternal.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
 * \file xpsInternal.h
 * \brief This file contains API prototypes and type definitions
 *        for use internally by the XPS layer
 */

#ifndef _xpsInternal_h_
#define _xpsInternal_h_

#include "xpsMirror.h"
#include "xpsSal.h"
#include "xpsCommon.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "cpssHalDevice.h"

//96k ARP Entries, 16 tunnel starts entries. Note: Tunnel Start is not supported.
//Reserving only 8 TS 4/6 for ERSPAN
//#define XP_ROUTER_ARP_DA_AND_TUNNEL_START_MAX_IDS                 ((96*1024)-32)//(0x10000)
#define XP_ROUTER_ARP_DA_AND_TUNNEL_START_MAX_IDS(devId)    cpssHalGetSKUmaxNeighbours(devId)

/*Ideally, IP neighbour can be programmed to Hash and/or LPM.
The max Neighbour entry supported in system depends on these 2 tables,
and uniqueness depends of neigh on ARP. So, for a unique neighbour,
this should be minimum of (FDB table size/LPM , ARP table).
This value should be derived from profile, or user configured
For now using same as ARP*/
#define XP_ROUTER_NEIGHBOUR_ENTRY_MAX(devId) cpssHalGetSKUmaxNeighbours(devId)

/*in the HW, all tables having ARP pointer uses 0 as preset value
which is valid address. On accessing ARP through such preset can
overwrite a valid instance in ARP table*/
#define XP_ROUTER_ARP_DA_AND_TUNNEL_START_RANGE_START             1

//TODO : Following hardcoding need to be in a profile construct.
/* NH ID 0(UC) and 1(MC) is default for HW. In case of (S,G) MC route, implicit (*,G)
   is added pointing to routeBaseIndex(NH) 1.Hence start the range form value 2.
 */
#define XPS_l3_DEFAULT_UC_NEXTHOP           0
#define XPS_l3_DEFAULT_MC_NEXTHOP           1
#define XPS_l3_NEXTHOP_RANGE_START          2

#define XPS_L3_MAC2ME_MAX_IDS(devId)              (cpssHalGetSKUmaxMac2Me(devId))
#define XPS_L3_MAC2ME_RANGE_START           0

#define XPS_L3_ECMP_TBL_RANGE_START         (0)

#define XPS_L3_NH_GRP_MAX_NEXTHOPS_PER_GRP(devId)  (cpssHalGetSKUmaxNhEcmpMbrPerGrp(devId))
#define XPS_L3_NH_GRP_RANGE_START           (0)

#define XPS_L3_PBR_LEAF_RANGE_START           0

/**
 * \def XPS_VLAN_MAP_TO_BD(vlanId)
 *
 * This Macro will convert vlanId to bdId
 * As of now vlan = bdId; Still keeping the mapping centralised
 */
//#define XPS_VLAN_MAP_TO_BD(vlanId) (vlanId)

/**
 * \def XPS_BD_MAP_TO_VLAN(bdId)
 *
 * This Macro will convert bdId to vlanId
 */
//#define XPS_BD_MAP_TO_VLAN(bdId) (bdId)

#define XPS_ADD_DEVICE_TO_MAP(map, devId) (map.bits |= (1 << devId))

#define XPS_REMOVE_DEVICE_FROM_MAP(map, devId) (map.bits &= ~(1 << devId))

#define XPS_L3_RESERVED_ROUTER_VLAN(devId)     ((cpssHalGetSKUMaxVlanIds(devId)/2))
#define XPS_L3_DEFAULT_VRF              (0)
#define XPS_MAX_USER_DEFINED_VLANS(devId) (XPS_L3_RESERVED_ROUTER_VLAN(devId))


#define XPS_L2_ECMP_EPORT_RANGE_START 512
#define XPS_L2_ECMP_EPORT_MAX_NUM     3584


#define XPS_LTT_ECMP_RANGE_START 0
#define XPS_LTT_ECMP_MAX_NUM     8199 /*16K - > 0 to max for L2ECMP and rest for LAG*/

#define XPS_GLOBAL_EPORT_RANGE_START     4096
#define XPS_GLOBAL_EPORT_MAX_NUM         4096
#define XPS_VXLAN_UNI_HW_MAX_IDS           1024
#define XPS_VXLAN_UNI_TERM_HW_RANGE_START 0

#define IS_EPORT(port) (port >= XPS_L2_ECMP_EPORT_RANGE_START && \
                        port < (XPS_GLOBAL_EPORT_RANGE_START+XPS_GLOBAL_EPORT_MAX_NUM))



/**
 * \private Port APIS
 * \brief Init API for portDb
 *
 * This API will create and initialize the per device port
 * database in a scope, that will be used to maintain the portNum to
 * interfaceId relationship
 *
 * \param [in] scopeId
 *
 * \return int
 */
XP_STATUS xpsPortDbInitScope(xpsScope_t scopeId);

/**
 * \brief De-Init API for portDb
 *
 * De-Init API for per device portDb in a scope. This API will destroy the
 * portNum to interfaceId database.
 *
 *
 * \param [in] scopeId
 *
 * \return int
 */
XP_STATUS xpsPortDbDeInitScope(xpsScope_t scopeId);


//Private vlan utility APIs
XP_STATUS xpsPVlanGetType(xpsDevice_t devId, xpsVlan_t vlanId,
                          xpsPrivateVlanType_e *vlanType);

//Vlan utility APIs
XP_STATUS xpsVlanIsExist(xpsScope_t scopeId, xpsVlan_t vlanId);

/**
 * \brief get endpoint Info of vlan
 *
 * \param [in] scopeId
 * \param [in] vlanId
 * \param [in] intfId
 * \param [out] tagType
 * \param [out] data
 * \returns XP_STATUS
 */
XP_STATUS xpsVlanGetEndpointInfo(xpsScope_t scopeId, xpsVlan_t vlanId,
                                 xpsInterfaceId_t intfId, xpsL2EncapType_e *tagType, uint32_t *data);
XP_STATUS xpsVlanGetIntfTagType(xpsVlan_t vlanId, xpsInterfaceId_t intfId,
                                xpsL2EncapType_e *tagType);
XP_STATUS xpsVlanGetIntfTagTypeScope(xpsScope_t scopeId, xpsVlan_t vlanId,
                                     xpsInterfaceId_t intfId, xpsL2EncapType_e *tagType);

XP_STATUS xpsVlanIsFdbLimitReached(xpsDevice_t devId, xpsVlan_t vlanId,
                                   uint8_t *isFdbLimitReached);
XP_STATUS xpsVlanGetVlanMulticastVIF(xpsScope_t scopeId, xpsVlan_t vlanId,
                                     xpL2EncapType_e encapType, xpVif_t *mcVif);
XP_STATUS xpsVlanHandlePortVlanTableRehash(xpsDevice_t devId,
                                           xpsHashIndexList_t *indexList);

XP_STATUS xpsStpIsExist(xpsScope_t scopeId, xpsStp_t stpId);

XP_STATUS xpsStgGetIntfStateDb(xpsScope_t scopeId, xpsStp_t stgId,
                               xpsInterfaceId_t intfId, xpsStpState_e *stpState);


//Internal APIs. Required by Lag manager. Not to be directly used by User.

/**
 *
 * \brief Ip Tunnel Remove
 *
 * \param [in] devId
 * \param [in] type
 * \param [in] tnlIntfId
 * \returns XP_STATUS
 *
 */
XP_STATUS xpIpTunnelRemove(xpsDevice_t devId, xpIpTunnelType_t type,
                           xpsInterfaceId_t tnlIntfId);

/**
 *
 * \brief IpTunnel Get Config
 *
 * \param [in] devId
 * \param [in] tnlIntfId
 * \param [in] type
 * \param [out] cfg
 * \returns XP_STATUS
 *
 */
//XP_STATUS xpsIpTunnelGblDbGetData (xpsScope_t scopeId, xpsInterfaceId_t intfId, xpsIpTunnelData_t *data);
XP_STATUS xpsIpTunnelDbGetTunnelTermIndex(xpsDevice_t devId,
                                          xpsInterfaceId_t intfId, uint32_t *index);
XP_STATUS xpsTunnelHandleTunnelIvifTableRehash(xpsDevice_t devId,
                                               xpsHashIndexList_t *indexList);


#if 0
XP_STATUS xpsPbbTunnelCreate(xpsScope_t scopeId, xpsInterfaceType_e type,
                             xpsPbbTunnelData_t *data, xpsInterfaceId_t *tnlIntfId);
XP_STATUS xpsPbbTunnelAddOrigination(xpsDevice_t devId,
                                     xpsInterfaceId_t pbbTnlId, uint32_t insertionId, xpsPbbTunnelData_t *tnlData);
XP_STATUS xpsPbbAddTunnelTerminationEntry(xpsDevice_t devId,
                                          xpsInterfaceId_t intfId, xpsPbbTunnelData_t *tnlData);
XP_STATUS xpsPbbTunnelAddLocalEntry(xpsDevice_t devId, xpsInterfaceId_t intfId,
                                    xpsPbbTunnelData_t *tnlData);
XP_STATUS xpsPbbTunnelDelete(xpsScope_t scopeId, xpsInterfaceId_t tnlIntfId);
XP_STATUS xpsPbbTunnelAdd(xpsDevice_t devId, xpsInterfaceId_t intfId);
XP_STATUS xpPbbTunnelRemove(xpsDevice_t devId, xpsInterfaceId_t tnlIntfId);
#endif
//Lag utility APIs

/**
 * \brief API to update L3 interface information to a LAG db in a scope
 *
 * This API updates L3 interface to  LAG interface DB.
 * Pass scopeId as XP_SCOPE_DEFAULT, if scope is not required.
 *
 * \param [in] scopeId
 * \param [in] lagIntf
 * \param [in] l3IntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagSetL3IntfToDbScope(xpsScope_t scopeId, xpsInterfaceId_t lagIntf,
                                   xpsInterfaceId_t l3IntfId);

/**
 * \brief API to Get L3 interface information from a LAG db in a scope
 *
 * This API gets L3 interface from  LAG interface DB.
 * Pass scopeId as XP_SCOPE_DEFAULT, if scope is not required.
 *
 * \param [in] scopeId
 * \param [in] lagIntf
 * \param [out] *l3IntfId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsLagGetL3IntfFromDbScope(xpsScope_t scopeId,
                                     xpsInterfaceId_t lagIntf, xpsInterfaceId_t *l3IntfId);

XP_STATUS xpsStpUpdLagMember(xpsDevice_t devId, xpsInterfaceId_t lagIntf,
                             xpsInterfaceId_t iface, bool removed);

XP_STATUS xpsStpSetPortStpState(xpsDevice_t devId, xpsInterfaceId_t iface,
                                xpVlanStgState_e stgState);

XP_STATUS xpsL3UpdateLagMember(xpsDevice_t devId, xpsInterfaceId_t lagIntfId,
                               xpsInterfaceId_t portIntfId, bool add);

/**
 * \brief Get the Nexthop group count supported
 */
XP_STATUS xpsL3GetNexthopGroupCount(xpsDevice_t devId, uint32_t *count);

/**
 * \brief Internal API to clear internal nexthop entry which is part of a ECMP.
 *
 * \param [in] devId
 * \param [in] nhId
 *
 * \return XP_STATUS
 */
XP_STATUS xpsL3ClearEcmpRouteNextHop(xpsDevice_t devId, uint32_t nhId);
#ifdef __cplusplus
}
#endif


#endif  //_xpsInternal_h_
