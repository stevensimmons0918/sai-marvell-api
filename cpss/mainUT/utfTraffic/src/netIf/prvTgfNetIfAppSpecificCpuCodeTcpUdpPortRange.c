/*******************************************************************************
*              (C), Copyright 2001, Marvell International Ltd.                 *
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
* @file prvTgfNetIfAppSpecificCpuCodeTcpUdpPortRange.c
*
* @brief Test to check CPU code assignment based on L4 dest/src port mode (TO_CPU command).
*
* Configurations: RegisterName - TCP/UDP Dest/Src Port Range CPU Code Entry Word 0 and
*                                TCP/UDP Dest/Src Port Range CPU Code Entry Word 1
*
*      At index 1 : configure dst port range : x-y, CPU code 3
*      At index 2 : configure src port range : m-n, CPU code 5
*      At index 3 : configure src+dst port range : a-b, CPU code 7
*
* Note : no Overlapping ranges
*
* Traffic:
*      TestCase 1: Send traffic, with dst port in range x-y, src port not in any range
*                  Expect: CPU code 3
*      TestCase 2: Send traffic, with dst port not in any range, src port not in any range
*                  Expect: CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E CPU code
*      TestCase 3: Send traffic, with dst port in range m-n, src port not in any range
*                  Expect: CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E CPU code
*      TestCase 4: Send traffic, with dst port in range a-b, src port not in any range
*                  Expect:  CPU code 7

*      TestCase 5: Send traffic, with dst port not in any range, src port in range x-y
*                  Expect: CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E CPU code
*      TestCase 6: Send traffic, with dst port not in any range, src port in range m-n
*                  Expect: CPU code 5
*      TestCase 7: Send traffic, with dst port not in any range, src port in range a-b
*                  Expect: CPU code 7
*
*      TestCase 8: Send traffic, with dst port in range x-y, src port in range m-n
*                  Expect: CPU code 3
*      TestCase 9: Send traffic, with dst port in range x-y, src port in range a-b
*                  Expect: CPU code 3
*
*      TestCase 10: Send traffic, with dst port in range a-b, src port in range m-n
*                   Expect: CPU code 5
*      TestCase 11: Send traffic, with dst port in range a-b, src port in range a-b
*                   Expect: CPU code 7
*
*
* @version   1
*************************************************************************************
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
#include <common/tgfCscdGen.h>
#include <common/tgfNetIfGen.h>
#include <common/tgfBridgeGen.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/****************************** Test Packets *********************************/

PRV_TGF_NETIF_APP_SPECIFIC_CPUCODE_TCP_UDP_PORT_RANGE_STC l4TypeInfoArr[3];

#define PRV_TGF_MAX_ITERATION_NAME_LEN_CNS 300

/* Ports */
#define PRV_TGF_INGRESS_PORT_IDX_CNS       0
#define PRV_TGF_EGRESS_PORT_IDX_CNS        1

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x14, 0x01},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                 /* saMac */
};

/* ethertype part of packet1 */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart =
    {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2E,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    6,                  /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,   /* csum */
    {22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* packet's TCP part */
static TGF_PACKET_TCP_STC prvTgfPacketTcpPart =
{
    0,                  /* src port (will be initialized later) */
    0,                  /* dst port (will be initialized later) */
    1,                  /* sequence number */
    2,                  /* acknowledgment number */
    0,                  /* data offset */
    0,                  /* reserved */
    0x00,               /* flags */
    4096,               /* window */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS ,  /* csum */
    0                   /* urgent pointer */
};

/* DATA of packet */
static GT_U8 prvTgfPacketPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPacketPayloadDataArr),                 /* dataLength */
    prvTgfPacketPayloadDataArr                          /* dataPtr */
};

/* size of packet that came from ingress port */
#define PRV_TGF_INGRESS_PACKET_SIZE_CNS (  TGF_L2_HEADER_SIZE_CNS        \
                                           + TGF_ETHERTYPE_SIZE_CNS    \
                                           + TGF_IPV4_HEADER_SIZE_CNS \
                                           + TGF_TCP_HEADER_SIZE_CNS  \
                                           + sizeof(prvTgfPacketPayloadDataArr))

/* Part of packet that came from ingress port */
static TGF_PACKET_PART_STC prvTgfIngressPacketPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part}, /* type, partPtr  */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_TCP_E,       &prvTgfPacketTcpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* a network packet info */
static TGF_PACKET_STC prvTgfIngressPacketInfo = {
    PRV_TGF_INGRESS_PACKET_SIZE_CNS,                                    /* totalLen */
    sizeof(prvTgfIngressPacketPartsArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
    prvTgfIngressPacketPartsArray                                       /* partsArray */
};

/* default number of packets to send */
static GT_U32  prvTgfBurstCount = 1;


/******************************************************************************\
 *                            Private variables                               *
\******************************************************************************/

/* Scenarios */
static const GT_CHAR prvTgfIterationNameArray[][PRV_TGF_MAX_ITERATION_NAME_LEN_CNS] = {
    "Case 1: Sending traffic, with dst port in range of min-max dst port, src port not in any range  ...\n",
    "Case 2: Sending traffic, with dst port not in any range, src port not in any range...\n",
    "Case 3: Sending traffic, with dst port in range of min-max src port, src port not in any range...\n",
    "Case 4: Sending traffic, with dst port in range of min-max dst/src port, src port not in any range...\n",
    "Case 5: Sending traffic, with dst port not in any range, src port in range of min-max dst port...\n",
    "Case 6: Sending traffic, with dst port not in any range, src port in the range of min-max src port...\n",
    "Case 7: Sending traffic, with dst port not in any range, src port in the range of min-max dst/src port...\n",
    "Case 8: Sending traffic, with dst port in the range of min-max dst port, src port in the range of min-max src port ...\n",
    "Case 9: Sending traffic, with dst port in the range of min-max dst port, src port in the range of min-max dest/src port ...\n",
    "Case 10: Sending traffic, with dst port in range of min-max dst/src port, src port min-max src port ...\n",
    "Case 11: Sending traffic, with dst port in range of min-max dst/src port, src port min-max dst/src port ...\n"
};

/* L4 Dest and Src port details in each scenerios */
static GT_U32 prvTgfL4DestSrcPortInfoArray[11][2] = {
    {7,16},
    {17,16},
    {4,16},
    {14,16},
    {17,7},
    {17,4},
    {16,14},
    {7,4},
    {7,11},
    {12,4},
    {12,11}
};

/* Expected CPU code for SIP_6_20 devices */
static CPSS_NET_RX_CPU_CODE_ENT expectedCpuCodeSip6_20Arr[11] = {

    CPSS_NET_FIRST_USER_DEFINED_E + 3,
    CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E,
    CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E,
    CPSS_NET_FIRST_USER_DEFINED_E + 7,
    CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E,
    CPSS_NET_FIRST_USER_DEFINED_E + 5,
    CPSS_NET_FIRST_USER_DEFINED_E + 7,
    CPSS_NET_FIRST_USER_DEFINED_E + 3,
    CPSS_NET_FIRST_USER_DEFINED_E + 3,
    CPSS_NET_FIRST_USER_DEFINED_E + 5,
    CPSS_NET_FIRST_USER_DEFINED_E + 7

};

/* Expected CPU code for SIP_6_10 - 6_15 devices */
static CPSS_NET_RX_CPU_CODE_ENT expectedCpuCodeSip6_10Arr[11] = {

    CPSS_NET_FIRST_USER_DEFINED_E + 3,
    CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E,
    CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E,
    CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E, /* erratum EQ-872 - ERR-5209919 */
    CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E,
    CPSS_NET_FIRST_USER_DEFINED_E + 5,
    CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E, /* erratum EQ-872 - ERR-5209919 */
    CPSS_NET_FIRST_USER_DEFINED_E + 3,
    CPSS_NET_FIRST_USER_DEFINED_E + 3,
    CPSS_NET_FIRST_USER_DEFINED_E + 5,
    CPSS_NET_FIRST_USER_DEFINED_E + 7
};

/* Expected CPU code for non SIP_6_10 devices*/
static CPSS_NET_RX_CPU_CODE_ENT expectedCpuCodeArr[11] = {

    CPSS_NET_FIRST_USER_DEFINED_E + 3,
    CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E,
    CPSS_NET_FIRST_USER_DEFINED_E + 5,
    CPSS_NET_FIRST_USER_DEFINED_E + 7,
    CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E,
    CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E,
    CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E,
    CPSS_NET_FIRST_USER_DEFINED_E + 3,
    CPSS_NET_FIRST_USER_DEFINED_E + 3,
    CPSS_NET_FIRST_USER_DEFINED_E + 7,
    CPSS_NET_FIRST_USER_DEFINED_E + 7

};

/************************** PRIVATE DECLARATIONS ****************************/

/**
* @internal prvTgfNetIfAppSpecificCpuCodeTcpUdpPortRangeConfigSave function
* @endinternal
*
* @brief   Save configuration to be restored at the end of test
*/
static GT_VOID prvTgfNetIfAppSpecificCpuCodeTcpUdpPortRangeConfigSave
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32    index = 0;
    GT_BOOL   valid;

    for(index = 1; index <= 3; index++)
    {
        rc = prvTgfNetIfAppSpecificCpuCodeTcpUdpPortRangeGet(prvTgfDevNum, index, &valid, &l4TypeInfoArr[index]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfNetIfAppSpecificCpuCodeTcpUdpPortRangeGet");
    }

    /* force link up for all ports */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");
}

/**
* @internal prvTgfNetIfAppSpecificCpuCodeTcpUdpPortRangeConfigSet function
* @endinternal
*
* @brief   Setup configuration
*/
static GT_VOID prvTgfNetIfAppSpecificCpuCodeTcpUdpPortRangeConfigSet
(
    GT_VOID
)
{
    GT_STATUS                           rc;
    PRV_TGF_BRG_MAC_ENTRY_STC           macEntry;
    PRV_TGF_NETIF_APP_SPECIFIC_CPUCODE_TCP_UDP_PORT_RANGE_STC l4TypeInfo;

    prvTgfNetIfAppSpecificCpuCodeTcpUdpPortRangeConfigSave();

    /* add a MAC entry */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    macEntry.key.key.macVlan.vlanId         = 1;
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum  = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS];
    macEntry.isStatic                       = GT_TRUE;

    macEntry.daCommand                      = PRV_TGF_PACKET_CMD_CNTL_E;
    macEntry.saCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.daRoute                        = GT_FALSE;
    macEntry.userDefined                    = 0;
    macEntry.daQosIndex                     = 0;
    macEntry.saQosIndex                     = 0;
    macEntry.daSecurityLevel                = 0;
    macEntry.saSecurityLevel                = 0;
    macEntry.appSpecificCpuCode             = GT_TRUE;

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02 */
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite: %d", prvTgfDevNum);

    /* AUTODOC: SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Base Configuration =======\n");

    /* Configure the App Specific CPU code based on L4 dest/src port */

    /* Min-Max range for L4 dest port */
    l4TypeInfo.minL4Port    = 6;
    l4TypeInfo.maxL4Port    = 9;
    l4TypeInfo.packetType   = CPSS_NET_TCP_UDP_PACKET_UC_E;
    l4TypeInfo.protocol     = CPSS_NET_PROT_TCP_E;
    l4TypeInfo.cpuCode      = CPSS_NET_FIRST_USER_DEFINED_E + 3;
    l4TypeInfo.l4PortMode   = CPSS_NET_CPU_CODE_L4_DEST_PORT_E;

    rc = prvTgfNetIfAppSpecificCpuCodeTcpUdpPortRangeSet(prvTgfDevNum, 1, &l4TypeInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChNetIfAppSpecificCpuCodeTcpUdpPortRangeSet");

    /* Min-Max range for L4 src port */
    l4TypeInfo.minL4Port    = 0;
    l4TypeInfo.maxL4Port    = 5;
    l4TypeInfo.packetType   = CPSS_NET_TCP_UDP_PACKET_UC_E;
    l4TypeInfo.protocol     = CPSS_NET_PROT_TCP_E;
    l4TypeInfo.cpuCode      = CPSS_NET_FIRST_USER_DEFINED_E + 5;
    l4TypeInfo.l4PortMode   = CPSS_NET_CPU_CODE_L4_SRC_PORT_E;

    rc = prvTgfNetIfAppSpecificCpuCodeTcpUdpPortRangeSet(prvTgfDevNum, 2, &l4TypeInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChNetIfAppSpecificCpuCodeTcpUdpPortRangeSet");

    /* Min-Max range for both L4 dest and src port */
    l4TypeInfo.minL4Port    = 10;
    l4TypeInfo.maxL4Port    = 15;
    l4TypeInfo.packetType   = CPSS_NET_TCP_UDP_PACKET_UC_E;
    l4TypeInfo.protocol     = CPSS_NET_PROT_TCP_E;
    l4TypeInfo.cpuCode      = CPSS_NET_FIRST_USER_DEFINED_E + 7;
    l4TypeInfo.l4PortMode   = CPSS_NET_CPU_CODE_L4_BOTH_DEST_SRC_PORT_E;

    rc = prvTgfNetIfAppSpecificCpuCodeTcpUdpPortRangeSet(prvTgfDevNum, 3, &l4TypeInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChNetIfAppSpecificCpuCodeTcpUdpPortRangeSet");

}

/**
* @internal prvTgfNetIfAppSpecificCpuCodeTcpUdpPortRangeConfigRestore function
* @endinternal
*
* @brief   Restore configuration
*/
static GT_VOID prvTgfNetIfAppSpecificCpuCodeTcpUdpPortRangeConfigRestore
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC iface;
    GT_STATUS rc;
    GT_U32 index = 0;

    cpssOsMemSet(&iface, 0, sizeof(iface));

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");


    /* AUTODOC: App specific CPU code */
    for(index = 1; index <= 3; index++)
    {
        rc = prvTgfNetIfAppSpecificCpuCodeTcpUdpPortRangeSet(prvTgfDevNum, index, &l4TypeInfoArr[index]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfNetIfAppSpecificCpuCodeTcpUdpPortRangeSet");
    }

    /* AUTODOC: clear FDB. FDB entries could be created implicitly. */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush");

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

}

/**
* @internal  prvTgfNetIfAppSpecificCpuCodeTcpUdpPortRangeSendPacket function
* @endinternal
*
* @brief   Check counters on the ports.
*
* @param[in] isExpected               - whether the packet should be found on the tx port.
* @param[in] isSentWithDsa            - whether the packet setn with DSA-tag.
* @param[in] isExpWithDsa             - whether the packet should be received with DSA-tag.
*                                       None
*/
static GT_VOID  prvTgfNetIfAppSpecificCpuCodeTcpUdpPortRangeSendPacket
(
    IN CPSS_NET_RX_CPU_CODE_ENT    expectCpuCode
)
{

    GT_STATUS   rc       = GT_OK;
    GT_U32      portIter = 0;
    GT_U8       devNum;
    GT_U8       queue;
    GT_U8       packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32      buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_STATUS   expectedRcRxToCpu = GT_OK;
    GT_BOOL     getFirst = GT_TRUE;
    GT_U32      packetActualLength = 0;
    TGF_NET_DSA_STC rxParam;
    GT_BOOL packetToCpu = GT_TRUE;

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfIngressPacketInfo,
                             prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d",
                                 prvTgfDevNum);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* Start transmitting */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                                   prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);

    /* AUTODOC: Check ethernet counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* AUTODOC: verify traffic to and from ingress port */
        rc = prvTgfEthCountersCheck(
            prvTgfDevNum,
            prvTgfPortsArray[portIter],
            /*Rx*/((PRV_TGF_INGRESS_PORT_IDX_CNS  == portIter)  ? 1 : 0),
            /*Tx*/((PRV_TGF_INGRESS_PORT_IDX_CNS  == portIter)  ? 1 : 0),
            prvTgfIngressPacketInfo.totalLen,
            prvTgfBurstCount);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfEthCountersCheck: %d, %d\n",
            prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    if ( GT_FALSE == packetToCpu )
    {
        expectedRcRxToCpu = GT_NO_MORE;
    }

    /* stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* get first entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       getFirst, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(expectedRcRxToCpu, rc,
                                 "tgfTrafficGeneratorRxInCpuGet");

    if(packetToCpu)
    {
        PRV_UTF_LOG1_MAC("CPU Code %d\n", rxParam.cpuCode);
        /* check CPU code */
        UTF_VERIFY_EQUAL0_STRING_MAC(expectCpuCode, rxParam.cpuCode, "Wrong CPU Code");
    }

    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_FALSE, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet");
}


/**
* @internal  prvTgfNetIfAppSpecificCpuCodeTcpUdpPortRangeTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
static GT_VOID  prvTgfNetIfAppSpecificCpuCodeTcpUdpPortRangeTrafficGenerate
(
    GT_VOID
)
{
    GT_U32                      i;
    CPSS_NET_RX_CPU_CODE_ENT   *expectCpuCodeArr; /* pointer to array with expected CPU codes */

    if(PRV_CPSS_SIP_6_20_CHECK_MAC(prvTgfDevNum))
    {
        expectCpuCodeArr = expectedCpuCodeSip6_20Arr;
    }
    else if(PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum))
    {
        expectCpuCodeArr = expectedCpuCodeSip6_10Arr;
    }
    else
    {
        expectCpuCodeArr = expectedCpuCodeArr;
    }

    /* AUTODOC: setup a packet with  L4 destination and source port */
    for (i = 0; i < 11; i++)
    {
        PRV_UTF_LOG0_MAC(prvTgfIterationNameArray[i]);
        prvTgfPacketTcpPart.dstPort =  prvTgfL4DestSrcPortInfoArray[i][0];
        prvTgfPacketTcpPart.srcPort =  prvTgfL4DestSrcPortInfoArray[i][1];
        prvTgfNetIfAppSpecificCpuCodeTcpUdpPortRangeSendPacket(expectCpuCodeArr[i]);
    }
}

/************************** PUBLIC DECLARATIONS *******************************/
/**
* @internal prvTgfNetIfAppSpecificCpuCodeTcpUdpPortRangeTest function
* @endinternal
*
* @brief check the CPU code assigned based on L4 dest/src port
*/
GT_VOID prvTgfNetIfAppSpecificCpuCodeTcpUdpPortRangeTest
(
    GT_VOID
)
{
    prvTgfNetIfAppSpecificCpuCodeTcpUdpPortRangeConfigSet();
    prvTgfNetIfAppSpecificCpuCodeTcpUdpPortRangeTrafficGenerate();
    prvTgfNetIfAppSpecificCpuCodeTcpUdpPortRangeConfigRestore();
}
