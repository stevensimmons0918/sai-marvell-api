//  xpSaiAppTnl.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef _xpSaiAppTnl_h_
#define _xpSaiAppTnl_h_

#include "xpSaiApp.h"
#include "xpSaiAppL2.h"
#include "xpSaiAppL3.h"
#include "xpsErspanGre.h"
#include "xpAppUtil.h"

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
} xpSaiAppIpTunnelType;

typedef struct xpSaiAppIpTunnelData
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
    xpSaiAppIpTunnelType    tnlType;
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
} xpSaiAppIpTunnelData;


typedef struct xpSaiAppTnlInfParams
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
} xpSaiAppTnlInfParams;


//compare function for mpls tunnel vif database
int compareMplsTnlVifMap(const void *l, const void *r);
XP_STATUS xpSaiAppConfigureIpTnnlData(xpDevice_t devNum, sai_object_id_t switchOid, TunnelEntry *tunnelEntry);

XP_STATUS xpSaiAppConfigurePbbTunnelData(xpDevice_t devId, TunnelPBBEntry *tunnelPBBEntry, uint32_t *tunnelId);

XP_STATUS xpSaiAppSetTnlInfParams(xpDevice_t devId, xpsInterfaceId_t intfId, xpSaiAppTnlInfParams * tnlInfParams);

#endif // _xpSaiAppTnl_h_


