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
* @file prvTgfBrgSrcId.c
*
* @brief Bridge Source-Id UT.
*
* @version   10
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/generic/bridge/cpssGenBrgGen.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <bridge/prvTgfBrgSrcId.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgSrcId.h>
#include <sgt/prvTgfSgt.h>
#include <common/tgfIpGen.h>
#include <bridge/prvTgfFdbBasicIpv4UcRouting.h>

extern GT_STATUS internal_cpssDxChCfgEpclTrgPortModeSet
(
    IN   GT_U8          devNum,
    OUT  GT_U32         mode/* 0 = EPG mode , 1 = SRC_ID+ desc<copy_reserved> mode */
);

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN0 Id */
#define PRV_TGF_VLANID_CNS  2

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS     0

/* port number to send traffic to */
#define PRV_TGF_RECEIVE_PORT_IDX_CNS  1

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/* FDB MAC entry */
static PRV_TGF_BRG_MAC_ENTRY_STC prvTgfMacEntry;

/* Original configurations */
static GT_BOOL origPortSrcIdForceEnable;
static CPSS_BRG_SRC_ID_ASSIGN_MODE_ENT origGlobalSrcIdAssignMode;
static GT_BOOL origPortUcastEgressFilter;


/* MAC Address */
static TGF_MAC_ADDR macAddr1 = {0,0,0,0,0,0x01};

static TGF_MAC_ADDR macAddr2 = {0,0,0,0,0,0x02};


#define     SRC_ID_DEFAULT_CNS  2
#define     SRC_ID_GROUP_1_CNS  1
#define     SRC_ID_GROUP_0_CNS  0

#define     SRC_ID_RESET_CNS    0/* must be 0 */

/* bypass ingress pipe in PCL action */
#define     FULL_CONTROL_CNS    1
#define     RESTORE_DEFAULT_FULL_CONTROL_CNS    0

#ifdef CHX_FAMILY
extern GT_STATUS prvCpssDxChPclBypassFullControl_debug(IN GT_U32   fullControl);
#endif /*CHX_FAMILY*/

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* srcMac */
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

static GT_BOOL tgfEnablePclRedirect        = GT_FALSE;
static GT_BOOL tgfEnableBypassIngressPipe  = GT_FALSE;
/* PCL rule index */
static GT_U32   prvTgfPclRuleIndex = 987;


/******************************************************************************\
 *                            Scalable SGT                                    *
\******************************************************************************/

static GT_U32 prvTgfScalableSgtTargetIdSave;
static GT_BOOL prvTgfScalableSgtEnableSave;
static PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT prvTgfScalableSgtCfgTabAccModeSave;
static GT_ETHERADDR prvTgfScalableSgtArpMacAddrSave;
static CPSS_PACKET_CMD_ENT prvTgfScalableSgtNhPacketCmdSave;
static PRV_TGF_IP_UC_ROUTE_ENTRY_STC prvTgfScalableSgtRouteEntrySave;
static CPSS_DXCH_BRG_SRC_ID_TARGET_INDEX_CONFIG_STC prvTgfScalableSgtTargetConfigSave;

#define PRV_TGF_VLANID_5_CNS 5
/* the srcId by the EQ is limited to 'eport range' - 'physical range' :
    in AC5P/X/Harrier : the 'eport range' is 13 bits , 'physical range' is 7 bits
        that leaves us with 6 bits for override of src-id bits.
    in Ironman-L : the 'eport range' is 10 bits , 'physical range' is 6 bits
        that leaves us with only 4 bits for override of src-id bits.
        so we cant use '20' as it >= 16 (that 4 bits allow).
*/
#define PRV_TGF_SRC_ID_BASE_CNS (PRV_CPSS_SIP_6_30_CHECK_MAC(prvTgfDevNum) ? \
                            11/*IM_L: limited to 4 bits*/    :                \
                            20)/*     limited to 6 bits*/

#define PRV_TGF_EPORT_MUX_LEN_CNS (PRV_CPSS_SIP_6_30_CHECK_MAC(prvTgfDevNum) ? \
                            4/*IM_L*/    :                                     \
                            6)
#define PRV_TGF_ARP_INDEX_CNS 1
#define PRV_TGF_ROUTER_NH_INDEX_CNS 1
/* bit pos = eportFieldWidth - eportMuxlen*/
#define PRV_TGF_SHFT_PORT_BIT (PRV_CPSS_SIP_6_30_CHECK_MAC(prvTgfDevNum) ? \
                            (10 - PRV_TGF_EPORT_MUX_LEN_CNS)/*IM_L*/    : \
                            (13 - PRV_TGF_EPORT_MUX_LEN_CNS))

#define PRV_TGF_SRC_ID_LEN PRV_TGF_EPORT_MUX_LEN_CNS/*to use the same amount of bits from the eport */
#define PRV_TGF_SRC_ID_MSB (PRV_TGF_SRC_ID_LEN-1) /* to start at bit 0 of the src-id */



/* index to EPCL Configuration table in SRC_TRG mode */
#define PRV_TGF_EPCL_SRC_TRG_INDEX(_srcIndex, _dstIndex)      ((_dstIndex << 6) + _srcIndex)
/* index to EPCL Rule Index */
#define PRV_TGF_EPCL_RULE_INDEX(_index)      (prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(10 + _index))
#define PRV_TGF_COUNT_SET_CNS     CPSS_IP_CNT_SET0_E

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfScalableSgtPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* srcMac */
};
static TGF_PACKET_L2_STC prvTgfScalableSgtPacketL2PartPkt1 = {
    {0x00, 0x00, 0x00, 0x00, 0x12, 0x34},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* srcMac */
};
static TGF_PACKET_L2_STC prvTgfScalableSgtPacketL2PartPkt2 = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* srcMac */
};
/* VLAN_TAG part - Tag0 */
static TGF_PACKET_VLAN_TAG_STC prvTgfScalableSgtPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_5_CNS                          /* pri, cfi, VlanId */
};
/* IPv4 packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfScalableSgtPacketEtherTypePart_Ipv4 = {
    TGF_ETHERTYPE_0800_IPV4_TAG_CNS
};
static TGF_PACKET_IPV4_STC prvTgfScalableSgtPacketIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    10,                  /* typeOfService */
    0x2A,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0xFF,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    { 2,  2,  2,  2},   /* srcAddr */
    { 1,  1,  1,  1}    /* dstAddr */
};
TGF_PACKET_IPV4_STC prvTgfScalableSgtPacketIpv4PartPkt4 = {
    4,                  /* version */
    5,                  /* headerLen */
    10,                  /* typeOfService */
    0x2A,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0xFF,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    { 2,  2,  2,  2},   /* srcAddr */
    { 4,  4,  4,  4}    /* dstAddr */
};
/* DATA of packet */
static GT_U8 prvTgfScalableSgtPayloadDataArr[] = {
    0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc,
    0xdd, 0xee, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc,
    0xdd, 0xee, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc,
    0xdd, 0xee, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfScalableSgtPacketPayloadPart = {
    sizeof(prvTgfScalableSgtPayloadDataArr),                       /* dataLength */
    prvTgfScalableSgtPayloadDataArr                                /* dataPtr */
};
static TGF_PACKET_PART_STC prvTgfPacketScalableSgtPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfScalableSgtPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfScalableSgtPacketVlanTag0Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfScalableSgtPacketEtherTypePart_Ipv4},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfScalableSgtPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfScalableSgtPacketPayloadPart}
};

/* TAG0  tagged PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfoScalableSgt =
{
    TGF_L2_HEADER_SIZE_CNS +
    TGF_VLAN_TAG_SIZE_CNS +
    TGF_ETHERTYPE_SIZE_CNS +
    TGF_IPV4_HEADER_SIZE_CNS +
    sizeof(prvTgfScalableSgtPayloadDataArr),       /* totalLen */
    sizeof(prvTgfPacketScalableSgtPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketScalableSgtPartArray                                       /* partsArray */
};
/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/
/**
* @internal prvTgfBrgSrcIdTestInit function
* @endinternal
*
* @brief   Set VLAN entry.
*
* @param[in] vlanId                   -  to be configured
*                                       None
*/
static GT_VOID prvTgfBrgSrcIdTestInit
(
    IN GT_U16           vlanId
)
{
    GT_U32                      portIter  = 0;
    GT_U32                      portCount = 0;
    GT_STATUS                   rc        = GT_OK;
    CPSS_PORTS_BMP_STC          portsMembers;
    CPSS_PORTS_BMP_STC          portsTagging;
    PRV_TGF_BRG_VLAN_INFO_STC   vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;

    /* clear entry */
    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC));

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
    vlanInfo.portIsolationMode    = PRV_TGF_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E;
    vlanInfo.ucastLocalSwitchingEn = GT_FALSE;
    vlanInfo.mcastLocalSwitchingEn = GT_FALSE;

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    portCount = prvTgfPortsNum;
    for (portIter = 0; portIter < portCount; portIter++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers,prvTgfPortsArray[portIter]);

        portsTaggingCmd.portsCmd[prvTgfPortsArray[portIter]] = PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E;
    }

    /* AUTODOC: create VLAN 2 with all untagged ports */
    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum,
                                 vlanId,
                                 &portsMembers,
                                 &portsTagging,
                                 &vlanInfo,
                                 &portsTaggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d", vlanId);


    /* Add 2 MAC Entries on  */

    /* clear entry */
    cpssOsMemSet(&prvTgfMacEntry, 0, sizeof(prvTgfMacEntry));

    prvTgfMacEntry.key.entryType                = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    prvTgfMacEntry.key.key.macVlan.vlanId       = vlanId;
    prvTgfMacEntry.dstInterface.type            = CPSS_INTERFACE_PORT_E;
    prvTgfMacEntry.dstInterface.devPort.hwDevNum  = prvTgfDevsArray[PRV_TGF_SEND_PORT_IDX_CNS];/*the sender*/
    prvTgfMacEntry.dstInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];/*the sender*/
    prvTgfMacEntry.isStatic                     = GT_FALSE;
    prvTgfMacEntry.daCommand                    = PRV_TGF_PACKET_CMD_FORWARD_E;
    prvTgfMacEntry.saCommand                    = PRV_TGF_PACKET_CMD_FORWARD_E;
    prvTgfMacEntry.daRoute                      = GT_FALSE;
    prvTgfMacEntry.mirrorToRxAnalyzerPortEn     = GT_FALSE;
    prvTgfMacEntry.userDefined                  = 0;
    prvTgfMacEntry.daQosIndex                   = 0;
    prvTgfMacEntry.saQosIndex                   = 0;
    prvTgfMacEntry.daSecurityLevel              = 0;
    prvTgfMacEntry.saSecurityLevel              = 0;
    prvTgfMacEntry.appSpecificCpuCode           = GT_FALSE;
    prvTgfMacEntry.pwId                         = 0;
    prvTgfMacEntry.spUnknown                    = GT_FALSE;
    prvTgfMacEntry.sourceId                     = SRC_ID_GROUP_1_CNS;

    cpssOsMemCpy(prvTgfMacEntry.key.key.macVlan.macAddr.arEther, macAddr1, sizeof(TGF_MAC_ADDR));

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:01, VLAN 2, port 0, sourceId 1 */
    rc = prvTgfBrgFdbMacEntrySet(&prvTgfMacEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet");

    prvTgfMacEntry.dstInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
    prvTgfMacEntry.sourceId                     = SRC_ID_GROUP_0_CNS;
    cpssOsMemCpy(prvTgfMacEntry.key.key.macVlan.macAddr.arEther, macAddr2, sizeof(TGF_MAC_ADDR));

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:02, VLAN 2, port 1, sourceId 0 */
    rc = prvTgfBrgFdbMacEntrySet(&prvTgfMacEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet");

}

/**
* @internal prvTgfBrgSrcIdTestPacketSend function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*/
static GT_VOID prvTgfBrgSrcIdTestPacketSend
(
    GT_VOID
)
{
    GT_STATUS       rc           = GT_OK;

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the ethertype
      of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, NULL);

    /* AUTODOC: send packet with DA 00:00:00:00:00:02, SA 00:00:00:00:00:01 from port 0 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}

/**
* @internal prvTgfBrgSrcIdTestTableReset function
* @endinternal
*
* @brief   Function clears FDB, VLAN tables and internal table of captured packets.
*
* @param[in] vlanId                   -  to be cleared
*                                       None
*/
static GT_VOID prvTgfBrgSrcIdTestTableReset
(
    IN GT_U16 vlanId
)
{
    GT_STATUS   rc = GT_OK;

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(vlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, vlanId);

    /* AUTODOC: reset port default source-ID to 0 */
    rc = prvTgfBrgSrcIdPortDefaultSrcIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], SRC_ID_RESET_CNS);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdPortDefaultSrcIdSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], SRC_ID_RESET_CNS);
}

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/
/**
* @internal prvTgfTrafficPclRulesDefine function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfTrafficPclRulesDefine
(
    INOUT PRV_TGF_PCL_RULE_FORMAT_UNT *maskPtr,
    INOUT PRV_TGF_PCL_RULE_FORMAT_UNT *pattPtr,
    INOUT PRV_TGF_PCL_ACTION_STC      *actionPtr
)
{
    GT_STATUS rc = GT_OK;

    /* mask for MAC address */
    cpssOsMemSet(maskPtr, 0, sizeof(*maskPtr));
    cpssOsMemSet(maskPtr->ruleStdNotIp.macDa.arEther, 0xFF,
            sizeof(maskPtr->ruleStdNotIp.macDa.arEther));

    /* define mask, pattern and action */
    cpssOsMemSet(pattPtr, 0, sizeof(*pattPtr));
    cpssOsMemCpy(pattPtr->ruleStdNotIp.macDa.arEther, prvTgfPacketL2Part.daMac,
            sizeof(prvTgfPacketL2Part.daMac));

    /* action redirect */
    cpssOsMemSet(actionPtr, 0, sizeof(*actionPtr));
    actionPtr->pktCmd                                           = CPSS_PACKET_CMD_FORWARD_E;
    actionPtr->redirect.redirectCmd                             = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    actionPtr->redirect.data.outIf.outInterface.type            = CPSS_INTERFACE_PORT_E;
    actionPtr->redirect.data.outIf.outInterface.devPort.hwDevNum  = prvTgfDevNum;
    actionPtr->redirect.data.outIf.outInterface.devPort.portNum =
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];

    return rc;
};


/**
* @internal prvTgfPclBypassInit function
* @endinternal
*
* @brief   init PCL bypass test configuration
*/
static GT_VOID prvTgfPclBypassInit(GT_VOID)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      patt;
    PRV_TGF_PCL_ACTION_STC           action;

    /* init PCL Engine for send port */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);


    /* -------------------------------------------------------------------------
     * 2. Set PCL rule
     */

    /* define mask, pattern and action */
    rc = prvTgfTrafficPclRulesDefine(&mask, &patt, &action);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfTrafficPclRulesDefine: %d",
        prvTgfDevNum);


    action.bypassIngressPipe = tgfEnableBypassIngressPipe;
    action.bypassBridge      = GT_FALSE;

#ifdef CHX_FAMILY
    /* WA to state the CPSS not to manipulate the bypassIngressPipe,bypassBridge
       when 'redirect to egress interface' */
    prvCpssDxChPclBypassFullControl_debug(FULL_CONTROL_CNS);
#endif /*CHX_FAMILY*/

    /* set PCL rule */
    rc = prvTgfPclRuleSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
        prvTgfPclRuleIndex, &mask, &patt, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
        prvTgfDevNum, prvTgfPclRuleIndex);

}

/**
* @internal prvTgfPclBypassRestore function
* @endinternal
*
* @brief   None
*/
static GT_VOID prvTgfPclBypassRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* disable ingress policy on src port */
    rc = prvTgfPclPortIngressPolicyEnable(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPclPortIngressPolicyEnable");

    /* invalidate PCL rule */
    rc = prvTgfPclRuleValidStatusSet(
            CPSS_PCL_RULE_SIZE_STD_E, prvTgfPclRuleIndex, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPclRuleValidStatusSet");

#ifdef CHX_FAMILY
    prvCpssDxChPclBypassFullControl_debug(RESTORE_DEFAULT_FULL_CONTROL_CNS);
#endif /*CHX_FAMILY*/

    return ;
};

/**
* @internal prvTgfBrgSrcIdPortForceConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Create VLAN 2 on all ports.
*         Set MAC table with two entries:
*         - MAC = 00:00:00:00:00:01, VLAN = 2, intType = PORT,
*         port = 0, sourceId = 1
*         - MAC = 00:00:00:00:00:02, VLAN = 2, intType = PORT,
*         port = 8, sourceId = 0
*         Source ID configuration:
*         Disable port force source-ID assigment
*         Configure Global Assigment mode to FDB SA-based.
*         Configure default port source-ID to 2.
*         Delete port 8 from source-ID group 1.
*         Enable Unicast Egreess filter.
*         Generate Traffic:
*         Send to device's port 0 packet:
*         daMAC = 00:00:00:00:00:02, saMAC = 00:00:00:00:00:01,
*         etherType = 0x8100 vlanID = 0x2.
*         Success Criteria:
*         Packet is not captured in all ports.
*         Additional Configuration:
*         Enable Port Force Src-ID assigment.
*         Generate Traffic:
*         Send to device's port 0 packet:
*         daMAC = 00:00:00:00:00:02, saMAC = 00:00:00:00:00:01,
*         etherType = 0x8100 vlanID = 0x2.
*         Success Criteria:
*         Packet is captured in port: 8.
*/
GT_VOID prvTgfBrgSrcIdPortForceConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS       rc           = GT_OK;


    /* AUTODOC: SAVE CONFIGURATION: */
    /* AUTODOC: get force source-ID assigment on send port 0 */
    rc = prvTgfBrgSrcIdPortSrcIdForceEnableGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &origPortSrcIdForceEnable);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdPortSrcIdForceEnableGet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], origPortSrcIdForceEnable);

    /* AUTODOC: get global assign mode to FDB SA-based */
    rc = prvTgfBrgSrcIdGlobalSrcIdAssignModeGet(prvTgfDevNum, &origGlobalSrcIdAssignMode);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdGlobalSrcIdAssignModeGet: %d, %d", prvTgfDevNum, origGlobalSrcIdAssignMode);

    /* AUTODOC: get Unicast egress filter for port 1 */
    rc = prvTgfBrgSrcIdPortUcastEgressFilterGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], &origPortUcastEgressFilter);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdPortUcastEgressFilterGet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], origPortUcastEgressFilter);


    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: disable force source-ID assigment on send port 0 */
    rc = prvTgfBrgSrcIdPortSrcIdForceEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdPortSrcIdForceEnableSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);

    /* AUTODOC: set global assign mode to FDB SA-based */
    rc = prvTgfBrgSrcIdGlobalSrcIdAssignModeSet(CPSS_BRG_SRC_ID_ASSIGN_MODE_FDB_PORT_DEFAULT_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdGlobalSrcIdAssignModeSet: %d, %d", prvTgfDevNum, CPSS_BRG_SRC_ID_ASSIGN_MODE_FDB_PORT_DEFAULT_E);

    /* AUTODOC: set default source-ID to 2 for port 0 */
    rc = prvTgfBrgSrcIdPortDefaultSrcIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], SRC_ID_DEFAULT_CNS);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdPortDefaultSrcIdSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], SRC_ID_DEFAULT_CNS);

    if(tgfEnablePclRedirect == GT_TRUE)
    {
        /* AUTODOC: delete port 1 from source-ID group 2 */
        /* AUTODOC: meaning that if FDB will not do override of the srcId ... the packet will be dropped */
        rc = prvTgfBrgSrcIdGroupPortDelete (prvTgfDevNum, SRC_ID_DEFAULT_CNS, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdGroupPortDelete: %d, %d, %d",
                                     prvTgfDevNum, SRC_ID_DEFAULT_CNS, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);
    }
    else
    {
        /* AUTODOC: delete port 1 from source-ID group 1 */
        rc = prvTgfBrgSrcIdGroupPortDelete (prvTgfDevNum, SRC_ID_GROUP_1_CNS, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdGroupPortDelete: %d, %d, %d",
                                     prvTgfDevNum, SRC_ID_GROUP_1_CNS, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);
    }

    /* AUTODOC: enable unicast egress filter for port 1 */
    rc = prvTgfBrgSrcIdPortUcastEgressFilterSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdPortUcastEgressFilterSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], GT_TRUE);

    /* set VLAN entry, TPID global tables */
    prvTgfBrgSrcIdTestInit(PRV_TGF_VLANID_CNS);

    if(tgfEnablePclRedirect == GT_TRUE)
    {
        prvTgfPclBypassInit();
    }

}

/**
* @internal prvTgfBrgSrcIdPortForceTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*/
GT_VOID prvTgfBrgSrcIdPortForceTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       portsCount = prvTgfPortsNum;
    GT_U32       portIter   = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;

    /* AUTODOC: GENERATE TRAFFIC: */

    /*********************************************/
    /* AUTODOC: Phase 1 - FDB SA-based assigment */
    /*********************************************/

    /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* send packet */
    prvTgfBrgSrcIdTestPacketSend();

    /* check counters */

    /* read counters */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);

    if(tgfEnablePclRedirect == GT_TRUE &&
       tgfEnableBypassIngressPipe == GT_FALSE)
    {
        /* AUTODOC: verify packet is subject to FDB SRC-ID assignment */
        /* AUTODOC: meaning that default of the src port NOT used , so not drops */
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsRcv counter than expected");
    }
    else
    {
        /* AUTODOC: verify packet is not captured on port 1 */
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsRcv counter than expected");
    }

    /*******************************************/
    /* AUTODOC: Phase 2 - Port force assignment */
    /*******************************************/

    /* AUTODOC: enable force source-ID assigment on port 0 */
    prvTgfBrgSrcIdPortSrcIdForceEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);

    /* send packet */
    prvTgfBrgSrcIdTestPacketSend();

    /* read counters */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);

    if(tgfEnablePclRedirect == GT_TRUE)
    {
        /* AUTODOC: verify packet is NOT subject to FDB SRC-ID assignment */
        /* AUTODOC: meaning that default of the src port used , so drop packet */
        UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsRcv counter than expected");
    }
    else
    {
        /* AUTODOC: verify packet is captured on port 1 */
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsRcv counter than expected");
    }
}

/**
* @internal prvTgfBrgSrcIdPortForceConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgSrcIdPortForceConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    if(tgfEnablePclRedirect == GT_TRUE)
    {
        /* AUTODOC: add port 1 to source-ID group 0 */
        prvTgfBrgSrcIdGroupPortAdd(prvTgfDevNum, SRC_ID_DEFAULT_CNS, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);
    }
    else
    {
        /* AUTODOC: add port 1 to source-ID group 1 */
        prvTgfBrgSrcIdGroupPortAdd(prvTgfDevNum, SRC_ID_GROUP_1_CNS, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);
    }

    /* table reset */
    prvTgfBrgSrcIdTestTableReset(PRV_TGF_VLANID_CNS);

    if(tgfEnablePclRedirect == GT_TRUE)
    {
        /* need to remove the PCL related configurations */
        prvTgfPclBypassRestore();
    }

    tgfEnablePclRedirect        = GT_FALSE;
    tgfEnableBypassIngressPipe  = GT_FALSE;

    /* AUTODOC: set force source-ID assigment on send port 0 */
    rc = prvTgfBrgSrcIdPortSrcIdForceEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], origPortSrcIdForceEnable);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdPortSrcIdForceEnableSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], origPortSrcIdForceEnable);

    /* AUTODOC: set global assign mode to FDB SA-based */
    rc = prvTgfBrgSrcIdGlobalSrcIdAssignModeSet(origGlobalSrcIdAssignMode);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdGlobalSrcIdAssignModeSet: %d, %d", prvTgfDevNum, origGlobalSrcIdAssignMode);

    /* AUTODOC: set Unicast egress filter for port 1 */
    rc = prvTgfBrgSrcIdPortUcastEgressFilterSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], origPortUcastEgressFilter);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdPortUcastEgressFilterSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], origPortUcastEgressFilter);

}

/**
* @internal prvTgfBrgSrcIdPortPclRedirect function
* @endinternal
*
* @brief   Set parameters for SRC-ID tests. this only save values that later used
*         during run of test
* @param[in] enablePclRedirect        - enable PCL action 'redirect'
* @param[in] enableBypassIngressPipe  - enable PCL action 'BypassIngressPipe'
*                                       None
*/
GT_VOID prvTgfBrgSrcIdPortPclRedirect
(
    IN GT_BOOL  enablePclRedirect,
    IN GT_BOOL  enableBypassIngressPipe
)
{
    tgfEnablePclRedirect       = enablePclRedirect;
    tgfEnableBypassIngressPipe = enableBypassIngressPipe;
}


/**
* @internal prvTgfBrgSrcIdFdbSaAssigmentConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Create VLAN 2 on all ports.
*         Set MAC table with two entries:
*         - MAC = 00:00:00:00:00:01, VLAN = 2, intType = PORT,
*         port = 0, sourceId = 1
*         - MAC = 00:00:00:00:00:02, VLAN = 2, intType = PORT,
*         port = 8, sourceId = 0
*         Source ID configuration:
*         Disable port force source-ID assigment
*         Configure Global Assigment mode to FDB SA-based.
*         Configure default port source-ID to 2.
*         Delete port 8 from source-ID group 1.
*         Enable Unicast Egreess filter.
*         Generate Traffic:
*         Send to device's port 0 packet:
*         daMAC = 00:00:00:00:00:02, saMAC = 00:00:00:00:00:01,
*         etherType = 0x8100 vlanID = 0x2.
*         Success Criteria:
*         Packet is not captured in port 8.
*         Additional Configuration:
*         Add to Source-ID group 1 port 8.
*         Delete from Source-ID group 2 port 8.
*         Generate Traffic:
*         Send to device's port 0 packet:
*         daMAC = 00:00:00:00:00:02, saMAC = 00:00:00:00:00:01,
*         etherType = 0x8100 vlanID = 0x2.
*         Success Criteria:
*         Packet is captured on port: 8.
*/
GT_VOID prvTgfBrgSrcIdFdbSaAssigmentConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS       rc           = GT_OK;

    /* AUTODOC: SAVE CONFIGURATION: */
    /* AUTODOC: get force source-ID assigment on send port 0 */
    rc = prvTgfBrgSrcIdPortSrcIdForceEnableGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &origPortSrcIdForceEnable);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdPortSrcIdForceEnableGet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], origPortSrcIdForceEnable);

    /* AUTODOC: get global assign mode to FDB SA-based */
    rc = prvTgfBrgSrcIdGlobalSrcIdAssignModeGet(prvTgfDevNum, &origGlobalSrcIdAssignMode);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdGlobalSrcIdAssignModeGet: %d, %d", prvTgfDevNum, origGlobalSrcIdAssignMode);

    /* AUTODOC: get Unicast egress filter for port 1 */
    rc = prvTgfBrgSrcIdPortUcastEgressFilterGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], &origPortUcastEgressFilter);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdPortUcastEgressFilterGet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], origPortUcastEgressFilter);

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: disable force source-ID assigment on send port 0 */
    rc = prvTgfBrgSrcIdPortSrcIdForceEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdPortSrcIdForceEnableSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);

    /* AUTODOC: set global assign mode to FDB SA-based */
    rc = prvTgfBrgSrcIdGlobalSrcIdAssignModeSet(CPSS_BRG_SRC_ID_ASSIGN_MODE_FDB_PORT_DEFAULT_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdGlobalSrcIdAssignModeSet: %d, %d", prvTgfDevNum, CPSS_BRG_SRC_ID_ASSIGN_MODE_FDB_PORT_DEFAULT_E);

    /* AUTODOC: set default source-ID to 2 for port 0 */
    rc = prvTgfBrgSrcIdPortDefaultSrcIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], SRC_ID_DEFAULT_CNS);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdPortDefaultSrcIdSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], SRC_ID_DEFAULT_CNS);

    /* AUTODOC: delete port 1 from source-ID group 1 */
    rc = prvTgfBrgSrcIdGroupPortDelete (prvTgfDevNum, SRC_ID_GROUP_1_CNS, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdGroupPortDelete: %d, %d, %d",
                                 prvTgfDevNum, SRC_ID_GROUP_1_CNS, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);

    /* AUTODOC: enable unicast egress filter for port 1 */
    rc = prvTgfBrgSrcIdPortUcastEgressFilterSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdPortUcastEgressFilterSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], GT_TRUE);

    /* set VLAN entry, TPID global tables */
    prvTgfBrgSrcIdTestInit(PRV_TGF_VLANID_CNS);
}

/**
* @internal prvTgfBrgSrcIdFdbSaAssigmentTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*/
GT_VOID prvTgfBrgSrcIdFdbSaAssigmentTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       portsCount = prvTgfPortsNum;
    GT_U32       portIter   = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;

    /* AUTODOC: GENERATE TRAFFIC: */

    /*********************************************/
    /* AUTODOC: Phase 1 - FDB SA-based assigment */
    /*********************************************/

    /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* send packet */
    prvTgfBrgSrcIdTestPacketSend();

    /* check counters */

    /* read counters */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);

    /* AUTODOC: verify packet is not captured in port 1 */
    UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                 "get another goodPktsRcv counter than expected");

    /*********************************************/
    /* AUTODOC: Phase 2 - FDB SA-based assigment */
    /*********************************************/

    /* AUTODOC: add to Source-ID group 1 port 1 */
    prvTgfBrgSrcIdGroupPortAdd(prvTgfDevNum, 1, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);

    /* AUTODOC: delete from Source-ID group 0 port 1 */
    prvTgfBrgSrcIdGroupPortDelete(prvTgfDevNum, SRC_ID_GROUP_0_CNS, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);

    /* send packet */
    prvTgfBrgSrcIdTestPacketSend();

    /* read counters */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);

    /* AUTODOC: verify packet is captured in port 1 */
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                 "get another goodPktsRcv counter than expected");
}

/**
* @internal prvTgfBrgSrcIdFdbSaAssigmentConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgSrcIdFdbSaAssigmentConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: add to Source-ID group 2 port 1 */
    prvTgfBrgSrcIdGroupPortAdd(prvTgfDevNum, SRC_ID_GROUP_0_CNS, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);

    /* table reset */
    prvTgfBrgSrcIdTestTableReset(PRV_TGF_VLANID_CNS);

    /* AUTODOC: set force source-ID assigment on send port 0 */
    rc = prvTgfBrgSrcIdPortSrcIdForceEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], origPortSrcIdForceEnable);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdPortSrcIdForceEnableSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], origPortSrcIdForceEnable);

    /* AUTODOC: set global assign mode to FDB SA-based */
    rc = prvTgfBrgSrcIdGlobalSrcIdAssignModeSet(origGlobalSrcIdAssignMode);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdGlobalSrcIdAssignModeSet: %d, %d", prvTgfDevNum, origGlobalSrcIdAssignMode);

    /* AUTODOC: set Unicast egress filter for port 1 */
    rc = prvTgfBrgSrcIdPortUcastEgressFilterSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], origPortUcastEgressFilter);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdPortUcastEgressFilterSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], origPortUcastEgressFilter);


}

/**
* @internal prvTgfBrgSrcIdFdbDaAssigmentConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Create VLAN 2 on all ports.
*         Set MAC table with two entries:
*         - MAC = 00:00:00:00:00:01, VLAN = 2, intType = PORT,
*         port = 0, sourceId = 1
*         - MAC = 00:00:00:00:00:02, VLAN = 2, intType = PORT,
*         port = 8, sourceId = 0
*         Source ID configuration:
*         Disable port force source-ID assigment
*         Configure Global Assigment mode to FDB SA-based.
*         Configure default port source-ID to 2.
*         Delete port 8 from source-ID group 1.
*         Enable Unicast Egreess filter.
*         Generate Traffic:
*         Send to device's port 0 packet:
*         daMAC = 00:00:00:00:00:02, saMAC = 00:00:00:00:00:01,
*         etherType = 0x8100 vlanID = 0x2.
*         Success Criteria:
*         Packet is not captured in port 8.
*         Additional Configuration:
*         Add to Source-ID group 1 port 8.
*         Delete from Source-ID group 2 port 8.
*         Generate Traffic:
*         Send to device's port 0 packet:
*         daMAC = 00:00:00:00:00:02, saMAC = 00:00:00:00:00:01,
*         etherType = 0x8100 vlanID = 0x2.
*         Success Criteria:
*         Packet is captured on port: 8.
*/
GT_VOID prvTgfBrgSrcIdFdbDaAssigmentConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS       rc           = GT_OK;

    /* AUTODOC: SAVE CONFIGURATION: */
    /* AUTODOC: get force source-ID assigment on send port 0 */
    rc = prvTgfBrgSrcIdPortSrcIdForceEnableGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &origPortSrcIdForceEnable);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdPortSrcIdForceEnableGet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], origPortSrcIdForceEnable);

    /* AUTODOC: get global assign mode to FDB SA-based */
    rc = prvTgfBrgSrcIdGlobalSrcIdAssignModeGet(prvTgfDevNum, &origGlobalSrcIdAssignMode);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdGlobalSrcIdAssignModeGet: %d, %d", prvTgfDevNum, origGlobalSrcIdAssignMode);

    /* AUTODOC: get Unicast egress filter for port 1 */
    rc = prvTgfBrgSrcIdPortUcastEgressFilterGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], &origPortUcastEgressFilter);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdPortUcastEgressFilterGet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], origPortUcastEgressFilter);

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: disable force source-ID assigment on send port 0 */
    rc = prvTgfBrgSrcIdPortSrcIdForceEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdPortSrcIdForceEnableSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);

    /* AUTODOC: set global assign mode to FDB DA-based */
    rc = prvTgfBrgSrcIdGlobalSrcIdAssignModeSet(CPSS_BRG_SRC_ID_ASSIGN_MODE_FDB_DA_PORT_DEFAULT_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdGlobalSrcIdAssignModeSet: %d, %d", prvTgfDevNum, CPSS_BRG_SRC_ID_ASSIGN_MODE_FDB_PORT_DEFAULT_E);

    /* AUTODOC: set default source-ID to 2 for port 0 */
    rc = prvTgfBrgSrcIdPortDefaultSrcIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], SRC_ID_DEFAULT_CNS);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdPortDefaultSrcIdSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], SRC_ID_DEFAULT_CNS);

    /* AUTODOC: delete port 1 from source-ID group 0 */
    rc = prvTgfBrgSrcIdGroupPortDelete (prvTgfDevNum, SRC_ID_GROUP_0_CNS, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdGroupPortDelete: %d, %d, %d",
                                 prvTgfDevNum, 1, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);

    /* AUTODOC: enable unicast egress filter for port 1 */
    rc = prvTgfBrgSrcIdPortUcastEgressFilterSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdPortUcastEgressFilterSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], GT_TRUE);

    /* set VLAN entry, TPID global tables */
    prvTgfBrgSrcIdTestInit(PRV_TGF_VLANID_CNS);
}

/**
* @internal prvTgfBrgSrcIdFdbDaAssigmentTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 packet:
*         macDa = 00:00:00:00:00:22,
*         macSa = 00:00:00:00:00:11,
*/
GT_VOID prvTgfBrgSrcIdFdbDaAssigmentTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       portsCount = prvTgfPortsNum;
    GT_U32       portIter   = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;

    /* AUTODOC: GENERATE TRAFFIC: */

    /*********************************************/
    /* AUTODOC: Phase 1 - FDB DA-based assigment */
    /*********************************************/

    /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* send packet */
    prvTgfBrgSrcIdTestPacketSend();

    /* check counters */

    /* read counters */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);

    /* AUTODOC: verify packet is not captured in port 1 */
    UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                 "get another goodPktsRcv counter than expected");

    /*********************************************/
    /* AUTODOC: Phase 2 - FDB DA-based assigment */
    /*********************************************/

    /* AUTODOC: add to Source-ID group 0 port 1 */
    prvTgfBrgSrcIdGroupPortAdd(prvTgfDevNum, SRC_ID_GROUP_0_CNS, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);

    /* AUTODOC: delete from Source-ID group 1 port 1 */
    prvTgfBrgSrcIdGroupPortDelete(prvTgfDevNum, SRC_ID_GROUP_1_CNS, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);

    /* send packet */
    prvTgfBrgSrcIdTestPacketSend();

    /* read counters */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);

    /* AUTODOC: verify packet is captured in port 1 */
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                 "get another goodPktsRcv counter than expected");

}

/**
* @internal prvTgfBrgSrcIdFdbDaAssigmentConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgSrcIdFdbDaAssigmentConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: add to Source-ID group 1 port 1 */
    prvTgfBrgSrcIdGroupPortAdd(prvTgfDevNum, 1, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);

    /* table reset */
    prvTgfBrgSrcIdTestTableReset(PRV_TGF_VLANID_CNS);

    /* AUTODOC: set force source-ID assigment on send port 0 */
    rc = prvTgfBrgSrcIdPortSrcIdForceEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], origPortSrcIdForceEnable);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdPortSrcIdForceEnableSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], origPortSrcIdForceEnable);

    /* AUTODOC: set global assign mode to FDB SA-based */
    rc = prvTgfBrgSrcIdGlobalSrcIdAssignModeSet(origGlobalSrcIdAssignMode);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdGlobalSrcIdAssignModeSet: %d, %d", prvTgfDevNum, origGlobalSrcIdAssignMode);

    /* AUTODOC: set Unicast egress filter for port 1 */
    rc = prvTgfBrgSrcIdPortUcastEgressFilterSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], origPortUcastEgressFilter);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgSrcIdPortUcastEgressFilterSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], origPortUcastEgressFilter);
}

/**
* @internal prvTgfScalableSgtIpv4UcLpmRouteConfigSet function
* @endinternal
*
* @brief   Config route entry in LPM table
*
*            pkt_4 for UC ipv4 traffic.
*              SIP = 2.2.2.2
*              DIP = 4.4.4.4
*              VRF = 1
*              DST intf = eport_3
*
*/
GT_VOID prvTgfScalableSgtIpv4UcLpmRouteConfigSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntry;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    GT_IPADDR ipAddr;

    /* -------------------------------------------------------------------------
     * 1. Create the Route entry (Next hop) in Route table and Router ARP Table
     */

    rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum, PRV_TGF_ROUTER_NH_INDEX_CNS, &prvTgfScalableSgtRouteEntrySave, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesRead: %d", prvTgfDevNum);

    /* write a UC Route entry to the Route Table */
    cpssOsMemSet(&routeEntry, 0, sizeof(routeEntry));

    routeEntry.cmd                        = CPSS_PACKET_CMD_ROUTE_E;
    routeEntry.trapMirrorArpBcEnable      = GT_FALSE;
    routeEntry.sipAccessLevel             = 0;
    routeEntry.dipAccessLevel             = 0;
    routeEntry.mtuProfileIndex            = 0;
    routeEntry.nextHopVlanId              = PRV_TGF_VLANID_5_CNS;
    routeEntry.nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
    routeEntry.nextHopInterface.devPort.hwDevNum = prvTgfDevNum;
    routeEntry.nextHopInterface.devPort.portNum = ((PRV_TGF_SRC_ID_BASE_CNS+3) << PRV_TGF_SHFT_PORT_BIT) |
                                                    prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS] ;
    routeEntry.nextHopARPPointer          = PRV_TGF_ARP_INDEX_CNS;

    /* must be valid eport range otherwise E2PHY table accessed in non-exists index */
    routeEntry.nextHopInterface.devPort.portNum %=
        PRV_CPSS_DXCH_MAX_PORT_NUMBER_MAC(prvTgfDevNum);


    /* AUTODOC: add UC route entry with nexthop VLAN 5 and nexthop e-port  */
    rc = prvTgfIpUcRouteEntriesWrite(PRV_TGF_ROUTER_NH_INDEX_CNS, &routeEntry, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

    /* -------------------------------------------------------------------------
     * 2. Create a new Ipv4 prefix in a Virtual Router for the specified LPM DB
     */

    /* fill a nexthop info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = PRV_TGF_ROUTER_NH_INDEX_CNS;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_E;

    cpssOsMemCpy(ipAddr.arIP, prvTgfScalableSgtPacketIpv4PartPkt4.dstAddr, sizeof(ipAddr.arIP));
    /* AUTODOC: add IPv4 UC prefix 4.4.4.4/32 */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(0, 1, ipAddr, 32,
            &nextHopInfo, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);

}
/**
* @internal prvTgfScalableSgtIpv4UcLpmRouteConfigReset function
* @endinternal
*
* @brief   Remove route entry in LPM table
*
*/
GT_VOID prvTgfScalableSgtIpv4UcLpmRouteConfigReset
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_IPADDR   ipAddr;
    /*delete route*/
    cpssOsMemCpy(ipAddr.arIP, prvTgfScalableSgtPacketIpv4PartPkt4.dstAddr, sizeof(ipAddr.arIP));
    rc = prvTgfIpLpmIpv4UcPrefixDel(0, 1, ipAddr, 32);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);
    /*restore NH*/
    rc = prvTgfIpUcRouteEntriesWrite(PRV_TGF_ROUTER_NH_INDEX_CNS, &prvTgfScalableSgtRouteEntrySave, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);
}
/**
* @internal prvTgfScalableSgtIpv4UcFdbRouteConfigSet function
* @endinternal
*
* @brief   Config route entry in FDB table
*
*            pkt_3 for UC ipv4 traffic. FDB routed
*              SIP = 2.2.2.2
*              DIP = 1.1.1.1
*              VRF = 1
*              DST intf = eport_2
*
*/
GT_VOID prvTgfScalableSgtIpv4UcFdbRouteConfigSet
(
    GT_VOID
)
{
    GT_STATUS                               rc;
    GT_ETHERADDR                            arpMacAddr;
    PRV_TGF_MAC_ENTRY_KEY_STC               entryKey;
    PRV_TGF_BRG_MAC_ENTRY_STC               macEntry;

    /* AUTODOC: write a ARP MAC 00:00:00:00:12:34 to the Router ARP Table */
    cpssOsMemCpy(arpMacAddr.arEther, prvTgfScalableSgtPacketL2PartPkt1.daMac, sizeof(TGF_MAC_ADDR));

    /* Create the Route entry (Next hop) in FDB table and Router ARP Table */
    rc = prvTgfIpRouterArpAddrRead(prvTgfDevNum, PRV_TGF_ARP_INDEX_CNS, &prvTgfScalableSgtArpMacAddrSave);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrRead: %d", prvTgfDevNum);

    rc = prvTgfIpRouterArpAddrWrite(PRV_TGF_ARP_INDEX_CNS, &arpMacAddr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /* AUTODOC: get Packet Commands for FDB routed packets */
    rc = prvTgfBrgFdbRoutingNextHopPacketCmdGet( prvTgfDevNum, &prvTgfScalableSgtNhPacketCmdSave);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingNextHopPacketCmdGet");

    /* AUTODOC: set Packet Commands for FDB routed packets */
    rc = prvTgfBrgFdbRoutingNextHopPacketCmdSet( prvTgfDevNum, CPSS_PACKET_CMD_ROUTE_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingNextHopPacketCmdSet");

    /* AUTODOC: update VRF Id to '1' in vlan 5 */
    rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_VLANID_5_CNS, 1);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanVrfIdSet: %d %d",
                             prvTgfDevNum, PRV_TGF_VLANID_5_CNS);

    /* AUTODOC: calculate index for ipv4 uc route entry in FDB */
    cpssOsMemSet(&entryKey, 0, sizeof(entryKey));
    /* fill a destination IP address for the prefix */
    cpssOsMemCpy(entryKey.key.ipv4Unicast.dip,prvTgfScalableSgtPacketIpv4Part.dstAddr , sizeof(entryKey.key.ipv4Unicast.dip));
    /* set key virtual router */
    entryKey.key.ipv4Unicast.vrfId = 1;

    /* AUTODOC: set  ipv4 uc route entry in FDB*/
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    macEntry.key.entryType = PRV_TGF_FDB_ENTRY_TYPE_IPV4_UC_E;
    cpssOsMemCpy(&macEntry.key.key.ipv4Unicast, &entryKey.key.ipv4Unicast,sizeof(macEntry.key.key.ipv4Unicast));
    macEntry.key.entryType = PRV_TGF_FDB_ENTRY_TYPE_IPV4_UC_E;
    macEntry.fdbRoutingInfo.qosPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    macEntry.fdbRoutingInfo.modifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    macEntry.fdbRoutingInfo.modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    macEntry.fdbRoutingInfo.countSet = PRV_TGF_COUNT_SET_CNS;
    macEntry.fdbRoutingInfo.trapMirrorArpBcEnable = GT_FALSE;
    macEntry.fdbRoutingInfo.dipAccessLevel = 0;
    macEntry.fdbRoutingInfo.ICMPRedirectEnable = GT_FALSE;
    macEntry.fdbRoutingInfo.mtuProfileIndex = 0;
    macEntry.fdbRoutingInfo.nextHopVlanId = PRV_TGF_VLANID_5_CNS;
    macEntry.fdbRoutingInfo.nextHopARPPointer = PRV_TGF_ARP_INDEX_CNS;
    macEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum = ((PRV_TGF_SRC_ID_BASE_CNS + 2) << PRV_TGF_SHFT_PORT_BIT) |
                                                    prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS] ;

    /* must be valid eport range otherwise E2PHY table accessed in non-exists index */
    macEntry.dstInterface.devPort.portNum %=
        PRV_CPSS_DXCH_MAX_PORT_NUMBER_MAC(prvTgfDevNum);



    prvTgfFdbIpv4UcRoutingRouteConfigurationSet(GT_FALSE, &macEntry);
}
/**
* @internal prvTgfScalableSgtIpv4UcFdbRouteConfigReset function
* @endinternal
*
* @brief   Delete route entry in FDB table
*
*/
GT_VOID prvTgfScalableSgtIpv4UcFdbRouteConfigReset
(
    GT_VOID
)
{
    PRV_TGF_MAC_ENTRY_KEY_STC macEntry;
    GT_STATUS                               rc;

    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    macEntry.entryType = PRV_TGF_FDB_ENTRY_TYPE_IPV4_UC_E;
    cpssOsMemCpy(macEntry.key.ipv4Unicast.dip, prvTgfScalableSgtPacketIpv4Part.dstAddr, sizeof(macEntry.key.ipv4Unicast.dip));
    macEntry.key.ipv4Unicast.vrfId = 1;

    rc = prvTgfBrgFdbMacEntryDelete(&macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryDelete: %d", prvTgfDevNum);

    /*remove ARP*/
    rc = prvTgfIpRouterArpAddrWrite(PRV_TGF_ARP_INDEX_CNS, &prvTgfScalableSgtArpMacAddrSave);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: %d", prvTgfDevNum);

    /*reset pkt cmd*/
    rc = prvTgfBrgFdbRoutingNextHopPacketCmdSet( prvTgfDevNum, prvTgfScalableSgtNhPacketCmdSave);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbRoutingNextHopPacketCmdSet");

}
/**
* @internal prvTgfSgtEpclSrcDstConfigSet function
* @endinternal
*
* @brief   Configure EPCL rules to qualify SRC-ID in srcTrg mode
*          EPCL lookup is based on SRC-ID as target index and
*          Action is to change DSCP of ip header to qulaify each
*          egress packet uniquely
*/
static GT_VOID prvTgfSgtEpclSrcDstConfigSet
(
    IN GT_U32                               egressPortNum,
    IN GT_BOOL                              enable
)
{
    GT_STATUS                         rc;
    PRV_TGF_PCL_RULE_FORMAT_UNT     mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT     pattern;
    PRV_TGF_PCL_ACTION_STC          action;
    PRV_TGF_PCL_LOOKUP_CFG_STC      lookupCfg;
    CPSS_INTERFACE_INFO_STC         interfaceInfo;
    GT_U32                          epclIndex;
    GT_U8                           i;
    GT_U32                          pclId = 123;

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.enableLookup           = GT_TRUE;
    lookupCfg.pclId                  = pclId;
    lookupCfg.groupKeyTypes.nonIpKey = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key  = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E;
    lookupCfg.groupKeyTypes.ipv6Key  = PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E;

    /* AUTODOC: init PCL Engine for send port*/
    /* AUTODOC:   egress direction, lookup_0 */
    /* AUTODOC:   nonIpKey EGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key EGRESS_EXT_IPV4_L4 */
    /* AUTODOC:   ipv6Key EGRESS_EXT_IPv6_L4 */
    rc = prvTgfPclDefPortInitExt2(
            egressPortNum,
            CPSS_PCL_DIRECTION_EGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            &lookupCfg);
    UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfPclDefPortInitExt2: %d, %d", prvTgfDevNum,
            egressPortNum);

    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    mask.ruleEgrExtIpv6L4.common.pclId      = 0x3FF;
    pattern.ruleEgrExtIpv6L4.common.pclId   = pclId;

    /* AUTODOC: EPCL action */
    action.egressPolicy = GT_TRUE;
    action.pktCmd       = CPSS_PACKET_CMD_FORWARD_E;
    action.qos.egressDscpCmd   = PRV_TGF_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_OUTER_E;
    interfaceInfo.type  = CPSS_INTERFACE_INDEX_E;

    /* -----------+----------+----------+-------------+----------+--------
     * Pkt Number | DstIndex | srcIndex | CFG Tbl     | ruleIndex| Action
     * -----------+----------+----------+-------------+----------+--------
     * Pkt_1      | 20       | 0        | ??????????? | 10 (60)  | Forward, dscp = 20
     * Pkt_2      | 21       | 0        | ??????????? | 11 (66)  | Forward, dscp = 21
     * Pkt_3      | 22       | 0        | ??????????? | 12 (72)  | Forward, dscp = 22
     * Pkt_4      | 23       | 0        | ??????????? | 13 (78)  | Forward, dscp = 23
     * -----------+----------+----------+-------------+----------+--------*/
    for (i=0; i<4;i++)
    {
        action.qos.dscp   = (PRV_TGF_SRC_ID_BASE_CNS + i);
        rc = prvTgfPclRuleSet(
                PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E,
                PRV_TGF_EPCL_RULE_INDEX(i),
                &mask, &pattern, &action);
        UTF_VERIFY_EQUAL0_STRING_MAC(
                GT_OK, rc, "prvTgfPclRuleSet:");

        epclIndex       = PRV_TGF_EPCL_SRC_TRG_INDEX(0, (PRV_TGF_SRC_ID_BASE_CNS + i));
        interfaceInfo.index = epclIndex;
        PRV_UTF_LOG3_MAC("interface index = 0x%x, rule index 0x%x, action 0x%x\n",
                interfaceInfo.index, epclIndex, action.pktCmd);
        rc = prvTgfPclCfgTblSet(&interfaceInfo, CPSS_PCL_DIRECTION_EGRESS_E,
                CPSS_PCL_LOOKUP_0_E, &lookupCfg);
        UTF_VERIFY_EQUAL0_STRING_MAC( GT_OK, rc, "prvTgfPclCfgTblSet");
        pattern.ruleEgrExtIpv6L4.common.pclId++;
        lookupCfg.pclId++;
    }

    /* AUTODOC: Enables egress policy on port per packet type*/
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
            egressPortNum,
            PRV_TGF_PCL_EGRESS_PKT_NON_TS_E,
            enable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: Configure src trg access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeGet(egressPortNum,
                           CPSS_PCL_DIRECTION_EGRESS_E,
                           CPSS_PCL_LOOKUP_0_E,             /* Lookup Number */
                           0,                               /* SubLookup Number */
                           &prvTgfScalableSgtCfgTabAccModeSave);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeGet");

    /* AUTODOC: Configure src trg access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(egressPortNum,
                           CPSS_PCL_DIRECTION_EGRESS_E,
                           CPSS_PCL_LOOKUP_0_E,             /* Lookup Number */
                           0,                               /* SubLookup Number */
                           PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_SRC_TRG_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");
}
/**
* @internal prvTgfSgtEpclSrcDstConfigRestore function
* @endinternal
*
* @brief   Reset the EPCL rules which qualifies SRC-ID in srcTrg mode
*/
GT_VOID prvTgfSgtEpclSrcDstConfigRestore()
{
    PRV_TGF_PCL_LOOKUP_CFG_STC      lookupCfg;
    CPSS_INTERFACE_INFO_STC         interfaceInfo;
    GT_U32                          i;
    GT_STATUS                       rc;

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.groupKeyTypes.nonIpKey = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key  = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E;
    lookupCfg.groupKeyTypes.ipv6Key  = PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E;

    interfaceInfo.type  = CPSS_INTERFACE_INDEX_E;

    for (i=0; i<4;i++)
    {
        rc = prvTgfPclRuleValidStatusSet(
                CPSS_PCL_RULE_SIZE_STD_E,
                PRV_TGF_EPCL_RULE_INDEX(i),
                GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: rule %d", i);

        interfaceInfo.index = PRV_TGF_EPCL_SRC_TRG_INDEX(0, (PRV_TGF_SRC_ID_BASE_CNS + i));
        rc = prvTgfPclCfgTblSet(&interfaceInfo, CPSS_PCL_DIRECTION_EGRESS_E,
                CPSS_PCL_LOOKUP_0_E, &lookupCfg);
        UTF_VERIFY_EQUAL0_STRING_MAC( GT_OK, rc, "prvTgfPclCfgTblSet");
    }

    /* AUTODOC: Restore access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_EGRESS_E,
            CPSS_PCL_LOOKUP_0_E,             /* Lookup Number */
            0,                               /* SubLookup Number */
            prvTgfScalableSgtCfgTabAccModeSave);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

    prvTgfPclPortsRestoreAll();
}

/**
* @internal prvTgfBrgSrcIdScalableSgtConfigSet function
* @endinternal
*
* @brief   config
*           eport to physical port map
*           scalable SGT support register for mux bits
*           HA phy port table with SRC-ID
*           EPCL rules for mode SRC_TRG
*/
GT_VOID prvTgfBrgSrcIdScalableSgtConfigSet()
{
    CPSS_DXCH_BRG_SRC_ID_TARGET_INDEX_CONFIG_STC config;
    GT_U32 targetId;
    GT_STATUS rc;
    GT_U32 trgEport;

    rc = cpssDxChBrgSrcIdTargetIndexConfigGet(prvTgfDevNum, &prvTgfScalableSgtTargetConfigSave);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgSrcIdTargetIndexConfigGet");

    cpssOsMemSet(&config, 0, sizeof(config));
    config.srcIdMsb          = PRV_TGF_SRC_ID_MSB;
    config.srcIdLength       = PRV_TGF_SRC_ID_LEN;
    config.trgEportMuxLength = PRV_TGF_EPORT_MUX_LEN_CNS;
    config.trgDevMuxLength = 0;

    rc = cpssDxChBrgSrcIdTargetIndexConfigSet(prvTgfDevNum, &config);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgSrcIdTargetIndexConfigSet");

    targetId = PRV_TGF_SRC_ID_BASE_CNS << (config.srcIdMsb +1 - config.srcIdLength) ;
    rc = cpssDxChBrgSrcIdPortTargetConfigGet(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], &prvTgfScalableSgtEnableSave, &prvTgfScalableSgtTargetIdSave);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgSrcIdPortTargetConfigGet");

    rc = cpssDxChBrgSrcIdPortTargetConfigSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], GT_TRUE, targetId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgSrcIdPortTargetConfigSet");

    trgEport = ((PRV_TGF_SRC_ID_BASE_CNS + 1) << PRV_TGF_SHFT_PORT_BIT) | prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
    /* must be valid eport range otherwise E2PHY table accessed in non-exists index */
    trgEport %=
        PRV_CPSS_DXCH_MAX_PORT_NUMBER_MAC(prvTgfDevNum);

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:11, VLAN 5, port 2 */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfScalableSgtPacketL2PartPkt2.daMac, PRV_TGF_VLANID_5_CNS, prvTgfDevNum,
            trgEport, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);

    /* AUTODOC: add EPCL rules for port 2 */
    prvTgfSgtEpclSrcDstConfigSet( prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], GT_TRUE);

    if(PRV_CPSS_SIP_6_30_CHECK_MAC(prvTgfDevNum))
    {
        rc = internal_cpssDxChCfgEpclTrgPortModeSet(prvTgfDevNum,1/*Set the SRC_ID+ desc<copy_reserved> mode*/);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "internal_cpssDxChCfgEpclTrgPortModeSet");
    }
}
/**
* @internal prvTgfBrgSrcIdScalableSgtConfigReset function
* @endinternal
*
* @brief   Reset config for scalable SGT feature
*          Actions:
*           1. Remove FDB entries
*           2. Remove srcTrg EPCL rules
*/
GT_VOID prvTgfBrgSrcIdScalableSgtConfigReset()
{
    GT_STATUS rc;

    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush");

    if(PRV_CPSS_SIP_6_30_CHECK_MAC(prvTgfDevNum))
    {
        rc = internal_cpssDxChCfgEpclTrgPortModeSet(prvTgfDevNum,0/* restore the 'EPG mode'*/);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "internal_cpssDxChCfgEpclTrgPortModeSet");
    }

    rc = cpssDxChBrgSrcIdPortTargetConfigSet(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS], prvTgfScalableSgtEnableSave, prvTgfScalableSgtTargetIdSave);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgSrcIdPortTargetConfigSet");

    rc = cpssDxChBrgSrcIdTargetIndexConfigSet(prvTgfDevNum, &prvTgfScalableSgtTargetConfigSave);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgSrcIdTargetIndexConfigSet");

    prvTgfSgtEpclSrcDstConfigRestore();
    prvTgfScalableSgtIpv4UcLpmRouteConfigReset();
    prvTgfScalableSgtIpv4UcFdbRouteConfigReset();
}

/**
* @internal prvTgfBrgSrcIdScalableSgtTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic for scalable SGT feature
*           1. pkt_1 for unknown L2 traffic
*              SA = 00:00:00:00:00:01
*              DA = 00:00:00:00:12:34
*           2. pkt_2 for known UC L2 traffic
*              SA = 00:00:00:00:00:01
*              DA = 00:00:00:00:00:11
*           3. pkt_3 for UC ipv4 traffic. FDB routed
*              SA = 00:00:00:00:00:01
*              DA = 00:00:00:00:34:02
*              SIP = 2.2.2.2
*              DIP = 1.1.1.1
*              VRF = 1
*           4. pkt_3 for UC ipv4 traffic. FDB routed
*              SA = 00:00:00:00:00:01
*              DA = 00:00:00:00:34:02
*              SIP = 2.2.2.2
*              DIP = 4.4.4.4
*              VRF = 1
*/
GT_VOID prvTgfBrgSrcIdScalableSgtTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS             rc;
    GT_U8                 packetBuf[256];
    GT_U32                packetLen = sizeof(packetBuf);
    GT_U32                packetActualLength = 0;
    GT_U8                 queue = 0;
    GT_U8                 dev = 0;
    GT_U8                 pktCnt = 0, dscp;
    TGF_NET_DSA_STC       rxParam;
    IN GT_PORT_NUM        inPortId     = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    IN GT_PORT_NUM        outPortId    = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
    CPSS_INTERFACE_INFO_STC      egressPortInterface;
    GT_BOOL                 getFirst = GT_TRUE;

    /* Update egress portInterface for capturing */
    egressPortInterface.type              = CPSS_INTERFACE_PORT_E;
    egressPortInterface.devPort.hwDevNum  = prvTgfDevNum;
    egressPortInterface.devPort.portNum   = outPortId;

    /* AUTODOC: setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfoScalableSgt, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup:");

    for (pktCnt=0; pktCnt< 4; pktCnt++)
    {

        /* AUTODOC: update Packet */
        switch(pktCnt)
        {
            case 0:
                /* AUTODOC: send Packet-1
                   unknown UC L2 pkt*/
                prvTgfPacketScalableSgtPartArray[0].partPtr = &prvTgfScalableSgtPacketL2PartPkt1;
                break;
            case 1:
                /* AUTODOC: send Packet-2
                   known UC L2 pkt*/
                prvTgfPacketScalableSgtPartArray[0].partPtr = &prvTgfScalableSgtPacketL2PartPkt2;
                break;
            case 2:
                /* AUTODOC: send Packet-3
                   UC ipv4 pkt in FDB*/
                prvTgfPacketScalableSgtPartArray[0].partPtr = &prvTgfScalableSgtPacketL2Part;
                prvTgfPacketScalableSgtPartArray[3].partPtr = &prvTgfScalableSgtPacketIpv4Part;
                break;
            case 3:
                /* AUTODOC: send Packet-4
                   UC ipv4 pkt in LPM*/
                prvTgfPacketScalableSgtPartArray[0].partPtr = &prvTgfScalableSgtPacketL2Part;
                prvTgfPacketScalableSgtPartArray[3].partPtr = &prvTgfScalableSgtPacketIpv4PartPkt4;
                break;
        }

    /* AUTODOC: enable capture on egress port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&egressPortInterface,
            TGF_CAPTURE_MODE_MIRRORING_E,
            GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "Failure in tgftrafficgeneratorporttxethcaptureset port - %d",
            egressPortInterface.devPort.portNum);

    /* AUTODOC: Start pkt TX on ingress port */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, inPortId);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                prvTgfDevNum, inPortId);

    /* AUTODOC: Disable capture on egress */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&egressPortInterface,
            TGF_CAPTURE_MODE_MIRRORING_E,
            GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "Failure in tgftrafficgeneratorporttxethcaptureset port - %d",
            egressPortInterface.devPort.portNum);

    /* AUTODOC: Get the first rx pkt on egress port */
        rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&egressPortInterface,
                TGF_PACKET_TYPE_CAPTURE_E,
                getFirst, GT_TRUE, packetBuf,
                &packetLen, &packetActualLength,
                &dev, &queue, &rxParam);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                "Error: failed capture on inPortId %d outPortId %d \n", inPortId, outPortId);
        getFirst = GT_FALSE;

        dscp = packetBuf[TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + 1] >> 2;
        /*validate captured pkt*/
        UTF_VERIFY_EQUAL2_STRING_MAC(PRV_TGF_SRC_ID_BASE_CNS + pktCnt,dscp,
                                     "rx DSCP[0x%x] != exp DSCP[0x%x]\n", dscp, PRV_TGF_SRC_ID_BASE_CNS + pktCnt);
    }
}
