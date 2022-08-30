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
* @file prvTgfPpuGeneve.c
*
* @brief Programmable Parser Unit (PPU). Geneve Usecase.
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
#define PRV_TGF_NEXTHOP_VLANID_CNS       6

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
static GT_U32 prvTgfPpuProfileIdx = 2;

/* ppu profile index for TTI overwrite */
static GT_U32 prvTgfTtiPpuProfileIdx = 5;

/* default ppu State */
static GT_U32 prvTgfPpuState = 0x41;

/* Tunnel header length profile table index */
static GT_U32 prvTgfTtiHeaderLengthProfileIdx = 3;

/* SP Bus profile index */
static GT_U32 prvTgfSpBusProfileIdx0 = 1;
/* SP Bus profile index */
static GT_U32 prvTgfSpBusProfileIdx1 = 2;

static GT_BOOL prvTgfRestoreTunnelCfg = GT_FALSE;

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
    0x05,               /* timeToLive */
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
    0x07,               /* timeToLive */
    0xff,               /* ipv4 protocol */
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
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketPassengerVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketPassengerEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketPassengerIpv4Part},
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
    + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* normal network PACKET with 1 TLV to send */
static TGF_PACKET_STC prvTgfPacketInfo1Tlv =
{
    PRV_TGF_GENEVE_PACKET_1TLV_LEN_CNS,                              /* totalLen */
    sizeof(prvTgfPacketPartArray1Tlv) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray1Tlv                                        /* partsArray */
};

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_TTI_KEY_SIZE_ENT                        keySizeOrig;
    CPSS_DXCH_TTI_TUNNEL_HEADER_LENGTH_PROFILE_STC  tunnelHeaderLengthProfile;
    CPSS_DXCH_PPU_PROFILE_ENTRY_STC                 ppuProfileEntry;
    CPSS_DXCH_PPU_PROFILE_ENTRY_STC                 ttiPpuProfileEntry;
    CPSS_DXCH_PPU_SP_BUS_STC                        spBusProfile0;
    CPSS_DXCH_PPU_SP_BUS_STC                        spBusProfile1;
    GT_BOOL                                         ppuEnable;
    GT_U32                                          ppuProfileIdx;
    PRV_TGF_TTI_OFFSET_TYPE_ENT                     origUdbType[4];
    GT_U8                                           origUdb[4];
    CPSS_DXCH_PCL_OFFSET_TYPE_ENT                   pclUdbOffsetType;
    GT_U8                                           pclUdbOffset;
} prvTgfRestoreCfg;

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
 * @internal prvTgfPpuGeneveBasicConfigSet function
 * @endinternal
 *
 * @brief    Set basic port vlan configuration
 *
 */
GT_VOID prvTgfPpuGeneveBasicConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U8       tagArray[] = {1, 1, 1, 1};
    GT_U16      tempVlanId = 0;

    /* AUTODOC: create send VLAN 5*/
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS,
                                           prvTgfPortsArray, NULL, tagArray, 2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet failed");

    /* AUTODOC: create nexthop VLAN 6*/
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_NEXTHOP_VLANID_CNS,
                                           prvTgfPortsArray + 2, NULL, tagArray + 2, 2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet failed");

    /* AUTODOC: create nethop Vlan */
    tempVlanId = ((prvTgfPacketIpv4Part.timeToLive & 0xf) << 4) | (PRV_TGF_NEXTHOP_VLANID_CNS & 0xf);
    rc = prvTgfBrgDefVlanEntryWithPortsSet(tempVlanId,
                                           prvTgfPortsArray + 2, NULL, tagArray + 2, 2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet failed");

    /* AUTODOC: create nethop Vlan */
    tempVlanId = ((prvTgfPacketPassengerIpv4Part.timeToLive & 0xf) << 4) | (PRV_TGF_NEXTHOP_VLANID_CNS & 0xf);
    rc = prvTgfBrgDefVlanEntryWithPortsSet(tempVlanId,
                                           prvTgfPortsArray + 2, NULL, tagArray + 2, 2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet failed");
}

/**
 * @internal prvTgfPpuGeneveVariableTunnelLengthConfigSet function
 * @endinternal
 *
 * @brief    Set variable tunnel length configuration
 *
 */
GT_VOID prvTgfPpuGeneveVariableTunnelLengthConfigSet
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
    rc = cpssDxChTtiTunnelHeaderLengthProfileGet(prvTgfDevNum, prvTgfTtiHeaderLengthProfileIdx, &prvTgfRestoreCfg.tunnelHeaderLengthProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTtiTunnelHeaderLengthProfileGet failed");

    /* populate and set tunnel header length profile table entry*/
    tunnelHeaderLengthProfile.lengthFieldAnchorType = GT_TRUE;
    tunnelHeaderLengthProfile.lengthFieldUdbLsb = udbLsb&0xF;
    tunnelHeaderLengthProfile.lengthFieldUdbMsb = udbMsb&0xF;
    tunnelHeaderLengthProfile.lengthFieldLsBit = 2;
    tunnelHeaderLengthProfile.lengthFieldSize = 6;
    tunnelHeaderLengthProfile.constant = 16;
    tunnelHeaderLengthProfile.multiplier = 2;

    rc = cpssDxChTtiTunnelHeaderLengthProfileSet(prvTgfDevNum, prvTgfTtiHeaderLengthProfileIdx, &tunnelHeaderLengthProfile);
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
 * @internal prvTgfPpuGeneveTtiConfigSet function
 * @endinternal
 *
 * @brief    Set TTI configuration for tunnel terminating IPV4 UDP packet.
 *
 */
GT_VOID prvTgfPpuGeneveTtiConfigSet
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
    ttiAction.ttHeaderLength                    = prvTgfTtiHeaderLengthProfileIdx;

    /* write tcam rule to tcan table */
    rc = prvTgfTtiRule2Set(ruleIndex, PRV_TGF_TTI_KEY_UDB_IPV4_OTHER_E,
                              &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");

    rc = prvCpssDxChPpuTtiActionProfileIndexSet(prvTgfDevNum, ruleIndex, prvTgfTtiPpuProfileIdx);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssDxChPpuTtiActionProfileIndexSet failed");
}

/**
 * @internal prvTgfPpuGenevePpuInitConfigSet function
 * @endinternal
 *
 * @brief    Set PPU init configuration
 *           - Enable PPU in TTI global configuration ext2 and PPU unit.
 *           - Set default PPU profile index to ingress port
 *           - Configure PPU profile at default PPU profile index
 *
 */
static GT_VOID prvTgfPpuGenevePpuInitConfigSet
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
    rc = cpssDxChPpuSrcPortProfileIndexGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                         &prvTgfRestoreCfg.ppuProfileIdx);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssDxChPpuSrcPortProfileIndexGet failed");

    /* set default PPU Profile Idx of source port */
    rc = cpssDxChPpuSrcPortProfileIndexSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                         prvTgfPpuProfileIdx);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPpuSrcPortProfileIndexSet failed");

    /* ------------------------------------------
     * 3. Set PPU profile data to PPU profile table
     */

    /* save original PPU profile table entry*/
    rc = cpssDxChPpuProfileGet(prvTgfDevNum, prvTgfPpuProfileIdx, &prvTgfRestoreCfg.ppuProfileEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuProfileSet failed");

    /* set PPU Profile entry */
    profileEntry.ppuEnable           = GT_TRUE;
    profileEntry.anchorType          = CPSS_DXCH_PPU_PROFILE_ENTRY_ANCHOR_TYPE_L4_E;
    profileEntry.spBusDefaultProfile = prvTgfSpBusProfileIdx0;
    profileEntry.offset              = 4; /* 2 byte granularity */
    profileEntry.ppuState            = prvTgfPpuState;

    /* write entry to PPU profile table */
    rc = cpssDxChPpuProfileSet(prvTgfDevNum, prvTgfPpuProfileIdx, &profileEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuProfileSet failed");

    /* save original PPU profile table entry*/
    rc = cpssDxChPpuProfileGet(prvTgfDevNum, prvTgfTtiPpuProfileIdx, &prvTgfRestoreCfg.ttiPpuProfileEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuProfileSet failed");

    /* set PPU Profile entry */
    profileEntry.ppuEnable           = GT_TRUE;
    profileEntry.anchorType          = CPSS_DXCH_PPU_PROFILE_ENTRY_ANCHOR_TYPE_TUNNEL_L4_E;
    profileEntry.spBusDefaultProfile = prvTgfSpBusProfileIdx1;
    profileEntry.offset              = 4; /* 2 byte granularity */
    profileEntry.ppuState            = prvTgfPpuState;

    /* write entry to PPU profile table */
    rc = cpssDxChPpuProfileSet(prvTgfDevNum, prvTgfTtiPpuProfileIdx, &profileEntry);
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

    /* save original spBusProfile */
    rc = cpssDxChPpuSpBusDefaultProfileGet(prvTgfDevNum, prvTgfSpBusProfileIdx1, &prvTgfRestoreCfg.spBusProfile1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuSpBusDefaultProfileGet failed");

    cpssOsMemSet(&spBusProfile, 0, sizeof(spBusProfile));

    /* write SP Bus profile entry */
    spBusProfile.data[16] = 2; /* up0 */
    spBusProfile.data[17] = 4; /* up1 */
    rc = cpssDxChPpuSpBusDefaultProfileSet(prvTgfDevNum, prvTgfSpBusProfileIdx1, &spBusProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuSpBusDefaultProfileSet failed");

    /* ------------------------------------------
    * 5. Set UDB Metadata
    */
    /* store PCL udb */
    rc = cpssDxChPclUserDefinedByteGet(prvTgfDevNum,
                                       CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E,
                                       CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E,
                                       CPSS_PCL_DIRECTION_INGRESS_E,
                                       34,
                                       &prvTgfRestoreCfg.pclUdbOffsetType,
                                       &prvTgfRestoreCfg.pclUdbOffset);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPclUserDefinedByteGet failed");

    /* TTL */
    rc = cpssDxChPclUserDefinedByteSet(prvTgfDevNum,
                                       CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E,
                                       CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E,
                                       CPSS_PCL_DIRECTION_INGRESS_E,
                                       34,
                                       CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                       166);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPclUserDefinedByteSet failed");
}

#define OFFSET_MSB2LSB(x) ((32 - x / 8 - 1) * 8 + ((x) % 8))

/**
 * @internal prvTgfPpuGenevePpuKstg0ConfigSet function
 * @endinternal
 *
 * @brief    Set PPU K_stg 0 configuration
 *           - Set K_stg key generation profile.
 *           - Set tti key and mask to K_stg tcam table
 *           - Populate and write ppu Action Entry
 *
 */
static GT_VOID prvTgfPpuGenevePpuKstg0ConfigSet
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

    /* set key generation profile */

    keyGenProfile.byteSelOffset[0] = 61; /* 32; */  /* Header byte 0 */
    keyGenProfile.byteSelOffset[1] = 59; /* 30; */  /* Header byte 2 */
    keyGenProfile.byteSelOffset[2] = 58; /* 29; */  /* Header byte 3 */
    keyGenProfile.byteSelOffset[3] = 53; /* 24; */  /* Header byte 8 */
    keyGenProfile.byteSelOffset[4] = 52; /* 23; */  /* Header byte 9 */
    keyGenProfile.byteSelOffset[5] = 51; /* 22; */  /* Header byte 10 */
    keyGenProfile.byteSelOffset[6] = 50; /* 21; */  /* Header byte 11 */

    rc = cpssDxChPpuKstgKeyGenProfileSet(prvTgfDevNum, kstgNum, keyGenProfileIdx, &keyGenProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgKeyGenProfileSet failed");

    /* set kstg tcam profile */
    tcamEntry.isValid = 1;
    tcamEntry.keyLsb  = 0x11000808;
    tcamEntry.keyMsb  = 0x000811ab;
    tcamEntry.maskLsb = 0x00000000;
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
    ppuActionProfile.rotActionEntry[0].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_SUB_REG_E;
    ppuActionProfile.rotActionEntry[0].funcSecondOperand = CPSS_DXCH_PPU_KSTG_ROT_ACTION_SECOND_OPERAND_REG1_E;
    ppuActionProfile.rotActionEntry[0].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[0].ldSpBusOffset     = 1; /* byte offset */
    ppuActionProfile.rotActionEntry[0].ldSpBusNumBytes   = 1; /* number of bytes */

    /* Extract TLV<Length> to REG1 and add 1 */
    ppuActionProfile.rotActionEntry[1].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_HD_E;
    ppuActionProfile.rotActionEntry[1].srcOffset         = OFFSET_MSB2LSB(91);
    ppuActionProfile.rotActionEntry[1].srcNumValBits     = 4; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[1].addSubConstSel    = CPSS_DXCH_PPU_KSTG_ROT_ADD_SEL_E;
    ppuActionProfile.rotActionEntry[1].addSubConst       = 1;
    ppuActionProfile.rotActionEntry[1].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_NONE_E;
    ppuActionProfile.rotActionEntry[1].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[1].ldSpBusOffset     = 0; /* byte offset */
    ppuActionProfile.rotActionEntry[1].ldSpBusNumBytes   = 0; /* number of bytes */

    /* Extract 32bit TLV value */
    ppuActionProfile.rotActionEntry[2].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_HD_E;
    ppuActionProfile.rotActionEntry[2].srcOffset         = OFFSET_MSB2LSB(120);
    ppuActionProfile.rotActionEntry[2].srcNumValBits     = 31; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[2].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_NONE_E;
    ppuActionProfile.rotActionEntry[2].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[2].ldSpBusOffset     = 2; /* byte offset */
    ppuActionProfile.rotActionEntry[2].ldSpBusNumBytes   = 4; /* number of bytes */

    /* Header shift for TLV2*/
    ppuActionProfile.rotActionEntry[3].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_HD_E;
    ppuActionProfile.rotActionEntry[3].srcOffset         = OFFSET_MSB2LSB(91);
    ppuActionProfile.rotActionEntry[3].srcNumValBits     = 4; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[3].shiftLeftRightSel = CPSS_DXCH_PPU_KSTG_ROT_SHIFT_LEFT_E;
    ppuActionProfile.rotActionEntry[3].shiftNum          = 2;
    ppuActionProfile.rotActionEntry[3].addSubConstSel    = CPSS_DXCH_PPU_KSTG_ROT_ADD_SEL_E;
    ppuActionProfile.rotActionEntry[3].addSubConst       = 4;
    ppuActionProfile.rotActionEntry[3].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_NONE_E;
    ppuActionProfile.rotActionEntry[3].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_HD_SHIFT_E;

    ppuActionProfile.rotActionEntry[4].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_4BYTE_SET_CMD_E;
    ppuActionProfile.rotActionEntry[4].setCmd4Byte       = PRV_TGF_NEXTHOP_VLANID_CNS; /* new vlan ID */
    ppuActionProfile.rotActionEntry[4].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[4].ldSpBusOffset     = 6; /* byte offset */
    ppuActionProfile.rotActionEntry[4].ldSpBusNumBytes   = 2; /* number of bytes */

    ppuActionProfile.setNextState = keyGenProfileIdx << 4;

    /* Write PPU action table entry */
    rc = cpssDxChPpuActionTableEntrySet(prvTgfDevNum,
            kstgNum, kstgActionProfileIdx, &ppuActionProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuActionTableEntrySet failed");

}

/**
 * @internal prvTgfPpuGenevePpuKstg1ConfigSet function
 * @endinternal
 *
 * @brief    Set PPU K_stg 1 configuration
 *           - Set K_stg key generation profile.
 *           - Set tti key and mask to K_stg tcam table
 *           - Populate and write ppu Action Entry
 *
 */
static GT_VOID prvTgfPpuGenevePpuKstg1ConfigSet
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

    keyGenProfile.byteSelOffset[0] = 57; /* Header byte 4  */
    keyGenProfile.byteSelOffset[1] = 56; /* Header byte 5  */
    keyGenProfile.byteSelOffset[2] = 51; /* Header byte 10 */
    keyGenProfile.byteSelOffset[3] = 50; /* Header byte 11 */

    rc = cpssDxChPpuKstgKeyGenProfileSet(prvTgfDevNum, kstgNum, keyGenProfileIdx, &keyGenProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgKeyGenProfileSet failed");

    /* set kstg tcam profile */
    tcamEntry.isValid = 1;
    tcamEntry.keyLsb  = 0x55667788;
    tcamEntry.maskLsb = 0x00000000;
    tcamEntry.maskMsb = 0xFFFFFFFF;
    rc = cpssDxChPpuKstgTcamEntrySet(prvTgfDevNum,
            kstgNum, kstgTcamProfileIdx, &tcamEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgTcamEntrySet failed");

    ppuActionProfile.rotActionEntry[0].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_UDB_METADATA_E;
    ppuActionProfile.rotActionEntry[0].srcOffset         = 4 * 8;
    ppuActionProfile.rotActionEntry[0].srcNumValBits     = 7; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[0].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_NONE_E;
    ppuActionProfile.rotActionEntry[0].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[0].ldSpBusOffset     = 9; /* byte offset */
    ppuActionProfile.rotActionEntry[0].ldSpBusNumBytes   = 1; /* number of bytes */

    /* set kstg next state */
    ppuActionProfile.setNextState = keyGenProfileIdx << 4;

    /* Write PPU action table entry */
    rc = cpssDxChPpuActionTableEntrySet(prvTgfDevNum,
            kstgNum, kstgActionProfileIdx, &ppuActionProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuActionTableEntrySet failed");
}

/**
 * @internal prvTgfPpuGenevePpuKstg2ConfigSet function
 * @endinternal
 *
 * @brief    Set PPU K_stg 2 configuration
 *           - Set K_stg key generation profile.
 *           - Set tti key and mask to K_stg tcam table
 *           - Populate and write ppu Action Entry
 *
 */
static GT_VOID prvTgfPpuGenevePpuKstg2ConfigSet
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
    CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC keyGenProfile;
    CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC    ppuActionProfile;
    CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC      tcamEntry;

    /* initialize to ppuActionProfile to 0 */
    cpssOsMemSet(&ppuActionProfile, 0, sizeof(ppuActionProfile));
    cpssOsMemSet(&keyGenProfile, 0, sizeof(keyGenProfile));
    cpssOsMemSet(&tcamEntry, 0, sizeof(tcamEntry));

    /* set key generation profile */

    keyGenProfile.byteSelOffset[0] = 57; /* Header byte 4  */
    keyGenProfile.byteSelOffset[1] = 56; /* Header byte 5  */
    keyGenProfile.byteSelOffset[2] = 51; /* Header byte 10 */
    keyGenProfile.byteSelOffset[3] = 50; /* Header byte 11 */

    rc = cpssDxChPpuKstgKeyGenProfileSet(prvTgfDevNum, kstgNum, keyGenProfileIdx, &keyGenProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgKeyGenProfileSet failed");

    /* set kstg tcam profile */
    tcamEntry.isValid = 1;
    tcamEntry.keyLsb  = 0x55667788;
    tcamEntry.maskLsb = 0x00000000;
    tcamEntry.maskMsb = 0xFFFFFFFF;
    rc = cpssDxChPpuKstgTcamEntrySet(prvTgfDevNum,
            kstgNum, kstgTcamProfileIdx, &tcamEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgTcamEntrySet failed");

    ppuActionProfile.rotActionEntry[0].srcRegSel         = CPSS_DXCH_PPU_KSTG_ROT_SRC_REG_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[0].srcOffset         = 16 * 8;
    ppuActionProfile.rotActionEntry[0].srcNumValBits     = 2; /* numBits - 1 */
    ppuActionProfile.rotActionEntry[0].func              = CPSS_DXCH_PPU_KSTG_ROT_FUNC_NONE_E;
    ppuActionProfile.rotActionEntry[0].target            = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_SP_BUS_E;
    ppuActionProfile.rotActionEntry[0].ldSpBusOffset     = 8; /* byte offset */
    ppuActionProfile.rotActionEntry[0].ldSpBusNumBytes   = 1; /* number of bytes */

    /* set next state to DAU profile index */
    ppuActionProfile.setNextState = dauProfileIdx;

    /* Write PPU action table entry */
    rc = cpssDxChPpuActionTableEntrySet(prvTgfDevNum,
            kstgNum, kstgActionProfileIdx, &ppuActionProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuActionTableEntrySet failed");

}

/**
 * @internal prvTgfPpuGenevePpuDauConfigSet function
 * @endinternal
 *
 * @brief    Set PPU DAU profile configuration
 *           - Write first four bytes of MAC source address
 *
 */
static GT_VOID prvTgfPpuGenevePpuDauConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32      dauProfileIdx        = 12;
    CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC dauProfile;

    /* initialize DAU profile to 0 */
    cpssOsMemSet(&dauProfile, 0, sizeof(dauProfile));

    /* Populate DAU profile entry to write to first four bytes of MAC source address */
    dauProfile.setDescBits[0].spByteWriteEnable  = GT_TRUE;
    dauProfile.setDescBits[0].spByteSrcOffset    = 2 * 8;   /* bit offset */
    dauProfile.setDescBits[0].spByteNumBits      = 7;       /* num of bits - 1 = 8 - 1 */
    dauProfile.setDescBits[0].spByteTargetOffset = 1886;    /* MAC SA byte 1 bit offset */

    dauProfile.setDescBits[1].spByteWriteEnable  = GT_TRUE;
    dauProfile.setDescBits[1].spByteSrcOffset    = 3 * 8;   /* bit offset */
    dauProfile.setDescBits[1].spByteNumBits      = 7;       /* num of bits - 1 = 8 - 1 */
    dauProfile.setDescBits[1].spByteTargetOffset = 1894;    /* MAC SA byte 2 bit offset */

    dauProfile.setDescBits[2].spByteWriteEnable  = GT_TRUE;
    dauProfile.setDescBits[2].spByteSrcOffset    = 4 * 8;   /* bit offset */
    dauProfile.setDescBits[2].spByteNumBits      = 7;       /* num of bits - 1 = 8 - 1 */
    dauProfile.setDescBits[2].spByteTargetOffset = 1902;    /* MAC SA byte 3 bit offset */

    dauProfile.setDescBits[3].spByteWriteEnable  = GT_TRUE;
    dauProfile.setDescBits[3].spByteSrcOffset    = 5 * 8;   /* bit offset */
    dauProfile.setDescBits[3].spByteNumBits      = 7;       /* num of bits - 1 = 8 - 1 */
    dauProfile.setDescBits[3].spByteTargetOffset = 1910;    /* MAC SA byte 4 bit offset */

    /* write evlan */
    dauProfile.setDescBits[4].spByteWriteEnable  = GT_TRUE;
    dauProfile.setDescBits[4].spByteSrcOffset    = 6 * 8;   /* bit offset */
    dauProfile.setDescBits[4].spByteNumBits      = 3;       /* num of bits - 1 = 8 - 1 */
    dauProfile.setDescBits[4].spByteTargetOffset = 2179;    /* EVLAN  bit offset */

    /* write TTL */
    dauProfile.setDescBits[5].spByteWriteEnable  = GT_TRUE;
    dauProfile.setDescBits[5].spByteSrcOffset    = 8 * 8;   /* bit offset */
    dauProfile.setDescBits[5].spByteNumBits      = 7;       /* num of bits - 1 = 8 - 1 */
    dauProfile.setDescBits[5].spByteTargetOffset = 652;     /* TTL byte bit offset */

    /* write up0 field. SP Bus : 8th byte*/
    dauProfile.setDescBits[6].spByteWriteEnable  = GT_TRUE;
    dauProfile.setDescBits[6].spByteSrcOffset    = 8 * 8;   /* bit offset */
    dauProfile.setDescBits[6].spByteNumBits      = 2;       /* num of bits - 1 = 3 - 1 */
    dauProfile.setDescBits[6].spByteTargetOffset = 2175;     /* User priority byte bit offset */

    /* write evlan again*/
    dauProfile.setDescBits[7].spByteWriteEnable  = GT_TRUE;
    dauProfile.setDescBits[7].spByteSrcOffset    = 9 * 8;   /* bit offset */
    dauProfile.setDescBits[7].spByteNumBits      = 3;       /* num of bits - 1 = 8 - 1 */
    dauProfile.setDescBits[7].spByteTargetOffset = 2183;    /* EVLAN  bit offset */

    /* write mac byte to udb0*/
    dauProfile.setDescBits[8].spByteWriteEnable  = GT_TRUE;
    dauProfile.setDescBits[8].spByteSrcOffset    = 2 * 8;   /* bit offset */
    dauProfile.setDescBits[8].spByteNumBits      = 7;       /* num of bits - 1 = 8 - 1 */
    dauProfile.setDescBits[8].spByteTargetOffset = 867;    /* PCL UDB0  bit offset */

    /* write mac byte to udb1 */
    dauProfile.setDescBits[9].spByteWriteEnable  = GT_TRUE;
    dauProfile.setDescBits[9].spByteSrcOffset    = 3 * 8;   /* bit offset */
    dauProfile.setDescBits[9].spByteNumBits      = 7;       /* num of bits - 1 = 8 - 1 */
    dauProfile.setDescBits[9].spByteTargetOffset = 875;    /* PCL UDB1  bit offset */

    /* Set UDB Valid for udb0 */
    dauProfile.setDescBits[10].spByteWriteEnable  = GT_TRUE;
    dauProfile.setDescBits[10].spByteSrcOffset    = 8 * 8;   /* bit offset */
    dauProfile.setDescBits[10].spByteNumBits      = 0;       /* num of bits - 1 = 8 - 1 */
    dauProfile.setDescBits[10].spByteTargetOffset = 797;    /* UDB0 Valid  bit offset */

    /* Set UDB Valid for udb1 */
    dauProfile.setDescBits[11].spByteWriteEnable  = GT_TRUE;
    dauProfile.setDescBits[11].spByteSrcOffset    = 8 * 8;   /* bit offset */
    dauProfile.setDescBits[11].spByteNumBits      = 0;       /* num of bits - 1 = 8 - 1 */
    dauProfile.setDescBits[11].spByteTargetOffset = 798;    /* UDB1 Valid  bit offset */

    /* Set UDB Valid for udb51 */
    dauProfile.setDescBits[12].spByteWriteEnable  = GT_TRUE;
    dauProfile.setDescBits[12].spByteSrcOffset    = 8 * 8;   /* bit offset */
    dauProfile.setDescBits[12].spByteNumBits      = 0;       /* num of bits - 1 = 8 - 1 */
    dauProfile.setDescBits[12].spByteTargetOffset = 848;    /* UDB51 Valid bit offset */

    /* write mac byte to udb51 */
    dauProfile.setDescBits[13].spByteWriteEnable  = GT_TRUE;
    dauProfile.setDescBits[13].spByteSrcOffset    = 4 * 8;   /* bit offset */
    dauProfile.setDescBits[13].spByteNumBits      = 7;       /* num of bits - 1 = 8 - 1 */
    dauProfile.setDescBits[13].spByteTargetOffset = 1275;    /* UDB51  bit offset */

    /* write entry to DAU profile table */
    rc = cpssDxChPpuDauProfileEntrySet(prvTgfDevNum,
            dauProfileIdx, &dauProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuDauProfileEntrySet failed");

}

/**
* @internal prvTgfPpuGenevePpuConfigSet function
* @endinternal
*
* @brief  Set configuration for Geneve Packet
*           1. Initialize PPU
*           2. Configure K_stg0, K_stg1, K_stg2 and DAU
*/
GT_VOID prvTgfPpuGenevePpuConfigSet
(
        GT_VOID
)
{
    /* ppu init configuration set */
    prvTgfPpuGenevePpuInitConfigSet();

    /* PPU K_stg 0 configuration Set */
    prvTgfPpuGenevePpuKstg0ConfigSet();

    /* PPU K_stg 1 configuration Set */
    prvTgfPpuGenevePpuKstg1ConfigSet();

    /* PPU K_stg 2 configuration Set */
    prvTgfPpuGenevePpuKstg2ConfigSet();

    /* PPU DAU configuration Set */
    prvTgfPpuGenevePpuDauConfigSet();

}

/**
* @internal prvTgfPpuGeneveTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet and capture:
*         Check captured packet.
*/
GT_VOID prvTgfPpuGeneveTrafficGenerate
(
        GT_BOOL isTunnelTerminated
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
    GT_U16                  expectVlanId = 0;

    if (isTunnelTerminated)
    {
        prvTgfRestoreTunnelCfg = GT_TRUE;
    }

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
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo1Tlv, prvTgfBurstCount, 0, NULL);
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

    if (isTunnelTerminated)
    {
        expectVlanId = ((prvTgfPacketPassengerIpv4Part.timeToLive & 0xf) << 4) | (PRV_TGF_NEXTHOP_VLANID_CNS & 0xf);
        /* check modified Vlan */
        UTF_VERIFY_EQUAL0_STRING_MAC(expectVlanId, packetBuf[15], "Egress VLAN ID");
        /* check modified user priority */
        UTF_VERIFY_EQUAL0_STRING_MAC(2, (packetBuf[14] >> 5), "User Priority");
    }
    else
    {
        expectVlanId = ((prvTgfPacketIpv4Part.timeToLive & 0xf) << 4) | (PRV_TGF_NEXTHOP_VLANID_CNS & 0xf);
        /* check modified Vlan */
        UTF_VERIFY_EQUAL0_STRING_MAC(expectVlanId, packetBuf[15], "Egress VLAN ID");
        /* check modified user priority */
        UTF_VERIFY_EQUAL0_STRING_MAC(1, (packetBuf[14] >> 5), "User Priority");
    }

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
* @internal prvTgfPpuGeneveRestore function
* @endinternal
*
* @brief  Restore Configuration
*/
GT_VOID prvTgfPpuGeneveRestore
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
    GT_U16                                 tempVlanId;

    /* -------------------------------------------------------------------------
     * Restore Port Configuration
     */

    /* Invalidate VLAN */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_NEXTHOP_VLANID_CNS);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, PRV_TGF_NEXTHOP_VLANID_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, PRV_TGF_SEND_VLANID_CNS);

    /* AUTODOC: create nethop Vlan */
    tempVlanId = ((prvTgfPacketIpv4Part.timeToLive & 0xf) << 4) | (PRV_TGF_NEXTHOP_VLANID_CNS & 0xf);
    rc = prvTgfBrgDefVlanEntryInvalidate(tempVlanId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, tempVlanId);

    /* AUTODOC: create nethop Vlan */
    tempVlanId = ((prvTgfPacketPassengerIpv4Part.timeToLive & 0xf) << 4) | (PRV_TGF_NEXTHOP_VLANID_CNS & 0xf);
    rc = prvTgfBrgDefVlanEntryInvalidate(tempVlanId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, tempVlanId);


    /* -------------------------------------------------------------------------
     * Restore PPU Configuration
     */

    /* restore ppu enable in PPU unit */
    rc = cpssDxChPpuEnableSet(prvTgfDevNum, prvTgfRestoreCfg.ppuEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuEnableSet failed");

    /* restore profile entry */
    rc = cpssDxChPpuProfileSet(prvTgfDevNum, prvTgfPpuProfileIdx, &prvTgfRestoreCfg.ppuProfileEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuProfileSet failed");

        /* restore profile entry */
    rc = cpssDxChPpuProfileSet(prvTgfDevNum, prvTgfTtiPpuProfileIdx, &prvTgfRestoreCfg.ttiPpuProfileEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuProfileSet failed");

    /* restore SP bus profile entry 0*/
    rc = cpssDxChPpuSpBusDefaultProfileSet(prvTgfDevNum, prvTgfSpBusProfileIdx0, &prvTgfRestoreCfg.spBusProfile0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuSpBusDefaultProfileSet failed");

    /* restore SP bus profile entry 1*/
    rc = cpssDxChPpuSpBusDefaultProfileSet(prvTgfDevNum, prvTgfSpBusProfileIdx1, &prvTgfRestoreCfg.spBusProfile1);
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

    /* Restore PCL udb */
    rc = cpssDxChPclUserDefinedByteSet(prvTgfDevNum,
                                       CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E,
                                       CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E,
                                       CPSS_PCL_DIRECTION_INGRESS_E,
                                       34,
                                       prvTgfRestoreCfg.pclUdbOffsetType,
                                       prvTgfRestoreCfg.pclUdbOffset);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPclUserDefinedByteSet failed");

    if(prvTgfRestoreTunnelCfg == GT_TRUE)
    {
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
        rc = cpssDxChTtiTunnelHeaderLengthProfileSet(prvTgfDevNum, prvTgfTtiHeaderLengthProfileIdx, &prvTgfRestoreCfg.tunnelHeaderLengthProfile);
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

        prvTgfRestoreTunnelCfg = GT_FALSE;
    }

}
