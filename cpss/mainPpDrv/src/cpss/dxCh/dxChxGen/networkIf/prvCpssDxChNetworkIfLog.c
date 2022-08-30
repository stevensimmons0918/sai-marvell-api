/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChNetworkIfLog.c
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
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfMii.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/networkIf/private/prvCpssDxChNetworkIfLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssGenDbLog.h>
#include <cpss/generic/networkIf/private/prvCpssGenNetworkIfLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_DXCH_NET_DSA_CMD_ENT[]  =
{
    "CPSS_DXCH_NET_DSA_CMD_TO_CPU_E",
    "CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E",
    "CPSS_DXCH_NET_DSA_CMD_TO_ANALYZER_E",
    "CPSS_DXCH_NET_DSA_CMD_FORWARD_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_NET_DSA_CMD_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_NET_RESOURCE_ERROR_MODE_ENT[]  =
{
    "CPSS_DXCH_NET_RESOURCE_ERROR_MODE_RETRY_E",
    "CPSS_DXCH_NET_RESOURCE_ERROR_MODE_ABORT_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_NET_RESOURCE_ERROR_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_NET_TO_CPU_FLOW_ID_OR_TT_OFFSET_MODE_ENT[]  =
{
    "CPSS_DXCH_NET_TO_CPU_FLOW_ID_MODE_E",
    "CPSS_DXCH_NET_TO_CPU_TT_OFFSET_MODE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_NET_TO_CPU_FLOW_ID_OR_TT_OFFSET_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_NET_TX_GENERATOR_BURST_STATUS_ENT[]  =
{
    "CPSS_DXCH_NET_TX_GENERATOR_BURST_STATUS_IDLE_E",
    "CPSS_DXCH_NET_TX_GENERATOR_BURST_STATUS_RUN_E",
    "CPSS_DXCH_NET_TX_GENERATOR_BURST_STATUS_LIMIT_E",
    "CPSS_DXCH_NET_TX_GENERATOR_BURST_STATUS_TERMINATE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_NET_TX_GENERATOR_BURST_STATUS_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT[]  =
{
    "CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_GLOBAL_THROUGHPUT_E",
    "CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_PER_FLOW_INTERVAL_E",
    "CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_NO_GAP_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_NET_DSA_TYPE_ENT[]  =
{
    "CPSS_DXCH_NET_DSA_1_WORD_TYPE_ENT",
    "CPSS_DXCH_NET_DSA_2_WORD_TYPE_ENT",
    "CPSS_DXCH_NET_DSA_3_WORD_TYPE_ENT",
    "CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_NET_DSA_TYPE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_NETIF_APP_SPECIFIC_CPUCODE_TCP_UDP_PORT_RANGE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_NETIF_APP_SPECIFIC_CPUCODE_TCP_UDP_PORT_RANGE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, minL4Port);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxL4Port);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, packetType, CPSS_NET_TCP_UDP_PACKET_TYPE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, protocol, CPSS_NET_PROT_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, cpuCode, CPSS_NET_RX_CPU_CODE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, l4PortMode, CPSS_NET_CPU_CODE_L4_PORT_MODE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_NETIF_MII_INIT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_NETIF_MII_INIT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numOfTxDesc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, txInternalBufBlockPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, txInternalBufBlockSize);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, bufferPercentage, CPSS_MAX_RX_QUEUE_CNS, GT_U32);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, rxBufSize);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, headerOffset);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, rxBufBlockPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, rxBufBlockSize);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, tc);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, dp, CPSS_DP_LEVEL_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, truncate);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, cpuRateLimitMode, CPSS_NET_CPU_CODE_RATE_LIMIT_MODE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, cpuCodeRateLimiterIndex);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, cpuCodeStatRateLimitIndex);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, designatedDevNumIndex);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_NET_RX_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_NET_RX_PARAMS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, dsaParam, CPSS_DXCH_NET_DSA_PARAMS_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_NET_SDMA_RX_COUNTERS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_NET_SDMA_RX_COUNTERS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, rxInPkts);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, rxInOctets);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_NET_TX_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_NET_TX_PARAMS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, packetIsTagged);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, cookie);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, sdmaInfo, CPSS_DXCH_NET_SDMA_TX_PARAMS_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, dsaParam, CPSS_DXCH_NET_DSA_PARAMS_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_NET_DSA_COMMON_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_NET_DSA_COMMON_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, dsaTagType, CPSS_DXCH_NET_DSA_TYPE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vpt);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, cfiBit);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vid);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, dropOnSource);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, packetIsLooped);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_NET_SDMA_TX_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_NET_SDMA_TX_PARAMS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, recalcCrc);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, txQueue);
    PRV_CPSS_LOG_STC_UINTPTR_MAC(valPtr, evReqHndl);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, invokeTxBufferQueueEvent);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_NETIF_APP_SPECIFIC_CPUCODE_TCP_UDP_PORT_RANGE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_NETIF_APP_SPECIFIC_CPUCODE_TCP_UDP_PORT_RANGE_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_NETIF_APP_SPECIFIC_CPUCODE_TCP_UDP_PORT_RANGE_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_NETIF_MII_INIT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_NETIF_MII_INIT_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_NETIF_MII_INIT_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_NET_DSA_CMD_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_NET_DSA_CMD_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_NET_DSA_CMD_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_NET_DSA_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_NET_DSA_PARAMS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_NET_DSA_PARAMS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_NET_RESOURCE_ERROR_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_NET_RESOURCE_ERROR_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_NET_RESOURCE_ERROR_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_NET_RESOURCE_ERROR_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_NET_RESOURCE_ERROR_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_NET_RESOURCE_ERROR_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_NET_RX_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_NET_RX_PARAMS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_NET_RX_PARAMS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_NET_SDMA_RX_COUNTERS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_NET_SDMA_RX_COUNTERS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_NET_SDMA_RX_COUNTERS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_NET_TO_CPU_FLOW_ID_OR_TT_OFFSET_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_NET_TO_CPU_FLOW_ID_OR_TT_OFFSET_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_NET_TO_CPU_FLOW_ID_OR_TT_OFFSET_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_NET_TO_CPU_FLOW_ID_OR_TT_OFFSET_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_NET_TO_CPU_FLOW_ID_OR_TT_OFFSET_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_NET_TO_CPU_FLOW_ID_OR_TT_OFFSET_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_NET_TX_GENERATOR_BURST_STATUS_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_NET_TX_GENERATOR_BURST_STATUS_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_NET_TX_GENERATOR_BURST_STATUS_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_NET_TX_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_NET_TX_PARAMS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_NET_TX_PARAMS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_numOfBuffPtr = {
     "numOfBuffPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_NETIF_APP_SPECIFIC_CPUCODE_TCP_UDP_PORT_RANGE_STC_PTR_l4TypeInfoPtr = {
     "l4TypeInfoPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_NETIF_APP_SPECIFIC_CPUCODE_TCP_UDP_PORT_RANGE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_NETIF_MII_INIT_STC_PTR_miiInitPtr = {
     "miiInitPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_NETIF_MII_INIT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC_PTR_entryInfoPtr = {
     "entryInfoPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_NET_DSA_PARAMS_STC_PTR_dsaInfoPtr = {
     "dsaInfoPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_NET_DSA_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_NET_RESOURCE_ERROR_MODE_ENT_mode = {
     "mode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_NET_RESOURCE_ERROR_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_NET_TO_CPU_FLOW_ID_OR_TT_OFFSET_MODE_ENT_mode = {
     "mode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_NET_TO_CPU_FLOW_ID_OR_TT_OFFSET_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT_rateMode = {
     "rateMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_NET_TX_PARAMS_STC_PTR_packetParamsPtr = {
     "packetParamsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_NET_TX_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_NET_TX_PARAMS_STC_PTR_pcktParamsPtr = {
     "pcktParamsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_NET_TX_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_burstEnable = {
     "burstEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_HW_DEV_NUM_designatedHwDevNum = {
     "designatedHwDevNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_HW_DEV_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_PTR_buffLenList = {
     "buffLenList", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_PTR_rxBuffSizeList = {
     "rxBuffSizeList", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_buffListLen = {
     "buffListLen", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_burstPacketsNumber = {
     "burstPacketsNumber", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_dropCntrVal = {
     "dropCntrVal", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numOfBufs = {
     "numOfBufs", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_packetDataLength = {
     "packetDataLength", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_packetId = {
     "packetId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_pktLimit = {
     "pktLimit", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_rangeIndex = {
     "rangeIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_rateLimiterIndex = {
     "rateLimiterIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_statisticalRateLimit = {
     "statisticalRateLimit", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_windowResolution = {
     "windowResolution", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U64_PTR_rateValue = {
     "rateValue", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U64)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_PTR_PTR_buffList = {
     "buffList", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_PTR_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_PTR_PTR_rxBuffList = {
     "rxBuffList", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_PTR_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_PTR_dsaBytesPtr = {
     "dsaBytesPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_PTR_packetDataPtr = {
     "packetDataPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_queue = {
     "queue", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_queueIdx = {
     "queueIdx", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_rxQueue = {
     "rxQueue", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_txQueue = {
     "txQueue", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_UINTPTR_hndl = {
     "hndl", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_UINTPTR)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_NETIF_APP_SPECIFIC_CPUCODE_TCP_UDP_PORT_RANGE_STC_PTR_l4TypeInfoPtr = {
     "l4TypeInfoPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_NETIF_APP_SPECIFIC_CPUCODE_TCP_UDP_PORT_RANGE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC_PTR_entryInfoPtr = {
     "entryInfoPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_NET_DSA_PARAMS_STC_PTR_dsaInfoPtr = {
     "dsaInfoPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_NET_DSA_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_NET_RESOURCE_ERROR_MODE_ENT_PTR_modePtr = {
     "modePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_NET_RESOURCE_ERROR_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_NET_RX_PARAMS_STC_PTR_rxParamsPtr = {
     "rxParamsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_NET_RX_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_NET_SDMA_RX_COUNTERS_STC_PTR_rxCountersPtr = {
     "rxCountersPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_NET_SDMA_RX_COUNTERS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_NET_TO_CPU_FLOW_ID_OR_TT_OFFSET_MODE_ENT_PTR_modePtr = {
     "modePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_NET_TO_CPU_FLOW_ID_OR_TT_OFFSET_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_NET_TX_GENERATOR_BURST_STATUS_ENT_PTR_burstStatusPtr = {
     "burstStatusPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_NET_TX_GENERATOR_BURST_STATUS_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT_PTR_rateModePtr = {
     "rateModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_NET_SDMA_RX_ERROR_COUNTERS_STC_PTR_rxErrCountPtr = {
     "rxErrCountPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_NET_SDMA_RX_ERROR_COUNTERS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_HW_DEV_NUM_PTR_designatedHwDevNumPtr = {
     "designatedHwDevNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_HW_DEV_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_STATUS_PTR_statusPtr = {
     "statusPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_STATUS)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_buffLenArr = {
     "buffLenArr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_dropCntrPtr = {
     "dropCntrPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_numberOfFreeTxDescriptorsPtr = {
     "numberOfFreeTxDescriptorsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_packetCntrPtr = {
     "packetCntrPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_packetIdPtr = {
     "packetIdPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_pktLimitPtr = {
     "pktLimitPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_statisticalRateLimitPtr = {
     "statisticalRateLimitPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_windowResolutionPtr = {
     "windowResolutionPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U64_PTR_actualRateValuePtr = {
     "actualRateValuePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U64)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U64_PTR_rateValuePtr = {
     "rateValuePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U64)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_PTR_packetBuffsArrPtr = {
     "packetBuffsArrPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_PTR_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_devPtr = {
     "devPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_dsaBytesPtr = {
     "dsaBytesPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_firstQueuePtr = {
     "firstQueuePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_protocolPtr = {
     "protocolPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_queuePtr = {
     "queuePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U8)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfFromCpuDpSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DP_LEVEL_ENT_dpLevel
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfMiiInit_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_NETIF_MII_INIT_STC_PTR_miiInitPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfToCpuFlowIdOrTtOffsetModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_NET_TO_CPU_FLOW_ID_OR_TT_OFFSET_MODE_ENT_mode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfCpuCodeIpLinkLocalProtSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_IP_PROTOCOL_STACK_ENT_ipVer,
    &DX_IN_GT_U8_protocol,
    &DX_IN_CPSS_NET_RX_CPU_CODE_ENT_cpuCode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfCpuCodeIpLinkLocalProtGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_IP_PROTOCOL_STACK_ENT_ipVer,
    &DX_IN_GT_U8_protocol,
    &DX_OUT_CPSS_NET_RX_CPU_CODE_ENT_PTR_cpuCodePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfCpuCodeTableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_NET_RX_CPU_CODE_ENT_cpuCode,
    &DX_IN_CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC_PTR_entryInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfCpuCodeToPhysicalPortSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_NET_RX_CPU_CODE_ENT_cpuCode,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfCpuCodeTableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_NET_RX_CPU_CODE_ENT_cpuCode,
    &DX_OUT_CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC_PTR_entryInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfCpuCodeToPhysicalPortGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_NET_RX_CPU_CODE_ENT_cpuCode,
    &DX_OUT_GT_PHYSICAL_PORT_NUM_PTR_portNumPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfSdmaPhysicalPortToQueueGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_U8_PTR_firstQueuePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_dropCntrVal
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfPortGroupCpuCodeRateLimiterTableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_rateLimiterIndex,
    &DX_IN_GT_U32_windowSize,
    &DX_IN_GT_U32_pktLimit
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfPortGroupCpuCodeRateLimiterPacketCntrGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_rateLimiterIndex,
    &DX_OUT_GT_U32_PTR_packetCntrPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfPortGroupCpuCodeRateLimiterTableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_rateLimiterIndex,
    &DX_OUT_GT_U32_PTR_windowSizePtr,
    &DX_OUT_GT_U32_PTR_pktLimitPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_windowResolution
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfSdmaTxGeneratorDisable_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U8_txQueue
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfSdmaTxGeneratorRateSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U8_txQueue,
    &DX_IN_CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT_rateMode,
    &DX_IN_GT_U64_PTR_rateValue,
    &DX_OUT_GT_U64_PTR_actualRateValuePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfSdmaTxGeneratorEnable_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U8_txQueue,
    &DX_IN_GT_BOOL_burstEnable,
    &DX_IN_GT_U32_burstPacketsNumber
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfSdmaTxGeneratorPacketRemove_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U8_txQueue,
    &DX_IN_GT_U32_packetId
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfSdmaTxGeneratorBurstStatusGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U8_txQueue,
    &DX_OUT_CPSS_DXCH_NET_TX_GENERATOR_BURST_STATUS_ENT_PTR_burstStatusPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfSdmaTxGeneratorRateGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U8_txQueue,
    &DX_OUT_CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT_PTR_rateModePtr,
    &DX_OUT_GT_U64_PTR_rateValuePtr,
    &DX_OUT_GT_U64_PTR_actualRateValuePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_OUT_GT_U32_PTR_dropCntrPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_OUT_GT_U32_PTR_windowResolutionPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfCpuCodeRateLimiterDropCntrSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_dropCntrVal
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfCpuCodeDesignatedDeviceTableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_IN_GT_HW_DEV_NUM_designatedHwDevNum
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfCpuCodeStatisticalRateLimitsTableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_IN_GT_U32_statisticalRateLimit
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfAppSpecificCpuCodeIpProtocolSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_IN_GT_U8_protocol,
    &DX_IN_CPSS_NET_RX_CPU_CODE_ENT_cpuCode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfAppSpecificCpuCodeIpProtocolGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_OUT_GT_BOOL_PTR_validPtr,
    &DX_OUT_GT_U8_PTR_protocolPtr,
    &DX_OUT_CPSS_NET_RX_CPU_CODE_ENT_PTR_cpuCodePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfCpuCodeDesignatedDeviceTableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_OUT_GT_HW_DEV_NUM_PTR_designatedHwDevNumPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfCpuCodeStatisticalRateLimitsTableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_OUT_GT_U32_PTR_statisticalRateLimitPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeInvalidate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_rangeIndex
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfAppSpecificCpuCodeTcpUdpPortRangeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_rangeIndex,
    &DX_IN_CPSS_DXCH_NETIF_APP_SPECIFIC_CPUCODE_TCP_UDP_PORT_RANGE_STC_PTR_l4TypeInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfAppSpecificCpuCodeTcpUdpPortRangeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_rangeIndex,
    &DX_OUT_GT_BOOL_PTR_validPtr,
    &DX_OUT_CPSS_DXCH_NETIF_APP_SPECIFIC_CPUCODE_TCP_UDP_PORT_RANGE_STC_PTR_l4TypeInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfCpuCodeRateLimiterTableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_rateLimiterIndex,
    &DX_IN_GT_U32_windowSize,
    &DX_IN_GT_U32_pktLimit
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfCpuCodeRateLimiterPacketCntrGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_rateLimiterIndex,
    &DX_OUT_GT_U32_PTR_packetCntrPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfCpuCodeRateLimiterTableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_rateLimiterIndex,
    &DX_OUT_GT_U32_PTR_windowSizePtr,
    &DX_OUT_GT_U32_PTR_pktLimitPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfCpuCodeRateLimiterWindowResolutionSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_windowResolution
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfCpuCodeIeeeReservedMcastProtSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U8_protocol,
    &DX_IN_CPSS_NET_RX_CPU_CODE_ENT_cpuCode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfCpuCodeIeeeReservedMcastProtGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U8_protocol,
    &DX_OUT_CPSS_NET_RX_CPU_CODE_ENT_PTR_cpuCodePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfSdmaRxResourceErrorModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U8_queue,
    &DX_IN_CPSS_DXCH_NET_RESOURCE_ERROR_MODE_ENT_mode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfSdmaRxQueueEnable_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U8_queue,
    &DX_IN_GT_BOOL_enable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfSdmaRxResourceErrorModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U8_queue,
    &DX_OUT_CPSS_DXCH_NET_RESOURCE_ERROR_MODE_ENT_PTR_modePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfSdmaRxQueueEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U8_queue,
    &DX_OUT_GT_BOOL_PTR_enablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfSdmaRxCountersGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U8_queueIdx,
    &DX_OUT_CPSS_DXCH_NET_SDMA_RX_COUNTERS_STC_PTR_rxCountersPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfSdmaQueueToPhysicalPortGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U8_queueIdx,
    &DX_OUT_GT_PHYSICAL_PORT_NUM_PTR_portNumPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfSdmaTxBufferQueueDescFreeAndCookieGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U8_txQueue,
    &DX_OUT_GT_PTR_PTR_cookiePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfSdmaTxFreeDescripotrsNumberGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U8_txQueue,
    &DX_OUT_GT_U32_PTR_numberOfFreeTxDescriptorsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfFromCpuDpGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DP_LEVEL_ENT_PTR_dpLevelPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfToCpuFlowIdOrTtOffsetModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_NET_TO_CPU_FLOW_ID_OR_TT_OFFSET_MODE_ENT_PTR_modePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfSdmaRxErrorCountGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_NET_SDMA_RX_ERROR_COUNTERS_STC_PTR_rxErrCountPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfCpuCodeRateLimiterDropCntrGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_dropCntrPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfCpuCodeRateLimiterWindowResolutionGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_windowResolutionPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfMiiTxBufferQueueGet_PARAMS[] =  {
    &DX_IN_GT_UINTPTR_hndl,
    &DX_OUT_GT_U8_PTR_devPtr,
    &DX_OUT_GT_PTR_PTR_cookiePtr,
    &DX_OUT_GT_U8_PTR_queuePtr,
    &DX_OUT_GT_STATUS_PTR_statusPtr
};
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfDsaTagBuild_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfDsaTagParse_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfMiiRxBufFree_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfMiiRxPacketGet_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfMiiSyncTxPacketSend_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfMiiTxPacketSend_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfRxBufFree_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfRxBufFreeWithSize_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfSdmaRxPacketGet_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfSdmaSyncTxPacketSend_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfSdmaTxGeneratorPacketAdd_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfSdmaTxGeneratorPacketUpdate_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNetIfSdmaTxPacketSend_PARAMS[];


/********* lib API DB *********/

extern void cpssDxChNetIfSdmaTxPacketSend_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChNetIfSdmaTxPacketSend_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChNetIfSdmaRxPacketGet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChNetIfRxBufFree_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChNetIfRxBufFree_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChNetIfRxBufFreeWithSize_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChNetIfSdmaRxPacketGet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChNetIfSdmaTxPacketSend_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChNetIfSdmaTxPacketSend_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChNetIfSdmaTxGeneratorPacketAdd_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChNetIfSdmaTxGeneratorPacketUpdate_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChNetworkIfLogLibDb[] = {
    {"cpssDxChNetIfDsaTagParse", 3, cpssDxChNetIfDsaTagParse_PARAMS, NULL},
    {"cpssDxChNetIfDsaTagBuild", 3, cpssDxChNetIfDsaTagBuild_PARAMS, NULL},
    {"cpssDxChNetIfDuplicateEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChNetIfDuplicateEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChNetIfPortDuplicateToCpuSet", 3, prvCpssLogGenDevNumPortNumEnable_PARAMS, NULL},
    {"cpssDxChNetIfPortDuplicateToCpuGet", 3, prvCpssLogGenDevNumPortNumEnablePtr_PARAMS, NULL},
    {"cpssDxChNetIfAppSpecificCpuCodeTcpSynSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChNetIfAppSpecificCpuCodeTcpSynGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChNetIfAppSpecificCpuCodeIpProtocolSet", 4, cpssDxChNetIfAppSpecificCpuCodeIpProtocolSet_PARAMS, NULL},
    {"cpssDxChNetIfAppSpecificCpuCodeIpProtocolInvalidate", 2, prvCpssLogGenDevNumIndex_PARAMS, NULL},
    {"cpssDxChNetIfAppSpecificCpuCodeIpProtocolGet", 5, cpssDxChNetIfAppSpecificCpuCodeIpProtocolGet_PARAMS, NULL},
    {"cpssDxChNetIfAppSpecificCpuCodeTcpUdpPortRangeSet", 3, cpssDxChNetIfAppSpecificCpuCodeTcpUdpPortRangeSet_PARAMS, NULL},
    {"cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeInvalidate", 2, cpssDxChNetIfAppSpecificCpuCodeTcpUdpDestPortRangeInvalidate_PARAMS, NULL},
    {"cpssDxChNetIfAppSpecificCpuCodeTcpUdpPortRangeGet", 4, cpssDxChNetIfAppSpecificCpuCodeTcpUdpPortRangeGet_PARAMS, NULL},
    {"cpssDxChNetIfCpuCodeIpLinkLocalProtSet", 4, cpssDxChNetIfCpuCodeIpLinkLocalProtSet_PARAMS, NULL},
    {"cpssDxChNetIfCpuCodeIpLinkLocalProtGet", 4, cpssDxChNetIfCpuCodeIpLinkLocalProtGet_PARAMS, NULL},
    {"cpssDxChNetIfCpuCodeIeeeReservedMcastProtSet", 3, cpssDxChNetIfCpuCodeIeeeReservedMcastProtSet_PARAMS, NULL},
    {"cpssDxChNetIfCpuCodeIeeeReservedMcastProtGet", 3, cpssDxChNetIfCpuCodeIeeeReservedMcastProtGet_PARAMS, NULL},
    {"cpssDxChNetIfCpuCodeTableSet", 3, cpssDxChNetIfCpuCodeTableSet_PARAMS, NULL},
    {"cpssDxChNetIfCpuCodeTableGet", 3, cpssDxChNetIfCpuCodeTableGet_PARAMS, NULL},
    {"cpssDxChNetIfCpuCodeStatisticalRateLimitsTableSet", 3, cpssDxChNetIfCpuCodeStatisticalRateLimitsTableSet_PARAMS, NULL},
    {"cpssDxChNetIfCpuCodeStatisticalRateLimitsTableGet", 3, cpssDxChNetIfCpuCodeStatisticalRateLimitsTableGet_PARAMS, NULL},
    {"cpssDxChNetIfCpuCodeDesignatedDeviceTableSet", 3, cpssDxChNetIfCpuCodeDesignatedDeviceTableSet_PARAMS, NULL},
    {"cpssDxChNetIfCpuCodeDesignatedDeviceTableGet", 3, cpssDxChNetIfCpuCodeDesignatedDeviceTableGet_PARAMS, NULL},
    {"cpssDxChNetIfCpuCodeRateLimiterTableSet", 4, cpssDxChNetIfCpuCodeRateLimiterTableSet_PARAMS, NULL},
    {"cpssDxChNetIfCpuCodeRateLimiterTableGet", 4, cpssDxChNetIfCpuCodeRateLimiterTableGet_PARAMS, NULL},
    {"cpssDxChNetIfCpuCodeRateLimiterWindowResolutionSet", 2, cpssDxChNetIfCpuCodeRateLimiterWindowResolutionSet_PARAMS, NULL},
    {"cpssDxChNetIfCpuCodeRateLimiterWindowResolutionGet", 2, cpssDxChNetIfCpuCodeRateLimiterWindowResolutionGet_PARAMS, NULL},
    {"cpssDxChNetIfCpuCodeRateLimiterPacketCntrGet", 3, cpssDxChNetIfCpuCodeRateLimiterPacketCntrGet_PARAMS, NULL},
    {"cpssDxChNetIfCpuCodeRateLimiterDropCntrGet", 2, cpssDxChNetIfCpuCodeRateLimiterDropCntrGet_PARAMS, NULL},
    {"cpssDxChNetIfCpuCodeRateLimiterDropCntrSet", 2, cpssDxChNetIfCpuCodeRateLimiterDropCntrSet_PARAMS, NULL},
    {"cpssDxChNetIfFromCpuDpSet", 2, cpssDxChNetIfFromCpuDpSet_PARAMS, NULL},
    {"cpssDxChNetIfFromCpuDpGet", 2, cpssDxChNetIfFromCpuDpGet_PARAMS, NULL},
    {"cpssDxChNetIfSdmaRxResourceErrorModeSet", 3, cpssDxChNetIfSdmaRxResourceErrorModeSet_PARAMS, NULL},
    {"cpssDxChNetIfSdmaRxResourceErrorModeGet", 3, cpssDxChNetIfSdmaRxResourceErrorModeGet_PARAMS, NULL},
    {"cpssDxChNetIfPortGroupCpuCodeRateLimiterTableSet", 5, cpssDxChNetIfPortGroupCpuCodeRateLimiterTableSet_PARAMS, NULL},
    {"cpssDxChNetIfPortGroupCpuCodeRateLimiterTableGet", 5, cpssDxChNetIfPortGroupCpuCodeRateLimiterTableGet_PARAMS, NULL},
    {"cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionSet", 3, cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionSet_PARAMS, NULL},
    {"cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionGet", 3, cpssDxChNetIfPortGroupCpuCodeRateLimiterWindowResolutionGet_PARAMS, NULL},
    {"cpssDxChNetIfPortGroupCpuCodeRateLimiterPacketCntrGet", 4, cpssDxChNetIfPortGroupCpuCodeRateLimiterPacketCntrGet_PARAMS, NULL},
    {"cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrGet", 3, cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrGet_PARAMS, NULL},
    {"cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrSet", 3, cpssDxChNetIfPortGroupCpuCodeRateLimiterDropCntrSet_PARAMS, NULL},
    {"cpssDxChNetIfToCpuFlowIdOrTtOffsetModeSet", 2, cpssDxChNetIfToCpuFlowIdOrTtOffsetModeSet_PARAMS, NULL},
    {"cpssDxChNetIfToCpuFlowIdOrTtOffsetModeGet", 2, cpssDxChNetIfToCpuFlowIdOrTtOffsetModeGet_PARAMS, NULL},
    {"cpssDxChNetIfCpuCodeToPhysicalPortSet", 3, cpssDxChNetIfCpuCodeToPhysicalPortSet_PARAMS, NULL},
    {"cpssDxChNetIfCpuCodeToPhysicalPortGet", 3, cpssDxChNetIfCpuCodeToPhysicalPortGet_PARAMS, NULL},
    {"cpssDxChNetIfSdmaPhysicalPortToQueueGet", 3, cpssDxChNetIfSdmaPhysicalPortToQueueGet_PARAMS, NULL},
    {"cpssDxChNetIfSdmaQueueToPhysicalPortGet", 3, cpssDxChNetIfSdmaQueueToPhysicalPortGet_PARAMS, NULL},
    {"cpssDxChNetIfInit", 1, prvCpssLogGenDevNum_PARAMS, NULL},
    {"cpssDxChNetIfRemove", 1, prvCpssLogGenDevNum_PARAMS, NULL},
    {"cpssDxChNetIfRestore", 1, prvCpssLogGenDevNum_PARAMS, NULL},
    {"cpssDxChNetIfMiiTxPacketSend", 5, cpssDxChNetIfMiiTxPacketSend_PARAMS, cpssDxChNetIfSdmaTxPacketSend_preLogic},
    {"cpssDxChNetIfMiiSyncTxPacketSend", 5, cpssDxChNetIfMiiSyncTxPacketSend_PARAMS, cpssDxChNetIfSdmaTxPacketSend_preLogic},
    {"cpssDxChNetIfMiiTxBufferQueueGet", 5, cpssDxChNetIfMiiTxBufferQueueGet_PARAMS, NULL},
    {"cpssDxChNetIfMiiRxPacketGet", 6, cpssDxChNetIfMiiRxPacketGet_PARAMS, cpssDxChNetIfSdmaRxPacketGet_preLogic},
    {"cpssDxChNetIfMiiRxBufFree", 4, cpssDxChNetIfMiiRxBufFree_PARAMS, cpssDxChNetIfRxBufFree_preLogic},
    {"cpssDxChNetIfMiiInit", 2, cpssDxChNetIfMiiInit_PARAMS, NULL},
    {"cpssDxChNetIfRxBufFree", 4, cpssDxChNetIfRxBufFree_PARAMS, cpssDxChNetIfRxBufFree_preLogic},
    {"cpssDxChNetIfRxBufFreeWithSize", 5, cpssDxChNetIfRxBufFreeWithSize_PARAMS, cpssDxChNetIfRxBufFreeWithSize_preLogic},
    {"cpssDxChNetIfSdmaRxPacketGet", 6, cpssDxChNetIfSdmaRxPacketGet_PARAMS, cpssDxChNetIfSdmaRxPacketGet_preLogic},
    {"cpssDxChNetIfPrePendTwoBytesHeaderSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChNetIfPrePendTwoBytesHeaderGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChNetIfSdmaRxCountersGet", 3, cpssDxChNetIfSdmaRxCountersGet_PARAMS, NULL},
    {"cpssDxChNetIfSdmaRxErrorCountGet", 2, cpssDxChNetIfSdmaRxErrorCountGet_PARAMS, NULL},
    {"cpssDxChNetIfSdmaRxQueueEnable", 3, cpssDxChNetIfSdmaRxQueueEnable_PARAMS, NULL},
    {"cpssDxChNetIfSdmaRxQueueEnableGet", 3, cpssDxChNetIfSdmaRxQueueEnableGet_PARAMS, NULL},
    {"cpssDxChNetIfSdmaTxPacketSend", 5, cpssDxChNetIfSdmaTxPacketSend_PARAMS, cpssDxChNetIfSdmaTxPacketSend_preLogic},
    {"cpssDxChNetIfSdmaSyncTxPacketSend", 5, cpssDxChNetIfSdmaSyncTxPacketSend_PARAMS, cpssDxChNetIfSdmaTxPacketSend_preLogic},
    {"cpssDxChNetIfSdmaTxQueueEnable", 3, cpssDxChNetIfSdmaRxQueueEnable_PARAMS, NULL},
    {"cpssDxChNetIfSdmaTxQueueEnableGet", 3, cpssDxChNetIfSdmaRxQueueEnableGet_PARAMS, NULL},
    {"cpssDxChNetIfTxBufferQueueGet", 5, cpssDxChNetIfMiiTxBufferQueueGet_PARAMS, NULL},
    {"cpssDxChNetIfSdmaTxBufferQueueDescFreeAndCookieGet", 3, cpssDxChNetIfSdmaTxBufferQueueDescFreeAndCookieGet_PARAMS, NULL},
    {"cpssDxChNetIfSdmaTxFreeDescripotrsNumberGet", 3, cpssDxChNetIfSdmaTxFreeDescripotrsNumberGet_PARAMS, NULL},
    {"cpssDxChNetIfSdmaTxGeneratorPacketAdd", 6, cpssDxChNetIfSdmaTxGeneratorPacketAdd_PARAMS, cpssDxChNetIfSdmaTxGeneratorPacketAdd_preLogic},
    {"cpssDxChNetIfSdmaTxGeneratorPacketUpdate", 6, cpssDxChNetIfSdmaTxGeneratorPacketUpdate_PARAMS, cpssDxChNetIfSdmaTxGeneratorPacketUpdate_preLogic},
    {"cpssDxChNetIfSdmaTxGeneratorPacketRemove", 4, cpssDxChNetIfSdmaTxGeneratorPacketRemove_PARAMS, NULL},
    {"cpssDxChNetIfSdmaTxGeneratorRateSet", 6, cpssDxChNetIfSdmaTxGeneratorRateSet_PARAMS, NULL},
    {"cpssDxChNetIfSdmaTxGeneratorRateGet", 6, cpssDxChNetIfSdmaTxGeneratorRateGet_PARAMS, NULL},
    {"cpssDxChNetIfSdmaTxGeneratorEnable", 5, cpssDxChNetIfSdmaTxGeneratorEnable_PARAMS, NULL},
    {"cpssDxChNetIfSdmaTxGeneratorDisable", 3, cpssDxChNetIfSdmaTxGeneratorDisable_PARAMS, NULL},
    {"cpssDxChNetIfSdmaTxGeneratorBurstStatusGet", 4, cpssDxChNetIfSdmaTxGeneratorBurstStatusGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_NETWORK_IF(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChNetworkIfLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChNetworkIfLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

