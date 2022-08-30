/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssPxCncLog.c
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
#include <cpss/px/cnc/cpssPxCnc.h>
#include <cpss/px/cnc/private/prvCpssPxCncLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_PX_CNC_BYTE_COUNT_MODE_ENT[]  =
{
    "CPSS_PX_CNC_BYTE_COUNT_MODE_L2_E",
    "CPSS_PX_CNC_BYTE_COUNT_MODE_L3_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_CNC_BYTE_COUNT_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_PX_CNC_CLIENT_ENT[]  =
{
    "CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E",
    "CPSS_PX_CNC_CLIENT_EGRESS_PROCESSING_E",
    "CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_CNC_CLIENT_ENT);
const char * const prvCpssLogEnum_CPSS_PX_CNC_COUNTER_FORMAT_ENT[]  =
{
    "CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E",
    "CPSS_PX_CNC_COUNTER_FORMAT_MODE_1_E",
    "CPSS_PX_CNC_COUNTER_FORMAT_MODE_2_E",
    "CPSS_PX_CNC_COUNTER_FORMAT_MODE_3_E",
    "CPSS_PX_CNC_COUNTER_FORMAT_MODE_4_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_CNC_COUNTER_FORMAT_ENT);
const char * const prvCpssLogEnum_CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT[]  =
{
    "CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_E",
    "CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_CN_E",
    "CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_REDUCED_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_PX_CNC_COUNTER_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_CNC_COUNTER_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, byteCount, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, packetCount, GT_U64);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_PX_CNC_BYTE_COUNT_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_CNC_BYTE_COUNT_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_CNC_BYTE_COUNT_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_CNC_BYTE_COUNT_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_CNC_BYTE_COUNT_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_CNC_BYTE_COUNT_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_CNC_CLIENT_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_CNC_CLIENT_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_CNC_CLIENT_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_CNC_COUNTER_FORMAT_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_CNC_COUNTER_FORMAT_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_CNC_COUNTER_FORMAT_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_CNC_COUNTER_FORMAT_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_CNC_COUNTER_FORMAT_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_CNC_COUNTER_FORMAT_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_CNC_COUNTER_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_CNC_COUNTER_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_CNC_COUNTER_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC PX_INOUT_GT_U32_PTR_indexNumPtr = {
     "indexNumPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_INOUT_GT_U32_PTR_numOfCounterValuesPtr = {
     "numOfCounterValuesPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_DMA_QUEUE_CFG_STC_PTR_cncCfgPtr = {
     "cncCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DMA_QUEUE_CFG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_CNC_BYTE_COUNT_MODE_ENT_mode = {
     "mode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_CNC_BYTE_COUNT_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_CNC_CLIENT_ENT_client = {
     "client", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_CNC_CLIENT_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_CNC_COUNTER_FORMAT_ENT_format = {
     "format", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_CNC_COUNTER_FORMAT_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_CNC_COUNTER_STC_PTR_counterPtr = {
     "counterPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_CNC_COUNTER_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT_mode = {
     "mode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_updateEnable = {
     "updateEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_blockNum = {
     "blockNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U64_HEX_indexRangesBmp = {
     "indexRangesBmp", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U64_HEX)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_CNC_BYTE_COUNT_MODE_ENT_PTR_modePtr = {
     "modePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_CNC_BYTE_COUNT_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_CNC_COUNTER_FORMAT_ENT_PTR_formatPtr = {
     "formatPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_CNC_COUNTER_FORMAT_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_CNC_COUNTER_STC_PTR_counterPtr = {
     "counterPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_CNC_COUNTER_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_CNC_COUNTER_STC_PTR_counterValuesPtr = {
     "counterValuesPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_CNC_COUNTER_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT_PTR_modePtr = {
     "modePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_updateEnablePtr = {
     "updateEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_inProcessBlocksBmpPtr = {
     "inProcessBlocksBmpPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_indexesArr = {
     "indexesArr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U64_HEX_PTR_indexRangesBmpPtr = {
     "indexRangesBmpPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U64_HEX)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCncUploadInit_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_DMA_QUEUE_CFG_STC_PTR_cncCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCncClientByteCountModeSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_CNC_CLIENT_ENT_client,
    &PX_IN_CPSS_PX_CNC_BYTE_COUNT_MODE_ENT_mode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCncClientByteCountModeGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_CNC_CLIENT_ENT_client,
    &PX_OUT_CPSS_PX_CNC_BYTE_COUNT_MODE_ENT_PTR_modePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCncCounterClearByReadValueSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_CNC_COUNTER_FORMAT_ENT_format,
    &PX_IN_CPSS_PX_CNC_COUNTER_STC_PTR_counterPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCncCounterClearByReadValueGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_CNC_COUNTER_FORMAT_ENT_format,
    &PX_OUT_CPSS_PX_CNC_COUNTER_STC_PTR_counterPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCncEgressQueueClientModeSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT_mode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCncBlockUploadTrigger_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_blockNum
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCncBlockClientEnableSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_blockNum,
    &PX_IN_CPSS_PX_CNC_CLIENT_ENT_client,
    &PX_IN_GT_BOOL_updateEnable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCncBlockClientEnableGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_blockNum,
    &PX_IN_CPSS_PX_CNC_CLIENT_ENT_client,
    &PX_OUT_GT_BOOL_PTR_updateEnablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCncCounterFormatSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_blockNum,
    &PX_IN_CPSS_PX_CNC_COUNTER_FORMAT_ENT_format
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCncCounterGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_blockNum,
    &PX_IN_GT_U32_index,
    &PX_IN_CPSS_PX_CNC_COUNTER_FORMAT_ENT_format,
    &PX_OUT_CPSS_PX_CNC_COUNTER_STC_PTR_counterPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCncBlockClientRangesSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_blockNum,
    &PX_IN_GT_U64_HEX_indexRangesBmp
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCncCounterFormatGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_blockNum,
    &PX_OUT_CPSS_PX_CNC_COUNTER_FORMAT_ENT_PTR_formatPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCncBlockClientRangesGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_blockNum,
    &PX_OUT_GT_U64_HEX_PTR_indexRangesBmpPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCncEgressQueueClientModeGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT_PTR_modePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCncBlockUploadInProcessGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_GT_U32_PTR_inProcessBlocksBmpPtr
};
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCncCounterWraparoundIndexesGet_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCncUploadedBlockGet_PARAMS[];


/********* lib API DB *********/

extern void cpssPxCncUploadedBlockGet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssPxCncCounterWraparoundIndexesGet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssPxCncLogLibDb[] = {
    {"cpssPxCncUploadInit", 2, cpssPxCncUploadInit_PARAMS, NULL},
    {"cpssPxCncBlockClientEnableSet", 4, cpssPxCncBlockClientEnableSet_PARAMS, NULL},
    {"cpssPxCncBlockClientEnableGet", 4, cpssPxCncBlockClientEnableGet_PARAMS, NULL},
    {"cpssPxCncBlockClientRangesSet", 3, cpssPxCncBlockClientRangesSet_PARAMS, NULL},
    {"cpssPxCncBlockClientRangesGet", 3, cpssPxCncBlockClientRangesGet_PARAMS, NULL},
    {"cpssPxCncCounterFormatSet", 3, cpssPxCncCounterFormatSet_PARAMS, NULL},
    {"cpssPxCncCounterFormatGet", 3, cpssPxCncCounterFormatGet_PARAMS, NULL},
    {"cpssPxCncCounterClearByReadEnableSet", 2, prvCpssLogGenDevNumEnable2_PARAMS, NULL},
    {"cpssPxCncCounterClearByReadEnableGet", 2, prvCpssLogGenDevNumEnablePtr2_PARAMS, NULL},
    {"cpssPxCncCounterClearByReadValueSet", 3, cpssPxCncCounterClearByReadValueSet_PARAMS, NULL},
    {"cpssPxCncCounterClearByReadValueGet", 3, cpssPxCncCounterClearByReadValueGet_PARAMS, NULL},
    {"cpssPxCncBlockUploadTrigger", 2, cpssPxCncBlockUploadTrigger_PARAMS, NULL},
    {"cpssPxCncBlockUploadInProcessGet", 2, cpssPxCncBlockUploadInProcessGet_PARAMS, NULL},
    {"cpssPxCncUploadedBlockGet", 4, cpssPxCncUploadedBlockGet_PARAMS, cpssPxCncUploadedBlockGet_preLogic},
    {"cpssPxCncCounterWraparoundEnableSet", 2, prvCpssLogGenDevNumEnable2_PARAMS, NULL},
    {"cpssPxCncCounterWraparoundEnableGet", 2, prvCpssLogGenDevNumEnablePtr2_PARAMS, NULL},
    {"cpssPxCncCounterWraparoundIndexesGet", 4, cpssPxCncCounterWraparoundIndexesGet_PARAMS, cpssPxCncCounterWraparoundIndexesGet_preLogic},
    {"cpssPxCncCounterGet", 5, cpssPxCncCounterGet_PARAMS, NULL},
    {"cpssPxCncEgressQueueClientModeSet", 2, cpssPxCncEgressQueueClientModeSet_PARAMS, NULL},
    {"cpssPxCncEgressQueueClientModeGet", 2, cpssPxCncEgressQueueClientModeGet_PARAMS, NULL},
    {"cpssPxCncClientByteCountModeSet", 3, cpssPxCncClientByteCountModeSet_PARAMS, NULL},
    {"cpssPxCncClientByteCountModeGet", 3, cpssPxCncClientByteCountModeGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_PX_CPSS_LOG_LIB_CNC(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssPxCncLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssPxCncLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

