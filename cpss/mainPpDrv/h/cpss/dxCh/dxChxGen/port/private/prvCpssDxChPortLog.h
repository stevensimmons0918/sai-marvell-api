/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChPortLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssDxChPortLogh
#define __prvCpssDxChPortLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* manually implemented declarations *********/

void prvCpssLogParamFunc_CPSS_DXCH_PORT_MAP_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_DIAG_PIZZA_ARBITER_UNIT_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_MIN_SPEED_ENT);
PRV_CPSS_LOG_STC_ENUM_MAP_DECLARE_MAC(CPSS_DXCH_PA_UNIT_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_BUFFERS_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_CN_CONGESTED_Q_PRIORITY_LOCATION_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_CN_MESSAGE_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_CN_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_CN_PACKET_LENGTH_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_CN_PRIORITY_SPEED_LOCATION_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_COM_PHY_H_SUB_SEQ_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_CPLL_INPUT_FREQUENCY_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_CPLL_OUTPUT_FREQUENCY_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_CTLE_BIAS_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_FC_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_FEC_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_GROUP_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_HOL_FC_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_MAC_BR_COUNTER_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_MAP_DECLARE_MAC(CPSS_DXCH_PORT_MAPPING_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_MEMORY_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_SCOPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_VALUE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_PCS_LOOPBACK_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_PERIODIC_FC_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_PFC_COUNT_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_PFC_ENABLE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_PFC_RESPONCE_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPES_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPES_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_PIP_PROTOCOL_ENT);
PRV_CPSS_LOG_STC_ENUM_MAP_DECLARE_MAC(CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_SERDES_LOOPBACK_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_SERDES_SPEED_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_TXQ_OFFSET_WIDTH_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_TX_BC_CHANGE_ENABLE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_TX_SHARED_POLICY_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_TX_TOKEN_BUCKET_MTU_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_UNITS_ID_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_QUEUE_OFFSET_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PORT_TX_PFC_AVAILABLE_BUFF_CONF_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PORT_TX_PFC_RESOURCE_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PORT_TX_SHARED_DP_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_AP_FLOW_CONTROL_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_EL_DB_OPERATION_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_PFC_XON_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_TX_SHAPER_GRANULARITY_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PORT_UBURST_TRIGGER_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_CPUPORT_TYPE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_ACTIVE_LANES_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_DETAILED_PORT_MAP_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PFC_HYSTERESIS_CONF_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PFC_THRESHOLD_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PIZZA_PROFILE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PORT_ALIGN90_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PORT_AP_INTROP_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PORT_AP_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PORT_AP_STATS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PORT_AP_STATUS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PORT_AUTONEG_ADVERTISMENT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PORT_CN_TRIGGER_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PORT_COMBO_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PORT_ECN_ENABLERS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PORT_EEE_LPI_STATUS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PORT_EOM_MATRIX_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PORT_MAC_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PORT_MAP_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PORT_PFC_LOSSY_DROP_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PORT_PFC_PROFILE_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PORT_PIP_MAC_DA_CLASSIFICATION_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PORT_RXDMA_DROP_COUNTER_MODE_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PORT_SERDES_AC3X1690TX_PARAM_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PORT_SERDES_EYE_INPUT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PORT_SERDES_EYE_RESULT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PORT_SERDES_OPERATION_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PORT_STATUS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PORT_STAT_TX_DROP_COUNTERS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PORT_TX_SHAPER_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_RSFEC_STATUS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_UBURST_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_UBURST_PROFILE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PORT_CNM_GENERATION_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PORT_CN_FB_CALCULATION_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PORT_CN_PROFILE_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PORT_QUEUE_ECN_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PORT_TX_MC_FIFO_ARBITER_WEIGHTS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PORT_TX_TAIL_DROP_WRTD_ENABLERS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PORT_TX_WRTD_MASK_LSB_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_BOBCAT2_DEV_PIZZA_ARBITER_STATE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_BOBK_DEV_PIZZA_ARBITER_STATE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_EXT_SHADOW_PORT_MAP_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_LION2_DEV_PIZZA_ARBITER_STATE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PORT_FEC_MODE_ENT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PORT_PIZZA_SLICENUM_TO_SLICE_LIST_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PORT_PRIORITY_MATR_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PORT_SPEED_TO_SLICENUM_LIST_ITEM_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_SHADOW_PORT_MAP_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_TXQDEF_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_UBURST_THRESHOLD_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_UBURST_TIME_STAMP_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PORT_QUEUE_DP_ECN_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_BOBCAT2_SLICES_PIZZA_ARBITER_STATE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_CPU_PORTDEF_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PA_UNIT_ENT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PIZZA_PORT_ID_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PORT_SPEED_TO_SLICE_NUM_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_TXQ_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_UNIT32_SLICES_PIZZA_ARBITER_STATE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_UNIT_TXQ_SLICES_PIZZA_ARBITER_STATE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_SliceNumXPortPizzaCfgSlicesList_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PIZZA_SLICE_ID_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PORT_PIZZA_CFG_PORT_SLICE_LIST_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_ACTIVE_LANES_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_DETAILED_PORT_MAP_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_DIAG_PIZZA_ARBITER_UNIT_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_MIN_SPEED_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_MIN_SPEED_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PA_UNIT_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PFC_HYSTERESIS_CONF_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PFC_THRESHOLD_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PIZZA_PROFILE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PIZZA_PROFILE_STC_PTR_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_ALIGN90_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_AP_INTROP_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_AP_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_AP_STATS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_AP_STATUS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_AUTONEG_ADVERTISMENT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_BUFFERS_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_BUFFERS_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_CN_CONGESTED_Q_PRIORITY_LOCATION_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_CN_CONGESTED_Q_PRIORITY_LOCATION_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_CN_MESSAGE_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_CN_MESSAGE_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_CN_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_CN_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_CN_PACKET_LENGTH_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_CN_PACKET_LENGTH_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_CN_PRIORITY_SPEED_LOCATION_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_CN_PRIORITY_SPEED_LOCATION_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_CN_TRIGGER_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_COMBO_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_COM_PHY_H_SUB_SEQ_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_CPLL_INPUT_FREQUENCY_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_CPLL_OUTPUT_FREQUENCY_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_CTLE_BIAS_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_CTLE_BIAS_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_ECN_ENABLERS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_EEE_LPI_STATUS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_EOM_MATRIX_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_FC_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_FC_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_FEC_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_FEC_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_GROUP_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_HOL_FC_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_HOL_FC_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_MAC_BR_COUNTER_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_MAC_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_MAPPING_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_MEMORY_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_SCOPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_VALUE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_PCS_LOOPBACK_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_PCS_LOOPBACK_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_PERIODIC_FC_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_PERIODIC_FC_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_PFC_COUNT_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_PFC_COUNT_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_PFC_ENABLE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_PFC_ENABLE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_PFC_LOSSY_DROP_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_PFC_PROFILE_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_PFC_RESPONCE_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_PFC_RESPONCE_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPES_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPES_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_PIP_MAC_DA_CLASSIFICATION_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_PIP_PROTOCOL_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC_PTR_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_RXDMA_DROP_COUNTER_MODE_INFO_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_SERDES_AC3X1690TX_PARAM_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_SERDES_EYE_INPUT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_SERDES_EYE_RESULT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_SERDES_LOOPBACK_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_SERDES_LOOPBACK_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_SERDES_OPERATION_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_SERDES_SPEED_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_STATUS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_STAT_TX_DROP_COUNTERS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_TXQ_OFFSET_WIDTH_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_TXQ_OFFSET_WIDTH_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_TX_BC_CHANGE_ENABLE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_TX_BC_CHANGE_ENABLE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_TX_SHAPER_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_TX_SHARED_POLICY_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_TX_SHARED_POLICY_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_TX_TOKEN_BUCKET_MTU_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_TX_TOKEN_BUCKET_MTU_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_UNITS_ID_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PORT_UNITS_ID_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_QUEUE_OFFSET_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_QUEUE_OFFSET_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_RSFEC_STATUS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_UBURST_INFO_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_UBURST_PROFILE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PORT_CNM_GENERATION_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PORT_CN_FB_CALCULATION_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PORT_CN_PROFILE_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PORT_QUEUE_ECN_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PORT_TX_MC_FIFO_ARBITER_WEIGHTS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PORT_TX_PFC_AVAILABLE_BUFF_CONF_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PORT_TX_PFC_RESOURCE_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PORT_TX_PFC_RESOURCE_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PORT_TX_SHARED_DP_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PORT_TX_SHARED_DP_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PORT_TX_TAIL_DROP_WRTD_ENABLERS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PORT_TX_WRTD_MASK_LSB_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC_PTR_pizzaDeviceStatePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_CPSS_DXCH_PORT_ALIGN90_PARAMS_STC_PTR_serdesParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_CPSS_PM_PORT_PARAMS_STC_PTR_portParamsStcPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_uBurstEventsSizePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_ADJUST_OPERATION_ENT_bcOp;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_BRG_TPID_SIZE_TYPE_ENT_tpidSize;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DP_LEVEL_ENT_maxDp;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_DIAG_PIZZA_ARBITER_UNIT_ENT_unitId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_MIN_SPEED_ENT_minimalPortSpeedMBps;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_NET_RX_PARAMS_STC_PTR_rxParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PA_UNIT_ENT_unit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PFC_HYSTERESIS_CONF_STC_PTR_hysteresisCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PFC_THRESHOLD_STC_PTR_thresholdCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PIZZA_PROFILE_STC_PTR_pizzaProfilePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_AP_INTROP_STC_PTR_apIntropPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_AP_PARAMS_STC_PTR_apParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_AUTONEG_ADVERTISMENT_STC_PTR_portAnAdvertismentPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_BUFFERS_MODE_ENT_bufferMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_BUF_MEM_FIFO_TYPE_ENT_fifoType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_CN_CONGESTED_Q_PRIORITY_LOCATION_ENT_congestedQPriorityLocation;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_CN_MESSAGE_TYPE_ENT_mType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_CN_MODE_ENT_enable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_CN_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_CN_PACKET_LENGTH_ENT_packetLength;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_CN_PRIORITY_SPEED_LOCATION_ENT_prioritySpeedLocation;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_CN_TRIGGER_CONFIG_STC_PTR_qcnTriggerPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_COMBO_PARAMS_STC_PTR_paramsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_COM_PHY_H_SUB_SEQ_ENT_seqType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_CPLL_INPUT_FREQUENCY_ENT_inputFreq;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_CPLL_OUTPUT_FREQUENCY_ENT_outputFreq;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_CTLE_BIAS_MODE_ENT_PTR_ctleBiasValuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_CTLE_BIAS_MODE_ENT_ctleBiasValue;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_ECN_ENABLERS_STC_PTR_enablersPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_FC_MODE_ENT_fcMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_FEC_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_GROUP_ENT_portGroup;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_MAC_BR_COUNTER_TYPE_ENT_macCountMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT_counterMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_MAC_PARAMS_STC_PTR_macPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_MAPPING_TYPE_ENT_origPortType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_MAP_STC_PTR_portMapArrayPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_MEMORY_ENT_counterMemory;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_SCOPE_ENT_counterScope;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_PACKET_BUFFER_COUNTER_VALUE_ENT_counterValue;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_PCS_LOOPBACK_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_PERIODIC_FC_TYPE_ENT_PTR_portTypePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_PERIODIC_FC_TYPE_ENT_portType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_PFC_COUNT_MODE_ENT_pfcCountMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_PFC_ENABLE_ENT_pfcEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_PFC_LOSSY_DROP_CONFIG_STC_PTR_lossyDropConfigPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_PFC_PROFILE_CONFIG_STC_PTR_pfcProfileCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_PFC_RESPONCE_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_PFC_RESPONCE_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ENT_type;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPES_ENT_vectorType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPES_ENT_thresholdType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_PIP_MAC_DA_CLASSIFICATION_STC_PTR_entryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT_type;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_PIP_PROTOCOL_ENT_protocol;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC_PTR_portGroupPortSpeed2SliceNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_RXDMA_DROP_COUNTER_MODE_INFO_STC_PTR_counterModeInfoPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_SERDES_AC3X1690TX_PARAM_CONFIG_STC_PTR_serdesRxauiTxCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_SERDES_AUTO_TUNE_MODE_ENT_portTuningMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_SERDES_EYE_INPUT_STC_PTR_eye_inputPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_SERDES_LOOPBACK_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_SERDES_OPERATION_CFG_STC_PTR_seqLinePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_SERDES_SPEED_ENT_serdesFrequency;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT_value;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT_recoveryClkType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT_clockSelect;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_TXQ_OFFSET_WIDTH_ENT_queueOffsetWidth;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_TX_BC_CHANGE_ENABLE_ENT_bcChangeEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_TX_BC_CHANGE_ENABLE_ENT_bcMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_TX_SHAPER_CONFIG_STC_PTR_configsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_TX_SHARED_POLICY_ENT_policy;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_TX_TOKEN_BUCKET_MTU_ENT_mtu;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_UNITS_ID_ENT_unitId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_QUEUE_OFFSET_MODE_ENT_offsetMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_UBURST_PROFILE_STC_PTR_profilePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_NET_RX_CPU_CODE_ENT_tailDropCode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PM_PORT_PARAMS_STC_PTR_portParamsStcPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORTS_BMP_STC_PTR_portsBmpPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_AP_SERDES_RX_CONFIG_STC_PTR_rxOverrideParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_AP_SERDES_TX_OFFSETS_STC_PTR_serdesTxOffsetsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_CNM_GENERATION_CONFIG_STC_PTR_cnmGenerationCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_CN_FB_CALCULATION_CONFIG_STC_PTR_fbCalcCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_CN_PROFILE_CONFIG_STC_PTR_cnProfileCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC_PTR_entryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_DUPLEX_ENT_dMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_EGRESS_CNT_MODE_ENT_setModeBmp;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_FLOW_CONTROL_ENT_state;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_INTERFACE_MODE_ENT_ifMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_INTERLAKEN_COUNTER_ID_ENT_counterId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_MAC_COUNTERS_ENT_cntrName;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_MAC_MTI_BR_STAT_COUNTER_ENT_cntType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_MAC_TO_SERDES_STC_PTR_macToSerdesMuxStcPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_MANAGER_GLOBAL_PARAMS_STC_PTR_globalParamsStcPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_MANAGER_PORT_TYPE_ENT_portType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_MANAGER_STC_PTR_portEventStcPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_MANAGER_UPDATE_PARAMS_STC_PTR_updateParamsStcPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_MODE_SPEED_STC_PTR_portModeSpeedOptionsArrayPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_PA_BW_MODE_ENT_portMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_PCS_RESET_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT_enable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_QUEUE_ECN_PARAMS_STC_PTR_tailDropEcnProfileParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_REF_CLOCK_SOURCE_ENT_refClockSource;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_RX_FC_PROFILE_SET_ENT_profileSet;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_SERDES_RX_CONFIG_STC_PTR_serdesRxCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_SERDES_TUNE_STC_PTR_tuneValuesPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_SERDES_TX_CONFIG_STC_PTR_serdesTxCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_TAIL_DROP_WRED_PARAMS_STC_PTR_tailDropWredProfileParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_TX_DROP_PROFILE_SET_ENT_pfSet;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_TX_DROP_PROFILE_SET_ENT_profile;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_TX_DROP_SHAPER_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_TX_MC_FIFO_ARBITER_WEIGHTS_STC_PTR_weigthsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_TX_PFC_AVAILABLE_BUFF_CONF_MODE_ENT_confMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_TX_PFC_RESOURCE_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_TX_Q_ARB_GROUP_ENT_arbGroup;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS_STC_PTR_tailDropProfileParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_TX_Q_TAIL_DROP_WRED_TC_DP_PROF_PARAMS_STC_PTR_tailDropWredProfileParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT_profile;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT_profileSet;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_TX_SHARED_DP_MODE_ENT_enableMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT_alpha;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT_portAlpha;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_ENT_resourceMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_TX_TAIL_DROP_WRTD_ENABLERS_STC_PTR_enablersPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_TX_WRR_MODE_ENT_wrrMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_TX_WRR_MTU_ENT_wrrMtu;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_TX_WRTD_MASK_LSB_STC_PTR_maskLsbPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_XGMII_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_XG_FIXED_IPG_ENT_ipgBase;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_32_bcValue;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_32_ppmValue;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_PTR_overrideEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_apEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_avbModeEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_contextDone;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_cutThroughPacket;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_cutThroughTerminated;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_descriptorValid;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_dummyDescriptor;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_enPortStat;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_enQueueStat;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_firstLine;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_fromCpuEn;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_headOrTailDispatch;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_highPriorityPacket;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_invertRx;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_invertTx;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_isEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_isLinkUp;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_latencySensitive;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_linkUp;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_multiTargetEn;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_multicastPacket;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_noeye;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_pauseAdvertise;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_poolNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_powerUp;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_send;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_singleTargetEn;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_srvCpuEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_startStopDba;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_trunkatedHeader;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_vosOverride;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_DATA_PATH_BMP_dataPathBmp;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_HW_DEV_NUM_targetHwDev;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_PHYSICAL_PORT_NUM_firstPhysicalPortNumber;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_PHYSICAL_PORT_NUM_localPortNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_PHYSICAL_PORT_NUM_loopbackPortNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_PHYSICAL_PORT_NUM_physicalPortNumber;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_PHYSICAL_PORT_NUM_unitRepresentingPort;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_PORT_NUM_targetPort;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_SW_DEV_NUM_devNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U16_PTR_serdesList;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U16_burstSize;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_PTR_rxSerdesLaneArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_PTR_serdesOptAlgBmpPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_PTR_txSerdesLaneArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_almostFullThreshold;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_availableBuff;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_availableBuffers;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_channelSpeedResMbps;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_cntrIdx;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_cntrNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_cpllNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_criteriaParamBmp;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_descFifoThreshold;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_dp;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_dropThreshold;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_fieldIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_fullThreshold;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_gigPortsSlowRateRatio;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_gigPortsTokensRate;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_globalAvailableBuffers;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_globalPbLimit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_interruptCode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_interruptData;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ipg;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_laneBmp;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_lanesBmp;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_length;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_liLimit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_lineNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_localPortNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_maxBufNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_maxBuff;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_maxBuffFillLvl;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_maxDescNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_maxSharedBufferLimit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_max_dwell_bits;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_mcFifo;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_mcastAvailableBuffersNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_mcastGuaranteedLimit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_mcastMaxBufNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_mcastMaxDescNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_minXFifoReadThreshold;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_min_dwell_bits;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numLsb;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numOfOp;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_origPortNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_pauseTimer;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_pfcCounterNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_pfcTimer;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_phase;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_pipProfile;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_pipeBandwithInGbps;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_pool0AvailableBuffers;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_pool1AvailableBuffers;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_poolAvailableBuff;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_poolNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_portGroup;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_portGroupBmp;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_portGroupId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_portGroupIdx;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_portGroupNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_portMapArraySize;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_portMaxBuffLimit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_portMaxDescrLimit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_portSpeedIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_priorityGroup;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_qcnSampleInterval;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_qlenOld;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_queue;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_queueOffset;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_queueOffset4bit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_queueOffset8bit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_rxBufLimit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_rxSniffMaxBufNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_rxSniffMaxDescNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_rxToCpPriority;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_samplingTime;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_serdesOptAlgBmp;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_shaperToPortRateRatio;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_sharedBufLimit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_sharedDescLimit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_sliceID;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_sliceNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_sliceNumToConfig;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_speed;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_squelch;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_srcProfile;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_tailDropDumpBmp;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_tcForPfcResponse;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_tcQueue;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_tcVector;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_tileIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_timer;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_tmBandwidthMbps;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_trgProfile;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_tsLimit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_twLimit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_txFifoThreshold;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_txSniffMaxBufNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_txSniffMaxDescNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_vidxOffset;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_wrrWeight;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_xgPortsTokensRate;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_xoffLimit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_xoffThreshold;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_xonLimit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_xonThreshold;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_PTR_currSerdesDelayArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_PTR_enhTrainDelayArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_PTR_trainLfArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_cntrSetNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_macSaLsb;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_max_LF;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_max_dly;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_minTc;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_min_LF;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_min_dly;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_numOfPorts;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_numOfSerdeses;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_number;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_optionsArrayLen;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_poolId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_serdesLane;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_trafClass;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_trafficClass;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_wrrWeight;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_MV_HWS_AP_DEBUG_LOG_OUTPUT_ENT_output;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_MV_HWS_PORT_STANDARD_portMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_MV_HWS_SERDES_PRESET_OVERRIDE_UNT_PTR_serdesPresetOverride;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_ADJUST_OPERATION_ENT_PTR_bcOpPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_ANP_INFO_STC_PTR_anpInfoPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_BRG_TPID_SIZE_TYPE_ENT_PTR_tpidSizePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DP_LEVEL_ENT_PTR_maxDpPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_ACTIVE_LANES_STC_PTR_activeLanePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_DETAILED_PORT_MAP_STC_PTR_portMapShadowPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC_PTR_pizzaDeviceStatePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_MIN_SPEED_ENT_PTR_minimalPortSpeedResolutionInMBpsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PFC_HYSTERESIS_CONF_STC_PTR_hysteresisCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PFC_THRESHOLD_STC_PTR_thresholdCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PIZZA_PROFILE_STC_PTR_PTR_pizzaProfilePtrPtrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_AP_INTROP_STC_PTR_apIntropPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_AP_PARAMS_STC_PTR_apParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_AP_STATS_STC_PTR_apStatsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_AP_STATUS_STC_PTR_apStatusPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_AUTONEG_ADVERTISMENT_STC_PTR_portAnAdvertismentPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_BUFFERS_MODE_ENT_PTR_bufferModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_CN_CONGESTED_Q_PRIORITY_LOCATION_ENT_PTR_congestedQPriorityLocationPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_CN_MESSAGE_TYPE_ENT_PTR_mTypePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_CN_MODE_ENT_PTR_enablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_CN_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_CN_PACKET_LENGTH_ENT_PTR_packetLengthPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_CN_PRIORITY_SPEED_LOCATION_ENT_PTR_prioritySpeedLocationPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_CN_TRIGGER_CONFIG_STC_PTR_qcnTriggerPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_COMBO_PARAMS_STC_PTR_paramsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_ECN_ENABLERS_STC_PTR_enablersPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_EEE_LPI_MANUAL_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_EEE_LPI_STATUS_STC_PTR_statusPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_EOM_MATRIX_STC_PTR_matrixPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_FC_MODE_ENT_PTR_fcModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_FEC_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT_PTR_counterModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_MAC_PARAMS_STC_PTR_macPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_MAP_STC_PTR_portMapArray;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_PCS_LOOPBACK_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_PFC_COUNT_MODE_ENT_PTR_pfcCountModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_PFC_ENABLE_ENT_PTR_pfcEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_PFC_LOSSY_DROP_CONFIG_STC_PTR_lossyDropConfigPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_PFC_PROFILE_CONFIG_STC_PTR_pfcProfileCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_PIP_MAC_DA_CLASSIFICATION_STC_PTR_entryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC_PTR_PTR_portGroupPortSpeed2SliceNumPtrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_RXDMA_DROP_COUNTER_MODE_INFO_STC_PTR_counterModeInfoPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT_PTR_rxTuneStatusPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT_PTR_statusPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_SERDES_AUTO_TUNE_STATUS_ENT_PTR_txTuneStatusPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_SERDES_EYE_RESULT_STC_PTR_eye_resultsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_SERDES_LOOPBACK_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_SERDES_OPERATION_CFG_STC_PTR_seqLinePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_STATUS_STC_PTR_portStatusPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_STAT_TX_DROP_COUNTERS_STC_PTR_dropCntrStcPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT_PTR_valuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_TXQ_OFFSET_WIDTH_ENT_PTR_queueOffsetWidthPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_TX_BC_CHANGE_ENABLE_ENT_PTR_bcChangeEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_TX_BC_CHANGE_ENABLE_ENT_PTR_bcModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_TX_SHAPER_CONFIG_STC_PTR_configsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_TX_SHARED_POLICY_ENT_PTR_policyPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_TX_TOKEN_BUCKET_MTU_ENT_PTR_mtuPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_UNITS_ID_ENT_PTR_unitIdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_QUEUE_OFFSET_MODE_ENT_PTR_offsetModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_RSFEC_STATUS_STC_PTR_rsfecStatusPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_UBURST_INFO_STC_PTR_uBurstDataPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_UBURST_PROFILE_STC_PTR_profilePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_FCFEC_COUNTERS_STC_PTR_fcfecCountersPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_NET_RX_CPU_CODE_ENT_PTR_tailDropCodePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PM_PORT_PARAMS_STC_PTR_portParamsStcPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_ATTRIBUTES_STC_PTR_portAttributSetArrayPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_CNM_GENERATION_CONFIG_STC_PTR_cnmGenerationCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_CN_FB_CALCULATION_CONFIG_STC_PTR_fbCalcCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_CN_PROFILE_CONFIG_STC_PTR_cnProfileCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC_PTR_entryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_DUPLEX_ENT_PTR_dModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_EGRESS_CNTR_STC_PTR_egrCntrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_EGRESS_CNT_MODE_ENT_PTR_setModeBmpPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_FLOW_CONTROL_ENT_PTR_statePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_INTERFACE_MODE_ENT_PTR_ifModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_MAC_CG_COUNTER_SET_STC_PTR_cgMibStcPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_MAC_COUNTER_SET_STC_PTR_portMacCounterSetArrayPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_MAC_MTI_COUNTER_SET_STC_PTR_mtiMibStcPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_MAC_STATUS_STC_PTR_portMacStatusPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_MAC_TO_SERDES_STC_PTR_macToSerdesMuxStcPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_MAC_TYPE_ENT_PTR_portMacTypePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_MANAGER_STATISTICS_STC_PTR_portStatStcPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_MANAGER_STATUS_STC_PTR_portStagePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_MODE_SPEED_STC_PTR_currentModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_PA_BW_MODE_ENT_PTR_portModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT_PTR_enablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_QUEUE_ECN_PARAMS_STC_PTR_tailDropEcnProfileParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_REF_CLOCK_SOURCE_ENT_PTR_refClockSourcePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_RX_FC_PROFILE_SET_ENT_PTR_profileSetPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_SERDES_RX_CONFIG_STC_PTR_serdesRxCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_SERDES_TUNE_STC_PTR_serdesTunePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_SERDES_TUNE_STC_PTR_tuneValuesPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_SERDES_TX_CONFIG_STC_PTR_serdesTxCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_TAIL_DROP_WRED_PARAMS_STC_PTR_tailDropWredProfileParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_TX_DROP_SHAPER_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_TX_MC_FIFO_ARBITER_WEIGHTS_STC_PTR_weigthsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_TX_PFC_RESOURCE_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_TX_Q_ARB_GROUP_ENT_PTR_arbGroupPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS_STC_PTR_tailDropProfileParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_TX_Q_TAIL_DROP_WRED_TC_DP_PROF_PARAMS_STC_PTR_tailDropWredProfileParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT_PTR_profileSetPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_TX_SHARED_DP_MODE_ENT_PTR_enableModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT_PTR_alphaPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT_PTR_portAlphaPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_ENT_PTR_resourceModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_TX_TAIL_DROP_WRTD_ENABLERS_STC_PTR_enablersPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_TX_WRR_MODE_ENT_PTR_wrrModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_TX_WRR_MTU_ENT_PTR_wrrMtuPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_TX_WRTD_MASK_LSB_STC_PTR_maskLsbPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_XGMII_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_XG_FIXED_IPG_ENT_PTR_ipgBasePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_RSFEC_COUNTERS_STC_PTR_rsfecCountersPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_32_PTR_bcValuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_32_PTR_ppmValuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_32_PTR_retVal;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_apEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_avbModeEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_captureIsDonePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_cdrLockPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_contextDonePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_cutThroughPacketPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_cutThroughTerminatedPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_descriptorValidPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_dummyDescriptorPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_enPortStatPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_enQueueStatPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_fromCpuEnPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_gbLockPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_headOrTailDispatchPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_highPriorityPacketPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_invertRx;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_invertRxPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_invertTx;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_invertTxPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_isCpuPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_isEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_isLinkUpPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_isLocalFaultPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_isOccupiedPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_isRemoteFaultPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_latencySensitivePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_lockPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_multiTargetEnPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_multicastPacketPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_pauseAdvertisePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_signalStatePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_singleTargetEnPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_srvCpuEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_supportedPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_syncPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_trunkatedHeaderPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_vosOverridePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_HW_DEV_NUM_PTR_targetHwDevPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_PHYSICAL_PORT_NUM_PTR_loopbackPortNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_PHYSICAL_PORT_NUM_PTR_physicalPortNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_PORT_NUM_PTR_targetPortPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U16_PTR_burstSizePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U16_PTR_cpuRxBufCntPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U16_PTR_currSerdesDelayArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U16_PTR_dro;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U16_PTR_enhTrainDelayArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U16_PTR_intropAbilityMaxIntervalPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U16_PTR_numberPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U16_PTR_serdesList;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U16_PTR_trainLfArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_actualSpeedPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_agedBuffersPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_almostFullThresholdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_apResolvedPortsBmpPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_availableBuffPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_availableBuffersPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_baseAddrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_baudRatePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_cntPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_criteriaParamBmpPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_ctrValPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_currentValPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_descFifoThresholdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_dfeResPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_dropThresholdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_droppedCntPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_fullThresholdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_gigPortsSlowRateRatioPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_gigPortsTokensRatePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_globalAvailableBuffersPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_globalPbLimitPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_highSpeedPortIdxArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_highSpeedPortNumberPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_ipgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_laneNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_liLimitPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_localPortNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_loopbackPortNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_maxBufNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_maxBuffFillLvlPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_maxBuffPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_maxBufferLimitPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_maxDescNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_maxFreeBuffersPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_maxIlknIFSpeedMbpsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_maxPeakValPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_maxSharedBufferLimitPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_maxThresholdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_mcCntrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_mcFifoPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_mcastAvailableBuffersNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_mcastGuaranteedLimitPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_mcastMaxBufNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_mcastMaxDescNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_minFreeBuffersPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_minPeakValPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_minThresholdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_minXFifoReadThresholdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_numLsbPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_numOfBuffersPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_numOfConfiguredSlicesPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_numOfPacketsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_numPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_numberPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_pauseTimerPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_pfcCounterNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_pfcCounterValuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_pipProfilePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_pipeBandwithInGbpsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_pool0AvailableBuffersPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_pool1AvailableBuffersPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_poolAvailableBuffPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_poolNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_portMaxBuffLimitPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_portMaxDescrLimitPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_portNumArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_portSerdesGroupPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_portSpeedIndexPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_priorityGroupPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_qcnSampleIntervalPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_qlenOldPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_queueOffset4bitPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_receivedCntPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_receivedValuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_rowSizePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_rxBufLimitPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_rxNumberPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_rxSerdesLaneArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_rxSniffMaxBufNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_rxSniffMaxDescNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_rxToCpPriorityPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_shaperToPortRateRatioPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_sharedBufLimitPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_sharedDescLimitPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_sliceNumUsedPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_speedPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_speedResulutionMBpsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tcForPfcResponsePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tcQueuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tcVectorPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tmBandwidthMbpsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tsLimitPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_twLimitPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_txFifoThresholdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_txNumberPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_txSerdesLaneArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_txSniffMaxBufNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_txSniffMaxDescNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_txqDqPortNumArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_txqLLPortNumArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_unitIndexPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_vidxOffsetPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_wrrWeightPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_xgPortsTokensRatePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_xoffLimitPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_xoffThresholdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_xonLimitPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_xonThresholdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U64_PTR_cntrValuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U64_PTR_countersArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_macSaLsbPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_minTcPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_numOfSerdeses;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_numberPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_tcPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_tcQueuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_wrrWeightPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_MV_HWS_SERDES_ENCODING_TYPE_PTR_rxEncodingPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_MV_HWS_SERDES_ENCODING_TYPE_PTR_txEncodingPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_MV_HWS_SERDES_PRESET_OVERRIDE_UNT_PTR_serdesPresetOverridePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_MV_HWS_SERDES_RX_DATAPATH_CONFIG_STC_PTR_rxDatapathConfigPtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssDxChPortApEnableSet_E = (CPSS_LOG_LIB_PORT_E << 16) | (1 << 24),
    PRV_CPSS_LOG_FUNC_cpssDxChPortApEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortApPortConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortApPortConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortApPortEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortApPortStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortApResolvedPortsBmpGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortApSetActiveMode_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortApLock_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortApUnLock_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortApStatsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortApStatsReset_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortApIntropSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortApIntropGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortApDebugInfoGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortApPortEnableCtrlSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortApPortEnableCtrlGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortApSerdesTxParametersOffsetSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortApSerdesRxParametersManualSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortAdaptiveCtlePortEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortIsLinkUpStatusSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortAutoNegAdvertismentConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortAutoNegAdvertismentConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortAutoNegMasterModeEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortAutoNegMasterModeEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortModeSpeedAutoDetectAndConfig_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortInbandAutoNegRestart_E,
    PRV_CPSS_LOG_FUNC_cpssDxChGlobalXonLimitSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChGlobalXonLimitGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChGlobalXoffLimitSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChGlobalXoffLimitGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortRxFcProfileSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortRxFcProfileGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortXonLimitSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortXonLimitGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortXoffLimitSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortXoffLimitGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortRxBufLimitSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortRxBufLimitGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCpuRxBufCountGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortFcHolSysModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortFcHolSysModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortBuffersModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortBuffersModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortGroupXonLimitSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortGroupXonLimitGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortGroupXoffLimitSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortGroupXoffLimitGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortGroupRxBufLimitSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortGroupRxBufLimitGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCrossChipFcPacketRelayEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCrossChipFcPacketRelayEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortGlobalRxBufNumberGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortGroupRxBufNumberGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortRxBufNumberGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortGlobalPacketNumberGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortRxNumOfAgedBuffersGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortRxMcCntrGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortBufMemFifosThresholdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortBufMemFifosThresholdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxdmaBurstLimitEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxdmaBurstLimitEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxdmaBurstLimitThresholdsSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxdmaBurstLimitThresholdsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortRxMaxBufLimitGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnModeEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnModeEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnProfileSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnProfileGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnProfileQueueThresholdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnProfileQueueThresholdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnProfileThresholdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnProfileThresholdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnQueueAwareEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnQueueAwareEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnFrameQueueSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnFrameQueueGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnEtherTypeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnEtherTypeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnSpeedIndexSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnSpeedIndexGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnFcEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnFcEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnPauseTriggerEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnPauseTriggerEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnFcTimerSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnFcTimerGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnPanicPauseThresholdsSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnPanicPauseThresholdsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnTerminationEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnTerminationEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnProfileQueueConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnProfileQueueConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnQueueStatusModeEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnQueueStatusModeEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnSampleEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnSampleEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnFbCalcConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnFbCalcConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnPacketLengthSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnPacketLengthGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnMessageGenerationConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnMessageGenerationConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnPrioritySpeedLocationSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnPrioritySpeedLocationGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnMessageTypeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnMessageTypeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnMessageTriggeringStateSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnMessageTriggeringStateGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnPauseTimerMapTableEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnPauseTimerMapTableEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnDbaAvailableBuffSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnDbaAvailableBuffGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnDbaModeEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnDbaModeEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnDbaPoolAvailableBuffSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnDbaPoolAvailableBuffGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnQcnTriggerSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnQcnTriggerGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnCNMEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnCNMEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnProfilePortConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnProfilePortConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnCongestedQPriorityLocationSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnCongestedQPriorityLocationGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCnModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortComboModeEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortComboModeEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortComboPortActiveMacSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortComboPortActiveMacGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMacSaLsbSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMacSaLsbGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortDuplexModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortDuplexModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortDuplexAutoNegEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortDuplexAutoNegEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortFlowCntrlAutoNegEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortFlowCntrlAutoNegEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSpeedAutoNegEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSpeedAutoNegEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortFlowControlEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortFlowControlEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPeriodicFcEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPeriodicFcEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortBackPressureEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortBackPressureEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortLinkStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortForceLinkPassEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortForceLinkPassEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortForceLinkDownEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortForceLinkDownEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMruSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMruGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMruProfileSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMruProfileGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortProfileMruSizeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortProfileMruSizeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMruExceptionCommandSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMruExceptionCommandGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMruExceptionCpuCodeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMruExceptionCpuCodeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCrcCheckEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCrcCheckEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortXGmiiModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortXGmiiModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortIpgBaseSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortIpgBaseGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortIpgSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortIpgGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortExtraIpgSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortExtraIpgGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortXgmiiLocalFaultGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortRemoteFaultConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortXgmiiRemoteFaultGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortRemoteFaultSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMacStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortInternalLoopbackEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortInternalLoopbackEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortInbandAutoNegEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortInbandAutoNegEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortAttributesOnPortGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPreambleLengthSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPreambleLengthGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMacSaBaseSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMacSaBaseGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPaddingEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPaddingEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortExcessiveCollisionDropEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortExcessiveCollisionDropEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortInBandAutoNegBypassEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortInBandAutoNegBypassEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesGroupGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortFlowControlModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortFlowControlModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMacResetStateSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortForward802_3xEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortForward802_3xEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortForwardUnknownMacControlFramesEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortForwardUnknownMacControlFramesEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCpllCfgInit_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPeriodicFlowControlIntervalSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPeriodicFlowControlIntervalGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPeriodicFlowControlIntervalSelectionSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPeriodicFlowControlIntervalSelectionGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMacTypeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPeriodicFlowControlCounterSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPeriodicFlowControlCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortUnitInfoGetByAddr_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortUnitInfoGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCtleBiasOverrideEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCtleBiasOverrideEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortVosOverrideControlModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortVosOverrideControlModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortXlgReduceAverageIPGSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortXlgReduceAverageIPGGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCascadePfcParametersSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortCascadePfcParametersGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortRemoteFcParametersSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortRemoteFcParametersGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortLaneMacToSerdesMuxSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortLaneMacToSerdesMuxGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortDebugLogEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortEnableWaWithLinkStatusSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMacPcsStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChRsFecStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChRsFecCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChFcFecCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesActiveLanesListGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortFastLinkDownEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortFastLinkDownEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortAnpInfoGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortDiagVoltageGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortDiagTemperatureGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortDpDebugCountersResetAndEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortDpDebugRxToCpCountingConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortDpDebugRxToCpCountingConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortDpDebugRxToCpCountValueGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortDpDebugCpToRxCountingConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortDpDebugCpToRxCountingConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortDpDebugCpToRxCountValueGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortEcnMarkingEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortEcnMarkingEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortEcnMarkingTailDropProfileEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortEcnMarkingTailDropProfileEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTx4TcTailDropEcnMarkingProfileSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTx4TcTailDropEcnMarkingProfileGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxTailDropPoolEcnMarkingEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxTailDropPoolEcnMarkingEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxTailDropMcEcnMarkingEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxTailDropMcEcnMarkingEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxTailDropEcnMarkingOnCongestionEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxTailDropEcnMarkingOnCongestionEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortEeeLpiRequestEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortEeeLpiRequestEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortEeeLpiManualModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortEeeLpiManualModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortEeeLpiTimeLimitsSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortEeeLpiTimeLimitsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortEeeLpiStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortInterfaceModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortInterfaceModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortModeSpeedSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortExtendedModeEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortExtendedModeEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortInterfaceSpeedSupportGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortFecModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortFecModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortRefClockSourceOverrideEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortRefClockSourceOverrideEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortResourceTmBandwidthSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortResourceTmBandwidthGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortInterlakenCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortIlknChannelSpeedResolutionSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortIlknChannelSpeedResolutionGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortIlknChannelSpeedSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortIlknChannelSpeedGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortIlknChannelEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortIlknChannelEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortLoopbackEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortLoopbackEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortLoopbackPktTypeForwardAndFromCpuEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortLoopbackPktTypeForwardAndFromCpuSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortLoopbackPktTypeForwardAndFromCpuGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortLoopbackPktTypeToCpuSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortLoopbackPktTypeToCpuGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortLoopbackPktTypeToAnalyzerSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortLoopbackPktTypeToAnalyzerGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortLoopbackProfileSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortLoopbackProfileGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortLoopbackEvidxMappingSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortLoopbackEvidxMappingGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortLoopbackEnableEgressMirroringSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortLoopbackEnableEgressMirroringGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortManagerEventSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortManagerStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortManagerPortParamsSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortManagerPortParamsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortManagerPortParamsStructInit_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortManagerGlobalParamsOverride_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortManagerPortParamsUpdate_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortManagerInit_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortManagerEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortManagerStatGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortManagerStatClear_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPhysicalPortMapSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPhysicalPortMapGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPhysicalPortMapIsValidGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPhysicalPortMapIsCpuGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPhysicalPortMapReverseMappingGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPhysicalPortDetailedMapGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPacketBufferFillCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPcsLoopbackModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPcsLoopbackModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPcsResetSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPcsGearBoxStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPcsSyncStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPcsSyncStableStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortXgLanesSwapEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortXgLanesSwapEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortXgPscLanesSwapSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortXgPscLanesSwapGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcProfileIndexSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcProfileIndexGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcProfileQueueConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcProfileQueueConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcCountingModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcCountingModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcGlobalDropEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcGlobalDropEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcGlobalQueueConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcGlobalQueueConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcTimerMapEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcTimerMapEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcTimerToQueueMapSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcTimerToQueueMapGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcShaperToPortRateRatioSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcShaperToPortRateRatioGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcForwardEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcForwardEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcLossyDropQueueEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcLossyDropQueueEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcLossyDropConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcLossyDropConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcTerminateFramesEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcTerminateFramesEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortUnknownMacControlFramesCmdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortUnknownMacControlFramesCmdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortFlowControlPacketsCntGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcQueueCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcSourcePortToPfcCounterSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcSourcePortToPfcCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcXonMessageFilterEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcXonMessageFilterEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcDbaAvailableBuffSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcDbaAvailableBuffGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcDbaModeEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcDbaModeEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcGlobalPbLimitSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcGlobalPbLimitGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcGenerationEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcGenerationEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcGlobalTcThresholdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcGlobalTcThresholdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcPortThresholdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcPortThresholdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcPortTcThresholdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcPortTcThresholdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcResponceEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcResponceEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcTcToQueueMapSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcTcToQueueMapGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcReponceModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcReponceModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcLinkPauseCompatibilityModeEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcLinkPauseCompatibilityModeEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcDbaTcAvailableBuffersSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcDbaTcAvailableBuffersGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcTcResourceModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcTcResourceModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcPortTcHeadroomCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcPortTcHeadroomPeakCountersGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcPortTcHeadroomPeakMonitorSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcHeadroomThresholdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcHeadroomThresholdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcGlobalTcHeadroomCountersGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcPoolHeadroomCountersGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcXonGenerationEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPfcXonGenerationEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPipGlobalEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPipGlobalEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPipGlobalEtherTypeProtocolSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPipGlobalEtherTypeProtocolGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPipTrustEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPipTrustEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPipProfileSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPipProfileGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPipVidClassificationEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPipVidClassificationEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPipPrioritySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPipPriorityGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPipGlobalProfilePrioritySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPipGlobalProfilePriorityGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPipGlobalVidClassificationSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPipGlobalVidClassificationGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPipGlobalMacDaClassificationEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPipGlobalMacDaClassificationEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPipGlobalBurstFifoThresholdsSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPipGlobalBurstFifoThresholdsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPipGlobalDropCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPipDropCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortParserGlobalTpidSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortParserGlobalTpidGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPipGlobalThresholdsRandomizationNumLsbSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPipGlobalThresholdsRandomizationNumLsbGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPipGlobalThresholdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPipGlobalThresholdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPipGlobalPfcTcVectorSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPipGlobalPfcTcVectorGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortRxDmaGlobalDropCounterModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortRxDmaGlobalDropCounterModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortRxDmaGlobalDropCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesPowerStatusSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesTuningSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesTuningGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesLaneTuningSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesLaneTuningGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesResetStateSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesAutoTune_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesAlign90StateSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesAlign90StatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesAutoTuneExt_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesAutoTuneStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesLanePolarityGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesLanePolaritySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesPolaritySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesPolarityGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesLoopbackModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesLoopbackModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortEomDfeResGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortEomMatrixGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesEyeMatrixGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesEyeMatrixPrint_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortEomBaudRateGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesTxEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesTxEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesPpmSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesPpmGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesManualTxConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesManualTxConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesManualRxConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesManualRxConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesRxauiManualTxConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesSquelchSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesSignalDetectGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesStableSignalDetectGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesLaneSignalDetectGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesCDRLockStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesEncodingTypeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesRxDatapathConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesRxPllLockGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesPMDLogGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesDroGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesPresetOverrideSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesPresetOverrideGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesAutoTuneOptAlgSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesAutoTuneOptAlgGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesAutoTuneOptAlgRun_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesAutoTuneResultsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesEnhancedAutoTune_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesEnhancedTuneLite_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesAdaptiveCtleBasedTemperature_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortAdaptiveCtleBasedTemperatureDbGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortAdaptiveCtleBasedTemperatureSendMsg_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesSequenceSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesErrorInject_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesSequenceGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSerdesInternalRegisterAccess_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSpeedSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSpeedGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortStatInit_E,
    PRV_CPSS_LOG_FUNC_cpssDxChMacCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMacCountersOnCgPortGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMacBrCountersOnMtiPortGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMacCountersOnMtiPortGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMacCountersOnPortGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMacCounterCaptureGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMacCountersCaptureOnPortGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMacCountersCaptureTriggerSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMacCountersCaptureTriggerGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortGroupEgressCntrModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortEgressCntrModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortGroupEgressCntrModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortEgressCntrModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortGroupEgressCntrsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortEgressCntrsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMacCountersEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMacCountersClearOnReadSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMacCountersClearOnReadGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMacCountersRxHistogramEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMacCountersTxHistogramEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMacOversizedPacketsCounterModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMacOversizedPacketsCounterModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortStatTxDebugCountersGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMacCountersRxHistogramEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMacCountersTxHistogramEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMacCountersEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortRemotePortsMacCountersByPacketParse_E,
    PRV_CPSS_LOG_FUNC_cpssDxChMacCounterOnPhySideGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMacCountersOnPhySidePortGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortMacPreemptionStatCounterstGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSyncEtherRecoveryClkConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSyncEtherRecoveryClkConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSyncEtherRecoveryClkDividerValueSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSyncEtherRecoveryClkDividerValueGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxInit_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxQueueEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxQueueEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxQueueingEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxQueueingEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxQueueTxEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxQueueTxEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxFlushQueuesSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxShaperEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxShaperEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxShaperProfileSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxShaperProfileGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxQShaperEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxQShaperEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxQShaperProfileSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxQShaperProfileGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxBindPortToSchedulerProfileSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxBindPortToSchedulerProfileGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTx4TcTailDropProfileSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTx4TcTailDropProfileGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxTailDropProfileSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxTailDropProfileGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxSniffedPcktDescrLimitSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxSniffedPcktDescrLimitGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxMcastPcktDescrLimitSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxMcastPcktDescrLimitGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxWrrGlobalParamSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxWrrGlobalParamGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxShaperGlobalParamsSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxShaperGlobalParamsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxQWrrProfileSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxQWrrProfileGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxQArbGroupSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxQArbGroupGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxBindPortToDpSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxBindPortToDpGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxDescNumberGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxBufNumberGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTx4TcDescNumberGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTx4TcBufNumberGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxToCpuShaperModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxToCpuShaperModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxSharingGlobalResourceEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxSharingGlobalResourceEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxSharedGlobalResourceLimitsSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxSharedGlobalResourceLimitsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxSharedPolicySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxSharedPolicyGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxTailDropUcEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxTailDropUcEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxBufferTailDropEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxBufferTailDropEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxBuffersSharingMaxLimitSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxBuffersSharingMaxLimitGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxDp1SharedEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxDp1SharedEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxTcSharedProfileEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxTcSharedProfileEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxShaperTokenBucketMtuSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxShaperTokenBucketMtuGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxRandomTailDropEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxRandomTailDropEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTxPortAllShapersDisable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTxPortShapersDisable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxSchedulerDeficitModeEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxSchedulerDeficitModeEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxShaperModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxShaperModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxShaperBaselineSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxShaperBaselineGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxShaperConfigurationSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxShaperConfigurationGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxTailDropBufferConsumptionModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxTailDropBufferConsumptionModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxTcProfileSharedPoolSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxTcProfileSharedPoolGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxMcastBuffersLimitSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxMcastBuffersLimitGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxSniffedBuffersLimitSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxSniffedBuffersLimitGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxSharedPoolLimitsSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxSharedPoolLimitsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxProfileWeightedRandomTailDropEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxProfileWeightedRandomTailDropEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxTailDropWrtdMasksSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxTailDropWrtdMasksGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxResourceHistogramThresholdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxResourceHistogramThresholdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxResourceHistogramCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxGlobalDescNumberGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxGlobalBufNumberGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxMcastDescNumberGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxMcastBufNumberGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxSniffedDescNumberGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxSniffedBufNumberGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxSharedResourceDescNumberGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxSharedResourceBufNumberGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxShaperOnStackAsGigEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxShaperOnStackAsGigEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortProfileTxByteCountChangeEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortProfileTxByteCountChangeEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxByteCountChangeEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxByteCountChangeEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxByteCountChangeValueSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxByteCountChangeValueGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxGlobalDescLimitSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxGlobalDescLimitGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxMcFifoSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxMcFifoGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxMcFifoArbiterWeigthsSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxMcFifoArbiterWeigthsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxQueueGlobalTxEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxQueueGlobalTxEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxMcShaperMaskSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxMcShaperMaskGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxTailDropProfileBufferConsumptionModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxTailDropProfileBufferConsumptionModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxSchedulerProfileBufferConsumptionModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxSchedulerProfileBufferConsumptionModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxShaperAvbModeEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxShaperAvbModeEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxQShaperAvbModeEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxQShaperAvbModeEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxSchedulerProfileCountModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxSchedulerProfileCountModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxTailDropMaskSharedBuffEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxTailDropMaskSharedBuffEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxTailDropSharedBuffMaxLimitSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxTailDropSharedBuffMaxLimitGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTx4TcMcBufNumberGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxQueueDumpAll_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxBufferStatisticsEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxBufferStatisticsEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxMaxBufferFillLevelSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxMaxBufferFillLevelGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxQueueMaxBufferFillLevelSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxQueueMaxBufferFillLevelGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxMcFifoPriorityAttributesSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxMcFifoPriorityAttributesGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxMcastBuffersPriorityLimitSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxMcastBuffersPriorityLimitGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxMcastAvailableBuffersSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxMcastAvailableBuffersGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxTcMapToSharedPoolSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxTcMapToSharedPoolGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxTailDropGlobalParamsSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxTailDropGlobalParamsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTx4TcTailDropWredProfileSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTx4TcTailDropWredProfileGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxTailDropWredProfileSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxTailDropWredProfileGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxTailDropCommandSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxTailDropCommandGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxQMinimalRateEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxQMinimalRateEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxQMinimalRateSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxQMinimalRateGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxQueueLowLatencyModeEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxQueueLowLatencyModeEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxQueueOptimizedThroughputEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxQueueOptimizedThroughputEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxQueueOffsetWidthSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxQueueOffsetWidthGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxQueueOffsetModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxQueueOffsetModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxQueueOffsetMapSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxQueueOffsetMapGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxPriorityGroupSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxPriorityGroupGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxWrrWeightSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxWrrWeightGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxDbaEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxDbaEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxDbaDebugControl_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxDbaAvailBuffSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxDbaAvailBuffGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxDbaDebugClearGlobalCounters_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxDbaDebugGlobalCountersGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxDbaDebugProfileCountersEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxDbaDebugProfileCountersGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxUburstEventInfoGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxUburstDetectionEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxUburstDetectionEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxUburstDetectionProfileBindSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxUburstDetectionProfileBindGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxUburstDetectionProfileSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortTxUburstDetectionProfileGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPizzaArbiterDevStateInit_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPizzaArbiterDevStateGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPizzaArbiterIfConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortResourcesConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPizzaArbiterIfPortStateGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPizzaArbiterIfSliceStateGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPizzaArbiterIfUserTableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPizzaArbiterIfUserTableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPizzaArbiterPipeBWMinPortSpeedResolutionGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPizzaArbiterBWModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortPizzaArbiterBWModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortDynamicPATxQHighSpeedPortGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPortDynamicPATxQHighSpeedPortDumpGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChSlicesNumSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChSlicesNumGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChSliceStateSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChSliceStateGet_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssDxChPortLogh */
