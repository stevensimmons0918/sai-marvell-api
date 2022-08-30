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
* @file prvTgfOamEgressNodeUseCase.c
*
* @brief Egress Node use case testing
*
* @version   1
********************************************************************************
*/
#include <oam/prvTgfOamEgressNodeUseCase.h>
#include <common/tgfIpGen.h>
#include <common/tgfMirror.h>
#include <cpss/dxCh/dxChxGen/pha/cpssDxChPha.h>
#include <cpss/dxCh/dxChxGen/oam/cpssDxChOam.h>
#include <exactMatch/prvTgfExactMatchBasicDefaultActionPcl.h>
#include <common/tgfCosGen.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqPreemptionUtils.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


#if (!(defined ASIC_SIMULATION_ENV_FORBIDDEN) && defined ASIC_SIMULATION)
    /* need to run with simulation lib exists */
    #define WM_IMPLEMENTED
#endif

#define PRV_TGF_DEFAULT_VLANID_CNS                          1
#define PRV_TGF_PHA_THREAD_ID_TRANSIT                       14
#define PRV_TGF_PHA_THREAD_ID_EGRESS                        28
#define PRV_TGF_PHA_THREAD_ID_EGRESS_METADATA               47

/* Tunnel start entry index */
#define PRV_TGF_TUNNEL_START_INDEX_CNS                      8

/* port to Send/Receive traffic*/
#define PRV_TGF_INGRESS_PORT_IDX_CNS                        3
#define PRV_TGF_TRANS_PORT_IDX_CNS                          2
#define PRV_TGF_EGR_PORT_IDX_CNS                            1
#define IPV6_DUMMY_PROTOCOL_CNS                             0x3b
#define PRV_TGF_UDP_DST_PORT_CNS                            4790

/* Verification part of IOAM Metadata */
#define PRV_TGF_HOP_LIMIT_CNS                               0x40
#define PRV_TGF_NODE_ID_CNS                                 0x17

/* size of packet */
#define PRV_TGF_PACKET_SIZE_CNS                             140

/* Template profile index */
#define PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS                  1

/* Qos profile */
#define PRV_TGF_CNC_GEN_DXCH_TEST_QOS_PROFILE0              0
#define PRV_TGF_TRAFFIC_CLASS_CNS                           5

#ifdef WM_IMPLEMENTED
extern GT_U32     simPhaDebugQueueLimit;                     /* 20 Bit Value */
extern GT_U32     simPhaDebugQueueLen;                       /* 20 Bit Value */
extern GT_U8      snetPclDebugOriginalQueueFb;               /* 6  Bit Value */
#endif

static PRV_TGF_TUNNEL_START_GEN_IP_PROFILE_STC              profileData;


/*
 * L2          14
 * IPv6        40
 * GRE         4
 * L2          14
 * IPv6        40
 * UDP         8
 * VXLAN       8
 * SHIM Header 4
 * Shim trace  4
 */
#define PRV_TGF_IOAM_MD_OFFSET_CNS     (TGF_L2_HEADER_SIZE_CNS          +\
                                        TGF_ETHERTYPE_SIZE_CNS          +\
                                        TGF_IPV6_HEADER_SIZE_CNS        +\
                                        TGF_GRE_HEADER_SIZE_CNS         +\
                                        TGF_L2_HEADER_SIZE_CNS          +\
                                        TGF_ETHERTYPE_SIZE_CNS          +\
                                        TGF_IPV6_HEADER_SIZE_CNS        +\
                                        TGF_UDP_HEADER_SIZE_CNS         +\
                                        TGF_VXLAN_GPE_HEADER_SIZE_CNS   +\
                                        TGF_IOAM_SHIM_HEADER_SIZE_CNS   +\
                                        TGF_IOAM_SHIM_HEADER_SIZE_CNS)


#define PRV_TGF_MIRROR_METADATA_OFFSET_CNS  (TGF_L2_HEADER_SIZE_CNS          +\
                                             TGF_ETHERTYPE_SIZE_CNS          +\
                                             TGF_IPV6_HEADER_SIZE_CNS        +\
                                             TGF_GRE_HEADER_SIZE_CNS)

/* analyzer index */
static TGF_PACKET_STC           packetInfo;
static GT_U32                   analyzerIndex               = 1;
static CPSS_INTERFACE_INFO_STC  transPortInterface, egrPortInterface;

/****** Original Ingress Packet ******/
/* VXLAN and IOAM header
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |R|R|Ver|I|P|B|O|               Reserved        |     NP = IOAM |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |         Virtual Network Identifier (VNI)      |      Reserved |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    | Type =IOAM     | IOAM HDR len   | Reserved    | Next Protocol |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                      IOAM options                             |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x01, 0x02, 0x00, 0x01, 0x80, 0x00, 0x80, 0x00, /* IOAM Header --- type=01 HeaderLength=0x02, NextProt=01, TraceType=0xFA, MaximumLength=0x80, Flags=0x00 */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr_withOutVxlan[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x20, 0x21, 0x22, 0x23, 0x24, 0x24,
    0x25, 0x26
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart_withOutVxlan = {
    sizeof(prvTgfPayloadDataArr_withOutVxlan),          /* dataLength */
    prvTgfPayloadDataArr_withOutVxlan                   /* dataPtr */
};


static TGF_PACKET_L2_STC prvTgfOriginalPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x03}                /* saMac */
};

/* Tunnel section: L2 part */
static TGF_PACKET_L2_STC prvTgfPacketTunnelL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x44, 0x04},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x45, 0x05}                /* saMac */
};

static TGF_IPV6_ADDR        prvTgfTunnelStartSrcIPv6         = {0x2001, 0x2002, 0x2003, 0x2004, 0x2005, 0x2006, 0x2007, 0x2008};
static TGF_IPV6_ADDR        prvTgfTunnelStartDstIPv6         = {0x3001, 0x3002, 0x3003, 0x3004, 0x3005, 0x3006, 0x3007, 0x3008};

/* IPv6 packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart_Ipv6 = { TGF_ETHERTYPE_86DD_IPV6_TAG_CNS };

/* Original packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfOriginalPacketIpv6Part = {
    6,                      /* version */
    0,                      /* trafficClass */
    0,                      /* flowLabel */
    0x1a,                   /* payloadLen */
    IPV6_DUMMY_PROTOCOL_CNS,/* nextHeader */
    PRV_TGF_HOP_LIMIT_CNS,  /* hopLimit */
    {0x4001, 0x4002, 0x4003, 0x4004, 0x4005, 0x4006, 0x4007, 0x4008}, /* TGF_IPV6_ADDR srcAddr */
    {0x5001, 0x5002, 0x5003, 0x5004, 0x5005, 0x5006, 0x5007, 0x5008}  /* TGF_IPV6_ADDR dstAddr */
};

/* packet's UDP part */
static TGF_PACKET_UDP_STC prvTgfPacketUdpPart =
{
    0x2000,                           /* src port */
    PRV_TGF_UDP_DST_PORT_CNS,         /* dst port */
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,  /* length */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS /* csum */
};

/* packet's VXLAN-GPE header */
static TGF_PACKET_VXLAN_GPE_STC prvTgfPacketVxlanGpePart = {
    0xF,                /* Flags */
    0x81,               /* Reserved */
    0x78,               /* Next Protocol */
    0x98,               /* VXLAN Network Identifier (VNI) - 24Bits */
    0x80                /* Reserved */
};

/* IPv6 - PARTS of packet */
static TGF_PACKET_PART_STC prvTgfIpv6PacketArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfOriginalPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart_Ipv6},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfOriginalPacketIpv6Part},
    {TGF_PACKET_PART_UDP_E,       &prvTgfPacketUdpPart},
    {TGF_PACKET_PART_VXLAN_GPE_E, &prvTgfPacketVxlanGpePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* IPv6 - PARTS of packet - Without VXLAN & UDP*/
static TGF_PACKET_PART_STC prvTgfIpv6PacketArray_withOutVxlan[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfOriginalPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart_Ipv6},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfOriginalPacketIpv6Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart_withOutVxlan}
};

/*************************** Restore config ***********************************/
/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC           interface;
    GT_BOOL                                         prvTgfEnableMirror;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC                egressInfo;
    PRV_TGF_TUNNEL_START_ENTRY_UNT                  tunnelEntry;
    GT_BOOL                                         treatIngressAsEgress;
} prvTgfRestoreCfg;

/*************************** Test config ***********************************/

/* debug flag to disable using of PHA for the test */
static GT_BOOL prvTgfOamEgressNodeSkipPha = GT_FALSE;
GT_BOOL prvTgfOamEgressNodeSkipPhaSet(GT_BOOL newSkipPha)
{
    GT_BOOL oldSkipPha = prvTgfOamEgressNodeSkipPha;

    prvTgfOamEgressNodeSkipPha = newSkipPha;

    return oldSkipPha;
}

/**
* internal prvTgfPhaConfigSet function
* @endinternal
*
* @brief   Assigns PHA to ePort.
*/
static GT_VOID prvTgfPhaConfigSet
(
    IN GT_PORT_NUM                      portNum,
    IN GT_U32                           threadId,
    IN CPSS_DXCH_PHA_THREAD_TYPE_ENT    extType
)
{
    GT_STATUS                               rc = GT_OK;
    CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC    commonInfo;
    CPSS_DXCH_PHA_THREAD_INFO_UNT           extInfo;

    cpssOsMemSet(&extInfo, 0, sizeof(extInfo));
    cpssOsMemSet(&commonInfo,0,sizeof(CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC));
    commonInfo.stallDropCode                      = CPSS_NET_FIRST_USER_DEFINED_E;
    commonInfo.statisticalProcessingFactor        = 0;
    commonInfo.busyStallMode                      = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;

    /*AUTODOC: Assign thread to the ePort */
    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum,
                                    portNum,
                                    GT_TRUE,
                                    threadId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "[TGF]: cpssDxChPhaPortThreadIdSet FAILED for PHA_THREAD_ID_MIRRORING, rc = [%d]", rc);

    /* AUTODOC: Set the thread entry */
    cpssOsMemSet(&extInfo, 0, sizeof(extInfo));     /* No template */
    rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
                                     threadId,
                                     &commonInfo,
                                     extType,
                                     &extInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"[TGF]: cpssDxChPhaThreadIdEntrySet FAILED, rc = [%d]", rc);
}

/**
* @internal prvTgfEgressNodeRoutingConfigSet function
* @endinternal
*
* @brief   Routing configuration for egress Node use case
*/
static GT_VOID prvTgfEgressNodeRoutingConfigSet
(
    GT_U8    nextHopPortNum
)
{
    PRV_TGF_BRG_MAC_ENTRY_STC       macEntry;
    GT_STATUS                       rc = GT_OK;

    /* create a macEntry with .daRoute = GT_TRUE */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfOriginalPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    macEntry.key.entryType                  = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId         = PRV_TGF_DEFAULT_VLANID_CNS;
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum  = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum   = nextHopPortNum;
    macEntry.isStatic                       = GT_TRUE;
    macEntry.daCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.saCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.daRoute                        = GT_TRUE;

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 1, port 2 */
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite: %d", prvTgfDevNum);
}

/**
* @internal prvTgfEgressNodeOrigPacketEpclConfigSet function
* @endinternal
*
* @brief   Original Packet EPCL Configuration for Egress Node
*/
static GT_VOID prvTgfEgressNodeOrigPacketEpclConfigSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_UNT             mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT             pattern;
    PRV_TGF_PCL_ACTION_STC                  action;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT        ruleFormat;
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC   interface;

    /* AUTODOC: Mirroring SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Mirroring Configuration =======\n");

    /* AUTODOC: save analyzer interface in index 1 */
    rc = prvTgfMirrorAnalyzerInterfaceGet(prvTgfDevNum, analyzerIndex,
            &prvTgfRestoreCfg.interface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfMirrorAnalyzerInterfaceGet: %d",
            prvTgfDevNum);

    /* AUTODOC: Set analyzer interface */
    interface.interface.type                = CPSS_INTERFACE_PORT_E;
    interface.interface.devPort.hwDevNum    = prvTgfDevNum;
    interface.interface.devPort.portNum     = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];
    rc = prvTgfMirrorAnalyzerInterfaceSet(analyzerIndex, &interface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                            "prvTgfMirrorAnalyzerInterfaceSet: %d", analyzerIndex);

    /* AUTODOC: Init PCL Engine for Egress PCL */
    rc = prvTgfPclDefPortInit(
            prvTgfPortsArray[PRV_TGF_TRANS_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_EGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);

    /* AUTODOC: Set PCL rule 0, To Capture the Original packet and Trigger egress mirroring */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    mask.ruleEgrStdIpL2Qos.common.pclId          = 0x3FF;
    pattern.ruleEgrStdIpL2Qos.common.pclId       = (GT_U16)PRV_TGF_PCL_DEFAULT_ID_MAC(
                                                            CPSS_PCL_DIRECTION_EGRESS_E,
                                                            CPSS_PCL_LOOKUP_0_E,
                                                            prvTgfPortsArray[PRV_TGF_TRANS_PORT_IDX_CNS]);
    ruleIndex                                    = 0;
    ruleFormat                                   = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;
    action.pktCmd                                = CPSS_PACKET_CMD_FORWARD_E;
    action.egressPolicy                          = GT_TRUE;
    action.mirror.cpuCode                        = CPSS_NET_IP_HDR_ERROR_E;
    action.mirror.mirrorToTxAnalyzerPortEn       = GT_TRUE;
    action.mirror.egressMirrorToAnalyzerIndex    = analyzerIndex;
    action.mirror.egressMirrorToAnalyzerMode     = CPSS_DXCH_MIRROR_EGRESS_NOT_DROPPED_E;

    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d", prvTgfDevNum, ruleFormat, ruleIndex);
}

/**
* internal prvTgfEgressNodeHaConfigSet function
* @endinternal
*
* @brief   Egress Node use case HA configurations
*/
static GT_VOID prvTgfEgressNodeHaConfigSet
(
    IN GT_BOOL              templateProfileEnable
)
{
    GT_STATUS                               rc;
    PRV_TGF_TUNNEL_START_ENTRY_UNT          tunnelEntry;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC        egressInfo;
    GT_U32                                  ii;

    /* Save ePort attributes configuration */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                                                       prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                                       &(prvTgfRestoreCfg.egressInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoGet);

    /* Set ePort attributes configuration */
    cpssOsMemSet(&egressInfo,0,sizeof(egressInfo));
    egressInfo                        = prvTgfRestoreCfg.egressInfo;
    egressInfo.tunnelStart            = GT_TRUE;
    egressInfo.tunnelStartPtr         = PRV_TGF_TUNNEL_START_INDEX_CNS;
    egressInfo.tsPassengerPacketType  = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;

    /* AUTODOC: config ePort#2 attributes: */
    /* AUTODOC:   Tunnel Start=ENABLE, tunnelStartPtr=8 */
    /* AUTODOC:   tsPassenger=ETHERNET */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                        prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                        &egressInfo);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

    /* AUTODOC: Set Generic IPv6 tunnel start entry */
    PRV_UTF_LOG0_MAC("======= Setting Tunnel Start Configuration =======\n");
    cpssOsMemSet(&tunnelEntry.ipv6Cfg,0,sizeof(tunnelEntry.ipv6Cfg));
    /*** Set a tunnel start entry ***/
    tunnelEntry.ipv6Cfg.tagEnable               = GT_FALSE;
    tunnelEntry.ipv6Cfg.vlanId                  = 0;
    tunnelEntry.ipv6Cfg.ttl                     = 33;
    tunnelEntry.ipv6Cfg.retainCrc               = GT_FALSE;
    tunnelEntry.ipv6Cfg.ipHeaderProtocol        = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;
    tunnelEntry.ipv6Cfg.greProtocolForEthernet  = 0x86DD;
    tunnelEntry.ipv6Cfg.greFlagsAndVersion      = 0;
    tunnelEntry.ipv6Cfg.udpSrcPort              = 0x2000;
    tunnelEntry.ipv6Cfg.udpDstPort              = PRV_TGF_UDP_DST_PORT_CNS;   /*4790*/
    cpssOsMemCpy(tunnelEntry.ipv6Cfg.macDa.arEther, prvTgfPacketTunnelL2Part.daMac, sizeof(TGF_MAC_ADDR));

    if(templateProfileEnable)
    {
        tunnelEntry.ipv6Cfg.profileIndex        = PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS;

        /* AUTODOC: configure tunnel start profile for IPv6-GRE */
        cpssOsMemSet(&profileData,0,sizeof(profileData));
        profileData.templateDataSize = PRV_TGF_TUNNEL_START_TEMPLATE_DATA_SIZE_12B_E; /* 4 bytes */

        rc = prvTgfTunnelStartGenProfileTableEntrySet(prvTgfDevNum,
                PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS,
                &profileData);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartGenProfileTableEntrySet");
    }

    /* tunnel next hop MAC DA, IP DA, IP SA */
    cpssOsMemCpy(tunnelEntry.ipv6Cfg.macDa.arEther, prvTgfOriginalPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));

    for (ii = 0; ii < 8; ii++)
    {
        tunnelEntry.ipv6Cfg.destIp.arIP[ii * 2]     = (GT_U8)(prvTgfTunnelStartSrcIPv6[ii] >> 8);
        tunnelEntry.ipv6Cfg.destIp.arIP[ii * 2 + 1] = (GT_U8) prvTgfTunnelStartSrcIPv6[ii];
        tunnelEntry.ipv6Cfg.srcIp.arIP[ii * 2]     = (GT_U8)(prvTgfTunnelStartDstIPv6[ii] >> 8);
        tunnelEntry.ipv6Cfg.srcIp.arIP[ii * 2 + 1] = (GT_U8) prvTgfTunnelStartDstIPv6[ii];
    }

    /* AUTODOC: add TS entry 8 with: */
    /* AUTODOC:   tunnelType = GENERIC_IPV6 */
    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_INDEX_CNS, CPSS_TUNNEL_GENERIC_IPV6_E, &tunnelEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntrySet: %d", prvTgfDevNum);
}

/**
* internal prvTgfEgressNodePhaConfigSet function
* @endinternal
*
* @brief   CurrentHop Telemetry use case configurations
*/
static GT_VOID prvTgfEgressNodePhaConfigSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC    commonInfo;
    CPSS_DXCH_PHA_THREAD_INFO_UNT           extInfo;

    /* GM does not support PHA */
    if(GT_TRUE == prvUtfIsGmCompilation() || (prvTgfOamEgressNodeSkipPha == GT_TRUE))
        return;

    cpssOsMemSet(&commonInfo,0,sizeof(CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC));
    commonInfo.stallDropCode                      = CPSS_NET_FIRST_USER_DEFINED_E;
    commonInfo.statisticalProcessingFactor        = 0;
    commonInfo.busyStallMode                      = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;

    /*************** Transit thread **********************************/
    /*AUTODOC: Assign tunnel termination thread to the ePort#2 - To Update the copy Reserved field */
    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum,
                                    prvTgfPortsArray[PRV_TGF_TRANS_PORT_IDX_CNS],
                                    GT_TRUE,
                                    PRV_TGF_PHA_THREAD_ID_TRANSIT);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaPortThreadIdSet FAILED for PHA_THREAD_ID_MIRRORING, rc = [%d]", rc);

    /* AUTODOC: Set the thread entry - INT_IOAM_EGRESS_SWITCH */
    cpssOsMemSet(&extInfo, 0, sizeof(extInfo));  /* No template */
    rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
                                     PRV_TGF_PHA_THREAD_ID_TRANSIT,
                                     &commonInfo,
                                     CPSS_DXCH_PHA_THREAD_TYPE_INT_IOAM_EGRESS_SWITCH_E,
                                     &extInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"[TGF]: cpssDxChPhaThreadIdEntrySet FAILED, rc = [%d]", rc);

    /*************** Egress thread **********************************/
    /* AUTODOC: Assign IOAM mirroring thread to the transit eport#1 - to update the node_id and hop_limit */
    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum,
                                    prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                    GT_TRUE,
                                    PRV_TGF_PHA_THREAD_ID_EGRESS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaPortThreadIdSet FAILED for PHA_THREAD_ID_EGRESS, rc = [%d]", rc);

    /* assign node_id, hop_lim will be taken from desc ttl */
    cpssOsMemSet(&extInfo, 0, sizeof(extInfo));
    extInfo.ioamEgressSwitchIpv6.node_id    = PRV_TGF_NODE_ID_CNS;

    /* AUTODOC: Set the thread entry - IOAM_EGRESS_SWITCH_IPV6 */
    rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
                                     PRV_TGF_PHA_THREAD_ID_EGRESS,
                                     &commonInfo,
                                     CPSS_DXCH_PHA_THREAD_TYPE_IOAM_EGRESS_SWITCH_IPV6_E,
                                     &extInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"[TGF]: cpssDxChPhaThreadIdEntrySet FAILED, rc = [%d]", rc);
}

/**
* internal prvTgfTreatIngressAsEgressConfigSet function
* @endinternal
*
* @brief   Treat ingress as egress.
*/
static GT_VOID prvTgfTreatIngressAsEgressConfigSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;

    /* AUTODOC: Enable EgressMirroredAsIngressMirrored on analyzer index 1 */
    rc = cpssDxChMirrorTreatEgressMirroredAsIngressMirroredEnableGet(prvTgfDevNum,
                                                                     analyzerIndex, &prvTgfRestoreCfg.treatIngressAsEgress);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
         "[TGF]: cpssDxChMirrorTreatEgressMirroredAsIngressMirroredEnableGet FAILED , rc = [%d]", rc);

    /* AUTODOC: Enable EgressMirroredAsIngressMirrored on analyzer index 1 */
    rc = cpssDxChMirrorTreatEgressMirroredAsIngressMirroredEnableSet(prvTgfDevNum,
                                                                     analyzerIndex, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
         "[TGF]: cpssDxChMirrorTreatEgressMirroredAsIngressMirroredEnableSet FAILED , rc = [%d]", rc);
}

/**
* internal tgfOamEgressNodeConfigSet function
* @endinternal
*
* @brief   CurrentHop Telemetry use case configurations
*/
GT_VOID tgfOamEgressNodeConfigSet
(
    GT_VOID
)
{
    /* Bridging Engine */
    prvTgfEgressNodeRoutingConfigSet(prvTgfPortsArray[PRV_TGF_TRANS_PORT_IDX_CNS]);

    /* Original Packet EPCL and Analyzer Configuration */
    prvTgfEgressNodeOrigPacketEpclConfigSet();

    /* HA Config
     *      - Add tunnel start with profile data */
    prvTgfEgressNodeHaConfigSet(GT_FALSE /* Profile data */);

    /* PHA - ePort to PHA asignment and template configuration */
    prvTgfEgressNodePhaConfigSet();

    /* Mirrored packet configuration
     *      - egress mirroring to ingress mirroring enable, OAM config */
    prvTgfTreatIngressAsEgressConfigSet();
}

/**
* internal tgfOamEgressNodeVerification function
* @endinternal
*
* @brief   CurrentHop Telemetry use case verification
*/
GT_VOID tgfOamEgressNodeVerification
(
    GT_VOID
)
{
    GT_STATUS                           rc = GT_OK;
    GT_U8                               packetBuf[PRV_TGF_PACKET_SIZE_CNS] = {0};
    GT_U32                              packetLen = PRV_TGF_PACKET_SIZE_CNS;
    GT_U32                              packetActualLength = 0;
    GT_U8                               queue = 0;
    GT_U8                               dev = 0;
    TGF_NET_DSA_STC                     rxParam;
    GT_U32                              pktOffset = PRV_TGF_IOAM_MD_OFFSET_CNS;

    rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&egrPortInterface,
            TGF_PACKET_TYPE_CAPTURE_E,
            GT_TRUE/*getFirst*/, GT_TRUE, packetBuf,
            &packetLen, &packetActualLength,
            &dev, &queue, &rxParam);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet: egrPortInterface %d", prvTgfDevNum);

    /* Hop Limit verification */
    UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_HOP_LIMIT_CNS, (GT_U32)packetBuf[pktOffset], "IOAM HOP limit is wrong is not as expected");
    pktOffset+=1;

    /* Node ID Verification */
    UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_NODE_ID_CNS,
                                (GT_U32)((packetBuf[pktOffset + 2]) | (packetBuf[pktOffset + 1] << 8) | ((packetBuf[pktOffset]) << 16)),
                                "IOAM node ID is wrong is not as expected");
}

/**
* @internal tgfOamEgressNodeConfigRestore function
* @endinternal
*
* @brief   CurrentHop Telemetry use case configurations restore.
* @note    1. Restore TTI Configuration
*          2. Restore Route Configuration
*          3. Restore EPCL Configuration
*          4. Restore PHA Configuration
*          5. Restore Mirrored packet Configuration
*          6. Restore Base Configuration
*
*/
GT_VOID tgfOamEgressNodeConfigRestore
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;

    /* -------------------------------------------------------------------------
     * 1. Restore EPCL Configuration
     */
    /* AUTODOC: Restore analyzer interface */
    rc = prvTgfMirrorAnalyzerInterfaceSet(analyzerIndex, &prvTgfRestoreCfg.interface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfMirrorAnalyzerInterfaceSet: %d", analyzerIndex);

    /* 2. Invalidate PCL rules */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, 0, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet, index = %d", 0);

    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, 1, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet, index = %d", 1);

    /* AUTODOC : disable all PCL configurations on the ports */
    prvTgfPclPortsRestoreAll();

    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TO_ANALYZER_E, GT_FALSE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfPclEgressPclPacketTypesSet");

    /* -------------------------------------------------------------------------
     * 3. Restore PHA Configuration
     */

    /* Clear IOAM mirroring thread from the transit eport#3 */
    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                        GT_FALSE, PRV_TGF_PHA_THREAD_ID_EGRESS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaPortThreadIdSet FAILED for PHA_THREAD_ID_EGRESS, rc = [%d]", rc);

    /* Clear tunnel termination thread from the ePort#2 */
    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TRANS_PORT_IDX_CNS],
                                        GT_FALSE, PRV_TGF_PHA_THREAD_ID_TRANSIT);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaPortThreadIdSet FAILED for PHA_THREAD_ID_MIRRORING, rc = [%d]", rc);

    /* -------------------------------------------------------------------------
     * 4. Restore Mirrored packet Configuration
     */

    /* AUTODOC: Disable EgressMirroredAsIngressMirrored on analyzer index 1 */
    rc = cpssDxChMirrorTreatEgressMirroredAsIngressMirroredEnableSet(prvTgfDevNum,
                                                           analyzerIndex, prvTgfRestoreCfg.treatIngressAsEgress);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, cpssDxChMirrorTreatEgressMirroredAsIngressMirroredEnableSet);

    /* AUTODOC: Restore ePort attributes */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                        prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                        &(prvTgfRestoreCfg.egressInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

    /* -------------------------------------------------------------------------
     * 5. Restore base Configuration
     */

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "tgfTrafficTableRxPcktTblClear");

    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "tgfTrafficTableRxStartCapture");
}

/**
* @internal tgfOamEgressNodeTest function
* @endinternal
*
* @brief   CurrentHop Telemetry use case test.
*/
GT_VOID tgfOamEgressNodeTest
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIter;

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum,
                    prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "prvTgfResetCountersEth: %d, %d",
                    prvTgfDevNum,
                    prvTgfPortsArray[portIter]);
    }

    packetInfo.numOfParts = sizeof(prvTgfIpv6PacketArray) / sizeof(prvTgfIpv6PacketArray[0]);
    packetInfo.partsArray = prvTgfIpv6PacketArray;
    packetInfo.totalLen   = TGF_PACKET_AUTO_CALC_LENGTH_CNS;

    /* setup receive portInterface for capturing */
    transPortInterface.type             = CPSS_INTERFACE_PORT_E;
    transPortInterface.devPort.hwDevNum = prvTgfDevNum;
    transPortInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_TRANS_PORT_IDX_CNS];
    egrPortInterface.type               = CPSS_INTERFACE_PORT_E;
    egrPortInterface.devPort.hwDevNum   = prvTgfDevNum;
    egrPortInterface.devPort.portNum    = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    /* AUTODOC: GENERATE TRAFFIC & Verify at ePort#3 and ePort#2 */
    /* enable capture on receive port ePort#3 and ePort#2 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &transPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgftrafficgeneratorporttxethcaptureset port - %d",
                                        transPortInterface.devPort.portNum);

    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
                        &egrPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgftrafficgeneratorporttxethcaptureset port - %d",
                                        egrPortInterface.devPort.portNum);

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, 1, 0, NULL);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfSetTxSetupEth");

    /* Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");

    /* disable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &transPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet port - %d",
            prvTgfPortsArray[PRV_TGF_TRANS_PORT_IDX_CNS]);

    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &egrPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet port - %d",
            egrPortInterface.devPort.portNum);

    /* AUTODOC: Verification Started */
    tgfOamEgressNodeVerification();
}

/**
* @internal prvTgfEgressMirroringMetadataOrigPacketEpclConfigSet function
* @endinternal
*
* @brief   Original Packet EPCL Configuration for egress mirroring metadata
*/
static GT_VOID prvTgfEgressMirroringMetadataOrigPacketEpclConfigSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC   interface;
    GT_U32                                  udbDataIndex;
    CPSS_QOS_ENTRY_STC                      qosEntry;
    PRV_TGF_COS_PROFILE_STC                 qosProfile;
    GT_U32                                  udbData[3][2]={/*UDB Index, Anchor offset */
                                                            {54,         106},
                                                            {55,         105},
                                                            {56,         104}};

    /* AUTODOC: Mirroring SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Mirroring Configuration =======\n");

    /* AUTODOC: save analyzer interface in index 1 */
    rc = prvTgfMirrorAnalyzerInterfaceGet(prvTgfDevNum, analyzerIndex,
            &prvTgfRestoreCfg.interface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfMirrorAnalyzerInterfaceGet: %d",
            prvTgfDevNum);

    /* AUTODOC: Set analyzer interface */
    interface.interface.type = CPSS_INTERFACE_PORT_E;
    interface.interface.devPort.hwDevNum    = prvTgfDevNum;
    interface.interface.devPort.portNum     = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];
    rc = prvTgfMirrorAnalyzerInterfaceSet(analyzerIndex, &interface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                            "prvTgfMirrorAnalyzerInterfaceSet: %d", analyzerIndex);

    /* AUTODOC: Enable TX mirroring on Target port */
    rc = cpssDxChMirrorTxPortSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_TRANS_PORT_IDX_CNS],
            GT_FALSE,   /* is physical false, eport is mapped to phy port */
            GT_TRUE,    /* Mirroring enabled */
            analyzerIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMirrorTxPortSet");

    /*********************** UDB configuration(UDB 54, 55, 56) ***************************/
    for (udbDataIndex=0; udbDataIndex<3; udbDataIndex++)
    {
        rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E,
                PRV_TGF_PCL_PACKET_TYPE_IPV6_E,
                CPSS_PCL_DIRECTION_EGRESS_E,
                udbData[udbDataIndex][0],
                PRV_TGF_PCL_OFFSET_METADATA_E,
                udbData[udbDataIndex][1]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d", prvTgfDevNum);
    }

    /* Qos Configuration */
    cpssOsMemSet(&qosProfile, 0, sizeof(qosProfile));
    cpssOsMemSet(&qosEntry, 0, sizeof(qosEntry));
    qosProfile.dropPrecedence = CPSS_DP_YELLOW_E;
    qosProfile.trafficClass   = PRV_TGF_TRAFFIC_CLASS_CNS;

    qosEntry.assignPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
    qosEntry.enableModifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    qosEntry.enableModifyUp   = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    qosEntry.qosProfileId     = PRV_TGF_CNC_GEN_DXCH_TEST_QOS_PROFILE0;

    rc = prvTgfCosProfileEntrySet(
            PRV_TGF_CNC_GEN_DXCH_TEST_QOS_PROFILE0, &qosProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet");

    rc = prvTgfCosPortQosConfigSet(
            prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS], &qosEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosPortQosConfigSet");
}

/**
* internal tgfEgressMirroringMetadataVerificationConfigSet function
* @endinternal
*
* @brief   Egress Mirroring verification use case - Configuration
*/
GT_VOID tgfEgressMirroringMetadataVerificationConfigSet
(
    GT_VOID
)
{
    /* Bridging Engine - Add MAC entry to bridge the packet to transit port(Analyzer port) */
    prvTgfEgressNodeRoutingConfigSet(prvTgfPortsArray[PRV_TGF_TRANS_PORT_IDX_CNS]);

    /* HA Config - Add tunnel start with profile data */
    prvTgfEgressNodeHaConfigSet(GT_TRUE /* Profile data */);

    /* EPCL configuration */
    prvTgfEgressMirroringMetadataOrigPacketEpclConfigSet();

    /* PHA config - Assign EGRESS_METADATA thread to Egress Port */
    if(prvTgfOamEgressNodeSkipPha == GT_FALSE)
    {
#ifdef WM_IMPLEMENTED
        /* Debug code to test not supported TXq fields in case of simulation */
        simPhaDebugQueueLimit                   = cpssOsRand() & 0xFFFFF;   /* 20 Bits */
        simPhaDebugQueueLen                     = cpssOsRand() & 0xFFFFF;   /* 20 Bits */
        snetPclDebugOriginalQueueFb             = cpssOsRand() & 0x3F;      /* 6  Bits */
#endif

        prvTgfPhaConfigSet(prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                PRV_TGF_PHA_THREAD_ID_EGRESS_METADATA,
                CPSS_DXCH_PHA_THREAD_TYPE_EGRESS_MIRRORING_METADATA_E);
    }

    /* Mirrored packet configuration
     *      - egress mirroring to ingress mirroring enable, OAM config */
    prvTgfTreatIngressAsEgressConfigSet();
}

/**
* internal tgfOamEgressMirroringMetadataVerification function
* @endinternal
*
* @brief   Egress Mirroring verification - Verify egress packet content
*/
GT_VOID tgfOamEgressMirroringMetadataVerification
(
    GT_VOID
)
{
    GT_STATUS                           rc = GT_OK;
    GT_U8                               packetBuf[PRV_TGF_PACKET_SIZE_CNS] = {0};
    GT_U32                              packetLen = PRV_TGF_PACKET_SIZE_CNS;
    GT_U32                              packetActualLength = 0;
    GT_U8                               queue = 0;
    GT_U8                               dev = 0;
    TGF_NET_DSA_STC                     rxParam;
    GT_U32                              pktOffset = PRV_TGF_MIRROR_METADATA_OFFSET_CNS;
#ifdef WM_IMPLEMENTED
    GT_U32                              pktData;
    GT_U32                              groupOfQueuesIndex;
#endif

    rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&egrPortInterface,
            TGF_PACKET_TYPE_CAPTURE_E,
            GT_TRUE/*getFirst*/, GT_TRUE, packetBuf,
            &packetLen, &packetActualLength,
            &dev, &queue, &rxParam);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet: egrPortInterface %d", prvTgfDevNum);

    /* Egress Packet verification (PHA Processed) */
    /* Timestamp (32b)*/

    pktOffset +=4;

#ifdef WM_IMPLEMENTED
    /* Original Queue Length (20b), Original Queue Limit (20b) */
    pktData = (GT_U32)((packetBuf[pktOffset + 2] >> 4) |
                       (packetBuf[pktOffset + 1] << 4) |
                       (packetBuf[pktOffset] << 12));
    UTF_VERIFY_EQUAL0_STRING_MAC(simPhaDebugQueueLen,
            pktData,
            "Queue Length is not as expected");
    pktOffset +=2;

    pktData = (GT_U32)((packetBuf[pktOffset + 2]        |
                       (packetBuf[pktOffset + 1] << 8)  |
                       ((packetBuf[pktOffset] & 0x0F) << 16)));
    UTF_VERIFY_EQUAL0_STRING_MAC(simPhaDebugQueueLimit,
            pktData,
            "Queue Limit is not as expected");
    pktOffset +=3;
    if(PRV_CPSS_SIP_6_CHECK_MAC(dev)==GT_TRUE)
    {
    /*get queue group index*/
        rc = prvCpssDxChTxqSip6GoQIndexGet(dev,prvTgfPortsArray[PRV_TGF_TRANS_PORT_IDX_CNS],
                GT_TRUE,&groupOfQueuesIndex);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvCpssDxChTxqSip6GoQIndexGet: portNum %d", prvTgfPortsArray[PRV_TGF_TRANS_PORT_IDX_CNS]);
    }
    else
    {
        groupOfQueuesIndex = prvTgfPortsArray[PRV_TGF_TRANS_PORT_IDX_CNS];
    }
    /* Original Queue Group Index 10b (UDB55 [1:0], UDB54 [7:0]) - Original egress port */
    pktData = ((packetBuf[pktOffset] << 2) | (packetBuf[pktOffset+1] >> 6));
    pktOffset +=1;
    UTF_VERIFY_EQUAL0_STRING_MAC(groupOfQueuesIndex,
            pktData,
            "Queue group index is not as expected");

    /* Original Queue Offset (UDB 55 [5:2]) - Traffic class */
    pktData = ((packetBuf[pktOffset] >> 2) & 0xF);
    UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_TRAFFIC_CLASS_CNS,
            pktData,
            "Queue offset is not as expected");

    /* Original Queue Fb 6b (UDB56 [5:0]) */
    pktData = (((packetBuf[pktOffset] & 0x3) << 4 ) |
               ((packetBuf[pktOffset+1] >> 4) & 0xF));
    UTF_VERIFY_EQUAL0_STRING_MAC(snetPclDebugOriginalQueueFb,
            pktData,
            "Queue Fb is not as expected");
#endif
}

/**
* internal tgfSrv6ThreeSegmentConfigSet function
* @endinternal
*
* @brief   Egress Mirroring verification use case - Traffic and verify
*/
GT_VOID tgfEgressMirroringMetadataVerificationTrafficTest
(
    GT_VOID
)
{
    GT_STATUS               rc = GT_OK;

    /* AUTODOC: reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    packetInfo.numOfParts = sizeof(prvTgfIpv6PacketArray_withOutVxlan) / sizeof(prvTgfIpv6PacketArray_withOutVxlan[0]);
    packetInfo.partsArray = prvTgfIpv6PacketArray_withOutVxlan;
    packetInfo.totalLen   = TGF_PACKET_AUTO_CALC_LENGTH_CNS;

    /* setup receive portInterface for capturing */
    transPortInterface.type             = CPSS_INTERFACE_PORT_E;
    transPortInterface.devPort.hwDevNum = prvTgfDevNum;
    transPortInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_TRANS_PORT_IDX_CNS];
    egrPortInterface.type               = CPSS_INTERFACE_PORT_E;
    egrPortInterface.devPort.hwDevNum   = prvTgfDevNum;
    egrPortInterface.devPort.portNum    = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    /* AUTODOC: GENERATE TRAFFIC & Verify at ePort#3 and ePort#2 */
    /* enable capture on receive port ePort#3 and ePort#2 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&transPortInterface,
            TGF_CAPTURE_MODE_MIRRORING_E,
            GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "tgftrafficgeneratorporttxethcaptureset port - %d",
            transPortInterface.devPort.portNum);

    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&egrPortInterface,
            TGF_CAPTURE_MODE_MIRRORING_E,
            GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "tgftrafficgeneratorporttxethcaptureset port - %d",
            egrPortInterface.devPort.portNum);

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth");

    /* AUTODOC: Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth");

    /* AUTODOC: Disable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&transPortInterface,
            TGF_CAPTURE_MODE_MIRRORING_E,
            GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "tgfTrafficGeneratorPortTxEthCaptureSet port - %d",
            prvTgfPortsArray[PRV_TGF_TRANS_PORT_IDX_CNS]);

    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&egrPortInterface,
            TGF_CAPTURE_MODE_MIRRORING_E,
            GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "tgfTrafficGeneratorPortTxEthCaptureSet port - %d",
            egrPortInterface.devPort.portNum);

    /* AUTODOC: Verification Started */
    tgfOamEgressMirroringMetadataVerification();
}

/**
* internal sMirroringMetadataVerificationConfigRestore function
* @endinternal
*
* @brief   Egress Mirroring verification use case - Configuration restore
*/
GT_VOID tgfEgressMirroringMetadataVerificationConfigRestore
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_COS_PROFILE_STC                 qosProfile;

    /* -------------------------------------------------------------------------
     * 1. Restore EPCL Configuration
     */
    /* AUTODOC: Restore analyzer interface */
    rc = prvTgfMirrorAnalyzerInterfaceSet(analyzerIndex, &prvTgfRestoreCfg.interface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfMirrorAnalyzerInterfaceSet: %d", analyzerIndex);

     /* AUTODOC: Disable TX mirroring on Target port */
    rc = cpssDxChMirrorTxPortSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_TRANS_PORT_IDX_CNS],
            GT_FALSE,   /* is physical false, eport is mapped to phy port */
            GT_FALSE,    /* Mirroring enabled */
            analyzerIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMirrorTxPortSet");

    /* -------------------------------------------------------------------------
     * 3. Restore PHA Configuration
     */
    /* Clear IOAM mirroring thread from the transit eport#3 */
    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
            GT_FALSE,
            PRV_TGF_PHA_THREAD_ID_EGRESS_METADATA);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "[TGF]: cpssDxChPhaPortThreadIdSet FAILED for PHA_THREAD_ID_EGRESS_METADATA, rc = [%d]", rc);

    /* -------------------------------------------------------------------------
     * 4. Restore Mirrored packet Configuration
     */
    /* AUTODOC: Disable EgressMirroredAsIngressMirrored on analyzer index 1 */
    rc = cpssDxChMirrorTreatEgressMirroredAsIngressMirroredEnableSet(prvTgfDevNum,
                                                           analyzerIndex,
                                                           prvTgfRestoreCfg.treatIngressAsEgress);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, cpssDxChMirrorTreatEgressMirroredAsIngressMirroredEnableSet);

    /* AUTODOC: Restore ePort attributes */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                        prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                        &(prvTgfRestoreCfg.egressInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

    /* Restore COS profile */
    cpssOsMemSet(&qosProfile, 0, sizeof(qosProfile));
    rc = prvTgfCosProfileEntrySet(
            PRV_TGF_CNC_GEN_DXCH_TEST_QOS_PROFILE0, &qosProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet");

    /* -------------------------------------------------------------------------
     * 5. Restore base Configuration
     */
    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "tgfTrafficTableRxPcktTblClear");

    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "tgfTrafficTableRxStartCapture");

#ifdef WM_IMPLEMENTED
    /* restore Desc debug params */
    simPhaDebugQueueLimit                   = 0;
    simPhaDebugQueueLen                     = 0;
    snetPclDebugOriginalQueueFb             = 0;
#endif
}
