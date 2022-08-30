/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChBrgManualLog.c
*       Manually implemented CPSS Log type wrappers
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdb.h>
#include <cpss/generic/log/prvCpssGenLog.h>
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrgLog.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgStp.h>

/********* enums *********/
/********* structure fields log functions *********/
/********* parameters log functions *********/
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_ARRAY_crcMultiHashArr = {
     "crcMultiHashArr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_ARRAY_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_ARRAY_dipBytesSelectMapArr = {
     "dipBytesSelectMapArr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_ARRAY_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_ARRAY_sipBytesSelectMapArr = {
     "sipBytesSelectMapArr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_ARRAY_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_ARRAY_dipBytesSelectMapArr = {
     "dipBytesSelectMapArr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_ARRAY_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_ARRAY_sipBytesSelectMapArr = {
     "sipBytesSelectMapArr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_ARRAY_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_ARRAY_stpEntryWordArr = {
     "stpEntryWordArr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_ARRAY_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ARRAY_stpEntryWordArr = {
     "stpEntryWordArr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_ARRAY_MAC(GT_U32)
};

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChBrgFdbHashCrcMultiResultsByParamsCalc_PARAMS[] =  {
    &DX_IN_CPSS_DXCH_BRG_FDB_HASH_PARAMS_STC_PTR_hashParamsPtr,
    &DX_IN_CPSS_MAC_ENTRY_EXT_KEY_STC_PTR_entryKeyPtr,
    &DX_IN_GT_U32_multiHashStartBankIndex,
    &DX_IN_GT_U32_numOfBanks,
    &DX_OUT_GT_U32_ARRAY_crcMultiHashArr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChBrgFdbHashCrcMultiResultsCalc_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_MAC_ENTRY_EXT_KEY_STC_PTR_entryKeyPtr,
    &DX_IN_GT_U32_multiHashStartBankIndex,
    &DX_IN_GT_U32_numOfBanks,
    &DX_OUT_GT_U32_ARRAY_crcMultiHashArr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChBrgFdbHashResultsGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_multiHashStartBankIndex,
    &DX_IN_GT_U32_numOfBanks,
    &DX_OUT_GT_U32_ARRAY_crcMultiHashArr,
    &DX_INOUT_GT_U32_PTR_xorHashPtr,
    &DX_INOUT_GT_U32_PTR_crcHashPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChBrgMcIpv6BytesSelectSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U8_ARRAY_dipBytesSelectMapArr,
    &DX_IN_GT_U8_ARRAY_sipBytesSelectMapArr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChBrgMcIpv6BytesSelectGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U8_ARRAY_dipBytesSelectMapArr,
    &DX_OUT_GT_U8_ARRAY_sipBytesSelectMapArr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChBrgStpEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_dev,
    &DX_IN_GT_U16_stpId,
    &DX_OUT_GT_U32_ARRAY_stpEntryWordArr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChBrgStpEntryWrite_PARAMS[] =  {
    &DX_IN_GT_U8_dev,
    &DX_IN_GT_U16_stpId,
    &DX_IN_GT_U32_ARRAY_stpEntryWordArr
};
/********* api pre-log functions *********/
void cpssDxChBrgFdbFuMsgBlockGet_preLogic
(
    IN va_list  args,
    OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr
)
{
    GT_U32  *sizePtr; /*pointer to array size*/

    /* devNum */
    va_arg(args, GT_U32);
    /* actualNumOfFuPtr */
    sizePtr = (GT_U32 *)va_arg(args, GT_U32 *);
    paramDataPtr->paramKey.paramKeyArr[0] = (GT_UINTPTR)sizePtr;
    /* maxNumOfFu */
    if (paramDataPtr->paramKey.paramKeyArr[0] != 0)
        paramDataPtr->paramKey.paramKeyArr[1] = (GT_U32)*sizePtr;
    /* sign that the output might be an array of strucutures */
    paramDataPtr->paramKey.paramKeyArr[3] = 1;
}
void cpssDxChBrgFdbHashCrcMultiResultsByParamsCalc_preLogic
(
    IN va_list  args,
    OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr
)
{
    /* hashParamsPtr */
    va_arg(args, CPSS_DXCH_BRG_FDB_HASH_PARAMS_STC*);
    /* entryKeyPtr */
    va_arg(args, CPSS_MAC_ENTRY_EXT_KEY_STC*);
    /* multiHashStartBankIndex */
    va_arg(args, GT_U32);
    /* numOfBanks */
    paramDataPtr->paramKey.paramKeyArr[2] = va_arg(args, GT_U32);
}
void cpssDxChBrgFdbHashCrcMultiResultsCalc_preLogic
(
    IN va_list  args,
    OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr
)
{
    /* devNum */
    va_arg(args, GT_U32);
    /* entryKeyPtr */
    va_arg(args, CPSS_MAC_ENTRY_EXT_KEY_STC*);
    /* multiHashStartBankIndex */
    va_arg(args, GT_U32);
    /* numOfBanks */
    paramDataPtr->paramKey.paramKeyArr[2] = va_arg(args, GT_U32);
}
void cpssDxChBrgFdbHashResultsGet_preLogic
(
    IN va_list  args,
    OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr
)
{
    /* devNum */
    va_arg(args, GT_U32);
    /* multiHashStartBankIndex */
    va_arg(args, GT_U32);
    /* numOfBanks */
    paramDataPtr->paramKey.paramKeyArr[2] = va_arg(args, GT_U32);
}
void cpssDxChBrgMcIpv6BytesSelectSet_preLogic
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
void cpssDxChBrgStpEntryGet_preLogic
(
    IN va_list  args,
    OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr
)
{
    /* dev */
    va_arg(args, GT_U32);
    /* stpEntryWordArr size (constant) */
    paramDataPtr->paramKey.paramKeyArr[2] = CPSS_DXCH_STG_ENTRY_SIZE_CNS;
}
