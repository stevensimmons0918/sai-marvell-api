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
* @file prvTgfOamCurrentHopTelemetryUseCase.c
*
* @brief Current Hop Telemetry use case testing
*
* @version   1
********************************************************************************
*/
#include <oam/prvTgfOamCurrentHopTelemetryUseCase.h>
#include <common/tgfIpGen.h>
#include <common/tgfMirror.h>
#include <cpss/dxCh/dxChxGen/pha/cpssDxChPha.h>
#include <cpss/dxCh/dxChxGen/oam/cpssDxChOam.h>
#include <exactMatch/prvTgfExactMatchBasicDefaultActionPcl.h>
#include <common/tgfCosGen.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define PRV_TGF_DEFAULT_VLANID_CNS          1
#define PRV_TGF_PHA_THREAD_ID_EGRESS_CNS    14
#define PRV_TGF_PHA_THREAD_ID_MIRRORING_CNS 13

/* template profile index */
#define PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS  1

/* Tunnel start entry index */
#define PRV_TGF_TUNNEL_START_INDEX_CNS      8

/* port to Send/Receive traffic*/
#define PRV_TGF_INGRESS_PORT_IDX_CNS        3
#define PRV_TGF_TRANS_PORT_IDX_CNS          2
#define PRV_TGF_EGR_PORT_IDX_CNS            1

/* billing flow Id based counting index base  */
#define PRV_TGF_BILLING_COUNTERS_FLOW_ID_INDEX_BASE_CNS     10

/* billing flow Id minimal for counting  */
#define PRV_TGF_BILLING_COUNTERS_FLOW_ID_MIN_CNS            20

/* billing flow Id maximal for counting  */
#define PRV_TGF_BILLING_COUNTERS_FLOW_ID_MAX_CNS            50

/* packet flow Id to set by PCL Action  */
#define PRV_TGF_EPCL_FLOW_ID_CNS                            25

#define PRV_TGF_BURST_COUNT_CNS                             2
#define PRV_TGF_QOS_PROFILE_INDEX_CNS                       2
#define PRV_TGF_TC_CNS                                      7

/* billing flow Id based counting index  */
#define PRV_TGF_BILLING_COUNTERS_FLOW_ID_INDEX_CNS   \
    (PRV_TGF_BILLING_COUNTERS_FLOW_ID_INDEX_BASE_CNS \
     + PRV_TGF_EPCL_FLOW_ID_CNS                      \
     - PRV_TGF_BILLING_COUNTERS_FLOW_ID_MIN_CNS)

/* the Mask for IP address */
static TGF_IPV4_ADDR    prvTgfFullMaskIp    = {255, 255, 255, 255};

/* the LPM DB id for LPM Table */
static GT_U32           prvTgfLpmDBId       = 0;

/* the virtual router id */
static GT_U32           prvTgfVrId          = 0;

/* the Route entry index for UC Route entry Table */
static GT_U32           prvTgfRouteEntryBaseIndex = 10;

static TGF_PACKET_STC   packetInfo;

/* analyzer index */
static GT_U32           analyzerIndex       = 1;

static GT_U32           prvTgfTtiRuleIndex;

static GT_U32           oamIndex = 1;

static CPSS_INTERFACE_INFO_STC transPortInterface, egrPortInterface;
#define PRV_TGF_GRE_ETHER_TYPE_CNS 0X6558

#define PRV_TGF_GRE_PKT_OFFSET  (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS + 2)

/* size of packet */
#define PRV_TGF_PACKET_SIZE_CNS 140

/* packet info */
/* Mirrored packet, Tunnel part */
static TGF_PACKET_L2_STC prvTgfMirroredPacketTunnelL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x36, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x36, 0x03}                /* saMac */
};

static TGF_PACKET_IPV4_STC prvTgfMirroredPacketTunnelIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x3f,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x04,               /* protocol */
    0x5EA0,             /* csum */
    {20,  1,  1,  3},   /* srcAddr */
    {20,  1,  1,  2}    /* dstAddr */
};

#if 0
static TGF_PACKET_VXLAN_GPE_STC prvTgfVxlanGpePart = {
    0x0C,               /* Flags */
    0,                  /* Reserved */
    0x55,               /* Next Protocol */
    0xF4240,            /* VXLAN Network Identifier (VNI) - 24Bits */
    0                   /* Reserved */
};

static TGF_PACKET_IOAM_STC prvTgfIoamPart = {
   0,                   /* IOAM-Trace-Type */
   0,                   /* Node-Len */
   0,                   /* Flags */
   0                    /* Remaining Length */
};
#endif


/* Original Ingress Packet */
/* Tunnel part */
static TGF_PACKET_L2_STC prvTgfOriginalPacketTunnelL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x03}                /* saMac */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

static TGF_PACKET_IPV4_STC prvTgfOriginalPacketTunnelIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x3f,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x04,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,      /* csum */ /*0x5EA0*/
    {10,  1,  1,  4},   /* srcAddr */
    {10,  1,  1,  3}    /* dstAddr */
};

/* packet's IPv4 over IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2b,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0xFF,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,      /* csum */ /*0x5EA0*/
    {192,  168,  1,  4},   /* srcAddr */
    {192,  168,  1,  3}    /* dstAddr */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfOriginalPacketTunnelL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfOriginalPacketTunnelIpv4Part},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC   interface;
    GT_BOOL                                 prvTgfEnableMirror;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC        egressInfo;
    PRV_TGF_TUNNEL_START_ENTRY_UNT          tunnelEntry;
    PRV_TGF_OAM_COMMON_CONFIG_STC           oamConfig;
    GT_BOOL                                 lmStampingEnable;
    CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT cfgTabAccMode;
    GT_BOOL                                 meteringCountingStatus;
} prvTgfRestoreCfg;


/*************************** Test config ***********************************/
static GT_VOID prvTgfCurrentHopTelemetryTtiConfigSet
(
    GT_U32      ttHeaderLength
)
{
    GT_STATUS                rc = GT_OK;
    PRV_TGF_TTI_ACTION_2_STC ttiAction;
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;
    prvTgfTtiRuleIndex       = prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(PRV_TTI_LOOKUP_0, 3);

    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: enable the TTI lookup for IPv4 at the port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS], PRV_TGF_TTI_KEY_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: enable IPv4 TTI lookup for only tunneled packets received on port 0 */
    rc = prvTgfTtiPortIpv4OnlyTunneledEnableSet(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortIpv4OnlyTunneledEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: configure specific TTI action values */
    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));
    ttiAction.ttPassengerPacketType            = PRV_TGF_TTI_PASSENGER_IPV4_E;
    ttiAction.redirectCommand                  = PRV_TGF_TTI_NO_REDIRECT_E;
    ttiAction.ttHeaderLength                   = ttHeaderLength;
    ttiAction.tunnelTerminate                  = GT_TRUE;
    ttiAction.ttPassengerPacketType            = PRV_TGF_TTI_PASSENGER_IPV4_E;
    ttiAction.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    ttiAction.egressInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS];

    /* AUTODOC: set TTI Pattern */
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    ttiPattern.ipv4.common.pclId        = 1;
    cpssOsMemCpy(ttiPattern.ipv4.destIp.arIP, prvTgfOriginalPacketTunnelIpv4Part.dstAddr, sizeof(TGF_IPV4_ADDR));

    /* AUTODOC: set TTI Mask */
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
    ttiMask.ipv4.common.pclId           = 0x3FF;
    cpssOsMemCpy(ttiMask.ipv4.destIp.arIP, prvTgfFullMaskIp, sizeof(TGF_IPV4_ADDR));

    /* AUTODOC: add TTI rule 1 with: */
    /* AUTODOC:   key IPv4, cmd FORWARD, redirectCmd TO_ROUTER_LOOKUP */
    /* AUTODOC:   pattern dstIp = 10.1.1.4 */
    /* AUTODOC:   action Tunnel Terminate, PASSENGER_IPV4, port=0 */
    rc = prvTgfTtiRule2Set(prvTgfTtiRuleIndex, PRV_TGF_TTI_KEY_IPV4_E,
                           &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet: %d", prvTgfDevNum);

    /* Verify added TTI Rule */
    cpssOsMemSet(&ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet(&ttiMask, 0, sizeof(ttiMask));
    cpssOsMemSet(&ttiAction, 0, sizeof(ttiAction));

    /* AUTODOC: check added TTI Rule */
    rc = prvTgfTtiRule2Get(prvTgfDevNum, prvTgfTtiRuleIndex, PRV_TGF_TTI_KEY_IPV4_E,
                           &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleGet: %d %d", prvTgfDevNum, ttiPattern.ipv4.destIp.u32Ip);
}

/**
* @internal prvTgfCurrentHopTelemetryRoutingConfigSet function
* @endinternal
*
* @brief   Routing configuration for current hop telemetry use case
*/
static GT_VOID prvTgfCurrentHopTelemetryRoutingConfigSet
(
    GT_U8    nextHopPortNum
)
{
    GT_IPADDR                               ipAddr;
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT    nextHopInfo;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *regularEntryPtr;

    /* AUTODOC: enable IPv4 Unicast Routing on sending port */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_INGRESS_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                    CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                                    prvTgfDevNum, PRV_TGF_INGRESS_PORT_IDX_CNS);

    /* AUTODOC: enable IPv4 Unicast Routing on Default Vlan */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_DEFAULT_VLANID_CNS, CPSS_IP_UNICAST_E,
                                    CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                    prvTgfDevNum, PRV_TGF_DEFAULT_VLANID_CNS);

    /* AUTODOC: write a UC Route entry to the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));
    regularEntryPtr = &routeEntriesArray[0];
    regularEntryPtr->cmd                        = CPSS_PACKET_CMD_ROUTE_E;
    regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
    regularEntryPtr->nextHopVlanId              = PRV_TGF_DEFAULT_VLANID_CNS;
    regularEntryPtr->nextHopInterface.devPort.hwDevNum  = prvTgfDevNum;
    regularEntryPtr->nextHopInterface.devPort.portNum = nextHopPortNum;
    regularEntryPtr->qosProfileMarkingEnable = GT_TRUE;
    regularEntryPtr->qosProfileIndex = PRV_TGF_QOS_PROFILE_INDEX_CNS;

    rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex,
            routeEntriesArray, 1 /* numOfRouteEntries */);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

    /* fill a nexthope info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    /* fill a destination IP address for the prefix */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipAddr.arIP));

    /* AUTODOC: add IPv4 UC prefix 192.168.1.4/32 */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(prvTgfLpmDBId, prvTgfVrId, ipAddr, 32, &nextHopInfo, GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);
}

/**
* @internal prvTgfCurrentHopTelemetryOrigPacketConfigSet function
* @endinternal
*
* @brief   Original Packet Configuration for current hop telemetry use case
*/
static GT_VOID prvTgfCurrentHopTelemetryOrigPacketConfigSet
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
    interface.interface.type = CPSS_INTERFACE_PORT_E;
    interface.interface.devPort.hwDevNum  = prvTgfDevNum;
    interface.interface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];
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

/* debug flag to disable using of PHA for the test */
static GT_BOOL prvTgfCurrentHopSkipPha = GT_FALSE;
GT_BOOL prvTgfCurrentHopSkipPhaSet(GT_BOOL newSkipPha)
{
    GT_BOOL oldSkipPha = prvTgfCurrentHopSkipPha;

    prvTgfCurrentHopSkipPha = newSkipPha;

    return oldSkipPha;
}

/**
* internal prvTgfCurrentHopTelemetryPhaConfigSet function
* @endinternal
*
* @brief   CurrentHop Telemetry use case configurations
*/
static GT_VOID prvTgfCurrentHopTelemetryPhaConfigSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC    commonInfo;
    CPSS_DXCH_PHA_THREAD_INFO_UNT           extInfo;

    /* GM does not support PHA */
    if(GT_TRUE == prvUtfIsGmCompilation() || (prvTgfCurrentHopSkipPha == GT_TRUE))
        return;

    /* AUTODOC: Assign IOAM mirroring thread to the transit eport#2 - to update the ERSPAN header from copy-reserved field*/
    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                        GT_TRUE, PRV_TGF_PHA_THREAD_ID_MIRRORING_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaPortThreadIdSet FAILED for PHA_THREAD_ID_EGRESS, rc = [%d]", rc);

    cpssOsMemSet(&commonInfo, 0, sizeof(commonInfo));
    extInfo.notNeeded = 0;
    commonInfo.statisticalProcessingFactor = 0;
    commonInfo.busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;
    commonInfo.stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;

    /* AUTODOC: Set the thread entry - THREAD_ID_MIRRORING */
    rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
                                     PRV_TGF_PHA_THREAD_ID_MIRRORING_CNS,
                                     &commonInfo,
                                     CPSS_DXCH_PHA_THREAD_TYPE_INT_IOAM_MIRRORING_E,
                                     &extInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"[TGF]: cpssDxChPhaThreadIdEntrySet FAILED, rc = [%d]", rc);

    /*AUTODOC: Assign tunnel termination thread to the ePort#3 - To Update the copy Reserved field */
    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TRANS_PORT_IDX_CNS],
                                        GT_TRUE, PRV_TGF_PHA_THREAD_ID_EGRESS_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaPortThreadIdSet FAILED for PHA_THREAD_ID_MIRRORING, rc = [%d]", rc);

    /* AUTODOC: Set the thread entry - THREAD_ID_EGRES */
    rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
                                     PRV_TGF_PHA_THREAD_ID_EGRESS_CNS,
                                     &commonInfo,
                                     CPSS_DXCH_PHA_THREAD_TYPE_INT_IOAM_EGRESS_SWITCH_E,
                                     &extInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,"[TGF]: cpssDxChPhaThreadIdEntrySet FAILED, rc = [%d]", rc);
}

/**
* internal prvTgfCurrentHopTelemetryMirroredPacketConfigSet function
* @endinternal
*
* @brief   CurrentHop Telemetry use case configurations
*/
static GT_VOID prvTgfCurrentHopTelemetryMirroredPacketConfigSet
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
    PRV_TGF_POLICER_BILLING_INDEX_CFG_STC   billingIndexCfg;

    PRV_TGF_TUNNEL_START_ENTRY_UNT          tunnelEntry;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC        egressInfo;
    PRV_TGF_POLICER_BILLING_ENTRY_STC       prvTgfBillingCntr; /* Billing Counters */
    PRV_TGF_TUNNEL_START_GEN_IP_PROFILE_STC profileData;
    PRV_TGF_OAM_COMMON_CONFIG_STC           oamConfig;

    /* AUTODOC: Enable EgressMirroredAsIngressMirrored on analyzer index 1 */
    rc = cpssDxChMirrorTreatEgressMirroredAsIngressMirroredEnableSet(prvTgfDevNum,
                                                                     analyzerIndex, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
         "[TGF]: cpssDxChMirrorTreatEgressMirroredAsIngressMirroredEnableSet FAILED , rc = [%d]", rc);

    /* Save ePort attributes configuration */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                                                       prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                                       &(prvTgfRestoreCfg.egressInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoGet);

    /* Set ePort attributes configuration */
    cpssOsMemSet(&egressInfo,0,sizeof(egressInfo));
    egressInfo = prvTgfRestoreCfg.egressInfo;
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

    /* AUTODOC: Set Generic IPv4 tunnel start entry */
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));
    tunnelEntry.ipv4Cfg.tagEnable              = GT_TRUE;
    tunnelEntry.ipv4Cfg.vlanId                 = PRV_TGF_DEFAULT_VLANID_CNS;
    tunnelEntry.ipv4Cfg.ipHeaderProtocol       = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;
    tunnelEntry.ipv4Cfg.ethType                = CPSS_TUNNEL_GRE0_ETHER_TYPE_E;
    tunnelEntry.ipv4Cfg.profileIndex           = PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS;
    tunnelEntry.ipv4Cfg.greProtocolForEthernet = PRV_TGF_GRE_ETHER_TYPE_CNS;
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.macDa.arEther, prvTgfMirroredPacketTunnelL2Part.daMac,    sizeof(TGF_MAC_ADDR));
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.destIp.arIP, prvTgfMirroredPacketTunnelIpv4Part.dstAddr, sizeof(TGF_IPV4_ADDR));
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.srcIp.arIP, prvTgfMirroredPacketTunnelIpv4Part.srcAddr, sizeof(TGF_IPV4_ADDR));

    /* AUTODOC: add Tunnel Start entry 8 with: */
    /* AUTODOC:   tunnelType=Generic IPv4 */
    /* AUTODOC:   tagEnable=TRUE, vlanId=1, ipHeaderProtocol=GRE */
    /* AUTODOC:   MACDA=00:00:00:00:36:02 , DIP=20.1.1.2, SIP=20.1.1.3  */
    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_INDEX_CNS, CPSS_TUNNEL_GENERIC_IPV4_E, &tunnelEntry);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfTunnelStartEntrySet);

    /* AUTODOC: configure tunnel start profile(16Bytes as "Zero") */
    cpssOsMemSet(&profileData,0,sizeof(profileData));
    profileData.templateDataSize = PRV_TGF_TUNNEL_START_TEMPLATE_DATA_SIZE_16B_E; /* 16 byte */
    rc = prvTgfTunnelStartGenProfileTableEntrySet(prvTgfDevNum,
                                                  PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS, &profileData);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartGenProfileTableEntrySet: %d", prvTgfDevNum);

    /**** EPCL ****/
    /* AUTODOC: init PCL Engine for Egress PCL for ePort#2 */
    rc = prvTgfPclDefPortInit(
            prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_EGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);

    /* enable EPCL on Analyzer port */
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TO_ANALYZER_E, GT_TRUE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: Configure "Access to the EPCL configuration" table with Port Mode */
    rc = cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeGet(prvTgfDevNum,
                                   &prvTgfRestoreCfg.cfgTabAccMode);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeGet");

    rc = cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet(prvTgfDevNum,
                                CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet");

    /* AUTODOC: set PCL rule 0 */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    mask.ruleEgrStdIpL2Qos.common.pclId          = 0x3FF;
    pattern.ruleEgrStdIpL2Qos.common.pclId       = (GT_U16)PRV_TGF_PCL_DEFAULT_ID_MAC(
                                                            CPSS_PCL_DIRECTION_EGRESS_E,
                                                            CPSS_PCL_LOOKUP_0_E,
                                                            prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS]);
    ruleIndex                                    = 1;
    ruleFormat                                   = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;
    action.pktCmd                                = CPSS_PACKET_CMD_FORWARD_E;
    action.oam.oamProcessEnable                  = GT_TRUE;
    action.oam.oamProfile                        = PRV_TGF_OAM_PROFILE_INDEX_CNS;
    action.flowId                                = PRV_TGF_EPCL_FLOW_ID_CNS;
    action.policer.policerEnable                 = PRV_TGF_PCL_POLICER_ENABLE_COUNTER_ONLY_E;
    action.egressPolicy                          = GT_TRUE;
    action.epclPhaInfo.phaThreadType             = PRV_TGF_PCL_ACTION_EGRESS_PHA_THREAD_INT_IOAM_EGRESS_E;
    action.epclPhaInfo.phaThreadUnion.epclIntIoamMirror.erspanIII_P = GT_TRUE;

    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d", prvTgfDevNum, ruleFormat, ruleIndex);

    /**** EOAM ****/
    cpssOsMemSet(&oamConfig, 0, sizeof(oamConfig));
    oamConfig.oamEntryIndex = PRV_TGF_EPCL_FLOW_ID_CNS;

    /* Get OAM common configuration */
    prvTgfOamConfigurationGet(&oamConfig);
    cpssOsMemCpy(&(prvTgfRestoreCfg.oamConfig), &oamConfig, sizeof(oamConfig)); /* For restoration */
    oamConfig.oamEntry.oamPtpOffsetIndex = oamIndex;
    oamConfig.oamEntry.opcodeParsingEnable = GT_FALSE;
    oamConfig.oamEntry.lmCountingMode = PRV_TGF_OAM_LM_COUNTING_MODE_ENABLE_E;
    oamConfig.oamEntry.lmCounterCaptureEnable = GT_TRUE;

    /* AUTODOC: Set common OAM configurations */
    prvTgfOamConfigurationSet(&oamConfig);

    /* AUTODOC: Set OAM LM offset table */
    prvTgfOamLmOffsetTableSet(oamIndex,
            PRV_TGF_OAM_TLV_DATA_OFFSET_STC + 4 * oamIndex);

    /* AUTODOC: Enable OAM processing - Egress */
    rc = prvTgfOamEnableSet(PRV_TGF_OAM_STAGE_TYPE_EGRESS_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, "prvTgfOamEnableSet: EOAM enable");


    /**** EPLR ****/
    cpssOsMemSet(&billingIndexCfg, 0, sizeof(billingIndexCfg));
    billingIndexCfg.billingIndexMode       = PRV_TGF_POLICER_BILLING_INDEX_MODE_FLOW_ID_E;
    billingIndexCfg.billingFlowIdIndexBase = PRV_TGF_BILLING_COUNTERS_FLOW_ID_INDEX_BASE_CNS;
    billingIndexCfg.billingMinFlowId       = PRV_TGF_BILLING_COUNTERS_FLOW_ID_MIN_CNS;
    billingIndexCfg.billingMaxFlowId       = PRV_TGF_BILLING_COUNTERS_FLOW_ID_MAX_CNS;

    /* AUTODOC: Set Egress policer counter based on Flow Id */
    rc = prvTgfPolicerFlowIdCountingCfgSet(PRV_TGF_POLICER_STAGE_EGRESS_E, &billingIndexCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPolicerFlowIdCountingCfgSet");

    /* AUTODOC: Flush internal Write Back Cache (WBC) of counting entries */
    rc = prvTgfPolicerCountingWriteBackCacheFlush(PRV_TGF_POLICER_STAGE_EGRESS_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingWriteBackCacheFlush");

    cpssOsMemSet(&prvTgfBillingCntr, 0, sizeof(prvTgfBillingCntr));
    prvTgfBillingCntr.billingCntrMode = PRV_TGF_POLICER_BILLING_CNTR_PACKET_E;
    prvTgfBillingCntr.packetSizeMode  = CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;
    prvTgfBillingCntr.lmCntrCaptureMode = PRV_TGF_POLICER_LM_COUNTER_CAPTURE_MODE_INSERT_E;
    rc = prvTgfPolicerBillingEntrySet(prvTgfDevNum,
                                      PRV_TGF_POLICER_STAGE_EGRESS_E,
                                      PRV_TGF_BILLING_COUNTERS_FLOW_ID_INDEX_CNS,
                                      &prvTgfBillingCntr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerBillingEntrySet: %d", prvTgfDevNum);

    /* Set LM counter capture enable */
    rc = prvTgfPolicerLossMeasurementCounterCaptureEnableSet(
                                      PRV_TGF_POLICER_STAGE_EGRESS_E,
                                      GT_TRUE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfPolicerLossMeasurementCounterCaptureEnableSet");

    /* AUTODOC: Disable Stamping of LM counter in packet */
    rc = cpssDxChOamLmStampingEnableGet(prvTgfDevNum, &prvTgfRestoreCfg.lmStampingEnable);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "cpssDxChOamLmStampingEnableGet");

    rc = cpssDxChOamLmStampingEnableSet(prvTgfDevNum, GT_FALSE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "cpssDxChOamLmStampingEnableSet - Disable");

    /* AUTODOC: Enable Metering and counting for "TO_ANALYZER" packets */
    rc = cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableGet(prvTgfDevNum,
                                        &prvTgfRestoreCfg.meteringCountingStatus);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableGet");

    rc = cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableSet(prvTgfDevNum, GT_TRUE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableSet - Enable");
}

/**
* internal tgfOamCurrentHopTelemetryConfigSet function
* @endinternal
*
* @brief   CurrentHop Telemetry use case configurations
*/
GT_VOID tgfOamCurrentHopTelemetryConfigSet
(
    GT_VOID
)
{
    GT_U32 ttHeaderLength = TGF_IPV4_HEADER_SIZE_CNS;

    /* TTI Configuration */
    prvTgfCurrentHopTelemetryTtiConfigSet(ttHeaderLength);

    /* Routing Engine */
    prvTgfCurrentHopTelemetryRoutingConfigSet(prvTgfPortsArray[PRV_TGF_TRANS_PORT_IDX_CNS]);

    /* Original Packet EPCL and Analyzer Configuration */
    prvTgfCurrentHopTelemetryOrigPacketConfigSet();

    /* PHA */
    prvTgfCurrentHopTelemetryPhaConfigSet();

    /* Mirrored packet configuration */
    prvTgfCurrentHopTelemetryMirroredPacketConfigSet();
}

/**
* internal tgfOamCurrentHopTelemetryVerification function
* @endinternal
*
* @brief   CurrentHop Telemetry use case verification
*/
GT_VOID tgfOamCurrentHopTelemetryVerification
(
    GT_VOID
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_POLICER_BILLING_ENTRY_STC   prvTgfBillingCntr;
    GT_U8                               packetBuf[PRV_TGF_BURST_COUNT_CNS][PRV_TGF_PACKET_SIZE_CNS] = {{0}, {0}};
    GT_U32                              packetLen = PRV_TGF_PACKET_SIZE_CNS;
    GT_U32                              packetActualLength = 0;
    GT_U8                               queue = 0;
    GT_U8                               dev = 0;
    TGF_NET_DSA_STC                     rxParam;
    GT_U32                              pktOffset = PRV_TGF_GRE_PKT_OFFSET;
    GT_U8                               i = 0;
    GT_BOOL                             getFirst = GT_TRUE;
    GT_U32                              greEtherType = 0, lmCounter[PRV_TGF_BURST_COUNT_CNS]= {0};
    GT_U32                              erspanIIIPlatSpec, erspanIIIPlatSpecExpected[2] = {0};

    erspanIIIPlatSpecExpected[0] = prvTgfPortsArray[PRV_TGF_TRANS_PORT_IDX_CNS] << 15;
    erspanIIIPlatSpecExpected[1] = ((PRV_TGF_TC_CNS << 25) | (prvTgfPortsArray[PRV_TGF_TRANS_PORT_IDX_CNS] << 15));

    for (i = 0; i<PRV_TGF_BURST_COUNT_CNS; i++)
    {
        pktOffset = PRV_TGF_GRE_PKT_OFFSET;
        rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&egrPortInterface,
                                                    TGF_PACKET_TYPE_CAPTURE_E,
                                                    getFirst, GT_TRUE, packetBuf[i],
                                                    &packetLen, &packetActualLength,
                                                    &dev, &queue, &rxParam);

        /* AUTODOC: Verify the GRE Header part */
        greEtherType = (GT_U32)(packetBuf[i][pktOffset + 3] | (packetBuf[i][pktOffset + 2] << 8) |
                               (packetBuf[i][pktOffset + 1] << 16) | (packetBuf[i][pktOffset] << 24));
        UTF_VERIFY_EQUAL1_STRING_MAC(greEtherType,
                                     PRV_TGF_GRE_ETHER_TYPE_CNS,
                                     "GRE Ethertype is not as expected", i);
        pktOffset +=4;

        lmCounter[i] = (GT_U32)(packetBuf[i][pktOffset + 3] | (packetBuf[i][pktOffset + 2] << 8) |
                               (packetBuf[i][pktOffset + 1] << 16) | (packetBuf[i][pktOffset] << 24));
        pktOffset +=4;

        /* AUTODOC: Verify ERSPAN_III <P> & <FT> */
        /* ERSPAN<P> = PHA Metadata[31]     = 1,
         * ERSPAN<FT>=PHA Metadata[30:26]   = 0 */
        pktOffset +=10;
        UTF_VERIFY_EQUAL0_STRING_MAC(packetBuf[i][pktOffset], 0x80,
                "ERSPAN_III <P> & <FT> is not as expected");
        pktOffset +=3;

        /* AUTODOC: ERSPAN<Platform Specific Info> = {6 bits of zero, 20 bit Desc<Copy Reserved>} */
        /* Data = 00000000 00000000 00000000 00000000
                      1112 22222222 23333333
           MASK = 00FF     FFFF     FFFF     0
           1. TC[3]                     - 0
           2. Target Port[10]           - 0x24
           3. Encoded Queue Length[7]   - 0
         */

        erspanIIIPlatSpec = (GT_U32)((packetBuf[i][pktOffset + 2] << 8) |
                               (packetBuf[i][pktOffset + 1] << 16) | ((packetBuf[i][pktOffset] & 0x0F) << 24));
        UTF_VERIFY_EQUAL0_STRING_MAC(erspanIIIPlatSpec, erspanIIIPlatSpecExpected[i],
                                "TC bit in copy reserved is wrong is not as expected");
        getFirst = GT_FALSE;
    }

    /* AUTODOC: Verify LM counter sequence number */
    UTF_VERIFY_EQUAL0_STRING_MAC(lmCounter[0] + 1,
                                 lmCounter[1],
                                 "LM Counter sequence number is not incremented as expected");

    /* AUTODOC: Verify Policer Billing Counters */
    rc = prvTgfPolicerBillingEntryGet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E,
                                      PRV_TGF_BILLING_COUNTERS_FLOW_ID_INDEX_CNS,
                                      GT_FALSE, &prvTgfBillingCntr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfPolicerBillingEntryGet: %d", prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(PRV_TGF_BURST_COUNT_CNS,
                                 prvTgfBillingCntr.greenCntr.l[0],
            "prvTgfPolicerBillingEntryGet: billingCntr_Green mismatch %d", prvTgfDevNum);
}

/**
* @internal tgfOamCurrentHopTelemetryConfigRestore function
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
GT_VOID tgfOamCurrentHopTelemetryConfigRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_IPADDR ipAddr;
    PRV_TGF_POLICER_BILLING_INDEX_CFG_STC  billingIndexCfg;

    /* -------------------------------------------------------------------------
     * 1. Restore TTI Configuration
     */

    /* AUTODOC: disable TTI lookup for ePort#4 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS], PRV_TGF_TTI_KEY_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: disable IPv4 TTI lookup for tunneled packets on ePort#4 */
    rc = prvTgfTtiPortIpv4OnlyTunneledEnableSet(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortIpv4OnlyTunneledEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: invalidate TTI rules */
    rc = prvTgfTtiRuleValidStatusSet(prvTgfTtiRuleIndex, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                    "prvTgfTtiRuleValidStatusSet: %d, %d",
                                    prvTgfDevNum, GT_FALSE);

    /* -------------------------------------------------------------------------
     * 2. Restore Route Configuration
     */

    /* AUTODOC: disable UC IPv4 Routing on ePort#4 */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_INGRESS_PORT_IDX_CNS, CPSS_IP_UNICAST_E,
                                            CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d",
                        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);

    /* AUTODOC: disable IPv4 Unicast Routing on Default Vlan */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_DEFAULT_VLANID_CNS, CPSS_IP_UNICAST_E,
                                    CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                    prvTgfDevNum, PRV_TGF_DEFAULT_VLANID_CNS);

    /* AUTODOC: delete IPv4 prefix */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipAddr.arIP));
    rc = prvTgfIpLpmIpv4UcPrefixDel(prvTgfLpmDBId, prvTgfVrId, ipAddr, 32);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);


    /* -------------------------------------------------------------------------
     * 3. Restore EPCL Configuration
     */
    /* AUTODOC: Restore analyzer interface */
    rc = prvTgfMirrorAnalyzerInterfaceSet(analyzerIndex, &prvTgfRestoreCfg.interface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfMirrorAnalyzerInterfaceSet: %d", analyzerIndex);

    /* invalidate PCL rules */
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
     * 4. Restore PHA Configuration
     */

    /* Clear IOAM mirroring thread from the transit eport#2 */
    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                        GT_FALSE, PRV_TGF_PHA_THREAD_ID_MIRRORING_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaPortThreadIdSet FAILED for PHA_THREAD_ID_EGRESS, rc = [%d]", rc);

    /* Clear tunnel termination thread from the ePort#3 */
    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TRANS_PORT_IDX_CNS],
                                        GT_FALSE, PRV_TGF_PHA_THREAD_ID_EGRESS_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaPortThreadIdSet FAILED for PHA_THREAD_ID_MIRRORING, rc = [%d]", rc);

    /* -------------------------------------------------------------------------
     * 5. Restore Mirrored packet Configuration
     */

    /* AUTODOC: Disable EgressMirroredAsIngressMirrored on analyzer index 1 */
    rc = cpssDxChMirrorTreatEgressMirroredAsIngressMirroredEnableSet(prvTgfDevNum,
                                                           analyzerIndex, GT_FALSE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, cpssDxChMirrorTreatEgressMirroredAsIngressMirroredEnableSet);

    /* AUTODOC: Restore ePort attributes */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                        prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS],
                                        &(prvTgfRestoreCfg.egressInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

    /* Restore common OAM configurations */
    prvTgfDefOamGeneralConfigurationRestore();

    /* AUTODOC: Restore LM Counter status */
    rc = cpssDxChOamLmStampingEnableSet(prvTgfDevNum, prvTgfRestoreCfg.lmStampingEnable);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "cpssDxChOamLmStampingEnableSet");

    /* AUTODOC: Restore Egress policer counter based on Flow Id */
    cpssOsMemSet(&billingIndexCfg, 0, sizeof(billingIndexCfg));
    billingIndexCfg.billingIndexMode       = PRV_TGF_POLICER_BILLING_INDEX_MODE_STANDARD_E;
    billingIndexCfg.billingFlowIdIndexBase = 0;
    billingIndexCfg.billingMinFlowId       = 0;
    billingIndexCfg.billingMaxFlowId       = 0xFFFF;

    rc = prvTgfPolicerFlowIdCountingCfgSet(PRV_TGF_POLICER_STAGE_EGRESS_E, &billingIndexCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPolicerFlowIdCountingCfgSet");

    /* AUTODOC: Disable Metering for "TO_ANALYZER" packets */
    rc = cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableSet(prvTgfDevNum, GT_FALSE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableSet - Disable");

    rc = cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet(prvTgfDevNum,
                                                prvTgfRestoreCfg.cfgTabAccMode);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet");

    /* AUTODOC: Metering and counting status */
    rc = cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableSet(prvTgfDevNum,
                                                prvTgfRestoreCfg.meteringCountingStatus);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableSet - Enable");

    /* -------------------------------------------------------------------------
     * 6. Restore base Configuration
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
* @internal tgfOamCurrentHopTelemetryTest function
* @endinternal
*
* @brief   CurrentHop Telemetry use case test.
*/
GT_VOID tgfOamCurrentHopTelemetryTest
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    PRV_TGF_COS_PROFILE_STC             qosProfile; /* QoS Profile */
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

    packetInfo.numOfParts =  sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);
    packetInfo.partsArray = prvTgfPacketPartArray;
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

    /* AUTODOC: Set TC to 0 for first packet */
    cpssOsMemSet(&qosProfile, 0, sizeof(qosProfile));
    qosProfile.trafficClass   = 0;
    rc = prvTgfCosProfileEntrySet(PRV_TGF_QOS_PROFILE_INDEX_CNS, &qosProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet");

    /* Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfStartTransmitingEth");

    /* AUTODOC: Set TC to 7 for second packet */
    qosProfile.trafficClass   = PRV_TGF_TC_CNS;
    rc = prvTgfCosProfileEntrySet(PRV_TGF_QOS_PROFILE_INDEX_CNS, &qosProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet");

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, 1, 0, NULL);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfSetTxSetupEth");

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

    qosProfile.trafficClass   = 0;
    rc = prvTgfCosProfileEntrySet(PRV_TGF_QOS_PROFILE_INDEX_CNS, &qosProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet");

    /* AUTODOC: Verification Started */
    tgfOamCurrentHopTelemetryVerification();
}
