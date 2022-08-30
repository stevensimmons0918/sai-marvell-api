/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChHsrPrpLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssDxChHsrPrpLogh
#define __prvCpssDxChHsrPrpLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_HSR_PRP_COUNTER_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_HSR_PRP_PNT_OPER_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_HSR_PRP_PNT_STATE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_HSR_PRP_PORTS_BMP_ID_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_HSR_PRP_FDB_DDL_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_HSR_PRP_LRE_PORT_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_HSR_PRP_PNT_LOOKUP_MODE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_HSR_PRP_DDE_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_HSR_PRP_DDE_HASH_KEY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_HSR_PRP_HPORT_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_HSR_PRP_LRE_INSTANCE_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_HSR_PRP_PNT_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_COUNTER_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_DDE_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_DDE_HASH_KEY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_HPORT_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_LRE_INSTANCE_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_PNT_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_PNT_OPER_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_PNT_STATE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_PORTS_BMP_ID_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_HSR_PRP_COUNTER_TYPE_ENT_counterType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_HSR_PRP_DDE_ENTRY_STC_PTR_entryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_HSR_PRP_DDE_HASH_KEY_STC_PTR_entryKeyPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_HSR_PRP_DDE_TIME_FIELD_ENT_field;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_HSR_PRP_EXCEPTION_TYPE_ENT_type;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_HSR_PRP_HPORT_ENTRY_STC_PTR_infoPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_HSR_PRP_LRE_INSTANCE_ENTRY_STC_PTR_infoPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_HSR_PRP_PNT_ENTRY_STC_PTR_infoPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_HSR_PRP_PNT_OPER_ENT_operation;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_HSR_PRP_PORTS_BMP_ID_ENT_bmpType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ENT_action;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORTS_SHORT_BMP_STC_bmp;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_PORT_NUM_baseEport;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_hPort;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_lanId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_lsduOffset;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_startBank;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_timeInMicroSec;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_HSR_PRP_DDE_ENTRY_STC_PTR_entryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_HSR_PRP_DDE_TIMER_GRANULARITY_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_HSR_PRP_FDB_PARTITION_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_HSR_PRP_HPORT_ENTRY_STC_PTR_infoPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_HSR_PRP_LRE_INSTANCE_ENTRY_STC_PTR_infoPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_HSR_PRP_PNT_ENTRY_STC_PTR_infoPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_HSR_PRP_PNT_STATE_ENT_PTR_statePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_HSR_PRP_PRP_PADDING_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_HSR_PRP_PRP_TRAILER_ACTION_ENT_PTR_actionPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORTS_SHORT_BMP_STC_PTR_bmpPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_PORT_NUM_PTR_baseEportPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_hashArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_lanIdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_lsduOffsetPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_numOfEntriesDeletedPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_timeInMicroSecPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U64_PTR_firstCounterPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U64_PTR_lreADiscardCounterPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U64_PTR_lreBDiscardCounterPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U64_PTR_secondCounterPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U64_PTR_thirdCounterPtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpExceptionCommandSet_E = (CPSS_LOG_LIB_HSR_PRP_E << 16) | (1 << 24),
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpExceptionCommandGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpExceptionCpuCodeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpExceptionCpuCodeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpPortPrpEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpPortPrpEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpPortPrpLanIdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpPortPrpLanIdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpPortPrpTreatWrongLanIdAsRctExistsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpPortsBmpSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpPortsBmpGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpHPortSrcEPortBaseSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpHPortSrcEPortBaseGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpPntNumOfReadyEntriesThresholdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpPntNumOfReadyEntriesThresholdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpHPortEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpHPortEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpPntEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpPntEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpPntAgingApply_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpLreInstanceEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpLreInstanceEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpFdbPartitionModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpFdbPartitionModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpDdeTimerGranularityModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpDdeTimerGranularityModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpDdeTimeFieldSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpDdeTimeFieldGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpPortPrpTrailerActionSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpPortPrpTrailerActionGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpPortPrpPaddingModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpPortPrpPaddingModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpDdeEntryInvalidate_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpDdeEntryWrite_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpDdeEntryRead_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpDdeAgingApply_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpDdeHashCalc_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpPortTrgPrpBaseLsduOffsetGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsrPrpPortSrcHsrBaseLsduOffsetGet_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssDxChHsrPrpLogh */
