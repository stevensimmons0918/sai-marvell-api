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
* @file prvTgfPclUdbOnlyKeys.c
*
* @brief SIP5 specific PCL UDB Only keys testing
*
* @version   12
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfTrunkGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfOamGen.h>
#include <pcl/prvTgfPclUdbOnlyKeys.h>
#include <common/tgfCncGen.h>
#include <cnc/prvTgfCncVlanL2L3.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS            1

/* port number to receive traffic from */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS         2

/* PCL rule index */
#define PRV_TGF_PCL_RULE_IDX_CNS             8

/* TTI rule index */
#define PRV_TGF_TTI_RULE_IDX_CNS             2

/* ARP Entry index */
#define PRV_TGF_ARP_ENTRY_IDX_CNS            3

/* TTI ID */
#define PRV_TGF_TTI_ID_CNS                   1

/* PCL ID */
#define PRV_TGF_PCL_ID_CNS                   0x55

/* original VLAN Id */
#define PRV_TGF_PASSENGER_TAG0_VLAN_ID_CNS   0x10

/* original VLAN Id */
#define PRV_TGF_PASSENGER_TAG1_VLAN_ID_CNS   0x11

/* original VLAN Id */
#define PRV_TGF_TUNNEL_TAG0_VLAN_ID_CNS      0x21

/* original VLAN Id */
#define PRV_TGF_TUNNEL_TAG1_VLAN_ID_CNS      0x22

/* lookup1 VLAN Id */
#define PRV_TGF_PCL_NEW_VLAN_ID_CNS          0x31

/* Action Flow Id  */
#define PRV_TGF_TTI_ACTION_FLOW_ID_CNS       0x25

/* Action Flow Id  */
#define PRV_TGF_PCL_ACTION_FLOW_ID_CNS       0x26

/* Router Source MAC_SA index  */
#define PRV_TGF_PCL_ROUTER_MAC_SA_INDEX_CNS  0x12

/* TCP Sorce port  */
#define PRV_TGF_PCL_TCP_SRC_PORT_CNS         0x7654

/* TCP Destination port  */
#define PRV_TGF_PCL_TCP_DST_PORT_CNS         0xFECB

/* UDP Sorce port  */
#define PRV_TGF_PCL_UDP_SRC_PORT_CNS         0x4567

/* UDP Destination port  */
#define PRV_TGF_PCL_UDP_DST_PORT_CNS         0xBCEF

/* IPV6 Sorce IP  */
#define PRV_TGF_PCL_IPV6_SRC_IP_CNS          {0x23, 0x45, 0x67, 0x89, 0x87, 0x65, 0x43, 0x21}

/* IPV6 Destination IP  */
#define PRV_TGF_PCL_IPV6_DST_IP_CNS          {0x01, 0x2E, 0x3D, 0x4C, 0x5B, 0x6A, 0x79, 0x88}

/* PCL id for restore procedure */
static GT_U32 savePclId;
/********************************************************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketPassengerL2Part =
{
    {0x00, 0x12, 0x34, 0x56, 0x78, 0x9A},                /* daMac */
    {0x00, 0x11, 0x22, 0x33, 0x44, 0x55}                 /* saMac */
};

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketTunnelL2Part =
{
    {0x00, 0xFE, 0xDC, 0xBA, 0x98, 0x76},                /* daMac */
    {0x00, 0xBB, 0xAA, 0x11, 0x22, 0x33}                 /* saMac */
};

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2PartWithBytesLess64 =
{
    {0x00, 0x12, 0x34, 0x16, 0x28, 0x3A},                /* daMac */
    {0x00, 0x11, 0x22, 0x03, 0x14, 0x25}                 /* saMac */
};


/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketPassengerVlanTag0Part =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,              /* etherType */
    0, 0, PRV_TGF_PASSENGER_TAG0_VLAN_ID_CNS      /* pri, cfi, VlanId */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketPassengerVlanTag1Part =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,              /* etherType */
    0, 0, PRV_TGF_PASSENGER_TAG1_VLAN_ID_CNS      /* pri, cfi, VlanId */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketTunnelVlanTag0Part =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,           /* etherType */
    0, 0, PRV_TGF_TUNNEL_TAG0_VLAN_ID_CNS      /* pri, cfi, VlanId */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketTunnelVlanTag1Part =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,           /* etherType */
    0, 0, PRV_TGF_TUNNEL_TAG1_VLAN_ID_CNS      /* pri, cfi, VlanId */
};

/* LLC Non SNAP Length part */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketLlcNonSnapLengthPart =
{
    0x0100
};

/* Ethernet Type part */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEthOth1EthertypePart =
{
    0x3456
};

/* Ethernet Type part of MPLS packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketMplsEtherTypePart =
{
    TGF_ETHERTYPE_8847_MPLS_TAG_CNS
};

/* ethertype part of IPV4 packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv4EtherTypePart =
{
    TGF_ETHERTYPE_0800_IPV4_TAG_CNS
};

/* ethertype part of IPV6 packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv6EtherTypePart =
{
    TGF_ETHERTYPE_86DD_IPV6_TAG_CNS
};

/* Payload of packet */
static GT_U8 prvTgfPacketPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPacketPayloadDataArr),            /* dataLength */
    prvTgfPacketPayloadDataArr                     /* dataPtr */
};

/* packet's UDP part */
static TGF_PACKET_UDP_STC prvTgfPacketPassengerUdpPart =
{
    PRV_TGF_PCL_UDP_SRC_PORT_CNS,                  /* src port */
    PRV_TGF_PCL_UDP_DST_PORT_CNS,                  /* dst port */
    sizeof(prvTgfPacketPayloadDataArr),            /* length   */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS              /* csum     */
};

/* packet's TCP part */
static TGF_PACKET_TCP_STC prvTgfPacketPassengerTcpPart =
{
    PRV_TGF_PCL_TCP_SRC_PORT_CNS,                  /* src port */
    PRV_TGF_PCL_TCP_DST_PORT_CNS,                  /* dst port */
    1,                                             /* sequence number */
    2,                                             /* acknowledgment number */
    0,                                             /* data offset */
    0,                                             /* reserved */
    0x00,                                          /* flags */
    4096,                                          /* window */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    0                   /* urgent pointer */
};

/* packet's IPv6 header */
static TGF_PACKET_IPV6_STC prvTgfPacketPassengerIpv6OtherHeaderPart =
{
    6,                                  /* version */
    0,                                  /* trafficClass */
    0,                                  /* flowLabel */
    sizeof(prvTgfPacketPayloadDataArr), /* payloadLen */
    0x3b,                               /* nextHeader */
    0x40,                               /* hopLimit */
    PRV_TGF_PCL_IPV6_SRC_IP_CNS,        /* srcAddr */
    PRV_TGF_PCL_IPV6_DST_IP_CNS         /* dstAddr */
};

/* packet's IPv6 header */
static TGF_PACKET_IPV6_STC prvTgfPacketPassengerIpv6UdpHeaderPart =
{
    6,                                                              /* version */
    0,                                                              /* trafficClass */
    0,                                                              /* flowLabel */
    (sizeof(prvTgfPacketPayloadDataArr) + TGF_UDP_HEADER_SIZE_CNS), /* payloadLen */
    TGF_PROTOCOL_UDP_E,                                             /* nextHeader */
    0x40,                                                           /* hopLimit */
    PRV_TGF_PCL_IPV6_SRC_IP_CNS,                                    /* srcAddr */
    PRV_TGF_PCL_IPV6_DST_IP_CNS                                     /* dstAddr */
};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketPassengerIpv4OtherHeaderPart =
{
    4,                                                               /* version */
    (TGF_IPV4_HEADER_SIZE_CNS / 4),                                  /* headerLen */
    0,                                                               /* typeOfService */
    (TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)), /* totalLen */
    0,                                                               /* id */
    4,                                                               /* flags */
    0,                                                               /* offset */
    0x40,                                                            /* timeToLive */
    4,                                                               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,                               /* csum */
    {21, 22, 23, 24},                                                /* srcAddr */
    { 1,  1,  1,  3}                                                 /* dstAddr */
};

/* packet's IPv4 UDP*/
static TGF_PACKET_IPV4_STC prvTgfPacketPassengerIpv4UdpHeaderPart =
{
    4,                                                               /* version */
    (TGF_IPV4_HEADER_SIZE_CNS / 4),                                  /* headerLen */
    0,                                                               /* typeOfService */
    (TGF_IPV4_HEADER_SIZE_CNS + TGF_UDP_HEADER_SIZE_CNS
         + sizeof(prvTgfPacketPayloadDataArr)),                      /* totalLen */
    0,                                                               /* id */
    4,                                                               /* flags */
    0,                                                               /* offset */
    0x40,                                                            /* timeToLive */
    TGF_PROTOCOL_UDP_E,                                              /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,                               /* csum */
    {32, 42, 52, 62},                                                /* srcAddr */
    { 1,  2,  3,  4}                                                 /* dstAddr */
};


/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketPassengerIpv4TcpHeaderPart =
{
    4,                                                               /* version */
    (TGF_IPV4_HEADER_SIZE_CNS / 4),                                  /* headerLen */
    0,                                                               /* typeOfService */
    (TGF_IPV4_HEADER_SIZE_CNS + TGF_TCP_HEADER_SIZE_CNS
         + sizeof(prvTgfPacketPayloadDataArr)),                      /* totalLen */
    0,                                                               /* id */
    4,                                                               /* flags */
    0,                                                               /* offset */
    0x40,                                                            /* timeToLive */
    TGF_PROTOCOL_TCP_E,                                              /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,                               /* csum */
    {32, 42, 52, 62},                                                /* srcAddr */
    { 1,  2,  3,  4}                                                 /* dstAddr */
};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketTunnelIpv4OtherHeaderPart = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    (TGF_IPV6_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)),               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x29,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    {16, 17, 18, 19},   /* srcAddr */
    { 4,  5,  6,  7}    /* dstAddr */
};

/* Double Tagged Ethernet Other Packet Parts Array*/
static TGF_PACKET_PART_STC prvTgfPacketNotTunneled2tagsEthOthPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag1Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEthOth1EthertypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Double Tagged Ethernet Other Packet Info */
static TGF_PACKET_STC prvTgfPacketNotTunneled2tagsEthOthInfo = {
    (TGF_L2_HEADER_SIZE_CNS + (2 * TGF_VLAN_TAG_SIZE_CNS) + TGF_ETHERTYPE_SIZE_CNS
     + sizeof(prvTgfPacketPayloadDataArr)),                      /* totalLen */
    sizeof(prvTgfPacketNotTunneled2tagsEthOthPartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketNotTunneled2tagsEthOthPartsArray                                        /* partsArray */
};

/* Double Tagged IPV6 Other Packet Parts Array*/
static TGF_PACKET_PART_STC prvTgfPacketNotTunneled2tagsIpv6OthPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag1Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv6EtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketPassengerIpv6OtherHeaderPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Double Tagged IPV6 Other Packet Info */
static TGF_PACKET_STC prvTgfPacketNotTunneled2tagsIpv6OthInfo = {
    (TGF_L2_HEADER_SIZE_CNS + (2 * TGF_VLAN_TAG_SIZE_CNS) + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV6_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)),                   /* totalLen */
    sizeof(prvTgfPacketNotTunneled2tagsIpv6OthPartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketNotTunneled2tagsIpv6OthPartsArray                                        /* partsArray */
};

/* MPLS Tunnel Header */
static TGF_PACKET_MPLS_STC prvTgfPacketTunnelMplsLabel0 =
{
    1111 /* Addressing Label */,
    0/*exp*/,
    1/*stack - last*/,
    100/*timeToLive*/
};

/* Double Tagged MPLS Packet Tunneled IPV4 Other Parts Array*/
static TGF_PACKET_PART_STC prvTgfPacketMplsTunneled2tagsIpv4OthPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketTunnelL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketTunnelVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketTunnelVlanTag1Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMplsEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketTunnelMplsLabel0},
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag1Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketPassengerIpv4OtherHeaderPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Double Tagged MPLS Packet Tunneled IPV4 Other Packet Info */
static TGF_PACKET_STC prvTgfPacketMplsTunneled2tagsIpv4OthInfo = {
    (TGF_L2_HEADER_SIZE_CNS + (2 * TGF_VLAN_TAG_SIZE_CNS) + TGF_ETHERTYPE_SIZE_CNS
     + TGF_MPLS_HEADER_SIZE_CNS
     + TGF_L2_HEADER_SIZE_CNS + (2 * TGF_VLAN_TAG_SIZE_CNS) + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)),                    /* totalLen */
    sizeof(prvTgfPacketMplsTunneled2tagsIpv4OthPartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketMplsTunneled2tagsIpv4OthPartsArray                                        /* partsArray */
};

/* Double Tagged IPV6 Packet Tunneled L2 IPV6 Other Parts Array*/
static TGF_PACKET_PART_STC prvTgfPacketIpv4TunneledL2Ipv6OthPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketTunnelL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketTunnelVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketTunnelVlanTag1Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketTunnelIpv4OtherHeaderPart},
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag1Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv6EtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketPassengerIpv6OtherHeaderPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Double Tagged IPV6 Packet Tunneled L2 IPV6 Other Packet Info */
static TGF_PACKET_STC prvTgfPacketIpv4TunneledL2Ipv6OthInfo = {
    (TGF_L2_HEADER_SIZE_CNS + (2 * TGF_VLAN_TAG_SIZE_CNS) + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV4_HEADER_SIZE_CNS
     + TGF_L2_HEADER_SIZE_CNS + (2 * TGF_VLAN_TAG_SIZE_CNS) + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV6_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)),                    /* totalLen */
    sizeof(prvTgfPacketIpv4TunneledL2Ipv6OthPartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketIpv4TunneledL2Ipv6OthPartsArray                                        /* partsArray */
};

/* Double Tagged IPV4 Other Passenger Packet Parts Array */
static TGF_PACKET_PART_STC prvTgfPacketPassenger2tagsIpv4OthPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag1Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketPassengerIpv4OtherHeaderPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Double Tagged IPV4 Other Passenger Packet Info */
static TGF_PACKET_STC prvTgfPacketPassenger2tagsIpv4OthInfo = {
    (TGF_L2_HEADER_SIZE_CNS + (2 * TGF_VLAN_TAG_SIZE_CNS) + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)),                 /* totalLen */
    sizeof(prvTgfPacketPassenger2tagsIpv4OthPartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPassenger2tagsIpv4OthPartsArray                                        /* partsArray */
};

/* Double Tagged Packet IPV6 Other Parts Array*/
static TGF_PACKET_PART_STC prvTgfPacketPassengerIpv6OthPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag1Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv6EtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketPassengerIpv6OtherHeaderPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Double Tagged IPV6 Packet Tunneled L2 IPV6 Other Packet Info */
static TGF_PACKET_STC prvTgfPacketPassengerIpv6OthInfo = {
    (TGF_L2_HEADER_SIZE_CNS + (2 * TGF_VLAN_TAG_SIZE_CNS) + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV6_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)),                    /* totalLen */
    sizeof(prvTgfPacketPassengerIpv6OthPartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPassengerIpv6OthPartsArray                                        /* partsArray */
};

/* Double Tagged MPLS Tunneled Ethernet Other Packet Parts Array*/
static TGF_PACKET_PART_STC prvTgfPacketMplsTunneled2tagsEthOthPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketTunnelL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketTunnelVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketTunnelVlanTag1Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMplsEtherTypePart},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketTunnelMplsLabel0},
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag1Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEthOth1EthertypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Double Tagged MPLS Tunneled Ethernet Other Packet Info */
static TGF_PACKET_STC prvTgfPacketMplsTunneled2tagsEthOthInfo = {
    (TGF_L2_HEADER_SIZE_CNS + (2 * TGF_VLAN_TAG_SIZE_CNS) + TGF_ETHERTYPE_SIZE_CNS
     + TGF_MPLS_HEADER_SIZE_CNS
     + TGF_L2_HEADER_SIZE_CNS + (2 * TGF_VLAN_TAG_SIZE_CNS) + TGF_ETHERTYPE_SIZE_CNS
     + sizeof(prvTgfPacketPayloadDataArr)),                      /* totalLen */
    sizeof(prvTgfPacketMplsTunneled2tagsEthOthPartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketMplsTunneled2tagsEthOthPartsArray                                        /* partsArray */
};

/* Double Tagged Packet IPV4 TCP Parts Array*/
static TGF_PACKET_PART_STC prvTgfPacketPasenger2tagsIpv4TcpPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag1Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketPassengerIpv4TcpHeaderPart},
    {TGF_PACKET_PART_TCP_E,       &prvTgfPacketPassengerTcpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Double Tagged Packet IPV4 TCP Packet Info */
static TGF_PACKET_STC prvTgfPacketPassenger2tagsIpv4TcpInfo = {
    (TGF_L2_HEADER_SIZE_CNS + (2 * TGF_VLAN_TAG_SIZE_CNS) + TGF_ETHERTYPE_SIZE_CNS
      + TGF_IPV4_HEADER_SIZE_CNS + TGF_TCP_HEADER_SIZE_CNS
      + sizeof(prvTgfPacketPayloadDataArr)),                    /* totalLen */
    sizeof(prvTgfPacketPasenger2tagsIpv4TcpPartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPasenger2tagsIpv4TcpPartsArray                                        /* partsArray */
};

/* Double Tagged Packet IPV4 UDP Parts Array*/
static TGF_PACKET_PART_STC prvTgfPacketPassengerIpv4UdpPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag1Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketPassengerIpv4UdpHeaderPart},
    {TGF_PACKET_PART_UDP_E,       &prvTgfPacketPassengerUdpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Double Tagged IPV4 Packet UDP Packet Info */
static TGF_PACKET_STC prvTgfPacketPassengerIpv4UdpInfo = {
    (TGF_L2_HEADER_SIZE_CNS + (2 * TGF_VLAN_TAG_SIZE_CNS)+ TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV4_HEADER_SIZE_CNS + TGF_UDP_HEADER_SIZE_CNS
     + sizeof(prvTgfPacketPayloadDataArr)),                    /* totalLen */
    sizeof(prvTgfPacketPassengerIpv4UdpPartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPassengerIpv4UdpPartsArray                                        /* partsArray */
};

/* Double Tagged Packet IPV6 UDP Parts Array*/
static TGF_PACKET_PART_STC prvTgfPacketPassengerIpv6UdpPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag1Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv6EtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketPassengerIpv6UdpHeaderPart},
    {TGF_PACKET_PART_UDP_E,       &prvTgfPacketPassengerUdpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Double Tagged IPV6 Packet Tunneled L2 IPV6 UDP Packet Info */
static TGF_PACKET_STC prvTgfPacketPassengerIpv6UdpInfo = {
    (TGF_L2_HEADER_SIZE_CNS + (2 * TGF_VLAN_TAG_SIZE_CNS) + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV6_HEADER_SIZE_CNS + TGF_UDP_HEADER_SIZE_CNS
     + sizeof(prvTgfPacketPayloadDataArr)),                    /* totalLen */
    sizeof(prvTgfPacketPassengerIpv6UdpPartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPassengerIpv6UdpPartsArray                                        /* partsArray */
};

/* Single Tagged LLC Non SNAP Packet Parts Array*/
static TGF_PACKET_PART_STC prvTgfPacketPassengerLlcNonSnapPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag1Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketLlcNonSnapLengthPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Single Tagged LLC Non SNAP Packet Info */
static TGF_PACKET_STC prvTgfPacketPassengerLlcNonSnapInfo = {
    (TGF_L2_HEADER_SIZE_CNS + (2 * TGF_VLAN_TAG_SIZE_CNS) + TGF_ETHERTYPE_SIZE_CNS +
     + sizeof(prvTgfPacketPayloadDataArr)),                    /* totalLen */
    sizeof(prvTgfPacketPassengerLlcNonSnapPartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPassengerLlcNonSnapPartsArray                                        /* partsArray */
};

/* backup configuration for restore after the test */
static GT_U32   origRouterMacSaIndex = 0;
static GT_ETHERADDR origMacSaAddr = {{0}};
static CPSS_DXCH_PCL_EPG_CONFIG_STC epgConfigRestore;
static GT_U32   currentRuleIndex = 0;

static void setRuleIndex(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum
)
{
    GT_U32  lookupId;

    if(direction == CPSS_PCL_DIRECTION_EGRESS_E)
    {
        currentRuleIndex =  prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(PRV_TGF_PCL_RULE_IDX_CNS); ;
    }
    else
    {
        lookupId = lookupNum == CPSS_PCL_LOOKUP_NUMBER_0_E ? 0 :
                   lookupNum == CPSS_PCL_LOOKUP_NUMBER_1_E ? 1 :
                   lookupNum == CPSS_PCL_LOOKUP_NUMBER_2_E ? 2 :
                                                             0 ;/*CPSS_PCL_LOOKUP_0_E*/

        currentRuleIndex =  prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(lookupId,PRV_TGF_PCL_RULE_IDX_CNS);
    }
}

static GT_U32 getRuleIndexBase(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum
)
{
    GT_U32  lookupId;
    GT_U32  indexBase;

    if(direction == CPSS_PCL_DIRECTION_EGRESS_E)
    {
        indexBase = prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(0); ;
    }
    else
    {
        lookupId = lookupNum == CPSS_PCL_LOOKUP_NUMBER_0_E ? 0 :
                   lookupNum == CPSS_PCL_LOOKUP_NUMBER_1_E ? 1 :
                   lookupNum == CPSS_PCL_LOOKUP_NUMBER_2_E ? 2 :
                                                             0 ;/*CPSS_PCL_LOOKUP_0_E*/

        indexBase =  prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(lookupId,0);
    }
    return indexBase;
}


/**
* @internal prvTgfPclUdbOnlyKeysPortVlanFdbSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_STATUS prvTgfPclUdbOnlyKeysPortVlanFdbSet
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: reset counters, force Link UP */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfEthCountersReset");

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: create VLAN as TAG in Passanger with all TAG0 and TAG1 tagged ports */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_PASSENGER_TAG0_VLAN_ID_CNS, PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    /* AUTODOC: create VLAN as TAG in Tunnel with all TAG0 and TAG1 tagged ports */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_TUNNEL_TAG0_VLAN_ID_CNS, PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    /* AUTODOC: create VLAN as PCL assigned VID with all TAG0 and TAG1 tagged ports */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_PCL_NEW_VLAN_ID_CNS, PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    /* AUTODOC: Learn MAC_DA of Passenger with source VID to receive port (to be Known UC) */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
        prvTgfPacketPassengerL2Part.daMac,
        PRV_TGF_PASSENGER_TAG0_VLAN_ID_CNS /*vlanId*/,
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        GT_TRUE /*isStatic*/);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    /* AUTODOC: Learn MAC_DA of Tunnel with source VID to receive port (to be Known UC) */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
        prvTgfPacketTunnelL2Part.daMac,
        PRV_TGF_TUNNEL_TAG0_VLAN_ID_CNS /*vlanId*/,
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        GT_TRUE /*isStatic*/);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    /* AUTODOC: Learn MAC_DA of Passenger with Assigned VID to receive port (to be Known UC) */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
        prvTgfPacketPassengerL2Part.daMac,
        PRV_TGF_PCL_NEW_VLAN_ID_CNS /*vlanId*/,
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        GT_TRUE /*isStatic*/);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    /* AUTODOC: Learn MAC_DA of Tunnel with Assigned VID to receive port (to be Known UC) */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
        prvTgfPacketTunnelL2Part.daMac,
        PRV_TGF_PCL_NEW_VLAN_ID_CNS /*vlanId*/,
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        GT_TRUE /*isStatic*/);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    return GT_OK;
}

/**
* @internal prvTgfPclUdbOnlyKeysPortVlanFdbReset function
* @endinternal
*
* @brief   Set test configuration
*/
GT_STATUS prvTgfPclUdbOnlyKeysPortVlanFdbReset
(
    GT_VOID
)
{
    GT_STATUS rc, rc1 = GT_OK;

    /* AUTODOC: reset counters, force Link UP */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfEthCountersReset");

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgFdbFlush");

    /* AUTODOC: invalidate VLAN Table Entry, VID as TAG in Passanger */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_PASSENGER_TAG0_VLAN_ID_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");

    /* AUTODOC: invalidate VLAN Table Entry, VID as TAG in Tunnel */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_TUNNEL_TAG0_VLAN_ID_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");

    /* AUTODOC: invalidate VLAN Table Entry, VID as PCL assigned */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_PCL_NEW_VLAN_ID_CNS);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfBrgDefVlanEntryInvalidate");

    return rc1;
}

/**
* @internal prvTgfPclUdbOnlyKeysVidTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] prvTgfPacketInfoPtr      - (pointer to) packet info
*                                       None
*/
GT_VOID prvTgfPclUdbOnlyKeysVidTrafficGenerate
(
    IN TGF_PACKET_STC *prvTgfPacketInfoPtr
)
{
    GT_STATUS            rc         = GT_OK;

    /* AUTODOC: Transmit Packets With Capture*/

    rc = prvTgfTransmitPacketsWithCapture(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPacketInfoPtr,
        1      /*burstCount*/,
        0      /*numVfd*/,
        NULL   /*vfdArray*/,
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        TGF_CAPTURE_MODE_MIRRORING_E,
        200                                /*captureOnTime*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfTransmitPacketsWithCapture");
}

/**
* @internal prvTgfPclUdbOnlyKeysTrafficEgressVidCheck function
* @endinternal
*
* @brief   Checks traffic egress VID in the Tag
*
* @param[in] egressVid                - VID found in egressed packets VLAN Tag
* @param[in] checkMsb                 - to check High bits of VID and UP
*                                       None
*/
static GT_VOID prvTgfPclUdbOnlyKeysTrafficEgressVidCheck
(
    IN GT_U16  egressVid,
    IN GT_BOOL checkMsb
)
{
    GT_STATUS                       rc         = GT_OK;
    CPSS_INTERFACE_INFO_STC         portInterface;
    TGF_VFD_INFO_STC                vfdArray[1];
    GT_U32                          numTriggersBmp;

    /* AUTODOC: Check VID from Tag of captured packet */

    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));
    cpssOsMemSet(vfdArray, 0, sizeof(vfdArray));

    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];

    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    if (checkMsb == GT_FALSE)
    {
        vfdArray[0].offset = 15; /* 6 + 6 + 3 */
        vfdArray[0].cycleCount = 1;
        vfdArray[0].patternPtr[0] = (GT_U8)(egressVid & 0xFF);
    }
    else
    {
        vfdArray[0].offset = 14; /* 6 + 6 + 2 */
        vfdArray[0].cycleCount = 2;
        vfdArray[0].patternPtr[0] = (GT_U8)((egressVid >> 8) & 0xFF);
        vfdArray[0].patternPtr[1] = (GT_U8)(egressVid & 0xFF);
    }

    rc =  tgfTrafficGeneratorPortTxEthTriggerCountersGet(
            &portInterface,
            1 /*numVfd*/,
            vfdArray,
            &numTriggersBmp);
    if (rc != GT_NO_MORE)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d",
            prvTgfDevNum);
    }
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
        0, numTriggersBmp, "received patterns bitmap");
}

/**
* @internal prvTgfPclUdbOnlyKeysPclLegacyCfgSetWithUDB49BmpExt function
* @endinternal
*
* @brief   Set Legacy PCL Configuration
*
* @param[in] portNum                  - port Num
* @param[in] direction                - PCL direction
* @param[in] lookupNum                - PCL Lookup number
* @param[in] accessMode               - PCL CFG table access mode
* @param[in] cfgIndex                 - PCL CFG table entry index (if mode not "port")
* @param[in] pktType                  - packet type: 0- not IP, 1 - IPV4, 2 - IPV6
* @param[in] pclId                    - PCL Id
* @param[in] ruleFormat               - rule Format
* @param[in] ruleIndex                - rule index
* @param[in] maskPtr                  - (pointer to) rule mask
* @param[in] patternPtr               - (pointer to) rule pattern
* @param[in] actionPtr                - (pointer to) Action
* @param[in] udb49BitmapEnable        - enable/disable bitmap of UDB49 values
*/
GT_STATUS prvTgfPclUdbOnlyKeysPclLegacyCfgSetWithUDB49BmpExt
(
    IN GT_U32                                       portNum,
    IN CPSS_PCL_DIRECTION_ENT                       direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT                   lookupNum,
    IN PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT accessMode,
    IN GT_U32                                       cfgIndex,
    IN GT_U32                                       pktType, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
    IN GT_U32                                       pclId,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT             ruleFormat,
    IN GT_U32                                       ruleIndex,
    IN PRV_TGF_PCL_RULE_FORMAT_UNT                  *maskPtr,
    IN PRV_TGF_PCL_RULE_FORMAT_UNT                  *patternPtr,
    IN PRV_TGF_PCL_ACTION_STC                       *actionPtr,
    IN GT_BOOL                                      udb49BitmapEnable
)
{
    GT_STATUS rc;
    PRV_TGF_PCL_LOOKUP_CFG_STC       lookupCfg;

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.enableLookup           = GT_TRUE;
    lookupCfg.udbKeyBitmapEnable     = udb49BitmapEnable;
    lookupCfg.pclId                  = pclId;
    lookupCfg.udbKeyBitmapEnable     = udb49BitmapEnable;
    lookupCfg.groupKeyTypes.nonIpKey =
        (direction == CPSS_PCL_DIRECTION_EGRESS_E)
            ? PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E
            : PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key =
        (direction == CPSS_PCL_DIRECTION_EGRESS_E)
            ? PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E
            : PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key = lookupCfg.groupKeyTypes.ipv4Key;
    switch (pktType)
    {
        default:
        case 0: lookupCfg.groupKeyTypes.nonIpKey = ruleFormat; break;
        case 1: lookupCfg.groupKeyTypes.ipv4Key  = ruleFormat; break;
        case 2: lookupCfg.groupKeyTypes.ipv6Key  = ruleFormat; break;
    }

    rc = prvTgfPclDefPortInitExtGeneric(
        portNum, direction, lookupNum, accessMode, cfgIndex, &lookupCfg);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclDefPortInitExtGeneric");

    /* AUTODOC: PCL rule: Write Rule */
    rc = prvTgfPclRuleSet(
        ruleFormat, ruleIndex,
        maskPtr, patternPtr, actionPtr);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclRuleSet");

    if (direction == CPSS_PCL_DIRECTION_EGRESS_E)
    {
        /* enables egress policy per devices */
        rc = prvTgfPclEgressPclPacketTypesSet(
            prvTgfDevNum, portNum,
            PRV_TGF_PCL_EGRESS_PKT_TS_E, GT_TRUE);
        PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclEgressPclPacketTypesSet");
    }

    return GT_OK;
}

/**
* @internal prvTgfPclUdbOnlyKeysPclLegacyCfgSetWithUDB49Bitmap function
* @endinternal
*
* @brief   Set Legacy PCL Configuration
*
* @param[in] direction                - PCL direction
* @param[in] lookupNum                - PCL Lookup number
* @param[in] pktType                  - packet type: 0- not IP, 1 - IPV4, 2 - IPV6
* @param[in] pclId                    - PCL Id
* @param[in] ruleFormat               - rule Format
* @param[in] maskPtr                  - (pointer to) rule mask
* @param[in] patternPtr               - (pointer to) rule pattern
* @param[in] udb49BitmapEnable        - enable/disable bitmap of UDB49 values
*/
GT_STATUS prvTgfPclUdbOnlyKeysPclLegacyCfgSetWithUDB49Bitmap
(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN GT_U32                           pktType, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
    IN GT_U32                           pclId,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN PRV_TGF_PCL_RULE_FORMAT_UNT      *maskPtr,
    IN PRV_TGF_PCL_RULE_FORMAT_UNT      *patternPtr,
    IN GT_BOOL                          udb49BitmapEnable
)
{
    PRV_TGF_PCL_ACTION_STC           action;
    GT_U32                           portIndex;

    portIndex =
        (direction == CPSS_PCL_DIRECTION_EGRESS_E)
            ? PRV_TGF_RECEIVE_PORT_IDX_CNS
            : PRV_TGF_SEND_PORT_IDX_CNS;

    /* AUTODOC: action - TAG0 vlan modification */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    if (direction == CPSS_PCL_DIRECTION_EGRESS_E)
    {
        action.egressPolicy = GT_TRUE;
        action.vlan.egressVlanId0Cmd =
            PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E;
        action.vlan.vlanId = PRV_TGF_PCL_NEW_VLAN_ID_CNS;
    }
    else
    {
        action.egressPolicy = GT_FALSE;
        action.vlan.vlanCmd =
            CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
        action.vlan.modifyVlan =
            CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
        action.vlan.vlanId = PRV_TGF_PCL_NEW_VLAN_ID_CNS;
    }

    /* set currentRuleIndex */
    setRuleIndex(direction, lookupNum);

    return prvTgfPclUdbOnlyKeysPclLegacyCfgSetWithUDB49BmpExt(
        prvTgfPortsArray[portIndex], direction, lookupNum,
        PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E,
        0 /*cfgIndex not relevant for port mode */,
        pktType, pclId, ruleFormat, currentRuleIndex,
        maskPtr, patternPtr, &action, udb49BitmapEnable);
}

/**
* @internal prvTgfPclUdbOnlyKeysPclLegacyCfgSet function
* @endinternal
*
* @brief   Set Legacy PCL Configuration
*
* @param[in] direction                - PCL direction
* @param[in] lookupNum                - PCL Lookup number
* @param[in] pktType                  - packet type: 0- not IP, 1 - IPV4, 2 - IPV6
* @param[in] ruleFormat               - rule Format
* @param[in] maskPtr                  - (pointer to) rule mask
* @param[in] patternPtr               - (pointer to) rule pattern
*                                       None
*/
GT_STATUS prvTgfPclUdbOnlyKeysPclLegacyCfgSet
(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN GT_U32                           pktType, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN PRV_TGF_PCL_RULE_FORMAT_UNT      *maskPtr,
    IN PRV_TGF_PCL_RULE_FORMAT_UNT      *patternPtr
)
{
    return prvTgfPclUdbOnlyKeysPclLegacyCfgSetWithUDB49Bitmap(
    direction, lookupNum, pktType, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
    PRV_TGF_PCL_ID_CNS, ruleFormat, maskPtr, patternPtr,
    GT_FALSE /*udb49BitmapEnable*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysPclLegacyCfgResetExt function
* @endinternal
*
* @brief   Restore Legacy PCL Configuration
*
* @param[in] direction                - PCL direction
* @param[in] lookupNum                - PCL Lookup number
* @param[in] ruleSize                 - Rule Size
* @param[in] ruleIndex                - Rule Index
*                                       None
*/
GT_STATUS prvTgfPclUdbOnlyKeysPclLegacyCfgResetExt
(
    IN CPSS_PCL_DIRECTION_ENT          direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum,
    IN CPSS_PCL_RULE_SIZE_ENT          ruleSize,
    IN GT_U32                          ruleIndex
)
{
    GT_STATUS rc,rc1 = GT_OK;

    /* AUTODOC: restore PCL configuration table entry */
    prvTgfPclPortsRestore(direction, lookupNum);

    /* AUTODOC: invalidate PCL rule */
    rc = prvTgfPclRuleValidStatusSet(
        ruleSize,
        ruleIndex,
        GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclRuleValidStatusSet");

    if (direction == CPSS_PCL_DIRECTION_EGRESS_E)
    {
        /* enables egress policy per devices */
        rc = prvTgfPclEgressPclPacketTypesSet(
            prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            PRV_TGF_PCL_EGRESS_PKT_TS_E, GT_FALSE);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfPclEgressPclPacketTypesSet");
    }

    return rc1;
}

/**
* @internal prvTgfPclUdbOnlyKeysPclLegacyCfgReset function
* @endinternal
*
* @brief   Restore Legacy PCL Configuration
*
* @param[in] direction                - PCL direction
* @param[in] lookupNum                - PCL Lookup number
* @param[in] ruleSize                 - Rule Size
*                                       None
*/
GT_STATUS prvTgfPclUdbOnlyKeysPclLegacyCfgReset
(
    IN CPSS_PCL_DIRECTION_ENT          direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum,
    IN CPSS_PCL_RULE_SIZE_ENT          ruleSize
)
{
    return prvTgfPclUdbOnlyKeysPclLegacyCfgResetExt(
        direction, lookupNum, ruleSize, currentRuleIndex);
}

/**
* @internal prvTgfPclUdbOnlyKeysPclUdbRangeCfgSet function
* @endinternal
*
* @brief   Set PCL UDB Range Configuration
*
* @param[in] direction                - PCL direction
* @param[in] packetType               - packet type
* @param[in] offsetType               - offset Type
* @param[in] udbIndexBase             - udb IndexB ase
* @param[in] udbAmount                - udb Amount
* @param[in] udbOffsetBase            - udb Offset Base
* @param[in] udbOffsetIncrement       - udb Offset Increment (signed value, typical 1 or -1)
*                                       None
*/
GT_STATUS prvTgfPclUdbOnlyKeysPclUdbRangeCfgSet
(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN PRV_TGF_PCL_OFFSET_TYPE_ENT      offsetType,
    IN GT_U32                           udbIndexBase,
    IN GT_U32                           udbAmount,
    IN GT_U32                           udbOffsetBase,
    IN GT_32                            udbOffsetIncrement
)
{
    GT_STATUS                            rc;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT     ruleFormat;
    GT_U8                                offset;
    GT_U32                               udbIndex;
    GT_U32                               i;

    ruleFormat =
        (direction == CPSS_PCL_DIRECTION_EGRESS_E)
            ? PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E
            : PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;

    for (i = 0; (i < udbAmount); i++)
    {
        udbIndex = udbIndexBase + i;
        offset = (GT_U8)(udbOffsetBase + (i * udbOffsetIncrement));

        rc = prvTgfPclUserDefinedByteSet(
            ruleFormat, packetType, direction,udbIndex, offsetType, offset);
        PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclUserDefinedByteSet");
    }

    return GT_OK;
}


/**
* @internal prvTgfPclUdbOnlyKeysPclUdbRangeCfgReset function
* @endinternal
*
* @brief   Reset PCL UDB Range Configuration
*
* @param[in] direction                - PCL direction
* @param[in] packetType               - packet type
* @param[in] udbIndexBase             - udb IndexB ase
* @param[in] udbAmount                - udb Amount
*                                       None
*/
GT_STATUS prvTgfPclUdbOnlyKeysPclUdbRangeCfgReset
(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN GT_U32                           udbIndexBase,
    IN GT_U32                           udbAmount
)
{
    GT_STATUS                            rc, rc1 = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT     ruleFormat;
    GT_U32                               udbIndex;
    GT_U32                               i;

    ruleFormat =
        (direction == CPSS_PCL_DIRECTION_EGRESS_E)
            ? PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E
            : PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;

    for (i = 0; (i < udbAmount); i++)
    {
        udbIndex = udbIndexBase + i;

        rc = prvTgfPclUserDefinedByteSet(
            ruleFormat, packetType, direction, udbIndex,
            PRV_TGF_PCL_OFFSET_INVALID_E, 0/*offset*/);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfPclUserDefinedByteSet");
    }

    return rc1;
}

/**
* @internal prvTgfPclUdbOnlyKeysTtiTunnelTermConfigSet function
* @endinternal
*
* @brief   build TTI Basic rule
*
* @param[in] tunnelPacketType         - Tunnel Packet Type (to enable lookup)
* @param[in] passengerPacketType      - Passenger Packet Type
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvTgfPclUdbOnlyKeysTtiTunnelTermConfigSet
(
    IN PRV_TGF_TTI_KEY_TYPE_ENT       tunnelPacketType,
    IN PRV_TGF_TTI_PASSENGER_TYPE_ENT passengerPacketType
)
{
    GT_STATUS                rc = GT_OK;
    PRV_TGF_TTI_ACTION_2_STC ttiAction;
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for TCAM location */

    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));


    /* AUTODOC: set TTI Action */
    /* AUTODOC:   cmd FORWARD, given passenger type */
    ttiAction.tunnelTerminate                   = GT_TRUE;
    ttiAction.ttPassengerPacketType             = passengerPacketType;
    ttiAction.command                           = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction.tag0VlanCmd                       = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
    ttiAction.tag0VlanPrecedence                = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for MPLS TCAM location */
    /* AUTODOC: rule is empty and matches all packets */
    switch (tunnelPacketType)
    {
        case PRV_TGF_TTI_KEY_MPLS_E:
            ttiMask.mpls.common.pclId    = 0x3FF;
            ttiPattern.mpls.common.pclId = PRV_TGF_TTI_ID_CNS;
            break;
        case PRV_TGF_TTI_KEY_IPV4_E:
            ttiMask.ipv4.common.pclId    = 0x3FF;
            ttiPattern.ipv4.common.pclId = PRV_TGF_TTI_ID_CNS;
            break;
        default: break;
    }

    /* AUTODOC: enable the TTI lookup for given packet type at the port */
    rc = prvTgfTtiPortLookupEnableSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        tunnelPacketType, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPortLookupEnableSet");

    /* save PCL id */
    rc = prvTgfTtiPclIdGet(
        prvTgfDevNum, tunnelPacketType, &savePclId);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPclIdGet");

    rc = prvTgfTtiPclIdSet(
        prvTgfDevNum, tunnelPacketType, PRV_TGF_TTI_ID_CNS);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPclIdSet");

    rc = prvTgfTtiRule2Set(
        PRV_TGF_TTI_RULE_IDX_CNS, tunnelPacketType,
        &ttiPattern, &ttiMask, &ttiAction);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiRule2Set");

    prvTgfPclTunnelTermForceVlanModeEnableSet(GT_FALSE);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclTunnelTermForceVlanModeEnableSet");

    return GT_OK;
}

/**
* @internal prvTgfPclUdbOnlyKeysTtiTunnelTermConfigReset function
* @endinternal
*
* @brief   invalidate TTI Basic rule
*
* @param[in] tunnelPacketType         - Tunnel Packet Type (to enable lookup)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvTgfPclUdbOnlyKeysTtiTunnelTermConfigReset
(
    IN PRV_TGF_TTI_KEY_TYPE_ENT       tunnelPacketType
)
{
    GT_STATUS                rc, rc1 = GT_OK;

    PRV_UTF_LOG0_MAC("======= Resetting TTI Configuration =======\n");

    /* AUTODOC: Invalidate the TTI Rule */

    /* AUTODOC: disable the TTI lookup for given packet type at the port */
    rc = prvTgfTtiPortLookupEnableSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        tunnelPacketType, GT_FALSE);

    /* restore PCL id */
    rc = prvTgfTtiPclIdSet(
        prvTgfDevNum, tunnelPacketType, savePclId);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPclIdSet");

    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_RULE_IDX_CNS, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfTtiRuleValidStatusSet");

    rc = prvTgfPclTunnelTermForceVlanModeEnableSet(GT_TRUE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclTunnelTermForceVlanModeEnableSet");

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);

    return rc1;
}

/**
* @internal prvTgfPclUdbOnlyKeysTtiTunnelStartConfigSet function
* @endinternal
*
* @brief   build TTI Basic rule
*
* @param[in] originalPacketType       - original Packet Type (to enable lookup)
* @param[in] passengerPacketType      - Passenger Packet Type
* @param[in] tunnelType               - tunnel Type
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvTgfPclUdbOnlyKeysTtiTunnelStartConfigSet
(
    IN PRV_TGF_TTI_KEY_TYPE_ENT          originalPacketType,
    IN PRV_TGF_TUNNEL_PASSANGER_TYPE_ENT passengerPacketType,
    IN CPSS_TUNNEL_TYPE_ENT              tunnelType
)
{
    GT_STATUS                      rc = GT_OK;
    PRV_TGF_TTI_ACTION_2_STC       ttiAction;
    PRV_TGF_TTI_RULE_UNT           ttiPattern;
    PRV_TGF_TTI_RULE_UNT           ttiMask;
    PRV_TGF_TUNNEL_START_ENTRY_UNT tunnelEntry;
    GT_ETHERADDR                   macSaAddr;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* AUTODOC: set the TTunnel/ARP Table entry */

    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));
    switch (tunnelType)
    {
        case CPSS_TUNNEL_X_OVER_MPLS_E:
            tunnelEntry.mplsCfg.tagEnable    = GT_TRUE;
            tunnelEntry.mplsCfg.vlanId       = prvTgfPacketTunnelVlanTag0Part.vid;
            cpssOsMemCpy(
                tunnelEntry.mplsCfg.macDa.arEther, prvTgfPacketTunnelL2Part.daMac, sizeof(TGF_MAC_ADDR));
            tunnelEntry.mplsCfg.numLabels    = 1;
            tunnelEntry.mplsCfg.ttl          = prvTgfPacketTunnelMplsLabel0.timeToLive;
            tunnelEntry.mplsCfg.exp1         = prvTgfPacketTunnelMplsLabel0.exp;
            tunnelEntry.mplsCfg.label1       = prvTgfPacketTunnelMplsLabel0.label;
            break;
        case CPSS_TUNNEL_X_OVER_IPV4_E:
        case CPSS_TUNNEL_X_OVER_GRE_IPV4_E:
        case CPSS_TUNNEL_GENERIC_IPV4_E:
            tunnelEntry.ipv4Cfg.tagEnable        = GT_TRUE;
            tunnelEntry.ipv4Cfg.vlanId           = prvTgfPacketTunnelVlanTag0Part.vid;
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

            /* tunnel next hop MAC DA, IP DA, IP SA */
            cpssOsMemCpy(
                tunnelEntry.ipv4Cfg.macDa.arEther,
                prvTgfPacketTunnelL2Part.daMac, sizeof(TGF_MAC_ADDR));
            cpssOsMemCpy(
                tunnelEntry.ipv4Cfg.destIp.arIP,
                prvTgfPacketTunnelIpv4OtherHeaderPart.dstAddr, sizeof(TGF_IPV4_ADDR));
            cpssOsMemCpy(
                tunnelEntry.ipv4Cfg.srcIp.arIP,
                prvTgfPacketTunnelIpv4OtherHeaderPart.srcAddr, sizeof(TGF_IPV4_ADDR));
            break;
        default:
            rc = GT_BAD_PARAM;
            PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "not supported tunnel type");
            return rc;
    }

    rc = prvTgfTunnelStartEntrySet(
        PRV_TGF_ARP_ENTRY_IDX_CNS, tunnelType, &tunnelEntry);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTunnelStartEntrySet");

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for TCAM location */

    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));


    /* AUTODOC: set TTI Action */
    /* AUTODOC:   cmd FORWARD, given passenger type */
    ttiAction.command                           = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction.redirectCommand                   = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ttiAction.egressInterface.type              = CPSS_INTERFACE_PORT_E;
    ttiAction.egressInterface.devPort.hwDevNum    = prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
    ttiAction.egressInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
    ttiAction.bridgeBypass                      = GT_TRUE;
    ttiAction.tunnelStart                       = GT_TRUE;
    ttiAction.tsPassengerPacketType             = passengerPacketType;
    ttiAction.tunnelStartPtr                    = PRV_TGF_ARP_ENTRY_IDX_CNS;
    ttiAction.tag0VlanCmd                       = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
    ttiAction.tag0VlanPrecedence                = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    ttiAction.keepPreviousQoS                   = GT_TRUE;
    ttiAction.mplsCommand                       = PRV_TGF_TTI_MPLS_NOP_COMMAND_E;
    ttiAction.mplsLabel                         = prvTgfPacketTunnelMplsLabel0.label;
    ttiAction.mplsTtl                           = prvTgfPacketTunnelMplsLabel0.timeToLive;


    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action for MPLS TCAM location */
    /* AUTODOC: rule is empty and matches all packets */
    switch (originalPacketType)
    {
        case PRV_TGF_TTI_KEY_MPLS_E:
            ttiMask.mpls.common.pclId    = 0x3FF;
            ttiPattern.mpls.common.pclId = PRV_TGF_TTI_ID_CNS;
            break;
        case PRV_TGF_TTI_KEY_IPV4_E:
            ttiMask.ipv4.common.pclId    = 0x3FF;
            ttiPattern.ipv4.common.pclId = PRV_TGF_TTI_ID_CNS;
            break;
        case PRV_TGF_TTI_KEY_ETH_E:
            ttiMask.eth.common.pclId    = 0x3FF;
            ttiPattern.eth.common.pclId = PRV_TGF_TTI_ID_CNS;
            break;
        case PRV_TGF_TTI_KEY_MIM_E:
            ttiMask.mim.common.pclId    = 0x3FF;
            ttiPattern.mim.common.pclId = PRV_TGF_TTI_ID_CNS;
            break;
        default: break;
    }

    /* AUTODOC: enable the TTI lookup for given packet type at the port */
    rc = prvTgfTtiPortLookupEnableSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        originalPacketType, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPortLookupEnableSet");

    /* save PCL id */
    rc = prvTgfTtiPclIdGet(
        prvTgfDevNum, originalPacketType, &savePclId);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPclIdGet");

    rc = prvTgfTtiPclIdSet(
        prvTgfDevNum, originalPacketType, PRV_TGF_TTI_ID_CNS);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPclIdSet");

    rc = prvTgfTtiRule2Set(
        PRV_TGF_TTI_RULE_IDX_CNS, originalPacketType,
        &ttiPattern, &ttiMask, &ttiAction);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiRule2Set");

    prvTgfPclEgressTunnelStartPacketsCfgTabAccessModeSet(
        PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclEgressTunnelStartPacketsCfgTabAccessModeSet");

    /* AUTODOC: set MAC_SA of tunnel header */

    macSaAddr.arEther[0] =  prvTgfPacketTunnelL2Part.saMac[0];
    macSaAddr.arEther[1] =  prvTgfPacketTunnelL2Part.saMac[1];
    macSaAddr.arEther[2] =  prvTgfPacketTunnelL2Part.saMac[2];
    macSaAddr.arEther[3] =  prvTgfPacketTunnelL2Part.saMac[3];
    macSaAddr.arEther[4] =  prvTgfPacketTunnelL2Part.saMac[4];
    macSaAddr.arEther[5] =  prvTgfPacketTunnelL2Part.saMac[5];

    rc = prvTgfIpRouterPortGlobalMacSaIndexGet(
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        &origRouterMacSaIndex);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfIpRouterPortGlobalMacSaIndexGet");

    rc = prvTgfIpRouterPortGlobalMacSaIndexSet(
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        PRV_TGF_PCL_ROUTER_MAC_SA_INDEX_CNS);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfIpRouterPortGlobalMacSaIndexSet");

    rc = prvTgfIpRouterGlobalMacSaGet(
        prvTgfDevNum,
        PRV_TGF_PCL_ROUTER_MAC_SA_INDEX_CNS,
        &origMacSaAddr);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfIpRouterGlobalMacSaGet");

    rc = prvTgfIpRouterGlobalMacSaSet(
        prvTgfDevNum,
        PRV_TGF_PCL_ROUTER_MAC_SA_INDEX_CNS,
        &macSaAddr);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfIpRouterGlobalMacSaSet");

    return GT_OK;
}

/**
* @internal prvTgfPclUdbOnlyKeysTtiTunnelStartConfigReset function
* @endinternal
*
* @brief   invalidate TTI Basic rule
*
* @param[in] originalPacketType       - origina Packet Type (to enable lookup)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS prvTgfPclUdbOnlyKeysTtiTunnelStartConfigReset
(
    IN PRV_TGF_TTI_KEY_TYPE_ENT          originalPacketType
)
{
    GT_STATUS                      rc, rc1 = GT_OK;
    PRV_TGF_TUNNEL_START_ENTRY_UNT tunnelEntry;

    PRV_UTF_LOG0_MAC("======= Resetting TTI Configuration =======\n");

    /* AUTODOC: Invalidate the TTI Rule */

    /* AUTODOC: disable the TTI lookup for given packet type at the port */
    rc = prvTgfTtiPortLookupEnableSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        originalPacketType, GT_FALSE);

    /* restore PCL id */
    rc = prvTgfTtiPclIdSet(
        prvTgfDevNum, originalPacketType, savePclId);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfTtiPclIdSet");

    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_RULE_IDX_CNS, GT_FALSE);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfTtiRuleValidStatusSet");

    prvTgfPclEgressTunnelStartPacketsCfgTabAccessModeSet(
        PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_VLAN_E);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfPclEgressTunnelStartPacketsCfgTabAccessModeSet");

    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));
    rc = prvTgfTunnelStartEntrySet(
        PRV_TGF_ARP_ENTRY_IDX_CNS, CPSS_TUNNEL_X_OVER_IPV4_E, &tunnelEntry);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfTunnelStartEntrySet");

    rc = prvTgfIpRouterPortGlobalMacSaIndexSet(
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        origRouterMacSaIndex);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfIpRouterPortGlobalMacSaIndexSet");

    rc = prvTgfIpRouterGlobalMacSaSet(
        prvTgfDevNum,
        PRV_TGF_PCL_ROUTER_MAC_SA_INDEX_CNS,
        &origMacSaAddr);
    PRV_UTF_VERIFY_RC1(rc, "prvTgfIpRouterGlobalMacSaSet");

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);

    return rc1;
}

/**
* @internal tgfPclUdbSelectInfoGet function
* @endinternal
*
* @brief   get info relevant to UDB only keys
*
* @param[in] lookupNum                - PCL Lookup number
* @param[in] ruleFormat               - rule Format (10/20/30..60 ingress/egress)
* @param[out] egressPtr               - (pointer to) is egress rule format
* @param[out] supportUdbSelectPtr     - (pointer to) is the lookup support Udb Select
* @param[out] sizePtr                 - (pointer to) the size of the key (10/20/30..60)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong ruleFormat
*/
GT_STATUS   tgfPclUdbSelectInfoGet
(
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    OUT GT_BOOL     *egressPtr,
    OUT GT_BOOL     *supportUdbSelectPtr,
    OUT GT_U32      *sizePtr
)
{
    *egressPtr = GT_FALSE;
    *supportUdbSelectPtr = GT_TRUE;
    switch (ruleFormat)
    {
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_60_NO_FIXED_FIELDS_E:
            *sizePtr = 60;
            break;

        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_20_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_30_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_40_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_50_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_60_E:
        case PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_80_E:
            *sizePtr = ((ruleFormat-PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E)+1) * 10;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E:
        case PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_20_E:
        case PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_30_E:
        case PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_40_E:
        case PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_50_E:
        case PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_60_E:
            *sizePtr = ((ruleFormat-PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E)+1) * 10;
            *egressPtr = GT_TRUE;
            break;
        default:
            return GT_BAD_PARAM;
    }

    if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E &&
        ((*egressPtr) || lookupNum == CPSS_PCL_LOOKUP_NUMBER_1_E))
    {
        *supportUdbSelectPtr = GT_FALSE;
    }

    return GT_OK;
}

/**
* @internal prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectSet function
* @endinternal
*
* @brief   Set UDB Only PCL Key UDB Range selection
*         Replaced UDB positions ommited.
* @param[in] lookupNum                - PCL Lookup number
* @param[in] packetType               - packet Type
* @param[in] ruleFormat               - rule Format
* @param[in] udbReplaceBitmap         - bitmap of replace UDBs (12 bits ingress, 2 bits egress)
* @param[in] udbAmount                - udb Amount
* @param[in] udbIndexBase             - udb Index Base
*                                      udbOffsetIncrement  - udb Offset Increment (signed value, typical 1 or -1)
*                                       None
*/
GT_STATUS prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectSet
(
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN GT_U32                           udbReplaceBitmap,
    IN GT_U32                           udbAmount,
    IN GT_U32                           udbIndexBase,
    IN GT_32                            udbIndexIncrement
)
{
    GT_STATUS                           rc;
    GT_U32                              i;
    GT_U32                              udbInKeyMapped;
    PRV_TGF_PCL_UDB_SELECT_STC          udbSelect;
    GT_BOOL                             egress;
    GT_BOOL                             supportUdbSelect;
    GT_U32                              size;
    GT_U32                              ingressRepBmpSize;

    ingressRepBmpSize = 
        (sizeof(udbSelect.ingrUdbReplaceArr) / sizeof(udbSelect.ingrUdbReplaceArr[0]));

    cpssOsMemSet(&udbSelect, 0, sizeof(udbSelect));

    udbInKeyMapped = 0;

    rc = tgfPclUdbSelectInfoGet(lookupNum,ruleFormat,&egress,&supportUdbSelect,&size);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "tgfPclUdbSelectInfoGet");

    for (i = 0; (i < 32); i++)
    {
        if (udbReplaceBitmap & (1 << i))
        {
            if (egress == GT_FALSE)
            {
                if (i < ingressRepBmpSize) udbSelect.ingrUdbReplaceArr[i] = GT_TRUE;
            }
            else
            {
                if (i == 0) udbSelect.egrUdb01Replace      = GT_TRUE;
                if (i == 1) udbSelect.egrUdbBit15Replace   = GT_TRUE;
            }
        }
    }

    for (i = 0; (i < udbAmount); i++)
    {
        if ((udbReplaceBitmap & (1 << i)) == 0)
        {
            udbSelect.udbSelectArr[i] =
                udbIndexBase + (udbIndexIncrement * udbInKeyMapped);
            udbInKeyMapped ++;
        }

        if(supportUdbSelect == GT_FALSE)
        {
            /* the device supports only 1:1 mapping */
            udbSelect.udbSelectArr[i] = i;
        }
    }

    if(supportUdbSelect == GT_FALSE && size > udbAmount)
    {
        for (i = 0; (i < size); i++)
        {
            /* the device supports only 1:1 mapping */
            udbSelect.udbSelectArr[i] = i;
        }
    }


    rc = prvTgfPclUserDefinedBytesSelectSet(
        ruleFormat, packetType, lookupNum,
        &udbSelect);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclUserDefinedBytesSelectSet");

    return GT_OK;
}

/**
* @internal prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectReset function
* @endinternal
*
* @brief   Reset UDB Only PCL Key UDB selection
*
* @param[in] lookupNum                - PCL Lookup number
* @param[in] packetType               - packet Type
* @param[in] ruleFormat               - rule Format
*                                       None
*/
GT_STATUS prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectReset
(
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat
)
{
    GT_STATUS                           rc;
    PRV_TGF_PCL_UDB_SELECT_STC          udbSelect;
    GT_BOOL                             egress;
    GT_BOOL                             supportUdbSelect;
    GT_U32                              size;
    GT_U32                              ii;

    cpssOsMemSet(&udbSelect, 0, sizeof(udbSelect));

    rc = tgfPclUdbSelectInfoGet(lookupNum,ruleFormat,&egress,&supportUdbSelect,&size);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "tgfPclUdbSelectInfoGet");

    if(supportUdbSelect == GT_FALSE)
    {
        for (ii = 0; ii < size; ii++)
        {
            /* the device supports only 1:1 mapping */
            udbSelect.udbSelectArr[ii] = ii;
        }
    }

    rc = prvTgfPclUserDefinedBytesSelectSet(
        ruleFormat, packetType, lookupNum,
        &udbSelect);
    PRV_UTF_VERIFY_GT_OK_LOG_MAC(rc, "prvTgfPclUserDefinedBytesSelectSet");

    return GT_OK;
}

/**
* @internal prvTgfPclUdbOnlyKeysUdbOnlyGenericTest function
* @endinternal
*
* @brief   Generic Test on UDB Only Key
*
* @param[in] direction                - PCL direction
* @param[in] lookupNum                - lookup Number
* @param[in] packetType               - packet type
* @param[in] packetTypeIndex          - 0 - not IP, 1 - IPV4, 2 - IPV6
* @param[in] ruleFormat               - rule Format
* @param[in] ruleSize                 - rule Size
* @param[in] offsetType               - offset Type
* @param[in] udbAmount                - UDB Amount
* @param[in] udbIndexLow              - The lowest UDB Index in the UDB range
* @param[in] udbOffsetLow             - UDB Offset Base of the field (the lowest byte offset)
* @param[in] udbOrderAscend           - UDB Order Ascend (GT_TRUE) or Descend (GT_FALSE)
* @param[in] packetPtr                - (pointer to) Info of the packet used in the test
* @param[in] offsetInPacket           - offset of matched data in the packet
*                                       None
*/
static GT_VOID prvTgfPclUdbOnlyKeysUdbOnlyGenericTest
(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN GT_U32                           packetTypeIndex,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN CPSS_PCL_RULE_SIZE_ENT           ruleSize,
    IN PRV_TGF_PCL_OFFSET_TYPE_ENT      offsetType,
    IN GT_U32                           udbAmount,
    IN GT_U32                           udbIndexLow,
    IN GT_U32                           udbOffsetLow,
    IN GT_BOOL                          udbOrderAscend,
    IN TGF_PACKET_STC                   *packetPtr,
    IN GT_U32                           offsetInPacket
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    GT_U8                            *udbMaskPtr;
    GT_U8                            *udbPatternPtr;
    GT_U32                           i;
    GT_U32                           udbCfgIndexBase;
    GT_32                            udbCfgOffsetIncrement;
    GT_U32                           udbCfgOffsetBase;
    GT_U32                           udbSelIndexBase;
    GT_32                            udbSelIndexIncrement;
    static GT_U8                     packetBuffer[256];
    GT_U32                           numOfBytesInBuff;
    GT_U8                            *patternPtr;
    TGF_AUTO_CHECKSUM_FULL_INFO_STC  checksumInfoArr[TGF_AUTO_CHECKSUM_NUM_OF_HEADERS_CNS];/* auto checksum info */
    TGF_AUTO_CHECKSUM_EXTRA_INFO_STC extraChecksumInfoArr[TGF_AUTO_CHECKSUM_NUM_OF_HEADERS_CNS];/* extra checksum info */

    if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E &&
       ((direction == CPSS_PCL_DIRECTION_INGRESS_E && lookupNum == CPSS_PCL_LOOKUP_0_1_E) ||
        (direction == CPSS_PCL_DIRECTION_EGRESS_E)))
    {
        /* IPCL1/EPCL hold no UDB select and must be 1:1 */
        udbIndexLow  = 0;
        udbOrderAscend = GT_TRUE;
    }

    udbCfgIndexBase       = udbIndexLow;
    if (udbOrderAscend == GT_TRUE)
    {
        udbCfgOffsetBase      = udbOffsetLow;
        udbCfgOffsetIncrement = 1;
        udbSelIndexBase       = udbIndexLow;
        udbSelIndexIncrement  = 1;
    }
    else
    {
        /* assumed udbOrderAscend == GT_FALSE */
        udbCfgOffsetBase      = udbOffsetLow + udbAmount - 1;
        udbCfgOffsetIncrement = -1;
        udbSelIndexBase       = udbIndexLow + udbAmount - 1;
        udbSelIndexIncrement  = -1;
    }

    numOfBytesInBuff = sizeof(packetBuffer);
    cpssOsMemSet(checksumInfoArr , 0, sizeof(checksumInfoArr));
    cpssOsMemSet(extraChecksumInfoArr , 0, sizeof(extraChecksumInfoArr));

    rc = tgfTrafficEnginePacketBuild(
        packetPtr, packetBuffer, &numOfBytesInBuff,
        checksumInfoArr, extraChecksumInfoArr);
    if (rc != GT_OK) goto label_restore;

    /* apply auto checksum fields */
    rc = tgfTrafficGeneratorPacketChecksumUpdate(
        checksumInfoArr, extraChecksumInfoArr);
    if (rc != GT_OK) goto label_restore;

    patternPtr = &(packetBuffer[offsetInPacket]);

    /* AUTODOC: Set Vlan And FDB configuration */
    rc = prvTgfPclUdbOnlyKeysPortVlanFdbSet();
    if (rc != GT_OK) goto label_restore;

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    if (direction != CPSS_PCL_DIRECTION_EGRESS_E)
    {
        udbMaskPtr    = mask.ruleIngrUdbOnly.udb;
        udbPatternPtr = pattern.ruleIngrUdbOnly.udb;
    }
    else
    {
        udbMaskPtr    = mask.ruleEgrUdbOnly.udb;
        udbPatternPtr = pattern.ruleEgrUdbOnly.udb;
    }

    for (i = 0; (i < udbAmount); i++)
    {
        udbMaskPtr[i]    = 0xFF;
        udbPatternPtr[i] = patternPtr[i];
    }

    rc = prvTgfPclUdbOnlyKeysPclLegacyCfgSet(
        direction,
        lookupNum,
        packetTypeIndex, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
        ruleFormat,
        &mask, &pattern);
    if (rc != GT_OK) goto label_restore;

    rc = prvTgfPclUdbOnlyKeysPclUdbRangeCfgSet(
        direction,
        packetType,
        offsetType,
        udbCfgIndexBase,
        udbAmount,
        udbCfgOffsetBase,
        udbCfgOffsetIncrement);
    if (rc != GT_OK) goto label_restore;

    rc = prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectSet(
        lookupNum, packetType, ruleFormat,
        0 /*udbReplaceBitmap*/,
        udbAmount,
        udbSelIndexBase,
        udbSelIndexIncrement);
    if (rc != GT_OK) goto label_restore;

    /* AUTODOC: Double Tagged Packet */
    prvTgfPclUdbOnlyKeysVidTrafficGenerate(packetPtr);

    /* AUTODOC: Check that TAG0 VID is modified */
    prvTgfPclUdbOnlyKeysTrafficEgressVidCheck(PRV_TGF_PCL_NEW_VLAN_ID_CNS, GT_TRUE/*check 16 bit*/);

    label_restore:

    /* AUTODOC: Restore Vlan And FDB configuration */
    prvTgfPclUdbOnlyKeysPortVlanFdbReset();

    prvTgfPclUdbOnlyKeysPclLegacyCfgReset(
        direction,
        lookupNum,
        ruleSize);

    prvTgfPclUdbOnlyKeysPclUdbRangeCfgReset(
        direction,
        packetType,
        udbCfgIndexBase,
        udbAmount);

    prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectReset(
        lookupNum, packetType, ruleFormat);
}

/**
* @internal prvTgfPclUdbOnlyKeysIngressMetadataTest function
* @endinternal
*
* @brief   Test on Ingress PCL metadata
*
* @note Metadata[207:192] - Ethernet type
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngressMetadataTest
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;

    /* AUTODOC: Set Vlan And FDB configuration */
    rc = prvTgfPclUdbOnlyKeysPortVlanFdbSet();
    if (rc != GT_OK) goto label_restore;

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    /* UDB 40-41 */
    mask.ruleExtNotIpv6.udb[40] = 0xFF;
    mask.ruleExtNotIpv6.udb[41] = 0xFF;
    pattern.ruleExtNotIpv6.udb[40] =
        (GT_U8)(prvTgfPacketEthOth1EthertypePart.etherType & 0xFF);
    pattern.ruleExtNotIpv6.udb[41] =
        (GT_U8)((prvTgfPacketEthOth1EthertypePart.etherType >> 8) & 0xFF);

    rc = prvTgfPclUdbOnlyKeysPclLegacyCfgSet(
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_0_E,
        0, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E,
        &mask, &pattern);
    if (rc != GT_OK) goto label_restore;

    rc = prvTgfPclUdbOnlyKeysPclUdbRangeCfgSet(
        CPSS_PCL_DIRECTION_INGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
        PRV_TGF_PCL_OFFSET_METADATA_E,
        40 /*udbIndexBase*/,
        2  /*udbAmount*/,
        24 /*udbOffsetBase*/,
        1 /*udbOffsetIncrement*/);
    if (rc != GT_OK) goto label_restore;

    /* AUTODOC: Send Double Tagged Ethernet Other Packet without Tunnel header */
    prvTgfPclUdbOnlyKeysVidTrafficGenerate(&prvTgfPacketNotTunneled2tagsEthOthInfo);

    /* AUTODOC: Check that TAG0 VID is modified */
    prvTgfPclUdbOnlyKeysTrafficEgressVidCheck(PRV_TGF_PCL_NEW_VLAN_ID_CNS, GT_TRUE/*check 16 bit*/);

    label_restore:

    /* AUTODOC: Restore Vlan And FDB configuration */
    prvTgfPclUdbOnlyKeysPortVlanFdbReset();

    prvTgfPclUdbOnlyKeysPclLegacyCfgReset(
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_0_E,
        CPSS_PCL_RULE_SIZE_EXT_E);

    prvTgfPclUdbOnlyKeysPclUdbRangeCfgReset(
        CPSS_PCL_DIRECTION_INGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
        40 /*udbIndexBase*/,
        2  /*udbAmount*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysEgressMetadataTest function
* @endinternal
*
* @brief   Test on Egress PCL metadata
*
* @note Metadata[207:196] - Original VID
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgressMetadataTest
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;

    /* AUTODOC: Set Vlan And FDB configuration */
    rc = prvTgfPclUdbOnlyKeysPortVlanFdbSet();
    if (rc != GT_OK) goto label_restore;

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    /* UDB 30-31 */
    mask.ruleEgrExtIpv6L4.udb[30] = 0xF0;
    mask.ruleEgrExtIpv6L4.udb[31] = 0xFF;
    pattern.ruleEgrExtIpv6L4.udb[30] =
        (GT_U8)((prvTgfPacketPassengerVlanTag0Part.vid << 4) & 0xF0);
    pattern.ruleEgrExtIpv6L4.udb[31] =
        (GT_U8)((prvTgfPacketPassengerVlanTag0Part.vid >> 4) & 0xFF);

    rc = prvTgfPclUdbOnlyKeysPclLegacyCfgSet(
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        2, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E,
        &mask, &pattern);
    if (rc != GT_OK) goto label_restore;

    rc = prvTgfPclUdbOnlyKeysPclUdbRangeCfgSet(
        CPSS_PCL_DIRECTION_EGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV6_E,
        PRV_TGF_PCL_OFFSET_METADATA_E,
        30 /*udbIndexBase*/,
        2  /*udbAmount*/,
        24 /*udbOffsetBase*/,
        1 /*udbOffsetIncrement*/);
    if (rc != GT_OK) goto label_restore;

    /* AUTODOC: Send Double Tagged IPV6 Other Packet without Tunnel header */
    prvTgfPclUdbOnlyKeysVidTrafficGenerate(&prvTgfPacketNotTunneled2tagsIpv6OthInfo);

    /* AUTODOC: Check that TAG0 VID is modified */
    prvTgfPclUdbOnlyKeysTrafficEgressVidCheck(PRV_TGF_PCL_NEW_VLAN_ID_CNS, GT_TRUE/*check 16 bit*/);

    label_restore:

    /* AUTODOC: Restore Vlan And FDB configuration */
    prvTgfPclUdbOnlyKeysPortVlanFdbReset();

    prvTgfPclUdbOnlyKeysPclLegacyCfgReset(
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_0_E,
        CPSS_PCL_RULE_SIZE_EXT_E);

    prvTgfPclUdbOnlyKeysPclUdbRangeCfgReset(
        CPSS_PCL_DIRECTION_EGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV6_E,
        30 /*udbIndexBase*/,
        2  /*udbAmount*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysIngressTunnelL2Test function
* @endinternal
*
* @brief   Test on Ingress PCL Tunnel L2 offset type
*
* @note Tunnel Header MAC_SA
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngressTunnelL2Test
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;

    /* AUTODOC: Set Vlan And FDB configuration */
    rc = prvTgfPclUdbOnlyKeysPortVlanFdbSet();
    if (rc != GT_OK) goto label_restore;

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    /* UDB 20-22, 31-33 */
    mask.ruleStdIpv4L4.udb[20]         = 0xFF;
    mask.ruleStdIpv4L4.udb[21]         = 0xFF;
    mask.ruleStdIpv4L4.udb[22]         = 0xFF;
    mask.ruleStdIpv4L4.udb[31]    = 0xFF;
    mask.ruleStdIpv4L4.udb[32]    = 0xFF;
    mask.ruleStdIpv4L4.udb[33]    = 0xFF;

    pattern.ruleStdIpv4L4.udb[20]      = prvTgfPacketTunnelL2Part.saMac[0];
    pattern.ruleStdIpv4L4.udb[21]      = prvTgfPacketTunnelL2Part.saMac[1];
    pattern.ruleStdIpv4L4.udb[22]      = prvTgfPacketTunnelL2Part.saMac[2];
    pattern.ruleStdIpv4L4.udb[31] = prvTgfPacketTunnelL2Part.saMac[3];
    pattern.ruleStdIpv4L4.udb[32] = prvTgfPacketTunnelL2Part.saMac[4];
    pattern.ruleStdIpv4L4.udb[33] = prvTgfPacketTunnelL2Part.saMac[5];

    rc = prvTgfPclUdbOnlyKeysPclLegacyCfgSet(
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_0_E,
        1, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
        &mask, &pattern);
    if (rc != GT_OK) goto label_restore;

    rc = prvTgfPclUdbOnlyKeysPclUdbRangeCfgSet(
        CPSS_PCL_DIRECTION_INGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
        PRV_TGF_PCL_OFFSET_TUNNEL_L2_E,
        20 /*udbIndexBase*/,
        3  /*udbAmount*/,
        6 /*udbOffsetBase*/,
        1 /*udbOffsetIncrement*/);
    if (rc != GT_OK) goto label_restore;

    rc = prvTgfPclUdbOnlyKeysPclUdbRangeCfgSet(
        CPSS_PCL_DIRECTION_INGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
        PRV_TGF_PCL_OFFSET_TUNNEL_L2_E,
        31 /*udbIndexBase*/,
        3  /*udbAmount*/,
        9 /*udbOffsetBase*/,
        1 /*udbOffsetIncrement*/);
    if (rc != GT_OK) goto label_restore;

    rc = prvTgfPclUdbOnlyKeysTtiTunnelTermConfigSet(
        PRV_TGF_TTI_KEY_MPLS_E,
        PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E);
    if (rc != GT_OK) goto label_restore;

    /* AUTODOC: Send Double Tagged IPV4 Other Packet with MPLS Tunnel header */
    prvTgfPclUdbOnlyKeysVidTrafficGenerate(&prvTgfPacketMplsTunneled2tagsIpv4OthInfo);

    /* AUTODOC: Check that TAG0 VID is modified */
    prvTgfPclUdbOnlyKeysTrafficEgressVidCheck(PRV_TGF_PCL_NEW_VLAN_ID_CNS, GT_TRUE/*check 16 bit*/);

    label_restore:

    /* AUTODOC: Restore Vlan And FDB configuration */
    prvTgfPclUdbOnlyKeysPortVlanFdbReset();

    prvTgfPclUdbOnlyKeysPclLegacyCfgReset(
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_0_E,
        CPSS_PCL_RULE_SIZE_STD_E);

    prvTgfPclUdbOnlyKeysPclUdbRangeCfgReset(
        CPSS_PCL_DIRECTION_INGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
        20 /*udbIndexBase*/,
        3  /*udbAmount*/);

    prvTgfPclUdbOnlyKeysPclUdbRangeCfgReset(
        CPSS_PCL_DIRECTION_INGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
        31 /*udbIndexBase*/,
        3  /*udbAmount*/);

    rc = prvTgfPclUdbOnlyKeysTtiTunnelTermConfigReset(
        PRV_TGF_TTI_KEY_MPLS_E);
}

/**
* @internal prvTgfPclUdbOnlyKeysIngressTunnelL3Test function
* @endinternal
*
* @brief   Test on Ingress PCL Tunnel L3 offset type
*
* @note Tunnel Header DIP[15:0]
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngressTunnelL3Test
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;

    /* AUTODOC: Set Vlan And FDB configuration */
    rc = prvTgfPclUdbOnlyKeysPortVlanFdbSet();
    if (rc != GT_OK) goto label_restore;

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    /* UDB 18-19 */
    mask.ruleStdIpL2Qos.udb[18]         = 0xFF;
    mask.ruleStdIpL2Qos.udb[19]         = 0xFF;

    pattern.ruleStdIpL2Qos.udb[18]      =
        (GT_U8)prvTgfPacketTunnelIpv4OtherHeaderPart.dstAddr[2];
    pattern.ruleStdIpL2Qos.udb[19]      =
        (GT_U8)prvTgfPacketTunnelIpv4OtherHeaderPart.dstAddr[3];

    rc = prvTgfPclUdbOnlyKeysPclLegacyCfgSet(
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_0_E,
        2, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
        &mask, &pattern);
    if (rc != GT_OK) goto label_restore;

    rc = prvTgfPclUdbOnlyKeysPclUdbRangeCfgSet(
        CPSS_PCL_DIRECTION_INGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV6_OTHER_E,
        PRV_TGF_PCL_OFFSET_TUNNEL_L3_MINUS_2_E,
        18 /*udbIndexBase*/,
        2  /*udbAmount*/,
        20 /*udbOffsetBase*/,
        1 /*udbOffsetIncrement*/);
    if (rc != GT_OK) goto label_restore;

    rc = prvTgfPclUdbOnlyKeysTtiTunnelTermConfigSet(
        PRV_TGF_TTI_KEY_IPV4_E,
        PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E);
    if (rc != GT_OK) goto label_restore;

    /* AUTODOC: Double Tagged IPV6 Packet Tunneled L2 IPV6 Other */
    prvTgfPclUdbOnlyKeysVidTrafficGenerate(&prvTgfPacketIpv4TunneledL2Ipv6OthInfo);

    /* AUTODOC: Check that TAG0 VID is modified */
    prvTgfPclUdbOnlyKeysTrafficEgressVidCheck(PRV_TGF_PCL_NEW_VLAN_ID_CNS, GT_TRUE/*check 16 bit*/);

    label_restore:

    /* AUTODOC: Restore Vlan And FDB configuration */
    prvTgfPclUdbOnlyKeysPortVlanFdbReset();

    prvTgfPclUdbOnlyKeysPclLegacyCfgReset(
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_0_E,
        CPSS_PCL_RULE_SIZE_STD_E);

    prvTgfPclUdbOnlyKeysPclUdbRangeCfgReset(
        CPSS_PCL_DIRECTION_INGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV6_OTHER_E,
        18 /*udbIndexBase*/,
        2  /*udbAmount*/);

    rc = prvTgfPclUdbOnlyKeysTtiTunnelTermConfigReset(
        PRV_TGF_TTI_KEY_IPV4_E);
}

/**
* @internal prvTgfPclUdbOnlyKeysEgressTunnelL2Test function
* @endinternal
*
* @brief   Test on Egress PCL Tunnel L2 offset type
*
* @note Tunnel Header MAC_DA[15:0]
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgressTunnelL2Test
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;

    /* AUTODOC: Set Vlan And FDB configuration */
    rc = prvTgfPclUdbOnlyKeysPortVlanFdbSet();
    if (rc != GT_OK) goto label_restore;

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    /* UDB 4-5 */
    mask.ruleEgrStdIpL2Qos.udb[4]         = 0xFF;
    mask.ruleEgrStdIpL2Qos.udb[5]         = 0xFF;

    pattern.ruleEgrStdIpL2Qos.udb[4]      =
        (GT_U8)prvTgfPacketTunnelL2Part.daMac[4];
    pattern.ruleEgrStdIpL2Qos.udb[5]      =
        (GT_U8)prvTgfPacketTunnelL2Part.daMac[5];

    rc = prvTgfPclUdbOnlyKeysPclLegacyCfgSet(
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_0_E,
        1, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E,
        &mask, &pattern);
    if (rc != GT_OK) goto label_restore;

    rc = prvTgfPclUdbOnlyKeysPclUdbRangeCfgSet(
        CPSS_PCL_DIRECTION_EGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
        PRV_TGF_PCL_OFFSET_TUNNEL_L2_E,
        4 /*udbIndexBase*/,
        2  /*udbAmount*/,
        4 /*udbOffsetBase*/,
        1 /*udbOffsetIncrement*/);
    if (rc != GT_OK) goto label_restore;

    rc = prvTgfPclUdbOnlyKeysTtiTunnelStartConfigSet(
        PRV_TGF_TTI_KEY_IPV4_E,
        PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E,
        CPSS_TUNNEL_X_OVER_MPLS_E);
    if (rc != GT_OK) goto label_restore;

    /* AUTODOC: Double Tagged Packet IPV4 Other */
    prvTgfPclUdbOnlyKeysVidTrafficGenerate(&prvTgfPacketPassenger2tagsIpv4OthInfo);

    /* AUTODOC: Check that TAG0 VID is modified */
    prvTgfPclUdbOnlyKeysTrafficEgressVidCheck(PRV_TGF_PCL_NEW_VLAN_ID_CNS, GT_TRUE/*check 16 bit*/);

    label_restore:

    /* AUTODOC: Restore Vlan And FDB configuration */
    prvTgfPclUdbOnlyKeysPortVlanFdbReset();

    prvTgfPclUdbOnlyKeysPclLegacyCfgReset(
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_0_E,
        CPSS_PCL_RULE_SIZE_STD_E);

    prvTgfPclUdbOnlyKeysPclUdbRangeCfgReset(
        CPSS_PCL_DIRECTION_EGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
        4 /*udbIndexBase*/,
        2  /*udbAmount*/);

    rc = prvTgfPclUdbOnlyKeysTtiTunnelStartConfigReset(
        PRV_TGF_TTI_KEY_IPV4_E);
}

/**
* @internal prvTgfPclUdbOnlyKeysEgressTunnelL3Minus2Test function
* @endinternal
*
* @brief   Test on Egress PCL Tunnel L3 Minus 2 offset type
*
* @note Tunnel Header SIP[31:0]
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgressTunnelL3Minus2Test
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;

    /* AUTODOC: Set Vlan And FDB configuration */
    rc = prvTgfPclUdbOnlyKeysPortVlanFdbSet();
    if (rc != GT_OK) goto label_restore;

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    /* UDB 20-23 */
    mask.ruleEgrExtIpv6L2.udb[20]         = 0xFF;
    mask.ruleEgrExtIpv6L2.udb[21]         = 0xFF;
    mask.ruleEgrExtIpv6L2.udb[22]         = 0xFF;
    mask.ruleEgrExtIpv6L2.udb[23]         = 0xFF;

    pattern.ruleEgrExtIpv6L2.udb[20]      =
        (GT_U8)prvTgfPacketTunnelIpv4OtherHeaderPart.srcAddr[0];
    pattern.ruleEgrExtIpv6L2.udb[21]      =
        (GT_U8)prvTgfPacketTunnelIpv4OtherHeaderPart.srcAddr[1];
    pattern.ruleEgrExtIpv6L2.udb[22]      =
        (GT_U8)prvTgfPacketTunnelIpv4OtherHeaderPart.srcAddr[2];
    pattern.ruleEgrExtIpv6L2.udb[23]      =
        (GT_U8)prvTgfPacketTunnelIpv4OtherHeaderPart.srcAddr[3];

    rc = prvTgfPclUdbOnlyKeysPclLegacyCfgSet(
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_0_E,
        2, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E,
        &mask, &pattern);
    if (rc != GT_OK) goto label_restore;

    rc = prvTgfPclUdbOnlyKeysPclUdbRangeCfgSet(
        CPSS_PCL_DIRECTION_EGRESS_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV6_OTHER_E,
        PRV_TGF_PCL_OFFSET_TUNNEL_L3_MINUS_2_E,
        20 /*udbIndexBase*/,
        4  /*udbAmount*/,
        14 /*udbOffsetBase*/,
        1 /*udbOffsetIncrement*/);
    if (rc != GT_OK) goto label_restore;

    rc = prvTgfPclUdbOnlyKeysTtiTunnelStartConfigSet(
        PRV_TGF_TTI_KEY_ETH_E,
        PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E,
        CPSS_TUNNEL_GENERIC_IPV4_E);
    if (rc != GT_OK) goto label_restore;

    /* AUTODOC: Double Tagged IPV6 Packet Tunneled L2 IPV6 Other */
    prvTgfPclUdbOnlyKeysVidTrafficGenerate(&prvTgfPacketPassengerIpv6OthInfo);

    /* AUTODOC: Check that TAG0 VID is modified */
    prvTgfPclUdbOnlyKeysTrafficEgressVidCheck(PRV_TGF_PCL_NEW_VLAN_ID_CNS, GT_TRUE/*check 16 bit*/);

    label_restore:

    /* AUTODOC: Restore Vlan And FDB configuration */
    prvTgfPclUdbOnlyKeysPortVlanFdbReset();

    prvTgfPclUdbOnlyKeysPclLegacyCfgReset(
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_0_E,
        CPSS_PCL_RULE_SIZE_EXT_E);

    prvTgfPclUdbOnlyKeysPclUdbRangeCfgReset(
        CPSS_PCL_DIRECTION_EGRESS_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E,
        20 /*udbIndexBase*/,
        4  /*udbAmount*/);

    rc = prvTgfPclUdbOnlyKeysTtiTunnelStartConfigReset(
        PRV_TGF_TTI_KEY_ETH_E);
}

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb10Ipv4UdpL4Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB10 L4 offset type on IPV4 UDP packet
*
* @note 10 first bytes from UDP header beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb10Ipv4UdpL4Test
(
    GT_VOID
)
{
    GT_U32 offsetInPacket =
        (TGF_L2_HEADER_SIZE_CNS
         + (2 * TGF_VLAN_TAG_SIZE_CNS)
         + TGF_ETHERTYPE_SIZE_CNS
         + TGF_IPV4_HEADER_SIZE_CNS);

    prvTgfPclUdbOnlyKeysUdbOnlyGenericTest(
        CPSS_PCL_DIRECTION_INGRESS_E               /*direction*/,
        CPSS_PCL_LOOKUP_0_0_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E         /*packetType*/,
        1 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E   /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_10_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_L4_E                    /*offsetType*/,
        10                                         /*udbAmount*/,
        40                                         /*udbIndexLow*/,
        0                                          /*udbOffsetLow*/,
        GT_TRUE                                    /*udbOrderAscend*/,
        &prvTgfPacketPassengerIpv4UdpInfo          /*packetPtr*/,
        offsetInPacket                             /*offsetInPacket*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb10Ipv4TcpL4Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB10 L4 offset type on IPV4 TCP packet
*
* @note 10 first bytes from TCP header beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb10Ipv4TcpL4Test
(
    GT_VOID
)
{
    GT_U32 offsetInPacket =
        (TGF_L2_HEADER_SIZE_CNS
         + (2 * TGF_VLAN_TAG_SIZE_CNS)
         + TGF_ETHERTYPE_SIZE_CNS
         + TGF_IPV4_HEADER_SIZE_CNS);

    prvTgfPclUdbOnlyKeysUdbOnlyGenericTest(
        CPSS_PCL_DIRECTION_INGRESS_E               /*direction*/,
        CPSS_PCL_LOOKUP_0_0_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_TCP_E         /*packetType*/,
        1 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_10_E   /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_10_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_L4_E                    /*offsetType*/,
        10                                         /*udbAmount*/,
        40                                         /*udbIndexLow*/,
        0                                          /*udbOffsetLow*/,
        GT_TRUE                                    /*udbOrderAscend*/,
        &prvTgfPacketPassenger2tagsIpv4TcpInfo     /*packetPtr*/,
        offsetInPacket                             /*offsetInPacket*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb20Ipv6UdpL4Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB20 L4 offset type on IPV6 UDP packet
*
* @note 20 first bytes from UDP header beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb20Ipv6UdpL4Test
(
    GT_VOID
)
{
    GT_U32 offsetInPacket =
        (TGF_L2_HEADER_SIZE_CNS
         + (2 * TGF_VLAN_TAG_SIZE_CNS)
         + TGF_ETHERTYPE_SIZE_CNS
         + TGF_IPV6_HEADER_SIZE_CNS);

    prvTgfPclUdbOnlyKeysUdbOnlyGenericTest(
        CPSS_PCL_DIRECTION_INGRESS_E               /*direction*/,
        CPSS_PCL_LOOKUP_0_1_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_IPV6_UDP_E         /*packetType*/,
        2 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_20_E   /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_20_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_L4_E                    /*offsetType*/,
        20                                         /*udbAmount*/,
        20                                         /*udbIndexLow*/,
        0                                          /*udbOffsetLow*/,
        GT_FALSE                                   /*udbOrderAscend*/,
        &prvTgfPacketPassengerIpv6UdpInfo          /*packetPtr*/,
        offsetInPacket                             /*offsetInPacket*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb30MplsEthMplsMinus2Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB30 MplsMinus2 offset type on Ethernet over MPLS packet
*
* @note 20 first bytes from MPLS Ethernet Type beginning.
*       MPLS offset supported by HW only up to 20 bytes
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb30MplsEthMplsMinus2Test
(
    GT_VOID
)
{
    GT_U32 offsetInPacket =
        (TGF_L2_HEADER_SIZE_CNS
         + (2 * TGF_VLAN_TAG_SIZE_CNS));

    prvTgfPclUdbOnlyKeysUdbOnlyGenericTest(
        CPSS_PCL_DIRECTION_INGRESS_E               /*direction*/,
        CPSS_PCL_LOOKUP_0_1_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_MPLS_E             /*packetType*/,
        0 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_30_E   /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_30_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_MPLS_MINUS_2_E          /*offsetType*/,
        20                                         /*udbAmount*/,
        20                                         /*udbIndexLow*/,
        0                                          /*udbOffsetLow*/,
        GT_TRUE                                    /*udbOrderAscend*/,
        &prvTgfPacketMplsTunneled2tagsEthOthInfo   /*packetPtr*/,
        offsetInPacket                             /*offsetInPacket*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb40EthOtherL2Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB00 L2 offset type on Ethernet Other packet
*
* @note 40 first bytes from L2 header beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb40EthOtherL2Test
(
    GT_VOID
)
{
    GT_U32 offsetInPacket = 0;

    prvTgfPclUdbOnlyKeysUdbOnlyGenericTest(
        CPSS_PCL_DIRECTION_INGRESS_E                /*direction*/,
        CPSS_PCL_LOOKUP_0_0_E                       /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E    /*packetType*/,
        0 /*0 - not IP, 1 - IPV4, 2 - IPV6*/        /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_40_E    /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_40_BYTES_E               /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_L2_E                     /*offsetType*/,
        40                                          /*udbAmount*/,
        10                                          /*udbIndexLow*/,
        0                                           /*udbOffsetLow*/,
        GT_TRUE                                     /*udbOrderAscend*/,
        &prvTgfPacketNotTunneled2tagsEthOthInfo     /*packetPtr*/,
        offsetInPacket                              /*offsetInPacket*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb50Ipv6OtherL3Minus2Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB50 L3Minus2 offset type on IPV6 Other packet
*
* @note 50 first bytes from IPV6 Ethernet Type beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb50Ipv6OtherL3Minus2Test
(
    GT_VOID
)
{
    GT_U32 offsetInPacket =
        (TGF_L2_HEADER_SIZE_CNS
         + (2 * TGF_VLAN_TAG_SIZE_CNS));

    prvTgfPclUdbOnlyKeysUdbOnlyGenericTest(
        CPSS_PCL_DIRECTION_INGRESS_E               /*direction*/,
        CPSS_PCL_LOOKUP_0_1_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_IPV6_OTHER_E       /*packetType*/,
        2 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_50_E   /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_50_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_L3_MINUS_2_E            /*offsetType*/,
        50                                         /*udbAmount*/,
        0                                          /*udbIndexLow*/,
        0                                          /*udbOffsetLow*/,
        GT_FALSE                                   /*udbOrderAscend*/,
        &prvTgfPacketNotTunneled2tagsIpv6OthInfo   /*packetPtr*/,
        offsetInPacket                             /*offsetInPacket*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb60Ipv4OtherL3Minus2Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB60 L3Minus2 offset type on IPV4 Other packet
*
* @note 50 first bytes from IPV4 Ethernet Type beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb60Ipv4OtherL3Minus2Test
(
    GT_VOID
)
{
    GT_U32 offsetInPacket =
        (TGF_L2_HEADER_SIZE_CNS
         + (2 * TGF_VLAN_TAG_SIZE_CNS));

    prvTgfPclUdbOnlyKeysUdbOnlyGenericTest(
        CPSS_PCL_DIRECTION_INGRESS_E               /*direction*/,
        CPSS_PCL_LOOKUP_0_1_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E       /*packetType*/,
        1 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_60_E   /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_60_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_L3_MINUS_2_E            /*offsetType*/,
        50                                         /*udbAmount*/,
        0                                          /*udbIndexLow*/,
        0                                          /*udbOffsetLow*/,
        GT_TRUE                                    /*udbOrderAscend*/,
        &prvTgfPacketPassenger2tagsIpv4OthInfo     /*packetPtr*/,
        offsetInPacket                             /*offsetInPacket*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb60NoFixedIpv4OtherL3Minus2Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB60_NO_FIXED L3Minus2 offset type on IPV4 Other
*         packet
*
* @note 60 first bytes from IPV4 Ethernet Type beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb60NoFixedIpv4OtherL3Minus2Test
(
    GT_VOID
)
{
    GT_U32 offsetInPacket =
        (TGF_L2_HEADER_SIZE_CNS
         + (2 * TGF_VLAN_TAG_SIZE_CNS));

    prvTgfPclUdbOnlyKeysUdbOnlyGenericTest(
        CPSS_PCL_DIRECTION_INGRESS_E               /*direction*/,
        CPSS_PCL_LOOKUP_0_1_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E       /*packetType*/,
        1 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_60_NO_FIXED_FIELDS_E   /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_60_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_L3_MINUS_2_E            /*offsetType*/,
        60                                         /*udbAmount*/,
        0                                          /*udbIndexLow*/,
        0                                          /*udbOffsetLow*/,
        GT_TRUE                                    /*udbOrderAscend*/,
        &prvTgfPacketPassenger2tagsIpv4OthInfo     /*packetPtr*/,
        offsetInPacket                             /*offsetInPacket*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb10Ipv4TcpL4Test function
* @endinternal
*
* @brief   Test on Egress PCL UDB10 L4 offset type on IPV4 TCP packet
*
* @note 10 first bytes from TCP header beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb10Ipv4TcpL4Test
(
    GT_VOID
)
{
    GT_U32 offsetInPacket =
        (TGF_L2_HEADER_SIZE_CNS
         + (2 * TGF_VLAN_TAG_SIZE_CNS)
         + TGF_ETHERTYPE_SIZE_CNS
         + TGF_IPV4_HEADER_SIZE_CNS);

    prvTgfPclUdbOnlyKeysUdbOnlyGenericTest(
        CPSS_PCL_DIRECTION_EGRESS_E                /*direction*/,
        CPSS_PCL_LOOKUP_0_0_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_TCP_E         /*packetType*/,
        1 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E    /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_10_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_L4_E                    /*offsetType*/,
        10                                         /*udbAmount*/,
        40                                         /*udbIndexLow*/,
        0                                          /*udbOffsetLow*/,
        GT_TRUE                                    /*udbOrderAscend*/,
        &prvTgfPacketPassenger2tagsIpv4TcpInfo     /*packetPtr*/,
        offsetInPacket                             /*offsetInPacket*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb20Ipv6UdpL4Test function
* @endinternal
*
* @brief   Test on Egress PCL UDB20 L4 offset type on IPV6 UDP packet
*
* @note 20 first bytes from UDP header beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb20Ipv6UdpL4Test
(
    GT_VOID
)
{
    GT_U32 offsetInPacket =
        (TGF_L2_HEADER_SIZE_CNS
         + (2 * TGF_VLAN_TAG_SIZE_CNS)
         + TGF_ETHERTYPE_SIZE_CNS
         + TGF_IPV6_HEADER_SIZE_CNS);

    prvTgfPclUdbOnlyKeysUdbOnlyGenericTest(
        CPSS_PCL_DIRECTION_EGRESS_E                /*direction*/,
        CPSS_PCL_LOOKUP_0_0_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_IPV6_UDP_E         /*packetType*/,
        2 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_20_E    /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_20_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_L4_E                    /*offsetType*/,
        20                                         /*udbAmount*/,
        20                                         /*udbIndexLow*/,
        0                                          /*udbOffsetLow*/,
        GT_FALSE                                   /*udbOrderAscend*/,
        &prvTgfPacketPassengerIpv6UdpInfo          /*packetPtr*/,
        offsetInPacket                             /*offsetInPacket*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb30EthOverMplsMplsMinus2Test function
* @endinternal
*
* @brief   Test on Egress PCL UDB30 MplsMinus2 offset type on ETH over MPLS packet.
*
* @note 20 first bytes from MPLS Ethernet Type beginning.
*       MPLS offset supported by HW only up to 20 bytes
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb30EthOverMplsMplsMinus2Test
(
    GT_VOID
)
{
    GT_U32 offsetInPacket =
        (TGF_L2_HEADER_SIZE_CNS
         + (2 * TGF_VLAN_TAG_SIZE_CNS));

    prvTgfPclUdbOnlyKeysUdbOnlyGenericTest(
        CPSS_PCL_DIRECTION_EGRESS_E                /*direction*/,
        CPSS_PCL_LOOKUP_0_0_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_MPLS_E             /*packetType*/,
        0 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_30_E    /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_30_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_MPLS_MINUS_2_E          /*offsetType*/,
        20                                         /*udbAmount*/,
        20                                         /*udbIndexLow*/,
        0                                          /*udbOffsetLow*/,
        GT_TRUE                                    /*udbOrderAscend*/,
        &prvTgfPacketMplsTunneled2tagsEthOthInfo   /*packetPtr*/,
        offsetInPacket                             /*offsetInPacket*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb40EthOtherL2Test function
* @endinternal
*
* @brief   Test on Egress PCL UDB40 L2 offset type on ETH Other packet.
*
* @note 40 first bytes from L2 Header beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb40EthOtherL2Test
(
    GT_VOID
)
{
    GT_U32 offsetInPacket = 0;

    prvTgfPclUdbOnlyKeysUdbOnlyGenericTest(
        CPSS_PCL_DIRECTION_EGRESS_E                /*direction*/,
        CPSS_PCL_LOOKUP_0_0_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E   /*packetType*/,
        0 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_40_E    /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_40_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_L2_E                    /*offsetType*/,
        40                                         /*udbAmount*/,
        10                                         /*udbIndexLow*/,
        0                                          /*udbOffsetLow*/,
        GT_TRUE                                    /*udbOrderAscend*/,
        &prvTgfPacketNotTunneled2tagsEthOthInfo    /*packetPtr*/,
        offsetInPacket                             /*offsetInPacket*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb50Ipv6OtherL3Minus2Test function
* @endinternal
*
* @brief   Test on Egress PCL UDB50 L3Minus2 offset type on IPV6 Other packet.
*
* @note 50 first bytes from IPV6 Ethernet Type beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb50Ipv6OtherL3Minus2Test
(
    GT_VOID
)
{
    GT_U32 offsetInPacket =
        (TGF_L2_HEADER_SIZE_CNS
         + (2 * TGF_VLAN_TAG_SIZE_CNS));

    prvTgfPclUdbOnlyKeysUdbOnlyGenericTest(
        CPSS_PCL_DIRECTION_EGRESS_E                /*direction*/,
        CPSS_PCL_LOOKUP_0_0_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_IPV6_OTHER_E       /*packetType*/,
        2 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_50_E    /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_50_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_L3_MINUS_2_E            /*offsetType*/,
        50                                         /*udbAmount*/,
        0                                          /*udbIndexLow*/,
        0                                          /*udbOffsetLow*/,
        GT_FALSE                                   /*udbOrderAscend*/,
        &prvTgfPacketNotTunneled2tagsIpv6OthInfo   /*packetPtr*/,
        offsetInPacket                             /*offsetInPacket*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb60Ipv4OtherL3Minus2Test function
* @endinternal
*
* @brief   Test on Egress PCL UDB60 L3Minus2 offset type on IPV4 Other packet.
*
* @note 50 first bytes from IPV4 Ethernet Type beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb60Ipv4OtherL3Minus2Test
(
    GT_VOID
)
{
    GT_U32 offsetInPacket =
        (TGF_L2_HEADER_SIZE_CNS
         + (2 * TGF_VLAN_TAG_SIZE_CNS));

    prvTgfPclUdbOnlyKeysUdbOnlyGenericTest(
        CPSS_PCL_DIRECTION_EGRESS_E                /*direction*/,
        CPSS_PCL_LOOKUP_0_0_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E       /*packetType*/,
        1 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_60_E    /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_60_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_L3_MINUS_2_E            /*offsetType*/,
        50                                         /*udbAmount*/,
        0                                          /*udbIndexLow*/,
        0                                          /*udbOffsetLow*/,
        GT_TRUE                                    /*udbOrderAscend*/,
        &prvTgfPacketPassenger2tagsIpv4OthInfo     /*packetPtr*/,
        offsetInPacket                             /*offsetInPacket*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb30MplsEthL3Minus2Test function
* @endinternal
*
* @brief   Test on Ingress PCL UDB30 L3Minus2 offset type on Ethernet over MPLS packet
*
* @note 30 first bytes from MPLS Ethernet Type beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb30MplsEthL3Minus2Test
(
    GT_VOID
)
{
    GT_U32 offsetInPacket =
        (TGF_L2_HEADER_SIZE_CNS
         + (2 * TGF_VLAN_TAG_SIZE_CNS)
         + TGF_ETHERTYPE_SIZE_CNS
         + TGF_MPLS_HEADER_SIZE_CNS) - 2;

    prvTgfPclUdbOnlyKeysUdbOnlyGenericTest(
        CPSS_PCL_DIRECTION_INGRESS_E               /*direction*/,
        CPSS_PCL_LOOKUP_0_1_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_MPLS_E             /*packetType*/,
        0 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_30_E   /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_30_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_L3_MINUS_2_E            /*offsetType*/,
        30                                         /*udbAmount*/,
        20                                         /*udbIndexLow*/,
        0                                          /*udbOffsetLow*/,
        GT_TRUE                                    /*udbOrderAscend*/,
        &prvTgfPacketMplsTunneled2tagsEthOthInfo   /*packetPtr*/,
        offsetInPacket                             /*offsetInPacket*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb30EthOverMplsL3Minus2Test function
* @endinternal
*
* @brief   Test on Egress PCL UDB30 L3Minus2 offset type on ETH over MPLS packet.
*
* @note 30 first bytes from MPLS Ethernet Type beginning.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb30EthOverMplsL3Minus2Test
(
    GT_VOID
)
{
    GT_U32 offsetInPacket =
        (TGF_L2_HEADER_SIZE_CNS
         + (2 * TGF_VLAN_TAG_SIZE_CNS)
         + TGF_ETHERTYPE_SIZE_CNS
         + TGF_MPLS_HEADER_SIZE_CNS) - 2;

    prvTgfPclUdbOnlyKeysUdbOnlyGenericTest(
        CPSS_PCL_DIRECTION_EGRESS_E                /*direction*/,
        CPSS_PCL_LOOKUP_0_0_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_MPLS_E             /*packetType*/,
        0 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_30_E    /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_30_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_L3_MINUS_2_E            /*offsetType*/,
        30                                         /*udbAmount*/,
        20                                         /*udbIndexLow*/,
        0                                          /*udbOffsetLow*/,
        GT_TRUE                                    /*udbOrderAscend*/,
        &prvTgfPacketMplsTunneled2tagsEthOthInfo   /*packetPtr*/,
        offsetInPacket                             /*offsetInPacket*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb10LlcNonSnapTest function
* @endinternal
*
* @brief   Test Egress PCL on LLC Non SNAP packet.
*
* @note Used 10 first bytes of anchor 'L3 offset - 2'.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb10LlcNonSnapTest
(
    GT_VOID
)
{
    GT_U32 offsetInPacket =
            (TGF_L2_HEADER_SIZE_CNS
             + (2 * TGF_VLAN_TAG_SIZE_CNS) + 3);

    prvTgfPclUdbOnlyKeysUdbOnlyGenericTest(
        CPSS_PCL_DIRECTION_EGRESS_E                /*direction*/,
        CPSS_PCL_LOOKUP_0_0_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E   /*packetType*/,
        0 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E    /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_10_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_L3_MINUS_2_E            /*offsetType*/,
        10                                         /*udbAmount*/,
        40                                         /*udbIndexLow*/,
        0                                          /*udbOffsetLow*/,
        GT_TRUE                                    /*udbOrderAscend*/,
        &prvTgfPacketPassengerLlcNonSnapInfo       /*packetPtr*/,
        offsetInPacket                             /*offsetInPacket*/);
}

/* test on UDB Only EPCL UDB_Replace fields */

/**
* @internal prvTgfPclUdbOnlyKeysUdbOnlyUdbReplaceTestPclCfg function
* @endinternal
*
* @brief   PCL Configuration of UdbReplace Test on UDB Only Key
*
* @param[in] direction                - PCL direction
* @param[in] lookupNum                - lookup Number
* @param[in] packetType               - packet type
* @param[in] packetTypeIndex          - 0 - not IP, 1 - IPV4, 2 - IPV6
* @param[in] ruleFormat               - rule Format
* @param[in] udbAmount                - UDB Amount
* @param[in] pclId                    - PCL Id
* @param[in] maskPtr                  - (pointer to) mask for replaced UDBs
* @param[in] patternPtr               - (pointer to) pattern for replaced UDBs 
*                                       None
*/
static GT_VOID prvTgfPclUdbOnlyKeysUdbOnlyUdbReplaceTestPclCfg
(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN GT_U32                           packetTypeIndex,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN GT_U32                           udbAmount,
    IN GT_U32                           pclId,
    IN GT_U8                            *maskPtr,
    IN GT_U8                            *patternPtr
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    GT_U8                            *udbMaskPtr;
    GT_U8                            *udbPatternPtr;
    GT_U32                           i;
    GT_U32                           udbReplaceBitmap;

    udbReplaceBitmap = ((1 << udbAmount) - 1);

    if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E &&
       ((direction == CPSS_PCL_DIRECTION_INGRESS_E && lookupNum == CPSS_PCL_LOOKUP_0_1_E) ||
        (direction == CPSS_PCL_DIRECTION_EGRESS_E)))
    {
        /* IPCL1/EPCL test not applicable - will fail */
        return;
    }

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    if (direction != CPSS_PCL_DIRECTION_EGRESS_E)
    {
        udbMaskPtr    = mask.ruleIngrUdbOnly.udb;
        udbPatternPtr = pattern.ruleIngrUdbOnly.udb;
    }
    else
    {
        udbMaskPtr    = mask.ruleEgrUdbOnly.udb;
        udbPatternPtr = pattern.ruleEgrUdbOnly.udb;
    }

    for (i = 0; (i < udbAmount); i++)
    {
        udbMaskPtr[i]    = maskPtr[i];
        udbPatternPtr[i] = patternPtr[i];
    }

    rc = prvTgfPclUdbOnlyKeysPclLegacyCfgSetWithUDB49Bitmap(
        direction, lookupNum,
        packetTypeIndex, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
        pclId, ruleFormat,
        &mask, &pattern, GT_FALSE /*udb49BitmapEnable*/);
    if (rc != GT_OK) return;

    rc = prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectSet(
        lookupNum, packetType, ruleFormat,
        udbReplaceBitmap,
        0/*udbAmount*/,
        0/*udbSelIndexBase*/,
        0/*udbSelIndexIncrement*/);
    if (rc != GT_OK) return;

}

/**
* @internal prvTgfPclUdbOnlyKeysUdbOnlyUdbReplaceTestPclCfgRestore function
* @endinternal
*
* @brief   Restore PCL Configuration of UdbReplace Test on UDB Only Key
*
* @param[in] direction                - PCL direction
* @param[in] lookupNum                - lookup Number
* @param[in] packetType               - packet type
* @param[in] ruleFormat               - rule Format
* @param[in] ruleSize                 - rule Size
* @param[in] udbAmount                - UDB Amount
*                                       None
*/
static GT_VOID prvTgfPclUdbOnlyKeysUdbOnlyUdbReplaceTestPclCfgRestore
(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN CPSS_PCL_RULE_SIZE_ENT           ruleSize,
    IN GT_U32                           udbAmount
)
{
    prvTgfPclUdbOnlyKeysPclLegacyCfgReset(
        direction,
        lookupNum,
        ruleSize);

    prvTgfPclUdbOnlyKeysPclUdbRangeCfgReset(
        direction,
        packetType,
        0/*udbCfgIndexBase*/,
        udbAmount);

    prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectReset(
        lookupNum, packetType, ruleFormat);
}

/**
* @internal prvTgfPclUdbOnlyKeysUdbOnlyUdbReplaceTestTraffic function
* @endinternal
*
* @brief   UdbReplace Test on UDB Only Key
*
* @param[in] packetPtr                - (pointer to) Info of the packet used in the test
*                                       None
*/
static GT_VOID prvTgfPclUdbOnlyKeysUdbOnlyUdbReplaceTestTraffic
(
    IN TGF_PACKET_STC                   *packetPtr
)
{
    GT_STATUS                        rc = GT_OK;
    static GT_U8                     packetBuffer[256];
    GT_U32                           numOfBytesInBuff;
    TGF_AUTO_CHECKSUM_FULL_INFO_STC  checksumInfoArr[TGF_AUTO_CHECKSUM_NUM_OF_HEADERS_CNS];/* auto checksum info */
    TGF_AUTO_CHECKSUM_EXTRA_INFO_STC extraChecksumInfoArr[TGF_AUTO_CHECKSUM_NUM_OF_HEADERS_CNS];/* extra checksum info */

    numOfBytesInBuff = sizeof(packetBuffer);
    cpssOsMemSet(checksumInfoArr , 0, sizeof(checksumInfoArr));
    cpssOsMemSet(extraChecksumInfoArr , 0, sizeof(extraChecksumInfoArr));

    rc = tgfTrafficEnginePacketBuild(
        packetPtr, packetBuffer, &numOfBytesInBuff,
        checksumInfoArr, extraChecksumInfoArr);
    if (rc != GT_OK) goto label_restore;

    /* apply auto checksum fields */
    rc = tgfTrafficGeneratorPacketChecksumUpdate(
        checksumInfoArr, extraChecksumInfoArr);
    if (rc != GT_OK) goto label_restore;

    /* AUTODOC: Set Vlan And FDB configuration */
    rc = prvTgfPclUdbOnlyKeysPortVlanFdbSet();
    if (rc != GT_OK) goto label_restore;

    /* AUTODOC: Double Tagged Packet */
    prvTgfPclUdbOnlyKeysVidTrafficGenerate(packetPtr);

    /* AUTODOC: Check that TAG0 VID is modified */
    prvTgfPclUdbOnlyKeysTrafficEgressVidCheck(PRV_TGF_PCL_NEW_VLAN_ID_CNS, GT_TRUE/*check 16 bit*/);

    label_restore:

    /* AUTODOC: Restore Vlan And FDB configuration */
    prvTgfPclUdbOnlyKeysPortVlanFdbReset();
}

/**
* @internal prvTgfPclUdbOnlyKeysUdbOnlyUdbReplaceTest function
* @endinternal
*
* @brief   UdbReplace Test on UDB Only Key
*
* @param[in] direction                - PCL direction
* @param[in] lookupNum                - lookup Number
* @param[in] packetType               - packet type
* @param[in] packetTypeIndex          - 0 - not IP, 1 - IPV4, 2 - IPV6
* @param[in] ruleFormat               - rule Format
* @param[in] ruleSize                 - rule Size
* @param[in] udbAmount                - UDB Amount
* @param[in] pclId                    - PCL Id
* @param[in] maskPtr                  - (pointer to) mask for replaced UDBs
* @param[in] patternPtr               - (pointer to) pattern for replaced UDBs 
* @param[in] packetPtr                - (pointer to) Info of the packet used in the test
*                                       None
*/
static GT_VOID prvTgfPclUdbOnlyKeysUdbOnlyUdbReplaceTest
(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN GT_U32                           packetTypeIndex,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN CPSS_PCL_RULE_SIZE_ENT           ruleSize,
    IN GT_U32                           udbAmount,
    IN GT_U32                           pclId,
    IN GT_U8                            *maskPtr,
    IN GT_U8                            *patternPtr,
    IN TGF_PACKET_STC                   *packetPtr
)
{
    prvTgfPclUdbOnlyKeysUdbOnlyUdbReplaceTestPclCfg(
        direction, lookupNum, packetType, packetTypeIndex,
        ruleFormat, udbAmount, pclId,
        maskPtr, patternPtr);

    prvTgfPclUdbOnlyKeysUdbOnlyUdbReplaceTestTraffic(packetPtr);

    prvTgfPclUdbOnlyKeysUdbOnlyUdbReplaceTestPclCfgRestore(
        direction, lookupNum, packetType,
        ruleFormat, ruleSize, udbAmount);
}

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb20Ipv6UdpReplacedPclIdTest function
* @endinternal
*
* @brief   Test on Egress PCL UDB20 L4 offset type on IPV6 UDP packet
*
* @note 2 replaced bytes with PCL Id.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb20Ipv6UdpReplacedPclIdTest
(
    GT_VOID
)
{
    GT_U32 pclId = 0x2AA;
    GT_U8  mask[2] = {0xFF, 0x03};
    GT_U8  pattern[2];

    mask[0]    = 0xFF;
    pattern[0] = (pclId & 0xFF);
    mask[1]    = 0x03;
    pattern[1] = ((pclId >> 8) & 0x03);

    prvTgfPclUdbOnlyKeysUdbOnlyUdbReplaceTest(
        CPSS_PCL_DIRECTION_EGRESS_E                /*direction*/,
        CPSS_PCL_LOOKUP_0_0_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_IPV6_UDP_E         /*packetType*/,
        2 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_20_E    /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_20_BYTES_E              /*ruleSize*/,
        2 /*udbAmount*/, pclId,
        mask, pattern,
        &prvTgfPacketPassengerIpv6UdpInfo);
}

/* Test of SRC-TRG Configuration table access mode */

/**
* @internal prvTgfPclUdbOnlyKeysIPclLegacyCfgSourceIdSet function
* @endinternal
*
* @brief   Set Legacy IPCL Configuration for combining sourceIndex and targetIndex in srcId 
*          for next IPCL lookup in source-target mode.
* @note    Not usable for next EPCL lookup. 
*
* @param[in] lookupNum                - PCL Lookup number
* @param[in] packetType               - packet type
* @param[in] packetTypeIndex          - 0 - not IP, 1 - IPV4, 2 - IPV6
* @param[in] pclId                    - PCL Id
* @param[in] sourceIndex              - source Index
* @param[in] targetIndex              - target Index
* @param[in] ruleFormat               - rule Format
* @param[in] ruleIndexOffset          - rule Index Offset (inside TCAM arrea mapped to lookup)
* @param[in] maskPtr                  - (pointer to) rule mask
* @param[in] patternPtr               - (pointer to) rule pattern
*/
static GT_VOID prvTgfPclUdbOnlyKeysIPclLegacyCfgSourceIdSet
(
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN GT_U32                           packetTypeIndex,
    IN GT_U32                           pclId,
    IN GT_U32                           sourceIndex,
    IN GT_U32                           targetIndex,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN GT_U32                           ruleIndexOffset,
    IN GT_U8                            *maskPtr,
    IN GT_U8                            *patternPtr
)
{
    GT_STATUS                        rc;
    PRV_TGF_PCL_ACTION_STC           action;
    GT_U32                           ruleIndex;
    GT_U32                           i;
    GT_U8                            *udbMaskPtr;
    GT_U8                            *udbPatternPtr;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;

    /* using bits5:0 for sorce and bits11:6 for target is default */
    /* CPSS has no API to set other and to check the values       */

    rc = cpssDxChPclSourceIdMaskSet(
        prvTgfDevNum, lookupNum, 0xFFF);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPclSourceIdMaskSet");

    rc = cpssDxChPclCopyReservedMaskSet(
        prvTgfDevNum, CPSS_PCL_DIRECTION_INGRESS_E, lookupNum, 0x003FF);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPclCopyReservedMaskSet");

    /* AUTODOC: action - TAG0 vlan modification */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.egressPolicy = GT_FALSE;
    action.sourceId.assignSourceId = GT_TRUE;
    action.sourceId.sourceIdValue  = (targetIndex << 6);
    action.copyReserved.copyReserved = sourceIndex;
    action.copyReserved.assignEnable = GT_TRUE;

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    udbMaskPtr    = mask.ruleIngrUdbOnly.udb;
    udbPatternPtr = pattern.ruleIngrUdbOnly.udb;

    for (i = 0; (i < 2); i++)
    {
        udbMaskPtr[i]    = maskPtr[i];
        udbPatternPtr[i] = patternPtr[i];
    }

    ruleIndex = getRuleIndexBase(CPSS_PCL_DIRECTION_INGRESS_E, lookupNum) + ruleIndexOffset;

    prvTgfPclUdbOnlyKeysPclLegacyCfgSetWithUDB49BmpExt(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E, lookupNum,
        PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E,
        0 /*cfgIndex not relevant for port mode */,
        packetTypeIndex, pclId, ruleFormat, ruleIndex,
        &mask, &pattern, &action, GT_FALSE /*udb49BitmapEnable*/);

    prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectSet(
        lookupNum, packetType, ruleFormat,
        3/*udbReplaceBitmap*/,
        0/*udbAmount*/,
        0/*udbSelIndexBase*/,
        0/*udbSelIndexIncrement*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysPclLegacyCfgSetWithUDB49Bitmap function
* @endinternal
*
* @brief   Set Legacy PCL Configuration
*
* @param[in] direction                - PCL direction
* @param[in] lookupNum                - PCL Lookup number
* @param[in] packetType               - packet type
* @param[in] packetTypeIndex          - 0 - not IP, 1 - IPV4, 2 - IPV6
* @param[in] pclId                    - PCL Id
* @param[in] sourceIndex              - source Index
* @param[in] targetIndex              - target Index
* @param[in] ruleFormat               - rule Format
* @param[in] ruleIndexOffset          - rule Index Offset (inside TCAM arrea mapped to lookup)
* @param[in] maskPtr                  - (pointer to) rule mask
* @param[in] patternPtr               - (pointer to) rule pattern
*/
static GT_VOID prvTgfPclUdbOnlyKeysPclLegacyCfgSrcTrgModeSet
(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN GT_U32                           packetTypeIndex,
    IN GT_U32                           pclId,
    IN GT_U32                           sourceIndex,
    IN GT_U32                           targetIndex,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN GT_U32                           ruleIndexOffset,
    IN GT_U8                            *maskPtr,
    IN GT_U8                            *patternPtr
)
{
    PRV_TGF_PCL_ACTION_STC           action;
    GT_U32                           portIndex;
    GT_U32                           cfgIndex;
    GT_U32                           ruleIndex;
    GT_U32                           i;
    GT_U8                            *udbMaskPtr;
    GT_U8                            *udbPatternPtr;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    CPSS_DXCH_PCL_EPG_CONFIG_STC     epgConfig;
    GT_STATUS                        rc;

    if(PRV_CPSS_SIP_6_30_CHECK_MAC(prvTgfDevNum))
    {
        rc = cpssDxChPclEpgConfigGet(prvTgfDevNum, lookupNum, &epgConfigRestore);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPclEpgConfigGet");

        epgConfig.useSrcTrgMode      = CPSS_DXCH_PCL_USE_SRC_TRG_MODE_COPY_RESERVED_SRC_ID_E;
        epgConfig.assignFromUdbsMode = epgConfigRestore.assignFromUdbsMode;
        rc = cpssDxChPclEpgConfigSet(prvTgfDevNum, lookupNum, &epgConfig);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPclEpgConfigSet");
    }
    portIndex =
        (direction == CPSS_PCL_DIRECTION_EGRESS_E)
            ? PRV_TGF_RECEIVE_PORT_IDX_CNS
            : PRV_TGF_SEND_PORT_IDX_CNS;

    /* not configurable HW behavior */
    cfgIndex = ((targetIndex & 0x3F) << 6) | (sourceIndex & 0x3F);

    /* AUTODOC: action - TAG0 vlan modification */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    if (direction == CPSS_PCL_DIRECTION_EGRESS_E)
    {
        action.egressPolicy = GT_TRUE;
        action.vlan.egressVlanId0Cmd =
            PRV_TGF_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E;
        action.vlan.vlanId = PRV_TGF_PCL_NEW_VLAN_ID_CNS;
    }
    else
    {
        action.egressPolicy = GT_FALSE;
        action.vlan.vlanCmd =
            CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
        action.vlan.modifyVlan =
            CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E;
        action.vlan.vlanId = PRV_TGF_PCL_NEW_VLAN_ID_CNS;
    }

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    if (direction != CPSS_PCL_DIRECTION_EGRESS_E)
    {
        udbMaskPtr    = mask.ruleIngrUdbOnly.udb;
        udbPatternPtr = pattern.ruleIngrUdbOnly.udb;
    }
    else
    {
        udbMaskPtr    = mask.ruleEgrUdbOnly.udb;
        udbPatternPtr = pattern.ruleEgrUdbOnly.udb;
    }

    for (i = 0; (i < 2); i++)
    {
        udbMaskPtr[i]    = maskPtr[i];
        udbPatternPtr[i] = patternPtr[i];
    }

    ruleIndex = getRuleIndexBase(direction, lookupNum) + ruleIndexOffset;

    prvTgfPclUdbOnlyKeysPclLegacyCfgSetWithUDB49BmpExt(
        prvTgfPortsArray[portIndex], direction, lookupNum,
        PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_SRC_TRG_E,
        cfgIndex, packetTypeIndex, pclId, ruleFormat, ruleIndex,
        &mask, &pattern, &action, GT_FALSE /*udb49BitmapEnable*/);

    prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectSet(
        lookupNum, packetType, ruleFormat,
        3/*udbReplaceBitmap*/,
        0/*udbAmount*/,
        0/*udbSelIndexBase*/,
        0/*udbSelIndexIncrement*/);
}



/**
* @internal prvTgfPclUdbOnlyKeysUdbOnlyUdbReplaceTestSrcTrgPclCfgRestore function
* @endinternal
*
* @brief   Restore PCL Configuration of UdbReplace Test on UDB Only Key
*
* @param[in] direction                - PCL direction
* @param[in] lookupNum                - lookup Number
* @param[in] packetType               - packet type
* @param[in] ruleFormat               - rule Format
* @param[in] ruleSize                 - rule Size
* @param[in] ruleIndexOffset          - rule Index Offset
*                                       None
*/
static GT_VOID prvTgfPclUdbOnlyKeysUdbOnlyUdbReplaceTestSrcTrgPclCfgRestore
(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN CPSS_PCL_RULE_SIZE_ENT           ruleSize,
    IN GT_U32                           ruleIndexOffset
)
{
    GT_U32 ruleIndex;
    GT_STATUS   rc;

    ruleIndex = getRuleIndexBase(direction, lookupNum) + ruleIndexOffset;

    prvTgfPclUdbOnlyKeysPclLegacyCfgResetExt(
        CPSS_PCL_DIRECTION_INGRESS_E,
        lookupNum, ruleSize, ruleIndex);

    prvTgfPclUdbOnlyKeysPclUdbRangeCfgReset(
        CPSS_PCL_DIRECTION_INGRESS_E,
        packetType,
        0/*udbCfgIndexBase*/,
        0/*udbAmount*/);

    prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectReset(
        lookupNum, packetType, ruleFormat);

    if(PRV_CPSS_SIP_6_30_CHECK_MAC(prvTgfDevNum))
    {
        rc = cpssDxChPclEpgConfigSet(prvTgfDevNum, lookupNum, &epgConfigRestore);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPclEpgConfigSet");
    }
}

/**
* @internal prvTgfPclUdbOnlyKeysIngressUdb20Ipv6UdpReplacedPclIdSrcTrgModeTest function
* @endinternal
*
* @brief   Test on Ingress PCL UDB20 L4 offset type on IPV6 UDP packet in SRC-TRG mode 
*
* @note 2 replaced bytes with PCL Id.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngressUdb20Ipv6UdpReplacedPclIdSrcTrgModeTest
(
    GT_VOID
)
{
    GT_U32 pclId0 = 0x155;
    GT_U32 pclId1 = 0x2AA;
    GT_U32 sourceIndex = 0x15;
    GT_U32 targetIndex = 0x2A;
    GT_U32 ruleIndexOffset0 = 0;
    GT_U32 ruleIndexOffset1 = 12;
    GT_U8  mask[2];
    GT_U8  pattern0[2];
    GT_U8  pattern1[2];

    mask[0]     = 0xFF;
    mask[1]     = 0x03;
    pattern0[0] = (pclId0 & 0xFF);
    pattern0[1] = ((pclId0 >> 8) & 0x03);
    pattern1[0] = (pclId1 & 0xFF);
    pattern1[1] = ((pclId1 >> 8) & 0x03);

    prvTgfPclUdbOnlyKeysIPclLegacyCfgSourceIdSet(
        CPSS_PCL_LOOKUP_0_0_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV6_UDP_E,
        2 /*0 - not IP, 1 - IPV4, 2 - IPV6 */,
        pclId0, sourceIndex, targetIndex,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_20_E,
        ruleIndexOffset0, mask, pattern0);

    prvTgfPclUdbOnlyKeysPclLegacyCfgSrcTrgModeSet(
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_1_E,
        PRV_TGF_PCL_PACKET_TYPE_IPV6_UDP_E,
        2 /*0 - not IP, 1 - IPV4, 2 - IPV6 */,
        pclId1, sourceIndex, targetIndex,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_20_E,
        ruleIndexOffset1, mask, pattern1);

    prvTgfPclUdbOnlyKeysUdbOnlyUdbReplaceTestTraffic(&prvTgfPacketPassengerIpv6UdpInfo);

    prvTgfPclUdbOnlyKeysUdbOnlyUdbReplaceTestSrcTrgPclCfgRestore(
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_0_E, 
        PRV_TGF_PCL_PACKET_TYPE_IPV6_UDP_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_20_E,
        CPSS_PCL_RULE_SIZE_20_BYTES_E,
        ruleIndexOffset0);

    prvTgfPclUdbOnlyKeysUdbOnlyUdbReplaceTestSrcTrgPclCfgRestore(
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_1_E, 
        PRV_TGF_PCL_PACKET_TYPE_IPV6_UDP_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_20_E,
        CPSS_PCL_RULE_SIZE_20_BYTES_E,
        ruleIndexOffset1);
}



/* Tests on bitmap of UDB49 values */

/**
* @internal prvTgfPclUdbOnlyKeysUDB49ValuesBitmapTest function
* @endinternal
*
* @brief   Test on UDB Only Key Bitmap of UDB49 values in the key bytes 14-45
*
* @param[in] direction                - PCL direction
* @param[in] lookupNum                - lookup Number
* @param[in] packetType               - packet type
* @param[in] packetTypeIndex          - 0 - not IP, 1 - IPV4, 2 - IPV6
* @param[in] ruleFormat               - rule Format
* @param[in] ruleSize                 - rule Size
* @param[in] udb49OffsetType          - offset Type for UDB49
* @param[in] udb49CfgOffset           - offset for UDB49
* @param[in] matchBytesNum            - amount of bytes in rule to match
* @param[in] matchBitIndex            - the number of the single bit containing one in the key
* @param[in] packetPtr                - pointer to the packet data
*/
static GT_VOID prvTgfPclUdbOnlyKeysUDB49ValuesBitmapTest
(
    IN CPSS_PCL_DIRECTION_ENT           direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN GT_U32                           packetTypeIndex,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN CPSS_PCL_RULE_SIZE_ENT           ruleSize,
    IN PRV_TGF_PCL_OFFSET_TYPE_ENT      udb49OffsetType,
    IN GT_U32                           udb49CfgOffset,
    IN GT_U32                           matchBytesNum,
    IN GT_U32                           matchBitIndex,
    IN TGF_PACKET_STC                   *packetPtr
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    GT_U8                            *udbMaskPtr;
    GT_U8                            *udbPatternPtr;
    GT_U32                           i;
    static GT_U8                     packetBuffer[256];
    GT_U32                           numOfBytesInBuff;
    TGF_AUTO_CHECKSUM_FULL_INFO_STC  checksumInfoArr[TGF_AUTO_CHECKSUM_NUM_OF_HEADERS_CNS];/* auto checksum info */
    TGF_AUTO_CHECKSUM_EXTRA_INFO_STC extraChecksumInfoArr[TGF_AUTO_CHECKSUM_NUM_OF_HEADERS_CNS];/* extra checksum info */
    PRV_TGF_PCL_UDB_SELECT_STC       udbSelect;

    numOfBytesInBuff = sizeof(packetBuffer);
    cpssOsMemSet(checksumInfoArr , 0, sizeof(checksumInfoArr));
    cpssOsMemSet(extraChecksumInfoArr , 0, sizeof(extraChecksumInfoArr));

    rc = tgfTrafficEnginePacketBuild(
        packetPtr, packetBuffer, &numOfBytesInBuff,
        checksumInfoArr, extraChecksumInfoArr);
    if (rc != GT_OK) goto label_restore;

    /* apply auto checksum fields */
    rc = tgfTrafficGeneratorPacketChecksumUpdate(
        checksumInfoArr, extraChecksumInfoArr);
    if (rc != GT_OK) goto label_restore;

    /* AUTODOC: Set Vlan And FDB configuration */
    rc = prvTgfPclUdbOnlyKeysPortVlanFdbSet();
    if (rc != GT_OK) goto label_restore;

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    if (direction != CPSS_PCL_DIRECTION_EGRESS_E)
    {
        udbMaskPtr    = mask.ruleIngrUdbOnly.udb;
        udbPatternPtr = pattern.ruleIngrUdbOnly.udb;
    }
    else
    {
        udbMaskPtr    = mask.ruleEgrUdbOnly.udb;
        udbPatternPtr = pattern.ruleEgrUdbOnly.udb;
    }

    for (i = 0; (i < matchBytesNum); i++)
    {
        /* key byte 14 is a beginning of bitmap */
        udbMaskPtr[14 + i]    = 0xFF;
        udbPatternPtr[14 + i] = 0;
    }
    i = (matchBitIndex / 8);
    udbPatternPtr[14 + i] |= (1 << (matchBitIndex % 8));

    /* add UDB49 == matchBitIndex immediately after bitmap */
    udbMaskPtr[14 + matchBytesNum]    = 0xFF;
    udbPatternPtr[14 + matchBytesNum] = (GT_U8)matchBitIndex;

    rc = prvTgfPclUdbOnlyKeysPclLegacyCfgSetWithUDB49Bitmap(
        direction,
        lookupNum,
        packetTypeIndex, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
        PRV_TGF_PCL_ID_CNS,
        ruleFormat,
        &mask, &pattern, GT_TRUE);
    if (rc != GT_OK) goto label_restore;

    rc = prvTgfPclUdbOnlyKeysPclUdbRangeCfgSet(
        direction,
        packetType,
        udb49OffsetType,
        49 /*udbCfgIndexBase*/,
        1 /*udbAmount*/,
        udb49CfgOffset,
        0/*udbCfgOffsetIncrement*/);
    if (rc != GT_OK) goto label_restore;

    /* UDB49 values bitmap caused by Configuration table entry   */
    /* include UDB49 immediately after bitmap - check bitmap size */
    cpssOsMemSet(&udbSelect, 0, sizeof(udbSelect));
    udbSelect.udbSelectArr[14 + matchBytesNum] = 49;
    rc = prvTgfPclUserDefinedBytesSelectSet(
        ruleFormat, packetType, lookupNum,
        &udbSelect);
    utfEqualVerify(GT_OK, rc, __LINE__, __FILE__);
    if (rc != GT_OK) goto label_restore;

    /* AUTODOC: Double Tagged Packet */
    prvTgfPclUdbOnlyKeysVidTrafficGenerate(packetPtr);

    /* AUTODOC: Check that TAG0 VID is modified */
    prvTgfPclUdbOnlyKeysTrafficEgressVidCheck(PRV_TGF_PCL_NEW_VLAN_ID_CNS, GT_TRUE/*check 16 bit*/);

    label_restore:

    /* AUTODOC: Restore Vlan And FDB configuration */
    prvTgfPclUdbOnlyKeysPortVlanFdbReset();

    prvTgfPclUdbOnlyKeysPclLegacyCfgReset(
        direction,
        lookupNum,
        ruleSize);

    prvTgfPclUdbOnlyKeysPclUdbRangeCfgReset(
        direction,
        packetType,
        49 /*udbCfgIndexBase*/,
        1/*udbAmount*/);

    prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectReset(
        lookupNum, packetType, ruleFormat);
}

/* Single Tagged Ethernet Other Packet Parts Array*/
static TGF_PACKET_PART_STC prvTgfPacketEthOthPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2PartWithBytesLess64},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTag0Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEthOth1EthertypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Double Tagged Ethernet Other Packet Info */
static TGF_PACKET_STC prvTgfPacketEthOthInfo = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
     + sizeof(prvTgfPacketPayloadDataArr)),                      /* totalLen */
    sizeof(prvTgfPacketEthOthPartsArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketEthOthPartsArray                                        /* partsArray */
};

/**
* @internal prvTgfPclUdbOnlyKeysIngrUdb50EthOtherUDB49Bitmap function
* @endinternal
*
* @brief   Test on Ingress PCL UDB50 UDB49-Values-Bitmap, used L2 offset type on ETH Other packet.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysIngrUdb50EthOtherUDB49Bitmap
(
    GT_VOID
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* legacy mode */
        rc = cpssDxChPclUserDefinedByteValuesBitmapExpansionModeSet(
            prvTgfDevNum, CPSS_DXCH_PCL_UDB_VALUES_BITMAP_MODE_32_BYTES_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPclUserDefinedByteValuesBitmapExpansionModeSet");
    }
#endif
    prvTgfPclUdbOnlyKeysUDB49ValuesBitmapTest(
        CPSS_PCL_DIRECTION_INGRESS_E               /*direction*/,
        CPSS_PCL_LOOKUP_0_0_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E   /*packetType*/,
        0 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_50_E   /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_50_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_L2_E                    /*udb49OffsetType*/,
        5                                          /*udb49CfgOffset*/,
        32                                         /*matchBytesNum*/,
        prvTgfPacketL2PartWithBytesLess64.daMac[5] /*matchBitIndex*/,
        &prvTgfPacketEthOthInfo                    /*packetPtr*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysEgrUdb60EthOtherUDB49Bitmap function
* @endinternal
*
* @brief   Test on Egress PCL UDB60 UDB49-Values-Bitmap, used L2 offset type on ETH Other packet.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysEgrUdb60EthOtherUDB49Bitmap
(
    GT_VOID
)
{
    prvTgfPclUdbOnlyKeysUDB49ValuesBitmapTest(
        CPSS_PCL_DIRECTION_EGRESS_E                /*direction*/,
        CPSS_PCL_LOOKUP_0_0_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E   /*packetType*/,
        0 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_60_E    /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_60_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_L2_E                    /*udb49OffsetType*/,
        11                                         /*udb49CfgOffset*/,
        32                                         /*matchBytesNum*/,
        prvTgfPacketL2PartWithBytesLess64.saMac[5] /*matchBitIndex*/,
        &prvTgfPacketEthOthInfo                    /*packetPtr*/);
}

/**
* @internal prvTgfPclUdbOnlyKeysSip6IngrUdb50EthOtherUDB49Bitmap16BytesMode function
* @endinternal
*
* @brief   Test on Ingress PCL UDB50 SIP6 UDB49-Values-Bitmap, used L2 offset type on ETH Other packet.
* @brief   Used 16-byte bitmap mode.
* @brief   Fixed from 60-byte key - test used 60-byte key with fixed fields.
* @brief   Falcon does not support it correct.
*
*/
GT_VOID prvTgfPclUdbOnlyKeysSip6IngrUdb50EthOtherUDB49Bitmap16BytesMode
(
    GT_VOID
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    /* legacy mode */
    rc = cpssDxChPclUserDefinedByteValuesBitmapExpansionModeSet(
        prvTgfDevNum, CPSS_DXCH_PCL_UDB_VALUES_BITMAP_MODE_16_BYTES_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPclUserDefinedByteValuesBitmapExpansionModeSet");
#endif
    prvTgfPclUdbOnlyKeysUDB49ValuesBitmapTest(
        CPSS_PCL_DIRECTION_INGRESS_E               /*direction*/,
        CPSS_PCL_LOOKUP_0_0_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E   /*packetType*/,
        0 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_50_E   /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_50_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_L2_E                    /*udb49OffsetType*/,
        4                                          /*udb49CfgOffset*/,
        16                                         /*matchBytesNum*/,
        prvTgfPacketL2PartWithBytesLess64.daMac[4] /*matchBitIndex*/,
        &prvTgfPacketEthOthInfo                    /*packetPtr*/);
#ifdef CHX_FAMILY
    /* legacy mode */
    rc = cpssDxChPclUserDefinedByteValuesBitmapExpansionModeSet(
        prvTgfDevNum, CPSS_DXCH_PCL_UDB_VALUES_BITMAP_MODE_32_BYTES_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPclUserDefinedByteValuesBitmapExpansionModeSet");
#endif
}

/**
* @internal prvTgfPclUdbOnlyKeysSip6IngrUdb50EthOtherUDB8Bitmap16BytesMode function
* @endinternal
*
* @brief   Test on Ingress PCL UDB50 SIP6 UDB49-Values-Bitmap, used L2 offset type on ETH Other packet.
* @brief   Used 8-byte bitmap mode.
*/
GT_VOID prvTgfPclUdbOnlyKeysSip6IngrUdb50EthOtherUDB49Bitmap8BytesMode
(
    GT_VOID
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc;
    /* legacy mode */
    rc = cpssDxChPclUserDefinedByteValuesBitmapExpansionModeSet(
        prvTgfDevNum, CPSS_DXCH_PCL_UDB_VALUES_BITMAP_MODE_8_BYTES_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPclUserDefinedByteValuesBitmapExpansionModeSet");
#endif
    prvTgfPclUdbOnlyKeysUDB49ValuesBitmapTest(
        CPSS_PCL_DIRECTION_INGRESS_E               /*direction*/,
        CPSS_PCL_LOOKUP_0_0_E                      /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E   /*packetType*/,
        0 /*0 - not IP, 1 - IPV4, 2 - IPV6*/       /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_50_E   /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_50_BYTES_E              /*ruleSize*/,
        PRV_TGF_PCL_OFFSET_L2_E                    /*udb49OffsetType*/,
        9                                          /*udb49CfgOffset*/,
        8                                          /*matchBytesNum*/,
        prvTgfPacketL2PartWithBytesLess64.saMac[3] /*matchBitIndex*/,
        &prvTgfPacketEthOthInfo                    /*packetPtr*/);
#ifdef CHX_FAMILY
    /* legacy mode */
    rc = cpssDxChPclUserDefinedByteValuesBitmapExpansionModeSet(
        prvTgfDevNum, CPSS_DXCH_PCL_UDB_VALUES_BITMAP_MODE_32_BYTES_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPclUserDefinedByteValuesBitmapExpansionModeSet");
#endif
}

static GT_BOOL prvTgfPclUdbOnlyKeysIngressUdb60FixedFieldsDataValid = GT_FALSE;
static struct
{
    GT_BOOL cncPclUnitEnable;
} prvTgfPclUdbOnlyKeysIngressUdb60FixedFieldsData;

/**
* @internal prvTgfPclUdbOnlyKeysIngressUdb60FixedFields function
* @endinternal
*
* @brief   Test on Ingress PCL UDB60_with_fixed_fields fixed_fields.
*          Configurationm, traffic and check results
*/
GT_VOID prvTgfPclUdbOnlyKeysIngressUdb60FixedFieldsTrafficTest
(
    GT_VOID
)
{
    GT_STATUS                        rc;
    GT_PORT_NUM                      portNum;
    GT_U32                           indexRangesBmp[4];
    PRV_TGF_CNC_CLIENT_ENT           client = PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E;
    GT_U32                           cncBlockNum = 0;
    GT_U32                           counterIdx = 0;
    PRV_TGF_CNC_COUNTER_STC          counter;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT    mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT    pattern;
    CPSS_DXCH_PCL_ACTION_STC         action;
    GT_U32                           pclId = 0x155;

    /* DATA of bypass packet */
    static GT_U8 payloadDataArr[48] ={0x33, 0x33, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    /* Bypass PAYLOAD part */
    static TGF_PACKET_PAYLOAD_STC payloadPart = {
        sizeof(payloadDataArr),                       /* dataLength */
        payloadDataArr                                /* dataPtr */
    };
    /*  L2 part of packet  */
    static TGF_PACKET_L2_STC packetL2Part = {
        {0x0C, 0x06, 0x01, 0x02, 0x03, 0x00},                /* daMac */
        {0x00, 0x11, 0x22, 0x44, 0x55, 0x66}                 /* saMac */
    };
    /* VLAN_TAG part */
    static TGF_PACKET_VLAN_TAG_STC packetVlanTag0Part =
    {
        TGF_ETHERTYPE_8100_VLAN_TAG_CNS,           /* etherType */
        1, 1, 1      /* pri, cfi, VlanId */
    };
    /* VLAN_TAG part */
    static TGF_PACKET_VLAN_TAG_STC packetVlanTag1Part =
    {
        TGF_ETHERTYPE_8100_VLAN_TAG_CNS,           /* etherType */
        2, 0, 3      /* pri, cfi, VlanId */
    };
    /* PARTS of packet */
    static TGF_PACKET_PART_STC packetPartArray[] = {
        {TGF_PACKET_PART_L2_E,        &packetL2Part},  /* type, partPtr */
        {TGF_PACKET_PART_VLAN_TAG_E,  &packetVlanTag0Part},
        {TGF_PACKET_PART_VLAN_TAG_E,  &packetVlanTag1Part},
        {TGF_PACKET_PART_PAYLOAD_E,   &payloadPart}
    };
    /*  PACKET to send info */
    static TGF_PACKET_STC packetInfo = {
        /* LENGTH of packet with CRC */
        (TGF_L2_HEADER_SIZE_CNS + sizeof(payloadDataArr)
            + (2 * TGF_VLAN_TAG_SIZE_CNS) + TGF_CRC_LEN_CNS),  /* totalLen */
        sizeof(packetPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
        packetPartArray                                        /* partsArray */
    };

    /* Get data for restore */
    prvTgfPclUdbOnlyKeysIngressUdb60FixedFieldsDataValid = GT_FALSE;
    rc = cpssDxChCncCountingEnableGet(
        prvTgfDevNum, CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_PCL_E,
        &(prvTgfPclUdbOnlyKeysIngressUdb60FixedFieldsData.cncPclUnitEnable));
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "cpssDxChCncCountingEnableGet: %d", prvTgfDevNum);

    /* Begin configuration */
    prvTgfPclUdbOnlyKeysIngressUdb60FixedFieldsDataValid = GT_TRUE;
    portNum = prvTgfPortsArray[0];

    /* AUTODOC: Init IPCL Engine for send port */
    rc = prvTgfPclDefPortInitExt1(
        portNum,
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        pclId,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_60_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_60_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_60_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: port %d", portNum);

    /* rule matching any packet */
    /* action - CNC counter     */
    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    action.pktCmd                         = CPSS_PACKET_CMD_FORWARD_E;
    action.mirror.cpuCode                 = CPSS_NET_FIRST_USER_DEFINED_E;
    action.matchCounter.enableMatchCount  = GT_TRUE;
    action.matchCounter.matchCounterIndex = counterIdx;

    /*fields*/
    mask.ruleIngrUdbOnly.udb60FixedFld.pclId          = 0x3FF;
    pattern.ruleIngrUdbOnly.udb60FixedFld.pclId       = pclId;
    mask.ruleIngrUdbOnly.udb60FixedFld.srcPort        = 0x7FFF;
    pattern.ruleIngrUdbOnly.udb60FixedFld.srcPort     = portNum;
    mask.ruleIngrUdbOnly.udb60FixedFld.macToMe        = 1;
    pattern.ruleIngrUdbOnly.udb60FixedFld.macToMe     = 0;
    mask.ruleIngrUdbOnly.udb60FixedFld.srcDevIsOwn    = 1;
    pattern.ruleIngrUdbOnly.udb60FixedFld.srcDevIsOwn = 1;
    mask.ruleIngrUdbOnly.udb60FixedFld.vid            = 0xFFF;
    pattern.ruleIngrUdbOnly.udb60FixedFld.vid         = packetVlanTag0Part.vid;
    mask.ruleIngrUdbOnly.udb60FixedFld.vid1           = 0xFFF;
    pattern.ruleIngrUdbOnly.udb60FixedFld.vid1        = packetVlanTag1Part.vid;
    mask.ruleIngrUdbOnly.udb60FixedFld.up1            = 0x7;
    pattern.ruleIngrUdbOnly.udb60FixedFld.up1         = packetVlanTag1Part.pri;
    /* qos profile and flow id not checked */
    /* call */
    rc = cpssDxChPclRuleSet(
        prvTgfDevNum, prvTgfPclTcamIndexGet(prvTgfDevNum),
        CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E,
        0 /*ruleIndex*/, 0 /*ruleOptionsBmp*/,
        &mask, &pattern, &action);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "cpssDxChPclRuleSet: failed");

    cpssOsMemSet(indexRangesBmp, 0, sizeof(indexRangesBmp));
    indexRangesBmp[0] = 0x1;

    /* enable client and set range for CNC block */
    rc = prvTgfCncTestCncBlockConfigure(
        cncBlockNum,
        client,
        GT_TRUE /*enable*/,
        indexRangesBmp,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfCncTestCncBlockConfigure: block %d, client %d",
            cncBlockNum,
            client);

    /* AUTODOC: enable PCL clients for CNC */
    rc = prvTgfCncCountingEnableSet(
        PRV_TGF_CNC_COUNTING_ENABLE_UNIT_PCL_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCncCountingEnableSet failed");

    /* clean counters */
    cpssOsMemSet(&counter, 0, sizeof(counter));
    rc = prvTgfCncCounterSet(
        cncBlockNum, counterIdx,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCncCounterSet failed");

    /*traffic*/

    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfSetTxSetupEth error");

    /* send Packet from port portNum */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfStartTransmitingEth error");

    /* wait to get stable traffic */
    cpssOsTimerWkAfter(0);

    /* check CNC counters */
    rc = prvTgfCncCounterGet(
        cncBlockNum, counterIdx,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCncCounterGet failed");
    UTF_VERIFY_EQUAL0_STRING_MAC(
        1, counter.packetCount.l[0], "Wrong CNC counter value");
}

/**
* @internal prvTgfPclUdbOnlyKeysIngressUdb60FixedFieldsConfigurationRestore function
* @endinternal
*
* @brief   Test on Ingress PCL UDB60_with_fixed_fields fixed_fields.
*          Configuration restore.
*/
GT_VOID prvTgfPclUdbOnlyKeysIngressUdb60FixedFieldsConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS                        rc;
    GT_PORT_NUM                      portNum;
    GT_U32                           indexRangesBmp[4];
    PRV_TGF_CNC_CLIENT_ENT           client = PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E;
    GT_U32                           cncBlockNum = 0;
    GT_U32                           counterIdx = 0;
    PRV_TGF_CNC_COUNTER_STC          counter;

    if (prvTgfPclUdbOnlyKeysIngressUdb60FixedFieldsDataValid == GT_FALSE)
    {
        return;
    }

    portNum = prvTgfPortsArray[0];

    cpssOsMemSet(indexRangesBmp, 0, sizeof(indexRangesBmp));
    cpssOsMemSet(&counter, 0, sizeof(counter));

    /* disable client */
    rc = prvTgfCncBlockClientEnableSet(
        cncBlockNum, client, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "prvTgfCncBlockClientEnableSet failed");

    /* reset ranges */
    rc = prvTgfCncBlockClientRangesSet(
        cncBlockNum, client, indexRangesBmp);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "prvTgfCncBlockClientRangesSet failed");

    /* clean counters */
    rc = prvTgfCncCounterSet(
        cncBlockNum, counterIdx,
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "prvTgfCncCounterSet failed");

    rc = prvTgfCncCountingEnableSet(
        PRV_TGF_CNC_COUNTING_ENABLE_UNIT_PCL_E,
        prvTgfPclUdbOnlyKeysIngressUdb60FixedFieldsData.cncPclUnitEnable);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "prvTgfCncCountingEnableSet failed");

    /* AUTODOC: Clear IPCL Engine configuration tables */
    prvTgfPclRestore();

    /* AUTODOC: Disables ingress policy for port 1 (the ingress port) */
    rc = prvTgfPclPortIngressPolicyEnable(portNum, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_NO_RETURN_MAC(
        GT_OK, rc, "prvTgfPclPortIngressPolicyEnable failed");

    prvTgfPclUdbOnlyKeysIngressUdb60FixedFieldsDataValid = GT_FALSE;
}

/**
* @internal prvTgfPclUdbOnlyKeysUdbOnlyIngressPortPclId2GenericTest function
* @endinternal
*
* @brief   PclId2 and hash Test on UDB Only Key
*
* @param[in] lookupNum                - lookup Number
* @param[in] packetType               - packet type
* @param[in] packetTypeIndex          - 0 - not IP, 1 - IPV4, 2 - IPV6
* @param[in] ruleFormat               - rule Format
* @param[in] ruleSize                 - rule Size
* @param[in] offsetType               - offset Type
* @param[in] packetPtr                - (pointer to) Info of the packet used in the test
* @param[in] portPclId2               - port Pcl Id2
*                                       None
*/
static GT_VOID prvTgfPclUdbOnlyKeysUdbOnlyIngressPortPclId2GenericTest
(
    IN CPSS_PCL_LOOKUP_NUMBER_ENT       lookupNum,
    IN PRV_TGF_PCL_PACKET_TYPE_ENT      packetType,
    IN GT_U32                           packetTypeIndex,
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat,
    IN CPSS_PCL_RULE_SIZE_ENT           ruleSize,
    IN TGF_PACKET_STC                   *packetPtr,
    IN GT_U32                           portPclId2
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    static GT_U8                     packetBuffer[256];
    GT_U32                           numOfBytesInBuff;
    TGF_AUTO_CHECKSUM_FULL_INFO_STC  checksumInfoArr[TGF_AUTO_CHECKSUM_NUM_OF_HEADERS_CNS];/* auto checksum info */
    TGF_AUTO_CHECKSUM_EXTRA_INFO_STC extraChecksumInfoArr[TGF_AUTO_CHECKSUM_NUM_OF_HEADERS_CNS];/* extra checksum info */
    PRV_TGF_TRUNK_LBH_GLOBAL_MODE_ENT globalHashModeGet;
    PRV_TGF_TRUNK_LBH_CRC_MODE_ENT    crcHashModeGet;
    GT_U32                            crcSeedGet;
    PRV_TGF_TRUNK_LBH_CRC_MASK_ENTRY_STC maskEntry;

    numOfBytesInBuff = sizeof(packetBuffer);
    cpssOsMemSet(checksumInfoArr , 0, sizeof(checksumInfoArr));
    cpssOsMemSet(extraChecksumInfoArr , 0, sizeof(extraChecksumInfoArr));

    rc = tgfTrafficEnginePacketBuild(
        packetPtr, packetBuffer, &numOfBytesInBuff,
        checksumInfoArr, extraChecksumInfoArr);
    if (rc != GT_OK) goto label_restore;

    /* apply auto checksum fields */
    rc = tgfTrafficGeneratorPacketChecksumUpdate(
        checksumInfoArr, extraChecksumInfoArr);
    if (rc != GT_OK) goto label_restore;

    /* AUTODOC: Set Vlan And FDB configuration */
    rc = prvTgfPclUdbOnlyKeysPortVlanFdbSet();
    if (rc != GT_OK) goto label_restore;

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    mask.ruleIngrUdbOnly.replacedFld.portPclId2 = 0xFFFFFF;
    pattern.ruleIngrUdbOnly.replacedFld.portPclId2 = portPclId2;
    mask.ruleIngrUdbOnly.replacedFld.hash = 0xFFFFFFFF;
    pattern.ruleIngrUdbOnly.replacedFld.hash = 0x867ae83c;

    rc = prvTgfPclUdbOnlyKeysPclLegacyCfgSet(
        CPSS_PCL_DIRECTION_INGRESS_E,
        lookupNum,
        packetTypeIndex, /*0 - not IP, 1 - IPV4, 2 - IPV6*/
        ruleFormat,
        &mask, &pattern);
    if (rc != GT_OK) goto label_restore;

    rc = prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectSet(
        lookupNum, packetType, ruleFormat,
        0x7F000 /*udbReplaceBitmap - UDB12,13,14,15,16,17,18 replaced by portPclId2 and hash */,
        0 /*udbAmount*/,
        0 /*udbSelIndexBase*/,
        0 /*udbSelIndexIncrement*/);
    if (rc != GT_OK) goto label_restore;

    rc = cpssDxChPclPortPclId2Set(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        lookupNum, portPclId2);
    PRV_UTF_VERIFY_LOG_NO_STOP_MAC(rc, rc, "cpssDxChPclPortPclId2Set");
    if (rc != GT_OK) goto label_restore;

    /* Get the general hashing mode of trunk hash generation as CRC Hash based on the packet's data*/
    rc =  prvTgfTrunkHashGlobalModeGet(prvTgfDevNum,&globalHashModeGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashGlobalModeGet: %d", prvTgfDevNum);

    /* Set the general hashing mode of trunk hash generation as CRC Hash based on the packet's data*/
    rc =  prvTgfTrunkHashGlobalModeSet(PRV_TGF_TRUNK_LBH_PACKETS_INFO_CRC_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashGlobalModeSet: %d", prvTgfDevNum);

    /* get crc hash parameters */
    rc =  prvTgfTrunkHashCrcParametersGet(prvTgfDevNum, &crcHashModeGet, &crcSeedGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashCrcParametersGet: %d", prvTgfDevNum);

    /* set Enhanced CRC-Based Hash Mode */
    rc =  prvTgfTrunkHashCrcParametersSet(PRV_TGF_TRUNK_LBH_CRC_32_MODE_E, 0/*crcSeed*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashCrcParametersSet: %d", prvTgfDevNum);

    cpssOsMemSet(&maskEntry,0,sizeof(maskEntry));

    maskEntry.macDaMaskBmp      = 0x3f;
    maskEntry.macSaMaskBmp      = 0x3f;

    /* set HASH mask */
    rc =  prvTgfTrunkHashMaskCrcEntrySet(GT_FALSE,0,0,packetType,&maskEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashMaskCrcEntrySet: %d", prvTgfDevNum);

    /* AUTODOC: Double Tagged Packet */
    prvTgfPclUdbOnlyKeysVidTrafficGenerate(packetPtr);

    /* AUTODOC: Check that TAG0 VID is modified */
    prvTgfPclUdbOnlyKeysTrafficEgressVidCheck(PRV_TGF_PCL_NEW_VLAN_ID_CNS, GT_TRUE/*check 16 bit*/);

    label_restore:

    /* AUTODOC: Restore Vlan And FDB configuration */

    cpssDxChPclPortPclId2Set(
        prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        lookupNum, 0 /*portPclId2*/);

    prvTgfPclUdbOnlyKeysPortVlanFdbReset();

    prvTgfPclUdbOnlyKeysPclLegacyCfgReset(
        CPSS_PCL_DIRECTION_INGRESS_E,
        lookupNum,
        ruleSize);

    prvTgfPclUdbOnlyKeysUdbOnlyRangeSelectReset(
        lookupNum, packetType, ruleFormat);

    /* AUTODOC: restore enhanced crc hash data */
    /* restore global hash mode */
    rc =  prvTgfTrunkHashGlobalModeSet(globalHashModeGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashGlobalModeSet: %d", prvTgfDevNum);

    /* restore crc hash parameters */
    rc =  prvTgfTrunkHashCrcParametersSet(crcHashModeGet, crcSeedGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTrunkHashCrcParametersSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfPclUdbOnlyKeysUdbOnlyIngressPortPclId2Test function
* @endinternal
*
* @brief   PclId2 and hash Test on UDB Only Key
*
*/
GT_VOID prvTgfPclUdbOnlyKeysUdbOnlyIngressPortPclId2Test(GT_VOID)
{
    prvTgfPclUdbOnlyKeysUdbOnlyIngressPortPclId2GenericTest(
        CPSS_PCL_LOOKUP_NUMBER_0_E     /*lookupNum*/,
        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E    /*packetType*/,
        0 /*0 - not IP, 1 - IPV4, 2 - IPV6*/        /*packetTypeIndex*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_30_E    /*ruleFormat*/,
        CPSS_PCL_RULE_SIZE_30_BYTES_E               /*ruleSize*/,
        &prvTgfPacketNotTunneled2tagsEthOthInfo     /*packetPtr*/,
        0x123456                                    /*portPclId2*/);
}


/*====================================================*/
/* sample of using POST_TEST_EXIT for trace           */
/*====================================================*/

#if 0

static GT_VOID checkTtiEnableOnEgressPort
(
    IN const GT_CHAR*          suitNamePtr,
    IN const GT_CHAR*          testNamePtr
)
{
    GT_STATUS rc0, rc1, rc2;
    GT_BOOL   enableEth;
    GT_BOOL   enableIpv4;
    GT_BOOL   enableMpls;

    /* pach */
    rc0 = cpssDxChTtiPortLookupEnableGet(
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        PRV_TGF_TTI_KEY_ETH_E, &enableEth);

    rc1 = cpssDxChTtiPortLookupEnableGet(
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        PRV_TGF_TTI_KEY_IPV4_E, &enableIpv4);

    rc2 = cpssDxChTtiPortLookupEnableGet(
        prvTgfDevsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
        PRV_TGF_TTI_KEY_MPLS_E, &enableMpls);

    cpssOsPrintf(
        "\n<<<<<POST_TEST_EXIT %s.%s rc: %d, enable: %d >>>>>\n",
        suitNamePtr, testNamePtr,
        (rc0 + rc1 + rc2), (enableEth + enableIpv4 + enableMpls));
}

void prvTgfPclUdbOnlyKeysBindCheckTtiEnableOnEgressPort()
{
    utfDebugPostTestExitBind(checkTtiEnableOnEgressPort);
}

#endif

