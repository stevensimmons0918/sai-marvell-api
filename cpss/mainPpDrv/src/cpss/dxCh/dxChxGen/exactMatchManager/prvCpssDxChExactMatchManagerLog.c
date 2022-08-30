/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChExactMatchManagerLog.c
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
#include <cpss/dxCh/dxChxGen/exactMatch/private/prvCpssDxChExactMatchLog.h>
#include <cpss/dxCh/dxChxGen/exactMatchManager/cpssDxChExactMatchManagerTypes.h>
#include <cpss/dxCh/dxChxGen/exactMatchManager/private/prvCpssDxChExactMatchManagerLog.h>
#include <cpss/dxCh/dxChxGen/pcl/private/prvCpssDxChPclLog.h>
#include <cpss/dxCh/dxChxGen/tti/private/prvCpssDxChTtiLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssGenDbLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT[]  =
{
    "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_GLOBAL_PARAM_HW_INVALID_E",
    "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_TYPE_COUNTERS_INVALID_E",
    "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_BANK_COUNTERS_INVALID_E",
    "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_CUCKOO_BANK_COUNTERS_MISMATCH_E",
    "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_FREE_LIST_1ST_PTR_INVALID_E",
    "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_FREE_LIST_ENTRIES_NUM_MISMATCH_E",
    "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_1ST_PTR_INVALID_E",
    "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_LAST_PTR_INVALID_E",
    "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_GETNEXT_PTR_INVALID_E",
    "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_DELETE_SCAN_PTR_INVALID_E",
    "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_ENTRY_PREV_PTR_INVALID_E",
    "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_ENTRIES_NUM_MISMATCH_E",
    "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_INDEX_POINT_INVALID_ENTRY_E",
    "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_ENTRY_INDEX_INVALID_E",
    "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_BIN_ENTRIES_NUM_MISMATCH_E",
    "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E",
    "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_ENTRY_BIN_INDEX_INVALID_E",
    "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_SCAN_PTR_INVALID_E",
    "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L1_INVALID_E",
    "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L2_INVALID_E",
    "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_ENTRY_INVALID_E",
    "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_CLIENT_INVALID_E",
    "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_PROFILE_ID_INVALID_E",
    "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_KEY_PARAMS_INVALID_E",
    "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_DEFAULT_ACTION_ENABLE_INVALID_E",
    "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_DEFAULT_ACTION_INVALID_E",
    "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_EXPANDER_CONFIG_INVALID_E",
    "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_CUCKOO_ENTRY_INVALID_E",
    "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS___LAST___E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_TYPE_ENT[]  =
{
    "CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_TYPE_AGED_OUT_E",
    "CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_TYPE_AGED_DELETED_E",
    "CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_TYPE_DELETED_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_TYPE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_SCAN_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_SCAN_PARAMS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, checkAgeLookupEntries, CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E, GT_BOOL);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, checkAgeClientEntries, CPSS_DXCH_EXACT_MATCH_ACTION_LAST_E, GT_BOOL);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, checkAgeKeySizeEntries, CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E, GT_BOOL);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, pattern, CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS, GT_U8);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, mask, CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS, GT_U8);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, userDefinedPattern);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, userDefinedMask);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, deleteAgeoutLookupEntries, CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E, GT_BOOL);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, deleteAgeoutClientEntries, CPSS_DXCH_EXACT_MATCH_ACTION_LAST_E, GT_BOOL);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, deleteAgeoutKeySizeEntries, CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E, GT_BOOL);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, agingRefreshEnable);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, hwCapacity, CPSS_DXCH_EXACT_MATCH_MANAGER_HW_CAPACITY_STC);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxTotalEntries);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxEntriesPerAgingScan);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxEntriesPerDeleteScan);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_MANAGER_COUNTERS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_COUNTERS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, ttiClientKeySizeEntriesArray, CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E, GT_U32);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, ipcl0ClientKeySizeEntriesArray, CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E, GT_U32);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, ipcl1ClientKeySizeEntriesArray, CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E, GT_U32);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, ipcl2ClientKeySizeEntriesArray, CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E, GT_U32);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, epclClientKeySizeEntriesArray, CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E, GT_U32);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, usedEntriesIndexes);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, freeEntriesIndexes);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, bankCounters, CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_NUM_BANKS_CNS, GT_U32);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, globalCheckEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, countersCheckEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, dbFreeListCheckEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, dbUsedListCheckEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, dbIndexPointerCheckEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, dbAgingBinCheckEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, dbAgingBinUsageMatrixCheckEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, hwUsedListCheckEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, cuckooDbCheckEnable);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_MANAGER_DELETE_SCAN_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_DELETE_SCAN_PARAMS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, deleteLookupEntries, CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E, GT_BOOL);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, deleteClientEntries, CPSS_DXCH_EXACT_MATCH_ACTION_LAST_E, GT_BOOL);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, deleteKeySizeEntries, CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E, GT_BOOL);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, pattern, CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS, GT_U8);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, mask, CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS, GT_U8);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, userDefinedPattern);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, userDefinedMask);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, devNum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, portGroupsBmp);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, rehashEnable);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, expandedArray, CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_ENTRIES_NUM_CNS, CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, exactMatchEntry, CPSS_DXCH_EXACT_MATCH_ENTRY_STC);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, exactMatchActionType, CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, exactMatchAction, CPSS_DXCH_EXACT_MATCH_ACTION_UNT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, expandedActionIndex);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, exactMatchUserDefined);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, updateType, CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_TYPE_ENT);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, entry, CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, exactMatchExpandedEntryValid);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, expandedActionType, CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, expandedAction, CPSS_DXCH_EXACT_MATCH_ACTION_UNT);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, expandedActionOrigin, CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, lookupsArray, CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E, CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_CONFIG_STC);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, profileEntryParamsArray, CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_PROFILES_NUM_CNS, CPSS_DXCH_EXACT_MATCH_MANAGER_PROFILE_ENTRY_PARAMS_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_MANAGER_STATISTICS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_STATISTICS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryAddOkRehashingStage0);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryAddOkRehashingStage1);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryAddOkRehashingStage2);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryAddOkRehashingStage3);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryAddOkRehashingStage4);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryAddOkRehashingStageMoreThanFour);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryAddErrorInputInvalid);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryAddErrorBadState);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryAddErrorEntryExist);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryAddErrorTableFull);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryAddErrorReplayEntryNotFound);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryUpdateOk);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryUpdateErrorInputInvalid);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryUpdateErrorNotFound);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryDeleteOk);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryDeleteErrorInputInvalid);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryDeleteErrorNotfound);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, scanDeleteOk);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, scanDeleteOkNoMore);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, scanDeleteErrorInputInvalid);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, scanDeleteTotalDeletedEntries);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, scanAgingOk);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, scanAgingErrorInputInvalid);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, scanAgingTotalAgedOutEntries);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, scanAgingTotalAgedOutDeleteEntries);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entriesRewriteOk);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entriesRewriteErrorInputInvalid);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entriesRewriteTotalRewrite);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_MANAGER_HW_CAPACITY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_HW_CAPACITY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numOfHwIndexes);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numOfHashes);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_CONFIG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, lookupEnable);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, lookupClient, CPSS_DXCH_EXACT_MATCH_CLIENT_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, lookupClientMappingsNum);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, lookupClientMappingsArray, MAX_NUM_PROFILE_ID_MAPPING_ENTRIES_CNS, CPSS_DXCH_EXACT_MATCH_MANAGER_PROFILE_ID_MAPPING_UNT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_MANAGER_PROFILE_ENTRY_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_PROFILE_ENTRY_PARAMS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, isValidProfileId);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, keyParams, CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, defaultActionType, CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, defaultAction, CPSS_DXCH_EXACT_MATCH_ACTION_UNT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, defaultActionEn);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_MANAGER_PROFILE_ID_MAPPING_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_PROFILE_ID_MAPPING_UNT *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ttiMappingElem, CPSS_DXCH_EXACT_MATCH_MANAGER_PROFILE_ID_MAPPING_TTI_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, pclMappingElem, CPSS_DXCH_EXACT_MATCH_MANAGER_PROFILE_ID_MAPPING_PCL_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_MANAGER_PROFILE_ID_MAPPING_PCL_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_PROFILE_ID_MAPPING_PCL_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, packetType, CPSS_DXCH_PCL_PACKET_TYPE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, subProfileId);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, enableExactMatchLookup);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, profileId);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_MANAGER_PROFILE_ID_MAPPING_TTI_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_PROFILE_ID_MAPPING_TTI_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, keyType, CPSS_DXCH_TTI_KEY_TYPE_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, enableExactMatchLookup);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, profileId);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_SCAN_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_SCAN_PARAMS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_SCAN_PARAMS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_MANAGER_COUNTERS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_COUNTERS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_MANAGER_COUNTERS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_MANAGER_DELETE_SCAN_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_DELETE_SCAN_PARAMS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_MANAGER_DELETE_SCAN_PARAMS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_MANAGER_STATISTICS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_STATISTICS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_MANAGER_STATISTICS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC_PTR_entryPtr = {
     "entryPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_numOfPairsPtr = {
     "numOfPairsPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_SCAN_PARAMS_STC_PTR_paramsPtr = {
     "paramsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_SCAN_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC_PTR_agingPtr = {
     "agingPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC_PTR_capacityPtr = {
     "capacityPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_STC_PTR_checksPtr = {
     "checksPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_MANAGER_DELETE_SCAN_PARAMS_STC_PTR_paramsPtr = {
     "paramsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_DELETE_SCAN_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC_PTR_pairListArr = {
     "pairListArr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC_PTR_paramsPtr = {
     "paramsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC_PTR_entryAttrPtr = {
     "entryAttrPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC_PTR_entryPtr = {
     "entryPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC_PTR_paramsPtr = {
     "paramsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC_PTR_lookupPtr = {
     "lookupPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_exactMatchScanStart = {
     "exactMatchScanStart", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_exactMatchManagerId = {
     "exactMatchManagerId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numOfPairs = {
     "numOfPairs", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC_PTR_agingPtr = {
     "agingPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC_PTR_capacityPtr = {
     "capacityPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_MANAGER_COUNTERS_STC_PTR_countersPtr = {
     "countersPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_COUNTERS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT_PTR_resultArray = {
     "resultArray", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC_PTR_pairListManagedArray = {
     "pairListManagedArray", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC_PTR_entryAttrPtr = {
     "entryAttrPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC_PTR_entryPtr = {
     "entryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC_PTR_entriesAgedOutArray = {
     "entriesAgedOutArray", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC_PTR_entriesDeletedArray = {
     "entriesDeletedArray", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC_PTR_paramsPtr = {
     "paramsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC_PTR_lookupPtr = {
     "lookupPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_MANAGER_STATISTICS_STC_PTR_statisticsPtr = {
     "statisticsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_STATISTICS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_entriesAgedOutNumPtr = {
     "entriesAgedOutNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_entriesDeletedNumberPtr = {
     "entriesDeletedNumberPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_errorNumberPtr = {
     "errorNumberPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchManagerDelete_PARAMS[] =  {
    &DX_IN_GT_U32_exactMatchManagerId
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchManagerAgingScan_PARAMS[] =  {
    &DX_IN_GT_U32_exactMatchManagerId,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_SCAN_PARAMS_STC_PTR_paramsPtr,
    &DX_OUT_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC_PTR_entriesAgedOutArray,
    &DX_OUT_GT_U32_PTR_entriesAgedOutNumPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchManagerCreate_PARAMS[] =  {
    &DX_IN_GT_U32_exactMatchManagerId,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC_PTR_capacityPtr,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC_PTR_lookupPtr,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC_PTR_entryAttrPtr,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC_PTR_agingPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchManagerDatabaseCheck_PARAMS[] =  {
    &DX_IN_GT_U32_exactMatchManagerId,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_STC_PTR_checksPtr,
    &DX_OUT_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT_PTR_resultArray,
    &DX_OUT_GT_U32_PTR_errorNumberPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchManagerDevListAdd_PARAMS[] =  {
    &DX_IN_GT_U32_exactMatchManagerId,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC_PTR_pairListArr,
    &DX_IN_GT_U32_numOfPairs
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchManagerEntryUpdate_PARAMS[] =  {
    &DX_IN_GT_U32_exactMatchManagerId,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC_PTR_entryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchManagerEntryAdd_PARAMS[] =  {
    &DX_IN_GT_U32_exactMatchManagerId,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC_PTR_entryPtr,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC_PTR_paramsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchManagerDeleteScan_PARAMS[] =  {
    &DX_IN_GT_U32_exactMatchManagerId,
    &DX_IN_GT_BOOL_exactMatchScanStart,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_MANAGER_DELETE_SCAN_PARAMS_STC_PTR_paramsPtr,
    &DX_OUT_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC_PTR_entriesDeletedArray,
    &DX_OUT_GT_U32_PTR_entriesDeletedNumberPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchManagerEntryGetNext_PARAMS[] =  {
    &DX_IN_GT_U32_exactMatchManagerId,
    &DX_IN_GT_BOOL_getFirst,
    &DX_OUT_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC_PTR_entryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchManagerEntryRewrite_PARAMS[] =  {
    &DX_IN_GT_U32_exactMatchManagerId,
    &DX_IN_GT_U32_PTR_entriesIndexesArray,
    &DX_IN_GT_U32_entriesIndexesNum
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchManagerExpandedActionUpdate_PARAMS[] =  {
    &DX_IN_GT_U32_exactMatchManagerId,
    &DX_IN_GT_U32_expandedActionIndex,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC_PTR_paramsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchManagerExpandedActionGet_PARAMS[] =  {
    &DX_IN_GT_U32_exactMatchManagerId,
    &DX_IN_GT_U32_expandedActionIndex,
    &DX_OUT_CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC_PTR_paramsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchManagerEntryGet_PARAMS[] =  {
    &DX_IN_GT_U32_exactMatchManagerId,
    &DX_INOUT_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC_PTR_entryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchManagerDevListGet_PARAMS[] =  {
    &DX_IN_GT_U32_exactMatchManagerId,
    &DX_INOUT_GT_U32_PTR_numOfPairsPtr,
    &DX_OUT_CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC_PTR_pairListManagedArray
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchManagerConfigGet_PARAMS[] =  {
    &DX_IN_GT_U32_exactMatchManagerId,
    &DX_OUT_CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC_PTR_capacityPtr,
    &DX_OUT_CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC_PTR_lookupPtr,
    &DX_OUT_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC_PTR_entryAttrPtr,
    &DX_OUT_CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC_PTR_agingPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchManagerCountersGet_PARAMS[] =  {
    &DX_IN_GT_U32_exactMatchManagerId,
    &DX_OUT_CPSS_DXCH_EXACT_MATCH_MANAGER_COUNTERS_STC_PTR_countersPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchManagerStatisticsGet_PARAMS[] =  {
    &DX_IN_GT_U32_exactMatchManagerId,
    &DX_OUT_CPSS_DXCH_EXACT_MATCH_MANAGER_STATISTICS_STC_PTR_statisticsPtr
};


/********* lib API DB *********/

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChExactMatchManagerLogLibDb[] = {
    {"cpssDxChExactMatchManagerCreate", 5, cpssDxChExactMatchManagerCreate_PARAMS, NULL},
    {"cpssDxChExactMatchManagerDelete", 1, cpssDxChExactMatchManagerDelete_PARAMS, NULL},
    {"cpssDxChExactMatchManagerDevListAdd", 3, cpssDxChExactMatchManagerDevListAdd_PARAMS, NULL},
    {"cpssDxChExactMatchManagerDevListRemove", 3, cpssDxChExactMatchManagerDevListAdd_PARAMS, NULL},
    {"cpssDxChExactMatchManagerEntryAdd", 3, cpssDxChExactMatchManagerEntryAdd_PARAMS, NULL},
    {"cpssDxChExactMatchManagerEntryUpdate", 2, cpssDxChExactMatchManagerEntryUpdate_PARAMS, NULL},
    {"cpssDxChExactMatchManagerEntryDelete", 2, cpssDxChExactMatchManagerEntryUpdate_PARAMS, NULL},
    {"cpssDxChExactMatchManagerEntryGet", 2, cpssDxChExactMatchManagerEntryGet_PARAMS, NULL},
    {"cpssDxChExactMatchManagerEntryGetNext", 3, cpssDxChExactMatchManagerEntryGetNext_PARAMS, NULL},
    {"cpssDxChExactMatchManagerExpandedActionUpdate", 3, cpssDxChExactMatchManagerExpandedActionUpdate_PARAMS, NULL},
    {"cpssDxChExactMatchManagerExpandedActionGet", 3, cpssDxChExactMatchManagerExpandedActionGet_PARAMS, NULL},
    {"cpssDxChExactMatchManagerConfigGet", 5, cpssDxChExactMatchManagerConfigGet_PARAMS, NULL},
    {"cpssDxChExactMatchManagerDevListGet", 3, cpssDxChExactMatchManagerDevListGet_PARAMS, NULL},
    {"cpssDxChExactMatchManagerCountersGet", 2, cpssDxChExactMatchManagerCountersGet_PARAMS, NULL},
    {"cpssDxChExactMatchManagerStatisticsGet", 2, cpssDxChExactMatchManagerStatisticsGet_PARAMS, NULL},
    {"cpssDxChExactMatchManagerStatisticsClear", 1, cpssDxChExactMatchManagerDelete_PARAMS, NULL},
    {"cpssDxChExactMatchManagerDatabaseCheck", 4, cpssDxChExactMatchManagerDatabaseCheck_PARAMS, NULL},
    {"cpssDxChExactMatchManagerDeleteScan", 5, cpssDxChExactMatchManagerDeleteScan_PARAMS, NULL},
    {"cpssDxChExactMatchManagerAgingScan", 4, cpssDxChExactMatchManagerAgingScan_PARAMS, NULL},
    {"cpssDxChExactMatchManagerEntryRewrite", 3, cpssDxChExactMatchManagerEntryRewrite_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_EXACT_MATCH_MANAGER(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChExactMatchManagerLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChExactMatchManagerLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

