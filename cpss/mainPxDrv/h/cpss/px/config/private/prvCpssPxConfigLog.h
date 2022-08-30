/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssPxConfigLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssPxConfigLogh
#define __prvCpssPxConfigLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_PX_CFG_CNTR_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_PX_CFG_DEV_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_PX_CFG_HW_ACCESS_OBJ_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_PX_CFG_CNTR_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_CFG_DEV_INFO_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_PX_CFG_HW_ACCESS_OBJ_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_CFG_CNTR_ENT_counterType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_CFG_HW_ACCESS_OBJ_STC_PTR_cfgAccessObjPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_TABLE_ENT_table;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_bind;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_SW_DEV_NUM_newDevNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_SW_DEV_NUM_oldDevNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_CFG_DEV_INFO_STC_PTR_devInfoPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_SW_DEV_NUM_PTR_nextDevNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_numEntriesPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_timeStampValuePtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssPxCfgDevRemove_E = (CPSS_LOG_LIB_CONFIG_E << 16) | (2 << 24),
    PRV_CPSS_LOG_FUNC_cpssPxCfgDevEnable_E,
    PRV_CPSS_LOG_FUNC_cpssPxCfgDevEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCfgNextDevGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCfgTableNumEntriesGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCfgReNumberDevNum_E,
    PRV_CPSS_LOG_FUNC_cpssPxCfgDevInfoGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCfgHwAccessObjectBind_E,
    PRV_CPSS_LOG_FUNC_cpssPxCfgHitlessWriteMethodEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCfgLastRegAccessTimeStampGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCfgCntrGet_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssPxConfigLogh */
