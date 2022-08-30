/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssI2cLog.c
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/common/i2c/private/prvCpssGenI2cLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_frequency = {
     "frequency", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_tclk = {
     "tclk", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U8_bus_id = {
     "bus_id", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_CPSS_HW_DRIVER_STC_PTR_PTR_drvPtr = {
     "drvPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_PTR_MAC(CPSS_HW_DRIVER_STC)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssI2cHwDriverCreateDrv_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U8_bus_id,
    &IN_GT_U32_frequency,
    &IN_GT_U32_tclk,
    &OUT_CPSS_HW_DRIVER_STC_PTR_PTR_drvPtr
};


/********* lib API DB *********/


#ifdef CHX_FAMILY
static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssCommonDXI2cLogLibDb[] = {
    {"cpssI2cHwDriverCreateDrv", 5, cpssI2cHwDriverCreateDrv_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_I2C(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssCommonDXI2cLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssCommonDXI2cLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}
#endif

#ifdef PX_FAMILY
static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssCommonPXI2cLogLibDb[] = {
    {"cpssI2cHwDriverCreateDrv", 5, cpssI2cHwDriverCreateDrv_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_PX_CPSS_LOG_LIB_I2C(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssCommonPXI2cLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssCommonPXI2cLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}
#endif

