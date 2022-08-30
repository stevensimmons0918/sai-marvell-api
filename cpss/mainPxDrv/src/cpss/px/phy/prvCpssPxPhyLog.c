/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssPxPhyLog.c
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
#include <cpss/px/phy/private/prvCpssPxPhyLog.h>


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT_divisionFactor = {
     "divisionFactor", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT_PTR_divisionFactorPtr = {
     "divisionFactorPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPhySmiMdcDivisionFactorSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT_divisionFactor
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPhySmiMdcDivisionFactorGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT_PTR_divisionFactorPtr
};


/********* lib API DB *********/

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssPxPhyLogLibDb[] = {
    {"cpssPxPhySmiMdcDivisionFactorSet", 2, cpssPxPhySmiMdcDivisionFactorSet_PARAMS, NULL},
    {"cpssPxPhySmiMdcDivisionFactorGet", 2, cpssPxPhySmiMdcDivisionFactorGet_PARAMS, NULL},
    {"cpssPxPhyXsmiMdcDivisionFactorSet", 2, cpssPxPhySmiMdcDivisionFactorSet_PARAMS, NULL},
    {"cpssPxPhyXsmiMdcDivisionFactorGet", 2, cpssPxPhySmiMdcDivisionFactorGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_PX_CPSS_LOG_LIB_PHY(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssPxPhyLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssPxPhyLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

