/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChResourceManagerLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssDxChResourceManagerLogh
#define __prvCpssDxChResourceManagerLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* manually implemented declarations *********/

void prvCpssLogParamFunc_CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_TCAM_MANAGER_ALLOC_METHOD_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_TCAM_MANAGER_DUMP_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_TCAM_MANAGER_ENTRIES_RESERVATION_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_TCAM_MANAGER_ENTRY_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_TCAM_MANAGER_RANGE_UPDATE_METHOD_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_TCAM_MANAGER_TCAM_TYPE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_TCAM_MANAGER_CLIENT_FUNC_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_TCAM_MANAGER_RANGE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_TCAM_MANAGER_TCAM_LOCATION_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_TCAM_MANAGER_ALLOC_METHOD_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TCAM_MANAGER_CLIENT_FUNC_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TCAM_MANAGER_DUMP_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TCAM_MANAGER_ENTRIES_RESERVATION_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TCAM_MANAGER_ENTRIES_RESERVATION_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TCAM_MANAGER_ENTRY_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TCAM_MANAGER_ENTRY_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TCAM_MANAGER_RANGE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TCAM_MANAGER_RANGE_UPDATE_METHOD_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TCAM_MANAGER_TCAM_LOCATION_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TCAM_MANAGER_TCAM_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TCAM_MANAGER_TCAM_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_sizeOfArrayPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_sizeOfReservedArrayPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_sizeofAllocatedArrayPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TCAM_MANAGER_ALLOC_METHOD_ENT_allocMethod;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TCAM_MANAGER_CLIENT_FUNC_STC_PTR_clientFuncPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TCAM_MANAGER_DUMP_TYPE_ENT_dumpType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TCAM_MANAGER_ENTRIES_RESERVATION_TYPE_ENT_reservationType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC_PTR_requestedEntriesArray;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TCAM_MANAGER_ENTRY_TYPE_ENT_entryType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TCAM_MANAGER_RANGE_STC_PTR_rangePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TCAM_MANAGER_RANGE_STC_PTR_reservationRangePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TCAM_MANAGER_RANGE_UPDATE_METHOD_ENT_rangeUpdateMethod;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TCAM_MANAGER_TCAM_TYPE_ENT_tcamType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_defragEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_usageOfSpecificClient;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_CHAR_PTR_clientNameArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_clientId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_entryToken;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_firstToken;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_lowerBoundToken;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_secondToken;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_sizeOfArray;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_upperBoundToken;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_VOID_PTR_clientCookiePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_VOID_PTR_tcamManagerHandlerPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TCAM_MANAGER_ENTRIES_RESERVATION_TYPE_ENT_PTR_reservationTypePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC_PTR_allocatedEntriesArray;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TCAM_MANAGER_ENTRY_AMOUNT_STC_PTR_reservedEntriesArray;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TCAM_MANAGER_ENTRY_TYPE_ENT_PTR_entryTypePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TCAM_MANAGER_RANGE_STC_PTR_rangePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TCAM_MANAGER_RANGE_STC_PTR_reservationRangePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TCAM_MANAGER_TCAM_LOCATION_STC_PTR_tcamLocationPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TCAM_MANAGER_TCAM_TYPE_ENT_PTR_tcamTypePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_COMP_RES_PTR_resultPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_allocEntryTokenPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_clientIdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_VOID_PTR_PTR_clientCookiePtrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_VOID_PTR_PTR_tcamManagerHandlerPtrPtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssDxChTcamManagerCreate_E = (CPSS_LOG_LIB_RESOURCE_MANAGER_E << 16) | (1 << 24),
    PRV_CPSS_LOG_FUNC_cpssDxChTcamManagerConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamManagerDelete_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamManagerRangeUpdate_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamManagerClientRegister_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamManagerClientUnregister_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamManagerClientFuncUpdate_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamManagerEntriesReservationSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamManagerEntriesReservationGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamManagerUsageGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamManagerDump_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamManagerEntryAllocate_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamManagerEntryClientCookieUpdate_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamManagerEntryFree_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamManagerAvailableEntriesCheck_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamManagerTokenToTcamLocation_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamManagerTokenCompare_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamManagerTokenInfoGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamManagerHsuSizeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamManagerHsuExport_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamManagerHsuImport_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamManagerHsuInactiveClientDelete_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssDxChResourceManagerLogh */
