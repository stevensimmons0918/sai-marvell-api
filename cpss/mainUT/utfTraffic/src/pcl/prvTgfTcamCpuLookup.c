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
* @file prvTgfTcamCpuLookup.c
*
* @brief
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
#include <common/tgfBridgeGen.h>
#include <common/tgfPclGen.h>
#include <common/tgfTcamGen.h>

#include <pcl/prvTgfTcamCpuLookup.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/


/************************* General packet's parts *****************************/
static GT_U32 prvTgfDataArr50[] = {0x03020100, 0x07060504, 0x0b0a0908, 0x0f0e0d0c, 0x13121110, 0x17161514, 0x1b1a1918, 0x1f1e1d1c, 0x23222120, 0x27262524, 0x2b2a2928, 0x2f2e2d2c, 0x33323130};

/******************************************************************************/

/*******************************************************************************/
/**
* @internal prvTgfTcamCpuLookupTestPclRulesAndActionsSet function
* @endinternal
*
* @brief   Set PCL rules and actions
*/
static GT_VOID prvTgfTcamCpuLookupTestPclRulesAndActionsSet
(
    IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleType,
    IN GT_U32                           udbMask,
    IN GT_U32                           ruleIndex
)
{
    GT_STATUS rc;
    PRV_TGF_PCL_RULE_FORMAT_UNT         mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT         pattern;
    PRV_TGF_PCL_ACTION_STC              action;
    GT_U32                              udbNum, ii;

    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));

    switch (ruleType)
    {
        case PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E:
            udbNum = 10;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_30_E:
            udbNum = 30;
            break;
        case PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_50_E:
            udbNum = 50;
            break;
        default:
            udbNum = 50;
            break;
    }

    for (ii = 0; ii < udbNum; ii++)
    {
        mask.ruleEgrUdbOnly.udb[ii]    = (GT_U8)udbMask;
        pattern.ruleEgrUdbOnly.udb[ii] = (GT_U8)ii;
    }

    /* AUTODOC: EPCL action - drop the packet */
    action.bypassIngressPipe = GT_TRUE;
    action.bypassBridge = GT_TRUE;
    action.egressPolicy = GT_TRUE;
    action.pktCmd       = CPSS_PACKET_CMD_DROP_HARD_E;

    rc = prvTgfPclRuleSet(ruleType, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d", ruleType,
                                 ruleIndex);

}
/**
* @internal prvTgfTcamCpuLookupTestConfigurationSet function
* @endinternal
*
* @brief   initial configurations
*/
static GT_VOID prvTgfTcamCpuLookupTestConfigurationSet
(
     IN PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT ruleType,
     IN CPSS_DXCH_TCAM_RULE_SIZE_ENT     tcamRuleSize,
     IN GT_U32                          *prvTgfDataArr,
     IN GT_U32                           mask,
     IN GT_U32                           ruleIndex
)
{
    GT_STATUS   rc;
    GT_U32      group;
    GT_BOOL     enable;

    prvTgfTcamCpuLookupTestPclRulesAndActionsSet(ruleType, mask, ruleIndex);

    rc = cpssDxChTcamPortGroupClientGroupGet(prvTgfDevNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, CPSS_DXCH_TCAM_EPCL_E, &group, &enable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChTcamPortGroupClientGroupGet: %d", CPSS_DXCH_TCAM_EPCL_E);

    rc = prvTgfTcamPortGroupCpuLookupTriggerSet(prvTgfDevNum, 0xFFFFFFFF, group, tcamRuleSize, prvTgfDataArr, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTcamPortGroupCpuLookupTriggerSet: %d", tcamRuleSize);
}
/**
* @internal prvTgfTcamCpuLookupTestConfigurationRestore function
* @endinternal
*
* @brief   Test restore configurations
*/
GT_VOID prvTgfTcamCpuLookupTestConfigurationRestore
(
    IN CPSS_PCL_RULE_SIZE_ENT pclRuleSize,
    IN GT_U32                 ruleIndex
)
{
    GT_STATUS   rc;

    /* AUTODOC: RESTORE CONFIGURATION: */
    rc = prvTgfPclRuleValidStatusSet(pclRuleSize,
                                     ruleIndex,
                                     GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 pclRuleSize,
                                 ruleIndex,
                                 GT_FALSE);

}

/**
* @internal prvTgfTcamCpuLookupTestGenerateTraffic function
* @endinternal
*
* @brief   Test generate traffic and check results
*/
GT_VOID prvTgfTcamCpuLookupTestGenerateTraffic
(
    IN GT_U32                           ruleIndex,
    IN CPSS_DXCH_TCAM_RULE_SIZE_ENT     tcamRuleSize
)
{
    GT_STATUS   rc;
    GT_BOOL     isValid;
    GT_BOOL     isHit;
    GT_U32      hitIndex;
    GT_U32      hitNum;
    GT_U32      group;
    GT_BOOL     enable;
    GT_U32      devRuleIndex;

    rc = cpssDxChTcamPortGroupClientGroupGet(prvTgfDevNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, CPSS_DXCH_TCAM_EPCL_E, &group, &enable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChTcamPortGroupClientGroupGet: %d", CPSS_DXCH_TCAM_EPCL_E);

    devRuleIndex = prvWrAppDxChTcamPclConvertedIndexGet_fromUT(prvTgfDevNum,ruleIndex,tcamRuleSize);

    for(hitNum = 0; hitNum <=3; hitNum++)
    {
        rc = prvTgfTcamCpuLookupResultsGet(prvTgfDevNum, 0xFFFFFFFF, group, hitNum, &isValid, &isHit, &hitIndex);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTcamCpuLookupResultsGet failed");

        PRV_UTF_LOG4_MAC(" CPU Lookup hit [%d] isValid[%d] isHit[%d] hitIndex[%d]\n", hitNum, isValid, isHit, hitIndex);
        if (hitNum == 0)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, isHit, "failed - no hit");
            UTF_VERIFY_EQUAL0_STRING_MAC(devRuleIndex, hitIndex, "failed - hit index");
        }
        else
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_FALSE, isHit, "failed - unexpected hit");
        }
    }
}
/**
* @internal prvTgfTcamCpuLookupTest function
* @endinternal
*
* @brief   cpu lookup trigger test
*/
GT_VOID prvTgfTcamCpuLookupTest
(
    GT_VOID
)
{
    GT_U32  ruleIndex;

    /* Check for ruleSize = 10B */
    ruleIndex = prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(11);

    prvTgfTcamCpuLookupTestConfigurationSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E, CPSS_DXCH_TCAM_RULE_SIZE_10_B_E,
                                            prvTgfDataArr50, 0xFF/*mask*/, ruleIndex);
    prvTgfTcamCpuLookupTestGenerateTraffic(ruleIndex, CPSS_DXCH_TCAM_RULE_SIZE_10_B_E);
    prvTgfTcamCpuLookupTestConfigurationRestore(CPSS_PCL_RULE_SIZE_10_BYTES_E, ruleIndex);

    /* Check for ruleSize = 30B */
    ruleIndex = prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(12);

    prvTgfTcamCpuLookupTestConfigurationSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_30_E, CPSS_DXCH_TCAM_RULE_SIZE_30_B_E,
                                            prvTgfDataArr50, 0xFF/*mask*/, ruleIndex);
    prvTgfTcamCpuLookupTestGenerateTraffic(ruleIndex /*ruleIndex*/,CPSS_DXCH_TCAM_RULE_SIZE_30_B_E);
    prvTgfTcamCpuLookupTestConfigurationRestore(CPSS_PCL_RULE_SIZE_30_BYTES_E, ruleIndex /*ruleIndex*/);

    /* Check for ruleSize = 50B */
    ruleIndex = prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(13);

    prvTgfTcamCpuLookupTestConfigurationSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_50_E, CPSS_DXCH_TCAM_RULE_SIZE_50_B_E,
                                            prvTgfDataArr50, 0xFF/*mask*/, ruleIndex);
    prvTgfTcamCpuLookupTestGenerateTraffic(ruleIndex/*ruleIndex*/,CPSS_DXCH_TCAM_RULE_SIZE_50_B_E);
    prvTgfTcamCpuLookupTestConfigurationRestore(CPSS_PCL_RULE_SIZE_50_BYTES_E, ruleIndex);
}
