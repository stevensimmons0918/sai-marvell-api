// xpSaiNextHop.h

/*******************************************************************************
* copyright (c) 2021 marvell. all rights reserved. the following file is       *
* subject to the limited use license agreement by and between marvell and you, *
* your employer or other entity on behalf of whom you act. in the absence of   *
* such license agreement the following file is subject to marvell’s standard   *
* limited use license agreement.                                               *
********************************************************************************/

#ifndef _xpSaiNextHop_h_
#define _xpSaiNextHop_h_

#include "xpSai.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct _xpSaiNextHopAttributesT
{
    sai_attribute_value_t ip;
    sai_attribute_value_t routerInterfaceId;
    sai_attribute_value_t type;
    sai_attribute_value_t tunnelId;
    sai_attribute_value_t tunnelMac;
    sai_attribute_value_t tunnelVni;
} xpSaiNextHopAttributesT;

#define XPS_VERSION 3

/* key nhId */
typedef struct xpSaiNextHopEntry_t
{
    uint32_t   nhId;
    sai_ip_addr_family_t type;
    ipv4Addr_t ipv4Addr;
    ipv6Addr_t ipv6Addr;
    /* FIXME: vrfid is redundant as we have rif id. Remove it */
    uint32_t vrfId;
    sai_object_id_t rifId;
    uint32_t routeRefCount;
    uint32_t aclRefCount;
    sai_next_hop_type_t nhType; // For tunnel nexthop support

    /*
     * Workaround: FIXME:
     * xpsL3GetRouteNextHop is not returning proper results.
     * To fix it, it is required to create a DB in xps NH.
     * As a temporary workaround, caching the XPS nh entry
     * created during SAI NH creation.
     */
    xpsL3NextHopEntry_t xpsNhCache;
} xpSaiNextHopEntry_t;

XP_STATUS xpSaiNextHopInit(xpsDevice_t xpSaiDevId);
XP_STATUS xpSaiNextHopDeInit(xpsDevice_t xpSaiDevId);

XP_STATUS xpSaiNextHopApiInit(uint64_t flag,
                              const sai_service_method_table_t* adapHostServiceMethodTable);
XP_STATUS xpSaiNextHopApiDeinit();

sai_status_t xpSaiIsNextHopInUse(xpsDevice_t xpsDevId, uint32_t xpsNhId);
sai_status_t xpSaiConvertNextHopOid(sai_object_id_t next_hop_id,
                                    uint32_t* pNhId);
sai_status_t xpSaiConvertNexthopXpsId(uint32_t xpsNhId,
                                      sai_object_id_t *saiNhId);

/**
 * \brief API to increment Route reference count of a particular nhId in Sai DB
 *
 * \param [in] nhId
 * \param [in] isRouteNH
 *
 * \return SAI Status
 */
sai_status_t xpSaiIncrementNextHopRouteRefCount(uint32_t nhId, bool isRouteNH);

/**
 * \brief API to decrement Route reference count of a particular nhId in Sai DB
 *
 * \param [in] nhId
 * \param [in] isRouteNH
 *
 * \return SAI Status
 */
sai_status_t xpSaiDecrementNextHopRouteRefCount(uint32_t nhId, bool isRouteNH);

/*
 * Handles neighbor change notification.
 */
sai_status_t xpSaiNextHopHandleNeighborChangeNotification(
    const xpSaiNeighborChangeNotif_t *nbrNotif);

/* XPS programming wrappers */
sai_status_t xpSaiNextHopProgramXpsNextHop(uint32_t nhId,
                                           xpsL3NextHopEntry_t *xpsNhEntry);
sai_status_t xpSaiNextHopGetXpsNextHop(uint32_t nhId,
                                       xpsL3NextHopEntry_t *xpsNhEntry);

sai_status_t xpSaiMaxCountNextHopAttribute(uint32_t *count);
sai_status_t xpSaiCountNextHopObjects(uint32_t *count);
sai_status_t xpSaiGetNextHopObjectList(uint32_t *object_count,
                                       sai_object_key_t *object_list);
sai_status_t xpSaiBulkGetNextHopAttributes(sai_object_id_t id,
                                           uint32_t *attr_count, sai_attribute_t *attr_list);
sai_status_t xpSaiIsAclNextHopInUse(xpsDevice_t xpsDevId, uint32_t xpsNhId);

#ifdef __cplusplus
}
#endif

#endif //_xpSaiNextHop_h_
