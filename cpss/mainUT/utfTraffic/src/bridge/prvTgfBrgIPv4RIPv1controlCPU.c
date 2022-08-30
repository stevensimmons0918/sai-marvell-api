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
* @file prvTgfBrgIPv4RIPv1controlCPU.c
*
* @brief IPv4 RIPv1 control CPU configuration test
*
* @version   0
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <bridge/prvTgfBrgIPv4RIPv1controlCPU.h>
#include <cpss/generic/private/prvCpssCommonDefs.h>

/*********************************************/
/* default VLAN Id */
#define PRV_TGF_VLANID_CNS                73

/* port to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         0

/* default number of packets to send */
static GT_U32        prvTgfBurstCount =   1;

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                 /* saMac */
};


/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                           /* pri, cfi, VlanId */
};


/* DATA of packet */
static GT_U8 prvTgfPacket1PayloadDataArr[] = {
    0xfa, 0xb5, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPacket1PayloadDataArr),                 /* dataLength */
    prvTgfPacket1PayloadDataArr                          /* dataPtr */
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


/* packet's UDP part */
static TGF_PACKET_UDP_STC prvTgfPacketUdpPart =
{   111,   /* src port */
    520,   /* dst port */
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,   /* len */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,      /* check sum */
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
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_IPV4_HEADER_SIZE_CNS  + sizeof(prvTgfPacket1PayloadDataArr)

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfPacketIpv4Info = {
    PRV_TGF_PACKET_IPV4_LEN_CNS,                                  /* totalLen */
    sizeof(prvTgfPacketIpv4PartArray)/sizeof(TGF_PACKET_PART_STC),/* numOfParts*/
    prvTgfPacketIpv4PartArray                                     /* partsArray*/
};







/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* LENGTH of buffer*/
#define TGF_RX_BUFFER_SIZE_CNS   0x80


/* stored default VID */
static GT_U16   prvTgfDefVlanId = 0;
/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/
static CPSS_PACKET_CMD_ENT saveIpV4Cmd;

/**
* @internal prvTgfIpV4BrgConfigurationTrafficGenerator function
* @endinternal
*
* @brief   Generate traffic and compare counters
*
* @param[in] packetToCpu              - send packet to cpu?
* @param[in] floodOrDrop              - send packet to ports?
*/
GT_VOID prvTgfIpV4BrgConfigurationTrafficGenerator(
    IN CPSS_PACKET_CMD_ENT cmd

)
{
    GT_STATUS                       rc;
    GT_U32                          portIter = 0;
    GT_BOOL     packetToCpu = GT_TRUE;
    GT_BOOL     floodOrDrop = GT_TRUE;
    GT_BOOL     getFirst = GT_TRUE;
    GT_U8       packetBuff[TGF_RX_BUFFER_SIZE_CNS] = {0};
    GT_U32      buffLen  = TGF_RX_BUFFER_SIZE_CNS;
    GT_U32      packetActualLength = 0;
    GT_U8       devNum;
    GT_U8       queue;
    TGF_NET_DSA_STC rxParam;
    CPSS_NET_RX_CPU_CODE_ENT expectedCpuCode;
    GT_STATUS   expectedRcRxToCpu= GT_OK;
    switch (cmd)
        {
        case CPSS_PACKET_CMD_FORWARD_E:
            packetToCpu = GT_FALSE;
            floodOrDrop = GT_TRUE;
            break;
        case CPSS_PACKET_CMD_MIRROR_TO_CPU_E:
            packetToCpu = GT_TRUE;
            floodOrDrop = GT_TRUE;
            expectedCpuCode= CPSS_NET_MIRROR_IPV4_RIPV1_TO_CPU_E;
            break;
        case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
            packetToCpu = GT_TRUE;
            floodOrDrop = GT_FALSE;
            expectedCpuCode= CPSS_NET_MIRROR_IPV4_RIPV1_TO_CPU_E;
            break;
        case CPSS_PACKET_CMD_DROP_HARD_E:
            packetToCpu = GT_FALSE;
            floodOrDrop = GT_FALSE;
            break;
        case CPSS_PACKET_CMD_DROP_SOFT_E:
            packetToCpu = GT_FALSE;
            floodOrDrop = GT_FALSE;
            break;
        default: break;
        }

    /* AUTODOC: GENERATE TRAFFIC: */
    /* AUTODOC:   send MC packet with: */
    /* AUTODOC:   DA=01:00:0c:xx:xx:xx, SA=00:00:00:00:00:11 */
    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketIpv4Info,
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

    rc = prvTgfStartTransmitingEth(prvTgfDevNum,
                                   prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

   /* read and check ethernet counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* AUTODOC: verify traffic is dropped */
        rc = prvTgfEthCountersCheck(
            prvTgfDevNum,
            prvTgfPortsArray[portIter],
            /*expected Rx*/((PRV_TGF_SEND_PORT_IDX_CNS  == portIter)  ? 1 : 0),
            /*expected Tx*/((PRV_TGF_SEND_PORT_IDX_CNS  == portIter)  ? 1 : ((floodOrDrop == GT_TRUE) ? 1 : 0)),
            prvTgfPacketIpv4Info.totalLen,
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
    if (packetToCpu)
    {
        PRV_UTF_LOG1_MAC("CPU Code %d\n", rxParam.cpuCode);
        /* check CPU code */
        UTF_VERIFY_EQUAL0_STRING_MAC(expectedCpuCode, rxParam.cpuCode, "Wrong CPU Code");
    }
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_FALSE, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc,
                                            "tgfTrafficGeneratorRxInCpuGet %d");
}

/**
* @internal prvTgfIpV4BrgConfigurationSet function
* @endinternal
*
* @brief   Set test bridge configuration
*/
GT_VOID prvTgfIpV4BrgConfigurationSet(
    IN GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    /* AUTODOC: SETUP CONFIGURATION: */
    /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 &prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfBrgVlanPortVidSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: create VLAN 5 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_VLANID_CNS, prvTgfPortsArray,
                                           NULL, NULL, prvTgfPortsNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                               "ERROR of prvTgfBrgDefVlanEntryWithPortsSet: %d",
                                 prvTgfDevNum);


    /* AUTODOC: enable  IP control traffic */
    rc = prvTgfBrgVlanIpCntlToCpuSet(PRV_TGF_VLANID_CNS,
                                     PRV_TGF_BRG_IP_CTRL_IPV4_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanIpCntlToCpuSet: %d");



    /* AUTODOC: Save Proprietary Layer 2 control protocol command */
    rc = cpssDxChBrgGenRipV1CmdGet(prvTgfDevNum,&saveIpV4Cmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgGenArpBcastToCpuCmdGet: %d");
}

/**
* @internal prvTgfIpV4BrgConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration
*/
GT_VOID prvTgfIpV4BrgConfigurationRestore(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    /* AUTODOC: RESTORE CONFIGURATION: */
    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
                                 prvTgfDevNum, GT_TRUE);
    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d",
                                 prvTgfDevNum, prvTgfDefVlanId);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    /* AUTODOC: restore trapping command for Proprietary Layer 2 control MC packets */
    rc = cpssDxChBrgGenRipV1CmdSet(prvTgfDevNum,saveIpV4Cmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "cpssDxChBrgGenRipV1CmdSet: %d");

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

/**
* @internal prvTgfIpV4BrgConfigurationTest function
* @endinternal
*
* @brief   IpV4Rip1 configuration test
*/
GT_VOID prvTgfIpV4BrgConfigurationTest(
    GT_VOID
)
{
    CPSS_PACKET_CMD_ENT cmdlist[] = {CPSS_PACKET_CMD_FORWARD_E,
                                 CPSS_PACKET_CMD_MIRROR_TO_CPU_E,
                                 CPSS_PACKET_CMD_TRAP_TO_CPU_E,
                                 CPSS_PACKET_CMD_DROP_HARD_E,
                                 CPSS_PACKET_CMD_DROP_SOFT_E};
    GT_U32 pcktNum=0;
    for (pcktNum=0 ; pcktNum < sizeof(cmdlist)/sizeof(cmdlist[0]) ; pcktNum++) {
         /*e-Arch devices support all commands, non e-Arch devices don't support Trap/HardDrop/SofDrop*/
        if(PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum) == GT_FALSE && pcktNum > 1) /*stops the test if the device is non e-Arch after testing the first 2 commands*/
        {
            break;
        }
        else
        {
            PRV_UTF_LOG1_MAC("\nCase %d. Command \n", pcktNum);
            /* AUTODOC: set command for Ipv4Rip1 packets */
            cpssDxChBrgGenRipV1CmdSet(prvTgfDevNum, cmdlist[pcktNum]);
            /* Generate traffic */
            prvTgfIpV4BrgConfigurationTrafficGenerator(cmdlist[pcktNum]);
        }
    }
}

