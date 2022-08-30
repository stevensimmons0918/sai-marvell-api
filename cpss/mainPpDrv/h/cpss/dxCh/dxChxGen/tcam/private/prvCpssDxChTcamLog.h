/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChTcamLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssDxChTcamLogh
#define __prvCpssDxChTcamLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* manually implemented declarations *********/

void prvCpssLogParamFunc_CPSS_DXCH_TCAM_BLOCK_INFO_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_TCAM_CLIENT_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_TCAM_RULE_SIZE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_TCAM_BLOCK_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_TCAM_LOOKUP_MUX_TABLE_LINE_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_TCAM_LOOKUP_PROFILE_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_TCAM_RULE_SIZE_ENT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_TCAM_CLIENT_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TCAM_LOOKUP_MUX_TABLE_LINE_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TCAM_LOOKUP_PROFILE_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TCAM_RULE_SIZE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_TCAM_RULE_SIZE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TCAM_BLOCK_INFO_STC_PTR_floorInfoArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TCAM_CLIENT_ENT_tcamClient;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TCAM_LOOKUP_MUX_TABLE_LINE_CFG_STC_PTR_tcamLookupMuxTableLineCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TCAM_LOOKUP_PROFILE_CFG_STC_PTR_tcamLookupProfileCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TCAM_RULE_SIZE_ENT_ruleSize;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TCAM_RULE_SIZE_ENT_size;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_HEX_PTR_tcamMaskPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_PTR_tcamKeyArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_PTR_tcamPatternPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_floorIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numOfActiveFloors;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_subKeyProfile;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_tcamGroup;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TCAM_BLOCK_INFO_STC_PTR_floorInfoArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TCAM_LOOKUP_MUX_TABLE_LINE_CFG_STC_PTR_tcamLookupMuxTableLineCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TCAM_LOOKUP_PROFILE_CFG_STC_PTR_tcamLookupProfileCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TCAM_RULE_SIZE_ENT_PTR_ruleSizePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TCAM_RULE_SIZE_ENT_PTR_sizePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_isHitPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_HEX_PTR_tcamMaskPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_hitIndexPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_numOfActiveFloorsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_subKeyProfilePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tcamGroupPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tcamKeyArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tcamPatternPtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssDxChTcamPortGroupRuleWrite_E = (CPSS_LOG_LIB_TCAM_E << 16) | (1 << 24),
    PRV_CPSS_LOG_FUNC_cpssDxChTcamPortGroupRuleRead_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamPortGroupRuleValidStatusSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamPortGroupRuleValidStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamPortGroupClientGroupSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamPortGroupClientGroupGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamPortGroupIndexRangeHitNumAndGroupSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamIndexRangeHitNumAndGroupSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamPortGroupIndexRangeHitNumAndGroupGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamIndexRangeHitNumAndGroupGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamPortGroupActiveFloorsSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamActiveFloorsSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamPortGroupActiveFloorsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamActiveFloorsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamPortGroupCpuLookupTriggerSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamPortGroupCpuLookupTriggerGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamCpuLookupResultsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamLookupProfileCfgSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamLookupProfileCfgGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamLookupMuxTableLineSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChTcamLookupMuxTableLineGet_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssDxChTcamLogh */
