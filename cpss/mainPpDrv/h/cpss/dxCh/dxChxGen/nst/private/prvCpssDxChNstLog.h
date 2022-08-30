/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChNstLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssDxChNstLogh
#define __prvCpssDxChNstLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_NST_PORT_ISOLATION_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_NST_PORT_ISOLATION_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_NST_PORT_ISOLATION_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_NST_PORT_ISOLATION_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_NST_PORT_ISOLATION_TRAFFIC_TYPE_ENT_trafficType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_INTERFACE_INFO_STC_PTR_srcInterfacePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_NST_AM_PARAM_ENT_paramType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_NST_CHECK_ENT_checkType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_NST_EGRESS_FRW_FILTER_ENT_filterType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_NST_INGRESS_FRW_FILTER_ENT_filterType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORTS_BMP_STC_PTR_localPortsMembersPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_cpuPortMember;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_daAccessLevel;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_dipAccessLevel;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ingressCnt;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numberOfDeviceBits;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numberOfPortBits;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numberOfTrunkBits;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_saAccessLevel;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_sipAccessLevel;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_srcIdBitLocation;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_trunkIndexBase;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_NST_PORT_ISOLATION_INDEXING_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_NST_PORT_ISOLATION_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORTS_BMP_STC_PTR_localPortsMembersPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_cpuPortMemberPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_ingressCntPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_numberOfDeviceBitsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_numberOfPortBitsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_numberOfTrunkBitsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_srcIdBitLocationPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_trunkIndexBasePtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssDxChNstBridgeAccessMatrixCmdSet_E = (CPSS_LOG_LIB_NST_E << 16) | (1 << 24),
    PRV_CPSS_LOG_FUNC_cpssDxChNstBridgeAccessMatrixCmdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChNstDefaultAccessLevelsSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChNstDefaultAccessLevelsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChNstProtSanityCheckPacketCommandSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChNstProtSanityCheckPacketCommandGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChNstProtSanityCheckSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChNstProtSanityCheckGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChNstPortIngressFrwFilterSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChNstPortIngressFrwFilterGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChNstPortEgressFrwFilterSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChNstPortEgressFrwFilterGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChNstIngressFrwFilterDropCntrSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChNstIngressFrwFilterDropCntrGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChNstRouterAccessMatrixCmdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChNstRouterAccessMatrixCmdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChNstPortGroupIngressFrwFilterDropCntrSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChNstPortGroupIngressFrwFilterDropCntrGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChNstPortIsolationEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChNstPortIsolationEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChNstPortIsolationTableEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChNstPortIsolationTableEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChNstPortIsolationPortAdd_E,
    PRV_CPSS_LOG_FUNC_cpssDxChNstPortIsolationPortDelete_E,
    PRV_CPSS_LOG_FUNC_cpssDxChNstPortIsolationModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChNstPortIsolationModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChNstPortIsolationLookupBitsSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChNstPortIsolationLookupBitsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChNstPortIsolationOnEportsEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChNstPortIsolationOnEportsEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChNstPortIsolationLookupTrunkIndexBaseSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChNstPortIsolationLookupTrunkIndexBaseGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChNstPortIsolationSrcIdBitLocationGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChNstPortIsolationSrcIdBitLocationSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChNstPortIsolationIndexingModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChNstPortIsolationIndexingModeGet_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssDxChNstLogh */
