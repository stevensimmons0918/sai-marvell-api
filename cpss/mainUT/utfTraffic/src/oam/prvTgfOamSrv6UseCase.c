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
* @file prvTgfOamSrv6UseCase.c
*
* @brief SRV6 use case testing
*
* @version   1
********************************************************************************
*/
#include <oam/prvTgfOamSrv6UseCase.h>
#include <common/tgfIpGen.h>
#include <common/tgfMirror.h>
#include <cpss/dxCh/dxChxGen/pha/cpssDxChPha.h>
#include <cpss/dxCh/dxChxGen/oam/cpssDxChOam.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortLoopback.h>
#include <exactMatch/prvTgfExactMatchBasicDefaultActionPcl.h>
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
#include <common/tgfCscdGen.h>
#include <common/tgfCosGen.h>
#include <common/tgfNetIfGen.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgE2Phy.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/ppu/cpssDxChPpuTypes.h>
#include <cpss/dxCh/dxChxGen/ppu/cpssDxChPpu.h>
#include <ip/prvTgfIpv4UcPbrRoutingPriotiry.h>
#include <bridge/prvTgfFdbBasicIpv4UcRouting.h>
#include <bridge/prvTgfFdbBasicIpv6UcRouting.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* PHA Thread IDs */
#define PRV_TGF_PHA_THREAD_SRC_NODE_1_SEGMENT                               29
#define PRV_TGF_PHA_THREAD_SRC_NODE_FIRST_PASS_2_3_SEGMENTS                 30
#define PRV_TGF_PHA_THREAD_SRC_NODE_SECOND_PASS_3_SEGMENTS                  31
#define PRV_TGF_PHA_THREAD_SRC_NODE_SECOND_PASS_2_SEGMENTS                  32
#define PRV_TGF_PHA_THREAD_TYPE_SRV6_BEST_EFFORT_E                          48
#define PRV_TGF_PHA_THREAD_TYPE_SRV6_SRC_NODE_1_CONTAINER_E                 49
#define PRV_TGF_PHA_THREAD_TYPE_SRV6_SRC_NODE_FIRST_PASS_2_3_CONTAINERS_E   50
#define PRV_TGF_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_2_CONTAINERS_E    52
#define PRV_TGF_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_3_CONTAINERS_E    51
#define PRV_TGF_PHA_THREAD_TYPE_SRV6_END_NODE_COC32_GSID_E                  53

#define PRV_TGF_EGR_PACKET_TO_ANALYZER_TYPE                     2

/* Mask for Source-ID extraction */
#define PRV_TGF_SOURCE_ID_MASK                                  0x7FE

/* Mask for App-ID extraction */
#define PRV_TGF_APP_ID_MASK                                     0x3FF

/* Source-ID First Bit */
#define PRV_TGF_SOURCE_ID_FIRST_BIT                             0

/* Source-ID Last Bit */
#define PRV_TGF_SOURCE_ID_LAST_BIT                              0xF

/* Number of Reserved bits configured */
#define PRV_TGF_RESERVE_ID_BIT_CNS                              6


/* default number of tunnel start entry extension value */
#define PRV_TGF_EVLAN_SERVISE_ID_CNS                            0x1234

/* SRv6 DIP SGT index */
#define PRV_TGF_SOURCE_INDEX_CNS                                0x1EF

/* SRv6 DIP APP index */
#define PRV_TGF_APP_INDEX_CNS                                   0x7FF

/* Mask for App-ID extraction */
#define PRV_TGF_APP_ID_MASK                                     0x3FF

/* SRv6 G-SID tunnel node index */
#define PRV_TGF_GSID_NODE_INDEX_CNS                             0x4321

/* SRv6 G-SID tunnel function index */
#define PRV_TGF_GSID_FUNCTION_INDEX_CNS                         0xBCDE

/* Number of Reserved bits configured */
#define PRV_TGF_RESERVE_ID_BIT_CNS                              6

/* tunnel term entry index */
#define PRV_TGF_TTI_INDEX_CNS                                   1

/* pcl entry index */
#define PRV_TGF_PCL_INDEX_CNS                                   2

/* Ude Packet types count */
#define PRV_TGF_PCL_UDE_PKT_TYPES_CNS                           6

/* Udeb indexes count */
#define PRV_TGF_PCL_UDB_INDEX_CNS                               8

/* Ude Packet L3 0ffset base */
#define PRV_TGF_PCL_UDE_L3_OFFSET_BASE_CNS                      50

/* Ude Packet key UDB base index */
#define PRV_TGF_PCL_UDE_UDB_BASE_INDEX_CNS                      62

/* TTI rule index count */
#define PRV_TGF_TTI_RULE_INDEX_CNS                              12

/* PCL-Id */
#define PRV_TGF_PCL_CONFIG_INDEX_CNS                            10

/* PCL rule & action index */
#define IPCL_MATCH_INDEX_0_CNS                                  0
#define EPCL_MATCH_INDEX_CNS                                    prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(5)
#define PRV_TGF_SECOND_PASS_EGR_EPORT_CNS                       (1 + prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS])

static TGF_IPV6_ADDR        prvTgfPhaTemplateIPv6               = {0x1001, 0x1002, 0x1003, 0x1004, 0x1005, 0x1006, 0x1007, 0x1008};
static TGF_IPV6_ADDR        prvTgf1stTunnelStartSrcIPv6         = {0x2001, 0x2002, 0x2003, 0x2004, 0x2005, 0x2006, 0x2007, 0x2008};
static TGF_IPV6_ADDR        prvTgf1stTunnelStartDstIPv6         = {0x3001, 0x3002, 0x3003, 0x3004, 0x3005, 0x3006, 0x3007, 0x3008};
static TGF_PACKET_L2_STC    prvTgf1stTunnelL2Part               = {{0x00, 0x00, 0x00, 0x00, 0x34, 0x05}, {0x00, 0x00, 0x00, 0x00, 0x34, 0x06}};
static TGF_PACKET_L2_STC    prvTgf2ndTunnelL2Part               = {{0x00, 0x00, 0x00, 0x00, 0x34, 0x07}, {0x00, 0x00, 0x00, 0x00, 0x34, 0x08}};

/* the Arp Address index of the Router ARP Table
   to write to the UC Route entry Arp nextHopARPPointer field */
static GT_U32        prvTgfRouterArpIndex      = 1;

static GT_U32        prvTgfFdbRoutingArpIndex  = 3;

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 0;

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfPbrArpMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x33};
static TGF_MAC_ADDR  prvTgfFdbArpMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x44};
/* the MAC address to write to 40 MSBs of MAC SA base addr */
static GT_ETHERADDR  prvTgfMacAddr = {{0x00, 0x50, 0x36, 0x24, 0x00, 0x00}};
static GT_U8         dipCommonPrefix[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40};

#define TGF_ETHERTYPE_6666_VLAN_TAG_CNS                         0x6666

/* Default vlan ID */
#define PRV_TGF_DEFAULT_VLANID_CNS                              1
#define TGF_SEGMENT_LEN_CNS                                     16
#define TGF_IPV6_ADDR_LEN_CNS                                   16
#define TGF_TWO_SRH_CONTAINERS_LEN_CNS                          32
#define TGF_THREE_SRH_CONTAINERS_LEN_CNS                        48
#define TGF_HDR_LEN_EXT_UNIT_CNS                                8       /* Measured in 8Bytes unit, e.g in case of 16Byte HDR_Len_Ext is 2 */

/* Tunnel start entry index */
#define PRV_TGF_TUNNEL_START_INDEX_CNS                          8

/* Template profile index */
#define PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS                      1

/* port to Send/Receive traffic*/
#define PRV_TGF_INGRESS_PORT_IDX_CNS                            3
#define PRV_TGF_EGR_PORT_IDX_CNS                                1
#define PRV_TGF_LOOPBACK_PORT_IDX_CNS                           0

/* Loopback profiles */
#define PRV_TGF_LB_PROFILE_1_CNS                                1

/* MAC2ME entry index */
#define PRV_TGF_MAC_TO_ME_INDEX_CNS                             1

/* size of packet */
#define PRV_TGF_PACKET_SIZE_CNS                                 140
#define PRV_TGF_BURST_COUNT_CNS                                 2
#define OFFSET_MSB2LSB(x)                                       ((32 - x / 8 - 1) * 8 + ((x) % 8))

/* analyzer index */
static GT_U16                   udpSrcPort                      = 64212;
static GT_U16                   udpDstPort                      = 5247;

static TGF_PACKET_STC           packetInfo;
static CPSS_INTERFACE_INFO_STC  egrPortInterface;

static PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT   pktType[2]           = {PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E, PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E};
/* default ppu profile index */
static GT_U32 prvTgfPpuProfileIdx                               = 2;
/* SP Bus profile index */
static GT_U32 prvTgfSpBusProfileIdx0                            = 1;
/* default ppu State */
static GT_U32 prvTgfPpuState                                    = 0x41;

/* the Route entry index for UC Route entry Table */
static GT_U32        prvTgfRouteEntryBaseIndex                  = 8;
/* the leaf index in the RAM PBR section */
static GT_U32        prvTgfPbrLeafIndex                         = 964;
/* the Virtual Router index */
static GT_U32        prvUtfVrfId                                = 1;

/* Leaf entry orig */
static PRV_TGF_LPM_LEAF_ENTRY_STC leafEntryOrig;
/* Route entry orig */
static PRV_TGF_IP_UC_ROUTE_ENTRY_STC routeEntriesArrayOrig[1];

/****** Original Ingress Packet ******/
/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

static TGF_PACKET_L2_STC prvTgfOriginalPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x03}                /* saMac */
};

static TGF_PACKET_L2_STC prvTgfTunnelPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}                /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_DEFAULT_VLANID_CNS                    /* pri, cfi, VlanId */
};

/* IPv6 packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart_Ipv6 = {
    TGF_ETHERTYPE_86DD_IPV6_TAG_CNS
};

#define IPV6_DUMMY_PROTOCOL_CNS  0x3b   /* No next header */

/* Original packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfOriginalPacketIpv6Part = {
    6,                      /* version */
    0,                      /* trafficClass */
    0,                      /* flowLabel */
    0x1a,                   /* payloadLen */
    IPV6_DUMMY_PROTOCOL_CNS,/* nextHeader */
    0x40,                   /* hopLimit */
    {0x4001, 0x4002, 0x4003, 0x4004, 0x4005, 0x4006, 0x4007, 0x4008}, /* TGF_IPV6_ADDR srcAddr */
    {0x5001, 0x5002, 0x5003, 0x5004, 0x5005, 0x5006, 0x5007, 0x5008}  /* TGF_IPV6_ADDR dstAddr */
};

/* Tunnel packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfTunnelPacketIpv6Part = {
    6,                      /* version */
    0,                      /* trafficClass */
    0,                      /* flowLabel */
    0x1a,                   /* payloadLen */
    IPV6_DUMMY_PROTOCOL_CNS,/* nextHeader */
    0x40,                   /* hopLimit */
    {0x4001, 0x4002, 0x4003, 0x4004, 0x4005, 0x4006, 0x4007, 0x4008}, /* TGF_IPV6_ADDR srcAddr */
    {0x5001, 0x5002, 0x5003, 0x5004, 0x5005, 0x5006, 0x5007, 0x5008}  /* TGF_IPV6_ADDR dstAddr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfIpv6PacketArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfOriginalPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart_Ipv6},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfOriginalPacketIpv6Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV6_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfSRv6PacketArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfTunnelPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart_Ipv6},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfTunnelPacketIpv6Part},
    {TGF_PACKET_PART_L2_E,        &prvTgfOriginalPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart_Ipv6},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfOriginalPacketIpv6Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

static GT_U8 prvTgfTunnelStart[36] = {
    0x60, 0x0,  0x0,  0x1,
    0x60, 0x0,  0x0,  0x2,
    0x60, 0x0,  0x0,  0x3,
    0x60, 0x0,  0x0,  0x4,
    0x60, 0x0,  0x0,  0x5,
    0x60, 0x0,  0x0,  0x6,
    0x60, 0x0,  0x0,  0x7,
    0x60, 0x0,  0x0,  0x8,
    0x60, 0x0,  0x0,  0x9
};

/* IPv6 Packet for SRv6-GSID CoC32 End Node Processing */
static TGF_PACKET_IPV6_STC prvTgfGsidCoc32PacketIpv6Part = {
    6,                      /* version */
    0,                      /* trafficClass */
    0,                      /* flowLabel */
    0x1a,                   /* payloadLen */
    43,                     /* nextHeader */
    0x40,                   /* hopLimit */
    {0x4001, 0x4002, 0x4003, 0x4004, 0x4005, 0x4006, 0x4007, 0x4008}, /* TGF_IPV6_ADDR srcAddr */
    {0x0000, 0x0000, 0x0000, 0x0040, 0x1234, 0x5678, 0x0000, 0x0000}  /* TGF_IPV6_ADDR dstAddr */
};

/* packet's TCP part */
static TGF_PACKET_TCP_STC prvTgfGsidCoc32PacketIpv6TcpPart =
{
    8,                  /* src port */
    0,                  /* dst port */
    123456,             /* sequence number */
    234567,             /* acknowledgment number */
    5,                  /* data offset */
    0x50,               /* reserved */
    0x10,               /* flags */
    4096,               /* window */
    0xFAF6,             /* csum */
    0                   /* urgent pointer */
};

/* Data of packet */
static GT_U8 prvTgfGsidCoc32PacketPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19
};

/* Payload part */
static TGF_PACKET_PAYLOAD_STC prvTgfGsidCoc32CustomerPacketPayloadPart =
{
    sizeof(prvTgfGsidCoc32PacketPayloadDataArr), /* dataLength */
    prvTgfGsidCoc32PacketPayloadDataArr          /* dataPtr */
};

/* SRH for SRv6-GSID CoC32 End Node Processing */
static TGF_PACKET_SRH_STC prvTgfGsidCoc32PacketSrhPart = {
    6,                      /* nextHeader */
    6,                       /* headerLength */
    4,                       /* routingType */
    3,                       /* segmentsLeft */
    0,                       /* lastEntry */
    0,                       /* flags */
    0                        /* tag */
};

static GT_U8 prvTgfCoc32Gsid[48] = {
    0x11, 0x22,  0x33,  0x44,
    0x22, 0x33,  0x44,  0x55,
    0x33, 0x44,  0x55,  0x66,
    0x44, 0x55,  0x66,  0x77,
    0x55, 0x66,  0x77,  0x88,
    0x66, 0x77,  0x88,  0x99,
    0x77, 0x88,  0x99,  0xaa,
    0x88, 0x99,  0xaa,  0xbb,
    0x99, 0xaa,  0xbb,  0xcc,
    0xaa, 0xbb,  0xcc,  0xdd,
    0xbb, 0xcc,  0xdd,  0xee,
    0xcc, 0xdd,  0xee,  0xff
};

/* COC32 GSID PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfGsidCoc32PacketPayloadPart = {
    sizeof(prvTgfCoc32Gsid),                       /* dataLength */
    prvTgfCoc32Gsid                                /* dataPtr */
};


/* LENGTH of packet */
#define PRV_TGF_SRV6_GSID_COC32_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV6_HEADER_SIZE_CNS + TGF_SRH_HEADER_SIZE_CNS + sizeof(prvTgfCoc32Gsid)

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfGsidCoc32PacketIpv6PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfOriginalPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart_Ipv6},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfGsidCoc32PacketIpv6Part},
    {TGF_PACKET_PART_SRH_E,       &prvTgfGsidCoc32PacketSrhPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfGsidCoc32PacketPayloadPart},
    {TGF_PACKET_PART_TCP_E,       &prvTgfGsidCoc32PacketIpv6TcpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfGsidCoc32CustomerPacketPayloadPart}
};

/* IPv6 Packet for SRv6-GSID CoC32 End Node Processing Second Iter */
static TGF_PACKET_IPV6_STC prvTgfGsidCoc32PacketIpv6Part2 = {
    6,                      /* version */
    0,                      /* trafficClass */
    0,                      /* flowLabel */
    0x1a,                   /* payloadLen */
    43,                     /* nextHeader */
    0x3f,                   /* hopLimit */
    {0x4001, 0x4002, 0x4003, 0x4004, 0x4005, 0x4006, 0x4007, 0x4008}, /* TGF_IPV6_ADDR srcAddr */
    {0x0000, 0x0000, 0x0000, 0x0040, 0xccdd, 0xeeff, 0x0000, 0x0003}  /* TGF_IPV6_ADDR dstAddr */
};

/* SRH for SRv6-GSID CoC32 End Node Processing Second Iter */
static TGF_PACKET_SRH_STC prvTgfGsidCoc32PacketSrhPart2 = {
    6,                      /* nextHeader */
    6,                       /* headerLength */
    4,                       /* routingType */
    2,                       /* segmentsLeft */
    0,                       /* lastEntry */
    0,                       /* flags */
    0                        /* tag */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfGsidCoc32PacketIpv6PartArray2[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfOriginalPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart_Ipv6},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfGsidCoc32PacketIpv6Part2},
    {TGF_PACKET_PART_SRH_E,       &prvTgfGsidCoc32PacketSrhPart2},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfGsidCoc32PacketPayloadPart},
    {TGF_PACKET_PART_TCP_E,       &prvTgfGsidCoc32PacketIpv6TcpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfGsidCoc32CustomerPacketPayloadPart}
};

/******* Common Utils ******/
/**
* @internal prvTgSrv6BridgingConfigSet function
* @endinternal
*
* @brief   Bridging configuration for SRv6 use case (1 Segment)
*/
static GT_VOID prvTgSrv6BridgingConfigSet
(
    GT_PORT_NUM     nextHopPortNum,
    TGF_MAC_ADDR    *daMac
)
{
    PRV_TGF_BRG_MAC_ENTRY_STC       macEntry;
    GT_STATUS                       rc = GT_OK;

    /* create a macEntry with .daRoute = GT_TRUE */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, daMac, sizeof(TGF_MAC_ADDR));

    macEntry.key.entryType                  = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId         = PRV_TGF_DEFAULT_VLANID_CNS;
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum  = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum   = nextHopPortNum;
    macEntry.isStatic                       = GT_TRUE;
    macEntry.daCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.saCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.daRoute                        = GT_TRUE;

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 1, port 2 */
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite: %d", prvTgfDevNum);
}

/**
* @internal prvTgSrv6PhaThreadConfigSet function
* @endinternal
*
* @brief   PHA thread configuration for SRv6(ID and entry)
*/
static GT_VOID prvTgSrv6PhaThreadConfigSet
(
    GT_PORT_NUM                         portNum,
    GT_U32                              phaThreadId,
    IN CPSS_DXCH_PHA_THREAD_TYPE_ENT    extType,
    IN CPSS_DXCH_PHA_THREAD_INFO_UNT    *extInfoPtr
)
{
    GT_STATUS                               rc = GT_OK;
    CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC    commonInfo;

    cpssOsMemSet(&commonInfo, 0, sizeof(commonInfo));
    commonInfo.statisticalProcessingFactor  = 0;
    commonInfo.busyStallMode                = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;
    commonInfo.stallDropCode                = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;

    /* AUTODOC: Assign PHA thread ID to Port */
    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum, portNum, GT_TRUE, phaThreadId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaPortThreadIdSet FAILED for phaThreadId = [%d], rc = [%d]", phaThreadId, rc);

    /* AUTODOC: Set the thread entry */
    rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum, phaThreadId, &commonInfo, extType, extInfoPtr);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,"[TGF]: cpssDxChPhaThreadIdEntrySet FAILED, phaThreadId = [%d], extType = [%d], rc = [%d]", phaThreadId, extType, rc);
}

/*************************** Restore config ***********************************/
/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC           interface;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC                egressInfo;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC                egress1Info;
    PRV_TGF_TUNNEL_START_ENTRY_UNT                  tunnelEntry;
    GT_U32                                          srcLbProfile;
    GT_U32                                          trgLbProfile;
    GT_BOOL                                         lbEnArr[CPSS_TC_RANGE_CNS];
    CPSS_CSCD_PORT_TYPE_ENT                         cscdPortType;
    CPSS_INTERFACE_INFO_STC                         physicalInfo;
    PRV_TGF_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT       vlanEgressPortTagStateMode;
    PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT               vlanEgressPortTagState;
    GT_BOOL                                         savedFromCpuEn;
    GT_BOOL                                         savedFwdSingleEn;
    GT_BOOL                                         savedFwdMultiEn;
    GT_U32                                          tcQueueGet;
    GT_BOOL                                         lookupEnableGet;
    PRV_TGF_TTI_KEY_SIZE_ENT                        keySizeGet;
    PRV_TGF_NET_CPU_CODE_TABLE_ENTRY_STC            cpuCodeEntryGet;
    GT_BOOL                                         ppuEnable;
    GT_U32                                          ppuProfileIdx;
    CPSS_DXCH_PPU_PROFILE_ENTRY_STC                 ppuProfileEntry;
    CPSS_DXCH_PPU_SP_BUS_STC                        spBusProfile0;
    PRV_TGF_TTI_MAC_MODE_ENT                        origMacMode;
    GT_U32                                          vlanServiceIdGet;
    CPSS_DXCH_PHA_FW_IMAGE_ID_ENT                   phaFwImageIdGet;
    GT_BOOL                                         fdbRoutingPortEnableOrig;
    GT_BOOL                                         ipv4UcPortRoutingEnableOrig;
    GT_ETHERADDR                                    pbrArpMacAddrOrig;
    GT_ETHERADDR                                    fdbArpMacAddrOrig;
    CPSS_PACKET_CMD_ENT                             nhPacketCmdOrig;
    GT_BOOL                                         pbrBypassTriggerOrig;
    GT_BOOL                                         fdbUnicastRouteForPbrOrig;
    GT_ETHERADDR                                    macSaBaseOrig;
} prvTgfRestoreCfg;

/**
* @internal prvTgfSrv6OneSegmentEgressConfigSet function
* @endinternal
*
* @brief   Egress port TS IPv6 config for "SRv6 1 segment use case".
*/
static GT_VOID prvTgfSrv6OneSegmentEgressConfigSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC        egressInfo;
    PRV_TGF_TUNNEL_START_ENTRY_UNT          tunnelEntry, tunnelEntryGet;
    CPSS_TUNNEL_TYPE_ENT                    prvTgfTunnelType = CPSS_TUNNEL_GENERIC_IPV6_E;
    CPSS_TUNNEL_TYPE_ENT                    tunnelTypeGet;
    GT_U32                                  ii;

    /* AUTODOC: Egress tunnel SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Egress tunnel ipv6 Configuration =======\n");

    /* Save ePort attributes configuration */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                                                       prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                                       &(prvTgfRestoreCfg.egressInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoGet);

    /* Set ePort attributes configuration */
    cpssOsMemSet(&egressInfo,0,sizeof(egressInfo));
    egressInfo                        = prvTgfRestoreCfg.egressInfo;
    egressInfo.tunnelStart            = GT_TRUE;
    egressInfo.tunnelStartPtr         = PRV_TGF_TUNNEL_START_INDEX_CNS;
    egressInfo.tsPassengerPacketType  = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;

    /* AUTODOC: config ePort#2 attributes: */
    /* AUTODOC:   Tunnel Start=ENABLE, tunnelStartPtr=8 */
    /* AUTODOC:   tsPassenger=ETHERNET */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                        prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                        &egressInfo);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

    /* AUTODOC: Set Generic IPv6 tunnel start entry */
    PRV_UTF_LOG0_MAC("======= Setting Tunnel Start Configuration =======\n");
    cpssOsMemSet(&tunnelEntry.ipv6Cfg,0,sizeof(tunnelEntry.ipv6Cfg));
    /*** Set a tunnel start entry ***/
    tunnelEntry.ipv6Cfg.tagEnable        = GT_TRUE;
    tunnelEntry.ipv6Cfg.vlanId           = PRV_TGF_DEFAULT_VLANID_CNS;
    tunnelEntry.ipv6Cfg.ttl              = 33;
    tunnelEntry.ipv6Cfg.retainCrc        = GT_FALSE;
    tunnelEntry.ipv6Cfg.ipHeaderProtocol = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
    tunnelEntry.ipv6Cfg.udpSrcPort       = udpSrcPort;
    tunnelEntry.ipv6Cfg.udpDstPort       = udpDstPort;

    /* tunnel next hop MAC DA, IP DA, IP SA */
    cpssOsMemCpy(tunnelEntry.ipv6Cfg.macDa.arEther, prvTgfOriginalPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));

    for (ii = 0; ii < 8; ii++)
    {
        tunnelEntry.ipv6Cfg.destIp.arIP[ii * 2]     = (GT_U8)(prvTgf1stTunnelStartDstIPv6[ii] >> 8);
        tunnelEntry.ipv6Cfg.destIp.arIP[ii * 2 + 1] = (GT_U8) prvTgf1stTunnelStartDstIPv6[ii];
        tunnelEntry.ipv6Cfg.srcIp.arIP[ii * 2]     = (GT_U8)(prvTgf1stTunnelStartSrcIPv6[ii] >> 8);
        tunnelEntry.ipv6Cfg.srcIp.arIP[ii * 2 + 1] = (GT_U8) prvTgf1stTunnelStartSrcIPv6[ii];
    }

    /* AUTODOC: add TS entry 8 with: */
    /* AUTODOC:   tunnelType = GENERIC_IPV6 */
    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_INDEX_CNS, prvTgfTunnelType, &tunnelEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntrySet: %d", prvTgfDevNum);

    /* AUTODOC: get and check added TS Entry */
    rc = prvTgfTunnelStartEntryGet(prvTgfDevNum, PRV_TGF_TUNNEL_START_INDEX_CNS, &tunnelTypeGet, &tunnelEntryGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntryGet: %d", prvTgfDevNum);

    /* Check vlanId field */
    UTF_VERIFY_EQUAL1_STRING_MAC(tunnelEntryGet.ipv6Cfg.vlanId, tunnelEntry.ipv6Cfg.vlanId,
            "Vlan Id mismatch with configured value for device %d", prvTgfDevNum);

    /* Check srcIp field */
    rc = cpssOsMemCmp(&tunnelEntryGet.ipv6Cfg.srcIp, &tunnelEntry.ipv6Cfg.srcIp, sizeof(GT_IPV6ADDR)) == 0 ? GT_OK : GT_FALSE;
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tunnelEntry.ipv6Cfg.srcIp did not match");
}

/* debug flag to disable using of PHA for the test */
static GT_BOOL prvTgfSrv6OneSegmentSkipPha = GT_FALSE;
GT_BOOL prvTgfSrv6OneSegmentSkipPhaSet(GT_BOOL newSkipPha)
{
    GT_BOOL oldSkipPha = prvTgfSrv6OneSegmentSkipPha;

    prvTgfSrv6OneSegmentSkipPha = newSkipPha;

    return oldSkipPha;
}

/**
* internal prvTgfSrv6OneSegmentPhaConfigSet function
* @endinternal
*
* @brief   PHA config for SRv6 1 segment use case.
*/
static GT_VOID prvTgfSrv6OneSegmentPhaConfigSet
(
    GT_VOID
)
{
    CPSS_DXCH_PHA_THREAD_INFO_UNT           extInfo;
    GT_U32                                  ii;

    /* GM does not support PHA */
    if(GT_TRUE == prvUtfIsGmCompilation() || (prvTgfSrv6OneSegmentSkipPha == GT_TRUE))
        return;

    for (ii = 0; ii < 8; ii++)
    {
        extInfo.srv6SrcNode.srcAddr.arIP[ii * 2]     = (GT_U8)(prvTgfPhaTemplateIPv6[ii] >> 8);
        extInfo.srv6SrcNode.srcAddr.arIP[ii * 2 + 1] = (GT_U8) prvTgfPhaTemplateIPv6[ii];
    }

    /* AUTODOC: Assign PHA thread "SRv6 1 Segment" to egress port */
    prvTgSrv6PhaThreadConfigSet(prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
            PRV_TGF_PHA_THREAD_SRC_NODE_1_SEGMENT,
            CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_1_SEGMENT_E,
            &extInfo);
}

/**
* internal tgfSrv6OneSegmentConfigSet function
* @endinternal
*
* @brief   SRV6 One Segment use case configurations
*/
GT_VOID tgfSrv6OneSegmentConfigSet
(
    GT_VOID
)
{
    /* Bridging Engine Configuration
     * Route the packet from ingress to Egress */
    prvTgSrv6BridgingConfigSet(prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS], &prvTgfOriginalPacketL2Part.daMac);

    /* Add TS IPv6 at egress Port */
    prvTgfSrv6OneSegmentEgressConfigSet();

    /* PHA - Attach PHA Thread "1 segment" at Egress port */
    prvTgfSrv6OneSegmentPhaConfigSet();
}

/**
* internal tgfSrv6OneSegmentVerification function
* @endinternal
*
* @brief   SRv6 one segment use case verification
*/
GT_VOID tgfSrv6OneSegmentVerification
(
    GT_VOID
)
{
    GT_STATUS                           rc = GT_OK;
    GT_U8                               packetBuf[PRV_TGF_PACKET_SIZE_CNS] = {0};
    GT_U32                              packetLen = PRV_TGF_PACKET_SIZE_CNS;
    GT_U32                              packetActualLength = 0;
    GT_U8                               queue = 0;
    GT_U8                               dev = 0, ii;
    TGF_NET_DSA_STC                     rxParam;
    GT_BOOL                             getFirst = GT_TRUE;
    GT_U32                              pktOffset1, pktOffset = TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS;

    rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&egrPortInterface,
            TGF_PACKET_TYPE_CAPTURE_E,
            getFirst, GT_TRUE, packetBuf,
            &packetLen, &packetActualLength,
            &dev, &queue, &rxParam);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet: egrPortInterface %d", prvTgfDevNum);

    /* Add Tag length in case of tagged packet */
    pktOffset = TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS;

    /* IPv6 Next Header Verification - Should be SRH */
    pktOffset += 6;             /* Point to Next Header index */
    UTF_VERIFY_EQUAL0_STRING_MAC(43, packetBuf[pktOffset],
            "IPv6 Next Header is not matching ");

    /* AUTODOC: Tunnel SIP and DIP verification
     * DIP - Remains same as configured prvTgf1stTunnelStartDstIPv6
     * SIP - Replaced by the template value
     */
    pktOffset += 2;             /* Point to SIP index */
    pktOffset1 = pktOffset+16;  /* Point to DIP index */
    for (ii = 0; ii < 8; ii++)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfPhaTemplateIPv6[ii],
                (GT_U16)(packetBuf[pktOffset + 1] | (packetBuf[pktOffset] << 8)),
                "SIP is not matching at index[%d]", ii);
        UTF_VERIFY_EQUAL1_STRING_MAC(prvTgf1stTunnelStartDstIPv6[ii],
                (GT_U16)(packetBuf[pktOffset1+1] | (packetBuf[pktOffset1] << 8)),
                "DIP is not matching at index[%d]", ii);
        pktOffset+=2;
        pktOffset1+=2;
    }

    /*** Next_Hdr +++ Hdr_Ext_Len +++ Routing_Type +++ Segments_Left ***/
    /* Next_Hdr */
    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_L4_PROTOCOL_UDP_E,
            (GT_U8)(packetBuf[pktOffset1]),
            "Next_Hdr is not matching with ");
    pktOffset1+=1;

    /* Hdr_Ext_Len */
    UTF_VERIFY_EQUAL0_STRING_MAC(TGF_SEGMENT_LEN_CNS/TGF_HDR_LEN_EXT_UNIT_CNS,
            (GT_U8)(packetBuf[pktOffset1]),
            "Hdr_Len_Ext is not matching with ");
    pktOffset1+=1;

    /* Routing_Type */
    UTF_VERIFY_EQUAL0_STRING_MAC(4, /* Constant 4 */
            (GT_U8)(packetBuf[pktOffset1]),
            "Routing_Type is not matching with constant value");
    pktOffset1+=1;

    /* Segments_Left */
    UTF_VERIFY_EQUAL0_STRING_MAC(1, /* Num of segments left */
            (GT_U8)(packetBuf[pktOffset1]),
            "Segments_Left is not matching with constant value");
    pktOffset1+=1;

    /* AUTODOC: SRH Header last 4 Bytes should be 0 */
    for (ii = 0; ii < 4; ii++)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(0, (GT_U8)packetBuf[pktOffset1],
                "SRH base header is not matching at index[%d]", ii);
        pktOffset1+=1;
    }
}

/**
* @internal tgfSrv6OneSegmentConfigRestore function
* @endinternal
*
* @brief   SRv6 One segment use case configurations restore.
* @note    1. Restore PHA Configuration
*          2. Restore Bridge Configuration
*          3. Restore Base Configuration
*
*/
GT_VOID tgfSrv6OneSegmentConfigRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* Restore PHA Configuration */
    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                        GT_FALSE, PRV_TGF_PHA_THREAD_SRC_NODE_1_SEGMENT);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaPortThreadIdSet FAILED for SRC_NODE_1_SEGMENT, rc = [%d]", rc);

    /* AUTODOC: Restore ePort attributes */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                        prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                        &(prvTgfRestoreCfg.egressInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "tgfTrafficTableRxPcktTblClear");

    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "tgfTrafficTableRxStartCapture");
}

/**
* @internal tgfSrv6OneSegmentTest function
* @endinternal
*
* @brief   SRv6 One segment use case test.
*/
GT_VOID tgfSrv6OneSegmentTest
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIter;

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum,
                    prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "prvTgfResetCountersEth: %d, %d",
                    prvTgfDevNum,
                    prvTgfPortsArray[portIter]);
    }

    packetInfo.numOfParts =  sizeof(prvTgfIpv6PacketArray) / sizeof(prvTgfIpv6PacketArray[0]);
    packetInfo.partsArray = prvTgfIpv6PacketArray;
    packetInfo.totalLen   = TGF_PACKET_AUTO_CALC_LENGTH_CNS;

    /* setup receive portInterface for capturing */
    egrPortInterface.type               = CPSS_INTERFACE_PORT_E;
    egrPortInterface.devPort.hwDevNum   = prvTgfDevNum;
    egrPortInterface.devPort.portNum    = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    /* AUTODOC: GENERATE TRAFFIC & Verify at ePort#3 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
                        &egrPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgftrafficgeneratorporttxethcaptureset port - %d",
                                        egrPortInterface.devPort.portNum);

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, 1, 0, NULL);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfSetTxSetupEth");

    /* Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");

    /* disable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &egrPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet port - %d",
            egrPortInterface.devPort.portNum);

    /* AUTODOC: Verification Started */
    tgfSrv6OneSegmentVerification();
}

/***********************************************************/
/************* SRv6 Two/Three Segment Use case *************/
/***********************************************************/

/**
* @internal prvTgfSrv6TwoThreeSegmentLoopBackConfigSet function
* @endinternal
*
* @brief   Loop back config for "SRv6 Two/Three segment use case".
*/
static GT_VOID prvTgfSrv6TwoThreeSegmentLoopBackConfigSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    CPSS_INTERFACE_INFO_STC                 portInterface;
    GT_U32                                  i;

    PRV_UTF_LOG0_MAC("======= Setting Loopback Configuration =======\n");
    /* AUTODOC: save Ingress port loopback source profile */
    rc = cpssDxChPortLoopbackProfileGet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
            CPSS_DIRECTION_INGRESS_E,
            &prvTgfRestoreCfg.srcLbProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackProfileGet");

    /* AUTODOC: set Ingress port loopback profile {source #1} */
    rc = cpssDxChPortLoopbackProfileSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
            CPSS_DIRECTION_INGRESS_E,
            PRV_TGF_LB_PROFILE_1_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackProfileSet");

    /* AUTODOC: save analyzer port loopback target profile */
    rc = cpssDxChPortLoopbackProfileGet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
            CPSS_DIRECTION_EGRESS_E,
            &prvTgfRestoreCfg.trgLbProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackProfileGet");

    /* AUTODOC: set analyzer port loopback profile {target #1} */
    rc = cpssDxChPortLoopbackProfileSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
            CPSS_DIRECTION_EGRESS_E,
            PRV_TGF_LB_PROFILE_1_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackProfileSet");

    for (i = 0; i < CPSS_TC_RANGE_CNS; i++)
    {
        /* AUTODOC: save general FTL trigger status */
        rc = cpssDxChPortLoopbackEnableGet(prvTgfDevNum,
                                           PRV_TGF_LB_PROFILE_1_CNS,
                                           PRV_TGF_LB_PROFILE_1_CNS,
                                           i,
                                           &prvTgfRestoreCfg.lbEnArr[i]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackEnableGet, i:%d", i);

        /* AUTODOC: enable FTL for loopback profiles {source #1, target #1} & for all traffic classes */
        rc = cpssDxChPortLoopbackEnableSet(prvTgfDevNum,
                                           PRV_TGF_LB_PROFILE_1_CNS,
                                           PRV_TGF_LB_PROFILE_1_CNS,
                                           i,
                                           GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackEnableSet, i:%d", i);
    }

    /* AUTODOC: enable FTL for single-target FROM_CPU packet type */
    rc = cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableGet(prvTgfDevNum,
                            &prvTgfRestoreCfg.savedFromCpuEn,
                            &prvTgfRestoreCfg.savedFwdSingleEn,
                            &prvTgfRestoreCfg.savedFwdMultiEn);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    rc = cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableSet(prvTgfDevNum, GT_FALSE, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableSet");

    /* AUTODOC: assign egress port with loopback target port */
    rc = cpssDxChPortLoopbackPktTypeForwardAndFromCpuSet(
                                    prvTgfDevNum,
                                    prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                    prvTgfPortsArray[PRV_TGF_LOOPBACK_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS], prvTgfPortsArray[PRV_TGF_LOOPBACK_PORT_IDX_CNS]);

    /* AUTODOC: Enable the internal Loopback state in the packet processor MAC port */
    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));
    portInterface.type             = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_LOOPBACK_PORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(&portInterface, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficGeneratorPortLoopbackModeEnableSet,"
                                            " portNum:%d", portInterface.devPort.portNum);
}

/**
* @internal prvTgfSrv6TwoThreeSegmentHaConfigSet function
* @endinternal
*
* @brief   TS config for "SRv6 Two/Three segment use case".
*/
static GT_VOID prvTgfSrv6TwoThreeSegmentHaConfigSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC        egress1Info;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC        egressInfo;
    PRV_TGF_TUNNEL_START_ENTRY_UNT          tunnelEntry;
    PRV_TGF_TUNNEL_START_ENTRY_UNT          tunnelEntryGet;
    CPSS_TUNNEL_TYPE_ENT                    tunnelTypeGet;
    GT_U32                                  ii;
    CPSS_INTERFACE_INFO_STC                 physicalInfo;

    /* AUTODOC: Save loopback port cscd type */
    rc = cpssDxChCscdPortTypeGet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_LOOPBACK_PORT_IDX_CNS],
                                 CPSS_PORT_DIRECTION_BOTH_E,
                                 &prvTgfRestoreCfg.cscdPortType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCscdPortTypeGet");

    /* AUTODOC: Set loopback port as cascade to add 16 bytes of eDSA tag */
    rc = cpssDxChCscdPortTypeSet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_LOOPBACK_PORT_IDX_CNS],
                                 CPSS_PORT_DIRECTION_BOTH_E,
                                 CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCscdPortTypeSet");

    /***********************************************/
    /* ePort Mapping : Egress_port = "1+egress_port" */
    /***********************************************/
    /* AUTODOC: save ePort mapping configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                                           PRV_TGF_SECOND_PASS_EGR_EPORT_CNS,
                                                           &(prvTgfRestoreCfg.physicalInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableGet);

    /* AUTODOC: set ePort mapping configuration */
    cpssOsMemSet(&physicalInfo, 0, sizeof(physicalInfo));
    physicalInfo.type               = CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.hwDevNum   = prvTgfDevNum;
    physicalInfo.devPort.portNum    = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    /* AUTODOC: map Egress to ePort (1+Egress) */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           PRV_TGF_SECOND_PASS_EGR_EPORT_CNS,
                                                           &physicalInfo);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableSet);

    /***********************************************/
    /* Egress Port TS = Generic IPV6 */
    /***********************************************/
    /* AUTODOC: Egress port TS SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Egress tunnel ipv6 Configuration =======\n");

    /* AUTODOC: Save ePort attributes configuration */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                                                       prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                                       &(prvTgfRestoreCfg.egressInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoGet);

    /* AUTODOC: Set ePort attributes configuration */
    cpssOsMemSet(&egressInfo, 0, sizeof(egressInfo));
    egressInfo                        = prvTgfRestoreCfg.egressInfo;
    egressInfo.tunnelStart            = GT_TRUE;
    egressInfo.tunnelStartPtr         = PRV_TGF_TUNNEL_START_INDEX_CNS;
    egressInfo.tsPassengerPacketType  = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;

    /* AUTODOC: config ePort#2 attributes: */
    /* AUTODOC:   Tunnel Start=ENABLE, tunnelStartPtr=8 */
    /* AUTODOC:   tsPassenger=ETHERNET */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                        prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                        &egressInfo);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

    /* AUTODOC: Set Generic IPv6 tunnel start entry */
    PRV_UTF_LOG0_MAC("======= Setting Tunnel Start Configuration =======\n");
    cpssOsMemSet(&tunnelEntry.ipv6Cfg,0,sizeof(tunnelEntry.ipv6Cfg));
    /*** Set a tunnel start entry ***/
    tunnelEntry.ipv6Cfg.tagEnable        = GT_TRUE;
    tunnelEntry.ipv6Cfg.vlanId           = PRV_TGF_DEFAULT_VLANID_CNS;
    tunnelEntry.ipv6Cfg.upMarkMode       = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tunnelEntry.ipv6Cfg.dscpMarkMode     = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tunnelEntry.ipv6Cfg.ttl              = 33;
    tunnelEntry.ipv6Cfg.retainCrc        = GT_FALSE;
    tunnelEntry.ipv6Cfg.ipHeaderProtocol = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_IP_E;
    tunnelEntry.ipv6Cfg.ipProtocol       = 43;  /* Ignored */
    tunnelEntry.ipv6Cfg.udpSrcPort       = udpSrcPort;
    tunnelEntry.ipv6Cfg.udpDstPort       = udpDstPort;

    /* tunnel next hop MAC DA, IP DA, IP SA */
    cpssOsMemCpy(tunnelEntry.ipv6Cfg.macDa.arEther, prvTgf2ndTunnelL2Part.daMac, sizeof(TGF_MAC_ADDR));

    for (ii = 0; ii < 8; ii++)
    {
        tunnelEntry.ipv6Cfg.destIp.arIP[ii * 2]     = (GT_U8)(prvTgf1stTunnelStartDstIPv6[ii] >> 8);
        tunnelEntry.ipv6Cfg.destIp.arIP[ii * 2 + 1] = (GT_U8) prvTgf1stTunnelStartDstIPv6[ii];
        tunnelEntry.ipv6Cfg.srcIp.arIP[ii * 2]     = (GT_U8)(prvTgf1stTunnelStartSrcIPv6[ii] >> 8);
        tunnelEntry.ipv6Cfg.srcIp.arIP[ii * 2 + 1] = (GT_U8) prvTgf1stTunnelStartSrcIPv6[ii];
    }

    /* AUTODOC: add TS entry 8 with: */
    /* AUTODOC:   tunnelType = GENERIC_IPV6 */
    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_INDEX_CNS, CPSS_TUNNEL_GENERIC_IPV6_E, &tunnelEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntrySet: %d", prvTgfDevNum);

    /* AUTODOC: get and check added TS Entry */
    rc = prvTgfTunnelStartEntryGet(prvTgfDevNum, PRV_TGF_TUNNEL_START_INDEX_CNS, &tunnelTypeGet, &tunnelEntryGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntryGet: %d", prvTgfDevNum);

    /* Check vlanId field */
    UTF_VERIFY_EQUAL1_STRING_MAC(tunnelEntryGet.ipv6Cfg.vlanId, tunnelEntry.ipv6Cfg.vlanId,
            "Vlan Id mismatch with configured value for device %d", prvTgfDevNum);

    /* Check srcIp field */
    rc = cpssOsMemCmp(&tunnelEntryGet.ipv6Cfg.srcIp, &tunnelEntry.ipv6Cfg.srcIp, sizeof(GT_IPV6ADDR)) == 0 ? GT_OK : GT_FALSE;
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tunnelEntry.ipv6Cfg.srcIp did not match");

    /***********************************************/
    /* Egress Port(ePort+1) TS = Generic TS */
    /***********************************************/
    /* AUTODOC: Egress ePort+1 TS SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Egress tunnel ipv6 Configuration =======\n");

    /* AUTODOC: Save ePort attributes configuration */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                                                       PRV_TGF_SECOND_PASS_EGR_EPORT_CNS,
                                                       &(prvTgfRestoreCfg.egress1Info));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoGet);

    /* AUTODOC: Set ePort attributes configuration */
    cpssOsMemSet(&egress1Info,0,sizeof(egress1Info));
    egress1Info                        = prvTgfRestoreCfg.egress1Info;
    egress1Info.tunnelStart            = GT_TRUE;
    egress1Info.tunnelStartPtr         = PRV_TGF_TUNNEL_START_INDEX_CNS+8;
    egress1Info.tsPassengerPacketType  = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;

    /* AUTODOC: config ePort#2 attributes: */
    /* AUTODOC:   Tunnel Start=ENABLE, tunnelStartPtr=8+8 */
    /* AUTODOC:   tsPassenger=ETHERNET */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                        PRV_TGF_SECOND_PASS_EGR_EPORT_CNS,
                                        &egress1Info);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

    /* AUTODOC: Set Tunnel Start entry 8+8 with: */
    /* AUTODOC: genericType = long, tagEnable = TRUE, vlanId=1 */
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));
    tunnelEntry.genCfg.tagEnable            = GT_TRUE;
    tunnelEntry.genCfg.vlanId               = PRV_TGF_DEFAULT_VLANID_CNS;
    tunnelEntry.genCfg.genericType          = PRV_TGF_TUNNEL_START_GENERIC_LONG_TYPE_E;
    tunnelEntry.genCfg.etherType            = TGF_ETHERTYPE_6666_VLAN_TAG_CNS;
    cpssOsMemCpy(tunnelEntry.genCfg.data, prvTgfTunnelStart, sizeof(prvTgfTunnelStart));
    cpssOsMemCpy(tunnelEntry.genCfg.macDa.arEther, prvTgf1stTunnelL2Part.daMac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_INDEX_CNS+8, CPSS_TUNNEL_GENERIC_E, &tunnelEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartEntrySet");

    /* AUTODOC: set EPORT tag state mode for egress port */
    rc = prvTgfBrgVlanEgressPortTagStateModeGet(prvTgfDevNum,
                                                PRV_TGF_SECOND_PASS_EGR_EPORT_CNS,
                                                &(prvTgfRestoreCfg.vlanEgressPortTagStateMode));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateModeGet);

    rc = prvTgfBrgVlanEgressPortTagStateModeSet(prvTgfDevNum,
                                                PRV_TGF_SECOND_PASS_EGR_EPORT_CNS,
                                                PRV_TGF_BRG_VLAN_EPORT_TAG_STATE_EPORT_MODE_E);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateModeSet);

    /* AUTODOC: set TAG0_CMD port tag state for egress port  */
    rc = prvTgfBrgVlanEgressPortTagStateGet(prvTgfDevNum,
                                            PRV_TGF_SECOND_PASS_EGR_EPORT_CNS,
                                            &(prvTgfRestoreCfg.vlanEgressPortTagState));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateGet);

    rc = prvTgfBrgVlanEgressPortTagStateSet(prvTgfDevNum,
                                            PRV_TGF_SECOND_PASS_EGR_EPORT_CNS,
                                            pktType[0]);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateSet);
}

/**
* internal prvTgfSrv6TwoThreeSegmentPhaConfigSet function
* @endinternal
*
* @brief   PHA config for SRv6 Two/Three segment use case.
*/
static GT_VOID prvTgfSrv6TwoThreeSegmentPhaConfigSet
(
    CPSS_DXCH_PHA_THREAD_TYPE_ENT       secondPassThread
)
{
    CPSS_DXCH_PHA_THREAD_INFO_UNT           extInfo;
    GT_U32                                  ii;

    /* GM does not support PHA */
    if(GT_TRUE == prvUtfIsGmCompilation() || (prvTgfSrv6OneSegmentSkipPha == GT_TRUE))
        return;

    /* AUTODOC: Assign PHA thread "SRv6 SECOND_PASS_2_SEGMENTS" to egress "ePort+1" */
    cpssOsMemSet(&extInfo, 0, sizeof(extInfo));
    prvTgSrv6PhaThreadConfigSet(PRV_TGF_SECOND_PASS_EGR_EPORT_CNS,
                                PRV_TGF_PHA_THREAD_SRC_NODE_SECOND_PASS_2_SEGMENTS,
                                secondPassThread,
                                &extInfo);

    /* AUTODOC: Assign PHA thread "SRv6 FIRST_PASS_2_3_SEGMENTS" to egress Port */
    for (ii = 0; ii < 8; ii++)
    {
        extInfo.srv6SrcNode.srcAddr.arIP[ii * 2]     = (GT_U8)(prvTgfPhaTemplateIPv6[ii] >> 8);
        extInfo.srv6SrcNode.srcAddr.arIP[ii * 2 + 1] = (GT_U8) prvTgfPhaTemplateIPv6[ii];
    }

    prvTgSrv6PhaThreadConfigSet(prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                PRV_TGF_PHA_THREAD_SRC_NODE_FIRST_PASS_2_3_SEGMENTS,
                                CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_FIRST_PASS_2_3_SEGMENTS_E,
                                &extInfo);
}

/**
* internal tgfSrv6TwoSegmentConfigSet function
* @endinternal
*
* @brief   SRV6 Two Segment use case configurations
*/
GT_VOID tgfSrv6TwoSegmentConfigSet
(
    GT_VOID
)
{
    /* Bridging Engine Configuration
     * Route the packet from ingress to egress port */
    prvTgSrv6BridgingConfigSet(prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS], &prvTgfOriginalPacketL2Part.daMac);

    /* Loop Back configuration */
    prvTgfSrv6TwoThreeSegmentLoopBackConfigSet();

    /* Add TS IPv6 at egress ePort */
    prvTgfSrv6TwoThreeSegmentHaConfigSet();

    /* PHA - Attach PHA Thread "2 segment" at Egress port */
    prvTgfSrv6TwoThreeSegmentPhaConfigSet(CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_2_SEGMENTS_E);
}

/**
* internal tgfSrv6TwoThreeSegmentVerification function
* @endinternal
*
* @brief   SRv6 segment use case verification
*/
GT_VOID tgfSrv6TwoThreeSegmentVerification
(
    CPSS_DXCH_PHA_THREAD_TYPE_ENT           threadType
)
{
    GT_STATUS                           rc = GT_OK;
    GT_U8                               packetBuf[PRV_TGF_PACKET_SIZE_CNS] = {0};
    GT_U32                              packetLen = PRV_TGF_PACKET_SIZE_CNS;
    GT_U32                              packetActualLength = 0;
    GT_U8                               queue = 0;
    GT_U8                               dev = 0, ii, pktCnt, secondPassSegmentCnt, loopEnd;
    TGF_NET_DSA_STC                     rxParam;
    GT_BOOL                             getFirst = GT_TRUE;
    GT_U32                              pktOffset1, pktOffset = TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS;

    for(pktCnt = 0; pktCnt < 2; pktCnt++)
    {
        /* Add Tag length in case of tagged packet */
        if(pktType[pktCnt] != PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E)
        {
            pktOffset = TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS;
        }
        else
        {
            pktOffset = TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS;
        }

        PRV_UTF_LOG0_MAC("\nPacket at Egress interface\n");
        rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&egrPortInterface,
                TGF_PACKET_TYPE_CAPTURE_E,
                getFirst, GT_TRUE, packetBuf,
                &packetLen, &packetActualLength,
                &dev, &queue, &rxParam);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet: egrPortInterface %d", prvTgfDevNum);
        getFirst = GT_FALSE;
        /* IPv6 Next Header Verification - Should be SRH */
        pktOffset += 6;             /* Point to Next Header index */
        UTF_VERIFY_EQUAL1_STRING_MAC(43, packetBuf[pktOffset],
                "IPv6 Next Header is not matching pktCnt[%d]", pktCnt);

        /* AUTODOC: Tunnel SIP and DIP verification
         * DIP - Remains same as configured prvTgf1stTunnelStartDstIPv6
         * SIP - Replaced by the template value
         */
        pktOffset += 2;             /* Point to SIP index */
        pktOffset1 = pktOffset+16;  /* Point to DIP index */

        for (ii = 0; ii < 8; ii++)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfPhaTemplateIPv6[ii],
                    (GT_U16)(packetBuf[pktOffset + 1] | (packetBuf[pktOffset] << 8)),
                    "SIP is not matching at index[%d], pktCnt[%d]", ii, pktCnt);
            UTF_VERIFY_EQUAL2_STRING_MAC(prvTgf1stTunnelStartDstIPv6[ii],
                    (GT_U16)(packetBuf[pktOffset1+1] | (packetBuf[pktOffset1] << 8)),
                    "DIP is not matching at index[%d] pktCnt[%d]", ii, pktCnt);
            pktOffset+=2;
            pktOffset1+=2;
        }

        /* SRH Next Header Verification - Assuming IPv6 over IPv6 tunnel */
        UTF_VERIFY_EQUAL1_STRING_MAC(41, packetBuf[pktOffset1],
                "SRH Next Header is not matching pktCnt[%d]", pktCnt);
        pktOffset1++;

        /* Generic tunnel start etherType Verification for 3 Segment */
        UTF_VERIFY_EQUAL0_STRING_MAC(TGF_ETHERTYPE_6666_VLAN_TAG_CNS,
                (GT_U16)(packetBuf[pktOffset1 + 1] | (packetBuf[pktOffset1] << 8)),
                "Generic tunnel start etherType is not matching");
        pktOffset1+=2;

        /* VLAN ID Verification (LSB only) */
        UTF_VERIFY_EQUAL0_STRING_MAC((PRV_TGF_DEFAULT_VLANID_CNS & 0xFF),
                packetBuf[pktOffset1],
                "VLAN ID is not matching");
        pktOffset1++;

        /* AUTODOC: SRH Base Verification */
        for (ii = 0; ii < 4; ii++)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfTunnelStart[ii], (GT_U8)packetBuf[pktOffset1],
                    "SRH base header is not matching at index[%d] pktCnt[%d]", ii, pktCnt);
            pktOffset1+=1;
        }


        if(threadType == CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_3_SEGMENTS_E)
        {
            secondPassSegmentCnt = 2;
        }
        else
        {
            ii +=16;
            secondPassSegmentCnt = 1;
        }

        /* AUTODOC: Segment 1 Verification */
        loopEnd = (ii + (secondPassSegmentCnt * TGF_SEGMENT_LEN_CNS));
        for (;ii < loopEnd; ii++)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfTunnelStart[ii], (GT_U8)packetBuf[pktOffset1],
                    "Segment0 is not matching at index[%d], pktCnt[%d]", ii-8, pktCnt);
            pktOffset1+=1;
        }

        /* AUTODOC: Segment 2 Verification */
        for (ii = 0; ii < 8; ii++)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(prvTgf1stTunnelStartSrcIPv6[ii],
                    (GT_U16)(packetBuf[pktOffset1 + 1] | (packetBuf[pktOffset1] << 8)),
                    "Segment0 is not matching at index[%d], pktCnt[%d]", ii-8, pktCnt);
            pktOffset1+=2;
        }
    }
}

/**
* internal tgfSrv6TwoThreeSrhContainersVerification function
* @endinternal
*
* @brief   SRv6 Two/Three SRH Containers use case verification
*/
GT_VOID tgfSrv6TwoThreeSrhContainersVerification
(
    CPSS_DXCH_PHA_THREAD_TYPE_ENT           threadType
)
{
    GT_STATUS                           rc = GT_OK;
    GT_U8                               packetBuf[PRV_TGF_PACKET_SIZE_CNS] = {0};
    GT_U32                              packetLen = PRV_TGF_PACKET_SIZE_CNS;
    GT_BOOL                             getFirst = GT_TRUE;
    GT_U32                              packetActualLength = 0;
    GT_U8                               queue = 0;
    GT_U8                               dev = 0, ii, pktCnt = 0, secondPassSegmentCnt, loopEnd, loopStart;
    TGF_NET_DSA_STC                     rxParam;
    GT_U32                              pktOffset1, pktOffset = TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS;
    GT_U32                              hdrExtLen, segmentsLeft;

    for(pktCnt = 0; pktCnt < 2; pktCnt++)
    {
        /* Add Tag length in case of tagged packet */
        if(pktType[pktCnt] != PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E)
        {
            pktOffset = TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS;
        }
        else
        {
            pktOffset = TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS;
        }

        PRV_UTF_LOG0_MAC("\nPacket at Egress interface\n");
        rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&egrPortInterface,
                TGF_PACKET_TYPE_CAPTURE_E,
                getFirst, GT_TRUE, packetBuf,
                &packetLen, &packetActualLength,
                &dev, &queue, &rxParam);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet: egrPortInterface %d", prvTgfDevNum);
        getFirst = GT_FALSE;

        /* IPv6 Next Header Verification - Should be SRH */
        pktOffset += 6;             /* Point to Next Header index */
        UTF_VERIFY_EQUAL1_STRING_MAC(43, packetBuf[pktOffset],
                "IPv6 Next Header is not matching pktCnt[%d]", pktCnt);

        /* AUTODOC: Tunnel SIP and DIP verification
         * DIP - Remains same as configured prvTgf1stTunnelStartDstIPv6
         * SIP - Replaced by the template value
         */
        pktOffset += 2;             /* Point to SIP index */
        pktOffset1 = pktOffset+16;  /* Point to DIP index */

        for (ii = 0; ii < 8; ii++)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfPhaTemplateIPv6[ii],
                    (GT_U16)(packetBuf[pktOffset + 1] | (packetBuf[pktOffset] << 8)),
                    "SIP is not matching at index[%d]", ii);
            if(ii <= 3)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(prvTgf1stTunnelStartDstIPv6[ii],
                        (GT_U16)(packetBuf[pktOffset1+1] | (packetBuf[pktOffset1] << 8)),
                        "DIP Locator is not matching at index[%d]", ii);
                pktOffset1+=2;
            }
            pktOffset+=2;
        }

        ii = 4;
        UTF_VERIFY_EQUAL1_STRING_MAC(PRV_TGF_GSID_NODE_INDEX_CNS,
                (GT_U16)(packetBuf[pktOffset1+1] | (packetBuf[pktOffset1] << 8)),
                "DIP Node ID is not matching at index[%d]", ii);

        ii++;
        pktOffset1+=2;
        UTF_VERIFY_EQUAL1_STRING_MAC(PRV_TGF_GSID_FUNCTION_INDEX_CNS,
                (GT_U16)(packetBuf[pktOffset1+1] | (packetBuf[pktOffset1] << 8)),
                "DIP Function ID is not matching at index[%d]", ii);

        ii++;
        pktOffset1+=2;
        UTF_VERIFY_EQUAL1_STRING_MAC(PRV_TGF_SOURCE_INDEX_CNS,
                (GT_U16)(packetBuf[pktOffset1+1] | (packetBuf[pktOffset1] << 8)),
                "DIP SourceId is not matching at index[%d]", ii);

        ii++;
        pktOffset1+=2;
        UTF_VERIFY_EQUAL1_STRING_MAC(0x0,
                (GT_U16)(packetBuf[pktOffset1+1] | (packetBuf[pktOffset1] << 8)),
                "DIP AppId is not matching at index[%d]", ii);

        pktOffset1+=2;

        /* SRH Next Header Verification - Assuming IPv6 over IPv6 tunnel */
        UTF_VERIFY_EQUAL1_STRING_MAC(0x2b, packetBuf[pktOffset1],
                "SRH Next Header is not matching at index[%d]", pktOffset1);
        pktOffset1++;

        hdrExtLen = (threadType == CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_2_CONTAINERS_E) ?
                    TGF_TWO_SRH_CONTAINERS_LEN_CNS/TGF_HDR_LEN_EXT_UNIT_CNS :
                    TGF_THREE_SRH_CONTAINERS_LEN_CNS/TGF_HDR_LEN_EXT_UNIT_CNS;

        /* Hdr_Ext_Len */
        UTF_VERIFY_EQUAL0_STRING_MAC(hdrExtLen,
                packetBuf[pktOffset1],
                "SRH Hdr Len Ext is not matching");
        pktOffset1++;

        /* Routing_Type */
        UTF_VERIFY_EQUAL0_STRING_MAC(4, /* constant */
                packetBuf[pktOffset1],
                "SRH Routing Type is not matching");
        pktOffset1++;

        /* Segments_Left */
        segmentsLeft = (threadType == CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_2_CONTAINERS_E) ?
                       TGF_TWO_SRH_CONTAINERS_LEN_CNS/TGF_SEGMENT_LEN_CNS :
                       TGF_THREE_SRH_CONTAINERS_LEN_CNS/TGF_SEGMENT_LEN_CNS;
        UTF_VERIFY_EQUAL0_STRING_MAC(segmentsLeft, /* Num of segments left */
                (GT_U8)(packetBuf[pktOffset1]),
                "Segments_Left is not matching with constant value");
        pktOffset1+=1;

        /* AUTODOC: SRH Base Verification */
        for (ii = 0; ii < 4; ii++)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfTunnelStart[ii], (GT_U8)packetBuf[pktOffset1],
                    "SRH base header is not matching at index[%d]", ii);
            pktOffset1+=1;
        }

        if(threadType == CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_2_CONTAINERS_E)
        {
            secondPassSegmentCnt = 1;
        }
        else
        {
            secondPassSegmentCnt = 0;
        }

        /* AUTODOC: Segment 1 Verification */
        loopStart = ii + (secondPassSegmentCnt * TGF_SEGMENT_LEN_CNS);
        loopEnd   = sizeof(prvTgfTunnelStart)/sizeof(prvTgfTunnelStart[0]);
        for (ii = loopStart; ii < loopEnd; ii++)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfTunnelStart[ii], (GT_U8)packetBuf[pktOffset1],
                    "Segment0 is not matching at index[%d]", ii);
            pktOffset1+=1;
        }

        /* AUTODOC: Segment 2 Verification */
        for (ii = 0; ii < 8; ii++)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgf1stTunnelStartSrcIPv6[ii],
                    (GT_U16)(packetBuf[pktOffset1 + 1] | (packetBuf[pktOffset1] << 8)),
                    "Segment0 is not matching at index[%d]", ii-8);
            pktOffset1+=2;
        }
    }
}

/**
* @internal tgfSrv6TwoThreeSegmentConfigRestore function
* @endinternal
*
* @brief   SRv6 Two segment use case configurations restore.
* @note    1. Restore PHA Configuration
*          2. Restore Bridge Configuration
*          3. Restore Base Configuration
*
*/
GT_VOID tgfSrv6TwoThreeSegmentConfigRestore
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  i;
    CPSS_INTERFACE_INFO_STC                 portInterface;

    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           PRV_TGF_SECOND_PASS_EGR_EPORT_CNS,
                                                           &prvTgfRestoreCfg.physicalInfo);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableSet);

    /* Loop Back config */
    rc = cpssDxChPortLoopbackProfileSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
            CPSS_DIRECTION_INGRESS_E,
            prvTgfRestoreCfg.srcLbProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackProfileSet");

    rc = cpssDxChPortLoopbackProfileSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
            CPSS_DIRECTION_EGRESS_E,
            prvTgfRestoreCfg.trgLbProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackProfileSet");

    /* AUTODOC: Restore cascade Type */
    rc = cpssDxChCscdPortTypeSet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_LOOPBACK_PORT_IDX_CNS],
                                 CPSS_PORT_DIRECTION_BOTH_E,
                                 prvTgfRestoreCfg.cscdPortType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCscdPortTypeSet");

    for (i = 0; i < CPSS_TC_RANGE_CNS; i++)
    {
        /* AUTODOC: enable FTL for loopback profiles {source #1, target #1} & for all traffic classes */
        rc = cpssDxChPortLoopbackEnableSet(prvTgfDevNum,
                                           PRV_TGF_LB_PROFILE_1_CNS,
                                           PRV_TGF_LB_PROFILE_1_CNS,
                                           i,
                                           prvTgfRestoreCfg.lbEnArr[i]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackEnableSet, i:%d", i);
    }

    /* AUTODOC: enable FTL for single-target FROM_CPU packet type */
    rc = cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableSet(prvTgfDevNum,
                            prvTgfRestoreCfg.savedFromCpuEn,
                            prvTgfRestoreCfg.savedFwdSingleEn,
                            prvTgfRestoreCfg.savedFwdMultiEn);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: Enable the internal Loopback state in the packet processor MAC port */
    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));
    portInterface.type             = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_LOOPBACK_PORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(&portInterface, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficGeneratorPortLoopbackModeEnableSet,"
                                            " portNum:%d", portInterface.devPort.portNum);

    /* Restore PHA Configuration */
    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum, PRV_TGF_SECOND_PASS_EGR_EPORT_CNS,
                                        GT_FALSE, PRV_TGF_PHA_THREAD_SRC_NODE_SECOND_PASS_2_SEGMENTS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaPortThreadIdSet FAILED for SRC_NODE_SECOND_PASS_2, rc = [%d]", rc);

    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                        GT_FALSE, PRV_TGF_PHA_THREAD_SRC_NODE_FIRST_PASS_2_3_SEGMENTS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaPortThreadIdSet FAILED for SRC_NODE_FIRST_PASS_2_3_SEGMENT, rc = [%d]", rc);

    /* AUTODOC: Restore ePort attributes */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                        prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                        &prvTgfRestoreCfg.egressInfo);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                        PRV_TGF_SECOND_PASS_EGR_EPORT_CNS,
                                        &prvTgfRestoreCfg.egress1Info);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);



    /* AUTODOC: set EPORT tag state mode for egress port */
    rc = prvTgfBrgVlanEgressPortTagStateModeSet(prvTgfDevNum,
                                                PRV_TGF_SECOND_PASS_EGR_EPORT_CNS,
                                                prvTgfRestoreCfg.vlanEgressPortTagStateMode);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateModeSet);

    /* AUTODOC: set TAG0_CMD port tag state for egress port  */
    rc = prvTgfBrgVlanEgressPortTagStateSet(prvTgfDevNum,
                                            PRV_TGF_SECOND_PASS_EGR_EPORT_CNS,
                                            prvTgfRestoreCfg.vlanEgressPortTagState);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateSet);

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "tgfTrafficTableRxPcktTblClear");

    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "tgfTrafficTableRxStartCapture");
}

/**
* @internal tgfSrv6TwoThreeSrhContainersConfigRestore function
* @endinternal
*
* @brief   SRv6 Two/Three SRH Containers use case configurations restore.
* @note    1. Restore PHA Configuration
*          2. Restore Bridge Configuration
*          3. Restore Base Configuration
*
*/
static GT_VOID tgfSrv6TwoThreeSrhContainersConfigRestore
(
    GT_U32 secondPassThread
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  i, phaThreadId;
    CPSS_INTERFACE_INFO_STC                 portInterface;

    /* AUTODOC: Restore TTI/PCL Configuration */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_ETH_E, prvTgfRestoreCfg.origMacMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet");

    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_ETH_E, prvTgfRestoreCfg.lookupEnableGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: invalidate TTI rules  */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");

    /* AUTODOC: invalidate PCL rules  */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, IPCL_MATCH_INDEX_0_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d",
                                     CPSS_PCL_RULE_SIZE_STD_E,
                                     IPCL_MATCH_INDEX_0_CNS);

    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           PRV_TGF_SECOND_PASS_EGR_EPORT_CNS,
                                                           &prvTgfRestoreCfg.physicalInfo);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableSet);

    /* Loop Back config */
    rc = cpssDxChPortLoopbackProfileSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
            CPSS_DIRECTION_INGRESS_E,
            prvTgfRestoreCfg.srcLbProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackProfileSet");

    rc = cpssDxChPortLoopbackProfileSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
            CPSS_DIRECTION_EGRESS_E,
            prvTgfRestoreCfg.trgLbProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackProfileSet");

    /* AUTODOC: Restore cascade Type */
    rc = cpssDxChCscdPortTypeSet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_LOOPBACK_PORT_IDX_CNS],
                                 CPSS_PORT_DIRECTION_BOTH_E,
                                 prvTgfRestoreCfg.cscdPortType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCscdPortTypeSet");

    for (i = 0; i < CPSS_TC_RANGE_CNS; i++)
    {
        /* AUTODOC: enable FTL for loopback profiles {source #1, target #1} & for all traffic classes */
        rc = cpssDxChPortLoopbackEnableSet(prvTgfDevNum,
                                           PRV_TGF_LB_PROFILE_1_CNS,
                                           PRV_TGF_LB_PROFILE_1_CNS,
                                           i,
                                           prvTgfRestoreCfg.lbEnArr[i]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackEnableSet, i:%d", i);
    }

    /* AUTODOC: enable FTL for single-target FROM_CPU packet type */
    rc = cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableSet(prvTgfDevNum,
                            prvTgfRestoreCfg.savedFromCpuEn,
                            prvTgfRestoreCfg.savedFwdSingleEn,
                            prvTgfRestoreCfg.savedFwdMultiEn);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: Enable the internal Loopback state in the packet processor MAC port */
    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));
    portInterface.type             = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_LOOPBACK_PORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(&portInterface, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficGeneratorPortLoopbackModeEnableSet,"
                                            " portNum:%d", portInterface.devPort.portNum);

    /* Restore PHA Configuration */

    rc = cpssDxChPhaInit(prvTgfDevNum, GT_FALSE, prvTgfRestoreCfg.phaFwImageIdGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaInit FAILED for phaFwImageId = [%d], rc = [%d]", prvTgfRestoreCfg.phaFwImageIdGet, rc);

    phaThreadId = (secondPassThread == CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_2_CONTAINERS_E) ?
                                       PRV_TGF_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_2_CONTAINERS_E :
                                       PRV_TGF_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_3_CONTAINERS_E;

    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum, PRV_TGF_SECOND_PASS_EGR_EPORT_CNS,
                                        GT_FALSE, phaThreadId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaPortThreadIdSet FAILED for SRC_NODE_SECOND_PASS_2, rc = [%d]", rc);

    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                        GT_FALSE, PRV_TGF_PHA_THREAD_TYPE_SRV6_SRC_NODE_FIRST_PASS_2_3_CONTAINERS_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaPortThreadIdSet FAILED for SRC_NODE_FIRST_PASS_2_3_SEGMENT, rc = [%d]", rc);

    /* AUTODOC: Restore ePort attributes */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                        prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                        &prvTgfRestoreCfg.egressInfo);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                        PRV_TGF_SECOND_PASS_EGR_EPORT_CNS,
                                        &prvTgfRestoreCfg.egress1Info);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

    /* AUTODOC: set EPORT tag state mode for egress port */
    rc = prvTgfBrgVlanEgressPortTagStateModeSet(prvTgfDevNum,
                                                PRV_TGF_SECOND_PASS_EGR_EPORT_CNS,
                                                prvTgfRestoreCfg.vlanEgressPortTagStateMode);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateModeSet);

    /* AUTODOC: set TAG0_CMD port tag state for egress port  */
    rc = prvTgfBrgVlanEgressPortTagStateSet(prvTgfDevNum,
                                            PRV_TGF_SECOND_PASS_EGR_EPORT_CNS,
                                            prvTgfRestoreCfg.vlanEgressPortTagState);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateSet);

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "tgfTrafficTableRxPcktTblClear");

    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "tgfTrafficTableRxStartCapture");
}

/**
* @internal tgfSrv6TwoThreeSegmentTest function
* @endinternal
*
* @brief   SRv6 Two/Three segment use case test.
*/
GT_VOID tgfSrv6TwoThreeSegmentTest
(
    CPSS_DXCH_PHA_THREAD_TYPE_ENT           threadType
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIter;

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum,
                    prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "prvTgfResetCountersEth: %d, %d",
                    prvTgfDevNum,
                    prvTgfPortsArray[portIter]);
    }

    packetInfo.numOfParts =  sizeof(prvTgfIpv6PacketArray) / sizeof(prvTgfIpv6PacketArray[0]);
    packetInfo.partsArray = prvTgfIpv6PacketArray;
    packetInfo.totalLen   = TGF_PACKET_AUTO_CALC_LENGTH_CNS;

    /* setup receive portInterface for capturing */
    egrPortInterface.type               = CPSS_INTERFACE_PORT_E;
    egrPortInterface.devPort.hwDevNum   = prvTgfDevNum;
    egrPortInterface.devPort.portNum    = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    /* AUTODOC: GENERATE TRAFFIC & Verify at ePort#3 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
                        &egrPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgftrafficgeneratorporttxethcaptureset port - %d",
                                        egrPortInterface.devPort.portNum);

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, 1, 0, NULL);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfSetTxSetupEth");

    /* Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");

    /* Make the egress port tagged */
    rc = prvTgfBrgVlanEgressPortTagStateSet(prvTgfDevNum,
                                            PRV_TGF_SECOND_PASS_EGR_EPORT_CNS,
                                            pktType[1]);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateSet);

    /* Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");

    /* disable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &egrPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet port - %d",
            egrPortInterface.devPort.portNum);

    /* AUTODOC: Verification Started - 2 Segment Untagged */
    tgfSrv6TwoThreeSegmentVerification(threadType);
}

/**
* @internal tgfSrv6TwoThreeSrhContainersTest function
* @endinternal
*
* @brief   SRv6 Two/Three SRH Containers  use case test.
*/
GT_VOID tgfSrv6TwoThreeSrhContainersTest
(
    CPSS_DXCH_PHA_THREAD_TYPE_ENT           threadType
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIter;

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum,
                    prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "prvTgfResetCountersEth: %d, %d",
                    prvTgfDevNum,
                    prvTgfPortsArray[portIter]);
    }

    packetInfo.numOfParts =  sizeof(prvTgfIpv6PacketArray) / sizeof(prvTgfIpv6PacketArray[0]);
    packetInfo.partsArray = prvTgfIpv6PacketArray;
    packetInfo.totalLen   = TGF_PACKET_AUTO_CALC_LENGTH_CNS;

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* setup receive portInterface for capturing */
    egrPortInterface.type               = CPSS_INTERFACE_PORT_E;
    egrPortInterface.devPort.hwDevNum   = prvTgfDevNum;
    egrPortInterface.devPort.portNum    = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    /* AUTODOC: GENERATE TRAFFIC & Verify at ePort#3 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
                        &egrPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgftrafficgeneratorporttxethcaptureset port - %d",
                                        egrPortInterface.devPort.portNum);

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, 1, 0, NULL);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfSetTxSetupEth");

    /* Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");

    /* Make the egress port tagged */
    rc = prvTgfBrgVlanEgressPortTagStateSet(prvTgfDevNum,
                                            PRV_TGF_SECOND_PASS_EGR_EPORT_CNS,
                                            pktType[1]);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateSet);

    /* Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");

    /* disable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &egrPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet port - %d",
            egrPortInterface.devPort.portNum);

    /* AUTODOC: Verification Started - 2 Segment Untagged */
    tgfSrv6TwoThreeSrhContainersVerification(threadType);

}


/**
* internal tgfSrv6ThreeSegmentConfigSet function
* @endinternal
*
* @brief   SRV6 Three Segment use case configurations
*/
GT_VOID tgfSrv6ThreeSegmentConfigSet
(
    GT_VOID
)
{
    /* Bridging Engine Configuration
     * Route the packet from ingress to egress port */
    prvTgSrv6BridgingConfigSet(prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS], &prvTgfOriginalPacketL2Part.daMac);

    /* Loop Back configuration */
    prvTgfSrv6TwoThreeSegmentLoopBackConfigSet();

    /* Add TS IPv6 at egress ePort */
    prvTgfSrv6TwoThreeSegmentHaConfigSet();

    /* PHA - Attach PHA Thread "3 segment" at Egress port */
    prvTgfSrv6TwoThreeSegmentPhaConfigSet(CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_3_SEGMENTS_E);
}

/**
* @internal prvTgSrv6BestEffortTunnelTtiPclConfigSet function
* @endinternal
*
* @brief   TTI/IPCL configuration for SRv6 Best Effort Tunnel use case
*/
static GT_VOID prvTgSrv6BestEffortTunnelTtiPclConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc;
    PRV_TGF_TTI_ACTION_2_STC            ttiAction;
    PRV_TGF_TTI_RULE_UNT                ttiMask;
    PRV_TGF_TTI_RULE_UNT                ttiPattern;
    PRV_TGF_PCL_RULE_FORMAT_UNT         mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT         pattern;
    PRV_TGF_PCL_ACTION_STC              action;

    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* set the TTI lookup MAC mode for PRV_TGF_TTI_KEY_ETH_E */
    rc = prvTgfTtiMacModeGet(prvTgfDevNum,PRV_TGF_TTI_KEY_ETH_E,&prvTgfRestoreCfg.origMacMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeGet");

    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_ETH_E,PRV_TGF_TTI_MAC_MODE_SA_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet");

    /* enable the TTI lookup for PRV_TGF_TTI_KEY_ETH_E */
    rc = prvTgfTtiPortLookupEnableGet(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_ETH_E, &prvTgfRestoreCfg.lookupEnableGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableGet");

    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_ETH_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: TTI Mask for MAC SA */
    cpssOsMemSet(ttiMask.eth.common.mac.arEther, 0xFF, sizeof(GT_ETHERADDR));

    /* AUTODOC: TTI Pattern for MAC SA */
    cpssOsMemCpy(ttiPattern.eth.common.mac.arEther,
                 prvTgfOriginalPacketL2Part.saMac,
                 sizeof(GT_ETHERADDR));

    /* TTI actions - FORWARD packet */
    ttiAction.command                          = CPSS_PACKET_CMD_FORWARD_E;
    /* TTI actions - Copy Reserved Assignment */
    ttiAction.copyReservedAssignmentEnable     = GT_TRUE;
    ttiAction.copyReserved                     = PRV_TGF_APP_INDEX_CNS;
    /* TTI actions - Source ID Assignment */
    ttiAction.sourceIdSetEnable                = GT_TRUE;
    ttiAction.sourceId                         = PRV_TGF_SOURCE_INDEX_CNS;

    rc = prvTgfTtiRule2Set(PRV_TGF_TTI_INDEX_CNS, PRV_TGF_TTI_KEY_ETH_E,
                           &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet");

    /* AUTODOC: Init IPCL Engine for port 0 */
    rc = prvTgfPclDefPortInit(
            prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_NUMBER_0_E,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E     /*nonIpKey*/,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E  /*ipv6Key*/);
    UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfPclDefPortInit: %d, %d, %d",
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_INGRESS_E);

    /* IPCL actions - FORWARD packet */
    action.pktCmd                                            = CPSS_PACKET_CMD_FORWARD_E;
    action.bypassBridge                                      = GT_TRUE;
    action.redirect.redirectCmd                              = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    action.redirect.data.outIf.outInterface.type             = CPSS_INTERFACE_PORT_E;
    action.redirect.data.outIf.outInterface.devPort.hwDevNum = prvTgfDevNum;
    action.redirect.data.outIf.outInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    /* AUTODOC: IPCL Mask for MAC SA */
    cpssOsMemSet(mask.ruleStdNotIp.macSa.arEther, 0xFF, sizeof(GT_ETHERADDR));

    /* AUTODOC: IPCL Pattern for MAC SA */
    cpssOsMemCpy(pattern.ruleStdNotIp.macSa.arEther,
                prvTgfOriginalPacketL2Part.saMac,
                sizeof(GT_ETHERADDR));

    rc = prvTgfPclRuleSet(
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                IPCL_MATCH_INDEX_0_CNS, &mask, &pattern, &action);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                     PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                                     IPCL_MATCH_INDEX_0_CNS);
}

/**
* @internal prvTgfSrv6BestEffortTunnelEgressConfigSet function
* @endinternal
*
* @brief   Egress port TS IPv6 config for "SRv6 BE Tunnel use case".
*/
static GT_VOID prvTgfSrv6BestEffortTunnelEgressConfigSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC        egressInfo;
    PRV_TGF_TUNNEL_START_ENTRY_UNT          tunnelEntry, tunnelEntryGet;
    CPSS_TUNNEL_TYPE_ENT                    prvTgfTunnelType = CPSS_TUNNEL_GENERIC_IPV6_E;
    CPSS_TUNNEL_TYPE_ENT                    tunnelTypeGet;
    GT_U32                                  ii;
    PRV_TGF_TUNNEL_START_GEN_IP_PROFILE_STC profileData;

    /* AUTODOC: Egress tunnel SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Egress tunnel ipv6 Configuration =======\n");

    /* Save ePort attributes configuration */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                                                       prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                                       &(prvTgfRestoreCfg.egressInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoGet);

    /* Set ePort attributes configuration */
    cpssOsMemSet(&egressInfo,0,sizeof(egressInfo));
    egressInfo                        = prvTgfRestoreCfg.egressInfo;
    egressInfo.tunnelStart            = GT_TRUE;
    egressInfo.tunnelStartPtr         = PRV_TGF_TUNNEL_START_INDEX_CNS;
    egressInfo.tsPassengerPacketType  = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;

    /* AUTODOC: config ePort attributes: */
    /* AUTODOC:   Tunnel Start=ENABLE, tunnelStartPtr=8 */
    /* AUTODOC:   tsPassenger=ETHERNET */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                        prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                        &egressInfo);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

    /* AUTODOC: Set Generic IPv6 tunnel start entry */
    PRV_UTF_LOG0_MAC("======= Setting Tunnel Start Configuration =======\n");
    cpssOsMemSet(&tunnelEntry.ipv6Cfg,0,sizeof(tunnelEntry.ipv6Cfg));
    /*** Set a tunnel start entry ***/
    tunnelEntry.ipv6Cfg.tagEnable        = GT_TRUE;
    tunnelEntry.ipv6Cfg.vlanId           = PRV_TGF_DEFAULT_VLANID_CNS;
    tunnelEntry.ipv6Cfg.ttl              = 33;
    tunnelEntry.ipv6Cfg.retainCrc        = GT_FALSE;
    tunnelEntry.ipv6Cfg.ipHeaderProtocol = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_IP_E;
    tunnelEntry.ipv6Cfg.ipProtocol       = 43 /* Ignored */;
    tunnelEntry.ipv6Cfg.udpSrcPort       = udpSrcPort;
    tunnelEntry.ipv6Cfg.udpDstPort       = udpDstPort;
    tunnelEntry.ipv6Cfg.profileIndex     = PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS;

    /* tunnel next hop MAC DA, IP DA, IP SA */
    cpssOsMemCpy(tunnelEntry.ipv6Cfg.macDa.arEther, prvTgfOriginalPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    /* 80b Locator */
    for (ii = 0; ii < 5; ii++)
    {
        tunnelEntry.ipv6Cfg.destIp.arIP[ii * 2]     = (GT_U8)(prvTgf1stTunnelStartDstIPv6[ii] >> 8 & 0xFF);
        tunnelEntry.ipv6Cfg.destIp.arIP[ii * 2 + 1] = (GT_U8) prvTgf1stTunnelStartDstIPv6[ii] & 0xFF;
    }
    for (ii = 0; ii < 8; ii++)
    {
        tunnelEntry.ipv6Cfg.srcIp.arIP[ii * 2]     = (GT_U8)(prvTgf1stTunnelStartSrcIPv6[ii] >> 8 & 0xFF);
        tunnelEntry.ipv6Cfg.srcIp.arIP[ii * 2 + 1] = (GT_U8) prvTgf1stTunnelStartSrcIPv6[ii] & 0xFF;
    }

    /* FirstBit and LastBit of Source-ID to represent SGT-ID */
    tunnelEntry.ipv6Cfg.destIp.arIP[12] = PRV_TGF_SOURCE_ID_LAST_BIT;
    tunnelEntry.ipv6Cfg.destIp.arIP[13] = PRV_TGF_SOURCE_ID_FIRST_BIT;

    tunnelEntry.ipv6Cfg.destIp.arIP[14] = (GT_U8)(prvTgf1stTunnelStartDstIPv6[ii] >> 8 & 0xFF);
    tunnelEntry.ipv6Cfg.destIp.arIP[15] = (GT_U8)(prvTgf1stTunnelStartDstIPv6[ii] & 0xFF);

    /* AUTODOC: add TS entry 8 with: */
    /* AUTODOC:   tunnelType = GENERIC_IPV6 */
    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_INDEX_CNS, prvTgfTunnelType, &tunnelEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntrySet: %d", prvTgfDevNum);

    /* AUTODOC: get and check added TS Entry */
    rc = prvTgfTunnelStartEntryGet(prvTgfDevNum, PRV_TGF_TUNNEL_START_INDEX_CNS, &tunnelTypeGet, &tunnelEntryGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntryGet: %d", prvTgfDevNum);

    /* Check vlanId field */
    UTF_VERIFY_EQUAL1_STRING_MAC(tunnelEntryGet.ipv6Cfg.vlanId, tunnelEntry.ipv6Cfg.vlanId,
            "Vlan Id mismatch with configured value for device %d", prvTgfDevNum);

    /* Check srcIp field */
    rc = cpssOsMemCmp(&tunnelEntryGet.ipv6Cfg.srcIp, &tunnelEntry.ipv6Cfg.srcIp, sizeof(GT_IPV6ADDR)) == 0 ? GT_OK : GT_FALSE;
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tunnelEntry.ipv6Cfg.srcIp did not match");

    /* AUTODOC: IP Profile Table Entry <DIP Mode> */
    cpssOsMemSet(&profileData, 0, sizeof(profileData));
    profileData.dipMode = 16;
    profileData.serviceIdCircularShiftSize = 0;
    rc = prvTgfTunnelStartGenProfileTableEntrySet(prvTgfDevNum,
                                                  PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS,
                                                  &profileData);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartGenProfileTableEntrySet");

    /*  AUTODOC: set egress vlan table service Id */
    rc = prvTgfTunnelStartEgessVlanTableServiceIdGet(prvTgfDevNum, PRV_TGF_DEFAULT_VLANID_CNS, &(prvTgfRestoreCfg.vlanServiceIdGet));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEgessVlanTableServiceIdGet");

    rc = prvTgfTunnelStartEgessVlanTableServiceIdSet(prvTgfDevNum, PRV_TGF_DEFAULT_VLANID_CNS, PRV_TGF_EVLAN_SERVISE_ID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEgessVlanTableServiceIdSet");

}

/**
* internal prvTgfSrv6BestEffortTunnelPhaConfigSet function
* @endinternal
*
* @brief   PHA config for SRv6 BE Tunnel use case.
*/
static GT_VOID prvTgfSrv6BestEffortTunnelPhaConfigSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    CPSS_DXCH_PHA_THREAD_INFO_UNT           extInfo;

    /* GM does not support PHA */
    if(GT_TRUE == prvUtfIsGmCompilation() || (prvTgfSrv6OneSegmentSkipPha == GT_TRUE))
        return;

    /* AUTODOC: Assign PHA firmware image ID 01 */
    rc = cpssDxChPhaFwImageIdGet(prvTgfDevNum, &(prvTgfRestoreCfg.phaFwImageIdGet));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaFwImageIdGet FAILED with rc = [%d]", rc);

    rc = cpssDxChPhaInit(prvTgfDevNum, GT_FALSE, CPSS_DXCH_PHA_FW_IMAGE_ID_01_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaInit FAILED for phaFwImageId = [%d], rc = [%d]", CPSS_DXCH_PHA_FW_IMAGE_ID_01_E, rc);

    cpssOsMemSet(&extInfo,   0, sizeof(extInfo));     /* No template */
    /* AUTODOC: Assign PHA thread "SRv6 BE Tunnel" to egress port */
    prvTgSrv6PhaThreadConfigSet(prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
            PRV_TGF_PHA_THREAD_TYPE_SRV6_BEST_EFFORT_E,
            CPSS_DXCH_PHA_THREAD_TYPE_SRV6_BEST_EFFORT_E,
            &extInfo);
}

/**
* internal tgfSrv6BestEffortTunnelConfigSet function
* @endinternal
*
* @brief   SRV6 Best Effort Tunnel use case configurations
*/
GT_VOID tgfSrv6BestEffortTunnelConfigSet
(
    GT_VOID
)
{

    /* TTI/PCL Engine Configurations */
    prvTgSrv6BestEffortTunnelTtiPclConfigSet();

    /* Add TS IPv6 at egress Port */
    prvTgfSrv6BestEffortTunnelEgressConfigSet();

    /* PHA - Attach PHA Thread "SRv6 BE Tunnel" at Egress port */
    prvTgfSrv6BestEffortTunnelPhaConfigSet();
}

/**
* internal tgfSrv6BestEffortTunnelVerification function
* @endinternal
*
* @brief   SRv6 Best Effort Tunnel use case verification
*/
GT_VOID tgfSrv6BestEffortTunnelVerification
(
    GT_VOID
)
{
    GT_STATUS                           rc = GT_OK;
    GT_U8                               packetBuf[PRV_TGF_PACKET_SIZE_CNS] = {0};
    GT_U32                              packetLen = PRV_TGF_PACKET_SIZE_CNS;
    GT_U32                              packetActualLength = 0;
    GT_U8                               queue = 0;
    GT_U8                               dev = 0, ii;
    TGF_NET_DSA_STC                     rxParam;
    GT_BOOL                             getFirst = GT_TRUE;
    GT_U32                              pktOffset1, pktOffset = TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS;
    GT_U32                              appId;

    rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&egrPortInterface,
            TGF_PACKET_TYPE_CAPTURE_E,
            getFirst, GT_TRUE, packetBuf,
            &packetLen, &packetActualLength,
            &dev, &queue, &rxParam);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet: egrPortInterface %d", prvTgfDevNum);

    /* Add Tag length in case of tagged packet */
    pktOffset = TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS;

    /* IPv6 Next Header Verification - No SRH extension so 43 not valid */
    pktOffset += 6;             /* Point to Next Header index */
    UTF_VERIFY_EQUAL0_STRING_MAC(41, packetBuf[pktOffset],
            "IPv6 Next Header is not matching ");

    /* AUTODOC: Tunnel SIP and DIP verification
     * DIP - Configured DIP via prvTgf1stTunnelStartDstIPv6 changes as per PHA
     * SIP - Must be default value
     */
    pktOffset += 2;             /* Point to SIP index */
    pktOffset1 = pktOffset+16;  /* Point to DIP index */
    for (ii = 0; ii < 8; ii++)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(prvTgf1stTunnelStartSrcIPv6[ii],
                (GT_U16)(packetBuf[pktOffset + 1] | (packetBuf[pktOffset] << 8)),
                "SIP is not matching at index[%d]", ii);
        if(ii <= 4)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgf1stTunnelStartDstIPv6[ii],
                    (GT_U16)(packetBuf[pktOffset1+1] | (packetBuf[pktOffset1] << 8)),
                    "DIP Locator is not matching at index[%d]", ii);
            pktOffset1+=2;
        }
        pktOffset+=2;
    }

    ii = 5;
    UTF_VERIFY_EQUAL1_STRING_MAC(PRV_TGF_EVLAN_SERVISE_ID_CNS,
            (GT_U16)(packetBuf[pktOffset1+1] | (packetBuf[pktOffset1] << 8)),
            "DIP Function ID is not matching at index[%d]", ii);

    ii++;
    pktOffset1+=2;
    UTF_VERIFY_EQUAL1_STRING_MAC(PRV_TGF_SOURCE_INDEX_CNS,
            (GT_U16)(packetBuf[pktOffset1+1] | (packetBuf[pktOffset1] << 8)),
            "DIP SourceId is not matching at index[%d]", ii);

    ii++;
    pktOffset1+=2;
    appId = (PRV_TGF_APP_INDEX_CNS & PRV_TGF_APP_ID_MASK) << PRV_TGF_RESERVE_ID_BIT_CNS;
    UTF_VERIFY_EQUAL1_STRING_MAC(appId,
            (GT_U16)(packetBuf[pktOffset1+1] | (packetBuf[pktOffset1] << 8)),
            "DIP AppId is not matching at index[%d]", ii);

    UTF_VERIFY_EQUAL1_STRING_MAC(0x0,
            packetBuf[pktOffset1 + 1] & 0xF,
            "DIP Reserved bit is not matching at index[%d]", ii);
}

/**
* @internal tgfSrv6BestEffortTunnelTest function
* @endinternal
*
* @brief   SRv6 Best Effort Tunnel use case test.
*/
GT_VOID tgfSrv6BestEffortTunnelTest
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIter;

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum,
                    prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "prvTgfResetCountersEth: %d, %d",
                    prvTgfDevNum,
                    prvTgfPortsArray[portIter]);
    }

    packetInfo.numOfParts =  sizeof(prvTgfIpv6PacketArray) / sizeof(prvTgfIpv6PacketArray[0]);
    packetInfo.partsArray = prvTgfIpv6PacketArray;
    packetInfo.totalLen   = TGF_PACKET_AUTO_CALC_LENGTH_CNS;

    /* setup receive portInterface for capturing */
    egrPortInterface.type               = CPSS_INTERFACE_PORT_E;
    egrPortInterface.devPort.hwDevNum   = prvTgfDevNum;
    egrPortInterface.devPort.portNum    = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    /* AUTODOC: GENERATE TRAFFIC & Verify at ePort#3 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
                        &egrPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgftrafficgeneratorporttxethcaptureset port - %d",
                                        egrPortInterface.devPort.portNum);

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, 1, 0, NULL);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfSetTxSetupEth");

    /* Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");

    /* disable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &egrPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet port - %d",
            egrPortInterface.devPort.portNum);

    /* AUTODOC: Verification Started */
    tgfSrv6BestEffortTunnelVerification();
}

/**
* @internal tgfSrv6BestEffortTunnelConfigRestore function
* @endinternal
*
* @brief   SRv6 Best Effort Tunnel use case configurations restore.
* @note    1. Restore PHA Configuration
*          2. Restore Bridge Configuration
*          3. Restore Base Configuration
*
*/
GT_VOID tgfSrv6BestEffortTunnelConfigRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: Restore TTI/PCL Configuration */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_ETH_E, prvTgfRestoreCfg.origMacMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet");

    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_ETH_E, prvTgfRestoreCfg.lookupEnableGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: invalidate TTI rules  */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");

    /* AUTODOC: invalidate PCL rules  */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, IPCL_MATCH_INDEX_0_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d",
                                     CPSS_PCL_RULE_SIZE_STD_E,
                                     IPCL_MATCH_INDEX_0_CNS);

    rc = prvTgfTunnelStartEgessVlanTableServiceIdSet(prvTgfDevNum, PRV_TGF_DEFAULT_VLANID_CNS, prvTgfRestoreCfg.vlanServiceIdGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEgessVlanTableServiceIdSet");

    /* AUTODOC: Restore PHA Configuration */
    rc = cpssDxChPhaInit(prvTgfDevNum, GT_FALSE, prvTgfRestoreCfg.phaFwImageIdGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaInit FAILED for phaFwImageId = [%d], rc = [%d]", prvTgfRestoreCfg.phaFwImageIdGet, rc);

    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                    GT_FALSE, PRV_TGF_PHA_THREAD_TYPE_SRV6_BEST_EFFORT_E);
   UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaPortThreadIdSet FAILED for SRV6_BEST_EFFORT, rc = [%d]", rc);

    /* AUTODOC: Restore ePort attributes */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                        prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                        &(prvTgfRestoreCfg.egressInfo));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortEgressPortInfoSet");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");
}

/**
* @internal prvTgSrv6BeTunnelTtiPclParseOnBorderNodeConfigSet function
* @endinternal
*
* @brief   SRv6 Best Effort Tunnel Parse on Border Node use case test.
*/
GT_VOID prvTgSrv6BeTunnelTtiPclParseOnBorderNodeConfigSet
(
    GT_VOID
)
{
    GT_STATUS                             rc = GT_OK;
    PRV_TGF_TTI_ACTION_2_STC              ttiAction ;
    PRV_TGF_TTI_RULE_UNT                  ttiPattern;
    PRV_TGF_TTI_RULE_UNT                  ttiMask;
    PRV_TGF_PCL_RULE_FORMAT_UNT           mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT           pattern;
    PRV_TGF_PCL_ACTION_STC                action;
    GT_U32                                ii, tcQueue = 2;
    CPSS_INTERFACE_INFO_STC               phyPortInfo;
    GT_U32                                udbIndexArr[3] = {40,41,42};
    PRV_TGF_PCL_UDB_SELECT_STC            udbSelect;
    PRV_TGF_NET_CPU_CODE_TABLE_ENTRY_STC  cpuCodeEntry;
    CPSS_DXCH_NET_SDMA_RX_COUNTERS_STC    rxCounters;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* AUTODOC: enable TTI lookup for port PRV_TGF_INGRESS_PORT_IDX_CNS, key PRV_TGF_TTI_KEY_UDB_IPV6_E */
    rc = prvTgfTtiPortLookupEnableGet(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_UDB_IPV6_E, &prvTgfRestoreCfg.lookupEnableGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableGet");

    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                PRV_TGF_TTI_KEY_UDB_IPV6_E,
                                GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* set the TTI Rule Pattern, Mask and Action for Ether Type Key */

    /* clear entry */
    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));
    cpssOsMemSet(&ttiPattern,0, sizeof(ttiPattern));
    cpssOsMemSet(&ttiMask,   0, sizeof(ttiMask));
    cpssOsMemSet(&action,    0, sizeof(action));
    cpssOsMemSet(&mask,      0, sizeof(mask));
    cpssOsMemSet(&pattern,   0, sizeof(pattern));
    cpssOsMemSet(&udbSelect, 0, sizeof(udbSelect));

    for(ii = 0; ii < 5 ; ii++)
    {
        ttiPattern.udbArray.udb[ii*2]   = (prvTgf1stTunnelStartDstIPv6[ii] >> 8) & 0xFF;
        ttiPattern.udbArray.udb[ii*2+1] = (prvTgf1stTunnelStartDstIPv6[ii] & 0xFF);
        ttiMask.udbArray.udb[ii*2]      = 0xff;
        ttiMask.udbArray.udb[ii*2+1]    = 0xff;
    }

    PRV_UTF_LOG0_MAC("======= Setting UDB Configuration =======\n");

    rc = prvTgfTtiPacketTypeKeySizeGet(prvTgfDevNum,
                            PRV_TGF_TTI_KEY_UDB_IPV6_E,
                            &prvTgfRestoreCfg.keySizeGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);

    rc = prvTgfTtiPacketTypeKeySizeSet(prvTgfDevNum,
                            PRV_TGF_TTI_KEY_UDB_IPV6_E,
                            PRV_TGF_TTI_KEY_SIZE_10_B_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);

    /* TTI actions - FORWARD packet */
    ttiAction.command                          = CPSS_PACKET_CMD_FORWARD_E;
    /* TTI actions - Copy Reserved Assignment [11:2] */
    ttiAction.copyReservedAssignmentEnable     = GT_TRUE;
    /* assign copyReserved bits 11:2 with Tunnel DIP-APP ID bits 15:6 */
    ttiAction.copyReserved                     = PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum) ? (PRV_TGF_APP_INDEX_CNS & 0x3FF) << 1 : PRV_TGF_APP_INDEX_CNS & 0x3FF;
    /* TTI actions - Source ID Assignment */
    ttiAction.sourceIdSetEnable                = GT_TRUE;
    ttiAction.sourceId                         = PRV_TGF_SOURCE_INDEX_CNS;

    /* AUTODOC: set TTI rule */
    rc = prvTgfTtiRuleUdbSet(PRV_TGF_TTI_INDEX_CNS,PRV_TGF_TTI_RULE_UDB_10_E,
                             &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleUdbSet");

    for(ii = 0; ii < 10 ; ii++)
    {
        rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,
                        PRV_TGF_TTI_KEY_UDB_IPV6_E, ii,
                        PRV_TGF_TTI_OFFSET_L3_MINUS_2_E,(GT_U8) ii + 26 /* start of L3 DIPv6 */);
    }

    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_INDEX_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d, %d", prvTgfDevNum, GT_FALSE);

    rc = prvTgfTtiPclIdSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV6_E, PRV_TGF_PCL_INDEX_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdSet: %d", prvTgfDevNum);

    /* AUTODOC: Init IPCL Engine for port 3 */
    rc = prvTgfPclDefPortInit(
            prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_NUMBER_0_E,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E     /*nonIpKey*/,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E  /*ipv6Key*/);
    UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfPclDefPortInit: %d, %d, %d",
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_INGRESS_E);

    /* IPCL actions - FORWARD packet */
    action.pktCmd                                            = CPSS_PACKET_CMD_FORWARD_E;
    action.bypassBridge                                      = GT_TRUE;
    action.redirect.redirectCmd                              = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    action.redirect.data.outIf.outInterface.type             = CPSS_INTERFACE_PORT_E;
    action.redirect.data.outIf.outInterface.devPort.hwDevNum = prvTgfDevNum;
    action.redirect.data.outIf.outInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    /* AUTODOC: IPCL Mask for MAC SA */
    cpssOsMemSet(mask.ruleStdNotIp.macDa.arEther, 0xFF, sizeof(GT_ETHERADDR));

    /* AUTODOC: IPCL Pattern for MAC SA */
    cpssOsMemCpy(pattern.ruleStdNotIp.macDa.arEther,
                prvTgfOriginalPacketL2Part.daMac,
                sizeof(GT_ETHERADDR));

    rc = prvTgfPclRuleSet(
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                IPCL_MATCH_INDEX_0_CNS, &mask, &pattern, &action);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                     PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                                     IPCL_MATCH_INDEX_0_CNS);

    /* AUTODOC: Init EPCL Engine for port 1 */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_NUMBER_0_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E  /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfPclDefPortInit: %d, %d, %d",
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_EGRESS_E);

    /* Mapping UDB_0/1/2 to UDB_40/41/42 */
    udbSelect.udbSelectArr[0] = udbIndexArr[0];
    udbSelect.udbSelectArr[1] = udbIndexArr[1];
    udbSelect.udbSelectArr[2] = udbIndexArr[2];

    if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
    {
        GT_U32  ii;
        for(ii = 0 ; ii < 10; ii++)
        {
            udbSelect.udbSelectArr[ii] = ii;
        }
        udbIndexArr[0] = udbSelect.udbSelectArr[0];
        udbIndexArr[1] = udbSelect.udbSelectArr[1];
        udbIndexArr[2] = udbSelect.udbSelectArr[2];
    }

    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    /* AUTODOC: Metadata UDBs configuration for copyReserved */
    /* EPCL UDBs Metadata:(724:705) -  copyReserved(20 bits) */
    rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                 PRV_TGF_PCL_PACKET_TYPE_IPV6_OTHER_E,
                                 CPSS_PCL_DIRECTION_EGRESS_E,
                                 udbIndexArr[0],
                                 PRV_TGF_PCL_OFFSET_METADATA_E,
                                 88);   /* Anchor offset: 88 Bit offset: 7:1 */
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                                "prvTgfPclUserDefinedByteSet: packetType %d, udbIndex %d,"
                                "OffsetType %d, Offset %d\n",
                                 PRV_TGF_PCL_PACKET_TYPE_IPV6_OTHER_E,
                                 udbIndexArr[0], PRV_TGF_PCL_OFFSET_METADATA_E, 88);

    rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                 PRV_TGF_PCL_PACKET_TYPE_IPV6_OTHER_E,
                                 CPSS_PCL_DIRECTION_EGRESS_E,
                                 udbIndexArr[1],
                                 PRV_TGF_PCL_OFFSET_METADATA_E,
                                 89);  /* Anchor offset: 89 Bit offset: 7:0 */
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                                "prvTgfPclUserDefinedByteSet: packetType %d, udbIndex %d,"
                                "OffsetType %d, Offset %d\n",
                                 PRV_TGF_PCL_PACKET_TYPE_IPV6_OTHER_E,
                                 udbIndexArr[1], PRV_TGF_PCL_OFFSET_METADATA_E, 89);

    rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                 PRV_TGF_PCL_PACKET_TYPE_IPV6_OTHER_E,
                                 CPSS_PCL_DIRECTION_EGRESS_E,
                                 udbIndexArr[2],
                                 PRV_TGF_PCL_OFFSET_METADATA_E,
                                 90); /* Anchor offset: 90 Bit offset: 4:0 */
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclUserDefinedByteSet: packetType %d, udbIndex %d,"
                                 "OffsetType %d, Offset %d\n",
                                 PRV_TGF_PCL_PACKET_TYPE_IPV6_OTHER_E,
                                 udbIndexArr[2], PRV_TGF_PCL_OFFSET_METADATA_E, 90);

    rc = prvTgfPclUserDefinedBytesSelectSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                            PRV_TGF_PCL_PACKET_TYPE_IPV6_OTHER_E, CPSS_PCL_LOOKUP_0_E,
                                            &udbSelect);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedBytesSelectSet: %d, %d",
                                 PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E, CPSS_PCL_LOOKUP_0_E);

    /* AUTODOC: mask and pattern for copyReserved */
    mask.ruleEgrUdbOnly.udb[0] = 0xFE;
    mask.ruleEgrUdbOnly.udb[1] = 0xFF;
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        mask.ruleEgrUdbOnly.udb[2] = 0x1F;
    }
    else if (PRV_CPSS_SIP_5_20_CHECK_MAC(prvTgfDevNum))
    {
        mask.ruleEgrUdbOnly.udb[2] = 0xF;
    }
    else
    {
        mask.ruleEgrUdbOnly.udb[2] = 0x3;
    }

    pattern.ruleEgrUdbOnly.udb[0] =  ((PRV_TGF_APP_ID_MASK) << 3)  & 0xFE;
    pattern.ruleEgrUdbOnly.udb[1] =  ((PRV_TGF_APP_ID_MASK) >> 5)  & 0xFF;
    pattern.ruleEgrUdbOnly.udb[2] =  ((PRV_TGF_APP_ID_MASK) >> 13) & 0x1F;

    /* AUTODOC: EPCL action - drop the packet */
    action.egressPolicy                          = GT_TRUE;
    action.pktCmd                                = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    action.mirror.cpuCode                        = CPSS_NET_USER_DEFINED_0_E;
    action.redirect.redirectCmd                  = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    action.redirect.data.outIf.outInterface.type = CPSS_INTERFACE_VID_E;

    rc = prvTgfPclRuleSet(
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            EPCL_MATCH_INDEX_CNS, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                 PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                 EPCL_MATCH_INDEX_CNS);

    /* set tcQueue for cpuCode CPSS_NET_INTERVENTION_PORT_LOCK_E to validate packet exception based on new MAC SA */
    cpssOsMemSet(&cpuCodeEntry, 0, sizeof(cpuCodeEntry));
    cpuCodeEntry.tc = tcQueue;
    prvTgfRestoreCfg.tcQueueGet = tcQueue;
    rc = prvTgfNetIfCpuCodeTableGet(prvTgfDevNum, CPSS_NET_USER_DEFINED_0_E, &prvTgfRestoreCfg.cpuCodeEntryGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfNetIfCpuCodeTableSet failed, device %d cpuCode %d\n", prvTgfDevNum, CPSS_NET_FIRST_USER_DEFINED_E);
    rc = prvTgfNetIfCpuCodeTableSet(prvTgfDevNum, CPSS_NET_USER_DEFINED_0_E, &cpuCodeEntry);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfNetIfCpuCodeTableSet failed, device %d cpuCode %d\n", prvTgfDevNum, CPSS_NET_FIRST_USER_DEFINED_E);

    /* clear the queue counters */
    rc = cpssDxChNetIfSdmaRxCountersGet(prvTgfDevNum, tcQueue, &rxCounters);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChNetIfSdmaRxCountersGet failed, device %d tcQueue %d\n", prvTgfDevNum, tcQueue);

    /* AUTODOC: Set physical info for the given ePort in the E2PHY mapping table */
    cpssOsMemSet(&phyPortInfo,0,sizeof(phyPortInfo));
    phyPortInfo.type              = CPSS_INTERFACE_PORT_E;
    phyPortInfo.devPort.hwDevNum  = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);
    phyPortInfo.devPort.portNum   = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                                                       prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                                       &(prvTgfRestoreCfg.egressInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoGet);

    rc = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS], &phyPortInfo);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgEportToPhysicalPortTargetMappingTableSet: %d %d",prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS]);
}

/**
* internal tgfSrv6BestEffortTunnelParseOnBorderNodeConfigSet function
* @endinternal
*
* @brief   SRV6 Best Effort Tunnel Parse on Border Node use case configurations
*/
GT_VOID tgfSrv6BestEffortTunnelParseOnBorderNodeConfigSet
(
    GT_VOID
)
{
    /* TTI/PCL Engine Configurations */
    prvTgSrv6BeTunnelTtiPclParseOnBorderNodeConfigSet();

}

/**
* @internal tgfSrv6BestEffortTunnelParseOnBorderNodeTest function
* @endinternal
*
* @brief   SRv6 Best Effort Tunnel Parse on Border Node use case test.
*/
GT_VOID tgfSrv6BestEffortTunnelParseOnBorderNodeTest
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  portIter, ii;
    CPSS_DXCH_NET_SDMA_RX_COUNTERS_STC      rxCounters;
    GT_U32                                  exceptionPktCount = 0;

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum,
                    prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "prvTgfResetCountersEth: %d, %d",
                    prvTgfDevNum,
                    prvTgfPortsArray[portIter]);
    }

    for (ii = 0; ii < 8; ii++)
    {
        prvTgfTunnelPacketIpv6Part.dstAddr[ii] = prvTgf1stTunnelStartDstIPv6[ii];
        prvTgfTunnelPacketIpv6Part.srcAddr[ii] = prvTgf1stTunnelStartSrcIPv6[ii];
    }
    prvTgfTunnelPacketIpv6Part.dstAddr[5] = PRV_TGF_EVLAN_SERVISE_ID_CNS;
    prvTgfTunnelPacketIpv6Part.dstAddr[6] = PRV_TGF_SOURCE_INDEX_CNS;
    prvTgfTunnelPacketIpv6Part.dstAddr[7] = (PRV_TGF_APP_INDEX_CNS & PRV_TGF_APP_ID_MASK) << PRV_TGF_RESERVE_ID_BIT_CNS;

    packetInfo.numOfParts =  sizeof(prvTgfSRv6PacketArray) / sizeof(prvTgfSRv6PacketArray[0]);
    packetInfo.partsArray = prvTgfSRv6PacketArray;
    packetInfo.totalLen   = TGF_PACKET_AUTO_CALC_LENGTH_CNS;

    /* setup receive portInterface for capturing */
    egrPortInterface.type               = CPSS_INTERFACE_PORT_E;
    egrPortInterface.devPort.hwDevNum   = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);
    egrPortInterface.devPort.portNum    = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    /* AUTODOC: GENERATE TRAFFIC & Verify at ePort#3 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
                        &egrPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgftrafficgeneratorporttxethcaptureset port - %d",
                                        egrPortInterface.devPort.portNum);

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, 1, 0, NULL);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfSetTxSetupEth");

    /* Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");

    /* Verify packet should reach in case of enabled queue*/
    rc = cpssDxChNetIfSdmaRxCountersGet(prvTgfDevNum, prvTgfRestoreCfg.tcQueueGet, &rxCounters);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChNetIfSdmaRxCountersGet failed, device %d tcQueue %d\n", prvTgfDevNum, prvTgfRestoreCfg.tcQueueGet);

    exceptionPktCount = rxCounters.rxInPkts;
    rc = (exceptionPktCount == 1) ? GT_OK : GT_FAIL;
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "SDMA Rx count expected %d received %d tcQueue %d\n", 1, exceptionPktCount, prvTgfRestoreCfg.tcQueueGet);

    /* disable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &egrPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet port - %d",
            egrPortInterface.devPort.portNum);

}

/**
* @internal tgfSrv6BestEffortTunnelParseOnBorderNodeConfigRestore function
* @endinternal
*
* @brief   SRv6 Best Effort Tunnel Parse on Border Node use case configurations restore.
*          1. Restore Bridge Configuration
*          2. Restore Base Configuration
*
*/
GT_VOID tgfSrv6BestEffortTunnelParseOnBorderNodeConfigRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_UDB_IPV6_E, prvTgfRestoreCfg.lookupEnableGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: invalidate TTI rules  */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");

    /* AUTODOC: invalidate PCL rules  */
    rc = prvTgfPclRuleValidStatusSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E, IPCL_MATCH_INDEX_0_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d",
                                     PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                                     IPCL_MATCH_INDEX_0_CNS);
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, EPCL_MATCH_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d",
                                     PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                     EPCL_MATCH_INDEX_CNS);

    /* AUTODOC: Disables I/EPCL Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);
    rc = prvTgfPclEgressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclEgressPolicyEnable: %d", GT_FALSE);

    /* AUTODOC: Restore ePort attributes */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                        prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                        &(prvTgfRestoreCfg.egressInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

    /* AUTODOC: Restore cpu code table */
    rc = prvTgfNetIfCpuCodeTableSet(prvTgfDevNum, CPSS_NET_USER_DEFINED_0_E, &prvTgfRestoreCfg.cpuCodeEntryGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfNetIfCpuCodeTableSet failed, device %d cpuCode %d\n", prvTgfDevNum, CPSS_NET_FIRST_USER_DEFINED_E);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "tgfTrafficTableRxPcktTblClear");

    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "tgfTrafficTableRxStartCapture");

}

/**
* @internal prvTgSrv6BeTunnelTtiParseOnLeafNodeConfigSet function
* @endinternal
*
* @brief   SRv6 Best Effort Tunnel Parse on Leaf Node use case test.
*/
GT_VOID prvTgSrv6BeTunnelTtiParseOnLeafNodeConfigSet
(
    GT_VOID
)
{
    GT_STATUS                             rc = GT_OK;
    PRV_TGF_TTI_ACTION_2_STC              ttiAction ;
    PRV_TGF_TTI_RULE_UNT                  ttiPattern;
    PRV_TGF_TTI_RULE_UNT                  ttiMask;
    GT_U32                                ii;
    PRV_TGF_PCL_UDB_SELECT_STC            udbSelect;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* AUTODOC: enable TTI lookup for port PRV_TGF_INGRESS_PORT_IDX_CNS, key PRV_TGF_TTI_KEY_UDB_IPV6_E */
    rc = prvTgfTtiPortLookupEnableGet(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_UDB_IPV6_E, &prvTgfRestoreCfg.lookupEnableGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableGet");

    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                PRV_TGF_TTI_KEY_UDB_IPV6_E,
                                GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* set the TTI Rule Pattern, Mask and Action for Ether Type Key */

    /* clear entry */
    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));
    cpssOsMemSet(&ttiPattern,0, sizeof(ttiPattern));
    cpssOsMemSet(&ttiMask,   0, sizeof(ttiMask));
    cpssOsMemSet(&udbSelect, 0, sizeof(udbSelect));

    for(ii = 0; ii < 5 ; ii++)
    {
        ttiPattern.udbArray.udb[ii*2]   = (prvTgf1stTunnelStartDstIPv6[ii] >> 8) & 0xFF;
        ttiPattern.udbArray.udb[ii*2+1] = (prvTgf1stTunnelStartDstIPv6[ii] & 0xFF);
        ttiMask.udbArray.udb[ii*2]      = 0xff;
        ttiMask.udbArray.udb[ii*2+1]    = 0xff;
    }

    PRV_UTF_LOG0_MAC("======= Setting UDB Configuration =======\n");

    rc = prvTgfTtiPacketTypeKeySizeGet(prvTgfDevNum,
                            PRV_TGF_TTI_KEY_UDB_IPV6_E,
                            &prvTgfRestoreCfg.keySizeGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);

    rc = prvTgfTtiPacketTypeKeySizeSet(prvTgfDevNum,
                            PRV_TGF_TTI_KEY_UDB_IPV6_E,
                            PRV_TGF_TTI_KEY_SIZE_10_B_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);

    /* TTI actions - FORWARD packet */
    ttiAction.command                                        = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction.bridgeBypass                                   = GT_TRUE;
    ttiAction.redirectCommand                                = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    ttiAction.egressInterface.type                           = CPSS_INTERFACE_PORT_E;
    ttiAction.egressInterface.devPort.hwDevNum               = prvTgfDevNum;
    ttiAction.egressInterface.devPort.portNum                = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    /* AUTODOC: set TTI rule */
    rc = prvTgfTtiRuleUdbSet(PRV_TGF_TTI_INDEX_CNS,PRV_TGF_TTI_RULE_UDB_10_E,
                             &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleUdbSet");

    for(ii = 0; ii < 10 ; ii++)
    {
        rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,
                        PRV_TGF_TTI_KEY_UDB_IPV6_E, ii,
                        PRV_TGF_TTI_OFFSET_L3_MINUS_2_E,(GT_U8) ii + 26 /* start of L3 DIPv6 */);
    }

    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_INDEX_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d, %d", prvTgfDevNum, GT_FALSE);

    rc = prvTgfTtiPclIdSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV6_E, PRV_TGF_PCL_INDEX_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdSet: %d", prvTgfDevNum);
}

/**
 * @internal prvTgfPpuInitConfigSet function
 * @endinternal
 *
 * @brief    Set PPU init configuration
 *           - Enable PPU in TTI global configuration ext2 and PPU unit.
 *           - Set default PPU profile index to ingress port
 *           - Configure PPU profile at default PPU profile index
 *
 */
static GT_VOID prvTgfPpuInitConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc;
    CPSS_DXCH_PPU_PROFILE_ENTRY_STC profileEntry;
    CPSS_DXCH_PPU_SP_BUS_STC        spBusProfile;

    /* ------------------------------------------
     * 1. Enable PPU in TTI global configuration ext2 and PPU unit.
     */

    /* store default ppu configuration */
    rc = cpssDxChPpuEnableGet(prvTgfDevNum, &prvTgfRestoreCfg.ppuEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuEnableGet failed");

    /* enable ppu unit */
    rc = cpssDxChPpuEnableSet(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuEnableSet failed");

    /* ------------------------------------------
     * 2. Set default PPU profile index to ingress port.
     */

    /* store default PPU Profile Idx of source port */
    rc = cpssDxChPpuSrcPortProfileIndexGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                         &prvTgfRestoreCfg.ppuProfileIdx);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssDxChPpuSrcPortProfileIndexGet failed");
    /* set default PPU Profile Idx of source port */
    rc = cpssDxChPpuSrcPortProfileIndexSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                         prvTgfPpuProfileIdx);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPpuSrcPortProfileIndexSet failed");

    /* ------------------------------------------------------
     * 3. Set PPU PPU profile table for a given profile index
     */

    /* save original PPU profile table entry*/
    rc = cpssDxChPpuProfileSet(prvTgfDevNum, prvTgfPpuProfileIdx, &prvTgfRestoreCfg.ppuProfileEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuProfileSet failed");

    /* set PPU Profile entry */
    profileEntry.ppuEnable           = GT_TRUE;
    profileEntry.anchorType          = CPSS_DXCH_PPU_PROFILE_ENTRY_ANCHOR_TYPE_L3_E;
    profileEntry.spBusDefaultProfile = prvTgfSpBusProfileIdx0;
    profileEntry.offset              = 19; /* 2 byte granularity pointing to IPv6 Tunell DIP SGT-ID ie byte #38 */
    profileEntry.ppuState            = prvTgfPpuState;

    /* write entry to PPU profile table */
    rc = cpssDxChPpuProfileSet(prvTgfDevNum, prvTgfPpuProfileIdx, &profileEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuProfileSet failed");

    /* ------------------------------------------
     * 4. Set SP Bus profile data to SP Bus profile table
     */

    /* save original spBusProfile */
    rc = cpssDxChPpuSpBusDefaultProfileGet(prvTgfDevNum, prvTgfSpBusProfileIdx0, &prvTgfRestoreCfg.spBusProfile0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuSpBusDefaultProfileGet failed");

    cpssOsMemSet(&spBusProfile, 0, sizeof(spBusProfile));

    /* write SP Bus profile entry */
    spBusProfile.data[16] = 1; /* up0 */
    spBusProfile.data[17] = 2; /* up1 */
    rc = cpssDxChPpuSpBusDefaultProfileSet(prvTgfDevNum, prvTgfSpBusProfileIdx0, &spBusProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuSpBusDefaultProfileSet failed");

}

/**
 * @internal prvTgfPpuKstg0ConfigSet function
 * @endinternal
 *
 * @brief    Set PPU K_stg 0 configuration
 *           - Set K_stg key generation profile.
 *           - Set tti key and mask to K_stg tcam table
 *           - Populate and write ppu Action Entry
 *
 */
static GT_VOID prvTgfPpuKstg0ConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32      kstgNum              = 0;
    GT_U32      keyGenProfileIdx     = (prvTgfPpuState >> 4) & 0xf;
    GT_U32      kstgTcamProfileIdx   = 8;
    GT_U32      kstgActionProfileIdx = kstgTcamProfileIdx;
    CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC keyGenProfile;
    CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC    ppuActionProfile;
    CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC      tcamEntry;

    /* initialize to ppuActionProfile to 0 */
    cpssOsMemSet(&ppuActionProfile, 0, sizeof(ppuActionProfile));
    cpssOsMemSet(&keyGenProfile, 0, sizeof(keyGenProfile));
    cpssOsMemSet(&tcamEntry, 0, sizeof(tcamEntry));

    /* set key generation profile from the extracted 32B packet header */
    keyGenProfile.byteSelOffset[0] = 61;   /* Header byte 0 value 0x01 */
    keyGenProfile.byteSelOffset[1] = 60;   /* Header byte 1 value 0xef */
    keyGenProfile.byteSelOffset[2] = 59;   /* Header byte 2 value 0xff */
    keyGenProfile.byteSelOffset[3] = 58;   /* Header byte 3 value 0xc0 */

    rc = cpssDxChPpuKstgKeyGenProfileSet(prvTgfDevNum, kstgNum, keyGenProfileIdx, &keyGenProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgKeyGenProfileSet failed");
    /* set kstg tcam profile */
    /* byte offset 60 and 61 contains the required Tunnel DIP.SGT-ID  and 58 and 59 contains copy reserved to be matched */
    tcamEntry.isValid = 1;
    tcamEntry.keyLsb  = 0xC0FFEF01;
    tcamEntry.keyMsb  = 0x00414141;
    tcamEntry.maskLsb = 0x00000000;
    tcamEntry.maskMsb = 0xFFFFFFFF;

    rc = cpssDxChPpuKstgTcamEntrySet(prvTgfDevNum,
            kstgNum, kstgTcamProfileIdx, &tcamEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgTcamEntrySet failed");

    /* Populate ppu action profile entry */
    ppuActionProfile.rotActionEntry[0].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_HD_E;
    ppuActionProfile.rotActionEntry[0].srcOffset         = OFFSET_MSB2LSB(0); /* 4 bits of SGT-ID from bit offset 248 */
    ppuActionProfile.rotActionEntry[0].srcNumValBits     = 3; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[0].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_NONE_E;
    ppuActionProfile.rotActionEntry[0].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[0].ldSpBusOffset     = 2; /* byte offset on SP bus value is 01 */
    ppuActionProfile.rotActionEntry[0].ldSpBusNumBytes   = 1; /* number of bytes */

    ppuActionProfile.rotActionEntry[1].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_HD_E;
    ppuActionProfile.rotActionEntry[1].srcOffset         = OFFSET_MSB2LSB(8); /* 8 bits of SGT-ID from bit offset 240 */
    ppuActionProfile.rotActionEntry[1].srcNumValBits     = 7; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[1].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_NONE_E;
    ppuActionProfile.rotActionEntry[1].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[1].ldSpBusOffset     = 1; /* byte offset on SP bus value is ef */
    ppuActionProfile.rotActionEntry[1].ldSpBusNumBytes   = 1; /* number of bytes */

    ppuActionProfile.rotActionEntry[2].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_HD_E;
    ppuActionProfile.rotActionEntry[2].srcOffset         = OFFSET_MSB2LSB(16); /* 8 bits of APP-ID from bit offset 232 */
    ppuActionProfile.rotActionEntry[2].srcNumValBits     = 7; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[2].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_NONE_E;
    ppuActionProfile.rotActionEntry[2].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[2].ldSpBusOffset     = 4; /* byte offset on SP bus value is ff */
    ppuActionProfile.rotActionEntry[2].ldSpBusNumBytes   = 1; /* number of bytes */

    ppuActionProfile.rotActionEntry[3].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_HD_E;
    ppuActionProfile.rotActionEntry[3].srcOffset         = OFFSET_MSB2LSB(30); /* 2 bits of APP-ID from bit offset 230 */
    ppuActionProfile.rotActionEntry[3].srcNumValBits     = 1; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[3].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_NONE_E;
    ppuActionProfile.rotActionEntry[3].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[3].ldSpBusOffset     = 3; /* byte offset on SP bus value is 3 */
    ppuActionProfile.rotActionEntry[3].ldSpBusNumBytes   = 1; /* number of bytes */

    /* set next state to DAU profile index */
    ppuActionProfile.setNextState = keyGenProfileIdx << 4;

    /* Write PPU action table entry */
    rc = cpssDxChPpuActionTableEntrySet(prvTgfDevNum,
            kstgNum, kstgActionProfileIdx, &ppuActionProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuActionTableEntrySet failed");

}

/**
 * @internal prvTgfPpuKstg1ConfigSet function
 * @endinternal
 *
 * @brief    Set PPU K_stg 1 configuration
 *           - Set K_stg key generation profile.
 *           - Set tti key and mask to K_stg tcam table
 *           - Populate and write ppu Action Entry
 *
 */
static GT_VOID prvTgfPpuKstg1ConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32      kstgNum              = 1;
    GT_U32      keyGenProfileIdx     = (prvTgfPpuState >> 4) & 0xf;
    GT_U32      kstgTcamProfileIdx   = 8;
    GT_U32      kstgActionProfileIdx = kstgTcamProfileIdx;
    CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC keyGenProfile;
    CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC    ppuActionProfile;
    CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC      tcamEntry;

    /* initialize to ppuActionProfile to 0 */
    cpssOsMemSet(&ppuActionProfile, 0, sizeof(ppuActionProfile));
    cpssOsMemSet(&keyGenProfile, 0, sizeof(keyGenProfile));
    cpssOsMemSet(&tcamEntry, 0, sizeof(tcamEntry));

    /* set key generation profile */

    keyGenProfile.byteSelOffset[0] = 61;   /* Header byte 0 */
    keyGenProfile.byteSelOffset[1] = 60;   /* Header byte 1 */
    keyGenProfile.byteSelOffset[2] = 59;   /* Header byte 2 */
    keyGenProfile.byteSelOffset[3] = 58;   /* Header byte 3 */

    rc = cpssDxChPpuKstgKeyGenProfileSet(prvTgfDevNum, kstgNum, keyGenProfileIdx, &keyGenProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgKeyGenProfileSet failed");
    /* set kstg tcam profile */
    /* byte offset 57 and 58 contains the required Tunnel DIP.SGT-ID to be matched */
    tcamEntry.isValid = 1;
    tcamEntry.keyLsb  = 0xC0FFEF01;
    tcamEntry.keyMsb  = 0x00414141;
    tcamEntry.maskLsb = 0x00000000;
    tcamEntry.maskMsb = 0xFFFFFFFF;

    rc = cpssDxChPpuKstgTcamEntrySet(prvTgfDevNum,
            kstgNum, kstgTcamProfileIdx, &tcamEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgTcamEntrySet failed");

    /* Populate ppu action profile entry */
    ppuActionProfile.rotActionEntry[0].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_HD_E;
    ppuActionProfile.rotActionEntry[0].srcOffset         = OFFSET_MSB2LSB(0);
    ppuActionProfile.rotActionEntry[0].srcNumValBits     = 3; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[0].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_NONE_E;
    ppuActionProfile.rotActionEntry[0].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[0].ldSpBusOffset     = 2; /* byte offset */
    ppuActionProfile.rotActionEntry[0].ldSpBusNumBytes   = 1; /* number of bytes */

    ppuActionProfile.rotActionEntry[1].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_HD_E;
    ppuActionProfile.rotActionEntry[1].srcOffset         = OFFSET_MSB2LSB(8);
    ppuActionProfile.rotActionEntry[1].srcNumValBits     = 7; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[1].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_NONE_E;
    ppuActionProfile.rotActionEntry[1].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[1].ldSpBusOffset     = 1; /* byte offset */
    ppuActionProfile.rotActionEntry[1].ldSpBusNumBytes   = 1; /* number of bytes */

    ppuActionProfile.rotActionEntry[2].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_HD_E;
    ppuActionProfile.rotActionEntry[2].srcOffset         = OFFSET_MSB2LSB(16);
    ppuActionProfile.rotActionEntry[2].srcNumValBits     = 7; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[2].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_NONE_E;
    ppuActionProfile.rotActionEntry[2].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[2].ldSpBusOffset     = 4; /* byte offset */
    ppuActionProfile.rotActionEntry[2].ldSpBusNumBytes   = 1; /* number of bytes */

    ppuActionProfile.rotActionEntry[3].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_HD_E;
    ppuActionProfile.rotActionEntry[3].srcOffset         = OFFSET_MSB2LSB(30);
    ppuActionProfile.rotActionEntry[3].srcNumValBits     = 1; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[3].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_NONE_E;
    ppuActionProfile.rotActionEntry[3].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[3].ldSpBusOffset     = 3; /* byte offset */
    ppuActionProfile.rotActionEntry[3].ldSpBusNumBytes   = 1; /* number of bytes */

    /* set next state to DAU profile index */
    ppuActionProfile.setNextState = keyGenProfileIdx << 4;

    /* Write PPU action table entry */
    rc = cpssDxChPpuActionTableEntrySet(prvTgfDevNum,
            kstgNum, kstgActionProfileIdx, &ppuActionProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuActionTableEntrySet failed");

}

/**
 * @internal prvTgfPpuKstg2ConfigSet function
 * @endinternal
 *
 * @brief    Set PPU K_stg 0 configuration
 *           - Set K_stg key generation profile.
 *           - Set tti key and mask to K_stg tcam table
 *           - Populate and write ppu Action Entry
 *
 */
static GT_VOID prvTgfPpuKstg2ConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32      kstgNum              = 2;
    GT_U32      keyGenProfileIdx     = (prvTgfPpuState >> 4) & 0xf;
    GT_U32      kstgTcamProfileIdx   = 8;
    GT_U32      dauProfileIdx        = 12;
    GT_U32      kstgActionProfileIdx = kstgTcamProfileIdx;
    CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC keyGenProfile;
    CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC    ppuActionProfile;
    CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC      tcamEntry;

    /* initialize to ppuActionProfile to 0 */
    cpssOsMemSet(&ppuActionProfile, 0, sizeof(ppuActionProfile));
    cpssOsMemSet(&keyGenProfile, 0, sizeof(keyGenProfile));
    cpssOsMemSet(&tcamEntry, 0, sizeof(tcamEntry));

    /* set key generation profile */
    keyGenProfile.byteSelOffset[0] = 61;   /* Header byte 0 */
    keyGenProfile.byteSelOffset[1] = 60;   /* Header byte 1 */
    keyGenProfile.byteSelOffset[2] = 59;   /* Header byte 2 */
    keyGenProfile.byteSelOffset[3] = 58;   /* Header byte 3 */

    rc = cpssDxChPpuKstgKeyGenProfileSet(prvTgfDevNum, kstgNum, keyGenProfileIdx, &keyGenProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgKeyGenProfileSet failed");
    /* set kstg tcam profile */
    /* byte offset 57 and 58 contains the required Tunnel DIP.SGT-ID to be matched */
    tcamEntry.isValid = 1;
    tcamEntry.keyLsb  = 0xC0FFEF01;
    tcamEntry.keyMsb  = 0x00414141;
    tcamEntry.maskLsb = 0x00000000;
    tcamEntry.maskMsb = 0xFFFFFFFF;

    rc = cpssDxChPpuKstgTcamEntrySet(prvTgfDevNum,
            kstgNum, kstgTcamProfileIdx, &tcamEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgTcamEntrySet failed");

    /* Populate ppu action profile entry */
    ppuActionProfile.rotActionEntry[0].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_HD_E;
    ppuActionProfile.rotActionEntry[0].srcOffset         = OFFSET_MSB2LSB(0);
    ppuActionProfile.rotActionEntry[0].srcNumValBits     = 3; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[0].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_NONE_E;
    ppuActionProfile.rotActionEntry[0].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[0].ldSpBusOffset     = 2; /* byte offset */
    ppuActionProfile.rotActionEntry[0].ldSpBusNumBytes   = 1; /* number of bytes */

    ppuActionProfile.rotActionEntry[1].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_HD_E;
    ppuActionProfile.rotActionEntry[1].srcOffset         = OFFSET_MSB2LSB(8);
    ppuActionProfile.rotActionEntry[1].srcNumValBits     = 7; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[1].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_NONE_E;
    ppuActionProfile.rotActionEntry[1].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[1].ldSpBusOffset     = 1; /* byte offset */
    ppuActionProfile.rotActionEntry[1].ldSpBusNumBytes   = 1; /* number of bytes */

    ppuActionProfile.rotActionEntry[2].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_HD_E;
    ppuActionProfile.rotActionEntry[2].srcOffset         = OFFSET_MSB2LSB(16);
    ppuActionProfile.rotActionEntry[2].srcNumValBits     = 7; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[2].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_NONE_E;
    ppuActionProfile.rotActionEntry[2].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[2].ldSpBusOffset     = 4; /* byte offset */
    ppuActionProfile.rotActionEntry[2].ldSpBusNumBytes   = 1; /* number of bytes */

    ppuActionProfile.rotActionEntry[3].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_HD_E;
    ppuActionProfile.rotActionEntry[3].srcOffset         = OFFSET_MSB2LSB(30);
    ppuActionProfile.rotActionEntry[3].srcNumValBits     = 1; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[3].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_NONE_E;
    ppuActionProfile.rotActionEntry[3].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[3].ldSpBusOffset     = 3; /* byte offset */
    ppuActionProfile.rotActionEntry[3].ldSpBusNumBytes   = 1; /* number of bytes */

    /* set next state to DAU profile index */
    ppuActionProfile.setNextState = dauProfileIdx;

    /* Write PPU action table entry */
    rc = cpssDxChPpuActionTableEntrySet(prvTgfDevNum,
            kstgNum, kstgActionProfileIdx, &ppuActionProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuActionTableEntrySet failed");

}

/**
 * @internal prvTgfPpuDauConfigSet function
 * @endinternal
 *
 * @brief    Set PPU DAU profile configuration
 *           - Write first four bytes of MAC source address
 *
 */
static GT_VOID prvTgfPpuDauConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32      dauProfileIdx        = 12;
    CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC dauProfile;

    /* initialize DAU profile to 0 */
    cpssOsMemSet(&dauProfile, 0, sizeof(dauProfile));

    dauProfile.setDescBits[0].spByteWriteEnable  = GT_TRUE;
    dauProfile.setDescBits[0].spByteSrcOffset    = 1 * 8;   /* bit offset on SP bus */
    dauProfile.setDescBits[0].spByteNumBits      = 7;       /* num of bits - 1 = 8 - 1 */
    dauProfile.setDescBits[0].spByteTargetOffset = 1998;    /* SST ID bit offset 0 */

    dauProfile.setDescBits[1].spByteWriteEnable  = GT_TRUE;
    dauProfile.setDescBits[1].spByteSrcOffset    = 2 * 8;   /* bit offset on SP bus */
    dauProfile.setDescBits[1].spByteNumBits      = 3;       /* num of bits - 1 = 4 - 1 */
    dauProfile.setDescBits[1].spByteTargetOffset = 2006;    /* SST ID bit offset 8 */

    dauProfile.setDescBits[2].spByteWriteEnable  = GT_TRUE;
    dauProfile.setDescBits[2].spByteSrcOffset    = 3 * 8;   /* bit offset on SP bus */
    dauProfile.setDescBits[2].spByteNumBits      = 1;       /* num of bits - 1 = 2 - 1 */
    dauProfile.setDescBits[2].spByteTargetOffset = 2056;    /* Copy reserved bit offset 2*/

    dauProfile.setDescBits[3].spByteWriteEnable  = GT_TRUE;
    dauProfile.setDescBits[3].spByteSrcOffset    = 4 * 8;   /* bit offset on SP bus */
    dauProfile.setDescBits[3].spByteNumBits      = 7;       /* num of bits - 1 = 8 - 1 */
    dauProfile.setDescBits[3].spByteTargetOffset = 2058;    /* Copy Reserved bit offset 4 */

    /* write entry to DAU profile tabl */
    rc = cpssDxChPpuDauProfileEntrySet(prvTgfDevNum,
            dauProfileIdx, &dauProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuDauProfileEntrySet failed");

}

/**
* @internal prvTgSrv6BeTunnelPclParseOnLeafNodeConfigSet function
* @endinternal
*
* @brief   SRv6 Best Effort Tunnel Parse on Border Node use case test.
*/
GT_VOID prvTgSrv6BeTunnelPclParseOnLeafNodeConfigSet
(
    GT_VOID
)
{
    GT_STATUS                             rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT           mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT           pattern;
    PRV_TGF_PCL_ACTION_STC                action;
    GT_U32                                tcQueue = 2;
    CPSS_INTERFACE_INFO_STC               phyPortInfo;
    GT_U32                                udbIndexArr[3] = {40,41,42};
    PRV_TGF_PCL_UDB_SELECT_STC            udbSelect;
    PRV_TGF_NET_CPU_CODE_TABLE_ENTRY_STC  cpuCodeEntry;
    CPSS_DXCH_NET_SDMA_RX_COUNTERS_STC    rxCounters;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* clear entry */
    cpssOsMemSet(&action,    0, sizeof(action));
    cpssOsMemSet(&mask,      0, sizeof(mask));
    cpssOsMemSet(&pattern,   0, sizeof(pattern));
    cpssOsMemSet(&udbSelect, 0, sizeof(udbSelect));

    /* AUTODOC: Init IPCL Engine for port 3 */
    rc = prvTgfPclDefPortInit(
            prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_NUMBER_0_E,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E     /*nonIpKey*/,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E  /*ipv6Key*/);
    UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfPclDefPortInit: %d, %d, %d",
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_INGRESS_E);

    /* IPCL actions - FORWARD packet */
    action.pktCmd                                            = CPSS_PACKET_CMD_FORWARD_E;
    action.bypassBridge                                      = GT_TRUE;
    action.redirect.redirectCmd                              = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    action.redirect.data.outIf.outInterface.type             = CPSS_INTERFACE_PORT_E;
    action.redirect.data.outIf.outInterface.devPort.hwDevNum = prvTgfDevNum;
    action.redirect.data.outIf.outInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    /* AUTODOC: IPCL Mask for MAC SA */
    cpssOsMemSet(mask.ruleStdNotIp.macDa.arEther, 0xFF, sizeof(GT_ETHERADDR));

    /* AUTODOC: IPCL Pattern for MAC SA */
    cpssOsMemCpy(pattern.ruleStdNotIp.macDa.arEther,
                prvTgfOriginalPacketL2Part.daMac,
                sizeof(GT_ETHERADDR));

    rc = prvTgfPclRuleSet(
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                IPCL_MATCH_INDEX_0_CNS, &mask, &pattern, &action);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                     PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                                     IPCL_MATCH_INDEX_0_CNS);

    /* AUTODOC: Init EPCL Engine for port 1 */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_NUMBER_0_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E  /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfPclDefPortInit: %d, %d, %d",
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_EGRESS_E);

    /* Mapping UDB_0/1/2 to UDB_40/41/42 */
    udbSelect.udbSelectArr[0] = udbIndexArr[0];
    udbSelect.udbSelectArr[1] = udbIndexArr[1];
    udbSelect.udbSelectArr[2] = udbIndexArr[2];

    if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
    {
        GT_U32  ii;
        for(ii = 0 ; ii < 10; ii++)
        {
            udbSelect.udbSelectArr[ii] = ii;
        }
        udbIndexArr[0] = udbSelect.udbSelectArr[0];
        udbIndexArr[1] = udbSelect.udbSelectArr[1];
        udbIndexArr[2] = udbSelect.udbSelectArr[2];
    }

    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    /* AUTODOC: Metadata UDBs configuration for copyReserved */
    /* EPCL UDBs Metadata:(724:705) -  copyReserved(20 bits) */
    rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                 PRV_TGF_PCL_PACKET_TYPE_IPV6_OTHER_E,
                                 CPSS_PCL_DIRECTION_EGRESS_E,
                                 udbIndexArr[0],
                                 PRV_TGF_PCL_OFFSET_METADATA_E,
                                 88);   /* Anchor offset: 88 Bit offset: 7:1 */
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                                "prvTgfPclUserDefinedByteSet: packetType %d, udbIndex %d,"
                                "OffsetType %d, Offset %d\n",
                                 PRV_TGF_PCL_PACKET_TYPE_IPV6_OTHER_E,
                                 udbIndexArr[0], PRV_TGF_PCL_OFFSET_METADATA_E, 88);

    rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                 PRV_TGF_PCL_PACKET_TYPE_IPV6_OTHER_E,
                                 CPSS_PCL_DIRECTION_EGRESS_E,
                                 udbIndexArr[1],
                                 PRV_TGF_PCL_OFFSET_METADATA_E,
                                 89);  /* Anchor offset: 89 Bit offset: 7:0 */
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                                "prvTgfPclUserDefinedByteSet: packetType %d, udbIndex %d,"
                                "OffsetType %d, Offset %d\n",
                                 PRV_TGF_PCL_PACKET_TYPE_IPV6_OTHER_E,
                                 udbIndexArr[1], PRV_TGF_PCL_OFFSET_METADATA_E, 89);

    rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                 PRV_TGF_PCL_PACKET_TYPE_IPV6_OTHER_E,
                                 CPSS_PCL_DIRECTION_EGRESS_E,
                                 udbIndexArr[2],
                                 PRV_TGF_PCL_OFFSET_METADATA_E,
                                 90); /* Anchor offset: 90 Bit offset: 4:0 */
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                                 "prvTgfPclUserDefinedByteSet: packetType %d, udbIndex %d,"
                                 "OffsetType %d, Offset %d\n",
                                 PRV_TGF_PCL_PACKET_TYPE_IPV6_OTHER_E,
                                 udbIndexArr[2], PRV_TGF_PCL_OFFSET_METADATA_E, 90);

    rc = prvTgfPclUserDefinedBytesSelectSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                            PRV_TGF_PCL_PACKET_TYPE_IPV6_OTHER_E, CPSS_PCL_LOOKUP_0_E,
                                            &udbSelect);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedBytesSelectSet: %d, %d",
                                 PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E, CPSS_PCL_LOOKUP_0_E);

    /* AUTODOC: mask and pattern for copyReserved */
    mask.ruleEgrUdbOnly.udb[0] = 0xFE;
    mask.ruleEgrUdbOnly.udb[1] = 0xFF;
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        mask.ruleEgrUdbOnly.udb[2] = 0x1F;
    }
    else if (PRV_CPSS_SIP_5_20_CHECK_MAC(prvTgfDevNum))
    {
        mask.ruleEgrUdbOnly.udb[2] = 0xF;
    }
    else
    {
        mask.ruleEgrUdbOnly.udb[2] = 0x3;
    }

    pattern.ruleEgrUdbOnly.udb[0] =  ((PRV_TGF_APP_ID_MASK) << 3)  & 0xFE;
    pattern.ruleEgrUdbOnly.udb[1] =  ((PRV_TGF_APP_ID_MASK) >> 5)  & 0xFF;
    pattern.ruleEgrUdbOnly.udb[2] =  ((PRV_TGF_APP_ID_MASK) >> 13) & 0x1F;

    /* AUTODOC: EPCL action - drop the packet */
    action.egressPolicy                          = GT_TRUE;
    action.pktCmd                                = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    action.mirror.cpuCode                       = CPSS_NET_USER_DEFINED_0_E;
    action.redirect.redirectCmd                  = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    action.redirect.data.outIf.outInterface.type = CPSS_INTERFACE_VID_E;

    rc = prvTgfPclRuleSet(
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            EPCL_MATCH_INDEX_CNS, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                 PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                 EPCL_MATCH_INDEX_CNS);

    /* set tcQueue for cpuCode CPSS_NET_INTERVENTION_PORT_LOCK_E to validate packet exception based on new MAC SA */
    cpssOsMemSet(&cpuCodeEntry, 0, sizeof(cpuCodeEntry));
    cpuCodeEntry.tc = tcQueue;
    prvTgfRestoreCfg.tcQueueGet = tcQueue;
    rc = prvTgfNetIfCpuCodeTableGet(prvTgfDevNum, CPSS_NET_USER_DEFINED_0_E, &prvTgfRestoreCfg.cpuCodeEntryGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfNetIfCpuCodeTableSet failed, device %d cpuCode %d\n", prvTgfDevNum, CPSS_NET_FIRST_USER_DEFINED_E);
    rc = prvTgfNetIfCpuCodeTableSet(prvTgfDevNum, CPSS_NET_USER_DEFINED_0_E, &cpuCodeEntry);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfNetIfCpuCodeTableSet failed, device %d cpuCode %d\n", prvTgfDevNum, CPSS_NET_FIRST_USER_DEFINED_E);

    /* clear the queue counters */
    rc = cpssDxChNetIfSdmaRxCountersGet(prvTgfDevNum, tcQueue, &rxCounters);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChNetIfSdmaRxCountersGet failed, device %d tcQueue %d\n", prvTgfDevNum, tcQueue);

    /* AUTODOC: Set physical info for the given ePort in the E2PHY mapping table */
    cpssOsMemSet(&phyPortInfo,0,sizeof(phyPortInfo));
    phyPortInfo.type              = CPSS_INTERFACE_PORT_E;
    phyPortInfo.devPort.hwDevNum  = PRV_CPSS_HW_DEV_NUM_MAC(prvTgfDevNum);
    phyPortInfo.devPort.portNum   = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                                                       prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                                       &(prvTgfRestoreCfg.egressInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoGet);

    rc = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS], &phyPortInfo);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgEportToPhysicalPortTargetMappingTableSet: %d %d",prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS]);
}

/**
* @internal prvTgSrv6BeTunnelTtiPclParseOnLeafNodeConfigSet function
* @endinternal
*
* @brief   SRv6 Best Effort Tunnel Parse on Leaf Node use case test.
*/
GT_VOID prvTgSrv6BeTunnelTtiPclParseOnLeafNodeConfigSet
(
    GT_VOID
)
{
    /* TTI Engine Configurations */
    prvTgSrv6BeTunnelTtiParseOnLeafNodeConfigSet();

    /* ppu init configuration set */
    prvTgfPpuInitConfigSet();

    /* PPU K_stg 0 configuration Set */
    prvTgfPpuKstg0ConfigSet();

    /* PPU K_stg 1 configuration Set */
    prvTgfPpuKstg1ConfigSet();

    /* PPU K_stg 2 configuration Set */
    prvTgfPpuKstg2ConfigSet();

    /* PPU DAU configuration Set */
    prvTgfPpuDauConfigSet();

    /* PCL Engine Configurations */
    prvTgSrv6BeTunnelPclParseOnLeafNodeConfigSet();

}

/**
* internal tgfSrv6BestEffortTunnelParseOnLeafNodeConfigSet function
* @endinternal
*
* @brief   SRV6 Best Effort Tunnel Parse on Leaf Node use case configurations
*/
GT_VOID tgfSrv6BestEffortTunnelParseOnLeafNodeConfigSet
(
    GT_VOID
)
{
    /* Bridging Engine Configuration
     * Route the packet from ingress to Egress */
    prvTgSrv6BridgingConfigSet(prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS], &prvTgfOriginalPacketL2Part.daMac);

    /* TTI Engine Configurations */
    prvTgSrv6BeTunnelTtiPclParseOnLeafNodeConfigSet();

}

/**
* @internal tgfSrv6BestEffortTunnelParseOnLeafNodeTest function
* @endinternal
*
* @brief   SRv6 Best Effort Tunnel Parse on Leaf Node use case test.
*/
GT_VOID tgfSrv6BestEffortTunnelParseOnLeafNodeTest
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIter, ii;
    CPSS_DXCH_NET_SDMA_RX_COUNTERS_STC      rxCounters;
    GT_U32                                  exceptionPktCount = 0;

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum,
                    prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "prvTgfResetCountersEth: %d, %d",
                    prvTgfDevNum,
                    prvTgfPortsArray[portIter]);
    }

    for (ii = 0; ii < 8; ii++)
    {
        prvTgfTunnelPacketIpv6Part.dstAddr[ii] = prvTgf1stTunnelStartDstIPv6[ii];
        prvTgfTunnelPacketIpv6Part.srcAddr[ii] = prvTgf1stTunnelStartSrcIPv6[ii];
    }
    prvTgfTunnelPacketIpv6Part.dstAddr[5] = PRV_TGF_EVLAN_SERVISE_ID_CNS;
    prvTgfTunnelPacketIpv6Part.dstAddr[6] = PRV_TGF_SOURCE_INDEX_CNS;
    prvTgfTunnelPacketIpv6Part.dstAddr[7] = (PRV_TGF_APP_INDEX_CNS & PRV_TGF_APP_ID_MASK) << PRV_TGF_RESERVE_ID_BIT_CNS;

    packetInfo.numOfParts =  sizeof(prvTgfSRv6PacketArray) / sizeof(prvTgfSRv6PacketArray[0]);
    packetInfo.partsArray = prvTgfSRv6PacketArray;
    packetInfo.totalLen   = TGF_PACKET_AUTO_CALC_LENGTH_CNS;

    /* setup receive portInterface for capturing */
    egrPortInterface.type               = CPSS_INTERFACE_PORT_E;
    egrPortInterface.devPort.hwDevNum   = prvTgfDevNum;
    egrPortInterface.devPort.portNum    = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    /* AUTODOC: GENERATE TRAFFIC & Verify at ePort#3 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
                        &egrPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgftrafficgeneratorporttxethcaptureset port - %d",
                                        egrPortInterface.devPort.portNum);

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, 1, 0, NULL);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfSetTxSetupEth");

    /* Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");

    /* Verify packet should reach in case of enabled queue*/
    rc = cpssDxChNetIfSdmaRxCountersGet(prvTgfDevNum, prvTgfRestoreCfg.tcQueueGet, &rxCounters);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChNetIfSdmaRxCountersGet failed, device %d tcQueue %d\n", prvTgfDevNum, prvTgfRestoreCfg.tcQueueGet);

    exceptionPktCount = rxCounters.rxInPkts;
    rc = (exceptionPktCount == 1) ? GT_OK : GT_FAIL;
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "SDMA Rx count expected %d received %d tcQueue %d\n", 1, exceptionPktCount, prvTgfRestoreCfg.tcQueueGet);

    /* disable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &egrPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet port - %d",
            egrPortInterface.devPort.portNum);

}

/**
* @internal tgfSrv6BestEffortTunnelParseOnLeafNodeConfigRestore function
* @endinternal
*
* @brief   SRv6 Best Effort Tunnel Parse on Border Node use case configurations restore.
*          1. Restore Bridge Configuration
*          2. Restore Base Configuration
*
*/
GT_VOID tgfSrv6BestEffortTunnelParseOnLeafNodeConfigRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U32                                 keyGenProfileIdx     = (prvTgfPpuState >> 4) & 0xf;
    GT_U32                                 kstgTcamProfileIdx   = 8;
    GT_U32                                 dauProfileIdx        = 12;
    GT_U32                                 kstgActionProfileIdx = kstgTcamProfileIdx;
    CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC keyGenProfile;
    CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC    ppuActionProfile;
    CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC      tcamEntry;
    CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC    dauProfile;

    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_UDB_IPV6_E, prvTgfRestoreCfg.lookupEnableGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: invalidate TTI rules  */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");

    /* AUTODOC: invalidate PCL rules  */
    rc = prvTgfPclRuleValidStatusSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E, IPCL_MATCH_INDEX_0_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d",
                                     PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                                     IPCL_MATCH_INDEX_0_CNS);
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, EPCL_MATCH_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d",
                                     PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                     EPCL_MATCH_INDEX_CNS);

    /* AUTODOC: Disables I/EPCL Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);
    rc = prvTgfPclEgressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclEgressPolicyEnable: %d", GT_FALSE);

    /* AUTODOC: Restore ePort attributes */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                        prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                        &(prvTgfRestoreCfg.egressInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: Restore cpu code table */
    rc = prvTgfNetIfCpuCodeTableSet(prvTgfDevNum, CPSS_NET_USER_DEFINED_0_E, &prvTgfRestoreCfg.cpuCodeEntryGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfNetIfCpuCodeTableSet failed, device %d cpuCode %d\n", prvTgfDevNum, CPSS_NET_FIRST_USER_DEFINED_E);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "tgfTrafficTableRxPcktTblClear");

    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "tgfTrafficTableRxStartCapture");

    /* -------------------------------------------------------------------------
     * Restore PPU Configuration
     */

    /* restore ppu enable in PPU unit */
    rc = cpssDxChPpuEnableSet(prvTgfDevNum, prvTgfRestoreCfg.ppuEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuEnableSet failed");

    /* restore profile entry */
    rc = cpssDxChPpuProfileSet(prvTgfDevNum, prvTgfPpuProfileIdx, &prvTgfRestoreCfg.ppuProfileEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuProfileSet failed");

    /* restore SP bus profile entry 0*/
    rc = cpssDxChPpuSpBusDefaultProfileSet(prvTgfDevNum, prvTgfSpBusProfileIdx0, &prvTgfRestoreCfg.spBusProfile0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuSpBusDefaultProfileSet failed");

    /* restore key generation profiles */
    cpssOsMemSet(&keyGenProfile, 0, sizeof(keyGenProfile));
    rc = cpssDxChPpuKstgKeyGenProfileSet(prvTgfDevNum, 0 /*kstgNum*/, keyGenProfileIdx, &keyGenProfile);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgKeyGenProfileSet K_stg=%d failed", 0);
    rc = cpssDxChPpuKstgKeyGenProfileSet(prvTgfDevNum, 1 /*kstgNum*/, keyGenProfileIdx, &keyGenProfile);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgKeyGenProfileSet K_stg=d failed", 1);
    rc = cpssDxChPpuKstgKeyGenProfileSet(prvTgfDevNum, 2 /*kstgNum*/, keyGenProfileIdx, &keyGenProfile);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgKeyGenProfileSet K_stg=%d failed", 2);

    /* restore tcam profiles */
    cpssOsMemSet(&tcamEntry, 0, sizeof(tcamEntry));
    rc = cpssDxChPpuKstgTcamEntrySet(prvTgfDevNum, 0 /*kstgNum*/, kstgTcamProfileIdx, &tcamEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgTcamEntrySet K_stg=%d failed", 0);
    rc = cpssDxChPpuKstgTcamEntrySet(prvTgfDevNum, 1 /*kstgNum*/, kstgTcamProfileIdx, &tcamEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgTcamEntrySet K_stg=%d failed", 1);
    rc = cpssDxChPpuKstgTcamEntrySet(prvTgfDevNum, 2 /*kstgNum*/, kstgTcamProfileIdx, &tcamEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgTcamEntrySet K_stg=%d failed", 2);

    /* restore ppu action profiles */
    cpssOsMemSet(&ppuActionProfile, 0, sizeof(ppuActionProfile));
    rc = cpssDxChPpuActionTableEntrySet(prvTgfDevNum, 0 /*kstgNum*/, kstgActionProfileIdx, &ppuActionProfile);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"cpssDxChPpuActionTableEntrySet K_stg=%d failed", 0);
    rc = cpssDxChPpuActionTableEntrySet(prvTgfDevNum, 1 /*kstgNum*/, kstgActionProfileIdx, &ppuActionProfile);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"cpssDxChPpuActionTableEntrySet K_stg=%d failed", 1);
    rc = cpssDxChPpuActionTableEntrySet(prvTgfDevNum, 2 /*kstgNum*/, kstgActionProfileIdx, &ppuActionProfile);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"cpssDxChPpuActionTableEntrySet K_stg=%d failed", 2);

    /* restore dau profile */
    cpssOsMemSet(&dauProfile, 0, sizeof(dauProfile));
    rc = cpssDxChPpuDauProfileEntrySet(prvTgfDevNum, dauProfileIdx, &dauProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuDauProfileEntrySet failed");
}

/**
* @internal prvTgSrv6GSidTunnelTtiPclConfigSet function
* @endinternal
*
* @brief   TTI/IPCL configuration for SRv6 G-SID Tunnel use case
*/
static GT_VOID prvTgSrv6GSidTunnelTtiPclConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc;
    PRV_TGF_TTI_ACTION_2_STC            ttiAction;
    PRV_TGF_TTI_RULE_UNT                ttiMask;
    PRV_TGF_TTI_RULE_UNT                ttiPattern;
    PRV_TGF_PCL_RULE_FORMAT_UNT         mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT         pattern;
    PRV_TGF_PCL_ACTION_STC              action;

    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* set the TTI lookup MAC mode for PRV_TGF_TTI_KEY_ETH_E */
    rc = prvTgfTtiMacModeGet(prvTgfDevNum,PRV_TGF_TTI_KEY_ETH_E,&prvTgfRestoreCfg.origMacMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeGet");

    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_ETH_E,PRV_TGF_TTI_MAC_MODE_SA_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet");

    /* enable the TTI lookup for PRV_TGF_TTI_KEY_ETH_E */
    rc = prvTgfTtiPortLookupEnableGet(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_ETH_E, &prvTgfRestoreCfg.lookupEnableGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableGet");

    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_ETH_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: TTI Mask for MAC SA */
    cpssOsMemSet(ttiMask.eth.common.mac.arEther, 0xFF, sizeof(GT_ETHERADDR));

    /* AUTODOC: TTI Pattern for MAC SA */
    cpssOsMemCpy(ttiPattern.eth.common.mac.arEther,
                 prvTgfOriginalPacketL2Part.saMac,
                 sizeof(GT_ETHERADDR));

    /* TTI actions - FORWARD packet */
    ttiAction.command                          = CPSS_PACKET_CMD_FORWARD_E;
    /* TTI actions - Copy Reserved Assignment */
    ttiAction.copyReservedAssignmentEnable     = GT_TRUE;
    ttiAction.copyReserved                     = PRV_TGF_APP_INDEX_CNS;
    /* TTI actions - Source ID Assignment */
    ttiAction.sourceIdSetEnable                = GT_TRUE;
    ttiAction.sourceId                         = PRV_TGF_SOURCE_INDEX_CNS;

    rc = prvTgfTtiRule2Set(PRV_TGF_TTI_INDEX_CNS, PRV_TGF_TTI_KEY_ETH_E,
                           &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet");

    /* AUTODOC: Init IPCL Engine for port 0 */
    rc = prvTgfPclDefPortInit(
            prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_NUMBER_0_E,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E     /*nonIpKey*/,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E  /*ipv6Key*/);
    UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfPclDefPortInit: %d, %d, %d",
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_INGRESS_E);

    /* IPCL actions - FORWARD packet */
    action.pktCmd                                            = CPSS_PACKET_CMD_FORWARD_E;
    action.bypassBridge                                      = GT_TRUE;
    action.redirect.redirectCmd                              = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    action.redirect.data.outIf.outInterface.type             = CPSS_INTERFACE_PORT_E;
    action.redirect.data.outIf.outInterface.devPort.hwDevNum = prvTgfDevNum;
    action.redirect.data.outIf.outInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    /* AUTODOC: IPCL Mask for MAC SA */
    cpssOsMemSet(mask.ruleStdNotIp.macSa.arEther, 0xFF, sizeof(GT_ETHERADDR));

    /* AUTODOC: IPCL Pattern for MAC SA */
    cpssOsMemCpy(pattern.ruleStdNotIp.macSa.arEther,
                prvTgfOriginalPacketL2Part.saMac,
                sizeof(GT_ETHERADDR));

    rc = prvTgfPclRuleSet(
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                IPCL_MATCH_INDEX_0_CNS, &mask, &pattern, &action);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                     PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                                     IPCL_MATCH_INDEX_0_CNS);
}

/**
* @internal prvTgfSrv6GSidTunnelEgressConfigSet function
* @endinternal
*
* @brief   Egress port TS IPv6 config for "SRv6 G-SID Tunnel use case".
*/
static GT_VOID prvTgfSrv6GSidTunnelEgressConfigSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC        egressInfo;
    PRV_TGF_TUNNEL_START_ENTRY_UNT          tunnelEntry, tunnelEntryGet;
    CPSS_TUNNEL_TYPE_ENT                    prvTgfTunnelType = CPSS_TUNNEL_GENERIC_IPV6_E;
    CPSS_TUNNEL_TYPE_ENT                    tunnelTypeGet;
    GT_U32                                  ii;

    /* AUTODOC: Egress tunnel SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Egress tunnel ipv6 Configuration =======\n");

    /* Save ePort attributes configuration */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                                                       prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                                       &(prvTgfRestoreCfg.egressInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoGet);

    /* Set ePort attributes configuration */
    cpssOsMemSet(&egressInfo,0,sizeof(egressInfo));
    egressInfo                        = prvTgfRestoreCfg.egressInfo;
    egressInfo.tunnelStart            = GT_TRUE;
    egressInfo.tunnelStartPtr         = PRV_TGF_TUNNEL_START_INDEX_CNS;
    egressInfo.tsPassengerPacketType  = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;

    /* AUTODOC: config ePort attributes: */
    /* AUTODOC:   Tunnel Start=ENABLE, tunnelStartPtr=8 */
    /* AUTODOC:   tsPassenger=ETHERNET */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                        prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                        &egressInfo);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

    /* AUTODOC: Set Generic IPv6 tunnel start entry */
    PRV_UTF_LOG0_MAC("======= Setting Tunnel Start Configuration =======\n");
    cpssOsMemSet(&tunnelEntry.ipv6Cfg,0,sizeof(tunnelEntry.ipv6Cfg));
    /*** Set a tunnel start entry ***/
    tunnelEntry.ipv6Cfg.tagEnable        = GT_TRUE;
    tunnelEntry.ipv6Cfg.vlanId           = PRV_TGF_DEFAULT_VLANID_CNS;
    tunnelEntry.ipv6Cfg.ttl              = 33;
    tunnelEntry.ipv6Cfg.retainCrc        = GT_FALSE;
    tunnelEntry.ipv6Cfg.ipHeaderProtocol = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_GENERIC_E;
    tunnelEntry.ipv6Cfg.ipProtocol       = 43 /* Ignored */;
    tunnelEntry.ipv6Cfg.udpSrcPort       = udpSrcPort;
    tunnelEntry.ipv6Cfg.udpDstPort       = udpDstPort;
    tunnelEntry.ipv6Cfg.profileIndex     = PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS;

    /* tunnel next hop MAC DA, IP DA, IP SA */
    cpssOsMemCpy(tunnelEntry.ipv6Cfg.macDa.arEther, prvTgfOriginalPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    /* 80b Locator */
    for (ii = 0; ii < 4; ii++)
    {
        tunnelEntry.ipv6Cfg.destIp.arIP[ii * 2]     = (GT_U8)(prvTgf1stTunnelStartDstIPv6[ii] >> 8 & 0xFF);
        tunnelEntry.ipv6Cfg.destIp.arIP[ii * 2 + 1] = (GT_U8) prvTgf1stTunnelStartDstIPv6[ii] & 0xFF;
    }
    for (ii = 0; ii < 8; ii++)
    {
        tunnelEntry.ipv6Cfg.srcIp.arIP[ii * 2]     = (GT_U8)(prvTgf1stTunnelStartSrcIPv6[ii] >> 8 & 0xFF);
        tunnelEntry.ipv6Cfg.srcIp.arIP[ii * 2 + 1] = (GT_U8) prvTgf1stTunnelStartSrcIPv6[ii] & 0xFF;
    }

    tunnelEntry.ipv6Cfg.destIp.arIP[8]  = (GT_U8) ((PRV_TGF_GSID_NODE_INDEX_CNS >> 8) & 0xFF);
    tunnelEntry.ipv6Cfg.destIp.arIP[9]  = (GT_U8) PRV_TGF_GSID_NODE_INDEX_CNS & 0xFF;
    tunnelEntry.ipv6Cfg.destIp.arIP[10] = (GT_U8)((PRV_TGF_GSID_FUNCTION_INDEX_CNS >> 8) & 0xFF);
    tunnelEntry.ipv6Cfg.destIp.arIP[11] = (GT_U8) PRV_TGF_GSID_FUNCTION_INDEX_CNS & 0xFF;

    /* FirstBit and LastBit of Source-ID to represent SGT-ID */
    tunnelEntry.ipv6Cfg.destIp.arIP[12] = PRV_TGF_SOURCE_ID_LAST_BIT;
    tunnelEntry.ipv6Cfg.destIp.arIP[13] = PRV_TGF_SOURCE_ID_FIRST_BIT;

    /* AUTODOC: add TS entry 8 with: */
    /* AUTODOC:   tunnelType = GENERIC_IPV6 */
    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_INDEX_CNS, prvTgfTunnelType, &tunnelEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntrySet: %d", prvTgfDevNum);

    /* AUTODOC: get and check added TS Entry */
    rc = prvTgfTunnelStartEntryGet(prvTgfDevNum, PRV_TGF_TUNNEL_START_INDEX_CNS, &tunnelTypeGet, &tunnelEntryGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntryGet: %d", prvTgfDevNum);

    /* Check vlanId field */
    UTF_VERIFY_EQUAL1_STRING_MAC(tunnelEntryGet.ipv6Cfg.vlanId, tunnelEntry.ipv6Cfg.vlanId,
            "Vlan Id mismatch with configured value for device %d", prvTgfDevNum);

    /* Check srcIp field */
    rc = cpssOsMemCmp(&tunnelEntryGet.ipv6Cfg.srcIp, &tunnelEntry.ipv6Cfg.srcIp, sizeof(GT_IPV6ADDR)) == 0 ? GT_OK : GT_FALSE;
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tunnelEntry.ipv6Cfg.srcIp did not match");

}

/**
* internal prvTgfSrv6GSidTunnelPhaConfigSet function
* @endinternal
*
* @brief   PHA config for SRv6 G-SID Tunnel One SRH Container use case.
*/
static GT_VOID prvTgfSrv6GSidTunnelPhaConfigSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    CPSS_DXCH_PHA_THREAD_INFO_UNT           extInfo;
    GT_U32                                  ii;

    /* GM does not support PHA */
    if(GT_TRUE == prvUtfIsGmCompilation() || (prvTgfSrv6OneSegmentSkipPha == GT_TRUE))
        return;

    /* AUTODOC: Assign PHA firmware image ID 01 */
    rc = cpssDxChPhaFwImageIdGet(prvTgfDevNum, &(prvTgfRestoreCfg.phaFwImageIdGet));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaFwImageIdGet FAILED with rc = [%d]", rc);

    rc = cpssDxChPhaInit(prvTgfDevNum, GT_FALSE, CPSS_DXCH_PHA_FW_IMAGE_ID_01_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaInit FAILED for phaFwImageId = [%d], rc = [%d]", CPSS_DXCH_PHA_FW_IMAGE_ID_01_E, rc);

    cpssOsMemSet(&extInfo,   0, sizeof(extInfo));     /* No template */
    for (ii = 0; ii < 8; ii++)
    {
        extInfo.srv6SrcNode.srcAddr.arIP[ii * 2]     = (GT_U8)(prvTgfPhaTemplateIPv6[ii] >> 8);
        extInfo.srv6SrcNode.srcAddr.arIP[ii * 2 + 1] = (GT_U8) prvTgfPhaTemplateIPv6[ii];
    }

    /* AUTODOC: Assign PHA thread "SRv6 BE Tunnel" to egress port */
    prvTgSrv6PhaThreadConfigSet(prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
            PRV_TGF_PHA_THREAD_TYPE_SRV6_SRC_NODE_1_CONTAINER_E,
            CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_1_CONTAINER_E,
            &extInfo);
}

/**
* internal tgfSrv6GSidTunnelOneSrhContainerConfigSet function
* @endinternal
*
* @brief   SRV6 G-SID Tunnel One SRH Container use case configurations
*/
GT_VOID tgfSrv6GSidTunnelOneSrhContainerConfigSet
(
    GT_VOID
)
{

    /* TTI/PCL Engine Configurations */
    prvTgSrv6GSidTunnelTtiPclConfigSet();

    /* Add TS IPv6 at egress Port */
    prvTgfSrv6GSidTunnelEgressConfigSet();

    /* PHA - Attach PHA Thread "SRv6 G-SID Tunnel" at Egress port */
    prvTgfSrv6GSidTunnelPhaConfigSet();
}

/**
* internal tgfSrv6GSidOneSrhContainerTunnelVerification function
* @endinternal
*
* @brief   SRv6 G-SID Tunnel One SRH Container use case verification
*/
GT_VOID tgfSrv6GSidOneSrhContainerTunnelVerification
(
    GT_VOID
)
{
    GT_STATUS                           rc = GT_OK;
    GT_U8                               packetBuf[PRV_TGF_PACKET_SIZE_CNS] = {0};
    GT_U32                              packetLen = PRV_TGF_PACKET_SIZE_CNS;
    GT_U32                              packetActualLength = 0;
    GT_U8                               queue = 0;
    GT_U8                               dev = 0, ii;
    TGF_NET_DSA_STC                     rxParam;
    GT_BOOL                             getFirst = GT_TRUE;
    GT_U32                              pktOffset1, pktOffset = TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS;
    GT_U32                              appId;

    rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&egrPortInterface,
            TGF_PACKET_TYPE_CAPTURE_E,
            getFirst, GT_TRUE, packetBuf,
            &packetLen, &packetActualLength,
            &dev, &queue, &rxParam);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet: egrPortInterface %d", prvTgfDevNum);

    /* Add Tag length in case of tagged packet */
    pktOffset = TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS;

    /* IPv6 Payload Verification */
    pktOffset += 4;
    UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_PACKET_LEN_CNS + 0x18,
                                 (GT_U16)(packetBuf[pktOffset + 1] | (packetBuf[pktOffset] << 8)),
            "IPv6 Payload is not matching ");

    /* IPv6 Next Header Verification - SRH extension so 43 valid */
    pktOffset += 2;             /* Point to Next Header index */
    UTF_VERIFY_EQUAL0_STRING_MAC(43, packetBuf[pktOffset],
            "IPv6 Next Header is not matching ");

    /* AUTODOC: Tunnel SIP and DIP verification
     * DIP - Configured DIP via prvTgf1stTunnelStartDstIPv6 changes as per PHA
     * SIP - Must be default value
     */
    pktOffset += 2;             /* Point to SIP index */
    pktOffset1 = pktOffset+16;  /* Point to DIP index */
    for (ii = 0; ii < 8; ii++)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfPhaTemplateIPv6[ii],
                (GT_U16)(packetBuf[pktOffset + 1] | (packetBuf[pktOffset] << 8)),
                "SIP is not matching at index[%d]", ii);
        if(ii <= 3)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgf1stTunnelStartDstIPv6[ii],
                    (GT_U16)(packetBuf[pktOffset1+1] | (packetBuf[pktOffset1] << 8)),
                    "DIP Locator is not matching at index[%d]", ii);
            pktOffset1+=2;
        }
        pktOffset+=2;
    }

    ii = 4;
    UTF_VERIFY_EQUAL1_STRING_MAC(PRV_TGF_GSID_NODE_INDEX_CNS,
            (GT_U16)(packetBuf[pktOffset1+1] | (packetBuf[pktOffset1] << 8)),
            "DIP Node ID is not matching at index[%d]", ii);

    ii++;
    pktOffset1+=2;
    UTF_VERIFY_EQUAL1_STRING_MAC(PRV_TGF_GSID_FUNCTION_INDEX_CNS,
            (GT_U16)(packetBuf[pktOffset1+1] | (packetBuf[pktOffset1] << 8)),
            "DIP Function ID is not matching at index[%d]", ii);

    ii++;
    pktOffset1+=2;
    UTF_VERIFY_EQUAL1_STRING_MAC(PRV_TGF_SOURCE_INDEX_CNS,
            (GT_U16)(packetBuf[pktOffset1+1] | (packetBuf[pktOffset1] << 8)),
            "DIP SourceId is not matching at index[%d]", ii);

    ii++;
    pktOffset1+=2;
    appId = (PRV_TGF_APP_INDEX_CNS & PRV_TGF_APP_ID_MASK) << PRV_TGF_RESERVE_ID_BIT_CNS;
    UTF_VERIFY_EQUAL1_STRING_MAC(appId,
            (GT_U16)(packetBuf[pktOffset1+1] | (packetBuf[pktOffset1] << 8)),
            "DIP AppId is not matching at index[%d]", ii);

    UTF_VERIFY_EQUAL1_STRING_MAC(0x0,
            packetBuf[pktOffset1 + 1] & 0xF,
            "DIP Reserved bit is not matching at index[%d]", ii);

    pktOffset1+=2;
    /*** Next_Hdr +++ Hdr_Ext_Len +++ Routing_Type +++ Segments_Left ***/
    /* Next_Hdr */
    UTF_VERIFY_EQUAL0_STRING_MAC(43,
            (GT_U8)(packetBuf[pktOffset1]),
            "Next_Hdr is not matching with ");
    pktOffset1+=1;

    /* Hdr_Ext_Len */
    UTF_VERIFY_EQUAL0_STRING_MAC(TGF_SEGMENT_LEN_CNS/TGF_HDR_LEN_EXT_UNIT_CNS,
            (GT_U8)(packetBuf[pktOffset1]),
            "Hdr_Len_Ext is not matching with ");
    pktOffset1+=1;

    /* Routing_Type */
    UTF_VERIFY_EQUAL0_STRING_MAC(4, /* Constant 4 */
            (GT_U8)(packetBuf[pktOffset1]),
            "Routing_Type is not matching with constant value");
    pktOffset1+=1;

    /* Segments_Left */
    UTF_VERIFY_EQUAL0_STRING_MAC(1, /* Num of segments left */
            (GT_U8)(packetBuf[pktOffset1]),
            "Segments_Left is not matching with constant value");
    pktOffset1+=1;

    /* AUTODOC: SRH Header last 4 Bytes should be 0 */
    for (ii = 0; ii < 4; ii++)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(0, (GT_U8)packetBuf[pktOffset1],
                "SRH base header is not matching at index[%d]", ii);
        pktOffset1+=1;
    }

    for (ii = 0; ii < 8; ii++)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(prvTgf1stTunnelStartSrcIPv6[ii],
                (GT_U16)(packetBuf[pktOffset1+1] | (packetBuf[pktOffset1] << 8)),
                "SRH container is not matching at index[%d]", ii);
        pktOffset1+=2;
    }
}

/**
* @internal tgfSrv6GSidTunnelOneSrhContainerTest function
* @endinternal
*
* @brief   SRv6 G-SID Tunnel One SRH Container use case test.
*/
GT_VOID tgfSrv6GSidTunnelOneSrhContainerTest
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIter;

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum,
                    prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "prvTgfResetCountersEth: %d, %d",
                    prvTgfDevNum,
                    prvTgfPortsArray[portIter]);
    }

    packetInfo.numOfParts =  sizeof(prvTgfIpv6PacketArray) / sizeof(prvTgfIpv6PacketArray[0]);
    packetInfo.partsArray = prvTgfIpv6PacketArray;
    packetInfo.totalLen   = TGF_PACKET_AUTO_CALC_LENGTH_CNS;

    /* setup receive portInterface for capturing */
    egrPortInterface.type               = CPSS_INTERFACE_PORT_E;
    egrPortInterface.devPort.hwDevNum   = prvTgfDevNum;
    egrPortInterface.devPort.portNum    = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    /* AUTODOC: GENERATE TRAFFIC & Verify at ePort#3 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
                        &egrPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgftrafficgeneratorporttxethcaptureset port - %d",
                                        egrPortInterface.devPort.portNum);

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, 1, 0, NULL);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfSetTxSetupEth");

    /* Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");

    /* disable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &egrPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet port - %d",
            egrPortInterface.devPort.portNum);

    /* AUTODOC: Verification Started */
    tgfSrv6GSidOneSrhContainerTunnelVerification();
}

/**
* @internal tgfSrv6GSidTunnelOneSrhContainerConfigRestore function
* @endinternal
*
* @brief   SRv6 G-SID Tunnel One SRH Container use case configurations restore.
* @note    1. Restore PHA Configuration
*          2. Restore Bridge Configuration
*          3. Restore Base Configuration
*
*/
GT_VOID tgfSrv6GSidTunnelOneSrhContainerConfigRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: Restore TTI/PCL Configuration */
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_ETH_E, prvTgfRestoreCfg.origMacMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet");

    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_ETH_E, prvTgfRestoreCfg.lookupEnableGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: invalidate TTI rules  */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");

    /* AUTODOC: invalidate PCL rules  */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, IPCL_MATCH_INDEX_0_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d",
                                     CPSS_PCL_RULE_SIZE_STD_E,
                                     IPCL_MATCH_INDEX_0_CNS);

    /* AUTODOC: Restore PHA Configuration */
    rc = cpssDxChPhaInit(prvTgfDevNum, GT_FALSE, prvTgfRestoreCfg.phaFwImageIdGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaInit FAILED for phaFwImageId = [%d], rc = [%d]", prvTgfRestoreCfg.phaFwImageIdGet, rc);

    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                    GT_FALSE, PRV_TGF_PHA_THREAD_TYPE_SRV6_SRC_NODE_1_CONTAINER_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaPortThreadIdSet FAILED for SRV6_BEST_EFFORT, rc = [%d]", rc);

    /* AUTODOC: Restore ePort attributes */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                        prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                        &(prvTgfRestoreCfg.egressInfo));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortEgressPortInfoSet");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");
}

/**
* @internal prvTgfSrv6TwoThreeSrhContainersHaConfigSet function
* @endinternal
*
* @brief   TS config for "SRv6 Two/Three STH Containers use case".
*/
static GT_VOID prvTgfSrv6TwoThreeSrhContainersHaConfigSet
(
    CPSS_DXCH_PHA_THREAD_TYPE_ENT       secondPassThread
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC        egress1Info;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC        egressInfo;
    PRV_TGF_TUNNEL_START_ENTRY_UNT          tunnelEntry;
    PRV_TGF_TUNNEL_START_ENTRY_UNT          tunnelEntryGet;
    CPSS_TUNNEL_TYPE_ENT                    tunnelTypeGet;
    GT_U32                                  ii;
    CPSS_INTERFACE_INFO_STC                 physicalInfo;

    /* AUTODOC: Save loopback port cscd type */
    rc = cpssDxChCscdPortTypeGet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_LOOPBACK_PORT_IDX_CNS],
                                 CPSS_PORT_DIRECTION_BOTH_E,
                                 &prvTgfRestoreCfg.cscdPortType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCscdPortTypeGet");

    /* AUTODOC: Set loopback port as cascade to add 16 bytes of eDSA tag */
    rc = cpssDxChCscdPortTypeSet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_LOOPBACK_PORT_IDX_CNS],
                                 CPSS_PORT_DIRECTION_BOTH_E,
                                 CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCscdPortTypeSet");

    /***********************************************/
    /* ePort Mapping : Egress_port = "1+egress_port" */
    /***********************************************/
    /* AUTODOC: save ePort mapping configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                                           PRV_TGF_SECOND_PASS_EGR_EPORT_CNS,
                                                           &(prvTgfRestoreCfg.physicalInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableGet);

    /* AUTODOC: set ePort mapping configuration */
    cpssOsMemSet(&physicalInfo, 0, sizeof(physicalInfo));
    physicalInfo.type               = CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.hwDevNum   = prvTgfDevNum;
    physicalInfo.devPort.portNum    = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    /* AUTODOC: map Egress to ePort (1+Egress) */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           PRV_TGF_SECOND_PASS_EGR_EPORT_CNS,
                                                           &physicalInfo);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableSet);

    /***********************************************/
    /* Egress Port TS = Generic IPV6 */
    /***********************************************/
    /* AUTODOC: Egress port TS SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Egress tunnel ipv6 Configuration =======\n");

    /* AUTODOC: Save ePort attributes configuration */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                                                       prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                                       &(prvTgfRestoreCfg.egressInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoGet);

    /* AUTODOC: Set ePort attributes configuration */
    cpssOsMemSet(&egressInfo, 0, sizeof(egressInfo));
    egressInfo                        = prvTgfRestoreCfg.egressInfo;
    egressInfo.tunnelStart            = GT_TRUE;
    egressInfo.tunnelStartPtr         = PRV_TGF_TUNNEL_START_INDEX_CNS;
    egressInfo.tsPassengerPacketType  = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;

    /* AUTODOC: config ePort#2 attributes: */
    /* AUTODOC:   Tunnel Start=ENABLE, tunnelStartPtr=8 */
    /* AUTODOC:   tsPassenger=ETHERNET */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                        prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                        &egressInfo);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

    /* AUTODOC: Set Generic IPv6 tunnel start entry */
    PRV_UTF_LOG0_MAC("======= Setting Tunnel Start Configuration =======\n");
    cpssOsMemSet(&tunnelEntry.ipv6Cfg,0,sizeof(tunnelEntry.ipv6Cfg));
    /*** Set a tunnel start entry ***/
    tunnelEntry.ipv6Cfg.tagEnable        = GT_TRUE;
    tunnelEntry.ipv6Cfg.vlanId           = PRV_TGF_DEFAULT_VLANID_CNS;
    tunnelEntry.ipv6Cfg.upMarkMode       = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tunnelEntry.ipv6Cfg.dscpMarkMode     = PRV_TGF_TUNNEL_START_MARK_FROM_ENTRY_E;
    tunnelEntry.ipv6Cfg.ttl              = 33;
    tunnelEntry.ipv6Cfg.retainCrc        = GT_FALSE;
    tunnelEntry.ipv6Cfg.ipHeaderProtocol = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_GENERIC_E;
    tunnelEntry.ipv6Cfg.ipProtocol       = 43;  /* Ignored */
    tunnelEntry.ipv6Cfg.udpSrcPort       = udpSrcPort;
    tunnelEntry.ipv6Cfg.udpDstPort       = udpDstPort;

    /* tunnel next hop MAC DA, IP DA, IP SA */
    cpssOsMemCpy(tunnelEntry.ipv6Cfg.macDa.arEther, prvTgfOriginalPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    /* 80b Locator */
    for (ii = 0; ii < 4; ii++)
    {
        tunnelEntry.ipv6Cfg.destIp.arIP[ii * 2]     = (GT_U8)(prvTgf1stTunnelStartDstIPv6[ii] >> 8 & 0xFF);
        tunnelEntry.ipv6Cfg.destIp.arIP[ii * 2 + 1] = (GT_U8) prvTgf1stTunnelStartDstIPv6[ii] & 0xFF;
    }
    for (ii = 0; ii < 8; ii++)
    {
        tunnelEntry.ipv6Cfg.srcIp.arIP[ii * 2]     = (GT_U8)(prvTgf1stTunnelStartSrcIPv6[ii] >> 8 & 0xFF);
        tunnelEntry.ipv6Cfg.srcIp.arIP[ii * 2 + 1] = (GT_U8) prvTgf1stTunnelStartSrcIPv6[ii] & 0xFF;
    }

    tunnelEntry.ipv6Cfg.destIp.arIP[8]  = (GT_U8) ((PRV_TGF_GSID_NODE_INDEX_CNS >> 8) & 0xFF);
    tunnelEntry.ipv6Cfg.destIp.arIP[9]  = (GT_U8) PRV_TGF_GSID_NODE_INDEX_CNS & 0xFF;
    tunnelEntry.ipv6Cfg.destIp.arIP[10] = (GT_U8)((PRV_TGF_GSID_FUNCTION_INDEX_CNS >> 8) & 0xFF);
    tunnelEntry.ipv6Cfg.destIp.arIP[11] = (GT_U8) PRV_TGF_GSID_FUNCTION_INDEX_CNS & 0xFF;

    /* FirstBit and LastBit of Source-ID to represent SGT-ID */
    tunnelEntry.ipv6Cfg.destIp.arIP[12] = PRV_TGF_SOURCE_ID_LAST_BIT;
    tunnelEntry.ipv6Cfg.destIp.arIP[13] = PRV_TGF_SOURCE_ID_FIRST_BIT;

    /* AUTODOC: add TS entry 8 with: */
    /* AUTODOC:   tunnelType = GENERIC_IPV6 */
    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_INDEX_CNS, CPSS_TUNNEL_GENERIC_IPV6_E, &tunnelEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntrySet: %d", prvTgfDevNum);

    /* AUTODOC: get and check added TS Entry */
    rc = prvTgfTunnelStartEntryGet(prvTgfDevNum, PRV_TGF_TUNNEL_START_INDEX_CNS, &tunnelTypeGet, &tunnelEntryGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntryGet: %d", prvTgfDevNum);

    /* Check vlanId field */
    UTF_VERIFY_EQUAL1_STRING_MAC(tunnelEntryGet.ipv6Cfg.vlanId, tunnelEntry.ipv6Cfg.vlanId,
            "Vlan Id mismatch with configured value for device %d", prvTgfDevNum);

    /* Check srcIp field */
    rc = cpssOsMemCmp(&tunnelEntryGet.ipv6Cfg.srcIp, &tunnelEntry.ipv6Cfg.srcIp, sizeof(GT_IPV6ADDR)) == 0 ? GT_OK : GT_FALSE;
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tunnelEntry.ipv6Cfg.srcIp did not match");

    /***********************************************/
    /* Egress Port(ePort+1) TS = Generic TS */
    /***********************************************/
    /* AUTODOC: Egress ePort+1 TS SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Egress tunnel ipv6 Configuration =======\n");

    /* AUTODOC: Save ePort attributes configuration */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                                                       PRV_TGF_SECOND_PASS_EGR_EPORT_CNS,
                                                       &(prvTgfRestoreCfg.egress1Info));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoGet);

    /* AUTODOC: Set ePort attributes configuration */
    cpssOsMemSet(&egress1Info,0,sizeof(egress1Info));
    egress1Info                        = prvTgfRestoreCfg.egress1Info;
    egress1Info.tunnelStart            = GT_TRUE;
    egress1Info.tunnelStartPtr         = PRV_TGF_TUNNEL_START_INDEX_CNS+8;
    egress1Info.tsPassengerPacketType  = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;

    /* AUTODOC: config ePort#2 attributes: */
    /* AUTODOC:   Tunnel Start=ENABLE, tunnelStartPtr=8+8 */
    /* AUTODOC:   tsPassenger=ETHERNET */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                        PRV_TGF_SECOND_PASS_EGR_EPORT_CNS,
                                        &egress1Info);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

    /* AUTODOC: Set Tunnel Start entry 8+8 with: */
    /* AUTODOC: genericType = long, tagEnable = TRUE, vlanId=1 */
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));
    tunnelEntry.genCfg.tagEnable            = GT_TRUE;
    tunnelEntry.genCfg.vlanId               = PRV_TGF_DEFAULT_VLANID_CNS;
    tunnelEntry.genCfg.vlanId               = (secondPassThread == CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_2_CONTAINERS_E) ? 2: 3;
    tunnelEntry.genCfg.genericType          = PRV_TGF_TUNNEL_START_GENERIC_LONG_TYPE_E;
    tunnelEntry.genCfg.etherType            = (secondPassThread == CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_2_CONTAINERS_E) ?
                                               TGF_TWO_SRH_CONTAINERS_LEN_CNS/TGF_HDR_LEN_EXT_UNIT_CNS :
                                               TGF_THREE_SRH_CONTAINERS_LEN_CNS/TGF_HDR_LEN_EXT_UNIT_CNS;
    tunnelEntry.genCfg.etherType            = tunnelEntry.genCfg.etherType << 8 | 4;

    cpssOsMemCpy(tunnelEntry.genCfg.data, prvTgfTunnelStart, sizeof(prvTgfTunnelStart));
    cpssOsMemCpy(tunnelEntry.genCfg.macDa.arEther, prvTgf1stTunnelL2Part.daMac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_INDEX_CNS+8, CPSS_TUNNEL_GENERIC_E, &tunnelEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartEntrySet");

    /* AUTODOC: set EPORT tag state mode for egress port */
    rc = prvTgfBrgVlanEgressPortTagStateModeGet(prvTgfDevNum,
                                                PRV_TGF_SECOND_PASS_EGR_EPORT_CNS,
                                                &(prvTgfRestoreCfg.vlanEgressPortTagStateMode));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateModeGet);

    rc = prvTgfBrgVlanEgressPortTagStateModeSet(prvTgfDevNum,
                                                PRV_TGF_SECOND_PASS_EGR_EPORT_CNS,
                                                PRV_TGF_BRG_VLAN_EPORT_TAG_STATE_EPORT_MODE_E);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateModeSet);

    /* AUTODOC: set TAG0_CMD port tag state for egress port  */
    rc = prvTgfBrgVlanEgressPortTagStateGet(prvTgfDevNum,
                                            PRV_TGF_SECOND_PASS_EGR_EPORT_CNS,
                                            &(prvTgfRestoreCfg.vlanEgressPortTagState));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateGet);

    rc = prvTgfBrgVlanEgressPortTagStateSet(prvTgfDevNum,
                                            PRV_TGF_SECOND_PASS_EGR_EPORT_CNS,
                                            pktType[0]);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgVlanEgressPortTagStateSet);
}

/**
* internal prvTgfSrv6TwoThreeSrhContainersPhaConfigSet function
* @endinternal
*
* @brief   PHA config for SRv6 Two/Three SRH Container use case.
*/
static GT_VOID prvTgfSrv6TwoThreeSrhContainersPhaConfigSet
(
    CPSS_DXCH_PHA_THREAD_TYPE_ENT       secondPassThread
)
{
    CPSS_DXCH_PHA_THREAD_INFO_UNT           extInfo;
    GT_U32                                  ii, phaThreadId;
    GT_STATUS                               rc = GT_OK;

    /* GM does not support PHA */
    if(GT_TRUE == prvUtfIsGmCompilation() || (prvTgfSrv6OneSegmentSkipPha == GT_TRUE))
        return;

    /* AUTODOC: Assign PHA firmware image ID 01 */
    rc = cpssDxChPhaFwImageIdGet(prvTgfDevNum, &(prvTgfRestoreCfg.phaFwImageIdGet));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaFwImageIdGet FAILED with rc = [%d]", rc);

    rc = cpssDxChPhaInit(prvTgfDevNum, GT_FALSE, CPSS_DXCH_PHA_FW_IMAGE_ID_01_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaInit FAILED for phaFwImageId = [%d], rc = [%d]", CPSS_DXCH_PHA_FW_IMAGE_ID_01_E, rc);

    /* AUTODOC: Assign PHA thread "SRv6 SECOND_PASS_2_SEGMENTS" to egress "ePort+1" */
    cpssOsMemSet(&extInfo, 0, sizeof(extInfo));
    phaThreadId = (secondPassThread == CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_2_CONTAINERS_E) ?
                                       PRV_TGF_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_2_CONTAINERS_E :
                                       PRV_TGF_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_3_CONTAINERS_E;

    prvTgSrv6PhaThreadConfigSet(PRV_TGF_SECOND_PASS_EGR_EPORT_CNS,
                                phaThreadId,
                                secondPassThread,
                                &extInfo);

    /* AUTODOC: Assign PHA thread "SRv6 FIRST_PASS_2_3_SEGMENTS" to egress Port */
    for (ii = 0; ii < 8; ii++)
    {
        extInfo.srv6SrcNode.srcAddr.arIP[ii * 2]     = (GT_U8)(prvTgfPhaTemplateIPv6[ii] >> 8);
        extInfo.srv6SrcNode.srcAddr.arIP[ii * 2 + 1] = (GT_U8) prvTgfPhaTemplateIPv6[ii];
    }

    prvTgSrv6PhaThreadConfigSet(prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                PRV_TGF_PHA_THREAD_TYPE_SRV6_SRC_NODE_FIRST_PASS_2_3_CONTAINERS_E,
                                CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_FIRST_PASS_2_3_CONTAINERS_E,
                                &extInfo);
}

/**
* internal tgfSrv6GSidTunnelTwoSrhContainerConfigSet function
* @endinternal
*
* @brief   SRV6 G-SID Tunnel Two SRH Container use case configurations
*/
GT_VOID tgfSrv6GSidTunnelTwoSrhContainerConfigSet
(
    GT_VOID
)
{
    /* TTI/PCL Engine Configurations */
    prvTgSrv6GSidTunnelTtiPclConfigSet();

    /* Loop Back configuration */
    prvTgfSrv6TwoThreeSegmentLoopBackConfigSet();

    /* Add TS IPv6 at egress ePort */
    prvTgfSrv6TwoThreeSrhContainersHaConfigSet(CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_2_CONTAINERS_E);

    /* PHA - Attach PHA Thread "2 SRH Containers" at Egress port */
    prvTgfSrv6TwoThreeSrhContainersPhaConfigSet(CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_2_CONTAINERS_E);
}

/**
* @internal tgfSrv6GSidTunnelTwoSrhContainerTest function
* @endinternal
*
* @brief   SRv6 G-SID Tunnel Two SRH Container use case test.
*/
GT_VOID tgfSrv6GSidTunnelTwoSrhContainerTest
(
    CPSS_DXCH_PHA_THREAD_TYPE_ENT           threadType
)
{
    tgfSrv6TwoThreeSrhContainersTest(threadType);
}

/**
* @internal tgfSrv6GSidTunnelTwoSrhContainerConfigRestore function
* @endinternal
*
* @brief  SRv6 G-SID Tunnel Two SRH Container use case configurations restore.
*/
GT_VOID tgfSrv6GSidTunnelTwoSrhContainerConfigRestore
(
    GT_VOID
)
{
    tgfSrv6TwoThreeSrhContainersConfigRestore(CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_2_CONTAINERS_E);
}

/**
* internal tgfSrv6GSidTunnelThreeSrhContainerConfigSet function
* @endinternal
*
* @brief   SRV6 G-SID Tunnel Three SRH Container use case configurations
*/
GT_VOID tgfSrv6GSidTunnelThreeSrhContainerConfigSet
(
    GT_VOID
)
{
    /* TTI/PCL Engine Configurations */
    prvTgSrv6GSidTunnelTtiPclConfigSet();

    /* Loop Back configuration */
    prvTgfSrv6TwoThreeSegmentLoopBackConfigSet();

    /* Add TS IPv6 at egress ePort */
    prvTgfSrv6TwoThreeSrhContainersHaConfigSet(CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_3_CONTAINERS_E);

    /* PHA - Attach PHA Thread "2 SRH Containers" at Egress port */
    prvTgfSrv6TwoThreeSrhContainersPhaConfigSet(CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_3_CONTAINERS_E);
}

/**
* @internal tgfSrv6GSidTunnelThreeSrhContainerTest function
* @endinternal
*
* @brief   SRv6 G-SID Tunnel Three SRH Container use case test.
*/
GT_VOID tgfSrv6GSidTunnelThreeSrhContainerTest
(
    CPSS_DXCH_PHA_THREAD_TYPE_ENT           threadType
)
{
    tgfSrv6TwoThreeSrhContainersTest(threadType);
}

/**
* @internal tgfSrv6GSidTunnelThreeSrhContainerConfigRestore function
* @endinternal
*
* @brief  SRv6 G-SID Tunnel Three SRH Container use case configurations restore.
*/
GT_VOID tgfSrv6GSidTunnelThreeSrhContainerConfigRestore
(
    GT_VOID
)
{
    tgfSrv6TwoThreeSrhContainersConfigRestore(CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_3_CONTAINERS_E);
}

/* Configure TTI rules for each of the SL,SI pairs as shown in the PCL
 * configuration for function prvTgSrv6GSidTunnelCoc32PclConfigSet
 */
/**
* @internal prvTgSrv6GSidTunnelCoc32TtiConfigSet function
* @endinternal
*
* @brief  SRv6 G-SID Tunnel CoC32 G-SID end node use case TTI configurations.
*/
static GT_VOID prvTgSrv6GSidTunnelCoc32TtiConfigSet
(
    GT_VOID
)
{

    GT_STATUS rc = GT_OK;
    PRV_TGF_TTI_ACTION_2_STC            ttiAction;
    PRV_TGF_TTI_RULE_UNT                ttiMask;
    PRV_TGF_TTI_RULE_UNT                ttiPattern;
    PRV_TGF_TTI_MAC_VLAN_STC            macToMePattern;
    PRV_TGF_TTI_MAC_VLAN_STC            macToMeMask;
    GT_U32                              ii = 0;
    GT_U32                              ttiRuleIndex, jj;

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* enable the TTI lookup for PRV_TGF_TTI_KEY_ETH_E */
    rc = prvTgfTtiPortLookupEnableGet(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_UDB_IPV6_TCP_E, &prvTgfRestoreCfg.lookupEnableGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableGet");

    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_UDB_IPV6_TCP_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    rc = prvTgfTtiPacketTypeKeySizeGet(prvTgfDevNum,
                            PRV_TGF_TTI_KEY_UDB_IPV6_TCP_E,
                            &prvTgfRestoreCfg.keySizeGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);

    rc = prvTgfTtiPacketTypeKeySizeSet(prvTgfDevNum,
                            PRV_TGF_TTI_KEY_UDB_IPV6_TCP_E,
                            PRV_TGF_TTI_KEY_SIZE_20_B_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);

    PRV_UTF_LOG0_MAC("======= Setting UDB Configuration =======\n");

    /* AUTODOC: set MAC to ME table */
    cpssOsMemCpy((GT_VOID*)macToMePattern.mac.arEther,
                 (GT_VOID*)prvTgfOriginalPacketL2Part.daMac,
                 sizeof(macToMePattern.mac.arEther));
    macToMePattern.vlanId = 1;

    cpssOsMemSet((GT_VOID*)macToMeMask.mac.arEther, 0xFF,
                 sizeof(macToMeMask.mac.arEther));
    macToMeMask.vlanId = 0xFFF;

    /* AUTODOC: config MAC to ME table entry index 1: */
    /* AUTODOC:   DA=00:00:00:00:34:02, VID=5 */
    rc = prvTgfTtiMacToMeSet(PRV_TGF_MAC_TO_ME_INDEX_CNS, &macToMePattern, &macToMeMask);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacToMeSet");

     /* UDB for MAC2ME */
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV6_TCP_E, 0,
                                     CPSS_DXCH_TTI_OFFSET_METADATA_E, 22);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,
                    PRV_TGF_TTI_KEY_UDB_IPV6_TCP_E, 1,
                    PRV_TGF_TTI_OFFSET_L3_MINUS_2_E, 0 /* start of L3 EtherType */);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,
                    PRV_TGF_TTI_KEY_UDB_IPV6_TCP_E, 2,
                    PRV_TGF_TTI_OFFSET_L3_MINUS_2_E, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    for(ii = 3; ii < 15 ; ii++)
    {
        rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,
                        PRV_TGF_TTI_KEY_UDB_IPV6_TCP_E, ii,
                        PRV_TGF_TTI_OFFSET_L3_MINUS_2_E,(GT_U8) (ii-3) + 26 /* start of L3 DIPv6 */);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    }

    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,
                    PRV_TGF_TTI_KEY_UDB_IPV6_TCP_E, 15,
                    PRV_TGF_TTI_OFFSET_L3_MINUS_2_E, 8 /* DIPv6 Next Header */);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,
                    PRV_TGF_TTI_KEY_UDB_IPV6_TCP_E, 16,
                    PRV_TGF_TTI_OFFSET_L3_MINUS_2_E, 44 /* SRH Routing Type */);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,
                    PRV_TGF_TTI_KEY_UDB_IPV6_TCP_E, 17,
                    PRV_TGF_TTI_OFFSET_L3_MINUS_2_E, 45 /* SRH Segment Left */);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,
                    PRV_TGF_TTI_KEY_UDB_IPV6_TCP_E, 18,
                    PRV_TGF_TTI_OFFSET_L3_MINUS_2_E, 41 /* DIPv6 Segments Identifier */);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    jj = 0;
    for(ttiRuleIndex = 0; ; ttiRuleIndex++)
    {
        if(ttiRuleIndex % 2 == 0 && ttiRuleIndex % 12 != 0)
        {
            cpssOsMemSet(&(ttiAction),  0, sizeof(ttiAction));
            cpssOsMemSet(&(ttiMask),    0, sizeof(ttiMask));
            cpssOsMemSet(&(ttiPattern), 0, sizeof(ttiPattern));

            ttiPattern.udbArray.udb[0] = 0x80;
            ttiMask.udbArray.udb[0]    = 0x80; /* MAC2ME[7] */

            ttiPattern.udbArray.udb[1] = 0x86;
            ttiMask.udbArray.udb[1]    = 0xFF; /* IPv6 Ether Type[7:0] */
            ttiPattern.udbArray.udb[2] = 0xDD;
            ttiMask.udbArray.udb[2]    = 0xFF; /* IPv6 Ether Type[15:8] */

            for(ii = 1; ii < 4 ; ii++)
            {
                ttiPattern.udbArray.udb[ii*2+1]   = 0x00;
                ttiPattern.udbArray.udb[ii*2+2]   = 0x00;
                ttiMask.udbArray.udb[ii*2+1]      = 0xff;
                ttiMask.udbArray.udb[ii*2+2]      = 0xff;
            }

            ttiPattern.udbArray.udb[9]    = 0x00;
            ttiMask.udbArray.udb[9]       = 0xff;
            ttiPattern.udbArray.udb[10]   = 0x40;
            ttiMask.udbArray.udb[10]      = 0xff;

            for(ii = 0; ii < 2 ; ii++)
            {
                ttiPattern.udbArray.udb[11 + ii*2]       = (jj == 0) ? prvTgfGsidCoc32PacketIpv6Part.dstAddr[4 + ii] >> 8 & 0xFF
                                                                     : prvTgfCoc32Gsid[(11 - ii/2) * 4 + (2 * ii)];
                ttiMask.udbArray.udb[11 + ii*2]          = 0xff;
                ttiPattern.udbArray.udb[11 + ii*2 + 1]   = (jj == 0) ? prvTgfGsidCoc32PacketIpv6Part.dstAddr[4 + ii] & 0xFF
                                                                     : prvTgfCoc32Gsid[(11 - ii/2) * 4 + (2 * ii) + 1];
                ttiMask.udbArray.udb[11 + ii*2 + 1]      = 0xff;
            }

            ttiPattern.udbArray.udb[15] = 43;
            ttiMask.udbArray.udb[15]    = 0xFF; /* IPv6 Next Header */

            ttiPattern.udbArray.udb[16] = 4;
            ttiMask.udbArray.udb[16]    = 0xFF; /* SRH Routing Type */

            ttiPattern.udbArray.udb[17] = (jj == 0) ? 3 : (jj <=8) ? (2 - jj/5) : 0;
            ttiMask.udbArray.udb[17]    = 0xFF; /* SRH Segments Left */

            ttiPattern.udbArray.udb[18] = (jj == 0) ? 0 : (jj == 1 || jj == 5 || jj == 9) ? 3 : (4 - jj%5);
            ttiMask.udbArray.udb[18]    = 0xFF; /* SRv6 DIP SI */

            /* TTI actions - FORWARD packet */
            ttiAction.command                                        = CPSS_PACKET_CMD_FORWARD_E;
            ttiAction.pcl0OverrideConfigIndex                        = PRV_TGF_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
            ttiAction.iPclConfigIndex                                = PRV_TGF_PCL_CONFIG_INDEX_CNS + jj;
            ttiAction.iPclUdbConfigTableEnable                       = GT_TRUE;
            ttiAction.iPclUdbConfigTableIndex                        = (jj == 0 || jj == 1)
                                                                           ?  PRV_TGF_PCL_PACKET_TYPE_UDE5_E
                                                                           :  (jj == 10 || jj == 11)
                                                                           ?  PRV_TGF_PCL_PACKET_TYPE_UDE_E
                                                                           :  (PRV_TGF_PCL_PACKET_TYPE_UDE_4_E - ((jj / 2) - 1) );

            /* AUTODOC: set TTI rule */
            rc = prvTgfTtiRuleUdbSet((ttiRuleIndex),PRV_TGF_TTI_RULE_UDB_20_E,
                                     &(ttiPattern), &(ttiMask), &(ttiAction));
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleUdbSet");

            /* AUTODOC: validate TTI rules */
            rc = prvTgfTtiRuleValidStatusSet(ttiRuleIndex, GT_TRUE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d, %d", prvTgfDevNum, GT_TRUE);

            jj++;

            if(jj == PRV_TGF_TTI_RULE_INDEX_CNS)
            {
                break;
            }
        }
    }
}

/**
* @internal prvTgfIpv6UcPbrRoutingPriorityLeafEntrySet function
* @endinternal
*
* @brief   Set LPM leaf entry
*
* @param[in] priority                        - LPM leaf entry priority
* @param[in] prvTgfPbrLeafIndex              - LPM leaf index
* @param[in] prvTgfRouteEntryBaseIndex       - LPM route entry base index
*
*/
GT_STATUS prvTgfIpv6UcPbrRoutingPriorityLeafEntrySet
(
    CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_TYPE_ENT priority,
    GT_U32                                     prvTgfPbrLeafIndex,
    GT_U32                                     prvTgfRouteEntryBaseIndex
)
{
    GT_STATUS rc;
    PRV_TGF_LPM_LEAF_ENTRY_STC              leafEntry;

    cpssOsMemSet(&leafEntry, 0, sizeof(PRV_TGF_LPM_LEAF_ENTRY_STC));

    /* write a leaf entry to the PBR area in RAM */
    leafEntry.entryType                = PRV_TGF_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
    leafEntry.index                    = prvTgfRouteEntryBaseIndex;
    leafEntry.ucRPFCheckEnable         = GT_FALSE;
    leafEntry.sipSaCheckMismatchEnable = GT_FALSE;
    leafEntry.ipv6MCGroupScopeLevel    = 0;
    leafEntry.priority                 = priority;

    /* AUTODOC: Read a PBR leaf */
    rc = prvTgfLpmLeafEntryRead(prvTgfDevNum, prvTgfPbrLeafIndex, &leafEntryOrig);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfLpmLeafEntryRead");

    /* AUTODOC: Write a PBR leaf */
    rc = prvTgfLpmLeafEntryWrite(prvTgfDevNum, prvTgfPbrLeafIndex, &leafEntry);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfLpmLeafEntryWrite");

    return rc;
}

/**
* @internal prvTgfIpv6UcPbrRoutingPriorityConfigurationSet function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfIpv6UcPbrRoutingPriorityConfigurationSet
(
    GT_U32 prvTgfPbrLeafIndex,
    GT_U32 prvTgfRouteEntryBaseIndex
)
{
    GT_STATUS      rc = GT_OK;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *regularEntryPtr;
    GT_U32                                  portIter;

    /* clear also counters at end of test */
    for(portIter = 0; portIter < prvTgfPortsNum ; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevsArray[portIter],
                                    prvTgfPortsArray[portIter]);
        PRV_UTF_VERIFY_GT_OK(rc,"prvTgfResetCountersEth");
    }

    PRV_UTF_LOG0_MAC("======= Setting LTT Route Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 1. Create the Route entry (Next hop) in Route table and Router ARP Table
     */

    /* write a ARP MAC address to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfPbrArpMac, sizeof(TGF_MAC_ADDR));

    /* AUTODOC: write ARP MAC 00:12:23:45:67:89 to the Router ARP Table */
    rc = prvTgfIpRouterArpAddrRead(prvTgfDevNum, prvTgfFdbRoutingArpIndex, &prvTgfRestoreCfg.pbrArpMacAddrOrig);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpRouterArpAddrRead");

    /* AUTODOC: write ARP MAC 00:12:23:45:67:89 to the Router ARP Table */
    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &arpMacAddr);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpRouterArpAddrWrite");

    /* write a UC Route entry to the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));
    regularEntryPtr = &routeEntriesArray[0];
    regularEntryPtr->cmd                        = CPSS_PACKET_CMD_ROUTE_E;
    regularEntryPtr->cpuCodeIndex               = 0;
    regularEntryPtr->appSpecificCpuCodeEnable   = GT_FALSE;
    regularEntryPtr->unicastPacketSipFilterEnable = GT_FALSE;
    regularEntryPtr->ttlHopLimitDecEnable       = GT_TRUE;
    regularEntryPtr->ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
    regularEntryPtr->ingressMirror              = GT_FALSE;
    regularEntryPtr->qosProfileMarkingEnable    = GT_FALSE;
    regularEntryPtr->qosProfileIndex            = 0;
    regularEntryPtr->qosPrecedence              = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    regularEntryPtr->modifyUp                   = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    regularEntryPtr->modifyDscp                 = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    regularEntryPtr->countSet                   = 0;
    regularEntryPtr->trapMirrorArpBcEnable      = GT_FALSE;
    regularEntryPtr->sipAccessLevel             = 0;
    regularEntryPtr->dipAccessLevel             = 0;
    regularEntryPtr->ICMPRedirectEnable         = GT_FALSE;
    regularEntryPtr->scopeCheckingEnable        = GT_FALSE;
    regularEntryPtr->siteId                     = CPSS_IP_SITE_ID_INTERNAL_E;
    regularEntryPtr->mtuProfileIndex            = 0;
    regularEntryPtr->isTunnelStart              = GT_FALSE;
    regularEntryPtr->nextHopVlanId              = 1;
    regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
    regularEntryPtr->nextHopInterface.devPort.hwDevNum = prvTgfDevNum;
    regularEntryPtr->nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];
    regularEntryPtr->nextHopARPPointer          = prvTgfRouterArpIndex;
    regularEntryPtr->nextHopTunnelPointer       = 0;

    /* AUTODOC: save route entry */
    rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum, prvTgfRouteEntryBaseIndex, routeEntriesArrayOrig, 1);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpUcRouteEntriesRead");

    /* AUTODOC: add UC route entry 8 with nexthop VLAN 1 and nexthop port 3 */
    rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpUcRouteEntriesWrite");

    /* AUTODOC: read and check the UC Route entry from the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum, prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpUcRouteEntriesRead");
    PRV_UTF_LOG2_MAC("nextHopVlanId = %d, portNum = %d\n",
                     routeEntriesArray[0].nextHopVlanId,
                     routeEntriesArray[0].nextHopInterface.devPort.portNum);

    /* -------------------------------------------------------------------------
     * 2. Create a Leaf Entry
     */
    rc = prvTgfIpv6UcPbrRoutingPriorityLeafEntrySet(CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_LPM_E, prvTgfPbrLeafIndex, prvTgfRouteEntryBaseIndex);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpv4UcPbrRoutingPriorityLeafEntrySet");

    /* Save the enabling status of bypassing the router triggering requirements for PBR packets */
    rc = prvTgfIpPbrBypassRouterTriggerRequirementsEnableGet(prvTgfDevNum, &prvTgfRestoreCfg.pbrBypassTriggerOrig);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpPbrBypassRouterTriggerRequirementsEnableGet");

    /* Enable bypassing the router triggering requirements for PBR packets */
    rc = prvTgfIpPbrBypassRouterTriggerRequirementsEnableSet(prvTgfDevNum, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpPbrBypassRouterTriggerRequirementsEnableSet");

    rc = prvTgfIpFdbUnicastRouteForPbrEnableGet(prvTgfDevNum, &prvTgfRestoreCfg.fdbUnicastRouteForPbrOrig);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpFdbUnicastRouteForPbrEnableGet");

    rc = prvTgfIpFdbUnicastRouteForPbrEnableSet(prvTgfDevNum, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfIpFdbUnicastRouteForPbrEnableSet");

    return rc;
}

/* UDEs and UDBs allocation */
/*
 * UDE  | SL,SI in packet |      UDB Configuration         | Next G-SID in SRH
 * 0    | SL=0, SI=1      | UDBs 62-65: L3 offsets 50-53   | G-SID13
 * 0    | SL=0, SI=2      | UDBs 66-69: L3 offsets 54-57   | G-SID12
 * 1    | SL=0, SI=3      | UDBs 62-65: L3 offsets 58-61   | G-SID11
 * 1    | SL=1, SI=0      | UDBs 66-69: L3 offsets 62-65   | G-SID10
 * 2    | SL=1, SI=1      | UDBs 62-65: L3 offsets 66-69   | G-SID9
 * 2    | SL=1, SI=2      | UDBs 66-69: L3 offsets 70-73   | G-SID8
 * 3    | SL=1, SI=3      | UDBs 62-65: L3 offsets 74-77   | G-SID7
 * 3    | SL=2, SI=0      | UDBs 66-69: L3 offsets 78-81   | G-SID6
 * 4    | SL=2, SI=1      | UDBs 62-65: L3 offsets 82-85   | G-SID5
 * 4    | SL=2, SI=2      | UDBs 66-69: L3 offsets 86-89   | G-SID4
 * 5    | SL=2, SI=3      | UDBs 62-65: L3 offsets 90-93   | G-SID3
 * 5    | SL=3, SI=0      | UDBs 66-69: L3 offsets 94-97   | G-SID2
 */
/**
* @internal prvTgSrv6GSidTunnelCoc32PclConfigSet function
* @endinternal
*
* @brief  SRv6 G-SID Tunnel CoC32 G-SID end node use case PCL configurations.
*/
static GT_VOID prvTgSrv6GSidTunnelCoc32PclConfigSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  udeIndex, udbIndex, udbIndexPacketType, ii;
    PRV_TGF_PCL_PACKET_TYPE_ENT             packetType;
    PRV_TGF_PCL_RULE_FORMAT_UNT             mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT             pattern;
    PRV_TGF_PCL_ACTION_STC                  action;
    PRV_TGF_PCL_UDB_SELECT_STC              udbSelect;
    PRV_TGF_PCL_LOOKUP_CFG_STC              lookupCfg;
    CPSS_INTERFACE_INFO_STC                 interfaceInfo;

    /* UDE0- UDBs:62-69 */
    for(udeIndex = 0; udeIndex < PRV_TGF_PCL_UDE_PKT_TYPES_CNS; udeIndex++)
    {
        packetType =
            (udeIndex == 0)
                ? PRV_TGF_PCL_PACKET_TYPE_UDE5_E
                : (udeIndex == 5)
                ? PRV_TGF_PCL_PACKET_TYPE_UDE_E
                : (PRV_TGF_PCL_PACKET_TYPE_UDE_4_E - (udeIndex - 1));

        for(udbIndex = 0; udbIndex < PRV_TGF_PCL_UDB_INDEX_CNS; udbIndex++)
        {
            udbIndexPacketType = PRV_TGF_PCL_UDE_L3_OFFSET_BASE_CNS + udbIndex + (((PRV_TGF_PCL_UDE_PKT_TYPES_CNS - 1) - udeIndex) * 8);
            rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E,
                            packetType, /* packet type */
                            CPSS_PCL_DIRECTION_INGRESS_E, (PRV_TGF_PCL_UDE_UDB_BASE_INDEX_CNS + udbIndex) /*udbIndex*/,
                            PRV_TGF_PCL_OFFSET_L3_MINUS_2_E, udbIndexPacketType /*offset*/);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                            "prvTgfPclUserDefinedByteSet: %d", (PRV_TGF_PCL_UDE_UDB_BASE_INDEX_CNS + udbIndex));
        }
    }

    /* it's a must to enable UDB's */
    for (ii = 0; (ii < PRV_TGF_TTI_RULE_INDEX_CNS); ii++)
    {
        cpssOsMemSet(&udbSelect,                   0, sizeof(udbSelect));
        packetType =
            (ii == 0 || ii == 1)
                ? PRV_TGF_PCL_PACKET_TYPE_UDE5_E
                : (ii == 10 || ii == 11)
                ? PRV_TGF_PCL_PACKET_TYPE_UDE_E
                : (PRV_TGF_PCL_PACKET_TYPE_UDE_4_E - ((ii / 2) - 1));

        udbSelect.ingrUdbReplaceArr[0] = GT_TRUE; /* PCL-ID */
        udbSelect.udbSelectArr[1]      = PRV_TGF_PCL_UDE_UDB_BASE_INDEX_CNS; /*  */
        udbSelect.udbSelectArr[2]      = PRV_TGF_PCL_UDE_UDB_BASE_INDEX_CNS + 1; /* DIPv4 */
        udbSelect.udbSelectArr[3]      = PRV_TGF_PCL_UDE_UDB_BASE_INDEX_CNS + 2; /* DIPv4 */
        udbSelect.udbSelectArr[4]      = PRV_TGF_PCL_UDE_UDB_BASE_INDEX_CNS + 3; /* DIPv4 */
        udbSelect.udbSelectArr[5]      = PRV_TGF_PCL_UDE_UDB_BASE_INDEX_CNS + 4; /* DIPv4 */
        udbSelect.udbSelectArr[6]      = PRV_TGF_PCL_UDE_UDB_BASE_INDEX_CNS + 5; /* DIPv4 */
        udbSelect.udbSelectArr[7]      = PRV_TGF_PCL_UDE_UDB_BASE_INDEX_CNS + 6; /* DIPv4 */
        udbSelect.udbSelectArr[8]      = PRV_TGF_PCL_UDE_UDB_BASE_INDEX_CNS + 7; /* DIPv4 */

        rc = prvTgfPclUserDefinedBytesSelectSet(  PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E,
                                                  packetType,
                                                  CPSS_PCL_LOOKUP_0_E,
                                                  &udbSelect);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedBytesSelectSet: %d", prvTgfDevNum);
    }

    /* AUTODOC: init PCL Engine for send port 1: */
    /* AUTODOC:   ingress direction, lookup_0 */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_STD_IP_L2_QOS */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    /* Enable PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_TRUE);

    /* Enabling IPCL for port 3 */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
                                 prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                 GT_FALSE);

    /* Set PCL0 configuration table */
    for (ii = 0; (ii < PRV_TGF_TTI_RULE_INDEX_CNS); ii++)
    {
        cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
        cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

        interfaceInfo.type            = CPSS_INTERFACE_INDEX_E;
        interfaceInfo.index           = PRV_TGF_PCL_CONFIG_INDEX_CNS + ii;

        lookupCfg.enableLookup           = GT_TRUE;
        lookupCfg.pclId                  = PRV_TGF_PCL_INDEX_CNS + ii;
        lookupCfg.groupKeyTypes.ipv4Key  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E;
        lookupCfg.groupKeyTypes.nonIpKey = PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E;
        lookupCfg.groupKeyTypes.ipv6Key  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E;

        rc = prvTgfPclCfgTblSet(&interfaceInfo,
                                CPSS_PCL_DIRECTION_INGRESS_E,
                                CPSS_PCL_LOOKUP_0_E,
                                &lookupCfg);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet");
    }

    for (ii = 0; (ii < PRV_TGF_TTI_RULE_INDEX_CNS); ii++)
    {
        cpssOsMemSet(&action,           0, sizeof(action));
        cpssOsMemSet(&(mask),           0, sizeof(mask));
        cpssOsMemSet(&(pattern),        0, sizeof(pattern));

        pattern.ruleIngrUdbOnly.udb[0] = (GT_U8)(PRV_TGF_PCL_INDEX_CNS + ii);
        mask.ruleIngrUdbOnly.udb[0]    = 0xFF;
        if(ii % 2 == 0)
        {
            pattern.ruleIngrUdbOnly.udb[1] = prvTgfCoc32Gsid[((10 - ii/2) * 4 + 0)];
            mask.ruleIngrUdbOnly.udb[1]    = 0x00;
            pattern.ruleIngrUdbOnly.udb[2] = prvTgfCoc32Gsid[((10 - ii/2) * 4 + 1)];
            mask.ruleIngrUdbOnly.udb[2]    = 0x00;
            pattern.ruleIngrUdbOnly.udb[3] = prvTgfCoc32Gsid[((10 - ii/2) * 4 + 2)];
            mask.ruleIngrUdbOnly.udb[3]    = 0x00;
            pattern.ruleIngrUdbOnly.udb[4] = prvTgfCoc32Gsid[((10 - ii/2) * 4 + 3)];
            mask.ruleIngrUdbOnly.udb[4]    = 0x00;
            pattern.ruleIngrUdbOnly.udb[5] = prvTgfCoc32Gsid[((11 - ii/2) * 4 + 0)];
            mask.ruleIngrUdbOnly.udb[5]    = 0xFF;
            pattern.ruleIngrUdbOnly.udb[6] = prvTgfCoc32Gsid[((11 - ii/2) * 4 + 1)];
            mask.ruleIngrUdbOnly.udb[6]    = 0xFF;
            pattern.ruleIngrUdbOnly.udb[7] = prvTgfCoc32Gsid[((11 - ii/2) * 4 + 2)];
            mask.ruleIngrUdbOnly.udb[7]    = 0xFF;
            pattern.ruleIngrUdbOnly.udb[8] = prvTgfCoc32Gsid[((11 - ii/2) * 4 + 3)];
            mask.ruleIngrUdbOnly.udb[8]    = 0xFF;
        }
        else
        {
            pattern.ruleIngrUdbOnly.udb[1] = prvTgfCoc32Gsid[((10 - ii/2) * 4 + 0)];
            mask.ruleIngrUdbOnly.udb[1]    = 0xFF;
            pattern.ruleIngrUdbOnly.udb[2] = prvTgfCoc32Gsid[((10 - ii/2) * 4 + 1)];
            mask.ruleIngrUdbOnly.udb[2]    = 0xFF;
            pattern.ruleIngrUdbOnly.udb[3] = prvTgfCoc32Gsid[((10 - ii/2) * 4 + 2)];
            mask.ruleIngrUdbOnly.udb[3]    = 0xFF;
            pattern.ruleIngrUdbOnly.udb[4] = prvTgfCoc32Gsid[((10 - ii/2) * 4 + 3)];
            mask.ruleIngrUdbOnly.udb[4]    = 0xFF;
            pattern.ruleIngrUdbOnly.udb[5] = prvTgfCoc32Gsid[((11 - ii/2) * 4 + 0)];
            mask.ruleIngrUdbOnly.udb[5]    = 0x00;
            pattern.ruleIngrUdbOnly.udb[6] = prvTgfCoc32Gsid[((11 - ii/2) * 4 + 1)];
            mask.ruleIngrUdbOnly.udb[6]    = 0x00;
            pattern.ruleIngrUdbOnly.udb[7] = prvTgfCoc32Gsid[((11 - ii/2) * 4 + 2)];
            mask.ruleIngrUdbOnly.udb[7]    = 0x00;
            pattern.ruleIngrUdbOnly.udb[8] = prvTgfCoc32Gsid[((11 - ii/2) * 4 + 3)];
            mask.ruleIngrUdbOnly.udb[8]    = 0x00;
        }

        action.pktCmd                                                                = CPSS_PACKET_CMD_FORWARD_E;
        action.redirect.redirectCmd                                                  = PRV_TGF_PCL_ACTION_REDIRECT_CMD_ROUTER_E;
        action.redirect.data.routerLttIndex                                          = prvTgfPbrLeafIndex + ii;

        rc = prvTgfIpv6UcPbrRoutingPriorityConfigurationSet(prvTgfPbrLeafIndex + ii, prvTgfRouteEntryBaseIndex + ii);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpv6UcPbrRoutingPriorityConfigurationSet: %d", prvTgfDevNum);

        /* AUTODOC: config 5 PCL rules [0..4] with: */
        /* AUTODOC:   format - INGRESS_STD_NOT_IP */
        /* AUTODOC:   cmd FORWARD */
        /* AUTODOC:   pattern UDBs=[0x70..0x74] */
        rc = prvTgfPclRuleSet(
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E, (PRV_TGF_PCL_CONFIG_INDEX_CNS + ii), &mask, &pattern, &action);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
                                     prvTgfDevNum, PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E, (PRV_TGF_PCL_CONFIG_INDEX_CNS + ii));
    }
}

/**
* internal prvTgfSrv6GSidTunnelCoc32PhaConfigSet function
* @endinternal
*
* @brief   PHA config for SRv6 G-SID Tunnel Coc32 End Node use case.
*/
static GT_VOID prvTgfSrv6GSidTunnelCoc32PhaConfigSet
(
    GT_VOID
)
{
    CPSS_DXCH_PHA_THREAD_INFO_UNT           extInfo;

    /* GM does not support PHA */
    if(GT_TRUE == prvUtfIsGmCompilation() || (prvTgfSrv6OneSegmentSkipPha == GT_TRUE))
        return;

    cpssOsMemSet(&extInfo, 0, sizeof(extInfo));
    extInfo.srv6Coc32GsidCommonPrefix.dipCommonPrefixLength = 64;
    /* AUTODOC: Assign PHA thread "SRv6 END_NODE_32_GSID" to egress Port */

    prvTgSrv6PhaThreadConfigSet(prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                PRV_TGF_PHA_THREAD_TYPE_SRV6_END_NODE_COC32_GSID_E,
                                CPSS_DXCH_PHA_THREAD_TYPE_SRV6_END_NODE_COC32_GSID_E,
                                &extInfo);
}

/**
* @internal prvTgfIpv4UcPbrRoutingPriorityBaseConfigurationSet function
* @endinternal
*
* @brief   Set Base Configuration
*
* @param[in] prvUtfVrfId    - virtual router ID
*
*/
static GT_VOID prvTgfIpv6UcPbrRoutingPriorityBaseConfigurationSet
(
    GT_U32  prvUtfVrfId
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */

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
}

/**
* @internal prvTgfIpv6UcPbrRoutingPriorityFdbRoutingGenericConfigurationSet function
* @endinternal
*
* @brief   Set FDB Generic Route Configuration
*/
GT_VOID prvTgfIpv6UcPbrRoutingPriorityFdbRoutingGenericConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: get enable Unicast IPv4 Routing status on port 3 */
    rc = prvTgfIpPortRoutingEnableGet(PRV_TGF_INGRESS_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV6_E,
                                   &prvTgfRestoreCfg.ipv4UcPortRoutingEnableOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnableGet");


    /* AUTODOC: enable Unicast IPv4 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_INGRESS_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                               CPSS_IP_PROTOCOL_IPV6_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                             prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);

    /* AUTODOC: Get enable/disable status of  using of FDB for Destination IP lookup for IP UC routing */
    rc = prvTgfBrgFdbRoutingPortIpUcEnableGet( prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                           CPSS_IP_PROTOCOL_IPV4_E, &prvTgfRestoreCfg.fdbRoutingPortEnableOrig );
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingPortIpUcEnableGet");
    /* AUTODOC: Enable using of FDB for Destination IP lookup for IP UC routing */
    rc = prvTgfBrgFdbRoutingPortIpUcEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                          CPSS_IP_PROTOCOL_IPV6_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingPortIpUcEnableSet: %d %d",
                             prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: enable IPv6 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_DEFAULT_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV6_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                             prvTgfDevNum, PRV_TGF_DEFAULT_VLANID_CNS);

    /* AUTODOC: save MAC SA Base  */
    rc = prvTgfIpRouterMacSaBaseGet(prvTgfDevNum, &prvTgfRestoreCfg.macSaBaseOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterMacSaBaseGet");

    /* AUTODOC: sets 40 MSBs of MAC SA base addr to 00:00:00:00:00 */
    rc = prvTgfIpRouterMacSaBaseSet(prvTgfDevNum, &prvTgfMacAddr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterMacSaBaseSet");
}

/**
* @internal prvTgfIpv6UcPbrRoutingPriorityFdbRoutingConfigurationSet function
* @endinternal
*
* @brief   Set FDB Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*
*/
GT_VOID prvTgfIpv6UcPbrRoutingPriorityFdbRoutingConfigurationSet
(
    GT_U32  prvUtfVrfId
)
{
    GT_ETHERADDR                            arpMacAddr;
    GT_STATUS                               rc = GT_OK;

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

    /* AUTODOC: update VRF Id in vlan[PRV_TGF_DEFAULT_VLANID_CNS] */
    rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_DEFAULT_VLANID_CNS, prvUtfVrfId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanVrfIdSet: %d %d",
                             prvTgfDevNum, PRV_TGF_DEFAULT_VLANID_CNS);
}

/**
* internal tgfSrv6GSidTunnelCoc32EndNodeConfigSet function
* @endinternal
*
* @brief   SRV6 G-SID Tunnel Coc32 End Node use case configurations
*/
GT_VOID tgfSrv6GSidTunnelCoc32EndNodeConfigSet
(
    GT_VOID
)
{
    /* TTI Engine Configurations */
    prvTgSrv6GSidTunnelCoc32TtiConfigSet();

    /* IPCL Engine Configurations */
    prvTgSrv6GSidTunnelCoc32PclConfigSet();

    /* Select random vrfId */
    prvUtfVrfId = prvUtfRandomVrfIdNumberGet();

    /* Set PBR Base configuration */
    prvTgfIpv6UcPbrRoutingPriorityBaseConfigurationSet(prvUtfVrfId);

    /* Set Generic FDB Route configuration */
    prvTgfIpv6UcPbrRoutingPriorityFdbRoutingGenericConfigurationSet();

    /* Set FDB Route configuration */
    prvTgfIpv6UcPbrRoutingPriorityFdbRoutingConfigurationSet(prvUtfVrfId);

    /* PHA - Attach PHA Thread "SRv6 G-SID Tunnel" at Egress port */
    prvTgfSrv6GSidTunnelCoc32PhaConfigSet();

}

/**
* internal tgfSrv6GSidTunnelCoc32EndNodeVerification function
* @endinternal
*
* @brief   SRv6 G-SID Tunnel CoC32 End Node use case verification
*/
GT_VOID tgfSrv6GSidTunnelCoc32EndNodeVerification
(
    GT_VOID
)
{
    GT_STATUS                           rc = GT_OK;
    GT_U8                               packetBuf[PRV_TGF_PACKET_SIZE_CNS] = {0};
    GT_U32                              packetLen = PRV_TGF_PACKET_SIZE_CNS;
    GT_U32                              packetActualLength = 0;
    GT_U8                               queue = 0;
    GT_U8                               dev = 0, ii;
    TGF_NET_DSA_STC                     rxParam;
    GT_BOOL                             getFirst = GT_TRUE;
    GT_U32                              pktParse = 0;
    GT_U32                              pktCount, hopLimit = 0;

    for(pktCount = 0; pktCount < 2; pktCount++)
    {
        rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&egrPortInterface,
                TGF_PACKET_TYPE_CAPTURE_E,
                getFirst, GT_TRUE, packetBuf,
                &packetLen, &packetActualLength,
                &dev, &queue, &rxParam);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet: egrPortInterface %d", prvTgfDevNum);
        getFirst = GT_FALSE;

        /* MAC DA Verification */
        for (ii = 0; ii < 6; ii++)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfPbrArpMac[ii],
                    packetBuf[pktParse],
                    "ARP DA is not matching at index[%d]", ii);
            pktParse++;
        }

        /* MAC SA Verification */
        pktParse = TGF_L2_HEADER_SIZE_CNS / 2;
        for (ii = 0; ii < 5; ii++)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfMacAddr.arEther[ii],
                    packetBuf[pktParse],
                    "Router SA is not matching at index[%d]", ii);
            pktParse++;
        }

        /* IPv6 Ether Type Verification */
        pktParse = TGF_L2_HEADER_SIZE_CNS;
        UTF_VERIFY_EQUAL0_STRING_MAC(TGF_ETHERTYPE_86DD_IPV6_TAG_CNS,
                                     (GT_U16)(packetBuf[pktParse + 1] | (packetBuf[pktParse] << 8)),
                                     "IPv6 Ether Type is not matching");

        /* IPv6 Next Header Verification */
        pktParse += TGF_ETHERTYPE_SIZE_CNS + 6;
        UTF_VERIFY_EQUAL0_STRING_MAC(0x2b,
                                     (packetBuf[pktParse]),
                "IPv6 Next Header is not matching");

        /* IPv6 Hop Limit Verification */
        pktParse++;
        hopLimit = prvTgfGsidCoc32PacketIpv6Part.hopLimit - (pktCount + 1);
        UTF_VERIFY_EQUAL0_STRING_MAC(hopLimit,
                                     (packetBuf[pktParse]),
                "IPv6 Hop Limit is not matching");

        /* IPv6 DIP Common Prefix Verification */
        pktParse++;
        pktParse+= TGF_IPV6_ADDR_LEN_CNS; /* Point to IPv6 DIP Common Prefix */
        for (ii = 0; ii < 8; ii++)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(dipCommonPrefix[ii], packetBuf[pktParse],
                    "IPv6 DIP Common Prefix part is not matching at index[%d]", ii);
            pktParse++;
        }

        /* IPv6 DIP G-SID Verification */
        for (ii = 0; ii < 4; ii++)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfCoc32Gsid[(44 - (pktCount*4)) + ii], packetBuf[pktParse],
                    "IPv6 DIP G-SID2 part is not matching at index[%d]", ii);
            pktParse++;
        }

        /* IPv6 DIP SI Verification */
        pktParse+= 3; /* Point to IPv6 DIP LSB */
        UTF_VERIFY_EQUAL0_STRING_MAC(0x3 - pktCount,
                                     (packetBuf[pktParse] & 0x3),
                "IPv6 Segments Identifier is not matching");

        /*** Next_Hdr +++ Hdr_Ext_Len +++ Routing_Type +++ Segments_Left ***/
        /* Next_Hdr */
        pktParse++;
        UTF_VERIFY_EQUAL0_STRING_MAC(0x06,
                (GT_U8)(packetBuf[pktParse]),
                "Next_Hdr is not matching with ");
        pktParse++;

        /* Hdr_Ext_Len */
        UTF_VERIFY_EQUAL0_STRING_MAC(0x06,
                (GT_U8)(packetBuf[pktParse]),
                "Hdr_Len_Ext is not matching with ");
        pktParse++;

        /* Routing_Type */
        UTF_VERIFY_EQUAL0_STRING_MAC(4, /* Constant 4 */
                (GT_U8)(packetBuf[pktParse]),
                "Routing_Type is not matching with constant value");
        pktParse++;

        /* Segments_Left */
        UTF_VERIFY_EQUAL0_STRING_MAC(2, /* Num of segments left */
                (GT_U8)(packetBuf[pktParse]),
                "Segments_Left is not matching with constant value");
        pktParse++;

        /* AUTODOC: SRH Header last 4 Bytes should be 0 */
        for (ii = 0; ii < 4; ii++)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(0, (GT_U8)packetBuf[pktParse],
                    "SRH base header is not matching at index[%d]", ii);
            pktParse++;
        }

        for (ii = 0; ii < sizeof(prvTgfCoc32Gsid)/sizeof(prvTgfCoc32Gsid[0]); ii++)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfCoc32Gsid[ii],
                    (packetBuf[pktParse]),
                    "SRH container is not matching at index[%d]", ii);
            pktParse++;
        }
        pktParse = 0;
    }
}

/* Test packet explanation */
/* SRv6 encapsulation with 13 G-SIDs
 * Container 0 contains COC32 G-SIDs 10,11,12 and non-COC32 G-SID13
 * Container 1 contains COC32 G-SIDs 6,7,8,9
 * Container 2 contains COC32 G-SIDs 2,3,4,5
 *             IPv6 Header
 * DIP : Prefix + G-SID1 (CoC)+ SI = 0
 * SRH Fixed Header SL = 3
 * G-SID13 | G-SID12 | G-SID11 | G-SID10
 * G-SID9  | G-SID8  | G-SID7  | G-SID6
 * G-SID5  | G-SID4  | G-SID3  | G-SID2
 * Customer Packet
 */
/**
* @internal tgfSrv6GSidTunnelCoc32EndNodeTest function
* @endinternal
*
* @brief   SRv6 G-SID Tunnel Coc32 End Node use case test.
*/
GT_VOID tgfSrv6GSidTunnelCoc32EndNodeTest
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIter;

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum,
                    prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "prvTgfResetCountersEth: %d, %d",
                    prvTgfDevNum,
                    prvTgfPortsArray[portIter]);
    }

    packetInfo.numOfParts =  sizeof(prvTgfGsidCoc32PacketIpv6PartArray) / sizeof(prvTgfGsidCoc32PacketIpv6PartArray[0]);
    packetInfo.partsArray = prvTgfGsidCoc32PacketIpv6PartArray;
    packetInfo.totalLen   = TGF_PACKET_AUTO_CALC_LENGTH_CNS;

    /* setup receive portInterface for capturing */
    egrPortInterface.type               = CPSS_INTERFACE_PORT_E;
    egrPortInterface.devPort.hwDevNum   = prvTgfDevNum;
    egrPortInterface.devPort.portNum    = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    /* AUTODOC: GENERATE TRAFFIC & Verify at ePort#3 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
                        &egrPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgftrafficgeneratorporttxethcaptureset port - %d",
                                        egrPortInterface.devPort.portNum);

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, 1, 0, NULL);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfSetTxSetupEth");

    /* Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");

    packetInfo.numOfParts =  sizeof(prvTgfGsidCoc32PacketIpv6PartArray2) / sizeof(prvTgfGsidCoc32PacketIpv6PartArray2[0]);
    packetInfo.partsArray = prvTgfGsidCoc32PacketIpv6PartArray2;
    packetInfo.totalLen   = TGF_PACKET_AUTO_CALC_LENGTH_CNS;

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, 1, 0, NULL);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfSetTxSetupEth");

    /* Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");

    /* disable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &egrPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet port - %d",
            egrPortInterface.devPort.portNum);

    /* CoC32 G-SID End Node Processing Verification */
    tgfSrv6GSidTunnelCoc32EndNodeVerification();
}

/**
* @internal tgfSrv6GSidTunnelCoc32EndNodeConfigRestore function
* @endinternal
*
* @brief  SRv6 G-SID Tunnel Coc32 End Node use case configurations restore.
*/
GT_VOID tgfSrv6GSidTunnelCoc32EndNodeConfigRestore
(
    GT_VOID
)
{
    GT_STATUS                  rc = GT_OK;
    GT_U32                     ii,ttiRuleIndex;
    PRV_TGF_TTI_MAC_VLAN_STC   macToMePattern;
    PRV_TGF_TTI_MAC_VLAN_STC   macToMeMask;
    CPSS_INTERFACE_INFO_STC    interfaceInfo;
    PRV_TGF_PCL_LOOKUP_CFG_STC lookupCfg;

    /* AUTODOC: Restore TTI/PCL Configuration */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_UDB_IPV6_TCP_E, prvTgfRestoreCfg.lookupEnableGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    rc = prvTgfTtiPacketTypeKeySizeSet(prvTgfDevNum,
                            PRV_TGF_TTI_KEY_UDB_IPV6_TCP_E,
                            prvTgfRestoreCfg.keySizeGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);

    /* AUTODOC: invalidate TTI rules  */
    ii = 0;
    for(ttiRuleIndex = 0; ; ttiRuleIndex++)
    {
        if(ttiRuleIndex % 2 == 0 && ttiRuleIndex % 12 != 0)
        {
            rc = prvTgfTtiRuleValidStatusSet(ttiRuleIndex, GT_FALSE);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");
        }

        ii++;
        if(ii == PRV_TGF_TTI_RULE_INDEX_CNS)
        {
            break;
        }
    }

    /* AUTODOC: invalidate PCL rules  */
    for(ii = 0; ii < PRV_TGF_TTI_RULE_INDEX_CNS; ii++)
    {
        rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_10_BYTES_E, (PRV_TGF_PCL_CONFIG_INDEX_CNS + ii), GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d",
                                     CPSS_PCL_RULE_SIZE_10_BYTES_E,
                                     PRV_TGF_PCL_CONFIG_INDEX_CNS + ii);
    }

    /* Disabling IPCL for port 3 */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
                                 prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                 GT_FALSE);

    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", prvTgfDevNum);

    /* AUTODOC: remove PCL configurations */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    for (ii = 0; (ii < PRV_TGF_TTI_RULE_INDEX_CNS); ii++)
    {
        cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
        cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

        interfaceInfo.type            = CPSS_INTERFACE_INDEX_E;
        interfaceInfo.index           = PRV_TGF_PCL_CONFIG_INDEX_CNS + ii;

        lookupCfg.enableLookup           = GT_FALSE;  /* --> disable the PCL lookup */
        lookupCfg.pclId                  = 0;
        lookupCfg.groupKeyTypes.nonIpKey = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/;
        lookupCfg.groupKeyTypes.ipv4Key  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E /*ipv4Key*/;
        lookupCfg.groupKeyTypes.ipv6Key  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv6Key*/;

        rc = prvTgfPclCfgTblSet(&interfaceInfo,
                                CPSS_PCL_DIRECTION_INGRESS_E,
                                CPSS_PCL_LOOKUP_0_E,
                                &lookupCfg);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet");
    }


    interfaceInfo.type               = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum   = prvTgfDevNum;
    interfaceInfo.devPort.portNum    = CPSS_NULL_PORT_NUM_CNS;
    lookupCfg.enableLookup           = GT_FALSE;  /* --> disable the PCL lookup */
    lookupCfg.pclId                  = 0;
    lookupCfg.groupKeyTypes.nonIpKey = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/;
    lookupCfg.groupKeyTypes.ipv4Key  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E /*ipv4Key*/;
    lookupCfg.groupKeyTypes.ipv6Key  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv6Key*/;

    rc = prvTgfPclCfgTblSet(
        &interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E, 0/*dont care*/, &lookupCfg);
    if (GT_OK != rc)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                         "[TGF]: prvTgfPclCfgTblSet FAILED: EGRESS, on NULL port");
    }

    /* clean PCL configuration */
    prvTgfPclPortsRestoreAll();

    /* AUTODOC: disable IPv6 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_DEFAULT_VLANID_CNS, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV6_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                    prvTgfDevNum, PRV_TGF_DEFAULT_VLANID_CNS);

    /* AUTODOC: get enable Unicast IPv4 Routing status on port 3 */
    rc = prvTgfIpPortRoutingEnable( PRV_TGF_INGRESS_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV6_E,
                                   prvTgfRestoreCfg.ipv4UcPortRoutingEnableOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable");


    /* remove Virtual Router [prvUtfVrfId] */
    if (0 != prvUtfVrfId)
    {
        rc = prvTgfIpLpmVirtualRouterDel(prvTgfLpmDBId, prvUtfVrfId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                        "prvTgfIpLpmVirtualRouterDel: %d",
                                        prvTgfDevNum);
    }

    /* enable Patricia trie validity */
    prvTgfIpValidityCheckEnable(GT_TRUE);

    /* restore ARP MACs*/
    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &prvTgfRestoreCfg.pbrArpMacAddrOrig);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    rc = prvTgfIpRouterArpAddrWrite(prvTgfFdbRoutingArpIndex, &prvTgfRestoreCfg.fdbArpMacAddrOrig);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* AUTODOC: restore Packet Commands for FDB routed packets */
    rc = prvTgfBrgFdbRoutingNextHopPacketCmdSet( prvTgfDevNum, prvTgfRestoreCfg.nhPacketCmdOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingNextHopPacketCmdSet");

    /* AUTODOC: restore enable value of FDB IP lookup for IP UC routing */
    rc = prvTgfBrgFdbRoutingPortIpUcEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                          CPSS_IP_PROTOCOL_IPV6_E, prvTgfRestoreCfg.fdbRoutingPortEnableOrig);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingPortIpUcEnableSet: %d %d",
                             prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);

    /* Restore FDB routing configuration */

    /* Restore PBR configuration */
    /* Enable bypassing the router triggering requirements for PBR packets */
    rc = prvTgfIpPbrBypassRouterTriggerRequirementsEnableSet(prvTgfDevNum, prvTgfRestoreCfg.pbrBypassTriggerOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpPbrBypassRouterTriggerRequirementsEnableSet");

    rc = prvTgfIpFdbUnicastRouteForPbrEnableSet(prvTgfDevNum, prvTgfRestoreCfg.fdbUnicastRouteForPbrOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpFdbUnicastRouteForPbrEnableSet");

    for(ii = 0; ii < PRV_TGF_TTI_RULE_INDEX_CNS; ii++)
    {
        /* AUTODOC: restore a PBR leaf */
        prvTgfLpmLeafEntryWrite(prvTgfDevNum, prvTgfPbrLeafIndex + ii, &leafEntryOrig);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "restore prvTgfLpmLeafEntryWrite");

        /* AUTODOC: restore route entry */
        rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex + ii, routeEntriesArrayOrig, 1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite");
    }

    /* AUTODOC: restore MAC SA Base  */
    rc = prvTgfIpRouterMacSaBaseSet(prvTgfDevNum, &prvTgfRestoreCfg.macSaBaseOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterMacSaBaseGet");

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgFdbFlush");

    /* AUTODOC: Invalidate MAC to ME table entry index 1 */

    /* full mask for VLAN and MAC */
    macToMeMask.mac.arEther[0] =
    macToMeMask.mac.arEther[1] =
    macToMeMask.mac.arEther[2] =
    macToMeMask.mac.arEther[3] =
    macToMeMask.mac.arEther[4] =
    macToMeMask.mac.arEther[5] = 0xFF;
    macToMeMask.vlanId = 0x1FFF;

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

    /* AUTODOC: Restore PHA Configuration */
    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                    GT_FALSE, PRV_TGF_PHA_THREAD_TYPE_SRV6_END_NODE_COC32_GSID_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaPortThreadIdSet FAILED for SRV6_BEST_EFFORT, rc = [%d]", rc);


    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");
}

static GT_U32 restorePclId      = 1;
static GT_U32 origNextHeader;
static TGF_IPV6_ADDR ipv6_Addr_global  = {0x99aa, 0xbbcc, 0xaabb, 0xccdd, 0xbbcc, 0xddee, 0xccdd, 0xeeff};
static TGF_IPV6_ADDR ipv6_Addr_restore;

/**
 * @internal prvTgfSrv6TunnelEndNodeDipUpdateVerificationTest_config function
 * @endinternal
 *
 * @brief  SRv6 Tunnel end node DIP update - Config.
 */
GT_VOID prvTgfSrv6TunnelEndNodeDipUpdateVerificationTest_config
(
   GT_VOID
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_TTI_ACTION_2_STC            ttiAction;
    PRV_TGF_TTI_RULE_UNT                ttiMask;
    PRV_TGF_TTI_RULE_UNT                ttiPattern;
    PRV_TGF_TTI_KEY_TYPE_ENT            tunnelPacketType = PRV_TGF_TTI_KEY_ETH_E;
    GT_U32                              ii, pclId = 1;
    GT_IPV6ADDR                         ipAddr;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC       routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC       *regularEntryPtr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* enable the TTI lookup for PRV_TGF_TTI_KEY_ETH_E */
    rc = prvTgfTtiPortLookupEnableGet(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
            PRV_TGF_TTI_KEY_UDB_IPV6_TCP_E, &prvTgfRestoreCfg.lookupEnableGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableGet");

    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
            tunnelPacketType, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    cpssOsMemSet(&(ttiAction),  0, sizeof(ttiAction));
    cpssOsMemSet(&(ttiMask),    0, sizeof(ttiMask));
    cpssOsMemSet(&(ttiPattern), 0, sizeof(ttiPattern));

    /* AUTODOC: Sets TTI Rule Pattern and Mask */
    /* AUTODOC: Rule is empty and matches all packets */
    ttiMask.eth.common.pclId    = 0x3FF;
    ttiPattern.eth.common.pclId = pclId;

    /* TTI actions - FORWARD packet */
    ttiAction.command                         = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction.ipv6SegmentRoutingEndNodeEnable = GT_TRUE;

    /* AUTODOC: set TTI rule */
    rc = prvTgfTtiRule2Set(0, tunnelPacketType,
            &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");

    /* AUTODOC: Gets the PCL ID for the specified key type */
    rc = prvTgfTtiPclIdGet(prvTgfDevNum, tunnelPacketType, &restorePclId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdSet");

    /* AUTODOC: Sets the PCL ID for the specified key type */
    rc = prvTgfTtiPclIdSet(prvTgfDevNum, tunnelPacketType, pclId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdSet");

    /* AUTODOC: get enable Unicast IPv6 Routing status on port 3 */
    rc = prvTgfIpPortRoutingEnableGet(PRV_TGF_INGRESS_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
            CPSS_IP_PROTOCOL_IPV6_E,
            &prvTgfRestoreCfg.ipv4UcPortRoutingEnableOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnableGet");

    rc = prvTgfIpPortRoutingEnable(PRV_TGF_INGRESS_PORT_IDX_CNS,
            CPSS_IP_UNICAST_E,
            CPSS_IP_PROTOCOL_IPV6_E,
            GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
            prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);

    /* AUTODOC: enable IPv6 Unicast Routing on Vlan 1 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_DEFAULT_VLANID_CNS,
            CPSS_IP_UNICAST_E,
            CPSS_IP_PROTOCOL_IPV6_E,
            GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
            prvTgfDevNum, PRV_TGF_DEFAULT_VLANID_CNS);

    /* write a UC Route entry to the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));
    regularEntryPtr = &routeEntriesArray[0];
    regularEntryPtr->cmd                        = CPSS_PACKET_CMD_ROUTE_E;
    regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
    regularEntryPtr->nextHopVlanId              = 1;
    regularEntryPtr->nextHopInterface.devPort.hwDevNum = prvTgfDevNum;
    regularEntryPtr->nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    /* AUTODOC: add UC route entry with nexthop VLAN 2 and nexthop port 3 */
    rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

    /* Create a new Ipv6 prefix in a Virtual Router for the specified LPM DB */
    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex          = prvTgfRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType                    = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* fill a destination IPv6 address for the prefix - End segment */
    for (ii = 0; ii < 8; ii++)
    {
        ipAddr.arIP[ii * 2]     = (GT_U8)(ipv6_Addr_global[ii] >> 8);
        ipAddr.arIP[ii * 2 + 1] = (GT_U8) ipv6_Addr_global[ii];

        /* TODO - Remove after the IPv6 routing fix */
        /* Update packet DIP with route Data */
        ipv6_Addr_restore[ii] = prvTgfGsidCoc32PacketIpv6Part.dstAddr[ii];
        prvTgfGsidCoc32PacketIpv6Part.dstAddr[ii] = ipv6_Addr_global[ii];
    }

    /* call CPSS function */
    rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, 0/*prvUtfVrfId*/,
            ipAddr, 128, &nextHopInfo, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixAdd: %d", prvTgfDevNum);

    /* Bridging Engine Configuration
     * Route the packet from ingress to Egress */
    prvTgSrv6BridgingConfigSet(prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS], &prvTgfOriginalPacketL2Part.daMac);

    /* AUTODOC: update VRF Id in vlan[PRV_TGF_DEFAULT_VLANID_CNS] */
    rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_DEFAULT_VLANID_CNS, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanVrfIdSet: %d %d",
                             prvTgfDevNum, PRV_TGF_DEFAULT_VLANID_CNS);
}

/**
 * @internal prvTgfSrv6TunnelEndNodeDipUpdateVerificationTest_trafficAndVerify function
 * @endinternal
 *
 * @brief  SRv6 Tunnel end node DIP update - Traffic and verify.
 */
GT_VOID prvTgfSrv6TunnelEndNodeDipUpdateVerificationTest_trafficAndVerify
(
    GT_VOID
)
{
    GT_STATUS                           rc = GT_OK;
    GT_U32                              portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC       portCntrs;

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum,
                prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                "prvTgfResetCountersEth: %d, %d",
                prvTgfDevNum,
                prvTgfPortsArray[portIter]);
    }

    packetInfo.numOfParts =  sizeof(prvTgfGsidCoc32PacketIpv6PartArray) / sizeof(prvTgfGsidCoc32PacketIpv6PartArray[0]);
    packetInfo.partsArray = prvTgfGsidCoc32PacketIpv6PartArray;
    packetInfo.totalLen   = TGF_PACKET_AUTO_CALC_LENGTH_CNS;

    origNextHeader   = prvTgfGsidCoc32PacketSrhPart.nextHeader;
    prvTgfGsidCoc32PacketSrhPart.nextHeader   = 43;

    /* setup receive portInterface for capturing */
    egrPortInterface.type               = CPSS_INTERFACE_PORT_E;
    egrPortInterface.devPort.hwDevNum   = prvTgfDevNum;
    egrPortInterface.devPort.portNum    = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, 1, 0, NULL);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfSetTxSetupEth");

    /* Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");

    /* sleep for 5 milisec */
    tgfSimSleep(5);

    /* Verify packet counter */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfReadPortCountersEth(prvTgfDevNum,
                prvTgfPortsArray[portIter],
                GT_TRUE, &portCntrs);
        PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfReadPortCountersEth");

        if(portIter == PRV_TGF_INGRESS_PORT_IDX_CNS)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(portCntrs.goodPktsRcv.l[0], 1,
                   "get another counters values");
        }
        else
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(portCntrs.goodPktsSent.l[0],
                    (portIter == PRV_TGF_EGR_PORT_IDX_CNS) ? 1 : 0,
                    "get another counters values");
        }
    }
}

/**
 * @internal prvTgfSrv6TunnelEndNodeDipUpdateVerificationTest_restore function
 * @endinternal
 *
 * @brief  SRv6 Tunnel end node DIP update - restore.
 */
GT_VOID prvTgfSrv6TunnelEndNodeDipUpdateVerificationTest_restore
(
    GT_VOID
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_TTI_KEY_TYPE_ENT            tunnelPacketType = PRV_TGF_TTI_KEY_ETH_E;
    GT_U32                              ii;
    GT_IPV6ADDR                         ipAddr;

    PRV_UTF_LOG0_MAC("======= Resetting TTI Configurations =======\n");

    /* AUTODOC: Restore PCL ID for the specified key type */
    rc = prvTgfTtiPclIdSet(prvTgfDevNum, tunnelPacketType, restorePclId);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfTtiPclIdSet");

    /* AUTODOC: Invalidates the TTI rule */
    rc = prvTgfTtiRuleValidStatusSet(0 /*ruleId*/, GT_FALSE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfTtiRuleValidStatusSet");

    /* AUTODOC: Disables the TTI lookup for port[PRV_TGF_INGRESS_PORT_IDX_CNS] and key PRV_TGF_TTI_KEY_ETH_E */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS], tunnelPacketType, GT_FALSE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: Sets TTI indexes to absolute */
    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);

    prvTgfGsidCoc32PacketSrhPart.nextHeader   = origNextHeader;

    /* AUTODOC: disable IPv6 Unicast Routing on Vlan 1 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_DEFAULT_VLANID_CNS,
            CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV6_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
            prvTgfDevNum, PRV_TGF_DEFAULT_VLANID_CNS);

    /* AUTODOC: get enable Unicast IPv4 Routing status on port 3 */
    rc = prvTgfIpPortRoutingEnable( PRV_TGF_INGRESS_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
            CPSS_IP_PROTOCOL_IPV6_E,
            prvTgfRestoreCfg.ipv4UcPortRoutingEnableOrig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable");

    /* fill a destination IPv6 address for the prefix */
    for (ii = 0; ii < 8; ii++)
    {
        ipAddr.arIP[ii * 2]     = (GT_U8)(ipv6_Addr_global[ii] >> 8);
        ipAddr.arIP[ii * 2 + 1] = (GT_U8) ipv6_Addr_global[ii];

        /* TODO - Remove after the IPv6 routing fix */
        /* Restore */
        prvTgfGsidCoc32PacketIpv6Part.dstAddr[ii] = ipv6_Addr_restore[ii];
    }

    /* AUTODOC: delete the IPv6 prefix */
    rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, 0/*prvUtfVrfId*/, ipAddr, 128);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "tgfTrafficTableRxPcktTblClear");

    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "tgfTrafficTableRxStartCapture");
}

/**
 * @internal tgfSrv6TunnelEndNodeDipUpdateVerificationTest function
 * @endinternal
 *
 * @brief  SRv6 Tunnel end node DIP update.
 */
GT_VOID tgfSrv6TunnelEndNodeDipUpdateVerificationTest
(
    GT_VOID
)
{
    prvTgfSrv6TunnelEndNodeDipUpdateVerificationTest_config();

    prvTgfSrv6TunnelEndNodeDipUpdateVerificationTest_trafficAndVerify();

    prvTgfSrv6TunnelEndNodeDipUpdateVerificationTest_restore();
}
