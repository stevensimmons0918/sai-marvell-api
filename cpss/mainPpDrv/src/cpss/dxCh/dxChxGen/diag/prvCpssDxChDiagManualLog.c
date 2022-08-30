/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChDiagManualLog.c
*       Manually implemented CPSS Log type wrappers
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiag.h>
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagDescriptor.h>
#include <cpss/dxCh/dxChxGen/diag/private/prvCpssDxChDiagLog.h>
#include <cpss/generic/log/prvCpssGenLog.h>
#include <cpss/generic/diag/private/prvCpssGenDiagLog.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
/********* enums *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_ARRAY_regAddrPtr = {
     "regAddrPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_ARRAY_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_ARRAY_regDataPtr = {
     "regDataPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_ARRAY_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ARRAY_cyclicDataArr = {
     "cyclicDataArr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_ARRAY_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_ARRAY_cyclicDataArr = {
     "cyclicDataArr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_ARRAY_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_ARRAY_rawDataPtr = {
     "rawDataPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_ARRAY_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChDiagRegsDump_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_INOUT_GT_U32_PTR_regsNumPtr,
    &DX_IN_GT_U32_offset,
    &DX_OUT_GT_U32_ARRAY_regAddrPtr,
    &DX_OUT_GT_U32_ARRAY_regDataPtr
};

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChDiagPrbsCyclicDataSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U32_laneNum,
    &DX_IN_GT_U32_ARRAY_cyclicDataArr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChDiagPrbsCyclicDataGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U32_laneNum,
    &DX_OUT_GT_U32_ARRAY_cyclicDataArr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChDiagPortGroupRegsDump_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_INOUT_GT_U32_PTR_regsNumPtr,
    &DX_IN_GT_U32_offset,
    &DX_OUT_GT_U32_ARRAY_regAddrPtr,
    &DX_OUT_GT_U32_ARRAY_regDataPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChDiagDescriptorRawGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_DIAG_DESCRIPTOR_TYPE_ENT_descriptorType,
    &DX_INOUT_GT_U32_PTR_rawDataLengthPtr,
    &DX_OUT_GT_U32_ARRAY_rawDataPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChDiagDescriptorPortGroupRawGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_CPSS_DXCH_DIAG_DESCRIPTOR_TYPE_ENT_descriptorType,
    &DX_INOUT_GT_U32_PTR_rawDataLengthPtr,
    &DX_OUT_GT_U32_ARRAY_rawDataPtr
};
/********* structure fields log functions *********/
void prvCpssLogParamFunc_CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC*, paramVal);
    prvCpssLogArrayOfParamsHandle(contextLib,logType,namePtr,(void*) paramVal, sizeof(CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC), inOutParamInfoPtr,
                                  prvCpssLogParamFuncStc_CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC_PTR);
}
void prvCpssLogParamFunc_CPSS_DXCH_DIAG_SERDES_TUNE_PORT_LANE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_DIAG_SERDES_TUNE_PORT_LANE_STC*, paramVal);
    prvCpssLogArrayOfParamsHandle(contextLib,logType,namePtr,(void*) paramVal, sizeof(CPSS_DXCH_DIAG_SERDES_TUNE_PORT_LANE_STC), inOutParamInfoPtr,
                                  prvCpssLogParamFuncStc_CPSS_DXCH_DIAG_SERDES_TUNE_PORT_LANE_STC_PTR);
}
void prvCpssLogParamFunc_CPSS_DXCH_DIAG_BIST_RESULT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_DIAG_BIST_RESULT_STC*, paramVal);
    prvCpssLogArrayOfParamsHandle(contextLib,logType,namePtr,(void*) paramVal, sizeof(CPSS_DXCH_DIAG_BIST_RESULT_STC), inOutParamInfoPtr,
                                  prvCpssLogParamFuncStc_CPSS_DXCH_DIAG_BIST_RESULT_STC_PTR);
}
void prvCpssLogParamFunc_CPSS_DXCH_DIAG_EXT_MEMORY_INF_BIST_ERROR_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_DIAG_EXT_MEMORY_INF_BIST_ERROR_STC*, paramVal);
    prvCpssLogArrayOfParamsHandle(contextLib,logType,namePtr,(void*) paramVal, sizeof(CPSS_DXCH_DIAG_EXT_MEMORY_INF_BIST_ERROR_STC), inOutParamInfoPtr,
                                  prvCpssLogParamFuncStc_CPSS_DXCH_DIAG_EXT_MEMORY_INF_BIST_ERROR_STC_PTR);
}
/********* parameters log functions *********/

void cpssDxChDiagRegsDump_preLogic
(
    IN va_list  args,
    OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr
)
{
    GT_U32  *sizePtr; /*pointer to array size*/

    /* devNum */
    va_arg(args, GT_U32);
    /* maxRegsNumPtr */
    sizePtr = (GT_U32 *)va_arg(args, GT_U32 *);
    paramDataPtr->paramKey.paramKeyArr[0] = (GT_UINTPTR)sizePtr;
    if (paramDataPtr->paramKey.paramKeyArr[0] != 0)
        paramDataPtr->paramKey.paramKeyArr[1] = (GT_U32)*sizePtr;
    /* regsNumPtr */
    paramDataPtr->paramKey.paramKeyArr[2] = (GT_UINTPTR)va_arg(args, GT_U32 *);
    /* sign that the output might be an array of strucutures */
    paramDataPtr->paramKey.paramKeyArr[3] = 1;
}
void cpssDxChDiagPortGroupRegsDump_preLogic
(
    IN va_list  args,
    OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr
)
{
    GT_U32  *sizePtr; /*pointer to array size*/

    /* devNum */
    va_arg(args, GT_U32);
    /* portGroupsBmp */
    va_arg(args, GT_PORT_GROUPS_BMP);
    /* maxRegsNumPtr */
    sizePtr = (GT_U32 *)va_arg(args, GT_U32 *);
    paramDataPtr->paramKey.paramKeyArr[0] = (GT_UINTPTR)sizePtr;
    if (paramDataPtr->paramKey.paramKeyArr[0] != 0)
        paramDataPtr->paramKey.paramKeyArr[1] = (GT_U32)*sizePtr;
    /* regsNumPtr */
    paramDataPtr->paramKey.paramKeyArr[2] = (GT_UINTPTR)va_arg(args, GT_U32 *);
    /* sign that the output might be an array of strucutures */
    paramDataPtr->paramKey.paramKeyArr[3] = 1;
}

void cpssDxChDiagPrbsCyclicDataSet_preLogic
(
    IN va_list  args,
    OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr
)
{
    /* devNum */
    va_arg(args, GT_U32);
    /* dipBytesSelectMapArr and sipBytesSelectMapArr size (constant) */
    paramDataPtr->paramKey.paramKeyArr[2] = 4;
}

void cpssDxChDiagSerdesTuningRxTune_preLogic
(
    IN va_list  args,
    OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr
)
{
    /* devNum */
    va_arg(args, GT_U32);
    /* portLaneArrPtr */
    va_arg(args, CPSS_DXCH_DIAG_SERDES_TUNE_PORT_LANE_STC*);
    /* portLaneArrLength */
    paramDataPtr->paramKey.paramKeyArr[2] = (GT_U32)va_arg(args, GT_U32);
}

void cpssDxChDiagBistResultsGet_preLogic
(
    IN va_list  args,
    OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr
)
{
    GT_U32  *sizePtr; /*pointer to array size*/

    /* devNum */
    va_arg(args, GT_U32);
    /* resultsStatusPtr */
    va_arg(args, CPSS_DXCH_DIAG_BIST_STATUS_ENT*);
    /* resultsArr */
    va_arg(args, CPSS_DXCH_DIAG_BIST_RESULT_STC*);
   /* maxResultsNumPtr */
    sizePtr = (GT_U32 *)va_arg(args, GT_U32 *);
    paramDataPtr->paramKey.paramKeyArr[0] = (GT_UINTPTR)sizePtr;
    if (paramDataPtr->paramKey.paramKeyArr[0] != 0)
        paramDataPtr->paramKey.paramKeyArr[1] = (GT_U32)*sizePtr;
    /* resultsNumPtr */
    paramDataPtr->paramKey.paramKeyArr[2] = (GT_UINTPTR)va_arg(args, GT_U32 *);
    /* sign that the output might be an array of strucutures */
    paramDataPtr->paramKey.paramKeyArr[3] = 1;
}

