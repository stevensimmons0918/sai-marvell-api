/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssPxCpssHwInitLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssPxCpssHwInitLogh
#define __prvCpssPxCpssHwInitLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_MAP_DECLARE_MAC(CPSS_PX_TABLE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_PHA_FIRMWARE_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_POWER_SUPPLIES_NUMBER_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_LOGICAL_TABLE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_SHADOW_TYPE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_PX_INIT_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_LED_CLASS_MANIPULATION_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_LED_CONF_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_LOGICAL_TABLES_SHADOW_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_PX_INIT_INFO_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_LED_CLASS_MANIPULATION_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_LED_CONF_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_TABLE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_HW_PP_RESET_SKIP_TYPE_ENT_skipType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_LED_GROUP_CONF_STC_PTR_groupParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PP_DEVICE_TYPE_devType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_INIT_INFO_STC_PTR_initParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_LED_CLASS_MANIPULATION_STC_PTR_classParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_LED_CONF_STC_PTR_ledConfPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_invertEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_linkChangeOverride;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_skipEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_classNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_groupNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_period;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_position;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_HW_PP_INIT_STAGE_ENT_PTR_initStagePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_LED_GROUP_CONF_STC_PTR_groupParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PP_DEVICE_TYPE_PTR_deviceTypePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_LED_CLASS_MANIPULATION_STC_PTR_classParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_LED_CONF_STC_PTR_ledConfPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_invertEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_linkChangeOverridePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_skipEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_HW_DEV_NUM_PTR_hwDevNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_cncDescSizePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_coreClkPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_periodPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_positionPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_rxBufAlignmentPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_rxDescSizePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_txDescSizePtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssPxHwInit_E = (CPSS_LOG_LIB_HW_INIT_E << 16) | (2 << 24),
    PRV_CPSS_LOG_FUNC_cpssPxHwDescSizeGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxHwPpSoftResetTrigger_E,
    PRV_CPSS_LOG_FUNC_cpssPxHwPpSoftResetSkipParamSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxHwPpSoftResetSkipParamGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxHwPpInitStageGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxHwInterruptCoalescingSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxHwInterruptCoalescingGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxHwCoreClockGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCfgHwDevNumGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxLedStreamClassManipulationSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxLedStreamClassManipulationGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxLedStreamConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxLedStreamConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxLedStreamGroupConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxLedStreamGroupConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxLedStreamPortPositionSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxLedStreamPortPositionGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxLedStreamPortClassPolarityInvertEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxLedStreamPortClassPolarityInvertEnableGet_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssPxCpssHwInitLogh */
