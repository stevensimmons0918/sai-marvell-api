/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssPxCutThroughLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssPxCutThroughLogh
#define __prvCpssPxCutThroughLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_untaggedEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_HEX_etherType0;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_HEX_etherType1;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_buffersLimit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_up;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_untaggedEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_HEX_PTR_etherType0Ptr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_HEX_PTR_etherType1Ptr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_buffersLimitPtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssPxCutThroughPortEnableSet_E = (CPSS_LOG_LIB_CUT_THROUGH_E << 16) | (2 << 24),
    PRV_CPSS_LOG_FUNC_cpssPxCutThroughPortEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCutThroughUpEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCutThroughUpEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCutThroughVlanEthertypeSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCutThroughVlanEthertypeGet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCutThroughMaxBuffersLimitSet_E,
    PRV_CPSS_LOG_FUNC_cpssPxCutThroughMaxBuffersLimitGet_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssPxCutThroughLogh */
