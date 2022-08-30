/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssGenI2cLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssGenI2cLogh
#define __prvCpssGenI2cLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_frequency;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_tclk;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U8_bus_id;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_CPSS_HW_DRIVER_STC_PTR_PTR_drvPtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssI2cHwDriverCreateDrv_E = (CPSS_LOG_LIB_I2C_E << 16)
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssGenI2cLogh */
