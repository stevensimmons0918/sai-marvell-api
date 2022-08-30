/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssCommonSystemRecoveryLog.c
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/common/systemRecovery/private/prvCpssCommonSystemRecoveryLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_SYSTEM_RECOVERY_MANAGER_ENT[]  =
{
    "CPSS_SYSTEM_RECOVERY_NO_MANAGERS_E",
    "CPSS_SYSTEM_RECOVERY_PORT_MANAGER_E",
    "CPSS_SYSTEM_RECOVERY_FDB_MANAGER_E",
    "CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E",
    "CPSS_SYSTEM_RECOVERY_TCAM_MANAGER_E",
    "CPSS_SYSTEM_RECOVERY_EXACT_MATCH_MANAGER_E",
    "CPSS_SYSTEM_RECOVERY_TRUNK_MANAGER_E",
    "CPSS_SYSTEM_RECOVERY_LAST_MANAGER_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_SYSTEM_RECOVERY_MANAGER_ENT);
const char * const prvCpssLogEnum_CPSS_SYSTEM_RECOVERY_STATE_ENT[]  =
{
    "CPSS_SYSTEM_RECOVERY_PREPARATION_STATE_E",
    "CPSS_SYSTEM_RECOVERY_INIT_STATE_E",
    "CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E",
    "CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_SYSTEM_RECOVERY_STATE_ENT);
const char * const prvCpssLogEnum_CPSS_SYSTEM_RECOVERY_PROCESS_ENT[]  =
{
    "CPSS_SYSTEM_RECOVERY_PROCESS_HSU_E",
    "CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E",
    "CPSS_SYSTEM_RECOVERY_PROCESS_HA_E",
    "CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E",
    "CPSS_SYSTEM_RECOVERY_PROCESS_PARALLEL_HA_E",
    "CPSS_SYSTEM_RECOVERY_PROCESS_HITLESS_STARTUP_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_SYSTEM_RECOVERY_PROCESS_ENT);
const char * const prvCpssLogEnum_CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_ENT[]  =
{
    "CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_NONE_E",
    "CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_PHASE1_E",
    "CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_PHASE2_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_ENT);
const char * const prvCpssLogEnum_CPSS_SYSTEM_RECOVERY_HA_STATE_ENT[]  =
{
    "CPSS_SYSTEM_RECOVERY_HA_STATE_READ_ENABLE_WRITE_ENABLE_E",
    "CPSS_SYSTEM_RECOVERY_HA_STATE_READ_ENABLE_WRITE_DISABLE_E",
    "CPSS_SYSTEM_RECOVERY_HA_STATE_READ_DISABLE_WRITE_ENABLE_E",
    "CPSS_SYSTEM_RECOVERY_HA_STATE_READ_DISABLE_WRITE_DISABLE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_SYSTEM_RECOVERY_HA_STATE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_SYSTEM_RECOVERY_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_SYSTEM_RECOVERY_INFO_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, systemRecoveryState, CPSS_SYSTEM_RECOVERY_STATE_ENT);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, systemRecoveryMode, CPSS_SYSTEM_RECOVERY_MODE_STC);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, systemRecoveryProcess, CPSS_SYSTEM_RECOVERY_PROCESS_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_SYSTEM_RECOVERY_MODE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_SYSTEM_RECOVERY_MODE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, continuousRx);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, continuousTx);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, continuousAuMessages);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, continuousFuMessages);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, haCpuMemoryAccessBlocked);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, ha2phasesInitPhase, CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, haReadWriteState, CPSS_SYSTEM_RECOVERY_HA_STATE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_SYSTEM_RECOVERY_INFO_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_SYSTEM_RECOVERY_INFO_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_SYSTEM_RECOVERY_INFO_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_SYSTEM_RECOVERY_MANAGER_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_SYSTEM_RECOVERY_MANAGER_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_SYSTEM_RECOVERY_MANAGER_ENT);
}
void prvCpssLogParamFunc_CPSS_SYSTEM_RECOVERY_STATE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_SYSTEM_RECOVERY_STATE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_SYSTEM_RECOVERY_STATE_ENT);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC IN_CPSS_SYSTEM_RECOVERY_INFO_STC_PTR_systemRecoveryInfoPtr = {
     "systemRecoveryInfoPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_SYSTEM_RECOVERY_INFO_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_CPSS_SYSTEM_RECOVERY_MANAGER_ENT_PTR_managerListArray = {
     "managerListArray", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_SYSTEM_RECOVERY_MANAGER_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_CPSS_SYSTEM_RECOVERY_STATE_ENT_state = {
     "state", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_SYSTEM_RECOVERY_STATE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_numOfManagers = {
     "numOfManagers", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_CPSS_SYSTEM_RECOVERY_INFO_STC_PTR_systemRecoveryInfoPtr = {
     "systemRecoveryInfoPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_SYSTEM_RECOVERY_INFO_STC)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssSystemRecoveryStateSet_PARAMS[] =  {
    &IN_CPSS_SYSTEM_RECOVERY_INFO_STC_PTR_systemRecoveryInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssSystemRecoveryHaParallelModeSet_PARAMS[] =  {
    &IN_CPSS_SYSTEM_RECOVERY_STATE_ENT_state,
    &IN_GT_U32_numOfManagers,
    &IN_CPSS_SYSTEM_RECOVERY_MANAGER_ENT_PTR_managerListArray
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssSystemRecoveryStateGet_PARAMS[] =  {
    &OUT_CPSS_SYSTEM_RECOVERY_INFO_STC_PTR_systemRecoveryInfoPtr
};


/********* lib API DB *********/


#ifdef CHX_FAMILY
static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssCommonDXSystemRecovery_commonLogLibDb[] = {
    {"cpssSystemRecoveryStateSet", 1, cpssSystemRecoveryStateSet_PARAMS, NULL},
    {"cpssSystemRecoveryStateGet", 1, cpssSystemRecoveryStateGet_PARAMS, NULL},
    {"cpssSystemRecoveryHaParallelModeSet", 3, cpssSystemRecoveryHaParallelModeSet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_SYSTEM_RECOVERY_COMMON(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssCommonDXSystemRecovery_commonLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssCommonDXSystemRecovery_commonLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}
#endif

#ifdef PX_FAMILY
static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssCommonPXSystemRecovery_commonLogLibDb[] = {
    {"cpssSystemRecoveryStateSet", 1, cpssSystemRecoveryStateSet_PARAMS, NULL},
    {"cpssSystemRecoveryStateGet", 1, cpssSystemRecoveryStateGet_PARAMS, NULL},
    {"cpssSystemRecoveryHaParallelModeSet", 3, cpssSystemRecoveryHaParallelModeSet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_PX_CPSS_LOG_LIB_SYSTEM_RECOVERY_COMMON(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssCommonPXSystemRecovery_commonLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssCommonPXSystemRecovery_commonLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}
#endif

