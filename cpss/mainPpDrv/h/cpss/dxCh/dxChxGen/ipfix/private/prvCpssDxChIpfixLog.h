/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChIpfixLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssDxChIpfixLogh
#define __prvCpssDxChIpfixLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_IPFIX_RESET_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_IPFIX_PHA_METADATA_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_IPFIX_SAMPLING_ACTION_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_IPFIX_SAMPLING_DIST_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_IPFIX_SAMPLING_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_IPFIX_WRAPAROUND_ACTION_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_TIMER_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_RESET_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_TIMER_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPFIX_ENTRY_STC_PTR_ipfixEntryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC_PTR_firstPacketCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPFIX_RESET_MODE_ENT_reset;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC_PTR_uploadPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC_PTR_confPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_endIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ipfixIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U64_phaCounterMask;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IPFIX_ENTRY_STC_PTR_ipfixEntryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC_PTR_firstPacketCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IPFIX_TIMER_STC_PTR_timerPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC_PTR_uploadPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC_PTR_confPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_uploadStatusPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_bmpPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_eventsArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_eventsNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U64_PTR_phaCounterMaskPtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssDxChIpfixEntrySet_E = (CPSS_LOG_LIB_IPFIX_E << 16) | (1 << 24),
    PRV_CPSS_LOG_FUNC_cpssDxChIpfixEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpfixDropCountModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpfixDropCountModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpfixCpuCodeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpfixCpuCodeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpfixAgingEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpfixAgingEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpfixWraparoundConfSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpfixWraparoundConfGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpfixWraparoundStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpfixAgingStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpfixTimestampUploadSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpfixTimestampUploadGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpfixTimestampUploadTrigger_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpfixTimestampUploadStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpfixTimerGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpfixAlarmEventsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpfixPortGroupEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpfixPortGroupEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpfixPortGroupWraparoundStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpfixPortGroupAgingStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpfixPortGroupAlarmEventsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpfixTimestampToCpuEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpfixTimestampToCpuEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpfixBaseFlowIdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpfixBaseFlowIdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpfixFirstPacketConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpfixFirstPacketConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpfixPhaMetadataCounterMaskSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpfixPhaMetadataCounterMaskGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpfixFirstPacketsMirrorEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChIpfixFirstPacketsMirrorEnableGet_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssDxChIpfixLogh */
