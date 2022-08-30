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
* @file prvTgfPpuGeneveLoopback.c
*
* @brief Programmable Parser Unit (PPU). Geneve Usecase with loopback.
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

#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/ppu/private/prvCpssDxChPpu.h>
#include <cpss/dxCh/dxChxGen/ppu/cpssDxChPpu.h>
#include <cpss/dxCh/dxChxGen/ppu/cpssDxChPpuTypes.h>
#include <cpssDriver/pp/config/generic/cpssDrvPpGenDump.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <ppu/prvTgfPpuGeneve.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>
#include <trafficEngine/private/prvTgfTrafficParser.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           5

/* nextHop VLAN Id */
#define PRV_TGF_RECV_VLANID_CNS       6

/* nextHop expected VLAN Id */
#define PRV_TGF_EXP_VLANID_CNS       0x3F

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         0

/* port number to receive traffic to */
#define PRV_TGF_RECV_PORT_IDX_CNS          3

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = 1;

/* the TTI Rule index */
#define prvTgfTtiRuleIndex  ((UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum)) ? \
(prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(PRV_TTI_LOOKUP_0, 3)) : 1)

/* default ppu profile index */
static GT_U32 prvTgfPpuProfileIdx  = 2;
static GT_U32 prvTgfPpuProfileIdx2 = 3;

/* default ppu State */
static GT_U32 prvTgfPpuState = 0x41;

/* Tunnel header length profile table index */
static GT_U32 prvTgfTtiProfileIdx = 3;

/* SP Bus profile index */
static GT_U32 prvTgfSpBusProfileIdx = 1;

/* loopback error profile index */
static GT_U32 prvTgfErrProfIdx = 2;

/* DAU profile index */
static GT_U32 prvTgfDauProfIdx = 12;

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
    0x6d,               /* totalLen */
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

/* L2 part of passenger packet */
static TGF_PACKET_L2_STC prvTgfPacketPassengerL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x88, 0x77},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x66, 0x55}                /* saMac */
};

/* passenger VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketPassengerVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};

/* passenger packet's ethertype */
/*static TGF_PACKET_ETHERTYPE_STC prvTgfPacketPassengerEtherTypePart = {0x0640};*/
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketPassengerEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* Unicast packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketPassengerIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2f,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x30,               /* timeToLive */
    0xff,                /* ipv4 protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */ /*0x5EA0*/
    { 6,  6,  6,  6},   /* srcAddr */
    { 8,  8,  8,  8}    /* dstAddr */
};

/* Geneve header
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |Ver|  Opt Len  |O|C|    Rsvd.  |          Protocol Type        |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |        Virtual Network Identifier (VNI)       |    Reserved   |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *  Ver = 0, Opt Len = 12, Protocol Type = 0x0800, VNI = 0x7d
 */
static GT_U8 prvTgfGeneveHeader[] = {
    0x30, 0x00, 0x08, 0x00,
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
    0x00, 0x00, 0x00, 0x01
};

/* Geneve Option 2 */
static GT_U8 prvTgfGeneveTlv2[] = {
    0x11, 0xab, 0x22, 0x08,
    0x00, 0x00, 0x00, 0x02
};
/* Geneve Option 3 */
static GT_U8 prvTgfGeneveTlv3[] = {
    0x11, 0xab, 0x11, 0x08,
    0x00, 0x00, 0x00, 0x04
};

/* Geneve Option 4 */
static GT_U8 prvTgfGeneveTlv4[] = {
    0x11, 0xab, 0x22, 0x08,
    0x00, 0x00, 0x00, 0x08
};

/* Geneve Option 5 */
static GT_U8 prvTgfGeneveTlv5[] = {
    0x11, 0xab, 0x22, 0x08,
    0x00, 0x00, 0x00, 0x10
};

/* Geneve Option 6 */
static GT_U8 prvTgfGeneveTlv6[] = {
    0x11, 0xab, 0x22, 0x08,
    0x00, 0x00, 0x00, 0x20
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
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

/* Geneve Option2 part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketGeneveTlv2Part = {
        sizeof(prvTgfGeneveTlv2),
        prvTgfGeneveTlv2
};

/* Geneve Option3 part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketGeneveTlv3Part = {
        sizeof(prvTgfGeneveTlv3),
        prvTgfGeneveTlv3
};

/* Geneve Option4 part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketGeneveTlv4Part = {
        sizeof(prvTgfGeneveTlv4),
        prvTgfGeneveTlv4
};

/* Geneve Option5 part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketGeneveTlv5Part = {
        sizeof(prvTgfGeneveTlv5),
        prvTgfGeneveTlv5
};

/* Geneve Option6 part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketGeneveTlv6Part = {
        sizeof(prvTgfGeneveTlv6),
        prvTgfGeneveTlv6
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of unicast packet with 6 TLVs */
static TGF_PACKET_PART_STC prvTgfPacketPartArray6Tlv[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_UDP_E,       &prvTgfPacketUdpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketGeneveHeaderPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketGeneveTlv1Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketGeneveTlv2Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketGeneveTlv3Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketGeneveTlv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketGeneveTlv5Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketGeneveTlv6Part},
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketPassengerEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketPassengerIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Geneve header length with 6 TLV */
#define PRV_TGF_GENEVE_TOTAL_HEADER_6TLV_LEN_CNS \
    sizeof(prvTgfGeneveHeader) + sizeof(prvTgfGeneveTlv1) + sizeof(prvTgfGeneveTlv2) \
    + sizeof(prvTgfGeneveTlv3) + sizeof(prvTgfGeneveTlv4) \
    + sizeof(prvTgfGeneveTlv5) + sizeof(prvTgfGeneveTlv6)

/* LENGTH of Geneve packet 2ith 6 TLVs*/
#define PRV_TGF_GENEVE_PACKET_6TLV_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_IPV4_HEADER_SIZE_CNS + TGF_UDP_HEADER_SIZE_CNS + PRV_TGF_GENEVE_TOTAL_HEADER_6TLV_LEN_CNS \
    + TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* normal network PACKET with 6 TLV to send */
static TGF_PACKET_STC prvTgfPacketInfo6Tlv =
{
    PRV_TGF_GENEVE_PACKET_6TLV_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray6Tlv) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray6Tlv                                        /* partsArray */
};

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_TTI_KEY_SIZE_ENT                        keySizeOrig;
    CPSS_DXCH_TTI_TUNNEL_HEADER_LENGTH_PROFILE_STC  tunnelHeaderLengthProfile;
    CPSS_DXCH_PPU_PROFILE_ENTRY_STC                 profileEntry;
    CPSS_DXCH_PPU_SP_BUS_STC                        spBusProfile;
    GT_BOOL                                         ppuEnable;
    GT_U32                                          ppuProfileIdx;
    CPSS_DXCH_PPU_GLOBAL_CONFIG_STC                 globalConfig;
    PRV_TGF_TTI_OFFSET_TYPE_ENT                     origUdbType[4];
    GT_U8                                           origUdb[4];
    CPSS_DXCH_PCL_OFFSET_TYPE_ENT                   pclUdbOffsetType[7];
    GT_U8                                           pclUdbOffset[7];
} prvTgfRestoreCfg;

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
 * @internal prvTgfPpuGeneveLoopbackPortConfigSet function
 * @endinternal
 *
 * @brief    Set basic port vlan configuration
 *
 */
static GT_VOID prvTgfPpuGeneveLoopbackPortConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U8       tagArray[] = {1, 1, 1, 1};

    /* AUTODOC: create VLAN 5 with untagged ports 0,1 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS, prvTgfPortsArray,
                                           NULL, tagArray, 2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet failed");

    /* AUTODOC: create VLAN 5 with untagged ports 0,1 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_RECV_VLANID_CNS, prvTgfPortsArray+2,
                                           NULL, tagArray+2, 2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet failed");

    /* AUTODOC: create VLAN 5 with untagged ports 0,1 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_EXP_VLANID_CNS, prvTgfPortsArray+2,
                                           NULL, tagArray+2, 2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet failed");

}

/**
 * @internal prvTgfPpuGeneveLoopbackVariableTunnelLengthConfigSet function
 * @endinternal
 *
 * @brief    Set variable tunnel length configuration
 *
 */
static GT_VOID prvTgfPpuGeneveLoopbackVariableTunnelLengthConfigSet
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
 * @internal prvTgfPpuGeneveLoopbackTtiConfigSet function
 * @endinternal
 *
 * @brief    Set TTI configuration for tunnel terminating IPV4 UDP packet.
 *
 */
static GT_VOID prvTgfPpuGeneveLoopbackTtiConfigSet
(
    GT_VOID
)
{
    GT_STATUS                rc;
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;
    PRV_TGF_TTI_ACTION_2_STC ttiAction;
    GT_U32                   ruleIndex = prvTgfTtiRuleIndex;

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

    /* store original user defined bytes */
    rc = prvTgfTtiUserDefinedByteGet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV4_OTHER_E, 0,
            &prvTgfRestoreCfg.origUdbType[0], &prvTgfRestoreCfg.origUdb[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteGet failed");

    rc = prvTgfTtiUserDefinedByteGet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV4_OTHER_E, 1,
            &prvTgfRestoreCfg.origUdbType[1], &prvTgfRestoreCfg.origUdb[1]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteGet failed");

    rc = prvTgfTtiUserDefinedByteGet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV4_OTHER_E, 2,
            &prvTgfRestoreCfg.origUdbType[2], &prvTgfRestoreCfg.origUdb[2]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteGet failed");

    rc = prvTgfTtiUserDefinedByteGet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV4_OTHER_E, 3,
            &prvTgfRestoreCfg.origUdbType[3], &prvTgfRestoreCfg.origUdb[3]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteGet failed");

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
    rc = prvTgfTtiRule2Set(ruleIndex, PRV_TGF_TTI_KEY_UDB_IPV4_OTHER_E,
                              &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");

    rc = prvCpssDxChPpuTtiActionProfileIndexSet(prvTgfDevNum, ruleIndex, prvTgfPpuProfileIdx);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssDxChPpuTtiActionProfileIndexSet failed");
}

/**
 * @internal prvTgfPpuGeneveLoopbackPpuInitConfigSet function
 * @endinternal
 *
 * @brief    Set PPU init configuration
 *           - Enable PPU in TTI global configuration ext2 and PPU unit.
 *           - Set default PPU profile index to ingress port
 *           - Configure PPU profile at default PPU profile index
 *
 */
static GT_VOID prvTgfPpuGeneveLoopbackPpuInitConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc;
    CPSS_DXCH_PPU_PROFILE_ENTRY_STC profileEntry;
    CPSS_DXCH_PPU_SP_BUS_STC        spBusProfile;
    GT_U32 i;

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
    rc = cpssDxChPpuSrcPortProfileIndexGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                         &prvTgfRestoreCfg.ppuProfileIdx);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssDxChPpuSrcPortProfileIndexGet failed");

    /* set default PPU Profile Idx of source port */
    rc = cpssDxChPpuSrcPortProfileIndexSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                         prvTgfPpuProfileIdx);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPpuSrcPortProfileIndexGet failed");

    /* ------------------------------------------
     * 3. Set PPU profile data to PPU profile table
     */

    /* save original PPU profile table entry*/
    rc = cpssDxChPpuProfileGet(prvTgfDevNum, prvTgfPpuProfileIdx, &prvTgfRestoreCfg.profileEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuProfileSet failed");

    /* set PPU Profile entry */
    profileEntry.ppuEnable           = GT_TRUE;
    profileEntry.anchorType          = CPSS_DXCH_PPU_PROFILE_ENTRY_ANCHOR_TYPE_TUNNEL_L4_E;
    profileEntry.spBusDefaultProfile = prvTgfSpBusProfileIdx;
    profileEntry.offset              = 4; /* 2 byte granularity */
    profileEntry.ppuState            = prvTgfPpuState;

    /* write entry to PPU profile table */
    rc = cpssDxChPpuProfileSet(prvTgfDevNum, prvTgfPpuProfileIdx, &profileEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuProfileSet failed");

    /* ------------------------------------------
     * 4. Set SP Bus profile data to SP Bus profile table
     */

    /* save original spBusProfile */
    rc = cpssDxChPpuSpBusDefaultProfileGet(prvTgfDevNum, prvTgfSpBusProfileIdx, &prvTgfRestoreCfg.spBusProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuSpBusDefaultProfileGet failed");

    cpssOsMemSet(&spBusProfile, 0, sizeof(spBusProfile));

    /* write SP Bus profile entry */
    spBusProfile.data[16] = 1; /* up0 */
    spBusProfile.data[17] = 2; /* up1 */
    rc = cpssDxChPpuSpBusDefaultProfileSet(prvTgfDevNum, prvTgfSpBusProfileIdx, &spBusProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuSpBusDefaultProfileSet failed");

    /* ------------------------------------------
    * 5. Set UDB Metadata
    */
    /* store pcl udb data */

    for (i=0; i<7; i++)
    {
        rc = cpssDxChPclUserDefinedByteGet(prvTgfDevNum,
                                          CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E,
                                          CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E,
                                          CPSS_PCL_DIRECTION_INGRESS_E,
                                          30,
                                          &prvTgfRestoreCfg.pclUdbOffsetType[i],
                                          &prvTgfRestoreCfg.pclUdbOffset[i]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPclUserDefinedByteGet failed");
    }

    /* Orig Vlan */
    rc = cpssDxChPclUserDefinedByteSet(prvTgfDevNum,
                                       CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E,
                                       CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E,
                                       CPSS_PCL_DIRECTION_INGRESS_E,
                                       30,
                                       CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                       27);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPclUserDefinedByteSet failed");

    /* Orig eport MSB [13:8] */
    rc = cpssDxChPclUserDefinedByteSet(prvTgfDevNum,
                                       CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E,
                                       CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E,
                                       CPSS_PCL_DIRECTION_INGRESS_E,
                                       31,
                                       CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                       22);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPclUserDefinedByteSet failed");

    /* Orig eport LSB [7:0]]*/
    rc = cpssDxChPclUserDefinedByteSet(prvTgfDevNum,
                                       CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E,
                                       CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E,
                                       CPSS_PCL_DIRECTION_INGRESS_E,
                                       32,
                                       CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                       21);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPclUserDefinedByteSet failed");

    /*Src IP Byte 3 */
    rc = cpssDxChPclUserDefinedByteSet(prvTgfDevNum,
                                       CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E,
                                       CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E,
                                       CPSS_PCL_DIRECTION_INGRESS_E,
                                       33,
                                       CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                       34);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPclUserDefinedByteSet failed");

    /*Src IP Byte 2 */
    rc = cpssDxChPclUserDefinedByteSet(prvTgfDevNum,
                                       CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E,
                                       CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E,
                                       CPSS_PCL_DIRECTION_INGRESS_E,
                                       34,
                                       CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                       33);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPclUserDefinedByteSet failed");

    /*Src IP Byte 1 */
    rc = cpssDxChPclUserDefinedByteSet(prvTgfDevNum,
                                       CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E,
                                       CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E,
                                       CPSS_PCL_DIRECTION_INGRESS_E,
                                       35,
                                       CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                       32);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPclUserDefinedByteSet failed");

    /*Src IP Byte 0 */
    rc = cpssDxChPclUserDefinedByteSet(prvTgfDevNum,
                                       CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E,
                                       CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E,
                                       CPSS_PCL_DIRECTION_INGRESS_E,
                                       36,
                                       CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                       31);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPclUserDefinedByteSet failed");
}

#define OFFSET_MSB2LSB(x) ((32 - x / 8 - 1) * 8 + ((x) % 8))

/**
 * @internal prvTgfPpuGeneveLoopbackPpuKstg0ConfigSet function
 * @endinternal
 *
 * @brief    Set PPU K_stg 0 configuration
 *           - Set K_stg key generation profile.
 *           - Set tti key and mask to K_stg tcam table
 *           - Populate and write ppu Action Entry
 *
 */
static GT_VOID prvTgfPpuGeneveLoopbackPpuKstg0ConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32      kstgNum              = 0;
    GT_U32      keyGenProfileIdx     = (prvTgfPpuState >> 4) & 0xf;
    GT_U32      kstgTcamProfileIdx   = 8;
    GT_U32      kstgActionProfileIdx = kstgTcamProfileIdx;
    GT_U8       tlvCount            = 0;
    CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC keyGenProfile;
    CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC    ppuActionProfile;
    CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC      tcamEntry;

    /* initialize to ppuActionProfile to 0 */
    cpssOsMemSet(&ppuActionProfile, 0, sizeof(ppuActionProfile));
    cpssOsMemSet(&keyGenProfile, 0, sizeof(keyGenProfile));
    cpssOsMemSet(&tcamEntry, 0, sizeof(tcamEntry));

    /* set key generation profile */
    keyGenProfile.byteSelOffset[0] = 21; /* UDB Byte 0 */
    keyGenProfile.byteSelOffset[1] = 23; /* UDB Byte 2 */
    keyGenProfile.byteSelOffset[2] = 24; /* UDB Byte 3 */
    keyGenProfile.byteSelOffset[3] = 25; /* UDB Byte 4 */
    keyGenProfile.byteSelOffset[4] = 26; /* UDB Byte 5 */
    keyGenProfile.byteSelOffset[5] = 27; /* UDB Byte 6 */
    keyGenProfile.byteSelOffset[6] = 62; /* Count */

    rc = cpssDxChPpuKstgKeyGenProfileSet(prvTgfDevNum, kstgNum, keyGenProfileIdx, &keyGenProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgKeyGenProfileSet failed");

    /* ---------------------------------------------------------------------------------------*/
    /* Process Geneve Header + TLV0 */

    /* set kstg tcam profile */
    tlvCount = 0;
    tcamEntry.isValid = 1;
    tcamEntry.keyLsb  = ((prvTgfPacketPassengerIpv4Part.srcAddr[2] & 0xFF) << 24) |
                        ((prvTgfPacketPassengerIpv4Part.srcAddr[3] & 0xFF) << 16) |
                        ((prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS] & 0xFF) << 8) |
                        (PRV_TGF_SEND_VLANID_CNS & 0xFF);
    tcamEntry.keyMsb  = ((tlvCount & 0xFF) << 16) |
                        ((prvTgfPacketPassengerIpv4Part.srcAddr[0] & 0xFF) <<  8) |
                        ((prvTgfPacketPassengerIpv4Part.srcAddr[1] & 0xFF) <<  0);
    tcamEntry.maskLsb = 0x0000FF00;
    tcamEntry.maskMsb = 0xFF000000;
    rc = cpssDxChPpuKstgTcamEntrySet(prvTgfDevNum,
            kstgNum, kstgTcamProfileIdx, &tcamEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgTcamEntrySet failed");

    /* Populate ppu action profile entry */
    /* REG0 and REG1 implement SP<Length Left>= Geneve <Opt Len> - (TLV<Length>+1) */
    /* Extract Geneve <Opt Len> to REG 0 */
    ppuActionProfile.rotActionEntry[0].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_HD_E;
    ppuActionProfile.rotActionEntry[0].srcOffset         = OFFSET_MSB2LSB(2);
    ppuActionProfile.rotActionEntry[0].srcNumValBits     = 5; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[0].shiftLeftRightSel = CPSS_DXCH_PPU_KSTG_ROT_SHIFT_LEFT_E;
    ppuActionProfile.rotActionEntry[0].shiftNum          = 2;
    ppuActionProfile.rotActionEntry[0].addSubConstSel    = CPSS_DXCH_PPU_KSTG_ROT_ADD_SEL_E;
    ppuActionProfile.rotActionEntry[0].addSubConst       = 8;
    ppuActionProfile.rotActionEntry[0].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_SUB_REG_E;
    ppuActionProfile.rotActionEntry[0].funcSecondOperand = CPSS_DXCH_PPU_KSTG_ROT_ACTION_SECOND_OPERAND_REG1_E;
    ppuActionProfile.rotActionEntry[0].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[0].ldSpBusOffset     = 1; /* byte offset */
    ppuActionProfile.rotActionEntry[0].ldSpBusNumBytes   = 1; /* number of bytes */

    /* Extract TLV<Length> to REG1, Header shift */
    ppuActionProfile.rotActionEntry[1].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_HD_E;
    ppuActionProfile.rotActionEntry[1].srcOffset         = OFFSET_MSB2LSB(91);
    ppuActionProfile.rotActionEntry[1].srcNumValBits     = 4; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[1].shiftLeftRightSel = CPSS_DXCH_PPU_KSTG_ROT_SHIFT_LEFT_E;
    ppuActionProfile.rotActionEntry[1].shiftNum          = 2;
    ppuActionProfile.rotActionEntry[1].addSubConstSel    = CPSS_DXCH_PPU_KSTG_ROT_ADD_SEL_E;
    ppuActionProfile.rotActionEntry[1].addSubConst       = 4;
    ppuActionProfile.rotActionEntry[1].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_NONE_E;
    ppuActionProfile.rotActionEntry[1].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_HD_SHIFT_E;

    /* Extract 32bit TLV value add Add to previous value */
    ppuActionProfile.rotActionEntry[2].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_HD_E;
    ppuActionProfile.rotActionEntry[2].srcOffset         = OFFSET_MSB2LSB(120);
    ppuActionProfile.rotActionEntry[2].srcNumValBits     = 7; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[2].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_ADD_REG_E;
    ppuActionProfile.rotActionEntry[2].funcSecondOperand = CPSS_DXCH_PPU_KSTG_ROT_ACTION_SECOND_OPERAND_REG3_E;
    ppuActionProfile.rotActionEntry[2].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[2].ldSpBusOffset     = 3; /* byte offset */
    ppuActionProfile.rotActionEntry[2].ldSpBusNumBytes   = 1; /* number of bytes */

    /* extract previous value at spBus[3] */
    ppuActionProfile.rotActionEntry[3].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[3].srcOffset         = 24;
    ppuActionProfile.rotActionEntry[3].srcNumValBits     = 7; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[3].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_NONE_E;

    ppuActionProfile.setNextState                        = keyGenProfileIdx << 4;
    ppuActionProfile.counterOper                         = CPSS_DXCH_PPU_KSTG_ACT_COUNTER_OPER_INC_E;

    /* Write PPU action table entry */
    rc = cpssDxChPpuActionTableEntrySet(prvTgfDevNum,
            kstgNum, kstgActionProfileIdx, &ppuActionProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuActionTableEntrySet failed");

    /* ---------------------------------------------------------------------------------------*/
    /* Process TLV3 */

    cpssOsMemSet(&tcamEntry, 0, sizeof(tcamEntry));
    cpssOsMemSet(&ppuActionProfile, 0, sizeof(ppuActionProfile));

    /* set kstg tcam profile */
    tlvCount = 3;
    tcamEntry.isValid = 1;
    tcamEntry.keyLsb  = ((prvTgfPacketPassengerIpv4Part.srcAddr[2] & 0xFF) << 24) |
                        ((prvTgfPacketPassengerIpv4Part.srcAddr[3] & 0xFF) << 16) |
                        ((prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS] & 0xFF) << 8) |
                        (PRV_TGF_SEND_VLANID_CNS & 0xFF);
    tcamEntry.keyMsb  = ((tlvCount & 0xFF) << 16) |
                        ((prvTgfPacketPassengerIpv4Part.srcAddr[0] & 0xFF) <<  8) |
                        ((prvTgfPacketPassengerIpv4Part.srcAddr[1] & 0xFF) <<  0);
    tcamEntry.maskLsb = 0x0000FF00;
    tcamEntry.maskMsb = 0xFF000000;
    rc = cpssDxChPpuKstgTcamEntrySet(prvTgfDevNum,
            kstgNum, kstgTcamProfileIdx+1, &tcamEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgTcamEntrySet failed");

    /* Populate ppu action profile entry */
    /* REG0 and REG1 implement SP<Length Left>= Geneve <Opt Len> - (TLV<Length>+1) */
    /* Extract Geneve <Opt Len> to REG 0 */
    ppuActionProfile.rotActionEntry[0].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[0].srcOffset         = 8;
    ppuActionProfile.rotActionEntry[0].srcNumValBits     = 7; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[0].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_SUB_REG_E;
    ppuActionProfile.rotActionEntry[0].funcSecondOperand = CPSS_DXCH_PPU_KSTG_ROT_ACTION_SECOND_OPERAND_REG1_E;
    ppuActionProfile.rotActionEntry[0].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[0].ldSpBusOffset     = 1; /* byte offset */
    ppuActionProfile.rotActionEntry[0].ldSpBusNumBytes   = 1; /* number of bytes */

    /* Extract TLV<Length> to REG1, Header shift */
    ppuActionProfile.rotActionEntry[1].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_HD_E;
    ppuActionProfile.rotActionEntry[1].srcOffset         = OFFSET_MSB2LSB(27);
    ppuActionProfile.rotActionEntry[1].srcNumValBits     = 4; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[1].shiftLeftRightSel = CPSS_DXCH_PPU_KSTG_ROT_SHIFT_LEFT_E;
    ppuActionProfile.rotActionEntry[1].shiftNum          = 1;
    ppuActionProfile.rotActionEntry[1].addSubConstSel    = CPSS_DXCH_PPU_KSTG_ROT_ADD_SEL_E;
    ppuActionProfile.rotActionEntry[1].addSubConst       = 2;
    ppuActionProfile.rotActionEntry[1].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_NONE_E;
    ppuActionProfile.rotActionEntry[1].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_HD_SHIFT_E;

    /* Extract 32bit TLV value add Add to previous value */
    ppuActionProfile.rotActionEntry[2].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_HD_E;
    ppuActionProfile.rotActionEntry[2].srcOffset         = 192;
    ppuActionProfile.rotActionEntry[2].srcNumValBits     = 7; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[2].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_ADD_REG_E;
    ppuActionProfile.rotActionEntry[2].funcSecondOperand = CPSS_DXCH_PPU_KSTG_ROT_ACTION_SECOND_OPERAND_REG3_E;
    ppuActionProfile.rotActionEntry[2].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[2].ldSpBusOffset     = 3; /* byte offset */
    ppuActionProfile.rotActionEntry[2].ldSpBusNumBytes   = 1; /* number of bytes */

    /* extract previous value at spBus[3] */
    ppuActionProfile.rotActionEntry[3].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[3].srcOffset         = 24;
    ppuActionProfile.rotActionEntry[3].srcNumValBits     = 7; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[3].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_NONE_E;

    ppuActionProfile.setNextState                        = keyGenProfileIdx << 4;
    ppuActionProfile.counterOper                         = CPSS_DXCH_PPU_KSTG_ACT_COUNTER_OPER_INC_E;

    /* Write PPU action table entry */
    rc = cpssDxChPpuActionTableEntrySet(prvTgfDevNum,
            kstgNum, kstgActionProfileIdx+1, &ppuActionProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuActionTableEntrySet failed");
}

/**
 * @internal prvTgfPpuGeneveLoopbackPpuKstg1ConfigSet function
 * @endinternal
 *
 * @brief    Set PPU K_stg 1 configuration
 *           - Set K_stg key generation profile.
 *           - Set tti key and mask to K_stg tcam table
 *           - Populate and write ppu Action Entry
 *
 */
static GT_VOID prvTgfPpuGeneveLoopbackPpuKstg1ConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32      kstgNum              = 1;
    GT_U32      keyGenProfileIdx     = (prvTgfPpuState >> 4) & 0xf;
    GT_U32      kstgTcamProfileIdx   = 8;
    GT_U32      kstgActionProfileIdx = kstgTcamProfileIdx;
    GT_U8       tlvCount             = 0;
    CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC keyGenProfile;
    CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC    ppuActionProfile;
    CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC      tcamEntry;

    /* initialize to ppuActionProfile to 0 */
    cpssOsMemSet(&ppuActionProfile, 0, sizeof(ppuActionProfile));
    cpssOsMemSet(&keyGenProfile, 0, sizeof(keyGenProfile));
    cpssOsMemSet(&tcamEntry, 0, sizeof(tcamEntry));

    /* set key generation profile */
    keyGenProfile.byteSelOffset[0] = 21; /* UDB Byte 0 */
    keyGenProfile.byteSelOffset[1] = 23; /* UDB Byte 2 */
    keyGenProfile.byteSelOffset[2] = 24; /* UDB Byte 3 */
    keyGenProfile.byteSelOffset[3] = 25; /* UDB Byte 4 */
    keyGenProfile.byteSelOffset[4] = 26; /* UDB Byte 5 */
    keyGenProfile.byteSelOffset[5] = 27; /* UDB Byte 6 */
    keyGenProfile.byteSelOffset[6] = 62; /* UDB Byte 7 */

    rc = cpssDxChPpuKstgKeyGenProfileSet(prvTgfDevNum, kstgNum, keyGenProfileIdx, &keyGenProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgKeyGenProfileSet failed");

    /* set kstg tcam profile */
    tlvCount = 1;
    tcamEntry.isValid = 1;
    tcamEntry.keyLsb  = ((prvTgfPacketPassengerIpv4Part.srcAddr[2] & 0xFF) << 24) |
                        ((prvTgfPacketPassengerIpv4Part.srcAddr[3] & 0xFF) << 16) |
                        ((prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS] & 0xFF) << 8) |
                        (PRV_TGF_SEND_VLANID_CNS & 0xFF);
    tcamEntry.keyMsb  = ((tlvCount & 0xFF) << 16) |
                        ((prvTgfPacketPassengerIpv4Part.srcAddr[0] & 0xFF) <<  8) |
                        ((prvTgfPacketPassengerIpv4Part.srcAddr[1] & 0xFF) <<  0);
    tcamEntry.maskLsb = 0x0000FF00;
    tcamEntry.maskMsb = 0xFF000000;
    rc = cpssDxChPpuKstgTcamEntrySet(prvTgfDevNum,
            kstgNum, kstgTcamProfileIdx, &tcamEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgTcamEntrySet failed");

    /* Populate ppu action profile entry */
    /* REG0 and REG1 implement SP<Length Left>= Geneve <Opt Len> - (TLV<Length>+1) */
    /* Extract Geneve <Opt Len> to REG 0 */
    ppuActionProfile.rotActionEntry[0].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[0].srcOffset         = 8;
    ppuActionProfile.rotActionEntry[0].srcNumValBits     = 7; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[0].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_SUB_REG_E;
    ppuActionProfile.rotActionEntry[0].funcSecondOperand = CPSS_DXCH_PPU_KSTG_ROT_ACTION_SECOND_OPERAND_REG1_E;
    ppuActionProfile.rotActionEntry[0].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[0].ldSpBusOffset     = 1; /* byte offset */
    ppuActionProfile.rotActionEntry[0].ldSpBusNumBytes   = 1; /* number of bytes */

    /* Extract TLV<Length> to REG1, Header shift */
    ppuActionProfile.rotActionEntry[1].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_HD_E;
    ppuActionProfile.rotActionEntry[1].srcOffset         = OFFSET_MSB2LSB(27);
    ppuActionProfile.rotActionEntry[1].srcNumValBits     = 4; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[1].shiftLeftRightSel = CPSS_DXCH_PPU_KSTG_ROT_SHIFT_LEFT_E;
    ppuActionProfile.rotActionEntry[1].shiftNum          = 1;
    ppuActionProfile.rotActionEntry[1].addSubConstSel    = CPSS_DXCH_PPU_KSTG_ROT_ADD_SEL_E;
    ppuActionProfile.rotActionEntry[1].addSubConst       = 2;
    ppuActionProfile.rotActionEntry[1].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_NONE_E;
    ppuActionProfile.rotActionEntry[1].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_HD_SHIFT_E;

    /* Extract 32bit TLV value add Add to previous value */
    ppuActionProfile.rotActionEntry[2].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_HD_E;
    ppuActionProfile.rotActionEntry[2].srcOffset         = 192;
    ppuActionProfile.rotActionEntry[2].srcNumValBits     = 7; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[2].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_ADD_REG_E;
    ppuActionProfile.rotActionEntry[2].funcSecondOperand = CPSS_DXCH_PPU_KSTG_ROT_ACTION_SECOND_OPERAND_REG3_E;
    ppuActionProfile.rotActionEntry[2].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[2].ldSpBusOffset     = 3; /* byte offset */
    ppuActionProfile.rotActionEntry[2].ldSpBusNumBytes   = 1; /* number of bytes */

    /* extract previous value at spBus[3] */
    ppuActionProfile.rotActionEntry[3].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[3].srcOffset         = 24;
    ppuActionProfile.rotActionEntry[3].srcNumValBits     = 7; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[3].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_NONE_E;

    ppuActionProfile.setNextState                        = keyGenProfileIdx << 4;
    ppuActionProfile.counterOper                         = CPSS_DXCH_PPU_KSTG_ACT_COUNTER_OPER_INC_E;

    /* Write PPU action table entry */
    rc = cpssDxChPpuActionTableEntrySet(prvTgfDevNum,
            kstgNum, kstgActionProfileIdx, &ppuActionProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuActionTableEntrySet failed");

    /* initialize to ppuActionProfile to 0 */
    cpssOsMemSet(&ppuActionProfile, 0, sizeof(ppuActionProfile));
    cpssOsMemSet(&tcamEntry, 0, sizeof(tcamEntry));

    /* set kstg tcam profile */
    tlvCount = 4;
    tcamEntry.isValid = 1;
    tcamEntry.keyLsb  = ((prvTgfPacketPassengerIpv4Part.srcAddr[2] & 0xFF) << 24) |
                        ((prvTgfPacketPassengerIpv4Part.srcAddr[3] & 0xFF) << 16) |
                        ((prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS] & 0xFF) << 8) |
                        (PRV_TGF_SEND_VLANID_CNS & 0xFF);
    tcamEntry.keyMsb  = ((tlvCount & 0xFF) << 16) |
                        ((prvTgfPacketPassengerIpv4Part.srcAddr[0] & 0xFF) <<  8) |
                        ((prvTgfPacketPassengerIpv4Part.srcAddr[1] & 0xFF) <<  0);
    tcamEntry.maskLsb = 0x0000FF00;
    tcamEntry.maskMsb = 0xFF000000;
    rc = cpssDxChPpuKstgTcamEntrySet(prvTgfDevNum,
            kstgNum, kstgTcamProfileIdx+1, &tcamEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgTcamEntrySet failed");

    /* Populate ppu action profile entry */
    /* REG0 and REG1 implement SP<Length Left>= Geneve <Opt Len> - (TLV<Length>+1) */
    /* Extract Geneve <Opt Len> to REG 0 */
    ppuActionProfile.rotActionEntry[0].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[0].srcOffset         = 8;
    ppuActionProfile.rotActionEntry[0].srcNumValBits     = 7; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[0].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_SUB_REG_E;
    ppuActionProfile.rotActionEntry[0].funcSecondOperand = CPSS_DXCH_PPU_KSTG_ROT_ACTION_SECOND_OPERAND_REG1_E;
    ppuActionProfile.rotActionEntry[0].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[0].ldSpBusOffset     = 1; /* byte offset */
    ppuActionProfile.rotActionEntry[0].ldSpBusNumBytes   = 1; /* number of bytes */

    /* Extract TLV<Length> to REG1, Header shift */
    ppuActionProfile.rotActionEntry[1].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_HD_E;
    ppuActionProfile.rotActionEntry[1].srcOffset         = OFFSET_MSB2LSB(27);
    ppuActionProfile.rotActionEntry[1].srcNumValBits     = 4; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[1].shiftLeftRightSel = CPSS_DXCH_PPU_KSTG_ROT_SHIFT_LEFT_E;
    ppuActionProfile.rotActionEntry[1].shiftNum          = 1;
    ppuActionProfile.rotActionEntry[1].addSubConstSel    = CPSS_DXCH_PPU_KSTG_ROT_ADD_SEL_E;
    ppuActionProfile.rotActionEntry[1].addSubConst       = 2;
    ppuActionProfile.rotActionEntry[1].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_NONE_E;
    ppuActionProfile.rotActionEntry[1].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_HD_SHIFT_E;

    /* Extract 32bit TLV value add Add to previous value */
    ppuActionProfile.rotActionEntry[2].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_HD_E;
    ppuActionProfile.rotActionEntry[2].srcOffset         = 192;
    ppuActionProfile.rotActionEntry[2].srcNumValBits     = 7; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[2].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_ADD_REG_E;
    ppuActionProfile.rotActionEntry[2].funcSecondOperand = CPSS_DXCH_PPU_KSTG_ROT_ACTION_SECOND_OPERAND_REG3_E;
    ppuActionProfile.rotActionEntry[2].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[2].ldSpBusOffset     = 3; /* byte offset */
    ppuActionProfile.rotActionEntry[2].ldSpBusNumBytes   = 1; /* number of bytes */

    /* extract previous value at spBus[3] */
    ppuActionProfile.rotActionEntry[3].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[3].srcOffset         = 24;
    ppuActionProfile.rotActionEntry[3].srcNumValBits     = 7; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[3].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_NONE_E;

    ppuActionProfile.setNextState                        = keyGenProfileIdx << 4;
    ppuActionProfile.counterOper                         = CPSS_DXCH_PPU_KSTG_ACT_COUNTER_OPER_INC_E;

    /* Write PPU action table entry */
    rc = cpssDxChPpuActionTableEntrySet(prvTgfDevNum,
            kstgNum, kstgActionProfileIdx+1, &ppuActionProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuActionTableEntrySet failed");
}

/**
 * @internal prvTgfPpuGeneveLoopbackPpuKstg2ConfigSet function
 * @endinternal
 *
 * @brief    Set PPU K_stg 2 configuration
 *           - Set K_stg key generation profile.
 *           - Set tti key and mask to K_stg tcam table
 *           - Populate and write ppu Action Entry
 *
 */
static GT_VOID prvTgfPpuGeneveLoopbackPpuKstg2ConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32      kstgNum              = 2;
    GT_U32      keyGenProfileIdx     = (prvTgfPpuState >> 4) & 0xf;
    GT_U32      kstgTcamProfileIdx   = 8;
    GT_U32      kstgActionProfileIdx = kstgTcamProfileIdx;
    GT_U32      dauProfileIdx        = 12;
    GT_U8       tlvCount             = 0;
    CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC keyGenProfile;
    CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC    ppuActionProfile;
    CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC      tcamEntry;

    /* initialize to ppuActionProfile to 0 */
    cpssOsMemSet(&ppuActionProfile, 0, sizeof(ppuActionProfile));
    cpssOsMemSet(&keyGenProfile, 0, sizeof(keyGenProfile));
    cpssOsMemSet(&tcamEntry, 0, sizeof(tcamEntry));

    /* set key generation profile */
    keyGenProfile.byteSelOffset[0] = 21; /* UDB Byte 0 */
    keyGenProfile.byteSelOffset[1] = 23; /* UDB Byte 2 */
    keyGenProfile.byteSelOffset[2] = 24; /* UDB Byte 3 */
    keyGenProfile.byteSelOffset[3] = 25; /* UDB Byte 4 */
    keyGenProfile.byteSelOffset[4] = 26; /* UDB Byte 5 */
    keyGenProfile.byteSelOffset[5] = 27; /* UDB Byte 6 */
    keyGenProfile.byteSelOffset[6] = 62; /* Count */

    rc = cpssDxChPpuKstgKeyGenProfileSet(prvTgfDevNum, kstgNum, keyGenProfileIdx, &keyGenProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgKeyGenProfileSet failed");

    /* set kstg tcam profile */
    tlvCount = 2;
    tcamEntry.isValid = 1;
    tcamEntry.keyLsb  = ((prvTgfPacketPassengerIpv4Part.srcAddr[2] & 0xFF) << 24) |
                        ((prvTgfPacketPassengerIpv4Part.srcAddr[3] & 0xFF) << 16) |
                        ((prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS] & 0xFF) << 8) |
                        (PRV_TGF_SEND_VLANID_CNS & 0xFF);
    tcamEntry.keyMsb  = ((tlvCount & 0xFF) << 16) |
                        ((prvTgfPacketPassengerIpv4Part.srcAddr[0] & 0xFF) <<  8) |
                        ((prvTgfPacketPassengerIpv4Part.srcAddr[1] & 0xFF) <<  0);
    tcamEntry.maskLsb = 0x0000FF00;
    tcamEntry.maskMsb = 0xFF000000;
    rc = cpssDxChPpuKstgTcamEntrySet(prvTgfDevNum,
            kstgNum, kstgTcamProfileIdx, &tcamEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgTcamEntrySet failed");

    /* Populate ppu action profile entry */
    /* REG0 and REG1 implement SP<Length Left>= Geneve <Opt Len> - (TLV<Length>+1) */
    /* Extract Geneve <Opt Len> to REG 0 */
    ppuActionProfile.rotActionEntry[0].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[0].srcOffset         = 8;
    ppuActionProfile.rotActionEntry[0].srcNumValBits     = 7; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[0].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_SUB_REG_E;
    ppuActionProfile.rotActionEntry[0].funcSecondOperand = CPSS_DXCH_PPU_KSTG_ROT_ACTION_SECOND_OPERAND_REG1_E;
    ppuActionProfile.rotActionEntry[0].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[0].ldSpBusOffset     = 1; /* byte offset */
    ppuActionProfile.rotActionEntry[0].ldSpBusNumBytes   = 1; /* number of bytes */

    /* Extract TLV<Length> to REG1, Header shift */
    ppuActionProfile.rotActionEntry[1].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_HD_E;
    ppuActionProfile.rotActionEntry[1].srcOffset         = OFFSET_MSB2LSB(27);
    ppuActionProfile.rotActionEntry[1].srcNumValBits     = 4; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[1].shiftLeftRightSel = CPSS_DXCH_PPU_KSTG_ROT_SHIFT_LEFT_E;
    ppuActionProfile.rotActionEntry[1].shiftNum          = 1;
    ppuActionProfile.rotActionEntry[1].addSubConstSel    = CPSS_DXCH_PPU_KSTG_ROT_ADD_SEL_E;
    ppuActionProfile.rotActionEntry[1].addSubConst       = 2;
    ppuActionProfile.rotActionEntry[1].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_NONE_E;
    ppuActionProfile.rotActionEntry[1].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_HD_SHIFT_E;

    /* Extract 32bit TLV value add Add to previous value */
    ppuActionProfile.rotActionEntry[2].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_HD_E;
    ppuActionProfile.rotActionEntry[2].srcOffset         = 192;
    ppuActionProfile.rotActionEntry[2].srcNumValBits     = 7; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[2].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_ADD_REG_E;
    ppuActionProfile.rotActionEntry[2].funcSecondOperand = CPSS_DXCH_PPU_KSTG_ROT_ACTION_SECOND_OPERAND_REG3_E;
    ppuActionProfile.rotActionEntry[2].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[2].ldSpBusOffset     = 3; /* byte offset */
    ppuActionProfile.rotActionEntry[2].ldSpBusNumBytes   = 1; /* number of bytes */

    /* extract previous value at spBus[3] */
    ppuActionProfile.rotActionEntry[3].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[3].srcOffset         = 24;
    ppuActionProfile.rotActionEntry[3].srcNumValBits     = 7; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[3].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_NONE_E;

    ppuActionProfile.setNextState                        = keyGenProfileIdx << 4;
    ppuActionProfile.counterOper                         = CPSS_DXCH_PPU_KSTG_ACT_COUNTER_OPER_INC_E;
    ppuActionProfile.setLoopBack                         = GT_TRUE;

    /* Write PPU action table entry */
    rc = cpssDxChPpuActionTableEntrySet(prvTgfDevNum,
            kstgNum, kstgActionProfileIdx, &ppuActionProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuActionTableEntrySet failed");

    cpssOsMemSet(&ppuActionProfile, 0, sizeof(ppuActionProfile));
    cpssOsMemSet(&tcamEntry, 0, sizeof(tcamEntry));

    /* set kstg tcam profile */
    tlvCount = 5;
    tcamEntry.isValid = 1;
    tcamEntry.keyLsb  = ((prvTgfPacketPassengerIpv4Part.srcAddr[2] & 0xFF) << 24) |
                        ((prvTgfPacketPassengerIpv4Part.srcAddr[3] & 0xFF) << 16) |
                        ((prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS] & 0xFF) << 8) |
                        (PRV_TGF_SEND_VLANID_CNS & 0xFF);
    tcamEntry.keyMsb  = ((tlvCount & 0xFF) << 16) |
                        ((prvTgfPacketPassengerIpv4Part.srcAddr[0] & 0xFF) <<  8) |
                        ((prvTgfPacketPassengerIpv4Part.srcAddr[1] & 0xFF) <<  0);
    tcamEntry.maskLsb = 0x0000FF00;
    tcamEntry.maskMsb = 0xFF000000;
    rc = cpssDxChPpuKstgTcamEntrySet(prvTgfDevNum,
            kstgNum, kstgTcamProfileIdx+1, &tcamEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgTcamEntrySet failed");

    /* Populate ppu action profile entry */
    /* REG0 and REG1 implement SP<Length Left>= Geneve <Opt Len> - (TLV<Length>+1) */
    /* Extract Geneve <Opt Len> to REG 0 */
    ppuActionProfile.rotActionEntry[0].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[0].srcOffset         = 8;
    ppuActionProfile.rotActionEntry[0].srcNumValBits     = 7; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[0].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_SUB_REG_E;
    ppuActionProfile.rotActionEntry[0].funcSecondOperand = CPSS_DXCH_PPU_KSTG_ROT_ACTION_SECOND_OPERAND_REG1_E;
    ppuActionProfile.rotActionEntry[0].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[0].ldSpBusOffset     = 1; /* byte offset */
    ppuActionProfile.rotActionEntry[0].ldSpBusNumBytes   = 1; /* number of bytes */

    /* Extract TLV<Length> to REG1, Header shift */
    ppuActionProfile.rotActionEntry[1].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_HD_E;
    ppuActionProfile.rotActionEntry[1].srcOffset         = OFFSET_MSB2LSB(27);
    ppuActionProfile.rotActionEntry[1].srcNumValBits     = 4; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[1].shiftLeftRightSel = CPSS_DXCH_PPU_KSTG_ROT_SHIFT_LEFT_E;
    ppuActionProfile.rotActionEntry[1].shiftNum          = 1;
    ppuActionProfile.rotActionEntry[1].addSubConstSel    = CPSS_DXCH_PPU_KSTG_ROT_ADD_SEL_E;
    ppuActionProfile.rotActionEntry[1].addSubConst       = 2;
    ppuActionProfile.rotActionEntry[1].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_NONE_E;
    ppuActionProfile.rotActionEntry[1].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_HD_SHIFT_E;

    /* Extract 32bit TLV value add Add to previous value */
    ppuActionProfile.rotActionEntry[2].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_HD_E;
    ppuActionProfile.rotActionEntry[2].srcOffset         = 192;
    ppuActionProfile.rotActionEntry[2].srcNumValBits     = 7; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[2].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_ADD_REG_E;
    ppuActionProfile.rotActionEntry[2].funcSecondOperand = CPSS_DXCH_PPU_KSTG_ROT_ACTION_SECOND_OPERAND_REG3_E;
    ppuActionProfile.rotActionEntry[2].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[2].ldSpBusOffset     = 3; /* byte offset */
    ppuActionProfile.rotActionEntry[2].ldSpBusNumBytes   = 1; /* number of bytes */

    /* extract previous value at spBus[3] */
    ppuActionProfile.rotActionEntry[3].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[3].srcOffset         = 24;
    ppuActionProfile.rotActionEntry[3].srcNumValBits     = 7; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[3].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_NONE_E;

    ppuActionProfile.setNextState                        = dauProfileIdx;
    ppuActionProfile.counterOper                         = CPSS_DXCH_PPU_KSTG_ACT_COUNTER_OPER_INC_E;

    /* Write PPU action table entry */
    rc = cpssDxChPpuActionTableEntrySet(prvTgfDevNum,
            kstgNum, kstgActionProfileIdx+1, &ppuActionProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuActionTableEntrySet failed");
}

/**
 * @internal prvTgfPpuGeneveLoopbackPpuDauConfigSet function
 * @endinternal
 *
 * @brief    Set PPU DAU profile configuration
 *           - Write first four bytes of MAC source address
 *
 */
static GT_VOID prvTgfPpuGeneveLoopbackPpuDauConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32      dauProfileIdx = prvTgfDauProfIdx;
    CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC dauProfile;

    /* initialize DAU profile to 0 */
    cpssOsMemSet(&dauProfile, 0, sizeof(dauProfile));

    /* Populate DAU profile entry to write to first four bytes of MAC source address */

    /* write evlan */
    dauProfile.setDescBits[4].spByteWriteEnable  = GT_TRUE;
    dauProfile.setDescBits[4].spByteSrcOffset    = 3 * 8;   /* bit offset */
    dauProfile.setDescBits[4].spByteNumBits      = 7;       /* num of bits - 1 = 8 - 1 */
    dauProfile.setDescBits[4].spByteTargetOffset = 2179;    /* EVLAN  bit offset */

    /* write entry to DAU profile table */
    rc = cpssDxChPpuDauProfileEntrySet(prvTgfDevNum,
            dauProfileIdx, &dauProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuDauProfileEntrySet failed");

}

/**
 * @internal prvTgfPpuGeneveLoopbackErrorProfileConfigSet function
 * @endinternal
 *
 * @brief    Set PPU DAU error profile.
  *
 */
static GT_VOID prvTgfPpuGeneveErrorProfileConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32      profileIdx = prvTgfErrProfIdx;
    CPSS_DXCH_PPU_ERROR_PROFILE_STC errProfile;

    /* initialize error profile to 0 */
    cpssOsMemSet(&errProfile, 0, sizeof(CPSS_DXCH_PPU_ERROR_PROFILE_STC));

    /* Populate error profile */
    errProfile.errProfileField[1].writeEnable    = GT_TRUE;
    errProfile.errProfileField[1].numBits        = 2;  /* numBits -1 */
    errProfile.errProfileField[1].errorDataField = 1;
    errProfile.errProfileField[1].targetOffset   = 2175; /* user priority */

    /* Populate error profile */
    errProfile.errProfileField[2].writeEnable    = GT_TRUE;
    errProfile.errProfileField[2].numBits        = 7;  /* numBits -1 */
    errProfile.errProfileField[2].errorDataField = PRV_TGF_RECV_VLANID_CNS;
    errProfile.errProfileField[2].targetOffset   = 2179; /* evlan */

    /* set error profile */
    rc = cpssDxChPpuErrorProfileSet(prvTgfDevNum, profileIdx-1, &errProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuErrorProfileSet failed");

}

/**
 * @internal prvTgfPpuGeneveLoopbackErrorProfileConfigRestore function
 * @endinternal
 *
 * @brief    Restore PPU DAU error profile.
  *
 */
static GT_VOID prvTgfPpuGeneveErrorProfileConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32      profileIdx = prvTgfErrProfIdx;
    CPSS_DXCH_PPU_ERROR_PROFILE_STC errProfile;

    /* initialize error profile to 0 */
    cpssOsMemSet(&errProfile, 0, sizeof(CPSS_DXCH_PPU_ERROR_PROFILE_STC));

    /* set error profile */
    rc = cpssDxChPpuErrorProfileSet(prvTgfDevNum, profileIdx-1, &errProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuErrorProfileSet failed");

}

/**
* @internal prvTgfPpuGeneveLoopbackConfigure function
* @endinternal
*
* @brief  Set configuration for Geneve Packet. Extract values from 6 TLVs and add them
*         and update eVid to the value.
*          1. Set Port Configuration
*          2. Initialize PPU
*          3. Configure K_stg0, K_stg1, K_stg2 and DAU
*/
GT_VOID prvTgfPpuGeneveLoopbackConfigure
(
        GT_VOID
)
{
    /* configure port vlan */
    prvTgfPpuGeneveLoopbackPortConfigSet();

    /* set variable tunnel length configuration*/
    prvTgfPpuGeneveLoopbackVariableTunnelLengthConfigSet();

    /* set tti configuration */
    prvTgfPpuGeneveLoopbackTtiConfigSet();

    /* ppu init configuration set */
    prvTgfPpuGeneveLoopbackPpuInitConfigSet();

    /* PPU K_stg 0 configuration Set */
    prvTgfPpuGeneveLoopbackPpuKstg0ConfigSet();

    /* PPU K_stg 1 configuration Set */
    prvTgfPpuGeneveLoopbackPpuKstg1ConfigSet();

    /* PPU K_stg 2 configuration Set */
    prvTgfPpuGeneveLoopbackPpuKstg2ConfigSet();

    /* PPU DAU configuration Set */
    prvTgfPpuGeneveLoopbackPpuDauConfigSet();

}

/**
* @internal prvTgfPpuGeneveLoopbackTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet and capture:
*         Check captured packet.
*/
GT_VOID prvTgfPpuGeneveLoopbackTrafficGenerate
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
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo6Tlv, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* AUTODOC: Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, sendPort);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n", prvTgfDevNum, sendPort);

    cpssOsTimerWkAfter(300);

    /* AUTODOC: stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* AUTODOC: restore port capture setting */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInt, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
            prvTgfDevNum, portInt.devPort.portNum);


    /* AUTODOC: capture packet by trapping to CPU*/
    rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&portInt,
            TGF_PACKET_TYPE_CAPTURE_E,
            GT_TRUE, GT_TRUE, packetBuf,
            &packetBufLen, &packetActualLength,
            &dev, &queueCpu, &rxParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet failed\n");

    /* check modified Vlan */
    UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_EXP_VLANID_CNS, packetBuf[15], "Egress VLAN ID");

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
* @internal prvTgfPpuGeneveProtectedWindowTest function
* @endinternal
*
* @brief  Write into protected window and check that it is not written.
*/
GT_VOID prvTgfPpuGeneveProtectedWindowTest
(
        GT_VOID
)
{
    GT_STATUS               rc;
    GT_U8                   dev = prvTgfDevNum;
    GT_PORT_NUM             sendPort = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    GT_PORT_NUM             recvPort = prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS];
    CPSS_INTERFACE_INFO_STC portInt;
    GT_U8                   packetBuf[256] = {0};
    GT_U32                  packetBufLen = sizeof(packetBuf);
    GT_U32                  packetActualLength = 0;
    GT_U8                   queueCpu = 0;
    TGF_NET_DSA_STC         rxParams;
    GT_U32                  dauProfileIdx = prvTgfDauProfIdx;
    GT_U32                  protWinIdx = 4;
    CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC dauProfile;
    CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC origDauProfile;
    CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_STC protWin;
    CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_STC origProtWin;

    /* -------------------------------------------------
     * 1. Set protected window and write dau profile
     */

    /* store original protected window */
    rc = cpssDxChPpuDauProtectedWindowGet(prvTgfDevNum, protWinIdx, &origProtWin);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuDauProtectedWindowGet failed");

    cpssOsMemCpy(&protWin, &origProtWin, sizeof(protWin));

    protWin.protWinEnable      = GT_TRUE;
    protWin.protWinStartOffset = 2174;
    protWin.protWinEndOffset   = 2178;

    /* write new protected window */
    rc = cpssDxChPpuDauProtectedWindowSet(prvTgfDevNum, protWinIdx, &protWin);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuDauProtectedWindowSet failed");

    /* store original DAU profile table entry*/
    rc = cpssDxChPpuDauProfileEntryGet(prvTgfDevNum, dauProfileIdx, &origDauProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuDauProfileEntryGet failed");

    cpssOsMemCpy(&dauProfile, &origDauProfile, sizeof(dauProfile));

    /* write up0 field. SP Bus : 8th byte*/
    dauProfile.setDescBits[6].spByteWriteEnable  = GT_TRUE;
    dauProfile.setDescBits[6].spByteSrcOffset    = 3 * 8;   /* bit offset */
    dauProfile.setDescBits[6].spByteNumBits      = 2;       /* num of bits - 1 = 3 - 1 */
    dauProfile.setDescBits[6].spByteTargetOffset = 2175;     /* User priority byte bit offset */

    /* write entry to DAU profile table */
    rc = cpssDxChPpuDauProfileEntrySet(prvTgfDevNum, dauProfileIdx, &dauProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuDauProfileEntrySet failed");

    /* --------------------------------------------------
     * 2. Send packet
     */

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
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo6Tlv, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* AUTODOC: Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, sendPort);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n", prvTgfDevNum, sendPort);

    cpssOsTimerWkAfter(300);

    /* AUTODOC: stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* AUTODOC: restore port capture setting */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInt, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
            prvTgfDevNum, portInt.devPort.portNum);

    /* AUTODOC: capture packet by trapping to CPU*/
    rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&portInt,
            TGF_PACKET_TYPE_CAPTURE_E,
            GT_TRUE, GT_TRUE, packetBuf,
            &packetBufLen, &packetActualLength,
            &dev, &queueCpu, &rxParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet failed\n");

    /* Flush fdb */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfBrgFdbFlush");

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* -----------------------------------------------------
     * 3. compare result with expected values
     */

    /* check modified user priority */
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPacketPassengerVlanTagPart.pri, (packetBuf[14] >> 5), "User Priority");

    /* check modified Vlan */
    UTF_VERIFY_EQUAL0_STRING_MAC(0x3f, packetBuf[15], "Egress VLAN ID");

    /* ------------------------------------------------------
     * 5. Restore defaults
     */

    /* store original protected window */
    rc = cpssDxChPpuDauProtectedWindowSet(prvTgfDevNum, protWinIdx, &origProtWin);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuDauProtectedWindowSet failed");

    /* restore original DAU profile table entry*/
    rc = cpssDxChPpuDauProfileEntrySet(prvTgfDevNum, dauProfileIdx, &origDauProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuDauProfileEntrySet failed");
}


/**
* @internal prvTgfPpuGeneveLoopbackErrorTest function
* @endinternal
*
* @brief  Create loopback error and check error profile is used
*/
GT_VOID prvTgfPpuGeneveLoopbackErrorTest
(
        GT_VOID
)
{
    GT_STATUS               rc;
    GT_U8                   dev = prvTgfDevNum;
    GT_PORT_NUM             sendPort = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    GT_PORT_NUM             recvPort = prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS];
    CPSS_INTERFACE_INFO_STC portInt;
    GT_U8                   packetBuf[256] = {0};
    GT_U32                  packetBufLen = sizeof(packetBuf);
    GT_U32                  packetActualLength = 0;
    GT_U8                   queueCpu = 0;
    TGF_NET_DSA_STC         rxParams;
    CPSS_DXCH_PPU_GLOBAL_CONFIG_STC  globalConfig;

    /* -------------------------------------------------
     * 1. Set Max Loopback to 0 to catch loopback error
     */

    /* store original global config value */
    rc = cpssDxChPpuGlobalConfigGet(prvTgfDevNum, &prvTgfRestoreCfg.globalConfig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuGlobalConfigGet failed");

    cpssOsMemCpy(&globalConfig, &prvTgfRestoreCfg.globalConfig, sizeof(CPSS_DXCH_PPU_GLOBAL_CONFIG_STC));

    /* set max loopback and loopback error profile index */
    globalConfig.maxLoopBack = 0;
    globalConfig.errProfMaxLoopBack = prvTgfErrProfIdx;
    rc = cpssDxChPpuGlobalConfigSet(prvTgfDevNum, &globalConfig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuGlobalConfigSet failed");

    /* set error profile */
    prvTgfPpuGeneveErrorProfileConfigSet();

    /* --------------------------------------------------
     * 2. Send packet
     */

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
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo6Tlv, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* AUTODOC: Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, sendPort);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n", prvTgfDevNum, sendPort);

    cpssOsTimerWkAfter(300);

    /* AUTODOC: stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* AUTODOC: restore port capture setting */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInt, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
            prvTgfDevNum, portInt.devPort.portNum);

    /* AUTODOC: capture packet by trapping to CPU*/
    rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&portInt,
            TGF_PACKET_TYPE_CAPTURE_E,
            GT_TRUE, GT_TRUE, packetBuf,
            &packetBufLen, &packetActualLength,
            &dev, &queueCpu, &rxParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet failed\n");

    /* Flush fdb */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfBrgFdbFlush");

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* -----------------------------------------------------
     * 3. compare result with expected values
     */

    /* check modified user priority */
    UTF_VERIFY_EQUAL0_STRING_MAC(1, (packetBuf[14] >> 5), "User Priority");

    /* check non modified Vlan */
    UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_RECV_VLANID_CNS, packetBuf[15], "Egress VLAN ID");

    /* ------------------------------------------------------
     * 4. Restore defaults
     */

    /* restore original global config value */
    rc = cpssDxChPpuGlobalConfigSet(prvTgfDevNum, &prvTgfRestoreCfg.globalConfig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuGlobalConfigSet failed");

    prvTgfPpuGeneveErrorProfileConfigRestore();
}

/**
* @internal prvTgfPpuGeneveOorErrorTest function
* @endinternal
*
* @brief  Create out of range error and check error profile is used
*/
GT_VOID prvTgfPpuGeneveOorErrorTest
(
        GT_VOID
)
{
    GT_STATUS               rc;
    GT_U8                   dev = prvTgfDevNum;
    GT_PORT_NUM             sendPort = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    GT_PORT_NUM             recvPort = prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS];
    CPSS_INTERFACE_INFO_STC portInt;
    GT_U8                   packetBuf[256] = {0};
    GT_U32                  packetBufLen = sizeof(packetBuf);
    GT_U32                  packetActualLength = 0;
    GT_U8                   queueCpu = 0;
    TGF_NET_DSA_STC         rxParams;
    CPSS_DXCH_PPU_GLOBAL_CONFIG_STC  globalConfig;
    GT_U32                  ruleIndex = prvTgfTtiRuleIndex;
    GT_U32                  entryNumber = 0;
    GT_U32                  dummy;
    GT_U32                  hwTtiActionArray[8];  /* TTI_ACTION_TYPE_2_SIZE_CNS = 8 */
    GT_U32                  origHwTtiActionArray[8];  /* TTI_ACTION_TYPE_2_SIZE_CNS = 8 */
    CPSS_DXCH_PPU_PROFILE_ENTRY_STC profileEntry;
    CPSS_DXCH_PPU_PROFILE_ENTRY_STC origProfileEntry;

    /* -------------------------------------------------
     * 1. Set Orr error and proifile
     */

    /* store original global config value */
    rc = cpssDxChPpuGlobalConfigGet(prvTgfDevNum, &prvTgfRestoreCfg.globalConfig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuGlobalConfigGet failed");

    cpssOsMemCpy(&globalConfig, &prvTgfRestoreCfg.globalConfig, sizeof(CPSS_DXCH_PPU_GLOBAL_CONFIG_STC));

    /* set out of range error profile index */
    globalConfig.errProfOffsetOor = prvTgfErrProfIdx;
    rc = cpssDxChPpuGlobalConfigSet(prvTgfDevNum, &globalConfig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuGlobalConfigSet failed");

    /* set error profile */
    prvTgfPpuGeneveErrorProfileConfigSet();

    /* -------------------------------------------------
     * 2. Write new PPU profile index to TTI action table
     */

    /* convert rule global index to TCAM entry number */
    rc = prvCpssDxChTcamRuleIndexToEntryNumber(prvTgfDevNum, ruleIndex, &entryNumber, &dummy);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssDxChTcamRuleIndexToEntryNumber failed");

    /* Write ppu profile to TTI action entry */
    rc = prvCpssDxChReadTableEntry(prvTgfDevNum, CPSS_DXCH_SIP5_TABLE_TCAM_PCL_TTI_ACTION_E,
                        (entryNumber/2), origHwTtiActionArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssDxChReadTableEntry failed");

    cpssOsMemCpy(hwTtiActionArray, origHwTtiActionArray, sizeof(hwTtiActionArray));

    rc = prvCpssFieldValueSet(hwTtiActionArray, 250, 4, prvTgfPpuProfileIdx2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssFieldValueSet failed");

    rc = prvCpssDxChWriteTableEntry(prvTgfDevNum, CPSS_DXCH_SIP5_TABLE_TCAM_PCL_TTI_ACTION_E,
                        (entryNumber/2), hwTtiActionArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssDxChWriteTableEntry failed");

    /* ------------------------------------------
     * 3. Set PPU profile data to PPU profile table
     */

    /* save original PPU profile table entry*/
    rc = cpssDxChPpuProfileGet(prvTgfDevNum, prvTgfPpuProfileIdx2, &origProfileEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuProfileGet failed");

    /* set PPU Profile entry */
    profileEntry.ppuEnable           = GT_TRUE;
    profileEntry.anchorType          = CPSS_DXCH_PPU_PROFILE_ENTRY_ANCHOR_TYPE_TUNNEL_L4_E;
    profileEntry.spBusDefaultProfile = prvTgfSpBusProfileIdx;
    profileEntry.offset              = 0x7E; /* 2 byte granularity */
    profileEntry.ppuState            = prvTgfPpuState;

    /* write entry to PPU profile table */
    rc = cpssDxChPpuProfileSet(prvTgfDevNum, prvTgfPpuProfileIdx2, &profileEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuProfileSet failed");

    /* --------------------------------------------------
     * 4. Send packet
     */

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
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo6Tlv, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* AUTODOC: Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, sendPort);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n", prvTgfDevNum, sendPort);

    cpssOsTimerWkAfter(300);

    /* AUTODOC: stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* AUTODOC: restore port capture setting */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInt, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
            prvTgfDevNum, portInt.devPort.portNum);

    /* AUTODOC: capture packet by trapping to CPU*/
    rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&portInt,
            TGF_PACKET_TYPE_CAPTURE_E,
            GT_TRUE, GT_TRUE, packetBuf,
            &packetBufLen, &packetActualLength,
            &dev, &queueCpu, &rxParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet failed\n");

    /* Flush fdb */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfBrgFdbFlush");

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* -----------------------------------------------------
     * 5. compare result with expected values
     */

    /* check modified user priority */
    UTF_VERIFY_EQUAL0_STRING_MAC(1, (packetBuf[14] >> 5), "User Priority");

    /* check non modified vlan */
    UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_RECV_VLANID_CNS, packetBuf[15], "Egress VLAN ID");

    /* ------------------------------------------------------
     * 6. Restore defaults
     */

    /* restore original global config value */
    rc = cpssDxChPpuGlobalConfigSet(prvTgfDevNum, &prvTgfRestoreCfg.globalConfig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuGlobalConfigSet failed");

    /* restore original PPU profile table entry*/
    rc = cpssDxChPpuProfileSet(prvTgfDevNum, prvTgfPpuProfileIdx2, &origProfileEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuProfileSet failed");

    /* restore original ppu profile index to TTI action entry */
    rc = prvCpssDxChWriteTableEntry(prvTgfDevNum, CPSS_DXCH_SIP5_TABLE_TCAM_PCL_TTI_ACTION_E,
                        (entryNumber/2), origHwTtiActionArray);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssDxChReadTableEntry failed");

    prvTgfPpuGeneveErrorProfileConfigRestore();
}


/**
* @internal prvTgfPpuGeneveDebugCountersTest function
* @endinternal
*
* @brief  Enable debug counters
*         Send packet
*         Read and verify debug counters
*/
GT_VOID prvTgfPpuGeneveDebugCountersTest
(
        GT_VOID
)
{
    GT_STATUS               rc;
    GT_U8                   dev = prvTgfDevNum;
    GT_PORT_NUM             sendPort = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    GT_PORT_NUM             recvPort = prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS];
    CPSS_DXCH_PPU_GLOBAL_CONFIG_STC  globalConfig;
    CPSS_DXCH_PPU_DEBUG_COUNTERS_STC dbgCounters;
    CPSS_INTERFACE_INFO_STC portInt;
    GT_U8                   packetBuf[256] = {0};
    GT_U32                  packetBufLen = sizeof(packetBuf);
    GT_U32                  packetActualLength = 0;
    GT_U8                   queueCpu = 0;
    TGF_NET_DSA_STC         rxParams;
    GT_U32                  i;
    GT_U32                  numPkts = 3;

    /* -------------------------------------------------
     * 1. Enable debug counters
     */

    /* store original global config value */
    rc = cpssDxChPpuGlobalConfigGet(prvTgfDevNum, &prvTgfRestoreCfg.globalConfig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuGlobalConfigGet failed");

    cpssOsMemCpy(&globalConfig, &prvTgfRestoreCfg.globalConfig, sizeof(CPSS_DXCH_PPU_GLOBAL_CONFIG_STC));

    /* enable debug counters */
    globalConfig.debugCounterEnable = GT_TRUE;
    rc = cpssDxChPpuGlobalConfigSet(prvTgfDevNum, &globalConfig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuGlobalConfigSet failed");

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
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo6Tlv, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    for (i=0; i<numPkts; i++) {
        /* --------------------------------------------------
         * 2. Send packet
         */
        /* AUTODOC: Transmit packet */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, sendPort);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n", prvTgfDevNum, sendPort);

        cpssOsTimerWkAfter(300);

        /* AUTODOC: capture packet by trapping to CPU*/
        rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&portInt,
                TGF_PACKET_TYPE_CAPTURE_E,
                GT_TRUE, GT_TRUE, packetBuf,
                &packetBufLen, &packetActualLength,
                &dev, &queueCpu, &rxParams);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet failed\n");

        /* check modified Vlan */
        UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_EXP_VLANID_CNS, packetBuf[15], "Egress VLAN ID");

        /* -----------------------------------------------------
         * 3. compare result with expected values
         */

        /* get debug counters */
        rc = cpssDxChPpuDebugCountersGet(prvTgfDevNum, &dbgCounters);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuDebugCountersGet failed");

        UTF_VERIFY_EQUAL0_STRING_MAC(0x3, dbgCounters.counterIn, "Debug counter In");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x3, dbgCounters.counterOut, "Debug counter Out");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x4, dbgCounters.counterKstg0, "Debug counter K_stg 0");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x4, dbgCounters.counterKstg1, "Debug counter K_stg 1");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x4, dbgCounters.counterKstg2, "Debug counter K_stg 2");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x3, dbgCounters.counterDau, "Debug counter DAU");
        UTF_VERIFY_EQUAL0_STRING_MAC(0x1, dbgCounters.counterLoopback, "Debug counter Loopback");
    }

    /* ------------------------------------------------------
     * 4. Restore defaults
     */

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

    /* restore original global config value */
    rc = cpssDxChPpuGlobalConfigSet(prvTgfDevNum, &prvTgfRestoreCfg.globalConfig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuGlobalConfigSet failed");

    rc = cpssDxChPpuDebugCountersClear(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuDebugCountersClear failed");
}

/**
* @internal prvTgfPpuGeneveLoopbackRestore function
* @endinternal
*
* @brief  Restore Configuration
*/
GT_VOID prvTgfPpuGeneveLoopbackRestore
(
        GT_VOID
)
{
    GT_STATUS                              rc;
    GT_U32                                 keyGenProfileIdx     = (prvTgfPpuState >> 4) & 0xf;
    GT_U32                                 kstgTcamProfileIdx   = 8;
    GT_U32                                 dauProfileIdx        = 12;
    GT_U32                                 kstgActionProfileIdx = kstgTcamProfileIdx;
    CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC keyGenProfile;
    CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC    ppuActionProfile;
    CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC      tcamEntry;
    CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC    dauProfile;
    GT_U32                                 i;

    /* -------------------------------------------------------------------------
     * Restore Port Configuration
     */

    /* Invalidate VLAN */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_RECV_VLANID_CNS);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, PRV_TGF_RECV_VLANID_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, PRV_TGF_SEND_VLANID_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_EXP_VLANID_CNS);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, PRV_TGF_EXP_VLANID_CNS);

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

    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);

    /* Restore default PPU Profile Idx of source port */
    rc = cpssDxChPpuSrcPortProfileIndexSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                         prvTgfRestoreCfg.ppuProfileIdx);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPpuSrcPortProfileIndexSet failed");

    /* Restore variable tunnel length Configuration */
    rc = cpssDxChTtiTunnelHeaderLengthProfileSet(prvTgfDevNum, prvTgfTtiProfileIdx, &prvTgfRestoreCfg.tunnelHeaderLengthProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTtiTunnelHeaderLengthProfileSet failed");


    /* Restore original user defined bytes */
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV4_OTHER_E, 0,
            prvTgfRestoreCfg.origUdbType[0], prvTgfRestoreCfg.origUdb[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet failed");

    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV4_OTHER_E, 1,
            prvTgfRestoreCfg.origUdbType[1], prvTgfRestoreCfg.origUdb[1]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet failed");

    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV4_OTHER_E, 2,
            prvTgfRestoreCfg.origUdbType[2], prvTgfRestoreCfg.origUdb[2]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet failed");

    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV4_OTHER_E, 3,
            prvTgfRestoreCfg.origUdbType[3], prvTgfRestoreCfg.origUdb[3]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet failed");

    for (i=0; i<7; i++)
    {
        rc = cpssDxChPclUserDefinedByteSet(prvTgfDevNum,
                                          CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E,
                                          CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E,
                                          CPSS_PCL_DIRECTION_INGRESS_E,
                                          30,
                                          prvTgfRestoreCfg.pclUdbOffsetType[i],
                                          prvTgfRestoreCfg.pclUdbOffset[i]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPclUserDefinedByteSet failed");
    }

    /* -------------------------------------------------------------------------
     * Restore PPU Configuration
     */

    /* restore ppu enable in PPU unit */
    rc = cpssDxChPpuEnableSet(prvTgfDevNum, prvTgfRestoreCfg.ppuEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuEnableSet failed");

    /* restore profile entry */
    rc = cpssDxChPpuProfileSet(prvTgfDevNum, prvTgfPpuProfileIdx, &prvTgfRestoreCfg.profileEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuProfileSet failed");

    /* restore SP bus profile entry*/
    rc = cpssDxChPpuSpBusDefaultProfileSet(prvTgfDevNum, prvTgfSpBusProfileIdx, &prvTgfRestoreCfg.spBusProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuSpBusDefaultProfileSet failed");

    /* restore key generation profiles */
    cpssOsMemSet(&keyGenProfile, 0, sizeof(keyGenProfile));
    rc = cpssDxChPpuKstgKeyGenProfileSet(prvTgfDevNum, 0 /*kstgNum*/, keyGenProfileIdx, &keyGenProfile);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgKeyGenProfileSet K_stg=%d failed", 0);
    rc = cpssDxChPpuKstgKeyGenProfileSet(prvTgfDevNum, 1 /*kstgNum*/, keyGenProfileIdx, &keyGenProfile);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgKeyGenProfileSet K_stg=%d failed", 1);
    rc = cpssDxChPpuKstgKeyGenProfileSet(prvTgfDevNum, 2 /*kstgNum*/, keyGenProfileIdx, &keyGenProfile);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgKeyGenProfileSet K_stg=%d failed", 2);

    /* restore tcam profiles */
    cpssOsMemSet(&tcamEntry, 0, sizeof(tcamEntry));
    rc = cpssDxChPpuKstgTcamEntrySet(prvTgfDevNum, 0 /*kstgNum*/, kstgTcamProfileIdx, &tcamEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgTcamEntrySet K_stg=%d failed", 0);
    rc = cpssDxChPpuKstgTcamEntrySet(prvTgfDevNum, 0 /*kstgNum*/, kstgTcamProfileIdx + 1, &tcamEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgTcamEntrySet K_stg=%d failed", 0);
    rc = cpssDxChPpuKstgTcamEntrySet(prvTgfDevNum, 1 /*kstgNum*/, kstgTcamProfileIdx, &tcamEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgTcamEntrySet K_stg=%d failed", 1);
    rc = cpssDxChPpuKstgTcamEntrySet(prvTgfDevNum, 1 /*kstgNum*/, kstgTcamProfileIdx + 1, &tcamEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgTcamEntrySet K_stg=%d failed", 1);
    rc = cpssDxChPpuKstgTcamEntrySet(prvTgfDevNum, 2 /*kstgNum*/, kstgTcamProfileIdx, &tcamEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgTcamEntrySet K_stg=%d failed", 2);
    rc = cpssDxChPpuKstgTcamEntrySet(prvTgfDevNum, 2 /*kstgNum*/, kstgTcamProfileIdx + 1, &tcamEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgTcamEntrySet K_stg=%d failed", 2);

    /* restore ppu action profiles */
    cpssOsMemSet(&ppuActionProfile, 0, sizeof(ppuActionProfile));
    rc = cpssDxChPpuActionTableEntrySet(prvTgfDevNum, 0 /*kstgNum*/, kstgActionProfileIdx, &ppuActionProfile);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"cpssDxChPpuActionTableEntrySet K_stg=%d failed", 0);
    rc = cpssDxChPpuActionTableEntrySet(prvTgfDevNum, 0 /*kstgNum*/, kstgActionProfileIdx + 1, &ppuActionProfile);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"cpssDxChPpuActionTableEntrySet K_stg=%d failed", 0);
    rc = cpssDxChPpuActionTableEntrySet(prvTgfDevNum, 1 /*kstgNum*/, kstgActionProfileIdx, &ppuActionProfile);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"cpssDxChPpuActionTableEntrySet K_stg=%d failed", 1);
    rc = cpssDxChPpuActionTableEntrySet(prvTgfDevNum, 1 /*kstgNum*/, kstgActionProfileIdx + 1, &ppuActionProfile);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"cpssDxChPpuActionTableEntrySet K_stg=%d failed", 1);
    rc = cpssDxChPpuActionTableEntrySet(prvTgfDevNum, 2 /*kstgNum*/, kstgActionProfileIdx, &ppuActionProfile);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"cpssDxChPpuActionTableEntrySet K_stg=%d failed", 2);
    rc = cpssDxChPpuActionTableEntrySet(prvTgfDevNum, 2 /*kstgNum*/, kstgActionProfileIdx + 1, &ppuActionProfile);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"cpssDxChPpuActionTableEntrySet K_stg=%d failed", 2);

    /* restore dau profile */
    cpssOsMemSet(&dauProfile, 0, sizeof(dauProfile));
    rc = cpssDxChPpuDauProfileEntrySet(prvTgfDevNum, dauProfileIdx, &dauProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuDauProfileEntrySet failed");
}
