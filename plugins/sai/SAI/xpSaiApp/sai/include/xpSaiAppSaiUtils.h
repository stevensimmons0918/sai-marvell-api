// xpSaiAppSaiUtils.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef _xpSaiAppSaiUtils_h_
#define _xpSaiAppSaiUtils_h_

#ifdef __cplusplus
extern "C" {
#endif
#include "sai.h"
#ifdef __cplusplus
}
#endif

#include <string.h>
#include <stdlib.h>
#include "xpSaiApp.h"

#define SAI_NEIGHBOR_ATTR_COUNT 2
#define SAI_NEXT_HOP_ATTR_COUNT 3
#define SAI_ROUTE_ATTR_COUNT    2 

uint8_t* xpSaiAppMacReverse(uint8_t* mac);
uint64_t xpSaiAppSaiGetiVrfOid(uint32_t l3Intf);
uint64_t xpSaiAppSaiGetVlanRifOid(uint32_t l3IntfId);

XP_STATUS xpSaiAppSaiConvertTaggingMode(xpsL2EncapType_e xps_tagging_mode, sai_vlan_tagging_mode_t* sai_tagging_mode);
XP_STATUS xpSaiAppSaiConvertStpState(xpVlanStgState_e xpsStpState, sai_stp_port_state_t* saiStpState);

XP_STATUS xpSaiAppSaiWriteIpv4RouteEntry(xpDevice_t devId, sai_object_id_t switchOid, Ipv4SaiRouteEntry* ipv4SaiRouteData, uint64_t nhId);
XP_STATUS xpSaiAppSaiWriteIpv6RouteEntry(xpDevice_t devId, sai_object_id_t switchOid, Ipv6SaiRouteEntry* ipv6SaiRouteData, uint64_t nhId);
XP_STATUS xpSaiAppSaiWriteIpv4HostEntry(xpDevice_t devId, sai_object_id_t switchOid, Ipv4SaiNeighborEntry* ipv4NeighborData);
XP_STATUS xpSaiAppSaiWriteIpv4NextHopEntry(xpDevice_t devId, sai_object_id_t switchOid, Ipv4SaiNhEntry* ipv4SaiNhData, uint64_t* nhId);
XP_STATUS xpSaiAppConfigureIpv4NextHopGroupEntry(xpDevice_t devId, sai_object_id_t switchOid, IpvxSaiNhGroupEntry *ipvxSaiNhGroupData, uint64_t *nhIdIp4, uint64_t *nhGrpIdIp4);
XP_STATUS xpSaiAppSaiWriteIpv6HostEntry(xpDevice_t devId, sai_object_id_t switchOid, Ipv6SaiNeighborEntry* ipv6NeighborData);
XP_STATUS xpSaiAppSaiWriteIpv6NextHopEntry(xpDevice_t devId, sai_object_id_t switchOid, Ipv6SaiNhEntry* ipv6SaiNhData, uint64_t* nhId);
XP_STATUS xpSaiAppPrintAllObjectAttributes(sai_object_id_t switch_id);
XP_STATUS xpSaiAppStpSetState(sai_object_id_t *stpPortOid, sai_object_id_t stpOid, sai_object_id_t saiPortOid, sai_stp_port_state_t saiStpState);
XP_STATUS xpSaiAppPortSetAttr(uint16_t portNum, xpDevice_t devId);
uint8_t* xpSaiAppIPv6Reverse(sai_ip6_t ipv6Addr);

#endif //_xpSaiAppSaiUtils_h_
