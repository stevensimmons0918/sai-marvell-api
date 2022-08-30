/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChIpLpmEngineLog.c
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
#include <cpss/dxCh/dxChxGen/ip/private/prvCpssDxChIpLog.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/cpssDxChIpLpmTypes.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/private/prvCpssDxChIpLpmEngineLog.h>
#include <cpss/dxCh/dxChxGen/lpm/private/prvCpssDxChLpmLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssGenDbLog.h>
#include <cpss/generic/log/prvCpssGenLog.h>


/********* enums *********/

const PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT[]  =
{PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_IP_LPM_TCAM_XCAT_SHADOW_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_IP_LPM_TCAM_XCAT_POLICY_BASED_ROUTING_SHADOW_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_IP_LPM_RAM_SIP6_SHADOW_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DXCH_IP_LPM_SHADOW_TYPE_LAST_E)
};
PRV_CPSS_LOG_STC_ENUM_MAP_ARRAY_SIZE_MAC(CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_FCOE_LPM_PREFIX_BULK_OPERATION_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_FCOE_LPM_PREFIX_BULK_OPERATION_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vrId);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, fcoeAddr, GT_FCID);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, prefixLen);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, nextHopInfo, CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, override);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, returnStatus);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vrId);
    PRV_CPSS_LOG_STC_IPV4_MAC(valPtr, ipAddr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, prefixLen);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, nextHopInfo, CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, override);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, returnStatus);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vrId);
    PRV_CPSS_LOG_STC_IPV6_MAC(valPtr, ipAddr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, prefixLen);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, nextHopInfo, CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, override);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, returnStatus);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, tcamDbCfg, CPSS_DXCH_IP_LPM_TCAM_CONFIG_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ramDbCfg, CPSS_DXCH_LPM_RAM_CONFIG_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_LPM_VR_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IP_LPM_VR_CONFIG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, supportIpv4Uc);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, defIpv4UcNextHopInfo, CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, supportIpv4Mc);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, defIpv4McRouteLttEntry, CPSS_DXCH_IP_LTT_ENTRY_STC);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, supportIpv6Uc);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, defIpv6UcNextHopInfo, CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, supportIpv6Mc);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, defIpv6McRouteLttEntry, CPSS_DXCH_IP_LTT_ENTRY_STC);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, supportFcoe);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, defaultFcoeForwardingNextHopInfo, CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numOfIpv4Prefixes);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numOfIpv4McSourcePrefixes);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numOfIpv6Prefixes);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, firstIndex);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, lastIndex);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IP_LPM_TCAM_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IP_LPM_TCAM_CONFIG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_PTR_MAC(valPtr, indexesRangePtr, CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, partitionEnable);
    PRV_CPSS_LOG_STC_STC_PTR_MAC(valPtr, tcamLpmManagerCapcityCfgPtr, CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, tcamManagerHandlerPtr);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_FCOE_LPM_PREFIX_BULK_OPERATION_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_FCOE_LPM_PREFIX_BULK_OPERATION_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_FCOE_LPM_PREFIX_BULK_OPERATION_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAP_MAC(namePtr, paramVal, CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAP_MAC(namePtr, *paramVal, CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_IP_LPM_VR_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_IP_LPM_VR_CONFIG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_IP_LPM_VR_CONFIG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_FCID_PTR_fcoeAddrPtr = {
     "fcoeAddrPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_FCID)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_IPADDR_PTR_ipAddrPtr = {
     "ipAddrPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_IPADDR)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_IPADDR_PTR_ipGroupPtr = {
     "ipGroupPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_IPADDR)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_IPADDR_PTR_ipSrcPtr = {
     "ipSrcPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_IPADDR)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_IPV6ADDR_PTR_ipAddrPtr = {
     "ipAddrPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_IPV6ADDR)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_IPV6ADDR_PTR_ipGroupPtr = {
     "ipGroupPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_IPV6ADDR)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_IPV6ADDR_PTR_ipSrcPtr = {
     "ipSrcPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_IPV6ADDR)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_defaultIpv4RuleIndexArrayLenPtr = {
     "defaultIpv4RuleIndexArrayLenPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_defaultIpv6RuleIndexArrayLenPtr = {
     "defaultIpv6RuleIndexArrayLenPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_ipGroupPrefixLenPtr = {
     "ipGroupPrefixLenPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_ipSrcPrefixLenPtr = {
     "ipSrcPrefixLenPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_lpmDbMemBlockSizePtr = {
     "lpmDbMemBlockSizePtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_numOfDevsPtr = {
     "numOfDevsPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_pclIdArrayLenPtr = {
     "pclIdArrayLenPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_prefixLenPtr = {
     "prefixLenPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_UINTPTR_PTR_iterPtr = {
     "iterPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_UINTPTR)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_FCOE_LPM_PREFIX_BULK_OPERATION_STC_PTR_fcoeAddrPrefixArrayPtr = {
     "fcoeAddrPrefixArrayPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_FCOE_LPM_PREFIX_BULK_OPERATION_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC_PTR_ipv4PrefixArrayPtr = {
     "ipv4PrefixArrayPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC_PTR_ipv6PrefixArrayPtr = {
     "ipv6PrefixArrayPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT_PTR_memoryCfgPtr = {
     "memoryCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT_shadowType = {
     "shadowType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_LPM_VR_CONFIG_STC_PTR_vrConfigPtr = {
     "vrConfigPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_LPM_VR_CONFIG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_LTT_ENTRY_STC_PTR_mcRouteLttEntryPtr = {
     "mcRouteLttEntryPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_LTT_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC_PTR_tcamLpmManagerCapcityCfgPtr = {
     "tcamLpmManagerCapcityCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC_PTR_indexesRangePtr = {
     "indexesRangePtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT_PTR_nextHopInfoPtr = {
     "nextHopInfoPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_activityBit = {
     "activityBit", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_clearActivity = {
     "clearActivity", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_defragmentationEnable = {
     "defragmentationEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_dump = {
     "dump", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_returnOnFailure = {
     "returnOnFailure", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_FCID_PTR_fcoeAddrPtr = {
     "fcoeAddrPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_FCID)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_IPADDR_PTR_ipAddrPtr = {
     "ipAddrPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_IPADDR)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_IPADDR_PTR_ipGroupPtr = {
     "ipGroupPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_IPADDR)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_IPADDR_PTR_ipSrcPtr = {
     "ipSrcPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_IPADDR)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_IPV6ADDR_PTR_ipAddrPtr = {
     "ipAddrPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_IPV6ADDR)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_IPV6ADDR_PTR_ipGroupPtr = {
     "ipGroupPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_IPV6ADDR)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_IPV6ADDR_PTR_ipSrcPtr = {
     "ipSrcPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_IPV6ADDR)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_PTR_lpmDbMemBlockPtr = {
     "lpmDbMemBlockPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_PTR_pclIdArray = {
     "pclIdArray", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_fcoeAddrPrefixArrayLen = {
     "fcoeAddrPrefixArrayLen", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ipGroupPrefixLen = {
     "ipGroupPrefixLen", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ipSrcPrefixLen = {
     "ipSrcPrefixLen", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ipv4PrefixArrayLen = {
     "ipv4PrefixArrayLen", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ipv6PrefixArrayLen = {
     "ipv6PrefixArrayLen", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_lpmDBId = {
     "lpmDBId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_lpmDbId = {
     "lpmDbId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_pclIdArrayLen = {
     "pclIdArrayLen", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_vrId = {
     "vrId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT_PTR_memoryCfgPtr = {
     "memoryCfgPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT_PTR_shadowTypePtr = {
     "shadowTypePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_LPM_VR_CONFIG_STC_PTR_vrConfigPtr = {
     "vrConfigPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_LPM_VR_CONFIG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_LTT_ENTRY_STC_PTR_mcRouteLttEntryPtr = {
     "mcRouteLttEntryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_LTT_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC_PTR_tcamLpmManagerCapcityCfgPtr = {
     "tcamLpmManagerCapcityCfgPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC_PTR_indexesRangePtr = {
     "indexesRangePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT_PTR_nextHopInfoPtr = {
     "nextHopInfoPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_IP_PROTOCOL_STACK_ENT_PTR_protocolStackPtr = {
     "protocolStackPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_IP_PROTOCOL_STACK_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_activityBitPtr = {
     "activityBitPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_activityStatusPtr = {
     "activityStatusPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_defaultMcUsedPtr = {
     "defaultMcUsedPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_partitionEnablePtr = {
     "partitionEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_defaultIpv4RuleIndexArray = {
     "defaultIpv4RuleIndexArray", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_defaultIpv6RuleIndexArray = {
     "defaultIpv6RuleIndexArray", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_ipv4McPrefixNumberPtr = {
     "ipv4McPrefixNumberPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_ipv4UcPrefixNumberPtr = {
     "ipv4UcPrefixNumberPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_ipv6McPrefixNumberPtr = {
     "ipv6McPrefixNumberPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_ipv6UcPrefixNumberPtr = {
     "ipv6UcPrefixNumberPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_lpmDbMemBlockPtr = {
     "lpmDbMemBlockPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_lpmDbSizePtr = {
     "lpmDbSizePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_pclIdArray = {
     "pclIdArray", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tcamColumnIndexPtr = {
     "tcamColumnIndexPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tcamGroupColumnIndexPtr = {
     "tcamGroupColumnIndexPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tcamGroupRowIndexPtr = {
     "tcamGroupRowIndexPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tcamRowIndexPtr = {
     "tcamRowIndexPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tcamSrcColumnIndexPtr = {
     "tcamSrcColumnIndexPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_tcamSrcRowIndexPtr = {
     "tcamSrcRowIndexPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_devListArray = {
     "devListArray", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U8)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmDbgRouteTcamDump_PARAMS[] =  {
    &DX_IN_GT_BOOL_dump
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmDBCreate_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT_shadowType,
    &DX_IN_CPSS_IP_PROTOCOL_STACK_ENT_protocolStack,
    &DX_IN_CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT_PTR_memoryCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmDBCapacityUpdate_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC_PTR_indexesRangePtr,
    &DX_IN_CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC_PTR_tcamLpmManagerCapcityCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmDBImport_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_GT_U32_PTR_lpmDbMemBlockPtr,
    &DX_INOUT_GT_U32_PTR_lpmDbMemBlockSizePtr,
    &DX_INOUT_GT_UINTPTR_PTR_iterPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmFcoePrefixAddBulk_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_GT_U32_fcoeAddrPrefixArrayLen,
    &DX_IN_CPSS_DXCH_FCOE_LPM_PREFIX_BULK_OPERATION_STC_PTR_fcoeAddrPrefixArrayPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmIpv4UcPrefixAddBulk_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_GT_U32_ipv4PrefixArrayLen,
    &DX_IN_CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC_PTR_ipv4PrefixArrayPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmIpv6UcPrefixDelBulk_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_GT_U32_ipv6PrefixArrayLen,
    &DX_IN_CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC_PTR_ipv6PrefixArrayPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmIpv6UcPrefixAddBulk_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_GT_U32_ipv6PrefixArrayLen,
    &DX_IN_CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC_PTR_ipv6PrefixArrayPtr,
    &DX_IN_GT_BOOL_defragmentationEnable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmVirtualRouterDel_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_GT_U32_vrId
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmVirtualRouterAdd_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_GT_U32_vrId,
    &DX_IN_CPSS_DXCH_IP_LPM_VR_CONFIG_STC_PTR_vrConfigPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmDbgPatTrieValidityCheck_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_GT_U32_vrId,
    &DX_IN_CPSS_IP_PROTOCOL_STACK_ENT_protocol,
    &DX_IN_CPSS_UNICAST_MULTICAST_ENT_prefixType
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmFcoePrefixDel_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_GT_U32_vrId,
    &DX_IN_GT_FCID_PTR_fcoeAddrPtr,
    &DX_IN_GT_U32_prefixLen
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmFcoePrefixAdd_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_GT_U32_vrId,
    &DX_IN_GT_FCID_PTR_fcoeAddrPtr,
    &DX_IN_GT_U32_prefixLen,
    &DX_IN_CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT_PTR_nextHopInfoPtr,
    &DX_IN_GT_BOOL_override,
    &DX_IN_GT_BOOL_defragmentationEnable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmFcoePrefixSearch_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_GT_U32_vrId,
    &DX_IN_GT_FCID_PTR_fcoeAddrPtr,
    &DX_IN_GT_U32_prefixLen,
    &DX_OUT_CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT_PTR_nextHopInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmFcoePrefixGet_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_GT_U32_vrId,
    &DX_IN_GT_FCID_PTR_fcoeAddrPtr,
    &DX_OUT_GT_U32_PTR_prefixLenPtr,
    &DX_OUT_CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT_PTR_nextHopInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmIpv4UcPrefixDel_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_GT_U32_vrId,
    &DX_IN_GT_IPADDR_PTR_ipAddrPtr,
    &DX_IN_GT_U32_prefixLen
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmIpv4UcPrefixAdd_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_GT_U32_vrId,
    &DX_IN_GT_IPADDR_PTR_ipAddrPtr,
    &DX_IN_GT_U32_prefixLen,
    &DX_IN_CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT_PTR_nextHopInfoPtr,
    &DX_IN_GT_BOOL_override,
    &DX_IN_GT_BOOL_defragmentationEnable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmIpv4UcPrefixSearch_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_GT_U32_vrId,
    &DX_IN_GT_IPADDR_PTR_ipAddrPtr,
    &DX_IN_GT_U32_prefixLen,
    &DX_OUT_CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT_PTR_nextHopInfoPtr,
    &DX_OUT_GT_U32_PTR_tcamRowIndexPtr,
    &DX_OUT_GT_U32_PTR_tcamColumnIndexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmIpv4UcPrefixGet_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_GT_U32_vrId,
    &DX_IN_GT_IPADDR_PTR_ipAddrPtr,
    &DX_OUT_GT_U32_PTR_prefixLenPtr,
    &DX_OUT_CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT_PTR_nextHopInfoPtr,
    &DX_OUT_GT_U32_PTR_tcamRowIndexPtr,
    &DX_OUT_GT_U32_PTR_tcamColumnIndexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmIpv4McEntryDel_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_GT_U32_vrId,
    &DX_IN_GT_IPADDR_PTR_ipGroupPtr,
    &DX_IN_GT_U32_ipGroupPrefixLen,
    &DX_IN_GT_IPADDR_PTR_ipSrcPtr,
    &DX_IN_GT_U32_ipSrcPrefixLen
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmIpv4McEntryAdd_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_GT_U32_vrId,
    &DX_IN_GT_IPADDR_PTR_ipGroupPtr,
    &DX_IN_GT_U32_ipGroupPrefixLen,
    &DX_IN_GT_IPADDR_PTR_ipSrcPtr,
    &DX_IN_GT_U32_ipSrcPrefixLen,
    &DX_IN_CPSS_DXCH_IP_LTT_ENTRY_STC_PTR_mcRouteLttEntryPtr,
    &DX_IN_GT_BOOL_override,
    &DX_IN_GT_BOOL_defragmentationEnable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmIpv4McEntrySearch_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_GT_U32_vrId,
    &DX_IN_GT_IPADDR_PTR_ipGroupPtr,
    &DX_IN_GT_U32_ipGroupPrefixLen,
    &DX_IN_GT_IPADDR_PTR_ipSrcPtr,
    &DX_IN_GT_U32_ipSrcPrefixLen,
    &DX_OUT_CPSS_DXCH_IP_LTT_ENTRY_STC_PTR_mcRouteLttEntryPtr,
    &DX_OUT_GT_U32_PTR_tcamGroupRowIndexPtr,
    &DX_OUT_GT_U32_PTR_tcamGroupColumnIndexPtr,
    &DX_OUT_GT_U32_PTR_tcamSrcRowIndexPtr,
    &DX_OUT_GT_U32_PTR_tcamSrcColumnIndexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmIpv6UcPrefixDel_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_GT_U32_vrId,
    &DX_IN_GT_IPV6ADDR_PTR_ipAddrPtr,
    &DX_IN_GT_U32_prefixLen
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmIpv6UcPrefixAdd_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_GT_U32_vrId,
    &DX_IN_GT_IPV6ADDR_PTR_ipAddrPtr,
    &DX_IN_GT_U32_prefixLen,
    &DX_IN_CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT_PTR_nextHopInfoPtr,
    &DX_IN_GT_BOOL_override,
    &DX_IN_GT_BOOL_defragmentationEnable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmIpv6UcPrefixSearch_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_GT_U32_vrId,
    &DX_IN_GT_IPV6ADDR_PTR_ipAddrPtr,
    &DX_IN_GT_U32_prefixLen,
    &DX_OUT_CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT_PTR_nextHopInfoPtr,
    &DX_OUT_GT_U32_PTR_tcamRowIndexPtr,
    &DX_OUT_GT_U32_PTR_tcamColumnIndexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmIpv6UcPrefixGet_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_GT_U32_vrId,
    &DX_IN_GT_IPV6ADDR_PTR_ipAddrPtr,
    &DX_OUT_GT_U32_PTR_prefixLenPtr,
    &DX_OUT_CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT_PTR_nextHopInfoPtr,
    &DX_OUT_GT_U32_PTR_tcamRowIndexPtr,
    &DX_OUT_GT_U32_PTR_tcamColumnIndexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmIpv6McEntryDel_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_GT_U32_vrId,
    &DX_IN_GT_IPV6ADDR_PTR_ipGroupPtr,
    &DX_IN_GT_U32_ipGroupPrefixLen,
    &DX_IN_GT_IPV6ADDR_PTR_ipSrcPtr,
    &DX_IN_GT_U32_ipSrcPrefixLen
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmIpv6McEntryAdd_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_GT_U32_vrId,
    &DX_IN_GT_IPV6ADDR_PTR_ipGroupPtr,
    &DX_IN_GT_U32_ipGroupPrefixLen,
    &DX_IN_GT_IPV6ADDR_PTR_ipSrcPtr,
    &DX_IN_GT_U32_ipSrcPrefixLen,
    &DX_IN_CPSS_DXCH_IP_LTT_ENTRY_STC_PTR_mcRouteLttEntryPtr,
    &DX_IN_GT_BOOL_override,
    &DX_IN_GT_BOOL_defragmentationEnable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmIpv6McEntrySearch_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_GT_U32_vrId,
    &DX_IN_GT_IPV6ADDR_PTR_ipGroupPtr,
    &DX_IN_GT_U32_ipGroupPrefixLen,
    &DX_IN_GT_IPV6ADDR_PTR_ipSrcPtr,
    &DX_IN_GT_U32_ipSrcPrefixLen,
    &DX_OUT_CPSS_DXCH_IP_LTT_ENTRY_STC_PTR_mcRouteLttEntryPtr,
    &DX_OUT_GT_U32_PTR_tcamGroupRowIndexPtr,
    &DX_OUT_GT_U32_PTR_tcamSrcRowIndexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmFcoePrefixGetNext_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_GT_U32_vrId,
    &DX_INOUT_GT_FCID_PTR_fcoeAddrPtr,
    &DX_INOUT_GT_U32_PTR_prefixLenPtr,
    &DX_OUT_CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT_PTR_nextHopInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmIpv4UcPrefixGetNext_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_GT_U32_vrId,
    &DX_INOUT_GT_IPADDR_PTR_ipAddrPtr,
    &DX_INOUT_GT_U32_PTR_prefixLenPtr,
    &DX_OUT_CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT_PTR_nextHopInfoPtr,
    &DX_OUT_GT_U32_PTR_tcamRowIndexPtr,
    &DX_OUT_GT_U32_PTR_tcamColumnIndexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmIpv4McEntryGetNext_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_GT_U32_vrId,
    &DX_INOUT_GT_IPADDR_PTR_ipGroupPtr,
    &DX_INOUT_GT_U32_PTR_ipGroupPrefixLenPtr,
    &DX_INOUT_GT_IPADDR_PTR_ipSrcPtr,
    &DX_INOUT_GT_U32_PTR_ipSrcPrefixLenPtr,
    &DX_OUT_CPSS_DXCH_IP_LTT_ENTRY_STC_PTR_mcRouteLttEntryPtr,
    &DX_OUT_GT_U32_PTR_tcamGroupRowIndexPtr,
    &DX_OUT_GT_U32_PTR_tcamGroupColumnIndexPtr,
    &DX_OUT_GT_U32_PTR_tcamSrcRowIndexPtr,
    &DX_OUT_GT_U32_PTR_tcamSrcColumnIndexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmIpv6UcPrefixGetNext_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_GT_U32_vrId,
    &DX_INOUT_GT_IPV6ADDR_PTR_ipAddrPtr,
    &DX_INOUT_GT_U32_PTR_prefixLenPtr,
    &DX_OUT_CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT_PTR_nextHopInfoPtr,
    &DX_OUT_GT_U32_PTR_tcamRowIndexPtr,
    &DX_OUT_GT_U32_PTR_tcamColumnIndexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmIpv6McEntryGetNext_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_GT_U32_vrId,
    &DX_INOUT_GT_IPV6ADDR_PTR_ipGroupPtr,
    &DX_INOUT_GT_U32_PTR_ipGroupPrefixLenPtr,
    &DX_INOUT_GT_IPV6ADDR_PTR_ipSrcPtr,
    &DX_INOUT_GT_U32_PTR_ipSrcPrefixLenPtr,
    &DX_OUT_CPSS_DXCH_IP_LTT_ENTRY_STC_PTR_mcRouteLttEntryPtr,
    &DX_OUT_GT_U32_PTR_tcamGroupRowIndexPtr,
    &DX_OUT_GT_U32_PTR_tcamSrcRowIndexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmVirtualRouterGet_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_GT_U32_vrId,
    &DX_OUT_CPSS_DXCH_IP_LPM_VR_CONFIG_STC_PTR_vrConfigPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmPrefixesNumberGet_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_GT_U32_vrId,
    &DX_OUT_GT_U32_PTR_ipv4UcPrefixNumberPtr,
    &DX_OUT_GT_U32_PTR_ipv4McPrefixNumberPtr,
    &DX_OUT_GT_U32_PTR_ipv6UcPrefixNumberPtr,
    &DX_OUT_GT_U32_PTR_ipv6McPrefixNumberPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmDBDevsListRemove_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_IN_GT_U8_PTR_devListArr,
    &DX_IN_GT_U32_numOfDevs
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmDBConfigGet_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_OUT_CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT_PTR_shadowTypePtr,
    &DX_OUT_CPSS_IP_PROTOCOL_STACK_ENT_PTR_protocolStackPtr,
    &DX_OUT_CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT_PTR_memoryCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmDBCapacityGet_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_OUT_GT_BOOL_PTR_partitionEnablePtr,
    &DX_OUT_CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC_PTR_indexesRangePtr,
    &DX_OUT_CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC_PTR_tcamLpmManagerCapcityCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmDBExport_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_OUT_GT_U32_PTR_lpmDbMemBlockPtr,
    &DX_INOUT_GT_U32_PTR_lpmDbMemBlockSizePtr,
    &DX_INOUT_GT_UINTPTR_PTR_iterPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmDBMemSizeGet_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDBId,
    &DX_OUT_GT_U32_PTR_lpmDbSizePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmDBDelete_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDbId
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmDbgPatTriePrint_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDbId,
    &DX_IN_GT_U32_vrId,
    &DX_IN_CPSS_IP_PROTOCOL_STACK_ENT_protocol,
    &DX_IN_CPSS_UNICAST_MULTICAST_ENT_prefixType
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmDbgShadowValidityCheck_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDbId,
    &DX_IN_GT_U32_vrId,
    &DX_IN_CPSS_IP_PROTOCOL_STACK_ENT_protocol,
    &DX_IN_CPSS_UNICAST_MULTICAST_ENT_prefixType,
    &DX_IN_GT_BOOL_returnOnFailure
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmIpv4UcPrefixActivityStatusGet_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDbId,
    &DX_IN_GT_U32_vrId,
    &DX_IN_GT_IPADDR_PTR_ipAddrPtr,
    &DX_IN_GT_U32_prefixLen,
    &DX_IN_GT_BOOL_clearActivity,
    &DX_OUT_GT_BOOL_PTR_activityStatusPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmIpv4McEntryActivityStatusGet_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDbId,
    &DX_IN_GT_U32_vrId,
    &DX_IN_GT_IPADDR_PTR_ipGroupPtr,
    &DX_IN_GT_U32_ipGroupPrefixLen,
    &DX_IN_GT_IPADDR_PTR_ipSrcPtr,
    &DX_IN_GT_U32_ipSrcPrefixLen,
    &DX_IN_GT_BOOL_clearActivity,
    &DX_OUT_GT_BOOL_PTR_activityStatusPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmIpv6UcPrefixActivityStatusGet_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDbId,
    &DX_IN_GT_U32_vrId,
    &DX_IN_GT_IPV6ADDR_PTR_ipAddrPtr,
    &DX_IN_GT_U32_prefixLen,
    &DX_IN_GT_BOOL_clearActivity,
    &DX_OUT_GT_BOOL_PTR_activityStatusPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmIpv6McEntryActivityStatusGet_PARAMS[] =  {
    &DX_IN_GT_U32_lpmDbId,
    &DX_IN_GT_U32_vrId,
    &DX_IN_GT_IPV6ADDR_PTR_ipGroupPtr,
    &DX_IN_GT_U32_ipGroupPrefixLen,
    &DX_IN_GT_IPV6ADDR_PTR_ipSrcPtr,
    &DX_IN_GT_U32_ipSrcPrefixLen,
    &DX_IN_GT_BOOL_clearActivity,
    &DX_OUT_GT_BOOL_PTR_activityStatusPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmActivityBitEnableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_BOOL_activityBit
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmDbgHwShadowSyncValidityCheck_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_lpmDbId,
    &DX_IN_GT_U32_vrId,
    &DX_IN_CPSS_IP_PROTOCOL_STACK_ENT_protocol,
    &DX_IN_CPSS_UNICAST_MULTICAST_ENT_prefixType,
    &DX_IN_GT_BOOL_returnOnFailure
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmDbgDump_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_vrId,
    &DX_IN_CPSS_IP_PROTOCOL_STACK_ENT_protocol,
    &DX_IN_CPSS_UNICAST_MULTICAST_ENT_prefixType
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmActivityBitEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_BOOL_PTR_activityBitPtr
};
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmDBDevListAdd_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmDBDevListGet_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmPolicyBasedRoutingDefaultMcGet_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpLpmPolicyBasedRoutingDefaultMcSet_PARAMS[];


/********* lib API DB *********/

extern void cpssDxChIpLpmDBCreate_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChIpLpmDBConfigGet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChIpLpmDBDevListAdd_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChIpLpmDBDevListAdd_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChIpLpmDBDevListGet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChIpLpmPolicyBasedRoutingDefaultMcSet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChIpLpmPolicyBasedRoutingDefaultMcGet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChIpLpmIpv4UcPrefixAdd_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChIpLpmIpv4UcPrefixAdd_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChIpLpmIpv4UcPrefixAdd_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChIpLpmIpv4UcPrefixAdd_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChIpLpmIpv4UcPrefixAddBulk_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChIpLpmIpv4UcPrefixAddBulk_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChIpLpmIpv4UcPrefixAdd_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChIpLpmIpv4UcPrefixAdd_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChIpLpmIpv4UcPrefixAdd_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChIpLpmIpv4UcPrefixAdd_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChIpLpmIpv4UcPrefixAddBulk_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChIpLpmIpv4UcPrefixAddBulk_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChIpLpmIpv4UcPrefixAdd_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChIpLpmIpv4UcPrefixAdd_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChIpLpmIpv4UcPrefixAdd_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChIpLpmEngineLogLibDb[] = {
    {"cpssDxChIpLpmDBCreate", 4, cpssDxChIpLpmDBCreate_PARAMS, cpssDxChIpLpmDBCreate_preLogic},
    {"cpssDxChIpLpmDBDelete", 1, cpssDxChIpLpmDBDelete_PARAMS, NULL},
    {"cpssDxChIpLpmDBConfigGet", 4, cpssDxChIpLpmDBConfigGet_PARAMS, cpssDxChIpLpmDBConfigGet_preLogic},
    {"cpssDxChIpLpmDBCapacityUpdate", 3, cpssDxChIpLpmDBCapacityUpdate_PARAMS, NULL},
    {"cpssDxChIpLpmDBCapacityGet", 4, cpssDxChIpLpmDBCapacityGet_PARAMS, NULL},
    {"cpssDxChIpLpmDBDevListAdd", 3, cpssDxChIpLpmDBDevListAdd_PARAMS, cpssDxChIpLpmDBDevListAdd_preLogic},
    {"cpssDxChIpLpmDBDevsListRemove", 3, cpssDxChIpLpmDBDevsListRemove_PARAMS, cpssDxChIpLpmDBDevListAdd_preLogic},
    {"cpssDxChIpLpmDBDevListGet", 3, cpssDxChIpLpmDBDevListGet_PARAMS, cpssDxChIpLpmDBDevListGet_preLogic},
    {"cpssDxChIpLpmPolicyBasedRoutingDefaultMcSet", 3, cpssDxChIpLpmPolicyBasedRoutingDefaultMcSet_PARAMS, cpssDxChIpLpmPolicyBasedRoutingDefaultMcSet_preLogic},
    {"cpssDxChIpLpmPolicyBasedRoutingDefaultMcGet", 9, cpssDxChIpLpmPolicyBasedRoutingDefaultMcGet_PARAMS, cpssDxChIpLpmPolicyBasedRoutingDefaultMcGet_preLogic},
    {"cpssDxChIpLpmVirtualRouterAdd", 3, cpssDxChIpLpmVirtualRouterAdd_PARAMS, cpssDxChIpLpmIpv4UcPrefixAdd_preLogic},
    {"cpssDxChIpLpmVirtualRouterSharedAdd", 3, cpssDxChIpLpmVirtualRouterAdd_PARAMS, cpssDxChIpLpmIpv4UcPrefixAdd_preLogic},
    {"cpssDxChIpLpmVirtualRouterGet", 3, cpssDxChIpLpmVirtualRouterGet_PARAMS, cpssDxChIpLpmIpv4UcPrefixAdd_preLogic},
    {"cpssDxChIpLpmVirtualRouterDel", 2, cpssDxChIpLpmVirtualRouterDel_PARAMS, NULL},
    {"cpssDxChIpLpmIpv4UcPrefixAdd", 7, cpssDxChIpLpmIpv4UcPrefixAdd_PARAMS, cpssDxChIpLpmIpv4UcPrefixAdd_preLogic},
    {"cpssDxChIpLpmIpv4UcPrefixAddBulk", 3, cpssDxChIpLpmIpv4UcPrefixAddBulk_PARAMS, cpssDxChIpLpmIpv4UcPrefixAddBulk_preLogic},
    {"cpssDxChIpLpmIpv4UcPrefixDel", 4, cpssDxChIpLpmIpv4UcPrefixDel_PARAMS, NULL},
    {"cpssDxChIpLpmIpv4UcPrefixDelBulk", 3, cpssDxChIpLpmIpv4UcPrefixAddBulk_PARAMS, cpssDxChIpLpmIpv4UcPrefixAddBulk_preLogic},
    {"cpssDxChIpLpmIpv4UcPrefixesFlush", 2, cpssDxChIpLpmVirtualRouterDel_PARAMS, NULL},
    {"cpssDxChIpLpmIpv4UcPrefixSearch", 7, cpssDxChIpLpmIpv4UcPrefixSearch_PARAMS, cpssDxChIpLpmIpv4UcPrefixAdd_preLogic},
    {"cpssDxChIpLpmIpv4UcPrefixGetNext", 7, cpssDxChIpLpmIpv4UcPrefixGetNext_PARAMS, cpssDxChIpLpmIpv4UcPrefixAdd_preLogic},
    {"cpssDxChIpLpmIpv4UcPrefixGet", 7, cpssDxChIpLpmIpv4UcPrefixGet_PARAMS, cpssDxChIpLpmIpv4UcPrefixAdd_preLogic},
    {"cpssDxChIpLpmIpv4McEntryAdd", 9, cpssDxChIpLpmIpv4McEntryAdd_PARAMS, NULL},
    {"cpssDxChIpLpmIpv4McEntryDel", 6, cpssDxChIpLpmIpv4McEntryDel_PARAMS, NULL},
    {"cpssDxChIpLpmIpv4McEntriesFlush", 2, cpssDxChIpLpmVirtualRouterDel_PARAMS, NULL},
    {"cpssDxChIpLpmIpv4McEntryGetNext", 11, cpssDxChIpLpmIpv4McEntryGetNext_PARAMS, NULL},
    {"cpssDxChIpLpmIpv4McEntrySearch", 11, cpssDxChIpLpmIpv4McEntrySearch_PARAMS, NULL},
    {"cpssDxChIpLpmIpv6UcPrefixAdd", 7, cpssDxChIpLpmIpv6UcPrefixAdd_PARAMS, cpssDxChIpLpmIpv4UcPrefixAdd_preLogic},
    {"cpssDxChIpLpmIpv6UcPrefixAddBulk", 4, cpssDxChIpLpmIpv6UcPrefixAddBulk_PARAMS, cpssDxChIpLpmIpv4UcPrefixAddBulk_preLogic},
    {"cpssDxChIpLpmIpv6UcPrefixDel", 4, cpssDxChIpLpmIpv6UcPrefixDel_PARAMS, NULL},
    {"cpssDxChIpLpmIpv6UcPrefixDelBulk", 3, cpssDxChIpLpmIpv6UcPrefixDelBulk_PARAMS, cpssDxChIpLpmIpv4UcPrefixAddBulk_preLogic},
    {"cpssDxChIpLpmIpv6UcPrefixesFlush", 2, cpssDxChIpLpmVirtualRouterDel_PARAMS, NULL},
    {"cpssDxChIpLpmIpv6UcPrefixSearch", 7, cpssDxChIpLpmIpv6UcPrefixSearch_PARAMS, cpssDxChIpLpmIpv4UcPrefixAdd_preLogic},
    {"cpssDxChIpLpmIpv6UcPrefixGetNext", 7, cpssDxChIpLpmIpv6UcPrefixGetNext_PARAMS, cpssDxChIpLpmIpv4UcPrefixAdd_preLogic},
    {"cpssDxChIpLpmIpv6UcPrefixGet", 7, cpssDxChIpLpmIpv6UcPrefixGet_PARAMS, cpssDxChIpLpmIpv4UcPrefixAdd_preLogic},
    {"cpssDxChIpLpmIpv6McEntryAdd", 9, cpssDxChIpLpmIpv6McEntryAdd_PARAMS, NULL},
    {"cpssDxChIpLpmIpv6McEntryDel", 6, cpssDxChIpLpmIpv6McEntryDel_PARAMS, NULL},
    {"cpssDxChIpLpmIpv6McEntriesFlush", 2, cpssDxChIpLpmVirtualRouterDel_PARAMS, NULL},
    {"cpssDxChIpLpmIpv6McEntryGetNext", 9, cpssDxChIpLpmIpv6McEntryGetNext_PARAMS, NULL},
    {"cpssDxChIpLpmIpv6McEntrySearch", 9, cpssDxChIpLpmIpv6McEntrySearch_PARAMS, NULL},
    {"cpssDxChIpLpmFcoePrefixAdd", 7, cpssDxChIpLpmFcoePrefixAdd_PARAMS, NULL},
    {"cpssDxChIpLpmFcoePrefixDel", 4, cpssDxChIpLpmFcoePrefixDel_PARAMS, NULL},
    {"cpssDxChIpLpmFcoePrefixAddBulk", 3, cpssDxChIpLpmFcoePrefixAddBulk_PARAMS, NULL},
    {"cpssDxChIpLpmFcoePrefixDelBulk", 3, cpssDxChIpLpmFcoePrefixAddBulk_PARAMS, NULL},
    {"cpssDxChIpLpmFcoePrefixesFlush", 2, cpssDxChIpLpmVirtualRouterDel_PARAMS, NULL},
    {"cpssDxChIpLpmFcoePrefixSearch", 5, cpssDxChIpLpmFcoePrefixSearch_PARAMS, NULL},
    {"cpssDxChIpLpmFcoePrefixGet", 5, cpssDxChIpLpmFcoePrefixGet_PARAMS, NULL},
    {"cpssDxChIpLpmFcoePrefixGetNext", 5, cpssDxChIpLpmFcoePrefixGetNext_PARAMS, NULL},
    {"cpssDxChIpLpmDBMemSizeGet", 2, cpssDxChIpLpmDBMemSizeGet_PARAMS, NULL},
    {"cpssDxChIpLpmDBExport", 4, cpssDxChIpLpmDBExport_PARAMS, NULL},
    {"cpssDxChIpLpmDBImport", 4, cpssDxChIpLpmDBImport_PARAMS, NULL},
    {"cpssDxChIpLpmActivityBitEnableGet", 2, cpssDxChIpLpmActivityBitEnableGet_PARAMS, NULL},
    {"cpssDxChIpLpmActivityBitEnableSet", 2, cpssDxChIpLpmActivityBitEnableSet_PARAMS, NULL},
    {"cpssDxChIpLpmIpv4UcPrefixActivityStatusGet", 6, cpssDxChIpLpmIpv4UcPrefixActivityStatusGet_PARAMS, NULL},
    {"cpssDxChIpLpmIpv6UcPrefixActivityStatusGet", 6, cpssDxChIpLpmIpv6UcPrefixActivityStatusGet_PARAMS, NULL},
    {"cpssDxChIpLpmIpv4McEntryActivityStatusGet", 8, cpssDxChIpLpmIpv4McEntryActivityStatusGet_PARAMS, NULL},
    {"cpssDxChIpLpmIpv6McEntryActivityStatusGet", 8, cpssDxChIpLpmIpv6McEntryActivityStatusGet_PARAMS, NULL},
    {"cpssDxChIpLpmPrefixesNumberGet", 6, cpssDxChIpLpmPrefixesNumberGet_PARAMS, NULL},
    {"cpssDxChIpLpmDbgRouteTcamDump", 1, cpssDxChIpLpmDbgRouteTcamDump_PARAMS, NULL},
    {"cpssDxChIpLpmDbgPatTriePrint", 4, cpssDxChIpLpmDbgPatTriePrint_PARAMS, NULL},
    {"cpssDxChIpLpmDbgPatTrieValidityCheck", 4, cpssDxChIpLpmDbgPatTrieValidityCheck_PARAMS, NULL},
    {"cpssDxChIpLpmDbgDump", 4, cpssDxChIpLpmDbgDump_PARAMS, NULL},
    {"cpssDxChIpLpmDbgHwMemPrint", 1, cpssDxChIpLpmDBDelete_PARAMS, NULL},
    {"cpssDxChIpLpmDbgHwValidation", 4, cpssDxChIpLpmDbgDump_PARAMS, NULL},
    {"cpssDxChIpLpmDbgShadowValidityCheck", 5, cpssDxChIpLpmDbgShadowValidityCheck_PARAMS, NULL},
    {"cpssDxChIpLpmDbgHwShadowSyncValidityCheck", 6, cpssDxChIpLpmDbgHwShadowSyncValidityCheck_PARAMS, NULL},
    {"cpssDxChIpLpmDbgHwShadowValidityAndSyncCheck", 6, cpssDxChIpLpmDbgHwShadowSyncValidityCheck_PARAMS, NULL},
    {"cpssDxChIpLpmDbgHwOctetPerBlockPrint", 1, cpssDxChIpLpmDBDelete_PARAMS, NULL},
    {"cpssDxChIpLpmDbgHwBlockInfoPrint", 1, cpssDxChIpLpmDBDelete_PARAMS, NULL},
    {"cpssDxChIpLpmDbgHwOctetPerProtocolPrint", 1, cpssDxChIpLpmDBDelete_PARAMS, NULL},
    {"cpssDxChIpLpmDbgHwOctetPerProtocolPrintLpmLinesCounters", 1, cpssDxChIpLpmDBDelete_PARAMS, NULL},
    {"cpssDxChIpLpmDbgHwOctetPerProtocolPrintLpmMemoryAndCounters", 1, cpssDxChIpLpmDBDelete_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_IP_LPM(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChIpLpmEngineLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChIpLpmEngineLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

