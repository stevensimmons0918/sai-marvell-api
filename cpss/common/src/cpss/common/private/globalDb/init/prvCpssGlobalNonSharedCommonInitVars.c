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
* @file prvCpssGlobalNonSharedCommonInitVars.c
*
* @brief This file  Initialize global non shared variables used in  module:common
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/common/cpssHwInit/cpssHwInit.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


/**
 * @internal cpssGlobalNonSharedDbCommonModSysRecoveryDirSysRecoverySrcDataSectionInit function
 * @endinternal
 *
 * @brief   Initialize global non-shared variables used in :
 *          module:common
 *          dir:systemRecovery
 *          src:cpssGenSystemRecovery.c
 *
 */
static GT_VOID cpssGlobalNonSharedDbCommonModSysRecoveryDirSysRecoverySrcDataSectionInit
(
    GT_VOID
)
{
   CPSS_SYSTEM_RECOVERY_INFO_STC                     systemRecoveryInfo =
    {
     CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E,
     {
        GT_FALSE,
        GT_FALSE,
        GT_FALSE,
        GT_FALSE,
        GT_FALSE,
        CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_NONE_E,
        CPSS_SYSTEM_RECOVERY_HA_STATE_READ_DISABLE_WRITE_DISABLE_E
     },
     CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E
    };
   CPSS_SYSTEM_RECOVERY_INFO_STC * systemRecoveryInfoPtr;

   systemRecoveryInfoPtr=
    &(PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO);

   cpssOsMemCpy(systemRecoveryInfoPtr,&systemRecoveryInfo,sizeof(CPSS_SYSTEM_RECOVERY_INFO_STC));
}


/**
 * @internal cpssGlobalNonSharedDbMainPpDrvModHwInitDirDataSectionInit function
 * @endinternal
 *
 * @brief   Initialize global non shared variables used in :
 *          module:common
 *          dir:systemRecovery
 *
 */
static GT_VOID cpssGlobalNonSharedDbCommonModSysRecoveryDirDataSectionInit
(
    GT_VOID
)
{
     /*add here source files   initializers*/
    cpssGlobalNonSharedDbCommonModSysRecoveryDirSysRecoverySrcDataSectionInit();
}


/**
 * @internal cpssGlobalSharedDbCommonModHwInitDirInit function
 * @endinternal
 *
 * @brief   Initialize global shared variables of hwInit used in :
 *          module:common
 *
 */
static GT_VOID cpssGlobalNonSharedDbCommonModHwInitDirInit
(
    GT_VOID
)
{   cpssNonSharedGlobalVarsPtr->commonMod.genericInterrupsDir.genericInterruptsSrc.eventGppIsrConnectFunc =
        (PRV_CPSS_EVENT_GPP_ISR_CONNECT_FUNC)prvCpssGenEventGppIsrConnect;
}


/**
 * @internal cpssGlobalSharedDbMainPpDrvModDataSectionInit function
 * @endinternal
 *
 * @brief  Initialize global non shared variables used in :
 *         module:common
 *
 */
GT_VOID cpssGlobalNonSharedDbCommonModDataSectionInit
(
    GT_VOID
)
{
    /*add here directory  initializers*/
    cpssGlobalNonSharedDbCommonModSysRecoveryDirDataSectionInit();
    cpssGlobalNonSharedDbCommonModHwInitDirInit();
}




