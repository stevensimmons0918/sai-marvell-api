/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssPxNetworkIfLog.c
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
#include <cpss/common/cpssHwInit/private/prvCpssCommonCpssHwInitLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssPxGenDbLog.h>
#include <cpss/px/networkIf/cpssPxNetIfTypes.h>
#include <cpss/px/networkIf/private/prvCpssPxNetworkIfLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT[]  =
{
    "CPSS_PX_NET_RESOURCE_ERROR_MODE_RETRY_E",
    "CPSS_PX_NET_RESOURCE_ERROR_MODE_ABORT_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_PX_NET_SDMA_RX_COUNTERS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_NET_SDMA_RX_COUNTERS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, rxInPkts);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, rxInOctets);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_NET_TX_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_NET_TX_PARAMS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, txQueue);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, recalcCrc);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_NET_SDMA_RX_COUNTERS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_NET_SDMA_RX_COUNTERS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_NET_SDMA_RX_COUNTERS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_NET_TX_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_NET_TX_PARAMS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_NET_TX_PARAMS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC PX_INOUT_GT_U32_PTR_numOfBuffPtr = {
     "numOfBuffPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_NET_IF_CFG_STC_PTR_netIfCfgPtr = {
     "netIfCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_NET_IF_CFG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT_mode = {
     "mode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_NET_TX_PARAMS_STC_PTR_pcktParamsPtr = {
     "pcktParamsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_NET_TX_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_PTR_buffLenList = {
     "buffLenList", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_PTR_rxBuffSizeList = {
     "rxBuffSizeList", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_buffListLen = {
     "buffListLen", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_numOfBufs = {
     "numOfBufs", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_queue = {
     "queue", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_rxQueue = {
     "rxQueue", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_txQueue = {
     "txQueue", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U8_PTR_PTR_buffList = {
     "buffList", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_PTR_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U8_PTR_PTR_rxBuffList = {
     "rxBuffList", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_PTR_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_NET_SDMA_RX_ERROR_COUNTERS_STC_PTR_rxErrCountPtr = {
     "rxErrCountPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_NET_SDMA_RX_ERROR_COUNTERS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT_PTR_modePtr = {
     "modePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_NET_SDMA_RX_COUNTERS_STC_PTR_rxCountersPtr = {
     "rxCountersPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_NET_SDMA_RX_COUNTERS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_buffLenArr = {
     "buffLenArr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_numberOfFreeTxDescriptorsPtr = {
     "numberOfFreeTxDescriptorsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U8_PTR_PTR_packetBuffsArrPtr = {
     "packetBuffsArrPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_PTR_MAC(GT_U8)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxNetIfInit_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_NET_IF_CFG_STC_PTR_netIfCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxNetIfSdmaRxResourceErrorModeSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_queue,
    &PX_IN_CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT_mode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxNetIfSdmaRxResourceErrorModeGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_queue,
    &PX_OUT_CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT_PTR_modePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxNetIfSdmaRxQueueEnable_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_rxQueue,
    &PX_IN_GT_BOOL_enable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxNetIfSdmaRxCountersGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_rxQueue,
    &PX_OUT_CPSS_PX_NET_SDMA_RX_COUNTERS_STC_PTR_rxCountersPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxNetIfSdmaRxQueueEnableGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_rxQueue,
    &PX_OUT_GT_BOOL_PTR_enablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxNetIfSdmaTxQueueEnable_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_txQueue,
    &PX_IN_GT_BOOL_enable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxNetIfSdmaTxQueueEnableGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_txQueue,
    &PX_OUT_GT_BOOL_PTR_enablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxNetIfSdmaTxFreeDescripotrsNumberGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_txQueue,
    &PX_OUT_GT_U32_PTR_numberOfFreeTxDescriptorsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxNetIfSdmaRxErrorCountGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_CPSS_NET_SDMA_RX_ERROR_COUNTERS_STC_PTR_rxErrCountPtr
};
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxNetIfRxBufFree_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxNetIfRxBufFreeWithSize_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxNetIfSdmaRxPacketGet_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxNetIfSdmaSyncTxPacketSend_PARAMS[];


/********* lib API DB *********/

extern void cpssPxNetIfRxBufFree_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssPxNetIfRxBufFreeWithSize_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssPxNetIfSdmaRxPacketGet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssPxNetworkIfLogLibDb[] = {
    {"cpssPxNetIfSdmaRxResourceErrorModeSet", 3, cpssPxNetIfSdmaRxResourceErrorModeSet_PARAMS, NULL},
    {"cpssPxNetIfSdmaRxResourceErrorModeGet", 3, cpssPxNetIfSdmaRxResourceErrorModeGet_PARAMS, NULL},
    {"cpssPxNetIfInit", 2, cpssPxNetIfInit_PARAMS, NULL},
    {"cpssPxNetIfRxBufFree", 4, cpssPxNetIfRxBufFree_PARAMS, cpssPxNetIfRxBufFree_preLogic},
    {"cpssPxNetIfRxBufFreeWithSize", 5, cpssPxNetIfRxBufFreeWithSize_PARAMS, cpssPxNetIfRxBufFreeWithSize_preLogic},
    {"cpssPxNetIfSdmaRxPacketGet", 5, cpssPxNetIfSdmaRxPacketGet_PARAMS, cpssPxNetIfSdmaRxPacketGet_preLogic},
    {"cpssPxNetIfSdmaRxCountersGet", 3, cpssPxNetIfSdmaRxCountersGet_PARAMS, NULL},
    {"cpssPxNetIfSdmaRxErrorCountGet", 2, cpssPxNetIfSdmaRxErrorCountGet_PARAMS, NULL},
    {"cpssPxNetIfSdmaRxQueueEnable", 3, cpssPxNetIfSdmaRxQueueEnable_PARAMS, NULL},
    {"cpssPxNetIfSdmaRxQueueEnableGet", 3, cpssPxNetIfSdmaRxQueueEnableGet_PARAMS, NULL},
    {"cpssPxNetIfSdmaSyncTxPacketSend", 5, cpssPxNetIfSdmaSyncTxPacketSend_PARAMS, NULL},
    {"cpssPxNetIfSdmaTxQueueEnable", 3, cpssPxNetIfSdmaTxQueueEnable_PARAMS, NULL},
    {"cpssPxNetIfSdmaTxQueueEnableGet", 3, cpssPxNetIfSdmaTxQueueEnableGet_PARAMS, NULL},
    {"cpssPxNetIfSdmaTxFreeDescripotrsNumberGet", 3, cpssPxNetIfSdmaTxFreeDescripotrsNumberGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_PX_CPSS_LOG_LIB_NETWORK_IF(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssPxNetworkIfLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssPxNetworkIfLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

