/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChL2mllLog.c
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
#include <cpss/dxCh/dxChxGen/l2mll/cpssDxChL2Mll.h>
#include <cpss/dxCh/dxChxGen/l2mll/private/prvCpssDxChL2mllLog.h>
#include <cpss/dxCh/dxChxGen/tunnel/private/prvCpssDxChTunnelLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssGenDbLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_DXCH_L2MLL_PORT_TRUNK_CNT_MODE_ENT[]  =
{
    "CPSS_DXCH_L2MLL_DISREGARD_PORT_TRUNK_CNT_MODE_E",
    "CPSS_DXCH_L2MLL_PORT_CNT_MODE_E",
    "CPSS_DXCH_L2MLL_TRUNK_CNT_MODE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_L2MLL_PORT_TRUNK_CNT_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_L2MLL_VLAN_CNT_MODE_ENT[]  =
{
    "CPSS_DXCH_L2MLL_DISREGARD_VLAN_CNT_MODE_E",
    "CPSS_DXCH_L2MLL_USE_VLAN_CNT_MODE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_L2MLL_VLAN_CNT_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_ENT[]  =
{
    "CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E",
    "CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_SECOND_MLL_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_L2_MLL_EXCEPTION_COUNTERS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_L2_MLL_EXCEPTION_COUNTERS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, skip);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ttl);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_L2_MLL_LTT_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_L2_MLL_LTT_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, mllPointer);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, entrySelector, CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, mllMaskProfileEnable);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, mllMaskProfile);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_L2_MLL_MULTI_TARGET_PORT_RANGE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_L2_MLL_MULTI_TARGET_PORT_RANGE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, minValue);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxValue);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_L2_MLL_PAIR_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_L2_MLL_PAIR_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, firstMllNode, CPSS_DXCH_L2_MLL_ENTRY_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, secondMllNode, CPSS_DXCH_L2_MLL_ENTRY_STC);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, nextPointer);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, entrySelector, CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_L2_MLL_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_L2_MLL_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, last);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, unknownUcFilterEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, unregMcFilterEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, bcFilterEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, mcLocalSwitchingEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, maxHopCountEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxOutgoingHopCount);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, egressInterface, CPSS_INTERFACE_INFO_STC);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, maskBitmap);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ttlThreshold);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, bindToMllCounterEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, mllCounterIndex);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, onePlusOneFilteringEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, meshId);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, tunnelStartEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, tunnelStartPointer);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, tunnelStartPassengerType, CPSS_DXCH_TUNNEL_PASSANGER_TYPE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_L2MLL_COUNTER_SET_INTERFACE_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_L2MLL_COUNTER_SET_INTERFACE_CFG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_L2MLL_COUNTER_SET_INTERFACE_CFG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_L2_MLL_EXCEPTION_COUNTERS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_L2_MLL_EXCEPTION_COUNTERS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_L2_MLL_EXCEPTION_COUNTERS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_L2_MLL_LTT_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_L2_MLL_LTT_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_L2_MLL_LTT_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_L2_MLL_MULTI_TARGET_PORT_RANGE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_L2_MLL_MULTI_TARGET_PORT_RANGE_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_L2_MLL_MULTI_TARGET_PORT_RANGE_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_L2_MLL_PAIR_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_L2_MLL_PAIR_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_L2_MLL_PAIR_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_L2MLL_COUNTER_SET_INTERFACE_CFG_STC_PTR_interfaceCfgPtr = {
     "interfaceCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_L2MLL_COUNTER_SET_INTERFACE_CFG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_L2_MLL_LTT_ENTRY_STC_PTR_lttEntryPtr = {
     "lttEntryPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_L2_MLL_LTT_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_L2_MLL_MULTI_TARGET_PORT_RANGE_STC_PTR_portRangeConfigPtr = {
     "portRangeConfigPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_L2_MLL_MULTI_TARGET_PORT_RANGE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_L2_MLL_PAIR_STC_PTR_mllPairEntryPtr = {
     "mllPairEntryPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_L2_MLL_PAIR_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_trapEnable = {
     "trapEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_maxVidxIndex = {
     "maxVidxIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_mllPointer = {
     "mllPointer", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_portBase = {
     "portBase", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_vidxBase = {
     "vidxBase", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_targetDevNum = {
     "targetDevNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_targetPortNum = {
     "targetPortNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_L2MLL_COUNTER_SET_INTERFACE_CFG_STC_PTR_interfaceCfgPtr = {
     "interfaceCfgPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_L2MLL_COUNTER_SET_INTERFACE_CFG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_L2_MLL_EXCEPTION_COUNTERS_STC_PTR_countersPtr = {
     "countersPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_L2_MLL_EXCEPTION_COUNTERS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_L2_MLL_LTT_ENTRY_STC_PTR_lttEntryPtr = {
     "lttEntryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_L2_MLL_LTT_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_L2_MLL_MULTI_TARGET_PORT_RANGE_STC_PTR_portRangeConfigPtr = {
     "portRangeConfigPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_L2_MLL_MULTI_TARGET_PORT_RANGE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_L2_MLL_PAIR_STC_PTR_mllPairEntryPtr = {
     "mllPairEntryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_L2_MLL_PAIR_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_trapEnablePtr = {
     "trapEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_maxVidxIndexPtr = {
     "maxVidxIndexPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_mllPointer = {
     "mllPointer", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_portBasePtr = {
     "portBasePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_vidxBasePtr = {
     "vidxBasePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChL2MllMultiTargetPortRangeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_L2_MLL_MULTI_TARGET_PORT_RANGE_STC_PTR_portRangeConfigPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChL2MllTtlExceptionConfigurationSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_BOOL_trapEnable,
    &DX_IN_CPSS_NET_RX_CPU_CODE_ENT_cpuCode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChL2MllPortGroupCounterGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_index,
    &DX_OUT_GT_U32_PTR_counterPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChL2MllPortGroupExceptionCountersGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_OUT_CPSS_DXCH_L2_MLL_EXCEPTION_COUNTERS_STC_PTR_countersPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChL2MllVidxEnableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U16_vidx,
    &DX_IN_GT_BOOL_enable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChL2MllVidxEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U16_vidx,
    &DX_OUT_GT_BOOL_PTR_enablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChL2MllLttEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_IN_CPSS_DXCH_L2_MLL_LTT_ENTRY_STC_PTR_lttEntryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChL2MllLttEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_OUT_CPSS_DXCH_L2_MLL_LTT_ENTRY_STC_PTR_lttEntryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChL2MllCounterGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_OUT_GT_U32_PTR_counterPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChL2MllLookupMaxVidxIndexSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_maxVidxIndex
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChL2MllSetCntInterfaceCfg_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_mllCntSet,
    &DX_IN_CPSS_DXCH_L2MLL_COUNTER_SET_INTERFACE_CFG_STC_PTR_interfaceCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChL2MllGetCntInterfaceCfg_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_mllCntSet,
    &DX_OUT_CPSS_DXCH_L2MLL_COUNTER_SET_INTERFACE_CFG_STC_PTR_interfaceCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChL2MllPairRead_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_mllPairEntryIndex,
    &DX_IN_CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT_mllPairReadForm,
    &DX_OUT_CPSS_DXCH_L2_MLL_PAIR_STC_PTR_mllPairEntryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChL2MllPairWrite_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_mllPairEntryIndex,
    &DX_IN_CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT_mllPairWriteForm,
    &DX_IN_CPSS_DXCH_L2_MLL_PAIR_STC_PTR_mllPairEntryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChL2MllMultiTargetPortBaseSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_portBase
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChL2MllMultiTargetPortSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_value,
    &DX_IN_GT_U32_HEX_mask
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChL2MllPortToVidxBaseSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_vidxBase
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChL2MLLVirtualPortToMllMappingTableEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U8_targetDevNum,
    &DX_IN_GT_U8_targetPortNum,
    &DX_IN_GT_U32_mllPointer
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChL2MLLVirtualPortToMllMappingTableEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U8_targetDevNum,
    &DX_IN_GT_U8_targetPortNum,
    &DX_OUT_GT_U32_PTR_mllPointer
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChL2MllExceptionCountersGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_L2_MLL_EXCEPTION_COUNTERS_STC_PTR_countersPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChL2MllMultiTargetPortRangeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_L2_MLL_MULTI_TARGET_PORT_RANGE_STC_PTR_portRangeConfigPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChL2MllTtlExceptionConfigurationGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_BOOL_PTR_trapEnablePtr,
    &DX_OUT_CPSS_NET_RX_CPU_CODE_ENT_PTR_cpuCodePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChL2MllLookupMaxVidxIndexGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_maxVidxIndexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChL2MllMultiTargetPortBaseGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_portBasePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChL2MllMultiTargetPortGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_valuePtr,
    &DX_OUT_GT_U32_HEX_PTR_maskPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChL2MllPortToVidxBaseGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_vidxBasePtr
};


/********* lib API DB *********/

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChL2mllLogLibDb[] = {
    {"cpssDxChL2MllLookupForAllEvidxEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChL2MllLookupForAllEvidxEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChL2MllLookupMaxVidxIndexSet", 2, cpssDxChL2MllLookupMaxVidxIndexSet_PARAMS, NULL},
    {"cpssDxChL2MllLookupMaxVidxIndexGet", 2, cpssDxChL2MllLookupMaxVidxIndexGet_PARAMS, NULL},
    {"cpssDxChL2MllTtlExceptionConfigurationSet", 3, cpssDxChL2MllTtlExceptionConfigurationSet_PARAMS, NULL},
    {"cpssDxChL2MllTtlExceptionConfigurationGet", 3, cpssDxChL2MllTtlExceptionConfigurationGet_PARAMS, NULL},
    {"cpssDxChL2MllExceptionCountersGet", 2, cpssDxChL2MllExceptionCountersGet_PARAMS, NULL},
    {"cpssDxChL2MllPortGroupExceptionCountersGet", 3, cpssDxChL2MllPortGroupExceptionCountersGet_PARAMS, NULL},
    {"cpssDxChL2MllCounterGet", 3, cpssDxChL2MllCounterGet_PARAMS, NULL},
    {"cpssDxChL2MllPortGroupCounterGet", 4, cpssDxChL2MllPortGroupCounterGet_PARAMS, NULL},
    {"cpssDxChL2MllSetCntInterfaceCfg", 3, cpssDxChL2MllSetCntInterfaceCfg_PARAMS, NULL},
    {"cpssDxChL2MllGetCntInterfaceCfg", 3, cpssDxChL2MllGetCntInterfaceCfg_PARAMS, NULL},
    {"cpssDxChL2MllPortGroupMcCntGet", 4, prvCpssLogGenDevNumPortGroupsBmpMllCntSetMllOutMCPktsPtr_PARAMS, NULL},
    {"cpssDxChL2MllMcCntGet", 3, prvCpssLogGenDevNumMllCntSetMllOutMCPktsPtr_PARAMS, NULL},
    {"cpssDxChL2MllSilentDropCntGet", 2, prvCpssLogGenDevNumSilentDropPktsPtr_PARAMS, NULL},
    {"cpssDxChL2MllPortGroupSilentDropCntGet", 3, prvCpssLogGenDevNumPortGroupsBmpSilentDropPktsPtr_PARAMS, NULL},
    {"cpssDxChL2MllLttEntrySet", 3, cpssDxChL2MllLttEntrySet_PARAMS, NULL},
    {"cpssDxChL2MllLttEntryGet", 3, cpssDxChL2MllLttEntryGet_PARAMS, NULL},
    {"cpssDxChL2MllPairWrite", 4, cpssDxChL2MllPairWrite_PARAMS, NULL},
    {"cpssDxChL2MllPairRead", 4, cpssDxChL2MllPairRead_PARAMS, NULL},
    {"cpssDxChL2MllMultiTargetPortEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChL2MllMultiTargetPortEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChL2MllMultiTargetPortRangeSet", 2, cpssDxChL2MllMultiTargetPortRangeSet_PARAMS, NULL},
    {"cpssDxChL2MllMultiTargetPortRangeGet", 2, cpssDxChL2MllMultiTargetPortRangeGet_PARAMS, NULL},
    {"cpssDxChL2MllMultiTargetPortSet", 3, cpssDxChL2MllMultiTargetPortSet_PARAMS, NULL},
    {"cpssDxChL2MllMultiTargetPortGet", 3, cpssDxChL2MllMultiTargetPortGet_PARAMS, NULL},
    {"cpssDxChL2MllMultiTargetPortBaseSet", 2, cpssDxChL2MllMultiTargetPortBaseSet_PARAMS, NULL},
    {"cpssDxChL2MllMultiTargetPortBaseGet", 2, cpssDxChL2MllMultiTargetPortBaseGet_PARAMS, NULL},
    {"cpssDxChL2MllPortToVidxBaseSet", 2, cpssDxChL2MllPortToVidxBaseSet_PARAMS, NULL},
    {"cpssDxChL2MllPortToVidxBaseGet", 2, cpssDxChL2MllPortToVidxBaseGet_PARAMS, NULL},
    {"cpssDxChL2MllSourceBasedFilteringConfigurationSet", 4, prvCpssLogGenDevNumEnableMeshIdOffsetMeshIdSize_PARAMS, NULL},
    {"cpssDxChL2MllSourceBasedFilteringConfigurationGet", 4, prvCpssLogGenDevNumEnablePtrMeshIdOffsetPtrMeshIdSizePtr_PARAMS, NULL},
    {"cpssDxChL2MLLVirtualPortToMllMappingTableEntrySet", 4, cpssDxChL2MLLVirtualPortToMllMappingTableEntrySet_PARAMS, NULL},
    {"cpssDxChL2MLLVirtualPortToMllMappingTableEntryGet", 4, cpssDxChL2MLLVirtualPortToMllMappingTableEntryGet_PARAMS, NULL},
    {"cpssDxChL2MllVidxEnableSet", 3, cpssDxChL2MllVidxEnableSet_PARAMS, NULL},
    {"cpssDxChL2MllVidxEnableGet", 3, cpssDxChL2MllVidxEnableGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_L2_MLL(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChL2mllLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChL2mllLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

