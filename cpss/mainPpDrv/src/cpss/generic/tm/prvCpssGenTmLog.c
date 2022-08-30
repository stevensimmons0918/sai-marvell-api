/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssGenTmLog.c
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/

/* disable deprecation warnings (if one) */
#ifdef __GNUC__
#if  (__GNUC__*100+__GNUC_MINOR__) >= 406
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#endif

#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssGenDbLog.h>
#include <cpss/generic/tm/cpssTmPublicDefs.h>
#include <cpss/generic/tm/private/prvCpssGenTmLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_TM2TM_CHANNEL_ENT[]  =
{
    "CPSS_TM2TM_CHANNEL_NODE_E",
    "CPSS_TM2TM_CHANNEL_PORT_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_TM2TM_CHANNEL_ENT);
const char * const prvCpssLogEnum_CPSS_TM_COLOR_NUM_ENT[]  =
{
    "CPSS_TM_COLOR_NUM_1_E",
    "CPSS_TM_COLOR_NUM_2_E",
    "CPSS_TM_COLOR_NUM_3_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_TM_COLOR_NUM_ENT);
const char * const prvCpssLogEnum_CPSS_TM_DP_SOURCE_ENT[]  =
{
    "CPSS_TM_DP_SOURCE_AQL_E",
    "CPSS_TM_DP_SOURCE_QL_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_TM_DP_SOURCE_ENT);
const PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_CPSS_TM_ELIG_FUNC_NODE_ENT[]  =
{PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_N_PRIO1_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_N_PRIO5_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_N_SHP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_N_MIN_SHP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_N_PPA_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_N_PPA_SP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_N_PPA_SHP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_N_PPA_SP_MIN_SHP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_N_PPA_SHP_IGN_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_N_PPA_MIN_SHP_SP_IGN_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_N_FP0_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_N_FP1_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_N_FP2_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_N_FP3_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_N_FP4_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_N_FP5_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_N_FP6_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_N_FP7_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_N_MIN_SHP_FP0_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_N_MIN_SHP_FP1_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_N_MIN_SHP_FP2_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_N_MIN_SHP_FP3_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_N_MIN_SHP_FP4_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_N_MIN_SHP_FP5_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_N_MIN_SHP_FP6_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_N_MIN_SHP_FP7_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_N_MAX_INC_MIN_SHP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_N_PP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_N_MIN_SHP_PP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_N_PP_SHP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_N_SHP_4P_MIN_4P_MAX_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_N_SHP_PP_TB_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_N_SHP_PP_MAX_TB_0),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_N_DEQ_DIS_E)
};
PRV_CPSS_LOG_STC_ENUM_MAP_ARRAY_SIZE_MAC(CPSS_TM_ELIG_FUNC_NODE_ENT);
const char * const prvCpssLogEnum_CPSS_TM_LEVEL_ENT[]  =
{
    "CPSS_TM_LEVEL_Q_E",
    "CPSS_TM_LEVEL_A_E",
    "CPSS_TM_LEVEL_B_E",
    "CPSS_TM_LEVEL_C_E",
    "CPSS_TM_LEVEL_P_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_TM_LEVEL_ENT);
const char * const prvCpssLogEnum_CPSS_TM_SCHD_MODE_ENT[]  =
{
    "CPSS_TM_SCHD_MODE_RR_E",
    "CPSS_TM_SCHD_MODE_DWRR_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_TM_SCHD_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_TM_DROP_MODE_ENT[]  =
{
    "CPSS_TM_DROP_MODE_CB_TD_CA_WRED_E",
    "CPSS_TM_DROP_MODE_CB_TD_CA_TD_E",
    "CPSS_TM_DROP_MODE_CB_TD_ONLY_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_TM_DROP_MODE_ENT);
const PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_CPSS_TM_ELIG_FUNC_QUEUE_ENT[]  =
{PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_Q_PRIO0_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_Q_PRIO1_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_Q_PRIO2_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_Q_PRIO3_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_Q_MIN_SHP_PRIO0_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_Q_MIN_SHP_PRIO1_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_Q_MIN_SHP_PRIO2_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_Q_MIN_SHP_PRIO3_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_Q_PRIO4_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_Q_PRIO5_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_Q_PRIO6_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_Q_PRIO7_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_Q_MIN_SHP_PRIO4_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_Q_MIN_SHP_PRIO5_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_Q_MIN_SHP_PRIO6_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_Q_MIN_SHP_PRIO7_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_Q_SHP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_Q_MAX_INC_MIN_SHP_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_Q_SHP_SCHED00_PROP00),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_Q_SHP_SCHED10_PROP10),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_Q_SHP_SCHED20_PROP20),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_Q_SHP_SCHED30_PROP30),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_Q_SHP_SCHED40_PROP40),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_Q_SHP_SCHED50_PROP50),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_Q_SHP_SCHED60_PROP60),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_Q_SHP_SCHED70_PROP70),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TM_ELIG_Q_DEQ_DIS_E)
};
PRV_CPSS_LOG_STC_ENUM_MAP_ARRAY_SIZE_MAC(CPSS_TM_ELIG_FUNC_QUEUE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_TM2TM_CNTRL_PKT_STRUCT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_TM2TM_CNTRL_PKT_STRUCT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, portsNum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, nodesNum);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_TM2TM_DELTA_RANGE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_TM2TM_DELTA_RANGE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, upperRange0);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, upperRange1);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, upperRange2);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_TM2TM_EXTERNAL_HDR_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_TM2TM_EXTERNAL_HDR_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, size);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, contentArr, 4, GT_U64);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_TM_A_NODE_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_TM_A_NODE_PARAMS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, shapingProfilePtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, quantum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dropProfileInd);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, eligiblePrioFuncId, CPSS_TM_ELIG_FUNC_NODE_ENT);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, schdModeArr, 8, CPSS_TM_SCHD_MODE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numOfChildren);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_TM_BAP_UNIT_ERROR_STATUS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_TM_BAP_UNIT_ERROR_STATUS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, bapStatus);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, bapNum);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_TM_B_NODE_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_TM_B_NODE_PARAMS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, shapingProfilePtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, quantum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dropProfileInd);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, eligiblePrioFuncId, CPSS_TM_ELIG_FUNC_NODE_ENT);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, schdModeArr, 8, CPSS_TM_SCHD_MODE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numOfChildren);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_TM_C_NODE_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_TM_C_NODE_PARAMS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, shapingProfilePtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, quantum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dropCosMap);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, dropProfileIndArr, 8, GT_U32);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, eligiblePrioFuncId, CPSS_TM_ELIG_FUNC_NODE_ENT);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, schdModeArr, 8, CPSS_TM_SCHD_MODE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numOfChildren);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_TM_DROP_PROFILE_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_TM_DROP_PROFILE_PARAMS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, dropMode, CPSS_TM_DROP_MODE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, cbTdThresholdBytes);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, caTdDp, CPSS_TM_DROP_PROFILE_CA_TD_PARAMS_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, caWredDp, CPSS_TM_DROP_PROFILE_CA_WRED_PARAMS_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_TM_LEVEL_PERIODIC_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_TM_LEVEL_PERIODIC_PARAMS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, periodicState);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, shaperDecoupling);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_TM_LIB_INIT_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_TM_LIB_INIT_PARAMS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, tmMtu);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_TM_NODE_STATUS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_TM_NODE_STATUS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxBucketLevel);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, minBucketLevel);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, deficit);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_TM_PORT_DROP_PER_COS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_TM_PORT_DROP_PER_COS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dropCosMap);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, dropProfileIndArr, 8, GT_U32);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_TM_PORT_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_TM_PORT_PARAMS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, cirBw);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, eirBw);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, cbs);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ebs);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, quantumArr, 8, GT_U32);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, schdModeArr, 8, CPSS_TM_SCHD_MODE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dropProfileInd);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, eligiblePrioFuncId, CPSS_TM_ELIG_FUNC_NODE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numOfChildren);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_TM_PORT_STATUS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_TM_PORT_STATUS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxBucketLevel);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, minBucketLevel);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, deficitArr, 8, GT_U32);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_TM_QMR_PKT_STATISTICS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_TM_QMR_PKT_STATISTICS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, numPktsToUnInstallQueue, GT_U64);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_TM_QUANTUM_LIMITS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_TM_QUANTUM_LIMITS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, minQuantum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxQuantum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, resolution);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_TM_QUEUE_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_TM_QUEUE_PARAMS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, shapingProfilePtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, quantum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dropProfileInd);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, eligiblePrioFuncId, CPSS_TM_ELIG_FUNC_QUEUE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_TM_RCB_PKT_STATISTICS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_TM_RCB_PKT_STATISTICS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, numPktsToSms, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, numCrcErrPktsToSms, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, numErrsFromSmsToDram, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, numPortFlushDrpDataPkts, GT_U64);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_TM_SHAPING_PROFILE_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_TM_SHAPING_PROFILE_PARAMS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, cirBw);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, cbs);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, eirBw);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ebs);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_TM_TREE_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_TM_TREE_PARAMS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxQueues);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxAnodes);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxBnodes);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxCnodes);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxPorts);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_TM_UNITS_ERROR_STATUS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_TM_UNITS_ERROR_STATUS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, qmngrStatus);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dropStatus);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, schedStatus);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, rcbStatus);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_TM_DROP_PROFILE_CA_TD_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_TM_DROP_PROFILE_CA_TD_PARAMS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, caTdThreshold, 3, GT_U32);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_TM_DROP_PROFILE_CA_WRED_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_TM_DROP_PROFILE_CA_WRED_PARAMS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, aqlExponent);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, curveIndex, 3, GT_U8);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, dpCurveScale, 3, GT_U8);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, caWredTdMinThreshold, 3, GT_U32);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, caWredTdMaxThreshold, 3, GT_U32);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_TM_ELIG_PRIO_FUNC_OUT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_TM_ELIG_PRIO_FUNC_OUT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxTb);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, minTb);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, propPrio);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, schedPrio);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, elig);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_TM_SCHD_MODE_ENT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_FIELD_VAL_PTR_MAC(CPSS_TM_SCHD_MODE_ENT *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *valPtr, CPSS_TM_SCHD_MODE_ENT);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_TM2TM_CHANNEL_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_TM2TM_CHANNEL_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_TM2TM_CHANNEL_ENT);
}
void prvCpssLogParamFunc_CPSS_TM2TM_CNTRL_PKT_STRUCT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_TM2TM_CNTRL_PKT_STRUCT_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_TM2TM_CNTRL_PKT_STRUCT_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_TM2TM_DELTA_RANGE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_TM2TM_DELTA_RANGE_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_TM2TM_DELTA_RANGE_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_TM2TM_EXTERNAL_HDR_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_TM2TM_EXTERNAL_HDR_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_TM2TM_EXTERNAL_HDR_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_TM_A_NODE_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_TM_A_NODE_PARAMS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_TM_A_NODE_PARAMS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_TM_BAP_UNIT_ERROR_STATUS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_TM_BAP_UNIT_ERROR_STATUS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_TM_BAP_UNIT_ERROR_STATUS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_TM_B_NODE_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_TM_B_NODE_PARAMS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_TM_B_NODE_PARAMS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_TM_COLOR_NUM_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_TM_COLOR_NUM_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_TM_COLOR_NUM_ENT);
}
void prvCpssLogParamFunc_CPSS_TM_C_NODE_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_TM_C_NODE_PARAMS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_TM_C_NODE_PARAMS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_TM_DP_SOURCE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_TM_DP_SOURCE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_TM_DP_SOURCE_ENT);
}
void prvCpssLogParamFunc_CPSS_TM_ELIG_FUNC_NODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_TM_ELIG_FUNC_NODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAP_MAC(namePtr, paramVal, CPSS_TM_ELIG_FUNC_NODE_ENT);
}
void prvCpssLogParamFunc_CPSS_TM_ELIG_PRIO_FUNC_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_TM_ELIG_PRIO_FUNC_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_TM_ELIG_PRIO_FUNC_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_TM_LEVEL_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_TM_LEVEL_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_TM_LEVEL_ENT);
}
void prvCpssLogParamFunc_CPSS_TM_LEVEL_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_TM_LEVEL_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_TM_LEVEL_ENT);
}
void prvCpssLogParamFunc_CPSS_TM_LEVEL_PERIODIC_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_TM_LEVEL_PERIODIC_PARAMS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_TM_LEVEL_PERIODIC_PARAMS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_TM_LIB_INIT_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_TM_LIB_INIT_PARAMS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_TM_LIB_INIT_PARAMS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_TM_NODE_STATUS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_TM_NODE_STATUS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_TM_NODE_STATUS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_TM_PORT_DROP_PER_COS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_TM_PORT_DROP_PER_COS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_TM_PORT_DROP_PER_COS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_TM_PORT_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_TM_PORT_PARAMS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_TM_PORT_PARAMS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_TM_PORT_STATUS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_TM_PORT_STATUS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_TM_PORT_STATUS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_TM_QMR_PKT_STATISTICS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_TM_QMR_PKT_STATISTICS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_TM_QMR_PKT_STATISTICS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_TM_QUANTUM_LIMITS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_TM_QUANTUM_LIMITS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_TM_QUANTUM_LIMITS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_TM_QUEUE_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_TM_QUEUE_PARAMS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_TM_QUEUE_PARAMS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_TM_RCB_PKT_STATISTICS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_TM_RCB_PKT_STATISTICS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_TM_RCB_PKT_STATISTICS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_TM_SHAPING_PROFILE_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_TM_SHAPING_PROFILE_PARAMS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_TM_SHAPING_PROFILE_PARAMS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_TM_TREE_CHANGE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_TM_TREE_CHANGE_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_TM_TREE_CHANGE_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_TM_TREE_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_TM_TREE_PARAMS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_TM_TREE_PARAMS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_TM_UNITS_ERROR_STATUS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_TM_UNITS_ERROR_STATUS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_TM_UNITS_ERROR_STATUS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TM2TM_CHANNEL_ENT_channel = {
     "channel", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_TM2TM_CHANNEL_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TM2TM_CHANNEL_ENT_remoteLevel = {
     "remoteLevel", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_TM2TM_CHANNEL_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TM2TM_CNTRL_PKT_STRUCT_STC_PTR_ctrlPacketStrPtr = {
     "ctrlPacketStrPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM2TM_CNTRL_PKT_STRUCT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TM2TM_DELTA_RANGE_STC_PTR_rangePtr = {
     "rangePtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM2TM_DELTA_RANGE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TM2TM_EXTERNAL_HDR_STC_PTR_extHeadersPtr = {
     "extHeadersPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM2TM_EXTERNAL_HDR_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TM_A_NODE_PARAMS_STC_PTR_aParamsPtr = {
     "aParamsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_A_NODE_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TM_A_NODE_PARAMS_STC_PTR_paramsPtr = {
     "paramsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_A_NODE_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TM_B_NODE_PARAMS_STC_PTR_bParamsPtr = {
     "bParamsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_B_NODE_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TM_B_NODE_PARAMS_STC_PTR_paramsPtr = {
     "paramsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_B_NODE_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TM_COLOR_NUM_ENT_colorNum = {
     "colorNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_TM_COLOR_NUM_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TM_COLOR_NUM_ENT_number = {
     "number", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_TM_COLOR_NUM_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TM_C_NODE_PARAMS_STC_PTR_cParamsPtr = {
     "cParamsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_C_NODE_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TM_C_NODE_PARAMS_STC_PTR_paramsPtr = {
     "paramsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_C_NODE_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TM_DP_SOURCE_ENT_PTR_dpSourcePtr = {
     "dpSourcePtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_DP_SOURCE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TM_DP_SOURCE_ENT_source = {
     "source", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_TM_DP_SOURCE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TM_DROP_PROFILE_PARAMS_STC_PTR_profileParamsPtr = {
     "profileParamsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_DROP_PROFILE_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TM_DROP_PROFILE_PARAMS_STC_PTR_profileStcPtr = {
     "profileStcPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_DROP_PROFILE_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TM_DROP_PROFILE_PARAMS_STC_PTR_profileStrArr = {
     "profileStrArr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_DROP_PROFILE_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TM_ELIG_FUNC_NODE_ENT_eligPrioFuncId = {
     "eligPrioFuncId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_TM_ELIG_FUNC_NODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TM_ELIG_PRIO_FUNC_STC_PTR_funcOutArr = {
     "funcOutArr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_ELIG_PRIO_FUNC_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TM_LEVEL_ENT_bpLevel = {
     "bpLevel", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_TM_LEVEL_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TM_LEVEL_ENT_level = {
     "level", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_TM_LEVEL_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TM_LEVEL_ENT_localLevel = {
     "localLevel", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_TM_LEVEL_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TM_LEVEL_ENT_srcLevel = {
     "srcLevel", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_TM_LEVEL_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TM_LEVEL_PERIODIC_PARAMS_STC_PTR_paramsPtr = {
     "paramsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_LEVEL_PERIODIC_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TM_LIB_INIT_PARAMS_STC_PTR_tmLibInitParamsPtr = {
     "tmLibInitParamsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_LIB_INIT_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TM_PORT_DROP_PER_COS_STC_PTR_paramsPtr = {
     "paramsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_PORT_DROP_PER_COS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TM_PORT_PARAMS_STC_PTR_paramsPtr = {
     "paramsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_PORT_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TM_QUEUE_PARAMS_STC_PTR_paramsPtr = {
     "paramsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_QUEUE_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TM_QUEUE_PARAMS_STC_PTR_qParamsPtr = {
     "qParamsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_QUEUE_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TM_SCHD_MODE_ENT_PTR_schdModeArr = {
     "schdModeArr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_SCHD_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_TM_SHAPING_PROFILE_PARAMS_STC_PTR_profileStrPtr = {
     "profileStrPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_SHAPING_PROFILE_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_8_PTR_nodeName = {
     "nodeName", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_8_PTR)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_8_PTR_portName = {
     "portName", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_8_PTR)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_8_PTR_queueName = {
     "queueName", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_8_PTR)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_PTR_prios = {
     "prios", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_portExtBp = {
     "portExtBp", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_PTR_pcbs = {
     "pcbs", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_PTR_pebs = {
     "pebs", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_PTR_probabilityArr = {
     "probabilityArr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_PTR_quantumArrPtr = {
     "quantumArrPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_aNodeInd = {
     "aNodeInd", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_aNodesNum = {
     "aNodesNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_bNodeInd = {
     "bNodeInd", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_bNodesNum = {
     "bNodesNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_bpOffset = {
     "bpOffset", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_bpXoff = {
     "bpXoff", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_bpXon = {
     "bpXon", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_bytes = {
     "bytes", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_cNodeInd = {
     "cNodeInd", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_cNodesNum = {
     "cNodesNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_cbs = {
     "cbs", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_cirBw = {
     "cirBw", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_cosSelector = {
     "cosSelector", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ctrlHeader = {
     "ctrlHeader", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_dpType = {
     "dpType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ebs = {
     "ebs", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_egressElements = {
     "egressElements", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_eirBw = {
     "eirBw", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_eligPrioFunc = {
     "eligPrioFunc", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_eligPrioFuncPtr = {
     "eligPrioFuncPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_firstQueueInRange = {
     "firstQueueInRange", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_maxProbability = {
     "maxProbability", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_nodeAIndex = {
     "nodeAIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_nodeBIndex = {
     "nodeBIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_nodeIndex = {
     "nodeIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_portDp = {
     "portDp", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_portInd = {
     "portInd", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_portIndex = {
     "portIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_profileInd = {
     "profileInd", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_profileOffset = {
     "profileOffset", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_quantum = {
     "quantum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_queueInd = {
     "queueInd", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_queueRangeSize = {
     "queueRangeSize", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_queuesNum = {
     "queuesNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_status = {
     "status", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_wredProfileRef = {
     "wredProfileRef", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U64_HEX_PTR_dataPtr = {
     "dataPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U64_HEX)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U64_HEX_regAddr = {
     "regAddr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U64_HEX)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U64_qmapHeader = {
     "qmapHeader", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U64)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_bapNum = {
     "bapNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TM_A_NODE_PARAMS_STC_PTR_paramsPtr = {
     "paramsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_A_NODE_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TM_BAP_UNIT_ERROR_STATUS_STC_PTR_bapErrorsPtr = {
     "bapErrorsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_BAP_UNIT_ERROR_STATUS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TM_B_NODE_PARAMS_STC_PTR_paramsPtr = {
     "paramsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_B_NODE_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TM_C_NODE_PARAMS_STC_PTR_paramsPtr = {
     "paramsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_C_NODE_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TM_DP_SOURCE_ENT_PTR_sourcePtr = {
     "sourcePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_DP_SOURCE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TM_DROP_PROFILE_PARAMS_STC_PTR_profileStcPtr = {
     "profileStcPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_DROP_PROFILE_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TM_LEVEL_ENT_PTR_levelPtr = {
     "levelPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_LEVEL_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TM_NODE_STATUS_STC_PTR_statusPtr = {
     "statusPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_NODE_STATUS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TM_PORT_DROP_PER_COS_STC_PTR_cosParamsPtr = {
     "cosParamsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_PORT_DROP_PER_COS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TM_PORT_PARAMS_STC_PTR_paramsPtr = {
     "paramsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_PORT_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TM_PORT_STATUS_STC_PTR_statusPtr = {
     "statusPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_PORT_STATUS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TM_QMR_PKT_STATISTICS_STC_PTR_pktCntPtr = {
     "pktCntPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_QMR_PKT_STATISTICS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TM_QUANTUM_LIMITS_STC_PTR_quantumLimitsPtr = {
     "quantumLimitsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_QUANTUM_LIMITS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TM_QUEUE_PARAMS_STC_PTR_paramsPtr = {
     "paramsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_QUEUE_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TM_RCB_PKT_STATISTICS_STC_PTR_pktCntPtr = {
     "pktCntPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_RCB_PKT_STATISTICS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TM_SHAPING_PROFILE_PARAMS_STC_PTR_profileStrPtr = {
     "profileStrPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_SHAPING_PROFILE_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TM_TREE_CHANGE_STC_PTR_changePtr = {
     "changePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_TREE_CHANGE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TM_TREE_PARAMS_STC_PTR_tmTreeParamsPtr = {
     "tmTreeParamsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_TREE_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_TM_UNITS_ERROR_STATUS_STC_PTR_unitsErrorPtr = {
     "unitsErrorPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_TM_UNITS_ERROR_STATUS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_32_PTR_nodeIndexPtr = {
     "nodeIndexPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_8_PTR_PTR_nodeNamePtr = {
     "nodeNamePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_8_PTR)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_prios = {
     "prios", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_aNodeIndPtr = {
     "aNodeIndPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_averageQueueLengthPtr = {
     "averageQueueLengthPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_bNodeIndPtr = {
     "bNodeIndPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_cNodeIndPtr = {
     "cNodeIndPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_curveIndexPtr = {
     "curveIndexPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_instantQueueLengthPtr = {
     "instantQueueLengthPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_portDpPtr = {
     "portDpPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_profileIndPtr = {
     "profileIndPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_profileIndex = {
     "profileIndex", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_queueIndPtr = {
     "queueIndPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_statusPtr = {
     "statusPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_colorNumPtr = {
     "colorNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_resolutionPtr = {
     "resolutionPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U8)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTm2TmIngressDropAqmModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_TM2TM_CHANNEL_ENT_channel,
    &DX_IN_CPSS_TM_COLOR_NUM_ENT_colorNum
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmDropProfileAgingBlockCreate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_TM_DROP_PROFILE_PARAMS_STC_PTR_profileStrArr,
    &DX_OUT_GT_U32_PTR_profileIndexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmDpSourceSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_TM_LEVEL_ENT_level,
    &DX_IN_CPSS_DP_LEVEL_ENT_color,
    &DX_IN_CPSS_TM_DP_SOURCE_ENT_source
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmDpSourceGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_TM_LEVEL_ENT_level,
    &DX_IN_CPSS_DP_LEVEL_ENT_color,
    &DX_OUT_CPSS_TM_DP_SOURCE_ENT_PTR_sourcePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTm2TmEgressDropAqmModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_TM_LEVEL_ENT_level,
    &DX_IN_CPSS_TM_COLOR_NUM_ENT_colorNum,
    &DX_IN_CPSS_TM_DP_SOURCE_ENT_PTR_dpSourcePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmDropColorNumSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_TM_LEVEL_ENT_level,
    &DX_IN_CPSS_TM_COLOR_NUM_ENT_number
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmShapingProfileValidate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_TM_LEVEL_ENT_level,
    &DX_IN_CPSS_TM_SHAPING_PROFILE_PARAMS_STC_PTR_profileStrPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmShapingProfileCreate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_TM_LEVEL_ENT_level,
    &DX_IN_CPSS_TM_SHAPING_PROFILE_PARAMS_STC_PTR_profileStrPtr,
    &DX_OUT_GT_U32_PTR_profileIndPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmGetLogicalNodeIndex_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_TM_LEVEL_ENT_level,
    &DX_IN_GT_8_PTR_nodeName,
    &DX_OUT_GT_32_PTR_nodeIndexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmShapingPeriodicUpdateEnableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_TM_LEVEL_ENT_level,
    &DX_IN_GT_BOOL_status
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmDropProfileCreate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_TM_LEVEL_ENT_level,
    &DX_IN_GT_U32_cos,
    &DX_IN_CPSS_TM_DROP_PROFILE_PARAMS_STC_PTR_profileStcPtr,
    &DX_OUT_GT_U32_PTR_profileIndexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmDropWredTraditionalCurveCreate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_TM_LEVEL_ENT_level,
    &DX_IN_GT_U32_cos,
    &DX_IN_GT_U32_maxProbability,
    &DX_OUT_GT_U32_PTR_curveIndexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmNodeDropProfileIndexRead_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_TM_LEVEL_ENT_level,
    &DX_IN_GT_U32_cos,
    &DX_IN_GT_U32_nodeIndex,
    &DX_OUT_GT_U32_PTR_profileIndex
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmDropProfileDelete_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_TM_LEVEL_ENT_level,
    &DX_IN_GT_U32_cos,
    &DX_IN_GT_U32_profile
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmDropProfileRead_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_TM_LEVEL_ENT_level,
    &DX_IN_GT_U32_cos,
    &DX_IN_GT_U32_profileInd,
    &DX_OUT_CPSS_TM_DROP_PROFILE_PARAMS_STC_PTR_profileStcPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmDropProfileUpdate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_TM_LEVEL_ENT_level,
    &DX_IN_GT_U32_cos,
    &DX_IN_GT_U32_profileIndex,
    &DX_IN_CPSS_TM_DROP_PROFILE_PARAMS_STC_PTR_profileStcPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmNodesCtlNodeDelete_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_TM_LEVEL_ENT_level,
    &DX_IN_GT_U32_index
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmDropQueueLengthGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_TM_LEVEL_ENT_level,
    &DX_IN_GT_U32_index,
    &DX_OUT_GT_U32_PTR_instantQueueLengthPtr,
    &DX_OUT_GT_U32_PTR_averageQueueLengthPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmNodesSwitch_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_TM_LEVEL_ENT_level,
    &DX_IN_GT_U32_nodeAIndex,
    &DX_IN_GT_U32_nodeBIndex
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmGetNodeLogicalName_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_TM_LEVEL_ENT_level,
    &DX_IN_GT_U32_nodeIndex,
    &DX_OUT_GT_8_PTR_PTR_nodeNamePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmShapingProfileUpdate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_TM_LEVEL_ENT_level,
    &DX_IN_GT_U32_profileInd,
    &DX_IN_CPSS_TM_SHAPING_PROFILE_PARAMS_STC_PTR_profileStrPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmShapingProfileRead_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_TM_LEVEL_ENT_level,
    &DX_IN_GT_U32_profileInd,
    &DX_OUT_CPSS_TM_SHAPING_PROFILE_PARAMS_STC_PTR_profileStrPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmDropColorNumResolutionGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_TM_LEVEL_ENT_level,
    &DX_OUT_GT_U8_PTR_colorNumPtr,
    &DX_OUT_GT_U8_PTR_resolutionPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmSchedPeriodicSchemeConfig_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_TM_LEVEL_PERIODIC_PARAMS_STC_PTR_paramsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmInitExt_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_TM_LIB_INIT_PARAMS_STC_PTR_tmLibInitParamsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmSchedPortExternalBpSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_BOOL_portExtBp
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmTreeChangeStatus_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_BOOL_status
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmNamedQueueToAnodeCreate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_aNodeInd,
    &DX_IN_CPSS_TM_QUEUE_PARAMS_STC_PTR_qParamsPtr,
    &DX_IN_GT_8_PTR_queueName,
    &DX_OUT_GT_U32_PTR_queueIndPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmQueueToAnodeByIndexCreate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_aNodeInd,
    &DX_IN_CPSS_TM_QUEUE_PARAMS_STC_PTR_qParamsPtr,
    &DX_IN_GT_U32_queueInd
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmQueueToAnodeCreate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_aNodeInd,
    &DX_IN_CPSS_TM_QUEUE_PARAMS_STC_PTR_qParamsPtr,
    &DX_OUT_GT_U32_PTR_queueIndPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmNamedAnodeToBnodeCreate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_bNodeInd,
    &DX_IN_CPSS_TM_A_NODE_PARAMS_STC_PTR_aParamsPtr,
    &DX_IN_GT_8_PTR_nodeName,
    &DX_OUT_GT_U32_PTR_aNodeIndPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmAnodeToBnodeWithQueuePoolCreate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_bNodeInd,
    &DX_IN_CPSS_TM_A_NODE_PARAMS_STC_PTR_aParamsPtr,
    &DX_IN_GT_U32_firstQueueInRange,
    &DX_IN_GT_U32_queueRangeSize,
    &DX_OUT_GT_U32_PTR_aNodeIndPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmAnodeToBnodeCreate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_bNodeInd,
    &DX_IN_CPSS_TM_A_NODE_PARAMS_STC_PTR_aParamsPtr,
    &DX_OUT_GT_U32_PTR_aNodeIndPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmQueueToBnodeCreate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_bNodeInd,
    &DX_IN_CPSS_TM_QUEUE_PARAMS_STC_PTR_qParamsPtr,
    &DX_IN_CPSS_TM_A_NODE_PARAMS_STC_PTR_aParamsPtr,
    &DX_OUT_GT_U32_PTR_queueIndPtr,
    &DX_OUT_GT_U32_PTR_aNodeIndPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmSchedPortLvlDwrrBytesPerBurstLimitSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_bytes
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmAnodeToCnodeCreate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_cNodeInd,
    &DX_IN_CPSS_TM_A_NODE_PARAMS_STC_PTR_aParamsPtr,
    &DX_IN_CPSS_TM_B_NODE_PARAMS_STC_PTR_bParamsPtr,
    &DX_OUT_GT_U32_PTR_aNodeIndPtr,
    &DX_OUT_GT_U32_PTR_bNodeIndPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmNamedBnodeToCnodeCreate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_cNodeInd,
    &DX_IN_CPSS_TM_B_NODE_PARAMS_STC_PTR_bParamsPtr,
    &DX_IN_GT_8_PTR_nodeName,
    &DX_OUT_GT_U32_PTR_bNodeIndPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmBnodeToCnodeCreate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_cNodeInd,
    &DX_IN_CPSS_TM_B_NODE_PARAMS_STC_PTR_bParamsPtr,
    &DX_OUT_GT_U32_PTR_bNodeIndPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmQueueToCnodeCreate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_cNodeInd,
    &DX_IN_CPSS_TM_QUEUE_PARAMS_STC_PTR_qParamsPtr,
    &DX_IN_CPSS_TM_A_NODE_PARAMS_STC_PTR_aParamsPtr,
    &DX_IN_CPSS_TM_B_NODE_PARAMS_STC_PTR_bParamsPtr,
    &DX_OUT_GT_U32_PTR_queueIndPtr,
    &DX_OUT_GT_U32_PTR_aNodeIndPtr,
    &DX_OUT_GT_U32_PTR_bNodeIndPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmPortShapingValidate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_cirBw,
    &DX_IN_GT_U32_eirBw,
    &DX_IN_GT_U32_PTR_pcbs,
    &DX_IN_GT_U32_PTR_pebs
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTm2TmGlobalConfig_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_cosSelector,
    &DX_IN_CPSS_TM2TM_EXTERNAL_HDR_STC_PTR_extHeadersPtr,
    &DX_IN_CPSS_TM2TM_CNTRL_PKT_STRUCT_STC_PTR_ctrlPacketStrPtr,
    &DX_IN_CPSS_TM2TM_DELTA_RANGE_STC_PTR_rangePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTm2TmDpQueryResponceLevelSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_dpType,
    &DX_IN_GT_U32_portDp,
    &DX_IN_CPSS_TM_LEVEL_ENT_localLevel,
    &DX_IN_CPSS_TM2TM_CHANNEL_ENT_remoteLevel
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTm2TmChannelPortConfig_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_egressElements,
    &DX_IN_CPSS_TM_LEVEL_ENT_srcLevel,
    &DX_IN_CPSS_TM_LEVEL_ENT_bpLevel,
    &DX_IN_GT_U32_bpOffset,
    &DX_IN_GT_U32_bpXon,
    &DX_IN_GT_U32_bpXoff
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmEligPrioFuncQueueConfig_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_eligPrioFunc,
    &DX_IN_CPSS_TM_ELIG_PRIO_FUNC_STC_PTR_funcOutArr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmEligPrioFuncConfigAllLevels_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_eligPrioFuncPtr,
    &DX_IN_CPSS_TM_ELIG_PRIO_FUNC_STC_PTR_funcOutArr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmEligPrioFuncNodeConfig_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_eligPrioFuncPtr,
    &DX_IN_CPSS_TM_LEVEL_ENT_level,
    &DX_IN_CPSS_TM_ELIG_PRIO_FUNC_STC_PTR_funcOutArr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmAnodeUpdate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_IN_CPSS_TM_A_NODE_PARAMS_STC_PTR_paramsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmBnodeUpdate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_IN_CPSS_TM_B_NODE_PARAMS_STC_PTR_paramsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmCnodeUpdate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_IN_CPSS_TM_C_NODE_PARAMS_STC_PTR_paramsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmPortDropCosUpdate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_IN_CPSS_TM_PORT_DROP_PER_COS_STC_PTR_paramsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmQueueUpdate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_IN_CPSS_TM_QUEUE_PARAMS_STC_PTR_paramsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmPortShapingUpdate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_IN_GT_U32_cirBw,
    &DX_IN_GT_U32_eirBw,
    &DX_IN_GT_U32_cbs,
    &DX_IN_GT_U32_ebs
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmDropQueueCosSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_IN_GT_U32_cos
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmPortDropUpdate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_IN_GT_U32_wredProfileRef
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmAnodeConfigurationRead_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_OUT_CPSS_TM_A_NODE_PARAMS_STC_PTR_paramsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmBnodeConfigurationRead_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_OUT_CPSS_TM_B_NODE_PARAMS_STC_PTR_paramsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmCnodeConfigurationRead_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_OUT_CPSS_TM_C_NODE_PARAMS_STC_PTR_paramsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmQueueStatusRead_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_OUT_CPSS_TM_NODE_STATUS_STC_PTR_statusPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmPortConfigurationRead_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_OUT_CPSS_TM_PORT_PARAMS_STC_PTR_paramsPtr,
    &DX_OUT_CPSS_TM_PORT_DROP_PER_COS_STC_PTR_cosParamsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmPortStatusRead_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_OUT_CPSS_TM_PORT_STATUS_STC_PTR_statusPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmQueueConfigurationRead_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_OUT_CPSS_TM_QUEUE_PARAMS_STC_PTR_paramsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmQueueFlushTriggerActionStatusGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_OUT_GT_BOOL_PTR_actFinishedPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmDropQueueCosGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_OUT_GT_U32_PTR_cosPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmDropQueryResponceSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_portDp,
    &DX_IN_CPSS_TM_LEVEL_ENT_level
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmAnodeToPortCreate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_portInd,
    &DX_IN_CPSS_TM_A_NODE_PARAMS_STC_PTR_aParamsPtr,
    &DX_IN_CPSS_TM_B_NODE_PARAMS_STC_PTR_bParamsPtr,
    &DX_IN_CPSS_TM_C_NODE_PARAMS_STC_PTR_cParamsPtr,
    &DX_OUT_GT_U32_PTR_aNodeIndPtr,
    &DX_OUT_GT_U32_PTR_bNodeIndPtr,
    &DX_OUT_GT_U32_PTR_cNodeIndPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmBnodeToPortCreate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_portInd,
    &DX_IN_CPSS_TM_B_NODE_PARAMS_STC_PTR_bParamsPtr,
    &DX_IN_CPSS_TM_C_NODE_PARAMS_STC_PTR_cParamsPtr,
    &DX_OUT_GT_U32_PTR_bNodeIndPtr,
    &DX_OUT_GT_U32_PTR_cNodeIndPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmNamedCnodeToPortCreate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_portInd,
    &DX_IN_CPSS_TM_C_NODE_PARAMS_STC_PTR_cParamsPtr,
    &DX_IN_GT_8_PTR_nodeName,
    &DX_OUT_GT_U32_PTR_cNodeIndPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmCnodeToPortCreate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_portInd,
    &DX_IN_CPSS_TM_C_NODE_PARAMS_STC_PTR_cParamsPtr,
    &DX_OUT_GT_U32_PTR_cNodeIndPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmPortDropPerCosConfig_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_portInd,
    &DX_IN_CPSS_TM_PORT_DROP_PER_COS_STC_PTR_paramsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmAsymPortCreate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_portInd,
    &DX_IN_CPSS_TM_PORT_PARAMS_STC_PTR_paramsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmNamedAsymPortCreate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_portInd,
    &DX_IN_CPSS_TM_PORT_PARAMS_STC_PTR_paramsPtr,
    &DX_IN_GT_8_PTR_portName
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmPortCreate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_portInd,
    &DX_IN_CPSS_TM_PORT_PARAMS_STC_PTR_paramsPtr,
    &DX_IN_GT_U32_cNodesNum,
    &DX_IN_GT_U32_bNodesNum,
    &DX_IN_GT_U32_aNodesNum,
    &DX_IN_GT_U32_queuesNum
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmNamedPortCreate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_portInd,
    &DX_IN_CPSS_TM_PORT_PARAMS_STC_PTR_paramsPtr,
    &DX_IN_GT_U32_cNodesNum,
    &DX_IN_GT_U32_bNodesNum,
    &DX_IN_GT_U32_aNodesNum,
    &DX_IN_GT_U32_queuesNum,
    &DX_IN_GT_8_PTR_portName
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmQueueToPortCreate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_portInd,
    &DX_IN_CPSS_TM_QUEUE_PARAMS_STC_PTR_qParamsPtr,
    &DX_IN_CPSS_TM_A_NODE_PARAMS_STC_PTR_aParamsPtr,
    &DX_IN_CPSS_TM_B_NODE_PARAMS_STC_PTR_bParamsPtr,
    &DX_IN_CPSS_TM_C_NODE_PARAMS_STC_PTR_cParamsPtr,
    &DX_OUT_GT_U32_PTR_queueIndPtr,
    &DX_OUT_GT_U32_PTR_aNodeIndPtr,
    &DX_OUT_GT_U32_PTR_bNodeIndPtr,
    &DX_OUT_GT_U32_PTR_cNodeIndPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmTransQueueToPortCreate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_portInd,
    &DX_IN_CPSS_TM_QUEUE_PARAMS_STC_PTR_qParamsPtr,
    &DX_OUT_GT_U32_PTR_queueIndPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const prvCpssTmDumpPort_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_portIndex
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmShapingProfileDelete_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_profileInd
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmDropProfileAgingBlockDelete_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_profileIndex
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmDropProfileAgingUpdate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_profileIndex,
    &DX_IN_GT_U32_profileOffset,
    &DX_IN_CPSS_TM_DROP_PROFILE_PARAMS_STC_PTR_profileParamsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTm2TmLcConfig_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_quantum,
    &DX_IN_GT_U64_qmapHeader,
    &DX_IN_GT_U32_ctrlHeader,
    &DX_IN_GT_U32_tmPort
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmNodesCtlQueueInstall_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_queueInd
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmNodesCtlQueueInstallStatusGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_queueInd,
    &DX_OUT_GT_U32_PTR_statusPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmAgingChangeStatus_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_status
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmCtlWriteRegister_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U64_HEX_regAddr,
    &DX_IN_GT_U64_HEX_PTR_dataPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmBapUnitErrorStatusGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U8_bapNum,
    &DX_OUT_CPSS_TM_BAP_UNIT_ERROR_STATUS_STC_PTR_bapErrorsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmQmrPktStatisticsGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_TM_QMR_PKT_STATISTICS_STC_PTR_pktCntPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmPortQuantumLimitsGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_TM_QUANTUM_LIMITS_STC_PTR_quantumLimitsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmRcbPktStatisticsGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_TM_RCB_PKT_STATISTICS_STC_PTR_pktCntPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmNodesCtlReadNextChange_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_TM_TREE_CHANGE_STC_PTR_changePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmTreeParamsGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_TM_TREE_PARAMS_STC_PTR_tmTreeParamsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmUnitsErrorStatusGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_TM_UNITS_ERROR_STATUS_STC_PTR_unitsErrorPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmTreeStatusGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_BOOL_PTR_statusPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmTreeDwrrPrioGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_BOOL_PTR_prios
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmDropQueryResponceGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_portDpPtr,
    &DX_OUT_CPSS_TM_LEVEL_ENT_PTR_levelPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmAgingStatusGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_statusPtr
};
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmDropWredCurveCreate_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmPortSchedulingUpdate_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssTmTreeDwrrPrioSet_PARAMS[];


/********* lib API DB *********/

extern void cpssTm2TmEgressDropAqmModeSet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssTmDropWredCurveCreate_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssTmDropProfileAgingBlockCreate_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssTmEligPrioFuncQueueConfig_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssTmEligPrioFuncNodeConfig_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssTmEligPrioFuncNodeConfig_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssTmPortSchedulingUpdate_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssTmPortSchedulingUpdate_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChTmLogLibDb[] = {
    {"cpssTm2TmGlobalConfig", 5, cpssTm2TmGlobalConfig_PARAMS, NULL},
    {"cpssTm2TmChannelPortConfig", 7, cpssTm2TmChannelPortConfig_PARAMS, NULL},
    {"cpssTm2TmChannelNodeConfig", 7, cpssTm2TmChannelPortConfig_PARAMS, NULL},
    {"cpssTm2TmDpQueryResponceLevelSet", 5, cpssTm2TmDpQueryResponceLevelSet_PARAMS, NULL},
    {"cpssTm2TmLcConfig", 5, cpssTm2TmLcConfig_PARAMS, NULL},
    {"cpssTm2TmEgressDropAqmModeSet", 4, cpssTm2TmEgressDropAqmModeSet_PARAMS, cpssTm2TmEgressDropAqmModeSet_preLogic},
    {"cpssTm2TmIngressDropAqmModeSet", 3, cpssTm2TmIngressDropAqmModeSet_PARAMS, NULL},
    {"cpssTm2TmEnable", 1, prvCpssLogGenDevNum_PARAMS, NULL},
    {"cpssTmCtlWriteRegister", 3, cpssTmCtlWriteRegister_PARAMS, NULL},
    {"cpssTmCtlReadRegister", 3, cpssTmCtlWriteRegister_PARAMS, NULL},
    {"cpssTmInit", 1, prvCpssLogGenDevNum_PARAMS, NULL},
    {"cpssTmInitExt", 2, cpssTmInitExt_PARAMS, NULL},
    {"cpssTmClose", 1, prvCpssLogGenDevNum_PARAMS, NULL},
    {"cpssTmTreeParamsGet", 2, cpssTmTreeParamsGet_PARAMS, NULL},
    {"cpssTmUnitsErrorStatusGet", 2, cpssTmUnitsErrorStatusGet_PARAMS, NULL},
    {"cpssTmBapUnitErrorStatusGet", 3, cpssTmBapUnitErrorStatusGet_PARAMS, NULL},
    {"cpssTmDropWredCurveCreate", 5, cpssTmDropWredCurveCreate_PARAMS, cpssTmDropWredCurveCreate_preLogic},
    {"cpssTmDropWredTraditionalCurveCreate", 5, cpssTmDropWredTraditionalCurveCreate_PARAMS, NULL},
    {"cpssTmDropProfileCreate", 5, cpssTmDropProfileCreate_PARAMS, NULL},
    {"cpssTmDropProfileUpdate", 5, cpssTmDropProfileUpdate_PARAMS, NULL},
    {"cpssTmDropProfileDelete", 4, cpssTmDropProfileDelete_PARAMS, NULL},
    {"cpssTmDropProfileRead", 5, cpssTmDropProfileRead_PARAMS, NULL},
    {"cpssTmDropColorNumSet", 3, cpssTmDropColorNumSet_PARAMS, NULL},
    {"cpssTmDropColorNumResolutionGet", 4, cpssTmDropColorNumResolutionGet_PARAMS, NULL},
    {"cpssTmAgingChangeStatus", 2, cpssTmAgingChangeStatus_PARAMS, NULL},
    {"cpssTmAgingStatusGet", 2, cpssTmAgingStatusGet_PARAMS, NULL},
    {"cpssTmDpSourceSet", 4, cpssTmDpSourceSet_PARAMS, NULL},
    {"cpssTmDpSourceGet", 4, cpssTmDpSourceGet_PARAMS, NULL},
    {"cpssTmDropQueryResponceSet", 3, cpssTmDropQueryResponceSet_PARAMS, NULL},
    {"cpssTmDropQueryResponceGet", 3, cpssTmDropQueryResponceGet_PARAMS, NULL},
    {"cpssTmDropQueueCosSet", 3, cpssTmDropQueueCosSet_PARAMS, NULL},
    {"cpssTmDropQueueCosGet", 3, cpssTmDropQueueCosGet_PARAMS, NULL},
    {"cpssTmDropProfileAgingBlockCreate", 3, cpssTmDropProfileAgingBlockCreate_PARAMS, cpssTmDropProfileAgingBlockCreate_preLogic},
    {"cpssTmDropProfileAgingBlockDelete", 2, cpssTmDropProfileAgingBlockDelete_PARAMS, NULL},
    {"cpssTmDropProfileAgingUpdate", 4, cpssTmDropProfileAgingUpdate_PARAMS, NULL},
    {"cpssTmNodeDropProfileIndexRead", 5, cpssTmNodeDropProfileIndexRead_PARAMS, NULL},
    {"cpssTmEligPrioFuncQueueConfig", 3, cpssTmEligPrioFuncQueueConfig_PARAMS, cpssTmEligPrioFuncQueueConfig_preLogic},
    {"cpssTmEligPrioFuncNodeConfig", 4, cpssTmEligPrioFuncNodeConfig_PARAMS, cpssTmEligPrioFuncNodeConfig_preLogic},
    {"cpssTmEligPrioFuncConfigAllLevels", 3, cpssTmEligPrioFuncConfigAllLevels_PARAMS, cpssTmEligPrioFuncNodeConfig_preLogic},
    {"cpssTmNamedPortCreate", 8, cpssTmNamedPortCreate_PARAMS, NULL},
    {"cpssTmNamedAsymPortCreate", 4, cpssTmNamedAsymPortCreate_PARAMS, NULL},
    {"cpssTmNamedQueueToAnodeCreate", 5, cpssTmNamedQueueToAnodeCreate_PARAMS, NULL},
    {"cpssTmNamedAnodeToBnodeCreate", 5, cpssTmNamedAnodeToBnodeCreate_PARAMS, NULL},
    {"cpssTmNamedBnodeToCnodeCreate", 5, cpssTmNamedBnodeToCnodeCreate_PARAMS, NULL},
    {"cpssTmNamedCnodeToPortCreate", 5, cpssTmNamedCnodeToPortCreate_PARAMS, NULL},
    {"cpssTmGetLogicalNodeIndex", 4, cpssTmGetLogicalNodeIndex_PARAMS, NULL},
    {"cpssTmGetNodeLogicalName", 4, cpssTmGetNodeLogicalName_PARAMS, NULL},
    {"cpssTmPortCreate", 7, cpssTmPortCreate_PARAMS, NULL},
    {"cpssTmAsymPortCreate", 3, cpssTmAsymPortCreate_PARAMS, NULL},
    {"cpssTmPortDropPerCosConfig", 3, cpssTmPortDropPerCosConfig_PARAMS, NULL},
    {"cpssTmQueueToPortCreate", 10, cpssTmQueueToPortCreate_PARAMS, NULL},
    {"cpssTmTransQueueToPortCreate", 4, cpssTmTransQueueToPortCreate_PARAMS, NULL},
    {"cpssTmQueueToCnodeCreate", 8, cpssTmQueueToCnodeCreate_PARAMS, NULL},
    {"cpssTmQueueToBnodeCreate", 6, cpssTmQueueToBnodeCreate_PARAMS, NULL},
    {"cpssTmQueueToAnodeCreate", 4, cpssTmQueueToAnodeCreate_PARAMS, NULL},
    {"cpssTmQueueToAnodeByIndexCreate", 4, cpssTmQueueToAnodeByIndexCreate_PARAMS, NULL},
    {"cpssTmAnodeToPortCreate", 8, cpssTmAnodeToPortCreate_PARAMS, NULL},
    {"cpssTmAnodeToCnodeCreate", 6, cpssTmAnodeToCnodeCreate_PARAMS, NULL},
    {"cpssTmAnodeToBnodeCreate", 4, cpssTmAnodeToBnodeCreate_PARAMS, NULL},
    {"cpssTmAnodeToBnodeWithQueuePoolCreate", 6, cpssTmAnodeToBnodeWithQueuePoolCreate_PARAMS, NULL},
    {"cpssTmBnodeToPortCreate", 6, cpssTmBnodeToPortCreate_PARAMS, NULL},
    {"cpssTmBnodeToCnodeCreate", 4, cpssTmBnodeToCnodeCreate_PARAMS, NULL},
    {"cpssTmCnodeToPortCreate", 4, cpssTmCnodeToPortCreate_PARAMS, NULL},
    {"cpssTmNodesCtlQueueInstall", 2, cpssTmNodesCtlQueueInstall_PARAMS, NULL},
    {"cpssTmNodesCtlQueueUninstall", 2, cpssTmNodesCtlQueueInstall_PARAMS, NULL},
    {"cpssTmNodesCtlQueueInstallStatusGet", 3, cpssTmNodesCtlQueueInstallStatusGet_PARAMS, NULL},
    {"cpssTmNodesCtlNodeDelete", 3, cpssTmNodesCtlNodeDelete_PARAMS, NULL},
    {"cpssTmNodesCtlTransPortDelete", 2, prvCpssLogGenDevNumIndex_PARAMS, NULL},
    {"cpssTmQueueFlush", 2, prvCpssLogGenDevNumIndex_PARAMS, NULL},
    {"cpssTmQueueFlushTriggerActionStart", 2, prvCpssLogGenDevNumIndex_PARAMS, NULL},
    {"cpssTmQueueFlushTriggerActionStatusGet", 3, cpssTmQueueFlushTriggerActionStatusGet_PARAMS, NULL},
    {"cpssTmPortFlush", 2, prvCpssLogGenDevNumIndex_PARAMS, NULL},
    {"cpssTmPortFlushTriggerActionStart", 2, prvCpssLogGenDevNumIndex_PARAMS, NULL},
    {"cpssTmPortFlushTriggerActionStatusGet", 3, cpssTmQueueFlushTriggerActionStatusGet_PARAMS, NULL},
    {"cpssTmNodesCtlReadNextChange", 2, cpssTmNodesCtlReadNextChange_PARAMS, NULL},
    {"cpssTmNodesCtlCleanList", 1, prvCpssLogGenDevNum_PARAMS, NULL},
    {"cpssTmQueueConfigurationRead", 3, cpssTmQueueConfigurationRead_PARAMS, NULL},
    {"cpssTmAnodeConfigurationRead", 3, cpssTmAnodeConfigurationRead_PARAMS, NULL},
    {"cpssTmBnodeConfigurationRead", 3, cpssTmBnodeConfigurationRead_PARAMS, NULL},
    {"cpssTmCnodeConfigurationRead", 3, cpssTmCnodeConfigurationRead_PARAMS, NULL},
    {"cpssTmPortConfigurationRead", 4, cpssTmPortConfigurationRead_PARAMS, NULL},
    {"cpssTmNodesSwitch", 4, cpssTmNodesSwitch_PARAMS, NULL},
    {"cpssTmQueueStatusRead", 3, cpssTmQueueStatusRead_PARAMS, NULL},
    {"cpssTmAnodeStatusRead", 3, cpssTmQueueStatusRead_PARAMS, NULL},
    {"cpssTmBnodeStatusRead", 3, cpssTmQueueStatusRead_PARAMS, NULL},
    {"cpssTmCnodeStatusRead", 3, cpssTmQueueStatusRead_PARAMS, NULL},
    {"cpssTmPortStatusRead", 3, cpssTmPortStatusRead_PARAMS, NULL},
    {"cpssTmDropQueueLengthGet", 5, cpssTmDropQueueLengthGet_PARAMS, NULL},
    {"cpssTmTreeChangeStatus", 2, cpssTmTreeChangeStatus_PARAMS, NULL},
    {"cpssTmTreeStatusGet", 2, cpssTmTreeStatusGet_PARAMS, NULL},
    {"cpssTmTreeDwrrPrioSet", 2, cpssTmTreeDwrrPrioSet_PARAMS, cpssTmPortSchedulingUpdate_preLogic},
    {"cpssTmTreeDwrrPrioGet", 2, cpssTmTreeDwrrPrioGet_PARAMS, NULL},
    {"cpssTmQueueUpdate", 3, cpssTmQueueUpdate_PARAMS, NULL},
    {"cpssTmAnodeUpdate", 3, cpssTmAnodeUpdate_PARAMS, NULL},
    {"cpssTmBnodeUpdate", 3, cpssTmBnodeUpdate_PARAMS, NULL},
    {"cpssTmCnodeUpdate", 3, cpssTmCnodeUpdate_PARAMS, NULL},
    {"cpssTmPortShapingUpdate", 6, cpssTmPortShapingUpdate_PARAMS, NULL},
    {"cpssTmPortSchedulingUpdate", 5, cpssTmPortSchedulingUpdate_PARAMS, cpssTmPortSchedulingUpdate_preLogic},
    {"cpssTmPortDropUpdate", 3, cpssTmPortDropUpdate_PARAMS, NULL},
    {"cpssTmPortDropCosUpdate", 3, cpssTmPortDropCosUpdate_PARAMS, NULL},
    {"cpssTmSchedPortExternalBpSet", 2, cpssTmSchedPortExternalBpSet_PARAMS, NULL},
    {"cpssTmSchedPeriodicSchemeConfig", 2, cpssTmSchedPeriodicSchemeConfig_PARAMS, NULL},
    {"cpssTmShapingPeriodicUpdateEnableSet", 3, cpssTmShapingPeriodicUpdateEnableSet_PARAMS, NULL},
    {"cpssTmShapingPeriodicUpdateStatusGet", 2, cpssTmSchedPeriodicSchemeConfig_PARAMS, NULL},
    {"cpssTmSchedPortLvlDwrrBytesPerBurstLimitSet", 2, cpssTmSchedPortLvlDwrrBytesPerBurstLimitSet_PARAMS, NULL},
    {"cpssTmPortQuantumLimitsGet", 2, cpssTmPortQuantumLimitsGet_PARAMS, NULL},
    {"cpssTmNodeQuantumLimitsGet", 2, cpssTmPortQuantumLimitsGet_PARAMS, NULL},
    {"cpssTmToCpssErrCodesInit", 0, NULL, NULL},
    {"cpssTmShapingProfileCreate", 4, cpssTmShapingProfileCreate_PARAMS, NULL},
    {"cpssTmShapingProfileDelete", 2, cpssTmShapingProfileDelete_PARAMS, NULL},
    {"cpssTmShapingProfileRead", 4, cpssTmShapingProfileRead_PARAMS, NULL},
    {"cpssTmShapingProfileUpdate", 4, cpssTmShapingProfileUpdate_PARAMS, NULL},
    {"cpssTmShapingProfileValidate", 3, cpssTmShapingProfileValidate_PARAMS, NULL},
    {"cpssTmPortShapingValidate", 5, cpssTmPortShapingValidate_PARAMS, NULL},
    {"cpssTmQmrPktStatisticsGet", 2, cpssTmQmrPktStatisticsGet_PARAMS, NULL},
    {"cpssTmRcbPktStatisticsGet", 2, cpssTmRcbPktStatisticsGet_PARAMS, NULL},
    {"prvCpssTmDumpPort", 2, prvCpssTmDumpPort_PARAMS, NULL},
    {"prvCpssTmCtlDumpPortHW", 2, prvCpssTmDumpPort_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_TM(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChTmLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChTmLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

