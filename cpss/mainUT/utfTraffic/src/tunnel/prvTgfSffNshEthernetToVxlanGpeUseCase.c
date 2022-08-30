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
* @file prvTgfSffNshEthernetToVxlanGpeUseCase.c
*
* @brief SFF NSH Ethernet to VXLAN-GPE use cases testing.
*
*        Ports allocation:
*           ^ Port#0: Ingress port
*           ^ Port#1: Target port
*
*        Packet flow:
*           1.  Encapsulated NSH over Ethernet packet enters the device via Ingress Port#0
*           2.  Outer tunnel of L2, EtherType and VLAN tag is removed by TTI up to
*               (not included) NSH header
*           3.  After TT IPCL forward the packet based on its NSH content
*               If NSH<TTL> field = 1 than packet is trapped to CPU
*               If not redirects to ePort#1 pending NSH<Service Path Identifier> and
*               NSH<Service Index>
*           4.  packet is encapsulated with L2,EtherType,VLAN tag,IPv6,UDP and VXLAN-GPE
*               tunnel by HA unit
*           5.  PHA thread is triggered in which fw decrement NSH<TTL> field by 1
*           3.  The new encapsulated packet exits the device from target port#1.
*
*        Test description:
*        Send 2 packets from ingress port#0 to target port#1:
*           - First packet with NSH TTL value different than 1.
*             Expects Ethernet tunnel to be replaced with vxlan-gpe tunnel
*             and TTL value should be decremented by 1 by PHA
*           - Second packet with NSH TTL value equal to 1.
*             Expects packet to be trapped to CPU.
*             In this case target port counters should all be zero.
*
* @version   1
********************************************************************************
*/
#include <tunnel/prvTgfSffNshEthernetToVxlanGpeUseCase.h>
#include <oam/prvTgfOamGeneral.h>
#include <cpss/dxCh/dxChxGen/pha/cpssDxChPha.h>
#include <cpss/dxCh/dxChxGen/oam/cpssDxChOam.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgE2Phy.h>


/*************************** Constants definitions ***********************************/

/* PHA fw thread ID of SFF NSH Ethernet to VXLAN-GPE */
#define PRV_TGF_PHA_THREAD_ID_SFF_NSH_ETHERNET_TO_VXLAN_GPE_CNS  23

/* PHA fw thread ID to VXLAN-GBP */
#define PRV_TGF_PHA_THREAD_ID_TO_VXLAN_GBP_CNS  11

/* Size of output packet in bytes include CRC */
#define PRV_TGF_PACKET_SIZE_SFF_NSH_ETHERNET_TO_VXLAN_GPE_CNS   166

/* Number of packets */
#define PRV_TGF_BURST_COUNT_CNS             2

/* Template profile index */
#define PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS  1

/* Tunnel start entry index */
#define PRV_TGF_TUNNEL_START_INDEX_CNS      8

/* Ports number allocation */
#define PRV_TGF_INGRESS_PORT_IDX_CNS        0
#define PRV_TGF_TARGET_PORT_IDX_CNS         1

/* Default VLAN Id */
#define PRV_TGF_DEFAULT_VLANID_CNS          1
#define PRV_TGF_SEND_VLANID_CNS             2
/*  number of packets to send */
#define PRV_TGF_SEND_PACKETS_NUM_CNS            1
/* NSH UDP destination port number  */
#define PRV_TGF_NSH_UDP_DST_PORT_CNS        4790

/* VXLAN-GBP UDP destination port number  */
#define PRV_TGF_VXLAN_GBP_UDP_DST_PORT_CNS  4789

/* VXLAN-GBP UDP source port number  */
#define PRV_TGF_VXLAN_GBP_UDP_SRC_PORT_CNS  64212

/* VXLAN-GBP default UDP source port number  */
#define PRV_TGF_VXLAN_GBP_RFC_UDP_SRC_PORT_CNS  49152

#define PRV_TGF_VLANID_2_CNS 2

/* recv port number */
#define PRV_TGF_SEND_PORT_IDX_CNS                      1

/* send port number */
#define PRV_TGF_RECV_PORT_IDX_CNS                      2

/* capturing port number 1 */
#define PRV_TGF_CAPTURE_PORT1_IDX_CNS                  2

/* capturing port number 2 */
#define PRV_TGF_CAPTURE_PORT2_IDX_CNS                  3

/* Size of packet */
#define PRV_TGF_MAX_PACKET_SIZE_CNS                    160

/* Size of packets in bytes include CRC */
#define PRV_TGF_PACKET_SIZE_ERSPAN_II_CNS              160

/* Offset to IP header part in the output packet */
#define PRV_TGF_ETH_TYPE_PKT_OFFSET_CNS  (TGF_L2_HEADER_SIZE_CNS)

/* IPv4 addr offset in IPv4 header */
#define PRV_TGF_IPV4_ADDR_OFFSET_CNS                    14

/* IPv6 addr offset in IPv6 header */
#define PRV_TGF_IPV6_ADDR_OFFSET_CNS                    10

/* Offset to EtherType part in the output packet */
#define PRV_TGF_TUNNEL_ETHERTYPE_PKT_OFFSET_CNS    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS)

/* Offset to UDP part in the output packet */
#define PRV_TGF_TUNNEL_UDP_PKT_OFFSET_CNS    (PRV_TGF_TUNNEL_ETHERTYPE_PKT_OFFSET_CNS + \
                                              TGF_ETHERTYPE_SIZE_CNS                  + \
                                              TGF_IPV6_HEADER_SIZE_CNS)

/* Offset to UDP part in the output IPv6 packet */
#define PRV_TGF_IPV6_TUNNEL_UDP_PKT_OFFSET_CNS    (TGF_L2_HEADER_SIZE_CNS                  + \
                                                   TGF_ETHERTYPE_SIZE_CNS                  + \
                                                   TGF_IPV6_HEADER_SIZE_CNS)
/* Offset to UDP part in the output IPv4 packet */
#define PRV_TGF_IPV4_TUNNEL_UDP_PKT_OFFSET_CNS    (TGF_L2_HEADER_SIZE_CNS                  + \
                                                   TGF_ETHERTYPE_SIZE_CNS                  + \
                                                   TGF_IPV4_HEADER_SIZE_CNS)

/* Offset to NSH part in the output packet */
#define PRV_TGF_TUNNEL_NSH_PKT_OFFSET_CNS    (PRV_TGF_TUNNEL_UDP_PKT_OFFSET_CNS + \
                                              TGF_UDP_HEADER_SIZE_CNS           + \
                                              TGF_VXLAN_GPE_HEADER_SIZE_CNS)


/*************************** static DB definitions ***********************************/

/* Should holds NSH TTL value */
static GT_U8     nshTtlVal;

/* Should holds port counters */
static GT_U32    actualPacketsNum[PRV_TGF_BURST_COUNT_CNS];
static GT_U32    actualBytesNum[PRV_TGF_BURST_COUNT_CNS];

/* The TTI Rule index */
static GT_U32    prvTgfTtiRuleIndex;

/* DB to hold input packet information */
static TGF_PACKET_STC   packetInfo;

/* DB to hold target port interface */
static CPSS_INTERFACE_INFO_STC  targetPortInterface;

/* DB to hold relevant PHA threads information */
static CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC     commonInfo;
static CPSS_DXCH_PHA_THREAD_INFO_UNT            extInfo;
static PRV_TGF_TUNNEL_START_ENTRY_UNT           tunnelEntry;
static PRV_TGF_TUNNEL_START_GEN_IP_PROFILE_STC  profileData;

/* tagging array */
static GT_U8   prvTgfTagArray[] = {1, 1, 1, 0};

/******************* Packet parts definition *************************/

/* Input section: L2 part */
static TGF_PACKET_L2_STC prvTgfPacketInputL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x44, 0x04},    /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x45, 0x05}     /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketInputVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,         /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS            /* pri, cfi, VlanId */
};

/* packet's NSH ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketInputNshEtherTypePart = {
    TGF_ETHERTYPE_894F_NSH_TAG_CNS
};

/* packet's NSH data */
static TGF_PACKET_NSH_STC prvTgfPacketInputNshPart = {
    0x10c01010, /* Base header:         |Ver 2b|O 1b|U 1b|TTL 6b|Length 6b|Res 4b|MD Type 4b|Next protocol 8b| */
    0x20202323, /* Service Path header: |Service Path Identifier (SPI) 24b|Service Index 8b| */
    0x30303434, /* Metadata word 3:     |D 1b|F 2b|R 1b|Source Node ID 12b|Source Interface ID 16b| */
    0x40404545, /* Metadata word 2:     |Res 8b|Tenant ID 24b| */
    0x50505656, /* Metadata word 1:     |Res 16b|Source Class 16b| */
    0x60606767  /* Metadata word 0:     |Seconds 16b|Fraction 16b */
};

/* Original Ingress Packet: L2 part */
static TGF_PACKET_L2_STC prvTgfOriginalPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},    /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x03}     /* saMac */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x66, 0x66, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
    0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
    0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
    0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
    0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
    0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d,
    0x2e, 0x2f, 0x30, 0x31 /* length 52 bytes */
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),    /* dataLength */
    prvTgfPayloadDataArr             /* dataPtr */
};


/* Tunnel section: L2 part */
static TGF_PACKET_L2_STC prvTgfPacketTunnelL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x66, 0x06},    /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x77, 0x07}     /* saMac */
};

/* DB to hold IPv6 IP addresses */
static TGF_IPV6_ADDR prvTgfTunnelStartSrcIPv6 = {0x2001, 0x2002, 0x2003, 0x2004, 0x2005, 0x2006, 0x2007, 0x2008};
static TGF_IPV6_ADDR prvTgfTunnelStartDstIPv6 = {0x3001, 0x3002, 0x3003, 0x3004, 0x3005, 0x3006, 0x3007, 0x3008};
static TGF_IPV4_ADDR prvTgfTunnelStartSrcIPv4 = {0x20, 0x30, 0x40, 0x50};
static TGF_IPV4_ADDR prvTgfTunnelStartDstIPv4 = {0x30, 0x40, 0x50, 0x60};

static CPSS_INTERFACE_INFO_STC portInterface1;

/* PARTS of packet:
   |L2|VLAN|EtherType|NSH Base hdr|NSH Service Path hdr|NSH metadata|payload passenger|                 */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,           &prvTgfPacketInputL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,     &prvTgfPacketInputVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E,    &prvTgfPacketInputNshEtherTypePart},
    {TGF_PACKET_PART_NSH_E,          &prvTgfPacketInputNshPart},
    {TGF_PACKET_PART_L2_E,           &prvTgfOriginalPacketL2Part},
    {TGF_PACKET_PART_PAYLOAD_E,      &prvTgfPacketPayloadPart}
};

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* srcMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_2_CNS                          /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfVxPayloadDataArr[] =
{
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0F, 0x10, 0x11,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21,
    0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
    0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfVxPacketPayloadPart =
{
    sizeof(prvTgfVxPayloadDataArr),                       /* dataLength */
    prvTgfVxPayloadDataArr                                /* dataPtr */
};

/* PARTS of tagged packet */
static TGF_PACKET_PART_STC prvTgfTaggedPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfVxPacketPayloadPart}
};

/* LENGTH of TAGGED packet */
#define PRV_TGF_TAGGED_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* TAGGED PACKET to send */
static TGF_PACKET_STC prvTgfTaggedPacketInfo =
{
    PRV_TGF_TAGGED_PACKET_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfTaggedPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfTaggedPacketPartArray                                        /* partsArray */
};

/* type of traffic to send (tagged or untagged) */
static TGF_PACKET_STC* prvTgfPacketInfoArr[PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
    &prvTgfTaggedPacketInfo
};

/* send ports array */
static GT_U8 prvTgfSendPort[PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
    PRV_TGF_SEND_PORT_IDX_CNS
};

/* capture ports array for the tests */
static GT_U8 prvTgfCapturePortsArr[PRV_TGF_SEND_PACKETS_NUM_CNS] =
{
        PRV_TGF_CAPTURE_PORT1_IDX_CNS
};

/* array of destination MACs for the tests */
static TGF_MAC_ADDR prvTgfDaMacArr[PRV_TGF_SEND_PACKETS_NUM_CNS + 1] =
{
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x03}
};

/* parameters that are needed to be restored */
static struct
{
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC    egressInfo;
    GT_U8                               udbOfsetArray[PRV_TGF_TTI_MAX_UDB_CNS];
    PRV_TGF_TTI_OFFSET_TYPE_ENT         udbOffsetTypeArray[PRV_TGF_TTI_MAX_UDB_CNS];
    PRV_TGF_TTI_KEY_SIZE_ENT            keySize;
    GT_U32                              pclId;
    GT_BOOL                             pclTtForceVlanMode;
    CPSS_INTERFACE_INFO_STC             phyPortInfoGet;
    PRV_TGF_TTI_MAC_MODE_ENT            macMode;
    GT_BOOL                             enable;
} prvTgfRestoreCfg;



/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/


/**
* @internal prvTgfSffNshEthernetToVxlanGpeTtiConfigSet function
* @endinternal
*
* @brief   TTI test configurations
*          Tunnel terminate the packet - strip the header up to
*          (not including) the NSH header
*/
static GT_VOID prvTgfSffNshEthernetToVxlanGpeTtiConfigSet
(
    GT_VOID
)
{
    GT_STATUS                 rc = GT_OK;
    GT_U32                    i;
    GT_U8                     pclId;
    PRV_TGF_TTI_ACTION_STC    ttiAction;
    PRV_TGF_TTI_RULE_UNT      ttiPattern;
    PRV_TGF_TTI_RULE_UNT      ttiMask;


    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: Since same TCAM is used by both TTI and PCL need to set whether
                the TTI indexes are relative to the start of the TTI section */
    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiTcamIndexIsRelativeSet");

    /* AUTODOC: enable the TTI lookup for Ethernet type at port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E,
                                      GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: Save pclId for restoration */
    rc = prvTgfTtiPclIdGet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E,&prvTgfRestoreCfg.pclId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdGet");

    /* AUTODOC: set pclId for TTI key type of IPv6  */
    pclId = 0x11;
    rc = prvTgfTtiPclIdSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E,pclId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdSet");


    /***************** Set TTI Rule Pattern, Mask *************************************/

    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));

    /* set TTI Pattern & Mask */
    /* PCL ID */
    ttiPattern.udbArray.udb[0] = pclId;
    ttiMask.udbArray.udb[0] = 0x1f;

    /* Local Device Source ePort */
    ttiPattern.udbArray.udb[1] = (GT_U8)prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS];
    ttiMask.udbArray.udb[1] = 0xff;

    /* NSH Ethertype */
    ttiPattern.udbArray.udb[2] = (GT_U8)((prvTgfPacketInputNshEtherTypePart.etherType >> 8) & 0xff);
    ttiMask.udbArray.udb[2] = 0xff;
    ttiPattern.udbArray.udb[3] = (GT_U8)(prvTgfPacketInputNshEtherTypePart.etherType & 0xff);
    ttiMask.udbArray.udb[3] = 0xff;


    /********************** Setting UDB Configuration  *******************************/

    /* save TTI key size for restore */
    rc = prvTgfTtiPacketTypeKeySizeGet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E,&prvTgfRestoreCfg.keySize);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeGet: %d", prvTgfDevNum);

    /* Set TTI key size (20B) */
    rc = prvTgfTtiPacketTypeKeySizeSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E , PRV_TGF_TTI_KEY_SIZE_10_B_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);

    /* save udb for restore*/
    for(i=0; i < 4; i++)
    {
        rc = prvTgfTtiUserDefinedByteGet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E,
                                         i,
                                         &prvTgfRestoreCfg.udbOffsetTypeArray[i],
                                         &prvTgfRestoreCfg.udbOfsetArray[i]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteGet: %d", prvTgfDevNum);
    }

    /* For PCL ID */
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E,0,
                                     PRV_TGF_TTI_OFFSET_METADATA_E,22);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    /* For local source ePort */
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E,1,
                                     PRV_TGF_TTI_OFFSET_METADATA_E,26);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    /* For NSH EtherType */
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E,2,
                                     PRV_TGF_TTI_OFFSET_L2_E,16);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E,3,
                                     PRV_TGF_TTI_OFFSET_L2_E,17);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    /************************* set TTI Action **************************************/
    /* Set to tunnel terminate */
    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));
    prvTgfTtiRuleIndex = 2;
    ttiAction.command                        = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction.redirectCommand                = PRV_TGF_TTI_NO_REDIRECT_E;
    ttiAction.tunnelTerminate                = GT_TRUE;
    ttiAction.passengerPacketType            = PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E;
    ttiAction.interfaceInfo.type             = CPSS_INTERFACE_PORT_E;
    ttiAction.interfaceInfo.devPort.hwDevNum = prvTgfDevNum;
    ttiAction.interfaceInfo.devPort.portNum  = prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS];
    rc = prvTgfTtiRuleSet(prvTgfTtiRuleIndex,PRV_TGF_TTI_RULE_UDB_10_E, &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet");
}


/**
* @internal prvTgfSffNshEthernetToVxlanGpeIpclConfigSet function
* @endinternal
*
* @brief   IPCL test configurations
*          Forward the packet based on its NSH
*          Two rules:
*          - Rule that checks if TTL=1, and traps to the CPU.
*          - Rule that uses <Service Path Identifier>, <Service Index>, and redirects to ePort#1
*/
static GT_VOID prvTgfSffNshEthernetToVxlanGpeIpclConfigSet
(
    GT_VOID
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT         mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT         pattern;
    PRV_TGF_PCL_ACTION_STC              action;
    GT_U32                              ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT    ruleFormat;
    GT_U8                               i;


    PRV_UTF_LOG0_MAC("======= Setting IPCL Configuration =======\n");

    /* AUTODOC: Get PCL TT force vlan mode for restore */
    rc = cpssDxCh3PclTunnelTermForceVlanModeEnableGet(prvTgfDevNum, &prvTgfRestoreCfg.pclTtForceVlanMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxCh3PclTunnelTermForceVlanModeEnableGet");

    /* AUTODOC: Force TT packets assigned to PCL according to ingress port and not by vlan */
    rc = cpssDxCh3PclTunnelTermForceVlanModeEnableSet(prvTgfDevNum, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxCh3PclTunnelTermForceVlanModeEnableSet");

    /* AUTODOC: init PCL Engine for Ingress PCL for ingress port */
    rc = prvTgfPclDefPortInit(
            prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclDefPortInit");


    /************************* mask & pattern (first rule) **********************************/
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    /* Service Path Identifier (SPI) MS byte */
    mask.ruleStdNotIp.udb[15] = 0xFF;
    pattern.ruleStdNotIp.udb[15] = (GT_U8)((prvTgfPacketInputNshPart.servicePathHdr >> 24) & 0xFF);
    /* Service Path Identifier (SPI) middle byte */
    mask.ruleStdNotIp.udb[16] = 0xFF;
    pattern.ruleStdNotIp.udb[16] = (GT_U8)((prvTgfPacketInputNshPart.servicePathHdr >> 16) & 0xFF);
    /* Service Path Identifier (SPI) LS byte */
    mask.ruleStdNotIp.udb[17] = 0xFF;
    pattern.ruleStdNotIp.udb[17] = (GT_U8)((prvTgfPacketInputNshPart.servicePathHdr >> 8) & 0xFF);
    /* Service Index */
    mask.ruleStdNotIp.udb[23] = 0xFF;
    pattern.ruleStdNotIp.udb[23] = (GT_U8)(prvTgfPacketInputNshPart.servicePathHdr & 0xFF);

    /*********************** UDB configuration (first rule) *******************************/
    /* NOTE: Can only use UDB number 15,16,17,23,24,25,26 !!! */
    for (i=0;i<3;i++)
    {
        /* Set UDB configuration for SPI (3 bytes) */
        rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                                    PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                    CPSS_PCL_DIRECTION_INGRESS_E,
                                    15+i,   /* UDP index */
                                    PRV_TGF_PCL_OFFSET_L2_E,
                                    (4+i)); /* skip the first 4B (NSH Base hdr) to point to Service Path hdr */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d", prvTgfDevNum);
    }

    /* Set UDB configuration for Service Index (1 byte) */
    rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                                PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                CPSS_PCL_DIRECTION_INGRESS_E,
                                23,  /* UDP index */
                                PRV_TGF_PCL_OFFSET_L2_E,
                                7);  /* point to Service Index */
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d", prvTgfDevNum);


    /*********************** PCL action (first rule) **************************************/
    /* Redirect packet to target port, bypass bridge */
    cpssOsMemSet(&action, 0, sizeof(action));
    ruleIndex                           = 2;
    ruleFormat                          = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    action.pktCmd                       = CPSS_PACKET_CMD_FORWARD_E;
    action.bypassBridge                 = GT_TRUE;
    action.redirect.redirectCmd         = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    action.redirect.data.outIf.outInterface.type             = CPSS_INTERFACE_PORT_E;
    action.redirect.data.outIf.outInterface.devPort.hwDevNum = prvTgfDevNum;
    action.redirect.data.outIf.outInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS];
    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclRuleSet, prvTgfDevNum:%d,"
                                            "ruleFormat:%d, ruleIndex:%d", prvTgfDevNum, ruleFormat, ruleIndex);


    /************************* mask & pattern (second rule)*********************************/
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    /* Set mask & pattern for NSH TTL value 1. This field is splited between 2 bytes */
    /* First byte holds NSH TTL[5:2] */
    mask.ruleStdNotIp.udb[24] = 0x0F;    /* mask 4LS bits of the MS byte 0x0F*/
    pattern.ruleStdNotIp.udb[24] = 0x00; /* to represent 1 clear 4 LS bits to zero */

    /* Next byte holds NSH TTL[1:0] */
    mask.ruleStdNotIp.udb[25] = 0xC0;    /* mask 2MS bits of the next byte 0xC0*/
    pattern.ruleStdNotIp.udb[25] = 0x40; /* to represent 1 set the 2 MS bits to 1 */


    /*********************** UDB configuration (second rule) *******************************/
    /* Use the next valid free UDB number 24 & 25 */
    for (i=0;i<2;i++)
    {
        rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                                    PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                    CPSS_PCL_DIRECTION_INGRESS_E,
                                    24+i,  /* UDP index 4*/
                                    PRV_TGF_PCL_OFFSET_L2_E,
                                    i);    /* points to 2 MSB of NSH base header where TTL field is */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d", prvTgfDevNum);
    }

    /*********************** PCL action (second rule) **************************************/
    /* Trap to CPU, bypass bridge */
    cpssOsMemSet(&action, 0, sizeof(action));
    ruleIndex                = 0;
    ruleFormat               = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    action.pktCmd            = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    action.mirror.cpuCode    = CPSS_NET_FIRST_USER_DEFINED_E;
    action.bypassBridge      = GT_TRUE;
    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclRuleSet, prvTgfDevNum:%d,"
                                            "ruleFormat:%d, ruleIndex:%d", prvTgfDevNum, ruleFormat, ruleIndex);
}


/**
* @internal prvTgfSffNshEthernetToVxlanGpeBridgeConfigSet function
* @endinternal
*
* @brief   Bridge test configurations
*/
static GT_VOID prvTgfSffNshEthernetToVxlanGpeBridgeConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U16      vlanId = PRV_TGF_SEND_VLANID_CNS;

    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* Set vlan entry with the requested ports */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(vlanId,
                                           prvTgfPortsArray, /* array of requested ports */
                                           NULL,             /* Assume all ports belong to devNum */
                                           NULL,             /* Assume all ports untagged */
                                           2);               /* Number of ports in the array */
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);
}


/**
* @internal prvTgfSffNshEthernetToVxlanGpeHaConfigSet function
* @endinternal
*
* @brief   HA test configurations
*/
static GT_VOID prvTgfSffNshEthernetToVxlanGpeHaConfigSet
(
    GT_VOID
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC    egressInfo;
    GT_U32                              i;


    PRV_UTF_LOG0_MAC("======= Setting HA Configuration =======\n");

    /* AUTODOC: Save ePort attributes configuration */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                                                       prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                                       &(prvTgfRestoreCfg.egressInfo));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoGet");

    /* AUTODOC: Set target port attributes configuration */
    /* AUTODOC: Tunnel Start = ENABLE, tunnelStartPtr = 8, tsPassenger = ETHERNET */
    cpssOsMemSet(&egressInfo,0,sizeof(egressInfo));
    egressInfo = prvTgfRestoreCfg.egressInfo;
    egressInfo.tunnelStart            = GT_TRUE;
    egressInfo.tunnelStartPtr         = PRV_TGF_TUNNEL_START_INDEX_CNS;
    egressInfo.tsPassengerPacketType  = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                                       &egressInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoSet");

    /* AUTODOC: Set Tunnel Start entry 8 with: */
    /* AUTODOC: Type=CPSS_TUNNEL_GENERIC_IPV6_E, tagEnable=TRUE, vlanId=1, ipHeaderProtocol=UDP */
    /* AUTODOC: udpSrcPort = 0x2000, udpDstPort=4790, MACDA = 00:00:00:00:66:06 */
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));

    /* Set Tunnel IPv6 addresses */
    for (i=0; i<8; i++)
    {
        tunnelEntry.ipv6Cfg.destIp.arIP[i*2]     = (GT_U8)(prvTgfTunnelStartDstIPv6[i] >> 8);
        tunnelEntry.ipv6Cfg.destIp.arIP[i*2 + 1] = (GT_U8) prvTgfTunnelStartDstIPv6[i];
        tunnelEntry.ipv6Cfg.srcIp.arIP[i*2]      = (GT_U8)(prvTgfTunnelStartSrcIPv6[i] >> 8);
        tunnelEntry.ipv6Cfg.srcIp.arIP[i*2 + 1]  = (GT_U8) prvTgfTunnelStartSrcIPv6[i];
    }
    tunnelEntry.ipv6Cfg.tagEnable        = GT_TRUE;
    tunnelEntry.ipv6Cfg.vlanId           = PRV_TGF_DEFAULT_VLANID_CNS;
    tunnelEntry.ipv6Cfg.ttl              = 20;
    tunnelEntry.ipv6Cfg.ipHeaderProtocol = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
    tunnelEntry.ipv6Cfg.profileIndex     = PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS;
    tunnelEntry.ipv6Cfg.udpSrcPort       = 0x2000;
    tunnelEntry.ipv6Cfg.udpDstPort       = PRV_TGF_NSH_UDP_DST_PORT_CNS; /* 4790 (0x12B6) */
    cpssOsMemCpy(tunnelEntry.ipv6Cfg.macDa.arEther, prvTgfPacketTunnelL2Part.daMac, sizeof(TGF_MAC_ADDR));
    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_INDEX_CNS, CPSS_TUNNEL_GENERIC_IPV6_E, &tunnelEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartEntrySet");

    /* AUTODOC: configure tunnel start profile for Classifier NSH over VXLAN-GPE */
    cpssOsMemSet(&profileData,0,sizeof(profileData));
    profileData.templateDataSize = PRV_TGF_TUNNEL_START_TEMPLATE_DATA_SIZE_8B_E; /* 8 bytes */
    rc = prvTgfTunnelStartGenProfileTableEntrySet(prvTgfDevNum,
                                                  PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS,
                                                  &profileData);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartGenProfileTableEntrySet");


    /**** PHA fw threads configurations ****/
    /* AUTODOC: Assign SFF NSH Ethernet to vxlan-gpe fw thread to target port */
    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                        GT_TRUE, PRV_TGF_PHA_THREAD_ID_SFF_NSH_ETHERNET_TO_VXLAN_GPE_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaPortThreadIdSet");

    /* AUTODOC: Set the thread entry - PRV_TGF_PHA_THREAD_ID_SFF_NSH_ETHERNET_TO_VXLAN_GPE_CNS */
    cpssOsMemSet(&commonInfo, 0, sizeof(commonInfo));
    extInfo.notNeeded = 0;
    commonInfo.statisticalProcessingFactor = 0;
    commonInfo.busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;
    commonInfo.stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;
    rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
                                     PRV_TGF_PHA_THREAD_ID_SFF_NSH_ETHERNET_TO_VXLAN_GPE_CNS,
                                     &commonInfo,
                                     CPSS_DXCH_PHA_THREAD_TYPE_SFF_NSH_ETHERNET_TO_VXLAN_GPE_E,
                                     &extInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaThreadIdEntrySet");
}

/**
* internal prvTgfSffNshEthernetToVxlanGpeConfigSet function
* @endinternal
*
* @brief   SFF NSH Ethernet to VXLAN-GPE use case configurations
*          Includes the follwoing configurations:
*          - TTI
*          - IPCL
*          - Bridge
*          - HA
*/
GT_VOID prvTgfSffNshEthernetToVxlanGpeConfigSet
(
    GT_VOID
)
{
    /* TTI Configuration */
    prvTgfSffNshEthernetToVxlanGpeTtiConfigSet();

    /* IPCL Configuration */
    prvTgfSffNshEthernetToVxlanGpeIpclConfigSet();

    /* Bridge Configuration */
    prvTgfSffNshEthernetToVxlanGpeBridgeConfigSet();

    /* HA Configuration */
    prvTgfSffNshEthernetToVxlanGpeHaConfigSet();
}



/**
* @internal prvTgfSffNshEthernetToVxlanGpeConfigRestore function
* @endinternal
*
* @brief   SFF NSH Ethernet to VXLAN-GPE configurations restore.
* @note    1. Restore TTI Configuration
*          2. Restore IPCL Configuration
*          3. Restore Bridge Configuration
*          4. Restore HA Configuration
*          5. Restore base Configuration
*/
GT_VOID prvTgfSffNshEthernetToVxlanGpeConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      i;

    PRV_UTF_LOG0_MAC("\n======= Restore configurations =======\n");

    /* -------------------------------------------------------------------------
     * 1. Restore TTI Configuration
     */
    /* AUTODOC: invalidate TTI rule */
    rc = prvTgfTtiRuleValidStatusSet(prvTgfTtiRuleIndex, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d", prvTgfDevNum);

    /* AUTODOC: disable the TTI lookup for Ethernet other at port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                       PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: restore key size for PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E */
    rc = prvTgfTtiPacketTypeKeySizeSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E, prvTgfRestoreCfg.keySize);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);

    /* AUTODOC: restore pclId */
    rc = prvTgfTtiPclIdSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E, prvTgfRestoreCfg.pclId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdSet: %d", prvTgfDevNum);

    /* AUTODOC: restore udb */
    for(i=0; i<20; i++)
    {
        rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E,i,prvTgfRestoreCfg.udbOffsetTypeArray[i],prvTgfRestoreCfg.udbOfsetArray[i]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    }

    /* AUTODOC: Disable relative index for TTI TCAM  */
    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTtiTcamIndexIsRelativeSet");


    /* -------------------------------------------------------------------------
     * 2. Restore IPCL Configuration
     */
    /* AUTODOC: Set previous PCL TT force vlan mode */
    rc = cpssDxCh3PclTunnelTermForceVlanModeEnableSet(prvTgfDevNum, prvTgfRestoreCfg.pclTtForceVlanMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxCh3PclTunnelTermForceVlanModeEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: Invalidate PCL rule 0 */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, 0, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclRuleValidStatusSet, index = %d", 0);

    /* AUTODOC: Invalidate PCL rule 2 */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, 2, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclRuleValidStatusSet, index = %d", 2);

    /* AUTODOC: disable all PCL configurations on the ports */
    prvTgfPclPortsRestoreAll();


    /* -------------------------------------------------------------------------
     * 3. Restore Bridge Configuration
     */
    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgFdbFlush");

    /* AUTODOC: invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* -------------------------------------------------------------------------
     * 4. Restore HA Configuration
     */
    /* AUTODOC: restore ePort attributes */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                                       &(prvTgfRestoreCfg.egressInfo));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoSet");

    /* AUTODOC: Clear PHA fw thread from target port */
    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum,
                                    prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                    GT_FALSE,
                                    0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaPortThreadIdSet");


    /* -------------------------------------------------------------------------
     * 5. Restore base Configuration
     */
    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "Failure in tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Disable RX capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "Failure in tgfTrafficTableRxStartCapture");

    /* AUTODOC: reset hw counters */
    rc = prvTgfCommonAllBridgeCntrsReset();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfCommonAllCntrsReset");
}



/**
* internal tgfSffNshEthernetToVxlanGpePacketVerification function
* @endinternal
*
* @brief  SFF NSH Ethernet to VXLAN-GPE use case verification
*         Do the following:
*
*         - Check Tunnel Ethertype = IPv6 type
*         - Check IPv6<Next Header> = UDP type
*         - Check UDP<destination port> = NSH port number
*         - Check NSH Base header TTL field was decrment by one
*         - Check NSH Base header rest of fields were not overwritten
*         - Check NSH Service Path header was not overwritten
*         - Check NSH metadata was not overwritten
*         - Check MAC addresses of passenger are not damaged
*         - Check payload data of passenger is not damaged
*/
GT_VOID tgfSffNshEthernetToVxlanGpePacketVerification
(
    IN  GT_U8  * packetBuf
)
{
    GT_U32              pktOffset,payloadPktOffset;
    GT_U32              nshValActualVal,nshValExpectedVal;
    GT_U32              i;
    GT_U32              daMacErr=0, saMacErr=0, passengerPayloadErr=0;
    TGF_PACKET_L2_STC   receivedMac;
    GT_U32              recPassengerPayload, expPassengerPayload;

    PRV_UTF_LOG0_MAC("======= Check content of packet =======\n");

    PRV_UTF_LOG0_MAC("======= Check Tunnel Ethertype field  =======\n");

    /* AUTODOC: Update offset to point to Tunnel Ethertype field */
    pktOffset = PRV_TGF_TUNNEL_ETHERTYPE_PKT_OFFSET_CNS;

    /* Read Ethertype from packet */
    nshValActualVal = (GT_U32)((packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));
    nshValActualVal >>=16;

    /* Should be IPv6 Type (0x86DD) */
    nshValExpectedVal = TGF_ETHERTYPE_86DD_IPV6_TAG_CNS;

    /* Check expected vs actual and notify if mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(nshValExpectedVal, nshValActualVal,
                                "--- Failure in tgfClassifierNshOverEthernetVerification --- \n"
                                "Tunnel Ethertype is not as expected => Expected:0x%x != Received:0x%x \n",
                                 nshValExpectedVal, nshValActualVal);

    PRV_UTF_LOG0_MAC("======= Check IPv6 Next Header field  =======\n");

    /* AUTODOC: Update offset to point to IPv6 Next header field field */
    pktOffset += 8;

    /* Read IPv6<Next Header> from packet */
    nshValActualVal = (GT_U32)((packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));
    nshValActualVal >>=24;

    /* Should be UDP type (0x11) */
    nshValExpectedVal = TGF_PROTOCOL_UDP_E;

    /* Check expected vs actual and notify if mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(nshValExpectedVal, nshValActualVal,
                                "--- Failure in tgfClassifierNshOverEthernetVerification --- \n"
                                "IPv6<Next protocol> field is not as expected => Expected:0x%x != Received:0x%x \n",
                                 nshValExpectedVal, nshValActualVal);


    PRV_UTF_LOG0_MAC("======= Check UDP destination port field  =======\n");

    /* AUTODOC: Update offset to point to UDP<destination port> field */
    pktOffset = PRV_TGF_TUNNEL_UDP_PKT_OFFSET_CNS + 2;

    /* Read UDP<destination port> from packet */
    nshValActualVal = (GT_U32)((packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));
    nshValActualVal >>=16;

    /* Should be 4790 (0x12b6) */
    nshValExpectedVal = PRV_TGF_NSH_UDP_DST_PORT_CNS;

    /* Check expected vs actual and notify if mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(nshValExpectedVal, nshValActualVal,
                                "--- Failure in tgfClassifierNshOverEthernetVerification --- \n"
                                "UDP<destination port> field is not as expected => Expected:0x%x != Received:0x%x \n",
                                 nshValExpectedVal, nshValActualVal);


    PRV_UTF_LOG0_MAC("======= Check NSH Base header  =======\n");

    /* AUTODOC: Update offset to point to UDP<destination port> field */
    pktOffset = PRV_TGF_TUNNEL_NSH_PKT_OFFSET_CNS;


    /********************* Check NSH Base hdr (TTL field) ************************/
    /* Read TTL field from packet(extract 6 bits) */
    nshValActualVal = (GT_U32)((packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));
    nshValActualVal = (nshValActualVal>>22) & 0x3F;

    /* Get expected TTL value from origin packet.
       Decrement by 1 since we expect PHA to do that */
    nshValExpectedVal = nshTtlVal;
    nshValExpectedVal -=1;
    nshValExpectedVal &= 0x3F;

    /* Check expected vs actual and notify if mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(nshValExpectedVal, nshValActualVal,
                                "--- Failure in tgfClassifierNshOverEthernetVerification --- \n"
                                "NSH Base hdr (TTL) field is not as expected => Expected:0x%x != Received:0x%x \n",
                                 nshValExpectedVal, nshValActualVal);


    /********************* Check NSH Base hdr (|Ver|O|U|) ************************/
    /* Get |Ver|O|U| (4b) fields from packet */
    nshValActualVal = (GT_U32)((packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));
    nshValActualVal = (nshValActualVal >> 28) & 0xF ;

    /* Get expected |Ver|O|U| (4b) value from origin packet */
    nshValExpectedVal = (prvTgfPacketInputNshPart.baseHdr >> 28) & 0x3F;

    /* Check expected vs actual and notify if mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(nshValExpectedVal, nshValActualVal,
                                "--- Failure in tgfClassifierNshOverEthernetVerification --- \n"
                                "NSH Base hdr (|Ver|O|U|) fields are not as expected => Expected:0x%x != Received:0x%x \n",
                                 nshValExpectedVal, nshValActualVal);


    /********************* Check NSH Base hdr (Length) ************************/
    /* Get Length (6b) field from packet */
    nshValActualVal = (GT_U32)((packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));
    nshValActualVal = (nshValActualVal >> 16) & 0x3F ;

    /* Get expected Length (6b) value from origin packet */
    nshValExpectedVal = (prvTgfPacketInputNshPart.baseHdr >> 16) & 0x3F;

    /* Check expected vs actual and notify if mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(nshValExpectedVal, nshValActualVal,
                                "--- Failure in tgfClassifierNshOverEthernetVerification --- \n"
                                "NSH Base hdr (Length) field is not as expected => Expected:0x%x != Received:0x%x \n",
                                 nshValExpectedVal, nshValActualVal);


    /********************* Check NSH Base hdr (|Res|MD Type|Next protocol|) ************************/
    /* AUTODOC: increment offset by 2 bytes */
    pktOffset +=2 ;

    /* Read |Res|MD Type|Next protocol| fields from packet */
    nshValActualVal = (GT_U32)((packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));
    nshValActualVal = (nshValActualVal>>16) & 0xFFFF;

    /* Get expected |Res|MD Type|Next protocol| fields from origin packet */
    nshValExpectedVal = prvTgfPacketInputNshPart.baseHdr & 0xFFFF;

    /* Check expected vs actual and notify if mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(nshValExpectedVal, nshValActualVal,
                                "--- Failure in tgfClassifierNshOverEthernetVerification --- \n"
                                "NSH Base hdr (|Res|MD Type|Next protocol|) are not as expected => Expected:0x%x != Received:0x%x \n",
                                 nshValExpectedVal, nshValActualVal);


    PRV_UTF_LOG0_MAC("======= Check NSH Service Path header  =======\n");

    /* AUTODOC: increment offset by 2 bytes to point to start of NSH Service Path */
    pktOffset +=2 ;

    /* Get NSH Service Path hdr from packet */
    nshValActualVal = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                               (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));

    /* Get expected NSH Service Path hdr value from origin packet */
    nshValExpectedVal = prvTgfPacketInputNshPart.servicePathHdr;

    /* Check expected vs actual and notify if mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(nshValExpectedVal, nshValActualVal,
                                "--- Failure in tgfClassifierNshOverEthernetVerification --- \n"
                                "NSH Service Path hdr is not as expected => Expected:0x%x != Received:0x%x \n",
                                 nshValExpectedVal, nshValActualVal);


    PRV_UTF_LOG0_MAC("======= Check NSH metadata  =======\n");

    /********************* Check NSH Metadata word3 ************************/
    /* AUTODOC: increment offset by 4 bytes to point to start of NSH metadata */
    pktOffset +=4 ;

    /* Get NSH Metadata word3 from packet */
    nshValActualVal = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                              (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));

    /* Get expected NSH metadata word3 value from origin packet */
    nshValExpectedVal = prvTgfPacketInputNshPart.metaDataWord3Hdr;

    /* Check expected vs actual and notify if mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(nshValExpectedVal, nshValActualVal,
                                "--- Failure in tgfClassifierNshOverEthernetVerification --- \n"
                                "NSH metadata word3 is not as expected => Expected:0x%x != Received:0x%x \n",
                                 nshValExpectedVal, nshValActualVal);


    /********************* Check NSH Metadata word2 ************************/
    /* AUTODOC: increment offset by 4 bytes to point to word2 */
    pktOffset +=4 ;

    /* Get NSH Metadata word2 from packet */
    nshValActualVal = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                               (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));

    /* Get expected NSH metadata word2 value from origin packet */
    nshValExpectedVal = prvTgfPacketInputNshPart.metaDataWord2Hdr;

    /* Check expected vs actual and notify if mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(nshValExpectedVal, nshValActualVal,
                                "--- Failure in tgfClassifierNshOverEthernetVerification --- \n"
                                "NSH metadata word2 is not as expected => Expected:0x%x != Received:0x%x \n",
                                 nshValExpectedVal, nshValActualVal);


    /********************* Check NSH Metadata word1 ************************/
    /* AUTODOC: increment offset by 4 bytes to point to word1 */
    pktOffset +=4 ;

    /* Get NSH Metadata word1 from packet */
    nshValActualVal = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                               (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));

    /* Get NSH metadata word1 value from origin packet */
    nshValExpectedVal = prvTgfPacketInputNshPart.metaDataWord1Hdr;

    /* Check expected vs actual and notify if mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(nshValExpectedVal, nshValActualVal,
                                "--- Failure in tgfClassifierNshOverEthernetVerification --- \n"
                                "NSH metadata word1 is not as expected => Expected:0x%x != Received:0x%x \n",
                                 nshValExpectedVal, nshValActualVal);


    /********************* Check NSH Metadata word0 ************************/
    /* AUTODOC: increment offset by 4 bytes to point to word0 */
    pktOffset +=4 ;

    /* Get NSH Metadata word0 from packet */
    nshValActualVal = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                               (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));

    /* Get expected NSH metadata word0 value from origin packet */
    nshValExpectedVal = prvTgfPacketInputNshPart.metaDataWord0Hdr;

    /* Check expected vs actual and notify if mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(nshValExpectedVal, nshValActualVal,
                                "--- Failure in tgfClassifierNshOverEthernetVerification --- \n"
                                "NSH metadata word0 is not as expected => Expected:0x%x != Received:0x%x \n",
                                 nshValExpectedVal, nshValActualVal);


    PRV_UTF_LOG0_MAC("======= Check MAC DA SA  =======\n");

    /* Increment offset by 8B to point to MAC DA  */
    pktOffset +=4;

    /* AUTODOC: Get actual MAC DA & MAC SA */
    for (i=0;i<(TGF_L2_HEADER_SIZE_CNS/2);i++)
    {
        receivedMac.daMac[i] = packetBuf[pktOffset + i];
        receivedMac.saMac[i] = packetBuf[pktOffset + i + sizeof(prvTgfOriginalPacketL2Part.daMac)];

        /* Decrment received with expected to check if there is any mismatch */
        daMacErr += receivedMac.daMac[i] - prvTgfOriginalPacketL2Part.daMac[i] ;
        saMacErr += receivedMac.saMac[i] - prvTgfOriginalPacketL2Part.saMac[i] ;
    }

    /********************* Compare MAC DA ********************************************/
    /* AUTODOC: compare received MAC DA vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL0_STRING_MAC(0, daMacErr,
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "Mismatch in MAC DA address");
    if (daMacErr != 0)
    {
        /* Print received and expected MAC DA */
        PRV_UTF_LOG12_MAC("Expected MAC DA:%.2x%.2x%.2x%.2x%.2x%.2x \n"
                          "Received MAC DA:%.2x%.2x%.2x%.2x%.2x%.2x \n",
                          prvTgfOriginalPacketL2Part.daMac[0],prvTgfOriginalPacketL2Part.daMac[1],prvTgfOriginalPacketL2Part.daMac[2],
                          prvTgfOriginalPacketL2Part.daMac[3],prvTgfOriginalPacketL2Part.daMac[4],prvTgfOriginalPacketL2Part.daMac[5],
                          receivedMac.daMac[0],receivedMac.daMac[1],receivedMac.daMac[2],
                          receivedMac.daMac[3],receivedMac.daMac[4],receivedMac.daMac[5]);
    }

    /********************* Compare MAC SA ********************************************/
    /* AUTODOC: compare received MAC SA vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL0_STRING_MAC(0, saMacErr,
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "Mismatch in MAC SA address \n");
    if (saMacErr != 0)
    {
        /* Print received and expected MAC SA */
        PRV_UTF_LOG12_MAC("Expected MAC SA:%.2x%.2x%.2x%.2x%.2x%.2x \n"
                          "Received MAC SA:%.2x%.2x%.2x%.2x%.2x%.2x \n",
                          prvTgfOriginalPacketL2Part.saMac[0],prvTgfOriginalPacketL2Part.saMac[1],prvTgfOriginalPacketL2Part.saMac[2],
                          prvTgfOriginalPacketL2Part.saMac[3],prvTgfOriginalPacketL2Part.saMac[4],prvTgfOriginalPacketL2Part.saMac[5],
                          receivedMac.saMac[0],receivedMac.saMac[1],receivedMac.saMac[2],
                          receivedMac.saMac[3],receivedMac.saMac[4],receivedMac.saMac[5]);
    }


    PRV_UTF_LOG0_MAC("======= Check Passenger payload  =======\n\n");

    /* Increment offset by 12B of MACs to point to passenger payload (EtherType)  */
    pktOffset +=TGF_L2_HEADER_SIZE_CNS;

    /* Anchor offset to passenger */
    payloadPktOffset = pktOffset;

    /* AUTODOC: Get actual passenger payload  */
    for (i=0;i<prvTgfPacketPayloadPart.dataLength;i+=4)
    {
        /* Update offset every 4 bytes */
        pktOffset = payloadPktOffset + i;

        /* Get actual payload */
        recPassengerPayload = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                                       (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));
        /* Get expected payload */
        expPassengerPayload = (GT_U32)(prvTgfPayloadDataArr[i+3] | (prvTgfPayloadDataArr[i+2] << 8) |
                                           (prvTgfPayloadDataArr[i+1] << 16) | (prvTgfPayloadDataArr[i] << 24));

        /* Decrement received with expected to check if there is any mismatch */
        passengerPayloadErr += (recPassengerPayload - expPassengerPayload) ;
    }

    /* AUTODOC: compare received passenger payload vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL0_STRING_MAC(0, passengerPayloadErr,
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "Mismatch in passenger payload data ");
}


/**
* internal tgfSffNshEthernetToVxlanGpePortCountersVerification function
* @endinternal
*
* @brief  Compare expected vs actual target port counters
*         Target port counters were already read upon packets transmission
*/
GT_VOID tgfSffNshEthernetToVxlanGpePortCountersVerification
(
    IN  GT_U32    expPacketsNum,
    IN  GT_U32    actualPacketsNum,
    IN  GT_U32    expBytesNum,
    IN  GT_U32    actualBytesNum
)
{
    PRV_UTF_LOG0_MAC("======= Check target port counters  =======\n\n");

    /* Check expected vs actual and notify if mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(expPacketsNum, actualPacketsNum,
                                 "--- Failure in tgfSffNshEthernetToVxlanGpePortCountersVerification --- \n"
                                 "Packets number is not as expected => Expected:0x%x != Received:0x%x \n",
                                  expPacketsNum, actualPacketsNum);

    /* Check expected vs actual and notify if mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(expBytesNum, actualBytesNum,
                                 "--- Failure in tgfSffNshEthernetToVxlanGpePortCountersVerification --- \n"
                                 "Bytes number is not as expected => Expected:0x%x != Received:0x%x \n",
                                  expBytesNum, actualBytesNum);
}


/**
* internal prvTgfSffNshEthernetToVxlanGpeVerification function
* @endinternal
*
* @brief  Loop over packets and pending packet type check packet
*         content and target port counters
*/
GT_VOID prvTgfSffNshEthernetToVxlanGpeVerification
(
    GT_VOID
)
{
    GT_STATUS          rc = GT_OK;
    GT_U8              packetBuf[PRV_TGF_BURST_COUNT_CNS][PRV_TGF_MAX_PACKET_SIZE_CNS]= {{0}, {0}};
    GT_U32             packetLen[PRV_TGF_BURST_COUNT_CNS] = {PRV_TGF_PACKET_SIZE_SFF_NSH_ETHERNET_TO_VXLAN_GPE_CNS,
                                                             PRV_TGF_PACKET_SIZE_SFF_NSH_ETHERNET_TO_VXLAN_GPE_CNS};

    GT_U32             packetActualLength = 0;
    GT_U8              queue = 0;
    GT_U8              dev = 0;
    TGF_NET_DSA_STC    rxParam;
    GT_U8              pktNum;
    GT_BOOL            getFirst = GT_TRUE;
    GT_U32             expPacketsNum, expBytesNum;

    /* Loop over packets */
    for (pktNum = 0; pktNum<PRV_TGF_BURST_COUNT_CNS; pktNum++)
    {
        if (pktNum==0)
        { /* First packet includes NSH TTL value different than 1
             Expects packet tunnel to be replaced from VXLAN-GPE to Ethernet */

            PRV_UTF_LOG0_MAC("======= Check first packet  =======\n");
            /* Expects packet to arrive to target port */
            expPacketsNum = 1;
            /* New packet format should be as shown below with total 110 bytes
               |L2|vlan|EtherType|NSH base hdr|NSH Service Path hdr|NSH metadata|
               |payload passenger|CRC| */
            expBytesNum = packetLen[pktNum];
            /* AUTODOC: Check port counters */
            tgfSffNshEthernetToVxlanGpePortCountersVerification(expPacketsNum,
                                                                actualPacketsNum[pktNum],
                                                                expBytesNum,
                                                                actualBytesNum[pktNum]);

            /* Get the captured packets */
            rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&targetPortInterface,
                                                    TGF_PACKET_TYPE_CAPTURE_E,
                                                    getFirst, GT_TRUE, packetBuf[pktNum],
                                                    &packetLen[pktNum], &packetActualLength,
                                                    &dev, &queue, &rxParam);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet");

            /* Check packet content */
            tgfSffNshEthernetToVxlanGpePacketVerification(packetBuf[pktNum]);

            /* Set to false for next iteration in order to get the next packet from next Rx CPU entry */
            getFirst = GT_FALSE;
        }
        else if (pktNum==1)
        { /* Second packet includes NSH TTL value equal to 1
             Expects packet to be trapped to CPU. Should not arrived to Target port */

            PRV_UTF_LOG0_MAC("\n======= Check second packet  =======\n");
            /* Since NSH TTL value is '1' packet should be trapped to CPU and not arrive
               to target port therefore all target port counters should be zero */
            expPacketsNum = 0;
            expBytesNum = 0;

            /* Check port counters */
            tgfSffNshEthernetToVxlanGpePortCountersVerification(expPacketsNum,
                                                                actualPacketsNum[pktNum],
                                                                expBytesNum,
                                                                actualBytesNum[pktNum]);
        }
    }
}


/**
* @internal prvTgfSffNshEthernetToVxlanGpeTest function
* @endinternal
*
* @brief   SFF NSH VXLAN-GPE to Ethernet use case test.
*          Create tunnel which includes IPv6, UDP, VXLAN-GPE, NSH
*          headers over Ethernet packet. During this process PHA
*          fw thread is triggered in which NSH metadata is
*          added. At the end packets are captured and content is
*          checked to verify right behaviour.
*          Send 2 packets from ingress port#0 to target port#1:
*          - First packet with NSH TTL value different than 1
*            expects vxlan-gpe tunnel to be replaced with Ethernet tunnel
*            and TTL value should be decremented by 1 by PHA
*          - Second packet with NSH TTL value equal to 1
*            expects packet to be trapped to CPU.
*            Target port counters should all be zero.
*/
GT_VOID prvTgfSffNshEthernetToVxlanGpeTest
(
    GT_VOID
)
{
    GT_STATUS                       rc = GT_OK;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    GT_U32                          portIter,packetSize;
    GT_U8                           nshTtlLocalVal;

    PRV_UTF_LOG0_MAC("======= Ports allocation =======\n");
    PRV_UTF_LOG1_MAC("Port [%d]: Ingress Port \n", prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    PRV_UTF_LOG1_MAC("Port [%d]: Target Port \n", prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS]);

    /* AUTODOC: reset counters for all 4 ports */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "Failure in prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficTableRxPcktTblClear");

    /* Enable capture on target port */
    targetPortInterface.type             = CPSS_INTERFACE_PORT_E;
    targetPortInterface.devPort.hwDevNum = prvTgfDevNum;
    targetPortInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&targetPortInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

    /* Set input packet interface structure  */
    packetInfo.numOfParts =  sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);

    /* calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketPartArray, packetInfo.numOfParts, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* build packet */
    packetInfo.totalLen = packetSize;

    packetInfo.partsArray = prvTgfPacketPartArray;
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");


    PRV_UTF_LOG0_MAC("\n======= Send first packet =======\n");

    /* Make sure NSH base hdr<TTL> value different than '1' therefore set it to '3' */
    nshTtlLocalVal = 3;
    prvTgfPacketInputNshPart.baseHdr &=0xF03FFFFF ;
    prvTgfPacketInputNshPart.baseHdr |= (nshTtlLocalVal<<22);

    /* Save NSH TTL value (6b) from first packet to be verified later */
    nshTtlVal = (prvTgfPacketInputNshPart.baseHdr >> 22) & 0x3F;

    /* Send first packet  */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStartTransmitingEth");

    /* Read counters from target port and save them to be verified */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                   GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfReadPortCountersEth");
    actualPacketsNum[0] =  portCntrs.goodPktsRcv.l[0];
    actualBytesNum[0] = portCntrs.goodOctetsRcv.l[0];


    PRV_UTF_LOG0_MAC("======= Send second packet =======\n");

    /* Set NSH TTL value to '1' */
    nshTtlLocalVal = 1;
    prvTgfPacketInputNshPart.baseHdr &=0xF03FFFFF ;
    prvTgfPacketInputNshPart.baseHdr |= (nshTtlLocalVal<<22);

    /* Send second packet  */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStartTransmitingEth");

    /* Read counters from target port and save them to be verified */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                   GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfReadPortCountersEth");
    actualPacketsNum[1] =  portCntrs.goodPktsRcv.l[0];
    actualBytesNum[1] = portCntrs.goodOctetsRcv.l[0];

    /* Disable capture on target port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &targetPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");
}

/**
* @internal prvTgfVxlanGbpTtiConfigSet function
* @endinternal
*
* @brief   TTI test configurations
*          Tunnel terminate the packet - strip the header up to
*          (not including) the NSH header
*/
static GT_VOID prvTgfVxlanGbpTtiConfigSet
(
    GT_VOID
)
{
    GT_STATUS                 rc = GT_OK;
    PRV_TGF_TTI_ACTION_2_STC  ttiAction;
    PRV_TGF_TTI_RULE_UNT      ttiPattern;
    PRV_TGF_TTI_RULE_UNT      ttiMask;
    GT_U8                     pclId = 0x11;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* enable the TTI lookup for PRV_TGF_TTI_KEY_ETH_E */
    rc = prvTgfTtiPortLookupEnableGet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_ETH_E, &prvTgfRestoreCfg.enable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableGet");
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_ETH_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: Save pclId for restoration */
    rc = prvTgfTtiPclIdGet(prvTgfDevNum,PRV_TGF_TTI_KEY_ETH_E,&prvTgfRestoreCfg.pclId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdGet");

    /* AUTODOC: set pclId for TTI key type of IPv6  */
    rc = prvTgfTtiPclIdSet(prvTgfDevNum, PRV_TGF_TTI_KEY_ETH_E,pclId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdSet");

    rc = prvTgfTtiMacModeGet(prvTgfDevNum, PRV_TGF_TTI_KEY_ETH_E, &prvTgfRestoreCfg.macMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeGet");
    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_ETH_E, PRV_TGF_TTI_MAC_MODE_DA_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet");

    /* clear entries */
    cpssOsMemSet((GT_VOID*) &ttiAction, 0, sizeof(ttiAction));
    cpssOsMemSet((GT_VOID*) &ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet((GT_VOID*) &ttiMask, 0, sizeof(ttiMask));

    /* AUTODOC: TTI Mask for MAC */
    cpssOsMemSet(ttiMask.eth.common.mac.arEther, 0xFF, sizeof(GT_ETHERADDR));

    /* AUTODOC: TTI Pattern for MAC DA */
    cpssOsMemCpy(ttiPattern.eth.common.mac.arEther,
                 prvTgfPacketL2Part.daMac,
                 sizeof(GT_ETHERADDR));

    /* TTI actions - FORWARD packet, assign copyReserved = daMac[5] + 3 to test
     * different copy_reserved bits extraction
     * IPv6 : Bit 0-1 => Group Policy ID = 1
     * IPv4 : Bit 0-2 => Group Policy ID = 5
     */
    ttiAction.command = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction.copyReservedAssignmentEnable = GT_TRUE;
    ttiAction.copyReserved = prvTgfPacketL2Part.daMac[5] + 3;

    prvTgfTtiRuleIndex = 0x6;
    rc = prvTgfTtiRule2Set(prvTgfTtiRuleIndex, PRV_TGF_TTI_KEY_ETH_E,
                           &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet");
}

/**
* @internal prvTgfVxlanGbpBridgeConfigSet function
* @endinternal
*
* @brief   Bridge test configurations
*/
static GT_VOID prvTgfVxlanGbpBridgeConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 2 with tagged ports 0, 1, 2, untagged port 3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_2_CNS,
                                           prvTgfPortsArray, NULL,
                                           prvTgfTagArray, prvTgfPortsNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:02, VLAN 2, port 2 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfDaMacArr[0],
                                          PRV_TGF_VLANID_2_CNS,
                                          prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
                                          GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:03, VLAN 2, port 3 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfDaMacArr[1],
                                          PRV_TGF_VLANID_2_CNS,
                                          prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS + 1],
                                          GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    /* AUTODOC: set PVID to 2 for port 1 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 PRV_TGF_VLANID_2_CNS);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 PRV_TGF_VLANID_2_CNS);

}

/**
* @internal prvTgfVxlanGbpHaConfigSet function
* @endinternal
*
* @brief   HA test configurations
*/
static GT_VOID prvTgfVxlanGbpHaConfigSet
(
    GT_BOOL protocol,
    GT_BOOL udpSrcPortGeneric
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC    egressInfo;
    GT_U32                              i;
    GT_HW_DEV_NUM                       hwDevNum;                   /* HW device number */

    /* HW devNum */
    rc = cpssDxChCfgHwDevNumGet(prvTgfDevNum, &hwDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCfgHwDevNumGet");

    PRV_UTF_LOG0_MAC("======= Setting HA Configuration =======\n");

    /* AUTODOC: Save ePort attributes configuration */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                                                       prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
                                                       &(prvTgfRestoreCfg.egressInfo));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoGet");

    /* AUTODOC: Set target port attributes configuration */
    /* AUTODOC: Tunnel Start = ENABLE, tunnelStartPtr = 8, tsPassenger = ETHERNET */
    cpssOsMemSet(&egressInfo,0,sizeof(egressInfo));
    egressInfo = prvTgfRestoreCfg.egressInfo;
    egressInfo.tunnelStart            = GT_TRUE;
    egressInfo.tunnelStartPtr         = PRV_TGF_TUNNEL_START_INDEX_CNS;
    egressInfo.tsPassengerPacketType  = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
                                                       &egressInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoSet");

    /* AUTODOC: Set Tunnel Start entry 8 with: */
    /* AUTODOC: Type=CPSS_TUNNEL_GENERIC_IPV6_E, tagEnable=TRUE, vlanId=1, ipHeaderProtocol=UDP */
    /* AUTODOC: udpSrcPort = 0x2000, udpDstPort=4789, MACDA = 00:00:00:00:66:06 */
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));

    if(protocol == 1)
    {
        /* Set Tunnel IPv6 addresses */
        for (i=0; i<8; i++)
        {
            tunnelEntry.ipv6Cfg.destIp.arIP[i*2]     = (GT_U8)(prvTgfTunnelStartDstIPv6[i] >> 8);
            tunnelEntry.ipv6Cfg.destIp.arIP[i*2 + 1] = (GT_U8) prvTgfTunnelStartDstIPv6[i];
            tunnelEntry.ipv6Cfg.srcIp.arIP[i*2]      = (GT_U8)(prvTgfTunnelStartSrcIPv6[i] >> 8);
            tunnelEntry.ipv6Cfg.srcIp.arIP[i*2 + 1]  = (GT_U8) prvTgfTunnelStartSrcIPv6[i];
        }
        tunnelEntry.ipv6Cfg.tagEnable        = GT_FALSE;
        tunnelEntry.ipv6Cfg.vlanId           = 1;
        tunnelEntry.ipv6Cfg.ttl              = 20;
        tunnelEntry.ipv6Cfg.ipHeaderProtocol = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
        tunnelEntry.ipv6Cfg.profileIndex     = PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS;
        tunnelEntry.ipv6Cfg.udpSrcPort       = PRV_TGF_VXLAN_GBP_UDP_SRC_PORT_CNS;
        tunnelEntry.ipv6Cfg.udpDstPort       = PRV_TGF_VXLAN_GBP_UDP_DST_PORT_CNS; /* 4789 (0x12B5) */
        cpssOsMemCpy(tunnelEntry.ipv6Cfg.macDa.arEther, prvTgfPacketTunnelL2Part.daMac, sizeof(TGF_MAC_ADDR));
        rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_INDEX_CNS, CPSS_TUNNEL_GENERIC_IPV6_E, &tunnelEntry);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartEntrySet");
    }
    else
    {
        /* Set Tunnel IPv4 addresses */
        for (i=0; i<4; i++)
        {
            tunnelEntry.ipv4Cfg.destIp.arIP[i]     = (GT_U8)(prvTgfTunnelStartDstIPv4[i]);
            tunnelEntry.ipv4Cfg.srcIp.arIP[i]      = (GT_U8)(prvTgfTunnelStartSrcIPv4[i]);
        }
        tunnelEntry.ipv4Cfg.tagEnable        = GT_FALSE;
        tunnelEntry.ipv4Cfg.vlanId           = 1;
        tunnelEntry.ipv4Cfg.ttl              = 20;
        tunnelEntry.ipv4Cfg.ipHeaderProtocol = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
        tunnelEntry.ipv4Cfg.profileIndex     = PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS;
        tunnelEntry.ipv4Cfg.udpSrcPort       = PRV_TGF_VXLAN_GBP_UDP_SRC_PORT_CNS;
        tunnelEntry.ipv4Cfg.udpDstPort       = PRV_TGF_VXLAN_GBP_UDP_DST_PORT_CNS; /* 4789 (0x12B5) */
        cpssOsMemCpy(tunnelEntry.ipv4Cfg.macDa.arEther, prvTgfPacketTunnelL2Part.daMac, sizeof(TGF_MAC_ADDR));
        rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_INDEX_CNS, CPSS_TUNNEL_GENERIC_IPV4_E, &tunnelEntry);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartEntrySet");
    }

    /* AUTODOC: configure tunnel start profile for VXLAN-GBP */
    cpssOsMemSet(&profileData,0,sizeof(profileData));
    profileData.udpSrcPortMode          = (udpSrcPortGeneric == 1) ? CPSS_DXCH_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_IP_GEN_TS_ENTRY_E:
                                                                     CPSS_DXCH_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_PACKET_HASH_VALUE_E;
    profileData.templateDataSize        = PRV_TGF_TUNNEL_START_TEMPLATE_DATA_SIZE_8B_E; /* 8 bytes */
    profileData.templateDataBitsCfg[7]  = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_1_E; /* G Bit */
    profileData.templateDataBitsCfg[3]  = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_1_E; /* I bit */
    profileData.templateDataBitsCfg[14] = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_1_E; /* D Bit */
    profileData.templateDataBitsCfg[11] = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_1_E; /* A Bit */
    profileData.templateDataBitsCfg[51] = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_1_E; /* VNI, 0x18 in our case*/
    profileData.templateDataBitsCfg[52] = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_1_E;

    rc = prvTgfTunnelStartGenProfileTableEntrySet(prvTgfDevNum,
                                                  PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS,
                                                  &profileData);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartGenProfileTableEntrySet");


    /**** PHA fw threads configurations ****/
    /* AUTODOC: Assign vxlan-gbp fw thread to target port */
    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
                                        GT_TRUE, PRV_TGF_PHA_THREAD_ID_TO_VXLAN_GBP_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaPortThreadIdSet");

    /* AUTODOC: Set the thread entry - PRV_TGF_PHA_THREAD_ID_TO_VXLAN_GBP_CNS */
    cpssOsMemSet(&commonInfo, 0, sizeof(commonInfo));
    extInfo.vxlanGbpSourceGroupPolicyId.copyReservedLsb = 0;
    /* PHA FW will set VXLAN-GBP <Group Policy ID> field based on copyReserved [MSB:LSB]
     * IPv6 use bits 1:0 => Group Policy ID = 1
     * IPv4 use bits 0:2 => Group Policy ID = 5
     */
    extInfo.vxlanGbpSourceGroupPolicyId.copyReservedMsb = (protocol == 1) ? 1 : 2;
    commonInfo.statisticalProcessingFactor = 0;
    commonInfo.busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;
    commonInfo.stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;
    rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
                                     PRV_TGF_PHA_THREAD_ID_TO_VXLAN_GBP_CNS,
                                     &commonInfo,
                                     CPSS_DXCH_PHA_THREAD_TYPE_VXLAN_GBP_SOURCE_GROUP_POLICY_ID_E,
                                     &extInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaThreadIdEntrySet");
}


/**
* internal prvTgfVxlanGbpConfigSet function
* @endinternal
*
* @brief   VXLAN-GBP use case configurations
*          Includes the follwoing configurations:
*          - TTI
*          - Bridge
*          - HA
* @param[in] protocol       - VXLAN tunnel is ipv4 or ipv6 based
*                            GT_TRUE - VXLAN tunnel is ipv6 based
*                            GT_FALSE - VXLAN tunnel is ipv4 based
* @param[in] udpSrcPortMode - VXLAN tunnel UDP source port mode
*                           GT_TRUE - source port mode is based on IP generic
*                           GT_FALSE - source port mode is based on packet hash
*
*/
GT_VOID prvTgfVxlanGbpConfigSet
(
    GT_BOOL protocol,
    GT_BOOL udpSrcPortMode
)
{
    /* TTI Configuration */
    prvTgfVxlanGbpTtiConfigSet();
    /* Bridge Configuration */
    prvTgfVxlanGbpBridgeConfigSet();
    /* HA Configuration */
    prvTgfVxlanGbpHaConfigSet(protocol, udpSrcPortMode);
}

/**
* @internal prvTgfVxlanGbpTest function
* @endinternal
*
* @brief   SFF NSH VXLAN-GPE to Ethernet use case test.
*          Create tunnel which includes IPv6, UDP, VXLAN-GPE, NSH
*          headers over Ethernet packet. During this process PHA
*          fw thread is triggered in which NSH metadata is
*          added. At the end packets are captured and content is
*          checked to verify right behaviour.
*          Send 2 packets from ingress port#0 to target port#1:
*          - First packet with NSH TTL value different than 1
*            expects vxlan-gpe tunnel to be replaced with Ethernet tunnel
*            and TTL value should be decremented by 1 by PHA
*          - Second packet with NSH TTL value equal to 1
*            expects packet to be trapped to CPU.
*            Target port counters should all be zero.
*/
GT_VOID prvTgfVxlanGbpTest
(
    GT_VOID
)
{
    GT_STATUS                           rc             = GT_OK;
    GT_U32                              portIter       = 0;
    GT_U32                              sendIter       = 0;
    CPSS_INTERFACE_INFO_STC             portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC       portCntrs;

    /* AUTODOC: GENERATE TRAFFIC: */

    PRV_UTF_LOG0_MAC("======= Sending packets =======\n\n");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* enable capture  */
    portInterface.type              = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum   = prvTgfPortsArray[prvTgfCapturePortsArr[sendIter]];

    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[prvTgfSendPort[sendIter]]);

    /* AUTODOC:   configure 1 tagged packets to send: */
    /* AUTODOC:   1 packet - DA=00:00:00:00:00:02, SA=00:00:00:00:00:01, VID=2 */

    /* set destination MAC address in Packet */
    cpssOsMemCpy(prvTgfPacketL2Part.daMac, prvTgfDaMacArr[sendIter],
                 sizeof(prvTgfPacketL2Part.daMac));

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, prvTgfPacketInfoArr[sendIter], 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup");

    PRV_UTF_LOG2_MAC("========== Sending packet iter=%d  on port=%d==========\n\n",
                     sendIter + 1, prvTgfPortsArray[prvTgfSendPort[sendIter]]);

    /* AUTODOC: send configured packets on port 1 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[prvTgfSendPort[sendIter]]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d",
                                 prvTgfDevNum, prvTgfPortsArray[prvTgfSendPort[sendIter]]);

    /* disable capture */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, portInterface.devPort.portNum);

    rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS], GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfReadPortCountersEth");
    rc = prvTgfPortCapturedPacketPrint(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS]);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "Failure in prvTgfPortCapturedPacketPrint");
}


GT_VOID prvTgfVxlanGbpConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIter       = 0;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: restore TTI configuration */
    rc = prvTgfTtiRuleValidStatusSet(prvTgfTtiRuleIndex, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d", prvTgfDevNum);

    rc = prvTgfTtiMacModeSet(PRV_TGF_TTI_KEY_ETH_E, prvTgfRestoreCfg.macMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiMacModeSet");

    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_ETH_E, prvTgfRestoreCfg.enable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: restore pclId */
    rc = prvTgfTtiPclIdSet(prvTgfDevNum,PRV_TGF_TTI_KEY_ETH_E, prvTgfRestoreCfg.pclId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdSet: %d", prvTgfDevNum);

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTtiTcamIndexIsRelativeSet");

    /* AUTODOC: restore HA configuration */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
                                                       &(prvTgfRestoreCfg.egressInfo));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoGet");

    /* AUTODOC: restore PHA configurations */
    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
                                    GT_FALSE, PRV_TGF_PHA_THREAD_ID_TO_VXLAN_GBP_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaPortThreadIdSet");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: restore Bridge configurations */
    /* invalidate VLAN 2 entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_2_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate 0");

    /* AUTODOC: restore default PVID on port 1 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                             1);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d",
                             prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                             1);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",prvTgfDevNum, GT_TRUE);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

}

static GT_VOID prvTgfVxlanGbpPacketVerification
(
    IN  GT_U8   *packetBuf,
    IN  GT_BOOL proto,
    IN  GT_BOOL udpSrcPortMode
)
{
    GT_U32              pktOffset = 0, payloadPktOffset;
    GT_U32              vxlanFlags, vxlanVni, vxlanGroupPolicyIdReceived, vxlanGroupPolicyIdExpected;
    GT_U32              i;
    GT_U32              daMacErr=0, saMacErr=0, passengerPayloadErr=0, saIpErr = 0, daIpErr = 0, protocol = 0;
    GT_U32              recPassengerPayload, expPassengerPayload;
    TGF_PACKET_L2_STC   receivedMac;
    TGF_PACKET_L2_STC   vxlanReceivedMacDa, vxlanReceivedMacSa;
    TGF_IPV4_ADDR       vxlanReceivedSrcAddr, vxlanReceivedDstAddr;
    GT_U16              vxlanReceivedSrc6Addr[8], vxlanReceivedDst6Addr[8];
    GT_U16              daIp6Err = 0, saIp6Err = 0;
    GT_U16              ethTypeReceived, ethTypeExpected;
    GT_U16              udpSrcPortReceived, udpSrcPortExpected, udpDstPortReceived, udpDstPortExpected;

    cpssOsMemSet(&vxlanReceivedMacDa.daMac[0], 0, sizeof(TGF_MAC_ADDR));
    cpssOsMemSet(&vxlanReceivedMacSa.saMac[0], 0, sizeof(TGF_MAC_ADDR));
    cpssOsMemSet(&receivedMac.daMac[0], 0, sizeof(TGF_MAC_ADDR));
    cpssOsMemSet(&receivedMac.saMac[0], 0, sizeof(TGF_MAC_ADDR));

    PRV_UTF_LOG0_MAC("======= Packet verification of VXLAN-GBP =======\n");

    /* AUTODOC: Get ERSPAN MAC DA & MAC SA */
    for (i=0;i<(TGF_L2_HEADER_SIZE_CNS/2);i++)
    {
        vxlanReceivedMacDa.daMac[i] = packetBuf[pktOffset + i];
        /* Decrment received with expected to check if there is any mismatch */
        daMacErr += vxlanReceivedMacDa.daMac[i] - prvTgfPacketTunnelL2Part.daMac[i] ;
    }

    /* AUTODOC: compare received MAC DA vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL0_STRING_MAC(0, daMacErr,
                                "--- Failure in prvTgfVxlanGbpPacketVerification --- \n"
                                "Mismatch in MAC DA address");
    if (daMacErr != 0)
    {
        /* Print received and expected MAC DA */
        PRV_UTF_LOG12_MAC("Expected MAC DA:%.2x%.2x%.2x%.2x%.2x%.2x \n"
                          "Received MAC DA:%.2x%.2x%.2x%.2x%.2x%.2x \n",
                          prvTgfPacketTunnelL2Part.daMac[0],prvTgfPacketTunnelL2Part.daMac[1],prvTgfPacketTunnelL2Part.daMac[2],
                          prvTgfPacketTunnelL2Part.daMac[3],prvTgfPacketTunnelL2Part.daMac[4],prvTgfPacketTunnelL2Part.daMac[5],
                          vxlanReceivedMacDa.daMac[0],vxlanReceivedMacDa.daMac[1],vxlanReceivedMacDa.daMac[2],
                          vxlanReceivedMacDa.daMac[3],vxlanReceivedMacDa.daMac[4],vxlanReceivedMacDa.daMac[5]);
    }

    /*********************** VXLAN Eth Type Check *********************************************/
    /* AUTODOC: Get VXLAN Packet Ether Type */
    pktOffset = PRV_TGF_ETH_TYPE_PKT_OFFSET_CNS;
    ethTypeReceived = (packetBuf[pktOffset + 1] | (packetBuf[pktOffset] << 8));
    ethTypeExpected = (proto == 0) ? TGF_ETHERTYPE_0800_IPV4_TAG_CNS : TGF_ETHERTYPE_86DD_IPV6_TAG_CNS;
    UTF_VERIFY_EQUAL0_STRING_MAC(ethTypeExpected, ethTypeReceived,
                                "--- Failure in prvTgfVxlanGbpPacketVerification --- \n"
                                "Mismatch in Ether Type \n");

    protocol = (packetBuf[pktOffset + 2] >> 4) & 0xF;
    pktOffset += (protocol == 4) ? PRV_TGF_IPV4_ADDR_OFFSET_CNS : PRV_TGF_IPV6_ADDR_OFFSET_CNS;

    if(protocol == 4)
    {
        /* AUTODOC: Get ERSPAN SRC IP & DST IP */
        for (i=0;i<(TGF_IPV4_HEADER_SIZE_CNS/5);i++)
        {
            vxlanReceivedSrcAddr[i] = packetBuf[pktOffset + i];
            vxlanReceivedDstAddr[i] = packetBuf[pktOffset + i + sizeof(prvTgfTunnelStartSrcIPv4)];

            /* Decrment received with expected to check if there is any mismatch */
            saIpErr += vxlanReceivedSrcAddr[i] - prvTgfTunnelStartSrcIPv4[i] ;
            daIpErr += vxlanReceivedDstAddr[i] - prvTgfTunnelStartDstIPv4[i] ;
        }

        /* AUTODOC: compare received SRC IP vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL0_STRING_MAC(0, saIpErr,
                                    "--- Failure in prvTgfVxlanGbpPacketVerification --- \n"
                                    "Mismatch in SRC IP address");
        if (saIpErr != 0)
        {
            /* Print received and expected SRC IP */
            PRV_UTF_LOG8_MAC("Expected SRC IP ADDR:%.2x%.2x%.2x%.2x \n"
                              "Received SRC IP ADDR:%.2x%.2x%.2x%.2x \n",
                              prvTgfTunnelStartSrcIPv4[0],prvTgfTunnelStartSrcIPv4[1],
                              prvTgfTunnelStartSrcIPv4[2],prvTgfTunnelStartSrcIPv4[3],
                              vxlanReceivedSrcAddr[0],vxlanReceivedSrcAddr[1],
                              vxlanReceivedSrcAddr[2],vxlanReceivedSrcAddr[3]);
        }

        /*********************** Check Origin DST IP ************************************/
        /* AUTODOC: compare received DST IP vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL0_STRING_MAC(0, daIpErr,
                                    "--- Failure in prvTgfVxlanGbpPacketVerification --- \n"
                                    "Mismatch in DST IP address \n");
        if (daIpErr != 0)
        {
            /* Print received and expected DST IP */
            PRV_UTF_LOG8_MAC("Expected DST IP ADDR:%.2x%.2x%.2x%.2x \n"
                              "Received DST IP ADDR:%.2x%.2x%.2x%.2x \n",
                              prvTgfTunnelStartSrcIPv4[0],prvTgfTunnelStartSrcIPv4[1],
                              prvTgfTunnelStartSrcIPv4[2],prvTgfTunnelStartSrcIPv4[3],
                              vxlanReceivedDstAddr[0],vxlanReceivedDstAddr[1],
                              vxlanReceivedDstAddr[2],vxlanReceivedDstAddr[3]);
        }
    }
    else
    {
        /* AUTODOC: Get VXLAN SRC IP & DST IP */
        for (i=0;i<16;i+=2)
        {
            vxlanReceivedSrc6Addr[i/2] = (GT_U16)((packetBuf[pktOffset + i] << 8)| (packetBuf[pktOffset + i + 1]));
            vxlanReceivedDst6Addr[i/2] = ((packetBuf[pktOffset + i + sizeof(prvTgfTunnelStartSrcIPv6)] << 8)|
                                        (packetBuf[pktOffset + i + 1 + sizeof(prvTgfTunnelStartSrcIPv6)]));

            /* Decrment received with expected to check if there is any mismatch */
            saIp6Err = vxlanReceivedSrc6Addr[i/2] - prvTgfTunnelStartSrcIPv6[i/2];
            daIp6Err = vxlanReceivedDst6Addr[i/2] - prvTgfTunnelStartDstIPv6[i/2];

            /* AUTODOC: compare received SRC IP vs expected and notify if there is any mismatch */
            UTF_VERIFY_EQUAL0_STRING_MAC(0, saIp6Err,
                                        "--- Failure in prvTgfVxlanGbpPacketVerification --- \n"
                                        "Mismatch in SRC IP address at offset\n");
            if (saIp6Err != 0)
            {
                /* Print received and expected SRC IP */
                PRV_UTF_LOG2_MAC("Expected byte of SRC IP ADDR:%.2x \n"
                                  "Received byte of SRC IP ADDR:%.2x \n",
                                  prvTgfTunnelStartSrcIPv6[i/2],
                                  vxlanReceivedSrc6Addr[i/2]);
            }

            /*********************** Check Origin DST IP ************************************/
            /* AUTODOC: compare received DST IP vs expected and notify if there is any mismatch */
            UTF_VERIFY_EQUAL0_STRING_MAC(0, daIp6Err,
                                        "--- Failure in prvTgfVxlanGbpPacketVerification --- \n"
                                        "Mismatch in DST IP address \n");
            if (daIp6Err != 0)
            {
                /* Print received and expected DST IP */
                PRV_UTF_LOG2_MAC("Expected byte of DST IP ADDR:%.2x \n"
                                  "Received byte of DST IP ADDR:%.2x \n",
                                  prvTgfTunnelStartDstIPv6[i/2],
                                  vxlanReceivedDst6Addr[i/2]);
            }
        }
    }

    pktOffset = (protocol == 4) ? PRV_TGF_IPV4_TUNNEL_UDP_PKT_OFFSET_CNS : PRV_TGF_IPV6_TUNNEL_UDP_PKT_OFFSET_CNS;

    /*********************** Check UDP Header *********************************************/

    /* AUTODOC: get the first 2B of UDP header to verify UDP Source Port */
    udpSrcPortReceived = (GT_U32)((packetBuf[pktOffset] << 8) | (packetBuf[pktOffset + 1]));
    /* AUTODOC: get the first 2B of GRE header to verify UDP Source Port */
    /* NOTE: 'Src UDP port' set by descrPtr->pktHash[0x000] 12 LSbits , and 4 MSbits are HardCoded 0xC (as RFC6335 : recommends 'UDP src port' in 'dynamic/private'
       port range of 49152-65535) !!!*/
    udpSrcPortExpected = (udpSrcPortMode == 1) ? PRV_TGF_VXLAN_GBP_UDP_SRC_PORT_CNS : PRV_TGF_VXLAN_GBP_RFC_UDP_SRC_PORT_CNS;

    /* AUTODOC: compare received UDP Source Port vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(udpSrcPortExpected, udpSrcPortReceived,
                                "--- Failure in prvTgfVxlanGbpPacketVerification --- \n"
                                "UDP Source Port is not as expected => Expected:0x%x != Received:0x%x \n",
                                 udpSrcPortExpected, udpSrcPortReceived);

    /* AUTODOC: get the next 2B of UDP header to verify UDP Dest Port */
    udpDstPortReceived = (GT_U32)((packetBuf[pktOffset + 2] << 8) | (packetBuf[pktOffset + 3]));
    /* AUTODOC: get the next 2B of GRE header to verify UDP Dest Port */
    udpDstPortExpected = PRV_TGF_VXLAN_GBP_UDP_DST_PORT_CNS;

    /* AUTODOC: compare received UDP Dest Port vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(udpDstPortExpected, udpDstPortReceived,
                                "--- Failure in prvTgfVxlanGbpPacketVerification --- \n"
                                "UDP Dest Port is not as expected => Expected:0x%x != Received:0x%x \n",
                                 udpDstPortExpected, udpDstPortReceived);

    /*********************** Check VXLAN Header *********************************************/
    /* Increment offset by 8 bytes to point to VXLAN header low part  */
    pktOffset +=8;

    /* AUTODOC: get the first 2B of VXLAN header to verify VXLAN Flags */
    vxlanFlags = (GT_U32)((packetBuf[pktOffset] << 8) | (packetBuf[pktOffset + 1]));
    /* AUTODOC: compare received VXLAN Flags vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(0x8848, vxlanFlags,
                                "--- Failure in prvTgfVxlanGbpPacketVerification --- \n"
                                "VXLAN Flags is not as expected => Expected:0x%x != Received:0x%x \n",
                                 0x8848, vxlanFlags);

    /* AUTODOC: get the next 2B of VXLAN header to verify VXLAN Group Policy ID */
    vxlanGroupPolicyIdReceived = (GT_U32)((packetBuf[pktOffset + 2] << 8) | (packetBuf[pktOffset + 3]));
    /* IPv6 config use 2 bits[1:0] for Group Policy ID update from
     * desc<copy_reserved>
     * IPv4 config use 3 bits[2:0] for Group Policy ID update from
     * desc<copy_reserved>
     */
    vxlanGroupPolicyIdExpected = (protocol == 4) ? (prvTgfPacketL2Part.daMac[5] + 3) & 0x7 : (prvTgfPacketL2Part.daMac[5] + 3) & 0x3;

    /* AUTODOC: compare received VXLAN Group Policy ID vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(vxlanGroupPolicyIdExpected, vxlanGroupPolicyIdReceived,
                                "--- Failure in prvTgfVxlanGbpPacketVerification --- \n"
                                "VXLAN GroupPolicy ID is not as expected => Expected:0x%x != Received:0x%x \n",
                                 vxlanGroupPolicyIdExpected, vxlanGroupPolicyIdReceived);

    pktOffset +=4;

    /* AUTODOC: Get the next 3B of VXLAN header to verify VXLAN VNI */
    vxlanVni = (GT_U32)(packetBuf[pktOffset + 2] | (packetBuf[pktOffset + 1] << 8) |
                        (packetBuf[pktOffset] << 16));
    /* AUTODOC: compare received VXLAN VNI vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(0x18, vxlanVni,
                                "--- Failure in prvTgfVxlanGbpPacketVerification --- \n"
                                "VXLAN VNI is not as expected => Expected:0x%x != Received:0x%x \n",
                                 0x18, vxlanVni);

    /*********************** Check Origin MAC DA ************************************/
    /* Increment offset by 4B to point to MAC DA  */
    pktOffset +=4;

    /* AUTODOC: Get actual MAC DA & MAC SA */
    for (i=0;i<(TGF_L2_HEADER_SIZE_CNS/2);i++)
    {
        receivedMac.daMac[i] = packetBuf[pktOffset + i];
        receivedMac.saMac[i] = packetBuf[pktOffset + i + sizeof(prvTgfPacketL2Part.daMac)];

        /* Decrment received with expected to check if there is any mismatch */
        daMacErr += receivedMac.daMac[i] - prvTgfPacketL2Part.daMac[i] ;
        saMacErr += receivedMac.saMac[i] - prvTgfPacketL2Part.saMac[i] ;
    }

    /* AUTODOC: compare received MAC DA vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL0_STRING_MAC(0, daMacErr,
                                "--- Failure in prvTgfVxlanGbpPacketVerification --- \n"
                                "Mismatch in MAC DA address");
    if (daMacErr != 0)
    {
        /* Print received and expected MAC DA */
        PRV_UTF_LOG12_MAC("Expected MAC DA:%.2x%.2x%.2x%.2x%.2x%.2x \n"
                          "Received MAC DA:%.2x%.2x%.2x%.2x%.2x%.2x \n",
                          prvTgfPacketL2Part.daMac[0],prvTgfPacketL2Part.daMac[1],prvTgfPacketL2Part.daMac[2],
                          prvTgfPacketL2Part.daMac[3],prvTgfPacketL2Part.daMac[4],prvTgfPacketL2Part.daMac[5],
                          receivedMac.daMac[0],receivedMac.daMac[1],receivedMac.daMac[2],
                          receivedMac.daMac[3],receivedMac.daMac[4],receivedMac.daMac[5]);
    }
    /*********************** Check Origin MAC SA ************************************/
    /* AUTODOC: compare received MAC SA vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL0_STRING_MAC(0, saMacErr,
                                "--- Failure in prvTgfVxlanGbpPacketVerification --- \n"
                                "Mismatch in MAC SA address \n");
    if (saMacErr != 0)
    {
        /* Print received and expected MAC SA */
        PRV_UTF_LOG12_MAC("Expected MAC SA:%.2x%.2x%.2x%.2x%.2x%.2x \n"
                          "Received MAC SA:%.2x%.2x%.2x%.2x%.2x%.2x \n",
                          prvTgfPacketL2Part.saMac[0],prvTgfPacketL2Part.saMac[1],prvTgfPacketL2Part.saMac[2],
                          prvTgfPacketL2Part.saMac[3],prvTgfPacketL2Part.saMac[4],prvTgfPacketL2Part.saMac[5],
                          receivedMac.saMac[0],receivedMac.saMac[1],receivedMac.saMac[2],
                          receivedMac.saMac[3],receivedMac.saMac[4],receivedMac.saMac[5]);
    }

    /*********************** Check Passenger payload ************************************/
    /* Increment offset by 12B of MACs to point to passenger payload (EtherType)  */
    pktOffset += TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS;

    /* Anchor offset to passenger */
    payloadPktOffset = pktOffset;

    /* AUTODOC: Get actual passenger payload  */
    for (i=0; i< prvTgfVxPacketPayloadPart.dataLength; i+=4)
    {
        /* Update offset every 4 bytes */
        pktOffset = payloadPktOffset + i;

        /* Get actual payload */
        recPassengerPayload = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                                   (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));
        /* Get expected payload */
        expPassengerPayload = (GT_U32)(prvTgfVxPayloadDataArr[i+3] | (prvTgfVxPayloadDataArr[i+2] << 8) |
                                           (prvTgfVxPayloadDataArr[i+1] << 16) | (prvTgfVxPayloadDataArr[i] << 24));

        UTF_VERIFY_EQUAL2_STRING_MAC(expPassengerPayload, recPassengerPayload,
                                "--- Failure in prvTgfVxlanGbpPacketVerification --- \n"
                                "Mismatch in passenger payload data expected 0x%0x, received 0x%0x\n",
                                expPassengerPayload, recPassengerPayload);
        /* Decrement received with expected to check if there is any mismatch */
        passengerPayloadErr += (recPassengerPayload - expPassengerPayload) ;
    }
    /* AUTODOC: compare received passenger payload vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL0_STRING_MAC(0, passengerPayloadErr,
                                "--- Failure in prvTgfVxlanGbpPacketVerification --- \n"
                                "Mismatch in passenger payload data ");
}

/**
* @internal prvTgfVxlanGbpVerification function
* @endinternal
*
* @brief  VXLAN-GBP Packet Verification
*
* @param[in] protocol    - VXLAN tunnel is ipv4 or ipv6 based
*                           GT_TRUE - VXLAN tunnel is ipv6 based
*                           GT_FALSE - VXLAN tunnel is ipv4 based
* @param[in] udpSrcPortMode - VXLAN tunnel UDP source port mode
*                           GT_TRUE - source port mode is based on IP generic
*                           GT_FALSE - source port mode is based on packet hash
*
*/
GT_VOID prvTgfVxlanGbpVerification
(
    GT_BOOL protocol,
    GT_BOOL udpSrcPortMode
)
{
    GT_U8                               packetBuf[1][PRV_TGF_MAX_PACKET_SIZE_CNS] = {{0}};
    GT_U32                              packetLen[1] = {PRV_TGF_MAX_PACKET_SIZE_CNS};
    GT_U32                              packetActualLength = 0;
    GT_U8                               queue = 0;
    GT_U8                               dev = 0;
    TGF_NET_DSA_STC                     rxParam;
    GT_U8                               i = 0;
    GT_BOOL                             getFirst = GT_TRUE;
    GT_STATUS                           rc = GT_OK;

    portInterface1.type              = CPSS_INTERFACE_PORT_E;
    portInterface1.devPort.hwDevNum  = prvTgfDevNum;
    portInterface1.devPort.portNum   = prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS];

    /* AUTODOC: Get the captured packets */
    rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&portInterface1,
                                                TGF_PACKET_TYPE_CAPTURE_E,
                                                getFirst, GT_TRUE, packetBuf[i],
                                                &packetLen[i], &packetActualLength,
                                                &dev, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet");

    /* VXLAN-GBP Packet Verification */
    prvTgfVxlanGbpPacketVerification(packetBuf[0], protocol, udpSrcPortMode);
}
