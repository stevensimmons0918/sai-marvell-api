/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChVirtualTcamLog.c
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
#include <cpss/dxCh/dxChxGen/pcl/private/prvCpssDxChPclLog.h>
#include <cpss/dxCh/dxChxGen/tti/private/prvCpssDxChTtiLog.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcam.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/private/prvCpssDxChVirtualTcamLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssGenDbLog.h>
#include <cpss/generic/pcl/private/prvCpssGenPclLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_DXCH_VIRTUAL_TCAM_AVAILABILTY_ENT[]  =
{
    "CPSS_DXCH_VIRTUAL_TCAM_AVAILABILTY_AVAILABLE_E",
    "CPSS_DXCH_VIRTUAL_TCAM_AVAILABILTY_NOT_AVAILABLE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_VIRTUAL_TCAM_AVAILABILTY_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_ENT[]  =
{
    "CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E",
    "CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_ENT[]  =
{
    "CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E",
    "CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_ENT);
const PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT[]  =
{PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_20_B_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_40_B_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_50_B_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_STD_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_EXT_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ULTRA_E)
};
PRV_CPSS_LOG_STC_ENUM_MAP_ARRAY_SIZE_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_ENT[]  =
{
    "CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E",
    "CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_VIRTUAL_TCAM_CHECK_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_VIRTUAL_TCAM_CHECK_INFO_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, isExist);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vTcamId);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, vTcamInfo, CPSS_DXCH_VIRTUAL_TCAM_INFO_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_REPLAY_RULE_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_VIRTUAL_TCAM_HA_DB_REPLAY_RULE_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vTcamMngId);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vTcamId);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ruleId);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ruleAttributes, CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ruleType, CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ruleData, CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, pattern, CPSS_DXCH_VIRTUAL_TCAM_HA_REPLAY_DB_RULE_UNT);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, mask, CPSS_DXCH_VIRTUAL_TCAM_HA_REPLAY_DB_RULE_UNT);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, action, CPSS_DXCH_VIRTUAL_TCAM_HA_REPLAY_DB_ACTION_UNT);
    PRV_CPSS_LOG_STC_STC_PTR_MAC(valPtr, next, CPSS_DXCH_VIRTUAL_TCAM_HA_DB_REPLAY_RULE_ENTRY_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_VIRTUAL_TCAM_HA_UNMAPPED_RULE_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_VIRTUAL_TCAM_HA_UNMAPPED_RULE_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, physicalIndex);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, action, CPSS_DXCH_PCL_ACTION_SIZE_IN_WORDS_CNS, GT_U32);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, mask, CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS, GT_U32_HEX);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, pattern, CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS, GT_U32);
    PRV_CPSS_LOG_STC_STC_PTR_MAC(valPtr, next, CPSS_DXCH_VIRTUAL_TCAM_HA_UNMAPPED_RULE_ENTRY_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_VIRTUAL_TCAM_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_VIRTUAL_TCAM_INFO_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, clientGroup);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, hitNumber);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, ruleSize, CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, autoResize);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, guaranteedNumOfRules);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, ruleAdditionMethod, CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, tcamSpaceUnmovable);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_VIRTUAL_TCAM_MNG_CONFIG_PARAM_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_VIRTUAL_TCAM_MNG_CONFIG_PARAM_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, haSupportEnabled);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, haFeaturesEnabledBmp);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_VIRTUAL_TCAM_PORT_GROUP_BITMAP_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_VIRTUAL_TCAM_PORT_GROUP_BITMAP_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, devNum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, portGroupsBmp);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, priority);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_VIRTUAL_TCAM_SPACE_SEGMENT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_VIRTUAL_TCAM_SPACE_SEGMENT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, startRowIndex);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numOfRows);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, columnsBitmap);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, rulesUsed);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, rulesFree);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_VIRTUAL_TCAM_HA_REPLAY_DB_ACTION_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_VIRTUAL_TCAM_HA_REPLAY_DB_ACTION_UNT *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, pcl, CPSS_DXCH_PCL_ACTION_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, tti, CPSS_DXCH_TTI_ACTION_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_VIRTUAL_TCAM_HA_REPLAY_DB_RULE_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_VIRTUAL_TCAM_HA_REPLAY_DB_RULE_UNT *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, pcl, CPSS_DXCH_PCL_RULE_FORMAT_UNT);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, tti, CPSS_DXCH_TTI_RULE_UNT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_UNT *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, pcl, CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_PCL_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, tti, CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_TTI_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_UNT *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, pcl, CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_PCL_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_UNT *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, pcl, CPSS_DXCH_VIRTUAL_TCAM_PCL_RULE_DATA_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, tti, CPSS_DXCH_VIRTUAL_TCAM_TTI_RULE_DATA_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_UNT *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, pcl, CPSS_DXCH_VIRTUAL_TCAM_PCL_RULE_TYPE_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, tti, CPSS_DXCH_VIRTUAL_TCAM_TTI_RULE_TYPE_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_VIRTUAL_TCAM_PCL_RULE_DATA_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_VIRTUAL_TCAM_PCL_RULE_DATA_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_PTR_MAC(valPtr, maskPtr, CPSS_DXCH_PCL_RULE_FORMAT_UNT);
    PRV_CPSS_LOG_STC_STC_PTR_MAC(valPtr, patternPtr, CPSS_DXCH_PCL_RULE_FORMAT_UNT);
    PRV_CPSS_LOG_STC_STC_PTR_MAC(valPtr, actionPtr, CPSS_DXCH_PCL_ACTION_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_VIRTUAL_TCAM_PCL_RULE_TYPE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_VIRTUAL_TCAM_PCL_RULE_TYPE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, ruleFormat, CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_PCL_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_PCL_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_PTR_MAC(valPtr, actionPtr, CPSS_DXCH_PCL_ACTION_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_TTI_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_TTI_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_PTR_MAC(valPtr, actionPtr, CPSS_DXCH_TTI_ACTION_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_PCL_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_PCL_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, direction, CPSS_PCL_DIRECTION_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_VIRTUAL_TCAM_TTI_RULE_DATA_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_VIRTUAL_TCAM_TTI_RULE_DATA_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_PTR_MAC(valPtr, maskPtr, CPSS_DXCH_TTI_RULE_UNT);
    PRV_CPSS_LOG_STC_STC_PTR_MAC(valPtr, patternPtr, CPSS_DXCH_TTI_RULE_UNT);
    PRV_CPSS_LOG_STC_STC_PTR_MAC(valPtr, actionPtr, CPSS_DXCH_TTI_ACTION_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_VIRTUAL_TCAM_TTI_RULE_TYPE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_VIRTUAL_TCAM_TTI_RULE_TYPE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, ruleFormat, CPSS_DXCH_TTI_RULE_TYPE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_VIRTUAL_TCAM_AVAILABILTY_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_VIRTUAL_TCAM_AVAILABILTY_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_VIRTUAL_TCAM_AVAILABILTY_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_REPLAY_RULE_ENTRY_STC_PTR_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_VIRTUAL_TCAM_HA_DB_REPLAY_RULE_ENTRY_STC**, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_REPLAY_RULE_ENTRY_STC_PTR(contextLib, logType, namePtr, *paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_VIRTUAL_TCAM_HA_HW_DB_READ_FUNC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    char *formatPtr;
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_VIRTUAL_TCAM_HA_HW_DB_READ_FUNC*, paramVal);
    PRV_CPSS_LOG_PTR_FORMAT_MAC(formatPtr);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    if (inOutParamInfoPtr->formatPtr)
    {
        formatPtr = inOutParamInfoPtr->formatPtr;
    }
    PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, formatPtr, namePtr, *paramVal);
}
void prvCpssLogParamFunc_CPSS_DXCH_VIRTUAL_TCAM_HA_HW_DB_WRITE_FUNC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    char *formatPtr;
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_VIRTUAL_TCAM_HA_HW_DB_WRITE_FUNC*, paramVal);
    PRV_CPSS_LOG_PTR_FORMAT_MAC(formatPtr);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    if (inOutParamInfoPtr->formatPtr)
    {
        formatPtr = inOutParamInfoPtr->formatPtr;
    }
    PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, formatPtr, namePtr, *paramVal);
}
void prvCpssLogParamFunc_CPSS_DXCH_VIRTUAL_TCAM_HA_UNMAPPED_RULE_ENTRY_STC_PTR_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_VIRTUAL_TCAM_HA_UNMAPPED_RULE_ENTRY_STC**, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_VIRTUAL_TCAM_HA_UNMAPPED_RULE_ENTRY_STC_PTR(contextLib, logType, namePtr, *paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_VIRTUAL_TCAM_INFO_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_VIRTUAL_TCAM_INFO_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_VIRTUAL_TCAM_INFO_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_VIRTUAL_TCAM_MNG_CONFIG_PARAM_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_VIRTUAL_TCAM_MNG_CONFIG_PARAM_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_VIRTUAL_TCAM_MNG_CONFIG_PARAM_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_VIRTUAL_TCAM_PORT_GROUP_BITMAP_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_VIRTUAL_TCAM_PORT_GROUP_BITMAP_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_VIRTUAL_TCAM_PORT_GROUP_BITMAP_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_VIRTUAL_TCAM_RULE_ID(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    char * formatPtr = "%s = %d\n";
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_ID, paramVal);
    if (inOutParamInfoPtr->formatPtr)
    {
        formatPtr = inOutParamInfoPtr->formatPtr;
    }
    PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, formatPtr, namePtr, paramVal);
}
void prvCpssLogParamFunc_CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_VIRTUAL_TCAM_SPACE_SEGMENT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_VIRTUAL_TCAM_SPACE_SEGMENT_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_VIRTUAL_TCAM_SPACE_SEGMENT_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_numOfPortGroupsPtr = {
     "numOfPortGroupsPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_VIRTUAL_TCAM_CHECK_INFO_STC_PTR_vTcamCheckInfoArr = {
     "vTcamCheckInfoArr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_VIRTUAL_TCAM_CHECK_INFO_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_ENT_position = {
     "position", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_VIRTUAL_TCAM_HA_HW_DB_READ_FUNC_PTR_hwDbReadFuncPtr = {
     "hwDbReadFuncPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_VIRTUAL_TCAM_HA_HW_DB_READ_FUNC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_VIRTUAL_TCAM_HA_HW_DB_WRITE_FUNC_PTR_hwDbWriteFuncPtr = {
     "hwDbWriteFuncPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_VIRTUAL_TCAM_HA_HW_DB_WRITE_FUNC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_VIRTUAL_TCAM_INFO_STC_PTR_vTcamInfoPtr = {
     "vTcamInfoPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_VIRTUAL_TCAM_INFO_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_VIRTUAL_TCAM_MNG_CONFIG_PARAM_STC_PTR_vTcamMngCfgPtr = {
     "vTcamMngCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_VIRTUAL_TCAM_MNG_CONFIG_PARAM_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_VIRTUAL_TCAM_PORT_GROUP_BITMAP_STC_PTR_portGroupBmpListArr = {
     "portGroupBmpListArr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_VIRTUAL_TCAM_PORT_GROUP_BITMAP_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_STC_PTR_actionDataPtr = {
     "actionDataPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_STC_PTR_actionTypePtr = {
     "actionTypePtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC_PTR_ruleAttributesPtr = {
     "ruleAttributesPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC_PTR_ruleDataPtr = {
     "ruleDataPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_VIRTUAL_TCAM_RULE_ID_dstRuleId = {
     "dstRuleId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_ID)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_VIRTUAL_TCAM_RULE_ID_ruleId = {
     "ruleId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_ID)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_VIRTUAL_TCAM_RULE_ID_srcRuleId = {
     "srcRuleId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_ID)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC_PTR_ruleTypePtr = {
     "ruleTypePtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_invalidateRulesInRemoved = {
     "invalidateRulesInRemoved", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_toInsert = {
     "toInsert", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_granularity = {
     "granularity", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_maxNumOfSegments = {
     "maxNumOfSegments", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numOfPortGroupBmps = {
     "numOfPortGroupBmps", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_rulePlace = {
     "rulePlace", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_sizeInRules = {
     "sizeInRules", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_vTcamAmount = {
     "vTcamAmount", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_vTcamId = {
     "vTcamId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_vTcamMngId = {
     "vTcamMngId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_VIRTUAL_TCAM_AVAILABILTY_ENT_PTR_tcamAvailabilityPtr = {
     "tcamAvailabilityPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_VIRTUAL_TCAM_AVAILABILTY_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_REPLAY_RULE_ENTRY_STC_PTR_PTR_gtNokRulesListPtr = {
     "gtNokRulesListPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_PTR_MAC(CPSS_DXCH_VIRTUAL_TCAM_HA_DB_REPLAY_RULE_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_VIRTUAL_TCAM_HA_UNMAPPED_RULE_ENTRY_STC_PTR_PTR_unmappedRulesListPtr = {
     "unmappedRulesListPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_PTR_MAC(CPSS_DXCH_VIRTUAL_TCAM_HA_UNMAPPED_RULE_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_VIRTUAL_TCAM_INFO_STC_PTR_vTcamInfoPtr = {
     "vTcamInfoPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_VIRTUAL_TCAM_INFO_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_VIRTUAL_TCAM_PORT_GROUP_BITMAP_STC_PTR_portGroupBmpListArr = {
     "portGroupBmpListArr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_VIRTUAL_TCAM_PORT_GROUP_BITMAP_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_STC_PTR_actionDataPtr = {
     "actionDataPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC_PTR_ruleDataPtr = {
     "ruleDataPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_VIRTUAL_TCAM_SPACE_SEGMENT_STC_PTR_segmentArrayPtr = {
     "segmentArrayPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_VIRTUAL_TCAM_SPACE_SEGMENT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC_PTR_vTcamUsagePtr = {
     "vTcamUsagePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_hwIndexPtr = {
     "hwIndexPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_logicalIndexPtr = {
     "logicalIndexPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_nextRuleIdPtr = {
     "nextRuleIdPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_numOfSegmentsPtr = {
     "numOfSegmentsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_rulePriorityPtr = {
     "rulePriorityPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChVirtualTcamManagerDelete_PARAMS[] =  {
    &DX_IN_GT_U32_vTcamMngId
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChVirtualTcamHaHwDbAccessBind_PARAMS[] =  {
    &DX_IN_GT_U32_vTcamMngId,
    &DX_IN_CPSS_DXCH_VIRTUAL_TCAM_HA_HW_DB_WRITE_FUNC_PTR_hwDbWriteFuncPtr,
    &DX_IN_CPSS_DXCH_VIRTUAL_TCAM_HA_HW_DB_READ_FUNC_PTR_hwDbReadFuncPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChVirtualTcamManagerCreate_PARAMS[] =  {
    &DX_IN_GT_U32_vTcamMngId,
    &DX_IN_CPSS_DXCH_VIRTUAL_TCAM_MNG_CONFIG_PARAM_STC_PTR_vTcamMngCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChVirtualTcamManagerPortGroupListAdd_PARAMS[] =  {
    &DX_IN_GT_U32_vTcamMngId,
    &DX_IN_CPSS_DXCH_VIRTUAL_TCAM_PORT_GROUP_BITMAP_STC_PTR_portGroupBmpListArr,
    &DX_IN_GT_U32_numOfPortGroupBmps
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChVirtualTcamManagerPortGroupListRemove_PARAMS[] =  {
    &DX_IN_GT_U32_vTcamMngId,
    &DX_IN_CPSS_DXCH_VIRTUAL_TCAM_PORT_GROUP_BITMAP_STC_PTR_portGroupBmpListArr,
    &DX_IN_GT_U32_numOfPortGroupBmps,
    &DX_IN_GT_BOOL_invalidateRulesInRemoved
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChVirtualTcamAutoResizeGranularitySet_PARAMS[] =  {
    &DX_IN_GT_U32_vTcamMngId,
    &DX_IN_GT_U32_granularity
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChVirtualTcamMemoryAvailabilityCheck_PARAMS[] =  {
    &DX_IN_GT_U32_vTcamMngId,
    &DX_IN_GT_U32_vTcamAmount,
    &DX_IN_CPSS_DXCH_VIRTUAL_TCAM_CHECK_INFO_STC_PTR_vTcamCheckInfoArr,
    &DX_OUT_CPSS_DXCH_VIRTUAL_TCAM_AVAILABILTY_ENT_PTR_tcamAvailabilityPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChVirtualTcamRemove_PARAMS[] =  {
    &DX_IN_GT_U32_vTcamMngId,
    &DX_IN_GT_U32_vTcamId
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChVirtualTcamCreate_PARAMS[] =  {
    &DX_IN_GT_U32_vTcamMngId,
    &DX_IN_GT_U32_vTcamId,
    &DX_IN_CPSS_DXCH_VIRTUAL_TCAM_INFO_STC_PTR_vTcamInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChVirtualTcamRuleDelete_PARAMS[] =  {
    &DX_IN_GT_U32_vTcamMngId,
    &DX_IN_GT_U32_vTcamId,
    &DX_IN_CPSS_DXCH_VIRTUAL_TCAM_RULE_ID_ruleId
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChVirtualTcamRuleActionUpdate_PARAMS[] =  {
    &DX_IN_GT_U32_vTcamMngId,
    &DX_IN_GT_U32_vTcamId,
    &DX_IN_CPSS_DXCH_VIRTUAL_TCAM_RULE_ID_ruleId,
    &DX_IN_CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_STC_PTR_actionTypePtr,
    &DX_IN_CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_STC_PTR_actionDataPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChVirtualTcamRuleActionGet_PARAMS[] =  {
    &DX_IN_GT_U32_vTcamMngId,
    &DX_IN_GT_U32_vTcamId,
    &DX_IN_CPSS_DXCH_VIRTUAL_TCAM_RULE_ID_ruleId,
    &DX_IN_CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_STC_PTR_actionTypePtr,
    &DX_OUT_CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_STC_PTR_actionDataPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChVirtualTcamRuleWrite_PARAMS[] =  {
    &DX_IN_GT_U32_vTcamMngId,
    &DX_IN_GT_U32_vTcamId,
    &DX_IN_CPSS_DXCH_VIRTUAL_TCAM_RULE_ID_ruleId,
    &DX_IN_CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC_PTR_ruleAttributesPtr,
    &DX_IN_CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC_PTR_ruleTypePtr,
    &DX_IN_CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC_PTR_ruleDataPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChVirtualTcamRuleRead_PARAMS[] =  {
    &DX_IN_GT_U32_vTcamMngId,
    &DX_IN_GT_U32_vTcamId,
    &DX_IN_CPSS_DXCH_VIRTUAL_TCAM_RULE_ID_ruleId,
    &DX_IN_CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC_PTR_ruleTypePtr,
    &DX_OUT_CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC_PTR_ruleDataPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChVirtualTcamRuleValidStatusSet_PARAMS[] =  {
    &DX_IN_GT_U32_vTcamMngId,
    &DX_IN_GT_U32_vTcamId,
    &DX_IN_CPSS_DXCH_VIRTUAL_TCAM_RULE_ID_ruleId,
    &DX_IN_GT_BOOL_valid
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChVirtualTcamRulePriorityUpdate_PARAMS[] =  {
    &DX_IN_GT_U32_vTcamMngId,
    &DX_IN_GT_U32_vTcamId,
    &DX_IN_CPSS_DXCH_VIRTUAL_TCAM_RULE_ID_ruleId,
    &DX_IN_GT_U32_priority,
    &DX_IN_CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_ENT_position
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChVirtualTcamRuleValidStatusGet_PARAMS[] =  {
    &DX_IN_GT_U32_vTcamMngId,
    &DX_IN_GT_U32_vTcamId,
    &DX_IN_CPSS_DXCH_VIRTUAL_TCAM_RULE_ID_ruleId,
    &DX_OUT_GT_BOOL_PTR_validPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChVirtualTcamRulePriorityGet_PARAMS[] =  {
    &DX_IN_GT_U32_vTcamMngId,
    &DX_IN_GT_U32_vTcamId,
    &DX_IN_CPSS_DXCH_VIRTUAL_TCAM_RULE_ID_ruleId,
    &DX_OUT_GT_U32_PTR_rulePriorityPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChVirtualTcamRuleMove_PARAMS[] =  {
    &DX_IN_GT_U32_vTcamMngId,
    &DX_IN_GT_U32_vTcamId,
    &DX_IN_CPSS_DXCH_VIRTUAL_TCAM_RULE_ID_srcRuleId,
    &DX_IN_CPSS_DXCH_VIRTUAL_TCAM_RULE_ID_dstRuleId
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChVirtualTcamAutoResizeEnable_PARAMS[] =  {
    &DX_IN_GT_U32_vTcamMngId,
    &DX_IN_GT_U32_vTcamId,
    &DX_IN_GT_BOOL_enable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChVirtualTcamSpaceLayoutGet_PARAMS[] =  {
    &DX_IN_GT_U32_vTcamMngId,
    &DX_IN_GT_U32_vTcamId,
    &DX_IN_GT_U32_maxNumOfSegments,
    &DX_OUT_GT_U32_PTR_numOfSegmentsPtr,
    &DX_OUT_CPSS_DXCH_VIRTUAL_TCAM_SPACE_SEGMENT_STC_PTR_segmentArrayPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChVirtualTcamDbRuleIdToHwIndexConvert_PARAMS[] =  {
    &DX_IN_GT_U32_vTcamMngId,
    &DX_IN_GT_U32_vTcamId,
    &DX_IN_GT_U32_ruleId,
    &DX_OUT_GT_U32_PTR_logicalIndexPtr,
    &DX_OUT_GT_U32_PTR_hwIndexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChVirtualTcamNextRuleIdGet_PARAMS[] =  {
    &DX_IN_GT_U32_vTcamMngId,
    &DX_IN_GT_U32_vTcamId,
    &DX_IN_GT_U32_ruleId,
    &DX_OUT_GT_U32_PTR_nextRuleIdPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChVirtualTcamResize_PARAMS[] =  {
    &DX_IN_GT_U32_vTcamMngId,
    &DX_IN_GT_U32_vTcamId,
    &DX_IN_GT_U32_rulePlace,
    &DX_IN_GT_BOOL_toInsert,
    &DX_IN_GT_U32_sizeInRules
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChVirtualTcamInfoGet_PARAMS[] =  {
    &DX_IN_GT_U32_vTcamMngId,
    &DX_IN_GT_U32_vTcamId,
    &DX_OUT_CPSS_DXCH_VIRTUAL_TCAM_INFO_STC_PTR_vTcamInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChVirtualTcamUsageGet_PARAMS[] =  {
    &DX_IN_GT_U32_vTcamMngId,
    &DX_IN_GT_U32_vTcamId,
    &DX_OUT_CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC_PTR_vTcamUsagePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChVirtualTcamManagerDevListRemove_PARAMS[] =  {
    &DX_IN_GT_U32_vTcamMngId,
    &DX_IN_GT_U8_PTR_devListArr,
    &DX_IN_GT_U32_numOfDevs
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChVirtualTcamManagerPortGroupListGet_PARAMS[] =  {
    &DX_IN_GT_U32_vTcamMngId,
    &DX_INOUT_GT_U32_PTR_numOfPortGroupsPtr,
    &DX_OUT_CPSS_DXCH_VIRTUAL_TCAM_PORT_GROUP_BITMAP_STC_PTR_portGroupBmpListArr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChVirtualTcamManagerHaConfigReplayRuleListsGet_PARAMS[] =  {
    &DX_OUT_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_REPLAY_RULE_ENTRY_STC_PTR_PTR_gtNokRulesListPtr,
    &DX_OUT_CPSS_DXCH_VIRTUAL_TCAM_HA_UNMAPPED_RULE_ENTRY_STC_PTR_PTR_unmappedRulesListPtr
};
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChVirtualTcamManagerDevListAdd_PARAMS[];


/********* lib API DB *********/

extern void cpssDxChVirtualTcamManagerDevListAdd_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChVirtualTcamManagerDevListAdd_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChVirtualTcamRuleWrite_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChVirtualTcamRuleRead_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChVirtualTcamRuleActionUpdate_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChVirtualTcamRuleActionUpdate_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChVirtualTcamMemoryAvailabilityCheck_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChVirtualTcamLogLibDb[] = {
    {"cpssDxChVirtualTcamHaHwDbAccessBind", 3, cpssDxChVirtualTcamHaHwDbAccessBind_PARAMS, NULL},
    {"cpssDxChVirtualTcamManagerDelete", 1, cpssDxChVirtualTcamManagerDelete_PARAMS, NULL},
    {"cpssDxChVirtualTcamManagerCreate", 2, cpssDxChVirtualTcamManagerCreate_PARAMS, NULL},
    {"cpssDxChVirtualTcamManagerDevListAdd", 3, cpssDxChVirtualTcamManagerDevListAdd_PARAMS, cpssDxChVirtualTcamManagerDevListAdd_preLogic},
    {"cpssDxChVirtualTcamManagerDevListRemove", 3, cpssDxChVirtualTcamManagerDevListRemove_PARAMS, cpssDxChVirtualTcamManagerDevListAdd_preLogic},
    {"cpssDxChVirtualTcamManagerPortGroupListAdd", 3, cpssDxChVirtualTcamManagerPortGroupListAdd_PARAMS, NULL},
    {"cpssDxChVirtualTcamManagerPortGroupListRemove", 4, cpssDxChVirtualTcamManagerPortGroupListRemove_PARAMS, NULL},
    {"cpssDxChVirtualTcamManagerPortGroupListGet", 3, cpssDxChVirtualTcamManagerPortGroupListGet_PARAMS, NULL},
    {"cpssDxChVirtualTcamRuleWrite", 6, cpssDxChVirtualTcamRuleWrite_PARAMS, cpssDxChVirtualTcamRuleWrite_preLogic},
    {"cpssDxChVirtualTcamRuleRead", 5, cpssDxChVirtualTcamRuleRead_PARAMS, cpssDxChVirtualTcamRuleRead_preLogic},
    {"cpssDxChVirtualTcamRuleActionUpdate", 5, cpssDxChVirtualTcamRuleActionUpdate_PARAMS, cpssDxChVirtualTcamRuleActionUpdate_preLogic},
    {"cpssDxChVirtualTcamRuleActionGet", 5, cpssDxChVirtualTcamRuleActionGet_PARAMS, cpssDxChVirtualTcamRuleActionUpdate_preLogic},
    {"cpssDxChVirtualTcamRuleValidStatusSet", 4, cpssDxChVirtualTcamRuleValidStatusSet_PARAMS, NULL},
    {"cpssDxChVirtualTcamRuleValidStatusGet", 4, cpssDxChVirtualTcamRuleValidStatusGet_PARAMS, NULL},
    {"cpssDxChVirtualTcamRuleDelete", 3, cpssDxChVirtualTcamRuleDelete_PARAMS, NULL},
    {"cpssDxChVirtualTcamRuleMove", 4, cpssDxChVirtualTcamRuleMove_PARAMS, NULL},
    {"cpssDxChVirtualTcamRulePriorityGet", 4, cpssDxChVirtualTcamRulePriorityGet_PARAMS, NULL},
    {"cpssDxChVirtualTcamRulePriorityUpdate", 5, cpssDxChVirtualTcamRulePriorityUpdate_PARAMS, NULL},
    {"cpssDxChVirtualTcamDbRuleIdToHwIndexConvert", 5, cpssDxChVirtualTcamDbRuleIdToHwIndexConvert_PARAMS, NULL},
    {"cpssDxChVirtualTcamUsageGet", 3, cpssDxChVirtualTcamUsageGet_PARAMS, NULL},
    {"cpssDxChVirtualTcamInfoGet", 3, cpssDxChVirtualTcamInfoGet_PARAMS, NULL},
    {"cpssDxChVirtualTcamCreate", 3, cpssDxChVirtualTcamCreate_PARAMS, NULL},
    {"cpssDxChVirtualTcamRemove", 2, cpssDxChVirtualTcamRemove_PARAMS, NULL},
    {"cpssDxChVirtualTcamResize", 5, cpssDxChVirtualTcamResize_PARAMS, NULL},
    {"cpssDxChVirtualTcamNextRuleIdGet", 4, cpssDxChVirtualTcamNextRuleIdGet_PARAMS, NULL},
    {"cpssDxChVirtualTcamAutoResizeEnable", 3, cpssDxChVirtualTcamAutoResizeEnable_PARAMS, NULL},
    {"cpssDxChVirtualTcamAutoResizeGranularitySet", 2, cpssDxChVirtualTcamAutoResizeGranularitySet_PARAMS, NULL},
    {"cpssDxChVirtualTcamMemoryAvailabilityCheck", 4, cpssDxChVirtualTcamMemoryAvailabilityCheck_PARAMS, cpssDxChVirtualTcamMemoryAvailabilityCheck_preLogic},
    {"cpssDxChVirtualTcamSpaceLayoutGet", 5, cpssDxChVirtualTcamSpaceLayoutGet_PARAMS, NULL},
    {"cpssDxChVirtualTcamManagerHaConfigReplayRuleListsGet", 2, cpssDxChVirtualTcamManagerHaConfigReplayRuleListsGet_PARAMS, NULL},
    {"cpssDxChVirtualTcamManagerHaConfigReplayRuleListsFree", 0, NULL, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_VIRTUAL_TCAM(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChVirtualTcamLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChVirtualTcamLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

