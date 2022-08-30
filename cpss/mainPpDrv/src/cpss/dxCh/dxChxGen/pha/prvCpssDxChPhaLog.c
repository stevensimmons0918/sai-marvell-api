/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChPhaLog.c
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
#include <cpss/dxCh/dxChxGen/pha/cpssDxChPha.h>
#include <cpss/dxCh/dxChxGen/pha/private/prvCpssDxChPhaLog.h>
#include <cpss/generic/log/prvCpssGenLog.h>
#include <cpss/generic/networkIf/private/prvCpssGenNetworkIfLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_DXCH_PHA_FW_IMAGE_ID_ENT[]  =
{
    "CPSS_DXCH_PHA_FW_IMAGE_ID_DEFAULT_E",
    "CPSS_DXCH_PHA_FW_IMAGE_ID_01_E",
    "CPSS_DXCH_PHA_FW_IMAGE_ID_02_E",
    "CPSS_DXCH_PHA_FW_IMAGE_ID_LAST_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PHA_FW_IMAGE_ID_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_ENT[]  =
{
    "CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_FW_INC_OVERSIZE_E",
    "CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_HEADER_OVERSIZE_E",
    "CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_FW_DEC_VIOLATION_E",
    "CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_HEADER_UNDERSIZE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT[]  =
{
    "CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNUSED_E",
    "CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ERSPAN_TYPE_II_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT[]  =
{
    "CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNUSED_E",
    "CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ERSPAN_TYPE_II_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PHA_THREAD_TYPE_ENT[]  =
{
    "CPSS_DXCH_PHA_THREAD_TYPE_UNUSED_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV4_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV6_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV4_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV6_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_INT_IOAM_MIRRORING_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_INT_IOAM_EGRESS_SWITCH_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_NO_EL_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_ONE_EL_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_TWO_EL_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_MPLS_SR_THREE_EL_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_UNIFIED_SR_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_CLASSIFIER_NSH_OVER_ETHERNET_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_CLASSIFIER_NSH_OVER_VXLAN_GPE_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_SFF_NSH_VXLAN_GPE_TO_ETHERNET_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_SFF_NSH_ETHERNET_TO_VXLAN_GPE_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_IOAM_EGRESS_SWITCH_IPV6_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_SRV6_END_NODE_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_SRV6_PENULTIMATE_END_NODE_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_1_SEGMENT_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_FIRST_PASS_2_3_SEGMENTS_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_3_SEGMENTS_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_2_SEGMENTS_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_SGT_NETWORK_ADD_MSB_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_SGT_NETWORK_FIX_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_SGT_NETWORK_REMOVE_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_SGT_EDSA_FIX_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_SGT_EDSA_REMOVE_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_FIX_IPV4_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_FIX_IPV6_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_REMOVE_IPV4_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_REMOVE_IPV6_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_PTP_PHY_1_STEP_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_EGRESS_MIRRORING_METADATA_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_LC_IPV4_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_LC_IPV6_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_DIRECT_IPV4_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_DIRECT_IPV6_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_SRC_DEV_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_VXLAN_GBP_SOURCE_GROUP_POLICY_ID_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_SRV6_BEST_EFFORT_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_1_CONTAINER_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_FIRST_PASS_2_3_CONTAINERS_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_2_CONTAINERS_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_3_CONTAINERS_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_SRV6_END_NODE_COC32_GSID_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_IPV4_TTL_INCREMENT_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_IPV6_HOP_LIMIT_INCREMENT_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_CLEAR_OUTGOING_MTAG_COMMAND_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_SFLOW_V5_IPV4_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_SFLOW_V5_IPV6_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_SLS_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_DROP_ALL_TRAFFIC_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_ERSPAN_TYPE_II_SAME_DEV_IPV4_E",
    "CPSS_DXCH_PHA_THREAD_TYPE_ERSPAN_TYPE_II_SAME_DEV_IPV6_E",
    "CPSS_DXCH_PHA_THREAD_TYPE___LAST___E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PHA_THREAD_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PHA_BUSY_STALL_MODE_ENT[]  =
{
    "CPSS_DXCH_PHA_BUSY_STALL_MODE_PUSH_BACK_E",
    "CPSS_DXCH_PHA_BUSY_STALL_MODE_DROP_E",
    "CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PHA_BUSY_STALL_MODE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, statisticalProcessingFactor);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, busyStallMode, CPSS_DXCH_PHA_BUSY_STALL_MODE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, stallDropCode, CPSS_NET_RX_CPU_CODE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_ERROR_CODE_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PHA_ERROR_CODE_CONFIG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, tablesReadErrorDropCode, CPSS_NET_RX_CPU_CODE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, ppaClockDownErrorDropCode, CPSS_NET_RX_CPU_CODE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_FW_IMAGE_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PHA_FW_IMAGE_INFO_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, fwImageId, CPSS_DXCH_PHA_FW_IMAGE_ID_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, fwVersion);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, fwVersionsArr, CPSS_DXCH_PHA_FW_NUM_OF_SUPPORTED_VERSIONS_PER_IMAGE_CNS, GT_U32);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, rawFormat);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, erspanSameDevMirror, CPSS_DXCH_PHA_PORT_ENTRY_TYPE_ERSPAN_SAME_DEV_MIRROR_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, rawFormat);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, erspanSameDevMirror, CPSS_DXCH_PHA_PORT_ENTRY_TYPE_ERSPAN_SAME_DEV_MIRROR_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_THREAD_INFO_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PHA_THREAD_INFO_UNT *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, notNeeded);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ioamIngressSwitchIpv4, CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_IPV4_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ioamIngressSwitchIpv6, CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_IPV6_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ioamTransitSwitchIpv4, CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_IPV4_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ioamTransitSwitchIpv6, CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_IPV6_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ioamEgressSwitchIpv6, CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_EGRESS_SWITCH_IPV6_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, unifiedSRIpv6, CPSS_DXCH_PHA_THREAD_INFO_TYPE_UNIFIED_SR_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, classifierNshOverVxlanGpe, CPSS_DXCH_PHA_THREAD_INFO_TYPE_CLASSIFIER_NSH_OVER_VXLAN_GPE_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, srv6SrcNode, CPSS_DXCH_PHA_THREAD_INFO_TYPE_SRV6_SRC_NODE_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, sgtNetwork, CPSS_DXCH_PHA_THREAD_INFO_TYPE_SGT_NETWORK_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, vxlanGbpSourceGroupPolicyId, CPSS_DXCH_PHA_THREAD_INFO_TYPE_VXLAN_GBP_SOURCE_GROUP_POLICY_ID_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, srv6Coc32GsidCommonPrefix, CPSS_DXCH_PHA_THREAD_INFO_TYPE_SRV6_COC32_GSID_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, sflowV5Mirror, CPSS_DXCH_PHA_THREAD_INFO_TYPE_SFLOW_V5_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, l2Info, CPSS_DXCH_PHA_SHARED_MEMORY_ERSPAN_L2_STC);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, protocol, CPSS_IP_PROTOCOL_STACK_ENT);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ipInfo, CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_UNT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ingressSessionId);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, egressSessionId);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_PORT_ENTRY_TYPE_ERSPAN_SAME_DEV_MIRROR_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PHA_PORT_ENTRY_TYPE_ERSPAN_SAME_DEV_MIRROR_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, erspanIndex);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_SHARED_MEMORY_ERSPAN_L2_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PHA_SHARED_MEMORY_ERSPAN_L2_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ETH_MAC(valPtr, macDa);
    PRV_CPSS_LOG_STC_ETH_MAC(valPtr, macSa);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, tpid);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, up);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, cfi);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vid);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_THREAD_INFO_TYPE_CLASSIFIER_NSH_OVER_VXLAN_GPE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PHA_THREAD_INFO_TYPE_CLASSIFIER_NSH_OVER_VXLAN_GPE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, source_node_id);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, reserved);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_EGRESS_SWITCH_IPV6_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_EGRESS_SWITCH_IPV6_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, node_id);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_IPV4_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_IPV4_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, IOAM_Trace_Type);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Maximum_Length);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Flags);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Hop_Lim);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, node_id);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Type1);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, IOAM_HDR_len1);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Reserved1);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Next_Protocol1);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Type2);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, IOAM_HDR_len2);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Reserved2);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Next_Protocol2);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_IPV6_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_IPV6_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, IOAM_Trace_Type);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Maximum_Length);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Flags);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Hop_Lim);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, node_id);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Type1);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, IOAM_HDR_len1);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Reserved1);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Next_Protocol1);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Type2);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, IOAM_HDR_len2);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Reserved2);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, Next_Protocol2);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_IPV4_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_IPV4_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, node_id);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_IPV6_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_IPV6_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, node_id);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_THREAD_INFO_TYPE_SFLOW_V5_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PHA_THREAD_INFO_TYPE_SFLOW_V5_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_IPV4_MAC(valPtr, sflowAgentIpAddr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, sflowDataFormat);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_THREAD_INFO_TYPE_SGT_NETWORK_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PHA_THREAD_INFO_TYPE_SGT_NETWORK_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_16_HEX_MAC(valPtr, etherType);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, version);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, length);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, optionType);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_THREAD_INFO_TYPE_SRV6_COC32_GSID_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PHA_THREAD_INFO_TYPE_SRV6_COC32_GSID_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dipCommonPrefixLength);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_THREAD_INFO_TYPE_SRV6_SRC_NODE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PHA_THREAD_INFO_TYPE_SRV6_SRC_NODE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_IPV6_MAC(valPtr, srcAddr);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_THREAD_INFO_TYPE_UNIFIED_SR_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PHA_THREAD_INFO_TYPE_UNIFIED_SR_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_IPV6_MAC(valPtr, srcAddr);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_THREAD_INFO_TYPE_VXLAN_GBP_SOURCE_GROUP_POLICY_ID_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PHA_THREAD_INFO_TYPE_VXLAN_GBP_SOURCE_GROUP_POLICY_ID_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, copyReservedLsb);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, copyReservedMsb);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_UNT *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ipv4, CPSS_DXCH_PHA_SHARED_MEMORY_ERSPAN_IPV4_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ipv6, CPSS_DXCH_PHA_SHARED_MEMORY_ERSPAN_IPV6_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_SHARED_MEMORY_ERSPAN_IPV4_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PHA_SHARED_MEMORY_ERSPAN_IPV4_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dscp);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, flags);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ttl);
    PRV_CPSS_LOG_STC_IPV4_MAC(valPtr, sipAddr);
    PRV_CPSS_LOG_STC_IPV4_MAC(valPtr, dipAddr);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PHA_SHARED_MEMORY_ERSPAN_IPV6_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PHA_SHARED_MEMORY_ERSPAN_IPV6_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, tc);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, flowLabel);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, hopLimit);
    PRV_CPSS_LOG_STC_IPV6_MAC(valPtr, sipAddr);
    PRV_CPSS_LOG_STC_IPV6_MAC(valPtr, dipAddr);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PHA_ERROR_CODE_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PHA_ERROR_CODE_CONFIG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PHA_ERROR_CODE_CONFIG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PHA_FW_IMAGE_ID_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PHA_FW_IMAGE_ID_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PHA_FW_IMAGE_ID_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PHA_FW_IMAGE_ID_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PHA_FW_IMAGE_ID_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_PHA_FW_IMAGE_ID_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PHA_FW_IMAGE_INFO_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PHA_FW_IMAGE_INFO_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PHA_FW_IMAGE_INFO_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PHA_THREAD_INFO_UNT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PHA_THREAD_INFO_UNT*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PHA_THREAD_INFO_UNT_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PHA_THREAD_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PHA_THREAD_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PHA_THREAD_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PHA_THREAD_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PHA_THREAD_TYPE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_PHA_THREAD_TYPE_ENT);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC_PTR_commonInfoPtr = {
     "commonInfoPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PHA_ERROR_CODE_CONFIG_STC_PTR_errorCodePtr = {
     "errorCodePtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PHA_ERROR_CODE_CONFIG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PHA_FW_IMAGE_ID_ENT_phaFwImageId = {
     "phaFwImageId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PHA_FW_IMAGE_ID_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PHA_FW_IMAGE_INFO_STC_PTR_fwImageInfoPtr = {
     "fwImageInfoPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PHA_FW_IMAGE_INFO_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT_infoType = {
     "infoType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT_PTR_portInfoPtr = {
     "portInfoPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT_infoType = {
     "infoType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT_PTR_portInfoPtr = {
     "portInfoPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PHA_THREAD_INFO_UNT_PTR_extInfoPtr = {
     "extInfoPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PHA_THREAD_INFO_UNT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC_PTR_entryPtr = {
     "entryPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PHA_THREAD_TYPE_ENT_extType = {
     "extType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PHA_THREAD_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PACKET_CMD_ENT_packetCommand = {
     "packetCommand", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PACKET_CMD_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_isVoQ = {
     "isVoQ", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_packetOrderChangeEnable = {
     "packetOrderChangeEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U16_erspanDevId = {
     "erspanDevId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U16)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_phaThreadId = {
     "phaThreadId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC_PTR_commonInfoPtr = {
     "commonInfoPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHA_ERROR_CODE_CONFIG_STC_PTR_errorCodePtr = {
     "errorCodePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PHA_ERROR_CODE_CONFIG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHA_FW_IMAGE_ID_ENT_PTR_phaFwImageIdPtr = {
     "phaFwImageIdPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PHA_FW_IMAGE_ID_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHA_FW_IMAGE_INFO_STC_PTR_fwImageInfoPtr = {
     "fwImageInfoPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PHA_FW_IMAGE_INFO_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_ENT_PTR_violationTypePtr = {
     "violationTypePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT_PTR_infoTypePtr = {
     "infoTypePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT_PTR_portInfoPtr = {
     "portInfoPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT_PTR_infoTypePtr = {
     "infoTypePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT_PTR_portInfoPtr = {
     "portInfoPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHA_THREAD_INFO_UNT_PTR_extInfoPtr = {
     "extInfoPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PHA_THREAD_INFO_UNT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC_PTR_entryPtr = {
     "entryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHA_THREAD_TYPE_ENT_PTR_extTypePtr = {
     "extTypePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PHA_THREAD_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PACKET_CMD_ENT_PTR_packetCommandPtr = {
     "packetCommandPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PACKET_CMD_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_isVoQPtr = {
     "isVoQPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U16_PTR_erspanDevIdPtr = {
     "erspanDevIdPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U16)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_capturedThreadIdPtr = {
     "capturedThreadIdPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_phaThreadIdPtr = {
     "phaThreadIdPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPhaErrorsConfigSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PHA_ERROR_CODE_CONFIG_STC_PTR_errorCodePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPhaHeaderModificationViolationInfoSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_NET_RX_CPU_CODE_ENT_dropCode,
    &DX_IN_CPSS_PACKET_CMD_ENT_packetCommand
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPhaInit_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_BOOL_packetOrderChangeEnable,
    &DX_IN_CPSS_DXCH_PHA_FW_IMAGE_ID_ENT_phaFwImageId
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPhaFwImageUpgrade_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_BOOL_packetOrderChangeEnable,
    &DX_IN_CPSS_DXCH_PHA_FW_IMAGE_INFO_STC_PTR_fwImageInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPhaSourcePortEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT_infoType,
    &DX_IN_CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT_PTR_portInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPhaTargetPortEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT_infoType,
    &DX_IN_CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT_PTR_portInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPhaSourcePortEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT_PTR_infoTypePtr,
    &DX_OUT_CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT_PTR_portInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPhaTargetPortEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT_PTR_infoTypePtr,
    &DX_OUT_CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT_PTR_portInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPhaPortThreadIdSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_GT_BOOL_enable,
    &DX_IN_GT_U32_phaThreadId
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPhaPortThreadIdGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_OUT_GT_BOOL_PTR_enablePtr,
    &DX_OUT_GT_U32_PTR_phaThreadIdPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPhaSharedMemoryErspanGlobalConfigSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U16_erspanDevId,
    &DX_IN_GT_BOOL_isVoQ
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_analyzerIndex,
    &DX_IN_CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC_PTR_entryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPhaSharedMemoryErspanIpTemplateEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_analyzerIndex,
    &DX_OUT_CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC_PTR_entryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPhaStatisticalProcessingCounterThreadIdSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_phaThreadId
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPhaThreadIdEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_phaThreadId,
    &DX_IN_CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC_PTR_commonInfoPtr,
    &DX_IN_CPSS_DXCH_PHA_THREAD_TYPE_ENT_extType,
    &DX_IN_CPSS_DXCH_PHA_THREAD_INFO_UNT_PTR_extInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPhaThreadIdEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_phaThreadId,
    &DX_OUT_CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC_PTR_commonInfoPtr,
    &DX_OUT_CPSS_DXCH_PHA_THREAD_TYPE_ENT_PTR_extTypePtr,
    &DX_OUT_CPSS_DXCH_PHA_THREAD_INFO_UNT_PTR_extInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPhaErrorsConfigGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_PHA_ERROR_CODE_CONFIG_STC_PTR_errorCodePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPhaFwImageIdGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_PHA_FW_IMAGE_ID_ENT_PTR_phaFwImageIdPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPhaFwImageInfoGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_PHA_FW_IMAGE_INFO_STC_PTR_fwImageInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPhaHeaderModificationViolationInfoGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_NET_RX_CPU_CODE_ENT_PTR_dropCodePtr,
    &DX_OUT_CPSS_PACKET_CMD_ENT_PTR_packetCommandPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPhaSharedMemoryErspanGlobalConfigGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U16_PTR_erspanDevIdPtr,
    &DX_OUT_GT_BOOL_PTR_isVoQPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPhaHeaderModificationViolationCapturedGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_capturedThreadIdPtr,
    &DX_OUT_CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_ENT_PTR_violationTypePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPhaStatisticalProcessingCounterThreadIdGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_phaThreadIdPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPhaStatisticalProcessingCounterGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U64_PTR_counterPtr
};


/********* lib API DB *********/

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChPhaLogLibDb[] = {
    {"cpssDxChPhaInit", 3, cpssDxChPhaInit_PARAMS, NULL},
    {"cpssDxChPhaFwImageIdGet", 2, cpssDxChPhaFwImageIdGet_PARAMS, NULL},
    {"cpssDxChPhaThreadIdEntrySet", 5, cpssDxChPhaThreadIdEntrySet_PARAMS, NULL},
    {"cpssDxChPhaThreadIdEntryGet", 5, cpssDxChPhaThreadIdEntryGet_PARAMS, NULL},
    {"cpssDxChPhaPortThreadIdSet", 4, cpssDxChPhaPortThreadIdSet_PARAMS, NULL},
    {"cpssDxChPhaPortThreadIdGet", 4, cpssDxChPhaPortThreadIdGet_PARAMS, NULL},
    {"cpssDxChPhaSourcePortEntrySet", 4, cpssDxChPhaSourcePortEntrySet_PARAMS, NULL},
    {"cpssDxChPhaSourcePortEntryGet", 4, cpssDxChPhaSourcePortEntryGet_PARAMS, NULL},
    {"cpssDxChPhaTargetPortEntrySet", 4, cpssDxChPhaTargetPortEntrySet_PARAMS, NULL},
    {"cpssDxChPhaTargetPortEntryGet", 4, cpssDxChPhaTargetPortEntryGet_PARAMS, NULL},
    {"cpssDxChPhaHeaderModificationViolationInfoSet", 3, cpssDxChPhaHeaderModificationViolationInfoSet_PARAMS, NULL},
    {"cpssDxChPhaHeaderModificationViolationInfoGet", 3, cpssDxChPhaHeaderModificationViolationInfoGet_PARAMS, NULL},
    {"cpssDxChPhaHeaderModificationViolationCapturedGet", 3, cpssDxChPhaHeaderModificationViolationCapturedGet_PARAMS, NULL},
    {"cpssDxChPhaStatisticalProcessingCounterThreadIdSet", 2, cpssDxChPhaStatisticalProcessingCounterThreadIdSet_PARAMS, NULL},
    {"cpssDxChPhaStatisticalProcessingCounterThreadIdGet", 2, cpssDxChPhaStatisticalProcessingCounterThreadIdGet_PARAMS, NULL},
    {"cpssDxChPhaStatisticalProcessingCounterGet", 2, cpssDxChPhaStatisticalProcessingCounterGet_PARAMS, NULL},
    {"cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet", 3, cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet_PARAMS, NULL},
    {"cpssDxChPhaSharedMemoryErspanIpTemplateEntryGet", 3, cpssDxChPhaSharedMemoryErspanIpTemplateEntryGet_PARAMS, NULL},
    {"cpssDxChPhaSharedMemoryErspanGlobalConfigSet", 3, cpssDxChPhaSharedMemoryErspanGlobalConfigSet_PARAMS, NULL},
    {"cpssDxChPhaSharedMemoryErspanGlobalConfigGet", 3, cpssDxChPhaSharedMemoryErspanGlobalConfigGet_PARAMS, NULL},
    {"cpssDxChPhaErrorsConfigSet", 2, cpssDxChPhaErrorsConfigSet_PARAMS, NULL},
    {"cpssDxChPhaErrorsConfigGet", 2, cpssDxChPhaErrorsConfigGet_PARAMS, NULL},
    {"cpssDxChPhaFwImageInfoGet", 2, cpssDxChPhaFwImageInfoGet_PARAMS, NULL},
    {"cpssDxChPhaFwImageUpgrade", 3, cpssDxChPhaFwImageUpgrade_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_PHA(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChPhaLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChPhaLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

