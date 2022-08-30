/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChExactMatchLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssDxChExactMatchLogh
#define __prvCpssDxChExactMatchLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_EXACT_MATCH_CLIENT_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_EXACT_MATCH_MHT_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_EXACT_MATCH_TABLE_SIZE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_ACTION_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FAIL_COUNTERS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_ALLOC_STATUS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_KEY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_EXPANDED_PCL_ACTION_ORIGIN_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_EXPANDED_TTI_ACTION_ORIGIN_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_ACTION_UNT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FAIL_COUNTERS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_ALLOC_STATUS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_CLIENT_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_CLIENT_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_KEY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_EXACT_MATCH_MHT_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_EXACT_MATCH_TABLE_SIZE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT_actionType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_ACTION_UNT_PTR_actionPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_PARAMS_STC_PTR_confPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_CLIENT_ENT_clientType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_ENTRY_STC_PTR_entryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT_PTR_expandedActionOriginPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_KEY_STC_PTR_entryKeyPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT_exactMatchAutoLearnLookupNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT_exactMatchLookupNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT_pclLookupNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT_ttiLookupNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_ENT_profileIdMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC_PTR_keyParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TTI_RULE_UNT_PTR_maskFixedKeyFormatPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TTI_RULE_UNT_PTR_patternFixedKeyFormatPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_EXACT_MATCH_MHT_ENT_exactMatchMht;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_EXACT_MATCH_TABLE_SIZE_ENT_exactMatchSize;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_actionEn;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_defActionEn;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_enableExactMatchLookup;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_exactMatchClearActivity;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_exactMatchEntryIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_exactMatchProfileIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numOfRules;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_subProfileId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_PTR_maskUdbFormatArray;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_PTR_patternUdbFormatArray;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT_PTR_actionTypePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_ACTION_UNT_PTR_actionPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FAIL_COUNTERS_STC_PTR_failCounterPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_ALLOC_STATUS_STC_PTR_statusPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_PARAMS_STC_PTR_confPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_CLIENT_ENT_PTR_clientTypePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_ENTRY_STC_PTR_entryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_ENTRY_STC_PTR_keyParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT_PTR_expandedActionOriginPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT_PTR_exactMatchAutoLearnLookupNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_ENT_PTR_profileIdModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC_PTR_keyParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TTI_RULE_UNT_PTR_maskFixedKeyFormatPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TTI_RULE_UNT_PTR_patternFixedKeyFormatPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_actionEnPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_defActionEnPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_enableExactMatchLookupPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_exactMatchActivityStatusPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_exactMatchCrcMultiHashArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_exactMatchIndexPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_expandedActionIndexPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_numberOfElemInCrcMultiHashArrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_maskUdbFormatArray;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_patternUdbFormatArray;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchTtiProfileIdModePacketTypeSet_E = (CPSS_LOG_LIB_EXACT_MATCH_E << 16) | (1 << 24),
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchTtiProfileIdModePacketTypeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchTtiPortProfileIdModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchTtiPortProfileIdModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchTtiPortProfileIdModePortSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchTtiPortProfileIdModePortGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchPclProfileIdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchPclProfileIdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchClientLookupSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchClientLookupGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchActivityBitEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchActivityBitEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchActivityStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchProfileKeyParamsSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchProfileKeyParamsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchProfileDefaultActionSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchProfileDefaultActionGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchExpandedActionSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchExpandedActionGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchPortGroupEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchPortGroupEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchPortGroupEntryInvalidate_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchPortGroupEntryStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchHashCrcMultiResultsByParamsCalc_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchRulesDump_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchTtiRuleConvertToUdbFormat_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchTtiRuleConvertFromUdbFormat_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchAutoLearnLookupSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchAutoLearnLookupGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchAutoLearnProfileDefaultActionSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchAutoLearnProfileDefaultActionGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchAutoLearnProfileDefaultActionEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchPortGroupAutoLearnFlowIdParamSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchPortGroupAutoLearnFlowIdParamGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchPortGroupAutoLearnFlowIdAllocStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchPortGroupAutoLearnEntryGetNext_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchPortGroupAutoLearnCollisionKeyParamsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchPortGroupAutoLearnEntryRecycleGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger_E,
    PRV_CPSS_LOG_FUNC_cpssDxChExactMatchPortGroupAutoLearnFailCountersGet_E,
    PRV_CPSS_LOG_FUNC_prvCpssDxChExactMatchHashCrcMultiResultsByParamsCalcExt_E,
    PRV_CPSS_LOG_FUNC_prvCpssDxChExactMatchHashCrcMultiResultsByParamsCalc_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssDxChExactMatchLogh */
