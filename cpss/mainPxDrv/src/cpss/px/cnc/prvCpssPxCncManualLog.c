/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssPxCncManualLog.c
*       Manually implemented CPSS Log type wrappers
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/generic/log/prvCpssPxGenDbLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/px/cnc/cpssPxCnc.h>
#include <cpss/px/cnc/private/prvCpssPxCncLog.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
/********* enums *********/
/********* structure fields log functions *********/
void prvCpssLogParamFunc_CPSS_PX_CNC_COUNTER_STC_ARRAY
(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_CNC_COUNTER_STC*, paramVal);
    if(0 == inOutParamInfoPtr->paramKey.paramKeyArr[0])
    {
        return;
    }
    prvCpssLogParamFuncStcTypeArray(
        contextLib, logType, namePtr,
        paramVal,
        *(GT_U32 *)(inOutParamInfoPtr->paramKey.paramKeyArr[0]),
        sizeof(CPSS_PX_CNC_COUNTER_STC),
        PRV_CPSS_LOG_FUNC_STC_TYPE_PTR_MAC(CPSS_PX_CNC_COUNTER_STC),
        inOutParamInfoPtr
    );
}
/********* parameters log functions *********/
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_ARRAY_indexesArr = {
     "indexesArr", PRV_CPSS_LOG_PARAM_OUT_E, PRV_CPSS_LOG_FUNC_TYPE_ARRAY_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_CNC_COUNTER_STC_ARRAY_counterValuesPtr = {
     "counterValuesPtr", PRV_CPSS_LOG_PARAM_OUT_E, prvCpssLogParamFunc_CPSS_PX_CNC_COUNTER_STC_ARRAY
};

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCncCounterWraparoundIndexesGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_blockNum,
    &PX_INOUT_GT_U32_PTR_indexNumPtr,
    &PX_OUT_GT_U32_ARRAY_indexesArr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCncUploadedBlockGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_INOUT_GT_U32_PTR_numOfCounterValuesPtr,
    &PX_IN_CPSS_PX_CNC_COUNTER_FORMAT_ENT_format,
    &PX_OUT_CPSS_PX_CNC_COUNTER_STC_ARRAY_counterValuesPtr
};

/********* api pre-log functions *********/
void cpssPxCncUploadedBlockGet_preLogic
(
    IN va_list  args,
    OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr
)
{
    GT_U32  *sizePtr; /*pointer to array size*/

    /* devNum */
    va_arg(args, GT_U32);
    /* numOfCounterValuesPtr */
    sizePtr = (GT_U32 *)va_arg(args, GT_U32 *);
    paramDataPtr->paramKey.paramKeyArr[0] = (GT_UINTPTR)sizePtr;
    /* maxNumOfCounterValues */
    if (paramDataPtr->paramKey.paramKeyArr[0] != 0)
        paramDataPtr->paramKey.paramKeyArr[1] = (GT_U32)*sizePtr;
    /* sign that the output might be an array of strucutures */
    paramDataPtr->paramKey.paramKeyArr[3] = 1;
}
void cpssPxCncCounterWraparoundIndexesGet_preLogic
(
    IN va_list  args,
    OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr
)
{
    GT_U32  *sizePtr; /*pointer to array size*/

    /* devNum */
    va_arg(args, GT_U32);
    /* blockNum */
    va_arg(args, GT_U32);
    /* indexNumPtr */
    sizePtr = (GT_U32 *)va_arg(args, GT_U32 *);
    paramDataPtr->paramKey.paramKeyArr[0] = (GT_UINTPTR)sizePtr;
    /* maxIndexNumPtr */
    if (paramDataPtr->paramKey.paramKeyArr[0] != 0)
        paramDataPtr->paramKey.paramKeyArr[1] = (GT_U32)*sizePtr;
    /* sign that the output might be an array of strucutures */
    paramDataPtr->paramKey.paramKeyArr[3] = 1;
}
