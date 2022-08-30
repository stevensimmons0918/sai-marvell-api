/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*
*/
/**
********************************************************************************
* @file cpssGenSystemRecovery.c
*
* @brief Generic System Recovery ApIs
*
* @version   9
********************************************************************************
*/
#ifdef CHX_FAMILY
#define CPSS_LOG_IN_MODULE_ENABLE
#endif /*CHX_FAMILY*/

#include <cpss/common/cpssHwInit/cpssHwInit.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/common/config/private/prvCpssGenIntDefs.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/common/systemRecovery/private/prvCpssCommonSystemRecoveryLog.h>
#include <cpss/common/systemRecovery/private/prvCpssCommonSystemRecoveryParallel.h>


/**
* @internal internal_cpssSystemRecoveryStateSet function
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
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on bad systemRecoveryState
*/
static GT_STATUS internal_cpssSystemRecoveryStateSet
(
    IN CPSS_SYSTEM_RECOVERY_INFO_STC   *systemRecoveryInfoPtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_SYSTEM_RECOVERY_INFO_STC   systemRecoveryInfoOld;  /* For check purposes */

    CPSS_NULL_PTR_CHECK_MAC(systemRecoveryInfoPtr);
    switch(systemRecoveryInfoPtr->systemRecoveryState)
    {
    case CPSS_SYSTEM_RECOVERY_PREPARATION_STATE_E:
    case CPSS_SYSTEM_RECOVERY_INIT_STATE_E:
    case CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E:
    case CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E:
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    systemRecoveryInfoOld = PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO;
    PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO.systemRecoveryState = systemRecoveryInfoPtr->systemRecoveryState;
    PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO.systemRecoveryMode  = systemRecoveryInfoPtr->systemRecoveryMode;
    PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO.systemRecoveryProcess = systemRecoveryInfoPtr->systemRecoveryProcess;

    switch(systemRecoveryInfoPtr->systemRecoveryProcess)
    {
    case CPSS_SYSTEM_RECOVERY_PROCESS_HSU_E:
        if (systemRecoveryInfoPtr->systemRecoveryMode.haCpuMemoryAccessBlocked == GT_TRUE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        if ( ((systemRecoveryInfoPtr->systemRecoveryMode.continuousRx == GT_FALSE)          ||
              (systemRecoveryInfoPtr->systemRecoveryMode.continuousTx == GT_FALSE)          ||
              (systemRecoveryInfoPtr->systemRecoveryMode.continuousAuMessages == GT_FALSE)  ||
              (systemRecoveryInfoPtr->systemRecoveryMode.continuousFuMessages == GT_FALSE) ) &&
              (PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO.systemRecoveryState == CPSS_SYSTEM_RECOVERY_PREPARATION_STATE_E) )
        {
            CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_SYSTEM_RECOVERY_CNS);

            /* make specific to this mode arrangements for RX,TX,AU,FU queues*/
            CPSS_FUNCPTR_CALL_NULL_COMPARED_MAC(PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_GENERAL_VAR.prvCpssSystemRecoveryMsgDisableModeHandleFuncPtr,rc);

            CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_SYSTEM_RECOVERY_CNS);
        }
        break;
    case CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E:
    case CPSS_SYSTEM_RECOVERY_PROCESS_HITLESS_STARTUP_E:
        if ( (systemRecoveryInfoPtr->systemRecoveryMode.haCpuMemoryAccessBlocked == GT_TRUE) ||
             (systemRecoveryInfoPtr->systemRecoveryMode.continuousRx == GT_TRUE)             ||
             (systemRecoveryInfoPtr->systemRecoveryMode.continuousTx == GT_TRUE)             ||
             (systemRecoveryInfoPtr->systemRecoveryMode.continuousAuMessages == GT_TRUE)     ||
             (systemRecoveryInfoPtr->systemRecoveryMode.continuousFuMessages == GT_TRUE))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        if (PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO.systemRecoveryState == CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E)
        {
            if ((systemRecoveryInfoOld.systemRecoveryState != CPSS_SYSTEM_RECOVERY_INIT_STATE_E) ||
                (systemRecoveryInfoOld.systemRecoveryMode.continuousAuMessages !=
                    systemRecoveryInfoPtr->systemRecoveryMode.continuousAuMessages) ||
                (systemRecoveryInfoOld.systemRecoveryMode.continuousFuMessages !=
                    systemRecoveryInfoPtr->systemRecoveryMode.continuousFuMessages) ||
                (systemRecoveryInfoOld.systemRecoveryMode.continuousRx !=
                    systemRecoveryInfoPtr->systemRecoveryMode.continuousRx) ||
                (systemRecoveryInfoOld.systemRecoveryMode.continuousTx !=
                    systemRecoveryInfoPtr->systemRecoveryMode.continuousTx))
            {
                /* Only allowed to catch up from INIT state, and with exact same parameters */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_SYSTEM_RECOVERY_CNS);
            CPSS_FUNCPTR_CALL_NULL_COMPARED_MAC(PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_GENERAL_VAR.prvCpssSystemRecoveryCatchUpHandleFuncPtr,rc);
            CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_SYSTEM_RECOVERY_CNS);
        }
        break;

    case CPSS_SYSTEM_RECOVERY_PROCESS_HA_E:

        switch (systemRecoveryInfoPtr->systemRecoveryMode.ha2phasesInitPhase)
        {
        case CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_NONE_E:
        case CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_PHASE1_E:
        case CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_PHASE2_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        switch (systemRecoveryInfoPtr->systemRecoveryMode.haReadWriteState)
        {
        case CPSS_SYSTEM_RECOVERY_HA_STATE_READ_ENABLE_WRITE_ENABLE_E:
        case CPSS_SYSTEM_RECOVERY_HA_STATE_READ_ENABLE_WRITE_DISABLE_E:
        case CPSS_SYSTEM_RECOVERY_HA_STATE_READ_DISABLE_WRITE_ENABLE_E:
        case CPSS_SYSTEM_RECOVERY_HA_STATE_READ_DISABLE_WRITE_DISABLE_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }


        if ( (systemRecoveryInfoPtr->systemRecoveryMode.haCpuMemoryAccessBlocked == GT_TRUE) &&
             ((systemRecoveryInfoPtr->systemRecoveryMode.continuousRx == GT_TRUE)         ||
              (systemRecoveryInfoPtr->systemRecoveryMode.continuousTx == GT_TRUE)         ||
              (systemRecoveryInfoPtr->systemRecoveryMode.continuousAuMessages == GT_TRUE) ||
              (systemRecoveryInfoPtr->systemRecoveryMode.continuousFuMessages == GT_TRUE)) )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
        if (PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E)
        {
            if (PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO.systemRecoveryMode.ha2phasesInitPhase == CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_PHASE1_E)
            {
                if (PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO.systemRecoveryMode.haReadWriteState == CPSS_SYSTEM_RECOVERY_HA_STATE_READ_DISABLE_WRITE_DISABLE_E)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                }
                if (PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO.systemRecoveryMode.haReadWriteState == CPSS_SYSTEM_RECOVERY_HA_STATE_READ_DISABLE_WRITE_ENABLE_E)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                }
            }

            if (PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO.systemRecoveryMode.ha2phasesInitPhase == CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_PHASE2_E)
            {
                if (systemRecoveryInfoOld.systemRecoveryMode.ha2phasesInitPhase != CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_PHASE1_E)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_SYSTEM_RECOVERY_CNS);
                CPSS_FUNCPTR_CALL_NULL_COMPARED_MAC(PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_GENERAL_VAR.prvCpssSystemRecoveryHa2PhasesInitHandleFuncPtr,rc);
                CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_SYSTEM_RECOVERY_CNS);
            }
        }
        if (PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO.systemRecoveryState == CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E)
        {
            if ((systemRecoveryInfoOld.systemRecoveryState != CPSS_SYSTEM_RECOVERY_INIT_STATE_E) ||
                (systemRecoveryInfoOld.systemRecoveryMode.continuousAuMessages !=
                    systemRecoveryInfoPtr->systemRecoveryMode.continuousAuMessages) ||
                (systemRecoveryInfoOld.systemRecoveryMode.continuousFuMessages !=
                    systemRecoveryInfoPtr->systemRecoveryMode.continuousFuMessages) ||
                (systemRecoveryInfoOld.systemRecoveryMode.continuousRx !=
                    systemRecoveryInfoPtr->systemRecoveryMode.continuousRx) ||
                (systemRecoveryInfoOld.systemRecoveryMode.continuousTx !=
                    systemRecoveryInfoPtr->systemRecoveryMode.continuousTx))
            {
                /* Only allowed to catch up from INIT state, and with exact same parameters */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
             CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_SYSTEM_RECOVERY_CNS);
             CPSS_FUNCPTR_CALL_NULL_COMPARED_MAC(PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_GENERAL_VAR.prvCpssSystemRecoveryCatchUpHandleFuncPtr,rc);
             CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_SYSTEM_RECOVERY_CNS);
        }
        break;
    case CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E:
    case CPSS_SYSTEM_RECOVERY_PROCESS_PARALLEL_HA_E:
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO.systemRecoveryState == CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E)
    {
        CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_SYSTEM_RECOVERY_CNS);
        if (PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
        {
            if (PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_GENERAL_VAR.prvCpssSystemRecoveryCompletionHandleFuncPtr != NULL)
            {
                CPSS_FUNCPTR_CALL_NULL_COMPARED_MAC(PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_GENERAL_VAR.prvCpssSystemRecoveryCompletionHandleFuncPtr,rc);
            }
            if (PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_GENERAL_VAR.prvCpssSystemRecoveryCompletionGenHandleFuncPtr != NULL)
            {
                CPSS_FUNCPTR_CALL_NULL_COMPARED_MAC(PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_GENERAL_VAR.prvCpssSystemRecoveryCompletionGenHandleFuncPtr, rc);
            }
        }

        if (PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_PARALLEL_HA_E)
        {
            if (PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_GENERAL_VAR.prvCpssSystemRecoveryParallelCompletionHandleFuncPtr != NULL)
            {
                /*run completion handle on non managaer units*/
                CPSS_FUNCPTR_CALL_NULL_COMPARED_1_PARAM_MAC(PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_GENERAL_VAR.prvCpssSystemRecoveryParallelCompletionHandleFuncPtr,CPSS_SYSTEM_RECOVERY_NO_MANAGERS_E,rc);
            }
        }

        if (PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HITLESS_STARTUP_E)
        {
            if (PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_GENERAL_VAR.prvCpssSystemRecoveryCompletionHandleFuncPtr != NULL)
            {
                CPSS_FUNCPTR_CALL_NULL_COMPARED_MAC(PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_GENERAL_VAR.prvCpssSystemRecoveryCompletionHandleFuncPtr,rc);
            }
        }
        CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_SYSTEM_RECOVERY_CNS);

        /* it means that system recovery process is finished  */
        PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E;
        PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO.systemRecoveryMode.continuousAuMessages = GT_FALSE;
        PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO.systemRecoveryMode.continuousFuMessages = GT_FALSE;
        PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO.systemRecoveryMode.continuousRx = GT_FALSE;
        PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO.systemRecoveryMode.continuousTx = GT_FALSE;
        PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO.systemRecoveryMode.haCpuMemoryAccessBlocked = GT_FALSE;
        PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO.systemRecoveryMode.ha2phasesInitPhase = CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_NONE_E;
        PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO.systemRecoveryMode.haReadWriteState = CPSS_SYSTEM_RECOVERY_HA_STATE_READ_ENABLE_WRITE_ENABLE_E;
    }

    return rc;
}

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
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on bad systemRecoveryState
*/
GT_STATUS cpssSystemRecoveryStateSet
(
    IN CPSS_SYSTEM_RECOVERY_INFO_STC   *systemRecoveryInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssSystemRecoveryStateSet);
    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_SYSTEM_RECOVERY_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, systemRecoveryInfoPtr));

    rc = internal_cpssSystemRecoveryStateSet(systemRecoveryInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, systemRecoveryInfoPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_SYSTEM_RECOVERY_CNS);

    return rc;
}

