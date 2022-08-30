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
* @file prvTgfBasicTests.c
*
* @brief Basic tests for IPFIX
*
* @version   13
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>
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
#include <common/tgfPolicerGen.h>
#include <common/tgfIpfixGen.h>

#include <ipfix/prvTgfBasicIngressPclKeys.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/dxCh/dxChxGen/policer/cpssDxChPolicer.h>

#define PRV_TGF_TEST_DEFINITIONS

#include <ipfix/prvTgfBasicTests.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>

/* port bitmap VLAN members */
CPSS_PORTS_BMP_STC localPortsVlanMembers = {{0, 0}};

PRV_TGF_TEST_CONFIG_STC basicTestConf[5];

PRV_TGF_POLICER_STAGE_TYPE_ENT testedStage;

PRV_TGF_POLICER_COUNTING_MODE_ENT stagesCountingModeSave[PRV_TGF_POLICER_STAGE_NUM];
GT_BOOL                           stagesMeterEnableSave[PRV_TGF_POLICER_STAGE_NUM];
GT_BOOL                           ipfixAgingEnableRestore[PRV_TGF_POLICER_STAGE_NUM];
static PRV_TGF_POLICER_STAGE_METER_MODE_ENT stagesModeSave[PRV_TGF_POLICER_STAGE_NUM];
static PRV_TGF_POLICER_MEMORY_STC memoryCfgGet;
static GT_U32 eplrIpfixIndex;
static GT_BOOL enableRestore;
static PRV_TGF_IPFIX_FIRST_PACKET_CONFIG_STC firstPktCfgRestore;

/* Rx buffer size */
#define TGF_RX_BUFFER_MAX_SIZE_CNS   0x3FFF

#define PRV_TGF_IPCL_ACTION_FLOW_ID_CNS   0x10
#define PRV_TGF_EPCL_ACTION_FLOW_ID_CNS   0x20
#define PRV_TGF_AGING_START_INDEX_CNS     0x0
#define PRV_TGF_AGING_END_INDEX_CNS       0x1F

/******************************* Test packet **********************************/

/* L2 part of packet */
TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x99},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}                /* srcMac */
};

/* First VLAN_TAG part */
TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};
/* use 933 instead of 1004 to avoid problems in xcat-A1 that total packet length
   was into the issue of :
         add 8 byte padding (another buffer is added, numOfAddedBuffs = 3) should
         be done only if packets length is <byteblock>*n+k (1<=k<=8) (including
         DSA tag), where n and k are integers and <byteblock> is 256 for xCat
         and 512 for DxChLion,
         otherwise (numOfAddedBuffs = 2) and previous flow is preserved.
*/
#define LONG_PAYLOAD_LEN_CNS     933

/* DATA of packet */
GT_U8 prvTgfPayloadDataArr[1004];

/* PAYLOAD part */
TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    0,                       /* dataLength */
    prvTgfPayloadDataArr     /* dataPtr */
};

/* PARTS of packet */
TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* PACKET to send */
TGF_PACKET_STC prvTgfPacketInfo = {
    0,                                                           /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

/**
* @internal prvTgfIpfixStagesParamsSaveAndReset function
* @endinternal
*
* @brief   Save stages counting mode and enable metering state for later restore
*         and set stages to disable counting and metering.
*/
GT_VOID prvTgfIpfixStagesParamsSaveAndReset
(
    GT_VOID
)
{
    GT_U32      i;
    GT_STATUS   rc;

    for( i = 0 ; i < PRV_TGF_POLICER_STAGE_NUM ; i++ )
    {
        if(GT_FALSE ==  prvTgfPolicerStageCheck(i))
        {
            continue ;
        }

        rc = prvTgfPolicerCountingModeGet(prvTgfDevNum, i, &stagesCountingModeSave[i]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingModeGet: %d, %d",
                                     prvTgfDevNum, i);
        rc = prvTgfPolicerMeteringEnableGet(prvTgfDevNum, i, &stagesMeterEnableSave[i]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerMeteringEnableGet: %d, %d",
                                     prvTgfDevNum, i);
        rc = prvTgfPolicerStageMeterModeGet(prvTgfDevNum, i, &stagesModeSave[i]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeGet: %d, %d",
                                     prvTgfDevNum, i);
        rc = prvTgfPolicerCountingModeSet(prvTgfDevNum, i, PRV_TGF_POLICER_COUNTING_DISABLE_E);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingModeSet: %d, %d, %d",
                                     prvTgfDevNum, i, PRV_TGF_POLICER_COUNTING_DISABLE_E);
        rc = prvTgfPolicerMeteringEnableSet(i, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerMeteringEnableSet: %d, %d",
                                     i, GT_FALSE);
        rc = prvTgfPolicerStageMeterModeSet(i, PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeSet: %d, %d",
                                     i, PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E);

        rc = prvTgfIpfixAgingEnableGet(prvTgfDevNum, i, &ipfixAgingEnableRestore[i]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixAgingEnableGet: %d, %d, %d",
                                     prvTgfDevNum, i);

    }
}

/**
* @internal prvTgfIpfixStagesParamsRestore function
* @endinternal
*
* @brief   Restore stages counting mode and enable metering state.
*/
GT_VOID prvTgfIpfixStagesParamsRestore
(
    GT_VOID
)
{
    GT_U32      i;
    GT_STATUS   rc;

    for( i = 0 ; i < PRV_TGF_POLICER_STAGE_NUM ; i++ )
    {
        if(GT_FALSE ==  prvTgfPolicerStageCheck(i))
        {
            continue ;
        }

        rc = prvTgfPolicerCountingModeSet(prvTgfDevNum, i, stagesCountingModeSave[i]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingModeSet: %d, %d",
                                     i, stagesCountingModeSave[i]);
        rc = prvTgfPolicerMeteringEnableSet(i, stagesMeterEnableSave[i]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerMeteringEnableSet: %d, %d",
                                     i, stagesMeterEnableSave[i]);
        rc = prvTgfPolicerStageMeterModeSet(i, stagesModeSave[i]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeSet: %d, %d",
                                     i, stagesModeSave[i]);

        rc = prvTgfIpfixAgingEnableSet(prvTgfDevNum, i, ipfixAgingEnableRestore[i]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixAgingEnableSet: %d, %d, %d",
                                     prvTgfDevNum, i);
    }
}

/**
* @internal prvTgfIpfixIngressTestInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine set Pcl rule.
*/
GT_VOID prvTgfIpfixIngressTestInit
(
    GT_VOID
)
{
    PRV_TGF_PCL_LOOKUP_CFG_STC lookupCfg;
    GT_STATUS               rc = GT_OK;
    GT_U32                  portIter = 0;
    GT_U32                           ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    CPSS_INTERFACE_INFO_STC          interfaceInfo;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;
    GT_U32                           ipfixIndex;
    PRV_TGF_IPFIX_ENTRY_STC          ipfixEntry;
    PRV_TGF_IPFIX_WRAPAROUND_CFG_STC wraparounfConf;

    /* clear entry */
    cpssOsMemSet(&localPortsVlanMembers, 0, sizeof(localPortsVlanMembers));

    /* ports 0, 8, 18, 23 are VLAN Members */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[portIter]);
    }


    /* set VLAN entry */
     prvTgfIpfixVlanTestInit(PRV_TGF_VLANID_CNS, localPortsVlanMembers);

     /* Init PCL. */
    rc = prvTgfPclInit();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"prvTgfPclInit");

    /* Enable PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_TRUE);

    /* enables ingress policy for port 8 */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 GT_TRUE);

    rc = prvTgfPclPortLookupCfgTabAccessModeSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                                CPSS_PCL_DIRECTION_INGRESS_E,
                                                CPSS_PCL_LOOKUP_0_E,
                                                0,
                                                PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet: %d, %d, %d, %d, %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 CPSS_PCL_DIRECTION_INGRESS_E,
                                 CPSS_PCL_LOOKUP_0_E,
                                 0,
                                 PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);

    /* Set PCL configuration table */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    interfaceInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    lookupCfg.enableLookup        = GT_TRUE;

    lookupCfg.enableLookup = GT_TRUE;
    lookupCfg.pclId = 0;

    lookupCfg.groupKeyTypes.nonIpKey = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

    rc = prvTgfPclCfgTblSet(&interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E,
                            CPSS_PCL_LOOKUP_0_E, &lookupCfg);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet");

    /* set PCL rule */
    ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    ruleIndex  = basicTestConf[0].ruleIndex;

    cpssOsMemSet(&mask, 0, sizeof(mask));
    mask.ruleStdNotIp.etherType = 0xFFFF;

    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    pattern.ruleStdNotIp.etherType = 0x1000;

    cpssOsMemSet(&action, 0, sizeof(action));

    ipfixIndex = basicTestConf[0].ipfixIndex;

    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    action.policer.policerEnable = PRV_TGF_PCL_POLICER_ENABLE_COUNTER_ONLY_E;
    action.policer.policerId = ipfixIndex;

    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                  ruleFormat, ruleIndex);

    prvTgfIpfixStagesParamsSaveAndReset();

    wraparounfConf.action = PRV_TGF_IPFIX_WRAPAROUND_ACTION_NONE_E;
    wraparounfConf.dropThreshold = 0x3FFFFFFF;
    wraparounfConf.packetThreshold = 0x3FFFFFFF;
    wraparounfConf.byteThreshold.l[0] = 0xFFFFFFFF;
    wraparounfConf.byteThreshold.l[1] = 0xF;

    /* set wraparound configuration */
    rc = prvTgfIpfixWraparoundConfSet(prvTgfDevNum, testedStage, &wraparounfConf);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundConfSet: %d, %d",
                                 prvTgfDevNum, testedStage);

    /* IPFIX works in Flow mode */
    rc = prvTgfPolicerStageMeterModeSet(testedStage, PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeSet: %d",
                                 testedStage);

    rc = prvTgfPolicerCountingModeSet(prvTgfDevNum, testedStage,
                                      PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingModeSet: %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E);

    cpssOsMemSet(&ipfixEntry, 0, sizeof(ipfixEntry));
    if(PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum))
    {
        /* For AC5P, default cpuCode is CPSS_NET_UNDEFINED_CPU_CODE_E and hence need to set
         */
        ipfixEntry.lastCpuOrDropCode = CPSS_NET_FIRST_USER_DEFINED_E;
    }

    rc = prvTgfIpfixEntrySet(prvTgfDevNum, testedStage,
                             ipfixIndex, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntrySet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    ruleIndex  = basicTestConf[1].ruleIndex;
    pattern.ruleStdNotIp.etherType = 0x1001;
    ipfixIndex = basicTestConf[1].ipfixIndex;
    action.policer.policerId = ipfixIndex;

    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                  ruleFormat, ruleIndex);

    rc = prvTgfIpfixEntrySet(prvTgfDevNum, testedStage,
                             ipfixIndex, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntrySet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    ruleIndex  = basicTestConf[2].ruleIndex;
    pattern.ruleStdNotIp.etherType = 0x1002;
    ipfixIndex = basicTestConf[2].ipfixIndex;
    action.policer.policerId = ipfixIndex;

    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                  ruleFormat, ruleIndex);

    rc = prvTgfIpfixEntrySet(prvTgfDevNum, testedStage,
                             ipfixIndex, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntrySet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);
}

/**
* @internal prvTgfIpfixFirstNPacketsConfigInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine set Pcl rule.
*/
GT_VOID prvTgfIpfixFirstNPacketsConfigInit
(
    GT_VOID
)
{
    PRV_TGF_PCL_LOOKUP_CFG_STC lookupCfg;
    GT_STATUS               rc = GT_OK;
    GT_U32                  portIter = 0;
    GT_U32                           ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    CPSS_INTERFACE_INFO_STC          interfaceInfo;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;
    GT_U32                           ipfixIndex;
    PRV_TGF_IPFIX_ENTRY_STC          ipfixEntry;
    PRV_TGF_IPFIX_WRAPAROUND_CFG_STC wraparounfConf;

    /* clear entry */
    cpssOsMemSet(&localPortsVlanMembers, 0, sizeof(localPortsVlanMembers));

    /* ports 0, 8, 18, 23 are VLAN Members */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[portIter]);
    }


    /* set VLAN entry */
     prvTgfIpfixVlanTestInit(PRV_TGF_VLANID_CNS, localPortsVlanMembers);

     /* Init PCL. */
    rc = prvTgfPclInit();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"prvTgfPclInit");

    /* Enable PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_TRUE);

    /* enables ingress policy for port 8 */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 GT_TRUE);

    rc = prvTgfPclPortLookupCfgTabAccessModeSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                                CPSS_PCL_DIRECTION_INGRESS_E,
                                                CPSS_PCL_LOOKUP_0_E,
                                                0,
                                                PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet: %d, %d, %d, %d, %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 CPSS_PCL_DIRECTION_INGRESS_E,
                                 CPSS_PCL_LOOKUP_0_E,
                                 0,
                                 PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);

    /* Set PCL configuration table */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    interfaceInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    lookupCfg.enableLookup        = GT_TRUE;

    lookupCfg.enableLookup = GT_TRUE;
    lookupCfg.pclId = 0;

    lookupCfg.groupKeyTypes.nonIpKey = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

    rc = prvTgfPclCfgTblSet(&interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E,
                            CPSS_PCL_LOOKUP_0_E, &lookupCfg);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet");

    /* set PCL rule */
    ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    ruleIndex  = basicTestConf[0].ruleIndex;

    cpssOsMemSet(&mask, 0, sizeof(mask));
    mask.ruleStdNotIp.etherType = 0xFFFF;

    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    pattern.ruleStdNotIp.etherType = 0x1000;

    cpssOsMemSet(&action, 0, sizeof(action));

    ipfixIndex = basicTestConf[0].ipfixIndex;

    action.policer.policerEnable = PRV_TGF_PCL_POLICER_DISABLE_ALL_E;
    action.policer.policerId = ipfixIndex;
    action.flowId = ipfixIndex;
    action.ipfixEnable = GT_TRUE;
    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;

    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                  ruleFormat, ruleIndex);

    prvTgfIpfixStagesParamsSaveAndReset();

    wraparounfConf.action = PRV_TGF_IPFIX_WRAPAROUND_ACTION_NONE_E;
    wraparounfConf.dropThreshold = 0x3FFFFFFF;
    wraparounfConf.packetThreshold = 0x3FFFFFFF;
    wraparounfConf.byteThreshold.l[0] = 0xFFFFFFFF;
    wraparounfConf.byteThreshold.l[1] = 0xF;

    /* set wraparound configuration */
    rc = prvTgfIpfixWraparoundConfSet(prvTgfDevNum, testedStage, &wraparounfConf);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundConfSet: %d, %d",
                                 prvTgfDevNum, testedStage);

    /* IPFIX works in Flow mode */
    rc = prvTgfPolicerStageMeterModeSet(testedStage, PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeSet: %d",
                                 testedStage);

    rc = prvTgfPolicerCountingModeSet(prvTgfDevNum, testedStage,
                                      PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingModeSet: %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E);

    cpssOsMemSet(&ipfixEntry, 0, sizeof(ipfixEntry));
    /* For AC5P, default cpuCode is CPSS_NET_UNDEFINED_CPU_CODE_E and hence need to set
     */
    ipfixEntry.lastCpuOrDropCode = CPSS_NET_FIRST_USER_DEFINED_E + 15;
    ipfixEntry.lastPacketCommand = CPSS_PACKET_CMD_FORWARD_E;
    ipfixEntry.numberOfFirstPacketsToMirror = 4;

    rc = prvTgfIpfixEntrySet(prvTgfDevNum, testedStage,
                             ipfixIndex, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntrySet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    ruleIndex  = basicTestConf[1].ruleIndex;
    pattern.ruleStdNotIp.etherType = 0x1001;
    ipfixIndex = basicTestConf[1].ipfixIndex;
    action.policer.policerEnable = PRV_TGF_PCL_POLICER_DISABLE_ALL_E;
    action.policer.policerId = ipfixIndex;
    action.flowId = ipfixIndex;
    action.ipfixEnable = GT_TRUE;
    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;

    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                  ruleFormat, ruleIndex);

    rc = cpssDxChPolicerTrappedPacketsBillingEnableSet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPolicerTrappedPacketsBillingEnableSet stage");

    ipfixEntry.lastCpuOrDropCode = CPSS_NET_FIRST_USER_DEFINED_E + 25;
    ipfixEntry.lastPacketCommand = CPSS_PACKET_CMD_DROP_SOFT_E;
    ipfixEntry.numberOfFirstPacketsToMirror = 2;
    rc = prvTgfIpfixEntrySet(prvTgfDevNum, testedStage,
                             ipfixIndex, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntrySet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    ruleIndex  = basicTestConf[2].ruleIndex;
    pattern.ruleStdNotIp.etherType = 0x1002;
    ipfixIndex = basicTestConf[2].ipfixIndex;
    action.policer.policerEnable = PRV_TGF_PCL_POLICER_DISABLE_ALL_E;
    action.policer.policerId = ipfixIndex;
    action.flowId = ipfixIndex;
    action.ipfixEnable = GT_TRUE;
    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;

    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                  ruleFormat, ruleIndex);

    ipfixEntry.lastCpuOrDropCode = CPSS_NET_FIRST_USER_DEFINED_E + 45;
    ipfixEntry.lastPacketCommand = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    ipfixEntry.numberOfFirstPacketsToMirror = 5;
    rc = prvTgfIpfixEntrySet(prvTgfDevNum, testedStage,
                             ipfixIndex, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntrySet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);
}

/**
* @internal prvTgfIpfixEnableIpclEpclConfigInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine set Pcl rule.
*/
GT_VOID prvTgfIpfixEnableIpclEpclConfigInit
(
    GT_VOID
)
{
    PRV_TGF_PCL_LOOKUP_CFG_STC       lookupCfg;
    GT_STATUS                        rc = GT_OK;
    GT_U32                           portIter = 0;
    PRV_TGF_PCL_RULE_FORMAT_UNT      mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT      pattern;
    PRV_TGF_PCL_ACTION_STC           action;
    CPSS_INTERFACE_INFO_STC          interfaceInfo;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleFormat;
    PRV_TGF_IPFIX_ENTRY_STC          ipfixEntry;
    PRV_TGF_IPFIX_WRAPAROUND_CFG_STC wraparounfConf;
    PRV_TGF_POLICER_MEMORY_STC       memoryCfg;
    PRV_TGF_IPFIX_FIRST_PACKET_CONFIG_STC firstPacketCfg;

    /* clear entry */
    cpssOsMemSet(&localPortsVlanMembers, 0, sizeof(localPortsVlanMembers));
    cpssOsMemSet(&firstPacketCfg, 0, sizeof(firstPacketCfg));

    /* ports 0, 8, 18, 23 are VLAN Members */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&localPortsVlanMembers,prvTgfPortsArray[portIter]);
    }


    /* set VLAN entry */
     prvTgfIpfixVlanTestInit(PRV_TGF_VLANID_CNS, localPortsVlanMembers);

     /* Init PCL. */
    rc = prvTgfPclInit();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"prvTgfPclInit");

    /* Enable PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_TRUE);

    /* enables ingress policy for port 8 */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 GT_TRUE);

    rc = prvTgfPclPortLookupCfgTabAccessModeSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                                CPSS_PCL_DIRECTION_INGRESS_E,
                                                CPSS_PCL_LOOKUP_0_E,
                                                0,
                                                PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet: %d, %d, %d, %d, %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 CPSS_PCL_DIRECTION_INGRESS_E,
                                 CPSS_PCL_LOOKUP_0_E,
                                 0,
                                 PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);

    /* Set PCL configuration table */
    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    interfaceInfo.type            = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    interfaceInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    lookupCfg.enableLookup        = GT_TRUE;

    lookupCfg.enableLookup = GT_TRUE;
    lookupCfg.pclId = 0;

    lookupCfg.groupKeyTypes.nonIpKey = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    lookupCfg.groupKeyTypes.ipv4Key  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    lookupCfg.groupKeyTypes.ipv6Key  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;

    rc = prvTgfPclCfgTblSet(&interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E,
                            CPSS_PCL_LOOKUP_0_E, &lookupCfg);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclCfgTblSet");

    /* set PCL rule */
    ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;

    cpssOsMemSet(&mask, 0, sizeof(mask));
    mask.ruleStdNotIp.etherType = 0xFFFF;

    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    pattern.ruleStdNotIp.etherType = 0x1000;

    cpssOsMemSet(&action, 0, sizeof(action));

    action.policer.policerEnable = PRV_TGF_PCL_POLICER_DISABLE_ALL_E;
    action.flowId                = PRV_TGF_IPCL_ACTION_FLOW_ID_CNS;
    action.ipfixEnable           = GT_TRUE;
    action.pktCmd                = CPSS_PACKET_CMD_FORWARD_E;

    rc = prvTgfPclRuleSet(ruleFormat, prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(0, 1), &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                  ruleFormat, prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(0, 1));

    prvTgfIpfixStagesParamsSaveAndReset();

    wraparounfConf.action = PRV_TGF_IPFIX_WRAPAROUND_ACTION_NONE_E;
    wraparounfConf.dropThreshold = 0x3FFFFFFF;
    wraparounfConf.packetThreshold = 0x3FFFFFFF;
    wraparounfConf.byteThreshold.l[0] = 0xFFFFFFFF;
    wraparounfConf.byteThreshold.l[1] = 0xF;

    /* set wraparound configuration */
    rc = prvTgfIpfixWraparoundConfSet(prvTgfDevNum, testedStage, &wraparounfConf);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundConfSet: %d, %d",
                                 prvTgfDevNum, testedStage);

    /* IPFIX works in Flow mode */
    rc = prvTgfPolicerStageMeterModeSet(testedStage, PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeSet: %d",
                                 testedStage);

    rc = prvTgfPolicerCountingModeSet(prvTgfDevNum, testedStage,
                                      PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingModeSet: %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E);

    cpssOsMemSet(&ipfixEntry, 0, sizeof(ipfixEntry));
    /* For AC5P, default cpuCode is CPSS_NET_UNDEFINED_CPU_CODE_E and hence need to set
     */
    ipfixEntry.lastCpuOrDropCode = CPSS_NET_FIRST_USER_DEFINED_E + 15;

    rc = prvTgfIpfixEntrySet(prvTgfDevNum, testedStage,
                             PRV_TGF_IPCL_ACTION_FLOW_ID_CNS, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntrySet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, PRV_TGF_IPCL_ACTION_FLOW_ID_CNS);

    /* IPFIX works in Flow mode */
    rc = prvTgfPolicerStageMeterModeSet(PRV_TGF_POLICER_STAGE_EGRESS_E, PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeSet: %d",
                                 PRV_TGF_POLICER_STAGE_EGRESS_E);
    rc = prvTgfPolicerCountingModeSet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E,
                                      PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPolicerCountingModeSet: %d, %d, %d",
                                 prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E,
                                 PRV_TGF_POLICER_COUNTING_BILLING_IPFIX_E);

    /* AUTODOC: init PCL Engine for send port 0: */
    /* AUTODOC:   ingress direction, first lookup */
    /* AUTODOC:   nonIpKey INGRESS_EXT_NOT_IPV6 */
    /* AUTODOC:   ipv4Key INGRESS_EXT_NOT_IPV6 */
    /* AUTODOC:   ipv6Key INGRESS_EXT_IPV6_L2 */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS + 2],
        CPSS_PCL_DIRECTION_EGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInit: %d", GT_TRUE);

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    cpssOsMemSet(&memoryCfg, 0, sizeof(memoryCfg));
    rc = prvTgfPolicerMemorySizeGet(prvTgfDevNum, &memoryCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerMemorySizeGet: %d",prvTgfDevNum);

    /* set PCL rule */
    ruleFormat                   = PRV_TGF_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E;
    action.policer.policerEnable = PRV_TGF_PCL_POLICER_DISABLE_ALL_E;
    action.flowId                = memoryCfg.numCountingEntries[PRV_TGF_POLICER_STAGE_INGRESS_0_E] + PRV_TGF_EPCL_ACTION_FLOW_ID_CNS;
    action.ipfixEnable           = GT_TRUE;
    action.egressPolicy          = GT_TRUE;
    action.pktCmd                = CPSS_PACKET_CMD_FORWARD_E;

    rc = prvTgfPclRuleSet(ruleFormat, prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(1), &mask, &pattern, &action);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet:");

    wraparounfConf.action = PRV_TGF_IPFIX_WRAPAROUND_ACTION_NONE_E;
    wraparounfConf.dropThreshold = 0x3FFFFFFF;
    wraparounfConf.packetThreshold = 0x3FFFFFFF;
    wraparounfConf.byteThreshold.l[0] = 0xFFFFFFFF;
    wraparounfConf.byteThreshold.l[1] = 0xF;

    /* set wraparound configuration */
    rc = prvTgfIpfixWraparoundConfSet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E, &wraparounfConf);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixWraparoundConfSet: %d, %d",
                                 prvTgfDevNum, testedStage);

    eplrIpfixIndex = memoryCfg.numCountingEntries[PRV_TGF_POLICER_STAGE_INGRESS_0_E] + PRV_TGF_EPCL_ACTION_FLOW_ID_CNS;
    rc = prvTgfIpfixFirstPacketsMirrorEnableGet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E,
                                                eplrIpfixIndex, &enableRestore);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixFirstPacketsMirrorEnableGet: %d ipfixIndex %d",
                                 prvTgfDevNum, eplrIpfixIndex);

    rc = prvTgfIpfixFirstPacketsMirrorEnableSet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E,
                                                eplrIpfixIndex, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixFirstPacketsMirrorEnableSet: %d ipfixIndex %d",
                                 prvTgfDevNum, PRV_TGF_EPCL_ACTION_FLOW_ID_CNS);

    rc = prvTgfIpfixFirstPacketConfigGet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E, &firstPktCfgRestore);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixFirstPacketConfigGet: pktCmd %d cpuCode %d",
                                 firstPktCfgRestore.packetCmd, firstPktCfgRestore.cpuCode);

    firstPacketCfg.packetCmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
    firstPacketCfg.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E + 63;
    rc = prvTgfIpfixFirstPacketConfigSet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E, &firstPacketCfg);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixFirstPacketConfigSet: pktCmd %d cpuCode %d",
                                 firstPacketCfg.packetCmd, firstPacketCfg.cpuCode);

    cpssOsMemSet(&ipfixEntry, 0, sizeof(ipfixEntry));
    /* For AC5P, default cpuCode is CPSS_NET_UNDEFINED_CPU_CODE_E and hence need to set
     */
    ipfixEntry.lastCpuOrDropCode = CPSS_NET_FIRST_USER_DEFINED_E + 15;
    ipfixEntry.lastPacketCommand = CPSS_PACKET_CMD_FORWARD_E;
    ipfixEntry.numberOfFirstPacketsToMirror = 1;

    rc = prvTgfIpfixEntrySet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E,
                             PRV_TGF_EPCL_ACTION_FLOW_ID_CNS, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntrySet: %d, %d, %d",
                                 prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E, PRV_TGF_EPCL_ACTION_FLOW_ID_CNS);

}

/**
* @internal prvTgfIpfixFirstIngressTestInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine set Pcl rule.
*/
GT_VOID prvTgfIpfixFirstIngressTestInit
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32 maxCountersNum;
    GT_U32  numPhyPorts = MAX(256,PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.maxPhysicalPorts);

    rc = prvUtfCfgTableNumEntriesGet(prvTgfDevNum,
                                     PRV_TGF_CFG_TABLE_POLICER_COUNTERS_E,
                                     &maxCountersNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvUtfCfgTableNumEntriesGet: %d, %d",
                                 prvTgfDevNum,
                                 PRV_TGF_CFG_TABLE_POLICER_COUNTERS_E);

    cpssOsMemSet(basicTestConf, 0, sizeof(basicTestConf));

    basicTestConf[0].ruleIndex  = 0;
    basicTestConf[0].ipfixIndex = 0;
    basicTestConf[0].burstCount = 1;
    basicTestConf[0].payloadLen = 44;

    basicTestConf[1].ruleIndex  = 1;
    basicTestConf[1].ipfixIndex = maxCountersNum/2 - 1;
    basicTestConf[1].burstCount = 2;
    basicTestConf[1].payloadLen = 480;

    basicTestConf[2].ruleIndex  = 2;
    basicTestConf[2].ipfixIndex = (maxCountersNum > numPhyPorts) ? (maxCountersNum - 1 - numPhyPorts) : maxCountersNum/4 - 1;
    basicTestConf[2].burstCount = 3;
    basicTestConf[2].payloadLen = LONG_PAYLOAD_LEN_CNS;

    testedStage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;

    prvTgfIpfixIngressTestInit();
}

/**
* @internal prvTgfIpfixFirstUseAllIngressTestInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine set Pcl rule.
*/
GT_VOID prvTgfIpfixFirstUseAllIngressTestInit
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32 maxCountersNum;
    PRV_TGF_POLICER_MEMORY_STC memoryCfg;

    rc = prvUtfCfgTableNumEntriesGet(prvTgfDevNum,
                                     PRV_TGF_CFG_TABLE_POLICER_COUNTERS_E,
                                     &maxCountersNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvUtfCfgTableNumEntriesGet: %d, %d",
                                 prvTgfDevNum,
                                 PRV_TGF_CFG_TABLE_POLICER_COUNTERS_E);

    cpssOsMemSet(basicTestConf, 0, sizeof(basicTestConf));
    cpssOsMemSet(&memoryCfg, 0, sizeof(PRV_TGF_POLICER_MEMORY_STC));

    basicTestConf[0].ruleIndex  = 0;
    basicTestConf[0].ipfixIndex = 0;
    basicTestConf[0].burstCount = 1;
    basicTestConf[0].payloadLen = 44;

    basicTestConf[1].ruleIndex  = 1;
    basicTestConf[1].ipfixIndex = maxCountersNum/2;
    basicTestConf[1].burstCount = 2;
    basicTestConf[1].payloadLen = 480;

    basicTestConf[2].ruleIndex  = 2;
    basicTestConf[2].ipfixIndex = maxCountersNum - 1;
    basicTestConf[2].burstCount = 3;
    basicTestConf[2].payloadLen = LONG_PAYLOAD_LEN_CNS;

    testedStage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;

    if(PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum))
    {
        memoryCfg.numMeteringEntries[PRV_TGF_POLICER_STAGE_EGRESS_E]    =  _1K;
        memoryCfg.numMeteringEntries[PRV_TGF_POLICER_STAGE_INGRESS_1_E] =  0;
        memoryCfg.numMeteringEntries[PRV_TGF_POLICER_STAGE_INGRESS_0_E] =  PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning.tableSize.policersNum -
                                                                           memoryCfg.numMeteringEntries[PRV_TGF_POLICER_STAGE_EGRESS_E];

        memoryCfg.numCountingEntries[PRV_TGF_POLICER_STAGE_EGRESS_E]    =  memoryCfg.numMeteringEntries[PRV_TGF_POLICER_STAGE_EGRESS_E];
        memoryCfg.numCountingEntries[PRV_TGF_POLICER_STAGE_INGRESS_1_E] =  memoryCfg.numMeteringEntries[PRV_TGF_POLICER_STAGE_INGRESS_1_E];
        memoryCfg.numCountingEntries[PRV_TGF_POLICER_STAGE_INGRESS_0_E] =  memoryCfg.numMeteringEntries[PRV_TGF_POLICER_STAGE_INGRESS_0_E];

        rc = prvTgfPolicerMemorySizeSet(prvTgfDevNum,
                                        &memoryCfg);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerMemorySizeSet: %d",
                                     prvTgfDevNum);
    }
    else
    {
        rc = prvTgfPolicerMemorySizeModeSet(prvTgfDevNum,
                                            PRV_TGF_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_AND_LOWER_E,
                                            0,0);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerMemorySizeModeSet: %d, %d",
                                     prvTgfDevNum,
                                     PRV_TGF_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_AND_LOWER_E);
    }

    prvTgfIpfixIngressTestInit();
}

/**
* @internal prvTgfIpfixSecondUseAllIngressTestInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine set Pcl rule.
*/
GT_VOID prvTgfIpfixSecondUseAllIngressTestInit
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32 maxCountersNum;
    PRV_TGF_POLICER_MEMORY_STC memoryCfg;

    rc = prvUtfCfgTableNumEntriesGet(prvTgfDevNum,
                                     PRV_TGF_CFG_TABLE_POLICER_COUNTERS_E,
                                     &maxCountersNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvUtfCfgTableNumEntriesGet: %d, %d",
                                 prvTgfDevNum,
                                 PRV_TGF_CFG_TABLE_POLICER_COUNTERS_E);

    cpssOsMemSet(basicTestConf, 0, sizeof(basicTestConf));
    cpssOsMemSet(&memoryCfg, 0, sizeof(PRV_TGF_POLICER_MEMORY_STC));

    basicTestConf[0].ruleIndex  = 0;
    basicTestConf[0].ipfixIndex = 0;
    basicTestConf[0].burstCount = 1;
    basicTestConf[0].payloadLen = 44;

    basicTestConf[1].ruleIndex  = 1;
    basicTestConf[1].ipfixIndex = maxCountersNum/2;
    basicTestConf[1].burstCount = 2;
    basicTestConf[1].payloadLen = 480;

    basicTestConf[2].ruleIndex  = 2;
    basicTestConf[2].ipfixIndex = maxCountersNum - 1;
    basicTestConf[2].burstCount = 1;
    basicTestConf[2].payloadLen = LONG_PAYLOAD_LEN_CNS;

    testedStage = PRV_TGF_POLICER_STAGE_INGRESS_1_E;

    if(GT_FALSE ==  prvTgfPolicerStageCheck(testedStage))
    {
        return ;
    }

    if(PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum))
    {
        memoryCfg.numMeteringEntries[PRV_TGF_POLICER_STAGE_INGRESS_0_E]  = 0;
        memoryCfg.numMeteringEntries[PRV_TGF_POLICER_STAGE_EGRESS_E]    = _1K;
        memoryCfg.numMeteringEntries[PRV_TGF_POLICER_STAGE_INGRESS_1_E]  = PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning.tableSize.policersNum -
                                                                           memoryCfg.numMeteringEntries[PRV_TGF_POLICER_STAGE_EGRESS_E];

        memoryCfg.numCountingEntries[PRV_TGF_POLICER_STAGE_INGRESS_0_E]  = memoryCfg.numMeteringEntries[PRV_TGF_POLICER_STAGE_INGRESS_0_E];
        memoryCfg.numCountingEntries[PRV_TGF_POLICER_STAGE_INGRESS_1_E]  = memoryCfg.numMeteringEntries[PRV_TGF_POLICER_STAGE_INGRESS_1_E];
        memoryCfg.numCountingEntries[PRV_TGF_POLICER_STAGE_EGRESS_E]    = memoryCfg.numMeteringEntries[PRV_TGF_POLICER_STAGE_EGRESS_E];

        rc = prvTgfPolicerMemorySizeSet(prvTgfDevNum, &memoryCfg);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerMemorySizeSet: %d",
                                     prvTgfDevNum);
    }
    else
    {
        rc = prvTgfPolicerMemorySizeModeSet(prvTgfDevNum,
                                            PRV_TGF_POLICER_MEMORY_CTRL_MODE_PLR1_UPPER_AND_LOWER_E,
                                            0,0);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerMemorySizeModeSet: %d, %d",
                                     prvTgfDevNum,
                                     PRV_TGF_POLICER_MEMORY_CTRL_MODE_PLR1_UPPER_AND_LOWER_E);
    }

    prvTgfIpfixIngressTestInit();

    /* switch IPLR0 to be in Port mode in order to avoid influence of IPLR0 on the test.
       this is MUST be done for SIP5 devices with hierarchical policing. */
    rc = prvTgfPolicerStageMeterModeSet(PRV_TGF_POLICER_STAGE_INGRESS_0_E, PRV_TGF_POLICER_STAGE_METER_MODE_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeSet: INGRESS_0, MODE_PORT");
}

/**
* @internal prvTgfIpfixTimestampVerificationTestInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine set Pcl rule.
*/
GT_VOID prvTgfIpfixTimestampVerificationTestInit
(
    GT_VOID
)
{
    cpssOsMemSet(basicTestConf, 0, sizeof(basicTestConf));

    basicTestConf[0].ruleIndex  = 0;
    basicTestConf[0].ipfixIndex = 0;
    basicTestConf[0].burstCount = 1;
    basicTestConf[0].payloadLen = 44;

    basicTestConf[1].ruleIndex  = 1;
    basicTestConf[1].ipfixIndex = 1;
    basicTestConf[1].burstCount = 1;
    basicTestConf[1].payloadLen = 44;

    basicTestConf[2].ruleIndex  = 2;
    basicTestConf[2].ipfixIndex = 2;
    basicTestConf[2].burstCount = 1;
    basicTestConf[2].payloadLen = 44;

    testedStage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;

    prvTgfIpfixIngressTestInit();
}

/**
* @internal prvTgfIpfixFirstNPacketsTestInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine set Pcl rule.
*/
GT_VOID prvTgfIpfixFirstNPacketsTestInit
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    PRV_TGF_POLICER_MEMORY_STC memoryCfg;
    PRV_TGF_IPFIX_FIRST_PACKET_CONFIG_STC firstPacketCfg;
    GT_U32 ipFixMirrorMaxIndex;
    GT_U32 ipFixMaxIndex;

    ipFixMirrorMaxIndex = PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.parametericTables.numEntriesIplrIpfix;

    cpssOsMemSet(&firstPacketCfg, 0, sizeof(firstPacketCfg));
    cpssOsMemSet(basicTestConf, 0, sizeof(basicTestConf));

    testedStage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;
    rc = prvTgfPolicerMemorySizeGet(prvTgfDevNum, &memoryCfgGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerMemorySizeGet: %d",
                                 prvTgfDevNum);
    memoryCfg.numMeteringEntries[PRV_TGF_POLICER_STAGE_INGRESS_1_E]  = 0;
    memoryCfg.numMeteringEntries[PRV_TGF_POLICER_STAGE_EGRESS_E]    = _1K;
    memoryCfg.numMeteringEntries[PRV_TGF_POLICER_STAGE_INGRESS_0_E]  = PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning.tableSize.policersNum -
                                                                       memoryCfg.numMeteringEntries[PRV_TGF_POLICER_STAGE_EGRESS_E];

    memoryCfg.numCountingEntries[PRV_TGF_POLICER_STAGE_INGRESS_1_E]  = memoryCfg.numMeteringEntries[PRV_TGF_POLICER_STAGE_INGRESS_1_E];
    memoryCfg.numCountingEntries[PRV_TGF_POLICER_STAGE_EGRESS_E]    = memoryCfg.numMeteringEntries[PRV_TGF_POLICER_STAGE_EGRESS_E];
    memoryCfg.numCountingEntries[PRV_TGF_POLICER_STAGE_INGRESS_0_E]  = (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.plrInfo.countingEntriesNum) -
        (memoryCfg.numCountingEntries[PRV_TGF_POLICER_STAGE_INGRESS_1_E] + memoryCfg.numCountingEntries[PRV_TGF_POLICER_STAGE_EGRESS_E]);

    rc = prvTgfPolicerMemorySizeSet(prvTgfDevNum, &memoryCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerMemorySizeSet: %d",
                                 prvTgfDevNum);

    ipFixMaxIndex = ipFixMirrorMaxIndex;
    if (ipFixMaxIndex > memoryCfg.numCountingEntries[testedStage])
    {
        ipFixMaxIndex = memoryCfg.numCountingEntries[testedStage];
    }

    basicTestConf[0].ruleIndex  = 1;
    basicTestConf[0].ipfixIndex = 1;
    basicTestConf[0].burstCount = 5;
    basicTestConf[0].payloadLen = 44;
    rc = prvTgfIpfixFirstPacketsMirrorEnableSet(prvTgfDevNum, testedStage, basicTestConf[0].ipfixIndex, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixFirstPacketsMirrorEnableSet: %d ipfixIndex %d",
                                 prvTgfDevNum, basicTestConf[0].ipfixIndex);

    firstPacketCfg.packetCmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
    firstPacketCfg.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E;
    rc = prvTgfIpfixFirstPacketConfigSet(prvTgfDevNum, testedStage, &firstPacketCfg);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixFirstPacketConfigSet: pktCmd %d cpuCode %d",
                                 firstPacketCfg.packetCmd, firstPacketCfg.cpuCode);

    basicTestConf[1].ruleIndex  = 2;
    basicTestConf[1].ipfixIndex = _8K;
    basicTestConf[1].burstCount = 5;
    basicTestConf[1].payloadLen = 44;
    if (ipFixMaxIndex <= (basicTestConf[1].ipfixIndex + 2))
    {
        basicTestConf[1].ipfixIndex = ipFixMaxIndex - 2;
    }
    rc = prvTgfIpfixFirstPacketsMirrorEnableSet(prvTgfDevNum, testedStage, basicTestConf[1].ipfixIndex, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixFirstPacketsMirrorEnableSet: %d ipfixIndex %d",
                                 prvTgfDevNum, basicTestConf[1].ipfixIndex);

    firstPacketCfg.packetCmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    firstPacketCfg.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E + 30;
    rc = prvTgfIpfixFirstPacketConfigSet(prvTgfDevNum, testedStage, &firstPacketCfg);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixFirstPacketConfigSet: pktCmd %d cpuCode %d",
                                 firstPacketCfg.packetCmd, firstPacketCfg.cpuCode);

    basicTestConf[2].ruleIndex  = 3;
    basicTestConf[2].ipfixIndex = _12K;
    basicTestConf[2].burstCount = 5;
    basicTestConf[2].payloadLen = 44;
    if (ipFixMaxIndex <= (basicTestConf[2].ipfixIndex + 1))
    {
        basicTestConf[2].ipfixIndex = ipFixMaxIndex - 1;
    }
    rc = prvTgfIpfixFirstPacketsMirrorEnableSet(prvTgfDevNum, testedStage, basicTestConf[2].ipfixIndex, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixFirstPacketsMirrorEnableSet: %d ipfixIndex %d",
                                 prvTgfDevNum, basicTestConf[2].ipfixIndex);

    firstPacketCfg.packetCmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
    firstPacketCfg.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E + 63;
    rc = prvTgfIpfixFirstPacketConfigSet(prvTgfDevNum, testedStage, &firstPacketCfg);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixFirstPacketConfigSet: pktCmd %d cpuCode %d",
                                 firstPacketCfg.packetCmd, firstPacketCfg.cpuCode);

    prvTgfIpfixFirstNPacketsConfigInit();
}

/**
* @internal prvTgfIpfixEnableIpclEpclTestInit function
* @endinternal
*
* @brief   Set test configuration:
*         Enable the Ingress Policy Engine set Pcl rule.
*/
GT_VOID prvTgfIpfixEnableIpclEpclTestInit
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    PRV_TGF_POLICER_MEMORY_STC memoryCfg;
    GT_U32 ipFixMirrorMaxIndex;
    GT_U32 ipFixMaxIndex;

    ipFixMirrorMaxIndex = PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.parametericTables.numEntriesIplrIpfix;

    cpssOsMemSet(basicTestConf, 0, sizeof(basicTestConf));

    testedStage = PRV_TGF_POLICER_STAGE_INGRESS_0_E;
    rc = prvTgfPolicerMemorySizeGet(prvTgfDevNum, &memoryCfgGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerMemorySizeGet: %d",
                                 prvTgfDevNum);
    memoryCfg.numMeteringEntries[PRV_TGF_POLICER_STAGE_INGRESS_1_E]  = 0;
    memoryCfg.numMeteringEntries[PRV_TGF_POLICER_STAGE_EGRESS_E]     = _1K;
    memoryCfg.numMeteringEntries[PRV_TGF_POLICER_STAGE_INGRESS_0_E]  = _1K;

    memoryCfg.numCountingEntries[PRV_TGF_POLICER_STAGE_INGRESS_1_E]  = 0;
    memoryCfg.numCountingEntries[PRV_TGF_POLICER_STAGE_EGRESS_E]     = 64;
    memoryCfg.numCountingEntries[PRV_TGF_POLICER_STAGE_INGRESS_0_E]  = 64;

    rc = prvTgfPolicerMemorySizeSet(prvTgfDevNum, &memoryCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerMemorySizeSet: %d",
                                 prvTgfDevNum);

    ipFixMaxIndex = ipFixMirrorMaxIndex;
    if (ipFixMaxIndex > memoryCfg.numCountingEntries[testedStage])
    {
        ipFixMaxIndex = memoryCfg.numCountingEntries[testedStage];
    }

    basicTestConf[0].ruleIndex  = 1;
    basicTestConf[0].ipfixIndex = 1;
    basicTestConf[0].burstCount = 5;
    basicTestConf[0].payloadLen = 44;

    prvTgfIpfixEnableIpclEpclConfigInit();
}

/**
* @internal prvTgfIpfixIngressTestTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpfixIngressTestTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_U32                  ipfixIndex;
    PRV_TGF_IPFIX_ENTRY_STC ipfixEntry;
    GT_U32                  burstCount;

    if(GT_FALSE ==  prvTgfPolicerStageCheck(testedStage))
    {
        return ;
    }

    /* enable packet trace */
    rc = tgfTrafficTracePacketByteSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "Error in tgfTrafficTracePacketByteSet %d", GT_TRUE);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* 64 bytes packet length */
    prvTgfPacketPayloadPart.dataLength = basicTestConf[0].payloadLen;
    prvTgfPacketInfo.totalLen =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;

    /* Set ethertype to value 0x1000 */
    prvTgfPayloadDataArr[0] = 0x10;
    prvTgfPayloadDataArr[1] = 0x00;

    burstCount = basicTestConf[0].burstCount;

    /* send packet */
    prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, burstCount);

    cpssOsTimerWkAfter(200);

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");
            continue;
        }


        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    ipfixIndex = basicTestConf[0].ipfixIndex;
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             ipfixIndex, GT_FALSE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    /* check IPFIX counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount * (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                 ipfixEntry.byteCount.l[0],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.packetCount,
                                 "packetCount different then expected");

    /* check IPFIX counters again with reset flag enabled */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             ipfixIndex, GT_TRUE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount * (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                 ipfixEntry.byteCount.l[0],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.packetCount,
                                 "packetCount different then expected");

    /* check IPFIX counters again and verify all counters are zero */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             ipfixIndex, GT_TRUE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    if (!prvTgfIpfixIsResetOnReadErratum(prvTgfDevNum))
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[0],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.packetCount,
                                     "packetCount different then expected");
    }
    else
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount * (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                     ipfixEntry.byteCount.l[0],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.packetCount,
                                     "packetCount different then expected");
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* 500 bytes packet length */
    prvTgfPacketPayloadPart.dataLength = basicTestConf[1].payloadLen;
    prvTgfPacketInfo.totalLen =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;

    /* Set ethertype to value 0x1001 */
    prvTgfPayloadDataArr[0] = 0x10;
    prvTgfPayloadDataArr[1] = 0x01;

    burstCount = basicTestConf[1].burstCount;

    /* send packet */
    prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, burstCount);

    cpssOsTimerWkAfter(200);

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");

            continue;
        }

        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    ipfixIndex = basicTestConf[1].ipfixIndex;
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             ipfixIndex, GT_FALSE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    /* check IPFIX counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount * (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                 ipfixEntry.byteCount.l[0],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.packetCount,
                                 "packetCount different then expected");

    /* check IPFIX counters again with reset flag enabled */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             ipfixIndex, GT_TRUE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount * (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                 ipfixEntry.byteCount.l[0],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.packetCount,
                                 "packetCount different then expected");

    /* check IPFIX counters again and verify all counters are zero */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             ipfixIndex, GT_TRUE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);
    if (!prvTgfIpfixIsResetOnReadErratum(prvTgfDevNum))
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[0],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.packetCount,
                                     "packetCount different then expected");
    }
    else
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount * (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                     ipfixEntry.byteCount.l[0],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.packetCount,
                                     "packetCount different then expected");
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* 1024 bytes packet length */
    prvTgfPacketPayloadPart.dataLength = basicTestConf[2].payloadLen;
    prvTgfPacketInfo.totalLen =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;

    /* Set ethertype to value 0x1002 */
    prvTgfPayloadDataArr[0] = 0x10;
    prvTgfPayloadDataArr[1] = 0x02;

    burstCount = basicTestConf[2].burstCount;

    /* send packet */
    prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, burstCount);

    cpssOsTimerWkAfter(200);

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");

            continue;
        }

        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    ipfixIndex = basicTestConf[2].ipfixIndex;
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             ipfixIndex, GT_FALSE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    /* check IPFIX counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount * (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                 ipfixEntry.byteCount.l[0],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.packetCount,
                                 "packetCount different then expected");

    /* check IPFIX counters again with reset flag enabled */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             ipfixIndex, GT_TRUE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount * (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                 ipfixEntry.byteCount.l[0],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.packetCount,
                                 "packetCount different then expected");

    /* check IPFIX counters again and verify all counters are zero */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             ipfixIndex, GT_TRUE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    if (!prvTgfIpfixIsResetOnReadErratum(prvTgfDevNum))
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[0],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.packetCount,
                                     "packetCount different then expected");
    }
    else
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount * (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                     ipfixEntry.byteCount.l[0],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.packetCount,
                                     "packetCount different then expected");
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}

/**
* @internal prvTgfIpfixFirstNPacketsTestTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpfixFirstNPacketsTestTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                     rc = GT_OK;
    GT_U32                        portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_U32                        ipfixIndex;
    PRV_TGF_IPFIX_ENTRY_STC       ipfixEntry;
    GT_U32                        burstCount;
    CPSS_DXCH_NET_RX_PARAMS_STC   dxChPcktParams;
    GT_U8                         queue;
    GT_U32                        numOfCpuPackets = 0;
    GT_U8                         packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32                        buffLen = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32                        packetActualLength = 0;

    if(GT_FALSE ==  prvTgfPolicerStageCheck(testedStage))
    {
        return ;
    }

    /* enable packet trace */
    rc = tgfTrafficTracePacketByteSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "Error in tgfTrafficTracePacketByteSet %d", GT_TRUE);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* 64 bytes packet length */
    prvTgfPacketPayloadPart.dataLength = basicTestConf[0].payloadLen;
    prvTgfPacketInfo.totalLen =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;

    /* Set ethertype to value 0x1000 */
    prvTgfPayloadDataArr[0] = 0x10;
    prvTgfPayloadDataArr[1] = 0x00;

    burstCount = basicTestConf[0].burstCount;

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* send packet */
    prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, burstCount);

    cpssOsTimerWkAfter(200);

    /* stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");

            continue;
        }

        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    rc = tgfTrafficTableRxPcktGet(numOfCpuPackets, packetBuff, &buffLen, &packetActualLength,
                        &prvTgfDevNum, &queue, (GT_VOID *)&dxChPcktParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktGet");

    UTF_VERIFY_EQUAL0_STRING_MAC(dxChPcktParams.dsaParam.commonParams.dsaTagType,
        CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT, "dsaTagType different than expected which is extended DSA tag - 4 words");

    UTF_VERIFY_EQUAL0_STRING_MAC(dxChPcktParams.dsaParam.dsaType,
        CPSS_DXCH_NET_DSA_CMD_TO_CPU_E, "dsaType is different than expected which is TO_CPU");

    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_NET_FIRST_USER_DEFINED_E + 63, dxChPcktParams.dsaParam.dsaInfo.toCpu.cpuCode,
                                 "dsaType CPU code different than expected");

    UTF_VERIFY_EQUAL0_STRING_MAC(basicTestConf[0].ipfixIndex, dxChPcktParams.dsaParam.dsaInfo.toCpu.flowIdTtOffset.flowId,
                                 "dsaType flow id different than expected");

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    ipfixIndex = basicTestConf[0].ipfixIndex;
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             ipfixIndex, GT_FALSE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    /* check IPFIX counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount * (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                 ipfixEntry.byteCount.l[0],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.packetCount,
                                 "packetCount different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.firstPacketsCounter,
                                 "first packets counter different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, ipfixEntry.firstTimestampValid,
                                 "first timestamp valid different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_PACKET_CMD_FORWARD_E, ipfixEntry.lastPacketCommand,
                                 "last packet command different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_NET_FIRST_USER_DEFINED_E + 15, ipfixEntry.lastCpuOrDropCode,
                                 "last CPU or drop code different than expected");

    /* check IPFIX counters again with reset flag enabled */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             ipfixIndex, GT_TRUE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount * (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                 ipfixEntry.byteCount.l[0],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.packetCount,
                                 "packetCount different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.firstPacketsCounter,
                                 "first packets counter different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, ipfixEntry.firstTimestampValid,
                                 "first timestamp valid different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_PACKET_CMD_FORWARD_E, ipfixEntry.lastPacketCommand,
                                 "last packet command different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_NET_FIRST_USER_DEFINED_E + 15, ipfixEntry.lastCpuOrDropCode,
                                 "last CPU or drop code different than expected");

    /* check IPFIX counters again and verify all counters are zero */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             ipfixIndex, GT_TRUE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    if (prvTgfIpfixIsResetOnReadErratum(prvTgfDevNum))
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(2 * burstCount * (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                     ipfixEntry.byteCount.l[0],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(2 * burstCount, ipfixEntry.packetCount,
                                     "packetCount different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, ipfixEntry.firstTimestampValid,
                                     "first timestamp valid different than expected");
    }
    else
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0,
                                     ipfixEntry.byteCount.l[0],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0,
                                     ipfixEntry.packetCount,
                                     "packetCount different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, ipfixEntry.firstTimestampValid,
                                     "first timestamp valid different than expected");
    }

    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.firstPacketsCounter,
                                 "first packets counter different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_PACKET_CMD_FORWARD_E, ipfixEntry.lastPacketCommand,
                                 "last packet command different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_NET_FIRST_USER_DEFINED_E + 15, ipfixEntry.lastCpuOrDropCode,
                                 "last CPU or drop code different than expected");

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* 500 bytes packet length */
    prvTgfPacketPayloadPart.dataLength = basicTestConf[1].payloadLen;
    prvTgfPacketInfo.totalLen =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;

    /* Set ethertype to value 0x1001 */
    prvTgfPayloadDataArr[0] = 0x10;
    prvTgfPayloadDataArr[1] = 0x01;

    burstCount = basicTestConf[1].burstCount;

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* send packet */
    prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, burstCount);

    cpssOsTimerWkAfter(200);

    /* stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");

            continue;
        }

        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(5, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    rc = tgfTrafficTableRxPcktGet(numOfCpuPackets, packetBuff, &buffLen, &packetActualLength,
                        &prvTgfDevNum, &queue, (GT_VOID *)&dxChPcktParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktGet");

    UTF_VERIFY_EQUAL0_STRING_MAC(dxChPcktParams.dsaParam.commonParams.dsaTagType,
        CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT, "dsaTagType different than expected which is extended DSA tag - 4 words");

    UTF_VERIFY_EQUAL0_STRING_MAC(dxChPcktParams.dsaParam.dsaType,
        CPSS_DXCH_NET_DSA_CMD_TO_CPU_E, "dsaType is different than expected which is TO_CPU");

    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_NET_FIRST_USER_DEFINED_E + 63, dxChPcktParams.dsaParam.dsaInfo.toCpu.cpuCode,
                                 "dsaType CPU code different than expected");

    UTF_VERIFY_EQUAL0_STRING_MAC(basicTestConf[1].ipfixIndex, dxChPcktParams.dsaParam.dsaInfo.toCpu.flowIdTtOffset.flowId,
                                 "dsaType flow id different than expected");

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    ipfixIndex = basicTestConf[1].ipfixIndex;

    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             ipfixIndex, GT_FALSE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    /* check IPFIX counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount * (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                 ipfixEntry.byteCount.l[0],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.packetCount,
                                 "packetCount different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.firstPacketsCounter,
                                 "first packets counter different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, ipfixEntry.firstTimestampValid,
                                 "first timestamp valid different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_PACKET_CMD_FORWARD_E, ipfixEntry.lastPacketCommand,
                                 "last packet command different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_NET_FIRST_USER_DEFINED_E + 25, ipfixEntry.lastCpuOrDropCode,
                                 "last CPU or drop code different than expected");

    /* check IPFIX counters again with reset flag enabled */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             ipfixIndex, GT_TRUE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount * (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                 ipfixEntry.byteCount.l[0],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.packetCount,
                                 "packetCount different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.firstPacketsCounter,
                                 "first packets counter different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, ipfixEntry.firstTimestampValid,
                                 "first timestamp valid different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_PACKET_CMD_FORWARD_E, ipfixEntry.lastPacketCommand,
                                 "last packet command different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_NET_FIRST_USER_DEFINED_E + 25, ipfixEntry.lastCpuOrDropCode,
                                 "last CPU or drop code different than expected");

    /* check IPFIX counters again and verify all counters are zero */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             ipfixIndex, GT_TRUE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    if (prvTgfIpfixIsResetOnReadErratum(prvTgfDevNum))
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(2 * burstCount * (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                     ipfixEntry.byteCount.l[0],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(2 * burstCount, ipfixEntry.packetCount,
                                     "packetCount different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, ipfixEntry.firstTimestampValid,
                                     "first timestamp valid different than expected");
    }
    else
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0,
                                     ipfixEntry.byteCount.l[0],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0,
                                     ipfixEntry.packetCount,
                                     "packetCount different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, ipfixEntry.firstTimestampValid,
                                     "first timestamp valid different than expected");
    }

    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.firstPacketsCounter,
                                 "first packets counter different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_PACKET_CMD_FORWARD_E, ipfixEntry.lastPacketCommand,
                                 "last packet command different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_NET_FIRST_USER_DEFINED_E + 25, ipfixEntry.lastCpuOrDropCode,
                                 "last CPU or drop code different than expected");

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* 1024 bytes packet length */
    prvTgfPacketPayloadPart.dataLength = basicTestConf[2].payloadLen;
    prvTgfPacketInfo.totalLen =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;

    /* Set ethertype to value 0x1002 */
    prvTgfPayloadDataArr[0] = 0x10;
    prvTgfPayloadDataArr[1] = 0x02;

    burstCount = basicTestConf[2].burstCount;
    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* send packet */
    prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, burstCount);

    cpssOsTimerWkAfter(200);

    /* stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");

            continue;
        }

        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(5, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    rc = tgfTrafficTableRxPcktGet(numOfCpuPackets, packetBuff, &buffLen, &packetActualLength,
                        &prvTgfDevNum, &queue, (GT_VOID *)&dxChPcktParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktGet");

    UTF_VERIFY_EQUAL0_STRING_MAC(dxChPcktParams.dsaParam.commonParams.dsaTagType,
        CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT, "dsaTagType different than expected which is extended DSA tag - 4 words");

    UTF_VERIFY_EQUAL0_STRING_MAC(dxChPcktParams.dsaParam.dsaType,
        CPSS_DXCH_NET_DSA_CMD_TO_CPU_E, "dsaType is different than expected which is TO_CPU");

    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_NET_FIRST_USER_DEFINED_E + 63, dxChPcktParams.dsaParam.dsaInfo.toCpu.cpuCode,
                                 "dsaType CPU code different than expected");

    UTF_VERIFY_EQUAL0_STRING_MAC(basicTestConf[2].ipfixIndex, dxChPcktParams.dsaParam.dsaInfo.toCpu.flowIdTtOffset.flowId,
                                 "dsaType flow id different than expected");

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    ipfixIndex = basicTestConf[2].ipfixIndex;
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             ipfixIndex, GT_FALSE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    /* check IPFIX counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount * (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                 ipfixEntry.byteCount.l[0],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.packetCount,
                                 "packetCount different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.firstPacketsCounter,
                                 "first packets counter different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, ipfixEntry.firstTimestampValid,
                                 "first timestamp valid different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_PACKET_CMD_FORWARD_E, ipfixEntry.lastPacketCommand,
                                 "last packet command different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_NET_FIRST_USER_DEFINED_E + 45, ipfixEntry.lastCpuOrDropCode,
                                 "last CPU or drop code different than expected");

    /* check IPFIX counters again with reset flag enabled */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             ipfixIndex, GT_TRUE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount * (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                 ipfixEntry.byteCount.l[0],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.packetCount,
                                 "packetCount different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.firstPacketsCounter,
                                 "first packets counter different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, ipfixEntry.firstTimestampValid,
                                 "first timestamp valid different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_PACKET_CMD_FORWARD_E, ipfixEntry.lastPacketCommand,
                                 "last packet command different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_NET_FIRST_USER_DEFINED_E + 45, ipfixEntry.lastCpuOrDropCode,
                                 "last CPU or drop code different than expected");

    /* check IPFIX counters again and verify all counters are zero */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             ipfixIndex, GT_TRUE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    if (prvTgfIpfixIsResetOnReadErratum(prvTgfDevNum))
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(2 * burstCount * (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                     ipfixEntry.byteCount.l[0],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(2 * burstCount, ipfixEntry.packetCount,
                                     "packetCount different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, ipfixEntry.firstTimestampValid,
                                     "first timestamp valid different than expected");
    }
    else
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0,
                                     ipfixEntry.byteCount.l[0],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0,
                                     ipfixEntry.packetCount,
                                     "packetCount different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, ipfixEntry.firstTimestampValid,
                                     "first timestamp valid different than expected");
    }

    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.firstPacketsCounter,
                                 "first packets counter different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_PACKET_CMD_FORWARD_E, ipfixEntry.lastPacketCommand,
                                 "last packet command different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_NET_FIRST_USER_DEFINED_E + 45, ipfixEntry.lastCpuOrDropCode,
                                 "last CPU or drop code different than expected");
    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}

/**
* @internal prvTgfIpfixEnableIpclEpclTestTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpfixEnableIpclEpclTestTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS                     rc = GT_OK;
    GT_U32                        portIter = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    GT_U32                        ipfixIndex;
    PRV_TGF_IPFIX_ENTRY_STC       ipfixEntry;
    GT_U32                        burstCount;
    GT_BOOL                       enable;
    GT_U32                        bmp[2];
    GT_U32                        bmpMask[2];
    GT_U32                        expectedBmp[2];
    CPSS_DXCH_NET_RX_PARAMS_STC   dxChPcktParams;
    GT_U8                         queue;
    GT_U32                        numOfCpuPackets = 0;
    GT_U8                         packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32                        buffLen = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32                        packetActualLength = 0;
    PRV_TGF_POLICER_MEMORY_STC    memoryCfg;

    if(GT_FALSE ==  prvTgfPolicerStageCheck(testedStage))
    {
        return ;
    }

    /* enable packet trace */
    rc = tgfTrafficTracePacketByteSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "Error in tgfTrafficTracePacketByteSet %d", GT_TRUE);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* 64 bytes packet length */
    prvTgfPacketPayloadPart.dataLength = basicTestConf[0].payloadLen;
    prvTgfPacketInfo.totalLen =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;

    /* Set ethertype to value 0x1000 */
    prvTgfPayloadDataArr[0] = 0x10;
    prvTgfPayloadDataArr[1] = 0x00;

    burstCount = basicTestConf[0].burstCount;

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* enabling aging on IPLR0 */
    rc = prvTgfIpfixAgingEnableSet(prvTgfDevNum, testedStage, GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixAgingEnableSet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, GT_TRUE);

    rc = prvTgfIpfixAgingEnableGet(prvTgfDevNum, testedStage, &enable);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixAgingEnableGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage);

    UTF_VERIFY_EQUAL0_STRING_MAC(enable, GT_TRUE,
                                 "Aging is expected to be enabled but disabled");

    /* enabling aging on EPLR */
    rc = prvTgfIpfixAgingEnableSet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E, GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixAgingEnableSet: %d, %d, %d",
                                 prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E, GT_TRUE);

    rc = prvTgfIpfixAgingEnableGet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E, &enable);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixAgingEnableGet: %d, %d, %d",
                                 prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E);

    UTF_VERIFY_EQUAL0_STRING_MAC(enable, GT_TRUE,
                                 "Aging is expected to be enabled but disabled");

    /* send packet */
    prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, burstCount);

    cpssOsTimerWkAfter(200);

    /* stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* check counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
        {
            /* check Rx counters */
            UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsRcv.l[0],
                                         "get another goodPktsRcv counter than expected");

            continue;
        }

        /* check Tx counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, portCntrs.goodPktsSent.l[0],
                                     "get another goodPktsSent counter than expected");
    }

    ipfixIndex = PRV_TGF_IPCL_ACTION_FLOW_ID_CNS;
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             ipfixIndex, GT_FALSE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    /* check IPFIX counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount * (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                 ipfixEntry.byteCount.l[0],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.packetCount,
                                 "packetCount different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[0] different then expected");

    /* check IPFIX counters again with reset flag enabled */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             ipfixIndex, GT_TRUE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount * (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                 ipfixEntry.byteCount.l[0],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, ipfixEntry.packetCount,
                                 "packetCount different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[0] different then expected");

    /* check IPFIX counters again and verify all counters are zero */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             ipfixIndex, GT_TRUE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    if (prvTgfIpfixIsResetOnReadErratum(prvTgfDevNum))
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(2 * burstCount * (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                     ipfixEntry.byteCount.l[0],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(2 * burstCount, ipfixEntry.packetCount,
                                     "packetCount different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, ipfixEntry.firstTimestampValid,
                                     "first timestamp valid different than expected");
    }
    else
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0,
                                     ipfixEntry.byteCount.l[0],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0,
                                     ipfixEntry.packetCount,
                                     "packetCount different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, ipfixEntry.firstTimestampValid,
                                     "first timestamp valid different than expected");
    }

    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[0] different then expected");

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    rc = tgfTrafficTableRxPcktGet(numOfCpuPackets, packetBuff, &buffLen, &packetActualLength,
                        &prvTgfDevNum, &queue, (GT_VOID *)&dxChPcktParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktGet");

    UTF_VERIFY_EQUAL0_STRING_MAC(dxChPcktParams.dsaParam.commonParams.dsaTagType,
        CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT, "dsaTagType different than expected which is extended DSA tag - 4 words");

    UTF_VERIFY_EQUAL0_STRING_MAC(dxChPcktParams.dsaParam.dsaType,
        CPSS_DXCH_NET_DSA_CMD_TO_CPU_E, "dsaType is different than expected which is TO_CPU");

    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_NET_FIRST_USER_DEFINED_E + 63, dxChPcktParams.dsaParam.dsaInfo.toCpu.cpuCode,
                                 "dsaType CPU code different than expected");

    rc = prvTgfPolicerMemorySizeGet(prvTgfDevNum, &memoryCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerMemorySizeGet: %d",
                                 prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(memoryCfg.numCountingEntries[PRV_TGF_POLICER_STAGE_INGRESS_0_E] + PRV_TGF_EPCL_ACTION_FLOW_ID_CNS,
                                 dxChPcktParams.dsaParam.dsaInfo.toCpu.flowIdTtOffset.flowId,
                                 "dsaType flow id different than expected");

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    ipfixIndex = PRV_TGF_EPCL_ACTION_FLOW_ID_CNS;
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E,
                             ipfixIndex, GT_FALSE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    /* check IPFIX counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount + 1/* mirror packet */, ipfixEntry.packetCount,
                                 "packetCount different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[0] different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount + 1, ipfixEntry.firstPacketsCounter,
                                 "first packets counter different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, ipfixEntry.firstTimestampValid,
                                 "first timestamp valid different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_PACKET_CMD_FORWARD_E, ipfixEntry.lastPacketCommand,
                                 "last packet command different than expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_NET_FIRST_USER_DEFINED_E + 15, ipfixEntry.lastCpuOrDropCode,
                                 "last CPU or drop code different than expected");


    /* check IPFIX counters again with reset flag enabled */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E,
                             ipfixIndex, GT_TRUE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount + 1/* mirror packet */, ipfixEntry.packetCount,
                                 "packetCount different then expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[0] different then expected");

    /* check IPFIX counters again and verify all counters are zero */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E,
                             ipfixIndex, GT_TRUE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    if (prvTgfIpfixIsResetOnReadErratum(prvTgfDevNum))
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(2 * burstCount * (TGF_L1_OVERHEAD_CNS + prvTgfPacketInfo.totalLen + TGF_CRC_LEN_CNS),
                                     ipfixEntry.byteCount.l[0],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(2 * burstCount, ipfixEntry.packetCount,
                                     "packetCount different then expected");
    }
    else
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0,
                                     ipfixEntry.byteCount.l[0],
                                     "btyeCount.l[0] different then expected");
        UTF_VERIFY_EQUAL0_STRING_MAC(0,
                                     ipfixEntry.packetCount,
                                     "packetCount different then expected");
    }

    UTF_VERIFY_EQUAL0_STRING_MAC(0, ipfixEntry.byteCount.l[1],
                                 "btyeCount.l[0] different then expected");

    bmpMask[0] = 0xFFFFFFFF;
    expectedBmp[0] = 1 << PRV_TGF_IPCL_ACTION_FLOW_ID_CNS;
    bmpMask[1] = 0xFFFFFFFF;
    expectedBmp[1] = 1 << (PRV_TGF_EPCL_ACTION_FLOW_ID_CNS - 32);

    /* check aging bit map updated correctly on IPLR0 */
    rc = prvTgfIpfixAgingStatusGet(prvTgfDevNum, testedStage,
                                   PRV_TGF_AGING_START_INDEX_CNS, PRV_TGF_AGING_END_INDEX_CNS,
                                   GT_FALSE, &bmp[0]);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixAgingStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 PRV_TGF_AGING_START_INDEX_CNS, PRV_TGF_AGING_END_INDEX_CNS,
                                 GT_FALSE);

    UTF_VERIFY_EQUAL0_STRING_MAC((bmp[0] & bmpMask[0]), expectedBmp[0],
                                 "Aging indication expected");

    /* read aging bit map again with reset flag enabled */
    rc = prvTgfIpfixAgingStatusGet(prvTgfDevNum, testedStage,
                                   PRV_TGF_AGING_START_INDEX_CNS, PRV_TGF_AGING_END_INDEX_CNS,
                                   GT_TRUE, &bmp[0]);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixAgingStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 PRV_TGF_AGING_START_INDEX_CNS, PRV_TGF_AGING_END_INDEX_CNS,
                                 GT_TRUE);

    UTF_VERIFY_EQUAL0_STRING_MAC((bmp[0] & bmpMask[0]), expectedBmp[0],
                                 "Aging indication expected");

    /* verify bit map is cleared */
    rc = prvTgfIpfixAgingStatusGet(prvTgfDevNum, testedStage,
                                   PRV_TGF_AGING_START_INDEX_CNS, PRV_TGF_AGING_END_INDEX_CNS,
                                   GT_TRUE, &bmp[0]);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixAgingStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, testedStage,
                                 PRV_TGF_AGING_START_INDEX_CNS, PRV_TGF_AGING_END_INDEX_CNS,
                                 GT_TRUE);

    UTF_VERIFY_EQUAL0_STRING_MAC((bmp[0] & bmpMask[0]), 0,
                                 "No aging indication expected");

    /* check aging bit map updated correctly on EPLR */
    rc = prvTgfIpfixAgingStatusGet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E,
                                   PRV_TGF_AGING_START_INDEX_CNS+PRV_TGF_EPCL_ACTION_FLOW_ID_CNS, PRV_TGF_AGING_END_INDEX_CNS+PRV_TGF_EPCL_ACTION_FLOW_ID_CNS,
                                   GT_FALSE, &bmp[1]);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixAgingStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E,
                                 PRV_TGF_AGING_START_INDEX_CNS+PRV_TGF_EPCL_ACTION_FLOW_ID_CNS, PRV_TGF_AGING_END_INDEX_CNS+PRV_TGF_EPCL_ACTION_FLOW_ID_CNS,
                                 GT_FALSE);

    UTF_VERIFY_EQUAL0_STRING_MAC((bmp[1] & bmpMask[1]), expectedBmp[1],
                                 "Aging indication expected");

    /* read aging bit map again with reset flag enabled */
    rc = prvTgfIpfixAgingStatusGet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E,
                                   PRV_TGF_AGING_START_INDEX_CNS+PRV_TGF_EPCL_ACTION_FLOW_ID_CNS, PRV_TGF_AGING_END_INDEX_CNS+PRV_TGF_EPCL_ACTION_FLOW_ID_CNS,
                                   GT_TRUE, &bmp[1]);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixAgingStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E,
                                 PRV_TGF_AGING_START_INDEX_CNS+PRV_TGF_EPCL_ACTION_FLOW_ID_CNS, PRV_TGF_AGING_END_INDEX_CNS+PRV_TGF_EPCL_ACTION_FLOW_ID_CNS,
                                 GT_TRUE);

    UTF_VERIFY_EQUAL0_STRING_MAC((bmp[1] & bmpMask[1]), expectedBmp[1],
                                 "Aging indication expected");

    /* verify bit map is cleared */
    rc = prvTgfIpfixAgingStatusGet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E,
                                   PRV_TGF_AGING_START_INDEX_CNS+PRV_TGF_EPCL_ACTION_FLOW_ID_CNS, PRV_TGF_AGING_END_INDEX_CNS+PRV_TGF_EPCL_ACTION_FLOW_ID_CNS,
                                   GT_TRUE, &bmp[1]);
    UTF_VERIFY_EQUAL5_STRING_MAC(GT_OK, rc, "prvTgfIpfixAgingStatusGet: %d, %d, %d, %d, %d",
                                 prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E,
                                 PRV_TGF_AGING_START_INDEX_CNS+PRV_TGF_EPCL_ACTION_FLOW_ID_CNS, PRV_TGF_AGING_END_INDEX_CNS+PRV_TGF_EPCL_ACTION_FLOW_ID_CNS,
                                 GT_TRUE);

    UTF_VERIFY_EQUAL0_STRING_MAC((bmp[1] & bmpMask[1]), 0,
                                 "No aging indication expected");

}

/**
* @internal prvTgfIpfixTimestampVerificationTestTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfIpfixTimestampVerificationTestTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       portIter = 0;
    /*CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;*/
    GT_U32                  ipfixIndex;
    PRV_TGF_IPFIX_ENTRY_STC ipfixEntry;
    GT_U32                  burstCount;
    GT_U32                  secondsFromTimerToCompare;
    GT_U32                  secondsFromEntryToCompare;
    PRV_TGF_IPFIX_TIMER_STC timer;

    /* enable packet trace */
    rc = tgfTrafficTracePacketByteSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "Error in tgfTrafficTracePacketByteSet %d", GT_TRUE);

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* 64 bytes packet length */
    prvTgfPacketPayloadPart.dataLength = basicTestConf[0].payloadLen;
    prvTgfPacketInfo.totalLen =
            TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + prvTgfPacketPayloadPart.dataLength;

    /* Set ethertype to value 0x1000 */
    prvTgfPayloadDataArr[0] = 0x10;
    prvTgfPayloadDataArr[1] = 0x00;

    burstCount = basicTestConf[0].burstCount;

    /* get IPFIX timer */
    rc = prvTgfIpfixTimerGet(prvTgfDevNum, testedStage, &timer);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d",
                                 prvTgfDevNum, testedStage);

    /* send packet */
    prvTgfIpfixTestPacketSend(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfPacketInfo, burstCount);

    cpssOsTimerWkAfter(5000);

    ipfixIndex = basicTestConf[0].ipfixIndex;

    /* check IPFIX timestamp */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             ipfixIndex, GT_TRUE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    secondsFromEntryToCompare = ((ipfixEntry.timeStamp & 0xFF00 ) >> 8);

    secondsFromTimerToCompare = (timer.secondTimer.l[0] & 0xFF);

    if( (secondsFromEntryToCompare != secondsFromTimerToCompare) &&
        (secondsFromEntryToCompare != (secondsFromTimerToCompare + 1)) &&
        ((secondsFromEntryToCompare + 1) != secondsFromTimerToCompare) )
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(secondsFromEntryToCompare,
                                     secondsFromTimerToCompare,
                                     "ipfix timestamp too different then timer");
    }

    /* check IPFIX timestamp after read with reset flag enabled */
    rc = prvTgfIpfixEntryGet(prvTgfDevNum, testedStage,
                             ipfixIndex, GT_FALSE, &ipfixEntry);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d, %d",
                                 prvTgfDevNum, testedStage, ipfixIndex);

    secondsFromEntryToCompare = ((ipfixEntry.timeStamp & 0xFF00 ) >> 8);

    secondsFromTimerToCompare = ((timer.secondTimer.l[0] + 5) & 0xFF);

    if( (secondsFromEntryToCompare != secondsFromTimerToCompare) &&
        (secondsFromEntryToCompare != (secondsFromTimerToCompare + 1)) &&
        ((secondsFromEntryToCompare + 1) != secondsFromTimerToCompare) )
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(secondsFromEntryToCompare,
                                     secondsFromTimerToCompare,
                                     "ipfix timestamp after reset too different then timer");
    }

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}

/**
* @internal prvTgfIpfixIngressTestRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfIpfixIngressTestRestore
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       ruleIndex;
    GT_U32       i;
    CPSS_PCL_RULE_SIZE_ENT  ruleSize;
    PRV_TGF_POLICER_MEMORY_STC memoryCfg;

    cpssOsMemSet(&memoryCfg, 0, sizeof(PRV_TGF_POLICER_MEMORY_STC));
    prvTgfPayloadDataArr[0] = 0;
    prvTgfPayloadDataArr[1] = 0;

    if(GT_FALSE ==  prvTgfPolicerStageCheck(testedStage))
    {
        return ;
    }

    ruleSize = CPSS_PCL_RULE_SIZE_STD_E;

    prvTgfIpfixStagesParamsRestore();

    for( i = 0 ; i < 5 ; i++ )
    {
        ruleIndex = basicTestConf[i].ruleIndex;

        rc = prvTgfPclRuleValidStatusSet(ruleSize, ruleIndex, GT_FALSE);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                     ruleSize, ruleIndex, GT_FALSE);
    }

    /* Disables ingress policy for port 8 */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 GT_FALSE);

    /* Disables PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);

    if(PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum))
    {
        memoryCfg.numMeteringEntries[PRV_TGF_POLICER_STAGE_INGRESS_1_E] =  128;
        memoryCfg.numMeteringEntries[PRV_TGF_POLICER_STAGE_EGRESS_E]    =  _1K;
        memoryCfg.numMeteringEntries[PRV_TGF_POLICER_STAGE_INGRESS_0_E] =  PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning.tableSize.policersNum -
        (memoryCfg.numMeteringEntries[PRV_TGF_POLICER_STAGE_INGRESS_1_E] + memoryCfg.numMeteringEntries[PRV_TGF_POLICER_STAGE_EGRESS_E]);

        memoryCfg.numCountingEntries[PRV_TGF_POLICER_STAGE_INGRESS_0_E] =  memoryCfg.numMeteringEntries[PRV_TGF_POLICER_STAGE_INGRESS_0_E];
        memoryCfg.numCountingEntries[PRV_TGF_POLICER_STAGE_INGRESS_1_E] =  memoryCfg.numMeteringEntries[PRV_TGF_POLICER_STAGE_INGRESS_1_E];
        memoryCfg.numCountingEntries[PRV_TGF_POLICER_STAGE_EGRESS_E]    =  memoryCfg.numMeteringEntries[PRV_TGF_POLICER_STAGE_EGRESS_E];

        rc = prvTgfPolicerMemorySizeSet(prvTgfDevNum,
                                        &memoryCfg);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerMemorySizeSet: %d",
                                     prvTgfDevNum);
    }
    else
    {
        rc = prvTgfPolicerMemorySizeModeSet(prvTgfDevNum,
                                            PRV_TGF_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_PLR1_LOWER_E,
                                            0,0);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPolicerMemorySizeModeSet: %d, %d",
                                     prvTgfDevNum,
                                     PRV_TGF_POLICER_MEMORY_CTRL_MODE_PLR0_UPPER_PLR1_LOWER_E);
    }

    prvTgfIpfixVlanRestore(PRV_TGF_VLANID_CNS, localPortsVlanMembers);
}

/**
* @internal prvTgfIpfixFirstNPacketsTestRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfIpfixFirstNPacketsTestRestore
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       ruleIndex;
    GT_U32       i;
    CPSS_PCL_RULE_SIZE_ENT  ruleSize;
    PRV_TGF_POLICER_MEMORY_STC memoryCfg;

    cpssOsMemSet(&memoryCfg, 0, sizeof(PRV_TGF_POLICER_MEMORY_STC));
    prvTgfPayloadDataArr[0] = 0;
    prvTgfPayloadDataArr[1] = 0;

    if(GT_FALSE ==  prvTgfPolicerStageCheck(testedStage))
    {
        return ;
    }

    ruleSize = CPSS_PCL_RULE_SIZE_STD_E;

    prvTgfIpfixStagesParamsRestore();

    for( i = 0 ; i < 5 ; i++ )
    {
        ruleIndex = basicTestConf[i].ruleIndex;

        rc = prvTgfPclRuleValidStatusSet(ruleSize, ruleIndex, GT_FALSE);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                     ruleSize, ruleIndex, GT_FALSE);
    }

    /* Disables ingress policy for port 8 */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 GT_FALSE);

    /* Disables PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);

    rc = prvTgfPolicerMemorySizeSet(prvTgfDevNum,
                                    &memoryCfgGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerMemorySizeSet: %d",
                                 prvTgfDevNum);

    prvTgfIpfixVlanRestore(PRV_TGF_VLANID_CNS, localPortsVlanMembers);
}

/**
* @internal  function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfIpfixEnableIpclEpclTestRestore
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    CPSS_PCL_RULE_SIZE_ENT  ruleSize;
    PRV_TGF_POLICER_MEMORY_STC memoryCfg;

    cpssOsMemSet(&memoryCfg, 0, sizeof(PRV_TGF_POLICER_MEMORY_STC));
    prvTgfPayloadDataArr[0] = 0;
    prvTgfPayloadDataArr[1] = 0;

    if(GT_FALSE ==  prvTgfPolicerStageCheck(testedStage))
    {
        return ;
    }

    ruleSize = CPSS_PCL_RULE_SIZE_STD_E;

    prvTgfIpfixStagesParamsRestore();

    rc = prvTgfPclRuleValidStatusSet(ruleSize, prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(0,1), GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 ruleSize, prvTgfIpclTcamAbsoluteIndexWithinTheLookupGet(0,1), GT_FALSE);

    rc = prvTgfPclRuleValidStatusSet(ruleSize, prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(1), GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 ruleSize, prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(1), GT_FALSE);

    /* Disables ingress policy for port 8 */
    rc = prvTgfPclPortIngressPolicyEnable(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclPortIngressPolicyEnable: %d, %d",
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 GT_FALSE);

    /* Disables PCL ingress Policy */
    rc = prvTgfPclIngressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);

    /* Disables PCL egress Policy */
    rc = prvTgfPclEgressPolicyEnable(GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclIngressPolicyEnable: %d", GT_FALSE);

    /* Restore First N Packet Configurations */
    rc = prvTgfIpfixFirstPacketsMirrorEnableSet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E,
                                                eplrIpfixIndex, enableRestore);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixFirstPacketsMirrorEnableSet: %d ipfixIndex %d",
                                 prvTgfDevNum, eplrIpfixIndex);

    rc = prvTgfIpfixFirstPacketConfigSet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E, &firstPktCfgRestore);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixFirstPacketConfigSet: pktCmd %d cpuCode %d",
                                 firstPktCfgRestore.packetCmd, firstPktCfgRestore.cpuCode);

    rc = prvTgfPolicerMemorySizeSet(prvTgfDevNum,
                                    &memoryCfgGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerMemorySizeSet: %d",
                                 prvTgfDevNum);

    prvTgfPclPortsRestoreAll();

    prvTgfIpfixVlanRestore(PRV_TGF_VLANID_CNS, localPortsVlanMembers);

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

/**
* @internal prvTgfIpfixTrafficGenManager function
* @endinternal
*
* @brief   Perform the test traffic generation and checks for all port groups
*         (unaware mode) and again for first core.
*/
GT_VOID prvTgfIpfixTrafficGenManager
(
    GT_VOID (*initFuncPtr)(GT_VOID),
    GT_VOID (*trafficGenFuncPtr)(GT_VOID),
    GT_VOID (*restoreFuncPtr)(GT_VOID)
)
{
    initFuncPtr();
    trafficGenFuncPtr();
    restoreFuncPtr();

    /* set <currPortGroupsBmp> , <usePortGroupsBmp> */
    TGF_SET_CURRPORT_GROUPS_BMP_WITH_PORT_GROUP_OF_PORT_MAC(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    initFuncPtr();
    trafficGenFuncPtr();
    restoreFuncPtr();

    /* restore PortGroupsBmp mode */
    usePortGroupsBmp  = GT_FALSE;
    currPortGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
}

