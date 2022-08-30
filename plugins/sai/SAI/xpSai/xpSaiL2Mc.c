// xpSaiL2Mc.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiL2Mc.h"
#include "xpSaiRoute.h"
#include "xpSaiSwitch.h"
#include "xpSaiValidationArrays.h"
#include "xpsMulticast.h"
#include "xpsL3.h"

XP_SAI_LOG_REGISTER_API(SAI_API_L2MC);

static sai_l2mc_api_t* _xpSaiL2McApi;
static xpsDbHandle_t saiL2McDataDbHndl = XPS_STATE_INVALID_DB_HANDLE;

static sai_status_t xpSaiGetL2McPktCmdIndex(xpsDevice_t xpsDevId,
                                            xpsMulticastIPv4BridgeEntry_t *entry,
                                            uint32_t *index)
{
    XP_STATUS xpsStatus = XP_NO_ERR;
    uint32_t routeEntryIdx = 0xFFFFFFFF;
    xpSaiRouteStaticDbEntry     *staticVarDb    = NULL;
    if (!entry || !index)
    {
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((xpsStatus = xpSaiRouteGetStaticVariablesDb(&staticVarDb)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Preinstalled next hops are not ready.\n");
        return xpsStatus2SaiStatus(xpsStatus);
    }

    if (entry->pktCmd == XP_MC_PKTCMD_FWD)
    {
        xpsStatus = xpsMcL2GetRouteEntryIdx(xpsDevId, entry->multicastVifIdx,
                                            &routeEntryIdx);
        if (xpsStatus != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Multicast L2 Interface List id does not exist, l2IntfListId(%d)",
                  entry->multicastVifIdx);
            return xpsStatus2SaiStatus(xpsStatus);
        }
    }
    else if (entry->pktCmd == XP_MC_PKTCMD_DROP)
    {
        xpsStatus = xpsL3NextHopMapGet(staticVarDb->gSaiDefaultDropNhId,
                                       &routeEntryIdx);
        if (xpsStatus != XP_NO_ERR || routeEntryIdx  == INVALID_INDEX)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "get NH map failed");
            return xpsStatus2SaiStatus(xpsStatus);
        }
    }
    else if (entry->pktCmd == XP_MC_PKTCMD_TRAP)
    {
        xpsStatus = xpsL3NextHopMapGet(staticVarDb->gSaiDefaultTrapNhId,
                                       &routeEntryIdx);
        if (xpsStatus != XP_NO_ERR || routeEntryIdx  == INVALID_INDEX)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "get NH map failed");
            return xpsStatus2SaiStatus(xpsStatus);
        }
    }
    else
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Invalid pktCmd : %d", entry->pktCmd);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    *index = routeEntryIdx;

    return SAI_STATUS_SUCCESS;
}

static sai_status_t xpSaiConvertL2McEntryToL2McBridgeEntry(
    sai_l2mc_entry_t l2mc_entry,
    xpSaiL2McAttributesT attributes,
    void *entry)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    uint32_t vlanId = xpSaiObjIdValueGet(l2mc_entry.bv_id);

    if (entry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument (enrty: NULL)\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (l2mc_entry.type == SAI_L2MC_ENTRY_TYPE_SG)
    {
        if (l2mc_entry.destination.addr_family != l2mc_entry.source.addr_family)
        {
            XP_SAI_LOG_ERR("Source and destination IP type is not matched \n");
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }

    if (l2mc_entry.destination.addr_family == SAI_IP_ADDR_FAMILY_IPV4)
    {
        xpsMulticastIPv4BridgeEntry_t *ipv4Entry = (xpsMulticastIPv4BridgeEntry_t*)
                                                   entry;
        ipv4Entry->vlanId = vlanId;
        ipv4Entry->bdId = vlanId;
        switch (l2mc_entry.type)
        {
            case SAI_L2MC_ENTRY_TYPE_SG:
                {
                    xpSaiIpCopy((uint8_t*) &ipv4Entry->sourceAddress,
                                (uint8_t*) &l2mc_entry.source.addr.ip4,
                                SAI_IP_ADDR_FAMILY_IPV4);
                    break;
                }
            case SAI_L2MC_ENTRY_TYPE_XG:
                {
                    /* in case of SAI_L2MC_ENTRY_TYPE_XG mode sourceAddress should be zeroes */
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("L2 Multicast entry type is not supported (mode: %u)\n",
                                   l2mc_entry.type);
                    return SAI_STATUS_INVALID_PARAMETER;
                }
        }
        xpSaiIpCopy((uint8_t*) &ipv4Entry->groupAddress,
                    (uint8_t*) &l2mc_entry.destination.addr.ip4,
                    SAI_IP_ADDR_FAMILY_IPV4);

        if (xpSaiConvertSaiPacketAction2xps((sai_packet_action_t) attributes.pktCmd,
                                            (xpPktCmd_e*) &ipv4Entry->pktCmd) != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiConvertSaiPacketAction2xps failed\n");
            return SAI_STATUS_INVALID_PARAMETER;
        }

        if (ipv4Entry->pktCmd == XP_MC_PKTCMD_FWD &&
            attributes.groupId != SAI_NULL_OBJECT_ID)
        {
            retVal = xpSaiGetL2McGroupId(l2mc_entry.switch_id, attributes.groupId, vlanId,
                                         &ipv4Entry->multicastVifIdx);
            if (retVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("xpSaiGetL2McGroupId failed (groupIdOid: %" PRIu64
                               ", retVal: %d)\n", attributes.groupId, retVal);
                return retVal;
            }
        }
        else
        {
            ipv4Entry->multicastVifIdx = xpSaiGetDefaultL2McGroupId();
        }

        ipv4Entry->mirrorMask = 0;
        ipv4Entry->countMode = 0;
        ipv4Entry->counterIdx = 0;
        ipv4Entry->isControl = 0;
        ipv4Entry->isStatic = 1;
    }
    else
    {
        xpsMulticastIPv6BridgeEntry_t *ipv6Entry = (xpsMulticastIPv6BridgeEntry_t*)
                                                   entry;

        ipv6Entry->vlanId = vlanId;
        ipv6Entry->bdId = vlanId;
        switch (l2mc_entry.type)
        {
            case SAI_L2MC_ENTRY_TYPE_SG:
                {
                    xpSaiIpCopy((uint8_t*) &ipv6Entry->sourceAddress,
                                (uint8_t*) &l2mc_entry.source.addr.ip6,
                                SAI_IP_ADDR_FAMILY_IPV6);
                    break;
                }
            case SAI_L2MC_ENTRY_TYPE_XG:
                {
                    /* in case of SAI_L2MC_ENTRY_TYPE_XG mode sourceAddress should be zeroes */
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("L2 Multicast entry type is not supported (mode: %u)\n",
                                   l2mc_entry.type);
                    return SAI_STATUS_INVALID_PARAMETER;
                }
        }
        xpSaiIpCopy((uint8_t*) &ipv6Entry->groupAddress,
                    (uint8_t*) &l2mc_entry.destination.addr.ip6,
                    SAI_IP_ADDR_FAMILY_IPV6);

        if (xpSaiConvertSaiPacketAction2xps((sai_packet_action_t) attributes.pktCmd,
                                            (xpPktCmd_e*) &ipv6Entry->pktCmd) != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiConvertSaiPacketAction2xps failed\n");
            return SAI_STATUS_INVALID_PARAMETER;
        }

        if (ipv6Entry->pktCmd == XP_MC_PKTCMD_FWD)
        {
            retVal = xpSaiGetL2McGroupId(l2mc_entry.switch_id, attributes.groupId, vlanId,
                                         &ipv6Entry->multicastVifIdx);
            if (retVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("xpSaiGetL2McGroupId failed (retVal: %d)\n", retVal);
                return retVal;
            }
        }
        else
        {
            ipv6Entry->multicastVifIdx = xpSaiGetDefaultL2McGroupId();
        }

        ipv6Entry->mirrorMask = 0;
        ipv6Entry->countMode = 0;
        ipv6Entry->counterIdx = 0;
        ipv6Entry->isControl = 0;
        ipv6Entry->isStatic = 1;
    }

    return retVal;
}

void xpSaiUpdateL2McAttributeVals(const uint32_t attr_count,
                                  const sai_attribute_t* attr_list, xpSaiL2McAttributesT* attributes)
{
    if ((attributes == NULL) || (attr_list == NULL))
    {
        XP_SAI_LOG_ERR("Invalid input parameter\n");
        return;
    }

    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_L2MC_ENTRY_ATTR_PACKET_ACTION:
                {
                    attributes->pktCmd = attr_list[count].value.s32;
                    break;
                }
            case SAI_L2MC_ENTRY_ATTR_OUTPUT_GROUP_ID:
                {
                    attributes->groupId = attr_list[count].value.oid;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to set %d\n", attr_list[count].id);
                }
        }
    }
}

#if 0
static sai_status_t xpSaiL2McGetBridgeEntryIndexAndAttributes(
    sai_l2mc_entry_t l2mc_entry, uint32_t *index, xpSaiL2McAttributesT *attributes)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiObjIdValueGet(l2mc_entry.switch_id);
    uint32_t groupId;
    xpPktCmd_e pktCmd;
    uint32_t vlanId = xpSaiObjIdValueGet(l2mc_entry.bv_id);

    if (l2mc_entry.type == SAI_L2MC_ENTRY_TYPE_SG)
    {
        if (l2mc_entry.destination.addr_family != l2mc_entry.source.addr_family)
        {
            XP_SAI_LOG_ERR("Source and destination IP type is not matched \n");
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }

    if (l2mc_entry.destination.addr_family == SAI_IP_ADDR_FAMILY_IPV4)
    {
        xpsMulticastIPv4BridgeEntry_t entry;

        memset(&entry, 0, sizeof(entry));

        entry.bdId = vlanId;

        switch (l2mc_entry.type)
        {
            case SAI_L2MC_ENTRY_TYPE_SG:
                {
                    xpSaiIpCopyAndReverse((uint8_t*) &entry.sourceAddress,
                                          (uint8_t*) &l2mc_entry.source.addr.ip4,
                                          SAI_IP_ADDR_FAMILY_IPV4);
                    break;
                }
            case SAI_L2MC_ENTRY_TYPE_XG:
                {
                    /* in case of SAI_L2MC_ENTRY_TYPE_XG mode sourceAddress should be zeroes */
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("L2 Multicast entry type is not supported (mode: %u)\n",
                                   l2mc_entry.type);
                    return SAI_STATUS_INVALID_PARAMETER;
                }

        }

        xpSaiIpCopyAndReverse((uint8_t*) &entry.groupAddress,
                              (uint8_t*) &l2mc_entry.destination.addr.ip4,
                              SAI_IP_ADDR_FAMILY_IPV4);

        xpRetVal = xpsMulticastGetIPv4BridgeEntryIndex(xpsDevId, &entry, index);
        if (xpRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsMulticastGetIPv4BridgeEntryIndex failed (retVal: %d)\n",
                           xpRetVal);
            return xpsStatus2SaiStatus(xpRetVal);
        }

        xpRetVal = xpsMulticastGetIPv4BridgeEntry(xpsDevId, *index, &entry);
        if (xpRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to get IPv4 Bridge Entry by Index (Index: %u, retVal: %d)\n",
                           *index, xpRetVal);
            return xpsStatus2SaiStatus(xpRetVal);
        }

        groupId = entry.multicastVifIdx;
        pktCmd = (xpPktCmd_e) entry.pktCmd;
    }
    else
    {
        xpsMulticastIPv6BridgeEntry_t entry;

        memset(&entry, 0, sizeof(entry));

        entry.bdId = vlanId;

        switch (l2mc_entry.type)
        {
            case SAI_L2MC_ENTRY_TYPE_SG:
                {
                    xpSaiIpCopyAndReverse((uint8_t*) &entry.sourceAddress,
                                          (uint8_t*) &l2mc_entry.source.addr.ip6,
                                          SAI_IP_ADDR_FAMILY_IPV6);
                    break;
                }
            case SAI_L2MC_ENTRY_TYPE_XG:
                {
                    /* in case of SAI_L2MC_ENTRY_TYPE_XG mode sourceAddress should be zeroes */
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("L2 Multicast entry type is not supported (mode: %u)\n",
                                   l2mc_entry.type);
                    return SAI_STATUS_INVALID_PARAMETER;
                }

        }
        xpSaiIpCopyAndReverse((uint8_t*) &entry.groupAddress,
                              (uint8_t*) &l2mc_entry.destination.addr.ip6,
                              SAI_IP_ADDR_FAMILY_IPV6);

        xpRetVal = xpsMulticastGetIPv6BridgeEntryIndex(xpsDevId, &entry, index);
        if (xpRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsMulticastGetIPv6BridgeEntryIndex failed (retVal: %d)\n",
                           xpRetVal);
            return xpsStatus2SaiStatus(xpRetVal);
        }

        xpRetVal = xpsMulticastGetIPv6BridgeEntry(xpsDevId, *index, &entry);
        if (xpRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to get IPv6 Bridge Entry by Index (Index: %u, retVal: %d)\n",
                           *index, xpRetVal);
            return xpsStatus2SaiStatus(xpRetVal);
        }

        groupId = entry.multicastVifIdx;
        pktCmd = (xpPktCmd_e) entry.pktCmd;
    }

    retVal = xpSaiConvertXpsPacketAction2Sai(pktCmd,
                                             (sai_packet_action_t*) &attributes->pktCmd);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertXpsPacketAction2Sai failed (retVal: %d)\n", retVal);
        return SAI_STATUS_ITEM_NOT_FOUND;
    }

    if (groupId != xpSaiGetDefaultL2McGroupId())
    {
        retVal = xpSaiGetL2McGroupIdOid(xpsDevId, groupId, vlanId,
                                        &attributes->groupId);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiGetL2McGroupIdOid failed (groupId: %u, vlanId: %u, retVal: %d)\n",
                           groupId, vlanId, retVal);
            return retVal;
        }
    }
    else
    {
        attributes->groupId = SAI_NULL_OBJECT_ID;
    }

    return retVal;
}
#endif

static sai_status_t xpSaiCreateL2McIpv4Entry(xpsDevice_t xpsDevId,
                                             sai_l2mc_entry_t l2mc_entry,
                                             xpSaiL2McAttributesT attributes)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;
    xpsMulticastIPv4BridgeEntry_t entry;
    uint32_t routeEntryIdx = 0xFFFFFFFF;

    memset(&entry, 0, sizeof(entry));

    retVal = xpSaiConvertL2McEntryToL2McBridgeEntry(l2mc_entry, attributes,
                                                    (void*) &entry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertL2McEntryToL2McBridgeEntry failed (retVal: %d)\n",
                       retVal);
        return retVal;
    }

    retVal = xpSaiGetL2McPktCmdIndex(xpsDevId, &entry, &routeEntryIdx);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiGetL2McPktCmdIndex failed (retVal : %d)\n",
                       retVal);
        return retVal;
    }

    xpRetVal = xpsMulticastAddIPv4BridgeEntry(xpsDevId, &entry, routeEntryIdx);
    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsMulticastAddIPv4BridgeEntry failed (retVal : %d)\n",
                       xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    return retVal;
}

#if 0
static sai_status_t xpSaiCreateL2McIpv6Entry(xpsDevice_t xpsDevId,
                                             sai_l2mc_entry_t l2mc_entry,
                                             xpSaiL2McAttributesT attributes)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;
    xpsMulticastIPv6BridgeEntry_t entry;
    xpsHashIndexList_t indexList;
    uint32_t xpsEntryIndex = -1;

    memset(&entry, 0, sizeof(entry));

    retVal = xpSaiConvertL2McEntryToL2McBridgeEntry(l2mc_entry, attributes,
                                                    (void*) &entry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertL2McEntryToL2McBridgeEntry failed (retVal: %d)\n",
                       retVal);
        return retVal;
    }

    xpRetVal = xpsMulticastGetIPv6BridgeEntryIndex(xpsDevId, &entry,
                                                   &xpsEntryIndex);
    if (xpRetVal == SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpsMulticastGetIPv6BridgeEntryIndex Entry is already present in the table, bdId %u, vlanId %u\n",
                       entry.bdId,
                       entry.vlanId);
        return SAI_STATUS_ITEM_ALREADY_EXISTS;
    }

    xpRetVal = xpsMulticastAddIPv6BridgeEntry(xpsDevId, &entry, &indexList);
    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsMulticastAddIPv6BridgeEntry failed (retVal: %d)\n",
                       xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    return retVal;
}
#endif

static XP_STATUS xpSaiL2McInsertDB(const sai_l2mc_entry_t
                                   *l2mc_entry, xpSaiL2McAttributesT* pAttributes)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpSaiL2McDbEntry_t *saiL2McEntry = NULL;

    if ((retVal = (XP_STATUS)xpsStateHeapMalloc(sizeof(xpSaiL2McDbEntry_t),
                                                (void**)&saiL2McEntry)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsStateHeapMalloc failed: retVal %d \n", retVal);
        return retVal;
    }

    memset(saiL2McEntry, 0, sizeof(xpSaiL2McDbEntry_t));
    saiL2McEntry->switch_id = l2mc_entry->switch_id;
    saiL2McEntry->bv_id = l2mc_entry->bv_id;
    saiL2McEntry->type = l2mc_entry->type;

    if (l2mc_entry->destination.addr_family == SAI_IP_ADDR_FAMILY_IPV4)
    {
        saiL2McEntry->destination.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
        memcpy(&saiL2McEntry->destination.addr.ip4,
               &l2mc_entry->destination.addr.ip4, sizeof(sai_ip4_t));
    }
    else
    {
        saiL2McEntry->destination.addr_family = SAI_IP_ADDR_FAMILY_IPV6;
        memcpy(&saiL2McEntry->destination.addr.ip6,
               &l2mc_entry->destination.addr.ip6, sizeof(sai_ip6_t));
    }

    saiL2McEntry->l2McAttr.pktCmd = pAttributes->pktCmd;
    saiL2McEntry->l2McAttr.groupId = pAttributes->groupId;

    /* Insert the sai-Route Entry structure into the databse, using the IP-addr and vr_id as key */
    if ((retVal = xpsStateInsertData(XP_SCOPE_DEFAULT, saiL2McDataDbHndl,
                                     saiL2McEntry)) != XP_NO_ERR)
    {
        // Free Allocated Memory
        xpsStateHeapFree(saiL2McEntry);
        XP_SAI_LOG_ERR("Error in xpsStateInsertData an entry: error code: %d\n",
                       retVal);
        return retVal;
    }
    return retVal;
}

static XP_STATUS xpSaiL2McGetDB(const sai_l2mc_entry_t
                                *l2mc_entry, xpSaiL2McDbEntry_t ** l2McCtxPtr)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpSaiL2McDbEntry_t saiL2McKey;
    memset(&saiL2McKey, 0, sizeof(xpSaiL2McDbEntry_t));

    /*sai-Route Key*/
    saiL2McKey.switch_id = l2mc_entry->switch_id;
    saiL2McKey.bv_id = l2mc_entry->bv_id;
    saiL2McKey.type = l2mc_entry->type;

    if (l2mc_entry->destination.addr_family == SAI_IP_ADDR_FAMILY_IPV4)
    {
        saiL2McKey.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
        memcpy(&saiL2McKey.destination.addr.ip4,
               &l2mc_entry->destination.addr.ip4, sizeof(sai_ip4_t));
    }
    else
    {
        saiL2McKey.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV6;
        memcpy(&saiL2McKey.destination.addr.ip6,
               &l2mc_entry->destination.addr.ip6, sizeof(sai_ip6_t));
    }

    retVal = xpsStateSearchData(XP_SCOPE_DEFAULT, saiL2McDataDbHndl,
                                (xpsDbKey_t)&saiL2McKey, (void **)l2McCtxPtr);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error in xpsStateSearchData an entry: error code: %d\n",
                       retVal);
        return retVal;
    }

    if (*l2McCtxPtr == NULL)
    {
        return XP_ERR_KEY_NOT_FOUND;
    }

    return retVal;
}

static XP_STATUS xpSaiL2McRemoveDB(const sai_l2mc_entry_t
                                   *l2mc_entry)
{
    XP_STATUS retVal = XP_NO_ERR;
    xpSaiL2McDbEntry_t saiL2McKey;
    xpSaiL2McDbEntry_t *saiL2McEntry = NULL;
    memset(&saiL2McKey, 0, sizeof(xpSaiL2McDbEntry_t));

    saiL2McKey.switch_id = l2mc_entry->switch_id;
    saiL2McKey.bv_id = l2mc_entry->bv_id;
    saiL2McKey.type = l2mc_entry->type;

    if (l2mc_entry->destination.addr_family == SAI_IP_ADDR_FAMILY_IPV4)
    {
        saiL2McKey.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
        memcpy(&saiL2McKey.destination.addr.ip4,
               &l2mc_entry->destination.addr.ip4, sizeof(sai_ip4_t));
    }
    else
    {
        saiL2McKey.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV6;
        memcpy(&saiL2McKey.destination.addr.ip6,
               &l2mc_entry->destination.addr.ip6, sizeof(sai_ip6_t));
    }

    retVal = xpsStateDeleteData(XP_SCOPE_DEFAULT, saiL2McDataDbHndl,
                                (xpsDbKey_t)&saiL2McKey, (void **)&saiL2McEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to delete state route-entry from DB\n");
        return retVal;
    }

    // Free the memory allocated for the corresponding state
    if ((retVal = xpsStateHeapFree(saiL2McEntry)) != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to free Route entry");
        return retVal;
    }

    return retVal;
}

static int32_t xpSaiL2McEntryKeyComp(void* _key1, void* _key2)
{
    xpSaiL2McDbEntry_t *key1 = (xpSaiL2McDbEntry_t *)_key1;
    xpSaiL2McDbEntry_t *key2 = (xpSaiL2McDbEntry_t *)_key2;

    if (key1->switch_id > key2->switch_id)
    {
        return 1;
    }
    else if (key1->switch_id < key2->switch_id)
    {
        return -1;
    }
    // key1->switch_id == key2->switch_id

    if (key1->bv_id > key2->bv_id)
    {
        return 1;
    }
    else if (key1->bv_id < key2->bv_id)
    {
        return -1;
    }
    // key1->vr_id == key2->vr_id
    if (key1->type > key2->type)
    {
        return 1;
    }
    else if (key1->type < key2->type)
    {
        return -1;
    }
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
    else
    {
        return COMPARE_IPV6_ADDR_T(key1->destination.addr.ip6,
                                   key2->destination.addr.ip6);
    }
}

sai_status_t xpSaiL2McInit(xpsDevice_t xpsDevId)
{
    XP_STATUS retVal = XP_NO_ERR;
    saiL2McDataDbHndl = XPSAI_L2MC_DB_HNDL;

    retVal = xpsStateRegisterDb(XP_SCOPE_DEFAULT, "Sai Route global variables",
                                XPS_GLOBAL,
                                &xpSaiL2McEntryKeyComp, saiL2McDataDbHndl);
    if (retVal != XP_NO_ERR)
    {
        saiL2McDataDbHndl = XPS_STATE_INVALID_DB_HANDLE;
        XP_SAI_LOG_ERR("Failed to register SAI Route Static DB\n");
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiL2McDeInit(xpsDevice_t xpsDevId)
{
    XP_STATUS retVal = XP_NO_ERR;

    retVal = xpsStateDeRegisterDb(XP_SCOPE_DEFAULT, &saiL2McDataDbHndl);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to de-register SAI Route Static DB\n");
        return xpsStatus2SaiStatus(retVal);
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t xpSaiCreateL2McEntry(const sai_l2mc_entry_t *l2mc_entry,
                                  uint32_t attr_count,
                                  const sai_attribute_t *attr_list)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpSaiL2McAttributesT attributes;
    xpsDevice_t xpsDevId;

    if (l2mc_entry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument (l2mc_entry: NULL)\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiAttrCheck(attr_count, attr_list, L2MC_ENTRY_VALIDATION_ARRAY_SIZE,
                            l2mc_entry_attribs, SAI_COMMON_API_CREATE);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Invalid attribute provided (retVal: %d)\n", retVal);
        return retVal;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(l2mc_entry->switch_id, SAI_OBJECT_TYPE_SWITCH))
    {
        XP_SAI_LOG_ERR("Wrong object type received (type: %u)\n",
                       xpSaiObjIdTypeGet(l2mc_entry->switch_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if ((!XDK_SAI_OBJID_TYPE_CHECK(l2mc_entry->bv_id, SAI_OBJECT_TYPE_BRIDGE))
        && (!XDK_SAI_OBJID_TYPE_CHECK(l2mc_entry->bv_id, SAI_OBJECT_TYPE_VLAN)))
    {
        XP_SAI_LOG_ERR("Wrong object type received (type: %u)\n",
                       xpSaiObjIdTypeGet(l2mc_entry->bv_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memset(&attributes, 0, sizeof(attributes));
    xpSaiUpdateL2McAttributeVals(attr_count, attr_list, &attributes);

    if (attributes.pktCmd == SAI_PACKET_ACTION_FORWARD &&
        attributes.groupId == SAI_NULL_OBJECT_ID)
    {
        XP_SAI_LOG_ERR("SAI_L2MC_ENTRY_ATTR_OUTPUT_GROUP_ID attribute value is required when SAI_L2MC_ENTRY_ATTR_PACKET_ACTION is set to SAI_PACKET_ACTION_FORWARD\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsDevId = xpSaiObjIdValueGet(l2mc_entry->switch_id);

    /* Activate L2 Multicast Group */
    if (XDK_SAI_OBJID_TYPE_CHECK(l2mc_entry->bv_id, SAI_OBJECT_TYPE_BRIDGE))
    {
        /* Activate L2 Multicast Group only for Dot1D */
        retVal = xpSaiActivateBridgeL2McGroup(xpsDevId, l2mc_entry->bv_id,
                                              attributes.groupId, NULL);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiActivateBridgeL2McGroup failed (groupIdOid: %" PRIu64
                           ", vlanId: %" PRIu64 ", retVal: %d)\n",
                           attributes.groupId, l2mc_entry->bv_id, retVal);
            return retVal;
        }
    }
    else if (XDK_SAI_OBJID_TYPE_CHECK(l2mc_entry->bv_id, SAI_OBJECT_TYPE_VLAN))
    {
        /* Activate L2 Multicast Group only for Dot1Q */
        retVal = xpSaiActivateVlanL2McGroup(xpsDevId, l2mc_entry->bv_id,
                                            attributes.groupId, NULL);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiActivateVlanL2McGroup failed (groupIdOid: %" PRIu64
                           ", vlanId: %" PRIu64 ", retVal: %d)\n",
                           attributes.groupId, l2mc_entry->bv_id, retVal);
            return retVal;
        }
    }
    else
    {
        XP_SAI_LOG_ERR("Wrong object type received: bvid= %lu\n", l2mc_entry->bv_id);
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (l2mc_entry->type == SAI_L2MC_ENTRY_TYPE_SG)
    {
        if (l2mc_entry->destination.addr_family != l2mc_entry->source.addr_family)
        {
            XP_SAI_LOG_ERR("Source and destination IP type is not matched \n");
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }

    if (l2mc_entry->source.addr_family == SAI_IP_ADDR_FAMILY_IPV4)
    {
        retVal = xpSaiCreateL2McIpv4Entry(xpsDevId, *l2mc_entry, attributes);
    }
    else
    {
        //retVal = xpSaiCreateL2McIpv6Entry(xpsDevId, *l2mc_entry, attributes);
        return SAI_STATUS_NOT_SUPPORTED;
    }

    if (retVal != SAI_STATUS_SUCCESS)
    {
        xpSaiDeActivateVlanL2McGroup(xpsDevId, l2mc_entry->bv_id, attributes.groupId);
        XP_SAI_LOG_ERR("Failed to apply L2 Multicast Entry to hardware (retVal: %d)\n",
                       retVal);
        return retVal;
    }

    retVal = xpSaiL2McInsertDB(l2mc_entry, &attributes);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error in xpSaiRouteInsertRouteDB: error code: %d\n", retVal);
        xpSaiDeActivateVlanL2McGroup(xpsDevId, l2mc_entry->bv_id, attributes.groupId);
        return retVal;
    }

    return retVal;
}

sai_status_t xpSaiRemoveL2McEntry(const sai_l2mc_entry_t *l2mc_entry)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;
    xpSaiL2McAttributesT attributes;
    xpsDevice_t xpsDevId;
    xpSaiL2McDbEntry_t *saiL2McEntry = NULL;
    xpsMulticastIPv4BridgeEntry_t entry;

    memset(&entry, 0, sizeof(entry));
    if (l2mc_entry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument (l2mc_entry: NULL)\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (l2mc_entry->type == SAI_L2MC_ENTRY_TYPE_SG)
    {
        if (l2mc_entry->destination.addr_family != l2mc_entry->source.addr_family)
        {
            XP_SAI_LOG_ERR("Source and destination IP type is not matched \n");
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }

    bool isIPv4 = l2mc_entry->destination.addr_family == SAI_IP_ADDR_FAMILY_IPV4;

    retVal = xpSaiL2McGetDB(l2mc_entry, &saiL2McEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error in xpSaiL2McGetDB an entry: error code: %d\n",
                       retVal);
        return (retVal);
    }

    attributes.pktCmd = saiL2McEntry->l2McAttr.pktCmd;
    attributes.groupId = saiL2McEntry->l2McAttr.groupId;
    retVal = xpSaiConvertL2McEntryToL2McBridgeEntry(*l2mc_entry, attributes,
                                                    (void*) &entry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertL2McEntryToL2McBridgeEntry failed (retVal: %d)\n",
                       retVal);
        return retVal;
    }

#if 0
    retVal = xpSaiL2McGetBridgeEntryIndexAndAttributes(*l2mc_entry, &index,
                                                       &attributes);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get Bridge entry index and attributes (retVal: %d)\n",
                       retVal);
        return retVal;
    }
#endif

    xpsDevId = xpSaiObjIdValueGet(l2mc_entry->switch_id);

    if (isIPv4)
    {
        xpRetVal = xpsMulticastRemoveIPv4BridgeEntry(xpsDevId, &entry);
    }
    else
    {
        return SAI_STATUS_NOT_SUPPORTED;
        //xpRetVal = xpsMulticastRemoveIPv6BridgeEntry(xpsDevId, index);
    }

    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to remove Multicast Bridge entry from hardware (retVal: %d)\n",
                       xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    if (attributes.groupId != SAI_NULL_OBJECT_ID)
    {
        retVal = xpSaiDeActivateVlanL2McGroup(xpsDevId, l2mc_entry->bv_id,
                                              attributes.groupId);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiDeActivateVlanL2McGroup failed (groupIdOid: %" PRIu64
                           ", retVal: %d)\n", attributes.groupId, retVal);
            return retVal;
        }
    }

    xpRetVal = xpSaiL2McRemoveDB(l2mc_entry);
    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpSaiL2McRemoveDB Failed : %d \n", xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    return retVal;
}

sai_status_t xpSaiSetL2McEntryAttribute(const sai_l2mc_entry_t *l2mc_entry,
                                        const sai_attribute_t *attr)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;
    xpSaiL2McAttributesT attributes;
    xpSaiL2McDbEntry_t *saiL2McEntry = NULL;
    xpsMulticastIPv4BridgeEntry_t entry;
    uint32_t routeEntryIdx = 0xFFFFFFFF;
    sai_attribute_t     attr_list[2];

    if ((l2mc_entry == NULL) || (attr == NULL))
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }
    memset(&entry, 0, sizeof(entry));
    memset(&attributes, 0, sizeof(attributes));

    if (l2mc_entry->type == SAI_L2MC_ENTRY_TYPE_SG)
    {
        if (l2mc_entry->destination.addr_family != l2mc_entry->source.addr_family)
        {
            XP_SAI_LOG_ERR("Source and destination IP type is not matched \n");
            return SAI_STATUS_INVALID_PARAMETER;
        }
    }

    bool isIPv4 = l2mc_entry->destination.addr_family == SAI_IP_ADDR_FAMILY_IPV4;
    xpsDevice_t xpsDevId = xpSaiObjIdValueGet(l2mc_entry->switch_id);

    retVal = xpSaiAttrCheck(1, attr, L2MC_ENTRY_VALIDATION_ARRAY_SIZE,
                            l2mc_entry_attribs, SAI_COMMON_API_SET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Invalid attribute provided (retVal: %d)\n", retVal);
        return retVal;
    }

    retVal = xpSaiL2McGetDB(l2mc_entry, &saiL2McEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error in xpSaiL2McGetDB an entry: error code: %d\n",
                       retVal);
        return (retVal);
    }

    if (saiL2McEntry == NULL)
    {
        XP_SAI_LOG_ERR("xpSaiL2McGetDB Failed\n");
        return xpsStatus2SaiStatus(XP_ERR_KEY_NOT_FOUND);
    }

    switch (attr->id)
    {
        case SAI_L2MC_ENTRY_ATTR_PACKET_ACTION:
            {
                if (saiL2McEntry->l2McAttr.pktCmd == attr->value.s32)
                {
                    break;
                }

                attributes.groupId = saiL2McEntry->l2McAttr.groupId;
                attributes.pktCmd = attr->value.s32;

                if (attributes.groupId == SAI_NULL_OBJECT_ID &&
                    attr->value.s32 == SAI_PACKET_ACTION_FORWARD)
                {
                    saiL2McEntry->l2McAttr.pktCmd = attr->value.s32;
                    XP_SAI_LOG_NOTICE("NULL L2MC GroupOid with FWD action. NOP!!!\n");
                    return SAI_STATUS_SUCCESS;
                }

                retVal = xpSaiConvertL2McEntryToL2McBridgeEntry(*l2mc_entry, attributes,
                                                                (void*) &entry);
                if (retVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("xpSaiConvertL2McEntryToL2McBridgeEntry failed (retVal: %d)\n",
                                   retVal);
                    return retVal;
                }

                retVal = xpSaiGetL2McPktCmdIndex(xpsDevId, &entry, &routeEntryIdx);
                if (retVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("xpSaiGetL2McPktCmdIndex failed (retVal : %d)\n",
                                   retVal);
                    return retVal;
                }

                /* De-activate/re-activate ??*/
                if (isIPv4)
                {
                    xpRetVal = xpsMulticastRemoveIPv4BridgeEntry(xpsDevId, &entry);

                    if (xpRetVal != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("xpsMulticastRemoveIPv4BridgeEntry failed  (retVal: %u)\n",
                                       xpRetVal);
                        return xpsStatus2SaiStatus(xpRetVal);
                    }

                    xpRetVal = xpsMulticastAddIPv4BridgeEntry(xpsDevId, &entry, routeEntryIdx);

                    if (xpRetVal != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("xpsMulticastAddIPv4BridgeEntry failed  (retVal: %u)\n",
                                       xpRetVal);
                        return xpsStatus2SaiStatus(xpRetVal);
                    }
                }
                else
                {

                    //     xpRetVal = xpsMulticastSetIpv6MulticastBridgeEntryPktCmd(xpsDevId, index,
                    //             (xpMulticastPktCmdType_e) ptkCmd);
                }
                saiL2McEntry->l2McAttr.pktCmd = attr->value.s32;

                break;
            }
        case SAI_L2MC_ENTRY_ATTR_OUTPUT_GROUP_ID:
            {
                attributes.groupId = saiL2McEntry->l2McAttr.groupId;
                attributes.pktCmd = saiL2McEntry->l2McAttr.pktCmd;

                attr_list[0].id = SAI_L2MC_ENTRY_ATTR_PACKET_ACTION;
                attr_list[0].value.s32 = saiL2McEntry->l2McAttr.pktCmd;

                attr_list[1].id = SAI_L2MC_ENTRY_ATTR_OUTPUT_GROUP_ID;
                attr_list[1].value.oid = attr->value.oid;

                if (!XDK_SAI_OBJID_TYPE_CHECK(attr->value.oid, SAI_OBJECT_TYPE_L2MC_GROUP))
                {
                    XP_SAI_LOG_ERR("Wrong object type received (type: %u)\n",
                                   xpSaiObjIdTypeGet(attr->value.oid));
                    return SAI_STATUS_INVALID_PARAMETER;
                }

                if (attr->value.oid == SAI_NULL_OBJECT_ID &&
                    attributes.pktCmd == SAI_PACKET_ACTION_FORWARD)
                {
                    XP_SAI_LOG_ERR("NULL groupOid for pkt action FORWARD\n");
                    return SAI_STATUS_INVALID_PARAMETER;
                }

                /* GroupId is the same, just return SUCCESS */
                if (attributes.groupId == attr->value.oid)
                {
                    break;
                }

                if (attributes.pktCmd == SAI_PACKET_ACTION_FORWARD)
                {
                    if (l2mc_entry->source.addr_family == SAI_IP_ADDR_FAMILY_IPV4)
                    {
                        retVal = xpSaiRemoveL2McEntry(l2mc_entry);
                        if (retVal != SAI_STATUS_SUCCESS)
                        {
                            XP_SAI_LOG_ERR("SET: xpSaiRemoveL2McEntry failed  (retVal: %u)\n",
                                           retVal);
                            return (retVal);
                        }

                        retVal = xpSaiCreateL2McEntry(l2mc_entry, 2, attr_list);;
                        if (retVal != SAI_STATUS_SUCCESS)
                        {
                            XP_SAI_LOG_ERR("SET:xpSaiCreateL2McEntry failed  (retVal: %u)\n",
                                           retVal);
                            return (retVal);
                        }
                    }
                }
                break;
            }
        default:
            {
                XP_SAI_LOG_ERR("Invalid parameter received %d\n", attr->id);
                return SAI_STATUS_INVALID_PARAMETER;
            }
    }

    return retVal;
}

sai_status_t xpSaiGetL2McEntryAttribute(const sai_l2mc_entry_t *l2mc_entry,
                                        uint32_t attr_count,
                                        sai_attribute_t *attr_list)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpSaiL2McDbEntry_t *saiL2McEntry = NULL;

    if (l2mc_entry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument (l2mc_entry: NULL)\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiAttrCheck(attr_count, attr_list, L2MC_ENTRY_VALIDATION_ARRAY_SIZE,
                            l2mc_entry_attribs, SAI_COMMON_API_GET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Invalid attribute provided (retVal: %d)\n", retVal);
        return retVal;
    }
    retVal = xpSaiL2McGetDB(l2mc_entry, &saiL2McEntry);
    if (retVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Error in xpSaiL2McGetDB an entry: error code: %d\n",
                       retVal);
        return (retVal);
    }

    if (saiL2McEntry == NULL)
    {
        XP_SAI_LOG_ERR("xpSaiL2McGetDB Failed\n");
        return xpsStatus2SaiStatus(XP_ERR_KEY_NOT_FOUND);
    }

    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_L2MC_ENTRY_ATTR_PACKET_ACTION:
                {
                    attr_list[count].value.s32 = saiL2McEntry->l2McAttr.pktCmd;

                    break;
                }
            case SAI_L2MC_ENTRY_ATTR_OUTPUT_GROUP_ID:
                {
                    attr_list[count].value.oid = saiL2McEntry->l2McAttr.groupId;

                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to get %d\n", attr_list[count].id);
                    return SAI_STATUS_INVALID_PARAMETER;
                }
        }
    }

    return retVal;
}

sai_status_t xpSaiL2McApiInit(uint64_t flag,
                              const sai_service_method_table_t* adapHostServiceMethodTable)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    _xpSaiL2McApi = (sai_l2mc_api_t *) xpMalloc(sizeof(sai_l2mc_api_t));

    if (_xpSaiL2McApi == NULL)
    {
        XP_SAI_LOG_ERR("Failed to allocate _xpSaiL2McApi\n");
        return SAI_STATUS_NO_MEMORY;
    }

    _xpSaiL2McApi->create_l2mc_entry = xpSaiCreateL2McEntry;
    _xpSaiL2McApi->remove_l2mc_entry = xpSaiRemoveL2McEntry;
    _xpSaiL2McApi->set_l2mc_entry_attribute = xpSaiSetL2McEntryAttribute;
    _xpSaiL2McApi->get_l2mc_entry_attribute = xpSaiGetL2McEntryAttribute;

    retVal = xpSaiApiRegister(SAI_API_L2MC, (void*)_xpSaiL2McApi);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to register L2MC API\n");
        return retVal;
    }

    return retVal;
}

sai_status_t xpSaiL2McApiDeinit()
{
    xpFree(_xpSaiL2McApi);
    _xpSaiL2McApi = NULL;

    return SAI_STATUS_SUCCESS;
}


