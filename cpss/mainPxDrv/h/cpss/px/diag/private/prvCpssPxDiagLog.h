/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssPxDiagLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssPxDiagLogh
#define __prvCpssPxDiagLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_DIAG_BIST_STATUS_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_DIAG_TEMPERATURE_SENSOR_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_DIAG_TRANSMIT_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_LOCATION_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_PX_DIAG_BIST_RESULT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_DIAG_PG_CONFIGURATIONS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_LOCATION_SPECIFIC_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_LOGICAL_SHADOW_TABLES_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_LOCATION_FULL_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_LOCATION_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_LOGICAL_TABLE_SHADOW_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_HW_INDEX_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_LOGICAL_TABLE_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_RAM_INDEX_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_PX_DIAG_BIST_RESULT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_DIAG_BIST_STATUS_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_DIAG_PG_CONFIGURATIONS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_DIAG_TEMPERATURE_SENSOR_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_DIAG_TEMPERATURE_SENSOR_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_DIAG_TRANSMIT_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_DIAG_TRANSMIT_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_LOCATION_SPECIFIC_INFO_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_LOGICAL_SHADOW_TABLES_INFO_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_INOUT_CPSS_PX_LOGICAL_SHADOW_TABLES_INFO_STC_PTR_tablesInfoPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_INOUT_GT_U32_PTR_eventsNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_INOUT_GT_U32_PTR_regsNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_INOUT_GT_U32_PTR_resultsNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT_errorType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT_injectMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_DIAG_DATA_INTEGRITY_MPPM_MEMORY_LOCATION_STC_PTR_mppmMemLocationPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_DIAG_PP_REG_TYPE_ENT_regType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_DIAG_TEST_PROFILE_ENT_profile;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_EVENT_MASK_SET_ENT_operation;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PP_INTERFACE_CHANNEL_ENT_ifChannel;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_DIAG_PG_CONFIGURATIONS_STC_PTR_configPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_DIAG_TEMPERATURE_SENSOR_ENT_sensorType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_DIAG_TRANSMIT_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_LOCATION_SPECIFIC_INFO_STC_PTR_locationPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_LOCATION_SPECIFIC_INFO_STC_PTR_memEntryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_32_thresholdValue;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_connect;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_countEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_doByteSwap;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_injectEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_HEX_PTR_cyclicDataArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_HEX_data;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_HEX_evExtData;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_HEX_offset;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_HEX_regAddr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_HEX_regMask;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_numOfEntries;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_sensorNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_UINTPTR_baseAddr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT_PTR_errorTypePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT_PTR_injectModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT_PTR_protectionTypePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_EVENT_MASK_SET_ENT_PTR_operationPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_DIAG_BIST_RESULT_STC_PTR_resultsArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_DIAG_BIST_STATUS_ENT_PTR_resultsStatusPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC_PTR_eventsArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_DIAG_PG_CONFIGURATIONS_STC_PTR_configPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_DIAG_TEMPERATURE_SENSOR_ENT_PTR_sensorTypePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_DIAG_TRANSMIT_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_32_PTR_temperaturePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_32_PTR_thresholdValuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_burstTransmitDonePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_checkerLockedPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_connectPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_countEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_injectEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_isNoMoreEventsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_isReadyPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_lockedPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_testStatusPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_wasWrapAroundPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_HEX_PTR_badRegPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_HEX_PTR_cyclicDataArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_HEX_PTR_dataPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_HEX_PTR_readValPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_HEX_PTR_regAddrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_HEX_PTR_regDataPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_HEX_PTR_writeValPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_errorCntrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_failedRowPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_failedSegmentPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_failedSyndromePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_nextEntryIndexPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_regsNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_totalMemSizePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_voltagePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U64_PTR_patternCntrPtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssPxDiagRegsNumGet_E = (CPSS_LOG_LIB_DIAG_E << 16) | (2 << 24),
    PRV_CPSS_LOG_FUNC_cpssPxDiagResetAndInitControllerRegsNumGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagRegWrite_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagRegRead_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagRegsDump_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagResetAndInitControllerRegsDump_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagRegTest_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagAllRegTest_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagDeviceTemperatureSensorsSelectSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagDeviceTemperatureSensorsSelectGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagDeviceTemperatureThresholdSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagDeviceTemperatureThresholdGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagDeviceTemperatureGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagDeviceVoltageGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagPrbsPortTransmitModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagPrbsPortTransmitModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagPrbsPortGenerateEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagPrbsPortGenerateEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagPrbsPortCheckEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagPrbsPortCheckEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagPrbsPortCheckReadyGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagPrbsPortStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagPrbsCyclicDataSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagPrbsCyclicDataGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagPrbsSerdesTestEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagPrbsSerdesTestEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagPrbsSerdesTransmitModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagPrbsSerdesTransmitModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagPrbsSerdesCounterClearOnReadEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagPrbsSerdesCounterClearOnReadEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagPrbsSerdesStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagBistTriggerAllSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagBistResultsGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagDataIntegrityEventsGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagDataIntegrityEventMaskSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagDataIntegrityEventMaskGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagDataIntegrityErrorInfoGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagDataIntegrityErrorInjectionConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagDataIntegrityErrorInjectionConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagDataIntegrityErrorCountEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagDataIntegrityErrorCountEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagDataIntegrityProtectionTypeGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagDataIntegrityTableScan_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagDataIntegrityTableEntryFix_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagDataIntegrityShadowTableSizeGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagPacketGeneratorConnectSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagPacketGeneratorConnectGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagPacketGeneratorTransmitEnable_E,
    PRV_CPSS_LOG_FUNC_cpssPxDiagPacketGeneratorBurstTransmitStatusGet_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssPxDiagLogh */
