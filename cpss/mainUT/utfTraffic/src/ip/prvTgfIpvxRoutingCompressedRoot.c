/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file prvTgfIpvxRoutingCompressedRoot.c
*
* @brief IPv6 LPM capacity test for merge banks
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfIpGen.h>
#include <common/tgfBridgeGen.h>
#include <ip/prvTgfIpvxRoutingCompressedRoot.h>


#define BREAK_LOOP_IF_ERROR_OCCURS  \
            if (rc != GT_OK)        \
            {                       \
                breakNow = GT_TRUE; \
                break;              \
            }

#define RETURN_IF_LOOP_BREAK_OCCURS  \
            if (breakNow == GT_TRUE) \
            {                        \
                return;              \
            }

/* next hop vlan id */
#define PRV_TGF_NEXTHOP_VLANID1_CNS       6

/* the counter set for a route entry is linked to */
#define PRV_TGF_COUNT_SET_CNS     CPSS_IP_CNT_SET0_E

#define PRV_TGF_NEXTHOP_PORT_IDX_CNS 3

/* lpm db ID */
static GT_U32        prvTgfLpmDBId = 0;

/* route entry base index */
static GT_U32        prvTgfLpmRouteEntryBaseIndex = 4;

/* mll pointer index */
static GT_U32        prvTgfMllPointerIndex     = 5;

/* arp index */
static GT_U32        prvTgfLpmRoutingArpIndex     = 1;

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfLpmArpMac      = {0x00, 0x00, 0x00, 0x00, 0x00, 0x33};

static GT_U32       prvTgfNumVrfs = 4096;

/* parameters that is needed to be restored */
static struct
{
    GT_ETHERADDR                                lpmArpMacAddrOrig;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC               routeEntriesArray[1];
} prvTgfRestoreCfg;

static GT_VOID prvTgfIpvxNextHopRouteConfigSet
(
    TGF_MAC_ADDR *trgArpMacAddr,
    GT_ETHERADDR *restoreArpMacAddr,
    GT_U32       arpIndex,
    GT_U32       nhVlanId,
    GT_U32       nhRouteIndex
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *regularEntryPtr;
    GT_ETHERADDR                            arpMacAddr;
    static GT_U32                           firstEntry = 1;


    /* AUTODOC: write a ARP MAC 00:00:00:00:00:33 to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, trgArpMacAddr, sizeof(TGF_MAC_ADDR));

    /* Create the Route entry (Next hop) in FDB table and Router ARP Table */
    rc = prvTgfIpRouterArpAddrRead(prvTgfDevNum, arpIndex, restoreArpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrRead: %d", prvTgfDevNum);

    rc = prvTgfIpRouterArpAddrWrite(arpIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* write a UC Route entry to the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    regularEntryPtr = &routeEntriesArray[0];
    regularEntryPtr->cmd                        = CPSS_PACKET_CMD_ROUTE_E;
    regularEntryPtr->countSet                   = PRV_TGF_COUNT_SET_CNS;
    regularEntryPtr->nextHopVlanId              = nhVlanId;
    regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
    regularEntryPtr->nextHopInterface.devPort.hwDevNum = prvTgfDevNum;
    regularEntryPtr->nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOP_PORT_IDX_CNS];
    regularEntryPtr->nextHopARPPointer          = arpIndex;

    if (firstEntry == 1) {
        firstEntry = 0;
        /* AUTODOC: store UC route entry with nexthop VLAN 6 and nexthop port 3 */
        cpssOsMemSet(prvTgfRestoreCfg.routeEntriesArray, 0, sizeof(prvTgfRestoreCfg.routeEntriesArray));
        rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum, nhRouteIndex, prvTgfRestoreCfg.routeEntriesArray, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);
    }

    /* AUTODOC: add UC route entry with nexthop VLAN 6 and nexthop port 3 */
    rc = prvTgfIpUcRouteEntriesWrite(nhRouteIndex, routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);
}

/**
 * @internal prvTgfIpv4UcRoutingCompressedRoot function
 * @endinternal
 *
 * @brief Ipv4 Unicast Prefixes add/delete with compressed root node.
 *        1. Add basic configurations.
 *        2. Check root node converstions, embedded, compressed, regular.
 *        3. Create multiple VRFs and prefixes in each vrf
 *        4. Delete prefixes in each vrf and delete VRFs
*/
GT_VOID prvTgfIpv4UcRoutingCompressedRoot
(
    GT_VOID
)
{
    GT_STATUS                              rc = GT_OK;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC         vr_config;
    GT_U32                                 i = 0, j = 0;
    GT_IPADDR                              ipaddr = {0};
    GT_U32                                 numPrefixes = 6;
    GT_U32                                 numVrfs     = 100;
    GT_U32                                 step = 2;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT   nextHopInfo;
    GT_BOOL                                breakNow = GT_FALSE;

    /* -------------------------------------------------------------------
     * 1. Add Basic configuraion.
     * ------------------------------------------------------------------- */

    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfLpmRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType           = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    cpssOsMemSet(&vr_config, 0, sizeof(vr_config));
    vr_config.supportIpv4Uc = GT_TRUE;
    vr_config.defIpv4UcNextHopInfo.ipLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    vr_config.supportIpv6Uc = GT_TRUE;
    vr_config.defIpv6UcNextHopInfo.ipLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    vr_config.supportIpv4Mc = GT_TRUE;
    vr_config.defIpv4McRouteLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    vr_config.supportIpv6Mc = GT_TRUE;
    vr_config.defIpv6McRouteLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;

    /* Set next hop for LPM routing */
    prvTgfIpvxNextHopRouteConfigSet(&prvTgfLpmArpMac, &prvTgfRestoreCfg.lpmArpMacAddrOrig, prvTgfLpmRoutingArpIndex,
                                      PRV_TGF_NEXTHOP_VLANID1_CNS, prvTgfLpmRouteEntryBaseIndex);

    /* -------------------------------------------------------------------
     * 2. Check root node converstions, embedded, compressed, regular.
     * ------------------------------------------------------------------- */

    ipaddr.arIP[0] = 0;
    ipaddr.arIP[1] = 0;
    ipaddr.arIP[2] = 1;
    /* Add and Delete Ipv4 UC prefix */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, i, ipaddr, 24, &nextHopInfo, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd failed: vrf=%d", i);
    rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, i, ipaddr, 24);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel failed: vrf=%d", i);


    /* -------------------------------------------------------------------
     * 3. Create multiple VRFs and add prefixes in each vrf
     * ------------------------------------------------------------------- */

    /* 1. Add prefixes with incrementing octet 2, vrf 0*/
    for (j = 0; j < step*numPrefixes; j += step)
    {
        ipaddr.arIP[0] = 0;
        ipaddr.arIP[2] = j;
        /* try to add Ipv4 UC prefix */
        rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, i, ipaddr, 24, &nextHopInfo, GT_TRUE, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd failed: vrf=%d", i);
        BREAK_LOOP_IF_ERROR_OCCURS;
    }
    RETURN_IF_LOOP_BREAK_OCCURS;
    /* 1. Add prefixes with incrementing octet 2, vrf 0*/
    for (j = 0; j < step*numPrefixes; j += step)
    {
        ipaddr.arIP[0] = 100;
        ipaddr.arIP[2] = j;
        /* try to add Ipv4 UC prefix */
        rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, i, ipaddr, 24, &nextHopInfo, GT_TRUE, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd failed: vrf=%d", i);
        BREAK_LOOP_IF_ERROR_OCCURS;
    }
    RETURN_IF_LOOP_BREAK_OCCURS;
    /* Add prefixes with incrementing octet 0, vrf 0 */
    for (j = 0; j < step*numPrefixes; j += step)
    {
        ipaddr.arIP[0] = j;
        ipaddr.arIP[2] = 1;
        /* try to add Ipv4 UC prefix */
        rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, i, ipaddr, 24, &nextHopInfo, GT_TRUE, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd failed: vrf=%d", i);
        BREAK_LOOP_IF_ERROR_OCCURS;
    }
    RETURN_IF_LOOP_BREAK_OCCURS;

    /* Delete prefixes with incrementing octet 2, vrf 0*/
    for (j = 0; j < step*numPrefixes; j += step)
    {
        ipaddr.arIP[0] = 0;
        ipaddr.arIP[2] = j;
        /* try to add Ipv4 UC prefix */
        rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, i, ipaddr, 24);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel failed: vrf=%d", i);
        BREAK_LOOP_IF_ERROR_OCCURS;
    }
    RETURN_IF_LOOP_BREAK_OCCURS;
    /* 1. Delete prefixes with incrementing octet 2, vrf 0*/
    for (j = 0; j < step*numPrefixes; j += step)
    {
        ipaddr.arIP[0] = 100;
        ipaddr.arIP[2] = j;
        /* try to add Ipv4 UC prefix */
        rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, i, ipaddr, 24);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel failed: vrf=%d", i);
        BREAK_LOOP_IF_ERROR_OCCURS;
    }
    RETURN_IF_LOOP_BREAK_OCCURS;
    /* Delete prefixes with incrementing octet 0, vrf 0 */
    for (j = 0; j < step*numPrefixes; j += step)
    {
        ipaddr.arIP[0] = j;
        ipaddr.arIP[2] = 1;
        /* try to add Ipv4 UC prefix */
        rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, i, ipaddr, 24);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel failed: vrf=%d", i);
        BREAK_LOOP_IF_ERROR_OCCURS;
    }
    RETURN_IF_LOOP_BREAK_OCCURS;

    /* Add vrfs and few prefixes in each vrf */
    for (i = 1; i < numVrfs; i++)
    {
        /* Add virtual router */
        rc = cpssDxChIpLpmVirtualRouterAdd(prvTgfLpmDBId, i, &vr_config);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpLpmVirtualRouterAdd failed: vrf=%d", i);

        /* Add prefixes with incrementing octet 1*/
        for (j = 0; j < step*numPrefixes; j += step)
        {
            ipaddr.arIP[0] = 1;
            ipaddr.arIP[1] = j;
            /* try to add Ipv4 UC prefix */
            rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, i, ipaddr, 16, &nextHopInfo, GT_TRUE, GT_FALSE);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd failed: vrf=%d", i);
            BREAK_LOOP_IF_ERROR_OCCURS;
        }
        BREAK_LOOP_IF_ERROR_OCCURS;

        /* Add prefixes with incrementing octet 0*/
        for (j = 0; j<step*numPrefixes; j += step)
        {
            ipaddr.arIP[0] = j;
            /* try to add Ipv4 UC prefix */
            rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, i, ipaddr, 8, &nextHopInfo, GT_TRUE, GT_FALSE);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd failed: vrf=%d", i);
            BREAK_LOOP_IF_ERROR_OCCURS;
        }
        BREAK_LOOP_IF_ERROR_OCCURS;
    }

    /* print the block mapping state */
    rc = prvTgfIpLpmDbgHwBlockInfoPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwBlockInfoPrint failed");
    RETURN_IF_LOOP_BREAK_OCCURS;


    /* -------------------------------------------------------------------
     * 4. Delete prefixes in each vrf and delete VRFs
     * ------------------------------------------------------------------- */

    /* Delete Vrfs */
    for (j = i-1; j > 0; j--)
    {
        /* Flush Ipv4 prefixes */
        rc = cpssDxChIpLpmIpv4UcPrefixesFlush(prvTgfLpmDBId, j);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpLpmIpv4UcPrefixesFlush failed: vrf=%d", j);
        BREAK_LOOP_IF_ERROR_OCCURS;

        /* Delete virtual router */
        rc = cpssDxChIpLpmVirtualRouterDel(prvTgfLpmDBId, j);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpLpmVirtualRouterDel failed: vrf=%d", j);
        BREAK_LOOP_IF_ERROR_OCCURS;
    }

    /* print the block mapping state */
    rc = prvTgfIpLpmDbgHwBlockInfoPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwBlockInfoPrint failed");
}

/**
 * @internal prvTgfIpv6UcRoutingCompressedRoot function
 * @endinternal
 *
 * @brief Ipv6 Unicast Prefixes add/delete with compressed root node.
 *        1. Add basic configurations.
 *        2. Check root node converstions, embedded, compressed, regular.
 *        3. Create multiple VRFs and prefixes in each vrf
 *        4. Delete prefixes in each vrf and delete VRFs
*/
GT_VOID prvTgfIpv6UcRoutingCompressedRoot
(
    GT_VOID
)
{
    GT_STATUS                              rc = GT_OK;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC         vr_config;
    GT_U32                                 i = 0, j = 0;
    GT_IPV6ADDR                            ipv6Addr = {{0}};
    GT_U32                                 numPrefixes = 6;
    GT_U32                                 numVrfs     = 100;
    GT_U32                                 step = 2;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT   nextHopInfo;
    GT_BOOL                                breakNow = GT_FALSE;

    /* -------------------------------------------------------------------
     * 1. Add Basic configuraion.
     * ------------------------------------------------------------------- */

    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfLpmRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType           = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    cpssOsMemSet(&vr_config, 0, sizeof(vr_config));
    vr_config.supportIpv4Uc = GT_TRUE;
    vr_config.defIpv4UcNextHopInfo.ipLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    vr_config.supportIpv6Uc = GT_TRUE;
    vr_config.defIpv6UcNextHopInfo.ipLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    vr_config.supportIpv4Mc = GT_TRUE;
    vr_config.defIpv4McRouteLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    vr_config.supportIpv6Mc = GT_TRUE;
    vr_config.defIpv6McRouteLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;


    /* Set next hop for LPM routing */
    prvTgfIpvxNextHopRouteConfigSet(&prvTgfLpmArpMac, &prvTgfRestoreCfg.lpmArpMacAddrOrig, prvTgfLpmRoutingArpIndex,
                                      PRV_TGF_NEXTHOP_VLANID1_CNS, prvTgfLpmRouteEntryBaseIndex);

    /* -------------------------------------------------------------------
     * 2. Check root node converstions, embedded, compressed, regular.
     * ------------------------------------------------------------------- */

    ipv6Addr.arIP[0] = 1;

    /* Add and delete 1::0/8. Convert root embed_2_2 to compress and vice versa*/
    rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, i, ipv6Addr, 8, &nextHopInfo, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixAdd failed: vrf=%d", i);
    rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, i, ipv6Addr, 8);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixDel failed: vrf=%d", i);


    /* Add and delete 1::0/1. Convert root embed_2_2 to embed 3_3 and vice versa*/
    rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, i, ipv6Addr, 1, &nextHopInfo, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixAdd failed: vrf=%d", i);
    rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, i, ipv6Addr, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixDel failed: vrf=%d", i);


    /* Convert root from compress to regular and vice versa. prefix length = 8 */
    for (j = 0; j<step*numPrefixes; j += step)
    {
        ipv6Addr.arIP[0] = j;
        /* try to add IPv6 UC prefix */
        rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, i, ipv6Addr, 8, &nextHopInfo, GT_TRUE, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixAdd failed: vrf=%d", i);
        BREAK_LOOP_IF_ERROR_OCCURS;
    }
    RETURN_IF_LOOP_BREAK_OCCURS;
    for (j = 0; j<step*numPrefixes; j += step)
    {
        ipv6Addr.arIP[0] = j;
        /* try to add IPv6 UC prefix */
        rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, i, ipv6Addr, 8);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixDel failed: vrf=%d", i);
        BREAK_LOOP_IF_ERROR_OCCURS;
    }
    RETURN_IF_LOOP_BREAK_OCCURS;


    /* convert root from compress to regular and vice versa. prefix length = 24*/
    for (j = 0; j < step*numPrefixes; j += step)
    {
        ipv6Addr.arIP[0] = j;
        ipv6Addr.arIP[2] = 1;
        /* try to add IPv6 UC prefix */
        rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, i, ipv6Addr, 24, &nextHopInfo, GT_TRUE, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixAdd failed: vrf=%d", i);
        BREAK_LOOP_IF_ERROR_OCCURS;
    }
    RETURN_IF_LOOP_BREAK_OCCURS;
    for (j = 0; j < step*numPrefixes; j += step)
    {
        ipv6Addr.arIP[0] = j;
        ipv6Addr.arIP[2] = 1;
        /* try to add IPv6 UC prefix */
        rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, i, ipv6Addr, 24);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixDel failed: vrf=%d", i);
        BREAK_LOOP_IF_ERROR_OCCURS;
    }
    RETURN_IF_LOOP_BREAK_OCCURS;


    ipv6Addr.arIP[0] = 1;
    /* Convert 1st Octet from compress to regular and vice versa */
    for (j = 0; j < step*numPrefixes; j += step)
    {
        ipv6Addr.arIP[1] = j;
        /* try to add IPv6 UC prefix */
        rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, i, ipv6Addr, 16, &nextHopInfo, GT_TRUE, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixAdd failed: vrf=%d", i);
        BREAK_LOOP_IF_ERROR_OCCURS;
    }
    RETURN_IF_LOOP_BREAK_OCCURS;
    for (j = 0; j < step*numPrefixes; j += step)
    {
        ipv6Addr.arIP[1] = j;
        /* try to add IPv6 UC prefix */
        rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, i, ipv6Addr, 16);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixDel failed: vrf=%d", i);
        BREAK_LOOP_IF_ERROR_OCCURS;
    }
    RETURN_IF_LOOP_BREAK_OCCURS;


    ipv6Addr.arIP[0] = 0;
    ipv6Addr.arIP[1] = 0;
    /* Convert 2nd Octet from compress to regular and vice versa */
    for (j = 0; j < step*numPrefixes; j += step)
    {
        ipv6Addr.arIP[2] = j;
        /* try to add IPv6 UC prefix */
        rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, i, ipv6Addr, 24, &nextHopInfo, GT_TRUE, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixAdd failed: vrf=%d", i);
        BREAK_LOOP_IF_ERROR_OCCURS;
    }
    RETURN_IF_LOOP_BREAK_OCCURS;
    for (j = 0; j < step*numPrefixes; j += step)
    {
        ipv6Addr.arIP[2] = j;
        /* try to add IPv6 UC prefix */
        rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, i, ipv6Addr, 24);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixDel failed: vrf=%d", i);
        BREAK_LOOP_IF_ERROR_OCCURS;
    }
    RETURN_IF_LOOP_BREAK_OCCURS;


    /* -------------------------------------------------------------------
     * 3. Create multiple VRFs and add prefixes in each vrf
     * ------------------------------------------------------------------- */

    /* Add vrfs and few prefixes in each vrf */
    for (i = 1; i < numVrfs; i++)
    {
        /* Add virtual router */
        rc = cpssDxChIpLpmVirtualRouterAdd(prvTgfLpmDBId, i, &vr_config);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpLpmVirtualRouterAdd failed: vrf=%d", i);

        /* Add prefixes with incrementing octet 1*/
        for (j = 0; j < step*numPrefixes; j += step)
        {
            ipv6Addr.arIP[0] = 1;
            ipv6Addr.arIP[1] = j;
            /* try to add IPv6 UC prefix */
            rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, i, ipv6Addr, 16, &nextHopInfo, GT_TRUE, GT_FALSE);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixAdd failed: vrf=%d", i);
            BREAK_LOOP_IF_ERROR_OCCURS;
        }
        BREAK_LOOP_IF_ERROR_OCCURS;

        /* Add prefixes with incrementing octet 0*/
        for (j = 0; j<step*numPrefixes; j += step)
        {
            ipv6Addr.arIP[0] = j;
            /* try to add IPv6 UC prefix */
            rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, i, ipv6Addr, 8, &nextHopInfo, GT_TRUE, GT_FALSE);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixAdd failed: vrf=%d", i);
            BREAK_LOOP_IF_ERROR_OCCURS;
        }
        BREAK_LOOP_IF_ERROR_OCCURS;
    }

    /* print the block mapping state */
    rc = prvTgfIpLpmDbgHwBlockInfoPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwBlockInfoPrint failed");
    RETURN_IF_LOOP_BREAK_OCCURS;


    /* -------------------------------------------------------------------
     * 4. Delete prefixes in each vrf and delete VRFs
     * ------------------------------------------------------------------- */

    /* Delete Vrfs */
    for (j = i-1; j > 0; j--)
    {
        /* Flush IPv6 prefixes */
        rc = cpssDxChIpLpmIpv6UcPrefixesFlush(prvTgfLpmDBId, j);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpLpmIpv6UcPrefixesFlush failed: vrf=%d", j);
        BREAK_LOOP_IF_ERROR_OCCURS;

        /* Delete virtual router */
        rc = cpssDxChIpLpmVirtualRouterDel(prvTgfLpmDBId, j);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpLpmVirtualRouterDel failed: vrf=%d", j);
        BREAK_LOOP_IF_ERROR_OCCURS;
    }

    /* print the block mapping state */
    rc = prvTgfIpLpmDbgHwBlockInfoPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwBlockInfoPrint failed");
}


