/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChStreamLog.c
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
#include <cpss/dxCh/dxChxGen/cnc/private/prvCpssDxChCncLog.h>
#include <cpss/dxCh/dxChxGen/stream/cpssDxChStream.h>
#include <cpss/dxCh/dxChxGen/stream/private/prvCpssDxChStreamLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssGenDbLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT[]  =
{
    "CPSS_DXCH_STREAM_EXCEPTION_TYPE_IRF_SAME_SEQ_ID_E",
    "CPSS_DXCH_STREAM_EXCEPTION_TYPE_SRF_TAGLESS_E",
    "CPSS_DXCH_STREAM_EXCEPTION_TYPE_SRF_ROGUE_E",
    "CPSS_DXCH_STREAM_EXCEPTION_TYPE_SRF_DUPLICATED_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_STREAM_IRF_COUNTERS_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_STREAM_IRF_COUNTERS_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, passedPacketCounter, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, discardedPacketsOrSameIdCounter, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, outOfOrderPacketsCounter, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, taglessPacketsCounter, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, byteCounter, GT_U64);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_STREAM_SNG_IRF_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_STREAM_SNG_IRF_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, seqNumAssignmentEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, resetSeqNum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, currentSeqNum);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, individualRecoveryEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, takeAnySeqNum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, recoverySeqNum);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_STREAM_SRF_CONFIG_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_STREAM_SRF_CONFIG_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, useVectorRecoveryAlgorithm);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vectorSequenceHistoryLength);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, takeNoSeqNum);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, takeAnySeqNum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, firstHistoryBuffer);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numOfBuffers);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, restartThreshold);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, leEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, leResetTimeEnabled);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, leResetTimeInSeconds);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_STREAM_SRF_COUNTERS_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_STREAM_SRF_COUNTERS_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, passedPacketsCounter, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, discardedPacketsCounter, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, outOfOrderPacketsCounter, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, clearedZeroHistoryBitsCounter, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, roguePacketCounter, GT_U64);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_STREAM_SRF_DAEMON_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_STREAM_SRF_DAEMON_INFO_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, restartDaemonEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, latentErrorDaemonEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, restartPeriodTimeInMicroSec);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, latentErrorDetectionPeriodTimeInMicroSec);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, latentErrorDetectionAlgorithmEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, latentErrorDetectionResetEnable);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_STREAM_SRF_HISTORY_BUFFER_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_STREAM_SRF_HISTORY_BUFFER_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, historyBuffer, 4, GT_U32);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_STREAM_SRF_MAPPING_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_STREAM_SRF_MAPPING_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, srfEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, isMulticast);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, srfIndex);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, egressPortsBmp, CPSS_PORTS_BMP_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_STREAM_SRF_STATUS_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_STREAM_SRF_STATUS_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, recoverySeqNum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, currHistoryBuffer);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, currHistoryBufferBit);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, untillRestartTimeInMicroSec);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numberOfResets);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, leBaseDifference);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, leTimeInSecSinceLastReset);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_STREAM_IRF_COUNTERS_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_STREAM_IRF_COUNTERS_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_STREAM_IRF_COUNTERS_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_STREAM_SNG_IRF_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_STREAM_SNG_IRF_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_STREAM_SNG_IRF_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_STREAM_SRF_CONFIG_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_STREAM_SRF_CONFIG_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_STREAM_SRF_CONFIG_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_STREAM_SRF_COUNTERS_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_STREAM_SRF_COUNTERS_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_STREAM_SRF_COUNTERS_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_STREAM_SRF_DAEMON_INFO_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_STREAM_SRF_DAEMON_INFO_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_STREAM_SRF_DAEMON_INFO_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_STREAM_SRF_HISTORY_BUFFER_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_STREAM_SRF_HISTORY_BUFFER_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_STREAM_SRF_HISTORY_BUFFER_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_STREAM_SRF_MAPPING_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_STREAM_SRF_MAPPING_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_STREAM_SRF_MAPPING_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_STREAM_SRF_STATUS_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_STREAM_SRF_STATUS_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_STREAM_SRF_STATUS_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT_irfMode = {
     "irfMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT_type = {
     "type", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_STREAM_SNG_IRF_ENTRY_STC_PTR_sngIrfInfoPtr = {
     "sngIrfInfoPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_STREAM_SNG_IRF_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_STREAM_SRF_CONFIG_ENTRY_STC_PTR_srfConfigInfoPtr = {
     "srfConfigInfoPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_STREAM_SRF_CONFIG_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_STREAM_SRF_DAEMON_INFO_STC_PTR_infoPtr = {
     "infoPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_STREAM_SRF_DAEMON_INFO_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_STREAM_SRF_HISTORY_BUFFER_ENTRY_STC_PTR_histBuffPtr = {
     "histBuffPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_STREAM_SRF_HISTORY_BUFFER_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_STREAM_SRF_MAPPING_ENTRY_STC_PTR_srfMappingInfoPtr = {
     "srfMappingInfoPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_STREAM_SRF_MAPPING_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_srfTaglessEnable = {
     "srfTaglessEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_agingGroupBmp = {
     "agingGroupBmp", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_vector = {
     "vector", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT_PTR_irfModePtr = {
     "irfModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_STREAM_IRF_COUNTERS_ENTRY_STC_PTR_irfCountersPtr = {
     "irfCountersPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_STREAM_IRF_COUNTERS_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_STREAM_SNG_IRF_ENTRY_STC_PTR_sngIrfInfoPtr = {
     "sngIrfInfoPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_STREAM_SNG_IRF_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_STREAM_SRF_CONFIG_ENTRY_STC_PTR_srfConfigInfoPtr = {
     "srfConfigInfoPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_STREAM_SRF_CONFIG_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_STREAM_SRF_COUNTERS_ENTRY_STC_PTR_srfCountersPtr = {
     "srfCountersPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_STREAM_SRF_COUNTERS_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_STREAM_SRF_DAEMON_INFO_STC_PTR_infoPtr = {
     "infoPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_STREAM_SRF_DAEMON_INFO_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_STREAM_SRF_HISTORY_BUFFER_ENTRY_STC_PTR_histBuffPtr = {
     "histBuffPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_STREAM_SRF_HISTORY_BUFFER_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_STREAM_SRF_MAPPING_ENTRY_STC_PTR_srfMappingInfoPtr = {
     "srfMappingInfoPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_STREAM_SRF_MAPPING_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_STREAM_SRF_STATUS_ENTRY_STC_PTR_srfStatusInfoPtr = {
     "srfStatusInfoPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_STREAM_SRF_STATUS_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_srfTaglessEnablePtr = {
     "srfTaglessEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_agingGroupBmpPtr = {
     "agingGroupBmpPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_errorGroupBmpPtr = {
     "errorGroupBmpPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_vectorPtr = {
     "vectorPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChStreamCountingModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT_irfMode,
    &DX_IN_GT_BOOL_srfTaglessEnable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChStreamExceptionCpuCodeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT_type,
    &DX_IN_CPSS_NET_RX_CPU_CODE_ENT_cpuCode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChStreamExceptionCommandSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT_type,
    &DX_IN_CPSS_PACKET_CMD_ENT_command
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChStreamExceptionCpuCodeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT_type,
    &DX_OUT_CPSS_NET_RX_CPU_CODE_ENT_PTR_cpuCodePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChStreamExceptionCommandGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_STREAM_EXCEPTION_TYPE_ENT_type,
    &DX_OUT_CPSS_PACKET_CMD_ENT_PTR_commandPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChStreamSrfDaemonInfoSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_STREAM_SRF_DAEMON_INFO_STC_PTR_infoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChStreamSrfTrgPortMappedMulticastIndexSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U32_index
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChStreamSrfTrgPortMappedMulticastIndexGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_U32_PTR_indexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChStreamEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_IN_CPSS_DXCH_STREAM_SNG_IRF_ENTRY_STC_PTR_sngIrfInfoPtr,
    &DX_IN_CPSS_DXCH_STREAM_SRF_MAPPING_ENTRY_STC_PTR_srfMappingInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChStreamSrfEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_IN_CPSS_DXCH_STREAM_SRF_CONFIG_ENTRY_STC_PTR_srfConfigInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChStreamSrfHistoryBufferEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_IN_CPSS_DXCH_STREAM_SRF_HISTORY_BUFFER_ENTRY_STC_PTR_histBuffPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChStreamEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_OUT_CPSS_DXCH_STREAM_SNG_IRF_ENTRY_STC_PTR_sngIrfInfoPtr,
    &DX_OUT_CPSS_DXCH_STREAM_IRF_COUNTERS_ENTRY_STC_PTR_irfCountersPtr,
    &DX_OUT_CPSS_DXCH_STREAM_SRF_MAPPING_ENTRY_STC_PTR_srfMappingInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChStreamSrfEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_OUT_CPSS_DXCH_STREAM_SRF_CONFIG_ENTRY_STC_PTR_srfConfigInfoPtr,
    &DX_OUT_CPSS_DXCH_STREAM_SRF_COUNTERS_ENTRY_STC_PTR_srfCountersPtr,
    &DX_OUT_CPSS_DXCH_STREAM_SRF_STATUS_ENTRY_STC_PTR_srfStatusInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChStreamSrfHistoryBufferEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_OUT_CPSS_DXCH_STREAM_SRF_HISTORY_BUFFER_ENTRY_STC_PTR_histBuffPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChStreamAgingGroupSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_startIndex,
    &DX_IN_GT_U32_agingGroupBmp
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChStreamSrfZeroBitVectorGroupEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_startIndex,
    &DX_IN_GT_U32_vector
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChStreamAgingGroupGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_startIndex,
    &DX_OUT_GT_U32_PTR_agingGroupBmpPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChStreamSrfLatentErrorDetectedGroupGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_startIndex,
    &DX_OUT_GT_U32_PTR_errorGroupBmpPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChStreamSrfZeroBitVectorGroupEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_startIndex,
    &DX_OUT_GT_U32_PTR_vectorPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChStreamCountingModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT_PTR_irfModePtr,
    &DX_OUT_GT_BOOL_PTR_srfTaglessEnablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChStreamSrfDaemonInfoGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_STREAM_SRF_DAEMON_INFO_STC_PTR_infoPtr
};


/********* lib API DB *********/

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChStreamLogLibDb[] = {
    {"cpssDxChStreamSngEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChStreamSngEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChStreamIrfEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChStreamIrfEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChStreamExceptionCommandSet", 3, cpssDxChStreamExceptionCommandSet_PARAMS, NULL},
    {"cpssDxChStreamExceptionCommandGet", 3, cpssDxChStreamExceptionCommandGet_PARAMS, NULL},
    {"cpssDxChStreamExceptionCpuCodeSet", 3, cpssDxChStreamExceptionCpuCodeSet_PARAMS, NULL},
    {"cpssDxChStreamExceptionCpuCodeGet", 3, cpssDxChStreamExceptionCpuCodeGet_PARAMS, NULL},
    {"cpssDxChStreamCountingModeSet", 3, cpssDxChStreamCountingModeSet_PARAMS, NULL},
    {"cpssDxChStreamCountingModeGet", 3, cpssDxChStreamCountingModeGet_PARAMS, NULL},
    {"cpssDxChStreamSrfDaemonInfoSet", 2, cpssDxChStreamSrfDaemonInfoSet_PARAMS, NULL},
    {"cpssDxChStreamSrfDaemonInfoGet", 2, cpssDxChStreamSrfDaemonInfoGet_PARAMS, NULL},
    {"cpssDxChStreamEntrySet", 4, cpssDxChStreamEntrySet_PARAMS, NULL},
    {"cpssDxChStreamEntryGet", 5, cpssDxChStreamEntryGet_PARAMS, NULL},
    {"cpssDxChStreamAgingGroupSet", 3, cpssDxChStreamAgingGroupSet_PARAMS, NULL},
    {"cpssDxChStreamAgingGroupGet", 3, cpssDxChStreamAgingGroupGet_PARAMS, NULL},
    {"cpssDxChStreamSrfEntrySet", 3, cpssDxChStreamSrfEntrySet_PARAMS, NULL},
    {"cpssDxChStreamSrfEntryGet", 5, cpssDxChStreamSrfEntryGet_PARAMS, NULL},
    {"cpssDxChStreamSrfLatentErrorDetectedGroupGet", 3, cpssDxChStreamSrfLatentErrorDetectedGroupGet_PARAMS, NULL},
    {"cpssDxChStreamSrfHistoryBufferEntrySet", 3, cpssDxChStreamSrfHistoryBufferEntrySet_PARAMS, NULL},
    {"cpssDxChStreamSrfHistoryBufferEntryGet", 3, cpssDxChStreamSrfHistoryBufferEntryGet_PARAMS, NULL},
    {"cpssDxChStreamSrfZeroBitVectorGroupEntrySet", 3, cpssDxChStreamSrfZeroBitVectorGroupEntrySet_PARAMS, NULL},
    {"cpssDxChStreamSrfZeroBitVectorGroupEntryGet", 3, cpssDxChStreamSrfZeroBitVectorGroupEntryGet_PARAMS, NULL},
    {"cpssDxChStreamSrfTrgPortMappedMulticastIndexSet", 3, cpssDxChStreamSrfTrgPortMappedMulticastIndexSet_PARAMS, NULL},
    {"cpssDxChStreamSrfTrgPortMappedMulticastIndexGet", 3, cpssDxChStreamSrfTrgPortMappedMulticastIndexGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_STREAM(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChStreamLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChStreamLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

