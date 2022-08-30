/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChMirrorLog.c
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
#include <cpss/dxCh/dxChxGen/mirror/cpssDxChMirror.h>
#include <cpss/dxCh/dxChxGen/mirror/cpssDxChStc.h>
#include <cpss/dxCh/dxChxGen/mirror/private/prvCpssDxChMirrorLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssGenDbLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_DXCH_MIRROR_EGRESS_MODE_ENT[]  =
{
    "CPSS_DXCH_MIRROR_EGRESS_NOT_DROPPED_E",
    "CPSS_DXCH_MIRROR_EGRESS_TAIL_DROP_E",
    "CPSS_DXCH_MIRROR_EGRESS_CONGESTION_E",
    "CPSS_DXCH_MIRROR_EGRESS_CONGESTION_OR_TAIL_DROP_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_MIRROR_EGRESS_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_MODE_ENT[]  =
{
    "CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_TDM_E",
    "CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_SP_INGRESS_EGRESS_MIRROR_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_MIRROR_ON_CONGESTION_LIMITS_MODE_ENT[]  =
{
    "CPSS_DXCH_MIRROR_ON_CONGESTION_MODE_QCN_E",
    "CPSS_DXCH_MIRROR_ON_CONGESTION_MODE_ECN_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_MIRROR_ON_CONGESTION_LIMITS_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT[]  =
{
    "CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E",
    "CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_OVERRIDE_E",
    "CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_STC_COUNT_MODE_ENT[]  =
{
    "CPSS_DXCH_STC_COUNT_ALL_PACKETS_E",
    "CPSS_DXCH_STC_COUNT_NON_DROPPED_PACKETS_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_STC_COUNT_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_STC_COUNT_RELOAD_MODE_ENT[]  =
{
    "CPSS_DXCH_STC_COUNT_RELOAD_CONTINUOUS_E",
    "CPSS_DXCH_STC_COUNT_RELOAD_TRIGGERED_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_STC_COUNT_RELOAD_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_STC_TYPE_ENT[]  =
{
    "CPSS_DXCH_STC_INGRESS_E",
    "CPSS_DXCH_STC_EGRESS_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_STC_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_MIRROR_ON_DROP_MODE_ENT[]  =
{
    "CPSS_DXCH_MIRROR_ON_DROP_MODE_DISABLE_E",
    "CPSS_DXCH_MIRROR_ON_DROP_MODE_TRAP_E",
    "CPSS_DXCH_MIRROR_ON_DROP_MODE_MIRROR_TO_ANALYZER_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_MIRROR_ON_DROP_MODE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, interface, CPSS_INTERFACE_INFO_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_16_HEX_MAC(valPtr, etherType);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vpt);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, cfi);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vid);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MIRROR_EGRESS_SCHEDULER_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MIRROR_EGRESS_SCHEDULER_CONFIG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, shapedWeightedRoundRobinEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, strictPriorityAnalyzerEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, mirrorToCpuWrrPriority);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, analyzerWrrPriority);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MIRROR_ON_DROP_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MIRROR_ON_DROP_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, mode, CPSS_DXCH_MIRROR_ON_DROP_MODE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, analyzerIndex);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, droppedEgressReplicatedPacketMirrorEnable);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MIRROR_REPLICATION_SP_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MIRROR_REPLICATION_SP_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ingressMirrorPriority);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ingressTrapPriority);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, egressMirrorAndTrapPriority);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, egressQCNPriority);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ingressControlPipePriority);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_MIRROR_REPLICATION_WRR_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_MIRROR_REPLICATION_WRR_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ingressMirrorWeight);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ingressTrapWeight);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, egressMirrorAndTrapWeight);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, egressQCNWeight);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ingressControlPipeWeight);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_MIRROR_EGRESS_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_MIRROR_EGRESS_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_MIRROR_EGRESS_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_MIRROR_EGRESS_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_MIRROR_EGRESS_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_MIRROR_EGRESS_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_MIRROR_EGRESS_SCHEDULER_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_MIRROR_EGRESS_SCHEDULER_CONFIG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_MIRROR_EGRESS_SCHEDULER_CONFIG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_MIRROR_ON_CONGESTION_LIMITS_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_MIRROR_ON_CONGESTION_LIMITS_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_MIRROR_ON_CONGESTION_LIMITS_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_MIRROR_ON_CONGESTION_LIMITS_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_MIRROR_ON_CONGESTION_LIMITS_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_MIRROR_ON_CONGESTION_LIMITS_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_MIRROR_ON_DROP_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_MIRROR_ON_DROP_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_MIRROR_ON_DROP_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_MIRROR_REPLICATION_SP_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_MIRROR_REPLICATION_SP_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_MIRROR_REPLICATION_SP_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_MIRROR_REPLICATION_WRR_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_MIRROR_REPLICATION_WRR_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_MIRROR_REPLICATION_WRR_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_STC_COUNT_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_STC_COUNT_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_STC_COUNT_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_STC_COUNT_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_STC_COUNT_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_STC_COUNT_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_STC_COUNT_RELOAD_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_STC_COUNT_RELOAD_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_STC_COUNT_RELOAD_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_STC_COUNT_RELOAD_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_STC_COUNT_RELOAD_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_STC_COUNT_RELOAD_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_STC_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_STC_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_STC_TYPE_ENT);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DP_LEVEL_ENT_analyzerDp = {
     "analyzerDp", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DP_LEVEL_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC_PTR_interfacePtr = {
     "interfacePtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC_PTR_analyzerVlanTagConfigPtr = {
     "analyzerVlanTagConfigPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_MIRROR_EGRESS_SCHEDULER_CONFIG_STC_PTR_egressSchedulerConfigPtr = {
     "egressSchedulerConfigPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MIRROR_EGRESS_SCHEDULER_CONFIG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_MODE_ENT_mode = {
     "mode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_MIRROR_ON_CONGESTION_LIMITS_MODE_ENT_congestionMode = {
     "congestionMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_MIRROR_ON_CONGESTION_LIMITS_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_MIRROR_ON_DROP_STC_PTR_configPtr = {
     "configPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MIRROR_ON_DROP_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_MIRROR_REPLICATION_SP_STC_PTR_priorityConfigPtr = {
     "priorityConfigPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MIRROR_REPLICATION_SP_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_MIRROR_REPLICATION_WRR_STC_PTR_wrrConfigPtr = {
     "wrrConfigPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MIRROR_REPLICATION_WRR_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT_mode = {
     "mode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_STC_COUNT_MODE_ENT_mode = {
     "mode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_STC_COUNT_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_STC_COUNT_RELOAD_MODE_ENT_mode = {
     "mode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_STC_COUNT_RELOAD_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_STC_TYPE_ENT_stcType = {
     "stcType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_STC_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_truncate = {
     "truncate", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_PORT_NUM_mirrPort = {
     "mirrPort", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_PORT_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_cntr = {
     "cntr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ratio = {
     "ratio", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_analyzerTc = {
     "analyzerTc", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DP_LEVEL_ENT_PTR_analyzerDpPtr = {
     "analyzerDpPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DP_LEVEL_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC_PTR_interfacePtr = {
     "interfacePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC_PTR_analyzerVlanTagConfigPtr = {
     "analyzerVlanTagConfigPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_MIRROR_EGRESS_MODE_ENT_PTR_egressMirroringModePtr = {
     "egressMirroringModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MIRROR_EGRESS_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_MIRROR_EGRESS_SCHEDULER_CONFIG_STC_PTR_egressSchedulerConfigPtr = {
     "egressSchedulerConfigPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MIRROR_EGRESS_SCHEDULER_CONFIG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_MODE_ENT_PTR_modePtr = {
     "modePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_MIRROR_ON_CONGESTION_LIMITS_MODE_ENT_PTR_congestionModePtr = {
     "congestionModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MIRROR_ON_CONGESTION_LIMITS_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_MIRROR_ON_DROP_STC_PTR_configPtr = {
     "configPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MIRROR_ON_DROP_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_MIRROR_REPLICATION_SP_STC_PTR_priorityConfigPtr = {
     "priorityConfigPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MIRROR_REPLICATION_SP_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_MIRROR_REPLICATION_WRR_STC_PTR_wrrConfigPtr = {
     "wrrConfigPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MIRROR_REPLICATION_WRR_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT_PTR_modePtr = {
     "modePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_STC_COUNT_MODE_ENT_PTR_modePtr = {
     "modePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_STC_COUNT_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_STC_COUNT_RELOAD_MODE_ENT_PTR_modePtr = {
     "modePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_STC_COUNT_RELOAD_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_truncatePtr = {
     "truncatePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_ratioPtr = {
     "ratioPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_analyzerTcPtr = {
     "analyzerTcPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U8)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMirrRxAnalyzerDpTcSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DP_LEVEL_ENT_analyzerDp,
    &DX_IN_GT_U8_analyzerTc
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMirrorTxAnalyzerVlanTagConfig_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC_PTR_analyzerVlanTagConfigPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_MIRROR_EGRESS_MODE_ENT_egressMirroringMode,
    &DX_IN_GT_BOOL_enable,
    &DX_IN_GT_U32_index
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_MIRROR_EGRESS_MODE_ENT_egressMirroringMode,
    &DX_OUT_GT_BOOL_PTR_enablePtr,
    &DX_OUT_GT_U32_PTR_indexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMirrorArbiterSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_MIRROR_EGRESS_SCHEDULER_CONFIG_STC_PTR_egressSchedulerConfigPtr,
    &DX_IN_CPSS_DXCH_MIRROR_REPLICATION_SP_STC_PTR_priorityConfigPtr,
    &DX_IN_CPSS_DXCH_MIRROR_REPLICATION_WRR_STC_PTR_wrrConfigPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMirrorEnhancedMirroringPriorityModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_MODE_ENT_mode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMirrorOnCongestionModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_MIRROR_ON_CONGESTION_LIMITS_MODE_ENT_congestionMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMirrorOnDropConfigSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_MIRROR_ON_DROP_STC_PTR_configPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMirrorToAnalyzerForwardingModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT_mode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChStcIngressCountModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_STC_COUNT_MODE_ENT_mode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChStcReloadModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_STC_TYPE_ENT_stcType,
    &DX_IN_CPSS_DXCH_STC_COUNT_RELOAD_MODE_ENT_mode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChStcEnableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_STC_TYPE_ENT_stcType,
    &DX_IN_GT_BOOL_enable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChStcReloadModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_STC_TYPE_ENT_stcType,
    &DX_OUT_CPSS_DXCH_STC_COUNT_RELOAD_MODE_ENT_PTR_modePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChStcEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_STC_TYPE_ENT_stcType,
    &DX_OUT_GT_BOOL_PTR_enablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_NET_RX_CPU_CODE_ENT_dropCode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChStcPortSampledPacketsCntrSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_port,
    &DX_IN_CPSS_DXCH_STC_TYPE_ENT_stcType,
    &DX_IN_GT_U32_cntr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChStcPortLimitSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_port,
    &DX_IN_CPSS_DXCH_STC_TYPE_ENT_stcType,
    &DX_IN_GT_U32_limit
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChStcPortReadyForNewLimitGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_port,
    &DX_IN_CPSS_DXCH_STC_TYPE_ENT_stcType,
    &DX_OUT_GT_BOOL_PTR_isReadyPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChStcPortCountdownCntrGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_port,
    &DX_IN_CPSS_DXCH_STC_TYPE_ENT_stcType,
    &DX_OUT_GT_U32_PTR_cntrPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChStcPortLimitGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_port,
    &DX_IN_CPSS_DXCH_STC_TYPE_ENT_stcType,
    &DX_OUT_GT_U32_PTR_limitPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMirrorTxPortModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_mirrPort,
    &DX_IN_GT_BOOL_isPhysicalPort,
    &DX_IN_CPSS_DXCH_MIRROR_EGRESS_MODE_ENT_egressMirroringMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMirrorRxPortSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_mirrPort,
    &DX_IN_GT_BOOL_isPhysicalPort,
    &DX_IN_GT_BOOL_enable,
    &DX_IN_GT_U32_index
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMirrorTxPortModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_mirrPort,
    &DX_IN_GT_BOOL_isPhysicalPort,
    &DX_OUT_CPSS_DXCH_MIRROR_EGRESS_MODE_ENT_PTR_egressMirroringModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMirrorRxPortGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_mirrPort,
    &DX_IN_GT_BOOL_isPhysicalPort,
    &DX_OUT_GT_BOOL_PTR_enablePtr,
    &DX_OUT_GT_U32_PTR_indexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMirrorAnalyzerInterfaceSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_IN_CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC_PTR_interfacePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_IN_GT_BOOL_enable,
    &DX_IN_GT_U32_ratio
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMirrorToAnalyzerTruncateSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_IN_GT_BOOL_truncate
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMirrorTxStatMirrorToAnalyzerIndexRatioSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_IN_GT_U32_ratio
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMirrorAnalyzerInterfaceGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_OUT_CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC_PTR_interfacePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_OUT_GT_BOOL_PTR_enablePtr,
    &DX_OUT_GT_U32_PTR_ratioPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMirrorToAnalyzerTruncateGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_OUT_GT_BOOL_PTR_truncatePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMirrorTxStatMirrorToAnalyzerIndexRatioGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_OUT_GT_U32_PTR_ratioPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMirrRxStatMirroringToAnalyzerRatioSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_ratio
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMirrRxAnalyzerDpTcGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DP_LEVEL_ENT_PTR_analyzerDpPtr,
    &DX_OUT_GT_U8_PTR_analyzerTcPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMirrorTxAnalyzerVlanTagConfigGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_MIRROR_ANALYZER_VLAN_TAG_CFG_STC_PTR_analyzerVlanTagConfigPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMirrorArbiterGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_MIRROR_EGRESS_SCHEDULER_CONFIG_STC_PTR_egressSchedulerConfigPtr,
    &DX_OUT_CPSS_DXCH_MIRROR_REPLICATION_SP_STC_PTR_priorityConfigPtr,
    &DX_OUT_CPSS_DXCH_MIRROR_REPLICATION_WRR_STC_PTR_wrrConfigPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMirrorEnhancedMirroringPriorityModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_MIRROR_ENHANCED_PRIORITY_MODE_ENT_PTR_modePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMirrorOnCongestionModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_MIRROR_ON_CONGESTION_LIMITS_MODE_ENT_PTR_congestionModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMirrorOnDropConfigGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_MIRROR_ON_DROP_STC_PTR_configPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMirrorToAnalyzerForwardingModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT_PTR_modePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChStcIngressCountModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_STC_COUNT_MODE_ENT_PTR_modePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_NET_RX_CPU_CODE_ENT_PTR_dropCodePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChMirrRxStatMirroringToAnalyzerRatioGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_ratioPtr
};


/********* lib API DB *********/

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChMirrorLogLibDb[] = {
    {"cpssDxChMirrorAnalyzerVlanTagEnable", 3, prvCpssLogGenDevNumPortNumEnable2_PARAMS, NULL},
    {"cpssDxChMirrorAnalyzerVlanTagEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr2_PARAMS, NULL},
    {"cpssDxChMirrorTxAnalyzerVlanTagConfig", 2, cpssDxChMirrorTxAnalyzerVlanTagConfig_PARAMS, NULL},
    {"cpssDxChMirrorTxAnalyzerVlanTagConfigGet", 2, cpssDxChMirrorTxAnalyzerVlanTagConfigGet_PARAMS, NULL},
    {"cpssDxChMirrorRxAnalyzerVlanTagConfig", 2, cpssDxChMirrorTxAnalyzerVlanTagConfig_PARAMS, NULL},
    {"cpssDxChMirrorRxAnalyzerVlanTagConfigGet", 2, cpssDxChMirrorTxAnalyzerVlanTagConfigGet_PARAMS, NULL},
    {"cpssDxChMirrorTxCascadeMonitorEnable", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChMirrorTxCascadeMonitorEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChMirrorRxPortSet", 5, cpssDxChMirrorRxPortSet_PARAMS, NULL},
    {"cpssDxChMirrorRxPortGet", 5, cpssDxChMirrorRxPortGet_PARAMS, NULL},
    {"cpssDxChMirrorTxPortSet", 5, cpssDxChMirrorRxPortSet_PARAMS, NULL},
    {"cpssDxChMirrorTxPortGet", 5, cpssDxChMirrorRxPortGet_PARAMS, NULL},
    {"cpssDxChMirrRxStatMirroringToAnalyzerRatioSet", 2, cpssDxChMirrRxStatMirroringToAnalyzerRatioSet_PARAMS, NULL},
    {"cpssDxChMirrRxStatMirroringToAnalyzerRatioGet", 2, cpssDxChMirrRxStatMirroringToAnalyzerRatioGet_PARAMS, NULL},
    {"cpssDxChMirrRxStatMirrorToAnalyzerEnable", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChMirrRxStatMirrorToAnalyzerEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChMirrRxAnalyzerDpTcSet", 3, cpssDxChMirrRxAnalyzerDpTcSet_PARAMS, NULL},
    {"cpssDxChMirrRxAnalyzerDpTcGet", 3, cpssDxChMirrRxAnalyzerDpTcGet_PARAMS, NULL},
    {"cpssDxChMirrTxAnalyzerDpTcSet", 3, cpssDxChMirrRxAnalyzerDpTcSet_PARAMS, NULL},
    {"cpssDxChMirrTxAnalyzerDpTcGet", 3, cpssDxChMirrRxAnalyzerDpTcGet_PARAMS, NULL},
    {"cpssDxChMirrTxStatMirroringToAnalyzerRatioSet", 2, cpssDxChMirrRxStatMirroringToAnalyzerRatioSet_PARAMS, NULL},
    {"cpssDxChMirrTxStatMirroringToAnalyzerRatioGet", 2, cpssDxChMirrRxStatMirroringToAnalyzerRatioGet_PARAMS, NULL},
    {"cpssDxChMirrTxStatMirrorToAnalyzerEnable", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChMirrTxStatMirrorToAnalyzerEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChMirrorToAnalyzerForwardingModeSet", 2, cpssDxChMirrorToAnalyzerForwardingModeSet_PARAMS, NULL},
    {"cpssDxChMirrorToAnalyzerForwardingModeGet", 2, cpssDxChMirrorToAnalyzerForwardingModeGet_PARAMS, NULL},
    {"cpssDxChMirrorAnalyzerInterfaceSet", 3, cpssDxChMirrorAnalyzerInterfaceSet_PARAMS, NULL},
    {"cpssDxChMirrorAnalyzerInterfaceGet", 3, cpssDxChMirrorAnalyzerInterfaceGet_PARAMS, NULL},
    {"cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexSet", 3, prvCpssLogGenDevNumEnableIndex_PARAMS, NULL},
    {"cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexGet", 3, prvCpssLogGenDevNumEnablePtrIndexPtr_PARAMS, NULL},
    {"cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexSet", 3, prvCpssLogGenDevNumEnableIndex_PARAMS, NULL},
    {"cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexGet", 3, prvCpssLogGenDevNumEnablePtrIndexPtr_PARAMS, NULL},
    {"cpssDxChMirrorAnalyzerMirrorOnDropEnableSet", 3, prvCpssLogGenDevNumIndexEnable_PARAMS, NULL},
    {"cpssDxChMirrorAnalyzerMirrorOnDropEnableGet", 3, prvCpssLogGenDevNumIndexEnablePtr_PARAMS, NULL},
    {"cpssDxChMirrorAnalyzerVlanTagRemoveEnableSet", 3, prvCpssLogGenDevNumPortNumEnable2_PARAMS, NULL},
    {"cpssDxChMirrorAnalyzerVlanTagRemoveEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr2_PARAMS, NULL},
    {"cpssDxChMirrorEnhancedMirroringPriorityModeSet", 2, cpssDxChMirrorEnhancedMirroringPriorityModeSet_PARAMS, NULL},
    {"cpssDxChMirrorEnhancedMirroringPriorityModeGet", 2, cpssDxChMirrorEnhancedMirroringPriorityModeGet_PARAMS, NULL},
    {"cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeSet", 2, cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeSet_PARAMS, NULL},
    {"cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeGet", 2, cpssDxChMirrorHopByHopBadAnalyzerIndexDropCodeGet_PARAMS, NULL},
    {"cpssDxChMirrorTxPortVlanEnableSet", 3, prvCpssLogGenDevNumPortNumEnable_PARAMS, NULL},
    {"cpssDxChMirrorTxPortVlanEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr_PARAMS, NULL},
    {"cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexSet", 4, cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexSet_PARAMS, NULL},
    {"cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexGet", 4, cpssDxChMirrorTxModeGlobalAnalyzerInterfaceIndexGet_PARAMS, NULL},
    {"cpssDxChMirrorTxPortModeSet", 4, cpssDxChMirrorTxPortModeSet_PARAMS, NULL},
    {"cpssDxChMirrorTxPortModeGet", 4, cpssDxChMirrorTxPortModeGet_PARAMS, NULL},
    {"cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet", 4, cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet_PARAMS, NULL},
    {"cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet", 4, cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet_PARAMS, NULL},
    {"cpssDxChMirrorTxStatMirrorToAnalyzerIndexRatioSet", 3, cpssDxChMirrorTxStatMirrorToAnalyzerIndexRatioSet_PARAMS, NULL},
    {"cpssDxChMirrorTxStatMirrorToAnalyzerIndexRatioGet", 3, cpssDxChMirrorTxStatMirrorToAnalyzerIndexRatioGet_PARAMS, NULL},
    {"cpssDxChMirrorToAnalyzerTruncateSet", 3, cpssDxChMirrorToAnalyzerTruncateSet_PARAMS, NULL},
    {"cpssDxChMirrorToAnalyzerTruncateGet", 3, cpssDxChMirrorToAnalyzerTruncateGet_PARAMS, NULL},
    {"cpssDxChMirrorOnCongestionModeSet", 2, cpssDxChMirrorOnCongestionModeSet_PARAMS, NULL},
    {"cpssDxChMirrorOnCongestionModeGet", 2, cpssDxChMirrorOnCongestionModeGet_PARAMS, NULL},
    {"cpssDxChMirrorTreatEgressMirroredAsIngressMirroredEnableSet", 3, prvCpssLogGenDevNumIndexEnable_PARAMS, NULL},
    {"cpssDxChMirrorTreatEgressMirroredAsIngressMirroredEnableGet", 3, prvCpssLogGenDevNumIndexEnablePtr_PARAMS, NULL},
    {"cpssDxChMirrorTreatToAnalyzerDsaPktAsIngressMirroredEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChMirrorTreatToAnalyzerDsaPktAsIngressMirroredEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChMirrorArbiterSet", 4, cpssDxChMirrorArbiterSet_PARAMS, NULL},
    {"cpssDxChMirrorArbiterGet", 4, cpssDxChMirrorArbiterGet_PARAMS, NULL},
    {"cpssDxChMirrorOnDropConfigSet", 2, cpssDxChMirrorOnDropConfigSet_PARAMS, NULL},
    {"cpssDxChMirrorOnDropConfigGet", 2, cpssDxChMirrorOnDropConfigGet_PARAMS, NULL},
    {"cpssDxChStcIngressCountModeSet", 2, cpssDxChStcIngressCountModeSet_PARAMS, NULL},
    {"cpssDxChStcIngressCountModeGet", 2, cpssDxChStcIngressCountModeGet_PARAMS, NULL},
    {"cpssDxChStcReloadModeSet", 3, cpssDxChStcReloadModeSet_PARAMS, NULL},
    {"cpssDxChStcReloadModeGet", 3, cpssDxChStcReloadModeGet_PARAMS, NULL},
    {"cpssDxChStcEnableSet", 3, cpssDxChStcEnableSet_PARAMS, NULL},
    {"cpssDxChStcEnableGet", 3, cpssDxChStcEnableGet_PARAMS, NULL},
    {"cpssDxChStcPortLimitSet", 4, cpssDxChStcPortLimitSet_PARAMS, NULL},
    {"cpssDxChStcPortLimitGet", 4, cpssDxChStcPortLimitGet_PARAMS, NULL},
    {"cpssDxChStcPortReadyForNewLimitGet", 4, cpssDxChStcPortReadyForNewLimitGet_PARAMS, NULL},
    {"cpssDxChStcPortCountdownCntrGet", 4, cpssDxChStcPortCountdownCntrGet_PARAMS, NULL},
    {"cpssDxChStcPortSampledPacketsCntrSet", 4, cpssDxChStcPortSampledPacketsCntrSet_PARAMS, NULL},
    {"cpssDxChStcPortSampledPacketsCntrGet", 4, cpssDxChStcPortCountdownCntrGet_PARAMS, NULL},
    {"cpssDxChStcEgressAnalyzerIndexSet", 3, prvCpssLogGenDevNumEnableIndex_PARAMS, NULL},
    {"cpssDxChStcEgressAnalyzerIndexGet", 3, prvCpssLogGenDevNumEnablePtrIndexPtr_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_MIRROR(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChMirrorLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChMirrorLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

