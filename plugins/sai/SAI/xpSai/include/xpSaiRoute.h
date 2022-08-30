// xpSaiRoute.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiRoute_h_
#define _xpSaiRoute_h_

#include "xpSai.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct _xpSaiRouteAttributesT
{
    sai_packet_action_t packetAction;
    sai_attribute_value_t trapPriority;
    //Used nextHopId for nexthop,nexthop-group, routerIntfId(RIF) and cpuIntfId; Use the  OBJID_TYPE_CHECK to figure out actual type
    sai_attribute_value_t nextHopId;
} xpSaiRouteAttributesT;


/* Global variable DB Programming for HA */

typedef enum xpSaiRouteStaticDataType_e
{
    SAI_ROUTE_STATIC_VARIABLES, //SAI_ROUTE_GLOBAL_DB_KEY
} xpSaiRouteStaticDataType_e;

//Holds the installed default DropNextHop and Default TrapNextHop value.
typedef struct xpSaiRouteStaticDbEntry
{
    //Key
    xpSaiRouteStaticDataType_e staticDataType;

    //Data
    uint32_t gSaiDefaultDropNhId;
    uint32_t gSaiDefaultTrapNhId;
} xpSaiRouteStaticDbEntry;


typedef struct xpSaiRouteDbEntry_t
{
    //Sai Route-key
    sai_object_id_t switch_id;
    sai_object_id_t vr_id;
    sai_ip_prefix_t destination;

    //Sai Route-Data
    xpSaiRouteAttributesT routeAttr;
} xpSaiRouteDbEntry_t;

XP_STATUS xpSaiRouteApiInit(uint64_t flag,
                            const sai_service_method_table_t* adapHostServiceMethodTable);
XP_STATUS xpSaiRouteApiDeinit();
XP_STATUS xpSaiRouteInit(xpsDevice_t xpSaiDevId);
XP_STATUS xpSaiRouteDeinit(xpsDevice_t xpSaiDevId);
XP_STATUS xpSaiRouteDevInit(xpsDevice_t xpsDevId);
sai_status_t xpSaiConvertPrefix2IPv4Mask(uint32_t ipPrefix, sai_ip4_t* pMask);
sai_status_t xpSaiConvertPrefix2IPv6Mask(uint32_t ipPrefix, sai_ip6_t mask);
XP_STATUS xpSaiRouteGetStaticVariablesDb(xpSaiRouteStaticDbEntry **
                                         staticVarDbPtr);

#ifdef __cplusplus
}
#endif

#endif //_xpSaiRoute_h_
