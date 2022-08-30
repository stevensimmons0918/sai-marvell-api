/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChCncLog.c
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
#include <cpss/dxCh/dxChxGen/cnc/cpssDxChCnc.h>
#include <cpss/dxCh/dxChxGen/cnc/private/prvCpssDxChCncLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssGenDbLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT[]  =
{
    "CPSS_DXCH_CNC_BYTE_COUNT_MODE_L2_E",
    "CPSS_DXCH_CNC_BYTE_COUNT_MODE_L3_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_CNC_CLIENT_ENT[]  =
{
    "CPSS_DXCH_CNC_CLIENT_L2L3_INGRESS_VLAN_E",
    "CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E",
    "CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_1_E",
    "CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_E",
    "CPSS_DXCH_CNC_CLIENT_INGRESS_VLAN_PASS_DROP_E",
    "CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_PASS_DROP_E",
    "CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E",
    "CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_E",
    "CPSS_DXCH_CNC_CLIENT_ARP_TABLE_ACCESS_E",
    "CPSS_DXCH_CNC_CLIENT_TUNNEL_START_E",
    "CPSS_DXCH_CNC_CLIENT_TTI_E",
    "CPSS_DXCH_CNC_CLIENT_INGRESS_SRC_EPORT_E",
    "CPSS_DXCH_CNC_CLIENT_EGRESS_TRG_EPORT_E",
    "CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_0_E",
    "CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_1_E",
    "CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_0_E",
    "CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_1_E",
    "CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_2_E",
    "CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_3_E",
    "CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_0_E",
    "CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_1_E",
    "CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_2_E",
    "CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_1_PARALLEL_3_E",
    "CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_0_E",
    "CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_1_E",
    "CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_2_E",
    "CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_2_PARALLEL_3_E",
    "CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_0_E",
    "CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_1_E",
    "CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_2_E",
    "CPSS_DXCH_CNC_CLIENT_EGRESS_PCL_PARALLEL_3_E",
    "CPSS_DXCH_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E",
    "CPSS_DXCH_CNC_CLIENT_TM_PASS_DROP_E",
    "CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_2_E",
    "CPSS_DXCH_CNC_CLIENT_TTI_PARALLEL_3_E",
    "CPSS_DXCH_CNC_CLIENT_EGRESS_VLAN_EGRESS_FILTER_PASS_DROP_E",
    "CPSS_DXCH_CNC_CLIENT_EGRESS_PACKET_TYPE_PASS_DROP_E",
    "CPSS_DXCH_CNC_CLIENT_QUEUE_STAT_E",
    "CPSS_DXCH_CNC_CLIENT_PORT_STAT_E",
    "CPSS_DXCH_CNC_CLIENT_PHA_E",
    "CPSS_DXCH_CNC_CLIENT_LAST_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_CNC_CLIENT_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_CNC_COUNTER_FORMAT_ENT[]  =
{
    "CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E",
    "CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_1_E",
    "CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_2_E",
    "CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_3_E",
    "CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_4_E",
    "CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_5_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_CNC_COUNTER_FORMAT_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_ENT[]  =
{
    "CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_TTI_E",
    "CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_PCL_E",
    "CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_EGRESS_FILTER_PASS_DROP_E",
    "CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_PHA_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_ENT[]  =
{
    "CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_EGRESS_FILTER_AND_TAIL_DROP_E",
    "CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_EGRESS_FILTER_ONLY_E",
    "CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_TAIL_DROP_ONLY_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT[]  =
{
    "CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_E",
    "CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_CN_E",
    "CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_REDUCED_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT[]  =
{
    "CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_SRC_PORT_E",
    "CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_CPU_CODE_E",
    "CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_STREAM_GATE_MODE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_CNC_TM_INDEX_MODE_ENT[]  =
{
    "CPSS_DXCH_CNC_TM_INDEX_MODE_0_E",
    "CPSS_DXCH_CNC_TM_INDEX_MODE_1_E",
    "CPSS_DXCH_CNC_TM_INDEX_MODE_2_E",
    "CPSS_DXCH_CNC_TM_INDEX_MODE_3_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_CNC_TM_INDEX_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_CNC_VLAN_INDEX_MODE_ENT[]  =
{
    "CPSS_DXCH_CNC_VLAN_INDEX_MODE_ORIGINAL_VID_E",
    "CPSS_DXCH_CNC_VLAN_INDEX_MODE_EVID_E",
    "CPSS_DXCH_CNC_VLAN_INDEX_MODE_TAG1_VID_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_CNC_VLAN_INDEX_MODE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_CNC_COUNTER_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_CNC_COUNTER_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, byteCount, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, packetCount, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, maxValue, GT_U64);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_CNC_CLIENT_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CNC_CLIENT_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_CNC_CLIENT_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_CNC_COUNTER_FORMAT_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CNC_COUNTER_FORMAT_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_CNC_COUNTER_FORMAT_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_CNC_COUNTER_FORMAT_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CNC_COUNTER_FORMAT_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_CNC_COUNTER_FORMAT_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_CNC_TM_INDEX_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CNC_TM_INDEX_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_CNC_TM_INDEX_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_CNC_TM_INDEX_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CNC_TM_INDEX_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_CNC_TM_INDEX_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_CNC_VLAN_INDEX_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CNC_VLAN_INDEX_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_CNC_VLAN_INDEX_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_CNC_VLAN_INDEX_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CNC_VLAN_INDEX_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_CNC_VLAN_INDEX_MODE_ENT);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_indexNumPtr = {
     "indexNumPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_numOfCounterValuesPtr = {
     "numOfCounterValuesPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT_countMode = {
     "countMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CNC_CLIENT_ENT_client = {
     "client", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_CNC_CLIENT_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CNC_CLIENT_ENT_vlanClient = {
     "vlanClient", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_CNC_CLIENT_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CNC_COUNTER_FORMAT_ENT_format = {
     "format", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_CNC_COUNTER_FORMAT_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CNC_COUNTER_STC_PTR_counterPtr = {
     "counterPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CNC_COUNTER_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_ENT_cncUnit = {
     "cncUnit", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_ENT_mode = {
     "mode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT_mode = {
     "mode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT_toCpuMode = {
     "toCpuMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CNC_TM_INDEX_MODE_ENT_indexMode = {
     "indexMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_CNC_TM_INDEX_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CNC_VLAN_INDEX_MODE_ENT_indexMode = {
     "indexMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_CNC_VLAN_INDEX_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_updateEnable = {
     "updateEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_blockNum = {
     "blockNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_portLimit = {
     "portLimit", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_queueBase = {
     "queueBase", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_queueLimit = {
     "queueLimit", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U64_HEX_PTR_indexRangesBmpPtr = {
     "indexRangesBmpPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U64_HEX)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT_PTR_countModePtr = {
     "countModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CNC_COUNTER_FORMAT_ENT_PTR_formatPtr = {
     "formatPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CNC_COUNTER_FORMAT_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CNC_COUNTER_STC_PTR_counterPtr = {
     "counterPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CNC_COUNTER_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CNC_COUNTER_STC_PTR_counterValuesPtr = {
     "counterValuesPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CNC_COUNTER_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_ENT_PTR_modePtr = {
     "modePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT_PTR_modePtr = {
     "modePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT_PTR_toCpuModePtr = {
     "toCpuModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CNC_TM_INDEX_MODE_ENT_PTR_indexModePtr = {
     "indexModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CNC_TM_INDEX_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CNC_VLAN_INDEX_MODE_ENT_PTR_indexModePtr = {
     "indexModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CNC_VLAN_INDEX_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_updateEnablePtr = {
     "updateEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_inProcessBlocksBmpPtr = {
     "inProcessBlocksBmpPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_portGroupIdArr = {
     "portGroupIdArr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_portLimitPtr = {
     "portLimitPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_queueBasePtr = {
     "queueBasePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_queueLimitPtr = {
     "queueLimitPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U64_HEX_PTR_indexRangesBmpPtr = {
     "indexRangesBmpPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U64_HEX)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncClientByteCountModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_CNC_CLIENT_ENT_client,
    &DX_IN_CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT_countMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncClientByteCountModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_CNC_CLIENT_ENT_client,
    &DX_OUT_CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT_PTR_countModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncVlanClientIndexModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_CNC_CLIENT_ENT_vlanClient,
    &DX_IN_CPSS_DXCH_CNC_VLAN_INDEX_MODE_ENT_indexMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncVlanClientIndexModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_CNC_CLIENT_ENT_vlanClient,
    &DX_OUT_CPSS_DXCH_CNC_VLAN_INDEX_MODE_ENT_PTR_indexModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncCounterClearByReadValueSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_CNC_COUNTER_FORMAT_ENT_format,
    &DX_IN_CPSS_DXCH_CNC_COUNTER_STC_PTR_counterPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncCounterClearByReadValueGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_CNC_COUNTER_FORMAT_ENT_format,
    &DX_OUT_CPSS_DXCH_CNC_COUNTER_STC_PTR_counterPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncCountingEnableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_ENT_cncUnit,
    &DX_IN_GT_BOOL_enable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncCountingEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_ENT_cncUnit,
    &DX_OUT_GT_BOOL_PTR_enablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncEgressVlanDropCountModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_ENT_mode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncEgressQueueClientModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT_mode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncPacketTypePassDropToCpuModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT_toCpuMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncTmClientIndexModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_CNC_TM_INDEX_MODE_ENT_indexMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncPortStatusLimitSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_PORT_TX_DROP_PROFILE_SET_ENT_profileSet,
    &DX_IN_GT_U32_portLimit
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncQueueStatusLimitSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_PORT_TX_DROP_PROFILE_SET_ENT_profileSet,
    &DX_IN_GT_U8_tcQueue,
    &DX_IN_GT_U32_queueLimit
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncQueueStatusLimitGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_PORT_TX_DROP_PROFILE_SET_ENT_profileSet,
    &DX_IN_GT_U8_tcQueue,
    &DX_OUT_GT_U32_PTR_queueLimitPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncPortStatusLimitGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_PORT_TX_DROP_PROFILE_SET_ENT_profileSet,
    &DX_OUT_GT_U32_PTR_portLimitPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncPortClientEnableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_CNC_CLIENT_ENT_client,
    &DX_IN_GT_BOOL_enable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncPortQueueGroupBaseSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_CNC_CLIENT_ENT_client,
    &DX_IN_GT_U32_queueBase
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncPortClientEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_CNC_CLIENT_ENT_client,
    &DX_OUT_GT_BOOL_PTR_enablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncPortQueueGroupBaseGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_CNC_CLIENT_ENT_client,
    &DX_OUT_GT_U32_PTR_queueBasePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncPortGroupBlockUploadTrigger_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_blockNum
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncPortGroupBlockClientEnableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_blockNum,
    &DX_IN_CPSS_DXCH_CNC_CLIENT_ENT_client,
    &DX_IN_GT_BOOL_updateEnable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncPortGroupBlockClientRangesSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_blockNum,
    &DX_IN_CPSS_DXCH_CNC_CLIENT_ENT_client,
    &DX_IN_GT_U64_HEX_PTR_indexRangesBmpPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncPortGroupBlockClientEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_blockNum,
    &DX_IN_CPSS_DXCH_CNC_CLIENT_ENT_client,
    &DX_OUT_GT_BOOL_PTR_updateEnablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncPortGroupBlockClientRangesGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_blockNum,
    &DX_IN_CPSS_DXCH_CNC_CLIENT_ENT_client,
    &DX_OUT_GT_U64_HEX_PTR_indexRangesBmpPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncPortGroupCounterFormatSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_blockNum,
    &DX_IN_CPSS_DXCH_CNC_COUNTER_FORMAT_ENT_format
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncPortGroupCounterSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_blockNum,
    &DX_IN_GT_U32_index,
    &DX_IN_CPSS_DXCH_CNC_COUNTER_FORMAT_ENT_format,
    &DX_IN_CPSS_DXCH_CNC_COUNTER_STC_PTR_counterPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncPortGroupCounterGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_blockNum,
    &DX_IN_GT_U32_index,
    &DX_IN_CPSS_DXCH_CNC_COUNTER_FORMAT_ENT_format,
    &DX_OUT_CPSS_DXCH_CNC_COUNTER_STC_PTR_counterPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncPortGroupCounterFormatGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_blockNum,
    &DX_OUT_CPSS_DXCH_CNC_COUNTER_FORMAT_ENT_PTR_formatPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncPortGroupUploadedBlockGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_INOUT_GT_U32_PTR_numOfCounterValuesPtr,
    &DX_IN_CPSS_DXCH_CNC_COUNTER_FORMAT_ENT_format,
    &DX_OUT_CPSS_DXCH_CNC_COUNTER_STC_PTR_counterValuesPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncPortGroupBlockUploadInProcessGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_OUT_GT_U32_PTR_inProcessBlocksBmpPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncBlockUploadTrigger_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_blockNum
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncBlockClientEnableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_blockNum,
    &DX_IN_CPSS_DXCH_CNC_CLIENT_ENT_client,
    &DX_IN_GT_BOOL_updateEnable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncBlockClientRangesSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_blockNum,
    &DX_IN_CPSS_DXCH_CNC_CLIENT_ENT_client,
    &DX_IN_GT_U64_HEX_PTR_indexRangesBmpPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncBlockClientEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_blockNum,
    &DX_IN_CPSS_DXCH_CNC_CLIENT_ENT_client,
    &DX_OUT_GT_BOOL_PTR_updateEnablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncBlockClientRangesGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_blockNum,
    &DX_IN_CPSS_DXCH_CNC_CLIENT_ENT_client,
    &DX_OUT_GT_U64_HEX_PTR_indexRangesBmpPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncCounterFormatSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_blockNum,
    &DX_IN_CPSS_DXCH_CNC_COUNTER_FORMAT_ENT_format
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncCounterSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_blockNum,
    &DX_IN_GT_U32_index,
    &DX_IN_CPSS_DXCH_CNC_COUNTER_FORMAT_ENT_format,
    &DX_IN_CPSS_DXCH_CNC_COUNTER_STC_PTR_counterPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncCounterGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_blockNum,
    &DX_IN_GT_U32_index,
    &DX_IN_CPSS_DXCH_CNC_COUNTER_FORMAT_ENT_format,
    &DX_OUT_CPSS_DXCH_CNC_COUNTER_STC_PTR_counterPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncCounterFormatGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_blockNum,
    &DX_OUT_CPSS_DXCH_CNC_COUNTER_FORMAT_ENT_PTR_formatPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncOffsetForNatClientSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_offset
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncUploadedBlockGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_INOUT_GT_U32_PTR_numOfCounterValuesPtr,
    &DX_IN_CPSS_DXCH_CNC_COUNTER_FORMAT_ENT_format,
    &DX_OUT_CPSS_DXCH_CNC_COUNTER_STC_PTR_counterValuesPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncEgressVlanDropCountModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_ENT_PTR_modePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncEgressQueueClientModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT_PTR_modePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncPacketTypePassDropToCpuModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT_PTR_toCpuModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncTmClientIndexModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_CNC_TM_INDEX_MODE_ENT_PTR_indexModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncBlockUploadInProcessGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_inProcessBlocksBmpPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncOffsetForNatClientGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_offsetPtr
};
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncCounterWraparoundIndexesGet_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCncPortGroupCounterWraparoundIndexesGet_PARAMS[];


/********* lib API DB *********/

extern void cpssDxChCncCounterWraparoundIndexesGet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChCncUploadedBlockGet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChCncPortGroupCounterWraparoundIndexesGet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChCncPortGroupUploadedBlockGet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChCncLogLibDb[] = {
    {"cpssDxChCncBlockClientEnableSet", 4, cpssDxChCncBlockClientEnableSet_PARAMS, NULL},
    {"cpssDxChCncBlockClientEnableGet", 4, cpssDxChCncBlockClientEnableGet_PARAMS, NULL},
    {"cpssDxChCncBlockClientRangesSet", 4, cpssDxChCncBlockClientRangesSet_PARAMS, NULL},
    {"cpssDxChCncBlockClientRangesGet", 4, cpssDxChCncBlockClientRangesGet_PARAMS, NULL},
    {"cpssDxChCncPortClientEnableSet", 4, cpssDxChCncPortClientEnableSet_PARAMS, NULL},
    {"cpssDxChCncPortClientEnableGet", 4, cpssDxChCncPortClientEnableGet_PARAMS, NULL},
    {"cpssDxChCncIngressVlanPassDropFromCpuCountEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChCncIngressVlanPassDropFromCpuCountEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChCncVlanClientIndexModeSet", 3, cpssDxChCncVlanClientIndexModeSet_PARAMS, NULL},
    {"cpssDxChCncVlanClientIndexModeGet", 3, cpssDxChCncVlanClientIndexModeGet_PARAMS, NULL},
    {"cpssDxChCncPacketTypePassDropToCpuModeSet", 2, cpssDxChCncPacketTypePassDropToCpuModeSet_PARAMS, NULL},
    {"cpssDxChCncPacketTypePassDropToCpuModeGet", 2, cpssDxChCncPacketTypePassDropToCpuModeGet_PARAMS, NULL},
    {"cpssDxChCncTmClientIndexModeSet", 2, cpssDxChCncTmClientIndexModeSet_PARAMS, NULL},
    {"cpssDxChCncTmClientIndexModeGet", 2, cpssDxChCncTmClientIndexModeGet_PARAMS, NULL},
    {"cpssDxChCncClientByteCountModeSet", 3, cpssDxChCncClientByteCountModeSet_PARAMS, NULL},
    {"cpssDxChCncClientByteCountModeGet", 3, cpssDxChCncClientByteCountModeGet_PARAMS, NULL},
    {"cpssDxChCncEgressVlanDropCountModeSet", 2, cpssDxChCncEgressVlanDropCountModeSet_PARAMS, NULL},
    {"cpssDxChCncEgressVlanDropCountModeGet", 2, cpssDxChCncEgressVlanDropCountModeGet_PARAMS, NULL},
    {"cpssDxChCncCounterClearByReadEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChCncCounterClearByReadEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChCncCounterClearByReadValueSet", 3, cpssDxChCncCounterClearByReadValueSet_PARAMS, NULL},
    {"cpssDxChCncCounterClearByReadValueGet", 3, cpssDxChCncCounterClearByReadValueGet_PARAMS, NULL},
    {"cpssDxChCncCounterWraparoundEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChCncCounterWraparoundEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChCncCounterWraparoundIndexesGet", 4, cpssDxChCncCounterWraparoundIndexesGet_PARAMS, cpssDxChCncCounterWraparoundIndexesGet_preLogic},
    {"cpssDxChCncCounterSet", 5, cpssDxChCncCounterSet_PARAMS, NULL},
    {"cpssDxChCncCounterGet", 5, cpssDxChCncCounterGet_PARAMS, NULL},
    {"cpssDxChCncBlockUploadTrigger", 2, cpssDxChCncBlockUploadTrigger_PARAMS, NULL},
    {"cpssDxChCncBlockUploadInProcessGet", 2, cpssDxChCncBlockUploadInProcessGet_PARAMS, NULL},
    {"cpssDxChCncUploadedBlockGet", 4, cpssDxChCncUploadedBlockGet_PARAMS, cpssDxChCncUploadedBlockGet_preLogic},
    {"cpssDxChCncCountingEnableSet", 3, cpssDxChCncCountingEnableSet_PARAMS, NULL},
    {"cpssDxChCncCountingEnableGet", 3, cpssDxChCncCountingEnableGet_PARAMS, NULL},
    {"cpssDxChCncCounterFormatSet", 3, cpssDxChCncCounterFormatSet_PARAMS, NULL},
    {"cpssDxChCncCounterFormatGet", 3, cpssDxChCncCounterFormatGet_PARAMS, NULL},
    {"cpssDxChCncEgressQueueClientModeSet", 2, cpssDxChCncEgressQueueClientModeSet_PARAMS, NULL},
    {"cpssDxChCncEgressQueueClientModeGet", 2, cpssDxChCncEgressQueueClientModeGet_PARAMS, NULL},
    {"cpssDxChCncCpuAccessStrictPriorityEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChCncCpuAccessStrictPriorityEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChCncPortGroupBlockClientEnableSet", 5, cpssDxChCncPortGroupBlockClientEnableSet_PARAMS, NULL},
    {"cpssDxChCncPortGroupBlockClientEnableGet", 5, cpssDxChCncPortGroupBlockClientEnableGet_PARAMS, NULL},
    {"cpssDxChCncPortGroupBlockClientRangesSet", 5, cpssDxChCncPortGroupBlockClientRangesSet_PARAMS, NULL},
    {"cpssDxChCncPortGroupBlockClientRangesGet", 5, cpssDxChCncPortGroupBlockClientRangesGet_PARAMS, NULL},
    {"cpssDxChCncPortGroupCounterWraparoundIndexesGet", 6, cpssDxChCncPortGroupCounterWraparoundIndexesGet_PARAMS, cpssDxChCncPortGroupCounterWraparoundIndexesGet_preLogic},
    {"cpssDxChCncPortGroupCounterSet", 6, cpssDxChCncPortGroupCounterSet_PARAMS, NULL},
    {"cpssDxChCncPortGroupCounterGet", 6, cpssDxChCncPortGroupCounterGet_PARAMS, NULL},
    {"cpssDxChCncPortGroupBlockUploadTrigger", 3, cpssDxChCncPortGroupBlockUploadTrigger_PARAMS, NULL},
    {"cpssDxChCncPortGroupBlockUploadInProcessGet", 3, cpssDxChCncPortGroupBlockUploadInProcessGet_PARAMS, NULL},
    {"cpssDxChCncPortGroupUploadedBlockGet", 5, cpssDxChCncPortGroupUploadedBlockGet_PARAMS, cpssDxChCncPortGroupUploadedBlockGet_preLogic},
    {"cpssDxChCncPortGroupCounterFormatSet", 4, cpssDxChCncPortGroupCounterFormatSet_PARAMS, NULL},
    {"cpssDxChCncPortGroupCounterFormatGet", 4, cpssDxChCncPortGroupCounterFormatGet_PARAMS, NULL},
    {"cpssDxChCncOffsetForNatClientSet", 2, cpssDxChCncOffsetForNatClientSet_PARAMS, NULL},
    {"cpssDxChCncOffsetForNatClientGet", 2, cpssDxChCncOffsetForNatClientGet_PARAMS, NULL},
    {"cpssDxChCncQueueStatusLimitSet", 4, cpssDxChCncQueueStatusLimitSet_PARAMS, NULL},
    {"cpssDxChCncQueueStatusLimitGet", 4, cpssDxChCncQueueStatusLimitGet_PARAMS, NULL},
    {"cpssDxChCncPortStatusLimitSet", 3, cpssDxChCncPortStatusLimitSet_PARAMS, NULL},
    {"cpssDxChCncPortStatusLimitGet", 3, cpssDxChCncPortStatusLimitGet_PARAMS, NULL},
    {"cpssDxChCncIngressPacketTypeClientHashModeEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChCncIngressPacketTypeClientHashModeEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChCncPortHashClientEnableSet", 3, prvCpssLogGenDevNumPortNumEnable_PARAMS, NULL},
    {"cpssDxChCncPortHashClientEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr_PARAMS, NULL},
    {"cpssDxChCncPortQueueGroupBaseSet", 4, cpssDxChCncPortQueueGroupBaseSet_PARAMS, NULL},
    {"cpssDxChCncPortQueueGroupBaseGet", 4, cpssDxChCncPortQueueGroupBaseGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_CNC(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChCncLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChCncLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

