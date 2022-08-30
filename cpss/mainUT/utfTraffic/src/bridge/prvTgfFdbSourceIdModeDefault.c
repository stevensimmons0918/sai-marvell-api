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
* @file prvTgfFdbSourceIdModeDefault.c
*
* @brief Verify that when source ID assignment mode is default source ID,
* packets should filtered only if egress port is deleted from the
* default source ID that configured on ingress port.
* When egress port is deleted from the source ID group of the FDB entry,
* packets should not be filtered.
*
* @version   4
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTrunkGen.h>

#include <common/tgfMirror.h>

#include <bridge/prvTgfFdbSourceIdModeDefault.h>
#include <common/tgfCscdGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfL2MllGen.h>
#include <l2mll/prvTgfL2MllUtils.h>
#include <common/tgfConfigGen.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgSrcId.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* capture port index */
#define PRV_TGF_CPORT_IDX_CNS               1

/* default VLAN Id */
#define PRV_TGF_VLAN_ID_CNS                 2

/* port index to send traffic to */
#define PRV_TGF_TX_PORTIDX_CNS              0

/* default ingress port index */
#define PRV_TGF_INGRESS_PORT_IDX_CNS        0

/* port's default source id */
#define PRV_TGF_SOURCE_ID_CNS               2

/* assigned packets will not be forwarded with this sourceId */
#define PRV_TGF_DISABLED_SOURCEID_CNS       1

/* default egress port index */
#define PRV_TGF_EGRESS_PORT_IDX_CNS         1

/* default number of used FDB mac entries */
#define PRV_TGF_FDB_MAC_ENTRIES_NUM_CNS     2

/* the EVIDX that point to L2 MLL LTT */
#define PRV_TGF_L2_EVIDX_CNS                8

/* MLL pair entry index */
#define PRV_TGF_MLL_PAIR_ENTRY_INDEX_CNS    20

#define PRV_TGF_MLL_LTT_INDEX_CNS           8
#define PRV_TGF_MLL_VIDX_INDEX_CNS          9
#define PRV_TGF_SOURCE_ID_INDEX_CNS         375

/* default number of packets to send */
static GT_U32  prvTgfBurstCount              = 5;

/* default number of dsa packets to send */
static GT_U32  prvTgfDsaBurstCount           = 1;

static CPSS_INTERFACE_INFO_STC egressPortInterface;

/* traffic Tx delay */
#define PRV_TGF_TX_DELAY                    300

/* expected number of Rx and Tx packets on ports*/
static GT_U8 prvTgfPacketsCountRxTxArr[][PRV_TGF_PORTS_NUM_CNS] =
{
    {1, 0, 0, 0},  /* Rx count for generate trafic */
    {1, 1, 0, 0},  /* Tx count for generate trafic */
    {1, 0, 0, 0},  /* Rx count for generate trafic */
    {1, 0, 0, 0}   /* Tx count for generate trafic */
};

/* expected capture triggers */
static GT_U32  prvTgfExpTriggersArr[] =
{
     0x3FF, 0x00
};

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* srcMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, 2                                             /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0F, 0x10, 0x11,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21,
    0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
    0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),                           /* dataLength */
    prvTgfPayloadDataArr                                    /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},      /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
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

/* size of packet that came from cascade port */
#define PRV_TGF_CASCADE_PACKET_SIZE_CNS (  TGF_L2_HEADER_SIZE_CNS        \
                                           + TGF_eDSA_TAG_SIZE_CNS       \
                                           + TGF_VLAN_TAG_SIZE_CNS       \
                                           + sizeof(prvTgfPayloadDataArr))

static TGF_DSA_DSA_FROM_CPU_STC  prvTgfDsaInfo_fromCpu = {
/* struct */ /* dstInterface */
    {
        CPSS_INTERFACE_PORT_E,
        /* struct */
        {
            0,    /* hwDevNum */
            0     /* portNum */
        }, /* devPort */

        0,  /* trunkId */
        0,  /* vidx  */
        0,  /* vlanId */
        0,  /* hwDevNum */
        0,  /* fabricVidx */
        0  /* index */
    }, /* desInterface */

    0,  /*  tc  */
    CPSS_DP_GREEN_E,  /* dp  */
    GT_FALSE,  /* egrFilterEn */
    GT_FALSE,  /* cascadeControl  */
    GT_FALSE,  /*  egrFilterRegistered  */

    0,   /* srcId  */
    0,   /* srcHwDev  */

    {
        {
            GT_FALSE,                        /* excludeInterface;*/
            {
                CPSS_INTERFACE_PORT_E,
                /* struct */
                {
                    0,    /* hwDevNum */
                    0     /* portNum */
                }, /* devPort */

                0,  /* trunkId */
                0,  /* vidx  */
                0,  /* vlanId */
                0,  /* hwDevNum */
                0,  /* fabricVidx */
                0   /* index */
            }, /* desInterface */
            GT_FALSE,                        /* mirrorToAllCPUs;*/

            /* supported in extended, 4 words,  DSA tag */
            GT_FALSE,                     /*  excludedIsPhyPort; */
            PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E  /* srcIsTagged;*/

        },/*multiDest;*/

    },

    /* supported in extended, 4 words,  DSA tag */

    GT_TRUE,   /* isTrgPhyPortValid */
    0,         /* dstEport */
    0         /*  tag0TpidIndex */

};

/* DSA tag without command-specific part(dsaInfo) */
static TGF_PACKET_DSA_TAG_STC  prvTgfPacketDsaTagPart = {
    TGF_DSA_CMD_FROM_CPU_E, /*dsaCommand*/
    TGF_DSA_4_WORD_TYPE_E, /*dsaType*/

    /*TGF_DSA_DSA_COMMON_STC*/
    {
        0,        /*vpt*/
        0,        /*cfiBit*/
        0,        /*vid*/
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

/* parts of packet that came from cascade port */
static TGF_PACKET_PART_STC prvTgfCascadePacketPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_DSA_TAG_E,   &prvTgfPacketDsaTagPart},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* a cascade packet info */
static TGF_PACKET_STC prvTgfCascadePacketInfo = {
    PRV_TGF_CASCADE_PACKET_SIZE_CNS,                                    /* totalLen */
    sizeof(prvTgfCascadePacketPartsArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
    prvTgfCascadePacketPartsArray                                       /* partsArray */
};

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    GT_U32                            portsDefaultSourceId;
    CPSS_CSCD_PORT_TYPE_ENT           prvTgfSavedRxCscdPortType;
    CPSS_CSCD_PORT_TYPE_ENT           prvTgfSavedTxCscdPortType;
    GT_BOOL                           prvTgfSavedRxSrcIdForceEnable;
    GT_U32                            prvTgfSavedRxDefSrcId;
    GT_BOOL                           enableRestore;
    PRV_TGF_L2_MLL_PAIR_STC           mllPairEntry;
    CPSS_BRG_SRC_ID_ASSIGN_MODE_ENT   modeRestore;
    CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_ENT muxModeRestore;
    GT_BOOL                           eVidxEnableRestore;
    PRV_TGF_L2_MLL_LTT_ENTRY_STC      lttEntry;
    CPSS_PORTS_BMP_STC                portsMembers;
    GT_BOOL                           cpuSrcIdMemberRestore;
    GT_BOOL                           mllReplicatedRestore;
} prvTgfRestoreCfg;

/******************************************************************************/


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/
/**
* @internal prvTgfFdbSourceIdModeDefaultConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfFdbSourceIdModeDefaultConfigSet
(
    GT_VOID
)
{
    GT_STATUS                   rc              = GT_OK;
    GT_U32                      portIter        = 0;
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;
    TGF_MAC_ADDR                fdbEntryMacAddr;

    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemSet(&fdbEntryMacAddr, 0, sizeof(fdbEntryMacAddr));

    /* AUTODOC: SETUP CONFIGURATION: */

    PRV_UTF_LOG0_MAC("\n========== Setup config =====================\n\n");

    PRV_UTF_LOG0_MAC("\nSet Vlan configuration\n");

    /* AUTODOC: create VLAN 2 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLAN_ID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "Error of prvTgfBrgDefVlanEntryWrite");

    PRV_UTF_LOG0_MAC("\nAdd MAC entries\n");

    /* AUTODOC: add 2 FDB entries with: */
    /* AUTODOC:   MAC 00:00:00:00:00:01, VLAN 2, srcId 1, port 0 */
    /* AUTODOC:   MAC 00:00:00:00:00:02, VLAN 2, srcId 0, port 1 */
    for (portIter = 0; portIter < PRV_TGF_FDB_MAC_ENTRIES_NUM_CNS; portIter++ )
    {
        /* clear entry */
        cpssOsMemSet(&macEntry, 0, sizeof(macEntry));

        /* fill mac entry by defaults */
        macEntry.key.entryType              = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
        macEntry.key.key.macVlan.vlanId         = PRV_TGF_VLAN_ID_CNS;
        macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
        macEntry.dstInterface.devPort.hwDevNum    = prvTgfDevNum;
        macEntry.dstInterface.devPort.portNum   = prvTgfPortsArray[portIter];

        macEntry.daCommand  = PRV_TGF_PACKET_CMD_FORWARD_E;
        macEntry.saCommand  = PRV_TGF_PACKET_CMD_FORWARD_E;
        macEntry.sourceId   = ( 0 == portIter ) ? 1 : 0;

        cpssOsMemSet(&fdbEntryMacAddr, 0, sizeof(fdbEntryMacAddr));
        /* set default MAC addr */
        fdbEntryMacAddr[5] = (GT_U8)(portIter + 1);
        cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
                     fdbEntryMacAddr, sizeof(TGF_MAC_ADDR));

        /* write MAC entry */
        rc = prvTgfBrgFdbMacEntrySet(&macEntry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "ERROR of prvTgfBrgFdbMacEntrySet #%d",
                                     portIter+1);
    }

    PRV_UTF_LOG0_MAC("\nSet Source ID configuration\n");

    /* AUTODOC: set srcID assignment mode PORT_DEFAULT for port 0 */
    rc = prvTgfBrgSrcIdPortAssignModeSet(prvTgfDevNum,
                                         prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                         CPSS_BRG_SRC_ID_ASSIGN_MODE_PORT_DEFAULT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgSrcIdPortAssignModeSet");

    /* store port's default source ID */
    rc = prvTgfBrgSrcIdPortDefaultSrcIdGet(prvTgfDevNum,
                                           prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                           &prvTgfRestoreCfg.portsDefaultSourceId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgSrcIdPortDefaultSrcIdGet");

    /* AUTODOC: set default srcID 2 for port 0 */
    rc = prvTgfBrgSrcIdPortDefaultSrcIdSet(prvTgfDevNum,
                                           prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                           PRV_TGF_SOURCE_ID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgSrcIdPortDefaultSrcIdSet");

    /* AUTODOC: delete port 1 from srcId group 1 */
    /* packets assigned with this source ID will not be forwarded to this port */
    rc = prvTgfBrgSrcIdGroupPortDelete(prvTgfDevNum,
                                       PRV_TGF_DISABLED_SOURCEID_CNS,
                                       prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgSrcIdGroupPortDelete");

    /* AUTODOC: enable srcID egress filter for UC packets for port 1 */
    rc = prvTgfBrgSrcIdPortUcastEgressFilterSet(prvTgfDevNum,
                                                prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS],
                                                GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgSrcIdPortUcastEgressFilterSet");

}

/**
* @internal prvTgfDoNotOverrideFromCpuSourceIdConfigSave function
* @endinternal
*
* @brief   Save configuration to be restored at the end of test
*/
static GT_VOID prvTgfDoNotOverrideFromCpuSourceIdConfigSave
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* save a configuration of a rx-port to be a cascade port */
    rc = prvTgfCscdPortTypeGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS], CPSS_PORT_DIRECTION_RX_E,
                               &prvTgfRestoreCfg.prvTgfSavedRxCscdPortType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeGet");

    /* save a configuration of a tx-port to be a cascade port */
    rc = prvTgfCscdPortTypeGet(prvTgfDevNum,
                                   prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS],
                                   CPSS_PORT_DIRECTION_TX_E,
                                   &prvTgfRestoreCfg.prvTgfSavedTxCscdPortType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeGet");

    /* save  default source id value of rx port */
    rc = prvTgfBrgSrcIdPortDefaultSrcIdGet(prvTgfDevNum,
                                      prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                      &prvTgfRestoreCfg.prvTgfSavedRxDefSrcId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgSrcIdPortDefaultSrcIdGet");

    /* save a ForceSrcId state of rx port */
    rc = prvTgfBrgSrcIdPortSrcIdForceEnableGet(prvTgfDevNum,
                                      prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                      &prvTgfRestoreCfg.prvTgfSavedRxSrcIdForceEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgSrcIdPortSrcIdForceEnableGet");

    /* force link up for all ports */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

}

/**
* @internal prvTgfDoNotOverrideFromCpuSourceIdConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfDoNotOverrideFromCpuSourceIdConfigSet
(
    GT_VOID
)
{
    GT_STATUS                   rc              = GT_OK;
    /* AUTODOC: SETUP CONFIGURATION: */

    PRV_UTF_LOG0_MAC("\n========== Setup config =====================\n\n");
    prvTgfDoNotOverrideFromCpuSourceIdConfigSave();

    PRV_UTF_LOG0_MAC("\nSet Vlan configuration\n");
    /* AUTODOC: create VLAN 2 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLAN_ID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "Error of prvTgfBrgDefVlanEntryWrite");
    PRV_UTF_LOG0_MAC("\nSet Source ID configuration\n");

    /* Enable ForceSrcId on rx port to avoid override of srcId by FDB */
    rc = prvTgfBrgSrcIdPortSrcIdForceEnableSet(prvTgfDevNum,
                            prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgSrcIdPortSrcIdForceEnableSet");

    /* store port's default source ID */
    rc = prvTgfBrgSrcIdPortDefaultSrcIdGet(prvTgfDevNum,
                                           prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                           &prvTgfRestoreCfg.portsDefaultSourceId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgSrcIdPortDefaultSrcIdGet");

    /* AUTODOC: set default srcID 2 for port 0 */
    rc = prvTgfBrgSrcIdPortDefaultSrcIdSet(prvTgfDevNum,
                                           prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                           PRV_TGF_SOURCE_ID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgSrcIdPortDefaultSrcIdSet");

    /* set rx/tx-port to be a cascade port */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_DIRECTION_INGRESS_E, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                               CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_DIRECTION_EGRESS_E, prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS],
                               CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");

}

/**
* @internal prvTgfFdbSourceIdModeDefaultTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfFdbSourceIdModeDefaultTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS   rc             = GT_OK;
    GT_U32      portIter       = 0;
    GT_U32      expPacketLen   = 0;
    GT_U32      numTriggers    = 0;
    GT_U32      rxPacketsCount = 0;
    GT_U32      txPacketsCount = 0;

    GT_U32      stageNum       = 0;

    CPSS_INTERFACE_INFO_STC     portInterface;
    TGF_VFD_INFO_STC            vfdArray[2];

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* AUTODOC: Iterate thru 2 stages: */
    for (stageNum = 0; stageNum < 2; stageNum++ )
    {
        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficTableRxPcktTblClear");

        /* AUTODOC: for Stage#2 do the following: */
        if (stageNum == 1)
        {
            /* AUTODOC:   add port 1 to srcId group 1 */
            /* packets assigned with this source ID may be forwarded to this port */
            rc = prvTgfBrgSrcIdGroupPortAdd(prvTgfDevNum,
                                           PRV_TGF_DISABLED_SOURCEID_CNS,
                                           prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS]);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgSrcIdGroupPortAdd");

            /* AUTODOC:   delete port 1 from srcId group 2 */
            /* packets assigned with this source ID will not be forwarded to this port */
            rc = prvTgfBrgSrcIdGroupPortDelete(prvTgfDevNum,
                                               PRV_TGF_SOURCE_ID_CNS,
                                               prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS]);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgSrcIdGroupPortDelete");
        }

        /* reset all counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersReset");

        /* enable capture */
        portInterface.type              = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_CPORT_IDX_CNS];

        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
                                                    TGF_CAPTURE_MODE_MIRRORING_E,
                                                    GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                     "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet\n");

        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum,
                                 &prvTgfPacketInfo,
                                 prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "ERROR of SetTxSetupEth: dev=%d, burst=%d",
                                     prvTgfDevNum, prvTgfBurstCount);

        /* set Tx delay */
        prvTgfSetTxSetup2Eth(1, PRV_TGF_TX_DELAY);

        /* AUTODOC: send 5 packets on port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:00:02, SA=00:00:00:00:00:01, VID=2 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                                       prvTgfPortsArray[PRV_TGF_TX_PORTIDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "ERROR of StartTransmitingEth: dev=%d, port=%d",
                                     prvTgfDevNum,
                                     prvTgfPortsArray[PRV_TGF_TX_PORTIDX_CNS]);

        /* remove Tx delay */
        prvTgfSetTxSetup2Eth(0, 0);

        /* disable capture */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
                                                    TGF_CAPTURE_MODE_MIRRORING_E,
                                                    GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum,
                                     portInterface.devPort.portNum);

        /* AUTODOC: verify traffic: */
        /* AUTODOC:   stage#1 - 5 packets on port 1 */
        /* AUTODOC:   stage#2 - no traffic */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            /* expected number of Tx packets */
            txPacketsCount = prvTgfPacketsCountRxTxArr[1 + (stageNum*2)][portIter];

            /* expected number of Rx packets */
            rxPacketsCount = prvTgfPacketsCountRxTxArr[0 + (stageNum*2)][portIter]+
                             ((PRV_TGF_CPORT_IDX_CNS == portIter) * txPacketsCount);

            expPacketLen = ( PRV_TGF_TX_PORTIDX_CNS == portIter ) ?
                /* prvTgfFdbSourceIdModeDefaultIsMcMember(portIter) ? */
                            PRV_TGF_PACKET_LEN_CNS :
                            PRV_TGF_PACKET_LEN_CNS - TGF_VLAN_TAG_SIZE_CNS;

            /* check ETH counters */
            rc = prvTgfEthCountersCheck(prvTgfDevNum, prvTgfPortsArray[portIter],
                                        rxPacketsCount, txPacketsCount, expPacketLen,
                                        prvTgfBurstCount);
            UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                                         "ERROR of prvTgfEthCountersCheck"\
                                         "  Port=%02d  expectedRx=%02d"\
                                         "  expectedTx=%02d"\
                                         "  expectedLen=%02d\n",
                                         prvTgfPortsArray[portIter],
                                         rxPacketsCount, txPacketsCount,
                                         expPacketLen);
        }

        /* get trigger counters */
        PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", portInterface.devPort.portNum);

        /* get trigger counters where packet has MAC DA as macAddr */
        vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[0].modeExtraInfo = 0;
        vfdArray[0].offset = 0;
        vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
        cpssOsMemCpy(vfdArray[0].patternPtr,
                     prvTgfPacketL2Part.daMac,
                     sizeof(TGF_MAC_ADDR));
        /* get trigger counters where packet has MAC SA as macAddr */
        vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[1].modeExtraInfo = 0;
        vfdArray[1].offset = sizeof(TGF_MAC_ADDR);
        vfdArray[1].cycleCount = sizeof(TGF_MAC_ADDR);
        cpssOsMemCpy(vfdArray[1].patternPtr,
                     prvTgfPacketL2Part.saMac,
                     sizeof(TGF_MAC_ADDR));
        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface,
                                                            sizeof(vfdArray) / sizeof(vfdArray[0]),
                                                            vfdArray,
                                                            &numTriggers);
        PRV_UTF_LOG1_MAC("numTriggers = %d\n", numTriggers);

        /* check TriggerCounters */
        rc = rc == GT_NO_MORE ? GT_OK : rc;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "ERROR of tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                     portInterface.devPort.hwDevNum,
                                     portInterface.devPort.portNum);

        /* check if captured packet has the same MAC DA as prvTgfArpMac */
        PRV_UTF_LOG1_MAC("expTriggers = %d\n\n", prvTgfExpTriggersArr[stageNum]);

        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfExpTriggersArr[stageNum], numTriggers,
                                     "\n ERROR: Got another MAC DA/SA\n");
    }
}

/**
* internal tgfDoNotOverrideFromCpuSourceIdVerification function
* @endinternal
*
* @brief   DoNotOverrideFromCpuSourceId use case verification
*/
static GT_VOID tgfDoNotOverrideFromCpuSourceIdVerification
(
    GT_U32 iter
)
{
    GT_STATUS                           rc = GT_OK;
    GT_U8                               packetBuf[140] = {0};
    GT_U32                              packetLen = 140;
    GT_BOOL                             getFirst = GT_TRUE;
    GT_U32                              packetActualLength = 0;
    GT_U8                               queue = 0;
    GT_U8                               dev = 0;
    TGF_NET_DSA_STC                     rxParam;
    GT_U32                              pktOffset = TGF_L2_HEADER_SIZE_CNS;
    GT_U32                              receivedSrdId = 0, expectedSrdId = 0;

    PRV_UTF_LOG0_MAC("\nPacket at Egress interface\n");
    rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&egressPortInterface,
            TGF_PACKET_TYPE_CAPTURE_E,
            getFirst, GT_TRUE, packetBuf,
            &packetLen, &packetActualLength,
            &dev, &queue, &rxParam);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet: egrPortInterface %d", prvTgfDevNum);

    UTF_VERIFY_EQUAL2_STRING_MAC(0x1/*FROM_CPU*/, (packetBuf[pktOffset] >> 6) & 0x3,
            "DSA Tag Command [%d]is different than expected[%d]\n", (packetBuf[pktOffset] >> 6) & 0x3, 0x1);

    pktOffset += 4;
    receivedSrdId = (((packetBuf[pktOffset]) & 0x1) << 4) | ((packetBuf[pktOffset + 1] >> 4) & 0xF);
    expectedSrdId = (iter == 0) ? PRV_TGF_SOURCE_ID_CNS : prvTgfDsaInfo_fromCpu.srcId;
    UTF_VERIFY_EQUAL2_STRING_MAC(expectedSrdId, receivedSrdId,
            "DSA Tag FROM_CPU SrcId [%d]is different than expected[%d]\n", receivedSrdId, expectedSrdId);
}

/**
* @internal prvTgfDoNotOverrideFromCpuSourceIdTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfDoNotOverrideFromCpuSourceIdTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS   rc                = GT_OK;
    GT_U32      portIter          = 0;
    GT_U32      expPacketLen      = 0;
    GT_U32      rxPacketsCount    = 0;
    GT_U32      txPacketsCount    = 0;
    GT_U32      prvTgfExpTriggers = 3;
    GT_U32      stageNum          = 0;
    GT_U32      numTriggers       = 0;
    GT_U32      prvTgfSavedHwDevNum;
    TGF_VFD_INFO_STC            vfdArray[2];

    /* AUTODOC: GENERATE TRAFFIC: */

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    rc = prvUtfHwDeviceNumberGet(prvTgfDevNum, &prvTgfSavedHwDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberGet");

    /* AUTODOC: Iterate thru 2 stages: */
    for (stageNum = 0; stageNum < 2; stageNum++ )
    {
        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficTableRxPcktTblClear");

        /* AUTODOC: for Stage#2 do the following: */
        if (stageNum == 1)
        {
            /* AUTODOC:   add port 1 to srcId group 1 */
            /* packets assigned with this source ID may be forwarded to this port */
            rc = prvTgfBrgSrcIdGroupPortAdd(prvTgfDevNum,
                                           PRV_TGF_DISABLED_SOURCEID_CNS,
                                           prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS]);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgSrcIdGroupPortAdd");

            /* AUTODOC:   delete port 1 from srcId group 2 */
            /* packets assigned with this source ID will not be forwarded to this port */
            rc = prvTgfBrgSrcIdGroupPortDelete(prvTgfDevNum,
                                               PRV_TGF_SOURCE_ID_CNS,
                                               prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS]);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgSrcIdGroupPortDelete");

            rc = prvTgfBrgSrcIdNotOverrideFromCpuEnableGet(prvTgfDevNum, &prvTgfRestoreCfg.enableRestore);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgSrcIdNotOverrideFromCpuEnableGet");

            rc = prvTgfBrgSrcIdNotOverrideFromCpuEnableSet(prvTgfDevNum, GT_TRUE);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgSrcIdNotOverrideFromCpuEnableSet");

            /* AUTODOC: set default srcID 2 for port 0 */
            rc = prvTgfBrgSrcIdPortDefaultSrcIdSet(prvTgfDevNum,
                                                   prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS],
                                                   PRV_TGF_SOURCE_ID_CNS + 3);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgSrcIdPortDefaultSrcIdSet");
        }

        /* reset all counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersReset");

        /* enable capture */
        egressPortInterface.type              = CPSS_INTERFACE_PORT_E;
        egressPortInterface.devPort.hwDevNum  = prvTgfDevNum;
        egressPortInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS];

        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&egressPortInterface,
                                                    TGF_CAPTURE_MODE_MIRRORING_E,
                                                    GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                     "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet\n");

        prvTgfPacketDsaTagPart.commonParams.vid                              = prvTgfPacketVlanTagPart.vid;
        prvTgfPacketDsaTagPart.commonParams.cfiBit                           = 0;
        prvTgfDsaInfo_fromCpu.dstInterface.vlanId                            = prvTgfPacketVlanTagPart.vid;
        prvTgfDsaInfo_fromCpu.dstInterface.vidx                              = 0;
        prvTgfDsaInfo_fromCpu.dstEport                                       = prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS];
        prvTgfDsaInfo_fromCpu.isTrgPhyPortValid                              = GT_TRUE;
        prvTgfDsaInfo_fromCpu.dstInterface.devPort.hwDevNum                  = prvTgfDevNum;
        prvTgfDsaInfo_fromCpu.dstInterface.devPort.portNum                   = prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS];

        prvTgfDsaInfo_fromCpu.tc                                             = 0;
        prvTgfDsaInfo_fromCpu.srcId                                          = PRV_TGF_SOURCE_ID_CNS + 3;
        prvTgfDsaInfo_fromCpu.srcHwDev                                       = prvTgfSavedHwDevNum;

        prvTgfPacketDsaTagPart.dsaInfo.fromCpu.extDestInfo.devPort.dstIsTagged          = GT_TRUE;
        prvTgfPacketDsaTagPart.dsaInfo.fromCpu.extDestInfo.devPort.mailBoxToNeighborCPU = GT_FALSE;

        /* bind the DSA tag FROM_CPU part at run time because a union is used */
        prvTgfPacketDsaTagPart.dsaInfo.fromCpu = prvTgfDsaInfo_fromCpu;

        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum,
                                 &prvTgfCascadePacketInfo,
                                 prvTgfDsaBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "ERROR of SetTxSetupEth: dev=%d, burst=%d",
                                     prvTgfDevNum, prvTgfDsaBurstCount);

        /* set Tx delay */
        prvTgfSetTxSetup2Eth(1, PRV_TGF_TX_DELAY);

        /* AUTODOC: send 5 packets on port 0 with: */
        /* AUTODOC:   DA=00:00:00:00:00:02, SA=00:00:00:00:00:01, VID=2 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                                       prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "ERROR of StartTransmitingEth: dev=%d, port=%d",
                                     prvTgfDevNum,
                                     prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);

        /* remove Tx delay */
        prvTgfSetTxSetup2Eth(0, 0);

        /* disable capture */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&egressPortInterface,
                                                    TGF_CAPTURE_MODE_MIRRORING_E,
                                                    GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum,
                                     egressPortInterface.devPort.portNum);

        /* AUTODOC: verify traffic: */
        /* AUTODOC:   stage#1 - 5 packets on port 1 */
        /* AUTODOC:   stage#2 - no traffic */
        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            if (portIter == PRV_TGF_EGRESS_PORT_IDX_CNS || portIter == PRV_TGF_INGRESS_PORT_IDX_CNS)
            {
                /* expected number of Tx packets */
                txPacketsCount = 1;

                /* expected number of Rx packets */
                rxPacketsCount = 1;

                expPacketLen = PRV_TGF_CASCADE_PACKET_SIZE_CNS;

                /* check ETH counters */
                rc = prvTgfEthCountersCheck(prvTgfDevNum, prvTgfPortsArray[portIter],
                                            rxPacketsCount, txPacketsCount, expPacketLen,
                                            prvTgfDsaBurstCount);
                UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc,
                                             "ERROR of prvTgfEthCountersCheck"\
                                             "  Port=%02d  expectedRx=%02d"\
                                             "  expectedTx=%02d"\
                                             "  expectedLen=%02d\n",
                                             prvTgfPortsArray[portIter],
                                             rxPacketsCount, txPacketsCount,
                                             expPacketLen);
            }
        }

        /* get trigger counters */
        PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", egressPortInterface.devPort.portNum);

        /* get trigger counters where packet has MAC DA as macAddr */
        vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[0].modeExtraInfo = 0;
        vfdArray[0].offset = 0;
        vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
        cpssOsMemCpy(vfdArray[0].patternPtr,
                     prvTgfPacketL2Part.daMac,
                     sizeof(TGF_MAC_ADDR));
        /* get trigger counters where packet has MAC SA as macAddr */
        vfdArray[1].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray[1].modeExtraInfo = 0;
        vfdArray[1].offset = sizeof(TGF_MAC_ADDR);
        vfdArray[1].cycleCount = sizeof(TGF_MAC_ADDR);
        cpssOsMemCpy(vfdArray[1].patternPtr,
                     prvTgfPacketL2Part.saMac,
                     sizeof(TGF_MAC_ADDR));
        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&egressPortInterface,
                                                            sizeof(vfdArray) / sizeof(vfdArray[0]),
                                                            vfdArray,
                                                            &numTriggers);
        PRV_UTF_LOG1_MAC("numTriggers = %d\n", numTriggers);

        /* check TriggerCounters */
        rc = rc == GT_NO_MORE ? GT_OK : rc;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "ERROR of tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                     egressPortInterface.devPort.hwDevNum,
                                     egressPortInterface.devPort.portNum);

        /* check if captured packet has the same MAC DA as prvTgfArpMac */
        PRV_UTF_LOG1_MAC("expTriggers = %d\n\n", prvTgfExpTriggers);

        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfExpTriggers, numTriggers,
                                     "\n ERROR: Got another MAC DA/SA\n");
        /* AUTODOC: Verification Started */
        tgfDoNotOverrideFromCpuSourceIdVerification(stageNum);
    }

}

/**
* @internal prvTgfFdbSourceIdModeDefaultConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfFdbSourceIdModeDefaultConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc          = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("\n========== Restore config: ==================\n\n");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: disable srcID egress filter for UC packets on port 1 */
    rc = prvTgfBrgSrcIdPortUcastEgressFilterSet(prvTgfDevNum,
                                                prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS],
                                                GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgSrcIdPortUcastEgressFilterSet");

    /* AUTODOC: restore srcId group 2 */
    rc = prvTgfBrgSrcIdGroupPortAdd(prvTgfDevNum,
                                    PRV_TGF_SOURCE_ID_CNS,
                                    prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgSrcIdGroupPortAdd");

    /* AUTODOC: restore default source ID on port 0 */
    rc = prvTgfBrgSrcIdPortDefaultSrcIdSet(prvTgfDevNum,
                                           prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                           prvTgfRestoreCfg.portsDefaultSourceId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgSrcIdPortDefaultSrcIdSet");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbFlush");

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLAN_ID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfBrgDefVlanEntryInvalidate: vlan %d",
                                 PRV_TGF_VLAN_ID_CNS);

}

/**
* @internal prvTgfDoNotOverrideFromCpuSourceIdConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfDoNotOverrideFromCpuSourceIdConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc          = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("\n========== Restore config: ==================\n\n");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: disable srcID egress filter for UC packets on port 1 */
    rc = prvTgfBrgSrcIdPortUcastEgressFilterSet(prvTgfDevNum,
                                                prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS],
                                                GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgSrcIdPortUcastEgressFilterSet");

    /* AUTODOC: restore srcId group 2 */
    rc = prvTgfBrgSrcIdGroupPortAdd(prvTgfDevNum,
                                    PRV_TGF_SOURCE_ID_CNS,
                                    prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgSrcIdGroupPortAdd");

    /* AUTODOC: restore default source ID on port 0 */
    rc = prvTgfBrgSrcIdPortDefaultSrcIdSet(prvTgfDevNum,
                                           prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                           prvTgfRestoreCfg.portsDefaultSourceId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgSrcIdPortDefaultSrcIdSet");

    /* AUTODOC: restore ForceSrcId on rx port */
    rc = prvTgfBrgSrcIdPortSrcIdForceEnableSet(prvTgfDevNum,
                            prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS], prvTgfRestoreCfg.prvTgfSavedRxSrcIdForceEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgSrcIdPortSrcIdForceEnableSet");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbFlush");

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLAN_ID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfBrgDefVlanEntryInvalidate: vlan %d",
                                 PRV_TGF_VLAN_ID_CNS);

    rc = prvTgfBrgSrcIdNotOverrideFromCpuEnableSet(prvTgfDevNum, prvTgfRestoreCfg.enableRestore);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgSrcIdNotOverrideFromCpuEnableSet");

    /* AUTODOC: restore a cascade configuration of the rx-port */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum,
                               CPSS_PORT_DIRECTION_RX_E,
                               prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                               prvTgfRestoreCfg.prvTgfSavedRxCscdPortType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");


    /* AUTODOC: restore a cascade configuration of the tx-port */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum,
                                   CPSS_PORT_DIRECTION_TX_E,
                                   prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS],
                                   prvTgfRestoreCfg.prvTgfSavedTxCscdPortType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");

}

/**
* @internal prvTgfTreatMllReplicatedAsMcForUcTargetConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfTreatMllReplicatedAsMcForUcTargetConfigSet
(
    GT_VOID
)
{
    GT_STATUS                           rc;
    PRV_TGF_PAIR_READ_WRITE_FORM_ENT    mllPairWriteForm;
    PRV_TGF_L2_MLL_PAIR_STC             mllPairEntry;
    GT_U32                              portsArray[1];
    CPSS_PORTS_BMP_STC                  portsMembers;
    PRV_TGF_L2_MLL_LTT_ENTRY_STC        l2LttEntry;
    PRV_TGF_BRG_MAC_ENTRY_STC           macEntry;

    /* Assign srcId based on FDB DA entry */
    rc = prvTgfBrgSrcIdGlobalSrcIdAssignModeGet(prvTgfDevNum, &prvTgfRestoreCfg.modeRestore);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgSrcIdGlobalSrcIdAssignModeGet");
    rc = prvTgfBrgSrcIdGlobalSrcIdAssignModeSet(CPSS_BRG_SRC_ID_ASSIGN_MODE_FDB_DA_PORT_DEFAULT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgSrcIdGlobalSrcIdAssignModeSet");

    /* Set FDB entry Muxing mode for 12-bits SrcId derivation */
    rc = prvTgfBrgFdbMacEntryMuxingModeGet(&prvTgfRestoreCfg.muxModeRestore);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryMuxingModeGet: %d", prvTgfDevNum);

    rc = prvTgfBrgFdbMacEntryMuxingModeSet(CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_SRC_ID_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryMuxingModeSet: %d", prvTgfDevNum);

    /* create eVidx 1 with single port member port 1 */
    portsArray[0] = prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS];
    /* AUTODOC: create eVIDX 1 with ports [1] */
    rc = prvTgfBrgVidxEntrySet(PRV_TGF_MLL_VIDX_INDEX_CNS, portsArray, NULL, 1);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVidxEntrySet: %d, %d",
                                 prvTgfDevNum, 1);

    /* New ports added: Link up and reset counters */
    prvTgfEthCountersReset(prvTgfDevNum);

    /* AUTODOC: enable MLL lookup for all multi-target packets */
    rc = prvTgfL2MllLookupForAllEvidxEnableGet(prvTgfDevNum, &prvTgfRestoreCfg.eVidxEnableRestore);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllLookupForAllEvidxEnableGet: enable %d",
                                 GT_FALSE);
    rc = prvTgfL2MllLookupForAllEvidxEnableSet(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllLookupForAllEvidxEnableSet: enable %d",
                                 GT_FALSE);

    /* create L2 MLL LTT entries */
    rc = prvTgfL2MllLttEntryGet(prvTgfDevNum, PRV_TGF_MLL_LTT_INDEX_CNS, &prvTgfRestoreCfg.lttEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntryGet: index 0");

    cpssOsMemSet(&l2LttEntry, 0, sizeof(PRV_TGF_L2_MLL_LTT_ENTRY_STC));

    l2LttEntry.mllPointer = PRV_TGF_MLL_PAIR_ENTRY_INDEX_CNS;
    l2LttEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;
    /* no mask profile - replicate to all:  */
    /* port 1                               */
    /* vidx 1 - ports 1                     */
    l2LttEntry.mllMaskProfileEnable = GT_FALSE;
    l2LttEntry.mllMaskProfile       = 0;

    /* AUTODOC: add L2 MLL LTT entry 0 with: */
    /* AUTODOC:   mllPointer=20, mllMaskProfileEnable=False, mllMaskProfile=0 */
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, PRV_TGF_MLL_LTT_INDEX_CNS, &l2LttEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet: index 0");

    /* AUTODOC: Store L2 MLL entry */
    rc = prvTgfL2MllPairRead(prvTgfDevNum, PRV_TGF_MLL_PAIR_ENTRY_INDEX_CNS,
                             PRV_TGF_PAIR_READ_WRITE_WHOLE_E,
                             &prvTgfRestoreCfg.mllPairEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairRead: %d", prvTgfDevNum);

    /* create L2 MLL entries */
    cpssOsMemSet(&mllPairEntry, 0, sizeof(PRV_TGF_L2_MLL_PAIR_STC));

    mllPairWriteForm = PRV_TGF_PAIR_READ_WRITE_WHOLE_E;
    mllPairEntry.nextPointer = 1;
    mllPairEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;

    /* AUTODOC:   nextMllPointer=1, maskBitmap=0x11, port=9 */
    mllPairEntry.firstMllNode.last = GT_FALSE;
    mllPairEntry.firstMllNode.maskBitmap = 0x0;
    mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
    mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    mllPairEntry.firstMllNode.egressInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS];

    /* AUTODOC:   maskBitmap=0x16, vidx=1 */
    mllPairEntry.secondMllNode.last = GT_TRUE;
    mllPairEntry.secondMllNode.maskBitmap = 0x0;
    mllPairEntry.secondMllNode.egressInterface.type = CPSS_INTERFACE_VIDX_E;
    mllPairEntry.secondMllNode.egressInterface.vidx = PRV_TGF_MLL_VIDX_INDEX_CNS;

    /* AUTODOC: add L2 MLL entry 0: */
    rc = prvTgfL2MllPairWrite(prvTgfDevNum, PRV_TGF_MLL_PAIR_ENTRY_INDEX_CNS, mllPairWriteForm, &mllPairEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: index 0");

    /* AUTODOC: create VLAN 2 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLAN_ID_CNS);

    /* add a MAC entry */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    macEntry.key.entryType                  = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId         = PRV_TGF_VLAN_ID_CNS;
    macEntry.dstInterface.type              = CPSS_INTERFACE_VIDX_E;
    macEntry.dstInterface.vidx              = PRV_TGF_MLL_LTT_INDEX_CNS;
    macEntry.isStatic                       = GT_TRUE;
    macEntry.daCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.saCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.daRoute                        = GT_FALSE;
    macEntry.sourceId                       = PRV_TGF_SOURCE_ID_INDEX_CNS;
    macEntry.userDefined                    = 0;

    /* AUTODOC: add FDB entry with MAC 01:00:00:00:00:22, prvtgfVlanId 2 and vidx 8 */
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite: %d", prvTgfDevNum);

    /* Set port bitmap entry in source id egress filtering table */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    rc = cpssDxChBrgSrcIdGroupEntryGet(prvTgfDevNum, PRV_TGF_SOURCE_ID_INDEX_CNS, &prvTgfRestoreCfg.cpuSrcIdMemberRestore, &prvTgfRestoreCfg.portsMembers);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChBrgSrcIdGroupEntryGet");
    rc = cpssDxChBrgSrcIdGroupEntrySet(prvTgfDevNum, PRV_TGF_SOURCE_ID_INDEX_CNS, GT_FALSE, &portsMembers);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChBrgSrcIdGroupEntrySet");

    /* AUTODOC: disable srcID egress filter for UC packets on port 1 */
    rc = prvTgfBrgSrcIdPortUcastEgressFilterGet(prvTgfDevNum,
                                                prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS],
                                                &prvTgfRestoreCfg.enableRestore);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgSrcIdPortUcastEgressFilterGet");

    rc = prvTgfBrgSrcIdPortUcastEgressFilterSet(prvTgfDevNum,
                                                prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS],
                                                GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgSrcIdPortUcastEgressFilterSet");
}

/**
* @internal prvTgfTreatMllReplicatedAsMcForUcTargetTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfTreatMllReplicatedAsMcForUcTargetTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS   rc                = GT_OK;
    CPSS_INTERFACE_INFO_STC       portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_U32      txPacketsCount    = 0;
    GT_U32      portIter          = 0;
    GT_U32      stageNum          = 0;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC: Iterate thru 2 stages: */
    for (stageNum = 0; stageNum < 2; stageNum++ )
    {
        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficTableRxPcktTblClear");
        /* AUTODOC: for Stage#2 do the following: */
        if (stageNum == 1)
        {
            rc = prvTgfBrgSrcIdTreatMllReplicatedAsMcEnableGet(prvTgfDevNum, &prvTgfRestoreCfg.mllReplicatedRestore);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgSrcIdTreatMllReplicatedAsMcEnableGet");

            rc = prvTgfBrgSrcIdTreatMllReplicatedAsMcEnableSet(prvTgfDevNum, GT_TRUE);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgSrcIdTreatMllReplicatedAsMcEnableSet");
        }

        /* reset all counters */
        rc = prvTgfEthCountersReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfEthCountersReset");

        /* enable capture */
        portInterface.type              = CPSS_INTERFACE_PORT_E;
        portInterface.devPort.hwDevNum  = prvTgfDevNum;
        portInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_CPORT_IDX_CNS];

        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
                                                    TGF_CAPTURE_MODE_MIRRORING_E,
                                                    GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                     "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet\n");

        /* AUTODOC: send Ethernet packet from port 0 with: */
        /* AUTODOC:   DA=01:00:00:00:00:22, SA=00:00:00:00:00:99, VID=2 */
        prvTgfL2MllUtilsPacketSend(&prvTgfPacketInfo, 1, PRV_TGF_INGRESS_PORT_IDX_CNS);

        /* disable capture */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
                                                    TGF_CAPTURE_MODE_MIRRORING_E,
                                                    GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                     prvTgfDevNum,
                                     portInterface.devPort.portNum);

        for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
        {
            /* read counters */
            rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);

            switch(portIter)
            {
                case 0: /* port 0 */ txPacketsCount = 1;
                        break;
                case 1: /* port 1 */ txPacketsCount = (stageNum == 1) ? 0 : 1;
                        break;
                default: txPacketsCount = 0;
                         break;
            }

            UTF_VERIFY_EQUAL2_STRING_MAC(txPacketsCount, portCntrs.goodPktsSent.l[0],
                                         "get another Tx packets than expected on port[%d] = %d",
                                         portIter, prvTgfPortsArray[portIter]);
        }
    }
}

/**
* @internal prvTgfTreatMllReplicatedAsMcForUcTargetConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfTreatMllReplicatedAsMcForUcTargetConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc          = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */

    PRV_UTF_LOG0_MAC("\n========== Restore config: ==================\n\n");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbFlush");

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLAN_ID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfBrgDefVlanEntryInvalidate: vlan %d",
                                 PRV_TGF_VLAN_ID_CNS);

    /* Assign srcId based on FDB DA entry */
    rc = prvTgfBrgSrcIdGlobalSrcIdAssignModeSet(prvTgfRestoreCfg.modeRestore);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgSrcIdGlobalSrcIdAssignModeSet");

    /* Set FDB entry Muxing mode for 12-bits SrcId derivation */
    rc = prvTgfBrgFdbMacEntryMuxingModeSet(prvTgfRestoreCfg.muxModeRestore);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryMuxingModeSet: %d", prvTgfDevNum);

    /* AUTODOC: disable MLL lookup for all multi-target packets */
    rc = prvTgfL2MllLookupForAllEvidxEnableSet(prvTgfDevNum, prvTgfRestoreCfg.eVidxEnableRestore);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllLookupForAllEvidxEnableSet: enable %d",
                                 GT_FALSE);

    /* create L2 MLL LTT entries */
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, PRV_TGF_MLL_LTT_INDEX_CNS, &prvTgfRestoreCfg.lttEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntryGet: index 0");

    /* AUTODOC: Restore L2 MLL entry */
    rc = prvTgfL2MllPairWrite(prvTgfDevNum, PRV_TGF_MLL_PAIR_ENTRY_INDEX_CNS,
                             PRV_TGF_PAIR_READ_WRITE_WHOLE_E,
                             &prvTgfRestoreCfg.mllPairEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: %d", prvTgfDevNum);

    /* AUTODOC: disable srcID egress filter for UC packets on port 1 */
    rc = prvTgfBrgSrcIdPortUcastEgressFilterSet(prvTgfDevNum,
                                                prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS],
                                                prvTgfRestoreCfg.enableRestore);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgSrcIdPortUcastEgressFilterSet");

    /* AUTODOC: Restore Source-ID egress filtering for MLL replicated packets */
    rc = prvTgfBrgSrcIdTreatMllReplicatedAsMcEnableSet(prvTgfDevNum, prvTgfRestoreCfg.mllReplicatedRestore);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgSrcIdTreatMllReplicatedAsMcEnableSet");

    /* Set port bitmap entry in source id egress filtering table */
    rc = cpssDxChBrgSrcIdGroupEntrySet(prvTgfDevNum, PRV_TGF_SOURCE_ID_INDEX_CNS, prvTgfRestoreCfg.cpuSrcIdMemberRestore, &prvTgfRestoreCfg.portsMembers);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChBrgSrcIdGroupEntryGet");
}


