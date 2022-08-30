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
* @file prvTgfBrgExceptionCount.c
*
* @brief Bridged packets exception count check
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTrunkGen.h>
#include <common/tgfIpGen.h>
#include <trunk/prvTgfTrunk.h>

#include <bridge/prvTgfBrgExceptionCount.h>
/*#include <ip/prvTgfBasicIpv4UcRouting.h>*/

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN ID */
#define PRV_TGF_DEF_VLANID_CNS                  1

/* port number to send traffic */
#define PRV_TGF_SEND_PORT_IDX_CNS               0

/* port number to receive traffic */
#define PRV_TGF_RECV_PORT_IDX_CNS               1

/* namber of packets to send */
#define PRV_TGF_SEND_PACKETS_NUM_CNS            4

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = PRV_TGF_SEND_PACKETS_NUM_CNS;

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* srcMac */
};

/* First VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_DEF_VLANID_CNS                        /* pri, cfi, VlanId */
};

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
    0xFF,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    { 2,  2,  2,  4},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* packet's IPv4 - error IPv4 packet*/
static TGF_PACKET_IPV4_STC prvTgfErrorPacketIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2E,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0xFF,               /* protocol */
    0x0001,             /* csum with error */
    { 2,  2,  2,  4},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};



/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS \
    + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* the ARP MAC address to write to the Router ARP Table */
/*static TGF_MAC_ADDR  prvTgfArpMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x22};*/

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_SKIP_E ,      NULL},/*reserved for DSA tag (instead of the vlan tag)*/
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* PARTS of error packet */
static TGF_PACKET_PART_STC prvTgfErrorPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_SKIP_E ,      NULL},/*reserved for DSA tag (instead of the vlan tag)*/
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfErrorPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};


/******************************* Configuration Setup **********************************/


/**
* @internal prvTgfRouterBrgExceptionCountConfigurationSet function
* @endinternal
*
* @brief   Set Configuration to enable special services for bridged traffic
*
*/
GT_VOID prvTgfRouterBrgExceptionCountConfigurationSet
(
    GT_VOID
)
{
    GT_STATUS  rc = GT_OK;

    rc = prvTgfIpBridgeServiceEnable(prvTgfDevNum,
        PRV_TGF_IP_HEADER_CHECK_BRG_SERVICE_E,
        PRV_TGF_IP_BRG_SERVICE_IPV4_ENABLE_DISABLE_E,
        GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "Error in prvTgfIpBridgeServiceEnable");

}

/**
* @internal prvTgfRouterBrgExceptionCountConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfRouterBrgExceptionCountConfigurationRestore
(
    GT_VOID
)
{

    GT_STATUS  rc = GT_OK;

    rc = prvTgfIpBridgeServiceEnable(prvTgfDevNum,
        PRV_TGF_IP_HEADER_CHECK_BRG_SERVICE_E,
        PRV_TGF_IP_BRG_SERVICE_IPV4_ENABLE_DISABLE_E,
        GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "Error in prvTgfIpBridgeServiceEnable");

}


/**
* @internal prvTgfBrgExceptionCountTrafficGenerate function
* @endinternal
*
* @brief   Checked expection count by comparing register value
*/
GT_VOID prvTgfBrgExceptionCountTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;

    /*CPSS_INTERFACE_INFO_STC         portInterface;*/
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    TGF_PACKET_PART_STC             *packetPartsPtr;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          errorPartsCount  = 0;
    GT_U32                          errorPacketSize  = 0;
    TGF_PACKET_PART_STC             *errorPacketPartsPtr;
    TGF_PACKET_STC                  errorPacketInfo;
    GT_U32                          exceptionPacketCount = 0;
    GT_U32                          numVfd = 0;/* number of VFDs in vfdArray */
    TGF_VFD_INFO_STC                vfdArray[1];



    /* reset Exception counters */
    rc = cpssDxChIpRouterBridgedPacketsExceptionCntSet(prvTgfDevNum,0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "Error in cpssDxChIpRouterBridgedPacketsExceptionCntSet");

    /*  Packet setup for normal packet*/
    partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);
    packetPartsPtr = prvTgfPacketPartArray;

    /* calculate packet size */
    rc = prvTgfPacketSizeGet(packetPartsPtr, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* build packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = packetPartsPtr;


    /*  Packet setup for error packet */
    errorPartsCount = sizeof(prvTgfErrorPacketPartArray) / sizeof(prvTgfErrorPacketPartArray[0]);
    errorPacketPartsPtr = prvTgfErrorPacketPartArray;

    /* calculate packet size */
    rc = prvTgfPacketSizeGet(errorPacketPartsPtr, errorPartsCount, &errorPacketSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* build packet */
    errorPacketInfo.totalLen   = errorPacketSize;
    errorPacketInfo.numOfParts = errorPartsCount;
    errorPacketInfo.partsArray = errorPacketPartsPtr;

    /* Transmit setup for normal packet */
     /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, numVfd, &vfdArray[0]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* set transmit timings */
    rc = prvTgfSetTxSetup2Eth(0, 50);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

    /* AUTODOC: send packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:01, VID=5 */
    /* AUTODOC:   srcIP=2.2.2.4 */
    /* AUTODOC:   dstIP=1.1.1.3 */

    /* send packet -- send from specific port */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_CNS);
    cpssOsTimerWkAfter(1000);

    /* Exception counter get */
    rc = cpssDxChIpRouterBridgedPacketsExceptionCntGet(prvTgfDevNum,&exceptionPacketCount);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "Error in cpssDxChIpRouterBridgedPacketsExceptionCntGet");
    PRV_UTF_LOG2_MAC("Exception Count : expectedValue[%d], receivedValue[%d]\n", 0,exceptionPacketCount);
    UTF_VERIFY_EQUAL2_STRING_MAC(0, exceptionPacketCount, "ERROR in exception count: %d, %d\n",
                                 0, exceptionPacketCount);

    /* Transmit setup for error packet */
    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &errorPacketInfo, prvTgfBurstCount, numVfd, &vfdArray[0]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* set transmit timings */
    rc = prvTgfSetTxSetup2Eth(0, 50);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

    /* AUTODOC: send packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:01, VID=5 */
    /* AUTODOC:   srcIP=2.2.2.4 */
    /* AUTODOC:   dstIP=1.1.1.3 */

    /* send packet -- send from specific port */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_CNS);
    cpssOsTimerWkAfter(1000);

    /* Exception counter get */
    rc = cpssDxChIpRouterBridgedPacketsExceptionCntGet(prvTgfDevNum,&exceptionPacketCount);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "Error in cpssDxChIpRouterBridgedPacketsExceptionCntGet");
    PRV_UTF_LOG2_MAC("Exception Count : expectedValue[%d], receivedValue[%d]\n", PRV_TGF_SEND_PACKETS_NUM_CNS,exceptionPacketCount);
    UTF_VERIFY_EQUAL2_STRING_MAC(PRV_TGF_SEND_PACKETS_NUM_CNS, exceptionPacketCount, "ERROR in exception count: %d, %d\n",
                                 PRV_TGF_SEND_PACKETS_NUM_CNS, exceptionPacketCount);

   /* -------------------  Round 2 ------------------------------------- */
   /* Transmit setup for normal packet */
     /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, numVfd, &vfdArray[0]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* set transmit timings */
    rc = prvTgfSetTxSetup2Eth(0, 50);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

    /* AUTODOC: send packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:01, VID=5 */
    /* AUTODOC:   srcIP=2.2.2.4 */
    /* AUTODOC:   dstIP=1.1.1.3 */

    /* send packet -- send from specific port */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_CNS);
    cpssOsTimerWkAfter(1000);

    /* Exception counter get */
    rc = cpssDxChIpRouterBridgedPacketsExceptionCntGet(prvTgfDevNum,&exceptionPacketCount);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "Error in cpssDxChIpRouterBridgedPacketsExceptionCntGet");
    PRV_UTF_LOG2_MAC("Exception Count : expectedValue[%d], receivedValue[%d]\n", PRV_TGF_SEND_PACKETS_NUM_CNS,exceptionPacketCount);
    UTF_VERIFY_EQUAL2_STRING_MAC(PRV_TGF_SEND_PACKETS_NUM_CNS, exceptionPacketCount, "ERROR in exception count: %d, %d\n",
                                 PRV_TGF_SEND_PACKETS_NUM_CNS, exceptionPacketCount);

    /* Transmit setup for error packet */
    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &errorPacketInfo, prvTgfBurstCount, numVfd, &vfdArray[0]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* set transmit timings */
    rc = prvTgfSetTxSetup2Eth(0, 50);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

    /* AUTODOC: send packet from port 0 with: */
    /* AUTODOC:   DA=00:00:00:00:34:02, SA=00:00:00:00:00:01, VID=5 */
    /* AUTODOC:   srcIP=2.2.2.4 */
    /* AUTODOC:   dstIP=1.1.1.3 */

    /* send packet -- send from specific port */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum, PRV_TGF_SEND_PORT_IDX_CNS);
    cpssOsTimerWkAfter(1000);

    /* Exception counter get */
    rc = cpssDxChIpRouterBridgedPacketsExceptionCntGet(prvTgfDevNum,&exceptionPacketCount);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "Error in cpssDxChIpRouterBridgedPacketsExceptionCntGet");
    PRV_UTF_LOG2_MAC("Exception Count : expectedValue[%d], receivedValue[%d]\n", 2*PRV_TGF_SEND_PACKETS_NUM_CNS,exceptionPacketCount);
    UTF_VERIFY_EQUAL2_STRING_MAC(2*PRV_TGF_SEND_PACKETS_NUM_CNS, exceptionPacketCount, "ERROR in exception count: %d, %d\n",
                                 2*PRV_TGF_SEND_PACKETS_NUM_CNS, exceptionPacketCount);



}


