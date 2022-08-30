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
* @file prvCpssCommonSystemRecoveryParallel.h
*
* @brief private CPSS Common system recovery date.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssCommonSystemRecoveryParallelh
#define __prvCpssCommonSystemRecoveryParallelh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/common/systemRecovery/private/prvCpssCommonSystemRecovery.h>

/* macro to get manager status */
#define PRV_SYSTEM_RECOVERY_MANAGER_STATUS_MAC(manager)  (PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_MANAGERS_DB.managersStatusArray[manager])

#define SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(_manager)     \
        ((PRV_SYSTEM_RECOVERY_MANAGER_STATUS_MAC(CPSS_SYSTEM_RECOVERY_NO_MANAGERS_E).recoveryDone == GT_TRUE) && \
         (PRV_SYSTEM_RECOVERY_MANAGER_STATUS_MAC(_manager).recoveryStarted == GT_TRUE) && \
         (PRV_SYSTEM_RECOVERY_MANAGER_STATUS_MAC(_manager).recoveryDone == GT_FALSE)) ? GT_TRUE:GT_FALSE
/*******************************************************************************
* PRV_CPSS_SYSTEM_RECOVERY_PARALLEL_COMPLETION_FUN
*
* DESCRIPTION:
*       Perform synchronization of hardware data and software DB after special init sequence for selected manager..
*
* APPLICABLE DEVICES:
*          Falcon; AC5P; AC5X; Harrier; Ironman.
*
* NOT APPLICABLE DEVICES:
*       Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* INPUTS:
*       manager - selected manager.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*PRV_CPSS_SYSTEM_RECOVERY_PARALLEL_COMPLETION_FUN)
(
 IN CPSS_SYSTEM_RECOVERY_MANAGER_ENT manager
);

/**
* @struct CPSS_SYSTEM_RECOVERY_INFO_STC
 *
 * @brief This struct containes information about system recovery process.
*/

/**
* @struct PRV_CPSS_SYSTEM_RECOVERY_MANAGER_STATUS_STC
 *
 * @brief This struct containes information about system recovery status for each manager.
*/
typedef struct{

    /** Indicates manger started replay and blocked for HW writes. */
    GT_BOOL recoveryStarted;

    /** Indicates manger finished CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E */
    GT_BOOL recoveryDone;

} PRV_CPSS_SYSTEM_RECOVERY_MANAGER_STATUS_STC;

/**
* @struct PRV_CPSS_SYSTEM_RECOVERY_MANAGER_STATUS_STC
 *
 * @brief This struct containes information about system recovery managers status (used for parallel HA).
*/
typedef struct{

     /** containes information about system recovery status for each manager*/
    PRV_CPSS_SYSTEM_RECOVERY_MANAGER_STATUS_STC managersStatusArray[CPSS_SYSTEM_RECOVERY_LAST_MANAGER_E];
    /** containes information about system recovery error Injection DB*/
    CPSS_SYSTEM_RECOVERY_ERROR_INJECTION_STC errorInjection ;

} PRV_CPSS_SYSTEM_RECOVERY_MANAGER_STC;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssCommonSystemRecoveryParallelh */


