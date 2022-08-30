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
* @file prvCpssCommonSystemRecoveryDrv.h
*
* @brief Includes generic system recovery definitions.
*
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssCommonSystemRecoveryDrvh
#define __prvCpssCommonSystemRecoveryDrvh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>

/* to exit all functions according to system state */
#define SYSTEM_STATE_CAUSED_SKIP_AND_RETURN_MAC(_rc)        \
    do                                                      \
    {                                                       \
        if ( ( (PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E) ||                 \
               (PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO.systemRecoveryState == CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E) )&&         \
             ((PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E) &&         \
              (PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HITLESS_STARTUP_E)) )   \
        {                                                                                                       \
            /* we are in HSU or HA process -- hardware should not be affected */                                \
            return _rc;                                                                                         \
        }                                                                                                       \
    } while (0);

/* to exit all functions according to system state */
#define SYSTEM_STATE_CAUSED_SKIP_MAC  SYSTEM_STATE_CAUSED_SKIP_AND_RETURN_MAC(GT_OK)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssCommonSystemRecoveryDrvh */

