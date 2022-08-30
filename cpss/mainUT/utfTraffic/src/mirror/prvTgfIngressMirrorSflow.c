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
* @file prvTgfIngressMirrorSflow.c
*
* @brief Ingress SFLOW use cases testing.
*        Supporting SFLOW V5 use case
*
*        Ports allocation:
*           ^ Port#1: Analyzer port
*           ^ Port#2: Target port
*           ^ Port#3: Ingress port
*
*        Packet flow:
*
* @version   1
********************************************************************************
*/
#include <mirror/prvTgfIngressMirrorSflow.h>
#include <oam/prvTgfOamGeneral.h>
#include <common/tgfL2MllGen.h>
#include <cpss/dxCh/dxChxGen/pha/cpssDxChPha.h>
#include <cpss/dxCh/dxChxGen/oam/cpssDxChOam.h>
#include <common/tgfConfigGen.h>
#include <common/tgfBridgeL2EcmpGen.h>
#include <common/tgfMirror.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgE2Phy.h>
#include <cpss/dxCh/dxChxGen/pha/cpssDxChPha.h>
#include <common/tgfCscdGen.h>

/*************************** Constants definitions ***********************************/

/* PHA fw thread IDs of */ 
#define PRV_TGF_PHA_THREAD_ID_SFLOW_V5_IPV4_CNS   57
#define PRV_TGF_PHA_THREAD_ID_SFLOW_V5_IPV6_CNS   58


/* Template profile index */
#define PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS  1

/* Tunnel start entry index */
#define PRV_TGF_TUNNEL_START_IPV4_INDEX_CNS      10
#define PRV_TGF_TUNNEL_START_IPV6_INDEX_CNS      20

/* MLL pair entry index */
#define PRV_TGF_MLL_PAIR_ENTRY_INDEX_CNS    12
#define PRV_TGF_MLL_LTT_INDEX_CNS           1 
#define PRV_TGF_EVIDX_CNS                   (PRV_TGF_MLL_LTT_INDEX_CNS + 4096) 

/* default VLAN0 Id */
#define PRV_TGF_VLANID_CNS  5

/*SFLOW UDP port number*/
#define PRV_TGF_SFLOW_V5_UDP_PORT_CNS 6343
/* Target LC device number */
#define TARGET_LC_DEVICE_NUMBER_CNS                    16

/* port number to receive traffic from*/
#define PRV_TGF_TX_PORT_IDX_CNS        0
#define PRV_TGF_ANALYZER_PORT_IDX_CNS  1
#define PRV_TGF_ANALYZER_PORT2_IDX_CNS 2 
#define PRV_TGF_SERVICE_PORT_IDX_CNS   3

#define PRV_TGF_DEFAULT_EPORT_CNS     1024 
#define PRV_TGF_ANALYZER_EPORT_1_CNS  1025 
#define PRV_TGF_ANALYZER_EPORT_2_CNS  1026 

/* Packet flow Id to set by PCL Action  */
#define PRV_TGF_EPCL_FLOW_ID_CNS            25
#define PRV_TGF_EPCL_IPV4_RULE_INDEX_CNS    0
#define PRV_TGF_EPCL_IPV6_RULE_INDEX_CNS    10
/*analyzer index for mirroring sflow */
#define PRV_TGF_SFLOW_MIRROR_IDX_CNS 2

/*sampling rate*/
#define PRV_TGF_SFLOW_MIRROR_RATE_CNS 1

/* Billing flow Id based counting index base  */
#define PRV_TGF_BILLING_COUNTERS_FLOW_ID_INDEX_BASE_CNS 10

/* Billing flow Id minimal for counting  */
#define PRV_TGF_BILLING_COUNTERS_FLOW_ID_MIN_CNS        20

/* Billing flow Id maximal for counting  */
#define PRV_TGF_BILLING_COUNTERS_FLOW_ID_MAX_CNS        50

/*SFLOW V5 offsets*/
#define PRV_TGF_SFLOW_V5_IPV6_OFFSET_CNS (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV6_HEADER_SIZE_CNS + TGF_UDP_HEADER_SIZE_CNS )
#define PRV_TGF_SFLOW_V5_IPV4_OFFSET_CNS (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS + TGF_UDP_HEADER_SIZE_CNS )
/*Agent ip offset from start of SFLOW header*/
#define PRV_TGF_SFLOW_V5_AGENT_OFFSET_CNS 8
/*standard SFLOW header len 28B*/
#define PRV_TGF_SFLOW_V5_HDR_LEN_CNS 28
/*Sample Data header (Data format + Sample Length) len 8B*/
#define PRV_TGF_SFLOW_V5_SD_HDR_LEN_CNS 8
/*Sampling rate offset from start of flow sample header*/
#define PRV_TGF_SFLOW_V5_RATE_OFFSET_CNS 8
/*Flow sample header len 28B*/
#define PRV_TGF_SFLOW_V5_FS_LEN_CNS 28
/*Standard Enterprise data format type*/
#define PRV_TGF_SFLOW_V5_DATA_FORMAT_CNS 0x01

/*Reserved eth type*/
#define PRV_TGF_SFLOW_RESERVED_ETHER_TYPE_CNS 0xFFFF
/*Remote vlan tag for LB port*/
#define PRV_TGF_SFLOW_REMOTE_VLAN_CNS 100
#define PRV_TGF_INGRESS_EPORT_CNS PRV_TGF_SFLOW_REMOTE_VLAN_CNS

#define PRV_TGF_TTI0_INDEX_CNS prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(0, 3)
/*#define PRV_TGF_TTI0_INDEX_CNS 3*/

/* Billing flow Id based counting index  */
#define PRV_TGF_BILLING_COUNTERS_FLOW_ID_INDEX_CNS   \
    (PRV_TGF_BILLING_COUNTERS_FLOW_ID_INDEX_BASE_CNS \
     + PRV_TGF_EPCL_FLOW_ID_CNS                      \
     - PRV_TGF_BILLING_COUNTERS_FLOW_ID_MIN_CNS)

/*Agent IPv4 address per collector*/
static GT_U8 sflowV5AgentIpv4AddrCollector1[4] = {41, 42, 43, 44};
static GT_U8 sflowV5AgentIpv4AddrCollector2[4] = {61, 62, 63, 64};
/*TTI keytype*/
static CPSS_DXCH_TTI_KEY_TYPE_ENT  keyType = CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E;
/* PCL UDB configuration */
typedef struct{
    GT_U32                               udbIndex;
    CPSS_DXCH_PCL_OFFSET_TYPE_ENT        offsetType;
    GT_U8                                offset;
    GT_U8                                byteMask;
}prvTgfPclUdbsflowV5;

/* parameters that are needed to be restored */
static struct
{
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC      interface;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC           egressInfo;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC           egressInfo2;
    GT_BOOL                                    samplingEn;
    GT_U32                                     samplingRatio;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC           brgEgressInfo;
    CPSS_INTERFACE_INFO_STC                    phyPortInfo;
    CPSS_INTERFACE_INFO_STC                    phyPortInfo2;
    PRV_TGF_OAM_COMMON_CONFIG_STC              oamConfig;
    GT_BOOL                                    oamEnStatus;
    PRV_TGF_POLICER_STAGE_METER_MODE_ENT       stageModeSave;
    GT_BOOL                                    meteringCountingStatus;
    GT_BOOL                                    lmStampingEnable;
    GT_BOOL                                    rxMirrorEnable;
    GT_BOOL                                    rxMirrorIdx;
    CPSS_DXCH_PHA_FW_IMAGE_ID_ENT              phaFwImageId;
    PRV_TGF_L2_MLL_LTT_ENTRY_STC               lttEntry;
    PRV_TGF_L2_MLL_PAIR_STC                    mllPairEntry;
    GT_BOOL                                    ttiEnable;
    GT_BOOL                                    phaEnable2;
    GT_BOOL                                    phaEnable;
    GT_U32                                     phaThreadId;
    GT_U32                                     phaThreadId2;
    GT_BOOL                                    analyzerVlanTagEnable;
    CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC analyzerVlanTagConfig;
    GT_U16                                     pvid;
    PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT pclPortAccessModeCfgGet;
    CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT pclTsAccessModeCfgGet;
    PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT pclPortAccessModeCfgGet1;
    CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT pclTsAccessModeCfgGet1;
    PRV_TGF_TTI_OFFSET_TYPE_ENT                 ttiAnchor[2];
    GT_U8                                       ttiOffset[2];
    CPSS_DXCH_TTI_KEY_SIZE_ENT                  ttiKeySize;
    GT_U32                                      phyPortBase;
    PRV_TGF_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT phyPortAssignMode;
    GT_U32                                      vlanTagTpid;
} prvTgfRestoreCfg;

/*flag for PHA init status*/
static GT_BOOL phaInitDone = GT_FALSE;
static prvTgfPclUdbsflowV5 prvTgfsflowV5EpclMetadataUdbInfo[] = {

     {0 , CPSS_DXCH_PCL_OFFSET_METADATA_E, 53, 0xFE},   /* Analyzer ePort[0:6] */
     {1 , CPSS_DXCH_PCL_OFFSET_METADATA_E, 54, 0x3F},   /* Analyzer ePort[12:7] */
     {2 , CPSS_DXCH_PCL_OFFSET_METADATA_E, 57, 0x20},   /* Analyzer ePort[13] */
     {3 , CPSS_DXCH_PCL_OFFSET_METADATA_E, 64, 0x03},   /* egress marvell tagged[0:1] TO_ANALYZER */

     /* must be last */
     {4, CPSS_DXCH_PCL_OFFSET_INVALID_E , 0, 0}
};
/* Mirrored packet, Tunnel part */
/* L2 part */
static TGF_PACKET_L2_STC prvTgfMirroredPacketTunnelL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x44, 0x04},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x45, 0x05}                /* saMac */
};

/* L3 part */
/* IPv4*/
static TGF_PACKET_IPV4_STC prvTgfMirroredPacketTunnelIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x3f,               /* totalLen */
    0,                  /* id */
    1,                  /* flags */
    0,                  /* offset */
    0x2,                /* timeToLive */
    0x04,               /* protocol */
    0x5EA0,             /* csum */
    {20,  1,  1,  3},   /* srcAddr */
    {20,  1,  1,  2}    /* dstAddr */
};

/* IPv6 */
static TGF_PACKET_IPV6_STC prvTgfMirroredPacketTunnelIpv6Part =
{
    6,                                       /* version */
    0,                                       /* trafficClass */
    0,                                       /* flowLabel */
    0x02,                                    /* payloadLen */
    0x3b,                                    /* nextHeader */
    0x40,                                    /* hopLimit */
    {0x6545, 0, 0, 0, 0, 0, 0x0000, 0x3212}, /* TGF_IPV6_ADDR srcAddr */
    {0xff02, 0, 0, 0, 0, 0, 0xccdd, 0xeeff}  /* TGF_IPV6_ADDR dstAddr */
};

/* Original Ingress Packet */
/* L2 part */
static TGF_PACKET_L2_STC prvTgfOriginalPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x03}                /* saMac */
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
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* EtherType part (for untagged packet) = lentgh of the packet's payload */
static TGF_PACKET_ETHERTYPE_STC prvTgfPackeEthertypePart =
{
    sizeof(prvTgfPayloadDataArr)
};
/* parts of the original packet */
static TGF_PACKET_PART_STC prvTgfOriginalPacketPartsArray[] = {
     {TGF_PACKET_PART_L2_E,        &prvTgfOriginalPacketL2Part},
     {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPackeEthertypePart},
     {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
 };
/* final original packet info */
static TGF_PACKET_STC prvTgfOriginalPacketInfo = {
    TGF_L2_HEADER_SIZE_CNS+TGF_ETHERTYPE_SIZE_CNS+sizeof(prvTgfPayloadDataArr), /* totalLen */
    sizeof(prvTgfOriginalPacketPartsArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
    prvTgfOriginalPacketPartsArray                                       /* partsArray */
};


/*----------------------------------------------------------------------------*/

/**
* @internal prvTgfIngresssflowV5EpclConfigSet function
* @endinternal
*
* @brief   EPCL configurations
*           Enable analyzer port for EPCL trigger
*           Enable port mode for config table
*           10B UDB keys with eport, egr mtag cmd = TO_ANALYZER
*           Action = trigger PHA thread, flow based OAM
*/
static GT_VOID prvTgfIngresssflowV5EpclConfigSet
(
    GT_U32 protocol
)
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
            prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_EGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclDefPortInit");

    /* AUTODOC: enable EPCL on Analyzer port */
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TO_ANALYZER_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: Configure ePort access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeGet(prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           &prvTgfRestoreCfg.pclPortAccessModeCfgGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeGet");

    rc = prvTgfPclPortLookupCfgTabAccessModeSet(prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

    /* AUTODOC: Configure "Access to the EPCL configuration" table with Port Mode */
    rc = cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeGet(prvTgfDevNum,
                                &prvTgfRestoreCfg.pclTsAccessModeCfgGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet");

    rc = cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet(prvTgfDevNum,
                                CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet");

    cpssOsMemSet(&udbSelect, 0, sizeof(CPSS_DXCH_PCL_UDB_SELECT_STC));
    udbSelectidx = 0;

    while(1)
    {
        if(prvTgfsflowV5EpclMetadataUdbInfo[udbSelectidx].offsetType == CPSS_DXCH_PCL_OFFSET_INVALID_E)
            break;

        /* AUTODOC:   configure User Defined Byte(UDB) */
        /* AUTODOC:   format CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E */
        /* AUTODOC:   packet type PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E */
        /* AUTODOC:   offsetType CPSS_DXCH_PCL_OFFSET_METADATA_E */
        /* AUTODOC:   offset 53 Analyzer ePort[0:6] */
        /* AUTODOC:   offset 54 Analyzer ePort[12:7] */
        /* AUTODOC:   offset 57 Analyzer ePort[13] */
        /* AUTODOC:   offset 64 egress marvell tagged[1:0] */
        rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                        CPSS_PCL_DIRECTION_EGRESS_E,
                                        prvTgfsflowV5EpclMetadataUdbInfo[udbSelectidx].udbIndex,
                                        prvTgfsflowV5EpclMetadataUdbInfo[udbSelectidx].offsetType,
                                        prvTgfsflowV5EpclMetadataUdbInfo[udbSelectidx].offset);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d for udbSelectidx = %d\n", prvTgfDevNum, udbSelectidx);

        udbSelect.udbSelectArr[udbSelectidx]=prvTgfsflowV5EpclMetadataUdbInfo[udbSelectidx].udbIndex;
        udbSelectidx++;
    }

    rc = prvTgfPclUserDefinedBytesSelectSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E, PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                           CPSS_PCL_LOOKUP_0_E, &udbSelect);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedBytesSelectSet: %d\n", prvTgfDevNum);
    /* AUTODOC: set PCL rule 0 */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    /* bits 1:7 is ePort[0:6] */
    pattern.ruleEgrUdbOnly.udb[0] = (PRV_TGF_DEFAULT_EPORT_CNS&0x7f) <<1;
    mask.ruleEgrUdbOnly.udb[0] = prvTgfsflowV5EpclMetadataUdbInfo[0].byteMask;

    /* bits 0:5 is ePort[12:7] */
    pattern.ruleEgrUdbOnly.udb[1] = ((PRV_TGF_DEFAULT_EPORT_CNS>>7)&0x3f);
    mask.ruleEgrUdbOnly.udb[1] = prvTgfsflowV5EpclMetadataUdbInfo[1].byteMask;

    /* bits 5 is ePort[13] */
    pattern.ruleEgrUdbOnly.udb[2] = (PRV_TGF_DEFAULT_EPORT_CNS>>8)&0x20;
    mask.ruleEgrUdbOnly.udb[2] = prvTgfsflowV5EpclMetadataUdbInfo[2].byteMask;

    /*egress pkt cmd*/
    pattern.ruleEgrUdbOnly.udb[3] = 0x02;
    mask.ruleEgrUdbOnly.udb[3] = prvTgfsflowV5EpclMetadataUdbInfo[3].byteMask;

    ruleIndex                                    = 0;
    ruleFormat                                   = PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E;
    action.pktCmd                                = CPSS_PACKET_CMD_FORWARD_E;
    action.oam.oamProcessEnable                  = GT_TRUE;
    action.oam.oamProfile                        = PRV_TGF_OAM_PROFILE_INDEX_CNS;
    action.flowId                                = PRV_TGF_EPCL_FLOW_ID_CNS;
    action.policer.policerEnable                 = PRV_TGF_PCL_POLICER_ENABLE_COUNTER_ONLY_E;
    action.egressPolicy                          = GT_TRUE;
    action.epclPhaInfo.phaThreadIdAssignmentMode = CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENABLED_E;
    action.epclPhaInfo.phaThreadId               = (protocol == CPSS_IP_PROTOCOL_IPV4_E) ? PRV_TGF_PHA_THREAD_ID_SFLOW_V5_IPV4_CNS : PRV_TGF_PHA_THREAD_ID_SFLOW_V5_IPV6_CNS;
    action.epclPhaInfo.phaThreadType             = PRV_TGF_PCL_ACTION_EGRESS_PHA_THREAD_SFLOW_V5_MIRROR_E;
    action.epclPhaInfo.phaThreadUnion.epclSflowV5Mirror.sflowSamplingRate = PRV_TGF_SFLOW_MIRROR_RATE_CNS;
    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclRuleSet, prvTgfDevNum:%d,"
                                            "ruleFormat:%d, ruleIndex:%d", prvTgfDevNum, ruleFormat, ruleIndex);
}
/**
* @internal prvTgfIngresssflowV5EpclConfigReset function
* @endinternal
*
* @brief   EPCL configurations restore
*           Restore port config
*           invalidate EPCL rule
*/
static GT_VOID prvTgfIngresssflowV5EpclConfigReset()
{
    GT_STATUS rc;
    /* -------------------------------------------------------------------------
     * Restore EPCL Configuration
     */
    /* AUTODOC: Invalidate PCL rule 0 */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_10_BYTES_E, 0, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclRuleValidStatusSet, index = %d", 0);

    /* AUTODOC: disable all PCL configurations on the ports */
    prvTgfPclPortsRestoreAll();

    /* AUTODOC: disable EPCL on Analyzer port per packet type */
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TO_ANALYZER_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TS_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");
}
/**
* @internal prvTgfIngresssflowV5IpTsConfigReset function
* @endinternal
*
* @brief   Tunnel start configurations reset
*           invalidate TS rule based on TS type IPv4/v6
*/
static GT_VOID prvTgfIngresssflowV5IpTsConfigReset(GT_U32 tunnelIdx, CPSS_IP_PROTOCOL_STACK_ENT protocol)
{
    GT_STATUS rc;
    PRV_TGF_TUNNEL_START_ENTRY_UNT      tunnelEntry;

    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));

    rc = prvTgfTunnelStartEntrySet(tunnelIdx, protocol == CPSS_IP_PROTOCOL_IPV4_E? CPSS_TUNNEL_GENERIC_IPV4_E: CPSS_TUNNEL_GENERIC_IPV6_E, &tunnelEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartEntrySet");
}
/**
* @internal prvTgfIngresssflowV5Ipv4TsConfig function
* @endinternal
*
* @brief   TS configurations for IPv4 tunnel
*           Set Tunnel Start entry 8 with:
*           tunnelType = Generic IPv4, tagEnable = TRUE, vlanId=5, ipHeaderProtocol = UDP, udp port = 6343 
*           MACDA = 00:00:00:00:44:04 , DIP = 20.1.1.2, SIP=20.1.1.3
*           Tunnel start profile with 16B zeros
*           
*/
static GT_VOID prvTgfIngresssflowV5Ipv4TsConfig()
{
    GT_STATUS rc;
    PRV_TGF_TUNNEL_START_ENTRY_UNT      tunnelEntry;
    PRV_TGF_TUNNEL_START_GEN_IP_PROFILE_STC         profileData;

    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));
    tunnelEntry.ipv4Cfg.tagEnable              = GT_TRUE;
    tunnelEntry.ipv4Cfg.vlanId                 = PRV_TGF_VLANID_CNS;
    tunnelEntry.ipv6Cfg.ttl                    = 20;
    tunnelEntry.ipv4Cfg.ipHeaderProtocol       = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
    tunnelEntry.ipv4Cfg.udpDstPort             = PRV_TGF_SFLOW_V5_UDP_PORT_CNS;
    tunnelEntry.ipv4Cfg.udpSrcPort             = PRV_TGF_SFLOW_V5_UDP_PORT_CNS;
    tunnelEntry.ipv4Cfg.profileIndex           = PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS;
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.macDa.arEther, prvTgfMirroredPacketTunnelL2Part.daMac,    sizeof(TGF_MAC_ADDR));
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.destIp.arIP, prvTgfMirroredPacketTunnelIpv4Part.dstAddr, sizeof(TGF_IPV4_ADDR));
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.srcIp.arIP, prvTgfMirroredPacketTunnelIpv4Part.srcAddr, sizeof(TGF_IPV4_ADDR));
    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_IPV4_INDEX_CNS, CPSS_TUNNEL_GENERIC_IPV4_E, &tunnelEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartEntrySet");

    /* AUTODOC: configure tunnel start profile for Sflow v5 (16 Bytes as "Zero") */
    cpssOsMemSet(&profileData,0,sizeof(profileData));
    profileData.templateDataSize = PRV_TGF_TUNNEL_START_TEMPLATE_DATA_SIZE_16B_E;
    rc = prvTgfTunnelStartGenProfileTableEntrySet(prvTgfDevNum,
                                                  PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS, &profileData);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartGenProfileTableEntrySet");
}
/**
* @internal prvTgfIngresssflowV5Ipv6TsConfig function
* @endinternal
*
* @brief   TS configurations for IPv4 tunnel
*           Set Tunnel Start entry 8 with:
*           tunnelType = Generic IPv4, tagEnable = TRUE, vlanId=5, ipHeaderProtocol = UDP, udp port = 6343 
*           MACDA = 00:00:00:00:44:04 , DIP = 65:45::32:12, SIP=ff:02::cc:dd:ee:ff
*           Tunnel start profile with 16B zeros
*           
*/
static GT_VOID prvTgfIngresssflowV5Ipv6TsConfig()
{
    GT_STATUS rc;
    PRV_TGF_TUNNEL_START_ENTRY_UNT      tunnelEntry;
    PRV_TGF_TUNNEL_START_GEN_IP_PROFILE_STC         profileData;

    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));
    tunnelEntry.ipv6Cfg.tagEnable              = GT_TRUE;
    tunnelEntry.ipv6Cfg.vlanId                 = PRV_TGF_VLANID_CNS;
    tunnelEntry.ipv6Cfg.ipHeaderProtocol       = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
    tunnelEntry.ipv6Cfg.profileIndex           = PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS;
    tunnelEntry.ipv6Cfg.udpDstPort             = PRV_TGF_SFLOW_V5_UDP_PORT_CNS;
    tunnelEntry.ipv6Cfg.udpSrcPort             = PRV_TGF_SFLOW_V5_UDP_PORT_CNS;
    cpssOsMemCpy(tunnelEntry.ipv6Cfg.macDa.arEther, prvTgfMirroredPacketTunnelL2Part.daMac,    sizeof(TGF_MAC_ADDR));
    cpssOsMemCpy(tunnelEntry.ipv6Cfg.destIp.arIP, prvTgfMirroredPacketTunnelIpv6Part.dstAddr, sizeof(TGF_IPV6_ADDR));
    cpssOsMemCpy(tunnelEntry.ipv6Cfg.srcIp.arIP, prvTgfMirroredPacketTunnelIpv6Part.srcAddr, sizeof(TGF_IPV6_ADDR));
    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_IPV6_INDEX_CNS, CPSS_TUNNEL_GENERIC_IPV6_E, &tunnelEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartEntrySet");

    /* AUTODOC: configure tunnel start profile for Sflow v5 (16 Bytes as "Zero") */
    cpssOsMemSet(&profileData,0,sizeof(profileData));
    profileData.templateDataSize = PRV_TGF_TUNNEL_START_TEMPLATE_DATA_SIZE_16B_E;
    rc = prvTgfTunnelStartGenProfileTableEntrySet(prvTgfDevNum,
                                                  PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS, &profileData);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartGenProfileTableEntrySet");
}
/**
* @internal prvTgfIngresssflowV5MirrorConfig function
* @endinternal
*
* @brief   Ingress mirror config
*           Enable source port mirroring for RX port and Set analyzer index to 1
*           map analyzer index to eport, and set sampling ratio
*           map eport to phy port#2 and enable TS for the eport
*           
*/
static GT_VOID prvTgfIngresssflowV5MirrorConfig(CPSS_IP_PROTOCOL_STACK_ENT protocol)
{

    GT_STATUS rc;
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC interface;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC     egressInfo;
    CPSS_INTERFACE_INFO_STC phyPortInfo;

    /* AUTODOC: Save analyzer interface */
    rc = prvTgfMirrorAnalyzerInterfaceGet(prvTgfDevNum, PRV_TGF_SFLOW_MIRROR_IDX_CNS, &prvTgfRestoreCfg.interface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceGet: %d", prvTgfDevNum);

    /* AUTODOC: set analyzer interface index=1: */
    /* AUTODOC:   analyzer devNum=0, eport = 1024 , phy port=2 */
    interface.interface.type = CPSS_INTERFACE_PORT_E;
    interface.interface.devPort.hwDevNum  = prvTgfDevNum;
    interface.interface.devPort.portNum = PRV_TGF_DEFAULT_EPORT_CNS;
    rc = prvTgfMirrorAnalyzerInterfaceSet(PRV_TGF_SFLOW_MIRROR_IDX_CNS, &interface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceSet: %d", PRV_TGF_SFLOW_MIRROR_IDX_CNS);

    /* AUTODOC: Save ePort to phyport configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                                      PRV_TGF_DEFAULT_EPORT_CNS, 
                                                       &(prvTgfRestoreCfg.phyPortInfo));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortTargetMappingTableGet");
    phyPortInfo.type = CPSS_INTERFACE_PORT_E;
    phyPortInfo.devPort.hwDevNum = prvTgfDevNum;
    phyPortInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS];
    prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                       PRV_TGF_DEFAULT_EPORT_CNS, &phyPortInfo );
    /*Set sampling ratio*/
    rc = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet(prvTgfDevNum, PRV_TGF_SFLOW_MIRROR_IDX_CNS, &prvTgfRestoreCfg.samplingEn, &prvTgfRestoreCfg.samplingRatio);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet");
    rc = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet(prvTgfDevNum, PRV_TGF_SFLOW_MIRROR_IDX_CNS, GT_TRUE, PRV_TGF_SFLOW_MIRROR_RATE_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet");
    /* AUTODOC: Set analyzer ePort#1 attributes configuration */
    /* AUTODOC: Tunnel Start = ENABLE, tunnelStartPtr = 8, tsPassenger = ETHERNET */
    /*Get eport attribute config*/
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                                                       PRV_TGF_DEFAULT_EPORT_CNS,
                                                       &prvTgfRestoreCfg.egressInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoGet");
    cpssOsMemSet(&egressInfo,0,sizeof(egressInfo));
    egressInfo = prvTgfRestoreCfg.egressInfo;
    egressInfo.tunnelStart            = GT_TRUE;
    egressInfo.tunnelStartPtr         = protocol == CPSS_IP_PROTOCOL_IPV4_E? PRV_TGF_TUNNEL_START_IPV4_INDEX_CNS : PRV_TGF_TUNNEL_START_IPV6_INDEX_CNS;
    egressInfo.tsPassengerPacketType  = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       PRV_TGF_DEFAULT_EPORT_CNS,
                                                       &egressInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoSet");
}
/**
* @internal prvTgfIngresssflowV5MirrorConfigReset function
* @endinternal
*
* @brief   Ingress mirror config restore
*           Disable mirroring for rx port
*           Restore eport config, and e2phy map
*           
*/
static GT_VOID prvTgfIngresssflowV5MirrorConfigReset()
{

    GT_STATUS rc;

    /* AUTODOC: Restore analyzer interface from index 1 */
    rc = prvTgfMirrorAnalyzerInterfaceSet(PRV_TGF_SFLOW_MIRROR_IDX_CNS, &prvTgfRestoreCfg.interface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceSet: %d", PRV_TGF_SFLOW_MIRROR_IDX_CNS);

    /* AUTODOC: Restore ePort attributes configuration */
    prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                       PRV_TGF_DEFAULT_EPORT_CNS, &prvTgfRestoreCfg.phyPortInfo );
    /*Restore sampling ratio*/
    rc = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet(prvTgfDevNum, PRV_TGF_SFLOW_MIRROR_IDX_CNS, prvTgfRestoreCfg.samplingEn, prvTgfRestoreCfg.samplingRatio);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet");

    /* AUTODOC: Restore analyzer ePort#1 attributes configuration */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       PRV_TGF_DEFAULT_EPORT_CNS,
                                                       &prvTgfRestoreCfg.brgEgressInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoSet");
}
/**
* @internal prvTgfIngresssflowV5PhaConfig function
* @endinternal
*
* @brief   PHA thread config 
*           Set the FW thread id 57/58 with SFLOW IPv4/v6 thread id
*           Set the thread memory with IPv4 agent address
*           Set the thread memory with sflow data format
*           
*/
static GT_VOID prvTgfIngresssflowV5PhaConfig(CPSS_IP_PROTOCOL_STACK_ENT protocol)
{
    GT_STATUS rc;

    /* AUTODOC: Set the thread entry */
    CPSS_DXCH_PHA_THREAD_INFO_UNT            extInfo;
    CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC     commonInfo;

    /*load FW img 02*/
    if (!phaInitDone)
    {
        rc = cpssDxChPhaFwImageIdGet(prvTgfDevNum, &prvTgfRestoreCfg.phaFwImageId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaFwImageIdGet");
        rc = cpssDxChPhaInit(prvTgfDevNum, GT_TRUE, CPSS_DXCH_PHA_FW_IMAGE_ID_02_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaInit");
        phaInitDone = GT_TRUE;
    }

    extInfo.sflowV5Mirror.sflowDataFormat = PRV_TGF_SFLOW_V5_DATA_FORMAT_CNS;
    cpssOsMemSet(&commonInfo, 0, sizeof(commonInfo));
    commonInfo.statisticalProcessingFactor = 0;
    commonInfo.busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;
    commonInfo.stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;
    if(protocol == CPSS_IP_PROTOCOL_IPV4_E)
    {
        extInfo.sflowV5Mirror.sflowAgentIpAddr.arIP[0] = sflowV5AgentIpv4AddrCollector1[0];
        extInfo.sflowV5Mirror.sflowAgentIpAddr.arIP[1] = sflowV5AgentIpv4AddrCollector1[1];
        extInfo.sflowV5Mirror.sflowAgentIpAddr.arIP[2] = sflowV5AgentIpv4AddrCollector1[2];
        extInfo.sflowV5Mirror.sflowAgentIpAddr.arIP[3] = sflowV5AgentIpv4AddrCollector1[3];
        rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
                PRV_TGF_PHA_THREAD_ID_SFLOW_V5_IPV4_CNS,
                &commonInfo,
                CPSS_DXCH_PHA_THREAD_TYPE_SFLOW_V5_IPV4_E,
                &extInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaThreadIdEntrySet");
    }
    else if (protocol == CPSS_IP_PROTOCOL_IPV6_E)
    {
        extInfo.sflowV5Mirror.sflowAgentIpAddr.arIP[0] = sflowV5AgentIpv4AddrCollector2[0];
        extInfo.sflowV5Mirror.sflowAgentIpAddr.arIP[1] = sflowV5AgentIpv4AddrCollector2[1];
        extInfo.sflowV5Mirror.sflowAgentIpAddr.arIP[2] = sflowV5AgentIpv4AddrCollector2[2];
        extInfo.sflowV5Mirror.sflowAgentIpAddr.arIP[3] = sflowV5AgentIpv4AddrCollector2[3];
        rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
                PRV_TGF_PHA_THREAD_ID_SFLOW_V5_IPV6_CNS,
                &commonInfo,
                CPSS_DXCH_PHA_THREAD_TYPE_SFLOW_V5_IPV6_E,
                &extInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaThreadIdEntrySet");
    }
}
/**
* @internal prvTgfFalconMirrorIngressSflowEoamConfigSet function
* @endinternal
*
* @brief   EOAM config 
*            <OAM Opcode Parsing En>=0
*            <LM Counter Capture En>=1
*            <LM Counting Mode>= Enable Counting
*            <OAM PTP Offset Index>=0
*           
*/
static GT_VOID  prvTgfFalconMirrorIngressSflowEoamConfigSet
(
    GT_U32 oamEntryIndex
)
{
    GT_STATUS                       rc = GT_OK;
    PRV_TGF_OAM_COMMON_CONFIG_STC   oamConfig;
    GT_U32           oamIndex = 1;

    PRV_UTF_LOG0_MAC("======= Setting EOAM Configuration =======\n");

    cpssOsMemSet(&oamConfig, 0, sizeof(oamConfig));
    oamConfig.oamEntryIndex = oamEntryIndex;

    /* Get OAM common configuration and save it for restortion */
    prvTgfOamConfigurationGet(&oamConfig);
    cpssOsMemCpy(&(prvTgfRestoreCfg.oamConfig), &oamConfig, sizeof(oamConfig));

    /* AUTODOC: Set common OAM configurations */
    oamConfig.oamEntry.oamPtpOffsetIndex      = 0;
    oamConfig.oamEntry.opcodeParsingEnable    = GT_FALSE;
    oamConfig.oamEntry.lmCountingMode         = PRV_TGF_OAM_LM_COUNTING_MODE_ENABLE_E;
    oamConfig.oamEntry.lmCounterCaptureEnable = GT_TRUE;
    prvTgfOamConfigurationSet(&oamConfig);

    /* AUTODOC: Set OAM LM offset table */
    rc = prvTgfOamLmOffsetTableSet(oamIndex, PRV_TGF_OAM_TLV_DATA_OFFSET_STC + 4*oamIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfOamLmOffsetTableSet");

    /* AUTODOC: save OAM Enabler status */
    rc = prvTgfOamEnableGet(PRV_TGF_OAM_STAGE_TYPE_EGRESS_E, &prvTgfRestoreCfg.oamEnStatus);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfOamEnableGet");

    /* AUTODOC: Enable OAM processing - Egress */
    rc = prvTgfOamEnableSet(PRV_TGF_OAM_STAGE_TYPE_EGRESS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfOamEnableSet");
}
/**
* @internal prvTgfFalconMirrorIngressSflowEplrConfigSet function
* @endinternal
*
* @brief   EPLR test configurations
*           Global<Billing Index Mode> = FlowID
*            <LM Counter Capture Mode>=Insert
*            <Billing Color Counting Mode>=0x1 (ALL)
*/
static GT_VOID prvTgfFalconMirrorIngressSflowEplrConfigSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_POLICER_BILLING_INDEX_CFG_STC   billingIndexCfg;
    PRV_TGF_POLICER_BILLING_ENTRY_STC       prvTgfBillingCntr;

    PRV_UTF_LOG0_MAC("======= Setting EPLR Configuration =======\n");

    /* Save stage mode */
    rc = prvTgfPolicerStageMeterModeGet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E, &prvTgfRestoreCfg.stageModeSave);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeGet: %d",
                                 PRV_TGF_POLICER_STAGE_EGRESS_E);
    /* AUTODOC: test works in Flow mode */
    rc = prvTgfPolicerStageMeterModeSet(PRV_TGF_POLICER_STAGE_EGRESS_E, PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeSet: %d",
                                 PRV_TGF_POLICER_STAGE_EGRESS_E);

    /* AUTODOC: Set Egress policer counter based on Flow Id */
    cpssOsMemSet(&billingIndexCfg, 0, sizeof(billingIndexCfg));
    billingIndexCfg.billingIndexMode       = PRV_TGF_POLICER_BILLING_INDEX_MODE_FLOW_ID_E;
    billingIndexCfg.billingFlowIdIndexBase = PRV_TGF_BILLING_COUNTERS_FLOW_ID_INDEX_BASE_CNS;
    billingIndexCfg.billingMinFlowId       = PRV_TGF_BILLING_COUNTERS_FLOW_ID_MIN_CNS;
    billingIndexCfg.billingMaxFlowId       = PRV_TGF_BILLING_COUNTERS_FLOW_ID_MAX_CNS;
    rc = prvTgfPolicerFlowIdCountingCfgSet(PRV_TGF_POLICER_STAGE_EGRESS_E, &billingIndexCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPolicerFlowIdCountingCfgSet");

    /* AUTODOC: Flush internal Write Back Cache (WBC) of counting entries */
    rc = prvTgfPolicerCountingWriteBackCacheFlush(PRV_TGF_POLICER_STAGE_EGRESS_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPolicerCountingWriteBackCacheFlush");

    /* AUTODOC: Configure counters mode */
    cpssOsMemSet(&prvTgfBillingCntr, 0, sizeof(prvTgfBillingCntr));
    prvTgfBillingCntr.billingCntrMode = PRV_TGF_POLICER_BILLING_CNTR_PACKET_E;
    prvTgfBillingCntr.packetSizeMode  = CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;
    prvTgfBillingCntr.lmCntrCaptureMode = PRV_TGF_POLICER_LM_COUNTER_CAPTURE_MODE_INSERT_E;
    prvTgfBillingCntr.billingCntrAllEnable = GT_TRUE;
    rc = prvTgfPolicerBillingEntrySet(prvTgfDevNum,
                                      PRV_TGF_POLICER_STAGE_EGRESS_E,
                                      PRV_TGF_BILLING_COUNTERS_FLOW_ID_INDEX_CNS,
                                      &prvTgfBillingCntr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfPolicerBillingEntrySet,prvTgfDevNum:%d", prvTgfDevNum);

    /* AUTODOC: Set LM counter capture enable */
    rc = prvTgfPolicerLossMeasurementCounterCaptureEnableSet(PRV_TGF_POLICER_STAGE_EGRESS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPolicerLossMeasurementCounterCaptureEnableSet");

    /* Get Egress to analyzer metering and counting status for restortion */
    rc = cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableGet(prvTgfDevNum,
                                        &prvTgfRestoreCfg.meteringCountingStatus);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableGet");

    /* AUTODOC: Enable Metering and counting for "TO_ANALYZER" packets */
    rc = cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableSet(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableSet");
}
/**
* @internal prvTgfFalconMirrorIngressSflowErmrkConfigSet function
* @endinternal
*
* @brief   ERMRK test configurations
*           <LM Stamping En> is disabled, as the LM counter is inserted by the PHA
*/
static GT_VOID prvTgfFalconMirrorIngressSflowErmrkConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    PRV_UTF_LOG0_MAC("======= Setting ERMRK Configuration =======\n");

    /* AUTODOC: save current state of LM stamping counter for restortion */
    rc = cpssDxChOamLmStampingEnableGet(prvTgfDevNum, &prvTgfRestoreCfg.lmStampingEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChOamLmStampingEnableGet");

    /* AUTODOC: Disable Stamping of LM counter in packet (since fw does it instead) */
    rc = cpssDxChOamLmStampingEnableSet(prvTgfDevNum, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChOamLmStampingEnableSet");

}
/**
* @internal prvTgfIngresssflowV5EoamConfigReset function
* @endinternal
*
* @brief   EOAM test configurations restore
*/
static GT_VOID prvTgfIngresssflowV5EoamConfigReset()
{
    GT_STATUS                           rc = GT_OK;
    /* -------------------------------------------------------------------------
     * Restore EOAM Configuration
     */
    /* AUTODOC: restore common EOAM configurations */
    prvTgfOamConfigurationSet(&prvTgfRestoreCfg.oamConfig);

    /* AUTODOC: restore OAM processing - Egress */
    rc = prvTgfOamEnableSet(PRV_TGF_OAM_STAGE_TYPE_EGRESS_E, prvTgfRestoreCfg.oamEnStatus);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfOamEnableSet");
}

/**
* @internal prvTgfIngresssflowV5EplrConfigReset function
* @endinternal
*
* @brief   Egress policer test configurations restore
*/
static GT_VOID prvTgfIngresssflowV5EplrConfigReset()
{
    GT_STATUS                           rc = GT_OK;

    /* -------------------------------------------------------------------------
     * Restore EPLR Configuration
     */
    /* AUTODOC: Restore Policer Egress to Analyzer metering and counting status */
    rc = cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableSet(prvTgfDevNum,
                                                prvTgfRestoreCfg.meteringCountingStatus);
    UTF_VERIFY_EQUAL0_STRING_MAC(rc,GT_OK, "Failure in cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableSet");

    /* AUTODOC: Restore stage mode */
    rc = prvTgfPolicerStageMeterModeSet(PRV_TGF_POLICER_STAGE_EGRESS_E, prvTgfRestoreCfg.stageModeSave);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeSet: %d",
                                 prvTgfRestoreCfg.stageModeSave);
} 
/**
* @internal prvTgfIngresssflowV5ErmrkConfigReset function
* @endinternal
*
* @brief   ERMRK test configurations restore
*/
static GT_VOID prvTgfIngresssflowV5ErmrkConfigReset()
{
    GT_STATUS                           rc = GT_OK;
    /* -------------------------------------------------------------------------
     * Restore ERMRK Configuration
     */
    /* AUTODOC: Restore LM Counter status */
    rc = cpssDxChOamLmStampingEnableSet(prvTgfDevNum, prvTgfRestoreCfg.lmStampingEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(rc, GT_OK, "Failure in cpssDxChOamLmStampingEnableSet");
}
/**
* @internal prvTgfFalconMirrorIngressSflowBridgeConfigSet function
* @endinternal
*
* @brief  Bridge config for vlan 5 traffic
*/
static void prvTgfFalconMirrorIngressSflowBridgeConfigSet()
{
    GT_STATUS rc;

    prvTgfBrgVlanEntryStore(1);
    prvTgfBrgVlanMemberRemove(prvTgfDevNum, 1, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);
    prvTgfBrgVlanMemberRemove(prvTgfDevNum, 1, prvTgfPortsArray[PRV_TGF_SERVICE_PORT_IDX_CNS]);
    /* Create VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", PRV_TGF_VLANID_CNS);
    /*remove all, except ingress, remote ports from vlan 5*/
    prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_VLANID_CNS, prvTgfPortsArray[PRV_TGF_ANALYZER_PORT2_IDX_CNS]);
    prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_VLANID_CNS, prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS]);
    prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_VLANID_CNS, prvTgfPortsArray[PRV_TGF_SERVICE_PORT_IDX_CNS]);

    prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], &prvTgfRestoreCfg.pvid);
    prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], PRV_TGF_VLANID_CNS);
}
/**
* @internal prvTgfFalconMirrorIngressSflowBridgeConfigReset function
* @endinternal
*
* @brief  Bridge config for vlan 5 traffic
*/
static void prvTgfFalconMirrorIngressSflowBridgeConfigReset()
{
    GT_STATUS rc;
    prvTgfBrgVlanEntryRestore(1);
    prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    /* Flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);
    prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], prvTgfRestoreCfg.pvid);
}
static void prvTgfIngressSflowV5ValidatePacket(GT_U8* packetBuf, CPSS_IP_PROTOCOL_STACK_ENT protocol)
{
    GT_STATUS                           rc = GT_OK;
    GT_U32                  ipHeaderOffset;
    GT_U32                  samplingRate;
    GT_U32                  dataFormat;
    GT_U32                  offset;
    GT_U8*                  ipAddr;
    /* Components to be verified
    1. Sampling rate in SFLOW header
    2. Agent ip in SFLOW
    3. Data Format in SFLOW*/
    if (protocol == CPSS_IP_PROTOCOL_IPV4_E)
    {
        ipHeaderOffset = PRV_TGF_SFLOW_V5_IPV4_OFFSET_CNS;
        ipAddr = sflowV5AgentIpv4AddrCollector1;
     }
    else
    {
        ipHeaderOffset = PRV_TGF_SFLOW_V5_IPV6_OFFSET_CNS;
        ipAddr = sflowV5AgentIpv4AddrCollector2;
    }

    offset = ipHeaderOffset + PRV_TGF_SFLOW_V5_HDR_LEN_CNS + PRV_TGF_SFLOW_V5_SD_HDR_LEN_CNS + PRV_TGF_SFLOW_V5_RATE_OFFSET_CNS;
    samplingRate = 
        (packetBuf[offset] << 24) |
        (packetBuf[offset+1] << 16)|
        (packetBuf[offset+2] << 8) |
        (packetBuf[offset+3]);

    UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_SFLOW_MIRROR_RATE_CNS, samplingRate, "Sampling rate mismatch"); 
    

    rc = cpssOsMemCmp(&packetBuf[ipHeaderOffset + PRV_TGF_SFLOW_V5_AGENT_OFFSET_CNS], ipAddr, sizeof(sflowV5AgentIpv4AddrCollector2));
    UTF_VERIFY_EQUAL0_STRING_MAC(0, rc, "Agent IP mismatch");

    offset = ipHeaderOffset + PRV_TGF_SFLOW_V5_HDR_LEN_CNS;
    dataFormat = 
        (packetBuf[offset] << 24) |
        (packetBuf[offset+1] << 16)|
        (packetBuf[offset+2] << 8) |
        (packetBuf[offset+3]);
    UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_SFLOW_V5_DATA_FORMAT_CNS, dataFormat, "data format mismatch"); 
}
/**
* @internal prvTgfFalconMirrorIngressSflowIpv4ConfigurationSet function
* @endinternal
*
* @brief  Test ingress mirroring functionality with sflow v5 encapsulation
*    configure VLAN;
*    configure an analyzer index, sampling rate and eport;
*    enable Rx mirroring for ingress port;
*    E2phy and IPv4 TS for eport
*    Configure OAM, EPLR & ERMRK for other fields of SFLOW header
*    configure EPCL rule to trigger IPv4 PHA thread, and pha metadata
*    send BC traffic and verify expected Sflow mirrored traffic on analyzer port;
*/
void    prvTgfFalconMirrorIngressSflowIpv4ConfigurationSet()
{

    prvTgfFalconMirrorIngressSflowBridgeConfigSet();
    /*mirror port config*/
    prvTgfIngresssflowV5MirrorConfig(CPSS_IP_PROTOCOL_IPV4_E);

    /* AUTODOC: enable Rx mirroring on port=1, index=0 */
    prvTgfMirrorRxPortGet(prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], &prvTgfRestoreCfg.rxMirrorEnable, &prvTgfRestoreCfg.rxMirrorIdx);
    prvTgfMirrorRxPortSet(prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], GT_TRUE, PRV_TGF_SFLOW_MIRROR_IDX_CNS);
    
    /*tunnel start profile*/
    prvTgfIngresssflowV5Ipv4TsConfig();

    /* AUTODOC: EPCL trigger for PHA SFLOW*/ 
    prvTgfIngresssflowV5EpclConfigSet(CPSS_IP_PROTOCOL_IPV4_E);
    
    /* EOAM Configuration */
    prvTgfFalconMirrorIngressSflowEoamConfigSet(PRV_TGF_EPCL_FLOW_ID_CNS);

    /* EPLR Configuration */
    prvTgfFalconMirrorIngressSflowEplrConfigSet();

    /* ERMRK Configuration */
    prvTgfFalconMirrorIngressSflowErmrkConfigSet();

    /*PHA config*/
    prvTgfIngresssflowV5PhaConfig(CPSS_IP_PROTOCOL_IPV4_E);
}

/**
* @internal prvTgfFalconMirrorIngressSflowIpv4ConfigurationReset function
* @endinternal
*
* @brief  Restore ingress mirroring functionality with sflow v5 encapsulation
*/
void    prvTgfFalconMirrorIngressSflowIpv4ConfigurationReset()
{
    GT_STATUS rc;

    prvTgfFalconMirrorIngressSflowBridgeConfigReset();
    prvTgfIngresssflowV5MirrorConfigReset();
    prvTgfMirrorRxPortSet(prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], prvTgfRestoreCfg.rxMirrorEnable, prvTgfRestoreCfg.rxMirrorIdx);
    prvTgfIngresssflowV5EpclConfigReset();
    prvTgfIngresssflowV5IpTsConfigReset(PRV_TGF_TUNNEL_START_IPV4_INDEX_CNS , CPSS_IP_PROTOCOL_IPV4_E);
    prvTgfIngresssflowV5EoamConfigReset();
    prvTgfIngresssflowV5EplrConfigReset();
    prvTgfIngresssflowV5ErmrkConfigReset();
    /*restore PHA FW img*/
    rc = cpssDxChPhaInit(prvTgfDevNum, GT_TRUE, prvTgfRestoreCfg.phaFwImageId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaInit"); 
    phaInitDone = GT_FALSE;
}
/**
* @internal prvTgfFalconMirrorIngressSflowIpv4TrafficSend function
* @endinternal
*
* @brief  Traffic test for ingress mirroring functionality 
*           with sflow v5 IPv4 encapsulation
*           RX port #1
*           TX analyzer port #2
*           Check in the SFLOW header for
*           1. agent IPv4 address
*           2. enterprise format
*           3. sampling ratio
*/
void    prvTgfFalconMirrorIngressSflowIpv4TrafficSend()
{

    CPSS_INTERFACE_INFO_STC  analyzerPortInterface;
    GT_U8                 packetBuf[256];
    GT_U32                packetLen = sizeof(packetBuf);
    GT_U32                packetActualLength = 0;
    GT_U8                 queue = 0;
    GT_U8                 dev = 0;
    GT_BOOL                 getFirst = GT_TRUE;
    TGF_NET_DSA_STC       rxParam;
    GT_STATUS                           rc = GT_OK;

    /* Enable capture on analyzer port */
    analyzerPortInterface.type              = CPSS_INTERFACE_PORT_E;
    analyzerPortInterface.devPort.hwDevNum  = prvTgfDevNum;
    analyzerPortInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&analyzerPortInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfOriginalPacketInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");

    PRV_UTF_LOG0_MAC("======= Send first packet =======\n");
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

    tgfTrafficTableRxPcktTblClear();

    /*verify the packet*/
    prvTgfIngressSflowV5ValidatePacket(packetBuf, CPSS_IP_PROTOCOL_IPV4_E);
}

