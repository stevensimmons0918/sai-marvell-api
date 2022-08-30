/*******************************************************************************
*              (c), Copyright 2013, Marvell International Ltd.                 *
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
* @file prvCpssGlobalCommonInitVars.h
*
* @brief This file define common module global variables initialization function.
*
* @version  1
********************************************************************************
*/

#ifndef _prvCpssGlobalCommonInitVars
#define _prvCpssGlobalCommonInitVars

/**
 * @internal cpssGlobalSharedDbCommonModDataSectionInit function
 * @endinternal
 *
 * @brief   Initialize global shared variables used in :
 *          module:common
 *
 */
GT_VOID cpssGlobalSharedDbCommonModDataSectionInit
(
    GT_VOID
);

/**
 * @internal cpssGlobalSharedDbCommonModDataSectionInit function
 * @endinternal
 *
 * @brief   Initialize global non-shared variables used in :
 *          module:common
 *
 */
GT_VOID cpssGlobalNonSharedDbCommonModDataSectionInit
(
    GT_VOID
);




#endif /* _prvCpssGlobalCommonInitVars */


