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
* @file prvTgfMplsSrEntropyLabelUseCase
*
* @brief Unified SR IPv6 Use case
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfMplsGen.h>
#include <mpls/prvTgfMplsUnifiedSRUseCase.h>
#include <cpss/dxCh/dxChxGen/pha/cpssDxChPha.h>
#include <common/tgfTrunkGen.h>

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */
static struct
{
        CPSS_INTERFACE_INFO_STC             physicalInfo;
        PRV_TGF_BRG_EGRESS_PORT_INFO_STC    egressInfo;
} prvTgfRestoreCfg;

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* Phy Port & Eport Numbers */
#define PRV_TGF_RCV_EPORT_IDX_CNS                4444

#define PRV_TGF_SEND_PORT_IDX_CNS                0
#define PRV_TGF_RECEIVE_PORT_IDX_CNS             1

#define PRV_TGF_PHA_THREAD_ID_UNIFIED_SR         19

#define PRV_TGF_VLANID_10_CNS                    10
#define PRV_TGF_MPLS_UDP_PORT_IDX_CNS            6635

/* Tunnel start entry index */
#define PRV_TGF_TUNNEL_START_INDEX_CNS           8

/* template profile index */
#define PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS       1

#define PRV_UTF_LOG_VERIFY_GT_OK(rc, name)                                   \
    if (GT_OK != (rc))                                                       \
    {                                                                        \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", name, rc);       \
    }

#define IS_SUPPORT_CRC_RETAIN                                                \
    /* sip6 not support 'retain inner CRC' */                                \
    ((GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(prvTgfDevNum,                  \
     PRV_CPSS_DXCH_XCAT_PASSENGER_ETH_PACKET_CRC_CANT_BE_RETAINED_WA_E) ||   \
        PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum)) ? 0 : 1)

/************************ common parts of packets **************************/

static GT_U16                           udpDstPort[4]      = {4,8,12,16};
static GT_U16                           udpSrcPort         = 64212;
static PRV_TGF_TUNNEL_START_ENTRY_UNT   tunnelEntry;
static GT_BOOL                          tunnelStartEnabled = GT_FALSE;
static TGF_IPV6_ADDR prvTgfPhaTemplateIPv6                 = {0x1001, 0x1002, 0x1003, 0x1004, 0x1005, 0x1006, 0x1007, 0x1008};
static TGF_IPV6_ADDR prvTgfTunnelStartSrcIPv6              = {0x2001, 0x2002, 0x2003, 0x2004, 0x2005, 0x2006, 0x2007, 0x2008};
static TGF_IPV6_ADDR prvTgfTunnelStartDstIPv6              = {0x3001, 0x3002, 0x3003, 0x3004, 0x3005, 0x3006, 0x3007, 0x3008};

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x01},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x11, 0x11}                 /* saMac */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29
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
    {1,  1,  1,  1},   /* srcAddr */
    {2,  2,  2,  2}    /* dstAddr */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    3, 0, PRV_TGF_VLANID_10_CNS                          /* pri, cfi, VlanId */
};

/* Packet ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = { TGF_ETHERTYPE_0800_IPV4_TAG_CNS };

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPayloadDataArr),  /* dataLength */
    prvTgfPayloadDataArr           /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Length of packet */
#define PRV_TGF_PACKET_LEN_CNS    \
    TGF_L2_HEADER_SIZE_CNS      + \
    TGF_VLAN_TAG_SIZE_CNS       + \
    TGF_ETHERTYPE_SIZE_CNS      + \
    TGF_IPV4_HEADER_SIZE_CNS    + \
    sizeof(prvTgfPayloadDataArr)

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                  /* totalLen */
    (sizeof(prvTgfPacketPartArray)
     / sizeof(prvTgfPacketPartArray[0])),    /* numOfParts */
    prvTgfPacketPartArray                    /* partsArray */
};

/**
* internal prvTgfTunnelConfigSet function
* @endinternal
*
* @brief   Tunnel configurations for Unified SR IPv6 use case
*/
static GT_VOID prvTgfTunnelConfigSet
(
    GT_U32  index
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_TUNNEL_START_ENTRY_UNT          tunnelEntryGet;
    CPSS_TUNNEL_TYPE_ENT                    tunnelTypeGet;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC        egressInfo;
    PRV_TGF_TUNNEL_START_GEN_IP_PROFILE_STC profileData;
    GT_U32                                  ii = 0;
    GT_U32                                  i,j;

    PRV_UTF_LOG0_MAC("======= Setting Tunnel Configuration =======\n");

    /* Add the mapping only for the first time
     * This API will be called for updating the UDP dst port also */
    if(tunnelStartEnabled == GT_FALSE)
    {
        /* save ePort attributes configuration */
        rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
                &(prvTgfRestoreCfg.egressInfo));
        PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoGet);

        /* Set ePort attributes configuration */
        cpssOsMemSet(&egressInfo,0,sizeof(egressInfo));
        egressInfo.tunnelStart            = GT_TRUE;
        egressInfo.tunnelStartPtr         = PRV_TGF_TUNNEL_START_INDEX_CNS;
        egressInfo.tsPassengerPacketType  = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;

        /* AUTODOC: config ePort#4444 attributes: */
        /* AUTODOC:   Tunnel Start=ENABLE, tunnelStartPtr = 8 */
        /* AUTODOC:   tsPassenger=ETHERNET */
        rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
                &egressInfo);
        PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

        /*** Set a tunnel start entry ***/
        cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));

        for (ii = 0; ii < 8; ii++)
        {
            tunnelEntry.ipv6Cfg.destIp.arIP[ii * 2]     = (GT_U8)(prvTgfTunnelStartDstIPv6[ii] >> 8);
            tunnelEntry.ipv6Cfg.destIp.arIP[ii * 2 + 1] = (GT_U8) prvTgfTunnelStartDstIPv6[ii];
            tunnelEntry.ipv6Cfg.srcIp.arIP[ii * 2]     = (GT_U8)(prvTgfTunnelStartSrcIPv6[ii] >> 8);
            tunnelEntry.ipv6Cfg.srcIp.arIP[ii * 2 + 1] = (GT_U8) prvTgfTunnelStartSrcIPv6[ii];
        }
        tunnelEntry.ipv6Cfg.tagEnable        = GT_TRUE;
        tunnelEntry.ipv6Cfg.vlanId           = PRV_TGF_VLANID_10_CNS;
        tunnelEntry.ipv6Cfg.retainCrc        = GT_FALSE;
        tunnelEntry.ipv6Cfg.ttl              = 33;
        tunnelEntry.ipv6Cfg.ipHeaderProtocol = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
        tunnelEntry.ipv6Cfg.profileIndex     = PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS;
        tunnelEntry.ipv6Cfg.udpSrcPort       = udpSrcPort;

        /* Indicates tunnel start is enabled, can skip while calling this API for udp port updation */
        tunnelStartEnabled = GT_TRUE;
    }

    tunnelEntry.ipv6Cfg.udpDstPort       = udpDstPort[index];

    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_INDEX_CNS, CPSS_TUNNEL_GENERIC_IPV6_E, &tunnelEntry);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntrySet: %d, for index[%d]", prvTgfDevNum, index);

    /* AUTODOC: get and check added TS Entry */
    rc = prvTgfTunnelStartEntryGet(prvTgfDevNum, PRV_TGF_TUNNEL_START_INDEX_CNS, &tunnelTypeGet, &tunnelEntryGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntryGet: %d, for index[%d]", prvTgfDevNum, index);

    /* Check vlanId field */
    UTF_VERIFY_EQUAL1_STRING_MAC(tunnelEntryGet.ipv6Cfg.vlanId, tunnelEntry.ipv6Cfg.vlanId,
            "Vlan Id mismatch with configured value for device %d", prvTgfDevNum);

    /* Check srcIp field */
    rc = cpssOsMemCmp(&tunnelEntryGet.ipv6Cfg.srcIp, &tunnelEntry.ipv6Cfg.srcIp, sizeof(GT_IPV6ADDR)) == 0 ?
        GT_OK : GT_FALSE;
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tunnelEntry.ipv6Cfg.srcIp did not match, for index[%d]",
            index);

    /* Check Udp dst port */
    UTF_VERIFY_EQUAL1_STRING_MAC(tunnelEntryGet.ipv6Cfg.udpDstPort, udpDstPort[index],
            "udpDstPort mismatch with configured value for device %d", prvTgfDevNum);

    /* AUTODOC: configure tunnel start profile */
    cpssOsMemSet(&profileData,0,sizeof(profileData));
    profileData.templateDataSize = PRV_TGF_TUNNEL_START_TEMPLATE_DATA_SIZE_4B_E; /* 8 byte */
    profileData.udpSrcPortMode = PRV_TGF_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_IP_GEN_TS_ENTRY_E;
    profileData.hashShiftLeftBitsNumber = 0;
    /* At the begining all bits are zero. In this cycle only "ones" are filled  */
    for (i = 0; i < 8; i++)
    {
        for (j = 0; j < 8; j++)
        {
            if ( i == 1)
            {
                if (j == 3)
                {
                    profileData.templateDataBitsCfg[i*8+j] = PRV_TGF_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_1_E;
                }
            }
            if (i == 2)
            {
                if (j == 6)
                {
                    profileData.templateDataBitsCfg[i*8+j] = PRV_TGF_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_1_E;
                }
            }
        }
    }
    rc = prvTgfTunnelStartGenProfileTableEntrySet(prvTgfDevNum, PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS, &profileData);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntrySet: %d", prvTgfDevNum);
}

