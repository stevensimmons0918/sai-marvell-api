/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChHsrPrpLog.c
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
#include <cpss/dxCh/dxChxGen/hsrPrp/cpssDxChHsrPrp.h>
#include <cpss/dxCh/dxChxGen/hsrPrp/private/prvCpssDxChHsrPrpLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssGenDbLog.h>
#include <cpss/generic/log/prvCpssGenLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_DXCH_HSR_PRP_COUNTER_TYPE_ENT[]  =
{
    "CPSS_DXCH_HSR_PRP_COUNTER_TYPE_PNT_NUM_OF_READY_ENTRIES_E",
    "CPSS_DXCH_HSR_PRP_COUNTER_OVERRIDEN_NON_EXPIRED_ENTRIES_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_HSR_PRP_COUNTER_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT[]  =
{
    "CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_10_MICRO_SEC_E",
    "CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_20_MICRO_SEC_E",
    "CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_40_MICRO_SEC_E",
    "CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_80_MICRO_SEC_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_ENT[]  =
{
    "CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_SAME_KEY_EXPIRATION_E",
    "CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_EXPIRATION_E",
    "CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_TIMER_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT[]  =
{
    "CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_HSR_SIZE_MISMATCH_E",
    "CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_PNT_FULL_UNTAGGED_E",
    "CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_PNT_FULL_TAGGED_E",
    "CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ZERO_BMP_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT[]  =
{
    "CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_NONE_E",
    "CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_1_2_E",
    "CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_3_4_E",
    "CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_7_8_E",
    "CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_DDE_ALL_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_HSR_PRP_PNT_OPER_ENT[]  =
{
    "CPSS_DXCH_HSR_PRP_PNT_OPER_SET_READY_E",
    "CPSS_DXCH_HSR_PRP_PNT_OPER_SET_INVALID_E",
    "CPSS_DXCH_HSR_PRP_PNT_OPER_SET_VALID_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_HSR_PRP_PNT_OPER_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_HSR_PRP_PNT_STATE_ENT[]  =
{
    "CPSS_DXCH_HSR_PRP_PNT_STATE_READY_E",
    "CPSS_DXCH_HSR_PRP_PNT_STATE_VALID_E",
    "CPSS_DXCH_HSR_PRP_PNT_STATE_NOT_VALID_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_HSR_PRP_PNT_STATE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_HSR_PRP_PORTS_BMP_ID_ENT[]  =
{
    "CPSS_DXCH_HSR_PRP_PORTS_BMP_FILTER_ID_1_E",
    "CPSS_DXCH_HSR_PRP_PORTS_BMP_FILTER_ID_2_E",
    "CPSS_DXCH_HSR_PRP_PORTS_BMP_FILTER_ID_3_E",
    "CPSS_DXCH_HSR_PRP_PORTS_BMP_IGNORE_DDL_DECISION_E",
    "CPSS_DXCH_HSR_PRP_PORTS_BMP_IGNORE_SOURCE_DUPLICATION_DISCARD_E",
    "CPSS_DXCH_HSR_PRP_PORTS_BMP_SOURCE_IS_ME_CLEANING_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_HSR_PRP_PORTS_BMP_ID_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_ENT[]  =
{
    "CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_PAD_UNTAGGED_TO_60_OR_VLAN_TAGGED_TO_64_E",
    "CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_PAD_TO_60_E",
    "CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_PAD_TO_64_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ENT[]  =
{
    "CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_NONE_E",
    "CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ADD_PRP_E",
    "CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_REMOVE_PRP_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_HSR_PRP_FDB_DDL_MODE_ENT[]  =
{
    "CPSS_DXCH_HSR_PRP_FDB_DDL_MODE_SKIP_E",
    "CPSS_DXCH_HSR_PRP_FDB_DDL_MODE_DDL_E",
    "CPSS_DXCH_HSR_PRP_FDB_DDL_MODE_FDB_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_HSR_PRP_FDB_DDL_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_HSR_PRP_LRE_PORT_TYPE_ENT[]  =
{
    "CPSS_DXCH_HSR_PRP_LRE_PORT_TYPE_A_E",
    "CPSS_DXCH_HSR_PRP_LRE_PORT_TYPE_B_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_HSR_PRP_LRE_PORT_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_HSR_PRP_PNT_LOOKUP_MODE_ENT[]  =
{
    "CPSS_DXCH_HSR_PRP_PNT_LOOKUP_MODE_SKIP_E",
    "CPSS_DXCH_HSR_PRP_PNT_LOOKUP_MODE_SRC_AND_LEARN_E",
    "CPSS_DXCH_HSR_PRP_PNT_LOOKUP_MODE_DST_AND_SRC_E",
    "CPSS_DXCH_HSR_PRP_PNT_LOOKUP_MODE_SRC_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_HSR_PRP_PNT_LOOKUP_MODE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_HSR_PRP_DDE_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_HSR_PRP_DDE_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ddeKey, CPSS_DXCH_HSR_PRP_DDE_HASH_KEY_STC);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, destBmp);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, srcHPort);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, timeInMicroSec);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, destHportCounters, CPSS_DXCH_HSR_PRP_H_PORTS_NUM_CNS, GT_U8);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ageBit);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_HSR_PRP_DDE_HASH_KEY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_HSR_PRP_DDE_HASH_KEY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ETH_MAC(valPtr, macAddr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, seqNum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, lreInstance);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_HSR_PRP_HPORT_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_HSR_PRP_HPORT_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, pntLookupMode, CPSS_DXCH_HSR_PRP_PNT_LOOKUP_MODE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, fdbDdlMode, CPSS_DXCH_HSR_PRP_FDB_DDL_MODE_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, pntLearnTaggedPackets);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, acceptSamePort);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, lreRingPortType, CPSS_DXCH_HSR_PRP_LRE_PORT_TYPE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, lreInstance);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, defaultForwardingBmp);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, srcIsMeBmp);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, srcDuplicateDiscardBmp);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, destToMeForwardingBmp);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_HSR_PRP_LRE_INSTANCE_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_HSR_PRP_LRE_INSTANCE_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numOfPorts);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, lowestHPort);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, eVidxMappingBase);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_HSR_PRP_PNT_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_HSR_PRP_PNT_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, streamId);
    PRV_CPSS_LOG_STC_ETH_MAC(valPtr, macAddr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ageBit);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, untagged);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, lreInstance);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, destBmp);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_COUNTER_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_HSR_PRP_COUNTER_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_HSR_PRP_COUNTER_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_DDE_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_HSR_PRP_DDE_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_HSR_PRP_DDE_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_DDE_HASH_KEY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_HSR_PRP_DDE_HASH_KEY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_HSR_PRP_DDE_HASH_KEY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_HPORT_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_HSR_PRP_HPORT_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_HSR_PRP_HPORT_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_LRE_INSTANCE_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_HSR_PRP_LRE_INSTANCE_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_HSR_PRP_LRE_INSTANCE_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_PNT_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_HSR_PRP_PNT_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_HSR_PRP_PNT_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_PNT_OPER_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_HSR_PRP_PNT_OPER_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_HSR_PRP_PNT_OPER_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_PNT_STATE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_HSR_PRP_PNT_STATE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_HSR_PRP_PNT_STATE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_PORTS_BMP_ID_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_HSR_PRP_PORTS_BMP_ID_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_HSR_PRP_PORTS_BMP_ID_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ENT);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_HSR_PRP_COUNTER_TYPE_ENT_counterType = {
     "counterType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_HSR_PRP_COUNTER_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_HSR_PRP_DDE_ENTRY_STC_PTR_entryPtr = {
     "entryPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_HSR_PRP_DDE_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_HSR_PRP_DDE_HASH_KEY_STC_PTR_entryKeyPtr = {
     "entryKeyPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_HSR_PRP_DDE_HASH_KEY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT_mode = {
     "mode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_ENT_field = {
     "field", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT_type = {
     "type", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT_mode = {
     "mode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_HSR_PRP_HPORT_ENTRY_STC_PTR_infoPtr = {
     "infoPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_HSR_PRP_HPORT_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_HSR_PRP_LRE_INSTANCE_ENTRY_STC_PTR_infoPtr = {
     "infoPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_HSR_PRP_LRE_INSTANCE_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_HSR_PRP_PNT_ENTRY_STC_PTR_infoPtr = {
     "infoPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_HSR_PRP_PNT_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_HSR_PRP_PNT_OPER_ENT_operation = {
     "operation", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_HSR_PRP_PNT_OPER_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_HSR_PRP_PORTS_BMP_ID_ENT_bmpType = {
     "bmpType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_HSR_PRP_PORTS_BMP_ID_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_ENT_mode = {
     "mode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ENT_action = {
     "action", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORTS_SHORT_BMP_STC_bmp = {
     "bmp", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PORTS_SHORT_BMP_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_PORT_NUM_baseEport = {
     "baseEport", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_PORT_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_hPort = {
     "hPort", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_lanId = {
     "lanId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_lsduOffset = {
     "lsduOffset", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_startBank = {
     "startBank", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_timeInMicroSec = {
     "timeInMicroSec", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_HSR_PRP_DDE_ENTRY_STC_PTR_entryPtr = {
     "entryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_HSR_PRP_DDE_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT_PTR_modePtr = {
     "modePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT_PTR_modePtr = {
     "modePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_HSR_PRP_HPORT_ENTRY_STC_PTR_infoPtr = {
     "infoPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_HSR_PRP_HPORT_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_HSR_PRP_LRE_INSTANCE_ENTRY_STC_PTR_infoPtr = {
     "infoPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_HSR_PRP_LRE_INSTANCE_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_HSR_PRP_PNT_ENTRY_STC_PTR_infoPtr = {
     "infoPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_HSR_PRP_PNT_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_HSR_PRP_PNT_STATE_ENT_PTR_statePtr = {
     "statePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_HSR_PRP_PNT_STATE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_ENT_PTR_modePtr = {
     "modePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ENT_PTR_actionPtr = {
     "actionPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORTS_SHORT_BMP_STC_PTR_bmpPtr = {
     "bmpPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORTS_SHORT_BMP_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_PORT_NUM_PTR_baseEportPtr = {
     "baseEportPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_PORT_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_hashArr = {
     "hashArr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_lanIdPtr = {
     "lanIdPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_lsduOffsetPtr = {
     "lsduOffsetPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_numOfEntriesDeletedPtr = {
     "numOfEntriesDeletedPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_timeInMicroSecPtr = {
     "timeInMicroSecPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U64_PTR_firstCounterPtr = {
     "firstCounterPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U64)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U64_PTR_lreADiscardCounterPtr = {
     "lreADiscardCounterPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U64)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U64_PTR_lreBDiscardCounterPtr = {
     "lreBDiscardCounterPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U64)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U64_PTR_secondCounterPtr = {
     "secondCounterPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U64)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U64_PTR_thirdCounterPtr = {
     "thirdCounterPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U64)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsrPrpCounterGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_HSR_PRP_COUNTER_TYPE_ENT_counterType,
    &DX_OUT_GT_U64_PTR_counterPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsrPrpDdeHashCalc_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_HSR_PRP_DDE_HASH_KEY_STC_PTR_entryKeyPtr,
    &DX_IN_GT_U32_startBank,
    &DX_IN_GT_U32_numOfBanks,
    &DX_OUT_GT_U32_PTR_hashArr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsrPrpDdeTimerGranularityModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT_mode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsrPrpDdeTimeFieldSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_ENT_field,
    &DX_IN_GT_U32_timeInMicroSec
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsrPrpDdeTimeFieldGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_ENT_field,
    &DX_OUT_GT_U32_PTR_timeInMicroSecPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsrPrpExceptionCpuCodeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT_type,
    &DX_IN_CPSS_NET_RX_CPU_CODE_ENT_cpuCode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsrPrpExceptionCommandSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT_type,
    &DX_IN_CPSS_PACKET_CMD_ENT_command
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsrPrpExceptionCpuCodeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT_type,
    &DX_OUT_CPSS_NET_RX_CPU_CODE_ENT_PTR_cpuCodePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsrPrpExceptionCommandGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT_type,
    &DX_OUT_CPSS_PACKET_CMD_ENT_PTR_commandPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsrPrpFdbPartitionModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT_mode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsrPrpPortsBmpSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_HSR_PRP_PORTS_BMP_ID_ENT_bmpType,
    &DX_IN_CPSS_PORTS_SHORT_BMP_STC_bmp
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsrPrpPortsBmpGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_HSR_PRP_PORTS_BMP_ID_ENT_bmpType,
    &DX_OUT_CPSS_PORTS_SHORT_BMP_STC_PTR_bmpPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsrPrpPortPrpPaddingModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_ENT_mode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsrPrpPortPrpTrailerActionSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ENT_action
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U32_lsduOffset
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsrPrpPortPrpPaddingModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_ENT_PTR_modePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsrPrpPortPrpTrailerActionGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ENT_PTR_actionPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_U32_PTR_lsduOffsetPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsrPrpHPortSrcEPortBaseSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_baseEport
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsrPrpPortPrpLanIdSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_CPSS_DIRECTION_ENT_direction,
    &DX_IN_GT_U32_lanId
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsrPrpPortPrpLanIdGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_CPSS_DIRECTION_ENT_direction,
    &DX_OUT_GT_U32_PTR_lanIdPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsrPrpHPortEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_hPort,
    &DX_IN_CPSS_DXCH_HSR_PRP_HPORT_ENTRY_STC_PTR_infoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsrPrpHPortEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_hPort,
    &DX_OUT_CPSS_DXCH_HSR_PRP_HPORT_ENTRY_STC_PTR_infoPtr,
    &DX_OUT_GT_U64_PTR_firstCounterPtr,
    &DX_OUT_GT_U64_PTR_secondCounterPtr,
    &DX_OUT_GT_U64_PTR_thirdCounterPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsrPrpDdeEntryWrite_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_IN_CPSS_DXCH_HSR_PRP_DDE_ENTRY_STC_PTR_entryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsrPrpLreInstanceEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_IN_CPSS_DXCH_HSR_PRP_LRE_INSTANCE_ENTRY_STC_PTR_infoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsrPrpPntEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_IN_CPSS_DXCH_HSR_PRP_PNT_OPER_ENT_operation,
    &DX_IN_CPSS_DXCH_HSR_PRP_PNT_ENTRY_STC_PTR_infoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsrPrpPntAgingApply_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_IN_GT_U32_numOfEntries,
    &DX_OUT_GT_U32_PTR_numOfEntriesDeletedPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsrPrpDdeEntryRead_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_OUT_CPSS_DXCH_HSR_PRP_DDE_ENTRY_STC_PTR_entryPtr,
    &DX_OUT_GT_BOOL_PTR_isValidPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsrPrpLreInstanceEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_OUT_CPSS_DXCH_HSR_PRP_LRE_INSTANCE_ENTRY_STC_PTR_infoPtr,
    &DX_OUT_GT_U64_PTR_lreADiscardCounterPtr,
    &DX_OUT_GT_U64_PTR_lreBDiscardCounterPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsrPrpPntEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_OUT_CPSS_DXCH_HSR_PRP_PNT_STATE_ENT_PTR_statePtr,
    &DX_OUT_CPSS_DXCH_HSR_PRP_PNT_ENTRY_STC_PTR_infoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsrPrpPntNumOfReadyEntriesThresholdSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_threshold
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsrPrpDdeTimerGranularityModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT_PTR_modePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsrPrpFdbPartitionModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT_PTR_modePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsrPrpHPortSrcEPortBaseGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_PORT_NUM_PTR_baseEportPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsrPrpPntNumOfReadyEntriesThresholdGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_thresholdPtr
};


/********* lib API DB *********/

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChHsrPrpLogLibDb[] = {
    {"cpssDxChHsrPrpExceptionCommandSet", 3, cpssDxChHsrPrpExceptionCommandSet_PARAMS, NULL},
    {"cpssDxChHsrPrpExceptionCommandGet", 3, cpssDxChHsrPrpExceptionCommandGet_PARAMS, NULL},
    {"cpssDxChHsrPrpExceptionCpuCodeSet", 3, cpssDxChHsrPrpExceptionCpuCodeSet_PARAMS, NULL},
    {"cpssDxChHsrPrpExceptionCpuCodeGet", 3, cpssDxChHsrPrpExceptionCpuCodeGet_PARAMS, NULL},
    {"cpssDxChHsrPrpPortPrpEnableSet", 3, prvCpssLogGenDevNumPortNumEnable2_PARAMS, NULL},
    {"cpssDxChHsrPrpPortPrpEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr2_PARAMS, NULL},
    {"cpssDxChHsrPrpPortPrpLanIdSet", 4, cpssDxChHsrPrpPortPrpLanIdSet_PARAMS, NULL},
    {"cpssDxChHsrPrpPortPrpLanIdGet", 4, cpssDxChHsrPrpPortPrpLanIdGet_PARAMS, NULL},
    {"cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsSet", 3, prvCpssLogGenDevNumPortNumEnable2_PARAMS, NULL},
    {"cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsGet", 3, prvCpssLogGenDevNumPortNumEnablePtr2_PARAMS, NULL},
    {"cpssDxChHsrPrpPortsBmpSet", 3, cpssDxChHsrPrpPortsBmpSet_PARAMS, NULL},
    {"cpssDxChHsrPrpPortsBmpGet", 3, cpssDxChHsrPrpPortsBmpGet_PARAMS, NULL},
    {"cpssDxChHsrPrpHPortSrcEPortBaseSet", 2, cpssDxChHsrPrpHPortSrcEPortBaseSet_PARAMS, NULL},
    {"cpssDxChHsrPrpHPortSrcEPortBaseGet", 2, cpssDxChHsrPrpHPortSrcEPortBaseGet_PARAMS, NULL},
    {"cpssDxChHsrPrpCounterGet", 3, cpssDxChHsrPrpCounterGet_PARAMS, NULL},
    {"cpssDxChHsrPrpPntNumOfReadyEntriesThresholdSet", 2, cpssDxChHsrPrpPntNumOfReadyEntriesThresholdSet_PARAMS, NULL},
    {"cpssDxChHsrPrpPntNumOfReadyEntriesThresholdGet", 2, cpssDxChHsrPrpPntNumOfReadyEntriesThresholdGet_PARAMS, NULL},
    {"cpssDxChHsrPrpHPortEntrySet", 3, cpssDxChHsrPrpHPortEntrySet_PARAMS, NULL},
    {"cpssDxChHsrPrpHPortEntryGet", 6, cpssDxChHsrPrpHPortEntryGet_PARAMS, NULL},
    {"cpssDxChHsrPrpPntEntrySet", 4, cpssDxChHsrPrpPntEntrySet_PARAMS, NULL},
    {"cpssDxChHsrPrpPntEntryGet", 4, cpssDxChHsrPrpPntEntryGet_PARAMS, NULL},
    {"cpssDxChHsrPrpPntAgingApply", 4, cpssDxChHsrPrpPntAgingApply_PARAMS, NULL},
    {"cpssDxChHsrPrpLreInstanceEntrySet", 3, cpssDxChHsrPrpLreInstanceEntrySet_PARAMS, NULL},
    {"cpssDxChHsrPrpLreInstanceEntryGet", 5, cpssDxChHsrPrpLreInstanceEntryGet_PARAMS, NULL},
    {"cpssDxChHsrPrpFdbPartitionModeSet", 2, cpssDxChHsrPrpFdbPartitionModeSet_PARAMS, NULL},
    {"cpssDxChHsrPrpFdbPartitionModeGet", 2, cpssDxChHsrPrpFdbPartitionModeGet_PARAMS, NULL},
    {"cpssDxChHsrPrpDdeTimerGranularityModeSet", 2, cpssDxChHsrPrpDdeTimerGranularityModeSet_PARAMS, NULL},
    {"cpssDxChHsrPrpDdeTimerGranularityModeGet", 2, cpssDxChHsrPrpDdeTimerGranularityModeGet_PARAMS, NULL},
    {"cpssDxChHsrPrpDdeTimeFieldSet", 3, cpssDxChHsrPrpDdeTimeFieldSet_PARAMS, NULL},
    {"cpssDxChHsrPrpDdeTimeFieldGet", 3, cpssDxChHsrPrpDdeTimeFieldGet_PARAMS, NULL},
    {"cpssDxChHsrPrpPortPrpTrailerActionSet", 3, cpssDxChHsrPrpPortPrpTrailerActionSet_PARAMS, NULL},
    {"cpssDxChHsrPrpPortPrpTrailerActionGet", 3, cpssDxChHsrPrpPortPrpTrailerActionGet_PARAMS, NULL},
    {"cpssDxChHsrPrpPortPrpPaddingModeSet", 3, cpssDxChHsrPrpPortPrpPaddingModeSet_PARAMS, NULL},
    {"cpssDxChHsrPrpPortPrpPaddingModeGet", 3, cpssDxChHsrPrpPortPrpPaddingModeGet_PARAMS, NULL},
    {"cpssDxChHsrPrpDdeEntryInvalidate", 2, prvCpssLogGenDevNumIndex_PARAMS, NULL},
    {"cpssDxChHsrPrpDdeEntryWrite", 3, cpssDxChHsrPrpDdeEntryWrite_PARAMS, NULL},
    {"cpssDxChHsrPrpDdeEntryRead", 4, cpssDxChHsrPrpDdeEntryRead_PARAMS, NULL},
    {"cpssDxChHsrPrpDdeAgingApply", 4, cpssDxChHsrPrpPntAgingApply_PARAMS, NULL},
    {"cpssDxChHsrPrpDdeHashCalc", 5, cpssDxChHsrPrpDdeHashCalc_PARAMS, NULL},
    {"cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetSet", 3, cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetSet_PARAMS, NULL},
    {"cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetGet", 3, cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetGet_PARAMS, NULL},
    {"cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetSet", 3, cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetSet_PARAMS, NULL},
    {"cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetGet", 3, cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_HSR_PRP(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChHsrPrpLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChHsrPrpLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

