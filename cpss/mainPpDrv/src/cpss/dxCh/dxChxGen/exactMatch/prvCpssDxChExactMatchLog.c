/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChExactMatchLog.c
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
#include <cpss/dxCh/dxChxGen/exactMatch/cpssDxChExactMatch.h>
#include <cpss/dxCh/dxChxGen/exactMatch/private/prvCpssDxChExactMatchHash.h>
#include <cpss/dxCh/dxChxGen/exactMatch/private/prvCpssDxChExactMatchLog.h>
#include <cpss/dxCh/dxChxGen/pcl/private/prvCpssDxChPclLog.h>
#include <cpss/dxCh/dxChxGen/tti/private/prvCpssDxChTtiLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssGenDbLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT[]  =
{
    "CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E",
    "CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E",
    "CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E",
    "CPSS_DXCH_EXACT_MATCH_ACTION_LAST_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_EXACT_MATCH_CLIENT_ENT[]  =
{
    "CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E",
    "CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_0_E",
    "CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_1_E",
    "CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_2_E",
    "CPSS_DXCH_EXACT_MATCH_CLIENT_EPCL_E",
    "CPSS_DXCH_EXACT_MATCH_CLIENT_LAST_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_EXACT_MATCH_CLIENT_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT[]  =
{
    "CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E",
    "CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E",
    "CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_ENT[]  =
{
    "CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_PER_PACKET_TYPE_E",
    "CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_PER_PORT_E",
    "CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_PER_PORT_PACKET_TYPE_E",
    "CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_LAST_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_EXACT_MATCH_MHT_ENT[]  =
{
    "CPSS_EXACT_MATCH_MHT_4_E",
    "CPSS_EXACT_MATCH_MHT_8_E",
    "CPSS_EXACT_MATCH_MHT_16_E",
    "CPSS_EXACT_MATCH_MHT_LAST_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_EXACT_MATCH_MHT_ENT);
const char * const prvCpssLogEnum_CPSS_EXACT_MATCH_TABLE_SIZE_ENT[]  =
{
    "CPSS_EXACT_MATCH_TABLE_SIZE_4KB",
    "CPSS_EXACT_MATCH_TABLE_SIZE_8KB",
    "CPSS_EXACT_MATCH_TABLE_SIZE_16KB",
    "CPSS_EXACT_MATCH_TABLE_SIZE_32KB",
    "CPSS_EXACT_MATCH_TABLE_SIZE_64KB",
    "CPSS_EXACT_MATCH_TABLE_SIZE_128KB",
    "CPSS_EXACT_MATCH_TABLE_SIZE_256KB",
    "CPSS_EXACT_MATCH_TABLE_SIZE_512KB",
    "CPSS_EXACT_MATCH_TABLE_SIZE_LAST_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_EXACT_MATCH_TABLE_SIZE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT[]  =
{
    "CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E",
    "CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E",
    "CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E",
    "CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E",
    "CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_ACTION_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_EXACT_MATCH_ACTION_UNT *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ttiAction, CPSS_DXCH_TTI_ACTION_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, pclAction, CPSS_DXCH_PCL_ACTION_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FAIL_COUNTERS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FAIL_COUNTERS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, flowIdFailCounter);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, indexFailCounter);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_ALLOC_STATUS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_ALLOC_STATUS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numAllocated);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, flowIdRangeIsFull);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_PARAMS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, threshold);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxNum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, baseFlowId);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_EXACT_MATCH_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, key, CPSS_DXCH_EXACT_MATCH_KEY_STC);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, lookupNum, CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ttiExpandedActionOrigin, CPSS_DXCH_EXACT_MATCH_EXPANDED_TTI_ACTION_ORIGIN_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, pclExpandedActionOrigin, CPSS_DXCH_EXACT_MATCH_EXPANDED_PCL_ACTION_ORIGIN_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_KEY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_EXACT_MATCH_KEY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, keySize, CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, pattern, CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS, GT_U8);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, keySize, CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, keyStart);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, mask, CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS, GT_U8);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_EXPANDED_PCL_ACTION_ORIGIN_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_EXACT_MATCH_EXPANDED_PCL_ACTION_ORIGIN_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionUserDefinedCpuCode);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionPktCmd);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionRedirectCommand);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionEgressInterface);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, egressInterfaceMask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionMacSa);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, macSa_27_0_Mask);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, macSa_47_28_Mask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionRouterLttPtr);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, routerLttPtrMask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionVntl2Echo);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionTunnelStart);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionArpPtr);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, arpPtrMask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionTunnelIndex);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, tunnelIndexMask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionTsPassengerPacketType);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionVrfId);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, vrfIdMask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionActionStop);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionSetMacToMe);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionPCL1OverrideConfigIndex);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionPCL2OverrideConfigIndex);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionIPCLConfigurationIndex);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, ipclConfigurationIndexMask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionPolicerEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionPolicerIndex);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, policerIndexMask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionMirrorMode);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionMirrorTcpRstAndFinPacketsToCpu);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionMirror);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionModifyMacDA);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionModifyMacSA);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionBypassBridge);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionBypassIngressPipe);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionNestedVlanEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionVlanPrecedence);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionVlan0Command);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionVlan0);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, vlan0Mask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionVlan1Command);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionVlan1);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, vlan1Mask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionSourceIdEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionSourceId);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, sourceIdMask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionMatchCounterEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionMatchCounterIndex);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, matchCounterIndexMask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionQosProfileMakingEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionQosPrecedence);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionQoSProfile);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, qosProfileMask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionQoSModifyDSCP);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionUp1Command);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionModifyUp1);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionUp1);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionDscpExp);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, dscpExpMask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionUp0);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionModifyUp0);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionOamTimestampEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionOamOffsetIndex);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, oamOffsetIndexMask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionOamProcessingEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionOamProfile);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionFlowId);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, flowIdMask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionSourcePortEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionSourcePort);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, sourceEPortMask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionLatencyMonitor);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, latencyMonitorMask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionLatencyMonitorEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionSkipFdbSaLookup);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionTriggerInterrupt);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionPhaMetadataAssignEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionPhaMetadata);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, phaMetadataMask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionPhaThreadNumberAssignEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionPhaThreadNumber);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, phaThreadNumberMask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionCutThroughTerminateId);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpendedActionCopyReservedEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpendedActionCopyReserved);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, copyReservedMask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionIpfixEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionTriggerHashCncClient);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionEgressCncIndexMode);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionEnableEgressMaxSduSizeCheck);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionEgressMaxSduSizeProfile);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_EXPANDED_TTI_ACTION_ORIGIN_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_EXACT_MATCH_EXPANDED_TTI_ACTION_ORIGIN_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionCommand);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionUserDefinedCpuCode);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionMirrorToIngressAnalyzerIndex);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionRedirectCommand);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionEgressInterface);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, egressInterfaceMask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionVrfId);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, vrfIdMask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionFlowId);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, flowIdMask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionRouterLttPtr);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, routerLttPtrMask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionIpclUdbConfigTable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionVntl2Echo);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionTunnelStart);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionArpPtr);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, arpPtrMask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionTunnelIndex);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, tunnelIndexMask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionPcl0OverrideConfigIndex);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionPcl1OverrideConfigIndex);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionPcl2OverrideConfigIndex);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionIpclConfigIndex);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, ipclConfigIndexMask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionTsPassengerPacketType);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionModifyMacDa);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionModifyMacSa);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionBindToCentralCounter);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionCentralCounterIndex);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, centralCounterIndexMask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionBindToPolicerMeter);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionBindToPolicer);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionPolicerIndex);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, policerIndexMask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionSourceIdSetEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionSourceId);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, sourceIdMask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionActionStop);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionBridgeBypass);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionIngressPipeBypass);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionTag0VlanPrecedence);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionNestedVlanEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionTag0VlanCmd);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionTag0VlanId);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, tag0VlanIdMask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionTag1VlanCmd);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionTag1VlanId);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, tag1VlanIdMask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionQosPrecedence);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionQosProfile);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionModifyDscp);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionModifyTag0);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionKeepPreviousQoS);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionTrustUp);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionTrustDscp);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionTrustExp);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionRemapDscp);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionTag0Up);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionTag1UpCommand);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionTag1Up);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionTtPassengerPacketType);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionCopyTtlExpFromTunnelHeader);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionTunnelTerminate);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionMplsCommand);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionHashMaskIndex);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionQosMappingTableIndex);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionMplsLLspQoSProfileEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionMplsTtl);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionEnableDecrementTtl);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionSourceEPortAssignmentEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionSourceEPort);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, sourceEPortMask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionTtHeaderLength);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionIsPtpPacket);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionOamTimeStampEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionPtpTriggerType);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionPtpOffset);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionOamOffsetIndex);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionOamProcessWhenGalOrOalExistsEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionOamProcessEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionCwBasedPw);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionTtlExpiryVccvEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionPwe3FlowLabelExist);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionPwCwBasedETreeEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionQosUseUpAsIndexEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionRxIsProtectionPath);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionRxProtectionSwitchEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionSetMacToMe);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionOamProfile);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionApplyNonDataCwCommand);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionContinueToNextTtiLookup);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionOamChannelTypeToOpcodeMappingEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionPassengerParsingOfTransitMplsTunnelMode);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionPassengerParsingOfTransitNonMplsTransitTunnelEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionSkipFdbSaLookupEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionIpv6SegmentRoutingEndNodeEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionTunnelHeaderStartL4Enable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionIpfixEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionCopyReservedAssignmentEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, overwriteExpandedActionCopyReserved);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, overwriteExpandedActionTriggerHashCncClient);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_ACTION_UNT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_EXACT_MATCH_ACTION_UNT*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_ACTION_UNT_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FAIL_COUNTERS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FAIL_COUNTERS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FAIL_COUNTERS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_ALLOC_STATUS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_ALLOC_STATUS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_ALLOC_STATUS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_PARAMS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_PARAMS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_CLIENT_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_EXACT_MATCH_CLIENT_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_EXACT_MATCH_CLIENT_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_CLIENT_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_EXACT_MATCH_CLIENT_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_EXACT_MATCH_CLIENT_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_EXACT_MATCH_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_KEY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_EXACT_MATCH_KEY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_KEY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_EXACT_MATCH_MHT_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_EXACT_MATCH_MHT_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_EXACT_MATCH_MHT_ENT);
}
void prvCpssLogParamFunc_CPSS_EXACT_MATCH_TABLE_SIZE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_EXACT_MATCH_TABLE_SIZE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_EXACT_MATCH_TABLE_SIZE_ENT);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT_actionType = {
     "actionType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_ACTION_UNT_PTR_actionPtr = {
     "actionPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_ACTION_UNT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_PARAMS_STC_PTR_confPtr = {
     "confPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_CLIENT_ENT_clientType = {
     "clientType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_EXACT_MATCH_CLIENT_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_ENTRY_STC_PTR_entryPtr = {
     "entryPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT_PTR_expandedActionOriginPtr = {
     "expandedActionOriginPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_KEY_STC_PTR_entryKeyPtr = {
     "entryKeyPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_KEY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT_exactMatchAutoLearnLookupNum = {
     "exactMatchAutoLearnLookupNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT_exactMatchLookupNum = {
     "exactMatchLookupNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT_pclLookupNum = {
     "pclLookupNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT_ttiLookupNum = {
     "ttiLookupNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_ENT_profileIdMode = {
     "profileIdMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC_PTR_keyParamsPtr = {
     "keyParamsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TTI_RULE_UNT_PTR_maskFixedKeyFormatPtr = {
     "maskFixedKeyFormatPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TTI_RULE_UNT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_TTI_RULE_UNT_PTR_patternFixedKeyFormatPtr = {
     "patternFixedKeyFormatPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TTI_RULE_UNT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_EXACT_MATCH_MHT_ENT_exactMatchMht = {
     "exactMatchMht", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_EXACT_MATCH_MHT_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_EXACT_MATCH_TABLE_SIZE_ENT_exactMatchSize = {
     "exactMatchSize", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_EXACT_MATCH_TABLE_SIZE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_actionEn = {
     "actionEn", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_defActionEn = {
     "defActionEn", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_enableExactMatchLookup = {
     "enableExactMatchLookup", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_exactMatchClearActivity = {
     "exactMatchClearActivity", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_exactMatchEntryIndex = {
     "exactMatchEntryIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_exactMatchProfileIndex = {
     "exactMatchProfileIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numOfRules = {
     "numOfRules", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_subProfileId = {
     "subProfileId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_PTR_maskUdbFormatArray = {
     "maskUdbFormatArray", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_PTR_patternUdbFormatArray = {
     "patternUdbFormatArray", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT_PTR_actionTypePtr = {
     "actionTypePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_ACTION_UNT_PTR_actionPtr = {
     "actionPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_ACTION_UNT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FAIL_COUNTERS_STC_PTR_failCounterPtr = {
     "failCounterPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FAIL_COUNTERS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_ALLOC_STATUS_STC_PTR_statusPtr = {
     "statusPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_ALLOC_STATUS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_PARAMS_STC_PTR_confPtr = {
     "confPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_CLIENT_ENT_PTR_clientTypePtr = {
     "clientTypePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_CLIENT_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_ENTRY_STC_PTR_entryPtr = {
     "entryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_ENTRY_STC_PTR_keyParamsPtr = {
     "keyParamsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT_PTR_expandedActionOriginPtr = {
     "expandedActionOriginPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT_PTR_exactMatchAutoLearnLookupNumPtr = {
     "exactMatchAutoLearnLookupNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_ENT_PTR_profileIdModePtr = {
     "profileIdModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC_PTR_keyParamsPtr = {
     "keyParamsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TTI_RULE_UNT_PTR_maskFixedKeyFormatPtr = {
     "maskFixedKeyFormatPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TTI_RULE_UNT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_TTI_RULE_UNT_PTR_patternFixedKeyFormatPtr = {
     "patternFixedKeyFormatPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_TTI_RULE_UNT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_actionEnPtr = {
     "actionEnPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_defActionEnPtr = {
     "defActionEnPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_enableExactMatchLookupPtr = {
     "enableExactMatchLookupPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_exactMatchActivityStatusPtr = {
     "exactMatchActivityStatusPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_exactMatchCrcMultiHashArr = {
     "exactMatchCrcMultiHashArr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_exactMatchIndexPtr = {
     "exactMatchIndexPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_expandedActionIndexPtr = {
     "expandedActionIndexPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_numberOfElemInCrcMultiHashArrPtr = {
     "numberOfElemInCrcMultiHashArrPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_maskUdbFormatArray = {
     "maskUdbFormatArray", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_patternUdbFormatArray = {
     "patternUdbFormatArray", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U8)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchTtiRuleConvertToUdbFormat_PARAMS[] =  {
    &DX_IN_CPSS_DXCH_TTI_RULE_TYPE_ENT_ruleType,
    &DX_IN_CPSS_DXCH_TTI_RULE_UNT_PTR_patternFixedKeyFormatPtr,
    &DX_IN_CPSS_DXCH_TTI_RULE_UNT_PTR_maskFixedKeyFormatPtr,
    &DX_OUT_GT_U8_PTR_patternUdbFormatArray,
    &DX_OUT_GT_U8_PTR_maskUdbFormatArray
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchTtiRuleConvertFromUdbFormat_PARAMS[] =  {
    &DX_IN_CPSS_DXCH_TTI_RULE_TYPE_ENT_ruleType,
    &DX_IN_GT_U8_PTR_patternUdbFormatArray,
    &DX_IN_GT_U8_PTR_maskUdbFormatArray,
    &DX_OUT_CPSS_DXCH_TTI_RULE_UNT_PTR_patternFixedKeyFormatPtr,
    &DX_OUT_CPSS_DXCH_TTI_RULE_UNT_PTR_maskFixedKeyFormatPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const prvCpssDxChExactMatchHashCrcMultiResultsByParamsCalcExt_PARAMS[] =  {
    &DX_IN_CPSS_EXACT_MATCH_TABLE_SIZE_ENT_exactMatchSize,
    &DX_IN_CPSS_EXACT_MATCH_MHT_ENT_exactMatchMht,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_KEY_STC_PTR_entryKeyPtr,
    &DX_OUT_GT_U32_PTR_numberOfElemInCrcMultiHashArrPtr,
    &DX_OUT_GT_U32_PTR_crcMultiHashArr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const prvCpssDxChExactMatchHashCrcMultiResultsByParamsCalc_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_KEY_STC_PTR_entryKeyPtr,
    &DX_OUT_GT_U32_PTR_numberOfElemInCrcMultiHashArrPtr,
    &DX_OUT_GT_U32_PTR_crcMultiHashArr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchHashCrcMultiResultsByParamsCalc_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_KEY_STC_PTR_entryKeyPtr,
    &DX_OUT_GT_U32_PTR_numberOfElemInCrcMultiHashArrPtr,
    &DX_OUT_GT_U32_PTR_exactMatchCrcMultiHashArr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchAutoLearnLookupSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT_exactMatchAutoLearnLookupNum
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchClientLookupSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT_exactMatchLookupNum,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_CLIENT_ENT_clientType
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchClientLookupGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT_exactMatchLookupNum,
    &DX_OUT_CPSS_DXCH_EXACT_MATCH_CLIENT_ENT_PTR_clientTypePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchTtiProfileIdModePacketTypeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TTI_KEY_TYPE_ENT_keyType,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT_ttiLookupNum,
    &DX_IN_GT_BOOL_enableExactMatchLookup,
    &DX_IN_GT_U32_profileId
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchTtiProfileIdModePacketTypeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_TTI_KEY_TYPE_ENT_keyType,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT_ttiLookupNum,
    &DX_OUT_GT_BOOL_PTR_enableExactMatchLookupPtr,
    &DX_OUT_GT_U32_PTR_profileIdPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchPclProfileIdSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_PCL_DIRECTION_ENT_direction,
    &DX_IN_CPSS_DXCH_PCL_PACKET_TYPE_ENT_packetType,
    &DX_IN_GT_U32_subProfileId,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT_pclLookupNum,
    &DX_IN_GT_BOOL_enableExactMatchLookup,
    &DX_IN_GT_U32_profileId
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchPclProfileIdGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_PCL_DIRECTION_ENT_direction,
    &DX_IN_CPSS_DXCH_PCL_PACKET_TYPE_ENT_packetType,
    &DX_IN_GT_U32_subProfileId,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT_pclLookupNum,
    &DX_OUT_GT_BOOL_PTR_enableExactMatchLookupPtr,
    &DX_OUT_GT_U32_PTR_profileIdPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchPortGroupAutoLearnFlowIdParamSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_PARAMS_STC_PTR_confPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchActivityStatusGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_exactMatchEntryIndex,
    &DX_IN_GT_BOOL_exactMatchClearActivity,
    &DX_OUT_GT_BOOL_PTR_exactMatchActivityStatusPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchPortGroupEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_index,
    &DX_IN_GT_U32_expandedActionIndex,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_ENTRY_STC_PTR_entryPtr,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT_actionType,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_ACTION_UNT_PTR_actionPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchPortGroupEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_index,
    &DX_OUT_GT_BOOL_PTR_validPtr,
    &DX_OUT_CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT_PTR_actionTypePtr,
    &DX_OUT_CPSS_DXCH_EXACT_MATCH_ACTION_UNT_PTR_actionPtr,
    &DX_OUT_CPSS_DXCH_EXACT_MATCH_ENTRY_STC_PTR_entryPtr,
    &DX_OUT_GT_U32_PTR_expandedActionIndexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchRulesDump_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_GT_U32_startIndex,
    &DX_IN_GT_U32_numOfRules
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchPortGroupAutoLearnFailCountersGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_OUT_CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FAIL_COUNTERS_STC_PTR_failCounterPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchPortGroupAutoLearnFlowIdAllocStatusGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_OUT_CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_ALLOC_STATUS_STC_PTR_statusPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchPortGroupAutoLearnFlowIdParamGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_OUT_CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_PARAMS_STC_PTR_confPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchPortGroupAutoLearnCollisionKeyParamsGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_OUT_CPSS_DXCH_EXACT_MATCH_ENTRY_STC_PTR_keyParamsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchPortGroupAutoLearnEntryGetNext_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_OUT_GT_U32_PTR_flowIdPtr,
    &DX_OUT_GT_U32_PTR_exactMatchIndexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchTtiPortProfileIdModePortSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT_ttiLookupNum,
    &DX_IN_GT_BOOL_enableExactMatchLookup,
    &DX_IN_GT_U32_profileId
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchTtiPortProfileIdModePortGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT_ttiLookupNum,
    &DX_OUT_GT_BOOL_PTR_enableExactMatchLookupPtr,
    &DX_OUT_GT_U32_PTR_profileIdPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchTtiPortProfileIdModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_ENT_profileIdMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_TTI_KEY_TYPE_ENT_keyType,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT_ttiLookupNum,
    &DX_IN_GT_BOOL_enableExactMatchLookup,
    &DX_IN_GT_U32_profileId
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_TTI_KEY_TYPE_ENT_keyType,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT_ttiLookupNum,
    &DX_OUT_GT_BOOL_PTR_enableExactMatchLookupPtr,
    &DX_OUT_GT_U32_PTR_profileIdPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchTtiPortProfileIdModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_OUT_CPSS_DXCH_EXACT_MATCH_PROFILE_ID_MODE_ENT_PTR_profileIdModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchProfileDefaultActionSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_exactMatchProfileIndex,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT_actionType,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_ACTION_UNT_PTR_actionPtr,
    &DX_IN_GT_BOOL_defActionEn
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchProfileDefaultActionGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_exactMatchProfileIndex,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT_actionType,
    &DX_OUT_CPSS_DXCH_EXACT_MATCH_ACTION_UNT_PTR_actionPtr,
    &DX_OUT_GT_BOOL_PTR_defActionEnPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchProfileKeyParamsSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_exactMatchProfileIndex,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC_PTR_keyParamsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_exactMatchProfileIndex,
    &DX_IN_GT_BOOL_actionEn
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchAutoLearnProfileDefaultActionSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_exactMatchProfileIndex,
    &DX_IN_GT_U32_expandedActionIndex,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT_actionType,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_ACTION_UNT_PTR_actionPtr,
    &DX_IN_GT_BOOL_actionEn
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchProfileKeyParamsGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_exactMatchProfileIndex,
    &DX_OUT_CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC_PTR_keyParamsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchAutoLearnProfileDefaultActionEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_exactMatchProfileIndex,
    &DX_OUT_GT_BOOL_PTR_actionEnPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchAutoLearnProfileDefaultActionGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_exactMatchProfileIndex,
    &DX_OUT_GT_U32_PTR_expandedActionIndexPtr,
    &DX_OUT_CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT_PTR_actionTypePtr,
    &DX_OUT_CPSS_DXCH_EXACT_MATCH_ACTION_UNT_PTR_actionPtr,
    &DX_OUT_GT_BOOL_PTR_actionEnPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchExpandedActionSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_expandedActionIndex,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT_actionType,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_ACTION_UNT_PTR_actionPtr,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT_PTR_expandedActionOriginPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchExpandedActionGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_expandedActionIndex,
    &DX_IN_CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT_actionType,
    &DX_OUT_CPSS_DXCH_EXACT_MATCH_ACTION_UNT_PTR_actionPtr,
    &DX_OUT_CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT_PTR_expandedActionOriginPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChExactMatchAutoLearnLookupGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT_PTR_exactMatchAutoLearnLookupNumPtr
};


/********* lib API DB *********/

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChExactMatchLogLibDb[] = {
    {"cpssDxChExactMatchTtiProfileIdModePacketTypeSet", 5, cpssDxChExactMatchTtiProfileIdModePacketTypeSet_PARAMS, NULL},
    {"cpssDxChExactMatchTtiProfileIdModePacketTypeGet", 5, cpssDxChExactMatchTtiProfileIdModePacketTypeGet_PARAMS, NULL},
    {"cpssDxChExactMatchTtiPortProfileIdModeSet", 3, cpssDxChExactMatchTtiPortProfileIdModeSet_PARAMS, NULL},
    {"cpssDxChExactMatchTtiPortProfileIdModeGet", 3, cpssDxChExactMatchTtiPortProfileIdModeGet_PARAMS, NULL},
    {"cpssDxChExactMatchTtiPortProfileIdModePortSet", 5, cpssDxChExactMatchTtiPortProfileIdModePortSet_PARAMS, NULL},
    {"cpssDxChExactMatchTtiPortProfileIdModePortGet", 5, cpssDxChExactMatchTtiPortProfileIdModePortGet_PARAMS, NULL},
    {"cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeSet", 6, cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeSet_PARAMS, NULL},
    {"cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeGet", 6, cpssDxChExactMatchTtiPortProfileIdModePortPacketTypeGet_PARAMS, NULL},
    {"cpssDxChExactMatchPclProfileIdSet", 7, cpssDxChExactMatchPclProfileIdSet_PARAMS, NULL},
    {"cpssDxChExactMatchPclProfileIdGet", 7, cpssDxChExactMatchPclProfileIdGet_PARAMS, NULL},
    {"cpssDxChExactMatchClientLookupSet", 3, cpssDxChExactMatchClientLookupSet_PARAMS, NULL},
    {"cpssDxChExactMatchClientLookupGet", 3, cpssDxChExactMatchClientLookupGet_PARAMS, NULL},
    {"cpssDxChExactMatchActivityBitEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChExactMatchActivityBitEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChExactMatchActivityStatusGet", 5, cpssDxChExactMatchActivityStatusGet_PARAMS, NULL},
    {"cpssDxChExactMatchProfileKeyParamsSet", 3, cpssDxChExactMatchProfileKeyParamsSet_PARAMS, NULL},
    {"cpssDxChExactMatchProfileKeyParamsGet", 3, cpssDxChExactMatchProfileKeyParamsGet_PARAMS, NULL},
    {"cpssDxChExactMatchProfileDefaultActionSet", 5, cpssDxChExactMatchProfileDefaultActionSet_PARAMS, NULL},
    {"cpssDxChExactMatchProfileDefaultActionGet", 5, cpssDxChExactMatchProfileDefaultActionGet_PARAMS, NULL},
    {"cpssDxChExactMatchExpandedActionSet", 5, cpssDxChExactMatchExpandedActionSet_PARAMS, NULL},
    {"cpssDxChExactMatchExpandedActionGet", 5, cpssDxChExactMatchExpandedActionGet_PARAMS, NULL},
    {"cpssDxChExactMatchPortGroupEntrySet", 7, cpssDxChExactMatchPortGroupEntrySet_PARAMS, NULL},
    {"cpssDxChExactMatchPortGroupEntryGet", 8, cpssDxChExactMatchPortGroupEntryGet_PARAMS, NULL},
    {"cpssDxChExactMatchPortGroupEntryInvalidate", 3, prvCpssLogGenDevNumPortGroupsBmpIndex_PARAMS, NULL},
    {"cpssDxChExactMatchPortGroupEntryStatusGet", 4, prvCpssLogGenDevNumPortGroupsBmpIndexValidPtr_PARAMS, NULL},
    {"cpssDxChExactMatchHashCrcMultiResultsByParamsCalc", 4, cpssDxChExactMatchHashCrcMultiResultsByParamsCalc_PARAMS, NULL},
    {"cpssDxChExactMatchRulesDump", 4, cpssDxChExactMatchRulesDump_PARAMS, NULL},
    {"cpssDxChExactMatchTtiRuleConvertToUdbFormat", 5, cpssDxChExactMatchTtiRuleConvertToUdbFormat_PARAMS, NULL},
    {"cpssDxChExactMatchTtiRuleConvertFromUdbFormat", 5, cpssDxChExactMatchTtiRuleConvertFromUdbFormat_PARAMS, NULL},
    {"cpssDxChExactMatchAutoLearnLookupSet", 2, cpssDxChExactMatchAutoLearnLookupSet_PARAMS, NULL},
    {"cpssDxChExactMatchAutoLearnLookupGet", 2, cpssDxChExactMatchAutoLearnLookupGet_PARAMS, NULL},
    {"cpssDxChExactMatchAutoLearnProfileDefaultActionSet", 6, cpssDxChExactMatchAutoLearnProfileDefaultActionSet_PARAMS, NULL},
    {"cpssDxChExactMatchAutoLearnProfileDefaultActionGet", 6, cpssDxChExactMatchAutoLearnProfileDefaultActionGet_PARAMS, NULL},
    {"cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet", 3, cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet_PARAMS, NULL},
    {"cpssDxChExactMatchAutoLearnProfileDefaultActionEnableGet", 3, cpssDxChExactMatchAutoLearnProfileDefaultActionEnableGet_PARAMS, NULL},
    {"cpssDxChExactMatchPortGroupAutoLearnFlowIdParamSet", 3, cpssDxChExactMatchPortGroupAutoLearnFlowIdParamSet_PARAMS, NULL},
    {"cpssDxChExactMatchPortGroupAutoLearnFlowIdParamGet", 3, cpssDxChExactMatchPortGroupAutoLearnFlowIdParamGet_PARAMS, NULL},
    {"cpssDxChExactMatchPortGroupAutoLearnFlowIdAllocStatusGet", 3, cpssDxChExactMatchPortGroupAutoLearnFlowIdAllocStatusGet_PARAMS, NULL},
    {"cpssDxChExactMatchPortGroupAutoLearnEntryGetNext", 4, cpssDxChExactMatchPortGroupAutoLearnEntryGetNext_PARAMS, NULL},
    {"cpssDxChExactMatchPortGroupAutoLearnCollisionKeyParamsGet", 3, cpssDxChExactMatchPortGroupAutoLearnCollisionKeyParamsGet_PARAMS, NULL},
    {"cpssDxChExactMatchPortGroupAutoLearnEntryRecycleGet", 4, cpssDxChExactMatchPortGroupAutoLearnEntryGetNext_PARAMS, NULL},
    {"cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger", 2, cpssDxChExactMatchPortGroupAutoLearnEntryRecycleTrigger_PARAMS, NULL},
    {"cpssDxChExactMatchPortGroupAutoLearnFailCountersGet", 3, cpssDxChExactMatchPortGroupAutoLearnFailCountersGet_PARAMS, NULL},
    {"prvCpssDxChExactMatchHashCrcMultiResultsByParamsCalcExt", 5, prvCpssDxChExactMatchHashCrcMultiResultsByParamsCalcExt_PARAMS, NULL},
    {"prvCpssDxChExactMatchHashCrcMultiResultsByParamsCalc", 4, prvCpssDxChExactMatchHashCrcMultiResultsByParamsCalc_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_EXACT_MATCH(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChExactMatchLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChExactMatchLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

