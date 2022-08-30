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
* @file  gtOsGlobalDbEnablerInitVars.h
*
* @brief This file define cpssEnabler module global data base  initialization functions
*
* @version  1
********************************************************************************
*/

#ifndef _gtOsGlobalDbEnablerInitVars
#define _gtOsGlobalDbEnablerInitVars

/**
 * @internal osHelperGlobalSharedDbEnablerModDataSectionInit function
 * @endinternal
 *
 * @brief   Initialize global shared variables used in :
 *          module:cpssEnabler
 *
 */
GT_VOID osHelperGlobalSharedDbEnablerModDataSectionInit
(
    GT_VOID
);


/**
 * @internal osHelperNonGlobalSharedDbEnablerModDataSectionInit function
 * @endinternal
 *
 * @brief   Initialize global non-shared variables used in :
 *          module:cpssEnabler
 *
 */
GT_VOID osHelperGlobalNonSharedDbEnablerModDataSectionInit
(
    GT_VOID
);


#endif /* _gtOsGlobalDbEnablerInitVars */