/**
* @internal internal_cpssSystemRecoveryStateGet function
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
static GT_STATUS internal_cpssSystemRecoveryStateGet
(
    OUT CPSS_SYSTEM_RECOVERY_INFO_STC   *systemRecoveryInfoPtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_NULL_PTR_CHECK_MAC(systemRecoveryInfoPtr);

    *systemRecoveryInfoPtr = PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO;

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssSystemRecoveryStateGet);
    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_SYSTEM_RECOVERY_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, systemRecoveryInfoPtr));

    rc = internal_cpssSystemRecoveryStateGet(systemRecoveryInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, systemRecoveryInfoPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_SYSTEM_RECOVERY_CNS);

    return rc;
}

/**
* @internal internal_cpssSystemRecoveryHaParallelModeSet function
* @endinternal
*
* @brief   This function inform cpss about system recovery state under parallel HA mode
*
* @note   APPLICABLE DEVICES:      Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
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
static GT_STATUS internal_cpssSystemRecoveryHaParallelModeSet
(
   IN CPSS_SYSTEM_RECOVERY_STATE_ENT        state,
   IN GT_U32                                numOfManagers,
   IN CPSS_SYSTEM_RECOVERY_MANAGER_ENT      managerListArray[] /*arrSizeVarName=numOfManagers*/
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    ii;

    if (PRV_SYSTEM_RECOVERY_MANAGER_STATUS_MAC(CPSS_SYSTEM_RECOVERY_NO_MANAGERS_E).recoveryDone != GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "general system completion needed first");
    }

    for (ii = 0; ii < numOfManagers; ii++)
    {
        /*check params validity*/
        if (managerListArray[ii] == CPSS_SYSTEM_RECOVERY_NO_MANAGERS_E )
            continue;

        if (managerListArray[ii] == CPSS_SYSTEM_RECOVERY_LAST_MANAGER_E )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        if (PRV_SYSTEM_RECOVERY_MANAGER_STATUS_MAC(managerListArray[ii]).recoveryDone == GT_TRUE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "can't call catch up/completion on manager that finished recovery");
        }

        switch (state)
        {
            case CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E :
                PRV_SYSTEM_RECOVERY_MANAGER_STATUS_MAC(managerListArray[ii]).recoveryStarted = GT_TRUE;
                /*in case port manager start replay , all system is hw write blocked */
                if( managerListArray[ii] == CPSS_SYSTEM_RECOVERY_PORT_MANAGER_E)
                {
                    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_SYSTEM_RECOVERY_CNS);
                    PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO.systemRecoveryState = CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E ;
                    PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_HA_E;
                    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_SYSTEM_RECOVERY_CNS);
                }
                break;
            case CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E :
                /*return system to RECOVERY_PROCESS_NOT_ACTIVE if port manager replay done  */
                if( managerListArray[ii] == CPSS_SYSTEM_RECOVERY_PORT_MANAGER_E)
                {
                    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_SYSTEM_RECOVERY_CNS);
                    PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E ;
                    PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E;
                    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_SYSTEM_RECOVERY_CNS);
                }
                /*remove the hw write block for manager temporarily*/
                PRV_SYSTEM_RECOVERY_MANAGER_STATUS_MAC(managerListArray[ii]).recoveryStarted = GT_FALSE;

                if (PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_GENERAL_VAR.prvCpssSystemRecoveryParallelCompletionHandleFuncPtr != NULL)
                {
                    CPSS_FUNCPTR_CALL_NULL_COMPARED_1_PARAM_MAC(PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_GENERAL_VAR.prvCpssSystemRecoveryParallelCompletionHandleFuncPtr,managerListArray[ii],rc);
                }
                /*system recovery for selected manager is done  */
                PRV_SYSTEM_RECOVERY_MANAGER_STATUS_MAC(managerListArray[ii]).recoveryStarted = GT_TRUE;

                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
    }

    for (ii=0;ii<CPSS_SYSTEM_RECOVERY_LAST_MANAGER_E;ii++)
    {
        if (PRV_SYSTEM_RECOVERY_MANAGER_STATUS_MAC(ii).recoveryDone == GT_FALSE)
        {
            break;
        }
    }
    if (ii == CPSS_SYSTEM_RECOVERY_LAST_MANAGER_E)
    {
        cpssOsMemSet(&(PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_MANAGERS_DB.managersStatusArray), 0, sizeof(PRV_CPSS_SYSTEM_RECOVERY_MANAGER_STATUS_STC)*CPSS_SYSTEM_RECOVERY_LAST_MANAGER_E);
    }

    return rc;
}


