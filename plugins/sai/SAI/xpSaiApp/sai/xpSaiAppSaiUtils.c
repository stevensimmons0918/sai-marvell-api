// xpSaiAppSaiUtils.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include <netinet/in.h>
#include "xpSaiAppSaiUtils.h"
#include "xpSaiAppL3.h"
#include "xpSaiRoute.h"

XP_SAI_LOG_REGISTER_API(SAI_API_UNSPECIFIED);

extern uint32_t vrfIdxToVrf[MAX_VRF_IDX];
extern uint32_t vlanToL3Intf[4096];

extern uint64_t vlanToRif[XP_MAX_DOT1Q_VLAN];

uint8_t* xpSaiAppMacReverse(uint8_t* mac)
{
    uint32_t i = 0;
    uint32_t j = 0;
    uint8_t macAddr[XP_MAC_ADDR_LEN];

    memcpy(macAddr, mac, XP_MAC_ADDR_LEN);
    for (i = 0, j = XP_MAC_ADDR_LEN - 1; i < XP_MAC_ADDR_LEN; i++, j--)
    {
        mac[i] = macAddr[j];
    }

    return mac;
}

uint8_t* xpSaiAppIPv6Reverse(sai_ip6_t ipv6Addr)
{
    uint32_t i = 0;
    uint32_t j = 0;
    uint8_t ipv6Addr_buf[16];

    memcpy(ipv6Addr_buf, ipv6Addr, 16);

    for (i = 0, j = 16 - 1; i < 16; i++, j--)
    {
        ipv6Addr[i] = ipv6Addr_buf[j];
    }

    return ipv6Addr;
}

uint64_t xpSaiAppSaiGetVlanRifOid(uint32_t l3IntfId)
{
    uint32_t bd = XPS_INTF_MAP_INTF_TO_BD(l3IntfId);

    if (bd >= XP_MAX_DOT1Q_VLAN)
    {
        return SAI_NULL_OBJECT_ID;
    }

    return vlanToRif[bd];
}

uint64_t xpSaiAppSaiGetiVrfOid(uint32_t l3Intf)
{
    uint32_t    saiRetVal   = SAI_STATUS_SUCCESS;
    uint64_t    oid         = 0;
    xpsDevice_t devId       = xpSaiGetDevId();

    saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_VIRTUAL_ROUTER, devId,
                                 (sai_uint64_t) l3Intf, &oid);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        XP_SAI_LOG_ERR("Failed to create SAI virtual router OID\n");
        return 0x00;
    }

    return oid;
}

//Func: xpsSaiConvertTaggingMode

XP_STATUS xpSaiAppSaiConvertTaggingMode(xpsL2EncapType_e xps_tagging_mode,
                                        sai_vlan_tagging_mode_t* sai_tagging_mode)
{
    switch (xps_tagging_mode)
    {
        case XP_L2_ENCAP_DOT1Q_UNTAGGED:
            {
                *sai_tagging_mode = SAI_VLAN_TAGGING_MODE_UNTAGGED;
                break;
            }
        case XP_L2_ENCAP_DOT1Q_TAGGED:
            {
                *sai_tagging_mode =  SAI_VLAN_TAGGING_MODE_TAGGED;
                break;
            }
        case XP_L2_ENCAP_DOT1Q_PRIOTAGGED:
            {
                *sai_tagging_mode =  SAI_VLAN_TAGGING_MODE_PRIORITY_TAGGED;
                break;
            }
        default:
            {
                printf("Unknown tagging mode %d\n", xps_tagging_mode);
                return XP_ERR_INVALID_ARG;
            }
    }

    return XP_NO_ERR;
}

//Func: xpsSaiConvertStpState
XP_STATUS xpSaiAppSaiConvertStpState(xpVlanStgState_e xpsStpState,
                                     sai_stp_port_state_t* saiStpState)
{
    switch (xpsStpState)
    {
        case SPAN_STATE_BLOCK:
            {
                *saiStpState = SAI_STP_PORT_STATE_BLOCKING;
                break;
            }
        case SPAN_STATE_LEARN:
            {
                *saiStpState =  SAI_STP_PORT_STATE_LEARNING;
                break;
            }
        case SPAN_STATE_DISABLE:
        case SPAN_STATE_FORWARD:
            {
                *saiStpState =  SAI_STP_PORT_STATE_FORWARDING;
                break;
            }
        default:
            {
                printf("Unknown stp state %d\n", xpsStpState);
                return XP_ERR_INVALID_ARG;
            }
    }

    return XP_NO_ERR;
}

XP_STATUS
xpSaiAppSaiWriteIpv4HostEntry(xpDevice_t devId, sai_object_id_t switchOid,
                              Ipv4SaiNeighborEntry* ipv4NeighborData)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    sai_attribute_t saiAttr[SAI_NEIGHBOR_ATTR_COUNT];
    sai_neighbor_entry_t l3HostEntry;
    uint32_t count = 0;
    uint32_t ipV4Addr = 0;

    memset(saiAttr, 0, sizeof(saiAttr));
    memset(&l3HostEntry, 0, sizeof(l3HostEntry));

    if (!ipv4NeighborData)
    {
        printf("Null pointer received while adding Ipv4Host Entry \n");
        return XP_ERR_NULL_POINTER;
    }

    saiAttr[0].id = SAI_NEIGHBOR_ENTRY_ATTR_DST_MAC_ADDRESS;
    saiAttr[1].id = SAI_NEIGHBOR_ENTRY_ATTR_PACKET_ACTION;

    l3HostEntry.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV4;

    memcpy(&ipV4Addr, ipv4NeighborData->ipv4Addr, sizeof(ipV4Addr));
    l3HostEntry.ip_address.addr.ip4 = htonl(ipV4Addr);

    l3HostEntry.rif_id = xpSaiAppSaiGetVlanRifOid(ipv4NeighborData->l3IntfId);
    l3HostEntry.switch_id = switchOid;

    for (count = 0; count < SAI_NEIGHBOR_ATTR_COUNT; count++)
    {
        switch (saiAttr[count].id)
        {
            case SAI_NEIGHBOR_ENTRY_ATTR_DST_MAC_ADDRESS:
                {
                    memcpy(saiAttr[count].value.mac, ipv4NeighborData->macSA, sizeof(macAddr_t));
                    xpSaiAppMacReverse(saiAttr[count].value.mac);
                    break;
                }
            case SAI_NEIGHBOR_ENTRY_ATTR_PACKET_ACTION:
                {
                    saiAttr[count].value.s32 = ipv4NeighborData->pktCmd;
                    break;
                }
            default:
                {
                    printf("Failed to set %d\n", saiAttr[count].id);
                }
        }
    }

    //SAI neighbor call to program the neighbor info in H/W
    saiRetVal = xpSaiNeighborApi->create_neighbor_entry(&l3HostEntry,
                                                        SAI_NEIGHBOR_ATTR_COUNT, saiAttr);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        printf("Error : SAI create_neighbor() failed : retVal %d | devId %d \n",
               saiRetVal, devId);
    }

    printf("xpSaiAppSaiWriteIpv4HostEntry returns:%d \n", saiRetVal);
    return saiStatus2XpsStatus(saiRetVal);
}

XP_STATUS
xpSaiAppSaiWriteIpv6HostEntry(xpDevice_t devId, sai_object_id_t switchOid,
                              Ipv6SaiNeighborEntry* ipv6NeighborData)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    sai_attribute_t saiAttr[SAI_NEIGHBOR_ATTR_COUNT];
    sai_neighbor_entry_t l3HostEntry;
    uint32_t count = 0;

    memset(saiAttr, 0, sizeof(saiAttr));
    memset(&l3HostEntry, 0, sizeof(l3HostEntry));

    if (!ipv6NeighborData)
    {
        printf("Null pointer received while adding Ipv6Host Entry \n");
        return XP_ERR_NULL_POINTER;
    }

    saiAttr[0].id = SAI_NEIGHBOR_ENTRY_ATTR_DST_MAC_ADDRESS;
    saiAttr[1].id = SAI_NEIGHBOR_ENTRY_ATTR_PACKET_ACTION;

    l3HostEntry.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV6;
    memcpy(&(l3HostEntry.ip_address.addr.ip6), ipv6NeighborData->ipv6Addr,
           sizeof(ipv6Addr_t));

    xpSaiAppIPv6Reverse(l3HostEntry.ip_address.addr.ip6);

    l3HostEntry.rif_id = xpSaiAppSaiGetVlanRifOid(ipv6NeighborData->l3IntfId);
    l3HostEntry.switch_id = switchOid;

    for (count = 0; count < SAI_NEIGHBOR_ATTR_COUNT; count++)
    {
        switch (saiAttr[count].id)
        {
            case SAI_NEIGHBOR_ENTRY_ATTR_DST_MAC_ADDRESS:
                {
                    memcpy(saiAttr[count].value.mac, ipv6NeighborData->macSA, sizeof(macAddr_t));
                    xpSaiAppMacReverse(saiAttr[count].value.mac);
                    break;
                }
            case SAI_NEIGHBOR_ENTRY_ATTR_PACKET_ACTION:
                {
                    saiAttr[count].value.s32 = ipv6NeighborData->pktCmd;
                    break;
                }
            default:
                {
                    printf("Failed to set %d\n", saiAttr[count].id);
                }
        }
    }

    //SAI neighbor call to program the neighbor info in H/W
    saiRetVal = xpSaiNeighborApi->create_neighbor_entry(&l3HostEntry,
                                                        SAI_NEIGHBOR_ATTR_COUNT, saiAttr);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        printf("Error : SAI create_neighbor() failed : retVal %d | devId %d \n",
               saiRetVal, devId);
    }

    printf("xpSaiAppSaiWriteIpv6HostEntry returns:%d \n", saiRetVal);
    return saiStatus2XpsStatus(saiRetVal);
}


XP_STATUS
xpSaiAppSaiWriteIpv4NextHopEntry(xpDevice_t devId, sai_object_id_t switchOid,
                                 Ipv4SaiNhEntry* ipv4SaiNhData, uint64_t* nhId)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    uint32_t  count     = 0;
    uint32_t  ipV4Addr  = 0;
    sai_attribute_t saiAttr[SAI_NEXT_HOP_ATTR_COUNT];
    sai_neighbor_entry_t l3NextHopEntry;

    memset(saiAttr, 0, sizeof(saiAttr));
    memset(&l3NextHopEntry, 0, sizeof(l3NextHopEntry));

    saiAttr[0].id = SAI_NEXT_HOP_ATTR_TYPE;
    saiAttr[1].id = SAI_NEXT_HOP_ATTR_IP;
    saiAttr[2].id = SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID;

    if (!ipv4SaiNhData)
    {
        printf("Null pointer received while adding Ipv4Host Entry \n");
        return XP_ERR_NULL_POINTER;
    }

    l3NextHopEntry.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    memcpy(&(l3NextHopEntry.ip_address.addr.ip4), ipv4SaiNhData->ipv4Addr,
           sizeof(ipv4Addr_t));

    l3NextHopEntry.rif_id = xpSaiAppSaiGetVlanRifOid(
                                vlanToL3Intf[ipv4SaiNhData->l3IntfId]);

    for (count = 0; count < SAI_NEXT_HOP_ATTR_COUNT; count++)
    {
        switch (saiAttr[count].id)
        {
            case SAI_NEXT_HOP_ATTR_IP:
                {
                    saiAttr[count].value.ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV4;

                    memcpy(&ipV4Addr, ipv4SaiNhData->ipv4Addr, sizeof(ipV4Addr));
                    saiAttr[count].value.ipaddr.addr.ip4 = htonl(ipV4Addr);

                    break;
                }
            case SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID:
                {
                    saiAttr[count].value.oid = l3NextHopEntry.rif_id;
                    break;
                }
            case SAI_NEXT_HOP_ATTR_TYPE:
                {
                    saiAttr[count].value.s32 = SAI_NEXT_HOP_TYPE_IP;
                    break;
                }
            default:
                {
                    printf("Failed to set %d\n", saiAttr[count].id);
                }
        }
    }

    saiRetVal = xpSaiNexthopApi->create_next_hop(nhId, switchOid,
                                                 SAI_NEXT_HOP_ATTR_COUNT, saiAttr);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        printf("Error : SAI create_nexthop() failed : retVal %d | devId %d \n",
               saiRetVal, devId);
    }

    printf("xpSaiAppSaiWriteIpv4NextHopEntry returns: %d -----> nhid = %lu Pass\n",
           saiRetVal, (long unsigned int) *nhId);

    return saiStatus2XpsStatus(saiRetVal);
}

XP_STATUS
xpSaiAppSaiWriteIpv6NextHopEntry(xpDevice_t devId, sai_object_id_t switchOid,
                                 Ipv6SaiNhEntry* ipv6SaiNhData, uint64_t* nhId)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    sai_attribute_t saiAttr[SAI_NEXT_HOP_ATTR_COUNT];
    sai_neighbor_entry_t l3NextHopEntry;
    uint32_t count = 0;

    memset(saiAttr, 0, sizeof(saiAttr));
    memset(&l3NextHopEntry, 0, sizeof(l3NextHopEntry));

    saiAttr[0].id = SAI_NEXT_HOP_ATTR_TYPE;
    saiAttr[1].id = SAI_NEXT_HOP_ATTR_IP;
    saiAttr[2].id = SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID;

    if (!ipv6SaiNhData)
    {
        printf("Null pointer received while adding Ipv6Host Entry \n");
        return XP_ERR_NULL_POINTER;
    }

    l3NextHopEntry.ip_address.addr_family = SAI_IP_ADDR_FAMILY_IPV6;
    memcpy(&(l3NextHopEntry.ip_address.addr.ip6), ipv6SaiNhData->ipv6Addr,
           sizeof(ipv6Addr_t));
    xpSaiAppIPv6Reverse(l3NextHopEntry.ip_address.addr.ip6);

    l3NextHopEntry.rif_id = xpSaiAppSaiGetVlanRifOid(
                                vlanToL3Intf[ipv6SaiNhData->l3IntfId]);

    for (count = 0; count < SAI_NEXT_HOP_ATTR_COUNT; count++)
    {
        switch (saiAttr[count].id)
        {
            case SAI_NEXT_HOP_ATTR_IP:
                {
                    saiAttr[count].value.ipaddr.addr_family = SAI_IP_ADDR_FAMILY_IPV6;
                    memcpy(&saiAttr[count].value.ipaddr.addr.ip6, ipv6SaiNhData->ipv6Addr,
                           sizeof(ipv6Addr_t));
                    xpSaiAppIPv6Reverse(saiAttr[count].value.ipaddr.addr.ip6);
                    break;
                }
            case SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID:
                {
                    saiAttr[count].value.oid = l3NextHopEntry.rif_id;
                    break;
                }
            case SAI_NEXT_HOP_ATTR_TYPE:
                {
                    saiAttr[count].value.s32 = SAI_NEXT_HOP_TYPE_IP;
                    break;
                }
            default:
                {
                    printf("Failed to set %d\n", saiAttr[count].id);
                }
        }
    }

    saiRetVal = xpSaiNexthopApi->create_next_hop(nhId, switchOid,
                                                 SAI_NEXT_HOP_ATTR_COUNT, saiAttr);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        printf("Error : SAI create_nexthop() failed : retVal %d | devId %d \n",
               saiRetVal, devId);
    }
    printf("xpSaiAppSaiWriteIpv6NextHopEntry returns: %d -----> nhid = %lu Pass\n",
           saiRetVal, (long unsigned int) *nhId);
    return saiStatus2XpsStatus(saiRetVal);
}

XP_STATUS xpSaiAppSaiWriteIpv4RouteEntry(xpDevice_t devId,
                                         sai_object_id_t switchOid, Ipv4SaiRouteEntry* ipv4SaiRouteData, uint64_t nhId)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    uint32_t  ipV4Addr  = 0;
    uint32_t  ipV4Mask  = 0;
    sai_attribute_t saiAttr[SAI_ROUTE_ATTR_COUNT];
    sai_route_entry_t routeEntry;

    memset(saiAttr, 0, sizeof(saiAttr));
    memset(&routeEntry, 0, sizeof(routeEntry));

    if (!ipv4SaiRouteData)
    {
        printf("Null pointer received while adding xpSaiAppSaiWriteIpv4RouteEntry Entry \n");
        return XP_ERR_NULL_POINTER;
    }

    routeEntry.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV4;
    memcpy(&ipV4Addr, ipv4SaiRouteData->ipv4Addr, sizeof(ipV4Addr));
    routeEntry.destination.addr.ip4 = htonl(ipV4Addr);
    routeEntry.vr_id = xpSaiAppSaiGetiVrfOid(vrfIdxToVrf[ipv4SaiRouteData->vrfId]);
    routeEntry.switch_id = switchOid;

    saiRetVal = xpSaiConvertPrefix2IPv4Mask(ipv4SaiRouteData->netMask, &ipV4Mask);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        printf("Error : SAI xpSaiConvertPrefix2IPv4Mask() failed : retVal %d | devId %d \n",
               saiRetVal, devId);
        return saiStatus2XpsStatus(saiRetVal);
    }

    routeEntry.destination.mask.ip4 = htonl(ipV4Mask);

    saiAttr[0].id = SAI_ROUTE_ENTRY_ATTR_PACKET_ACTION;
    saiAttr[0].value.s32 = SAI_PACKET_ACTION_FORWARD;

    saiAttr[1].id = SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID;
    saiAttr[1].value.oid = nhId;

    saiRetVal = xpSaiRouteApi->create_route_entry(&routeEntry, SAI_ROUTE_ATTR_COUNT,
                                                  saiAttr);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        printf("Error : SAI create_route_entry() failed : retVal %d | devId %d \n",
               saiRetVal, devId);
    }

    return saiStatus2XpsStatus(saiRetVal);
}

XP_STATUS xpSaiAppSaiWriteIpv6RouteEntry(xpDevice_t devId,
                                         sai_object_id_t switchOid, Ipv6SaiRouteEntry* ipv6SaiRouteData, uint64_t nhId)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    sai_attribute_t saiAttr[SAI_ROUTE_ATTR_COUNT];
    sai_route_entry_t routeEntry;

    memset(&routeEntry, 0, sizeof(routeEntry));
    memset(saiAttr, 0, sizeof(saiAttr));

    if (!ipv6SaiRouteData)
    {
        printf("Null pointer received while adding xpSaiAppSaiWriteIpv6RouteEntry Entry \n");
        return XP_ERR_NULL_POINTER;
    }

    routeEntry.destination.addr_family = SAI_IP_ADDR_FAMILY_IPV6;
    memcpy(&routeEntry.destination.addr.ip6, ipv6SaiRouteData->ipv6Addr,
           sizeof(routeEntry.destination.addr.ip6));
    xpSaiAppIPv6Reverse(routeEntry.destination.addr.ip6);
    routeEntry.vr_id = xpSaiAppSaiGetiVrfOid(vrfIdxToVrf[ipv6SaiRouteData->vrfId]);
    routeEntry.switch_id = switchOid;

    saiRetVal = xpSaiConvertPrefix2IPv6Mask(ipv6SaiRouteData->netMask,
                                            routeEntry.destination.mask.ip6);

    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        printf("Error : SAI xpSaiConvertPrefix2IPv6Mask() failed : retVal %d | devId %d \n",
               saiRetVal, devId);
        return saiStatus2XpsStatus(saiRetVal);
    }

    saiAttr[0].id = SAI_ROUTE_ENTRY_ATTR_PACKET_ACTION;
    saiAttr[0].value.s32 = SAI_PACKET_ACTION_FORWARD;

    saiAttr[1].id = SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID;
    saiAttr[1].value.oid = nhId;

    saiRetVal = xpSaiRouteApi->create_route_entry(&routeEntry, SAI_ROUTE_ATTR_COUNT,
                                                  saiAttr);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        printf("Error : SAI create_route_entry() failed : retVal %d | devId %d \n",
               saiRetVal, devId);
    }

    return saiStatus2XpsStatus(saiRetVal);
}

XP_STATUS xpSaiAppConfigureIpv4NextHopGroupEntry(xpDevice_t devId,
                                                 sai_object_id_t switchOid, IpvxSaiNhGroupEntry *ipvxSaiNhGroupData,
                                                 uint64_t *nhIdIp4, uint64_t *nhGrpIdIp4)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    uint8_t idx = 0;
    uint32_t nhIdx = 0;
    sai_attribute_t saiAttr[2] = {};
    sai_object_id_t next_hop_group_id, next_hop_group_member_id;

    saiAttr[0].id = SAI_NEXT_HOP_GROUP_ATTR_TYPE;
    saiAttr[0].value.u8 = SAI_NEXT_HOP_GROUP_TYPE_ECMP;

    saiRetVal = xpSaiNextHopGroupApi->create_next_hop_group(&next_hop_group_id,
                                                            switchOid, 1, saiAttr);
    if (saiRetVal != SAI_STATUS_SUCCESS)
    {
        printf("Error : SAI create_next_hop_group() failed : retVal %d | devId %d \n",
               saiRetVal, devId);
    }

    saiAttr[0].id = SAI_NEXT_HOP_GROUP_MEMBER_ATTR_NEXT_HOP_GROUP_ID;
    saiAttr[0].value.oid = next_hop_group_id;
    *nhGrpIdIp4 = next_hop_group_id;

    for (idx = 0; idx < ipvxSaiNhGroupData->ecmpSize; idx++)
    {
        nhIdx=*((uint16_t *)getEntry(&(ipvxSaiNhGroupData->nhIdList), idx));

        saiAttr[1].id = SAI_NEXT_HOP_GROUP_MEMBER_ATTR_NEXT_HOP_ID;
        saiAttr[1].value.oid = nhIdIp4[nhIdx];

        saiRetVal = xpSaiNextHopGroupApi->create_next_hop_group_member(
                        &next_hop_group_member_id, switchOid, 2, saiAttr);
        if (saiRetVal != SAI_STATUS_SUCCESS)
        {
            printf("Error : SAI create_next_hop_group_member() failed : retVal %d | devId %d \n",
                   saiRetVal, devId);
        }
    }

    return saiStatus2XpsStatus(saiRetVal);
}

sai_status_t xpSaiPrintObjectAttributes(sai_object_key_t *object_keys,
                                        sai_object_id_t switch_id, sai_object_type_t objectType,
                                        sai_uint32_t objectCount)
{
    sai_status_t sai_status = SAI_STATUS_SUCCESS;
    sai_uint32_t i = 0, j = 0;
    sai_uint32_t *attrList;
    sai_status_t *objectStatuses;
    sai_attribute_t **attrs;
    sai_uint32_t maxAttrCount = 0;

    sai_status = sai_get_maximum_attribute_count(switch_id, objectType,
                                                 &maxAttrCount);
    if (sai_status != SAI_STATUS_SUCCESS)
    {
        printf("Error: sai_get_maximum_attribute_count failed with a status: %d\n",
               sai_status);
        return sai_status;
    }

    attrList = (sai_uint32_t *) malloc(objectCount * sizeof(sai_uint32_t));
    if (!attrList)
    {
        printf("Error: attrCount, Out of memory!\n");
        return SAI_STATUS_NO_MEMORY;
    }

    for (i = 0; i < objectCount; i++)
    {
        attrList[i] = maxAttrCount;
    }

    objectStatuses = (sai_status_t *) malloc(objectCount * sizeof(sai_status_t));
    if (!objectStatuses)
    {
        printf("Error: object_statuses, Out of memory!\n");
        free(attrList);
        return SAI_STATUS_NO_MEMORY;
    }
    memset(objectStatuses, SAI_STATUS_SUCCESS, objectCount * sizeof(sai_status_t));

    attrs = (sai_attribute_t **) malloc(objectCount * sizeof(sai_attribute_t *));
    if (!attrs)
    {
        printf("Error: attrs, Out of memory!\n");
        free(objectStatuses);
        free(attrList);
        return SAI_STATUS_NO_MEMORY;
    }

    for (i = 0; i < objectCount; i++)
    {
        attrs[i] = (sai_attribute_t *) malloc(maxAttrCount * sizeof(sai_attribute_t));
        if (!attrs[i])
        {
            printf("Error: attrs[%d], Out of memory!\n", i);
            if (i > 0)
            {
                for (j = 0; j < i - 1; j++)
                {
                    free(attrs[j]);
                }
            }
            free(attrs);
            free(objectStatuses);
            free(attrList);
            return SAI_STATUS_NO_MEMORY;
        }
        memset(attrs[i], 0x0, maxAttrCount * sizeof(sai_attribute_t));
    }

    sai_status = sai_bulk_get_attribute(switch_id, objectType, objectCount,
                                        object_keys, attrList, attrs, objectStatuses);
    if (sai_status != SAI_STATUS_SUCCESS)
    {
        printf("Error: sai_bulk_get_attribute failed with a status: %d\n", sai_status);
        free(objectStatuses);
        free(attrList);
        for (i = 0; i < objectCount; i++)
        {
            free(attrs[i]);
        }
        free(attrs);
        return sai_status;
    }

    for (i = 0; i < objectCount; i++)
    {
        printf("OID - %" PRIu64 "\n", object_keys[i].key.object_id);
        for (j = 0; j < attrList[i]; j++)
        {
            printf("attrId - %d,vlan_count -%d\n", attrs[i][j].id,
                   attrs[i][j].value.vlanlist.count);
        }
    }

    for (i = 0; i < objectCount; i++)
    {
        free(attrs[i]);
    }
    free(attrs);
    free(objectStatuses);
    free(attrList);

    return sai_status;
}

XP_STATUS xpSaiAppPrintAllObjectAttributes(sai_object_id_t switch_id)
{
    sai_status_t sai_status = SAI_STATUS_SUCCESS;
    sai_uint32_t objcount = 0;
    sai_uint32_t *object_count = &objcount ;
    sai_object_key_t *object_keys;
    sai_uint32_t objectType;

    for (objectType = SAI_OBJECT_TYPE_PORT; objectType < SAI_OBJECT_TYPE_MAX;
         objectType++)
    {
        //1.Get the count of objects for the object type
        sai_status = sai_get_object_count(switch_id, (sai_object_type_t)objectType,
                                          object_count);
        if (sai_status != SAI_STATUS_SUCCESS)
        {
            printf("Warning: sai_get_object_count failed with a code: %d, objType: %d\n",
                   sai_status, objectType);
            continue;
        }

        object_keys = (sai_object_key_t *) malloc((*object_count)*sizeof(
                                                      sai_object_key_t));
        if (!object_keys)
        {
            printf("Error: Out of memory!\n");
            return XP_ERR_OUT_OF_MEM;
        }

        sai_status = sai_get_object_key(switch_id, (sai_object_type_t)objectType,
                                        object_count, object_keys);
        if (sai_status != SAI_STATUS_SUCCESS)
        {
            printf("Warning : sai_get_object_key failed, status: %d\n", sai_status);
            free(object_keys);
            continue;
        }

        sai_status = xpSaiPrintObjectAttributes(object_keys, switch_id,
                                                (sai_object_type_t)objectType, *object_count);
        if (sai_status != SAI_STATUS_SUCCESS)
        {
            printf("Warning : xpSaiPrintAttributes failed, status: %d", sai_status);
            free(object_keys);
            continue;
        }

        free(object_keys);
    }
    return XP_NO_ERR;
}

XP_STATUS xpSaiAppStpSetState(sai_object_id_t *stpPortOid,
                              sai_object_id_t stpOid, sai_object_id_t saiPortOid,
                              sai_stp_port_state_t saiStpState)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    sai_attribute_t attr_list[3];

    attr_list[0].id = SAI_STP_PORT_ATTR_STP;
    attr_list[0].value.oid = stpOid;

    attr_list[1].id = SAI_STP_PORT_ATTR_BRIDGE_PORT;
    attr_list[1].value.oid = saiPortOid;

    attr_list[2].id = SAI_STP_PORT_ATTR_STATE;
    attr_list[2].value.s32 = saiStpState;

    if ((saiRetVal = xpSaiStpApi->create_stp_port(stpPortOid, saiPortOid, 3,
                                                  attr_list)) != SAI_STATUS_SUCCESS)
    {
        printf("%s:Error:Failed to set stp port state %d for stpId %" PRId64 ", port %"
               PRId64 " | retVal : %d \n", __FUNCTION__, saiStpState,
               xpSaiObjIdValueGet(stpOid), xpSaiObjIdValueGet(saiPortOid), saiRetVal);
        return saiStatus2XpsStatus(saiRetVal);
    }
    printf("%s:Info: Sai set stp port state %d for stpId %" PRId64 ", port %" PRId64
           " successfully \n", __FUNCTION__, saiStpState, xpSaiObjIdValueGet(stpOid),
           xpSaiObjIdValueGet(saiPortOid));

    return saiStatus2XpsStatus(saiRetVal);
}
#if 0
TODO:
Move this to the bridge port usage when bridge port learning mode is supported
in SAI
XP_STATUS xpSaiAppPortSetAttr(uint16_t portNum, xpDevice_t devId)
{
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
    sai_object_id_t saiPortOid;
    sai_attribute_t port_attr;
    port_attr.id = SAI_PORT_ATTR_FDB_LEARNING_MODE;
    port_attr.value.s32 = SAI_PORT_FDB_LEARNING_MODE_DISABLE;

    if ((saiRetVal = xpSaiObjIdCreate(SAI_OBJECT_TYPE_PORT, devId, portNum,
                                      &saiPortOid)) != SAI_STATUS_SUCCESS)
    {
        printf("Error : SAI object can not be created.\n");
        return saiStatus2XpsStatus(saiRetVal);
    }

    if ((saiRetVal = xpSaiPortApi->set_port_attribute(saiPortOid,
                                                      &port_attr)) != SAI_STATUS_SUCCESS)
    {
        printf("Error : set_port_attribute failed for port : %d\n",
               (uint32_t)xpSaiObjIdValueGet(saiPortOid));
        return saiStatus2XpsStatus(saiRetVal);
    }
    return saiStatus2XpsStatus(saiRetVal);
}
#endif
