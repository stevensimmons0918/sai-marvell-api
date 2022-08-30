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
* @file prvTgfExactMatchManagerBasicDefaultActionPcl.h
*
* @brief Test Exact Match Manager Default Action functionality with PCL Action Type
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfExactMatchManagerBasicDefaultActionPclh
#define __prvTgfExactMatchManagerBasicDefaultActionPclh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfExactMatchManagerGen.h>

/**
* @internal prvTgfExactMatchManagerBasicDefaultActionPclManagerCreate function
* @endinternal
*
* @brief   Create Exact Match Manager with device
* @param[in] exactMatchManagerId - manager Id
*
*/
GT_VOID prvTgfExactMatchManagerBasicDefaultActionPclManagerCreate
(
    IN GT_U32                               exactMatchManagerId
);

/**
* @internal prvTgfExactMatchManagerBasicDefaultActionPclManagerDelete function
* @endinternal
*
* @brief   Delete Exact Match Manager
* @param[in] exactMatchManagerId - manager Id
*
*/
GT_VOID prvTgfExactMatchManagerBasicDefaultActionPclManagerDelete
(
    IN GT_U32                               exactMatchManagerId
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
