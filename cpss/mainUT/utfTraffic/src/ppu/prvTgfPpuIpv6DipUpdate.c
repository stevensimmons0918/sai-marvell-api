/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfPpuIpv6DipUpdate.c
*
* DESCRIPTION:
*       Update Ipv6 DIP using Programmable Parser Unit (PPU)
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

#include <common/tgfTunnelGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>

#include <cpss/dxCh/dxChxGen/ppu/private/prvCpssDxChPpu.h>
#include <cpss/dxCh/dxChxGen/ppu/cpssDxChPpu.h>
#include <cpss/dxCh/dxChxGen/ppu/cpssDxChPpuTypes.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>
#include <trafficEngine/private/prvTgfTrafficParser.h>

#include <ppu/prvTgfPpuIpv6DipUpdate.h>

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           5

/* nextHop VLAN Id */
#define PRV_TGF_NEXTHOP_VLANID_CNS       6

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         0

#define PRV_TGF_FDB_PORT_IDX_CNS          2

/* port number to receive traffic to */
#define PRV_TGF_RECV_PORT_IDX_CNS          2

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = 1;

/* the TTI Rule index */
#define prvTgfTtiRuleIndex  ((UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum)) ? \
(prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(PRV_TTI_LOOKUP_0, 3)) : 1)

/* default ppu profile index */
static GT_U32 prvTgfPpuProfileIdx = 2;

/* default ppu State */
static GT_U32 prvTgfPpuState = 0x41;

/* SP Bus profile index */
static GT_U32 prvTgfSpBusProfileIdx = 1;

/* DAU profile index */
static GT_U32 prvTgfDauProfileIdx   = 12;

/* the Arp Address index of the Router ARP Table
   to write to the UC Route entry Arp nextHopARPPointer field */
static GT_U32        prvTgfRouterArpIndex      = 1;

/* the Route entry index for UC Route entry Table */
static GT_U32        prvTgfRouteEntryBaseIndex = 3;

/* the LPM DB id for LPM Table */
static GT_U32        prvTgfLpmDBId             = 0;

/* the ARP MAC address to write to the Router ARP Table */
static TGF_MAC_ADDR  prvTgfArpMac = {0x00, 0x00, 0x00, 0x00, 0x66, 0x55};


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
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_86DD_IPV6_TAG_CNS};

/* packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketIpv6Part = {
    6,                  /* version */
    0,                  /* trafficClass */
    0,                  /* flowLabel */
    0x02,               /* payloadLen */
    0x3b,               /* nextHeader */
    0x40,               /* hopLimit */
    {0x0B0A, 0, 0, 0, 0, 0, 0x0000, 0x0201}, /* TGF_IPV6_ADDR srcAddr */
    {0x1B1A, 0, 0, 0, 0, 0, 0x0000, 0x1211}  /* TGF_IPV6_ADDR dstAddr */
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


/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of unicast packet with 1 TLV */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of Geneve packet with 1 TLV*/
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_IPV6_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* normal network PACKET with 1 TLV to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                              /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_TTI_KEY_SIZE_ENT                        keySizeOrig;
    CPSS_DXCH_PPU_PROFILE_ENTRY_STC                 ppuProfileEntry;
    CPSS_DXCH_PPU_SP_BUS_STC                        spBusProfile;
    GT_BOOL                                         ppuEnable;
    GT_U32                                          ppuProfileIdx;
} prvTgfRestoreCfg;

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
 * @internal prvTgfPpuIpv6DipUpdateBaseConfigSet function
 * @endinternal
 *
 * @brief  set basic port and vlan configuration
*/
GT_VOID prvTgfPpuIpv6DipUpdateBaseConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U8       tagArray[] = {1, 1, 1, 1};

    /* AUTODOC: create send VLAN 5*/
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS,
                                           prvTgfPortsArray, NULL, tagArray, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet failed");

    /* AUTODOC: create nexthop VLAN 6*/
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_NEXTHOP_VLANID_CNS,
                                           prvTgfPortsArray + 2, NULL, tagArray + 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet failed");

    /* AUTODOC: create nexthop VLAN 6*/
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_NEXTHOP_VLANID_CNS + 1,
                                           prvTgfPortsArray + 2, NULL, tagArray + 2, 1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet failed");

}

/**
 * @internal prvTgfPpuIpv6DipUpdatePpuInitConfigSet function
 * @endinternal
 *
 * @brief    Set PPU init configuration
 *           - Enable PPU in TTI global configuration ext2 and PPU unit.
 *           - Set default PPU profile index to ingress port
 *           - Configure PPU profile at default PPU profile index
 *
 */
static GT_VOID prvTgfPpuIpv6DipUpdatePpuInitConfigSet
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
    profileEntry.anchorType          = CPSS_DXCH_PPU_PROFILE_ENTRY_ANCHOR_TYPE_L3_E;
    profileEntry.spBusDefaultProfile = prvTgfSpBusProfileIdx;
    /* Offset should point to SIP */
    profileEntry.offset              = 5; /* 2 byte granularity */
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

    spBusProfile.data[1] = 0xA1;
    spBusProfile.data[2] = 0xA2;

    /* write SP Bus profile entry */
    rc = cpssDxChPpuSpBusDefaultProfileSet(prvTgfDevNum, prvTgfSpBusProfileIdx, &spBusProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuSpBusDefaultProfileSet failed");

}

#define OFFSET_MSB2LSB(x) ((32 - x / 8 - 1) * 8 + ((x) % 8))

/**
 * @internal prvTgfPpuIpv6DipUpdatePpuKstg0ConfigSet function
 * @endinternal
 *
 * @brief    Set PPU K_stg 0 configuration
 *           - Set K_stg key generation profile.
 *           - Set tti key and mask to K_stg tcam table
 *           - Populate and write ppu Action Entry
 *
 */
static GT_VOID prvTgfPpuIpv6DipUpdatePpuKstgConfigSet
(
    GT_U32      kstgNum
)
{
    GT_STATUS   rc;
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

    /* count(1B) + Header(32B) + UDB Meatadata(9B) + SP_BUS(20B) + KeyState(1B) [MSB to LSB]*/
    /* Header (32B) : IPv6 DIP and IPv6 SIP. (LSB to MSB converted)
       11 12 00 00 00 00 00 00 00 00 00 00 00 00 1A 1B
       01 02 00 00 00 00 00 00 00 00 00 00 00 00 0A 0B */

    /* set key generation profile */
    keyGenProfile.byteSelOffset[0] = 30; /* Header byte 31 */
    keyGenProfile.byteSelOffset[1] = 31; /* Header byte 30 */
    keyGenProfile.byteSelOffset[2] = 44; /* Header byte 17 */
    keyGenProfile.byteSelOffset[3] = 45; /* Header byte 16 */

    rc = cpssDxChPpuKstgKeyGenProfileSet(prvTgfDevNum, kstgNum, keyGenProfileIdx, &keyGenProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgKeyGenProfileSet failed");

    /* set kstg tcam profile */
    tcamEntry.isValid = 1;
    tcamEntry.keyLsb  = 0x1B1A1211;
    tcamEntry.keyMsb  = 0x00000000;
    tcamEntry.maskLsb = 0x00000000; /* Valid mask */
    tcamEntry.maskMsb = 0xFFFFFFFF; /* Invalid mask */
    rc = cpssDxChPpuKstgTcamEntrySet(prvTgfDevNum,
            kstgNum, kstgTcamProfileIdx, &tcamEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgTcamEntrySet failed");

    /* No ROT operations */

    if (kstgNum == 2)
    {
        ppuActionProfile.setNextState = prvTgfDauProfileIdx;
    }
    else
    {
        ppuActionProfile.setNextState = keyGenProfileIdx << 4;
    }

    /* Write PPU action table entry */
    rc = cpssDxChPpuActionTableEntrySet(prvTgfDevNum,
            kstgNum, kstgActionProfileIdx, &ppuActionProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuActionTableEntrySet failed");
}

/**
 * @internal prvTgfPpuIpv6DipUpdatePpuDauConfigSet function
 * @endinternal
 *
 * @brief    Set PPU DAU profile configuration
 *           - Write first four bytes of MAC source address
 *
 */
static GT_VOID prvTgfPpuIpv6DipUpdatePpuDauConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc;
    CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC dauProfile;

    /* initialize DAU profile to 0 */
    cpssOsMemSet(&dauProfile, 0, sizeof(dauProfile));

    /* Populate DAU profile entry to write to first four bytes of MAC source address */
    dauProfile.setDescBits[0].spByteWriteEnable  = GT_TRUE;
    dauProfile.setDescBits[0].spByteSrcOffset    = 1 * 8;   /* bit offset */
    dauProfile.setDescBits[0].spByteNumBits      = 7;       /* num of bits - 1 = 8 - 1 */
    dauProfile.setDescBits[0].spByteTargetOffset = 0 * 8;

    dauProfile.setDescBits[1].spByteWriteEnable  = GT_TRUE;
    dauProfile.setDescBits[1].spByteSrcOffset    = 2 * 8;   /* bit offset */
    dauProfile.setDescBits[1].spByteNumBits      = 7;       /* num of bits - 1 = 8 - 1 */
    dauProfile.setDescBits[1].spByteTargetOffset = 1 * 8;

    /* write entry to DAU profile table */
    rc = cpssDxChPpuDauProfileEntrySet(prvTgfDevNum, prvTgfDauProfileIdx, &dauProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuDauProfileEntrySet failed");

}

/**
* @internal prvTgfPpuIpv6DipUpdatePpuConfigSet function
* @endinternal
*
* @brief  Set configuration for Ipv6DipUpdate Packet
*           1. Initialize PPU
*           2. Configure K_stg0, K_stg1, K_stg2 and DAU
*/
GT_VOID prvTgfPpuIpv6DipUpdatePpuConfigSet
(
        GT_VOID
)
{
    GT_U32 i;

    /* ppu init configuration set */
    prvTgfPpuIpv6DipUpdatePpuInitConfigSet();

    for (i=0; i<3; i++)
    {
        /* PPU K_stg 0 configuration Set */
        prvTgfPpuIpv6DipUpdatePpuKstgConfigSet(i);
    }

    /* PPU DAU configuration Set */
    prvTgfPpuIpv6DipUpdatePpuDauConfigSet();
}

/**
* @internal prvTgfPpuIpv6DipUpdateRoutingConfigSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*
*/
GT_VOID prvTgfPpuIpv6DipUpdateRoutingConfigSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *regularEntryPtr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPV6ADDR                             ipAddr;
    PRV_TGF_BRG_MAC_ENTRY_STC               macEntry;

    /* create a macEntry with .daRoute = GT_TRUE */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    macEntry.key.entryType                  = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId         = PRV_TGF_SEND_VLANID_CNS;
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum  = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_FDB_PORT_IDX_CNS];
    macEntry.daRoute                        = GT_TRUE;

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port 1 */
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite: %d", prvTgfDevNum);


    PRV_UTF_LOG0_MAC("==== Setting LTT Route Configuration ====\n");

    /* -------------------------------------------------------------------------
     * 1. Enable Routing
     */

    /* AUTODOC: enable Unicast IPv6 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV6_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: enable IPv6 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV6_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* -------------------------------------------------------------------------
     * 2. Create the Route entry (Next hop) in Route table and Router ARP Table
     */

    /* AUTODOC: write a ARP MAC 00:00:00:00:00:22 to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfArpMac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfIpRouterArpAddrWrite(prvTgfRouterArpIndex, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* write a UC Route entry to the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    regularEntryPtr = &routeEntriesArray[0];
    regularEntryPtr->cmd                        = CPSS_PACKET_CMD_ROUTE_E;
    regularEntryPtr->nextHopVlanId              = PRV_TGF_NEXTHOP_VLANID_CNS;
    regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
    regularEntryPtr->nextHopInterface.devPort.hwDevNum = prvTgfDevNum;
    regularEntryPtr->nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS];
    regularEntryPtr->nextHopARPPointer          = prvTgfRouterArpIndex;

    /* AUTODOC: add UC route entry with nexthop VLAN 6 and nexthop port 3 */
    rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 4. Create a new Ipv6 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* fill a destination IPv6 address for the prefix */
    cpssOsMemSet(&ipAddr, 0, sizeof(ipAddr));
    ipAddr.arIP[0] = 0x1B;
    ipAddr.arIP[1] = 0x1A;
    ipAddr.arIP[14] = 0xA2;
    ipAddr.arIP[15] = 0xA1;
    rc = prvTgfIpLpmIpv6UcPrefixAdd(prvTgfLpmDBId, 0 /* vrfId */, ipAddr,
                                    128, &nextHopInfo, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv6UcPrefixAdd: %d", prvTgfDevNum);

}

/**
* @internal prvTgfPpuIpv6DipUpdateTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet and capture:
*         Check captured packet.
*/
GT_VOID prvTgfPpuIpv6DipUpdateTrafficGenerate
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
 * @internal prvTgfPpuIpv6DipUpdateRestore function
 * @endinternal
 *
 * @brief Restore original configuration.
 */
GT_VOID prvTgfPpuIpv6DipUpdateRestore
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
    GT_IPV6ADDR                            ipAddr;

    /* -------------------------------------------------------------------------
     * Restore Port Configuration
     */

    /* Invalidate VLAN */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_NEXTHOP_VLANID_CNS);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, PRV_TGF_NEXTHOP_VLANID_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, PRV_TGF_SEND_VLANID_CNS);

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
    rc = cpssDxChPpuSpBusDefaultProfileSet(prvTgfDevNum, prvTgfSpBusProfileIdx, &prvTgfRestoreCfg.spBusProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuSpBusDefaultProfileSet failed");

    cpssOsMemSet(&keyGenProfile, 0, sizeof(keyGenProfile));
    cpssOsMemSet(&tcamEntry, 0, sizeof(tcamEntry));
    cpssOsMemSet(&ppuActionProfile, 0, sizeof(ppuActionProfile));
    for (i=0; i<3; i++)
    {
        rc = cpssDxChPpuKstgKeyGenProfileSet(prvTgfDevNum, i /*kstgNum*/, keyGenProfileIdx, &keyGenProfile);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgKeyGenProfileSet K_stg=%d failed", 0);

        rc = cpssDxChPpuKstgTcamEntrySet(prvTgfDevNum, i /*kstgNum*/, kstgTcamProfileIdx, &tcamEntry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"cpssDxChPpuKstgTcamEntrySet K_stg=%d failed", 0);

        rc = cpssDxChPpuActionTableEntrySet(prvTgfDevNum, i /*kstgNum*/, kstgActionProfileIdx, &ppuActionProfile);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"cpssDxChPpuActionTableEntrySet K_stg=%d failed", 0);
    }

    /* restore dau profile */
    cpssOsMemSet(&dauProfile, 0, sizeof(dauProfile));
    rc = cpssDxChPpuDauProfileEntrySet(prvTgfDevNum, dauProfileIdx, &dauProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChPpuDauProfileEntrySet failed");

    /* AUTODOC: delete the IPv6 prefix */
    /* fill a destination IPv6 address for the prefix */
    cpssOsMemSet(&ipAddr, 0, sizeof(ipAddr));
    ipAddr.arIP[0] = 0x1B;
    ipAddr.arIP[1] = 0x1A;
    ipAddr.arIP[14] = 0xA2;
    ipAddr.arIP[15] = 0xA1;

    rc = prvTgfIpLpmIpv6UcPrefixDel(prvTgfLpmDBId, 0, ipAddr, 128);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    /* AUTODOC: disable Unicast IPv6 Routing on port 0 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV6_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: disable IPv6 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS, CPSS_IP_UNICAST_E,
                                   CPSS_IP_PROTOCOL_IPV6_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* disable Patricia trie validity */
    prvTgfIpValidityCheckEnable(GT_TRUE);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", prvTgfDevNum);

    /* AUTODOC: disable VLAN based MAC learning */
    rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_SEND_VLANID_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);
}
