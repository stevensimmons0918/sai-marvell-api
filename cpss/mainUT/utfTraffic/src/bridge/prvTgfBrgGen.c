/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfFdbAging.c
*
* DESCRIPTION:
*       Bridge Generic APIs UT.
*
* FILE REVISION NUMBER:
*       $Revision: 20 $
*
*******************************************************************************/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <bridge/prvTgfBrgGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfCscdGen.h>

#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <extUtils/trafficEngine/private/prvTgfTrafficParser.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgE2Phy.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdb.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgSrcId.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN0 Id */
#define PRV_TGF_VLANID_CNS  5
/* the vlan ID that the from CPU uses ... WA to Erratum of device ... */
#define FROM_CPU_VLANID_CNS 4094

/* number of fdb upload messages */
#define PRV_TGF_FDB_UPLOAD_MESSAGE_CNS 4

/* we have erratum : The ‘FROM_CPU’ packet is subject to ‘L2i filters’ (on my case ‘vlan not valid’) when “Bridge Bypass Mode" is in mode : "Bypass Forwarding Decision only"*/
static GT_BOOL isErratumForFromCpuForBridgeBypassForwardingDecisionOnlyMode = GT_TRUE;

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/******************************* Test packets **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x01, 0x80, 0xc2, 0x00, 0x00, 0x00},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                /* srcMac */
};

static TGF_PACKET_L2_STC prvTgfBypassPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0xBB},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0xAA}                 /* saMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};
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
    0xFF,               /* protocol */
    0x79D4,             /* csum */
    { 0,  0,  0,  0},   /* srcAddr */
    { 0,  0,  0,  2}    /* dstAddr */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* DATA of bypass packet */
static GT_U8 prvTgfBypassPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f
};

/* Bypass PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfBypassPacketPayloadPart = {
    sizeof(prvTgfBypassPayloadDataArr),                       /* dataLength */
    prvTgfBypassPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* PARTS of Bypass packet */
static TGF_PACKET_PART_STC prvTgfBypassPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfBypassPacketL2Part}, /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfBypassPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr))

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  (PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS)

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo = {
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/* LENGTH of bypass packet */
#define PRV_TGF_BYPASS_PACKET_LEN_CNS \
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfBypassPayloadDataArr))

/* LENGTH of packet with CRC */
#define PRV_TGF_BYPASS_PACKET_CRC_LEN_CNS  (PRV_TGF_BYPASS_PACKET_LEN_CNS + TGF_CRC_LEN_CNS)

static TGF_PACKET_STC prvTgfBypassPacketInfo = {
    PRV_TGF_BYPASS_PACKET_CRC_LEN_CNS,                                   /* totalLen */
    sizeof(prvTgfBypassPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfBypassPacketPartArray                                        /* partsArray */
};

/******************************* IP MC to CPU test case **********************************/
/* ports */
#define PRV_TGF_RX_PORT_IDX_CNS       0
#define PRV_TGF_TX_PORT_IDX_CNS       1
#define NOT_VALID_CNS   0xFFFFFFFF
/******************************* Test packet **********************************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketBrgIpMcL2Part =
{
    {0x01, 0x00, 0x5e, 0x00, 0x00, 0x01},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x88}                /* saMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketBrgIpMcVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketBrgIpMcEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketBrgIpMcIpv4Part =
{
    4,                                 /* version */
    5,                                 /* headerLen */
    0,                                 /* typeOfService */
    0x2A,                              /* totalLen */
    0,                                 /* id */
    0,                                 /* flags */
    0,                                 /* offset */
    0x40,                              /* timeToLive */
    0xff,                              /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    {1, 0, 0, 1},                      /* srcAddr */
    {0xE0, 0, 0, 2}                    /* dstAddr */
};

/* DATA of packet */
static GT_U8 prvTgfPacketBrgIpMcPayloadDataArr[] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07, 0x08, 0x09
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketBrgIpMcPayloadPart = {
    sizeof(prvTgfPacketBrgIpMcPayloadDataArr),                       /* dataLength */
    prvTgfPacketBrgIpMcPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketBrgIpMcPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketBrgIpMcL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketBrgIpMcVlanTag0Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketBrgIpMcEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketBrgIpMcIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketBrgIpMcPayloadPart}
};

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketBrgIpMcInfo = {
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketBrgIpMcPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketBrgIpMcPartArray                                        /* partsArray */
};

static struct
{
    CPSS_PACKET_CMD_ENT                   ipv4McPortRoutingEnableGet;
    GT_BOOL                               globalMcMirrorEnable;
    CPSS_PACKET_CMD_ENT                   protoMcMirrorCmd;
    CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT        protocol;
    CPSS_FDB_ACTION_MODE_ENT              fdbActionModeGet;
    GT_BOOL                               fdbUploadEnableGet;
    CPSS_MAC_ACTION_MODE_ENT              macActionModeGet;
}prvTgfRestoreCfg;
/**********************************************************************/
/* TC for assinging eport attr locally*/
/**********************************************************************/
#define PRV_TGF_BRG_VLAN_PORT_TPID_ENTRY_ID_CNS 1
#define PRV_TGF_BRG_VLAN_PORT_TPID_ETHER_TYPE_CNS 0xABCD
#define PRV_TGF_TARGET_EPORT_CNS        1000
/* final target device & port */
#define PRV_TGF_DSA_DEV_CNS    765
#define PRV_TGF_OWN_DEV_CNS    763
#define PRV_TGF_DSA_PORT_CNS    0x01 /*dummy*/

/******************************* Test packet **********************************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketBrgLocalAssignL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* saMac */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketBrgLocalAssignEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* DATA of packet */
static GT_U8 prvTgfPacketBrgLocalAssignPayloadDataArr[] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07, 0x08, 0x09
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketBrgLocalAssignPayloadPart = {
    sizeof(prvTgfPacketBrgLocalAssignPayloadDataArr),                       /* dataLength */
    prvTgfPacketBrgLocalAssignPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketBrgLocalAssignPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketBrgLocalAssignL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketBrgIpMcVlanTag0Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketBrgLocalAssignEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketBrgLocalAssignPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_LOCAL_ASSGN_PACKET_LEN_CNS \
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + sizeof(prvTgfPacketBrgLocalAssignPayloadDataArr))

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketBrgLocalAssignInfo = {
    PRV_TGF_LOCAL_ASSGN_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketBrgLocalAssignPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketBrgLocalAssignPartArray                                        /* partsArray */
};

#define PRV_TGF_SRC_EPG_CNS 0x1
#define PRV_TGF_DST_EPG_CNS 0x2
#define PRV_TGF_EPG_DSCP_CNS 25
#define PRV_TGF_EPCL_RULE_INDEX(_index)      (prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(10 + _index))
static struct
{
   CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_ENT fdbMacMuxMode;
   CPSS_BRG_SRC_ID_ASSIGN_MODE_ENT brgSrcIdAssignMode;
   CPSS_DXCH_BRG_FDB_EPG_CONFIG_STC epgConfig; 
   PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT pclPortAccessModeCfgGet;
}prvTgfEpgRestoreCfg;
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfBrgFdbEpgPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* srcMac */
};
/* VLAN_TAG part - Tag0 */
static TGF_PACKET_VLAN_TAG_STC prvTgfBrgFdbEpgPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS/* pri, cfi, VlanId */
};
/* IPv4 packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfBrgFdbEpgPacketEtherTypePart_Ipv4 = {
    TGF_ETHERTYPE_0800_IPV4_TAG_CNS
};
static TGF_PACKET_IPV4_STC prvTgfBrgFdbEpgPacketIpv4Part = {
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
static GT_U8 prvTgfBrgFdbEpgPayloadDataArr[] = {
    0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc,
    0xdd, 0xee, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc,
    0xdd, 0xee, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc,
    0xdd, 0xee, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfBrgFdbEpgPacketPayloadPart = {
    sizeof(prvTgfBrgFdbEpgPayloadDataArr),                       /* dataLength */
    prvTgfBrgFdbEpgPayloadDataArr                                /* dataPtr */
};
static TGF_PACKET_PART_STC prvTgfBrgFdbEpgPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfBrgFdbEpgPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfBrgFdbEpgPacketVlanTag0Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfBrgFdbEpgPacketEtherTypePart_Ipv4},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfBrgFdbEpgPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfBrgFdbEpgPacketPayloadPart}
};

/* TAG0  tagged PACKET to send */
static TGF_PACKET_STC prvTgfBrgFdbEpgPacketInfo =
{
    TGF_L2_HEADER_SIZE_CNS +
    TGF_VLAN_TAG_SIZE_CNS +
    TGF_ETHERTYPE_SIZE_CNS +
    TGF_IPV4_HEADER_SIZE_CNS +
    sizeof(prvTgfBrgFdbEpgPayloadDataArr),       /* totalLen */
    sizeof(prvTgfBrgFdbEpgPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfBrgFdbEpgPartArray                                       /* partsArray */
};
/* PCL UDB configuration */
typedef struct{
    GT_U32                               udbIndex;
    CPSS_DXCH_PCL_OFFSET_TYPE_ENT        offsetType;
    GT_U8                                offset;
    GT_U8                                byteMask;
}prvTgfPclUdbEpg;
static prvTgfPclUdbEpg prvTgfEpgEpclMetadataUdbInfo[] = 
{
     {0 , CPSS_DXCH_PCL_OFFSET_METADATA_E, 88, 0xFC }, /* Packet Source Group ID[1:9] = PCL_MD_88[2:7] + PCL_MD_89[2:0] */
     {1 , CPSS_DXCH_PCL_OFFSET_METADATA_E, 89, 0xFF }, /* Packet Destination Group ID[1:9] = PCL_MD_89[3:7] + PCL_MD_90[3:0] */
     {2 , CPSS_DXCH_PCL_OFFSET_METADATA_E, 90, 0x0F },
};
static GT_U16 prvTgfBrgFdbUploadVlanId[4] = {100, 250, 50, 150};
static GT_ETHERADDR prvTgfBrgFdbUploadMacAddr[2] = {
                                           {{0x0, 0x1A, 0x2B, 0x3C, 0x4D, 0x5E}},
                                           {{0x0, 0x1A, 0x2B, 0x3C, 0x4D, 0x5F}}
                                          };
static CPSS_INTERFACE_TYPE_ENT prvTgfBrgFdbUploadDstInterface[4] = {CPSS_INTERFACE_TRUNK_E, CPSS_INTERFACE_PORT_E,
                                                           CPSS_INTERFACE_VID_E, CPSS_INTERFACE_VIDX_E};
static CPSS_MAC_ENTRY_EXT_TYPE_ENT prvTgfBrgFdbUploadMacEntryType[4] = {CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E, CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E,
                                                               CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E, CPSS_MAC_ENTRY_EXT_TYPE_IPV4_MCAST_E};
static GT_U16 prvTgfBrgFdbUploadTrunkId[4]             = {10, 0, 0, 0};
static GT_U32 prvTgfBrgFdbUploadPortNum[4]             = {0, 25, 0, 0};
static GT_U16 prvTgfBrgFdbUploadVidx[4]                = {0, 0, 10, 15};
static GT_U8  prvTgfBrgFdbUploadVidInterfaceSip[4]     = {224, 12, 22, 3};
static GT_U8  prvTgfBrgFdbUploadVidxInterfaceSip[4]    = {224, 200, 100,25};
static GT_U8  prvTgfBrgFdbUploadVidInterfaceDip[4]     = {123, 34, 67, 8};
static GT_U8  prvTgfBrgFdbUploadVidxInterfaceDip[4]    = {111, 11, 17, 19};
static GT_U32 prvTgfBrgFdbUploadSourceId[4]           = {1, 4, 5, 10};
static GT_BOOL prvTgfBrgFdbUploadAge[4]                = {GT_TRUE, GT_FALSE, GT_TRUE, GT_FALSE};
static GT_BOOL prvTgfBrgFdbUploadSpUnknown[4]          = {GT_FALSE, GT_TRUE, GT_FALSE, GT_TRUE};

static struct
{
    CPSS_CSCD_LINK_TYPE_STC     cascadeLink;
    PRV_TGF_CSCD_TRUNK_LINK_HASH_ENT srcPortTrunkHashEn;
    GT_BOOL                     localAssignEnable;
    CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_ENT vlanCmd;
    GT_U16                      etherType;
    GT_U32                      tpidEntryIndex;
    GT_HW_DEV_NUM               savedDevNum;
}prvTgfEportLocalAssignRestore;

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/
/**
* @internal prvTgfBrgGenIeeeReservedMcastProfilesTestInit function
* @endinternal
*
* @brief   Set VLAN entry.
*
* @param[in] vlanId                   -  to be configured
*                                       None
*/
static GT_VOID prvTgfBrgGenIeeeReservedMcastProfilesTestInit
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
    vlanInfo.fidValue              = vlanId;

    /* set vlan entry */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    portCount = prvTgfPortsNum;
    for (portIter = 0; portIter < portCount; portIter++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers,prvTgfPortsArray[portIter]);

        if(vlanId == FROM_CPU_VLANID_CNS)
        {
            /* as this vlan is usually never valid , we unaware that if valid we
               need to allow the tag after the DSA to egress the device ...
               tagged/untagged packet */
            portsTaggingCmd.portsCmd[prvTgfPortsArray[portIter]] = PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E;
        }
        else
        {
            portsTaggingCmd.portsCmd[prvTgfPortsArray[portIter]] = PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E;
        }
        /* reset counters and set force link up */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum,
                                 vlanId,
                                 &portsMembers,
                                 &portsTagging,
                                 &vlanInfo,
                                 &portsTaggingCmd);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d", vlanId);
}

/**
* @internal prvTgfBrgGenIeeeReservedMcastProfilesTestPacketSend function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*
* @param[in] portNum                  - port number
* @param[in] notIngressPort           - VLAN member but not ingress port
*                                       None
*/
static GT_VOID prvTgfBrgGenIeeeReservedMcastProfilesTestPacketSend
(
    IN GT_U32 portNum,
    IN GT_U32 notIngressPort,
    IN GT_U32 expectPacketNum
)
{
    GT_STATUS       rc = GT_OK;
    GT_U8           devNum   = 0;
    GT_U8           queue    = 0;
    GT_BOOL         getFirst = GT_TRUE;
    GT_U32          buffLen  = 0x600;
    GT_U32          packetActualLength = 0;
    static  GT_U8   packetBuff[0x600] = {0};
    TGF_NET_DSA_STC rxParam;
    CPSS_INTERFACE_INFO_STC portInterface;

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the ethertype
      of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, NULL);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* setup portInterface for capturing.
      there is no packets should be captured on this port.
      because packet may be trapped or dropped on ingress one. */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = notIngressPort;

    /* enable capture on port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, portNum);

    /* send Packet from port 0 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, portNum);

    /* disable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, portNum);

    /* Get entry from captured packet's table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       getFirst, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue,
                                       &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_PACKET_CRC_LEN_CNS * expectPacketNum, packetActualLength, "Number of expected packets is wrong\n");

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/
/**
* @internal prvTgfBrgGenIeeeReservedMcastProfilesConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Create VLAN 2 on all ports (0,8,18,23)
*         Set capture to CPU
*         BridgingModeSet = CPSS_BRG_MODE_802_1Q_E
*         GenIeeeReservedMcastTrapEnable = GT_TRUE
*         For all ports (0,8,18,23):
*         For all profileIndex (0...3):
*         IeeeReservedMcastProfileIndexSet (devNum = 0,
*         portNum,
*         profileIndex)
*         For all protocols (0...255)
*         IeeeReservedMcastProtCmdSet1 (dev = 0,
*         profileIndex,
*         protocol_iterator,
*         CPSS_PACKET_CMD_TRAP_TO_CPU_E)
*         Send Packet from to portNum.
*         IeeeReservedMcastProtCmdSet1 (dev = 0,
*         profileIndex,
*         protocol_iterator,
*         CPSS_PACKET_CMD_DROP_SOFT_E)
*         Success Criteria:
*         Fisrt packet is not captured on ports 0,8,18,23 but is captured in CPU.
*         Second packet is dropped.
*/
GT_VOID prvTgfBrgGenIeeeReservedMcastProfilesConfigurationSet
(
    GT_VOID
)
{
    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: configure VLAN-aware mode */
    prvTgfBrgVlanBridgingModeSet(CPSS_BRG_MODE_802_1Q_E);

    /* AUTODOC: enable IEEE Reserved Mcast trap */
    prvTgfBrgGenIeeeReservedMcastTrapEnable(prvTgfDevNum, GT_TRUE);

    /* AUTODOC: create VLAN 2 with port 0,1,2,3 */
    prvTgfBrgGenIeeeReservedMcastProfilesTestInit(PRV_TGF_VLANID_CNS);
}

/**
* @internal prvTgfBrgGenIeeeReservedMcastProfilesTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to all ports the following IP4 packet:
*         000000   01 80 c2 00 00 03 00 00 00 00 00 11 81 00 00 02
*         000010   08 00 45 00 00 2a 00 00 00 00 40 ff 79 d4 00 00
*         000020   00 00 00 00 00 02 00 01 02 03 04 05 06 07 08 09
*         000030   0a 0b 0c 0d 0e 0f 10 11 12 13 14 15
*/
GT_VOID prvTgfBrgGenIeeeReservedMcastProfilesTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS rc;           /* returned status */
    GT_U8  portIter;        /* loop port iterator */
    GT_U32 protocolIter;    /* loop protocol iterator */
    GT_U32 profileIndex;    /* loop profile iterator */
    GT_U32  notIngressPort;  /* not ingress port */
    GT_U32 numOfProfiles;   /* number of profiles that the device supports : DX - 4 , Puma - 2 */
    GT_U32 numOfRanges;     /* number of ranges that the device supports :
                                DX - single range : 01-80-C2-00-00-00 - 01-80-C2-00-00-FF
                                Puma - 2 ranges :  01-80-C2-00-00-00 - 01-80-C2-00-00-0F
                                                   01-80-C2-00-00-20 - 01-80-C2-00-00-2F
                                */
    GT_U32 rangeIndex;     /* range index */
    GT_U32 protocolIterIncrement;/* increment value on each protocol iteration */
    GT_U32  startProtocol[2] = {0};/* first protocol number in each range */
    GT_U32  protocolsNum[2] = {0};/* number of protocols in each range */


    if(CPSS_IS_DXCH_FAMILY_MAC(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily))
    {
        numOfProfiles = 4;
        protocolIterIncrement = 0x20;
        numOfRanges  = 1;
        startProtocol[0] = 0;       /* 01-80-C2-00-00-00 - 01-80-C2-00-00-FF*/
        protocolsNum[0] = 0x100;
    }
    else/* Puma */
    {
        numOfProfiles = 2;
        protocolIterIncrement = 2;

        numOfRanges  = 2;
        startProtocol[0] = 0;       /* 01-80-C2-00-00-00 - 01-80-C2-00-00-0F*/
        protocolsNum[0] = 0x10;
        startProtocol[1] = 0x20;    /*01-80-C2-00-00-20 - 01-80-C2-00-00-2F*/
        protocolsNum[1] = 0x10;
    }

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: for all ports 0,1,2,3 do: */
#ifdef GM_USED
    portIter = 2;/* no loops on ports. --> reduce time */
    protocolIterIncrement *= 5;/* reduce time */
#else
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
#endif
    {
        /* use another port to check that packet was dropped (not forwarded ) */
        notIngressPort = prvTgfPortsArray[(portIter + 1)%prvTgfPortsNum];

        /* AUTODOC: for all profile Idx (0..3) do: */
#ifdef GM_USED
        profileIndex = numOfProfiles - 1;/* no loops on profiles. --> reduce time */
#else
        for(profileIndex = 0; profileIndex < numOfProfiles; profileIndex++)
#endif
        {
            /* AUTODOC: set profile indexes: */
            /* AUTODOC:   port 0 -> index (0,1,2,3) */
            /* AUTODOC:   port 1 -> index (0,1,2,3) */
            /* AUTODOC:   port 2 -> index (0,1,2,3) */
            /* AUTODOC:   port 3 -> index (0,1,2,3) */
            prvTgfBrgGenPortIeeeReservedMcastProfileIndexSet(prvTgfDevNum,
                                                             prvTgfPortsArray[portIter],
                                                             profileIndex);

            for(rangeIndex = 0; rangeIndex < numOfRanges; rangeIndex++)
            {
                for(protocolIter = startProtocol[rangeIndex];
                    protocolIter < (startProtocol[rangeIndex] + protocolsNum[rangeIndex]);
                    protocolIter += protocolIterIncrement)
                {
                    /* skip protocol 1 (PAUSE frame) */
                    if (protocolIter == 1)
                    {
                        continue;
                    }

                    /* AUTODOC: for each profile idx enable trap to the CPU IEEE reserved MC addresses in range 01:80:C2:00:00:[00..FF] */
                    prvTgfBrgGenIeeeReservedMcastProtCmdSet(prvTgfDevNum,
                                                            profileIndex,
                                                            (GT_U8)protocolIter,
                                                            CPSS_PACKET_CMD_TRAP_TO_CPU_E);

                    prvTgfPacketL2Part.daMac[5] = (GT_U8)protocolIter;
                    utfPrintKeepAlive();

                    /* AUTODOC: send IPv4 packets with DAs 01:80:C2:00:00:00..01:80:C2:00:00:FF from ports 0, 1, 2, 3 */
                    /* AUTODOC: verify packet is captured on CPU */
                    prvTgfBrgGenIeeeReservedMcastProfilesTestPacketSend(prvTgfPortsArray[portIter], notIngressPort, 1);


                    /* AUTODOC: set SOFT DROP command */
                    prvTgfBrgGenIeeeReservedMcastProtCmdSet(prvTgfDevNum,
                                                            profileIndex,
                                                            (GT_U8)protocolIter,
                                                            CPSS_PACKET_CMD_DROP_SOFT_E);

                    /* reset counters */
                    rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                                 prvTgfDevNum, prvTgfPortsArray[portIter]);

                    /* AUTODOC: send IPv4 packets with DAs 01:80:C2:00:00:00..01:80:C2:00:00:FF from port 0 */
                    /* AUTODOC: verify packet is not captured but dropped */
                    prvTgfBrgGenIeeeReservedMcastProfilesTestPacketSend(prvTgfPortsArray[portIter], notIngressPort, 0);

                    /* reset counters */
                    rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                                 prvTgfDevNum, prvTgfPortsArray[portIter]);
                }/*protocolIter*/
            }/*rangeIndex*/
        }/*profileIndex*/
    }/*portIter*/
}

/**
* @internal prvTgfBrgGenIeeeReservedMcastProfilesConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgGenIeeeReservedMcastProfilesConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /****************/
    /* Tables Reset */
    /****************/

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* AUTODOC: disable IEEE Reserved Mcast trap */
    prvTgfBrgGenIeeeReservedMcastTrapEnable(prvTgfDevNum, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgGenIeeeReservedMcastTrapEnable: %d",
                                 prvTgfDevNum);
}

/******************************************************************************\
 *                            Private variables                               *
\******************************************************************************/

/* traffic generation sending port */
#define ING_PORT_IDX_CNS    1

/* PCL target port */
#define EGR_PCL_PORT_IDX_CNS    0

/* FDB target port */
#define EGR_FDB_PORT_IDX_CNS    2

/* PCL rule index */
static GT_U32   prvTgfPclRuleIndex = 1;

/* Save Bridge Bypass Mode for restore */
static PRV_TGF_BRG_GEN_BYPASS_MODE_ENT  prvTgfBridgeBypassMode;

/* Save Per Source Physical Port Bridge Bypass Mode for restore */
static PRV_TGF_BRG_GEN_BYPASS_MODE_ENT  prvTgfBridgePortBypassMode;


/**
* @internal prvTgfTrafficPclRuleAndActionSet function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfTrafficPclRuleAndActionSet
(
    IN    GT_BOOL                       bypassBridgeEn
)
{
    PRV_TGF_PCL_RULE_FORMAT_UNT mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT pattern;
    PRV_TGF_PCL_ACTION_STC      action;

    /* mask for MAC address */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(mask.ruleStdNotIp.macDa.arEther, 0xFF,
                 sizeof(mask.ruleStdNotIp.macDa.arEther));

    /* pattern for MAC address */
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemCpy(pattern.ruleStdNotIp.macDa.arEther, prvTgfBypassPacketL2Part.daMac,
                 sizeof(prvTgfBypassPacketL2Part.daMac));

    /* action redirect */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd                                           = PRV_TGF_PACKET_CMD_FORWARD_E;
    action.mirror.cpuCode = 0xf8;
    action.redirect.redirectCmd                             = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    action.redirect.data.outIf.outInterface.type            = CPSS_INTERFACE_PORT_E;
    action.redirect.data.outIf.outInterface.devPort.hwDevNum  = prvTgfDevNum;
    action.redirect.data.outIf.outInterface.devPort.portNum =
        prvTgfPortsArray[EGR_PCL_PORT_IDX_CNS];

    if( GT_TRUE == bypassBridgeEn )
    {
        action.bypassBridge = GT_TRUE;
    }
    else
    {
        action.bypassBridge = GT_FALSE;
    }

    return prvTgfPclRuleSet(
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
            prvTgfPclRuleIndex, &mask, &pattern, &action);
}

/**
* @internal prvTgfFdbMacLearningCheck function
* @endinternal
*
* @brief   None
*/
static GT_STATUS prvTgfFdbMacLearningCheck
(
    IN TGF_MAC_ADDR macAddrArr
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_BRG_MAC_ENTRY_STC     macEntry;
    PRV_TGF_MAC_ENTRY_KEY_STC     macEntryKey;

    /* get FDB entry */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemSet(&macEntryKey, 0, sizeof(macEntryKey));
    macEntryKey.entryType = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntryKey.key.macVlan.vlanId = PRV_TGF_VLANID_CNS;

    cpssOsMemCpy(macEntryKey.key.macVlan.macAddr.arEther, macAddrArr, sizeof(TGF_MAC_ADDR));

    rc = prvTgfBrgFdbMacEntryGet(&macEntryKey, &macEntry);

    /* Checking if there is MAC learning */
    if (rc != GT_OK)
    {
        PRV_UTF_LOG6_MAC("Device has NOT learned MAC SA %02X:%02X:%02X:%02X:%02X:%02X\n",
                macAddrArr[0],macAddrArr[1],macAddrArr[2],macAddrArr[3],macAddrArr[4],macAddrArr[5]);
        return GT_FAIL;
    }

    /* compare entryType */
    rc = (macEntry.key.entryType == PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E) ? GT_OK : GT_FAIL;
    PRV_UTF_VERIFY_GT_OK(rc, "(macEntry.key.entryType != MAC_ADDR_E");

    /* compare MAC address */
    rc = cpssOsMemCmp(macEntry.key.key.macVlan.macAddr.arEther,
                      macEntryKey.key.macVlan.macAddr.arEther,
                      sizeof(TGF_MAC_ADDR)) == 0 ? GT_OK : GT_FAIL;
    PRV_UTF_VERIFY_GT_OK(rc, "FDB has another MAC address");

    /* compare vlanId */
    rc = (macEntry.key.key.macVlan.vlanId == macEntryKey.key.macVlan.vlanId) ? GT_OK : GT_FAIL;
    PRV_UTF_VERIFY_GT_OK(rc, "FDB has another vlanid");

    /* compare dstInterface.type */
    rc = (macEntry.dstInterface.type == CPSS_INTERFACE_PORT_E) ? GT_OK : GT_FAIL;
    PRV_UTF_VERIFY_GT_OK(rc, "FDB has another dstInterface.type");

    /* compare dstInterface */
    if (macEntry.dstInterface.type == CPSS_INTERFACE_PORT_E) {
        rc = (macEntry.dstInterface.devPort.hwDevNum == prvTgfDevNum) ? GT_OK : GT_FAIL;
        PRV_UTF_VERIFY_GT_OK(rc, "FDB has another dstInterface.devPort.hwDevNum");
    }

    /* compare isStatic */
    rc = (macEntry.isStatic == GT_FALSE) ? GT_OK : GT_FAIL;
    if (rc == GT_OK)
        PRV_UTF_LOG6_MAC("Device has learned MAC SA %02X:%02X:%02X:%02X:%02X:%02X\n",
                macAddrArr[0],macAddrArr[1],macAddrArr[2],macAddrArr[3],macAddrArr[4],macAddrArr[5]);

    return rc;
};

/**
* @internal prvTgfBrgDefFdbMacEntryAndCommandOnPortSet function
* @endinternal
*
* @brief   Set FDB entry on port
*
* @param[in] macAddr                  - mac address
* @param[in] vlanId                   - vlan Id
* @param[in] dstDevNum                - destination device
* @param[in] dstPortNum               - destination port
* @param[in] daCommand                - destination command
* @param[in] isStatic                 - is the entry static/dynamic
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
* @retval GT_BAD_PARAM             - wrong devNum or vid
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfBrgDefFdbMacEntryAndCommandOnPortSet
(
    IN TGF_MAC_ADDR                   macAddr,
    IN GT_U16                         vlanId,
    IN GT_U8                          dstDevNum,
    IN GT_PORT_NUM                    dstPortNum,
    IN PRV_TGF_PACKET_CMD_ENT         daCommand,
    IN GT_BOOL                        isStatic
)
{
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;

    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, macAddr, sizeof(TGF_MAC_ADDR));

    macEntry.key.entryType                = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId       = vlanId;
    macEntry.dstInterface.type            = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum  = dstDevNum;
    macEntry.dstInterface.devPort.portNum = dstPortNum;
    macEntry.isStatic                     = isStatic;
    macEntry.daCommand                    = daCommand;

    return prvTgfBrgFdbMacEntrySet(&macEntry);
}

/**
* @internal prvTgfBrgGenBypassModeTestPacketSend function
* @endinternal
*
* @brief   Function sends packetand performs trace.
*
* @param[in] portInterfacePtr         - (pointer to) port interface (port only is legal param)
* @param[in] packetInfoPtr            - (pointer to) the packet info
*                                       None
*/
static GT_VOID prvTgfBrgGenBypassModeTestPacketSend
(
    IN CPSS_INTERFACE_INFO_STC  *portInterfacePtr,
    IN TGF_PACKET_STC           *packetInfoPtr
)
{
    GT_STATUS rc          = GT_OK;
    GT_U32    portsCount  = prvTgfPortsNum;
    GT_U32    portIter    = 0;

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the ethertype
      of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /* reset counters */
    for (portIter = 0; portIter < portsCount; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, 0);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* Start capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(portInterfacePtr, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterfacePtr->devPort.hwDevNum, portInterfacePtr->devPort.portNum, GT_TRUE);

    /* send packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS]);

    /* wait for packets come to CPU */
    (void) tgfTrafficGeneratorRxInCpuNumWait(prvTgfBurstCount, 500, NULL);

    /* Stop capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(portInterfacePtr, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterfacePtr->devPort.hwDevNum, portInterfacePtr->devPort.portNum, GT_FALSE);

    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterfacePtr->devPort.portNum);

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}

/**
* @internal prvTgfBrgGenBypassModeTestConfigurationSet function
* @endinternal
*
* @brief   Bridge Bypass mode initial configurations
*/
GT_VOID prvTgfBrgGenBypassModeTestConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 2 with port 0,1,2,3. */
    prvTgfBrgGenIeeeReservedMcastProfilesTestInit(PRV_TGF_VLANID_CNS);

    /* AUTODOC: flush FDB include static entries. */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d",
                                 GT_TRUE);

    /* AUTODOC: create a macEntry for bypass packet forwarding to port 2,
                packet command MIRROR. */
    rc = prvTgfBrgDefFdbMacEntryAndCommandOnPortSet(
            prvTgfBypassPacketL2Part.daMac, PRV_TGF_VLANID_CNS, prvTgfDevNum,
            prvTgfPortsArray[EGR_FDB_PORT_IDX_CNS],
            PRV_TGF_PACKET_CMD_MIRROR_TO_CPU_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefFdbMacEntryAndCommandOnPortSet: %d", prvTgfDevNum);

    /* init PCL Engine for send port */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[ING_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);

    /* AUTODOC: set PCL rule with bypass bridge DISABLED*/
    rc = prvTgfTrafficPclRuleAndActionSet(GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfTrafficPclRuleAndActionSet: %d, %d",
        prvTgfDevNum, GT_FALSE);

    /* Save Bridge Bypass Mode for restore */
    rc = prvTgfBrgGenBypassModeGet(prvTgfDevNum, &prvTgfBridgeBypassMode);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgGenBypassModeGet: %d, %d",
        prvTgfDevNum, prvTgfBridgeBypassMode);
}

/**
* @internal prvTgfBrgGenBypassModeTestConfigurationRestore function
* @endinternal
*
* @brief   Bridge Bypass mode restore configurations
*/
GT_VOID prvTgfBrgGenBypassModeTestConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: invalidate PCL rule */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     prvTgfPclRuleIndex,
                                     GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_STD_E,
                                 prvTgfPclRuleIndex,
                                 GT_FALSE);

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d",
                                 GT_TRUE);

    /* AUTODOC: invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* restore Bridge Bypass Mode */
    rc = prvTgfBrgGenBypassModeSet(prvTgfBridgeBypassMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgGenBypassModeSet: %d",
                                 prvTgfBridgeBypassMode);

    if(isErratumForFromCpuForBridgeBypassForwardingDecisionOnlyMode == GT_TRUE)
    {
        /* invalidate vlan entry (and reset vlan entry) */
        rc = prvTgfBrgDefVlanEntryInvalidate(FROM_CPU_VLANID_CNS);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                     prvTgfDevNum, FROM_CPU_VLANID_CNS);
    }
}

/**
* @internal prvTgfBrgGenBypassModeTest function
* @endinternal
*
* @brief   Bridge Bypass mode envelope
*/
GT_VOID prvTgfBrgGenBypassModeTest
(
    GT_VOID
)
{
    GT_STATUS                       rc;
    CPSS_INTERFACE_INFO_STC         portInterface;
    GT_U32                          portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;

    GT_U8           packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32          buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32          packetActualLength = 0;
    GT_U8           devNum;
    GT_U8           queue;
    TGF_NET_DSA_STC rxParam;

    GT_STATUS   expectedRcRxToCpu = GT_OK;

    if(GT_TRUE == prvUtfIsGmCompilation())
    {
        /* the GM behave different then the HW !!! */
        /* the WM was fixed to be like HA */
        isErratumForFromCpuForBridgeBypassForwardingDecisionOnlyMode = GT_FALSE;
    }
    else
    {
        if(PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum))
        {
             /* In BC2 B0 errata has fixed */
             isErratumForFromCpuForBridgeBypassForwardingDecisionOnlyMode = GT_FALSE;
        }
    }

    prvTgfBrgGenBypassModeTestConfigurationSet();

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[EGR_FDB_PORT_IDX_CNS];

    /* packet for FDB forwarding - no bridge bypass, exit on port 2 (FDB), */
    /*                             mirroring expected.                     */

    /* AUTODOC: send packet. */
    prvTgfBrgGenBypassModeTestPacketSend(&portInterface, &prvTgfBypassPacketInfo);

    /* AUTODOC: packet forward to port 2 (FDB).  */
    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (ING_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
            continue;
        }

        /* check Tx counters */
        if (EGR_FDB_PORT_IDX_CNS == portIter)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
        }
    }

    /* AUTODOC: packet MAC SA learning verified. */
    rc = prvTgfFdbMacLearningCheck(prvTgfBypassPacketL2Part.saMac);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFdbMacLearningCheck");

    /* AUTODOC: packet MIRROR to CPU (FDB).      */
    /* get 1 entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_TRUE, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");

    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_FALSE, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet");

    /* AUTODOC: flush FDB exclude static entries */
    rc = prvTgfBrgFdbFlush(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d",
                                 GT_FALSE);

    /* AUTODOC: set PCL rule with bypass bridge ENABLED*/
    rc = prvTgfTrafficPclRuleAndActionSet(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfTrafficPclRuleAndActionSet: %d, %d",
        prvTgfDevNum, GT_FALSE);

    /* AUTODOC: set bypass bridge mode to all except SA learning. */
    rc = prvTgfBrgGenBypassModeSet(PRV_TGF_BRG_GEN_BYPASS_MODE_ALL_EXCEPT_SA_LEARNING_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgGenBypassModeSet: %d",
                                 PRV_TGF_BRG_GEN_BYPASS_MODE_ALL_EXCEPT_SA_LEARNING_E);

    /* packet according to PCL action - bridge bypass, exit on port 0 (PCL), */
    /*                                  no mirroring expected.               */
    portInterface.devPort.portNum = prvTgfPortsArray[EGR_PCL_PORT_IDX_CNS];

    /* AUTODOC: send packet. */
    prvTgfBrgGenBypassModeTestPacketSend(&portInterface, &prvTgfBypassPacketInfo);

    /* AUTODOC: packet forward to port 0 (PCL).  */
    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (ING_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
            continue;
        }

        /* check Tx counters */
        if (EGR_PCL_PORT_IDX_CNS == portIter)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
        }
    }

    /* AUTODOC: packet MAC SA learning verified. */
    rc = prvTgfFdbMacLearningCheck(prvTgfBypassPacketL2Part.saMac);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFdbMacLearningCheck");

    /* AUTODOC: packet NOT MIRROR to CPU, only FORWARD (PCL).      */
    /* get 1 entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_TRUE, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet");

    /* AUTODOC: flush FDB exclude static entries */
    rc = prvTgfBrgFdbFlush(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d",
                                 GT_FALSE);

    /* AUTODOC: set bypass bridge mode to bypass only forwarding decision. */
    rc = prvTgfBrgGenBypassModeSet(PRV_TGF_BRG_GEN_BYPASS_MODE_ONLY_FORWARDING_DECISION_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgGenBypassModeSet: %d",
                                 PRV_TGF_BRG_GEN_BYPASS_MODE_ONLY_FORWARDING_DECISION_E);

    if(isErratumForFromCpuForBridgeBypassForwardingDecisionOnlyMode == GT_TRUE)
    {
        /* packet according to PCL action - bridge bypass, NO exit any port (the FROM_CPU filtered), */
        /*                                  NO mirroring expected                   */

        cpssOsPrintf("WARNING: Due to Erratum : The 'FROM_CPU' should be filtered !!! in 'Bridge Bypass Forwarding Decision Only Mode' \n");

        /* AUTODOC: send packet. */
        prvTgfBrgGenBypassModeTestPacketSend(&portInterface, &prvTgfBypassPacketInfo);

        /* it seems that the device filters the 'from cpu' packet when mode is
            'Bypass Forwarding Decision only' due to 'vlan not valid' */
        /* read counters */
        portIter = ING_PORT_IDX_CNS;
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL1_STRING_MAC(0/*filtered !!!*/, portCntrs.goodPktsRcv.l[0],
                                     "ERROR : The FROM_CPU should be filtered !!! (and not reach port %d)",
                                     prvTgfPortsArray[portIter]);
        /* clear all the counters */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            (void)prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        }

        PRV_UTF_LOG0_MAC("WARNING: Due to Erratum : validate the VLAN that the 'FROM_CPU' uses and set the egress port member so the packet can egress in the next try !!! \n");
        /* validate the VLAN that the 'FROM_CPU' uses and set the egress port member .
           so the packet can egress in the next try !!! */
        prvTgfBrgGenIeeeReservedMcastProfilesTestInit(FROM_CPU_VLANID_CNS);

    }

    /* packet according to PCL action - bridge bypass, exit on port 0 (PCL), */
    /*                                  mirroring expected                   */

    /* AUTODOC: send packet. */
    prvTgfBrgGenBypassModeTestPacketSend(&portInterface, &prvTgfBypassPacketInfo);

    /* AUTODOC: packet forward to port 0 (PCL).  */
    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (ING_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
            continue;
        }

        /* check Tx counters */
        if (EGR_PCL_PORT_IDX_CNS == portIter)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
        }
    }

    /* AUTODOC: packet MAC SA learning verified. */
    rc = prvTgfFdbMacLearningCheck(prvTgfBypassPacketL2Part.saMac);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFdbMacLearningCheck");

    if(PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum) == GT_FALSE)
    {
        /* Due to Bobcat2 errata: No traffic to CPU in Bridge Bypass Forwarding Decision Only mode. */
        expectedRcRxToCpu = GT_NO_MORE;
    }

    /* AUTODOC: packet NOT MIRROR to CPU, only FORWARD (PCL).      */
    /* get 1 entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_TRUE, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(expectedRcRxToCpu, rc, "tgfTrafficGeneratorRxInCpuGet");

    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_FALSE, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet");

    prvTgfBrgGenBypassModeTestConfigurationRestore();
}

/**
* @internal prvTgfBrgPhysicalPortsSetUpTest function
* @endinternal
*
* @brief   Bridge physical port setup test
*/
GT_VOID prvTgfBrgPhysicalPortsSetUpTest
(
    GT_VOID
)
{
    GT_U32                  port;
    GT_U32                  numPorts;
    GT_BOOL                 status;
    CPSS_PORT_LOCK_CMD_ENT  cmd;
    GT_BOOL                 forwarddEn;
    GT_STATUS               rc;

    /* AUTODOC: check default values for cpssDxChBrgFdbNaToCpuPerPortSet and cpssDxChBrgFdbPortLearnStatusSet
       on all ports only for eArch devices */
    numPorts = 256;

    for (port = 0; port < numPorts; port++)
    {
        /* AUTODOC: get current value of forwarding of NA to CPU enable */
        rc = cpssDxChBrgFdbNaToCpuPerPortGet(prvTgfDevNum, port, &forwarddEn);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbNaToCpuPerPortGet");
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, forwarddEn, "cpssDxChBrgFdbNaToCpuPerPortGet: port[%d], forwardEn[%d]", port, forwarddEn);

        rc = cpssDxChBrgFdbPortLearnStatusGet(prvTgfDevNum, port, &status, &cmd);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbPortLearnStatusGet");
        if(prvWrAppForceAutoLearn() == GT_TRUE)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_TRUE, status, "cpssDxChBrgFdbPortLearnStatusGet:port[%d] learn status[%d]", port, status);
        }
        else
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_FALSE, status, "cpssDxChBrgFdbPortLearnStatusGet:port[%d] learn status[%d]", port, status);
        }

        UTF_VERIFY_EQUAL2_STRING_MAC(CPSS_LOCK_FRWRD_E, cmd, "cpssDxChBrgFdbPortLearnStatusGet:port[%d] learn command[%d]", port, cmd);
    }
}

/**
* @internal prvTgfBrgGenPortBypassModeTestConfigurationRestore function
* @endinternal
*
* @brief   Per Source Physical Port Bridge Bypass mode restore configurations
*/
GT_VOID prvTgfBrgGenPortBypassModeTestConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: invalidate PCL rule */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     prvTgfPclRuleIndex,
                                     GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_STD_E,
                                 prvTgfPclRuleIndex,
                                 GT_FALSE);

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d",
                                 GT_TRUE);

    /* AUTODOC: invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* restore Per Source Physical Port Bridge Bypass Mode */
    rc = prvTgfBrgGenPortBypassModeSet(prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS], prvTgfBridgePortBypassMode);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "port %d prvTgfBrgGenPortBypassModeSet: %d",
                                 prvTgfPortsArray[ING_PORT_IDX_CNS], prvTgfBridgeBypassMode);

}

/**
* @internal prvTgfBrgGenPortBypassModeTestConfigurationSet function
* @endinternal
*
* @brief   Per Source Physical Port Bridge Bypass mode initial configurations
*/
GT_VOID prvTgfBrgGenPortBypassModeTestConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 2 with port 0,1,2,3. */
    prvTgfBrgGenIeeeReservedMcastProfilesTestInit(PRV_TGF_VLANID_CNS);

    /* AUTODOC: flush FDB include static entries. */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d",
                                 GT_TRUE);

    /* AUTODOC: create a macEntry for bypass packet forwarding to port 2,
                packet command MIRROR. */
    rc = prvTgfBrgDefFdbMacEntryAndCommandOnPortSet(
            prvTgfBypassPacketL2Part.daMac, PRV_TGF_VLANID_CNS, prvTgfDevNum,
            prvTgfPortsArray[EGR_FDB_PORT_IDX_CNS],
            PRV_TGF_PACKET_CMD_MIRROR_TO_CPU_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfBrgDefFdbMacEntryAndCommandOnPortSet: %d", prvTgfDevNum);

    /* init PCL Engine for send port */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[ING_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);

    /* AUTODOC: set PCL rule with bypass bridge DISABLED*/
    rc = prvTgfTrafficPclRuleAndActionSet(GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfTrafficPclRuleAndActionSet: %d, %d",
        prvTgfDevNum, GT_FALSE);

    /* Save Per Source Physical Port Bridge Bypass Mode for restore */
    rc = prvTgfBrgGenPortBypassModeGet(prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS], &prvTgfBridgePortBypassMode);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfBrgGenPortBypassModeGet: %d, %d",
        prvTgfDevNum, prvTgfBridgePortBypassMode);
}

/**
* @internal prvTgfBrgGenPortBypassModeTest function
* @endinternal
*
* @brief   Per Source Physical Port Bridge Bypass mode
*/
GT_VOID prvTgfBrgGenPortBypassModeTest
(
    GT_VOID
)
{
    GT_STATUS                       rc;
    CPSS_INTERFACE_INFO_STC         portInterface;
    GT_U32                          portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;

    GT_U8           packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32          buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32          packetActualLength = 0;
    GT_U8           devNum;
    GT_U8           queue;
    TGF_NET_DSA_STC rxParam;

    GT_STATUS   expectedRcRxToCpu = GT_OK;

    prvTgfBrgGenPortBypassModeTestConfigurationSet();

    /* set port for packet capture */
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[EGR_FDB_PORT_IDX_CNS];

    /* packet for FDB forwarding - no bridge bypass, exit on port 2 (FDB), */
    /*                             mirroring expected.                     */

    /* AUTODOC: send packet. */
    prvTgfBrgGenBypassModeTestPacketSend(&portInterface, &prvTgfBypassPacketInfo);

    /* AUTODOC: packet forward to port 2 (FDB).  */
    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (ING_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
            continue;
        }

        /* check Tx counters */
        if (EGR_FDB_PORT_IDX_CNS == portIter)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
        }
    }

    /* AUTODOC: packet MAC SA learning verified. */
    rc = prvTgfFdbMacLearningCheck(prvTgfBypassPacketL2Part.saMac);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFdbMacLearningCheck");

    /* AUTODOC: packet MIRROR to CPU (FDB).      */
    /* get 1 entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_TRUE, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");

    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_FALSE, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet");

    /* AUTODOC: flush FDB exclude static entries */
    rc = prvTgfBrgFdbFlush(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d",
                                 GT_FALSE);

    /* AUTODOC: set PCL rule with bypass bridge ENABLED*/
    rc = prvTgfTrafficPclRuleAndActionSet(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfTrafficPclRuleAndActionSet: %d, %d",
        prvTgfDevNum, GT_FALSE);

    /* AUTODOC: set bypass bridge mode to all except SA learning. */
    rc = prvTgfBrgGenPortBypassModeSet(prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS], PRV_TGF_BRG_GEN_BYPASS_MODE_ALL_EXCEPT_SA_LEARNING_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "port %d prvTgfBrgGenPortBypassModeSet: %d",
                                 prvTgfPortsArray[ING_PORT_IDX_CNS], PRV_TGF_BRG_GEN_BYPASS_MODE_ALL_EXCEPT_SA_LEARNING_E);

    /* packet according to PCL action - bridge bypass, exit on port 0 (PCL), */
    /*                                  no mirroring expected.               */
    portInterface.devPort.portNum = prvTgfPortsArray[EGR_PCL_PORT_IDX_CNS];

    /* AUTODOC: send packet. */
    prvTgfBrgGenBypassModeTestPacketSend(&portInterface, &prvTgfBypassPacketInfo);

    /* AUTODOC: packet forward to port 0 (PCL).  */
    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (ING_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
            continue;
        }

        /* check Tx counters */
        if (EGR_PCL_PORT_IDX_CNS == portIter)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
        }
    }

    /* AUTODOC: packet MAC SA learning verified. */
    rc = prvTgfFdbMacLearningCheck(prvTgfBypassPacketL2Part.saMac);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFdbMacLearningCheck");

    /* AUTODOC: packet NOT MIRROR to CPU, only FORWARD (PCL).      */
    /* get 1 entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_TRUE, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet");

    /* AUTODOC: flush FDB exclude static entries */
    rc = prvTgfBrgFdbFlush(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d",
                                 GT_FALSE);

    /* AUTODOC: set per source physical port bypass bridge mode to bypass only forwarding decision. */
    rc = prvTgfBrgGenPortBypassModeSet(prvTgfDevNum, prvTgfPortsArray[ING_PORT_IDX_CNS], PRV_TGF_BRG_GEN_BYPASS_MODE_ONLY_FORWARDING_DECISION_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,"port %d  prvTgfBrgGenPortBypassModeSet: %d",
                                 prvTgfPortsArray[ING_PORT_IDX_CNS], PRV_TGF_BRG_GEN_BYPASS_MODE_ONLY_FORWARDING_DECISION_E);

    /* packet according to PCL action - bridge bypass, exit on port 0 (PCL), */
    /*                                  mirroring expected                   */

    /* AUTODOC: send packet. */
    prvTgfBrgGenBypassModeTestPacketSend(&portInterface, &prvTgfBypassPacketInfo);

    /* AUTODOC: packet forward to port 0 (PCL).  */
    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (ING_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
            continue;
        }

        /* check Tx counters */
        if (EGR_PCL_PORT_IDX_CNS == portIter)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected on port %d",
                                         prvTgfPortsArray[portIter]);
        }
    }

    /* AUTODOC: packet MAC SA learning verified. */
    rc = prvTgfFdbMacLearningCheck(prvTgfBypassPacketL2Part.saMac);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfFdbMacLearningCheck");

    /* AUTODOC: packet NOT MIRROR to CPU, only FORWARD (PCL).      */
    /* get 1 entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_TRUE, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(expectedRcRxToCpu, rc, "tgfTrafficGeneratorRxInCpuGet");

    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_FALSE, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet");

    prvTgfBrgGenPortBypassModeTestConfigurationRestore();
}


/**
* @internal prvTgfBrgGenIpLinkLocalProtCmdSetConfig function
* @endinternal
*
* @brief   Function to config the IP MC linklocal packets on global and vlan level.
*
*/
GT_VOID prvTgfBrgGenIpLinkLocalProtCmdSetConfig()
{
    GT_STATUS rc;

    /* Create VLAN 5 with tagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(PRV_TGF_VLANID_CNS, PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", PRV_TGF_VLANID_CNS);

    /* AUTODOC: enable vlan MC mirror to cpu  */
    rc = cpssDxChBrgVlanIpCntlToCpuGet(prvTgfDevNum, PRV_TGF_VLANID_CNS, &prvTgfRestoreCfg.protocol);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanIpCntlToCpuGet: %d vlan %d", prvTgfDevNum, PRV_TGF_VLANID_CNS);
    rc = cpssDxChBrgVlanIpCntlToCpuSet(prvTgfDevNum, PRV_TGF_VLANID_CNS, CPSS_DXCH_BRG_IP_CTRL_IPV4_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanIpCntlToCpuSet: %d vlan %d", prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* AUTODOC: save and enable global MC mirror to cpu  */
    rc = cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet(prvTgfDevNum, CPSS_IP_PROTOCOL_IPV4_E, &prvTgfRestoreCfg.globalMcMirrorEnable);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet: %d vlan %d", prvTgfDevNum, PRV_TGF_VLANID_CNS);
    rc = cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable(prvTgfDevNum, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable: %d vlan %d", prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* AUTODOC: save MC mirror cmd  */
    rc = cpssDxChBrgGenIpLinkLocalProtCmdGet(prvTgfDevNum, CPSS_IP_PROTOCOL_IPV4_E, prvTgfPacketBrgIpMcIpv4Part.dstAddr[3] , &prvTgfRestoreCfg.protoMcMirrorCmd);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgGenIpLinkLocalProtCmdGet: %d vlan %d", prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* AUTODOC: save MC mirror cmd  */
    rc = cpssDxChBrgGenIpLinkLocalProtCmdGet(prvTgfDevNum, CPSS_IP_PROTOCOL_IPV4_E, prvTgfPacketBrgIpMcIpv4Part.dstAddr[3] , &prvTgfRestoreCfg.ipv4McPortRoutingEnableGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgGenIpLinkLocalProtCmdGet: %d vlan %d", prvTgfDevNum, PRV_TGF_VLANID_CNS);
}

/**
* @internal prvTgfBrgGenIpLinkLocalProtCmdSetConfigReset function
* @endinternal
*
* @brief   Function to reset config the IP MC linklocal packets on global and vlan level.
*
*/
GT_VOID prvTgfBrgGenIpLinkLocalProtCmdSetConfigReset()
{
    GT_STATUS rc;

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, vlan %d", prvTgfDevNum, PRV_TGF_VLANID_CNS);
    /* Flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: restore global MC mirror to cpu  */
    rc = cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable(prvTgfDevNum, CPSS_IP_PROTOCOL_IPV4_E, prvTgfRestoreCfg.globalMcMirrorEnable);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable: %d vlan %d", prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* AUTODOC: restore MC mirror cmd  */
    rc = cpssDxChBrgGenIpLinkLocalProtCmdSet(prvTgfDevNum, CPSS_IP_PROTOCOL_IPV4_E, prvTgfPacketBrgIpMcIpv4Part.dstAddr[3] , prvTgfRestoreCfg.protoMcMirrorCmd);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgGenIpLinkLocalProtCmdSet: %d vlan %d", prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* AUTODOC: restore MC mirror cmd  */
    rc = cpssDxChBrgGenIpLinkLocalProtCmdSet(prvTgfDevNum, CPSS_IP_PROTOCOL_IPV4_E, prvTgfPacketBrgIpMcIpv4Part.dstAddr[3] , prvTgfRestoreCfg.ipv4McPortRoutingEnableGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgGenIpLinkLocalProtCmdGet: %d vlan %d", prvTgfDevNum, PRV_TGF_VLANID_CNS);

    tgfTrafficTableRxPcktTblClear();
}
/**
* @internal prvTgfBrgGenIpLinkLocalProtCmdSetTest function
* @endinternal
*
* @brief   Function to send traffic and test the IP MC
*           linklocal packets on global and vlan level.
*           This tests by configuring different packet commands for
*           IPv4 protocol and sending traffic
*
*/
static GT_VOID prvTgfBrgGenIpLinkLocalProtCmdSetTest(CPSS_PACKET_CMD_ENT pktCmd)
{
    GT_U8                 queue = 0;
    GT_STATUS             rc = GT_OK;
    GT_BOOL floodOrMirror=GT_FALSE, toCpu=GT_FALSE;
    GT_U32                          portIter = 0;
    GT_STATUS   expectedRcRxToCpu = GT_OK;
    GT_BOOL     getFirst = GT_TRUE;
    GT_U8       packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32      buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32      packetActualLength = 0;
    GT_U8       devNum;
    TGF_NET_DSA_STC rxParam;

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /*start capture*/
    tgfTrafficTableRxStartCapture(GT_TRUE);

    /*setup packet*/
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketBrgIpMcInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");

    PRV_UTF_LOG0_MAC("======= Send packet =======\n");
    /* Send  packet  */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStartTransmitingEth");

    /*stop capture*/
    tgfTrafficTableRxStartCapture(GT_FALSE);

    if (pktCmd == CPSS_PACKET_CMD_FORWARD_E
        || pktCmd == CPSS_PACKET_CMD_MIRROR_TO_CPU_E
        )
    {
        floodOrMirror = GT_TRUE;
    }
    if (pktCmd == CPSS_PACKET_CMD_MIRROR_TO_CPU_E
        || pktCmd == CPSS_PACKET_CMD_TRAP_TO_CPU_E
       )
    {
        toCpu = GT_TRUE;
    }

    /*rx port counters*/
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfEthCountersCheck(
            prvTgfDevNum,
            prvTgfPortsArray[portIter],
            /*Rx*/ (PRV_TGF_TX_PORT_IDX_CNS  == portIter)  ? 1 : 0,
            /*Tx*/ (PRV_TGF_TX_PORT_IDX_CNS  == portIter)  ? 1 :(floodOrMirror == GT_TRUE) ? 1 : 0,
            prvTgfPacketBrgIpMcInfo.totalLen,
            prvTgfBurstCount);
        UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfEthCountersCheck: %d, port %d pktCmd %d\n",
            prvTgfDevNum, prvTgfPortsArray[portIter], pktCmd);
    }
    if ( GT_FALSE == toCpu )
    {
        expectedRcRxToCpu = GT_NO_MORE;
    }
    /* get first entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       getFirst, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(expectedRcRxToCpu, rc,
                                 "tgfTrafficGeneratorRxInCpuGet");
    if (toCpu)
    {
        PRV_UTF_LOG1_MAC("CPU Code %d\n", rxParam.cpuCode);
        /* check CPU code */
        UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_NET_IPV4_IPV6_LINK_LOCAL_MC_DIP_TRP_MRR_E, rxParam.cpuCode, "Wrong CPU Code");
    }
}

/**
* @internal prvTgfBrgGenIpLinkLocalProtCmdSetTrafficTest function
* @endinternal
*
* @brief   Function to send traffic and test the IP MC
*           linklocal packets on global and vlan level.
*
*/
GT_VOID prvTgfBrgGenIpLinkLocalProtCmdSetTrafficTest()
{
    GT_STATUS             rc = GT_OK;
    CPSS_PACKET_CMD_ENT pktCmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;

    for (pktCmd = CPSS_PACKET_CMD_FORWARD_E; pktCmd <= CPSS_PACKET_CMD_DROP_SOFT_E; pktCmd++)
    {
        if (!(PRV_CPSS_SIP_5_10_CHECK_MAC(prvTgfDevNum)) && !((pktCmd == CPSS_PACKET_CMD_FORWARD_E) || (pktCmd == CPSS_PACKET_CMD_MIRROR_TO_CPU_E)))
        {
            /*for not SIP5 devices only valid cmds are FORWARD and MIRROR*/
            continue;
        }
        rc = cpssDxChBrgGenIpLinkLocalProtCmdSet(prvTgfDevNum, CPSS_IP_PROTOCOL_IPV4_E, prvTgfPacketBrgIpMcIpv4Part.dstAddr[3] , pktCmd);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgGenIpLinkLocalProtCmdSet: %d vlan %d", prvTgfDevNum, PRV_TGF_VLANID_CNS);

        prvTgfBrgGenIpLinkLocalProtCmdSetTest(pktCmd);
    }
}

/**********************************************************************/
/* TC for assinging eport attr locally*/
/**********************************************************************/
/**
* @internal prvTgfBrgGenEportAssignEgressAttributesLocallySet function
* @endinternal
*
* @brief   Function to config the assigning the eport attr locally.
*           Create VLAN 5 with tagged ports [0,1,2,3]
*           add FDB entry with MAC 00:00:00:00:00:02,  port 2
*           Add FDB entry dest port as remoteDev, ePort
*           set cascade mapping for remote device\port
*           assign local attributes to the eport enable
*           config push tag on local eport index.
*           config TPID index entry
*           config eport with the TPID entry
*
*/
GT_VOID prvTgfPacketBrgLocalAssignConfig()
{
    GT_STATUS rc;
    PRV_TGF_BRG_MAC_ENTRY_STC prvTgfMacEntry;
    CPSS_CSCD_LINK_TYPE_STC cascadeLink;
    PRV_TGF_CSCD_TRUNK_LINK_HASH_ENT srcPortTrunkHashEn;

    /*AUTODOC: set the own HW DEV num*/
    rc = cpssDxChCfgHwDevNumGet(prvTgfDevNum, &prvTgfEportLocalAssignRestore.savedDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberGet \n");

    rc = cpssDxChCfgHwDevNumSet(prvTgfDevNum, PRV_TGF_OWN_DEV_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgHwDevNumSet \n");

    /* Create VLAN 5 with tagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(PRV_TGF_VLANID_CNS, PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", PRV_TGF_VLANID_CNS);

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:02,  port 2 */
    cpssOsMemSet(&prvTgfMacEntry, 0, sizeof(prvTgfMacEntry));

    /* AUTODOC: Set FDB entry with remoteDev, ePort */
    prvTgfMacEntry.key.key.macVlan.vlanId = PRV_TGF_VLANID_CNS;
    prvTgfMacEntry.isStatic = GT_TRUE;
    prvTgfMacEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;
    prvTgfMacEntry.dstInterface.devPort.portNum  = PRV_TGF_TARGET_EPORT_CNS;
    prvTgfMacEntry.dstInterface.devPort.hwDevNum = PRV_TGF_DSA_DEV_CNS;
    cpssOsMemCpy(prvTgfMacEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketBrgLocalAssignL2Part.daMac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfBrgFdbMacEntrySet(&prvTgfMacEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet");

    /* AUTODOC: save cascade mapping for restore */
    rc = prvTgfCscdDevMapTableGet(prvTgfDevNum, PRV_TGF_DSA_DEV_CNS, PRV_TGF_DSA_PORT_CNS,0,
                                  &prvTgfEportLocalAssignRestore.cascadeLink, &prvTgfEportLocalAssignRestore.srcPortTrunkHashEn);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableGet: %d,%d,%d",
                                 prvTgfDevNum, PRV_TGF_DSA_DEV_CNS, PRV_TGF_DSA_PORT_CNS);

    cascadeLink.linkNum = prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS];
    cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
    srcPortTrunkHashEn = PRV_TGF_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E;

    /* AUTODOC: set cascade mapping for remote device\port */
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum, PRV_TGF_DSA_DEV_CNS, PRV_TGF_DSA_PORT_CNS, 0,
                                  &cascadeLink, srcPortTrunkHashEn, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableSet: %d,%d,%d",
                                 prvTgfDevNum, PRV_TGF_DSA_DEV_CNS, PRV_TGF_DSA_PORT_CNS);

    /* AUTODOC: assign local attributes to the eport enable  */
    rc = cpssDxChBrgEportAssignEgressAttributesLocallyGet(prvTgfDevNum, PRV_TGF_TARGET_EPORT_CNS, &prvTgfEportLocalAssignRestore.localAssignEnable);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgEportAssignEgressAttributesLocallyGet: %d,%d",
                                 prvTgfDevNum, PRV_TGF_TARGET_EPORT_CNS);
    /* AUTODOC: config push tag on local eport index.*/
    rc = cpssDxChBrgVlanPortPushVlanCommandGet(prvTgfDevNum, PRV_TGF_TARGET_EPORT_CNS, &prvTgfEportLocalAssignRestore.vlanCmd);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanPortPushVlanCommandGet: %d,%d",
                                 prvTgfDevNum, PRV_TGF_TARGET_EPORT_CNS);
    rc = cpssDxChBrgVlanPortPushVlanCommandSet(prvTgfDevNum, PRV_TGF_TARGET_EPORT_CNS, CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_PUSH_SINGLE_TAG_ON_PASSENGER_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanPortPushVlanCommandSet: dev %d, port %d val %d",
                                 prvTgfDevNum, PRV_TGF_TARGET_EPORT_CNS, CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_PUSH_SINGLE_TAG_ON_PASSENGER_E);

    /* AUTODOC: config TPID index entry*/
    rc = cpssDxChBrgVlanTpidEntryGet(prvTgfDevNum, CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_E, PRV_TGF_BRG_VLAN_PORT_TPID_ENTRY_ID_CNS, &prvTgfEportLocalAssignRestore.etherType);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanPortPushVlanCommandGet: %d,%d",
                                 prvTgfDevNum, PRV_TGF_TARGET_EPORT_CNS);
    rc = cpssDxChBrgVlanTpidEntrySet(prvTgfDevNum, CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_E, PRV_TGF_BRG_VLAN_PORT_TPID_ENTRY_ID_CNS, PRV_TGF_BRG_VLAN_PORT_TPID_ETHER_TYPE_CNS);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanTpidEntrySet: dev %d, port %d val 0x%x",
                                 prvTgfDevNum, PRV_TGF_TARGET_EPORT_CNS, PRV_TGF_BRG_VLAN_PORT_TPID_ETHER_TYPE_CNS);

    /* AUTODOC: config eport with the TPID entry*/
    rc = cpssDxChBrgVlanPortPushedTagTpidSelectEgressGet(prvTgfDevNum, PRV_TGF_TARGET_EPORT_CNS, &prvTgfEportLocalAssignRestore.tpidEntryIndex);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet: %d,%d",
                                 prvTgfDevNum, PRV_TGF_TARGET_EPORT_CNS);
    rc = cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet(prvTgfDevNum, PRV_TGF_TARGET_EPORT_CNS, PRV_TGF_BRG_VLAN_PORT_TPID_ENTRY_ID_CNS);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet: dev %d, port %d val %d",
                                 prvTgfDevNum, PRV_TGF_TARGET_EPORT_CNS, PRV_TGF_BRG_VLAN_PORT_TPID_ENTRY_ID_CNS);
}
/**
* @internal prvTgfPacketBrgLocalAssignConfigReset function
* @endinternal
*
* @brief   Function to reset the assigning the eport attr locally.
*
*/
GT_VOID prvTgfPacketBrgLocalAssignConfigReset()
{
    GT_STATUS rc;

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, vlan %d", prvTgfDevNum, PRV_TGF_VLANID_CNS);
    /* Flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: save cascade mapping for restore */
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum, PRV_TGF_DSA_DEV_CNS, PRV_TGF_DSA_PORT_CNS,0,
                                  &prvTgfEportLocalAssignRestore.cascadeLink, prvTgfEportLocalAssignRestore.srcPortTrunkHashEn, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableSet: %d,%d,%d",
                                 prvTgfDevNum, PRV_TGF_DSA_DEV_CNS, PRV_TGF_DSA_PORT_CNS);
    /* AUTODOC: assign local attributes to the eport enable  */
    rc = cpssDxChBrgEportAssignEgressAttributesLocallySet(prvTgfDevNum, PRV_TGF_TARGET_EPORT_CNS, prvTgfEportLocalAssignRestore.localAssignEnable);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgEportAssignEgressAttributesLocallySet: %d,%d",
                                 prvTgfDevNum, PRV_TGF_TARGET_EPORT_CNS);
    /* AUTODOC: config push tag on local eport index.*/
    rc = cpssDxChBrgVlanPortPushVlanCommandSet(prvTgfDevNum, PRV_TGF_TARGET_EPORT_CNS, prvTgfEportLocalAssignRestore.vlanCmd);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanPortPushVlanCommandSet: %d,%d",
                                 prvTgfDevNum, PRV_TGF_TARGET_EPORT_CNS);
    /* AUTODOC: config TPID index entry*/
    rc = cpssDxChBrgVlanTpidEntrySet(prvTgfDevNum, CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_E, PRV_TGF_BRG_VLAN_PORT_TPID_ENTRY_ID_CNS, prvTgfEportLocalAssignRestore.etherType);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanPortPushVlanCommandSet: %d,%d",
                                 prvTgfDevNum, PRV_TGF_TARGET_EPORT_CNS);
    /* AUTODOC: config eport with the TPID entry*/
    rc = cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet(prvTgfDevNum, PRV_TGF_TARGET_EPORT_CNS, prvTgfEportLocalAssignRestore.tpidEntryIndex);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet: %d,%d",
                                 prvTgfDevNum, PRV_TGF_TARGET_EPORT_CNS);
    /*AUTODOC: restore dev num*/
    rc = cpssDxChCfgHwDevNumSet(prvTgfDevNum, prvTgfEportLocalAssignRestore.savedDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgHwDevNumSet\n");

    tgfTrafficTableRxPcktTblClear();
}
/**
* @internal prvTgfPacketBrgLocalAssignTraffic function
* @endinternal
*
* @brief   Function to send traffic and test the egress eport filtering mechanism.
*/
static GT_VOID prvTgfPacketBrgLocalAssignTraffic(GT_BOOL assignTrgEPortAttributesLocally)
{
    CPSS_INTERFACE_INFO_STC  analyzerPortInterface;
    GT_U8                 packetBuf[256];
    GT_U32                packetLen = sizeof(packetBuf);
    GT_U32                packetActualLength = 0;
    GT_U8                 queue = 0;
    GT_U8                 dev = 0;
    GT_BOOL               getFirst = GT_TRUE;
    TGF_NET_DSA_STC       rxParam;
    GT_STATUS             rc = GT_OK;
    GT_U16                tpid;

    /* Enable capture on analyzer port */
    analyzerPortInterface.type              = CPSS_INTERFACE_PORT_E;
    analyzerPortInterface.devPort.hwDevNum  = prvTgfDevNum;
    analyzerPortInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&analyzerPortInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

    /*setup packet*/
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketBrgLocalAssignInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    PRV_UTF_LOG0_MAC("======= Send packet =======\n");
    /* Send first packet  */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStartTransmitingEth");

    /* Disable capture on analyzer port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &analyzerPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

    /* AUTODOC: Get the first rx pkt on egress port */
        rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&analyzerPortInterface,
                TGF_PACKET_TYPE_CAPTURE_E,
                getFirst, GT_TRUE, packetBuf,
                &packetLen, &packetActualLength,
                &dev, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "Error: failed capture on inPortId %d outPortId %d \n" );

     /* AUTODOC: verify packet*/
        tpid = packetBuf[TGF_L2_HEADER_SIZE_CNS] << 8 | packetBuf[TGF_L2_HEADER_SIZE_CNS+1];
        if (assignTrgEPortAttributesLocally)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(PRV_TGF_BRG_VLAN_PORT_TPID_ETHER_TYPE_CNS, tpid, "TPID mismatch: dev %d ", prvTgfDevNum);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(TGF_ETHERTYPE_8100_VLAN_TAG_CNS, tpid, "TPID mismatch: dev %d ", prvTgfDevNum);
        }
}
/**
* @internal prvTgfPacketBrgLocalAssignTrafficTest function
* @endinternal
*
* @brief   Function to send traffic and test the egress eport filtering mechanism.
*           1. Enable locally assign to TRUE and send packet.
*               Expected: Packet egressed has outer tag as configured.
*           2. Disable locally assign to TRUE and send packet.
*               Expected: Packet egressed SHOULD NOT have outer tag
*
*/
GT_VOID prvTgfPacketBrgLocalAssignTrafficTest()
{

    GT_STATUS             rc = GT_OK;
    rc = cpssDxChBrgEportAssignEgressAttributesLocallySet(prvTgfDevNum, PRV_TGF_TARGET_EPORT_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "cpssDxChBrgEportAssignEgressAttributesLocallySet: %d,%d,%d",
                                 prvTgfDevNum, PRV_TGF_TARGET_EPORT_CNS, GT_FALSE);
    prvTgfPacketBrgLocalAssignTraffic(GT_FALSE);

    rc = cpssDxChBrgEportAssignEgressAttributesLocallySet(prvTgfDevNum, PRV_TGF_TARGET_EPORT_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "cpssDxChBrgEportAssignEgressAttributesLocallySet: %d,%d,%d",
                                 prvTgfDevNum, PRV_TGF_TARGET_EPORT_CNS, GT_TRUE);
    prvTgfPacketBrgLocalAssignTraffic(GT_TRUE);

}

/**
* @internal prvTgfBrgFdbUploadConfigSet function
* @endinternal
*
* @brief Set test configuration
*/
GT_VOID prvTgfBrgFdbUploadConfigSet
(
    GT_VOID
)
{
    GT_STATUS                   st = GT_OK;
    CPSS_MAC_ENTRY_EXT_STC      macEntry;
    GT_U32                      iter, macIter, ipIter;
    GT_U32                      hwDevNum;
    CPSS_MAC_UPDATE_MSG_EXT_STC *fuMessage;
    GT_U32                      numOfMsgs;
    GT_U32                      expectedIndex;
    GT_U32                      expectedVidx;
    GT_U32                      expectedHwDevNum;
    GT_U32                      expectedSrcId;

    /* Flush the FDB */
    st = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "prvTgfBrgFdbFlush");

    st = cpssDxChCfgHwDevNumGet((GT_U8)prvTgfDevNum, &hwDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChCfgHwDevNumGet: %d", prvTgfDevNum);

    /* add FDB entry,and then trigger FDB upload */
    /* add FDB entries on TRUNK, PORT, VID and VIDX interfaces */
    for(iter = 0 ; iter < PRV_TGF_FDB_UPLOAD_MESSAGE_CNS; iter++)
    {
        cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
        if(iter <= 1)
        {
            macEntry.key.entryType                      = prvTgfBrgFdbUploadMacEntryType[0];
            macEntry.key.key.macVlan.vlanId             = (iter == 0) ? prvTgfBrgFdbUploadVlanId[0] : prvTgfBrgFdbUploadVlanId[1];

            macEntry.key.key.macVlan.macAddr.arEther[0] = prvTgfBrgFdbUploadMacAddr[0].arEther[0];
            macEntry.key.key.macVlan.macAddr.arEther[1] = prvTgfBrgFdbUploadMacAddr[0].arEther[1];
            macEntry.key.key.macVlan.macAddr.arEther[2] = prvTgfBrgFdbUploadMacAddr[0].arEther[2];
            macEntry.key.key.macVlan.macAddr.arEther[3] = prvTgfBrgFdbUploadMacAddr[0].arEther[3];
            macEntry.key.key.macVlan.macAddr.arEther[4] = prvTgfBrgFdbUploadMacAddr[0].arEther[4];
            macEntry.key.key.macVlan.macAddr.arEther[5] = (iter == 0) ? prvTgfBrgFdbUploadMacAddr[0].arEther[5] : prvTgfBrgFdbUploadMacAddr[1].arEther[5];

            macEntry.dstInterface.type                  = (iter == 0) ? prvTgfBrgFdbUploadDstInterface[0] : prvTgfBrgFdbUploadDstInterface[1];
            macEntry.dstInterface.devPort.hwDevNum      = hwDevNum;
            macEntry.dstInterface.devPort.portNum       = prvTgfBrgFdbUploadPortNum[1];
            macEntry.dstInterface.trunkId               = prvTgfBrgFdbUploadTrunkId[0];
            CONVERT_TRUNK_ID_TEST_TO_CPSS_MAC(macEntry.dstInterface.trunkId);
            macEntry.dstInterface.hwDevNum              = hwDevNum;
            macEntry.dstInterface.vlanId                = (iter == 0) ? prvTgfBrgFdbUploadVlanId[0] : prvTgfBrgFdbUploadVlanId[1];

            macEntry.isStatic                           = GT_FALSE;
            macEntry.daCommand                          = CPSS_MAC_TABLE_FRWRD_E;
            macEntry.saCommand                          = CPSS_MAC_TABLE_FRWRD_E;
            macEntry.daRoute                            = GT_FALSE;
            macEntry.mirrorToRxAnalyzerPortEn           = GT_FALSE;
            macEntry.saMirrorToRxAnalyzerPortEn         = GT_FALSE;
            macEntry.daMirrorToRxAnalyzerPortEn         = GT_FALSE;
            macEntry.sourceID                           = (iter == 0) ? prvTgfBrgFdbUploadSourceId[0] : prvTgfBrgFdbUploadSourceId[1];
            macEntry.userDefined                        = 0;
            macEntry.daQosIndex                         = 0;
            macEntry.saQosIndex                         = 0;
            macEntry.daSecurityLevel                    = 0;
            macEntry.saSecurityLevel                    = 0;
            macEntry.appSpecificCpuCode                 = GT_FALSE;
            macEntry.age                                = (iter == 0) ? prvTgfBrgFdbUploadAge[0] : prvTgfBrgFdbUploadAge[1];
            macEntry.spUnknown                          = (iter == 0) ? prvTgfBrgFdbUploadSpUnknown[0] : prvTgfBrgFdbUploadSpUnknown[1];
        }
        else
        {
            macEntry.key.entryType                      = prvTgfBrgFdbUploadMacEntryType[2];
            macEntry.key.key.ipMcast.vlanId             = (iter == 2) ? prvTgfBrgFdbUploadVlanId[2] : prvTgfBrgFdbUploadVlanId[3];

            macEntry.key.key.ipMcast.sip[0]             = (iter == 2) ? prvTgfBrgFdbUploadVidInterfaceSip[0]   : prvTgfBrgFdbUploadVidxInterfaceSip[0];
            macEntry.key.key.ipMcast.sip[1]             = (iter == 2) ? prvTgfBrgFdbUploadVidInterfaceSip[1]   : prvTgfBrgFdbUploadVidxInterfaceSip[1];
            macEntry.key.key.ipMcast.sip[2]             = (iter == 2) ? prvTgfBrgFdbUploadVidInterfaceSip[2]   : prvTgfBrgFdbUploadVidxInterfaceSip[2];
            macEntry.key.key.ipMcast.sip[3]             = (iter == 2) ? prvTgfBrgFdbUploadVidInterfaceSip[3]   : prvTgfBrgFdbUploadVidxInterfaceSip[3];
            macEntry.key.key.ipMcast.dip[0]             = (iter == 2) ? prvTgfBrgFdbUploadVidInterfaceDip[0]   : prvTgfBrgFdbUploadVidxInterfaceDip[0];
            macEntry.key.key.ipMcast.dip[1]             = (iter == 2) ? prvTgfBrgFdbUploadVidInterfaceDip[1]   : prvTgfBrgFdbUploadVidxInterfaceDip[1];
            macEntry.key.key.ipMcast.dip[2]             = (iter == 2) ? prvTgfBrgFdbUploadVidInterfaceDip[2]   : prvTgfBrgFdbUploadVidxInterfaceDip[2];
            macEntry.key.key.ipMcast.dip[3]             = (iter == 2) ? prvTgfBrgFdbUploadVidInterfaceDip[3]   : prvTgfBrgFdbUploadVidxInterfaceDip[3];

            macEntry.dstInterface.type                  = (iter == 2) ? prvTgfBrgFdbUploadDstInterface[2] : prvTgfBrgFdbUploadDstInterface[3];
            macEntry.dstInterface.devPort.hwDevNum      = hwDevNum;
            macEntry.dstInterface.devPort.portNum       = 0;
            macEntry.dstInterface.hwDevNum              = hwDevNum;
            macEntry.dstInterface.vidx                  = (iter == 2) ? prvTgfBrgFdbUploadVidx[2] : prvTgfBrgFdbUploadVidx[3];
            macEntry.dstInterface.vlanId                = (iter == 2) ? prvTgfBrgFdbUploadVlanId[2] : prvTgfBrgFdbUploadVlanId[3];

            macEntry.isStatic                           = GT_FALSE;
            macEntry.daCommand                          = CPSS_MAC_TABLE_FRWRD_E;
            macEntry.saCommand                          = CPSS_MAC_TABLE_FRWRD_E;
            macEntry.daRoute                            = GT_FALSE;
            macEntry.mirrorToRxAnalyzerPortEn           = GT_FALSE;
            macEntry.saMirrorToRxAnalyzerPortEn         = GT_FALSE;
            macEntry.daMirrorToRxAnalyzerPortEn         = GT_FALSE;
            macEntry.sourceID                           = (iter == 2) ? prvTgfBrgFdbUploadSourceId[2] : prvTgfBrgFdbUploadSourceId[3];
            macEntry.userDefined                        = 0;
            macEntry.daQosIndex                         = 0;
            macEntry.saQosIndex                         = 0;
            macEntry.daSecurityLevel                    = 0;
            macEntry.saSecurityLevel                    = 0;
            macEntry.appSpecificCpuCode                 = GT_TRUE;
            macEntry.age                                = (iter == 2) ? prvTgfBrgFdbUploadAge[2] : prvTgfBrgFdbUploadAge[3];
            macEntry.spUnknown                          = (iter == 2) ? prvTgfBrgFdbUploadSpUnknown[2] : prvTgfBrgFdbUploadSpUnknown[3];
        }
        /* Falcon 6_4 and 12_8 has 2/4 FDBs i.e. FDB per tile which results in
         * FU trigger from each tile. Using portGroupsBmp as 0x1 to write to
         * first tile only so FU will trigger from only that specific tile.
         */
        st = cpssDxChBrgFdbPortGroupMacEntryWrite(prvTgfDevNum, 0x1/*portGroupsBmp*/, iter, GT_FALSE, &macEntry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, prvTgfDevNum, iter);
    }

    /* Set FDB action mode */
    st =  cpssDxChBrgFdbActionModeGet(prvTgfDevNum, &prvTgfRestoreCfg.fdbActionModeGet);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    st =  cpssDxChBrgFdbActionModeSet(prvTgfDevNum, CPSS_FDB_ACTION_AGE_WITHOUT_REMOVAL_E);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* Enable FDB upload */
    st =  cpssDxChBrgFdbUploadEnableGet(prvTgfDevNum, &prvTgfRestoreCfg.fdbUploadEnableGet);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    st =  cpssDxChBrgFdbUploadEnableSet(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* Set FDB trigger action mode */
    st = cpssDxChBrgFdbMacTriggerModeGet(prvTgfDevNum, &prvTgfRestoreCfg.macActionModeGet);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    st = cpssDxChBrgFdbMacTriggerModeSet(prvTgfDevNum,CPSS_ACT_TRIG_E);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* Trigger FDB action */
    st = cpssDxChBrgFdbTrigActionStart(prvTgfDevNum,CPSS_FDB_ACTION_AGE_WITHOUT_REMOVAL_E);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    fuMessage = (CPSS_MAC_UPDATE_MSG_EXT_STC *)cpssOsMalloc(sizeof(CPSS_MAC_UPDATE_MSG_EXT_STC)*4);
    st = (fuMessage == NULL) ? GT_OUT_OF_CPU_MEM : GT_OK;
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, prvTgfDevNum, numOfMsgs);
    cpssOsMemSet(fuMessage, 0, sizeof(CPSS_MAC_UPDATE_MSG_EXT_STC)*4);

    numOfMsgs = PRV_TGF_FDB_UPLOAD_MESSAGE_CNS;
    cpssOsTimerWkAfter(100);
    st = cpssDxChBrgFdbFuMsgBlockGet(prvTgfDevNum, &numOfMsgs, &fuMessage[0]);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, prvTgfDevNum, numOfMsgs);
    UTF_VERIFY_EQUAL2_STRING_MAC(PRV_TGF_FDB_UPLOAD_MESSAGE_CNS, numOfMsgs, "Expected number of FU messages 0x%x is different than received 0x%x\n",
                                 PRV_TGF_FDB_UPLOAD_MESSAGE_CNS, numOfMsgs);

    /* Validate FDB entry based on FDB Upload Message */
    /* iter = 0 corresponds to FU message for CPSS_INTERFACE_VIDX_E
     * iter = 1 corresponds to FU message for CPSS_INTERFACE_VID_E
     * iter = 2 corresponds to FU message for CPSS_INTERFACE_TRUNK_E
     * iter = 3 corresponds to FU message for CPSS_INTERFACE_PORT_E
     */
    for(iter = 0 ; iter < PRV_TGF_FDB_UPLOAD_MESSAGE_CNS; iter++)
    {
        UTF_VERIFY_EQUAL3_STRING_MAC(CPSS_FU_E, fuMessage[iter].updType, "Expected Update Message Type 0x%x is different than received 0x%x\n for index %d",
                                     CPSS_FU_E, fuMessage[iter].updType, iter);

        UTF_VERIFY_EQUAL3_STRING_MAC(GT_FALSE,  fuMessage[iter].entryWasFound, "Expected entryWasFound 0x%x is different than received 0x%x for index %d\n",
                                     GT_FALSE,  fuMessage[iter].entryWasFound, iter);

        if(iter > 1)
        {
            expectedIndex = (iter == 2) ? 2 : 3;
            UTF_VERIFY_EQUAL3_STRING_MAC(prvTgfBrgFdbUploadMacEntryType[expectedIndex],fuMessage[iter].macEntry.key.entryType,
                "Expected entryType 0x%x is different than received 0x%x for index %d\n",
                prvTgfBrgFdbUploadMacEntryType[expectedIndex],fuMessage[iter].macEntry.key.entryType, iter);

            UTF_VERIFY_EQUAL3_STRING_MAC(0x0, fuMessage[iter].macEntry.dstInterface.devPort.hwDevNum,
                "Expected hwDevNum 0x%x is different than received 0x%x for index %d\n",
                hwDevNum,fuMessage[iter].macEntry.dstInterface.devPort.hwDevNum, iter);

            UTF_VERIFY_EQUAL3_STRING_MAC(prvTgfBrgFdbUploadVlanId[expectedIndex],fuMessage[iter].macEntry.key.key.ipMcast.vlanId,
                "Expected vlanId 0x%x is different than received 0x%x for index %d\n",
                prvTgfBrgFdbUploadVlanId[expectedIndex],fuMessage[iter].macEntry.key.key.ipMcast.vlanId, iter);

            UTF_VERIFY_EQUAL3_STRING_MAC(prvTgfBrgFdbUploadDstInterface[expectedIndex],fuMessage[iter].macEntry.dstInterface.type,
                "Expected dstInterfaceType 0x%x is different than received 0x%x for index %d\n",
                prvTgfBrgFdbUploadDstInterface[expectedIndex],fuMessage[iter].macEntry.dstInterface.type, iter);

            if(prvTgfBrgFdbUploadDstInterface[expectedIndex] == CPSS_INTERFACE_VID_E)
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(prvTgfBrgFdbUploadVlanId[expectedIndex],fuMessage[iter].macEntry.dstInterface.vlanId,
                    "Expected dstInterfaceVlanId 0x%x is different than received 0x%x for index %d\n",
                    prvTgfBrgFdbUploadVlanId[expectedIndex],fuMessage[iter].macEntry.dstInterface.vlanId, iter);
            }

            expectedVidx = (prvTgfBrgFdbUploadDstInterface[expectedIndex] == CPSS_INTERFACE_VIDX_E) ?
                           prvTgfBrgFdbUploadVidx[expectedIndex]:
                           0xFFF;
            UTF_VERIFY_EQUAL3_STRING_MAC(expectedVidx,fuMessage[iter].macEntry.dstInterface.vidx,
                "Expected dstInterfaceVidx 0x%x is different than received 0x%x for index %d\n",
                expectedVidx,fuMessage[iter].macEntry.dstInterface.vidx, iter);

            for(ipIter = 1; ipIter < 4; ipIter++)
            {
                if(iter == 3)
                {
                    UTF_VERIFY_EQUAL3_STRING_MAC(prvTgfBrgFdbUploadVidxInterfaceSip[ipIter],fuMessage[iter].macEntry.key.key.ipMcast.sip[ipIter],
                        "Expected source ip address 0x%x is different than received 0x%x at index %d\n",
                        prvTgfBrgFdbUploadVidxInterfaceSip[ipIter],fuMessage[iter].macEntry.key.key.ipMcast.sip[ipIter], ipIter);

                    UTF_VERIFY_EQUAL3_STRING_MAC(prvTgfBrgFdbUploadVidxInterfaceDip[ipIter],fuMessage[iter].macEntry.key.key.ipMcast.dip[ipIter],
                        "Expected dest ip address 0x%x is different than received 0x%x at index %d\n",
                        prvTgfBrgFdbUploadVidxInterfaceDip[ipIter],fuMessage[iter].macEntry.key.key.ipMcast.dip[ipIter], ipIter);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_STRING_MAC(prvTgfBrgFdbUploadVidInterfaceSip[ipIter], fuMessage[iter].macEntry.key.key.ipMcast.sip[ipIter],
                        "Expected source ip address 0x%x is different than received 0x%x at index %d\n",
                        prvTgfBrgFdbUploadVidInterfaceSip[ipIter], fuMessage[iter].macEntry.key.key.ipMcast.sip[ipIter], ipIter);

                    UTF_VERIFY_EQUAL3_STRING_MAC(prvTgfBrgFdbUploadVidInterfaceDip[ipIter], fuMessage[iter].macEntry.key.key.ipMcast.dip[ipIter],
                        "Expected dest ip address 0x%x is different than received 0x%x at index %d\n",
                        prvTgfBrgFdbUploadVidInterfaceDip[ipIter], fuMessage[iter].macEntry.key.key.ipMcast.dip[ipIter], ipIter);
                }
            }

            /* For SIP_5 devices, SourceID is valid only when MACEntryType = "MAC" for FU message */
            if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(prvTgfBrgFdbUploadSourceId[expectedIndex],fuMessage[iter].macEntry.sourceID,
                    "Expected srcId 0x%x is different than received 0x%x for index %d\n",
                    prvTgfBrgFdbUploadSourceId[expectedIndex],fuMessage[iter].macEntry.sourceID, iter);
            }

            UTF_VERIFY_EQUAL3_STRING_MAC(prvTgfBrgFdbUploadAge[expectedIndex],fuMessage[iter].macEntry.age,
                "Expected age flag 0x%x is different than received 0x%x for index %d\n",
                prvTgfBrgFdbUploadAge[expectedIndex],fuMessage[iter].macEntry.age, iter);

            UTF_VERIFY_EQUAL3_STRING_MAC(prvTgfBrgFdbUploadSpUnknown[expectedIndex],fuMessage[iter].macEntry.spUnknown,
                "Expected spUnknown flag 0x%x is different than received 0x%x\n for index %d",
                prvTgfBrgFdbUploadSpUnknown[expectedIndex],fuMessage[iter].macEntry.spUnknown, iter);
        }
        else
        {
            expectedIndex = (iter == 0) ? 0 : 1;
            UTF_VERIFY_EQUAL3_STRING_MAC(prvTgfBrgFdbUploadMacEntryType[expectedIndex],fuMessage[iter].macEntry.key.entryType,
                "Expected entryType 0x%x is different than received 0x%x for index %d\n",
                prvTgfBrgFdbUploadMacEntryType[expectedIndex],fuMessage[iter].macEntry.key.entryType, iter);

            expectedHwDevNum = (prvTgfBrgFdbUploadDstInterface[expectedIndex] == CPSS_INTERFACE_PORT_E) ? hwDevNum : 0x0;
            UTF_VERIFY_EQUAL3_STRING_MAC(expectedHwDevNum, fuMessage[iter].macEntry.dstInterface.devPort.hwDevNum,
                "Expected hwDevNum 0x%x is different than received 0x%x for index %d\n",
                expectedHwDevNum,fuMessage[iter].macEntry.dstInterface.devPort.hwDevNum, iter);

            UTF_VERIFY_EQUAL3_STRING_MAC(prvTgfBrgFdbUploadVlanId[expectedIndex],fuMessage[iter].macEntry.key.key.macVlan.vlanId,
                "Expected vlanId 0x%x is different than received 0x%x for index %d\n",
                prvTgfBrgFdbUploadVlanId[expectedIndex], fuMessage[iter].macEntry.key.key.macVlan.vlanId, iter);

            UTF_VERIFY_EQUAL3_STRING_MAC(prvTgfBrgFdbUploadDstInterface[expectedIndex],fuMessage[iter].macEntry.dstInterface.type,
                "Expected dstInterfaceType 0x%x is different than received 0x%x for index %d\n",
                prvTgfBrgFdbUploadDstInterface[expectedIndex],fuMessage[iter].macEntry.dstInterface.type, iter);

            UTF_VERIFY_EQUAL3_STRING_MAC(0x0, fuMessage[iter].macEntry.dstInterface.vlanId,
                "Expected dstInterfaceVlanId 0x%x is different than received 0x%x for index %d\n",
                0x0,fuMessage[iter].macEntry.dstInterface.vlanId, iter);

            UTF_VERIFY_EQUAL3_STRING_MAC(prvTgfBrgFdbUploadVidx[expectedIndex],fuMessage[iter].macEntry.dstInterface.vidx,
                "Expected dstInterfaceVidx 0x%x is different than received 0x%x for index %d\n",
                prvTgfBrgFdbUploadVidx[expectedIndex],fuMessage[iter].macEntry.dstInterface.vidx, iter);

            for(macIter = 0; macIter < 6; macIter++)
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(prvTgfBrgFdbUploadMacAddr[expectedIndex].arEther[macIter],fuMessage[iter].macEntry.key.key.macVlan.macAddr.arEther[macIter],
                    "Expected mac address 0x%x is different than received 0x%x at index %d\n",
                    prvTgfBrgFdbUploadMacAddr[expectedIndex].arEther[macIter], fuMessage[iter].macEntry.key.key.macVlan.macAddr.arEther[macIter], macIter);
            }
            UTF_VERIFY_EQUAL3_STRING_MAC(prvTgfBrgFdbUploadPortNum[expectedIndex],fuMessage[iter].macEntry.dstInterface.devPort.portNum,
                "Expected dstPortNum 0x%x is different than received 0x%x for index %d\n",
                prvTgfBrgFdbUploadPortNum[expectedIndex],fuMessage[iter].macEntry.dstInterface.devPort.portNum, iter);

            UTF_VERIFY_EQUAL3_STRING_MAC(prvTgfBrgFdbUploadTrunkId[expectedIndex],fuMessage[iter].macEntry.dstInterface.trunkId,
                "Expected dstTrunkId 0x%x is different than received 0x%x for index %d\n",
                prvTgfBrgFdbUploadTrunkId[expectedIndex],fuMessage[iter].macEntry.dstInterface.trunkId, iter);

            expectedSrcId = (prvTgfBrgFdbUploadDstInterface[expectedIndex] == CPSS_INTERFACE_PORT_E) ? prvTgfBrgFdbUploadSourceId[expectedIndex] : 0x0;
            UTF_VERIFY_EQUAL3_STRING_MAC(prvTgfBrgFdbUploadSourceId[expectedIndex],fuMessage[iter].macEntry.sourceID,
                "Expected srcId 0x%x is different than received 0x%x at index %d\n",
                expectedSrcId,fuMessage[iter].macEntry.sourceID, iter);

            UTF_VERIFY_EQUAL3_STRING_MAC(prvTgfBrgFdbUploadAge[expectedIndex],fuMessage[iter].macEntry.age,
                "Expected age flag 0x%x is different than received 0x%x for index %d\n",
                prvTgfBrgFdbUploadAge[expectedIndex],fuMessage[iter].macEntry.age, iter);

            UTF_VERIFY_EQUAL3_STRING_MAC(prvTgfBrgFdbUploadSpUnknown[expectedIndex],fuMessage[iter].macEntry.spUnknown,
                "Expected spUnknown flag 0x%x is different than received 0x%x for index %d\n",
                prvTgfBrgFdbUploadSpUnknown[expectedIndex],fuMessage[iter].macEntry.spUnknown, iter);
        }
    }
    if(fuMessage != NULL)
    {
        cpssOsFree(fuMessage);
    }
}

/**
* @internal prvTgfBrgFdbUploadConfigRestore function
* @endinternal
*
* @brief Restore configuration
*/
GT_VOID prvTgfBrgFdbUploadConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc  = GT_OK;
    GT_U32      hwIndex;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* FDB Upload Configuration Restore */
    rc =  cpssDxChBrgFdbActionModeSet(prvTgfDevNum, prvTgfRestoreCfg.fdbActionModeGet);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    rc =  cpssDxChBrgFdbUploadEnableSet(prvTgfDevNum, prvTgfRestoreCfg.fdbUploadEnableGet);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    rc = cpssDxChBrgFdbMacTriggerModeSet(prvTgfDevNum, prvTgfRestoreCfg.macActionModeGet);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* use entry by entry invalidation to guarantee empty FDB.
       "flushFdb" may leave some SKIPep valid entries when non multi-hash mode is used. */
    for(hwIndex = 0; hwIndex < PRV_TGF_FDB_UPLOAD_MESSAGE_CNS; hwIndex++)
    {
        rc = cpssDxChBrgFdbMacEntryInvalidate(prvTgfDevNum, hwIndex);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbMacEntryInvalidate: %d, %d", prvTgfDevNum, hwIndex);
    }
}

/**
* @internal prvTgfBrgFdbEpgEpclConfig function
* @endinternal
*
* @brief The api to config the EPCL matching the SRC and DST EPG ID,
*           and modify the DSCP of the original packet
*/
GT_VOID prvTgfBrgFdbEpgEpclConfig(GT_U32 srcGrpId, GT_U32 dstGrpId)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT         mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT         pattern;
    PRV_TGF_PCL_ACTION_STC              action;
    GT_U32                              ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT    ruleFormat;
    PRV_TGF_PCL_UDB_SELECT_STC          udbSelect;
    GT_U32                              udbSelectidx;
    PRV_UTF_LOG0_MAC("======= Setting EPCL Configuration =======\n");

    /* AUTODOC: init PCL Engine for Egress PCL for analyzer port */
    rc = prvTgfPclDefPortInit(
            prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_EGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclDefPortInit");

    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
            PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: Configure ePort access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeGet(prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
            &prvTgfEpgRestoreCfg.pclPortAccessModeCfgGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeGet");

    rc = prvTgfPclPortLookupCfgTabAccessModeSet(prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
            PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

    cpssOsMemSet(&udbSelect, 0, sizeof(CPSS_DXCH_PCL_UDB_SELECT_STC));
    udbSelectidx = 0;

    for (udbSelectidx =0 ; udbSelectidx < sizeof(prvTgfEpgEpclMetadataUdbInfo)/(sizeof(prvTgfEpgEpclMetadataUdbInfo[0])); udbSelectidx++)
    {
        /* AUTODOC:   configure User Defined Byte(UDB) */
        /* AUTODOC:   format CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E */
        /* AUTODOC:   packet type PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E */
        /* AUTODOC:   offsetType CPSS_DXCH_PCL_OFFSET_METADATA_E */
        rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
                CPSS_PCL_DIRECTION_EGRESS_E,
                prvTgfEpgEpclMetadataUdbInfo[udbSelectidx].udbIndex,
                prvTgfEpgEpclMetadataUdbInfo[udbSelectidx].offsetType,
                prvTgfEpgEpclMetadataUdbInfo[udbSelectidx].offset);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d for udbSelectidx = %d\n", prvTgfDevNum, udbSelectidx);

        udbSelect.udbSelectArr[udbSelectidx]=prvTgfEpgEpclMetadataUdbInfo[udbSelectidx].udbIndex;
    }

    rc = prvTgfPclUserDefinedBytesSelectSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E, PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
            CPSS_PCL_LOOKUP_0_E, &udbSelect);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedBytesSelectSet: %d\n", prvTgfDevNum);
    /* AUTODOC: set PCL rule 0 */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    pattern.ruleEgrUdbOnly.udb[0] = (srcGrpId & 0x3F) <<2;
    mask.ruleEgrUdbOnly.udb[0] = prvTgfEpgEpclMetadataUdbInfo[0].byteMask;

    pattern.ruleEgrUdbOnly.udb[1] = ((srcGrpId >> 6)&0x07) | ((dstGrpId & 0x1F) << 3);
    mask.ruleEgrUdbOnly.udb[1] = prvTgfEpgEpclMetadataUdbInfo[1].byteMask;

    pattern.ruleEgrUdbOnly.udb[2] = (dstGrpId >> 5)&0x0F;
    mask.ruleEgrUdbOnly.udb[2] = prvTgfEpgEpclMetadataUdbInfo[2].byteMask;


    ruleIndex                  = PRV_TGF_EPCL_RULE_INDEX(0);
    ruleFormat                 = PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E;
    action.pktCmd              = CPSS_PACKET_CMD_FORWARD_E;
    action.qos.dscp            = PRV_TGF_EPG_DSCP_CNS;
    action.qos.egressDscpCmd   = PRV_TGF_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_OUTER_E;
    action.egressPolicy        = GT_TRUE;
    rc = prvTgfPclRuleSet(
            ruleFormat,                
            ruleIndex,
            &mask, &pattern, &action);
    UTF_VERIFY_EQUAL0_STRING_MAC(
            GT_OK, rc, "prvTgfPclRuleSet:");
}
/**
* @internal prvTgfBrgFdbEpgConfigSet function
* @endinternal
*
* @brief The api to config the FDB SA/DA based EPG id.
*    Create VLAN with tagged ports [0,1,2,3]
*    add FDB entry with MAC 00:00:00:00:00:02,  port 2
*    Set FDB entry for DA  match
*    config all SRC-ID bits to be used for EPG bits
*    global fdb based src id assignment
*    config the SRC-ID for source EPG ID based on SA lookup
*    EPCL matching the SRC and DST EPG ID and modify the DSCP of the original packet
*/
GT_VOID prvTgfBrgFdbEpgConfigSet(GT_BOOL setSrcEpg)
{
    GT_STATUS rc;
    PRV_TGF_BRG_MAC_ENTRY_STC prvTgfMacEntry;
    CPSS_DXCH_BRG_FDB_EPG_CONFIG_STC epgConfig;

    /* Create VLAN with tagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(PRV_TGF_VLANID_CNS, PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", PRV_TGF_VLANID_CNS);

    /*config all SRC-ID bits to be used for EPG bits*/   
    rc = cpssDxChBrgFdbMacEntryMuxingModeGet(prvTgfDevNum, &prvTgfEpgRestoreCfg.fdbMacMuxMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbMacEntryMuxingModeGet");
    rc = cpssDxChBrgFdbMacEntryMuxingModeSet(prvTgfDevNum, CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_SRC_ID_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbMacEntryMuxingModeSet");

    if (setSrcEpg)
    {
        /*global fdb based src id assignment*/
        rc = cpssDxChBrgSrcIdGlobalSrcIdAssignModeGet(prvTgfDevNum, &prvTgfEpgRestoreCfg.brgSrcIdAssignMode);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgSrcIdGlobalSrcIdAssignModeGet");
        rc = cpssDxChBrgSrcIdGlobalSrcIdAssignModeSet(prvTgfDevNum, CPSS_BRG_SRC_ID_ASSIGN_MODE_FDB_PORT_DEFAULT_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgSrcIdGlobalSrcIdAssignModeSet");

        /*config the SRC-ID for source EPG ID based on SA lookup*/
        rc = cpssDxChBrgFdbEpgConfigGet(prvTgfDevNum, &prvTgfEpgRestoreCfg.epgConfig);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbEpgConfigGet");
        epgConfig.srcEpgAssignEnable = GT_TRUE;
        epgConfig.dstEpgAssignEnable = GT_FALSE;
        rc = cpssDxChBrgFdbEpgConfigSet(prvTgfDevNum, &epgConfig);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbEpgConfigSet");

        prvTgfBrgFdbEpgEpclConfig(PRV_TGF_SRC_EPG_CNS, 0);
    }
    else
    {
        /*global fdb based src id assignment*/
        rc = cpssDxChBrgSrcIdGlobalSrcIdAssignModeGet(prvTgfDevNum, &prvTgfEpgRestoreCfg.brgSrcIdAssignMode);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgSrcIdGlobalSrcIdAssignModeGet");
        rc = cpssDxChBrgSrcIdGlobalSrcIdAssignModeSet(prvTgfDevNum, CPSS_BRG_SRC_ID_ASSIGN_MODE_FDB_DA_PORT_DEFAULT_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgSrcIdGlobalSrcIdAssignModeSet");

        /*config the SRC-ID for source EPG ID based on DA lookup*/
        rc = cpssDxChBrgFdbEpgConfigGet(prvTgfDevNum, &prvTgfEpgRestoreCfg.epgConfig);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbEpgConfigGet");
        epgConfig.srcEpgAssignEnable = GT_FALSE;
        epgConfig.dstEpgAssignEnable = GT_TRUE;
        rc = cpssDxChBrgFdbEpgConfigSet(prvTgfDevNum, &epgConfig);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbEpgConfigSet");

        prvTgfBrgFdbEpgEpclConfig(0, PRV_TGF_DST_EPG_CNS);
    }
    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:02,  port 2 */
    cpssOsMemSet(&prvTgfMacEntry, 0, sizeof(prvTgfMacEntry));

    /* AUTODOC: Set FDB entry for SA match*/
    prvTgfMacEntry.key.key.macVlan.vlanId = PRV_TGF_VLANID_CNS;
    prvTgfMacEntry.isStatic = GT_TRUE;
    prvTgfMacEntry.sourceId = PRV_TGF_SRC_EPG_CNS;
    prvTgfMacEntry.saCommand  = CPSS_MAC_TABLE_FRWRD_E;
    prvTgfMacEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;
    prvTgfMacEntry.dstInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS];
    prvTgfMacEntry.dstInterface.devPort.hwDevNum = prvTgfDevNum;
    cpssOsMemCpy(prvTgfMacEntry.key.key.macVlan.macAddr.arEther, prvTgfBrgFdbEpgPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfBrgFdbMacEntrySet(&prvTgfMacEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet");

    /* AUTODOC: Set FDB entry for DA  match*/
    prvTgfMacEntry.key.key.macVlan.vlanId = PRV_TGF_VLANID_CNS;
    prvTgfMacEntry.isStatic = GT_TRUE;
    prvTgfMacEntry.sourceId = PRV_TGF_DST_EPG_CNS;
    prvTgfMacEntry.saCommand  = CPSS_MAC_TABLE_FRWRD_E;
    prvTgfMacEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;
    prvTgfMacEntry.dstInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS];
    prvTgfMacEntry.dstInterface.devPort.hwDevNum = prvTgfDevNum;
    cpssOsMemCpy(prvTgfMacEntry.key.key.macVlan.macAddr.arEther, prvTgfBrgFdbEpgPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfBrgFdbMacEntrySet(&prvTgfMacEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet");

}

/**
* @internal prvTgfBrgFdbEpgEpclConfigRestore function
* @endinternal
*
* @brief Restore EPCL configuration for EPG
*/
GT_VOID prvTgfBrgFdbEpgEpclConfigRestore()
{
    PRV_TGF_PCL_LOOKUP_CFG_STC      lookupCfg;
    CPSS_INTERFACE_INFO_STC         interfaceInfo;
    GT_STATUS                       rc;

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.groupKeyTypes.nonIpKey = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key  = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E;
    lookupCfg.groupKeyTypes.ipv6Key  = PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E;

    interfaceInfo.type  = CPSS_INTERFACE_INDEX_E;

        rc = prvTgfPclRuleValidStatusSet(
                CPSS_PCL_RULE_SIZE_STD_E,
                PRV_TGF_EPCL_RULE_INDEX(0),
                GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: rule %d", 0);

        interfaceInfo.index = prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS];
        rc = prvTgfPclCfgTblSet(&interfaceInfo, CPSS_PCL_DIRECTION_EGRESS_E,
                CPSS_PCL_LOOKUP_0_E, &lookupCfg);
        UTF_VERIFY_EQUAL0_STRING_MAC( GT_OK, rc, "prvTgfPclCfgTblSet");

    /* AUTODOC: Restore access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(
            prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_EGRESS_E,
            CPSS_PCL_LOOKUP_0_E,             /* Lookup Number */
            0,                               /* SubLookup Number */
            prvTgfEpgRestoreCfg.pclPortAccessModeCfgGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

    prvTgfPclPortsRestoreAll();
}
/**
* @internal prvTgfBrgSrcIdScalableSgtTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic and test the egress packet DSCP as set by 
*          the EPCL group ID indices.
*/
GT_VOID prvTgfBrgFdbEpgTrafficGenerate
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
    GT_U8                 dscp;
    TGF_NET_DSA_STC       rxParam;
    IN GT_PORT_NUM        inPortId     = prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS];
    IN GT_PORT_NUM        outPortId    = prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS];
    CPSS_INTERFACE_INFO_STC      egressPortInterface;
    GT_BOOL                 getFirst = GT_TRUE;

    /* Update egress portInterface for capturing */
    egressPortInterface.type              = CPSS_INTERFACE_PORT_E;
    egressPortInterface.devPort.hwDevNum  = prvTgfDevNum;
    egressPortInterface.devPort.portNum   = outPortId;

    /* AUTODOC: setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfBrgFdbEpgPacketInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup:");


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
        UTF_VERIFY_EQUAL2_STRING_MAC(PRV_TGF_EPG_DSCP_CNS, dscp,
                                     "rx DSCP[0x%x] != exp DSCP[0x%x]\n", dscp, PRV_TGF_EPG_DSCP_CNS );
}
/**
* @internal prvTgfBrgFdbEpgConfigRestore function
* @endinternal
*
* @brief Restore configuration
*/
GT_VOID prvTgfBrgFdbEpgConfigRestore()
{
    GT_STATUS rc;

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, vlan %d", prvTgfDevNum, PRV_TGF_VLANID_CNS);
    /* Flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    rc = cpssDxChBrgFdbMacEntryMuxingModeSet(prvTgfDevNum, prvTgfEpgRestoreCfg.fdbMacMuxMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbMacEntryMuxingModeGet");

    rc = cpssDxChBrgSrcIdGlobalSrcIdAssignModeSet(prvTgfDevNum, prvTgfEpgRestoreCfg.brgSrcIdAssignMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgSrcIdGlobalSrcIdAssignModeGet");

    rc = cpssDxChBrgFdbEpgConfigSet(prvTgfDevNum, &prvTgfEpgRestoreCfg.epgConfig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbEpgConfigGet");

    prvTgfBrgFdbEpgEpclConfigRestore();

    tgfTrafficTableRxPcktTblClear();
}
