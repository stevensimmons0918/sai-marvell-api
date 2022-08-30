// xpsAppL3.h

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#ifndef _xpsAppL3_h_
#define _xpsAppL3_h_

#include "xpsApp.h"
#include "xpsAppL2.h"
#include "xpAppUtil.h"
#include "xpsL3.h"

//context for nh database
typedef struct nhContextMap
{
    int key;
    uint32_t nhIdx;
    xpsL3NextHopEntry_t nh;
} nhContextMap;

XP_STATUS xpsAppWriteIpv6HostEntry(xpDevice_t devId, Ipv6HostEntry *ipv6HostData );
XP_STATUS xpsAppWriteIpv6RouteEntry(xpDevice_t devId, Ipv6RouteEntry* ipv6RouteData);
XP_STATUS xpsAppWriteIpv4HostEntry(xpDevice_t devId, Ipv4HostEntry *ipv4HostData );
XP_STATUS xpsAppWriteIpv4RouteEntry(xpDevice_t devId, Ipv4RouteEntry *ipv4RouteData  );
XP_STATUS xpsAppPopulateNh(xpDevice_t devId, IpxNhEntry* ipvxNhData);
XP_STATUS xpsAppWriteIpv4HostScopeEntry(xpDevice_t devId, Ipv4HostScopeEntry *ipv4HostScopeData);
XP_STATUS xpsAppWriteIpv4RouteScopeEntry(xpDevice_t devId, Ipv4RouteScopeEntry *ipv4RouteScopeData);
XP_STATUS xpsAppPopulateNhScope(xpDevice_t devId, IpxNhScopeEntry* ipvxNhScopeData);
XP_STATUS xpsAppWriteIpv4HostControl(xpDevice_t devId, Ipv4HostControlEntry *ipv4HostControlData);
XP_STATUS xpsAppWriteIpv6HostControl(xpDevice_t devId, Ipv6HostControlEntry *ipv6HostControlData);
void* xpsAppGetNhContext(xpDevice_t devId, uint32_t nhIndex);
#endif // _xpsAppL3_h_

