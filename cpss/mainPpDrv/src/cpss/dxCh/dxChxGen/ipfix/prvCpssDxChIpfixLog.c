/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChIpfixLog.c
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
#include <cpss/dxCh/dxChxGen/ipfix/cpssDxChIpfix.h>
#include <cpss/dxCh/dxChxGen/ipfix/private/prvCpssDxChIpfixLog.h>
#include <cpss/generic/log/prvCpssGenLog.h>
#include <cpss/generic/networkIf/private/prvCpssGenNetworkIfLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT[]  =
{
    "CPSS_DXCH_IPFIX_DROP_COUNT_MODE_METER_ONLY_E",
    "CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ALL_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_IPFIX_RESET_MODE_ENT[]  =
{
    "CPSS_DXCH_IPFIX_RESET_MODE_DISABLED_E",
    "CPSS_DXCH_IPFIX_RESET_MODE_LEGACY_E",
    "CPSS_DXCH_IPFIX_RESET_MODE_IPFIX_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IPFIX_RESET_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_IPFIX_PHA_METADATA_MODE_ENT[]  =
{
    "CPSS_DXCH_IPFIX_PHA_METADATA_MODE_DISABLE_E",
    "CPSS_DXCH_IPFIX_PHA_METADATA_MODE_COUNTER_E",
    "CPSS_DXCH_IPFIX_PHA_METADATA_MODE_RANDOM_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IPFIX_PHA_METADATA_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_IPFIX_SAMPLING_ACTION_ENT[]  =
{
    "CPSS_DXCH_IPFIX_SAMPLING_ACTION_ALARM_E",
    "CPSS_DXCH_IPFIX_SAMPLING_ACTION_MIRROR_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IPFIX_SAMPLING_ACTION_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_IPFIX_SAMPLING_DIST_ENT[]  =
{
    "CPSS_DXCH_IPFIX_SAMPLING_DIST_DETERMINISTIC_E",
    "CPSS_DXCH_IPFIX_SAMPLING_DIST_RANDOM_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IPFIX_SAMPLING_DIST_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_IPFIX_SAMPLING_MODE_ENT[]  =
{
    "CPSS_DXCH_IPFIX_SAMPLING_MODE_DISABLE_E",
    "CPSS_DXCH_IPFIX_SAMPLING_MODE_PACKET_E",
    "CPSS_DXCH_IPFIX_SAMPLING_MODE_BYTE_E",
    "CPSS_DXCH_IPFIX_SAMPLING_MODE_TIME_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IPFIX_SAMPLING_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_MODE_ENT[]  =
{
    "CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_MODE_ABSOLUTE_E",
    "CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_MODE_INCREMENTAL_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_IPFIX_WRAPAROUND_ACTION_ENT[]  =
{
    "CPSS_DXCH_IPFIX_WRAPAROUND_ACTION_NONE_E",
    "CPSS_DXCH_IPFIX_WRAPAROUND_ACTION_CLEAR_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IPFIX_WRAPAROUND_ACTION_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IPFIX_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, timeStamp);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, packetCount);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, byteCount, GT_U64);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dropCounter);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, randomOffset);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, lastSampledValue, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, samplingWindow, GT_U64);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, samplingAction, CPSS_DXCH_IPFIX_SAMPLING_ACTION_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, logSamplingRange);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, randomFlag, CPSS_DXCH_IPFIX_SAMPLING_DIST_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, samplingMode, CPSS_DXCH_IPFIX_SAMPLING_MODE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, cpuSubCode);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, firstPacketsCounter);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numberOfFirstPacketsToMirror);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, phaMetadataMode, CPSS_DXCH_IPFIX_PHA_METADATA_MODE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, firstTimestamp);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, firstTimestampValid);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, lastPacketCommand, CPSS_PACKET_CMD_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, lastCpuOrDropCode, CPSS_NET_RX_CPU_CODE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, packetCmd, CPSS_PACKET_CMD_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, cpuCode, CPSS_NET_RX_CPU_CODE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_TIMER_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IPFIX_TIMER_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, nanoSecondTimer);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, secondTimer, GT_U64);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, uploadMode, CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_MODE_ENT);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, timer, CPSS_DXCH_IPFIX_TIMER_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, action, CPSS_DXCH_IPFIX_WRAPAROUND_ACTION_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dropThreshold);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, packetThreshold);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, byteThreshold, GT_U64);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_IPFIX_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_RESET_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_IPFIX_RESET_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_IPFIX_RESET_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_TIMER_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_IPFIX_TIMER_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_TIMER_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT_mode = {
     "mode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPFIX_ENTRY_STC_PTR_ipfixEntryPtr = {
     "ipfixEntryPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPFIX_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC_PTR_firstPacketCfgPtr = {
     "firstPacketCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPFIX_RESET_MODE_ENT_reset = {
     "reset", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_IPFIX_RESET_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC_PTR_uploadPtr = {
     "uploadPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC_PTR_confPtr = {
     "confPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_endIndex = {
     "endIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ipfixIndex = {
     "ipfixIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U64_phaCounterMask = {
     "phaCounterMask", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U64)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT_PTR_modePtr = {
     "modePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IPFIX_ENTRY_STC_PTR_ipfixEntryPtr = {
     "ipfixEntryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPFIX_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC_PTR_firstPacketCfgPtr = {
     "firstPacketCfgPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IPFIX_TIMER_STC_PTR_timerPtr = {
     "timerPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPFIX_TIMER_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC_PTR_uploadPtr = {
     "uploadPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC_PTR_confPtr = {
     "confPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_uploadStatusPtr = {
     "uploadStatusPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_bmpPtr = {
     "bmpPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_eventsArr = {
     "eventsArr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_eventsNumPtr = {
     "eventsNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U64_PTR_phaCounterMaskPtr = {
     "phaCounterMaskPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U64)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixDropCountModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_IN_CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT_mode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixFirstPacketConfigSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_IN_CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC_PTR_firstPacketCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixTimestampUploadSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_IN_CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC_PTR_uploadPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixWraparoundConfSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_IN_CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC_PTR_confPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixCpuCodeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_IN_CPSS_NET_RX_CPU_CODE_ENT_cpuCode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixBaseFlowIdSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_IN_GT_U32_baseFlowId
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_IN_GT_U32_entryIndex,
    &DX_IN_CPSS_DXCH_IPFIX_ENTRY_STC_PTR_ipfixEntryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_IN_GT_U32_entryIndex,
    &DX_IN_CPSS_DXCH_IPFIX_RESET_MODE_ENT_reset,
    &DX_OUT_CPSS_DXCH_IPFIX_ENTRY_STC_PTR_ipfixEntryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixFirstPacketsMirrorEnableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_IN_GT_U32_ipfixIndex,
    &DX_IN_GT_BOOL_enable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixFirstPacketsMirrorEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_IN_GT_U32_ipfixIndex,
    &DX_OUT_GT_BOOL_PTR_enablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixWraparoundStatusGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_IN_GT_U32_startIndex,
    &DX_IN_GT_U32_endIndex,
    &DX_IN_GT_BOOL_reset,
    &DX_OUT_GT_U32_PTR_bmpPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixPhaMetadataCounterMaskSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_IN_GT_U64_phaCounterMask
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixDropCountModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_OUT_CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT_PTR_modePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixFirstPacketConfigGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_OUT_CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC_PTR_firstPacketCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixTimerGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_OUT_CPSS_DXCH_IPFIX_TIMER_STC_PTR_timerPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixTimestampUploadGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_OUT_CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC_PTR_uploadPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixWraparoundConfGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_OUT_CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC_PTR_confPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixCpuCodeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_OUT_CPSS_NET_RX_CPU_CODE_ENT_PTR_cpuCodePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixBaseFlowIdGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_OUT_GT_U32_PTR_baseFlowIdPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixPhaMetadataCounterMaskGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_OUT_GT_U64_PTR_phaCounterMaskPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixPortGroupEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_IN_GT_U32_entryIndex,
    &DX_IN_CPSS_DXCH_IPFIX_ENTRY_STC_PTR_ipfixEntryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixPortGroupEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_IN_GT_U32_entryIndex,
    &DX_IN_CPSS_DXCH_IPFIX_RESET_MODE_ENT_reset,
    &DX_OUT_CPSS_DXCH_IPFIX_ENTRY_STC_PTR_ipfixEntryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixPortGroupWraparoundStatusGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_IN_GT_U32_startIndex,
    &DX_IN_GT_U32_endIndex,
    &DX_IN_GT_BOOL_reset,
    &DX_OUT_GT_U32_PTR_bmpPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixTimestampUploadStatusGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_BOOL_PTR_uploadStatusPtr
};
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixAlarmEventsGet_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixPortGroupAlarmEventsGet_PARAMS[];


/********* lib API DB *********/

extern void cpssDxChIpfixAlarmEventsGet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChIpfixPortGroupAlarmEventsGet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChIpfixLogLibDb[] = {
    {"cpssDxChIpfixEntrySet", 4, cpssDxChIpfixEntrySet_PARAMS, NULL},
    {"cpssDxChIpfixEntryGet", 5, cpssDxChIpfixEntryGet_PARAMS, NULL},
    {"cpssDxChIpfixDropCountModeSet", 3, cpssDxChIpfixDropCountModeSet_PARAMS, NULL},
    {"cpssDxChIpfixDropCountModeGet", 3, cpssDxChIpfixDropCountModeGet_PARAMS, NULL},
    {"cpssDxChIpfixCpuCodeSet", 3, cpssDxChIpfixCpuCodeSet_PARAMS, NULL},
    {"cpssDxChIpfixCpuCodeGet", 3, cpssDxChIpfixCpuCodeGet_PARAMS, NULL},
    {"cpssDxChIpfixAgingEnableSet", 3, prvCpssLogGenDevNumStageEnable_PARAMS, NULL},
    {"cpssDxChIpfixAgingEnableGet", 3, prvCpssLogGenDevNumStageEnablePtr_PARAMS, NULL},
    {"cpssDxChIpfixWraparoundConfSet", 3, cpssDxChIpfixWraparoundConfSet_PARAMS, NULL},
    {"cpssDxChIpfixWraparoundConfGet", 3, cpssDxChIpfixWraparoundConfGet_PARAMS, NULL},
    {"cpssDxChIpfixWraparoundStatusGet", 6, cpssDxChIpfixWraparoundStatusGet_PARAMS, NULL},
    {"cpssDxChIpfixAgingStatusGet", 6, cpssDxChIpfixWraparoundStatusGet_PARAMS, NULL},
    {"cpssDxChIpfixTimestampUploadSet", 3, cpssDxChIpfixTimestampUploadSet_PARAMS, NULL},
    {"cpssDxChIpfixTimestampUploadGet", 3, cpssDxChIpfixTimestampUploadGet_PARAMS, NULL},
    {"cpssDxChIpfixTimestampUploadTrigger", 1, prvCpssLogGenDevNum_PARAMS, NULL},
    {"cpssDxChIpfixTimestampUploadStatusGet", 2, cpssDxChIpfixTimestampUploadStatusGet_PARAMS, NULL},
    {"cpssDxChIpfixTimerGet", 3, cpssDxChIpfixTimerGet_PARAMS, NULL},
    {"cpssDxChIpfixAlarmEventsGet", 4, cpssDxChIpfixAlarmEventsGet_PARAMS, cpssDxChIpfixAlarmEventsGet_preLogic},
    {"cpssDxChIpfixPortGroupEntrySet", 5, cpssDxChIpfixPortGroupEntrySet_PARAMS, NULL},
    {"cpssDxChIpfixPortGroupEntryGet", 6, cpssDxChIpfixPortGroupEntryGet_PARAMS, NULL},
    {"cpssDxChIpfixPortGroupWraparoundStatusGet", 7, cpssDxChIpfixPortGroupWraparoundStatusGet_PARAMS, NULL},
    {"cpssDxChIpfixPortGroupAgingStatusGet", 7, cpssDxChIpfixPortGroupWraparoundStatusGet_PARAMS, NULL},
    {"cpssDxChIpfixPortGroupAlarmEventsGet", 5, cpssDxChIpfixPortGroupAlarmEventsGet_PARAMS, cpssDxChIpfixPortGroupAlarmEventsGet_preLogic},
    {"cpssDxChIpfixTimestampToCpuEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChIpfixTimestampToCpuEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChIpfixBaseFlowIdSet", 3, cpssDxChIpfixBaseFlowIdSet_PARAMS, NULL},
    {"cpssDxChIpfixBaseFlowIdGet", 3, cpssDxChIpfixBaseFlowIdGet_PARAMS, NULL},
    {"cpssDxChIpfixFirstPacketConfigSet", 3, cpssDxChIpfixFirstPacketConfigSet_PARAMS, NULL},
    {"cpssDxChIpfixFirstPacketConfigGet", 3, cpssDxChIpfixFirstPacketConfigGet_PARAMS, NULL},
    {"cpssDxChIpfixPhaMetadataCounterMaskSet", 3, cpssDxChIpfixPhaMetadataCounterMaskSet_PARAMS, NULL},
    {"cpssDxChIpfixPhaMetadataCounterMaskGet", 3, cpssDxChIpfixPhaMetadataCounterMaskGet_PARAMS, NULL},
    {"cpssDxChIpfixFirstPacketsMirrorEnableSet", 4, cpssDxChIpfixFirstPacketsMirrorEnableSet_PARAMS, NULL},
    {"cpssDxChIpfixFirstPacketsMirrorEnableGet", 4, cpssDxChIpfixFirstPacketsMirrorEnableGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_IPFIX(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChIpfixLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChIpfixLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

