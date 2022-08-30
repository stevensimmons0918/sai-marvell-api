/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssPxEgressLog.c
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
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssPxGenDbLog.h>
#include <cpss/generic/log/prvCpssPxGenLog.h>
#include <cpss/px/egress/cpssPxEgress.h>
#include <cpss/px/egress/private/prvCpssPxEgressLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT[]  =
{
    "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E",
    "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_E",
    "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CASCADE_PORT_E",
    "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CASCADE_PORT_TO_UPSTREAM_PORT_E",
    "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_CPU_PORT_E",
    "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_CPU_PORT_TO_UPSTREAM_PORT_E",
    "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E",
    "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E",
    "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_TO_EXTENDED_PORT_E",
    "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_MIRRORING_TO_EXTENDED_PORT_E",
    "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_CPU_PORT_TO_UPSTREAM_PORT_E",
    "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_E",
    "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_IPL_PORT_E",
    "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_IPL_PORT_TO_IPL_PORT_E",
    "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_UNTAGGED_E",
    "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M4_PORT_E",
    "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_M8_PORT_E",
    "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DROP_PACKET_E",
    "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_E2U_PACKET_E",
    "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_U2E_PACKET_E",
    "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_QCN_E",
    "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_EXTENDED_PORT_TO_UPSTREAM_PORT_E",
    "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_UPSTREAM_PORT_TO_EXTENDED_PORT_E",
    "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DO_NOT_MODIFY_E",
    "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E",
    "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAG1_TAGGED_TO_UPSTREAM_PORT_E",
    "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E",
    "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_DTAGGED_TO_UPSTREAM_PORT_E",
    "CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE___MUST_BE_LAST___E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT[]  =
{
    "CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_E",
    "CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_UNTAGGED_E",
    "CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_DSA_E",
    "CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_NOT_USED_E",
    "CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_EVB_E",
    "CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_EDSA_E",
    "CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_PRE_DA_E",
    "CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE___MUST_BE_LAST___E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT[]  =
{
    "CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_COMMON_E",
    "CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_DSA_QCN_E",
    "CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_DO_NOT_MODIFY_E",
    "CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_EVB_E",
    "CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE___MUST_BE_LAST___E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_PX_EGRESS_SOURCE_PORT_PTP_MODE_ENT[]  =
{
    "CPSS_PX_EGRESS_SOURCE_PORT_PTP_MODE_TC_E",
    "CPSS_PX_EGRESS_SOURCE_PORT_PTP_MODE_OC_BC_INGRESS_E",
    "CPSS_PX_EGRESS_SOURCE_PORT_PTP_MODE_OC_BC_EGRESS_E",
    "CPSS_PX_EGRESS_SOURCE_PORT_PTP_MODE_OC_BC_EGRESS_CF_TIMESTAMP_E",
    "CPSS_PX_EGRESS_SOURCE_PORT_PTP_MODE___MUST_BE_LAST___E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_EGRESS_SOURCE_PORT_PTP_MODE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_PX_VLAN_TAG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_VLAN_TAG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, pcp);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dei);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vid);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, eTag, CPSS_802_1BR_ETAG_STC);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vlanTagTpid);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_MC_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_MC_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, pcid, 7, GT_U32);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, dsaForward, CPSS_PX_REGULAR_DSA_FORWARD_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, dsaForward, CPSS_PX_REGULAR_DSA_FORWARD_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ETH_MAC(valPtr, macSa);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, dsaExtForward, CPSS_PX_EXTENDED_DSA_FORWARD_STC);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, cnmTpid);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TO_UPSTREAM_PORT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TO_UPSTREAM_PORT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, eDsaForward, CPSS_PX_EDSA_FORWARD_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_QCN_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_QCN_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ETH_MAC(valPtr, macSa);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, qcnTpid);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_PTP_DATA_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_PTP_DATA_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, messageType);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ptpVersion);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, domainNumber);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, udpDestPort);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ipv4ProtocolIpv6NextHeader);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, pcid);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, srcPortInfo, CPSS_PX_EGRESS_SRC_PORT_INFO_STC);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, upstreamPort);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ptpPortInfo, CPSS_PX_EGRESS_SOURCE_PORT_PTP_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, srcPortNum);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ptpPortInfo, CPSS_PX_EGRESS_SOURCE_PORT_PTP_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_EGRESS_SOURCE_PORT_EVB_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_EGRESS_SOURCE_PORT_EVB_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vid);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ptpPortInfo, CPSS_PX_EGRESS_SOURCE_PORT_PTP_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_EGRESS_SOURCE_PORT_PRE_DA_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_EGRESS_SOURCE_PORT_PRE_DA_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, port);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ptpPortInfo, CPSS_PX_EGRESS_SOURCE_PORT_PTP_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_EGRESS_SOURCE_PORT_PTP_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_EGRESS_SOURCE_PORT_PTP_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ptpOverMplsEn);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, ptpPortMode, CPSS_PX_EGRESS_SOURCE_PORT_PTP_MODE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, tpid);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, pcid);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, egressDelay);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_EDSA_FORWARD_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_EDSA_FORWARD_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, srcTagged);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, hwSrcDev);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, tpIdIndex);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, tag1SrcTagged);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_EGRESS_SRC_PORT_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_EGRESS_SRC_PORT_INFO_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, srcFilteringVector);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, vlanTag, CPSS_PX_VLAN_TAG_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_EXTENDED_DSA_FORWARD_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_EXTENDED_DSA_FORWARD_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, srcTagged);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, hwSrcDev);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, srcPortOrTrunk);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, srcIsTrunk);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, cfi);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, up);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vid);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, egrFilterRegistered);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, dropOnSource);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, packetIsLooped);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, wasRouted);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, srcId);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, qosProfileIndex);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, useVidx);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, trgVidx);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, trgPort);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, hwTrgDev);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_REGULAR_DSA_FORWARD_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_REGULAR_DSA_FORWARD_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, srcTagged);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, hwSrcDev);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, srcPortOrTrunk);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, srcIsTrunk);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, cfi);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, up);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vid);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_VLAN_TAG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_VLAN_TAG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_VLAN_TAG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT_PTR_operationInfoPtr = {
     "operationInfoPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT_operationType = {
     "operationType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT_infoType = {
     "infoType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT_PTR_portInfoPtr = {
     "portInfoPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT_infoType = {
     "infoType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT_PTR_portInfoPtr = {
     "portInfoPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORTS_BMP_portsTagging = {
     "portsTagging", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PORTS_BMP)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_VLAN_TAG_STC_PTR_vlanTagPtr = {
     "vlanTagPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_VLAN_TAG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_phaBypassEnable = {
     "phaBypassEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_ppaClockEnable = {
     "ppaClockEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_useTod = {
     "useTod", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_vlanId = {
     "vlanId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT_PTR_operationInfoPtr = {
     "operationInfoPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT_PTR_operationTypePtr = {
     "operationTypePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT_PTR_infoTypePtr = {
     "infoTypePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT_PTR_portInfoPtr = {
     "portInfoPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT_PTR_infoTypePtr = {
     "infoTypePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT_PTR_portInfoPtr = {
     "portInfoPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORTS_BMP_PTR_portsTaggingPtr = {
     "portsTaggingPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORTS_BMP)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_VLAN_TAG_STC_PTR_vlanTagPtr = {
     "vlanTagPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_VLAN_TAG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_phaBypassEnablePtr = {
     "phaBypassEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_ppaClockEnablePtr = {
     "ppaClockEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_useTodPtr = {
     "useTodPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxEgressTimestampModeSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PACKET_TYPE_packetType,
    &PX_IN_GT_BOOL_useTod
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxEgressTimestampModeGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PACKET_TYPE_packetType,
    &PX_OUT_GT_BOOL_PTR_useTodPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxEgressBypassModeSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_BOOL_phaBypassEnable,
    &PX_IN_GT_BOOL_ppaClockEnable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxEgressSourcePortEntrySet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT_infoType,
    &PX_IN_CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT_PTR_portInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxEgressTargetPortEntrySet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT_infoType,
    &PX_IN_CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT_PTR_portInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxEgressHeaderAlterationEntrySet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PX_PACKET_TYPE_packetType,
    &PX_IN_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT_operationType,
    &PX_IN_CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT_PTR_operationInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxEgressHeaderAlterationEntryGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PX_PACKET_TYPE_packetType,
    &PX_OUT_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT_PTR_operationTypePtr,
    &PX_OUT_CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT_PTR_operationInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxEgressQcnVlanTagEntrySet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PX_VLAN_TAG_STC_PTR_vlanTagPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxEgressSourcePortEntryGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT_PTR_infoTypePtr,
    &PX_OUT_CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT_PTR_portInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxEgressTargetPortEntryGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT_PTR_infoTypePtr,
    &PX_OUT_CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT_PTR_portInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxEgressQcnVlanTagEntryGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PX_VLAN_TAG_STC_PTR_vlanTagPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxEgressVlanTagStateEntrySet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_vlanId,
    &PX_IN_CPSS_PX_PORTS_BMP_portsTagging
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxEgressVlanTagStateEntryGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_vlanId,
    &PX_OUT_CPSS_PX_PORTS_BMP_PTR_portsTaggingPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxEgressBypassModeGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_GT_BOOL_PTR_phaBypassEnablePtr,
    &PX_OUT_GT_BOOL_PTR_ppaClockEnablePtr
};


/********* lib API DB *********/

extern void cpssPxEgressHeaderAlterationEntrySet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssPxEgressHeaderAlterationEntryGet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssPxEgressSourcePortEntrySet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssPxEgressSourcePortEntryGet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssPxEgressTargetPortEntrySet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssPxEgressTargetPortEntryGet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssPxEgressLogLibDb[] = {
    {"cpssPxEgressBypassModeSet", 3, cpssPxEgressBypassModeSet_PARAMS, NULL},
    {"cpssPxEgressBypassModeGet", 3, cpssPxEgressBypassModeGet_PARAMS, NULL},
    {"cpssPxEgressTimestampModeSet", 3, cpssPxEgressTimestampModeSet_PARAMS, NULL},
    {"cpssPxEgressTimestampModeGet", 3, cpssPxEgressTimestampModeGet_PARAMS, NULL},
    {"cpssPxEgressHeaderAlterationEntrySet", 5, cpssPxEgressHeaderAlterationEntrySet_PARAMS, cpssPxEgressHeaderAlterationEntrySet_preLogic},
    {"cpssPxEgressHeaderAlterationEntryGet", 5, cpssPxEgressHeaderAlterationEntryGet_PARAMS, cpssPxEgressHeaderAlterationEntryGet_preLogic},
    {"cpssPxEgressSourcePortEntrySet", 4, cpssPxEgressSourcePortEntrySet_PARAMS, cpssPxEgressSourcePortEntrySet_preLogic},
    {"cpssPxEgressSourcePortEntryGet", 4, cpssPxEgressSourcePortEntryGet_PARAMS, cpssPxEgressSourcePortEntryGet_preLogic},
    {"cpssPxEgressTargetPortEntrySet", 4, cpssPxEgressTargetPortEntrySet_PARAMS, cpssPxEgressTargetPortEntrySet_preLogic},
    {"cpssPxEgressTargetPortEntryGet", 4, cpssPxEgressTargetPortEntryGet_PARAMS, cpssPxEgressTargetPortEntryGet_preLogic},
    {"cpssPxEgressVlanTagStateEntrySet", 3, cpssPxEgressVlanTagStateEntrySet_PARAMS, NULL},
    {"cpssPxEgressVlanTagStateEntryGet", 3, cpssPxEgressVlanTagStateEntryGet_PARAMS, NULL},
    {"cpssPxEgressQcnVlanTagEntrySet", 3, cpssPxEgressQcnVlanTagEntrySet_PARAMS, NULL},
    {"cpssPxEgressQcnVlanTagEntryGet", 3, cpssPxEgressQcnVlanTagEntryGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_PX_CPSS_LOG_LIB_EGRESS(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssPxEgressLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssPxEgressLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

