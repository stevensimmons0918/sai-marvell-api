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
* @file prvTgfTunnelStartIpv4overGreIpv4.c
*
* @brief Tunnel Start: Ipv4 over GRE Ipv4 - Basic
*
* @version   24
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
#include <common/tgfTunnelGen.h>
#include <tunnel/prvTgfTunnelStartIpv4overGreIpv4.h>



/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* GRE header size (in bytes) */
#define TGF_GRE_HEADER_SIZE_CNS           4

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           5

/* nextHop VLAN Id */
#define PRV_TGF_NEXTHOPE_VLANID_CNS       6

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS            4

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         0

/* port number to send traffic to */
#define PRV_TGF_FDB_PORT_IDX_CNS          1

/* nextHop port number to receive traffic from */
#define PRV_TGF_NEXTHOPE_PORT_IDX_CNS     3

/* the counter set for a route entry is linked to */
#define PRV_TGF_COUNT_SET_CNS     CPSS_IP_CNT_SET0_E

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = 1;

/* VLANs array */
static GT_U8         prvTgfVlanArray[] = {PRV_TGF_SEND_VLANID_CNS, PRV_TGF_NEXTHOPE_VLANID_CNS};

/* VLANs array */
static GT_U8         prvTgfVlanPerPortArray[PRV_TGF_PORT_COUNT_CNS] = {
    PRV_TGF_SEND_VLANID_CNS, PRV_TGF_SEND_VLANID_CNS,
    PRV_TGF_NEXTHOPE_VLANID_CNS, PRV_TGF_NEXTHOPE_VLANID_CNS
};

/* the Route entry index for UC Route entry Table */
static GT_U32        prvTgfRouteEntryBaseIndex = 5;

/* line index for the tunnel start entry in the router ARP / tunnel start table (0..1023) */
static GT_U32        prvTgfRouterArpTunnelStartLineIndex = 8;

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 0;

/* Tunnel next hop MAC DA */
static TGF_MAC_ADDR  prvTgfTunnelMacDa = {0x88, 0x77, 0x11, 0x11, 0x55, 0x66};

/* Tunnel destination IP */
static TGF_IPV4_ADDR prvTgfTunnelDstIp = {1, 1, 1, 2};

/* Tunnel source IP */
static TGF_IPV4_ADDR prvTgfTunnelSrcIp = {2, 2, 3, 3};

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
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
    0x2E,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0xFF,               /* protocol */
    0x73C9,             /* csum */
    { 2,  2,  2,  2},   /* srcAddr */
    { 1,  2,  1,  3}    /* dstAddr */
};
/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19
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
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfTunnelStartIpv4overGreIpv4BaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfTunnelStartIpv4overGreIpv4BaseConfigurationSet
(
    GT_VOID
)
{
    GT_BOOL                     isTagged  = GT_FALSE;
    GT_U32                      vlanIter  = 0;
    GT_U32                      portIter  = 0;
    GT_U32                      vlanCount = sizeof(prvTgfVlanArray);
    GT_STATUS                   rc        = GT_OK;
    CPSS_PORTS_BMP_STC          portsMembers = {{0, 0}};
    CPSS_PORTS_BMP_STC          portsTagging = {{0, 0}};
    PRV_TGF_BRG_VLAN_INFO_STC   vlanInfo;
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC  portsTaggingCmd;
    GT_U8       devNum  = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&devNum, UTF_NONE_FAMILY_E);

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Base Configuration =======\n");

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    /* set vlan entry */
    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(portsTaggingCmd));

    vlanInfo.unkSrcAddrSecBreach  = GT_FALSE;
    vlanInfo.unregNonIpMcastCmd   = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv4McastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv6McastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unkUcastCmd          = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv4BcastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregNonIpv4BcastCmd = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.ipv4IgmpToCpuEn      = GT_FALSE;
    vlanInfo.mirrToRxAnalyzerEn   = GT_FALSE;
    vlanInfo.ipv6IcmpToCpuEn      = GT_FALSE;
    vlanInfo.ipCtrlToCpuEn        = PRV_TGF_BRG_IP_CTRL_NONE_E;
    vlanInfo.ipv4IpmBrgMode       = CPSS_BRG_IPM_SGV_E;
    vlanInfo.ipv6IpmBrgMode       = CPSS_BRG_IPM_SGV_E;
    vlanInfo.ipv4IpmBrgEn         = GT_FALSE;
    vlanInfo.ipv6IpmBrgEn         = GT_FALSE;
    vlanInfo.ipv6SiteIdMode       = CPSS_IP_SITE_ID_INTERNAL_E;
    vlanInfo.ipv4UcastRouteEn     = GT_FALSE;
    vlanInfo.ipv4McastRouteEn     = GT_FALSE;
    vlanInfo.ipv6UcastRouteEn     = GT_FALSE;
    vlanInfo.ipv6McastRouteEn     = GT_FALSE;
    vlanInfo.stgId                = 0;
    vlanInfo.autoLearnDisable     = GT_TRUE;
    vlanInfo.naMsgToCpuEn         = GT_TRUE;
    vlanInfo.mruIdx               = 0;
    vlanInfo.bcastUdpTrapMirrEn   = GT_FALSE;
    /* go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* AUTODOC: create VLAN 5 with untagged ports [0,1] */
        /* AUTODOC: create VLAN 6 with untagged ports [2,3] */
        for (vlanIter = 0; vlanIter < vlanCount; vlanIter++)
        {
            rc = prvTgfBrgVlanEntryWrite(devNum, prvTgfVlanArray[vlanIter],
                                         &portsMembers, &portsTagging,
                                         &vlanInfo, &portsTaggingCmd);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d, %d",
                                         devNum, prvTgfVlanArray[vlanIter]);
        }

        /* add ports to vlan member */
        for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
        {
            rc = prvTgfBrgVlanMemberAdd(devNum, prvTgfVlanPerPortArray[portIter],
                                        prvTgfPortsArray[portIter], isTagged);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberAdd: %d, %d, %d, %d",
                                         devNum, prvTgfVlanPerPortArray[portIter],
                                         prvTgfPortsArray[portIter], isTagged);
        }
    }
    /* AUTODOC: enable VLAN based MAC learning for VLAN 5 */
    rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_SEND_VLANID_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);

    /* create a macEntry with .daRoute = GT_TRUE */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    macEntry.key.entryType                  = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId         = PRV_TGF_SEND_VLANID_CNS;
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum    = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS];
    macEntry.isStatic                       = GT_TRUE;
    macEntry.daCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.saCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.daRoute                        = GT_TRUE;
    macEntry.sourceId                       = 0;
    macEntry.userDefined                    = 0;
    macEntry.daQosIndex                     = 0;
    macEntry.saQosIndex                     = 0;
    macEntry.daSecurityLevel                = 0;
    macEntry.saSecurityLevel                = 0;
    macEntry.appSpecificCpuCode             = GT_FALSE;
    macEntry.mirrorToRxAnalyzerPortEn       = GT_FALSE;

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 1 */
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite: %d", prvTgfDevNum);
}

/**
* @internal prvTgfTunnelStartIpv4overGreIpv4LttRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*                                       None
*/
static GT_VOID prvTgfTunnelStartIpv4overGreIpv4LttRouteConfigurationSet
(
    GT_U32      prvUtfVrfId
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *regularEntryPtr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR                               ipAddr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv4UcRouteEntryInfo;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv6UcRouteEntryInfo;
    PRV_TGF_IP_LTT_ENTRY_STC                *ipLttEntryPtr = NULL;
    GT_U32                                  numOfPaths;
    GT_U32                                  ii;

    PRV_UTF_LOG0_MAC("======= Setting LTT Route Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 1. Enable Routing
     */

    /* AUTODOC: enable Unicast IPv4 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: update VRF Id in vlan[PRV_TGF_SEND_VLANID_CNS] */
    rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_SEND_VLANID_CNS, prvUtfVrfId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanVrfIdSet: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* AUTODOC: enable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 2. Create the Route entry (Next hop) in Route table and Router ARP Table
     */

    /* write a UC Route entry to the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    regularEntryPtr = &routeEntriesArray[0];
    regularEntryPtr->cmd                        = CPSS_PACKET_CMD_ROUTE_E;
    regularEntryPtr->cpuCodeIndex               = 0;
    regularEntryPtr->appSpecificCpuCodeEnable   = GT_FALSE;
    regularEntryPtr->unicastPacketSipFilterEnable = GT_FALSE;
    regularEntryPtr->ttlHopLimitDecEnable       = GT_FALSE;
    regularEntryPtr->ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
    regularEntryPtr->ingressMirror              = GT_FALSE;
    regularEntryPtr->qosProfileMarkingEnable    = GT_FALSE;
    regularEntryPtr->qosProfileIndex            = 0;
    regularEntryPtr->qosPrecedence              = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    regularEntryPtr->modifyUp                   = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    regularEntryPtr->modifyDscp                 = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    regularEntryPtr->countSet                   = PRV_TGF_COUNT_SET_CNS;
    regularEntryPtr->trapMirrorArpBcEnable      = GT_FALSE;
    regularEntryPtr->sipAccessLevel             = 0;
    regularEntryPtr->dipAccessLevel             = 0;
    regularEntryPtr->ICMPRedirectEnable         = GT_FALSE;
    regularEntryPtr->scopeCheckingEnable        = GT_FALSE;
    regularEntryPtr->siteId                     = CPSS_IP_SITE_ID_INTERNAL_E;
    regularEntryPtr->mtuProfileIndex            = 0;
    regularEntryPtr->isTunnelStart              = GT_TRUE;
    regularEntryPtr->nextHopVlanId              = PRV_TGF_NEXTHOPE_VLANID_CNS;
    regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
    regularEntryPtr->nextHopInterface.devPort.hwDevNum = prvTgfDevNum;
    regularEntryPtr->nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS];
    regularEntryPtr->nextHopARPPointer          = 0;
    regularEntryPtr->nextHopTunnelPointer       = prvTgfRouterArpTunnelStartLineIndex;

    /* AUTODOC: add UC route entry with nexthop VLAN 6 and nexthop port 3 */
    rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);


    /* AUTODOC: read and check the UC Route entry from the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum, prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesRead: %d", prvTgfDevNum);

    PRV_UTF_LOG2_MAC("nextHopVlanId = %d, portNum = %d\n",
                     routeEntriesArray[0].nextHopVlanId,
                     routeEntriesArray[0].nextHopInterface.devPort.portNum);

    /* -------------------------------------------------------------------------
     * 3. Create Virtual Router [prvUtfVrfId]
     */

    /* AUTODOC: create Virtual Router [prvUtfVrfId] */

    if (0 != prvUtfVrfId)
    {
        /* clear entry */
        cpssOsMemSet(&defIpv4UcRouteEntryInfo, 0,
                     sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        cpssOsMemSet(&defIpv6UcRouteEntryInfo, 0,
                         sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));

        /* define max number of paths */
        if(prvUtfDeviceTestNotSupport(prvTgfDevNum, UTF_PUMA_E))
        {
            numOfPaths = 0;
        }
        else
        {
            numOfPaths = 1;
        }

        for (ii = 0; ii < 2; ii++)
        {
            if (ii == 0)
            {
                ipLttEntryPtr = &defIpv4UcRouteEntryInfo.ipLttEntry;
            }
            if (ii == 1)
            {
                ipLttEntryPtr = &defIpv6UcRouteEntryInfo.ipLttEntry;
            }
            /* set defUcRouteEntryInfo */
            ipLttEntryPtr->routeType                = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;
            ipLttEntryPtr->numOfPaths               = numOfPaths;
            ipLttEntryPtr->routeEntryBaseIndex      = prvTgfRouteEntryBaseIndex;
            ipLttEntryPtr->ucRPFCheckEnable         = GT_FALSE;
            ipLttEntryPtr->sipSaCheckMismatchEnable = GT_FALSE;
            ipLttEntryPtr->ipv6MCGroupScopeLevel    = CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E;
        }

        /* create Virtual Router [prvUtfVrfId] with created default LTT entries */
        rc = prvTgfIpLpmVirtualRouterAdd(prvTgfLpmDBId, prvUtfVrfId,
                                         &defIpv4UcRouteEntryInfo,
                                         &defIpv6UcRouteEntryInfo,
                                         NULL, NULL, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "prvTgfIpLpmVirtualRouterAdd: %d",
                                     prvTgfDevNum);
    }

    /* -------------------------------------------------------------------------
     * 4. Create a new Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* fill a destination IP address for the prefix */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipAddr.arIP));

    /* AUTODOC: add IPv4 UC prefix 1.1.1.3/32 */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId, ipAddr,
                                    32, &nextHopInfo, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);
}

/**
* @internal prvTgfTunnelStartIpv4overGreIpv4PbrConfigurationSet function
* @endinternal
*
* @brief   Set PBR Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*                                       None
*/
static GT_VOID prvTgfTunnelStartIpv4overGreIpv4PbrConfigurationSet
(
    GT_U32      prvUtfVrfId
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR                               ipAddr;
    GT_U32                                  ii;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    *defUcNextHopInfoPtr = NULL;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv4UcNextHopInfo;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    defIpv6UcNextHopInfo;

    PRV_UTF_LOG0_MAC("==== Setting PBR Configuration ====\n");


    /* -------------------------------------------------------------------------
     * 1. PCL Config
     */

    /* init PCL Engine for send port */
    rc = prvTgfPclDefPortInitExt1(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_1_E,/* PBR must be in lookup 1 ! */
        PRV_TGF_PCL_PBR_ID_FOR_VIRTUAL_ROUTER_CNS, /* pclId */
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInitExt1: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 2. IP Config
     */

    /* update VRF Id in vlan[PRV_TGF_SEND_VLANID_CNS] */
    rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_SEND_VLANID_CNS, prvUtfVrfId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanVrfIdSet: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* enable IPv4 Unicast Routing on Vlan */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 3. Create Virtual Router [prvUtfVrfId]
     */

    /* AUTODOC: create Virtual Router [prvUtfVrfId] */
    if (0 != prvUtfVrfId)
    {
        /* set next hop to all 0 and change only values other than 0 */
        cpssOsMemSet(&defIpv4UcNextHopInfo, 0,
                     sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
        cpssOsMemSet(&defIpv6UcNextHopInfo, 0,
                     sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));

        /* ipv4 uc and ipv6 uc default route */
        for (ii = 0; ii < 2; ii++)
        {
            if (ii == 0)
            {
                defUcNextHopInfoPtr = &defIpv4UcNextHopInfo;
            }
            else
            {
                defUcNextHopInfoPtr = &defIpv6UcNextHopInfo;
            }
            defUcNextHopInfoPtr->pclIpUcAction.pktCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
            defUcNextHopInfoPtr->pclIpUcAction.mirror.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E;
            defUcNextHopInfoPtr->pclIpUcAction.vlan.modifyVlan = CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
            defUcNextHopInfoPtr->pclIpUcAction.redirect.redirectCmd = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
            defUcNextHopInfoPtr->pclIpUcAction.ipUcRoute.doIpUcRoute = GT_TRUE;
        }

        prvTgfIpLpmVirtualRouterAdd(prvTgfLpmDBId,
                                    prvUtfVrfId,
                                    &defIpv4UcNextHopInfo,
                                    &defIpv6UcNextHopInfo,
                                    NULL, NULL, NULL);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "prvTgfIpLpmVirtualRouterAdd: %d",
                                     prvTgfDevNum);
    }

    /* -------------------------------------------------------------------------
     * 4. Create a new Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));

    nextHopInfo.pclIpUcAction.pktCmd                        = CPSS_PACKET_CMD_FORWARD_E;
    nextHopInfo.pclIpUcAction.mirror.cpuCode                = (CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + 1);
    nextHopInfo.pclIpUcAction.matchCounter.enableMatchCount = GT_TRUE;
    nextHopInfo.pclIpUcAction.vlan.vlanId                   = PRV_TGF_NEXTHOPE_VLANID_CNS;

    nextHopInfo.pclIpUcAction.redirect.redirectCmd          = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.type            = CPSS_INTERFACE_PORT_E;
    nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.devPort.hwDevNum  = prvTgfDevNum;
    nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS];

    nextHopInfo.pclIpUcAction.redirect.data.outIf.outlifType = PRV_TGF_OUTLIF_TYPE_TUNNEL_E;
    nextHopInfo.pclIpUcAction.redirect.data.outIf.outlifPointer.tunnelStartPtr.tunnelType = PRV_TGF_PCL_ACTION_REDIRECT_TUNNEL_TYPE_IP_E;
    nextHopInfo.pclIpUcAction.redirect.data.outIf.outlifPointer.tunnelStartPtr.ptr = prvTgfRouterArpTunnelStartLineIndex;

    /* fill a destination IP address for the prefix */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipAddr.arIP));

    /* call CPSS function */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, prvUtfVrfId, ipAddr,
                                    32, &nextHopInfo, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);
}

/**
* @internal prvTgfTunnelStartIpv4overGreIpv4RouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*                                       None
*/
GT_VOID prvTgfTunnelStartIpv4overGreIpv4RouteConfigurationSet
(
    GT_U32      prvUtfVrfId
)
{
    PRV_TGF_IP_ROUTING_MODE_ENT routingMode;
    GT_STATUS                   rc = GT_OK;

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

    switch(routingMode)
    {
        case PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E:

            prvTgfTunnelStartIpv4overGreIpv4PbrConfigurationSet(prvUtfVrfId);
            break;

        case PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E:

            prvTgfTunnelStartIpv4overGreIpv4LttRouteConfigurationSet(prvUtfVrfId);
            break;

        case PRV_TGF_IP_ROUTING_MODE_UNDEFINED_E:

            break;
    }
}

/**
* @internal prvTgfTunnelStartIpv4overGreIpv4TunnelConfigurationSet function
* @endinternal
*
* @brief   Set Tunnel Configuration
*/
GT_VOID prvTgfTunnelStartIpv4overGreIpv4TunnelConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                      rc = GT_OK;
    CPSS_TUNNEL_TYPE_ENT           tunnelType;
    CPSS_TUNNEL_TYPE_ENT           tunnelTypeGet;
    PRV_TGF_TUNNEL_START_ENTRY_UNT tunnelEntry;

    PRV_UTF_LOG0_MAC("======= Setting Tunnel Configuration =======\n");

    if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
    {
        tunnelType = CPSS_TUNNEL_GENERIC_IPV4_E;
        tunnelTypeGet = CPSS_TUNNEL_GENERIC_IPV4_E;
    }
    else
    {
        tunnelType = CPSS_TUNNEL_X_OVER_GRE_IPV4_E;
        tunnelTypeGet = CPSS_TUNNEL_X_OVER_GRE_IPV4_E;
    }

    cpssOsMemSet(&tunnelEntry,0,sizeof(tunnelEntry));
    /*** Set a tunnel start entry ***/
    tunnelEntry.ipv4Cfg.tagEnable        = GT_TRUE;
    tunnelEntry.ipv4Cfg.vlanId           = PRV_TGF_NEXTHOPE_VLANID_CNS;
    tunnelEntry.ipv4Cfg.upMarkMode       = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tunnelEntry.ipv4Cfg.up               = 0;
    tunnelEntry.ipv4Cfg.dscpMarkMode     = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tunnelEntry.ipv4Cfg.dscp             = 0;
    tunnelEntry.ipv4Cfg.dontFragmentFlag = GT_FALSE;
    tunnelEntry.ipv4Cfg.ttl              = 33;
    tunnelEntry.ipv4Cfg.autoTunnel       = GT_FALSE;
    tunnelEntry.ipv4Cfg.autoTunnelOffset = 0;
    tunnelEntry.ipv4Cfg.ethType          = CPSS_TUNNEL_GRE0_ETHER_TYPE_E;
    tunnelEntry.ipv4Cfg.cfi              = 0;
    tunnelEntry.ipv4Cfg.retainCrc        = GT_FALSE;
    if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
        tunnelEntry.ipv4Cfg.ipHeaderProtocol = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;

    /* tunnel next hop MAC DA, IP DA, IP SA */
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.macDa.arEther, prvTgfTunnelMacDa, sizeof(TGF_MAC_ADDR));
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.destIp.arIP, prvTgfTunnelDstIp, sizeof(TGF_IPV4_ADDR));
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.srcIp.arIP, prvTgfTunnelSrcIp, sizeof(TGF_IPV4_ADDR));

    /* AUTODOC: add TS entry 8 with: */
    /* AUTODOC:   tunnelType=IPV4_OVER_IPV4 */
    /* AUTODOC:   vlanId=6, ttl=33, ethType=TUNNEL_GRE0_ETHER_TYPE */
    /* AUTODOC:   DA=88:77:11:11:55:66, srcIp=2.2.3.3, dstIp=1.1.1.2 */
    rc = prvTgfTunnelStartEntrySet(prvTgfRouterArpTunnelStartLineIndex, tunnelType, &tunnelEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntrySet: %d", prvTgfDevNum);

    /*** Get a tunnel start entry ***/
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));

    /* AUTODOC: get and check added TS Entry */
    rc = prvTgfTunnelStartEntryGet(prvTgfDevNum, prvTgfRouterArpTunnelStartLineIndex, &tunnelTypeGet, &tunnelEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntryGet: %d", prvTgfDevNum);

    /* Check vlanId field */
    UTF_VERIFY_EQUAL1_STRING_MAC(PRV_TGF_NEXTHOPE_VLANID_CNS, tunnelEntry.ipv4Cfg.vlanId,
                                 "tunnelEntry.ipv4Cfg.vlanId: %d", tunnelEntry.ipv4Cfg.vlanId);

    /* Check macDa field */
    rc = cpssOsMemCmp(tunnelEntry.ipv4Cfg.macDa.arEther, prvTgfTunnelMacDa, sizeof(TGF_MAC_ADDR)) == 0 ?
        GT_OK : GT_FALSE;
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tunnelEntry.ipv4Cfg.macDa.arEther[5]: 0x%2X",
                                 tunnelEntry.ipv4Cfg.macDa.arEther[5]);
}

/**
* @internal prvTgfTunnelStartIpv4overGreIpv4TrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfTunnelStartIpv4overGreIpv4TrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          portIter    = 0;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[1];
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    PRV_TGF_IP_COUNTER_SET_STC      ipCounters;
    PRV_TGF_IP_ROUTING_MODE_ENT     routingMode;

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");


    /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
     */

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (routingMode == PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E)
        {
            /* reset IP couters and set ROUTE_ENTRY mode */
            rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }
    }

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* setup nexthope portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS];

    /* enable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS]);


    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    /* number of parts in packet */
    partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);

    /* calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketPartArray, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* build packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = prvTgfPacketPartArray;

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* AUTODOC: send IPv4 packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:11, VID=5 */
    /* AUTODOC:   srcIP=2.2.2.2, dstIP=1.2.1.3 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* disable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_NEXTHOPE_PORT_IDX_CNS]);


    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* AUTODOC: verify to get tunneled packet on port 3 with: */
    /* AUTODOC:   DA=88:77:11:11:55:66, SA=00:00:00:00:00:06 */
    /* AUTODOC:   srcIP=2.2.3.3, dstIP=1.1.1.2 */
    /* AUTODOC:   passenger srcIP=2.2.2.2, dstIP=1.2.1.3 */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        GT_BOOL isOk;
        GT_U32  expectedPacketSize;

        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* check Tx and Rx counters */
        switch (portIter)
        {
            case PRV_TGF_SEND_PORT_IDX_CNS:

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

                break;

            case PRV_TGF_NEXTHOPE_PORT_IDX_CNS:

                /* packetSize is changed because that will be new packet for IPV4 VLAN */
                expectedPacketSize = (packetSize + TGF_GRE_HEADER_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS + TGF_CRC_LEN_CNS);
                /* check if there is need for padding */
                if (expectedPacketSize < 64)
                    expectedPacketSize = 64;
                expectedCntrs.goodOctetsSent.l[0] = expectedPacketSize * prvTgfBurstCount;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = expectedPacketSize * prvTgfBurstCount;
                expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;

                break;

            default:

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

                break;
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
            PRV_UTF_LOG1_MAC(" goodOctetsSent = %d\n", expectedCntrs.goodOctetsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsSent = %d\n", expectedCntrs.goodPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsSent = %d\n", expectedCntrs.ucPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsSent = %d\n", expectedCntrs.brdcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsSent = %d\n", expectedCntrs.mcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodOctetsRcv = %d\n", expectedCntrs.goodOctetsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsRcv = %d\n", expectedCntrs.goodPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsRcv = %d\n", expectedCntrs.ucPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsRcv = %d\n", expectedCntrs.brdcPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsRcv = %d\n", expectedCntrs.mcPktsRcv.l[0]);
            PRV_UTF_LOG0_MAC("\n");
        }
    }


    /* -------------------------------------------------------------------------
     * 4. Get Trigger Counters
     */

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

    /* check if captured packet has the same MAC DA as prvTgfTunnelMacDa */
    cpssOsMemSet(&vfdArray[0], 0, sizeof(TGF_VFD_INFO_STC));
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].offset = 0;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfTunnelMacDa, sizeof(TGF_MAC_ADDR));

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    UTF_VERIFY_EQUAL6_STRING_MAC(1, numTriggers,
            "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
            prvTgfTunnelMacDa[0], prvTgfTunnelMacDa[1], prvTgfTunnelMacDa[2],
            prvTgfTunnelMacDa[3], prvTgfTunnelMacDa[4], prvTgfTunnelMacDa[5]);

    /* check if captured packet has right length field in IPv4 */
    cpssOsMemSet(&vfdArray[0], 0, sizeof(TGF_VFD_INFO_STC));
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].offset = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS
                       + TGF_ETHERTYPE_SIZE_CNS + 3;
    vfdArray[0].cycleCount = 1;
    vfdArray[0].patternPtr[0] = 0x46;

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    UTF_VERIFY_EQUAL1_STRING_MAC(1, numTriggers,
            "\n   Length field in IPv4 header must be: 0x%02x",
            vfdArray[0].patternPtr[0]);


    /* check if captured packet has right  protocol type */
    cpssOsMemSet(&vfdArray[0], 0, sizeof(TGF_VFD_INFO_STC));
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].offset = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS
                       + TGF_ETHERTYPE_SIZE_CNS + 9;
    vfdArray[0].cycleCount = 1;
    vfdArray[0].patternPtr[0] = 0x2f;

     rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
         rc = rc == GT_NO_MORE ? GT_OK : rc;

        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

     UTF_VERIFY_EQUAL1_STRING_MAC(1, numTriggers,
             "\n   IP protocol  field in IPv4 header must be: 0x%02x",
                    vfdArray[0].patternPtr[0]);




    /* check if captured packet has some checksum in IPv4 */
    cpssOsMemSet(&vfdArray[0], 0, sizeof(TGF_VFD_INFO_STC));
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_EXPECTED_NO_MATCH_E;
    vfdArray[0].offset = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS
                       + TGF_ETHERTYPE_SIZE_CNS + 10;
    vfdArray[0].cycleCount = 2;
    vfdArray[0].patternPtr[0] = 0x00;
    vfdArray[0].patternPtr[0] = 0x00;

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    UTF_VERIFY_EQUAL0_STRING_MAC(0, numTriggers,
            "\n   The checksum is not calculated in IPv4 header");

    /* -------------------------------------------------------------------------
     * 5. Get IP Counters
     */

    /* get and print ip counters values */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++) {
        PRV_UTF_LOG1_MAC("IP counters for Port [%d]:\n", prvTgfPortsArray[portIter]);
        prvTgfCountersIpGet(prvTgfDevNum, portIter, GT_TRUE, &ipCounters);
    }
    PRV_UTF_LOG0_MAC("\n");
}

/**
* @internal prvTgfTunnelStartIpv4overGreIpv4ConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*                                       None
*
* @note 3. Restore Tunnel Configuration
*       2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfTunnelStartIpv4overGreIpv4ConfigurationRestore
(
    GT_U32  prvUtfVrfId
)
{
    GT_U32      vlanIter  = 0;
    GT_U32      vlanCount = sizeof(prvTgfVlanArray);
    GT_STATUS   rc        = GT_OK;
    GT_IPADDR   ipAddr;
    PRV_TGF_IP_ROUTING_MODE_ENT     routingMode;

    /* AUTODOC: RESTORE CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 2. Restore Route Configuration
     */

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

    /* fill the destination IP address for Ipv4 prefix in Virtual Router */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipAddr.arIP));

    /* AUTODOC: delete the Ipv4 prefix */
    rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, prvUtfVrfId, ipAddr, 32);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    if (routingMode == PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E)
    {
        /* AUTODOC: disable Unicast IPv4 Routing on port 0 */
        rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                       CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    }

    /* AUTODOC: disable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* remove Virtual Router [prvUtfVrfId] */
    if (0 != prvUtfVrfId)
    {
        rc = prvTgfIpLpmVirtualRouterDel(prvTgfLpmDBId, prvUtfVrfId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "prvTgfIpLpmVirtualRouterDel: %d",
                                     prvTgfDevNum);
    }

    /* -------------------------------------------------------------------------
     * 1. Restore Base Configuration
     */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: invalidate VLANs 5,6 */
    for (vlanIter = 0; vlanIter < vlanCount; vlanIter++)
    {
        /* invalidate vlan entry */
        rc = prvTgfBrgVlanEntryInvalidate(prvTgfVlanArray[vlanIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d, %d",
                                     prvTgfDevNum, prvTgfVlanArray[vlanIter]);
    }
}


