/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChPtpLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssDxChPtpLogh
#define __prvCpssDxChPtpLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PTP_REF_CLOCK_SELECT_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PTP_TAI_CLOCK_FREQ_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PTP_TAI_CLOCK_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PTP_TAI_CLOCK_SELECT_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PTP_TAI_NUMBER_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PTP_TAI_PIN_SELECT_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PTP_TAI_SELECT_UNIT_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PTP_TAI_TOD_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PTP_TS_ACTION_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PTP_U_BIT_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PTP_TAI_INSTANCE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PTP_TRANSPORT_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PTP_TS_PACKET_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PTP_TS_TAG_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_EGRESS_EXCEPTION_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_EGRESS_EXCEPTION_COUNTERS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_INGRESS_EXCEPTION_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TAI_ID_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TAI_TOD_STEP_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TOD_COUNT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TSU_CONTROL_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TSU_FRAME_COUNTER_CFG_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TSU_TIMESTAMP_MASKING_PROFILE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TS_CFG_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TS_EGRESS_PIPE_DELAY_VALUES_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TS_LOCAL_ACTION_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TS_TAG_PORT_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_EGRESS_EXCEPTION_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_EGRESS_EXCEPTION_COUNTERS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_INGRESS_EXCEPTION_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_REF_CLOCK_SELECT_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_REF_CLOCK_SELECT_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_CLOCK_FREQ_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_CLOCK_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_CLOCK_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_CLOCK_SELECT_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_CLOCK_SELECT_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_ID_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_NUMBER_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_NUMBER_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_PIN_SELECT_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_SELECT_UNIT_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_TOD_STEP_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_TOD_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TOD_COUNT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TSU_CONTROL_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TSU_FRAME_COUNTER_CFG_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TSU_TIMESTAMP_MASKING_PROFILE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TS_ACTION_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TS_CFG_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TS_EGRESS_PIPE_DELAY_VALUES_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TS_LOCAL_ACTION_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TS_TAG_PORT_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_U_BIT_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PTP_U_BIT_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DIRECTION_ENT_clockInterfaceDirection;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DIRECTION_ENT_pulseInterfaceDirection;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC_PTR_entryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_EGRESS_EXCEPTION_CFG_STC_PTR_egrExceptionCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_ENT_checkingMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_ENT_domainMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_INGRESS_EXCEPTION_CFG_STC_PTR_ingrExceptionCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_REF_CLOCK_SELECT_ENT_refClockSelect;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TAI_CLOCK_FREQ_ENT_clockFrequency;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_ENT_clockGenerationMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TAI_CLOCK_MODE_ENT_clockMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_ENT_clockReceptionMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TAI_CLOCK_SELECT_ENT_clockSelect;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_ENT_interfaceType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_ENT_pclkRcvrClkMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_ENT_pClkSelMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TAI_PIN_SELECT_ENT_pinId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT_pulseMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TAI_SELECT_UNIT_ENT_taiUnit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TAI_TOD_STEP_STC_PTR_todStepPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TAI_TOD_TYPE_ENT_todValueType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TSU_CONTROL_STC_PTR_controlPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TSU_FRAME_COUNTER_CFG_ENTRY_STC_PTR_frameCounterConfigPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_ENT_counterType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TSU_TIMESTAMP_MASKING_PROFILE_STC_PTR_maskingProfilePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TS_CFG_ENTRY_STC_PTR_entryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT_egrTimeCorrTaiSelMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TS_EGRESS_PIPE_DELAY_VALUES_STC_PTR_egrDelayValPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TS_LOCAL_ACTION_ENTRY_STC_PTR_entryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC_PTR_tsTagGlobalCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC_PTR_udpCsUpdModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_U_BIT_MODE_ENT_uBitMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_32_egrAsymmetryCorr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_32_fracNanoSecond;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_32_ingressDelay;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_32_ingressDelayCorr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_egrCorrFldPBEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_ingrCorrFldPBEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_resetRxUnit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_resetTxUnit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_tsuBypassEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_tsuEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_TAI_SELECT_BMP_taiSelectBmp;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_HEX_etherTypeIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_PTR_domainIdArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_captureIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_delay;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_domainId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_domainProfile;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_edge;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_egrTsTaiNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_frameCounter;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_idMapBmp;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_messageType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ntpTimeOffset;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_queueSize;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_rxTaiSelect;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_taiSelect;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_udpPortIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_udpPortNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DIRECTION_ENT_PTR_clockInterfaceDirectionPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DIRECTION_ENT_PTR_pulseInterfaceDirectionPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC_PTR_entryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_EGRESS_EXCEPTION_CFG_STC_PTR_egrExceptionCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_EGRESS_EXCEPTION_COUNTERS_STC_PTR_egrExceptionCntPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_ENT_PTR_checkingModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_ENT_PTR_domainModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_INGRESS_EXCEPTION_CFG_STC_PTR_ingrExceptionCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_REF_CLOCK_SELECT_ENT_PTR_refClockSelectPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_ENT_PTR_clockGenerationModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TAI_CLOCK_MODE_ENT_PTR_clockModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_ENT_PTR_clockReceptionModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TAI_CLOCK_SELECT_ENT_PTR_clockSelectPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TAI_NUMBER_ENT_PTR_taiNumberPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_ENT_PTR_pclkRcvrClkModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_ENT_PTR_pClkSelModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT_PTR_pulseModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TAI_TOD_STEP_STC_PTR_todStepPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT_PTR_functionPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TOD_COUNT_STC_PTR_todValuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TSU_CONTROL_STC_PTR_controlPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TSU_FRAME_COUNTER_CFG_ENTRY_STC_PTR_frameCounterConfigPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TSU_TIMESTAMP_MASKING_PROFILE_STC_PTR_maskingProfilePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC_PTR_entryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TS_CFG_ENTRY_STC_PTR_entryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT_PTR_egrTimeCorrTaiSelModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TS_EGRESS_PIPE_DELAY_VALUES_STC_PTR_egrDelayValPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC_PTR_tsQueueEntryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC_PTR_tsQueueEntryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TS_LOCAL_ACTION_ENTRY_STC_PTR_entryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC_PTR_tsTagGlobalCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TS_TAG_PORT_CFG_STC_PTR_tsTagPortCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC_PTR_udpCsUpdModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_U_BIT_MODE_ENT_PTR_uBitModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_32_PTR_egrAsymmetryCorrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_32_PTR_fracNanoSecondPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_32_PTR_ingressDelayCorrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_32_PTR_ingressDelayPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_egrCorrFldPBEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_finishPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_ingrCorrFldPBEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_resetRxUnitPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_resetTxUnitPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_tsuBypassEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_tsuEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_TAI_SELECT_BMP_PTR_taiSelectBmpPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_delayPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_domainIdArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_domainIdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_edgePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_egrTsTaiNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_extPulseWidthPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_fracNanoSecondsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_gracefulStepPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_idMapBmpPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_nanoSecondsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_ntpTimeOffsetPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_queueEntryId0Ptr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_queueEntryId1Ptr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_queueSizePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_rxTaiSelectPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_secondsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_taiSelectPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_udpPortNumPtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssDxChPtpEtherTypeSet_E = (CPSS_LOG_LIB_PTP_E << 16) | (1 << 24),
    PRV_CPSS_LOG_FUNC_cpssDxChPtpEtherTypeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpUdpDestPortsSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpUdpDestPortsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpMessageTypeCmdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpMessageTypeCmdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpCpuCodeBaseSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpCpuCodeBaseGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTodCounterFunctionSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTodCounterFunctionGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTodCounterFunctionTriggerSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTodCounterFunctionTriggerStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiClockModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiClockModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiInternalClockGenerateEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiInternalClockGenerateEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiPClkDriftAdjustEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiPClkDriftAdjustEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiCaptureOverrideEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiCaptureOverrideEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiInputTriggersCountEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiInputTriggersCountEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiExternalPulseWidthSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiExternalPulseWidthGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiTodSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiTodGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiOutputTriggerEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiOutputTriggerEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiTodStepSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiTodStepGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiFractionalNanosecondDriftSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiFractionalNanosecondDriftGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiPClkCycleSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiPClkCycleGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiClockCycleSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiClockCycleGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiTodCaptureStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiTodUpdateCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiIncomingTriggerCounterSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiIncomingTriggerCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiIncomingClockCounterSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiIncomingClockCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiGracefulStepSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiGracefulStepGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpRxTaiSelectSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpRxTaiSelectGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsuBypassEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsuBypassEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsuEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsuEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsuControlSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsuControlGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsuTxTimestampQueueRead_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsuCountersClear_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsuPacketCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsuNtpTimeOffsetGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsTagGlobalCfgSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsTagGlobalCfgGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsTagPortCfgSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsTagPortCfgGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsDelayIngressPortDelaySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsDelayIngressPortDelayGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsDelayIngressLinkDelaySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsDelayIngressLinkDelayGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsDelayEgressPipeDelaySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsDelayEgressPipeDelayGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsDelayEgressAsymmetryCorrectionSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsDelayEgressAsymmetryCorrectionGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsDelayEgressTimestampTaiSelectSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsDelayEgressTimestampTaiSelectGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsCfgTableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsCfgTableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsLocalActionTableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsLocalActionTableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsUdpChecksumUpdateModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsUdpChecksumUpdateModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpOverEthernetEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpOverEthernetEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpOverUdpEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpOverUdpEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpDomainModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpDomainModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpDomainV1IdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpDomainV1IdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpDomainV2IdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpDomainV2IdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpEgressDomainTableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpEgressDomainTableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpIngressExceptionCfgSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpIngressExceptionCfgGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpIngressPacketCheckingModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpIngressPacketCheckingModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpIngressExceptionCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpEgressExceptionCfgSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpEgressExceptionCfgGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpEgressExceptionCountersGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsIngressTimestampQueueEntryRead_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsEgressTimestampQueueEntryRead_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsMessageTypeToQueueIdMapSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsMessageTypeToQueueIdMapGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsQueuesSizeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsQueuesSizeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsDebugQueuesEntryIdsClear_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsDebugQueuesEntryIdsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsNtpTimeOffsetSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsNtpTimeOffsetGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiPulseInModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiPulseInModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiPtpPulseInterfaceSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiPtpPulseInterfaceGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiPClkInterfaceSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiPClkInterfaceGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiPClkOutputInterfaceSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiPClkOutputInterfaceGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiInputClockSelectSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiInputClockSelectGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiTodCounterFunctionAllTriggerSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpPortUnitResetSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpPortUnitResetGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpPortTxPipeStatusDelaySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpPortTxPipeStatusDelayGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiCapturePreviousModeEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiCapturePreviousModeEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiPClkSelectionModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiPClkSelectionModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiClockGenerationModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiClockGenerationModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiClockReceptionModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiClockReceptionModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiClockGenerationEdgeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiClockGenerationEdgeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiClockReceptionEdgeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiClockReceptionEdgeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsuTimeStampMaskingProfileSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsuTimeStampMaskingProfileGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsuTSFrameCounterControlSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsuTSFrameCounterControlGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTsuTsFrameCounterIndexSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpUBitModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpUBitModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpPLLBypassEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpPLLBypassEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpRefClockSourceSelectSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpRefClockSourceSelectGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpNonPtpPacketTaiSelectSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpNonPtpPacketTaiSelectGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiOutputInterfaceSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiOutputInterfaceGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiInputInterfaceSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiInputInterfaceGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiTodCounterFunctionGroupTriggerSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiTodCounterFunctionGroupTriggerGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiSerRxFrameLatencyDbgGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiTodStepDbgSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPtpTaiSelectDbgSet_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssDxChPtpLogh */
