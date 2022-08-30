/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssPxConfigLog.c
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
#include <cpss/common/config/private/prvCpssCommonConfigLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssPxGenDbLog.h>
#include <cpss/px/config/cpssPxCfgInit.h>
#include <cpss/px/config/private/prvCpssPxConfigLog.h>
#include <cpss/px/cpssHwInit/private/prvCpssPxCpssHwInitLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_PX_CFG_CNTR_ENT[]  =
{
    "CPSS_PX_CFG_CNTR_RECEIVED_PFC_E",
    "CPSS_PX_CFG_CNTR_RECEIVED_MC_E",
    "CPSS_PX_CFG_CNTR_RECEIVED_UC_E",
    "CPSS_PX_CFG_CNTR_RECEIVED_QCN_E",
    "CPSS_PX_CFG_CNTR_RECEIVED_MC_QCN_E",
    "CPSS_PX_CFG_CNTR_INGRESS_DROP_E",
    "CPSS_PX_CFG_CNTR_OUT_MC_PACKETS_E",
    "CPSS_PX_CFG_CNTR_OUT_QCN_TO_PFC_MSG_E",
    "CPSS_PX_CFG_CNTR_OUT_QCN_PACKETS_E",
    "CPSS_PX_CFG_CNTR_OUT_PFC_PACKETS_E",
    "CPSS_PX_CFG_CNTR_OUT_UC_PACKETS_E",
    "CPSS_PX_CFG_CNTR_OUT_MC_QCN_PACKETS_E",
    "CPSS_PX_CFG_CNTR_LAST_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_CFG_CNTR_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_PX_CFG_DEV_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_CFG_DEV_INFO_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, genDevInfo, CPSS_GEN_CFG_DEV_INFO_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_CFG_HW_ACCESS_OBJ_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_CFG_HW_ACCESS_OBJ_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, hwAccessTableEntryReadFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, hwAccessTableEntryWriteFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, hwAccessTableEntryFieldReadFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, hwAccessTableEntryFieldWriteFunc);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_PX_CFG_CNTR_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_CFG_CNTR_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_CFG_CNTR_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_CFG_DEV_INFO_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_CFG_DEV_INFO_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_CFG_DEV_INFO_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_CFG_HW_ACCESS_OBJ_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_CFG_HW_ACCESS_OBJ_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_CFG_HW_ACCESS_OBJ_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_CFG_CNTR_ENT_counterType = {
     "counterType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_CFG_CNTR_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_CFG_HW_ACCESS_OBJ_STC_PTR_cfgAccessObjPtr = {
     "cfgAccessObjPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_CFG_HW_ACCESS_OBJ_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_TABLE_ENT_table = {
     "table", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_TABLE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_bind = {
     "bind", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_SW_DEV_NUM_newDevNum = {
     "newDevNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_SW_DEV_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_SW_DEV_NUM_oldDevNum = {
     "oldDevNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_SW_DEV_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_CFG_DEV_INFO_STC_PTR_devInfoPtr = {
     "devInfoPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_CFG_DEV_INFO_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_SW_DEV_NUM_PTR_nextDevNumPtr = {
     "nextDevNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_SW_DEV_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_numEntriesPtr = {
     "numEntriesPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_timeStampValuePtr = {
     "timeStampValuePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCfgHwAccessObjectBind_PARAMS[] =  {
    &PX_IN_CPSS_PX_CFG_HW_ACCESS_OBJ_STC_PTR_cfgAccessObjPtr,
    &PX_IN_GT_BOOL_bind
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCfgCntrGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_CFG_CNTR_ENT_counterType,
    &PX_OUT_GT_U32_PTR_cntPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCfgTableNumEntriesGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_TABLE_ENT_table,
    &PX_OUT_GT_U32_PTR_numEntriesPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCfgDevInfoGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_CPSS_PX_CFG_DEV_INFO_STC_PTR_devInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCfgNextDevGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_GT_SW_DEV_NUM_PTR_nextDevNumPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCfgLastRegAccessTimeStampGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_GT_U32_PTR_timeStampValuePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCfgReNumberDevNum_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_oldDevNum,
    &PX_IN_GT_SW_DEV_NUM_newDevNum
};


/********* lib API DB *********/

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssPxConfigLogLibDb[] = {
    {"cpssPxCfgDevRemove", 1, prvCpssLogGenDevNum2_PARAMS, NULL},
    {"cpssPxCfgDevEnable", 2, prvCpssLogGenDevNumEnable2_PARAMS, NULL},
    {"cpssPxCfgDevEnableGet", 2, prvCpssLogGenDevNumEnablePtr2_PARAMS, NULL},
    {"cpssPxCfgNextDevGet", 2, cpssPxCfgNextDevGet_PARAMS, NULL},
    {"cpssPxCfgTableNumEntriesGet", 3, cpssPxCfgTableNumEntriesGet_PARAMS, NULL},
    {"cpssPxCfgReNumberDevNum", 2, cpssPxCfgReNumberDevNum_PARAMS, NULL},
    {"cpssPxCfgDevInfoGet", 2, cpssPxCfgDevInfoGet_PARAMS, NULL},
    {"cpssPxCfgHwAccessObjectBind", 2, cpssPxCfgHwAccessObjectBind_PARAMS, NULL},
    {"cpssPxCfgHitlessWriteMethodEnableSet", 2, prvCpssLogGenDevNumEnable2_PARAMS, NULL},
    {"cpssPxCfgLastRegAccessTimeStampGet", 2, cpssPxCfgLastRegAccessTimeStampGet_PARAMS, NULL},
    {"cpssPxCfgCntrGet", 3, cpssPxCfgCntrGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_PX_CPSS_LOG_LIB_CONFIG(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssPxConfigLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssPxConfigLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

