/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChCosLog.c
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
#include <cpss/dxCh/dxChxGen/cos/cpssDxChCos.h>
#include <cpss/dxCh/dxChxGen/cos/private/prvCpssDxChCosLog.h>
#include <cpss/generic/bridge/private/prvCpssGenBrgLog.h>
#include <cpss/generic/cos/private/prvCpssGenCosLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssGenDbLog.h>


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_COS_PROFILE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_COS_PROFILE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, dropPrecedence, CPSS_DP_LEVEL_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, userPriority);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, trafficClass);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dscp);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, exp);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_COS_PROFILE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_COS_PROFILE_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_COS_PROFILE_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_COS_PROFILE_STC_PTR_cosPtr = {
     "cosPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_COS_PROFILE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_MAC_QOS_RESOLVE_ENT_macQosResolvType = {
     "macQosResolvType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_MAC_QOS_RESOLVE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_QOS_ENTRY_STC_PTR_macQosCfgPtr = {
     "macQosCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_QOS_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_QOS_ENTRY_STC_PTR_portQosCfgPtr = {
     "portQosCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_QOS_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_QOS_PORT_TRUST_MODE_ENT_portQosTrustMode = {
     "portQosTrustMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_QOS_PORT_TRUST_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_VLAN_TAG_TYPE_ENT_vlanTagType = {
     "vlanTagType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_VLAN_TAG_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_enableDscpMutation = {
     "enableDscpMutation", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_isDefaultVlanTagType = {
     "isDefaultVlanTagType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_upOverrideEnable = {
     "upOverrideEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_useUpAsIndex = {
     "useUpAsIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_dscp = {
     "dscp", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_entryIdx = {
     "entryIdx", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_mappingTableIndex = {
     "mappingTableIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_newDscp = {
     "newDscp", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_newExp = {
     "newExp", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_newUp = {
     "newUp", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_upProfileIndex = {
     "upProfileIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_cfiDeiBit = {
     "cfiDeiBit", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_defaultUserPrio = {
     "defaultUserPrio", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_dscp = {
     "dscp", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_newDscp = {
     "newDscp", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_COS_PROFILE_STC_PTR_cosPtr = {
     "cosPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_COS_PROFILE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_MAC_QOS_RESOLVE_ENT_PTR_macQosResolvTypePtr = {
     "macQosResolvTypePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_MAC_QOS_RESOLVE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_QOS_ENTRY_STC_PTR_macQosCfgPtr = {
     "macQosCfgPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_QOS_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_QOS_ENTRY_STC_PTR_portQosCfgPtr = {
     "portQosCfgPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_QOS_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_QOS_PORT_TRUST_MODE_ENT_PTR_portQosTrustModePtr = {
     "portQosTrustModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_QOS_PORT_TRUST_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_VLAN_TAG_TYPE_ENT_PTR_vlanTagTypePtr = {
     "vlanTagTypePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_VLAN_TAG_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_enableDscpMutationPtr = {
     "enableDscpMutationPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_upOverrideEnablePtr = {
     "upOverrideEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_useUpAsIndexPtr = {
     "useUpAsIndexPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_dscpPtr = {
     "dscpPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_entryPtr = {
     "entryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_mappingTableIndexPtr = {
     "mappingTableIndexPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_newDscpPtr = {
     "newDscpPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_newExpPtr = {
     "newExpPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_newUpPtr = {
     "newUpPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_upProfileIndexPtr = {
     "upProfileIndexPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_cfiDeiBitPtr = {
     "cfiDeiBitPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_defaultUserPrioPtr = {
     "defaultUserPrioPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_newDscpPtr = {
     "newDscpPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U8)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosDpToCfiDeiMapSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DP_LEVEL_ENT_dp,
    &DX_IN_GT_U8_cfiDeiBit
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosDpToCfiDeiMapGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DP_LEVEL_ENT_dp,
    &DX_OUT_GT_U8_PTR_cfiDeiBitPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosMacQosConflictResolutionSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_MAC_QOS_RESOLVE_ENT_macQosResolvType
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosPortUpProfileIndexSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U8_up,
    &DX_IN_GT_U32_upProfileIndex
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosPortUpProfileIndexGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U8_up,
    &DX_OUT_GT_U32_PTR_upProfileIndexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosPortVlanQoSCfgEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_port,
    &DX_OUT_GT_U32_PTR_entryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosPortQosConfigSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_CPSS_QOS_ENTRY_STC_PTR_portQosCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosPortQosTrustModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_CPSS_QOS_PORT_TRUST_MODE_ENT_portQosTrustMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosPortReMapDSCPSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_GT_BOOL_enableDscpMutation
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosL2TrustModeVlanTagSelectSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_GT_BOOL_isDefaultVlanTagType,
    &DX_IN_CPSS_VLAN_TAG_TYPE_ENT_vlanTagType
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosL2TrustModeVlanTagSelectGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_GT_BOOL_isDefaultVlanTagType,
    &DX_OUT_CPSS_VLAN_TAG_TYPE_ENT_PTR_vlanTagTypePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPortModifyUPSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_GT_BOOL_upOverrideEnable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosPortTrustQosMappingTableIndexSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_GT_BOOL_useUpAsIndex,
    &DX_IN_GT_U32_mappingTableIndex
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosPortEgressQosMappingTableIndexSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_GT_U32_mappingTableIndex
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPortDefaultUPSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_GT_U8_defaultUserPrio
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosPortQosConfigGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_OUT_CPSS_QOS_ENTRY_STC_PTR_portQosCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosPortQosTrustModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_OUT_CPSS_QOS_PORT_TRUST_MODE_ENT_PTR_portQosTrustModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosPortReMapDSCPGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_OUT_GT_BOOL_PTR_enableDscpMutationPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPortModifyUPGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_OUT_GT_BOOL_PTR_upOverrideEnablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosPortTrustQosMappingTableIndexGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_OUT_GT_BOOL_PTR_useUpAsIndexPtr,
    &DX_OUT_GT_U32_PTR_mappingTableIndexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosPortEgressQosMappingTableIndexGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_OUT_GT_U32_PTR_mappingTableIndexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPortDefaultUPGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_OUT_GT_U8_PTR_defaultUserPrioPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosMacQosEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_entryIdx,
    &DX_IN_CPSS_QOS_ENTRY_STC_PTR_macQosCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosMacQosEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_entryIdx,
    &DX_OUT_CPSS_QOS_ENTRY_STC_PTR_macQosCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosQoSProfileEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_OUT_GT_U32_PTR_entryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosEgressDscp2DscpMappingEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_mappingTableIndex,
    &DX_IN_GT_U32_dscp,
    &DX_IN_GT_U32_newDscp
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosEgressDscp2DscpMappingEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_mappingTableIndex,
    &DX_IN_GT_U32_dscp,
    &DX_OUT_GT_U32_PTR_newDscpPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosEgressExp2ExpMappingEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_mappingTableIndex,
    &DX_IN_GT_U32_exp,
    &DX_IN_GT_U32_newExp
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosExpToProfileMapSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_mappingTableIndex,
    &DX_IN_GT_U32_exp,
    &DX_IN_GT_U32_profileIndex
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosEgressExp2ExpMappingEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_mappingTableIndex,
    &DX_IN_GT_U32_exp,
    &DX_OUT_GT_U32_PTR_newExpPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosExpToProfileMapGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_mappingTableIndex,
    &DX_IN_GT_U32_exp,
    &DX_OUT_GT_U32_PTR_profileIndexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosEgressTcDp2UpExpDscpMappingEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_mappingTableIndex,
    &DX_IN_GT_U32_tc,
    &DX_IN_CPSS_DP_LEVEL_ENT_dp,
    &DX_IN_GT_U32_up,
    &DX_IN_GT_U32_exp,
    &DX_IN_GT_U32_dscp
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosEgressTcDp2UpExpDscpMappingEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_mappingTableIndex,
    &DX_IN_GT_U32_tc,
    &DX_IN_CPSS_DP_LEVEL_ENT_dp,
    &DX_OUT_GT_U32_PTR_upPtr,
    &DX_OUT_GT_U32_PTR_expPtr,
    &DX_OUT_GT_U32_PTR_dscpPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosEgressUp2UpMappingEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_mappingTableIndex,
    &DX_IN_GT_U32_up,
    &DX_IN_GT_U32_newUp
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosEgressUp2UpMappingEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_mappingTableIndex,
    &DX_IN_GT_U32_up,
    &DX_OUT_GT_U32_PTR_newUpPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosUpCfiDeiToProfileMapSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_mappingTableIndex,
    &DX_IN_GT_U32_upProfileIndex,
    &DX_IN_GT_U8_up,
    &DX_IN_GT_U8_cfiDeiBit,
    &DX_IN_GT_U32_profileIndex
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosUpCfiDeiToProfileMapGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_mappingTableIndex,
    &DX_IN_GT_U32_upProfileIndex,
    &DX_IN_GT_U8_up,
    &DX_IN_GT_U8_cfiDeiBit,
    &DX_OUT_GT_U32_PTR_profileIndexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosDscpToProfileMapSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_mappingTableIndex,
    &DX_IN_GT_U8_dscp,
    &DX_IN_GT_U32_profileIndex
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosDscpMutationEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_mappingTableIndex,
    &DX_IN_GT_U8_dscp,
    &DX_IN_GT_U8_newDscp
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosDscpToProfileMapGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_mappingTableIndex,
    &DX_IN_GT_U8_dscp,
    &DX_OUT_GT_U32_PTR_profileIndexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosDscpMutationEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_mappingTableIndex,
    &DX_IN_GT_U8_dscp,
    &DX_OUT_GT_U8_PTR_newDscpPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosProfileEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_profileIndex,
    &DX_IN_CPSS_DXCH_COS_PROFILE_STC_PTR_cosPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosProfileEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_profileIndex,
    &DX_OUT_CPSS_DXCH_COS_PROFILE_STC_PTR_cosPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCosMacQosConflictResolutionGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_MAC_QOS_RESOLVE_ENT_PTR_macQosResolvTypePtr
};


/********* lib API DB *********/

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChCosLogLibDb[] = {
    {"cpssDxChCosProfileEntrySet", 3, cpssDxChCosProfileEntrySet_PARAMS, NULL},
    {"cpssDxChCosProfileEntryGet", 3, cpssDxChCosProfileEntryGet_PARAMS, NULL},
    {"cpssDxChCosDscpToProfileMapSet", 4, cpssDxChCosDscpToProfileMapSet_PARAMS, NULL},
    {"cpssDxChCosDscpToProfileMapGet", 4, cpssDxChCosDscpToProfileMapGet_PARAMS, NULL},
    {"cpssDxChCosDscpMutationEntrySet", 4, cpssDxChCosDscpMutationEntrySet_PARAMS, NULL},
    {"cpssDxChCosDscpMutationEntryGet", 4, cpssDxChCosDscpMutationEntryGet_PARAMS, NULL},
    {"cpssDxChCosUpCfiDeiToProfileMapSet", 6, cpssDxChCosUpCfiDeiToProfileMapSet_PARAMS, NULL},
    {"cpssDxChCosUpCfiDeiToProfileMapGet", 6, cpssDxChCosUpCfiDeiToProfileMapGet_PARAMS, NULL},
    {"cpssDxChCosPortDpToCfiDeiMapEnableSet", 3, prvCpssLogGenDevNumPortNumEnable2_PARAMS, NULL},
    {"cpssDxChCosPortDpToCfiDeiMapEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr2_PARAMS, NULL},
    {"cpssDxChCosPortDpToCfiDei1MapEnableSet", 3, prvCpssLogGenDevNumPortNumEnable2_PARAMS, NULL},
    {"cpssDxChCosPortDpToCfiDei1MapEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr2_PARAMS, NULL},
    {"cpssDxChCosDpToCfiDeiMapSet", 3, cpssDxChCosDpToCfiDeiMapSet_PARAMS, NULL},
    {"cpssDxChCosDpToCfiDeiMapGet", 3, cpssDxChCosDpToCfiDeiMapGet_PARAMS, NULL},
    {"cpssDxChCosExpToProfileMapSet", 4, cpssDxChCosExpToProfileMapSet_PARAMS, NULL},
    {"cpssDxChCosExpToProfileMapGet", 4, cpssDxChCosExpToProfileMapGet_PARAMS, NULL},
    {"cpssDxChCosPortQosConfigSet", 3, cpssDxChCosPortQosConfigSet_PARAMS, NULL},
    {"cpssDxChCosPortQosConfigGet", 3, cpssDxChCosPortQosConfigGet_PARAMS, NULL},
    {"cpssDxChCosMacQosEntrySet", 3, cpssDxChCosMacQosEntrySet_PARAMS, NULL},
    {"cpssDxChCosMacQosEntryGet", 3, cpssDxChCosMacQosEntryGet_PARAMS, NULL},
    {"cpssDxChCosMacQosConflictResolutionSet", 2, cpssDxChCosMacQosConflictResolutionSet_PARAMS, NULL},
    {"cpssDxChCosMacQosConflictResolutionGet", 2, cpssDxChCosMacQosConflictResolutionGet_PARAMS, NULL},
    {"cpssDxChPortDefaultUPSet", 3, cpssDxChPortDefaultUPSet_PARAMS, NULL},
    {"cpssDxChPortDefaultUPGet", 3, cpssDxChPortDefaultUPGet_PARAMS, NULL},
    {"cpssDxChPortModifyUPSet", 3, cpssDxChPortModifyUPSet_PARAMS, NULL},
    {"cpssDxChPortModifyUPGet", 3, cpssDxChPortModifyUPGet_PARAMS, NULL},
    {"cpssDxChCosPortQosTrustModeSet", 3, cpssDxChCosPortQosTrustModeSet_PARAMS, NULL},
    {"cpssDxChCosPortQosTrustModeGet", 3, cpssDxChCosPortQosTrustModeGet_PARAMS, NULL},
    {"cpssDxChCosPortReMapDSCPSet", 3, cpssDxChCosPortReMapDSCPSet_PARAMS, NULL},
    {"cpssDxChCosPortReMapDSCPGet", 3, cpssDxChCosPortReMapDSCPGet_PARAMS, NULL},
    {"cpssDxChCosPortVlanQoSCfgEntryGet", 3, cpssDxChCosPortVlanQoSCfgEntryGet_PARAMS, NULL},
    {"cpssDxChCosQoSProfileEntryGet", 3, cpssDxChCosQoSProfileEntryGet_PARAMS, NULL},
    {"cpssDxChCosTrustDsaTagQosModeSet", 3, prvCpssLogGenDevNumPortNumEnable2_PARAMS, NULL},
    {"cpssDxChCosTrustDsaTagQosModeGet", 3, prvCpssLogGenDevNumPortNumEnablePtr2_PARAMS, NULL},
    {"cpssDxChCosTrustExpModeSet", 3, prvCpssLogGenDevNumPortNumEnable2_PARAMS, NULL},
    {"cpssDxChCosTrustExpModeGet", 3, prvCpssLogGenDevNumPortNumEnablePtr2_PARAMS, NULL},
    {"cpssDxChCosL2TrustModeVlanTagSelectSet", 4, cpssDxChCosL2TrustModeVlanTagSelectSet_PARAMS, NULL},
    {"cpssDxChCosL2TrustModeVlanTagSelectGet", 4, cpssDxChCosL2TrustModeVlanTagSelectGet_PARAMS, NULL},
    {"cpssDxChCosPortUpProfileIndexSet", 4, cpssDxChCosPortUpProfileIndexSet_PARAMS, NULL},
    {"cpssDxChCosPortUpProfileIndexGet", 4, cpssDxChCosPortUpProfileIndexGet_PARAMS, NULL},
    {"cpssDxChCosPortTrustQosMappingTableIndexSet", 4, cpssDxChCosPortTrustQosMappingTableIndexSet_PARAMS, NULL},
    {"cpssDxChCosPortTrustQosMappingTableIndexGet", 4, cpssDxChCosPortTrustQosMappingTableIndexGet_PARAMS, NULL},
    {"cpssDxChCosPortEgressQosMappingTableIndexSet", 3, cpssDxChCosPortEgressQosMappingTableIndexSet_PARAMS, NULL},
    {"cpssDxChCosPortEgressQosMappingTableIndexGet", 3, cpssDxChCosPortEgressQosMappingTableIndexGet_PARAMS, NULL},
    {"cpssDxChCosPortEgressQosExpMappingEnableSet", 3, prvCpssLogGenDevNumPortNumEnable2_PARAMS, NULL},
    {"cpssDxChCosPortEgressQosExpMappingEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr2_PARAMS, NULL},
    {"cpssDxChCosPortEgressQosTcDpMappingEnableSet", 3, prvCpssLogGenDevNumPortNumEnable2_PARAMS, NULL},
    {"cpssDxChCosPortEgressQosTcDpMappingEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr2_PARAMS, NULL},
    {"cpssDxChCosPortEgressQosUpMappingEnableSet", 3, prvCpssLogGenDevNumPortNumEnable2_PARAMS, NULL},
    {"cpssDxChCosPortEgressQosUpMappingEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr2_PARAMS, NULL},
    {"cpssDxChCosPortEgressQosDscpMappingEnableSet", 3, prvCpssLogGenDevNumPortNumEnable2_PARAMS, NULL},
    {"cpssDxChCosPortEgressQosDscpMappingEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr2_PARAMS, NULL},
    {"cpssDxChCosEgressDscp2DscpMappingEntrySet", 4, cpssDxChCosEgressDscp2DscpMappingEntrySet_PARAMS, NULL},
    {"cpssDxChCosEgressDscp2DscpMappingEntryGet", 4, cpssDxChCosEgressDscp2DscpMappingEntryGet_PARAMS, NULL},
    {"cpssDxChCosEgressExp2ExpMappingEntrySet", 4, cpssDxChCosEgressExp2ExpMappingEntrySet_PARAMS, NULL},
    {"cpssDxChCosEgressExp2ExpMappingEntryGet", 4, cpssDxChCosEgressExp2ExpMappingEntryGet_PARAMS, NULL},
    {"cpssDxChCosEgressUp2UpMappingEntrySet", 4, cpssDxChCosEgressUp2UpMappingEntrySet_PARAMS, NULL},
    {"cpssDxChCosEgressUp2UpMappingEntryGet", 4, cpssDxChCosEgressUp2UpMappingEntryGet_PARAMS, NULL},
    {"cpssDxChCosEgressTcDp2UpExpDscpMappingEntrySet", 7, cpssDxChCosEgressTcDp2UpExpDscpMappingEntrySet_PARAMS, NULL},
    {"cpssDxChCosEgressTcDp2UpExpDscpMappingEntryGet", 7, cpssDxChCosEgressTcDp2UpExpDscpMappingEntryGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_COS(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChCosLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChCosLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

