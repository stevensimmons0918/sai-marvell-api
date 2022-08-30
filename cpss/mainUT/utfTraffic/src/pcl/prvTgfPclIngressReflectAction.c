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
* @file prvTgfPclIngressReflectAction.c
*
* @brief Ingress reflect action test
*
* @version 1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <pcl/prvTgfPclIngressReflectAction.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* PCL rule & action index */
#define IPCL_MATCH_INDEX_0_CNS     0

/* Ports array index */
#define PRV_TGF_PORTS_ARRAY_IDX_CNS  0

/* default number of packets to send */
static GT_U32   prvTgfBurstCount = 10;

/************************* General packet's parts *****************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x13, 0x14, 0x15},                /* daMac */
    {0x00, 0x00, 0x00, 0x03, 0x04, 0x05}                 /* saMac */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfIpv4PacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part =
{
    4,                                 /* version */
    5,                                 /* headerLen */
    0,                                 /* typeOfService */
    0x114,                             /* totalLen */
    0,                                 /* id */
    0,                                 /* flags */
    0,                                 /* offset */
    0xC0,                              /* timeToLive */
    0x04,                              /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    { 1,  1,  1,  1},                  /* srcAddr */
    { 1,  1,  1,  3}                   /* dstAddr */
};

/* Data of packet */
static GT_U8 prvTgfPacketPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPacketPayloadDataArr),         /* dataLength */
    prvTgfPacketPayloadDataArr                  /* dataPtr */
};

/* PARTS of IPV4 packet */
static TGF_PACKET_PART_STC prvTgfIpv4PacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfIpv4PacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* LENGTH of IPv4 packet */
#define PRV_TGF_IPV4_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS +  \
    TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)

/* IPv4 PACKET to send */
static TGF_PACKET_STC prvTgfIpv4PacketInfo =
{
    PRV_TGF_IPV4_PACKET_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfIpv4PacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfIpv4PacketPartArray                                        /* partsArray */
};

/******************************************************************************/

/* RESTORE VALUE */
static GT_U32 origPortsArray;

/*******************************************************************************/
/**
* @internal prvTgfPclIngressReflectActionRulesAndActionSet function
* @endinternal
*
* @brief   Set PCL rules and actions
*/
static GT_VOID prvTgfPclIngressReflectActionRulesAndActionSet
(
    IN GT_PHYSICAL_PORT_NUM portNum
)
{
    GT_STATUS rc;
    PRV_TGF_PCL_RULE_FORMAT_UNT         mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT         pattern;
    PRV_TGF_PCL_ACTION_STC              action;

    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    action.bypassBridge = GT_TRUE;
    action.redirect.redirectCmd = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    action.redirect.data.outIf.outInterface.type = CPSS_INTERFACE_PORT_E;
    action.redirect.data.outIf.outInterface.devPort.hwDevNum = prvTgfDevNum;
    action.redirect.data.outIf.outInterface.devPort.portNum = portNum;

    /* AUTODOC: IPCL Mask for DIP */
    cpssOsMemSet(&mask.ruleStdIpv4L4.dip, 0xFF, sizeof(GT_IPADDR));

    /* AUTODOC: IPCL Pattern for DIP */
    cpssOsMemCpy(pattern.ruleStdIpv4L4.dip.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(GT_IPADDR));

    rc = prvTgfPclRuleSet(
                PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
                IPCL_MATCH_INDEX_0_CNS, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                 PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E,
                                 IPCL_MATCH_INDEX_0_CNS);
}

/**
* @internal prvTgfPclIngressReflectActionConfigurationSet function
* @endinternal
*
* @brief   initial configurations
*/
GT_VOID prvTgfPclIngressReflectActionConfigurationSet
(
     IN GT_PHYSICAL_PORT_NUM portNum
)
{
    GT_STATUS   rc;

    /* Save the value of prvTgfPortsArray[PRV_TGF_PORTS_ARRAY_IDX_CNS] for restoring */
    origPortsArray = prvTgfPortsArray[PRV_TGF_PORTS_ARRAY_IDX_CNS];

    /* Assign prvTgfPortsArray[PRV_TGF_PORTS_ARRAY_IDX_CNS] */
    prvTgfPortsArray[PRV_TGF_PORTS_ARRAY_IDX_CNS] = portNum;

    /* AUTODOC: SETUP CONFIGURATION: */
    /* AUTODOC: Init IPCL Engine for portNum */
    rc = prvTgfPclDefPortInit(
            prvTgfPortsArray[PRV_TGF_PORTS_ARRAY_IDX_CNS],
            CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E  /*nonIpKey*/,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E  /*ipv4Key*/,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E  /*ipv6Key*/);
    UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfPclDefPortInit: %d, %d, %d",
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_PORTS_ARRAY_IDX_CNS],
            CPSS_PCL_DIRECTION_INGRESS_E);

    /* Set PCL rules and actions */
    prvTgfPclIngressReflectActionRulesAndActionSet(prvTgfPortsArray[PRV_TGF_PORTS_ARRAY_IDX_CNS]);
}

/**
* @internal prvTgfPclIngressReflectActionTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Test generate traffic and check results
*/
GT_VOID prvTgfPclIngressReflectActionTrafficGenerateAndCheck
(
   IN GT_PHYSICAL_PORT_NUM portNum
)
{

    GT_STATUS  rc;
    GT_U32     portIter;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;

    /* Set prvTgfPortsArray[PRV_TGF_PORTS_ARRAY_IDX_CNS] value */
    prvTgfPortsArray[PRV_TGF_PORTS_ARRAY_IDX_CNS] = portNum;

    /* reset counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* Setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfIpv4PacketInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, 0);

    /* Send packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_PORTS_ARRAY_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_PORTS_ARRAY_IDX_CNS]);

    /* Do some waiting */
    cpssOsTimerWkAfter(100);

    /* AUTODOC: Check ethernet counters */
    for (portIter = 0; portIter < PRV_TGF_PORTS_NUM_CNS; portIter++)
    {
        /* read counters with print enabled */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum,
                                       prvTgfPortsArray[portIter],
                                       GT_TRUE, &portCntrs);

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                     "ERROR of prvTgfReadPortCountersEth Port=%02d\n",
                                     prvTgfPortsArray[portIter]);

        /* check ETH counters */
        if (portIter == PRV_TGF_PORTS_ARRAY_IDX_CNS)
        {
            /* Possibly storming of packets */
            rc = (portCntrs.goodPktsRcv.l[0] >= 2 * prvTgfBurstCount) ? GT_OK : GT_ERROR;
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "got another goodPktsRcv counter than expected");

            rc = (portCntrs.goodPktsSent.l[0] >= 2 * prvTgfBurstCount) ? GT_OK : GT_ERROR;
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "got another goodPktsSent counter than expected");

            rc = (portCntrs.goodOctetsRcv.l[0] >=
                      (2 * (PRV_TGF_IPV4_PACKET_LEN_CNS + TGF_CRC_LEN_CNS) * prvTgfBurstCount)) ? GT_OK : GT_ERROR;
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "got another goodOctetsRcv counter than expected");

            rc = (portCntrs.goodOctetsSent.l[0] >=
                      (2 * (PRV_TGF_IPV4_PACKET_LEN_CNS + TGF_CRC_LEN_CNS) * prvTgfBurstCount)) ? GT_OK : GT_ERROR;
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "got another goodOctetsSent counter than expected");

        }
        else
        {
            /* Check ETH counters in common case */
            rc = (portCntrs.goodPktsRcv.l[0] == 0) ? GT_OK : GT_ERROR;
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "got another goodPktsRcv counter than expected");

            rc = (portCntrs.goodPktsSent.l[0] == 0) ? GT_OK : GT_ERROR;
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "got another goodPktsSent counter than expected");

            rc = (portCntrs.goodOctetsRcv.l[0] == 0) ? GT_OK : GT_ERROR;
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "got another goodOctetsRcv counter than expected");

            rc = (portCntrs.goodOctetsSent.l[0] == 0) ? GT_OK : GT_ERROR;
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "got another goodOctetsSent counter than expected");
        }

    }
}

/**
* @internal prvTgfPclIngressReflectActionRestore function
* @endinternal
*
* @brief   Test restore configurations
*/
GT_VOID prvTgfPclIngressReflectActionRestore
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: invalidate PCL rule */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     IPCL_MATCH_INDEX_0_CNS,
                                     GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_STD_E,
                                 IPCL_MATCH_INDEX_0_CNS,
                                 GT_FALSE);

    /* AUTODOC: Clear IPCL Engine configuration tables */
    prvTgfPclRestore();

    /* AUTODOC: Disables PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);

    /* Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* Restore prvTgfPortsArray[PRV_TGF_PORTS_ARRAY_IDX_CNS] value */
    prvTgfPortsArray[PRV_TGF_PORTS_ARRAY_IDX_CNS] = origPortsArray;
}

