/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChSystemRecoveryLog.c
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
#include <cpss/dxCh/dxChxGen/bootChannel/private/prvCpssDxChBootChannelLog.h>
#include <cpss/dxCh/dxChxGen/systemRecovery/hsu/cpssDxChHsu.h>
#include <cpss/dxCh/dxChxGen/systemRecovery/private/prvCpssDxChSystemRecoveryLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssGenDbLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_DXCH_HSU_DATA_TYPE_ENT[]  =
{
    "CPSS_DXCH_HSU_DATA_TYPE_LPM_DB_E",
    "CPSS_DXCH_HSU_DATA_TYPE_GLOBAL_E",
    "CPSS_DXCH_HSU_DATA_TYPE_ALL_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_HSU_DATA_TYPE_ENT);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_HSU_DATA_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_HSU_DATA_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_HSU_DATA_TYPE_ENT);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_HSU_DATA_TYPE_ENT_dataType = {
     "dataType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_HSU_DATA_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_CHAR_PTR_miVersionPtr = {
     "miVersionPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_CHAR)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_newEvReqHndl = {
     "newEvReqHndl", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_oldEvReqHndl = {
     "oldEvReqHndl", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_isCompatiblePtr = {
     "isCompatiblePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_PRV_CPSS_BOOT_CH_PORT_STATUS_STC_PTR_portInfoPtr = {
     "portInfoPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(PRV_CPSS_BOOT_CH_PORT_STATUS_STC)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsuExport_PARAMS[] =  {
    &DX_IN_CPSS_DXCH_HSU_DATA_TYPE_ENT_dataType,
    &DX_INOUT_GT_UINTPTR_PTR_iteratorPtr,
    &DX_INOUT_GT_U32_PTR_hsuBlockMemSizePtr,
    &DX_IN_GT_U8_PTR_hsuBlockMemPtr,
    &DX_OUT_GT_BOOL_PTR_exportCompletePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsuImport_PARAMS[] =  {
    &DX_IN_CPSS_DXCH_HSU_DATA_TYPE_ENT_dataType,
    &DX_INOUT_GT_UINTPTR_PTR_iteratorPtr,
    &DX_INOUT_GT_U32_PTR_hsuBlockMemSizePtr,
    &DX_IN_GT_U8_PTR_hsuBlockMemPtr,
    &DX_OUT_GT_BOOL_PTR_importCompletePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHsuBlockSizeGet_PARAMS[] =  {
    &DX_IN_CPSS_DXCH_HSU_DATA_TYPE_ENT_dataType,
    &DX_OUT_GT_U32_PTR_sizePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssHsuEventHandleUpdate_PARAMS[] =  {
    &DX_IN_GT_U32_oldEvReqHndl,
    &DX_IN_GT_U32_newEvReqHndl
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHitlessStartupMiCompatibilityCheck_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_CHAR_PTR_miVersionPtr,
    &DX_OUT_GT_BOOL_PTR_isCompatiblePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChHitlessStartupPortParamsGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_PRV_CPSS_BOOT_CH_PORT_STATUS_STC_PTR_portInfoPtr
};


/********* lib API DB *********/

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChSystemRecoveryLogLibDb[] = {
    {"cpssDxChHitlessStartupMiCompatibilityCheck", 3, cpssDxChHitlessStartupMiCompatibilityCheck_PARAMS, NULL},
    {"cpssDxChHitlessStartupPortParamsGet", 3, cpssDxChHitlessStartupPortParamsGet_PARAMS, NULL},
    {"cpssDxChHitlessStartupSyncPortManager", 1, prvCpssLogGenDevNum_PARAMS, NULL},
    {"cpssDxChHsuBlockSizeGet", 2, cpssDxChHsuBlockSizeGet_PARAMS, NULL},
    {"cpssDxChHsuExport", 5, cpssDxChHsuExport_PARAMS, NULL},
    {"cpssDxChHsuImport", 5, cpssDxChHsuImport_PARAMS, NULL},
    {"cpssDxChCatchUpValidityCheckEnable", 1, prvCpssLogGenDevNum_PARAMS, NULL},
    {"cpssDxChCatchUpValidityCheck", 1, prvCpssLogGenDevNum_PARAMS, NULL},
    {"cpssHsuEventHandleUpdate", 2, cpssHsuEventHandleUpdate_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_SYSTEM_RECOVERY(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChSystemRecoveryLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChSystemRecoveryLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

