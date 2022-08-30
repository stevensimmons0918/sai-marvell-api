/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssPxPtpLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssPxPtpLogh
#define __prvCpssPxPtpLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PTP_TAI_CLOCK_FREQ_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PTP_TAI_CLOCK_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PTP_TAI_TOD_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_PX_PTP_TAI_TOD_COUNT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_PTP_TAI_TOD_STEP_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_PTP_TSU_CONTROL_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_PTP_TYPE_KEY_FORMAT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_PTP_TYPE_KEY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_PX_PTP_TAI_CLOCK_FREQ_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PTP_TAI_CLOCK_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PTP_TAI_CLOCK_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PTP_TAI_TOD_COUNT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PTP_TAI_TOD_STEP_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PTP_TAI_TOD_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PTP_TSU_CONTROL_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PTP_TYPE_INDEX(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PTP_TYPE_KEY_FORMAT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_PTP_TYPE_KEY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PTP_TAI_CLOCK_FREQ_ENT_clockFrequency;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PTP_TAI_CLOCK_MODE_ENT_clockMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT_clockSelect;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT_pulseMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT_function;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PTP_TAI_TOD_COUNT_STC_PTR_todValuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PTP_TAI_TOD_STEP_STC_PTR_todStepPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PTP_TAI_TOD_TYPE_ENT_todValueType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PTP_TSU_CONTROL_STC_PTR_controlPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_ENT_counterType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PTP_TYPE_INDEX_ptpTypeIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PTP_TYPE_KEY_FORMAT_STC_PTR_keyDataPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PTP_TYPE_KEY_FORMAT_STC_PTR_keyMaskPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PTP_TYPE_KEY_STC_PTR_portKeyPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_32_fracNanoSecond;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_captureIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_extPulseWidth;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_gracefulStep;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_nanoSeconds;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_seconds;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PTP_TAI_CLOCK_MODE_ENT_PTR_clockModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT_PTR_clockSelectPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT_PTR_pulseModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT_PTR_functionPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PTP_TAI_TOD_COUNT_STC_PTR_todValuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PTP_TAI_TOD_STEP_STC_PTR_todStepPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PTP_TSU_CONTROL_STC_PTR_controlPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PTP_TYPE_KEY_FORMAT_STC_PTR_keyDataPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PTP_TYPE_KEY_FORMAT_STC_PTR_keyMaskPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PTP_TYPE_KEY_STC_PTR_portKeyPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_32_PTR_fracNanoSecondPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_extPulseWidthPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_gracefulStepPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_nanoSecondsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_secondsPtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiCaptureOverwriteEnableSet_E = (CPSS_LOG_LIB_PTP_E << 16) | (2 << 24),
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiCaptureOverwriteEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiClockCycleSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiClockCycleGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiClockModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiClockModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiExternalPulseWidthSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiExternalPulseWidthGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiFractionalNanosecondDriftSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiFractionalNanosecondDriftGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiGracefulStepSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiGracefulStepGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiIncomingClockCounterEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiIncomingClockCounterEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiIncomingClockCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiIncomingTriggerCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiInputClockSelectSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiInputClockSelectGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiInputTriggersCountEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiInputTriggersCountEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiOutputTriggerEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiOutputTriggerEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiPClockCycleSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiPClockCycleGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiPClockDriftAdjustEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiPtpPClockDriftAdjustEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiTodCaptureStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiTodSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiTodGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiTodStepSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiTodStepGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiTodUpdateCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiTodCounterFunctionSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiTodCounterFunctionGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiTodCounterFunctionTriggerSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiTodCounterFunctionTriggerGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTsuControlSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTsuControlGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTsuCountersClear_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTsuPacketCouterGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpPortTypeKeySet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpPortTypeKeyGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTypeKeyEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTypeKeyEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTypeKeyEntryEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTypeKeyEntryEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiPClockOutputEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiPClockOutputEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiPhaseUpdateEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiPhaseUpdateEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiPulseInModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxPtpTaiPulseInModeGet_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssPxPtpLogh */
