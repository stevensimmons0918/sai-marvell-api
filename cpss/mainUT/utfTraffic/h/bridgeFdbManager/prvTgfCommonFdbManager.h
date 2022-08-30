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
* @file prvTgfCommonFdbManager.h
*
* @brief Fdb manager common test case macros
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfCommonFdbManagerh
#define __prvTgfCommonFdbManagerh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

GT_VOID prvTgfBrgFdb_cleanup
(
    GT_BOOL initialClean
);

GT_VOID prvTgfBrgFdbManagerLearningScan_statistics_config
(
    IN GT_U32       fdbManagerId
);


GT_VOID prvTgfBrgFdbManagerLearningScan_statistics_traffic_and_verify
(
    IN GT_U32       fdbManagerId,
    IN GT_BOOL      reducedRun
);


GT_VOID prvTgfBrgFdbManagerLearningScan_statistics_restore
(
    IN GT_U32       fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerLearningScan_config
(
    IN GT_U32       fdbManagerId
);


GT_VOID prvTgfBrgFdbManagerLearningScan_traffic_and_verify
(
    IN GT_U32       fdbManagerId
);


GT_VOID prvTgfBrgFdbManagerLearningScan_restore
(
    IN GT_U32       fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerAgingScan_config
(
    IN GT_U32       fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerAgingScan_traffic_and_verify
(
    IN GT_U32   fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerAgingScan_restore
(
    IN GT_U32       fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerAgingScan_performance_config
(
    IN GT_U32       fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerAgingScan_performance_traffic_and_verify
(
    IN GT_U32   fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerAgingScan_performance_restore
(
    IN GT_U32       fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerDeleteScan_config
(
    IN GT_U32       fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerDeleteScan_traffic_and_verify
(
    IN GT_U32       fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerDeleteScan_restore
(
    IN GT_U32   fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerTransplantScan_config
(
    IN GT_U32       fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerTransplantScan_traffic_and_verify
(
    IN GT_U32   fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerTransplantScan_restore
(
    IN GT_U32       fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerAddEntry_config
(
    IN GT_U32       fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerAddEntry_traffic_and_verify
(
    IN GT_U32   fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerAddEntry_restore
(
    IN GT_U32       fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerAddEntry_cuckoo_config
(
    IN GT_U32       fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerAddEntry_cuckoo_restore
(
    IN GT_U32       fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerAddEntry_cuckoo_traffic
(
    IN GT_U32       fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerHashTest1_test(GT_VOID);


GT_VOID prvTgfBrgFdbManagerPortLearning_movedMacSaCmd
(
    IN GT_U32       fdbManagerId
);

GT_VOID prvTgfBrgFdbManager_clean
(
    GT_VOID
);

GT_VOID prvTgfBrgFdbManagerInvalidAuq_verify_test
(
    GT_VOID
);

/**
* @internal prvTgfBrgFdbManagerLearningScan_statistics_config function
* @endinternal
*
* @brief  test the limits on learning of dynamic UC MAC entries.
*       according to port/trunk/vlan/global limits
*/
GT_VOID prvTgfBrgFdbManagerBasicLimitDynamicUcLearn
(
    IN GT_U32       fdbManagerId
);

/**
* @internal prvTgfBrgFdbManager_hybridShadowTest function
* @endinternal
*
* @brief  test Hybrid shadow type.
*/
GT_VOID prvTgfBrgFdbManager_hybridShadowTest
(
    GT_VOID
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfCommonFdbManagerh */


