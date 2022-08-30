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
* @file prvTgfIpv6UcEcmpRoutingIpCounters.c
*
* @brief IPV6 UC Routing with Ecmp Indirection and IP counters
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
#include <common/tgfLpmGen.h>
#include <common/tgfTrunkGen.h>
#include <ip/prvTgfIpv6UcEcmpRoutingIpCounters.h>
#include <cpss/common/cpssTypes.h>
#include <bridge/prvTgfVlanManipulation.h>
#include <gtOs/gtOsMem.h>
#include <extUtils/trafficEngine/private/prvTgfTrafficParser.h>

#define PRV_UTF_LOG_VERIFY_GT_OK(rc, name)                                   \
    if (GT_OK != (rc))                                                       \
    {                                                                        \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", name, rc);       \
    }

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           5

/* nextHop VLAN Id */
#define PRV_TGF_NEXTHOP_VLANID_CNS       6

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS            4

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         0

/* nextHop port number to receive traffic from */
#define PRV_TGF_NEXTHOP_PORT_IDX_CNS     3

/* the counter set for a route entry is linked to */
#define PRV_TGF_COUNT_SET_CNS     CPSS_IP_CNT_SET0_E

/* number of ip counters */
#define PRV_TGF_IP_COUNTER_COUNT_CNS 4

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = 1;

/* Route entry index for UC Route entry Table */
static GT_U32        prvTgfRouteEntryBaseIndex = 3;

/* LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 0;

/* default Vrf ID */
static GT_U32        prvTgfVrfId               = 0;

/* Number of paths for the Router Table */
static GT_U32        prvTgfNumOfPaths          = 4;

/* Base Indexes for the Route, ARP, SIP and ChkSum */
static GT_U32        prvTgfEcmpEntryBaseIndex  = 89;
static GT_U32        prvTgfArpEntryBaseIndex   = 0;

/* ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfArpMac = {0x00, 0x00, 0x00, 0x11, 0x22, 0x22};

/* Restored Values */
static PRV_TGF_TRUNK_LBH_GLOBAL_MODE_ENT globalHashModeGet;

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* saMac */
};
/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};
/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_86DD_IPV6_TAG_CNS};
/* packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketIpv6Part = {
    6,                  /* version */
    0,                  /* trafficClass */
    0,                  /* flowLabel */
    0x02,               /* payloadLen */
    0x3b,               /* nextHeader */
    0x40,               /* hopLimit */
    {0x6545, 0, 0, 0, 0, 0, 0x0000, 0x3200}, /* TGF_IPV6_ADDR srcAddr */
    {0x1111, 0, 0, 0, 0, 0, 0x0000, 0x1100}  /* TGF_IPV6_ADDR dstAddr */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05
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
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* parameters that is needed to be restored */
static struct
{
    GT_U32        nextHopIndexRestore[64];
} prvTgfRestoreCfg;

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfIpv6UcEcmpRoutingIpCountersBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfIpv6UcEcmpRoutingIpCountersBaseConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U8     tagArray[] = {1, 1, 1, 1};
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;

    /* AUTODOC: create VLANs 5 with tagged send ports */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS,
            prvTgfPortsArray, NULL, tagArray, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLANs 6 with tagged nexthop ports*/
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_NEXTHOP_VLANID_CNS,
            prvTgfPortsArray + 2, NULL, tagArray + 2, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* create a macEntry with .daRoute = GT_TRUE */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    macEntry.key.entryType                  = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId         = PRV_TGF_SEND_VLANID_CNS;
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum  = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
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
* @internal prvTgfIpv6UcEcmpRoutingIpCountersLttRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*
*/
GT_VOID prvTgfIpv6UcEcmpRoutingIpCountersLttRouteConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *regularEntryPtr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPV6ADDR                             ipAddr;
    GT_U32                                  ii;
    GT_U32                                  pathIndex;
    PRV_TGF_IP_ECMP_ENTRY_STC               ecmpEntry;
    GT_U32                                  nextHopIndexWrite;
    GT_U32                                  lastNextHopIndexWrite = prvTgfRouteEntryBaseIndex;

    PRV_UTF_LOG0_MAC("==== Setting LTT Route Configuration ====\n");

    /* -------------------------------------------------------------------------
     * 1. Enable Routing
     */

    /* AUTODOC: enable Unicast IPv6 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV6_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: enable IPv6 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV6_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: enable Trunk Hash Ip Mode */
    rc = prvTgfTrunkHashIpModeSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashIpModeSet: %d", prvTgfDevNum);

    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* Get the general hashing mode of trunk hash generation */
        rc =  prvTgfTrunkHashGlobalModeGet(prvTgfDevNum,&globalHashModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashGlobalModeGet: %d", prvTgfDevNum);

        /* Set the general hashing mode of trunk hash generation based on packet data */
        rc =  prvTgfTrunkHashGlobalModeSet(CPSS_DXCH_TRUNK_LBH_PACKETS_INFO_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashGlobalModeSet: %d", prvTgfDevNum);
    }

    /* -------------------------------------------------------------------------
     * 2. Create the Route entry (Next hop) in Route table and Router ARP Table
     */

    for (pathIndex = 0; pathIndex < prvTgfNumOfPaths; pathIndex++)
    {
        /* write a ARP MAC address to the Router ARP Table */
        prvTgfArpMac[5] = (GT_U8) (prvTgfArpEntryBaseIndex + pathIndex);

        cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

        /* AUTODOC: write ARP MACs to the Router ARP Table */
        rc = prvTgfIpRouterArpAddrWrite(prvTgfArpEntryBaseIndex + pathIndex, &arpMacAddr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

        /* setup default values for the route entries */
        cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));
        regularEntryPtr = &routeEntriesArray[0];
        regularEntryPtr->cmd                                = CPSS_PACKET_CMD_ROUTE_E;
        regularEntryPtr->nextHopVlanId                      = PRV_TGF_NEXTHOP_VLANID_CNS;
        regularEntryPtr->nextHopInterface.type              = CPSS_INTERFACE_PORT_E;
        regularEntryPtr->nextHopInterface.devPort.hwDevNum  = prvTgfDevNum;
        regularEntryPtr->nextHopInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_NEXTHOP_PORT_IDX_CNS];
        /* setup the route entries by incremental values */
        regularEntryPtr->countSet                           = (CPSS_IP_CNT_SET_ENT) pathIndex % 4;
        regularEntryPtr->nextHopARPPointer                  = prvTgfArpEntryBaseIndex + pathIndex;

        nextHopIndexWrite =  prvTgfRouteEntryBaseIndex + pathIndex;

        /*scatter the next hops*/
        if( (pathIndex%3) == 0)
        {
            nextHopIndexWrite = lastNextHopIndexWrite + 2;
        }
        else if( (pathIndex%3) == 1)
        {
            nextHopIndexWrite = lastNextHopIndexWrite + 3;
        }
        else
        {
            nextHopIndexWrite = lastNextHopIndexWrite + 4;
        }

        lastNextHopIndexWrite =  nextHopIndexWrite;

        /* AUTODOC: save ECMP indirect next hop entry */
        rc = cpssDxChIpEcmpIndirectNextHopEntryGet(prvTgfDevNum, prvTgfRouteEntryBaseIndex + pathIndex,
                                                   &prvTgfRestoreCfg.nextHopIndexRestore[pathIndex]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpEcmpIndirectNextHopEntryGet: %d", prvTgfDevNum);

        rc = cpssDxChIpEcmpIndirectNextHopEntrySet(prvTgfDevNum, prvTgfRouteEntryBaseIndex + pathIndex,
                                                   nextHopIndexWrite);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpEcmpIndirectNextHopEntrySet: %d", prvTgfDevNum);

        PRV_UTF_LOG2_MAC("==== Indirect ecmp indirect  entry [%d] point to [%d] ====\n",
                         prvTgfRouteEntryBaseIndex + pathIndex, nextHopIndexWrite);

        /* AUTODOC: add UC route entries with nexthop VLAN 6 and nexthop port 3 */
        rc = prvTgfIpUcRouteEntriesWrite(nextHopIndexWrite, routeEntriesArray, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);
    }

     /* Add an ECMP entry */
    ecmpEntry.numOfPaths = prvTgfNumOfPaths;
    ecmpEntry.randomEnable = GT_FALSE;
    ecmpEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    ecmpEntry.multiPathMode = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    rc = prvTgfIpEcmpEntryWrite(prvTgfDevNum, prvTgfEcmpEntryBaseIndex, &ecmpEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpEcmpEntryWrite: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 3. Create a new Ipv6 prefix in a Virtual Router for the specified LPM DB
     */


    /* AUTODOC: for regular test add IPv6 UC prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));

    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_MULTIPATH_ROUTE_ENTRY_GROUP_E;
    }
    else
    {
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    }
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfEcmpEntryBaseIndex;

    /* fill a destination IPv6 address for the prefix */
    for (ii = 0; ii < 8; ii++)
    {
        ipAddr.arIP[ii * 2]     = (GT_U8)(prvTgfPacketIpv6Part.dstAddr[ii] >> 8);
        ipAddr.arIP[ii * 2 + 1] = (GT_U8) prvTgfPacketIpv6Part.dstAddr[ii];
    }
    rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, prvTgfVrfId, ipAddr,
                                    128, &nextHopInfo, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixAdd: %d", prvTgfDevNum);
}

/**
* @internal prvTgfIpv6UcEcmpRoutingIpCountersTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
*/
GT_VOID prvTgfIpv6UcEcmpRoutingIpCountersTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[1];
    CPSS_INTERFACE_INFO_STC         portInterface;
    PRV_TGF_IP_COUNTER_SET_STC      ipCounters;
    GT_U32                          pathIndex;
    GT_U32                          counterIter = 0;
    GT_U32                          portIter = 0;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    TGF_PACKET_STC                  packetInfo;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic for ECMP =======\n");

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* enable capture on nexthope port */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOP_PORT_IDX_CNS];

    /* reset IP couters and set ROUTE_ENTRY mode */
    for (portIter = 0; portIter < 4; portIter++)
    {
        for (counterIter = 0; counterIter < PRV_TGF_IP_COUNTER_COUNT_CNS; counterIter++)
        {
            rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], counterIter);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: dev=%d, port=%d, counter=%d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter], counterIter);
        }
    }

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

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

    /* set transmit timings */
    rc = prvTgfSetTxSetup2Eth(0, 10);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

    for (pathIndex = 0; pathIndex < prvTgfNumOfPaths; pathIndex++)
    {
        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

        /* enable capture before sending the traffic */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, portInterface.devPort.portNum);

        /* modify the packet */
        prvTgfArpMac[5] = (GT_U8) (prvTgfArpEntryBaseIndex + pathIndex);
        prvTgfPacketIpv6Part.srcAddr[7] = (GT_U16)((0x32 << 8) | (16*pathIndex));

        /* send packet */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

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

            if(portIter == PRV_TGF_SEND_PORT_IDX_CNS)
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
             }
             else if(portIter == PRV_TGF_NEXTHOP_PORT_IDX_CNS)
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

        /* get trigger counters where packet has MAC DA as prvTgfArpMac */
        PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);
        vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[0].modeExtraInfo = 0;
        vfdArray[0].offset = 0;
        vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);

        cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

        /* disable capture on nexthope port - before check packets to the CPU */
        /* NOTE: this is done inside the loop , because:
           the function tgfTrafficGeneratorPortTxEthCaptureSet is general responsible
           to do needed 'sleep' to adjust the time that needed to allow the AppDemo
           to process all the packets sent to the CPU.
        */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum, portInterface.devPort.portNum);

        /* check TriggerCounters */
        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
        rc = rc == GT_NO_MORE ? GT_OK : rc;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "tgfTrafficGeneratorPortTxEthTriggerCountersGet:"
                                     " port = %d, rc = 0x%02X\n", portInterface.devPort.portNum, rc);

        UTF_VERIFY_EQUAL6_STRING_MAC(1, numTriggers,
                "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X\n",
                prvTgfArpMac[0], prvTgfArpMac[1], prvTgfArpMac[2],
                prvTgfArpMac[3], prvTgfArpMac[4], prvTgfArpMac[5]);
    }

    /* AUTODOC: get and check ip counters values */
    for (counterIter = 0; counterIter < PRV_TGF_IP_COUNTER_COUNT_CNS; counterIter++) {
        prvTgfCountersIpGet(prvTgfDevNum, counterIter, GT_FALSE, &ipCounters);
        UTF_VERIFY_EQUAL1_STRING_MAC(1, ipCounters.inUcPkts,
                                     "In Uc Packet mismatch for IP Counter: %d", counterIter);
        UTF_VERIFY_EQUAL1_STRING_MAC(1, ipCounters.outUcRoutedPkts,
                                     "Out UC Routed packets mismatch for IP Counter: %d", counterIter);
    }

    /* restore transmit timings */
    rc = prvTgfSetTxSetup2Eth(0, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);
}

/**
* @internal prvTgfIpv6UcEcmpRoutingIpCountersConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfIpv6UcEcmpRoutingIpCountersConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_IPV6ADDR ipAddr = {{0}};
    GT_U32      ii;
    GT_U32      portIter  = 0;
    PRV_TGF_IP_ECMP_ENTRY_STC ecmpEntry;
    GT_U8       pathIndex;

    /* AUTODOC: RESTORE CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 2. Restore fdb table and clear packet counters
     */

    /* delete the FDB entry */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", prvTgfDevNum);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* -------------------------------------------------------------------------
     * 2. Restore Route Configuration
     */

    /* fill the destination IP address for IPv6 prefix in Virtual Router */
    for (ii = 0; ii < 8; ii++)
    {
        ipAddr.arIP[ii * 2]     = (GT_U8)(prvTgfPacketIpv6Part.dstAddr[ii] >> 8);
        ipAddr.arIP[ii * 2 + 1] = (GT_U8) prvTgfPacketIpv6Part.dstAddr[ii];
    }

    /* AUTODOC: delete the IPv6 prefix */
    rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, prvTgfVrfId, ipAddr, 128);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    /* restore ECMM indirect not hop entries */
    for (pathIndex = 0; pathIndex < prvTgfNumOfPaths; pathIndex++)
    {
        rc = cpssDxChIpEcmpIndirectNextHopEntrySet(prvTgfDevNum,prvTgfRouteEntryBaseIndex + pathIndex,
                                                   prvTgfRestoreCfg.nextHopIndexRestore[pathIndex]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChIpEcmpIndirectNextHopEntrySet: %d", prvTgfDevNum);
    }

    /* restore ecmp entry */
    ecmpEntry.numOfPaths = 1;
    ecmpEntry.randomEnable = GT_FALSE;
    ecmpEntry.routeEntryBaseIndex = 0;
    ecmpEntry.multiPathMode = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    rc = prvTgfIpEcmpEntryWrite(prvTgfDevNum, prvTgfEcmpEntryBaseIndex, &ecmpEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpEcmpEntryWrite: %d", prvTgfDevNum);

    /* AUTODOC: disable Trunk Hash Ip Mode */
    rc = prvTgfTrunkHashIpModeSet(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashIpModeSet: %d", prvTgfDevNum);

    /* disable IPv6 Unicast routing on ingress port */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV6_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: disable IPv6 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E,
            CPSS_IP_PROTOCOL_IPV6_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* delete Ports 0,1 from VLAN 5 */
    for (portIter = 0; portIter < (PRV_TGF_PORT_COUNT_CNS/2); portIter++)
    {
        /* delete port from VLAN */
        rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS,
                                       prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: %d, %d, %d",
                                     prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS,
                                     prvTgfPortsArray[portIter]);
    }

    /* delete Ports 2,3 from VLAN 6 */
    for (portIter = 2; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* delete port from VLAN */
        rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_NEXTHOP_VLANID_CNS,
                                       prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: %d, %d, %d",
                                     prvTgfDevNum, PRV_TGF_NEXTHOP_VLANID_CNS,
                                     prvTgfPortsArray[portIter]);
    }

    /* invalidate SEND VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate");

    /* invalidate NEXTHOP VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_NEXTHOP_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate");

    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* AUTODOC: restore to enhanced crc hash data */
        /* restore global hash mode */
        rc =  prvTgfTrunkHashGlobalModeSet(globalHashModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashGlobalModeSet: %d", prvTgfDevNum);
    }
}
