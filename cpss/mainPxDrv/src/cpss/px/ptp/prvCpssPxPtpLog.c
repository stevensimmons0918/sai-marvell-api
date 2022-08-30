/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssPxPtpLog.c
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
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssPxGenDbLog.h>
#include <cpss/px/ingress/private/prvCpssPxIngressLog.h>
#include <cpss/px/ptp/cpssPxPtp.h>
#include <cpss/px/ptp/private/prvCpssPxPtpLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_PX_PTP_TAI_CLOCK_FREQ_ENT[]  =
{
    "CPSS_PX_PTP_10_FREQ_E",
    "CPSS_PX_PTP_20_FREQ_E",
    "CPSS_PX_PTP_25_FREQ_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PTP_TAI_CLOCK_FREQ_ENT);
const char * const prvCpssLogEnum_CPSS_PX_PTP_TAI_CLOCK_MODE_ENT[]  =
{
    "CPSS_PX_PTP_TAI_CLOCK_MODE_DISABLED_E",
    "CPSS_PX_PTP_TAI_CLOCK_MODE_OUTPUT_E",
    "CPSS_PX_PTP_TAI_CLOCK_MODE_INPUT_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PTP_TAI_CLOCK_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT[]  =
{
    "CPSS_PX_PTP_TAI_CLOCK_SELECT_PTP_PLL_E",
    "CPSS_PX_PTP_TAI_CLOCK_SELECT_CORE_PLL_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT);
const char * const prvCpssLogEnum_CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT[]  =
{
    "CPSS_PX_PTP_TAI_PULSE_IN_IS_PTP_PULSE_IN_E",
    "CPSS_PX_PTP_TAI_PULSE_IN_IS_PTP_CLOCK_IN_E",
    "CPSS_PX_PTP_TAI_PULSE_IN_DISABLED_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT[]  =
{
    "CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_UPDATE_E",
    "CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_INCREMENT_E",
    "CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_CAPTURE_E",
    "CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_DECREMENT_E",
    "CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_GRACEFULLY_INCREMENT_E",
    "CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_GRACEFULLY_DECREMENT_E",
    "CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_FREQUENCE_UPDATE_E",
    "CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_NOP_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT);
const char * const prvCpssLogEnum_CPSS_PX_PTP_TAI_TOD_TYPE_ENT[]  =
{
    "CPSS_PX_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E",
    "CPSS_PX_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_MASK_E",
    "CPSS_PX_PTP_TAI_TOD_TYPE_LOAD_VALUE_E",
    "CPSS_PX_PTP_TAI_TOD_TYPE_CAPTURE_VALUE0_E",
    "CPSS_PX_PTP_TAI_TOD_TYPE_CAPTURE_VALUE1_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PTP_TAI_TOD_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_ENT[]  =
{
    "CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_TOTAL_E",
    "CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_PTP_V1_E",
    "CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_PTP_V2_E",
    "CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_ADD_CORR_TIME_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_PX_PTP_TAI_TOD_COUNT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_PTP_TAI_TOD_COUNT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, nanoSeconds);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, seconds, GT_U64);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, fracNanoSeconds);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_PTP_TAI_TOD_STEP_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_PTP_TAI_TOD_STEP_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, nanoSeconds);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, fracNanoSeconds);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_PTP_TSU_CONTROL_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_PTP_TSU_CONTROL_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, unitEnable);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_PTP_TYPE_KEY_FORMAT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_PTP_TYPE_KEY_FORMAT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, isUdp);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, srcPortProfile);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, udbPairsArr, CPSS_PX_PTP_TYPE_UDBP_NUM_CNS, CPSS_PX_UDB_PAIR_DATA_STC);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, udbArr, CPSS_PX_PTP_TYPE_UDB_NUM_CNS, GT_U8);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_PTP_TYPE_KEY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_PTP_TYPE_KEY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, portUdbPairArr, CPSS_PX_PTP_TYPE_UDBP_NUM_CNS, CPSS_PX_UDB_PAIR_KEY_STC);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, portUdbArr, CPSS_PX_PTP_TYPE_UDB_NUM_CNS, CPSS_PX_UDB_KEY_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_PX_PTP_TAI_CLOCK_FREQ_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PTP_TAI_CLOCK_FREQ_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_PTP_TAI_CLOCK_FREQ_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PTP_TAI_CLOCK_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PTP_TAI_CLOCK_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_PTP_TAI_CLOCK_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PTP_TAI_CLOCK_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PTP_TAI_CLOCK_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_PTP_TAI_CLOCK_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PTP_TAI_TOD_COUNT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_PTP_TAI_TOD_COUNT_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_PTP_TAI_TOD_COUNT_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_PTP_TAI_TOD_STEP_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_PTP_TAI_TOD_STEP_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_PTP_TAI_TOD_STEP_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_PTP_TAI_TOD_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PTP_TAI_TOD_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_PTP_TAI_TOD_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PTP_TSU_CONTROL_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_PTP_TSU_CONTROL_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_PTP_TSU_CONTROL_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PTP_TYPE_INDEX(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    char * formatPtr = "%s = %d\n";
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PTP_TYPE_INDEX, paramVal);
    if (inOutParamInfoPtr->formatPtr)
    {
        formatPtr = inOutParamInfoPtr->formatPtr;
    }
    PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, formatPtr, namePtr, paramVal);
}
void prvCpssLogParamFunc_CPSS_PX_PTP_TYPE_KEY_FORMAT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_PTP_TYPE_KEY_FORMAT_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_PTP_TYPE_KEY_FORMAT_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_PTP_TYPE_KEY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_PTP_TYPE_KEY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_PTP_TYPE_KEY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PTP_TAI_CLOCK_FREQ_ENT_clockFrequency = {
     "clockFrequency", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PTP_TAI_CLOCK_FREQ_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PTP_TAI_CLOCK_MODE_ENT_clockMode = {
     "clockMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PTP_TAI_CLOCK_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT_clockSelect = {
     "clockSelect", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT_pulseMode = {
     "pulseMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT_function = {
     "function", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PTP_TAI_TOD_COUNT_STC_PTR_todValuePtr = {
     "todValuePtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PTP_TAI_TOD_COUNT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PTP_TAI_TOD_STEP_STC_PTR_todStepPtr = {
     "todStepPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PTP_TAI_TOD_STEP_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PTP_TAI_TOD_TYPE_ENT_todValueType = {
     "todValueType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PTP_TAI_TOD_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PTP_TSU_CONTROL_STC_PTR_controlPtr = {
     "controlPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PTP_TSU_CONTROL_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_ENT_counterType = {
     "counterType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PTP_TYPE_INDEX_ptpTypeIndex = {
     "ptpTypeIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PTP_TYPE_INDEX)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PTP_TYPE_KEY_FORMAT_STC_PTR_keyDataPtr = {
     "keyDataPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PTP_TYPE_KEY_FORMAT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PTP_TYPE_KEY_FORMAT_STC_PTR_keyMaskPtr = {
     "keyMaskPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PTP_TYPE_KEY_FORMAT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PTP_TYPE_KEY_STC_PTR_portKeyPtr = {
     "portKeyPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PTP_TYPE_KEY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_32_fracNanoSecond = {
     "fracNanoSecond", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_captureIndex = {
     "captureIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_extPulseWidth = {
     "extPulseWidth", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_gracefulStep = {
     "gracefulStep", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_nanoSeconds = {
     "nanoSeconds", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_seconds = {
     "seconds", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PTP_TAI_CLOCK_MODE_ENT_PTR_clockModePtr = {
     "clockModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PTP_TAI_CLOCK_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT_PTR_clockSelectPtr = {
     "clockSelectPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT_PTR_pulseModePtr = {
     "pulseModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT_PTR_functionPtr = {
     "functionPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PTP_TAI_TOD_COUNT_STC_PTR_todValuePtr = {
     "todValuePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PTP_TAI_TOD_COUNT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PTP_TAI_TOD_STEP_STC_PTR_todStepPtr = {
     "todStepPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PTP_TAI_TOD_STEP_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PTP_TSU_CONTROL_STC_PTR_controlPtr = {
     "controlPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PTP_TSU_CONTROL_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PTP_TYPE_KEY_FORMAT_STC_PTR_keyDataPtr = {
     "keyDataPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PTP_TYPE_KEY_FORMAT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PTP_TYPE_KEY_FORMAT_STC_PTR_keyMaskPtr = {
     "keyMaskPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PTP_TYPE_KEY_FORMAT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PTP_TYPE_KEY_STC_PTR_portKeyPtr = {
     "portKeyPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PTP_TYPE_KEY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_32_PTR_fracNanoSecondPtr = {
     "fracNanoSecondPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_extPulseWidthPtr = {
     "extPulseWidthPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_gracefulStepPtr = {
     "gracefulStepPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_nanoSecondsPtr = {
     "nanoSecondsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_secondsPtr = {
     "secondsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPtpTaiClockModeSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PTP_TAI_CLOCK_MODE_ENT_clockMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPtpTaiInputClockSelectSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT_clockSelect,
    &PX_IN_CPSS_PX_PTP_TAI_CLOCK_FREQ_ENT_clockFrequency
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPtpTaiPulseInModeSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT_pulseMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPtpTaiTodCounterFunctionSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT_function
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPtpTaiTodStepSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PTP_TAI_TOD_STEP_STC_PTR_todStepPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPtpTaiTodSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PTP_TAI_TOD_TYPE_ENT_todValueType,
    &PX_IN_CPSS_PX_PTP_TAI_TOD_COUNT_STC_PTR_todValuePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPtpTaiTodGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PTP_TAI_TOD_TYPE_ENT_todValueType,
    &PX_OUT_CPSS_PX_PTP_TAI_TOD_COUNT_STC_PTR_todValuePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPtpTypeKeyEntrySet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PTP_TYPE_INDEX_ptpTypeIndex,
    &PX_IN_CPSS_PX_PTP_TYPE_KEY_FORMAT_STC_PTR_keyDataPtr,
    &PX_IN_CPSS_PX_PTP_TYPE_KEY_FORMAT_STC_PTR_keyMaskPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPtpTypeKeyEntryEnableSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PTP_TYPE_INDEX_ptpTypeIndex,
    &PX_IN_GT_BOOL_enable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPtpTypeKeyEntryGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PTP_TYPE_INDEX_ptpTypeIndex,
    &PX_OUT_CPSS_PX_PTP_TYPE_KEY_FORMAT_STC_PTR_keyDataPtr,
    &PX_OUT_CPSS_PX_PTP_TYPE_KEY_FORMAT_STC_PTR_keyMaskPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPtpTypeKeyEntryEnableGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PTP_TYPE_INDEX_ptpTypeIndex,
    &PX_OUT_GT_BOOL_PTR_enablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPtpTaiFractionalNanosecondDriftSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_32_fracNanoSecond
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPtpTsuControlSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PX_PTP_TSU_CONTROL_STC_PTR_controlPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPtpTsuPacketCouterGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_ENT_counterType,
    &PX_OUT_GT_U32_PTR_valuePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPtpPortTypeKeySet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PX_PTP_TYPE_KEY_STC_PTR_portKeyPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPtpTsuControlGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PX_PTP_TSU_CONTROL_STC_PTR_controlPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPtpPortTypeKeyGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PX_PTP_TYPE_KEY_STC_PTR_portKeyPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPtpTaiTodCaptureStatusGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_captureIndex,
    &PX_OUT_GT_BOOL_PTR_validPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPtpTaiExternalPulseWidthSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_extPulseWidth
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPtpTaiGracefulStepSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_gracefulStep
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPtpTaiPClockCycleSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_nanoSeconds
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPtpTaiClockCycleSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_seconds,
    &PX_IN_GT_U32_nanoSeconds
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPtpTaiClockModeGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_CPSS_PX_PTP_TAI_CLOCK_MODE_ENT_PTR_clockModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPtpTaiInputClockSelectGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT_PTR_clockSelectPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPtpTaiPulseInModeGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT_PTR_pulseModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPtpTaiTodCounterFunctionGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT_PTR_functionPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPtpTaiTodStepGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_CPSS_PX_PTP_TAI_TOD_STEP_STC_PTR_todStepPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPtpTaiFractionalNanosecondDriftGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_GT_32_PTR_fracNanoSecondPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPtpTaiExternalPulseWidthGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_GT_U32_PTR_extPulseWidthPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPtpTaiGracefulStepGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_GT_U32_PTR_gracefulStepPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPtpTaiPClockCycleGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_GT_U32_PTR_nanoSecondsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPtpTaiClockCycleGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_GT_U32_PTR_secondsPtr,
    &PX_OUT_GT_U32_PTR_nanoSecondsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPtpTaiIncomingTriggerCounterGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_GT_U32_PTR_valuePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPtpTaiIncomingClockCounterGet_PARAMS[] =  {
    &PX_IN_GT_U8_devNum,
    &PX_OUT_GT_U32_PTR_valuePtr
};


/********* lib API DB *********/

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssPxPtpLogLibDb[] = {
    {"cpssPxPtpTaiCaptureOverwriteEnableSet", 2, prvCpssLogGenDevNumEnable2_PARAMS, NULL},
    {"cpssPxPtpTaiCaptureOverwriteEnableGet", 2, prvCpssLogGenDevNumEnablePtr2_PARAMS, NULL},
    {"cpssPxPtpTaiClockCycleSet", 3, cpssPxPtpTaiClockCycleSet_PARAMS, NULL},
    {"cpssPxPtpTaiClockCycleGet", 3, cpssPxPtpTaiClockCycleGet_PARAMS, NULL},
    {"cpssPxPtpTaiClockModeSet", 2, cpssPxPtpTaiClockModeSet_PARAMS, NULL},
    {"cpssPxPtpTaiClockModeGet", 2, cpssPxPtpTaiClockModeGet_PARAMS, NULL},
    {"cpssPxPtpTaiExternalPulseWidthSet", 2, cpssPxPtpTaiExternalPulseWidthSet_PARAMS, NULL},
    {"cpssPxPtpTaiExternalPulseWidthGet", 2, cpssPxPtpTaiExternalPulseWidthGet_PARAMS, NULL},
    {"cpssPxPtpTaiFractionalNanosecondDriftSet", 2, cpssPxPtpTaiFractionalNanosecondDriftSet_PARAMS, NULL},
    {"cpssPxPtpTaiFractionalNanosecondDriftGet", 2, cpssPxPtpTaiFractionalNanosecondDriftGet_PARAMS, NULL},
    {"cpssPxPtpTaiGracefulStepSet", 2, cpssPxPtpTaiGracefulStepSet_PARAMS, NULL},
    {"cpssPxPtpTaiGracefulStepGet", 2, cpssPxPtpTaiGracefulStepGet_PARAMS, NULL},
    {"cpssPxPtpTaiIncomingClockCounterEnableSet", 2, prvCpssLogGenDevNumEnable2_PARAMS, NULL},
    {"cpssPxPtpTaiIncomingClockCounterEnableGet", 2, prvCpssLogGenDevNumEnablePtr2_PARAMS, NULL},
    {"cpssPxPtpTaiIncomingClockCounterGet", 2, cpssPxPtpTaiIncomingClockCounterGet_PARAMS, NULL},
    {"cpssPxPtpTaiIncomingTriggerCounterGet", 2, cpssPxPtpTaiIncomingTriggerCounterGet_PARAMS, NULL},
    {"cpssPxPtpTaiInputClockSelectSet", 3, cpssPxPtpTaiInputClockSelectSet_PARAMS, NULL},
    {"cpssPxPtpTaiInputClockSelectGet", 2, cpssPxPtpTaiInputClockSelectGet_PARAMS, NULL},
    {"cpssPxPtpTaiInputTriggersCountEnableSet", 2, prvCpssLogGenDevNumEnable2_PARAMS, NULL},
    {"cpssPxPtpTaiInputTriggersCountEnableGet", 2, prvCpssLogGenDevNumEnablePtr2_PARAMS, NULL},
    {"cpssPxPtpTaiOutputTriggerEnableSet", 2, prvCpssLogGenDevNumEnable2_PARAMS, NULL},
    {"cpssPxPtpTaiOutputTriggerEnableGet", 2, prvCpssLogGenDevNumEnablePtr2_PARAMS, NULL},
    {"cpssPxPtpTaiPClockCycleSet", 2, cpssPxPtpTaiPClockCycleSet_PARAMS, NULL},
    {"cpssPxPtpTaiPClockCycleGet", 2, cpssPxPtpTaiPClockCycleGet_PARAMS, NULL},
    {"cpssPxPtpTaiPClockDriftAdjustEnableSet", 2, prvCpssLogGenDevNumEnable2_PARAMS, NULL},
    {"cpssPxPtpTaiPtpPClockDriftAdjustEnableGet", 2, prvCpssLogGenDevNumEnablePtr2_PARAMS, NULL},
    {"cpssPxPtpTaiTodCaptureStatusGet", 3, cpssPxPtpTaiTodCaptureStatusGet_PARAMS, NULL},
    {"cpssPxPtpTaiTodSet", 3, cpssPxPtpTaiTodSet_PARAMS, NULL},
    {"cpssPxPtpTaiTodGet", 3, cpssPxPtpTaiTodGet_PARAMS, NULL},
    {"cpssPxPtpTaiTodStepSet", 2, cpssPxPtpTaiTodStepSet_PARAMS, NULL},
    {"cpssPxPtpTaiTodStepGet", 2, cpssPxPtpTaiTodStepGet_PARAMS, NULL},
    {"cpssPxPtpTaiTodUpdateCounterGet", 2, cpssPxPtpTaiIncomingTriggerCounterGet_PARAMS, NULL},
    {"cpssPxPtpTaiTodCounterFunctionSet", 2, cpssPxPtpTaiTodCounterFunctionSet_PARAMS, NULL},
    {"cpssPxPtpTaiTodCounterFunctionGet", 2, cpssPxPtpTaiTodCounterFunctionGet_PARAMS, NULL},
    {"cpssPxPtpTaiTodCounterFunctionTriggerSet", 1, prvCpssLogGenDevNum2_PARAMS, NULL},
    {"cpssPxPtpTaiTodCounterFunctionTriggerGet", 2, prvCpssLogGenDevNumEnablePtr2_PARAMS, NULL},
    {"cpssPxPtpTsuControlSet", 3, cpssPxPtpTsuControlSet_PARAMS, NULL},
    {"cpssPxPtpTsuControlGet", 3, cpssPxPtpTsuControlGet_PARAMS, NULL},
    {"cpssPxPtpTsuCountersClear", 2, prvCpssLogGenDevNumPortNum2_PARAMS, NULL},
    {"cpssPxPtpTsuPacketCouterGet", 4, cpssPxPtpTsuPacketCouterGet_PARAMS, NULL},
    {"cpssPxPtpPortTypeKeySet", 3, cpssPxPtpPortTypeKeySet_PARAMS, NULL},
    {"cpssPxPtpPortTypeKeyGet", 3, cpssPxPtpPortTypeKeyGet_PARAMS, NULL},
    {"cpssPxPtpTypeKeyEntrySet", 4, cpssPxPtpTypeKeyEntrySet_PARAMS, NULL},
    {"cpssPxPtpTypeKeyEntryGet", 4, cpssPxPtpTypeKeyEntryGet_PARAMS, NULL},
    {"cpssPxPtpTypeKeyEntryEnableSet", 3, cpssPxPtpTypeKeyEntryEnableSet_PARAMS, NULL},
    {"cpssPxPtpTypeKeyEntryEnableGet", 3, cpssPxPtpTypeKeyEntryEnableGet_PARAMS, NULL},
    {"cpssPxPtpTaiPClockOutputEnableSet", 2, prvCpssLogGenDevNumEnable2_PARAMS, NULL},
    {"cpssPxPtpTaiPClockOutputEnableGet", 2, prvCpssLogGenDevNumEnablePtr2_PARAMS, NULL},
    {"cpssPxPtpTaiPhaseUpdateEnableSet", 2, prvCpssLogGenDevNumEnable2_PARAMS, NULL},
    {"cpssPxPtpTaiPhaseUpdateEnableGet", 2, prvCpssLogGenDevNumEnablePtr2_PARAMS, NULL},
    {"cpssPxPtpTaiPulseInModeSet", 2, cpssPxPtpTaiPulseInModeSet_PARAMS, NULL},
    {"cpssPxPtpTaiPulseInModeGet", 2, cpssPxPtpTaiPulseInModeGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_PX_CPSS_LOG_LIB_PTP(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssPxPtpLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssPxPtpLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

