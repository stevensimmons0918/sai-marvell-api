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
* @file prvTgfBrgUntaggedMruCheck.h
*
* @brief Untagged MRU check - basic
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfBrgUntaggedMruCheckh
#define __prvTgfBrgUntaggedMruCheckh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfBrgUntaggedMruCheckConfigSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgUntaggedMruCheckConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgUntaggedMruCheckTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgUntaggedMruCheckTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgUntaggedMruCommandCheckTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic - to test MRU command
*/
GT_VOID prvTgfBrgUntaggedMruCommandCheckTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgUntaggedMruCheckConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgUntaggedMruCheckConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBrgUntaggedMruCheckh */




