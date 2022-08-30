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
* @file prvTgfClassifierNshOverVxlanGpeUseCase.c
*
* @brief Classifier NSH over VXLAN-GPE use cases testing.
*
*        Ports allocation:
*           ^ Port#0: Ingress port
*           ^ Port#1: Target port
*
*        Packet flow:
*           1.  Ethernet frame packet enters the device via Ingress Port#0
*           2.  The packet is sent towards target port#1 and undergoes the following actions:
*               -   L2, IPv6, UDP, VXLAN-GPE tunnel + NSH headers are added by HA unit
*               -   PHA thread is triggered in which fw adds 16B for NSH metadata plus
*                   updates some of the fields using specific PHA template and PHA metadata
*           3.  The encapsulated packet exits the device from target port#1.
*
* @version   1
********************************************************************************
*/
#include <tunnel/prvTgfClassifierNshOverVxlanGpeUseCase.h>
#include <oam/prvTgfOamGeneral.h>
#include <cpss/dxCh/dxChxGen/pha/cpssDxChPha.h>
#include <cpss/dxCh/dxChxGen/oam/cpssDxChOam.h>




/*************************** Constants definitions ***********************************/

/* PHA fw thread ID of Classifier NSH over VXLAN-GPE */
#define PRV_TGF_PHA_THREAD_ID_CLASSIFIER_NSH_OVER_VXLAN_GPE_CNS  21

/* Size of packets in bytes include CRC */
#define PRV_TGF_PACKET_SIZE_CLASSIFIER_NSH_OVER_VXLAN_GPE_CNS   166
#define PRV_TGF_MAX_PACKET_SIZE_CNS         PRV_TGF_PACKET_SIZE_CLASSIFIER_NSH_OVER_VXLAN_GPE_CNS

/* Number of packets */
#define PRV_TGF_BURST_COUNT_CNS             2

/* Template profile index */
#define PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS  1

/* Tunnel start entry index */
#define PRV_TGF_TUNNEL_START_INDEX_CNS      8

/* Ports number allocation */
#define PRV_TGF_INGRESS_PORT_IDX_CNS        0
#define PRV_TGF_TARGET_PORT_IDX_CNS         1

/* Default VLAN Id */
#define PRV_TGF_DEFAULT_VLANID_CNS          1

/* Offset to IPv6 part in the output packet */
#define PRV_TGF_IPV6_PKT_OFFSET_CNS     (TGF_L2_HEADER_SIZE_CNS  + \
                                         TGF_VLAN_TAG_SIZE_CNS   + \
                                         TGF_ETHERTYPE_SIZE_CNS)

/* Offset to UDP part in the output packet */
#define PRV_TGF_UDP_PKT_OFFSET_CNS      (PRV_TGF_IPV6_PKT_OFFSET_CNS  + \
                                         TGF_IPV6_HEADER_SIZE_CNS)

/* Offset to NSH part in the output packet */
#define PRV_TGF_NSH_HDR_PKT_OFFSET_CNS  (PRV_TGF_UDP_PKT_OFFSET_CNS + \
                                         TGF_UDP_HEADER_SIZE_CNS    + \
                                         TGF_VXLAN_GPE_HEADER_SIZE_CNS)

/* NSH Service Path header SPI value placed here in 24 MS bits  */
#define PRV_TGF_NSH_SP_HDR_VAL_CNS         0x11223300

/* NSH SPI (Service Path Identifier) bit location within 128 bits of profile template
   0:63=>VXLAN-GPE, 64:95=>NSH Base header, 96:127=>NSH Service Path header */
#define PRV_TGF_NSH_SPI_OFST_CNS           96

/* PHA template for Classifier NSH over VXLAN Thread. Represent Source Node ID */
#define PRV_TGF_PHA_TEMPLATE_NSH_SRC_NODE_ID  0xABC

/* PHA metadata[31:16] for Classifier NSH over VXLAN Thread. Represent Tenant ID */
#define PRV_TGF_PHA_METADATA_NSH_TENANT_ID    0x1111

/* PHA metadata[15:0] for Classifier NSH over VXLAN Thread. Represent Source Class */
#define PRV_TGF_PHA_METADATA_NSH_SOURCE_CLASS 0x3333

/* NSH UDP destination port number  */
#define PRV_TGF_NSH_UDP_DST_PORT_CNS        4790

/*************************** static DB definitions ***********************************/
/* DB to hold input packet information */
static TGF_PACKET_STC   packetInfo;

/* DB to hold target port interface */
static CPSS_INTERFACE_INFO_STC  targetPortInterface;

/* Should holds port counters */
static GT_U32    actualPacketsNum[PRV_TGF_BURST_COUNT_CNS];
static GT_U32    actualBytesNum[PRV_TGF_BURST_COUNT_CNS];

/* DB to hold relevant PHA threads information */
static CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC     commonInfo;
static CPSS_DXCH_PHA_THREAD_INFO_UNT            extInfo;
static PRV_TGF_TUNNEL_START_GEN_IP_PROFILE_STC  profileData;
static PRV_TGF_TUNNEL_START_ENTRY_UNT           tunnelEntry;

/* DB to hold IPv6 IP addresses */
static TGF_IPV6_ADDR prvTgfTunnelStartSrcIPv6 = {0x2001, 0x2002, 0x2003, 0x2004, 0x2005, 0x2006, 0x2007, 0x2008};
static TGF_IPV6_ADDR prvTgfTunnelStartDstIPv6 = {0x3001, 0x3002, 0x3003, 0x3004, 0x3005, 0x3006, 0x3007, 0x3008};

/* DB to hold TS Extension byte location */
static GT_U8 prvTgfTsExtensionByteLocationArr[4] =
    {
        CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EPORT_TS_EXT_3_E,
        CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EPORT_TS_EXT_2_E,
        CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EPORT_TS_EXT_1_E,
        CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EPORT_TS_EXT_0_E
    };

/* Tunnel section: L2 part */
static TGF_PACKET_L2_STC prvTgfPacketTunnelL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x44, 0x04},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x45, 0x05}                /* saMac */
};


/* Original Ingress Packet: L2 part */
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
    sizeof(prvTgfPayloadDataArr),                      /* dataLength */
    prvTgfPayloadDataArr                               /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfOriginalPacketL2Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* parameters that are needed to be restored */
static struct
{
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC                egressInfo;
    PRV_TGF_TUNNEL_START_ENTRY_UNT                  tunnelEntry;
    CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT  cfgTabAccMode;
    GT_U32                                          tsExtension;
} prvTgfRestoreCfg;


/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/


/**
* @internal prvTgfClassifierNshOverVxlanGpeBridgeConfigSet function
* @endinternal
*
* @brief   Bridge test configurations
*          Forward the packet to ePort#1 according to MAC
*          destination address
*/
static GT_VOID prvTgfClassifierNshOverVxlanGpeBridgeConfigSet
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
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgDefFdbMacEntryOnPortSet");
}


/**
* @internal prvTgfClassifierNshOverVxlanGpeHaConfigSet function
* @endinternal
*
* @brief   HA test configurations
*          Add tunnel of L2|vlan|EtherType|IPv6|udp|vxlan-gpe|NSH headers
*          Update PHA fw thread in which 16B of NSH metadata will be added
*
*/
static GT_VOID prvTgfClassifierNshOverVxlanGpeHaConfigSet
(
    GT_VOID
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC    egressInfo;
    GT_U32                              tsExtension;
    GT_U32                              i,bitIndx,byteIndx,byteOfst;

    PRV_UTF_LOG0_MAC("======= Setting HA Configuration =======\n");

    /* AUTODOC: Get TS Extension value for restortion */
    rc = cpssDxChTunnelStartEntryExtensionGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS], &prvTgfRestoreCfg.tsExtension);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChTunnelStartEntryExtensionGet");

    /* AUTODOC: Set TS Extension */
    tsExtension = PRV_TGF_NSH_SP_HDR_VAL_CNS;
    rc = cpssDxChTunnelStartEntryExtensionSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS], tsExtension);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChTunnelStartEntryExtensionSet");

    /* AUTODOC: Save ePort attributes configuration */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                                                       prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                                       &(prvTgfRestoreCfg.egressInfo));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoGet");

    /* AUTODOC: Set target port attributes configuration */
    /* AUTODOC: Tunnel Start = ENABLE, tunnelStartPtr = 8, tsPassenger = ETHERNET */
    cpssOsMemSet(&egressInfo,0,sizeof(egressInfo));
    egressInfo = prvTgfRestoreCfg.egressInfo;
    egressInfo.tunnelStart            = GT_TRUE;
    egressInfo.tunnelStartPtr         = PRV_TGF_TUNNEL_START_INDEX_CNS;
    egressInfo.tsPassengerPacketType  = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                                       &egressInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoSet");

    /* AUTODOC: Set Tunnel Start entry 8 with: */
    /* AUTODOC: Type=CPSS_TUNNEL_GENERIC_IPV6_E, tagEnable=TRUE, vlanId=1, ipHeaderProtocol=UDP */
    /* AUTODOC: udpSrcPort = 0x2000, udpDstPort=4790, MACDA = 00:00:00:00:44:04 */
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));
    /* Set Tunnel IPv6 addresses */
    for (i=0; i<8; i++)
    {
        tunnelEntry.ipv6Cfg.destIp.arIP[i*2]     = (GT_U8)(prvTgfTunnelStartDstIPv6[i] >> 8);
        tunnelEntry.ipv6Cfg.destIp.arIP[i*2 + 1] = (GT_U8) prvTgfTunnelStartDstIPv6[i];
        tunnelEntry.ipv6Cfg.srcIp.arIP[i*2]      = (GT_U8)(prvTgfTunnelStartSrcIPv6[i] >> 8);
        tunnelEntry.ipv6Cfg.srcIp.arIP[i*2 + 1]  = (GT_U8) prvTgfTunnelStartSrcIPv6[i];
    }
    tunnelEntry.ipv6Cfg.tagEnable        = GT_TRUE;
    tunnelEntry.ipv6Cfg.vlanId           = PRV_TGF_DEFAULT_VLANID_CNS;
    tunnelEntry.ipv6Cfg.ttl              = 20;
    tunnelEntry.ipv6Cfg.ipHeaderProtocol = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
    tunnelEntry.ipv6Cfg.profileIndex     = PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS;
    tunnelEntry.ipv6Cfg.udpSrcPort       = 0x2000;
    tunnelEntry.ipv6Cfg.udpDstPort       = PRV_TGF_NSH_UDP_DST_PORT_CNS; /*4790*/
    cpssOsMemCpy(tunnelEntry.ipv6Cfg.macDa.arEther, prvTgfPacketTunnelL2Part.daMac, sizeof(TGF_MAC_ADDR));
    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_INDEX_CNS, CPSS_TUNNEL_GENERIC_IPV6_E, &tunnelEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartEntrySet");

    /* AUTODOC: configure tunnel start profile for Classifier NSH over VXLAN-GPE */
    cpssOsMemSet(&profileData,0,sizeof(profileData));
    profileData.templateDataSize = PRV_TGF_TUNNEL_START_TEMPLATE_DATA_SIZE_16B_E; /* 16 bytes */

    /* Fill 128 bits of profile template with SPI value taken from TS Extension written
       earlier in TS Entry and the rest will be filled with zeros (default). Starting from
       SPI location within the template (after 96 bits) need to configure each bit in the
       template the byte location in the TS Extension word from where it will take the value */

    /* Outer loop: byte resolution for setting different byte location within TS Extension word.
       Since SPI size is 24 bits need to run it over 3 bytes (byte3,byte2 till byte1).
       Inner loop: bits resolution. Running over 8 bits */
    byteOfst = 0;
    for (byteIndx=0;byteIndx<3;byteIndx++)
    {
        for (bitIndx=PRV_TGF_NSH_SPI_OFST_CNS+byteOfst;     \
             bitIndx<(PRV_TGF_NSH_SPI_OFST_CNS+8)+byteOfst; \
             bitIndx++)
        {
            /* Set value of each bit in the template according to value taken from
               the right byte location of TS Extension word */
            profileData.templateDataBitsCfg[bitIndx] = prvTgfTsExtensionByteLocationArr[byteIndx];
        }
        byteOfst +=8;
    }
    rc = prvTgfTunnelStartGenProfileTableEntrySet(prvTgfDevNum,
                                                  PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS, &profileData);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartGenProfileTableEntrySet");


    /**** PHA fw threads configurations ****/
    /* AUTODOC: Assign Classifier NSH over VXLAN-GPE fw thread to target port */
    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                        GT_TRUE, PRV_TGF_PHA_THREAD_ID_CLASSIFIER_NSH_OVER_VXLAN_GPE_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaPortThreadIdSet");

    /* AUTODOC: Set the thread entry - PRV_TGF_PHA_THREAD_ID_CLASSIFIER_NSH_OVER_VXLAN_GPE_CNS */
    cpssOsMemSet(&commonInfo, 0, sizeof(commonInfo));
    extInfo.classifierNshOverVxlanGpe.source_node_id = PRV_TGF_PHA_TEMPLATE_NSH_SRC_NODE_ID;
    commonInfo.statisticalProcessingFactor = 0;
    commonInfo.busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;
    commonInfo.stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;
    rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
                                     PRV_TGF_PHA_THREAD_ID_CLASSIFIER_NSH_OVER_VXLAN_GPE_CNS,
                                     &commonInfo,
                                     CPSS_DXCH_PHA_THREAD_TYPE_CLASSIFIER_NSH_OVER_VXLAN_GPE_E,
                                     &extInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaThreadIdEntrySet");
}

/**
* @internal prvTgfClassifierNshOverVxlanGpeEpclConfigSet function
* @endinternal
*
* @brief   EPCL test configurations
*          Set Desc<PHA metadata[31:16] = Tenant ID value
*          Set Desc<PHA metadata[15:0] = Source Class value
*/
static GT_VOID prvTgfClassifierNshOverVxlanGpeEpclConfigSet
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

    /* AUTODOC: init PCL Engine for Egress PCL for target port */
    rc = prvTgfPclDefPortInit(
            prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_EGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclDefPortInit");

    /* AUTODOC: enable EPCL on Target port */
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: Get EPCL configuration table access mode and save it for restortion */
    rc = cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeGet(prvTgfDevNum,
                                                                &prvTgfRestoreCfg.cfgTabAccMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeGet");

    /* AUTODOC: Configure "Access to the EPCL configuration" table with Port Mode */
    rc = cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet(prvTgfDevNum,
                                CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet");

    /* AUTODOC: set PCL rule 0 */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    /* mask & pattern for pclid */
    mask.ruleEgrStdIpL2Qos.common.pclId    = 0x3FF;
    pattern.ruleEgrStdIpL2Qos.common.pclId = (GT_U16)PRV_TGF_PCL_DEFAULT_ID_MAC(
                                                      CPSS_PCL_DIRECTION_EGRESS_E,
                                                      CPSS_PCL_LOOKUP_0_E,
                                                      prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS]);
    ruleIndex                           = 0;
    ruleFormat                          = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;
    action.pktCmd                       = CPSS_PACKET_CMD_FORWARD_E;
    action.egressPolicy                 = GT_TRUE;
    action.epclPhaInfo.phaThreadType    = PRV_TGF_PCL_ACTION_EGRESS_PHA_THREAD_CLASSIFIER_NSH_OVER_VXLAN_GPE_E;
    action.epclPhaInfo.phaThreadUnion.epclClassifierNshOverVxlanGpe.classifierNshOverVxlanGpe_TenantID = PRV_TGF_PHA_METADATA_NSH_TENANT_ID;
    action.epclPhaInfo.phaThreadUnion.epclClassifierNshOverVxlanGpe.classifierNshOverVxlanGpe_SourceClass = PRV_TGF_PHA_METADATA_NSH_SOURCE_CLASS;
    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclRuleSet, prvTgfDevNum:%d,"
                                            "ruleFormat:%d, ruleIndex:%d", prvTgfDevNum, ruleFormat, ruleIndex);
}



/**
* internal prvTgfClassifierNshOverVxlanGpeConfigSet function
* @endinternal
*
* @brief   Classifier NSH over VXLAN-GPE use case configurations
*/
GT_VOID prvTgfClassifierNshOverVxlanGpeConfigSet
(
    GT_VOID
)
{
    /* Bridge Configuration */
    prvTgfClassifierNshOverVxlanGpeBridgeConfigSet();

    /* HA Configuration */
    prvTgfClassifierNshOverVxlanGpeHaConfigSet();

    /* EPCL Configuration */
    prvTgfClassifierNshOverVxlanGpeEpclConfigSet();
}



/**
* @internal prvTgfClassifierNshOverVxlanGpeConfigRestore function
* @endinternal
*
* @brief   Classifier NSH over VXLAN-GPE configurations restore.
* @note    1. Restore Bridge Configuration
*          2. Restore HA Configuration
*          3. Restore EPCL Configuration
*          4. Restore base Configuration
*/
GT_VOID prvTgfClassifierNshOverVxlanGpeConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    PRV_UTF_LOG0_MAC("======= Restore configurations =======\n");

    /* -------------------------------------------------------------------------
     * 1. Restore Bridge Configuration
     */
    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgFdbFlush");

    /* -------------------------------------------------------------------------
     * 2. Restore HA Configuration
     */

    /* AUTODOC: restore TS Extension */
    rc = cpssDxChTunnelStartEntryExtensionSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS], prvTgfRestoreCfg.tsExtension);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChTunnelStartEntryExtensionSet");

    /* AUTODOC: restore ePort attributes */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                                       &(prvTgfRestoreCfg.egressInfo));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoSet");

    /* AUTODOC: Clear PHA fw thread from target port */
    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum,
                                    prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                    GT_FALSE,
                                    0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaPortThreadIdSet");

    /* -------------------------------------------------------------------------
     * 3. Restore EPCL Configuration
     */
    /* AUTODOC: Invalidate PCL rule 0 */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, 0, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclRuleValidStatusSet, index = %d", 0);

    /* AUTODOC: restore Access mode */
    rc = cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet(prvTgfDevNum, prvTgfRestoreCfg.cfgTabAccMode);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "Failure in cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet");

    /* AUTODOC: disable all PCL configurations on the ports */
    prvTgfPclPortsRestoreAll();

    /* -------------------------------------------------------------------------
     * 4. Restore base Configuration
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
* internal tgfClassifierNshOverVxlanGpeVerification function
* @endinternal
*
* @brief  Classifier NSH over VXLAN-GPE use case verification
*         Do the following:
*         - Check IPv6 payload length (should include addition of 16B)
*         - Check UDP length (should include addition of 16B)
*         - Check UDP Destination port = 4790
*         - Check NSH Base header was not overwritten
*         - Check NSH Service Path header (SPI field) = as set for TS extension
*         - Check NSH metadata:
*             - Check (<D>=0,<F>=3,<R>=0,<Source Node ID>=PHA template)
*             - Check 6 MS bits of <Source Interface ID> are zeros
*             - Check <Tenant ID> = PHA metadata[31:16]
*             - Check <Source Class> = PHA metadata[15:0]
*         - Check MAC addresses of passenger are not damaged
*         - Check payload data of passenger is not damaged
*/
GT_VOID tgfClassifierNshOverVxlanGpeVerification
(
    IN  GT_U8  * packetBuf
)
{
    GT_U32              pktOffset,payloadPktOffset;
    GT_U32              nshValActualVal,nshValExpectedVal;
    GT_U32              i;
    GT_U32              daMacErr=0, saMacErr=0, passengerPayloadErr=0;
    TGF_PACKET_L2_STC   receivedMac;
    GT_U32              recPassengerPayload, expPassengerPayload;


    PRV_UTF_LOG0_MAC("======= Check content of output packet =======\n");


    PRV_UTF_LOG0_MAC("======= Check IPv6 Payload Length  =======\n");

    /* AUTODOC: Update offset to point to NSH Base header */
    pktOffset = PRV_TGF_IPV6_PKT_OFFSET_CNS + 4;

    /* Read IPv6 payload length from packet */
    nshValActualVal = (GT_U32)((packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));
    nshValActualVal >>=16;

    /* Should be 104B (0x68) for 64b payload passenger data */
    nshValExpectedVal = 104;
    UTF_VERIFY_EQUAL2_STRING_MAC(nshValExpectedVal, nshValActualVal,
                                "--- Failure in tgfClassifierNshOverEthernetVerification --- \n"
                                "IPv6 payload size is not as expected => Expected:0x%x != Received:0x%x \n",
                                 nshValExpectedVal, nshValActualVal);


    PRV_UTF_LOG0_MAC("======= Check UDP Header  =======\n");

    /********************* Check UDP Destination port ************************/
    /* AUTODOC: Update offset to point to NSH Base header */
    pktOffset = PRV_TGF_UDP_PKT_OFFSET_CNS;

    /* Read UDP Destination port from packet */
    nshValActualVal = (GT_U32)(packetBuf[pktOffset+3] | (packetBuf[pktOffset+2] << 8));

    /* Should be 4790 (0x12b6) as was configured in HA */
    nshValExpectedVal = PRV_TGF_NSH_UDP_DST_PORT_CNS;
    UTF_VERIFY_EQUAL2_STRING_MAC(nshValExpectedVal, nshValActualVal,
                                "--- Failure in tgfClassifierNshOverEthernetVerification --- \n"
                                "UDP Destination port is not as expected => Expected:0x%x != Received:0x%x \n",
                                 nshValExpectedVal, nshValActualVal);

    /********************* Check UDP length **********************************/
    /* AUTODOC: Update offset to point to UDP length */
    pktOffset += 4;

    /* Read UDP Length from packet */
    nshValActualVal = (GT_U32)((packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));
    nshValActualVal >>=16;
    /* Should be 104B (0x68) for 64b payload passenger data */
    nshValExpectedVal = 104;
    UTF_VERIFY_EQUAL2_STRING_MAC(nshValExpectedVal, nshValActualVal,
                                "--- Failure in tgfClassifierNshOverEthernetVerification --- \n"
                                "UDP length is not as expected => Expected:0x%x != Received:0x%x \n",
                                 nshValExpectedVal, nshValActualVal);


    PRV_UTF_LOG0_MAC("======= Check NSH Base header  =======\n");

    /* AUTODOC: Update offset to point to NSH Base header */
    pktOffset = PRV_TGF_NSH_HDR_PKT_OFFSET_CNS;

    /* Read NSH Base header from packet */
    nshValActualVal = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                              (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));
    /* Should be zero as was configured in profile template */
    nshValExpectedVal = 0x0;
    UTF_VERIFY_EQUAL2_STRING_MAC(nshValExpectedVal, nshValActualVal,
                                "--- Failure in tgfClassifierNshOverEthernetVerification --- \n"
                                "NSH Base header (4B) are not as expected => Expected:0x%x != Received:0x%x \n",
                                 nshValExpectedVal, nshValActualVal);


    PRV_UTF_LOG0_MAC("======= Check NSH Service Path header  =======\n");

    /* AUTODOC: Update offset to point to NSH Base header */
    pktOffset +=4 ;

    /* Read NSH Service Path header from packet */
    nshValActualVal = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                              (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));
    /* Set expected SP header value as was configured in HA */
    nshValExpectedVal = PRV_TGF_NSH_SP_HDR_VAL_CNS;
    UTF_VERIFY_EQUAL2_STRING_MAC(nshValExpectedVal, nshValActualVal,
                                "--- Failure in tgfClassifierNshOverEthernetVerification --- \n"
                                "NSH Service header (4B) are not as expected => Expected:0x%x != Received:0x%x \n",
                                 nshValExpectedVal, nshValActualVal);


    PRV_UTF_LOG0_MAC("======= Check NSH Metadata  =======\n");

    /********************* Check <D>,<F>,<R>,<Source Node ID> ************************/
    /* AUTODOC: Update offset to NSH Metadata */
    pktOffset += 4;

    /* AUTODOC: Read <D>,<F>,<R>,<Source Node ID> from packet NSH metadata (2B) */
    nshValActualVal = (GT_U32)((packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));
    nshValActualVal >>= 16;
    /* AUTODOC: Set expected value: <D>=0,<F>=3,<R>=0,<Source Node ID>=PHA template  */
    nshValExpectedVal = ((0x6<<12)|(PRV_TGF_PHA_TEMPLATE_NSH_SRC_NODE_ID & 0xFFF));
    UTF_VERIFY_EQUAL2_STRING_MAC(nshValExpectedVal, nshValActualVal,
                                "--- Failure in tgfClassifierNshOverEthernetVerification --- \n"
                                "<D>,<F>,<R>,<Source Node ID> from NSH metadata are not as expected => Expected:0x%x != Received:0x%x \n",
                                 nshValExpectedVal, nshValActualVal);

    /********************* Check <Source Interface ID> *******************************/
    /* AUTODOC: Read <Source Interface ID> from packet NSH metadata (2B) */
    nshValActualVal = (GT_U32)(packetBuf[pktOffset+3] | (packetBuf[pktOffset+2] << 8));
    /* Since Desc<Local Dev Src Port> is only 10b and <Source Interface ID> is 16b
       PHA clears 6 MS bits to zero*/
    nshValActualVal &= 0xFC00 ;
    nshValExpectedVal = 0;
    UTF_VERIFY_EQUAL2_STRING_MAC(nshValExpectedVal, nshValActualVal,
                                "--- Failure in tgfClassifierNshOverEthernetVerification --- \n"
                                "6MS bits of <Source Interface ID> from NSH metadata are not zero => Expected:0x%x != Received:0x%x \n",
                                 nshValExpectedVal, nshValActualVal);


    /********************* Check <Tenant ID> ******************************************/
    /* AUTODOC: Update offset to point to second 4B word of NSH Metadata */
    pktOffset  += 4;

    /* AUTODOC: Read <Tenant ID> from packet NSH metadata (2B) */
    nshValActualVal = (GT_U32)(packetBuf[pktOffset+3] | (packetBuf[pktOffset+2] << 8));
    /* AUTODOC: Since field size is 24 bits */
    nshValActualVal &=0xFFFFFF;
    /* AUTODOC: Set expected value: PHA metadata[31:16] = Represent TENANT ID  */
    nshValExpectedVal = (PRV_TGF_PHA_METADATA_NSH_TENANT_ID);
    /* AUTODOC: Since field size is 24 bits */
    nshValExpectedVal &=0xFFFFFF;
    UTF_VERIFY_EQUAL2_STRING_MAC(nshValExpectedVal, nshValActualVal,
                                "--- Failure in tgfClassifierNshOverEthernetVerification --- \n"
                                "<Tenant ID> from NSH metadata is not as expected => Expected:0x%x != Received:0x%x \n",
                                 nshValExpectedVal, nshValActualVal);


    /********************* Check <Source Class> ******************************************/
    /* AUTODOC: Update offset to point to third 4B word of NSH Metadata */
    pktOffset  += 4;

    /* AUTODOC: Read <Source Class> from packet NSH metadata (2B) */
    nshValActualVal = (GT_U32)(packetBuf[pktOffset+3] | (packetBuf[pktOffset+2] << 8));
    /* AUTODOC: Since field size is 16 bits */
    nshValActualVal &=0xFFFF;
    /* AUTODOC: Set expected value: PHA metadata[15:0] = Represent Source Class  */
    nshValExpectedVal = (PRV_TGF_PHA_METADATA_NSH_SOURCE_CLASS);
    /* AUTODOC: Since field size is 16 bits */
    nshValExpectedVal &=0xFFFF;
    UTF_VERIFY_EQUAL2_STRING_MAC(nshValExpectedVal, nshValActualVal,
                                "--- Failure in tgfClassifierNshOverEthernetVerification --- \n"
                                "<Source Class> from NSH metadata is not as expected => Expected:0x%x != Received:0x%x \n",
                                 nshValExpectedVal, nshValActualVal);


    PRV_UTF_LOG0_MAC("======= Check MAC DA SA  =======\n");

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


    /********************* Compare MAC DA ********************************************/
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


    /********************* Compare MAC SA ********************************************/
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



    PRV_UTF_LOG0_MAC("======= Check Passenger payload  =======\n");

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
* internal tgfClassiferNshOverVxlanGpePortCountersVerification function
* @endinternal
*
* @brief  Compare expected vs actual target port counters
*         Target port counters were already read upon packets transmission
*/
GT_VOID tgfClassiferNshOverVxlanGpePortCountersVerification
(
    IN  GT_U32    expPacketsNum,
    IN  GT_U32    actualPacketsNum,
    IN  GT_U32    expBytesNum,
    IN  GT_U32    actualBytesNum
)
{
    PRV_UTF_LOG0_MAC("======= Check target port counters  =======\n\n");

    /* Check expected vs actual and notify if mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(expPacketsNum, actualPacketsNum,
                                 "--- Failure in tgfClassiferNshOverVxlanGpePortCountersVerification --- \n"
                                 "Packets number is not as expected => Expected:0x%x != Received:0x%x \n",
                                  expPacketsNum, actualPacketsNum);

    /* Check expected vs actual and notify if mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(expBytesNum, actualBytesNum,
                                 "--- Failure in tgfClassiferNshOverVxlanGpePortCountersVerification --- \n"
                                 "Bytes number is not as expected => Expected:0x%x != Received:0x%x \n",
                                  expBytesNum, actualBytesNum);
}


/**
* internal prvTgfClassifierNshOverVxlanGpeVerification function
* @endinternal
*
* @brief  Loop over packets check packet content and target port
*         counters
*/
GT_VOID prvTgfClassifierNshOverVxlanGpeVerification
(
    GT_VOID
)
{
    GT_STATUS          rc = GT_OK;
    GT_U8              packetBuf[PRV_TGF_BURST_COUNT_CNS][PRV_TGF_MAX_PACKET_SIZE_CNS]= {{0}, {0}};
    GT_U32             packetLen[PRV_TGF_BURST_COUNT_CNS] = {PRV_TGF_PACKET_SIZE_CLASSIFIER_NSH_OVER_VXLAN_GPE_CNS,
                                                             PRV_TGF_PACKET_SIZE_CLASSIFIER_NSH_OVER_VXLAN_GPE_CNS};

    GT_U32             packetActualLength = 0;
    GT_U8              queue = 0;
    GT_U8              dev = 0;
    TGF_NET_DSA_STC    rxParam;
    GT_U8              pktNum;
    GT_BOOL            getFirst = GT_TRUE;
    GT_U32             expPacketsNum, expBytesNum;

    /* Loop over packets (currently only 1 packet) */
    for (pktNum=0; pktNum<1; pktNum++)
    {
        /* Expects packet to arrive to target port */
        expPacketsNum = 1;
        /* New packet format should be as shown below with total 110 bytes
           |L2|vlan|EtherType|NSH base hdr|NSH Service Path hdr|NSH metadata|
           |payload passenger|CRC| */
        expBytesNum = packetLen[pktNum];
        /* AUTODOC: Check port counters */
        tgfClassiferNshOverVxlanGpePortCountersVerification(expPacketsNum,
                                                            actualPacketsNum[pktNum],
                                                            expBytesNum,
                                                            actualBytesNum[pktNum]);
        /* AUTODOC: Get the captured packets */
        rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&targetPortInterface,
                                                    TGF_PACKET_TYPE_CAPTURE_E,
                                                    getFirst, GT_TRUE, packetBuf[pktNum],
                                                    &packetLen[pktNum], &packetActualLength,
                                                    &dev, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet");

        /* Call the actual verification function to check this packet */
        tgfClassifierNshOverVxlanGpeVerification(packetBuf[pktNum]);

        /* Set to false for next iteration in order to get the next packet from next Rx CPU entry */
        getFirst = GT_FALSE;
    }
}


/**
* @internal prvTgfClassifierNshOverVxlanGpeTest function
* @endinternal
*
* @brief   Classifier NSH over VXLAN-GPE use case test.
*          Create tunnel which includes IPv6, UDP, VXLAN-GPE, NSH headers over Ethernet packet.
*          During this process PHA fw thread is triggered in which NSH metadata is added.
*          At the end packets are captured and content is checked to verify right behaviour.
*          Plus port counters are verified as well.
*/
GT_VOID prvTgfClassifierNshOverVxlanGpeTest
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

    /* AUTODOC: reset counters for all 4 ports */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "Failure in prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficTableRxPcktTblClear");

    /* Enable capture on target port */
    targetPortInterface.type             = CPSS_INTERFACE_PORT_E;
    targetPortInterface.devPort.hwDevNum = prvTgfDevNum;
    targetPortInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&targetPortInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

    /* Set input packet interface structure  */
    packetInfo.numOfParts =  sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);
    packetInfo.partsArray = prvTgfPacketPartArray;
    packetInfo.totalLen   = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");


    PRV_UTF_LOG0_MAC("======= Send first packet =======\n");
    /* Send first packet  */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStartTransmitingEth");

    /* Read counters from target port and save them to be verified */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                   GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfReadPortCountersEth");
    actualPacketsNum[0] =  portCntrs.goodPktsRcv.l[0];
    actualBytesNum[0] = portCntrs.goodOctetsRcv.l[0];

    /* Disable capture on target port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &targetPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");
}




