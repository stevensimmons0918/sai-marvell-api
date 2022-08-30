/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChIpLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssDxChIpLogh
#define __prvCpssDxChIpLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* manually implemented declarations *********/

void prvCpssLogParamFunc_CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_NAT_ENTRY_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_UC_ROUTE_ENTRY_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_IP_BRG_EXCP_CMD_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_IP_BRG_SERVICE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_IP_CNT_SET_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_IP_DROP_CNT_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_IP_EXCEPTION_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_IP_HEADER_ERROR_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_IP_MLL_PAIR_PART_LAST_BIT_WRITE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_IP_MT_TC_QUEUE_SCHED_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_IP_MT_UC_SCHED_MTU_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_IP_NEXT_HOP_MUX_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_IP_URPF_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_URPF_LOOSE_MODE_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_IP_NAT_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_IP_CPU_CODE_INDEX_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_IP_MULTICAST_INGRESS_VLAN_CHECK_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_IP_MULTICAST_RPF_FAIL_COMMAND_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_IP_PORT_TRUNK_CNT_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_IP_UC_ROUTE_ENTRY_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_IP_VLAN_CNT_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_IP_NAT66_MODIFY_COMMAND_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_IPV4_PREFIX_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IPV6_PREFIX_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_COUNTER_SET_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_ECMP_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_LTT_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_MLL_PAIR_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_MLL_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_NAT44_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_NAT66_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_PORT_TRUNK_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_UC_ECMP_RPF_FORMAT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_UC_ROUTE_ENTRY_FORMAT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_IPV4_PREFIX_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IPV6_PREFIX_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_BRG_EXCP_CMD_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_BRG_SERVICE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_CNT_SET_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_CNT_SET_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_COUNTER_SET_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_DROP_CNT_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_DROP_CNT_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_ECMP_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_EXCEPTION_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_HEADER_ERROR_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_LTT_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_MLL_PAIR_PART_LAST_BIT_WRITE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_MLL_PAIR_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_MT_TC_QUEUE_SCHED_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_MT_TC_QUEUE_SCHED_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_MT_UC_SCHED_MTU_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_MT_UC_SCHED_MTU_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_NAT_ENTRY_UNT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_NEXT_HOP_MUX_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_NEXT_HOP_MUX_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_URPF_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_URPF_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_URPF_LOOSE_MODE_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_URPF_LOOSE_MODE_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_IP_NAT_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_IP_NAT_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC_PTR_routeEntriesArray;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPV4_PREFIX_STC_PTR_maskPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPV4_PREFIX_STC_PTR_prefixPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPV6_PREFIX_STC_PTR_maskPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPV6_PREFIX_STC_PTR_prefixPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_BRG_EXCP_CMD_ENT_bridgeExceptionCmd;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT_enableDisableMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_BRG_SERVICE_ENT_bridgeService;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_CNT_SET_MODE_ENT_cntSetMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC_PTR_interfaceCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC_PTR_interfaceModeCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_COUNTER_SET_STC_PTR_countersPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_DROP_CNT_MODE_ENT_dropCntMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_ECMP_ENTRY_STC_PTR_ecmpEntryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_EXCEPTION_TYPE_ENT_exceptionType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_HEADER_ERROR_ENT_ipHeaderErrorType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_LTT_ENTRY_STC_PTR_lttEntryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC_PTR_routeEntryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_MLL_PAIR_PART_LAST_BIT_WRITE_ENT_mllEntryPart;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_MLL_PAIR_STC_PTR_mllPairEntryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_MT_TC_QUEUE_SCHED_MODE_ENT_schedulingMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_MT_UC_SCHED_MTU_ENT_schedMtu;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_NAT_ENTRY_UNT_PTR_entryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_NEXT_HOP_MUX_MODE_ENT_muxMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC_PTR_routeEntriesArray;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_URPF_MODE_ENT_uRpfMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_ENT_tokenBucketIntervalUpdateRatio;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_URPF_LOOSE_MODE_TYPE_ENT_looseModeType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_IPV6_MLL_SELECTION_RULE_ENT_mllSelectionRule;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_IPV6_PREFIX_SCOPE_ENT_addressScope;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_IPV6_PREFIX_SCOPE_ENT_addressScopeDest;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_IPV6_PREFIX_SCOPE_ENT_addressScopeSrc;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_IP_CNT_SET_ENT_cntSet;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_IP_NAT_TYPE_ENT_natType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_IP_UNICAST_MULTICAST_ENT_ucMcEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_IP_UNICAST_MULTICAST_ENT_ucMcSet;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_MAC_SA_LSB_MODE_ENT_saLsbMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PACKET_CMD_ENT_arpBcMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PACKET_CMD_ENT_scopeCommand;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_TX_DROP_SHAPER_MODE_ENT_tokenBucketMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_borderCrossed;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_enableRouterTrigger;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_enableRouting;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_enableService;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_lastBit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_mask;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_mllBridgeEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_multiTargetRateShaperEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_refreshEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_ucSPEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_ETHERADDR_PTR_arpMacAddrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_ETHERADDR_PTR_macSaAddrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_IPV6ADDR_PTR_prefixPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_HEX_sourceIdMask;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_baseRouteEntryIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ctrlMultiTargetTCQueue;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_dropPkts;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ecmpEntryIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_failRpfMultiTargetTCQueue;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_indirectIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_lttTtiColumn;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_lttTtiRow;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_maxBucketSize;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_mcWeight;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_mllOutMCPkts;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_mtu;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_mtuProfileIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_multiTargetMcQueue;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_multiTargetTCQueue;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_multiTargetTcQueue;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_nextHopIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numOfRouteEntries;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_prefixScopeIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_qosProfile;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_queuePriority;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_queueWeight;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_routeEntryIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_routeEntryOffset;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_routerBridgedExceptionPkts;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_routerMacSaIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_routerNextHopTableAgeBitsEntry;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_routerNextHopTableAgeBitsEntryIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_routerTtiTcamColumn;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_routerTtiTcamRow;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_saMac;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_seed;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_tokenBucketIntervalSlowUpdateRatio;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_tokenBucketUpdateInterval;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ucWeight;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U64_PTR_mrstBmpPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_saMac;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IPV4_PREFIX_STC_PTR_maskPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IPV4_PREFIX_STC_PTR_prefixPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IPV6_PREFIX_STC_PTR_maskPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IPV6_PREFIX_STC_PTR_prefixPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_CNT_SET_MODE_ENT_PTR_cntSetModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC_PTR_interfaceModeCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_COUNTER_SET_STC_PTR_countersPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_DROP_CNT_MODE_ENT_PTR_dropCntModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_ECMP_ENTRY_STC_PTR_ecmpEntryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_LTT_ENTRY_STC_PTR_lttEntryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC_PTR_routeEntryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_MLL_PAIR_STC_PTR_mllPairEntryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_MT_TC_QUEUE_SCHED_MODE_ENT_PTR_schedulingModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_MT_UC_SCHED_MTU_ENT_PTR_schedMtuPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_NAT_ENTRY_UNT_PTR_entryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_NEXT_HOP_MUX_MODE_ENT_PTR_muxModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_URPF_MODE_ENT_PTR_uRpfModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_ENT_PTR_tokenBucketIntervalUpdateRatioPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_URPF_LOOSE_MODE_TYPE_ENT_PTR_looseModeTypePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_IPV6_MLL_SELECTION_RULE_ENT_PTR_mllSelectionRulePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_IPV6_PREFIX_SCOPE_ENT_PTR_addressScopePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_IP_NAT_TYPE_ENT_PTR_natTypePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_MAC_SA_LSB_MODE_ENT_PTR_saLsbModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PACKET_CMD_ENT_PTR_arpBcModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PACKET_CMD_ENT_PTR_exceptionCmdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PACKET_CMD_ENT_PTR_scopeCommandPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_TX_DROP_SHAPER_MODE_ENT_PTR_tokenBucketModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_enableRouterTriggerPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_enableRoutingPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_enableServicePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_mllBridgeEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_multiTargetRateShaperEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_refreshEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_ucSPEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_ETHERADDR_PTR_arpMacAddrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_ETHERADDR_PTR_macSaAddrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_IPV6ADDR_PTR_prefixPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_HEX_PTR_sourceIdMaskPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_ctrlMultiTargetTCQueuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_dropPktsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_failRpfMultiTargetTCQueuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_maxBucketSizePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_mcWeightPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_mtuPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_multiTargetTCQueuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_natDropPktsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_nextHopIndexPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_queuePriorityPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_queueWeightPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_routeEntryOffsetPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_routerBridgedExceptionPktsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_routerMacSaIndexPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_routerNextHopTableAgeBitsEntryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_saMacPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_seedPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_skipCounterPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tokenBucketIntervalSlowUpdateRatioPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tokenBucketUpdateIntervalPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_ucWeightPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U64_PTR_mrstBmpPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_saMacPtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssDxChIpv4PrefixSet_E = (CPSS_LOG_LIB_IP_E << 16) | (1 << 24),
    PRV_CPSS_LOG_FUNC_cpssDxChIpv4PrefixGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpv4PrefixInvalidate_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpv6PrefixSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpv6PrefixGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpv6PrefixInvalidate_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLttWrite_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLttRead_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpUcRouteEntriesWrite_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpUcRouteEntriesRead_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMcRouteEntriesWrite_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMcRouteEntriesRead_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpRouterNextHopTableAgeBitsEntryWrite_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpRouterNextHopTableAgeBitsEntryRead_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMLLPairWrite_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMLLPairRead_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMLLLastBitWrite_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpRouterArpAddrWrite_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpRouterArpAddrRead_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpEcmpEntryWrite_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpEcmpEntryRead_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpEcmpHashNumBitsSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpEcmpHashNumBitsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpEcmpHashSeedValueSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpEcmpHashSeedValueGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpEcmpIndirectNextHopEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpEcmpIndirectNextHopEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpSpecialRouterTriggerEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpSpecialRouterTriggerEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpExceptionCommandSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpExceptionCommandGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpHeaderErrorMaskSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpHeaderErrorMaskGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpUcRouteAgingModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpUcRouteAgingModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpRouterSourceIdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpRouterSourceIdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpPortGroupRouterSourceIdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpPortGroupRouterSourceIdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpRouterSourceIdOverrideEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpRouterSourceIdOverrideEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpFailRpfCtrlTrafficMultiTargetTCQueueGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpQosProfileToMultiTargetTCQueueMapSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpQosProfileToMultiTargetTCQueueMapGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpTcDpToMultiTargetTcQueueMapSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpTcDpToMultiTargetTcQueueMapGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMultiTargetQueueFullDropCntGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMultiTargetQueueFullDropCntSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMultiTargetQueuePerQueueFullDropCntGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMultiTargetTCQueueSchedModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMultiTargetTCQueueSchedModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpBridgeServiceEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpBridgeServiceEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpPortGroupRouterBridgedPacketsExceptionCntGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpRouterBridgedPacketsExceptionCntGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpRouterBridgedPacketsExceptionCntSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMllBridgeEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMllBridgeEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMultiTargetRateShaperSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMultiTargetRateShaperGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMultiTargetUcSchedModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMultiTargetUcSchedModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpArpBcModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpArpBcModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpPortRoutingEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpPortRoutingEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpPortFcoeForwardingEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpPortFcoeForwardingEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpQosProfileToRouteEntryMapSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpQosProfileToRouteEntryMapGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpRoutingEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpRoutingEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpCntGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpCntSetModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpCntSetModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpCntSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMllSkippedEntriesCountersGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMllPortGroupSkippedEntriesCountersGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpSetMllCntInterface_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMllCntGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMllCntSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMllSilentDropCntGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMllPortGroupSilentDropCntGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpDropCntSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpSetDropCntMode_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpGetDropCntMode_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpDropCntGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMtuProfileSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMtuProfileGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpv6AddrPrefixScopeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpv6AddrPrefixScopeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpv6UcScopeCommandSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpv6UcScopeCommandGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpv6McScopeCommandSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpv6McScopeCommandGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpRouterMacSaBaseSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpRouterMacSaBaseGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpPortRouterMacSaLsbModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpPortRouterMacSaLsbModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpRouterPortMacSaLsbSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpRouterPortMacSaLsbGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpRouterVlanMacSaLsbSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpRouterVlanMacSaLsbGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpRouterGlobalMacSaSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpRouterGlobalMacSaGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpRouterPortGlobalMacSaIndexSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpRouterPortGlobalMacSaIndexGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpRouterMacSaModifyEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpRouterMacSaModifyEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpEcmpUcRpfCheckEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpEcmpUcRpfCheckEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpPortGroupCntSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpPortGroupCntGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpPortGroupDropCntSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpPortGroupDropCntGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpPortGroupMultiTargetQueueFullDropCntGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpPortGroupMultiTargetQueueFullDropCntSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpPortGroupMultiTargetQueuePerQueueFullDropCntGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpPortGroupMllCntGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpPortGroupMllCntSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpUcRpfModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpUcRpfModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpUrpfLooseModeTypeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpUrpfLooseModeTypeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpPortSipSaEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpPortSipSaEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpPortGroupMultiTargetRateShaperSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpPortGroupMultiTargetRateShaperGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpUcRoutingVid1AssignEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpUcRoutingVid1AssignEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMllMultiTargetShaperBaselineSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMllMultiTargetShaperBaselineGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMllMultiTargetShaperMtuSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMllMultiTargetShaperMtuGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMllMultiTargetShaperTokenBucketModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMllMultiTargetShaperTokenBucketModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMllMultiTargetShaperEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMllMultiTargetShaperEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMllMultiTargetShaperConfigurationSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMllMultiTargetShaperConfigurationGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMllMultiTargetShaperIntervalConfigurationSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpMllMultiTargetShaperIntervalConfigurationGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpPbrBypassRouterTriggerRequirementsEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpPbrBypassRouterTriggerRequirementsEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpVlanMrstBitmapSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpVlanMrstBitmapGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpFcoeExceptionPacketCommandSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpFcoeExceptionPacketCommandGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpFcoeExceptionCpuCodeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpFcoeExceptionCpuCodeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpFcoeSoftDropRouterEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpFcoeSoftDropRouterEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpFcoeTrapRouterEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpFcoeTrapRouterEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpFcoeBridgedUrpfCheckEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpFcoeBridgedUrpfCheckEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpFcoeBridgedUrpfCheckCommandSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpFcoeBridgedUrpfCheckCommandGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpFcoeUcRpfAccessLevelSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpFcoeUcRpfAccessLevelGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpFcoeBridgedSidSaMismatchCheckEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpFcoeBridgedSidSaMismatchCheckEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpFcoeBridgedSidSaMismatchCommandSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpFcoeBridgedSidSaMismatchCommandGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpFcoeBridgedSidFilterEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpFcoeBridgedSidFilterEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpFcoeBridgedHeaderErrorCheckEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpFcoeBridgedHeaderErrorCheckEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpFdbRoutePrefixLenSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpFdbRoutePrefixLenGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpFdbUnicastRouteForPbrEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpFdbUnicastRouteForPbrEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpFdbMulticastRouteForPbrEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpFdbMulticastRouteForPbrEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpNhMuxModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpNhMuxModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpNatEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpNatEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpNatDroppedPacketsCntGet_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssDxChIpLogh */
