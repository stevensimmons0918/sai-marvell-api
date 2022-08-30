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
* @file prvTgfExactMatchManagerBasicExpandedActionTti.h
*
* @brief Test Exact Match Manager Expanded Action functionality with TTI Action Type
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfExactMatchManagerBasicExpandedActionTtih
#define __prvTgfExactMatchManagerBasicExpandedActionTtih

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfExactMatchGen.h>

/**
* @internal prvTgfExactMatchManagerBasicExpandedActionTtiManagerCreate
*           function
* @endinternal
*
* @brief   Create Exact Match Manager with device
* @param[in] exactMatchManagerId - manager Id
*
*/
GT_VOID prvTgfExactMatchManagerBasicExpandedActionTtiManagerCreate
(
    IN GT_U32                               exactMatchManagerId
);

/**
* @internal prvTgfExactMatchBasicExpandedActionTtiConfigSet function
* @endinternal
*
* @brief   Set TTI test configuration related to Exact Match Expanded Action
* @param[in] exactMatchManagerId - manager Id
*
*/
GT_VOID prvTgfExactMatchManagerBasicExpandedActionTtiConfigSet
(
    IN GT_U32                               exactMatchManagerId
);

/**
* @internal prvTgfExactMatchManagerBasicExpandedActionTtiManagerDelete
*           function
* @endinternal
*
* @brief   Delete Exact Match Manager
* @param[in] exactMatchManagerId - manager Id
*
*/
GT_VOID prvTgfExactMatchManagerBasicExpandedActionTtiManagerDelete
(
    IN GT_U32                               exactMatchManagerId
);

/**
* @internal prvTgfExactMatcManagerhBasicExpandedActionTtiDeleteEmEntry function
* @endinternal
*
* @brief   Delete Exact Match Entry from manager
* @param[in] exactMatchManagerId - manager Id
*
*/
GT_VOID prvTgfExactMatcManagerhBasicExpandedActionTtiDeleteEmEntry
(
    IN GT_U32                               exactMatchManagerId
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
