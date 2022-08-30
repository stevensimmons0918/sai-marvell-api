/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChIpLpmEngineLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssDxChIpLpmEngineLogh
#define __prvCpssDxChIpLpmEngineLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* manually implemented declarations *********/

void prvCpssLogParamFunc_CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_MAP_DECLARE_MAC(CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_FCOE_LPM_PREFIX_BULK_OPERATION_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_LPM_VR_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_LPM_TCAM_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_FCOE_LPM_PREFIX_BULK_OPERATION_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_LPM_VR_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_FCID_PTR_fcoeAddrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_IPADDR_PTR_ipAddrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_IPADDR_PTR_ipGroupPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_IPADDR_PTR_ipSrcPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_IPV6ADDR_PTR_ipAddrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_IPV6ADDR_PTR_ipGroupPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_IPV6ADDR_PTR_ipSrcPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_defaultIpv4RuleIndexArrayLenPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_defaultIpv6RuleIndexArrayLenPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_ipGroupPrefixLenPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_ipSrcPrefixLenPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_lpmDbMemBlockSizePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_numOfDevsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_pclIdArrayLenPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_prefixLenPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_UINTPTR_PTR_iterPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_FCOE_LPM_PREFIX_BULK_OPERATION_STC_PTR_fcoeAddrPrefixArrayPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC_PTR_ipv4PrefixArrayPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC_PTR_ipv6PrefixArrayPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT_PTR_memoryCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT_shadowType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_LPM_VR_CONFIG_STC_PTR_vrConfigPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_LTT_ENTRY_STC_PTR_mcRouteLttEntryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC_PTR_tcamLpmManagerCapcityCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC_PTR_indexesRangePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT_PTR_nextHopInfoPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_activityBit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_clearActivity;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_defragmentationEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_dump;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_returnOnFailure;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_FCID_PTR_fcoeAddrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_IPADDR_PTR_ipAddrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_IPADDR_PTR_ipGroupPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_IPADDR_PTR_ipSrcPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_IPV6ADDR_PTR_ipAddrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_IPV6ADDR_PTR_ipGroupPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_IPV6ADDR_PTR_ipSrcPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_PTR_lpmDbMemBlockPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_PTR_pclIdArray;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_fcoeAddrPrefixArrayLen;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ipGroupPrefixLen;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ipSrcPrefixLen;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ipv4PrefixArrayLen;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ipv6PrefixArrayLen;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_lpmDBId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_lpmDbId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_pclIdArrayLen;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_vrId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT_PTR_memoryCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT_PTR_shadowTypePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_LPM_VR_CONFIG_STC_PTR_vrConfigPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_LTT_ENTRY_STC_PTR_mcRouteLttEntryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC_PTR_tcamLpmManagerCapcityCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC_PTR_indexesRangePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT_PTR_nextHopInfoPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_IP_PROTOCOL_STACK_ENT_PTR_protocolStackPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_activityBitPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_activityStatusPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_defaultMcUsedPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_partitionEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_defaultIpv4RuleIndexArray;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_defaultIpv6RuleIndexArray;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_ipv4McPrefixNumberPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_ipv4UcPrefixNumberPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_ipv6McPrefixNumberPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_ipv6UcPrefixNumberPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_lpmDbMemBlockPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_lpmDbSizePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_pclIdArray;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tcamColumnIndexPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tcamGroupColumnIndexPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tcamGroupRowIndexPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tcamRowIndexPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tcamSrcColumnIndexPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tcamSrcRowIndexPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_devListArray;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDBCreate_E = (CPSS_LOG_LIB_IP_LPM_E << 16) | (1 << 24),
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDBDelete_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDBConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDBCapacityUpdate_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDBCapacityGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDBDevListAdd_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDBDevsListRemove_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDBDevListGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmPolicyBasedRoutingDefaultMcSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmPolicyBasedRoutingDefaultMcGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmVirtualRouterAdd_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmVirtualRouterSharedAdd_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmVirtualRouterGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmVirtualRouterDel_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv4UcPrefixAdd_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv4UcPrefixAddBulk_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv4UcPrefixDel_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv4UcPrefixDelBulk_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv4UcPrefixesFlush_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv4UcPrefixSearch_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv4UcPrefixGetNext_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv4UcPrefixGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv4McEntryAdd_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv4McEntryDel_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv4McEntriesFlush_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv4McEntryGetNext_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv4McEntrySearch_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv6UcPrefixAdd_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv6UcPrefixAddBulk_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv6UcPrefixDel_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv6UcPrefixDelBulk_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv6UcPrefixesFlush_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv6UcPrefixSearch_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv6UcPrefixGetNext_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv6UcPrefixGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv6McEntryAdd_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv6McEntryDel_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv6McEntriesFlush_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv6McEntryGetNext_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv6McEntrySearch_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmFcoePrefixAdd_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmFcoePrefixDel_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmFcoePrefixAddBulk_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmFcoePrefixDelBulk_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmFcoePrefixesFlush_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmFcoePrefixSearch_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmFcoePrefixGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmFcoePrefixGetNext_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDBMemSizeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDBExport_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDBImport_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmActivityBitEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmActivityBitEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv4UcPrefixActivityStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv6UcPrefixActivityStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv4McEntryActivityStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmIpv6McEntryActivityStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmPrefixesNumberGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgRouteTcamDump_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgPatTriePrint_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgPatTrieValidityCheck_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgDump_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgHwMemPrint_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgHwValidation_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgShadowValidityCheck_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgHwShadowSyncValidityCheck_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgHwShadowValidityAndSyncCheck_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgHwOctetPerBlockPrint_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgHwBlockInfoPrint_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgHwOctetPerProtocolPrint_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssDxChIpLpmEngineLogh */
