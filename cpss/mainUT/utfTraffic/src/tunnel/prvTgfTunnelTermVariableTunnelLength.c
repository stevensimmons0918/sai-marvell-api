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
* @file prvTgfTunnelTermVariableTunnelLength.c
*
* @brief TTI Variable Tunnel Length Testcase
*
* @version   1
********************************************************************************
*/


#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <common/tgfTunnelGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfPacketGen.h>

#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpssDriver/pp/config/generic/cpssDrvPpGenDump.h>
#include <cpssCommon/private/prvCpssMath.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>
#include <trafficEngine/private/prvTgfTrafficParser.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           5

/* nextHop VLAN Id */
#define PRV_TGF_RECV_VLANID_CNS           6

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         0

/* port number to receive traffic to */
#define PRV_TGF_RECV_PORT_IDX_CNS         1

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = 1;

/* destination ePort */
#define PRV_TGF_DESTINATION_EPORT_CNS   (UTF_CPSS_PP_MAX_PORT_NUM_CNS(prvTgfDevNum)  - 5)

/* Tunnel header length profile table index */
static GT_U32 prvTgfTtiProfileIdx = 3;

/* the TTI Rule index */
#define prvTgfTtiRuleIndex  ((UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum)) ? \
(prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(PRV_TTI_LOOKUP_0, 3)) : 1)

/******************************* Test packet **********************************/

/* L2 part of unicast packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x44, 0x33},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x22, 0x11}                /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* Unicast packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x4f,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0xff,                /* ipv4 protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */ /*0x5EA0*/
    { 2,  2,  2,  2},   /* srcAddr */
    { 4,  4,  4,  4}    /* dstAddr */
};

/* packet's UDP part */
static TGF_PACKET_UDP_STC prvTgfPacketUdpPart =
{
    0xab,                                          /* src port */
    0xcd,                                          /* dst port */
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,               /* length */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS              /* csum */
};

/* L2 part of unicast packet */
static TGF_PACKET_L2_STC prvTgfPacketPassengerL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x88, 0x77},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x66, 0x55}                /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketPassengerVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_RECV_VLANID_CNS                       /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketPassengerEtherTypePart = {0x0640};

/* Geneve header
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |Ver|  Opt Len  |O|C|    Rsvd.  |          Protocol Type        |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |        Virtual Network Identifier (VNI)       |    Reserved   |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *  Ver = 0, Opt Len = 2, Protocol Type = 0x0800, VNI = 0x7d
 */
static GT_U8 prvTgfGeneveHeader[] = {
    0x08, 0x00, 0x08, 0x00,
    0x7d, 0x00, 0x00, 0x00,
};

/* Geneve Option 1
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |          Option Class         |C|    Type     |R|R|R| Length  |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                      Variable Option Data                     |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *  Option Class = 0x11ab, Type = 0x11, Length = 1, data = 0xaa55aa55
 */
static GT_U8 prvTgfGeneveTlv1[] = {
    0x11, 0xab, 0x11, 0x08,
    0x5a, 0x5b, 0x5c, 0x5d
};


/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f
};

/* Geneve Header part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketGeneveHeaderPart = {
        sizeof(prvTgfGeneveHeader),
        prvTgfGeneveHeader
};

/* Geneve Option1 part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketGeneveTlv1Part = {
        sizeof(prvTgfGeneveTlv1),
        prvTgfGeneveTlv1
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of unicast packet with 1 TLV */
static TGF_PACKET_PART_STC prvTgfPacketPartArray1Tlv[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_UDP_E,       &prvTgfPacketUdpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketGeneveHeaderPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketGeneveTlv1Part},
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketPassengerEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Geneve header length with 1 TLV */
#define PRV_TGF_GENEVE_TOTAL_HEADER_1TLV_LEN_CNS \
    sizeof(prvTgfGeneveHeader) + sizeof(prvTgfGeneveTlv1)

/* LENGTH of Geneve packet with 1 TLV*/
#define PRV_TGF_GENEVE_PACKET_1TLV_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_IPV4_HEADER_SIZE_CNS + TGF_UDP_HEADER_SIZE_CNS + PRV_TGF_GENEVE_TOTAL_HEADER_1TLV_LEN_CNS \
    + TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + sizeof(prvTgfPayloadDataArr)

/* normal network PACKET with 1 TLV to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_GENEVE_PACKET_1TLV_LEN_CNS,                              /* totalLen */
    sizeof(prvTgfPacketPartArray1Tlv) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray1Tlv                                        /* partsArray */
};

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_TTI_KEY_SIZE_ENT         keySizeOrig;
    CPSS_DXCH_TTI_TUNNEL_HEADER_LENGTH_PROFILE_STC tunnelHeaderLengthProfile;
} prvTgfRestoreCfg;

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
 * @internal prvTgfTunnelTermVariableTunnelLengthPortConfigSet function
 * @endinternal
 *
 * @brief    Set basic port vlan configuration
 *
 */
static GT_VOID prvTgfTunnelTermVariableTunnelLengthPortConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U8       tagArray[] = {1, 1, 0, 0};

    /* AUTODOC: create VLAN 5 with untagged ports 0,1 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS, prvTgfPortsArray, NULL, tagArray, 2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet failed");

    /* AUTODOC: create VLAN 5 with untagged ports 0,1 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_RECV_VLANID_CNS, prvTgfPortsArray, NULL, tagArray, 2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet failed");

}

/**
 * @internal prvTgfTunnelTermVariableTunnelLengthConfigSet function
 * @endinternal
 *
 * @brief    Set variable tunnel length configuration
 *
 */
static GT_VOID prvTgfTunnelTermVariableTunnelLengthConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32      udbLsb = 30;
    GT_U32      udbMsb = 31;
    GT_U8       lenOffset = 8;
    CPSS_DXCH_TTI_TUNNEL_HEADER_LENGTH_PROFILE_STC tunnelHeaderLengthProfile;

    /* save original profile */
    rc = cpssDxChTtiTunnelHeaderLengthProfileGet(prvTgfDevNum, prvTgfTtiProfileIdx, &prvTgfRestoreCfg.tunnelHeaderLengthProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTtiTunnelHeaderLengthProfileGet failed");

    /* populate and set tunnel header length profile table entry*/
    tunnelHeaderLengthProfile.lengthFieldAnchorType = GT_TRUE;
    tunnelHeaderLengthProfile.lengthFieldUdbLsb = udbLsb&0xF;
    tunnelHeaderLengthProfile.lengthFieldUdbMsb = udbMsb&0xF;
    tunnelHeaderLengthProfile.lengthFieldLsBit = 2;
    tunnelHeaderLengthProfile.lengthFieldSize = 6;
    tunnelHeaderLengthProfile.constant = 16;
    tunnelHeaderLengthProfile.multiplier = 2;

    rc = cpssDxChTtiTunnelHeaderLengthProfileSet(prvTgfDevNum, prvTgfTtiProfileIdx, &tunnelHeaderLengthProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTtiTunnelHeaderLengthProfileSet failed");

    /* set UDB's 30 and 31 for variable length field in the packet */
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV4_OTHER_E, udbLsb,
            PRV_TGF_TTI_OFFSET_L4_E, lenOffset);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet failed");

    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV4_OTHER_E, udbMsb,
            PRV_TGF_TTI_OFFSET_L4_E, lenOffset + 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet failed");

}

/**
 * @internal prvTgfTunnelTermVariableTunnelLengthTtiConfigSet function
 * @endinternal
 *
 * @brief    Set TTI configuration for tunnel terminating Ipv4 packet.
 *
 */
