/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssCommonSystemRecoveryLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssCommonSystemRecoveryLogh
#define __prvCpssCommonSystemRecoveryLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_SYSTEM_RECOVERY_MANAGER_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_SYSTEM_RECOVERY_STATE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_SYSTEM_RECOVERY_PROCESS_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_SYSTEM_RECOVERY_HA_STATE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_SYSTEM_RECOVERY_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_SYSTEM_RECOVERY_MODE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_SYSTEM_RECOVERY_INFO_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_SYSTEM_RECOVERY_MANAGER_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_SYSTEM_RECOVERY_STATE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_CPSS_SYSTEM_RECOVERY_INFO_STC_PTR_systemRecoveryInfoPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_CPSS_SYSTEM_RECOVERY_MANAGER_ENT_PTR_managerListArray;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_CPSS_SYSTEM_RECOVERY_STATE_ENT_state;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_numOfManagers;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_CPSS_SYSTEM_RECOVERY_INFO_STC_PTR_systemRecoveryInfoPtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssSystemRecoveryStateSet_E = (CPSS_LOG_LIB_SYSTEM_RECOVERY_COMMON_E << 16),
    PRV_CPSS_LOG_FUNC_cpssSystemRecoveryStateGet_E,
    PRV_CPSS_LOG_FUNC_cpssSystemRecoveryHaParallelModeSet_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssCommonSystemRecoveryLogh */
