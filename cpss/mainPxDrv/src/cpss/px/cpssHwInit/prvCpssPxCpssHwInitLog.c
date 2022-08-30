/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssPxCpssHwInitLog.c
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
#include <cpss/common/cpssHwInit/private/prvCpssCommonCpssHwInitLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssPxGenDbLog.h>
#include <cpss/px/cpssHwInit/cpssPxHwInit.h>
#include <cpss/px/cpssHwInit/cpssPxHwInitLedCtrl.h>
#include <cpss/px/cpssHwInit/cpssPxTables.h>
#include <cpss/px/cpssHwInit/private/prvCpssPxCpssHwInitLog.h>


/********* enums *********/

const PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_CPSS_PX_TABLE_ENT[]  =
{PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_TABLE_BMA_PORT_MAPPING_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_TABLE_BMA_MULTICAST_COUNTERS_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_TABLE_TXQ_DQ_PFC_RESPONSE_PORT_MAPPING_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_MAX_QUEUE_LIMITS_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_LIMITS_DP0_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_LIMITS_DP1_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_LIMITS_DP2_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_BUF_LIMITS_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_DESC_LIMITS_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_COUNTERS_Q_MAIN_BUFF_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_TABLE_TXQ_QCN_CN_SAMPLE_INTERVALS_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_TABLE_PFC_LLFC_COUNTERS_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_TABLE_CNC_0_COUNTERS_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_TABLE_CNC_1_COUNTERS_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_TABLE_PCP_DST_PORT_MAP_TABLE_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_TABLE_PCP_PORT_FILTERING_TABLE_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_TABLE_PHA_HA_TABLE_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_TABLE_PHA_SRC_PORT_DATA_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_TABLE_PHA_TARGET_PORT_DATA_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_TABLE_PHA_SHARED_DMEM_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_MULTI_INSTANCE_TXQ_DQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG____DQ_1___E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_MULTI_INSTANCE_TXQ_DQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG____DQ_1___E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_MULTI_INSTANCE_TXQ_DQ_PFC_RESPONSE_PORT_MAPPING_E____DQ_1___E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_TABLE_PHA_FW_IMAGE_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_TABLE_LAST_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_INTERNAL_TABLE_MANAGEMENT_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_INTERNAL_TABLE_PACKET_DATA_ECC_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_INTERNAL_TABLE_PACKET_DATA_PARITY_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_INTERNAL_TABLE_PACKET_DATA_CONTROL_ECC_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_INTERNAL_TABLE_PACKET_DATA_CONTROL_PARITY_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_INTERNAL_TABLE_DESCRIPTORS_CONTROL_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_INTERNAL_TABLE_CM3_RAM_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_INTERNAL_TABLE_PHA_RAM_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_HW_TABLE_LAST_E)
};
PRV_CPSS_LOG_STC_ENUM_MAP_ARRAY_SIZE_MAC(CPSS_PX_TABLE_ENT);
const char * const prvCpssLogEnum_CPSS_PX_PHA_FIRMWARE_TYPE_ENT[]  =
{
    "CPSS_PX_PHA_FIRMWARE_TYPE_DEFAULT_E",
    "CPSS_PX_PHA_FIRMWARE_TYPE_NOT_USED_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PHA_FIRMWARE_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_PX_POWER_SUPPLIES_NUMBER_ENT[]  =
{
    "CPSS_PX_POWER_SUPPLIES_DUAL_E",
    "CPSS_PX_POWER_SUPPLIES_SINGLE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_POWER_SUPPLIES_NUMBER_ENT);
const char * const prvCpssLogEnum_CPSS_PX_LOGICAL_TABLE_ENT[]  =
{
    "CPSS_PX_LOGICAL_TABLE_INGRESS_DST_PORT_MAP_TABLE_E",
    "CPSS_PX_LOGICAL_TABLE_INGRESS_PORT_FILTERING_TABLE_E",
    "CPSS_PX_LOGICAL_TABLE_EGRESS_HEADER_ALTERATION_TABLE_E",
    "CPSS_PX_LOGICAL_TABLE_TAIL_DROP_LIMITS_E",
    "CPSS_PX_LOGICAL_TABLE_BUFFERS_MANAGER_MULTICAST_COUNTERS_E",
    "CPSS_PX_LOGICAL_TABLE_CNC_0_COUNTERS_E",
    "CPSS_PX_LOGICAL_TABLE_CNC_1_COUNTERS_E",
    "CPSS_PX_LOGICAL_TABLE_TXQ_QCN_CN_SAMPLE_INTERVALS_E",
    "CPSS_PX_LOGICAL_TABLE_PHA_SHARED_DMEM_E",
    "CPSS_PX_LOGICAL_TABLE_PHA_FW_IMAGE_E",
    "CPSS_PX_LOGICAL_INTERNAL_TABLE_MANAGEMENT_E",
    "CPSS_PX_LOGICAL_INTERNAL_TABLE_PACKET_DATA_ECC_E",
    "CPSS_PX_LOGICAL_INTERNAL_TABLE_PACKET_DATA_PARITY_E",
    "CPSS_PX_LOGICAL_INTERNAL_TABLE_CM3_RAM_E",
    "CPSS_PX_LOGICAL_INTERNAL_TABLE_PHA_RAM_E",
    "CPSS_PX_LOGICAL_INTERNAL_TABLE_PACKET_DATA_CONTROL_ECC_E",
    "CPSS_PX_LOGICAL_INTERNAL_TABLE_PACKET_DATA_CONTROL_PARITY_E",
    "CPSS_PX_LOGICAL_INTERNAL_TABLE_DESCRIPTORS_CONTROL_E",
    "CPSS_PX_LOGICAL_TABLE_LAST_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_LOGICAL_TABLE_ENT);
const char * const prvCpssLogEnum_CPSS_PX_SHADOW_TYPE_ENT[]  =
{
    "CPSS_PX_SHADOW_TYPE_NONE_E",
    "CPSS_PX_SHADOW_TYPE_CPSS_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_SHADOW_TYPE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_PX_INIT_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_INIT_INFO_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, hwInfo, CPSS_HW_INFO_STC);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, mngInterfaceType, CPSS_PP_INTERFACE_CHANNEL_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, isrAddrCompletionRegionsBmp);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, appAddrCompletionRegionsBmp);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numOfDataIntegrityElements);
    PRV_CPSS_LOG_STC_STC_PTR_MAC(valPtr, dataIntegrityShadowPtr, CPSS_PX_LOGICAL_TABLES_SHADOW_STC);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, allowHwAccessOnly);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, phaFirmwareType, CPSS_PX_PHA_FIRMWARE_TYPE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, powerSuppliesNumbers, CPSS_PX_POWER_SUPPLIES_NUMBER_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_LED_CLASS_MANIPULATION_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_LED_CLASS_MANIPULATION_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, blinkEnable);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, blinkSelect, CPSS_LED_BLINK_SELECT_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, forceEnable);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, forceData);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, pulseStretchEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, disableOnLinkDown);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_LED_CONF_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_LED_CONF_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, ledOrganize, CPSS_LED_ORDER_MODE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, blink0DutyCycle, CPSS_LED_BLINK_DUTY_CYCLE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, blink0Duration, CPSS_LED_BLINK_DURATION_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, blink1DutyCycle, CPSS_LED_BLINK_DUTY_CYCLE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, blink1Duration, CPSS_LED_BLINK_DURATION_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, pulseStretch, CPSS_LED_PULSE_STRETCH_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ledStart);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ledEnd);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, invertEnable);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, ledClockFrequency, CPSS_LED_CLOCK_OUT_FREQUENCY_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_LOGICAL_TABLES_SHADOW_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_LOGICAL_TABLES_SHADOW_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, logicalTableName, CPSS_PX_LOGICAL_TABLE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, shadowType, CPSS_PX_SHADOW_TYPE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_PX_INIT_INFO_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_INIT_INFO_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_INIT_INFO_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_LED_CLASS_MANIPULATION_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_LED_CLASS_MANIPULATION_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_LED_CLASS_MANIPULATION_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_LED_CONF_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_LED_CONF_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_LED_CONF_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_TABLE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_TABLE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAP_MAC(namePtr, paramVal, CPSS_PX_TABLE_ENT);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_HW_PP_RESET_SKIP_TYPE_ENT_skipType = {
     "skipType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_HW_PP_RESET_SKIP_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_LED_GROUP_CONF_STC_PTR_groupParamsPtr = {
     "groupParamsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_LED_GROUP_CONF_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PP_DEVICE_TYPE_devType = {
     "devType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PP_DEVICE_TYPE)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_INIT_INFO_STC_PTR_initParamsPtr = {
     "initParamsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_INIT_INFO_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_LED_CLASS_MANIPULATION_STC_PTR_classParamsPtr = {
     "classParamsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_LED_CLASS_MANIPULATION_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_LED_CONF_STC_PTR_ledConfPtr = {
     "ledConfPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_LED_CONF_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_invertEnable = {
     "invertEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_linkChangeOverride = {
     "linkChangeOverride", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_skipEnable = {
     "skipEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_classNum = {
     "classNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_groupNum = {
     "groupNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_period = {
     "period", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_position = {
     "position", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_HW_PP_INIT_STAGE_ENT_PTR_initStagePtr = {
     "initStagePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_HW_PP_INIT_STAGE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_LED_GROUP_CONF_STC_PTR_groupParamsPtr = {
     "groupParamsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_LED_GROUP_CONF_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PP_DEVICE_TYPE_PTR_deviceTypePtr = {
     "deviceTypePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PP_DEVICE_TYPE)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_LED_CLASS_MANIPULATION_STC_PTR_classParamsPtr = {
     "classParamsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_LED_CLASS_MANIPULATION_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_LED_CONF_STC_PTR_ledConfPtr = {
     "ledConfPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_LED_CONF_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_invertEnablePtr = {
     "invertEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_linkChangeOverridePtr = {
     "linkChangeOverridePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_skipEnablePtr = {
     "skipEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_HW_DEV_NUM_PTR_hwDevNumPtr = {
     "hwDevNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_HW_DEV_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_cncDescSizePtr = {
     "cncDescSizePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_coreClkPtr = {
     "coreClkPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_periodPtr = {
     "periodPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_positionPtr = {
     "positionPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_rxBufAlignmentPtr = {
     "rxBufAlignmentPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_rxDescSizePtr = {
     "rxDescSizePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_txDescSizePtr = {
     "txDescSizePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxHwDescSizeGet_PARAMS[] =  {
    &PX_IN_CPSS_PP_DEVICE_TYPE_devType,
    &PX_OUT_GT_U32_PTR_rxDescSizePtr,
    &PX_OUT_GT_U32_PTR_rxBufAlignmentPtr,
    &PX_OUT_GT_U32_PTR_txDescSizePtr,
    &PX_OUT_GT_U32_PTR_cncDescSizePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxHwPpSoftResetSkipParamSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_HW_PP_RESET_SKIP_TYPE_ENT_skipType,
    &PX_IN_GT_BOOL_skipEnable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxHwPpSoftResetSkipParamGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_HW_PP_RESET_SKIP_TYPE_ENT_skipType,
    &PX_OUT_GT_BOOL_PTR_skipEnablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxHwInit_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_INIT_INFO_STC_PTR_initParamsPtr,
    &PX_OUT_CPSS_PP_DEVICE_TYPE_PTR_deviceTypePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxLedStreamConfigSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_LED_CONF_STC_PTR_ledConfPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxHwInterruptCoalescingSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_BOOL_enable,
    &PX_IN_GT_U32_period,
    &PX_IN_GT_BOOL_linkChangeOverride
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxLedStreamPortClassPolarityInvertEnableSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_classNum,
    &PX_IN_GT_BOOL_invertEnable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxLedStreamPortClassPolarityInvertEnableGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_classNum,
    &PX_OUT_GT_BOOL_PTR_invertEnablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxLedStreamPortPositionSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_position
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxLedStreamPortPositionGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_GT_U32_PTR_positionPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxLedStreamClassManipulationSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_classNum,
    &PX_IN_CPSS_PX_LED_CLASS_MANIPULATION_STC_PTR_classParamsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxLedStreamClassManipulationGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_classNum,
    &PX_OUT_CPSS_PX_LED_CLASS_MANIPULATION_STC_PTR_classParamsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxLedStreamGroupConfigSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_groupNum,
    &PX_IN_CPSS_LED_GROUP_CONF_STC_PTR_groupParamsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxLedStreamGroupConfigGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_groupNum,
    &PX_OUT_CPSS_LED_GROUP_CONF_STC_PTR_groupParamsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxHwPpInitStageGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_CPSS_HW_PP_INIT_STAGE_ENT_PTR_initStagePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxLedStreamConfigGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_CPSS_PX_LED_CONF_STC_PTR_ledConfPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxHwInterruptCoalescingGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_GT_BOOL_PTR_enablePtr,
    &PX_OUT_GT_U32_PTR_periodPtr,
    &PX_OUT_GT_BOOL_PTR_linkChangeOverridePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxHwCoreClockGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_GT_U32_PTR_coreClkPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCfgHwDevNumGet_PARAMS[] =  {
    &PX_IN_GT_U8_devNum,
    &PX_OUT_GT_HW_DEV_NUM_PTR_hwDevNumPtr
};


/********* lib API DB *********/

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssPxCpssHwInitLogLibDb[] = {
    {"cpssPxHwInit", 3, cpssPxHwInit_PARAMS, NULL},
    {"cpssPxHwDescSizeGet", 5, cpssPxHwDescSizeGet_PARAMS, NULL},
    {"cpssPxHwPpSoftResetTrigger", 1, prvCpssLogGenDevNum2_PARAMS, NULL},
    {"cpssPxHwPpSoftResetSkipParamSet", 3, cpssPxHwPpSoftResetSkipParamSet_PARAMS, NULL},
    {"cpssPxHwPpSoftResetSkipParamGet", 3, cpssPxHwPpSoftResetSkipParamGet_PARAMS, NULL},
    {"cpssPxHwPpInitStageGet", 2, cpssPxHwPpInitStageGet_PARAMS, NULL},
    {"cpssPxHwInterruptCoalescingSet", 4, cpssPxHwInterruptCoalescingSet_PARAMS, NULL},
    {"cpssPxHwInterruptCoalescingGet", 4, cpssPxHwInterruptCoalescingGet_PARAMS, NULL},
    {"cpssPxHwCoreClockGet", 2, cpssPxHwCoreClockGet_PARAMS, NULL},
    {"cpssPxCfgHwDevNumGet", 2, cpssPxCfgHwDevNumGet_PARAMS, NULL},
    {"cpssPxLedStreamClassManipulationSet", 3, cpssPxLedStreamClassManipulationSet_PARAMS, NULL},
    {"cpssPxLedStreamClassManipulationGet", 3, cpssPxLedStreamClassManipulationGet_PARAMS, NULL},
    {"cpssPxLedStreamConfigSet", 2, cpssPxLedStreamConfigSet_PARAMS, NULL},
    {"cpssPxLedStreamConfigGet", 2, cpssPxLedStreamConfigGet_PARAMS, NULL},
    {"cpssPxLedStreamGroupConfigSet", 3, cpssPxLedStreamGroupConfigSet_PARAMS, NULL},
    {"cpssPxLedStreamGroupConfigGet", 3, cpssPxLedStreamGroupConfigGet_PARAMS, NULL},
    {"cpssPxLedStreamPortPositionSet", 3, cpssPxLedStreamPortPositionSet_PARAMS, NULL},
    {"cpssPxLedStreamPortPositionGet", 3, cpssPxLedStreamPortPositionGet_PARAMS, NULL},
    {"cpssPxLedStreamPortClassPolarityInvertEnableSet", 4, cpssPxLedStreamPortClassPolarityInvertEnableSet_PARAMS, NULL},
    {"cpssPxLedStreamPortClassPolarityInvertEnableGet", 4, cpssPxLedStreamPortClassPolarityInvertEnableGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_PX_CPSS_LOG_LIB_HW_INIT(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssPxCpssHwInitLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssPxCpssHwInitLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

