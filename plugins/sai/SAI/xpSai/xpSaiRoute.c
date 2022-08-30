// xpSaiRoute.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiRoute.h"
#include "xpSaiVrf.h"
#include "xpSaiValidationArrays.h"
#include "xpsBitVector.h"
XP_SAI_LOG_REGISTER_API(SAI_API_ROUTE);

static sai_route_api_t* _xpSaiRouteApi;
static xpsDbHandle_t saiRouteStaticDataDbHndl = XPS_STATE_INVALID_DB_HANDLE;
static xpsDbHandle_t saiRouteDataDbHndl = XPS_STATE_INVALID_DB_HANDLE;

#define SAI_ROUTE_DEFAULT_TRAP_PRIORITY 0
#define XP_SAI_DEFAULT_ROUTE 0
#define SAI_ROUTE_BULK_PROCESS(func_with_args, obj_count, statuses, mode, anyFailed) \
{ \
    uint32_t count = 0; \
    uint32_t idx   = 0; \
\
    for(count = 0; count < obj_count; count++) \
    { \
        statuses[count] = func_with_args; \
        if(statuses[count] != SAI_STATUS_SUCCESS) \
        { \
            XP_SAI_LOG_ERR("Route API call failed for index %d with error code: %d!\n", count, statuses[count]); \
\
            if(!anyFailed) \
            { \
                anyFailed = true; \
            } \
\
            if(mode == SAI_BULK_OP_ERROR_MODE_STOP_ON_ERROR) \
            { \
                for(idx = (count + 1); idx < obj_count; idx++) \
                { \
                    statuses[idx] = SAI_STATUS_NOT_EXECUTED; \
                } \
                break; \
            } \
        } \
    } \
}

static sai_status_t xpSaiConvertRoute(const sai_route_entry_t
                                      *unicast_route_entry, xpSaiRouteAttributesT* pAttributes,
                                      xpsL3RouteEntry_t* pL3RouteEntry);

//Static variables DB functions
XP_STATUS xpSaiRouteGetStaticVariablesDb(xpSaiRouteStaticDbEntry **
                                         staticVarDbPtr)
{
    XP_STATUS result = XP_NO_ERR;
    xpSaiRouteStaticDbEntry keyStaticVarDb;

    keyStaticVarDb.staticDataType = SAI_ROUTE_STATIC_VARIABLES;
    if ((result = xpsStateSearchData(XP_SCOPE_DEFAULT, saiRouteStaticDataDbHndl,
                                     (xpsDbKey_t)&keyStaticVarDb, (void**)staticVarDbPtr)) != XP_NO_ERR)
    {
        //LOGFN()
        return result;
    }

    if (!(*staticVarDbPtr))
    {
        //LOGFN()
        return XP_ERR_KEY_NOT_FOUND;
    }

    return result;
}

static XP_STATUS xpSaiRouteRemoveStaticVariablesDb(void)
{
    XP_STATUS result = XP_NO_ERR;
    xpSaiRouteStaticDbEntry * staticVarDb = NULL;
    xpSaiRouteStaticDbEntry keyStaticVarDb;

    if ((result = xpSaiRouteGetStaticVariablesDb(&staticVarDb)) != XP_NO_ERR)
    {
        //LOGFN()
        return result;
    }

    if ((result = xpsL3DestroyRouteNextHop(1,
                                           staticVarDb->gSaiDefaultDropNhId)) != XP_NO_ERR)
    {
        //LOGFN()
        return result;
    }

    if ((result = xpsL3DestroyRouteNextHop(1,
                                           staticVarDb->gSaiDefaultTrapNhId)) != XP_NO_ERR)
    {
        //LOGFN()
        return result;
    }

    keyStaticVarDb.staticDataType = SAI_ROUTE_STATIC_VARIABLES;
    if ((result = xpsStateDeleteData(XP_SCOPE_DEFAULT, saiRouteStaticDataDbHndl,
                                     (xpsDbKey_t)&keyStaticVarDb, (void**)&staticVarDb)) != XP_NO_ERR)
    {
        //LOGFN()
        return result;
    }

    if ((result = xpsStateHeapFree((void*)staticVarDb)) != XP_NO_ERR)
    {
        //LOGFN()
        return result;
    }

    return result;
}

static XP_STATUS xpSaiRouteInsertStaticVariablesDb(void)
{
    XP_STATUS retVal = XP_NO_ERR;
    uint32_t saiRouteDefaultDropNhId;
    uint32_t saiRouteDefaultTrapNhId;
    xpSaiRouteStaticDbEntry *staticVarDb;

    if ((retVal = xpsStateHeapMalloc(sizeof(xpSaiRouteStaticDbEntry),
                                     (void**)&staticVarDb)) != XP_NO_ERR)
    {
        //LOGFN()
        return retVal;
    }

    if (staticVarDb == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }

    memset(staticVarDb, 0, sizeof(xpSaiRouteStaticDbEntry));
    staticVarDb->staticDataType = SAI_ROUTE_STATIC_VARIABLES;

    if ((retVal = xpsL3CreateRouteNextHop(1,
                                          &saiRouteDefaultDropNhId)) != XP_NO_ERR)
    {
        //LOGFN()
        xpsStateHeapFree((void*)staticVarDb);
        return retVal;
    }

    if ((retVal =  xpsL3CreateRouteNextHop(1,
                                           &saiRouteDefaultTrapNhId)) != XP_NO_ERR)
    {
        //LOGFN()
        xpsStateHeapFree((void*)staticVarDb);
        return retVal;
    }

    staticVarDb->gSaiDefaultDropNhId = saiRouteDefaultDropNhId;
    staticVarDb->gSaiDefaultTrapNhId = saiRouteDefaultTrapNhId;

    // Insert the saiRoute static variables into the database
    if ((retVal = xpsStateInsertData(XP_SCOPE_DEFAULT, saiRouteStaticDataDbHndl,
                                     (void*)staticVarDb)) != XP_NO_ERR)
    {
        xpsStateHeapFree((void*)staticVarDb);
        staticVarDb = NULL;
        return retVal;
    }

    return retVal;
}

static XP_STATUS xpSaiRouteInsertRouteDB(const sai_route_entry_t
                                         *unicast_route_entry, xpSaiRouteAttributesT* pAttributes)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpSaiRouteDbEntry_t *saiRouteEntry = NULL;

    if ((retVal = (XP_STATUS)xpsStateHeapMalloc(sizeof(xpSaiRouteDbEntry_t),
                                                (void**)&saiRouteEntry)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateHeapMalloc failed: retVal %d \n", retVal);
        return retVal;
    }

    memset(saiRouteEntry, 0, sizeof(xpSaiRouteDbEntry_t));
    saiRouteEntry->switch_id = unicast_route_entry->switch_id;
    saiRouteEntry->vr_id = unicast_route_entry->vr_id;

    if (unicast_route_entry->destination.addr_family == SAI_IP_ADDR_FAMILY_IPV4)
    {
        saiRouteEntry->destination.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
        memcpy(&saiRouteEntry->destination.addr.ip4,
               &unicast_route_entry->destination.addr.ip4, sizeof(sai_ip4_t));
        memcpy(&saiRouteEntry->destination.mask.ip4,
               &unicast_route_entry->destination.mask.ip4, sizeof(sai_ip4_t));
    }
    else
    {
        saiRouteEntry->destination.addr_family = SAI_IP_ADDR_FAMILY_IPV6;
        memcpy(&saiRouteEntry->destination.addr.ip6,
               &unicast_route_entry->destination.addr.ip6, sizeof(sai_ip6_t));
        memcpy(&saiRouteEntry->destination.mask.ip6,
               &unicast_route_entry->destination.mask.ip6, sizeof(sai_ip6_t));
    }

    saiRouteEntry->routeAttr.packetAction = pAttributes->packetAction;
    saiRouteEntry->routeAttr.nextHopId = pAttributes->nextHopId;

    /* Insert the sai-Route Entry structure into the databse, using the IP-addr and vr_id as key */
    if ((retVal = xpsStateInsertData(XP_SCOPE_DEFAULT, saiRouteDataDbHndl,
                                     saiRouteEntry)) != XP_NO_ERR)
    {
        // Free Allocated Memory
        xpsStateHeapFree(saiRouteEntry);
        XP_SAI_LOG_ERR("Error in xpsStateInsertData an entry: error code: %d\n",
                       retVal);
        return retVal;
    }
    return retVal;
}

static XP_STATUS xpSaiRouteGetRouteDB(const sai_route_entry_t
                                      *unicast_route_entry, xpSaiRouteDbEntry_t ** RouteCtxPtr)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpSaiRouteDbEntry_t saiRouteKey;
    memset(&saiRouteKey, 0, sizeof(xpSaiRouteDbEntry_t));

    /*sai-Route Key*/
    saiRouteKey.switch_id = unicast_route_entry->switch_id;
    saiRouteKey.vr_id = unicast_route_entry->vr_id;

    if (unicast_route_entry->destination.addr_family == SAI_IP_ADDR_FAMILY_IPV4)
    {
        saiRouteKey.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
        memcpy(&saiRouteKey.destination.addr.ip4,
               &unicast_route_entry->destination.addr.ip4, sizeof(sai_ip4_t));
        memcpy(&saiRouteKey.destination.mask.ip4,
               &unicast_route_entry->destination.mask.ip4, sizeof(sai_ip4_t));
    }
    else
    {
        saiRouteKey.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV6;
        memcpy(&saiRouteKey.destination.addr.ip6,
               &unicast_route_entry->destination.addr.ip6, sizeof(sai_ip6_t));
        memcpy(&saiRouteKey.destination.mask.ip6,
               &unicast_route_entry->destination.mask.ip6, sizeof(sai_ip6_t));
    }

    retVal = xpsStateSearchData(XP_SCOPE_DEFAULT, saiRouteDataDbHndl,
                                (xpsDbKey_t)&saiRouteKey, (void **)RouteCtxPtr);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error in xpsStateSearchData an entry: error code: %d\n",
                       retVal);
        return retVal;
    }

    return retVal;
}

static XP_STATUS xpSaiRouteRemoveRouteDB(const sai_route_entry_t
                                         *unicast_route_entry)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpSaiRouteDbEntry_t saiRouteKey;
    xpSaiRouteDbEntry_t *saiRouteEntry = NULL;
    memset(&saiRouteKey, 0, sizeof(xpSaiRouteDbEntry_t));

    saiRouteKey.switch_id = unicast_route_entry->switch_id;
    saiRouteKey.vr_id = unicast_route_entry->vr_id;

    if (unicast_route_entry->destination.addr_family == SAI_IP_ADDR_FAMILY_IPV4)
    {
        saiRouteKey.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
        memcpy(&saiRouteKey.destination.addr.ip4,
               &unicast_route_entry->destination.addr.ip4, sizeof(sai_ip4_t));
        memcpy(&saiRouteKey.destination.mask.ip4,
               &unicast_route_entry->destination.mask.ip4, sizeof(sai_ip4_t));
    }
    else
    {
        saiRouteKey.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV6;
        memcpy(&saiRouteKey.destination.addr.ip6,
               &unicast_route_entry->destination.addr.ip6, sizeof(sai_ip6_t));
        memcpy(&saiRouteKey.destination.mask.ip6,
               &unicast_route_entry->destination.mask.ip6, sizeof(sai_ip6_t));
    }

    retVal = xpsStateDeleteData(XP_SCOPE_DEFAULT, saiRouteDataDbHndl,
                                (xpsDbKey_t)&saiRouteKey, (void **)&saiRouteEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to delete state route-entry from DB\n");
        return retVal;
    }

    // Free the memory allocated for the corresponding state
    if ((retVal = xpsStateHeapFree(saiRouteEntry)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to free Route entry");
        return retVal;
    }

    return retVal;
}

void xpSaiSetDefaultRouteAttributeVals(xpSaiRouteAttributesT* attributes)
{
    attributes->packetAction = SAI_PACKET_ACTION_FORWARD;
}

//Func: xpSaiUpdateRouteAttributeVals

static sai_status_t xpSaiUpdateRouteAttributeVals(const uint32_t attr_count,
                                                  const sai_attribute_t* attr_list, xpSaiRouteAttributesT* attributes)
{
    XP_SAI_LOG_DBG("Calling xpSaiUpdateRouteAttributeVals\n");

    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_ROUTE_ENTRY_ATTR_PACKET_ACTION:
                {
                    attributes->packetAction = (sai_packet_action_t)attr_list[count].value.s32;
                    break;
                }
            case SAI_ROUTE_ENTRY_ATTR_USER_TRAP_ID:
                {
                    attributes->trapPriority = attr_list[count].value;
                    break;
                }
            case SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID:
                {
                    if (XDK_SAI_OBJID_TYPE_CHECK(attr_list[count].value.oid,
                                                 SAI_OBJECT_TYPE_NEXT_HOP))
                    {
                        attributes->nextHopId = attr_list[count].value;
                    }
                    else if (XDK_SAI_OBJID_TYPE_CHECK(attr_list[count].value.oid,
                                                      SAI_OBJECT_TYPE_NEXT_HOP_GROUP))
                    {
                        attributes->nextHopId = attr_list[count].value;
                    }
                    else if (XDK_SAI_OBJID_TYPE_CHECK(attr_list[count].value.oid,
                                                      SAI_OBJECT_TYPE_ROUTER_INTERFACE))
                    {
                        attributes->nextHopId = attr_list[count].value;
                    }
                    else if (XDK_SAI_OBJID_TYPE_CHECK(attr_list[count].value.oid,
                                                      SAI_OBJECT_TYPE_PORT))
                    {
                        attributes->nextHopId = attr_list[count].value;
                    }
                    else
                    {
                        XP_SAI_LOG_ERR("Unsupported next hop type %u\n",
                                       xpSaiObjIdTypeGet(attr_list[count].value.oid));
                        return SAI_STATUS_INVALID_ATTR_VALUE_0;
                    }
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to set %d\n", attr_list[count].id);
                    return SAI_STATUS_UNKNOWN_ATTRIBUTE_0;
                }
        }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetRouteAttrPacketAction

sai_status_t xpSaiSetRouteAttrPacketAction(const sai_route_entry_t
                                           *unicast_route_entry,  sai_attribute_value_t value,
                                           xpsL3RouteBulkInfo_t *routeInfo)
{
    XP_STATUS                   retVal          = XP_NO_ERR;
    sai_status_t                saiRetVal       = SAI_STATUS_FAILURE;
    xpsL3RouteEntry_t           l3RouteEntry;
    xpsDevice_t                 xpsDevId        = xpSaiGetDevId();
    xpSaiRouteStaticDbEntry     *staticVarDb    = NULL;
    uint32_t                    prefixIndex     = 0xFFFFFFFF;
    xpSaiRouteDbEntry_t *saiRouteEntry = NULL;
    sai_packet_action_t sai_pkt_action_val;

    memset(&l3RouteEntry, 0, sizeof(l3RouteEntry));

    /*sai_route_entry_t is Key for sai-route DB Entry */
    retVal = xpSaiRouteGetRouteDB(unicast_route_entry, &saiRouteEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error in xpSaiRouteGetRouteDB an entry: error code: %d\n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (NULL == saiRouteEntry)
    {
        XP_SAI_LOG_ERR("xpsRBTSearchNode Failed\n");
        return xpsStatus2SaiStatus(XP_ERR_KEY_NOT_FOUND);

    }

    XP_SAI_LOG_DBG("xpsRBTSearchNode Success:\n");
    sai_pkt_action_val = saiRouteEntry->routeAttr.packetAction;
    saiRouteEntry->routeAttr.packetAction = (sai_packet_action_t)value.s32;

    saiRetVal = xpSaiConvertRoute(unicast_route_entry, NULL, &l3RouteEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertRoute failed, retVal : %d \n", saiRetVal);
        return saiRetVal;
    }

    retVal = xpsL3FindIpRouteEntry(xpsDevId, &l3RouteEntry, &prefixIndex);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3FindIpRouteEntry failed, retVal : %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if ((retVal = xpSaiRouteGetStaticVariablesDb(&staticVarDb)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Preinstalled next hops are not ready.\n");
        return xpsStatus2SaiStatus(retVal);
    }

    if (value.s32 == SAI_PACKET_ACTION_DROP)
    {
        l3RouteEntry.nhId = staticVarDb->gSaiDefaultDropNhId;
        l3RouteEntry.nhEcmpSize = 1;
    }

    if (value.s32 == SAI_PACKET_ACTION_TRAP)
    {
        l3RouteEntry.nhId = staticVarDb->gSaiDefaultTrapNhId;
        l3RouteEntry.nhEcmpSize = 1;
    }

    if ((value.s32 == SAI_PACKET_ACTION_FORWARD) &&
        ((sai_pkt_action_val == SAI_PACKET_ACTION_TRAP) ||
         (sai_pkt_action_val == SAI_PACKET_ACTION_DROP)))
    {
        /*PKTCMD is FORWARD and Nh-id is valid nh-object; XPS NH USED: Derived value from valid-oid*/
        if (saiRouteEntry->routeAttr.nextHopId.oid)
        {
            if (XDK_SAI_OBJID_TYPE_CHECK(saiRouteEntry->routeAttr.nextHopId.oid,
                                         SAI_OBJECT_TYPE_NEXT_HOP))
            {
                l3RouteEntry.nhId = (uint32_t) xpSaiObjIdValueGet(
                                        saiRouteEntry->routeAttr.nextHopId.oid);
                l3RouteEntry.nhEcmpSize = 1;
            }
            else if (XDK_SAI_OBJID_TYPE_CHECK(saiRouteEntry->routeAttr.nextHopId.oid,
                                              SAI_OBJECT_TYPE_NEXT_HOP_GROUP))
            {
                l3RouteEntry.nhId = (uint32_t) xpSaiObjIdValueGet(
                                        saiRouteEntry->routeAttr.nextHopId.oid);
                retVal = xpsL3GetNextHopGroupMaxEcmpSize(&(l3RouteEntry.nhEcmpSize));
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_SWITCH_ATTR_ECMP_MEMBERS) | saiStatus : %d\n",
                                   xpsStatus2SaiStatus(retVal));
                    return xpsStatus2SaiStatus(retVal);
                }

            }
        }
        else
        {
            /*PKTCMD is FORWARD and Nh-id is NULL;  XPS NH USED: gSaiDefaultDropNhId value*/
            l3RouteEntry.nhId = staticVarDb->gSaiDefaultDropNhId;
            l3RouteEntry.nhEcmpSize = 1;
        }
    }

    retVal = xpsL3UpdateIpRouteEntry(xpsDevId, &l3RouteEntry, routeInfo);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3UpdateIpRouteEntry failed, retVal : %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    return xpsStatus2SaiStatus(retVal);
}


//Func: xpSaiSetRouteAttrTrapPriority

sai_status_t xpSaiSetRouteAttrTrapPriority(const sai_route_entry_t
                                           *unicast_route_entry,  sai_attribute_value_t value)
{
    // As of now, this attribute is not supported
    XP_SAI_LOG_DBG("Calling xpSaiSetRouteAttrTrapPriority\n");
    return SAI_STATUS_ATTR_NOT_SUPPORTED_0;
}


//Func: xpSaiSetRouteAttrNextHopId

sai_status_t xpSaiSetRouteAttrNextHopId(const sai_route_entry_t
                                        *unicast_route_entry,  sai_attribute_value_t value,
                                        xpsL3RouteBulkInfo_t *routeInfo)
{
    XP_STATUS    retVal        = XP_NO_ERR;
    sai_status_t saiRetVal     = SAI_STATUS_FAILURE;
    xpsL3RouteEntry_t l3RouteEntry;
    xpsDevice_t xpsDevId = xpSaiGetDevId();
    uint32_t prefixIndex = 0xFFFFFFFF;
    xpSaiRouteDbEntry_t *saiRouteEntry = NULL;
    xpSaiRouteStaticDbEntry *staticVarDb = NULL;
    uint32_t incNh= 0, decNh =
                        0; //NhIds/NhGrpIds that needs to updated when mapping between nhId/nhGrpId and Route changes.
    sai_object_id_t nhOid = 0;

    XP_SAI_LOG_DBG("enter xpSaiSetRouteAttrNextHopId nhId %" PRIx64 "\n",
                   value.oid);
    memset(&l3RouteEntry, 0, sizeof(l3RouteEntry));

    saiRetVal = xpSaiConvertRoute(unicast_route_entry, NULL, &l3RouteEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertRoute failed, retVal : %d \n", saiRetVal);
        return saiRetVal;
    }

    retVal = xpsL3FindIpRouteEntry(xpsDevId, &l3RouteEntry, &prefixIndex);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3FindIpRouteEntry failed, retVal : %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    /*sai_route_entry_t is Key for sai-route Entry */
    retVal = xpSaiRouteGetRouteDB(unicast_route_entry, &saiRouteEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error in xpSaiRouteGetRouteDB an entry: error code: %d\n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (NULL == saiRouteEntry)
    {
        XP_SAI_LOG_ERR("xpSaiRouteGetRouteDB Failed\n");
        return xpsStatus2SaiStatus(XP_ERR_KEY_NOT_FOUND);
    }

    XP_SAI_LOG_DBG("xpsRBTSearchNode Success\n");

    if ((retVal = xpSaiRouteGetStaticVariablesDb(&staticVarDb)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Preinstalled next hops are not ready.\n");
        return xpsStatus2SaiStatus(retVal);
    }

    if (saiRouteEntry->routeAttr.packetAction == SAI_PACKET_ACTION_DROP)
    {
        l3RouteEntry.nhId = staticVarDb->gSaiDefaultDropNhId;
        l3RouteEntry.nhEcmpSize = 1;
    }
    else if (saiRouteEntry->routeAttr.packetAction == SAI_PACKET_ACTION_TRAP)
    {
        l3RouteEntry.nhId = staticVarDb->gSaiDefaultTrapNhId;
        l3RouteEntry.nhEcmpSize = 1;
    }

    incNh = (uint32_t) xpSaiObjIdValueGet(value.oid);
    decNh = (uint32_t) xpSaiObjIdValueGet(saiRouteEntry->routeAttr.nextHopId.oid);
    nhOid = saiRouteEntry->routeAttr.nextHopId.oid;

    if (value.oid == SAI_NULL_OBJECT_ID)
    {
        l3RouteEntry.nhId = staticVarDb->gSaiDefaultDropNhId;
        l3RouteEntry.nhEcmpSize = 1;
        saiRouteEntry->routeAttr.nextHopId.oid = SAI_NULL_OBJECT_ID;
    }
    else if (value.oid &&
             XDK_SAI_OBJID_TYPE_CHECK(value.oid, SAI_OBJECT_TYPE_NEXT_HOP))
    {
        saiRouteEntry->routeAttr.nextHopId.oid = value.oid;
        if (saiRouteEntry->routeAttr.packetAction == SAI_PACKET_ACTION_FORWARD)
        {
            l3RouteEntry.nhId = (uint32_t) xpSaiObjIdValueGet(value.oid);
            l3RouteEntry.nhEcmpSize = 1;
        }
    }
    else if (value.oid &&
             XDK_SAI_OBJID_TYPE_CHECK(value.oid, SAI_OBJECT_TYPE_NEXT_HOP_GROUP))
    {
        saiRouteEntry->routeAttr.nextHopId.oid = value.oid;
        if (saiRouteEntry->routeAttr.packetAction == SAI_PACKET_ACTION_FORWARD)
        {
            l3RouteEntry.nhId = (uint32_t) xpSaiObjIdValueGet(value.oid);
            retVal = xpsL3GetNextHopGroupMaxEcmpSize(&(l3RouteEntry.nhEcmpSize));
            if (retVal != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Failed to get (SAI_SWITCH_ATTR_ECMP_MEMBERS) | saiStatus : %d\n",
                               xpsStatus2SaiStatus(retVal));
                return xpsStatus2SaiStatus(retVal);
            }
        }
    }
    else if (value.oid &&
             ((XDK_SAI_OBJID_TYPE_CHECK(value.oid,
                                        SAI_OBJECT_TYPE_ROUTER_INTERFACE) ||
               XDK_SAI_OBJID_TYPE_CHECK(value.oid, SAI_OBJECT_TYPE_PORT))))
    {
        saiRouteEntry->routeAttr.nextHopId.oid = value.oid;
        l3RouteEntry.nhId = staticVarDb->gSaiDefaultTrapNhId;
        l3RouteEntry.nhEcmpSize = 1;
    }
    else
    {
        XP_SAI_LOG_ERR("Unsupported next hop type %u\n", xpSaiObjIdTypeGet(value.oid));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpsL3UpdateIpRouteEntry(xpsDevId, &l3RouteEntry, routeInfo);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3UpdateIpRouteEntry failed, retVal : %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

#if 0
    /* As part of Default route, update Class E route. Set NH same as default route */
    if ((l3RouteEntry.type == XP_PREFIX_TYPE_IPV4) &&
        (unicast_route_entry->destination.addr.ip4 == XP_SAI_DEFAULT_ROUTE))
    {
        retVal = xpsL3WriteIpClassERouteEntry(xpsDevId, &l3RouteEntry, false);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsL3AddClassEIpRouteEntry failed, retVal : %d \n", retVal);
            /* Do not return error as this is implicit configuration */
            return xpsStatus2SaiStatus(retVal);
        }
    }
#endif
    if (value.oid &&
        (XDK_SAI_OBJID_TYPE_CHECK(value.oid, SAI_OBJECT_TYPE_NEXT_HOP)))
    {
        saiRetVal = xpSaiIncrementNextHopRouteRefCount(incNh, true);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiIncrementNextHopRefCount failed, retVal : %d \n",
                           saiRetVal);
            return saiRetVal;
        }
    }

    if (nhOid && (XDK_SAI_OBJID_TYPE_CHECK(nhOid, SAI_OBJECT_TYPE_NEXT_HOP)))
    {
        saiRetVal = xpSaiDecrementNextHopRouteRefCount(decNh, true);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiDecrementNextHopRefCount failed, retVal : %d \n",
                           saiRetVal);
            return saiRetVal;
        }
    }

    if (value.oid &&
        (XDK_SAI_OBJID_TYPE_CHECK(value.oid, SAI_OBJECT_TYPE_NEXT_HOP_GROUP)))
    {
        saiRetVal = xpSaiIncrementNextHopGroupRouteRefCount(incNh, true);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiIncrementNextHopGroupRouteRefCount failed, retVal : %d \n",
                           saiRetVal);
            return saiRetVal;
        }
    }

    if (nhOid && (XDK_SAI_OBJID_TYPE_CHECK(nhOid, SAI_OBJECT_TYPE_NEXT_HOP_GROUP)))
    {
        saiRetVal = xpSaiDecrementNextHopGroupRouteRefCount(decNh, true);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiDecrementNextHopGroupRouteRefCount failed, retVal : %d \n",
                           saiRetVal);
            return saiRetVal;
        }
    }

    XP_SAI_LOG_DBG("exit xpSaiSetRouteAttrNextHopId  \n");
    return xpsStatus2SaiStatus(retVal);
}

//Func: xpSaiConvertIpMask2Prefix

sai_status_t xpSaiConvertIpMask2Prefix(sai_ip_prefix_t destination,
                                       uint32_t* pIpPrefix)
{
    *pIpPrefix = 0;
    uint8_t i = 0;
    uint32_t ipV4Mask = destination.mask.ip4;

    if (destination.addr_family == SAI_IP_ADDR_FAMILY_IPV4)
    {
        while (ipV4Mask)
        {
            *pIpPrefix += (ipV4Mask & 0x01);
            ipV4Mask >>= 1;
        }

        XP_SAI_LOG_DBG("IPv4 prefix %d \n", *pIpPrefix);

        return SAI_STATUS_SUCCESS;
    }

    for (i = 0; i < sizeof(destination.mask.ip6); i++)
    {
        while (destination.mask.ip6[i])
        {
            *pIpPrefix += (destination.mask.ip6[i] & 0x01);
            destination.mask.ip6[i] >>= 1;
        }
    }

    XP_SAI_LOG_DBG("IPv6 prefix %d \n", *pIpPrefix);

    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiConvertPrefix2IPv4Mask

sai_status_t xpSaiConvertPrefix2IPv4Mask(uint32_t ipPrefix, sai_ip4_t* pMask)
{
    uint8_t  bitCnt      =  0;
    uint32_t zeroBitsCnt =  0;

    XP_SAI_LOG_DBG("Prefix %d \n", ipPrefix);

    *pMask = 0;
    zeroBitsCnt = 32 - ipPrefix;

    for (bitCnt = 0; bitCnt < zeroBitsCnt; bitCnt++)
    {
        *pMask <<= 1;
        *pMask = *pMask | 0x01;
    }

    *pMask = ~(*pMask);

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiConvertPrefix2IPv6Mask

sai_status_t xpSaiConvertPrefix2IPv6Mask(uint32_t ipPrefix, sai_ip6_t mask)
{
    uint8_t byteCnt           =   0;
    uint8_t bitCnt            =   0;
    uint8_t zeroBitsCnt       =   0;
    uint8_t zeroBytesCnt      =   0;
    uint8_t zeroBytesCntEx    =   0;
    uint8_t zeroBitsCntInByte =   0;
    uint8_t maskSize          =   16;

    XP_SAI_LOG_DBG("Prefix %d \n", ipPrefix);

    memset(mask, 0xFF, maskSize);

    zeroBitsCnt = maskSize * 8 - ipPrefix;
    zeroBytesCntEx = (zeroBitsCnt % 8) > 0 ? 1 : 0;
    zeroBytesCnt = zeroBitsCnt / 8 + zeroBytesCntEx;

    for (byteCnt = 0; byteCnt < zeroBytesCnt; byteCnt++)
    {
        zeroBitsCntInByte = MIN(zeroBitsCnt - byteCnt * 8, 8);

        for (bitCnt = 0; bitCnt < zeroBitsCntInByte; bitCnt++)
        {
            mask[byteCnt] <<= 1;
            mask[byteCnt] = mask[byteCnt] & 0xFE;
        }
    }

    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiConvertRoute

static sai_status_t xpSaiConvertRoute(const sai_route_entry_t
                                      *unicast_route_entry, xpSaiRouteAttributesT* pAttributes,
                                      xpsL3RouteEntry_t* pL3RouteEntry)
{
    sai_status_t saiRetVal = SAI_STATUS_FAILURE;
    xpSaiRouteStaticDbEntry* staticVarDb = NULL;
    XP_STATUS retVal = XP_NO_ERR;
    uint32_t ipV4Addr = 0;

    switch (unicast_route_entry->destination.addr_family)
    {
        case SAI_IP_ADDR_FAMILY_IPV4:
            {
                pL3RouteEntry->type = XP_PREFIX_TYPE_IPV4;

                ipV4Addr = unicast_route_entry->destination.addr.ip4;
                memcpy(pL3RouteEntry->ipv4Addr, &ipV4Addr, sizeof(pL3RouteEntry->ipv4Addr));

                saiRetVal = xpSaiConvertIpMask2Prefix(unicast_route_entry->destination,
                                                      &pL3RouteEntry->ipMaskLen);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("xpSaiConvertIpMask2Prefix failed, retVal : %d \n", saiRetVal);
                    return saiRetVal;
                }

                break;
            }

        case SAI_IP_ADDR_FAMILY_IPV6:
            {
                pL3RouteEntry->type = XP_PREFIX_TYPE_IPV6;

                memcpy(pL3RouteEntry->ipv6Addr, unicast_route_entry->destination.addr.ip6,
                       sizeof(pL3RouteEntry->ipv6Addr));

                saiRetVal = xpSaiConvertIpMask2Prefix(unicast_route_entry->destination,
                                                      &pL3RouteEntry->ipMaskLen);
                if (saiRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("xpSaiConvertIpMask2Prefix failed, retVal : %d \n", saiRetVal);
                    return saiRetVal;
                }

                break;
            }

        default:
            {
                XP_SAI_LOG_ERR("Unknown address family %d\n",
                               unicast_route_entry->destination.addr_family);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    if (unicast_route_entry->vr_id != SAI_NULL_OBJECT_ID)
    {
        saiRetVal = xpSaiConvertVirtualRouterOid(unicast_route_entry->vr_id,
                                                 &pL3RouteEntry->vrfId);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiConvertVirtualRouterOid() failed with error code: %d\n",
                           saiRetVal);
            return saiRetVal;
        }
    }
    else
    {
        XP_SAI_LOG_ERR("unicast_route_entry->vr_id is SAI_NULL_OBJECT_ID\n");
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if (pAttributes != NULL)
    {
        if ((retVal = xpSaiRouteGetStaticVariablesDb(&staticVarDb)) != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Preinstalled next hops are not ready.\n");
            return xpsStatus2SaiStatus(retVal);
        }

        if ((pAttributes->packetAction < SAI_PACKET_ACTION_DROP) ||
            (pAttributes->packetAction > SAI_PACKET_ACTION_TRANSIT))
        {
            XP_SAI_LOG_ERR("Invalid Packet_action value is received\n");
            return SAI_STATUS_INVALID_ATTR_VALUE_0;
        }

        if (pAttributes->packetAction == SAI_PACKET_ACTION_DROP)
        {
            XP_SAI_LOG_DBG("Drop action.\n");
            pL3RouteEntry->nhId = staticVarDb->gSaiDefaultDropNhId;
            pL3RouteEntry->nhEcmpSize = 1;
            return SAI_STATUS_SUCCESS;
        }

        if ((pAttributes->packetAction == SAI_PACKET_ACTION_TRAP) ||
            ((pAttributes->nextHopId.oid != SAI_NULL_OBJECT_ID) &&
             (XDK_SAI_OBJID_TYPE_CHECK(pAttributes->nextHopId.oid,
                                       SAI_OBJECT_TYPE_ROUTER_INTERFACE) ||
              XDK_SAI_OBJID_TYPE_CHECK(pAttributes->nextHopId.oid, SAI_OBJECT_TYPE_PORT))))
        {
            XP_SAI_LOG_DBG("Trap action.\n");
            pL3RouteEntry->nhId = staticVarDb->gSaiDefaultTrapNhId;
            pL3RouteEntry->nhEcmpSize = 1;
            return SAI_STATUS_SUCCESS;
        }

        /*PKTCMD is FORWARD and Nh-id is valid nh-object; XPS NH USED: Derived value from valid-oid*/
        if ((pAttributes->nextHopId.oid != SAI_NULL_OBJECT_ID) &&
            (pAttributes->packetAction == SAI_PACKET_ACTION_FORWARD))
        {
            if (XDK_SAI_OBJID_TYPE_CHECK(pAttributes->nextHopId.oid,
                                         SAI_OBJECT_TYPE_NEXT_HOP))
            {
                pL3RouteEntry->nhId = (uint32_t) xpSaiObjIdValueGet(pAttributes->nextHopId.oid);
                pL3RouteEntry->nhEcmpSize = 1;
            }
            else if (XDK_SAI_OBJID_TYPE_CHECK(pAttributes->nextHopId.oid,
                                              SAI_OBJECT_TYPE_NEXT_HOP_GROUP))
            {
                pL3RouteEntry->nhId = (uint32_t) xpSaiObjIdValueGet(pAttributes->nextHopId.oid);
                retVal = xpsL3GetNextHopGroupMaxEcmpSize(&(pL3RouteEntry->nhEcmpSize));
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Failed to get (SAI_SWITCH_ATTR_ECMP_MEMBERS) | saiStatus : %d\n",
                                   xpsStatus2SaiStatus(retVal));
                    return xpsStatus2SaiStatus(retVal);
                }
            }
        }
        else
        {
            pL3RouteEntry->nhId = staticVarDb->gSaiDefaultDropNhId;
            pL3RouteEntry->nhEcmpSize = 1;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiCreateRouteForBulk(const sai_route_entry_t
                                     *unicast_route_entry,
                                     uint32_t attr_count, const sai_attribute_t *attr_list,
                                     xpsL3RouteBulkInfo_t *routeInfo)
{
    XP_STATUS    retVal        = XP_NO_ERR;
    sai_status_t saiRetVal     = SAI_STATUS_FAILURE;
    uint32_t     routeObjectId = 0;
    xpsL3RouteEntry_t *l3RouteEntry = NULL;
    xpSaiRouteAttributesT *attributes = NULL;
    uint32_t prfxBucketIdx = 0;
    xpSaiRouteDbEntry_t *saiRouteEntry = NULL;
    uint32_t nhId, nhGrpId;
    xpSaiTableEntryCountDbEntry *entryCountCtxPtr = NULL;

    if (routeInfo == NULL)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsDevice_t xpsDevId = xpSaiGetDevId();

    if (unicast_route_entry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    XP_SAI_LOG_DBG("%s \n", __FUNCNAME__);

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               ROUTE_ENTRY_VALIDATION_ARRAY_SIZE, route_entry_attribs,
                               SAI_COMMON_API_CREATE);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    attributes = (xpSaiRouteAttributesT *)xpMalloc(sizeof(xpSaiRouteAttributesT));
    if (!attributes)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for attributes\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(attributes, 0, sizeof(xpSaiRouteAttributesT));

    xpSaiSetDefaultRouteAttributeVals(attributes);
    saiRetVal = xpSaiUpdateRouteAttributeVals(attr_count, attr_list, attributes);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiUpdateRouteAttributeVals failed, retVal : %d \n",
                       saiRetVal);
        xpFree(attributes);
        return saiRetVal;
    }

    if ((attributes->nextHopId.oid != SAI_NULL_OBJECT_ID) &&
        (XDK_SAI_OBJID_TYPE_CHECK(attributes->nextHopId.oid,
                                  SAI_OBJECT_TYPE_ROUTER_INTERFACE)))
    {
        if (xpSaiRouterInterfaceVirtualCheck(attributes->nextHopId.oid))
        {
            XP_SAI_LOG_ERR("Virtual RIF OID received\n");
            xpFree(attributes);
            return saiRetVal;
        }
    }

    l3RouteEntry = (xpsL3RouteEntry_t *)xpMalloc(sizeof(xpsL3RouteEntry_t));
    if (!l3RouteEntry)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for l3RouteEntry\n");
        xpFree(attributes);
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(l3RouteEntry, 0, sizeof(xpsL3RouteEntry_t));

    saiRetVal = xpSaiConvertRoute(unicast_route_entry, attributes, l3RouteEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertRoute failed, retVal : %d \n", saiRetVal);
        xpFree(l3RouteEntry);
        xpFree(attributes);
        return saiRetVal;
    }

    if (!xpSaiVrfExists(l3RouteEntry->vrfId))
    {
        XP_SAI_LOG_ERR("VRF does not exists, vrfId %d", l3RouteEntry->vrfId);
        xpFree(l3RouteEntry);
        xpFree(attributes);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (l3RouteEntry->ipMaskLen)
    {
        retVal = xpsL3FindIpRouteEntry(xpsDevId, l3RouteEntry, &prfxBucketIdx);
        if (retVal == XP_NO_ERR)
        {
            XP_SAI_LOG_WARNING("A route already exists. No action needed.");
            xpFree(l3RouteEntry);
            xpFree(attributes);
            return SAI_STATUS_ITEM_ALREADY_EXISTS;
        }
    }

    // Move this after configuring CPSS
    saiRetVal = xpSaiGetTableEntryCountCtxDb(xpsDevId, &entryCountCtxPtr);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get Table Entry Count DB, retVal : %d \n", retVal);
        xpFree(l3RouteEntry);
        xpFree(attributes);
        return saiRetVal;
    }

    if (l3RouteEntry->type == XP_PREFIX_TYPE_IPV4)
    {
        if (entryCountCtxPtr->max_ipv4RouteEntries <=
            entryCountCtxPtr->ipv4RouteEntries)
        {
            XP_SAI_LOG_ERR("Max IPv4 routes reached. retVal : %d \n", retVal);
            xpFree(l3RouteEntry);
            xpFree(attributes);
            return xpsStatus2SaiStatus(XP_ERR_OUT_OF_RANGE);
        }
    }
    else if (l3RouteEntry->type == XP_PREFIX_TYPE_IPV6)
    {
        if (entryCountCtxPtr->max_ipv6RouteEntries <=
            entryCountCtxPtr->ipv6RouteEntries)
        {
            XP_SAI_LOG_ERR("Max IPv6 routes reached. retVal : %d \n", retVal);
            xpFree(l3RouteEntry);
            xpFree(attributes);
            return xpsStatus2SaiStatus(XP_ERR_OUT_OF_RANGE);
        }
    }

    //program NH to HW now, if doesn't exist
    if (l3RouteEntry->nhEcmpSize == 1)
    {
        xpsL3NextHopEntry_t pL3NextHopEntry;
        saiRetVal = xpsL3GetRouteNextHopDb(xpsDevId, l3RouteEntry->nhId,
                                           &pL3NextHopEntry);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiCreateRoute xpsL3GetRouteNextHopDb() failed with error code: %d\n",
                           saiRetVal);
            xpFree(l3RouteEntry);
            xpFree(attributes);
            return saiRetVal;
        }
        retVal = xpsL3SetRouteNextHop(xpsDevId, l3RouteEntry->nhId, &pL3NextHopEntry);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiCreateRoute xpsL3SetRouteNextHop : error code: %d\n",
                           retVal);
            xpFree(l3RouteEntry);
            xpFree(attributes);
            return xpsStatus2SaiStatus(retVal);
        }
        XP_SAI_LOG_INFO("xpSaiCreateRoute NH %u prgm success\n", l3RouteEntry->nhId);
    }
    retVal = xpsL3AddIpRouteEntry(xpsDevId, l3RouteEntry, &routeObjectId,
                                  routeInfo);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3AddIpRouteEntry failed, retVal : %d \n", retVal);
        xpFree(l3RouteEntry);
        xpFree(attributes);
        return xpsStatus2SaiStatus(retVal);
    }
#if 0
    /* As part of Default route, update Class E route. Set NH same as default route */
    if ((l3RouteEntry->type == XP_PREFIX_TYPE_IPV4) &&
        (unicast_route_entry->destination.addr.ip4 == XP_SAI_DEFAULT_ROUTE))
    {
        retVal = xpsL3WriteIpClassERouteEntry(xpsDevId, l3RouteEntry, true);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsL3WriteClassEIpRouteEntry failed, retVal : %d \n", retVal);
            xpFree(l3RouteEntry);
            xpFree(attributes);
            return xpsStatus2SaiStatus(retVal);
        }
    }
#endif
    if (l3RouteEntry->type == XP_PREFIX_TYPE_IPV4)
    {
        entryCountCtxPtr->ipv4RouteEntries++;
    }
    if (l3RouteEntry->type == XP_PREFIX_TYPE_IPV6)
    {
        entryCountCtxPtr->ipv6RouteEntries++;
    }

    if ((attributes->nextHopId.oid != SAI_NULL_OBJECT_ID) &&
        (XDK_SAI_OBJID_TYPE_CHECK(attributes->nextHopId.oid, SAI_OBJECT_TYPE_NEXT_HOP)))
    {
        nhId = (uint32_t) xpSaiObjIdValueGet(attributes->nextHopId.oid);
        saiRetVal = xpSaiIncrementNextHopRouteRefCount(nhId, true);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiIncrementNextHopRouteRefCount failed, retVal : %d \n",
                           saiRetVal);
            xpFree(l3RouteEntry);
            xpFree(attributes);
            return saiRetVal;
        }
    }

    if ((attributes->nextHopId.oid != SAI_NULL_OBJECT_ID) &&
        (XDK_SAI_OBJID_TYPE_CHECK(attributes->nextHopId.oid,
                                  SAI_OBJECT_TYPE_NEXT_HOP_GROUP)))
    {
        nhGrpId = (uint32_t) xpSaiObjIdValueGet(attributes->nextHopId.oid);
        saiRetVal = xpSaiIncrementNextHopGroupRouteRefCount(nhGrpId, true);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiIncrementNextHopGroupRouteRefCount failed, retVal : %d \n",
                           saiRetVal);
            xpFree(l3RouteEntry);
            xpFree(attributes);
            return saiRetVal;
        }
    }

    retVal = xpSaiVrfRoutesNumInc(xpsDevId, l3RouteEntry->vrfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfRoutesNumInc failed, retVal : %d \n", retVal);
        xpFree(l3RouteEntry);
        xpFree(attributes);
        return xpsStatus2SaiStatus(retVal);
    }

    /*sai-Route DB search base on key: sai_route_entry_t is key*/
    retVal = xpSaiRouteGetRouteDB(unicast_route_entry, &saiRouteEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error in xpSaiRouteGetRouteDB an entry: error code: %d\n",
                       retVal);
        xpFree(l3RouteEntry);
        xpFree(attributes);
        return xpsStatus2SaiStatus(retVal);
    }

    if (saiRouteEntry != NULL)
    {
        XP_SAI_LOG_DBG("xpSaiRouteGetRouteDB Success:Entry already available in DB\n");
        xpFree(l3RouteEntry);
        xpFree(attributes);
        return SAI_STATUS_SUCCESS;
    }

    retVal = xpSaiRouteInsertRouteDB(unicast_route_entry, attributes);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error in xpSaiRouteInsertRouteDB: error code: %d\n", retVal);
        if (retVal == XP_ERR_KEY_EXISTS)
        {
            XP_SAI_LOG_ERR("Entry is already available in DB: error code: %d\n", retVal);
        }
        xpFree(l3RouteEntry);
        xpFree(attributes);
        return xpsStatus2SaiStatus(retVal);
    }

    xpFree(l3RouteEntry);
    xpFree(attributes);
    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiCreateRoute



sai_status_t xpSaiCreateRoute(const sai_route_entry_t *unicast_route_entry,
                              uint32_t attr_count, const sai_attribute_t *attr_list)
{
    XP_STATUS    retVal        = XP_NO_ERR;
    sai_status_t saiRetVal     = SAI_STATUS_FAILURE;
    uint32_t     routeObjectId = 0;
    xpsL3RouteEntry_t *l3RouteEntry = NULL;
    xpSaiRouteAttributesT *attributes = NULL;
    uint32_t prfxBucketIdx = 0;
    xpSaiRouteDbEntry_t *saiRouteEntry = NULL;
    uint32_t nhId, nhGrpId;
    xpSaiTableEntryCountDbEntry *entryCountCtxPtr = NULL;

    xpsDevice_t xpsDevId = xpSaiGetDevId();

    XP_SAI_LOG_DBG("xpSaiCreateRoute Enter:Attr_Count=%d!\n", attr_count);

    if (unicast_route_entry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               ROUTE_ENTRY_VALIDATION_ARRAY_SIZE, route_entry_attribs,
                               SAI_COMMON_API_CREATE);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }


    attributes = (xpSaiRouteAttributesT *)xpMalloc(sizeof(xpSaiRouteAttributesT));
    if (!attributes)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for attributes\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(attributes, 0, sizeof(xpSaiRouteAttributesT));

    xpSaiSetDefaultRouteAttributeVals(attributes);
    saiRetVal = xpSaiUpdateRouteAttributeVals(attr_count, attr_list, attributes);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiUpdateRouteAttributeVals failed, retVal : %d \n",
                       saiRetVal);
        xpFree(attributes);
        return saiRetVal;
    }

    if ((attributes->nextHopId.oid != SAI_NULL_OBJECT_ID) &&
        (XDK_SAI_OBJID_TYPE_CHECK(attributes->nextHopId.oid,
                                  SAI_OBJECT_TYPE_ROUTER_INTERFACE)))
    {
        if (xpSaiRouterInterfaceVirtualCheck(attributes->nextHopId.oid))
        {
            XP_SAI_LOG_ERR("Virtual RIF OID received\n");
            xpFree(attributes);
            return saiRetVal;
        }
    }

    l3RouteEntry = (xpsL3RouteEntry_t *)xpMalloc(sizeof(xpsL3RouteEntry_t));
    if (!l3RouteEntry)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for l3RouteEntry\n");
        xpFree(attributes);
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    memset(l3RouteEntry, 0, sizeof(xpsL3RouteEntry_t));

    saiRetVal = xpSaiConvertRoute(unicast_route_entry, attributes, l3RouteEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertRoute failed, retVal : %d \n", saiRetVal);
        xpFree(l3RouteEntry);
        xpFree(attributes);
        return saiRetVal;
    }

    if (!xpSaiVrfExists(l3RouteEntry->vrfId))
    {
        XP_SAI_LOG_ERR("VRF does not exists, vrfId %d", l3RouteEntry->vrfId);
        xpFree(l3RouteEntry);
        xpFree(attributes);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (l3RouteEntry->ipMaskLen)
    {
        retVal = xpsL3FindIpRouteEntry(xpsDevId, l3RouteEntry, &prfxBucketIdx);
        if (retVal == XP_NO_ERR)
        {
            XP_SAI_LOG_WARNING("A route already exists. No action needed.");
            xpFree(l3RouteEntry);
            xpFree(attributes);
            return SAI_STATUS_ITEM_ALREADY_EXISTS;
        }
    }

    saiRetVal = xpSaiGetTableEntryCountCtxDb(xpsDevId, &entryCountCtxPtr);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get Table Entry Count DB, retVal : %d \n", retVal);
        xpFree(l3RouteEntry);
        xpFree(attributes);
        return saiRetVal;
    }

    if (l3RouteEntry->type == XP_PREFIX_TYPE_IPV4)
    {
        if (entryCountCtxPtr->max_ipv4RouteEntries <=
            entryCountCtxPtr->ipv4RouteEntries)
        {
            XP_SAI_LOG_ERR("Max IPv4 routes reached. retVal : %d \n", retVal);
            xpFree(l3RouteEntry);
            xpFree(attributes);
            return xpsStatus2SaiStatus(XP_ERR_OUT_OF_RANGE);
        }
    }
    else if (l3RouteEntry->type == XP_PREFIX_TYPE_IPV6)
    {
        if (entryCountCtxPtr->max_ipv6RouteEntries <=
            entryCountCtxPtr->ipv6RouteEntries)
        {
            XP_SAI_LOG_ERR("Max IPv6 routes reached. retVal : %d \n", retVal);
            xpFree(l3RouteEntry);
            xpFree(attributes);
            return xpsStatus2SaiStatus(XP_ERR_OUT_OF_RANGE);
        }
    }

    //program NH to HW now, if doesn't exist
    if (l3RouteEntry->nhEcmpSize == 1)
    {
        xpsL3NextHopEntry_t pL3NextHopEntry;
        saiRetVal = xpsL3GetRouteNextHopDb(xpsDevId, l3RouteEntry->nhId,
                                           &pL3NextHopEntry);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiCreateRoute xpsL3GetRouteNextHopDb() failed with error code: %d\n",
                           saiRetVal);
            xpFree(l3RouteEntry);
            xpFree(attributes);
            return saiRetVal;
        }
        retVal = xpsL3SetRouteNextHop(xpsDevId, l3RouteEntry->nhId, &pL3NextHopEntry);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiCreateRoute xpsL3SetRouteNextHop : error code: %d\n",
                           retVal);
            xpFree(l3RouteEntry);
            xpFree(attributes);
            return xpsStatus2SaiStatus(retVal);
        }
        XP_SAI_LOG_INFO("xpSaiCreateRoute NH %u prgm success\n", l3RouteEntry->nhId);
    }
    retVal = xpsL3AddIpRouteEntry(xpsDevId, l3RouteEntry, &routeObjectId, NULL);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3AddIpRouteEntry failed, retVal : %d \n", retVal);
        xpFree(l3RouteEntry);
        xpFree(attributes);
        return xpsStatus2SaiStatus(retVal);
    }
#if 0
    /* As part of Default route, update Class E route. Set NH same as default route */
    if ((l3RouteEntry->type == XP_PREFIX_TYPE_IPV4) &&
        (unicast_route_entry->destination.addr.ip4 == XP_SAI_DEFAULT_ROUTE))
    {
        retVal = xpsL3WriteIpClassERouteEntry(xpsDevId, l3RouteEntry, true);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsL3WriteClassEIpRouteEntry failed, retVal : %d \n", retVal);
            /* Do not return error as this is implicit configuration */
            xpFree(l3RouteEntry);
            xpFree(attributes);
            return xpsStatus2SaiStatus(retVal);
        }
    }
#endif
    if (l3RouteEntry->type == XP_PREFIX_TYPE_IPV4)
    {
        entryCountCtxPtr->ipv4RouteEntries++;
    }
    if (l3RouteEntry->type == XP_PREFIX_TYPE_IPV6)
    {
        entryCountCtxPtr->ipv6RouteEntries++;
    }

    if ((attributes->nextHopId.oid != SAI_NULL_OBJECT_ID) &&
        (XDK_SAI_OBJID_TYPE_CHECK(attributes->nextHopId.oid, SAI_OBJECT_TYPE_NEXT_HOP)))
    {
        nhId = (uint32_t) xpSaiObjIdValueGet(attributes->nextHopId.oid);
        saiRetVal = xpSaiIncrementNextHopRouteRefCount(nhId, true);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiIncrementNextHopRouteRefCount failed, retVal : %d \n",
                           saiRetVal);
            xpFree(l3RouteEntry);
            xpFree(attributes);
            return saiRetVal;
        }
    }

    if ((attributes->nextHopId.oid != SAI_NULL_OBJECT_ID) &&
        (XDK_SAI_OBJID_TYPE_CHECK(attributes->nextHopId.oid,
                                  SAI_OBJECT_TYPE_NEXT_HOP_GROUP)))
    {
        nhGrpId = (uint32_t) xpSaiObjIdValueGet(attributes->nextHopId.oid);
        saiRetVal = xpSaiIncrementNextHopGroupRouteRefCount(nhGrpId, true);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiIncrementNextHopGroupRouteRefCount failed, retVal : %d \n",
                           saiRetVal);
            xpFree(l3RouteEntry);
            xpFree(attributes);
            return saiRetVal;
        }
    }

    retVal = xpSaiVrfRoutesNumInc(xpsDevId, l3RouteEntry->vrfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfRoutesNumInc failed, retVal : %d \n", retVal);
        xpFree(l3RouteEntry);
        xpFree(attributes);
        return xpsStatus2SaiStatus(retVal);
    }

    /*sai-Route DB search base on key: sai_route_entry_t is key*/
    retVal = xpSaiRouteGetRouteDB(unicast_route_entry, &saiRouteEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error in xpSaiRouteGetRouteDB an entry: error code: %d\n",
                       retVal);
        xpFree(l3RouteEntry);
        xpFree(attributes);
        return xpsStatus2SaiStatus(retVal);
    }

    if (saiRouteEntry != NULL)
    {
        XP_SAI_LOG_DBG("xpSaiRouteGetRouteDB Success:Entry already available in DB\n");
        xpFree(l3RouteEntry);
        xpFree(attributes);
        return SAI_STATUS_SUCCESS;
    }

    retVal = xpSaiRouteInsertRouteDB(unicast_route_entry, attributes);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error in xpSaiRouteInsertRouteDB: error code: %d\n", retVal);
        if (retVal == XP_ERR_KEY_EXISTS)
        {
            XP_SAI_LOG_ERR("Entry is already available in DB: error code: %d\n", retVal);
        }
        xpFree(l3RouteEntry);
        xpFree(attributes);
        return xpsStatus2SaiStatus(retVal);
    }

    xpFree(l3RouteEntry);
    xpFree(attributes);

    XP_SAI_LOG_DBG("xpSaiCreateRoute Success Exit!\n");

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiRemoveRouteForBulk(const sai_route_entry_t
                                     *unicast_route_entry,
                                     xpsL3RouteBulkInfo_t *routeInfo)
{
    XP_STATUS    retVal    = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_FAILURE;
    xpsL3RouteEntry_t l3RouteEntry;
    uint32_t prfxBucketIdx = 0;
    xpSaiRouteDbEntry_t saiRouteKey;
    xpSaiRouteDbEntry_t *saiRouteEntry = NULL;
    uint32_t nhId, nhGrpId;
    xpSaiTableEntryCountDbEntry *entryCountCtxPtr = NULL;

    memset(&saiRouteKey, 0, sizeof(xpSaiRouteDbEntry_t));
    memset(&l3RouteEntry, 0, sizeof(l3RouteEntry));

    xpsDevice_t xpsDevId = xpSaiGetDevId();

    XP_SAI_LOG_DBG("%s \n", __FUNCNAME__);

    if (!routeInfo)
    {
        return SAI_STATUS_FAILURE;
    }

    /* API used to validate the sai-route entry parameters and convert the sai-route in l3route-entry */
    saiRetVal = xpSaiConvertRoute(unicast_route_entry, NULL, &l3RouteEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertRoute failed, retVal : %d \n", saiRetVal);
        return saiRetVal;
    }

    /*sai_route_entry_t is Key for sai-route DB Entry */
    retVal = xpSaiRouteGetRouteDB(unicast_route_entry, &saiRouteEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error in xpSaiRouteGetRouteDB an entry: error code: %d\n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (NULL == saiRouteEntry)
    {
        XP_SAI_LOG_ERR("xpsRBTSearchNode Failed\n");
        return xpsStatus2SaiStatus(XP_ERR_KEY_NOT_FOUND);

    }

    XP_SAI_LOG_DBG("xpsRBTSearchNode Success:\n");

    retVal = xpsL3FindIpRouteEntry(xpsDevId, &l3RouteEntry, &prfxBucketIdx);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_DBG("A route does not exists. No action needed.");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    retVal = (XP_STATUS) xpsL3RemoveIpRouteEntry(xpsDevId, &l3RouteEntry,
                                                 routeInfo);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3RemoveIpRouteEntry failed, retVal : %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    saiRetVal = xpSaiGetTableEntryCountCtxDb(xpsDevId, &entryCountCtxPtr);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get Table Entry Count DB, retVal : %d \n", saiRetVal);
        return saiRetVal;
    }
    if (l3RouteEntry.type == XP_PREFIX_TYPE_IPV4)
    {
        entryCountCtxPtr->ipv4RouteEntries--;
    }
    if (l3RouteEntry.type == XP_PREFIX_TYPE_IPV6)
    {
        entryCountCtxPtr->ipv6RouteEntries--;
    }

    if ((saiRouteEntry->routeAttr.nextHopId.oid) &&
        XDK_SAI_OBJID_TYPE_CHECK(saiRouteEntry->routeAttr.nextHopId.oid,
                                 SAI_OBJECT_TYPE_NEXT_HOP))
    {
        nhId = (uint32_t) xpSaiObjIdValueGet(saiRouteEntry->routeAttr.nextHopId.oid);
        saiRetVal = xpSaiDecrementNextHopRouteRefCount(nhId, true);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiDecrementNextHopRouteRefCount failed, retVal : %d \n",
                           saiRetVal);
            return saiRetVal;
        }
    }

    if ((saiRouteEntry->routeAttr.nextHopId.oid) &&
        XDK_SAI_OBJID_TYPE_CHECK(saiRouteEntry->routeAttr.nextHopId.oid,
                                 SAI_OBJECT_TYPE_NEXT_HOP_GROUP))
    {
        nhGrpId = (uint32_t) xpSaiObjIdValueGet(saiRouteEntry->routeAttr.nextHopId.oid);
        saiRetVal = xpSaiDecrementNextHopGroupRouteRefCount(nhGrpId, true);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiDecrementNextHopGroupRouteRefCount failed, retVal : %d \n",
                           saiRetVal);
            return saiRetVal;
        }
    }

    retVal = xpSaiVrfRoutesNumDec(xpsDevId, l3RouteEntry.vrfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfRoutesNumInc failed, retVal : %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpSaiRouteRemoveRouteDB(unicast_route_entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiRouteRemoveRouteDB failed: Failed to delete state route-entry from DB\n");
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiRemoveRoute

sai_status_t xpSaiRemoveRoute(const sai_route_entry_t *unicast_route_entry)
{
    XP_STATUS    retVal    = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_FAILURE;
    xpsL3RouteEntry_t l3RouteEntry;
    uint32_t prfxBucketIdx = 0;
    xpSaiRouteDbEntry_t *saiRouteEntry = NULL;
    uint32_t nhId, nhGrpId;
    xpSaiTableEntryCountDbEntry *entryCountCtxPtr = NULL;

    memset(&l3RouteEntry, 0, sizeof(l3RouteEntry));

    xpsDevice_t xpsDevId = xpSaiGetDevId();

    XP_SAI_LOG_DBG("xpSaiRemoveRoute Enter before xpsL3RemoveIpRouteEntry!\n");

    /* API used to validate the sai-route entry parameters and convert the sai-route in l3route-entry */
    saiRetVal = xpSaiConvertRoute(unicast_route_entry, NULL, &l3RouteEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertRoute failed, retVal : %d \n", saiRetVal);
        return saiRetVal;
    }

    /*sai_route_entry_t is Key for sai-route DB Entry */
    retVal = xpSaiRouteGetRouteDB(unicast_route_entry, &saiRouteEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error in xpSaiRouteGetRouteDB an entry: error code: %d\n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (NULL == saiRouteEntry)
    {
        XP_SAI_LOG_ERR("xpsRBTSearchNode Failed\n");
        return xpsStatus2SaiStatus(XP_ERR_KEY_NOT_FOUND);

    }

    XP_SAI_LOG_DBG("xpsRBTSearchNode Success:\n");

    retVal = xpsL3FindIpRouteEntry(xpsDevId, &l3RouteEntry, &prfxBucketIdx);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_DBG("A route does not exists. No action needed.");
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    retVal = (XP_STATUS) xpsL3RemoveIpRouteEntry(xpsDevId, &l3RouteEntry, NULL);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3RemoveIpRouteEntry failed, retVal : %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    saiRetVal = xpSaiGetTableEntryCountCtxDb(xpsDevId, &entryCountCtxPtr);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get Table Entry Count DB, retVal : %d \n", saiRetVal);
        return saiRetVal;
    }
    if (l3RouteEntry.type == XP_PREFIX_TYPE_IPV4)
    {
        entryCountCtxPtr->ipv4RouteEntries--;
    }
    if (l3RouteEntry.type == XP_PREFIX_TYPE_IPV6)
    {
        entryCountCtxPtr->ipv6RouteEntries--;
    }

    if ((saiRouteEntry->routeAttr.nextHopId.oid) &&
        XDK_SAI_OBJID_TYPE_CHECK(saiRouteEntry->routeAttr.nextHopId.oid,
                                 SAI_OBJECT_TYPE_NEXT_HOP))
    {
        nhId = (uint32_t) xpSaiObjIdValueGet(saiRouteEntry->routeAttr.nextHopId.oid);
        saiRetVal = xpSaiDecrementNextHopRouteRefCount(nhId, true);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiDecrementNextHopRouteRefCount failed, retVal : %d \n",
                           saiRetVal);
            return saiRetVal;
        }
    }

    if ((saiRouteEntry->routeAttr.nextHopId.oid) &&
        XDK_SAI_OBJID_TYPE_CHECK(saiRouteEntry->routeAttr.nextHopId.oid,
                                 SAI_OBJECT_TYPE_NEXT_HOP_GROUP))
    {
        nhGrpId = (uint32_t) xpSaiObjIdValueGet(saiRouteEntry->routeAttr.nextHopId.oid);
        saiRetVal = xpSaiDecrementNextHopGroupRouteRefCount(nhGrpId, true);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiDecrementNextHopGroupRouteRefCount failed, retVal : %d \n",
                           saiRetVal);
            return saiRetVal;
        }
    }

    retVal = xpSaiVrfRoutesNumDec(xpsDevId, l3RouteEntry.vrfId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiVrfRoutesNumInc failed, retVal : %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    retVal = xpSaiRouteRemoveRouteDB(unicast_route_entry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiRouteRemoveRouteDB failed: Failed to delete state route-entry from DB\n");
        return xpsStatus2SaiStatus(retVal);
    }

    XP_SAI_LOG_DBG("xpSaiRemoveRoute Success Exit!\n");
    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiSetRouteAttributeForBulk(const sai_route_entry_t
                                           *unicast_route_entry, const sai_attribute_t *attr,
                                           xpsL3RouteBulkInfo_t *routeInfo)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    retVal = xpSaiAttrCheck(1, attr,
                            ROUTE_ENTRY_VALIDATION_ARRAY_SIZE, route_entry_attribs,
                            SAI_COMMON_API_SET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", retVal);
        return retVal;
    }

    if (unicast_route_entry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    switch (attr->id)
    {
        case SAI_ROUTE_ENTRY_ATTR_PACKET_ACTION:
            {
                retVal = xpSaiSetRouteAttrPacketAction(unicast_route_entry, attr->value,
                                                       routeInfo);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_ROUTE_ENTRY_ATTR_PACKET_ACTION)\n");
                    return retVal;
                }
                break;
            }
        case SAI_ROUTE_ENTRY_ATTR_USER_TRAP_ID:
            {
                retVal = xpSaiSetRouteAttrTrapPriority(unicast_route_entry, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_ROUTE_ENTRY_ATTR_TRAP_PRIORITY)\n");
                    return retVal;
                }
                break;
            }
        case SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID:
            {
                retVal = xpSaiSetRouteAttrNextHopId(unicast_route_entry, attr->value,
                                                    routeInfo);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID)\n");
                    return retVal;
                }
                break;
            }
        case SAI_ROUTE_ENTRY_ATTR_META_DATA:
            {
                XP_SAI_LOG_ERR("Attribute %d is not supported.\n", attr->id);
                return  SAI_STATUS_NOT_SUPPORTED;
            }
        default:
            {
                XP_SAI_LOG_ERR("Failed to set %d\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }
    return    retVal;
}

//Func: xpSaiSetRouteAttribute

sai_status_t xpSaiSetRouteAttribute(const sai_route_entry_t
                                    *unicast_route_entry, const sai_attribute_t *attr)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("xpSaiSetRouteAttribute Enter!\n");

    retVal = xpSaiAttrCheck(1, attr,
                            ROUTE_ENTRY_VALIDATION_ARRAY_SIZE, route_entry_attribs,
                            SAI_COMMON_API_SET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", retVal);
        return retVal;
    }

    if (unicast_route_entry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    switch (attr->id)
    {
        case SAI_ROUTE_ENTRY_ATTR_PACKET_ACTION:
            {
                retVal = xpSaiSetRouteAttrPacketAction(unicast_route_entry, attr->value, NULL);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_ROUTE_ENTRY_ATTR_PACKET_ACTION)\n");
                    return retVal;
                }
                break;
            }
        case SAI_ROUTE_ENTRY_ATTR_USER_TRAP_ID:
            {
                retVal = xpSaiSetRouteAttrTrapPriority(unicast_route_entry, attr->value);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_ROUTE_ENTRY_ATTR_TRAP_PRIORITY)\n");
                    return retVal;
                }
                break;
            }
        case SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID:
            {
                retVal = xpSaiSetRouteAttrNextHopId(unicast_route_entry, attr->value, NULL);
                if (SAI_STATUS_SUCCESS != retVal)
                {
                    XP_SAI_LOG_ERR("Failed to set (SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID)\n");
                    return retVal;
                }
                break;
            }
        case SAI_ROUTE_ENTRY_ATTR_META_DATA:
            {
                XP_SAI_LOG_ERR("Attribute %d is not supported.\n", attr->id);
                return  SAI_STATUS_NOT_SUPPORTED;
            }
        default:
            {
                XP_SAI_LOG_ERR("Failed to set %d\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    XP_SAI_LOG_DBG("xpSaiSetRouteAttribute exit:ret=%d!\n", retVal);

    return    retVal;
}

//Func: xpSaiGetRouteAttribute

sai_status_t xpSaiGetRouteAttribute(const sai_route_entry_t
                                    *unicast_route_entry, sai_attribute_t *attr)
{
    sai_status_t saiRetVal        = SAI_STATUS_SUCCESS;
    XP_STATUS    retVal              = XP_NO_ERR;
    XP_STATUS    xpsStatus        = XP_NO_ERR;
    uint32_t     prfxBucketIdx    = 0xFFFFFFFF;
    xpsL3RouteEntry_t l3RouteEntry;
    xpSaiRouteStaticDbEntry* staticVarDb = NULL;
    xpSaiRouteDbEntry_t *saiRouteEntry = NULL;

    memset(&l3RouteEntry, 0, sizeof(l3RouteEntry));

    xpsDevice_t xpsDevId = xpSaiGetDevId();

    saiRetVal = xpSaiConvertRoute(unicast_route_entry, NULL, &l3RouteEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertRoute failed, retVal : %d \n", saiRetVal);
        return saiRetVal;
    }

    xpsStatus = xpsL3FindIpRouteEntry(xpsDevId, &l3RouteEntry, &prfxBucketIdx);
    if (xpsStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3FindIpRouteEntry failed, retVal : %d \n", xpsStatus);
        return xpsStatus2SaiStatus(xpsStatus);
    }

    /*sai_route_entry_t is Key for sai-route Entry */
    retVal = xpSaiRouteGetRouteDB(unicast_route_entry, &saiRouteEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error in xpSaiRouteGetRouteDB an entry: error code: %d\n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (saiRouteEntry == NULL)
    {
        XP_SAI_LOG_ERR("xpSaiRouteGetRouteDB Failed\n");
        return xpsStatus2SaiStatus(XP_ERR_KEY_NOT_FOUND);
    }

    XP_SAI_LOG_DBG("xpSaiRouteGetRouteDB Success\n");

    switch (attr->id)
    {
        case SAI_ROUTE_ENTRY_ATTR_PACKET_ACTION:
            {
                retVal = xpSaiRouteGetStaticVariablesDb(&staticVarDb);
                if (retVal != XP_NO_ERR)
                {
                    return xpsStatus2SaiStatus(retVal);
                }

                if (staticVarDb->gSaiDefaultDropNhId == l3RouteEntry.nhId)
                {
                    attr->value.s32 = SAI_PACKET_ACTION_DROP;
                }
                else if (staticVarDb->gSaiDefaultTrapNhId == l3RouteEntry.nhId)
                {
                    attr->value.s32 = SAI_PACKET_ACTION_TRAP;
                }
                else
                {
                    attr->value.s32 = SAI_PACKET_ACTION_FORWARD;
                }

                break;
            }
        case SAI_ROUTE_ENTRY_ATTR_USER_TRAP_ID:
            {
                // As of now, this attribute is not supported
                // So, using it's default value which is <SAI_ROUTE_DFLT_TRAP_PRIORITY>
                attr->value.u8 = SAI_ROUTE_DEFAULT_TRAP_PRIORITY;
                break;
            }
        case SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID:
            {
                attr->value.oid = saiRouteEntry->routeAttr.nextHopId.oid;
                break;
            }
        case SAI_ROUTE_ENTRY_ATTR_META_DATA:
            {
                XP_SAI_LOG_ERR("Attribute %d is not supported.\n", attr->id);
                return  SAI_STATUS_NOT_SUPPORTED;
            }
        default:
            {
                XP_SAI_LOG_ERR("Failed to get %d\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    return xpsStatus2SaiStatus(xpsStatus);
}


//Func: xpSaiGetRouteAttributes

sai_status_t xpSaiGetRouteAttributes(const sai_route_entry_t
                                     *unicast_route_entry, uint32_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t      retVal   = SAI_STATUS_SUCCESS;

    retVal = xpSaiAttrCheck(attr_count, attr_list,
                            ROUTE_ENTRY_VALIDATION_ARRAY_SIZE, route_entry_attribs,
                            SAI_COMMON_API_GET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", retVal);
        return retVal;
    }

    for (uint32_t count = 0; count < attr_count; count++)
    {
        retVal = xpSaiGetRouteAttribute(unicast_route_entry, &attr_list[count]);
        if (SAI_STATUS_SUCCESS != retVal)
        {
            XP_SAI_LOG_ERR("xpSaiGetRouteAttribute failed\n");
            return retVal;
        }
    }

    return retVal;
}


static XP_STATUS
xpSaiCreateTrapNextHop(xpsDevice_t xpsDevId)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsL3NextHopEntry_t saiGlobalDefaultNh;
    xpSaiRouteStaticDbEntry* staticVarDb = NULL;

    retVal = xpSaiRouteGetStaticVariablesDb(&staticVarDb);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve route DB object, error %d\n", retVal);
        return retVal;
    }

    XP_SAI_LOG_DBG("gSaiDefaultTrapNhId Allocated :%d\n\n",
                   staticVarDb->gSaiDefaultTrapNhId);

    saiGlobalDefaultNh.pktCmd = XP_PKTCMD_TRAP;
    saiGlobalDefaultNh.nextHop.egressIntfId = XPS_INTF_INVALID_ID;

    retVal = xpsL3SetRouteNextHopDb(xpsDevId, staticVarDb->gSaiDefaultTrapNhId,
                                    &saiGlobalDefaultNh);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error in xpsL3SetRouteNextHopDb an entry: error code: %d\n",
                       retVal);
        return retVal;
    }
    retVal = xpsL3SetRouteNextHop(xpsDevId, staticVarDb->gSaiDefaultTrapNhId,
                                  &saiGlobalDefaultNh);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error in xpsL3SetRouteNextHop an entry: error code: %d\n",
                       retVal);
        return retVal;
    }

    return retVal;
}

static XP_STATUS
xpSaiCreateDropNextHop(xpsDevice_t xpsDevId)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpsL3NextHopEntry_t saiGlobalDefaultNh;
    xpSaiRouteStaticDbEntry* staticVarDb = NULL;

    memset(&saiGlobalDefaultNh, 0, sizeof(xpsL3NextHopEntry_t));
    retVal = xpSaiRouteGetStaticVariablesDb(&staticVarDb);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve route DB object, error %d\n", retVal);
        return retVal;
    }

    XP_SAI_LOG_DBG("gSaiDefaultDropNhId Allocated :%d\n\n",
                   staticVarDb->gSaiDefaultDropNhId);

    saiGlobalDefaultNh.pktCmd = XP_PKTCMD_DROP;
    saiGlobalDefaultNh.nextHop.egressIntfId = XPS_INTF_INVALID_ID;

    retVal = (XP_STATUS) xpsL3SetRouteNextHopDb(xpsDevId,
                                                staticVarDb->gSaiDefaultDropNhId, &saiGlobalDefaultNh);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error in xpsL3SetRouteNextHopDb an entry: error code: %d\n",
                       retVal);
        return retVal;
    }

    retVal = (XP_STATUS) xpsL3SetRouteNextHop(xpsDevId,
                                              staticVarDb->gSaiDefaultDropNhId, &saiGlobalDefaultNh);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error in xpsL3SetRouteNextHop an entry: error code: %d\n",
                       retVal);
        return retVal;
    }

    return retVal;
}

static int32_t saiRouteKeyComp(void* key1, void* key2)
{
    return (*(uint32_t *)key1 - *(uint32_t *)key2);
}

static int32_t xpSaiRouteEntryKeyComp(void* _key1, void* _key2)
{
    xpSaiRouteDbEntry_t *key1 = (xpSaiRouteDbEntry_t *)_key1;
    xpSaiRouteDbEntry_t *key2 = (xpSaiRouteDbEntry_t *)_key2;

    if (key1->switch_id > key2->switch_id)
    {
        return 1;
    }
    else if (key1->switch_id < key2->switch_id)
    {
        return -1;
    }
    // key1->switch_id == key2->switch_id

    if (key1->vr_id > key2->vr_id)
    {
        return 1;
    }
    else if (key1->vr_id < key2->vr_id)
    {
        return -1;
    }
    // key1->vr_id == key2->vr_id

    if (key1->destination.addr_family > key2->destination.addr_family)
    {
        return 1;
    }
    else if (key1->destination.addr_family < key2->destination.addr_family)
    {
        return -1;
    }
    // key1->destination.addr_family == key2->destination.addr_family

    if (key1->destination.addr_family == SAI_IP_ADDR_FAMILY_IPV4)
    {
        if (key1->destination.mask.ip4 > key2->destination.mask.ip4)
        {
            return 1;
        }
        else if (key1->destination.mask.ip4 < key2->destination.mask.ip4)
        {
            return -1;
        }
        else
        {
            if (key1->destination.addr.ip4 > key2->destination.addr.ip4)
            {
                return 1;
            }
            else if (key1->destination.addr.ip4 < key2->destination.addr.ip4)
            {
                return -1;
            }
            else
            {
                return 0;
            }
        }
    }
    else
    {
        if (COMPARE_IPV6_ADDR_T(key1->destination.mask.ip6,
                                key2->destination.mask.ip6) < 0)
        {
            return -1;
        }
        else if (COMPARE_IPV6_ADDR_T(key1->destination.mask.ip6,
                                     key2->destination.mask.ip6) > 0)
        {
            return 1;
        }

        return COMPARE_IPV6_ADDR_T(key1->destination.addr.ip6,
                                   key2->destination.addr.ip6);
    }
}

XP_STATUS xpSaiRouteInit(xpsDevice_t xpSaiDevId)
{
    XP_STATUS retVal = XP_NO_ERR;

    saiRouteStaticDataDbHndl = XPSAI_ROUTE_STATIC_DB_HNDL;
    saiRouteDataDbHndl = XPSAI_ROUTE_DB_HNDL;

    retVal = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "Sai Route global variables",
                                XPS_GLOBAL,
                                &saiRouteKeyComp, saiRouteStaticDataDbHndl);
    if (retVal != XP_NO_ERR)
    {
        saiRouteStaticDataDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        XP_SAI_LOG_ERR("Failed to register SAI Route Static DB\n");
        return retVal;
    }

    retVal = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "Sai Route global variables",
                                XPS_GLOBAL,
                                &xpSaiRouteEntryKeyComp, saiRouteDataDbHndl);
    if (retVal != XP_NO_ERR)
    {
        saiRouteDataDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        XP_SAI_LOG_ERR("Failed to register SAI Route Static DB\n");
        return retVal;
    }

    if (INIT_COLD == xpsGetInitType())
    {
        if ((retVal = xpSaiRouteInsertStaticVariablesDb()) != XP_NO_ERR)
        {
            return retVal;
        }
    }
    return retVal;
}

XP_STATUS xpSaiRouteDevInit(xpsDevice_t xpsDevId)
{
    XP_STATUS retVal = XP_NO_ERR;

    retVal = xpSaiCreateDropNextHop(xpsDevId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to create Drop NH\n");
        return retVal;
    }

    retVal = xpSaiCreateTrapNextHop(xpsDevId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to create Trap NH\n");
        return retVal;
    }

    return retVal;
}

XP_STATUS xpSaiRouteDeinit(xpsDevice_t xpsDevId)
{
    XP_STATUS retVal = XP_NO_ERR;

    retVal = xpSaiRouteRemoveStaticVariablesDb();
    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }

    retVal = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT, &saiRouteStaticDataDbHndl);
    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }

    retVal = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT, &saiRouteDataDbHndl);
    if (retVal != XP_NO_ERR)
    {
        return retVal;
    }

    return retVal;
}

//Func: xpSaiBulkCreateRoutes

static sai_status_t xpSaiBulkCreateRoutes(uint32_t object_count,
                                          const sai_route_entry_t *route_entry,
                                          const uint32_t *attr_count, const sai_attribute_t **attr_list,
                                          sai_bulk_op_error_mode_t mode, sai_status_t *object_statuses)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS status = XP_NO_ERR;
    //    bool anyFailed = false;

    XP_SAI_LOG_DBG("xpSaiBulkCreateRoutes Enter:objCount=%d!\n", object_count);

    if ((route_entry == NULL) || (attr_count == NULL) ||
        (attr_list == NULL) || (object_statuses == NULL))
    {
        XP_SAI_LOG_ERR("Null pointer passed as a parameter!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!object_count)
    {
        XP_SAI_LOG_ERR("Zero object_count passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    memset(object_statuses, 0, (sizeof(sai_status_t)*object_count));
    if (mode != SAI_BULK_OP_ERROR_MODE_IGNORE_ERROR)
    {
        XP_SAI_LOG_ERR("unsupported Mode %d \n", mode);
        return SAI_STATUS_NOT_SUPPORTED;
    }
#if 0
    SAI_ROUTE_BULK_PROCESS(xpSaiCreateRoute(&route_entry[count], attr_count[count],
                                            attr_list[count]),
                           object_count, object_statuses, mode, anyFailed);

    if (anyFailed)
    {
        return SAI_STATUS_FAILURE;
    }
#endif

    uint32_t count = 0;
    xpsL3RouteBulkInfo_t *routeInfo = NULL;

    routeInfo = (xpsL3RouteBulkInfo_t*)xpMalloc(sizeof(xpsL3RouteBulkInfo_t)
                                                *object_count);
    if (NULL == routeInfo)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for routeInfo\n");
        return SAI_STATUS_NO_MEMORY;
    }
    memset(routeInfo, 0, (sizeof(xpsL3RouteBulkInfo_t)*object_count));

    /* Build the data */
    for (count = 0; count < object_count; count++)
    {
        saiRetVal = xpSaiCreateRouteForBulk(&route_entry[count], attr_count[count],
                                            attr_list[count], &routeInfo[count]);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            routeInfo[count].retStatus = 0xFFFFFFFF;
        }
    }

    XP_SAI_LOG_DBG("xpSaiBulkCreateRoutes before xpsL3RouteBulkAdd:%d!\n", count);

    /*Configure Bulk in HW*/
    status = xpsL3RouteBulkAdd(routeInfo, object_count);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3RouteBulkAdd Failed %d \n", status);
        return SAI_STATUS_FAILURE;
    }

    /* Update status from HW */
    for (count = 0; count < object_count; count++)
    {
        if (routeInfo[count].retStatus != 0)
        {
            object_statuses[count] = SAI_STATUS_FAILURE;
        }
    }

    if (routeInfo)
    {
        xpFree(routeInfo);
    }

    XP_SAI_LOG_DBG("xpSaiBulkCreateRoutes success finish:%d!\n", count);

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBulkRemoveRoutes

static sai_status_t xpSaiBulkRemoveRoutes(uint32_t object_count,
                                          const sai_route_entry_t *route_entry,
                                          sai_bulk_op_error_mode_t mode, sai_status_t *object_statuses)
{
    //    bool anyFailed = false;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS status = XP_NO_ERR;

    XP_SAI_LOG_DBG("xpSaiBulkRemoveRoutes Enter:objCount=%d!\n", object_count);

    if ((route_entry == NULL) || (object_statuses == NULL))
    {
        XP_SAI_LOG_ERR("Null pointer passed as a parameter!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!object_count)
    {
        XP_SAI_LOG_ERR("Zero object_count passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    memset(object_statuses, 0, (sizeof(sai_status_t)*object_count));

    if (mode != SAI_BULK_OP_ERROR_MODE_IGNORE_ERROR)
    {
        XP_SAI_LOG_ERR("unsupported Mode %d \n", mode);
        return SAI_STATUS_NOT_SUPPORTED;
    }
#if 0
    SAI_ROUTE_BULK_PROCESS(xpSaiRemoveRoute(&route_entry[count]),
                           object_count, object_statuses, mode, anyFailed);

    if (anyFailed)
    {
        return SAI_STATUS_FAILURE;
    }
#endif
    uint32_t count = 0;
    xpsL3RouteBulkInfo_t *routeInfo = NULL;

    routeInfo = (xpsL3RouteBulkInfo_t*)xpMalloc(sizeof(xpsL3RouteBulkInfo_t)
                                                *object_count);
    if (NULL == routeInfo)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for routeInfo\n");
        return SAI_STATUS_NO_MEMORY;
    }
    memset(routeInfo, 0, (sizeof(xpsL3RouteBulkInfo_t)*object_count));

    for (count = 0; count < object_count; count++)
    {
        saiRetVal = xpSaiRemoveRouteForBulk(&route_entry[count], &routeInfo[count]);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            routeInfo[count].retStatus = 0xFFFFFFFF;
        }
    }

    XP_SAI_LOG_DBG("xpSaiBulkRemoveRoutes before xpsL3RouteBulkDel Enter:objCount=%d!\n",
                   object_count);

    /*Configure Bulk in HW*/
    status = xpsL3RouteBulkDel(routeInfo, object_count);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3RouteBulkAdd Failed %d \n", status);
        return SAI_STATUS_FAILURE;
    }

    for (count = 0; count < object_count; count++)
    {
        if (routeInfo[count].retStatus != 0)
        {
            object_statuses[count] = SAI_STATUS_FAILURE;
        }
    }

    if (routeInfo)
    {
        xpFree(routeInfo);
    }

    XP_SAI_LOG_DBG("xpSaiBulkRemoveRoutes success finish:%d!\n", count);

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBulkSetRoutesAttributes

static sai_status_t xpSaiBulkSetRoutesAttribute(uint32_t object_count,
                                                const sai_route_entry_t *route_entry,
                                                const sai_attribute_t *attr_list, sai_bulk_op_error_mode_t mode,
                                                sai_status_t *object_statuses)
{
    //bool anyFailed = false;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    XP_STATUS status = XP_NO_ERR;

    XP_SAI_LOG_DBG("xpSaiBulkSetRoutesAttribute Enter:objCount=%d!\n",
                   object_count);

    if ((route_entry == NULL) || (attr_list == NULL) || (object_statuses == NULL))
    {
        XP_SAI_LOG_ERR("Null pointer passed as a parameter!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!object_count)
    {
        XP_SAI_LOG_ERR("Zero object_count passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (mode != SAI_BULK_OP_ERROR_MODE_IGNORE_ERROR)
    {
        XP_SAI_LOG_ERR("unsupported Mode %d \n", mode);
        return SAI_STATUS_NOT_SUPPORTED;
    }
    memset(object_statuses, 0, (sizeof(sai_status_t)*object_count));
#if 0
    SAI_ROUTE_BULK_PROCESS(xpSaiSetRouteAttribute(&route_entry[count],
                                                  &attr_list[count]),
                           object_count, object_statuses, mode, anyFailed);

    if (anyFailed)
    {
        return SAI_STATUS_FAILURE;
    }
#endif
    uint32_t count = 0;
    xpsL3RouteBulkInfo_t *routeInfo = NULL;

    routeInfo = (xpsL3RouteBulkInfo_t*)xpMalloc(sizeof(xpsL3RouteBulkInfo_t)
                                                *object_count);
    if (NULL == routeInfo)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for routeInfo\n");
        return SAI_STATUS_NO_MEMORY;
    }
    memset(routeInfo, 0, (sizeof(xpsL3RouteBulkInfo_t)*object_count));

    for (count = 0; count < object_count; count++)
    {
        saiRetVal = xpSaiSetRouteAttributeForBulk(&route_entry[count],
                                                  &attr_list[count], &routeInfo[count]);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            routeInfo[count].retStatus = 0xFFFFFFFF;
        }
    }

    XP_SAI_LOG_DBG("xpSaiBulkSetRoutesAttribute before xpsL3RouteBulkAdd:%d!\n",
                   count);

    /*Configure Bulk in HW*/
    status = xpsL3RouteBulkAdd(routeInfo, object_count);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3RouteBulkAdd Failed %d \n", status);
        return SAI_STATUS_FAILURE;
    }

    for (count = 0; count < object_count; count++)
    {
        if (routeInfo[count].retStatus != 0)
        {
            object_statuses[count] = SAI_STATUS_FAILURE;
        }
    }

    if (routeInfo)
    {
        xpFree(routeInfo);
    }

    XP_SAI_LOG_DBG("xpSaiBulkSetRoutesAttribute finish:%d!\n", count);

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiBulkGetRoutesAttributes

static sai_status_t xpSaiBulkGetRoutesAttributes(uint32_t object_count,
                                                 const sai_route_entry_t *route_entry,
                                                 const uint32_t *attr_count, sai_attribute_t **attr_list,
                                                 sai_bulk_op_error_mode_t mode, sai_status_t *object_statuses)
{
    bool anyFailed = false;

    if ((route_entry == NULL) || (attr_count == NULL) ||
        (attr_list == NULL) || (object_statuses == NULL))
    {
        XP_SAI_LOG_ERR("Null pointer passed as a parameter!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!object_count)
    {
        XP_SAI_LOG_ERR("Zero object_count passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    SAI_ROUTE_BULK_PROCESS(xpSaiGetRouteAttributes(&route_entry[count],
                                                   attr_count[count], attr_list[count]),
                           object_count, object_statuses, mode, anyFailed);

    if (anyFailed)
    {
        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiRouteApiInit

XP_STATUS xpSaiRouteApiInit(uint64_t flag,
                            const sai_service_method_table_t* adapHostServiceMethodTable)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiRouteApiInit\n");

    _xpSaiRouteApi = (sai_route_api_t *) xpMalloc(sizeof(sai_route_api_t));
    if (NULL == _xpSaiRouteApi)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for _xpSaiRouteApi\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    _xpSaiRouteApi->create_route_entry = xpSaiCreateRoute;
    _xpSaiRouteApi->remove_route_entry = xpSaiRemoveRoute;
    _xpSaiRouteApi->set_route_entry_attribute = xpSaiSetRouteAttribute;
    _xpSaiRouteApi->get_route_entry_attribute = xpSaiGetRouteAttributes;
    _xpSaiRouteApi->create_route_entries = xpSaiBulkCreateRoutes;
    _xpSaiRouteApi->remove_route_entries = xpSaiBulkRemoveRoutes;
    _xpSaiRouteApi->set_route_entries_attribute = xpSaiBulkSetRoutesAttribute;
    _xpSaiRouteApi->get_route_entries_attribute = xpSaiBulkGetRoutesAttributes;

    saiRetVal = xpSaiApiRegister(SAI_API_ROUTE, (void*)_xpSaiRouteApi);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to register route API\n");
        return XP_ERR_ARRAY_OUT_OF_BOUNDS;
    }

    return retVal;
}


//Func: xpSaiRouteApiDeinit

XP_STATUS xpSaiRouteApiDeinit()
{
    XP_STATUS retVal = XP_NO_ERR;

    XP_SAI_LOG_DBG("Calling xpSaiRouteApiDeinit\n");

    xpFree(_xpSaiRouteApi);
    _xpSaiRouteApi = NULL;

    return retVal;
}

