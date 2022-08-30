/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChResourceManagerLog.c
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
#include <cpss/dxCh/dxChxGen/resourceManager/cpssDxChTcamManagerTypes.h>
#include <cpss/dxCh/dxChxGen/resourceManager/private/prvCpssDxChResourceManagerLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssGenDbLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_DXCH_TCAM_MANAGER_ALLOC_METHOD_ENT[]  =
{
    "CPSS_DXCH_TCAM_MANAGER_ALLOC_MIN_E",
    "CPSS_DXCH_TCAM_MANAGER_ALLOC_MID_E",
    "CPSS_DXCH_TCAM_MANAGER_ALLOC_MAX_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TCAM_MANAGER_ALLOC_METHOD_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TCAM_MANAGER_DUMP_TYPE_ENT[]  =
{
    "CPSS_DXCH_TCAM_MANAGER_DUMP_CLIENTS_E",
    "CPSS_DXCH_TCAM_MANAGER_DUMP_ENTRIES_FOR_SPECIFIC_CLIENT_E",
    "CPSS_DXCH_TCAM_MANAGER_DUMP_ENTRIES_FOR_ALL_CLIENTS_E",
    "CPSS_DXCH_TCAM_MANAGER_DUMP_CLIENTS_AND_ENTRIES_FOR_ALL_CLIENTS_E",
    "CPSS_DXCH_TCAM_MANAGER_DUMP_INTERNAL_INFO_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TCAM_MANAGER_DUMP_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TCAM_MANAGER_ENTRIES_RESERVATION_TYPE_ENT[]  =
{
    "CPSS_DXCH_TCAM_MANAGER_STATIC_ENTRIES_RESERVATION_E",
    "CPSS_DXCH_TCAM_MANAGER_DYNAMIC_ENTRIES_RESERVATION_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TCAM_MANAGER_ENTRIES_RESERVATION_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TCAM_MANAGER_ENTRY_TYPE_ENT[]  =
{
    "CPSS_DXCH_TCAM_MANAGER_SINGLE_ENTRY_E",
    "CPSS_DXCH_TCAM_MANAGER_DUAL_ENTRY_E",
    "CPSS_DXCH_TCAM_MANAGER_TRIPLE_ENTRY_E",
    "CPSS_DXCH_TCAM_MANAGER_QUAD_ENTRY_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TCAM_MANAGER_ENTRY_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TCAM_MANAGER_RANGE_UPDATE_METHOD_ENT[]  =
{
    "CPSS_DXCH_TCAM_MANAGER_DO_NOT_MOVE_RANGE_UPDATE_METHOD_E",
    "CPSS_DXCH_TCAM_MANAGER_MOVE_TOP_AND_COMPRESS_RANGE_UPDATE_METHOD_E",
    "CPSS_DXCH_TCAM_MANAGER_MOVE_BOTTOM_AND_COMPRESS_RANGE_UPDATE_METHOD_E",
    "CPSS_DXCH_TCAM_MANAGER_MOVE_MIDDLE_AND_COMPRESS_RANGE_UPDATE_METHOD_E",
    "CPSS_DXCH_TCAM_MANAGER_MOVE_TOP_AND_KEEP_OFFSETS_RANGE_UPDATE_METHOD_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TCAM_MANAGER_RANGE_UPDATE_METHOD_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TCAM_MANAGER_TCAM_TYPE_ENT[]  =
{
    "CPSS_DXCH_TCAM_MANAGER_XCAT_AND_ABOVE_ROUTER_TCAM_E",
    "CPSS_DXCH_TCAM_MANAGER_XCAT_AND_ABOVE_PCL_TCAM_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TCAM_MANAGER_TCAM_TYPE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_TCAM_MANAGER_CLIENT_FUNC_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TCAM_MANAGER_CLIENT_FUNC_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, moveToLocationFuncPtr);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, moveToAnywhereFuncPtr);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, checkIfDefaultLocationFuncPtr);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, entryType, CPSS_DXCH_TCAM_MANAGER_ENTRY_TYPE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, amount);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TCAM_MANAGER_RANGE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TCAM_MANAGER_RANGE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, firstLine);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, lastLine);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TCAM_MANAGER_TCAM_LOCATION_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TCAM_MANAGER_TCAM_LOCATION_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, row);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, column);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_TCAM_MANAGER_ALLOC_METHOD_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TCAM_MANAGER_ALLOC_METHOD_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_TCAM_MANAGER_ALLOC_METHOD_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TCAM_MANAGER_CLIENT_FUNC_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_TCAM_MANAGER_CLIENT_FUNC_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_TCAM_MANAGER_CLIENT_FUNC_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_TCAM_MANAGER_DUMP_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TCAM_MANAGER_DUMP_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_TCAM_MANAGER_DUMP_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TCAM_MANAGER_ENTRIES_RESERVATION_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TCAM_MANAGER_ENTRIES_RESERVATION_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_TCAM_MANAGER_ENTRIES_RESERVATION_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TCAM_MANAGER_ENTRIES_RESERVATION_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TCAM_MANAGER_ENTRIES_RESERVATION_TYPE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_TCAM_MANAGER_ENTRIES_RESERVATION_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TCAM_MANAGER_ENTRY_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TCAM_MANAGER_ENTRY_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_TCAM_MANAGER_ENTRY_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TCAM_MANAGER_ENTRY_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TCAM_MANAGER_ENTRY_TYPE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_TCAM_MANAGER_ENTRY_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TCAM_MANAGER_RANGE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_TCAM_MANAGER_RANGE_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_TCAM_MANAGER_RANGE_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_TCAM_MANAGER_RANGE_UPDATE_METHOD_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TCAM_MANAGER_RANGE_UPDATE_METHOD_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_TCAM_MANAGER_RANGE_UPDATE_METHOD_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TCAM_MANAGER_TCAM_LOCATION_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_TCAM_MANAGER_TCAM_LOCATION_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_TCAM_MANAGER_TCAM_LOCATION_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_TCAM_MANAGER_TCAM_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TCAM_MANAGER_TCAM_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_TCAM_MANAGER_TCAM_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TCAM_MANAGER_TCAM_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TCAM_MANAGER_TCAM_TYPE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_TCAM_MANAGER_TCAM_TYPE_ENT);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_sizeOfArrayPtr = {
     "sizeOfArrayPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_sizeOfReservedArrayPtr = {
     "sizeOfReservedArrayPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_sizeofAllocatedArrayPtr = {
     "sizeofAllocatedArrayPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TCAM_MANAGER_ALLOC_METHOD_ENT_allocMethod = {
     "allocMethod", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TCAM_MANAGER_ALLOC_METHOD_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TCAM_MANAGER_CLIENT_FUNC_STC_PTR_clientFuncPtr = {
     "clientFuncPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TCAM_MANAGER_CLIENT_FUNC_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TCAM_MANAGER_DUMP_TYPE_ENT_dumpType = {
     "dumpType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TCAM_MANAGER_DUMP_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TCAM_MANAGER_ENTRIES_RESERVATION_TYPE_ENT_reservationType = {
     "reservationType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TCAM_MANAGER_ENTRIES_RESERVATION_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC_PTR_requestedEntriesArray = {
     "requestedEntriesArray", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TCAM_MANAGER_ENTRY_TYPE_ENT_entryType = {
     "entryType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TCAM_MANAGER_ENTRY_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TCAM_MANAGER_RANGE_STC_PTR_rangePtr = {
     "rangePtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TCAM_MANAGER_RANGE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TCAM_MANAGER_RANGE_STC_PTR_reservationRangePtr = {
     "reservationRangePtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TCAM_MANAGER_RANGE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TCAM_MANAGER_RANGE_UPDATE_METHOD_ENT_rangeUpdateMethod = {
     "rangeUpdateMethod", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TCAM_MANAGER_RANGE_UPDATE_METHOD_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TCAM_MANAGER_TCAM_TYPE_ENT_tcamType = {
     "tcamType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TCAM_MANAGER_TCAM_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_defragEnable = {
     "defragEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_usageOfSpecificClient = {
     "usageOfSpecificClient", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_CHAR_PTR_clientNameArr = {
     "clientNameArr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_CHAR)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_clientId = {
     "clientId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_entryToken = {
     "entryToken", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_firstToken = {
     "firstToken", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_lowerBoundToken = {
     "lowerBoundToken", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_secondToken = {
     "secondToken", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_sizeOfArray = {
     "sizeOfArray", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_upperBoundToken = {
     "upperBoundToken", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_VOID_PTR_clientCookiePtr = {
     "clientCookiePtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_VOID)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_VOID_PTR_tcamManagerHandlerPtr = {
     "tcamManagerHandlerPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_VOID)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TCAM_MANAGER_ENTRIES_RESERVATION_TYPE_ENT_PTR_reservationTypePtr = {
     "reservationTypePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TCAM_MANAGER_ENTRIES_RESERVATION_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC_PTR_allocatedEntriesArray = {
     "allocatedEntriesArray", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC_PTR_reservedEntriesArray = {
     "reservedEntriesArray", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TCAM_MANAGER_ENTRY_TYPE_ENT_PTR_entryTypePtr = {
     "entryTypePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TCAM_MANAGER_ENTRY_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TCAM_MANAGER_RANGE_STC_PTR_rangePtr = {
     "rangePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TCAM_MANAGER_RANGE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TCAM_MANAGER_RANGE_STC_PTR_reservationRangePtr = {
     "reservationRangePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TCAM_MANAGER_RANGE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TCAM_MANAGER_TCAM_LOCATION_STC_PTR_tcamLocationPtr = {
     "tcamLocationPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TCAM_MANAGER_TCAM_LOCATION_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TCAM_MANAGER_TCAM_TYPE_ENT_PTR_tcamTypePtr = {
     "tcamTypePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TCAM_MANAGER_TCAM_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_COMP_RES_PTR_resultPtr = {
     "resultPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_COMP_RES)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_allocEntryTokenPtr = {
     "allocEntryTokenPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_clientIdPtr = {
     "clientIdPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_VOID_PTR_PTR_clientCookiePtrPtr = {
     "clientCookiePtrPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_PTR_MAC(GT_VOID)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_VOID_PTR_PTR_tcamManagerHandlerPtrPtr = {
     "tcamManagerHandlerPtrPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_PTR_MAC(GT_VOID)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamManagerCreate_PARAMS[] =  {
    &DX_IN_CPSS_DXCH_TCAM_MANAGER_TCAM_TYPE_ENT_tcamType,
    &DX_IN_CPSS_DXCH_TCAM_MANAGER_RANGE_STC_PTR_rangePtr,
    &DX_OUT_GT_VOID_PTR_PTR_tcamManagerHandlerPtrPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamManagerDelete_PARAMS[] =  {
    &DX_IN_GT_VOID_PTR_tcamManagerHandlerPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamManagerDump_PARAMS[] =  {
    &DX_IN_GT_VOID_PTR_tcamManagerHandlerPtr,
    &DX_IN_CPSS_DXCH_TCAM_MANAGER_DUMP_TYPE_ENT_dumpType,
    &DX_IN_GT_U32_clientId
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamManagerRangeUpdate_PARAMS[] =  {
    &DX_IN_GT_VOID_PTR_tcamManagerHandlerPtr,
    &DX_IN_CPSS_DXCH_TCAM_MANAGER_RANGE_STC_PTR_rangePtr,
    &DX_IN_CPSS_DXCH_TCAM_MANAGER_RANGE_UPDATE_METHOD_ENT_rangeUpdateMethod
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamManagerClientUnregister_PARAMS[] =  {
    &DX_IN_GT_VOID_PTR_tcamManagerHandlerPtr,
    &DX_IN_GT_U32_clientId
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamManagerClientFuncUpdate_PARAMS[] =  {
    &DX_IN_GT_VOID_PTR_tcamManagerHandlerPtr,
    &DX_IN_GT_U32_clientId,
    &DX_IN_CPSS_DXCH_TCAM_MANAGER_CLIENT_FUNC_STC_PTR_clientFuncPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamManagerEntriesReservationSet_PARAMS[] =  {
    &DX_IN_GT_VOID_PTR_tcamManagerHandlerPtr,
    &DX_IN_GT_U32_clientId,
    &DX_IN_CPSS_DXCH_TCAM_MANAGER_ENTRIES_RESERVATION_TYPE_ENT_reservationType,
    &DX_IN_CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC_PTR_requestedEntriesArray,
    &DX_IN_GT_U32_sizeOfArray,
    &DX_IN_CPSS_DXCH_TCAM_MANAGER_RANGE_STC_PTR_reservationRangePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamManagerAvailableEntriesCheck_PARAMS[] =  {
    &DX_IN_GT_VOID_PTR_tcamManagerHandlerPtr,
    &DX_IN_GT_U32_clientId,
    &DX_IN_CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC_PTR_requestedEntriesArray,
    &DX_IN_GT_U32_sizeOfArray
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamManagerEntryAllocate_PARAMS[] =  {
    &DX_IN_GT_VOID_PTR_tcamManagerHandlerPtr,
    &DX_IN_GT_U32_clientId,
    &DX_IN_CPSS_DXCH_TCAM_MANAGER_ENTRY_TYPE_ENT_entryType,
    &DX_IN_GT_U32_lowerBoundToken,
    &DX_IN_GT_U32_upperBoundToken,
    &DX_IN_CPSS_DXCH_TCAM_MANAGER_ALLOC_METHOD_ENT_allocMethod,
    &DX_IN_GT_BOOL_defragEnable,
    &DX_IN_GT_VOID_PTR_clientCookiePtr,
    &DX_OUT_GT_U32_PTR_allocEntryTokenPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamManagerEntryFree_PARAMS[] =  {
    &DX_IN_GT_VOID_PTR_tcamManagerHandlerPtr,
    &DX_IN_GT_U32_clientId,
    &DX_IN_GT_U32_entryToken
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamManagerEntryClientCookieUpdate_PARAMS[] =  {
    &DX_IN_GT_VOID_PTR_tcamManagerHandlerPtr,
    &DX_IN_GT_U32_clientId,
    &DX_IN_GT_U32_entryToken,
    &DX_IN_GT_VOID_PTR_clientCookiePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamManagerTokenInfoGet_PARAMS[] =  {
    &DX_IN_GT_VOID_PTR_tcamManagerHandlerPtr,
    &DX_IN_GT_U32_clientId,
    &DX_IN_GT_U32_entryToken,
    &DX_OUT_CPSS_DXCH_TCAM_MANAGER_ENTRY_TYPE_ENT_PTR_entryTypePtr,
    &DX_OUT_GT_VOID_PTR_PTR_clientCookiePtrPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamManagerTokenToTcamLocation_PARAMS[] =  {
    &DX_IN_GT_VOID_PTR_tcamManagerHandlerPtr,
    &DX_IN_GT_U32_clientId,
    &DX_IN_GT_U32_entryToken,
    &DX_OUT_CPSS_DXCH_TCAM_MANAGER_TCAM_LOCATION_STC_PTR_tcamLocationPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamManagerTokenCompare_PARAMS[] =  {
    &DX_IN_GT_VOID_PTR_tcamManagerHandlerPtr,
    &DX_IN_GT_U32_clientId,
    &DX_IN_GT_U32_firstToken,
    &DX_IN_GT_U32_secondToken,
    &DX_OUT_GT_COMP_RES_PTR_resultPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamManagerEntriesReservationGet_PARAMS[] =  {
    &DX_IN_GT_VOID_PTR_tcamManagerHandlerPtr,
    &DX_IN_GT_U32_clientId,
    &DX_OUT_CPSS_DXCH_TCAM_MANAGER_ENTRIES_RESERVATION_TYPE_ENT_PTR_reservationTypePtr,
    &DX_OUT_CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC_PTR_reservedEntriesArray,
    &DX_INOUT_GT_U32_PTR_sizeOfArrayPtr,
    &DX_OUT_CPSS_DXCH_TCAM_MANAGER_RANGE_STC_PTR_reservationRangePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamManagerHsuExport_PARAMS[] =  {
    &DX_IN_GT_VOID_PTR_tcamManagerHandlerPtr,
    &DX_INOUT_GT_UINTPTR_PTR_iteratorPtr,
    &DX_INOUT_GT_U32_PTR_hsuBlockMemSizePtr,
    &DX_IN_GT_U8_PTR_hsuBlockMemPtr,
    &DX_OUT_GT_BOOL_PTR_exportCompletePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamManagerHsuImport_PARAMS[] =  {
    &DX_IN_GT_VOID_PTR_tcamManagerHandlerPtr,
    &DX_INOUT_GT_UINTPTR_PTR_iteratorPtr,
    &DX_INOUT_GT_U32_PTR_hsuBlockMemSizePtr,
    &DX_IN_GT_U8_PTR_hsuBlockMemPtr,
    &DX_OUT_GT_BOOL_PTR_importCompletePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamManagerConfigGet_PARAMS[] =  {
    &DX_IN_GT_VOID_PTR_tcamManagerHandlerPtr,
    &DX_OUT_CPSS_DXCH_TCAM_MANAGER_TCAM_TYPE_ENT_PTR_tcamTypePtr,
    &DX_OUT_CPSS_DXCH_TCAM_MANAGER_RANGE_STC_PTR_rangePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamManagerHsuSizeGet_PARAMS[] =  {
    &DX_IN_GT_VOID_PTR_tcamManagerHandlerPtr,
    &DX_OUT_GT_U32_PTR_sizePtr
};
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamManagerClientRegister_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamManagerUsageGet_PARAMS[];


/********* lib API DB *********/

extern void cpssDxChTcamManagerEntriesReservationSet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChTcamManagerEntriesReservationGet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChTcamManagerUsageGet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChTcamManagerAvailableEntriesCheck_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChResourceManagerLogLibDb[] = {
    {"cpssDxChTcamManagerCreate", 3, cpssDxChTcamManagerCreate_PARAMS, NULL},
    {"cpssDxChTcamManagerConfigGet", 3, cpssDxChTcamManagerConfigGet_PARAMS, NULL},
    {"cpssDxChTcamManagerDelete", 1, cpssDxChTcamManagerDelete_PARAMS, NULL},
    {"cpssDxChTcamManagerRangeUpdate", 3, cpssDxChTcamManagerRangeUpdate_PARAMS, NULL},
    {"cpssDxChTcamManagerClientRegister", 4, cpssDxChTcamManagerClientRegister_PARAMS, NULL},
    {"cpssDxChTcamManagerClientUnregister", 2, cpssDxChTcamManagerClientUnregister_PARAMS, NULL},
    {"cpssDxChTcamManagerClientFuncUpdate", 3, cpssDxChTcamManagerClientFuncUpdate_PARAMS, NULL},
    {"cpssDxChTcamManagerEntriesReservationSet", 6, cpssDxChTcamManagerEntriesReservationSet_PARAMS, cpssDxChTcamManagerEntriesReservationSet_preLogic},
    {"cpssDxChTcamManagerEntriesReservationGet", 6, cpssDxChTcamManagerEntriesReservationGet_PARAMS, cpssDxChTcamManagerEntriesReservationGet_preLogic},
    {"cpssDxChTcamManagerUsageGet", 7, cpssDxChTcamManagerUsageGet_PARAMS, cpssDxChTcamManagerUsageGet_preLogic},
    {"cpssDxChTcamManagerDump", 3, cpssDxChTcamManagerDump_PARAMS, NULL},
    {"cpssDxChTcamManagerEntryAllocate", 9, cpssDxChTcamManagerEntryAllocate_PARAMS, NULL},
    {"cpssDxChTcamManagerEntryClientCookieUpdate", 4, cpssDxChTcamManagerEntryClientCookieUpdate_PARAMS, NULL},
    {"cpssDxChTcamManagerEntryFree", 3, cpssDxChTcamManagerEntryFree_PARAMS, NULL},
    {"cpssDxChTcamManagerAvailableEntriesCheck", 4, cpssDxChTcamManagerAvailableEntriesCheck_PARAMS, cpssDxChTcamManagerAvailableEntriesCheck_preLogic},
    {"cpssDxChTcamManagerTokenToTcamLocation", 4, cpssDxChTcamManagerTokenToTcamLocation_PARAMS, NULL},
    {"cpssDxChTcamManagerTokenCompare", 5, cpssDxChTcamManagerTokenCompare_PARAMS, NULL},
    {"cpssDxChTcamManagerTokenInfoGet", 5, cpssDxChTcamManagerTokenInfoGet_PARAMS, NULL},
    {"cpssDxChTcamManagerHsuSizeGet", 2, cpssDxChTcamManagerHsuSizeGet_PARAMS, NULL},
    {"cpssDxChTcamManagerHsuExport", 5, cpssDxChTcamManagerHsuExport_PARAMS, NULL},
    {"cpssDxChTcamManagerHsuImport", 5, cpssDxChTcamManagerHsuImport_PARAMS, NULL},
    {"cpssDxChTcamManagerHsuInactiveClientDelete", 1, cpssDxChTcamManagerDelete_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_RESOURCE_MANAGER(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChResourceManagerLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChResourceManagerLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

