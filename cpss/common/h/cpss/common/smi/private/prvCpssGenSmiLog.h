/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssGenSmiLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssGenSmiLogh
#define __prvCpssGenSmiLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_CPSS_PHY_SMI_INTERFACE_ENT_smiInterface;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_CPSS_PHY_XSMI_INTERFACE_ENT_xsmiInterface;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_PORT_GROUPS_BMP_portGroupsBmp;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U16_HEX_data;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_phyDev;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_regAddr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_smiAddr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_xsmiAddr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_GT_U16_HEX_PTR_dataPtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssSmiRegisterReadShort_E = (CPSS_LOG_LIB_SMI_E << 16),
    PRV_CPSS_LOG_FUNC_cpssSmiRegisterWriteShort_E,
    PRV_CPSS_LOG_FUNC_cpssSmiRegisterRead_E,
    PRV_CPSS_LOG_FUNC_cpssSmiRegisterWrite_E,
    PRV_CPSS_LOG_FUNC_cpssXsmiPortGroupRegisterWrite_E,
    PRV_CPSS_LOG_FUNC_cpssXsmiPortGroupRegisterRead_E,
    PRV_CPSS_LOG_FUNC_cpssXsmiRegisterWrite_E,
    PRV_CPSS_LOG_FUNC_cpssXsmiRegisterRead_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssGenSmiLogh */
