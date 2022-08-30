/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChCutThroughLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssDxChCutThroughLogh
#define __prvCpssDxChCutThroughLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_EARLY_PROCESSING_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_CUT_THROUGH_BYPASS_INGRESS_PCL_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_CUT_THROUGH_BYPASS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_CUT_THROUGH_PACKET_HDR_INTEGRITY_CHECK_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_CUT_THROUGH_BYPASS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_CUT_THROUGH_PACKET_HDR_INTEGRITY_CHECK_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_EARLY_PROCESSING_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_EARLY_PROCESSING_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CUT_THROUGH_BYPASS_STC_PTR_bypassModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CUT_THROUGH_PACKET_HDR_INTEGRITY_CHECK_CFG_STC_PTR_cfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_EARLY_PROCESSING_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_buffersLimitEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_dropPaddedPacket;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_dropTruncatedPacket;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_toSubtractOrToAdd;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_udeCutThroughEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_untaggedEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_HEX_etherType0;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_HEX_etherType1;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_buffersLimit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_subtractedOrAddedValue;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_udeByteCount;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_udeIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CUT_THROUGH_BYPASS_STC_PTR_bypassModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CUT_THROUGH_PACKET_HDR_INTEGRITY_CHECK_CFG_STC_PTR_cfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_EARLY_PROCESSING_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_SPEED_ENT_PTR_portSpeedPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_buffersLimitEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_dropPaddedPacketPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_dropTruncatedPacketPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_toSubtractOrToAddPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_udeCutThroughEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_untaggedEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_HEX_PTR_etherType0Ptr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_HEX_PTR_etherType1Ptr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_buffersLimitPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_countPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_subtractedOrAddedValuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_udeByteCountPtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssDxChCutThroughPortEnableSet_E = (CPSS_LOG_LIB_CUT_THROUGH_E << 16) | (1 << 24),
    PRV_CPSS_LOG_FUNC_cpssDxChCutThroughPortEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCutThroughUpEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCutThroughUpEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCutThroughVlanEthertypeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCutThroughVlanEthertypeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCutThroughMinimalPacketSizeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCutThroughMinimalPacketSizeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCutThroughMemoryRateLimitSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCutThroughMemoryRateLimitGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCutThroughBypassModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCutThroughBypassModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCutThroughPortGroupMaxBuffersLimitSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCutThroughPortGroupMaxBuffersLimitGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCutThroughEarlyProcessingModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCutThroughEarlyProcessingModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCutThroughUdeCfgSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCutThroughUdeCfgGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCutThroughByteCountExtractFailsCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCutThroughPacketHdrIntegrityCheckCfgSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCutThroughPacketHdrIntegrityCheckCfgGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCutThroughPortByteCountUpdateSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCutThroughPortByteCountUpdateGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCutThroughAllPacketTypesEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCutThroughAllPacketTypesEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCutThroughMplsPacketEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCutThroughMplsPacketEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCutThroughErrorConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCutThroughErrorConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCutThroughPortUnknownByteCountEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCutThroughPortUnknownByteCountEnableGet_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssDxChCutThroughLogh */
