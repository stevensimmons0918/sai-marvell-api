/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChNstLog.c
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
#include <cpss/dxCh/dxChxGen/nst/cpssDxChNstPortIsolation.h>
#include <cpss/dxCh/dxChxGen/nst/private/prvCpssDxChNstLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssGenDbLog.h>
#include <cpss/generic/nst/private/prvCpssGenNstLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_ENT[]  =
{
    "CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_PHYSICAL_PORT_E",
    "CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_EPORT_E",
    "CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_EVLAN_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_NST_PORT_ISOLATION_MODE_ENT[]  =
{
    "CPSS_DXCH_NST_PORT_ISOLATION_DISABLE_E",
    "CPSS_DXCH_NST_PORT_ISOLATION_L2_ENABLE_E",
    "CPSS_DXCH_NST_PORT_ISOLATION_L3_ENABLE_E",
    "CPSS_DXCH_NST_PORT_ISOLATION_ALL_ENABLE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_NST_PORT_ISOLATION_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT[]  =
{
    "CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L2_E",
    "CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_L3_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_NST_PORT_ISOLATION_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_NST_PORT_ISOLATION_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_NST_PORT_ISOLATION_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_NST_PORT_ISOLATION_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_NST_PORT_ISOLATION_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_NST_PORT_ISOLATION_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_ENT_mode = {
     "mode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_NST_PORT_ISOLATION_MODE_ENT_mode = {
     "mode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_NST_PORT_ISOLATION_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT_trafficType = {
     "trafficType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_INTERFACE_INFO_STC_PTR_srcInterfacePtr = {
     "srcInterfacePtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_INTERFACE_INFO_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_NST_AM_PARAM_ENT_paramType = {
     "paramType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_NST_AM_PARAM_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_NST_CHECK_ENT_checkType = {
     "checkType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_NST_CHECK_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_NST_EGRESS_FRW_FILTER_ENT_filterType = {
     "filterType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_NST_EGRESS_FRW_FILTER_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_NST_INGRESS_FRW_FILTER_ENT_filterType = {
     "filterType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_NST_INGRESS_FRW_FILTER_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORTS_BMP_STC_PTR_localPortsMembersPtr = {
     "localPortsMembersPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORTS_BMP_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_cpuPortMember = {
     "cpuPortMember", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_daAccessLevel = {
     "daAccessLevel", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_dipAccessLevel = {
     "dipAccessLevel", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ingressCnt = {
     "ingressCnt", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numberOfDeviceBits = {
     "numberOfDeviceBits", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numberOfPortBits = {
     "numberOfPortBits", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numberOfTrunkBits = {
     "numberOfTrunkBits", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_saAccessLevel = {
     "saAccessLevel", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_sipAccessLevel = {
     "sipAccessLevel", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_srcIdBitLocation = {
     "srcIdBitLocation", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_trunkIndexBase = {
     "trunkIndexBase", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_ENT_PTR_modePtr = {
     "modePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_NST_PORT_ISOLATION_MODE_ENT_PTR_modePtr = {
     "modePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_NST_PORT_ISOLATION_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORTS_BMP_STC_PTR_localPortsMembersPtr = {
     "localPortsMembersPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORTS_BMP_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_cpuPortMemberPtr = {
     "cpuPortMemberPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_ingressCntPtr = {
     "ingressCntPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_numberOfDeviceBitsPtr = {
     "numberOfDeviceBitsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_numberOfPortBitsPtr = {
     "numberOfPortBitsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_numberOfTrunkBitsPtr = {
     "numberOfTrunkBitsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_srcIdBitLocationPtr = {
     "srcIdBitLocationPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_trunkIndexBasePtr = {
     "trunkIndexBasePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNstPortIsolationIndexingModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_ENT_mode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNstPortIsolationTableEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT_trafficType,
    &DX_IN_CPSS_INTERFACE_INFO_STC_PTR_srcInterfacePtr,
    &DX_IN_GT_BOOL_cpuPortMember,
    &DX_IN_CPSS_PORTS_BMP_STC_PTR_localPortsMembersPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNstPortIsolationPortAdd_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT_trafficType,
    &DX_IN_CPSS_INTERFACE_INFO_STC_PTR_srcInterfacePtr,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNstPortIsolationTableEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT_trafficType,
    &DX_IN_CPSS_INTERFACE_INFO_STC_PTR_srcInterfacePtr,
    &DX_OUT_GT_BOOL_PTR_cpuPortMemberPtr,
    &DX_OUT_CPSS_PORTS_BMP_STC_PTR_localPortsMembersPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNstDefaultAccessLevelsSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_NST_AM_PARAM_ENT_paramType,
    &DX_IN_GT_U32_accessLevel
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNstDefaultAccessLevelsGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_NST_AM_PARAM_ENT_paramType,
    &DX_OUT_GT_U32_PTR_accessLevelPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNstProtSanityCheckPacketCommandSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_NST_CHECK_ENT_checkType,
    &DX_IN_CPSS_PACKET_CMD_ENT_packetCmd
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNstProtSanityCheckSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_NST_CHECK_ENT_checkType,
    &DX_IN_GT_BOOL_enable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNstProtSanityCheckPacketCommandGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_NST_CHECK_ENT_checkType,
    &DX_OUT_CPSS_PACKET_CMD_ENT_PTR_packetCmdPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNstProtSanityCheckGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_NST_CHECK_ENT_checkType,
    &DX_OUT_GT_BOOL_PTR_enablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNstPortGroupIngressFrwFilterDropCntrSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_ingressCnt
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNstPortGroupIngressFrwFilterDropCntrGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_OUT_GT_U32_PTR_ingressCntPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNstPortIsolationModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_NST_PORT_ISOLATION_MODE_ENT_mode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNstPortEgressFrwFilterSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_CPSS_NST_EGRESS_FRW_FILTER_ENT_filterType,
    &DX_IN_GT_BOOL_enable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNstPortEgressFrwFilterGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_CPSS_NST_EGRESS_FRW_FILTER_ENT_filterType,
    &DX_OUT_GT_BOOL_PTR_enablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNstPortIngressFrwFilterSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_CPSS_NST_INGRESS_FRW_FILTER_ENT_filterType,
    &DX_IN_GT_BOOL_enable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNstPortIngressFrwFilterGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_CPSS_NST_INGRESS_FRW_FILTER_ENT_filterType,
    &DX_OUT_GT_BOOL_PTR_enablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNstPortIsolationModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_OUT_CPSS_DXCH_NST_PORT_ISOLATION_MODE_ENT_PTR_modePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNstIngressFrwFilterDropCntrSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_ingressCnt
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNstPortIsolationLookupBitsSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_numberOfPortBits,
    &DX_IN_GT_U32_numberOfDeviceBits,
    &DX_IN_GT_U32_numberOfTrunkBits
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNstBridgeAccessMatrixCmdSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_saAccessLevel,
    &DX_IN_GT_U32_daAccessLevel,
    &DX_IN_CPSS_PACKET_CMD_ENT_command
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNstBridgeAccessMatrixCmdGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_saAccessLevel,
    &DX_IN_GT_U32_daAccessLevel,
    &DX_OUT_CPSS_PACKET_CMD_ENT_PTR_commandPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNstRouterAccessMatrixCmdSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_sipAccessLevel,
    &DX_IN_GT_U32_dipAccessLevel,
    &DX_IN_CPSS_PACKET_CMD_ENT_command
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNstRouterAccessMatrixCmdGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_sipAccessLevel,
    &DX_IN_GT_U32_dipAccessLevel,
    &DX_OUT_CPSS_PACKET_CMD_ENT_PTR_commandPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNstPortIsolationSrcIdBitLocationSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_srcIdBitLocation,
    &DX_IN_GT_BOOL_enable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNstPortIsolationLookupTrunkIndexBaseSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_trunkIndexBase
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNstPortIsolationIndexingModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_ENT_PTR_modePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNstIngressFrwFilterDropCntrGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_ingressCntPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNstPortIsolationLookupBitsGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_numberOfPortBitsPtr,
    &DX_OUT_GT_U32_PTR_numberOfDeviceBitsPtr,
    &DX_OUT_GT_U32_PTR_numberOfTrunkBitsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNstPortIsolationSrcIdBitLocationGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_srcIdBitLocationPtr,
    &DX_OUT_GT_BOOL_PTR_enablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChNstPortIsolationLookupTrunkIndexBaseGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_trunkIndexBasePtr
};


/********* lib API DB *********/

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChNstLogLibDb[] = {
    {"cpssDxChNstBridgeAccessMatrixCmdSet", 4, cpssDxChNstBridgeAccessMatrixCmdSet_PARAMS, NULL},
    {"cpssDxChNstBridgeAccessMatrixCmdGet", 4, cpssDxChNstBridgeAccessMatrixCmdGet_PARAMS, NULL},
    {"cpssDxChNstDefaultAccessLevelsSet", 3, cpssDxChNstDefaultAccessLevelsSet_PARAMS, NULL},
    {"cpssDxChNstDefaultAccessLevelsGet", 3, cpssDxChNstDefaultAccessLevelsGet_PARAMS, NULL},
    {"cpssDxChNstProtSanityCheckPacketCommandSet", 3, cpssDxChNstProtSanityCheckPacketCommandSet_PARAMS, NULL},
    {"cpssDxChNstProtSanityCheckPacketCommandGet", 3, cpssDxChNstProtSanityCheckPacketCommandGet_PARAMS, NULL},
    {"cpssDxChNstProtSanityCheckSet", 3, cpssDxChNstProtSanityCheckSet_PARAMS, NULL},
    {"cpssDxChNstProtSanityCheckGet", 3, cpssDxChNstProtSanityCheckGet_PARAMS, NULL},
    {"cpssDxChNstPortIngressFrwFilterSet", 4, cpssDxChNstPortIngressFrwFilterSet_PARAMS, NULL},
    {"cpssDxChNstPortIngressFrwFilterGet", 4, cpssDxChNstPortIngressFrwFilterGet_PARAMS, NULL},
    {"cpssDxChNstPortEgressFrwFilterSet", 4, cpssDxChNstPortEgressFrwFilterSet_PARAMS, NULL},
    {"cpssDxChNstPortEgressFrwFilterGet", 4, cpssDxChNstPortEgressFrwFilterGet_PARAMS, NULL},
    {"cpssDxChNstIngressFrwFilterDropCntrSet", 2, cpssDxChNstIngressFrwFilterDropCntrSet_PARAMS, NULL},
    {"cpssDxChNstIngressFrwFilterDropCntrGet", 2, cpssDxChNstIngressFrwFilterDropCntrGet_PARAMS, NULL},
    {"cpssDxChNstRouterAccessMatrixCmdSet", 4, cpssDxChNstRouterAccessMatrixCmdSet_PARAMS, NULL},
    {"cpssDxChNstRouterAccessMatrixCmdGet", 4, cpssDxChNstRouterAccessMatrixCmdGet_PARAMS, NULL},
    {"cpssDxChNstPortGroupIngressFrwFilterDropCntrSet", 3, cpssDxChNstPortGroupIngressFrwFilterDropCntrSet_PARAMS, NULL},
    {"cpssDxChNstPortGroupIngressFrwFilterDropCntrGet", 3, cpssDxChNstPortGroupIngressFrwFilterDropCntrGet_PARAMS, NULL},
    {"cpssDxChNstPortIsolationEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChNstPortIsolationEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChNstPortIsolationTableEntrySet", 5, cpssDxChNstPortIsolationTableEntrySet_PARAMS, NULL},
    {"cpssDxChNstPortIsolationTableEntryGet", 5, cpssDxChNstPortIsolationTableEntryGet_PARAMS, NULL},
    {"cpssDxChNstPortIsolationPortAdd", 4, cpssDxChNstPortIsolationPortAdd_PARAMS, NULL},
    {"cpssDxChNstPortIsolationPortDelete", 4, cpssDxChNstPortIsolationPortAdd_PARAMS, NULL},
    {"cpssDxChNstPortIsolationModeSet", 3, cpssDxChNstPortIsolationModeSet_PARAMS, NULL},
    {"cpssDxChNstPortIsolationModeGet", 3, cpssDxChNstPortIsolationModeGet_PARAMS, NULL},
    {"cpssDxChNstPortIsolationLookupBitsSet", 4, cpssDxChNstPortIsolationLookupBitsSet_PARAMS, NULL},
    {"cpssDxChNstPortIsolationLookupBitsGet", 4, cpssDxChNstPortIsolationLookupBitsGet_PARAMS, NULL},
    {"cpssDxChNstPortIsolationOnEportsEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChNstPortIsolationOnEportsEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChNstPortIsolationLookupTrunkIndexBaseSet", 2, cpssDxChNstPortIsolationLookupTrunkIndexBaseSet_PARAMS, NULL},
    {"cpssDxChNstPortIsolationLookupTrunkIndexBaseGet", 2, cpssDxChNstPortIsolationLookupTrunkIndexBaseGet_PARAMS, NULL},
    {"cpssDxChNstPortIsolationSrcIdBitLocationGet", 3, cpssDxChNstPortIsolationSrcIdBitLocationGet_PARAMS, NULL},
    {"cpssDxChNstPortIsolationSrcIdBitLocationSet", 3, cpssDxChNstPortIsolationSrcIdBitLocationSet_PARAMS, NULL},
    {"cpssDxChNstPortIsolationIndexingModeSet", 2, cpssDxChNstPortIsolationIndexingModeSet_PARAMS, NULL},
    {"cpssDxChNstPortIsolationIndexingModeGet", 2, cpssDxChNstPortIsolationIndexingModeGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_NST(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChNstLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChNstLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

