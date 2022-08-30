// xpsAppTnl.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef _xpsAppTnl_h_
#define _xpsAppTnl_h_

#include "xpsApp.h"
#include "xpsAppL2.h"
#include "xpsAppL3.h"
#include "xpsErspanGre.h"
#include "xpsVpnGre.h"
#include "xpAppUtil.h"
#include "xpsSr.h"

//Tunnel managers

typedef enum
{
    XPAPP_IPTNL_TYPE_VXLAN = 0,
    XPAPP_IPTNL_TYPE_NVGRE,
    XPAPP_IPTNL_TYPE_GRE,
    XPAPP_IPTNL_TYPE_IP_OVER_IP,
    XPAPP_IPTNL_TYPE_GENEVE,
    XPAPP_IPTNL_TYPE_VPN_GRE,
    XPAPP_IPTNL_TYPE_VPN_GRE_LOOSE,
    XPAPP_IPTNL_TYPE_GRE_ERSPAN2
} xpsAppIpTunnelType;

typedef struct xpsAppIpTunnelData
{
    uint32_t    tunnelId;
    uint8_t     dmac[XP_MAC_ADDR_LEN];
    uint8_t     smac;
    uint8_t     tagType;
    uint8_t     dip[XP_IPV4_ADDR_LEN];
    uint8_t     myVtepIp[XP_IPV4_ADDR_LEN];
    int         vni;
    uint16_t    vlan;
    int         udpPort;
    uint16_t    portId;
    uint16_t    bdId:12;    // Maximum 4k.
    uint8_t     isMcast;
    xpsAppIpTunnelType    tnlType;
    uint8_t     setBd;
    uint16_t    greProtocolId;
    uint16_t    tnlVlan:12;
    uint8_t     ipv4ARPBCCmd;
    uint8_t     ipv4RouteEn;
    uint8_t     ipv6RouteEn;
    uint8_t     ipv4BridgeMcMode;
    uint8_t     ipv6BridgeMcMode;
    uint8_t     ipv4RouteMcEn;
    uint8_t     ipv6RouteMcEn;
    uint8_t     mplsRouteEn;
    uint8_t     ipv4RouteMcMode;
    uint8_t     ipv6RouteMcMode;
    uint8_t     vrfId;
    uint8_t     unregMcCmd;
    uint8_t     bind2Vlan;
    uint32_t    bindVlan;
} xpsAppIpTunnelData;


typedef struct xpsAppTnlInfParams
{
    uint8_t     ipv4ARPBCCmd;
    uint8_t     ipv4RouteEn;
    uint8_t     ipv6RouteEn;
    uint8_t     ipv4BridgeMcMode;
    uint8_t     ipv6BridgeMcMode;
    uint8_t     ipv4RouteMcEn;
    uint8_t     ipv6RouteMcEn;
    uint8_t     mplsRouteEn;
    uint8_t     ipv4RouteMcMode;
    uint8_t     ipv6RouteMcMode;
    uint8_t     vrfId;
    uint8_t     unregMcCmd;
} xpsAppTnlInfParams;

typedef struct xpsAppPbbTunnelData
{
    uint32_t    tnlId;
    uint8_t     devBitMap[MAX_SYS_DEVICES/8 + 1];
    uint8_t     bDa[XP_MAC_ADDR_LEN];
    uint8_t     bSa[XP_MAC_ADDR_LEN];
    uint16_t    bTag:12;
    uint32_t    iSID:24;
    uint16_t    portId;
    uint8_t     setBd;
    uint16_t    bdId:12;
    uint8_t     bind2Vlan;
    uint16_t    bindVlan;
    uint32_t    tableEntryHandle;
    uint32_t    insrtId;
}xpsAppPbbTunnelData;

//compare function for mpls tunnel vif database
int compareMplsTnlVifMap(const void *l, const void *r);
uint8_t xpsAppConfigureIpTnnlData(xpDevice_t devNum, TunnelEntry *tunnelEntry);

uint8_t xpsAppConfigurePbbTunnelData(xpDevice_t devId, TunnelPBBEntry *tunnelPBBEntry, uint32_t *tunnelId);

XP_STATUS xpsAppConfigSrInterface(xpsDevice_t devId, SrhEntry *srhEntry, xpsInterfaceId_t *srIntf);
XP_STATUS xpsAppProgramSrhData(xpsDevice_t devId, uint8_t numSegment, xpsInterfaceId_t srIntf, uint32_t nhId);
XP_STATUS xpsAppConfigureLocalSidData(xpDevice_t devId, localSidEntry *sidEntry);
uint8_t xpsAppSetTnlInfParams(xpDevice_t devId, xpsInterfaceId_t intfId, xpsAppTnlInfParams * tnlInfParams);

#endif // _xpsAppTnl_h_


