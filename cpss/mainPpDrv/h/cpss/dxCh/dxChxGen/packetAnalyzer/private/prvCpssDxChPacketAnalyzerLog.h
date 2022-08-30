/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChPacketAnalyzerLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssDxChPacketAnalyzerLogh
#define __prvCpssDxChPacketAnalyzerLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PACKET_ANALYZER_SEARCH_ATTRIBUTE_TYPE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_PACKET_ANALYZER_ACTION_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PACKET_ANALYZER_GROUP_ATTRIBUTES_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PACKET_ANALYZER_INTERNAL_FIELD_VALUE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PACKET_ANALYZER_SEARCH_ATTRIBUTE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PACKET_ANALYZER_COUNTER_INTERRUPT_THRESH_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_PACKET_ANALYZER_ACTION_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PACKET_ANALYZER_GROUP_ATTRIBUTES_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PACKET_ANALYZER_INTERNAL_FIELD_VALUE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PACKET_ANALYZER_SEARCH_ATTRIBUTE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC_PTR_sampleFieldsValueArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_numOfApplicStagesPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_numOfFieldsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_numOfMatchedStagesPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_numOfMuxStagesPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_numOfSampleFieldsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_numOfStagesPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_numOfValidStagesPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PACKET_ANALYZER_ACTION_STC_PTR_actionPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ENT_fieldMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT_PTR_fieldsArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT_fieldName;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT_udfId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC_PTR_fieldNamePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC_PTR_fieldsValueArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PACKET_ANALYZER_GROUP_ATTRIBUTES_STC_PTR_groupAttrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC_PTR_keyAttrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT_PTR_stagesArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT_stageId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT_udsId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC_PTR_ruleAttrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PACKET_ANALYZER_SEARCH_ATTRIBUTE_STC_PTR_searchAttributePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC_PTR_interfaceAttrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC_PTR_udfAttrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC_PTR_udsAttrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_actionId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_groupId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_keyId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_managerId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numOfFields;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numOfStages;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PACKET_ANALYZER_ACTION_STC_PTR_actionPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ENT_PTR_fieldModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT_PTR_fieldsArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT_PTR_udfIdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC_PTR_fieldNamePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC_PTR_fieldsValueArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC_PTR_sampleFieldsValueArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PACKET_ANALYZER_INTERNAL_FIELD_VALUE_STC_PTR_sampleFieldsValueArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC_PTR_keyAttrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT_PTR_applicStagesListArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT_PTR_matchedStagesArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT_PTR_muxStagesListArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT_PTR_stagesArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT_PTR_udsIdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT_PTR_validStagesListArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC_PTR_ruleAttrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC_PTR_interfaceAttrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC_PTR_udfAttrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC_PTR_udsAttrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_deviceStatePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_actionIdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_keyIdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_numOfHitsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_devicesArr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerManagerCreate_E = (CPSS_LOG_LIB_PACKET_ANALYZER_E << 16) | (1 << 24),
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerManagerDelete_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerManagerDeviceAdd_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerManagerDeviceRemove_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerManagerEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerManagerDevicesGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerManagerResetToDefaults_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerFieldSizeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerLogicalKeyCreate_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerLogicalKeyFieldsAdd_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerLogicalKeyFieldsRemove_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerLogicalKeyDelete_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerLogicalKeyInfoGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerLogicalKeyStagesGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerStagesGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerMuxStagesGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerGroupCreate_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerGroupDelete_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerGroupRuleAdd_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerGroupRuleUpdate_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerGroupRuleDelete_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerGroupRuleGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerActionCreate_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerActionUpdate_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerActionDelete_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerActionGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerSampledDataCountersClear_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerActionSamplingEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerGroupActivateEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerRuleMatchStagesGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerStageMatchDataGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerStageMatchDataAllInternalFieldsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerStageMatchDataAllInternalFieldsByAttributesGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerStageFieldsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerFieldStagesGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerMuxStageBind_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerMuxStageUnbind_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerUserDefinedFieldAdd_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerUserDefinedFieldDelete_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerUserDefinedFieldGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerUserDefinedFieldInfoGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerUserDefinedStageAdd_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerUserDefinedStageDelete_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerUserDefinedStageGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerUserDefinedStageInfoGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerRuleMatchDataAllFieldsByAttributeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPacketAnalyzerStageFieldOverlappingFieldsGet_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssDxChPacketAnalyzerLogh */
