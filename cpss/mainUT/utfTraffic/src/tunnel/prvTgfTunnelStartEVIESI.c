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
 * @file prvTgfTunnelStartEVIESI.c
 *
 * @brief Adding EVI and ESI labels for IPv4 (MC) over IPv6 packet
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
#include <tunnel/prvTgfTunnelStartEVIESI.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgE2Phy.h>
#include <l2mll/prvTgfL2MllUtils.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/


/* send VLAN */
#define PRV_TGF_SEND_VLANID_CNS           80

/* multicast subscriber VLAN */
#define PRV_TGF_NEXTHOP_VLANID_CNS        55

/* port index to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         2

/* port index to receive traffic from */
#define PRV_TGF_RCV_PORT_IDX_CNS          1

/* default port index to receive traffic from */
#define PRV_TGF_DEF_RCV_PORT_IDX_CNS      0


/* e-port 1 */
#define PRV_TGF_E_PORT1_IDX_CNS           0x103

/* e-port 2 */
#define PRV_TGF_E_PORT2_IDX_CNS           0x105

/* number of packets to send */
#define PRV_TGF_SEND_PACKETS_NUM_CNS      1


/* the tunnel start pointer index 1 */
static GT_U32        prvTgfTunnelStartEntry1BaseIndex = 0x14;

/* the tunnel start pointer index 2 */
static GT_U32        prvTgfTunnelStartEntry2BaseIndex = 0x28;

/* Tunnel next hop MAC DA */
static TGF_MAC_ADDR  prvTgfTunnelMacDa = {0x00, 0x00, 0xEE, 0xEE, 0xFF, 0xFF};

/* Tunnel destination IP */
static GT_U8 prvTgfTunnelDstIp[] = {0x11, 0x11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x88, 0, 0x88 };

/* Tunnel source IP */
static GT_U8 prvTgfTunnelSrcIp[] = {0x22, 0x22, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x88, 0, 0x88};

/* MAC for Tunnel Start Entry */
static TGF_MAC_ADDR prvTgfTunnelMacDa2 = {0x00, 0x00, 0xAA, 0xBB, 0xCC, 0xDD};

/* Tunnel destination IP */
static GT_U8 prvTgfTunnelDstIp2[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x89 };

/* Tunnel source IP */
static GT_U8 prvTgfTunnelSrcIp2[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x99};

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = 1;

/* Default Route entry index for MC Route entry Table */
static GT_U32        prvTgfDefRouteEntryIndex  = 1;

/* the Route entry index for MC Route entry Table */
static GT_U32        prvTgfRouteEntryBaseIndex = 10;

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 0;

/* expected number of packets on MC subscriber's ports */
static GT_U8 prvTgfPacketsCountRxTxArr[PRV_TGF_SEND_PACKETS_NUM_CNS][PRV_TGF_PORTS_NUM_CNS] =
{
        {1, 1, 1, 0/* 2 MC routed */}
};


/* Source IP addresses for packets */
static TGF_IPV4_ADDR  prvTgfSrcAddrArr[PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
        {1, 1, 1, 1}
};


/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
        {0x01, 0x00, 0x5E, 0x01, 0x01, 0x01},               /* daMac = macGrp */
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x33}                /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
        TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
        0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart =
{
        TGF_ETHERTYPE_0800_IPV4_TAG_CNS
};
/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part =
{
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
        { 1,  1, 1, 1},                    /* srcAddr = ipSrc */
        { 224,1, 1, 1}                     /* dstAddr = ipGrp */
};

/* DATA of packet len = 22 (0x16) */
static GT_U8 prvTgfPayloadDataArr[] =
{
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
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
        TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + \
        TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
        PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
        sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
        prvTgfPacketPartArray                                        /* partsArray */
};

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    GT_U16              vid;
    CPSS_PORTS_BMP_STC  defPortMembers;
    GT_U8               eVlanTtl;
    GT_U32              controlWord;
} prvTgfRestoreCfg;

/******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
 * @internal prvTgfTunnelStartEVIESIBaseConfigurationSet function
 * @endinternal
 *
 * @brief   Set Base Configuration
 */
static GT_VOID prvTgfTunnelStartEVIESIBaseConfigurationSet
(
        GT_VOID
)
{
    GT_STATUS           rc          = GT_OK;

    /* AUTODOC: create VLAN 80 with untagged ports 2 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS,
            prvTgfPortsArray+2 , NULL, NULL, 1);

    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 55 with untagged ports 0,1 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_NEXTHOP_VLANID_CNS,
            prvTgfPortsArray, NULL, NULL, 3);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* save default vlanId for restore */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            &prvTgfRestoreCfg.vid);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d",
            prvTgfDevNum, &prvTgfRestoreCfg.vid);

    /* AUTODOC: set PVID 80 for port 2 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

}


/**
 * @internal prvTgfTunnelStartEVIESITunnelConfigurationSet function
 * @endinternal
 *
 * @brief   Set Tunnel Configuration
 *
 */
static GT_VOID prvTgfTunnelStartEVIESITunnelConfigurationSet
(
        GT_VOID
)
{

    GT_STATUS                               rc = GT_OK;
    CPSS_TUNNEL_TYPE_ENT                    tunnelType;
    PRV_TGF_TUNNEL_START_ENTRY_UNT          tunnelEntry;
    PRV_TGF_TUNNEL_START_GEN_IP_PROFILE_STC profileDataPtr;
    GT_U32                                  profileIndex = 7;
    GT_U32   prvTgfTunnelStartLineIndex = prvTgfTunnelStartEntry2BaseIndex;
    GT_U32                                   i;
    /*GT_U32                                   regAddr;*/

    /* Set tunnel 1 configuration */
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));
    tunnelType = CPSS_TUNNEL_GENERIC_IPV6_E;

    tunnelEntry.ipv6Cfg.tagEnable           = GT_TRUE;
    tunnelEntry.ipv6Cfg.vlanId              = 0x7;
    cpssOsMemCpy(tunnelEntry.ipv6Cfg.macDa.arEther,
            prvTgfTunnelMacDa, sizeof(TGF_MAC_ADDR));

    tunnelEntry.ipv6Cfg.ttl                 = 0x33;

    cpssOsMemCpy(tunnelEntry.ipv6Cfg.destIp.arIP,
            prvTgfTunnelDstIp,sizeof(prvTgfTunnelDstIp));
    cpssOsMemCpy(tunnelEntry.ipv6Cfg.srcIp.arIP,
            prvTgfTunnelSrcIp,sizeof(prvTgfTunnelSrcIp));


    tunnelEntry.ipv6Cfg.ipHeaderProtocol    =
            CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
    tunnelEntry.ipv6Cfg.profileIndex        = 7;
    tunnelEntry.ipv6Cfg.greFlagsAndVersion  = 7;

    tunnelEntry.ipv6Cfg.udpDstPort          = 0x45;
    tunnelEntry.ipv6Cfg.udpSrcPort          = 0xA1;

    rc = prvTgfTunnelStartEntrySet(prvTgfTunnelStartLineIndex,
            tunnelType, &tunnelEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntrySet");

    /* Set tunnel 2 configuration */
    prvTgfTunnelStartLineIndex = prvTgfTunnelStartEntry1BaseIndex;
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));
    tunnelType = CPSS_TUNNEL_GENERIC_IPV6_E;

    tunnelEntry.ipv6Cfg.tagEnable           = GT_TRUE;
    tunnelEntry.ipv6Cfg.vlanId              = 0x7;
    cpssOsMemCpy(tunnelEntry.ipv6Cfg.macDa.arEther,
            prvTgfTunnelMacDa2, sizeof(TGF_MAC_ADDR));

    tunnelEntry.ipv6Cfg.ttl                 = 0x33;

    cpssOsMemCpy(tunnelEntry.ipv6Cfg.destIp.arIP,
            prvTgfTunnelDstIp2,sizeof(prvTgfTunnelDstIp));
    cpssOsMemCpy(tunnelEntry.ipv6Cfg.srcIp.arIP,
            prvTgfTunnelSrcIp2,sizeof(prvTgfTunnelSrcIp));


    tunnelEntry.ipv6Cfg.ipHeaderProtocol    =
            CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
    tunnelEntry.ipv6Cfg.profileIndex        = 7;
    tunnelEntry.ipv6Cfg.greFlagsAndVersion  = 7;

    tunnelEntry.ipv6Cfg.udpDstPort          = 0x45;
    tunnelEntry.ipv6Cfg.udpSrcPort          = 0xA1;

    rc = prvTgfTunnelStartEntrySet(prvTgfTunnelStartLineIndex,
            tunnelType, &tunnelEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntrySet");

    /* Set profile data */
    cpssOsMemSet(&profileDataPtr, 0, sizeof(profileDataPtr));

    profileDataPtr.templateDataSize =
            PRV_TGF_TUNNEL_START_TEMPLATE_DATA_SIZE_16B_E;
    profileDataPtr.controlWordIndex = 1;
    for( i=0; i<8; i++) {
        profileDataPtr.templateDataBitsCfg[32+i] = 8;
        profileDataPtr.templateDataBitsCfg[40+i] = 7;
        profileDataPtr.templateDataBitsCfg[48+i] = 6;
    }
    profileDataPtr.templateDataBitsCfg[64] = 1;
    profileDataPtr.templateDataBitsCfg[72] = 1;
    profileDataPtr.templateDataBitsCfg[80] = 1;
    profileDataPtr.templateDataBitsCfg[88] = 1;
    profileDataPtr.templateDataBitsCfg[96] = 1;
    profileDataPtr.templateDataBitsCfg[104] = 1;
    profileDataPtr.templateDataBitsCfg[112] = 1;
    profileDataPtr.templateDataBitsCfg[120] = 1;

    rc = prvTgfTunnelStartGenProfileTableEntrySet(prvTgfDevNum,
            profileIndex,&profileDataPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "prvTgfTunnelStartGenProfileTableEntrySet");


    /* Store default evlan ttl value for restore */
    rc = cpssDxChTunnelStartMplsEVlanLabelTtlGet(prvTgfDevNum,
            &prvTgfRestoreCfg.eVlanTtl);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssDxChTunnelStartMplsEVlanLabelTtlGet");
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfRestoreCfg.eVlanTtl,0xFF,
            "cpssDxChTunnelStartMplsEVlanLabelTtlGet");

    /* Set evlan ttl value to 0x3F */
    rc = cpssDxChTunnelStartMplsEVlanLabelTtlSet(prvTgfDevNum, 0x27);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssDxChTunnelStartMplsEVlanLabelTtlSet");

    /* Set ESI and EVI enable for port 0 */
    rc = cpssDxChTunnelStartMplsPushSourceBasedLabelEnableSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_DEF_RCV_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssDxChTunnelStartMplsPushSourceBasedLabelEnableSet");
    rc = cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_DEF_RCV_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableSet");


    /* Set ESI and EVI enable for Eport1 */
    rc = cpssDxChTunnelStartMplsPushSourceBasedLabelEnableSet(prvTgfDevNum,
            PRV_TGF_E_PORT1_IDX_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssDxChTunnelStartMplsPushSourceBasedLabelEnableSet");
    rc = cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableSet(prvTgfDevNum,
            PRV_TGF_E_PORT1_IDX_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableSet");

    /* Set ESI label for send port 2 */
    rc = cpssDxChTunnelStartMplsPwLabelSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], 0x23);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssDxChTunnelStartMplsPwLabelSet");

    /* Set ESI exp value for send port 2 */
    rc = cpssDxChTunnelStartMplsPwLabelExpSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], 0x1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssDxChTunnelStartMplsPwLabelExpSet");

    /* Set ESI ttl for send port 2 */
    rc = cpssDxChTunnelStartMplsPwLabelTtlSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], 0x28);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssDxChTunnelStartMplsPwLabelTtlSet");

    /* Store control word for restore */
    rc = cpssDxChTunnelStartMplsPwControlWordGet(prvTgfDevNum,
            0x1, &prvTgfRestoreCfg.controlWord);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssDxChTunnelStartMplsPwControlWordGet");

    /* Set control word */
    rc = cpssDxChTunnelStartMplsPwControlWordSet(prvTgfDevNum,
            0x1, 0x6789afaa);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssDxChTunnelStartMplsPwControlWordSet");

    /* Set service id for send and nexthop vlans*/
    rc = cpssDxChTunnelStartEgessVlanTableServiceIdSet(prvTgfDevNum,
            PRV_TGF_SEND_VLANID_CNS, 0x366666);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssDxChTunnelStartEgessVlanTableServiceIdSet");
    rc = cpssDxChTunnelStartEgessVlanTableServiceIdSet(prvTgfDevNum,
            PRV_TGF_NEXTHOP_VLANID_CNS, 0x355555);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssDxChTunnelStartEgessVlanTableServiceIdSet");

}


/**
* @internal prvTgfTunnelStartEVIESIEPortConfigurationSet function
* @endinternal
*
* @brief   Set EPort Configuration
*
*/
static GT_VOID prvTgfTunnelStartEVIESIEPortConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    CPSS_INTERFACE_INFO_STC                 physicalInfo;
    CPSS_INTERFACE_INFO_STC                 physicalInfo1;
    CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC      egressEportInfo;
    CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC      egressEportInfo1;

    /* set physical to eport mapping for 1st eport */
    cpssOsMemSet(&physicalInfo, 0, sizeof(physicalInfo));

    physicalInfo.devPort.hwDevNum = prvTgfDevNum;
    physicalInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_RCV_PORT_IDX_CNS];
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
            PRV_TGF_E_PORT1_IDX_CNS,&physicalInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "cpssDxChBrgEportToPhysicalPortTargetMappingTableSet");

    /* set egress mode for the eport1*/
    cpssOsMemSet(&egressEportInfo,0,sizeof(egressEportInfo));

    egressEportInfo.tunnelStart = GT_TRUE;
    egressEportInfo.tunnelStartPtr = prvTgfTunnelStartEntry1BaseIndex;

    rc = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
            PRV_TGF_E_PORT1_IDX_CNS,&egressEportInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgEportToPhysicalPortEgressPortInfoSet");

    /* set physical to eport mapping for 2nd eport */
    cpssOsMemSet(&physicalInfo1, 0, sizeof(physicalInfo1));
    physicalInfo1.devPort.hwDevNum = prvTgfDevNum;
    physicalInfo1.devPort.portNum = prvTgfPortsArray[PRV_TGF_DEF_RCV_PORT_IDX_CNS];
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
            PRV_TGF_E_PORT2_IDX_CNS,&physicalInfo1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "cpssDxChBrgEportToPhysicalPortTargetMappingTableSet");

    /* set egress mode for the eport2*/
    cpssOsMemSet(&egressEportInfo1,0,sizeof(egressEportInfo1));

    egressEportInfo1.tunnelStart = GT_TRUE;
    egressEportInfo1.tunnelStartPtr = prvTgfTunnelStartEntry2BaseIndex;

    rc = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
            PRV_TGF_E_PORT2_IDX_CNS,&egressEportInfo1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgEportToPhysicalPortEgressPortInfoSet");

}

/**
 * @internal prvTgfTunnelStartEVIESIDefaultLttRouteConfigurationSet function
 * @endinternal
 *
 * @brief   Set LTT Route Configuration
 */
static GT_VOID prvTgfTunnelStartEVIESIDefaultLttRouteConfigurationSet
(
        GT_VOID
)
{
    GT_STATUS                       rc = GT_OK;
    PRV_TGF_IP_MLL_PAIR_STC         mllPairEntry;
    PRV_TGF_IP_MC_ROUTE_ENTRY_STC   mcRouteEntryArray[1];
    CPSS_IP_PROTOCOL_STACK_ENT      protocol = CPSS_IP_PROTOCOL_IPV4_E;
    PRV_TGF_IP_LTT_ENTRY_STC        mcRouteLttEntry;
    GT_U32                          ipGroupPrefix = 0;
    GT_U32                          ipSrcPrefix = 0;
    GT_BOOL                         override = GT_FALSE;
    GT_IPADDR                       ipGrp, ipSrc;

    /* AUTODOC: SETUP ROUTE CONFIGURATION: */

    /* AUTODOC: enable IPv4 Multicast Routing on port 2 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS,
            CPSS_IP_MULTICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: enable IPv4 Multicast Routing on Vlan 80 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS,
            CPSS_IP_MULTICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
            prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);


    /* Save L2 MLL configurations */
    prvTgfL2MllUtilsGlobalConfigurationGetAndSave();

    /* AUTODOC: create MC Link List with index 0, port 3, VLAN 80 */
    cpssOsMemSet(&mllPairEntry, 0, sizeof(mllPairEntry));

    mllPairEntry.firstMllNode.mllRPFFailCommand                = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mllPairEntry.firstMllNode.nextHopInterface.type            = CPSS_INTERFACE_PORT_E;
    mllPairEntry.firstMllNode.nextHopInterface.devPort.hwDevNum  = prvTgfDevNum;
    mllPairEntry.firstMllNode.nextHopInterface.devPort.portNum = PRV_TGF_E_PORT1_IDX_CNS;
    mllPairEntry.firstMllNode.nextHopVlanId                    = PRV_TGF_NEXTHOP_VLANID_CNS;
    mllPairEntry.firstMllNode.ttlHopLimitThreshold             = 4;
    mllPairEntry.firstMllNode.isTunnelStart                    = GT_FALSE;
    mllPairEntry.firstMllNode.last                             = GT_FALSE;

    mllPairEntry.secondMllNode.mllRPFFailCommand                = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mllPairEntry.secondMllNode.nextHopInterface.type            = CPSS_INTERFACE_PORT_E;
    mllPairEntry.secondMllNode.nextHopInterface.devPort.hwDevNum  = prvTgfDevNum;
    mllPairEntry.secondMllNode.nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_DEF_RCV_PORT_IDX_CNS];
    mllPairEntry.secondMllNode.nextHopVlanId                    = PRV_TGF_SEND_VLANID_CNS;
    mllPairEntry.secondMllNode.ttlHopLimitThreshold             = 4;
    mllPairEntry.secondMllNode.last                             = GT_TRUE;
    mllPairEntry.secondMllNode.isTunnelStart                    = GT_TRUE;
    mllPairEntry.secondMllNode.nextHopTunnelPointer             = prvTgfTunnelStartEntry1BaseIndex;
    mllPairEntry.nextPointer                     = 0;


    rc = prvTgfIpMllPairWrite(0,
            PRV_TGF_PAIR_READ_WRITE_WHOLE_E,
            &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMllPairWrite");

    /* create a MC Route entry */
    cpssOsMemSet(&mcRouteEntryArray[0], 0, sizeof(mcRouteEntryArray[0]));
    mcRouteEntryArray[0].cmd                = CPSS_PACKET_CMD_ROUTE_E;
    mcRouteEntryArray[0].countSet           = CPSS_IP_CNT_SET2_E;
    mcRouteEntryArray[0].RPFFailCommand     = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    mcRouteEntryArray[0].ipv4MllPtr = 0;

    /* AUTODOC: add default MC route entry with index 1 */
    rc = prvTgfIpMcRouteEntriesWrite(prvTgfDefRouteEntryIndex, 1, protocol, mcRouteEntryArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpMcRouteEntriesWrite");


    /* fill a nexthope info for the LPM */
    cpssOsMemSet(&mcRouteLttEntry, 0, sizeof(mcRouteLttEntry));
    mcRouteLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    mcRouteLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    ipGroupPrefix = 32;
    ipSrcPrefix = 32;
    override = GT_FALSE;

    cpssOsMemCpy(ipGrp.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipGrp.arIP));
    cpssOsMemCpy(ipSrc.arIP, prvTgfSrcAddrArr[0], sizeof(ipSrc.arIP));

    /* AUTODOC: add IPv4 MC prefix: */
    /* AUTODOC:   ipGrp=224.1.1.1/32, ipSrc=1.1.1.1/32 */
    rc = prvTgfIpLpmIpv4McEntryAdd(prvTgfLpmDBId, 0, ipGrp, ipGroupPrefix, ipSrc, ipSrcPrefix,
            &mcRouteLttEntry, override, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4McEntryAdd: %d", prvTgfDevNum);

}

/**
 * @internal prvTgfTunnelStartEVIESIDefaultTrafficGenerate function
 * @endinternal
 *
 * @brief   Generate traffic to defaults
 */

static GT_VOID prvTgfTunnelStartEVIESIDefaultTrafficGenerate
(
        GT_VOID
)
{
    GT_STATUS                   rc        = GT_OK;
    CPSS_INTERFACE_INFO_STC     portInterface;
    CPSS_INTERFACE_INFO_STC     portInterface1;
    GT_U32                      portNum = 0;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* setup receive portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[portNum];

    /* enable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "tgftrafficgeneratorporttxethcaptureset");

    /* setup receive portInterface for capturing */
    portInterface1.type            = CPSS_INTERFACE_PORT_E;
    portInterface1.devPort.hwDevNum  = prvTgfDevNum;
    portInterface1.devPort.portNum = prvTgfPortsArray[1];

    /* enable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &portInterface1, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "tgftrafficgeneratorporttxethcaptureset");


    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

    /* AUTODOC: send packet from port 2 with: */
    /* AUTODOC:   DA=01:00:5e:01:01:01, SA=00:00:00:00:00:33 */
    /* AUTODOC:   VID=80, srcIP=1.1.1.1, dstIP=224.1.1.1 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);


    /* disable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "tgfTrafficGeneratorPortTxEthCaptureSet");

    /* disable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &portInterface1, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "tgfTrafficGeneratorPortTxEthCaptureSet");

    /* print captured packet */
    rc = prvTgfPortCapturedPacketPrint(prvTgfDevNum, prvTgfPortsArray[portNum]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortCapturedPacketPrint: %d, %d\n",
            prvTgfDevNum, prvTgfPortsArray[portNum]);

    /* print captured packet */
    rc = prvTgfPortCapturedPacketPrint(prvTgfDevNum, prvTgfPortsArray[1]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPortCapturedPacketPrint: %d, %d\n",
            prvTgfDevNum, prvTgfPortsArray[portNum]);

}

/**
 * @internal prvTgfTunnelStartEVIESIAdditionalTrafficGenerate function
 * @endinternal
 *
 * @brief   Generate traffic
 *
 * @param[in] testNumber               - number of test
 *                                       None
 */
static GT_VOID prvTgfTunnelStartEVIESIAdditionalTrafficGenerate
(
       GT_VOID
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U8                           portIter    = 0;
    GT_U32                          sendIter    = 0;
    GT_U32                          packetLen   = 0;
    GT_U32                          sendPacketsNumber = 1 ; /*PRV_TGF_SEND_PACKETS_NUM_CNS;*/

      /* AUTODOC: GENERATE TRAFFIC: */
    for (sendIter = 0; sendIter < sendPacketsNumber; sendIter++)
    {
        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

        /* clear fdb*/
        rc = prvTgfBrgFdbFlush(GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush");

        /* stop capturing on all ports */
        rc = tgfTrafficTableRxStartCapture(GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCpature");

        /* reset ETH counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");


        /* fill source IP address for packet */
        cpssOsMemCpy(prvTgfPacketIpv4Part.srcAddr, prvTgfSrcAddrArr[sendIter],
                sizeof(prvTgfPacketIpv4Part.srcAddr));


        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

        /* AUTODOC: send 2 IPv4 packets from port 2 with: */
        /* AUTODOC:   DA=01:00:5E:01:01:01, SA=00:00:00:00:00:33 */
        /* AUTODOC:   VID=80, srcIP=[1.1.1.1,2.2.2.2] dstIP=224.1.1.1 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* check ETH counter of ports for MC subscriber */
        for (portIter = 0; portIter < prvTgfPortsNum-1; portIter++)
        {
            /* calculate packet length */
            if(portIter == 2)
            {
                packetLen = PRV_TGF_PACKET_LEN_CNS;
            }
            else
            {
                packetLen = 154 ;
            }

            /* AUTODOC:   1 packet - received on ports 1,3 */
            rc = prvTgfEthCountersCheck(prvTgfDevNum,
                    prvTgfPortsArray[portIter],
                    ((portIter==2)? 1 : 0),
                    prvTgfPacketsCountRxTxArr[sendIter][portIter],
                    packetLen, prvTgfBurstCount);

            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersCheck");

        }

    }
}

/**
 * @internal prvTgfTunnelStartEVIESIConfigurationRestore function
 * @endinternal
 *
 * @brief   Restore configuration
 */
static GT_VOID prvTgfTunnelStartEVIESIConfigurationRestore
(
        GT_VOID
)
{
    GT_STATUS                               rc       = GT_OK;
    GT_U32                                  lttIndexArray[1];
    GT_U32                                  mllIndexArray[1];
    PRV_TGF_TUNNEL_START_ENTRY_UNT          tunnelEntry;
    /*GT_U32                                  regAddr;*/
    CPSS_INTERFACE_INFO_STC                 physicalInfo;
    CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC      egressEportInfo;

    /* AUTODOC: RESTORE CONFIGURATION: */


    /* Restore Base Configuration */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgFdbFlush: %d", prvTgfDevNum);

    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfRestoreCfg.vid);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d",
            prvTgfDevNum, prvTgfRestoreCfg.vid);

    /* invalidate SEND VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryInvalidate: %d", PRV_TGF_SEND_VLANID_CNS);

    /* invalidate nexthop VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_NEXTHOP_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryInvalidate: %d",PRV_TGF_NEXTHOP_VLANID_CNS);

    /* Restore EPort related settings */

    /* Reset physical info table for eport 1*/
    cpssOsMemSet(&physicalInfo, 0, sizeof(physicalInfo));
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
            PRV_TGF_E_PORT1_IDX_CNS,&physicalInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgEportToPhysicalPortTargetMappingTableSet");

    /* Reset physical info table for eport 2*/
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
            PRV_TGF_E_PORT2_IDX_CNS,&physicalInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgEportToPhysicalPortTargetMappingTableSet");

    /* Reset egress mode for eport1*/
    cpssOsMemSet(&egressEportInfo,0,sizeof(egressEportInfo));
    rc = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
            PRV_TGF_E_PORT1_IDX_CNS,&egressEportInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgEportToPhysicalPortEgressPortInfoSet");

    /* Reset egress mode for eport2*/
    cpssOsMemSet(&egressEportInfo,0,sizeof(egressEportInfo));
    rc = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
            PRV_TGF_E_PORT2_IDX_CNS,&egressEportInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssDxChBrgEportToPhysicalPortEgressPortInfoSet");

    /* Restore Tunnel and MLL related settings */

    lttIndexArray[0] = prvTgfRouteEntryBaseIndex;
    mllIndexArray[0] = 0;

    cpssOsMemSet((GT_VOID*) &tunnelEntry, 0, sizeof(tunnelEntry));
    /* AUTODOC: clear L2 MLL & LTT entries */
    prvTgfL2MllUtilsMllAndLttEntriesClear(lttIndexArray, 1, mllIndexArray, 1);

    /* Restore L2 MLL configurations */
    prvTgfL2MllUtilsGlobalConfigurationRestore();

    /* AUTODOC: restore tunnel start entries [0x14,0x28] */
    rc = prvTgfTunnelStartEntrySet(prvTgfTunnelStartEntry1BaseIndex,
            CPSS_TUNNEL_GENERIC_IPV6_E, &tunnelEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntrySet");

    rc = prvTgfTunnelStartEntrySet(prvTgfTunnelStartEntry2BaseIndex,
            CPSS_TUNNEL_GENERIC_IPV6_E, &tunnelEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntrySet");


    /* flush all MC Ipv4 prefix */
    rc = prvTgfIpLpmIpv4McEntriesFlush(prvTgfLpmDBId, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4McEntriesFlush");

    /* AUTODOC: disable multicast IPv4 routing on port 2 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS,
            CPSS_IP_MULTICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable");

    /* AUTODOC: disable multicast IPv4 routing on Vlan 80 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_MULTICAST_E,
            CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable");

    /* Restore default evlan ttl */
    rc = cpssDxChTunnelStartMplsEVlanLabelTtlSet(prvTgfDevNum,
            prvTgfRestoreCfg.eVlanTtl);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssDxChTunnelStartMplsEVlanLabelTtlSet");

    /* Restore ESI and EVI enable for port 0 */
    rc = cpssDxChTunnelStartMplsPushSourceBasedLabelEnableSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_DEF_RCV_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssDxChTunnelStartMplsPushSourceBasedLabelEnableSet");
    rc = cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_DEF_RCV_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableSet");


    /* Restore ESI and EVI enable for Eport1 */
    rc = cpssDxChTunnelStartMplsPushSourceBasedLabelEnableSet(prvTgfDevNum,
            PRV_TGF_E_PORT1_IDX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssDxChTunnelStartMplsPushSourceBasedLabelEnableSet");
    rc = cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableSet(prvTgfDevNum,
            PRV_TGF_E_PORT1_IDX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssDxChTunnelStartMplsPushEVlanBasedLabelEnableSet");

    /* Restore ESI label for send port 2 */
    rc = cpssDxChTunnelStartMplsPwLabelSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssDxChTunnelStartMplsPwLabelSet");

    /* Restore ESI exp value for send port 2 */
    rc = cpssDxChTunnelStartMplsPwLabelExpSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssDxChTunnelStartMplsPwLabelExpSet");

    /* Restore ESI ttl for send port 2 */
    rc = cpssDxChTunnelStartMplsPwLabelTtlSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssDxChTunnelStartMplsPwLabelTtlSet");

    /* Store control word for restore */
    rc = cpssDxChTunnelStartMplsPwControlWordSet(prvTgfDevNum,
            0x1, prvTgfRestoreCfg.controlWord);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssDxChTunnelStartMplsPwControlWordSet");

    /* Reset the service id for send and nexthop vlans*/
    rc = cpssDxChTunnelStartEgessVlanTableServiceIdSet(prvTgfDevNum,
            PRV_TGF_SEND_VLANID_CNS, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssDxChTunnelStartEgessVlanTableServiceIdSet");
    rc = cpssDxChTunnelStartEgessVlanTableServiceIdSet(prvTgfDevNum,
            PRV_TGF_NEXTHOP_VLANID_CNS, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "cpssDxChTunnelStartEgessVlanTableServiceIdSet");

}

/**
* @internal prvTgfTunnelStartEVIESIConfigurationSet function
* @endinternal
*
* @brief   Set base Configuration
*
*/
GT_VOID prvTgfTunnelStartEVIESIConfigurationSet
(
    GT_VOID
)
{
    prvTgfTunnelStartEVIESIBaseConfigurationSet();
    prvTgfTunnelStartEVIESITunnelConfigurationSet();
    prvTgfTunnelStartEVIESIEPortConfigurationSet();
    prvTgfTunnelStartEVIESIDefaultLttRouteConfigurationSet();
}


/**
* @internal prvTgfTunnelStartEVIESITrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfTunnelStartEVIESITrafficGenerate
(
    GT_VOID
)
{
    prvTgfTunnelStartEVIESIDefaultTrafficGenerate();
    prvTgfTunnelStartEVIESIAdditionalTrafficGenerate();
}

/**
* @internal prvTgfTunnelStartEVIESIConfigurationReset function
* @endinternal
*
* @brief   Reset base Configuration
*
*/
GT_VOID prvTgfTunnelStartEVIESIConfigurationReset
(
    GT_VOID
)
{
    prvTgfTunnelStartEVIESIConfigurationRestore();
}




