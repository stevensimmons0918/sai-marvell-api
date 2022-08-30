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
* @file prvTgfVirtualTcam.h
*
* @brief VirtualTcam test header file
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfVirtualTcam
#define __prvTgfVirtualTcam

#include <trafficEngine/tgfTrafficGenerator.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcam.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfVirtualTcamPriorityMoveRuleTest function
* @endinternal
*
* @brief   Test basic moved rule between priorities (use traffic)-
*         priority mode
*         PURPOSE:
*         1. Check that the moved rule between priorities changed actual priority
*         AUTODOC:
*         1. Creat vTcam
*         2. Set rules with diffirent priorites
*         3. Send packet to hit the rules
*         4. Move rules
*         5. Send and check packets to garantee that moving are correct
* @param[in] useTti                   - GT_TRUE: test TTI TCAM, GT_FALSE: test PCL TCAM
* @param[in] ruleSize                 - Rule size to be tested (currently only 30 and 80 bytes supported)
*                                       None
*/

GT_VOID prvTgfVirtualTcamPriorityMoveRuleTest
(
    GT_BOOL                                         useTti,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT            ruleSize
);

/**
* @internal prvTgfVirtualTcamPriorityMoveRuleUnderTrafficTest function
* @endinternal
*
* @brief   Test basic moved rule between priorities (use traffic)-
*         priority mode
*         PURPOSE:
*         1. Check that the moved rule between priorities changed actual priority
*         AUTODOC:
*         1. Creat vTcam
*         2. Set rules with diffirent priorites
*         3. Generate traffic to hit the rules
*         4. Move rules under traffic
*         5. Stop traffic
*         6. Check the CNC counters
*/

GT_VOID prvTgfVirtualTcamPriorityMoveRuleUnderTrafficTest
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamPriorityMoveRuleConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration function
*         PURPOSE:
*         Restore configuration after prvTgfVirtualTcamPriorityMoveRuleTest
*         AUTODOC:
*         1. Deleting rules
*         2. Deleting vTcam
*         3. Deleting vTcam manager
*         4. restore vLan
* @param[in] cncUsed                  - to include CNC for cheking in
*                                      prvTgfVirtualTcamMoveRulePriorityUnderTrafficTest
*                                       None
*/
GT_VOID prvTgfVirtualTcamPriorityMoveRuleConfigurationRestore
(
    GT_BOOL cncUsed
);


/**
* @internal prvTgfVirtualTcamUpdateActionTest function
* @endinternal
*
* @brief   Test action update under traffic (use traffic)-
*         priority mode
*         PURPOSE:
*         1. Check that update action of rules under traffic not losing packets.
*         AUTODOC:
*         1. Create tcam manager with vtcam with 10 rules
*         2. Make rule #8 with action to count to CNC index 0 (redirect to single egress port)
*         3. Make rule #9 Make SAME content of rule like #8 with action (bypass bridge + flood)
*         4.
*         5. Send continues WS traffic that hit this rule
*         6. Updated the action of rule 8 to count to CNC index 1 (cpssDxChVirtualTcamRuleActionUpdate)
*         a. For simulation do sleep 50 millisec (to allow traffic hit the entry)
*         7. Updated the action of rule 8 to count to CNC index 2
*         8. …
*         9. Updated the action of rule 8 to count to CNC index 511
*         10. Stop the traffic
*         11. Check that the CNC summary of indexes 0..511 is equal to number of packets that egress the device.
*         12. Check that there was no flooding to other ports.
* @param[in] useTti                   - GT_TRUE: test TTI TCAM, GT_FALSE: test PCL TCAM
*                                       None
*/

GT_VOID prvTgfVirtualTcamUpdateActionTest
(
    GT_BOOL                                         useTti
);

/**
* @internal prvTgfVirtualTcamUpdateActionConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration function
*         PURPOSE:
*         Restore configuration after prvTgfVirtualTcamUpdateActionTest
*         AUTODOC:
*         1. Deleting rules
*         2. Deleting vTcam
*         3. Deleting vTcam manager
*         4. restore vLan
*/
GT_VOID prvTgfVirtualTcamUpdateActionConfigurationRestore
(
    GT_VOID
);


/**
* @internal prvTgfVirtualTcamResizeTest function
* @endinternal
*
* @brief   Check that resize under traffic not losing packets.-
*         logical index mode
*         PURPOSE:
*         1. Check that resize under traffic not losing packets.
*         AUTODOC:
*         1. Create vtcam , guaranteed 10 entries (30B rules)
*         2. Write rule#9 as redirect to egresss port A
*         3. Send WS traffic to hit this entry
*         4. Resize vtcam adding 1000 entries from index 0
*         5. Sleep 100 millisec
*         6. Resize vtcam adding 1000 entries from index 0
*         a. Sleep 100 millisec
*         7. Resize vtcam removing 500 entries from index 0
*         a. Sleep 100 millisec
*         8. Repeat 7. three more times (total 2000 removed)
*         9. Check no flooding in the vlan (all egress port A)
* @param[in] ruleSize                 - Rule size to be tested (currently only 30 and 80 bytes supported)
* @param[in] ruleAdditionMethod       - Either priority or logical index method
*                                       None
*/

GT_VOID prvTgfVirtualTcamResizeTest
(
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT            ruleSize,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_ENT ruleAdditionMethod
);

/**
* @internal prvTgfVirtualTcamResizeConfigurationRestore function
* @endinternal
*
* @brief   Restore configuration function
*         PURPOSE:
*         Restore configuration after prvTgfVirtualTcamResizeTest
*         AUTODOC:
*         1. Deleting rules
*         2. Deleting vTcam
*         3. Deleting vTcam manager
*         4. restore vLan
*/
GT_VOID prvTgfVirtualTcamResizeConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamDraggingTest function
* @endinternal
*
*/
GT_VOID prvTgfVirtualTcamDraggingTest
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamDraggingConfigurationRestore function
* @endinternal
*
* @brief   Virtual Tcam Dragging configurations restore
*/
GT_VOID prvTgfVirtualTcamDraggingConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamRulePriorityUpdateTrafficTest function
* @endinternal
*
* @brief   Test checks changing priorities of existing rules (use traffic)-
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
*/
GT_VOID prvTgfVirtualTcamRulePriorityUpdateTrafficTest
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamRulePriorityUpdateConfigurationRestore function
* @endinternal
*
* @brief   Virtual Tcam Update Rules Priority Test configurations restore
*
*/
GT_VOID prvTgfVirtualTcamRulePriorityUpdateConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamMngPerPortGroupPclTrafficTest function
* @endinternal
*
* @brief   Test Virtual Tcam manager per port group PCL Test by traffic.
*
*/
GT_VOID prvTgfVirtualTcamMngPerPortGroupPclTrafficTest
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamMngPerPortGroupPclTrafficConfigurationRestore function
* @endinternal
*
* @brief   Virtual TCAM manager per port group PCL Test configurations restore
*
*/
GT_VOID prvTgfVirtualTcamMngPerPortGroupPclTrafficConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamMngPerPortGroupTtiTrafficTest function
* @endinternal
*
* @brief   Test Virtual Tcam manager per port group TTI Test by traffic.
*
*/
GT_VOID prvTgfVirtualTcamMngPerPortGroupTtiTrafficTest
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamMngPerPortGroupTtiTrafficConfigurationRestore function
* @endinternal
*
* @brief   Virtual TCAM manager per port group TTI Test configurations restore
*
*/
GT_VOID prvTgfVirtualTcamMngPerPortGroupTtiTrafficConfigurationRestore
(
    GT_VOID
);

/*
* @internal prvTgfVirtualTcamRuleSizeCheckHa function
* @endinternal
*
* @brief -  Tests to check vtcam creation/rules addition of different sizes - 10, 20, 30, 40, 50, 60, 80B
* Vary the #rules as 1, 100, 1000
*/

GT_VOID prvTgfVirtualTcamRuleSizeCheckHa
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamMultipleRuleSizeCheckHa_5010B function
* @endinternal
*
* @brief -  Tests to check multiple rules installation - install 256(#rows in a block) 50B rules & then 256 10B rules. Also, do for 1000 rules each
* */

GT_VOID prvTgfVirtualTcamMultipleRuleSizeCheckHa_5010B
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamMultipleRuleSizeCheckHa_4020B function
* @endinternal
*
* @brief -  Tests to check multiple rules installation - install 256(#rows in a block) 40B rules & then 256 20B rules. Also, do for 1000 rules each
* */

GT_VOID prvTgfVirtualTcamMultipleRuleSizeCheckHa_4020B
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamMultipleRuleSizeCheckHa_4010B function
* @endinternal
*
* @brief -  Tests to check multiple rules installation - install 256(#rows in a block) 40B rules & then 256 10B rules. Also, do for 1000 rules each
* */

GT_VOID prvTgfVirtualTcamMultipleRuleSizeCheckHa_4010B
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamMultipleRuleSizeCheckHa_2010B function
* @endinternal
*
* @brief -  Tests to check multiple rules installation - install 256(#rows in a block) 20B rules & then 256 10B rules. Also, do for 1000 rules each
* */

GT_VOID prvTgfVirtualTcamMultipleRuleSizeCheckHa_2010B
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamMultipleRuleSizeCheckHa_802010B function
* @endinternal
*
* @brief -  Tests to check multiple rules installation - install 256(#rows in a block) 80B rules & then 256 20B rules & then 256 10B rules. Also, do for 1000 rules each
* */

GT_VOID prvTgfVirtualTcamMultipleRuleSizeCheckHa_802010B
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamMultipleRuleSizeCheckHa_8040B function
* @endinternal
*
* @brief -  Tests to check multiple rules installation - install 256(#rows in a block) 80B rules & then 256 40B rules. Also, do for 1000 rules each
* */

GT_VOID prvTgfVirtualTcamMultipleRuleSizeCheckHa_8040B
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamNormalRuleAddCheckHa function
* @endinternal
*
* @brief -  Test to install rules to vTcam after HA is complete (normal operation) - should fail for full vTcams & success for vTcams with free space
* */

GT_VOID prvTgfVirtualTcamNormalRuleAddCheckHa
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamSingleHoleStartHa function
* @endinternal
*
* @brief -  Test to install 100 rules for a vTcam with #guranteedrules = 200 & initial 100 ruleIDs not having ruleWrites
* */

GT_VOID prvTgfVirtualTcamSingleHoleStartHa
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamMultipleHolesHa function
* @endinternal
*
* @brief -  Test to Install 100 rules for a vTcam with #guranteedrules = 200 & ruleWrites for ruleIds(30-59, 90-129, 150-179)
*  Holes - (0-30, 60-89, 130-149, 180-199)
* */

GT_VOID prvTgfVirtualTcamMultipleHolesHa
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamMultipleVtcamsMulipleHolesHa function
* @endinternal
*
* @brief -  Test to create 2 vTcams with MultipleHoles in each(MultipleHolesHa for each vTcam)
* */

GT_VOID prvTgfVirtualTcamMultipleVtcamsMulipleHolesHa
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamMultipleVtcams100Ha_Single function
* @endinternal
*
* @brief -  Test to create 100 vtcams of varying size <10 for same rulesize
* */

GT_VOID prvTgfVirtualTcamMultipleVtcams100Ha_Single
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamMultipleVtcams100Ha_Multiple function
* @endinternal
*
* @brief -  Test to create 100 vtcams of varying size <10 for varying rulesize
* */

GT_VOID prvTgfVirtualTcamMultipleVtcams100Ha_Multiple
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamRuleSizeCheckHaPriority function
* @endinternal
*
* @brief -  Tests to check vtcam creation/rules addition of different sizes - 10, 20, 30, 40, 50, 60, 80B
* Vary the #rules as 1, 100, 1000
*/

GT_VOID prvTgfVirtualTcamRuleSizeCheckHaPriority
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamMultipleRuleSizeCheckHa_2RulesPriority function
* @endinternal
*
* @brief -  Tests to check multiple rules installation - install 256(#rows in a block) ruleSize1 rules & then 256 ruleSize2 rules. Also, do for 1000 rules each
**/

GT_VOID prvTgfVirtualTcamMultipleRuleSizeCheckHa_2RulesPriority
(
    GT_U32 ruleSize1,
    GT_U32 ruleSize2
);

/**
* @internal prvTgfVirtualTcamMultipleRuleSizeCheckHa_3RulesPriority function
* @endinternal
*
* @brief -  Tests to check multiple rules installation - install 256(#rows in a block) ruleSize1, ruleSize2, ruleSize3 rules. Also, do for 1000 rules each
**/

GT_VOID prvTgfVirtualTcamMultipleRuleSizeCheckHa_3RulesPriority
(
    GT_U32 ruleSize1,
    GT_U32 ruleSize2,
    GT_U32 ruleSize3
);

/**
* @internal prvTgfVirtualTcamNormalRuleAddCheckHaPriority function
* @endinternal
*
* @brief -  Test to install rules to vTcam after HA is complete (normal operation) - should fail for full vTcams & success for vTcams with free space
**/

GT_VOID prvTgfVirtualTcamNormalRuleAddCheckHaPriority
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamSingleHoleStartHaPriority function
* @endinternal
*
* @brief -  Test to install 100 rules for a vTcam with #guranteedrules = 200 & initial 100 ruleIDs not having ruleWrites
* */

GT_VOID prvTgfVirtualTcamSingleHoleStartHaPriority
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamMultipleHolesHaPriority function
* @endinternal
*
* @brief -  Test to Install 100 rules for a vTcam with #guranteedrules = 200 & ruleWrites for ruleIds(30-59, 90-129, 150-179)
*  Holes - (0-30, 60-89, 130-149, 180-199)
* */

GT_VOID prvTgfVirtualTcamMultipleHolesHaPriority
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamMultipleVtcamsMulipleHolesHaPriority function
* @endinternal
*
* @brief -  Test to create 2 vTcams with MultipleHoles in each(MultipleHolesHa for each vTcam)
* */

GT_VOID prvTgfVirtualTcamMultipleVtcamsMulipleHolesHaPriority
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamMultipleVtcams100Ha_SinglePriority function
* @endinternal
*
* @brief -  Test to create 100 vtcams of varying size <10 for same rulesize
* */

GT_VOID prvTgfVirtualTcamMultipleVtcams100Ha_SinglePriority
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamMultipleVtcams100Ha_MultiplePriority function
* @endinternal
*
* @brief -  Test to create 100 vtcams of varying size <10 for varying rulesize
* */

GT_VOID prvTgfVirtualTcamMultipleVtcams100Ha_MultiplePriority
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamMultipleVtcamsPriorityLogicalMode function
* @endinternal
*
* @brief -  Test to Create 4 vtams - 2 each of Priority & Logical Mode for rulesizes 30 & 60B
* */

GT_VOID prvTgfVirtualTcamMultipleVtcamsPriorityLogicalMode
(
    GT_VOID
);

GT_VOID prvTgfVirtualTcamTrafficTestHa
(
        GT_VOID
);

GT_VOID prvTgfVirtualTcamTrafficTestReplayHa
(
        GT_VOID
);

GT_VOID prvTgfVirtualTcamTrafficTestConfigReplayMatchFirstOccurrenceOnlyHaPositive
(
        GT_VOID
);

GT_VOID prvTgfVirtualTcamTrafficTestConfigReplayMatchFirstOccurrenceOnlyHaNegative
(
        GT_VOID
);

GT_VOID prvTgfVirtualTcamTrafficTestHigherPrioFirstReplayModeNegative
(
        GT_VOID
);

GT_VOID prvTgfVirtualTcamTrafficTestHigherPrioFirstReplayModePositive
(
        GT_VOID
);

/*
 * @internal prvTgfVirtualTcamListsGetHa function
 * @endinternal
 *
 * @brief -  Test to check functionality of the two lists of rules corres. to unmapped and failed rules during config replay
 */
GT_VOID prvTgfVirtualTcamListsGetHa
(
    GT_VOID
);

/*
 * @internal prvTgfVirtualTcamEmptyVtcamsHa function
 * @endinternal
 *
 * @brief -  Test to Create 4 vtams - 2 empty and 2 full with ruleSizes 30 & 60B
 */
GT_VOID prvTgfVirtualTcamEmptyVtcamsHa
(
    GT_VOID
);

/*
 * @internal prvTgfVirtualTcamMultipleBlockHolesHa function
 * @endinternal
 *
 * @brief -  Test to Create 2 vtams  - 1 logical, and then 1 priority vTCAM with holes spanning multiple blocks - 40B & 80B
 */
GT_VOID prvTgfVirtualTcamMultipleBlockHolesHa
(
    GT_VOID
);

/*
 * @internal prvTgfVirtualTcamMultipleBlockHolesTest2Ha function
 * @endinternal
 *
 * @brief -  Test to Create 2 vtams  - 1 priority, 1 logical with holes spanning multiple blocks - 80B & 40B
 */
GT_VOID prvTgfVirtualTcamMultipleBlockHolesTest2Ha
(
    GT_VOID
);

/*
 * @internal prvTgfVirtualTcamSingleRulePriorityHa function
 * @endinternal
 *
 * @brief -  Test to Create 1 priority vtam with a single ruleWrite & then add a rule after HA complete
 */
GT_VOID prvTgfVirtualTcamSingleRulePriorityHa
(
    GT_VOID
);

/*
 * @internal prvTgfVirtualTcamAutoResizeHa function
 * @endinternal
 *
 * @brief -  Test to Create 1 priority  autoresized vtam with guaranteedSize 10 & 20 rule adds each
 */
GT_VOID prvTgfVirtualTcamAutoResizeHa
(
    GT_VOID
);

/*
 * @internal prvTgfVirtualTcamZeroRuleHa function
 * @endinternal
 *
 * @brief -  Test to check valid/invalid rules with pattern and mask 0's - valid should be supported, invalid should fail
 */
GT_VOID prvTgfVirtualTcamZeroRuleHa
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestConfigurationSet function
* @endinternal
*
* @brief   Virtual Tcam AC5 TTI IPCL0_0 and IPCL1 test configuration set
*
*/
GT_VOID prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestConfigurationRestore function
* @endinternal
*
* * @brief   Virtual Tcam AC5 TTI IPCL0_0 and IPCL1 test configuration restore
*
*/
GT_VOID prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestConfigurationRestore
(
   GT_VOID
);

/**
* @internal prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestTraffic function
* @endinternal
*
* @brief    Virtual Tcam AC5 TTI IPCL0_0 and IPCL1 test traffic.
*
*/
GT_VOID prvTgfVirtualTcamAc5TtiIpcl00Ipcl1TestTraffic
(
    GT_VOID
);

/**
* @internal prvTgfVirtualTcamHaHwDbLpmAccessBind function
* @endinternal
*
* @brief   Bind LPM memort HW DB access functions to VTCAM Manager.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] vTcamMngId         - VTCAM Manager Id.
*
* @retval GT_OK                    - on success, others on fail.
*/
GT_STATUS prvTgfVirtualTcamHaHwDbLpmAccessBind
(
    IN GT_U32    vTcamMngId
);

/**
* @internal prvTgfVirtualTcamHaHwDbAccessUnbind function
* @endinternal
*
* @brief   Unbind any HW DB access functions from VTCAM Manager.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] vTcamMngId         - VTCAM Manager Id.
*
* @retval GT_OK                    - on success, others on fail.
*/
GT_STATUS prvTgfVirtualTcamHaHwDbAccessUnbind
(
    IN GT_U32    vTcamMngId
);

/* HA tests only callback to call prvTgfVirtualTcamHaHwDbLpmAccessBind/Unbind functions */
/* between VTCAM manager create and Add devices to it                                   */
GT_VOID prvTgfVirtualTcamHaLpmDbPostCreateMgrFuncBind(GT_BOOL toBind);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfVirtualTcam */


