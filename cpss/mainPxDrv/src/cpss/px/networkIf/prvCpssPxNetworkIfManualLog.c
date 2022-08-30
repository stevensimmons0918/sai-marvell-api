/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssPxNetworkIfManualLog.c
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
#include <cpss/px/networkIf/cpssPxNetIf.h>
#include <cpss/px/networkIf/private/prvCpssPxNetworkIfLog.h>

/********* parameters log functions *********/
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U8_PTR_ARRAY_rxBuffList = {
     "rxBuffList", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_ARRAY_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_ARRAY_rxBuffSizeList = {
     "rxBuffSizeList", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_ARRAY_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U8_ARRAY_packetBuffsArrPtr = {
     "packetBuffsArrPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_ARRAY_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_ARRAY_buffLenArr = {
     "buffLenArr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_ARRAY_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U8_ARRAY_buffList = {
     "buffList", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_ARRAY_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_ARRAY_buffLenList = {
     "buffLenList", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_ARRAY_MAC(GT_U32)
};


const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxNetIfRxBufFreeWithSize_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_rxQueue,
    &PX_IN_GT_U8_PTR_ARRAY_rxBuffList,
    &PX_IN_GT_U32_ARRAY_rxBuffSizeList,
    &PX_IN_GT_U32_buffListLen
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxNetIfSdmaRxPacketGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_rxQueue,
    &PX_INOUT_GT_U32_PTR_numOfBuffPtr,
    &PX_OUT_GT_U8_ARRAY_packetBuffsArrPtr,
    &PX_OUT_GT_U32_ARRAY_buffLenArr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxNetIfSdmaSyncTxPacketSend_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_NET_TX_PARAMS_STC_PTR_pcktParamsPtr,
    &PX_IN_GT_U8_ARRAY_buffList,
    &PX_IN_GT_U32_ARRAY_buffLenList,
    &PX_IN_GT_U32_numOfBufs
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxNetIfRxBufFree_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_rxQueue,
    &PX_IN_GT_U8_PTR_ARRAY_rxBuffList,
    &PX_IN_GT_U32_buffListLen
};

/********* api pre-log functions *********/
void cpssPxNetIfRxBufFree_preLogic
(
    IN va_list  args,
    OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr
)
{
    /* devNum */
    va_arg(args, GT_U32);
    /* rxQueue */
    va_arg(args, GT_U32);
    /* rxBuffList */
    va_arg(args, GT_U8**);
    /* buffListLen */
    paramDataPtr->paramKey.paramKeyArr[2] = (GT_U32)va_arg(args, GT_U32);
}
void cpssPxNetIfRxBufFreeWithSize_preLogic
(
    IN va_list  args,
    OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr
)
{
    GT_U32  *firstLenPtr;

    /* devNum */
    va_arg(args, GT_U32);
    /* rxQueue */
    va_arg(args, GT_U32);
    /* rxBuffList */
    va_arg(args, GT_U8**);
    /* rxBuffSizeList */
    firstLenPtr = (GT_U32 *)va_arg(args, GT_U32 *);
    paramDataPtr->paramKey.paramKeyArr[4] = (GT_UINTPTR)firstLenPtr;
    /* buffListLen */
    paramDataPtr->paramKey.paramKeyArr[2] = (GT_U32)va_arg(args, GT_U32);
}
void cpssPxNetIfSdmaRxPacketGet_preLogic
(
    IN va_list  args,
    OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr
)
{
    GT_U32  *firstLenPtr;
    GT_U32  *sizePtr;

    /* devNum */
    va_arg(args, GT_U32);
    /* queueIdx */
    va_arg(args, GT_U32);
    /* numOfBuffPtr */
    sizePtr = (GT_U32 *)va_arg(args, GT_U32 *);
    paramDataPtr->paramKey.paramKeyArr[0] = (GT_UINTPTR)sizePtr;
    /* numOfBuff */
    if (paramDataPtr->paramKey.paramKeyArr[0] != 0)
        paramDataPtr->paramKey.paramKeyArr[1] = (GT_U32)*sizePtr;
    /* packetBuffsArrPtr */
    (GT_U8 **)va_arg(args, GT_U8 **);
    /* buffLenArr */
    firstLenPtr = (GT_U32 *)va_arg(args, GT_U32 *);
    paramDataPtr->paramKey.paramKeyArr[4] = (GT_UINTPTR)firstLenPtr;
}