/**
 * @internal prvTgfIpv6McRoutingCompressedRoot function
 * @endinternal
 *
 * @brief Ipv6 Multicast Prefixes add/delete with compressed root node.
 *        1. Add basic configurations.
 *        2. Check root node conversions for source bucket, embedded,
 *           compressed, regular.
*/
GT_VOID prvTgfIpv6McRoutingCompressedRoot
(
    GT_VOID
)
{
    GT_STATUS                              rc = GT_OK;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC         vr_config;
    GT_U32                                 j = 0;
    GT_IPV6ADDR                            ipGrp = {{0}};
    GT_IPV6ADDR                            ipSrc = {{0}};
    PRV_TGF_IP_LTT_ENTRY_STC               mcRouteLttEntry;
    PRV_TGF_IP_MC_ROUTE_ENTRY_STC          mcRouteEntryArray[1];
    CPSS_IP_PROTOCOL_STACK_ENT             protocol = CPSS_IP_PROTOCOL_IPV6_E;
    GT_U32                                 numPrefixes = 6;
    GT_U32                                 step = 2;
    GT_BOOL                                breakNow = GT_FALSE;

    /* -------------------------------------------------------------------
     * 1. Add Basic configuraion.
     * ------------------------------------------------------------------- */

    /* create a MC Route entry */
    cpssOsMemSet(&mcRouteEntryArray[0], 0, sizeof(mcRouteEntryArray[0]));
    mcRouteEntryArray[0].cmd                = CPSS_PACKET_CMD_ROUTE_E;
    mcRouteEntryArray[0].countSet           = CPSS_IP_CNT_SET2_E;
    mcRouteEntryArray[0].RPFFailCommand     = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mcRouteEntryArray[0].ipv6ExternalMllPtr = prvTgfMllPointerIndex;
    mcRouteEntryArray[0].ipv6InternalMllPtr = prvTgfMllPointerIndex + 1;

    /* AUTODOC: add MC route entry with index 10, MLL Ptr 5 */
    rc = prvTgfIpMcRouteEntriesWrite(prvTgfLpmRouteEntryBaseIndex,
                                     1,
                                     protocol,
                                     mcRouteEntryArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite");

    /* fill a nexthope info for the LPM */
    cpssOsMemSet(&mcRouteLttEntry, 0, sizeof(mcRouteLttEntry));
    mcRouteLttEntry.routeEntryBaseIndex = prvTgfLpmRouteEntryBaseIndex;
    mcRouteLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    cpssOsMemSet(&vr_config, 0, sizeof(vr_config));
    vr_config.supportIpv4Uc = GT_TRUE;
    vr_config.defIpv4UcNextHopInfo.ipLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    vr_config.supportIpv6Uc = GT_TRUE;
    vr_config.defIpv6UcNextHopInfo.ipLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    vr_config.supportIpv4Mc = GT_TRUE;
    vr_config.defIpv4McRouteLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    vr_config.supportIpv6Mc = GT_TRUE;
    vr_config.defIpv6McRouteLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;

    /* -------------------------------------------------------------------
     * 2. Check src root node converstions, embedded, compressed, regular.
     * ------------------------------------------------------------------- */

    ipGrp.arIP[0] = 0xFF;
    ipGrp.arIP[14] = 0x22;
    ipGrp.arIP[15] = 0x22;

    ipSrc.arIP[0] = 0;
    /* Add prefix IpGrp = FF::22:22, IpSrc = 0::0/8 */
    rc = prvTgfIpLpmIpv6McEntryAdd(prvTgfLpmDBId, 0, ipGrp, 128, ipSrc, 8,
                                   &mcRouteLttEntry, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6McEntryAdd: vrf = %d", 0);

    ipSrc.arIP[0] = 0xFF;
    /* It is not a valid src address but using it for debug purpose */
    /* Add prefix IpGrp = FF::22:22, IpSrc = FF::0/8 */
    rc = prvTgfIpLpmIpv6McEntryAdd(prvTgfLpmDBId, 0, ipGrp, 128, ipSrc, 8,
                                   &mcRouteLttEntry, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6McEntryAdd: vrf = %d", 0);

    ipSrc.arIP[0] = 1;

    /* Add and delete prefix IpGrp = FF::22:22, IpSrc = 1::0/8.
       Convert src root embed 2_2 to compress and vice versa
    */
    rc = prvTgfIpLpmIpv6McEntryAdd(prvTgfLpmDBId, 0, ipGrp, 128, ipSrc, 8,
                                   &mcRouteLttEntry, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6McEntryAdd: vrf = %d", 0);
    rc = prvTgfIpLpmIpv6McEntryDel(prvTgfLpmDBId, 0, ipGrp, 128, ipSrc, 8);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6McEntryDel: vrf = %d", 0);

    /* Add and delete prefix IpGrp = FF::22:22, IpSrc = 1::0/1.
       Convert src root embed 2_2 to embed 3_3 and vice versa
    */
    rc = prvTgfIpLpmIpv6McEntryAdd(prvTgfLpmDBId, 0, ipGrp, 128, ipSrc, 1,
                                   &mcRouteLttEntry, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6McEntryAdd: vrf = %d", 0);
    rc = prvTgfIpLpmIpv6McEntryDel(prvTgfLpmDBId, 0, ipGrp, 128, ipSrc, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6McEntryDel: vrf = %d", 0);

    /* Convert root from compress to regular and vice versa. prefix length = 8 */
    for (j = 1; j<step*numPrefixes; j += step)
    {
        ipSrc.arIP[0] = j;
        /* try to add IPv6 UC prefix */
        rc = prvTgfIpLpmIpv6McEntryAdd(prvTgfLpmDBId, 0, ipGrp, 128, ipSrc, 8, &mcRouteLttEntry, GT_FALSE, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6McEntryAdd failed: vrf=%d", 0);
        BREAK_LOOP_IF_ERROR_OCCURS;
    }
    for (j = 1; j<step*numPrefixes; j += step)
    {
        ipSrc.arIP[0] = j;
        /* try to add IPv6 UC prefix */
        rc = prvTgfIpLpmIpv6McEntryDel(prvTgfLpmDBId, 0, ipGrp, 128, ipSrc, 8);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6McEntryDel failed: vrf=%d", 0);
        BREAK_LOOP_IF_ERROR_OCCURS;
    }
    RETURN_IF_LOOP_BREAK_OCCURS;

    /* convert root from compress to regular and vice versa. prefix length = 24*/
    for (j = 0; j < step*numPrefixes; j += step)
    {
        ipSrc.arIP[0] = j;
        ipSrc.arIP[2] = 1;
        /* try to add IPv6 UC prefix */
        rc = prvTgfIpLpmIpv6McEntryAdd(prvTgfLpmDBId, 0, ipGrp, 128, ipSrc, 24,
                                       &mcRouteLttEntry, GT_FALSE, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6McEntryAdd failed: vrf=%d", 0);
        BREAK_LOOP_IF_ERROR_OCCURS;
    }
    for (j = 0; j < step*numPrefixes; j += step)
    {
        ipSrc.arIP[0] = j;
        ipSrc.arIP[2] = 1;
        /* try to add IPv6 UC prefix */
        rc = prvTgfIpLpmIpv6McEntryDel(prvTgfLpmDBId, 0, ipGrp, 128, ipSrc, 24);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6McEntryDel failed: vrf=%d", 0);
        BREAK_LOOP_IF_ERROR_OCCURS;
    }
    RETURN_IF_LOOP_BREAK_OCCURS;

    ipSrc.arIP[0] = 1;
    /* Convert 1st Octet from compress to regular and vice versa */
    for (j = 0; j < step*numPrefixes; j += step)
    {
        ipSrc.arIP[1] = j;
        /* try to add IPv6 UC prefix */
        rc = prvTgfIpLpmIpv6McEntryAdd(prvTgfLpmDBId, 0, ipGrp, 128, ipSrc, 16,
                                       &mcRouteLttEntry, GT_FALSE, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6McEntryAdd failed: vrf=%d", 0);
        BREAK_LOOP_IF_ERROR_OCCURS;
    }
    for (j = 0; j < step*numPrefixes; j += step)
    {
        ipSrc.arIP[1] = j;
        /* try to add IPv6 UC prefix */
        rc = prvTgfIpLpmIpv6McEntryDel(prvTgfLpmDBId, 0, ipGrp, 128, ipSrc, 16);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6McEntryDel failed: vrf=%d", 0);
        BREAK_LOOP_IF_ERROR_OCCURS;
    }
    RETURN_IF_LOOP_BREAK_OCCURS;

    ipSrc.arIP[0] = 0;
    ipSrc.arIP[1] = 0;
    /* Convert 2nd Octet from compress to regular and vice versa */
    for (j = 0; j < step*numPrefixes; j += step)
    {
        ipSrc.arIP[2] = j;
        /* try to add IPv6 UC prefix */
        rc = prvTgfIpLpmIpv6McEntryAdd(prvTgfLpmDBId, 0, ipGrp, 128, ipSrc, 24,
                                       &mcRouteLttEntry, GT_FALSE, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6McEntryAdd failed: vrf=%d", 0);
        BREAK_LOOP_IF_ERROR_OCCURS;
    }
    for (j = 0; j < step*numPrefixes; j += step)
    {
        ipSrc.arIP[2] = j;
        /* try to add IPv6 UC prefix */
        rc = prvTgfIpLpmIpv6McEntryDel(prvTgfLpmDBId, 0, ipGrp, 128, ipSrc, 24);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6McEntryDel failed: vrf=%d", 0);
        BREAK_LOOP_IF_ERROR_OCCURS;
    }
    RETURN_IF_LOOP_BREAK_OCCURS;
}

/**
 * @internal prvTgfIpv4UcMcRoutingCompressedRoot function
 * @endinternal
 *
 * @brief Ipv4 unicast and multicast Prefixes add/delete with compressed root node.
 *        1. Add basic configurations.
 *        2. Check root node conversions for source bucket, embedded,
 *           compressed, regular.
*/
GT_VOID prvTgfIpv4UcMcRoutingCompressedRoot
(
    GT_VOID
)
{
    GT_STATUS                              rc = GT_OK;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC         vr_config;
    GT_U32                                 i = 0, j = 0;
    GT_IPADDR                              ipaddr = {0};
    GT_U32                                 numPrefixes = 6;
    GT_U32                                 step = 2;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT   nextHopInfo;
    GT_BOOL                                breakNow = GT_FALSE;
    GT_IPADDR                              ipGrp = {0};
    GT_IPADDR                              ipSrc = {0};
    PRV_TGF_IP_LTT_ENTRY_STC               mcRouteLttEntry;
    PRV_TGF_IP_MC_ROUTE_ENTRY_STC          mcRouteEntryArray[1];
    CPSS_IP_PROTOCOL_STACK_ENT             protocol = CPSS_IP_PROTOCOL_IPV6_E;

    /* -------------------------------------------------------------------
     * 1. Add Basic configuraion.
     * ------------------------------------------------------------------- */

    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfLpmRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType           = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    cpssOsMemSet(&vr_config, 0, sizeof(vr_config));
    vr_config.supportIpv4Uc = GT_TRUE;
    vr_config.defIpv4UcNextHopInfo.ipLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    vr_config.supportIpv6Uc = GT_TRUE;
    vr_config.defIpv6UcNextHopInfo.ipLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    vr_config.supportIpv4Mc = GT_TRUE;
    vr_config.defIpv4McRouteLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    vr_config.supportIpv6Mc = GT_TRUE;
    vr_config.defIpv6McRouteLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;

    /* Set next hop for LPM routing */
    prvTgfIpvxNextHopRouteConfigSet(&prvTgfLpmArpMac, &prvTgfRestoreCfg.lpmArpMacAddrOrig, prvTgfLpmRoutingArpIndex,
                                      PRV_TGF_NEXTHOP_VLANID1_CNS, prvTgfLpmRouteEntryBaseIndex);

    /* create a MC Route entry */
    cpssOsMemSet(&mcRouteEntryArray[0], 0, sizeof(mcRouteEntryArray[0]));
    mcRouteEntryArray[0].cmd                = CPSS_PACKET_CMD_ROUTE_E;
    mcRouteEntryArray[0].countSet           = CPSS_IP_CNT_SET2_E;
    mcRouteEntryArray[0].RPFFailCommand     = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mcRouteEntryArray[0].ipv6ExternalMllPtr = prvTgfMllPointerIndex;
    mcRouteEntryArray[0].ipv6InternalMllPtr = prvTgfMllPointerIndex + 1;

    /* AUTODOC: add MC route entry with index 10, MLL Ptr 5 */
    rc = prvTgfIpMcRouteEntriesWrite(prvTgfLpmRouteEntryBaseIndex,
                                     1,
                                     protocol,
                                     mcRouteEntryArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite");

    /* fill a nexthope info for the LPM */
    cpssOsMemSet(&mcRouteLttEntry, 0, sizeof(mcRouteLttEntry));
    mcRouteLttEntry.routeEntryBaseIndex = prvTgfLpmRouteEntryBaseIndex;
    mcRouteLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* -------------------------------------------------------------------
     * 2. Add UC prefixes
     * ------------------------------------------------------------------- */

    ipaddr.arIP[0] = 0;
    ipaddr.arIP[1] = 0;
    ipaddr.arIP[2] = 1;
    /* Add and Delete Ipv4 UC prefix */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, i, ipaddr, 24, &nextHopInfo, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd failed: vrf=%d", i);

    rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, i, ipaddr, 24);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel failed: vrf=%d", i);


    /* 1. Add prefixes with incrementing octet 2, vrf 0*/
    for (j = 0; j < step*numPrefixes; j += step)
    {
        ipaddr.arIP[0] = 0;
        ipaddr.arIP[2] = j;
        /* try to add Ipv4 UC prefix */
        rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, i, ipaddr, 24, &nextHopInfo, GT_TRUE, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd failed: vrf=%d", i);
        BREAK_LOOP_IF_ERROR_OCCURS;
    }
    RETURN_IF_LOOP_BREAK_OCCURS;

    /* -------------------------------------------------------------------
     * 3. Add Mc prefixes
     * ------------------------------------------------------------------- */

    ipGrp.arIP[0] = 224;
    ipGrp.arIP[1] = 1;
    ipGrp.arIP[2] = 1;
    ipGrp.arIP[3] = 1;

    ipSrc.arIP[0] = 1;
    ipSrc.arIP[1] = 1;
    /* Add prefix IpGrp = FF::22:22, IpSrc = 0::0/8 */
    rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, 0, ipGrp, 32, ipSrc, 16,
                                   &mcRouteLttEntry, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6McEntryAdd: vrf = %d", 0);

    /* 1. Add prefixes with incrementing octet 2, vrf 0*/
    for (j = 0; j < step*numPrefixes; j += step)
    {
        ipGrp.arIP[0] = 224;
        ipGrp.arIP[1] = 1;
        ipGrp.arIP[2] = 1;
        ipGrp.arIP[3] = j;

        ipSrc.arIP[0] = 1;
        ipSrc.arIP[1] = j;
        /* try to add Ipv4 UC prefix */
        rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, i, ipaddr, 24, &nextHopInfo, GT_TRUE, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd failed: vrf=%d", i);
        BREAK_LOOP_IF_ERROR_OCCURS;
    }
    RETURN_IF_LOOP_BREAK_OCCURS;
}

/**
 * @internal prvTgfIpv4McValidityWithUcPresence function
 * @endinternal
 *
 * @brief Add UC prefixes then add MC prefixes to test 
 *        Mc validity logic.
*/
GT_VOID prvTgfIpv4McValidityWithUcPresence
(
    GT_VOID
)
{
    GT_STATUS                              rc = GT_OK;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC         vr_config;
    GT_U32                                 i = 0, j = 0;
    GT_IPADDR                              ipaddr = {0};
    GT_U32                                 numPrefixes = 6;
    GT_U32                                 step = 2;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT   nextHopInfo;
    GT_BOOL                                breakNow = GT_FALSE;
    GT_IPADDR                              ipGrp = {0};
    GT_IPADDR                              ipSrc = {0};
    PRV_TGF_IP_LTT_ENTRY_STC               mcRouteLttEntry;
    PRV_TGF_IP_MC_ROUTE_ENTRY_STC          mcRouteEntryArray[1];
    CPSS_IP_PROTOCOL_STACK_ENT             protocol = CPSS_IP_PROTOCOL_IPV6_E;

    /* -------------------------------------------------------------------
     * 1. Add Basic configuraion.
     * ------------------------------------------------------------------- */

    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfLpmRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType           = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    cpssOsMemSet(&vr_config, 0, sizeof(vr_config));
    vr_config.supportIpv4Uc = GT_TRUE;
    vr_config.defIpv4UcNextHopInfo.ipLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    vr_config.supportIpv6Uc = GT_TRUE;
    vr_config.defIpv6UcNextHopInfo.ipLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    vr_config.supportIpv4Mc = GT_TRUE;
    vr_config.defIpv4McRouteLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    vr_config.supportIpv6Mc = GT_TRUE;
    vr_config.defIpv6McRouteLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;

    /* Set next hop for LPM routing */
    prvTgfIpvxNextHopRouteConfigSet(&prvTgfLpmArpMac, &prvTgfRestoreCfg.lpmArpMacAddrOrig, prvTgfLpmRoutingArpIndex,
                                      PRV_TGF_NEXTHOP_VLANID1_CNS, prvTgfLpmRouteEntryBaseIndex);

    /* create a MC Route entry */
    cpssOsMemSet(&mcRouteEntryArray[0], 0, sizeof(mcRouteEntryArray[0]));
    mcRouteEntryArray[0].cmd                = CPSS_PACKET_CMD_ROUTE_E;
    mcRouteEntryArray[0].countSet           = CPSS_IP_CNT_SET2_E;
    mcRouteEntryArray[0].RPFFailCommand     = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mcRouteEntryArray[0].ipv6ExternalMllPtr = prvTgfMllPointerIndex;
    mcRouteEntryArray[0].ipv6InternalMllPtr = prvTgfMllPointerIndex + 1;

    /* AUTODOC: add MC route entry with index 10, MLL Ptr 5 */
    rc = prvTgfIpMcRouteEntriesWrite(prvTgfLpmRouteEntryBaseIndex,
                                     1,
                                     protocol,
                                     mcRouteEntryArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite");

    /* fill a nexthope info for the LPM */
    cpssOsMemSet(&mcRouteLttEntry, 0, sizeof(mcRouteLttEntry));
    mcRouteLttEntry.routeEntryBaseIndex = prvTgfLpmRouteEntryBaseIndex;
    mcRouteLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* -------------------------------------------------------------------
     * 2. Add UC and MC prefixes
     * ------------------------------------------------------------------- */

    /* 2.1 Add Compressed nodes */
    ipaddr.arIP[0] = 220;
    ipaddr.arIP[1] = 0;
    ipaddr.arIP[2] = 1;
    /* Add and Delete Ipv4 UC prefix */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, i, ipaddr, 24, &nextHopInfo, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd failed: vrf=%d", i);

    ipGrp.arIP[0] = 224;
    ipGrp.arIP[1] = 1;
    ipGrp.arIP[2] = 1;
    ipGrp.arIP[3] = 1;

    ipSrc.arIP[0] = 1;
    ipSrc.arIP[1] = 1;
    ipSrc.arIP[2] = 1;
    /* Add prefix IpGrp = FF::22:22, IpSrc = 0::0/8 */
    rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, 0, ipGrp, 32, ipSrc, 16,
                                   &mcRouteLttEntry, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6McEntryAdd: vrf = %d", 0);

    /* 2.1 Add leaf nodes */
    ipaddr.arIP[0] = 221;
    /* Add and Delete Ipv4 UC prefix */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, i, ipaddr, 8, &nextHopInfo, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd failed: vrf=%d", i);

    ipGrp.arIP[0] = 224;
    ipSrc.arIP[0] = 2;
    ipSrc.arIP[1] = 1;
    /* Add prefix IpGrp = FF::22:22, IpSrc = 0::0/8 */
    rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, 0, ipGrp, 32, ipSrc, 16,
                                   &mcRouteLttEntry, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6McEntryAdd: vrf = %d", 0);


    /* 2.2 Add Regular nodes */
    for (j = 0; j < step*numPrefixes; j += step)
    {
        ipaddr.arIP[0] = 222;
        ipaddr.arIP[1] = j;
        /* Add and Delete Ipv4 UC prefix */
        rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, i, ipaddr, 16, &nextHopInfo, GT_TRUE, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd failed: vrf=%d", i);
        BREAK_LOOP_IF_ERROR_OCCURS;

        ipGrp.arIP[0] = 224;
        ipGrp.arIP[1] = 3 + j;

        ipSrc.arIP[0] = 3;
        ipSrc.arIP[1] = 1;
        /* Add prefix IpGrp = FF::22:22, IpSrc = 0::0/8 */
        rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, 0, ipGrp, 32, ipSrc, 16,
                                       &mcRouteLttEntry, GT_FALSE, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6McEntryAdd: vrf = %d", 0);
        BREAK_LOOP_IF_ERROR_OCCURS;
    }
    RETURN_IF_LOOP_BREAK_OCCURS;

    /* Flush Ipv4 prefixes */
    rc = cpssDxChIpLpmIpv4UcPrefixesFlush(prvTgfLpmDBId, i);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpLpmIpv4UcPrefixesFlush failed: vrf=%d", i);
    rc = cpssDxChIpLpmIpv4McEntriesFlush(prvTgfLpmDBId, i);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpLpmIpvMUcPrefixesFlush failed: vrf=%d", i);

}

/**
 *  @internal prvTgfIpvx4kVrfsAddDeleteCreateVrfs function
 *  @endinternal
 *
 *  @brief Create 4k Vrfs
*/
GT_VOID prvTgfIpvx4kVrfsAddDeleteCreateVrfs
(
    GT_VOID
)
{
    GT_U32                                 i;
    GT_STATUS                              rc = GT_OK;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC         vr_config;
    GT_BOOL                                breakNow = GT_FALSE;

    cpssOsMemSet(&vr_config, 0, sizeof(vr_config));
    vr_config.supportIpv4Uc = GT_TRUE;
    vr_config.defIpv4UcNextHopInfo.ipLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    vr_config.supportIpv6Uc = GT_TRUE;
    vr_config.defIpv6UcNextHopInfo.ipLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    vr_config.supportIpv4Mc = GT_TRUE;
    vr_config.defIpv4McRouteLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    vr_config.supportIpv6Mc = GT_TRUE;
    vr_config.defIpv6McRouteLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;

    /* Delete virtual router */
    rc = cpssDxChIpLpmVirtualRouterDel(prvTgfLpmDBId, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpLpmVirtualRouterDel failed: vrf=%d", 0);

    for (i = 0; i < prvTgfNumVrfs; i++)
    {
        /* Add virtual router */
        rc = cpssDxChIpLpmVirtualRouterAdd(prvTgfLpmDBId, i, &vr_config);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpLpmVirtualRouterAdd failed: vrf=%d", i);
        BREAK_LOOP_IF_ERROR_OCCURS;
    }

    /* print the block mapping state */
    rc = prvTgfIpLpmDbgHwBlockInfoPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwBlockInfoPrint failed");
    RETURN_IF_LOOP_BREAK_OCCURS;
}

/**
 * @internal  prvTgfIpvx4kVrfsAddDeleteAddPrefixes function
 * @endinternal
 *
 * @brief Add Ipv4/Ipv6 UC/MC prefixes
 *
*/
GT_VOID prvTgfIpvx4kVrfsAddDeleteAddPrefixes
(
    GT_VOID
)
{
    GT_STATUS                              rc = GT_OK;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT   nextHopInfo;
    GT_U32                                 i, j;
    GT_IPADDR                              ipaddr = {0};
    GT_IPV6ADDR                            ipv6Addr = {{0}};
    GT_BOOL                                breakNow = GT_FALSE;

    /* Set next hop for LPM routing */
    prvTgfIpvxNextHopRouteConfigSet(&prvTgfLpmArpMac, &prvTgfRestoreCfg.lpmArpMacAddrOrig, prvTgfLpmRoutingArpIndex,
                                      PRV_TGF_NEXTHOP_VLANID1_CNS, prvTgfLpmRouteEntryBaseIndex);

    /* fill a nexthope info for the LPM */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfLpmRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType           = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* Add Ipv6 Prefixes in 4K Vrfs */
    ipv6Addr.arIP[0] = 1;
    for (i=0; i<prvTgfNumVrfs; i++)
    {
        for (j=1; j<8; j++)
        {
            /* Add 2 Ipv6 UC prefixes with prefixlen = 128 */
            ipv6Addr.arIP[15] = j;
            rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, i, ipv6Addr, 128, &nextHopInfo, GT_TRUE, GT_FALSE);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixAdd failed: vrf=%d", i);
            BREAK_LOOP_IF_ERROR_OCCURS;
        }
        BREAK_LOOP_IF_ERROR_OCCURS;
    }

    /* Add Ipv4 Prefixes in 4K Vrfs */
    ipaddr.arIP[0]   = 1;
    for (i=0; i<prvTgfNumVrfs; i++)
    {
        for (j=1; j<8; j++)
        {
            /* Add 2 Ipv4 UC prefixes with prefixlen = 32 */
            ipaddr.arIP[3] = j;
            rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, i, ipaddr, 32, &nextHopInfo, GT_TRUE, GT_FALSE);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd failed: vrf=%d", i);
            BREAK_LOOP_IF_ERROR_OCCURS;
        }
        BREAK_LOOP_IF_ERROR_OCCURS;
    }


    /* print the block mapping state */
    rc = prvTgfIpLpmDbgHwBlockInfoPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwBlockInfoPrint failed");
    RETURN_IF_LOOP_BREAK_OCCURS;
}

/**
 * @internal  prvTgfIpvx4kVrfsAddDeleteDeleteVrfs
 * @endinternal
 *
 * @brief Delete Vrfs
 */
GT_VOID prvTgfIpvx4kVrfsAddDeleteDeleteVrfs
(
    GT_VOID
)
{
    GT_STATUS               rc = GT_OK;
    GT_U32                  i;
    GT_BOOL                 breakNow = GT_FALSE;

    for (i=0; i<prvTgfNumVrfs; i++)
    {
        /* Flush Ipv4 prefixes */
        rc = cpssDxChIpLpmIpv4UcPrefixesFlush(prvTgfLpmDBId, i);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpLpmIpv4UcPrefixesFlush failed: vrf=%d", i);
        BREAK_LOOP_IF_ERROR_OCCURS;
        rc = cpssDxChIpLpmIpv6UcPrefixesFlush(prvTgfLpmDBId, i);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpLpmIpv6UcPrefixesFlush failed: vrf=%d", i);
        BREAK_LOOP_IF_ERROR_OCCURS;


        /* Delete virtual router */
        rc = cpssDxChIpLpmVirtualRouterDel(prvTgfLpmDBId, i);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpLpmVirtualRouterDel failed: vrf=%d", i);
        BREAK_LOOP_IF_ERROR_OCCURS;
    }

    /* print the block mapping state */
    rc = prvTgfIpLpmDbgHwBlockInfoPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwBlockInfoPrint failed");
    RETURN_IF_LOOP_BREAK_OCCURS;
}

/**
 * @internal prvTgfIpv4UcRoutingVrfsAddDeleteForMerge function
 * @endinternal
 *
 * @brief Add and delete Vrfs and prefixes in each vrf such that
 *        defrag occurs in root bucket.
*/
GT_VOID prvTgfIpv4UcRoutingVrfsAddDeleteForMerge
(
    GT_VOID
)
{
    GT_STATUS                             rc = GT_OK;
    GT_IPADDR                             ucIpAddr = {0};
    GT_IPADDR                             lastIpAddrAdded = {0};
    GT_U8                                 stepArr[4];
    GT_U8                                 srcStepArr[4];
    GT_U32                                numOfPrefixesToAdd = 0;
    GT_U32                                numOfPrefixesAdded = 0;
    CPSS_IP_PROTOCOL_STACK_ENT            protocolStack = CPSS_IP_PROTOCOL_IPV4_E;
    CPSS_UNICAST_MULTICAST_ENT            prefixType = CPSS_UNICAST_E;
    GT_U32                                prefixLength = 32;
    GT_U32                                srcPrefixLength = 0;
    GT_U32                                i;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC        vr_config;
    GT_BOOL                               breakNow = GT_FALSE;
    GT_U32                                numVrfs = 1200;
    GT_U32                                startVrf = 10;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT  nextHopInfo;
    GT_U32                                vrId = 20;
    GT_BOOL                               tmpFlag;
    GT_U32                                skipNum = 10;

    /*----------------------------------------------------------------
     * 1. Add basic configuration
     * --------------------------------------------------------------  */
    cpssOsMemSet(&vr_config, 0, sizeof(vr_config));
    vr_config.supportIpv4Uc = GT_TRUE;
    vr_config.defIpv4UcNextHopInfo.ipLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    vr_config.supportIpv6Uc = GT_TRUE;
    vr_config.defIpv6UcNextHopInfo.ipLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    vr_config.supportIpv4Mc = GT_TRUE;
    vr_config.defIpv4McRouteLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    vr_config.supportIpv6Mc = GT_TRUE;
    vr_config.defIpv6McRouteLttEntry.routeType = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;

    /* Set next hop for LPM routing */
    prvTgfIpvxNextHopRouteConfigSet(&prvTgfLpmArpMac, &prvTgfRestoreCfg.lpmArpMacAddrOrig, prvTgfLpmRoutingArpIndex,
                                      PRV_TGF_NEXTHOP_VLANID1_CNS, prvTgfLpmRouteEntryBaseIndex);

    /* fill a nexthope info for the LPM */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfLpmRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType           = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;


    /* change global prefixLength used in the prvTgfIpLpmIpv6UcPrefixAddManyByOctet API */
    rc = prvTgfIpLpmDbgPrefixLengthSet(protocolStack,prefixType,prefixLength,srcPrefixLength);
    if (rc!=GT_OK)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmDbgPrefixLengthSet: %d\n", prvTgfDevNum);
    }

    for (i=0; i<4; i++)
    {
        stepArr[i] = 1;
        srcStepArr[i] = 1;
    }

    /* change global step size */
    rc = tgfCpssDxChIpLpmDbgIpv4SetStep(stepArr, prefixLength, srcStepArr, srcPrefixLength);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfCpssDxChIpLpmDbgIpv6SetStep: %d\n", prvTgfDevNum);

    /*----------------------------------------------------------------
     * 2. Add Vrfs and Delete half of them
     * --------------------------------------------------------------  */
    for (i = startVrf; i < startVrf + numVrfs; i++)
    {
        /* Add virtual router */
        rc = cpssDxChIpLpmVirtualRouterAdd(prvTgfLpmDBId, i, &vr_config);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpLpmVirtualRouterAdd failed: vrf=%d", i);
        BREAK_LOOP_IF_ERROR_OCCURS;
    }

    tmpFlag = GT_FALSE;
    for (i = startVrf; i < startVrf + numVrfs; i++)
    {
        if (i % skipNum == 0)
        {
            tmpFlag =  (tmpFlag == GT_FALSE) ? GT_TRUE : GT_FALSE;
        }
        if (tmpFlag == GT_TRUE)
        {
            /* Add virtual router */
            rc = cpssDxChIpLpmVirtualRouterDel(prvTgfLpmDBId, i);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpLpmVirtualRouterAdd failed: vrf=%d", i);
            BREAK_LOOP_IF_ERROR_OCCURS;
        }
    }

    /* print the block mapping state */
    rc = prvTgfIpLpmDbgHwBlockInfoPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwBlockInfoPrint failed");
    RETURN_IF_LOOP_BREAK_OCCURS;

    /*----------------------------------------------------------------
     * 3. Check merge banks for root bucket
     * --------------------------------------------------------------  */

    /* just add what there is in the HW */
    numOfPrefixesToAdd = 256*256;

    ucIpAddr.arIP[1] = 0;
    ucIpAddr.arIP[3] = 0;
    rc = prvTgfIpLpmIpv4UcPrefixAddManyByOctet(prvTgfLpmDBId, vrId, &ucIpAddr,
                                               prvTgfLpmRouteEntryBaseIndex,
                                               numOfPrefixesToAdd,
                                               3,2,1,0,
                                               &lastIpAddrAdded,
                                               &numOfPrefixesAdded);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv4UcPrefixAddManyByOctet\n");
    cpssOsPrintf("numOfPrefixesAdded: %d\n", numOfPrefixesAdded);

    /* print the block mapping state */
    rc = prvTgfIpLpmDbgHwBlockInfoPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwBlockInfoPrint failed");
    RETURN_IF_LOOP_BREAK_OCCURS;

    prefixLength = 16;
    /* change global prefixLength used in the prvTgfIpLpmIpv6UcPrefixAddManyByOctet API */
    rc = prvTgfIpLpmDbgPrefixLengthSet(protocolStack,prefixType,prefixLength,srcPrefixLength);
    if (rc!=GT_OK)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_FAIL, "prvTgfIpLpmDbgPrefixLengthSet: %d\n", prvTgfDevNum);
    }

    /*----------------------------------------------------------------
     * 4. Check shrink bank for root bucket
     * --------------------------------------------------------------  */

    tmpFlag = GT_TRUE;
    numOfPrefixesToAdd = 20;
    for (i = startVrf; i < startVrf + 20; i++)
    {
        if (i % skipNum == 0)
        {
            tmpFlag =  (tmpFlag == GT_FALSE) ? GT_TRUE : GT_FALSE;
        }
        if (tmpFlag == GT_TRUE)
        {
            rc = prvTgfIpLpmIpv4UcPrefixAddManyByOctet(prvTgfLpmDBId, i, &ucIpAddr,
                                                       prvTgfLpmRouteEntryBaseIndex,
                                                       numOfPrefixesToAdd,
                                                       0,1,2,3,
                                                       &lastIpAddrAdded,
                                                       &numOfPrefixesAdded);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "fail on prvTgfIpLpmIpv4UcPrefixAddManyByOctet\n");
            if (numOfPrefixesToAdd != numOfPrefixesAdded)
            {
                break;
            }
        }
    }

    /* print the block mapping state */
    rc = prvTgfIpLpmDbgHwBlockInfoPrint(prvTgfLpmDBId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmDbgHwBlockInfoPrint failed");

    /*----------------------------------------------------------------
     * 5. Delete all Vrfs
     * --------------------------------------------------------------  */

    tmpFlag = GT_TRUE;
    for (i = startVrf; i < startVrf + numVrfs; i++)
    {
        if (i % skipNum == 0)
        {
            tmpFlag =  (tmpFlag == GT_FALSE) ? GT_TRUE : GT_FALSE;
        }
        if (tmpFlag == GT_TRUE)
        {
            /* Add virtual router */
            rc = cpssDxChIpLpmVirtualRouterDel(prvTgfLpmDBId, i);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpLpmVirtualRouterAdd failed: vrf=%d", i);
            BREAK_LOOP_IF_ERROR_OCCURS;
        }
    }
}
