/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssPxPortLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssPxPortLogh
#define __prvCpssPxPortLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_MIN_SPEED_ENT);
PRV_CPSS_LOG_STC_ENUM_MAP_DECLARE_MAC(CPSS_PX_PA_UNIT_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PORT_CN_PACKET_LENGTH_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PORT_CPLL_INPUT_FREQUENCY_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PORT_CTLE_BIAS_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PORT_FC_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PORT_MAC_COUNTER_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_MAP_DECLARE_MAC(CPSS_PX_PORT_MAPPING_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PORT_PFC_COUNT_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PORT_PFC_ENABLE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PORT_TX_SHAPER_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PORT_AP_FLOW_CONTROL_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PORT_TX_SHAPER_GRANULARITY_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_PX_DETAILED_PORT_MAP_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_PORT_AP_INTROP_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_PORT_AP_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_PORT_AP_STATS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_PORT_AP_STATUS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_PORT_CN_PROFILE_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_PORT_ECN_ENABLERS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_PORT_EGRESS_CNTR_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_PORT_MAC_COUNTERS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_PORT_MAP_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_PORT_SERDES_EYE_INPUT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_PORT_SERDES_EYE_RESULT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_PORT_STAT_TX_DROP_COUNTERS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_PORT_TX_SHAPER_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_DEV_UNITS_PIZZA_ARBITER_STATE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_SHADOW_PORT_MAP_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_PA_UNIT_ENT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_PIZZA_ARBITER_UNIT_STATE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_PX_DETAILED_PORT_MAP_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_MIN_SPEED_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_MIN_SPEED_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PA_UNIT_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_AP_INTROP_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_AP_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_AP_STATS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_AP_STATUS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_CN_PACKET_LENGTH_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_CN_PACKET_LENGTH_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_CN_PROFILE_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_CPLL_INPUT_FREQUENCY_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_CTLE_BIAS_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_CTLE_BIAS_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_ECN_ENABLERS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_EGRESS_CNTR_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_FC_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_FC_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_MAC_COUNTERS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_MAC_COUNTER_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_MAPPING_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_MAP_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_PFC_COUNT_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_PFC_COUNT_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_PFC_ENABLE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_PFC_ENABLE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_SERDES_EYE_INPUT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_SERDES_EYE_RESULT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_STAT_TX_DROP_COUNTERS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_SHAPER_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_SHAPER_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_SHAPER_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_INOUT_CPSS_PM_PORT_PARAMS_STC_PTR_portParamsStcPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_INOUT_GT_U32_PTR_maxRatePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_ADJUST_OPERATION_ENT_bcOp;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_DP_LEVEL_ENT_dpLevel;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_IP_PROTOCOL_STACK_ENT_protocolStack;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PM_PORT_PARAMS_STC_PTR_portParamsStcPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORTS_BMP_STC_PTR_portsBmpPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_AP_SERDES_RX_CONFIG_STC_PTR_rxOverrideParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_AP_SERDES_TX_OFFSETS_STC_PTR_serdesTxOffsetsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_DIRECTION_ENT_direction;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_DIRECTION_ENT_portDirection;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_DUPLEX_ENT_dMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_EGRESS_CNT_MODE_ENT_setModeBmp;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_FEC_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_FLOW_CONTROL_ENT_state;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_INTERFACE_MODE_ENT_ifMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_MANAGER_GLOBAL_PARAMS_STC_PTR_globalParamsStcPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_MANAGER_PORT_TYPE_ENT_portType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_MANAGER_STC_PTR_portEventStcPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_MANAGER_UPDATE_PARAMS_STC_PTR_updateParamsStcPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_PCS_RESET_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT_enable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_REF_CLOCK_SOURCE_ENT_refClockSource;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_RX_FC_PROFILE_SET_ENT_profileSet;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT_portTuningMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_SERDES_RX_CONFIG_STC_PTR_serdesRxCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_SERDES_SPEED_ENT_serdesFrequency;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_SERDES_TUNE_STC_PTR_tuneValuesPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_SERDES_TX_CONFIG_STC_PTR_serdesTxCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_SPEED_ENT_speed;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_XGMII_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_XG_FIXED_IPG_ENT_ipgBase;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_MIN_SPEED_ENT_minimalPortSpeedMBps;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PA_UNIT_ENT_unit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_AP_INTROP_STC_PTR_apIntropPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_AP_PARAMS_STC_PTR_apParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC_PTR_portAnAdvertismentPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC_PTR_cnmGenerationCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC_PTR_fbCalcCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT_mType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_CN_PACKET_LENGTH_ENT_packetLength;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_CN_PROFILE_CONFIG_STC_PTR_cnProfileCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC_PTR_entryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_CPLL_INPUT_FREQUENCY_ENT_inputFreq;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_ENT_outputFreq;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_CTLE_BIAS_MODE_ENT_PTR_ctleBiasValuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_CTLE_BIAS_MODE_ENT_ctleBiasValue;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_ECN_ENABLERS_STC_PTR_enablersPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_FC_MODE_ENT_fcMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_MAC_COUNTER_ENT_cntrName;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT_counterMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_MAPPING_TYPE_ENT_origPortType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_MAP_STC_PTR_portMapArrayPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_PFC_COUNT_MODE_ENT_pfcCountMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_PFC_ENABLE_ENT_pfcEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC_PTR_pfcProfileCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_SERDES_EYE_INPUT_STC_PTR_eye_inputPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT_value;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT_recoveryClkType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT_clockSelect;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT_arbGroup;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT_bcMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT_profile;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT_profileSet;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT_wrrMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT_wrrMtu;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_TX_SHAPER_CONFIG_STC_PTR_configsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_TX_SHAPER_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT_portAlpha;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT_profile;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT_profileSet;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC_PTR_tailDropProfileParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT_enableMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT_policy;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC_PTR_enablerPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC_PTR_maskLsbPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_PTR_overrideEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_apEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_avbModeEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_invertRx;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_invertTx;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_overrideEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_pauseAdvertise;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_powerUp;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_send;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_srvCpuEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_state;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_status;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_vosOverride;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_ETHERADDR_PTR_macPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_PHYSICAL_PORT_NUM_firstPhysicalPortNumber;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_PORT_NUM_targetPort;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_PTR_rxSerdesLaneArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_PTR_serdesOptAlgBmpPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_PTR_txSerdesLaneArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_almostFullThreshold;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_availableBuff;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_baseline;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_bcValue;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_buffsNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_burstSize;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_cntrIdx;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_cntrNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_cntrSetNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_cpllNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_dp;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_dropThreshold;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_fullThreshold;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_ipg;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_laneBmp;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_length;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_limit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_maxBuffNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_maxDescNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_maxSharedBufferLimit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_mcastMaxBuffNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_mcastMaxDescNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_mruSize;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_numCrcBytes;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_numOfBits;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_origPortNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_pfcCounterNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_pfcTimer;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_pipeBandwithInGbps;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_poolNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_portGroup;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_portGroupId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_portGroupNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_portMapArraySize;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_portMaxBuffLimit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_portMaxDescLimit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_profileIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_qcnSampleInterval;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_qlenOld;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_rxBufLimit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_serdesNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_serdesOptAlgBmp;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_shaperToPortRateRatio;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_squelch;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_tailDropDumpBmp;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_tc;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_tcQueue;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_threshold;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_timer;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_udbpIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_value;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_wrrWeight;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_xoffLimit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_xoffThreshold;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_xonLimit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_xonThreshold;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U8_entryIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U8_macSaLsb;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U8_max_LF;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U8_min_LF;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U8_number;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U8_serdesLane;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U8_tcQueue;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_MV_HWS_AP_DEBUG_LOG_OUTPUT_ENT_output;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_ADJUST_OPERATION_ENT_PTR_bcOpPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_DP_LEVEL_ENT_PTR_dpLevelPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_FCFEC_COUNTERS_STC_PTR_fcfecCountersPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PM_PORT_PARAMS_STC_PTR_portParamsStcPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_ATTRIBUTES_STC_PTR_portAttributSetArrayPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_DUPLEX_ENT_PTR_dModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_EGRESS_CNT_MODE_ENT_PTR_setModeBmpPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_FEC_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_FLOW_CONTROL_ENT_PTR_statePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_INTERFACE_MODE_ENT_PTR_ifModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_MAC_CG_COUNTER_SET_STC_PTR_cgMibStcPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_MAC_STATUS_STC_PTR_portMacStatusPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_MAC_TYPE_ENT_PTR_portMacTypePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_MANAGER_STATISTICS_STC_PTR_portStatStcPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_MANAGER_STATUS_STC_PTR_portStagePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT_PTR_enablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_REF_CLOCK_SOURCE_ENT_PTR_refClockSourcePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_RX_FC_PROFILE_SET_ENT_PTR_profileSetPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT_PTR_rxTuneStatusPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT_PTR_txTuneStatusPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_SERDES_RX_CONFIG_STC_PTR_serdesRxCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_SERDES_TUNE_STC_PTR_serdesTunePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_SERDES_TUNE_STC_PTR_tuneValuesPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_SERDES_TX_CONFIG_STC_PTR_serdesTxCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_SPEED_ENT_PTR_speedPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_XGMII_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_XG_FIXED_IPG_ENT_PTR_ipgBasePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_DETAILED_PORT_MAP_STC_PTR_portMapShadowPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC_PTR_pizzaDeviceStatePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_MIN_SPEED_ENT_PTR_minimalPortSpeedResolutionInMBpsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PACKET_TYPE_PTR_packetTypePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_AP_INTROP_STC_PTR_apIntropPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_AP_PARAMS_STC_PTR_apParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_AP_STATS_STC_PTR_apStatsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_AP_STATUS_STC_PTR_apStatusPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC_PTR_portAnAdvertismentPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC_PTR_cnmGenerationCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC_PTR_fbCalcCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT_PTR_mTypePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_CN_PACKET_LENGTH_ENT_PTR_packetLengthPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_CN_PROFILE_CONFIG_STC_PTR_cnProfileCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC_PTR_entryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_ECN_ENABLERS_STC_PTR_enablersPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_EGRESS_CNTR_STC_PTR_egrCntrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_FC_MODE_ENT_PTR_fcModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_MAC_COUNTERS_STC_PTR_countersPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_MAC_COUNTERS_STC_PTR_portMacCounterSetArrayPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT_PTR_counterModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_MAP_STC_PTR_portMapArrayPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_PFC_COUNT_MODE_ENT_PTR_pfcCountModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_PFC_ENABLE_ENT_PTR_pfcEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC_PTR_pfcProfileCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_SERDES_EYE_RESULT_STC_PTR_eye_resultsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_STAT_TX_DROP_COUNTERS_STC_PTR_dropCntrStcPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT_PTR_valuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT_PTR_arbGroupPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT_PTR_bcModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT_PTR_profileSetPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT_PTR_wrrModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT_PTR_wrrMtuPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_TX_SHAPER_CONFIG_STC_PTR_configsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_TX_SHAPER_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT_PTR_portAlphaPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT_PTR_profileSetPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC_PTR_tailDropProfileParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT_PTR_enableModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT_PTR_policyPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC_PTR_enablerPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC_PTR_maskLsbPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_RSFEC_COUNTERS_STC_PTR_rsfecCountersPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_apEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_avbModeEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_captureIsDonePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_cdrLockPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_enabledPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_gbLockPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_invertRx;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_invertRxPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_invertTx;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_invertTxPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_isCpuPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_isLinkUpPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_isLocalFaultPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_isRemoteFaultPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_isValidPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_overrideEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_pauseAdvertisePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_signalStatePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_srvCpuEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_statePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_statusPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_supportedPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_syncPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_vosOverridePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_ETHERADDR_PTR_macPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_PHYSICAL_PORT_NUM_PTR_physicalPortNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_PHYSICAL_PORT_NUM_PTR_portNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_PORT_NUM_PTR_targetPortPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U16_PTR_intropAbilityMaxIntervalPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U16_PTR_numberPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_almostFullThresholdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_availableBuffPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_baselinePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_baudRatePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_bcValuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_buffsNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_burstSizePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_cntrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_dfeResPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_dropThresholdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_fullThresholdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_ipgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_laneNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_lengthPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_limitPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_maxBuffNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_maxDescNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_maxRatePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_maxSharedBufferLimitPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_mcCntrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_mcastMaxBuffNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_mcastMaxDescNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_mruSizePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_numCrcBytesPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_numOfBuffersPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_numOfPacketsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_numberPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_pfcCounterNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_pfcCounterValuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_pipeBandwithInGbpsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_poolNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_portMaxBuffLimitPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_portMaxDescLimitPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_profileIndexPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_qcnSampleIntervalPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_qlenOldPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_rxBufLimitPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_rxSerdesLaneArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_shaperToPortRateRatioPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_tcPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_tcQueuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_thresholdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_timerPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_txSerdesLaneArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_wrrWeightPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_xoffLimitPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_xoffThresholdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_xonLimitPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_xonThresholdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U64_PTR_cntrValuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U8_PTR_macSaLsbPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U8_PTR_numberPtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssPxPortApEnableSet_E = (CPSS_LOG_LIB_PORT_E << 16) | (2 << 24),
    PRV_CPSS_LOG_FUNC_cpssPxPortApEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortApPortConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortApPortConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortApPortEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortApPortStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortApStatsGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortApStatsReset_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortApIntropSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortApIntropGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortApDebugInfoGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortApPortEnableCtrlSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortApPortEnableCtrlGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortApSerdesRxParametersManualSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortApSerdesTxParametersOffsetSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortAutoNegAdvertismentConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortAutoNegAdvertismentConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortAutoNegMasterModeEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortAutoNegMasterModeEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortInbandAutoNegRestart_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortBufMgGlobalXonLimitSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortBufMgGlobalXonLimitGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortBufMgGlobalXoffLimitSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortBufMgGlobalXoffLimitGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortBufMgRxProfileSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortBufMgRxProfileGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortBufMgProfileXonLimitSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortBufMgProfileXonLimitGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortBufMgProfileXoffLimitSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortBufMgProfileXoffLimitGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortBufMgProfileRxBufLimitSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortBufMgProfileRxBufLimitGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortBufMgGlobalRxBufNumberGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortBufMgRxBufNumberGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortBufMgGlobalPacketNumberGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortBufMgRxMcCntrGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortBufMgTxDmaBurstLimitEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortBufMgTxDmaBurstLimitEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortBufMgTxDmaBurstLimitThresholdsSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortBufMgTxDmaBurstLimitThresholdsGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortCnModeEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortCnModeEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortCnPacketLengthSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortCnPacketLengthGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortCnSampleEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortCnSampleEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortCnFbCalcConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortCnFbCalcConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortCnMessageTriggeringStateSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortCnMessageTriggeringStateGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortCnMessageGenerationConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortCnMessageGenerationConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortCnMessagePortMapEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortCnMessagePortMapEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortCnTerminationEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortCnTerminationEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortCnFcEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortCnFcEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortCnFcTimerSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortCnFcTimerGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortCnQueueStatusModeEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortCnQueueStatusModeEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortCnProfileQueueConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortCnProfileQueueConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortCnAvailableBuffSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortCnAvailableBuffGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortCnMessageTypeSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortCnMessageTypeGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortForceLinkPassEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortForceLinkPassEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortForceLinkDownEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortForceLinkDownEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortMruSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortMruGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortLinkStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortInternalLoopbackEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortInternalLoopbackEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortCpllCfgInit_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortFaultSendSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortDuplexAutoNegEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortDuplexAutoNegEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortFlowCntrlAutoNegEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortFlowCntrlAutoNegEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortFlowControlEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortFlowControlEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortFlowControlModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortFlowControlModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortDuplexModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortDuplexModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortAttributesOnPortGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSpeedAutoNegEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSpeedAutoNegEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortBackPressureEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortBackPressureEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortCrcCheckEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortCrcCheckEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortExcessiveCollisionDropEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortExcessiveCollisionDropEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortInBandAutoNegBypassEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortInBandAutoNegBypassEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortMacResetStateSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortForward802_3xEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortForward802_3xEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortForwardUnknownMacControlFramesEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortForwardUnknownMacControlFramesEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortInbandAutoNegEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortInbandAutoNegEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortMacTypeGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortIpgSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortIpgGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortIpgBaseSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortIpgBaseGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPreambleLengthSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPreambleLengthGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPeriodicFcEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPeriodicFcEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPeriodicFlowControlCounterSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPeriodicFlowControlCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortMacSaLsbSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortMacSaLsbGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortMacSaBaseSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortMacSaBaseGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortXGmiiModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortXGmiiModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortExtraIpgSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortExtraIpgGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortXgmiiLocalFaultGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortXgmiiRemoteFaultGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortMacStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPaddingEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPaddingEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortCtleBiasOverrideEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortCtleBiasOverrideEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortVosOverrideControlModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortVosOverrideControlModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortXlgReduceAverageIPGSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortXlgReduceAverageIPGGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortCrcNumBytesSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortCrcNumBytesGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxFcFecCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxRsFecCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortEcnMarkingEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortEcnMarkingEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortEcnMarkingTailDropProfileEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortEcnMarkingTailDropProfileEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortInterfaceModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortInterfaceSpeedSupportGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortFecModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortFecModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortRefClockSourceOverrideEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortRefClockSourceOverrideEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortModeSpeedSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortManagerEventSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortManagerStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortManagerPortParamsSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortManagerPortParamsGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortManagerPortParamsStructInit_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortManagerGlobalParamsOverride_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortManagerPortParamsUpdate_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortManagerInit_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortManagerEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortManagerStatGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortManagerStatClear_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPhysicalPortMapSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPhysicalPortMapGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPhysicalPortMapIsValidGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPhysicalPortMapReverseMappingGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPhysicalPortDetailedMapGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPhysicalPortMapIsCpuGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPcsLoopbackModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPcsLoopbackModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPcsResetSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPcsGearBoxStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPcsSyncStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPcsSyncStableStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortXgLanesSwapEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortXgLanesSwapEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortXgPscLanesSwapSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortXgPscLanesSwapGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPfcEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPfcEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPfcProfileIndexSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPfcProfileIndexGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPfcDbaAvailableBuffersSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPfcDbaAvailableBuffersGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPfcProfileQueueConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPfcProfileQueueConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPfcCountingModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPfcCountingModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPfcGlobalDropEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPfcGlobalDropEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPfcGlobalQueueConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPfcGlobalQueueConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPfcSourcePortToPfcCounterSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPfcSourcePortToPfcCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPfcCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPfcTimerMapEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPfcTimerMapEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPfcTimerToQueueMapSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPfcTimerToQueueMapGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPfcShaperToPortRateRatioSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPfcShaperToPortRateRatioGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPfcForwardEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPfcForwardEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPfcPacketClassificationEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPfcPacketTypeGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPfcQueueCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPfcXonMessageFilterEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPfcXonMessageFilterEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSerdesPowerStatusSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSerdesTuningSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSerdesTuningGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSerdesLaneTuningSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSerdesLaneTuningGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSerdesResetStateSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSerdesAutoTune_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSerdesAutoTuneExt_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSerdesErrorInject_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSerdesAutoTuneStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSerdesLanePolarityGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSerdesLanePolaritySet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSerdesPolaritySet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSerdesPolarityGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSerdesLoopbackModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSerdesLoopbackModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortEomDfeResGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSerdesEyeMatrixGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortEomBaudRateGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSerdesTxEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSerdesTxEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSerdesManualTxConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSerdesManualTxConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSerdesManualRxConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSerdesManualRxConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSerdesSquelchSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSerdesSignalDetectGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSerdesStableSignalDetectGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSerdesLaneSignalDetectGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSerdesCDRLockStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSerdesAutoTuneOptAlgSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSerdesAutoTuneOptAlgGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSerdesAutoTuneResultsGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSerdesEnhancedAutoTune_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSpeedSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSpeedGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortMacCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortMacCountersOnPortGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortMacCountersClearOnReadSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortMacCountersClearOnReadGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortMacOversizedPacketsCounterModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortMacOversizedPacketsCounterModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortMacCountersEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortMacCountersEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortMacCounterCaptureGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortMacCountersCaptureOnPortGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortMacCountersCaptureTriggerSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortMacCountersCaptureTriggerGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortMacCountersOnCgPortGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortStatTxDebugCountersGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortEgressCntrModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortEgressCntrModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortEgressCntrsGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortMacCountersRxHistogramEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortMacCountersTxHistogramEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortMacCountersRxHistogramEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortMacCountersTxHistogramEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSyncEtherRecoveryClkConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSyncEtherRecoveryClkConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSyncEtherRecoveryClkDividerValueSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortSyncEtherRecoveryClkDividerValueGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxDebugQueueingEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxDebugQueueingEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxDebugQueueTxEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxDebugQueueTxEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxDebugResourceHistogramThresholdSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxDebugResourceHistogramThresholdGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxDebugResourceHistogramCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxDebugGlobalDescLimitSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxDebugGlobalDescLimitGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxDebugGlobalQueueTxEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxDebugGlobalQueueTxEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxDebugDescNumberGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxDebugQueueDumpAll_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxSchedulerProfileIdSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxSchedulerProfileIdGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxSchedulerWrrMtuSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxSchedulerWrrMtuGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxSchedulerWrrProfileSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxSchedulerWrrProfileGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxSchedulerArbitrationGroupSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxSchedulerArbitrationGroupGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxSchedulerProfileByteCountChangeEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxSchedulerProfileByteCountChangeEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxSchedulerShaperByteCountChangeValueSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxSchedulerShaperByteCountChangeValueGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxSchedulerProfileCountModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxSchedulerProfileCountModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxShaperEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxShaperEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxShaperQueueEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxShaperQueueEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxShaperProfileSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxShaperProfileGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxShaperModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxShaperModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxShaperBaselineSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxShaperBaselineGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxShaperConfigurationSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxShaperConfigurationGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxShaperQueueProfileSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxShaperQueueProfileGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropProfileIdSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropProfileIdGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropUcEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropUcEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropProfileSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropProfileGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropPacketModeLengthSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropPacketModeLengthGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropProfileBufferConsumptionModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropProfileBufferConsumptionModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropMaskSharedBuffEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropMaskSharedBuffEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropSharedBuffMaxLimitSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropSharedBuffMaxLimitGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropWrtdMasksSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropWrtdMasksGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropProfileTcSharingSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropProfileTcSharingGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropProfileTcSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropProfileTcGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropTcBuffNumberGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropMcastPcktDescLimitSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropMcastPcktDescLimitGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropMcastBuffersLimitSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropMcastBuffersLimitGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropMcastDescNumberGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropMcastBuffNumberGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropSharedPolicySet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropSharedPolicyGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropSharedPoolLimitsSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropSharedPoolLimitsGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropSharedResourceDescNumberGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropSharedResourceBuffNumberGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropGlobalDescNumberGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropGlobalBuffNumberGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropDescNumberGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropBuffNumberGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropDbaModeEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropDbaModeEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropDbaAvailableBuffSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortTxTailDropDbaAvailableBuffGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPizzaArbiterDevStateGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPizzaArbiterPipeBWMinPortSpeedResolutionSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortPizzaArbiterPipeBWMinPortSpeedResolutionGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortDynamicPizzaArbiterIfWorkConservingModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPortDynamicPizzaArbiterIfWorkConservingModeGet_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssPxPortLogh */
