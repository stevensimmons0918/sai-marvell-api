/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChCpssHwInitLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssDxChCpssHwInitLogh
#define __prvCpssDxChCpssHwInitLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* manually implemented declarations *********/

void prvCpssLogParamFunc_CPSS_DXCH_IMPLEMENT_WA_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_IMPLEMENT_WA_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_LED_CPU_OR_PORT27_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_LED_INDICATION_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_LED_PORT_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_MAP_DECLARE_MAC(CPSS_DXCH_TABLE_ENT);
PRV_CPSS_LOG_STC_ENUM_MAP_DECLARE_MAC(CPSS_DXCH_LED_PHY_SELECT_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PP_SERDES_REF_CLOCK_ENT);
PRV_CPSS_LOG_STC_ENUM_MAP_DECLARE_MAC(CPSS_DXCH_LOGICAL_TABLE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_SHADOW_TYPE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_ADDR_DECODE_WINDOW_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_HW_PP_IMPLEMENT_WA_INIT_FALCON_PORT_DELETE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_LED_CLASS_CONF_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_LED_GROUP_CONF_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_LED_PHY_GLOBAL_CONF_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_LED_PHY_PORT_CONF_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PP_PHASE1_INIT_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PP_PHASE2_INIT_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_HA_2_PHASES_INIT_PHASE1_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_HA_2_PHASES_INIT_PHASE2_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_LOGICAL_TABLES_SHADOW_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_ADDR_DECODE_WINDOW_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_HW_PP_IMPLEMENT_WA_INIT_FALCON_PORT_DELETE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IMPLEMENT_WA_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_LED_CLASS_CONF_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_LED_CPU_OR_PORT27_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_LED_CPU_OR_PORT27_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_LED_GROUP_CONF_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_LED_INDICATION_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_LED_INDICATION_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_LED_PHY_GLOBAL_CONF_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_LED_PHY_PORT_CONF_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_LED_PORT_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PP_PHASE1_INIT_INFO_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PP_PHASE2_INIT_INFO_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TABLE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_ADDR_DECODE_WINDOW_CONFIG_STC_PTR_windowConfigPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_HW_PP_IMPLEMENT_WA_INIT_FALCON_PORT_DELETE_STC_PTR_infoPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IMPLEMENT_WA_ENT_PTR_waArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IMPLEMENT_WA_ENT_wa;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_LED_CLASS_CONF_STC_PTR_ledClassConfPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_LED_CPU_OR_PORT27_ENT_indicatedPort;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_LED_GROUP_CONF_STC_PTR_ledGroupConfPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_LED_INDICATION_ENT_indication;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_LED_PHY_GLOBAL_CONF_STC_PTR_ledGlobalConfigPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_LED_PHY_PORT_CONF_STC_PTR_ledPerPortConfigPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_LED_PORT_TYPE_ENT_portType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PP_PHASE1_INIT_INFO_STC_PTR_ppPhase1ParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PP_PHASE2_INIT_INFO_STC_PTR_ppPhase2ParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_HW_PP_RESET_SKIP_TYPE_ENT_skipType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_LED_CLASS_MANIPULATION_STC_PTR_classParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_LED_CONF_STC_PTR_ledConfPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_LED_GROUP_CONF_STC_PTR_groupParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_LED_TWO_CLASS_MODE_CONF_STC_PTR_ledTwoClassModeConfPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PP_DEVICE_TYPE_devType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_REG_VALUE_INFO_STC_PTR_initDataListPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_invertEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_linkChangeOverride;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_skipEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_CHIPLETS_BMP_chipletsBmp;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_HEX_dataBitmapMask;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_PTR_additionalInfoBmpArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_classNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_dataBitmap;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_directionBitmap;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_featuresBitmap;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_groupNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_hwDevNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_initDataListLen;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ledClassNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ledControl;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ledGroupNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ledInterfaceNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ledUnitIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_mppNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_mppSelect;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numOfWa;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_period;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_position;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_windowNumber;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_ADDR_DECODE_WINDOW_CONFIG_STC_PTR_windowConfigPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_LED_CPU_OR_PORT27_ENT_PTR_indicatedPortPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_LED_INDICATION_ENT_PTR_indicationPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_LED_PHY_GLOBAL_CONF_STC_PTR_ledGlobalConfigPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_LED_PHY_PORT_CONF_STC_PTR_ledPerPortConfigPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_HW_PP_INIT_STAGE_ENT_PTR_initStagePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_LED_CLASS_MANIPULATION_STC_PTR_classParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_LED_CONF_STC_PTR_ledConfPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_LED_GROUP_CONF_STC_PTR_groupParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_LED_TWO_CLASS_MODE_CONF_STC_PTR_ledTwoClassModeConfPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PP_DEVICE_TYPE_PTR_deviceTypePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_invertEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_isImplemented;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_linkChangeOverridePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_skipEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_HW_DEV_NUM_PTR_hwDevNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_byteAlignmentPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_coreClkDbPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_coreClkHwPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_dataBitmapPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_descSizePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_directionBitmapPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_modeBitmapPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_mppSelectPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_periodPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_positionPtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssDxChHwPpPhase1Init_E = (CPSS_LOG_LIB_HW_INIT_E << 16) | (1 << 24),
    PRV_CPSS_LOG_FUNC_cpssDxChHwPpPhase2Init_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHwPpStartInit_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHwPpImplementWaInit_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPpHwImplementWaGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHwAuDescSizeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHwRxDescSizeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHwTxDescSizeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHwRxBufAlignmentGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHwPpSoftResetTrigger_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHwPpSoftResetSkipParamSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHwPpSoftResetSkipParamGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHwPpSoftResetChipletsTrigger_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHwPpSoftResetChipletsSkipParamSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHwPpSoftResetChipletsSkipParamGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHwPpInitStageGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCfgHwDevNumSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCfgHwDevNumGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHwCoreClockGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHwInterruptCoalescingSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHwInterruptCoalescingGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHwInitLion2GeBackwardCompatibility_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHwPpAddressDecodeWindowConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHwPpAddressDecodeWindowConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChMicroInitBasicCodeGenerate_E,
    PRV_CPSS_LOG_FUNC_cpssDxChGpioPhyConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChGpioPhyConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChGpioPhyDataRead_E,
    PRV_CPSS_LOG_FUNC_cpssDxChGpioPhyDataWrite_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHwPpImplementWaInit_FalconPortDelete_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHwMppSelectSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHwMppSelectGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLedStreamClassAndGroupConfig_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLedStreamHyperGStackTxQStatusEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLedStreamHyperGStackTxQStatusEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLedStreamCpuOrPort27ModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLedStreamCpuOrPort27ModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLedStreamClassManipulationSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLedStreamClassManipulationGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLedStreamGroupConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLedStreamGroupConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLedStreamConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLedStreamConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLedStreamClassIndicationSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLedStreamClassIndicationGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLedStreamDirectModeEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLedStreamDirectModeEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLedStreamPortGroupConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLedStreamPortGroupConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLedStreamPortGroupClassManipulationSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLedStreamPortGroupClassManipulationGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLedStreamPortGroupGroupConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLedStreamPortGroupGroupConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLedStreamPortPositionSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLedStreamPortPositionGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLedStreamPortClassIndicationSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLedStreamPortClassIndicationGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLedStreamPortClassPolarityInvertEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLedStreamPortClassPolarityInvertEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLedPhyControlGlobalSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLedPhyControlGlobalGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLedPhyControlPerPortSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLedPhyControlPerPortGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLedStreamTwoClassModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLedStreamTwoClassModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLedStreamUnitClassManipulationSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLedStreamUnitClassManipulationGet_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssDxChCpssHwInitLogh */
