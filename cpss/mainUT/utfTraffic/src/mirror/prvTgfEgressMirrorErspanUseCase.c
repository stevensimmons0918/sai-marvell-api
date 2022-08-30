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
* @file prvTgfEgressMirrorErspanUseCase.c
*
* @brief Egress ERSPAN use cases testing.
*        Ports allocation:
*           ^ Port#0: Loopback port
*           ^ Port#1: *Analyzer port
*           ^ Port#2: Target port
*           ^ Port#3: Ingress port
*
*        Packet flow:
*           1.  Ethernet frame packet enters the device via Ingress port
*           2.  Packet is sent towards target port in which TX mirroring is executed.
*           3.  One packet which is the origin exits the device, second packet is
*               replicated and sent towards loopback port starting First Pass use case:
*               -   eDSA tag is added by HA unit
*               -   PHA thread of First pass is triggered in which fw adds ERSPAN header
*           4.  Loopback operation is executed
*           5.  Packet is sent towards analyzer port starting Second Pass use case:
*                -  eDSA tag is removed and IP GRE tunnel is added by HA unit
*               -   PHA thread of second pass is triggered in which fw rearranges
*                   origin MAC and ERSPAN headers to valid format
*               -   Mirror packet exits the device from analyzer port
*
*            The above flow is done for 2 use cases of ERSPAN Type II and ERSPAN Type III
* @version   1
********************************************************************************
*/
#include <mirror/prvTgfEgressMirrorErspanUseCase.h>
#include <oam/prvTgfOamGeneral.h>
#include <cpss/dxCh/dxChxGen/pha/cpssDxChPha.h>
#include <cpss/dxCh/dxChxGen/oam/cpssDxChOam.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortLoopback.h>
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
#include <common/tgfCscdGen.h>





/*************************** Constants definitions ***********************************/

/* PHA fw thread IDs of Egress mirroring first & second Pass ERSPAN Type II & Type III */
#define PRV_TGF_PHA_THREAD_ID_EGRESS_FIRST_PASS_ERSPAN_TYPE_II_CNS   24
#define PRV_TGF_PHA_THREAD_ID_EGRESS_FIRST_PASS_ERSPAN_TYPE_III_CNS  25
#define PRV_TGF_PHA_THREAD_ID_EGRESS_SECOND_PASS_ERSPAN_TYPE_II_CNS  26
#define PRV_TGF_PHA_THREAD_ID_EGRESS_SECOND_PASS_ERSPAN_TYPE_III_CNS 27

/* Size of packets in bytes include CRC */
#define PRV_TGF_PACKET_SIZE_ERSPAN_II_CNS   122
#define PRV_TGF_PACKET_SIZE_ERSPAN_III_CNS  134
#define PRV_TGF_MAX_PACKET_SIZE_CNS         PRV_TGF_PACKET_SIZE_ERSPAN_III_CNS

/* Number of packets */
#define PRV_TGF_BURST_COUNT_CNS             2

/* Template profile index */
#define PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS  1

/* Tunnel start entry index */
#define PRV_TGF_TUNNEL_START_INDEX_CNS      8

/* Ports number allocation */
#define PRV_TGF_INGRESS_PORT_IDX_CNS        3
#define PRV_TGF_TARGET_PORT_IDX_CNS         2
#define PRV_TGF_ANALYZER_PORT_IDX_CNS       1
#define PRV_TGF_LOOPBACK_PORT_IDX_CNS       0

/* Default VLAN Id */
#define PRV_TGF_DEFAULT_VLANID_CNS          1

/* Packet flow Id to set by PCL Action  */
#define PRV_TGF_EPCL_FLOW_ID_CNS            25

/* Billing flow Id based counting index base  */
#define PRV_TGF_BILLING_COUNTERS_FLOW_ID_INDEX_BASE_CNS 10

/* Billing flow Id minimal for counting  */
#define PRV_TGF_BILLING_COUNTERS_FLOW_ID_MIN_CNS        20

/* Billing flow Id maximal for counting  */
#define PRV_TGF_BILLING_COUNTERS_FLOW_ID_MAX_CNS        50

/* Billing flow Id based counting index  */
#define PRV_TGF_BILLING_COUNTERS_FLOW_ID_INDEX_CNS   \
    (PRV_TGF_BILLING_COUNTERS_FLOW_ID_INDEX_BASE_CNS \
     + PRV_TGF_EPCL_FLOW_ID_CNS                      \
     - PRV_TGF_BILLING_COUNTERS_FLOW_ID_MIN_CNS)

/* GRE protocol type for ERSPAN Type II */
#define PRV_TGF_GRE_PROTOCOL_TYPE_ERSPAN_II_CNS         0x88BE

/* GRE protocol type for ERSPAN Type III */
#define PRV_TGF_GRE_PROTOCOL_TYPE_ERSPAN_III_CNS        0x22EB

/* ERSPAN II type  */
#define PRV_TGF_ERSPAN_TYPE_II_CNS   2

/* ERSPAN III type  */
#define PRV_TGF_ERSPAN_TYPE_III_CNS  3

/* Offset to IP part in the output packet */
#define PRV_TGF_IP_PKT_OFFSET_CNS  (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS)

/* Offset to GRE part in the output packet */
#define PRV_TGF_GRE_PKT_OFFSET_CNS  (PRV_TGF_IP_PKT_OFFSET_CNS + TGF_IPV4_HEADER_SIZE_CNS)

/* Loopback profiles */
#define PRV_TGF_LB_PROFILE_1_CNS    1

/* egrPacketType for PCL rule */
#define PRV_TGF_EGR_PACKET_TO_CPU_TYPE          0
#define PRV_TGF_EGR_PACKET_FROM_CPU_TYPE        1
#define PRV_TGF_EGR_PACKET_TO_ANALYZER_TYPE     2
#define PRV_TGF_EGR_FORWARD_DATA_PACKET_TYPE    3


/* Cleanup TBD during EnhancedUT Development */
#if 0
/*************************** static DB definitions ***********************************/
/* store value of the mode before test */
static PRV_TGF_POLICER_STAGE_METER_MODE_ENT stageModeSave;

/* DB to hold input packet information */
static TGF_PACKET_STC   packetInfo;

/* Analyzer index */
static GT_U32           analyzerIndex = 1;

/* OAM index */
static GT_U32           oamIndex = 1;

/* DB to hold analyzer port interface */
static CPSS_INTERFACE_INFO_STC  analyzerPortInterface;

/* DB to hold relevant PHA threads information */
static CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC     commonInfo;
static CPSS_DXCH_PHA_THREAD_INFO_UNT            extInfo;
static PRV_TGF_TUNNEL_START_GEN_IP_PROFILE_STC  profileData;
static PRV_TGF_TUNNEL_START_ENTRY_UNT           tunnelEntry;

/* DB to hold actual LM counter values taken from the output packet */
static GT_U32   lmCounter[PRV_TGF_BURST_COUNT_CNS];

/* Mirrored packet, Tunnel part */
/* L2 part */
static TGF_PACKET_L2_STC prvTgfMirroredPacketTunnelL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x44, 0x04},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x45, 0x05}                /* saMac */
};

/* L3 part */
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

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfOriginalPacketL2Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};


/* parameters that are needed to be restored */
static struct
{
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC           interface;
    GT_BOOL                                         prvTgfEnableMirror;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC                egressInfo;
    PRV_TGF_TUNNEL_START_ENTRY_UNT                  tunnelEntry;
    PRV_TGF_OAM_COMMON_CONFIG_STC                   oamConfig;
    CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT  cfgTabAccMode;
    GT_BOOL                                         lmStampingEnable;
    GT_BOOL                                         meteringCountingStatus;
    GT_U32                                          srcLbProfile;
    GT_U32                                          trgLbProfile;
    GT_BOOL                                         lbEnArr[CPSS_TC_RANGE_CNS];
    GT_PHYSICAL_PORT_NUM                            analyzerLbPort;
    GT_BOOL                                         analyzerLbEnable;
    GT_BOOL                                         treatEgressAsIngressEnable;
    CPSS_CSCD_PORT_TYPE_ENT                         cscdPortType;
    GT_BOOL                                         origOamEnStatus;
} prvTgfRestoreCfg;


/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfEgressErspanBridgeConfigSet function
* @endinternal
*
* @brief   Bridge test configurations
*/
static GT_VOID prvTgfEgressErspanBridgeConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN ID 1, target port */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfOriginalPacketL2Part.daMac,
                                          PRV_TGF_DEFAULT_VLANID_CNS,
                                          prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                          GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgDefFdbMacEntryOnPortSet");
}


/**
* @internal prvTgfEgressErspanMirrorConfigSet function
* @endinternal
*
* @brief   Mirroring test configurations
*/
static GT_VOID prvTgfEgressErspanMirrorConfigSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC   interface;

    PRV_UTF_LOG0_MAC("======= Setting Mirroring Configuration =======\n");

    /* AUTODOC: Save analyzer interface from index 1 */
    rc = prvTgfMirrorAnalyzerInterfaceGet(prvTgfDevNum, analyzerIndex, &prvTgfRestoreCfg.interface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceGet: %d", prvTgfDevNum);

    /* AUTODOC: set analyzer interface */
    interface.interface.type = CPSS_INTERFACE_PORT_E;
    interface.interface.devPort.hwDevNum  = prvTgfDevNum;
    interface.interface.devPort.portNum = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS];
    rc = prvTgfMirrorAnalyzerInterfaceSet(analyzerIndex, &interface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceSet: %d", analyzerIndex);

    /* AUTODOC: Enable TX mirroring on Target port, analyzer index=1, isPhysical = false */
    rc = cpssDxChMirrorTxPortSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                 GT_FALSE, GT_TRUE, analyzerIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMirrorTxPortSet");
}


/**
* @internal prvTgfEgressErspanLoopbackConfigSet function
* @endinternal
*
* @brief   Loopback test configurations
*/
static GT_VOID prvTgfEgressErspanLoopbackConfigSet
(
    GT_VOID
)
{
    GT_STATUS               rc = GT_OK;
    CPSS_INTERFACE_INFO_STC portInterface;
    GT_U32                  i;


    PRV_UTF_LOG0_MAC("======= Setting Loopback Configuration =======\n");

    /* AUTODOC: save Ingress port loopback source profile */
    rc = cpssDxChPortLoopbackProfileGet(prvTgfDevNum,
                                        prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                        CPSS_DIRECTION_INGRESS_E,
                                        &prvTgfRestoreCfg.srcLbProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackProfileGet");

    /* AUTODOC: set Ingress port loopback profile {source #1} */
    rc = cpssDxChPortLoopbackProfileSet(prvTgfDevNum,
                                        prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                        CPSS_DIRECTION_INGRESS_E,
                                        PRV_TGF_LB_PROFILE_1_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackProfileSet");

    /* AUTODOC: save analyzer port loopback target profile */
    rc = cpssDxChPortLoopbackProfileGet(prvTgfDevNum,
                                        prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                                        CPSS_DIRECTION_EGRESS_E,
                                        &prvTgfRestoreCfg.trgLbProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackProfileGet");

    /* AUTODOC: set analyzer port loopback profile {target #1} */
    rc = cpssDxChPortLoopbackProfileSet(prvTgfDevNum,
                                        prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                                        CPSS_DIRECTION_EGRESS_E,
                                        PRV_TGF_LB_PROFILE_1_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackProfileSet");

    for (i = 0; i < CPSS_TC_RANGE_CNS; i++)
    {
        /* AUTODOC: save general FTL trigger status */
        rc = cpssDxChPortLoopbackEnableGet(prvTgfDevNum,
                                           PRV_TGF_LB_PROFILE_1_CNS,
                                           PRV_TGF_LB_PROFILE_1_CNS,
                                           i,
                                           &prvTgfRestoreCfg.lbEnArr[i]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackEnableGet, i:%d", i);

        /* AUTODOC: enable FTL for loopback profiles {source #1, target #1} & for all traffic classes */
        rc = cpssDxChPortLoopbackEnableSet(prvTgfDevNum,
                                           PRV_TGF_LB_PROFILE_1_CNS,
                                           PRV_TGF_LB_PROFILE_1_CNS,
                                           i,
                                           GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackEnableSet, i:%d", i);

    }


    /* AUTODOC: save FTL status of analyzer index 1 */
    rc = cpssDxChPortLoopbackPktTypeToAnalyzerGet(prvTgfDevNum,
                                                  analyzerIndex,
                                                  &prvTgfRestoreCfg.analyzerLbPort,
                                                  &prvTgfRestoreCfg.analyzerLbEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackPktTypeToAnalyzerGet");

    /* AUTODOC: Enable forwarding to loopback port for analyzer index 1 */
    rc = cpssDxChPortLoopbackPktTypeToAnalyzerSet(prvTgfDevNum,
                                                  analyzerIndex,
                                                  prvTgfPortsArray[PRV_TGF_LOOPBACK_PORT_IDX_CNS],
                                                  GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackPktTypeToAnalyzerSet,"
                                            " analyzerIndex:%d", analyzerIndex);

    /* AUTODOC: Enable the internal Loopback state in the packet processor MAC port */
    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));
    portInterface.type             = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_LOOPBACK_PORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(&portInterface, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficGeneratorPortLoopbackModeEnableSet,"
                                            " portNum:%d", portInterface.devPort.portNum);

    /* AUTODOC: save status of EgressMirroredAsIngressMirrored */
    rc = cpssDxChMirrorTreatToAnalyzerDsaPktAsIngressMirroredEnableGet(prvTgfDevNum,
                                                                   &prvTgfRestoreCfg.treatEgressAsIngressEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMirrorTreatToAnalyzerDsaPktAsIngressMirroredEnableGet");

    /* AUTODOC: Enable EgressMirroredAsIngressMirrored */
    rc = cpssDxChMirrorTreatToAnalyzerDsaPktAsIngressMirroredEnableSet(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMirrorTreatToAnalyzerDsaPktAsIngressMirroredEnableSet");
}


/**
* @internal prvTgfEgressErspanHaConfigSet function
* @endinternal
*
* @brief   HA test configurations
*/
static GT_VOID prvTgfEgressErspanHaConfigSet
(
    GT_VOID
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC    egressInfo;


    PRV_UTF_LOG0_MAC("======= Setting HA Configuration =======\n");

    /************************ First Pass configurations *********************************/

    /* AUTODOC: Save loopback port cscd type */
    rc = cpssDxChCscdPortTypeGet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_LOOPBACK_PORT_IDX_CNS],
                                 CPSS_PORT_DIRECTION_BOTH_E,
                                 &prvTgfRestoreCfg.cscdPortType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCscdPortTypeGet");

    /* AUTODOC: Set loopback port as cascade to add 16 bytes of eDSA tag */
    rc = cpssDxChCscdPortTypeSet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_LOOPBACK_PORT_IDX_CNS],
                                 CPSS_PORT_DIRECTION_BOTH_E,
                                 CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCscdPortTypeSet");

    /* AUTODOC: Set the thread entry - PRV_TGF_PHA_THREAD_ID_EGRESS_FIRST_PASS_ERSPAN_TYPE_II_CNS */
    cpssOsMemSet(&commonInfo, 0, sizeof(commonInfo));
    extInfo.notNeeded = 0;
    commonInfo.statisticalProcessingFactor = 0;
    commonInfo.busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;
    commonInfo.stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;
    rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
                                     PRV_TGF_PHA_THREAD_ID_EGRESS_FIRST_PASS_ERSPAN_TYPE_II_CNS,
                                     &commonInfo,
                                     CPSS_DXCH_PHA_THREAD_TYPE_EGRESS_MIRRORING_FIRST_PASS_ERSPAN_TYPE_II_E,
                                     &extInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaThreadIdEntrySet");


    /************************ Second Pass configurations *********************************/

    /* AUTODOC: Save ePort attributes configuration */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                                                       prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                                                       &(prvTgfRestoreCfg.egressInfo));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoGet");

    /* AUTODOC: Set analyzer ePort#1 attributes configuration */
    /* AUTODOC: Tunnel Start = ENABLE, tunnelStartPtr = 8, tsPassenger = ETHERNET */
    cpssOsMemSet(&egressInfo,0,sizeof(egressInfo));
    egressInfo = prvTgfRestoreCfg.egressInfo;
    egressInfo.tunnelStart            = GT_TRUE;
    egressInfo.tunnelStartPtr         = PRV_TGF_TUNNEL_START_INDEX_CNS;
    egressInfo.tsPassengerPacketType  = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                                                       &egressInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoSet");

    /* AUTODOC: Set Tunnel Start entry 8 with: */
    /* AUTODOC: tunnelType = Generic IPv4, tagEnable = TRUE, vlanId=1, ipHeaderProtocol = GRE */
    /* AUTODOC: MACDA = 00:00:00:00:36:02 , DIP = 20.1.1.2, SIP=20.1.1.3  */
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));
    tunnelEntry.ipv4Cfg.tagEnable              = GT_TRUE;
    tunnelEntry.ipv4Cfg.vlanId                 = PRV_TGF_DEFAULT_VLANID_CNS;
    tunnelEntry.ipv4Cfg.ipHeaderProtocol       = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;
    tunnelEntry.ipv4Cfg.ethType                = CPSS_TUNNEL_GRE0_ETHER_TYPE_E;
    tunnelEntry.ipv4Cfg.profileIndex           = PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS;
    tunnelEntry.ipv4Cfg.greProtocolForEthernet = PRV_TGF_GRE_PROTOCOL_TYPE_ERSPAN_II_CNS;
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.macDa.arEther, prvTgfMirroredPacketTunnelL2Part.daMac,    sizeof(TGF_MAC_ADDR));
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.destIp.arIP, prvTgfMirroredPacketTunnelIpv4Part.dstAddr, sizeof(TGF_IPV4_ADDR));
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.srcIp.arIP, prvTgfMirroredPacketTunnelIpv4Part.srcAddr, sizeof(TGF_IPV4_ADDR));
    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_INDEX_CNS, CPSS_TUNNEL_GENERIC_IPV4_E, &tunnelEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartEntrySet");

    /* AUTODOC: configure tunnel start profile for ERSPAN Type II (4 Bytes as "Zero") */
    cpssOsMemSet(&profileData,0,sizeof(profileData));
    profileData.templateDataSize = PRV_TGF_TUNNEL_START_TEMPLATE_DATA_SIZE_4B_E; /* 4 bytes */
    rc = prvTgfTunnelStartGenProfileTableEntrySet(prvTgfDevNum,
                                                  PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS, &profileData);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartGenProfileTableEntrySet");

    /* AUTODOC: Assign Egress mirroring Second Pass ERSPAN Type II fw thread to analyzer ePort#1 */
    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                                        GT_TRUE, PRV_TGF_PHA_THREAD_ID_EGRESS_SECOND_PASS_ERSPAN_TYPE_II_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaPortThreadIdSet");

    /* AUTODOC: Set the thread entry - PRV_TGF_PHA_THREAD_ID_EGRESS_SECOND_PASS_ERSPAN_TYPE_II_CNS */
    rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
                                     PRV_TGF_PHA_THREAD_ID_EGRESS_SECOND_PASS_ERSPAN_TYPE_II_CNS,
                                     &commonInfo,
                                     CPSS_DXCH_PHA_THREAD_TYPE_EGRESS_MIRRORING_SECOND_PASS_ERSPAN_TYPE_II_E,
                                     &extInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaThreadIdEntrySet");
}


/**
* @internal prvTgfEgressErspanEpclConfigSet function
* @endinternal
*
* @brief   EPCL test configurations
*/
static GT_VOID prvTgfEgressErspanEpclConfigSet
(
    GT_VOID
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT         mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT         pattern;
    PRV_TGF_PCL_ACTION_STC              action;
    GT_U32                              ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT    ruleFormat;


    PRV_UTF_LOG0_MAC("======= Setting EPCL Configuration =======\n");


    /************************ First Pass configurations *********************************/

    /* AUTODOC: enable EPCL on loopback port */
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_LOOPBACK_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TO_ANALYZER_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: init PCL Engine for Egress PCL on target port */
    rc = prvTgfPclDefPortInit(
            prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_EGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclDefPortInit");

    /* AUTODOC: set PCL rule 0 */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    /* mask & pattern for pclid */
    mask.ruleEgrStdNotIp.common.pclId    = 0x3FF;
    pattern.ruleEgrStdNotIp.common.pclId = (GT_U16)PRV_TGF_PCL_DEFAULT_ID_MAC(
                                                      CPSS_PCL_DIRECTION_EGRESS_E,
                                                      CPSS_PCL_LOOKUP_0_E,
                                                      prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS]);

    /* mask & pattern for egrPacketType */
    mask.ruleEgrStdNotIp.common.egrPacketType = 0x3;
    pattern.ruleEgrStdNotIp.common.egrPacketType = PRV_TGF_EGR_PACKET_TO_ANALYZER_TYPE;

    /* mask & pattern for rxSniff */
    mask.ruleEgrStdNotIp.common.egrPktType.toAnalyzer.rxSniff = 0x1 ;
    pattern.ruleEgrStdNotIp.common.egrPktType.toAnalyzer.rxSniff = GT_FALSE;

    ruleIndex                                    = 0;
    ruleFormat                                   = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
    action.pktCmd                                = CPSS_PACKET_CMD_FORWARD_E;
    action.epclPhaInfo.phaThreadIdAssignmentMode = PRV_TGF_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENABLED_E;
    action.epclPhaInfo.phaThreadId               = PRV_TGF_PHA_THREAD_ID_EGRESS_FIRST_PASS_ERSPAN_TYPE_II_CNS;
    action.epclPhaInfo.phaThreadType             = PRV_TGF_PCL_ACTION_EGRESS_PHA_THREAD_TYPE_NOT_USED_E;
    action.egressPolicy                          = GT_TRUE;

    /* Add the rule to the PCL */
    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclRuleSet, prvTgfDevNum:%d,"
                                            "ruleFormat:%d, ruleIndex:%d", prvTgfDevNum, ruleFormat, ruleIndex);


    /************************ Second Pass configurations *********************************/

    /* AUTODOC: enable EPCL on analyzer port */
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TO_ANALYZER_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: init PCL Engine for Egress PCL for analyzer port */
    rc = prvTgfPclDefPortInit(
            prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_EGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclDefPortInit, prvTgfDevNum:%d", prvTgfDevNum);

    /* AUTODOC: Get EPCL configuration table access mode and save it for restortion */
    rc = cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeGet(prvTgfDevNum,
                                                                &prvTgfRestoreCfg.cfgTabAccMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeGet");

    /* AUTODOC: Configure "Access to the EPCL configuration" table with Port Mode */
    rc = cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet(prvTgfDevNum,
                                CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet");

    /* AUTODOC: set PCL rule 1 */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    /* mask & pattern for pclid */
    mask.ruleEgrStdNotIp.common.pclId    = 0x3FF;
    pattern.ruleEgrStdNotIp.common.pclId = (GT_U16)PRV_TGF_PCL_DEFAULT_ID_MAC(
                                                      CPSS_PCL_DIRECTION_EGRESS_E,
                                                      CPSS_PCL_LOOKUP_0_E,
                                                      prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS]);

    /* mask & pattern for egrPacketType */
    mask.ruleEgrStdNotIp.common.egrPacketType = 0x3;
    pattern.ruleEgrStdNotIp.common.egrPacketType = PRV_TGF_EGR_PACKET_TO_ANALYZER_TYPE;

    /* mask & pattern for rxSniff */
    mask.ruleEgrStdNotIp.common.egrPktType.toAnalyzer.rxSniff = 1 ;
    pattern.ruleEgrStdNotIp.common.egrPktType.toAnalyzer.rxSniff = GT_TRUE;

    ruleIndex                              = 1;
    ruleFormat                             = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;
    action.pktCmd                          = CPSS_PACKET_CMD_FORWARD_E;
    action.oam.oamProcessEnable            = GT_TRUE;
    action.oam.oamProfile                  = PRV_TGF_OAM_PROFILE_INDEX_CNS;
    action.flowId                          = PRV_TGF_EPCL_FLOW_ID_CNS;
    action.policer.policerEnable           = PRV_TGF_PCL_POLICER_ENABLE_COUNTER_ONLY_E;
    action.egressPolicy                    = GT_TRUE;

    /* add the rule to the PCL */
    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclRuleSet, prvTgfDevNum:%d,"
                                            "ruleFormat:%d, ruleIndex:%d", prvTgfDevNum, ruleFormat, ruleIndex);
}


/**
* @internal prvTgfEgressErspanEoamConfigSet function
* @endinternal
*
* @brief   EOAM test configurations
*/
static GT_VOID prvTgfEgressErspanEoamConfigSet
(
    GT_VOID
)
{
    GT_STATUS                       rc = GT_OK;
    PRV_TGF_OAM_COMMON_CONFIG_STC   oamConfig;


    PRV_UTF_LOG0_MAC("======= Setting EOAM Configuration =======\n");

    cpssOsMemSet(&oamConfig, 0, sizeof(oamConfig));

    /* Get OAM common configuration and save it for restortion */
    prvTgfOamConfigurationGet(&oamConfig);
    cpssOsMemCpy(&(prvTgfRestoreCfg.oamConfig), &oamConfig, sizeof(oamConfig));

    /* AUTODOC: set common OAM configurations */
    oamConfig.oamEntry.oamPtpOffsetIndex      = oamIndex;
    oamConfig.oamEntry.opcodeParsingEnable    = GT_FALSE;
    oamConfig.oamEntry.lmCountingMode         = PRV_TGF_OAM_LM_COUNTING_MODE_ENABLE_E;
    oamConfig.oamEntry.lmCounterCaptureEnable = GT_TRUE;
    oamConfig.oamEntryIndex                   = PRV_TGF_EPCL_FLOW_ID_CNS;
    prvTgfOamConfigurationSet(&oamConfig);

    /* AUTODOC: Set OAM LM offset table entry */
    rc = prvTgfOamLmOffsetTableSet(oamIndex, PRV_TGF_OAM_TLV_DATA_OFFSET_STC + 4*oamIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfOamLmOffsetTableSet");

    /* AUTODOC: save OAM Enabler status */
    rc = prvTgfOamEnableGet(PRV_TGF_OAM_STAGE_TYPE_EGRESS_E, &prvTgfRestoreCfg.origOamEnStatus);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfOamEnableGet");

    /* AUTODOC: Enable OAM processing - Egress */
    rc = prvTgfOamEnableSet(PRV_TGF_OAM_STAGE_TYPE_EGRESS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfOamEnableSet");
}


/**
* @internal prvTgfEgressErspanEplrConfigSet function
* @endinternal
*
* @brief   EPLR test configurations
*/
static GT_VOID prvTgfEgressErspanEplrConfigSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_POLICER_BILLING_INDEX_CFG_STC   billingIndexCfg;
    PRV_TGF_POLICER_BILLING_ENTRY_STC       prvTgfBillingCntr;


    PRV_UTF_LOG0_MAC("======= Setting EPLR Configuration =======\n");
    /* Save stage mode */
    rc = prvTgfPolicerStageMeterModeGet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E, &stageModeSave);
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

    /* AUTODOC: flush internal Write Back Cache (WBC) of counting entries */
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
* @internal prvTgfEgressErspanErmrkConfigSet function
* @endinternal
*
* @brief   ERMRK test configurations
*/
static GT_VOID prvTgfEgressErspanErmrkConfigSet
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
* internal prvTgfEgressMirrorErspanConfigSet function
* @endinternal
*
* @brief   Egress Mirroring use case configurations
*/
GT_VOID prvTgfEgressMirrorErspanConfigSet
(
    GT_VOID
)
{
    /* Bridge Configuration */
    prvTgfEgressErspanBridgeConfigSet();

    /* Mirroring Configuration */
    prvTgfEgressErspanMirrorConfigSet();

    /* Loopback Configuration */
    prvTgfEgressErspanLoopbackConfigSet();

    /* HA Configuration */
    prvTgfEgressErspanHaConfigSet();

    /* EPCL Configuration */
    prvTgfEgressErspanEpclConfigSet();

    /* EOAM Configuration */
    prvTgfEgressErspanEoamConfigSet();

    /* EPLR Configuration */
    prvTgfEgressErspanEplrConfigSet();

    /* ERMRK Configuration */
    prvTgfEgressErspanErmrkConfigSet();
}


/**
* @internal prvTgfEgressUpdateCfgToFitErspanTypeIII function
* @endinternal
*
* @brief   Update configurations to support ERSPAN Type III
*/
static GT_VOID prvTgfEgressUpdateCfgToFitErspanTypeIII
(
    GT_VOID
)
{
    GT_STATUS               rc = GT_OK;
    PRV_TGF_PCL_ACTION_STC  action;


    PRV_UTF_LOG0_MAC("======= Update HA Configuration =======\n");

    /************************ First Pass configurations *********************************/

    /* AUTODOC: Set the thread entry - PRV_TGF_PHA_THREAD_ID_EGRESS_FIRST_PASS_ERSPAN_TYPE_III_CNS */
    cpssOsMemSet(&commonInfo, 0, sizeof(commonInfo));
    extInfo.notNeeded = 0;
    commonInfo.statisticalProcessingFactor = 0;
    commonInfo.busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;
    commonInfo.stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;
    rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
                                     PRV_TGF_PHA_THREAD_ID_EGRESS_FIRST_PASS_ERSPAN_TYPE_III_CNS,
                                     &commonInfo,
                                     CPSS_DXCH_PHA_THREAD_TYPE_EGRESS_MIRRORING_FIRST_PASS_ERSPAN_TYPE_III_E,
                                     &extInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaThreadIdEntrySet");


    /************************ Second Pass configurations *********************************/

    /* AUTODOC: Set the thread entry - PRV_TGF_PHA_THREAD_ID_EGRESS_SECOND_PASS_ERSPAN_TYPE_III_CNS */
    extInfo.notNeeded = 0;
    commonInfo.statisticalProcessingFactor = 0;
    commonInfo.busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;
    commonInfo.stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;
    rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
                                     PRV_TGF_PHA_THREAD_ID_EGRESS_SECOND_PASS_ERSPAN_TYPE_III_CNS,
                                     &commonInfo,
                                     CPSS_DXCH_PHA_THREAD_TYPE_EGRESS_MIRRORING_SECOND_PASS_ERSPAN_TYPE_III_E,
                                     &extInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaThreadIdEntrySet");

    /* AUTODOC: Assign Egress mirroring Second Pass ERSPAN Type III fw thread to analyzer ePort#1 */
    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                                        GT_TRUE, PRV_TGF_PHA_THREAD_ID_EGRESS_SECOND_PASS_ERSPAN_TYPE_III_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaPortThreadIdSet");

    /* AUTODOC: Set Tunnel Start entry 8 with: */
    /* AUTODOC: tunnelType = Generic IPv4, tagEnable = TRUE, vlanId=1, ipHeaderProtocol = GRE */
    /* AUTODOC: MACDA = 00:00:00:00:36:02 , DIP = 20.1.1.2, SIP=20.1.1.3  */
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));
    tunnelEntry.ipv4Cfg.tagEnable              = GT_TRUE;
    tunnelEntry.ipv4Cfg.vlanId                 = PRV_TGF_DEFAULT_VLANID_CNS;
    tunnelEntry.ipv4Cfg.ipHeaderProtocol       = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;
    tunnelEntry.ipv4Cfg.ethType                = CPSS_TUNNEL_GRE0_ETHER_TYPE_E;
    tunnelEntry.ipv4Cfg.profileIndex           = PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS;
    tunnelEntry.ipv4Cfg.greProtocolForEthernet = PRV_TGF_GRE_PROTOCOL_TYPE_ERSPAN_III_CNS;
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.macDa.arEther, prvTgfMirroredPacketTunnelL2Part.daMac,    sizeof(TGF_MAC_ADDR));
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.destIp.arIP, prvTgfMirroredPacketTunnelIpv4Part.dstAddr, sizeof(TGF_IPV4_ADDR));
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.srcIp.arIP, prvTgfMirroredPacketTunnelIpv4Part.srcAddr, sizeof(TGF_IPV4_ADDR));
    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_INDEX_CNS, CPSS_TUNNEL_GENERIC_IPV4_E, &tunnelEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartEntrySet");


    PRV_UTF_LOG0_MAC("======= Updating PCL Actions ========\n");

    /* Update PCL Action 0: update to PRV_TGF_PHA_THREAD_ID_EGRESS_FIRST_PASS_ERSPAN_TYPE_III_CNS */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd                          = CPSS_PACKET_CMD_FORWARD_E;
    action.epclPhaInfo.phaThreadIdAssignmentMode = PRV_TGF_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENABLED_E;
    action.epclPhaInfo.phaThreadId         = PRV_TGF_PHA_THREAD_ID_EGRESS_FIRST_PASS_ERSPAN_TYPE_III_CNS;
    action.epclPhaInfo.phaThreadType       = PRV_TGF_PCL_ACTION_EGRESS_PHA_THREAD_ERSPAN_TYPE_III_MIRROR_E;
    action.epclPhaInfo.phaThreadUnion.epclErspanTypeIIIMirror.erspanIII_P = GT_TRUE;
    action.epclPhaInfo.phaThreadUnion.epclErspanTypeIIIMirror.erspanIII_FT = 0;
    action.egressPolicy = GT_TRUE;

    /* AUTODOC: update PCL Action for rule#0: update PHA info with ERSPAN Type III thread */
    rc = prvTgfPclRuleActionUpdate(CPSS_PCL_RULE_SIZE_STD_E, 0, &action);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclRuleActionUpdate");
}


/**
* @internal prvTgfEgressMirrorErspanConfigRestore function
* @endinternal
*
* @brief   Egress Mirroring ERSPAN configurations restore.
* @note    1. Restore Bridge Configuration
*          2. Restore Mirroring Configuration
*          3. Restore Loopback configuration
*          4. Restore HA Configuration
*          5. Restore EPCL Configuration
*          6. Restore EOAM Configuration
*          7. Restore EPLR Configuration
*          8. Restore ERMRK Configuration
*          9. Restore base Configuration
*/
GT_VOID prvTgfEgressMirrorErspanConfigRestore
(
    GT_VOID
)
{
    GT_STATUS               rc = GT_OK;
    GT_U32                  i;
    CPSS_INTERFACE_INFO_STC portInterface;


    PRV_UTF_LOG0_MAC("======= Restore configurations =======\n");

    /* -------------------------------------------------------------------------
     * 1. Restore Bridge Configuration
     */
    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgFdbFlush");


    /* -------------------------------------------------------------------------
     * 2. Restore Mirrored packet Configuration
     */
    /* AUTODOC: restore mirror interface configuration */
    rc = prvTgfMirrorAnalyzerInterfaceSet(analyzerIndex, &prvTgfRestoreCfg.interface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfMirrorAnalyzerInterfaceSet, analyzerIndex:%d", analyzerIndex);

    /* AUTODOC: disable TX mirroring */
    rc = cpssDxChMirrorTxPortSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                 GT_FALSE, GT_FALSE, analyzerIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMirrorTxPortSet, analyzerIndex:%d", analyzerIndex);


    /* -------------------------------------------------------------------------
     * 3. Restore loopback packet Configuration
     */
    /* AUTODOC: restore Ingress port loopback source profile */
    rc = cpssDxChPortLoopbackProfileSet(prvTgfDevNum,
                                        prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                        CPSS_DIRECTION_INGRESS_E,
                                        prvTgfRestoreCfg.srcLbProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackProfileSet");

    /* AUTODOC: restore analyzer port loopback target profile */
    rc = cpssDxChPortLoopbackProfileSet(prvTgfDevNum,
                                        prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                                        CPSS_DIRECTION_EGRESS_E,
                                        prvTgfRestoreCfg.trgLbProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackProfileSet");


    for (i = 0; i < CPSS_TC_RANGE_CNS; i++)
    {
        /* AUTODOC: restore general FTL trigger status */
        rc = cpssDxChPortLoopbackEnableSet(prvTgfDevNum,
                                           PRV_TGF_LB_PROFILE_1_CNS,
                                           PRV_TGF_LB_PROFILE_1_CNS,
                                           i,
                                           prvTgfRestoreCfg.lbEnArr[i]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackEnableSet, i:%d", i);
    }

    /* AUTODOC: restore FTL status of analyzer index 1 */
    rc = cpssDxChPortLoopbackPktTypeToAnalyzerSet(prvTgfDevNum,
                                                  analyzerIndex,
                                                  prvTgfRestoreCfg.analyzerLbPort,
                                                  prvTgfRestoreCfg.analyzerLbEnable);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, analyzerIndex, prvTgfRestoreCfg.analyzerLbPort);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackPktTypeToAnalyzerSet,"
                                            "analyzerLbPort:%d  analyzerLbEn:%d",
                                  prvTgfRestoreCfg.analyzerLbPort,prvTgfRestoreCfg.analyzerLbEnable);

    /* AUTODOC: disable the internal Loopback state in the packet processor MAC port */
    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));
    portInterface.type             = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_LOOPBACK_PORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(&portInterface, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficGeneratorPortLoopbackModeEnableSet, portNum:%d", portInterface.devPort.portNum);

    /* AUTODOC: save status of EgressMirroredAsIngressMirrored */
    rc = cpssDxChMirrorTreatToAnalyzerDsaPktAsIngressMirroredEnableSet(prvTgfDevNum,
                                                                   prvTgfRestoreCfg.treatEgressAsIngressEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMirrorTreatToAnalyzerDsaPktAsIngressMirroredEnableSet");


    /* -------------------------------------------------------------------------
     * 4. Restore HA Configuration
     */
    /* AUTODOC: restore loopback port cascade type */
    rc = cpssDxChCscdPortTypeSet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_LOOPBACK_PORT_IDX_CNS],
                                 CPSS_PORT_DIRECTION_BOTH_E,
                                 prvTgfRestoreCfg.cscdPortType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCscdPortTypeSet");

    /* AUTODOC: restore ePort attributes */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                                                       &(prvTgfRestoreCfg.egressInfo));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoSet");

    /* AUTODOC: clear PHA fw thread from the analyzer port */
    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum,
                                    prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                                    GT_FALSE,
                                    0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaPortThreadIdSet");


    /* -------------------------------------------------------------------------
     * 5. Restore EPCL Configuration
     */
    /* AUTODOC: invalidate PCL rule 0 */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, 0, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclRuleValidStatusSet, index = %d", 0);

    /* AUTODOC: invalidate PCL rule 1 */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, 1, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclRuleValidStatusSet, index = %d", 1);

    /* AUTODOC: restore Access mode */
    rc = cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet(prvTgfDevNum, prvTgfRestoreCfg.cfgTabAccMode);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "Failure in cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet");

    /* AUTODOC: disable all PCL configurations on the ports */
    prvTgfPclPortsRestoreAll();


    /* -------------------------------------------------------------------------
     * 6. Restore EOAM Configuration
     */
    /* AUTODOC: restore common EOAM configurations */
    prvTgfOamConfigurationSet(&prvTgfRestoreCfg.oamConfig);

    /* AUTODOC: restore OAM Enabler status */
    rc = prvTgfOamEnableSet(PRV_TGF_OAM_STAGE_TYPE_INGRESS_E, prvTgfRestoreCfg.origOamEnStatus);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfOamEnableSet");


    /* -------------------------------------------------------------------------
     * 7. Restore EPLR Configuration
     */
    /* AUTODOC: Restore Policer Egress to Analyzer metering and counting status */
    rc = cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableSet(prvTgfDevNum,
                                                prvTgfRestoreCfg.meteringCountingStatus);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "Failure in cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableSet");

    /* AUTODOC: Restore stage mode */
    rc = prvTgfPolicerStageMeterModeSet(PRV_TGF_POLICER_STAGE_EGRESS_E, stageModeSave);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeSet: %d",
                                 stageModeSave);

    /* -------------------------------------------------------------------------
     * 8. Restore ERMRK Configuration
     */
    /* AUTODOC: Restore LM Counter status */
    rc = cpssDxChOamLmStampingEnableSet(prvTgfDevNum, prvTgfRestoreCfg.lmStampingEnable);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "Failure in cpssDxChOamLmStampingEnableSet");


    /* -------------------------------------------------------------------------
     * 9. Restore base Configuration
     */
    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "Failure in tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Disable RX capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "Failure in tgfTrafficTableRxStartCapture");

    /* AUTODOC: reset hw counters */
    rc = prvTgfCommonAllBridgeCntrsReset();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfCommonAllCntrsReset");
}




/**
* internal tgfEgressMirrorErspanTypeIIVerification function
* @endinternal
*
* @brief  Egress Mirroring ERSPAN Type II use case verification
*         Do the following:
*         - Check GRE protocol = 0x88BE (ERSPAN Type II)
*         - Save GRE Sequence number. Will be verified in next packet.
*         - Check ERSPAN high part = 0x100000 (ERSPN<Ver>=1)
*         - Check ERSPAN low part = 0x0
*         - Check MAC addresses of passenger are not damaged
*         - Check payload data of passenger is not damaged
*/
GT_VOID tgfEgressMirrorErspanTypeIIVerification
(
    IN  GT_U8  * packetBuf
)
{
    GT_U32              pktOffset, payloadPktOffset;
    GT_U32              greProtocolType, erspanHighPart, erspanLowPart, erspanExpectedVal;
    GT_U32              i;
    GT_U32              daMacErr=0, saMacErr=0, passengerPayloadErr=0;
    GT_U32              recPassengerPayload, expPassengerPayload;
    TGF_PACKET_L2_STC   receivedMac;


    PRV_UTF_LOG0_MAC("======= Packet verification of ERSPAN Type II =======\n");

    /* Update offset to point to GRE header */
    pktOffset = PRV_TGF_GRE_PKT_OFFSET_CNS;

    /*********************** Check GRE protocol *********************************************/
    /* AUTODOC: get the first 4B of GRE header to verify GRE protocol */
    greProtocolType = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                           (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));

    /* AUTODOC: set expected value with GRE protocol type of ERSPAN II 0x88BE as defined in standart */
    erspanExpectedVal = PRV_TGF_GRE_PROTOCOL_TYPE_ERSPAN_II_CNS;

    /* AUTODOC: compare received GRE protocol type vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, greProtocolType,
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "GRE protocol type is not as expected => Expected:0x%x != Received:0x%x \n",
                                 erspanExpectedVal, greProtocolType);


    /*********************** Get GRE Seq. number *********************************************/
    /* Increment offset by 4B to point to GRE header (low part)  */
    pktOffset +=4;

    /* AUTODOC: get next 4B of GRE header to check GRE Seq.number. Will be verified in next packet */
    lmCounter[0] = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                        (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));


    /*********************** Check ERSPAN header high part ************************************/
    /* Increment offset by 4B to point to ERSPAN header (high part)  */
    pktOffset +=4;

    /* AUTODOC: get first 4B of ERSPAN header to verify ERSPAN Version and rest of bits are zero */
    erspanHighPart = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                          (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));

    /* AUTODOC: set expected value. All bits should be zero except for ERSPAN<Ver> = 1 */
    erspanExpectedVal = 0x10000000;

    /* AUTODOC: compare received ERSPAN header high part vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, erspanHighPart,
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "ERSPAN high part is not as expected => Expected:0x%x != Received:0x%x \n",
                                 erspanExpectedVal, erspanHighPart);


    /*********************** Check ERSPAN header low part ************************************/
    /* Increment offset by 4B to point to ERSPAN header (low part)  */
    pktOffset +=4;

    /* AUTODOC: get the next 4B of ERSPAN header to verify all bits are zero */
    erspanLowPart = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                           (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));

    /* AUTODOC: set expected value. All bits should be zero */
    erspanExpectedVal = 0x00000000;

    /* AUTODOC: compare received ERSPAN header low part vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, erspanLowPart,
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "ERSPAN low part is not as expected => Expected:0x%x != Received:0x%x \n",
                                 erspanExpectedVal, erspanLowPart);


    /*********************** Check Origin MAC DA ************************************/
    /* Increment offset by 4B to point to MAC DA  */
    pktOffset +=4;

    /* AUTODOC: Get actual MAC DA & MAC SA */
    for (i=0;i<(TGF_L2_HEADER_SIZE_CNS/2);i++)
    {
        receivedMac.daMac[i] = packetBuf[pktOffset + i];
        receivedMac.saMac[i] = packetBuf[pktOffset + i + sizeof(prvTgfOriginalPacketL2Part.daMac)];

        /* Decrment received with expected to check if there is any mismatch */
        daMacErr += receivedMac.daMac[i] - prvTgfOriginalPacketL2Part.daMac[i] ;
        saMacErr += receivedMac.saMac[i] - prvTgfOriginalPacketL2Part.saMac[i] ;
    }

    /* AUTODOC: compare received MAC DA vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL0_STRING_MAC(0, daMacErr,
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "Mismatch in MAC DA address");
    if (daMacErr != 0)
    {
        /* Print received and expected MAC DA */
        PRV_UTF_LOG12_MAC("Expected MAC DA:%.2x%.2x%.2x%.2x%.2x%.2x \n"
                          "Received MAC DA:%.2x%.2x%.2x%.2x%.2x%.2x \n",
                          prvTgfOriginalPacketL2Part.daMac[0],prvTgfOriginalPacketL2Part.daMac[1],prvTgfOriginalPacketL2Part.daMac[2],
                          prvTgfOriginalPacketL2Part.daMac[3],prvTgfOriginalPacketL2Part.daMac[4],prvTgfOriginalPacketL2Part.daMac[5],
                          receivedMac.daMac[0],receivedMac.daMac[1],receivedMac.daMac[2],
                          receivedMac.daMac[3],receivedMac.daMac[4],receivedMac.daMac[5]);
    }


    /*********************** Check Origin MAC SA ************************************/
    /* AUTODOC: compare received MAC SA vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL0_STRING_MAC(0, saMacErr,
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "Mismatch in MAC SA address \n");
    if (saMacErr != 0)
    {
        /* Print received and expected MAC SA */
        PRV_UTF_LOG12_MAC("Expected MAC SA:%.2x%.2x%.2x%.2x%.2x%.2x \n"
                          "Received MAC SA:%.2x%.2x%.2x%.2x%.2x%.2x \n",
                          prvTgfOriginalPacketL2Part.saMac[0],prvTgfOriginalPacketL2Part.saMac[1],prvTgfOriginalPacketL2Part.saMac[2],
                          prvTgfOriginalPacketL2Part.saMac[3],prvTgfOriginalPacketL2Part.saMac[4],prvTgfOriginalPacketL2Part.saMac[5],
                          receivedMac.saMac[0],receivedMac.saMac[1],receivedMac.saMac[2],
                          receivedMac.saMac[3],receivedMac.saMac[4],receivedMac.saMac[5]);
    }


    /*********************** Check Passenger payload ************************************/
    /* Increment offset by 12B of MACs to point to passenger payload (EtherType)  */
    pktOffset +=TGF_L2_HEADER_SIZE_CNS;

    /* Anchor offset to passenger */
    payloadPktOffset = pktOffset;

    /* AUTODOC: Get actual passenger payload  */
    for (i=0;i<prvTgfPacketPayloadPart.dataLength;i+=4)
    {
        /* Update offset every 4 bytes */
        pktOffset = payloadPktOffset + i;

        /* Get actual payload */
        recPassengerPayload = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                                   (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));
        /* Get expected payload */
        expPassengerPayload = (GT_U32)(prvTgfPayloadDataArr[i+3] | (prvTgfPayloadDataArr[i+2] << 8) |
                                           (prvTgfPayloadDataArr[i+1] << 16) | (prvTgfPayloadDataArr[i] << 24));

        /* Decrement received with expected to check if there is any mismatch */
        passengerPayloadErr += (recPassengerPayload - expPassengerPayload) ;
    }

    /* AUTODOC: compare received passenger payload vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL0_STRING_MAC(0, passengerPayloadErr,
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "Mismatch in passenger payload data ");
}



/**
* internal tgfEgressMirrorErspanTypeIIIVerification function
* @endinternal
*
* @brief Egress Mirroring ERSPAN Type III use case verification
*         Do the following:
*         - Check GRE protocol = 0x22EB (ERSPAN Type III)
*         - Check GRE Sequence number is indeed incremented
*         - Check ERSPAN high part = 0x200000 (ERSPN<Ver>=2)
*         - Check ERSPAN low part = 0x800d (ERSPAN<P>=PHA Metadata[31]=1,
*                                           ERSPAN<FT>=PHA Metadata[30:26]=0,
*                                           ERSPAN<D>=1,
*                                           ERSPAN<Gra>=2,
*                                           ERSPAN<O>=1)
*                                           rest bits should be cleared to zero)
*         - Check MAC addresses of passenger are not damaged
*         - Check payload data of passenger is not damaged
*/
GT_VOID tgfEgressMirrorErspanTypeIIIVerification
(
    IN  GT_U8   * packetBuf
)
{
    GT_U32              pktOffset, payloadPktOffset;
    GT_U32              greProtocolType, erspanHighPart, erspanLowPart, erspanExpectedVal;
    GT_U32              i;
    GT_U32              daMacErr=0, saMacErr=0, passengerPayloadErr=0;
    GT_U32              recPassengerPayload, expPassengerPayload;
    TGF_PACKET_L2_STC   receivedMac;


    PRV_UTF_LOG0_MAC("======= Packet verification of ERSPAN Type III =======\n");

    /* Update offset to point to GRE header */
    pktOffset = PRV_TGF_GRE_PKT_OFFSET_CNS;


    /*********************** Check GRE protocol *********************************************/
    /* AUTODOC: get the first 4B of GRE header to verify GRE protocol */
    greProtocolType = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                           (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));

    /* AUTODOC: set expected value with GRE protocol type of ERSPAN II 0x22EB as defined in standart */
    erspanExpectedVal = PRV_TGF_GRE_PROTOCOL_TYPE_ERSPAN_III_CNS;

    /* AUTODOC: compare received GRE protocol type vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, greProtocolType,
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "GRE protocol type is not as expected => Expected:0x%x != Received:0x%x \n",
                                 erspanExpectedVal, greProtocolType);


    /*********************** Get GRE Seq. number *********************************************/
    /* Increment offset by 4B to point to GRE header (low part)  */
    pktOffset +=4;

    /* AUTODOC: get the next 4B of GRE header to verify GRE Seq.number */
    lmCounter[1] = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                        (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));

    /* AUTODOC: Verify LM counter sequence number is indeed incremented */
    UTF_VERIFY_EQUAL0_STRING_MAC(lmCounter[0] + 1, lmCounter[1],
                                 "LM Counter sequence number is not incremented as expected");


    /*********************** Check ERSPAN header high part ************************************/
    /* Increment offset by 4B to point to ERSPAN header (high part)  */
    pktOffset +=4;

    /* AUTODOC: get first 4B of ERSPAN header to verify ERSPAN Version */
    erspanHighPart = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                          (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));

    /* AUTODOC: set expected value. All bits should be zero except for ERSPAN<Ver> = 2 */
    erspanExpectedVal = 0x20000000;

    /* AUTODOC: compare received ERSPAN header high part vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, erspanHighPart,
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "ERSPAN high part is not as expected => Expected:0x%x != Received:0x%x \n",
                                 erspanExpectedVal, erspanHighPart);


    /*********************** Check ERSPAN header low part ************************************/
    /* Increment offset by 8B to point to ERSPAN header (low part)  */
    pktOffset +=8;

    /* AUTODOC: get the next 4B of ERSPAN header */
    erspanLowPart = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                           (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));

    /* AUTODOC: Set expected word as below:
       ERSPAN<P> = PHA Metadata[31] = 1, ERSPAN<FT> = PHA Metadata[30:26] = 0
       ERSPAN<D> = 1 , ERSPAN<Gra> = 2 , ERSPAN<O> = 1
       Rest of the bits should be zero */
    erspanExpectedVal = 0x800d;

    /* AUTODOC: compare received ERSPAN header low part vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, erspanLowPart,
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "ERSPAN low part is not as expected => Expected:0x%x != Received:0x%x \n",
                                 erspanExpectedVal, erspanLowPart);


    /*********************** Check ERSPAN Platform specific subheader **************/
    /* Increment offset by 4 bytes  */
    pktOffset +=4;

    /* AUTODOC: get the first 4B of ERSPAN platform specific subheader */
    erspanHighPart = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                          (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));

    /* AUTODOC: Set expected word as below:
       ERSPAN subheader<Platf ID> = 0x3
       ERSPAN subheader<Platform Specific Info> clear to zero */
    erspanExpectedVal = 0x0c000000;

    /* AUTODOC: compare received ERSPAN suheader high part vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, erspanHighPart,
                                "--- Failure in tgfEgressMirrorErspanTypeIIIVerification --- \n"
                                "ERSPAN subheader high part is not as expected => Expected:0x%x != Received:0x%x \n",
                                 erspanExpectedVal, erspanLowPart);


    /*********************** Check Origin MAC DA ************************************/
    /* Increment offset by 8B to point to MAC DA  */
    pktOffset +=8;

    /* AUTODOC: Get actual MAC DA & MAC SA */
    for (i=0;i<(TGF_L2_HEADER_SIZE_CNS/2);i++)
    {
        receivedMac.daMac[i] = packetBuf[pktOffset + i];
        receivedMac.saMac[i] = packetBuf[pktOffset + i + sizeof(prvTgfOriginalPacketL2Part.daMac)];

        /* Decrment received with expected to check if there is any mismatch */
        daMacErr += receivedMac.daMac[i] - prvTgfOriginalPacketL2Part.daMac[i] ;
        saMacErr += receivedMac.saMac[i] - prvTgfOriginalPacketL2Part.saMac[i] ;
    }

    /* AUTODOC: compare received MAC DA vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL0_STRING_MAC(0, daMacErr,
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "Mismatch in MAC DA address");
    if (daMacErr != 0)
    {
        /* Print received and expected MAC DA */
        PRV_UTF_LOG12_MAC("Expected MAC DA:%.2x%.2x%.2x%.2x%.2x%.2x \n"
                          "Received MAC DA:%.2x%.2x%.2x%.2x%.2x%.2x \n",
                          prvTgfOriginalPacketL2Part.daMac[0],prvTgfOriginalPacketL2Part.daMac[1],prvTgfOriginalPacketL2Part.daMac[2],
                          prvTgfOriginalPacketL2Part.daMac[3],prvTgfOriginalPacketL2Part.daMac[4],prvTgfOriginalPacketL2Part.daMac[5],
                          receivedMac.daMac[0],receivedMac.daMac[1],receivedMac.daMac[2],
                          receivedMac.daMac[3],receivedMac.daMac[4],receivedMac.daMac[5]);
    }


    /*********************** Check Origin MAC SA ************************************/
    /* AUTODOC: compare received MAC SA vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL0_STRING_MAC(0, saMacErr,
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "Mismatch in MAC SA address \n");
    if (saMacErr != 0)
    {
        /* Print received and expected MAC SA */
        PRV_UTF_LOG12_MAC("Expected MAC SA:%.2x%.2x%.2x%.2x%.2x%.2x \n"
                          "Received MAC SA:%.2x%.2x%.2x%.2x%.2x%.2x \n",
                          prvTgfOriginalPacketL2Part.saMac[0],prvTgfOriginalPacketL2Part.saMac[1],prvTgfOriginalPacketL2Part.saMac[2],
                          prvTgfOriginalPacketL2Part.saMac[3],prvTgfOriginalPacketL2Part.saMac[4],prvTgfOriginalPacketL2Part.saMac[5],
                          receivedMac.saMac[0],receivedMac.saMac[1],receivedMac.saMac[2],
                          receivedMac.saMac[3],receivedMac.saMac[4],receivedMac.saMac[5]);
    }


    /*********************** Check Passenger payload ************************************/
    /* Increment offset by 12B of MACs to point to passenger payload (EtherType)  */
    pktOffset +=TGF_L2_HEADER_SIZE_CNS;

    /* Anchor offset to passenger */
    payloadPktOffset = pktOffset;

    /* AUTODOC: Get actual passenger payload  */
    for (i=0;i<prvTgfPacketPayloadPart.dataLength;i+=4)
    {
        /* Update offset every 4 bytes */
        pktOffset = payloadPktOffset + i;

        /* Get actual payload */
        recPassengerPayload = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                                   (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));
        /* Get expected payload */
        expPassengerPayload = (GT_U32)(prvTgfPayloadDataArr[i+3] | (prvTgfPayloadDataArr[i+2] << 8) |
                                           (prvTgfPayloadDataArr[i+1] << 16) | (prvTgfPayloadDataArr[i] << 24));

        /* Decrement received with expected to check if there is any mismatch */
        passengerPayloadErr += (recPassengerPayload - expPassengerPayload) ;
    }

    /* AUTODOC: compare received passenger payload vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL0_STRING_MAC(0, passengerPayloadErr,
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "Mismatch in passenger payload data ");
}



/**
* internal prvTgfEgressMirrorErspanVerification function
* @endinternal
*
* @brief  Egress Mirroring ERSPAN use case verification
*         Do the following:
*         - Loop over 2 iterations:
*              ^ First one check packet of ERSPAN Type II
*              ^ Second one check packet of ERSPAN Type III
*         - Check Policer billing counters are set to 2 packets
*/
GT_VOID prvTgfEgressMirrorErspanVerification
(
    GT_VOID
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_POLICER_BILLING_ENTRY_STC   prvTgfBillingCntr;
    GT_U8                               packetBuf[PRV_TGF_BURST_COUNT_CNS][PRV_TGF_MAX_PACKET_SIZE_CNS] = {{0}, {0}};
    GT_U32                              packetLen[PRV_TGF_BURST_COUNT_CNS] = {PRV_TGF_PACKET_SIZE_ERSPAN_II_CNS,
                                                                              PRV_TGF_PACKET_SIZE_ERSPAN_III_CNS};
    GT_U32                              packetActualLength = 0;
    GT_U8                               queue = 0;
    GT_U8                               dev = 0;
    TGF_NET_DSA_STC                     rxParam;
    GT_U8                               i = 0;
    GT_BOOL                             getFirst = GT_TRUE;
    GT_U8                               erspanType = PRV_TGF_ERSPAN_TYPE_II_CNS;


    /* Loop over 2 packets while first packet is with ERSPAN Type II and second with ERSPAN Type III */
    for (i = 0; i<PRV_TGF_BURST_COUNT_CNS; i++)
    {
        /* AUTODOC: Get the captured packets */
        rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&analyzerPortInterface,
                                                    TGF_PACKET_TYPE_CAPTURE_E,
                                                    getFirst, GT_TRUE, packetBuf[i],
                                                    &packetLen[i], &packetActualLength,
                                                    &dev, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet");

        /* Call the right verification function according to ERSPAN Type */
        if (erspanType == PRV_TGF_ERSPAN_TYPE_II_CNS)
        {
            /* ERSPAN Type II */
            tgfEgressMirrorErspanTypeIIVerification(packetBuf[i]);
        }
        else
        {
            /* ERSPAN Type III */
            tgfEgressMirrorErspanTypeIIIVerification(packetBuf[i]);
        }

        /* Update it to ERSPAN III for next iteration */
        erspanType = PRV_TGF_ERSPAN_TYPE_III_CNS;

        /* Set to false for next iteration in order to get the next packet from next Rx CPU entry */
        getFirst = GT_FALSE;
    }

    /* AUTODOC: Get Policer Billing Counters */
    rc = prvTgfPolicerBillingEntryGet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E,
                                      PRV_TGF_BILLING_COUNTERS_FLOW_ID_INDEX_CNS,
                                      GT_FALSE, &prvTgfBillingCntr);

    /* AUTODOC: Verify Policer Billing Counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPolicerBillingEntryGet");
    UTF_VERIFY_EQUAL2_STRING_MAC(PRV_TGF_BURST_COUNT_CNS, prvTgfBillingCntr.greenCntr.l[0],
                                 "billingCntr_Green mismatch => Expected:%d != Received:%d",
                                 PRV_TGF_BURST_COUNT_CNS, prvTgfBillingCntr.greenCntr.l[0]);
}



/**
* @internal prvTgfEgressMirrorErspanTest function
* @endinternal
*
* @brief   Egress Mirroring ERSPAN use case test.
*          Use the Loopback configurations to perform First and second pass use cases
*          in which Egress mirroring packet is performed over IP GRE plus ERSPAN header
*          tunnel. During this process 2 PHA fw threads are triggered one for First pass
*          in which ERSPAN header is added and the second is for Second Pass in which
*          origin MAC addresses and ERSPAN header are reordered to fit valid packet format.
*          Send 2 packets: first one with ERSPAN Type II second one with ERSPAN Type III
*          At the end packets are captured and content is checked to verify right behaviour.
*/
GT_VOID prvTgfEgressMirrorErspanTest
(
    GT_VOID
)
{
    GT_STATUS                       rc = GT_OK;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    GT_U32                          portIter;

    PRV_UTF_LOG0_MAC("======= Ports allocation =======\n");
    PRV_UTF_LOG1_MAC("Port [%d]: Ingress Port \n", prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    PRV_UTF_LOG1_MAC("Port [%d]: Target Port \n", prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS]);
    PRV_UTF_LOG1_MAC("Port [%d]: Analyzer Port \n", prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS]);
    PRV_UTF_LOG1_MAC("Port [%d]: LoopBack Port \n", prvTgfPortsArray[PRV_TGF_LOOPBACK_PORT_IDX_CNS]);

    /* AUTODOC: reset counters for all 4 ports */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "Failure in prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficTableRxPcktTblClear");

    /* Update analyzer portInterface for capturing */
    analyzerPortInterface.type              = CPSS_INTERFACE_PORT_E;
    analyzerPortInterface.devPort.hwDevNum  = prvTgfDevNum;
    analyzerPortInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS];

    /* AUTODOC: enable capture on analyzer port ePort#1 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
                        &analyzerPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in tgftrafficgeneratorporttxethcaptureset port - %d",
                                        analyzerPortInterface.devPort.portNum);

    /* Update input packet interface structure  */
    packetInfo.numOfParts =  sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);
    packetInfo.partsArray = prvTgfPacketPartArray;
    packetInfo.totalLen   = TGF_PACKET_AUTO_CALC_LENGTH_CNS;

    /* AUTODOC: set transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, 1, 0, NULL);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "Failure in prvTgfSetTxSetupEth");


    PRV_UTF_LOG0_MAC("======= Send first packet =======\n");
    /* AUTODOC: send first packet  */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "Failure in prvTgfStartTransmitingEth");


    PRV_UTF_LOG0_MAC("======= Update configurations to support ERSPAN Type III =======\n");
    /* Update configurations to support ERSPAN Type III  */
    prvTgfEgressUpdateCfgToFitErspanTypeIII();


    PRV_UTF_LOG0_MAC("======= Send second packet =======\n");
    /* AUTODOC: Send second packet  */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "Failure in prvTgfStartTransmitingEth");

    /* Disable capture on analyzer port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &analyzerPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficGeneratorPortTxEthCaptureSet port - %d",
            analyzerPortInterface.devPort.portNum);


    PRV_UTF_LOG0_MAC("======= Print Captured Packet and counters on Analyzer port =======\n");
    /* Print captured packets from analyzer port */
    PRV_UTF_LOG1_MAC("Port [%d] Mirrored capturing:\n", prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS]);
    rc = prvTgfPortCapturedPacketPrint(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS]);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "Failure in prvTgfPortCapturedPacketPrint");

    /* Read counters */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                                   GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "Failure in prvTgfReadPortCountersEth: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS]);
}
#endif
