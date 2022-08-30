/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssPxCosLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssPxCosLogh
#define __prvCpssPxCosLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_COS_MODE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_PX_COS_ATTRIBUTES_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_COS_FORMAT_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_PX_COS_ATTRIBUTES_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_COS_FORMAT_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_COS_ATTRIBUTES_STC_PTR_cosAttributesPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_COS_FORMAT_ENTRY_STC_PTR_cosFormatEntryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_dsaIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_l2Index;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_l3Index;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_mplsIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_COS_ATTRIBUTES_STC_PTR_cosAttributesPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_COS_FORMAT_ENTRY_STC_PTR_cosFormatEntryPtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssPxCosFormatEntrySet_E = (CPSS_LOG_LIB_COS_E << 16) | (2 << 24),
    PRV_CPSS_LOG_FUNC_cpssPxCosFormatEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCosPortAttributesSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCosPortAttributesGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCosPortL2MappingSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCosPortL2MappingGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCosL3MappingSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCosL3MappingGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCosMplsMappingSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCosMplsMappingGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCosDsaMappingSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCosDsaMappingGet_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssPxCosLogh */
