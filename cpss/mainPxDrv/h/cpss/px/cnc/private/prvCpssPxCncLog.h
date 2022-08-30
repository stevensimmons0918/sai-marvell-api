/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssPxCncLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssPxCncLogh
#define __prvCpssPxCncLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_CNC_BYTE_COUNT_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_CNC_CLIENT_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_CNC_COUNTER_FORMAT_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_PX_CNC_COUNTER_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_PX_CNC_BYTE_COUNT_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_CNC_BYTE_COUNT_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_CNC_CLIENT_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_CNC_COUNTER_FORMAT_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_CNC_COUNTER_FORMAT_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_CNC_COUNTER_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_INOUT_GT_U32_PTR_indexNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_INOUT_GT_U32_PTR_numOfCounterValuesPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_DMA_QUEUE_CFG_STC_PTR_cncCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_CNC_BYTE_COUNT_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_CNC_CLIENT_ENT_client;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_CNC_COUNTER_FORMAT_ENT_format;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_CNC_COUNTER_STC_PTR_counterPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_updateEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_blockNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U64_HEX_indexRangesBmp;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_CNC_BYTE_COUNT_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_CNC_COUNTER_FORMAT_ENT_PTR_formatPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_CNC_COUNTER_STC_PTR_counterPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_CNC_COUNTER_STC_PTR_counterValuesPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_updateEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_inProcessBlocksBmpPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_indexesArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U64_HEX_PTR_indexRangesBmpPtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssPxCncUploadInit_E = (CPSS_LOG_LIB_CNC_E << 16) | (2 << 24),
    PRV_CPSS_LOG_FUNC_cpssPxCncBlockClientEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCncBlockClientEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCncBlockClientRangesSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCncBlockClientRangesGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCncCounterFormatSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCncCounterFormatGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCncCounterClearByReadEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCncCounterClearByReadEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCncCounterClearByReadValueSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCncCounterClearByReadValueGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCncBlockUploadTrigger_E,
    PRV_CPSS_LOG_FUNC_cpssPxCncBlockUploadInProcessGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCncUploadedBlockGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCncCounterWraparoundEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCncCounterWraparoundEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCncCounterWraparoundIndexesGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCncCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCncEgressQueueClientModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCncEgressQueueClientModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCncClientByteCountModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCncClientByteCountModeGet_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssPxCncLogh */
