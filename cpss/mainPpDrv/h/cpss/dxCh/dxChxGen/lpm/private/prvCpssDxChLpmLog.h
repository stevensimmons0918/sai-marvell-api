/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChLpmLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssDxChLpmLogh
#define __prvCpssDxChLpmLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* manually implemented declarations *********/

void prvCpssLogParamFunc_CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_LPM_EXCEPTION_STATUS_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_LPM_LEAF_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_LPM_COMPRESSED_1_BIT_VECTOR_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_LPM_COMPRESSED_2_BIT_VECTOR_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_LPM_NODE_POINTER_DATA_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_LPM_RAM_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_LPM_REGULAR_BIT_VECTOR_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_LPM_NEXT_NODE_POINTER_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_LPM_NODE_NEXT_HOP_OR_ECMP_POINTER_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_LPM_REGULAR_NODE_BIT_VECTOR_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_LPM_EXCEPTION_STATUS_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_LPM_LEAF_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_LPM_LEAF_ENTRY_STC_PTR_leafPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT_nodeType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC_PTR_nextPointerArrayPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT_PTR_rangeSelectSecPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_bypassEnabled;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_leafIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_lpmEngine;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_lpmLineOffset;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numOfNextPointers;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numOfRanges;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_LPM_EXCEPTION_STATUS_ENT_PTR_hitExceptionPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_LPM_LEAF_ENTRY_STC_PTR_leafPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC_PTR_nextPointerArrayPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT_PTR_rangeSelectSecPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_LPM_RAM_MEM_MODE_ENT_PTR_lpmMemoryModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_bypassEnabledPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_baseOffsetPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_numOfRangesPtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssDxChLpmNodeWrite_E = (CPSS_LOG_LIB_LPM_E << 16) | (1 << 24),
    PRV_CPSS_LOG_FUNC_cpssDxChLpmNodeRead_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLpmNodeNextPointersWrite_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLpmLastLookupStagesBypassEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLpmLastLookupStagesBypassEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLpmPortSipLookupEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLpmPortSipLookupEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLpmLeafEntryWrite_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLpmLeafEntryRead_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLpmPbrBaseAndSizeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLpmExceptionStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChLpmMemoryModeGet_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssDxChLpmLogh */
