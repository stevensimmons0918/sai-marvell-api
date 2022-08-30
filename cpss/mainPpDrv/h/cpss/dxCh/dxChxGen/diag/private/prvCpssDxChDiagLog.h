/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChDiagLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssDxChDiagLogh
#define __prvCpssDxChDiagLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* manually implemented declarations *********/

void prvCpssLogParamFunc_CPSS_DXCH_DIAG_BIST_RESULT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_DIAG_EXT_MEMORY_INF_BIST_ERROR_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_DIAG_SERDES_TUNE_PORT_LANE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DIAG_PP_MEM_BIST_TEST_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DIAG_PP_MEM_BIST_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_DIAG_BIST_STATUS_ENT);
PRV_CPSS_LOG_STC_ENUM_MAP_DECLARE_MAC(CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_DIAG_DESCRIPTOR_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_DIAG_EXT_MEMORY_BIST_PATTERN_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_DIAG_SERDES_DUMP_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_DIAG_SERDES_OPTIMIZE_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_LOCATION_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_DIAG_BIST_RESULT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_DIAG_DESCRIPTOR_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_DIAG_EXT_MEMORY_INF_BIST_ERROR_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_DIAG_SERDES_TUNE_PORT_LANE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_LOGICAL_TABLES_SHADOW_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PORT_SERDES_DUMP_RESULTS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_LOCATION_FULL_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_LOCATION_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_LOGICAL_TABLE_SHADOW_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PORT_SERDES_DUMP_RESULTS_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_HW_INDEX_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_LOGICAL_INDEX_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_RAM_INDEX_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PORT_COMPHY_C28GP4X1_DUMP_PINS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PORT_COMPHY_C28GP4X4_DUMP_PINS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PORT_COMPHY_C28G_DUMP_SELECTED_FIELDS_0_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_LOGICAL_TABLE_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DIAG_PP_MEM_BIST_TEST_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DIAG_PP_MEM_BIST_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_DIAG_BIST_STATUS_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_DIAG_DESCRIPTOR_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_DIAG_DESCRIPTOR_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_DIAG_EXT_MEMORY_BIST_PATTERN_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_DIAG_SERDES_DUMP_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_DIAG_SERDES_OPTIMIZE_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_LOGICAL_TABLES_SHADOW_INFO_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PORT_SERDES_DUMP_RESULTS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_CPSS_DXCH_LOGICAL_TABLES_SHADOW_INFO_STC_PTR_tablesInfoPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_bufferLengthPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_eventsNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_rawDataLengthPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_regsNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_resultsNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT_errorType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT_injectMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DIAG_DATA_INTEGRITY_MPPM_MEMORY_LOCATION_STC_PTR_mppmMemLocationPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DIAG_PP_MEM_BIST_TEST_TYPE_ENT_bistTestType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DIAG_PP_MEM_BIST_TYPE_ENT_memBistType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DIAG_PP_MEM_TYPE_ENT_memType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DIAG_PP_REG_TYPE_ENT_regType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DIAG_TEST_PROFILE_ENT_profile;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT_memType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_DIAG_DESCRIPTOR_TYPE_ENT_descriptorType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_DIAG_EXT_MEMORY_BIST_PATTERN_ENT_pattern;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC_PTR_configPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_DIAG_SERDES_DUMP_TYPE_ENT_dumpType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_DIAG_SERDES_OPTIMIZE_MODE_ENT_optMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_DIAG_SERDES_TUNE_PORT_LANE_STC_PTR_portLaneArrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_ENT_sensorType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT_prbsType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC_PTR_locationPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC_PTR_memEntryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TABLE_ENT_tableType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_UNIT_ENT_unitId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_EVENT_MASK_SET_ENT_operation;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PACKET_CMD_ENT_pktCmd;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PP_INTERFACE_CHANNEL_ENT_ifChannel;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_32_thresholdValue;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_clearMemoryAfterTest;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_connect;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_countEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_doByteSwap;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_injectEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_printDump;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_tableValid;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_testWholeMemory;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_HEX_PTR_cyclicDataArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_HEX_PTR_testsToRunBmpPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_HEX_data;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_HEX_evExtData;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_HEX_extMemoBitmap;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_HEX_offset;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_HEX_regAddr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_HEX_regMask;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_HEX_smiRegOffset;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_HEX_startOffset;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_HEX_testedAreaOffset;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_blockIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_firstEntry;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_lastEntry;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_phyAddr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_portLaneArrLength;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_prbsTime;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_sensorNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_testedAreaLength;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_timeOut;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_UINTPTR_baseAddr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT_PTR_errorTypePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT_PTR_injectModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT_PTR_protectionTypePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_DIAG_BIST_RESULT_STC_PTR_resultsArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_DIAG_BIST_STATUS_ENT_PTR_resultsStatusPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC_PTR_eventsArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_DIAG_DESCRIPTOR_STC_PTR_descriptorPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_DIAG_EXT_MEMORY_INF_BIST_ERROR_STC_PTR_errorInfoArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_DIAG_PG_CONFIGURATIONS_STC_PTR_configPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_ENT_PTR_sensorTypePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_EVENT_MASK_SET_ENT_PTR_operationPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PACKET_CMD_ENT_PTR_pktCmdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_SERDES_DUMP_RESULTS_STC_PTR_dumpResultsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_SERDES_TUNE_STC_PTR_optResultArrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_32_PTR_thresholdValuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_blockFixedPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_burstTransmitDonePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_checkerLockedPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_connectPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_countEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_injectEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_isNoMoreEventsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_lockedPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_testStatusPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_wasWrapAroundPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_HEX_PTR_addrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_HEX_PTR_badRegPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_HEX_PTR_cyclicDataArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_HEX_PTR_dataPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_HEX_PTR_rawDataPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_HEX_PTR_readValPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_HEX_PTR_regAddrArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_HEX_PTR_regAddrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_HEX_PTR_regDataArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_HEX_PTR_regDataPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_HEX_PTR_testsResultBmpPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_HEX_PTR_writeValPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_entrySizePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_errorCntrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_errorCounterPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_failedRowPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_failedSegmentPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_failedSyndromePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_nextEntryIndexPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_rawDataLengthPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_regsNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_replacedIndexPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tableBufferArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_totalMemSizePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U64_PTR_patternCntrPtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssDxChDiagMemTest_E = (CPSS_LOG_LIB_DIAG_E << 16) | (1 << 24),
    PRV_CPSS_LOG_FUNC_cpssDxChDiagAllMemTest_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagMemWrite_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagMemRead_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagRegWrite_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagRegRead_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagPhyRegWrite_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagPhyRegRead_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagRegsNumGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagResetAndInitControllerRegsNumGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagRegsDump_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagResetAndInitControllerRegsDump_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagRegTest_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagAllRegTest_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagPrbsPortTransmitModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagPrbsPortTransmitModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagPrbsPortGenerateEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagPrbsPortGenerateEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagPrbsPortCheckEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagPrbsPortCheckEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagPrbsPortCheckReadyGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagPrbsPortStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagPrbsCyclicDataSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagPrbsCyclicDataGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagPrbsSerdesTestEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagPrbsSerdesTestEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagPrbsSerdesTransmitModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagPrbsSerdesTransmitModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagPrbsSerdesCounterClearOnReadEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagPrbsSerdesCounterClearOnReadEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagPrbsSerdesStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagMemoryBistsRun_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagMemoryBistBlockStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagDeviceTemperatureSensorsSelectSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagDeviceTemperatureSensorsSelectGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagDeviceTemperatureThresholdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagDeviceTemperatureThresholdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagDeviceTemperatureGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagDeviceVoltageGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagPortGroupMemWrite_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagPortGroupMemRead_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagPortGroupRegsNumGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagPortGroupRegsDump_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagSerdesTuningSystemInit_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagSerdesTuningTracePrintEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagSerdesTuningSystemClose_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagSerdesTuningRxTune_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagSerdesDumpInfo_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagRegDefaultsEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagFastBootSkipOwnDeviceInitEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagBistTriggerAllSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagBistResultsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagExternalMemoriesBistRun_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagUnitRegsNumGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagUnitRegsDump_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagUnitRegsPrint_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagTableDump_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagTablePrint_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagTableInfoGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagDataIntegrityEventsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagDataIntegrityEventMaskSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagDataIntegrityEventMaskGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagDataIntegrityErrorInfoGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagDataIntegrityErrorInjectionConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagDataIntegrityErrorInjectionConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagDataIntegrityErrorCountEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagDataIntegrityErrorCountEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagDataIntegrityProtectionTypeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagDataIntegrityTcamParityDaemonEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagDataIntegrityTcamParityDaemonEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagDataIntegritySerConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagDataIntegritySerConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagDataIntegrityTableScan_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagDataIntegrityTableEntryFix_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagDataIntegrityShadowEntryInfoGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagDescriptorRawGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagDescriptorPortGroupRawGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagDescriptorRawSizeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagDescriptorGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagDescriptorPortGroupGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagPacketGeneratorConnectSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagPacketGeneratorConnectGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagPacketGeneratorTransmitEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChDiagPacketGeneratorBurstTransmitStatusGet_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssDxChDiagLogh */
