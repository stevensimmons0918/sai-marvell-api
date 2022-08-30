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
* @file prvTgfIpv4UcPbrLpmRoutingPriority.c
*
* @brief Policy and LPM based IPV4 UC Routing. Decision making based on
*        PBR/LPM/FDB priority
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfTrunkGen.h>
#include <ip/prvTgfBasicIpv4UcRouting.h>
#include <ip/prvTgfIpv4UcRoutingPriotiry.h>
#include <ip/prvTgfIpv4UcPbrLpmRoutingPriority.h>
#include <ip/prvTgfBasicIpv4UcRouting.h>
#include <bridge/prvTgfFdbBasicIpv4UcRouting.h>
#include <trunk/prvTgfTrunk.h>
#include <common/tgfPacketGen.h>
#include <common/tgfConfigGen.h>
#include <gtOs/gtOsMem.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           5

/* nextHop VLAN Id */
#define PRV_TGF_NEXTHOP_VLANID1_CNS       6
#define PRV_TGF_NEXTHOP_VLANID2_CNS       7
#define PRV_TGF_NEXTHOP_VLANID3_CNS       8

/* port index to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         1

/* port index to next hop traffic to */
#define PRV_TGF_NEXTHOP_PORT_IDX_CNS         3

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS            4

/* the counter set for a route entry is linked to */
#define PRV_TGF_COUNT_SET_CNS     CPSS_IP_CNT_SET0_E

/* default number of packets to send */
#define PRV_TGF_BURST_COUNT_DEFAULT_CNS     1

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = PRV_TGF_BURST_COUNT_DEFAULT_CNS;

/* VLANs array */
static GT_U16         prvTgfVlanArray[] = {PRV_TGF_SEND_VLANID_CNS, PRV_TGF_NEXTHOP_VLANID1_CNS,
                                           PRV_TGF_NEXTHOP_VLANID2_CNS, PRV_TGF_NEXTHOP_VLANID3_CNS};

/* the Arp Address index of the Router ARP Table
   to write to the UC Route entry Arp nextHopARPPointer field */
static GT_U32        prvTgfLpmRoutingArpIndex     = 1;
static GT_U32        prvTgfEcmpPbrRoutingArpIndex = 2;
static GT_U32        prvTgfFdbRoutingArpIndex     = 3;
static GT_U32        prvTgfEcmpFdbRoutingArpIndex = 4;
static GT_U32        prvTgfEcmpFdbEntryBaseIndex  = 89;
static GT_U32        prvTgfEcmpPbrEntryBaseIndex  = 95;

static GT_U32       prvTgfIpv4UcFdbEntryIndex;

/* the Route entry index for UC Route entry Table */
static GT_U32        prvTgfEcmpPbrRouteEntryBaseIndex = 8;
static GT_U32        prvTgfLpmRouteEntryBaseIndex = 4;
static GT_U32        prvTgfEcmpFdbRouteEntryBaseIndex = 6;

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 0;

static GT_U32        prvTgfVrfId               = 0;

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfLpmArpMac      = {0x00, 0x00, 0x00, 0x00, 0x00, 0x33};
static TGF_MAC_ADDR  prvTgfEcmpPbrArpMac  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x44};
static TGF_MAC_ADDR  prvTgfFdbArpMac      = {0x00, 0x00, 0x00, 0x00, 0x00, 0x55};
static TGF_MAC_ADDR  prvTgfEcmpFdbArpMac  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x77};

/* MAC2ME entry index */
#define PRV_TGF_MAC_TO_ME_INDEX_CNS     1

/* PCL rule index */
static GT_U32   prvTgfPclRuleIndex = 1;
static GT_U32   prvTgfNumOfPaths = 1;

static TGF_IPV4_ADDR       prvTgfEcmpDstAddr = {2, 2, 2, 4};
static TGF_IPV4_ADDR       prvTgfFdbMismatchAddr = {3, 3, 3, 5};
static TGF_IPV4_ADDR       prvTgfOrigDstAddr = {1, 1, 1, 3};

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                /* saMac */
};
/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};
/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};
/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2A,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x04,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    { 1,  1,  1,  1},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};
/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_SKIP_E ,      NULL},/*reserved for DSA tag (instead of the vlan tag)*/
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/*capture type*/
static TGF_CAPTURE_MODE_ENT captureType = TGF_CAPTURE_MODE_MIRRORING_E;

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_MAC_HASH_FUNC_MODE_ENT              hashModeOrig;
    GT_BOOL                                     fdbRoutingPortEnableOrig;
    GT_ETHERADDR                                fdbArpMacAddrOrig;
    GT_ETHERADDR                                lpmArpMacAddrOrig;
    GT_ETHERADDR                                ecmpPbrArpMacAddrOrig;
    GT_ETHERADDR                                ecmpFdbArpMacAddrOrig;
    GT_U32                                      ecmpFdbNextHopIndex;
    GT_U32                                      ecmpPbrNextHopIndex;
    CPSS_PACKET_CMD_ENT                         nhPacketCmdOrig;
    GT_BOOL                                     ipv4UcPortRoutingEnableOrig;
    GT_BOOL                                     pbrBypassTriggerOrig;
    GT_BOOL                                     fdbUnicastRouteForPbrOrig;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC               routeEntriesArray[1];
} prvTgfRestoreCfg;

/* parameters that is needed to be restored */


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfIpv4UcPbrLpmRoutingPriorityBaseConfigSet function
* @endinternal
*
* @brief   Set Base Configuration
*
* @param[in] prvUtfVrfId    - virtual router ID
*
*/
GT_VOID prvTgfIpv4UcPbrLpmRoutingPriorityBaseConfigSet
(
    GT_U32  prvUtfVrfId
)
{
    GT_STATUS rc = GT_OK;
    GT_U8     tagArray[] = {1, 1, 1, 1};
    PRV_TGF_TTI_MAC_VLAN_STC                    macToMePattern;
    PRV_TGF_TTI_MAC_VLAN_STC                    macToMeMask;

    prvTgfVrfId = prvUtfVrfId;

    /* AUTODOC: create virtual router */
    if (0 != prvUtfVrfId)
    {
        PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv4UcRouteEntryInfo;
        PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv6UcRouteEntryInfo;

        /* clear entry */
        cpssOsMemSet(&defIpv4UcRouteEntryInfo, 0, sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        cpssOsMemSet(&defIpv6UcRouteEntryInfo, 0, sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));

        if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
        {
            if ( (defIpv4UcRouteEntryInfo.ipLttEntry.routeType == PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E)||
                 (defIpv4UcRouteEntryInfo.ipLttEntry.routeType == PRV_TGF_IP_COS_ROUTE_ENTRY_GROUP_E) )
            {
                defIpv4UcRouteEntryInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_E;
            }
            if ( (defIpv6UcRouteEntryInfo.ipLttEntry.routeType == PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E)||
                 (defIpv6UcRouteEntryInfo.ipLttEntry.routeType == PRV_TGF_IP_COS_ROUTE_ENTRY_GROUP_E) )
            {
                defIpv6UcRouteEntryInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_E;
            }

        }

        /* create Virtual Router [prvUtfVrfId] with created default LTT entries */
        rc = prvTgfIpLpmVirtualRouterAdd(prvTgfLpmDBId, prvUtfVrfId,
                                         &defIpv4UcRouteEntryInfo,
                                         &defIpv6UcRouteEntryInfo,
                                         NULL, NULL, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmVirtualRouterAdd: %d", prvTgfDevNum);
    }

    /* AUTODOC: create VLAN 5 with tagged ports 0,1 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS, prvTgfPortsArray, NULL, tagArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);
    /* AUTODOC: create VLAN 6 with tagged ports 2,3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_NEXTHOP_VLANID1_CNS, prvTgfPortsArray + 2, NULL, tagArray + 2, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);
    /* AUTODOC: create VLAN 7 with tagged ports 2,3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_NEXTHOP_VLANID2_CNS, prvTgfPortsArray + 2, NULL, tagArray + 2, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);
    /* AUTODOC: create VLAN 8 with tagged ports 2,3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_NEXTHOP_VLANID3_CNS, prvTgfPortsArray + 2, NULL, tagArray + 2, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: set MAC to ME table */
    cpssOsMemCpy((GT_VOID*)macToMePattern.mac.arEther, (GT_VOID*)prvTgfPacketL2Part.daMac, 
                 sizeof(macToMePattern.mac.arEther));
    macToMePattern.vlanId = PRV_TGF_SEND_VLANID_CNS;


    cpssOsMemSet((GT_VOID*)macToMeMask.mac.arEther, 0xFF,
                 sizeof(macToMeMask.mac.arEther));
    macToMeMask.vlanId = 0xFFF;

    /* AUTODOC: config MAC to ME table entry index 1: */
    /* AUTODOC:   DA=00:00:00:00:34:02, VID=5 */
    rc = prvTgfTtiMacToMeSet(PRV_TGF_MAC_TO_ME_INDEX_CNS, &macToMePattern, &macToMeMask);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacToMeSet");
}

/**
* @internal prvTgfIpv4UcPbrLpmRoutingPriorityPclConfigSet function
* @endinternal
*
* @brief   Set PCL configuration.
*/
GT_VOID prvTgfIpv4UcPbrLpmRoutingPriorityPclConfigSet
(
    GT_VOID
)
{
    GT_STATUS      rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT pattern;
    PRV_TGF_PCL_ACTION_STC      action;

    /* mask for MAC address */
    cpssOsMemSet(&mask, 0, sizeof(mask));

    /* define mask, pattern and action */
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    /* action redirect */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd                       = CPSS_PACKET_CMD_FORWARD_E;
    action.bypassIngressPipe            = GT_FALSE;
    action.redirect.redirectCmd         = PRV_TGF_PCL_ACTION_REDIRECT_CMD_ECMP_E;
    action.redirect.data.routerLttIndex = prvTgfEcmpPbrEntryBaseIndex;

    /* AUTODOC: init PCL Engine for send port 1: */
    /* AUTODOC:   ingress direction, lookup0 */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);

    /* AUTODOC: set PCL rule 1 with: */
    /* AUTODOC:   format INGRESS_STD_IP_L2_QOS, cmd=FORWARD */
    /* AUTODOC:   redirect to routerLtt entry 8 */
    rc = prvTgfPclRuleSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
        prvTgfPclRuleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
        prvTgfDevNum, prvTgfPclRuleIndex);
};

/**
* @internal prvTgfIpv4UcPbrLpmRoutingPriorityFdbRoutingGenericConfigSet function
* @endinternal
*
* @brief   Set FDB Generic Route Configuration
*/
GT_VOID prvTgfIpv4UcPbrLpmRoutingPriorityFdbRoutingGenericConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: get current  CRC Multi HASH mode */
    rc =  prvTgfBrgFdbHashModeGet(prvTgfDevNum, &prvTgfRestoreCfg.hashModeOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbHashModeGet");

    /* AUTODOC: set CRC Multi HASH mode */
    rc =  prvTgfBrgFdbHashModeSet(prvTgfDevNum, PRV_TGF_MAC_HASH_FUNC_CRC_MULTI_HASH_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbHashModeSet");

    /* AUTODOC: get enable Unicast IPv4 Routing status on port 0 */
    rc = prvTgfIpPortRoutingEnableGet( PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E,
                                   &prvTgfRestoreCfg.ipv4UcPortRoutingEnableOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnableGet");


    /* AUTODOC: enable Unicast IPv4 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                               CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                             prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: Get enable/disable status of  using of FDB for Destination IP lookup for IP UC routing */
    rc = prvTgfBrgFdbRoutingPortIpUcEnableGet( prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                           CPSS_IP_PROTOCOL_IPV4_E, &prvTgfRestoreCfg.fdbRoutingPortEnableOrig );
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingPortIpUcEnableGet");

    /* AUTODOC: Enable using of FDB for Destination IP lookup for IP UC routing */
    rc = prvTgfBrgFdbRoutingPortIpUcEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                          CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingPortIpUcEnableSet: %d %d",
                             prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: enable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                             prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* Save the enabling status of bypassing the router triggering requirements for PBR packets */
    rc = prvTgfIpPbrBypassRouterTriggerRequirementsEnableGet(prvTgfDevNum, &prvTgfRestoreCfg.pbrBypassTriggerOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpPbrBypassRouterTriggerRequirementsEnableGet");

    /* Enable bypassing the router triggering requirements for PBR packets */
    rc = prvTgfIpPbrBypassRouterTriggerRequirementsEnableSet(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpPbrBypassRouterTriggerRequirementsEnableSet");

    rc = prvTgfIpFdbUnicastRouteForPbrEnableGet(prvTgfDevNum, &prvTgfRestoreCfg.fdbUnicastRouteForPbrOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpFdbUnicastRouteForPbrEnableGet");

    rc = prvTgfIpFdbUnicastRouteForPbrEnableSet(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpFdbUnicastRouteForPbrEnableSet");

}

static GT_VOID prvTgfIpv4UcPbrLpmRoutingPriorityNextHopRouteConfigSet
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
* @internal prvTgfIpv4UcPbrLpmRoutingPriorityGenericLttRouteConfigSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*/
GT_VOID prvTgfIpv4UcPbrLpmRoutingPriorityGenericLttRouteConfigSet
(
    GT_U32      prvUtfVrfId
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_IP_ECMP_ENTRY_STC               ecmpEntry;

    GT_UNUSED_PARAM(prvUtfVrfId);

    /* Set next hop for LPM routing */
    prvTgfIpv4UcPbrLpmRoutingPriorityNextHopRouteConfigSet(&prvTgfLpmArpMac, &prvTgfRestoreCfg.lpmArpMacAddrOrig,
                                                    prvTgfLpmRoutingArpIndex, PRV_TGF_NEXTHOP_VLANID1_CNS,
                                                    prvTgfLpmRouteEntryBaseIndex);

    /* Set next hop for ECMP FDB routing */
    prvTgfIpv4UcPbrLpmRoutingPriorityNextHopRouteConfigSet(&prvTgfEcmpFdbArpMac, &prvTgfRestoreCfg.ecmpFdbArpMacAddrOrig,
                                                    prvTgfEcmpFdbRoutingArpIndex, PRV_TGF_NEXTHOP_VLANID3_CNS,
                                                    prvTgfEcmpFdbRouteEntryBaseIndex);

    /* save ECMP indirect next hop entry for FDB routing */
    rc = cpssDxChIpEcmpIndirectNextHopEntryGet(prvTgfDevNum, prvTgfEcmpFdbEntryBaseIndex, &prvTgfRestoreCfg.ecmpFdbNextHopIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpEcmpIndirectNextHopEntryGet: %d", prvTgfDevNum);

    /* save ECMP indirect next hop entry for FDB routing */
    rc = cpssDxChIpEcmpIndirectNextHopEntrySet(prvTgfDevNum, prvTgfEcmpFdbEntryBaseIndex, prvTgfEcmpFdbRouteEntryBaseIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpEcmpIndirectNextHopEntrySet: %d", prvTgfDevNum);

    /* set ECMP entry for FDB routing */
    ecmpEntry.numOfPaths = prvTgfNumOfPaths;
    ecmpEntry.randomEnable = GT_FALSE;
    ecmpEntry.routeEntryBaseIndex = prvTgfEcmpFdbRouteEntryBaseIndex;
    ecmpEntry.multiPathMode = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;

    rc = prvTgfIpEcmpEntryWrite(prvTgfDevNum, prvTgfEcmpFdbEntryBaseIndex, &ecmpEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpEcmpEntryWrite: %d", prvTgfDevNum);


    /* Set next hop for ECMP PBR routing */
    prvTgfIpv4UcPbrLpmRoutingPriorityNextHopRouteConfigSet(&prvTgfEcmpPbrArpMac, &prvTgfRestoreCfg.ecmpPbrArpMacAddrOrig,
                                                    prvTgfEcmpPbrRoutingArpIndex, PRV_TGF_NEXTHOP_VLANID2_CNS,
                                                    prvTgfEcmpPbrRouteEntryBaseIndex);

    /* save ECMP indirect next hop entry for PBR routing */
    rc = cpssDxChIpEcmpIndirectNextHopEntryGet(prvTgfDevNum, prvTgfEcmpPbrEntryBaseIndex, &prvTgfRestoreCfg.ecmpPbrNextHopIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpEcmpIndirectNextHopEntryGet: %d", prvTgfDevNum);

    /* set ECMP indirect next hop entry for PBR routing */
    rc = cpssDxChIpEcmpIndirectNextHopEntrySet(prvTgfDevNum, prvTgfEcmpPbrEntryBaseIndex, prvTgfEcmpPbrRouteEntryBaseIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpEcmpIndirectNextHopEntrySet: %d", prvTgfDevNum);

    /* set ECMP entry for PBR routing */
    ecmpEntry.numOfPaths = prvTgfNumOfPaths;
    ecmpEntry.randomEnable = GT_FALSE;
    ecmpEntry.routeEntryBaseIndex = prvTgfEcmpPbrRouteEntryBaseIndex;
    ecmpEntry.multiPathMode = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;

    rc = prvTgfIpEcmpEntryWrite(prvTgfDevNum, prvTgfEcmpPbrEntryBaseIndex, &ecmpEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpEcmpEntryWrite: %d", prvTgfDevNum);

}



/**
* @internal prvTgfIpv4UcPbrLpmRoutingPriorityRouteConfigSet function 
* @endinternal
*
* @brief   Set FDB Route Configuration
*
* @param[in] routingByIndex           - GT_TRUE:  set routing entry by index
*                                      GT_FALSE: set routing entry by message
* @param[in] macEntryPtr              - (pointer to) mac entry
*/
GT_VOID prvTgfIpv4UcPbrLpmRoutingPriorityRouteConfigSet
(
    GT_BOOL                                 routingByIndex,
    PRV_TGF_BRG_MAC_ENTRY_STC               *macEntryPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    counterVal;
    GT_BOOL                      valid;
    GT_BOOL                      skip;
    GT_BOOL                      aged;
    GT_HW_DEV_NUM                hwDevNum;
    PRV_TGF_BRG_MAC_ENTRY_STC    macEntry;

    if (routingByIndex == GT_TRUE)
    {
        rc =  prvTgfBrgFdbMacEntryIndexFind(&(macEntryPtr->key),&prvTgfIpv4UcFdbEntryIndex);
        if (rc == GT_NOT_FOUND)
        {
            rc = GT_OK;
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryIndexFind_MultiHash: %d", prvTgfDevNum);

        prvTgfFdbIpv4UcRoutingRouteConfigurationGetByIndex(&valid,&skip,&aged,&hwDevNum,&macEntry);

        rc = prvTgfBrgFdbMacEntryWrite(prvTgfIpv4UcFdbEntryIndex, GT_FALSE, macEntryPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite: %d", prvTgfDevNum);

        if(valid==GT_FALSE)/* new entry is added need to update the counter */
        {
            rc =  prvTgfBrgFdbBankCounterUpdate(prvTgfDevNum, prvTgfIpv4UcFdbEntryIndex%16,GT_TRUE);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbBankCounterUpdate: %d", prvTgfIpv4UcFdbEntryIndex%16);
        }

        rc =  prvTgfBrgFdbBankCounterValueGet(prvTgfDevNum, prvTgfIpv4UcFdbEntryIndex%16,&counterVal);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbBankCounterValueGet: %d", prvTgfIpv4UcFdbEntryIndex%16);
        PRV_UTF_LOG1_MAC("after adding an entry prvTgfBrgFdbBankCounterValueGet counter = %d\n",counterVal);
    }
    else
    {
        rc = prvTgfBrgFdbMacEntrySet(macEntryPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet: %d", prvTgfDevNum);

    }
}

/**
* @internal prvTgfIpv4UcPbrLpmRoutingPriorityFdbRoutingConfigSet function 
* @endinternal
*
* @brief   Set FDB Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*
*/
GT_VOID prvTgfIpv4UcPbrLpmRoutingPriorityFdbRoutingConfigSet
(
    GT_U32  prvUtfVrfId
)
{
    GT_STATUS                               rc;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_MAC_ENTRY_KEY_STC               entryKey;
    PRV_TGF_BRG_MAC_ENTRY_STC               macEntry;

    /* AUTODOC: write a ARP MAC 00:00:00:00:00:22 to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfFdbArpMac, sizeof(TGF_MAC_ADDR));

    /* Create the Route entry (Next hop) in FDB table and Router ARP Table */
    rc = prvTgfIpRouterArpAddrRead(prvTgfDevNum, prvTgfFdbRoutingArpIndex, &prvTgfRestoreCfg.fdbArpMacAddrOrig);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrRead: %d", prvTgfDevNum);

    rc = prvTgfIpRouterArpAddrWrite(prvTgfFdbRoutingArpIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* AUTODOC: get Packet Commands for FDB routed packets */
    rc = prvTgfBrgFdbRoutingNextHopPacketCmdGet(prvTgfDevNum, &prvTgfRestoreCfg.nhPacketCmdOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingNextHopPacketCmdGet");

    /* AUTODOC: set Packet Commands for FDB routed packets */
    rc = prvTgfBrgFdbRoutingNextHopPacketCmdSet(prvTgfDevNum, CPSS_PACKET_CMD_ROUTE_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingNextHopPacketCmdSet");

    /* AUTODOC: update VRF Id in vlan[PRV_TGF_SEND_VLANID_CNS] */
    rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_SEND_VLANID_CNS, prvUtfVrfId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanVrfIdSet: %d %d",
                             prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* AUTODOC: calculate index for ipv4 uc route entry in FDB */
    cpssOsMemSet(&entryKey, 0, sizeof(entryKey));
    /* fill a destination IP address for the prefix */
    cpssOsMemCpy(entryKey.key.ipv4Unicast.dip, prvTgfPacketIpv4Part.dstAddr, sizeof(entryKey.key.ipv4Unicast.dip));
    /* set key virtual router */
    entryKey.key.ipv4Unicast.vrfId = prvUtfVrfId;

    /* AUTODOC: set  ipv4 uc route entry in FDB*/
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    macEntry.key.entryType = PRV_TGF_FDB_ENTRY_TYPE_IPV4_UC_E;
    cpssOsMemCpy(&macEntry.key.key.ipv4Unicast, &entryKey.key.ipv4Unicast,sizeof(macEntry.key.key.ipv4Unicast));
    macEntry.key.entryType = PRV_TGF_FDB_ENTRY_TYPE_IPV4_UC_E;
    macEntry.fdbRoutingInfo.countSet = PRV_TGF_COUNT_SET_CNS;
    macEntry.fdbRoutingInfo.routingType = CPSS_FDB_UC_ROUTING_TYPE_NH_FDB_E;
    macEntry.fdbRoutingInfo.nextHopVlanId = PRV_TGF_NEXTHOP_VLANID3_CNS;
    macEntry.fdbRoutingInfo.nextHopARPPointer = prvTgfFdbRoutingArpIndex;
    macEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOP_PORT_IDX_CNS];

    prvTgfIpv4UcPbrLpmRoutingPriorityRouteConfigSet(GT_TRUE, &macEntry);

    /* AUTODOC: calculate index for ipv4 uc route entry in FDB */
    cpssOsMemSet(&entryKey, 0, sizeof(entryKey));
    /* fill a destination IP address for the prefix */
    cpssOsMemCpy(entryKey.key.ipv4Unicast.dip, prvTgfEcmpDstAddr, sizeof(entryKey.key.ipv4Unicast.dip));
    /* set key virtual router */
    entryKey.key.ipv4Unicast.vrfId = prvUtfVrfId;

    /* AUTODOC: set  ipv4 uc route entry in FDB*/
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    macEntry.key.entryType = PRV_TGF_FDB_ENTRY_TYPE_IPV4_UC_E;
    cpssOsMemCpy(&macEntry.key.key.ipv4Unicast, &entryKey.key.ipv4Unicast,sizeof(macEntry.key.key.ipv4Unicast));
    macEntry.key.entryType = PRV_TGF_FDB_ENTRY_TYPE_IPV4_UC_E;
    macEntry.fdbRoutingInfo.countSet = PRV_TGF_COUNT_SET_CNS;
    macEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOP_PORT_IDX_CNS];
    macEntry.fdbRoutingInfo.routingType = CPSS_FDB_UC_ROUTING_TYPE_MULTIPATH_ROUTER_E;
    macEntry.fdbRoutingInfo.multipathPointer = prvTgfEcmpFdbEntryBaseIndex;

    prvTgfIpv4UcPbrLpmRoutingPriorityRouteConfigSet(GT_TRUE, &macEntry);
}

/**
* @internal prvTgfIpv4UcPbrLpmRoutingPriorityLpmLeafEntrySet function 
* @endinternal
*
* @brief   Set routing priority between LPM and FDB loopup
*
* @param[in] prvUtfVrfId        - virtual router index
* @param[in] priority           - priority 
* @param[in] applyPbr           - apply policy based routing
* @param[in] deleteExisted      - GT_TRUE - delete existed prefix
*                                 GT_FALSE - don't delete
* @param[in] isEcmpTest         - GT_TRUE - update dst address for ecmp test.
*                               - GT_FALSE - use default dst address
* @param[in] isFdbMismatchTest  - GT_TRUE - update dst address for fdb mismatch test
*                                 GT_FALSE - use default dst address
*
*/
GT_VOID prvTgfIpv4UcPbrLpmRoutingPriorityLpmLeafEntrySet
(
    GT_U32                                        prvUtfVrfId,
    CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_TYPE_ENT    priority,
    GT_BOOL                                       applyPbr,
    GT_BOOL                                       deleteExisted,
    GT_BOOL                                       isEcmpTest,
    GT_BOOL                                       isFdbMismatchTest
)
{
    GT_STATUS                               rc;
    GT_IPADDR                               ipAddr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;

    if (isEcmpTest)
    {
        /* fill a destination IP address for the prefix */
        cpssOsMemCpy(ipAddr.arIP, prvTgfEcmpDstAddr, sizeof(ipAddr.arIP));
    }
    else if (isFdbMismatchTest)
    {
        /* fill a destination IP address for the prefix */
        cpssOsMemCpy(ipAddr.arIP, prvTgfFdbMismatchAddr, sizeof(ipAddr.arIP));
    }
    else
    {
        /* fill a destination IP address for the prefix */
        cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipAddr.arIP));
    }

    if (deleteExisted)
    {
        /* AUTODOC: delete the Ipv4 prefix */
        rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, prvUtfVrfId, ipAddr, 32);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);
    }

    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfLpmRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType           = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
    nextHopInfo.ipLttEntry.priority            = priority;
    nextHopInfo.ipLttEntry.applyPbr            = applyPbr;

    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId,
                                    ipAddr, 32, &nextHopInfo, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);
}

/**
* @internal prvTgfIpv4UcPbrLpmRoutingPriorityTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*  
* @param[in] expectLpmRouting - GT_TRUE - expect LPM routing
*                               GT_FALSE - expect non LPM routing
* @param[in] expectPbrRouting - GT_TRUE - expect PBR routing
*                               GT_FALSE - expect non PBR routing
* @param[in] expectFdbRouting - GT_TRUE - expect FDB routing
*                               GT_FALSE - expect non FDB routing
* @param[in] enableFdbMismatch - GT_TRUE - Modify dst address to get FDB mismatch
*                              - GT_FALSE - do not modify dst address
* @param[in] isEcmpTest        - GT_TRUE - Modify dst address with new address
*                                GT_FALSE - do not modify dst address
*/
GT_VOID prvTgfIpv4UcPbrLpmRoutingPriorityTrafficGenerate
(
    GT_BOOL expectLpmRouting,
    GT_BOOL expectPbrRouting,
    GT_BOOL expectFdbRouting,
    GT_BOOL enableFdbMismatch,
    GT_BOOL isEcmpTest
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          sendPortNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    GT_U32                          nextHopPortNum = prvTgfPortsArray[PRV_TGF_NEXTHOP_PORT_IDX_CNS];
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    TGF_PACKET_PART_STC             *packetPartsPtr;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          portIter    = 0;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[2];
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    PRV_TGF_IP_COUNTER_SET_STC      ipCounters;
    PRV_TGF_IP_ROUTING_MODE_ENT     routingMode;
    TGF_MAC_ADDR                    arpMacAddr;
    GT_U8                           expectVlan;
    GT_U32       egressPortIndex = 0xFF;
    GT_BOOL      isRoutedPacket = GT_FALSE;


    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");
    if (isEcmpTest)
    {
        cpssOsMemCpy(prvTgfPacketIpv4Part.dstAddr, prvTgfEcmpDstAddr, sizeof(prvTgfEcmpDstAddr));
    }
    else if (enableFdbMismatch)
    {
        cpssOsMemCpy(prvTgfPacketIpv4Part.dstAddr, prvTgfFdbMismatchAddr, sizeof(prvTgfFdbMismatchAddr));
    }
    else
    {
        cpssOsMemCpy(prvTgfPacketIpv4Part.dstAddr, prvTgfOrigDstAddr, sizeof(prvTgfOrigDstAddr));
    }

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    egressPortIndex = PRV_TGF_NEXTHOP_PORT_IDX_CNS;

    /* -------------------------------------------------------------------------
        * 1. Setup counters and enable capturing
        *
        */

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
       /* reset IP couters and set ROUTE_ENTRY mode */
       rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter);
       UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d, %d",
                                       prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = nextHopPortNum;

    captureType = TGF_CAPTURE_MODE_MIRRORING_E;


    /* enable capture on next hop port/trunk */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                 prvTgfDevNum, captureType);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");


    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);
    packetPartsPtr = prvTgfPacketPartArray;

    /* calculate packet size */
    rc = prvTgfPacketSizeGet(packetPartsPtr, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* build packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = packetPartsPtr;

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* set transmit timings */
    rc = prvTgfSetTxSetup2Eth(1, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

    /* AUTODOC: send packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:01, VID=5 */
    /* AUTODOC:   srcIP=1.1.1.1 */
    /* AUTODOC:   dstIP=1.1.1.3 for regular test, 1.1.1.3 or 3.2.2.3 for bulk test */

    /* send packet -- send from specific port -- even if member of a trunk */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, sendPortNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum, sendPortNum);
    cpssOsTimerWkAfter(1000);

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* read and check ethernet counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        GT_BOOL isOk;

        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);


        /* AUTODOC: verify routed packet on port 3 */
        if ( (prvTgfPortsArray[portIter]==sendPortNum) || (portIter ==egressPortIndex) )
        {
                isRoutedPacket = GT_TRUE;
        }
        if (isRoutedPacket == GT_TRUE)
        {
            /* packetSize is not changed */
                expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;

                isRoutedPacket = GT_FALSE;
        }
        else
        {
                 /* for other ports */
                expectedCntrs.goodOctetsSent.l[0] = 0;
                expectedCntrs.goodPktsSent.l[0]   = 0;
                expectedCntrs.ucPktsSent.l[0]     = 0;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = 0;
                expectedCntrs.goodPktsRcv.l[0]    = 0;
                expectedCntrs.ucPktsRcv.l[0]      = 0;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
        }

        isOk =
            portCntrs.goodOctetsSent.l[0] == expectedCntrs.goodOctetsSent.l[0] &&
            portCntrs.goodPktsSent.l[0]   == expectedCntrs.goodPktsSent.l[0] &&
            portCntrs.ucPktsSent.l[0]     == expectedCntrs.ucPktsSent.l[0] &&
            portCntrs.brdcPktsSent.l[0]   == expectedCntrs.brdcPktsSent.l[0] &&
            portCntrs.mcPktsSent.l[0]     == expectedCntrs.mcPktsSent.l[0] &&
            portCntrs.goodOctetsRcv.l[0]  == expectedCntrs.goodOctetsRcv.l[0] &&
            portCntrs.goodPktsRcv.l[0]    == expectedCntrs.goodPktsRcv.l[0] &&
            portCntrs.ucPktsRcv.l[0]      == expectedCntrs.ucPktsRcv.l[0] &&
            portCntrs.brdcPktsRcv.l[0]    == expectedCntrs.brdcPktsRcv.l[0] &&
            portCntrs.mcPktsRcv.l[0]      == expectedCntrs.mcPktsRcv.l[0];

        UTF_VERIFY_EQUAL0_STRING_MAC(isOk, GT_TRUE, "get another counters values.");

        /* print expected values if bug */
        if (isOk != GT_TRUE) {
            PRV_UTF_LOG0_MAC("Expected values:\n");
            PRV_UTF_LOG1_MAC("  goodOctetsSent = %d\n", expectedCntrs.goodOctetsSent.l[0]);
            PRV_UTF_LOG1_MAC("    goodPktsSent = %d\n", expectedCntrs.goodPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("      ucPktsSent = %d\n", expectedCntrs.ucPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("    brdcPktsSent = %d\n", expectedCntrs.brdcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("      mcPktsSent = %d\n", expectedCntrs.mcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC("   goodOctetsRcv = %d\n", expectedCntrs.goodOctetsRcv.l[0]);
            PRV_UTF_LOG1_MAC("     goodPktsRcv = %d\n", expectedCntrs.goodPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("       ucPktsRcv = %d\n", expectedCntrs.ucPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("     brdcPktsRcv = %d\n", expectedCntrs.brdcPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC("       mcPktsRcv = %d\n", expectedCntrs.mcPktsRcv.l[0]);
            PRV_UTF_LOG0_MAC("\n");
        }
    }

    /* -------------------------------------------------------------------------
     * 4. Get Trigger Counters
     */
    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", nextHopPortNum);

    /* get trigger counters where packet has MAC DA as prvTgfArpMac */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 0;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);

    /* get trigger counters where packet has Vlan as expected */
    vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[1].modeExtraInfo = 0;
    vfdArray[1].offset = 15;
    vfdArray[1].cycleCount = 1;

    if (expectLpmRouting)
    {
        cpssOsMemCpy(arpMacAddr, &prvTgfLpmArpMac, sizeof(TGF_MAC_ADDR));
        expectVlan = (GT_U8)(PRV_TGF_NEXTHOP_VLANID1_CNS & 0xff);
    }
    else if (expectPbrRouting)
    {
        cpssOsMemCpy(arpMacAddr, &prvTgfEcmpPbrArpMac, sizeof(TGF_MAC_ADDR));
        expectVlan = (GT_U8)(PRV_TGF_NEXTHOP_VLANID2_CNS & 0xff);
    }
    else if (expectFdbRouting)
    {
        cpssOsMemCpy(arpMacAddr, &prvTgfFdbArpMac, sizeof(TGF_MAC_ADDR));
        expectVlan = (GT_U8)(PRV_TGF_NEXTHOP_VLANID3_CNS & 0xff);
    }
    else
    {
        cpssOsMemCpy(arpMacAddr, &prvTgfEcmpFdbArpMac, sizeof(TGF_MAC_ADDR));
        expectVlan = (GT_U8)(PRV_TGF_NEXTHOP_VLANID3_CNS & 0xff);
    }

    cpssOsMemCpy(vfdArray[0].patternPtr, arpMacAddr, sizeof(TGF_MAC_ADDR));
    cpssOsMemCpy(vfdArray[1].patternPtr, &expectVlan, 1);

    /* disable capture on nexthope port , before check the packets to the CPU */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, captureType, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                    prvTgfDevNum, captureType);

    /* check the packets to the CPU */
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 2, vfdArray, &numTriggers);
    PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

    /* check TriggerCounters */
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    if(portInterface.type  == CPSS_INTERFACE_PORT_E)
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                        portInterface.devPort.hwDevNum, portInterface.devPort.portNum);
    }
    else
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, \n",
                                        prvTgfDevNum);
    }

    if(prvTgfBurstCount >= 32)
    {
        /* AUTODOC: for more than 32 transmits check that got only first 32 matches of the MAC of packet */
        UTF_VERIFY_EQUAL0_STRING_MAC(0xFFFFFFFF, numTriggers,
            "\n for more than 32 transmits check that got only first 32 matches of the MAC of packet \n");
    }
    else
    {
        /* AUTODOC: check if packet has the same MAC DA as ARP MAC */
        /* number of triggers should be according to number of transmit*/
        UTF_VERIFY_EQUAL6_STRING_MAC(3 /*prvTgfBurstCount=1*/, numTriggers,
                "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
                arpMacAddr[0], arpMacAddr[1], arpMacAddr[2],
                arpMacAddr[3], arpMacAddr[4], arpMacAddr[5]);
    }

    /* -------------------------------------------------------------------------
     * 5. Get IP Counters
     */

    /* AUTODOC: get and print ip counters values */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++) {
        PRV_UTF_LOG1_MAC("IP counters for Port [%d]:\n", prvTgfPortsArray[portIter]);
        prvTgfCountersIpGet(prvTgfDevNum, portIter, GT_TRUE, &ipCounters);
    }
    PRV_UTF_LOG0_MAC("\n");

    /* restore transmit timings */
    rc = prvTgfSetTxSetup2Eth(0, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

    /* restore original packet */
    cpssOsMemCpy(prvTgfPacketIpv4Part.dstAddr, prvTgfOrigDstAddr, sizeof(prvTgfOrigDstAddr));
}

/**
* @internal prvTgfIpv4UcPbrLpmRoutingPriorityConfigRestore function 
* @endinternal
*
* @brief   Restore configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*
*/
GT_VOID prvTgfIpv4UcPbrLpmRoutingPriorityConfigRestore
(
    GT_U32   prvUtfVrfId
)
{
    GT_U32      vlanIter  = 0;
    GT_U32      vlanCount = sizeof(prvTgfVlanArray)/sizeof(prvTgfVlanArray[0]);
    GT_STATUS   rc        = GT_OK;
    GT_IPADDR   ipAddr;
    GT_BOOL                 ucRouteDelStatusGet;

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    /* save fdb routing uc delete status*/
    rc =  prvTgfBrgFdbRoutingUcDeleteEnableGet(prvTgfDevNum,&ucRouteDelStatusGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingUcDeleteEnableGet: %d", prvTgfDevNum);

    /* set fdb uc route entries */
    rc =  prvTgfBrgFdbRoutingUcDeleteEnableSet(prvTgfDevNum,GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingUcDeleteEnableSet: %d", prvTgfDevNum);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbFlush");

    /* restore FDB uc route entry delete status */
    rc =  prvTgfBrgFdbRoutingUcDeleteEnableSet(prvTgfDevNum,ucRouteDelStatusGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingUcDeleteEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: set CRC Multi HASH mode */
    rc =  prvTgfBrgFdbHashModeSet(prvTgfDevNum, prvTgfRestoreCfg.hashModeOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbHashModeSet");

    prvTgfBurstCount = PRV_TGF_BURST_COUNT_DEFAULT_CNS;/* restore value to default */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* -------------------------------------------------------------------------
     * 2. Restore Route Configuration
     */
    /* fill the destination IP address for Ipv4 prefix in Virtual Router */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipAddr.arIP));
    /* AUTODOC: delete the Ipv4 prefix */
    rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, prvUtfVrfId, ipAddr, 32);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    cpssOsMemCpy(ipAddr.arIP, prvTgfEcmpDstAddr, sizeof(ipAddr.arIP));
    /* AUTODOC: delete the Ipv4 prefix */
    rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, prvUtfVrfId, ipAddr, 32);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    cpssOsMemCpy(ipAddr.arIP, prvTgfFdbMismatchAddr, sizeof(ipAddr.arIP));
    /* AUTODOC: delete the Ipv4 prefix */
    rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, prvUtfVrfId, ipAddr, 32);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    /* AUTODOC: disable Unicast IPv4 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                    CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                    prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_CNS);

        /* AUTODOC: disable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                    prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);
    
    /* remove Virtual Router [prvUtfVrfId] */
    if (0 != prvUtfVrfId)
    {
        rc = prvTgfIpLpmVirtualRouterDel(prvTgfLpmDBId, prvUtfVrfId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                        "prvTgfIpLpmVirtualRouterDel: %d",
                                        prvTgfDevNum);
    }
    /* restore ARP MACs*/
    rc = prvTgfIpRouterArpAddrWrite(prvTgfLpmRoutingArpIndex, &prvTgfRestoreCfg.lpmArpMacAddrOrig);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    rc = prvTgfIpRouterArpAddrWrite(prvTgfEcmpPbrRoutingArpIndex, &prvTgfRestoreCfg.ecmpPbrArpMacAddrOrig);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    rc = prvTgfIpRouterArpAddrWrite(prvTgfFdbRoutingArpIndex, &prvTgfRestoreCfg.fdbArpMacAddrOrig);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    rc = prvTgfIpRouterArpAddrWrite(prvTgfEcmpFdbRoutingArpIndex, &prvTgfRestoreCfg.ecmpFdbArpMacAddrOrig);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* AUTODOC: restore Packet Commands for FDB routed packets */
    rc = prvTgfBrgFdbRoutingNextHopPacketCmdSet( prvTgfDevNum, prvTgfRestoreCfg.nhPacketCmdOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingNextHopPacketCmdSet");

    /* AUTODOC: restore enable value of FDB IP lookup for IP UC routing */
    rc = prvTgfBrgFdbRoutingPortIpUcEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                          CPSS_IP_PROTOCOL_IPV4_E, prvTgfRestoreCfg.fdbRoutingPortEnableOrig);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingPortIpUcEnableSet: %d %d",
                             prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: restore UC route entry with nexthop VLAN 6 and nexthop port 3 */
    rc = prvTgfIpUcRouteEntriesWrite(prvTgfEcmpPbrRouteEntryBaseIndex, prvTgfRestoreCfg.routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);
    rc = prvTgfIpUcRouteEntriesWrite(prvTgfLpmRouteEntryBaseIndex, prvTgfRestoreCfg.routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);
    rc = prvTgfIpUcRouteEntriesWrite(prvTgfEcmpFdbRouteEntryBaseIndex, prvTgfRestoreCfg.routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

    rc = prvTgfIpFdbUnicastRouteForPbrEnableSet(prvTgfDevNum, prvTgfRestoreCfg.fdbUnicastRouteForPbrOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpFdbUnicastRouteForPbrEnableSet");

    /* restore ECMP indirect next hop entry for FDB routing */
    rc = cpssDxChIpEcmpIndirectNextHopEntrySet(prvTgfDevNum, prvTgfEcmpFdbEntryBaseIndex, prvTgfRestoreCfg.ecmpFdbNextHopIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpEcmpIndirectNextHopEntryGet: %d", prvTgfDevNum);

    /* restore ECMP indirect next hop entry for PBR routing */
    rc = cpssDxChIpEcmpIndirectNextHopEntrySet(prvTgfDevNum, prvTgfEcmpPbrEntryBaseIndex, prvTgfRestoreCfg.ecmpPbrNextHopIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpEcmpIndirectNextHopEntryGet: %d", prvTgfDevNum);

    /* Save the enabling status of bypassing the router triggering requirements for PBR packets */
    rc = prvTgfIpPbrBypassRouterTriggerRequirementsEnableSet(prvTgfDevNum, prvTgfRestoreCfg.pbrBypassTriggerOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpPbrBypassRouterTriggerRequirementsEnableSet");

    /* Restore PCL configuration */
    rc = prvTgfPclRuleValidStatusSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E, prvTgfPclRuleIndex, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet");

    prvTgfPclRestore();

    /* -------------------------------------------------------------------------
     * 3. Restore Base Configuration
     */

    /* delete port from both VLANs (support route from trunk A to trunk A) */
    for (vlanIter = 0; vlanIter < vlanCount; vlanIter++)
    {
        /* AUTODOC: invalidate vlan entries 5,6 */
        rc = prvTgfBrgVlanEntryInvalidate(prvTgfVlanArray[vlanIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d, %d",
                                     prvTgfDevNum, prvTgfVlanArray[vlanIter]);
    }

    /* AUTODOC: disable VLAN based MAC learning */
    rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_SEND_VLANID_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);

    /* restore default VLAN 1 */
    prvTgfBrgVlanEntryRestore(1);
}