/* debug flag to disable using of PHA for the test */
static GT_BOOL prvTgfUnifiedSrIpv6SkipPha = GT_FALSE;
GT_BOOL prvTgfUnifiedSrIpv6SkipPhaSet(GT_BOOL newSkipPha)
{
    GT_BOOL oldSkipPha = prvTgfUnifiedSrIpv6SkipPha;

    prvTgfUnifiedSrIpv6SkipPha = newSkipPha;

    return oldSkipPha;
}

/**
* internal prvTgfPhaConfigSet function
* @endinternal
*
* @brief   PHa Configurations for Unified SR IPv6 use case
*/
static GT_VOID prvTgfPhaConfigSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC    commonInfo;
    CPSS_DXCH_PHA_THREAD_INFO_UNT           extInfo;
    GT_U32                                  ii = 0;

    cpssOsMemSet(&commonInfo, 0, sizeof(commonInfo));
    for (ii = 0; ii < 4; ii++)
    {
        extInfo.unifiedSRIpv6.srcAddr.u32Ip[ii] =
                            (GT_U32)((prvTgfPhaTemplateIPv6[ii * 2] << 16) | prvTgfPhaTemplateIPv6[ii * 2 + 1]);
    }
    commonInfo.statisticalProcessingFactor = 0;
    commonInfo.busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;
    commonInfo.stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;

    /* GM does not support PHA */
    if(GT_TRUE == prvUtfIsGmCompilation() || (prvTgfUnifiedSrIpv6SkipPha == GT_TRUE))
        return;

    /* AUTODOC: Assign thread to the Port */
    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum,
                                    prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
                                    GT_TRUE,
                                    PRV_TGF_PHA_THREAD_ID_UNIFIED_SR);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                    "[TGF]: cpssDxChPhaPortThreadIdSet FAILED rc = [%d]", rc);

    /* AUTODOC: Set the thread entry */
    rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
                                     PRV_TGF_PHA_THREAD_ID_UNIFIED_SR,
                                     &commonInfo,
                                     CPSS_DXCH_PHA_THREAD_TYPE_UNIFIED_SR_E,
                                     &extInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                    "[TGF]: cpssDxChPhaThreadIdEntrySet FAILED, rc = [%d]", rc);
}

/**
* @internal prvTgfUnifiedSRIpv6ConfigurationSet function
* @endinternal
*
* @brief   Set Configuration for Unified SR Ipv6 use case
*/
GT_VOID prvTgfUnifiedSrIpv6ConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    CPSS_INTERFACE_INFO_STC                 physicalInfo;
    PRV_TGF_BRG_MAC_ENTRY_STC               macEntry;
    GT_U8                                   tagArray[] = {1, 1};

    PRV_UTF_LOG0_MAC("======= Setting Entropy Label Configuration =======\n");

    /* MAC Entry */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    macEntry.key.entryType                  = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId         = PRV_TGF_VLANID_10_CNS;
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum  = prvTgfDevNum;
    macEntry.isStatic                       = GT_TRUE;
    macEntry.daRoute                        = GT_TRUE;
    macEntry.dstInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    /* Physical port Entry */
    cpssOsMemSet(&physicalInfo, 0, sizeof(CPSS_INTERFACE_INFO_STC));
    physicalInfo.hwDevNum                   = prvTgfDevNum;
    physicalInfo.vlanId                     = PRV_TGF_VLANID_10_CNS;
    physicalInfo.devPort.hwDevNum           = prvTgfDevNum;
    physicalInfo.type                       = CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.portNum            = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];

    /* save ePort mapping configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                              PRV_TGF_RCV_EPORT_IDX_CNS,
                                              &(prvTgfRestoreCfg.physicalInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableGet);

    /* Configure Physical port to ePort Mapping */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                              PRV_TGF_RCV_EPORT_IDX_CNS,
                                              &physicalInfo);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableSet);

    /* AUTODOC: create VLAN 10 with tagged ports [0,1] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_10_CNS, prvTgfPortsArray,
                                           NULL, tagArray, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* create macEntry */
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite: %d", prvTgfDevNum);

    /* Configure the PHA entry */
    prvTgfPhaConfigSet();
}

/**
* internal prvTgfUnifiedSRIPv6Verification function
* @endinternal
*
* @brief   MPLS entropy label use case verification
*/
GT_VOID prvTgfUnifiedSRIpv6Verification
(
    GT_U32   index
)
{
    GT_STATUS                           rc = GT_OK;
    GT_U8                               packetBuf[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32                              packetLen = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32                              packetActualLength = 0;
    GT_U8                               queue = 0;
    GT_U8                               dev = 0;
    TGF_NET_DSA_STC                     rxParam;
    GT_BOOL                             getFirst = GT_TRUE;
    CPSS_INTERFACE_INFO_STC             portInterface;
    GT_U32                              pktOffset = 0;
    GT_U32                              ii = 0;

    PRV_UTF_LOG0_MAC("======= Packet Verification =======\n");

    /* setup portInterface for capturing */
    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));
    portInterface.type              = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];

    getFirst = (index == 0)?GT_TRUE:GT_FALSE;
    rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&portInterface,
            TGF_PACKET_TYPE_CAPTURE_E,
            getFirst, GT_TRUE, packetBuf,
            &packetLen, &packetActualLength,
            &dev, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
            "tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet");

    pktOffset = TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + 8;

    /* IPv6 SIP is updated by PHA template
     * verify the SIP matching with PHA template Data */
    for (ii = 0; ii < 8; ii++)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC((GT_U16)(packetBuf[pktOffset + 1] | (packetBuf[pktOffset] << 8)),
                                     prvTgfPhaTemplateIPv6[ii],
                                     "packet src ip is not as expectd for index[%d]", ii);
        pktOffset+=2;
    }

    pktOffset+=18; /* DIP - 16Byte, UDP src Port - 2Byte */
    /* Verify UDP Trg Port inserted by PHA */
    UTF_VERIFY_EQUAL0_STRING_MAC((GT_U32)(packetBuf[pktOffset + 1] | (packetBuf[pktOffset] << 8)),
                                 PRV_TGF_MPLS_UDP_PORT_IDX_CNS,
                                 "Packet UDP dst Port did not match");

    pktOffset+=10; /* Lable - 4Byte + 6 Byte of remaining UDP header */
    /* Verify New labels added by PHA, Copied from Original SIP */
    for (ii = 2 * (3 - index); ii < 8; ii++)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC((GT_U16)(packetBuf[pktOffset + 1] | (packetBuf[pktOffset] << 8)),
                                     prvTgfTunnelStartSrcIPv6[ii],
                                     "packet label is not as expectd for index[%d]", ii);
        pktOffset+=2;
    }
}


/**
* @internal prvTgfUnifiedSrIpv6TrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfUnifiedSrIpv6TrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                       rc          = GT_OK;
    CPSS_INTERFACE_INFO_STC         portInterface;
    GT_U32                          index = 0;

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* AUTODOC: clear counters and capturing RxPcktTable */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* AUTODOC: setup portInterface for capturing */
    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));
    portInterface.type              = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS];

    /* AUTODOC: Update tunnel start and Transmit packets */
    for(index = 0; index < 4; index++)
    {
        /* Configure tunnel start entry */
        prvTgfTunnelConfigSet(index);

        /* AUTODOC: enable capture on receiving port */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);

        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, 1, 0, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* AUTODOC: disable capture on receiving port */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS]);

        prvTgfUnifiedSRIpv6Verification(index);
    }
}

/**
* @internal prvTgfUnifiedSrIpv6ConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfUnifiedSrIpv6ConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    /* -------------------------------------------------------------------------
     * 1. Restore Entropy Label specific Configuration
     */

    /* AUTODOC: Restore thread to the ePort and egressinfo */
    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            GT_FALSE,
            PRV_TGF_PHA_THREAD_ID_UNIFIED_SR);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "[TGF]: cpssDxChPhaPortThreadIdSet FAILED, rc = [%d]", rc);

    /* Restore mapping */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
            PRV_TGF_RCV_EPORT_IDX_CNS,
            &(prvTgfRestoreCfg.physicalInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortTargetMappingTableSet);

    /* Restore egress info */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECEIVE_PORT_IDX_CNS],
            &(prvTgfRestoreCfg.egressInfo));
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfBrgEportToPhysicalPortEgressPortInfoSet);

    tunnelStartEnabled = GT_FALSE;

    /* -------------------------------------------------------------------------
     * 2. Restore Base Configuration
     */

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "tgfTrafficTableRxPcktTblClear");

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "prvTgfBrgFdbFlush");
}
