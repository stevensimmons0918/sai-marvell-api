// xpSaiNextHop.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSai.h"
#include "xpSaiUtil.h"
#include "xpSaiValidationArrays.h"

XP_SAI_LOG_REGISTER_API(SAI_API_NEXT_HOP);

static sai_next_hop_api_t* _xpSaiNextHopApi;
static xpsDbHandle_t gsaiNextHopDbHdle = XPS_STATE_INVALID_DB_HANDLE;

extern xpsDbHandle_t xpSaiSwitchStaticDataDbHandle;
static pthread_mutex_t gSaiNextHopLock = PTHREAD_MUTEX_INITIALIZER;

static inline sai_status_t xpSaiNextHopInitLock()
{
    if (pthread_mutex_init(&gSaiNextHopLock, NULL) != 0)
    {
        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static inline sai_status_t xpSaiNextHopDeinitLock()
{
    if (pthread_mutex_destroy(&gSaiNextHopLock) != 0)
    {
        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static inline void xpSaiNextHopLock()
{
    pthread_mutex_lock(&gSaiNextHopLock);
}

static inline void xpSaiNextHopUnlock()
{
    pthread_mutex_unlock(&gSaiNextHopLock);
}

//Func: xpSaiConvertNextHopOid

sai_status_t xpSaiConvertNextHopOid(sai_object_id_t next_hop_id,
                                    uint32_t* pNhId)
{
    if (!XDK_SAI_OBJID_TYPE_CHECK(next_hop_id, SAI_OBJECT_TYPE_NEXT_HOP))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(next_hop_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    *pNhId = (uint32_t)xpSaiObjIdValueGet(next_hop_id);

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiConvertNexthopXpsId(uint32_t xpsNhId,
                                      sai_object_id_t *saiNhId)
{
    xpsDevice_t devId = xpSaiGetDevId();

    if (xpSaiObjIdCreate(SAI_OBJECT_TYPE_NEXT_HOP, devId, (sai_uint64_t) xpsNhId,
                         saiNhId) != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI object can not be created.\n");
        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiNextHopDbInfoGet(sai_object_id_t nh_oid,
                                   xpSaiNextHopEntry_t **ppNextHopEntry)
{
    XP_STATUS           retVal      = XP_NO_ERR;
    xpSaiNextHopEntry_t *nhEntrykey = NULL;

    if (ppNextHopEntry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer passed as a parameter!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    nhEntrykey = (xpSaiNextHopEntry_t *) xpMalloc(sizeof(xpSaiNextHopEntry_t));
    if (!nhEntrykey)
    {
        XP_SAI_LOG_ERR("Could not allocate NextHop structure\n");
        return xpsStatus2SaiStatus(XP_ERR_MEM_ALLOC_ERROR);
    }
    memset(nhEntrykey, 0, sizeof(xpSaiNextHopEntry_t));

    nhEntrykey->nhId = (uint32_t)xpSaiObjIdValueGet(nh_oid);

    retVal = xpsStateSearchData(XP_SCOPE_DEFAULT, gsaiNextHopDbHdle, nhEntrykey,
                                (void**)ppNextHopEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateSearchData() failed with error code: %d!\n", retVal);
        xpFree(nhEntrykey);
        return xpsStatus2SaiStatus(retVal);
    }

    if (*ppNextHopEntry == NULL)
    {
        XP_SAI_LOG_ERR("Could not find NextHop entry in DB\n");
        xpFree(nhEntrykey);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    xpFree(nhEntrykey);
    return xpsStatus2SaiStatus(retVal);
}

bool xpSaiNextHopDbExists(sai_object_id_t nh_oid)
{
    xpSaiNextHopEntry_t *pNextHopEntry;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    saiRetVal = xpSaiNextHopDbInfoGet(nh_oid, &pNextHopEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        return FALSE;
    }
    return TRUE;
}

//Func: xpSaiSetDefaultNextHopAttributeVals

void xpSaiSetDefaultNextHopAttributeVals(xpSaiNextHopAttributesT* attributes)
{
    XP_SAI_LOG_DBG("Calling xpSaiSetDefaultNextHopAttributeVals\n");
}

//Func: xpSaiUpdateNextHopAttributeVals

sai_status_t xpSaiUpdateNextHopAttributeVals(const uint32_t attr_count,
                                             const sai_attribute_t* attr_list, xpSaiNextHopAttributesT* attributes)
{
    XP_SAI_LOG_DBG("Calling xpSaiUpdateNextHopAttributeVals\n");

    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_NEXT_HOP_ATTR_TYPE:
                {
                    attributes->type = attr_list[count].value;
                    break;
                }
            case SAI_NEXT_HOP_ATTR_IP:
                {
                    attributes->ip = attr_list[count].value;
                    break;
                }
            case SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID:
                {
                    attributes->routerInterfaceId = attr_list[count].value;
                    break;
                }
            case SAI_NEXT_HOP_ATTR_TUNNEL_ID:
                {
                    attributes->tunnelId = attr_list[count].value;
                    break;
                }
            case SAI_NEXT_HOP_ATTR_TUNNEL_MAC:
                {
                    attributes->tunnelMac = attr_list[count].value;
                    break;
                }
            case SAI_NEXT_HOP_ATTR_TUNNEL_VNI:
                {
                    attributes->tunnelVni = attr_list[count].value;
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

//Func: xpSaiGetNextHopAttrRouterInterfaceId

sai_status_t xpSaiGetNextHopAttrRouterInterfaceId(sai_object_id_t next_hop_id,
                                                  sai_attribute_value_t *value)
{
    sai_status_t        saiRetVal       = SAI_STATUS_SUCCESS;
    xpSaiNextHopEntry_t *pNextHopEntry  = NULL;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (value == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer passed as a parameter!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(next_hop_id, SAI_OBJECT_TYPE_NEXT_HOP))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(next_hop_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiNextHopDbInfoGet(next_hop_id, &pNextHopEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiNextHopDbInfoGet() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    value->oid = pNextHopEntry->rifId;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetNextHopAttrIp

sai_status_t xpSaiGetNextHopAttrIp(sai_object_id_t next_hop_id,
                                   sai_attribute_value_t* value)
{
    sai_status_t        saiRetVal      = SAI_STATUS_SUCCESS;
    xpSaiNextHopEntry_t *pNextHopEntry = NULL;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(next_hop_id, SAI_OBJECT_TYPE_NEXT_HOP))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(next_hop_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiNextHopDbInfoGet(next_hop_id, &pNextHopEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiNextHopDbInfoGet() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    switch (pNextHopEntry->type)
    {
        case SAI_IP_ADDR_FAMILY_IPV4:
            {
                value->ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
                xpSaiIpCopy((uint8_t*)&value->ipaddr.addr.ip4, pNextHopEntry->ipv4Addr,
                            SAI_IP_ADDR_FAMILY_IPV4);
                break;
            }
        case SAI_IP_ADDR_FAMILY_IPV6:
            {
                value->ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV6;
                xpSaiIpCopy(value->ipaddr.addr.ip6, pNextHopEntry->ipv6Addr,
                            SAI_IP_ADDR_FAMILY_IPV6);
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Invalid addr family %d\n", pNextHopEntry->type);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetNextHopAttrType

sai_status_t xpSaiGetNextHopAttrType(sai_object_id_t next_hop_id,
                                     sai_attribute_value_t *value)
{
    sai_status_t        saiRetVal       = SAI_STATUS_SUCCESS;
    xpSaiNextHopEntry_t *pNextHopEntry  = NULL;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCTION__);

    if (value == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer passed as a parameter!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(next_hop_id, SAI_OBJECT_TYPE_NEXT_HOP))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(next_hop_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiNextHopDbInfoGet(next_hop_id, &pNextHopEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiNextHopDbInfoGet() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    value->s32 = pNextHopEntry->nhType;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetNextHopAttrTunnelMac

sai_status_t xpSaiGetNextHopAttrTunnelMac(sai_object_id_t next_hop_id,
                                          sai_attribute_value_t *value)
{
    sai_status_t        saiRetVal       = SAI_STATUS_SUCCESS;
    xpSaiNextHopEntry_t *pNextHopEntry  = NULL;
    uint32_t            xpsNhId             = 0;
    xpsDevice_t xpsDevId = xpSaiObjIdSwitchGet(next_hop_id);
    XP_STATUS retVal;
    xpsL3NextHopEntry_t L3NextHopEntry;


    XP_SAI_LOG_DBG("Calling %s \n", __FUNCTION__);

    if (value == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer passed as a parameter!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(next_hop_id, SAI_OBJECT_TYPE_NEXT_HOP))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(next_hop_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiNextHopDbInfoGet(next_hop_id, &pNextHopEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiNextHopDbInfoGet() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    if (pNextHopEntry->nhType == SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP)
    {

        xpsNhId = (uint32_t)xpSaiObjIdValueGet(next_hop_id);

        XP_SAI_LOG_DBG("xpsNhId %u \n", xpsNhId);

        retVal = xpsL3GetRouteNextHopDb(xpsDevId, xpsNhId, &L3NextHopEntry);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiNextHopDbInfoGet() failed with error code: %d\n",
                           retVal);
            return xpsStatus2SaiStatus(retVal);
        }
        memset(value->mac, 0x0, sizeof(sai_mac_t));
        xpSaiMacCopy(value->mac, L3NextHopEntry.nextHop.macDa);

    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiGetNextHopAttrTunnelVni

sai_status_t xpSaiGetNextHopAttrTunnelVni(sai_object_id_t next_hop_id,
                                          sai_attribute_value_t *value)
{
    sai_status_t        saiRetVal       = SAI_STATUS_SUCCESS;
    xpSaiNextHopEntry_t *pNextHopEntry  = NULL;
    uint32_t            xpsNhId             = 0;
    xpsDevice_t xpsDevId = xpSaiObjIdSwitchGet(next_hop_id);
    XP_STATUS retVal;
    xpsL3NextHopEntry_t L3NextHopEntry;


    XP_SAI_LOG_DBG("Calling %s \n", __FUNCTION__);

    if (value == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer passed as a parameter!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(next_hop_id, SAI_OBJECT_TYPE_NEXT_HOP))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(next_hop_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiNextHopDbInfoGet(next_hop_id, &pNextHopEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiNextHopDbInfoGet() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    if (pNextHopEntry->nhType == SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP)
    {

        xpsNhId = (uint32_t)xpSaiObjIdValueGet(next_hop_id);

        XP_SAI_LOG_DBG("xpsNhId %u \n", xpsNhId);

        retVal = xpsL3GetRouteNextHopDb(xpsDevId, xpsNhId, &L3NextHopEntry);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiNextHopDbInfoGet() failed with error code: %d\n",
                           retVal);
            return xpsStatus2SaiStatus(retVal);
        }
        value->s32 = L3NextHopEntry.serviceInstId;

    }

    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiSetNextHopAttrTunnelMac

sai_status_t xpSaiSetNextHopAttrTunnelMac(sai_object_id_t next_hop_id,
                                          sai_attribute_value_t value)
{
    sai_status_t        saiRetVal       = SAI_STATUS_SUCCESS;
    xpSaiNextHopEntry_t *pNextHopEntry  = NULL;
    uint32_t            xpsNhId             = 0;
    xpsDevice_t xpsDevId = xpSaiObjIdSwitchGet(next_hop_id);
    XP_STATUS retVal;
    xpsL3NextHopEntry_t L3NextHopEntry;


    XP_SAI_LOG_DBG("Calling %s \n", __FUNCTION__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(next_hop_id, SAI_OBJECT_TYPE_NEXT_HOP))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(next_hop_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiNextHopDbInfoGet(next_hop_id, &pNextHopEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiNextHopDbInfoGet() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    if (pNextHopEntry->nhType == SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP)
    {

        xpsNhId = (uint32_t)xpSaiObjIdValueGet(next_hop_id);

        XP_SAI_LOG_DBG("xpsNhId %u \n", xpsNhId);

        retVal = xpsL3GetRouteNextHopDb(xpsDevId, xpsNhId, &L3NextHopEntry);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiNextHopDbInfoGet() failed with error code: %d\n",
                           retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        xpSaiMacCopy(L3NextHopEntry.nextHop.macDa, value.mac);

        retVal = xpsL3SetRouteNextHopDb(xpsDevId, xpsNhId, &L3NextHopEntry);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error in xpsL3SetRouteNextHop an entry: error code: %d\n",
                           retVal);
            return xpsStatus2SaiStatus(retVal);
        }

    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiSetNextHopAttrTunnelVni

sai_status_t xpSaiSetNextHopAttrTunnelVni(sai_object_id_t next_hop_id,
                                          sai_attribute_value_t value)
{
    sai_status_t        saiRetVal       = SAI_STATUS_SUCCESS;
    xpSaiNextHopEntry_t *pNextHopEntry  = NULL;
    uint32_t            xpsNhId             = 0;
    xpsDevice_t xpsDevId = xpSaiObjIdSwitchGet(next_hop_id);
    XP_STATUS retVal;
    xpsL3NextHopEntry_t L3NextHopEntry;


    XP_SAI_LOG_DBG("Calling %s \n", __FUNCTION__);

    if (!XDK_SAI_OBJID_TYPE_CHECK(next_hop_id, SAI_OBJECT_TYPE_NEXT_HOP))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(next_hop_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiNextHopDbInfoGet(next_hop_id, &pNextHopEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiNextHopDbInfoGet() failed with error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }

    if (pNextHopEntry->nhType == SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP)
    {

        xpsNhId = (uint32_t)xpSaiObjIdValueGet(next_hop_id);

        XP_SAI_LOG_DBG("xpsNhId %u \n", xpsNhId);

        retVal = xpsL3GetRouteNextHopDb(xpsDevId, xpsNhId, &L3NextHopEntry);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpSaiNextHopDbInfoGet() failed with error code: %d\n",
                           retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        L3NextHopEntry.serviceInstId = value.s32;

        retVal = xpsL3SetRouteNextHopDb(xpsDevId, xpsNhId, &L3NextHopEntry);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error in xpsL3SetRouteNextHop an entry: error code: %d\n",
                           retVal);
            return xpsStatus2SaiStatus(retVal);
        }

    }

    return SAI_STATUS_SUCCESS;
}


static int32_t xpSaiNextHopEntryKeyComp(void* key1, void* key2)
{
    return ((int32_t)(((xpSaiNextHopEntry_t*)key1)->nhId) - (int32_t)(((
                                                                           xpSaiNextHopEntry_t*)key2)->nhId));
}

/* DB traversal APIs */
sai_status_t xpSaiNextHopGetFirst(xpSaiNextHopEntry_t **first)
{
    XP_STATUS xpStatus = xpsStateGetNextData(XP_SCOPE_DEFAULT, gsaiNextHopDbHdle,
                                             (xpsDbKey_t)NULL, (void **)first);

    if (xpStatus != XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(xpStatus);
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiNextHopGetNext(xpSaiNextHopEntry_t *current,
                                 xpSaiNextHopEntry_t **next)
{
    XP_STATUS xpStatus = XP_NO_ERR;

    xpStatus = xpsStateGetNextData(XP_SCOPE_DEFAULT, gsaiNextHopDbHdle,
                                   (xpsDbKey_t)current, (void **)next);
    if (xpStatus != XP_NO_ERR)
    {
        return xpsStatus2SaiStatus(xpStatus);
    }

    return SAI_STATUS_SUCCESS;
}

XP_STATUS xpSaiNextHopInit(xpsDevice_t xpSaiDevId)
{
    XP_STATUS retVal  = XP_NO_ERR;
    gsaiNextHopDbHdle = XPSAI_NEXTHOP_DB_HNDL;

    retVal = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "Nexthop Db", XPS_GLOBAL,
                                &xpSaiNextHopEntryKeyComp, gsaiNextHopDbHdle);
    if (retVal != XP_NO_ERR)
    {
        gsaiNextHopDbHdle = XPS_STATE_INVALID_DB_HANDLE;
        XP_SAI_LOG_ERR("Failed to register SAI NextHop DB\n");
        return retVal;
    }

    return retVal;
}

XP_STATUS xpSaiNextHopDeInit(xpsDevice_t xpSaiDevId)
{
    XP_STATUS retVal = XP_NO_ERR;

    retVal = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT, &gsaiNextHopDbHdle);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("De-Register Sai NextHop handler failed!\n");
        return retVal;
    }

    return retVal;
}

//Func: saiConvertNexthopEntryToXps
static
sai_status_t saiConvertNexthopEntryToXps(const xpSaiNextHopAttributesT
                                         *attributes,
                                         xpsL3HostEntry_t *xpsL3NeighborEntry)
{
    uint32_t ipV4Addr = 0;

    if ((*attributes).ip.ipaddr.addr_family == SAI_IP_ADDR_FAMILY_IPV4)
    {
        xpsL3NeighborEntry->type = XP_PREFIX_TYPE_IPV4;

        ipV4Addr = (*attributes).ip.ipaddr.addr.ip4;
        memcpy(xpsL3NeighborEntry->ipv4Addr, &ipV4Addr,
               sizeof(xpsL3NeighborEntry->ipv4Addr));

        XP_SAI_LOG_DBG("Ipv4Address " FMT_IP4  "\n",
                       PRI_IP4(xpsL3NeighborEntry->ipv4Addr));
    }
    else if ((*attributes).ip.ipaddr.addr_family == SAI_IP_ADDR_FAMILY_IPV6)
    {
        xpsL3NeighborEntry->type = XP_PREFIX_TYPE_IPV6;
        memcpy(xpsL3NeighborEntry->ipv6Addr, &((*attributes).ip.ipaddr.addr.ip6),
               sizeof(ipv6Addr_t));

        XP_SAI_LOG_DBG("Ipv6Address " FMT_IP6 "\n",
                       PRI_IP6(xpsL3NeighborEntry->ipv6Addr));
    }
    else
    {
        XP_SAI_LOG_ERR("Invalid addr family %d\n", (*attributes).ip.ipaddr.addr_family);
        return SAI_STATUS_INVALID_ATTR_VALUE_0;
    }

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiCreateNextHop

sai_status_t xpSaiHandleCreateNextHop(sai_object_id_t *next_hop_id,
                                      sai_object_id_t switch_id,
                                      uint32_t attr_count, const sai_attribute_t *attr_list)
{
    XP_STATUS                    retVal      = XP_NO_ERR;
    sai_status_t                 saiRetVal   = SAI_STATUS_SUCCESS;
    uint32_t                     xpsNhId     = 0;
    uint32_t                     vrfId       = 0;
    xpSaiNextHopAttributesT      attributes;
    xpsL3HostEntry_t             xpsL3NeighborEntry;
    xpsL3NextHopEntry_t          l3NextHopEntry;
    xpsInterfaceType_e           intfType;
    sai_attribute_value_t        servInstId;
    xpSaiRouterInterfaceDbEntryT *rifEntry   = NULL;
    xpSaiTunnelInfoT             *pTunnelEntry   = NULL;
    xpsDevice_t                  xpsDevId    = xpSaiObjIdSwitchGet(switch_id);
    xpSaiTableEntryCountDbEntry *entryCountCtxPtr = NULL;
    xpSaiNeighborEntry_t        saiL3NeighborKey;
    xpSaiNeighborEntry_t        *saiL3NeighborEntry;
    extern xpsDbHandle_t        gsaiNeighborDbHdle;
    uint32_t                    vniId     = 0;
    xpSaiSwitchEntry_t* pSwitchEntry = NULL;
    xpSaiSwitchEntry_t  key;

    //Global State maintainance
    xpSaiNextHopEntry_t *saiL3NextHopEntry = NULL;
    uint32_t count = 0;

    memset(&xpsL3NeighborEntry, 0, sizeof(xpsL3NeighborEntry));
    memset(&l3NextHopEntry, 0, sizeof(xpsL3NextHopEntry_t));
    memset(&attributes, 0, sizeof(attributes));

    if (next_hop_id == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiAttrCheck(attr_count, attr_list,
                               NEXT_HOP_VALIDATION_ARRAY_SIZE, next_hop_attribs,
                               SAI_COMMON_API_CREATE);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", saiRetVal);
        return saiRetVal;
    }

    saiRetVal = xpSaiUpdateNextHopAttributeVals(attr_count, attr_list, &attributes);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        return saiRetVal;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(attributes.routerInterfaceId.oid,
                                  SAI_OBJECT_TYPE_ROUTER_INTERFACE) &&
        !XDK_SAI_OBJID_TYPE_CHECK(attributes.tunnelId.oid,
                                  SAI_OBJECT_TYPE_TUNNEL))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(attributes.routerInterfaceId.oid));
        for (uint32_t count = 0; count < attr_count; count++)
        {
            const sai_attribute_t* attribute = &attr_list[count];
            if (attribute->id == SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID)
            {
                return SAI_STATUS_INVALID_ATTR_VALUE_0 + SAI_STATUS_CODE(count);
            }
        }
    }

    if (attributes.type.s32 == SAI_NEXT_HOP_TYPE_IP)
    {
        if (xpSaiRouterInterfaceVirtualCheck(attributes.routerInterfaceId.oid))
        {
            XP_SAI_LOG_ERR("Virtual RIF OID received\n");
            return SAI_STATUS_INVALID_OBJECT_ID;
        }

        saiRetVal = xpSaiRouterInterfaceDbInfoGet(attributes.routerInterfaceId.oid,
                                                  &rifEntry);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiRouterInterfaceDbInfoGet() failed with error code: %d!\n",
                           saiRetVal);
            return saiRetVal;
        }
        //get the SW NHId
        retVal = xpsL3CreateRouteNextHop(1, &xpsNhId);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsL3CreateRouteNextHop failed: retVal %d \n", retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        saiRetVal = saiConvertNexthopEntryToXps(&attributes, &xpsL3NeighborEntry);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("saiConvertNexthopEntryToXps failed: saiRetVal %d \n",
                           saiRetVal);
            return saiRetVal;
        }

        retVal = xpsL3GetIntfVrf(xpsDevId, rifEntry->l3IntfId, &vrfId);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsL3GetIntfVrf failed: retVal %d \n", retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        retVal = xpsInterfaceGetType(rifEntry->l3IntfId, &intfType);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error in xpsInterfaceGetType: error code: %d\n", retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        /*
         * In SAI model, Nexthop entry points to the IP address of the neighbor.
         * To program the XPS Nexthop, fetch the neighbor data from neighbor DB.
         */
#if 0
        memset(&saiNeighborEntry, 0, sizeof(saiNeighborEntry));
        saiNeighborEntry.rifId = attributes.routerInterfaceId.oid;
        saiNeighborEntry.type = attributes.ip.ipaddr.addr_family;

    }
    else
    {
        memcpy(saiNeighborEntry.ipv6Addr, xpsL3NeighborEntry.ipv6Addr,
               sizeof(saiNeighborEntry.ipv6Addr));
    }

    saiRetVal = xpSaiNeighborEntryGet(&saiNeighborEntry);
#endif

    memset(&saiL3NeighborKey, 0, sizeof(saiL3NeighborKey));
    saiL3NeighborKey.rifId = attributes.routerInterfaceId.oid;
    saiL3NeighborKey.type = attributes.ip.ipaddr.addr_family;
    if (saiL3NeighborKey.type == SAI_IP_ADDR_FAMILY_IPV4)
    {
        memcpy(saiL3NeighborKey.ipv4Addr, xpsL3NeighborEntry.ipv4Addr,
               sizeof(saiL3NeighborKey.ipv4Addr));
    }
    else
    {
        memcpy(saiL3NeighborKey.ipv6Addr, xpsL3NeighborEntry.ipv6Addr,
               sizeof(saiL3NeighborKey.ipv6Addr));
    }

}
else if (attributes.type.s32 == SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP)
{

    saiRetVal = xpSaiTunnelGetCtxDb(xpsDevId, attributes.tunnelId.oid,
                                    &pTunnelEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiTunnelGetCtxDb() failed with error code: %d!\n",
                       saiRetVal);
        return saiRetVal;
    }

    //get the SW NHId
    retVal = xpsL3CreateRouteNextHop(1, &xpsNhId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3CreateRouteNextHop failed: retVal %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    saiRetVal = saiConvertNexthopEntryToXps(&attributes, &xpsL3NeighborEntry);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("saiConvertNexthopEntryToXps failed: saiRetVal %d \n",
                       saiRetVal);
        return saiRetVal;
    }

    l3NextHopEntry.nextHop.l3InterfaceId = pTunnelEntry->tnlIntfId;
    l3NextHopEntry.pktCmd = XP_PKTCMD_FWD;

    vniId = attributes.tunnelVni.s32;
    l3NextHopEntry.serviceInstId = vniId;
    if (!xpSaiEthAddrIsZero(attributes.tunnelMac.mac))
    {
        xpSaiMacCopy(l3NextHopEntry.nextHop.macDa,
                     attributes.tunnelMac.mac);
    }
    else
    {
        memset(&key, 0, sizeof(key));
        key.keyStaticDataType = SAI_SWITCH_STATIC_VARIABLES;

        retVal = xpsStateSearchData(XP_SCOPE_DEFAULT, xpSaiSwitchStaticDataDbHandle,
                                    &key, (void**)&pSwitchEntry);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsStateSearchData failed with status: %d!\n", retVal);
            return xpsStatus2SaiStatus(retVal);
        }

        if (pSwitchEntry == NULL)
        {
            XP_SAI_LOG_ERR("Could not find switch static variables entry!\n");
            return SAI_STATUS_ITEM_NOT_FOUND;
        }
        xpSaiMacCopy(l3NextHopEntry.nextHop.macDa,
                     pSwitchEntry->vxlan_default_router_mac);
    }
#if 1
    retVal = xpsL3SetRouteNextHopDb(xpsDevId, xpsNhId, &l3NextHopEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error in xpsL3SetRouteNextHop an entry: error code: %d\n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }
#endif

    saiRetVal = xpSaiGetTableEntryCountCtxDb(xpsDevId, &entryCountCtxPtr);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get Table Entry Count DB: error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }
    entryCountCtxPtr->nextHopEntries++;

    saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_NEXT_HOP, xpsDevId,
                                 (sai_uint64_t) xpsNhId, next_hop_id);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("SAI object can not be created.\n");
        return SAI_STATUS_FAILURE;
    }

    saiL3NextHopEntry = NULL;

    /* Allocate space for the nextHop entry data */
    retVal = xpsStateHeapMalloc(sizeof(xpSaiNextHopEntry_t),
                                (void**)&saiL3NextHopEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateHeapMalloc failed: retVal %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    memset(saiL3NextHopEntry, 0, sizeof(xpSaiNextHopEntry_t));
    if (xpsL3NeighborEntry.type == XP_PREFIX_TYPE_IPV4)
    {
        saiL3NextHopEntry->type = SAI_IP_ADDR_FAMILY_IPV4;
        memcpy(saiL3NextHopEntry->ipv4Addr, xpsL3NeighborEntry.ipv4Addr,
               sizeof(ipv4Addr_t));
    }
    else if (xpsL3NeighborEntry.type == XP_PREFIX_TYPE_IPV6)
    {
        saiL3NextHopEntry->type = SAI_IP_ADDR_FAMILY_IPV6;
        memcpy(saiL3NextHopEntry->ipv6Addr, xpsL3NeighborEntry.ipv6Addr,
               sizeof(ipv6Addr_t));
    }
    else
    {
        XP_SAI_LOG_ERR("Invalid parameter\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiL3NextHopEntry->nhId = xpsNhId;
    saiL3NextHopEntry->rifId = attributes.routerInterfaceId.oid;
    memcpy(&saiL3NextHopEntry->xpsNhCache, &l3NextHopEntry,
           sizeof(saiL3NextHopEntry->xpsNhCache));

    saiL3NextHopEntry->nhType = SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP;

    // Insert the nexthop entry structure into the databse, using the nhId as key
    // Add a new node
    retVal = xpsStateInsertData(XP_SCOPE_DEFAULT, gsaiNextHopDbHdle,
                                saiL3NextHopEntry);
    if (retVal != XP_NO_ERR)
    {
        // Free Allocated Memory
        xpsStateHeapFree((void*)saiL3NextHopEntry);
        XP_SAI_LOG_ERR("xpsStateInsertData failed: retVal %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    XP_SAI_LOG_DBG("Inserting neighbor Data in Global S/W DB success::saiL3NextHopEntry->nhId: %d \n\n",
                   saiL3NextHopEntry->nhId);

    return xpsStatus2SaiStatus(retVal);
}

/* Search for the node in global neighbor Tree database */
retVal = xpsStateSearchData(XP_SCOPE_DEFAULT, gsaiNeighborDbHdle,
                            (xpsDbKey_t)&saiL3NeighborKey, (void **) &saiL3NeighborEntry);
if (retVal != XP_NO_ERR)
{
    XP_SAI_LOG_ERR("xpSaiNeighborEntry search failed with error code %d\n", retVal);
    return xpsStatus2SaiStatus(retVal);

}
if (saiL3NeighborEntry == NULL)
{
    XP_SAI_LOG_NOTICE("Nbr not found with error code %d!\n", saiRetVal);
    /* Treated as neighbor miss case. Install a NH entry with Neighbor miss packet action. */
    saiRetVal = xpSaiConvertSaiPacketAction2xps((sai_packet_action_t)
                                                rifEntry->neighMissAction, &l3NextHopEntry.pktCmd);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiRouterInterfaceDbInfoGet() failed with error code: %d!\n",
                       saiRetVal);
        return saiRetVal;
    }
    l3NextHopEntry.nextHop.l3InterfaceId =
        rifEntry->l3IntfId; /* To avoid egress BD miss drops in XP pipeline */
    l3NextHopEntry.nextHop.egressIntfId = XPS_INTF_INVALID_ID;
}
else
{
    xpSaiMacCopy(l3NextHopEntry.nextHop.macDa, saiL3NeighborEntry->macDa);
    l3NextHopEntry.nextHop.egressIntfId = saiL3NeighborEntry->xpsEgressIntf;
    l3NextHopEntry.nextHop.l3InterfaceId = saiL3NeighborEntry->l3IntfId;
    l3NextHopEntry.pktCmd = saiL3NeighborEntry->resolvedPktCmd;

    if (intfType == XPS_SUBINTERFACE_ROUTER)
    {
        saiRetVal = xpSaiGetRouterInterfaceAttrVlanId(attributes.routerInterfaceId.oid,
                                                      &servInstId);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("Error in xpSaiGetRouterInterfaceAttrVlanId: error code: %d\n",
                           saiRetVal);
            return saiRetVal;
        }

        l3NextHopEntry.serviceInstId = (uint32_t)xpSaiObjIdValueGet(servInstId.oid);
    }
}

XP_SAI_LOG_DBG("%s:Service instance ID: %d \n", __FUNCTION__,
               l3NextHopEntry.serviceInstId);
XP_SAI_LOG_DBG("%s:Packet command: %d \n", __FUNCTION__, l3NextHopEntry.pktCmd);
XP_SAI_LOG_DBG("%s:L3 Interface Id: %d \n", __FUNCTION__,
               l3NextHopEntry.nextHop.l3InterfaceId);
XP_SAI_LOG_DBG("%s:MAC Addr " FMT_MAC "\n",
               __FUNCTION__, PRI_MAC(l3NextHopEntry.nextHop.macDa));
XP_SAI_LOG_DBG("%s:Egress Int/Port Id: %d \n", __FUNCTION__,
               l3NextHopEntry.nextHop.egressIntfId);

retVal = xpsStateGetCount(XP_SCOPE_DEFAULT, gsaiNextHopDbHdle, &count);
if (retVal != XP_NO_ERR)
{
    XP_SAI_LOG_ERR("Error in xpsStateGetCount, error code: %d\n", retVal);
    return xpsStatus2SaiStatus(retVal);
}

//Check if nextHop already exists in sai next hop db.
//If exists, then return item already exists error.
if ((saiL3NeighborEntry) && (saiL3NeighborEntry->nhId != INVALID_INDEX))
{
    XP_SAI_LOG_NOTICE("neigh entry exists with nhID %d\n",
                      saiL3NeighborEntry->nhId);
    return SAI_STATUS_ITEM_ALREADY_EXISTS;
}
#if 0
for (uint32_t i=0; i < count; i++)
{
    retVal = xpsStateGetNextData(XP_SCOPE_DEFAULT, gsaiNextHopDbHdle,
                                 (xpsDbKey_t)saiL3NextHopEntry, (void **) &saiL3NextHopEntry);
    if (retVal != XP_NO_ERR || saiL3NextHopEntry == NULL)
    {
        XP_SAI_LOG_ERR("Error in xpsStateGetNextData, error code: %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    /*Need to check the attribute addr_family with SAI_IP_ADDR_FAMILY_IPV4 or SAI_IP_ADDR_FAMILY_IPV6 also to verify the nh is exists or not*/
    if ((saiL3NextHopEntry->type == SAI_IP_ADDR_FAMILY_IPV4) &&
        (attributes.ip.ipaddr.addr_family == SAI_IP_ADDR_FAMILY_IPV4))
    {
        if ((!memcmp(saiL3NextHopEntry->ipv4Addr, xpsL3NeighborEntry.ipv4Addr, 4)) &&
            (saiL3NextHopEntry->rifId == attributes.routerInterfaceId.oid))
        {
            return SAI_STATUS_ITEM_ALREADY_EXISTS;
        }
    }
    else if ((saiL3NextHopEntry->type == SAI_IP_ADDR_FAMILY_IPV6) &&
             (attributes.ip.ipaddr.addr_family == SAI_IP_ADDR_FAMILY_IPV6))
    {
        if ((!memcmp(saiL3NextHopEntry->ipv6Addr, xpsL3NeighborEntry.ipv6Addr, 16)) &&
            (saiL3NextHopEntry->rifId == attributes.routerInterfaceId.oid))
        {
            return SAI_STATUS_ITEM_ALREADY_EXISTS;
        }
    }
}
#endif

//Program this macDa,egressIntfId and intId for this xpsNhId in H/W
//create only SW instance of the NH,
//program to HW only when a route/ecmp uses the entry
#if 1
retVal = xpsL3SetRouteNextHopDb(xpsDevId, xpsNhId, &l3NextHopEntry);
if (retVal != XP_NO_ERR)
{
    XP_SAI_LOG_ERR("Error in xpsL3SetRouteNextHop an entry: error code: %d\n",
                   retVal);
    return xpsStatus2SaiStatus(retVal);
}
#endif

saiRetVal = xpSaiGetTableEntryCountCtxDb(xpsDevId, &entryCountCtxPtr);
if (saiRetVal != SAI_STATUS_SUCCESS)
{
    XP_SAI_LOG_ERR("Failed to get Table Entry Count DB: error code: %d\n",
                   saiRetVal);
    return saiRetVal;
}
entryCountCtxPtr->nextHopEntries++;

saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_NEXT_HOP, xpsDevId,
                             (sai_uint64_t) xpsNhId, next_hop_id);
if (saiRetVal != SAI_STATUS_SUCCESS)
{
    XP_SAI_LOG_ERR("SAI object can not be created.\n");
    return SAI_STATUS_FAILURE;
}

saiL3NextHopEntry = NULL;

/* Allocate space for the nextHop entry data */
retVal = xpsStateHeapMalloc(sizeof(xpSaiNextHopEntry_t),
                            (void**)&saiL3NextHopEntry);
if (retVal != XP_NO_ERR)
{
    XP_SAI_LOG_ERR("xpsStateHeapMalloc failed: retVal %d \n", retVal);
    return xpsStatus2SaiStatus(retVal);
}

memset(saiL3NextHopEntry, 0, sizeof(xpSaiNextHopEntry_t));
if (xpsL3NeighborEntry.type == XP_PREFIX_TYPE_IPV4)
{
    saiL3NextHopEntry->type = SAI_IP_ADDR_FAMILY_IPV4;
    memcpy(saiL3NextHopEntry->ipv4Addr, xpsL3NeighborEntry.ipv4Addr,
           sizeof(ipv4Addr_t));
}
else if (xpsL3NeighborEntry.type == XP_PREFIX_TYPE_IPV6)
{
    saiL3NextHopEntry->type = SAI_IP_ADDR_FAMILY_IPV6;
    memcpy(saiL3NextHopEntry->ipv6Addr, xpsL3NeighborEntry.ipv6Addr,
           sizeof(ipv6Addr_t));
}
else
{
    XP_SAI_LOG_ERR("Invalid parameter\n");
    return SAI_STATUS_INVALID_PARAMETER;
}

saiL3NextHopEntry->vrfId = vrfId;
saiL3NextHopEntry->nhId = xpsNhId;
saiL3NextHopEntry->rifId = attributes.routerInterfaceId.oid;
memcpy(&saiL3NextHopEntry->xpsNhCache, &l3NextHopEntry,
       sizeof(saiL3NextHopEntry->xpsNhCache));

saiL3NextHopEntry->nhType = SAI_NEXT_HOP_TYPE_IP;

// Insert the nexthop entry structure into the databse, using the nhId as key
// Add a new node
retVal = xpsStateInsertData(XP_SCOPE_DEFAULT, gsaiNextHopDbHdle,
                            saiL3NextHopEntry);
if (retVal != XP_NO_ERR)
{
    // Free Allocated Memory
    xpsStateHeapFree((void*)saiL3NextHopEntry);
    XP_SAI_LOG_ERR("xpsStateInsertData failed: retVal %d \n", retVal);
    return xpsStatus2SaiStatus(retVal);
}

XP_SAI_LOG_DBG("Inserting neighbor Data in Global S/W DB success::saiL3NextHopEntry->nhId: %d \n\n",
               saiL3NextHopEntry->nhId);

/*update neigh DB with the new NHid
    if not update, write a new entry to DB*/
if ((saiL3NeighborEntry) && (saiL3NeighborEntry->nhId == INVALID_INDEX))
{
    saiL3NeighborEntry->nhId = xpsNhId;
    XP_SAI_LOG_NOTICE("updated neigh entry with nhID %d\n", xpsNhId);
    return xpsStatus2SaiStatus(XP_NO_ERR);
}
else if (!saiL3NeighborEntry)
{
    /* Allocate space for the neighbor entry data */
    if ((retVal = (XP_STATUS)xpsStateHeapMalloc(sizeof(xpSaiNeighborEntry_t),
                                                (void**)&saiL3NeighborEntry)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateHeapMalloc failed: retVal %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    memset(saiL3NeighborEntry, 0, sizeof(xpSaiNeighborEntry_t));
    if (xpsL3NeighborEntry.type == XP_PREFIX_TYPE_IPV4)
    {
        saiL3NeighborEntry->type = SAI_IP_ADDR_FAMILY_IPV4;
        memcpy(saiL3NeighborEntry->ipv4Addr, xpsL3NeighborEntry.ipv4Addr,
               sizeof(ipv4Addr_t));
    }
    else
    {
        saiL3NeighborEntry->type = SAI_IP_ADDR_FAMILY_IPV6;
        memcpy(saiL3NeighborEntry->ipv6Addr, xpsL3NeighborEntry.ipv6Addr,
               sizeof(ipv6Addr_t));
    }

    saiL3NeighborEntry->rifId = attributes.routerInterfaceId.oid;
    saiL3NeighborEntry->nhId = xpsNhId;

    /* Insert the neighbor entry structure into the databse, using the Ip addr and rif_id as key */
    if ((retVal = xpsStateInsertData(XP_SCOPE_DEFAULT, gsaiNeighborDbHdle,
                                     saiL3NeighborEntry)) != XP_NO_ERR)
    {
        // Free Allocated Memory
        xpsStateHeapFree((void*)saiL3NeighborEntry);
        return retVal;
    }

    XP_SAI_LOG_NOTICE("Created new neighbor Data in Global S/W DB nhId: %d type %d rif %"
                      PRIx64 " ",
                      saiL3NeighborEntry->nhId, saiL3NeighborEntry->type, saiL3NeighborEntry->rifId);
    if (saiL3NeighborEntry->type == SAI_IP_ADDR_FAMILY_IPV4)
    {
        XP_SAI_LOG_NOTICE("ipaddr " FMT_IP4"\n",
                          PRI_IP4(saiL3NeighborEntry->ipv4Addr));
    }
    else
    {
        XP_SAI_LOG_NOTICE("ipv6 addr " FMT_IP6 "\n",
                          PRI_IP6(saiL3NeighborEntry->ipv6Addr));
    }

}
return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiCreateNextHop(sai_object_id_t *next_hop_id,
                                sai_object_id_t switch_id,
                                uint32_t attr_count, const sai_attribute_t *attr_list)
{
    sai_status_t status;

    xpSaiNextHopLock();
    status = xpSaiHandleCreateNextHop(next_hop_id, switch_id, attr_count,
                                      attr_list);
    xpSaiNextHopUnlock();

    return status;
}

//Func: xpSaiRemoveNextHop

sai_status_t xpSaiHandleRemoveNextHop(sai_object_id_t next_hop_id)
{
    XP_STATUS           retVal              = XP_NO_ERR;
    sai_status_t        saiRetVal           = SAI_STATUS_SUCCESS;
    uint32_t            xpsNhId             = 0;
    xpSaiNextHopEntry_t *saiL3NextHopEntry  = NULL;
    xpSaiNextHopEntry_t saiL3NextHopEntryKey;
    xpSaiTableEntryCountDbEntry *entryCountCtxPtr = NULL;

    xpsDevice_t xpsDevId = xpSaiObjIdSwitchGet(next_hop_id);

    if (!XDK_SAI_OBJID_TYPE_CHECK(next_hop_id, SAI_OBJECT_TYPE_NEXT_HOP))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u)\n",
                       xpSaiObjIdTypeGet(next_hop_id));
        return SAI_STATUS_INVALID_OBJECT_TYPE;
    }

    xpsNhId = (uint32_t)xpSaiObjIdValueGet(next_hop_id);

    XP_SAI_LOG_DBG("xpsNhId %u \n", xpsNhId);

    saiRetVal = xpSaiIsNextHopInUse(xpsDevId, xpsNhId);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Next hop already in use, nhId: %d, retVal %d \n", xpsNhId,
                       saiRetVal);
        return saiRetVal;
    }

    /* Clear HW table record */
    retVal = xpsL3ClearRouteNextHop(xpsDevId, xpsNhId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3ClearRouteNextHop failed: retVal %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    saiRetVal = xpSaiGetTableEntryCountCtxDb(xpsDevId, &entryCountCtxPtr);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get Table Entry Count DB: error code: %d\n",
                       saiRetVal);
        return saiRetVal;
    }
    entryCountCtxPtr->nextHopEntries--;

    /* Remove NH id from allocator */
    retVal = xpsL3DestroyRouteNextHop(1, xpsNhId);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsL3DestroyRouteNextHop failed: retVal %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    //nhId used as key for nexthop-entry
    saiL3NextHopEntryKey.nhId = xpsNhId;

    retVal = xpsStateDeleteData(XP_SCOPE_DEFAULT, gsaiNextHopDbHdle,
                                (xpsDbKey_t)&saiL3NextHopEntryKey, (void **) &saiL3NextHopEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateDeleteData failed: retVal %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (saiL3NextHopEntry == NULL)
    {
        XP_SAI_LOG_ERR("Could not find next hop in DB\n");
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if (saiL3NextHopEntry->nhType == SAI_NEXT_HOP_TYPE_IP)
    {
        /*update unified neigh db with the Nh ID*/
        xpSaiNeighborEntry_t saiL3NeighborKey;
        xpSaiNeighborEntry_t *saiL3NeighborEntry;
        extern xpsDbHandle_t gsaiNeighborDbHdle;

        // get unified SAI neigh DB, which has NHid associated
        memset(&saiL3NeighborKey, 0, sizeof(xpSaiNeighborEntry_t));
        saiL3NeighborKey.rifId = saiL3NextHopEntry->rifId;
        saiL3NeighborKey.type = saiL3NextHopEntry->type;

        if (saiL3NeighborKey.type == SAI_IP_ADDR_FAMILY_IPV4)
        {
            memcpy(saiL3NeighborKey.ipv4Addr, saiL3NextHopEntry->ipv4Addr,
                   sizeof(ipv4Addr_t));
        }
        else
        {
            memcpy(saiL3NeighborKey.ipv6Addr, saiL3NextHopEntry->ipv6Addr,
                   sizeof(ipv6Addr_t));
        }

        /* Search for the node in global neighbor Tree database */
        retVal = xpsStateSearchData(XP_SCOPE_DEFAULT, gsaiNeighborDbHdle,
                                    (xpsDbKey_t)&saiL3NeighborKey, (void **) &saiL3NeighborEntry);
        if (retVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Error in NeighborDbHdle xpsStateSearchData : error code: %d\n",
                           retVal);
            return xpsStatus2SaiStatus(retVal);
        }
        if (saiL3NeighborEntry ==NULL)
        {
            /*this cannot happen, as update is given by neighbour module*/
            XP_SAI_LOG_ERR("Neigh DB has no entry\n");
            return xpsStatus2SaiStatus(XP_ERR_KEY_NOT_FOUND);
        }
        XP_SAI_LOG_INFO("removed nhid %d from neigh DB\n", xpsNhId);
        saiL3NeighborEntry->nhId = INVALID_INDEX;
    }
    /*now release mem*/
    retVal = xpsStateHeapFree(saiL3NextHopEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateHeapMalloc failed: retVal %d \n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiRemoveNextHop(sai_object_id_t next_hop_id)
{
    sai_status_t status;

    xpSaiNextHopLock();
    status = xpSaiHandleRemoveNextHop(next_hop_id);
    xpSaiNextHopUnlock();

    return status;
}

sai_status_t xpSaiIncrementNextHopRouteRefCount(uint32_t nhId, bool isRouteNH)
{
    XP_STATUS    retVal        = XP_NO_ERR;
    xpSaiNextHopEntry_t *saiL3NextHopEntry  = NULL;
    xpSaiNextHopEntry_t saiL3NextHopEntryKey;

    memset(&saiL3NextHopEntryKey, 0, sizeof(xpSaiNextHopEntry_t));

    saiL3NextHopEntryKey.nhId = nhId;

    // Search for the node in global nexthop Tree database
    retVal = xpsStateSearchData(XP_SCOPE_DEFAULT, gsaiNextHopDbHdle,
                                (xpsDbKey_t)&saiL3NextHopEntryKey, (void **) &saiL3NextHopEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error in xpsStateSearchData : error code: %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (saiL3NextHopEntry == NULL)
    {
        XP_SAI_LOG_ERR("Could not find next hop in DB\n");
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if (isRouteNH)
    {
        ++saiL3NextHopEntry->routeRefCount;
    }
    else
    {
        ++saiL3NextHopEntry->aclRefCount;
    }
    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiDecrementNextHopRouteRefCount(uint32_t nhId, bool isRouteNH)
{
    XP_STATUS    retVal        = XP_NO_ERR;
    xpSaiNextHopEntry_t *saiL3NextHopEntry  = NULL;
    xpSaiNextHopEntry_t saiL3NextHopEntryKey;

    memset(&saiL3NextHopEntryKey, 0, sizeof(xpSaiNextHopEntry_t));

    saiL3NextHopEntryKey.nhId = nhId;

    // Search for the node in global nexthop Tree database
    retVal = xpsStateSearchData(XP_SCOPE_DEFAULT, gsaiNextHopDbHdle,
                                (xpsDbKey_t)&saiL3NextHopEntryKey, (void **) &saiL3NextHopEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error in xpsStateSearchData : error code: %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (saiL3NextHopEntry == NULL)
    {
        XP_SAI_LOG_ERR("Could not find next hop in DB\n");
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if (isRouteNH)
    {
        --saiL3NextHopEntry->routeRefCount;
    }
    else
    {
        --saiL3NextHopEntry->aclRefCount;
    }

    return SAI_STATUS_SUCCESS;

}

sai_status_t xpSaiIsNextHopInUse(xpsDevice_t xpsDevId, uint32_t xpsNhId)
{
    XP_STATUS           retVal              = XP_NO_ERR;
    uint32_t            nhGrpId             = 0;
    xpSaiNextHopEntry_t *saiL3NextHopEntry  = NULL;
    xpSaiNextHopEntry_t saiL3NextHopEntryKey;

    /* Check whether xpsNhId exists in any of the NH group */
    retVal = xpsL3GetRouteNextHopNextHopGroup(xpsDevId, xpsNhId, &nhGrpId);
    if (retVal != XP_ERR_NOT_FOUND)
    {
        if (retVal == XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("nhId : %d is already a member of nhGrpId: %d \n", xpsNhId,
                           nhGrpId);
            return SAI_STATUS_OBJECT_IN_USE;
        }
        else
        {
            XP_SAI_LOG_ERR("xpsL3GetRouteNextHopNextHopGroup failed: retVal %d \n", retVal);
            return xpsStatus2SaiStatus(retVal);
        }
    }

    memset(&saiL3NextHopEntryKey, 0, sizeof(xpSaiNextHopEntry_t));

    saiL3NextHopEntryKey.nhId = xpsNhId;

    // Search for the node in global nexthop Tree database
    retVal = xpsStateSearchData(XP_SCOPE_DEFAULT, gsaiNextHopDbHdle,
                                (xpsDbKey_t)&saiL3NextHopEntryKey, (void **) &saiL3NextHopEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error in xpsStateSearchData : error code: %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (saiL3NextHopEntry == NULL)
    {
        XP_SAI_LOG_ERR("Could not find next hop in DB\n");
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if ((saiL3NextHopEntry->routeRefCount > 0) ||
        (saiL3NextHopEntry->aclRefCount > 0))
    {
        XP_SAI_LOG_ERR("nhId : %d is part of route table data, cannot remove Next Hop \n",
                       xpsNhId);
        return SAI_STATUS_OBJECT_IN_USE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiIsAclNextHopInUse(xpsDevice_t xpsDevId, uint32_t xpsNhId)
{
    XP_STATUS           retVal              = XP_NO_ERR;
    uint32_t            nhGrpId             = 0;
    xpSaiNextHopEntry_t *saiL3NextHopEntry  = NULL;
    xpSaiNextHopEntry_t saiL3NextHopEntryKey;

    /* Check whether xpsNhId exists in any of the NH group */
    retVal = xpsL3GetRouteNextHopNextHopGroup(xpsDevId, xpsNhId, &nhGrpId);
    if (retVal != XP_ERR_NOT_FOUND)
    {
        if (retVal == XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("nhId : %d is already a member of nhGrpId: %d \n", xpsNhId,
                           nhGrpId);
            return SAI_STATUS_OBJECT_IN_USE;
        }
        else
        {
            XP_SAI_LOG_ERR("xpsL3GetRouteNextHopNextHopGroup failed: retVal %d \n", retVal);
            return xpsStatus2SaiStatus(retVal);
        }
    }

    memset(&saiL3NextHopEntryKey, 0, sizeof(xpSaiNextHopEntry_t));

    saiL3NextHopEntryKey.nhId = xpsNhId;

    // Search for the node in global nexthop Tree database
    retVal = xpsStateSearchData(XP_SCOPE_DEFAULT, gsaiNextHopDbHdle,
                                (xpsDbKey_t)&saiL3NextHopEntryKey, (void **) &saiL3NextHopEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error in xpsStateSearchData : error code: %d\n", retVal);
        return xpsStatus2SaiStatus(retVal);
    }

    if (saiL3NextHopEntry == NULL)
    {
        XP_SAI_LOG_ERR("Could not find next hop in DB\n");
        return SAI_STATUS_INVALID_OBJECT_ID;
    }

    if (saiL3NextHopEntry->aclRefCount > 0)
    {
        XP_SAI_LOG_ERR("nhId : %d is part of ACL table data, cannot remove Next Hop \n",
                       xpsNhId);
        return SAI_STATUS_OBJECT_IN_USE;
    }

    return SAI_STATUS_SUCCESS;
}
//Func: xpSaiSetNextHopAttribute
//Currently there are no settable attributes, all attributes are CREATE_ONLY

sai_status_t xpSaiHandleSetNextHopAttribute(sai_object_id_t next_hop_id,
                                            const sai_attribute_t *attr)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiSetNextHopAttribute\n");

    retVal = xpSaiAttrCheck(1, attr,
                            NEXT_HOP_VALIDATION_ARRAY_SIZE, next_hop_attribs,
                            SAI_COMMON_API_SET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", retVal);
        return retVal;
    }
    switch (attr->id)
    {
        case SAI_NEXT_HOP_ATTR_TUNNEL_MAC:
            {
                return xpSaiSetNextHopAttrTunnelMac(next_hop_id, attr->value);
            }
        case SAI_NEXT_HOP_ATTR_TUNNEL_VNI:
            {
                return xpSaiSetNextHopAttrTunnelVni(next_hop_id, attr->value);
            }
        default:
            {
                XP_SAI_LOG_ERR("Failed to get %d\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    return retVal;
}

sai_status_t xpSaiSetNextHopAttribute(sai_object_id_t next_hop_id,
                                      const sai_attribute_t *attr)
{
    sai_status_t status;

    xpSaiNextHopLock();
    status = xpSaiHandleSetNextHopAttribute(next_hop_id, attr);
    xpSaiNextHopUnlock();

    return status;
}

//Func: xpSaiGetNextHopAttribute

sai_status_t xpSaiGetNextHopAttribute(sai_object_id_t next_hop_id,
                                      sai_attribute_t *attr, uint32_t attr_index)
{
    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    switch (attr->id)
    {
        case SAI_NEXT_HOP_ATTR_IP:
            {
                return xpSaiGetNextHopAttrIp(next_hop_id, &attr->value);
            }

        case SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID:
            {
                return xpSaiGetNextHopAttrRouterInterfaceId(next_hop_id, &attr->value);
            }

        case SAI_NEXT_HOP_ATTR_TYPE:
            {
                return xpSaiGetNextHopAttrType(next_hop_id, &attr->value);
            }
        case SAI_NEXT_HOP_ATTR_TUNNEL_MAC:
            {
                return xpSaiGetNextHopAttrTunnelMac(next_hop_id, &attr->value);
            }
        case SAI_NEXT_HOP_ATTR_TUNNEL_VNI:
            {
                return xpSaiGetNextHopAttrTunnelVni(next_hop_id, &attr->value);
            }
        default:
            {
                XP_SAI_LOG_ERR("Failed to get %d\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    return SAI_STATUS_SUCCESS;
}


//Func: xpSaiGetNextHopAttributes

static sai_status_t xpSaiHandleGetNextHopAttributes(sai_object_id_t next_hop_id,
                                                    uint32_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    retVal = xpSaiAttrCheck(attr_count, attr_list,
                            NEXT_HOP_VALIDATION_ARRAY_SIZE, next_hop_attribs,
                            SAI_COMMON_API_GET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Attribute check failed with error %d\n", retVal);
        return retVal;
    }

    for (uint32_t count = 0; count < attr_count; count++)
    {
        retVal = xpSaiGetNextHopAttribute(next_hop_id, &attr_list[count], count);
        if (SAI_STATUS_SUCCESS != retVal)
        {
            XP_SAI_LOG_ERR("xpSaiGetNextHopAttribute failed\n");
            return retVal;
        }
    }

    return retVal;
}

static sai_status_t xpSaiGetNextHopAttributes(sai_object_id_t next_hop_id,
                                              uint32_t attr_count, sai_attribute_t *attr_list)
{
    sai_status_t status;

    xpSaiNextHopLock();
    status = xpSaiHandleGetNextHopAttributes(next_hop_id, attr_count, attr_list);
    xpSaiNextHopUnlock();

    return status;
}

//Func: xpSaiBulkGetNextHopAttributes

sai_status_t xpSaiBulkGetNextHopAttributes(sai_object_id_t id,
                                           uint32_t *attr_count, sai_attribute_t *attr_list)
{
    sai_status_t     saiRetVal  = SAI_STATUS_SUCCESS;
    uint32_t         idx        = 0;
    uint32_t         maxcount   = 0;

    XP_SAI_LOG_DBG("Calling %s \n", __FUNCNAME__);

    /* Check incoming parameters */
    if (!XDK_SAI_OBJID_TYPE_CHECK(id, SAI_OBJECT_TYPE_NEXT_HOP))
    {
        XP_SAI_LOG_ERR("Wrong object type received(%u).", xpSaiObjIdTypeGet(id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((attr_count == NULL) || (attr_list == NULL))
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    saiRetVal = xpSaiMaxCountNextHopAttribute(&maxcount);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Could not get max attribute count!\n");
        return SAI_STATUS_FAILURE;
    }

    if (*attr_count < maxcount)
    {
        *attr_count = maxcount;
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    for (uint32_t count = 0; count < maxcount; count++)
    {
        attr_list[idx].id = SAI_NEXT_HOP_ATTR_START + count;
        saiRetVal = xpSaiGetNextHopAttribute(id, &attr_list[idx], count);

        if (saiRetVal == SAI_STATUS_SUCCESS)
        {
            idx++;
        }
    }
    *attr_count = idx;

    return SAI_STATUS_SUCCESS;
}

//Func: xpSaiNextHopApiInit

XP_STATUS xpSaiNextHopApiInit(uint64_t flag,
                              const sai_service_method_table_t* adapHostServiceMethodTable)
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;

    XP_SAI_LOG_DBG("Calling xpSaiNextHopApiInit\n");

    saiRetVal = xpSaiNextHopInitLock();
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to init SAI Nexthop lock\n");
        return XP_ERR_RESOURCE_NOT_AVAILABLE;
    }

    _xpSaiNextHopApi = (sai_next_hop_api_t *) xpMalloc(sizeof(sai_next_hop_api_t));
    if (NULL == _xpSaiNextHopApi)
    {
        XP_SAI_LOG_ERR("Error: allocation failed for _xpSaiNextHopApi\n");
        return XP_ERR_MEM_ALLOC_ERROR;
    }
    _xpSaiNextHopApi->create_next_hop = xpSaiCreateNextHop;
    _xpSaiNextHopApi->remove_next_hop = xpSaiRemoveNextHop;
    _xpSaiNextHopApi->set_next_hop_attribute = xpSaiSetNextHopAttribute;
    _xpSaiNextHopApi->get_next_hop_attribute = xpSaiGetNextHopAttributes;

    saiRetVal = xpSaiApiRegister(SAI_API_NEXT_HOP, (void*)_xpSaiNextHopApi);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to register next hop API\n");
        return XP_ERR_ARRAY_OUT_OF_BOUNDS;
    }

    return retVal;
}


//Func: xpSaiNextHopApiDeinit

XP_STATUS xpSaiNextHopApiDeinit()
{
    XP_STATUS retVal = XP_NO_ERR;
    sai_status_t saiRetVal;

    XP_SAI_LOG_DBG("Calling xpSaiNextHopApiDeinit\n");

    xpFree(_xpSaiNextHopApi);
    _xpSaiNextHopApi = NULL;

    saiRetVal = xpSaiNextHopInitLock();
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Error: Failed to init SAI Nexthop lock\n");
        return XP_ERR_RESOURCE_NOT_AVAILABLE;
    }

    return retVal;
}


// Func: Check if the nexthop Entry uses the neighbor

bool xpSaiNextHopUsesNeighbor(const xpSaiNextHopEntry_t *nhEntry,
                              const sai_neighbor_entry_t *nbrEntry)
{
    if ((nhEntry == NULL) || (nbrEntry == NULL))
    {
        XP_SAI_LOG_ERR("Invalid arguments.\n");
        return false;
    }

    if (nbrEntry->rif_id != nhEntry->rifId)
    {
        return false;
    }

    if (nbrEntry->ip_address.addr_family != nhEntry->type)
    {
        return false;
    }

    if (nbrEntry->ip_address.addr_family == SAI_IP_ADDR_FAMILY_IPV4)
    {
        uint32_t ipv4Addr;
        xpSaiIpCopy((uint8_t*)&ipv4Addr, (uint8_t*)&nbrEntry->ip_address.addr.ip4,
                    SAI_IP_ADDR_FAMILY_IPV4);
        return (COMPARE_IPV4_ADDR_T(&ipv4Addr, &nhEntry->ipv4Addr) == 0) ? true : false;
    }
    else
    {
        ipv6Addr_t ipv6Addr;
        xpSaiIpCopy((uint8_t*)&ipv6Addr, (uint8_t*)&nbrEntry->ip_address.addr.ip6,
                    SAI_IP_ADDR_FAMILY_IPV6);
        return (COMPARE_IPV6_ADDR_T(&ipv6Addr, &nhEntry->ipv6Addr) == 0) ? true : false;
    }
}

sai_status_t xpSaiNextHopProgramXpsNextHop(uint32_t nhId,
                                           xpsL3NextHopEntry_t *xpsNhEntry)
{
    XP_STATUS xpStatus = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiGetDevId();

    XP_SAI_LOG_DBG("Programming xps nexthop with parameters [TBD]\n");

    xpStatus = xpsL3SetRouteNextHopAndNextHopGroup(xpsDevId, nhId, xpsNhEntry);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to program XPS Nexthop entry for nhId: %d Err code %d \n",
                       nhId, xpStatus);
        return xpsStatus2SaiStatus(xpStatus);
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiNextHopGetXpsNextHop(uint32_t nhId,
                                       xpsL3NextHopEntry_t *xpsNhEntry)
{
    XP_STATUS xpStatus = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiGetDevId();

    XP_SAI_LOG_DBG("Programming xps nexthop with parameters [TBD]\n");
    xpStatus = xpsL3GetRouteNextHop(xpsDevId, nhId, xpsNhEntry);
    if (xpStatus != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get the XPS Nexthop entry for nhId: %d\n", nhId);
        return xpsStatus2SaiStatus(xpStatus);
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiNextHopNeighborChangeNotificationHandler(
    const xpSaiNeighborChangeNotif_t *nbrNotif)
{
    sai_status_t saiStatus = SAI_STATUS_SUCCESS;
    xpSaiNextHopEntry_t *currentNh = NULL;
    xpsL3NextHopEntry_t xpsNhEntry;
    xpsPktCmd_e pktCmd = XP_PKTCMD_DROP;
    macAddr_t macDa;
    xpsInterfaceId_t egrIntf = 0;
    uint32_t serviceInstId = 0;
    XP_STATUS           retVal      = XP_NO_ERR;


    memset(&xpsNhEntry, '\0', sizeof(xpsNhEntry));
    memset(&macDa, '\0', sizeof(macDa));

    XP_SAI_LOG_INFO("xpSaiNextHopNeighborChangeNotificationHandler notifType %d st %d mac "
                    FMT_MAC " egrInt %d\n",
                    nbrNotif->notifType, nbrNotif->props.state,
                    PRI_MAC(nbrNotif->props.macDa),
                    nbrNotif->props.egressIntf);
    switch (nbrNotif->notifType)
    {
        case XP_SAI_CREATED:
            {
                // If the SAI neighbour is not fully resolved (DA + phy intf),
                // set pktCmd is TRAP to facilitate neighbor resolution. But, if
                // the neighbor pktCmd is Drop, it should take the precedence.
                // If phy intf is unresolved (No FDB), cmd is set according to
                // SAI attr fdbUnicastMissAction.
                if (nbrNotif->props.state == XP_SAI_NEIGHBOR_UNRESOLVED)
                {
                    pktCmd = (nbrNotif->props.pktCmd == XP_PKTCMD_DROP)?XP_PKTCMD_DROP:
                             XP_PKTCMD_TRAP;
                }
                else
                {
                    if (nbrNotif->props.macDaValid)
                    {
                        memcpy(macDa, nbrNotif->props.macDa, sizeof(macDa));
                    }

                    if (nbrNotif->props.egrIntfValid)
                    {
                        egrIntf = nbrNotif->props.egressIntf;
                    }

                    if (nbrNotif->props.pktCmdValid)
                    {
                        pktCmd = nbrNotif->props.pktCmd;
                    }

                    if (nbrNotif->props.serviceInstIdValid)
                    {
                        serviceInstId = nbrNotif->props.serviceInstId;
                    }

                }
                break;
            }

        case XP_SAI_REMOVED:
            {
                // If the SAI neighbor got removed, set the pktCmd to TRAP to facilitate neighbor resolution.
                pktCmd = XP_PKTCMD_TRAP;
                break;
            }

        case XP_SAI_PROPERTY_CHANGED:
            {
                if (nbrNotif->props.macDaValid)
                {
                    memcpy(macDa, nbrNotif->props.macDa, sizeof(macDa));
                }

                if (nbrNotif->props.egrIntfValid)
                {
                    egrIntf = nbrNotif->props.egressIntf;
                }

                if (nbrNotif->props.pktCmdValid)
                {
                    pktCmd = nbrNotif->props.pktCmd;
                }

                if (nbrNotif->props.serviceInstIdValid)
                {
                    serviceInstId = nbrNotif->props.serviceInstId;
                }

                break;
            }
    }
    xpSaiNeighborEntry_t saiL3NeighborKey;
    xpSaiNeighborEntry_t *saiL3NeighborEntry;
    xpSaiNextHopEntry_t saiL3NextHopEntryKey;
    extern xpsDbHandle_t gsaiNeighborDbHdle;

    // get unified SAI neigh DB, which has NHid associated
    memset(&saiL3NeighborKey, 0, sizeof(xpSaiNeighborEntry_t));
    if ((saiL3NeighborKey.type = nbrNotif->neighborKey->ip_address.addr_family) ==
        SAI_IP_ADDR_FAMILY_IPV4)
    {
        memcpy(saiL3NeighborKey.ipv4Addr, &nbrNotif->neighborKey->ip_address.addr.ip4,
               sizeof(ipv4Addr_t));
    }
    else
    {
        memcpy(saiL3NeighborKey.ipv6Addr, nbrNotif->neighborKey->ip_address.addr.ip6,
               sizeof(ipv6Addr_t));
    }

    saiL3NeighborKey.rifId = nbrNotif->neighborKey->rif_id;

    /* Search for the node in global neighbor Tree database */
    retVal = xpsStateSearchData(XP_SCOPE_DEFAULT, gsaiNeighborDbHdle,
                                (xpsDbKey_t)&saiL3NeighborKey, (void **) &saiL3NeighborEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error in NeighborDbHdle xpsStateSearchData : error code: %d\n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }
    if (saiL3NeighborEntry ==NULL)
    {
        /*this cannot happen, as updated is given by neighbour module*/
        XP_SAI_LOG_ERR("Neigh DB has no entry\n");
        return xpsStatus2SaiStatus(XP_ERR_KEY_NOT_FOUND);
    }
    saiL3NextHopEntryKey.nhId = saiL3NeighborEntry->nhId;
    retVal = xpsStateSearchData(XP_SCOPE_DEFAULT, gsaiNextHopDbHdle,
                                (xpsDbKey_t)&saiL3NextHopEntryKey, (void **) &currentNh);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error in NextHopDbHdle xpsStateSearchData : error code: %d\n",
                       retVal);
        return xpsStatus2SaiStatus(retVal);
    }
    if ((currentNh == NULL) || (currentNh->nhId == INVALID_INDEX))
    {
        /*before installing the NH for neigh, got an update
            so, it's like no NH using the Neigh*/
        XP_SAI_LOG_NOTICE("No Neigh data for NH %d. DB 0x%x nh %d \n",
                          saiL3NextHopEntryKey.nhId,
                          currentNh? currentNh : NULL,
                          currentNh? currentNh->nhId : INVALID_INDEX);
        return xpsStatus2SaiStatus(XP_NO_ERR);
    }
    else
    {
        // FIXME: This API xpSaiNextHopGetXpsNextHop doesnt work. Using the XPS
        // NH entry cache instead as temporary workaround.
#if 0
        if ((saiStatus = xpSaiNextHopGetXpsNextHop(currentNh->nhId,
                                                   &xpsNhEntry)) != SAI_STATUS_SUCCESS)
        {
            return saiStatus;
        }
#endif
        memcpy(&xpsNhEntry, &currentNh->xpsNhCache, sizeof(xpsNhEntry));

        // Update the relevant fields.
        if (nbrNotif->props.pktCmdValid ||
            nbrNotif->notifType ==
            XP_SAI_REMOVED) /* Removed case shall not contain pktCmd valid */
        {
            xpsNhEntry.pktCmd = pktCmd;
        }

        if (nbrNotif->props.egrIntfValid)
        {
            xpsNhEntry.nextHop.egressIntfId = egrIntf;
        }

        if (nbrNotif->props.serviceInstIdValid)
        {
            xpsNhEntry.serviceInstId = serviceInstId;
        }

        if (nbrNotif->props.macDaValid)
        {
            xpSaiMacCopy(xpsNhEntry.nextHop.macDa, macDa);
        }

        // Program the updated entry.
        if ((saiStatus = xpSaiNextHopProgramXpsNextHop(currentNh->nhId,
                                                       &xpsNhEntry)) != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiNextHopProgramXpsNextHop: Failed for nhId=%i.\n",
                           currentNh->nhId);
        }

        // Copy back the changed XPS NH to cache.
        memcpy(&currentNh->xpsNhCache, &xpsNhEntry, sizeof(xpsNhEntry));
        xpSaiTunnelOnNextHopChanged(currentNh->nhId);
        XP_SAI_LOG_DBG("Neigh notify updated the NH id %d \n", currentNh->nhId);
    }

    XP_SAI_LOG_DBG("Exit fn xpSaiNextHopNeighborChangeNotificationHandler\n");
    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiNextHopHandleNeighborChangeNotification(
    const xpSaiNeighborChangeNotif_t *nbrNotif)
{
    sai_status_t status;

    xpSaiNextHopLock();
    status = xpSaiNextHopNeighborChangeNotificationHandler(nbrNotif);
    xpSaiNextHopUnlock();

    return status;
}

sai_status_t xpSaiMaxCountNextHopAttribute(uint32_t *count)
{
    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    *count = SAI_NEXT_HOP_ATTR_END;

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiCountNextHopObjects(uint32_t *count)
{
    XP_STATUS  retVal  = XP_NO_ERR;

    if (!count)
    {
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpsStateGetCount(XP_SCOPE_DEFAULT, gsaiNextHopDbHdle, count);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get Count. return Value : %d\n", retVal);
    }
    return xpsStatus2SaiStatus(retVal);
}

sai_status_t xpSaiGetNextHopObjectList(uint32_t *object_count,
                                       sai_object_key_t *object_list)
{
    XP_STATUS       retVal      = XP_NO_ERR;
    sai_status_t    saiRetVal   = SAI_STATUS_SUCCESS;
    uint32_t        objCount    = 0;
    xpsDevice_t     devId       = xpSaiGetDevId();

    xpSaiNextHopEntry_t   *pNextHopNext    = NULL;

    saiRetVal = xpSaiCountNextHopObjects(&objCount);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get object count!\n");
        return saiRetVal;
    }

    if (objCount > *object_count)
    {
        *object_count = objCount;
        XP_SAI_LOG_ERR("Buffer overflow occured\n");
        return SAI_STATUS_BUFFER_OVERFLOW;
    }

    if (object_list == NULL)
    {
        *object_count = objCount;
        XP_SAI_LOG_ERR("Invalid parameter have been passed!\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    *object_count = objCount;

    for (uint32_t i = 0; i < *object_count; i++)
    {
        retVal = xpsStateGetNextData(XP_SCOPE_DEFAULT, gsaiNextHopDbHdle, pNextHopNext,
                                     (void **)&pNextHopNext);
        if (retVal != XP_NO_ERR || pNextHopNext == NULL)
        {
            XP_SAI_LOG_ERR("Failed to retrieve NextHop object, error %d\n", retVal);
            return SAI_STATUS_FAILURE;
        }

        saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_NEXT_HOP, devId,
                                     (sai_uint64_t)pNextHopNext->nhId, &object_list[i].key.object_id);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("SAI objectId could not be created, error %d\n", saiRetVal);
            return saiRetVal;
        }
    }

    return SAI_STATUS_SUCCESS;
}

