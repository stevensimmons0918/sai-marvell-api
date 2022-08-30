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
* @file prvTgfMirror.c
*
* @brief CPSS Mirror
*
* @version   43
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#if (defined CHX_FAMILY)
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#else
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#endif

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfPclGen.h>
#include <common/tgfMirror.h>
#include <mirror/prvTgfMirror.h>
#include <common/tgfCscdGen.h>
#include <mirror/prvTgfStc.h>
#include <common/tgfIpGen.h>
#include <common/tgfTcamGen.h>
#include <bridge/prvTgfVlanIngressFiltering.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#define HWINIT_GLOVAR(_var) \
    PRV_SHARED_GLOBAL_VAR_GET(commonMod.genericHwInitDir._var)


extern GT_VOID prvTgfBrgE2PhyWaForLargePortsInNoneEdsa(IN GT_BOOL setConfig);

extern GT_BOOL tgfPrvIntendedError;
/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* LENGTH of buffer*/
#define TGF_RX_BUFFER_SIZE_CNS   0x80

/* index for device with single analyzer Rx/Tx */
#define SINGLE_ANALYZER_INDEX_CNS   0

/* default VLAN Id */
#define PRV_TGF_VLANID_CNS  5

/* default VLAN0 Id */
#define PRV_TGF_VLANID_ANOTHER_CNS  8

/* default VLAN0 Id */
#define PRV_TGF_VLANID_17_ANOTHER_CNS  17

/* egress vlan translation */
#define EGRESS_TRANS_VLAN_ID        300

/* ether type used for "vlan tag" */
#define TGF_ETHERTYPE_8100_VLAN_TAG_CNS 0x8100

/* port number to send traffic to */
#define PRV_TGF_PORT_IDX_CNS         3

/* port number to send traffic to */
#define PRV_TGF_PORT_IDX_ANOTHER_CNS         2

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         1

/* an index of cascade port which is vidx member */
#define PRV_TGF_PORT_IDX_VIDX_CSCD_MEMBER 0
/* another port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS 0

/* port number to receive traffic */
#define PRV_TGF_RCV1_PORT_IDX_CNS         2

/* port number to receive traffic */
#define PRV_TGF_RCV_PORT_IDX_CNS          3

/* Egress Source mode test */

/* Tx Mirrored ports */
#define PRV_TGF_TX_MIRR_PORT1_IDX_CNS      2
#define PRV_TGF_TX_MIRR_PORT2_IDX_CNS      3

/* Send port */
#define PRV_TGF_TX_MIRR_SEND_IDX_CNS       1

/* extra analyzer port index */
#define EXTRA_ANALYZER_PORT_INDEX_CNS      4

/* vlan port number */
#define VLAN_PORT_CNS      55

/* index of global analyzer */
#define PRV_TGF_MIRROR_GLOB_ANALYZER_IDX_CNS 3

/* index of enhanced mirroring test analyzer */
#define PRV_TGF_ENHANCED_MIRROR_ANALYZER_IDX_CNS 5

/* TTI floor 1 */
#define PRV_TGF_TTI_FLOOR1_CNS      1

/* TCAM group */
#define GROUP_1                     1

/* TCAM hit 0 */
#define HIT_NUM_0                   0

/* array size for test */
#define PRV_TGF_MIRR_TEST_TX_PACKET_ARRAY_SIZE_CNS 5

/* TCAM floor info saved for restore */
static PRV_TGF_TCAM_BLOCK_INFO_STC saveFloorInfoArr[PRV_TGF_TCAM_MAX_TCAM_BLOCKS_CNS];
#define VIDX_CNS 0xFB9
static GT_U16 usedVidx = VIDX_CNS;
/* VIDX for analyzer that using the vidx */
#define PRV_TGF_MIRROR_ANALYZER_VIDX_CNS    usedVidx
static CPSS_PORTS_BMP_STC      vidxMembersPortBitmap;

/* save ports list */
static GT_U32   prvTgfPortsArraySave[PRV_TGF_MAX_PORTS_NUM_CNS];
static GT_U8   prvTgfPortsNumSave;

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/* get the original sate of the filter on src device -- for DSA tagged traffic */
static GT_BOOL origSrcDevFilter = GT_TRUE;

/* FDB SA Analyzer state */
static GT_BOOL saveFdbSaAnalyzerEnable = GT_FALSE;
static GT_U32  saveFdbSaAnalyzerIndex = 0;

/* (egress) vlan translation */
static GT_U16  vlanTranslationEntryDef;

/******************************* Test packet **********************************/
/* default FDB entry macAddr */
static TGF_MAC_ADDR prvTgfFdbEntryMacAddr = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}                /* srcMac */
};

/* L2 part of UC packet 1 */
static TGF_PACKET_L2_STC prvTgfPacketL2PartUC1 =
{
    {0x00, 0x11, 0x22, 0x33, 0x44, 0x55},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x22, 0x22}                /* srcMac */
};

/* L2 part of UC packet 2 */
static TGF_PACKET_L2_STC prvTgfPacketL2PartUC2 =
{
    {0x00, 0xaa, 0xbb, 0xcc, 0xdd, 0xee},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x33, 0x33}                /* srcMac */
};

/* L2 part of UC packet 1 for learning */
static TGF_PACKET_L2_STC prvTgfPacketL2PartUC1Learn =
{
    {0x00, 0x00, 0x00, 0x00, 0x33, 0x33},               /* dstMac */
    {0x00, 0x11, 0x22, 0x33, 0x44, 0x55}                /* srcMac */
};

/* L2 part of UC packet 2 for learning */
static TGF_PACKET_L2_STC prvTgfPacketL2PartUC2Learn =
{
    {0x00, 0x00, 0x00, 0x00, 0x44, 0x55},               /* dstMac */
    {0x00, 0xaa, 0xbb, 0xcc, 0xdd, 0xee}                /* srcMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};


/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x02, 0x81, 0x00, 0x05, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};
#define VLAN_TAG_INFO_INDEX  1 /* index in prvTgfPacketPartArray[]*/
#define DSA_INFO_INDEX  2 /* index in prvTgfPacketPartArray[]*/

/* indication if current run is for DSA tag testing */
static GT_BOOL      testingDsaTag = GT_FALSE;
/* mirroring mode */
static PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT   testingMirrorMode;

#define DUMMY_REMOTE_DEV_NUM_CNS    (30)

/* restore e2Phy configurations */
static void restoreE2Phy(void);

/* the analyzer target eport - relevant to target analyzer on NOT in "hop by hop" mode */
#define TARGET_EPORT_NUM_CNS    ALIGN_EPORT_TO_512_TILL_MAX_DEV_MAC(0x1EEE)

/* dummy value : the analyzer target eport - used in "hop by hop" mode */
#define DUMMY_TARGET_EPORT_NUM_CNS    ALIGN_EPORT_TO_512_TILL_MAX_DEV_MAC(0x1DDD)

static GT_U32   dsaTagTrgDevPortIndex[4] =
{
    0,/*index in prvTgfPortsArray[],prvTgfDevsArray[] to be used when packet ingress index 0*/
    0,/*index in prvTgfPortsArray[],prvTgfDevsArray[] to be used when packet ingress index 1*/
    0,/*index in prvTgfPortsArray[],prvTgfDevsArray[] to be used when packet ingress index 2*/
    0 /*index in prvTgfPortsArray[],prvTgfDevsArray[] to be used when packet ingress index 3*/
};

static TGF_DSA_DSA_TO_ANALYZER_STC prvTgfPacketDsaTagPart_toAnalyzer = {
    GT_TRUE,/*rxSniffer - ingress mirroring */
    GT_TRUE,/*isTagged*/

    {/* the sniffed port info - src (for rx mirror) or dest (from tx mirror) */
        0,/*devNum*/
        0,/*portNum*/
        0/*ePort*/
    },/*devPort*/

    CPSS_INTERFACE_PORT_E,/*analyzerTrgType*/
    {{0}}, /*union*/
    0
};

static TGF_DSA_DSA_TO_ANALYZER_STC  prvTgfPacketDsaTagPart_toAnalyzer1 = {
    GT_FALSE,/*rxSniffer - egress mirroring */
    GT_FALSE,/*isTagged*/

    {/* the sniffed port info - src (for rx mirror) or dest (from tx mirror) */
        16,/*devNum*/
        0,/*portNum*/
        0,/*ePort*/
    },/*devPort*/

    CPSS_INTERFACE_VIDX_E, /*analyzerTrgType*/
    {
        {
            0/*set at run time PRV_TGF_MIRROR_ANALYZER_VIDX_CNS */    /*Analyzer eVIDX*/
        }
    }, /*union*/
    0
};


static TGF_PACKET_DSA_TAG_STC  prvTgfPacketDsaTagPart = {
    TGF_DSA_CMD_TO_ANALYZER_E ,/*dsaCommand*/
    TGF_DSA_2_WORD_TYPE_E ,/*dsaType*/

    /*TGF_DSA_DSA_COMMON_STC*/
    {
        0,        /*vpt*/
        0,        /*cfiBit*/
        PRV_TGF_VLANID_CNS, /*vid*/
        GT_FALSE, /*dropOnSource*/
        GT_FALSE  /*packetIsLooped*/
    },/*commonParams*/


    {
        {
            GT_FALSE, /* isEgressPipe */
            GT_FALSE, /* isTagged */
            0,        /* hwDevNum */
            GT_FALSE, /* srcIsTrunk */
            {
                0, /* srcTrunkId */
                0, /* portNum */
                0  /* ePort */
            },        /* interface */
            0,        /* cpuCode */
            GT_FALSE, /* wasTruncated */
            0,        /* originByteCount */
            0,        /* timestamp */
            GT_FALSE, /* packetIsTT */
            {
                0 /* flowId */
            },        /* flowIdTtOffset */
            0
        } /* TGF_DSA_DSA_TO_CPU_STC */

    }/* dsaInfo */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_SKIP_E    , NULL},/* will be used for DSA tag testing */
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};


static TGF_PACKET_PART_STC prvTgfPacketPartArrayUC1[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2PartUC1},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

static TGF_PACKET_PART_STC prvTgfPacketPartArrayUC2[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2PartUC2},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};


static TGF_PACKET_PART_STC prvTgfPacketPartArrayUC1Learn[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2PartUC1Learn},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

static TGF_PACKET_PART_STC prvTgfPacketPartArrayUC2Learn[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2PartUC2Learn},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};


/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfoUC1 =
{
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArrayUC1) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArrayUC1                                        /* partsArray */
};


static TGF_PACKET_STC prvTgfPacketInfoUC2 =
{
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArrayUC2) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArrayUC2                                        /* partsArray */
};

static TGF_PACKET_STC prvTgfPacketInfoUC1Learn =
{
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArrayUC1Learn) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArrayUC1Learn                                        /* partsArray */
};


static TGF_PACKET_STC prvTgfPacketInfoUC2Learn =
{
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArrayUC2Learn) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArrayUC2Learn                                        /* partsArray */
};

/* IPv4 */
/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvIpv4TgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_ANOTHER_CNS                       /* pri, cfi, VlanId */
};
/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {
    TGF_ETHERTYPE_0800_IPV4_TAG_CNS
};
/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                                 /* version */
    5,                                 /* headerLen */
    0,                                 /* typeOfService */
    0x2A,                              /* totalLen = 46 */
    0,                                 /* id */
    0,                                 /* flags */
    0,                                 /* offset */
    0x40,                              /* timeToLive = 64 */
    0x04,                              /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    { 1,  1,  1,  1},                  /* srcAddr = ipSrc */
    { 224,1,  1,  1}                   /* dstAddr = ipGrp */
};

/* DATA of packet len = 26 (0x16) */
static GT_U8 prvTgfIpv4PayloadDataArr[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfIPv4PacketPayloadPartEnahancedMirrorTest = {
    sizeof(prvTgfIpv4PayloadDataArr),                       /* dataLength */
    prvTgfIpv4PayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfIPv4PacketPartArrayEnahancedMirrorTest[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2PartUC1},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvIpv4TgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfIPv4PacketPayloadPartEnahancedMirrorTest}
};

/* LENGTH of packet */
#define PRV_TGF_IPV4_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfIpv4PayloadDataArr)

/* PACKET to send */
static TGF_PACKET_STC prvTgfIPv4PacketInfoEnahancedMirrorTest = {
    PRV_TGF_IPV4_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfIPv4PacketPartArrayEnahancedMirrorTest) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfIPv4PacketPartArrayEnahancedMirrorTest                                        /* partsArray */
};
/******************************************************************************\
 *                           Private declarations                             *
\******************************************************************************/

/* default enable/disable mirroring */
static GT_BOOL   prvTgfEnableMirror = GT_FALSE;

/* default analyzer destination interface index */
static GT_U32    prvTgfIndex        = 0;

/* Previously set monitoring interface for specified analyzer index */
static PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC originalAnalyzerInterface;

/* Previously set forwarding mode to analyzer port */
static PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT originalForwardingMode;

/* index of Tx Mirrored port */
#define PRV_IDX_TEST_TX_MIRR_PORT_IDX_CNS 0

/* the analyzer eport mapped to physical vidx ... so the eDSA hold 'vidx' info */
static TGF_DSA_DSA_TO_ANALYZER_STC prvTgfPacketDsaTagPart_toAnalyzer_vidx = {
    GT_TRUE                         /*rxSniffer;*/,

    GT_TRUE                         /*isTagged;*/,

    /*struct*/{
        0/*GT_HW_DEV_NUM               hwDevNum;*/,
        0/*GT_PHYSICAL_PORT_NUM        portNum;*/,

        /* supported in extended, 4 words,  DSA tag */
        0/*GT_PORT_NUM                 ePort;*/,
    }/*devPort;*/,

    /* supported in extended, 4 words,  DSA tag */
    CPSS_INTERFACE_VIDX_E/*CPSS_INTERFACE_TYPE_ENT         analyzerTrgType;*/,

    {{0}}/*union - extDestInfo*/,

    0 /*GT_U32                          tag0TpidIndex;*/
};

static TGF_PACKET_DSA_TAG_STC  prvTgfPacketDsaTagPart_vidx = {
    TGF_DSA_CMD_TO_ANALYZER_E ,/*dsaCommand*/
    TGF_DSA_4_WORD_TYPE_E ,/*dsaType*/

    /*TGF_DSA_DSA_COMMON_STC*/
    {
        0,        /*vpt*/
        0,        /*cfiBit*/
        PRV_TGF_VLANID_CNS, /*vid*/
        GT_FALSE, /*dropOnSource*/
        GT_FALSE  /*packetIsLooped*/
    },/*commonParams*/


    {
        {
            GT_FALSE, /* isEgressPipe */
            GT_FALSE, /* isTagged */
            0,        /* hwDevNum */
            GT_FALSE, /* srcIsTrunk */
            {
                0, /* srcTrunkId */
                0, /* portNum */
                0  /* ePort */
            },        /* interface */
            0,        /* cpuCode */
            GT_FALSE, /* wasTruncated */
            0,        /* originByteCount */
            0,        /* timestamp */
            GT_FALSE, /* packetIsTT */
            {
                0 /* flowId */
            },        /* flowIdTtOffset */
            0
        } /* TGF_DSA_DSA_TO_CPU_STC */

    }/* dsaInfo */
};


/* PARTS of the dummy packet for capturing egress DSA traffic */
static TGF_PACKET_PART_STC dummyExpectedPacketPartArray_vidx[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2PartUC1},
    {TGF_PACKET_PART_DSA_TAG_E,  &prvTgfPacketDsaTagPart_vidx},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* the dummy packet for capturing egress DSA traffic */
static TGF_PACKET_STC dummyExpectedPacket_vidxInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                                         /* totalLen */
    sizeof(dummyExpectedPacketPartArray_vidx) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    dummyExpectedPacketPartArray_vidx                                        /* partsArray */
};

/* PARTS of the dummy packet for capturing egress DSA traffic */
static TGF_PACKET_PART_STC origIngressPacketPartArray_vidx[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2PartUC1},
    {TGF_PACKET_PART_DSA_TAG_E,  &prvTgfPacketDsaTagPart_vidx},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

static TGF_PACKET_STC origIngressPacket_vidxInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                                         /* totalLen */
    sizeof(origIngressPacketPartArray_vidx) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    origIngressPacketPartArray_vidx                                        /* partsArray */
};

static TGF_DSA_DSA_FORWARD_STC  prvTgfPacketDsaTagPart_forward = {
    PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E /*GT_TRUE*/ , /*srcIsTagged*/ /* set to GT_TRUE for the copy to CPU , to be with vlan tag 0 */
    8,/*srcHwDev*/
    GT_FALSE,/* srcIsTrunk */
    /*union*/
    {
        /*trunkId*/
        5/*portNum*/
    },/*source;*/

    13,/*srcId*/

    GT_FALSE,/*egrFilterRegistered*/
    GT_FALSE,/*wasRouted*/
    0,/*qosProfileIndex*/

    /*CPSS_INTERFACE_INFO_STC         dstInterface*/
    {
        CPSS_INTERFACE_PORT_E,/*type*/

        /*struct*/{
            0,/*devNum*/
            15/*portNum*/
        },/*devPort*/

        0,/*trunkId*/
        0, /*vidx*/
        0,/*vlanId*/
        0,/*devNum*/
        0,/*fabricVidx*/
        0 /*index*/
    },/*dstInterface*/
    GT_FALSE,/*isTrgPhyPortValid*/
    0,/*dstEport --> filled in runtime */
    0,/*tag0TpidIndex*/
    GT_FALSE,/*origSrcPhyIsTrunk*/
    /* union */
    {
        /*trunkId*/
        19/*portNum*/
    },/*origSrcPhy*/
    GT_FALSE,/*phySrcMcFilterEnable*/
    0, /* hash */
    GT_TRUE /*skipFdbSaLookup*/
};

static TGF_PACKET_DSA_TAG_STC  prvTgfPacketForwardDsaTagPart_vidx = {
    TGF_DSA_CMD_FORWARD_E ,/*dsaCommand*/
    TGF_DSA_4_WORD_TYPE_E ,/*dsaType*/

    /*TGF_DSA_DSA_COMMON_STC*/
    {
        0,        /*vpt*/
        0,        /*cfiBit*/
        PRV_TGF_VLANID_CNS, /*vid*/
        GT_FALSE, /*dropOnSource*/
        GT_FALSE  /*packetIsLooped*/
    },/*commonParams*/


    {
        {
            GT_FALSE, /* isEgressPipe */
            GT_FALSE, /* isTagged */
            0,        /* hwDevNum */
            GT_FALSE, /* srcIsTrunk */
            {
                0, /* srcTrunkId */
                0, /* portNum */
                0  /* ePort */
            },        /* interface */
            0,        /* cpuCode */
            GT_FALSE, /* wasTruncated */
            0,        /* originByteCount */
            0,        /* timestamp */
            GT_FALSE, /* packetIsTT */
            {
                0 /* flowId */
            },        /* flowIdTtOffset */
            0
        } /* TGF_DSA_DSA_TO_CPU_STC */

    }/* dsaInfo */
};

/*************************** Restore config ***********************************/


/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/
static void initVidxToUse(void)
{
    GT_STATUS   rc;
    GT_U32  vidxNum;

    rc = prvUtfCfgTableNumEntriesGet(prvTgfDevNum,PRV_TGF_CFG_TABLE_VIDX_E,&vidxNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvUtfCfgTableNumEntriesGet: PRV_TGF_CFG_TABLE_VIDX_E : devNum %d",
                                 prvTgfDevNum);
    /* support device with less than 4K vidx */
    usedVidx %= vidxNum;
    if(VIDX_CNS > vidxNum)
    {
        cpssOsPrintf("NOTE: the device supports [%d] vidxs so use vidx[%d] instead of[%d] \n",
            vidxNum,usedVidx,VIDX_CNS);
    }
}

/**
* @internal prvTgfMirrorTcamSingleHitConfig function
* @endinternal
*
* @brief   Function configures all TTI TCAM floor for single hit
*
* @param[in] floorIndex               - TTI TCAM floor index
* @param[in] group                    - TTI TCAM group
* @param[in] hit                      - TTI TCAM lookup per floor
*                                       None
*/
static GT_VOID prvTgfMirrorTcamSingleHitConfig
(
    GT_U32 floorIndex,
    GT_U32 group,
    GT_U32 hit
)
{
    GT_U32 ii;
    PRV_TGF_TCAM_BLOCK_INFO_STC testFloorInfoArr[PRV_TGF_TCAM_MAX_TCAM_BLOCKS_CNS];
    GT_STATUS rc;

    if(prvTgfTcamFloorsNumGet() > 3)
    {
        return;
    }

    /* AUTODOC: Save TCAM floor info for restore */
    rc = prvTgfTcamIndexRangeHitNumAndGroupGet(floorIndex, &saveFloorInfoArr[0]);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfTcamIndexRangeHitNumAndGroupGet: %d", floorIndex);

    /* AUTODOC: Floor index 1: use all blocks for HIT_NUM_0 */
    for (ii = 0; ii < CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS; ii++)
    {
        testFloorInfoArr[ii].hitNum = hit;
        testFloorInfoArr[ii].group  = group;
    }

    rc = prvTgfTcamIndexRangeHitNumAndGroupSet(floorIndex, &testFloorInfoArr[0]);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfTcamIndexRangeHitNumAndGroupSet: %d",
        floorIndex);
}

/**
* @internal prvTgfMirrorTcamRestore function
* @endinternal
*
* @brief   Function restore TTI TCAM floor setup
*/
static GT_VOID prvTgfMirrorTcamRestore
(
    GT_U32 floorIndex
)
{
    GT_STATUS rc;

    if(prvTgfTcamFloorsNumGet() > 3)
    {
        return;
    }

    rc = prvTgfTcamIndexRangeHitNumAndGroupSet(floorIndex, &saveFloorInfoArr[0]);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfTcamIndexRangeHitNumAndGroupSet: %d",
        floorIndex);
}

/**
* @internal prvTgfMirrorTestPacketSend function
* @endinternal
*
* @brief   Function sends packet, performs trace and check expected results.
*
* @param[in] portNum                  - port number to send traffic from
* @param[in] packetInfoPtr            - PACKET to send
*                                       None
*/
static GT_VOID prvTgfMirrorTestPacketSend
(
    IN GT_U32           portNum,
    IN TGF_PACKET_STC *packetInfoPtr
)
{
    GT_STATUS       rc = GT_OK;

    /* use different 'etherType for vlan tagging' instead of 0x8100 */
    /* this will make our packet that has '0x8100' to be recognized as 'untagged'
      by the 'traffic generator' and also the CPSS will not override the ethertype
      of tag0 with the DSA tag */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount);

    if(testingDsaTag == GT_TRUE)
    {
        /* do this right before the send of packet */
        /* bind the DSA tag TO_ANALYZER part (since union is used .. can't do it in compilation time) */
        prvTgfPacketDsaTagPart.dsaInfo.toAnalyzer = prvTgfPacketDsaTagPart_toAnalyzer;

        rc = tgfTrafficGeneratorIngressCscdPortEnableSet(prvTgfDevNum,portNum,GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                     prvTgfDevNum, portNum);

    }

    /* send Packet from port portNum */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, portNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, portNum);

    if(testingDsaTag == GT_TRUE)
    {
        rc = tgfTrafficGeneratorIngressCscdPortEnableSet(prvTgfDevNum,portNum,GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                     prvTgfDevNum, portNum);

    }

    /* restore default ethertype */
    tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
}

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvMirrorAnalyzerInterfaceSet function
* @endinternal
*
* @brief   This function sets analyzer interface.
*/
static GT_VOID prvMirrorAnalyzerInterfaceSet
(
    IN GT_U32    index,
    IN GT_U32    analyzerDevNum,
    IN GT_U32    analyzerPort
)
{
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC  interface;
    GT_STATUS    rc = GT_OK;

    interface.interface.type = CPSS_INTERFACE_PORT_E;
    interface.interface.devPort.hwDevNum = analyzerDevNum;
    interface.interface.devPort.portNum = analyzerPort;

    /* set analyzer interface */
    rc = prvTgfMirrorAnalyzerInterfaceSet(index, &interface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceSet: %d", index);
}

/**
* @internal prvTgfIngressMirrorHopByHopConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set analyzer interface for index 4.
*         Device number 0, port number 18, port interface.
*         -    Set analyzer interface for index 0.
*         Device number 0, port number 23, port interface.
*         -    Set analyzer interface index to 4 and enable Rx mirroring.
*         - Enable Rx mirroring on port 8.
*/
GT_VOID prvTgfIngressMirrorHopByHopConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;


    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                 PRV_TGF_VLANID_CNS);

    /* AUTODOC: set analyzer interface index=4: */
    /* AUTODOC:   analyzer devNum=0, port=2 */
    prvMirrorAnalyzerInterfaceSet(4, prvTgfDevsArray[2], prvTgfPortsArray[2]);

    /* AUTODOC: set analyzer interface index=0: */
    /* AUTODOC:   analyzer devNum=0, port=3 */
    prvMirrorAnalyzerInterfaceSet(0, prvTgfDevsArray[3], prvTgfPortsArray[3]);

    /* save default analyzer interface */
    rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexGet(prvTgfDevNum, &prvTgfEnableMirror, &prvTgfIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxGlobalAnalyzerInterfaceIndexGet: %d", prvTgfDevNum);

    /* AUTODOC: enable global Rx mirroring index=4 for HopByHop mode */
    rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet(GT_TRUE, 4);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet: %d %d", GT_TRUE, 4);

    if(testingDsaTag == GT_TRUE)
    {
        /* in 'hop by hop forwarding mode' the DSA tag analyzer info is ignored*/
        prvTgfPacketDsaTagPart_toAnalyzer.rxSniffer = GT_TRUE;
        prvTgfPacketDsaTagPart_toAnalyzer.isTagged = GT_TRUE;
        prvTgfPacketDsaTagPart_toAnalyzer.devPort.hwDevNum = DUMMY_REMOTE_DEV_NUM_CNS;/*dummy device*/
        prvTgfPacketDsaTagPart_toAnalyzer.devPort.portNum = CPSS_NULL_PORT_NUM_CNS;/*dummy port*/
        prvTgfPacketDsaTagPart_toAnalyzer.devPort.ePort = DUMMY_TARGET_EPORT_NUM_CNS - 1;/*dummy eport*/

        if(prvTgfPacketDsaTagPart.dsaType == TGF_DSA_4_WORD_TYPE_E)
        {
            prvTgfPacketDsaTagPart_toAnalyzer.analyzerTrgType = CPSS_INTERFACE_PORT_E;
            prvTgfPacketDsaTagPart_toAnalyzer.extDestInfo.devPort.analyzerIsTrgPortValid = GT_FALSE;
            prvTgfPacketDsaTagPart_toAnalyzer.extDestInfo.devPort.analyzerHwTrgDev = DUMMY_REMOTE_DEV_NUM_CNS - 2;/*dummy device*/
            prvTgfPacketDsaTagPart_toAnalyzer.extDestInfo.devPort.analyzerTrgPort = CPSS_NULL_PORT_NUM_CNS - 2;/*dummy port*/
            prvTgfPacketDsaTagPart_toAnalyzer.extDestInfo.devPort.analyzerTrgEport = DUMMY_TARGET_EPORT_NUM_CNS;

            prvTgfPacketDsaTagPart_toAnalyzer.tag0TpidIndex = 5;
        }
        else
        {
            prvTgfPacketDsaTagPart_toAnalyzer.analyzerTrgType   = (CPSS_INTERFACE_TYPE_ENT)        0xFFFFFFFF;
            prvTgfPacketDsaTagPart_toAnalyzer.extDestInfo.devPort.analyzerIsTrgPortValid = (GT_BOOL)        0xFFFFFFFF;
            prvTgfPacketDsaTagPart_toAnalyzer.extDestInfo.devPort.analyzerHwTrgDev         = (GT_HW_DEV_NUM)  0xFFFFFFFF;
            prvTgfPacketDsaTagPart_toAnalyzer.extDestInfo.devPort.analyzerTrgPort        = (GT_PHYSICAL_PORT_NUM)0xFFFFFFFF;
            prvTgfPacketDsaTagPart_toAnalyzer.extDestInfo.devPort.analyzerTrgEport       = (GT_PORT_NUM)    0xFFFFFFFF;

            prvTgfPacketDsaTagPart_toAnalyzer.tag0TpidIndex = (GT_U32)    0xFFFFFFFF;
        }

    }
    else
    {
        /* AUTODOC: enable Rx mirroring on port=1, index=0 */
        rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE, 0);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet: %d %d",
                                     prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);
    }
}

/**
* @internal prvTgfIngressMirrorHopByHopTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 0, 23.
*         2 Packets are captured on port 18.
*         Disable Rx mirroring.
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         Packet is captured on ports 0, 18, 23.
*         Set analyzer interface index to 0 and enable Rx mirroring.
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 0, 18.
*         2 Packets are captured on port 23.
*         Disable Rx mirroring on port 8.
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         Packet is captured on ports 0, 18, 23.
*/
GT_VOID prvTgfIngressMirrorHopByHopTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_U32      numTxPackets;


    /* AUTODOC: GENERATE TRAFFIC: */

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: send BC packet from port 1 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:02, VID=5 */
    prvTgfMirrorTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

    /* AUTODOC: verify traffic: */
    /* AUTODOC:   port 2: 2 packets - original flood, copy to analyzer */
    /* AUTODOC:   ports 0,3: 1 packet - original flood */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {

            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        /* Verify that port 18 received 2 packets */
        if (PRV_TGF_RCV1_PORT_IDX_CNS == portIter)
        {
            if(testingDsaTag == GT_TRUE)
            {
                /* 1 packet per send : copy to analyzer (we not send flood) */
                numTxPackets = prvTgfBurstCount;
            }
            else
            {
                /* 2 packets per send : original flood + copy to analyzer */
                numTxPackets = prvTgfBurstCount * 2;
            }

            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(numTxPackets, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
        else
        {
            if(testingDsaTag == GT_TRUE)
            {
                /* 0 packets per send : we not send flood */
                numTxPackets = 0;
            }
            else
            {
                /* 1 packets per send : original flood  */
                numTxPackets = prvTgfBurstCount;
            }

            UTF_VERIFY_EQUAL0_STRING_MAC(numTxPackets, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    if(testingDsaTag == GT_TRUE)
    {
        /* there is no meaning to check 'disabled mirroring' for DSA tagged packets */
        goto newGlobalAnalyzer_lbl;
    }

    if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily >= CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        /* Disable Rx mirroring on sender port */
        rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE, 0);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet: %d %d",
                                     prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    }

    /* AUTODOC: disable global Rx mirroring index=0 for HopByHop mode */
    rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet(GT_FALSE, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet: %d", GT_FALSE);


    /* AUTODOC: send BC packet from port 1 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:02, VID=5 */
    prvTgfMirrorTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

    /* AUTODOC: verify traffic: */
    /* AUTODOC:   ports 0,2,3: 1 packet - original flood */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {

            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        /* Verify that all ports except of send port received only 1 packet.*/
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily >= CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        /* Enable Rx mirroring on sender port */
        rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE, 0);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet: %d %d",
                                     prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);
    }

newGlobalAnalyzer_lbl:

        /* AUTODOC: enable global Rx mirroring index=0 for HopByHop mode */
    rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet(GT_TRUE, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet: %d %d", GT_TRUE, 0);

    /* AUTODOC: send BC packet from port 1 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:02, VID=5 */
    prvTgfMirrorTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

    /* AUTODOC: verify traffic: */
    /* AUTODOC:   port 2: 2 packets - original flood, copy to analyzer */
    /* AUTODOC:   ports 0,3: 1 packet - original flood */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        /* Verify that port 23 received 2 packets */
        if (PRV_TGF_RCV_PORT_IDX_CNS == portIter)
        {
            if(testingDsaTag == GT_TRUE)
            {
                /* 1 packet per send : copy to analyzer (we not send flood) */
                numTxPackets = prvTgfBurstCount;
            }
            else
            {
                /* 2 packets per send : original flood + copy to analyzer */
                numTxPackets = prvTgfBurstCount * 2;
            }

            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(numTxPackets, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
        else
        {
            if(testingDsaTag == GT_TRUE)
            {
                /* 0 packets per send : we not send flood */
                numTxPackets = 0;
            }
            else
            {
                /* 1 packets per send : original flood  */
                numTxPackets = prvTgfBurstCount;
            }
            UTF_VERIFY_EQUAL0_STRING_MAC(numTxPackets, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    if(testingDsaTag == GT_TRUE)
    {
        /* there is no meaning to check 'disabled mirroring' for DSA tagged packets */
        return;
    }

    /* AUTODOC: disable Rx mirroring on port=1, index=0 */
    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet: %d %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);

    /* AUTODOC: send BC packet from port 1 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:02, VID=5 */
    prvTgfMirrorTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

    /* AUTODOC: verify traffic: */
    /* AUTODOC:   ports 0,2,3: 1 packet - original flood */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        /* Verify that all ports except of send port received only 1 packet.*/
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}

/**
* @internal prvTgfIngressMirrorHopByHopConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfIngressMirrorHopByHopConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: restore global Rx mirroring configuration */
    rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet(prvTgfEnableMirror, prvTgfIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet: %d", prvTgfEnableMirror);

    /* AUTODOC: disable Rx mirroring on port=1, index=0 */
    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet: %d %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);

    /* invalidate VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_VLANID_CNS);

    /* restore e2Phy configurations */
    restoreE2Phy();

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* restore DSA tag testing mode */
    prvTgfMirrorConfigParamsSet(GT_FALSE,TGF_DSA_2_WORD_TYPE_E,PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E);
}

extern void prvTgfLimitedHwDevNumSet(IN GT_U32  limitHwDevNum);
/**
* @internal prvTgfIngressMirrorSourceBasedConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set source-based forwarding mode.
*         - Set analyzer interface for index 6.
*         Device number 0, port number 18, port interface.
*         - Set analyzer interface for index 0.
*         Device number 0, port number 23, port interface.
*         - Enable Rx mirroring on port 8 and set analyzer
*         interface index for the port to 6.
*         - Enable Rx mirroring on port 0 and set analyzer interface
*         index for the port to 0.
*/
GT_VOID prvTgfIngressMirrorSourceBasedConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum) && GT_TRUE == testingDsaTag)
    {
        /* need to ensure that rx ports are able to be cascade */

        /*save prvTgfPortsArray */
        cpssOsMemCpy(prvTgfPortsArraySave, prvTgfPortsArray,
                     sizeof(prvTgfPortsArraySave));

        rc = prvTgfDefPortsArrange(GT_FALSE,
                                   PRV_TGF_SEND_PORT_IDX_CNS,
                                   PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS,
                                   -1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                "prvTgfPortsArray rearrangement can't be done");

    }

    if(testingDsaTag == GT_TRUE &&
       prvTgfPacketDsaTagPart.dsaType == TGF_DSA_2_WORD_TYPE_E)
    {
        /* the ownDevNum must be limited to 0x1f that can be passed on such DSA */
        /* this is needed for runs with random hwDevNum                         */
        prvTgfLimitedHwDevNumSet(0x1f);
    }

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                 PRV_TGF_VLANID_CNS);

    /* AUTODOC: set Source-Based forwarding mode for mirroring to analyzer */
    rc = (testingMirrorMode ==  PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E ||
         testingMirrorMode ==  PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E) ? GT_OK : GT_FAIL;
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "bad testingMirrorMode: %d", testingMirrorMode);

    /*  Set source-based forwarding mode. */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(testingMirrorMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorToAnalyzerForwardingModeSet: %d",
                                 testingMirrorMode);

    if(testingDsaTag == GT_TRUE)
    {
        /* in 'source-based forwarding mode' the DSA tag should give all the relevant info */
        prvTgfPacketDsaTagPart_toAnalyzer.rxSniffer = GT_TRUE;
        prvTgfPacketDsaTagPart_toAnalyzer.isTagged = GT_TRUE;
        /* state that rx mirroring came and should egress [index[2]]*/
        dsaTagTrgDevPortIndex[0] = 3;/*packet from index 0 is mirrored to analyzer port in index 3 */
        /* state that rx mirroring came and should egress [index[3]]*/
        dsaTagTrgDevPortIndex[1] = 2;/*packet from index 1 is mirrored to analyzer port in index 2 */

        if( prvTgfPacketDsaTagPart.dsaType == TGF_DSA_2_WORD_TYPE_E)
        {
            prvTgfBrgE2PhyWaForLargePortsInNoneEdsa(GT_TRUE);
        }
    }
    else
    {
            /* AUTODOC: set analyzer interface index=6: */
            /* AUTODOC:   analyzer devNum=0, port=2 */
        prvMirrorAnalyzerInterfaceSet(6, prvTgfDevsArray[2], prvTgfPortsArray[2]);

        /* AUTODOC: set analyzer interface index=0: */
        /* AUTODOC:   analyzer devNum=0, port=3 */
        prvMirrorAnalyzerInterfaceSet(0, prvTgfDevsArray[3], prvTgfPortsArray[3]);

        /* AUTODOC: enable Rx mirroring on port=1, index=6 */
        rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[1], GT_TRUE, 6);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet: %d %d %d",
                                     prvTgfPortsArray[1], GT_TRUE, 6);

        /* AUTODOC: enable Rx mirroring on port=0, index=0 */
        rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[0], GT_TRUE, 0);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet: %d %d %d",
                                     prvTgfPortsArray[0], GT_TRUE, 0);
    }
}

/**
* @internal prvTgfIngressMirrorSourceBasedTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 0, 23.
*         2 Packets are captured on port 18.
*         Send to device's port 0 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 0, 18.
*         2 Packets are captured on port 23.
*         Disable Rx mirroring on port 8.
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         Packet is captured on ports 0, 18, 23.
*         Disable Rx mirroring on port 0.
*         Send to device's port 0 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         Packet is captured on ports 8, 18, 23.
*/
GT_VOID prvTgfIngressMirrorSourceBasedTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_U32      numTxPackets;
    CPSS_INTERFACE_INFO_STC physicalInfo;/* physical port info for the analyzer in eArch */
    GT_PORT_NUM analyzerTrgEPort;/* analyzer target eport */


    /* AUTODOC: GENERATE TRAFFIC: */

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    if(testingDsaTag == GT_TRUE)
    {
        prvTgfPacketDsaTagPart_toAnalyzer.devPort.hwDevNum  = prvTgfDevNum;
        prvTgfPacketDsaTagPart_toAnalyzer.devPort.portNum = prvTgfPortsArray[dsaTagTrgDevPortIndex[PRV_TGF_SEND_PORT_IDX_CNS]];
        prvTgfPacketDsaTagPart_toAnalyzer.devPort.ePort = DUMMY_TARGET_EPORT_NUM_CNS - 1;/*dummy eport*/

        if(prvTgfPacketDsaTagPart.dsaType == TGF_DSA_4_WORD_TYPE_E)
        {
            if(testingMirrorMode == PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E)
            {
                /* the analyzer port is not in the next fields , as those used the
                   src/trg physical ports of the original packet that triggered mirroring (rx/tx mirror)

                   in this test we not care about the original physical ports of the mirroring trigger
                   but we need to check that the target analyzer is taken from the
                   analyzerHwTrgDev,analyzerTrgPort,analyzerTrgEport or analyzerEvidx
                */
                prvTgfPacketDsaTagPart_toAnalyzer.devPort.hwDevNum = DUMMY_REMOTE_DEV_NUM_CNS;/*dummy device*/
                prvTgfPacketDsaTagPart_toAnalyzer.devPort.portNum = CPSS_NULL_PORT_NUM_CNS;/*dummy port*/
                prvTgfPacketDsaTagPart_toAnalyzer.devPort.ePort = DUMMY_TARGET_EPORT_NUM_CNS;/*dummy eport*/

                /* set the target info */
                prvTgfPacketDsaTagPart_toAnalyzer.analyzerTrgType = CPSS_INTERFACE_PORT_E;
                prvTgfPacketDsaTagPart_toAnalyzer.extDestInfo.devPort.analyzerIsTrgPortValid = GT_FALSE;/* the physical not valid in legacy mode of 'Source Based Forwarding with Overriding of Src/Trg' */
                prvTgfPacketDsaTagPart_toAnalyzer.extDestInfo.devPort.analyzerHwTrgDev = prvTgfDevNum;
                prvTgfPacketDsaTagPart_toAnalyzer.extDestInfo.devPort.analyzerTrgPort = CPSS_NULL_PORT_NUM_CNS;
                prvTgfPacketDsaTagPart_toAnalyzer.extDestInfo.devPort.analyzerTrgEport = TARGET_EPORT_NUM_CNS;

                analyzerTrgEPort = prvTgfPacketDsaTagPart_toAnalyzer.extDestInfo.devPort.analyzerTrgEport;
            }
            else /*testingMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E*/
            {
                prvTgfPacketDsaTagPart_toAnalyzer.devPort.ePort = TARGET_EPORT_NUM_CNS - 1;
                analyzerTrgEPort = prvTgfPacketDsaTagPart_toAnalyzer.devPort.ePort;

                prvTgfPacketDsaTagPart_toAnalyzer.analyzerTrgType = CPSS_INTERFACE_PORT_E;
                prvTgfPacketDsaTagPart_toAnalyzer.extDestInfo.devPort.analyzerIsTrgPortValid = GT_FALSE;/* the physical not valid in legacy mode of 'Source Based Forwarding with Overriding of Src/Trg' */
                prvTgfPacketDsaTagPart_toAnalyzer.extDestInfo.devPort.analyzerHwTrgDev = DUMMY_REMOTE_DEV_NUM_CNS;
                prvTgfPacketDsaTagPart_toAnalyzer.extDestInfo.devPort.analyzerTrgPort = CPSS_NULL_PORT_NUM_CNS;
                prvTgfPacketDsaTagPart_toAnalyzer.extDestInfo.devPort.analyzerTrgEport = DUMMY_TARGET_EPORT_NUM_CNS;
            }

            /* set the e2Phy to set physical port for the eport TARGET_EPORT_NUM_CNS */
            physicalInfo.type = CPSS_INTERFACE_PORT_E;
            physicalInfo.devPort.hwDevNum = prvTgfDevNum;
            physicalInfo.devPort.portNum = prvTgfPortsArray[dsaTagTrgDevPortIndex[PRV_TGF_SEND_PORT_IDX_CNS]];

            rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                        analyzerTrgEPort,
                        &physicalInfo);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: %d, %d",
                                         prvTgfDevNum, analyzerTrgEPort);

            prvTgfPacketDsaTagPart_toAnalyzer.tag0TpidIndex = 5;
        }

    }

    /* AUTODOC: send BC packet from port 1 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:02, VID=5 */
    prvTgfMirrorTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

    /* AUTODOC: verify traffic: */
    /* AUTODOC:   port 2: 2 packets - original flood, copy to analyzer */
    /* AUTODOC:   ports 0,3: 1 packet - original flood */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        /* Verify that port 18 received 2 packets */
        if (PRV_TGF_RCV1_PORT_IDX_CNS == portIter)
        {
            if(testingDsaTag == GT_TRUE)
            {
                /* 1 packet per send : copy to analyzer (we not send flood) */
                numTxPackets = prvTgfBurstCount;
            }
            else
            {
                /* 2 packets per send : original flood + copy to analyzer */
                numTxPackets = prvTgfBurstCount * 2;
            }

            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(numTxPackets, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
        else
        {
            if(testingDsaTag == GT_TRUE)
            {
                /* 0 packets per send : we not send flood */
                numTxPackets = 0;
            }
            else
            {
                /* 1 packets per send : original flood  */
                numTxPackets = prvTgfBurstCount;
            }
            UTF_VERIFY_EQUAL0_STRING_MAC(numTxPackets, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    if(testingDsaTag == GT_TRUE)
    {
        prvTgfPacketDsaTagPart_toAnalyzer.devPort.hwDevNum  = prvTgfDevNum;
        prvTgfPacketDsaTagPart_toAnalyzer.devPort.portNum = prvTgfPortsArray[dsaTagTrgDevPortIndex[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS]];
        prvTgfPacketDsaTagPart_toAnalyzer.devPort.ePort = DUMMY_TARGET_EPORT_NUM_CNS - 1;/*dummy eport*/

        if(prvTgfPacketDsaTagPart.dsaType == TGF_DSA_4_WORD_TYPE_E)
        {
            if(testingMirrorMode == PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E)
            {
                /* the analyzer port is not in the next fields , as those used the
                   src/trg physical ports of the original packet that triggered mirroring (rx/tx mirror)

                   in this test we not care about the original physical ports of the mirroring trigger
                   but we need to check that the target analyzer is taken from the
                   analyzerHwTrgDev,analyzerTrgPort,analyzerTrgEport or analyzerEvidx
                */
                prvTgfPacketDsaTagPart_toAnalyzer.devPort.hwDevNum = DUMMY_REMOTE_DEV_NUM_CNS;/*dummy device*/
                prvTgfPacketDsaTagPart_toAnalyzer.devPort.portNum = CPSS_NULL_PORT_NUM_CNS;/*dummy port*/

                analyzerTrgEPort = prvTgfPacketDsaTagPart_toAnalyzer.extDestInfo.devPort.analyzerTrgEport;
            }
            else /*PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E*/
            {
                prvTgfPacketDsaTagPart_toAnalyzer.devPort.ePort = TARGET_EPORT_NUM_CNS - 1;
                analyzerTrgEPort = prvTgfPacketDsaTagPart_toAnalyzer.devPort.ePort;
            }

            /* set the target info */

            /* set the e2Phy to set physical port for the eport TARGET_EPORT_NUM_CNS */
            physicalInfo.type = CPSS_INTERFACE_PORT_E;
            physicalInfo.devPort.hwDevNum = prvTgfDevNum;
            physicalInfo.devPort.portNum = prvTgfPortsArray[dsaTagTrgDevPortIndex[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS]];
            rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                        analyzerTrgEPort,
                        &physicalInfo);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: %d, %d",
                                         prvTgfDevNum, analyzerTrgEPort);
        }
    }

    /* AUTODOC: send BC packet from port 0 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:02, VID=5 */
    prvTgfMirrorTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS], &prvTgfPacketInfo);

    /* AUTODOC: verify traffic: */
    /* AUTODOC:   port 3: 2 packets - original flood, copy to analyzer */
    /* AUTODOC:   ports 1,2: 1 packet - original flood */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        /* Verify that port 23 received 2 packets */
        if (PRV_TGF_RCV_PORT_IDX_CNS == portIter)
        {
            if(testingDsaTag == GT_TRUE)
            {
                /* 1 packet per send : copy to analyzer (we not send flood) */
                numTxPackets = prvTgfBurstCount;
            }
            else
            {
                /* 2 packets per send : original flood + copy to analyzer */
                numTxPackets = prvTgfBurstCount * 2;
            }

            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(numTxPackets, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
        else
        {
            if(testingDsaTag == GT_TRUE)
            {
                /* 0 packets per send : we not send flood */
                numTxPackets = 0;
            }
            else
            {
                /* 1 packets per send : original flood  */
                numTxPackets = prvTgfBurstCount;
            }

            UTF_VERIFY_EQUAL0_STRING_MAC(numTxPackets, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
    }

    if(testingDsaTag == GT_TRUE)
    {
        /* there is no meaning to check 'disabled mirroring' for DSA tagged packets */
        return;
    }

    /* AUTODOC: disable Rx mirroring on port=1, index=0 */
    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChMirrorRxPortSet: %d %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);

    /* AUTODOC: send BC packet from port 1 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:02, VID=5 */
    prvTgfMirrorTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

    /* AUTODOC: verify traffic: */
    /* AUTODOC:   ports 0,2,3: 1 packet - original flood */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        /* Verify that all ports except of send port received only 1 packet.*/
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: disable Rx mirroring on port=0, index=0 */
    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS], GT_FALSE, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChMirrorRxPortSet: %d %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS], GT_FALSE);

    /* AUTODOC: send BC packet from port 0 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:02, VID=5 */
    prvTgfMirrorTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS], &prvTgfPacketInfo);

    /* AUTODOC: verify traffic: */
    /* AUTODOC:   ports 1,2,3: 1 packet - original flood */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        /* Verify that all ports except of send port received only 1 packet.*/
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}

/**
* @internal prvTgfIngressMirrorSourceBasedConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfIngressMirrorSourceBasedConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* restore also cases of phy-port range mapping ! */
    prvTgfBrgE2PhyWaForLargePortsInNoneEdsa(GT_FALSE);

    /* AUTODOC: restore default forwarding mode for mirroring to analyzer */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorToAnalyzerForwardingModeSet: %d",
                                 PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E);

    /* AUTODOC: disable Rx mirroring on port=1, index=0 */
    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[1], GT_FALSE, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet: %d %d",
                                 prvTgfPortsArray[1], GT_FALSE);

    /* AUTODOC: disable Rx mirroring on port=0, index=0 */
    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[0], GT_FALSE, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet: %d %d",
                                 prvTgfPortsArray[0], GT_FALSE);

    /* invalidate VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_VLANID_CNS);

    /* restore e2Phy configurations */
    restoreE2Phy();

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);


    /* AUTODOC: restore prvTgfPortsArray if it is necessary */
    if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum) && GT_TRUE == testingDsaTag)
    {
        cpssOsMemCpy(prvTgfPortsArray, prvTgfPortsArraySave, sizeof(prvTgfPortsArraySave));
    }

    if(testingDsaTag == GT_TRUE &&
       prvTgfPacketDsaTagPart.dsaType == TGF_DSA_2_WORD_TYPE_E)
    {
        /* the ownDevNum must be limited to 0x1f that can be passed on such DSA */
        /* this is needed for runs with random hwDevNum                         */
        prvTgfLimitedHwDevNumSet(GT_NA);
    }

    /* restore DSA tag testing mode */
    prvTgfMirrorConfigParamsSet(GT_FALSE,TGF_DSA_2_WORD_TYPE_E,PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E);

}

/**
* @internal prvTgfIngressMirrorHighestIndexSelectionConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set source-based forwarding mode.
*         - Set analyzer interface for index 3.
*         Device number 0, port number 18, port interface.
*         - Set analyzer interface for index 1.
*         Device number 0, port number 23, port interface.
*         - Enable Rx mirroring on port 8 and set analyzer
*         interface index for the port to 1.
*         - Set FDB entry with MAC address 00:00:00:00:00:01
*         and mirrorToRxAnalyzerPortEn = GT_TRUE.
*         -    Set analyzer interface index to 3 and enable Rx mirroring.
*/
GT_VOID prvTgfIngressMirrorHighestIndexSelectionConfigurationSet
(
    GT_VOID
)
{
    PRV_TGF_BRG_MAC_ENTRY_STC prvTgfMacEntry;       /* FDB MAC entry */
    GT_STATUS               rc;

    /* AUTODOC: SETUP CONFIGURATION: */
    /* AUTODOC: create VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                 PRV_TGF_VLANID_CNS);

    /* clear entry */
    cpssOsMemSet(&prvTgfMacEntry, 0, sizeof(prvTgfMacEntry));
    /* we need to make the entry static otherwise the AppDemo will get message
       about station movement and will replace the entry.
       in multi-port group FDB this will cause the removal of the entry from 3
       port groups and will be left only to single port group.
       */
    prvTgfMacEntry.isStatic = GT_TRUE;
    /* Set FDB entry with MAC address 00:00:00:00:00:02
       and mirrorToRxAnalyzerPortEn = GT_TRUE. */
    prvTgfMacEntry.mirrorToRxAnalyzerPortEn     = GT_TRUE;
    prvTgfMacEntry.key.key.macVlan.vlanId = PRV_TGF_VLANID_CNS;
    cpssOsMemCpy(prvTgfMacEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:02, VLAN 5, mirrorToRxAnalyzer True */
    rc = prvTgfBrgFdbMacEntrySet(&prvTgfMacEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet");

    /* AUTODOC: set Source-Based forwarding mode for mirroring to analyzer */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorToAnalyzerForwardingModeSet: %d",
                                 PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E);

    /* AUTODOC: set analyzer interface index=3: */
    /* AUTODOC:   analyzer devNum=0, port=2 */
    prvMirrorAnalyzerInterfaceSet(PRV_TGF_MIRROR_GLOB_ANALYZER_IDX_CNS, prvTgfDevsArray[2], prvTgfPortsArray[2]);

    /* AUTODOC: set analyzer interface index=1: */
    /* AUTODOC:   analyzer devNum=0, port=3 */
    prvMirrorAnalyzerInterfaceSet(1, prvTgfDevsArray[3], prvTgfPortsArray[3]);

    /* AUTODOC: enable Rx mirroring on port=1, index=1 */
    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[1], GT_TRUE, 1);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet: %d %d %d",
                                 prvTgfPortsArray[1], GT_TRUE, 1);

    /* save default analyzer interface */
    rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexGet(prvTgfDevNum, &prvTgfEnableMirror, &prvTgfIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxGlobalAnalyzerInterfaceIndexGet: %d", prvTgfDevNum);

    /* AUTODOC: enable global Rx mirroring index=3 for HopByHop mode */
    rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet(GT_TRUE, PRV_TGF_MIRROR_GLOB_ANALYZER_IDX_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet: %d %d", GT_TRUE, 3);

    /* Configure FDB SA Analyzer index for eArch device */
    if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
    {
        /* save FDB SA analyzer state */
        rc = prvTgfBrgFdbSaLookupAnalyzerIndexGet(&saveFdbSaAnalyzerEnable,
                                                  &saveFdbSaAnalyzerIndex);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbSaLookupAnalyzerIndexGet: %d", prvTgfDevNum);

        /* AUTODOC: Set FDB SA analyzer state to be 3 */
        rc = prvTgfBrgFdbSaLookupAnalyzerIndexSet(GT_TRUE, PRV_TGF_MIRROR_GLOB_ANALYZER_IDX_CNS);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbSaLookupAnalyzerIndexSet: %d", prvTgfDevNum);
    }
}

/**
* @internal prvTgfIngressMirrorHighestIndexSelectionTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 0, 23.
*         2 Packets are captured on port 18.
*         Send to device's port 0 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:01,
*         Success Criteria:
*         1 packet is captured on ports 0, 18.
*         2 Packets are captured on port 23.
*         - Set analyzer interface index to 0 and enable Rx mirroring.
*         Send to device's port 0 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 0, 18.
*         2 Packets are captured on port 23.
*/
GT_VOID prvTgfIngressMirrorHighestIndexSelectionTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    TGF_MAC_ADDR    saMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x02};
    GT_U32                          expectedCount;/* expected count*/
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC   mirror0Interface;/* mirror interface in index 0 */


    /* AUTODOC: GENERATE TRAFFIC: */

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: send BC packet from port 1 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:02, VID=5 */
    prvTgfMirrorTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

    /* AUTODOC: verify traffic: */
    /* AUTODOC:   port 2: 2 packets - FDB match, original flood */
    /* AUTODOC:   ports 0,3: 1 packet - original flood */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {

            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        if(prvTgfNumOfPortGroups && usePortGroupsBmp == GT_FALSE && prvTgfDevPortForceNum)
        {
            /* the test added ring ports , and since the packet did flooding in
            the port groups , each port group will have FDB match on SA lookup
            that will result mirror to analyzer */
            expectedCount = prvTgfBurstCount * (prvTgfNumOfPortGroups - 1);
        }
        else
        {
            expectedCount = 0;
        }

        /* check Tx counters */
        /* Verify that port 18 received 2 packets - FDB match - max(1, 3). */
        if (PRV_TGF_RCV1_PORT_IDX_CNS == portIter)
        {
            expectedCount += (prvTgfBurstCount * 2);
            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(expectedCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
        else
        {
            expectedCount = prvTgfBurstCount;
            UTF_VERIFY_EQUAL0_STRING_MAC(expectedCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: Change global Rx mirroring index=0 for HopByHop mode */
    rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet(GT_TRUE, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet: %d %d", GT_TRUE, 0);

    /* Configure FDB SA Analyzer index for eArch device */
    if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
    {
        /* AUTODOC: Change FDB SA analyzer state to be 0 */
        rc = prvTgfBrgFdbSaLookupAnalyzerIndexSet(GT_TRUE, 0);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbSaLookupAnalyzerIndexSet: %d", prvTgfDevNum);
    }

    /* get analyzer interface in index 0 */
    rc = prvTgfMirrorAnalyzerInterfaceGet(prvTgfDevNum, 0, &mirror0Interface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceGet: %d", prvTgfDevNum);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    cpssOsMemCpy(prvTgfPacketL2Part.saMac, saMac, sizeof(TGF_MAC_ADDR));

    /* AUTODOC: send BC packet from port 1 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:02, VID=5 */
    prvTgfMirrorTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

    /* AUTODOC: verify traffic: */
    /* AUTODOC:   port 3: 2 packets - FDB match, original flood */
    /* AUTODOC:   ports 0,2: 1 packet - original flood */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        expectedCount = 0;

        if(prvTgfNumOfPortGroups && usePortGroupsBmp == GT_FALSE && prvTgfDevPortForceNum &&
            (mirror0Interface.interface.type == CPSS_INTERFACE_PORT_E &&
             prvTgfDevsArray[portIter] == mirror0Interface.interface.devPort.hwDevNum &&
             prvTgfPortsArray[portIter] == mirror0Interface.interface.devPort.portNum))
        {
            /* this port is the analyzer port of index 0 */
            /* this port will get extra copies due to mirror from the FDB on SA match */

            /* the test added ring ports , and since the packet did flooding in
            the port groups , each port group will have FDB match on SA lookup
            that will result mirror to analyzer */
            expectedCount += prvTgfBurstCount * (prvTgfNumOfPortGroups - 1);
        }

        /* check Tx counters */
        /* Verify that port 23 received 2 packets  - (FDB match - max(0, 1).)*/
        if (PRV_TGF_RCV_PORT_IDX_CNS == portIter)
        {
            expectedCount += (prvTgfBurstCount * 2);
            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(expectedCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
        else
        {
            expectedCount += prvTgfBurstCount;
            UTF_VERIFY_EQUAL0_STRING_MAC(expectedCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}

/**
* @internal prvTgfIngressMirrorHighestIndexSelectionConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfIngressMirrorHighestIndexSelectionConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: disable Rx mirroring on port=1, index=0 */
    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[1], GT_FALSE, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet: %d %d",
                                 prvTgfPortsArray[1], GT_FALSE);

    /* AUTODOC: restore global Rx mirroring configuration */
    rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet(prvTgfEnableMirror, prvTgfIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet: %d", prvTgfEnableMirror);

    /* Restore FDB SA Analyzer index for eArch device */
    if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
    {
        /* AUTODOC: Restore FDB SA analyzer state */
        rc = prvTgfBrgFdbSaLookupAnalyzerIndexSet(saveFdbSaAnalyzerEnable, saveFdbSaAnalyzerIndex);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbSaLookupAnalyzerIndexSet: %d", prvTgfDevNum);
    }

    /* AUTODOC: restore default forwarding mode for mirroring to analyzer */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorToAnalyzerForwardingModeSet: %d",
                                 PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E);

    /* invalidate VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_VLANID_CNS);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", GT_TRUE);
}

/**
* @internal prvTgfEgressMirrorHopByHopConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set analyzer interface for index 2.
*         Device number 0, port number 18, port interface.
*         -    Set analyzer interface for index 0.
*         Device number 0, port number 23, port interface.
*         - Set analyzer interface index to 2 and enable Tx mirroring.
*         - Enable Tx mirroring on port 0.
*/
GT_VOID prvTgfEgressMirrorHopByHopConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS               rc = GT_OK;


    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                 PRV_TGF_VLANID_CNS);

    /* egress mirroring */
    prvTgfMirrorAnalyzerInterfaceTypeSet(GT_TRUE);

    /* AUTODOC: set analyzer interface index=2: */
    /* AUTODOC:   analyzer devNum=0, port=2 */
    prvMirrorAnalyzerInterfaceSet(2, prvTgfDevsArray[2], prvTgfPortsArray[2]);

    /* AUTODOC: set analyzer interface index=0: */
    /* AUTODOC:   analyzer devNum=0, port=3 */
    prvMirrorAnalyzerInterfaceSet(0, prvTgfDevsArray[3], prvTgfPortsArray[3]);

    /* save default analyzer interface */
    rc = prvTgfMirrorTxGlobalAnalyzerInterfaceIndexGet(prvTgfDevNum, &prvTgfEnableMirror, &prvTgfIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxGlobalAnalyzerInterfaceIndexGet: %d", prvTgfDevNum);

    /* AUTODOC: enable global Tx mirroring index=2 for HopByHop mode */
    rc = prvTgfMirrorTxGlobalAnalyzerInterfaceIndexSet(GT_TRUE, 2);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxGlobalAnalyzerInterfaceIndexSet: %d %d", GT_TRUE, 2);

    if(testingDsaTag == GT_TRUE)
    {
        /* in 'hop by hop forwarding mode' the DSA tag analyzer info is ignored*/
        prvTgfPacketDsaTagPart_toAnalyzer.rxSniffer = GT_FALSE;
        prvTgfPacketDsaTagPart_toAnalyzer.isTagged = GT_TRUE;
        prvTgfPacketDsaTagPart_toAnalyzer.devPort.hwDevNum = DUMMY_REMOTE_DEV_NUM_CNS;/*dummy device*/
        prvTgfPacketDsaTagPart_toAnalyzer.devPort.portNum = CPSS_NULL_PORT_NUM_CNS;/*dummy port*/
        prvTgfPacketDsaTagPart_toAnalyzer.devPort.ePort = DUMMY_TARGET_EPORT_NUM_CNS - 1;/*dummy eport*/

        if(prvTgfPacketDsaTagPart.dsaType == TGF_DSA_4_WORD_TYPE_E)
        {
            prvTgfPacketDsaTagPart_toAnalyzer.analyzerTrgType = CPSS_INTERFACE_PORT_E;
            prvTgfPacketDsaTagPart_toAnalyzer.extDestInfo.devPort.analyzerIsTrgPortValid = GT_FALSE;
            prvTgfPacketDsaTagPart_toAnalyzer.extDestInfo.devPort.analyzerHwTrgDev = DUMMY_REMOTE_DEV_NUM_CNS - 2;/*dummy device*/
            prvTgfPacketDsaTagPart_toAnalyzer.extDestInfo.devPort.analyzerTrgPort = CPSS_NULL_PORT_NUM_CNS - 2;/*dummy port*/
            prvTgfPacketDsaTagPart_toAnalyzer.extDestInfo.devPort.analyzerTrgEport = DUMMY_TARGET_EPORT_NUM_CNS;

            prvTgfPacketDsaTagPart_toAnalyzer.tag0TpidIndex = 5;
        }
    }
    else
    {
            /* AUTODOC: enable Tx mirroring on port=0, index=0 */
        rc = prvTgfMirrorTxPortSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS], GT_TRUE, 0);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortSet: %d %d",
                                     prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS], GT_TRUE);
    }
}

/**
* @internal prvTgfEgressMirrorHopByHopTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 0, 23.
*         2 Packets are captured on port 18.
*         Disable Tx mirroring.
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         Packet is captured on ports 0, 18, 23.
*         Set analyzer interface index to 0 and enable Tx mirroring.
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 0, 18.
*         2 Packets are captured on port 23.
*         Disable Tx mirroring on port 0.
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         Packet is captured on ports 0, 18, 23.
*/
GT_VOID prvTgfEgressMirrorHopByHopTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_U32      numTxPackets;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: send BC packet from port 1 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:02, VID=5 */
    prvTgfMirrorTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

    /* AUTODOC: verify traffic: */
    /* AUTODOC:   port 2: 2 packets - original flood, copy to analyzer */
    /* AUTODOC:   ports 0,3: 1 packet - original flood */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        /* Verify that port 18 received 2 packets */
        if (PRV_TGF_RCV1_PORT_IDX_CNS == portIter)
        {
            if(testingDsaTag == GT_TRUE)
            {
                /* 1 packet per send : copy to analyzer (we not send flood) */
                numTxPackets = prvTgfBurstCount;
            }
            else
            {
                /* 2 packets per send : original flood + copy to analyzer */
                numTxPackets = prvTgfBurstCount * 2;
            }

            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(numTxPackets, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
        else
        {
            if(testingDsaTag == GT_TRUE)
            {
                /* 0 packets per send : we not send flood */
                numTxPackets = 0;
            }
            else
            {
                /* 1 packets per send : original flood  */
                numTxPackets = prvTgfBurstCount;
            }

            UTF_VERIFY_EQUAL0_STRING_MAC(numTxPackets, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }

    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    if(testingDsaTag == GT_TRUE)
    {
        /* there is no meaning to check 'disabled mirroring' for DSA tagged packets */
        goto newGlobalAnalyzer_lbl;
    }

    /* AUTODOC: disable global Tx mirroring index=2 for HopByHop mode */
    rc = prvTgfMirrorTxGlobalAnalyzerInterfaceIndexSet(GT_FALSE, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxGlobalAnalyzerInterfaceIndexSet: %d", GT_FALSE);

    /* AUTODOC: send BC packet from port 1 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:02, VID=5 */
    prvTgfMirrorTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

    /* AUTODOC: verify traffic: */
    /* AUTODOC:   ports 0,2,3: 1 packet - original flood */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        /* Verify that all ports except of send port received only 1 packet.*/
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

newGlobalAnalyzer_lbl:

        /* AUTODOC: enable global Tx mirroring index=0 for HopByHop mode */
    rc = prvTgfMirrorTxGlobalAnalyzerInterfaceIndexSet(GT_TRUE, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxGlobalAnalyzerInterfaceIndexSet: %d %d", GT_TRUE, 0);

    /* AUTODOC: send BC packet from port 1 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:02, VID=5 */
    prvTgfMirrorTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

    /* AUTODOC: verify traffic: */
    /* AUTODOC:   port 3: 2 packets - original flood, copy to analyzer */
    /* AUTODOC:   ports 0,3: 1 packet - original flood */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        /* Verify that port 23 received 2 packets */
        if (PRV_TGF_RCV_PORT_IDX_CNS == portIter)
        {
            if(testingDsaTag == GT_TRUE)
            {
                /* 1 packet per send : copy to analyzer (we not send flood) */
                numTxPackets = prvTgfBurstCount;
            }
            else
            {
                /* 2 packets per send : original flood + copy to analyzer */
                numTxPackets = prvTgfBurstCount * 2;
            }

            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(numTxPackets, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
        else
        {
            if(testingDsaTag == GT_TRUE)
            {
                /* 0 packets per send : we not send flood */
                numTxPackets = 0;
            }
            else
            {
                /* 1 packets per send : original flood  */
                numTxPackets = prvTgfBurstCount;
            }

            UTF_VERIFY_EQUAL0_STRING_MAC(numTxPackets, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
    }
    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    if(testingDsaTag == GT_TRUE)
    {
        /* there is no meaning to check 'disabled mirroring' for DSA tagged packets */
        return;
    }

    /* AUTODOC: disable Tx mirroring on port=0, index=0 */
    rc = prvTgfMirrorTxPortSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS], GT_FALSE, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortSet: %d %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS], GT_FALSE);

    /* AUTODOC: send BC packet from port 1 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:02, VID=5 */
    prvTgfMirrorTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

    /* AUTODOC: verify traffic: */
    /* AUTODOC:   ports 0,2,3: 1 packet - original flood */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        /* Verify that all ports except of send port received only 1 packet.*/
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}

/**
* @internal prvTgfEgressMirrorHopByHopConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfEgressMirrorHopByHopConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* restore to ingress mirroring */
    prvTgfMirrorAnalyzerInterfaceTypeSet(GT_FALSE);

    /* AUTODOC: disable global Tx mirroring index=0 for HopByHop mode */
    rc = prvTgfMirrorTxGlobalAnalyzerInterfaceIndexSet(prvTgfEnableMirror, prvTgfIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxGlobalAnalyzerInterfaceIndexSet: %d", prvTgfEnableMirror);

    /* AUTODOC: disable Tx mirroring on port=0, index=0 */
    rc = prvTgfMirrorTxPortSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS], GT_FALSE, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortSet: %d %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_ANOTHER_CNS], GT_FALSE);

    /* invalidate VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_VLANID_CNS);

    /* restore e2Phy configurations */
    restoreE2Phy();

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* restore DSA tag testing mode */
    prvTgfMirrorConfigParamsSet(GT_FALSE,TGF_DSA_2_WORD_TYPE_E,PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E);
}

/**
* @internal prvTgfEgressMirrorSourceBasedConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set source-based forwarding mode.
*         -    Set analyzer interface for index 5.
*         Device number 0, port number 17, port interface.
*         - Set analyzer interface for index 0.
*         Device number 0, port number 0, port interface.
*         -    Enable Tx mirroring on port 23 and
*         set analyzer interface index for the port to 5.
*         -    Enable Tx mirroring on port 18 and
*         set analyzer interface index for the port to 0.
*/
GT_VOID prvTgfEgressMirrorSourceBasedConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS               rc = GT_OK;

    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: test uses 5 ports. Save prvTgfPortsNum and prvTgfPortsArrays */
    prvTgfPortsNumSave = prvTgfPortsNum;
    cpssOsMemCpy(prvTgfPortsArraySave, prvTgfPortsArray,
                 sizeof(prvTgfPortsArraySave));

    if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum) && (GT_TRUE == testingDsaTag))
    {
        /* need to ensure that rx ports are able to be cascade */
        rc = prvTgfDefPortsArrange(GT_FALSE,
                                   PRV_TGF_TX_MIRR_PORT1_IDX_CNS,
                                   PRV_TGF_TX_MIRR_PORT2_IDX_CNS,
                                   PRV_TGF_TX_MIRR_SEND_IDX_CNS,
                                   -1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortsArray rearrangement can't be done");
    }

    if(MUST_USE_ORIG_PORTS_MAC(prvTgfDevNum))
    {
        /* do not modify the 12 ports in prvTgfPortsArray[] !!! */
    }
    else
    if(prvTgfNumOfPortGroups && usePortGroupsBmp == GT_FALSE && prvTgfDevPortForceNum)
    {
        prvTgfPortsArray[4] = 40;/* use port 40 and not 17 because 17 used as ring port */
    }
    else
    {
        prvTgfPortsArray[4] = (prvTgfDevicePortMode == PRV_TGF_DEVICE_PORT_MODE_XLG_E)? 16 : 17;
    }
    prvTgfPortsNum = 5;

    if(testingDsaTag == GT_TRUE &&
       prvTgfPacketDsaTagPart.dsaType == TGF_DSA_2_WORD_TYPE_E)
    {
        /* the ownDevNum must be limited to 0x1f that can be passed on such DSA */
        /* this is needed for runs with random hwDevNum                         */
        prvTgfLimitedHwDevNumSet(0x1f);
    }

    /* New ports added: Link up and reset counters */
    prvTgfEthCountersReset(prvTgfDevNum);

    {
        GT_U32 i;
        for (i = 0; (i < 4); i++)
        {
            if (prvTgfPortsArray[i] == prvTgfPortsArray[4])
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(
                    GT_OK, GT_FAIL,
                    "Test not compatible wit given list of ports");
                return;
            }
        }
    }

    if(testingDsaTag == GT_TRUE &&
       prvTgfPacketDsaTagPart.dsaType == TGF_DSA_2_WORD_TYPE_E)
    {
        prvTgfBrgE2PhyWaForLargePortsInNoneEdsa(GT_TRUE);
    }

    /* AUTODOC: create VLAN 5 with untagged ports [0,1,2,3,4] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                 PRV_TGF_VLANID_CNS);

    rc = (testingMirrorMode ==  PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E ||
         testingMirrorMode ==  PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E) ? GT_OK : GT_FAIL;
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "bad testingMirrorMode: %d", testingMirrorMode);

    /*  Set source-based forwarding mode. */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(testingMirrorMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorToAnalyzerForwardingModeSet: %d",
                                 testingMirrorMode);

    /* egress mirroring */

    if(testingDsaTag == GT_TRUE)
    {
        /* in 'source-based forwarding mode' the DSA tag should give all the relevant info */
        prvTgfPacketDsaTagPart_toAnalyzer.rxSniffer = GT_FALSE;
        prvTgfPacketDsaTagPart_toAnalyzer.isTagged = GT_TRUE;
        /* state that rx mirroring came and should egress [index[2]]*/
        dsaTagTrgDevPortIndex[PRV_TGF_TX_MIRR_PORT1_IDX_CNS] = 0;/*packet egress from index 2 is mirrored to analyzer port in index 0 */
        /* state that rx mirroring came and should egress [index[3]]*/
        dsaTagTrgDevPortIndex[PRV_TGF_TX_MIRR_PORT2_IDX_CNS] = 4;/*packet egress from index 3 is mirrored to analyzer port in index 4 */
    }
    else
    {
        prvTgfMirrorAnalyzerInterfaceTypeSet(GT_TRUE);

        /* AUTODOC: set analyzer interface index=5: */
        /* AUTODOC:   analyzer devNum=0, port=4 */
        prvMirrorAnalyzerInterfaceSet(5, prvTgfDevsArray[4], prvTgfPortsArray[4]);

        /* AUTODOC: set analyzer interface index=0: */
        /* AUTODOC:   analyzer devNum=0, port=0 */
        prvMirrorAnalyzerInterfaceSet(0, prvTgfDevsArray[0], prvTgfPortsArray[0]);

        /* AUTODOC: enable Tx mirroring on port=3, index=5 */
        rc = prvTgfMirrorTxPortSet(prvTgfPortsArray[PRV_TGF_TX_MIRR_PORT2_IDX_CNS], GT_TRUE, 5);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortSet: %d %d %d",
                                     prvTgfPortsArray[PRV_TGF_TX_MIRR_PORT2_IDX_CNS], GT_TRUE, 5);

        /* AUTODOC: enable Tx mirroring on port=2, index=0 */
        rc = prvTgfMirrorTxPortSet(prvTgfPortsArray[PRV_TGF_TX_MIRR_PORT1_IDX_CNS], GT_TRUE, 0);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet: %d %d %d",
                                     prvTgfPortsArray[PRV_TGF_TX_MIRR_PORT1_IDX_CNS], GT_TRUE, 0);
    }

    /* AUTODOC: remove analyzer ports [0,4] from VLAN 5 */
    rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_VLANID_CNS, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: %d%d",
                                 PRV_TGF_VLANID_CNS, prvTgfPortsArray[0]);


    rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_VLANID_CNS, prvTgfPortsArray[4]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: %d%d",
                                 PRV_TGF_VLANID_CNS, prvTgfPortsArray[4]);

}

/**
* @internal prvTgfEgressMirrorSourceBasedTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Learn MACs UC1 and UC2 on Tx mirrored ports 18 and 23
*         Send to device's port 8 two packets:
*         macDa = UC1,macDa = UC2,
*         Success Criteria:
*         1 packet is captured o n ports Tx mirrored 23.
*         1 Packets are captured on port Tx mirrored 18.
*         1 Packets are captured on analyzer port 0.
*         1 Packets are captured on analyzer port 17.
*         Disable Tx mirroring on port 18.
*         Send to device's port 8 two packets:
*         macDa = UC1,macDa = UC2,
*         Success Criteria:
*         1 packet is captured o n ports Tx mirrored 23.
*         1 Packets are captured on port Tx mirrored 18.
*         0 Packets are captured on analyzer port 0.
*         1 Packets are captured on analyzer port 17.
*         Disable Tx mirroring on port 23.
*         Send to device's port 8 two packets:
*         macDa = UC1,macDa = UC2,
*         Success Criteria:
*         1 packet is captured o n ports Tx mirrored 23.
*         1 Packets are captured on port Tx mirrored 18.
*         0 Packets are captured on analyzer port 0.
*         0 Packets are captured on analyzer port 17.
*/
GT_VOID prvTgfEgressMirrorSourceBasedTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    CPSS_INTERFACE_INFO_STC physicalInfo;/* physical port info for the analyzer in eArch */
    GT_PORT_NUM analyzerTrgEPort;/* analyzer target eport */

    /* AUTODOC: GENERATE TRAFFIC: */

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    if(testingDsaTag == GT_TRUE)
    {
        /* the DSA tag no need the learning in the device and then sending bridging */
        prvTgfPacketDsaTagPart_toAnalyzer.devPort.hwDevNum  = prvTgfDevNum;
        prvTgfPacketDsaTagPart_toAnalyzer.devPort.portNum = prvTgfPortsArray[dsaTagTrgDevPortIndex[PRV_TGF_TX_MIRR_PORT1_IDX_CNS]];
        prvTgfPacketDsaTagPart_toAnalyzer.devPort.ePort = DUMMY_TARGET_EPORT_NUM_CNS - 1;/*dummy eport*/

        if(prvTgfPacketDsaTagPart.dsaType == TGF_DSA_4_WORD_TYPE_E)
        {
            if(testingMirrorMode == PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E)
            {
                /* the analyzer port is not in the next fields , as those used the
                   src/trg physical ports of the original packet that triggered mirroring (rx/tx mirror)

                   in this test we not care about the original physical ports of the mirroring trigger
                   but we need to check that the target analyzer is taken from the
                   analyzerHwTrgDev,analyzerTrgPort,analyzerTrgEport or analyzerEvidx
                */
                prvTgfPacketDsaTagPart_toAnalyzer.devPort.hwDevNum = DUMMY_REMOTE_DEV_NUM_CNS;/*dummy device*/
                prvTgfPacketDsaTagPart_toAnalyzer.devPort.portNum = CPSS_NULL_PORT_NUM_CNS;/*dummy port*/
                prvTgfPacketDsaTagPart_toAnalyzer.devPort.ePort = DUMMY_TARGET_EPORT_NUM_CNS;/*dummy eport*/

                /* set the target info */
                prvTgfPacketDsaTagPart_toAnalyzer.analyzerTrgType = CPSS_INTERFACE_PORT_E;
                prvTgfPacketDsaTagPart_toAnalyzer.extDestInfo.devPort.analyzerIsTrgPortValid = GT_FALSE;/* the physical not valid in legacy mode of 'Source Based Forwarding with Overriding of Src/Trg' */
                prvTgfPacketDsaTagPart_toAnalyzer.extDestInfo.devPort.analyzerHwTrgDev = prvTgfDevNum;
                prvTgfPacketDsaTagPart_toAnalyzer.extDestInfo.devPort.analyzerTrgPort = CPSS_NULL_PORT_NUM_CNS;
                prvTgfPacketDsaTagPart_toAnalyzer.extDestInfo.devPort.analyzerTrgEport = TARGET_EPORT_NUM_CNS;

                analyzerTrgEPort = prvTgfPacketDsaTagPart_toAnalyzer.extDestInfo.devPort.analyzerTrgEport;
            }
            else /*testingMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E*/
            {
                prvTgfPacketDsaTagPart_toAnalyzer.devPort.ePort = TARGET_EPORT_NUM_CNS - 1;
                analyzerTrgEPort = prvTgfPacketDsaTagPart_toAnalyzer.devPort.ePort;

                prvTgfPacketDsaTagPart_toAnalyzer.analyzerTrgType = CPSS_INTERFACE_PORT_E;
                prvTgfPacketDsaTagPart_toAnalyzer.extDestInfo.devPort.analyzerIsTrgPortValid = GT_FALSE;/* the physical not valid in legacy mode of 'Source Based Forwarding with Overriding of Src/Trg' */
                prvTgfPacketDsaTagPart_toAnalyzer.extDestInfo.devPort.analyzerHwTrgDev = DUMMY_REMOTE_DEV_NUM_CNS;
                prvTgfPacketDsaTagPart_toAnalyzer.extDestInfo.devPort.analyzerTrgPort = CPSS_NULL_PORT_NUM_CNS;
                prvTgfPacketDsaTagPart_toAnalyzer.extDestInfo.devPort.analyzerTrgEport = DUMMY_TARGET_EPORT_NUM_CNS;
            }


            /* set the e2Phy to set physical port for the eport TARGET_EPORT_NUM_CNS */
            physicalInfo.type = CPSS_INTERFACE_PORT_E;
            physicalInfo.devPort.hwDevNum = prvTgfDevNum;
            physicalInfo.devPort.portNum = prvTgfPortsArray[dsaTagTrgDevPortIndex[PRV_TGF_TX_MIRR_PORT1_IDX_CNS]];
            rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                        analyzerTrgEPort,
                        &physicalInfo);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: %d, %d",
                                         prvTgfDevNum, analyzerTrgEPort);

            prvTgfPacketDsaTagPart_toAnalyzer.tag0TpidIndex = 5;
        }

        prvTgfMirrorTestPacketSend(prvTgfPortsArray[PRV_TGF_TX_MIRR_PORT1_IDX_CNS], &prvTgfPacketInfo);

        prvTgfPacketDsaTagPart_toAnalyzer.devPort.hwDevNum  = prvTgfDevNum;
        prvTgfPacketDsaTagPart_toAnalyzer.devPort.portNum = prvTgfPortsArray[dsaTagTrgDevPortIndex[PRV_TGF_TX_MIRR_PORT2_IDX_CNS]];
        prvTgfPacketDsaTagPart_toAnalyzer.devPort.ePort = DUMMY_TARGET_EPORT_NUM_CNS - 1;/*dummy eport*/

        if(prvTgfPacketDsaTagPart.dsaType == TGF_DSA_4_WORD_TYPE_E)
        {
            if(testingMirrorMode == PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E)
            {
                /* the analyzer port is not in the next fields , as those used the
                   src/trg physical ports of the original packet that triggered mirroring (rx/tx mirror)

                   in this test we not care about the original physical ports of the mirroring trigger
                   but we need to check that the target analyzer is taken from the
                   analyzerHwTrgDev,analyzerTrgPort,analyzerTrgEport or analyzerEvidx
                */
                prvTgfPacketDsaTagPart_toAnalyzer.devPort.hwDevNum = DUMMY_REMOTE_DEV_NUM_CNS;/*dummy device*/
                prvTgfPacketDsaTagPart_toAnalyzer.devPort.portNum = CPSS_NULL_PORT_NUM_CNS;/*dummy port*/

                analyzerTrgEPort = prvTgfPacketDsaTagPart_toAnalyzer.extDestInfo.devPort.analyzerTrgEport;
            }
            else /*PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E*/
            {
                prvTgfPacketDsaTagPart_toAnalyzer.devPort.ePort = TARGET_EPORT_NUM_CNS - 1;
                analyzerTrgEPort = prvTgfPacketDsaTagPart_toAnalyzer.devPort.ePort;
            }


            /* set the target info */

            /* set the e2Phy to set physical port for the eport TARGET_EPORT_NUM_CNS */
            physicalInfo.type = CPSS_INTERFACE_PORT_E;
            physicalInfo.devPort.hwDevNum = prvTgfDevNum;
            physicalInfo.devPort.portNum = prvTgfPortsArray[dsaTagTrgDevPortIndex[PRV_TGF_TX_MIRR_PORT2_IDX_CNS]];
            rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                        analyzerTrgEPort,
                        &physicalInfo);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: %d, %d",
                                         prvTgfDevNum, analyzerTrgEPort);

        }

        prvTgfMirrorTestPacketSend(prvTgfPortsArray[PRV_TGF_TX_MIRR_PORT2_IDX_CNS], &prvTgfPacketInfo);
    }
    else
    {
        /* learn MAC addresses on Tx mirrored ports */

            /* AUTODOC: send UC packet (to learn) from port 2 with: */
            /* AUTODOC:   DA=00:00:00:00:33:33, SA=00:11:22:33:44:55, VID=5 */
            prvTgfMirrorTestPacketSend(prvTgfPortsArray[PRV_TGF_TX_MIRR_PORT1_IDX_CNS], &prvTgfPacketInfoUC1Learn);

            /* AUTODOC: send UC packet (to learn) from port 3 with: */
            /* AUTODOC:   DA=00:00:00:00:44:55, SA=00:AA:BB:CC:DD:EE, VID=5 */
        prvTgfMirrorTestPacketSend(prvTgfPortsArray[PRV_TGF_TX_MIRR_PORT2_IDX_CNS], &prvTgfPacketInfoUC2Learn);

        /* wait to learn packets */
        cpssOsTimerWkAfter(1);

        /* AUTODOC: reset counters */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }

        /* AUTODOC: send UC packet from port 1 with: */
        /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:00:00:00:22:22, VID=5 */
        prvTgfMirrorTestPacketSend(prvTgfPortsArray[PRV_TGF_TX_MIRR_SEND_IDX_CNS], &prvTgfPacketInfoUC1);

        /* AUTODOC: send UC packet from port 1 with: */
        /* AUTODOC:   DA=00:AA:BB:CC:DD:EE, SA=00:00:00:00:33:33, VID=5 */
        prvTgfMirrorTestPacketSend(prvTgfPortsArray[PRV_TGF_TX_MIRR_SEND_IDX_CNS], &prvTgfPacketInfoUC2);
    }

    /* AUTODOC: verify traffic: */
    /* AUTODOC:   ports 0,4: 1 packet - as analyzer */
    /* AUTODOC:   ports 2,3: 1 packet - as VLAN members */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if(testingDsaTag == GT_TRUE)
        {
            GT_U32  rxPackets , txPackets;

            if(portIter == PRV_TGF_TX_MIRR_PORT1_IDX_CNS ||
               portIter == PRV_TGF_TX_MIRR_PORT2_IDX_CNS)
            {
                /* those ports injected the DSA tagged packets */
                rxPackets = prvTgfBurstCount;
                txPackets = rxPackets;
            }
            else if(portIter == 0 || portIter == 4)/*analyzer ports */
            {
                txPackets = prvTgfBurstCount;
                rxPackets = 0;
            }
            else/*index 1*/
            {
                rxPackets = 0;
                txPackets = 0;
            }


            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(rxPackets, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(txPackets, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");

            continue;
        }



        if (PRV_TGF_TX_MIRR_SEND_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount*2, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        /* Verify that analyzer ports 17, 0 received 1 packet.
           And other ports received 1 packet as VLAN members. */
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    if(testingDsaTag == GT_TRUE)
    {
        /* there is no meaning to check 'disabled mirroring' for DSA tagged packets */
        return;
    }

    /* AUTODOC: disable Tx mirroring on port=2, index=0 */
    rc = prvTgfMirrorTxPortSet(prvTgfPortsArray[PRV_TGF_TX_MIRR_PORT1_IDX_CNS], GT_FALSE, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortSet: %d %d",
                                 prvTgfPortsArray[PRV_TGF_TX_MIRR_PORT1_IDX_CNS], GT_FALSE);

    /* AUTODOC: send UC packet from port 1 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:00:00:00:22:22, VID=5 */
    prvTgfMirrorTestPacketSend(prvTgfPortsArray[PRV_TGF_TX_MIRR_SEND_IDX_CNS], &prvTgfPacketInfoUC1);

    /* AUTODOC: send UC packet from port 1 with: */
    /* AUTODOC:   DA=00:AA:BB:CC:DD:EE, SA=00:00:00:00:33:33, VID=5 */
    prvTgfMirrorTestPacketSend(prvTgfPortsArray[PRV_TGF_TX_MIRR_SEND_IDX_CNS], &prvTgfPacketInfoUC2);

    /* AUTODOC: verify traffic: */
    /* AUTODOC:   port 4: 1 packet - as analyzer */
    /* AUTODOC:   ports 2,3: 1 packet - as VLAN members */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_TX_MIRR_SEND_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount*2, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        /* Verify that port 0 do not receive packets because it's not
           VLAN member and mirroring to it is disabled */
        if (portIter == 0)
        {
            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
        else
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
    }

    /* AUTODOC: disable Tx mirroring on port=3, index=0 */
    rc = prvTgfMirrorTxPortSet(prvTgfPortsArray[PRV_TGF_TX_MIRR_PORT2_IDX_CNS], GT_FALSE, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortSet: %d %d",
                                 prvTgfPortsArray[PRV_TGF_TX_MIRR_PORT2_IDX_CNS], GT_FALSE);

    /* AUTODOC: send UC packet from port 1 with: */
    /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:00:00:00:22:22, VID=5 */
    prvTgfMirrorTestPacketSend(prvTgfPortsArray[PRV_TGF_TX_MIRR_SEND_IDX_CNS], &prvTgfPacketInfoUC1);

    /* AUTODOC: send UC packet from port 1 with: */
    /* AUTODOC:   DA=00:AA:BB:CC:DD:EE, SA=00:00:00:00:33:33, VID=5 */
    prvTgfMirrorTestPacketSend(prvTgfPortsArray[PRV_TGF_TX_MIRR_SEND_IDX_CNS], &prvTgfPacketInfoUC2);

    /* AUTODOC: verify traffic: */
    /* AUTODOC:   ports 2,3: 1 packet - as VLAN members */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_TX_MIRR_SEND_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount*2, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        /* Verify that port 0 and 17 do not receive packets because they are not
           VLAN members and mirroring to them is disabled */
        if ((portIter == 0) || (portIter == 4))
        {
            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
        else
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}

/**
* @internal prvTgfEgressMirrorSourceBasedConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfEgressMirrorSourceBasedConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* restore also cases of phy-port range mapping ! */
    prvTgfBrgE2PhyWaForLargePortsInNoneEdsa(GT_FALSE);

    /* restore to ingress mirroring */
    prvTgfMirrorAnalyzerInterfaceTypeSet(GT_FALSE);

    /* AUTODOC: restore default forwarding mode for mirroring to analyzer */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorToAnalyzerForwardingModeSet: %d",
                                 PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E);

    /* AUTODOC: disable Tx mirroring on port=2, index=0 */
    rc = prvTgfMirrorTxPortSet(prvTgfPortsArray[PRV_TGF_TX_MIRR_PORT1_IDX_CNS], GT_FALSE, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortSet: %d %d",
                                 prvTgfPortsArray[PRV_TGF_TX_MIRR_PORT1_IDX_CNS], GT_FALSE);

    /* AUTODOC: disable Tx mirroring on port=3, index=0 */
    rc = prvTgfMirrorTxPortSet(prvTgfPortsArray[PRV_TGF_TX_MIRR_PORT2_IDX_CNS], GT_FALSE, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet: %d %d",
                                 prvTgfPortsArray[PRV_TGF_TX_MIRR_PORT2_IDX_CNS], GT_FALSE);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* invalidate VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_VLANID_CNS);

    /* restore e2Phy configurations */
    restoreE2Phy();

    /* AUTODOC: test used 5 ports. Restore prvTgfPortsNum and prvTgfPortsArrays */
    cpssOsMemCpy(prvTgfPortsArray, prvTgfPortsArraySave, sizeof(prvTgfPortsArraySave));
    prvTgfPortsNumSave = prvTgfPortsNum;


    if(testingDsaTag == GT_TRUE &&
       prvTgfPacketDsaTagPart.dsaType == TGF_DSA_2_WORD_TYPE_E)
    {
        /* the ownDevNum must be limited to 0x1f that can be passed on such DSA */
        /* this is needed for runs with random hwDevNum                         */
        prvTgfLimitedHwDevNumSet(GT_NA);
    }

    /* restore DSA tag testing mode */
    prvTgfMirrorConfigParamsSet(GT_FALSE,TGF_DSA_2_WORD_TYPE_E,PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E);
}

/**
* @internal prvTgfIngressMirrorForwardingModeChangeConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set analyzer interface for index 3.
*         Device number 0, port number 18, port interface.
*         -    Set analyzer interface for index 0.
*         Device number 0, port number 23, port interface.
*         -    Set analyzer interface index to 3 and enable Rx mirroring.
*         - Enable Rx mirroring on port 8.
*/
GT_VOID prvTgfIngressMirrorForwardingModeChangeConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS               rc = GT_OK;


    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                 PRV_TGF_VLANID_CNS);

    /* AUTODOC: set analyzer interface index=3: */
    /* AUTODOC:   analyzer devNum=0, port=2 */
    prvMirrorAnalyzerInterfaceSet(3, prvTgfDevsArray[2], prvTgfPortsArray[2]);

    /* AUTODOC: set analyzer interface index=4: */
    /* AUTODOC:   analyzer devNum=0, port=3 */
    prvMirrorAnalyzerInterfaceSet(4, prvTgfDevsArray[3], prvTgfPortsArray[3]);

    /* save default analyzer interface */
    rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexGet(prvTgfDevNum, &prvTgfEnableMirror, &prvTgfIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxGlobalAnalyzerInterfaceIndexGet: %d", prvTgfDevNum);

    /* AUTODOC: enable global Rx mirroring index=3 for HopByHop mode */
    rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet(GT_TRUE, 3);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet: %d %d", GT_TRUE, 3);

    /* AUTODOC: enable Rx mirroring on port=1, index=0 */
    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet: %d %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);
}

/**
* @internal prvTgfIngressMirrorForwardingModeChangeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 0, 23.
*         2 Packets are captured on port 18.
*         Disable Rx mirroring on port 8.
*         Set source-based forwarding mode.
*         Enable Rx mirroring on port 8 and set
*         analyzer interface index for the port to 0.
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         Packet is captured on ports 0, 18.
*         2 Packets are captured on port 23.
*         Disable Rx mirroring on port 8.
*         Set hop-byhop forwarding mode.
*         Enable Rx mirroring on port 8.
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 0, 23.
*         2 Packets are captured on port 18.
*/
GT_VOID prvTgfIngressMirrorForwardingModeChangeTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;


    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* test will use 'PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E'
            but should be ok also for 'PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E'
            that sip6 supports */
        goto after_hop_by_hop;
    }
    /* AUTODOC: GENERATE TRAFFIC: */

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: send BC packet from port 1 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:02, VID=5 */
    prvTgfMirrorTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

    /* AUTODOC: verify traffic: */
    /* AUTODOC:   port 2: 2 packets - original flood, copy to analyzer */
    /* AUTODOC:   ports 0,3: 1 packet - original flood */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        /* Verify that port 18 received 2 packets */
        if (PRV_TGF_RCV1_PORT_IDX_CNS == portIter)
        {
            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount * 2, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
        else
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
    }

after_hop_by_hop:

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: disable Rx mirroring on port=1, index=0 */
    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet: %d %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);

    /* AUTODOC: set Source-Based forwarding mode for mirroring to analyzer */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorToAnalyzerForwardingModeSet: %d",
                                 PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E);

    /* AUTODOC: enable Rx mirroring on port=1, index=4 */
    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[1], GT_TRUE, 4);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet: %d %d %d",
                                 prvTgfPortsArray[1], GT_TRUE, 4);

    /* AUTODOC: send BC packet from port 1 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:02, VID=5 */
    prvTgfMirrorTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

    /* AUTODOC: verify traffic: */
    /* AUTODOC:   port 3: 2 packets - original flood, copy to analyzer */
    /* AUTODOC:   ports 0,2: 1 packet - original flood */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        /* Verify that port 23 received 2 packets */
        if (PRV_TGF_RCV_PORT_IDX_CNS == portIter)
        {
            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount * 2, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
        else
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: disable Rx mirroring on port=1, index=0 */
    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet: %d %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);

    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* no more needed from the test */
        return;
    }
    /* AUTODOC: set Hop-by-Hop forwarding mode for mirroring to analyzer */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorToAnalyzerForwardingModeSet: %d",
                                 PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E);

    /* AUTODOC: enable Rx mirroring on port=1, index=0 */
    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[1], GT_TRUE, 0);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet: %d %d %d",
                                 prvTgfPortsArray[1], GT_TRUE, 0);

    /* AUTODOC: send BC packet from port 1 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:02, VID=5 */
    prvTgfMirrorTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

    /* AUTODOC: verify traffic: */
    /* AUTODOC:   port 2: 2 packets - original flood, copy to analyzer */
    /* AUTODOC:   ports 0,3: 1 packet - original flood */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        /* Verify that port 18 received 2 packets */
        if (PRV_TGF_RCV1_PORT_IDX_CNS == portIter)
        {
            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount * 2, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
        else
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}

/**
* @internal prvTgfIngressMirrorForwardingModeChangeConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfIngressMirrorForwardingModeChangeConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: restore global Rx mirroring configuration */
    rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet(prvTgfEnableMirror, prvTgfIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet: %d", prvTgfEnableMirror);

    /* AUTODOC: disable Rx mirroring on port=1, index=0 */
    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet: %d %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);

    /* invalidate VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_VLANID_CNS);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);
}

/**
* @internal prvTgfEgressMirrorForwardingModeChangeConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Set analyzer interface for index 3.
*         Device number 0, port number 18, port interface.
*         -    Set analyzer interface for index 0.
*         Device number 0, port number 23, port interface.
*         -    Set analyzer interface index to 3 and enable Tx mirroring.
*         - Enable Tx mirroring on port 0.
*/
GT_VOID prvTgfEgressMirrorForwardingModeChangeConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS               rc = GT_OK;


    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                 PRV_TGF_VLANID_CNS);

    /* egress mirroring */
    prvTgfMirrorAnalyzerInterfaceTypeSet(GT_TRUE);

    /* AUTODOC: set analyzer interface index=3: */
    /* AUTODOC:   analyzer devNum=0, port=2 */
    prvMirrorAnalyzerInterfaceSet(3, prvTgfDevsArray[2], prvTgfPortsArray[2]);

    /* AUTODOC: set analyzer interface index=0: */
    /* AUTODOC:   analyzer devNum=0, port=3 */
    prvMirrorAnalyzerInterfaceSet(0, prvTgfDevsArray[3], prvTgfPortsArray[3]);

    /* save default analyzer interface */
    rc = prvTgfMirrorTxGlobalAnalyzerInterfaceIndexGet(prvTgfDevNum, &prvTgfEnableMirror, &prvTgfIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxGlobalAnalyzerInterfaceIndexGet: %d", prvTgfDevNum);

    /* AUTODOC: enable global Tx mirroring index=3 for HopByHop mode */
    rc = prvTgfMirrorTxGlobalAnalyzerInterfaceIndexSet(GT_TRUE, 3);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxGlobalAnalyzerInterfaceIndexSet: %d %d", GT_TRUE, 3);

    /* AUTODOC: enable Tx mirroring on port=0, index=0 */
    rc = prvTgfMirrorTxPortSet(prvTgfPortsArray[0], GT_TRUE, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortSet: %d %d",
                                 prvTgfPortsArray[0], GT_TRUE);
}

/**
* @internal prvTgfEgressMirrorForwardingModeChangeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 0, 23.
*         2 Packets are captured on port 18.
*         Disable Tx mirroring on port 0.
*         Set source-based forwarding mode.
*         Enable Tx mirroring on port 0 and set
*         analyzer interface index for the port to 0.
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         Packet is captured on ports 0, 18.
*         2 Packets are captured on port 23.
*         Disable Tx mirroring on port 0.
*         Set hop-byhop forwarding mode.
*         Enable Tx mirroring on port 0.
*         Send to device's port 8 packet:
*         macDa = FF:FF:FF:FF:FF:FF,
*         macSa = 00:00:00:00:00:02,
*         Success Criteria:
*         1 packet is captured on ports 0, 23.
*         2 Packets are captured on port 18.
*/
GT_VOID prvTgfEgressMirrorForwardingModeChangeTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;

    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* test will use 'PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E'
            but should be ok also for 'PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E'
            that sip6 supports */
        goto after_hop_by_hop;
    }

    /* AUTODOC: GENERATE TRAFFIC: */

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: send BC packet from port 1 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:02, VID=5 */
    prvTgfMirrorTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

    /* AUTODOC: verify traffic: */
    /* AUTODOC:   port 2: 2 packets - original flood, copy to analyzer */
    /* AUTODOC:   ports 0,3: 1 packet - original flood */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        /* Verify that port 18 received 2 packets */
        if (PRV_TGF_RCV1_PORT_IDX_CNS == portIter)
        {
            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount * 2, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
        else
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: disable Tx mirroring on port=0, index=0 */
    rc = prvTgfMirrorTxPortSet(prvTgfPortsArray[0], GT_FALSE, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortSet: %d %d", prvTgfPortsArray[0], GT_FALSE);

after_hop_by_hop:

    /* AUTODOC: set Source-Based forwarding mode for mirroring to analyzer */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorToAnalyzerForwardingModeSet: %d",
                                 PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E);

    /* AUTODOC: enable Tx mirroring on port=0, index=0 */
    rc = prvTgfMirrorTxPortSet(prvTgfPortsArray[0], GT_TRUE, 0);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortSet: %d %d %d",
                                 prvTgfPortsArray[0], GT_TRUE, 0);

    /* AUTODOC: send BC packet from port 1 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:02, VID=5 */
    prvTgfMirrorTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

    /* AUTODOC: verify traffic: */
    /* AUTODOC:   port 3: 2 packets - original flood, copy to analyzer */
    /* AUTODOC:   ports 0,2: 1 packet - original flood */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        /* Verify that port 23 received 2 packets */
        if (PRV_TGF_RCV_PORT_IDX_CNS == portIter)
        {
            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount * 2, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
        else
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }

    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: disable Tx mirroring on port=0, index=0 */
    rc = prvTgfMirrorTxPortSet(prvTgfPortsArray[0], GT_FALSE, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortSet: %d %d", prvTgfPortsArray[0], GT_FALSE);

    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* no more needed from the test */
        return;
    }

    /* AUTODOC: set Hop-by-Hop forwarding mode for mirroring to analyzer */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorToAnalyzerForwardingModeSet: %d",
                                 PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E);

    /* AUTODOC: enable Tx mirroring on port=0, index=0 */
    rc = prvTgfMirrorTxPortSet(prvTgfPortsArray[0], GT_TRUE, 0);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortSet: %d %d %d",
                                 prvTgfPortsArray[0], GT_TRUE, 0);

    /* AUTODOC: send BC packet from port 1 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:02, VID=5 */
    prvTgfMirrorTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo);

    /* AUTODOC: verify traffic: */
    /* AUTODOC:   port 2: 2 packets - original flood, copy to analyzer */
    /* AUTODOC:   ports 0,3: 1 packet - original flood */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        /* Verify that port 18 received 2 packets */
        if (PRV_TGF_RCV1_PORT_IDX_CNS == portIter)
        {
            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount * 2, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
        else
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}

/**
* @internal prvTgfEgressMirrorForwardingModeChangeConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfEgressMirrorForwardingModeChangeConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* restore to ingress mirroring */
    prvTgfMirrorAnalyzerInterfaceTypeSet(GT_FALSE);

    /* AUTODOC: disable global Tx mirroring index=3 for HopByHop mode */
    rc = prvTgfMirrorTxGlobalAnalyzerInterfaceIndexSet(prvTgfEnableMirror, prvTgfIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxGlobalAnalyzerInterfaceIndexSet: %d", prvTgfEnableMirror);

    /* AUTODOC: disable Tx mirroring on port=0, index=0 */
    rc = prvTgfMirrorTxPortSet(prvTgfPortsArray[0], GT_FALSE, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortSet: %d %d",
                                 prvTgfPortsArray[0], GT_FALSE);

    /* invalidate VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_VLANID_CNS);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);
}

/**
* @internal prvTgfIngressMirrorVlanTagRemovalConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Create VLAN 5 with all ports.
*         - Remove port 18 from VLAN 5.
*         - Set analyzer interface for index 3.
*         Device number 0, port number 18, port interface.
*         - Enable Rx mirroring on port 8.
*         -    Set analyzer interface index to 3 and enable Rx mirroring.
*         - Enable VLAN tag removal of mirrored traffic.
*/
GT_VOID prvTgfIngressMirrorVlanTagRemovalConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS               rc = GT_OK;
    GT_U32                  final_analyzerIndex;
    GT_U32                  port_analyzerIndex;
    GT_U32                  global_analyzerIndex;

    global_analyzerIndex = 3;
    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* we run the test with 'end-to-end' !!!
        (the 'hop-by-hop' is ignored , and replaced by 'end-to-end') */

        /* in this mode the 'PRV_TGF_ANALYZER_IDX_CNS' is ignored and so the 'port' */
        /* so we need to set analyzer to get the packet */
        port_analyzerIndex  = 2;
        final_analyzerIndex = port_analyzerIndex;/*the per port used and not 'global'*/
    }
    else
    {
        port_analyzerIndex  = 0;
        final_analyzerIndex = global_analyzerIndex;
    }
    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                 PRV_TGF_VLANID_CNS);

    /* AUTODOC: remove port 2 from VLAN 5 */
    rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_VLANID_CNS, prvTgfPortsArray[2]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d%d",
                                 PRV_TGF_VLANID_CNS, prvTgfPortsArray[2]);

    /* AUTODOC: enable PCL ingress policy */
    rc = prvTgfPclIngressPolicyEnable(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_TRUE);

    /* AUTODOC: enable Rx mirroring on port=1, index=0 */
    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE, port_analyzerIndex);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet: %d %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);

    /* AUTODOC: set analyzer interface index=3: */
    /* AUTODOC:   analyzer devNum=0, port=2 */
    prvMirrorAnalyzerInterfaceSet(final_analyzerIndex, prvTgfDevsArray[2], prvTgfPortsArray[2]);

    /* save default analyzer interface */
    rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexGet(prvTgfDevNum, &prvTgfEnableMirror, &prvTgfIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxGlobalAnalyzerInterfaceIndexGet: %d", prvTgfDevNum);

    /* AUTODOC: enable global Rx mirroring index=3 for HopByHop mode */
    rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet(GT_TRUE, global_analyzerIndex);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet: %d %d", GT_TRUE, global_analyzerIndex);

    /* AUTODOC: enable VLAN tag removal of mirrored traffic for port 2 */
    rc = prvTgfMirrorAnalyzerVlanTagRemoveEnableSet(prvTgfPortsArray[2], GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfcpssDxChMirrorAnalyzerVlanTagRemoveEnableSet: %d%d", prvTgfPortsArray[2], GT_TRUE);

}

/**
* @internal prvTgfMirrorVlanTagRemovalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         Success Criteria:
*         Untagged/Tagged packet is captured on port 18.
* @param[in] vfd                      - VFD info
* @param[in] ingressMirrorEnable      - GT_TRUE - Generate traffic for Vlan Tag Removal
*                                      in Ingress mirroring
*                                      - GT_FALSE - Generate traffic for Vlan Tag Removal
*                                      in Egress mirroring
*                                       None
*/
static GT_VOID prvTgfMirrorVlanTagRemovalTrafficGenerate
(
    TGF_VFD_INFO_STC     vfd,
    GT_BOOL              ingressMirrorEnable
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    CPSS_INTERFACE_INFO_STC       portInterface;
    GT_U8                trigPacketBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32               buffLen = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32               packetActualLength = 0;
    GT_U8                devNum;
    GT_U8                queue;
    TGF_NET_DSA_STC      rxParam;
    GT_BOOL              triggered;
    GT_U32               vfdNum = 1;
    TGF_CAPTURE_MODE_ENT mode;

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* enable packet trace */
    rc = tgfTrafficTracePacketByteSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "Error in tgfTrafficTracePacketByteSet %d", GT_TRUE);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* enable capture */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[2];


    if(ingressMirrorEnable == GT_TRUE)
    {
        mode = TGF_CAPTURE_MODE_PCL_E;
    }
    else
    {
        mode = TGF_CAPTURE_MODE_MIRRORING_E;
    }
    /* Start capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, mode, GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum, GT_TRUE);

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, 1, 0, NULL);

    /* send Packet from port 8 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    cpssOsTimerWkAfter(1);

    /* Stop capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, mode, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum, GT_FALSE);

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }

        /* check Tx counters */
        /* Verify that port 18 received 1 packet */
        if (PRV_TGF_RCV1_PORT_IDX_CNS == portIter)
        {
            /* check Tx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsRcv counter than expected");

            /* set max allowed buff len */
            buffLen = TGF_RX_BUFFER_MAX_SIZE_CNS;

            /* enable packet trace */
            rc = tgfTrafficTracePacketByteSet(GT_TRUE);
            UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "Error in tgfTrafficTracePacketByteSet %d", GT_TRUE);



            /* Get entry from captured packet's table */
            rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_CAPTURE_E,
                                               GT_TRUE, GT_TRUE, trigPacketBuff,
                                               &buffLen, &packetActualLength,
                                               &devNum, &queue,
                                               &rxParam);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK , rc , "Error in tgfTrafficGeneratorRxInCpuGet %d", TGF_PACKET_TYPE_CAPTURE_E);


            /* check triggers */
            rc = tgfTrafficGeneratorTxEthTriggerCheck(trigPacketBuff,
                                                      buffLen,
                                                      1,
                                                      &vfd,
                                                      &triggered);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "Error in tgfTrafficGeneratorTxEthTriggerCheck %d, %d",
                                         buffLen, vfdNum);

            UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, triggered,
                                   "Packet Tag remove is done");
        }
        else
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                         "get another goodPktsSent counter than expected");
        }
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }


}

/**
* @internal prvTgfIngressMirrorVlanTagRemovalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         Success Criteria:
*         Untagged packet is captured on port 18.
*         Disable VLAN tag removal of mirrored traffic.
*         Send to device's port 8 packet:
*         Success Criteria:
*         Tagged packet is captured on port 18.
*/
GT_VOID prvTgfIngressMirrorVlanTagRemovalTrafficGenerate
(
    GT_VOID
)
{
    TGF_VFD_INFO_STC     vfd;
    GT_STATUS    rc = GT_OK;


    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) &vfd, 0, sizeof(vfd));

    /* set VFD array of expected fields value in received packet*/
    vfd.cycleCount = 2;
    vfd.mode = TGF_VFD_MODE_STATIC_E;
    vfd.modeExtraInfo = 0;
    vfd.offset = 12;
    vfd.patternPtr[0] = 0x00;
    vfd.patternPtr[1] = 0x00;

    /* AUTODOC: send BC packet from port 1 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:02, VID=5 */
    prvTgfMirrorVlanTagRemovalTrafficGenerate(vfd, GT_TRUE);
    /* AUTODOC: verify traffic: */
    /* AUTODOC:   port 2: 1 packets - UnTagged packet */
    /* AUTODOC:   ports 0,3: 1 packet - original flood */

    cpssOsTimerWkAfter(10);

    /* AUTODOC: disable VLAN tag removal of mirrored traffic for port 2 */
    rc = prvTgfMirrorAnalyzerVlanTagRemoveEnableSet(prvTgfPortsArray[2], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfcpssDxChMirrorAnalyzerVlanTagRemoveEnableSet: %d%d", prvTgfPortsArray[2], GT_FALSE);

    /* set VFD array of expected fields value in received packet*/
    vfd.cycleCount = 4;
    vfd.patternPtr[0] = 0x81;
    vfd.patternPtr[1] = 0x00;
    vfd.patternPtr[2] = 0x00;
    vfd.patternPtr[3] = 0x05;

    /* AUTODOC: send BC packet from port 1 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:02, VID=5 */
    prvTgfMirrorVlanTagRemovalTrafficGenerate(vfd, GT_TRUE);
    /* AUTODOC: verify traffic: */
    /* AUTODOC:   port 2: 1 packets - Tagged packet */
    /* AUTODOC:   ports 0,3: 1 packet - original flood */

}


/**
* @internal prvTgfIngressMirrorVlanTagRemovalConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfIngressMirrorVlanTagRemovalConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: restore global Rx mirroring configuration */
    rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet(prvTgfEnableMirror, prvTgfIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet: %d", prvTgfEnableMirror);

    /* AUTODOC: disable Rx mirroring on port=1, index=0 */
    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet: %d %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);

    /* AUTODOC: disable PCL ingress policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);

    /* invalidate VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_VLANID_CNS);


    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);


}

/**
* @internal prvTgfEgressMirrorVlanTagRemovalConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         - Create VLAN 5 with all ports.
*         - Remove port 18 from VLAN 5.
*         - Set analyzer interface for index 3.
*         Device number 0, port number 18, port interface.
*         - Enable Rx mirroring on port 8.
*         -    Set analyzer interface index to 3 and enable Rx mirroring.
*         - Enable VLAN tag removal of mirrored traffic.
*/
GT_VOID prvTgfEgressMirrorVlanTagRemovalConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS               rc = GT_OK;
    GT_U32                  final_analyzerIndex;
    GT_U32                  port_analyzerIndex;
    GT_U32                  global_analyzerIndex;

    global_analyzerIndex = 3;
    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* we run the test with 'end-to-end' !!!
        (the 'hop-by-hop' is ignored , and replaced by 'end-to-end') */

        /* in this mode the 'PRV_TGF_ANALYZER_IDX_CNS' is ignored and so the 'port' */
        /* so we need to set analyzer to get the packet */
        port_analyzerIndex  = 2;
        final_analyzerIndex = port_analyzerIndex;/*the per port used and not 'global'*/
    }
    else
    {
        port_analyzerIndex  = 0;
        final_analyzerIndex = global_analyzerIndex;
    }


    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: create VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_CNS,
                                           prvTgfPortsArray, NULL,
                                           NULL, prvTgfPortsNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: remove port 2 from VLAN 5 */
    rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_VLANID_CNS, prvTgfPortsArray[2]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d%d",
                                 PRV_TGF_VLANID_CNS, prvTgfPortsArray[2]);


    /* AUTODOC: disable Tx mirroring on port=3, index=0 */
    rc = prvTgfMirrorTxPortSet(prvTgfPortsArray[3], GT_TRUE, port_analyzerIndex);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortSet: %d %d",
                                 prvTgfPortsArray[3], GT_TRUE);

    /* AUTODOC: add Tx mirroring tagged port 3 to VLAN 5 */
    rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum, PRV_TGF_VLANID_CNS,
                                prvTgfPortsArray[3], GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberAdd: %d %d",
                                 prvTgfPortsArray[3], GT_TRUE);

    /* egress mirroring */
    prvTgfMirrorAnalyzerInterfaceTypeSet(GT_TRUE);

    /* AUTODOC: set analyzer interface index=3: */
    /* AUTODOC:   analyzer devNum=0, port=2 */
    prvMirrorAnalyzerInterfaceSet(final_analyzerIndex, prvTgfDevsArray[2], prvTgfPortsArray[2]);

    /* save default analyzer interface */
    rc = prvTgfMirrorTxGlobalAnalyzerInterfaceIndexGet(prvTgfDevNum, &prvTgfEnableMirror, &prvTgfIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxGlobalAnalyzerInterfaceIndexGet: %d", prvTgfDevNum);

    /* AUTODOC: enable global Tx mirroring index=3 for HopByHop mode */
    rc = prvTgfMirrorTxGlobalAnalyzerInterfaceIndexSet(GT_TRUE, global_analyzerIndex);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxGlobalAnalyzerInterfaceIndexSet: %d %d", GT_TRUE, global_analyzerIndex);

    /* AUTODOC: enable VLAN tag removal of mirrored traffic for port 2 */
    rc = prvTgfMirrorAnalyzerVlanTagRemoveEnableSet(prvTgfPortsArray[2], GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfcpssDxChMirrorAnalyzerVlanTagRemoveEnableSet: %d%d", prvTgfPortsArray[2], GT_TRUE);
}

/**
* @internal prvTgfEgressMirrorVlanTagRemovalTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 8 packet:
*         Success Criteria:
*         Untagged packet is captured on port 18.
*         Disable VLAN tag removal of mirrored traffic.
*         Send to device's port 8 packet:
*         Success Criteria:
*         Tagged packet is captured on port 18.
*/
GT_VOID prvTgfEgressMirrorVlanTagRemovalTrafficGenerate
(
    GT_VOID
)
{
    TGF_VFD_INFO_STC     vfd;
    GT_STATUS    rc = GT_OK;


    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) &vfd, 0, sizeof(vfd));

    /* set VFD array of expected fields value in received packet*/
    vfd.cycleCount = 2;
    vfd.mode = TGF_VFD_MODE_STATIC_E;
    vfd.modeExtraInfo = 0;
    vfd.offset = 12;
    vfd.patternPtr[0] = 0x00;
    vfd.patternPtr[1] = 0x00;

    /* AUTODOC: send BC packet from port 1 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:02, VID=5 */
    prvTgfMirrorVlanTagRemovalTrafficGenerate(vfd, GT_FALSE);
    /* AUTODOC: verify traffic: */
    /* AUTODOC:   port 2: 1 packets - UnTagged packet */
    /* AUTODOC:   ports 0,3: 1 packet - original flood */

    cpssOsTimerWkAfter(10);

    /* AUTODOC: disable VLAN tag removal of mirrored traffic for port 2 */
    rc = prvTgfMirrorAnalyzerVlanTagRemoveEnableSet(prvTgfPortsArray[2], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfcpssDxChMirrorAnalyzerVlanTagRemoveEnableSet: %d%d", prvTgfPortsArray[2], GT_FALSE);

    /* set VFD array of expected fields value in received packet*/
    vfd.cycleCount = 4;
    vfd.patternPtr[0] = 0x81;
    vfd.patternPtr[1] = 0x00;
    vfd.patternPtr[2] = 0x00;
    vfd.patternPtr[3] = 0x05;

    /* AUTODOC: send BC packet from port 1 with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:02, VID=5 */
    prvTgfMirrorVlanTagRemovalTrafficGenerate(vfd, GT_FALSE);
    /* AUTODOC: verify traffic: */
    /* AUTODOC:   port 2: 1 packets - Tagged packet */
    /* AUTODOC:   ports 0,3: 1 packet - original flood */
}


/**
* @internal prvTgfEgressMirrorVlanTagRemovalConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfEgressMirrorVlanTagRemovalConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* restore to ingress mirroring */
    prvTgfMirrorAnalyzerInterfaceTypeSet(GT_FALSE);

    /* AUTODOC: disable global Tx mirroring index=3 for HopByHop mode */
    rc = prvTgfMirrorTxGlobalAnalyzerInterfaceIndexSet(prvTgfEnableMirror, prvTgfIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxGlobalAnalyzerInterfaceIndexSet: %d", prvTgfEnableMirror);

    /* AUTODOC: disable Tx mirroring on port=3, index=0 */
    rc = prvTgfMirrorTxPortSet(prvTgfPortsArray[3], GT_FALSE, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortSet: %d %d",
                                 prvTgfPortsArray[3], GT_FALSE);

    /* invalidate VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_VLANID_CNS);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);
}

/**
* @internal dsaTagEnableSet function
* @endinternal
*
* @brief   enable/disable running the test in DSA tag mode.
*
* @param[in] enable                   - enable/disable running the test in DSA tag mode.
*                                       None
*/
static void dsaTagEnableSet(
    IN GT_BOOL                  enable
)
{
    GT_STATUS   rc;
    GT_U32         portIter;
    GT_BOOL     dualDeviceId = GT_FALSE;

    if(enable == GT_TRUE)
    {
        /* set DSA tag info related values */

        /* bind the DSA tag info */
        prvTgfPacketPartArray[DSA_INFO_INDEX].type    = TGF_PACKET_PART_DSA_TAG_E;
        prvTgfPacketPartArray[DSA_INFO_INDEX].partPtr = &prvTgfPacketDsaTagPart;
        prvTgfPacketPartArray[VLAN_TAG_INFO_INDEX].type    = TGF_PACKET_PART_SKIP_E;

        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            if((prvTgfPortsArray[portIter] >= 64) && (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum) == GT_FALSE))
            {
                dualDeviceId = GT_TRUE;
            }
        }

        if(0 == PRV_CPSS_IS_DUAL_HW_DEVICE_MAC(PRV_CPSS_PP_MAC(prvTgfDevNum)->hwDevNum))
        {
            dualDeviceId = GT_FALSE;
        }

        if(dualDeviceId == GT_TRUE &&
           HWINIT_GLOVAR(sysGenGlobalInfo.supportDualDeviceId) == GT_TRUE)
        {
            /* allow the src port to be >= 64 */
            rc = prvTgfCscdDbRemoteHwDevNumModeSet(DUMMY_REMOTE_DEV_NUM_CNS,CPSS_GEN_CFG_HW_DEV_NUM_MODE_DUAL_E);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        }

        /* get the original sate of the filter on src device -- for DSA tagged traffic */
        rc = prvTgfCscdDsaSrcDevFilterGet(&origSrcDevFilter);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        /* disable the filter on src device  -- for DSA tagged traffic */
        rc = prvTgfCscdDsaSrcDevFilterSet(GT_FALSE);/* needed due to Erratum from xcat devices */
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    }
    else /*when restore values make sure to do all restore without rc checking !*/
    {
        /* restore values */
        prvTgfPacketPartArray[VLAN_TAG_INFO_INDEX].type    = TGF_PACKET_PART_VLAN_TAG_E;

        prvTgfPacketPartArray[DSA_INFO_INDEX].type    = TGF_PACKET_PART_SKIP_E;
        prvTgfPacketPartArray[DSA_INFO_INDEX].partPtr = NULL;

        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            tgfTrafficGeneratorIngressCscdPortEnableSet(prvTgfDevsArray[portIter],prvTgfPortsArray[portIter],GT_FALSE);
        }

        prvTgfCscdDbRemoteHwDevNumModeSet(DUMMY_REMOTE_DEV_NUM_CNS,CPSS_GEN_CFG_HW_DEV_NUM_MODE_SINGLE_E);

        /* restore the filter on src device  -- for DSA tagged traffic */
        prvTgfCscdDsaSrcDevFilterSet(origSrcDevFilter);
    }
}

/**
* @internal prvTgfMirrorConfigParamsSet function
* @endinternal
*
* @brief   1. enable/disable running the test in DSA tag mode.
*         2. set DSA tag type
*         3. set mirroring mode
* @param[in] dsaTagEnable             - enable/disable running the test in DSA tag mode.
* @param[in] dsaType                  - DSA type.
* @param[in] mirrorMode               - mirroring mode.
*
* @retval GT_TRUE                  - configuration is supported
* @retval GT_FALSE                 - configuration is NOT supported
*/
GT_BOOL prvTgfMirrorConfigParamsSet
(
    IN GT_BOOL                  dsaTagEnable,
    IN TGF_DSA_TYPE_ENT         dsaType,
    IN PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT   mirrorMode
)
{
#ifdef CHX_FAMILY
    if(((dsaType !=TGF_DSA_2_WORD_TYPE_E) && (dsaType != TGF_DSA_4_WORD_TYPE_E)) ||
        ((0 == UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum)) &&
          (dsaType == TGF_DSA_4_WORD_TYPE_E)))
    {
        /*  the dsaType != 2/4 words ,or
            dsaType == 4 words and the device not support it  */
        return GT_FALSE;
    }

    if(0 == UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum) &&
        mirrorMode != PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E &&
        mirrorMode != PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E)
    {
        /* the device not supports other types then: hop-by-hop and srcBased */
        return GT_FALSE;
    }

#endif/*CHX_FAMILY*/

    utfGeneralStateMessageSave(0,"dsaTagEnable = %d",dsaTagEnable);
    utfGeneralStateMessageSave(1,"dsaType = %s",
        dsaType == TGF_DSA_1_WORD_TYPE_E ? "TGF_DSA_1_WORD_TYPE_E" :
        dsaType == TGF_DSA_2_WORD_TYPE_E ? "TGF_DSA_2_WORD_TYPE_E" :
        dsaType == TGF_DSA_3_WORD_TYPE_E ? "TGF_DSA_3_WORD_TYPE_E" :
        dsaType == TGF_DSA_4_WORD_TYPE_E ? "TGF_DSA_4_WORD_TYPE_E" :
        "unknown");
    utfGeneralStateMessageSave(2,"mirrorMode = %s",
        mirrorMode == PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E ? "PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E" :
        mirrorMode == PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E ? "PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E" :
        mirrorMode == PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E ? "PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E" :
                "unknown");

    testingMirrorMode = mirrorMode;
    prvTgfPacketDsaTagPart.dsaType = dsaType;

    if(testingDsaTag == dsaTagEnable)
    {
        /* no 'restore' / 'new config' needed */
        return GT_TRUE;
    }

    testingDsaTag = dsaTagEnable;

    /*enable/disable running the test in DSA tag mode*/
    dsaTagEnableSet(dsaTagEnable);

    return GT_TRUE;
}

/* restore e2Phy configurations */
static void restoreE2Phy(void)
{
    CPSS_INTERFACE_INFO_STC physicalInfo;/* physical port info for the analyzer in eArch */
    GT_PORT_NUM analyzerTrgEPortArr[4]; /* analyzer target eport used in the tests */
    GT_U32  iiMax = sizeof(analyzerTrgEPortArr)/sizeof(analyzerTrgEPortArr[0]);
    GT_U32  ii;

    ii = 0;
    analyzerTrgEPortArr[ii++] = DUMMY_TARGET_EPORT_NUM_CNS;
    analyzerTrgEPortArr[ii++] = DUMMY_TARGET_EPORT_NUM_CNS - 1;
    analyzerTrgEPortArr[ii++] = TARGET_EPORT_NUM_CNS;
    analyzerTrgEPortArr[ii++] = TARGET_EPORT_NUM_CNS -1;


#ifdef CHX_FAMILY
    if(0 == UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(prvTgfDevNum))
    {
        return;
    }
#endif /*CHX_FAMILY*/

    physicalInfo.type = CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.hwDevNum = prvTgfDevNum;
    physicalInfo.devPort.portNum = 0;

    for(ii = 0 ; ii < iiMax ; ii++)
    {
        prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                    analyzerTrgEPortArr[ii],
                    &physicalInfo);
    }

    return;
}

#define NON_DEBUG_CNS   0xFFFFFFFF
static GT_U32    debug_specificCaseId = NON_DEBUG_CNS;
static GT_U32    debug_specificTrafficType = NON_DEBUG_CNS;

typedef struct{
    /* indication that eport/physical port need mirroring */
    GT_BOOL mirrorEn;
    /* the analyzer that given when mirrorEn == GT_TRUE */
    GT_U32  analyzerIndex;

    /* the actual analyzer that will get the traffic :
    NO_MIRRORING_CNS  - means no mirroring --> no analyzer.
    SAME_ANALYZER_CNS - means that same index as analyzerIndex.
    other value - the analyzer index to get the traffic (0..6)
     */
    GT_U32  actualAnalyzerIndex;
}MIRROR_INFO_STC;

typedef struct {
    MIRROR_INFO_STC     physicalPortInfo;  /* physical port info */
    MIRROR_INFO_STC     ePort1Info      ;  /* eport 1 info */
    MIRROR_INFO_STC     ePort2Info      ;  /* eport 2 info */
}MIRROR_INFO_3_PORTS_STC;

/* indication to use analyzerIndex and not other index */
#define SAME_ANALYZER_CNS   0x0000FFFF
/* indication that mirroring not expected */
#define NO_MIRRORING_CNS    0xFFFFFFFF

enum{
    /* analyzer index for physical port --> LOWER then those of the eports */
    PHYSICAL_PORT_LOW_ANALYZER_INDEX_E    = 3,
    EPORT_1_ANALYZER_INDEX_E              = 4,
    EPORT_2_ANALYZER_INDEX_E              = 5,
    /* analyzer index for physical port --> HIGHER then those of the eports */
    PHYSICAL_PORT_HIGH_ANALYZER_INDEX_E   = 6
};

static MIRROR_INFO_3_PORTS_STC  mirrorSystemArr[] =
{
/*
case 1:
    set pA with rx mirror (analyzer 3)
    set e1 with rx mirror (analyzer 4)
    set e2 with rx mirror (analyzer 5)
    --> check:
        traffic from e1 generate mirroring --> to analyzer 4.
        traffic from e2 generate mirroring --> to analyzer 5.
        other traffic from pA generate mirroring --> to analyzer 3.
*/
    {
/*physicalPortInfo*/ {GT_TRUE   , PHYSICAL_PORT_LOW_ANALYZER_INDEX_E ,    SAME_ANALYZER_CNS}
/*ePort1Info      */,{GT_TRUE   , EPORT_1_ANALYZER_INDEX_E ,              SAME_ANALYZER_CNS}
/*ePort2Info      */,{GT_TRUE   , EPORT_2_ANALYZER_INDEX_E ,              SAME_ANALYZER_CNS}
    }
/*
case 2:
    unset e1 from rx mirror
    --> check:
        traffic from e1 generate mirroring --> to analyzer 3(due to physical).
        traffic from e2 generate mirroring --> to analyzer 5.
        other traffic from pA generate mirroring --> to analyzer 3.
*/
    ,{
/*physicalPortInfo*/ {GT_TRUE   , PHYSICAL_PORT_LOW_ANALYZER_INDEX_E ,  SAME_ANALYZER_CNS}
/*ePort1Info      */,{GT_FALSE  , EPORT_1_ANALYZER_INDEX_E ,            PHYSICAL_PORT_LOW_ANALYZER_INDEX_E}
/*ePort2Info      */,{GT_TRUE   , EPORT_2_ANALYZER_INDEX_E ,            SAME_ANALYZER_CNS}
    }
/*
case 3:
    unset e2 from rx mirror
    --> check:
        traffic from e1 generate mirroring --> to analyzer 3(due to physical).
        traffic from e2 generate mirroring --> to analyzer 3(due to physical).
        other traffic from pA generate mirroring --> to analyzer 3.
*/
    ,{
/*physicalPortInfo*/ {GT_TRUE   , PHYSICAL_PORT_LOW_ANALYZER_INDEX_E ,  SAME_ANALYZER_CNS}
/*ePort1Info      */,{GT_FALSE  , EPORT_1_ANALYZER_INDEX_E ,            PHYSICAL_PORT_LOW_ANALYZER_INDEX_E}
/*ePort2Info      */,{GT_FALSE  , EPORT_2_ANALYZER_INDEX_E ,            PHYSICAL_PORT_LOW_ANALYZER_INDEX_E}
    }
/*
case 4:
    set pA with rx mirror (analyzer 6)
    set e1 with rx mirror (analyzer 4)
    set e2 with rx mirror (analyzer 5)
    --> check:
        traffic from e1 generate mirroring --> to analyzer 6(due to physical).
        traffic from e2 generate mirroring --> to analyzer 6(due to physical).
        other traffic from pA generate mirroring --> to analyzer 3.
*/
    ,{
/*physicalPortInfo*/ {GT_TRUE   , PHYSICAL_PORT_HIGH_ANALYZER_INDEX_E ,   SAME_ANALYZER_CNS}
/*ePort1Info      */,{GT_TRUE   , EPORT_1_ANALYZER_INDEX_E ,              PHYSICAL_PORT_HIGH_ANALYZER_INDEX_E}
/*ePort2Info      */,{GT_TRUE   , EPORT_2_ANALYZER_INDEX_E ,              PHYSICAL_PORT_HIGH_ANALYZER_INDEX_E}
    }
/*
case 5:
    unset pA from rx mirror
    set e1 with rx mirror (analyzer 4)
    set e2 with rx mirror (analyzer 5)
    --> check:
        traffic from e1 generate mirroring --> to analyzer 4.
        traffic from e2 generate mirroring --> to analyzer 5.
        other traffic from pA NOT generate mirroring.
*/
    ,{
/*physicalPortInfo*/ {GT_FALSE  , PHYSICAL_PORT_HIGH_ANALYZER_INDEX_E ,   NO_MIRRORING_CNS }
/*ePort1Info      */,{GT_TRUE   , EPORT_1_ANALYZER_INDEX_E ,              SAME_ANALYZER_CNS}
/*ePort2Info      */,{GT_TRUE   , EPORT_2_ANALYZER_INDEX_E ,              SAME_ANALYZER_CNS}
    }

/*
case 6:
    unset e1 from rx mirror
    --> check:
        traffic from e1 NOT generate mirroring.
        traffic from e2 generate mirroring --> to analyzer 5.
        other traffic from pA NOT generate mirroring.
*/
    ,{
/*physicalPortInfo*/ {GT_FALSE  , PHYSICAL_PORT_HIGH_ANALYZER_INDEX_E ,   NO_MIRRORING_CNS }
/*ePort1Info      */,{GT_FALSE  , EPORT_1_ANALYZER_INDEX_E ,              NO_MIRRORING_CNS }
/*ePort2Info      */,{GT_TRUE   , EPORT_2_ANALYZER_INDEX_E ,              SAME_ANALYZER_CNS}
    }

/*
case 7:
    unset e2 from rx mirror
    --> check:
        traffic from e1 NOT generate mirroring.
        traffic from e2 NOT generate mirroring.
        other traffic from pA NOT generate mirroring.
*/
    ,{
/*physicalPortInfo*/ {GT_FALSE  , PHYSICAL_PORT_HIGH_ANALYZER_INDEX_E ,   NO_MIRRORING_CNS }
/*ePort1Info      */,{GT_FALSE  , EPORT_1_ANALYZER_INDEX_E ,              NO_MIRRORING_CNS }
/*ePort2Info      */,{GT_FALSE  , EPORT_2_ANALYZER_INDEX_E ,              NO_MIRRORING_CNS}
    }


};
static GT_U32  mirrorSystemNumCases = sizeof(mirrorSystemArr) / sizeof(mirrorSystemArr[0]);

/* src eport 1 */
#define SRC_EPORT_1_CNS     UTF_CPSS_PP_VALID_EPORT1_NUM_CNS
/* src eport 2 */
#define SRC_EPORT_2_CNS     UTF_CPSS_PP_VALID_EPORT2_NUM_CNS

/* trg port that is (link status) DOWN and or not member in vlan and or not in tested ports */
#define TRG_PORT_LINK_STATUS_DOWN_CNS   19


typedef enum {
    /* send traffic to be classified as src eport 1 */
    SRC_TRAFFIC_TYPE_EPORT_1_E,
    /* send traffic to be classified as src eport 2 */
    SRC_TRAFFIC_TYPE_EPORT_2_E,
    /* send traffic to be NOT do src eport re-assign (keep the default src Eport) */
    SRC_TRAFFIC_TYPE_PHYSICAL_PORT_E,

    SRC_TRAFFIC_TYPE_LAST_E/* must be last */

}SRC_TRAFFIC_TYPE_ENT;

/* array of vlan tags to use for classifing the src eport 1,2 from default eport */
static GT_U16   vlanTagsArr[SRC_TRAFFIC_TYPE_LAST_E] = {
    0x111,  /* vlan id for eport 1 */
    0x222,  /* vlan id for eport 2 */
    0x399   /* vlan id for those that are not eport 1 or eport 2 */
};

/* default tunnel term entry index (first rule) */
static GT_U32        prvTgfTtiRuleLookup0Index;
static MIRROR_INFO_3_PORTS_STC null_portsMirrInfo;/*'all disabled'*/


/**
* @internal ingressMirrorEportVsPhysicalPort_configMirrorig function
* @endinternal
*
* @brief   configure mirroring for specific case of eport vs physical port ingress mirroring
*/
static GT_VOID ingressMirrorEportVsPhysicalPort_configMirrorig
(
    IN MIRROR_INFO_3_PORTS_STC *portsMirrInfoPtr
)
{
    GT_STATUS                rc;
    MIRROR_INFO_STC *currentPortInfoPtr;
    GT_PORT_NUM     currentPortNum;
    GT_BOOL         isPhysicalPort;

    if(portsMirrInfoPtr == NULL)
    {
        /* restore values */
        portsMirrInfoPtr = &null_portsMirrInfo;
    }

    /* set the physical port mirroring configurations */
    currentPortInfoPtr = &portsMirrInfoPtr->physicalPortInfo;
    currentPortNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    isPhysicalPort = GT_TRUE;

    rc = prvTgfMirrorRxPortTypeSet(
            currentPortNum,
            isPhysicalPort,
            currentPortInfoPtr->mirrorEn, /*enable*/
            currentPortInfoPtr->analyzerIndex); /*index*/
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortTypeSet");


    /* set the eport1 mirroring configurations */
    currentPortInfoPtr = &portsMirrInfoPtr->ePort1Info;
    currentPortNum = SRC_EPORT_1_CNS;
    isPhysicalPort = GT_FALSE;

    rc = prvTgfMirrorRxPortTypeSet(
            currentPortNum,
            isPhysicalPort,
            currentPortInfoPtr->mirrorEn, /*enable*/
            currentPortInfoPtr->analyzerIndex); /*index*/
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortTypeSet");

    /* set the eport2 mirroring configurations */
    currentPortInfoPtr = &portsMirrInfoPtr->ePort2Info;
    currentPortNum = SRC_EPORT_2_CNS;
    isPhysicalPort = GT_FALSE;

    rc = prvTgfMirrorRxPortTypeSet(
            currentPortNum,
            isPhysicalPort,
            currentPortInfoPtr->mirrorEn, /*enable*/
            currentPortInfoPtr->analyzerIndex); /*index*/
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortTypeSet");


}

/**
* @internal ingressMirrorEportVsPhysicalPort_generateTraffic function
* @endinternal
*
* @brief   generate traffic to be associated wit the src interface type.
*/
static GT_VOID ingressMirrorEportVsPhysicalPort_generateTraffic
(
    IN SRC_TRAFFIC_TYPE_ENT srcTrafficType
)
{
    GT_STATUS   rc;
    TGF_PACKET_STC      *packetInfoPtr_sender = &prvTgfPacketInfo;
    GT_U32  senderPortIndex = PRV_TGF_SEND_PORT_IDX_CNS;

    /* this is the vlan tag in prvTgfPacketInfo */
    prvTgfPacketVlanTag0Part.vid = vlanTagsArr[srcTrafficType];

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr_sender,
        prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[senderPortIndex]);

    /* start transmitting */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[senderPortIndex]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[senderPortIndex]);

}


/* expected counters : all counters should be ZERO */
static CPSS_PORT_MAC_COUNTER_SET_STC   allZeroCntrs;
/**
* @internal checkGotNoPacket function
* @endinternal
*
*/
static GT_VOID checkGotNoPacket
(
    IN GT_U32 egressPortIndex
)
{
    GT_STATUS   rc;
    GT_BOOL                         isEqualCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;/* current counters of the port */

    rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[egressPortIndex], GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[egressPortIndex]);

    /* compare the counters */
    PRV_TGF_VERIFY_COUNTERS_MAC(isEqualCntrs, allZeroCntrs, portCntrs);
    UTF_VERIFY_EQUAL0_STRING_MAC(isEqualCntrs, GT_TRUE, "get another counters values.");

    /* print expected values if not equal */
    PRV_TGF_PRINT_DIFF_COUNTERS_MAC(isEqualCntrs, allZeroCntrs, portCntrs);
}

/**
* @internal checkGotPacket function
* @endinternal
*
*/
static GT_VOID checkGotPacket
(
    IN GT_U32 egressPortIndex
)
{
    GT_STATUS   rc;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;/* current counters of the port */

    rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[egressPortIndex], GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[egressPortIndex]);

    /* check that egress packets as expected */
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount , portCntrs.goodPktsSent.l[0], "get another egress packets values.");
}


/**
* @internal getIngressAnalyzerEgressPortIndexFromAnalyzerIndex function
* @endinternal
*
* @brief   get index in prvTgfPortsArray[] (0..3) for the 'analyzer index' (0..6)
*/
static GT_U32  getIngressAnalyzerEgressPortIndexFromAnalyzerIndex(IN GT_U32   analyzerIndex)
{
    switch(analyzerIndex)
    {
        case    PHYSICAL_PORT_HIGH_ANALYZER_INDEX_E:
        case    PHYSICAL_PORT_LOW_ANALYZER_INDEX_E:
            return 0;
        case    EPORT_1_ANALYZER_INDEX_E:
            return 2;
        case    EPORT_2_ANALYZER_INDEX_E:
            return 3;
        default:
            return NO_MIRRORING_CNS;
    }
}

/**
* @internal ingressMirrorEportVsPhysicalPort_checkEgressPorts function
* @endinternal
*
* @brief   check egress port of ingress analyzer that got the traffic.
*/
static GT_VOID ingressMirrorEportVsPhysicalPort_checkEgressPorts
(
    IN SRC_TRAFFIC_TYPE_ENT srcTrafficType,
    IN MIRROR_INFO_3_PORTS_STC *portsMirrInfoPtr
)
{
    GT_U32  ii;
    GT_U32  actualAnalyzerIndex;
    GT_U32  senderPortIndex = PRV_TGF_SEND_PORT_IDX_CNS;
    GT_U32  expectedAnalyzerEgressPortIndex;/* index in prvTgfPortsArray[] */
    MIRROR_INFO_STC *currentMirrorInfoPtr;

    switch(srcTrafficType)
    {
        case SRC_TRAFFIC_TYPE_EPORT_1_E:
            currentMirrorInfoPtr = &portsMirrInfoPtr->ePort1Info;
            break;
        case SRC_TRAFFIC_TYPE_EPORT_2_E:
            currentMirrorInfoPtr = &portsMirrInfoPtr->ePort2Info;
            break;
        case SRC_TRAFFIC_TYPE_PHYSICAL_PORT_E:
            currentMirrorInfoPtr = &portsMirrInfoPtr->physicalPortInfo;
            break;
        default: /* should not happen */
            return;
    }

    actualAnalyzerIndex =
        currentMirrorInfoPtr->actualAnalyzerIndex ==  SAME_ANALYZER_CNS ?
        currentMirrorInfoPtr->analyzerIndex :
        currentMirrorInfoPtr->actualAnalyzerIndex;

    expectedAnalyzerEgressPortIndex =
        getIngressAnalyzerEgressPortIndexFromAnalyzerIndex(actualAnalyzerIndex);

    for (ii = 0 ; ii < prvTgfPortsNum; ii ++)
    {
        if(ii == senderPortIndex)
        {
            continue;
        }

        if(expectedAnalyzerEgressPortIndex == ii)
        {
            PRV_UTF_LOG1_MAC("port[%d] expected to get the traffic \n",
                prvTgfPortsArray[ii]);

            /* port expected to get the traffic */
            checkGotPacket(ii);
        }
        else
        {
            PRV_UTF_LOG1_MAC("port[%d] not expected to get the traffic \n",
                prvTgfPortsArray[ii]);
            /* port not expected to get the traffic */
            checkGotNoPacket(ii);
        }
    }

    /* reset port's counters */
    PRV_UTF_LOG0_MAC("reset ETH counters \n");
    prvTgfEthCountersReset(prvTgfDevNum);

}

/**
* @internal ingressMirrorEportVsPhysicalPortTest function
* @endinternal
*
* @brief   test a specific case of eport vs physical port ingress mirroring
*/
static GT_VOID ingressMirrorEportVsPhysicalPortTest
(
    IN MIRROR_INFO_3_PORTS_STC *portsMirrInfoPtr
)
{
    SRC_TRAFFIC_TYPE_ENT    trafficType;

    PRV_UTF_LOG3_MAC("physicalPortInfo: mirrorEn[%d],analyzerIndex[0x%x]actualAnalyzerIndex[0x%x] \n",
        portsMirrInfoPtr->physicalPortInfo.mirrorEn,
        portsMirrInfoPtr->physicalPortInfo.analyzerIndex,
        portsMirrInfoPtr->physicalPortInfo.actualAnalyzerIndex
        );
    PRV_UTF_LOG3_MAC("ePort1Info: mirrorEn[%d],analyzerIndex[0x%x]actualAnalyzerIndex[0x%x] \n",
        portsMirrInfoPtr->ePort1Info.mirrorEn,
        portsMirrInfoPtr->ePort1Info.analyzerIndex,
        portsMirrInfoPtr->ePort1Info.actualAnalyzerIndex
        );
    PRV_UTF_LOG3_MAC("ePort2Info: mirrorEn[%d],analyzerIndex[0x%x]actualAnalyzerIndex[0x%x] \n",
        portsMirrInfoPtr->ePort2Info.mirrorEn,
        portsMirrInfoPtr->ePort2Info.analyzerIndex,
        portsMirrInfoPtr->ePort2Info.actualAnalyzerIndex
        );


    utfGeneralStateMessageSave(1,"physicalPortInfo: mirrorEn[%d],analyzerIndex[0x%x]actualAnalyzerIndex[0x%x]",
        portsMirrInfoPtr->physicalPortInfo.mirrorEn,
        portsMirrInfoPtr->physicalPortInfo.analyzerIndex,
        portsMirrInfoPtr->physicalPortInfo.actualAnalyzerIndex
        );
    utfGeneralStateMessageSave(2,"ePort1Info: mirrorEn[%d],analyzerIndex[0x%x]actualAnalyzerIndex[0x%x]",
        portsMirrInfoPtr->ePort1Info.mirrorEn,
        portsMirrInfoPtr->ePort1Info.analyzerIndex,
        portsMirrInfoPtr->ePort1Info.actualAnalyzerIndex
        );
    utfGeneralStateMessageSave(3,"ePort2Info: mirrorEn[%d],analyzerIndex[0x%x]actualAnalyzerIndex[0x%x]",
        portsMirrInfoPtr->ePort2Info.mirrorEn,
        portsMirrInfoPtr->ePort2Info.analyzerIndex,
        portsMirrInfoPtr->ePort2Info.actualAnalyzerIndex
        );

    /* configure mirroring */
    ingressMirrorEportVsPhysicalPort_configMirrorig(portsMirrInfoPtr);


    for(trafficType = SRC_TRAFFIC_TYPE_EPORT_1_E ;
        trafficType < SRC_TRAFFIC_TYPE_LAST_E ;
        trafficType++)
    {
        if(debug_specificTrafficType != NON_DEBUG_CNS)
        {
            if(debug_specificTrafficType != (GT_U32)trafficType)
            {
                /* skip this not debugged more */
                continue;
            }
        }

        PRV_UTF_LOG1_MAC("send trafficType:[%d] \n",trafficType);
        utfGeneralStateMessageSave(4,"send trafficType:[%d]",trafficType);
        /* generate traffic */
        ingressMirrorEportVsPhysicalPort_generateTraffic(trafficType);

        PRV_UTF_LOG0_MAC("check egress analyzer that got the traffic \n");
        /* check egress port of ingress analyzer that got the traffic */
        ingressMirrorEportVsPhysicalPort_checkEgressPorts(trafficType, portsMirrInfoPtr);
    }
}

/**
* @internal ingressMirrorEportVsPhysicalPortTest_BasicConfig_bridge function
* @endinternal
*
* @brief   set basic initialization configurations for the test:
*         bridge - vlan entry for the traffic.
*/
static GT_VOID ingressMirrorEportVsPhysicalPortTest_BasicConfig_bridge
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: create VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                 PRV_TGF_VLANID_CNS);
}

/**
* @internal ingressMirrorEportVsPhysicalPortTest_BasicConfig_tti function
* @endinternal
*
* @brief   set basic initialization configurations for the test:
*         tti rules to assign new src eport , set evid = 'exists' vid
*/
static GT_VOID ingressMirrorEportVsPhysicalPortTest_BasicConfig_tti
(
    GT_VOID
)
{
    GT_STATUS                rc;
    PRV_TGF_TTI_ACTION_2_STC ttiAction2;
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;
    PRV_TGF_TTI_ETH_RULE_STC    *ethPatternPtr;
    PRV_TGF_TTI_ETH_RULE_STC    *ethMaskPtr;
    GT_U32  senderPortIndex = PRV_TGF_SEND_PORT_IDX_CNS;
    GT_U32  ttiIndex;

    prvTgfMirrorTcamSingleHitConfig(PRV_TGF_TTI_FLOOR1_CNS, GROUP_1, HIT_NUM_0);

    prvTgfTtiRuleLookup0Index = prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(0, 0);
    ttiIndex = prvTgfTtiRuleLookup0Index;

    /* AUTODOC: SETUP TTI CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: enable TTI lookup for port 0, key TTI_KEY_ETH */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[senderPortIndex],
                                      PRV_TGF_TTI_KEY_ETH_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d",
                                 prvTgfDevNum);

    /* clear TTI Rule Pattern, Mask to match all packets */
    cpssOsMemSet((GT_VOID*) &ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet((GT_VOID*) &ttiMask,    0, sizeof(ttiMask));
    cpssOsMemSet((GT_VOID*) &ttiAction2, 0, sizeof(ttiAction2));

    ethPatternPtr = &ttiPattern.eth;
    ethMaskPtr    = &ttiMask.eth;

    ethPatternPtr->common.isTagged = GT_TRUE;

    ethMaskPtr->common.isTagged = GT_TRUE;
    ethMaskPtr->common.vid = BIT_13 - 1;/*13 bits*/

    /* AUTODOC: set the TTI Rule for ETH Type Key */
    ttiAction2.tag0VlanCmd = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
    ttiAction2.tag0VlanId = PRV_TGF_VLANID_CNS;
    /* AUTODOC : assign egress interface --> to force target port that will not egress the packets ! */
    ttiAction2.redirectCommand = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ttiAction2.egressInterface.type = CPSS_INTERFACE_PORT_E;
    ttiAction2.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    ttiAction2.egressInterface.devPort.portNum = TRG_PORT_LINK_STATUS_DOWN_CNS;
    /* AUTODOC : do not let the bridge change the redirect decisions */
    ttiAction2.bridgeBypass = GT_TRUE;

    ethPatternPtr->common.vid = vlanTagsArr[SRC_TRAFFIC_TYPE_EPORT_1_E];
    ttiAction2.sourceEPortAssignmentEnable    = GT_TRUE;
    ttiAction2.sourceEPort = SRC_EPORT_1_CNS;
    /* AUTODOC: add TTI rules  */
    rc = prvTgfTtiRule2Set(
        ttiIndex ,
        PRV_TGF_TTI_KEY_ETH_E,
        &ttiPattern, &ttiMask, &ttiAction2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");

    ttiIndex += 3;

    ethPatternPtr->common.vid = vlanTagsArr[SRC_TRAFFIC_TYPE_EPORT_2_E];
    ttiAction2.sourceEPortAssignmentEnable    = GT_TRUE;
    ttiAction2.sourceEPort = SRC_EPORT_2_CNS;
    rc = prvTgfTtiRule2Set(
        ttiIndex ,
        PRV_TGF_TTI_KEY_ETH_E,
        &ttiPattern, &ttiMask, &ttiAction2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");


    ttiIndex += 3;

    ethPatternPtr->common.vid = vlanTagsArr[SRC_TRAFFIC_TYPE_PHYSICAL_PORT_E];
    ttiAction2.sourceEPortAssignmentEnable    = GT_FALSE;/*keep default eport --> the 'physical port'*/
    ttiAction2.sourceEPort = 0;/*ignores*/
    rc = prvTgfTtiRule2Set(
        ttiIndex ,
        PRV_TGF_TTI_KEY_ETH_E,
        &ttiPattern, &ttiMask, &ttiAction2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");
}

/**
* @internal ingressMirrorEportVsPhysicalPortTest_BasicConfig_analyzerPort function
* @endinternal
*
* @brief   set analyzer port
*/
static GT_VOID ingressMirrorEportVsPhysicalPortTest_BasicConfig_analyzerPort
(
    IN GT_U32  analyzerIndex,
    IN GT_BOOL  enable
)
{
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC  analyzerInterfaceInfo;
    GT_U32  portIndex;

    analyzerInterfaceInfo.interface.type = CPSS_INTERFACE_PORT_E;
    analyzerInterfaceInfo.interface.devPort.hwDevNum = prvTgfDevNum;

    if(enable == GT_TRUE)
    {
        portIndex = getIngressAnalyzerEgressPortIndexFromAnalyzerIndex(analyzerIndex);
        analyzerInterfaceInfo.interface.devPort.portNum = prvTgfPortsArray[portIndex];
    }
    else
    {
        /* HW default value */
        analyzerInterfaceInfo.interface.devPort.portNum = 0;
    }

    /* AUTODOC: set analyzer port for analyzer */
    prvTgfMirrorAnalyzerInterfaceSet(analyzerIndex , &analyzerInterfaceInfo);
}

/**
* @internal ingressMirrorEportVsPhysicalPortTest_BasicConfig_analyzerPorts function
* @endinternal
*
* @brief   set analyzer ports
*/
static GT_VOID ingressMirrorEportVsPhysicalPortTest_BasicConfig_analyzerPorts
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_BOOL enable = GT_TRUE;

    /* AUTODOC: define analyzer port for analyzer 3 */
    ingressMirrorEportVsPhysicalPortTest_BasicConfig_analyzerPort(PHYSICAL_PORT_LOW_ANALYZER_INDEX_E,enable);
    /* AUTODOC: define analyzer port for analyzer 4 */
    ingressMirrorEportVsPhysicalPortTest_BasicConfig_analyzerPort(EPORT_1_ANALYZER_INDEX_E,enable);
    /* AUTODOC: define analyzer port for analyzer 5 */
    ingressMirrorEportVsPhysicalPortTest_BasicConfig_analyzerPort(EPORT_2_ANALYZER_INDEX_E,enable);
    /* AUTODOC: define analyzer port for analyzer 6 */
    ingressMirrorEportVsPhysicalPortTest_BasicConfig_analyzerPort(PHYSICAL_PORT_HIGH_ANALYZER_INDEX_E,enable);

    /*  Set source-based forwarding mode. */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorToAnalyzerForwardingModeSet: %d",
                                 PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E);
}

/**
* @internal ingressMirrorEportVsPhysicalPortTest_BasicConfig function
* @endinternal
*
* @brief   set basic initialization configurations for the test:
*         set bridge
*         tti rules to assign new src eport , set evid = 'exists' vid
*         set analyzer ports
*/
static GT_VOID ingressMirrorEportVsPhysicalPortTest_BasicConfig
(
    GT_VOID
)
{
    utfGeneralStateMessageSave(0,"basic config bridge");
    ingressMirrorEportVsPhysicalPortTest_BasicConfig_bridge();

    utfGeneralStateMessageSave(0,"basic config tti");
    ingressMirrorEportVsPhysicalPortTest_BasicConfig_tti();

    utfGeneralStateMessageSave(0,"basic config analyzerPorts");
    ingressMirrorEportVsPhysicalPortTest_BasicConfig_analyzerPorts();

}


/**
* @internal ingressMirrorEportVsPhysicalPortTest_configRestore_analyzerPorts function
* @endinternal
*
* @brief   set analyzer ports
*/
static GT_VOID ingressMirrorEportVsPhysicalPortTest_configRestore_analyzerPorts
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_BOOL enable = GT_FALSE;

    /* AUTODOC: define analyzer port for analyzer 3 */
    ingressMirrorEportVsPhysicalPortTest_BasicConfig_analyzerPort(PHYSICAL_PORT_LOW_ANALYZER_INDEX_E,enable);
    /* AUTODOC: define analyzer port for analyzer 4 */
    ingressMirrorEportVsPhysicalPortTest_BasicConfig_analyzerPort(EPORT_1_ANALYZER_INDEX_E,enable);
    /* AUTODOC: define analyzer port for analyzer 5 */
    ingressMirrorEportVsPhysicalPortTest_BasicConfig_analyzerPort(EPORT_2_ANALYZER_INDEX_E,enable);
    /* AUTODOC: define analyzer port for analyzer 6 */
    ingressMirrorEportVsPhysicalPortTest_BasicConfig_analyzerPort(PHYSICAL_PORT_HIGH_ANALYZER_INDEX_E,enable);

    /*  restore analyzer forwarding mode. */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(testingMirrorMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorToAnalyzerForwardingModeSet: %d",
                                 testingMirrorMode);

}

/**
* @internal ingressMirrorEportVsPhysicalPortTest_configRestore_bridge function
* @endinternal
*
* @brief   restore bridge
*/
static GT_VOID ingressMirrorEportVsPhysicalPortTest_configRestore_bridge
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: invalidate VLAN 5 */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_VLANID_CNS);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

}

/**
* @internal ingressMirrorEportVsPhysicalPortTest_configRestore_tti function
* @endinternal
*
* @brief   restore tti
*/
static GT_VOID ingressMirrorEportVsPhysicalPortTest_configRestore_tti
(
    GT_VOID
)
{
    GT_STATUS                rc;
    GT_U32  senderPortIndex = PRV_TGF_SEND_PORT_IDX_CNS;
    GT_U32  ttiIndex;

    prvTgfTtiRuleLookup0Index = prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(0, 0);
    ttiIndex = prvTgfTtiRuleLookup0Index;

    /* AUTODOC: restore TTI CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= restore TTI Configuration =======\n");

    /* AUTODOC: disable TTI lookup for port 0, key TTI_KEY_ETH */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[senderPortIndex],
                                      PRV_TGF_TTI_KEY_ETH_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: invalidate TTI rules  */
    rc = prvTgfTtiRuleValidStatusSet(
        ttiIndex ,
        GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");

    ttiIndex += 3;

    rc = prvTgfTtiRuleValidStatusSet(
        ttiIndex ,
        GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");

    ttiIndex += 3;

    rc = prvTgfTtiRuleValidStatusSet(
        ttiIndex ,
        GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");

    prvTgfMirrorTcamRestore(PRV_TGF_TTI_FLOOR1_CNS);
}


/**
* @internal ingressMirrorEportVsPhysicalPortTest_configRestore function
* @endinternal
*
* @brief   restore all configurations
*/
static GT_VOID ingressMirrorEportVsPhysicalPortTest_configRestore
(
    GT_VOID
)
{
    utfGeneralStateMessageSave(0,"restore config");

    ingressMirrorEportVsPhysicalPortTest_configRestore_bridge();

    utfGeneralStateMessageSave(0,"restore config tti");
    ingressMirrorEportVsPhysicalPortTest_configRestore_tti();

    utfGeneralStateMessageSave(0,"restore config analyzerPorts");
    ingressMirrorEportVsPhysicalPortTest_configRestore_analyzerPorts();

    utfGeneralStateMessageSave(0,"restore config mirror ports");
    ingressMirrorEportVsPhysicalPort_configMirrorig(NULL);

}


/**
* @internal prvTgfIngressMirrorEportVsPhysicalPortTest function
* @endinternal
*
* @brief   configure and test next (and restore) :
*         Test EPort Vs Physical port ingress mirroring.
*         set eport e1 , ePort e2 both associated with physical port pA.
*         use tti to classify eVlan = 0x10 from port pA as e1
*         use tti to classify eVlan = 0x20 from port pA as e2
*         (all others keep the default eport and not get new assignment)
*         case 1:
*         set pA with rx mirror (analyzer 3)
*         set e1 with rx mirror (analyzer 4)
*         set e2 with rx mirror (analyzer 5)
*         --> check:
*         traffic from e1 generate mirroring --> to analyzer 4.
*         traffic from e2 generate mirroring --> to analyzer 5.
*         other traffic from pA generate mirroring --> to analyzer 3.
*         case 2:
*         unset e1 from rx mirror
*         --> check:
*         traffic from e1 generate mirroring --> to analyzer 3(due to physical).
*         traffic from e2 generate mirroring --> to analyzer 5.
*         other traffic from pA generate mirroring --> to analyzer 3.
*         case 3:
*         unset e2 from rx mirror
*         --> check:
*         traffic from e1 generate mirroring --> to analyzer 3(due to physical).
*         traffic from e2 generate mirroring --> to analyzer 3(due to physical).
*         other traffic from pA generate mirroring --> to analyzer 3.
*         case 4:
*         set pA with rx mirror (analyzer 6)
*         set e1 with rx mirror (analyzer 4)
*         set e2 with rx mirror (analyzer 5)
*         --> check:
*         traffic from e1 generate mirroring --> to analyzer 6(due to physical).
*         traffic from e2 generate mirroring --> to analyzer 6(due to physical).
*         other traffic from pA generate mirroring --> to analyzer 3.
*         case 5:
*         unset pA from rx mirror
*         set e1 with rx mirror (analyzer 4)
*         set e2 with rx mirror (analyzer 5)
*         --> check:
*         traffic from e1 generate mirroring --> to analyzer 4.
*         traffic from e2 generate mirroring --> to analyzer 5.
*         other traffic from pA NOT generate mirroring.
*         case 6:
*         unset e1 from rx mirror
*         --> check:
*         traffic from e1 NOT generate mirroring.
*         traffic from e2 generate mirroring --> to analyzer 5.
*         other traffic from pA NOT generate mirroring.
*         case 7:
*         unset e2 from rx mirror
*         --> check:
*         traffic from e1 NOT generate mirroring.
*         traffic from e2 NOT generate mirroring.
*         other traffic from pA NOT generate mirroring.
*/
GT_VOID prvTgfIngressMirrorEportVsPhysicalPortTest
(
    GT_VOID
)
{
    GT_U32  ii;

    /* reset port's counters */
    PRV_UTF_LOG0_MAC("reset ETH counters , and force link up on the ports \n");
    prvTgfEthCountersReset(prvTgfDevNum);

    utfGeneralStateMessageSave(0,"basic config");
    ingressMirrorEportVsPhysicalPortTest_BasicConfig();

    for(ii = 0 ; ii < mirrorSystemNumCases ; ii++)
    {
        if(debug_specificCaseId != NON_DEBUG_CNS)
        {
            if(debug_specificCaseId != ii)
            {
                /* skip this not debugged more */
                continue;
            }
        }
        utfGeneralStateMessageSave(0,"test case [%d]",ii);
        PRV_UTF_LOG1_MAC("test case [%d] \n",ii);
        ingressMirrorEportVsPhysicalPortTest(&mirrorSystemArr[ii]);
    }

    PRV_UTF_LOG0_MAC("restore config \n");
    utfGeneralStateMessageSave(0,"restore config");
    ingressMirrorEportVsPhysicalPortTest_configRestore();

}

/* debug indications */
GT_VOID prvTgfIngressMirrorEportVsPhysicalPortTest_debugIndications
(
    IN GT_U32   specificCaseId,/*0..6 - case to test . 0xFFFFFFFF - all */
    IN GT_U32   specificTrafficType/* 0..2 - traffic type (see SRC_TRAFFIC_TYPE_ENT),0xFFFFFFFF - all*/
)
{
    debug_specificCaseId = specificCaseId;
    debug_specificTrafficType = specificTrafficType;
}

/* target eport 1 */
#define TRG_EPORT_1_CNS     UTF_CPSS_PP_VALID_EPORT3_NUM_CNS
/* target eport 2 */
#define TRG_EPORT_2_CNS     UTF_CPSS_PP_VALID_EPORT4_NUM_CNS

/* trg port # 4 in addition to ports 0..3 */
#define PHY_PORT_4_CNS   5


/**
* @internal egressMirrorEportVsPhysicalPortTest_BasicConfig_bridge function
* @endinternal
*
* @brief   set basic initialization configurations for the test:
*         bridge - vlan entry for the traffic.
*/
static GT_VOID egressMirrorEportVsPhysicalPortTest_BasicConfig_bridge
(
    GT_VOID
)
{
    GT_STATUS   rc;
    CPSS_INTERFACE_INFO_STC                     physicalInfo;
    GT_U32  egressPortIndex = PRV_TGF_RCV_PORT_IDX_CNS;
    GT_U32  trgEport;

    /* AUTODOC: create VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                 PRV_TGF_VLANID_CNS);


    /* AUTODOC : set the e2Phy to set physical port for the target eports */
    /* AUTODOC : both target eports on the same physical port.  */
    physicalInfo.type = CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.hwDevNum = prvTgfDevNum;
    physicalInfo.devPort.portNum = prvTgfPortsArray[egressPortIndex];

    trgEport = TRG_EPORT_1_CNS;
    /* set first eport */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                trgEport,
                &physicalInfo);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: %d, %d",
                                 prvTgfDevNum, trgEport);

    trgEport = TRG_EPORT_2_CNS;
    /* set second eport */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                trgEport,
                &physicalInfo);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: %d, %d",
                                 prvTgfDevNum, trgEport);
}

/**
* @internal egressMirrorEportVsPhysicalPortTest_BasicConfig_tti function
* @endinternal
*
* @brief   set basic initialization configurations for the test:
*         tti rules to assign redirect target eport , set evid = 'exists' vid
*/
static GT_VOID egressMirrorEportVsPhysicalPortTest_BasicConfig_tti
(
    GT_VOID
)
{
    GT_STATUS                rc;
    PRV_TGF_TTI_ACTION_2_STC ttiAction2;
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;
    PRV_TGF_TTI_ETH_RULE_STC    *ethPatternPtr;
    PRV_TGF_TTI_ETH_RULE_STC    *ethMaskPtr;
    GT_U32  senderPortIndex = PRV_TGF_SEND_PORT_IDX_CNS;
    GT_U32  ttiIndex;

    prvTgfMirrorTcamSingleHitConfig(PRV_TGF_TTI_FLOOR1_CNS, GROUP_1, HIT_NUM_0);

    prvTgfTtiRuleLookup0Index = prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(0, 0);
    ttiIndex = prvTgfTtiRuleLookup0Index;

    /* AUTODOC: SETUP TTI CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: enable TTI lookup for port 0, key TTI_KEY_ETH */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[senderPortIndex],
                                      PRV_TGF_TTI_KEY_ETH_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d",
                                 prvTgfDevNum);

    /* clear TTI Rule Pattern, Mask to match all packets */
    cpssOsMemSet((GT_VOID*) &ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet((GT_VOID*) &ttiMask,    0, sizeof(ttiMask));
    cpssOsMemSet((GT_VOID*) &ttiAction2, 0, sizeof(ttiAction2));

    ethPatternPtr = &ttiPattern.eth;
    ethMaskPtr    = &ttiMask.eth;

    ethPatternPtr->common.isTagged = GT_TRUE;

    ethMaskPtr->common.isTagged = GT_TRUE;
    ethMaskPtr->common.vid = BIT_13 - 1;/*13 bits*/

    /* AUTODOC: set the TTI Rule for ETH Type Key */
    ttiAction2.tag0VlanCmd = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
    ttiAction2.tag0VlanId = PRV_TGF_VLANID_CNS;
    /* AUTODOC:  assign egress interface --> to force target eport that will egress the packets , and will have egress mirror */
    ttiAction2.redirectCommand = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ttiAction2.egressInterface.type = CPSS_INTERFACE_PORT_E;
    ttiAction2.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    /* AUTODOC : do not let the bridge change the redirect decisions */
    ttiAction2.bridgeBypass = GT_TRUE;

    ethPatternPtr->common.vid = vlanTagsArr[SRC_TRAFFIC_TYPE_EPORT_1_E];
    ttiAction2.sourceEPortAssignmentEnable    = GT_TRUE;
    ttiAction2.sourceEPort = SRC_EPORT_1_CNS;
    ttiAction2.egressInterface.devPort.portNum = TRG_EPORT_1_CNS;

    /* AUTODOC: add TTI rules  */
    rc = prvTgfTtiRule2Set(
        ttiIndex ,
        PRV_TGF_TTI_KEY_ETH_E,
        &ttiPattern, &ttiMask, &ttiAction2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");

    ttiIndex += 3;

    ethPatternPtr->common.vid = vlanTagsArr[SRC_TRAFFIC_TYPE_EPORT_2_E];
    ttiAction2.sourceEPortAssignmentEnable    = GT_TRUE;
    ttiAction2.sourceEPort = SRC_EPORT_2_CNS;
    ttiAction2.egressInterface.devPort.portNum = TRG_EPORT_2_CNS;
    rc = prvTgfTtiRule2Set(
        ttiIndex ,
        PRV_TGF_TTI_KEY_ETH_E,
        &ttiPattern, &ttiMask, &ttiAction2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");


    ttiIndex += 3;

    ethPatternPtr->common.vid = vlanTagsArr[SRC_TRAFFIC_TYPE_PHYSICAL_PORT_E];
    ttiAction2.sourceEPortAssignmentEnable    = GT_FALSE;/*keep default eport --> the 'physical port'*/
    ttiAction2.sourceEPort = 0;/*ignores*/
    ttiAction2.egressInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_PORT_IDX_CNS] ;

    rc = prvTgfTtiRule2Set(
        ttiIndex ,
        PRV_TGF_TTI_KEY_ETH_E,
        &ttiPattern, &ttiMask, &ttiAction2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");
}

/**
* @internal getEgressAnalyzerEgressPortIndexFromAnalyzerIndex function
* @endinternal
*
* @brief   get index in prvTgfPortsArray[] (0..4) for the 'analyzer index' (0..6)
*/
static GT_U32  getEgressAnalyzerEgressPortIndexFromAnalyzerIndex(IN GT_U32   analyzerIndex)
{
    /* ingress traffic from index prvTgfPortsArray[1] , egress to index prvTgfPortsArray[3].
       so analyzer can NOT be index 1,3.

       for 3 analyzers we need another port (total 5 ports in test !)
     */
    switch(analyzerIndex)
    {
        case    PHYSICAL_PORT_HIGH_ANALYZER_INDEX_E:
        case    PHYSICAL_PORT_LOW_ANALYZER_INDEX_E:
            return 4;/* this is 5'th port of the test --> tests uses 5 ports !!! */
        case    EPORT_1_ANALYZER_INDEX_E:
            return 2;
        case    EPORT_2_ANALYZER_INDEX_E:
            return 0;
        default:
            return NO_MIRRORING_CNS;
    }
}

/**
* @internal egressMirrorEportVsPhysicalPortTest_BasicConfig_analyzerPort function
* @endinternal
*
* @brief   set analyzer port
*/
static GT_VOID egressMirrorEportVsPhysicalPortTest_BasicConfig_analyzerPort
(
    IN GT_U32  analyzerIndex,
    IN GT_BOOL  enable
)
{
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC  analyzerInterfaceInfo;
    GT_U32  portIndex;

    analyzerInterfaceInfo.interface.type = CPSS_INTERFACE_PORT_E;
    analyzerInterfaceInfo.interface.devPort.hwDevNum = prvTgfDevNum;

    if(enable == GT_TRUE)
    {
        portIndex = getEgressAnalyzerEgressPortIndexFromAnalyzerIndex(analyzerIndex);
        analyzerInterfaceInfo.interface.devPort.portNum = prvTgfPortsArray[portIndex];
    }
    else
    {
        /* HW default value */
        analyzerInterfaceInfo.interface.devPort.portNum = 0;
    }

    /* AUTODOC: set analyzer port for analyzer */
    prvTgfMirrorAnalyzerInterfaceSet(analyzerIndex , &analyzerInterfaceInfo);
}

/**
* @internal egressMirrorEportVsPhysicalPortTest_BasicConfig_analyzerPorts function
* @endinternal
*
* @brief   set analyzer ports
*/
static GT_VOID egressMirrorEportVsPhysicalPortTest_BasicConfig_analyzerPorts
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_BOOL enable = GT_TRUE;

    /* AUTODOC: define analyzer port for analyzer 3 */
    egressMirrorEportVsPhysicalPortTest_BasicConfig_analyzerPort(PHYSICAL_PORT_LOW_ANALYZER_INDEX_E,enable);
    /* AUTODOC: define analyzer port for analyzer 4 */
    egressMirrorEportVsPhysicalPortTest_BasicConfig_analyzerPort(EPORT_1_ANALYZER_INDEX_E,enable);
    /* AUTODOC: define analyzer port for analyzer 5 */
    egressMirrorEportVsPhysicalPortTest_BasicConfig_analyzerPort(EPORT_2_ANALYZER_INDEX_E,enable);
    /* AUTODOC: define analyzer port for analyzer 6 */
    egressMirrorEportVsPhysicalPortTest_BasicConfig_analyzerPort(PHYSICAL_PORT_HIGH_ANALYZER_INDEX_E,enable);

    /*  Set source-based forwarding mode. */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorToAnalyzerForwardingModeSet: %d",
                                 PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E);
}

/**
* @internal egressMirrorEportVsPhysicalPortTest_BasicConfig function
* @endinternal
*
* @brief   set basic initialization configurations for the test:
*         set bridge
*         tti rules to assign redirect target eport , set evid = 'exists' vid
*         set analyzer ports
*/
static GT_VOID egressMirrorEportVsPhysicalPortTest_BasicConfig
(
    GT_VOID
)
{
    utfGeneralStateMessageSave(0,"basic config bridge");
    egressMirrorEportVsPhysicalPortTest_BasicConfig_bridge();

    utfGeneralStateMessageSave(0,"basic config tti");
    egressMirrorEportVsPhysicalPortTest_BasicConfig_tti();

    utfGeneralStateMessageSave(0,"basic config analyzerPorts");
    egressMirrorEportVsPhysicalPortTest_BasicConfig_analyzerPorts();

}

/**
* @internal egressMirrorEportVsPhysicalPort_configMirrorig function
* @endinternal
*
* @brief   configure mirroring for specific case of eport vs physical port egress mirroring
*/
static GT_VOID egressMirrorEportVsPhysicalPort_configMirrorig
(
    IN MIRROR_INFO_3_PORTS_STC *portsMirrInfoPtr
)
{
    GT_STATUS                rc;
    MIRROR_INFO_STC *currentPortInfoPtr;
    GT_PORT_NUM     currentPortNum;
    GT_BOOL         isPhysicalPort;

    if(portsMirrInfoPtr == NULL)
    {
        /* restore values */
        portsMirrInfoPtr = &null_portsMirrInfo;
    }

    /* set the physical port mirroring configurations */
    currentPortInfoPtr = &portsMirrInfoPtr->physicalPortInfo;
    currentPortNum = prvTgfPortsArray[PRV_TGF_RCV_PORT_IDX_CNS];
    isPhysicalPort = GT_TRUE;

    rc = prvTgfMirrorTxPortTypeSet(
            currentPortNum,
            isPhysicalPort,
            currentPortInfoPtr->mirrorEn, /*enable*/
            currentPortInfoPtr->analyzerIndex); /*index*/
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortTypeSet");


    /* set the eport1 mirroring configurations */
    currentPortInfoPtr = &portsMirrInfoPtr->ePort1Info;
    currentPortNum = TRG_EPORT_1_CNS;
    isPhysicalPort = GT_FALSE;

    rc = prvTgfMirrorTxPortTypeSet(
            currentPortNum,
            isPhysicalPort,
            currentPortInfoPtr->mirrorEn, /*enable*/
            currentPortInfoPtr->analyzerIndex); /*index*/
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortTypeSet");

    /* set the eport2 mirroring configurations */
    currentPortInfoPtr = &portsMirrInfoPtr->ePort2Info;
    currentPortNum = TRG_EPORT_2_CNS;
    isPhysicalPort = GT_FALSE;

    rc = prvTgfMirrorTxPortTypeSet(
            currentPortNum,
            isPhysicalPort,
            currentPortInfoPtr->mirrorEn, /*enable*/
            currentPortInfoPtr->analyzerIndex); /*index*/
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortTypeSet");
}

/**
* @internal egressMirrorEportVsPhysicalPort_generateTraffic function
* @endinternal
*
* @brief   generate traffic to be associated wit the src interface type.
*/
static GT_VOID egressMirrorEportVsPhysicalPort_generateTraffic
(
    IN SRC_TRAFFIC_TYPE_ENT srcTrafficType
)
{
    GT_STATUS   rc;
    TGF_PACKET_STC      *packetInfoPtr_sender = &prvTgfPacketInfo;
    GT_U32  senderPortIndex = PRV_TGF_SEND_PORT_IDX_CNS;

    /* this is the vlan tag in prvTgfPacketInfo */
    prvTgfPacketVlanTag0Part.vid = vlanTagsArr[srcTrafficType];

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr_sender,
        prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[senderPortIndex]);

    /* start transmitting */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[senderPortIndex]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[senderPortIndex]);

}

/**
* @internal egressMirrorEportVsPhysicalPort_checkEgressPorts function
* @endinternal
*
* @brief   check egress port of egress analyzer that got the traffic.
*/
static GT_VOID egressMirrorEportVsPhysicalPort_checkEgressPorts
(
    IN SRC_TRAFFIC_TYPE_ENT srcTrafficType,
    IN MIRROR_INFO_3_PORTS_STC *portsMirrInfoPtr
)
{
    GT_U32  ii;
    GT_U32  actualAnalyzerIndex;
    GT_U32  senderPortIndex = PRV_TGF_SEND_PORT_IDX_CNS;
    GT_U32  egressPortIndex = PRV_TGF_RCV_PORT_IDX_CNS;
    GT_U32  expectedAnalyzerEgressPortIndex;/* index in prvTgfPortsArray[] */
    MIRROR_INFO_STC *currentMirrorInfoPtr;

    switch(srcTrafficType)
    {
        case SRC_TRAFFIC_TYPE_EPORT_1_E:
            currentMirrorInfoPtr = &portsMirrInfoPtr->ePort1Info;
            break;
        case SRC_TRAFFIC_TYPE_EPORT_2_E:
            currentMirrorInfoPtr = &portsMirrInfoPtr->ePort2Info;
            break;
        case SRC_TRAFFIC_TYPE_PHYSICAL_PORT_E:
            currentMirrorInfoPtr = &portsMirrInfoPtr->physicalPortInfo;
            break;
        default: /* should not happen */
            return;
    }

    actualAnalyzerIndex =
        currentMirrorInfoPtr->actualAnalyzerIndex ==  SAME_ANALYZER_CNS ?
        currentMirrorInfoPtr->analyzerIndex :
        currentMirrorInfoPtr->actualAnalyzerIndex;

    expectedAnalyzerEgressPortIndex =
        getEgressAnalyzerEgressPortIndexFromAnalyzerIndex(actualAnalyzerIndex);

    for (ii = 0 ; ii < prvTgfPortsNum; ii ++)
    {
        if(ii == senderPortIndex || ii == egressPortIndex)
        {
            continue;
        }

        if(expectedAnalyzerEgressPortIndex == ii)
        {
            PRV_UTF_LOG1_MAC("port[%d] expected to get the traffic \n",
                prvTgfPortsArray[ii]);

            /* port expected to get the traffic */
            checkGotPacket(ii);
        }
        else
        {
            PRV_UTF_LOG1_MAC("port[%d] not expected to get the traffic \n",
                prvTgfPortsArray[ii]);
            /* port not expected to get the traffic */
            checkGotNoPacket(ii);
        }
    }

    /* reset port's counters */
    PRV_UTF_LOG0_MAC("reset ETH counters \n");
    prvTgfEthCountersReset(prvTgfDevNum);

}

/**
* @internal egressMirrorEportVsPhysicalPortTest function
* @endinternal
*
* @brief   test a specific case of eport vs physical port egress mirroring
*/
static GT_VOID egressMirrorEportVsPhysicalPortTest
(
    IN MIRROR_INFO_3_PORTS_STC *portsMirrInfoPtr
)
{
    SRC_TRAFFIC_TYPE_ENT    trafficType;

    PRV_UTF_LOG3_MAC("physicalPortInfo: mirrorEn[%d],analyzerIndex[0x%x]actualAnalyzerIndex[0x%x] \n",
        portsMirrInfoPtr->physicalPortInfo.mirrorEn,
        portsMirrInfoPtr->physicalPortInfo.analyzerIndex,
        portsMirrInfoPtr->physicalPortInfo.actualAnalyzerIndex
        );
    PRV_UTF_LOG3_MAC("ePort1Info: mirrorEn[%d],analyzerIndex[0x%x]actualAnalyzerIndex[0x%x] \n",
        portsMirrInfoPtr->ePort1Info.mirrorEn,
        portsMirrInfoPtr->ePort1Info.analyzerIndex,
        portsMirrInfoPtr->ePort1Info.actualAnalyzerIndex
        );
    PRV_UTF_LOG3_MAC("ePort2Info: mirrorEn[%d],analyzerIndex[0x%x]actualAnalyzerIndex[0x%x] \n",
        portsMirrInfoPtr->ePort2Info.mirrorEn,
        portsMirrInfoPtr->ePort2Info.analyzerIndex,
        portsMirrInfoPtr->ePort2Info.actualAnalyzerIndex
        );


    utfGeneralStateMessageSave(1,"physicalPortInfo: mirrorEn[%d],analyzerIndex[0x%x]actualAnalyzerIndex[0x%x]",
        portsMirrInfoPtr->physicalPortInfo.mirrorEn,
        portsMirrInfoPtr->physicalPortInfo.analyzerIndex,
        portsMirrInfoPtr->physicalPortInfo.actualAnalyzerIndex
        );
    utfGeneralStateMessageSave(2,"ePort1Info: mirrorEn[%d],analyzerIndex[0x%x]actualAnalyzerIndex[0x%x]",
        portsMirrInfoPtr->ePort1Info.mirrorEn,
        portsMirrInfoPtr->ePort1Info.analyzerIndex,
        portsMirrInfoPtr->ePort1Info.actualAnalyzerIndex
        );
    utfGeneralStateMessageSave(3,"ePort2Info: mirrorEn[%d],analyzerIndex[0x%x]actualAnalyzerIndex[0x%x]",
        portsMirrInfoPtr->ePort2Info.mirrorEn,
        portsMirrInfoPtr->ePort2Info.analyzerIndex,
        portsMirrInfoPtr->ePort2Info.actualAnalyzerIndex
        );

    /* configure mirroring */
    egressMirrorEportVsPhysicalPort_configMirrorig(portsMirrInfoPtr);


    for(trafficType = SRC_TRAFFIC_TYPE_EPORT_1_E ;
        trafficType < SRC_TRAFFIC_TYPE_LAST_E ;
        trafficType++)
    {
        if(debug_specificTrafficType != NON_DEBUG_CNS)
        {
            if(debug_specificTrafficType != (GT_U32)trafficType)
            {
                /* skip this not debugged more */
                continue;
            }
        }

        PRV_UTF_LOG1_MAC("send trafficType:[%d] \n",trafficType);
        utfGeneralStateMessageSave(4,"send trafficType:[%d]",trafficType);
        /* generate traffic */
        egressMirrorEportVsPhysicalPort_generateTraffic(trafficType);

        PRV_UTF_LOG0_MAC("check egress analyzer that got the traffic \n");
        /* check egress port of egress analyzer that got the traffic */
        egressMirrorEportVsPhysicalPort_checkEgressPorts(trafficType, portsMirrInfoPtr);
    }
}

/**
* @internal egressMirrorEportVsPhysicalPortTest_configRestore_bridge function
* @endinternal
*
* @brief   restore bridge
*/
static GT_VOID egressMirrorEportVsPhysicalPortTest_configRestore_bridge
(
    GT_VOID
)
{
    GT_STATUS   rc;
    CPSS_INTERFACE_INFO_STC                     physicalInfo;
    GT_U32  trgEport;

    /* AUTODOC: invalidate VLAN 5 */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_VLANID_CNS);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);


    /* AUTODOC: create VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                 PRV_TGF_VLANID_CNS);


    /* AUTODOC : unset the e2Phy to set physical port for the target eports */
    physicalInfo.type = CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.hwDevNum = prvTgfDevNum;
    physicalInfo.devPort.portNum = 0;

    trgEport = TRG_EPORT_1_CNS;
    /* set first eport */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                trgEport,
                &physicalInfo);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: %d, %d",
                                 prvTgfDevNum, trgEport);

    trgEport = TRG_EPORT_2_CNS;
    /* set second eport */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                trgEport,
                &physicalInfo);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: %d, %d",
                                 prvTgfDevNum, trgEport);
}

/**
* @internal egressMirrorEportVsPhysicalPortTest_configRestore_tti function
* @endinternal
*
* @brief   restore tti
*/
static GT_VOID egressMirrorEportVsPhysicalPortTest_configRestore_tti
(
    GT_VOID
)
{
    GT_STATUS                rc;
    GT_U32  senderPortIndex = PRV_TGF_SEND_PORT_IDX_CNS;
    GT_U32  ttiIndex;

    prvTgfTtiRuleLookup0Index = prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(0, 0);
    ttiIndex = prvTgfTtiRuleLookup0Index;

    /* AUTODOC: restore TTI CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= restore TTI Configuration =======\n");

    /* AUTODOC: disable TTI lookup for port 0, key TTI_KEY_ETH */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[senderPortIndex],
                                      PRV_TGF_TTI_KEY_ETH_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: invalidate TTI rules  */
    rc = prvTgfTtiRuleValidStatusSet(
        ttiIndex ,
        GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");

    ttiIndex += 3;

    rc = prvTgfTtiRuleValidStatusSet(
        ttiIndex ,
        GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");

    ttiIndex += 3;

    rc = prvTgfTtiRuleValidStatusSet(
        ttiIndex ,
        GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");

    prvTgfMirrorTcamRestore(PRV_TGF_TTI_FLOOR1_CNS);
}

/**
* @internal egressMirrorEportVsPhysicalPortTest_configRestore_analyzerPorts function
* @endinternal
*
* @brief   set analyzer ports
*/
static GT_VOID egressMirrorEportVsPhysicalPortTest_configRestore_analyzerPorts
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_BOOL enable = GT_FALSE;

    /* AUTODOC: define analyzer port for analyzer 3 */
    egressMirrorEportVsPhysicalPortTest_BasicConfig_analyzerPort(PHYSICAL_PORT_LOW_ANALYZER_INDEX_E,enable);
    /* AUTODOC: define analyzer port for analyzer 4 */
    egressMirrorEportVsPhysicalPortTest_BasicConfig_analyzerPort(EPORT_1_ANALYZER_INDEX_E,enable);
    /* AUTODOC: define analyzer port for analyzer 5 */
    egressMirrorEportVsPhysicalPortTest_BasicConfig_analyzerPort(EPORT_2_ANALYZER_INDEX_E,enable);
    /* AUTODOC: define analyzer port for analyzer 6 */
    egressMirrorEportVsPhysicalPortTest_BasicConfig_analyzerPort(PHYSICAL_PORT_HIGH_ANALYZER_INDEX_E,enable);

    /*  restore analyzer forwarding mode. */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(testingMirrorMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorToAnalyzerForwardingModeSet: %d",
                                 testingMirrorMode);

}

/**
* @internal egressMirrorEportVsPhysicalPortTest_configRestore function
* @endinternal
*
* @brief   restore all configurations
*/
static GT_VOID egressMirrorEportVsPhysicalPortTest_configRestore
(
    GT_VOID
)
{
    utfGeneralStateMessageSave(0,"restore config");

    egressMirrorEportVsPhysicalPortTest_configRestore_bridge();

    utfGeneralStateMessageSave(0,"restore config tti");
    egressMirrorEportVsPhysicalPortTest_configRestore_tti();

    utfGeneralStateMessageSave(0,"restore config analyzerPorts");
    egressMirrorEportVsPhysicalPortTest_configRestore_analyzerPorts();

    utfGeneralStateMessageSave(0,"restore config mirror ports");
    egressMirrorEportVsPhysicalPort_configMirrorig(NULL);

}


/**
* @internal prvTgfEgressMirrorEportVsPhysicalPortTest function
* @endinternal
*
* @brief   configure and test next (and restore) :
*         Test EPort Vs Physical port egress mirroring.
*         Test similar to prvTgfIngressMirrorEportVsPhysicalPort , but for 'egress mirror'
*         and not for 'ingress mirror'
*/
GT_VOID prvTgfEgressMirrorEportVsPhysicalPortTest
(
    GT_VOID
)
{
    GT_U32  ii;
    GT_U32   origPort4;
    GT_U8    origDev4,origNumPorts;
    GT_U32   analyzerPort;

    /* AUTODOC : this test using 5 physical ports (not only 4)
       ingress traffic from index prvTgfPortsArray[1] , egress to index prvTgfPortsArray[3].
       so analyzer can NOT be index 1,3.

       for 3 analyzers we need another port (total 5 ports in test !)
     */

     origPort4      = prvTgfPortsArray[4]   ;
     origDev4       = prvTgfDevsArray[4]    ;
     origNumPorts   = prvTgfPortsNum        ;

     if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devType == CPSS_98DX8332_Z0_CNS)
     {
         analyzerPort = 70;
     }
     else
     {
         analyzerPort = PHY_PORT_4_CNS;
     }

    if(MUST_USE_ORIG_PORTS_MAC(prvTgfDevNum))
    {
        /* do not modify the 12 ports in prvTgfPortsArray[] !!! */
    }
    else
    {
         prvTgfPortsArray[4] = analyzerPort;
         prvTgfDevsArray[4]  = prvTgfDevsArray[3];
    }
     prvTgfPortsNum = 5;

     {
         GT_U32 i;
         for (i = 0; (i < 4); i++)
         {
             if (prvTgfPortsArray[i] == prvTgfPortsArray[4])
             {
                 prvTgfPortsArray[4]    = origPort4     ;
                 prvTgfDevsArray[4]     = origDev4      ;
                 prvTgfPortsNum         = origNumPorts  ;
                 UTF_VERIFY_EQUAL0_STRING_MAC(
                     GT_OK, GT_FAIL,
                     "Test not compatible with given list of ports");
                 return;
             }
         }
     }

    /* reset port's counters */
    PRV_UTF_LOG0_MAC("reset ETH counters , and force link up on the ports \n");
    prvTgfEthCountersReset(prvTgfDevNum);

    utfGeneralStateMessageSave(0,"basic config");
    egressMirrorEportVsPhysicalPortTest_BasicConfig();

    for(ii = 0 ; ii < mirrorSystemNumCases ; ii++)
    {
        if(debug_specificCaseId != NON_DEBUG_CNS)
        {
            if(debug_specificCaseId != ii)
            {
                /* skip this not debugged more */
                continue;
            }
        }
        utfGeneralStateMessageSave(0,"test case [%d]",ii);
        PRV_UTF_LOG1_MAC("test case [%d] \n",ii);
        egressMirrorEportVsPhysicalPortTest(&mirrorSystemArr[ii]);
    }

    PRV_UTF_LOG0_MAC("restore config \n");
    utfGeneralStateMessageSave(0,"restore config");
    egressMirrorEportVsPhysicalPortTest_configRestore();

    /* restore the ports array */
    prvTgfPortsArray[4] = origPort4      ;
    prvTgfDevsArray[4]  = origDev4       ;
    prvTgfPortsNum      = origNumPorts   ;
}

#define  VLAN_INGRESS_ANALYZER_PORT_CNS   3

/**
* @internal ingressVlanMirroringConfigurationSet function
* @endinternal
*
* @brief   Prepare to test:
*         1. Create VLAN of ports [0,1,2,3]
*         2. Remove port 3 from VLAN
*         3. Turn on ingress VLAN mirroring
*         4. Save prior RX global analyzer
*         5. Set new analyzer as RX global analyzer
*         6. Save configuration of GA interface
*         7. Set new GA interface (i.e. new port)
*         8. Set source based forwarding mode
*
* @note Changes global variables: prvTgfEnableMirror, prvTgfIndex,
*       originalAnalyzerInterface
*       Reads global variables: prvTgfDevNum, prvTgfPortsArray
*
*/
static GT_VOID ingressVlanMirroringConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U16    vlanId = PRV_TGF_VLANID_CNS;

    /* Create VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(vlanId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
        vlanId);

    /* Analyzer port will be prvTgfPortsArray[3],
       lets remove it from the tested VLAN */
    rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum,
        vlanId, prvTgfPortsArray[VLAN_INGRESS_ANALYZER_PORT_CNS]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: %d",
        vlanId);

    /* Enable VLAN ingress mirroring */
    rc = prvTgfBrgVlanIngressMirrorEnable(prvTgfDevNum, vlanId, GT_TRUE,
        PRV_TGF_MIRROR_GLOB_ANALYZER_IDX_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
        "prvTgfvBrgVlanIngressMirrorEnable");

    /* Save default RX global analyzer interface */
    rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexGet(prvTgfDevNum,
        &prvTgfEnableMirror, &prvTgfIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfMirrorRxGlobalAnalyzerInterfaceIndexGet: %d", prvTgfDevNum);

    /* Enable global Rx mirroring */
    rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet(GT_TRUE,
        PRV_TGF_MIRROR_GLOB_ANALYZER_IDX_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
        "prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet: %d %d", GT_TRUE,
        PRV_TGF_MIRROR_GLOB_ANALYZER_IDX_CNS);

    /* Save original analyzer interface settings */
    prvTgfMirrorAnalyzerInterfaceGet(PRV_TGF_MIRROR_GLOB_ANALYZER_IDX_CNS,
        prvTgfDevNum, &originalAnalyzerInterface);

    /* Set analyzer port for analyzer */
    prvMirrorAnalyzerInterfaceSet(PRV_TGF_MIRROR_GLOB_ANALYZER_IDX_CNS,
        prvTgfDevNum, prvTgfPortsArray[VLAN_INGRESS_ANALYZER_PORT_CNS]);

    /* Set source-based forwarding mode */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(
        PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfMirrorToAnalyzerForwardingModeSet: %d",
        PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E);
}

/**
* @internal ingressVlanMirroringTrafficGenerate function
* @endinternal
*
* @brief   1. Send frame from port that belongs VLAN
*         2. Check if any frames sent by analyzer port
*         3. Reset counters
*
* @note Reads global variables: prvTgfPacketInfoUC1, prvTgfDevNum,
*       prvTgfPortsArray, prvTgfBurstCount
*
*/
static GT_VOID ingressVlanMirroringTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS       rc;
    TGF_PACKET_STC *PacketToSendPtr = &prvTgfPacketInfoUC1;/* Unicast */
    GT_U32          senderPortIndex = PRV_TGF_SEND_PORT_IDX_CNS;
    GT_U16          vlanId = PRV_TGF_VLANID_CNS;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;

    /* This is the vlan tag in prvTgfPacketInfo */
    prvTgfPacketVlanTag0Part.vid = vlanId;

    /* Setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, PacketToSendPtr, prvTgfBurstCount,
        0, NULL);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[senderPortIndex]);

    /* Start transmitting */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum,
        prvTgfPortsArray[senderPortIndex]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth:%d, %d",
        prvTgfDevNum, prvTgfPortsArray[senderPortIndex]);

    /* Check counters */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum,
        prvTgfPortsArray[VLAN_INGRESS_ANALYZER_PORT_CNS], GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[VLAN_INGRESS_ANALYZER_PORT_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
        "get another egress packets values.");

    /* Reset port's counters */
    PRV_UTF_LOG0_MAC("Reset ETH counters \n");
    prvTgfEthCountersReset(prvTgfDevNum);

}

/**
* @internal ingressVlanMirroringConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration:
*         1. Restore hop-by-hop forwarding mode
*         2. Restore settings of analyzer interface
*         3. Restore global RX mirroring analyzer index
*         4. Disable VLAN ingress mirroring
*         5. Remove VLAN
*         6. Flush FDB entries
*
* @note Reads global variables: originalAnalyzerInterface, prvTgfEnableMirror,
*       prvTgfIndex, prvTgfDevNum
*
*/
static GT_VOID ingressVlanMirroringConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U16    vlanId = PRV_TGF_VLANID_CNS;

    /* Restore default forwarding mode */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(
        PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfMirrorToAnalyzerForwardingModeSet: %d",
        PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E);

    /* Restore original analyzer interface settings */
    prvTgfMirrorAnalyzerInterfaceSet(PRV_TGF_MIRROR_GLOB_ANALYZER_IDX_CNS,
        &originalAnalyzerInterface);

    /* Restore global Rx mirroring configuration */
    rc = prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet(prvTgfEnableMirror,
        prvTgfIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfMirrorRxGlobalAnalyzerInterfaceIndexSet: %d",
        prvTgfEnableMirror);

    /* Disable VLAN ingress mirroring. */
    rc = prvTgfBrgVlanIngressMirrorEnable(prvTgfDevNum, vlanId, GT_FALSE,
        VLAN_INGRESS_ANALYZER_PORT_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
        "prvTgfvBrgVlanIngressMirrorEnable");

    /* Invalidate VLAN 5 */
    rc = prvTgfBrgDefVlanEntryInvalidate(vlanId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfBrgDefVlanEntryInvalidate: %d", vlanId);

    /* Flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
        prvTgfDevNum, GT_TRUE);

}

/**
* @internal prvTgfIngressVlanMirroringTest function
* @endinternal
*
* @brief   Main function of ingress VLAN mirroring test.
*/
GT_VOID prvTgfIngressVlanMirroringTest
(
    GT_VOID
)
{

    utfGeneralStateMessageSave(0,"Configure test");
    ingressVlanMirroringConfigurationSet();

    utfGeneralStateMessageSave(0,"Send packets");
    ingressVlanMirroringTrafficGenerate();

    utfGeneralStateMessageSave(0,"Deconfigure test");
    ingressVlanMirroringConfigurationRestore();

}

#undef VLAN_INGRESS_ANALYZER_PORT_CNS

#define VLAN_EGRESS_ANALYZER_INDEX_CNS   3
#define VLAN_ANALYZER_PORT_INDEX_CNS 2
#define VLAN_RX_PORT_INDEX_CNS 1
#define VLAN_TX_PORT_INDEX_CNS 0
#define VLAN_ANALYZER_INDEX_CNS 5

static GT_BOOL forceFromCpuWithNonRecognizeTag = GT_TRUE;

/**
* @internal prvTgfEgressVlanMirroringTest_BasicConfig_bridge function
* @endinternal
*
* @brief   set basic initialization configurations for the test:
*         bridge - vlan entry for the traffic.
*/
static GT_VOID prvTgfEgressVlanMirroringTest_BasicConfig_bridge
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U16  vlanId = PRV_TGF_VLANID_CNS;

    /* AUTODOC: create VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(vlanId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                 vlanId);

    /* AUTODOC : analyzer port will be prvTgfPortsArray[4] , lets remove it from the tested vlan */
    rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum,vlanId,prvTgfPortsArray[4]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: %d",
                                 vlanId);

    /* AUTODOC : enable vlan egress mirroring. */
    prvTgfBrgVlanMirrorToTxAnalyzerSet(vlanId,GT_TRUE,VLAN_EGRESS_ANALYZER_INDEX_CNS);

}

/**
* @internal prvTgfEgressVlanMirroringTest_vlanEgressMirrorEnableOnPort function
* @endinternal
*
* @brief   set egress port as enabled/disabled for vlan egress mirror
*/
static GT_VOID prvTgfEgressVlanMirroringTest_vlanEgressMirrorEnableOnPort
(
    IN GT_U32   portIndex,
    IN GT_BOOL  enableVlanEgressMirror
)
{
    GT_STATUS   rc;
    rc = prvTgfMirrorTxPortVlanEnableSet(prvTgfDevsArray[portIndex] ,
        prvTgfPortsArray[portIndex] , enableVlanEgressMirror);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortVlanEnableSet: %d",
                                 prvTgfDevsArray[portIndex]);
}

/**
* @internal prvTgfEgressVlanMirroringTest_BasicConfig_analyzerPort function
* @endinternal
*
* @brief   set analyzer port
*/
static GT_VOID prvTgfEgressVlanMirroringTest_BasicConfig_analyzerPort
(
    GT_VOID
)
{
    GT_STATUS   rc;
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC  analyzerInterfaceInfo;

    GT_U32  analyzerIndex = VLAN_EGRESS_ANALYZER_INDEX_CNS;
    GT_U32  ii;

    analyzerInterfaceInfo.interface.type = CPSS_INTERFACE_PORT_E;
    analyzerInterfaceInfo.interface.devPort.hwDevNum = prvTgfDevNum;
    analyzerInterfaceInfo.interface.devPort.portNum = prvTgfPortsArray[4];

    /* AUTODOC: set analyzer port for analyzer */
    prvTgfMirrorAnalyzerInterfaceSet(analyzerIndex , &analyzerInterfaceInfo);

    /*  Set source-based forwarding mode. */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorToAnalyzerForwardingModeSet: %d",
                                 PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E);

    /* AUTODOC : set all 5 ports as 'disable' the vlan egress mirroring */
    for(ii = 0 ; ii < prvTgfPortsNum ; ii++)
    {
        prvTgfEgressVlanMirroringTest_vlanEgressMirrorEnableOnPort(ii, GT_FALSE);
    }

}


/**
* @internal prvTgfEgressVlanMirroringTest_BasicConfig function
* @endinternal
*
* @brief   set basic initialization configurations for the test:
*         set bridge
*/
static GT_VOID prvTgfEgressVlanMirroringTest_BasicConfig
(
    GT_VOID
)
{
    utfGeneralStateMessageSave(0,"basic config bridge");
    prvTgfEgressVlanMirroringTest_BasicConfig_bridge();

    utfGeneralStateMessageSave(0,"basic config analyzerPort");
    prvTgfEgressVlanMirroringTest_BasicConfig_analyzerPort();

}

/**
* @internal prvTgfEgressVlanMirroringTest_generateTraffic function
* @endinternal
*
* @brief   generate traffic from ingress port.
*/
static GT_VOID prvTgfEgressVlanMirroringTest_generateTraffic
(
    GT_VOID
)
{
    GT_STATUS   rc;
    TGF_PACKET_STC      *packetInfoPtr_sender = &prvTgfPacketInfoUC1;/* the mac DA is unicast */
    GT_U32  senderPortIndex = PRV_TGF_SEND_PORT_IDX_CNS;
    GT_U16  vlanId = PRV_TGF_VLANID_CNS;

    if(forceFromCpuWithNonRecognizeTag == GT_TRUE)
    {
        /* use different 'etherType for vlan tagging' instead of 0x8100 */
        /* this will make our packet that has '0x8100' to be recognized as 'untagged'
          by the 'traffic generator' and also the CPSS will not override the ethertype
          of tag0 with the DSA tag */
        tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_NON_VALID_TAG_CNS);
    }

    /* this is the vlan tag in prvTgfPacketInfo */
    prvTgfPacketVlanTag0Part.vid = vlanId;

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr_sender,
        prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[senderPortIndex]);

    /* start transmitting */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[senderPortIndex]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[senderPortIndex]);


    if(forceFromCpuWithNonRecognizeTag == GT_TRUE)
    {
        /* restore default ethertype */
        tgfTrafficGeneratorEtherTypeForVlanTagSet(TGF_ETHERTYPE_8100_VLAN_TAG_CNS);
    }


}

/**
* @internal checkGotPackets function
* @endinternal
*
*/
static GT_VOID checkGotPackets
(
    IN GT_U32 egressPortIndex,
    IN GT_U32 numOfPackets
)
{
    GT_STATUS   rc;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;/* current counters of the port */

    rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[egressPortIndex], GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[egressPortIndex]);

    /* check that egress packets as expected */
    UTF_VERIFY_EQUAL0_STRING_MAC((prvTgfBurstCount * numOfPackets) , portCntrs.goodPktsSent.l[0], "get another egress packets values.");
}
/**
* @internal prvTgfEgressVlanMirroringTest_checkEgressPorts function
* @endinternal
*
* @brief   check egress ports of got the traffic as expected.
*/
static GT_VOID prvTgfEgressVlanMirroringTest_checkEgressPorts
(
    IN GT_BOOL      floodExpected,
    IN GT_U32       numOfMirrorCopies
)
{
    GT_U32  ii;
    GT_U32  senderPortIndex = PRV_TGF_SEND_PORT_IDX_CNS;
    GT_U32  trgPortIndex = PRV_TGF_RCV_PORT_IDX_CNS;

    GT_U32  analyzerEgressPortIndex = 4;

    /* first loop on the ports in vlan */
    for (ii = 0 ; ii < analyzerEgressPortIndex; ii ++)
    {
        if(ii == senderPortIndex)
        {
            continue;
        }

        if((trgPortIndex == ii && floodExpected == GT_FALSE) ||
           floodExpected == GT_TRUE)
        {
            PRV_UTF_LOG1_MAC("port[%d] expected to get the traffic \n",
                prvTgfPortsArray[ii]);

            /* port expected to get the traffic */
            checkGotPacket(ii);
        }
        else
        {
            PRV_UTF_LOG1_MAC("port[%d] not expected to get the traffic \n",
                prvTgfPortsArray[ii]);
            /* port not expected to get the traffic */
            checkGotNoPacket(ii);
        }
    }

    ii = analyzerEgressPortIndex;

    /* now check the amount of packets that the analyzer port got */
    if(numOfMirrorCopies == 0)
    {
        PRV_UTF_LOG1_MAC("analyzer port[%d] not expected to get the traffic \n",
            prvTgfPortsArray[ii]);
        /* port not expected to get the traffic */
        checkGotNoPacket(ii);
    }
    else
    {
        PRV_UTF_LOG2_MAC("analyzer port[%d]  expected to get the [%d] copies \n",
            prvTgfPortsArray[ii],numOfMirrorCopies);
        checkGotPackets(ii,numOfMirrorCopies);
    }


    /* reset port's counters */
    PRV_UTF_LOG0_MAC("reset ETH counters \n");
    prvTgfEthCountersReset(prvTgfDevNum);

}

/**
* @internal prvTgfEgressVlanMirroringTest_FdbMacEntryOnPortSet function
* @endinternal
*
* @brief   add FDB entry associated with egress port prvTgfPortsArray[PRV_TGF_RCV_PORT_IDX_CNS].
*/
static GT_VOID prvTgfEgressVlanMirroringTest_FdbMacEntryOnPortSet(GT_VOID)
{
    GT_STATUS   rc;
    GT_U16  vlanId = PRV_TGF_VLANID_CNS;
    GT_U32  trgPortIndex = PRV_TGF_RCV_PORT_IDX_CNS;

    /* AUTODOC: add FDB entry with MAC 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, VLAN 5, port [3] */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfPacketL2PartUC1.daMac,
                                          vlanId,
                                          prvTgfDevsArray[trgPortIndex],
                                          prvTgfPortsArray[trgPortIndex],
                                          GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevsArray[trgPortIndex]);
}

/**
* @internal prvTgfEgressVlanMirroringTest_checkSingleDestination function
* @endinternal
*
* @brief   check 2 cases of egress port 'enable/disable' the vlan egress mirroring.
*         check that the analyzer get/not get copy according to 'orig'
*         egress port 'enable/disable' the vlan egress mirroring
*/
static GT_VOID prvTgfEgressVlanMirroringTest_checkSingleDestination(GT_VOID)
{
    GT_BOOL  floodExpected;
    GT_U32   numOfMirrorCopies;
    GT_BOOL  enableVlanEgressMirror;
    GT_U32   portIndex = PRV_TGF_RCV_PORT_IDX_CNS;

    /* AUTODOC : add FDB entry associated with egress port prvTgfPortsArray[PRV_TGF_RCV_PORT_IDX_CNS] */
    prvTgfEgressVlanMirroringTest_FdbMacEntryOnPortSet();

    /* AUTODOC : currently egress port is not enabled from vlan egress mirror */
    prvTgfEgressVlanMirroringTest_generateTraffic();
    floodExpected = GT_FALSE;
    numOfMirrorCopies = 0;
    /* AUTODOC : check 0 copies on analyzer */
    prvTgfEgressVlanMirroringTest_checkEgressPorts(floodExpected,numOfMirrorCopies);

    /* AUTODOC : set egress port is to enabled vlan egress mirror */
    portIndex = PRV_TGF_RCV_PORT_IDX_CNS;
    enableVlanEgressMirror = GT_TRUE;
    prvTgfEgressVlanMirroringTest_vlanEgressMirrorEnableOnPort(portIndex,enableVlanEgressMirror);

    prvTgfEgressVlanMirroringTest_generateTraffic();
    floodExpected = GT_FALSE;
    numOfMirrorCopies = 1;
    /* AUTODOC : check 1 copy on analyzer */
    prvTgfEgressVlanMirroringTest_checkEgressPorts(floodExpected,numOfMirrorCopies);

    /* AUTODOC : set egress port to disabled vlan egress mirror */
    enableVlanEgressMirror = GT_FALSE;
    prvTgfEgressVlanMirroringTest_vlanEgressMirrorEnableOnPort(portIndex,enableVlanEgressMirror);

    prvTgfEgressVlanMirroringTest_generateTraffic();
    floodExpected = GT_FALSE;
    numOfMirrorCopies = 0;
    /* AUTODOC : check 0 copies on analyzer */
    prvTgfEgressVlanMirroringTest_checkEgressPorts(floodExpected,numOfMirrorCopies);
}


/**
* @internal prvTgfEgressVlanMirroringTest_configRestore_bridge function
* @endinternal
*
* @brief   restore bridge
*/
static GT_VOID prvTgfEgressVlanMirroringTest_configRestore_bridge
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U16  vlanId = PRV_TGF_VLANID_CNS;

    /* AUTODOC : enable vlan egress mirroring. */
    prvTgfBrgVlanMirrorToTxAnalyzerSet(vlanId,GT_TRUE,VLAN_EGRESS_ANALYZER_INDEX_CNS);

    /* AUTODOC: invalidate VLAN 5 */
    rc = prvTgfBrgDefVlanEntryInvalidate(vlanId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 vlanId);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

}

/**
* @internal prvTgfEgressVlanMirroringTest_configRestore_analyzerPort function
* @endinternal
*
* @brief   restore analyzer port
*/
static GT_VOID prvTgfEgressVlanMirroringTest_configRestore_analyzerPort
(
    GT_VOID
)
{
    GT_STATUS   rc;
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC  analyzerInterfaceInfo;

    GT_U32  analyzerIndex = VLAN_EGRESS_ANALYZER_INDEX_CNS;
    GT_U32  ii;

    analyzerInterfaceInfo.interface.type = CPSS_INTERFACE_PORT_E;
    analyzerInterfaceInfo.interface.devPort.hwDevNum = prvTgfDevNum;
    analyzerInterfaceInfo.interface.devPort.portNum = 0;

    /* AUTODOC: set analyzer port for analyzer */
    prvTgfMirrorAnalyzerInterfaceSet(analyzerIndex , &analyzerInterfaceInfo);

    /*  restore analyzer forwarding mode. */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(testingMirrorMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorToAnalyzerForwardingModeSet: %d",
                                 testingMirrorMode);

    /* AUTODOC : set all 5 ports as 'disable' the vlan egress mirroring */
    for(ii = 0 ; ii < prvTgfPortsNum ; ii++)
    {
        prvTgfEgressVlanMirroringTest_vlanEgressMirrorEnableOnPort(ii, GT_FALSE);
    }
}

/**
* @internal prvTgfEgressVlanMirroringTest_configRestore function
* @endinternal
*
* @brief   restore all configurations
*/
static GT_VOID prvTgfEgressVlanMirroringTest_configRestore
(
    GT_VOID
)
{
    utfGeneralStateMessageSave(0,"restore config");
    prvTgfEgressVlanMirroringTest_configRestore_bridge();

    utfGeneralStateMessageSave(0,"restore config analyzerPort");
    prvTgfEgressVlanMirroringTest_configRestore_analyzerPort();
}

/**
* @internal prvTgfEgressVlanMirroringTest function
* @endinternal
*
* @brief   Test eVlan based egress mirroring
*         two cases:
*         1. UC traffic
*         2. Multi-destination traffic e.g. flooding (several ports send mirror replications)
*/
GT_VOID prvTgfEgressVlanMirroringTest
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32  ii;
    GT_U32   origPort4;
    GT_U8    origDev4,origNumPorts;
    GT_BOOL      floodExpected;
    GT_U32       numOfMirrorCopies;
    GT_BOOL enableVlanEgressMirror; /*enable/disable  vlan egress mirror*/
    GT_U32  analyzerEgressPortIndex = 4;
    GT_U32  analyzerPort;
    GT_BOOL storedEtherTypeIgnore = tgfTrafficGeneratorEtherTypeIgnoreGet();

    /* AUTODOC : this test using 5 physical ports (not only 4)
       ingress traffic from index prvTgfPortsArray[1] , egress to index prvTgfPortsArray[3].
       so analyzer can NOT be index 1,3.

       for 3 analyzers we need another port (total 5 ports in test !)
     */

     origPort4      = prvTgfPortsArray[analyzerEgressPortIndex]   ;
     origDev4       = prvTgfDevsArray[analyzerEgressPortIndex]    ;
     origNumPorts   = prvTgfPortsNum        ;

     prvTgfPortsNum = 5;
     if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devType == CPSS_98DX8332_Z0_CNS)
     {
         analyzerPort = 70;
     }
     else
     {
         analyzerPort = PHY_PORT_4_CNS;
     }

    if(MUST_USE_ORIG_PORTS_MAC(prvTgfDevNum))
    {
        /* do not modify the 12 ports in prvTgfPortsArray[] !!! */
    }
    else
    {
         prvTgfPortsArray[analyzerEgressPortIndex] = analyzerPort;
         prvTgfDevsArray[analyzerEgressPortIndex]  = prvTgfDevsArray[3];
    }

     {
         GT_U32 i;
         for (i = 0; (i < 4); i++)
         {
             if (prvTgfPortsArray[i] == prvTgfPortsArray[4])
             {
                 prvTgfPortsArray[4]    = origPort4     ;
                 prvTgfDevsArray[4]     = origDev4      ;
                 prvTgfPortsNum         = origNumPorts  ;
                 UTF_VERIFY_EQUAL0_STRING_MAC(
                     GT_OK, GT_FAIL,
                     "Test not compatible wit given list of ports");
                 return;
             }
         }
     }

    /* reset port's counters */
    PRV_UTF_LOG0_MAC("reset ETH counters , and force link up on the ports \n");
    prvTgfEthCountersReset(prvTgfDevNum);

    utfGeneralStateMessageSave(0,"basic config");
    prvTgfEgressVlanMirroringTest_BasicConfig();

    /* AUTODOC : send flood in the vlan and check that analyzer port NOT get it */
    utfGeneralStateMessageSave(0,"send flood in the vlan and check that analyzer port NOT get it");
    prvTgfEgressVlanMirroringTest_generateTraffic();
    floodExpected = GT_TRUE;
    numOfMirrorCopies = 0;
    prvTgfEgressVlanMirroringTest_checkEgressPorts(floodExpected,numOfMirrorCopies);

    /* AUTODOC : check 'single destination' packet that trigger vlan egress mirroring */
    /* AUTODOC : check 2 cases of egress port 'enable/disable' the vlan egress mirroring.*/
    /* AUTODOC : check that the analyzer get/not get copy according to 'orig'
                 egress port 'enable/disable' the vlan egress mirroring */
    utfGeneralStateMessageSave(0,"check 'single destination' packet that trigger vlan egress mirroring");
    prvTgfEgressVlanMirroringTest_checkSingleDestination();

    /* AUTODOC : flush FDB (to remove the FDB entry) */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC : set all 4 ports in vlan (exclude the analyzer) as 'enable' the vlan egress mirroring */
    enableVlanEgressMirror = GT_TRUE;
    for(ii = 0 ; ii < analyzerEgressPortIndex; ii++)
    {
        prvTgfEgressVlanMirroringTest_vlanEgressMirrorEnableOnPort(
            ii,enableVlanEgressMirror);
    }

    enableVlanEgressMirror = GT_FALSE;

    /* AUTODOC : disable the vlan egress mirror on the sender port too. */
    utfGeneralStateMessageSave(0,"disable the vlan egress mirror on the sender port too.");
    ii = PRV_TGF_SEND_PORT_IDX_CNS;
    prvTgfEgressVlanMirroringTest_vlanEgressMirrorEnableOnPort(
        ii,enableVlanEgressMirror);

    /* AUTODOC : send flood in the vlan and check that analyzer port get one copy. */
    /* AUTODOC : NOTE: although 3 egress ports allow vlan egress mirroring still we
                expect only one copy because device not support more copies per
                'ingress' packet */
    utfGeneralStateMessageSave(0,"send flood in the vlan and check that analyzer port get one copy.");
    prvTgfEgressVlanMirroringTest_generateTraffic();
    floodExpected = GT_TRUE;
    numOfMirrorCopies = 3;
    prvTgfEgressVlanMirroringTest_checkEgressPorts(floodExpected,numOfMirrorCopies);

    /* AUTODOC : set one by one the 3 egress ports as 'disable' the vlan egress mirroring
                so we left with 2 and then 1 and then 0 egress ports that allow the vlan egress mirroring */
    /* AUTODOC : send flood in the vlan and check that analyzer port get one copy
                when still one of the egress ports is enabled from the mirroring.
                and NO copies when no egress port enabled for the vlan egress mirror. */

    for(ii = 0 ; ii < analyzerEgressPortIndex; ii++)
    {
        if(ii == PRV_TGF_SEND_PORT_IDX_CNS)
        {
            /* we not care about the sender */
            continue;
        }

        utfGeneralStateMessageSave(0,"send flood in the vlan and check that analyzer port get one copy. egress port index[%d]",ii);

        prvTgfEgressVlanMirroringTest_vlanEgressMirrorEnableOnPort(
            ii,enableVlanEgressMirror);

        prvTgfEgressVlanMirroringTest_generateTraffic();
        floodExpected = GT_TRUE;

        numOfMirrorCopies--;
        prvTgfEgressVlanMirroringTest_checkEgressPorts(floodExpected,numOfMirrorCopies);
    }
    enableVlanEgressMirror = GT_TRUE;
    /* AUTODOC : enable the vlan egress mirror only on the sender port. */
    utfGeneralStateMessageSave(0,"enable the vlan egress mirror only on the sender port.");
    ii = PRV_TGF_SEND_PORT_IDX_CNS;
    prvTgfEgressVlanMirroringTest_vlanEgressMirrorEnableOnPort(
        ii,enableVlanEgressMirror);

    prvTgfEgressVlanMirroringTest_generateTraffic();
    floodExpected = GT_TRUE;
    numOfMirrorCopies = 0;
    prvTgfEgressVlanMirroringTest_checkEgressPorts(floodExpected,numOfMirrorCopies);

    /* AUTODOC : enable the vlan egress mirror only on the sender port.
        expect the 'from cpu' to catch it and send copy to analyzer */
    utfGeneralStateMessageSave(0,"enable the vlan egress mirror only on the sender port. \n"
        "expect the 'from cpu' to catch it and send copy to analyzer \n");
    forceFromCpuWithNonRecognizeTag = GT_FALSE;
    tgfTrafficGeneratorEtherTypeIgnoreSet(GT_FALSE);
    prvTgfEgressVlanMirroringTest_generateTraffic();
    tgfTrafficGeneratorEtherTypeIgnoreSet(storedEtherTypeIgnore);
    floodExpected = GT_TRUE;
    numOfMirrorCopies = 1;
    prvTgfEgressVlanMirroringTest_checkEgressPorts(floodExpected,numOfMirrorCopies);

    /* restore value */
    forceFromCpuWithNonRecognizeTag = GT_TRUE;

    /* AUTODOC : enable the vlan egress mirror on the analyzer port. and on one of the ports in the vlan.
        check that there is no storming */
    utfGeneralStateMessageSave(0,"enable the vlan egress mirror on the analyzer port. and on one of the ports in the vlan. \n"
        "check that there is no storming \n");

    enableVlanEgressMirror = GT_TRUE;
    ii = analyzerEgressPortIndex;/* the analyzer port */
    prvTgfEgressVlanMirroringTest_vlanEgressMirrorEnableOnPort(
        ii,enableVlanEgressMirror);

    ii = PRV_TGF_RCV_PORT_IDX_CNS;/* one of the ports that get flood */
    prvTgfEgressVlanMirroringTest_vlanEgressMirrorEnableOnPort(
        ii,enableVlanEgressMirror);

    prvTgfEgressVlanMirroringTest_generateTraffic();
    floodExpected = GT_TRUE;
    numOfMirrorCopies = 1;
    prvTgfEgressVlanMirroringTest_checkEgressPorts(floodExpected,numOfMirrorCopies);


    PRV_UTF_LOG0_MAC("restore config \n");
    utfGeneralStateMessageSave(0,"restore config");
    prvTgfEgressVlanMirroringTest_configRestore();

    /* restore the ports array */
    prvTgfPortsArray[4] = origPort4      ;
    prvTgfDevsArray[4]  = origDev4       ;
    prvTgfPortsNum      = origNumPorts   ;

    forceFromCpuWithNonRecognizeTag = GT_TRUE;
}


static void stpStateSet(
    IN CPSS_STP_STATE_ENT stpStateVidx ,
    IN CPSS_STP_STATE_ENT stpStateNotInVidx)
{
    GT_STATUS   rc;
    GT_U32  portIter;
    CPSS_STP_STATE_ENT  stpState;
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            continue;
        }

        if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&vidxMembersPortBitmap,prvTgfPortsArray[portIter]))
        {
            stpState = stpStateNotInVidx;
        }
        else
        {
            stpState = stpStateVidx;
        }

        rc = prvTgfBrgStpPortStateSet(prvTgfDevNum, prvTgfPortsArray[portIter],
                                      0, stpState);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgStpPortStateSet: %d port %d state %d",
            prvTgfDevNum,
            prvTgfPortsArray[portIter],
            stpState);
    }
}

/**
* @internal prvTgfIngressMirrorAnalyzerIsEportIsVidxTest_config function
* @endinternal
*
* @brief   config for prvTgfIngressMirrorAnalyzerIsEportIsVidxTest
*/
static GT_VOID prvTgfIngressMirrorAnalyzerIsEportIsVidxTest_config
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32  analyzerIndex = PRV_TGF_MIRROR_GLOB_ANALYZER_IDX_CNS;
    GT_U32  analyzerDevNum = prvTgfDevNum;
    GT_U32  analyzerPort = TARGET_EPORT_NUM_CNS;
    CPSS_INTERFACE_INFO_STC physicalInfo;/* physical port info for the analyzer in eArch */
    GT_U16  vidx = PRV_TGF_MIRROR_ANALYZER_VIDX_CNS;
    GT_U32  ii;
    GT_U32  totalMembers;/* total num of vidx members*/
    GT_U16  myVid = PRV_TGF_VLANID_CNS;


    /*******************************************/
    /* AUTODOC: set analyzer mode : END_TO_END */
    /*******************************************/
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorToAnalyzerForwardingModeSet: %d",
                                 PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E);

    /**************************/
    /* AUTODOC: set the analyzer index */
    /**************************/
    prvMirrorAnalyzerInterfaceSet(analyzerIndex,analyzerDevNum,analyzerPort);

    /*************************************************************************/
    /* AUTODOC: map the E2Phy of the analyzer eport : from eport to VIDX !!! */
    /*************************************************************************/
    physicalInfo.type = CPSS_INTERFACE_VIDX_E;
    physicalInfo.vidx = vidx;
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                analyzerPort,
                &physicalInfo);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: dev %d, vidx %d",
                                 prvTgfDevNum, vidx);

    /********************************************************/
    /* AUTODOC: set the ingress port as subject to ingress mirroring */
    /********************************************************/
    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE, analyzerIndex);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet: %d %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);

    /************************************/
    /* AUTODOC: set 2 ports in the VIDX */
    /************************************/
    totalMembers = 0;
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&vidxMembersPortBitmap);
    for (ii = 0; ii < prvTgfPortsNum; ii++)
    {
        if (ii == PRV_TGF_SEND_PORT_IDX_CNS)
        {
            continue;
        }

        CPSS_PORTS_BMP_PORT_SET_MAC(&vidxMembersPortBitmap, prvTgfPortsArray[ii]);

        totalMembers++;

        if(totalMembers == 2)
        {
            break;
        }
    }

    /**********************************/
    /* AUTODOC:  write the vidx entry */
    /**********************************/
    rc = prvTgfBrgMcEntryWrite(prvTgfDevNum, vidx, &vidxMembersPortBitmap);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgMcEntryWrite: %d , vidx %d",
                                 prvTgfDevNum,vidx);

    /***********************************************************************/
    /* AUTODOC: define vlan that includes all the ports (and vidx members) */
    /***********************************************************************/
    rc = prvTgfBrgDefVlanEntryWrite(myVid);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: vid %d",
                                 myVid);



}

/**
* @internal prvTgfIngressMirrorAnalyzerIsEportIsVidxTest_deconfig function
* @endinternal
*
* @brief   de-config for prvTgfIngressMirrorAnalyzerIsEportIsVidxTest
*/
static GT_VOID prvTgfIngressMirrorAnalyzerIsEportIsVidxTest_deconfig
(
    GT_VOID
)
{
    GT_STATUS   rc;
    CPSS_PORTS_BMP_STC      portBitmap;
    GT_U32  analyzerIndex = PRV_TGF_MIRROR_GLOB_ANALYZER_IDX_CNS;
    GT_U32  analyzerDevNum = prvTgfDevNum;
    CPSS_INTERFACE_INFO_STC physicalInfo;/* physical port info for the analyzer in eArch */
    GT_U32  analyzerPort = 0;/*HW default value*/
    GT_U16  vidx = PRV_TGF_MIRROR_ANALYZER_VIDX_CNS;
    GT_U16  myVid = PRV_TGF_VLANID_CNS;

    /* Disable Rx mirroring on sender port */
    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet: %d %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);

    /* restore the 2 vidx ports (of analyzer) as 'STP DISABLED' */
    stpStateSet(CPSS_STP_DISABLED_E/*vidx*/,CPSS_STP_DISABLED_E/*not vidx*/);

    /*******************************************/
    /* restore analyzer mode : HOP_BY_HOP */
    /*******************************************/
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorToAnalyzerForwardingModeSet: %d",
                                 PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E);

    /* restore the analyzer index */
    prvMirrorAnalyzerInterfaceSet(analyzerIndex,analyzerDevNum,analyzerPort);

    /* invalidate the vidx entry */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portBitmap);
    rc = prvTgfBrgMcEntryWrite(prvTgfDevNum, vidx, &portBitmap);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgMcEntryWrite: %d , vidx %d",
                                 prvTgfDevNum,vidx);

    /* invalidate the vlan */
    rc = prvTgfBrgVlanEntryInvalidate(myVid);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: vid %d",
                                 myVid);

    /* restore the E2PHY */
    physicalInfo.type = CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.hwDevNum = analyzerDevNum;
    physicalInfo.devPort.portNum = 0;/*HW defaults*/
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                analyzerPort,
                &physicalInfo);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: dev %d, vidx %d",
                                 prvTgfDevNum, vidx);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* restore DSA tag testing mode */
    prvTgfMirrorConfigParamsSet(GT_FALSE,TGF_DSA_2_WORD_TYPE_E,PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E);

}

/* send traffic */
static void sendTraffic(void)
{
    GT_STATUS   rc;
    GT_U32  portIter;
    TGF_PACKET_STC *packetInfoPtr;

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    if(testingDsaTag == GT_FALSE)
    {
        packetInfoPtr = &prvTgfPacketInfoUC1;
    }
    else
    {
        prvTgfPacketDsaTagPart_toAnalyzer_vidx.devPort.hwDevNum = DUMMY_REMOTE_DEV_NUM_CNS;
        prvTgfPacketDsaTagPart_toAnalyzer_vidx.devPort.portNum = 0xa5;
        prvTgfPacketDsaTagPart_toAnalyzer_vidx.devPort.ePort = DUMMY_TARGET_EPORT_NUM_CNS - 1;/*dummy eport*/

        /* set 'union' field */
        prvTgfPacketDsaTagPart_toAnalyzer_vidx.extDestInfo.multiDest.analyzerEvidx = PRV_TGF_MIRROR_ANALYZER_VIDX_CNS;

        /* copy 'union' field */
        prvTgfPacketDsaTagPart_vidx.dsaInfo.toAnalyzer = prvTgfPacketDsaTagPart_toAnalyzer_vidx;
        prvTgfPacketDsaTagPart_vidx.commonParams.vid   = PRV_TGF_VLANID_CNS;

        packetInfoPtr = &origIngressPacket_vidxInfo;
    }

    /* AUTODOC: send UC packet from ingress port with: */
    /* AUTODOC: DA=00:11:22:33:44:55, SA=00:00:00:00:22:22, VID=5 */
    prvTgfMirrorTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], packetInfoPtr);


}

/* verify egress ports MIB counters */
static void verifyTraffic(
    IN GT_U32 numTxPacketsArr[/*prvTgfPortsNum*/],
    IN GT_BOOL checkSendPortTx
)
{
    GT_STATUS   rc;
    GT_U32  portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;

    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            if (checkSendPortTx == GT_TRUE)
            {
                /* check Rx counters */
                UTF_VERIFY_EQUAL0_STRING_MAC(numTxPacketsArr[portIter], portCntrs.goodPktsRcv.l[0],
                                             "get another goodPktsRcv counter than expected");
            }
            else
            {
                /* check Rx counters */
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                             "get another goodPktsRcv counter than expected");
                continue;
            }
        }

        /* check Tx counters */
        UTF_VERIFY_EQUAL1_STRING_MAC(numTxPacketsArr[portIter], portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected , on port[%d]",
                                     prvTgfPortsArray[portIter]);
    }
}

/* send traffic to cascade port with FORWARD DSA tag */
static void sendFrwrdCscdTraffic(void)
{
    GT_STATUS   rc;
    GT_U32  portIter;
    TGF_PACKET_STC *packetInfoPtr;

    /* AUTODOC: reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: complete an initialization of packets structures  */
    prvTgfPacketDsaTagPart_forward.dstInterface.devPort.hwDevNum  = prvTgfDevNum;
    prvTgfPacketDsaTagPart_forward.isTrgPhyPortValid = GT_FALSE;
    prvTgfPacketDsaTagPart_forward.dstEport = TRG_EPORT_1_CNS;

    /* set union fields */
    prvTgfPacketDsaTagPart_forward.source.portNum = 5;
    prvTgfPacketDsaTagPart_forward.origSrcPhy.portNum = 19;

    /* AUTODOC: copy 'union' field */
    prvTgfPacketForwardDsaTagPart_vidx.dsaInfo.forward = prvTgfPacketDsaTagPart_forward;

    /* AUTODOC: set send packet - FORWARD DSA tag */
    origIngressPacket_vidxInfo.partsArray[1].partPtr = &prvTgfPacketForwardDsaTagPart_vidx;

    packetInfoPtr = &origIngressPacket_vidxInfo;

    /* AUTODOC: send UC packet from ingress port with: */
    /* AUTODOC: DA=00:11:22:33:44:55, SA=00:00:00:00:22:22, VID=5 */
    prvTgfMirrorTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], packetInfoPtr);

    /* AUTODOC:  set receive packet - TO_ANALYZER DSA tag */
    prvTgfPacketDsaTagPart_toAnalyzer1.devPort.hwDevNum = prvTgfDevNum;
    prvTgfPacketDsaTagPart_toAnalyzer1.devPort.portNum = prvTgfPortsArray[PRV_TGF_RCV_PORT_IDX_CNS];
    prvTgfPacketDsaTagPart_toAnalyzer1.devPort.ePort = TRG_EPORT_1_CNS;
    prvTgfPacketDsaTagPart_vidx.dsaInfo.toAnalyzer = prvTgfPacketDsaTagPart_toAnalyzer1;
    prvTgfPacketDsaTagPart_vidx.commonParams.vid   = PRV_TGF_VLANID_CNS;
    origIngressPacket_vidxInfo.partsArray[1].partPtr = &prvTgfPacketDsaTagPart_vidx;
}

static GT_U32   prvTgfIngressMirrorAnalyzerIsEportIsVidxTest_traffic_debug =  0;
/* debug function to allow to check only the 'STP BLOCKING' */
GT_VOID prvTgfIngressMirrorAnalyzerIsEportIsVidxTest_traffic_debugSet(
    IN GT_U32  debug_case)
{
    prvTgfIngressMirrorAnalyzerIsEportIsVidxTest_traffic_debug = debug_case;
}

static void   setExpectedCounters
(
    IN GT_BOOL  stpBlockingVidx,
    IN GT_BOOL  stpBlockingNonVidx,
    INOUT GT_U32 numTxPacketsArr[PRV_TGF_MIRR_TEST_TX_PACKET_ARRAY_SIZE_CNS]
)
{
    GT_U32  portIter;

    cpssOsMemSet(numTxPacketsArr,0,(sizeof(GT_U32) * PRV_TGF_MIRR_TEST_TX_PACKET_ARRAY_SIZE_CNS));
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&vidxMembersPortBitmap,prvTgfPortsArray[portIter]))
        {
            if(stpBlockingNonVidx == GT_TRUE ||
               testingDsaTag == GT_TRUE)/* on DSA we not have 'Regular flood' */
            {
                numTxPacketsArr[portIter] = 0;
            }
            else
            {
                numTxPacketsArr[portIter] = prvTgfBurstCount;
            }
            continue;
        }

        numTxPacketsArr[portIter] = prvTgfBurstCount;/* single replication from 'to anlyzer'  */

        if(stpBlockingVidx == GT_FALSE &&
            testingDsaTag == GT_FALSE)/* on DSA we not have 'Regular flood' */
        {
            numTxPacketsArr[portIter] += prvTgfBurstCount;/* addition packet of original traffic */
        }
    }
}

static GT_VOID setEgressPortCascade
(
    IN GT_PHYSICAL_PORT_NUM     egressPort,
    IN GT_BOOL                  enable
)
{
    GT_STATUS rc;

    /* AUTODOC: set the egress port to be DSA tagged */
    rc = tgfTrafficGeneratorEgressCscdPortEnableSet(
        prvTgfDevNum, egressPort, enable,
        enable ? CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E : CPSS_CSCD_PORT_NETWORK_E);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                         "tgfTrafficGeneratorEgressCscdPortEnableSet: [%d] \n");
}

static void sendTrafficWithCaptureEgressPort(
    IN GT_U32   egressPortToCapture,
    IN GT_BOOL  ingressAnalyzerTest
)
{
    GT_STATUS   rc;
    TGF_CAPTURE_MODE_ENT      mode = TGF_CAPTURE_MODE_PCL_E;
    CPSS_INTERFACE_INFO_STC       portInterface;
    TGF_PACKET_STC *packetInfoPtr;
    GT_U32  actualCapturedNumOfPackets;/*actual number of packet captured from the interface*/

    portInterface.type             = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum  = egressPortToCapture;

    /* Start capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, mode, GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum, GT_TRUE);

    /* enable the cascade port on the egress port */
    setEgressPortCascade(egressPortToCapture,GT_TRUE);

    /* GENERATE TRAFFIC: */
    if (ingressAnalyzerTest == GT_TRUE)
    {
        sendTraffic();
    }
    else
    {
        sendFrwrdCscdTraffic();
    }
    /* disable the cascade port on the egress port */
    setEgressPortCascade(egressPortToCapture,GT_FALSE);


    /* Stop capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, mode, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum, GT_FALSE);

    /* build the expected packet parts */

    /* set runtime values */
    if(ingressAnalyzerTest == GT_FALSE)/* egress analyzer */
    {
        static TGF_DSA_DSA_TO_ANALYZER_STC prvTgfPacketDsaTagPart_toAnalyzer_vidx_tx;

        /* copy the info ... to not change the 'rxSniffer' value */
        cpssOsMemCpy(&prvTgfPacketDsaTagPart_toAnalyzer_vidx_tx,
               &prvTgfPacketDsaTagPart_toAnalyzer_vidx,
               sizeof(prvTgfPacketDsaTagPart_toAnalyzer_vidx));

        prvTgfPacketDsaTagPart_toAnalyzer_vidx_tx.rxSniffer = GT_FALSE;
        /* target information */
        prvTgfPacketDsaTagPart_toAnalyzer_vidx_tx.devPort.hwDevNum = prvTgfDevNum;
        prvTgfPacketDsaTagPart_toAnalyzer_vidx_tx.devPort.portNum = prvTgfPortsArray[PRV_TGF_RCV_PORT_IDX_CNS];
        prvTgfPacketDsaTagPart_toAnalyzer_vidx_tx.devPort.ePort = TRG_EPORT_1_CNS;
        /* set 'union' field */
        prvTgfPacketDsaTagPart_toAnalyzer_vidx_tx.extDestInfo.multiDest.analyzerEvidx = PRV_TGF_MIRROR_ANALYZER_VIDX_CNS;
        /* copy 'union' field */
        prvTgfPacketDsaTagPart_vidx.dsaInfo.toAnalyzer = prvTgfPacketDsaTagPart_toAnalyzer_vidx_tx;
        prvTgfPacketDsaTagPart_vidx.commonParams.vid = EGRESS_TRANS_VLAN_ID;/* after translation*/

        packetInfoPtr = &dummyExpectedPacket_vidxInfo;
    }
    else
    /* source port information */
    if(testingDsaTag == GT_FALSE)
    {
        prvTgfPacketDsaTagPart_toAnalyzer_vidx.devPort.hwDevNum = prvTgfDevNum;
        prvTgfPacketDsaTagPart_toAnalyzer_vidx.devPort.portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
        prvTgfPacketDsaTagPart_toAnalyzer_vidx.devPort.ePort = prvTgfPacketDsaTagPart_toAnalyzer_vidx.devPort.portNum;
        /* set 'union' field */
        prvTgfPacketDsaTagPart_toAnalyzer_vidx.extDestInfo.multiDest.analyzerEvidx = PRV_TGF_MIRROR_ANALYZER_VIDX_CNS;
        /* copy 'union' field */
        prvTgfPacketDsaTagPart_vidx.dsaInfo.toAnalyzer = prvTgfPacketDsaTagPart_toAnalyzer_vidx;
        prvTgfPacketDsaTagPart_vidx.commonParams.vid   = PRV_TGF_VLANID_CNS;

        packetInfoPtr = &dummyExpectedPacket_vidxInfo;
    }
    else
    {
        /* we already build the sender packet ... it should egress 'as is' */
        packetInfoPtr = &origIngressPacket_vidxInfo;
    }

    /* check that the port got proper packet */
    rc = tgfTrafficGeneratorPortTxEthCaptureCompare(
            &portInterface,
            packetInfoPtr,
            prvTgfBurstCount,/*numOfPackets*/
            0/*vfdNum*/,
            NULL /*vfdArray*/,
            NULL, /* bytesNum's skip list */
            0,    /* length of skip list */
            &actualCapturedNumOfPackets,
            NULL/*onFirstPacketNumTriggersBmpPtr*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureCompare: port %d",
        egressPortToCapture);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

}

static void fdbEntryOnVidxSet(void)
{
    GT_STATUS   rc;

    rc = prvTgfBrgDefFdbMacEntryOnVidxSet(prvTgfPacketL2PartUC1.daMac, PRV_TGF_VLANID_CNS,
                                          PRV_TGF_MIRROR_ANALYZER_VIDX_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnVidxSet");
}
/**
* @internal prvTgfIngressMirrorAnalyzerIsEportIsVidxTest_traffic function
* @endinternal
*
* @brief   send and check traffic for prvTgfIngressMirrorAnalyzerIsEportIsVidxTest
*/
static GT_VOID prvTgfIngressMirrorAnalyzerIsEportIsVidxTest_traffic
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32 numTxPacketsArr[PRV_TGF_MIRR_TEST_TX_PACKET_ARRAY_SIZE_CNS];
    GT_U32  portInVidxNotInVlanIndex = 2;
    GT_U32  portInVidxNotInVlan = prvTgfPortsArray[portInVidxNotInVlanIndex];
    GT_U32  portInVidxCascadeIndex = PRV_TGF_PORT_IDX_VIDX_CSCD_MEMBER;
    GT_U32  portInVidxCascade = prvTgfPortsArray[portInVidxCascadeIndex];
    GT_U32  tmpValue = 1;
    GT_U32  portIter;
    GT_U32  debug_case = 1;

    utfGeneralStateMessageSave(5,"STP not blocking - vidx");
    utfGeneralStateMessageSave(6,"STP blocking - non-vidx");

    setExpectedCounters(GT_FALSE/*STP not blocking - vidx*/,
                        GT_TRUE/*STP blocking - non-vidx*/ ,
                        numTxPacketsArr);
    /* AUTODOC: set the 2 vidx ports (of analyzer) as 'STP DISABLED' */
    stpStateSet(CPSS_STP_DISABLED_E/*vidx*/,CPSS_STP_BLCK_LSTN_E/*not vidx*/);

    utfGeneralStateMessageSave(11,"debug case: [%d]",debug_case);
    debug_case++;
    if(prvTgfIngressMirrorAnalyzerIsEportIsVidxTest_traffic_debug == 0 ||
       prvTgfIngressMirrorAnalyzerIsEportIsVidxTest_traffic_debug == (debug_case-1))
    {
        /* AUTODOC: GENERATE TRAFFIC: */
        sendTraffic();

        /* AUTODOC: verify traffic: egress only the analyzer ports */
        /* AUTODOC: because ingress traffic goes to vlan without members */
        verifyTraffic(numTxPacketsArr, GT_FALSE);
    }

    utfGeneralStateMessageSave(5,"STP blocking - vidx");
    utfGeneralStateMessageSave(6,"STP blocking - non-vidx");
    setExpectedCounters(GT_TRUE/*STP blocking - vidx*/,
                        GT_TRUE/*STP blocking - non-vidx*/,
                        numTxPacketsArr);

    /* AUTODOC: set the 2 vidx ports (of analyzer) as 'STP BLOCKING' */
    stpStateSet(CPSS_STP_BLCK_LSTN_E/*vidx*/,CPSS_STP_BLCK_LSTN_E/*not vidx*/);

    utfGeneralStateMessageSave(11,"debug case: [%d]",debug_case);
    debug_case++;
    if(prvTgfIngressMirrorAnalyzerIsEportIsVidxTest_traffic_debug == 0 ||
       prvTgfIngressMirrorAnalyzerIsEportIsVidxTest_traffic_debug == (debug_case-1))
    {
        /* AUTODOC: GENERATE TRAFFIC: */
        sendTraffic();

        /* AUTODOC: verify traffic not impact by STP blocking */
        verifyTraffic(numTxPacketsArr, GT_FALSE);
    }

    utfGeneralStateMessageSave(7,"remove one of the vidx members from the vlan");
    /* AUTODOC: remove one of the vidx members from the vlan */
    rc = prvTgfBrgVlanMemberRemove(
        prvTgfDevNum,PRV_TGF_VLANID_CNS,portInVidxNotInVlan);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: %d vlan %d port %d",
        prvTgfDevNum,
        PRV_TGF_VLANID_CNS,
        portInVidxNotInVlan);

    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /*AUTODOC: JIRA: EGF-1034: Fixed for sip6 , vlan filtering should not reflect in Analyzer Vidx. */
        utfGeneralStateMessageSave(8,"expect the packet NOT to be  filtered due to vlan filtering'");
    }
    else
    {
        utfGeneralStateMessageSave(8,"expect the packet to be filtered due to vlan filtering' on VIDX port that removed from vlan");
        /* AUTODOC: expect the packet to be filtered due to vlan filtering' on VIDX port that removed from vlan */
        tmpValue = numTxPacketsArr[portInVidxNotInVlanIndex];/* save the value */
        numTxPacketsArr[portInVidxNotInVlanIndex] = 0;
    }

    utfGeneralStateMessageSave(11,"debug case: [%d]",debug_case);
    debug_case++;
    if(prvTgfIngressMirrorAnalyzerIsEportIsVidxTest_traffic_debug == 0 ||
       prvTgfIngressMirrorAnalyzerIsEportIsVidxTest_traffic_debug == (debug_case-1))
    {
        /* AUTODOC: GENERATE TRAFFIC: */
        /* send traffic and set capture on the vidx cascade port */
        sendTrafficWithCaptureEgressPort(portInVidxCascade, GT_TRUE);

        /* AUTODOC: verify traffic subject to vlan filtering */
        verifyTraffic(numTxPacketsArr, GT_FALSE);
    }


/************************************************************/

    utfGeneralStateMessageSave(9,"set the port that was removed from the vlan to be without vlan filtering");
    /* AUTODOC: set the port that was removed from the vlan to be without vlan filtering */
    rc = prvTgfBrgEgrFltVlanPortFilteringEnableSet(prvTgfDevNum,portInVidxNotInVlan,GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgEgrFltVlanPortFilteringEnableSet: %d port %d",
        prvTgfDevNum,
        portInVidxNotInVlan);
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /*AUTODOC: JIRA: EGF-1034: Fixed for sip6 , vlan filtering should not reflect in Analyzer Vidx. */
        utfGeneralStateMessageSave(8,"expect the packet NOT be filtered");
    }
    else
    {
        utfGeneralStateMessageSave(8,"expect the packet still be filtered because 'disable egress vlan filter' not apply to 'to_analyzer'");
        /* AUTODOC: expect the packet still be filtered because 'disable egress vlan filter' not apply to 'to_analyzer' */
        numTxPacketsArr[portInVidxNotInVlanIndex] = 0;/*not packet expected even when egress vlan filter disabled */
    }

    utfGeneralStateMessageSave(11,"debug case: [%d]",debug_case);
    debug_case++;
    if(prvTgfIngressMirrorAnalyzerIsEportIsVidxTest_traffic_debug == 0 ||
       prvTgfIngressMirrorAnalyzerIsEportIsVidxTest_traffic_debug == (debug_case-1))
    {
        /* AUTODOC: GENERATE TRAFFIC: */
        /* send traffic */
        sendTraffic();

        /* AUTODOC: verify traffic subject to vlan filtering */
        verifyTraffic(numTxPacketsArr, GT_FALSE);
    }

    if(testingDsaTag == GT_FALSE)
    {
/************************************************************/

        utfGeneralStateMessageSave(10,"set the FDB to point the original traffic also to our VIDX ..."
           "lets check vlan egress filtering on 'forward' traffic");
        /* AUTODOC: set the FDB to point the original traffic also to our VIDX ...
           lets check vlan egress filtering on 'forward' traffic */
        fdbEntryOnVidxSet();

        utfGeneralStateMessageSave(5,"STP not blocking - vidx");
        utfGeneralStateMessageSave(6,"STP not blocking - non-vidx");
        setExpectedCounters(GT_FALSE/*STP not blocking - vidx*/,
                            GT_FALSE/*STP not blocking - non-vidx*/ ,
                            numTxPacketsArr);
        /* AUTODOC: set all ports as 'STP DISABLED' */
        stpStateSet(CPSS_STP_DISABLED_E/*vidx*/,CPSS_STP_DISABLED_E/*not vidx*/);
        if (!PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
        {
            numTxPacketsArr[portInVidxNotInVlanIndex] = tmpValue;/*reuse the saved value*/
        }
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            if (portIter == portInVidxNotInVlanIndex ||
                portIter == portInVidxCascadeIndex)
            {
                /* the 2 ports in the VIDX */
                continue;
            }
            else
            {
                /* we not expect traffic from it */
                numTxPacketsArr[portIter] = 0;
            }
        }

        utfGeneralStateMessageSave(8,"verify traffic subject to vlan filtering (only on to_analyzer traffic)");

        utfGeneralStateMessageSave(11,"debug case: [%d]",debug_case);
        debug_case++;
        if(prvTgfIngressMirrorAnalyzerIsEportIsVidxTest_traffic_debug == 0 ||
           prvTgfIngressMirrorAnalyzerIsEportIsVidxTest_traffic_debug == (debug_case-1))
        {
            /* AUTODOC: GENERATE TRAFFIC: */
            /* send traffic */
            sendTraffic();
            /* AUTODOC: verify traffic subject to vlan filtering (only on to_analyzer traffic) */
            verifyTraffic(numTxPacketsArr, GT_FALSE);
        }
/************************************************************/
    }
    else
    {
        /* not testing next case , because not sending 'forword' copy  */
    }

    utfGeneralStateMessageSave(9,NULL);
    /* restore the egress vlan filtering on the egress port */
    rc = prvTgfBrgEgrFltVlanPortFilteringEnableSet(prvTgfDevNum,portInVidxNotInVlan,GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgEgrFltVlanPortFilteringEnableSet: %d port %d",
        prvTgfDevNum,
        portInVidxNotInVlan);
}

/**
* @internal prvTgfIngressMirrorAnalyzerIsEportIsVidxTest function
* @endinternal
*
* @brief   Test ingress analyzer that is EPort that is mapped by the E2Phy to physical VIDX.
*         With analyzer (egress ports) vidx members in STP blocking/forward
*/
GT_VOID prvTgfIngressMirrorAnalyzerIsEportIsVidxTest
(
    GT_VOID
)
{
    GT_STATUS rc;

    initVidxToUse();

    /* AUTODOC: test uses 5 ports. Save prvTgfPortsNum and prvTgfPortsArrays */
    prvTgfPortsNumSave = prvTgfPortsNum;
    cpssOsMemCpy(prvTgfPortsArraySave, prvTgfPortsArray,
                 sizeof(prvTgfPortsArraySave));

    /* AUTODOC: re-arrange prvTgfPortsArray to ensure some ports (will be
     * configured cascade) are not remote ports */
    if (UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum))
    {
        if (GT_FALSE == testingDsaTag)
        {
            rc = prvTgfDefPortsArrange(GT_FALSE,
                                       PRV_TGF_PORT_IDX_VIDX_CSCD_MEMBER,
                                       -1);
        }
        else
        {
            /* ingress port will be configured as cascade too */
            rc = prvTgfDefPortsArrange(GT_FALSE,
                                       PRV_TGF_PORT_IDX_VIDX_CSCD_MEMBER,
                                       PRV_TGF_SEND_PORT_IDX_CNS,
                                       -1);
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPortsArray rearrangement can't be done");
    }

    else if(MUST_USE_ORIG_PORTS_MAC(prvTgfDevNum))
    {
        /* do not modify the 12 ports in prvTgfPortsArray[] !!! */
    }
    else
    if(prvTgfNumOfPortGroups && usePortGroupsBmp == GT_FALSE && prvTgfDevPortForceNum)
    {
        prvTgfPortsArray[4] = 40;/* use port 40 and not 17 because 17 used as ring port */
    }
    else
    {
        prvTgfPortsArray[4] = (prvTgfDevicePortMode == PRV_TGF_DEVICE_PORT_MODE_XLG_E)? 16 : 17;
    }
    prvTgfPortsNum = 5;

    /* New ports added: Link up and reset counters */
    prvTgfEthCountersReset(prvTgfDevNum);

    prvTgfIngressMirrorAnalyzerIsEportIsVidxTest_config();

    prvTgfIngressMirrorAnalyzerIsEportIsVidxTest_traffic();

    prvTgfIngressMirrorAnalyzerIsEportIsVidxTest_deconfig();

    /* AUTODOC: restore prvTgfPortsNum and prvTgfPortsArrays */
    cpssOsMemCpy(prvTgfPortsArray, prvTgfPortsArraySave, sizeof(prvTgfPortsArraySave));
    prvTgfPortsNum = prvTgfPortsNumSave;
}

/**
* @internal prvTgfEgressMirrorAnalyzerIsEportIsVidxTest_config function
* @endinternal
*
* @brief   config for prvTgfEgressMirrorAnalyzerIsEportIsVidxTest
*/
static GT_VOID prvTgfEgressMirrorAnalyzerIsEportIsVidxTest_config
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32  analyzerIndex = PRV_TGF_MIRROR_GLOB_ANALYZER_IDX_CNS;
    GT_U32  analyzerDevNum = prvTgfDevNum;
    GT_U32  analyzerPort = TARGET_EPORT_NUM_CNS;
    CPSS_INTERFACE_INFO_STC physicalInfo;/* physical port info for the analyzer in eArch */
    GT_U16  vidx = PRV_TGF_MIRROR_ANALYZER_VIDX_CNS;
    GT_U32  ii;
    GT_U32  totalMembers;/* total num of vidx members*/
    GT_U16  myVid = PRV_TGF_VLANID_CNS;
    GT_U32  trgEport;

    /**************************/
    /* AUTODOC : set the e2Phy to set physical port for the target eports */
    /**************************/
    physicalInfo.type = CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.hwDevNum = prvTgfDevNum;
    physicalInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_RCV_PORT_IDX_CNS];

    trgEport = TRG_EPORT_1_CNS;
    /* set first eport */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                trgEport,
                &physicalInfo);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: %d, %d",
                                 prvTgfDevNum, trgEport);

    /*******************************************/
    /* AUTODOC: set analyzer mode : END_TO_END */
    /*******************************************/
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorToAnalyzerForwardingModeSet: %d",
                                 PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E);

    /**************************/
    /* AUTODOC: set the analyzer index */
    /**************************/
    prvMirrorAnalyzerInterfaceSet(analyzerIndex,analyzerDevNum,analyzerPort);

    /*************************************************************************/
    /* AUTODOC: map the E2Phy of the analyzer eport : from eport to VIDX !!! */
    /*************************************************************************/
    physicalInfo.type = CPSS_INTERFACE_VIDX_E;
    physicalInfo.vidx = vidx;
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                analyzerPort,
                &physicalInfo);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: dev %d, vidx %d",
                                 prvTgfDevNum, vidx);

    /********************************************************/
    /* AUTODOC: set the egress port as subject to egress mirroring */
    /********************************************************/
    rc = prvTgfMirrorTxPortSet(prvTgfPortsArray[PRV_TGF_RCV_PORT_IDX_CNS], GT_TRUE, analyzerIndex);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortSet: %d %d",
                                 prvTgfPortsArray[PRV_TGF_RCV_PORT_IDX_CNS], GT_TRUE);

    /************************************/
    /* AUTODOC: set 2 (plus 1 for egress capture) ports in the VIDX */
    /************************************/
    totalMembers = 0;
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&vidxMembersPortBitmap);
    for (ii = 0; ii < prvTgfPortsNum; ii++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&vidxMembersPortBitmap, prvTgfPortsArray[ii]);

        totalMembers++;

        if(totalMembers == 3)
        {
            break;
        }
    }

    /**********************************/
    /* AUTODOC:  write the vidx entry */
    /**********************************/
    rc = prvTgfBrgMcEntryWrite(prvTgfDevNum, vidx, &vidxMembersPortBitmap);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgMcEntryWrite: %d , vidx %d",
                                 prvTgfDevNum,vidx);

    /***********************************************************************/
    /* AUTODOC: define vlan that includes all the ports (and vidx members) */
    /***********************************************************************/
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(myVid,PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd: vid %d",
                                 myVid);


    /************************************************************/
    /* AUTODOC: set the egress VLAN translation on egress eport */
    /************************************************************/
    rc = prvTgfBrgVlanPortTranslationEnableSet(prvTgfDevNum,trgEport,
        CPSS_DIRECTION_EGRESS_E,
        PRV_TGF_BRG_VLAN_TRANSLATION_VID0_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortTranslationEnableSet: trgEport %d",
                                 trgEport);

    /* save default value for restore */
    rc = prvTgfBrgVlanTranslationEntryRead(prvTgfDevNum, myVid,
        CPSS_DIRECTION_EGRESS_E, &vlanTranslationEntryDef);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTranslationEntryRead: from vid %d",
                                 myVid);

    /* state the mapping from vid 5 to vid 300 */
    rc = prvTgfBrgVlanTranslationEntryWrite(prvTgfDevNum,myVid,
        CPSS_DIRECTION_EGRESS_E ,
        EGRESS_TRANS_VLAN_ID);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTranslationEntryWrite: from vid %d to vid %d",
                                 myVid,
                                 EGRESS_TRANS_VLAN_ID);


}

/**
* @internal prvTgfEgressMirrorAnalyzerIsEportIsVidxTest_traffic_AC3X function
* @endinternal
*
* @brief   send and check traffic for prvTgfEgressMirrorAnalyzerIsEportIsVidxTest
*           for egress port in 'PHY1690' that not support cascade.
*/
static GT_VOID prvTgfEgressMirrorAnalyzerIsEportIsVidxTest_traffic_AC3X
(
    GT_VOID
)
{
    GT_U32 numTxPacketsArr[5] = {1, 2, 1, 1, 0};

    /* check only 'egress ports' to get the 'amount' of packets (not checking 'capture') */

    /* AUTODOC: GENERATE TRAFFIC: */
    sendFrwrdCscdTraffic();
    /* AUTODOC: verify traffic subject to vlan filtering */
    verifyTraffic(numTxPacketsArr, GT_TRUE);
}

/**
* @internal prvTgfEgressMirrorAnalyzerIsEportIsVidxTest_traffic function
* @endinternal
*
* @brief   send and check traffic for prvTgfEgressMirrorAnalyzerIsEportIsVidxTest
*/
static GT_VOID prvTgfEgressMirrorAnalyzerIsEportIsVidxTest_traffic
(
    GT_VOID
)
{
    GT_U32  portInVidxCascadeIndex = 0;
    GT_U32  portInVidxCascade = prvTgfPortsArray[portInVidxCascadeIndex];

    /* AUTODOC: GENERATE TRAFFIC: */
    /* send traffic and set capture on the vidx cascade port */
    sendTrafficWithCaptureEgressPort(portInVidxCascade, GT_FALSE);
}

/**
* @internal prvTgfEgressMirrorAnalyzerIsEportIsVidxTest_deconfig function
* @endinternal
*
* @brief   de-config for prvTgfEgressMirrorAnalyzerIsEportIsVidxTest
*/
static GT_VOID prvTgfEgressMirrorAnalyzerIsEportIsVidxTest_deconfig
(
    GT_VOID
)
{
    GT_STATUS   rc;
    CPSS_PORTS_BMP_STC      portBitmap;
    GT_U32  analyzerIndex = PRV_TGF_MIRROR_GLOB_ANALYZER_IDX_CNS;
    GT_U32  analyzerDevNum = prvTgfDevNum;
    CPSS_INTERFACE_INFO_STC physicalInfo;/* physical port info for the analyzer in eArch */
    GT_U32  analyzerPort = 0;/*HW default value*/
    GT_U16  vidx = PRV_TGF_MIRROR_ANALYZER_VIDX_CNS;
    GT_U16  myVid = PRV_TGF_VLANID_CNS;

    /* restore the mapping from vid 'default' */
    rc = prvTgfBrgVlanTranslationEntryWrite(prvTgfDevNum,myVid,
        CPSS_DIRECTION_EGRESS_E ,
        vlanTranslationEntryDef);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanTranslationEntryWrite: from vid %d to vid %d",
                                 myVid,
                                 vlanTranslationEntryDef);

    /* Disable Tx mirroring on sender port */
    rc = prvTgfMirrorTxPortSet(prvTgfPortsArray[PRV_TGF_RCV_PORT_IDX_CNS], GT_FALSE, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortSet: %d %d",
                                 prvTgfPortsArray[PRV_TGF_RCV_PORT_IDX_CNS], GT_FALSE);

    /* restore the 2 vidx ports (of analyzer) as 'STP DISABLED' */
    stpStateSet(CPSS_STP_DISABLED_E/*vidx*/,CPSS_STP_DISABLED_E/*not vidx*/);

    /*******************************************/
    /* restore analyzer mode : HOP_BY_HOP */
    /*******************************************/
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorToAnalyzerForwardingModeSet: %d",
                                 PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E);

    /* restore the analyzer index */
    prvMirrorAnalyzerInterfaceSet(analyzerIndex,analyzerDevNum,analyzerPort);

    /* invalidate the vidx entry */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portBitmap);
    rc = prvTgfBrgMcEntryWrite(prvTgfDevNum, vidx, &portBitmap);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgMcEntryWrite: %d , vidx %d",
                                 prvTgfDevNum,vidx);

    /* invalidate the vlan */
    rc = prvTgfBrgVlanEntryInvalidate(myVid);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: vid %d",
                                 myVid);

    /* restore the E2PHY */
    physicalInfo.type = CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.hwDevNum = analyzerDevNum;
    physicalInfo.devPort.portNum = 0;/*HW defaults*/
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                analyzerPort,
                &physicalInfo);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: dev %d, vidx %d",
                                 prvTgfDevNum, vidx);

    physicalInfo.devPort.hwDevNum = prvTgfDevNum;

    /* set first eport */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                TRG_EPORT_1_CNS,
                &physicalInfo);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: %d, %d",
                                 prvTgfDevNum, TRG_EPORT_1_CNS);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* restore DSA tag testing mode */
    prvTgfMirrorConfigParamsSet(GT_FALSE,TGF_DSA_2_WORD_TYPE_E,PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E);
}

/**
* @internal prvTgfEgressMirrorAnalyzerIsEportIsVidxTest function
* @endinternal
*
* @brief   Test egress analyzer that is EPort that is mapped by the E2Phy to physical VIDX.
*         With analyzer (egress ports) vidx members in STP blocking/forward
*/
GT_VOID prvTgfEgressMirrorAnalyzerIsEportIsVidxTest
(
    GT_VOID
)
{
    initVidxToUse();
    prvTgfEgressMirrorAnalyzerIsEportIsVidxTest_config();
    if(UTF_CPSS_DXCH_XCAT3X_CHECK_MAC(prvTgfDevNum))
    {
        /* the egress port is on PHY1690 ... that can not be set as cascade port */
        prvTgfEgressMirrorAnalyzerIsEportIsVidxTest_traffic_AC3X();
    }
    else
    {
        prvTgfEgressMirrorAnalyzerIsEportIsVidxTest_traffic();
    }
    prvTgfEgressMirrorAnalyzerIsEportIsVidxTest_deconfig();
}

/**
* @internal prvTgfEgressMirroringToAnalyzerCounterTest function
* @endinternal
*
* @brief Test to check the counters for mirror to analyzer packet.
*/
GT_VOID prvTgfEgressMirrorToAnalyzerCounterTest
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_BOOL      floodExpected;
    GT_U32       numOfMirrorCopies;
    CPSS_DXCH_CFG_REPLICATION_COUNTERS_TYPE_ENT replicationType;
    GT_U32      counterVal;

    /* reset port's counters */
    PRV_UTF_LOG0_MAC("reset ETH counters , and force link up on the ports \n");
    prvTgfEthCountersReset(prvTgfDevNum);

    prvTgfPortsNum = 5;

    /* AUTODOC: clear the replication counter before sending packet*/
    replicationType = CPSS_DXCH_CFG_SNIFF_REPLICATION_E;
    rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");

    replicationType = CPSS_DXCH_CFG_OUTGOING_FORWARD_E;
    rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");

    /* AUTODOC: Basic configuration*/
    prvTgfEgressVlanMirroringTest_BasicConfig();

    /* AUTODOC: Send flood in the vlan and check that analyzer port NOT get it */
    prvTgfEgressVlanMirroringTest_generateTraffic();

    /* AUTODOC: Get the Counter value and verify no mirrored packets*/
    replicationType = CPSS_DXCH_CFG_SNIFF_REPLICATION_E;
    rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, counterVal, "cpssDxChCfgReplicationCountersGet");
    PRV_UTF_LOG1_MAC("No of Packets replicated to analyzer: %d \n", counterVal);

    replicationType = CPSS_DXCH_CFG_OUTGOING_FORWARD_E;
    rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");
    UTF_VERIFY_EQUAL0_STRING_MAC(4, counterVal, "cpssDxChCfgReplicationCountersGet");
    PRV_UTF_LOG1_MAC("No of Outgoing Packets : %d \n", counterVal);

    floodExpected = GT_TRUE;
    numOfMirrorCopies = 0;
    prvTgfEgressVlanMirroringTest_checkEgressPorts(floodExpected,numOfMirrorCopies);

    /* AUTODOC: Configure to set Mirror to analyzer*/
    prvTgfEgressVlanMirroringTest_checkSingleDestination();

    /* AUTODOC: Get the counter value and verify Mirror to analyzer packet count*/
    replicationType = CPSS_DXCH_CFG_SNIFF_REPLICATION_E;
    rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");
    UTF_VERIFY_EQUAL0_STRING_MAC(1, counterVal, "cpssDxChCfgReplicationCountersGet");
    PRV_UTF_LOG1_MAC("No of Packets replicated to analyzer: %d \n", counterVal);

    replicationType = CPSS_DXCH_CFG_OUTGOING_FORWARD_E;
    rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");
    UTF_VERIFY_EQUAL0_STRING_MAC(7, counterVal, "cpssDxChCfgReplicationCountersGet");
    PRV_UTF_LOG1_MAC("No of Outgoing Packets : %d \n", counterVal);

    /* AUTODOC: flush FDB (to remove the FDB entry) */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);
    /* AUTODOC: Restore the config */
    prvTgfEgressVlanMirroringTest_configRestore();
}

/**
* @internal prvTgfBrgEnhancedMirroringPriorityConfigurationSet function
* @endinternal
*
* @brief   Prepare to test:
*         1. Create VLAN of ports [0,1]
*         2. Turn on ingress+Egress VLAN mirroring
*         4. Save configuration of analyzer interface
*         5. Set source based forwarding mode
*
*/
static GT_VOID prvTgfBrgEnhancedMirroringPriorityGlobalConfigurationSet
(
    OUT  CPSS_DXCH_MIRROR_REPLICATION_WRR_STC *origConfigPtr
)
{
    GT_STATUS rc;
    GT_U16    vlanId = PRV_TGF_VLANID_ANOTHER_CNS;
    GT_U32    analyzerIndex = PRV_TGF_ENHANCED_MIRROR_ANALYZER_IDX_CNS;

    /* AUTODOC: Save original configurations */
    /* Save original state of WRR Arbiter Weights register*/
    rc = cpssDxChMirrorArbiterGet(prvTgfDevNum, NULL, NULL,origConfigPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChMirrorArbiterGet");

    /* Save original analyzer interface settings */
    rc = prvTgfMirrorAnalyzerInterfaceGet(prvTgfDevNum, analyzerIndex, &originalAnalyzerInterface);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceGet");

    /* Save original forwarding mode settings */
    rc = prvTgfMirrorToAnalyzerForwardingModeGet(prvTgfDevNum, &originalForwardingMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorToAnalyzerForwardingModeGet");

    /* AUTODOC: Set VLAN with 2 ports*/
    rc = prvTgfBrgDefVlanEntryWithPortsSet(vlanId,
                                           prvTgfPortsArray, NULL, NULL, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

   /* AUTODOC: Set analyzer port for analyzer */
    prvMirrorAnalyzerInterfaceSet(analyzerIndex,prvTgfDevNum, prvTgfPortsArray[VLAN_ANALYZER_PORT_INDEX_CNS]);

    /* AUTODOC:  Set source-based forwarding mode. */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorToAnalyzerForwardingModeSet: %d",
                                 PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E);

}

/**
* @internal prvTgfBrgEnhancedMirroringPriorityConfigurationRestore function
* @endinternal
*
* @brief Restore all original configurations pre to EnhancedMirroringPriority test
*/
static GT_VOID prvTgfBrgEnhancedMirroringPriorityConfigurationRestore
(
    IN CPSS_DXCH_MIRROR_REPLICATION_WRR_STC *origConfigPtr
)
{
    GT_STATUS rc;
    GT_U16    vlanId = PRV_TGF_VLANID_ANOTHER_CNS;

    /* AUTODOC: Restore original configurations */
    /*Restore original state of WRR Arbiter Weights register*/
    rc = cpssDxChMirrorArbiterSet(prvTgfDevNum, NULL, NULL, origConfigPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChMirrorArbiterSet");

    /* Restore original analyzer interface settings */
    rc = prvTgfMirrorAnalyzerInterfaceSet(PRV_TGF_ENHANCED_MIRROR_ANALYZER_IDX_CNS,
        &originalAnalyzerInterface);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceSet");

    /* Restore default forwarding mode */
    rc = prvTgfMirrorToAnalyzerForwardingModeSet(originalForwardingMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfMirrorToAnalyzerForwardingModeSet: %d", originalForwardingMode);

    /* AUTODOC: Disable vlan egress mirroring. */
    rc = prvTgfBrgVlanMirrorToTxAnalyzerSet(vlanId,GT_FALSE,prvTgfPortsArray[VLAN_ANALYZER_PORT_INDEX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMirrorToTxAnalyzerSet");

    /* AUTODOC: Disable VLAN ingress mirroring. */
    rc = prvTgfBrgVlanIngressMirrorEnable(prvTgfDevNum, vlanId, GT_FALSE,
                                          prvTgfPortsArray[VLAN_ANALYZER_PORT_INDEX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfvBrgVlanIngressMirrorEnable");

    /* AUTODOC: Invalidate VLAN*/
    rc = prvTgfBrgDefVlanEntryInvalidate(vlanId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d", vlanId);

    /* AUTODOC: Flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
        prvTgfDevNum, GT_TRUE);

}

/**
* @internal prvTgfBrgEnhancedMirroringPriorityDefaultConfigurationSet function
* @endinternal
*
* @brief   set all weights to non zero value
*
*
*/
static GT_VOID prvTgfBrgEnhancedMirroringPriorityDefaultConfigurationSet
(
    IN GT_VOID
)
{
    /* AUTODOC: Set all Weights to non zero value */
    GT_STATUS rc;
    CPSS_DXCH_MIRROR_REPLICATION_WRR_STC config;
    config.egressQCNWeight=20;
    config.ingressMirrorWeight=20;
    config.ingressTrapWeight=20;
    config.egressMirrorAndTrapWeight=20;
    config.ingressControlPipeWeight=20;
    rc = cpssDxChMirrorArbiterSet(prvTgfDevNum, NULL, NULL, &config);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChMirrorArbiterSet: %d",GT_OK);
}

/**
* @internal prvTgfBrgEnhancedMirroringPriorityDefaultConfigurationSet function
* @endinternal
*
* @brief   Set configurations for EnhancedMirroringPriority test.
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:   DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2;
*                                   Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] ingressMirror      - enable - GT_TRUE, disabled GT_FALSE.
* @param[in] egressMirror       - enable - GT_TRUE, disabled GT_FALSE.
* @param[in] trapToCPU          - trap to CPU enable - GT_TRUE,trap to CPU disabled - GT_FALSE.
*
*
*
*/
static GT_VOID prvTgfBrgEnhancedMirroringPriorityConfigurationSet
(
    IN GT_BOOL ingressMirrorEn,
    IN GT_BOOL egressMirrorEn,
    IN GT_BOOL trapToCpuEn
)
{
    GT_STATUS   rc;
    GT_U16      vlanId = PRV_TGF_VLANID_ANOTHER_CNS;
    GT_U32      analyzerIndex = PRV_TGF_ENHANCED_MIRROR_ANALYZER_IDX_CNS;
    PRV_TGF_PACKET_CMD_ENT packetCommand;

    packetCommand = (trapToCpuEn == GT_TRUE) ? PRV_TGF_PACKET_CMD_TRAP_TO_CPU_E : PRV_TGF_PACKET_CMD_FORWARD_E ;

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:00:02, VLAN 2, port 1 */
    rc = prvTgfBrgDefFdbMacEntryAndCommandOnPortSet(prvTgfFdbEntryMacAddr,
                                          vlanId,
                                          prvTgfDevNum,
                                          prvTgfPortsArray[VLAN_RX_PORT_INDEX_CNS],
                                          packetCommand,
                                          GT_FALSE);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfBrgDefFdbMacEntryAndCommandOnPortSet");

    /* AUTODOC: Set VLAN ingress mirroring enabled/disabled */
    rc = prvTgfBrgVlanIngressMirrorEnable(prvTgfDevNum, vlanId, ingressMirrorEn, analyzerIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfvBrgVlanIngressMirrorEnable");

    /* AUTODOC: Set analyzer index . */
    rc = prvTgfBrgVlanMirrorToTxAnalyzerSet(vlanId, egressMirrorEn, analyzerIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMirrorToTxAnalyzerSet");

    /* AUTODOC: Set VLAN egress mirroring enabled/disabled */
    rc = prvTgfMirrorTxPortVlanEnableSet(prvTgfDevsArray[VLAN_TX_PORT_INDEX_CNS] ,
                                         prvTgfPortsArray[VLAN_TX_PORT_INDEX_CNS] , egressMirrorEn);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorTxPortVlanEnableSet");
}

/**
* @internal prvTgfBrgEnhancedMirroringPriorityTrafficGenerate function
* @endinternal
*/
static GT_VOID prvTgfBrgEnhancedMirroringPriorityTrafficGenerate
(
    IN GT_U32 expectedPackets[3][4][2],
    GT_U8     state
)
{
    GT_STATUS       rc;
    TGF_PACKET_STC *PacketToSendPtr =&prvTgfIPv4PacketInfoEnahancedMirrorTest;
    GT_U32          senderPortIndex = VLAN_TX_PORT_INDEX_CNS;
    GT_U32          recievingPortIndex = VLAN_RX_PORT_INDEX_CNS;

    /* AUTODOC: Generate traffic and check counters: */
    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset\n");

    /* Setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, PacketToSendPtr, prvTgfBurstCount,
        0, NULL);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
        prvTgfDevNum, prvTgfPortsArray[senderPortIndex]);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear\n");

    /* start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture\n");

    /* Transmitting from port 0*/
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[senderPortIndex]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
        prvTgfDevNum, prvTgfPortsArray[senderPortIndex]);

    /* check counters for Tx port */
    rc = prvTgfEthCountersCheck(
        prvTgfDevNum,
        prvTgfPortsArray[senderPortIndex],
        /*expected Rx*/expectedPackets[0/*TxPort*/][state /*case*/][0/*expected Rx packets*/],
        /*expected Tx*/expectedPackets[0/*TxPort*/][state /*case*/][1/*expected Tx packets*/],
        0,
        prvTgfBurstCount);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfEthCountersCheck: %d, %d\n",
        prvTgfDevNum, prvTgfPortsArray[senderPortIndex]);

    /* check counters for Rx port */
    rc = prvTgfEthCountersCheck(
        prvTgfDevNum,
        prvTgfPortsArray[recievingPortIndex],
        /*expected Rx*/expectedPackets[1/*RxPort*/][state /*case*/][0/*expected Rx packets*/],
        /*expected Tx*/expectedPackets[1/*RxPort*/][state /*case*/][1/*expected Tx packets*/],
        0,
        prvTgfBurstCount);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfEthCountersCheck: %d, %d\n",
        prvTgfDevNum, prvTgfPortsArray[recievingPortIndex]);

    /* check counters for analyzer port */
    rc = prvTgfEthCountersCheck(
        prvTgfDevNum,
        prvTgfPortsArray[VLAN_ANALYZER_PORT_INDEX_CNS],
        /*expected Rx*/expectedPackets[2/*AnalyzerPort*/][state /*case*/][0/*expected Rx packets*/],
        /*expected Tx*/expectedPackets[2/*AnalyzerPort*/][state /*case*/][1/*expected Tx packets*/],
        0,
        prvTgfBurstCount);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfEthCountersCheck: %d, %d\n",
        prvTgfDevNum, prvTgfPortsArray[VLAN_ANALYZER_PORT_INDEX_CNS]);
}


/**
* @internal prvTgfBrgEnhancedMirroringPrioritySingleTest function
* @endinternal
*
*/
static GT_VOID prvTgfBrgEnhancedMirroringPrioritySingleTest
(
    IN CPSS_DXCH_MIRROR_REPLICATION_WRR_STC config
)
{
    GT_STATUS       rc;
    GT_U16          vlanId = PRV_TGF_VLANID_ANOTHER_CNS;
    GT_U8           devNum = prvTgfDevNum;
    GT_BOOL         getFirst = GT_TRUE;
    GT_U8           packetBuff[TGF_RX_BUFFER_SIZE_CNS] = {0};
    GT_U32          buffLen  = TGF_RX_BUFFER_SIZE_CNS;
    GT_U32          packetActualLength = 0;
    GT_U8           queue;
    TGF_NET_DSA_STC rxParam;
    GT_U8           state =0;
    CPSS_NET_RX_CPU_CODE_ENT expectedCpuCode = CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E; /*CPSS_NET_IPV4_BROADCAST_PACKET_E*//*GT_NO_MORE habe to check on board the beahviour.*/
    GT_U32          expectedPackets[3][4][2] ={/*{expected Rx packets , expected Tx packets}*/
                                                                    {/*Tx port*/
                                                                    {1,1} , /* egressMirrorAndTrapWeight = 0 */
                                                                    {0,0} , /* ingressControlPipeWeight =0 */
                                                                    {1,1} , /* ingressMirrorWeight =0 */
                                                                    {1,1}   /* ingressTrapWeight=0 */
                                                                    } ,
                                                                    {/*Rx port*/
                                                                    {0,1} , /* egressMirrorAndTrapWeight = 0 */
                                                                    {0,0} , /* ingressControlPipeWeight =0 */
                                                                    {0,1} , /* ingressMirrorWeight =0 */
                                                                    {0,0}   /* ingressTrapWeight=0 */
                                                                    } ,
                                                                    {/*analyzer port*/
                                                                    {0,0} , /* egressMirrorAndTrapWeight = 0 */
                                                                    {0,0} , /* ingressControlPipeWeight =0 */
                                                                    {0,0} , /* ingressMirrorWeight =0 */
                                                                    {0,0}   /* ingressTrapWeight=0 */
                                                                    }};
    GT_U32          expectedPacketsBefore[3][4][2] ={/*{expected Rx packets , expected Tx packets}*/
                                                                    {/*Tx port*/
                                                                    {1,1} , /* egressMirrorAndTrapWeight = 0 */
                                                                    {1,1} , /* ingressControlPipeWeight =0 */
                                                                    {1,1} , /* ingressMirrorWeight =0 */
                                                                    {1,1}   /* ingressTrapWeight=0 */
                                                                    } ,
                                                                    {/*Rx port*/
                                                                    {0,1} , /* egressMirrorAndTrapWeight = 0 */
                                                                    {0,1} , /* ingressControlPipeWeight =0 */
                                                                    {0,1} , /* ingressMirrorWeight =0 */
                                                                    {0,0}   /* ingressTrapWeight=0 */
                                                                    } ,
                                                                    {/*analyzer port*/
                                                                    {0,1} , /* egressMirrorAndTrapWeight = 0 */
                                                                    {0,0} , /* ingressControlPipeWeight =0 */
                                                                    {0,1} , /* ingressMirrorWeight =0 */
                                                                    {0,0}   /* ingressTrapWeight=0 */
                                                                    }};

    /*  2 entries for each case {expected Rx packets,expected Tx packets}
        4 cases {egressMirrorAndTrapWeight = 0, ingressControlPipeWeight =0 , ingressMirrorWeight =0,ingressTrapWeight=0}*/
    /* AUTODOC: expectedPackets[Tx/Rx/analyzer][case][expected Rx/Tx packet]*/

    /* set all weights to non zero value */
    prvTgfBrgEnhancedMirroringPriorityDefaultConfigurationSet();
    /* AUTODOC: Set Configuration for each field tested */
    /*egress packet from analayzer port = 0 */
    if (config.egressMirrorAndTrapWeight == 0)
    {
        prvTgfBrgEnhancedMirroringPriorityConfigurationSet(GT_FALSE, /*ingressMirror*/
                                                           GT_TRUE,  /*EgressMirror*/
                                                           GT_FALSE  /*Trap to CPU*/);
        state = 0;
    }
    /*forwarding - no packets from the sender port*/
    if (config.ingressControlPipeWeight == 0)
    {
        prvTgfBrgEnhancedMirroringPriorityConfigurationSet(GT_FALSE, /*ingressMirror*/
                                                           GT_FALSE, /*EgressMirror*/
                                                           GT_FALSE  /*Trap to CPU*/);
        state = 1;
    }
    /* ingress mirror to analayzer port */
    if (config.ingressMirrorWeight == 0)

    {
        prvTgfBrgEnhancedMirroringPriorityConfigurationSet(GT_TRUE,  /*ingressMirror*/
                                                           GT_FALSE, /*EgressMirror*/
                                                           GT_FALSE  /*Trap to CPU*/);
        state = 2;
    }
    /*trap to CPU - no packets from the CPU*/
    if (config.ingressTrapWeight == 0)
    {
        /* Set trapping Enable,but the packet shoudln't get trapped because the weight set to zero!*/
        prvTgfBrgEnhancedMirroringPriorityConfigurationSet(GT_FALSE, /*ingressMirror*/
                                                           GT_FALSE, /*EgressMirror*/
                                                           GT_TRUE  /*Trap to CPU*/);
        state = 3;
    }

    /* Generate Traffic */
    PRV_UTF_LOG0_MAC("\n\n==========  Counters before configuration ===============\n\n");
    prvTgfBrgEnhancedMirroringPriorityTrafficGenerate(expectedPacketsBefore,state);

    /* AUTODOC: check CPU code for ingressTrapWeight test */
    if (config.ingressTrapWeight == 0)
    {
        /* get first entry from rxNetworkIf table */
        rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                           getFirst, GT_TRUE, packetBuff,
                                           &buffLen, &packetActualLength,
                                           &devNum, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");
        /* check CPU code */
        UTF_VERIFY_EQUAL0_STRING_MAC(expectedCpuCode, rxParam.cpuCode, "Wrong CPU Code");
        rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                           GT_FALSE, GT_TRUE, packetBuff,
                                           &buffLen, &packetActualLength,
                                           &devNum, &queue, &rxParam);
        /* check CPU code again to see that got only one packet */
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet %d");
    }

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear\n");

    /* AUTODOC: configure one weight to be zero */
    rc = cpssDxChMirrorArbiterSet(prvTgfDevNum, NULL, NULL, &config);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChMirrorArbiterSet: %d",GT_OK);

    /* Generate Traffic */
    PRV_UTF_LOG0_MAC("\n\n==========  Counters after configuration ===============\n\n");

    prvTgfBrgEnhancedMirroringPriorityTrafficGenerate(expectedPackets,state);

    /* stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* AUTODOC: check CPU code for ingressTrapWeight test */
    if (config.ingressTrapWeight == 0)
    {
         rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                           GT_FALSE, GT_TRUE, packetBuff,
                                           &buffLen, &packetActualLength,
                                           &devNum, &queue, &rxParam);
        /* check CPU code to see that no packets trapped to CPU */
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet");
        /* reset VLAN filtering command */
        rc = cpssDxChBrgVlanUnkUnregFilterSet(prvTgfDevNum,vlanId,CPSS_DXCH_BRG_VLAN_PACKET_UNREG_IPV4_BCAST_E , CPSS_PACKET_CMD_FORWARD_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanUnkUnregFilterSet");
    }


    /* AUTODOC: Reset port's counters */
    PRV_UTF_LOG0_MAC("Reset ETH counters \n");
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* AUTODOC: clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear\n");

}

/**
* @internal prvTgfBrgEnhancedMirroringPriorityTest function
* @endinternal
*
* @brief   Main function of "CPSS Enhanced Mirroring Priority on Falcon" test.
*/
GT_VOID prvTgfBrgEnhancedMirroringPriorityTest
(
    GT_VOID
)
{
    CPSS_DXCH_MIRROR_REPLICATION_WRR_STC config;
    CPSS_DXCH_MIRROR_REPLICATION_WRR_STC origConfig;

    /* AUTODOC: configurate defaults */
    prvTgfBrgEnhancedMirroringPriorityGlobalConfigurationSet(&origConfig);

    /* AUTODOC: test 1 - ingressMirrorWeight*/
    PRV_UTF_LOG0_MAC("\n\n***********  test 1 - ingressMirrorWeight=0  *****************\n\n");
    config.egressQCNWeight=20;
    config.ingressMirrorWeight=0;
    config.ingressTrapWeight=20;
    config.egressMirrorAndTrapWeight=20;
    config.ingressControlPipeWeight=20;
    utfGeneralStateMessageSave(0,"Send packets");
    prvTgfBrgEnhancedMirroringPrioritySingleTest(config);

    /* AUTODOC: test 2 - ingressTrapWeight*/
    PRV_UTF_LOG0_MAC("\n\n***********  test 2 - ingressTrapWeight=0  *****************\n\n");
    config.egressQCNWeight=20;
    config.ingressMirrorWeight=20;
    config.ingressTrapWeight=0;
    config.egressMirrorAndTrapWeight=20;
    config.ingressControlPipeWeight=20;
    utfGeneralStateMessageSave(0,"Send packets");
    prvTgfBrgEnhancedMirroringPrioritySingleTest(config);

    /* AUTODOC: test 3 - egressMirrorAndTrapWeight */
    PRV_UTF_LOG0_MAC("\n\n***********  test 3 - egressMirrorAndTrapWeight=0 *****************\n\n");
    config.egressQCNWeight=20;
    config.ingressMirrorWeight=20;
    config.ingressTrapWeight=20;
    config.egressMirrorAndTrapWeight=0;
    config.ingressControlPipeWeight=20;
    utfGeneralStateMessageSave(0,"Send packets");
    prvTgfBrgEnhancedMirroringPrioritySingleTest(config);

    /* AUTODOC: test 4 - ingressControlPipeWeight*/
    PRV_UTF_LOG0_MAC("\n\n***********  test 4 - ingressControlPipeWeight=0 *****************\n\n");
    config.egressQCNWeight=20;
    config.ingressMirrorWeight=20;
    config.ingressTrapWeight=20;
    config.egressMirrorAndTrapWeight=20;
    config.ingressControlPipeWeight=0;
    utfGeneralStateMessageSave(0,"Send packets");
    tgfPrvIntendedError = GT_TRUE;
    prvTgfBrgEnhancedMirroringPrioritySingleTest(config);
    tgfPrvIntendedError = GT_FALSE;

    /* Restore global configurations*/
    prvTgfBrgEnhancedMirroringPriorityConfigurationRestore(&origConfig);
}

/* number of send traffic iteration */
#define PRV_TGF_MAX_ITER_CNS          2

/**
* @internal prvTgfErepDropPacketTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfErepDropPacketTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS       rc         = GT_OK;
    GT_U32          portsCount = 0;
    GT_U32          partsCount = 0;
    GT_U32          packetSize = 0;
    GT_U32          portIter,i = 0;
    GT_U32          numVfd     = 0;
    GT_U32          dropCount  = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    TGF_PACKET_STC prvTgfPacketInfoLocal = prvTgfPacketInfo;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* number of parts in packet */
    partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);

    /* calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketPartArray, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

 /* reset drop counters before sending packets */
    rc = prvTgfBrgCntDropCntrSet(0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgCntDropCntrSet: %d", prvTgfDevNum);

    /* AUTODOC: enable ingress filtering on send port 0 */
    rc = prvTgfBrgVlanPortIngFltEnable(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngFltEnable: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* no VFD applied */
    numVfd = 0;

    /* update packet */
    ((TGF_PACKET_VLAN_TAG_STC*) prvTgfPacketInfoLocal.partsArray[1].partPtr)->vid = prvTgfBrgVlanIngrFltInvalidVidGet();


    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfoLocal, prvTgfBurstCount, numVfd, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d", prvTgfDevNum);


    /* AUTODOC: Make 2 iterations: */
    for (i = 0; i < PRV_TGF_MAX_ITER_CNS; i++)
    {
        /* reset counters */
        portsCount = prvTgfPortsNum;

        for (portIter = 0; portIter < portsCount; portIter++)
        {
            rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }

        /* AUTODOC: send BC packet from port 1 with NOT valid VLAN */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* AUTODOC: verify to get no traffic */
        for (portIter = 0; portIter < portsCount; portIter++)
        {
            /* read counters */
            rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);

            if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
            {
                /* check Rx counters */
                UTF_VERIFY_EQUAL0_STRING_MAC((packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount, portCntrs.goodOctetsRcv.l[0],
                                             "get another goodOctetsRcv counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                             "get another goodPktsRcv counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.brdcPktsRcv.l[0],
                                             "get another brdcPktsRcv counter than expected");
            }
            else
            {
                /* check Tx counters */
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodOctetsSent.l[0], "get another goodOctetsSent counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0], "get another goodPktsSent counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.ucPktsSent.l[0], "get another ucPktsSent counter than expected");
            }
        }

        /* get drop counter */
        rc = prvTgfBrgCntDropCntrGet(&dropCount);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgCntDropCntrGet: %d", prvTgfDevNum);

        /* AUTODOC: check dropped counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount * (i + 1), dropCount, "get another drop counter than expected");
    }

    /* restore packet's VLAN */
    ((TGF_PACKET_VLAN_TAG_STC*) prvTgfPacketInfoLocal.partsArray[1].partPtr)->vid = PRV_TGF_VLANID_CNS;

}

/**
* @internal prvTgfErepDropPacketTrafficGenerateWithEgressMirroring function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfErepDropPacketTrafficGenerateWithEgressMirroring
(
    GT_VOID
)
{
    GT_STATUS       rc         = GT_OK;
    GT_U32          partsCount = 0;
    GT_U32          packetSize = 0;
    GT_U32          portIter,i = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* number of parts in packet */
    partsCount = sizeof(prvTgfPacketPartArrayUC1Learn) / sizeof(TGF_PACKET_PART_STC);

    /* calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketPartArrayUC1Learn, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* Disable analyzer port */
    rc = cpssDxChBrgEgrFltPortLinkEnableSet(prvTgfDevNum, prvTgfPortsArray[4], CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "ERROR of cpssDxChBrgEgrFltPortLinkEnableSet: port=%d",
                                 prvTgfPortsArray[4]);

    /* AUTODOC: Make 2 iterations: */
    for (i = 0; i < PRV_TGF_MAX_ITER_CNS; i++)
    {
        /* reset counters */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }

        /* AUTODOC: send UC packet from port 1 with: */
        /* AUTODOC:   DA=00:11:22:33:44:55, SA=00:00:00:00:22:22, VID=5 */
        prvTgfMirrorTestPacketSend(prvTgfPortsArray[PRV_TGF_TX_MIRR_SEND_IDX_CNS], &prvTgfPacketInfoUC1);


        /* AUTODOC: verify to get no traffic */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            /* read counters */
            rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);

            if (PRV_TGF_TX_MIRR_SEND_IDX_CNS == portIter)
            {
                /* check Rx counters */
                UTF_VERIFY_EQUAL1_STRING_MAC((packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount, portCntrs.goodOctetsRcv.l[0],
                                             "get another goodOctetsRcv counter than expected, port[%d]", prvTgfPortsArray[portIter]);
                UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                             "get another goodPktsRcv counter than expected, port[%d]", prvTgfPortsArray[portIter]);
                UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfBurstCount, portCntrs.ucPktsRcv.l[0],
                                             "get another ucPktsRcv counter than expected, port[%d]", prvTgfPortsArray[portIter]);
            }
            else
            {
                if (portIter == 0)
                {
                    UTF_VERIFY_EQUAL1_STRING_MAC(0, portCntrs.goodPktsSent.l[0],
                                                 "get another goodPktsSent counter than expected, port[%d]", prvTgfPortsArray[portIter]);
                    UTF_VERIFY_EQUAL1_STRING_MAC(0, portCntrs.ucPktsSent.l[0],
                                                     "get another ucPktsSent counter than expected, port[%d]", prvTgfPortsArray[portIter]);
                }
                else
                {
                    UTF_VERIFY_EQUAL1_STRING_MAC(1, portCntrs.goodPktsSent.l[0],
                                                 "get another goodPktsSent counter than expected, port[%d]", prvTgfPortsArray[portIter]);
                    UTF_VERIFY_EQUAL1_STRING_MAC(1, portCntrs.ucPktsSent.l[0],
                                                     "get another ucPktsSent counter than expected, port[%d]", prvTgfPortsArray[portIter]);
                }
            }
        }
    }

    /* Enable analyzer port */
    rc = cpssDxChBrgEgrFltPortLinkEnableSet(prvTgfDevNum, prvTgfPortsArray[4], CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "ERROR of cpssDxChBrgEgrFltPortLinkEnableSet: port=%d",
                                 prvTgfPortsArray[4]);
}

#define NOT_VALID_CNS   0xFFFFFFFF

/**
* @internal prvTgfHawkErepDropToTrapOrMirrorTest function
* @endinternal
*
* @brief   Main function for EREP Trap to Drop or Mirrir To Ananlyze.
*/
GT_VOID prvTgfHawkErepDropToTrapOrMirrorTest
(
    GT_VOID
)
{
    GT_STATUS rc;
    CPSS_DXCH_MIRROR_ON_DROP_STC dropConfig;
    CPSS_DXCH_CFG_REPLICATION_COUNTERS_TYPE_ENT replicationType;
    GT_U32      counterVal;
    CPSS_INTERFACE_INFO_STC portInterface;

    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.portNum = NOT_VALID_CNS;

    /* Set L2 configuration */
    prvTgfBrgVlanIngrFltConfigurationSet();
    PRV_UTF_LOG0_MAC("\n\n***********  test 1 - DROP mode - CPSS_DXCH_MIRROR_ON_DROP_MODE_TRAP_E *****************\n\n");
    dropConfig.mode = CPSS_DXCH_MIRROR_ON_DROP_MODE_TRAP_E;
    dropConfig.analyzerIndex = 0;

    rc = prvTgfMirrorOnDropConfigSet(&dropConfig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorOnDropConfigSet\n");

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* Start capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d",
                                 portInterface.devPort.hwDevNum, GT_TRUE);

    /* AUTODOC: clear the replication counter before sending packet*/
    replicationType = CPSS_DXCH_CFG_SNIFF_REPLICATION_E;
    rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");

    replicationType = CPSS_DXCH_CFG_TRAP_REPLICATION_E;
    rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");

    /* Generate traffic */
    prvTgfErepDropPacketTrafficGenerate();

    /* AUTODOC: get the replication counter after sending packet*/
    replicationType = CPSS_DXCH_CFG_SNIFF_REPLICATION_E;
    rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, counterVal, "cpssDxChCfgReplicationCountersGet");
    PRV_UTF_LOG1_MAC("Number of Packets replicated to analyzer: %d \n", counterVal);

    replicationType = CPSS_DXCH_CFG_TRAP_REPLICATION_E;
    rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");
    UTF_VERIFY_EQUAL0_STRING_MAC(2, counterVal, "cpssDxChCfgReplicationCountersGet");
    PRV_UTF_LOG1_MAC("Number of Packets trapped to CPU: %d \n", counterVal);

    /* wait for packets come to CPU */
    rc = tgfTrafficGeneratorRxInCpuNumWait(PRV_TGF_MAX_ITER_CNS, 500, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuNumWait: %d, %d",
                                 portInterface.devPort.hwDevNum);
    /* Stop capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterface.devPort.hwDevNum, GT_FALSE);
    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    PRV_UTF_LOG0_MAC("\n\n***********  test 2 - DROP mode - CPSS_DXCH_MIRROR_ON_DROP_MODE_MIRROR_TO_ANALYZER_E *****************\n\n");
    dropConfig.mode = CPSS_DXCH_MIRROR_ON_DROP_MODE_MIRROR_TO_ANALYZER_E;
    dropConfig.analyzerIndex = 4;

    /* AUTODOC: clear the replication counter before sending packet*/
    replicationType = CPSS_DXCH_CFG_SNIFF_REPLICATION_E;
    rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");

    replicationType = CPSS_DXCH_CFG_TRAP_REPLICATION_E;
    rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");

    rc = prvTgfMirrorOnDropConfigSet(&dropConfig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorOnDropConfigSet\n");

    /* Generate traffic */
    prvTgfErepDropPacketTrafficGenerate();

    /* AUTODOC: get the replication counter after sending packet*/
    replicationType = CPSS_DXCH_CFG_SNIFF_REPLICATION_E;
    rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");
    UTF_VERIFY_EQUAL0_STRING_MAC(2, counterVal, "cpssDxChCfgReplicationCountersGet");
    PRV_UTF_LOG1_MAC("Number of Packets replicated to analyzer: %d \n", counterVal);

    replicationType = CPSS_DXCH_CFG_TRAP_REPLICATION_E;
    rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, counterVal, "cpssDxChCfgReplicationCountersGet");
    PRV_UTF_LOG1_MAC("Number of Packets trapped to CPU: %d \n", counterVal);

    /* Restore EREP global configuraions */
    dropConfig.mode = CPSS_DXCH_MIRROR_ON_DROP_MODE_DISABLE_E;
    dropConfig.analyzerIndex = 0;
    dropConfig.droppedEgressReplicatedPacketMirrorEnable = GT_FALSE;

    rc = prvTgfMirrorOnDropConfigSet(&dropConfig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorOnDropConfigSet\n");

    /* AUTODOC: disable ingress filtering on send port */
    rc = prvTgfBrgVlanPortIngFltEnable(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngFltEnable: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* Restore L2 configuration */
    prvTgfBrgVlanIngrFltConfigurationRestore();
}

/**
* @internal prvTgfHawkErepDropToTrapOrMirrorTestWithEgressMirroring function
* @endinternal
*
* @brief   Main function for EREP Trap to Drop or Mirrir To Ananlyze with mirroring to analyzer.
*/
GT_VOID prvTgfHawkErepDropToTrapOrMirrorTestWithEgressMirroring
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32      counterVal;
    CPSS_DXCH_MIRROR_ON_DROP_STC dropConfig;
    CPSS_DXCH_CFG_REPLICATION_COUNTERS_TYPE_ENT replicationType;
    CPSS_INTERFACE_INFO_STC portInterface;

    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.portNum = prvTgfPortsArray[4];

    PRV_UTF_LOG0_MAC("\n\n***********  test 1 - DROP mode - CPSS_DXCH_MIRROR_ON_DROP_MODE_MIRROR_TO_ANALYZER_E with egress replicated drop enable *****************\n\n");

    testingMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E;
    /* Set configuration */
    prvTgfEgressMirrorSourceBasedConfigurationSet();

    /* AUTODOC: clear the replication counter before sending packet*/
    replicationType = CPSS_DXCH_CFG_SNIFF_REPLICATION_E;
    rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");

    replicationType = CPSS_DXCH_CFG_TRAP_REPLICATION_E;
    rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* Start capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d",
                                 portInterface.devPort.hwDevNum, GT_TRUE);

#if 0
    dropConfig.mode = CPSS_DXCH_MIRROR_ON_DROP_MODE_MIRROR_TO_ANALYZER_E;
    dropConfig.analyzerIndex = 4;
#endif
    dropConfig.mode = CPSS_DXCH_MIRROR_ON_DROP_MODE_TRAP_E;
    dropConfig.analyzerIndex = 0;
    dropConfig.droppedEgressReplicatedPacketMirrorEnable = GT_TRUE;

    rc = prvTgfMirrorOnDropConfigSet(&dropConfig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorOnDropConfigSet\n");

    /* Generate traffic */
    prvTgfErepDropPacketTrafficGenerateWithEgressMirroring();

    /* AUTODOC: get the replication counter after sending packet*/
    replicationType = CPSS_DXCH_CFG_SNIFF_REPLICATION_E;
    rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");
    UTF_VERIFY_EQUAL0_STRING_MAC(2*PRV_TGF_MAX_ITER_CNS, counterVal, "cpssDxChCfgReplicationCountersGet");
    PRV_UTF_LOG1_MAC("Number of Packets replicated to analyzer: %d \n", counterVal);

    replicationType = CPSS_DXCH_CFG_TRAP_REPLICATION_E;
    rc = cpssDxChCfgReplicationCountersGet(prvTgfDevNum, replicationType, &counterVal);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgReplicationCountersGet");
    UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_MAX_ITER_CNS, counterVal, "cpssDxChCfgReplicationCountersGet");
    PRV_UTF_LOG1_MAC("Number of Packets trapped to CPU: %d \n", counterVal);

    /* wait for packets come to CPU */
    rc = tgfTrafficGeneratorRxInCpuNumWait(PRV_TGF_MAX_ITER_CNS, 500, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuNumWait: %d",
                                 portInterface.devPort.hwDevNum);

    /* Stop capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 portInterface.devPort.hwDevNum, GT_FALSE);
    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* Restore EREP global configuraions */
    dropConfig.mode = CPSS_DXCH_MIRROR_ON_DROP_MODE_DISABLE_E;
    dropConfig.analyzerIndex = 0;
    dropConfig.droppedEgressReplicatedPacketMirrorEnable = GT_FALSE;

    rc = prvTgfMirrorOnDropConfigSet(&dropConfig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfMirrorOnDropConfigSet\n");

    /* Restore configuration */
    prvTgfEgressMirrorSourceBasedConfigurationRestore();
}