/**
* @internal cpssSystemRecoveryHaParallelModeSet function
* @endinternal
*
* @brief  This function inform cpss about system recovery state under parallel HA mode
*
* @note   APPLICABLE DEVICES:      Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3,Aldrin,Aldrin2, ;
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId,cpssSystemRecoveryHaParallelModeSet);
    CPSS_NO_MUTEX_API_LOCK_MAC
    CPSS_LOG_API_ENTER_MAC((funcId, state,numOfManagers,managerListArray));

    rc = internal_cpssSystemRecoveryHaParallelModeSet(state,numOfManagers,managerListArray);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, state,numOfManagers,managerListArray));
    CPSS_NO_MUTEX_API_UNLOCK_MAC
    return rc;
}


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
)
{
    CPSS_API_LOCK_DEVICELESS_NO_RETURN_MAC(PRV_CPSS_FUNCTIONALITY_SYSTEM_RECOVERY_CNS);
    PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO.systemRecoveryState = systemRecoveryInfoPtr->systemRecoveryState;
    PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO.systemRecoveryMode  = systemRecoveryInfoPtr->systemRecoveryMode;
    PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO.systemRecoveryProcess = systemRecoveryInfoPtr->systemRecoveryProcess;
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_SYSTEM_RECOVERY_CNS);
}

