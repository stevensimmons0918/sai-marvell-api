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
* @file prvTgfBrgSrcIdPortOddOnlyFiltering.h
*
* @brief per egress ePort Source ID filtering for odd Source-ID values.
*
* @version   2.
********************************************************************************
*/
#ifndef __prvTgfBrgSrcIdPortOddOnlyFiltering
#define __prvTgfBrgSrcIdPortOddOnlyFiltering

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



/**
* @internal prvTgfBrgSrcIdPortOddOnlyFilteringVConfigurationSet function
* @endinternal
*
* @brief Function save and set test configurations
*
*/
GT_VOID prvTgfBrgSrcIdPortOddOnlyFilteringConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgSrcIdPortOddOnlyFilteringTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic and check counters
*
*/
GT_VOID prvTgfBrgSrcIdPortOddOnlyFilteringTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgSrcIdPortOddOnlyFilteringRestore function
* @endinternal
*
* @brief   Restore test configurations
*
*/
GT_VOID prvTgfBrgSrcIdPortOddOnlyFilteringRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBrgSrcIdPortOddOnlyFiltering */

