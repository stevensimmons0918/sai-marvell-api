/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChBrgLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssDxChBrgLogh
#define __prvCpssDxChBrgLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_BRG_STP_STATE_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_ARP_BCAST_CMD_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_BRG_FDB_AGE_TRUNK_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_BRG_FDB_DEL_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_BRG_FDB_STREAM_ID_ASSIGNMENT_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_BRG_GEN_BYPASS_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_BRG_L2_DLB_SCALE_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_BRG_L2_ECMP_MLL_REPLICATION_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_BRG_NEST_VLAN_TARGET_USE_INGRESS_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_MAP_DECLARE_MAC(CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_BRG_VLAN_PACKET_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_CMD_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_BRG_VLAN_TAG_TPID_SELECT_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_BRG_VLAN_TRANSLATION_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_BRIDGE_DROP_CNTR_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_FDB_IPMC_ENTRY_MUXING_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_FDB_QUEUE_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_FDB_SA_LOOKUP_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_BRG_FDB_STREAM_ID_EPG_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_BRG_FDB_TBL_SIZE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_EPG_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_HASH_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_GEN_RATE_LIMIT_PORT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_GEN_RATE_LIMIT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_L2_DLB_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_L2_DLB_PATH_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_L2_ECMP_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_QOS_PROT_CLASS_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_SRC_ID_TARGET_INDEX_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_VLAN_8B_TAG_PARSE_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_VLAN_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_VLAN_PROT_CLASS_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_BRIDGE_HOST_CNTR_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_BRG_STP_STATE_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_BRG_STP_STATE_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_ARP_BCAST_CMD_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_FDB_AGE_TRUNK_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_FDB_AGE_TRUNK_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_FDB_DEL_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_FDB_DEL_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_FDB_EPG_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_FDB_HASH_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_FDB_STREAM_ID_ASSIGNMENT_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_FDB_STREAM_ID_ASSIGNMENT_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_GEN_BYPASS_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_GEN_BYPASS_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_GEN_RATE_LIMIT_PORT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_GEN_RATE_LIMIT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_L2_DLB_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_L2_DLB_PATH_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_L2_DLB_SCALE_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_L2_DLB_SCALE_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_L2_ECMP_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_L2_ECMP_MLL_REPLICATION_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_L2_ECMP_MLL_REPLICATION_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_NEST_VLAN_TARGET_USE_INGRESS_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_NEST_VLAN_TARGET_USE_INGRESS_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_QOS_PROT_CLASS_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_SRC_ID_TARGET_INDEX_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_VLAN_8B_TAG_PARSE_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_VLAN_INFO_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_VLAN_PACKET_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_CMD_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_VLAN_PROT_CLASS_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_VLAN_TAG_TPID_SELECT_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_VLAN_TAG_TPID_SELECT_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_VLAN_TRANSLATION_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_VLAN_TRANSLATION_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRIDGE_DROP_CNTR_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRIDGE_DROP_CNTR_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_BRIDGE_HOST_CNTR_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_FDB_IPMC_ENTRY_MUXING_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_FDB_IPMC_ENTRY_MUXING_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_FDB_QUEUE_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_FDB_SA_LOOKUP_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_FDB_SA_LOOKUP_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_crcHashPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_entryOffsetPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_numOfAuPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_numOfFuPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_xorHashPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_BRG_IPM_MODE_ENT_ipmMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_BRG_MODE_ENT_brgMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_BRG_SECUR_BREACH_DROP_COUNT_CFG_STC_PTR_cntrCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_BRG_SECUR_BREACH_EVENTS_ENT_eventType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_BRG_SRC_ID_ASSIGN_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_BRG_STP_STATE_MODE_ENT_stateMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_BRG_TPID_SIZE_TYPE_ENT_type;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_BRIDGE_INGR_CNTR_MODES_ENT_setMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DROP_MODE_TYPE_ENT_dropMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DROP_MODE_TYPE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_ARP_BCAST_CMD_MODE_ENT_cmdMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC_PTR_egressInfoPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT_portLinkStatusState;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_ENT_fltTabAccessMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_ENT_vidSelectMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_FDB_AGE_TRUNK_MODE_ENT_trunkAgingMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_FDB_DEL_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_FDB_EPG_CONFIG_STC_PTR_epgConfigPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_FDB_HASH_PARAMS_STC_PTR_hashParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_FDB_STREAM_ID_ASSIGNMENT_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_GEN_BYPASS_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT_byteCountMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_GEN_RATE_LIMIT_PORT_STC_PTR_portGfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_GEN_RATE_LIMIT_STC_PTR_brgRateLimitPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT_ipCntrlType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_L2_DLB_ENTRY_STC_PTR_dlbEntryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_L2_DLB_PATH_ENTRY_STC_PTR_dlbEntryArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_L2_DLB_SCALE_MODE_ENT_scaleMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_L2_ECMP_ENTRY_STC_PTR_ecmpEntryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC_PTR_ecmpLttInfoPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_L2_ECMP_MLL_REPLICATION_MODE_ENT_replicationMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_NEST_VLAN_TARGET_USE_INGRESS_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_QOS_PROT_CLASS_CFG_STC_PTR_qosCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_SRC_ID_TARGET_INDEX_CONFIG_STC_PTR_configPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_VLAN_8B_TAG_PARSE_CONFIG_STC_PTR_parseConfigPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT_stateMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ENT_ethMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ENT_floodVidxMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_VLAN_INFO_STC_PTR_vlanInfoPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_ENT_trafficType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_VLAN_PACKET_TYPE_ENT_packetType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC_PTR_portsTaggingCmdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_CMD_ENT_cmd;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT_portTaggingCmd;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT_tagCmd;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT_taggingCmd;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_VLAN_PROT_CLASS_CFG_STC_PTR_vlanCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_ENT_vlanCmd;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_VLAN_TAG_TPID_SELECT_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_VLAN_TRANSLATION_ENT_enable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_ENT_unregIpmEVidxMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRIDGE_CNTR_SET_ID_ENT_cntrSetId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRIDGE_DROP_CNTR_MODE_ENT_dropMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_ETHERTYPE_TABLE_ENT_tableType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_FDB_IPMC_ENTRY_MUXING_MODE_ENT_muxingMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_ENT_muxingMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_FDB_QUEUE_TYPE_ENT_queueType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_FDB_SA_LOOKUP_MODE_ENT_saLookupMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_HASH_BIT_SELECTION_PROFILE_STC_PTR_hashBitSelectionProfileInfoPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_ETHER_MODE_ENT_ethMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_FDB_ACTION_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_IGMP_SNOOP_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_INTERFACE_INFO_STC_PTR_newInterfacePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_INTERFACE_INFO_STC_PTR_oldInterfacePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_INTERFACE_INFO_STC_PTR_physicalInfoPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_IP_SITE_ID_ENT_siteId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_MAC_ACTION_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_MAC_ENTRY_EXT_KEY_STC_PTR_entryKeyPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_MAC_ENTRY_EXT_KEY_STC_PTR_macEntryKeyPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_MAC_ENTRY_EXT_STC_PTR_macEntryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_MAC_HASH_FUNC_MODE_ENT_hashMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_MAC_HASH_FUNC_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_MAC_SECURE_AUTO_LEARN_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_MAC_TABLE_CMD_ENT_daDropCmdMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_MAC_TABLE_CMD_ENT_saDropCmdMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_MAC_VL_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_NET_RX_CPU_CODE_ENT_exceptionCpuCode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_NET_RX_CPU_CODE_ENT_replicationDropCode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT_precedence;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PACKET_CMD_ENT_exceptionCommand;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PACKET_CMD_ENT_movedMacSaCmd;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PACKET_CMD_ENT_nhPacketCmd;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PACKET_CMD_ENT_pktCommand;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORTS_BMP_STC_PTR_portBitmapPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORTS_BMP_STC_PTR_portsTaggingPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_ACCEPT_FRAME_TYPE_ENT_frameType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_LOCK_CMD_ENT_cmd;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_SPEED_ENT_speedGranularity;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PROT_CLASS_ENCAP_STC_PTR_encListPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_STP_STATE_ENT_state;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_age;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_ageOutAllDevOnNonTrunkEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_ageOutAllDevOnTrunkEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_cpuSrcIdMember;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_deleteStatic;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_dstTrunk;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_eCidOffsetNegative;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_forceSrcCidDefault;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_ignoreFilter;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_incOrDec;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_isDefaultProfile;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_isPortExtender;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_isTagged;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_mcFilterEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_mtuCheckEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_skip;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_ENT_numBytesToPop;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_ETHERADDR_PTR_daAddrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_ETHERADDR_PTR_saAddrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_HW_DEV_NUM_dstHwDev;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_IPV6ADDR_PTR_addressPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_IPV6ADDR_PTR_maskPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_PHYSICAL_PORT_NUM_cascadePortNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_PORT_NUM_dlbIndexBaseEport;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_PORT_NUM_dstPort;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_PORT_NUM_ecmpIndexBaseEport;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U16_floodVidx;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U16_pushTagVidValue;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U16_stpId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U16_tagValue;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U16_transVlanId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U16_udpPortNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U16_vid1;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U16_vidRange;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U16_vlanMask;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_HEX_PTR_stpEntryWordArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_HEX_actDevMask;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_HEX_actIsTrunkMask;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_HEX_actTrunkPortMask;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_HEX_actUerDefinedMask;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_HEX_tpidBmp;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_actDev;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_actIsTrunk;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_actTrunkPort;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_actUerDefined;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_bankIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_bitLocation;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_dBitInSrcId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_defaultSrcECid;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_defaultSrcId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_devTableBmp;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_dropCnt;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_eCidExtValue;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_eCidOffset;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_entryNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_fidValue;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ingressECidExtValue;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_lBitInSrcId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_lookupLen;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_lttIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_meshId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_minimumIpg;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_mruIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_mruValue;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_msgRate;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_mtuProfileIdx;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_mtuSize;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_multiHashStartBankIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numBitsInHash;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_pcid;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_reValue;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_secondaryDlbBaseIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_srcPortHash;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_trgIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_txAnalyzerIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_unregIpmEVidx;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_up0;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_up1;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_vid;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_vidIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_vlanQueueOffset;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_vrfId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U64_value;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_PTR_dipBytesSelectMapArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_PTR_sipBytesSelectMapArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_msgType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_BRG_MODE_ENT_PTR_brgModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_BRG_SECUR_BREACH_DROP_COUNT_CFG_STC_PTR_cntrCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_BRG_SECUR_BREACH_MSG_STC_PTR_sbMsgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_BRG_SRC_ID_ASSIGN_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_BRG_STP_STATE_MODE_ENT_PTR_stateModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_BRG_TPID_SIZE_TYPE_ENT_PTR_typePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_BRIDGE_INGRESS_CNTR_STC_PTR_ingressCntrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_BRIDGE_INGR_CNTR_MODES_ENT_PTR_setModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DROP_MODE_TYPE_ENT_PTR_dropModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DROP_MODE_TYPE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC_PTR_egressInfoPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT_PTR_portLinkStatusStatePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_ACCESS_MODE_ENT_PTR_fltTabAccessModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_EGR_FLT_VLAN_PORT_VID_SELECT_MODE_ENT_PTR_vidSelectModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_FDB_AGE_TRUNK_MODE_ENT_PTR_trunkAgingModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_FDB_CRC_HASH_UPPER_BITS_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_FDB_DEL_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_FDB_EPG_CONFIG_STC_PTR_epgConfigPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_FDB_STREAM_ID_ASSIGNMENT_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_GEN_BYPASS_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_GEN_BYTE_COUNT_MODE_MTU_ENT_PTR_byteCountModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_GEN_IPM_CLASSIFICATION_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_GEN_RATE_LIMIT_PORT_STC_PTR_portGfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_GEN_RATE_LIMIT_STC_PTR_brgRateLimitPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT_PTR_ipCntrlTypePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_L2_DLB_ENTRY_STC_PTR_dlbEntryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_L2_DLB_PATH_ENTRY_STC_PTR_dlbEntryArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_L2_DLB_SCALE_MODE_ENT_PTR_scaleModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_L2_ECMP_ENTRY_STC_PTR_ecmpEntryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC_PTR_ecmpLttInfoPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_L2_ECMP_MLL_REPLICATION_MODE_ENT_PTR_replicationModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_NEST_VLAN_TARGET_USE_INGRESS_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_QOS_PROT_CLASS_CFG_STC_PTR_qosCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_SRC_ID_TARGET_INDEX_CONFIG_STC_PTR_configPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_VLAN_8B_TAG_PARSE_CONFIG_STC_PTR_parseConfigPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_MODE_ENT_PTR_stateModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_VLAN_INFO_STC_PTR_vlanInfoPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC_PTR_portsTaggingCmdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT_PTR_tagCmdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_VLAN_PROT_CLASS_CFG_STC_PTR_vlanCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_ENT_PTR_vlanCmdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_VLAN_TAG_TPID_SELECT_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_VLAN_TBL_INDEX_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_VLAN_TRANSLATION_ENT_PTR_enablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRIDGE_DROP_CNTR_MODE_ENT_PTR_dropModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRIDGE_HOST_CNTR_STC_PTR_hostGroupCntPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_FDB_IPMC_ENTRY_MUXING_MODE_ENT_PTR_muxingModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_FDB_MAC_ENTRY_MUXING_MODE_ENT_PTR_muxingModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_FDB_SA_LOOKUP_MODE_ENT_PTR_saLookupModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_HASH_BIT_SELECTION_PROFILE_STC_PTR_hashBitSelectionProfileInfoPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_FDB_ACTION_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_IGMP_SNOOP_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_INTERFACE_INFO_STC_PTR_newInterfacePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_INTERFACE_INFO_STC_PTR_oldInterfacePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_INTERFACE_INFO_STC_PTR_physicalInfoPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_MAC_ACTION_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_MAC_ENTRY_EXT_STC_PTR_entryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_MAC_HASH_FUNC_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_MAC_SECURE_AUTO_LEARN_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_MAC_TABLE_CMD_ENT_PTR_daDropCmdModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_MAC_TABLE_CMD_ENT_PTR_saDropCmdModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_MAC_UPDATE_MSG_EXT_STC_PTR_auMessagesPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_MAC_UPDATE_MSG_EXT_STC_PTR_fuMessagesPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_MAC_VL_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_NET_RX_CPU_CODE_ENT_PTR_exceptionCpuCodePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_NET_RX_CPU_CODE_ENT_PTR_replicationDropCodePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT_PTR_precedencePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PACKET_CMD_ENT_PTR_exceptionCommandPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PACKET_CMD_ENT_PTR_movedMacSaCmdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PACKET_CMD_ENT_PTR_nhPacketCmdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PACKET_CMD_ENT_PTR_pktCommandPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORTS_BMP_STC_PTR_portBitmapPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORTS_BMP_STC_PTR_portsTaggingPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_ACCEPT_FRAME_TYPE_ENT_PTR_frameTypePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_LOCK_CMD_ENT_PTR_cmdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_SPEED_ENT_PTR_speedGranularityPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PROT_CLASS_ENCAP_STC_PTR_encListPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_STP_STATE_ENT_PTR_statePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_ageOutAllDevOnNonTrunkEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_ageOutAllDevOnTrunkEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_agedPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_completedPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_cpuSrcIdMemberPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_deleteStaticPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_dstTrunkPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_eCidOffsetNegativePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_endOfQueueReachedPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_forceSrcCidDefaultPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_ignoreFilterPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_isFinishedPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_isFullPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_isPortExtenderPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_mcFilterEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_mtuCheckEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_rewindPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_skipPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_succeededPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_validEntryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_DXCH_BRG_VLAN_NUM_BYTES_TO_POP_ENT_PTR_numBytesToPopPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_ETHERADDR_PTR_daAddrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_ETHERADDR_PTR_saAddrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_HW_DEV_NUM_PTR_associatedHwDevNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_HW_DEV_NUM_PTR_dstHwDevPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_IPV6ADDR_PTR_addressPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_IPV6ADDR_PTR_maskPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_PORT_GROUPS_BMP_PTR_actFinishedPortGroupsBmpPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_PORT_GROUPS_BMP_PTR_isFullPortGroupsBmpPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_PORT_NUM_PTR_dlbIndexBaseEportPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_PORT_NUM_PTR_dstPortPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_PORT_NUM_PTR_ecmpIndexBaseEportPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_PORT_NUM_PTR_portPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U16_PTR_pushTagVidValuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U16_PTR_stpIdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U16_PTR_tagValuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U16_PTR_transVlanIdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U16_PTR_udpPortNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U16_PTR_vid1Ptr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U16_PTR_vidRangePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U16_PTR_vlanMaskPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_HEX_PTR_actDevMaskPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_HEX_PTR_actIsTrunkMaskPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_HEX_PTR_actTrunkPortMaskPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_HEX_PTR_actUerDefinedMaskPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_HEX_PTR_completedPortGroupsBmpPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_HEX_PTR_stpEntryWordArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_HEX_PTR_succeededPortGroupsBmpPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_HEX_PTR_tpidBmpPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_actDevPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_actIsTrunkPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_actTrunkPortPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_actUerDefinedPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_bitLocationPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_counValuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_dBitInSrcIdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_defaultSrcECidPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_defaultSrcIdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_devTableBmpPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_dropCntPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_eCidExtValuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_eCidOffsetPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_fidValuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_hashPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_ingressECidExtValuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_lBitInSrcIdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_lookupLenPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_matrixCntSaDaPktsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_meshIdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_minimumIpgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_mruValuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_msgRatePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_mtuProfileIdxPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_mtuSizePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_numBitsInHashPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_numOfAuPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_reValuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_srcPortHashPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_timeoutPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_trgIndexPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_up0Ptr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_up1Ptr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_vidIndexPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_vlanQueueOffsetPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U64_PTR_valuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_dipBytesSelectMapArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_msgTypePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_sipBytesSelectMapArr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssDxChBrgCntDropCntrModeSet_E = (CPSS_LOG_LIB_BRIDGE_E << 16) | (1 << 24),
    PRV_CPSS_LOG_FUNC_cpssDxChBrgCntDropCntrModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgCntMacDaSaSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgCntMacDaSaGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgCntBridgeIngressCntrModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgCntBridgeIngressCntrModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgCntDropCntrGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgCntDropCntrSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgCntHostGroupCntrsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgCntMatrixGroupCntrsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgCntBridgeIngressCntrsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgCntLearnedEntryDiscGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgCntPortGroupDropCntrGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgCntPortGroupDropCntrSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgCntPortGroupHostGroupCntrsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgCntPortGroupMatrixGroupCntrsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgCntPortGroupBridgeIngressCntrsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgCntPortGroupLearnedEntryDiscGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgEportToPhysicalPortTargetMappingTableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgEportToPhysicalPortTargetMappingTableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgEportToPhysicalPortEgressPortInfoSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgEportToPhysicalPortEgressPortInfoGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgEportAssignEgressAttributesLocallySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgEportAssignEgressAttributesLocallyGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgPortEgrFltUnkEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgPortEgrFltUnkEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgPortEgrFltUregMcastEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgPortEgrFltUregMcastEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanEgressFilteringEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanEgressFilteringEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgRoutedUnicastEgressFilteringEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgRoutedUnicastEgressFilteringEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgRoutedSpanEgressFilteringEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgRoutedSpanEgressFilteringEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgPortEgrFltUregBcEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgPortEgrFltUregBcEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgPortEgressMcastLocalEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgPortEgressMcastLocalEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgPortEgrFltIpMcRoutedEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgPortEgrFltIpMcRoutedEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgEgrFltVlanPortFilteringEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgEgrFltVlanPortFilteringEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgEgrFltVlanPortVidSelectModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgEgrFltVlanPortVidSelectModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgEgrFltVlanPortVidMappingSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgEgrFltVlanPortVidMappingGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgEgrFltVlanPortAccessModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgEgrFltVlanPortAccessModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgEgrFltVlanPortMemberSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgEgrFltVlanPortMemberGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgEgrMeshIdConfigurationSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgEgrMeshIdConfigurationGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgEgrPortMeshIdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgEgrPortMeshIdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgEgrFltPortVid1FilteringEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgEgrFltPortVid1FilteringEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgEgrFltPortVid1Set_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgEgrFltPortVid1Get_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgEgrFltPortLinkEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgEgrFltPortLinkEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgEgrFltVlanEPortFilteringEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgEgrFltVlanEPortFilteringEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgEgrFltVlanQueueOffsetSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgEgrFltVlanQueueOffsetGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbPortLearnStatusSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbPortLearnStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbNaToCpuPerPortSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbNaToCpuPerPortGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbNaStormPreventSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbNaStormPreventGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbPortVid1LearningEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbPortVid1LearningEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbDeviceTableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbDeviceTableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbPortGroupMacEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbMacEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbPortGroupQaSend_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbQaSend_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbPortGroupMacEntryDelete_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbMacEntryDelete_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbPortGroupMacEntryWrite_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbMacEntryWrite_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbPortGroupMacEntryStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbMacEntryStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbPortGroupMacEntryRead_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbMacEntryRead_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbPortGroupMacEntryInvalidate_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbMacEntryInvalidate_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbMaxLookupLenSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbMaxLookupLenGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbMacVlanLookupModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbMacVlanLookupModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbAuMsgRateLimitSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbAuMsgRateLimitGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbNaMsgOnChainTooLongSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbNaMsgOnChainTooLongGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbSpAaMsgToCpuSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbSpAaMsgToCpuGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbAAandTAToCpuSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbAAandTAToCpuGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbHashModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbHashModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbAgingTimeoutSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbAgingTimeoutGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbSecureAgingSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbSecureAgingGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbSecureAutoLearnSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbSecureAutoLearnGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbStaticTransEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbStaticTransEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbStaticDelEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbStaticDelEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbActionsEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbActionsEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbTrigActionStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbMacTriggerModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbMacTriggerModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbStaticOfNonExistDevRemove_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbStaticOfNonExistDevRemoveGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbDropAuEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbDropAuEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbAgeOutAllDevOnTrunkEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbAgeOutAllDevOnTrunkEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbAgeOutAllDevOnNonTrunkEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbAgeOutAllDevOnNonTrunkEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbActionTransplantDataSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbActionTransplantDataGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbFromCpuAuMsgStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbActionActiveVlanSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbActionActiveVlanGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbActionActiveDevSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbActionActiveDevGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbActionActiveInterfaceSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbActionActiveInterfaceGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbIpmcAddrDelEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbIpmcAddrDelEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbUploadEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbUploadEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbTrigActionStart_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbActionModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbActionModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbMacTriggerToggle_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbTrunkAgingModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbTrunkAgingModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbInit_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbAgeBitDaRefreshEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbAgeBitDaRefreshEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbRoutedLearningEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbRoutedLearningEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbTriggerAuQueueWa_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbQueueFullGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbQueueRewindStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbMessagesQueueManagerInfoGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbPortGroupMacEntryAgeBitSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbPortGroupTrigActionStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbPortGroupQueueFullGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbMacEntryAgeBitSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbNaMsgVid1EnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbNaMsgVid1EnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbPortGroupFromCpuAuMsgStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdb16BitFidHashEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdb16BitFidHashEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbMaxLengthSrcIdEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbMaxLengthSrcIdEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbVid1AssignmentEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbVid1AssignmentEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbSaLookupAnalyzerIndexSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbSaLookupAnalyzerIndexGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbDaLookupAnalyzerIndexSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbDaLookupAnalyzerIndexGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbActionActiveUserDefinedSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbActionActiveUserDefinedGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbLearnPrioritySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbLearnPriorityGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbUserGroupSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbUserGroupGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbBankCounterValueGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbBankCounterUpdate_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbBankCounterUpdateStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbMacEntryMuxingModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbMacEntryMuxingModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbIpmcEntryMuxingModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbIpmcEntryMuxingModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbSaLookupSkipModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbSaLookupSkipModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbSaDaCmdDropModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbSaDaCmdDropModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbPortMovedMacSaCommandSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbPortMovedMacSaCommandGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbEpgConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbEpgConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbStreamIdAssignmentModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbStreamIdAssignmentModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbFuMsgBlockGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbAuMsgBlockGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbAuqFuqMessagesNumberGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbSecondaryAuMsgBlockGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbRoutingPortIpUcEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbRoutingPortIpUcEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbRoutingUcRefreshEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbRoutingUcRefreshEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbRoutingUcAgingEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbRoutingUcAgingEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbRoutingUcTransplantEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbRoutingUcTransplantEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbRoutingUcDeleteEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbRoutingUcDeleteEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbRoutingUcAAandTAToCpuSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbRoutingUcAAandTAToCpuGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbRoutingNextHopPacketCmdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbRoutingNextHopPacketCmdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenIgmpSnoopEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenIgmpSnoopEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenDropIpMcEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenDropIpMcEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenDropNonIpMcEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenDropNonIpMcEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenDropInvalidSaEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenDropInvalidSaEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenUcLocalSwitchingEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenUcLocalSwitchingEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenIgmpSnoopModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenIgmpSnoopModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenArpBcastToCpuCmdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenArpBcastToCpuCmdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenIeeeReservedMcastTrapEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenIeeeReservedMcastTrapEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenIeeeReservedMcastProtCmdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenIeeeReservedMcastProtCmdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenCiscoL2ProtCmdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenCiscoL2ProtCmdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenIpV6IcmpTrapEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenIpV6IcmpTrapEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenIcmpv6MsgTypeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenIcmpv6MsgTypeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenIpLinkLocalMirrorToCpuEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenIpLinkLocalMirrorToCpuEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenIpLinkLocalProtMirrorToCpuEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenIpV6SolicitedCmdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenIpV6SolicitedCmdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenIpV6SolicitedNodeMcastPrefixGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenRateLimitGlobalCfgSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenRateLimitGlobalCfgGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenPortRateLimitSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenPortRateLimitGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenPortRateLimitTcpSynSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenPortRateLimitTcpSynGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenUdpBcDestPortCfgSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenUdpBcDestPortCfgGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenUdpBcDestPortCfgInvalidate_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenBpduTrapEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenBpduTrapEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenArpTrapEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenArpTrapEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenDropIpMcModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenDropIpMcModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenDropNonIpMcModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenDropNonIpMcModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenCfiRelayEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenCfiRelayEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenPortIeeeReservedMcastProfileIndexSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenPortIeeeReservedMcastProfileIndexGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenPortOversizeUntaggedPacketsFilterEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenPortOversizeUntaggedPacketsFilterCmdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenRateLimitSpeedCfgEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenRateLimitSpeedCfgEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenPortRateLimitSpeedGranularitySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenPortRateLimitSpeedGranularityGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenExcludeLinkLocalMcFromUnregMcFilterEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenPortIeeeReservedMcastLearningEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenPortIeeeReservedMcastLearningEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenPortRateLimitDropCntrEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenPortRateLimitDropCntrEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenRateLimitDropCntrSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenRateLimitDropCntrGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenPortArpMacSaMismatchDropEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenPortArpMacSaMismatchDropEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenIngressPortUnknownUcFilterDaCommandGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenIngressPortUnregisteredMcFilterDaCommandGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenIngressPortBcFilterDaCommandSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenIngressPortBcFilterDaCommandGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenPortIpControlTrapEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenPortIpControlTrapEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenPortBcUdpTrapMirrorEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenPortBcUdpTrapMirrorEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenPortBypassModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenPortBypassModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenBypassModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenBypassModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenMtuConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenMtuConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenMtuPortProfileIdxSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenMtuPortProfileIdxGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenMtuExceedCntrGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenMtuProfileSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenMtuProfileGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenIpmClassificationModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenIpmClassificationModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenRipV1CmdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenRipV1CmdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenIpLinkLocalProtCmdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgGenIpLinkLocalProtCmdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgL2DlbEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgL2DlbEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgL2DlbLttIndexBaseEportSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgL2DlbLttIndexBaseEportGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgL2DlbTableEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgL2DlbTableEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgL2DlbPathUtilizationScaleModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgL2DlbPathUtilizationScaleModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgL2DlbMinimumFlowletIpgSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgL2DlbMinimumFlowletIpgGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgL2DlbPathUtilizationTableEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgL2DlbPathUtilizationTableEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgL2DlbWaLttInfoSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgL2DlbWaExecute_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgL2EcmpEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgL2EcmpEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgL2EcmpIndexBaseEportSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgL2EcmpIndexBaseEportGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgL2EcmpLttTableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgL2EcmpLttTableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgL2EcmpTableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgL2EcmpTableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgL2EcmpMemberSelectionModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgL2EcmpMemberSelectionModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgL2EcmpHashBitSelectionProfileSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgL2EcmpHashBitSelectionProfileGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgL2EcmpPortHashSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgL2EcmpPortHashGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgL2EcmpPortHashBitSelectionSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgL2EcmpPortHashBitSelectionGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgL2EcmpMllReplicationConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgL2EcmpMllReplicationConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgMcInit_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgMcIpv6BytesSelectSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgMcIpv6BytesSelectGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgMcEntryWrite_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgMcEntryRead_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgMcGroupDelete_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgMcMemberAdd_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgMcMemberDelete_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgMcPhysicalSourceFilteringEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgMcPhysicalSourceFilteringEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgMcPhysicalSourceFilteringIgnoreSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgMcPhysicalSourceFilteringIgnoreGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgNestVlanAccessPortSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgNestVlanAccessPortGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgNestVlanPortTargetEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgNestVlanPortTargetEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgNestVlanPortTargetUseIngressModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgNestVlanPortTargetUseIngressModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgPeEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgPeEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgPePortEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgPePortEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgPePortPcidMcFilterEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgPePortPcidMcFilterEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgPeCascadePortAssociateToUplinkTrunkEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgPrvEdgeVlanEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgPrvEdgeVlanEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgPrvEdgeVlanPortEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgPrvEdgeVlanPortEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSecurBreachPortVlanDropCntrModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSecurBreachPortVlanDropCntrModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSecurBreachGlobalDropCntrGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSecurBreachPortVlanCntrGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSecurBreachNaPerPortSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSecurBreachNaPerPortGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSecurBreachMovedStaticAddrSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSecurBreachMovedStaticAddrGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSecurBreachEventDropModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSecurBreachEventDropModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChSecurBreachMsgGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSecurBreachPortGroupPortVlanCntrGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSecurBreachPortGroupGlobalDropCntrGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSecurBreachEventPacketCommandSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSecurBreachEventPacketCommandGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSecurBreachMacSpoofProtectionSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSecurBreachMacSpoofProtectionGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSrcIdGroupPortAdd_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSrcIdGroupPortDelete_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSrcIdGroupEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSrcIdGroupEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSrcIdPortDefaultSrcIdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSrcIdPortDefaultSrcIdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSrcIdGlobalUcastEgressFilterSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSrcIdGlobalUcastEgressFilterGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSrcIdGlobalSrcIdAssignModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSrcIdGlobalSrcIdAssignModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSrcIdPortUcastEgressFilterSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSrcIdPortUcastEgressFilterGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSrcIdPortSrcIdForceEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSrcIdPortSrcIdForceEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSrcIdEtagTypeLocationSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSrcIdEtagTypeLocationGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSrcIdAddTag1LocationSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSrcIdAddTag1LocationGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSrcIdPortOddOnlyFilterEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSrcIdPortOddOnlyFilterEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSrcIdTargetIndexConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSrcIdTargetIndexConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSrcIdPortTargetConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSrcIdPortTargetConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSrcIdNotOverrideFromCpuEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSrcIdNotOverrideFromCpuEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSrcIdTreatMllReplicatedAsMcEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgSrcIdTreatMllReplicatedAsMcEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgStpInit_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgStpStateSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgStpStateGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgStpEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgStpEntryWrite_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgStpPortSpanningTreeStateSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgStpPortSpanningTreeStateGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgStpPortSpanningTreeStateModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgStpPortSpanningTreeStateModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanInit_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanEntryWrite_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanEntriesRangeWrite_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanEntryRead_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanMemberAdd_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortDelete_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanMemberSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortVidGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortVidSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortIngFltEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortIngFltEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortProtoClassVlanEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortProtoClassVlanEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortProtoClassQosEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortProtoClassQosEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanProtoClassSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanProtoClassGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanProtoClassInvalidate_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortProtoVlanQosSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortProtoVlanQosGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortProtoVlanQosInvalidate_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanLearningStateSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanIsDevMember_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanToStpIdBind_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanStpIdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanTableInvalidate_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanEntryInvalidate_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanUnkUnregFilterSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanForcePvidEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanForcePvidEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortVidPrecedenceSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortVidPrecedenceGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanIpUcRouteEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanIpMcRouteEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanNASecurEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanIgmpSnoopingEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanIgmpSnoopingEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanIpCntlToCpuSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanIpCntlToCpuGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanIpV6IcmpToCpuEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanIpV6IcmpToCpuEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanUdpBcPktsToCpuEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanUdpBcPktsToCpuEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanIpv6SourceSiteIdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanIpmBridgingEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanIpmBridgingModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanIngressMirrorEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortAccFrameTypeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortAccFrameTypeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanMruProfileIdxSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanMruProfileValueSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanMruProfileValueGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanNaToCpuEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanVrfIdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanBridgingModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanBridgingModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortTranslationEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortTranslationEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanTranslationEntryWrite_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanTranslationEntryRead_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanValidCheckEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanValidCheckEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanTpidEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanTpidEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanIngressTpidProfileSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanIngressTpidProfileGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanTpidTagTypeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanTpidTagTypeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortIngressTpidProfileSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortIngressTpidProfileGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortIngressTpidSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortIngressTpidGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortEgressTpidSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortEgressTpidGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanEgressTagTpidSelectModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanEgressTagTpidSelectModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanRangeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanRangeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortIsolationCmdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanLocalSwitchingEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanMirrorToTxAnalyzerSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanMirrorToTxAnalyzerModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanFloodVidxModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanRemoveVlanTag1IfZeroModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanForceNewDsaToCpuEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanForceNewDsaToCpuEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanKeepVlan1EnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanKeepVlan1EnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortEnableLearningOfOriginalTag1VidGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanIpv4McBcMirrorToAnalyzerIndexGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanIpv6McMirrorToAnalyzerIndexGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanForwardingIdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanForwardingIdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanUnknownMacSaCommandSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanUnknownMacSaCommandGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanEgressPortTagStateModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanEgressPortTagStateModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanEgressPortTagStateSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanEgressPortTagStateGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortPushVlanCommandSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortPushVlanCommandGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortPushedTagTpidSelectEgressGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortPushedTagValueSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortPushedTagValueGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortUpAndCfiAssignmentCommandSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortUpAndCfiAssignmentCommandGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortUpSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortUpGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortCfiEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortCfiEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortNumOfTagWordsToPopSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortNumOfTagWordsToPopGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortVid0CommandEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortVid0CommandEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortVid1CommandEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortVid1CommandEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortUp0CommandEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortUp0CommandEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortUp1CommandEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortUp1CommandEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortVid1Set_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortVid1Get_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortUp0Set_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortUp0Get_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortUp1Set_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortUp1Get_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortRemoveTag1IfRxWithoutTag1EnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortRemoveTag0IfRxWithoutTag0EnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortRemoveTag0IfRxWithoutTag0EnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanIndependentNonFloodVidxEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanIndependentNonFloodVidxEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanFcoeForwardingEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanFcoeForwardingEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanUnregisteredIpmEVidxSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanMembersTableIndexingModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanMembersTableIndexingModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanStgIndexingModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanStgIndexingModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanTagStateIndexingModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanTagStateIndexingModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanBpeTagSourceCidDefaultForceSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanBpeTagSourceCidDefaultForceGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanBpeTagMcCfgSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanBpeTagMcCfgGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanBpeTagReservedFieldsSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanBpeTagReservedFieldsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanFdbLookupKeyModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlan6BytesTagConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlan6BytesTagConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanBpeTagForceIngressEcidEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanBpeTagForceIngressEcidEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortPushedTagUsePhysicalEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortPushedTagUsePhysicalEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortPushedTagPhysicalValueSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPortPushedTagPhysicalValueGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPort8BTagInfoSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanPort8BTagInfoGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanMruCommandSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgVlanMruCommandGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbHashCalc_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbHashByParamsCalc_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbHashCrcMultiResultsCalc_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbHashCrcMultiResultsByParamsCalc_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbHashRequestSend_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbHashResultsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbCrcHashUpperBitsModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChBrgFdbCrcHashUpperBitsModeGet_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssDxChBrgLogh */
