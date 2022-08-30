/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChSystemRecoveryLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssDxChSystemRecoveryLogh
#define __prvCpssDxChSystemRecoveryLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_HSU_DATA_TYPE_ENT);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_HSU_DATA_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_HSU_DATA_TYPE_ENT_dataType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_CHAR_PTR_miVersionPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_newEvReqHndl;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_oldEvReqHndl;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_isCompatiblePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_PRV_CPSS_BOOT_CH_PORT_STATUS_STC_PTR_portInfoPtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssDxChHitlessStartupMiCompatibilityCheck_E = (CPSS_LOG_LIB_SYSTEM_RECOVERY_E << 16) | (1 << 24),
    PRV_CPSS_LOG_FUNC_cpssDxChHitlessStartupPortParamsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHitlessStartupSyncPortManager_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsuBlockSizeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsuExport_E,
    PRV_CPSS_LOG_FUNC_cpssDxChHsuImport_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCatchUpValidityCheckEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCatchUpValidityCheck_E,
    PRV_CPSS_LOG_FUNC_cpssHsuEventHandleUpdate_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssDxChSystemRecoveryLogh */
