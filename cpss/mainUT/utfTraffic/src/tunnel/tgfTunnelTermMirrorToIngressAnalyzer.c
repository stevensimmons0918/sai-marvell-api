/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *2
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file tgfTunnelTermMirrorToIngressAnalyzer.c
*
* @brief Tunnel Term: Ingress Mirror To Analyzer Index
*
* @version   9
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfTunnelGen.h>
#include <tunnel/tgfTunnelTermMirrorToIngressAnalyzer.h>

#include <mirror/prvTgfMirror.h>
#include <common/tgfBridgeGen.h>


/******************************* Test packet **********************************/

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS 5
#define PRV_TGF_EGR_VLANID_CNS  6

/* port to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS 0

/* egress port number to receive traffic from  */
#define PRV_TGF_EGR_PORT_IDX_CNS 3

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS 4

/* default tunnel term entry index */
#define PRV_TGF_TTI0_INDEX_CNS prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(0, 3)
#define PRV_TGF_TTI1_INDEX_CNS prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(1, 0)

/* default number of packets to send */
static GT_U32 prvTgfBurstCount = 1;

/* analyzer index */
static GT_U32 analyzerIndex = 0;

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC interface;
    GT_U32                                analyzerIndex;
    GT_BOOL                               prvTgfEnableMirror;
} prvTgfRestoreCfg;

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x44, 0x33, 0x22, 0x11},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x33}                /* srcMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x55, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x55
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo = {
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                             /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

static PRV_TGF_TTI_ACTION_2_STC ttiAction2 = {

 GT_FALSE, /* GT_BOOL tunnelTerminate; */
 0,        /* PRV_TGF_TTI_PASSENGER_TYPE_ENT ttPassengerPacketType; */
 0,        /* PRV_TGF_TUNNEL_PASSANGER_TYPE_ENT tsPassengerPacketType; */
 GT_FALSE, /* GT_BOOL copyTtlExpFromTunnelHeader; */
 0,        /* PRV_TGF_TTI_MPLS_COMMAND_ENT mplsCommand; */
 0,        /* GT_U32 mplsTtl; */
 0,        /* GT_U32 exp; */
 GT_FALSE, /* GT_BOOL setExp; */
 0,        /* GT_U32 mplsLabel; */
 0,        /* GT_U32 ttl; */
 GT_FALSE, /* GT_BOOL enableDecrementTtl; */
 0,        /* CPSS_PACKET_CMD_ENT command; */
 0,        /* PRV_TGF_TTI_REDIRECT_COMMAND_ENT redirectCommand; */
 {0, /* CPSS_INTERFACE_TYPE_ENT type; */
    {0, /* GT_HW_DEV_NUM   hwDevNum; */
     0, /* GT_PORT_NUM     portNum; */
    }, /* devPort; */
  0, /* GT_TRUNK_ID     trunkId; */
  0, /* GT_U16          vidx; */
  0, /* GT_U16          vlanId; */
  0, /* GT_HW_DEV_NUM   hwDevNum; */
  0, /* GT_U16          fabricVidx; */
  0, /* GT_U32          index; */
 },         /* CPSS_INTERFACE_INFO_STC egressInterface; */
 0,        /* GT_U32 ipNextHopIndex; */
 0,        /* GT_U32 arpPtr; */
 GT_FALSE, /* GT_BOOL tunnelStart; */
 0,        /* GT_U32 tunnelStartPtr; */
 0,        /* GT_U32 routerLttPtr; */
 0,        /* GT_U32 vrfId; */
 GT_FALSE, /* GT_BOOL sourceIdSetEnable; */
 0,        /* GT_U32 sourceId; */
 0,        /* PRV_TGF_TTI_VLAN_COMMAND_ENT tag0VlanCmd; */
 0,        /* GT_U16 tag0VlanId; */
 0,        /* PRV_TGF_TTI_VLAN1_COMMAND_ENT tag1VlanCmd; */
 0,        /* GT_U16 tag1VlanId; */
 0,        /* CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT tag0VlanPrecedence; */
 GT_FALSE, /* GT_BOOL nestedVlanEnable; */
 GT_FALSE, /* GT_BOOL bindToPolicerMeter; */
 GT_FALSE, /* GT_BOOL bindToPolicer; */
 0,        /* GT_U32 policerIndex; */
 0,        /* CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT qosPrecedence; */
 GT_FALSE, /* GT_BOOL keepPreviousQoS; */
 GT_FALSE, /* GT_BOOL trustUp; */
 GT_FALSE, /* GT_BOOL trustDscp; */
 GT_FALSE, /* GT_BOOL trustExp; */
 0,        /* GT_U32 qosProfile; */
 0,        /* PRV_TGF_TTI_MODIFY_UP_ENT modifyTag0Up; */
 0,        /* PRV_TGF_TTI_TAG1_UP_COMMAND_ENT tag1UpCommand; */
 0,        /* PRV_TGF_TTI_MODIFY_DSCP_ENT modifyDscp; */
 0,        /* GT_U32 tag0Up; */
 0,        /* GT_U32 tag1Up; */
 GT_FALSE, /* GT_BOOL remapDSCP; */
 0,        /* PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_ENT pcl0OverrideConfigIndex; */
 0,        /* PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_ENT pcl0_1OverrideConfigIndex; */
 0,        /* PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_ENT pcl1OverrideConfigIndex; */
 0,        /* GT_U32 iPclConfigIndex; */
 GT_FALSE, /* GT_BOOL mirrorToIngressAnalyzerEnable; */
 0,        /* CPSS_NET_RX_CPU_CODE_ENT userDefinedCpuCode; */
 GT_FALSE, /* GT_BOOL bindToCentralCounter; */
 0,        /* GT_U32 centralCounterIndex; */
 GT_FALSE, /* GT_BOOL vntl2Echo; */
 GT_FALSE, /* GT_BOOL bridgeBypass; */
 GT_FALSE, /* GT_BOOL ingressPipeBypass; */
 GT_FALSE, /* GT_BOOL actionStop; */
 0,        /* GT_U32 hashMaskIndex; */
 GT_FALSE, /* GT_BOOL modifyMacSa; */
 GT_FALSE, /* GT_BOOL modifyMacDa; */
 GT_FALSE, /* GT_BOOL ResetSrcPortGroupId; */
 GT_FALSE, /* GT_BOOL multiPortGroupTtiEnable; */
 GT_FALSE, /* GT_BOOL sourceEPortAssignmentEnable; */
 0,        /* GT_PORT_NUM sourceEPort; */
 0,        /* GT_U32 pwId; */
 GT_FALSE, /* GT_BOOL sourceIsPE; */
 GT_FALSE, /* GT_BOOL enableSourceLocalFiltering; */
 0,        /* GT_U32 floodDitPointer; */
 0,        /* GT_U32 baseMplsLabel; */
 0,        /* PRV_TGF_TTI_PW_ID_MODE_ENT pwIdMode; */
 GT_FALSE, /* GT_BOOL counterEnable; */
 GT_FALSE, /* GT_BOOL meterEnable; */
 0,        /* GT_U32 flowId; */
 GT_FALSE, /* GT_BOOL ipfixEn; */
 GT_FALSE, /* GT_BOOL sstIdEnable; */
 0,        /* GT_U32 sstId; */
 0,        /* GT_U32 ttHeaderLength; */
 0,        /* PRV_TGF_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_TYPE_ENT passengerParsingOfTransitMplsTunnelMode; */
 GT_FALSE, /* GT_BOOL passengerParsingOfTransitNonMplsTransitTunnelEnable; */
 GT_FALSE, /* GT_BOOL cwBasedPw; */
 GT_FALSE, /* GT_BOOL applyNonDataCwCommand; */
 GT_FALSE, /* GT_BOOL continueToNextTtiLookup; */
 0,        /* GT_U32 mirrorToIngressAnalyzerIndex; */
 GT_FALSE, /* GT_BOOL rxProtectionSwitchEnable; */
 GT_FALSE, /* GT_BOOL rxIsProtectionPath; */
 GT_FALSE, /* GT_BOOL setMacToMe; */
 GT_FALSE, /* GT_BOOL qosUseUpAsIndexEnable; */
 0,        /* GT_U32 qosMappingTableIndex; */
 GT_FALSE, /* GT_BOOL iPclUdbConfigTableEnable; */
 0,        /* PRV_TGF_PCL_PACKET_TYPE_ENT iPclUdbConfigTableIndex; */
 GT_FALSE, /* GT_BOOL ttlExpiryVccvEnable; */
 GT_FALSE, /* GT_BOOL pwe3FlowLabelExist; */
 GT_FALSE, /* GT_BOOL pwCwBasedETreeEnable; */
 GT_FALSE, /* GT_BOOL oamProcessEnable; */
 0,        /* GT_U32 oamProfile; */
 GT_FALSE, /* GT_BOOL oamChannelTypeToOpcodeMappingEnable; */
 GT_FALSE, /* GT_BOOL unknownSaCommandEnable; */
 0,        /* CPSS_PACKET_CMD_ENT unknownSaCommand; */
 GT_FALSE, /* GT_BOOL sourceMeshIdSetEnable; */
 0,        /* GT_U32 sourceMeshId; */
 0,        /* CPSS_DXCH_TUNNEL_HEADER_LENGTH_ANCHOR_TYPE_ENT tunnelHeaderLengthAnchorType; */
 GT_FALSE, /* GT_BOOL skipFdbSaLookupEnable; */
 GT_FALSE, /* GT_BOOL ipv6SegmentRoutingEndNodeEnable; */
 GT_FALSE, /* GT_BOOL exactMatchOverTtiEn; */
 GT_FALSE, /* GT_BOOL copyReservedAssignmentEnable */
 0,        /* copyReserved */
 GT_FALSE, /* GT_BOOL triggerHashCncClient */
};
static PRV_TGF_TTI_RULE_UNT     ttiPattern;
static PRV_TGF_TTI_RULE_UNT     ttiMask;


/******************************************************************************\
 *                        Private type definition                             *
\******************************************************************************/


/**
* @internal tgfTunnelTermMirrorToIngressAnalyzerBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*/
GT_VOID tgfTunnelTermMirrorToIngressAnalyzerBridgeConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* AUTODOC: create VLAN 5 with untagged ports [0,1] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS, prvTgfPortsArray, NULL, NULL, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with untagged ports [2,3] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_EGR_VLANID_CNS, prvTgfPortsArray + 2,
                                           NULL, NULL, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);
}


/**
* @internal tgfTunnelTermMirrorToIngressAnalyzerTtiConfigurationSet function
* @endinternal
*
* @brief   Set TIT configuration
*/
GT_VOID tgfTunnelTermMirrorToIngressAnalyzerTtiConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                rc         = GT_OK;

        /* AUTODOC: SETUP TTI CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* clear entry */
    cpssOsMemSet((GT_VOID*) &ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet((GT_VOID*) &ttiMask,    0, sizeof(ttiMask));

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* set the TTI Rule Pattern, Mask and Action for Ether Type Key */

    ttiAction2.redirectCommand                   = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ttiAction2.egressInterface.type              = CPSS_INTERFACE_PORT_E;
    ttiAction2.egressInterface.devPort.hwDevNum    = prvTgfDevNum;
    ttiAction2.egressInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    ttiAction2.tag1VlanCmd            = PRV_TGF_TTI_VLAN_MODIFY_UNTAGGED_E;
    ttiAction2.tag0VlanCmd            = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
    ttiAction2.tag0VlanId             = PRV_TGF_EGR_VLANID_CNS;

    ttiAction2.mirrorToIngressAnalyzerEnable = GT_TRUE;
    ttiAction2.mirrorToIngressAnalyzerIndex  = analyzerIndex;

    ttiAction2.continueToNextTtiLookup = GT_TRUE;

    /* AUTODOC: add TTI rule  with ETH key on port 0 VLAN 5 with action: redirect ot egress */
    rc = prvTgfTtiRule2Set(PRV_TGF_TTI0_INDEX_CNS, PRV_TGF_TTI_KEY_ETH_E,
                           &ttiPattern, &ttiMask, &ttiAction2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");

    /* AUTODOC: enable TTI lookup for port 0, key TTI_KEY_ETH */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_ETH_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d",
                                 prvTgfDevNum);
}

/**
* @internal tgfTunnelTermMirrorToIngressAnalyzerMirrorConfigurationSet function
* @endinternal
*
* @brief   Set mirroring configuration
*/
GT_VOID tgfTunnelTermMirrorToIngressAnalyzerMirrorConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                rc         = GT_OK;
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC  interface;

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Mirroring Configuration =======\n");

    /* AUTODOC: save analyzer interface in index 0 */
    rc = prvTgfMirrorAnalyzerInterfaceGet(prvTgfDevNum, analyzerIndex,
                                          &prvTgfRestoreCfg.interface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfMirrorAnalyzerInterfaceGet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: save analyzer enableMirror and index */
    rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexGet(prvTgfDevNum,
                                           &prvTgfRestoreCfg.prvTgfEnableMirror,
                                           &prvTgfRestoreCfg.analyzerIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                            "prvTgfMirrorRxGlobalAnalyzerInterfaceIndexGet: %d",
                            prvTgfDevNum);

    /* AUTODOC: set analyzer interface */
    interface.interface.type = CPSS_INTERFACE_PORT_E;
    interface.interface.devPort.hwDevNum  = prvTgfDevNum;
    interface.interface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    /* AUTODOC: set analyzer interface */
    rc = prvTgfMirrorAnalyzerInterfaceSet(analyzerIndex, &interface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "prvTgfMirrorAnalyzerInterfaceSet: %d", analyzerIndex);

    /* AUTODOC: enable global Rx mirroring */
    rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet(GT_TRUE, analyzerIndex);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                         "prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet: %d %d",
                         GT_TRUE, analyzerIndex);
}

/**
* @internal tgfTunnelTermMirrorToIngressAnalyzerTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID tgfTunnelTermMirrorToIngressAnalyzerTrafficGenerate
(
    GT_BOOL expectTraffic
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          portIter    = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;

    cpssOsMemSet(&portCntrs, 0, sizeof(portCntrs));
    cpssOsMemSet(&portCntrs, 0, sizeof(expectedCntrs));

    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
     */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                    "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
    */

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount, 0,
                             NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d",
                                 prvTgfDevNum);

    /* AUTODOC: send Ethernet packet from port 0 with: */
        /* AUTODOC: DA=00:00:44:33:22:11, SA=00:00:00:00:00:33, VID=1 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                                   prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     *  3. Get Ethernet Counters
     */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum,
                                   prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                   GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfReadPortCountersEth: %d, %d\n",
                                 prvTgfDevNum, prvTgfPortsArray[portIter]);

    /* check Tx counters */
    expectedCntrs.goodPktsSent.l[0]  = expectTraffic == GT_FALSE ? 1 : prvTgfBurstCount*2;


    rc = portCntrs.goodPktsSent.l[0] == expectedCntrs.goodPktsSent.l[0];
    UTF_VERIFY_EQUAL0_STRING_MAC(rc, GT_TRUE, "get another counters values.");
}


/**
* @internal tgfTunnelTermMirrorToIngressAnalyzerRuleSet function
* @endinternal
*
* @brief   Sets the TTTI rule mirrorToIngressAnalyzerEnable
*
* @param[in] enable                   - sets mirrorToIngressAnalyzerEnable
*                                      rule index - sets the index of a rule to set
*                                       None
*/
GT_VOID tgfTunnelTermMirrorToIngressAnalyzerRuleSet
(
    GT_BOOL enable,
    GT_U32  ruleIndex
)
{
    GT_STATUS rc = GT_OK;

    ttiAction2.mirrorToIngressAnalyzerEnable = enable;

    /* AUTODOC: add TTI rule  with ETH key on port 0 VLAN 5 with action: redirect ot egress */
    rc = prvTgfTtiRule2Set(ruleIndex, PRV_TGF_TTI_KEY_ETH_E,
                           &ttiPattern, &ttiMask, &ttiAction2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");
}

/**
* @internal tgfTunnelTermMirrorToIngressAnalyzerConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID tgfTunnelTermMirrorToIngressAnalyzerConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    PRV_UTF_LOG0_MAC("======= Restore Configuration =======\n");

    /* AUTODOC: clear packet table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: disable TTI lookup for port 0, key TTI_KEY_ETH */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_ETH_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: invalidate TTI rules */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI0_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfTtiRuleValidStatusSet: %d, %d",
                                 prvTgfDevNum, GT_FALSE);


    /* AUTODOC: invalidate TTI rules */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI1_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfTtiRuleValidStatusSet: %d, %d",
                                 prvTgfDevNum, GT_FALSE);

    /* AUTODOC: restore mirror interface configuration */
    rc = prvTgfMirrorAnalyzerInterfaceSet(prvTgfRestoreCfg.analyzerIndex,
                                          &prvTgfRestoreCfg.interface);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfTtiRuleValidStatusSet: %d, %d",
                                 prvTgfRestoreCfg.analyzerIndex,
                                 &prvTgfRestoreCfg.interface);

    /* AUTODOC: disable global Rx mirroring */
    rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet(
                    prvTgfRestoreCfg.prvTgfEnableMirror,
                    prvTgfRestoreCfg.analyzerIndex);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                         "prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet: %d %d",
                         prvTgfRestoreCfg.prvTgfEnableMirror,
                         prvTgfRestoreCfg.analyzerIndex);

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
                                 prvTgfDevNum, GT_TRUE);
}

/**
* @internal tgfTunnelTermMirrorToIngressAnalyzer function
* @endinternal
*
* @brief   Test main func
*/
GT_VOID tgfTunnelTermMirrorToIngressAnalyzer(GT_VOID)
{
    tgfTunnelTermMirrorToIngressAnalyzerBridgeConfigSet();

    /* AUTODOC: add TTI rule 0 with mirrorToIngressAnalyzerEnable [enabled] and index [0] with action: redirect to egress */
    tgfTunnelTermMirrorToIngressAnalyzerTtiConfigurationSet();

    /* AUTODOC: set analyzer interface configuration with index [0] */
    tgfTunnelTermMirrorToIngressAnalyzerMirrorConfigurationSet();

    /* ---------------------------------------------------------------------- */
    /* AUTODOC: send packet and expect TTI match and packet mirrored */
    tgfTunnelTermMirrorToIngressAnalyzerTrafficGenerate(GT_TRUE);

    /* ---------------------------------------------------------------------- */
    /* AUTODOC: change TTI rule 0 mirrorToIngressAnalyzerEnable to [disabled] */
    /* AUTODOC: send packet and expect no mirrored packet */
    tgfTunnelTermMirrorToIngressAnalyzerRuleSet(GT_FALSE, PRV_TGF_TTI0_INDEX_CNS);
    tgfTunnelTermMirrorToIngressAnalyzerTrafficGenerate(GT_FALSE);

    /* ---------------------------------------------------------------------- */
    /* AUTODOC: add TTI rule 1 with mirrorToIngressAnalyzerEnable [enabled] and index [0] with action: redirect to egress */
    /* AUTODOC: send packet and expect TTI0 and TT1 match and packet mirrored */
    tgfTunnelTermMirrorToIngressAnalyzerRuleSet(GT_TRUE, PRV_TGF_TTI1_INDEX_CNS);
    tgfTunnelTermMirrorToIngressAnalyzerTrafficGenerate(GT_TRUE);

    /* ---------------------------------------------------------------------- */
    /* Restore configuration */
    tgfTunnelTermMirrorToIngressAnalyzerConfigRestore();
}

