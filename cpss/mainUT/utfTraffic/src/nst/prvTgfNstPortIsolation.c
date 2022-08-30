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
* @file prvTgfNstPortIsolation.c
*
* @brief Port Isolation
*
* @version   39
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfNstGen.h>
#include <common/tgfTrunkGen.h>
#include <nst/prvTgfNstPortIsolation.h>
#include <cpss/dxCh/dxChxGen/nst/cpssDxChNstPortIsolation.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgStp.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgVlan.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <common/tgfNetIfGen.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/nst/cpssDxChNst.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* the isolated bit location in the Src-ID */
#define PRV_TGF_ISOLATED_BIT_CNS    5

#define PRV_TGF_E_VLANID_BASE_CNS    _4K
/* extended VLAN Id (for vlan > 4K) */
#define PRV_TGF_E_VLANID_CNS    (PRV_TGF_E_VLANID_BASE_CNS + (1000 % (UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(prvTgfDevNum)-PRV_TGF_E_VLANID_BASE_CNS)))

/* default VLAN0 Id */
#define PRV_TGF_VLANID_CNS  5

/* nextHop VLAN Id */
#define PRV_TGF_NEXTHOPE_VLANID_CNS   6

/* default Trunk Id */
#define PRV_TGF_TRUNK_ID_CNS  1

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         1

/* another port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS 0

/* isolated port number in VLAN */
#define PRV_TGF_ISOLATED_PORT_IDX_CNS 2

/* count of packets to send */
#define PRV_TGF_SEND_COUNT_CNS 2

/* STG id */
#define PRV_TGF_STG_ID_CNS 0

/* router sourceId value */
#define PRV_TGF_ROUTER_SOURCE_ID_CNS    0x20

/* router sourceId mask */
#define PRV_TGF_ROUTER_SOURCE_ID_MASK_CNS 0x20

/* ingress port index */
#define PRV_TGF_INGRESS_PORT_IDX_CNS 1

/* egress port index */
#define PRV_TGF_EGRESS_PORT_IDX_CNS  3

/* vlan id for packet sanity check test */
#define PRV_TGF_NST_VLANID_CNS 479

/* traffic class for trap sanity check packet */
#define PRV_TGF_NST_TC_IDX_CNS 2

/* config restore variables */
static GT_U32 sourceIdMaskOrig;
static GT_U32 sourceIdOrig;

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/* port bitmap VLAN members */
static CPSS_PORTS_BMP_STC localPortsVlanMembers = {{0, 0}};

/* port bitmap L2 Port Isolation Table */
static CPSS_PORTS_BMP_STC localPortsMembers;

/* port bitmap L2 and L3 Port Isolation Table */
static GT_U32 localL2L3PortsIdxMembers[] = {1, 2};

/* array of trunk members */
static CPSS_TRUNK_MEMBER_STC   trunkMembersArray[2] = {{0, 0}, {0, 0}};

/* type of traffic for port isolation */
static PRV_TGF_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT portIsolationTraffic = PRV_TGF_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E;

/* port isolation command of VLAN entry */
static PRV_TGF_BRG_VLAN_PORT_ISOLATION_CMD_ENT portIsolationCmd = PRV_TGF_BRG_VLAN_PORT_ISOLATION_L2_CMD_E;

/* the Arp Address indexes of the Router ARP Table
   to write to the UC Route entry Arp nextHopARPPointer field */
static GT_U32        prvTgfRouterArpIndexArray[]      = {1, 5};

/* the Route entry index for UC Route entry Table */
static GT_U32        prvTgfRouteEntryBaseIndexArray[] = {3, 5};

/* nextHop port numbers to receive traffic from */
static GT_U8         prvTgfNextHopePortIdxArray[]     = {2, 3};

/* the ARP MAC addresses to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfArpMacArray[] = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x22},
                                            {0x00, 0x00, 0x00, 0x00, 0x00, 0x33}};

/* the dst ip addresses for test packets */
static TGF_IPV4_ADDR prvTgfIpv4AddrArray[] = {{ 1,  1,  1,  3}, { 1,  2,  3,  4}};

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId       = 0;

/* original indexing mode and span states of isolated port in Vlan isolation test */
static CPSS_STP_STATE_ENT spanStateSendPort;
static CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_ENT originalInedxingMode;
static GT_U16        prvTgfDefVlanId = 0;
static GT_U32        numOfVlans;

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}                /* srcMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};


/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x55, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/*************************** Test packet Ipv4 *********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketIpv4L2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* saMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketIpv4VlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketTcpVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_NST_VLANID_CNS                        /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv4EtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

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
    0x06,/*TCP*/        /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    { 1,  1,  1,  1},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* DATA of packet */
static GT_U8 prvTgfPacketIpv4PayloadDataArr[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketIpv4PayloadPart = {
    sizeof(prvTgfPacketIpv4PayloadDataArr),             /* dataLength */
    prvTgfPacketIpv4PayloadDataArr                      /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketIpv4PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketIpv4L2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketIpv4VlanTag0Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketIpv4PayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_IPV4_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + \
    TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPacketIpv4PayloadDataArr)

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_IPV4_CRC_LEN_CNS  PRV_TGF_PACKET_IPV4_LEN_CNS + TGF_CRC_LEN_CNS

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketIpv4Info = {
    PRV_TGF_PACKET_IPV4_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfPacketIpv4PartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketIpv4PartArray                                        /* partsArray */
};

/* packet's TCP part */
static TGF_PACKET_TCP_STC prvTgfPacketTcpPart =
{
    0xEE22,   /* src port */
    0xEE22,   /* dst port */
    0,        /* sequence number */
    0,        /* acknowledgment number */
    5,        /* data offset */
    0,        /* reserved */
    0,        /* flags */
    0,        /* window */
    0xE444,   /* csum */
    0         /* urgent pointer */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfTcpPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketIpv4L2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketTcpVlanTag0Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_TCP_E,       &prvTgfPacketTcpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketIpv4PayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_TCP_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + \
    TGF_IPV4_HEADER_SIZE_CNS + TGF_TCP_HEADER_SIZE_CNS + sizeof(prvTgfPacketIpv4PayloadDataArr)

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketTcpInfo = {
    PRV_TGF_PACKET_TCP_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfTcpPacketPartArray) / sizeof(TGF_PACKET_PART_STC),  /* numOfParts */
    prvTgfTcpPacketPartArray                                         /* partsArray */
};

/* parameters that is needed to be restored */
static struct
{
    GT_U32                                          tcIngressTrapQueueGet;
    PRV_TGF_NET_CPU_CODE_TABLE_ENTRY_STC            ingressTrapCpuEntryGet;
    CPSS_PACKET_CMD_ENT                             packetCmd;
} prvTgfRestoreCfg;

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

static void prvTgfHwDevNumSet(IN GT_HW_DEV_NUM hwDevNum)
{
    GT_STATUS   rc;
    CPSS_INTERFACE_INFO_STC physicalInfo;
    GT_U32   portIter;

    rc = prvUtfHwDeviceNumberSet(prvTgfDevNum,hwDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberSet: %d", hwDevNum);

    if(!PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        /* no E2PHY table to update */
        return;
    }

    for(portIter = 0 ; portIter < prvTgfPortsNum ; portIter++)
    {
        rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(
            prvTgfDevNum,prvTgfPortsArray[portIter],&physicalInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgEportToPhysicalPortTargetMappingTableGet");

        if(physicalInfo.type == CPSS_INTERFACE_PORT_E)
        {
            physicalInfo.devPort.hwDevNum = hwDevNum;

            rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(
                prvTgfDevNum,prvTgfPortsArray[portIter],&physicalInfo);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgEportToPhysicalPortTargetMappingTableSet: %d", hwDevNum);
        }
    }
}

static GT_HW_DEV_NUM    origHwDevNum;

/*
   value of limitHwDevNum == GT_NA --> restore.
*/
void prvTgfLimitedHwDevNumSet(IN GT_U32  limitHwDevNum)
{
    GT_STATUS   rc;
    GT_HW_DEV_NUM    limitedRandomHwDevNum;

    if(limitHwDevNum == GT_NA)
    {
        prvTgfHwDevNumSet(origHwDevNum);
        return;
    }

    rc = prvUtfHwDeviceNumberGet(prvTgfDevNum,&origHwDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberGet:");

    if(origHwDevNum <= limitHwDevNum)
    {
        /* do not random on the legacy runs */
        return;
    }

    limitedRandomHwDevNum = prvUtfSeedFromStreamNameGet() % (limitHwDevNum-1);/* 0 .. 0x10 */

    prvTgfHwDevNumSet(limitedRandomHwDevNum);

    return;
}

/* for the mode that run with random devNum that lead to HwDevNum = devNum + 0x10 :
   the port isolation 2K+128 entries ,
   must be limited to HwDevNum of 5 bits of HW devNum in range 0x0..0x10
   and 7 bits of portNum

   and even with HwDevNum 0x10 it getting to 'trunk entries' range.
*/
static void portIsolationLimitedHwDevNumSet(void)
{
    prvTgfLimitedHwDevNumSet(0x10);
}
static void portIsolationLimitedHwDevNumRestore(void)
{
    prvTgfLimitedHwDevNumSet(GT_NA);
}

/**
* @internal prvTgfNstPortIsolationTestInit function
* @endinternal
*
* @brief   Set VLAN entry with recieved port isolation command.
*
* @param[in] vlanId                   -  to be configured
* @param[in] portIsolationCmd         - port isolation command
* @param[in] portsMembers             - VLAN's ports Members
*                                       None
*/
static GT_VOID prvTgfNstPortIsolationTestInit
(
    IN GT_U16                                  vlanId,
    IN PRV_TGF_BRG_VLAN_PORT_ISOLATION_CMD_ENT portIsolationCmd,
    CPSS_PORTS_BMP_STC                         portsMembers
)
{
    GT_STATUS                   rc = GT_OK;
    CPSS_PORTS_BMP_STC          portsTagging;
    PRV_TGF_BRG_VLAN_INFO_STC   vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;

    /* clear entry */
    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC));
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    /* set vlan entry */
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
    vlanInfo.autoLearnDisable     = GT_TRUE;/* working in controlled learning */
    vlanInfo.naMsgToCpuEn         = GT_TRUE;/* working in controlled learning */
    vlanInfo.mruIdx               = 0;
    vlanInfo.bcastUdpTrapMirrEn   = GT_FALSE;
    vlanInfo.floodVidx            = 0xFFF;
    vlanInfo.floodVidxMode        = PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;
    vlanInfo.portIsolationMode    = portIsolationCmd;
    vlanInfo.ucastLocalSwitchingEn = GT_FALSE;
    vlanInfo.mcastLocalSwitchingEn = GT_FALSE;

    /* set vlan entry */
    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum,
                                 vlanId,
                                 &portsMembers,
                                 &portsTagging,
                                 &vlanInfo,
                                 &portsTaggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d", vlanId);
}

/**
* @internal prvTgfNstPortIsolationTestPacketSend function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*
* @param[in] portNum                  - port number to send traffic from
* @param[in] packetInfoPtr            - PACKET to send
*                                       None
*/
static GT_VOID prvTgfNstPortIsolationTestPacketSend
(
    IN GT_U32           portNum,
    IN TGF_PACKET_STC *packetInfoPtr
)
{
    GT_STATUS       rc           = GT_OK;

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the ethertype
      of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, NULL);

    /* send Packet from port portNum */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, portNum);

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}

/**
* @internal prvTgfNstPortIsolationTestTableReset function
* @endinternal
*
* @brief   Function clears FDB, VLAN tables and internal table of captured packets.
*
* @param[in] vlanId                   -  to be cleared
*                                       None
*/
static GT_VOID prvTgfNstPortIsolationTestTableReset
(
    IN GT_U16 vlanId
)
{
    GT_STATUS   rc = GT_OK;
    CPSS_INTERFACE_INFO_STC srcInterface = UTF_INTERFACE_INIT_VAL_CNS;
    GT_U32      portIter;
    PRV_TGF_IP_ROUTING_MODE_ENT routingMode;
    /* bitmap exiting ports in the port isolation . LIMITED to 256 ports only ! */
    CPSS_PORTS_BMP_STC portIsolation_existingPorts;
    GT_U32  ii;
    GT_U32 isolationEntrySize = ( PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum) ? 4:8); /* sip6 isolation entry size is only 128 ports*/

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

    if (routingMode == PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E)
    {
        /* AUTODOC: disable Unicast IPv4 Routing on a Port */
        rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS, CPSS_IP_UNICAST_E,
                                       CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS]);
    }

    /* AUTODOC: disable IPv4 Unicast Routing on Vlan */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS]);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(vlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, vlanId);

    srcInterface.type = CPSS_INTERFACE_PORT_E;
    srcInterface.devPort.hwDevNum = prvTgfDevNum;

    cpssOsMemSet(&portIsolation_existingPorts,0,sizeof(portIsolation_existingPorts));
    for( ii = 0 ; ii < isolationEntrySize ; ii++ )
    {
        /* copy up to 256 ports */
        portIsolation_existingPorts.ports[ii] = PRV_CPSS_PP_MAC(prvTgfDevNum)->existingPorts.ports[ii];
    }

    /* AUTODOC: clear port isolation entries */
    for( portIter = 0 ; portIter < 4 ; portIter++ )
    {
        srcInterface.devPort.portNum = prvTgfPortsArray[portIter];

        /* need to remove casting and to fix code after lion2 development is done*/
        CPSS_TBD_BOOKMARK_EARCH

        rc = prvTgfNstPortIsolationTableEntrySet((GT_U8)srcInterface.devPort.hwDevNum,
                                                 PRV_TGF_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E,
                                                 srcInterface,
                                                 GT_FALSE,
                                                 &portIsolation_existingPorts);

        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfNstPortIsolationTableEntrySet: devNum = %d portNum = %d trafficType = %d",
                                 srcInterface.devPort.hwDevNum, srcInterface.devPort.portNum, PRV_TGF_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E);

        /* need to remove casting and to fix code after lion2 development is done*/
        CPSS_TBD_BOOKMARK_EARCH

        rc = prvTgfNstPortIsolationTableEntrySet((GT_U8)srcInterface.devPort.hwDevNum,
                                                 PRV_TGF_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E,
                                                 srcInterface,
                                                 GT_FALSE,
                                                 &portIsolation_existingPorts);

        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfNstPortIsolationTableEntrySet: devNum = %d portNum = %d trafficType = %d",
                                 srcInterface.devPort.hwDevNum, srcInterface.devPort.portNum, PRV_TGF_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E);
    }

    srcInterface.type    = CPSS_INTERFACE_TRUNK_E;
    srcInterface.trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(srcInterface.trunkId);

    rc = prvTgfNstPortIsolationTableEntrySet(prvTgfDevNum,
                                             portIsolationTraffic,
                                             srcInterface,
                                             GT_FALSE,
                                             &portIsolation_existingPorts);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfNstPortIsolationTableEntrySet: devNum = %d, trafficType = %d",
                                 prvTgfDevNum, portIsolationTraffic);
}

/**
* @internal prvTgfNstPortIsolationUcIpv4LttRoutingSet function
* @endinternal
*
* @brief   Set LTT Configuration
*/
static GT_VOID prvTgfNstPortIsolationUcIpv4LttRoutingSet
(
    IN GT_U32        routerArpIndex,
    IN TGF_MAC_ADDR  routerArpMac,
    IN GT_U32        routeEntryBaseIndex,
    IN TGF_IPV4_ADDR dstIpv4Addr,
    IN GT_U32         nextHopPortNum
)
{
    GT_STATUS                     rc = GT_OK;
    PRV_TGF_BRG_MAC_ENTRY_STC     macEntry;
    GT_ETHERADDR                  arpMacAddr;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC *regularEntryPtr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    GT_IPADDR                     ipAddr;

    PRV_UTF_LOG0_MAC("==== Setting LTT Configuration ====\n");

    /* -------------------------------------------------------------------------
     * 1. Set FDB Table
     */

    /* AUTODOC: enable VLAN based MAC learning on VLAN 5 */
    rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_VLANID_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);

    /* create a macEntry with .daRoute = GT_TRUE */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketIpv4L2Part.daMac, sizeof(TGF_MAC_ADDR));

    macEntry.key.entryType                  = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId         = PRV_TGF_VLANID_CNS;
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum    = prvTgfDevNum;
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

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, Port 1 */
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 2. Enable Routing
     */

    /* AUTODOC: enable UC IPv4 Routing on Port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS]);

    /* AUTODOC: enable IPv4 UC Routing on VLAN 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS]);

     /* for lpmDBId = 0 dev 0 was already added in the init phase,
       so we will get GT_ALREADY_EXIST */

    /* add device to LPM DB */
    rc = prvTgfIpLpmDBDevListAdd(prvTgfLpmDBId, &prvTgfDevNum, 1);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_ALREADY_EXIST, rc, "prvTgfIpLpmDBDevListAdd: %d %d",
                                 prvTgfLpmDBId, prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 3. Create the Route entry (Next hop) in Route table and Router ARP Table
     */

    /* write a ARP MAC address to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, routerArpMac, sizeof(TGF_MAC_ADDR));

    /* AUTODOC: write 2 ARP entries to ARP Table with: */
    /* AUTODOC:   arpIndex=1, MAC=00:00:00:00:00:22  */
    /* AUTODOC:   arpIndex=5, MAC=00:00:00:00:00:33  */
    rc = prvTgfIpRouterArpAddrWrite(routerArpIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

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
    regularEntryPtr->countSet                   = CPSS_IP_CNT_SET0_E;
    regularEntryPtr->trapMirrorArpBcEnable      = GT_FALSE;
    regularEntryPtr->sipAccessLevel             = 0;
    regularEntryPtr->dipAccessLevel             = 0;
    regularEntryPtr->ICMPRedirectEnable         = GT_FALSE;
    regularEntryPtr->scopeCheckingEnable        = GT_FALSE;
    regularEntryPtr->siteId                     = CPSS_IP_SITE_ID_INTERNAL_E;
    regularEntryPtr->mtuProfileIndex            = 0;
    regularEntryPtr->isTunnelStart              = GT_FALSE;
    regularEntryPtr->nextHopVlanId              = PRV_TGF_NEXTHOPE_VLANID_CNS;
    regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
    regularEntryPtr->nextHopInterface.devPort.hwDevNum = prvTgfDevNum;
    regularEntryPtr->nextHopInterface.devPort.portNum = nextHopPortNum;
    regularEntryPtr->nextHopARPPointer          = routerArpIndex;
    regularEntryPtr->nextHopTunnelPointer       = 0;

    /* AUTODOC: add 2 UC route entries with: */
    /* AUTODOC:   index=3, nextHopVlan=6, nextHopPort=2, nextHopArp=1 */
    /* AUTODOC:   index=5, nextHopVlan=6, nextHopPort=3, nextHopArp=5 */
    rc = prvTgfIpUcRouteEntriesWrite(routeEntryBaseIndex, routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 4. Create a new Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = routeEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* fill a destination IP address for the prefix */
    cpssOsMemCpy(ipAddr.arIP, dstIpv4Addr, sizeof(ipAddr.arIP));

    /* AUTODOC: add IPv4 UC prefix 1.1.1.3/32, routeIdx=3 */
    /* AUTODOC: add IPv4 UC prefix 1.2.3.4/32, routeIdx=5 */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, 0, ipAddr, 32, &nextHopInfo, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);
}

/**
* @internal prvTgfNstPortIsolationUcIpv4PbrRoutingSet function
* @endinternal
*
* @brief   Set PBR Configuration
*/
static GT_VOID prvTgfNstPortIsolationUcIpv4PbrRoutingSet
(
    IN GT_U32        routerArpIndex,
    IN TGF_MAC_ADDR  routerArpMac,
    IN TGF_IPV4_ADDR dstIpv4Addr,
    IN GT_U32         nextHopPortNum
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_BRG_MAC_ENTRY_STC               macEntry;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR                               ipAddr;

    PRV_UTF_LOG0_MAC("==== Setting PBR Configuration ====\n");

    /* -------------------------------------------------------------------------
     * 1. Set FDB Table
     */

    /* enable VLAN based MAC learning */
    rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_VLANID_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);

    /* create a macEntry with .daRoute = GT_TRUE */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketIpv4L2Part.daMac, sizeof(TGF_MAC_ADDR));

    macEntry.key.entryType                  = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId         = PRV_TGF_VLANID_CNS;
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum    = prvTgfDevNum;
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

    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 2. PCL Config
     */

    /* init PCL Engine for send port */
    rc = prvTgfPclDefPortInitExt1(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_1_E,/* PBR must be in lookup 1 ! */
        PRV_TGF_PCL_PBR_ID_FOR_VIRTUAL_ROUTER_CNS, /* pclId */
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInitExt1: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 3. IP Config
     */

    /* enable IPv4 Unicast Routing on Vlan */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* write ARP MAC address to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, routerArpMac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfIpRouterArpAddrWrite(routerArpIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 4. Create a new Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));

    nextHopInfo.pclIpUcAction.pktCmd                        = CPSS_PACKET_CMD_FORWARD_E;
    nextHopInfo.pclIpUcAction.mirror.cpuCode                = (CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + 1);
    nextHopInfo.pclIpUcAction.matchCounter.enableMatchCount = GT_TRUE;
    nextHopInfo.pclIpUcAction.vlan.vlanId                   = PRV_TGF_NEXTHOPE_VLANID_CNS;
    nextHopInfo.pclIpUcAction.ipUcRoute.doIpUcRoute         = GT_TRUE;
    nextHopInfo.pclIpUcAction.ipUcRoute.arpDaIndex          = routerArpIndex;

    nextHopInfo.pclIpUcAction.redirect.redirectCmd          = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.type            = CPSS_INTERFACE_PORT_E;
    nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.devPort.hwDevNum  = prvTgfDevNum;
    nextHopInfo.pclIpUcAction.redirect.data.outIf.outInterface.devPort.portNum = nextHopPortNum;

    /* fill a destination IP address for the prefix */
    cpssOsMemCpy(ipAddr.arIP, dstIpv4Addr, sizeof(ipAddr.arIP));

    /* call CPSS function */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, 0, ipAddr, 32, &nextHopInfo, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);
}

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfNstL2PortIsolationConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Configure portIsolationMode on VLAN entry (VID = 5).
*         Enable Port Isolation feature.
*         Set port isolation table entry for L2 with:
*         devNum    = 0
*         trafficType  = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E
*         srcInterface = devNum = 0, portNum = 8
*         cpuPortMember = GT_FALSE
*         localPortsMembersPtr = 0x40001
*/
GT_VOID prvTgfNstL2PortIsolationConfigurationSet
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC srcInterface = UTF_INTERFACE_INIT_VAL_CNS;
    GT_STATUS               rc = GT_OK;
    GT_U32                  portIter = 0;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* for the mode that run with random devNum that lead to HwDevNum = devNum + 0x10 :
       the port isolation 2K+128 entries ,
       must be limited to HwDevNum of 5 bits of HW devNum in range 0x0..0x10
       and 7 bits of portNum

       and even with HwDevNum 0x10 it getting to 'trunk entries' range.
    */
    portIsolationLimitedHwDevNumSet();

    srcInterface.type = CPSS_INTERFACE_PORT_E;
    srcInterface.devPort.hwDevNum = prvTgfDevNum;
    srcInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];

    portIsolationCmd = PRV_TGF_BRG_VLAN_PORT_ISOLATION_L2_CMD_E;

    /* clear entry */
    cpssOsMemSet(&localPortsVlanMembers, 0, sizeof(localPortsVlanMembers));

    /* ports 0, 8, 18, 23 are VLAN Members */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: create VLAN 5 with all untagged ports and L2 isolation mode */
    prvTgfNstPortIsolationTestInit(PRV_TGF_VLANID_CNS, portIsolationCmd, localPortsVlanMembers);

    /* need to remove casting and to fix code after lion2 development is done*/
    CPSS_TBD_BOOKMARK_EARCH

    /* AUTODOC: enable Port Isolation feature */
    rc = prvTgfNstPortIsolationEnableSet((GT_U8)srcInterface.devPort.hwDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfNstPortIsolationEnableSet: %d %d", srcInterface.devPort.hwDevNum, GT_TRUE);

    /* set L2 port isolation table entry */
    portIsolationTraffic = PRV_TGF_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E;

    /* clear entry */
    cpssOsMemSet(&localPortsMembers, 0, sizeof(localPortsMembers));

    /* ports 0, 18 are PortMembers */
    CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsMembers,prvTgfPortsArray[0]);
    CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsMembers,prvTgfPortsArray[2]);

    /* need to remove casting and to fix code after lion2 development is done*/
    CPSS_TBD_BOOKMARK_EARCH

    /* AUTODOC: add PortIsolation entry with ports [0, 2] for L2 packets */
    rc = prvTgfNstPortIsolationTableEntrySet((GT_U8)srcInterface.devPort.hwDevNum,
                                             portIsolationTraffic,
                                             srcInterface,
                                             GT_FALSE,
                                             &localPortsMembers);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfNstPortIsolationTableEntrySet: devNum = %d portNum = %d trafficType = %d",
                                 srcInterface.devPort.hwDevNum, srcInterface.devPort.hwDevNum, portIsolationTraffic);
}

/**
* @internal prvTgfNstL2PortIsolationTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:05,
*         Success Criteria:
*         Packet is captured on ports 0 and 18
*         Additional Configuration:
*         Disable port isolation feature in VLAN entry.
*         Generate Traffic:
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:05,
*         Success Criteria:
*         Packet is captured on ports: 0,18,23
*/
GT_VOID prvTgfNstL2PortIsolationTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       portIter = 0;
    GT_U32       expectedGoodOctetsValue;
    GT_U32       expectedGoodPctsValue;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: send packet from port 1 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:05 */
    prvTgfNstPortIsolationTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

    /* AUTODOC: verify to get packets only on ports [0,2] */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC((PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS) * prvTgfBurstCount, portCntrs.goodOctetsRcv.l[0],
                                         "get another goodOctetsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");

            continue;
        }

        if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(&localPortsMembers,prvTgfPortsArray[portIter]))
        {
            expectedGoodOctetsValue = (PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
            expectedGoodPctsValue = prvTgfBurstCount;
        }
        else
        {
            expectedGoodOctetsValue = 0;
            expectedGoodPctsValue = 0;
        }

        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(expectedGoodOctetsValue, portCntrs.goodOctetsSent.l[0],
                                     "get another goodOctetsSent counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(expectedGoodPctsValue, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: disable port isolation feature on VLAN 5 */
    prvTgfNstPortIsolationTestInit(PRV_TGF_VLANID_CNS,
             PRV_TGF_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E, localPortsVlanMembers);

    /* AUTODOC: send packet from port 1 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:05 */
    prvTgfNstPortIsolationTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

    /* AUTODOC: verify to get packets on all ports */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC((PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS) * prvTgfBurstCount, portCntrs.goodOctetsRcv.l[0],
                                         "get another goodOctetsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");

            continue;
        }

        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC((PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS) * prvTgfBurstCount, portCntrs.goodOctetsSent.l[0],
                                     "get another goodOctetsSent counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}

/**
* @internal prvTgfNstL2PortIsolationConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfNstL2PortIsolationConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    prvTgfNstPortIsolationTestTableReset(PRV_TGF_VLANID_CNS);

    /* AUTODOC: disable Port Isolation feature */
    rc = prvTgfNstPortIsolationEnableSet(prvTgfDevNum, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfNstPortIsolationEnableSet: %d %d", prvTgfDevNum, GT_FALSE);

    /* restore the original hwDevNum */
    portIsolationLimitedHwDevNumRestore();
}

/**
* @internal prvTgfNstL3PortIsolationConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Configure portIsolationMode on VLAN entry (VID = 5).
*         Enable Port Isolation feature.
*         Set port isolation table entry for L3 with:
*         devNum    = 0
*         trafficType  = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E
*         srcInterface = devNum = 0, portNum = 0
*         cpuPortMember = GT_FALSE
*         localPortsMembersPtr = 0x40000
*/
GT_VOID prvTgfNstL3PortIsolationConfigurationSet
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC srcInterface = UTF_INTERFACE_INIT_VAL_CNS;
    GT_STATUS               rc = GT_OK;
    GT_U32                  entryIndex;
    PRV_TGF_IP_ROUTING_MODE_ENT routingMode;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* for the mode that run with random devNum that lead to HwDevNum = devNum + 0x10 :
       the port isolation 2K+128 entries ,
       must be limited to HwDevNum of 5 bits of HW devNum in range 0x0..0x10
       and 7 bits of portNum

       and even with HwDevNum 0x10 it getting to 'trunk entries' range.
    */
    portIsolationLimitedHwDevNumSet();

    srcInterface.type = CPSS_INTERFACE_PORT_E;
    srcInterface.devPort.hwDevNum = prvTgfDevNum;
    srcInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS];

    portIsolationCmd = PRV_TGF_BRG_VLAN_PORT_ISOLATION_L3_CMD_E;

    /* clear entry */
    cpssOsMemSet(&localPortsVlanMembers, 0, sizeof(localPortsVlanMembers));

    /* ports 18, 23 are VLAN_2 Members */
    CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[2]);
    CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[3]);

    /* AUTODOC: create VLAN 6 with untagged ports [2,3] and L3 isolation mode */
    prvTgfNstPortIsolationTestInit(PRV_TGF_NEXTHOPE_VLANID_CNS, portIsolationCmd, localPortsVlanMembers);

    /* clear entry */
    cpssOsMemSet(&localPortsVlanMembers, 0, sizeof(localPortsVlanMembers));

    /* ports 0, 8 are VLAN_1 Members */
    CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[0]);
    CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[1]);

    /* AUTODOC: create VLAN 5 with untagged ports [0,1] and L3 isolation mode */
    prvTgfNstPortIsolationTestInit(PRV_TGF_VLANID_CNS, portIsolationCmd, localPortsVlanMembers);

    /* need to remove casting and to fix code after lion2 development is done*/
    CPSS_TBD_BOOKMARK_EARCH

    /* AUTODOC: enable Port Isolation feature */
    rc = prvTgfNstPortIsolationEnableSet((GT_U8)srcInterface.devPort.hwDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfNstPortIsolationEnableSet: %d %d", srcInterface.devPort.hwDevNum, GT_TRUE);

    /* set L3 port isolation table entry */
    portIsolationTraffic = PRV_TGF_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E;

    /* clear entry */
    cpssOsMemSet(&localPortsMembers, 0, sizeof(localPortsMembers));

    /* port 18 is PortMembers */
    CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsMembers,prvTgfPortsArray[2]);

    /* need to remove casting and to fix code after lion2 development is done*/
    CPSS_TBD_BOOKMARK_EARCH

    /* AUTODOC: add PortIsolation entry with ports [2] for L3 packets */
    rc = prvTgfNstPortIsolationTableEntrySet((GT_U8)srcInterface.devPort.hwDevNum,
                                             portIsolationTraffic,
                                             srcInterface,
                                             GT_FALSE,
                                             &localPortsMembers);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfNstPortIsolationTableEntrySet: devNum = %d portNum = %d trafficType = %d",
                                 srcInterface.devPort.hwDevNum, srcInterface.devPort.portNum, portIsolationTraffic);

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

    /* set Ipv4 Routing */
    for (entryIndex = 0; entryIndex < PRV_TGF_SEND_COUNT_CNS; entryIndex++)
        switch(routingMode)
        {
            case PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E:

                prvTgfNstPortIsolationUcIpv4PbrRoutingSet(
                        prvTgfRouterArpIndexArray[entryIndex],
                        prvTgfArpMacArray[entryIndex],
                        prvTgfIpv4AddrArray[entryIndex],
                        prvTgfPortsArray[prvTgfNextHopePortIdxArray[entryIndex]]);
                break;

            case PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E:

                prvTgfNstPortIsolationUcIpv4LttRoutingSet(
                        prvTgfRouterArpIndexArray[entryIndex],
                        prvTgfArpMacArray[entryIndex],
                        prvTgfRouteEntryBaseIndexArray[entryIndex],
                        prvTgfIpv4AddrArray[entryIndex],
                        prvTgfPortsArray[prvTgfNextHopePortIdxArray[entryIndex]]);
                break;

            case PRV_TGF_IP_ROUTING_MODE_UNDEFINED_E:

                break;
        }
}

/**
* @internal prvTgfNstL3PortIsolationTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 two IP packets:
*         0000: 00 00 00 00 34 02 00 00 00 00 00 01 81 00 00 05
*         0010: 08 00 45 00 00 2A 00 00 00 00 40 04 76 CB 01 01
*         0020: 01 01 01 01 01 03 00 00 00 00 00 00 00 00 00 00
*         0030: 00 00 00 00 00 00 00 00 00 00 00 00
*         0000: 00 00 00 00 34 02 00 00 00 00 00 01 81 00 00 05
*         0010: 08 00 45 00 00 2A 00 00 00 00 40 04 72 C7 02 02
*         0020: 02 02 01 02 03 04 00 00 00 00 00 00 00 00 00 00
*         0030: 00 00 00 00 00 00 00 00 00 00 00 00
*         Success Criteria:
*         One packet is captured on port 18
*         Additional Configuration:
*         Disable port isolation feature in VLAN entry.
*         Send to device's port 0 two IP packets:
*         0000: 00 00 00 00 34 02 00 00 00 00 00 01 81 00 00 05
*         0010: 08 00 45 00 00 2A 00 00 00 00 40 04 76 CB 01 01
*         0020: 01 01 01 01 01 03 00 00 00 00 00 00 00 00 00 00
*         0030: 00 00 00 00 00 00 00 00 00 00 00 00
*         0000: 00 00 00 00 34 02 00 00 00 00 00 01 81 00 00 05
*         0010: 08 00 45 00 00 2A 00 00 00 00 40 04 72 C7 02 02
*         0020: 02 02 01 02 03 04 00 00 00 00 00 00 00 00 00 00
*         0030: 00 00 00 00 00 00 00 00 00 00 00 00
*         Success Criteria:
*         Packet is captured on ports: 18,23
*/
GT_VOID prvTgfNstL3PortIsolationTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       portIter = 0;
    GT_U32       sendIter = 0;
    GT_U32       expectedGoodOctetsValue;
    GT_U32       expectedGoodPctsValue;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: Iterate thru 2 stages: */
    for (sendIter = 0; sendIter < PRV_TGF_SEND_COUNT_CNS; sendIter++)
    {
        /* reset counters */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }

        /* setting IP addresses and checksum for packet 1 */
        prvTgfPacketIpv4Part.srcAddr[0] = 1;
        prvTgfPacketIpv4Part.srcAddr[1] = 1;
        prvTgfPacketIpv4Part.srcAddr[2] = 1;
        prvTgfPacketIpv4Part.srcAddr[3] = 1;
        cpssOsMemCpy(prvTgfPacketIpv4Part.dstAddr, prvTgfIpv4AddrArray[0], sizeof(prvTgfPacketIpv4Part.dstAddr));

        /* AUTODOC: send IPv4 packet from port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:01, VID=5 */
        /* AUTODOC:   srcIP=1.1.1.1, dstIP=1.1.1.3 */
        prvTgfNstPortIsolationTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS], &prvTgfPacketIpv4Info);

        /* setting IP addresses and checksum for packet 2 */
        prvTgfPacketIpv4Part.srcAddr[0] = 2;
        prvTgfPacketIpv4Part.srcAddr[1] = 2;
        prvTgfPacketIpv4Part.srcAddr[2] = 2;
        prvTgfPacketIpv4Part.srcAddr[3] = 2;
        cpssOsMemCpy(prvTgfPacketIpv4Part.dstAddr, prvTgfIpv4AddrArray[1], sizeof(prvTgfPacketIpv4Part.dstAddr));

        /* AUTODOC: send IPv4 packet from port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:01, VID=5 */
        /* AUTODOC:   srcIP=2.2.2.2, dstIP=1.2.3.4 */
        prvTgfNstPortIsolationTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS], &prvTgfPacketIpv4Info);

        /* AUTODOC: verify traffic: */
        /* AUTODOC:   stage#1: packet is captured on port 2 */
        /* AUTODOC:   stage#2: packet is captured on ports [2,3] */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            /* read counters */
            rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);

            if (PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS == portIter)
            {
                /* check Rx counters */
                UTF_VERIFY_EQUAL0_STRING_MAC((PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS) * prvTgfBurstCount * 2, portCntrs.goodOctetsRcv.l[0],
                                             "get another goodOctetsRcv counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount * 2, portCntrs.goodPktsRcv.l[0],
                                             "get another goodPktsRcv counter than expected");

                continue;
            }

            if (sendIter == 0)
            {
                if (prvTgfNextHopePortIdxArray[0] == portIter)
                {
                    expectedGoodOctetsValue = (PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                    expectedGoodPctsValue = prvTgfBurstCount;
                }
                else
                {
                    expectedGoodOctetsValue = 0;
                    expectedGoodPctsValue = 0;
                }
            }
            else /* sendIter == 1 */
            {
                if (prvTgfNextHopePortIdxArray[0] == portIter ||
                    prvTgfNextHopePortIdxArray[1] == portIter)
                {
                    expectedGoodOctetsValue = (PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                    expectedGoodPctsValue = prvTgfBurstCount;
                }
                else
                {
                    expectedGoodOctetsValue = 0;
                    expectedGoodPctsValue = 0;
                }
            }

            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(expectedGoodOctetsValue, portCntrs.goodOctetsSent.l[0],
                                         "get another goodOctetsSent counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(expectedGoodPctsValue, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }

        /* disable port isolation feature per Next HOP VLAN */

        /* clear entry */
        cpssOsMemSet(&localPortsVlanMembers, 0, sizeof(localPortsVlanMembers));

        /* ports 18, 23 are VLAN_2 Members */
        CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[2]);
        CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[3]);

        /* AUTODOC: for Stage#2 disable port isolation for VLAN 6, ports [2,3] */
        prvTgfNstPortIsolationTestInit(PRV_TGF_NEXTHOPE_VLANID_CNS, PRV_TGF_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E, localPortsVlanMembers);
    }
}

/**
* @internal prvTgfNstL3PortIsolationConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfNstL3PortIsolationConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_IPADDR ipAddr;
    GT_U32    entryIndex;

    /* AUTODOC: RESTORE CONFIGURATION: */

    prvTgfNstPortIsolationTestTableReset(PRV_TGF_VLANID_CNS);
    prvTgfNstPortIsolationTestTableReset(PRV_TGF_NEXTHOPE_VLANID_CNS);

    /* AUTODOC: delete IPv4 prefixes */
    for (entryIndex = 0; entryIndex < PRV_TGF_SEND_COUNT_CNS; entryIndex++)
    {
        /* fill the destination IP address for Ipv4 prefix in Virtual Router */
        cpssOsMemCpy(ipAddr.arIP, prvTgfIpv4AddrArray[entryIndex], sizeof(ipAddr.arIP));

        /* delete the Ipv4 prefix */
        rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, 0, ipAddr, 32);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);
    }

    /* AUTODOC: disable Port Isolation Feature */
    rc = prvTgfNstPortIsolationEnableSet(prvTgfDevNum, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfNstPortIsolationEnableSet: %d %d", prvTgfDevNum, GT_FALSE);

    /* restore the original hwDevNum */
    portIsolationLimitedHwDevNumRestore();

}

/**
* @internal prvTgfNstL2L3PortIsolationConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Configure portIsolationMode on VLAN entry (VID = 5).
*         Enable Port Isolation feature.
*         Set port isolation table entry for L3 with:
*         devNum    = 0
*         trafficType  = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E
*         srcInterface = devNum = 0, portNum = 0
*         cpuPortMember = GT_FALSE
*         localPortsMembersPtr = 0x40000
*         Set port isolation table entry for L2 with:
*         devNum    = 0
*         trafficType  = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E
*         srcInterface = devNum = 0, portNum = 0
*         cpuPortMember = GT_FALSE
*         localPortsMembersPtr = 0x100
*/
GT_VOID prvTgfNstL2L3PortIsolationConfigurationSet
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC srcInterface = UTF_INTERFACE_INIT_VAL_CNS;
    GT_STATUS               rc = GT_OK;
    GT_U32                  entryIndex;
    PRV_TGF_IP_ROUTING_MODE_ENT routingMode;

    /* AUTODOC: SETUP CONFIGURATION: */
    /* for the mode that run with random devNum that lead to HwDevNum = devNum + 0x10 :
       the port isolation 2K+128 entries ,
       must be limited to HwDevNum of 5 bits of HW devNum in range 0x0..0x10
       and 7 bits of portNum

       and even with HwDevNum 0x10 it getting to 'trunk entries' range.
    */
    portIsolationLimitedHwDevNumSet();

    srcInterface.type = CPSS_INTERFACE_PORT_E;
    srcInterface.devPort.hwDevNum = prvTgfDevNum;
    srcInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS];

    portIsolationCmd = PRV_TGF_BRG_VLAN_PORT_ISOLATION_L2_L3_CMD_E;

    /* clear entry */
    cpssOsMemSet(&localPortsVlanMembers, 0, sizeof(localPortsVlanMembers));

    /* ports 18, 23 are VLAN_2 Members */
    CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[2]);
    CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[3]);

    /* AUTODOC: create VLAN 6 with untagged ports [2,3] and L2_L3 isolation mode */
    prvTgfNstPortIsolationTestInit(PRV_TGF_NEXTHOPE_VLANID_CNS, portIsolationCmd, localPortsVlanMembers);

    /* clear entry */
    cpssOsMemSet(&localPortsVlanMembers, 0, sizeof(localPortsVlanMembers));

    /* ports 0, 8 are VLAN_1 Members */
    CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[0]);
    CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[1]);

    /* AUTODOC: create VLAN 5 with untagged ports [0,1] and L2_L3 isolation mode */
    prvTgfNstPortIsolationTestInit(PRV_TGF_VLANID_CNS, portIsolationCmd, localPortsVlanMembers);

    /* need to remove casting and to fix code after lion2 development is done*/
    CPSS_TBD_BOOKMARK_EARCH

    /* AUTODOC: enable Port Isolation feature */
    rc = prvTgfNstPortIsolationEnableSet((GT_U8)srcInterface.devPort.hwDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfNstPortIsolationEnableSet: %d %d", srcInterface.devPort.hwDevNum, GT_TRUE);

    /* set L3 port isolation table entry */
    portIsolationTraffic = PRV_TGF_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E;

    /* clear entry */
    cpssOsMemSet(&localPortsMembers, 0, sizeof(localPortsMembers));

    /* port 18 is PortMembers */
    CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsMembers,prvTgfPortsArray[localL2L3PortsIdxMembers[1]]);

    /* need to remove casting and to fix code after lion2 development is done*/
    CPSS_TBD_BOOKMARK_EARCH

    /* AUTODOC: add PortIsolation entry with ports [2] for L3 packets */
    rc = prvTgfNstPortIsolationTableEntrySet((GT_U8)srcInterface.devPort.hwDevNum,
                                             portIsolationTraffic,
                                             srcInterface,
                                             GT_FALSE,
                                             &localPortsMembers);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfNstPortIsolationTableEntrySet: %d, %d, %d",
                                 srcInterface.devPort.hwDevNum, srcInterface.devPort.hwDevNum, portIsolationTraffic);

    /* clear entry */
    cpssOsMemSet(&localPortsMembers, 0, sizeof(localPortsMembers));

    /* port 8 is PortMembers */
    CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsMembers,prvTgfPortsArray[localL2L3PortsIdxMembers[0]]);

    /* set L2 port isolation table entry */
    portIsolationTraffic = PRV_TGF_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E;

    /* need to remove casting and to fix code after lion2 development is done*/
    CPSS_TBD_BOOKMARK_EARCH

    /* AUTODOC: add PortIsolation entry with ports [1] for L2 packets */
    rc = prvTgfNstPortIsolationTableEntrySet((GT_U8)srcInterface.devPort.hwDevNum,
                                             portIsolationTraffic,
                                             srcInterface,
                                             GT_FALSE,
                                             &localPortsMembers);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfNstPortIsolationTableEntrySet: devNum = %d portNum = %d trafficType = %d",
                                 srcInterface.devPort.hwDevNum, srcInterface.devPort.portNum, portIsolationTraffic);

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

    /* set Ipv4 Routing */
    for (entryIndex = 0; entryIndex < PRV_TGF_SEND_COUNT_CNS; entryIndex++)
        switch(routingMode)
        {
            case PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E:
                prvTgfNstPortIsolationUcIpv4PbrRoutingSet(
                        prvTgfRouterArpIndexArray[entryIndex],
                        prvTgfArpMacArray[entryIndex],
                        prvTgfIpv4AddrArray[entryIndex],
                        prvTgfPortsArray[prvTgfNextHopePortIdxArray[entryIndex]]);
                break;

            case PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E:

                prvTgfNstPortIsolationUcIpv4LttRoutingSet(
                        prvTgfRouterArpIndexArray[entryIndex],
                        prvTgfArpMacArray[entryIndex],
                        prvTgfRouteEntryBaseIndexArray[entryIndex],
                        prvTgfIpv4AddrArray[entryIndex],
                        prvTgfPortsArray[prvTgfNextHopePortIdxArray[entryIndex]]);
                break;

            case PRV_TGF_IP_ROUTING_MODE_UNDEFINED_E:

                break;
        }
}

/**
* @internal prvTgfNstL2L3PortIsolationTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:05,
*         Send to device's port 0 two IP packets:
*         0000: 00 00 00 00 34 02 00 00 00 00 00 01 81 00 00 05
*         0010: 08 00 45 00 00 2A 00 00 00 00 40 04 76 CB 01 01
*         0020: 01 01 01 01 01 03 00 00 00 00 00 00 00 00 00 00
*         0030: 00 00 00 00 00 00 00 00 00 00 00 00
*         0000: 00 00 00 00 34 02 00 00 00 00 00 01 81 00 00 05
*         0010: 08 00 45 00 00 2A 00 00 00 00 40 04 72 C7 02 02
*         0020: 02 02 01 02 03 04 00 00 00 00 00 00 00 00 00 00
*         0030: 00 00 00 00 00 00 00 00 00 00 00 00
*         Success Criteria:
*         L2 packet is captured on port 8
*         L3 packet is captured on port 18
*         Additional Configuration:
*         Disable port isolation feature in VLAN entry
*         Generate Traffic:
*         Send to device's port 0 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:05,
*         Send to device's port 0 two IP packets:
*         0000: 00 00 00 00 34 02 00 00 00 00 00 01 81 00 00 05
*         0010: 08 00 45 00 00 2A 00 00 00 00 40 04 76 CB 01 01
*         0020: 01 01 01 01 01 03 00 00 00 00 00 00 00 00 00 00
*         0030: 00 00 00 00 00 00 00 00 00 00 00 00
*         0000: 00 00 00 00 34 02 00 00 00 00 00 01 81 00 00 05
*         0010: 08 00 45 00 00 2A 00 00 00 00 40 04 72 C7 02 02
*         0020: 02 02 01 02 03 04 00 00 00 00 00 00 00 00 00 00
*         0030: 00 00 00 00 00 00 00 00 00 00 00 00
*         Success Criteria:
*         L2 packet is captured on port 8
*         L3 packet is captured on port 18, 23
*/
GT_VOID prvTgfNstL2L3PortIsolationTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       portIter = 0;
    GT_U32       sendIter = 0;
    GT_U32       expectedGoodOctetsValue;
    GT_U32       expectedGoodPctsValue;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: Iterate thru 2 stages: */
    for (sendIter = 0; sendIter < PRV_TGF_SEND_COUNT_CNS; sendIter++)
    {
        /* reset counters */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }

        /* AUTODOC: send BC packet from port 0 with: */
        /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:02, VID=5 */
        prvTgfNstPortIsolationTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS], &prvTgfPacketInfo);

        /* setting IP addresses and checksum for packet 2 */
        prvTgfPacketIpv4Part.srcAddr[0] = 1;
        prvTgfPacketIpv4Part.srcAddr[1] = 1;
        prvTgfPacketIpv4Part.srcAddr[2] = 1;
        prvTgfPacketIpv4Part.srcAddr[3] = 1;
        cpssOsMemCpy(prvTgfPacketIpv4Part.dstAddr, prvTgfIpv4AddrArray[0], sizeof(prvTgfPacketIpv4Part.dstAddr));

        /* AUTODOC: send IPv4 packet from port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:01, VID=5 */
        /* AUTODOC:   srcIP=1.1.1.1, dstIP=1.1.1.3 */
        prvTgfNstPortIsolationTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS], &prvTgfPacketIpv4Info);

        /* setting IP addresses and checksum for packet 3 */
        prvTgfPacketIpv4Part.srcAddr[0] = 2;
        prvTgfPacketIpv4Part.srcAddr[1] = 2;
        prvTgfPacketIpv4Part.srcAddr[2] = 2;
        prvTgfPacketIpv4Part.srcAddr[3] = 2;
        cpssOsMemCpy(prvTgfPacketIpv4Part.dstAddr, prvTgfIpv4AddrArray[1], sizeof(prvTgfPacketIpv4Part.dstAddr));

        /* AUTODOC: send IPv4 packet from port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:01, VID=5 */
        /* AUTODOC:   srcIP=2.2.2.2, dstIP=1.2.3.4 */
        prvTgfNstPortIsolationTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS], &prvTgfPacketIpv4Info);

        /* AUTODOC: verify traffic: */
        /* AUTODOC:   stage#1: L2 packet is captured on port 1, L3 - on port 2 */
        /* AUTODOC:   stage#1: L2 packet is captured on port 1, L3 - on ports 2,3 */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            /* read counters */
            rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);

            if (PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS == portIter)
            {
                /* check Rx counters */
                UTF_VERIFY_EQUAL0_STRING_MAC((PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS) * prvTgfBurstCount * 3, portCntrs.goodOctetsRcv.l[0],
                                             "get another goodOctetsRcv counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount * 3, portCntrs.goodPktsRcv.l[0],
                                             "get another goodPktsRcv counter than expected");

                continue;
            }

            if (sendIter == 0)
            {
                if (prvTgfNextHopePortIdxArray[1] != portIter)
                {
                    expectedGoodOctetsValue = (PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                    expectedGoodPctsValue = prvTgfBurstCount;
                }
                else
                {
                    expectedGoodOctetsValue = 0;
                    expectedGoodPctsValue = 0;
                }
            }
            else /* sendIter == 1 */
            {
                expectedGoodOctetsValue = (PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                expectedGoodPctsValue = prvTgfBurstCount;
            }

            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(expectedGoodOctetsValue, portCntrs.goodOctetsSent.l[0],
                                         "get another goodOctetsSent counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(expectedGoodPctsValue, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }

        /* disable port isolation feature per Ingress VLAN */

        /* clear entry */
        cpssOsMemSet(&localPortsVlanMembers, 0, sizeof(localPortsVlanMembers));

        /* ports 0, 8 are VLAN_1 Members */
        CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[0]);
        CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[1]);

        /* AUTODOC: for Stage#2 disable port isolation for VLAN 5, ports [0,1] */
        prvTgfNstPortIsolationTestInit(PRV_TGF_VLANID_CNS,
                 PRV_TGF_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E, localPortsVlanMembers);

        /* enable IPv4 Unicast Routing on Vlan because prvTgfNstPortIsolationTestInit disables it */
        rc = prvTgfIpVlanRoutingEnable(PRV_TGF_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfNstL2L3PortIsolationTrafficGenerate: %d %d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS]);

        /* disable port isolation feature per Next HOP VLAN */

        /* clear entry */
        cpssOsMemSet(&localPortsVlanMembers, 0, sizeof(localPortsVlanMembers));

        /* ports 18, 23 are VLAN_2 Members */
        CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[2]);
        CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[3]);

        /* AUTODOC: for Stage#2 disable port isolation for VLAN 6, ports [2,3] */
        prvTgfNstPortIsolationTestInit(PRV_TGF_NEXTHOPE_VLANID_CNS, PRV_TGF_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E, localPortsVlanMembers);
    }
}

/**
* @internal prvTgfNstL2L3PortIsolationConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfNstL2L3PortIsolationConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_IPADDR ipAddr;
    GT_U32    entryIndex;

    /* AUTODOC: RESTORE CONFIGURATION: */

    prvTgfNstPortIsolationTestTableReset(PRV_TGF_VLANID_CNS);
    prvTgfNstPortIsolationTestTableReset(PRV_TGF_NEXTHOPE_VLANID_CNS);

    /* AUTODOC: delete IPv4 prefixes */
    for (entryIndex = 0; entryIndex < PRV_TGF_SEND_COUNT_CNS; entryIndex++)
    {
        /* fill the destination IP address for Ipv4 prefix in Virtual Router */
        cpssOsMemCpy(ipAddr.arIP, prvTgfIpv4AddrArray[entryIndex], sizeof(ipAddr.arIP));

        /* delete the Ipv4 prefix */
        rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, 0, ipAddr, 32);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);
    }

    /* AUTODOC: disable Port Isolation Feature */
    rc = prvTgfNstPortIsolationEnableSet(prvTgfDevNum, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfNstPortIsolationEnableSet: %d %d", prvTgfDevNum, GT_FALSE);

    /* restore the original hwDevNum */
    portIsolationLimitedHwDevNumRestore();
}

/**
* @internal prvTgfNstL2TrunkPortIsolationConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Configure port 8 and port 23 as members in trunk with trunkId = 1
*         Configure portIsolationMode on VLAN entry (VID = 5).
*         Enable Port Isolation feature
*         Set port isolation table entry for L2 with:
*         devNum    = 0
*         trafficType  = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E
*         srcInterface.type  = CPSS_INTERFACE_TRUNK_E
*         srcInterface.trunkId = 1.
*         cpuPortMember    = GT_FALSE
*         localPortsMembersPtr = 0x40000
*/
GT_VOID prvTgfNstL2TrunkPortIsolationConfigurationSet
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC srcInterface = UTF_INTERFACE_INIT_VAL_CNS;
    GT_STATUS               rc = GT_OK;
    GT_U32                  portIter = 0;
    GT_TRUNK_ID         trunkId;

    /* AUTODOC: SETUP CONFIGURATION: */
    /* for the mode that run with random devNum that lead to HwDevNum = devNum + 0x10 :
       the port isolation 2K+128 entries ,
       must be limited to HwDevNum of 5 bits of HW devNum in range 0x0..0x10
       and 7 bits of portNum

       and even with HwDevNum 0x10 it getting to 'trunk entries' range.
    */
    portIsolationLimitedHwDevNumSet();

    /* set trunk members */
    trunkMembersArray[0].hwDevice = prvTgfDevNum;
    trunkMembersArray[1].hwDevice = prvTgfDevNum;

    trunkMembersArray[0].port = prvTgfPortsArray[1];
    trunkMembersArray[1].port = prvTgfPortsArray[3];

    /* AUTODOC: create trunk 1 with ports [1,3] */
    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);
    rc = prvTgfTrunkMembersSet(trunkId, 2, trunkMembersArray, 0, NULL);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTrunkMembersSet: %d %d", prvTgfDevNum, PRV_TGF_TRUNK_ID_CNS);

    portIsolationCmd = PRV_TGF_BRG_VLAN_PORT_ISOLATION_L2_CMD_E;

    /* clear entry */
    cpssOsMemSet(&localPortsVlanMembers, 0, sizeof(localPortsVlanMembers));

    /* ports 0, 8, 18, 23 are VLAN Members */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: create VLAN 5 with untagged ports [0,1,2,3] and L2 isolation mode */
    prvTgfNstPortIsolationTestInit(PRV_TGF_VLANID_CNS, portIsolationCmd, localPortsVlanMembers);

    /* AUTODOC: enable Port Isolation Feature */
    rc = prvTgfNstPortIsolationEnableSet(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfNstPortIsolationEnableSet: %d %d", prvTgfDevNum, GT_TRUE);

    /* set L2 port isolation table entry */
    portIsolationTraffic = PRV_TGF_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E;

    /* clear entry */
    cpssOsMemSet(&localPortsMembers, 0, sizeof(localPortsMembers));

    /* port 18 is PortMembers */
    CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsMembers,prvTgfPortsArray[2]);

    /* configure port isolation */
    srcInterface.type    = CPSS_INTERFACE_TRUNK_E;
    srcInterface.trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(srcInterface.trunkId);

    /* AUTODOC: add PortIsolation entry with ports [2] for L2 packets */
    rc = prvTgfNstPortIsolationTableEntrySet(prvTgfDevNum,
                                             portIsolationTraffic,
                                             srcInterface,
                                             GT_FALSE,
                                             &localPortsMembers);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfNstPortIsolationTableEntrySet: devNum = %d, trafficType = %d",
                                 prvTgfDevNum, portIsolationTraffic);

    /* WA for port isolation on trunk errata - */
    /* !!! FIXED in SIP5 devices !!!*/
    /* configuring the port isolation entries for trunk members */
    if ( PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(prvTgfDevNum) ||
         (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E))
    {
        srcInterface.type = CPSS_INTERFACE_PORT_E;
        srcInterface.devPort.hwDevNum = prvTgfDevNum;
        srcInterface.devPort.portNum = trunkMembersArray[0].port;

        rc = prvTgfNstPortIsolationTableEntrySet(prvTgfDevNum,
                                                 portIsolationTraffic,
                                                 srcInterface,
                                                 GT_FALSE,
                                                 &localPortsMembers);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfNstPortIsolationTableEntrySet: devNum = %d, trafficType = %d",
                                     prvTgfDevNum, portIsolationTraffic);

        srcInterface.devPort.portNum = trunkMembersArray[1].port;

        rc = prvTgfNstPortIsolationTableEntrySet(prvTgfDevNum,
                                                 portIsolationTraffic,
                                                 srcInterface,
                                                 GT_FALSE,
                                                 &localPortsMembers);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfNstPortIsolationTableEntrySet: devNum = %d, trafficType = %d",
                                     prvTgfDevNum, portIsolationTraffic);
    }

}

/**
* @internal prvTgfNstL2TrunkPortIsolationTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         Success Criteria:
*         Packet is captured on port 18
*         Additional Configuration:
*         Disable port isolation feature in VLAN entry
*         Generate Traffic:
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:05,
*         Success Criteria:
*         Packet is captured on ports 0, 18
*/
GT_VOID prvTgfNstL2TrunkPortIsolationTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                       rc       = GT_OK;
    GT_U32                          portIter = 0;
    GT_U32                          sendIter = 0;
    GT_U32                          expectedGoodOctetsValue;
    GT_U32                          expectedGoodPctsValue;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;


    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: Iterate thru 2 stages: */
    for (sendIter = 0; sendIter < PRV_TGF_SEND_COUNT_CNS; sendIter++)
    {
        /* reset counters */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }

        /* AUTODOC: send BC packet from port 1 with: */
        /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:02, VID=5 */
        prvTgfNstPortIsolationTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

        /* AUTODOC: verify traffic: */
        /* AUTODOC:   stage#1: packet is captured on port 2 */
        /* AUTODOC:   stage#1: packet is captured on ports [0,2] */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            /* read counters */
            rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);

            if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
            {
                /* check Rx counters */
                UTF_VERIFY_EQUAL0_STRING_MAC((PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS) * prvTgfBurstCount, portCntrs.goodOctetsRcv.l[0],
                                             "get another goodOctetsRcv counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                             "get another goodPktsRcv counter than expected");

                continue;
            }

            /* set expected counters */
            if (0 == sendIter)
            {
                if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(&localPortsMembers,prvTgfPortsArray[portIter]))
                {
                    expectedGoodOctetsValue = (PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                    expectedGoodPctsValue = prvTgfBurstCount;
                }
                else
                {
                    expectedGoodOctetsValue = 0;
                    expectedGoodPctsValue = 0;
                }
            }
            else
            {
                if ((trunkMembersArray[0].port != prvTgfPortsArray[portIter]) &&
                    (trunkMembersArray[1].port != prvTgfPortsArray[portIter]))
                {
                    expectedGoodOctetsValue = (PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                    expectedGoodPctsValue = prvTgfBurstCount;
                }
                else
                {
                    expectedGoodOctetsValue = 0;
                    expectedGoodPctsValue = 0;
                }
            }

            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(expectedGoodOctetsValue, portCntrs.goodOctetsSent.l[0],
                                         "get another goodOctetsSent counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(expectedGoodPctsValue, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }

        /* AUTODOC: for Stage#2 disable port isolation for VLAN 5, ports [0,1,2,3] */
        prvTgfNstPortIsolationTestInit(PRV_TGF_VLANID_CNS,
                 PRV_TGF_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E, localPortsVlanMembers);
    }
}

/**
* @internal prvTgfNstL2TrunkPortIsolationConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfNstL2TrunkPortIsolationConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_TRUNK_ID         trunkId;

    /* AUTODOC: RESTORE CONFIGURATION: */

    prvTgfNstPortIsolationTestTableReset(PRV_TGF_VLANID_CNS);

    /* AUTODOC: disable Port Isolation Feature */
    rc = prvTgfNstPortIsolationEnableSet(prvTgfDevNum, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfNstPortIsolationEnableSet: %d %d", prvTgfDevNum, GT_FALSE);

    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    /* AUTODOC: invalidate trunk entry 1 */
    rc = prvTgfTrunkMembersSet(trunkId, 0, NULL, 0, NULL);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTrunkMembersSet: %d %d", prvTgfDevNum, PRV_TGF_TRUNK_ID_CNS);

    /* restore the original hwDevNum */
    portIsolationLimitedHwDevNumRestore();
}

/**
* @internal prvTgfNstL3TrunkPortIsolationConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Configure port 0 as member in trunk with truckId = 1
*         Configure portIsolationMode on VLAN entry (VID = 5).
*         Enable Port Isolation feature.
*         Set port isolation table entry for L2 with:
*         devNum        = 0
*         trafficType     = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E
*         srcInterface.type  = CPSS_INTERFACE_TRUNK_E
*         srcInterface.trunkId = 1.
*         cpuPortMember    = GT_FALSE
*         localPortsMembersPtr = 0x40000
*         Perform Two route entries UC IPV4 Routing:
*         Note: change port interface to truck interface.
*/
GT_VOID prvTgfNstL3TrunkPortIsolationConfigurationSet
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC srcInterface = UTF_INTERFACE_INIT_VAL_CNS;
    GT_STATUS               rc = GT_OK;
    GT_U32                  entryIndex;
    GT_TRUNK_ID             trunkId;
    PRV_TGF_IP_ROUTING_MODE_ENT routingMode;

    /* AUTODOC: SETUP CONFIGURATION: */
    /* for the mode that run with random devNum that lead to HwDevNum = devNum + 0x10 :
       the port isolation 2K+128 entries ,
       must be limited to HwDevNum of 5 bits of HW devNum in range 0x0..0x10
       and 7 bits of portNum

       and even with HwDevNum 0x10 it getting to 'trunk entries' range.
    */
    portIsolationLimitedHwDevNumSet();

    /* set trunk members */
    trunkMembersArray[0].hwDevice = prvTgfDevNum;
    trunkMembersArray[0].port = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS];

    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    /* AUTODOC: create trunk 1 with port 0 */
    rc = prvTgfTrunkMembersSet(trunkId, 1, trunkMembersArray, 0, NULL);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTrunkMembersSet: %d %d", prvTgfDevNum, PRV_TGF_TRUNK_ID_CNS);

    portIsolationCmd = PRV_TGF_BRG_VLAN_PORT_ISOLATION_L3_CMD_E;

    /* clear entry */
    cpssOsMemSet(&localPortsVlanMembers, 0, sizeof(localPortsVlanMembers));

    /* ports 18, 23 are VLAN_2 Members */
    CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[2]);
    CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[3]);

    /* AUTODOC: create VLAN 6 with untagged ports [2,3] and L3 isolation mode */
    prvTgfNstPortIsolationTestInit(PRV_TGF_NEXTHOPE_VLANID_CNS, portIsolationCmd, localPortsVlanMembers);

    /* clear entry */
    cpssOsMemSet(&localPortsVlanMembers, 0, sizeof(localPortsVlanMembers));

    /* ports 0, 8 are VLAN_1 Members */
    CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[0]);
    CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[1]);

    /* AUTODOC: create VLAN 5 with untagged ports [0,1] and L3 isolation mode */
    prvTgfNstPortIsolationTestInit(PRV_TGF_VLANID_CNS, portIsolationCmd, localPortsVlanMembers);

    /* AUTODOC: enable Port Isolation Feature */
    rc = prvTgfNstPortIsolationEnableSet(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfNstPortIsolationEnableSet: %d %d", prvTgfDevNum, GT_TRUE);

    /* set L3 port isolation table entry */
    portIsolationTraffic = PRV_TGF_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E;

    /* clear entry */
    cpssOsMemSet(&localPortsMembers, 0, sizeof(localPortsMembers));

    /* port 18 is PortMembers */
    CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsMembers,prvTgfPortsArray[2]);

    /* configure port isolation */
    srcInterface.type    = CPSS_INTERFACE_TRUNK_E;
    srcInterface.trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(srcInterface.trunkId);

    /* AUTODOC: add PortIsolation entry with trunk [1] for L3 packets */
    rc = prvTgfNstPortIsolationTableEntrySet(prvTgfDevNum,
                                             portIsolationTraffic,
                                             srcInterface,
                                             GT_FALSE,
                                             &localPortsMembers);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfNstPortIsolationTableEntrySet: devNum = %d, trafficType = %d",
                                 prvTgfDevNum, portIsolationTraffic);

    /* WA for port isolation on trunk errata - */
    /* !!! FIXED in SIP5 devices !!!*/
    /* configuring the port isolation entries for trunk members */
    if (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(prvTgfDevNum) ||
        (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E))
    {
        srcInterface.type = CPSS_INTERFACE_PORT_E;
        srcInterface.devPort.hwDevNum = prvTgfDevNum;
        srcInterface.devPort.portNum = trunkMembersArray[0].port;

        rc = prvTgfNstPortIsolationTableEntrySet(prvTgfDevNum,
                                                 portIsolationTraffic,
                                                 srcInterface,
                                                 GT_FALSE,
                                                 &localPortsMembers);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfNstPortIsolationTableEntrySet: devNum = %d, trafficType = %d",
                                     prvTgfDevNum, portIsolationTraffic);
    }

    /* get routing mode */
    rc = prvTgfIpRoutingModeGet(&routingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

    /* set Ipv4 Routing */
    for (entryIndex = 0; entryIndex < PRV_TGF_SEND_COUNT_CNS; entryIndex++)
        switch(routingMode)
        {
            case PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E:

                prvTgfNstPortIsolationUcIpv4PbrRoutingSet(
                        prvTgfRouterArpIndexArray[entryIndex],
                        prvTgfArpMacArray[entryIndex],
                        prvTgfIpv4AddrArray[entryIndex],
                        prvTgfPortsArray[prvTgfNextHopePortIdxArray[entryIndex]]);
                break;

            case PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E:

                prvTgfNstPortIsolationUcIpv4LttRoutingSet(
                        prvTgfRouterArpIndexArray[entryIndex],
                        prvTgfArpMacArray[entryIndex],
                        prvTgfRouteEntryBaseIndexArray[entryIndex],
                        prvTgfIpv4AddrArray[entryIndex],
                        prvTgfPortsArray[prvTgfNextHopePortIdxArray[entryIndex]]);
                break;

            case PRV_TGF_IP_ROUTING_MODE_UNDEFINED_E:

                break;
        }
}

/**
* @internal prvTgfNstL3TrunkPortIsolationTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 two IP packets:
*         0000: 00 00 00 00 34 02 00 00 00 00 00 01 81 00 00 05
*         0010: 08 00 45 00 00 2A 00 00 00 00 40 04 76 CB 01 01
*         0020: 01 01 01 01 01 03 00 00 00 00 00 00 00 00 00 00
*         0030: 00 00 00 00 00 00 00 00 00 00 00 00
*         0000: 00 00 00 00 34 02 00 00 00 00 00 01 81 00 00 05
*         0010: 08 00 45 00 00 2A 00 00 00 00 40 04 72 C7 02 02
*         0020: 02 02 01 02 03 04 00 00 00 00 00 00 00 00 00 00
*         0030: 00 00 00 00 00 00 00 00 00 00 00 00
*         Success Criteria:
*         One packet is captured on port 18
*         Additional Configuration:
*         Disable port isolation feature in VLAN entry.
*         Send to device's port 0 two IP packets:
*         0000: 00 00 00 00 34 02 00 00 00 00 00 01 81 00 00 05
*         0010: 08 00 45 00 00 2A 00 00 00 00 40 04 76 CB 01 01
*         0020: 01 01 01 01 01 03 00 00 00 00 00 00 00 00 00 00
*         0030: 00 00 00 00 00 00 00 00 00 00 00 00
*         0000: 00 00 00 00 34 02 00 00 00 00 00 01 81 00 00 05
*         0010: 08 00 45 00 00 2A 00 00 00 00 40 04 72 C7 02 02
*         0020: 02 02 01 02 03 04 00 00 00 00 00 00 00 00 00 00
*         0030: 00 00 00 00 00 00 00 00 00 00 00 00
*         Success Criteria:
*         Packet is captured on ports: 18,23
*/
GT_VOID prvTgfNstL3TrunkPortIsolationTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       portIter = 0;
    GT_U32       sendIter = 0;
    GT_U32       expectedGoodOctetsValue;
    GT_U32       expectedGoodPctsValue;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: Iterate thru 2 stages: */
    for (sendIter = 0; sendIter < PRV_TGF_SEND_COUNT_CNS; sendIter++)
    {
        /* reset counters */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }

        /* setting IP addresses and checksum for packet 1 */
        prvTgfPacketIpv4Part.srcAddr[0] = 1;
        prvTgfPacketIpv4Part.srcAddr[1] = 1;
        prvTgfPacketIpv4Part.srcAddr[2] = 1;
        prvTgfPacketIpv4Part.srcAddr[3] = 1;
        cpssOsMemCpy(prvTgfPacketIpv4Part.dstAddr, prvTgfIpv4AddrArray[0], sizeof(prvTgfPacketIpv4Part.dstAddr));

        /* AUTODOC: send IPv4 packet from port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:01, VID=5 */
        /* AUTODOC:   srcIP=1.1.1.1, dstIP=1.1.1.3 */
        prvTgfNstPortIsolationTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS], &prvTgfPacketIpv4Info);

        /* setting IP addresses and checksum for packet 2 */
        prvTgfPacketIpv4Part.srcAddr[0] = 2;
        prvTgfPacketIpv4Part.srcAddr[1] = 2;
        prvTgfPacketIpv4Part.srcAddr[2] = 2;
        prvTgfPacketIpv4Part.srcAddr[3] = 2;
        cpssOsMemCpy(prvTgfPacketIpv4Part.dstAddr, prvTgfIpv4AddrArray[1], sizeof(prvTgfPacketIpv4Part.dstAddr));

        /* AUTODOC: send IPv4 packet from port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:01, VID=5 */
        /* AUTODOC:   srcIP=2.2.2.2, dstIP=1.2.3.4 */
        prvTgfNstPortIsolationTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS], &prvTgfPacketIpv4Info);

        /* AUTODOC: verify traffic: */
        /* AUTODOC:   stage#1: packet is captured on port 2 */
        /* AUTODOC:   stage#1: packet is captured on ports 2,3 */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            /* read counters */
            rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);

            if (PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS == portIter)
            {
                /* check Rx counters */
                UTF_VERIFY_EQUAL0_STRING_MAC((PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS) * prvTgfBurstCount * 2, portCntrs.goodOctetsRcv.l[0],
                                             "get another goodOctetsRcv counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount * 2, portCntrs.goodPktsRcv.l[0],
                                             "get another goodPktsRcv counter than expected");

                continue;
            }

            if (sendIter == 0)
            {
                if (prvTgfNextHopePortIdxArray[0] == portIter)
                {
                    expectedGoodOctetsValue = (PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                    expectedGoodPctsValue = prvTgfBurstCount;
                }
                else
                {
                    expectedGoodOctetsValue = 0;
                    expectedGoodPctsValue = 0;
                }
            }
            else /* sendIter == 1 */
            {
                if (prvTgfNextHopePortIdxArray[0] == portIter ||
                    prvTgfNextHopePortIdxArray[1] == portIter)
                {
                    expectedGoodOctetsValue = (PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                    expectedGoodPctsValue = prvTgfBurstCount;
                }
                else
                {
                    expectedGoodOctetsValue = 0;
                    expectedGoodPctsValue = 0;
                }
            }

            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(expectedGoodOctetsValue, portCntrs.goodOctetsSent.l[0],
                                         "get another goodOctetsSent counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(expectedGoodPctsValue, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
        /* disable port isolation feature per Next HOP VLAN */

        /* clear entry */
        cpssOsMemSet(&localPortsVlanMembers, 0, sizeof(localPortsVlanMembers));

        /* ports 18, 23 are VLAN_2 Members */
        CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[2]);
        CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[3]);

        /* AUTODOC: for Stage#2 disable port isolation for VLAN 6, ports [2,3] */
        prvTgfNstPortIsolationTestInit(PRV_TGF_NEXTHOPE_VLANID_CNS, PRV_TGF_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E, localPortsVlanMembers);

    }
}

/**
* @internal prvTgfNstL3TrunkPortIsolationConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfNstL3TrunkPortIsolationConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_IPADDR ipAddr;
    GT_U32    entryIndex;
    GT_TRUNK_ID         trunkId;

    /* AUTODOC: RESTORE CONFIGURATION: */

    prvTgfNstPortIsolationTestTableReset(PRV_TGF_VLANID_CNS);
    prvTgfNstPortIsolationTestTableReset(PRV_TGF_NEXTHOPE_VLANID_CNS);

    /* AUTODOC: delete Ipv4 prefixes */
    for (entryIndex = 0; entryIndex < PRV_TGF_SEND_COUNT_CNS; entryIndex++)
    {
        /* fill the destination IP address for Ipv4 prefix in Virtual Router */
        cpssOsMemCpy(ipAddr.arIP, prvTgfIpv4AddrArray[entryIndex], sizeof(ipAddr.arIP));

        /* delete the Ipv4 prefix */
        rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, 0, ipAddr, 32);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);
    }

    /* AUTODOC: disable Port Isolation Feature */
    rc = prvTgfNstPortIsolationEnableSet(prvTgfDevNum, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfNstPortIsolationEnableSet: %d %d", prvTgfDevNum, GT_FALSE);

    trunkId = PRV_TGF_TRUNK_ID_CNS;
    CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(trunkId);

    /* AUTODOC: invalidate trunk entry 1 */
    rc = prvTgfTrunkMembersSet(trunkId, 0, NULL, 0, NULL);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTrunkMembersSet: %d %d", prvTgfDevNum, PRV_TGF_TRUNK_ID_CNS);

    /* restore the original hwDevNum */
    portIsolationLimitedHwDevNumRestore();
}


/**
* @internal prvTgfNstL2VlanBasedPortIsolationVLANConfigurationSet function
* @endinternal
*
* @brief   Set vlan entry, with the all ports of the test as
*         in the vlan with specified tagging command
* @param[in] vlanId                   - the vlan Id
* @param[in] taggingCmd               - GT_TRUE - tagging command for all members
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfNstL2VlanBasedPortIsolationVLANConfigurationSet
(
    IN GT_U16                                   vlanId,
    IN PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT        taggingCmd,
    IN PRV_TGF_BRG_VLAN_PORT_ISOLATION_CMD_ENT  portIsolationCmd
)
{
    GT_U32                              portIter;
    CPSS_PORTS_BMP_STC                  portsTagging;
    PRV_TGF_BRG_VLAN_INFO_STC           vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC  portsTaggingCmd;

    /* clear entry */
    cpssOsMemSet(&localPortsVlanMembers, 0, sizeof(localPortsVlanMembers));
    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC));
    cpssOsMemSet(&portsTagging, 0, sizeof(PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC));
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    /* set vlan entry */
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
    vlanInfo.stgId                = PRV_TGF_STG_ID_CNS;
    vlanInfo.autoLearnDisable     = GT_TRUE;/* working in controlled learning */
    vlanInfo.naMsgToCpuEn         = GT_TRUE;/* working in controlled learning */
    vlanInfo.mruIdx               = 0;
    vlanInfo.bcastUdpTrapMirrEn   = GT_FALSE;
    vlanInfo.floodVidxMode        = PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;
    vlanInfo.portIsolationMode    = portIsolationCmd;
    vlanInfo.ucastLocalSwitchingEn = GT_FALSE;
    vlanInfo.mcastLocalSwitchingEn = GT_FALSE;
    vlanInfo.unknownMacSaCmd = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.floodVidx      = 0xfff;



    /* add the needed ports to the vlan */
    for(portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[portIter]);
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsTagging,prvTgfPortsArray[portIter]);
        portsTaggingCmd.portsCmd[prvTgfPortsArray[portIter]] =
            taggingCmd;
    }

    return prvTgfBrgVlanEntryWrite(
        prvTgfDevNum, vlanId, &localPortsVlanMembers, &portsTagging,
        &vlanInfo, &portsTaggingCmd);
}


/**
* @internal prvTgfNstL2AndL3VlanBasedPortIsolationConfigurationSave function
* @endinternal
*
* @brief   Save L2 VLAN based port isolation test configuration:
*/
GT_VOID prvTgfNstL2AndL3VlanBasedPortIsolationConfigurationSave
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: save span state to isolated forwarding for ports [1] */
    rc = cpssDxChBrgStpStateGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_STG_ID_CNS, &spanStateSendPort);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgStpStateGet");

    /* AUTODOC: save original PortIsolation indexing mode */
    rc = cpssDxChNstPortIsolationIndexingModeGet(prvTgfDevNum, &originalInedxingMode);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChNstPortIsolationIndexingModeGet: devNum = %d indexingMode = %d",
                                 prvTgfDevNum, originalInedxingMode);

    /* AUTODOC: get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d %d", prvTgfDevNum, prvTgfDefVlanId);

    /* AUTODOC: save source id mask and value for config restore */
    rc = prvTgfIpRouterSourceIdGet(CPSS_IP_UNICAST_E, &sourceIdOrig, &sourceIdMaskOrig);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterSourceIdGet: %d", prvTgfDevNum);
}



/**
* @internal prvTgfNstPortIsolationUcIpv4LttRoutingSet function
* @endinternal
*
* @brief   Set LTT Configuration
*/
static GT_VOID prvTgfNstVlanBasedPortIsolationUcIpv4LttRoutingSet
(
    IN GT_U32        routerArpIndex,
    IN TGF_MAC_ADDR  routerArpMac,
    IN GT_U32        routeEntryBaseIndex,
    IN TGF_IPV4_ADDR dstIpv4Addr,
    IN GT_U32         nextHopPortNum
)
{
    GT_STATUS                     rc = GT_OK;
    PRV_TGF_BRG_MAC_ENTRY_STC     macEntry;
    GT_ETHERADDR                  arpMacAddr;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC *regularEntryPtr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    GT_IPADDR                     ipAddr;

    PRV_UTF_LOG0_MAC("==== Setting LTT Configuration ====\n");

    /* -------------------------------------------------------------------------
     * 1. Set FDB Table
     */

    /* AUTODOC: enable VLAN based MAC learning on VLAN 5 */
    rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_VLANID_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);

    /* create a macEntry with .daRoute = GT_TRUE */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketIpv4L2Part.daMac, sizeof(TGF_MAC_ADDR));

    macEntry.key.entryType                  = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId         = PRV_TGF_VLANID_CNS;
    macEntry.dstInterface.type              = CPSS_INTERFACE_VID_E;
    macEntry.dstInterface.devPort.hwDevNum    = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS];
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

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, Port 1 */
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 2. Enable Routing
     */

    /* AUTODOC: enable UC IPv4 Routing on Port 1 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: enable IPv4 UC Routing on VLAN 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 3. Create the Route entry (Next hop) in Route table and Router ARP Table
     */

    /* write a ARP MAC address to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, routerArpMac, sizeof(TGF_MAC_ADDR));

    /* AUTODOC: write 2 ARP entries to ARP Table with: */
    /* AUTODOC:   arpIndex=1, MAC=00:00:00:00:00:22  */
    /* AUTODOC:   arpIndex=5, MAC=00:00:00:00:00:33  */
    rc = prvTgfIpRouterArpAddrWrite(routerArpIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

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
    regularEntryPtr->countSet                   = CPSS_IP_CNT_SET0_E;
    regularEntryPtr->trapMirrorArpBcEnable      = GT_FALSE;
    regularEntryPtr->sipAccessLevel             = 0;
    regularEntryPtr->dipAccessLevel             = 0;
    regularEntryPtr->ICMPRedirectEnable         = GT_FALSE;
    regularEntryPtr->scopeCheckingEnable        = GT_FALSE;
    regularEntryPtr->siteId                     = CPSS_IP_SITE_ID_INTERNAL_E;
    regularEntryPtr->mtuProfileIndex            = 0;
    regularEntryPtr->isTunnelStart              = GT_FALSE;
    regularEntryPtr->nextHopVlanId              = PRV_TGF_NEXTHOPE_VLANID_CNS;
    regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_VID_E;
    regularEntryPtr->nextHopInterface.devPort.hwDevNum = prvTgfDevNum;
    regularEntryPtr->nextHopInterface.devPort.portNum = nextHopPortNum;
    regularEntryPtr->nextHopARPPointer          = routerArpIndex;
    regularEntryPtr->nextHopTunnelPointer       = 0;

    /* AUTODOC: add 2 UC route entries with: */
    /* AUTODOC:   index=3, nextHopVlan=6, nextHopPort=2, nextHopArp=1 */
    /* AUTODOC:   index=5, nextHopVlan=6, nextHopPort=3, nextHopArp=5 */
    rc = prvTgfIpUcRouteEntriesWrite(routeEntryBaseIndex, routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 4. Create a new Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = routeEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* fill a destination IP address for the prefix */
    cpssOsMemCpy(ipAddr.arIP, dstIpv4Addr, sizeof(ipAddr.arIP));

    /* AUTODOC: add IPv4 UC prefix 1.1.1.3/32, routeIdx=3 */
    /* AUTODOC: add IPv4 UC prefix 1.2.3.4/32, routeIdx=5 */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, 0, ipAddr, 32, &nextHopInfo, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);
}

/**
* @internal prvTgfNstL2AndL3VlanBasedPortIsolationConfigurationSet function
* @endinternal
*
* @brief   Set L2 VLAN based port isolation test configurations.
*/
GT_VOID prvTgfNstL2AndL3VlanBasedPortIsolationConfigurationSet
(
    IN GT_BOOL enableL2
)
{
    GT_STATUS               rc = GT_OK;
    CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_ENT indexingMode = CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_EVLAN_E; /* the new indexing mode tested */
    GT_U32                  entryIndex;
    PRV_TGF_IP_ROUTING_MODE_ENT routingMode;
    GT_U32         portIter;

    if (enableL2)
    {
        /* AUTODOC: set Isolation Command */
        portIsolationCmd = PRV_TGF_BRG_VLAN_PORT_ISOLATION_L2_CMD_E;
        /* AUTODOC: set L2 port isolation table entry */
        portIsolationTraffic = PRV_TGF_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E;

        /* ports 18, 57, 64, 71 are VLAN Members */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[portIter]);
        }

        /* AUTODOC: create VLAN 5 with all untagged ports and L2 isolation mode */
        prvTgfNstPortIsolationTestInit(PRV_TGF_VLANID_CNS, portIsolationCmd, localPortsVlanMembers);

    }
    else
    {
        /* AUTODOC: set Isolation Command */
        portIsolationCmd = PRV_TGF_BRG_VLAN_PORT_ISOLATION_L3_CMD_E;
        /* AUTODOC: set L3 port isolation table entry */
        portIsolationTraffic = PRV_TGF_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E;

        /* ports 71, 64, 57 are PRV_TGF_NEXTHOPE_VLANID_CNS Members */
        CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[2]);
        CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[3]);
        CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[0]);

        /* AUTODOC: create VLAN 6 with untagged ports and L2 isolation mode */
        prvTgfNstPortIsolationTestInit(PRV_TGF_NEXTHOPE_VLANID_CNS, portIsolationCmd, localPortsVlanMembers);

        /* clear entry */
        cpssOsMemSet(&localPortsVlanMembers, 0, sizeof(localPortsVlanMembers));

        /* ports 18 are PRV_TGF_VLANID_CNS Members */
        CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[1]);

        /* AUTODOC: create VLAN 5 with untagged ports and L2 isolation mode */
        prvTgfNstPortIsolationTestInit(PRV_TGF_VLANID_CNS, portIsolationCmd, localPortsVlanMembers);

        /*rc = prvTgfNstL2VlanBasedPortIsolationVLANConfigurationSet(PRV_TGF_VLANID_CNS, PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E ,portIsolationCmd);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfNstL2VlanBasedPortIsolationVLANConfigurationSet: %d", prvTgfDevNum);*/

        /* get routing mode */
        rc = prvTgfIpRoutingModeGet(&routingMode);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRoutingModeGet: %d", prvTgfDevNum);

            /* AUTODOC: set router source id mask (less than value) and source id value   */
        prvTgfIpRouterSourceIdSet(CPSS_IP_UNICAST_E,
                              PRV_TGF_ROUTER_SOURCE_ID_CNS,         /* source id value */
                              PRV_TGF_ROUTER_SOURCE_ID_MASK_CNS);   /* source id mask (less than value) */

        /* set Ipv4 Routing */
        for (entryIndex = 0; entryIndex < PRV_TGF_SEND_COUNT_CNS; entryIndex++)
            switch(routingMode)
            {
                case PRV_TGF_IP_ROUTING_MODE_PCL_ACTION_E:
                    PRV_UTF_LOG0_MAC("==== PBR mode does not supported ====\n");
                    break;

                case PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E:
                    prvTgfNstVlanBasedPortIsolationUcIpv4LttRoutingSet(
                            prvTgfRouterArpIndexArray[entryIndex],
                            prvTgfArpMacArray[entryIndex],
                            prvTgfRouteEntryBaseIndexArray[entryIndex],
                            prvTgfIpv4AddrArray[entryIndex],
                            prvTgfPortsArray[entryIndex]);
                    break;

                case PRV_TGF_IP_ROUTING_MODE_UNDEFINED_E:

                    break;
            }
        }

    /* AUTODOC: get number of VLANs in the device */
    rc = cpssDxChCfgTableNumEntriesGet(prvTgfDevNum, CPSS_DXCH_CFG_TABLE_VLAN_E, &numOfVlans);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChCfgTableNumEntriesGet: %d", prvTgfDevNum);

    if (numOfVlans>=PRV_TGF_E_VLANID_BASE_CNS) /* this part relevant only if the device supports more than 4096 vlan IDs */
    {
        /* AUTODOC: create VLAN 5000 with untagged ports and L2 isolation mode */
        rc = prvTgfNstL2VlanBasedPortIsolationVLANConfigurationSet(PRV_TGF_E_VLANID_CNS, PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E ,portIsolationCmd);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfNstL2VlanBasedPortIsolationVLANConfigurationSet: %d", prvTgfDevNum);
    }

    /* AUTODOC: enable Port Isolation feature */
    rc = prvTgfNstPortIsolationEnableSet(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfNstPortIsolationEnableSet: %d %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: set eVlan PortIsolation indexing mode */
    rc = cpssDxChNstPortIsolationIndexingModeSet(prvTgfDevNum, indexingMode);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChNstPortIsolationIndexingModeSet: devNum = %d indexingMode = %d",
                                 prvTgfDevNum, indexingMode);
}

static GT_VOID prvTgfNstL2VlanBasedPortIsolationCheckTraffic
(
    GT_U32  testPart
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       portIter = 0;
    GT_U32       expectedGoodOctetsValue;
    GT_U32       expectedGoodPctsValue;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;

    /* AUTODOC: verify packets for each test part */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC((PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS) * prvTgfBurstCount, portCntrs.goodOctetsRcv.l[0],
                                         "get another goodOctetsRcv counter than expected");
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");

            continue;
        }
        else if ((testPart == 1) || (testPart == 3) || (testPart == 4))
        {
            if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(&localPortsMembers,prvTgfPortsArray[portIter]))
            {
                expectedGoodOctetsValue = (PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                expectedGoodPctsValue = prvTgfBurstCount;
            }
            else
            {
                expectedGoodOctetsValue = 0;
                expectedGoodPctsValue = 0;
            }
        }
        else /* parts 2 , 5*/
        {
            expectedGoodOctetsValue = (PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
            expectedGoodPctsValue = prvTgfBurstCount;
        }
        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(expectedGoodOctetsValue, portCntrs.goodOctetsSent.l[0],
                                     "get another goodOctetsSent counter than expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(expectedGoodPctsValue, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}

/**
* @internal prvTgfNstL2AndL3VlanBasedPortIsolationTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
*/
GT_VOID prvTgfNstL2AndL3VlanBasedPortIsolationTrafficGenerate
(
    GT_BOOL enableL2
)
{
#ifndef GM_USED /* don't test in GM */
    GT_U32      portIter;
#endif
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT dxChTrafficType;

    /* AUTODOC: SETUP CONFIGURATION: */
    CPSS_INTERFACE_INFO_STC srcInterface = UTF_INTERFACE_INIT_VAL_CNS;
    srcInterface.type = CPSS_INTERFACE_VID_E;
    srcInterface.devPort.hwDevNum = prvTgfDevNum;
    srcInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];

    if (enableL2)
    {
        srcInterface.vlanId = PRV_TGF_VLANID_CNS;
        dxChTrafficType = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E;
    }
    else
    {
        srcInterface.vlanId = PRV_TGF_NEXTHOPE_VLANID_CNS;
        dxChTrafficType = CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E;
    }
    /* AUTODOC: Part 1: for L2 and L3 isolation
                port[3] and port[0] are "Members"
                port[PRV_TGF_SEND_PORT_IDX_CNS] in Disabled STP mode ("Leaf").
                traffic will go only into port[0,1,3] */

    PRV_UTF_LOG0_MAC("\n[TGF]: Part 1\n");

    /* AUTODOC: set index of isolated bit it the SrcID */
    rc = cpssDxChNstPortIsolationSrcIdBitLocationSet(prvTgfDevNum, PRV_TGF_ISOLATED_BIT_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChNstPortIsolationSrcIdBitLocationSet: devNum = %d isolatedBitLoc = %d",
                                 prvTgfDevNum, PRV_TGF_ISOLATED_BIT_CNS);

    /* AUTODOC: set span state to isolated DSB for ports [PRV_TGF_SEND_PORT_IDX_CNS] */
    rc = prvTgfBrgStpPortStateSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_STG_ID_CNS, CPSS_STP_DISABLED_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgStpPortStateSet");

    /* clear entry */
    cpssOsMemSet(&localPortsMembers, 0, sizeof(localPortsMembers));

    /* port[3] and port[0] are "Members", means port[2] is not "Member" */
    CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsMembers,prvTgfPortsArray[3]);
    CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsMembers,prvTgfPortsArray[0]);

    /* call device specific API */
    rc = cpssDxChNstPortIsolationTableEntrySet(prvTgfDevNum,
                                           dxChTrafficType,
                                           &srcInterface,
                                           GT_FALSE,
                                           &localPortsMembers);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChNstPortIsolationTableEntrySet: %d",
                                 prvTgfDevNum);

    prvTgfNstPortIsolationTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketIpv4Info);

    /* check counters and reset */
    prvTgfNstL2VlanBasedPortIsolationCheckTraffic(1);

    /* test parts 2,3,4,5 only for L2 isolation */
    if (enableL2)
    {
        /* AUTODOC: Part 2:
                    port[3] and port[0] are still "Members"
                    port [1] will be in FWR mode.
                    traffic will reach all ports. */

        PRV_UTF_LOG0_MAC("[TGF]: Part 2 \n\n");

        rc = prvTgfBrgStpPortStateSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_STG_ID_CNS , CPSS_STP_FRWRD_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgStpPortStateSet");

        /* AUTODOC: send packet from port 18 */
        prvTgfNstPortIsolationTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

        /* check counters and reset */
        prvTgfNstL2VlanBasedPortIsolationCheckTraffic(2);

        /* AUTODOC: Part 3:
                    port[3] and port[0] are still "Members"
                    port [1] will be in FWR mode,
                    disable isolated bit.
                    traffic will reach only "Members" ports - ports [0,1,3] */

        PRV_UTF_LOG0_MAC("[TGF]: Part 3 \n\n");

        /* AUTODOC: Set index of isolated bit to 15. */
        rc = cpssDxChNstPortIsolationSrcIdBitLocationSet(prvTgfDevNum, 0, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChNstPortIsolationSrcIdBitLocationSet: devNum = %d isolatedBitLoc = 15",
                                     prvTgfDevNum);

        /* AUTODOC: send packet from port 1 with: */
        prvTgfNstPortIsolationTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

        /* check counters and reset */
        prvTgfNstL2VlanBasedPortIsolationCheckTraffic(3);

        /* AUTODOC: Part 4:
                    port[PRV_TGF_SEND_PORT_IDX_CNS] DSB mode.
                    - all packets will be filtered according to the "isMember" fields - means packet will leave through ports[0,1,3]*/

        PRV_UTF_LOG0_MAC("[TGF]: Part 4 \n\n");

        /* AUTODOC: set port [1] in DSB mode.*/
        prvTgfBrgStpPortStateSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_STG_ID_CNS , CPSS_STP_DISABLED_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgStpPortStateSet");

        /* AUTODOC: send packet from port[PRV_TGF_SEND_PORT_IDX_CNS] */
        prvTgfNstPortIsolationTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

        /* AUTODOC: check counters and reset */
        prvTgfNstL2VlanBasedPortIsolationCheckTraffic(4);

#ifndef GM_USED /* don't test in GM */
        /* AUTODOC: Part 5:
                    this part relevant only if the device supports more than 4096 vlan IDs
                    Set VLAN ID bigger than 4096
                    port[PRV_TGF_SEND_PORT_IDX_CNS] and port[PRV_TGF_ISOLATED_PORT_IDX_CNS] are "Leafs".
                    Set index of isolated bit to 5.
                    packet will be forwarded in all ports */
        /* AUTODOC: get number of VLANs in the device */

        if (numOfVlans>=PRV_TGF_E_VLANID_BASE_CNS)
        {
            PRV_UTF_LOG0_MAC("[TGF]: Part 5 \n\n");

            /* AUTODOC: Set index of isolated bit to 5. */
            rc = cpssDxChNstPortIsolationSrcIdBitLocationSet(prvTgfDevNum, PRV_TGF_ISOLATED_BIT_CNS, GT_TRUE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChNstPortIsolationSrcIdBitLocationSet: devNum = %d isolatedBitLoc = %d",
                                         prvTgfDevNum, PRV_TGF_ISOLATED_BIT_CNS);

            /* AUTODOC: set PVID 5000 for port 1 */
            rc = prvTgfBrgVlanPortVidSet(
                prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                PRV_TGF_E_VLANID_CNS);
            UTF_VERIFY_EQUAL2_STRING_MAC(
                GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
                prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

            /* AUTODOC: enable PVID forcing on port 1 */
            rc = prvTgfBrgVlanPortForcePvidEnable(
                prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                GT_TRUE);
            UTF_VERIFY_EQUAL2_STRING_MAC(
                GT_OK, rc, "prvTgfBrgVlanPortForcePvidEnable: %d, %d",
                prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

            /* ports 0, 8, 18, 23 are VLAN Members */
            for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
            {
                CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[portIter]);
            }

            /* AUTODOC: add PortIsolation entry with ports [0, 1, 2, 3] for L2 packets */
            rc = prvTgfNstPortIsolationTableEntrySet(prvTgfDevNum,
                                                     portIsolationTraffic,
                                                     srcInterface,
                                                     GT_FALSE,
                                                     &localPortsMembers);
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfNstPortIsolationTableEntrySet: devNum = %d portNum = %d trafficType = %d",
                                         srcInterface.devPort.hwDevNum, srcInterface.devPort.hwDevNum, portIsolationTraffic);

            /* AUTODOC: send packet from port[PRV_TGF_SEND_PORT_IDX_CNS] */
            prvTgfNstPortIsolationTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

            /* AUTODOC: check counters and reset */
            prvTgfNstL2VlanBasedPortIsolationCheckTraffic(5);
        }
#endif
    }
}

/**
* @internal prvTgfNstL2AndL3VlanBasedPortIsolationConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfNstL2AndL3VlanBasedPortIsolationConfigurationRestore
(
    GT_BOOL enableL2
)
{
    GT_STATUS rc;
    GT_IPADDR ipAddr;
    GT_U32    entryIndex;

    /* AUTODOC: RESTORE CONFIGURATION: */
    prvTgfNstPortIsolationTestTableReset(PRV_TGF_VLANID_CNS);
    prvTgfNstPortIsolationTestTableReset(PRV_TGF_NEXTHOPE_VLANID_CNS);

    if (!enableL2)
    {
        /* AUTODOC: delete IPv4 prefixes */
        for (entryIndex = 0; entryIndex < PRV_TGF_SEND_COUNT_CNS; entryIndex++)
        {
            /* fill the destination IP address for Ipv4 prefix in Virtual Router */
            cpssOsMemCpy(ipAddr.arIP, prvTgfIpv4AddrArray[entryIndex], sizeof(ipAddr.arIP));

            /* delete the Ipv4 prefix */
            rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, 0, ipAddr, 32);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);
        }
    }

    /* AUTODOC: disable Port Isolation feature */
    rc = prvTgfNstPortIsolationEnableSet(prvTgfDevNum, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfNstPortIsolationEnableSet: %d %d", prvTgfDevNum, GT_FALSE);

    /* AUTODOC: restore span state in ports [1] */
    prvTgfBrgStpPortStateSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], 0, spanStateSendPort);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgStpPortStateSet");

    /* AUTODOC: disable Src-ID bit location */
    rc = cpssDxChNstPortIsolationSrcIdBitLocationSet(prvTgfDevNum, PRV_TGF_ISOLATED_BIT_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChNstPortIsolationSrcIdBitLocationSet: devNum = %d isolatedBitLoc = %d",
                                 prvTgfDevNum, PRV_TGF_ISOLATED_BIT_CNS);

    /* AUTODOC: disable port isolation feature on VLAN 5 */
    prvTgfNstPortIsolationTestInit(PRV_TGF_VLANID_CNS,
             PRV_TGF_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E, localPortsVlanMembers);

    /* AUTODOC: set original PortIsolation indexing mode */
    rc = cpssDxChNstPortIsolationIndexingModeSet(prvTgfDevNum, originalInedxingMode);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChNstPortIsolationIndexingModeSet: devNum = %d indexingMode = %d",
                                 prvTgfDevNum, originalInedxingMode);
    if (numOfVlans>=PRV_TGF_E_VLANID_BASE_CNS)
    {
        /* AUTODOC: disable port isolation feature on VLAN 5000 */
        prvTgfNstPortIsolationTestInit(PRV_TGF_E_VLANID_CNS,
                                       PRV_TGF_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E, localPortsVlanMembers);

        /* AUTODOC: disable PVID forcing on port 0 */
        rc = prvTgfBrgVlanPortForcePvidEnable(
                prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortForcePvidEnable: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* AUTODOC: restore default vlanId for port 1 */
        rc = prvTgfBrgVlanPortVidSet(
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            prvTgfDefVlanId);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    }

    /* AUTODOC: restore default vlanId */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d",
                                 prvTgfDevNum, prvTgfDefVlanId);

    /* AUTODOC: restore source id mask and value */
    rc = prvTgfIpRouterSourceIdSet(CPSS_IP_MULTICAST_E, sourceIdOrig, sourceIdMaskOrig);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPvidSet: %d", prvTgfDevNum);





}

/**
* @internal prvTgfNstPacketSanityCheckConfigurationSet function
* @endinternal
*
* @brief Set test configuration
*/
GT_VOID prvTgfNstPacketSanityCheckConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_NET_CPU_CODE_TABLE_ENTRY_STC    cpuCodeEntry;
    CPSS_DXCH_NET_SDMA_RX_COUNTERS_STC      rxCounters;

    /* Create VLAN 479 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_NST_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite");

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS], &prvTgfDefVlanId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet");

    /* Set PVID 479 for port 3 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS], PRV_TGF_NST_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet");

    /* Add FDB entry with MAC 00:00:00:00:34:02, VLAN 479, port 1 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacketIpv4L2Part.daMac,
            PRV_TGF_NST_VLANID_CNS, prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    /* save CPU code table entry for cpu code CPSS_NET_UNKNOWN_UC_E */
    rc = prvTgfNetIfCpuCodeTableGet(prvTgfDevNum, CPSS_NET_UNKNOWN_UC_E, &prvTgfRestoreCfg.ingressTrapCpuEntryGet);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG2_MAC("prvTgfNetIfCpuCodeTableGet failed, device %d cpuCode %d\n", prvTgfDevNum, CPSS_NET_IP_HDR_ERROR_E);
    }

    /* set tcQueue for cpuCode CPSS_NET_UNKNOWN_UC_E  */
    cpssOsMemSet(&cpuCodeEntry, 0, sizeof(cpuCodeEntry));
    cpuCodeEntry.tc = PRV_TGF_NST_TC_IDX_CNS;
    rc = prvTgfNetIfCpuCodeTableSet(prvTgfDevNum, CPSS_NET_UNKNOWN_UC_E, &cpuCodeEntry);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfNetIfCpuCodeTableSet failed, device %d cpuCode %d\n", prvTgfDevNum, CPSS_NET_UNKNOWN_UC_E);

    /* clear the queue counters */
    rc = cpssDxChNetIfSdmaRxCountersGet(prvTgfDevNum, cpuCodeEntry.tc, &rxCounters);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChNetIfSdmaRxCountersGet failed, device %d tcQueue %d\n", prvTgfDevNum, cpuCodeEntry.tc);

    /* packet sanity check configuration */
    rc = cpssDxChNstProtSanityCheckPacketCommandGet(prvTgfDevNum, CPSS_NST_CHECK_TCP_SPORT_IS_DPORT_E, &prvTgfRestoreCfg.packetCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChNstProtSanityCheckPacketCommandGet");

    rc = cpssDxChNstProtSanityCheckPacketCommandSet(prvTgfDevNum, CPSS_NST_CHECK_TCP_SPORT_IS_DPORT_E, CPSS_PACKET_CMD_TRAP_TO_CPU_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChNstProtSanityCheckPacketCommandSet");
}

/**
* @internal prvTgfNstPacketSanityCheckTrafficGenerate function
* @endinternal
*
* @brief Generate traffic
*/
GT_VOID prvTgfNstPacketSanityCheckTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                                   rc  = GT_OK;
    GT_STATUS                                   st  = GT_OK;
    GT_U32                                      portIter;
    GT_U8                                       packetBufPtr[64] = {0};
    GT_U32                                      packetBufLen = 64;
    GT_U32                                      packetLen;
    GT_U8                                       devNum;
    GT_U8                                       queueCpu;
    TGF_NET_DSA_STC                             rxParams;
    CPSS_DXCH_NET_SDMA_RX_COUNTERS_STC          rxCounters;
    GT_U32                                      trapPktCount = 1;
    GT_U32                                      txPacketsCount = 0;
    GT_U32                                      stageNum;
    CPSS_PORT_MAC_COUNTER_SET_STC               portCntrs;

    /* Reset the port Counter */
    for (portIter = 0; portIter < PRV_TGF_PORTS_NUM_CNS; portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth");
    }

    /* Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Iterate thru 2 stages: */
    for (stageNum = 0; stageNum < 2; stageNum++ )
    {
        /* Start Rx capture */
        rc = tgfTrafficTableRxStartCapture(GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "tgfTrafficTableRxStartCapture");
        /* AUTODOC: for Stage#1 do the following: */
        if (stageNum == 1)
        {
            /* packet sanity check configuration */
            rc = cpssDxChNstProtSanityCheckPacketCommandSet(prvTgfDevNum, CPSS_NST_CHECK_TCP_SPORT_IS_DPORT_E, CPSS_PACKET_CMD_FORWARD_E);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChNstProtSanityCheckPacketCommandSet");
        }

        /* Generate traffic */
        rc = prvTgfSetTxSetupEth(
                prvTgfDevNum, &prvTgfPacketTcpInfo,
                1, 0 , NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfSetTxSetupEth");

        /* Send packet to port 0 */
        rc = prvTgfStartTransmitingEth(
                prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfStartTransmitingEth");

        rc = tgfTrafficTableRxStartCapture(GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "tgfTrafficTableRxStartCapture");

        /* Stage Number 0 corresponds to sanity check TRAP packet command and stage number 1
         * corresponds to sanity check FORWARD packet command. In case of TRAP stage, only
         * sent port corrsponds to received packet whereas in case of FORWARD stage all the
         * test ports corresponds to received packet.
         */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            /* read counters */
            rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
            switch(portIter)
            {
                case 0: /* port 0 */ txPacketsCount = 0;
                        break;
                case 1: /* port 1 */ txPacketsCount = 1;
                        break;
                case 2: /* port 2 */ txPacketsCount = 0;
                        break;
                case 3: /* port 3 */ txPacketsCount = (stageNum == 1) ? 1 : 0;
                        break;
                default: txPacketsCount = 0;
                         break;
            }

            UTF_VERIFY_EQUAL2_STRING_MAC(txPacketsCount, portCntrs.goodPktsSent.l[0],
                                         "get another Tx packets than expected on port[%d] = %d",
                                         portIter, prvTgfPortsArray[portIter]);
        }

        PRV_UTF_LOG0_MAC("check the CPU port capturing:\n");

        rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                           GT_TRUE, GT_TRUE,
                                           packetBufPtr,
                                           &packetBufLen,
                                           &packetLen,
                                           &devNum,
                                           &queueCpu,
                                           &rxParams);
        st = (stageNum == 0) ? GT_OK : GT_NO_MORE;
        UTF_VERIFY_EQUAL0_STRING_MAC(
            st, rc, "tgfTrafficGeneratorRxInCpuGet");

        if(stageNum == 0)
        {
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_TRUE,rxParams.dsaCmdIsToCpu);
            UTF_VERIFY_EQUAL0_PARAM_MAC(CPSS_NET_UNKNOWN_UC_E,
                                        rxParams.cpuCode);
        }

        /* Verify packet should reach in case of enabled queue*/
        rc = cpssDxChNetIfSdmaRxCountersGet(prvTgfDevNum, PRV_TGF_NST_TC_IDX_CNS, &rxCounters);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG2_MAC("cpssDxChNetIfSdmaRxCountersGet failed, device %d tcQueue %d\n", prvTgfDevNum, PRV_TGF_NST_TC_IDX_CNS);
        }

        trapPktCount = (stageNum == 0) ? 1 : 0;
        UTF_VERIFY_EQUAL0_STRING_MAC(trapPktCount, rxCounters.rxInPkts, "cpssDxChCfgReplicationCountersGet");

        /* Clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
    }
}

/**
* @internal prvTgfNstPacketSanityCheckConfigurationRestore function
* @endinternal
*
* @brief Restore configuration
*/
GT_VOID prvTgfNstPacketSanityCheckConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS                                   rc  = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    if (rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("tgfTrafficTableRxPcktTblClear failed for device %d\n", prvTgfDevNum);
    }

    /* AUTODOC: Disable RX capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* AUTODOC: reset hw counters */
    rc = prvTgfCommonAllBridgeCntrsReset();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfCommonAllCntrsReset");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush");

    /* Vlan restore */
    /* Restore default PVID on port 1 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS], prvTgfDefVlanId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet");

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_NST_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate");

    /* restore CPU code table entry for cpu code CPSS_NET_UNKNOWN_UC_E */
    rc = prvTgfNetIfCpuCodeTableSet(prvTgfDevNum, CPSS_NET_UNKNOWN_UC_E, &prvTgfRestoreCfg.ingressTrapCpuEntryGet);
    if (rc != GT_OK)
    {
        PRV_UTF_LOG2_MAC("prvTgfNetIfCpuCodeTableSet failed, device %d cpuCode %d\n", prvTgfDevNum, CPSS_NET_UNKNOWN_UC_E);
    }

    /* restore packet sanity check */
    rc = cpssDxChNstProtSanityCheckPacketCommandSet(prvTgfDevNum, CPSS_NST_CHECK_TCP_SPORT_IS_DPORT_E, prvTgfRestoreCfg.packetCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChNstProtSanityCheckPacketCommandSet");
}

