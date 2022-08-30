/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChProtectionLog.c
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
#include <cpss/dxCh/dxChxGen/protection/cpssDxChProtection.h>
#include <cpss/dxCh/dxChxGen/protection/private/prvCpssDxChProtectionLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssGenDbLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_DXCH_PROTECTION_LOC_STATUS_ENT[]  =
{
    "CPSS_DXCH_PROTECTION_LOC_NOT_DETECTED_E",
    "CPSS_DXCH_PROTECTION_LOC_DETECTED_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PROTECTION_LOC_STATUS_ENT);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_PROTECTION_LOC_STATUS_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PROTECTION_LOC_STATUS_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PROTECTION_LOC_STATUS_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PROTECTION_LOC_STATUS_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PROTECTION_LOC_STATUS_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_PROTECTION_LOC_STATUS_ENT);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PROTECTION_LOC_STATUS_ENT_status = {
     "status", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PROTECTION_LOC_STATUS_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_locTableIndex = {
     "locTableIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PROTECTION_LOC_STATUS_ENT_PTR_statusPtr = {
     "statusPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PROTECTION_LOC_STATUS_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_locTableIndexPtr = {
     "locTableIndexPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChProtectionPortToLocMappingSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_GT_U32_locTableIndex
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChProtectionPortToLocMappingGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_OUT_GT_U32_PTR_locTableIndexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChProtectionLocStatusSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_IN_CPSS_DXCH_PROTECTION_LOC_STATUS_ENT_status
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChProtectionLocStatusGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_OUT_CPSS_DXCH_PROTECTION_LOC_STATUS_ENT_PTR_statusPtr
};


/********* lib API DB *********/

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChProtectionLogLibDb[] = {
    {"cpssDxChProtectionEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChProtectionEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChProtectionTxEnableSet", 3, prvCpssLogGenDevNumPortNumEnable2_PARAMS, NULL},
    {"cpssDxChProtectionTxEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr2_PARAMS, NULL},
    {"cpssDxChProtectionPortToLocMappingSet", 3, cpssDxChProtectionPortToLocMappingSet_PARAMS, NULL},
    {"cpssDxChProtectionPortToLocMappingGet", 3, cpssDxChProtectionPortToLocMappingGet_PARAMS, NULL},
    {"cpssDxChProtectionLocStatusSet", 3, cpssDxChProtectionLocStatusSet_PARAMS, NULL},
    {"cpssDxChProtectionLocStatusGet", 3, cpssDxChProtectionLocStatusGet_PARAMS, NULL},
    {"cpssDxChProtectionRxExceptionPacketCommandSet", 2, prvCpssLogGenDevNumCommand_PARAMS, NULL},
    {"cpssDxChProtectionRxExceptionPacketCommandGet", 2, prvCpssLogGenDevNumCommandPtr_PARAMS, NULL},
    {"cpssDxChProtectionRxExceptionCpuCodeSet", 2, prvCpssLogGenDevNumCpuCode_PARAMS, NULL},
    {"cpssDxChProtectionRxExceptionCpuCodeGet", 2, prvCpssLogGenDevNumCpuCodePtr_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_PROTECTION(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChProtectionLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChProtectionLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

