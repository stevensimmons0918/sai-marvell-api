// xpSaiIpmc.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiIpmc.h"
#include "xpsMulticast.h"
#include "xpSaiSwitch.h"
#include "xpSaiValidationArrays.h"

XP_SAI_LOG_REGISTER_API(SAI_API_IPMC);

static sai_ipmc_api_t* _xpSaiIpmcApi;

static sai_status_t xpSaiConvertIpmcEntryToIpmcRouteEntry(
    sai_ipmc_entry_t ipmc_entry,
    xpSaiIpmcAttributesT attributes,
    void *entry)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    uint32_t vrfId = xpSaiObjIdValueGet(ipmc_entry.vr_id);

    if (entry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument (enrty: NULL)\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (ipmc_entry.source.addr_family == SAI_IP_ADDR_FAMILY_IPV4)
    {
        xpsMulticastIPv4RouteEntry_t *ipv4Entry = (xpsMulticastIPv4RouteEntry_t*) entry;

        memset(ipv4Entry, 0, sizeof(xpsMulticastIPv4RouteEntry_t));

        switch (ipmc_entry.type)
        {
            case SAI_IPMC_ENTRY_TYPE_SG:
                {
                    xpSaiIpCopyAndReverse((uint8_t*) &ipv4Entry->sourceAddress,
                                          (uint8_t*) &ipmc_entry.source.addr.ip4,
                                          SAI_IP_ADDR_FAMILY_IPV4);
                    break;
                }
            case SAI_IPMC_ENTRY_TYPE_XG:
                {
                    /* in case of SAI_IPMC_ENTRY_TYPE_XG mode sourceAddress should be zeroes */
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("IP Multicast entry type is not supported (mode: %u)\n",
                                   ipmc_entry.type);
                    return SAI_STATUS_INVALID_PARAMETER;
                }
        }
        xpSaiIpCopyAndReverse((uint8_t*) &ipv4Entry->groupAddress,
                              (uint8_t*) &ipmc_entry.destination.addr.ip4,
                              SAI_IP_ADDR_FAMILY_IPV4);

        if (xpSaiConvertSaiPacketAction2xps((sai_packet_action_t) attributes.pktCmd,
                                            (xpPktCmd_e*) &ipv4Entry->pktCmd) != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiConvertSaiPacketAction2xps failed\n");
            return SAI_STATUS_INVALID_PARAMETER;
        }

        retVal = xpSaiGetRpfValue(attributes.rpfGroupId, &ipv4Entry->rpfValue,
                                  &ipv4Entry->rpfType);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiGetRpfValue failed (retVal: %u)\n", retVal);
            return retVal;
        }

        ipv4Entry->multicastVifIdx = xpSaiObjIdValueGet(attributes.groupId);
        ipv4Entry->vrfIdx = vrfId;
        ipv4Entry->rpfFailCmd = XP_MC_RPF_FAIL_CMD_DEFER_TO_EGRESS;
    }
    else
    {
        xpsMulticastIPv6RouteEntry_t *ipv6Entry = (xpsMulticastIPv6RouteEntry_t*) entry;

        memset(ipv6Entry, 0, sizeof(xpsMulticastIPv6RouteEntry_t));

        switch (ipmc_entry.type)
        {
            case SAI_IPMC_ENTRY_TYPE_SG:
                {
                    xpSaiIpCopyAndReverse((uint8_t*) &ipv6Entry->sourceAddress,
                                          (uint8_t*) &ipmc_entry.source.addr.ip6,
                                          SAI_IP_ADDR_FAMILY_IPV6);
                    break;
                }
            case SAI_IPMC_ENTRY_TYPE_XG:
                {
                    /* in case of SAI_IPMC_ENTRY_TYPE_XG mode sourceAddress should be zeroes */
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("IP Multicast entry type is not supported (mode: %u)\n",
                                   ipmc_entry.type);
                    return SAI_STATUS_INVALID_PARAMETER;
                }
        }
        xpSaiIpCopyAndReverse((uint8_t*) &ipv6Entry->groupAddress,
                              (uint8_t*) &ipmc_entry.destination.addr.ip6,
                              SAI_IP_ADDR_FAMILY_IPV6);

        if (xpSaiConvertSaiPacketAction2xps((sai_packet_action_t) attributes.pktCmd,
                                            (xpPktCmd_e*) &ipv6Entry->pktCmd) != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiConvertSaiPacketAction2xps failed\n");
            return SAI_STATUS_INVALID_PARAMETER;
        }

        retVal = xpSaiGetRpfValue(attributes.rpfGroupId, &ipv6Entry->rpfValue,
                                  &ipv6Entry->rpfType);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiGetRpfValue failed (retVal: %u)\n", retVal);
            return retVal;
        }

        ipv6Entry->multicastVifIdx = xpSaiObjIdValueGet(attributes.groupId);
        ipv6Entry->vrfIdx = vrfId;
        ipv6Entry->ipv6McL3DomainId = vrfId;
        ipv6Entry->rpfFailCmd = XP_MC_RPF_FAIL_CMD_DEFER_TO_EGRESS;
    }

    return retVal;
}

static void xpSaiUpdateIpmcAttributeVals(const uint32_t attr_count,
                                         const sai_attribute_t* attr_list, xpSaiIpmcAttributesT* attributes)
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
            case SAI_IPMC_ENTRY_ATTR_PACKET_ACTION:
                {
                    attributes->pktCmd = attr_list[count].value.s32;
                    break;
                }
            case SAI_IPMC_ENTRY_ATTR_OUTPUT_GROUP_ID:
                {
                    attributes->groupId = attr_list[count].value.oid;
                    break;
                }
            case SAI_IPMC_ENTRY_ATTR_RPF_GROUP_ID:
                {
                    attributes->rpfGroupId = attr_list[count].value.oid;
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("Failed to set %d\n", attr_list[count].id);
                }
        }
    }
}

static sai_status_t xpSaiIpmcGetRouteEntryIndexAndAttributes(
    sai_ipmc_entry_t ipmc_entry, uint32_t *index, xpSaiIpmcAttributesT *attributes)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;
    xpsDevice_t xpsDevId = xpSaiObjIdValueGet(ipmc_entry.switch_id);
    uint32_t groupId;
    xpPktCmd_e pktCmd;
    uint32_t vrfId = xpSaiObjIdValueGet(ipmc_entry.vr_id);

    if (ipmc_entry.source.addr_family == SAI_IP_ADDR_FAMILY_IPV4)
    {
        xpsMulticastIPv4RouteEntry_t entry;

        memset(&entry, 0, sizeof(entry));

        entry.vrfIdx = vrfId;

        switch (ipmc_entry.type)
        {
            case SAI_IPMC_ENTRY_TYPE_SG:
                {
                    xpSaiIpCopyAndReverse((uint8_t*) &entry.sourceAddress,
                                          (uint8_t*) &ipmc_entry.source.addr.ip4,
                                          SAI_IP_ADDR_FAMILY_IPV4);
                    break;
                }
            case SAI_IPMC_ENTRY_TYPE_XG:
                {
                    /* in case of SAI_IPMC_ENTRY_TYPE_XG mode sourceAddress should be zeroes */
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("IP Multicast entry type is not supported (mode: %u)\n",
                                   ipmc_entry.type);
                    return SAI_STATUS_INVALID_PARAMETER;
                }

        }

        xpSaiIpCopyAndReverse((uint8_t*) &entry.groupAddress,
                              (uint8_t*) &ipmc_entry.destination.addr.ip4,
                              SAI_IP_ADDR_FAMILY_IPV4);

        xpRetVal = xpsMulticastGetIPv4RouteEntryIndex(xpsDevId, &entry, index);
        if (xpRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsMulticastGetIPv4RouteEntryIndex failed (retVal: %d)\n",
                           xpRetVal);
            return xpsStatus2SaiStatus(xpRetVal);
        }

        xpRetVal = xpsMulticastGetIPv4RouteEntry(xpsDevId, *index, &entry);
        if (xpRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to get IPv4 Route Entry by Index (Index: %u, retVal: %d)\n",
                           *index, xpRetVal);
            return xpsStatus2SaiStatus(xpRetVal);
        }

        groupId = entry.multicastVifIdx;
        pktCmd = (xpPktCmd_e) entry.pktCmd;
    }
    else
    {
        xpsMulticastIPv6RouteEntry_t entry;

        memset(&entry, 0, sizeof(entry));

        entry.vrfIdx = vrfId;

        switch (ipmc_entry.type)
        {
            case SAI_IPMC_ENTRY_TYPE_SG:
                {
                    xpSaiIpCopyAndReverse((uint8_t*) &entry.sourceAddress,
                                          (uint8_t*) &ipmc_entry.source.addr.ip6,
                                          SAI_IP_ADDR_FAMILY_IPV6);
                    break;
                }
            case SAI_IPMC_ENTRY_TYPE_XG:
                {
                    /* in case of SAI_IPMC_ENTRY_TYPE_XG mode sourceAddress should be zeroes */
                    break;
                }
            default:
                {
                    XP_SAI_LOG_ERR("IP Multicast entry type is not supported (mode: %u)\n",
                                   ipmc_entry.type);
                    return SAI_STATUS_INVALID_PARAMETER;
                }

        }
        xpSaiIpCopyAndReverse((uint8_t*) &entry.groupAddress,
                              (uint8_t*) &ipmc_entry.destination.addr.ip6,
                              SAI_IP_ADDR_FAMILY_IPV6);

        xpRetVal = xpsMulticastGetIPv6RouteEntryIndex(xpsDevId, &entry, index);
        if (xpRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("xpsMulticastGetIPv6RouteEntryIndex failed (retVal: %d)\n",
                           xpRetVal);
            return xpsStatus2SaiStatus(xpRetVal);
        }

        xpRetVal = xpsMulticastGetIPv6RouteEntry(xpsDevId, *index, &entry);
        if (xpRetVal != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Failed to get IPv6 Route Entry by Index (Index: %u, retVal: %d)\n",
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

    if (groupId != SAI_NULL_OBJECT_ID)
    {
        retVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_IPMC_GROUP, xpsDevId, groupId,
                                  &attributes->groupId);
        if (retVal != SAI_STATUS_SUCCESS)
        {
            XP_SAI_LOG_ERR("xpSaiObjIdCreate failed (groupId: %u, retVal: %d)\n", groupId,
                           retVal);
            return retVal;
        }
    }
    else
    {
        attributes->groupId = SAI_NULL_OBJECT_ID;
    }

    return retVal;
}

static sai_status_t xpSaiCreateIpmcIpv4Entry(xpsDevice_t xpsDevId,
                                             sai_ipmc_entry_t ipmc_entry,
                                             xpSaiIpmcAttributesT attributes)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;
    xpsMulticastIPv4RouteEntry_t entry;
    xpsHashIndexList_t indexList;
    uint32_t xpsEntryIndex = -1;

    memset(&entry, 0, sizeof(entry));

    retVal = xpSaiConvertIpmcEntryToIpmcRouteEntry(ipmc_entry, attributes,
                                                   (void*) &entry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertIpmcEntryToIpmcRouteEntry failed (retVal: %d)\n",
                       retVal);
        return retVal;
    }

    xpRetVal = xpsMulticastGetIPv4RouteEntryIndex(xpsDevId, &entry, &xpsEntryIndex);
    if (xpRetVal == SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpsMulticastGetIPv4RouteEntryIndex Entry is already present in the table\n");
        return SAI_STATUS_ITEM_ALREADY_EXISTS;
    }

    xpRetVal = xpsMulticastAddIPv4RouteEntry(xpsDevId, &entry, &indexList);
    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsMulticastAddIPv4RouteEntry failed (retVal : %d)\n",
                       xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    if (indexList.size == 1)
    {
        retVal = xpSaiRpfGroupMcEntryHwIdxAddElement(xpsDevId, attributes.rpfGroupId,
                                                     indexList.index[0]);
    }
    else
    {
        //Multilevel Hash is not supported for L3 Mc so not implemented
    }

    return retVal;
}

static sai_status_t xpSaiCreateIpmcIpv6Entry(xpsDevice_t xpsDevId,
                                             sai_ipmc_entry_t ipmc_entry,
                                             xpSaiIpmcAttributesT attributes)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;
    xpsMulticastIPv6RouteEntry_t entry;
    xpsHashIndexList_t indexList;
    uint32_t xpsEntryIndex = -1;

    memset(&entry, 0, sizeof(entry));

    retVal = xpSaiConvertIpmcEntryToIpmcRouteEntry(ipmc_entry, attributes,
                                                   (void*) &entry);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiConvertIpmcEntryToIpmcRouteEntry failed (retVal: %d)\n",
                       retVal);
        return retVal;
    }

    xpRetVal = xpsMulticastGetIPv6RouteEntryIndex(xpsDevId, &entry, &xpsEntryIndex);
    if (xpRetVal == SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpsMulticastGetIPv6RouteEntryIndex entry is already present in the table\n");
        return SAI_STATUS_ITEM_ALREADY_EXISTS;
    }

    xpRetVal = xpsMulticastAddIPv6RouteEntry(xpsDevId, &entry, &indexList);
    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("xpsMulticastAddIPv6RouteEntry failed (retVal: %d)\n", xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    if (indexList.size == 1)
    {
        indexList.index[0] |= XP_SAI_RPF_GROUP_HW_IDX_IPV6;
        retVal = xpSaiRpfGroupMcEntryHwIdxAddElement(xpsDevId, attributes.rpfGroupId,
                                                     indexList.index[0]);
    }
    else
    {
        //Multilevel Hash is not supported for L3 Mc so not implemented
    }

    return retVal;
}

sai_status_t xpSaiCreateIpmcEntry(const sai_ipmc_entry_t *ipmc_entry,
                                  uint32_t attr_count,
                                  const sai_attribute_t *attr_list)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    xpSaiIpmcAttributesT attributes;
    xpsDevice_t xpsDevId;

    if (ipmc_entry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument (ipmc_entry: NULL)\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiAttrCheck(attr_count, attr_list, IPMC_ENTRY_VALIDATION_ARRAY_SIZE,
                            ipmc_entry_attribs, SAI_COMMON_API_CREATE);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Invalid attribute provided (retVal: %d)\n", retVal);
        return retVal;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(ipmc_entry->switch_id, SAI_OBJECT_TYPE_SWITCH))
    {
        XP_SAI_LOG_ERR("Wrong object type received (type: %u)\n",
                       xpSaiObjIdTypeGet(ipmc_entry->switch_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    if (!XDK_SAI_OBJID_TYPE_CHECK(ipmc_entry->vr_id,
                                  SAI_OBJECT_TYPE_VIRTUAL_ROUTER))
    {
        XP_SAI_LOG_ERR("Wrong object type received (type: %u)\n",
                       xpSaiObjIdTypeGet(ipmc_entry->vr_id));
        return SAI_STATUS_INVALID_PARAMETER;
    }

    memset(&attributes, 0, sizeof(attributes));
    xpSaiUpdateIpmcAttributeVals(attr_count, attr_list, &attributes);

    if (attributes.pktCmd == SAI_PACKET_ACTION_FORWARD &&
        attributes.groupId == SAI_NULL_OBJECT_ID)
    {
        XP_SAI_LOG_ERR("SAI_IPMC_ENTRY_ATTR_OUTPUT_GROUP_ID attribute value is required when SAI_IPMC_ENTRY_ATTR_PACKET_ACTION is set to SAI_PACKET_ACTION_FORWARD\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    xpsDevId = xpSaiObjIdValueGet(ipmc_entry->switch_id);

    if (ipmc_entry->source.addr_family == SAI_IP_ADDR_FAMILY_IPV4)
    {
        retVal = xpSaiCreateIpmcIpv4Entry(xpsDevId, *ipmc_entry, attributes);
    }
    else
    {
        retVal = xpSaiCreateIpmcIpv6Entry(xpsDevId, *ipmc_entry, attributes);
    }
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to apply IP Multicast Entry to hardware (retVal: %d)\n",
                       retVal);
        return retVal;
    }

    return retVal;
}

sai_status_t xpSaiRemoveIpmcEntry(const sai_ipmc_entry_t *ipmc_entry)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;
    uint32_t index = 0;
    xpSaiIpmcAttributesT attributes;
    xpsDevice_t xpsDevId;
    sai_object_id_t rpfGroupId = 0;

    if (ipmc_entry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument (ipmc_entry: NULL)\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    bool isIPv4 = ipmc_entry->source.addr_family == SAI_IP_ADDR_FAMILY_IPV4;

    retVal = xpSaiIpmcGetRouteEntryIndexAndAttributes(*ipmc_entry, &index,
                                                      &attributes);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get Route entry index and attributes (retVal: %d)\n",
                       retVal);
        return retVal;
    }

    xpsDevId = xpSaiObjIdValueGet(ipmc_entry->switch_id);

    if (isIPv4)
    {
        xpRetVal = xpsMulticastRemoveIPv4RouteEntry(xpsDevId, index);
    }
    else
    {
        xpRetVal = xpsMulticastRemoveIPv6RouteEntry(xpsDevId, index);
        index |= XP_SAI_RPF_GROUP_HW_IDX_IPV6;
    }

    if (xpRetVal != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to remove Multicast Route entry from hardware (retVal: %d)\n",
                       xpRetVal);
        return xpsStatus2SaiStatus(xpRetVal);
    }

    retVal = xpSaiRpfGroupMcEntryHwIdxGetRpfGroupOId(xpsDevId, index, &rpfGroupId);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiRpfGroupMcEntryHwIdxGetRpfGroupOId failed. (retVal: %d)\n",
                       retVal);
        return retVal;
    }

    retVal = xpSaiRpfGroupMcEntryHwIdxDelElement(xpsDevId, rpfGroupId, index);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("xpSaiRpfGroupMcEntryHwIdxDelElement failed. (retVal: %d)\n",
                       retVal);
        return retVal;
    }

    return retVal;
}

sai_status_t xpSaiSetIpmcEntryAttribute(const sai_ipmc_entry_t *ipmc_entry,
                                        const sai_attribute_t *attr)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    XP_STATUS xpRetVal = XP_NO_ERR;
    uint32_t index = 0;
    xpSaiIpmcAttributesT attributes;
    sai_object_id_t rpfGroupId = 0;

    if ((ipmc_entry == NULL) || (attr == NULL))
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    bool isIPv4 = ipmc_entry->source.addr_family == SAI_IP_ADDR_FAMILY_IPV4;
    xpsDevice_t xpsDevId = xpSaiObjIdValueGet(ipmc_entry->switch_id);

    retVal = xpSaiAttrCheck(1, attr, IPMC_ENTRY_VALIDATION_ARRAY_SIZE,
                            ipmc_entry_attribs, SAI_COMMON_API_SET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Invalid attribute provided (retVal: %d)\n", retVal);
        return retVal;
    }

    retVal = xpSaiIpmcGetRouteEntryIndexAndAttributes(*ipmc_entry, &index,
                                                      &attributes);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get Route entry data (saiRetVal: %d)\n", retVal);
        return retVal;
    }

    switch (attr->id)
    {
        case SAI_IPMC_ENTRY_ATTR_PACKET_ACTION:
            {
                xpPktCmd_e ptkCmd;

                retVal = xpSaiConvertSaiPacketAction2xps((sai_packet_action_t) attr->value.s32,
                                                         &ptkCmd);
                if (retVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to convert SAI packet action to XPS (retVal: %d)\n",
                                   retVal);
                    return retVal;
                }

                if (attributes.groupId == SAI_NULL_OBJECT_ID &&
                    attr->value.s32 == SAI_PACKET_ACTION_FORWARD)
                {
                    XP_SAI_LOG_ERR("SAI_IPMC_ENTRY_ATTR_OUTPUT_GROUP_ID attribute value is required when SAI_IPMC_ENTRY_ATTR_PACKET_ACTION is set to SAI_PACKET_ACTION_FORWARD\n");
                    return SAI_STATUS_INVALID_PARAMETER;
                }

                if (isIPv4)
                {
                    xpRetVal = xpsMulticastSetIpv4MulticastRouteEntryPktCmd(xpsDevId, index,
                                                                            (xpMulticastPktCmdType_e) ptkCmd);
                }
                else
                {
                    xpRetVal = xpsMulticastSetIpv6MulticastRouteEntryPktCmd(xpsDevId, index,
                                                                            (xpMulticastPktCmdType_e) ptkCmd);
                }

                if (xpRetVal != XP_NO_ERR)
                {
                    XP_SAI_LOG_ERR("Failed to set SAI_IPMC_ENTRY_ATTR_PACKET_ACTION attribute on hardware (retVal: %u)\n",
                                   xpRetVal);
                    return xpsStatus2SaiStatus(xpRetVal);
                }

                break;
            }
        case SAI_IPMC_ENTRY_ATTR_OUTPUT_GROUP_ID:
            {
                uint32_t groupId = 0;

                if (!XDK_SAI_OBJID_TYPE_CHECK(attr->value.oid, SAI_OBJECT_TYPE_IPMC_GROUP))
                {
                    XP_SAI_LOG_ERR("Wrong object type received (type: %u)\n",
                                   xpSaiObjIdTypeGet(attr->value.oid));
                    return SAI_STATUS_INVALID_PARAMETER;
                }

                /* GroupId is the same, just return SUCCESS */
                if (attributes.groupId == attr->value.oid)
                {
                    break;
                }

                groupId = xpSaiObjIdValueGet(attr->value.oid);
                if (isIPv4)
                {
                    xpRetVal = xpsMulticastSetIpv4MulticastRouteEntryVifIdx(xpsDevId, index,
                                                                            groupId);
                }
                else
                {
                    xpRetVal = xpsMulticastSetIpv6MulticastRouteEntryVifIdx(xpsDevId, index,
                                                                            groupId);
                }

                if (xpRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set SAI_IPMC_ENTRY_ATTR_OUTPUT_GROUP_ID attribute on hardware (retVal: %u)\n",
                                   xpRetVal);
                    return xpsStatus2SaiStatus(xpRetVal);
                }

                break;
            }

        case SAI_IPMC_ENTRY_ATTR_RPF_GROUP_ID:
            {
                uint32_t rpfValue = 0;
                xpMulticastRpfCheckType_e rpfType;
                xpMulticastRpfFailCmd_e rpfFailCmd = XP_MC_RPF_FAIL_CMD_DEFER_TO_EGRESS;

                if (!XDK_SAI_OBJID_TYPE_CHECK(attr->value.oid, SAI_OBJECT_TYPE_RPF_GROUP))
                {
                    XP_SAI_LOG_ERR("Wrong object type received (type: %u)\n",
                                   xpSaiObjIdTypeGet(attr->value.oid));
                    return SAI_STATUS_INVALID_PARAMETER;
                }

                retVal = xpSaiGetRpfValue(attr->value.oid, &rpfValue, &rpfType);
                if (retVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("xpSaiGetRpfValue failed (retVal: %u)\n", retVal);
                    return retVal;
                }

                if (isIPv4)
                {
                    xpRetVal = xpsMulticastSetIpv4MulticastRouteEntryRpfParams(xpsDevId, index,
                                                                               rpfType, rpfValue, rpfFailCmd);
                }
                else
                {
                    xpRetVal = xpsMulticastSetIpv6MulticastRouteEntryRpfParams(xpsDevId, index,
                                                                               rpfType, rpfValue, rpfFailCmd);
                    index |= XP_SAI_RPF_GROUP_HW_IDX_IPV6;
                }

                if (xpRetVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("Failed to set SAI_IPMC_ENTRY_ATTR_RPF_GROUP_ID attribute on hardware (retVal: %u)\n",
                                   xpRetVal);
                    return xpsStatus2SaiStatus(xpRetVal);
                }

                retVal = xpSaiRpfGroupMcEntryHwIdxGetRpfGroupOId(xpsDevId, index, &rpfGroupId);
                if (retVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("xpSaiRpfGroupMcEntryHwIdxGetRpfGroupOId failed. (retVal: %d)\n",
                                   retVal);
                    return retVal;
                }

                retVal = xpSaiRpfGroupMcEntryHwIdxDelElement(xpsDevId, rpfGroupId, index);
                if (retVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("xpSaiRpfGroupMcEntryHwIdxDelElement failed. (retVal: %d)\n",
                                   retVal);
                    return retVal;
                }

                retVal = xpSaiRpfGroupMcEntryHwIdxAddElement(xpsDevId, attr->value.oid, index);
                if (retVal != SAI_STATUS_SUCCESS)
                {
                    XP_SAI_LOG_ERR("xpSaiRpfGroupMcEntryHwIdxAddElement failed. (retVal: %d)\n",
                                   retVal);
                    return retVal;
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

sai_status_t xpSaiGetIpmcEntryAttribute(const sai_ipmc_entry_t *ipmc_entry,
                                        uint32_t attr_count,
                                        sai_attribute_t *attr_list)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;
    uint32_t index;
    xpSaiIpmcAttributesT attributes;

    if (ipmc_entry == NULL)
    {
        XP_SAI_LOG_ERR("Null pointer provided as an argument (ipmc_entry: NULL)\n");
        return SAI_STATUS_INVALID_PARAMETER;
    }

    retVal = xpSaiAttrCheck(attr_count, attr_list, IPMC_ENTRY_VALIDATION_ARRAY_SIZE,
                            ipmc_entry_attribs, SAI_COMMON_API_GET);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Invalid attribute provided (retVal: %d)\n", retVal);
        return retVal;
    }

    retVal = xpSaiIpmcGetRouteEntryIndexAndAttributes(*ipmc_entry, &index,
                                                      &attributes);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to get Route entry data (retVal: %d)\n", retVal);
        return retVal;
    }

    for (uint32_t count = 0; count < attr_count; count++)
    {
        switch (attr_list[count].id)
        {
            case SAI_IPMC_ENTRY_ATTR_PACKET_ACTION:
                {
                    attr_list[count].value.s32 = attributes.pktCmd;
                    break;
                }
            case SAI_IPMC_ENTRY_ATTR_OUTPUT_GROUP_ID:
                {
                    attr_list[count].value.oid = attributes.groupId;
                    break;
                }
            case SAI_IPMC_ENTRY_ATTR_RPF_GROUP_ID:
                {
                    return SAI_STATUS_NOT_SUPPORTED;
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

sai_status_t xpSaiIpmcApiInit(uint64_t flag,
                              const sai_service_method_table_t* adapHostServiceMethodTable)
{
    sai_status_t retVal = SAI_STATUS_SUCCESS;

    _xpSaiIpmcApi = (sai_ipmc_api_t *) xpMalloc(sizeof(sai_ipmc_api_t));

    if (_xpSaiIpmcApi == NULL)
    {
        XP_SAI_LOG_ERR("Failed to allocate _xpSaiIpmcApi\n");
        return SAI_STATUS_NO_MEMORY;
    }

    _xpSaiIpmcApi->create_ipmc_entry = xpSaiCreateIpmcEntry;
    _xpSaiIpmcApi->remove_ipmc_entry = xpSaiRemoveIpmcEntry;
    _xpSaiIpmcApi->set_ipmc_entry_attribute = xpSaiSetIpmcEntryAttribute;
    _xpSaiIpmcApi->get_ipmc_entry_attribute = xpSaiGetIpmcEntryAttribute;

    retVal = xpSaiApiRegister(SAI_API_IPMC, (void*)_xpSaiIpmcApi);
    if (retVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to register IPMC API\n");
        return retVal;
    }

    return retVal;
}

sai_status_t xpSaiIpmcApiDeinit()
{
    xpFree(_xpSaiIpmcApi);
    _xpSaiIpmcApi = NULL;

    return SAI_STATUS_SUCCESS;
}
