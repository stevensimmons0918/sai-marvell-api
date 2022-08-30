// xpSaiAppL3.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiAppL3_h_
#define _xpSaiAppL3_h_

#include "xpSaiApp.h"
#include "xpSaiAppL2.h"
#include "xpAppUtil.h"

//Layer 3 managers
#include "xpsL3.h"
#include "xpsScope.h"

//context for nh database
typedef struct xpSaiAppNhContextMap
{
    int key;
    uint32_t nhIdx;
    xpsL3NextHopEntry_t nh;
} xpSaiAppNhContextMap;


XP_STATUS xpSaiAppWriteIpv6HostEntry(xpDevice_t devId, sai_object_id_t switchOid, Ipv6HostEntry *ipv6HostData );
XP_STATUS xpSaiAppWriteIpv6RouteEntry(xpDevice_t devId, sai_object_id_t switchOid, Ipv6RouteEntry* ipv6RouteData);
XP_STATUS xpSaiAppWriteIpv4HostEntry(xpDevice_t devId, sai_object_id_t switchOid, Ipv4HostEntry *ipv4HostData );
XP_STATUS xpSaiAppWriteIpv4RouteEntry(xpDevice_t devId, sai_object_id_t switchOid, Ipv4RouteEntry *ipv4RouteData  );
XP_STATUS xpSaiAppPopulateNh(xpDevice_t devId, sai_object_id_t switchOid, IpxNhEntry* ipvxNhData);
XP_STATUS xpSaiAppWriteIpv4HostScopeEntry(xpDevice_t devId, sai_object_id_t switchOid, Ipv4HostScopeEntry *ipv4HostScopeData);
XP_STATUS xpSaiAppWriteIpv4RouteScopeEntry(xpDevice_t devId, sai_object_id_t switchOid, Ipv4RouteScopeEntry *ipv4RouteScopeData);
XP_STATUS xpSaiAppPopulateNhScope(xpDevice_t devId, sai_object_id_t switchOid, IpxNhScopeEntry* ipvxNhScopeData);
void* xpSaiAppGetNhContext(xpDevice_t devId, uint32_t nhIndex);
void xpSaiAppL3CleanUp(xpsDevice_t devId);
#endif // _xpSaiAppL3_h_

