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
* @file prvTgfCfgIngressDropEnable.c
*
* @brief Test ingress drop status
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfConfigGen.h>
#include <common/tgfPclGen.h>
#include <common/tgfCncGen.h>
#include <cnc/prvTgfCncVlanL2L3.h>
#include <common/tgfTcamGen.h>
#include <config/prvTgfCfgIngressDropEnable.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* port to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS   0

/* port number to receive traffic from */
#define PRV_TGF_RECV_PORT_IDX_CNS   1

/* PCL rule & action index */
#define IPCL_MATCH_INDEX_0_CNS     17
#define EPCL_MATCH_INDEX_CNS       prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(18)

/* default number of packets to send */
static GT_U32                       prvTgfBurstCount = 1;
static CPSS_INTERFACE_INFO_STC      recvPortInterface;
static CPSS_INTERFACE_INFO_STC      sendPortInterface;
static GT_U32                       prvTgfCncBlockNumArr[] = {0, 1};

/************************* General packet's parts *****************************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x00, 0x00, 0x33, 0x44, 0x55},                /* daMac */
    {0x00, 0x00, 0x00, 0x03, 0x04, 0x05}                 /* saMac */
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

/* Payload part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPacketPayloadDataArr), /* dataLength */
    prvTgfPacketPayloadDataArr          /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* DB to restore values */
static GT_BOOL  restore_dropEnable;

/******************************************************************************/

/*******************************************************************************/

/**
* @internal prvTgfCfgIngressDropEnableTestPclRulesAndActionsSet function
* @endinternal
*
* @brief   Set PCL rules and actions
*/
static GT_VOID prvTgfCfgIngressDropEnableTestPclRulesAndActionsSet
(
    GT_VOID
)
{
    GT_STATUS rc;
    PRV_TGF_PCL_RULE_FORMAT_UNT         mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT         pattern;
    PRV_TGF_PCL_ACTION_STC              action;

    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    /* IPCL action */
    action.bypassBridge = GT_TRUE;
    action.pktCmd = CPSS_PACKET_CMD_DROP_HARD_E;
    action.mirror.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E;

    /* AUTODOC: IPCL Mask for MAC SA */
    cpssOsMemSet(mask.ruleStdNotIp.macSa.arEther, 0xFF, sizeof(GT_ETHERADDR));

    /* AUTODOC: IPCL Pattern for MAC SA */
    cpssOsMemCpy(pattern.ruleStdNotIp.macSa.arEther,
                 prvTgfPacketL2Part.saMac,
                 sizeof(GT_ETHERADDR));

    rc = prvTgfPclRuleSet(
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
            IPCL_MATCH_INDEX_0_CNS, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                 PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                                 IPCL_MATCH_INDEX_0_CNS);

    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    /* AUTODOC: EPCL Mask for MAC SA */
    cpssOsMemSet(mask.ruleEgrStdNotIp.macSa.arEther, 0xFF, sizeof(GT_ETHERADDR));

    /* AUTODOC: EPCL Pattern for MAC SA */
    cpssOsMemCpy(pattern.ruleEgrStdNotIp.macSa.arEther,
                 prvTgfPacketL2Part.saMac,
                 sizeof(GT_ETHERADDR));

    /* AUTODOC: EPCL action - forward the packet */
    action.egressPolicy = GT_TRUE;
    action.matchCounter.enableMatchCount = GT_TRUE;
    action.matchCounter.matchCounterIndex = 1;

    rc = prvTgfPclRuleSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E,
            EPCL_MATCH_INDEX_CNS, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                 PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E,
                                 EPCL_MATCH_INDEX_CNS);
}
/**
* @internal prvTgfCfgIngressDropEnableTestConfigurationSet function
* @endinternal
*
* @brief   initial configurations
*/
GT_VOID prvTgfCfgIngressDropEnableTestConfigurationSet
(
     IN GT_BOOL  pktDropEnable,
     IN GT_BOOL  epclTriggerEnable
)
{
    GT_STATUS                              rc;
    GT_U32                                 indexRangesBmp[2];
    PRV_TGF_CNC_COUNTER_STC                counter;
    GT_U32                                 blockSize;
    GT_U32                                 index = 0;
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT dsaCpuCode;

    /* AUTODOC: SETUP CONFIGURATION: */
    /* AUTODOC: Init IPCL Engine for port 0 */
    rc = prvTgfPclDefPortInit(
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_NUMBER_0_E,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E     /*nonIpKey*/,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
            PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E  /*ipv6Key*/);
    UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfPclDefPortInit: %d, %d, %d",
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_INGRESS_E);

    /* AUTODOC: Init EPCL Engine for port 62
                For packets not dropped by ingress processing engine,
                EPCL is enable on default port(62) */
    rc = prvTgfPclDefPortInit(
        CPSS_NULL_PORT_NUM_CNS,
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_NUMBER_0_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E  /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL3_STRING_MAC(
            GT_OK, rc, "prvTgfPclDefPortInit: %d, %d, %d",
            prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_EGRESS_E);

    prvTgfCfgIngressDropEnableTestPclRulesAndActionsSet();

    /* set port for packet send */
    sendPortInterface.type             = CPSS_INTERFACE_PORT_E;
    sendPortInterface.devPort.hwDevNum = prvTgfDevNum;
    sendPortInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];

    /* set port for packet capture */
    recvPortInterface.type             = CPSS_INTERFACE_PORT_E;
    recvPortInterface.devPort.hwDevNum = prvTgfDevNum;
    recvPortInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX_CNS];

    /* Save drop in EQ enable */
    rc = prvTgfCfgIngressDropEnableGet(prvTgfDevNum, &restore_dropEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCfgIngressDropEnableGet: %d", prvTgfDevNum);

    /* Configure drop in EQ enable */
    rc = prvTgfCfgIngressDropEnableSet(prvTgfDevNum, pktDropEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCfgIngressDropEnableSet: %d", prvTgfDevNum);

    /* Configure trigger EPCL lookup enable */
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum, CPSS_NULL_PORT_NUM_CNS, PRV_TGF_PCL_EGRESS_PKT_DROP_E, epclTriggerEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclEgressPclPacketTypesSet: %d", prvTgfDevNum);

    /* Bind CNC block to client[PRV_TGF_CNC_CLIENT_EGRESS_PACKET_TYPE_PASS_DROP_E] */
    counter.packetCount.l[0] = 0;
    counter.packetCount.l[1] = 0;

    blockSize = prvTgfCncFineTuningBlockSizeGet();

    rc = prvCpssDxChNetIfCpuToDsaCode(CPSS_NET_TARGET_NULL_PORT_E, &dsaCpuCode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssDxChNetIfCpuToDsaCode");

    index = ((CPSS_PACKET_CMD_DROP_HARD_E << 8) | (dsaCpuCode & 0xff));
    indexRangesBmp[0] = (1 << (index / blockSize));
    indexRangesBmp[1] = 0;

    rc = prvTgfCncTestCncBlockConfigure(
         prvTgfCncBlockNumArr[0],
         PRV_TGF_CNC_CLIENT_EGRESS_PACKET_TYPE_PASS_DROP_E,
         GT_TRUE /*enable*/,
         indexRangesBmp,
         PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"prvTgfCncTestCncBlockConfigure");

    /* Bind CNC block to client[PRV_TGF_CNC_CLIENT_EGRESS_PCL_E] */
    index = 1;
    indexRangesBmp[0] = (1 << (index / blockSize));
    indexRangesBmp[1] = 0;

    rc = prvTgfCncTestCncBlockConfigure(
         prvTgfCncBlockNumArr[1],
         PRV_TGF_CNC_CLIENT_EGRESS_PCL_E,
         GT_TRUE /*enable*/,
         indexRangesBmp,
         PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"prvTgfCncTestCncBlockConfigure");

    /* AUTODOC: enable clear by read mode of CNC counters read operation */
    rc = prvTgfCncCounterClearByReadEnableSet(
        GT_TRUE /*enable*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCounterClearByReadEnableSet");

    /* AUTODOC: for FORMAT_MODE_0 set counter clear value 0 */
    rc = prvTgfCncCounterClearByReadValueSet(
        PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCounterClearByReadValueSet");
}

/**
* @internal prvTgfCfgIngressDropEnableTestConfigurationRestore function
* @endinternal
*
* @brief   Test restore configurations
*/
GT_VOID prvTgfCfgIngressDropEnableTestConfigurationRestore
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32      indexRangesBmp[2];
    CPSS_INTERFACE_INFO_STC          interfaceInfo;
    PRV_TGF_PCL_LOOKUP_CFG_STC       lookupCfg;

    /* AUTODOC: RESTORE CONFIGURATION: */
    /* AUTODOC: invalidate PCL rule */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     IPCL_MATCH_INDEX_0_CNS,
                                     GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_STD_E,
                                 IPCL_MATCH_INDEX_0_CNS,
                                 GT_FALSE);

    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E,
                                     EPCL_MATCH_INDEX_CNS,
                                     GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_STD_E,
                                 EPCL_MATCH_INDEX_CNS,
                                 GT_FALSE);

    /* AUTODOC: Clear IPCL Engine configuration tables */
    prvTgfPclRestore();

    /* AUTODOC: Disables PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);

    /* AUTODOC: Disables PCL egress Policy */
    rc = prvTgfPclEgressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclEgressPolicyEnable: %d", GT_FALSE);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    indexRangesBmp[0] = 0;
    indexRangesBmp[1] = 0;

    rc = prvTgfCncTestCncBlockConfigure(
         prvTgfCncBlockNumArr[0],
         PRV_TGF_CNC_CLIENT_EGRESS_PACKET_TYPE_PASS_DROP_E,
         GT_FALSE /*enable*/,
         indexRangesBmp,
         PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"prvTgfCncTestCncBlockConfigure");

    rc = prvTgfCncTestCncBlockConfigure(
         prvTgfCncBlockNumArr[1],
         PRV_TGF_CNC_CLIENT_EGRESS_PCL_E,
         GT_FALSE /*enable*/,
         indexRangesBmp,
         PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"prvTgfCncTestCncBlockConfigure");

    /* Disable Egress PCL lookup  */
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum, CPSS_NULL_PORT_NUM_CNS, PRV_TGF_PCL_EGRESS_PKT_DROP_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclEgressPclPacketTypesSet: %d", prvTgfDevNum);

    /* Restore drop in EQ enable */
    rc = prvTgfCfgIngressDropEnableSet(prvTgfDevNum, restore_dropEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCfgIngressDropEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: remove PCL configurations */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    interfaceInfo.type               = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum   = prvTgfDevNum;
    interfaceInfo.devPort.portNum    = CPSS_NULL_PORT_NUM_CNS;
    lookupCfg.enableLookup           = GT_FALSE;  /* --> disable the PCL lookup */
    lookupCfg.pclId                  = 0;
    lookupCfg.groupKeyTypes.nonIpKey = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E /*nonIpKey*/;
    lookupCfg.groupKeyTypes.ipv4Key  = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E /*ipv4Key*/;
    lookupCfg.groupKeyTypes.ipv6Key  = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E /*ipv6Key*/;

    rc = prvTgfPclCfgTblSet(
        &interfaceInfo, CPSS_PCL_DIRECTION_EGRESS_E, 0/*dont care*/, &lookupCfg);
    if (GT_OK != rc)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                         "[TGF]: prvTgfPclCfgTblSet FAILED: EGRESS, on NULL port");
    }


}
/**
* @internal prvTgfCfgIngressDropEnableTestPacketSend function
* @endinternal
*
* @brief   Send packet to selected port (by port index).
*
* @param[in] portIndex                - port indexing place in test array ports.
* @param[in] packetInfoPtr            - (pointer to) the packet info
*                                       None
*/
static GT_VOID prvTgfCfgIngressDropEnableTestPacketSend
(
    IN TGF_PACKET_STC   *packetInfoPtr
)
{
    GT_STATUS   rc;
    GT_U32      portIter;
    PRV_TGF_CNC_COUNTER_STC         counter;
    GT_U32                                 index = 0;
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT dsaCpuCode;

    /* reset the ingress drop counter */
    rc = prvTgfCfgIngressDropCntrSet(0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCfgIngressDropCntrSet");

    /* clear counter */
    counter.packetCount.l[0] = 0;
    counter.packetCount.l[1] = 0;

    rc = prvCpssDxChNetIfCpuToDsaCode(CPSS_NET_TARGET_NULL_PORT_E, &dsaCpuCode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssDxChNetIfCpuToDsaCode");

    index = ((CPSS_PACKET_CMD_DROP_HARD_E << 8) | (dsaCpuCode & 0xff));

    rc = prvTgfCncCounterSet(prvTgfCncBlockNumArr[0], index, PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "[TGF]: prvTgfCncCounterSet FAILED, rc = [%d]", rc);

    index = 1;

    rc = prvTgfCncCounterSet(prvTgfCncBlockNumArr[1], index, PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "[TGF]: prvTgfCncCounterSet FAILED, rc = [%d]", rc);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");


    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d, %d",
                                 prvTgfDevNum, prvTgfBurstCount, 0, 0);

    /* Start capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&recvPortInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                           "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 recvPortInterface.devPort.hwDevNum,
                                 recvPortInterface.devPort.portNum,
                                 GT_TRUE);
    /* send packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* wait for packets come to CPU */
    (void) tgfTrafficGeneratorRxInCpuNumWait(prvTgfBurstCount, 500, NULL);

    /* Stop capturing Tx packets */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&recvPortInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
                           "tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d, %d",
                                 recvPortInterface.devPort.hwDevNum,
                                 recvPortInterface.devPort.portNum,
                                 GT_FALSE);
    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n",
                     recvPortInterface.devPort.portNum);
}
/**
* @internal prvTgfCfgIngressDropEnableTestGenerateTraffic function
* @endinternal
*
* @brief   Test generate traffic and check results
*/
GT_VOID prvTgfCfgIngressDropEnableTestGenerateTraffic
(
    IN GT_BOOL pktDrop,
    IN GT_BOOL epclTriggerEnable
)
{
    GT_STATUS                               rc;
    GT_U32                                  partsCount;
    GT_U32                                  packetSize;
    TGF_PACKET_STC                          packetInfo;
    GT_U32                                  ingressCounter;
    PRV_TGF_CNC_COUNTER_STC                 counter;
    GT_U32                                  actualCapturedNumOfPackets;
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT  dsaCpuCode;
    GT_U32                                  index;

    /* AUTODOC: number of parts in packet */
    partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);

    /* AUTODOC: calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketPartArray, partsCount, &packetSize);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet: %d, %d",
                                 partsCount, packetSize);

    /* AUTODOC: build packet info */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = prvTgfPacketPartArray;

    /* AUTODOC: Send packet with MAC SA */
    prvTgfCfgIngressDropEnableTestPacketSend(&packetInfo);

    /* print captured packets and check TriggerCounters */
    rc = tgfTrafficGeneratorPortTxEthCaptureCompare(&recvPortInterface,
                                        &packetInfo,
                                        0,/*numOfPackets*/
                                        0/*vfdNum*/,
                                        NULL /*vfdArray*/,
                                        NULL, /* bytesNum's skip list */
                                        0,    /* length of skip list */
                                        &actualCapturedNumOfPackets,
                                        NULL/*onFirstPacketNumTriggersBmpPtr*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                "tgfTrafficGeneratorPortTxEthCaptureCompare: %d",
                                recvPortInterface.devPort.portNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(0, actualCapturedNumOfPackets,"Expect drop in EREP");

    /* Check packet drop in EQ */
    rc = prvTgfCfgIngressDropCntrGet(&ingressCounter);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCfgIngressDropCntrGet: %d", prvTgfDevNum);

    rc = prvCpssDxChNetIfCpuToDsaCode(CPSS_NET_FIRST_USER_DEFINED_E, &dsaCpuCode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssDxChNetIfCpuToDsaCode");

    index = ((CPSS_PACKET_CMD_DROP_HARD_E << 8) | (dsaCpuCode & 0xff));

    if(pktDrop == GT_FALSE)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ingressCounter, "Expect no drop in ingress pipeline");

        /* AUTODOC: Get cnc counter value for drop packet [client PRV_TGF_CNC_CLIENT_EGRESS_PACKET_TYPE_PASS_DROP_E] */
        rc = prvTgfCncCounterGet(prvTgfCncBlockNumArr[0], index, PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCounterGet");

        UTF_VERIFY_EQUAL0_STRING_MAC(1, counter.packetCount.l[0],"mismatch in egress packet pass/drop counter value");

        /* AUTODOC: Get cnc counter value for drop packet [client PRV_TGF_CNC_CLIENT_EGRESS_PCL_E] */
        index = 1;
        rc = prvTgfCncCounterGet(prvTgfCncBlockNumArr[1], index, PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCounterGet");

        if(epclTriggerEnable == GT_TRUE)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(1, counter.packetCount.l[0],"Mismatch in Egress PCL counter value");
        }
        else
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(0, counter.packetCount.l[0],"Egress PCL Lookup enabled");
        }
    }
    else
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(1, ingressCounter, "Expect drop in ingress pipeline");

        /* AUTODOC: Get cnc counter value for drop packet [client PRV_TGF_CNC_CLIENT_EGRESS_PACKET_TYPE_PASS_DROP_E] */
        rc = prvTgfCncCounterGet(prvTgfCncBlockNumArr[0], index, PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCounterGet");

        UTF_VERIFY_EQUAL0_STRING_MAC(0, counter.packetCount.l[0],
        "mismatch in egress packet pass/drop counter value");
    }
}

/**
* @internal prvTgfCfgIngressDropEnableTest function
* @endinternal
*
* @brief   drop in ingress processing pipe test
*/
GT_VOID prvTgfCfgIngressDropEnableTest
(
    GT_VOID
)
{
    /* Check for drop in EQ enable = GT_FALSE, diable Egress PCL lookup */
    prvTgfCfgIngressDropEnableTestConfigurationSet(GT_FALSE, GT_FALSE);
    prvTgfCfgIngressDropEnableTestGenerateTraffic(GT_FALSE, GT_FALSE);
    prvTgfCfgIngressDropEnableTestConfigurationRestore();

    /* Check for drop in EQ enable = GT_FALSE, enable egress PCL lookup */
    prvTgfCfgIngressDropEnableTestConfigurationSet(GT_FALSE, GT_TRUE);
    prvTgfCfgIngressDropEnableTestGenerateTraffic(GT_FALSE, GT_TRUE);
    prvTgfCfgIngressDropEnableTestConfigurationRestore();

    /* Check for drop in EQ enable = GT_TRUE */
    prvTgfCfgIngressDropEnableTestConfigurationSet(GT_TRUE, GT_FALSE);
    prvTgfCfgIngressDropEnableTestGenerateTraffic(GT_TRUE, GT_FALSE);
    prvTgfCfgIngressDropEnableTestConfigurationRestore();
}
