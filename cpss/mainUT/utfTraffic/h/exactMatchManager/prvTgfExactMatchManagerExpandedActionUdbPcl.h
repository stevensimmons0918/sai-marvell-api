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
* @file prvTgfExactMatchManagerExpandedActionUdbPcl.h
*
* @brief Test Exact Match Manager Expanded Action functionality with PCL Action Type
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfExactMatchManagerExpandedActionUdbPclh
#define __prvTgfExactMatchManagerExpandedActionUdbPclh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfExactMatchGen.h>


/**
* @internal prvTgfExactMatchManagerExpandedActionUdbPclManagerCreate function
* @endinternal
*
* @brief   Create Exact Match Manager with device
* @param[in] exactMatchManagerId - manager Id
*
*/
GT_VOID prvTgfExactMatchManagerExpandedActionUdbPclManagerCreate
(
    IN GT_U32                               exactMatchManagerId
);

/**
* @internal prvTgfExactMatchManagerExpandedActionUdbPclConfigSet function
* @endinternal
*
* @brief   Set PCL test configuration related to Exact Match Expanded Action
* @param[in] exactMatchManagerId - manager Id
*
*/
GT_VOID prvTgfExactMatchManagerExpandedActionUdbPclConfigSet
(
    IN GT_U32                               exactMatchManagerId
);

/**
* @internal prvTgfExactMatchManagerExpandedActionUdbPclManagerDelete function
* @endinternal
*
* @brief   Delete Exact Match Manager
* @param[in] exactMatchManagerId - manager Id
*
*/
GT_VOID prvTgfExactMatchManagerExpandedActionUdbPclManagerDelete
(
    IN GT_U32                               exactMatchManagerId
);

/**
* @internal prvTgfExactMatchManagerExpandedActionUdbPclInvalidateEmEntry function
* @endinternal
*
* @brief   Delete Exact Match Entry from manager
* @param[in] exactMatchManagerId - manager Id
*
*/
GT_VOID prvTgfExactMatchManagerExpandedActionUdbPclInvalidateEmEntry
(
    IN GT_U32                               exactMatchManagerId
);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