void cpssDxChDiagExternalMemoriesBistRun_preLogic
(
    IN va_list  args,
    OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr
)
{
    /* devNum */
    va_arg(args, GT_U32);
    /* errorInfoArr size (constant) */
    paramDataPtr->paramKey.paramKeyArr[2] = CPSS_DXCH_DIAG_EXT_MEMORY_INF_NUM_CNS;
}

void cpssDxChDiagDataIntegrityEventsGet_preLogic
(
    IN va_list  args,
    OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr
)
{
    GT_U32  *sizePtr; /*pointer to array size*/

    /* devNum */
    va_arg(args, GT_U32);
    /* evExtData */
    va_arg(args, GT_U32);
    /* eventsNumPtr */
    sizePtr = (GT_U32 *)va_arg(args, GT_U32 *);

    paramDataPtr->paramKey.paramKeyArr[0] = (GT_UINTPTR)sizePtr;
    if (paramDataPtr->paramKey.paramKeyArr[0] != 0)
        paramDataPtr->paramKey.paramKeyArr[1] = (GT_U32)*sizePtr;

    paramDataPtr->paramKey.paramKeyArr[2] = (GT_UINTPTR)sizePtr;
    /* sign that the output might be an array of strucutures */
    paramDataPtr->paramKey.paramKeyArr[3] = 1;
}

void cpssDxChDiagDataIntegrityErrorInfoGet_preLogic
(
    IN va_list  args,
    OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr
)
{
    /* devNum */
    va_arg(args, GT_U32);
    /* memType */
    paramDataPtr->paramKey.paramKeyArr[4] = (CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT)va_arg(args, CPSS_DXCH_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT);
}

void cpssDxChDiagDescriptorRawGet_preLogic
(
    IN va_list  args,
    OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr
)
{
    GT_U32  *sizePtr; /*pointer to array size*/

    /* devNum */
    va_arg(args, GT_U32);
    /* descriptorType */
    va_arg(args, CPSS_DXCH_DIAG_DESCRIPTOR_TYPE_ENT);
    /* maxRawDataLengthPtr */
    sizePtr = (GT_U32 *)va_arg(args, GT_U32 *);
    paramDataPtr->paramKey.paramKeyArr[0] = (GT_UINTPTR)sizePtr;
    if (paramDataPtr->paramKey.paramKeyArr[0] != 0)
        paramDataPtr->paramKey.paramKeyArr[1] = (GT_U32)*sizePtr;
    /* rawDataLengthPtr */
    paramDataPtr->paramKey.paramKeyArr[2] = (GT_UINTPTR)va_arg(args, GT_U32 *);
    /* sign that the output might be an array of strucutures */
    paramDataPtr->paramKey.paramKeyArr[3] = 1;
}
void cpssDxChDiagDescriptorPortGroupRawGet_preLogic
(
    IN va_list  args,
    OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr
)
{
    GT_U32  *sizePtr; /*pointer to array size*/

    /* devNum */
    va_arg(args, GT_U32);
    /* portGroupsBmp */
    va_arg(args, GT_PORT_GROUPS_BMP);
    /* descriptorType */
    va_arg(args, CPSS_DXCH_DIAG_DESCRIPTOR_TYPE_ENT);
    /* maxRawDataLengthPtr */
    sizePtr = (GT_U32 *)va_arg(args, GT_U32 *);
    paramDataPtr->paramKey.paramKeyArr[0] = (GT_UINTPTR)sizePtr;
    if (paramDataPtr->paramKey.paramKeyArr[0] != 0)
        paramDataPtr->paramKey.paramKeyArr[1] = (GT_U32)*sizePtr;
    /* rawDataLengthPtr */
    paramDataPtr->paramKey.paramKeyArr[2] = (GT_UINTPTR)va_arg(args, GT_U32 *);
    /* sign that the output might be an array of strucutures */
    paramDataPtr->paramKey.paramKeyArr[3] = 1;
}
