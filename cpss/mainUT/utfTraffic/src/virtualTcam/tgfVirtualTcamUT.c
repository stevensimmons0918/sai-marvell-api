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
* @file tgfVirtualTcamUT.c
*
* @brief Enhanced UTs for Virtual Tcam Init.
*
* @version   2
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>
#include <virtualTcam/prvTgfVirtualTcam.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcam.h>

#include <common/tgfBridgeGen.h>

UTF_TEST_CASE_MAC(prvTgfVirtualTcamPriorityMoveRule)
{
    GT_U32 notAppFamilyBmp;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp & (~(UTF_XCAT3_E | UTF_AC5_E)) & (~UTF_LION2_E)) ;

    prvTgfVirtualTcamPriorityMoveRuleTest(GT_FALSE, CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E);
    prvTgfVirtualTcamPriorityMoveRuleConfigurationRestore(GT_FALSE);

}

UTF_TEST_CASE_MAC(prvTgfVirtualTcamPriorityMoveRule_80B)
{
    GT_U32 notAppFamilyBmp;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~(UTF_XCAT3_E | UTF_AC5_E));
    notAppFamilyBmp &= (~UTF_LION2_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfVirtualTcamPriorityMoveRuleTest(GT_FALSE, CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E);
    prvTgfVirtualTcamPriorityMoveRuleConfigurationRestore(GT_FALSE);

}

UTF_TEST_CASE_MAC(prvTgfVirtualTcamPriorityMoveRule_tti)
{
    GT_U32 notAppFamilyBmp;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp & (~(UTF_XCAT3_E | UTF_AC5_E)) & (~UTF_LION2_E)) ;

    prvTgfVirtualTcamPriorityMoveRuleTest(GT_TRUE, CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E);
    prvTgfVirtualTcamPriorityMoveRuleConfigurationRestore(GT_FALSE);

}

/* AUTODOC: Test - action update under traffic:
    1.  Create tcam manager with vtcam with 10 rules
    2.  Make rule #8 with action to count to CNC index 0 (redirect to single egress port)
    3.  Make rule #9 Make SAME content of rule like #8 with action (bypass bridge + flood)
    4.
    5.  Send continous WS traffic that hit this rule
    6.  Updated the action of rule 8  to count to CNC index 1 (cpssDxChVirtualTcamRuleActionUpdate)
        a.  For simulation do sleep 50 millisec (to allow traffic hit the entry)
    7.  Updated the action of rule 8 to count to CNC index 2
    8.  ..
    9.  Updated the action of rule 8 to count to CNC index 511
    10. Stop the traffic
    11. Check that the CNC summary of indexes 0..511 is equal to number of packets that egress the device.
    12. Check that there was no flooding to other ports.
*/
UTF_TEST_CASE_MAC(prvTgfVirtualTcamUpdateAction)
{
    GT_U32 notAppFamilyBmp;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp & (~(UTF_XCAT3_E | UTF_AC5_E)) & (~UTF_LION2_E)) ;

    prvTgfVirtualTcamUpdateActionTest(GT_FALSE);
    prvTgfVirtualTcamUpdateActionConfigurationRestore();

}

/* AUTODOC: Test - action update under traffic:
    1.  Create tcam manager with vtcam with 10 rules
    2.  Make rule #8 with action to count to CNC index 0 (redirect to single egress port)
    3.  Make rule #9 Make SAME content of rule like #8 with action (bypass bridge + flood)
    4.
    5.  Send continous WS traffic that hit this rule
    6.  Updated the action of rule 8  to count to CNC index 1 (cpssDxChVirtualTcamRuleActionUpdate)
        a.  For simulation do sleep 50 millisec (to allow traffic hit the entry)
    7.  Updated the action of rule 8 to count to CNC index 2
    8.  ..
    9.  Updated the action of rule 8 to count to CNC index 511
    10. Stop the traffic
    11. Check that the CNC summary of indexes 0..511 is equal to number of packets that egress the device.
    12. Check that there was no flooding to other ports.
*/
UTF_TEST_CASE_MAC(prvTgfVirtualTcamUpdateAction_tti)
{
    GT_U32 notAppFamilyBmp;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp & (~(UTF_XCAT3_E | UTF_AC5_E)) & (~UTF_LION2_E)) ;

    prvTgfVirtualTcamUpdateActionTest(GT_TRUE);
    prvTgfVirtualTcamUpdateActionConfigurationRestore();

}

/* AUTODOC: Test - resize under traffic - logical index mode:
    1.  Create vtcam , guaranteed 10 entries (30B rules)
    2.  Write rule#9 as redirect to egresss port A
    3.  Send WS traffic to hit this entry
    4.  Resize vtcam adding 1000 entries from index 0
    5.  Sleep 100 millisec
    6.  Resize vtcam adding 1000 entries from index 0
        a.  Sleep 100 millisec
    7.  Resize vtcam removing 500 entries from index 0
        a.  Sleep 100 millisec
    8.  Repeat 7. three more times (total 2000 removed)
    9.  Check no flooding in the vlan (all egress port A)
*/
UTF_TEST_CASE_MAC(prvTgfVirtualTcamResize)
{
    GT_U32 notAppFamilyBmp;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp & (~(UTF_XCAT3_E | UTF_AC5_E)) & (~UTF_LION2_E)) ;

    prvTgfVirtualTcamResizeTest(CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E,
                                CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E);
    prvTgfVirtualTcamResizeConfigurationRestore();
}

/* AUTODOC: Test - resize under traffic - 80B rules - logical index mode:
    1.  Create vtcam , guaranteed 10 entries (80B rules)
    2.  Write rule#9 as redirect to egresss port A
    3.  Send WS traffic to hit this entry
    4.  Resize vtcam adding 1000 entries from index 0
    5.  Sleep 100 millisec
    6.  Resize vtcam adding 1000 entries from index 0
        a.  Sleep 100 millisec
    7.  Resize vtcam removing 500 entries from index 0
        a.  Sleep 100 millisec
    8.  Repeat 7. three more times (total 2000 removed)
    9.  Check no flooding in the vlan (all egress port A)
*/
UTF_TEST_CASE_MAC(prvTgfVirtualTcamResize_80B)
{
    GT_U32 notAppFamilyBmp;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~(UTF_XCAT3_E | UTF_AC5_E));
    notAppFamilyBmp &= (~UTF_LION2_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp) ;

    prvTgfVirtualTcamResizeTest(CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E,
                                CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E);
    prvTgfVirtualTcamResizeConfigurationRestore();
}

/* AUTODOC: Test - resize under traffic - priority mode:
    1.  Create vtcam , guaranteed 10 entries (30B rules)
    2.  Write rule#9 as redirect to egresss port A
    3.  Send WS traffic to hit this entry
    4.  Resize vtcam adding 1000 entries from index 0
    5.  Sleep 100 millisec
    6.  Resize vtcam adding 1000 entries from index 0
        a.  Sleep 100 millisec
    7.  Resize vtcam removing 500 entries from index 0
        a.  Sleep 100 millisec
    8.  Repeat 7. three more times (total 2000 removed)
    9.  Check no flooding in the vlan (all egress port A)
*/
UTF_TEST_CASE_MAC(prvTgfVirtualTcamResize_PriorityMode)
{
    GT_U32 notAppFamilyBmp;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp & (~(UTF_XCAT3_E | UTF_AC5_E)) & (~UTF_LION2_E)) ;

    prvTgfVirtualTcamResizeTest(CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E,
                                CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E);
    prvTgfVirtualTcamResizeConfigurationRestore();
}

/*******************************************************************************
* prvTgfVirtualTcamPriorityMoveRuleUnderTraffic
*
* DESCRIPTION:
*       Test basic moved rule between priorities (use traffic)-
*       priority mode
* PURPOSE:
*       1. Check that the moved rule between priorities changed actual priority
* AUTODOC:
*       1. Create vTcam
*       2. Set rules with different priorities
*       3. Generate traffic to hit the rules
*       4. Move rules under traffic
*       5. Stop traffic
*       6. Check the CNC counters
*
*******************************************************************************/
UTF_TEST_CASE_MAC(prvTgfVirtualTcamPriorityMoveRuleUnderTraffic)
{
    GT_U32 notAppFamilyBmp;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp & (~(UTF_XCAT3_E | UTF_AC5_E)) & (~UTF_LION2_E)) ;

    ONLY_FWS_SUPPORT_THIS_TEST_MAC;

    prvTgfVirtualTcamPriorityMoveRuleUnderTrafficTest();
    prvTgfVirtualTcamPriorityMoveRuleConfigurationRestore(GT_TRUE);
}

/*******************************************************************************
* prvTgfVirtualTcamDragging
*
* DESCRIPTION:
*       Test dragging vTcam under traffic
*       priority mode
* PURPOSE:
*       1. Check that we can create new vTcam after successfull dragging
*          under the traffic
* AUTODOC:
*       1. Create vTcam #100 to fill all Tcam except 2 Blocks
*       2. Create vTcams #1 #2 #3 #4 #10 #11
*       3. Delete vTcams #1 #2 #3 #4.
*       2. Set rules with different priorities in vTcams #10 #11 that would be dragged
*       3. Generate traffic to hit this rules
*       4. create vTcam #20 after dragging vTcam #10 #11
*       5. Stop traffic
*       6. Check the CNC counters
*
*******************************************************************************/
UTF_TEST_CASE_MAC(prvTgfVirtualTcamDragging)
{
    GT_U32 notAppFamilyBmp;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

     /* asic simulation not support this --> skip */
    ONLY_FWS_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    prvTgfVirtualTcamDraggingTest();
    prvTgfVirtualTcamDraggingConfigurationRestore();
#endif
}

/*******************************************************************************
* prvTgfVirtualTcamRuleUpdatePriority
*
* DESCRIPTION:
*       Test checks changing priorities of existing rules
* PURPOSE:
*       1. Check that the existing rule priority updated correctly.
* AUTODOC:
*         1. Create vTcam, Configure PCL and CNC
*         2. Set 5 rules with priorities 101-105 counting macthes in different CNC counters
*         3. Clean CNC counters
*         4. Generate traffic to hit the rules
*         5. Check and clean CNC counters
*         6. Change priority of each rule to 150 aligning to first.
*         7. Generate traffic to hit the rules
*         8. Check and clean CNC counters
*         9. Change priority of each rule to 50 aligning to last.
*        10. Generate traffic to hit the rules
*        11. Check and clean CNC counters
*
*******************************************************************************/
UTF_TEST_CASE_MAC(prvTgfVirtualTcamRuleUpdatePriority)
{
    GT_U32 notAppFamilyBmp;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp & (~(UTF_XCAT3_E | UTF_AC5_E)) & (~UTF_LION2_E)) ;

    prvTgfVirtualTcamRulePriorityUpdateTrafficTest();
    prvTgfVirtualTcamRulePriorityUpdateConfigurationRestore();
}

/*******************************************************************************
* prvTgfVirtualTcam_Ac5_two_tcams
*
* DESCRIPTION:
*       Test checks rules in TTI_TCAM, PCL_TCAM0 and PCL_TCAM1
*
*******************************************************************************/
UTF_TEST_CASE_MAC(prvTgfVirtualTcam_Ac5_two_tcams)
{

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_AC5_E)) ;

    prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestConfigurationSet();
    prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestTraffic();
    prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestConfigurationRestore();
}

/*******************************************************************************
* prvTgfVirtualTcam_mng_per_port_group_pcl
*
* DESCRIPTION:
*       Test checks rules in same positions of different port groups
* PURPOSE:
*       1. Check that Virtual TCAM manager per port group are independant.
* AUTODOC:
*         1. Create 2 vTcam Managers add different port group of the same device to them.
*         2. Create vTcam, Configure PCL and CNC
*         3. Generate traffic to hit the rules
*         4. Check and clean CNC counters
*
*******************************************************************************/
UTF_TEST_CASE_MAC(prvTgfVirtualTcam_mng_per_port_group_pcl)
{
    GT_U32 boardIdx;
    GT_U32 boardRevId;
    GT_U32 reloadEeprom;

    prvWrAppInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);

    /* skip mixed multidevice boards */
    if((boardIdx == 31) /* Lion2 + BC2 */)
    {
        SKIP_TEST_MAC;
    }

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ (UTF_LION2_E | UTF_FALCON_E))) ;

    prvTgfVirtualTcamMngPerPortGroupPclTrafficTest();
    prvTgfVirtualTcamMngPerPortGroupPclTrafficConfigurationRestore();
}

/*******************************************************************************
* prvTgfVirtualTcam_mng_per_port_group_tti
*
* DESCRIPTION:
*       Test checks rules in same positions of different port groups
* PURPOSE:
*       1. Check that Virtual TCAM manager per port group are independant.
* AUTODOC:
*         1. Create 2 vTcam Managers add different port group of the same device to them.
*         2. Create vTcam, Configure PCL and CNC
*         3. Generate traffic to hit the rules
*         4. Check and clean CNC counters
*
*******************************************************************************/
UTF_TEST_CASE_MAC(prvTgfVirtualTcam_mng_per_port_group_tti)
{
    GT_U32 boardIdx;
    GT_U32 boardRevId;
    GT_U32 reloadEeprom;

    prvWrAppInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);

    /* skip mixed multidevice boards */
    if((boardIdx == 31) /* Lion2 + BC2 */)
    {
        SKIP_TEST_MAC;
    }

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ (UTF_LION2_E | UTF_FALCON_E))) ;

    prvTgfVirtualTcamMngPerPortGroupTtiTrafficTest();
    prvTgfVirtualTcamMngPerPortGroupTtiTrafficConfigurationRestore();
}

/* AUTODOC: Tests to check vtcam creation/rules addition of different sizes - 10, 20, 30, 40, 50, 60, 80B
 * Vary the #rules as 1, 100, 1000
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamRuleSizeCheckHa)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    prvTgfVirtualTcamRuleSizeCheckHa();
}

/* AUTODOC: Tests to check multiple rules installation - install 256(#rows in a block) 50B rules & then 256 10B rules. Also, do for 1000 rules each
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_5010B)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    prvTgfVirtualTcamMultipleRuleSizeCheckHa_5010B();
}

/* AUTODOC: Tests to check multiple rules installation - install 256(#rows in a block) 40B rules & then 256 20B rules. Also, do for 1000 rules each
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_4020B)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    prvTgfVirtualTcamMultipleRuleSizeCheckHa_4020B();
}

/* AUTODOC: Tests to check multiple rules installation - install 256(#rows in a block) 40B rules & then 256 10B rules. Also, do for 1000 rules each
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_4010B)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    prvTgfVirtualTcamMultipleRuleSizeCheckHa_4010B();
}

/* AUTODOC: Tests to check multiple rules installation - install 256(#rows in a block) 20B rules & then 256 10B rules. Also, do for 1000 rules each
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_2010B)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    prvTgfVirtualTcamMultipleRuleSizeCheckHa_2010B();
}

/* AUTODOC: Tests to check multiple rules installation - install 256(#rows in a block) 80B rules & then 256 20B rules & 10B rules. Also, do for 1000 rules each
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_802010B)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    prvTgfVirtualTcamMultipleRuleSizeCheckHa_802010B();
}

/* AUTODOC: Tests to check multiple rules installation - install 256(#rows in a block) 80B rules & then 256 40B rules. Also, do for 1000 rules each
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_8040B)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    prvTgfVirtualTcamMultipleRuleSizeCheckHa_8040B();

}

/* AUTODOC: Tests to check multiple rules installation - install 256(#rows in a block) 80B rules & then 256 40B rules. Also, do for 1000 rules each
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamNormalRuleAddCheckHa)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    prvTgfVirtualTcamNormalRuleAddCheckHa();

}

/* AUTODOC: Tests to install 100 rules for a vTcam with #guranteedrules = 200 & initial 100 ruleIDs not having ruleWrites
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamSingleHoleStartHa)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    prvTgfVirtualTcamSingleHoleStartHa();
}

/* AUTODOC: Tests to Install 100 rules for a vTcam with #guranteedrules = 200 & ruleWrites for ruleIds(30-59, 90-129, 150-179)
 * Holes - (0-30, 60-89, 130-149, 180-199)
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleHolesHa)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    prvTgfVirtualTcamMultipleHolesHa();
}

/* AUTODOC: Tests to create 2 vTcams with MultipleHoles in each(MultipleHolesHa for each vTcam)
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleVtcamsMulipleHolesHa)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    prvTgfVirtualTcamMultipleVtcamsMulipleHolesHa();
}

/* AUTODOC: Tests to create 100 vtcams of varying size <10 for same rulesize
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleVtcams100Ha_Single)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    prvTgfVirtualTcamMultipleVtcams100Ha_Single();
}

/* AUTODOC: Tests to create 100 vtcams of varying size <10 for varying rulesize
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleVtcams100Ha_Multiple)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    prvTgfVirtualTcamMultipleVtcams100Ha_Multiple();
}

/* AUTODOC: Tests to check vtcam creation/rules addition of different sizes - 10, 20, 30, 40, 50, 60, 80B
 * Vary the #rules as 1, 100, 1000
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamRuleSizeCheckHaPriority)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    /* this test is for baseline execution only because it's long */
    PRV_TGF_SKIP_NON_BASELINE_TEST_MAC();

    prvTgfVirtualTcamRuleSizeCheckHaPriority();
}

/* AUTODOC: Tests to check multiple rules installation - install 256(#rows in a block) 50B rules & then 256 10B rules. Also, do for 1000 rules each
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_5010BPriority)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    /* this test is for baseline execution only because it's long */
    PRV_TGF_SKIP_NON_BASELINE_TEST_MAC();

    prvTgfVirtualTcamMultipleRuleSizeCheckHa_2RulesPriority( 4, 0);
}

/* AUTODOC: Tests to check multiple rules installation - install 256(#rows in a block) 40B rules & then 256 20B rules. Also, do for 1000 rules each
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_4020BPriority)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    /* this test is for baseline execution only because it's long */
    PRV_TGF_SKIP_NON_BASELINE_TEST_MAC();

    prvTgfVirtualTcamMultipleRuleSizeCheckHa_2RulesPriority( 3, 1);
}

/* AUTODOC: Tests to check multiple rules installation - install 256(#rows in a block) 40B rules & then 256 10B rules. Also, do for 1000 rules each
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_4010BPriority)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    /* this test is for baseline execution only because it's long */
    PRV_TGF_SKIP_NON_BASELINE_TEST_MAC();

    prvTgfVirtualTcamMultipleRuleSizeCheckHa_2RulesPriority( 3, 0);

}

/* AUTODOC: Tests to check multiple rules installation - install 256(#rows in a block) 20B rules & then 256 10B rules. Also, do for 1000 rules each
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_2010BPriority)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    /* this test is for baseline execution only because it's long */
    PRV_TGF_SKIP_NON_BASELINE_TEST_MAC();

    prvTgfVirtualTcamMultipleRuleSizeCheckHa_2RulesPriority( 1, 0);
}

/* AUTODOC: Tests to check multiple rules installation - install 256(#rows in a block) 80B rules & then 256 20B rules & 10B rules. Also, do for 1000 rules each
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_802010BPriority)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    /* this test is for baseline execution only because it's long */
    PRV_TGF_SKIP_NON_BASELINE_TEST_MAC();

    prvTgfVirtualTcamMultipleRuleSizeCheckHa_3RulesPriority( 7, 1, 0);
}

/* AUTODOC: Tests to check multiple rules installation - install 256(#rows in a block) 80B rules & then 256 40B rules. Also, do for 1000 rules each
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_8040BPriority)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    /* this test is for baseline execution only because it's long */
    PRV_TGF_SKIP_NON_BASELINE_TEST_MAC();

    prvTgfVirtualTcamMultipleRuleSizeCheckHa_2RulesPriority( 7, 3);

}

/* AUTODOC: Tests to check multiple rules installation - install 256(#rows in a block) 80B rules & then 256 40B rules. Also, do for 1000 rules each
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamNormalRuleAddCheckHaPriority)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    prvTgfVirtualTcamNormalRuleAddCheckHaPriority();

}

/* AUTODOC: Tests to install 100 rules for a vTcam with #guranteedrules = 200 & initial 100 ruleIDs not having ruleWrites
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamSingleHoleStartHaPriority)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    prvTgfVirtualTcamSingleHoleStartHaPriority();

}

/* AUTODOC: Tests to Install 100 rules for a vTcam with #guranteedrules = 200 & ruleWrites for ruleIds(30-59, 90-129, 150-179)
 * Holes - (0-30, 60-89, 130-149, 180-199)
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleHolesHaPriority)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    prvTgfVirtualTcamMultipleHolesHaPriority();

}

/* AUTODOC: Tests to create 2 vTcams with MultipleHoles in each(MultipleHolesHa for each vTcam)
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleVtcamsMulipleHolesHaPriority)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    prvTgfVirtualTcamMultipleVtcamsMulipleHolesHaPriority();

}

/* AUTODOC: Tests to create 100 vtcams of varying size <10 for same rulesize
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleVtcams100Ha_SinglePriority)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    prvTgfVirtualTcamMultipleVtcams100Ha_SinglePriority();

}

/* AUTODOC: Tests to create 100 vtcams of varying size <10 for varying rulesize
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleVtcams100Ha_MultiplePriority)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    prvTgfVirtualTcamMultipleVtcams100Ha_MultiplePriority();

}

/* AUTODOC: Test to Create 4 vtams - 2 each of Priority & Logical Mode for rulesizes 30 & 60B
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleVtcamsPriorityLogicalMode)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    prvTgfVirtualTcamMultipleVtcamsPriorityLogicalMode();

}

/* AUTODOC:  Test to check functionality of the two lists of rules corres. to unmapped and failed rules during config replay
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamListsGetHa)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    prvTgfVirtualTcamListsGetHa();
}

/* AUTODOC: Test to Create 4 vtams - 2 empty and 2 full with ruleSizes 30 & 60B
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamEmptyVtcamsHa)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    prvTgfVirtualTcamEmptyVtcamsHa();

}

/* AUTODOC: Test to Create 2 vtams  - 1 logical and then 1 priority vTcam with holes spanning multiple blocks - 40B & 80B
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleBlockHolesHa)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    prvTgfVirtualTcamMultipleBlockHolesHa();

}

/* AUTODOC: Test to Create 2 vtams  - 1 priority, 1 logical with holes spanning multiple blocks - 80B & 40B
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleBlockHolesTest2Ha)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    prvTgfVirtualTcamMultipleBlockHolesTest2Ha();

}

/* AUTODOC: Test to Create 1 priority vtam with a single ruleWrite & then add a rule after HA complete
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamSingleRulePriorityHa)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    prvTgfVirtualTcamSingleRulePriorityHa();

}


/* AUTODOC: Test to Create 1 priority  autoresized vtams with guaranteedSize 10 & 20 rule adds each
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamAutoResizeHa)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    prvTgfVirtualTcamAutoResizeHa();

}

/* AUTODOC: Test to check valid/invalid rules with pattern and mask 0's - valid should be supported, invalid should fail
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamZeroRuleHa)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    prvTgfVirtualTcamZeroRuleHa();

}

/* AUTODOC: Traffic Test for HA
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamTrafficTestHa)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    prvTgfVirtualTcamTrafficTestHa();

}

/* AUTODOC: Traffic Test for failed config-replay rules HA
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamTrafficTestReplayHa)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    prvTgfVirtualTcamTrafficTestReplayHa();

}

/* AUTODOC: Traffic Test for positive case of higher priority rule replay first mode
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamTrafficTestHigherPrioFirstReplayModePositive)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    prvTgfVirtualTcamTrafficTestHigherPrioFirstReplayModePositive();
}

/* AUTODOC: Traffic Test for positive case of higher priority rule replay first mode
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamTrafficTestHigherPrioFirstReplayModeNegative)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    prvTgfVirtualTcamTrafficTestHigherPrioFirstReplayModeNegative();
}

/* AUTODOC: Traffic Test for positive case of match first occurrence only mode (during ha config-replay)
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamTrafficTestConfigReplayMatchFirstOccurrenceOnlyHaPositive)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    prvTgfVirtualTcamTrafficTestConfigReplayMatchFirstOccurrenceOnlyHaPositive();
}

/* AUTODOC: Traffic Test for negative case of match first occurrence only mode (during ha config-replay)
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamTrafficTestConfigReplayMatchFirstOccurrenceOnlyHaNegative)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ (UTF_FALCON_E | UTF_ALDRIN2_E | UTF_AC5X_E)))

    prvTgfVirtualTcamTrafficTestConfigReplayMatchFirstOccurrenceOnlyHaNegative();
}

/* AUTODOC: Tests to check vtcam creation/rules addition of different sizes - 10, 20, 30, 40, 50, 60, 80B
 * Vary the #rules as 1, 100, 1000
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamRuleSizeCheckHa_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamRuleSizeCheckHa();
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Tests to check multiple rules installation - install 256(#rows in a block) 50B rules & then 256 10B rules.
   Also, do for 1000 rules each
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_5010B_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamMultipleRuleSizeCheckHa_5010B();
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Tests to check multiple rules installation - install 256(#rows in a block) 40B rules & then 256 20B rules.
   Also, do for 1000 rules each
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_4020B_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamMultipleRuleSizeCheckHa_4020B();
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Tests to check multiple rules installation - install 256(#rows in a block) 40B rules & then 256 10B rules.
   Also, do for 1000 rules each
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_4010B_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamMultipleRuleSizeCheckHa_4010B();
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Tests to check multiple rules installation - install 256(#rows in a block) 20B rules & then 256 10B rules.
   Also, do for 1000 rules each
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_2010B_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamMultipleRuleSizeCheckHa_2010B();
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Tests to check multiple rules installation - install 256(#rows in a block) 80B rules
   & then 256 20B rules & 10B rules. Also, do for 1000 rules each
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_802010B_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamMultipleRuleSizeCheckHa_802010B();
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Tests to check multiple rules installation - install 256(#rows in a block) 80B rules
   & then 256 40B rules. Also, do for 1000 rules each
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_8040B_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamMultipleRuleSizeCheckHa_8040B();
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Tests to check multiple rules installation - install 256(#rows in a block) 80B rules
   & then 256 40B rules. Also, do for 1000 rules each
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamNormalRuleAddCheckHa_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamNormalRuleAddCheckHa();
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Tests to install 100 rules for a vTcam with #guranteedrules = 200
   & initial 100 ruleIDs not having ruleWrites
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamSingleHoleStartHa_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamSingleHoleStartHa();
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Tests to Install 100 rules for a vTcam with #guranteedrules = 200
 * & ruleWrites for ruleIds(30-59, 90-129, 150-179)
 * Holes - (0-30, 60-89, 130-149, 180-199)
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleHolesHa_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamMultipleHolesHa();
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Tests to create 2 vTcams with MultipleHoles in each(MultipleHolesHa for each vTcam)
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleVtcamsMulipleHolesHa_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamMultipleVtcamsMulipleHolesHa();
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Tests to create 100 vtcams of varying size <10 for same rulesize
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleVtcams100Ha_Single_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamMultipleVtcams100Ha_Single();
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Tests to create 100 vtcams of varying size <10 for varying rulesize
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleVtcams100Ha_Multiple_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamMultipleVtcams100Ha_Multiple();
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Tests to check vtcam creation/rules addition of different sizes - 10, 20, 30, 40, 50, 60, 80B
 * Vary the #rules as 1, 100, 1000
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamRuleSizeCheckHaPriority_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    /* this test is for baseline execution only because it's long */
    PRV_TGF_SKIP_NON_BASELINE_TEST_MAC();

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamRuleSizeCheckHaPriority();
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Tests to check multiple rules installation - install 256(#rows in a block) 50B rules
   & then 256 10B rules. Also, do for 1000 rules each
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_5010BPriority_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    /* this test is for baseline execution only because it's long */
    PRV_TGF_SKIP_NON_BASELINE_TEST_MAC();

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamMultipleRuleSizeCheckHa_2RulesPriority(4, 0);
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Tests to check multiple rules installation - install 256(#rows in a block) 40B rules
   & then 256 20B rules. Also, do for 1000 rules each
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_4020BPriority_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    /* this test is for baseline execution only because it's long */
    PRV_TGF_SKIP_NON_BASELINE_TEST_MAC();

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamMultipleRuleSizeCheckHa_2RulesPriority(3, 1);
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Tests to check multiple rules installation - install 256(#rows in a block) 40B rules
   & then 256 10B rules. Also, do for 1000 rules each
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_4010BPriority_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    /* this test is for baseline execution only because it's long */
    PRV_TGF_SKIP_NON_BASELINE_TEST_MAC();

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamMultipleRuleSizeCheckHa_2RulesPriority(3, 0);
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Tests to check multiple rules installation - install 256(#rows in a block) 20B rules & then 256 10B rules. Also, do for 1000 rules each
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_2010BPriority_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    /* this test is for baseline execution only because it's long */
    PRV_TGF_SKIP_NON_BASELINE_TEST_MAC();

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamMultipleRuleSizeCheckHa_2RulesPriority( 1, 0);
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Tests to check multiple rules installation - install 256(#rows in a block) 80B rules
   & then 256 20B rules & 10B rules. Also, do for 1000 rules each
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_802010BPriority_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    /* this test is for baseline execution only because it's long */
    PRV_TGF_SKIP_NON_BASELINE_TEST_MAC();

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamMultipleRuleSizeCheckHa_3RulesPriority(7, 1, 0);
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Tests to check multiple rules installation - install 256(#rows in a block) 80B rules
   & then 256 40B rules. Also, do for 1000 rules each
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_8040BPriority_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    /* this test is for baseline execution only because it's long */
    PRV_TGF_SKIP_NON_BASELINE_TEST_MAC();

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamMultipleRuleSizeCheckHa_2RulesPriority(7, 3);
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Tests to install 100 rules for a vTcam with #guranteedrules = 200
   & initial 100 ruleIDs not having ruleWrites
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamNormalRuleAddCheckHaPriority_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamNormalRuleAddCheckHaPriority();
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Tests to install 100 rules for a vTcam with #guranteedrules = 200
   & initial 100 ruleIDs not having ruleWrites
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamSingleHoleStartHaPriority_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamSingleHoleStartHaPriority();
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Tests to Install 100 rules for a vTcam with #guranteedrules = 200
 * & ruleWrites for ruleIds(30-59, 90-129, 150-179)
 * Holes - (0-30, 60-89, 130-149, 180-199)
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleHolesHaPriority_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamMultipleHolesHaPriority();
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Tests to create 2 vTcams with MultipleHoles in each(MultipleHolesHa for each vTcam)
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleVtcamsMulipleHolesHaPriority_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamMultipleVtcamsMulipleHolesHaPriority();
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Tests to create 100 vtcams of varying size <10 for same rulesize
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleVtcams100Ha_SinglePriority_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamMultipleVtcams100Ha_SinglePriority();
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Tests to create 100 vtcams of varying size <10 for varying rulesize
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleVtcams100Ha_MultiplePriority_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamMultipleVtcams100Ha_MultiplePriority();
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Test to Create 4 vtams - 2 each of Priority & Logical Mode for rulesizes 30 & 60B
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleVtcamsPriorityLogicalMode_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamMultipleVtcamsPriorityLogicalMode();
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Test to Create 4 vtams - 2 empty and 2 full with ruleSizes 30 & 60B
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamEmptyVtcamsHa_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamEmptyVtcamsHa();
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Test to Create 2 vtams  - 1 logical and then 1 priority vTcam
   with holes spanning multiple blocks - 40B & 80B
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleBlockHolesHa_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamMultipleBlockHolesHa();
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Test to Create 2 vtams  - 1 priority, 1 logical with holes spanning multiple blocks - 80B & 40B
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamMultipleBlockHolesTest2Ha_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamMultipleBlockHolesTest2Ha();
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Test to Create 1 priority vtam with a single ruleWrite & then add a rule after HA complete
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamSingleRulePriorityHa_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamSingleRulePriorityHa();
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Test to Create 1 priority  autoresized vtams with guaranteedSize 10 & 20 rule adds each
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamAutoResizeHa_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamAutoResizeHa();
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Test to check valid/invalid rules with pattern and mask 0's -
   valid should be supported, invalid should fail
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamZeroRuleHa_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamZeroRuleHa();
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC:  Test to check functionality of the two lists of rules corres.
   to unmapped and failed rules during config replay
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamListsGetHa_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamListsGetHa();
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Traffic Test for HA
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamTrafficTestHa_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamTrafficTestHa();
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Traffic Test for failed config-replay rules HA
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamTrafficTestReplayHa_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamTrafficTestReplayHa();
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Traffic Test for positive case of higher priority rule replay first mode
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamTrafficTestHigherPrioFirstReplayModeNegative_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamTrafficTestHigherPrioFirstReplayModeNegative();
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Traffic Test for positive case of higher priority rule replay first mode
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamTrafficTestHigherPrioFirstReplayModePositive_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamTrafficTestHigherPrioFirstReplayModePositive();
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Traffic Test for negative case of match first occurrence only mode (during ha config-replay)
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamTrafficTestConfigReplayMatchFirstOccurrenceOnlyHaNegative_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamTrafficTestConfigReplayMatchFirstOccurrenceOnlyHaNegative();
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/* AUTODOC: Traffic Test for positive case of match first occurrence only mode (during ha config-replay)
 */
UTF_TEST_CASE_MAC(prvTgfVirtualTcamTrafficTestConfigReplayMatchFirstOccurrenceOnlyHaPositive_HwDbLpmMemo)
{
    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_ALDRIN2_E))

    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_TRUE);
    prvTgfVirtualTcamTrafficTestConfigReplayMatchFirstOccurrenceOnlyHaPositive();
    prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_FALSE);
}

/*
 * Configuration of tgfVirtualTcam suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfVirtualTcam)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamPriorityMoveRule)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamPriorityMoveRule_80B)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamPriorityMoveRule_tti)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamPriorityMoveRuleUnderTraffic)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamUpdateAction)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamUpdateAction_tti)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamResize)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamResize_80B)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamResize_PriorityMode)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamDragging)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamRuleUpdatePriority)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcam_Ac5_two_tcams)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcam_mng_per_port_group_pcl)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcam_mng_per_port_group_tti)

    /* HA tests */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamRuleSizeCheckHa)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_5010B)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_4020B)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_4010B)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_2010B)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_802010B)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_8040B)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamNormalRuleAddCheckHa)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamSingleHoleStartHa)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleHolesHa)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleVtcamsMulipleHolesHa)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleVtcams100Ha_Single)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleVtcams100Ha_Multiple)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamRuleSizeCheckHaPriority)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_5010BPriority)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_4020BPriority)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_4010BPriority)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_2010BPriority)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_802010BPriority)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_8040BPriority)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamNormalRuleAddCheckHaPriority)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamSingleHoleStartHaPriority)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleHolesHaPriority)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleVtcamsMulipleHolesHaPriority)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleVtcams100Ha_SinglePriority)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleVtcams100Ha_MultiplePriority)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleVtcamsPriorityLogicalMode)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamEmptyVtcamsHa)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleBlockHolesHa)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleBlockHolesTest2Ha)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamSingleRulePriorityHa)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamAutoResizeHa)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamZeroRuleHa)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamListsGetHa)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamTrafficTestHa)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamTrafficTestReplayHa)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamTrafficTestHigherPrioFirstReplayModeNegative)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamTrafficTestHigherPrioFirstReplayModePositive)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamTrafficTestConfigReplayMatchFirstOccurrenceOnlyHaPositive)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamTrafficTestConfigReplayMatchFirstOccurrenceOnlyHaNegative)

    /* HA tests with DB in LPM memory of Aldrin2 */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamRuleSizeCheckHa_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_5010B_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_4020B_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_4010B_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_2010B_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_802010B_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_8040B_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamNormalRuleAddCheckHa_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamSingleHoleStartHa_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleVtcamsMulipleHolesHa_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleHolesHa_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleVtcams100Ha_Single_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleVtcams100Ha_Multiple_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamRuleSizeCheckHaPriority_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_5010BPriority_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_4020BPriority_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_4010BPriority_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_2010BPriority_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_802010BPriority_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleRuleSizeCheckHa_8040BPriority_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamNormalRuleAddCheckHaPriority_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamSingleHoleStartHaPriority_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleHolesHaPriority_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleVtcamsMulipleHolesHaPriority_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleVtcams100Ha_SinglePriority_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleVtcams100Ha_MultiplePriority_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleVtcamsPriorityLogicalMode_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamEmptyVtcamsHa_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleBlockHolesHa_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamMultipleBlockHolesTest2Ha_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamSingleRulePriorityHa_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamAutoResizeHa_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamZeroRuleHa_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamListsGetHa_HwDbLpmMemo)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamTrafficTestHa_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamTrafficTestReplayHa_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamTrafficTestHigherPrioFirstReplayModeNegative_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamTrafficTestHigherPrioFirstReplayModePositive_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamTrafficTestConfigReplayMatchFirstOccurrenceOnlyHaNegative_HwDbLpmMemo)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtualTcamTrafficTestConfigReplayMatchFirstOccurrenceOnlyHaPositive_HwDbLpmMemo)

UTF_SUIT_END_TESTS_MAC(tgfVirtualTcam)


