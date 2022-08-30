/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfFdbBasicIpv6UcRouting.c
*
* DESCRIPTION:
*       FDB IPv6 UC Routing
*
* FILE REVISION NUMBER:
*       $Revision: 7 $
*
*******************************************************************************/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTrunkGen.h>
#include <common/tgfIpGen.h>

#include <bridge/prvTgfFdbBasicIpv6UcRouting.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           5

/* nextHop VLAN Id */
#define PRV_TGF_NEXTHOPE_VLANID_CNS       6

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS            4

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         0

/* nextHop port number to receive traffic from */
#define PRV_TGF_NEXTHOPE_2_PORT_IDX_CNS   3

/* MAC2ME entry index */
#define PRV_TGF_MAC_TO_ME_INDEX_CNS       1

/* the counter set for a route entry is linked to */
#define PRV_TGF_COUNT_SET_CNS     CPSS_IP_CNT_SET0_E

static CPSS_PACKET_CMD_ENT              prvTgfNhPacketCmdGet;
static GT_ETHERADDR                     prvTgfArpMacAddrGet;

/* the Arp Address index of the Router ARP Table
   to write to the UC Route entry Arp nextHopARPPointer field */
static GT_U32        prvTgfRouterArpIndex      = 1;

/* the Virtual Router index */
static GT_U32        prvUtfVrfId  = 1;
static GT_U32        lpmDbId      = 0;


/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = 1;

/* keep address index and data index for future use*/
static GT_U32       prvTgfIpv6UcFdbEntryAddressIndex;
static GT_U32       prvTgfIpv6UcFdbEntryDataIndex;

static GT_U32       prefixLenGet;

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
    {0x6545, 0, 0, 0, 0, 0, 0x0000, 0x3212}, /* TGF_IPV6_ADDR srcAddr */
    {0x1122, 0, 0, 0, 0, 0, 0xccdd, 0xeeff}  /* TGF_IPV6_ADDR dstAddr */
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

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfArpMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x22};

/* FDB entry IPv6 destination address */
static TGF_IPV6_ADDR       dstAddr = {0x1122, 0, 0, 0, 0, 0, 0xccdd, 0};

/**
* @internal prvTgfFdbIpv6UcRoutingLookupMaskRouteConfigurationSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*/
GT_VOID prvTgfFdbIpv6UcRoutingLookupMaskRouteConfigurationSet
(
    GT_VOID
)
{
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_MAC_ENTRY_KEY_STC               entryKey;
    PRV_TGF_BRG_MAC_ENTRY_STC               macEntry;
    GT_U32                                  numOfBanks = 16;
    GT_U32                                  nextHopDataBankNumber;
    GT_U32                                  fdbEntryIndex;
    GT_STATUS rc = GT_OK;
    GT_U32    offset = 0;

    /* AUTODOC: write a ARP MAC 00:00:00:00:00:22 to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    /* Create the Route entry (Next hop) in FDB table and Router ARP Table */
    rc = prvTgfIpRouterArpAddrRead(prvTgfDevNum,prvTgfRouterArpIndex,&prvTgfArpMacAddrGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrRead: %d", prvTgfDevNum);

    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* AUTODOC: get Packet Commands for FDB routed packets */
    rc = prvTgfBrgFdbRoutingNextHopPacketCmdGet( prvTgfDevNum,&prvTgfNhPacketCmdGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingNextHopPacketCmdGet");

    /* AUTODOC: set Packet Commands for FDB routed packets */
    rc = prvTgfBrgFdbRoutingNextHopPacketCmdSet( prvTgfDevNum,CPSS_PACKET_CMD_ROUTE_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingNextHopPacketCmdSet");

    /* AUTODOC: update VRF Id in vlan[PRV_TGF_SEND_VLANID_CNS] */
    rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_SEND_VLANID_CNS, prvUtfVrfId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanVrfIdSet: %d %d",
                             prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    cpssOsMemSet(&entryKey, 0, sizeof(entryKey));
    /* fill a destination IP address for the prefix */

    for (offset = 0; offset < 8; offset++)
    {
        entryKey.key.ipv6Unicast.dip[offset << 1]       = (GT_U8)(dstAddr[offset] >> 8);
        entryKey.key.ipv6Unicast.dip[(offset << 1) + 1] = (GT_U8) dstAddr[offset];
    }

    /* set key virtual router */
    entryKey.key.ipv6Unicast.vrfId = prvUtfVrfId;

    /* AUTODOC: calculate index for ipv6 uc route data entry in FDB */

    /* set entry type */
    entryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E;
    rc =  prvTgfBrgFdbMacEntryIndexFind(&entryKey,&fdbEntryIndex); /* data */
    if (rc == GT_NOT_FOUND)
    {
        rc = GT_OK;
    }
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryIndexFind: %d", fdbEntryIndex);

    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum) == GT_FALSE)
    {
        prvTgfIpv6UcFdbEntryDataIndex = fdbEntryIndex;

        nextHopDataBankNumber = fdbEntryIndex % numOfBanks;
    }
    else
    {
        /* On SIP-6, IPV6 UC Address entry must be in even bank, and Data entry immediately follows */
        UTF_VERIFY_EQUAL1_STRING_MAC(0, fdbEntryIndex % 2, "prvTgfBrgFdbMacEntryIndexFind: %d", fdbEntryIndex);
        prvTgfIpv6UcFdbEntryDataIndex = fdbEntryIndex + 1;

        nextHopDataBankNumber = 0;
    }

    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(&macEntry.key.key.ipv6Unicast, &entryKey.key.ipv6Unicast,sizeof(macEntry.key.key.ipv6Unicast));
    macEntry.key.entryType = PRV_TGF_FDB_ENTRY_TYPE_IPV6_UC_DATA_ENTRY_E;
    macEntry.fdbRoutingInfo.ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
    macEntry.fdbRoutingInfo.ttlHopLimitDecEnable = GT_FALSE;
    macEntry.fdbRoutingInfo.ingressMirror = GT_FALSE;
    macEntry.fdbRoutingInfo.ingressMirrorToAnalyzerIndex = 0;
    macEntry.fdbRoutingInfo.qosProfileMarkingEnable = GT_FALSE;
    macEntry.fdbRoutingInfo.qosProfileIndex = 0;
    macEntry.fdbRoutingInfo.qosPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    macEntry.fdbRoutingInfo.modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    macEntry.fdbRoutingInfo.modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    macEntry.fdbRoutingInfo.countSet = PRV_TGF_COUNT_SET_CNS;
    macEntry.fdbRoutingInfo.trapMirrorArpBcEnable = GT_FALSE;
    macEntry.fdbRoutingInfo.dipAccessLevel = 0;
    macEntry.fdbRoutingInfo.ICMPRedirectEnable = GT_FALSE;
    macEntry.fdbRoutingInfo.mtuProfileIndex = 0;
    macEntry.fdbRoutingInfo.isTunnelStart = GT_FALSE;
    macEntry.fdbRoutingInfo.nextHopVlanId = PRV_TGF_NEXTHOPE_VLANID_CNS;
    macEntry.fdbRoutingInfo.nextHopTunnelPointer = 0;
    macEntry.fdbRoutingInfo.nextHopARPPointer = prvTgfRouterArpIndex;
    macEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_2_PORT_IDX_CNS];
    macEntry.fdbRoutingInfo.nextHopDataBankNumber = 0;
    macEntry.fdbRoutingInfo.scopeCheckingEnable = GT_FALSE;
    macEntry.fdbRoutingInfo.siteId = CPSS_IP_SITE_ID_INTERNAL_E;

    /* AUTODOC: set  ipv6 uc route data entry in FDB*/
    prvTgfFdbIpv6UcRoutingRouteByIndexConfigurationSet(&macEntry, prvTgfIpv6UcFdbEntryDataIndex);


    /* AUTODOC: calculate index for ipv6 uc route address entry in FDB */

    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum) == GT_FALSE)
    {
        /* set entry type */
        entryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E;
        rc =  prvTgfBrgFdbMacEntryIndexFind(&entryKey,&fdbEntryIndex); /* address */
        if (rc == GT_NOT_FOUND)
        {
            rc = GT_OK;
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryIndexFind: %d", fdbEntryIndex);
    }

    prvTgfIpv6UcFdbEntryAddressIndex = fdbEntryIndex;

    macEntry.key.entryType = PRV_TGF_FDB_ENTRY_TYPE_IPV6_UC_ADDR_ENTRY_E;
    macEntry.fdbRoutingInfo.nextHopDataBankNumber = nextHopDataBankNumber;

    /* AUTODOC: set  ipv6 uc route address entry in FDB*/
    prvTgfFdbIpv6UcRoutingRouteByIndexConfigurationSet(&macEntry, fdbEntryIndex);

    /* AUTODOC: Save prefix length value */
    rc = prvTgfIpFdbRoutePrefixLenGet(prvTgfDevNum, CPSS_IP_PROTOCOL_IPV6_E, &prefixLenGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpFdbRoutePrefixLenGet");
}

/**
* @internal prvTgfFdbIpv6UcRoutingLookupMaskRouteConfigurationSet function
* @endinternal
*
* @brief   Set FDB lookup prefix length
*
* @param[in] prefixLen - FDB lookup prefix length
*
*/
GT_VOID prvTgfFdbIpv6UcRoutingLookupMaskPrefixLengthSet
(
    GT_U32 prefixLen
)
{
    GT_STATUS rc;
    rc = prvTgfIpFdbRoutePrefixLenSet(prvTgfDevNum, CPSS_IP_PROTOCOL_IPV6_E, prefixLen);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpFdbRoutePrefixLenSet");
}

/**
* @internal prvTgfFdbBasicIpv6UcRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] dstIpAddr                - packet's destination IP address
* @param[in] expectTraffic            - whether to expect traffic or not
*                                       None
*/
static GT_VOID prvTgfFdbIpv6UcRoutingLookupMaskTrafficGenerate
(
    TGF_IPV6_ADDR   dstIpAddr,
    GT_BOOL         expectTraffic
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    GT_U32                          sizeDSATag = 0;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          portIter    = 0;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[5];
    GT_U8                           vfdNum = 1;
    GT_U32                          expectedTrigers = 1;
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    PRV_TGF_IP_COUNTER_SET_STC      ipCounters;
    PRV_TGF_IP_ROUTING_MODE_ENT     routingMode;


    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* set IPv6 packet */
    cpssOsMemCpy(&prvTgfPacketIpv6Part.dstAddr[0], &dstIpAddr[0], sizeof(TGF_IPV6_ADDR));

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
            /* AUTODOC: reset IP couters and set ROUTE_ENTRY mode for all ports */
            rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }
    }

    /* setup nexthope portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOPE_2_PORT_IDX_CNS];

    /* enable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_NEXTHOPE_2_PORT_IDX_CNS]);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");


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


    /* AUTODOC: send IPv6 packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:01, VID=5 */
    /* AUTODOC:   srcIP=6545:0000:0000:0000:0000:0000:0000:3212 */
    /* AUTODOC:   dstIP=1122:0000:0000:0000:0000:0000:ccdd:eeff for regular test */
    /* AUTODOC:   dstIP=1122:0000:0000:0000:0000:0000:ccdd:eeff or 2233:0000:0000:0000:0000:0000:5678:90ab for bulk test */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    cpssOsTimerWkAfter(200);

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

        /* AUTODOC: verify routed packet on port 3 with: */
        /* AUTODOC:   DA=00:00:00:00:00:22, SA=00:00:00:00:00:06 */
        switch (portIter) {
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

            case PRV_TGF_NEXTHOPE_2_PORT_IDX_CNS:
                if(expectTraffic==GT_TRUE)
                {
                    /* packetSize is not changed */
                    expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS + sizeDSATag) * prvTgfBurstCount;
                    expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                    expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                    expectedCntrs.brdcPktsSent.l[0]   = 0;
                    expectedCntrs.mcPktsSent.l[0]     = 0;
                    expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS + sizeDSATag) * prvTgfBurstCount;
                    expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                    expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                    expectedCntrs.brdcPktsRcv.l[0]    = 0;
                    expectedCntrs.mcPktsRcv.l[0]      = 0;
                }
                else
                {
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

    if(expectTraffic==GT_TRUE)
    {

        PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

        /* get trigger counters where packet has MAC DA as prvTgfArpMac */
        vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[0].modeExtraInfo = 0;
        vfdArray[0].offset = 0;
        vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
        cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, vfdNum, vfdArray, &numTriggers);
        PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

        /* check TriggerCounters */
        rc = rc == GT_NO_MORE ? GT_OK : rc;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                     portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

        /* check if captured packet has the same MAC DA as prvTgfArpMac */
        UTF_VERIFY_EQUAL6_STRING_MAC(expectedTrigers, numTriggers,
                "\n   MAC DA of captured packet must be: %02X:%02X:%02X:%02X:%02X:%02X",
                prvTgfArpMac[0], prvTgfArpMac[1], prvTgfArpMac[2],
                prvTgfArpMac[3], prvTgfArpMac[4], prvTgfArpMac[5]);

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

    /* disable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_NEXTHOPE_2_PORT_IDX_CNS]);

}

/**
* @internal prvTgfFdbIpv6UcRoutingLookupMaskConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbIpv6UcRoutingLookupMaskConfigRestore
(
    GT_VOID
)
{
    GT_STATUS       rc          = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("\n========== Restore config: ==================\n\n");

    /* AUTODOC: restore a ARP MAC  to the Router ARP Table */
    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &prvTgfArpMacAddrGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* AUTODOC: restore Packet Commands for FDB routed packets */
    rc = prvTgfBrgFdbRoutingNextHopPacketCmdSet( prvTgfDevNum,prvTgfNhPacketCmdGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingNextHopPacketCmdSet");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    if (0 != prvUtfVrfId)
    {
        /* Remove Virtual Router [prvUtfVrfId] with created default LTT entries */
        rc = prvTgfIpLpmVirtualRouterDel(lpmDbId, prvUtfVrfId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmVirtualRouterDel: %d", prvTgfDevNum);
    }

    /* AUTODOC: Restore prefix length value */
    rc = prvTgfIpFdbRoutePrefixLenSet(prvTgfDevNum, CPSS_IP_PROTOCOL_IPV6_E, prefixLenGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpFdbRoutePrefixLenSet");
}

/**
* @internal prvTgfFdbIpv6UcRoutingLookupMaskTest function
* @endinternal
*
* @brief   Test FDB IPv6 Lookup Mask
*/
GT_VOID prvTgfFdbIpv6UcRoutingLookupMaskTest
(
    GT_VOID
)
{
    GT_U32 prefixLen;
    TGF_IPV6_ADDR   dstIpAddr = {0x1122, 0, 0, 0, 0, 0, 0xccdd, 0x1};

    for(prefixLen = 112; prefixLen <= 128; prefixLen++)
    {
        /* Set FDB lookup prefix length */
        prvTgfFdbIpv6UcRoutingLookupMaskPrefixLengthSet(prefixLen);
        if (prefixLen == 128)
        {
            /* Generate traffic */
            prvTgfFdbIpv6UcRoutingLookupMaskTrafficGenerate(dstIpAddr, GT_FALSE);
        }
        else
        {
            /* Generate traffic */
            prvTgfFdbIpv6UcRoutingLookupMaskTrafficGenerate(dstIpAddr, GT_TRUE);
        }
    }

    dstIpAddr[7] = 0x8000;

    for(prefixLen = 112; prefixLen <= 128; prefixLen++)
    {
        /* Set FDB lookup prefix length */
        prvTgfFdbIpv6UcRoutingLookupMaskPrefixLengthSet(prefixLen);
        if (prefixLen != 112)
        {
            /* Generate traffic */
            prvTgfFdbIpv6UcRoutingLookupMaskTrafficGenerate(dstIpAddr,GT_FALSE);
        }
        else
        {
            /* Generate traffic */
            prvTgfFdbIpv6UcRoutingLookupMaskTrafficGenerate(dstIpAddr,GT_TRUE);
        }
    }
}