/**
* @internal prvTgfFalconMirrorIngressSflowIpv6ConfigurationSet function
* @endinternal
*
* @brief Test ingress mirroring functionality with sflow v5 encapsulation
*    configure VLAN;
*    configure an analyzer index, sampling rate and eport;
*    enable Rx mirroring for ingress port;
*    E2phy and IPv6 TS for eport
*    Configure OAM, EPLR & ERMRK for other fields of SFLOW header
*    configure EPCL rule to trigger IPv6 PHA thread, and pha metadata
*    send BC traffic and verify expected Sflow mirrored traffic on analyzer port;
*/
void    prvTgfFalconMirrorIngressSflowIpv6ConfigurationSet()
{
    GT_STATUS rc;

    prvTgfFalconMirrorIngressSflowBridgeConfigSet();
    /*mirror port config*/
    prvTgfIngresssflowV5MirrorConfig(CPSS_IP_PROTOCOL_IPV6_E);

    /* AUTODOC: enable Rx mirroring on port=1, index=0 */
    prvTgfMirrorRxPortGet(prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], &prvTgfRestoreCfg.rxMirrorEnable, &prvTgfRestoreCfg.rxMirrorIdx);
    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], GT_TRUE, PRV_TGF_SFLOW_MIRROR_IDX_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfMirrorRxPortSet: %d %d",
            prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], GT_TRUE);

    /*tunnel start profile*/
    prvTgfIngresssflowV5Ipv6TsConfig();

    /* AUTODOC: EPCL trigger for PHA SFLOW*/ 
    prvTgfIngresssflowV5EpclConfigSet(CPSS_IP_PROTOCOL_IPV6_E);

    /* EOAM Configuration */
    prvTgfFalconMirrorIngressSflowEoamConfigSet(PRV_TGF_EPCL_FLOW_ID_CNS);

    /* EPLR Configuration */
    prvTgfFalconMirrorIngressSflowEplrConfigSet();

    /* ERMRK Configuration */
    prvTgfFalconMirrorIngressSflowErmrkConfigSet();

    /*PHA config*/
    prvTgfIngresssflowV5PhaConfig(CPSS_IP_PROTOCOL_IPV6_E);
}

/**
* @internal prvTgfFalconMirrorIngressSflowIpv6ConfigurationReset function
* @endinternal
*
* @brief  Restore ingress mirroring functionality with sflow v5 encapsulation
*/
void    prvTgfFalconMirrorIngressSflowIpv6ConfigurationReset()
{
    GT_STATUS                           rc = GT_OK;
    prvTgfFalconMirrorIngressSflowBridgeConfigReset();
    prvTgfIngresssflowV5MirrorConfigReset();
    prvTgfMirrorRxPortSet(prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], GT_FALSE, prvTgfRestoreCfg.rxMirrorIdx);
    prvTgfIngresssflowV5EpclConfigReset();
    prvTgfIngresssflowV5IpTsConfigReset(PRV_TGF_TUNNEL_START_IPV6_INDEX_CNS, CPSS_IP_PROTOCOL_IPV6_E);
    prvTgfIngresssflowV5EoamConfigReset();
    prvTgfIngresssflowV5EplrConfigReset();
    prvTgfIngresssflowV5ErmrkConfigReset();
    /*restore PHA FW img*/
    rc = cpssDxChPhaInit(prvTgfDevNum, GT_TRUE, prvTgfRestoreCfg.phaFwImageId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaInit"); 
    phaInitDone = GT_FALSE;
}
/**
* @internal prvTgfFalconMirrorIngressSflowIpv6TrafficSend function
* @endinternal
*
* @brief  Traffic test for ingress mirroring functionality 
*           with sflow v5 IPv6 encapsulation
*           RX port #1
*           TX analyzer port #2
*           Check in the SFLOW header for
*           1. agent IPv4 address
*           2. enterprise format
*           3. sampling ratio
*/
void    prvTgfFalconMirrorIngressSflowIpv6TrafficSend()
{

    CPSS_INTERFACE_INFO_STC  analyzerPortInterface;
    GT_U8                 packetBuf[256];
    GT_U32                packetLen = sizeof(packetBuf);
    GT_U32                packetActualLength = 0;
    GT_U8                 queue = 0;
    GT_U8                 dev = 0;
    GT_BOOL                 getFirst = GT_TRUE;
    TGF_NET_DSA_STC       rxParam;
    GT_STATUS                           rc = GT_OK;

    /* Enable capture on analyzer port */
    analyzerPortInterface.type              = CPSS_INTERFACE_PORT_E;
    analyzerPortInterface.devPort.hwDevNum  = prvTgfDevNum;
    analyzerPortInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&analyzerPortInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfOriginalPacketInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");

    PRV_UTF_LOG0_MAC("======= Send first packet =======\n");
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

    tgfTrafficTableRxPcktTblClear();

    /*verify the packet*/
    prvTgfIngressSflowV5ValidatePacket(packetBuf, CPSS_IP_PROTOCOL_IPV6_E);
}

/*******************************************************************************/
/***********************Multi Target Use case***********************************/
/*******************************************************************************/
/**
* @internal prvTgfIngresssflowV5MultiTargetMirrorConfig function
* @endinternal
*
* @brief   Multi target Sflow pass #1.
*           Ingress mirror config
*           Enable source port mirroring for RX port and Set analyzer index to 1
*           map analyzer index to eport->LB, and set sampling ratio
*           
*/
static GT_VOID prvTgfIngresssflowV5MultiTargetMirrorConfig()
{

    GT_STATUS rc;
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC interface;

    /* AUTODOC: Save analyzer interface from index 2 */
    rc = prvTgfMirrorAnalyzerInterfaceGet(prvTgfDevNum, PRV_TGF_SFLOW_MIRROR_IDX_CNS, &prvTgfRestoreCfg.interface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceGet: %d", prvTgfDevNum);

    /* AUTODOC: set analyzer interface index=1: */
    /* AUTODOC:   analyzer devNum=0, eport = 1024 */
    interface.interface.type = CPSS_INTERFACE_PORT_E;
    interface.interface.devPort.hwDevNum  = prvTgfDevNum;
    interface.interface.devPort.portNum = PRV_TGF_DEFAULT_EPORT_CNS;
    rc = prvTgfMirrorAnalyzerInterfaceSet(PRV_TGF_SFLOW_MIRROR_IDX_CNS, &interface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceSet: %d", PRV_TGF_SFLOW_MIRROR_IDX_CNS);

    /*Set sampling ratio*/
    rc = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet(prvTgfDevNum, PRV_TGF_SFLOW_MIRROR_IDX_CNS, &prvTgfRestoreCfg.samplingEn, &prvTgfRestoreCfg.samplingRatio);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet"); 
    rc = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet(prvTgfDevNum, PRV_TGF_SFLOW_MIRROR_IDX_CNS, GT_TRUE, PRV_TGF_SFLOW_MIRROR_RATE_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet"); 
}
/**
* @internal prvTgfIngresssflowV5MultiTargetMirrorConfigReset function
* @endinternal
*
* @brief   Multi target Sflow pass #1.
*           Ingress mirror config reset
*           Restore analyzer index's interface
*           Restore original sampling ratio           
*/
static GT_VOID prvTgfIngresssflowV5MultiTargetMirrorConfigReset()
{

    GT_STATUS rc;

    /* AUTODOC: Save analyzer interface from index 2 */
    rc = prvTgfMirrorAnalyzerInterfaceSet(PRV_TGF_SFLOW_MIRROR_IDX_CNS, &prvTgfRestoreCfg.interface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceSet: %d", prvTgfDevNum);

    /*Set sampling ratio*/
    rc = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet(prvTgfDevNum, PRV_TGF_SFLOW_MIRROR_IDX_CNS, prvTgfRestoreCfg.samplingEn, prvTgfRestoreCfg.samplingRatio);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet"); 
}
/**
* @internal prvTgfIngresssflowV5MultiTargetTpidConfig function
* @endinternal
*
* @brief   Multi target Sflow pass #1.
*           e2phy for Analyzer eport to LB
*           Enable new vlan Tag for analyzer eport
*           set global RX to analyzer 
*                     eth type to 0xffff 
*                     vid same as remote physical port
*/
static GT_VOID prvTgfIngresssflowV5MultiTargetTpidConfig()
{
    CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC analyzerVlanTagConfig;
    CPSS_INTERFACE_INFO_STC phyPortInfo;
    GT_STATUS rc;
    /* AUTODOC: ePort to phyport configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                                      PRV_TGF_DEFAULT_EPORT_CNS, 
                                                       &(prvTgfRestoreCfg.phyPortInfo));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortTargetMappingTableGet");
    cpssOsMemSet(&phyPortInfo, 0, sizeof(phyPortInfo));
    phyPortInfo.type = CPSS_INTERFACE_PORT_E;
    phyPortInfo.devPort.hwDevNum = prvTgfDevNum;
    phyPortInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_SERVICE_PORT_IDX_CNS];
    prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                       PRV_TGF_DEFAULT_EPORT_CNS, &phyPortInfo );
    /*enable pushing TAG+TPID for TO_ANALYZER pkts*/
    rc = cpssDxChMirrorAnalyzerVlanTagEnableGet(prvTgfDevNum, PRV_TGF_DEFAULT_EPORT_CNS, &prvTgfRestoreCfg.analyzerVlanTagEnable );
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMirrorAnalyzerVlanTagEnableGet"); 
    rc = cpssDxChMirrorAnalyzerVlanTagEnable(prvTgfDevNum, PRV_TGF_DEFAULT_EPORT_CNS, GT_TRUE );
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMirrorAnalyzerVlanTagEnable"); 

    /*set vlan tag+TPID on e port*/
    rc = cpssDxChMirrorRxAnalyzerVlanTagConfigGet(prvTgfDevNum, &prvTgfRestoreCfg.analyzerVlanTagConfig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMirrorRxAnalyzerVlanTagConfigGet"); 

    cpssOsMemSet(&analyzerVlanTagConfig, 0, sizeof(analyzerVlanTagConfig));
    analyzerVlanTagConfig.etherType = PRV_TGF_SFLOW_RESERVED_ETHER_TYPE_CNS;
    analyzerVlanTagConfig.vid = PRV_TGF_INGRESS_EPORT_CNS;
    rc = cpssDxChMirrorRxAnalyzerVlanTagConfig(prvTgfDevNum, &analyzerVlanTagConfig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMirrorRxAnalyzerVlanTagConfig"); 
}
/**
* @internal prvTgfIngresssflowV5MultiTargetTpidConfigReset function
* @endinternal
*
* @brief   Multi target Sflow pass #1.
*           Restore e2phy for Analyzer eport to LB
*           Restore 'Enable new vlan Tag' for analyzer eport
*           restore global RX to analyzer eth type, vid 
*/
static GT_VOID prvTgfIngresssflowV5MultiTargetTpidConfigReset()
{
    GT_STATUS rc;
    /* AUTODOC: ePort to phyport configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                      PRV_TGF_DEFAULT_EPORT_CNS, 
                                                       &(prvTgfRestoreCfg.phyPortInfo));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortTargetMappingTableSet");

    /*reset pushing TAG+TPID for TO_ANALYZER pkts*/
    rc = cpssDxChMirrorAnalyzerVlanTagEnable(prvTgfDevNum, PRV_TGF_DEFAULT_EPORT_CNS, prvTgfRestoreCfg.analyzerVlanTagEnable );
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMirrorAnalyzerVlanTagEnable enable %d",prvTgfRestoreCfg.analyzerVlanTagEnable);

    /*set vlan tag+TPID on e port*/
    rc = cpssDxChMirrorRxAnalyzerVlanTagConfig(prvTgfDevNum, &prvTgfRestoreCfg.analyzerVlanTagConfig);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMirrorRxAnalyzerVlanTagConfig ");
}
/**
* @internal prvTgfIngresssflowV5MultiTargetLbPortConfig function
* @endinternal
*
* @brief   Multi target Sflow pass #1.
*           Config P#3 as LB and enable
*           enable "VLAN tag based Remote Physical Port mapping"  for TPID 0xFFFF
*/
static GT_VOID prvTgfIngresssflowV5MultiTargetLbPortConfig()
{
    CPSS_INTERFACE_INFO_STC  portInterface;
    GT_STATUS rc;

    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));

    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_SERVICE_PORT_IDX_CNS];

    /* set loopback mode on port */
    rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(&portInterface, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficGeneratorPortLoopbackModeEnableSet");


    rc = prvTgfCscdMyPhysicalPortBaseGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SERVICE_PORT_IDX_CNS], &prvTgfRestoreCfg.phyPortBase);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfCscdMyPhysicalPortBaseGet");
    rc = prvTgfCscdMyPhysicalPortBaseSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SERVICE_PORT_IDX_CNS], 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfCscdMyPhysicalPortBaseSet");
    rc = prvTgfCscdMyPhysicalPortAssignmentModeGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SERVICE_PORT_IDX_CNS], &prvTgfRestoreCfg.phyPortAssignMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfCscdMyPhysicalPortAssignmentModeGet");
    rc = prvTgfCscdMyPhysicalPortAssignmentModeSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SERVICE_PORT_IDX_CNS], PRV_TGF_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_VLAN_TAG_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfCscdMyPhysicalPortAssignmentModeSet");
    rc = prvTgfRemotePhysicalPortVlanTagTpidGet(prvTgfDevNum, &prvTgfRestoreCfg.vlanTagTpid);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfRemotePhysicalPortVlanTagTpidGet");
    rc = prvTgfRemotePhysicalPortVlanTagTpidSet(prvTgfDevNum, PRV_TGF_SFLOW_RESERVED_ETHER_TYPE_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfRemotePhysicalPortVlanTagTpidSet");

}
/**
* @internal prvTgfIngresssflowV5MultiTargetLbPortConfigReset function
* @endinternal
*
* @brief   Multi target Sflow pass #1.
*           Reset P#3 as LB
*           restore "VLAN tag based Remote Physical Port mapping"  for TPID 0xFFFF
*/
static GT_VOID prvTgfIngresssflowV5MultiTargetLbPortConfigReset()
{
    CPSS_INTERFACE_INFO_STC  portInterface;
    GT_STATUS rc;

    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));

    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_SERVICE_PORT_IDX_CNS];

    /* set loopback mode on port */
    rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(&portInterface, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficGeneratorPortLoopbackModeEnableSet");


    rc = prvTgfCscdMyPhysicalPortBaseSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SERVICE_PORT_IDX_CNS], prvTgfRestoreCfg.phyPortBase);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfCscdMyPhysicalPortBaseSet");
    rc = prvTgfCscdMyPhysicalPortAssignmentModeSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SERVICE_PORT_IDX_CNS], prvTgfRestoreCfg.phyPortAssignMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfCscdMyPhysicalPortAssignmentModeSet");
    rc = prvTgfRemotePhysicalPortVlanTagTpidSet(prvTgfDevNum, prvTgfRestoreCfg.vlanTagTpid);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfRemotePhysicalPortVlanTagTpidSet");
}

/****************************PASS 2*************************/
/**
* @internal prvTgfFalconMirrorIngressSflowMultiTargetPhaConfig function
* @endinternal
*
* @brief   Multi target Sflow pass #2.
*           Pha init, enable SFLOW pha IPv4 and IPv6 threads 
*           
*/
static GT_VOID prvTgfFalconMirrorIngressSflowMultiTargetPhaConfig(GT_U32 port, CPSS_IP_PROTOCOL_STACK_ENT protocol)
{
    GT_STATUS                           rc = GT_OK;
    prvTgfIngresssflowV5PhaConfig(protocol);
    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum, port, GT_TRUE, protocol == CPSS_IP_PROTOCOL_IPV4_E?PRV_TGF_PHA_THREAD_ID_SFLOW_V5_IPV4_CNS :PRV_TGF_PHA_THREAD_ID_SFLOW_V5_IPV6_CNS );
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaPortThreadIdSet"); 
}
/**
* @internal prvTgfFalconMirrorIngressSflowMultiTargetPhaConfigReset function
* @endinternal
*
* @brief   Multi target Sflow pass #2.
*           restore defalt PHA FW image
*           
*/
static GT_VOID prvTgfFalconMirrorIngressSflowMultiTargetPhaConfigReset()
{
    GT_STATUS                           rc = GT_OK;
    rc = cpssDxChPhaInit(prvTgfDevNum, GT_TRUE, prvTgfRestoreCfg.phaFwImageId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaInit"); 
    phaInitDone = GT_FALSE;
}
/**
* @internal prvTgfIngresssflowV5MultiTargetEpclConfig function
* @endinternal
*
* @brief   Multi target Sflow pass #2.
*          EPCL configurations
*           Enable analyzer port for EPCL trigger
*           Enable port mode for config table
*           10B UDB keys with eport, egr mtag cmd = FORWARD
*           Action = trigger PHA thread, flow based OAM
*/
static GT_VOID prvTgfIngresssflowV5MultiTargetEpclConfig
(
    CPSS_IP_PROTOCOL_STACK_ENT protocol
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT         mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT         pattern;
    PRV_TGF_PCL_ACTION_STC              action;
    GT_U32                              ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT    ruleFormat;
    PRV_TGF_PCL_UDB_SELECT_STC          udbSelect;
    GT_U32                              udbSelectidx;
    GT_U32                              phaThreadId;
    GT_U32                              egrPort;
    GT_U32                              ePort;
    PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT* pclPortAccessModeCfgGetPtr;
    CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT* pclTsAccessModeCfgGetPtr;

    PRV_UTF_LOG0_MAC("======= Setting EPCL Configuration =======\n");
    
    if (protocol == CPSS_IP_PROTOCOL_IPV4_E)
    {
        phaThreadId = PRV_TGF_PHA_THREAD_ID_SFLOW_V5_IPV4_CNS;
        egrPort = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS];
        ePort = PRV_TGF_ANALYZER_EPORT_1_CNS;
        ruleIndex = PRV_TGF_EPCL_IPV4_RULE_INDEX_CNS;
        pclTsAccessModeCfgGetPtr = &prvTgfRestoreCfg.pclTsAccessModeCfgGet;
        pclPortAccessModeCfgGetPtr = &prvTgfRestoreCfg.pclPortAccessModeCfgGet;
    }
    else
    {
        phaThreadId = PRV_TGF_PHA_THREAD_ID_SFLOW_V5_IPV6_CNS;
        egrPort = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT2_IDX_CNS];
        ePort = PRV_TGF_ANALYZER_EPORT_2_CNS;
        ruleIndex = PRV_TGF_EPCL_IPV6_RULE_INDEX_CNS;
        pclTsAccessModeCfgGetPtr = &prvTgfRestoreCfg.pclTsAccessModeCfgGet1;
        pclPortAccessModeCfgGetPtr = &prvTgfRestoreCfg.pclPortAccessModeCfgGet1;
    }
    /* AUTODOC: init PCL Engine for Egress PCL for analyzer port */
    rc = prvTgfPclDefPortInit(
            egrPort,
            CPSS_PCL_DIRECTION_EGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclDefPortInit");

    /* AUTODOC: enable EPCL on Analyzer port */
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          egrPort,
                                          PRV_TGF_PCL_EGRESS_PKT_TO_ANALYZER_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          egrPort,
                                          PRV_TGF_PCL_EGRESS_PKT_TS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          egrPort,
                                          PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: Configure ePort access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeGet(egrPort,
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           pclPortAccessModeCfgGetPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeGet");

    rc = prvTgfPclPortLookupCfgTabAccessModeSet(egrPort,
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

    /* AUTODOC: Configure "Access to the EPCL configuration" table with Port Mode */
    rc = cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeGet(prvTgfDevNum,
                                pclTsAccessModeCfgGetPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet");

    rc = cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet(prvTgfDevNum,
                                CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet");

    cpssOsMemSet(&udbSelect, 0, sizeof(CPSS_DXCH_PCL_UDB_SELECT_STC));
    udbSelectidx = 0;

    while(1)
    {
        if(prvTgfsflowV5EpclMetadataUdbInfo[udbSelectidx].offsetType == CPSS_DXCH_PCL_OFFSET_INVALID_E)
            break;

        /* AUTODOC:   configure User Defined Byte(UDB) */
        /* AUTODOC:   format CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E */
        /* AUTODOC:   packet type PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E */
        /* AUTODOC:   offsetType CPSS_DXCH_PCL_OFFSET_METADATA_E */
        /* AUTODOC:   offset 53 Analyzer ePort[0:6] */
        /* AUTODOC:   offset 54 Analyzer ePort[12:7] */
        /* AUTODOC:   offset 57 Analyzer ePort[13] */
        /* AUTODOC:   offset 64 egress marvell tagged[1:0] */
        rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                        CPSS_PCL_DIRECTION_EGRESS_E,
                                        prvTgfsflowV5EpclMetadataUdbInfo[udbSelectidx].udbIndex,
                                        prvTgfsflowV5EpclMetadataUdbInfo[udbSelectidx].offsetType,
                                        prvTgfsflowV5EpclMetadataUdbInfo[udbSelectidx].offset);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d for udbSelectidx = %d\n", prvTgfDevNum, udbSelectidx);

        udbSelect.udbSelectArr[udbSelectidx]=prvTgfsflowV5EpclMetadataUdbInfo[udbSelectidx].udbIndex;
        udbSelectidx++;
    }

    rc = prvTgfPclUserDefinedBytesSelectSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E, PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                           CPSS_PCL_LOOKUP_0_E, &udbSelect);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedBytesSelectSet: %d\n", prvTgfDevNum);
    /* AUTODOC: set PCL rule 0 */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    /* bits 1:7 is ePort[0:6] */
    pattern.ruleEgrUdbOnly.udb[0] = (ePort&0x7f) <<1;
    mask.ruleEgrUdbOnly.udb[0] = prvTgfsflowV5EpclMetadataUdbInfo[0].byteMask;

    /* bits 0:5 is ePort[12:7] */
    pattern.ruleEgrUdbOnly.udb[1] = ((ePort>>7)&0x3f);
    mask.ruleEgrUdbOnly.udb[1] = prvTgfsflowV5EpclMetadataUdbInfo[1].byteMask;

    /* bits 5 is ePort[13] */
    pattern.ruleEgrUdbOnly.udb[2] = (ePort>>8)&0x20;
    mask.ruleEgrUdbOnly.udb[2] = prvTgfsflowV5EpclMetadataUdbInfo[2].byteMask;

    /*egress pkt cmd*/
    pattern.ruleEgrUdbOnly.udb[3] = 0x03;
    mask.ruleEgrUdbOnly.udb[3] = prvTgfsflowV5EpclMetadataUdbInfo[3].byteMask;

    ruleFormat                                   = PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E;
    action.pktCmd                                = CPSS_PACKET_CMD_FORWARD_E;
    action.oam.oamProcessEnable                  = GT_TRUE;
    action.oam.oamProfile                        = PRV_TGF_OAM_PROFILE_INDEX_CNS;
    action.flowId                                = PRV_TGF_EPCL_FLOW_ID_CNS;
    action.policer.policerEnable                 = PRV_TGF_PCL_POLICER_ENABLE_COUNTER_ONLY_E;
    action.egressPolicy                          = GT_TRUE;
    action.epclPhaInfo.phaThreadIdAssignmentMode = CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENABLED_E;
    action.epclPhaInfo.phaThreadId               = phaThreadId;
    action.epclPhaInfo.phaThreadType             = PRV_TGF_PCL_ACTION_EGRESS_PHA_THREAD_SFLOW_V5_MIRROR_E;
    action.epclPhaInfo.phaThreadUnion.epclSflowV5Mirror.sflowSamplingRate = PRV_TGF_SFLOW_MIRROR_RATE_CNS;
    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclRuleSet, prvTgfDevNum:%d,"
                                            "ruleFormat:%d, ruleIndex:%d", prvTgfDevNum, ruleFormat, ruleIndex);
}
/**
* @internal prvTgfIngresssflowV5MultiTargetEpclConfigReset function
* @endinternal
*
* @brief   Multi target Sflow pass #2.
*          EPCL configurations restore
*           Restore port config
*           invalidate EPCL rule
*/
static GT_VOID prvTgfIngresssflowV5MultiTargetEpclConfigReset(CPSS_IP_PROTOCOL_STACK_ENT protocol)
{
    GT_STATUS rc;
    GT_U32                              ruleIndex;
    GT_U32                              egrPort;
    PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT* pclPortAccessModeCfgGetPtr;
    CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT* pclTsAccessModeCfgGetPtr;
    if (protocol == CPSS_IP_PROTOCOL_IPV4_E)
    {
        egrPort = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS];
        ruleIndex = PRV_TGF_EPCL_IPV4_RULE_INDEX_CNS;
        pclTsAccessModeCfgGetPtr = &prvTgfRestoreCfg.pclTsAccessModeCfgGet;
        pclPortAccessModeCfgGetPtr = &prvTgfRestoreCfg.pclPortAccessModeCfgGet;
    }
    else
    {
        egrPort = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT2_IDX_CNS];
        ruleIndex = PRV_TGF_EPCL_IPV6_RULE_INDEX_CNS;
        pclTsAccessModeCfgGetPtr = &prvTgfRestoreCfg.pclTsAccessModeCfgGet1;
        pclPortAccessModeCfgGetPtr = &prvTgfRestoreCfg.pclPortAccessModeCfgGet1;
    }
    /* -------------------------------------------------------------------------
     * Restore EPCL Configuration
     */
    /* AUTODOC: Invalidate PCL rule 0 */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_10_BYTES_E, ruleIndex, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclRuleValidStatusSet, index = %d", 0);

    /* AUTODOC: disable all PCL configurations on the ports */
    prvTgfPclPortsRestoreAll();

    /* AUTODOC: disable EPCL on Analyzer port per packet type */
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          egrPort,
                                          PRV_TGF_PCL_EGRESS_PKT_TO_ANALYZER_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          egrPort,
                                          PRV_TGF_PCL_EGRESS_PKT_TS_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          egrPort,
                                          PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");

    rc = prvTgfPclPortLookupCfgTabAccessModeSet(egrPort,
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           *pclPortAccessModeCfgGetPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

    rc = cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet(prvTgfDevNum,
                                *pclTsAccessModeCfgGetPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet");

}
/**
* @internal prvTgfFalconMirrorIngressSflowMultiTargetAnalyzerPort1Config function
* @endinternal
*
* @brief   Multi target Sflow pass #2.
*          Analyzer port 1 config
*           e2phy ePort#1 to P#1
*           ePort#1 to IPV4 TS
*           EPCL rule to trigger SFLOW IPv4 PHA
*/
static GT_VOID prvTgfFalconMirrorIngressSflowMultiTargetAnalyzerPort1Config()
{
    GT_STATUS rc;
    CPSS_INTERFACE_INFO_STC phyPortInfo;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC     egressInfo;

    /* AUTODOC: Save ePort to phyport configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                                      PRV_TGF_ANALYZER_EPORT_1_CNS, 
                                                       &(prvTgfRestoreCfg.phyPortInfo));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortTargetMappingTableGet");
    phyPortInfo.type = CPSS_INTERFACE_PORT_E;
    phyPortInfo.devPort.hwDevNum = prvTgfDevNum;
    phyPortInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS];
    prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                       PRV_TGF_ANALYZER_EPORT_1_CNS, &phyPortInfo );
    /* AUTODOC: Set analyzer ePort#1 attributes configuration */
    /* AUTODOC: Tunnel Start = ENABLE, tunnelStartPtr = 8, tsPassenger = ETHERNET */
    /*Get eport attribute config*/
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                                                       PRV_TGF_ANALYZER_EPORT_1_CNS,
                                                       &prvTgfRestoreCfg.egressInfo);
    cpssOsMemSet(&egressInfo,0,sizeof(egressInfo));
    egressInfo = prvTgfRestoreCfg.egressInfo;
    egressInfo.tunnelStart            = GT_TRUE;
    egressInfo.tunnelStartPtr         = PRV_TGF_TUNNEL_START_IPV4_INDEX_CNS;
    egressInfo.tsPassengerPacketType  = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       PRV_TGF_ANALYZER_EPORT_1_CNS,
                                                       &egressInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoSet");
    /*trigger PHA with EPCL*/
    prvTgfIngresssflowV5MultiTargetEpclConfig(CPSS_IP_PROTOCOL_IPV4_E);
}
/**
* @internal prvTgfFalconMirrorIngressSflowMultiTargetAnalyzerPort1ConfigReset function
* @endinternal
*
* @brief   Multi target Sflow pass #2.
*           reset e2phy ePort#1 to P#1
*           restore IPV4 TS
*           reset EPCL rule to trigger SFLOW IPv4 PHA
*/
static GT_VOID prvTgfFalconMirrorIngressSflowMultiTargetAnalyzerPort1ConfigReset()
{
    GT_STATUS rc;

    /* AUTODOC: Save ePort to phyport configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                      PRV_TGF_ANALYZER_EPORT_1_CNS, 
                                                       &(prvTgfRestoreCfg.phyPortInfo));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortTargetMappingTableSet");
    
    /*reset eport attribute config*/
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       PRV_TGF_ANALYZER_EPORT_1_CNS,
                                                       &prvTgfRestoreCfg.egressInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoSet");
    
    /*reset EPCL*/
    prvTgfIngresssflowV5MultiTargetEpclConfigReset(CPSS_IP_PROTOCOL_IPV4_E);
}
/**
* @internal prvTgfFalconMirrorIngressSflowMultiTargetAnalyzerPort2Config function
* @endinternal
*
* @brief   Multi target Sflow pass #2.
*          Analyzer port 2 config
*           e2phy ePort#2 to P#2
*           ePort#2 to IPV6 TS
*           EPCL rule to trigger SFLOW IPv6 PHA
*/
static GT_VOID prvTgfFalconMirrorIngressSflowMultiTargetAnalyzerPort2Config()
{

    GT_STATUS rc;
    CPSS_INTERFACE_INFO_STC phyPortInfo;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC     egressInfo;
    /* AUTODOC: Save ePort to phyport configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                                      PRV_TGF_ANALYZER_EPORT_2_CNS, 
                                                       &(prvTgfRestoreCfg.phyPortInfo2));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortTargetMappingTableGet");
    phyPortInfo.type = CPSS_INTERFACE_PORT_E;
    phyPortInfo.devPort.hwDevNum = prvTgfDevNum;
    phyPortInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT2_IDX_CNS];
    prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                       PRV_TGF_ANALYZER_EPORT_2_CNS, &phyPortInfo );
    /* AUTODOC: Set analyzer ePort#1 attributes configuration */
    /* AUTODOC: Tunnel Start = ENABLE, tunnelStartPtr = 8, tsPassenger = ETHERNET */
    /*Get eport attribute config*/
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                                                       PRV_TGF_ANALYZER_EPORT_2_CNS,
                                                       &prvTgfRestoreCfg.egressInfo2);
    cpssOsMemSet(&egressInfo,0,sizeof(egressInfo));
    egressInfo = prvTgfRestoreCfg.egressInfo;
    egressInfo.tunnelStart            = GT_TRUE;
    egressInfo.tunnelStartPtr         = PRV_TGF_TUNNEL_START_IPV6_INDEX_CNS;
    egressInfo.tsPassengerPacketType  = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                      PRV_TGF_ANALYZER_EPORT_2_CNS, 
                                                       &egressInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoSet");
    /*trigger PHA with EPCL*/
    prvTgfIngresssflowV5MultiTargetEpclConfig(CPSS_IP_PROTOCOL_IPV6_E);
}
/**
* @internal prvTgfFalconMirrorIngressSflowMultiTargetAnalyzerPort2ConfigReset function
* @endinternal
*
* @brief   Multi target Sflow pass #2.
*           reset e2phy ePort#2 to P#2
*           restore IPV6 TS
*           reset EPCL rule to trigger SFLOW IPv6 PHA
*/
static GT_VOID prvTgfFalconMirrorIngressSflowMultiTargetAnalyzerPort2ConfigReset()
{
    GT_STATUS rc;

    /* AUTODOC: Save ePort to phyport configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                      PRV_TGF_ANALYZER_EPORT_2_CNS, 
                                                       &(prvTgfRestoreCfg.phyPortInfo2));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortTargetMappingTableSet");
    
    /*reset eport attribute config*/
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       PRV_TGF_ANALYZER_EPORT_2_CNS,
                                                       &prvTgfRestoreCfg.egressInfo2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoSet");
    
    /*reset EPCL*/
    prvTgfIngresssflowV5MultiTargetEpclConfigReset(CPSS_IP_PROTOCOL_IPV6_E);
}

/**
* @internal prvTgfIngresssflowV5MultiTargetL2mllConfig function
* @endinternal
*
* @brief   Multi target Sflow pass #2.
*           L2 MLL config
*           configure eVIDX -> L2MLL LTT entry
*           config L2MLL LTT -> L2MLL pair
*           config L2MLL pair with 2 eports, each points to 
*               e2phy, TS, PHA
*/
static GT_VOID prvTgfIngresssflowV5MultiTargetL2mllConfig()
{
    GT_STATUS                                   rc;
    PRV_TGF_L2_MLL_PAIR_STC                     mllPairEntry;
    PRV_TGF_L2_MLL_LTT_ENTRY_STC                lttEntry;

    /* AUTODOC: Store MLL LTT entry */
    rc = prvTgfL2MllLttEntryGet(prvTgfDevNum, PRV_TGF_MLL_LTT_INDEX_CNS,
                                &prvTgfRestoreCfg.lttEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntryGet: %d",
                                 PRV_TGF_MLL_LTT_INDEX_CNS);

    /* AUTODOC: Configure MLL LTT entry */
    lttEntry.mllPointer = PRV_TGF_MLL_PAIR_ENTRY_INDEX_CNS;
    lttEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;
    lttEntry.mllMaskProfileEnable = GT_FALSE;
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, PRV_TGF_MLL_LTT_INDEX_CNS, &lttEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet: %d",
                                 PRV_TGF_MLL_LTT_INDEX_CNS);

    /* AUTODOC: Store L2 MLL entry */
    rc = prvTgfL2MllPairRead(prvTgfDevNum, PRV_TGF_MLL_PAIR_ENTRY_INDEX_CNS,
                             PRV_TGF_PAIR_READ_WRITE_WHOLE_E,
                             &prvTgfRestoreCfg.mllPairEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairRead: %d", prvTgfDevNum);

    /* AUTODOC: Configure L2 MLL entry */
    mllPairEntry.nextPointer = 0;
    mllPairEntry.entrySelector = PRV_TGF_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;
    cpssOsMemSet(&mllPairEntry.firstMllNode, 0, sizeof(PRV_TGF_L2_MLL_ENTRY_STC));
    mllPairEntry.firstMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
    mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    mllPairEntry.firstMllNode.egressInterface.devPort.portNum = PRV_TGF_ANALYZER_EPORT_1_CNS;
    mllPairEntry.firstMllNode.last= GT_FALSE;

    cpssOsMemSet(&mllPairEntry.secondMllNode, 0, sizeof(PRV_TGF_L2_MLL_ENTRY_STC));
    mllPairEntry.secondMllNode.egressInterface.type = CPSS_INTERFACE_PORT_E;
    mllPairEntry.secondMllNode.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    mllPairEntry.secondMllNode.egressInterface.devPort.portNum = PRV_TGF_ANALYZER_EPORT_2_CNS;
    mllPairEntry.secondMllNode.last= GT_TRUE;
    rc = prvTgfL2MllPairWrite(prvTgfDevNum, PRV_TGF_MLL_PAIR_ENTRY_INDEX_CNS,
                              PRV_TGF_PAIR_READ_WRITE_WHOLE_E,
                              &mllPairEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: %d", prvTgfDevNum);
}
/**
* @internal prvTgfIngresssflowV5MultiTargetL2mllConfigReset function
* @endinternal
*
* @brief   Multi target Sflow pass #2.
*           reset L2MLL LTT entry
*           reset L2MLL pair
*/
static GT_VOID prvTgfIngresssflowV5MultiTargetL2mllConfigReset()
{
    GT_STATUS                                   rc;

    /* AUTODOC: Store MLL LTT entry */
    rc = prvTgfL2MllLttEntrySet(prvTgfDevNum, PRV_TGF_MLL_LTT_INDEX_CNS,
                                &prvTgfRestoreCfg.lttEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllLttEntrySet: %d",
                                 PRV_TGF_MLL_LTT_INDEX_CNS);

    /* AUTODOC: Store L2 MLL entry */
    rc = prvTgfL2MllPairWrite(prvTgfDevNum, PRV_TGF_MLL_PAIR_ENTRY_INDEX_CNS,
                             PRV_TGF_PAIR_READ_WRITE_WHOLE_E,
                             &prvTgfRestoreCfg.mllPairEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfL2MllPairWrite: %d", prvTgfDevNum);
}
/**
* @internal prvTgfFalconMirrorIngressSflowMultiTargetTtiConfig function
* @endinternal
*
* @brief   Multi target Sflow pass #2.
*          TTI config to get eVIDX for multiple SLOW collectors
*               Enable TTI lookup for srcPort derived from remote Vlan tag
*               Create UDB size 10 for ETH_Other,
*                   key : reserved Ether type
*                   action: redirect to eVIDX, bridge bypass
*/
static GT_VOID prvTgfFalconMirrorIngressSflowMultiTargetTtiConfig(GT_VOID)
{
    GT_STATUS                rc = GT_OK;
    PRV_TGF_TTI_ACTION_STC   ttiAction;
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;

    /* AUTODOC: clear pattern and mask */
    cpssOsMemSet((GT_VOID*) &ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet((GT_VOID*) &ttiMask,    0, sizeof(ttiMask));
    cpssOsMemSet((GT_VOID*) &ttiAction, 0, sizeof(ttiAction));

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: set UDB byte for the key keyType: anchor L2 header, offset 12,13 (ether type)*/
    rc = prvTgfTtiUserDefinedByteGet(prvTgfDevNum, keyType, 0, &prvTgfRestoreCfg.ttiAnchor[0], &prvTgfRestoreCfg.ttiOffset[0]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteGet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteGet(prvTgfDevNum, keyType, 0, &prvTgfRestoreCfg.ttiAnchor[1], &prvTgfRestoreCfg.ttiOffset[1]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteGet: %d", prvTgfDevNum);

    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, keyType, 0, PRV_TGF_TTI_OFFSET_L2_E, 12);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, keyType, 1, PRV_TGF_TTI_OFFSET_L2_E, 13);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);


    /* AUTODOC: set the TTI Rule Pattern and Mask to match 
        reserved Ether type 0xFFFF*/
    ttiPattern.udbArray.udb[0] = PRV_TGF_SFLOW_RESERVED_ETHER_TYPE_CNS >> 8;
    ttiMask.udbArray.udb[0]    = 0xFF;
    ttiPattern.udbArray.udb[1] = PRV_TGF_SFLOW_RESERVED_ETHER_TYPE_CNS & 0xFF;
    ttiMask.udbArray.udb[1]    = 0xFF;

    /* AUTODOC: set the TTI Rule Action for matched packet */
    ttiAction.interfaceInfo.type  = CPSS_INTERFACE_VIDX_E;
    ttiAction.interfaceInfo.vidx  = PRV_TGF_EVIDX_CNS;
    ttiAction.bridgeBypass        = GT_TRUE;
    ttiAction.redirectCommand     = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;

    /* AUTODOC: enable the TTI lookup for keyType at the ingress remote port */
    rc = cpssDxChTtiPortLookupEnableGet(prvTgfDevNum, PRV_TGF_INGRESS_EPORT_CNS, keyType, &prvTgfRestoreCfg.ttiEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTtiPortLookupEnableGet");

    rc = cpssDxChTtiPortLookupEnableSet(prvTgfDevNum, PRV_TGF_INGRESS_EPORT_CNS, keyType, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTtiPortLookupEnableSet");

    /* AUTODOC: set key size TTI_KEY_SIZE_10_B_E for the key keyType */
    rc = cpssDxChTtiPacketTypeKeySizeGet(prvTgfDevNum, keyType, &prvTgfRestoreCfg.ttiKeySize);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeGet: %d", prvTgfDevNum);

    rc = cpssDxChTtiPacketTypeKeySizeSet(prvTgfDevNum, keyType, CPSS_DXCH_TTI_KEY_SIZE_10_B_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);

    /* AUTODOC: add TTI rule 1 */ 
    rc = prvTgfTtiRuleSet(PRV_TGF_TTI0_INDEX_CNS, PRV_TGF_TTI_RULE_UDB_10_E, &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");
}
/**
* @internal prvTgfFalconMirrorIngressSflowMultiTargetTtiConfigReset function
* @endinternal
*
* @brief   Multi target Sflow pass #2.
*          Reset TTI config to get eVIDX for multiple SLOW collectors
*               Disable TTI lookup for srcPort derived from remote Vlan tag
*               Delete TTI rule
*/
static GT_VOID prvTgfFalconMirrorIngressSflowMultiTargetTtiConfigReset(GT_VOID)
{
    GT_STATUS   rc;
    GT_U32      i;

    /* AUTODOC: disable TTI lookup for port 0, for keyType */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], keyType, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    rc = cpssDxChTtiPortLookupEnableSet(prvTgfDevNum, PRV_TGF_INGRESS_EPORT_CNS, keyType, prvTgfRestoreCfg.ttiEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTtiPortLookupEnableSet");

    rc = cpssDxChTtiPacketTypeKeySizeSet(prvTgfDevNum, keyType, prvTgfRestoreCfg.ttiKeySize);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);

    /* AUTODOC: invalidate TTI rules */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI0_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");
    
    /*Restore TTI UDB config*/
    for (i=0; i < 2 ; i++)
    {
        rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, keyType, 0, prvTgfRestoreCfg.ttiAnchor[i], prvTgfRestoreCfg.ttiOffset[i]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: index %d", i);
    }
}
/**
* @internal prvTgfFalconMirrorIngressSflowMultiTargetConfig function
* @endinternal
*
* @brief   Multi target Sflow Use case.
*          It needs 2 pass mechanism for sending a ingress SFLOW Mirrored to multiple collectors
*           ===PASS 1===
*               prepare ports, p#0 RX, p#1 TX1, p#2 TX2, p#3 LB
*               setup ingress mirroring index, target to eport, sampling ratio 
*               enable Rx mirroring on port=1, index=0 
*               prepare the analyzer eport, e2phy to LB, push reserved eth type 0xffff
*               configure the P#3 as LB port
*           ===PASS 2===
*               enable "VLAN tag based Remote Physical Port mapping"  for TPID 0xFFFF
*               TTI config, match reserved TPID, action: bypass ingress, assign eVIDX
*               configure evidx to l2mll mapping
*               for each eport of MLL
*                 1. e2phy to egress ports
*                 2. TS index for V4/V6
*                 3. PHA thread id
*               configure TS L2+IP+UDP, and TS profile
*               e2phy  and enable TS for eports
*               setup PHA threads
*/
GT_VOID prvTgfFalconMirrorIngressSflowMultiTargetConfig()
{
    /*===PASS 1===*/
    /*prepare ports, p#0 RX, p#1 TX1, p#2 TX2, p#3 LB*/
    prvTgfFalconMirrorIngressSflowBridgeConfigSet();

    /*setup ingress mirroring index, target to eport, sampling ratio */
    prvTgfIngresssflowV5MultiTargetMirrorConfig();
    /* AUTODOC: enable Rx mirroring on port=1, index=0 */
    prvTgfMirrorRxPortGet(prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], &prvTgfRestoreCfg.rxMirrorEnable, &prvTgfRestoreCfg.rxMirrorIdx);
    prvTgfMirrorRxPortSet(prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], GT_TRUE, PRV_TGF_SFLOW_MIRROR_IDX_CNS);
    /*prepare the analyzer eport, e2phy to LB, push reserved eth type 0xffff*/
    prvTgfIngresssflowV5MultiTargetTpidConfig();
    /*prepare the service port*/
    prvTgfIngresssflowV5MultiTargetLbPortConfig();

    /*===PASS 2===*/
    /*TTI config, match reserved TPID, action: bypass ingress, assign eVIDX*/
    prvTgfFalconMirrorIngressSflowMultiTargetTtiConfig();
    /*configure evidx to l2mll mapping*/
    prvTgfIngresssflowV5MultiTargetL2mllConfig();
    /*for each eport of MLL
        1. e2phy to egress ports
        2. TS index for V4/V6
        3. PHA thread id*/
    /*configure TS L2+IP+UDP, and TS profile*/
    prvTgfIngresssflowV5Ipv4TsConfig();
    prvTgfIngresssflowV5Ipv6TsConfig();
    /*e2phy  and enable TS for eports*/
    prvTgfFalconMirrorIngressSflowMultiTargetAnalyzerPort1Config();
    prvTgfFalconMirrorIngressSflowMultiTargetAnalyzerPort2Config();
    /*setup PHA threads*/
    prvTgfFalconMirrorIngressSflowMultiTargetPhaConfig(prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS], CPSS_IP_PROTOCOL_IPV4_E);
    prvTgfFalconMirrorIngressSflowMultiTargetPhaConfig(prvTgfPortsArray[PRV_TGF_ANALYZER_PORT2_IDX_CNS], CPSS_IP_PROTOCOL_IPV6_E);
    /* EOAM Configuration */
    prvTgfFalconMirrorIngressSflowEoamConfigSet(PRV_TGF_EPCL_FLOW_ID_CNS);

    /* EPLR Configuration */
    prvTgfFalconMirrorIngressSflowEplrConfigSet();

    /* ERMRK Configuration */
    prvTgfFalconMirrorIngressSflowErmrkConfigSet();

}

/**
* @internal prvTgfFalconMirrorIngressSflowMultiTargetTraffic function
* @endinternal
*
* @brief   Multi target Sflow Use case.
*           Test the captures for the configs for Multi sflow collectors.
*           RX:
*               L2 pkt for Vlan5
*           TX:
*               2 pkts on 2 Egr analyzer ports.
*               1. IPv4 encapsulated 
*               2. IPv6 encapsulated 
*/
GT_VOID prvTgfFalconMirrorIngressSflowMultiTargetTraffic()
{
    CPSS_INTERFACE_INFO_STC  analyzerPortInterface;
    GT_U8                 packetBuf[256];
    GT_U32                packetLen = sizeof(packetBuf);
    GT_U32                packetActualLength = 0;
    GT_U8                 queue = 0;
    GT_U8                 dev = 0;
    GT_BOOL                 getFirst = GT_TRUE;
    TGF_NET_DSA_STC       rxParam;
    GT_STATUS                           rc = GT_OK;

    /* Enable capture on analyzer port */
    analyzerPortInterface.type              = CPSS_INTERFACE_PORT_E;
    analyzerPortInterface.devPort.hwDevNum  = prvTgfDevNum;
    analyzerPortInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&analyzerPortInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

    /* Enable capture on analyzer port 2*/
    analyzerPortInterface.type              = CPSS_INTERFACE_PORT_E;
    analyzerPortInterface.devPort.hwDevNum  = prvTgfDevNum;
    analyzerPortInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT2_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&analyzerPortInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfOriginalPacketInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");

    PRV_UTF_LOG0_MAC("======= Send packet =======\n");
    /* Send first packet  */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStartTransmitingEth");

    /* Disable capture on analyzer port 1*/
    analyzerPortInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS];
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
    /*verify the packet*/
    prvTgfIngressSflowV5ValidatePacket(packetBuf, CPSS_IP_PROTOCOL_IPV4_E);

    /* Disable capture on analyzer port 2*/
    analyzerPortInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT2_IDX_CNS];
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
    /*verify the packet*/
    prvTgfIngressSflowV5ValidatePacket(packetBuf, CPSS_IP_PROTOCOL_IPV6_E);
}
/**
* @internal prvTgfFalconMirrorIngressSflowMultiTargetConfigReset function
* @endinternal
*
* @brief   Multi target Sflow Use case.
*           Reset all the configs for Multi sflow collectors
*/
GT_VOID prvTgfFalconMirrorIngressSflowMultiTargetConfigReset()
{
    /*reset Vlan and flush fdb*/
    prvTgfFalconMirrorIngressSflowBridgeConfigReset();
    
    /*reset ingress mirroring analyzer entry, eport config, and e2phy map*/
    prvTgfIngresssflowV5MirrorConfigReset();
    prvTgfIngresssflowV5MultiTargetMirrorConfigReset();

    /*disable rx mirroring on ingress port*/
    prvTgfMirrorRxPortSet(prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], GT_FALSE, prvTgfRestoreCfg.rxMirrorIdx);

    /*invalidate the IPv4 and IPV6 TS entries*/
    prvTgfIngresssflowV5MultiTargetTpidConfigReset();

    /*remove LB config, and restore service port*/
    prvTgfIngresssflowV5MultiTargetLbPortConfigReset();

    /*remove e2phy, TS, EPCL configs for Analyzer port #1*/
    prvTgfFalconMirrorIngressSflowMultiTargetPhaConfigReset();

    /*remove e2phy, TS, EPCL configs for Analyzer port #2*/
    prvTgfFalconMirrorIngressSflowMultiTargetAnalyzerPort1ConfigReset();

    /*invalidate the IPv4 and IPV6 TS entries*/
    prvTgfFalconMirrorIngressSflowMultiTargetAnalyzerPort2ConfigReset();

    /*reset L2MLL and LTT entries*/
    prvTgfIngresssflowV5MultiTargetL2mllConfigReset();

    /*remove TTI entry */
    prvTgfFalconMirrorIngressSflowMultiTargetTtiConfigReset ();

    /*invalidate the IPv4 and IPV6 TS entries*/
    prvTgfIngresssflowV5IpTsConfigReset(PRV_TGF_TUNNEL_START_IPV4_INDEX_CNS, CPSS_IP_PROTOCOL_IPV4_E);
    prvTgfIngresssflowV5IpTsConfigReset(PRV_TGF_TUNNEL_START_IPV6_INDEX_CNS, CPSS_IP_PROTOCOL_IPV6_E);

    prvTgfIngresssflowV5EoamConfigReset();

    prvTgfIngresssflowV5EplrConfigReset();

    prvTgfIngresssflowV5ErmrkConfigReset();

    /*restore all buffers and reset mac counters*/
    prvTgfEthCountersReset(prvTgfDevNum);
}
