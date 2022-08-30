/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChLatencyMonitoringLog.c
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
#include <cpss/dxCh/dxChxGen/latencyMonitoring/cpssDxChLatencyMonitoring.h>
#include <cpss/dxCh/dxChxGen/latencyMonitoring/private/prvCpssDxChLatencyMonitoringLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssGenDbLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT[]  =
{
    "CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_DETERMINISTIC_E",
    "CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_RANDOM_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_LATENCY_PROFILE_MODE_ENT[]  =
{
    "CPSS_DXCH_LATENCY_PROFILE_MODE_PORT_E",
    "CPSS_DXCH_LATENCY_PROFILE_MODE_QUEUE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_LATENCY_PROFILE_MODE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_LATENCY_MONITORING_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_LATENCY_MONITORING_CFG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, rangeMax);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, rangeMin);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, notificationThresh);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_LATENCY_MONITORING_STAT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_LATENCY_MONITORING_STAT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, minLatency);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxLatency);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, avgLatency);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, packetsInRange, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, packetsOutOfRange, GT_U64);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_LATENCY_MONITORING_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_LATENCY_MONITORING_CFG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_LATENCY_MONITORING_CFG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_LATENCY_MONITORING_STAT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_LATENCY_MONITORING_STAT_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_LATENCY_MONITORING_STAT_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_LATENCY_PROFILE_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_LATENCY_PROFILE_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_LATENCY_PROFILE_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_LATENCY_PROFILE_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_LATENCY_PROFILE_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_LATENCY_PROFILE_MODE_ENT);
}
void prvCpssLogParamFunc_GT_LATENCY_MONITORING_UNIT_BMP(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    char * formatPtr = "%s = %d\n";
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(GT_LATENCY_MONITORING_UNIT_BMP, paramVal);
    if (inOutParamInfoPtr->formatPtr)
    {
        formatPtr = inOutParamInfoPtr->formatPtr;
    }
    PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, formatPtr, namePtr, paramVal);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DIRECTION_ENT_portType = {
     "portType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DIRECTION_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_LATENCY_MONITORING_CFG_STC_PTR_lmuConfigPtr = {
     "lmuConfigPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_LATENCY_MONITORING_CFG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT_samplingMode = {
     "samplingMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_LATENCY_PROFILE_MODE_ENT_profileMode = {
     "profileMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_LATENCY_PROFILE_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_enabled = {
     "enabled", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_LATENCY_MONITORING_UNIT_BMP_lmuBmp = {
     "lmuBmp", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_LATENCY_MONITORING_UNIT_BMP)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_PHYSICAL_PORT_NUM_trgPortNum = {
     "trgPortNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_PHYSICAL_PORT_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_latencyProfile = {
     "latencyProfile", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_portProfile = {
     "portProfile", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_queueProfile = {
     "queueProfile", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_samplingProfile = {
     "samplingProfile", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_samplingThreshold = {
     "samplingThreshold", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_LATENCY_MONITORING_CFG_STC_PTR_lmuConfigPtr = {
     "lmuConfigPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_LATENCY_MONITORING_CFG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT_PTR_samplingModePtr = {
     "samplingModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_LATENCY_MONITORING_STAT_STC_PTR_statisticsPtr = {
     "statisticsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_LATENCY_MONITORING_STAT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_LATENCY_PROFILE_MODE_ENT_PTR_profileModePtr = {
     "profileModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_LATENCY_PROFILE_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_portProfilePtr = {
     "portProfilePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_queueProfilePtr = {
     "queueProfilePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_samplingProfilePtr = {
     "samplingProfilePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_samplingThresholdPtr = {
     "samplingThresholdPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChLatencyMonitoringProfileModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_LATENCY_PROFILE_MODE_ENT_profileMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChLatencyMonitoringStatReset_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_LATENCY_MONITORING_UNIT_BMP_lmuBmp,
    &DX_IN_GT_U32_latencyProfile
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChLatencyMonitoringCfgSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_LATENCY_MONITORING_UNIT_BMP_lmuBmp,
    &DX_IN_GT_U32_latencyProfile,
    &DX_IN_CPSS_DXCH_LATENCY_MONITORING_CFG_STC_PTR_lmuConfigPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChLatencyMonitoringCfgGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_LATENCY_MONITORING_UNIT_BMP_lmuBmp,
    &DX_IN_GT_U32_latencyProfile,
    &DX_OUT_CPSS_DXCH_LATENCY_MONITORING_CFG_STC_PTR_lmuConfigPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChLatencyMonitoringStatGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_LATENCY_MONITORING_UNIT_BMP_lmuBmp,
    &DX_IN_GT_U32_latencyProfile,
    &DX_OUT_CPSS_DXCH_LATENCY_MONITORING_STAT_STC_PTR_statisticsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChLatencyMonitoringPortCfgSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DIRECTION_ENT_portType,
    &DX_IN_GT_U32_index,
    &DX_IN_GT_U32_profile
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChLatencyMonitoringPortCfgGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DIRECTION_ENT_portType,
    &DX_OUT_GT_U32_PTR_indexPtr,
    &DX_OUT_GT_U32_PTR_profilePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChLatencyMonitoringPortProfileSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U32_portProfile
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChLatencyMonitoringPortProfileGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_U32_PTR_portProfilePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChLatencyMonitoringEnableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_srcPortNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_trgPortNum,
    &DX_IN_GT_BOOL_enabled
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChLatencyMonitoringEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_srcPortNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_trgPortNum,
    &DX_OUT_GT_BOOL_PTR_enabledPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChLatencyMonitoringSamplingProfileSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_latencyProfile,
    &DX_IN_GT_U32_samplingProfile
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChLatencyMonitoringSamplingProfileGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_latencyProfile,
    &DX_OUT_GT_U32_PTR_samplingProfilePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChLatencyMonitoringSamplingConfigurationSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_samplingProfile,
    &DX_IN_CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT_samplingMode,
    &DX_IN_GT_U32_samplingThreshold
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChLatencyMonitoringSamplingConfigurationGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_samplingProfile,
    &DX_OUT_CPSS_DXCH_LATENCY_MONITORING_SAMPLING_MODE_ENT_PTR_samplingModePtr,
    &DX_OUT_GT_U32_PTR_samplingThresholdPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChLatencyMonitoringQueueProfileSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U8_tcQueue,
    &DX_IN_GT_U32_queueProfile
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChLatencyMonitoringQueueProfileGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U8_tcQueue,
    &DX_OUT_GT_U32_PTR_queueProfilePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChLatencyMonitoringProfileModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_LATENCY_PROFILE_MODE_ENT_PTR_profileModePtr
};


/********* lib API DB *********/

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChLatencyMonitoringLogLibDb[] = {
    {"cpssDxChLatencyMonitoringPortCfgSet", 5, cpssDxChLatencyMonitoringPortCfgSet_PARAMS, NULL},
    {"cpssDxChLatencyMonitoringPortCfgGet", 5, cpssDxChLatencyMonitoringPortCfgGet_PARAMS, NULL},
    {"cpssDxChLatencyMonitoringEnableSet", 4, cpssDxChLatencyMonitoringEnableSet_PARAMS, NULL},
    {"cpssDxChLatencyMonitoringEnableGet", 4, cpssDxChLatencyMonitoringEnableGet_PARAMS, NULL},
    {"cpssDxChLatencyMonitoringSamplingProfileSet", 3, cpssDxChLatencyMonitoringSamplingProfileSet_PARAMS, NULL},
    {"cpssDxChLatencyMonitoringSamplingProfileGet", 3, cpssDxChLatencyMonitoringSamplingProfileGet_PARAMS, NULL},
    {"cpssDxChLatencyMonitoringSamplingConfigurationSet", 4, cpssDxChLatencyMonitoringSamplingConfigurationSet_PARAMS, NULL},
    {"cpssDxChLatencyMonitoringSamplingConfigurationGet", 4, cpssDxChLatencyMonitoringSamplingConfigurationGet_PARAMS, NULL},
    {"cpssDxChLatencyMonitoringStatGet", 4, cpssDxChLatencyMonitoringStatGet_PARAMS, NULL},
    {"cpssDxChLatencyMonitoringStatReset", 3, cpssDxChLatencyMonitoringStatReset_PARAMS, NULL},
    {"cpssDxChLatencyMonitoringCfgSet", 4, cpssDxChLatencyMonitoringCfgSet_PARAMS, NULL},
    {"cpssDxChLatencyMonitoringCfgGet", 4, cpssDxChLatencyMonitoringCfgGet_PARAMS, NULL},
    {"cpssDxChLatencyMonitoringPortEnableSet", 3, prvCpssLogGenDevNumPortNumEnable_PARAMS, NULL},
    {"cpssDxChLatencyMonitoringPortEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr_PARAMS, NULL},
    {"cpssDxChLatencyMonitoringProfileModeSet", 2, cpssDxChLatencyMonitoringProfileModeSet_PARAMS, NULL},
    {"cpssDxChLatencyMonitoringProfileModeGet", 2, cpssDxChLatencyMonitoringProfileModeGet_PARAMS, NULL},
    {"cpssDxChLatencyMonitoringPortProfileSet", 3, cpssDxChLatencyMonitoringPortProfileSet_PARAMS, NULL},
    {"cpssDxChLatencyMonitoringPortProfileGet", 3, cpssDxChLatencyMonitoringPortProfileGet_PARAMS, NULL},
    {"cpssDxChLatencyMonitoringQueueProfileSet", 3, cpssDxChLatencyMonitoringQueueProfileSet_PARAMS, NULL},
    {"cpssDxChLatencyMonitoringQueueProfileGet", 3, cpssDxChLatencyMonitoringQueueProfileGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_LATENCY_MONITORING(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChLatencyMonitoringLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChLatencyMonitoringLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

