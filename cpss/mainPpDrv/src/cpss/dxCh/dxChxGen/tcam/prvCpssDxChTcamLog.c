/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChTcamLog.c
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
#include <cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>
#include <cpss/dxCh/dxChxGen/tcam/private/prvCpssDxChTcamLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssGenDbLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_DXCH_TCAM_CLIENT_ENT[]  =
{
    "CPSS_DXCH_TCAM_IPCL_0_E",
    "CPSS_DXCH_TCAM_IPCL_1_E",
    "CPSS_DXCH_TCAM_IPCL_2_E",
    "CPSS_DXCH_TCAM_EPCL_E",
    "CPSS_DXCH_TCAM_TTI_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TCAM_CLIENT_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_TCAM_RULE_SIZE_ENT[]  =
{
    "CPSS_DXCH_TCAM_RULE_SIZE_10_B_E",
    "CPSS_DXCH_TCAM_RULE_SIZE_20_B_E",
    "CPSS_DXCH_TCAM_RULE_SIZE_30_B_E",
    "CPSS_DXCH_TCAM_RULE_SIZE_40_B_E",
    "CPSS_DXCH_TCAM_RULE_SIZE_50_B_E",
    "CPSS_DXCH_TCAM_RULE_SIZE_60_B_E",
    "CPSS_DXCH_TCAM_RULE_SIZE_80_B_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_TCAM_RULE_SIZE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_TCAM_BLOCK_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TCAM_BLOCK_INFO_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, group);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, hitNum);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TCAM_LOOKUP_MUX_TABLE_LINE_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TCAM_LOOKUP_MUX_TABLE_LINE_CFG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, superKeyTwoByteUnitsOffsets, 30, GT_U8);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TCAM_LOOKUP_PROFILE_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_TCAM_LOOKUP_PROFILE_CFG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, subKey0_3_Sizes, 4, CPSS_DXCH_TCAM_RULE_SIZE_ENT);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, subKey1_3_MuxTableLineIndexes, 3, GT_U32);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, subKey1_3_MuxTableLineOffsets, 3, GT_U32);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_TCAM_RULE_SIZE_ENT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_FIELD_VAL_PTR_MAC(CPSS_DXCH_TCAM_RULE_SIZE_ENT *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *valPtr, CPSS_DXCH_TCAM_RULE_SIZE_ENT);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_TCAM_CLIENT_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TCAM_CLIENT_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_TCAM_CLIENT_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TCAM_LOOKUP_MUX_TABLE_LINE_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_TCAM_LOOKUP_MUX_TABLE_LINE_CFG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_TCAM_LOOKUP_MUX_TABLE_LINE_CFG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_TCAM_LOOKUP_PROFILE_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_TCAM_LOOKUP_PROFILE_CFG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_TCAM_LOOKUP_PROFILE_CFG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_TCAM_RULE_SIZE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TCAM_RULE_SIZE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_TCAM_RULE_SIZE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_TCAM_RULE_SIZE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_TCAM_RULE_SIZE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_TCAM_RULE_SIZE_ENT);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TCAM_BLOCK_INFO_STC_PTR_floorInfoArr = {
     "floorInfoArr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TCAM_BLOCK_INFO_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TCAM_CLIENT_ENT_tcamClient = {
     "tcamClient", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TCAM_CLIENT_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TCAM_LOOKUP_MUX_TABLE_LINE_CFG_STC_PTR_tcamLookupMuxTableLineCfgPtr = {
     "tcamLookupMuxTableLineCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TCAM_LOOKUP_MUX_TABLE_LINE_CFG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TCAM_LOOKUP_PROFILE_CFG_STC_PTR_tcamLookupProfileCfgPtr = {
     "tcamLookupProfileCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TCAM_LOOKUP_PROFILE_CFG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TCAM_RULE_SIZE_ENT_ruleSize = {
     "ruleSize", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TCAM_RULE_SIZE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TCAM_RULE_SIZE_ENT_size = {
     "size", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_TCAM_RULE_SIZE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_HEX_PTR_tcamMaskPtr = {
     "tcamMaskPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32_HEX)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_PTR_tcamKeyArr = {
     "tcamKeyArr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_PTR_tcamPatternPtr = {
     "tcamPatternPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_floorIndex = {
     "floorIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numOfActiveFloors = {
     "numOfActiveFloors", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_subKeyProfile = {
     "subKeyProfile", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_tcamGroup = {
     "tcamGroup", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TCAM_BLOCK_INFO_STC_PTR_floorInfoArr = {
     "floorInfoArr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TCAM_BLOCK_INFO_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TCAM_LOOKUP_MUX_TABLE_LINE_CFG_STC_PTR_tcamLookupMuxTableLineCfgPtr = {
     "tcamLookupMuxTableLineCfgPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TCAM_LOOKUP_MUX_TABLE_LINE_CFG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TCAM_LOOKUP_PROFILE_CFG_STC_PTR_tcamLookupProfileCfgPtr = {
     "tcamLookupProfileCfgPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TCAM_LOOKUP_PROFILE_CFG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TCAM_RULE_SIZE_ENT_PTR_ruleSizePtr = {
     "ruleSizePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TCAM_RULE_SIZE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TCAM_RULE_SIZE_ENT_PTR_sizePtr = {
     "sizePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TCAM_RULE_SIZE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_isHitPtr = {
     "isHitPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_HEX_PTR_tcamMaskPtr = {
     "tcamMaskPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32_HEX)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_hitIndexPtr = {
     "hitIndexPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_numOfActiveFloorsPtr = {
     "numOfActiveFloorsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_subKeyProfilePtr = {
     "subKeyProfilePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tcamGroupPtr = {
     "tcamGroupPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tcamKeyArr = {
     "tcamKeyArr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tcamPatternPtr = {
     "tcamPatternPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamPortGroupClientGroupSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_CPSS_DXCH_TCAM_CLIENT_ENT_tcamClient,
    &DX_IN_GT_U32_tcamGroup,
    &DX_IN_GT_BOOL_enable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamPortGroupClientGroupGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_CPSS_DXCH_TCAM_CLIENT_ENT_tcamClient,
    &DX_OUT_GT_U32_PTR_tcamGroupPtr,
    &DX_OUT_GT_BOOL_PTR_enablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamPortGroupIndexRangeHitNumAndGroupSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_floorIndex,
    &DX_IN_CPSS_DXCH_TCAM_BLOCK_INFO_STC_PTR_floorInfoArr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamPortGroupIndexRangeHitNumAndGroupGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_floorIndex,
    &DX_OUT_CPSS_DXCH_TCAM_BLOCK_INFO_STC_PTR_floorInfoArr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamCpuLookupResultsGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_group,
    &DX_IN_GT_U32_hitNum,
    &DX_OUT_GT_BOOL_PTR_isValidPtr,
    &DX_OUT_GT_BOOL_PTR_isHitPtr,
    &DX_OUT_GT_U32_PTR_hitIndexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamPortGroupRuleValidStatusGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_index,
    &DX_OUT_GT_BOOL_PTR_validPtr,
    &DX_OUT_CPSS_DXCH_TCAM_RULE_SIZE_ENT_PTR_ruleSizePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamPortGroupActiveFloorsSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_numOfActiveFloors
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamPortGroupActiveFloorsGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_OUT_GT_U32_PTR_numOfActiveFloorsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamLookupMuxTableLineSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_entryIndex,
    &DX_IN_CPSS_DXCH_TCAM_LOOKUP_MUX_TABLE_LINE_CFG_STC_PTR_tcamLookupMuxTableLineCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamLookupMuxTableLineGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_entryIndex,
    &DX_OUT_CPSS_DXCH_TCAM_LOOKUP_MUX_TABLE_LINE_CFG_STC_PTR_tcamLookupMuxTableLineCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamIndexRangeHitNumAndGroupSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_floorIndex,
    &DX_IN_CPSS_DXCH_TCAM_BLOCK_INFO_STC_PTR_floorInfoArr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamIndexRangeHitNumAndGroupGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_floorIndex,
    &DX_OUT_CPSS_DXCH_TCAM_BLOCK_INFO_STC_PTR_floorInfoArr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamActiveFloorsSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_numOfActiveFloors
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamLookupProfileCfgSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_tcamProfileId,
    &DX_IN_CPSS_DXCH_TCAM_LOOKUP_PROFILE_CFG_STC_PTR_tcamLookupProfileCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamLookupProfileCfgGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_tcamProfileId,
    &DX_OUT_CPSS_DXCH_TCAM_LOOKUP_PROFILE_CFG_STC_PTR_tcamLookupProfileCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamActiveFloorsGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_numOfActiveFloorsPtr
};
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamPortGroupCpuLookupTriggerGet_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamPortGroupCpuLookupTriggerSet_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamPortGroupRuleRead_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTcamPortGroupRuleWrite_PARAMS[];


/********* lib API DB *********/

extern void cpssDxChTcamPortGroupRuleWrite_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChTcamPortGroupRuleRead_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChTcamIndexRangeHitNumAndGroupSet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChTcamIndexRangeHitNumAndGroupSet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChTcamPortGroupCpuLookupTriggerSet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChTcamPortGroupCpuLookupTriggerGet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChTcamLogLibDb[] = {
    {"cpssDxChTcamPortGroupRuleWrite", 7, cpssDxChTcamPortGroupRuleWrite_PARAMS, cpssDxChTcamPortGroupRuleWrite_preLogic},
    {"cpssDxChTcamPortGroupRuleRead", 7, cpssDxChTcamPortGroupRuleRead_PARAMS, cpssDxChTcamPortGroupRuleRead_preLogic},
    {"cpssDxChTcamPortGroupRuleValidStatusSet", 4, prvCpssLogGenDevNumPortGroupsBmpIndexValid_PARAMS, NULL},
    {"cpssDxChTcamPortGroupRuleValidStatusGet", 5, cpssDxChTcamPortGroupRuleValidStatusGet_PARAMS, NULL},
    {"cpssDxChTcamPortGroupClientGroupSet", 5, cpssDxChTcamPortGroupClientGroupSet_PARAMS, NULL},
    {"cpssDxChTcamPortGroupClientGroupGet", 5, cpssDxChTcamPortGroupClientGroupGet_PARAMS, NULL},
    {"cpssDxChTcamPortGroupIndexRangeHitNumAndGroupSet", 4, cpssDxChTcamPortGroupIndexRangeHitNumAndGroupSet_PARAMS, NULL},
    {"cpssDxChTcamIndexRangeHitNumAndGroupSet", 3, cpssDxChTcamIndexRangeHitNumAndGroupSet_PARAMS, cpssDxChTcamIndexRangeHitNumAndGroupSet_preLogic},
    {"cpssDxChTcamPortGroupIndexRangeHitNumAndGroupGet", 4, cpssDxChTcamPortGroupIndexRangeHitNumAndGroupGet_PARAMS, NULL},
    {"cpssDxChTcamIndexRangeHitNumAndGroupGet", 3, cpssDxChTcamIndexRangeHitNumAndGroupGet_PARAMS, cpssDxChTcamIndexRangeHitNumAndGroupSet_preLogic},
    {"cpssDxChTcamPortGroupActiveFloorsSet", 3, cpssDxChTcamPortGroupActiveFloorsSet_PARAMS, NULL},
    {"cpssDxChTcamActiveFloorsSet", 2, cpssDxChTcamActiveFloorsSet_PARAMS, NULL},
    {"cpssDxChTcamPortGroupActiveFloorsGet", 3, cpssDxChTcamPortGroupActiveFloorsGet_PARAMS, NULL},
    {"cpssDxChTcamActiveFloorsGet", 2, cpssDxChTcamActiveFloorsGet_PARAMS, NULL},
    {"cpssDxChTcamPortGroupCpuLookupTriggerSet", 6, cpssDxChTcamPortGroupCpuLookupTriggerSet_PARAMS, cpssDxChTcamPortGroupCpuLookupTriggerSet_preLogic},
    {"cpssDxChTcamPortGroupCpuLookupTriggerGet", 6, cpssDxChTcamPortGroupCpuLookupTriggerGet_PARAMS, cpssDxChTcamPortGroupCpuLookupTriggerGet_preLogic},
    {"cpssDxChTcamCpuLookupResultsGet", 7, cpssDxChTcamCpuLookupResultsGet_PARAMS, NULL},
    {"cpssDxChTcamLookupProfileCfgSet", 3, cpssDxChTcamLookupProfileCfgSet_PARAMS, NULL},
    {"cpssDxChTcamLookupProfileCfgGet", 3, cpssDxChTcamLookupProfileCfgGet_PARAMS, NULL},
    {"cpssDxChTcamLookupMuxTableLineSet", 3, cpssDxChTcamLookupMuxTableLineSet_PARAMS, NULL},
    {"cpssDxChTcamLookupMuxTableLineGet", 3, cpssDxChTcamLookupMuxTableLineGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_TCAM(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChTcamLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChTcamLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

