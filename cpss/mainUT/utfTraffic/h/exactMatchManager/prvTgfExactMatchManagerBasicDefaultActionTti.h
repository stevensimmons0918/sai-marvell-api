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
* @file prvTgfExactMatchManagerBasicDefaultActionTti.h
*
* @brief Test Exact Match Manager Default Action functionality with TTI Action Type
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfExactMatchManagerBasicDefaultActionTtih
#define __prvTgfExactMatchManagerBasicDefaultActionTtih

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfExactMatchManagerGen.h>

/**
* @internal prvTgfExactMatchManagerBasicDefaultActionTtiManagerCreate
*           function
* @endinternal
*
* @brief   Create Exact Match Manager with device
* @param[in] exactMatchManagerId - manager Id 
* @param[in] isTrap - GT_TRUE: configure trap parameters 
*                     GT_FALSE: configure drop parameters  
*
*/
GT_VOID prvTgfExactMatchManagerBasicDefaultActionTtiManagerCreate
(
    IN GT_U32                               exactMatchManagerId,
    IN GT_BOOL                              isTrap
);

/**
* @internal prvTgfExactMatchManagerBasicDefaultActionTtiManagerDelete
*           function
* @endinternal
*
* @brief   Delete Exact Match Manager
* @param[in] exactMatchManagerId - manager Id
*
*/
GT_VOID prvTgfExactMatchManagerBasicDefaultActionTtiManagerDelete
(
    IN GT_U32                               exactMatchManagerId
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
