/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssPxNetworkIfLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssPxNetworkIfLogh
#define __prvCpssPxNetworkIfLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_PX_NET_SDMA_RX_COUNTERS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_NET_TX_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_NET_SDMA_RX_COUNTERS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_NET_TX_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_INOUT_GT_U32_PTR_numOfBuffPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_NET_IF_CFG_STC_PTR_netIfCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_NET_TX_PARAMS_STC_PTR_pcktParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_PTR_buffLenList;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_PTR_rxBuffSizeList;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_buffListLen;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_numOfBufs;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_queue;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_rxQueue;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_txQueue;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U8_PTR_PTR_buffList;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U8_PTR_PTR_rxBuffList;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_NET_SDMA_RX_ERROR_COUNTERS_STC_PTR_rxErrCountPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_NET_SDMA_RX_COUNTERS_STC_PTR_rxCountersPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_buffLenArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_numberOfFreeTxDescriptorsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U8_PTR_PTR_packetBuffsArrPtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssPxNetIfSdmaRxResourceErrorModeSet_E = (CPSS_LOG_LIB_NETWORK_IF_E << 16) | (2 << 24),
    PRV_CPSS_LOG_FUNC_cpssPxNetIfSdmaRxResourceErrorModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxNetIfInit_E,
    PRV_CPSS_LOG_FUNC_cpssPxNetIfRxBufFree_E,
    PRV_CPSS_LOG_FUNC_cpssPxNetIfRxBufFreeWithSize_E,
    PRV_CPSS_LOG_FUNC_cpssPxNetIfSdmaRxPacketGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxNetIfSdmaRxCountersGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxNetIfSdmaRxErrorCountGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxNetIfSdmaRxQueueEnable_E,
    PRV_CPSS_LOG_FUNC_cpssPxNetIfSdmaRxQueueEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxNetIfSdmaSyncTxPacketSend_E,
    PRV_CPSS_LOG_FUNC_cpssPxNetIfSdmaTxQueueEnable_E,
    PRV_CPSS_LOG_FUNC_cpssPxNetIfSdmaTxQueueEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxNetIfSdmaTxFreeDescripotrsNumberGet_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssPxNetworkIfLogh */
