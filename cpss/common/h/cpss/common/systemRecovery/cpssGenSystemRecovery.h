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
* @file cpssGenSystemRecovery.h
*
* @brief Includes generic system recovery definitions.
*
*
* @version   6
********************************************************************************
*/

#ifndef __cpssGenSystemRecoveryh
#define __cpssGenSystemRecoveryh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

 /* macro to call a callback function,first check  if the function points to NULL */
#define CPSS_FUNCPTR_CALL_NULL_COMPARED_MAC(_func,_rc) \
     do { \
           /*check if callback is initialized*/\
             if(NULL == _func)\
             {\
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);\
             }\
             \
            _rc = (*_func)();\
            if (_rc != GT_OK)\
            {\
                return _rc;\
            }\
     } while (0)

 /* macro to call a callback function,first check  if the function points to NULL */
#define CPSS_FUNCPTR_CALL_NULL_COMPARED_1_PARAM_MAC(_func,_arg1,_rc) \
     do { \
           /*check if callback is initialized*/\
             if(NULL == _func)\
             {\
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);\
             }\
             \
            _rc = (*_func)(_arg1);\
            if (_rc != GT_OK)\
            {\
                return _rc;\
            }\
     } while (0)

 /**
 * Typedef enum CPSS_SYSTEM_RECOVERY_STATE_ENT
 *
 * @brief : Indicates in which state system recovery process is.
 *
 */
typedef enum
{
    /** system is preparing to recovery process*/
    CPSS_SYSTEM_RECOVERY_PREPARATION_STATE_E,
    /** system is going through recovery init */
    CPSS_SYSTEM_RECOVERY_INIT_STATE_E,
    /** system recovery is completed */
    CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E,
    /** system is going to make hw catch up */
    CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E
} CPSS_SYSTEM_RECOVERY_STATE_ENT;

 /**
 * Typedef enum CPSS_SYSTEM_RECOVERY_PROCESS_ENT
 *
 * @brief : Indicates which system recovery process is running.
 *
 */
typedef enum
{
    /** HSU process */
    CPSS_SYSTEM_RECOVERY_PROCESS_HSU_E,
    /** Fast Boot process */
    CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E,
    /** recovery process after High Availability event */
    CPSS_SYSTEM_RECOVERY_PROCESS_HA_E,
    /** no active system recovery process */
    CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E,
    /** recovery process after High Availability event with multi processes */
    CPSS_SYSTEM_RECOVERY_PROCESS_PARALLEL_HA_E,
    /** hitless startup process */
    CPSS_SYSTEM_RECOVERY_PROCESS_HITLESS_STARTUP_E

} CPSS_SYSTEM_RECOVERY_PROCESS_ENT;

 /**
 * Typedef enum CPSS_SYSTEM_RECOVERY_MANAGER_ENT
 *
 * @brief : Indicates which system recovery manager is handle ,used for parallel High Availability.
 *
 */
typedef enum
{
    /** indicate in completion state to sync only non managers units */
    CPSS_SYSTEM_RECOVERY_NO_MANAGERS_E,
    /** action take place only for port manager*/
    CPSS_SYSTEM_RECOVERY_PORT_MANAGER_E,
    /** action take place only for fdb manager*/
    CPSS_SYSTEM_RECOVERY_FDB_MANAGER_E,
    /** action take place only for lpm manager*/
    CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E,
    /** action take place only for tcam manager*/
    CPSS_SYSTEM_RECOVERY_TCAM_MANAGER_E,
    /** action take place only for exact match manager*/
    CPSS_SYSTEM_RECOVERY_EXACT_MATCH_MANAGER_E,
    /** action take place only for trunk manager*/
    CPSS_SYSTEM_RECOVERY_TRUNK_MANAGER_E,
    CPSS_SYSTEM_RECOVERY_LAST_MANAGER_E
} CPSS_SYSTEM_RECOVERY_MANAGER_ENT;


/**
 * Typedef enum CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_ENT
 *
 * @brief : Describes in which phase of HA_2_PHASES_INIT procedure system
 * recovery process is.
 *
 */
typedef enum
{
    /** HA_2_PHASES_INIT is not running */
    CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_NONE_E,
    /** phase1 of HA_2_PHASES_INIT is running */
    CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_PHASE1_E,
    /** phase2 of HA_2_PHASES_INIT is running */
    CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_PHASE2_E
} CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_ENT;



/**
 * Typedef enum CPSS_SYSTEM_RECOVERY_HA_STATE_ENT
 *
 * @brief : Describes in which state PEX during HA recovery process is.
 *
 */
typedef enum
{
    /** all operations via PEX are enabled */
    CPSS_SYSTEM_RECOVERY_HA_STATE_READ_ENABLE_WRITE_ENABLE_E,
    /** only read operations via PEX are enabled */
    CPSS_SYSTEM_RECOVERY_HA_STATE_READ_ENABLE_WRITE_DISABLE_E,
    /** only write operations via PEX are enabled */
    CPSS_SYSTEM_RECOVERY_HA_STATE_READ_DISABLE_WRITE_ENABLE_E,
    /** all operations via PEX are disabled */
    CPSS_SYSTEM_RECOVERY_HA_STATE_READ_DISABLE_WRITE_DISABLE_E

} CPSS_SYSTEM_RECOVERY_HA_STATE_ENT;


/**
* @struct CPSS_SYSTEM_RECOVERY_MODE_STC
 *
 * @brief This struct containes description of system recovery modes.
*/
typedef struct{

    /** @brief This mode describes what memory allocation is for Rx descriptors and Rx buffers:
     *  GT_TRUE: Rx descriptors and Rx buffers are allocated in the same memory
     *  before and after system recovery process.
     *  GT_FALSE: Rx descriptors and Rx buffers are allocated in different memory
     *  before and after system recovery process.
     */
    GT_BOOL continuousRx;

    /** @brief This mode describes what memory allocation is for Tx descriptors:
     *  GT_TRUE: Tx descriptors are allocated in the same memory
     *  before and after system recovery process.
     *  GT_FALSE: Tx descriptors are allocated in different memories
     *  before and after system recovery process.
     */
    GT_BOOL continuousTx;

    /** @brief This mode describes what memory allocation is for AUQ descriptors:
     *  GT_TRUE: AUQ descriptors are allocated in the same memory
     *  before and after system recovery process.
     *  GT_FALSE: AUQ descriptors are allocated in different memories
     *  before and after system recovery process.
     */
    GT_BOOL continuousAuMessages;

    /** @brief This mode describes what memory allocation is for FUQ descriptors:
     *  GT_TRUE: FUQ descriptors are allocated in the same memory
     *  before and after system recovery process.
     *  GT_FALSE: FUQ descriptors are allocated in different memories
     *  before and after system recovery process.
     */
    GT_BOOL continuousFuMessages;

    /** @brief GT_TRUE: special mode after HA event when PP can't access CPU memory but CPU can access PP.
     *  - GT_FALSE: regular mode; both CPU and PP can access each other memories.
     *  It is actually when not the same memory address spaces are used for AUQ/FUQ/RX messages
     *  before and after HA event.
     *  This mode should be set by Application after HA event before any usage of
     *  AUQ/FUQ/RX memories allocated before HA event, in order to prevent sending by PP
     *  messages to these memories already not related to PP.
     *  It is obligement of Application to revert memory access to normal mode after cpssInit.
     *  (During cpssInit PP is initialized with new AUQ/FUQ/RX pointers )
     */
    GT_BOOL haCpuMemoryAccessBlocked;

    /** @brief  Describes  HA cpss init mode */
    CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_ENT ha2phasesInitPhase;

    /** @brief Describes HA PEX state */
    CPSS_SYSTEM_RECOVERY_HA_STATE_ENT haReadWriteState;

} CPSS_SYSTEM_RECOVERY_MODE_STC;


/*******************************************************************************
* PRV_CPSS_SYSTEM_RECOVERY_MSG_DISABLE_MODE_FUN
*
* DESCRIPTION:
*       Handling au and fu queues,Rx SDMA under non-continues mode (all AU/FU/RX messages
*       during system recovery process are disregarded) to provide smooth reprogramming in new immage.
*
* APPLICABLE DEVICES:
*        xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* NOT APPLICABLE DEVICES:
*       none
*
* INPUTS:
*       None
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
typedef GT_STATUS (*PRV_CPSS_SYSTEM_RECOVERY_MSG_DISABLE_MODE_FUN)(GT_VOID);


/*******************************************************************************
* PRV_CPSS_SYSTEM_RECOVERY_HA_FUN
*
* DESCRIPTION:
*       Perform synchronization of hardware data and software DB after special init sequence.
*
* APPLICABLE DEVICES:
*         Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* NOT APPLICABLE DEVICES:
*       xCat3; AC5; Puma2; Puma3.
*
* INPUTS:
*       systemRecoveryModePtr - pointer to system recovery mode data.
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
typedef GT_STATUS (*PRV_CPSS_SYSTEM_RECOVERY_HA_FUN)(GT_VOID);

/**
* @struct CPSS_SYSTEM_RECOVERY_INFO_STC
 *
 * @brief This struct containes information about system recovery process.
*/
typedef struct{

    /** Indicates in which state system recovery process is */
    CPSS_SYSTEM_RECOVERY_STATE_ENT systemRecoveryState;

    /** Indicates in which working mode recovery process is. */
    CPSS_SYSTEM_RECOVERY_MODE_STC systemRecoveryMode;

    /** Indicates which recovery process is taking place. */
    CPSS_SYSTEM_RECOVERY_PROCESS_ENT systemRecoveryProcess;

} CPSS_SYSTEM_RECOVERY_INFO_STC;

/**
* @internal cpssSystemRecoveryStateSet function
* @endinternal
*
* @brief   This function inform cpss about system recovery state
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] systemRecoveryInfoPtr    - pointer to system recovery information
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on bad systemRecoveryState.
*/
GT_STATUS cpssSystemRecoveryStateSet
(
    IN CPSS_SYSTEM_RECOVERY_INFO_STC   *systemRecoveryInfoPtr
);

/**
* @internal cpssSystemRecoveryStateGet function
* @endinternal
*
* @brief   This function retrieve system recovery information
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @param[out] systemRecoveryInfoPtr    - pointer to system recovery
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on bad systemRecoveryState.
*/
GT_STATUS cpssSystemRecoveryStateGet
(
    OUT CPSS_SYSTEM_RECOVERY_INFO_STC   *systemRecoveryInfoPtr
);

/**
* @internal cpssSystemRecoveryHaParallelModeSet function
* @endinternal
*
* @brief  This function inform cpss about system recovery state under parallel HA mode
*
* @note   APPLICABLE DEVICES:      Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3,Aldrin;
*
*
* @param[in] state              - state of recovery to set
* @param[in] numOfManagers      - number of manager in array to set
* @param[in] managerListArray   - (pointer to) manager List Array
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on bad systemRecoveryState.
*/
GT_STATUS cpssSystemRecoveryHaParallelModeSet
(
   IN CPSS_SYSTEM_RECOVERY_STATE_ENT        state,
   IN GT_U32                                numOfManagers,
   IN CPSS_SYSTEM_RECOVERY_MANAGER_ENT      managerListArray[] /*arrSizeVarName=numOfManagers*/
);

/**
* @internal prvCpssSystemRecoveryStateUpdate function
* @endinternal
*
* @brief   This function update cpss system recovery DB state;
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] systemRecoveryInfoPtr    - pointer to system recovery information
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on bad systemRecoveryState
*/
GT_VOID prvCpssSystemRecoveryStateUpdate
(
    IN CPSS_SYSTEM_RECOVERY_INFO_STC   *systemRecoveryInfoPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssGenSystemRecoveryh */

