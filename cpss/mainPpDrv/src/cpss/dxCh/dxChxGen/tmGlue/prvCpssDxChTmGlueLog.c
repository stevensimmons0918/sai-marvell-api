/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChTmGlueLog.c
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
#include <cpss/dxCh/dxChxGen/networkIf/private/prvCpssDxChNetworkIfLog.h>
#include <cpss/dxCh/dxChxGen/tmGlue/cpssDxChTmGlueAgingAndDelay.h>
#include <cpss/dxCh/dxChxGen/tmGlue/cpssDxChTmGlueDram.h>
#include <cpss/dxCh/dxChxGen/tmGlue/cpssDxChTmGlueDrop.h>
#include <cpss/dxCh/dxChxGen/tmGlue/cpssDxChTmGluePfc.h>
#include <cpss/dxCh/dxChxGen/tmGlue/cpssDxChTmGlueQueueMap.h>
#include <cpss/dxCh/dxChxGen/tmGlue/private/prvCpssDxChTmGlueLog.h>
#include <cpss/generic/cpssHwInit/private/prvCpssGenCpssHwInitLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssGenDbLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_ENT[]  =
{
    "CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_TXQ_E",
    "CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_TM_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TM_GLUE_QUEUE_MAP_MC_MODE_ENT[]  =
{
    "CPSS_DXCH_TM_GLUE_QUEUE_MAP_MC_MODE_REGULAR_E",
    "CPSS_DXCH_TM_GLUE_QUEUE_MAP_MC_MODE_FORCE_EPCL_CONFIG_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TM_GLUE_QUEUE_MAP_MC_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TM_GLUE_QUEUE_MAP_SELECTOR_ACCESS_MODE_ENT[]  =
{
    "CPSS_DXCH_TM_GLUE_QUEUE_MAP_SELECTOR_ACCESS_MODE_LOCAL_TARGET_PORT_E",
    "CPSS_DXCH_TM_GLUE_QUEUE_MAP_SELECTOR_ACCESS_MODE_TARGET_DEV_MAP_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TM_GLUE_QUEUE_MAP_SELECTOR_ACCESS_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TM_GLUE_QUEUE_MAP_TO_CPU_MODE_ENT[]  =
{
    "CPSS_DXCH_TM_GLUE_QUEUE_MAP_TO_CPU_MODE_REGULAR_E",
    "CPSS_DXCH_TM_GLUE_QUEUE_MAP_TO_CPU_MODE_USE_TO_CPU_CONFIG_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TM_GLUE_QUEUE_MAP_TO_CPU_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TM_GLUE_DRAM_CONFIGURATION_TYPE_ENT[]  =
{
    "CPSS_DXCH_TM_GLUE_DRAM_CONFIGURATION_TYPE_DYNAMIC_E",
    "CPSS_DXCH_TM_GLUE_DRAM_CONFIGURATION_TYPE_STATIC_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TM_GLUE_DRAM_CONFIGURATION_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TM_GLUE_DROP_RED_PACKET_DROP_MODE_ENT[]  =
{
    "CPSS_DXCH_TM_GLUE_DROP_RED_PACKET_DROP_MODE_DROP_ALL_RED_E",
    "CPSS_DXCH_TM_GLUE_DROP_RED_PACKET_DROP_MODE_ASK_TM_RESP_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TM_GLUE_DROP_RED_PACKET_DROP_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_CMD_ENT[]  =
{
    "CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_CMD_ADD_E",
    "CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_CMD_SUBTRACT_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_CMD_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_ENT[]  =
{
    "CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_ZERO_E",
    "CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_EPCL_QUEUE_ID_E",
    "CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_FLOW_ID_E",
    "CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_LOCAL_TARGET_PHY_PORT_E",
    "CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_TM_TC_E",
    "CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_PACKET_HASH_E",
    "CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_TARGET_DEV_E",
    "CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_TARGET_PHY_PORT_E",
    "CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_TARGET_EPORT_E",
    "CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_ONE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_TM_AGING_AND_DELAY_PROFILE_THRESHOLDS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TM_AGING_AND_DELAY_PROFILE_THRESHOLDS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, threshold0);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, threshold1);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, threshold2);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TM_AGING_AND_DELAY_STATISTICS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TM_AGING_AND_DELAY_STATISTICS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, packetCounter);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, octetCounter, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, latencyTimeCounter, GT_U64);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, agedPacketCounter);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxLatencyTime);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, minLatencyTime);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TM_GLUE_DRAM_ALGORITHM_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TM_GLUE_DRAM_ALGORITHM_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, algoType, CPSS_DXCH_TM_GLUE_DRAM_CONFIGURATION_TYPE_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, performWriteLeveling);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TM_GLUE_DRAM_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TM_GLUE_DRAM_CFG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, activeInterfaceNum);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, activeInterfaceMask);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, interfaceParams, CPSS_DXCH_TM_GLUE_DRAM_INF_PARAM_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TM_GLUE_DROP_MASK_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TM_GLUE_DROP_MASK_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, qTailDropUnmask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, qWredDropUnmask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, aTailDropUnmask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, aWredDropUnmask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, bTailDropUnmask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, bWredDropUnmask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, cTailDropUnmask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, cWredDropUnmask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, portTailDropUnmask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, portWredDropUnmask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, outOfResourceDropUnmask);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, redPacketsDropMode, CPSS_DXCH_TM_GLUE_DROP_RED_PACKET_DROP_MODE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, queueIdBase);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, bitSelectArr, CPSS_DXCH_TM_GLUE_QUEUE_ID_BIT_WIDTH_CNS, CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, offsetCmd, CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_CMD_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, offsetValue);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TM_GLUE_DRAM_INF_PARAM_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TM_GLUE_DRAM_INF_PARAM_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, busParams, CPSS_DXCH_TM_GLUE_DRAM_INF_BUS_CNS, CPSS_DXCH_TM_GLUE_DRAM_BUS_PARAM_STC);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, speedBin, CPSS_DRAM_SPEED_BIN_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, busWidth, CPSS_DRAM_BUS_WIDTH_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, memorySize, CPSS_DRAM_SIZE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, memoryFreq, CPSS_DRAM_FREQUENCY_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, casWL);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, casL);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, interfaceTemp, CPSS_DRAM_TEMPERATURE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, selectType, CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, bitSelector);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TM_GLUE_DRAM_BUS_PARAM_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TM_GLUE_DRAM_BUS_PARAM_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, csBitmask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, mirrorEn);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, dqsSwapEn);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ckSwapEn);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_TM_AGING_AND_DELAY_PROFILE_THRESHOLDS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_TM_AGING_AND_DELAY_PROFILE_THRESHOLDS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_TM_AGING_AND_DELAY_PROFILE_THRESHOLDS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_TM_AGING_AND_DELAY_STATISTICS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_TM_AGING_AND_DELAY_STATISTICS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_TM_AGING_AND_DELAY_STATISTICS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_TM_GLUE_DRAM_ALGORITHM_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_TM_GLUE_DRAM_ALGORITHM_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_TM_GLUE_DRAM_ALGORITHM_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_TM_GLUE_DRAM_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_TM_GLUE_DRAM_CFG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_TM_GLUE_DRAM_CFG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_TM_GLUE_DROP_MASK_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_TM_GLUE_DROP_MASK_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_TM_GLUE_DROP_MASK_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_TM_GLUE_QUEUE_MAP_MC_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TM_GLUE_QUEUE_MAP_MC_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_TM_GLUE_QUEUE_MAP_MC_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TM_GLUE_QUEUE_MAP_MC_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TM_GLUE_QUEUE_MAP_MC_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_TM_GLUE_QUEUE_MAP_MC_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TM_GLUE_QUEUE_MAP_SELECTOR_ACCESS_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TM_GLUE_QUEUE_MAP_SELECTOR_ACCESS_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_TM_GLUE_QUEUE_MAP_SELECTOR_ACCESS_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TM_GLUE_QUEUE_MAP_SELECTOR_ACCESS_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TM_GLUE_QUEUE_MAP_SELECTOR_ACCESS_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_TM_GLUE_QUEUE_MAP_SELECTOR_ACCESS_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TM_GLUE_QUEUE_MAP_TO_CPU_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TM_GLUE_QUEUE_MAP_TO_CPU_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_TM_GLUE_QUEUE_MAP_TO_CPU_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TM_GLUE_QUEUE_MAP_TO_CPU_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TM_GLUE_QUEUE_MAP_TO_CPU_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_TM_GLUE_QUEUE_MAP_TO_CPU_MODE_ENT);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_timerResolutionPtr = {
     "timerResolutionPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_NET_DSA_CMD_ENT_packetDsaCommand = {
     "packetDsaCommand", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_NET_DSA_CMD_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TM_AGING_AND_DELAY_PROFILE_THRESHOLDS_STC_PTR_thresholdsPtr = {
     "thresholdsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TM_AGING_AND_DELAY_PROFILE_THRESHOLDS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TM_GLUE_DRAM_ALGORITHM_STC_PTR_dramAlgoPtr = {
     "dramAlgoPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TM_GLUE_DRAM_ALGORITHM_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TM_GLUE_DRAM_CFG_STC_PTR_dramCfgPtr = {
     "dramCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TM_GLUE_DRAM_CFG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TM_GLUE_DROP_MASK_STC_PTR_dropMaskCfgPtr = {
     "dropMaskCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TM_GLUE_DROP_MASK_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_ENT_responseMode = {
     "responseMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_ENTRY_STC_PTR_entryPtr = {
     "entryPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_STC_PTR_lengthOffsetPtr = {
     "lengthOffsetPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TM_GLUE_QUEUE_MAP_MC_MODE_ENT_mcMode = {
     "mcMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TM_GLUE_QUEUE_MAP_MC_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TM_GLUE_QUEUE_MAP_SELECTOR_ACCESS_MODE_ENT_mode = {
     "mode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TM_GLUE_QUEUE_MAP_SELECTOR_ACCESS_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TM_GLUE_QUEUE_MAP_TO_CPU_MODE_ENT_toCpuMode = {
     "toCpuMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TM_GLUE_QUEUE_MAP_TO_CPU_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_bypass = {
     "bypass", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_egressEnable = {
     "egressEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_forceCounting = {
     "forceCounting", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_forceMeter = {
     "forceMeter", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_ingressEnable = {
     "ingressEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_isUnicast = {
     "isUnicast", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_HW_DEV_NUM_targetHwDevId = {
     "targetHwDevId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_HW_DEV_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_PHYSICAL_PORT_NUM_pfcPortNum = {
     "pfcPortNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_PHYSICAL_PORT_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_PHYSICAL_PORT_NUM_physicalPort = {
     "physicalPort", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_PHYSICAL_PORT_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_HEX_agedPacketCouterQueueMask = {
     "agedPacketCouterQueueMask", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32_HEX)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_agedPacketCouterQueue = {
     "agedPacketCouterQueue", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_cNodeValue = {
     "cNodeValue", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_counterSetIndex = {
     "counterSetIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_externalMemoryInitFlags = {
     "externalMemoryInitFlags", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_queueId = {
     "queueId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_tmPortInd = {
     "tmPortInd", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_tmTc = {
     "tmTc", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_toCpuSelectorIndex = {
     "toCpuSelectorIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TM_AGING_AND_DELAY_PROFILE_THRESHOLDS_STC_PTR_thresholdsPtr = {
     "thresholdsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TM_AGING_AND_DELAY_PROFILE_THRESHOLDS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TM_AGING_AND_DELAY_STATISTICS_STC_PTR_agingCountersPtr = {
     "agingCountersPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TM_AGING_AND_DELAY_STATISTICS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TM_GLUE_DROP_MASK_STC_PTR_dropMaskCfgPtr = {
     "dropMaskCfgPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TM_GLUE_DROP_MASK_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_ENT_PTR_responseModePtr = {
     "responseModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_ENTRY_STC_PTR_entryPtr = {
     "entryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_STC_PTR_lengthOffsetPtr = {
     "lengthOffsetPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TM_GLUE_QUEUE_MAP_MC_MODE_ENT_PTR_mcModePtr = {
     "mcModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TM_GLUE_QUEUE_MAP_MC_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TM_GLUE_QUEUE_MAP_SELECTOR_ACCESS_MODE_ENT_PTR_modePtr = {
     "modePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TM_GLUE_QUEUE_MAP_SELECTOR_ACCESS_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TM_GLUE_QUEUE_MAP_TO_CPU_MODE_ENT_PTR_toCpuModePtr = {
     "toCpuModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TM_GLUE_QUEUE_MAP_TO_CPU_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_bypassPtr = {
     "bypassPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_egressEnablePtr = {
     "egressEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_forceCountingPtr = {
     "forceCountingPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_forceMeterPtr = {
     "forceMeterPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_ingressEnablePtr = {
     "ingressEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_PHYSICAL_PORT_NUM_PTR_pfcPortNumPtr = {
     "pfcPortNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_PHYSICAL_PORT_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_HEX_PTR_agedPacketCouterQueueMaskPtr = {
     "agedPacketCouterQueueMaskPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32_HEX)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_agedOutPacketCounterPtr = {
     "agedOutPacketCounterPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_agedPacketCounterPtr = {
     "agedPacketCounterPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_agedPacketCouterQueuePtr = {
     "agedPacketCouterQueuePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_cNodeValuePtr = {
     "cNodeValuePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_entryIndexPtr = {
     "entryIndexPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_queueIdPtr = {
     "queueIdPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_timerResolutionPtr = {
     "timerResolutionPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tmTcPtr = {
     "tmTcPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_toCpuSelectorIndexPtr = {
     "toCpuSelectorIndexPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueQueueMapTcToTcMapSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_NET_DSA_CMD_ENT_packetDsaCommand,
    &DX_IN_GT_BOOL_isUnicast,
    &DX_IN_GT_U32_tc,
    &DX_IN_GT_U32_tmTc
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueQueueMapTcToTcMapGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_NET_DSA_CMD_ENT_packetDsaCommand,
    &DX_IN_GT_BOOL_isUnicast,
    &DX_IN_GT_U32_tc,
    &DX_OUT_GT_U32_PTR_tmTcPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueDramInit_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TM_GLUE_DRAM_CFG_STC_PTR_dramCfgPtr,
    &DX_IN_CPSS_DXCH_TM_GLUE_DRAM_ALGORITHM_STC_PTR_dramAlgoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueQueueMapMcModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TM_GLUE_QUEUE_MAP_MC_MODE_ENT_mcMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueQueueMapSelectorTableAccessModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TM_GLUE_QUEUE_MAP_SELECTOR_ACCESS_MODE_ENT_mode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueQueueMapToCpuModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TM_GLUE_QUEUE_MAP_TO_CPU_MODE_ENT_toCpuMode,
    &DX_IN_GT_U32_toCpuSelectorIndex
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueQueueMapCpuCodeToTcMapSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_NET_RX_CPU_CODE_ENT_cpuCode,
    &DX_IN_GT_U32_tmTc
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueQueueMapCpuCodeToTcMapGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_NET_RX_CPU_CODE_ENT_cpuCode,
    &DX_OUT_GT_U32_PTR_tmTcPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueQueueMapBypassEnableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_BOOL_bypass
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueFlowControlEnableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_BOOL_egressEnable,
    &DX_IN_GT_BOOL_ingressEnable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueQueueMapTargetDeviceToIndexMapSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_HW_DEV_NUM_targetHwDevId,
    &DX_IN_GT_U32_entryIndex
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueQueueMapTargetDeviceToIndexMapGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_HW_DEV_NUM_targetHwDevId,
    &DX_OUT_GT_U32_PTR_entryIndexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGluePfcTmTcPort2CNodeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_pfcPortNum,
    &DX_IN_GT_U32_tc,
    &DX_IN_GT_U32_cNodeValue
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGluePfcTmTcPort2CNodeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_pfcPortNum,
    &DX_IN_GT_U32_tc,
    &DX_OUT_GT_U32_PTR_cNodeValuePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueQueueMapL1PacketLengthOffsetSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_physicalPort,
    &DX_IN_CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_STC_PTR_lengthOffsetPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueQueueMapL1PacketLengthOffsetGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_physicalPort,
    &DX_OUT_CPSS_DXCH_TM_GLUE_QUEUE_MAP_L1_PACKET_LENGTH_OFFSET_STC_PTR_lengthOffsetPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGluePfcResponseModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_ENT_responseMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGluePfcPortMappingSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_pfcPortNum
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGluePfcResponseModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_ENT_PTR_responseModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGluePfcPortMappingGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_PHYSICAL_PORT_NUM_PTR_pfcPortNumPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueAgingAndDelayCounterQueueIdSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_counterSetIndex,
    &DX_IN_GT_U32_queueId
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueAgingAndDelayCountersGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_counterSetIndex,
    &DX_OUT_CPSS_DXCH_TM_AGING_AND_DELAY_STATISTICS_STC_PTR_agingCountersPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueAgingAndDelayCounterQueueIdGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_counterSetIndex,
    &DX_OUT_GT_U32_PTR_queueIdPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueQueueMapBitSelectTableEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_entryIndex,
    &DX_IN_CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_ENTRY_STC_PTR_entryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueQueueMapBitSelectTableEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_entryIndex,
    &DX_OUT_CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_ENTRY_STC_PTR_entryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueDramInitFlagsSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_externalMemoryInitFlags
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_profileId,
    &DX_IN_CPSS_DXCH_TM_AGING_AND_DELAY_PROFILE_THRESHOLDS_STC_PTR_thresholdsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueDropProfileDropMaskSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_profileId,
    &DX_IN_GT_U32_tc,
    &DX_IN_CPSS_DXCH_TM_GLUE_DROP_MASK_STC_PTR_dropMaskCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueDropProfileDropMaskGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_profileId,
    &DX_IN_GT_U32_tc,
    &DX_OUT_CPSS_DXCH_TM_GLUE_DROP_MASK_STC_PTR_dropMaskCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_profileId,
    &DX_OUT_CPSS_DXCH_TM_AGING_AND_DELAY_PROFILE_THRESHOLDS_STC_PTR_thresholdsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueQueueMapEgressPolicerForceSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_queueId,
    &DX_IN_GT_BOOL_forceMeter,
    &DX_IN_GT_BOOL_forceCounting
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_queueId,
    &DX_IN_GT_U32_profileId
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueQueueMapEgressPolicerForceGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_queueId,
    &DX_OUT_GT_BOOL_PTR_forceMeterPtr,
    &DX_OUT_GT_BOOL_PTR_forceCountingPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_queueId,
    &DX_OUT_GT_U32_PTR_profileIdPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_tmPort,
    &DX_IN_GT_BOOL_PTR_enablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_tmPort,
    &DX_IN_GT_BOOL_enable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueAgingAndDelayPerPortAgedPacketCounterGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_tmPort,
    &DX_OUT_GT_U32_PTR_agedOutPacketCounterPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueFlowControlEgressCounterSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_tmPortInd,
    &DX_IN_GT_U32_value
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueFlowControlEgressThresholdsSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_tmPortInd,
    &DX_IN_GT_U32_xOffThreshold,
    &DX_IN_GT_U32_xOnThreshold
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueFlowControlEgressCounterGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_tmPortInd,
    &DX_OUT_GT_U32_PTR_valuePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueFlowControlEgressThresholdsGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_tmPortInd,
    &DX_OUT_GT_U32_PTR_xOffThresholdPtr,
    &DX_OUT_GT_U32_PTR_xOnThresholdPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueDropTcToCosSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_tmTc,
    &DX_IN_GT_U32_cos
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueDropTcToCosGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_tmTc,
    &DX_OUT_GT_U32_PTR_cosPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_HEX_agedPacketCouterQueueMask,
    &DX_IN_GT_U32_agedPacketCouterQueue
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueAgingAndDelayTimerResolutionSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_INOUT_GT_U32_PTR_timerResolutionPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueQueueMapMcModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_TM_GLUE_QUEUE_MAP_MC_MODE_ENT_PTR_mcModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueQueueMapSelectorTableAccessModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_TM_GLUE_QUEUE_MAP_SELECTOR_ACCESS_MODE_ENT_PTR_modePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueQueueMapToCpuModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_TM_GLUE_QUEUE_MAP_TO_CPU_MODE_ENT_PTR_toCpuModePtr,
    &DX_OUT_GT_U32_PTR_toCpuSelectorIndexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueQueueMapBypassEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_BOOL_PTR_bypassPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueFlowControlEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_BOOL_PTR_egressEnablePtr,
    &DX_OUT_GT_BOOL_PTR_ingressEnablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueAgingAndDelayAgedPacketCounterGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_agedPacketCounterPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueAgingAndDelayTimerGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_timerPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueAgingAndDelayTimerResolutionGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_timerResolutionPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_HEX_PTR_agedPacketCouterQueueMaskPtr,
    &DX_OUT_GT_U32_PTR_agedPacketCouterQueuePtr
};


/********* lib API DB *********/

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChTmGlueLogLibDb[] = {
    {"cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableSet", 3, cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableSet_PARAMS, NULL},
    {"cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableGet", 3, cpssDxChTmGlueGeneralPerPortDropOutgoingPacketEnableGet_PARAMS, NULL},
    {"cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigSet", 3, cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigSet_PARAMS, NULL},
    {"cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigGet", 3, cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigGet_PARAMS, NULL},
    {"cpssDxChTmGlueAgingAndDelayAgedPacketCounterGet", 2, cpssDxChTmGlueAgingAndDelayAgedPacketCounterGet_PARAMS, NULL},
    {"cpssDxChTmGlueAgingAndDelayTimerResolutionSet", 2, cpssDxChTmGlueAgingAndDelayTimerResolutionSet_PARAMS, NULL},
    {"cpssDxChTmGlueAgingAndDelayTimerResolutionGet", 2, cpssDxChTmGlueAgingAndDelayTimerResolutionGet_PARAMS, NULL},
    {"cpssDxChTmGlueAgingAndDelayTimerGet", 2, cpssDxChTmGlueAgingAndDelayTimerGet_PARAMS, NULL},
    {"cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsSet", 3, cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsSet_PARAMS, NULL},
    {"cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsGet", 3, cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsGet_PARAMS, NULL},
    {"cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdSet", 3, cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdSet_PARAMS, NULL},
    {"cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdGet", 3, cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdGet_PARAMS, NULL},
    {"cpssDxChTmGlueAgingAndDelayCounterQueueIdSet", 3, cpssDxChTmGlueAgingAndDelayCounterQueueIdSet_PARAMS, NULL},
    {"cpssDxChTmGlueAgingAndDelayCounterQueueIdGet", 3, cpssDxChTmGlueAgingAndDelayCounterQueueIdGet_PARAMS, NULL},
    {"cpssDxChTmGlueAgingAndDelayCountersGet", 3, cpssDxChTmGlueAgingAndDelayCountersGet_PARAMS, NULL},
    {"cpssDxChTmGlueAgingAndDelayPerPortAgedPacketCounterGet", 3, cpssDxChTmGlueAgingAndDelayPerPortAgedPacketCounterGet_PARAMS, NULL},
    {"cpssDxChTmGlueDramInit", 3, cpssDxChTmGlueDramInit_PARAMS, NULL},
    {"cpssDxChTmGlueDramInitFlagsSet", 2, cpssDxChTmGlueDramInitFlagsSet_PARAMS, NULL},
    {"cpssDxChTmGlueDropQueueProfileIdSet", 3, cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdSet_PARAMS, NULL},
    {"cpssDxChTmGlueDropQueueProfileIdGet", 3, cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdGet_PARAMS, NULL},
    {"cpssDxChTmGlueDropProfileDropMaskSet", 4, cpssDxChTmGlueDropProfileDropMaskSet_PARAMS, NULL},
    {"cpssDxChTmGlueDropProfileDropMaskGet", 4, cpssDxChTmGlueDropProfileDropMaskGet_PARAMS, NULL},
    {"cpssDxChTmGlueDropTcToCosSet", 3, cpssDxChTmGlueDropTcToCosSet_PARAMS, NULL},
    {"cpssDxChTmGlueDropTcToCosGet", 3, cpssDxChTmGlueDropTcToCosGet_PARAMS, NULL},
    {"cpssDxChTmGlueFlowControlEnableSet", 3, cpssDxChTmGlueFlowControlEnableSet_PARAMS, NULL},
    {"cpssDxChTmGlueFlowControlEnableGet", 3, cpssDxChTmGlueFlowControlEnableGet_PARAMS, NULL},
    {"cpssDxChTmGlueFlowControlEgressCounterSet", 3, cpssDxChTmGlueFlowControlEgressCounterSet_PARAMS, NULL},
    {"cpssDxChTmGlueFlowControlEgressCounterGet", 3, cpssDxChTmGlueFlowControlEgressCounterGet_PARAMS, NULL},
    {"cpssDxChTmGlueFlowControlEgressThresholdsSet", 4, cpssDxChTmGlueFlowControlEgressThresholdsSet_PARAMS, NULL},
    {"cpssDxChTmGlueFlowControlEgressThresholdsGet", 4, cpssDxChTmGlueFlowControlEgressThresholdsGet_PARAMS, NULL},
    {"cpssDxChTmGlueFlowControlPortSpeedSet", 3, prvCpssLogGenDevNumPortNumSpeed_PARAMS, NULL},
    {"cpssDxChTmGlueFlowControlPortSpeedGet", 3, prvCpssLogGenDevNumPortNumSpeedPtr_PARAMS, NULL},
    {"cpssDxChTmGluePfcTmTcPort2CNodeSet", 4, cpssDxChTmGluePfcTmTcPort2CNodeSet_PARAMS, NULL},
    {"cpssDxChTmGluePfcTmTcPort2CNodeGet", 4, cpssDxChTmGluePfcTmTcPort2CNodeGet_PARAMS, NULL},
    {"cpssDxChTmGluePfcResponseModeSet", 3, cpssDxChTmGluePfcResponseModeSet_PARAMS, NULL},
    {"cpssDxChTmGluePfcResponseModeGet", 3, cpssDxChTmGluePfcResponseModeGet_PARAMS, NULL},
    {"cpssDxChTmGluePfcPortMappingSet", 3, cpssDxChTmGluePfcPortMappingSet_PARAMS, NULL},
    {"cpssDxChTmGluePfcPortMappingGet", 3, cpssDxChTmGluePfcPortMappingGet_PARAMS, NULL},
    {"cpssDxChTmGlueQueueMapBypassEnableSet", 2, cpssDxChTmGlueQueueMapBypassEnableSet_PARAMS, NULL},
    {"cpssDxChTmGlueQueueMapBypassEnableGet", 2, cpssDxChTmGlueQueueMapBypassEnableGet_PARAMS, NULL},
    {"cpssDxChTmGlueQueueMapBitSelectTableEntrySet", 3, cpssDxChTmGlueQueueMapBitSelectTableEntrySet_PARAMS, NULL},
    {"cpssDxChTmGlueQueueMapBitSelectTableEntryGet", 3, cpssDxChTmGlueQueueMapBitSelectTableEntryGet_PARAMS, NULL},
    {"cpssDxChTmGlueQueueMapTargetDeviceToIndexMapSet", 3, cpssDxChTmGlueQueueMapTargetDeviceToIndexMapSet_PARAMS, NULL},
    {"cpssDxChTmGlueQueueMapTargetDeviceToIndexMapGet", 3, cpssDxChTmGlueQueueMapTargetDeviceToIndexMapGet_PARAMS, NULL},
    {"cpssDxChTmGlueQueueMapCpuCodeToTcMapSet", 3, cpssDxChTmGlueQueueMapCpuCodeToTcMapSet_PARAMS, NULL},
    {"cpssDxChTmGlueQueueMapCpuCodeToTcMapGet", 3, cpssDxChTmGlueQueueMapCpuCodeToTcMapGet_PARAMS, NULL},
    {"cpssDxChTmGlueQueueMapTcToTcMapSet", 5, cpssDxChTmGlueQueueMapTcToTcMapSet_PARAMS, NULL},
    {"cpssDxChTmGlueQueueMapTcToTcMapGet", 5, cpssDxChTmGlueQueueMapTcToTcMapGet_PARAMS, NULL},
    {"cpssDxChTmGlueQueueMapEgressPolicerForceSet", 4, cpssDxChTmGlueQueueMapEgressPolicerForceSet_PARAMS, NULL},
    {"cpssDxChTmGlueQueueMapEgressPolicerForceGet", 4, cpssDxChTmGlueQueueMapEgressPolicerForceGet_PARAMS, NULL},
    {"cpssDxChTmGlueQueueMapToCpuModeSet", 3, cpssDxChTmGlueQueueMapToCpuModeSet_PARAMS, NULL},
    {"cpssDxChTmGlueQueueMapToCpuModeGet", 3, cpssDxChTmGlueQueueMapToCpuModeGet_PARAMS, NULL},
    {"cpssDxChTmGlueQueueMapMcModeSet", 2, cpssDxChTmGlueQueueMapMcModeSet_PARAMS, NULL},
    {"cpssDxChTmGlueQueueMapMcModeGet", 2, cpssDxChTmGlueQueueMapMcModeGet_PARAMS, NULL},
    {"cpssDxChTmGlueQueueMapSelectorTableAccessModeSet", 2, cpssDxChTmGlueQueueMapSelectorTableAccessModeSet_PARAMS, NULL},
    {"cpssDxChTmGlueQueueMapSelectorTableAccessModeGet", 2, cpssDxChTmGlueQueueMapSelectorTableAccessModeGet_PARAMS, NULL},
    {"cpssDxChTmGlueQueueMapL1PacketLengthOffsetSet", 3, cpssDxChTmGlueQueueMapL1PacketLengthOffsetSet_PARAMS, NULL},
    {"cpssDxChTmGlueQueueMapL1PacketLengthOffsetGet", 3, cpssDxChTmGlueQueueMapL1PacketLengthOffsetGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_TM_GLUE(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChTmGlueLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChTmGlueLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

