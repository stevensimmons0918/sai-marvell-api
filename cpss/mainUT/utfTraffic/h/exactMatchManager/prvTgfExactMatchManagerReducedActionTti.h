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
* @file prvTgfExactMatchManagerReducedActionTti.h
*
* @brief Test Exact Match functionality with TTI Action Type and Reduced entry
*        configuration
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfExactMatchManagerReducedActionTtih
#define __prvTgfExactMatchManagerReducedActionTtih

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfExactMatchGen.h>

/**
* @internal prvTgfExactMatchManagerReducedActionTtiManagerCreate
*           function
* @endinternal
*
* @brief   Create Exact Match Manager with device
* @param[in] exactMatchManagerId - manager Id 
* @param[in] isReduced - GT_FALSE:  take CPU code and command
*                                   from Expander configuration
*                        GT_TRUE:   take CPU code and command
*                                   from Reduced entry configuration 
*
*/
GT_VOID prvTgfExactMatchManagerReducedActionTtiManagerCreate
(
    IN GT_U32                               exactMatchManagerId,
    IN GT_BOOL                              isReduced
);

/**
* @internal prvTgfExactMatchReducedActionTtiConfigSet function
* @endinternal
*
* @brief   Set TTI test configuration related to Exact Match Expanded Action
* @param[in] exactMatchManagerId - manager Id 
* @param[in] isReduced - GT_FALSE:  take CPU code and command
*                                   from Expander configuration
*                        GT_TRUE:   take CPU code and command
*                                   from Reduced entry configuration  
*
*/
GT_VOID prvTgfExactMatchManagerReducedActionTtiConfigSet
(
    IN GT_U32                               exactMatchManagerId,
    IN GT_BOOL                              isReduced
);

/**
* @internal prvTgfExactMatchManagerReducedActionTtiManagerDelete
*           function
* @endinternal
*
* @brief   Delete Exact Match Manager
* @param[in] exactMatchManagerId - manager Id
*
*/
GT_VOID prvTgfExactMatchManagerReducedActionTtiManagerDelete
(
    IN GT_U32                               exactMatchManagerId
);

/**
* @internal prvTgfExactMatcManagerhReducedActionTtiDeleteEmEntry function
* @endinternal
*
* @brief   Delete Exact Match Entry from manager
* @param[in] exactMatchManagerId - manager Id
*
*/
GT_VOID prvTgfExactMatcManagerhReducedActionTtiDeleteEmEntry
(
    IN GT_U32                               exactMatchManagerId
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
