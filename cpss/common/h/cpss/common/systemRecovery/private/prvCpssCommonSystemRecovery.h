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
* @file prvCpssCommonSystemRecovery.h
*
* @brief Includes generic system recovery definitions.
*
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssCommonSystemRecoveryh
#define __prvCpssCommonSystemRecoveryh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>

/*number of errorBmps */
#define MAX_ERROR_INJECTION_BMP_SIZE         3

/**
* @struct PRV_CPSS_SYSTEM_RECOVERY_DEBUG_STC
 *
 * @brief Defines sysyem recovery debug DB
*/
typedef struct{

    GT_BOOL hsSkipVersionCheck;         /*skip HS-MI version compatibility check  */
    GT_BOOL hsSkipMiReset;              /*skip MI reset at end of HS process  */
    GT_BOOL hsSkipDfxInitStateCheck;    /*skip HS Dfx init state register check*/

} PRV_CPSS_SYSTEM_RECOVERY_DEBUG_STC;

/**
* @struct CPSS_SYSTEM_RECOVERY_ERROR_INJECTION_BMP_STC
 *
 * @brief Defines sysyem recovery errors DB
*/
typedef struct{

    GT_BOOL errorInjectionEnable;  /* define if errors are set ,in case errorBmp != 0 this bit is on */

    GT_U32 errorBmp[MAX_ERROR_INJECTION_BMP_SIZE]; /* define errors bit map */

} CPSS_SYSTEM_RECOVERY_ERROR_INJECTION_STC;

/**
* @enum PRV_CPSS_SYSYEM_RECOVERY_ERROR_INJECTION_INDEX_ENT
 *
 * @brief Defines error injection index in cpss Code:
*/
typedef enum{

    /** no error injection set */
    PRV_CPSS_SYSYEM_RECOVERY_ERROR_INJECTION_INDEX_NONE_E,      /*0x0*/  /*always empty */
    PRV_CPSS_SYSYEM_RECOVERY_ERROR_INJECTION_INDEX_EM_1_E,      /*0x2*/  /*internal_cpssDxChExactMatchManagerDevListAdd after SW write befor Hw initialization on new device */
    PRV_CPSS_SYSYEM_RECOVERY_ERROR_INJECTION_INDEX_EM_2_E,      /*0x4*/  /*internal_cpssDxChExactMatchManagerDevListAdd after sw DB write & Hw initialization on new device before hw write */
    PRV_CPSS_SYSYEM_RECOVERY_ERROR_INJECTION_INDEX_EM_3_E,      /*0x8*/
    PRV_CPSS_SYSYEM_RECOVERY_ERROR_INJECTION_INDEX_EM_4_E,      /*0x10*/ /*prvCpssDxChExactMatchManagerAddEntry after SW write befor HW write */
    PRV_CPSS_SYSYEM_RECOVERY_ERROR_INJECTION_INDEX_EM_5_E,      /*0x20*/ /*prvCpssDxChExactMatchManagerAddEntry after HW entry write before SW DB update*/
    PRV_CPSS_SYSYEM_RECOVERY_ERROR_INJECTION_INDEX_EM_6_E,      /*0x40*/
    PRV_CPSS_SYSYEM_RECOVERY_ERROR_INJECTION_INDEX_EM_7_E,      /*0x80*/
    PRV_CPSS_SYSYEM_RECOVERY_ERROR_INJECTION_INDEX_EM_8_E,      /*0x100*/  /*prvCpssDxChExactMatchManagerEntryDelete after SW entry Delete before HW delete*/
    PRV_CPSS_SYSYEM_RECOVERY_ERROR_INJECTION_INDEX_EM_9_E,      /*0x200*/  /*mainLogicEntryDelete after SW & HW entry delete before cuckoo DB update*/
    PRV_CPSS_SYSYEM_RECOVERY_ERROR_INJECTION_INDEX_EM_10_E,     /*0x400*/
    PRV_CPSS_SYSYEM_RECOVERY_ERROR_INJECTION_INDEX_EM_11_E,     /*0x800*/
    PRV_CPSS_SYSYEM_RECOVERY_ERROR_INJECTION_INDEX_EM_12_E,     /*0x1000*/
    PRV_CPSS_SYSYEM_RECOVERY_ERROR_INJECTION_INDEX_EM_13_E,     /*0x2000*/
    PRV_CPSS_SYSYEM_RECOVERY_ERROR_INJECTION_INDEX_EM_14_E,     /*0x4000*/
    PRV_CPSS_SYSYEM_RECOVERY_ERROR_INJECTION_INDEX_EM_15_E,     /*0x8000*/

    PRV_CPSS_SYSYEM_RECOVERY_ERROR_INJECTION_INDEX_LAST_E

} PRV_CPSS_SYSYEM_RECOVERY_ERROR_INJECTION_INDEX_ENT;

/**
* @internal prvCpssSystemRecoveryInProgress function
* @endinternal
*
* @brief   This function inform if any system recovery process
*          take place
*
* @retval GT_TRUE       - system recovery process is going.
* @retval GT_FALSE      - otherwise.
*
*/
GT_BOOL  prvCpssSystemRecoveryInProgress
(
    GT_VOID
);

/**
* @internal prvCpssSystemRecoveryErrorInjectionCheck function
* @endinternal
*
* @brief   This function get error index and check if it match system recovery errorBmp DB
*          if match trigger SW crash (OS only)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - in case errorIndex not valid
* @retval GT_NOT_SUPPORTED         - in case of simulation ,doesn't crash SW only return error
*
*/
GT_STATUS prvCpssSystemRecoveryErrorInjectionCheck
(
    IN  PRV_CPSS_SYSYEM_RECOVERY_ERROR_INJECTION_INDEX_ENT errorInjectionIndex
);

/**
* @internal prvCpssSystemRecoveryErrorInjectionSet function
* @endinternal
*
* @brief   This function set error in system recovery error DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
*
*/
GT_STATUS  prvCpssSystemRecoveryErrorInjectionSet
(
    IN CPSS_SYSTEM_RECOVERY_ERROR_INJECTION_STC errorBmp
);

/**
* @internal prvCpssSystemRecoveryErrorInjectionGet function
* @endinternal
*
* @brief   This function Get errorBmp from system recovery error DB
*
* @retval GT_OK                    - on success
*
*/
GT_STATUS prvCpssSystemRecoveryErrorInjectionGet
(
    INOUT CPSS_SYSTEM_RECOVERY_ERROR_INJECTION_STC   *errorBmpPtr
);


/**
* @internal prvCpssSystemRecoveryDebugModeSkipVersionCheck function
* @endinternal
*
* @brief   This function enable to skip hs Version Check (HS recovery process)
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssSystemRecoveryDebugModeSkipVersionCheck
(
    IN GT_BOOL   enable
);

/**
* @internal prvCpssSystemRecoveryDebugModeSkipMiReset function
* @endinternal
*
* @brief   This function enable to skip hs Mi Reset (HS recovery process)
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssSystemRecoveryDebugModeSkipMiReset
(
    IN GT_BOOL   enable
);

/**
* @internal prvCpssSystemRecoveryDebugModeSkipDfxInitStateCheck function
* @endinternal
*
* @brief   This function enable to skip DFX init state check (HS recovery process)
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssSystemRecoveryDebugModeSkipDfxInitStateCheck
(
    IN GT_BOOL   enable
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssCommonSystemRecoveryh */

