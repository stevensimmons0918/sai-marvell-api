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
* @file prvTgfSffNshVxlanGpeToEthernetUseCase.c
*
* @brief SFF NSH VXLAN-GPE to Ethernet use cases testing.
*
*        Ports allocation:
*           ^ Port#0: Ingress port
*           ^ Port#1: Target port
*
*        Packet flow:
*           1.  Encapsulated NSH over VXLAN-GPE packet enters the device via Ingress Port#0
*           2.  Outer tunnel of L2,vlan,EtherType,IPv6,UDP and VXLAN-GPE is removed by TTI
*               up to (not included) NSH header
*           3.  After TT IPCL forward the packet based on its NSH content
*               If NSH<TTL> field = 1 than packet is trapped to CPU
*               If not redirects to ePort#1 pending NSH<Service Path Identifier> and
*               NSH<Service Index>
*           4.  packet is encapsulated with L2,EtherType,VLAN tag tunnel by HA unit
*           5.  PHA thread is triggered in which fw decrement NSH<TTL> field by 1
*           3.  The new encapsulated packet exits the device from target port#1.
*
*        Test description:
*        Send 2 packets from ingress port#0 to target port#1:
*           - First packet with NSH TTL value different than 1.
*             Expects vxlan-gpe tunnel to be replaced with Ethernet tunnel
*             and TTL value should be decremented by 1 by PHA
*           - Second packet with NSH TTL value equal to 1.
*             Expects packet to be trapped to CPU.
*             In this case target port counters should all be zero.
*
* @version   1
********************************************************************************
*/
#include <tunnel/prvTgfSffNshVxlanGpeToEthernetUseCase.h>
#include <oam/prvTgfOamGeneral.h>
#include <cpss/dxCh/dxChxGen/pha/cpssDxChPha.h>
#include <cpss/dxCh/dxChxGen/oam/cpssDxChOam.h>




/*************************** Constants definitions ***********************************/

/* PHA fw thread ID of SFF NSH VXLAN-GPE to Ethernet */
#define PRV_TGF_PHA_THREAD_ID_SFF_NSH_VXLAN_GPE_TO_ETHERNET_CNS  22

/* Size of output packets in bytes include CRC */
#define PRV_TGF_PACKET_SIZE_SFF_NSH_VXLAN_GPE_TO_ETHERNET_CNS   110
#define PRV_TGF_MAX_PACKET_SIZE_CNS    PRV_TGF_PACKET_SIZE_SFF_NSH_VXLAN_GPE_TO_ETHERNET_CNS

/* Number of packets */
#define PRV_TGF_BURST_COUNT_CNS             2

/* Tunnel start entry index */
#define PRV_TGF_TUNNEL_START_INDEX_CNS      8

/* Ports number allocation */
#define PRV_TGF_INGRESS_PORT_IDX_CNS        0
#define PRV_TGF_TARGET_PORT_IDX_CNS         1

/* Default VLAN Id */
#define PRV_TGF_DEFAULT_VLANID_CNS          1
#define PRV_TGF_SEND_VLANID_CNS             2

/* Offset to UDP part in the output packet */
#define PRV_TGF_TUNNEL_ETHERTYPE_PKT_OFFSET_CNS    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS)


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

/* packet's IPv6 ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketInputIpv6EtherTypePart = {
    TGF_ETHERTYPE_86DD_IPV6_TAG_CNS
};

/* packet's IPv6 header */
static TGF_PACKET_IPV6_STC prvTgfPacketInputIpv6Part = {
    6,                                  /* version */
    (0x15 << 2),                        /* DSCP */
    0,                                  /* flowLabel */
    0x68,                               /* payloadLen */
    0x11,                               /* nextHeader (UDP) */
    0x40,                               /* hopLimit */
    {0x00FF, 0, 0, 0, 0, 0, 0, 0x3333}, /* srcAddr */
    {1, 2, 3, 4, 5, 6, 7, 8}            /* dstAddr */
};

/* packet's UDP header */
static TGF_PACKET_UDP_STC prvTgfPacketInputUdpPart = {
    0x1010,
    0x12b6, /* UDP Destination = 4790 (VXLAN-GPE) */
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS
};

/* packet's VXLAN-GPE header */
static TGF_PACKET_VXLAN_GPE_STC prvTgfPacketInputVxlanGpePart = {
    0x12,               /* Flags */
    0x3456,             /* Reserved */
    0x78,               /* Next Protocol */
    0x9abcde,           /* VXLAN Network Identifier (VNI) - 24Bits */
    0xff                /* Reserved */
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


/* PARTS of input packet: |L2|EtherType|IPv6|UDP|VXLAN-GPE|NSH Base hdr|
                          |NSH Service Path hdr|NSH metadata|payload passenger| */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,           &prvTgfPacketInputL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,     &prvTgfPacketInputVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E,    &prvTgfPacketInputIpv6EtherTypePart},
    {TGF_PACKET_PART_IPV6_E,         &prvTgfPacketInputIpv6Part},
    {TGF_PACKET_PART_UDP_E,          &prvTgfPacketInputUdpPart},
    {TGF_PACKET_PART_VXLAN_GPE_E,    &prvTgfPacketInputVxlanGpePart},
    {TGF_PACKET_PART_NSH_E,          &prvTgfPacketInputNshPart},
    {TGF_PACKET_PART_L2_E,           &prvTgfOriginalPacketL2Part},
    {TGF_PACKET_PART_PAYLOAD_E,      &prvTgfPacketPayloadPart}
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
} prvTgfRestoreCfg;



/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/


/**
* @internal prvTgfSffNshVxlanGpeToEthernetTtiConfigSet function
* @endinternal
*
* @brief   TTI test configurations
*          Tunnel terminate the packet - strip the header up to
*          (not including) the NSH header
*/
static GT_VOID prvTgfSffNshVxlanGpeToEthernetTtiConfigSet
(
    GT_VOID
)
{
    GT_STATUS                 rc = GT_OK;
    GT_U32                    i,udbIndx;
    GT_U8                     pclId;
    PRV_TGF_TTI_ACTION_STC    ttiAction;
    PRV_TGF_TTI_RULE_UNT      ttiPattern;
    PRV_TGF_TTI_RULE_UNT      ttiMask;


    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: Since same TCAM is used by both TTI and PCL need to set whether
                the TTI indexes are relative to the start of the TTI section */
    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiTcamIndexIsRelativeSet");

    /* AUTODOC: enable the TTI lookup for IPv6 type at port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E,
                                      GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: Save pclId for restoration */
    rc = prvTgfTtiPclIdGet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E,&prvTgfRestoreCfg.pclId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdGet");

    /* AUTODOC: set pclId for TTI key type of IPv6  */
    pclId = 0x11;
    rc = prvTgfTtiPclIdSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E,pclId);
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

    /* udp destination port */
    ttiPattern.udbArray.udb[2] = (GT_U8)((prvTgfPacketInputUdpPart.dstPort >> 8) & 0xff);
    ttiMask.udbArray.udb[2] = 0xff;
    ttiPattern.udbArray.udb[3] = (GT_U8)(prvTgfPacketInputUdpPart.dstPort & 0xff);
    ttiMask.udbArray.udb[3] = 0xff;

    /* IPv6 IP address (takes 16 UDB entries 1 byte each) */
    udbIndx = 4;
    for (i=0; i<8; i++)
    {
        ttiPattern.udbArray.udb[udbIndx] = (GT_U8)(prvTgfPacketInputIpv6Part.dstAddr[i] >> 8);
        ttiMask.udbArray.udb[udbIndx] = 0xff;
        udbIndx +=1;
        ttiPattern.udbArray.udb[udbIndx] = (GT_U8) prvTgfPacketInputIpv6Part.dstAddr[i];
        ttiMask.udbArray.udb[udbIndx] = 0xff;
        udbIndx +=1;
    }


    /********************** Setting UDB Configuration  *******************************/

    /* save TTI key size for restore*/
    rc = prvTgfTtiPacketTypeKeySizeGet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E,&prvTgfRestoreCfg.keySize);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeGet: %d", prvTgfDevNum);

    /* Set TTI key size (20B) */
    rc = prvTgfTtiPacketTypeKeySizeSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E , PRV_TGF_TTI_KEY_SIZE_20_B_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);

    /*save udb for restore*/
    for(i=0; i < 20; i++)
    {
        rc = prvTgfTtiUserDefinedByteGet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E,i,&prvTgfRestoreCfg.udbOffsetTypeArray[i],&prvTgfRestoreCfg.udbOfsetArray[i]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteGet: %d", prvTgfDevNum);
    }

    /* For PCL ID */
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E,0,PRV_TGF_TTI_OFFSET_METADATA_E,22);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    /* For local source ePort */
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E,1,PRV_TGF_TTI_OFFSET_METADATA_E,26);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    /* For UDP Destination */
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E,2,PRV_TGF_TTI_OFFSET_L4_E,2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E,3,PRV_TGF_TTI_OFFSET_L4_E,3);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    /* For IPv6 IP destination address */
    for (i=0;i<16;i++)
    {
        rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,
                                         PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E,
                                         (4+i),   /* UDB index */
                                         PRV_TGF_TTI_OFFSET_L3_MINUS_2_E,
                                         (26+i)); /* IPv6 byte offset */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    }


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
    rc = prvTgfTtiRuleSet(prvTgfTtiRuleIndex,PRV_TGF_TTI_RULE_UDB_20_E, &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet");
}


/**
* @internal prvTgfSffNshVxlanGpeToEthernetIpclConfigSet function
* @endinternal
*
* @brief   IPCL test configurations
*          Forward the packet based on its NSH
*          Two rules:
*          - Rule that checks if TTL=1, and traps to the CPU.
*          - Rule that uses <Service Path Identifier>, <Service Index>, and redirects to ePort#1
*/
static GT_VOID prvTgfSffNshVxlanGpeToEthernetIpclConfigSet
(
    GT_VOID
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT         mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT         pattern;
    PRV_TGF_PCL_ACTION_STC              action;
    GT_U32                              ruleIndex,i;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT    ruleFormat;


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
                                    15+i,      /* UDP index */
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
* @internal prvTgfSffNshVxlanGpeToEthernetBridgeConfigSet function
* @endinternal
*
* @brief   Bridge test configurations
*/
static GT_VOID prvTgfSffNshVxlanGpeToEthernetBridgeConfigSet
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
* @internal prvTgfSffNshVxlanGpeToEthernetHaConfigSet function
* @endinternal
*
* @brief   HA test configurations
*/
static GT_VOID prvTgfSffNshVxlanGpeToEthernetHaConfigSet
(
    GT_VOID
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC    egressInfo;


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
    /* AUTODOC: genericType = short, tagEnable = TRUE, vlanId=1 */
    /* AUTODOC: ethType = 0x894F(NSH), MACDA = 00:00:00:00:44:04 */
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));
    tunnelEntry.genCfg.tagEnable   = GT_TRUE;
    tunnelEntry.genCfg.vlanId      = PRV_TGF_DEFAULT_VLANID_CNS;
    tunnelEntry.genCfg.genericType = PRV_TGF_TUNNEL_START_GENERIC_SHORT_TYPE_E;
    tunnelEntry.genCfg.etherType   = TGF_ETHERTYPE_894F_NSH_TAG_CNS; /* 0x894F */
    cpssOsMemCpy(tunnelEntry.genCfg.macDa.arEther, prvTgfPacketTunnelL2Part.daMac, sizeof(TGF_MAC_ADDR));
    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_INDEX_CNS, CPSS_TUNNEL_GENERIC_E, &tunnelEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartEntrySet");


    /**** PHA fw threads configurations ****/
    /* AUTODOC: Assign Classifier NSH over Ethernet fw thread to target port */
    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                        GT_TRUE, PRV_TGF_PHA_THREAD_ID_SFF_NSH_VXLAN_GPE_TO_ETHERNET_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaPortThreadIdSet");

    /* AUTODOC: Set the thread entry - PRV_TGF_PHA_THREAD_ID_SFF_NSH_VXLAN_GPE_TO_ETHERNET_CNS */
    cpssOsMemSet(&commonInfo, 0, sizeof(commonInfo));
    extInfo.notNeeded = 0;
    commonInfo.statisticalProcessingFactor = 0;
    commonInfo.busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;
    commonInfo.stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;
    rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
                                     PRV_TGF_PHA_THREAD_ID_SFF_NSH_VXLAN_GPE_TO_ETHERNET_CNS,
                                     &commonInfo,
                                     CPSS_DXCH_PHA_THREAD_TYPE_SFF_NSH_VXLAN_GPE_TO_ETHERNET_E,
                                     &extInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaThreadIdEntrySet");
}


/**
* internal prvTgfSffNshVxlanGpeToEthernetConfigSet function
* @endinternal
*
* @brief   SFF NSH VXLAN-GPE to Ethernet use case configurations
*          Includes the follwoing configurations:
*          - TTI
*          - IPCL
*          - Bridge
*          - HA
*/
GT_VOID prvTgfSffNshVxlanGpeToEthernetConfigSet
(
    GT_VOID
)
{
    /* TTI Configuration */
    prvTgfSffNshVxlanGpeToEthernetTtiConfigSet();

    /* IPCL Configuration */
    prvTgfSffNshVxlanGpeToEthernetIpclConfigSet();

    /* Bridge Configuration */
    prvTgfSffNshVxlanGpeToEthernetBridgeConfigSet();

    /* HA Configuration */
    prvTgfSffNshVxlanGpeToEthernetHaConfigSet();
}



/**
* @internal prvTgfSffNshVxlanGpeToEthernetConfigRestore function
* @endinternal
*
* @brief   SFF NSH VXLAN-GPE to Ethernet configurations restore.
* @note    1. Restore TTI Configuration
*          2. Restore IPCL Configuration
*          3. Restore Bridge Configuration
*          4. Restore HA Configuration
*          5. Restore base Configuration
*/
GT_VOID prvTgfSffNshVxlanGpeToEthernetConfigRestore
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

    /* AUTODOC: disable the TTI lookup for IPv6 at port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                       PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: restore key size for PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E */
    rc = prvTgfTtiPacketTypeKeySizeSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E, prvTgfRestoreCfg.keySize);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);

    /* AUTODOC: restore pclId */
    rc = prvTgfTtiPclIdSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E, prvTgfRestoreCfg.pclId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdSet: %d", prvTgfDevNum);

    /* AUTODOC: restore udb */
    for(i=0; i<20; i++)
    {
        rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum,PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E,i,prvTgfRestoreCfg.udbOffsetTypeArray[i],prvTgfRestoreCfg.udbOfsetArray[i]);
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
* internal tgfSffNshVxlanGpeToEthernetPacketVerification function
* @endinternal
*
* @brief  SFF NSH VXLAN-GPE to Ethernet use case verification Do the following:
*
*         - Check Tunnel Ethertype = NSH type
*         - Check NSH Base header TTL field was decrment by one
*         - Check NSH Base header rest of fields were not overwritten
*         - Check NSH Service Path header was not overwritten
*         - Check NSH metadata was not overwritten
*         - Check MAC addresses of passenger are not damaged
*         - Check payload data of passenger is not damaged
*/
GT_VOID tgfSffNshVxlanGpeToEthernetPacketVerification
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

    /* Should be NSH Type (0x894F) */
    nshValExpectedVal = TGF_ETHERTYPE_894F_NSH_TAG_CNS;

    /* Check expected vs actual and notify if mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(nshValExpectedVal, nshValActualVal,
                                "--- Failure in tgfClassifierNshOverEthernetVerification --- \n"
                                "Tunnel Ethertype is not as expected => Expected:0x%x != Received:0x%x \n",
                                 nshValExpectedVal, nshValActualVal);

    PRV_UTF_LOG0_MAC("======= Check NSH Base header  =======\n");

    /********************* Check NSH Base hdr (TTL field) ************************/
    /* Read TTL field from packet(extract 6 bits) */
    nshValActualVal = (GT_U32)(packetBuf[pktOffset+3] | (packetBuf[pktOffset+2] << 8));
    nshValActualVal = (nshValActualVal>>6) & 0x3F;

    /* Get expected TTL value from origin packet.
       Decrement by 1 since we expect PHA to do that */
    nshValExpectedVal = nshTtlVal;
    nshValExpectedVal -=1;
    nshValExpectedVal &=0x3F;

    /* Check expected vs actual and notify if mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(nshValExpectedVal, nshValActualVal,
                                "--- Failure in tgfClassifierNshOverEthernetVerification --- \n"
                                "NSH Base hdr (TTL) field is not as expected => Expected:0x%x != Received:0x%x \n",
                                 nshValExpectedVal, nshValActualVal);


    /********************* Check NSH Base hdr (|Ver|O|U|) ************************/
    /* Get |Ver|O|U| (4b) fields from packet */
    nshValActualVal = (GT_U32)(packetBuf[pktOffset+3] | (packetBuf[pktOffset+2] << 8));
    nshValActualVal = (nshValActualVal >> 12) & 0xF ;

    /* Get expected |Ver|O|U| (4b) value from origin packet */
    nshValExpectedVal = (prvTgfPacketInputNshPart.baseHdr >> 28) & 0x3F;

    /* Check expected vs actual and notify if mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(nshValExpectedVal, nshValActualVal,
                                "--- Failure in tgfClassifierNshOverEthernetVerification --- \n"
                                "NSH Base hdr (|Ver|O|U|) fields are not as expected => Expected:0x%x != Received:0x%x \n",
                                 nshValExpectedVal, nshValActualVal);


    /********************* Check NSH Base hdr (Length) ************************/
    /* Get Length (6b) field from packet */
    nshValActualVal = (GT_U32)(packetBuf[pktOffset+3] | (packetBuf[pktOffset+2] << 8));
    nshValActualVal &= 0x3F;

    /* Get expected Length (6b) value from origin packet */
    nshValExpectedVal = (prvTgfPacketInputNshPart.baseHdr >> 16) & 0x3F;

    /* Check expected vs actual and notify if mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(nshValExpectedVal, nshValActualVal,
                                "--- Failure in tgfClassifierNshOverEthernetVerification --- \n"
                                "NSH Base hdr (Length) field is not as expected => Expected:0x%x != Received:0x%x \n",
                                 nshValExpectedVal, nshValActualVal);


    /********************* Check NSH Base hdr (|Res|MD Type|Next protocol|) ************************/
    /* AUTODOC: increment offset by 4 bytes */
    pktOffset +=4 ;

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
* internal tgfSffNshVxlanGpeToEthernetPortCountersVerification function
* @endinternal
*
* @brief  Compare expected vs actual target port counters
*         Target port counters were already read upon packets transmission
*/
GT_VOID tgfSffNshVxlanGpeToEthernetPortCountersVerification
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
                                 "--- Failure in tgfSffNshVxlanGpeToEthernetPortCountersVerification --- \n"
                                 "Packets number is not as expected => Expected:0x%x != Received:0x%x \n",
                                  expPacketsNum, actualPacketsNum);

    /* Check expected vs actual and notify if mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(expBytesNum, actualBytesNum,
                                 "--- Failure in tgfSffNshVxlanGpeToEthernetPortCountersVerification --- \n"
                                 "Bytes number is not as expected => Expected:0x%x != Received:0x%x \n",
                                  expBytesNum, actualBytesNum);
}


/**
* internal prvTgfSffNshVxlanGpeToEthernetVerification function
* @endinternal
*
* @brief  Loop over packets and pending packet type check packet
*         content and target port counters
*/
GT_VOID prvTgfSffNshVxlanGpeToEthernetVerification
(
    GT_VOID
)
{
    GT_STATUS          rc = GT_OK;
    GT_U8              packetBuf[PRV_TGF_BURST_COUNT_CNS][PRV_TGF_MAX_PACKET_SIZE_CNS]= {{0}, {0}};
    GT_U32             packetLen[PRV_TGF_BURST_COUNT_CNS] = {PRV_TGF_PACKET_SIZE_SFF_NSH_VXLAN_GPE_TO_ETHERNET_CNS,
                                                             PRV_TGF_PACKET_SIZE_SFF_NSH_VXLAN_GPE_TO_ETHERNET_CNS};

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
            tgfSffNshVxlanGpeToEthernetPortCountersVerification(expPacketsNum,
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
            tgfSffNshVxlanGpeToEthernetPacketVerification(packetBuf[pktNum]);

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
            tgfSffNshVxlanGpeToEthernetPortCountersVerification(expPacketsNum,
                                                                actualPacketsNum[pktNum],
                                                                expBytesNum,
                                                                actualBytesNum[pktNum]);
        }
    }
}


/**
* @internal prvTgfSffNshVxlanGpeToEthernetTest function
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
GT_VOID prvTgfSffNshVxlanGpeToEthernetTest
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




