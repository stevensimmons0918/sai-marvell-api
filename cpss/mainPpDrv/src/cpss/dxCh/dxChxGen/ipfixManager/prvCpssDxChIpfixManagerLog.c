/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChIpfixManagerLog.c
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
#include <cpss/dxCh/dxChxGen/ipfixManager/cpssDxChIpfixManager.h>
#include <cpss/dxCh/dxChxGen/ipfixManager/private/prvCpssDxChIpfixManagerLog.h>
#include <cpss/dxCh/dxChxGen/policer/private/prvCpssDxChPolicerLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssGenDbLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_DXCH_IPFIX_MANAGER_ENTRIES_PER_FLOW_ENT[]  =
{
    "CPSS_DXCH_IPFIX_MANAGER_ENTRIES_PER_FLOW_IS_TWO_E",
    "CPSS_DXCH_IPFIX_MANAGER_ENTRIES_PER_FLOW_IS_ONE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IPFIX_MANAGER_ENTRIES_PER_FLOW_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_MANAGER_ATTRIBUTES_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IPFIX_MANAGER_ATTRIBUTES_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, serviceCpuNum);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_MANAGER_ENTRY_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IPFIX_MANAGER_ENTRY_PARAMS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, firstTs);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, firstTsValid);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_MANAGER_GLOBAL_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IPFIX_MANAGER_GLOBAL_CONFIG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, policerStage, CPSS_DXCH_POLICER_STAGE_TYPE_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, agingOffload);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, monitoringOffload);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, ipfixEntriesPerFlow, CPSS_DXCH_IPFIX_MANAGER_ENTRIES_PER_FLOW_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, idleTimeout);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, activeTimeout);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, deltaMode);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dataPktMtu);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, txDsaTag, 16, GT_U8);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ipfixDataQueueNum);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_MANAGER_PORT_GROUP_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IPFIX_MANAGER_PORT_GROUP_CONFIG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxIpfixIndex);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_MANAGER_ATTRIBUTES_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_IPFIX_MANAGER_ATTRIBUTES_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_MANAGER_ATTRIBUTES_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_MANAGER_ENTRY_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_IPFIX_MANAGER_ENTRY_PARAMS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_MANAGER_ENTRY_PARAMS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_MANAGER_EVENT_FUNC(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    char *formatPtr;
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_IPFIX_MANAGER_EVENT_FUNC, paramVal);
    PRV_CPSS_LOG_PTR_FORMAT_MAC(formatPtr);
    if (inOutParamInfoPtr->formatPtr)
    {
        formatPtr = inOutParamInfoPtr->formatPtr;
    }
    PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, formatPtr, namePtr, paramVal);
}
void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_MANAGER_GLOBAL_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_IPFIX_MANAGER_GLOBAL_CONFIG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_MANAGER_GLOBAL_CONFIG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_MANAGER_PORT_GROUP_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_IPFIX_MANAGER_PORT_GROUP_CONFIG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_MANAGER_PORT_GROUP_CONFIG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPFIX_MANAGER_ATTRIBUTES_STC_PTR_attributesPtr = {
     "attributesPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPFIX_MANAGER_ATTRIBUTES_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPFIX_MANAGER_ENTRY_PARAMS_STC_PTR_entryParamsPtr = {
     "entryParamsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPFIX_MANAGER_ENTRY_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPFIX_MANAGER_EVENT_FUNC_notifyFunc = {
     "notifyFunc", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_IPFIX_MANAGER_EVENT_FUNC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPFIX_MANAGER_GLOBAL_CONFIG_STC_PTR_globalCfgPtr = {
     "globalCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPFIX_MANAGER_GLOBAL_CONFIG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPFIX_MANAGER_PORT_GROUP_CONFIG_STC_PTR_portGroupCfgPtr = {
     "portGroupCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPFIX_MANAGER_PORT_GROUP_CONFIG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_ipfixEnable = {
     "ipfixEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_numDeletedPtr = {
     "numDeletedPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_numOfMsgFetchedPtr = {
     "numOfMsgFetchedPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixManagerCreate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_IPFIX_MANAGER_ATTRIBUTES_STC_PTR_attributesPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixManagerEventNotifyBind_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_IPFIX_MANAGER_EVENT_FUNC_notifyFunc
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixManagerGlobalConfigSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_IPFIX_MANAGER_GLOBAL_CONFIG_STC_PTR_globalCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixManagerEnableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_BOOL_ipfixEnable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixManagerEntryAdd_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_CPSS_DXCH_IPFIX_MANAGER_ENTRY_PARAMS_STC_PTR_entryParamsPtr,
    &DX_OUT_GT_U32_PTR_flowIdPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixManagerPortGroupConfigSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_CPSS_DXCH_IPFIX_MANAGER_PORT_GROUP_CONFIG_STC_PTR_portGroupCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixManagerEntryDelete_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_flowId
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixManagerEntryDeleteAll_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_numDeletedPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixManagerIpcMsgFetch_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_numOfMsgFetchedPtr
};


/********* lib API DB *********/

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChIpfixManagerLogLibDb[] = {
    {"cpssDxChIpfixManagerCreate", 2, cpssDxChIpfixManagerCreate_PARAMS, NULL},
    {"cpssDxChIpfixManagerEnableSet", 2, cpssDxChIpfixManagerEnableSet_PARAMS, NULL},
    {"cpssDxChIpfixManagerDelete", 1, prvCpssLogGenDevNum_PARAMS, NULL},
    {"cpssDxChIpfixManagerGlobalConfigSet", 2, cpssDxChIpfixManagerGlobalConfigSet_PARAMS, NULL},
    {"cpssDxChIpfixManagerPortGroupConfigSet", 3, cpssDxChIpfixManagerPortGroupConfigSet_PARAMS, NULL},
    {"cpssDxChIpfixManagerConfigGet", 1, prvCpssLogGenDevNum_PARAMS, NULL},
    {"cpssDxChIpfixManagerEntryAdd", 4, cpssDxChIpfixManagerEntryAdd_PARAMS, NULL},
    {"cpssDxChIpfixManagerEntryDelete", 3, cpssDxChIpfixManagerEntryDelete_PARAMS, NULL},
    {"cpssDxChIpfixManagerEntryDeleteAll", 2, cpssDxChIpfixManagerEntryDeleteAll_PARAMS, NULL},
    {"cpssDxChIpfixManagerIpfixDataGet", 3, cpssDxChIpfixManagerEntryDelete_PARAMS, NULL},
    {"cpssDxChIpfixManagerIpfixDataGetAll", 1, prvCpssLogGenDevNum_PARAMS, NULL},
    {"cpssDxChIpfixManagerIpcMsgFetch", 2, cpssDxChIpfixManagerIpcMsgFetch_PARAMS, NULL},
    {"cpssDxChIpfixManagerIpcMsgFetchAll", 2, cpssDxChIpfixManagerIpcMsgFetch_PARAMS, NULL},
    {"cpssDxChIpfixManagerEventNotifyBind", 2, cpssDxChIpfixManagerEventNotifyBind_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_IPFIX_MANAGER(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChIpfixManagerLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChIpfixManagerLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