static GT_VOID prvTgfTunnelTermVariableTunnelLengthTtiConfigSet
(
    GT_VOID
)
{
    GT_STATUS                rc;
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;
    PRV_TGF_TTI_ACTION_2_STC ttiAction;

    /* initialize structure to 0 */
    cpssOsMemSet(&ttiAction,     0, sizeof(PRV_TGF_TTI_ACTION_2_STC));
    cpssOsMemSet(&ttiPattern,    0, sizeof(PRV_TGF_TTI_RULE_UNT));
    cpssOsMemSet(&ttiMask,       0, sizeof(PRV_TGF_TTI_RULE_UNT));
    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);

    /* AUTODOC: enable the TTI lookup for IPv4 at the port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_TTI_KEY_UDB_IPV4_OTHER_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* set key size PRV_TGF_TTI_KEY_SIZE_30_B_E for the key keyType */
    rc = prvTgfTtiPacketTypeKeySizeGet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV4_OTHER_E, &prvTgfRestoreCfg.keySizeOrig);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeGet: %d", prvTgfDevNum);

    /* set key size PRV_TGF_TTI_KEY_SIZE_30_B_E for the key keyType */
    rc = prvTgfTtiPacketTypeKeySizeSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV4_OTHER_E, PRV_TGF_TTI_KEY_SIZE_30_B_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);

    /* AUTODOC: enable IPv4 TTI lookup for only tunneled packets received on port 0 */
    rc = prvTgfTtiPortIpv4OnlyTunneledEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortIpv4OnlyTunneledEnableSet");

    /* AUTODOC: set the TTI Rule Pattern, Mask and Action */
    /* AUTODOC: tti rule key is PRV_TGF_TTI_KEY_IPV4_E */

    ttiPattern.udbArray.udb[0] = 0x44;
    ttiMask.udbArray.udb[0]    = 0xff;

    ttiPattern.udbArray.udb[1] = 0x33;
    ttiMask.udbArray.udb[1]    = 0xff;

    ttiPattern.udbArray.udb[2] = 0x22;
    ttiMask.udbArray.udb[2]    = 0xff;

    ttiPattern.udbArray.udb[3] = 0x11;
    ttiMask.udbArray.udb[3]    = 0xff;

    /* configure user defined bytes */
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV4_OTHER_E, 0,
            PRV_TGF_TTI_OFFSET_L2_E, 4);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet failed");

    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV4_OTHER_E, 1,
            PRV_TGF_TTI_OFFSET_L2_E, 5);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet failed");

    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV4_OTHER_E, 2,
            PRV_TGF_TTI_OFFSET_L2_E, 10);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet failed");

    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV4_OTHER_E, 3,
            PRV_TGF_TTI_OFFSET_L2_E, 11);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet failed");

    ttiAction.command                           = CPSS_PACKET_CMD_FORWARD_E;
    ttiAction.redirectCommand                   = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ttiAction.userDefinedCpuCode                = CPSS_NET_FIRST_USER_DEFINED_E;
    ttiAction.tunnelTerminate                   = GT_TRUE;
    ttiAction.ttPassengerPacketType             = PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E;
    ttiAction.tsPassengerPacketType             = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;
    ttiAction.egressInterface.type              = CPSS_INTERFACE_PORT_E;
    ttiAction.egressInterface.devPort.hwDevNum  = prvTgfDevNum;
    ttiAction.egressInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS];
    ttiAction.tag0VlanPrecedence                = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    ttiAction.tunnelHeaderLengthAnchorType      = CPSS_DXCH_TUNNEL_HEADER_LENGTH_PROFILE_BASED_ANCHOR_TYPE_E;
    ttiAction.ttHeaderLength                    = prvTgfTtiProfileIdx;

    /* write tcam rule to tcan table */
    rc = prvTgfTtiRule2Set(prvTgfTtiRuleIndex, PRV_TGF_TTI_KEY_UDB_IPV4_OTHER_E,
                              &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");

}

/**
* @internal prvTgfTunnelTermVariableTunnelLengthConfigure function
* @endinternal
*
* @brief  Set configuration for Geneve Packet
*         1. Set Port Configuration
*         2. Configure Tunnel header length profile
*         3. Configure TTI
*/
GT_VOID prvTgfTunnelTermVariableTunnelLengthConfigure
(
        GT_VOID
)
{
    /* configure port vlan */
    prvTgfTunnelTermVariableTunnelLengthPortConfigSet();

    /* set variable tunnel length configuration*/
    prvTgfTunnelTermVariableTunnelLengthConfigSet();

    /* set tti configuration */
    prvTgfTunnelTermVariableTunnelLengthTtiConfigSet();

}

/**
* @internal prvTgfTunnelTermVariableTunnelLengthGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet and capture:
*         Check captured packet.
*/
GT_VOID prvTgfTunnelTermVariableTunnelLengthGenerate
(
        GT_VOID
)
{
    GT_STATUS rc;
    GT_U8                   dev = prvTgfDevNum;
    GT_PORT_NUM             sendPort = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    GT_PORT_NUM             recvPort = prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS];
    CPSS_INTERFACE_INFO_STC portInt;
    GT_U8                   packetBuf[256] = {0};
    GT_U32                  packetBufLen = sizeof(packetBuf);
    GT_U32                  packetActualLength = 0;
    GT_U8                   queueCpu = 0;
    TGF_NET_DSA_STC         rxParams;

    /* AUTODOC: Reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* enable capture on a MC subscriber port */
    portInt.type            = CPSS_INTERFACE_PORT_E;
    portInt.devPort.hwDevNum  = prvTgfDevNum;
    portInt.devPort.portNum = recvPort;

    /* AUTODOC: start capture on egress port*/
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInt, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet failed");

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Setup normal network packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* AUTODOC: Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, sendPort);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n", prvTgfDevNum, sendPort);

    /* AUTODOC: capture packet by trapping to CPU*/
    rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&portInt,
            TGF_PACKET_TYPE_CAPTURE_E,
            GT_TRUE, GT_TRUE, packetBuf,
            &packetBufLen, &packetActualLength,
            &dev, &queueCpu, &rxParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet failed\n");

    /* check modified Vlan */
    UTF_VERIFY_EQUAL0_STRING_MAC(0x88, packetBuf[4],  "DA 2nd Byte");
    UTF_VERIFY_EQUAL0_STRING_MAC(0x77, packetBuf[5],  "DA 1st Byte");
    UTF_VERIFY_EQUAL0_STRING_MAC(0x66, packetBuf[10], "SA 2nd Byte");
    UTF_VERIFY_EQUAL0_STRING_MAC(0x55, packetBuf[11], "SA 1st Byte");

    /* AUTODOC: stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* AUTODOC: restore port capture setting */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInt, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
            prvTgfDevNum, portInt.devPort.portNum);

    /* Flush fdb */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfBrgFdbFlush");

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");
}

/**
* @internal prvTgfTunnelTermVariableTunnelLengthRestore function
* @endinternal
*
* @brief  Restore Configuration
*/
GT_VOID prvTgfTunnelTermVariableTunnelLengthRestore
(
        GT_VOID
)
{
    GT_STATUS                                  rc;

    /* -------------------------------------------------------------------------
     * Restore Port Configuration
     */

    /* Invalidate VLAN */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_RECV_VLANID_CNS);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, PRV_TGF_RECV_VLANID_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, PRV_TGF_SEND_VLANID_CNS);

    /* -------------------------------------------------------------------------
     * Restore TTI Configuration
     */

    /* AUTODOC: invalidate TTI rule 1 */
    rc = prvTgfTtiRuleValidStatusSet(prvTgfTtiRuleIndex, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d", prvTgfDevNum);

    /* AUTODOC: disable the TTI lookup for IPv6 at port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], PRV_TGF_TTI_KEY_UDB_IPV4_OTHER_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: restore key size for PRV_TGF_TTI_KEY_UDB_IPV4_OTHER_E */
    rc = prvTgfTtiPacketTypeKeySizeSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV4_OTHER_E , prvTgfRestoreCfg.keySizeOrig);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);

    /* AUTODOC: disable the IPv4 TTI lookup for only tunneled packets received on port 0 */
    rc = prvTgfTtiPortIpv4OnlyTunneledEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortIpv4OnlyTunneledEnableSet: %d", prvTgfDevNum);

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);

    /* -------------------------------------------------------------------------
     * Restore variable tunnel length Configuration
     */
    rc = cpssDxChTtiTunnelHeaderLengthProfileSet(prvTgfDevNum, prvTgfTtiProfileIdx, &prvTgfRestoreCfg.tunnelHeaderLengthProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTtiTunnelHeaderLengthProfileSet failed");
}

