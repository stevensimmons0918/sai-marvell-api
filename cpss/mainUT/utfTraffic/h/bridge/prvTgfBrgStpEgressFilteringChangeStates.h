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
* @file prvTgfBrgStpEgressFilteringChangeStates.h
*
* @brief Verify that changing STP state works properly.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfBrgStpEgressFilteringChangeStatesh
#define __prvTgfBrgStpEgressFilteringChangeStatesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfBrgStpEgressFilteringChangeStatesConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgStpEgressFilteringChangeStatesConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgStpEgressFilteringChangeStatesTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgStpEgressFilteringChangeStatesTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgStpEgressFilteringChangeStatesConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgStpEgressFilteringChangeStatesConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBrgStpEgressFilteringChangeStatesh */




