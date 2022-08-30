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
* @file osGlobalNonSharedEnablerInitVars.c
*
* @brief This file  Initialize global non-shared variables used in  module:cpssEnabler
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/common/cpssHwInit/cpssHwInit.h>
#include <gtOs/globalDb/gtOsGlobalDb.h>
#include <gtOs/globalDb/gtOsGlobalDbInterface.h>




/*global variables macros*/

#define PRV_NON_SHARED_MAIN_OS_DIR_OS_LINUX_MEM_SRC_GLOBAL_VAR_INIT(_var,_value)\
    PRV_HELPER_NON_SHARED_GLOBAL_VAR_SET(cpssEnablerMod.mainOsDir.osLinuxMemSrc._var,_value)

/**
 * @internal osGlobalNonSharedDbEnablerModMainOsDirOsLinuxMemSrcDataSectionInit function
 * @endinternal
 *
 * @brief   Initialize global non-shared variables used in :
 *          module:mainPpDrv
 *          dir:cpssHwInit
 *          src:osLinuxMem
 *
 */
static GT_VOID osGlobalNonSharedDbEnablerModMainOsDirOsLinuxMemSrcDataSectionInit
(
    GT_VOID
)
{
    /*PRV_NON_SHARED_MAIN_OS_DIR_OS_LINUX_MEM_SRC_GLOBAL_VAR_INIT(dummy,1);*/
}


/**
 * @internal osHelperGlobalNonSharedDbEnablerModMainOsDirDataSectionInit function
 * @endinternal
 *
 * @brief   Initialize global shared variables used in :
 *          module:mainPpDrv
 *          dir:cpssHwInit
 *
 */
static GT_VOID osHelperGlobalNonSharedDbEnablerModMainOsDirDataSectionInit
(
    GT_VOID
)
{
     /*add here source files   initializers*/
    osGlobalNonSharedDbEnablerModMainOsDirOsLinuxMemSrcDataSectionInit();
}


/**
 * @internal osHelperGlobalNonSharedDbEnablerModDataSectionInit function
 * @endinternal
 *
 * @brief   Initialize global shared variables used in :
 *          module:cpssEnabler
 *
 */
GT_VOID osHelperGlobalNonSharedDbEnablerModDataSectionInit
(
    GT_VOID
)
{
    /*add here directory  initializers*/
    osHelperGlobalNonSharedDbEnablerModMainOsDirDataSectionInit();
}




