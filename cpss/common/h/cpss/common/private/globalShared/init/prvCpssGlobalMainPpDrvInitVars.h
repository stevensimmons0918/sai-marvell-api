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
* @file prvCpssGlobalMainPpDrvInitVars.h
*
* @brief This file define mainPpDrv module global variables initialization function.
*
* @version  1
********************************************************************************
*/

#ifndef _prvCpssGlobalMainPpDrvInitVars
#define _prvCpssGlobalMainPpDrvInitVars

/**
 * @internal cpssGlobalSharedDbMainPpDrvModDataSectionInit function
 * @endinternal
 *
 * @brief   Initialize global shared variables used in :
 *          module:mainPpDrv
 *
 */
GT_VOID cpssGlobalSharedDbMainPpDrvModDataSectionInit
(
    GT_VOID
);


/**
 * @internal cpssGlobalSharedDbMainPpDrvModDataSectionInit function
 * @endinternal
 *
 * @brief  Initialize global non shared variables used in :
 *         module:mainPpDrv
 *
 */
GT_VOID cpssGlobalNonSharedDbMainPpDrvModDataSectionInit
(
    GT_VOID
);


#endif /* _prvCpssGlobalMainPpDrvInitVars */


