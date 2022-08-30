
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
* @file prvTgfBrgUdpRelay.c
*
* @brief UDP Broadcast Mirror/Trap (UDP Relay) test
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <bridge/prvTgfBrgUdpRelay.h>


/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_VLANID_CNS                5

/* port to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         0

/* default number of packets to send */
static GT_U32        prvTgfBurstCount =   1;

/* index of UDP dest port */
#define PRV_TGF_UDP_DEST_PORT_IDX_CNS     1


/****************************** IpV6 packet ************************************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},   /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}    /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS            /* pri, cfi, VlanId */
};

/* Packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePartIpv6 =
{
    TGF_ETHERTYPE_86DD_IPV6_TAG_CNS
};

/* packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketIpv6Part =
{
    6,                                      /* version */
    0,                                      /* trafficClass */
    0,                                      /* flowLabel */
    0x02,                                   /* payloadLen */
    TGF_PROTOCOL_UDP_E,                     /* nextHeader */
    0x40,                                   /* hopLimit */
    {0x6545, 0, 0, 0, 0, 0, 0x0000, 0x3212},/* TGF_IPV6_ADDR srcAddr */
    {0x1122, 0, 0, 0, 0, 0, 0xCCDD, 0xEEFF} /* TGF_IPV6_ADDR dstAddr */
};

/* packet's UDP part */
static TGF_PACKET_UDP_STC prvTgfPacketUdpPart =
{
    8,                  /* src port */
    0,                  /* dst port - will be modified on runtime */
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,  /* length */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS /* csum */
};

/* DATA of packet */
static GT_U8 prvTgfPacketPayloadDataArr[] = {
    0xfa, 0xb5, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPacketPayloadDataArr),                 /* dataLength */
    prvTgfPacketPayloadDataArr                          /* dataPtr */
};

/* PARTS of IpV6 packet */
static TGF_PACKET_PART_STC prvTgfPacketBcIpv6UdpPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part}, /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePartIpv6},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6Part},
    {TGF_PACKET_PART_UDP_E,       &prvTgfPacketUdpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_BRD_IPV6_UDP_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_IPV6_HEADER_SIZE_CNS + TGF_UDP_HEADER_SIZE_CNS \
    + sizeof(prvTgfPacketPayloadDataArr)

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketBcIpv6UdpInfo = {
    PRV_TGF_PACKET_BRD_IPV6_UDP_LEN_CNS,                                    /* totalLen */
    sizeof(prvTgfPacketBcIpv6UdpPartArray)/sizeof(TGF_PACKET_PART_STC),/* numOfParts*/
    prvTgfPacketBcIpv6UdpPartArray                                     /* partsArray*/
};


/****************************** IpV4 packet ************************************/
/* ethertype part IpV4 packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv4EtherTypePart = {
    TGF_ETHERTYPE_0800_IPV4_TAG_CNS
};
/* packet's IpV4 header */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                  /* version */
    5,                  /* IHL: Internet Header Length;  Length of entire IP header. */
    0,                  /* typeOfService */
    0x2A,               /*  Length of entire IP Packet (including IP header and IP Payload).*/
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0,               /* timeToLive */
    17,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,            /* check sum */
    {10, 10, 10, 10},   /* srcAddr */
    { 4,  4,  4,  4}    /* dstAddr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketIpv4PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part}, /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_UDP_E,       &prvTgfPacketUdpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of packet IPV4 */
#define PRV_TGF_PACKET_IPV4_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS +TGF_ETHERTYPE_SIZE_CNS \
    + TGF_IPV4_HEADER_SIZE_CNS  + TGF_UDP_HEADER_SIZE_CNS + \
    sizeof(prvTgfPacketPayloadDataArr)

/* PACKET to send */
static TGF_PACKET_STC prvTgfPacketIpv4Info = {
    PRV_TGF_PACKET_IPV4_LEN_CNS,                                  /* totalLen */
    sizeof(prvTgfPacketIpv4PartArray)/sizeof(TGF_PACKET_PART_STC),/* numOfParts*/
    prvTgfPacketIpv4PartArray                                     /* partsArray*/
};

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/* default Vlan ID and brige parameters */
static GT_U16                   prvTgfDefVlanId = 0;
static GT_BOOL                  origValid;
static GT_U16                   origUdpPortNum;
static CPSS_NET_RX_CPU_CODE_ENT origCpuCode;
static CPSS_PACKET_CMD_ENT      origPacketCmd;
static GT_BOOL                  origMirrorTrapEnable;

static GT_U32 entryIndex = 0;
static CPSS_PACKET_CMD_ENT packetCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
static CPSS_NET_RX_CPU_CODE_ENT cpuCode = CPSS_NET_UDP_BC_MIRROR_TRAP0_E;

/**
* @internal prvTgfUdpRelayBrgConfigurationSet function
* @endinternal
*
* @brief   Set and save test bridge configuration
*/
GT_VOID prvTgfUdpRelayBrgConfigurationSet
(
    IN GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U8       tagArray[] = {0,0,0,0};

    /* AUTODOC: SETUP CONFIGURATION: */

    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 &prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* set vlanId */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfBrgVlanPortVidSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: create VLAN 5 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_CNS, prvTgfPortsArray,
                                           NULL, tagArray, prvTgfPortsNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgDefVlanEntryWithPortsSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: save UDP Broadcast Destination Port configuration */
    rc = cpssDxChBrgGenUdpBcDestPortCfgGet(prvTgfDevNum, entryIndex, &origValid, &origUdpPortNum, &origCpuCode, &origPacketCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgGenUdpBcDestPortCfgGet: %d");
}

/**
* @internal prvTgfUdpRelayBrgEnableTrap function
* @endinternal
*
* @brief   Set test bridge configuration to trap packet
*/
GT_VOID prvTgfUdpRelayBrgEnableTrap
(
    IN GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: set UDP Broadcast Destination Port configuration */
    rc = cpssDxChBrgGenUdpBcDestPortCfgSet(prvTgfDevNum, entryIndex, prvTgfPortsArray[PRV_TGF_UDP_DEST_PORT_IDX_CNS], cpuCode, packetCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgGenUdpBcDestPortCfgSet: %d");

    /* AUTODOC: enable Broadcast UDP packets Mirror/Trap */
    rc = cpssDxChBrgVlanUdpBcPktsToCpuEnable(prvTgfDevNum, PRV_TGF_VLANID_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanUdpBcPktsToCpuEnable: %d");
}

/**
* @internal prvTgfUdpRelayTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] floodOrDrop              - GT_TRUE - expected flooding
*                                      GT_FALSE - expected drop
*
*/
GT_VOID prvTgfUdpRelayTrafficGenerate
(
    IN GT_BOOL  floodOrDrop
)
{
    GT_STATUS                       rc       = GT_OK;
    GT_U32                          portIter = 0;

    GT_BOOL     getFirst = GT_TRUE;
    GT_U8       packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32      buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32      packetActualLength = 0;
    GT_U8       queue;
    TGF_NET_DSA_STC rxParam;

    GT_STATUS   expectedRcRxToCpu = GT_OK;

    /* AUTODOC: GENERATE TRAFFIC: */

    /* AUTODOC:   send BC packet with: */
    /* AUTODOC:   DA=FF:FF:FF:FF:FF:FF, SA=00:00:00:00:00:11 */

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    prvTgfPacketUdpPart.dstPort = prvTgfPortsArray[PRV_TGF_UDP_DEST_PORT_IDX_CNS];

    /* setup ipv6 packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketBcIpv6UdpInfo,
                             prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d",
                                 prvTgfDevNum);

    /* start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* send ipv6 packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                                   prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

     /* read and check ethernet counters - all IPv6 packets should be forwared */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* AUTODOC: verify traffic is dropped */
        rc = prvTgfEthCountersCheck(
            prvTgfDevNum,
            prvTgfPortsArray[portIter],
            /*Rx*/((PRV_TGF_SEND_PORT_IDX_CNS  == portIter)  ? 1 : 0),
            /*Tx*/ 1,
            prvTgfPacketBcIpv6UdpInfo.totalLen - ((PRV_TGF_SEND_PORT_IDX_CNS  == portIter) ? 0 : TGF_CRC_LEN_CNS),
            prvTgfBurstCount);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfEthCountersCheck: %d, %d\n",
            prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* setup ipv4 packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketIpv4Info,
                             prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d",
                                 prvTgfDevNum);

    /* send ipv4 packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                                   prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* read and check ethernet counters - IPv4 packets should be discarded after configuring UDP relay */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* AUTODOC: verify traffic is dropped */
        rc = prvTgfEthCountersCheck(
            prvTgfDevNum,
            prvTgfPortsArray[portIter],
            /*Rx*/((PRV_TGF_SEND_PORT_IDX_CNS  == portIter)  ? 1 : 0),
            /*Tx*/((PRV_TGF_SEND_PORT_IDX_CNS  == portIter)  ? 1 : ((floodOrDrop == GT_TRUE) ? 1 : 0)),
            prvTgfPacketIpv4Info.totalLen - ((PRV_TGF_SEND_PORT_IDX_CNS  == portIter) ? 0 : TGF_CRC_LEN_CNS),
            prvTgfBurstCount);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfEthCountersCheck: %d, %d\n",
            prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* check trapped packets */
    if ( GT_TRUE == floodOrDrop )
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
                                       &prvTgfDevNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(expectedRcRxToCpu, rc,
                                 "tgfTrafficGeneratorRxInCpuGet");

    if (floodOrDrop == GT_FALSE)
    {
        PRV_UTF_LOG1_MAC("CPU Code %d\n", rxParam.cpuCode);
        /* check CPU code */
        UTF_VERIFY_EQUAL0_STRING_MAC(cpuCode, rxParam.cpuCode, "Wrong CPU Code");
    }

    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_FALSE, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &prvTgfDevNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet");
}

/**
* @internal prvTgfUdpRelayConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfUdpRelayConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: RESTORE CONFIGURATION: */
    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
                                 prvTgfDevNum, GT_TRUE);

    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d",
                                 prvTgfDevNum, prvTgfDefVlanId);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* AUTODOC: restore UDP Broadcast Destination Port configuration */
    rc = cpssDxChBrgGenUdpBcDestPortCfgSet(prvTgfDevNum, entryIndex, origUdpPortNum, origCpuCode, origPacketCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgGenUdpBcDestPortCfgSet: %d");

    /* AUTODOC: restore Broadcast UDP packets Mirror/Trap */
    rc = cpssDxChBrgVlanUdpBcPktsToCpuEnable(prvTgfDevNum, PRV_TGF_VLANID_CNS, origMirrorTrapEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanUdpBcPktsToCpuEnable: %d");

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

/**
* @internal prvTgfUdpRelayTest function
* @endinternal
*
* @brief   UDP Broadcast Mirror/Trap (UDP Relay) test
*/
GT_VOID prvTgfUdpRelayTest
(
    GT_VOID
)
{
    /* Set Bridge configuration */
    prvTgfUdpRelayBrgConfigurationSet();

    /* Generate traffic */
    prvTgfUdpRelayTrafficGenerate(GT_TRUE);

    /* Set test bridge configuration to trap broadcast IpV4 UDP packets */
    prvTgfUdpRelayBrgEnableTrap();

    /* Generate traffic */
    prvTgfUdpRelayTrafficGenerate(GT_FALSE);

    /* Restore configuration */
    prvTgfUdpRelayConfigurationRestore();
}


