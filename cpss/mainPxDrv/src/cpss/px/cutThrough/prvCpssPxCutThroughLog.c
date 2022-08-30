/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssPxCutThroughLog.c
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
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssPxGenDbLog.h>
#include <cpss/px/cutThrough/private/prvCpssPxCutThroughLog.h>


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_untaggedEnable = {
     "untaggedEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_HEX_etherType0 = {
     "etherType0", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32_HEX)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_HEX_etherType1 = {
     "etherType1", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32_HEX)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_buffersLimit = {
     "buffersLimit", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_up = {
     "up", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_untaggedEnablePtr = {
     "untaggedEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_HEX_PTR_etherType0Ptr = {
     "etherType0Ptr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32_HEX)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_HEX_PTR_etherType1Ptr = {
     "etherType1Ptr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32_HEX)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_buffersLimitPtr = {
     "buffersLimitPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCutThroughPortEnableSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_BOOL_enable,
    &PX_IN_GT_BOOL_untaggedEnable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCutThroughMaxBuffersLimitSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_buffersLimit
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCutThroughPortEnableGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_GT_BOOL_PTR_enablePtr,
    &PX_OUT_GT_BOOL_PTR_untaggedEnablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCutThroughMaxBuffersLimitGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_GT_U32_PTR_buffersLimitPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCutThroughUpEnableSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_up,
    &PX_IN_GT_BOOL_enable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCutThroughUpEnableGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_up,
    &PX_OUT_GT_BOOL_PTR_enablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCutThroughVlanEthertypeSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_HEX_etherType0,
    &PX_IN_GT_U32_HEX_etherType1
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCutThroughVlanEthertypeGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_GT_U32_HEX_PTR_etherType0Ptr,
    &PX_OUT_GT_U32_HEX_PTR_etherType1Ptr
};


/********* lib API DB *********/

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssPxCutThroughLogLibDb[] = {
    {"cpssPxCutThroughPortEnableSet", 4, cpssPxCutThroughPortEnableSet_PARAMS, NULL},
    {"cpssPxCutThroughPortEnableGet", 4, cpssPxCutThroughPortEnableGet_PARAMS, NULL},
    {"cpssPxCutThroughUpEnableSet", 3, cpssPxCutThroughUpEnableSet_PARAMS, NULL},
    {"cpssPxCutThroughUpEnableGet", 3, cpssPxCutThroughUpEnableGet_PARAMS, NULL},
    {"cpssPxCutThroughVlanEthertypeSet", 3, cpssPxCutThroughVlanEthertypeSet_PARAMS, NULL},
    {"cpssPxCutThroughVlanEthertypeGet", 3, cpssPxCutThroughVlanEthertypeGet_PARAMS, NULL},
    {"cpssPxCutThroughMaxBuffersLimitSet", 3, cpssPxCutThroughMaxBuffersLimitSet_PARAMS, NULL},
    {"cpssPxCutThroughMaxBuffersLimitGet", 3, cpssPxCutThroughMaxBuffersLimitGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_PX_CPSS_LOG_LIB_CUT_THROUGH(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssPxCutThroughLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssPxCutThroughLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

