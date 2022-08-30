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
* @file prvTgfIpv4McRoutingWithFdb.c
*
* @brief FDB basedIPV4 MC Routing
*
* @version   39
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfL2MllGen.h>
#include <common/tgfIpGen.h>
#include <ip/prvTgfIpv4McRoutingWithFdb.h>
#include <gtOs/gtOsMem.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* send VLAN */
#define PRV_TGF_SEND_VLANID_CNS           5

/* next hop vlan 1 */
#define PRV_TGF_NEXTHOP_VLANID1_CNS       6

/* next hop vlan 2 */
#define PRV_TGF_NEXTHOP_VLANID2_CNS       7

/* port index to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         1

/* port index for subscriber 1 */
#define PRV_TGF_MC_1_PORT_IDX_CNS         3

/* port index for subscriber 2 */
#define PRV_TGF_MC_2_PORT_IDX_CNS         2

/* port index for subscriber 3 */
#define PRV_TGF_MC_3_PORT_IDX_CNS         0

/* is this a call from Dual HW Device test */
static GT_BOOL       isDualHwDevTest = GT_FALSE;

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = 1;

/* MLL pair pointer indexes */
static GT_U32        prvTgfMllPointerIndex1     = 5;
static GT_U32        prvTgfNextMllPointerIndex1 = 6;

/* MLL pair pointer indexes */
static GT_U32        prvTgfMllPointerIndex2     = 7;
static GT_U32        prvTgfNextMllPointerIndex2 = 8;

/* Route entry 1 index for MC Route entry Table */
static GT_U32        prvTgfRouteEntryMcBaseIndex1 = 8;
/* Route entry 2 index for MC Route entry Table */
static GT_U32        prvTgfRouteEntryMcBaseIndex2 = 10;

/* expected number of sent packets on MC subscriber's ports */
static GT_U8 prvTgfPacketsCountRxMC1 = 0;
static GT_U8 prvTgfPacketsCountRxMC2 = 1;
static GT_U8 prvTgfPacketsCountRxMC3 = 0;

/* expected number of sent packets on MC subscriber's ports */
static GT_U8 prvTgfPacketsCountTxMC1 = 1;
static GT_U8 prvTgfPacketsCountTxMC2 = 1;
static GT_U8 prvTgfPacketsCountTxMC3 = 1;

/* VRF-Id */
static GT_U32   prvTgfVrfId1 = 1;
static GT_U32   prvTgfVrfId2 = 2;

/* PCL rule index */
static GT_U32   prvTgfPclRuleIndex = 1;

static GT_U32   prvTgfPbrLeafIndex = 964;

static GT_U32 prvTgfIpv4McFdbEntryIndex;

/* MAC2ME entry index */
#define PRV_TGF_MAC_TO_ME_INDEX_CNS     1

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x01, 0x00, 0x5E, 0x01, 0x01, 0x01},               /* daMac = macGrp */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                /* saMac */
};
/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};
/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {
    TGF_ETHERTYPE_0800_IPV4_TAG_CNS
};
/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                                 /* version */
    5,                                 /* headerLen */
    0,                                 /* typeOfService */
    0x2A,                              /* totalLen = 42 */
    0,                                 /* id */
    0,                                 /* flags */
    0,                                 /* offset */
    0x40,                              /* timeToLive = 64 */
    0x04,                              /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    { 1,  1,  1,  1},                  /* srcAddr = ipSrc */
    { 224,1,  1,  1}                   /* dstAddr = ipGrp */
};
/* DATA of packet len = 22 (0x16) */
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
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/*************************** Restore config ***********************************/

/******************************************************************************/

/* parameters that is needed to be restored */
static struct
{
    GT_BOOL         mcPortRoutingEn;
    GT_BOOL         ucPortRoutingEn;
    GT_BOOL         mcFdbRoutingEn;
    GT_BOOL         ucFdbRoutingEn;
    GT_U32          bypassTrigger;
    PRV_TGF_MAC_HASH_FUNC_MODE_ENT multihash;
} prvTgfRestoreCfg;

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfBasicIpv4McRoutingBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*/
GT_VOID prvTgfIpv4McRoutingWithFdbBaseConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U8       tagArray[] = {1, 1, 1, 1};
    PRV_TGF_TTI_MAC_VLAN_STC             macToMePattern;
    PRV_TGF_TTI_MAC_VLAN_STC             macToMeMask;

    /* ---------------------------------------------------------------------------------
     * 1. Set VLAN
     */
    /* AUTODOC: set ingress vlan tag */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS, prvTgfPortsArray, NULL, tagArray, 2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet failed");

    /* AUTODOC: set egress multicast vlan tag 1*/
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_NEXTHOP_VLANID1_CNS, prvTgfPortsArray+2, NULL, tagArray, 2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet failed");

    /* AUTODOC: set egress multicast vlan tag 2*/
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_NEXTHOP_VLANID2_CNS, prvTgfPortsArray+2, NULL, tagArray, 2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet failed");

    /* ----------------------------------------------------------------------------------
     * 2. Enable Routing
     */
    /* AUTODOC: store MC IPv4 Routing on ingress port state*/
    rc = prvTgfIpPortRoutingEnableGet(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_MULTICAST_E,
            CPSS_IP_PROTOCOL_IPV4_E, &prvTgfRestoreCfg.mcPortRoutingEn);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnableGet failed");
    /* AUTODOC: enable MC IPv4 Routing on ingress port */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_MULTICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable failed");

    /* AUTODOC: enable IPv4 MC Routing on ingress Vlan */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_MULTICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable failed");

    /* Store FDB based multicast routing state*/
    rc = cpssDxChIpFdbMulticastRouteForPbrEnableGet(prvTgfDevNum, &prvTgfRestoreCfg.mcFdbRoutingEn);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChIpFdbMulticastRouteForPbrEnableGet failed");
    /* Enable FDB based multicast routing */
    rc = cpssDxChIpFdbMulticastRouteForPbrEnableSet(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChIpFdbMulticastRouteForPbrEnableSet failed");

    /* AUTODOC: store CRC Multi HASH mode */
    rc =  prvTgfBrgFdbHashModeGet(prvTgfDevNum,&prvTgfRestoreCfg.multihash);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbHashModeGet failed");
    /* AUTODOC: set CRC Multi HASH mode */
    rc =  prvTgfBrgFdbHashModeSet(prvTgfDevNum,PRV_TGF_MAC_HASH_FUNC_CRC_MULTI_HASH_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbHashModeSet failed");

    /* AUTODOC: set MAC to ME table */
    cpssOsMemCpy((GT_VOID*)macToMePattern.mac.arEther,
                 (GT_VOID*)prvTgfPacketL2Part.daMac,
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
* @internal prvTgfIpv4McRoutingWithFdbBaseVrfIdSet function
* @endinternal
*
* @brief   Assign VrfId
*
* @param   index - 0/1 first/second VrfId assingment
*/
GT_VOID prvTgfIpv4McRoutingWithFdbBaseVrfIdSet
(
    GT_U32 index
)
{
    GT_STATUS rc;
    GT_U32 vrfId;

    if(index == 0)
    {
        vrfId = prvTgfVrfId1;
    }
    else
    {
        vrfId = prvTgfVrfId2;
    }

    /* AUTODOC: update VRF Id in vlan[PRV_TGF_SEND_VLANID_CNS] */
    rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_SEND_VLANID_CNS, vrfId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanVrfIdSet failed");
}

/**
* @internal prvTgfIpv4McRoutingWithFdbPCLEntrySet function
* @endinternal
*
* @brief   Set PCL configuration
*/
static GT_VOID prvTgfIpv4McRoutingWithFdbPCLConfigSet
(
        GT_VOID
)
{
    GT_STATUS                   rc;
    PRV_TGF_PCL_RULE_FORMAT_UNT mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT pattern;
    PRV_TGF_PCL_ACTION_STC      action;

    /**************************************************************************
    * 4. PCL configuration
    */

    /* mask for MAC address */
    cpssOsMemSet(&mask, 0, sizeof(mask));

    /* define mask, pattern and action */
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    /* action redirect */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd                                           = CPSS_PACKET_CMD_FORWARD_E;
    action.bypassIngressPipe                                = GT_FALSE;
    action.redirect.redirectCmd                             = PRV_TGF_PCL_ACTION_REDIRECT_CMD_ROUTER_E;
    action.redirect.data.routerLttIndex                     = prvTgfPbrLeafIndex;
    /*action.redirect.vlanId                                  = PRV_TGF_NEXTHOP_VLAN_ID_CNS;*/

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
}

/**
* @internal prvTgfIpv4McRoutingWithFdbFdbConfigSet function
* @endinternal
*
* @brief   Set FDB configuration
*/
static GT_VOID prvTgfIpv4McRoutingWithFdbFdbConfigSet
(
        GT_VOID
)
{
    GT_STATUS                       rc;
    PRV_TGF_BRG_MAC_ENTRY_STC       macEntry;
    PRV_TGF_BRG_MAC_ENTRY_STC       macEntryGet;
    GT_BOOL                      valid;
    GT_BOOL                      skip;
    GT_BOOL                      aged;
    GT_HW_DEV_NUM                hwDevNum;
    GT_U32                       counterVal;

    /**************************************************************************
    * 3. Write FDB entry
    */
    /* AUTODOC: set Packet Commands for FDB routed packets */
    rc = prvTgfBrgFdbRoutingNextHopPacketCmdSet(prvTgfDevNum, CPSS_PACKET_CMD_ROUTE_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingNextHopPacketCmdSet");

    /* Write FDB entry in FDB table */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));

    /* AUTODOC: set  IPv4 MC route entry in FDB*/
    macEntry.key.key.ipv4Unicast.vrfId       = prvTgfVrfId1;
    macEntry.key.key.ipv4Unicast.dip[0]      = prvTgfPacketIpv4Part.srcAddr[0];
    macEntry.key.key.ipv4Unicast.dip[1]      = prvTgfPacketIpv4Part.srcAddr[1];
    macEntry.key.key.ipv4Unicast.dip[2]      = prvTgfPacketIpv4Part.srcAddr[2];
    macEntry.key.key.ipv4Unicast.dip[3]      = prvTgfPacketIpv4Part.srcAddr[3];
    macEntry.key.entryType                   = PRV_TGF_FDB_ENTRY_TYPE_IPV4_UC_E;
    macEntry.fdbRoutingInfo.routingType      = CPSS_FDB_UC_ROUTING_TYPE_MULTICAST_NH_ENTRY_E;
    macEntry.fdbRoutingInfo.nextHopMcPointer = prvTgfRouteEntryMcBaseIndex1;

    rc =  prvTgfBrgFdbMacEntryIndexFind(&(macEntry.key),&prvTgfIpv4McFdbEntryIndex);
    if (rc == GT_NOT_FOUND)
    {
        rc = GT_OK;
    }
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryIndexFind_MultiHash: %d", prvTgfDevNum);

    rc = prvTgfBrgFdbMacEntryRead(prvTgfIpv4McFdbEntryIndex,
                                  &valid,
                                  &skip,
                                  &aged,
                                  &hwDevNum,
                                  &macEntryGet);
    PRV_UTF_LOG1_MAC("prvTgfFdbIpv4UcRoutingRouteConfigurationGetByIndex, prvTgfIpv4McFdbEntryIndex[%d]\n", prvTgfIpv4McFdbEntryIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryRead: %d", prvTgfDevNum);

    rc = prvTgfBrgFdbMacEntryWrite(prvTgfIpv4McFdbEntryIndex, GT_FALSE, &macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite: %d", prvTgfDevNum);

    if(valid==GT_FALSE)/* new entry is added need to update the counter */
    {
        rc =  prvTgfBrgFdbBankCounterUpdate(prvTgfDevNum, prvTgfIpv4McFdbEntryIndex%16,GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbBankCounterUpdate: %d", prvTgfIpv4McFdbEntryIndex%16);
    }

    rc =  prvTgfBrgFdbBankCounterValueGet(prvTgfDevNum, prvTgfIpv4McFdbEntryIndex%16,&counterVal);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbBankCounterValueGet: %d", prvTgfIpv4McFdbEntryIndex%16);
    PRV_UTF_LOG1_MAC("after adding an entry prvTgfBrgFdbBankCounterValueGet counter = %d\n",counterVal);
}

/**
* @internal prvTgfIpv4McRoutingWithFdbMcNextHopEntrySet function
* @endinternal
*
* @brief   Set Mulitcast nextHop entry configuration
*
* @param[in]    nextHopEntryIndex   - index to nexhop entry
*               mllIndex            - index to mll entry
*               mllNextIndex        - index to mll next entry
*               nextHopVid          - next hop vlan id.
*/
static GT_VOID prvTgfIpv4McRoutingWithFdbMcNextHopConfigSet
(
    GT_U32      nextHopEntryIndex,
    GT_U32      mllIndex,
    GT_U32      mllNextIndex,
    GT_U32      nextHopVid
)
{
    GT_STATUS                       rc = GT_OK;
    PRV_TGF_IP_MLL_PAIR_STC         mllPairEntry;
    PRV_TGF_IP_MLL_STC             *mllNodePtr;
    PRV_TGF_IP_MC_ROUTE_ENTRY_STC   mcRouteEntryArray[1];
    CPSS_IP_PROTOCOL_STACK_ENT      protocol = CPSS_IP_PROTOCOL_IPV4_E;

    /**************************************************************************
     * 1. Write MLL Pair entry in MLL table
     */
    /* create first and second MC Link List */
    cpssOsMemSet(&mllPairEntry, 0, sizeof(mllPairEntry));

    mllNodePtr = &mllPairEntry.firstMllNode;
    mllNodePtr->mllRPFFailCommand                = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mllNodePtr->nextHopInterface.type            = CPSS_INTERFACE_PORT_E;
    mllNodePtr->nextHopInterface.devPort.hwDevNum  = prvTgfDevNum;
    mllNodePtr->nextHopInterface.devPort.portNum =
                                  prvTgfPortsArray[PRV_TGF_MC_1_PORT_IDX_CNS];
    mllNodePtr->nextHopVlanId                    = nextHopVid;
    mllNodePtr->ttlHopLimitThreshold             = 4;
    mllNodePtr->last                             = GT_FALSE;

    mllNodePtr = &mllPairEntry.secondMllNode;
    mllNodePtr->mllRPFFailCommand                = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mllNodePtr->nextHopInterface.type            = CPSS_INTERFACE_PORT_E;
    mllNodePtr->nextHopInterface.devPort.hwDevNum  = prvTgfDevNum;
    mllNodePtr->nextHopInterface.devPort.portNum =
                                  prvTgfPortsArray[PRV_TGF_MC_2_PORT_IDX_CNS];
    mllNodePtr->nextHopVlanId                    = nextHopVid;
    mllNodePtr->ttlHopLimitThreshold             = 4;
    mllNodePtr->last                             = GT_FALSE;

    mllPairEntry.nextPointer                     = (GT_U16)mllNextIndex;

    /* AUTODOC: create 1 and 2 MC Link List 5: */
    /* AUTODOC:   1. port 1, VLAN 55 */
    /* AUTODOC:   2. port 3, VLAN 17 */
    rc = prvTgfIpMllPairWrite(mllIndex,
                              PRV_TGF_PAIR_READ_WRITE_WHOLE_E,
                              &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllPairWrite");
    /* create third MC Link List */
    cpssOsMemSet(&mllPairEntry, 0, sizeof(mllPairEntry));

    mllNodePtr = &mllPairEntry.firstMllNode;
    mllNodePtr->mllRPFFailCommand                = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mllNodePtr->nextHopInterface.type            = CPSS_INTERFACE_PORT_E;
    mllNodePtr->nextHopInterface.devPort.hwDevNum  = prvTgfDevNum;
    mllNodePtr->nextHopInterface.devPort.portNum =
                                  prvTgfPortsArray[PRV_TGF_MC_3_PORT_IDX_CNS];
    mllNodePtr->nextHopVlanId                    = nextHopVid;
    mllNodePtr->ttlHopLimitThreshold             = 4;
    mllNodePtr->last                             = GT_TRUE;

    mllPairEntry.nextPointer                     = 0;

    /* AUTODOC: create 3 MC Link List 6: */
    /* AUTODOC:   3. port 0, VLAN 55 */
    rc = prvTgfIpMllPairWrite(mllNextIndex,
                              PRV_TGF_PAIR_READ_WRITE_FIRST_ONLY_E,
                              &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllPairWrite");

    /**************************************************************************
    * 2. Write Multicast Route entry
    */

    /* create a MC Route entry */
    cpssOsMemSet(&mcRouteEntryArray[0], 0, sizeof(mcRouteEntryArray[0]));
    mcRouteEntryArray[0].cmd                = CPSS_PACKET_CMD_ROUTE_E;
    mcRouteEntryArray[0].countSet           = CPSS_IP_CNT_SET2_E;
    mcRouteEntryArray[0].RPFFailCommand     = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mcRouteEntryArray[0].ipv4MllPtr         = mllIndex;

    /* AUTODOC: add MC route entry with index 10, MLL Ptr 5 */
    rc = prvTgfIpMcRouteEntriesWrite(nextHopEntryIndex,
                                     1,
                                     protocol,
                                     mcRouteEntryArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMcRouteEntriesWrite");
}

/**
* @internal prvTgfIpv4McRoutingWithFdbLpmLeafSet function
* @endinternal
*
* @brief   Set LPM Leaf Configuration with priority
*/
GT_VOID prvTgfIpv4McRoutingWithFdbLpmLeafSet
(
    CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_TYPE_ENT priority
)
{
    GT_STATUS                       rc = GT_OK;
    PRV_TGF_LPM_LEAF_ENTRY_STC      leafEntry;

    cpssOsMemSet(&leafEntry, 0, sizeof(PRV_TGF_LPM_LEAF_ENTRY_STC));

    /* write a leaf entry to the PBR area in RAM */
    leafEntry.entryType                = PRV_TGF_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
    leafEntry.index                    = prvTgfRouteEntryMcBaseIndex2;
    leafEntry.ucRPFCheckEnable         = GT_FALSE;
    leafEntry.sipSaCheckMismatchEnable = GT_FALSE;
    leafEntry.ipv6MCGroupScopeLevel    = 0;
    leafEntry.priority                 = priority;

    /* AUTODOC: Write a PBR leaf */
    rc = prvTgfLpmLeafEntryWrite(prvTgfDevNum, prvTgfPbrLeafIndex, &leafEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfLpmLeafEntryWrite: %d", prvTgfDevNum);
}

/**
* @internal prvTgfIpv4McRoutingWithFdbPclActionVrfIdAssignPCLConfigSet function
* @endinternal
*
* @brief   Set PCL configuration
*/
static GT_VOID prvTgfIpv4McRoutingWithFdbPclActionVrfIdAssignPCLConfigSet
(
        GT_VOID
)
{
    GT_STATUS                          rc;
    static PRV_TGF_PCL_RULE_FORMAT_UNT mask;
    static PRV_TGF_PCL_RULE_FORMAT_UNT pattern;
    static PRV_TGF_PCL_ACTION_STC      action;

    /**************************************************************************
    * 4. PCL configuration
    */

    /* mask for MAC address */
    cpssOsMemSet(&mask, 0, sizeof(mask));

    /* define mask, pattern and action */
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    /* action redirect */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd                                           = CPSS_PACKET_CMD_FORWARD_E;
    action.bypassIngressPipe                                = GT_FALSE;
    action.redirect.redirectCmd                             = PRV_TGF_PCL_ACTION_REDIRECT_CMD_ROUTER_AND_ASSIGN_VRF_ID_E;
    action.redirect.data.routeAndVrfId.routerLttIndex       = prvTgfPbrLeafIndex;
    action.redirect.data.routeAndVrfId.vrfId                = prvTgfVrfId1;

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
}

/**
* @internal prvTgfIpv4McRoutingWithFdbPclActionVrfIdAssignRouteConfigSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfIpv4McRoutingWithFdbPclActionVrfIdAssignRouteConfigSet
(
    GT_VOID
)
{
    /* set MC Route entry 1*/
    prvTgfIpv4McRoutingWithFdbMcNextHopConfigSet(prvTgfRouteEntryMcBaseIndex1, prvTgfMllPointerIndex1,
            prvTgfNextMllPointerIndex1, PRV_TGF_NEXTHOP_VLANID1_CNS);

    /* set FDB configuration */
    prvTgfIpv4McRoutingWithFdbFdbConfigSet();

    /* set PCL configuration */
    prvTgfIpv4McRoutingWithFdbPclActionVrfIdAssignPCLConfigSet();

}

/**
* @internal prvTgfBasicIpv4McRoutingLttRouteConfigurationSet function
* @endinternal
*
* @brief   Set Route Configuration
*/
GT_VOID prvTgfIpv4McRoutingWithFdbRouteConfigurationSet
(
    GT_VOID
)
{
    /* set MC Route entry 1*/
    prvTgfIpv4McRoutingWithFdbMcNextHopConfigSet(prvTgfRouteEntryMcBaseIndex1, prvTgfMllPointerIndex1,
            prvTgfNextMllPointerIndex1, PRV_TGF_NEXTHOP_VLANID1_CNS);

    /* set MC Route entry 2*/
    prvTgfIpv4McRoutingWithFdbMcNextHopConfigSet(prvTgfRouteEntryMcBaseIndex2, prvTgfMllPointerIndex2,
            prvTgfNextMllPointerIndex2, PRV_TGF_NEXTHOP_VLANID2_CNS);

    /* set FDB configuration */
    prvTgfIpv4McRoutingWithFdbFdbConfigSet();

    /* set PCL configuration */
    prvTgfIpv4McRoutingWithFdbPCLConfigSet();

}

/**
* @internal prvTgfBasicIpv4McRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpv4McRoutingWithFdbTrafficGenerate
(
    GT_BOOL expectFdbRouting
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          portIter    = 0;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[2];
    CPSS_INTERFACE_INFO_STC         portInt;
    PRV_TGF_IP_COUNTER_SET_STC      ipCounters;
    GT_U32                          mllOutMCPkts;
    GT_U32                          oldMllOutMCPktsSet0;
    GT_U32                          oldMllOutMCPktsSet1;
    GT_U32                          mllCntSet_0 = 0, mllCntSet_1 = 1;
    GT_U32                          expectVlan = 0;
    PRV_TGF_IP_COUNTER_SET_INTERFACE_CFG_STC cntIntCfg;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: reset IP couters and set ROUTE_ENTRY mode on all ports */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* enable capture on a MC subscriber port */
    portInt.type            = CPSS_INTERFACE_PORT_E;
    portInt.devPort.hwDevNum  = prvTgfDevNum;
    portInt.devPort.portNum = prvTgfPortsArray[PRV_TGF_MC_2_PORT_IDX_CNS];

    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInt,
           TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
           "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
           prvTgfDevNum, prvTgfPortsArray[PRV_TGF_MC_2_PORT_IDX_CNS]);

    /* get MLL counters for MC subscriber 1 */
    rc = prvTgfIpMllCntGet(prvTgfDevNum, mllCntSet_0, &oldMllOutMCPktsSet0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllCntGet");
    /* get MLL counters for MC subscriber 1 */
    rc = prvTgfIpMllCntGet(prvTgfDevNum, mllCntSet_0, &oldMllOutMCPktsSet1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllCntGet");

    /* set MLL counters mode for MC subscriber 1 */
    cpssOsMemSet(&cntIntCfg, 0, sizeof(cntIntCfg));
    cntIntCfg.ipMode           = CPSS_IP_PROTOCOL_IPV4_E;
    cntIntCfg.devNum           = prvTgfDevNum;
    cntIntCfg.portTrunkCntMode = PRV_TGF_IP_PORT_CNT_MODE_E;
    cntIntCfg.portTrunk.port   = prvTgfPortsArray[PRV_TGF_MC_1_PORT_IDX_CNS];

    rc = prvTgfIpSetMllCntInterface(mllCntSet_0, &cntIntCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpSetMllCntInterface");

    /* set MLL counters mode for MC subscriber 2 */
    cntIntCfg.portTrunk.port   = prvTgfPortsArray[PRV_TGF_MC_2_PORT_IDX_CNS];

    rc = prvTgfIpSetMllCntInterface(mllCntSet_1, &cntIntCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpSetMllCntInterface");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo,
                             prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

    /* AUTODOC: send IPv4 packet from port 2 with: */
    /* AUTODOC:   DA=01:00:5e:01:01:01, SA=00:00:00:00:00:33 */
    /* AUTODOC:   VID=5, srcIP=1.1.1.1, dstIP=224.1.1.1 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                                   prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "ERROR of StartTransmitting: %d, %d\n",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    cpssOsTimerWkAfter(100);

    /* AUTODOC: verify routed packet on ports 0,1,3 */
    if((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)&&
       (isDualHwDevTest == GT_TRUE))
    {
        /* AUTODOC: verify 1 flood in ingress VLAN 80 to port 1 */
        prvTgfPacketsCountTxMC1 = 2;
    }

    rc = prvTgfEthCountersCheck(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_MC_3_PORT_IDX_CNS],
            prvTgfPacketsCountRxMC3, prvTgfPacketsCountTxMC3,
            prvTgfPacketInfo.totalLen, prvTgfBurstCount);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

    rc = prvTgfEthCountersCheck(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_MC_1_PORT_IDX_CNS],
            prvTgfPacketsCountRxMC1, prvTgfPacketsCountTxMC1,
            prvTgfPacketInfo.totalLen, prvTgfBurstCount);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

    rc = prvTgfEthCountersCheck(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_MC_2_PORT_IDX_CNS],
            prvTgfPacketsCountRxMC2, prvTgfPacketsCountTxMC2,
            prvTgfPacketInfo.totalLen, prvTgfBurstCount);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

    /* get Trigger Counters */
    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInt.devPort.portNum);

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

    if (expectFdbRouting)
    {
        expectVlan = PRV_TGF_NEXTHOP_VLANID1_CNS;
    }
    else
    {
        expectVlan = PRV_TGF_NEXTHOP_VLANID2_CNS;
    }

    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));
    cpssOsMemCpy(vfdArray[1].patternPtr, &expectVlan, 1);

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInt, 2, vfdArray,
                                                        &numTriggers);
    PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

    /* check TriggerCounters */
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
        "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
        portInt.devPort.hwDevNum, portInt.devPort.portNum);

    /* check if captured packet has the same MAC DA as prvTgfArpMac */
    UTF_VERIFY_EQUAL0_STRING_MAC(3, numTriggers,
                                 "No packet captured or wrong Vlan packet received!");

    /* get and print ip counters values */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++) {
        PRV_UTF_LOG1_MAC("IP counters for Port [%d]:\n",
                         prvTgfPortsArray[portIter]);
        prvTgfCountersIpGet(prvTgfDevNum, portIter, GT_TRUE, &ipCounters);
    }
    PRV_UTF_LOG0_MAC("\n");

    /* disable capture on a MC subscriber port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInt,
            TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
            prvTgfDevNum, portInt.devPort.portNum);

    /* get MLL counters for MC subscriber 1 */
    rc = prvTgfIpMllCntGet(prvTgfDevNum, mllCntSet_0, &mllOutMCPkts);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllCntGet");
    PRV_UTF_LOG1_MAC("MC subscriber 1: mllOutMCPkts = %d\n", mllOutMCPkts - oldMllOutMCPktsSet0);

    /* get MLL counters for MC subscriber 2 */
    rc = prvTgfIpMllCntGet(prvTgfDevNum, mllCntSet_1, &mllOutMCPkts);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllCntGet");
    PRV_UTF_LOG1_MAC("MC subscriber 2: mllOutMCPkts = %d\n", mllOutMCPkts - oldMllOutMCPktsSet1);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

/**
* @internal prvTgfBasicIpv4McRoutingConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfIpv4McRoutingWithFdbConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_BOOL     ucRouteDelStatusGet;
    PRV_TGF_TTI_MAC_VLAN_STC             macToMePattern;
    PRV_TGF_TTI_MAC_VLAN_STC             macToMeMask;

    /* -------------------------------------------------------------------------
     * 1. Restore Vlan Configuration
     */
    /* invalidate send vlan id */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, PRV_TGF_SEND_VLANID_CNS);

    /* invalidate nexthop vid 1 */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_NEXTHOP_VLANID1_CNS);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, PRV_TGF_NEXTHOP_VLANID1_CNS);

    /* invalidate nexthop vid 2 */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_NEXTHOP_VLANID2_CNS);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, PRV_TGF_NEXTHOP_VLANID2_CNS);

    /* -------------------------------------------------------------------------
     * 2. Restore Routing enable Configuration
     */
    /* AUTODOC: restore MC IPv4 Routing on ingress port state*/
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_MULTICAST_E,
            CPSS_IP_PROTOCOL_IPV4_E, prvTgfRestoreCfg.mcPortRoutingEn);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable failed");

    /* AUTODOC: restore IPv4 MC Routing on ingress Vlan state*/
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_MULTICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable failed");

    /* restore FDB based multicast routing state*/
    rc = cpssDxChIpFdbMulticastRouteForPbrEnableSet(prvTgfDevNum, prvTgfRestoreCfg.mcFdbRoutingEn);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChIpFdbMulticastRouteForPbrEnableSet failed");

    /* AUTODOC: restore CRC Multi HASH mode */
    rc =  prvTgfBrgFdbHashModeSet(prvTgfDevNum, prvTgfRestoreCfg.multihash);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbHashModeSet failed");

    /* -----------------------------------------------------------------------------
     * 3. Restore PCL and FDB configuration
     */
    /* Restore PCL configuration */
    rc = prvTgfPclRuleValidStatusSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E, prvTgfPclRuleIndex, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet");

    prvTgfPclRestore();

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

    /* -----------------------------------------------------------------------------
     * 4. Invalidate MAC to ME table entry index 1
     */

    /* full mask for VLAN and MAC */
    macToMeMask.mac.arEther[0] =
    macToMeMask.mac.arEther[1] =
    macToMeMask.mac.arEther[2] =
    macToMeMask.mac.arEther[3] =
    macToMeMask.mac.arEther[4] =
    macToMeMask.mac.arEther[5] = 0xFF;
    macToMeMask.vlanId = 0xFFF;

    /* use 0 MAC DA and VID = 4095 */
    macToMePattern.mac.arEther[0] =
    macToMePattern.mac.arEther[1] =
    macToMePattern.mac.arEther[2] =
    macToMePattern.mac.arEther[3] =
    macToMePattern.mac.arEther[4] =
    macToMePattern.mac.arEther[5] = 0;
    macToMePattern.vlanId = 0xFFF;

    rc = prvTgfTtiMacToMeSet(PRV_TGF_MAC_TO_ME_INDEX_CNS, &macToMePattern, &macToMeMask);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacToMeSet");

}
